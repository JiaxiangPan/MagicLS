/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024

/**
 * @file arithmetic.hpp
 *
 * @brief Generate arithmetic logic networks
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP

#include <utility>
#include <vector>
#include <deque>
#include <list>
#include <stack>

#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

namespace mockturtle
{
    namespace detail
    {
        // Multiplexer
        template <typename Ntk>
        inline signal<Ntk> mux(Ntk &ntk, signal<Ntk> cond, signal<Ntk> f_then, signal<Ntk> f_else)
        {
            return ntk.create_or(ntk.create_and(cond, f_then), ntk.create_and(!cond, f_else));
        }

        /* Creates generate signal and propagate signal for Manchester carry chain. */
        template <typename Ntk>
        inline void borrow_lookahead_subtractor_inplace_pow2(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
        {
            if (a.size() == 1u)
            {
                a[0] = full_subtractor(ntk, a[0], b[0], borrow).first;
                return;
            }

            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = borrow;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(ntk.create_not(f), g); });
            // std::transform( a.begin(), a.end(), b.begin(), pro.begin(), [&]( auto const& f, auto const& g ) { return ntk.create_xor( ntk.create_not( f ), g ); } );
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_or(ntk.create_not(f), g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            carry_lookahead_adder_inplace_rec(ntk, gen.begin(), gen.end(), pro.begin(), bor.begin());
            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
        }

        /*! \brief Creates borrow lookahead subtractor structure.
         *
         * Creates a borrow lookahead structure composed of full subtractors.  The vectors `a`
         * and `b` must have the same size.  The resulting different bits are eventually
         * stored in `a` and the borrow bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param borrow Borrow bit, will also have the output borrow after the call
         */
        template <typename Ntk>
        inline void borrow_lookahead_subtractor_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
        {
            /* extend bitsize to next power of two */
            const auto log2 = static_cast<uint32_t>(std::ceil(std::log2(static_cast<double>(a.size() + 1))));

            std::vector<signal<Ntk>> a_ext(a.begin(), a.end());
            a_ext.resize(static_cast<uint64_t>(1) << log2, ntk.get_constant(false));
            std::vector<signal<Ntk>> b_ext(b.begin(), b.end());
            b_ext.resize(static_cast<uint64_t>(1) << log2, ntk.get_constant(false));

            detail::borrow_lookahead_subtractor_inplace_pow2(ntk, a_ext, b_ext, borrow);

            std::copy_n(a_ext.begin(), a.size(), a.begin());
            borrow = a_ext[a.size()];
        }

        /* The processing module for generate signal and propagate signal */
        template <typename Ntk>
        class PG
        {
        public:
            PG() = delete;

            PG(signal<Ntk> a, signal<Ntk> b, uint32_t bit_width, uint32_t index) : g(a), p(b), begin(index), end(index)
            {
                assert(index <= bit_width);
            }

            void o_operation(Ntk &ntk, PG const &other)
            {
                this->g = ntk.create_or(this->g, ntk.create_and(this->p, other.g));
                this->p = ntk.create_and(this->p, other.p);
                assert(this->begin >= other.begin);
                this->end = other.end;
            }

            signal<Ntk> g, p;
            uint32_t begin;
            uint32_t end;
        };

        /*! \brief Creates brent-kung subtractor structure.
         *
         * Creates a brent-kung structure composed of full subtractors.  The vectors `a`
         * and `b` must have the same size.  The resulting different bits are eventually
         * stored in `a` and the borrow bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param borrow Borrow bit, will also have the output borrow after the call
         */
        template <typename Ntk>
        inline void brent_kung_subtractor_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
        {
            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = borrow;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(ntk.create_not(f), g); });
            // std::transform( a.begin(), a.end(), b.begin(), pro.begin(), [&]( auto const& f, auto const& g ) { return ntk.create_xor( ntk.create_not( f ), g ); } );
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_or(ntk.create_not(f), g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special borrow (equal to 1), which defaults to 0 */
            //   PG<Ntk> pg0( borrow, ntk.get_constant( false ), a.size(), 0 );
            //   pg[0].o_operation( ntk, pg0);

            /* The first round of carry generation */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                for (auto i = std::pow(2, j) - 1; i < a.size(); i += std::pow(2, j + 1))
                {
                    if (i + std::pow(2, j) >= pg.size())
                    {
                        continue;
                    }
                    pg[i + std::pow(2, j)].o_operation(ntk, pg[i]);
                }
            }

            // The second round of carry generation
            /* From left to right, as long as you encounter 'end' is not '0', first go to the right to find a pg[i] with 'end' '0',
             * and then to the right to find pg[j] with end just now pg[i]->begin+1
             * */
            for (auto i = pg.rbegin(); i != pg.rend(); i++)
            {
                if (i->end == 0)
                {
                    continue;
                }

                auto j = i; // find the first operand of "o"
                for (; j != pg.rend(); j++)
                {
                    if (j->end == 0)
                    {
                        break;
                    }
                }

                auto k = i; // find the second operand of "o"
                for (; k != j; k++)
                {
                    if (k->end == (j - 1)->end)
                    {
                        break;
                    }
                }

                k->o_operation(ntk, *j);

                if (k != i)
                {
                    i--;
                    continue;
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            // carry_lookahead_adder_inplace_rec( ntk, gen.begin(), gen.end(), pro.begin(), bor.begin() );
            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            borrow = bor.back();

            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }

        /*! \brief Creates brent-kung adder structure.
         *
         * Creates a brent-kung structure adder. The vectors `a`and `b` must have the same size.
         * The resulting sum bits are eventually stored in `a` and the carry bit
         * will be overriden to store the output carry bit.
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param carry Carry bit, will also have the output carry after the call
         */
        template <typename Ntk>
        inline void brent_kung_adder_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &carry)
        {
            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = carry;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special carry (equal to 1), which defaults to 0 */
            PG<Ntk> pg0(carry, ntk.get_constant(false), a.size(), 0);
            pg[0].o_operation(ntk, pg0);

            /* The first round of carry generation */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                for (auto i = std::pow(2, j) - 1; i < a.size(); i += std::pow(2, j + 1))
                {
                    if (i + std::pow(2, j) >= pg.size())
                    {
                        continue;
                    }
                    pg[i + std::pow(2, j)].o_operation(ntk, pg[i]);
                }
            }

            // The second round of carry generation
            /* From left to right, as long as you encounter 'end' is not '0', first go to the right to find a pg[i] with 'end' '0',
             * and then to the right to find pg[j] with end just now pg[i]->begin+1
             * */
            for (auto i = pg.rbegin(); i != pg.rend(); i++)
            {
                if (i->end == 0)
                {
                    continue;
                }

                auto j = i; // find the first operand of "o"
                for (; j != pg.rend(); j++)
                {
                    if (j->end == 0)
                    {
                        break;
                    }
                }

                auto k = i; // find the second operand of "o"
                for (; k != j; k++)
                {
                    if (k->end == (j - 1)->end)
                    {
                        break;
                    }
                }

                k->o_operation(ntk, *j);

                if (k != i)
                {
                    i--;
                    continue;
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            carry = bor.back();

            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }

        /*! \brief Creates kogge-stone subtractor structure.
         *
         * Creates a kogge-stone structure composed of full subtractors.  The vectors `a`
         * and `b` must have the same size.  The resulting different bits are eventually
         * stored in `a` and the borrow bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param borrow Borrow bit, will also have the output borrow after the call
         */
        template <typename Ntk>
        inline void kogge_stone_subtractor_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
        {
            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = borrow;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(ntk.create_not(f), g); });
            // std::transform( a.begin(), a.end(), b.begin(), pro.begin(), [&]( auto const& f, auto const& g ) { return ntk.create_xor( ntk.create_not( f ), g ); } );
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_or(ntk.create_not(f), g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special borrow, which defaults to 0 */
            PG<Ntk> pg0(borrow, ntk.get_constant(false), a.size(), 0);
            pg[0].o_operation(ntk, pg0);

            /* The first round of carry generation */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                auto pg_bak = pg;
                for (auto i = 0u; i < a.size(); i++)
                {
                    if (i + std::pow(2, j) >= pg.size())
                    {
                        break;
                    }
                    pg[i + std::pow(2, j)].o_operation(ntk, pg_bak[i]);
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            borrow = bor.back();

            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }

        /*! \brief Creates kogge-stone adder structure.
         *
         * Creates a kogge-stone adder composed of full subtractors.  The vectors `a`
         * and `b` must have the same size.  The resulting sum bits are eventually
         * stored in `a` and the carry bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param carry Carry bit, will also have the output carry after the call
         */
        template <typename Ntk>
        inline void kogge_stone_adder_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &carry)
        {
            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = carry;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); }); // what?

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special carry, which defaults to 0 */
            PG<Ntk> pg0(carry, ntk.get_constant(false), a.size(), 0);
            pg[0].o_operation(ntk, pg0);

            /* The first round of carry generation */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                auto pg_bak = pg;
                for (auto i = 0u; i < a.size(); i++)
                {
                    if (i + std::pow(2, j) >= pg.size())
                    {
                        break;
                    }
                    pg[i + std::pow(2, j)].o_operation(ntk, pg_bak[i]);
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            carry = bor.back();

            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }

        /*! \brief Creates han-carlson subtractor structure.
         *
         * Creates a han-carlson structure composed of full subtractors.  The vectors `a`
         * and `b` must have the same size.  The resulting different bits are eventually
         * stored in `a` and the borrow bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param borrow Borrow bit, will also have the output borrow after the call
         */
        template <typename Ntk>
        inline void han_carlson_subtractor_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
        {

            if (a.size() <= 4u)
            {
                brent_kung_subtractor_inplace(ntk, a, b, borrow);
                return;
            }

            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = borrow;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(ntk.create_not(f), g); });
            // std::transform( a.begin(), a.end(), b.begin(), pro.begin(), [&]( auto const& f, auto const& g ) { return ntk.create_xor( ntk.create_not( f ), g ); } );
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_or(ntk.create_not(f), g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special borrow, which defaults to 0 */
            //   PG<Ntk> pg0( borrow, ntk.get_constant( false ), a.size(), 0 );
            //   pg[0].o_operation( ntk, pg0);

#if 0
        /* The first 2 round of carry generation in first Brent-Kung stage */
        for( auto j = 0u; pg.rbegin()->end != 0 && j < 2; j++ )
        {
          for( auto i = std::pow(2, j)-1; i < a.size(); i += std::pow(2, j+1) )
          {
            if( i + std::pow(2, j) >= pg.size() )
            {
              continue;
            }
            pg[i + std::pow(2, j)].o_operation( ntk, pg[i] );
          }
        }

        /* The mid log_2(N)+2-4 round of carry generation in Kogge-Stone stage */
        //for( auto j = 2u; pg.rbegin()->end != 0 && j < std::ceil( std::log2( pg.size() ) - 2 ) + 2; j++ )
        for( auto j = 2u; pg.rbegin()->end != 0 && j < std::ceil( std::log2( pg.size() ) ); j++ )
        {
          auto pg_bak = pg;
          for( auto i = std::pow(2, 2)-1; i < a.size(); i += std::pow(2, 2) )
          {
            if( i + std::pow(2, j) >= pg.size() )
            {
              continue;
            }
            pg[i + std::pow(2, j)].o_operation( ntk, pg_bak[i] );
          }
        }
#endif

            /* There are log2(N)+2 rounds for advanced Han-Carlson structure, the fist 2 rounds along with the last 2 rounds are BK
             * and the mid rounds are KS
             * */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                if (j < 2u)
                {
                    /* The first 2 rounds of carry generation in first Brent-Kung stage */
                    for (auto i = std::pow(2, j) - 1; i < a.size(); i += std::pow(2, j + 1))
                    {
                        if (i + std::pow(2, j) >= pg.size())
                        {
                            continue;
                        }
                        pg[i + std::pow(2, j)].o_operation(ntk, pg[i]);
                    }
                }
                else
                {
                    /* The mid log2(N)+2-4 rounds of carry generation in Kogge-Stone stage. So there are log2(N)-2 rounds for KS,
                     * The KS is start with j = 2u round, so the last round of KS is j = log2(N)-2+2.*/
                    auto pg_bak = pg;
                    for (auto i = std::pow(2, 2) - 1; i < a.size(); i += std::pow(2, 2))
                    {
                        if (i + std::pow(2, j) >= pg.size())
                        {
                            continue;
                        }
                        pg[i + std::pow(2, j)].o_operation(ntk, pg_bak[i]);
                    }
                }
            }

            /* the last 2 rounds of carry generation in second Brent-Kung stage */
            for (auto i = pg.rbegin(); i != pg.rend(); i++)
            {
                if (i->end == 0)
                {
                    continue;
                }

                auto j = i; // find the first operand of "o"
                for (; j != pg.rend(); j++)
                {
                    if (j->end == 0)
                    {
                        break;
                    }
                }

                auto k = i; // find the second operand of "o"
                for (; k != j; k++)
                {
                    if (k->end == (j - 1)->end)
                    {
                        break;
                    }
                }

                k->o_operation(ntk, *j);

                if (k != i)
                {
                    i--;
                    continue;
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            borrow = bor.back();

            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }

        /*! \brief Creates han-carlson adder structure.
         *
         * Creates a han-carlson structure.  The vectors `a`
         * and `b` must have the same size.  The resulting sum bits are eventually
         * stored in `a` and thecarry bit will be overriden to store the output carry
         * bit.
         *
         * \param a First input operand, will also have the output after the call
         * \param b Second input operand
         * \param carry Carry bit, will also have the output carry after the call
         */
        template <typename Ntk>
        inline void han_carlson_adder_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &carry)
        {

            if (a.size() <= 4u)
            {
                brent_kung_adder_inplace(ntk, a, b, carry);
                return;
            }

            std::vector<signal<Ntk>> gen(a.size()), pro(a.size()), pro2(a.size()), bor(a.size() + 1);
            bor[0] = carry;
            std::transform(a.begin(), a.end(), b.begin(), gen.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_and(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            std::transform(a.begin(), a.end(), b.begin(), pro2.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });

            std::vector<PG<Ntk>> pg;
            std::vector<PG<Ntk> *> pg_ptr;
            for (auto i = 0u; i < a.size(); i++)
            {
                PG<Ntk> *pg_i = new PG<Ntk>(gen[i], pro[i], a.size(), i);
                pg.emplace_back(*pg_i);
                pg_ptr.emplace_back(pg_i);
            }

            /* This section is used if there is a special carry, which defaults to 0 */
            //   PG<Ntk> pg0( carry, ntk.get_constant( false ), a.size(), 0 );
            //   pg[0].o_operation( ntk, pg0);

            /* There are log2(N)+2 rounds for advanced Han-Carlson structure, the fist 2 rounds along with the last 2 rounds are BK
             * and the mid rounds are KS
             * */
            for (auto j = 0u; pg.rbegin()->end != 0 && j < std::ceil(std::log2(pg.size())); j++)
            {
                if (j < 2u)
                {
                    /* The first 2 rounds of carry generation in first Brent-Kung stage */
                    for (auto i = std::pow(2, j) - 1; i < a.size(); i += std::pow(2, j + 1))
                    {
                        if (i + std::pow(2, j) >= pg.size())
                        {
                            continue;
                        }
                        pg[i + std::pow(2, j)].o_operation(ntk, pg[i]);
                    }
                }
                else
                {
                    /* The mid log2(N)+2-4 rounds of carry generation in Kogge-Stone stage. So there are log2(N)-2 rounds for KS,
                     * The KS is start with j = 2u round, so the last round of KS is j = log2(N)-2+2.*/
                    auto pg_bak = pg;
                    for (auto i = std::pow(2, 2) - 1; i < a.size(); i += std::pow(2, 2))
                    {
                        if (i + std::pow(2, j) >= pg.size())
                        {
                            continue;
                        }
                        pg[i + std::pow(2, j)].o_operation(ntk, pg_bak[i]);
                    }
                }
            }

            /* the last 2 rounds of carry generation in second Brent-Kung stage */
            for (auto i = pg.rbegin(); i != pg.rend(); i++)
            {
                if (i->end == 0)
                {
                    continue;
                }

                auto j = i; // find the first operand of "o"
                for (; j != pg.rend(); j++)
                {
                    if (j->end == 0)
                    {
                        break;
                    }
                }

                auto k = i; // find the second operand of "o"
                for (; k != j; k++)
                {
                    if (k->end == (j - 1)->end)
                    {
                        break;
                    }
                }

                k->o_operation(ntk, *j);

                if (k != i)
                {
                    i--;
                    continue;
                }
            }

            for (auto i = 1u; i < bor.size(); i++)
            {
                bor[i] = pg[i - 1].g;
            }

            // carry_lookahead_adder_inplace_rec( ntk, gen.begin(), gen.end(), pro.begin(), bor.begin() );
            std::transform(pro2.begin(), pro2.end(), bor.begin(), a.begin(), [&](auto const &f, auto const &g)
                           { return ntk.create_xor(f, g); });
            carry = bor.back();
            for_each(pg_ptr.begin(), pg_ptr.end(), [&](auto &pg)
                     { delete pg; });
        }
    } // detail

    /*! \brief Inserts a full subtract into a network.
     *
     * Inserts a full subtract for three inputs (two 1-bit operands and one borrow)
     * into the network and returns a pair of diferrence and borrow bit.
     *
     * By default creates a seven 2-input gate network composed of AND, NOR, and OR
     * gates.  If network has `create_node` function, creates two 3-input gate
     * network.  If the network has ternary `create_maj` and `create_xor3`
     * functions, it will use them (except for AIGs).
     *
     * \param ntk Network
     * \param a First input operand
     * \param b Second input operand
     * \param c Borrow
     * \return Pair of sum (`first`) and carry (`second`)
     */
    template <typename Ntk>
    inline std::pair<signal<Ntk>, signal<Ntk>> full_subtractor(Ntk &ntk, const signal<Ntk> &a, const signal<Ntk> &b, const signal<Ntk> &c)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");

        /* specialization for LUT-ish networks */
        if constexpr (has_create_node_v<Ntk>)
        {
            kitty::dynamic_truth_table tt_borrow(3u), tt_xor(3u);
            kitty::create_from_hex_string(tt_borrow, "4e");
            kitty::create_from_hex_string(tt_xor, "96");

            const auto difference = ntk.create_node({a, b, c}, tt_xor);
            const auto borrow = ntk.create_node({a, b, c}, tt_borrow);

            return {difference, borrow};
        }
        /* use MAJ and XOR3 if available by network, unless network is AIG */
        else if constexpr (!std::is_same_v<typename Ntk::base_type, aig_network> && has_create_maj_v<Ntk> && has_create_xor3_v<Ntk>)
        {
            const auto borrow = ntk.create_maj(!a, b, c);
            const auto difference = ntk.create_xor3(a, b, c);
            return {difference, borrow};
        }
        else
        {
            static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
            static_assert(has_create_nor_v<Ntk>, "Ntk does not implement the create_nor method");
            static_assert(has_create_or_v<Ntk>, "Ntk does not implement the create_or method");

            const auto w1 = ntk.create_and(a, !c);
            const auto w2 = ntk.create_and(!a, c);
            const auto w3 = ntk.create_nor(w1, w2);
            const auto w4 = ntk.create_and(!b, w3);
            const auto w5 = ntk.create_and(b, !w3);
            const auto sum = ntk.create_nor(w4, w5);
            const auto carry = ntk.create_nor(w1, w4);

            return {sum, carry};
        }
    }

    /*! \brief Creates borrow ripple subtractor structure.
     *
     * Creates a borrow ripple structure composed of full subtractors.  The vectors `a`
     * and `b` must have the same size.  The resulting difference bits are eventually
     * stored in `a` and the borrow bit will be overriden to store the output borrow
     * bit.
     *
     * \param a First input operand, will also have the output after the call
     * \param b Second input operand
     * \param borrow Borrow bit, will also have the output borrow after the call
     */

    /* Inplace */
    template <typename Ntk>
    inline void borrow_ripple_subtractor_inplace(Ntk &ntk, std::vector<signal<Ntk>> &a, std::vector<signal<Ntk>> const &b, signal<Ntk> &borrow)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");

        assert(a.size() == b.size());

        auto pa = a.begin();
        for (auto pb = b.begin(); pa != a.end(); ++pa, ++pb)
        {
            std::tie(*pa, borrow) = full_subtractor(ntk, *pa, *pb, borrow);
        }
    }

    /* Not Inplace --------- not use so far*/
    template <typename Ntk>
    inline std::pair<std::vector<signal<Ntk>>, signal<Ntk>> borrow_ripple_subtractor(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b, signal<Ntk> const &borrow)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");

        assert(a.size() == b.size());

        auto x{a};
        auto z{borrow};

        auto pa = x.begin();
        for (auto pb = b.begin(); pa != x.end(); ++pa, ++pb)
        {
            std::tie(*pa, z) = full_subtractor(ntk, *pa, *pb, z);
        }
        return {x, z}; //{difference, borrow}
    }

    /* =================================================================================== */

    /*! \brief Creates a classical unsigned restoring array divider.
     *
     * The vectors `a` and `b` must be 2n/n bits. The function creates
     * the divider in `ntk` and returns output signals.
     *
     *
     * \param ntk Network
     * \param a First input operand
     * \param b Second input operand
     */

    /* Not use so far because 2n/n is not efficient and sufficient*/
    template <typename Ntk>
    inline std::pair<std::vector<signal<Ntk>>, std::vector<signal<Ntk>>> restoring_array_divider_bak(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
        static_assert(has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method");

        assert(a.size() == 2 * b.size());

        auto N = b.size();

        auto quo = constant_word(ntk, 0, static_cast<uint32_t>(N));
        auto rem = constant_word(ntk, 0, static_cast<uint32_t>(N));

        std::vector<signal<Ntk>> part_rem(a.begin() + N - 1, a.begin() + 2 * N - 1);
        auto qsd_signal = a[2 * N - 1];

        for (auto i = 0u; i < N; i++)
        {
            auto tmp = part_rem;
            auto carry = ntk.get_constant(true);
            carry_ripple_subtractor_inplace(ntk, part_rem, b, carry);

            quo[N - i - 1] = ntk.create_xor(qsd_signal, !carry);

            mux_inplace(ntk, quo[N - i - 1], part_rem, tmp);
        }
        rem = part_rem;

        return {quo, rem};
    }

    /*! \brief Creates a classical unsigned n/n restoring array divider.
     *
     * The vectors `a` and `b` must have the same size. The function creates
     * the divider in `ntk` and returns output signals.
     *
     *
     * \param ntk Network
     * \param a First input operand
     * \param b Second input operand
     */
    template <typename Ntk>
    inline std::pair<std::vector<signal<Ntk>>, std::vector<signal<Ntk>>> restoring_array_divider(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
        static_assert(has_create_not_v<Ntk>, "Ntk does not implement the create_not method");
        static_assert(has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method");

        assert(a.size() == b.size());

        auto x{a};
        auto y{b};

        // get the bit width
        auto N = x.size();

        // b shift left by N-1 bits to prevent overflow
        y.insert(y.begin(), N - 1, ntk.get_constant(false));

        std::vector<signal<Ntk>> quo;

        // partial remainder keep the same size with divider
        std::vector<signal<Ntk>> p_rem(x);
        p_rem = zero_extend(ntk, p_rem, y.size());

        for (auto i = 0u; i < N; i++)
        {
            auto tmp = p_rem;

            auto borrow = ntk.get_constant(false);
            borrow_ripple_subtractor_inplace(ntk, p_rem, y, borrow);

            // quo.emplace_back( ntk.create_or( ntk.get_constant( false ), ntk.create_not( borrow ) ) );
            quo.emplace_back(ntk.create_not(borrow));

            mux_inplace(ntk, quo.back(), p_rem, tmp);

            y.erase(y.begin()); // b shift right 1 bit
            p_rem.pop_back();   // keep the same size with divider
        }

        // correspond the norm
        reverse(quo.begin(), quo.end());

        return {quo, p_rem};
    }

    template <typename Ntk>
    inline std::pair<std::vector<signal<Ntk>>, std::vector<signal<Ntk>>> restoring_array_divider_advance(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b, void (*func)(Ntk &, std::vector<signal<Ntk>> &, std::vector<signal<Ntk>> const &, signal<Ntk> &))
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
        static_assert(has_create_or_v<Ntk>, "Ntk does not implement the create_or method");
        static_assert(has_create_not_v<Ntk>, "Ntk does not implement the create_not method");
        static_assert(has_create_nary_or_v<Ntk>, "Ntk does not implement the create_nary_or method");
        static_assert(has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method");

        assert(a.size() == b.size());

        auto logic0 = ntk.get_constant(false);

        auto x{a};
        auto y{b};

        // get the bit width
        auto N = x.size();

        // b shift left by N-1 bits to prevent overflow, so b'size is 2*N-1
        y.insert(y.begin(), N - 1, logic0);

        std::vector<signal<Ntk>> quo;

        // partial remainder
        auto p_rem = x;

        for (auto i = 0u; i < N && y.size() > N; i++)
        {
            // get the most N bits and the least N bits of divisor
            std::vector<signal<Ntk>> y_mNbit(y.begin() + N, y.end());
            std::vector<signal<Ntk>> y_lNbit(y.begin(), y.begin() + N);

            // get result of the most significant (y.size() -N ) bit with OR gate
            auto cond = ntk.create_nary_or(y_mNbit);

            auto tmp_prem = p_rem;

            auto borrow = logic0;
            // borrow_ripple_subtractor_inplace( ntk, p_rem, y_lNbit, borrow );
            func(ntk, p_rem, y_lNbit, borrow);

            quo.emplace_back(detail::mux(ntk, cond, ntk.create_not(cond), ntk.create_not(borrow)));

            p_rem = mux(ntk, cond, tmp_prem, mux(ntk, borrow, tmp_prem, p_rem));

            y.erase(y.begin()); // b shift right 1 bit
        }

        auto tmp_prem = p_rem;

        std::vector<signal<Ntk>> y_lNbit(y.begin(), y.begin() + N);
        auto borrow = logic0;
        // borrow_ripple_subtractor_inplace( ntk, p_rem, y_lNbit, borrow );
        func(ntk, p_rem, y_lNbit, borrow);

        // quo.emplace_back( ntk.create_or( ntk.get_constant( false ), ntk.create_not( borrow ) ) );
        quo.emplace_back(ntk.create_not(borrow));

        mux_inplace(ntk, quo.back(), p_rem, tmp_prem);

        y.erase(y.begin()); // b shift right 1 bit

        // correspond the norm
        reverse(quo.begin(), quo.end());

        return {quo, p_rem};
    }

    template <typename Ntk>
    inline std::pair<signal<Ntk>, signal<Ntk>> kogge_stone_full_adder(Ntk &ntk, const signal<Ntk> &a, const signal<Ntk> &b, const signal<Ntk> &c)
    {
        std::vector<signal<Ntk>> A{}, B{};
        A.emplace_back(a);
        B.emplace_back(b);
        signal<Ntk> carry = ntk.create_buf(c);
        mockturtle::detail::kogge_stone_adder_inplace(ntk, A, B, carry);
        // return { ntk.create_buf( A[0] ),  ntk.create_buf( carry ) };
        return {A[0], carry};
    }

    // new multiplier based on kogge_stone_full_adder
    template <typename Ntk>
    inline std::vector<signal<Ntk>> new_multiplier(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b)
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
        static_assert(has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method");

        auto res = constant_word(ntk, 0, static_cast<uint32_t>(a.size() + b.size()));
        auto tmp = constant_word(ntk, 0, static_cast<uint32_t>(a.size() * 2));

        for (auto j = 0u; j < b.size(); ++j)
        {
            for (auto i = 0u; i < a.size(); ++i)
            {
                std::tie(i ? tmp[a.size() + i - 1] : res[j], tmp[i]) = kogge_stone_full_adder(ntk, ntk.create_and(a[i], b[j]), tmp[a.size() + i], tmp[i]);
            }
        }

        auto carry = tmp.back() = ntk.get_constant(false);
        for (auto i = 0u; i < a.size(); ++i)
        {
            std::tie(res[b.size() + i], carry) = kogge_stone_full_adder(ntk, tmp[i], tmp[a.size() + i], carry);
        }

        return res;
    }

    // multiplier based on multi-bit full_adder
    template <typename Ntk>
    inline std::vector<signal<Ntk>> advance_multiplier(Ntk &ntk, std::vector<signal<Ntk>> const &a, std::vector<signal<Ntk>> const &b, void (*func)(Ntk &, std::vector<signal<Ntk>> &, std::vector<signal<Ntk>> const &, signal<Ntk> &))
    {
        static_assert(is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(has_create_and_v<Ntk>, "Ntk does not implement the create_and method");
        static_assert(has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method");

        uint32_t res_bit = a.size() + b.size();

        std::vector<std::vector<signal<Ntk>>> partial_product{};

        // partial_product generation
        for (auto j = 0u; j < b.size(); ++j)
        {
            std::vector<signal<Ntk>> products{};
            for (auto i = 0u; i < a.size(); ++i)
            {
                auto product = ntk.create_and(a[i], b[j]);
                products.push_back(product);
            }
            partial_product.push_back(products);
        }

        // extend partial_product to a+b bit
        for (auto i = 0u; i < partial_product.size(); ++i)
        {
            if (i == 0)
            {
                partial_product[i] = zero_extend(ntk, partial_product[i], res_bit);
            }
            else
            {
                partial_product[i].insert(partial_product[i].begin(), i, ntk.get_constant(false));
                assert(partial_product[i].size() <= res_bit);
                partial_product[i] = zero_extend(ntk, partial_product[i], res_bit);
            }
        }

        // n bit x n bit
        std::vector<signal<Ntk>> multiplier_result{};

        uint32_t add_num = b.size() - 1; // number of adder, also equals to "partial_product.size() - 1"

        for (auto i = 0u; i < partial_product.size() - 1; ++i)
        {
            signal<Ntk> carry = ntk.get_constant(false);
            // mockturtle::detail::han_carlson_adder_inplace( ntk, partial_product[i+1], partial_product[i], carry );
            func(ntk, partial_product[i + 1], partial_product[i], carry);
            if (i == (partial_product.size() - 2))
                multiplier_result = partial_product[i + 1];
        }

        return multiplier_result;

        // // n bit x 2 bit( cec pass )
        // signal<Ntk> carry = ntk.get_constant( false );
        // mockturtle::detail::han_carlson_adder_inplace( ntk, partial_product[0], partial_product[1], carry );

        // return partial_product[0];
    }

} // namespace mockturtle

#endif

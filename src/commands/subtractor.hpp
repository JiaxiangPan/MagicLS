/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file subtractor.hpp
 *
 * @brief  Generator for subtractor
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef SUBTRACTOR_HPP
#define SUBTRACTOR_HPP

#include <iostream>
#include <vector>
#include <utility>

#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/algorithms/cleanup.hpp>

#include "../core/arithmetic.hpp"
#include "../core/my_function.hpp"

using namespace std;
using namespace mockturtle;

namespace alice
{
    class subtractor_command : public command
    {
    public:
        explicit subtractor_command(const environment::ptr &env)
            : command(env, "Create subtractor logic network [default = AIG]")
        {
            add_option("-b, --bit", BIT, "set the bit width");
            add_flag("--carry_ripple_subtractor, -f", "create carry ripple subtractor(based on 1bit full adder)");
            add_flag("--borrow_ripple_subtractor, -B", "create borrow ripple subtractor(based on 1bit full subtractor)");
            add_flag("--borrow_lookahead_subtractor, -l", "create borrow lookahead subtractor");
            add_flag("--brent_kung_subtractor, -g", "create brent kung subtractor");
            add_flag("--kogge_stone_subtractor, -k", "create kogge stone subtractor");
            add_flag("--han_carlson_subtractor, -c", "create han carlson subtractor");
        }

    protected:
        void execute()
        {
            if (is_set("bit"))
            {
                if (is_set("carry_ripple_subtractor"))
                {
                    std::cout << "carry_ripple_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.create_not(aig.get_constant(false));
                    // Based on the full adder, carry signal need to be inverted
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    carry_ripple_subtractor_inplace(aig, a, b, carry);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(carry);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("borrow_ripple_subtractor"))
                {
                    std::cout << "borrow_ripple_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    borrow_ripple_subtractor_inplace(aig, a, b, borrow);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(borrow);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("borrow_lookahead_subtractor"))
                {
                    std::cout << "borrow_lookahead_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::borrow_lookahead_subtractor_inplace(aig, a, b, borrow);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(borrow);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("brent_kung_subtractor"))
                {
                    std::cout << "brent_kung_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::brent_kung_subtractor_inplace(aig, a, b, borrow);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(borrow);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("kogge_stone_subtractor"))
                {
                    std::cout << "kogge_stone_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::kogge_stone_subtractor_inplace(aig, a, b, borrow);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(borrow);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("han_carlson_subtractor"))
                {
                    std::cout << "han_carlson_subtractor\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::han_carlson_subtractor_inplace(aig, a, b, borrow);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    // aig.create_po(borrow);
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else
                {
                    std::cerr << "select one flag!" << std::endl;
                }
            }
        }

    private:
        __uint32_t BIT = 0u;
    };

    ALICE_ADD_COMMAND(subtractor, "Generator")

} // namespace alice

#endif

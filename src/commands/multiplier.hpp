/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file multiplier.hpp
 *
 * @brief Generator for multiplier
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef MULTIPLIER_HPP
#define MULTIPLIER_HPP

#include <iostream>
#include <vector>
#include <utility>

#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <kitty/dynamic_truth_table.hpp>

#include "../core/arithmetic.hpp"
#include "../core/my_function.hpp"

using namespace std;
using namespace mockturtle;

namespace alice
{
    class multiplier_command : public command
    {
    public:
        explicit multiplier_command(const environment::ptr &env)
            : command(env, "Create multiplier logic network [default = AIG]")
        {
            add_option("-b, --bit", BIT, "set the bit width");
            add_option("-B, --bit1", BIT_1, "set the second bit width for multiplier");
            add_flag("--carry_ripple_multiplier, -m", "create carry ripple multiplier based on full adder");
            add_flag("--new_multiplier, -n", "create new multiplier based on kogge-stone based full adder");
            add_option("-a, --advance", func, "set the advanced adder to the partial product adder function, set{brent-kung; kogge-stone; han-carlson;}");
            add_flag("--print_tt, -p", "print the network's output truth table (BIT <= 8).");
        }

    protected:
        void execute()
        {
            if (is_set("bit"))
            {
                if (is_set("carry_ripple_multiplier"))
                {
                    std::cout << "carry_ripple_multiplier\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a{}, b{};
                    if (is_set("bit1"))
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT_1);
                        std::cout << "multiplicand bit: " << BIT << " multiplier bit: " << BIT_1 << std::endl;
                        a = A;
                        b = B;
                    }
                    else
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT);
                        std::cout << "multiplicand and multiplier bit are same: " << BIT << std::endl;
                        a = A;
                        b = B;
                    }
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::vector<aig_network::signal> res = carry_ripple_multiplier(aig, a, b);
                    for (const auto &out : res)
                    {
                        aig.create_po(out);
                    }
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("new_multiplier"))
                {
                    std::cout << "multiplier based on kogge-stone based full adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a{}, b{};
                    if (is_set("bit1"))
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT_1);
                        std::cout << "multiplicand bit: " << BIT << " multiplier bit: " << BIT_1 << std::endl;
                        a = A;
                        b = B;
                    }
                    else
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT);
                        std::cout << "multiplicand and multiplier bit are same: " << BIT << std::endl;
                        a = A;
                        b = B;
                    }
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::vector<aig_network::signal> res = new_multiplier(aig, a, b);
                    for (const auto &out : res)
                    {
                        aig.create_po(out);
                    }
                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("advance"))
                {
                    aig_network aig;
                    std::vector<aig_network::signal> a{}, b{};
                    if (is_set("bit1"))
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT_1);
                        std::cout << "multiplicand bit: " << BIT << " multiplier bit: " << BIT_1 << std::endl;
                        a = A;
                        b = B;
                    }
                    else
                    {
                        std::vector<aig_network::signal> A(BIT), B(BIT);
                        std::cout << "multiplicand and multiplier bit are same: " << BIT << std::endl;
                        a = A;
                        b = B;
                    }
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::vector<aig_network::signal> results{};
                    if (func == "brent-kung")
                    {
                        std::cout << "multiplier based on partial product adder function: brent-kung\n";
                        results = advance_multiplier(aig, a, b, mockturtle::detail::brent_kung_adder_inplace);
                    }
                    else if (func == "kogge-stone")
                    {
                        std::cout << "multiplier based on partial product adder function: kogge-stone\n";
                        results = advance_multiplier(aig, a, b, mockturtle::detail::kogge_stone_adder_inplace);
                    }
                    else if (func == "han-carlson")
                    {
                        std::cout << "multiplier based on partial product adder function: han-carlson\n";
                        results = advance_multiplier(aig, a, b, mockturtle::detail::han_carlson_adder_inplace);
                    }
                    else
                    {
                        std::cout << "error: no " << func << "function!" << std::endl;
                        exit(1);
                    }
                    assert(results.size() == (a.size() + b.size()));
                    for (const auto &result : results)
                    {
                        aig.create_po(result);
                    }
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
        __uint32_t BIT_1 = 0u; // for different bit in multiplier
        std::string func = "";
    };

    ALICE_ADD_COMMAND(multiplier, "Generator")

} // namespace alice

#endif

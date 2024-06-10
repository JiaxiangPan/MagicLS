/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file adder.hpp
 *
 * @brief  Generator for adder
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef ADDER_HPP
#define ADDER_HPP

#include <iostream>
#include <vector>
#include <utility>

#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/xmg.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <kitty/dynamic_truth_table.hpp>

#include "../core/arithmetic.hpp"
#include "../core/my_function.hpp"

using namespace std;
using namespace mockturtle;

namespace alice
{
    class adder_command : public command
    {
    public:
        explicit adder_command(const environment::ptr &env)
            : command(env, "Create adder logic network [default = AIG]")
        {
            add_option("-b, --bit", BIT, "set the bit width");
            add_flag("--half_adder, -H", "create half adder");
            add_flag("--full_adder, -f", "create full adder");
            add_flag("--carry_ripple_adder, -r", "create carry_ripple_adder(Based on full adder)");
            add_flag("--carry_lookahead_adder, -l", "create carry lookahead adder");

            add_flag("--borrow_ripple_subtractor, -B", "create adder based on borrow ripple subtractor(based on 1bit full subtractor)");
            add_flag("--borrow_lookahead_subtractor, -L", "create adder based on borrow lookahead subtractor");
            add_flag("--brent_kung_adder, -g", "create adder based on brent kung adder");
            add_flag("--kogge_stone_adder, -k", "create adder based on kogge stone adder");
            add_flag("--han_carlson_adder, -c", "create adder based on han carlson adder");

            add_flag("--xmg, -x", "Construct adder(BRS BLS) by XMG."); // TO DO
            add_flag("--print_tt, -p", "print the network's output truth table (BIT <= 8).");
        }

    protected:
        void execute()
        {
            if (is_set("bit"))
            {
                if (is_set("carry_ripple_adder"))
                {
                    std::cout << "carry_ripple_adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.get_constant(false); // carry equals to zero
                    // aig_network::signal carry = aig.create_pi();
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    carry_ripple_adder_inplace(aig, a, b, carry);

                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(carry);

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                    }

                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("carry_lookahead_adder"))
                {
                    std::cout << "carry_lookahead_adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });
                    carry_lookahead_adder_inplace(aig, a, b, carry);
                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(carry);

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                    }

                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("borrow_ripple_subtractor"))
                {
                    if (is_set("xmg"))
                    {

                        xmg_network xmg;
                        std::vector<xmg_network::signal> a(BIT), b(BIT);
                        // xmg_network::signal borrow =  xmg.create_not(xmg.get_constant( false ));
                        xmg_network::signal borrow = xmg.get_constant(true);
                        std::generate(a.begin(), a.end(), [&xmg]()
                                      { return xmg.create_pi(); });
                        std::generate(b.begin(), b.end(), [&xmg]()
                                      { return xmg.create_not(xmg.create_pi()); });

                        borrow_ripple_subtractor_inplace(xmg, a, b, borrow);
                        for (const auto &out : a)
                        {
                            xmg.create_po(out);
                        }
                        xmg.create_po(xmg.create_not(borrow));

                        if (is_set("print_tt"))
                        {
                            default_simulator<kitty::dynamic_truth_table> sim(xmg.num_pis());
                            const auto tts = simulate<kitty::dynamic_truth_table>(xmg, sim);

                            xmg.foreach_po([&](auto const &, auto i)
                                           { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                        }

                        xmg = cleanup_dangling(xmg);

                        store<xmg_network>().extend();
                        store<xmg_network>().current() = xmg;

                        MagicLS::print_stats(xmg);
                    }
                    else
                    {
                        aig_network aig;
                        std::vector<aig_network::signal> a(BIT), b(BIT);
                        aig_network::signal borrow = aig.get_constant(true);
                        std::generate(a.begin(), a.end(), [&aig]()
                                      { return aig.create_pi(); });
                        std::generate(b.begin(), b.end(), [&aig]()
                                      { return aig.create_not(aig.create_pi()); });

                        borrow_ripple_subtractor_inplace(aig, a, b, borrow);
                        for (const auto &out : a)
                        {
                            aig.create_po(out);
                        }
                        aig.create_po(aig.create_not(borrow));

                        if (is_set("print_tt"))
                        {
                            default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                            const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                            aig.foreach_po([&](auto const &, auto i)
                                           { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                        }

                        aig = cleanup_dangling(aig);

                        store<aig_network>().extend();
                        store<aig_network>().current() = aig;

                        MagicLS::print_stats(aig);
                    }
                }
                else if (is_set("borrow_lookahead_subtractor"))
                {

                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal borrow = aig.get_constant(true);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_not(aig.create_pi()); });

                    mockturtle::detail::borrow_lookahead_subtractor_inplace(aig, a, b, borrow);
                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(aig.create_not(borrow));

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                    }

                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("brent_kung_adder"))
                {
                    std::cout << "brent_kung_adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::brent_kung_adder_inplace(aig, a, b, carry);
                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(carry);

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                    }

                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("kogge_stone_adder"))
                {
                    std::cout << "kogge_stone_adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.get_constant(false);
                    // aig_network::signal carry = aig.create_pi();
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::kogge_stone_adder_inplace(aig, a, b, carry);
                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(carry);

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                    }

                    aig = cleanup_dangling(aig);

                    store<aig_network>().extend();
                    store<aig_network>().current() = aig;

                    MagicLS::print_stats(aig);
                }
                else if (is_set("han_carlson_adder"))
                {
                    std::cout << "han_carlson_adder\n";
                    aig_network aig;
                    std::vector<aig_network::signal> a(BIT), b(BIT);
                    aig_network::signal carry = aig.get_constant(false);
                    std::generate(a.begin(), a.end(), [&aig]()
                                  { return aig.create_pi(); });
                    std::generate(b.begin(), b.end(), [&aig]()
                                  { return aig.create_pi(); });

                    mockturtle::detail::han_carlson_adder_inplace(aig, a, b, carry);
                    for (const auto &out : a)
                    {
                        aig.create_po(out);
                    }
                    aig.create_po(carry);

                    if (is_set("print_tt"))
                    {
                        default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                        const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                        aig.foreach_po([&](auto const &, auto i)
                                       { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
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
            if (is_set("half_adder"))
            {
                std::cout << "half_adder\n";
                aig_network aig;
                aig_network::signal a = aig.create_pi(), b = aig.create_pi();
                auto [sum, carry] = half_adder(aig, a, b);
                aig.create_po(sum);
                aig.create_po(carry);

                if (is_set("print_tt"))
                {
                    default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                    const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                    aig.foreach_po([&](auto const &, auto i)
                                   { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                }

                aig = cleanup_dangling(aig);

                store<aig_network>().extend();
                store<aig_network>().current() = aig;

                MagicLS::print_stats(aig);
            }
            if (is_set("full_adder"))
            {
                std::cout << "full_adder\n";
                aig_network aig;
                aig_network::signal a = aig.create_pi(), b = aig.create_pi(), c = aig.create_pi();
                auto [sum, carry] = full_adder(aig, a, b, c);
                aig.create_po(sum);
                aig.create_po(carry);

                if (is_set("print_tt"))
                {
                    default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
                    const auto tts = simulate<kitty::dynamic_truth_table>(aig, sim);

                    aig.foreach_po([&](auto const &, auto i)
                                   { std::cout << fmt::format("truth table of output {} is {}\n", i, kitty::to_hex(tts[i])); });
                }

                aig = cleanup_dangling(aig);

                store<aig_network>().extend();
                store<aig_network>().current() = aig;

                MagicLS::print_stats(aig);
            }
        }

    private:
        __uint32_t BIT = 0u;
    };

    ALICE_ADD_COMMAND(adder, "Generator")

} // namespace alice

#endif

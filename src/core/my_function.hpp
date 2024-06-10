#pragma once

#include <mockturtle/mockturtle.hpp>

using namespace mockturtle;

namespace MagicLS
{
    template <class Ntk>
    void print_stats(const Ntk &ntk)
    {
        using depth_ntk = depth_view<Ntk>;
        std::cout << fmt::format("ntk   i/o = {}/{}   gates = {}   level = {}\n",
                                 ntk.num_pis(), ntk.num_pos(), ntk.num_gates(), depth_ntk{ntk}.depth());
    }
}
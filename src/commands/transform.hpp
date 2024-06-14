/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file transform.hpp
 *
 * @brief  transform network between AIG and GIA
 *
 * @author Jiaxiang Pan
 * @since  2024/06/13
 */

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <string>

#include <mockturtle/networks/aig.hpp>
#include "../core/abc_gia.hpp"
#include "../core/abc.hpp"

using namespace std;
using namespace mockturtle;

namespace alice
{
    class transform_command : public command
    {
    public:
        explicit transform_command(const environment::ptr &env)
            : command(env, "transform network between AIG and GIA")
        {
            add_flag("--aig2gia, -a", "convert aig to gia");
            add_flag("--gia2aig, -g", "convert gia to aig");
            add_option("-s, --string", script, "set the opt string in ABC9 [default = &ps]");
        }

    protected:
        void execute()
        {
            if (is_set("aig2gia"))
            {
                if (store<mockturtle::aig_network>().size() == 0)
                {
                    std::cerr << "NO AIG\n";
                    return;
                }
                mockturtle::aig_network aig = store<mockturtle::aig_network>().current();
                fmt::print(" [AIG] PI/PO = {}/{}  nodes = {}  level = {}\n ", aig.num_pis(), aig.num_pos(), aig.num_gates(), mockturtle::depth_view(aig).depth());
                mockturtle::gia_network gia(aig.size() << 1);
                aig_to_gia(gia, aig);
                fmt::print(" Before [GIA] PI/PO = {}/{}  nodes = {}  level = {}\n ", gia.num_pis(), gia.num_pos(), gia.num_gates(), gia.num_levels());
                gia.run_opt_script(script);
                fmt::print(" After [GIA] PI/PO = {}/{}  nodes = {}  level = {}\n ", gia.num_pis(), gia.num_pos(), gia.num_gates(), gia.num_levels());

                store<pabc::Gia_Man_t *>().extend();
                store<pabc::Gia_Man_t *>().current() = gia.get_gia();
            }
            else if(is_set("gia2aig"))
            {
                if (store<pabc::Gia_Man_t *>().size() == 0)
                {
                    std::cerr << "NO GIA\n";
                    return;
                }
                pabc::Gia_Man_t * gia_ntk = store<pabc::Gia_Man_t *>().current();
                mockturtle::gia_network gia( gia_ntk );
                fmt::print(" Before [GIA] PI/PO = {}/{}  nodes = {}  level = {}\n ", gia.num_pis(), gia.num_pos(), gia.num_gates(), gia.num_levels());
                mockturtle::aig_network aig;
                gia_to_aig( aig, gia );
                fmt::print(" [AIG] PI/PO = {}/{}  nodes = {}  level = {}\n ", aig.num_pis(), aig.num_pos(), aig.num_gates(), mockturtle::depth_view(aig).depth());
                store<mockturtle::aig_network>().extend();
                store<mockturtle::aig_network>().current() = aig;
            }
            else 
            {
                std::cerr << "no this flag\n";
                return;
            }
        }

    private:
        std::string script = "&ps";//default &ps
    };

    ALICE_ADD_COMMAND(transform, "General")

} // namespace alice

#endif

/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file dc2.hpp
 *
 * @brief performs combinational AIG optimization
 *
 * @author Jiaxiang Pan
 * @since  2024/06/11
 */

#ifndef DC2_HPP
#define DC2_HPP

#include "base/abc/abc.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class dc2_command : public command
    {
    public:
        explicit dc2_command(const environment::ptr &env) : command(env, "performs combinational AIG optimization")
        {
            add_flag("--balance, -b", "toggle internal balancing [default = no]");
            add_flag("--updateLevel, -l", "toggle updating level [default = no]");
            add_flag("--fanout, -f", "toggle representing fanouts [default = yes]");
            add_flag("--power, -p", "toggle power-aware rewriting [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;

            if (is_set("updateLevel"))
                fUpdateLevel ^= 1;
            if (is_set("fanout"))
                fFanout ^= 1;
            if (is_set("power"))
                fPower ^= 1;
            if (is_set("verbose"))
                fVerbose ^= 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty ABC AIG network\n";
            else
            {
                Abc_Ntk_t *pNtk, *pNtkRes;
                pNtk = store<pabc::Abc_Ntk_t *>().current();
                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (!Abc_NtkIsStrash(pNtk))
                {
                    Abc_Print(-1, "This command works only for strashed networks.\n");
                    return;
                }
                pNtkRes =
                    Abc_NtkDC2(pNtk, fBalance, fUpdateLevel, fFanout, fPower, fVerbose);
                if (pNtkRes == NULL)
                {
                    Abc_Print(-1, "Command has failed.\n");
                    return;
                }

                store<pabc::Abc_Ntk_t *>().extend();
                store<pabc::Abc_Ntk_t *>().current() = pNtkRes;
            }

            end = clock();
            totalTime = (double)(end - begin) / CLOCKS_PER_SEC;

            cout.setf(ios::fixed);
            cout << "[CPU time]   " << setprecision(2) << totalTime << " s" << endl;
        }

    private:
        int fBalance = 0;
        int fVerbose = 0;
        int fUpdateLevel = 0;
        int fFanout = 1;
        int fPower = 0;
    };

    ALICE_ADD_COMMAND(dc2, "ABC")

} // namespace alice

#endif
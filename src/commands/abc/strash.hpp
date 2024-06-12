/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file strash.hpp
 *
 * @brief  transforms combinational logic into an AIG
 *
 * @author Jiaxiang Pan
 * @since  2024/06/12
 */

#ifndef STRASH_HPP
#define STRASH_HPP

#include "base/abc/abc.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{
    class strash_command : public command
    {
    public:
        explicit strash_command(const environment::ptr &env) : command(env, "transforms combinational logic into an AIG")
        {
            add_flag("--allnodes, -a", "toggles between using all nodes and DFS nodes [default = DFS]");
            add_flag("--cleanup, -c", "toggles cleanup to remove the dagling AIG nodes [default = all]");
            add_flag("--record, -r", "toggles using the record of AIG subgraphs [default = no]");
            add_flag("--complouts, -i", "toggles complementing the POs of the AIG [default = no]");
        }

    protected:
        void execute() override
        {
            clock_t begin, end;
            double totalTime;

            if (is_set("allnodes"))
                fAllNodes ^= 1;
            if (is_set("cleanup"))
                fCleanup ^= 1;
            if (is_set("record"))
                fRecord ^= 1;
            if (is_set("complouts"))
                fComplOuts ^= 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty AIG network.\n";
            else
            {
                pabc::Abc_Ntk_t *pNtk, *pNtkRes;
                Abc_Obj_t * pObj;
                int c;
                pNtk = store<pabc::Abc_Ntk_t *>().current();

                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (fComplOuts)
                Abc_NtkForEachPo(pNtkRes, pObj, c)
                    Abc_ObjXorFaninC(pObj, 0);

                store<pabc::Abc_Ntk_t *>().extend();
                store<pabc::Abc_Ntk_t *>().current() = pNtk;
            }

            end = clock();
            totalTime = (double)(end - begin) / CLOCKS_PER_SEC;

            cout.setf(ios::fixed);
            cout << "[CPU time]   " << setprecision(2) << totalTime << " s" << endl;
        }

    private:
        int fAllNodes = 0;
        int fRecord = 0;
        int fCleanup = 1;
        int fComplOuts = 0;
    };

    ALICE_ADD_COMMAND(strash, "ABC")

} // namespace alice

#endif
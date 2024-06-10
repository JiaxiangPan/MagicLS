/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file balance.hpp
 *
 * @brief transforms the current network into a well-balanced AIG
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef ABC_BALANCE_HPP
#define ABC_BALANCE_HPP

#include "base/abc/abc.h"
#include "base/abci/abcBalance.c"
#include "base/abci/abcDar.c"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_balance_command : public command
    {
    public:
        explicit abc_balance_command(const environment::ptr &env) : command(env, "transforms the current network into a well-balanced AIG")
        {
            add_flag("--updateLevel, -l", "toggle minimizing the number of levels [default = yes]");
            add_flag("--duplicate, -d", "toggle duplication of logic [default = no]");
            add_flag("--selective, -s", "toggle duplication on the critical paths [default = no]");
            add_flag("--exor, -e", "toggle balancing multi-input EXORs [default = no]");
            add_flag("--verbose, -v", "print verbose information [default = no]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;

            if (is_set("updateLevel"))
                fUpdateLevel ^= 1;
            if (is_set("duplicate"))
                fDuplicate ^= 1;
            if (is_set("selective"))
                fSelective ^= 1;
            if (is_set("exor"))
                fExor ^= 1;
            if (is_set("verbose"))
                fVerbose ^= 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty ABC AIG network\n";
            else
            {
                Abc_Ntk_t *pNtk, *pNtkRes, *pNtkTemp;
                pNtk = store<pabc::Abc_Ntk_t *>().current();
                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                // get the new network
                if (Abc_NtkIsStrash(pNtk))
                {
                    if (fExor)
                        pNtkRes = Abc_NtkBalanceExor(pNtk, fUpdateLevel, fVerbose);
                    else
                        pNtkRes = Abc_NtkBalance(pNtk, fDuplicate, fSelective, fUpdateLevel);
                }
                else
                {
                    pNtkTemp = Abc_NtkStrash(pNtk, 0, 0, 0);
                    if (pNtkTemp == NULL)
                    {
                        Abc_Print(-1, "Strashing before balancing has failed.\n");
                        return;
                    }
                    if (fExor)
                        pNtkRes = Abc_NtkBalanceExor(pNtkTemp, fUpdateLevel, fVerbose);
                    else
                        pNtkRes =
                            Abc_NtkBalance(pNtkTemp, fDuplicate, fSelective, fUpdateLevel);
                    Abc_NtkDelete(pNtkTemp);
                }
                // check if balancing worked
                if (pNtkRes == NULL)
                {
                    Abc_Print(-1, "Balancing has failed.\n");
                    return;
                }
                // replace the current network
                // Abc_FrameReplaceCurrentNetwork(pAbc, pNtkRes);

                store<pabc::Abc_Ntk_t *>().extend();
                store<pabc::Abc_Ntk_t *>().current() = pNtkRes;
            }

            end = clock();
            totalTime = (double)(end - begin) / CLOCKS_PER_SEC;

            cout.setf(ios::fixed);
            cout << "[CPU time]   " << setprecision(2) << totalTime << " s" << endl;
        }

    private:
        int fDuplicate = 0;
        int fSelective = 0;
        int fUpdateLevel = 1;
        int fExor = 0;
        int fVerbose = 0;
    };
    ALICE_ADD_COMMAND(abc_balance, "ABC")

} // namespace alice

#endif
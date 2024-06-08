/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file rewrite.hpp
 *
 * @brief performs technology-independent rewriting of the AIG
 *
 * @author Jiaxiang Pan
 * @since  2024/06/08
 */

#ifndef ABC_REWRITE_HPP
#define ABC_REWRITE_HPP

#include "base/abc/abc.h"
#include "base/main/main.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_rewrite_command : public command
    {
    public:
        explicit abc_rewrite_command(const environment::ptr &env) : command(env, "performs technology-independent rewriting of the AIG")
        {
            add_flag("--updateLevel, -l", "toggle preserving the number of levels [default = yes]");
            add_flag("--usezeros, -z", "toggle using zero-cost replacements [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
            add_flag("--veryverbose, -V", "toggle printout subgraph statistics [default = no]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;

            if (is_set("updateLevel"))
                fUpdateLevel ^= 1;
            if (is_set("usezeros"))
                fUseZeros ^= 1;
            if (is_set("verbose"))
                fVerbose ^= 1;
            if (is_set("veryverbose"))
                fVeryVerbose ^= 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty ABC AIG network\n";
            else
            {
                Abc_Ntk_t *pNtk, *pDup;
                pNtk = store<pabc::Abc_Ntk_t *>().current();

                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (!Abc_NtkIsStrash(pNtk))
                {
                    Abc_Print(-1, "This command can only be applied to an AIG (run \"strash\").\n");
                    return;
                }
                if (Abc_NtkGetChoiceNum(pNtk))
                {
                    Abc_Print(-1, "AIG resynthesis cannot be applied to AIGs with choice nodes.\n");
                    return;
                }

                pDup = Abc_NtkDup(pNtk);
                int RetValue = Abc_NtkRewrite(pNtk, fUpdateLevel, fUseZeros, fVerbose, fVeryVerbose, fPlaceEnable);
                if (RetValue == -1)
                {
                    // TO DO
                    //  Abc_FrameReplaceCurrentNetwork(pAbc, pDup);
                    printf("An error occurred during computation. The original network is restored.\n");
                }
                else
                {
                    Abc_NtkDelete(pDup);
                    if (RetValue == 0)
                    {
                        std::cerr << "Rewriting has failed.\n";
                        return;
                    }
                }
                store<pabc::Abc_Ntk_t *>().extend();
                store<pabc::Abc_Ntk_t *>().current() = pNtk;
            }

            end = clock();
            totalTime = (double)(end - begin) / CLOCKS_PER_SEC;

            cout.setf(ios::fixed);
            cout << "[CPU time]   " << setprecision(2) << totalTime << " s" << endl;
        }

    private:
        int fUpdateLevel = 1;
        int fUseZeros = 0;
        int fVerbose = 0;
        int fVeryVerbose = 0;
        int fPlaceEnable = 0;
    };

    ALICE_ADD_COMMAND(abc_rewrite, "ABC")

} // namespace alice

#endif
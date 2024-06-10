/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file refactor.hpp
 *
 * @brief performs technology-independent refactoring of the AIG
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef ABC_REFACTOR_HPP
#define ABC_REFACTOR_HPP

#include "base/abc/abc.h"
#include "base/abci/abc.c"
#include "base/abci/abcRefactor.c"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_refactor_command : public command
    {
    public:
        explicit abc_refactor_command(const environment::ptr &env) : command(env, "performs technology-independent refactoring of the AIG")
        {
            add_option("-N, --nodesizemax", nNodeSizeMax, "set the max support of the collapsed node [default = 10]");
            add_option("-M, --minsaved", nMinSaved, "the min number of nodes saved after one step (0 <= num) [default = 1]");
            add_option("-C, --conesizemax", nConeSizeMax, "the max support of the containing cone [default = 16]");
            add_flag("--updateLevel, -l", "toggle preserving the number of levels [default = yes]");
            add_flag("--usezeros, -z", "toggle using zero-cost replacements [default = no]");
            add_flag("--usedcs, -d", "toggle using don't-cares [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
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
            if (is_set("usedcs"))
                fUseDcs ^= 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty AIG network.\n";
            else
            {
                pabc::Abc_Ntk_t *pNtk, *pDup;
                pNtk = store<pabc::Abc_Ntk_t *>().current();
                int c, RetValue;

                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (!Abc_NtkIsStrash(pNtk))
                {
                    Abc_Print(
                        -1,
                        "This command can only be applied to an AIG (run \"strash\").\n");
                    return;
                }
                if (Abc_NtkGetChoiceNum(pNtk))
                {
                    Abc_Print(
                        -1,
                        "AIG resynthesis cannot be applied to AIGs with choice nodes.\n");
                    return;
                }
                if (nNodeSizeMax > 15)
                {
                    Abc_Print(-1, "The cone size cannot exceed 15.\n");
                    return;
                }

                if (fUseDcs && nNodeSizeMax >= nConeSizeMax)
                {
                    Abc_Print(-1,
                              "For don't-care to work, containing cone should be larger "
                              "than collapsed node.\n");
                    return;
                }

                // modify the current network
                pDup = Abc_NtkDup(pNtk);
                RetValue = Abc_NtkRefactor(pNtk, nNodeSizeMax, nMinSaved, nConeSizeMax,
                                           fUpdateLevel, fUseZeros, fUseDcs, fVerbose);
                if (RetValue == -1)
                {
                    // Abc_FrameReplaceCurrentNetwork(pAbc, pDup);
                    printf(
                        "An error occurred during computation. The original network is "
                        "restored.\n");
                }
                else
                {
                    Abc_NtkDelete(pDup);
                    if (RetValue == 0)
                    {
                        Abc_Print(0, "Refactoring has failed.\n");
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
        int nNodeSizeMax = 10;
        int nMinSaved = 1;
        int nConeSizeMax = 16;
        int fUpdateLevel = 1;
        int fUseZeros = 0;
        int fUseDcs = 0;
        int fVerbose = 0;
    };

    ALICE_ADD_COMMAND(abc_refactor, "ABC")

} // namespace alice

#endif
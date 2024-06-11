/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file rewrite.hpp
 *
 * @brief performs technology-independent restructuring of the AIG
 *
 * @author Jiaxiang Pan
 * @since  2024/06/11
 */

#ifndef ABC_RESUB_HPP
#define ABC_RESUB_HPP

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/abci/abcResub.c"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_resub_command : public command
    {
    public:
        explicit abc_resub_command(const environment::ptr &env) : command(env, "performs technology-independent restructuring of the AIG")
        {
            add_option("-K, --cutsmax", nCutsMax, "the max cut size (4 <= num <= 16) [default = 8]");
            add_option("-N, --nodesmax", nNodesMax, "the max number of nodes to add (0 <= num <= 3) [default = 1]");
            add_option("-M, --minsaved", nMinSaved, "the min number of nodes saved after one step (0 <= num) [default = 1]");
            add_option("-F, --levelsodc", nLevelsOdc, "the number of fanout levels for ODC computation [default = 0]");
            add_flag("--updateLevel, -l", "toggle preserving the number of levels [default = yes]");
            add_flag("--usezeros, -z", "toggle using zero-cost replacements [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
            add_flag("--veryverbose, -w", "toggle verbose printout of ODC computation [default = no]");
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

            if (fUseZeros)
                nMinSaved = 0;
            if (nMinSaved == 0)
                fUseZeros = 1;

            begin = clock();

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty ABC AIG network\n";
            else
            {
                Abc_Ntk_t *pNtk;
                pNtk = store<pabc::Abc_Ntk_t *>().current();
                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (nCutsMax < RS_CUT_MIN || nCutsMax > RS_CUT_MAX)
                {
                    Abc_Print(-1, "Can only compute cuts for %d <= K <= %d.\n", RS_CUT_MIN, RS_CUT_MAX);
                    return;
                }
                if (nNodesMax < 0 || nNodesMax > 3)
                {
                    Abc_Print(-1, "Can only resubstitute at most 3 nodes.\n");
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

                // modify the current network
                if (!Abc_NtkResubstitute(pNtk, nCutsMax, nNodesMax, nMinSaved, nLevelsOdc, fUpdateLevel, fVerbose, fVeryVerbose))
                {
                    Abc_Print(-1, "Refactoring has failed.\n");
                    return;
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
        int RS_CUT_MIN = 4;
        int RS_CUT_MAX = 16;
        int nCutsMax = 8;
        int nNodesMax = 1;
        int nLevelsOdc = 0;
        int nMinSaved = 1;
        int fUpdateLevel = 1;
        int fUseZeros = 0;
        int fVerbose = 0;
        int fVeryVerbose = 0;
    };

    ALICE_ADD_COMMAND(abc_resub, "ABC")

} // namespace alice

#endif
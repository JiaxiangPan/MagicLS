/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file ifraig.hpp
 *
 * @brief performs fraiging using a new method
 *
 * @author Jiaxiang Pan
 * @since  2024/06/11
 */

#ifndef IFRAIG_HPP
#define IFRAIG_HPP

#include "base/abc/abc.h"
// #include "base/abci/abcDar.c"
#include "base/abci/abcIvy.c"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class ifraig_command : public command
    {
    public:
        explicit ifraig_command(const environment::ptr &env) : command(env, "performs fraiging using a new method")
        {
            add_option("-P, --partsize", nPartSize, "partition size (0 = partitioning is not used) [default = 0]");
            add_option("-C, --conflimit", nConfLimit, "limit on the number of conflicts [default = 100]");
            add_option("-F, --levelmax", nLevelMax, "limit on node level to fraig (0 = fraig all nodes) [default = 0]");
            add_flag("--dosparse, -s", "toggle considering sparse functions [default = yes]");
            add_flag("--prove, -p", "toggle proving the miter outputs [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
        }

    protected:
        void execute()
        {
            // extern Abc_Ntk_t * Abc_NtkIvyFraig( Abc_Ntk_t * pNtk, int nConfLimit, int fDoSparse, int fProve, int fTransfer, int fVerbose );
            // extern Abc_Ntk_t * Abc_NtkDarFraigPart( Abc_Ntk_t * pNtk, int nPartSize, int nConfLimit, int nLevelMax, int fVerbose );
            clock_t begin, end;
            double totalTime;

            if (is_set("dosparse"))
                fDoSparse ^= 1;
            if (is_set("prove"))
                fProve ^= 1;
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

                if (nPartSize > 0)
                    pNtkRes = Abc_NtkDarFraigPart(pNtk, nPartSize, nConfLimit, nLevelMax, fVerbose);
                else
                    pNtkRes = Abc_NtkIvyFraig(pNtk, nConfLimit, fDoSparse, fProve, 0, fVerbose);
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
        int nPartSize = 0;
        int nLevelMax = 0;
        int nConfLimit = 100;
        int fDoSparse = 1;
        int fProve = 0;
        int fVerbose = 0;
    };

    ALICE_ADD_COMMAND(ifraig, "ABC")

} // namespace alice

#endif
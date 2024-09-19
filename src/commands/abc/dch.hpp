/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file dch.hpp
 *
 * @brief computes structural choices using a new approach
 *
 * @author Jiaxiang Pan
 * @since  2024/06/12
 */

#ifndef DCH_HPP
#define DCH_HPP

#include "base/abc/abc.h"
// #include "base/main/main.h"
// #include "proof/dch/dch.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class dch_command : public command
    {
    public:
        explicit dch_command(const environment::ptr &env) : command(env, "computes structural choices using a new approach")
        {
            add_option("-W, --words", pPars->nWords, "the max number of simulation words [default = 8]");
            add_option("-C, --btlimit", pPars->nBTLimit, "the max number of conflicts at a node [default = 1000]");
            add_option("-S, --satvarmax", pPars->nSatVarMax, "the max number of SAT variables [default = 5000]");
            add_flag("--synthesis, -s", "toggle synthesizing three snapshots [default = yes]");
            add_flag("--power, -p", "toggle power-aware rewriting [default = no]");
            add_flag("--simulatetfo, -t", "toggle simulation of the TFO classes [default = yes]");
            add_flag("--usegia, -g", "toggle using GIA to prove equivalences [default = no]");
            add_flag("--usecsat, -c", "toggle using circuit-based SAT vs. MiniSat [default = no]");
            add_flag("--lightsynth, -f", "toggle using faster logic synthesis [default = no]");
            add_flag("--skipredsupp, -r", "toggle skipping choices with redundant support [default = no]");
            add_flag("--usenew, -x", "toggle using new choice computation [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;

            // set defaults
            Dch_ManSetDefaultParams(pPars);

            if (is_set("synthesis"))
                pPars->fSynthesis ^= 1;
            if (is_set("power"))
                pPars->fPower ^= 1;
            if (is_set("simulatetfo"))
                pPars->fSimulateTfo ^= 1;
            if (is_set("usegia"))
                pPars->fUseGia ^= 1;
            if (is_set("usecsat"))
                pPars->fUseCSat ^= 1;
            if (is_set("lightsynth"))
                pPars->fLightSynth ^= 1;
            if (is_set("skipredsupp"))
                pPars->fSkipRedSupp ^= 1;
            if (is_set("usenew"))
                pPars->fUseNew ^= 1;
            if (is_set("verbose"))
                pPars->fVerbose ^= 1;

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
                pNtkRes = Abc_NtkDch(pNtk, pPars);
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
        Dch_Pars_t Pars, *pPars = &Pars;
    };

    ALICE_ADD_COMMAND(dch, "ABC")

} // namespace alice

#endif
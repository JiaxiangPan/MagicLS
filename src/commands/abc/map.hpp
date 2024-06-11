/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file map.hpp
 *
 * @brief  performs standard cell mapping of the current network
 *
 * @author Jiaxiang Pan
 * @since  2024/06/11
 */

#ifndef ABC_MAP_HPP
#define ABC_MAP_HPP

#define ABC_INFINITY (1000000000)

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/abci/abcMap.c"
#include "base/abci/abcSweep.c"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_map_command : public command
    {
    public:
        explicit abc_map_command(const environment::ptr &env) : command(env, "performs standard cell mapping of the current network")
        {
            add_option("-D, --delaytarget", DelayTarget, "sets the global required times [default = not used]]");
            add_option("-A, --areamulti", AreaMulti, "\"area multiplier\" to bias gate selection [default = 0.00]");
            add_option("-B, --delaymulti", DelayMulti, "\"delay multiplier\" to bias gate selection [default = 0.00]");
            add_option("-F, --logfan", LogFan, "the logarithmic fanout delay parameter [default = 0.00]");
            add_option("-S, --slew", Slew, "the slew parameter used to generate the library [default = 0.00]");
            add_option("-G, --gain", Gain, "the gain parameter used to generate the library [default = 250.00]");
            add_option("-M, --gatesmin", nGatesMin, "skip gate classes whose size is less than this [default = 0]");
            add_flag("--areaonly, -a", "toggles area-only mapping [default = no]");
            add_flag("--recovery, -r", "toggles area recovery [default = yes]");
            add_flag("--sweep, -s", "toggles sweep after mapping [default = no]");
            add_flag("--switching, -p", "optimizes power by minimizing switching [default = no]");
            add_flag("--skipfanout, -f", "do not use large gates to map high-fanout nodes [default = no]");
            add_flag("--useprofile, -u", "use standard-cell profile [default = no]");
            add_flag("--usebuffs, -o", "toggles using buffers to decouple combinational outputs [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = no]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;

            if (is_set("areaonly"))
                fAreaOnly ^= 1;
            if (is_set("recovery"))
                fRecovery ^= 1;
            if (is_set("sweep"))
                fSweep ^= 1;
            if (is_set("switching"))
                fSwitching ^= 1;
            if (is_set("skipfanout"))
                fSkipFanout ^= 1;
            if (is_set("useprofile"))
                fUseProfile ^= 1;
            if (is_set("usebuffs"))
                fUseBuffs ^= 1;
            if (is_set("verbose"))
                fVerbose ^= 1;

            begin = clock();

            pabc::Abc_Ntk_t *pNtkRes;

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty ABC AIG network\n";
            else
            {
                pabc::Abc_Ntk_t *pNtk = store<pabc::Abc_Ntk_t *>().current();
                if (pNtk == NULL)
                {
                    Abc_Print(-1, "Empty network.\n");
                    return;
                }
                if (fAreaOnly)
                    DelayTarget = ABC_INFINITY;
                if (!Abc_NtkIsStrash(pNtk))
                {
                    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
                    if (pNtk == NULL)
                    {
                        Abc_Print(-1, "Strashing before mapping has failed.\n");
                        return;
                    }
                    pNtk = Abc_NtkBalance(pNtkRes = pNtk, 0, 0, 1);
                    Abc_NtkDelete(pNtkRes);
                    if (pNtk == NULL)
                    {
                        Abc_Print(-1, "Balancing before mapping has failed.\n");
                        return;
                    }
                    Abc_Print(0, "The network was strashed and balanced before mapping.\n");
                    // get the new network
                    pNtkRes = Abc_NtkMap(pNtk, DelayTarget, AreaMulti, DelayMulti, LogFan, Slew, Gain, nGatesMin, fRecovery, fSwitching, fSkipFanout, fUseProfile, fUseBuffs, fVerbose);
                    if (pNtkRes == NULL)
                    {
                        Abc_NtkDelete(pNtk);
                        Abc_Print(-1, "Mapping has failed.\n");
                        return;
                    }
                    Abc_NtkDelete(pNtk);
                }
                else
                {
                    // get the new network
                    pNtkRes = Abc_NtkMap(pNtk, DelayTarget, AreaMulti, DelayMulti, LogFan, Slew, Gain, nGatesMin, fRecovery, fSwitching, fSkipFanout, fUseProfile, fUseBuffs, fVerbose);
                    if (pNtkRes == NULL)
                    {
                        Abc_Print(-1, "Mapping has failed.\n");
                        return;
                    }
                }
                if (fSweep)
                {
                    Abc_NtkFraigSweep(pNtkRes, 0, 0, 0, 0);
                    if (Abc_NtkHasMapping(pNtkRes))
                    {
                        pNtkRes = Abc_NtkDupDfs(pNtk = pNtkRes);
                        Abc_NtkDelete(pNtk);
                    }
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
        double DelayTarget = -1;
        double AreaMulti = 0;
        double DelayMulti = 0;
        float LogFan = 0;
        float Slew = 0; // choose based on the library
        float Gain = 250;
        int nGatesMin = 0;
        int fAreaOnly = 0;
        int fRecovery = 1;
        int fSweep = 0;
        int fSwitching = 0;
        int fSkipFanout = 0;
        int fUseProfile = 0;
        int fUseBuffs = 0;
        int fVerbose = 0;
    };

    ALICE_ADD_COMMAND(abc_map, "ABC")

} // namespace alice

#endif
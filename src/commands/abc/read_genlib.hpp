/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file read_genlib.hpp
 *
 * @brief  read the library from a genlib file
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef READ_GENLIB_HPP
#define READ_GENLIB_HPP

#include <string>

#include "base/abc/abc.h"
#include "base/io/ioAbc.h"
#include "base/main/abcapis.h"
#include "base/main/main.h"
#include "map/amap/amap.h"
#include "map/mapper/mapper.h"
#include "map/mio/mio.h"
#include "misc/extra/extra.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class abc_read_genlib_command : public command
    {
    public:
        explicit abc_read_genlib_command(const environment::ptr &env)
            : command(env, "read the library from a genlib file")
        {
            add_option("filename, -f", file_name, "name of input file");
            add_option("wiredelay, -W", WireDelay, "wire delay (added to pin-to-pin gate delays) [default = 0]");
            add_flag("--shortnames, -n", "toggle replacing gate/pin names by short strings [default = no]");
            add_flag("--verbose, -v", "toggle verbose printout [default = yes]");
        }

    protected:
        void execute()
        {
            clock_t begin, end;
            double totalTime;
            begin = clock();

            if (is_set("verbose"))
                fVerbose ^= 1;
            if (is_set("shortnames"))
                fShortNames ^= 1;

            pabc::Abc_Frame_t *pAbc;
            pAbc = pabc::Abc_FrameGetGlobalFrame();

            // set defaults
            FILE *pFile;
            pabc::Mio_Library_t *pLib;
            pabc::Amap_Lib_t *pLib2;
            char *pFileName;
            char *pExcludeFile = NULL;

            // get the input file name
            pFileName = (char *)malloc(file_name.length() + 1);
            if (pFileName != NULL)
            {
                strcpy(pFileName, file_name.c_str());
            }
            else
            {
                printf("Memory allocation failed\n");
            }

            // set the new network
            pLib = pabc::Mio_LibraryRead(pFileName, NULL, pExcludeFile, fVerbose);
            if (pLib == NULL)
            {
                printf("Reading genlib library has failed.\n");
                return;
            }
            if (fVerbose)
                printf("Entered genlib library with %d gates from file \"%s\".\n", Mio_LibraryReadGateNum(pLib), pFileName);

            // prepare libraries
            pabc::Mio_UpdateGenlib(pLib);

            // replace the current library
            pLib2 = pabc::Amap_LibReadAndPrepare(pFileName, NULL, 0, 0);
            if (pLib2 == NULL)
            {
                printf("Reading second genlib library has failed.\n");
                return;
            }
            Abc_FrameSetLibGen2(pLib2);
        }

    private:
        std::string file_name;
        double WireDelay = 0.0;
        int fShortNames = 0;
        int fVerbose = 1;
    };

    ALICE_ADD_COMMAND(abc_read_genlib, "I/O")

} // namespace alice

#endif
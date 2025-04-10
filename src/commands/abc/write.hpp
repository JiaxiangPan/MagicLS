/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file write.hpp
 *
 * @brief writes the current network into <file> by calling the writer that matches the extension of <file>
 *
 * @author Jiaxiang Pan
 * @since  2024/06/12
 */

#ifndef WRITE_HPP
#define WRITE_HPP

#include "base/abc/abc.h"

using namespace std;
using namespace mockturtle;
using namespace pabc;

namespace alice
{

    class write_command : public command
    {
    public:
        explicit write_command(const environment::ptr &env)
            : command(env, "writes the current network into file by ABC parser")
        {
            add_option("filename,-f", file_name, "name of output file");
        }

    protected:
        void execute() override
        {
            assert(strlen((char *)(file_name.c_str())) < 900);

            if (store<pabc::Abc_Ntk_t *>().size() == 0u)
                std::cerr << "Error: Empty network.\n";
            else
            {
                auto pNtk = store<pabc::Abc_Ntk_t *>().current();
                pabc::Io_Write(pNtk, (char *)(file_name.c_str()),
                               pabc::Io_ReadFileType((char *)(file_name.c_str())));
            }
        }

    private:
        std::string file_name;
    };

    ALICE_ADD_COMMAND(write, "I/O")

} // namespace alice

#endif
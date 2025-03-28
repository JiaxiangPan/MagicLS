/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "store.hpp"
#include "commands/abc/rewrite.hpp"
#include "commands/abc/read.hpp"
#include "commands/adder.hpp"
#include "commands/subtractor.hpp"
#include "commands/multiplier.hpp"
#include "commands/divider.hpp"
#include "commands/abc/balance.hpp"
#include "commands/exprsim.hpp"
#include "commands/abc/refactor.hpp"
#include "commands/abc/resub.hpp"
#include "commands/abc/map.hpp"
#include "commands/abc/dc2.hpp"
#include "commands/abc/ifraig.hpp"
#include "commands/abc/dch.hpp"
#include "commands/abc/write.hpp"
#include "commands/abc/read_genlib.hpp"
#include "commands/abc/strash.hpp"
#include "commands/transform.hpp"
#include "commands/abc/&fraig.hpp"
#include "commands/abc/gia_opt.hpp"

ALICE_MAIN(MagicLS)

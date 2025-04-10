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

#ifndef STORE_HPP
#define STORE_HPP

#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <base/abc/abc.h>
#include <base/io/ioAbc.h>
#include <fmt/format.h>

#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <lorina/diagnostics.hpp>
#include <lorina/genlib.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/io/write_aiger.hpp>
#include <mockturtle/io/write_blif.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/mockturtle.hpp>
#include <mockturtle/views/names_view.hpp>

#include "./core/abc2mockturtle.hpp"
#include "./core/abc_api.hpp"
#include "./core/abc_gia.hpp"
#include "./core/abc.hpp"

#include <mockturtle/algorithms/node_resynthesis.hpp>

using namespace mockturtle;

namespace alice {

/********************************************************************
 * Genral stores                                                    *
 ********************************************************************/

/* aiger */
ALICE_ADD_STORE(aig_network, "aig", "a", "AIG", "AIGs")

ALICE_PRINT_STORE(aig_network, os, element) {
  os << "AIG PI/PO = " << element.num_pis() << "/" << element.num_pos() << "\n";
}

ALICE_DESCRIBE_STORE(aig_network, element) {
  return fmt::format("{} nodes", element.size());
}

/* mig */
ALICE_ADD_STORE(mig_network, "mig", "m", "MIG", "MIGs")

ALICE_PRINT_STORE(mig_network, os, element) {
  os << "MIG PI/PO = " << element.num_pis() << "/" << element.num_pos() << "\n";
}

ALICE_DESCRIBE_STORE(mig_network, element) {
  return fmt::format("{} nodes", element.size());
}

/* xmg */
ALICE_ADD_STORE(xmg_network, "xmg", "x", "xmg", "xmgs")

ALICE_PRINT_STORE(xmg_network, os, element) {
  os << fmt::format(" xmg i/o = {}/{} gates = {} ", element.num_pis(),
                    element.num_pos(), element.num_gates());
  os << "\n";
}

ALICE_DESCRIBE_STORE(xmg_network, element) {
  return fmt::format("{} nodes", element.size());
}

/* xag */
ALICE_ADD_STORE(xag_network, "xag", "g", "xag", "xags")

ALICE_PRINT_STORE(xag_network, os, element) {
  os << fmt::format(" xag i/o = {}/{} gates = {} ", element.num_pis(),
                    element.num_pos(), element.num_gates());
  os << "\n";
}

ALICE_DESCRIBE_STORE(xag_network, element) {
  return fmt::format("{} nodes", element.size());
}

/*klut network*/
ALICE_ADD_STORE(klut_network, "lut", "l", "LUT network", "LUT networks")

ALICE_PRINT_STORE(klut_network, os, element) {
  os << fmt::format(" klut i/o = {}/{} gates = {} ", element.num_pis(),
                    element.num_pos(), element.num_gates());
  os << "\n";
}

ALICE_DESCRIBE_STORE(klut_network, element) {
  return fmt::format("{} nodes", element.size());
}

ALICE_PRINT_STORE_STATISTICS(klut_network, os, lut) {
  mockturtle::depth_view depth_lut{lut};
  os << fmt::format("LUTs   i/o = {}/{}   gates = {}   level = {}",
                    lut.num_pis(), lut.num_pos(), lut.num_gates(),
                    depth_lut.depth());
  os << "\n";
}

/* opt_network */
class optimum_network {
 public:
  optimum_network() = default;

  optimum_network(const kitty::dynamic_truth_table &function)
      : function(function) {}

  optimum_network(kitty::dynamic_truth_table &&function)
      : function(std::move(function)) {}

  bool exists() const {
    static std::vector<std::unordered_set<
        kitty::dynamic_truth_table, kitty::hash<kitty::dynamic_truth_table>>>
        hash;

    if (function.num_vars() >= hash.size()) {
      hash.resize(function.num_vars() + 1);
    }

    return !hash[function.num_vars()].insert(function).second;
  }

 public: /* field access */
  kitty::dynamic_truth_table function{0};
  std::string network;
};

ALICE_ADD_STORE(optimum_network, "opt", "o", "network", "networks")

ALICE_DESCRIBE_STORE(optimum_network, opt) {
  if (opt.network.empty()) {
    return fmt::format("{}", kitty::to_hex(opt.function));
  } else {
    return fmt::format("{}, optimum network computed",
                       kitty::to_hex(opt.function));
  }
}

ALICE_PRINT_STORE(optimum_network, os, opt) {
  os << fmt::format("function (hex): {}\nfunction (bin): {}\n",
                    kitty::to_hex(opt.function),
                    kitty::to_binary(opt.function));

  if (opt.network.empty()) {
    os << "no optimum network computed\n";
  } else {
    os << fmt::format("optimum network: {}\n", opt.network);
  }
}

/* genlib */
ALICE_ADD_STORE(std::vector<mockturtle::gate>, "genlib", "f", "GENLIB",
                "GENLIBs")

ALICE_PRINT_STORE(std::vector<mockturtle::gate>, os, element) {
  os << "GENLIB gate size = " << element.size() << "\n";
}

ALICE_DESCRIBE_STORE(std::vector<mockturtle::gate>, element) {
  return fmt::format("{} gates", element.size());
}

ALICE_ADD_FILE_TYPE(genlib, "Genlib");

ALICE_READ_FILE(std::vector<mockturtle::gate>, genlib, filename, cmd) {
  std::vector<mockturtle::gate> gates;
  if (lorina::read_genlib(filename, mockturtle::genlib_reader(gates)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return gates;
}

ALICE_WRITE_FILE(std::vector<mockturtle::gate>, genlib, gates, filename, cmd) {
  std::cout << "[e] not supported" << std::endl;
}

ALICE_PRINT_STORE_STATISTICS(std::vector<mockturtle::gate>, os, gates) {
  os << fmt::format("Entered genlib library with {} gates", gates.size());
  os << "\n";
}

/********************************************************************
 * Read and Write                                                   *
 ********************************************************************/
ALICE_ADD_FILE_TYPE(aiger, "Aiger");

ALICE_READ_FILE(aig_network, aiger, filename, cmd) {
  aig_network aig;
  if (lorina::read_aiger(filename, mockturtle::aiger_reader(aig)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return aig;
}

ALICE_PRINT_STORE_STATISTICS(aig_network, os, aig) {
  auto aig_copy = mockturtle::cleanup_dangling(aig);
  mockturtle::depth_view depth_aig{aig_copy};
  os << fmt::format("AIG   i/o = {}/{}   gates = {}   level = {}",
                    aig.num_pis(), aig.num_pos(), aig.num_gates(),
                    depth_aig.depth());
  os << "\n";
}

ALICE_ADD_FILE_TYPE(verilog, "Verilog");

ALICE_READ_FILE(aig_network, verilog, filename, cmd) {
  aig_network aig;

  if (lorina::read_verilog(filename, mockturtle::verilog_reader(aig)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return aig;
}

ALICE_READ_FILE(xmg_network, verilog, filename, cmd) {
  xmg_network xmg;

  if (lorina::read_verilog(filename, mockturtle::verilog_reader(xmg)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return xmg;
}

ALICE_WRITE_FILE(xmg_network, verilog, xmg, filename, cmd) {
  mockturtle::write_verilog(xmg, filename);
}

ALICE_WRITE_FILE(aig_network, verilog, aig, filename, cmd) {
  mockturtle::write_verilog(aig, filename);
}

ALICE_PRINT_STORE_STATISTICS(xmg_network, os, xmg) {
  auto xmg_copy = mockturtle::cleanup_dangling(xmg);
  mockturtle::depth_view depth_xmg{xmg_copy};
  os << fmt::format("XMG   i/o = {}/{}   gates = {}   level = {}",
                    xmg.num_pis(), xmg.num_pos(), xmg.num_gates(),
                    depth_xmg.depth());
  os << "\n";
}

ALICE_READ_FILE(mig_network, verilog, filename, cmd) {
  mig_network mig;
  if (lorina::read_verilog(filename, mockturtle::verilog_reader(mig)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return mig;
}

ALICE_WRITE_FILE(mig_network, verilog, mig, filename, cmd) {
  mockturtle::write_verilog(mig, filename);
}

ALICE_PRINT_STORE_STATISTICS(mig_network, os, mig) {
  auto mig_copy = mockturtle::cleanup_dangling(mig);
  mockturtle::depth_view depth_mig{mig_copy};
  os << fmt::format("MIG   i/o = {}/{}   gates = {}   level = {}",
                    mig.num_pis(), mig.num_pos(), mig.num_gates(),
                    depth_mig.depth());
  os << "\n";
}

ALICE_READ_FILE(xag_network, verilog, filename, cmd) {
  xag_network xag;
  if (lorina::read_verilog(filename, mockturtle::verilog_reader(xag)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }

  return xag;
}

ALICE_WRITE_FILE(xag_network, verilog, xag, filename, cmd) {
  mockturtle::write_verilog(xag, filename);
}

ALICE_PRINT_STORE_STATISTICS(xag_network, os, xag) {
  auto xag_copy = mockturtle::cleanup_dangling(xag);
  mockturtle::depth_view depth_xag{xag_copy};
  os << fmt::format("XAG   i/o = {}/{}   gates = {}   level = {}",
                    xag.num_pis(), xag.num_pos(), xag.num_gates(),
                    depth_xag.depth());
  os << "\n";
}

ALICE_ADD_FILE_TYPE(bench, "BENCH");

ALICE_READ_FILE(klut_network, bench, filename, cmd) {
  klut_network klut;
  if (lorina::read_bench(filename, mockturtle::bench_reader(klut)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }
  return klut;
}

ALICE_WRITE_FILE(xmg_network, bench, xmg, filename, cmd) {
  mockturtle::write_bench(xmg, filename);
}

ALICE_WRITE_FILE(mig_network, bench, mig, filename, cmd) {
  mockturtle::write_bench(mig, filename);
}

ALICE_WRITE_FILE(aig_network, bench, aig, filename, cmd) {
  mockturtle::write_bench(aig, filename);
}

ALICE_WRITE_FILE(xag_network, bench, xag, filename, cmd) {
  mockturtle::write_bench(xag, filename);
}

ALICE_WRITE_FILE(klut_network, bench, klut, filename, cmd) {
  mockturtle::write_bench(klut, filename);
}

ALICE_WRITE_FILE(aig_network, aiger, aig, filename, cmd) {
  mockturtle::write_aiger(aig, filename);
}

ALICE_ADD_FILE_TYPE(blif, "Blif");

ALICE_READ_FILE(klut_network, blif, filename, cmd) {
  klut_network klut;

  if (lorina::read_blif(filename, mockturtle::blif_reader(klut)) !=
      lorina::return_code::success) {
    std::cout << "[w] parse error\n";
  }

  return klut;
}

ALICE_WRITE_FILE(xmg_network, blif, xmg, filename, cmd) {
  mockturtle::write_blif(xmg, filename);
}

ALICE_WRITE_FILE(klut_network, blif, klut, filename, cmd) {
  mockturtle::write_blif(klut, filename);
}

/********************************************************************
 * Convert from aig to mig                                          *
 ********************************************************************/
ALICE_CONVERT(aig_network, element, mig_network) {
  aig_network aig = element;

  /* LUT mapping */
  mapping_view<aig_network, true> mapped_aig{aig};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<aig_network, true>, true>(mapped_aig, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_aig);

  /* node resynthesis */
  mig_npn_resynthesis resyn;
  auto mig = node_resynthesis<mig_network>(klut, resyn);

  return mig;
}

/********************************************************************
 * Convert from aig to xag                                          *
 ********************************************************************/
ALICE_CONVERT(aig_network, element, xag_network){
  aig_network aig = element;

  /* LUT mapping */
  mapping_view<aig_network, true> mapped_aig{aig};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<aig_network, true>, true>(mapped_aig, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_aig);

  /* node resynthesis */
  xag_npn_resynthesis<xag_network> resyn;
  auto xag = node_resynthesis<xag_network>(klut, resyn);

  return xag;
}

/********************************************************************
 * Convert from xmg to aig                                          *
 ********************************************************************/
ALICE_CONVERT(xmg_network, element, aig_network){
  xmg_network xmg = element;

  /* LUT mapping */
  mapping_view<xmg_network, true> mapped_xmg{xmg};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<xmg_network, true>, true>(mapped_xmg, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_xmg);

  /* node resynthesis */
  exact_resynthesis_params ps2;
  ps2.cache = std::make_shared<exact_resynthesis_params::cache_map_t>();
  exact_aig_resynthesis<aig_network> exact_resyn(false, ps2);
  node_resynthesis_stats nrst;
  dsd_resynthesis<aig_network, decltype(exact_resyn)> resyn(exact_resyn);
  //DSD decomposition may not be able to decompose the whole truth table, \
  a different fall-back resynthesis function must be passed to this function
  const auto aig = node_resynthesis<aig_network>(klut, resyn, {}, &nrst);

  return aig;
}

/* show */
template <>
bool can_show<aig_network>(std::string &extension, command &cmd) {
  extension = "dot";

  return true;
}

template <>
void show<aig_network>(std::ostream &os, const aig_network &element,
                       const command &cmd) {
  gate_dot_drawer<aig_network> drawer;
  write_dot(element, os, drawer);
}

template <>
bool can_show<mig_network>(std::string &extension, command &cmd) {
  extension = "dot";

  return true;
}

template <>
void show<mig_network>(std::ostream &os, const mig_network &element,
                       const command &cmd) {
  gate_dot_drawer<mig_network> drawer;
  write_dot(element, os, drawer);
}

template <>
bool can_show<xmg_network>(std::string &extension, command &cmd) {
  extension = "dot";

  return true;
}

template <>
void show<xmg_network>(std::ostream &os, const xmg_network &element,
                       const command &cmd) {
  gate_dot_drawer<xmg_network> drawer;
  write_dot(element, os, drawer);
}

template <>
bool can_show<klut_network>(std::string &extension, command &cmd) {
  extension = "dot";

  return true;
}

template <>
void show<klut_network>(std::ostream &os, const klut_network &element,
                        const command &cmd) {
  gate_dot_drawer<klut_network> drawer;
  write_dot(element, os, drawer);
}

template <>
bool can_show<xag_network>(std::string &extension, command &cmd) {
  extension = "dot";

  return true;
}

template <>
void show<xag_network>(std::ostream &os, const xag_network &element,
                       const command &cmd) {
  gate_dot_drawer<xag_network> drawer;
  write_dot(element, os, drawer);
}

/********************************************************************
 * Convert from aig to xmg                                          *
 ********************************************************************/
ALICE_CONVERT(aig_network, element, xmg_network) {
  aig_network aig = element;

  /* LUT mapping */
  mapping_view<aig_network, true> mapped_aig{aig};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<aig_network, true>, true>(mapped_aig, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_aig);

  /* node resynthesis */
  xmg_npn_resynthesis resyn;
  auto xmg = node_resynthesis<xmg_network>(klut, resyn);

  return xmg;
}

ALICE_CONVERT(mig_network, element, xmg_network) {
  mig_network mig = element;

  /* LUT mapping */
  mapping_view<mig_network, true> mapped_mig{mig};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<mig_network, true>, true>(mapped_mig, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_mig);

  /* node resynthesis */
  xmg_npn_resynthesis resyn;
  auto xmg = node_resynthesis<xmg_network>(klut, resyn);
  return xmg;
}

ALICE_CONVERT(xmg_network, element, mig_network) {
  xmg_network xmg = element;

  /* LUT mapping */
  mapping_view<xmg_network, true> mapped_xmg{xmg};
  lut_mapping_params ps;
  ps.cut_enumeration_ps.cut_size = 4;
  lut_mapping<mapping_view<xmg_network, true>, true>(mapped_xmg, ps);

  /* collapse into k-LUT network */
  const auto klut = *collapse_mapped_network<klut_network>(mapped_xmg);

  /* node resynthesis */
  mig_npn_resynthesis resyn;
  auto mig = node_resynthesis<mig_network>(klut, resyn);
  return mig;
}

/* ABC aiger */
ALICE_ADD_STORE(pabc::Abc_Ntk_t *, "abc", "b", "ABC", "ABCs")

ALICE_DESCRIBE_STORE(pabc::Abc_Ntk_t *, abc) {
  const auto name = pabc::Abc_NtkName(abc);
  const auto pi_num = pabc::Abc_NtkPiNum(abc);
  const auto po_num = pabc::Abc_NtkPoNum(abc);
  return fmt::format("{}   i/o = {}/{}", name, pi_num, po_num);
}

ALICE_PRINT_STORE(pabc::Abc_Ntk_t *, os, abc) {
  os << "AIG PI/PO = " << pabc::Abc_NtkPiNum(abc) << "/"
     << pabc::Abc_NtkPoNum(abc) << "\n";
}

ALICE_PRINT_STORE_STATISTICS(pabc::Abc_Ntk_t *, os, abc) {
  Abc_NtkPrintStats(abc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
}

ALICE_LOG_STORE_STATISTICS(pabc::Abc_Ntk_t *, abc) {
  return {{"name", pabc::Abc_NtkName(abc)},
          {"inputs", pabc::Abc_NtkPiNum(abc)},
          {"outputs", pabc::Abc_NtkPoNum(abc)},
          {"nodes", pabc::Abc_NtkNodeNum(abc)},
          {"levels", pabc::Abc_NtkLevel(abc)}};
}

ALICE_CONVERT(pabc::Abc_Ntk_t *, element, aig_network) {
  pabc::Abc_Ntk_t *pNtk = element;
  aig_network aig = MagicLS::abc2mockturtle_a(pNtk);
  return aig;
}

ALICE_CONVERT(xmg_network, element, pabc::Abc_Ntk_t *) {
  xmg_network xmg = element;
  pabc::Abc_Ntk_t *pNtk = MagicLS::mockturtle2abc_x(xmg);
  pabc::Abc_Ntk_t *pNtkLogic = pabc::Abc_NtkToLogic(pNtk);
  return pNtkLogic;
}

ALICE_CONVERT(aig_network, element, pabc::Abc_Ntk_t *) {
  aig_network aig = element;
  pabc::Abc_Ntk_t *pNtk = MagicLS::mockturtle2abc_a(aig);
  pabc::Abc_Ntk_t *pNtkLogic = pabc::Abc_NtkToLogic(pNtk);
  return pNtkLogic;
}

ALICE_CONVERT(mig_network, element, pabc::Abc_Ntk_t *) {
  mig_network mig = element;
  pabc::Abc_Ntk_t *pNtk = MagicLS::mockturtle2abc_m(mig);
  pabc::Abc_Ntk_t *pNtkLogic = pabc::Abc_NtkToLogic(pNtk);
  return pNtkLogic;
}

ALICE_CONVERT(xag_network, element, pabc::Abc_Ntk_t *) {
  xag_network xag = element;
  pabc::Abc_Ntk_t *pNtk = MagicLS::mockturtle2abc_g(xag);
  pabc::Abc_Ntk_t *pNtkLogic = pabc::Abc_NtkToLogic(pNtk);
  return pNtkLogic;
}

ALICE_CONVERT(klut_network, element, pabc::Abc_Ntk_t *) {
  klut_network klut = element;
  pabc::Abc_Ntk_t *pNtk = MagicLS::mockturtle2abc_l(klut);
  pabc::Abc_Ntk_t *pNtkLogic = pabc::Abc_NtkToLogic(pNtk);
  return pNtkLogic;
}

/* ABC Gia */
ALICE_ADD_STORE(pabc::Gia_Man_t *, "gia", "i", "GIA", "GIAs")

ALICE_DESCRIBE_STORE(pabc::Gia_Man_t *, gia) {
  // const auto name = pabc::Gia_ManName(gia);
  //When using `aig_to_gia`, the generated GIA network does not contain `Gia_ManName` information, hence it is disabled.
  const auto pi_num = pabc::Gia_ManPiNum(gia);
  const auto po_num = pabc::Gia_ManPoNum(gia);
  const auto gates_num = pabc::Gia_ManAndNum(gia);
  const auto level = pabc::Gia_ManLevelNum(gia);
  // return fmt::format("{}   i/o = {}/{}", name, pi_num, po_num);
  return fmt::format("[GIA]   i/o = {}/{}  nodes = {}  level = {}", pi_num, po_num, gates_num, level);
}

ALICE_PRINT_STORE(pabc::Gia_Man_t *, os, gia) {
  os << "GIA PI/PO = " << pabc::Gia_ManPiNum(gia) << "/"
     << pabc::Gia_ManPoNum(gia) << "\n";
}

ALICE_PRINT_STORE_STATISTICS(pabc::Gia_Man_t *, os, gia) {
  pabc::Gps_Par_t Pars{};
  pabc::Gia_ManPrintStats(gia, &Pars);
}

ALICE_LOG_STORE_STATISTICS(pabc::Gia_Man_t *, gia) {
  return {{"name", pabc::Gia_ManName(gia)},
          {"inputs", pabc::Gia_ManPiNum(gia)},
          {"outputs", pabc::Gia_ManPoNum(gia)},
          {"nodes", pabc::Gia_ManAndNum(gia)},
          {"levels", pabc::Gia_ManLevelNum(gia)}};
}

ALICE_ADD_FILE_TYPE(gia, "Gia");

ALICE_READ_FILE(pabc::Gia_Man_t *, gia, filename, cmd) {
  return pabc::Gia_AigerRead((char *)filename.c_str(), 0, 0, 0);
}

ALICE_WRITE_FILE(pabc::Gia_Man_t *, gia, gia, filename, cmd) {
  pabc::Gia_AigerWrite(gia, (char *)filename.c_str(), 1, 0, 0);
}

/* gia */
//ALICE_ADD_STORE(gia_network, "gia", "q", "gia", "GIA")

ALICE_CONVERT(aig_network, element, pabc::Gia_Man_t *) {
  aig_network aig = element;
  gia_network gia(aig.size() << 1);
  aig_to_gia(gia, aig);
  pabc::Gia_Man_t * GIA = const_cast<pabc::Gia_Man_t*>(gia.get_gia());

  return GIA;
}
ALICE_CONVERT(pabc::Gia_Man_t *, element, aig_network) {
  pabc::Gia_Man_t * gia_ = element;
  aig_network aig;
  gia_network gia(gia_);
  gia_to_aig(aig, gia);

  return aig;
}

}  // namespace alice

#endif

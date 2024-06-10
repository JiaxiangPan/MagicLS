/* MagicLS: Magic Logic Synthesis
 * Copyright (C) 2024 */

/**
 * @file divider.hpp
 *
 * @brief Generator for divider
 *
 * @author Jiaxiang Pan
 * @since  2024/06/10
 */

#ifndef DIVIDER_HPP
#define DIVIDER_HPP

#include <iostream>
#include <vector>
#include <utility>

#include <mockturtle/traits.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/generators/control.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <kitty/static_truth_table.hpp>

#include "../core/arithmetic.hpp"
#include "../core/my_function.hpp"

namespace alice
{

  using namespace mockturtle;

  class divider_command: public command
  {
    public:
      explicit divider_command( const environment::ptr& env ) 
        : command( env, "divider generator" )
      {
        add_option( "-b, --bit", BIT, "set the bit width of divider" );
        add_flag( "--rbs, -r", "set the ripple borrow subtractor to the trial-subtractor function" );
        add_option( "-a, --advance", func, "set the advanced subtractor to the trial-subtractor function, set{brent-kung; kogge-stone; han-carlson; BRS; BLS}" );
        add_flag( "--print_tt, -p", "print the network's output truth table (BIT <= 8)." );
      }

    protected:
      void execute()
      {
        if( is_set( "bit" ) )
        {
          if( is_set( "rbs" ) )
          {
            std::cout << "divider based on ripple borrow subtractor\n";
            aig_network aig;

            // ripple_borrow_divider(inital NRD)
            std::vector<aig_network::signal> a ( BIT ), b( BIT );
            std::generate( a.begin(), a.end(), [&aig]() { return aig.create_pi(); } );
            std::generate( b.begin(), b.end(), [&aig]() { return aig.create_pi(); } );
            
            auto [quo, rem] = restoring_array_divider( aig, a, b );

            for( auto const &o : quo )
            {
              aig.create_po( o );
            }

            for( auto const &r : rem )
            {
              aig.create_po( r );
            }

            if( is_set( "print_tt" ) && BIT <= 8u )
            {
              assert( BIT <= 8u );
              default_simulator<kitty::dynamic_truth_table> sim( BIT*2 );
              const auto tt = simulate<kitty::dynamic_truth_table>( aig, sim );

              for( int i = 0; i < BIT; i++ )
              {
                std::cout << "tt: 0x" << kitty::to_hex( tt[i] ) << std::endl;
              }
            }

            aig = cleanup_dangling( aig );

            store<aig_network>().extend();
            store<aig_network>().current() = aig;

            MagicLS::print_stats(aig);
          }

          if( is_set( "advance" ) )
          {
            // advanced algorithm
            aig_network AIG;
            
            std::vector<aig_network::signal> A ( BIT ), B( BIT );
            std::generate( A.begin(), A.end(), [&AIG]() { return AIG.create_pi(); } );
            std::generate( B.begin(), B.end(), [&AIG]() { return AIG.create_pi(); } );
            
            std::pair<vector<aig_network::signal>, vector<aig_network::signal>> QaR; // QUO and REM, e.g. quotient and reminder
            if( func == "brent-kung" )
            {
              std::cout << "divider based on brent-kung subtractor\n";
              QaR = restoring_array_divider_advance( AIG, A, B, mockturtle::detail::brent_kung_subtractor_inplace );
            }
            else if( func == "kogge-stone")
            {
              std::cout << "divider based on kogge-stone subtractor\n";
              QaR = restoring_array_divider_advance( AIG, A, B, mockturtle::detail::kogge_stone_subtractor_inplace );
            }
            else if( func == "han-carlson" )
            {
              std::cout << "divider based on han-carlson subtractor\n";
              QaR = restoring_array_divider_advance( AIG, A, B, mockturtle::detail::han_carlson_subtractor_inplace );
            }
            else if( func == "BLS" )
            {
              std::cout << "divider based on BLS subtractor\n";
              QaR = restoring_array_divider_advance( AIG, A, B, mockturtle::detail::borrow_lookahead_subtractor_inplace );
            }
            else if( func == "BRS")
            {
              std::cout << "divider based on BRS subtractor\n";
              QaR = restoring_array_divider_advance( AIG, A, B, mockturtle::borrow_ripple_subtractor_inplace );
            }
            else
            {
              std::cout << "error: no " << func << "function!" << std::endl;
              exit(1);
            }

            for( auto const &O : QaR.first )
            {
              AIG.create_po( O );
            }

            for( auto const &R : QaR.second )
            {
              AIG.create_po( R );
            }

            if( is_set( "print_tt" ) && BIT <= 8u )
            {
              assert( BIT <= 8u );
              default_simulator<kitty::dynamic_truth_table> SIM( BIT*2 );
              const auto TT = simulate<kitty::dynamic_truth_table>( AIG, SIM );

              for( int i = 0; i < BIT; i++ )
              {
                std::cout << "TT: 0x" << kitty::to_hex( TT[i] ) << std::endl;
              }
            }

            AIG = cleanup_dangling( AIG );

            store<aig_network>().extend();
            store<aig_network>().current() = AIG;

            MagicLS::print_stats(AIG);
          }
        }
      }

    private:
      uint32_t BIT = 0u;
      std::string func = "";
  };

  ALICE_ADD_COMMAND( divider, "Generator" )

}

#endif

// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : search.h
//                       Header zu search.cpp
//
//  Anfang des Projekts: So, 5.August, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: search.h,v 1.59 2003/05/31 19:25:51 rosendahl Exp $
// Copyright (C) 2001 Peter Rosendahl
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Email: p_rosendahl@t-online.de
// -------------------------------------------------------------------------

#ifndef SEARCH_H
#define SEARCH_H

#ifdef USE_DEFINESH
#include "defines.h"
#endif
#include "board.h" 
#include "hash.h"
#include <sys/timeb.h>
#include <fstream>


// Max.Tiefe durch Extensions
const int STOP_EXTENSIONS = 80;

// Parameter für Extensions 
const int CHECK_EXTENSION_VAL           = 16;
const int MATE_THREAD_EXTENSION_VAL     = 12;
const int EXCHANGE_EXTENSION_VAL        =  4; 
const int PAWN_PROMOTION_EXTENSION_VAL  =  6;
const int ONE_MOVE_EXTENSIONS_VAL       = 12;

// Futility pruning
const int FUTIL_MARGIN                  =  300;  // nach Heinz: E{2P_v, ..., 4P_v}
const int EXTD_FUTIL_MARGIN             =  500;  //   "    "  : E{R_v, ..., R_v+P_v}
const int RAZOR_MARGIN                  =  900;  //   "    "  : E{Q_v, ..., Q_val+P_v}

const int EXT_PLY_1                     = 16;
const int EXT_PLY_2                     = 32;
const int EXT_PLY_3                     = 48;
const int EXT_PLY_4                     = 64;
const int EXT_SHFT                      =  4;  // == ln(16)/ln(2)



// ----------------------------------------------------------------
//                      c l a s s   S e a r c h
// ----------------------------------------------------------------

class Search
{
// ------------- private members --------------------
public:
    Board* pBoard;

    // -------------------------
    //      Statistik
    // -------------------------
    long nNodes;            // Anzahl besuchter Positionen
    long nQNodes;           // Anzahl besuchter Positionen in Quiescence
    long nFaileHigh;        // Anzahl beta-cutoffs
    long nFaileHighOnFirst; // Anzahl beta-cutoffs beim ersten Zug
    long nHashEntriesFound; // Anzahl Stellugen aus dem Speicher

    int nMateThreadExt;    // Extensions durch Mattdrohung
    int nCheckExt;         // Extensions durch angegriffenen König
    int nExchangeExt;      // Extensions durch Figurenabtausch
    int nPromotionExt;     // Extensions durch Bauernumwandlung
    int nOneMoveExt;       // Extensions durch Bauernumwandlung
    int nExFulPrune;       // Wie oft wurde Extended Futility Pruning angewendet

    long nWrongHashmove;    // Hash debugging


    // Für Zugsortierung an der Wurzel:
    // wie viele Knoten wurden für Zug besucht?
    long nNodesRootMove[MAX_MOVESTACK];  

private:
    // -------------------------
    //      Zeitmessung
    // -------------------------
    long start;
    bool stop_now;
    double  max_time;

    long timer_calls;
    unsigned  tm_call_mask;

    int rootage;       // Alter der Position an der Wurzel der Suche

    int iter_depth;    // Momentane Iterationstiefe

    int root_alpha,root_beta;   // Suchfenster an der Wurzel

    // Ponder Zug
    Move ponder_move;
    bool got_ponder_move;
    bool pondering;


    // EPD Tests
    bool isEPDTest;

    // Zugriff auf Hash
    Hash* p_TT;

#ifdef TRACE_SEARCH
    std::fstream tracelog;
#endif
// ------------- public members  --------------------
public:
    // Principal variation array
    struct PVEntry {
        Move m;
        int type;
    };

    PVEntry pv[MAXPLY][MAXPLY];
    int     pv_length[MAXPLY];

    int last_pv_score;

// ------------- private methods --------------------
private:
    Search();
    void dump_pc(int ply, int score);
    void dump_pc_san(int depth, int score, bool depth_completed, bool aspir_low = false,
                     bool aspir_high = false);
    void dump_current_board(int ply, Move m);
    void dumpStatistics(); 

    bool time_up();


    enum PvCodes {
        END    = 32,
        HASH   = 16,
        FHIGH  =  8,
        FLOW   =  4,
        QUIES  =  2,
        NORMAL =  1
    };


// ------------- public methods  --------------------
public:
    Search(Board* pbd);
    ~Search();

    // Suche starten (maximale Zeit in Millisek.: max_time)
    int go(long max_time_ms, int& it_depth,  bool ponder, int max_iter,
               std::vector<Move>* p_epd_move_list, long& epd_time, bool epd_bm);


    // Zug mit bester Score nach vorne bringen
    void swapWithBestScoringMove(int ply, int idx);

    // -----------------------
    //  Zugriff auf pv:
    // -----------------------

    // Evaluierung bei Zugtiefe 'ply'
    void setEvalMark(int ply)
    {
        pv[ply][ply].m    = 0;
        pv[ply][ply].type = END;
        pv_length[ply]    = 0;
    }


    // Momentan beste Linie innerhalb der Suche ab Tiefe 'ply' speichern; 'm' ist der 
    // Zug der ausgeführt wurde, um die aktuelle Auswertung zu erhalten
    void saveBestLine(int ply,const Move& m, int type)
    {
        pv[ply][ply].m    = m;
        pv[ply][ply].type = type; 

        for (int i = ply+1; i <= ply + pv_length[ply+1]; i++)
        {
            pv[ply][i] = pv[ply+1][i];
            //if (pv[ply][i].type == END)
            //    break;
        }
        pv_length[ply] = pv_length[ply+1]+1;
    }

    // Initialisierung von 'pv'
    void init_pv()
    {
        for (int i=0; i < MAXPLY; i++)
        {
            pv_length[i] = 0;
            for (int j = 0; j < MAXPLY; j++)
                pv[i][j].type = END;
        }
    }

    
    // Für Hash debugging
    bool hashmove_ok(const Move& m, int ply) const;


    // -----------------------------------------------
    //          S u c h o p t i o n e n
    // -----------------------------------------------

    struct SearchOptions {
        bool do_extensions;
        bool do_nullmoves;
        bool do_ef_prune;
        bool no_post;
    } options;

    bool getOptExtensions() const
    {
        return options.do_extensions;
    }
    void setOptExtensions(bool do_ext)
    {
        options.do_extensions = do_ext;
    }
    bool getOptEFPruning() const
    {
        return options.do_ef_prune;
    }
    void setOptEFPruning(bool do_efp)
    {
        options.do_ef_prune = do_efp;
    }
    bool getOptNullMoves() const
    {
        return options.do_nullmoves;
    }
    void setOptNullmoves(bool do_null)
    {
        options.do_nullmoves = do_null;
    }
    bool getOptNoPost() const
    {
        return options.no_post;
    }
    void setOptNoPost(bool no_post)
    {
        options.no_post = no_post;
    }

    // -----------------------------------------------
    // Ponder Zug verfügbar?
    // -----------------------------------------------
    bool gotPonderMove() const
    {
        return got_ponder_move;
    }
    Move getPonderMove() const
    {
        return ponder_move;
    }



    // -----------------------------------------------
    //          S U C H A L G O R I T H M E N
    // -----------------------------------------------

    // Perft
    int perft(int depth, int ply);
    void run_perft(int depth);

    // Quiescence Suche
    int quiescence(int ply, int alpha, int beta);

    // Negascout Suche
    int negascout(int alpha, int beta, int depth, int ply, bool do_nullmove,
                  bool inCheck);

    // 1.Zug suchen
    int rootSearch(int alpha, int beta, int depth, std::vector<Move>* p_epd_move_list, 
                      long& epd_time, bool epd_bm, long epd_start_time);


    // Iterative Deepening
    int iter_deep(int max_depth, std::vector<Move>* p_epd_move_list, 
                  long &epd_time, bool epd_bm);

    void updateEpdTime(long&epd_time, long epd_start_time, 
                           std::vector<Move>* p_m_list, bool epd_bm);

    // Zugsortierung
    void calc_move_score_caps(int ply);

    // Null-Moves
    bool try_null(const int beta, const int frac_depth, int& null_red);


};

#endif // SEARCH_H




// -------------------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : eval.h
//                       Evaluierungsklasse Eval; zweite,vollkommen überarbeitete 
//                       Version.
//                       Im Gegensatz zur Vorgängerversion sind die Features keine
//                       Klassen mehr, wie z.Bsp. class PawnMaterial : public Feature,
//                       sondern komplett in der Evaluierungsfunktion integriert.
//                       Um dennoch flexible Parameter für Lernalgorithmen zu haben,
//                       werden die Feature-Gewichte in einem Array abgespeichert.
//                       
//
//  Anfang             : Freitag 2.November, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: eval.h,v 1.50 2003/06/02 18:12:53 rosendahl Exp $
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

#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "phash.h"
#include <vector>


// Konstanten
static const int LAZY_EXIT_OFFSET = 135;    // ermöglicht  vorzeitiges
                                            // Beenden von eval

// -------------------------------------------------------------------------
//  Für Lern-Algorithmen: double Version der FeatureGewichte
// -------------------------------------------------------------------------
typedef std::vector<double> FeatureEval;



// -------------------------------------------------------------------------------------
//                                  c l a s s  E v a l 
// -------------------------------------------------------------------------------------
class Eval
{
public:
    // -------------------------------------------------------------
    //    W e r t e  f ü r  G e w i c h t e
    // -------------------------------------------------------------
    enum {
        // Material
        PAWN_MATERIAL    = 100,
        BISHOP_MATERIAL  = 325,
        KNIGHT_MATERIAL  = 325,
        ROOK_MATERIAL    = 500,
        QUEEN_MATERIAL   = 950,

        BAD_TRADE        = 65,
        ROOK_VS_MINOR    = 80,

        // Bauernstruktur
        DOUBLE_PAWN_PENALTY        =  6,
        PAWN_DUO_BONUS             =  1,
        ISOLATED_PAWN_PENALTY      =  9,
        BACKWARD_PAWN_PENALTY      =  3,
        PASSED_PAWN_BONUS          =  8,
        CONN_PASSED_PAWNS          = 20,
        ISOLATED_DBLPAWN_PENALTY   = 25,


        // SKALIEREN DER STRUKTURWERTE IN ABHÄNGIGKEIT VON DER SPIELPHASE
        // Einheit: 1/64 CentiPawns, d.h: Faktor 32 entspr. 0.5
/*        SCALE_PWN_OP   = 56,
        SCALE_PWN_EM   = 60,
        SCALE_PWN_LM   = 64,
        SCALE_PWN_EG   = 64, */

        // Läufer
        TRAPPED_BISHOP_PENALTY  = 103,
        BISHOPPAIR_BONUS        =   6,
        BISHOP_KING_ATTACK      =   6,

        // Springer
        KNIGHT_KING_ATTACK     = 6,


        // Türme
        ROOK_ON_OPEN_FILE       = 11,
        ROOK_ON_HALFOPEN_FILE   =  7,
        ROOKS_ON_7              = 15,
        ROOK_KING_ATTACK        =  5,
        ROOK_FRONT_PAWN_PENALTY =  6,

        // Dame
        QUEEN_KING_ATTACK      =  6,
        QUEEN_ROOK_ON_7        = 15,
        QUEEN_DEFENDER         = 75,
    

        // Königssicherheit
        KING_ON_OPEN_FILE      = 38,
        KING_ON_HALFOPEN_FILE  = 27,
        CRITICAL_PAWN_WALL     = 55,
        BAD_PAWN_WALL          = 35,
        MEDIUM_PAWN_WALL       = 18,
        GOOD_PAWN_WALL         =  4,

        // Rochaden
        NO_CASTLE_PENALTY      = 26,

        // Mobilität
        ROOK_MOBILITY          =  1,
        BISHOP_MOBILITY        =  1, 
        FENCED_ROOK_PENALTY    = 15,
        FENCED_KING_ENDG_PENALTY = 14
    };

    enum Stages {
        OPENING       = 0,
        EARLY_MIDGAME = 1,
        LATE_MIDGAME  = 2,
        ENDGAME       = 3  
    };

private:
    // -------------------------------------------------
    //  Statische Evaluierungstabellen
    // -------------------------------------------------
    // Bauern
    static int posval_pawn_op[64];
    static int posval_pawn_emid[64];
    static int posval_pawn_lmid[64];
    static int posval_pawn_end[64];
    // Springer
    static int posval_knight_op_emid[64];
    static int posval_knight_lmid_end[64];
    // Läufer
    static int posval_bishop_op_emid[64];
    static int posval_bishop_lmid_end[64];
    // Türme
    static int posval_rook_op[64];
    static int posval_rook_emid[64];
    static int posval_rook_lmid[64];
    static int posval_rook_end[64];
    // König
    static int posval_king_op[64];
    static int posval_king_emid[64];
    static int posval_king_lmid[64];
    static int posval_king_end[64];



    // Score-Tabellen
    static int score_passed_pawn[8];
    static int score_out_ppawns[8];
    static int score_block_passer[8];

    // Pawn Hash
    PHash* p_pawnTT;

    // Board
    Board* p_board;
private:
    // -------------------------------------------------
    //  Singleton
    // -------------------------------------------------
    Eval();
    Eval(const Eval&);
    static Eval* p_instance;



    // -------------------------------------------------
    //  Evaluierungsfunktionen für die einzelnen
    //  Spielstufen
    // -------------------------------------------------
    int eval_op( int alpha, int beta);
    int eval_emid(int alpha, int beta);
    int eval_lmid(int alpha, int beta);
    int eval_end(int alpha, int beta);

    void pawn_eval(PTTe*& pawn_info,int stage, 
                    const int posval_pawn[]);

    int uncatchable_pawn_eval(const PTTe& pawn_info );
    bool drawish();


    int bishop_eval(const int posval_bishop[], int stage);
    int knight_eval(const int posval_knight[],
                    const PTTe* const p_pawn_info, int stage);
    int rook_eval(const int posval_rook[], 
        const PTTe* const p_pawn_info, int stage);
    int queen_eval(int stage);
    int king_eval(const int posval_king[], int stage);
    int king_safety_score(const PTTe* const p_pawn_info);

public:
    // Statistik
//    int nPHashEntriesUsed;
    int nLazyExits;
    int nWrongLazyExits;    // Für Debugging

public:
    // Zugriff auf Eval-Object
    static Eval* getHandle() { 
       if (p_instance == NULL) {
          p_instance = new Eval();
       }
       return p_instance; 
    }

    // INitialisierung der Feature Gewichte
    void initPieceVal();

    // Spielabschnitt der Position aus pB bestimmen
    int getStage();

    // -------------------------------------------------
    //  Evaluierungsfunktion ... alpha,beta werden
    //  übergeben, um "Lazy Evaluations" zu ermöglichen
    // -------------------------------------------------
    int eval(int alpha = -INF_SCORE, int beta = +INF_SCORE);

    // Materialscore
    int getMatScore();

    // Auswerten schlechter Materialverteilungen
    int bad_trades();


    inline int mirror(int x) const
    {
        return (x&0x7) | ( (x&0x38) ^ 0x38);
    }

    /** Zeigt die statische Evaluierung einer Positon.
     */
    void showEval();

};

#endif // EVAL_H

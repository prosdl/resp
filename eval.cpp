// -------------------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : eval.cpp
//                       Evaluierungsklasse Eval; zweite,vollkommen überarbeitete 
//                       Version.
//                       Im Gegensatz zur Vorgängerversion sind die Features keine
//                       Klassen mehr, wie z.Bsp. class PawnMaterial : public Feature,
//                       sondern komplett in der Evaluierungsfunktion integrierte
//                       Verfahren.
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
// $Id: eval.cpp,v 1.79 2003/06/03 17:46:06 rosendahl Exp $
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

#include "eval.h"
#include "phash.h"
#include "basic_stuff.h"
#include "bitboard.h"
#include "recognizer.h"
#include "board.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
using namespace std;



// ----------------------------------------------------------------
//  S t a t i s c h e  E v a l u i e ru n g s t a b e l l e n
// ----------------------------------------------------------------

//  ----------------
//    B a u e r n
//  ----------------


int Eval::posval_pawn_op[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
    -4, -4,  2,  6,  6,-10,-10,-10,
    -3, -3,  2,  8, 10, -9, -9, -9,
    -2, -2,  1, 15, 16, -6, -6, -6,
    -1, -1,  1,  6,  6, -3, -3, -3,
     0,  0,  0,-10,-10,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};



int Eval::posval_pawn_emid[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  4,  4,  0,  0,  0,
     0,  0,  2,  8,  8,  0,  0,  0,
     5,  5,  7, 12, 12,  4,  4,  0,
     3,  2,  3,  6,  6, -2, -1, -1,
     0,  0,  0,-20,-20,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

int Eval::posval_pawn_lmid[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
     9, 10, 10, 10, 10, 10, 10,  9,
     7,  8,  8,  8,  8,  8,  8,  7,
     5,  6,  6,  6,  6,  6,  6,  5,
     3,  4,  4,  4,  4,  4,  4,  3,
     1,  2,  2,  2,  2,  2,  2,  1,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

int Eval::posval_pawn_end[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
    10, 11, 11, 11, 11, 11, 11, 10,
     8,  9,  9,  9,  9,  9,  9,  8,
     6,  7,  7,  7,  7,  7,  7,  6,
     4,  4,  4,  4,  4,  4,  4,  4,
     2,  2,  2,  2,  2,  2,  2,  2,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

//  ----------------
//   K ö n i g 
//  ----------------
int Eval::posval_king_op[64] = {
   -99,-99,-99,-99,-99,-99,-99,-99,
   -99,-99,-99,-99,-99,-99,-99,-99,
   -99,-99,-99,-99,-99,-99,-99,-99,
   -99,-99,-99,-99,-99,-99,-99,-99,
   -99,-99,-99,-99,-99,-99,-99,-99,
   -80,-80,-80,-80,-80,-80,-80,-80,
   -35,-35,-35,-45,-45,-35,-35,-35,
    10, 10, 10,-15,  0,-15, 14, 12
};
int Eval::posval_king_emid[64] = {
   -30,-30,-30,-30,-30,-30,-30,-30,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -15,-15,-15,-15,-15,-15,-15,-15,
    10, 10, 10,-15,  0,-15, 12, 12
};
int Eval::posval_king_lmid[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
    -1, -1, -1,  0,  0, -1, -1, -1,
     1,  5,  1,  0,  0,  1,  5,  1
};
int Eval::posval_king_end[64] = {
     0,  6, 12, 18, 18, 12,  6,  0,
     6, 12, 18, 24, 24, 18, 12,  6,
    12, 18, 24, 30, 30, 24, 18, 12,
    18, 24, 30, 36, 36, 30, 24, 18,
    18, 24, 30, 36, 36, 30, 24, 18,
    12, 18, 24, 30, 30, 24, 18, 12,
     6, 12, 18, 24, 24, 18, 12,  6,
     0,  6, 12, 18, 18, 12,  6,  0
};

//  -----------------
//   S p r i n g e r
//  -----------------
int Eval::posval_knight_op_emid[64] = {
    -8, -5, -5, -5, -5, -5, -5, -8,
    -5,  0,  0,  3,  3,  0,  0, -5,
    -5,  0,  5,  5,  5,  5,  0, -5,
    -5,  0,  5,  9,  9,  5,  0, -5,
    -5,  0,  5,  9,  9,  5,  0, -5,
    -5,  0,  5,  7,  7,  5,  0, -5,
    -5,  0,  0,  3,  3,  0,  0, -5,
    -7, -8, -4, -4, -4, -4, -8, -7
};

int Eval::posval_knight_lmid_end[64] = {
   - 5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  1,  1,  0,  0,  0,
     0,  0,  1,  3,  3,  1,  0,  0,
     0,  1,  3,  5,  5,  3,  1,  0,
     0,  1,  3,  5,  5,  3,  1,  0,
     0,  0,  1,  3,  3,  1,  0,  0,
     0,  0,  0,  1,  1,  0,  0,  0,
   - 5,  0,  0,  0,  0,  0,  0, -5
};

//  -----------------
//    L ä u f e r 
//  -----------------
int Eval::posval_bishop_op_emid[64] = {
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5,  7,  5,  7,  7,  5,  7, -5,
    -5,  5,  3,  6,  6,  3,  5, -5,
    -5,  3,  6,  5,  5,  6,  3, -5,
    -5,  3,  8,  5,  5,  8,  3, -5,
    -5,  5,  3,  4,  4,  3,  5, -5,
    -5,  7,  5,  5,  5,  5,  7, -5,
    -5, -8, -5, -5, -5, -5, -8, -5,
};

int Eval::posval_bishop_lmid_end[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  4,  4,  0,  0,  0,
     0,  0,  4,  8,  8,  4,  0,  0,
     0,  4,  8,  9,  9,  8,  4,  0,
     0,  4,  8,  9,  9,  8,  4,  0,
     0,  0,  4,  8,  8,  4,  0,  0,
     0,  0,  0,  4,  4,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

//  -----------------
//      T ü r m e
//  -----------------
int Eval::posval_rook_op[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    34, 34, 34, 34, 34, 34, 34, 34,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  5,  5,  3,  0,  0
};
int Eval::posval_rook_emid[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    28, 28, 28, 28, 28, 28, 28, 28,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  5,  5,  3,  0,  0
};
int Eval::posval_rook_lmid[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    24, 24, 24, 24, 24, 24, 24, 24,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};
int Eval::posval_rook_end[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    15, 15, 15, 15, 15, 15, 15, 15,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};


// --------------------------------------------------------------
//  Score-Tabellen
// --------------------------------------------------------------

int Eval::score_passed_pawn[8] = {
    0,8,14,30,60,100,130,0
};

int Eval::score_block_passer[8] = {
    0,2,4,6,12,25,32,0
};

int Eval::score_out_ppawns[8] = {
    1,2,6,7,11,13,15,17
};


// --------------------------------------------------------------
//  Definition Eval Object
// --------------------------------------------------------------
Eval* Eval::p_instance = NULL;


// --------------------------------------------------------------
//  Konstruktor
// --------------------------------------------------------------

Eval::Eval()
{
    cout << "#Initializing Eval" << endl;
    initPieceVal();
    p_pawnTT = PHash::getHandle();
    p_board  = Board::getHandle();
    cout << "#Eval ready" << endl;
}


// --------------------------------------------------------------
// Initialisierung der Feature Gewichte
// --------------------------------------------------------------


void Eval::initPieceVal()
{
    // piece_val aktualisieren
    piece_val[PAWN]     = PAWN_MATERIAL;
    piece_val[BISHOP]   = BISHOP_MATERIAL;
    piece_val[KNIGHT]   = KNIGHT_MATERIAL;
    piece_val[ROOK]     = ROOK_MATERIAL;
    piece_val[QUEEN]    = QUEEN_MATERIAL;
}

// --------------------------------------------------------------
//  Bestimmen der aktuellen Spielphase (Eröffnung, Mittelspiel,
//  Endspiel)
// --------------------------------------------------------------
inline int Eval::getStage()
{
    int n_pawns = p_board->stats.nBlackPieces[PAWN] + 
                  p_board->stats.nWhitePieces[PAWN];
    int n_pieces = p_board->stats.nBlackPiecesTotal + 
                   p_board->stats.nWhitePiecesTotal - n_pawns;


    if (n_pawns > 12 && n_pieces > 12)
        return OPENING;
    else if (n_pieces >= 12 || (n_pawns > 10 && n_pieces > 10) )
        return EARLY_MIDGAME;
    else if (n_pawns > 8 && n_pieces > 8)
        return LATE_MIDGAME;
    else
        return ENDGAME;
}


// --------------------------------------------------------------
//                         e v a l
// --------------------------------------------------------------
//  Evaluierungsfunktions für resp. Neben der aktuellen Position
//  aus p_board wird das alpha-beta Fenster der Suche übergeben. Dies
//  ermöglicht 'eval' eine "Lazy Evaluation", falls in einem
//  frühen Stadium der Evaluierung klar ist, dass 
//
//      score <= alpha     ODER   score >= beta
//  
//  gilt.
// --------------------------------------------------------------
int Eval::eval(int alpha, int beta)
{
    // Aktuelle Spielphase bestimmen
    int stage = getStage();
    switch (stage)
    {
    case OPENING:

#ifdef DEBUG_EVAL
        cout << ".................OPENING EVALUATION" << endl;
#endif
        return eval_op(alpha,beta);
    case EARLY_MIDGAME:

#ifdef DEBUG_EVAL
        cout << ".................EARLY MIDGAME EVALUATION" << endl;
#endif
        return eval_emid(alpha,beta);
    case LATE_MIDGAME:

#ifdef DEBUG_EVAL
        cout << ".................LATE MIDGAME EVALUATION" << endl;
#endif
        return eval_lmid(alpha,beta);
    default:

#ifdef DEBUG_EVAL
        cout << ".................ENDGAME EVALUATION" << endl;
#endif
        return eval_end(alpha,beta);
    }
}

// --------------------------------------------------------------
//  Evaluierungsfunktion nur Material
// --------------------------------------------------------------

int Eval::getMatScore()
{
    int score = 0;
    // --------------------------
    // Materialscore berechnen
    // --------------------------
    score = (p_board->stats.nWhitePieces[PAWN] - p_board->stats.nBlackPieces[PAWN]) *
                PAWN_MATERIAL;
    score += (p_board->stats.nWhitePieces[BISHOP] - p_board->stats.nBlackPieces[BISHOP]) *
                BISHOP_MATERIAL;
    score += (p_board->stats.nWhitePieces[KNIGHT] - p_board->stats.nBlackPieces[KNIGHT]) *
                KNIGHT_MATERIAL;
    score += (p_board->stats.nWhitePieces[ROOK] - p_board->stats.nBlackPieces[ROOK]) *
                ROOK_MATERIAL;
    score += (p_board->stats.nWhitePieces[QUEEN] - p_board->stats.nBlackPieces[QUEEN]) * 
                QUEEN_MATERIAL;


    if (p_board->sideToMove() == WHITE)
        return score;
    else 
        return -score;

}

//  ---------------------------------------------------------
//       E v a l u i e r u n g s f u n k t i o n  f ü r  
//       B a u e r n s t r u k t ur 
//  ---------------------------------------------------------

void Eval::pawn_eval(PTTe*& p_pawn_info,int stage, 
                    const int posval_pawn[])
{
    static int whiteLAdvPawn[10];
    static int nWhitePawns[10];
    static int blackLAdvPawn[10];
    static int nBlackPawns[10];
    static bool punishedDblIso[8];

    PTTe pawn_info = PTTe(p_board->get_zp2_hash(),0,stage,0,0);

    Bitboard set;
    int pos;

    int i;

#ifdef DEBUG_EVALPAWNS
    cout << "-PAWN EVAL --------------------------------------" << endl;
#endif


    // Initialisieren
    for (i = 0; i < 10; i++)
    {
        whiteLAdvPawn[i] = 0;
        nWhitePawns[i]   = 0;
        blackLAdvPawn[i] = 7;
        nBlackPawns[i]   = 0;
    }

    // -----------------------------
    //    D u r c h g a n g   ( I )
    // -----------------------------

    //    S c a n n e n   d e r  w e i s s e n  B a u e r n

    set = p_board->whitePawns;

    int pscore = 0; // Position
    int sscore = 0; // Struktur
    while (set)
    {
        pos = set.msb();
        set ^= BBTables::bbmask[pos];

        pscore += posval_pawn[pos];

        int c = COL(pos); 
        int r = ROW(pos);

        if (r > whiteLAdvPawn[ c + 1 ])
            whiteLAdvPawn[ c + 1 ] = r;

        nWhitePawns[c + 1]++;

        if (nWhitePawns[c + 1] >= 2)
        {
            // Doppelbauern gefunden
            sscore -= DOUBLE_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
            cout << "WHITE DOUBLE PAWN ON COL = " << c ;
            cout << "  [ " << -DOUBLE_PAWN_PENALTY << " ]" << endl;
#endif
        }
    }

    // Halboffene Linien für Schwarz nach pinfo speichern
    for (i=1; i <= 8; i++)
        if (!nWhitePawns[i])
            pawn_info.setHalfOpenWhite(i - 1);


    //    S c a n n e n  d e r  s c h w a r z e n  B a u e r n
    
    set = p_board->blackPawns;

    while (set)
    {
        pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        pscore -= posval_pawn[mirror(pos)];


        int c = COL(pos); 
        int r = ROW(pos);

        if (r < blackLAdvPawn[ c + 1 ])
            blackLAdvPawn[ c + 1 ] = r;

        nBlackPawns[c + 1]++;

        if (nBlackPawns[c + 1] >= 2)
        {
            // Doppelbauern gefunden
            sscore += DOUBLE_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
            cout << "BLACK DOUBLE PAWN ON COL = " << c ;
            cout << "  [ " << DOUBLE_PAWN_PENALTY << " ]" << endl;
#endif
        }
    }

    // Halboffene Linien für Weiss nach pinfo speichern
    for (i=1; i <= 8; i++)
        if (!nBlackPawns[i])
            pawn_info.setHalfOpenBlack(i - 1);


    // ------------------------------
    //  D u r c h g a n g   ( I I )
    // ------------------------------
    
    // Stärken/Schwächen der Bauernstruktur feststellen


    // *****************************************
    // *******  w e i s s e  B a u e r n *******
    // *****************************************

    // Flaggenfeld für isolierte Doppelbauern
    for (i=0; i < 8; i++)
        punishedDblIso[i] = false;

    set = p_board->whitePawns;
    while (set)
    {
        pos = set.msb();
        set ^= BBTables::bbmask[pos];

        // --------------------------------------
        //          Bauern-Duo
        // --------------------------------------
        // Dies sind einfach benachbarte Bauern
        // gleicher Farbe

        int c = COL(pos);
        int r = ROW(pos);

        if (c < 7)
        {
            if (BBTables::bbmask[pos+1] & p_board->whitePawns)
            {
                sscore += PAWN_DUO_BONUS;

#ifdef DEBUG_EVALPAWNS
                cout << "WHITE DUO ON " << c << ", " << c+1;
                cout << "  [ " << PAWN_DUO_BONUS << " ]" << endl;
#endif
            }
        }


        bool left_partner = (c>0) ?  !pawn_info.isHalfOpenWhite(c-1) : 
                                      false;
                                      
        bool right_partner = (c<7) ? !pawn_info.isHalfOpenWhite(c+1) : 
                                      false;

        // ----------------------------------------
        //              Freibauern
        // ----------------------------------------
        // Bauern, die nicht mehr von gegnerischen
        // Bauern geschlagen werden können

        bool passed_pawn = false;
        if ((BBTables::w_ppawn_mask[pos] & p_board->blackPawns) == 0)
        {
            passed_pawn = true;
            sscore += stage*score_passed_pawn[7-r]/4;
            pawn_info.setWPassed(c);

#ifdef DEBUG_EVALPAWNS
            cout << "PASSED WHITE PAWN: " << pos;
            cout << "  [ " << stage*score_passed_pawn[7-r]/4
                 << " ]" << endl;
#endif
        }

        // --------------------------------------
        //      Isolierter Bauer
        // --------------------------------------
        // Keine Bauern gleicher Farbe auf den
        // Nachbarlinien
        if (!(left_partner || right_partner))
        {
            // Nicht doppelt?
            if (nWhitePawns[c + 1] == 1)
            {
                sscore -= ISOLATED_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
                cout << "ISOLATED WHITE PAWN: " << pos;
                cout << "  [ " << -ISOLATED_PAWN_PENALTY << " ]" << endl;
#endif
            }
            else // Doppelt
            {
                if (!punishedDblIso[c])
                {
                    punishedDblIso[c] = true;
                    // Besonders übel sind isolierte Doppelbauern: 
                    // Als Strafe gibt es: DOUBLE_PAWN_PENALTY  und dazu 
                    // ISOLATED_DBLPAWN_PENALTY
                    sscore -= ISOLATED_DBLPAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
                    cout << "ISOLATED WHITE DBLPAWN: " << pos;
                    cout << "  [ " << -ISOLATED_DBLPAWN_PENALTY << " ]" << endl;
#endif
                }
            }
            
        }

        // --------------------------------------
        //       Rückständige Bauern
        // --------------------------------------
        // Bauern ohne Schutz von freundlichen
        // Bauern; Freibauern werden ausgeschlossen
        else if (!passed_pawn && (!left_partner || whiteLAdvPawn[c] < r)  && 
                 (!right_partner || whiteLAdvPawn[c+2] < r))
        {
            sscore -= BACKWARD_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
            cout << "BACKWARD WHITE PAWN: " << pos;
            cout << "  [ " << -BACKWARD_PAWN_PENALTY << " ]" << endl;
#endif
        }
    
    
    }

    // *****************************************
    // ****** s c h w a r z e  B a u e r n *****
    // *****************************************

    // Flaggenfeld für isolierte Doppelbauern
    for (i=0; i < 8; i++)
        punishedDblIso[i] = false;

    set = p_board->blackPawns;
    while (set)
    {
        pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        // --------------------------------------
        //          Bauern-Duo
        // --------------------------------------
        // Dies sind einfach benachbarte Bauern
        // gleicher Farbe

        int c = COL(pos);
        int r = ROW(pos);

        if (c < 7)
        {
            if (BBTables::bbmask[pos+1] & p_board->blackPawns)
            {
                sscore -= PAWN_DUO_BONUS;

#ifdef DEBUG_EVALPAWNS
                cout << "BLACK DUO ON " << c << ", " << c+1;
                cout << "  [ " << -PAWN_DUO_BONUS << " ]" << endl;
#endif
            }
        }


        bool left_partner = (c>0) ?  !pawn_info.isHalfOpenBlack(c-1) : 
                                      false;
                                      
        bool right_partner = (c<7) ? !pawn_info.isHalfOpenBlack(c+1) : 
                                      false;

        // ----------------------------------------
        //              Freibauern
        // ----------------------------------------
        // Bauern, die nicht mehr von gegnerischen
        // Bauern geschlagen werden können

        bool passed_pawn = false;
        if ((BBTables::b_ppawn_mask[pos] & p_board->whitePawns) == 0)
        {
            passed_pawn = true;
            sscore -= stage*score_passed_pawn[r]/4;
            pawn_info.setBPassed(c);

#ifdef DEBUG_EVALPAWNS
            cout << "PASSED BLACK PAWN: " << pos;
            cout << "  [ " << -stage*score_passed_pawn[r]/4
                 << " ]" << endl;
#endif
        }


        // --------------------------------------
        //      Isolierter Bauer
        // --------------------------------------
        // Keine Bauern gleicher Farbe auf den
        // Nachbarlinien
        if (!(left_partner || right_partner))
        {
            // Nicht doppelt?
            if (nBlackPawns[c + 1] == 1)
            {
                sscore += ISOLATED_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
                cout << "ISOLATED BLACK PAWN: " << pos;
                cout << "  [ " << ISOLATED_PAWN_PENALTY << " ]" << endl;
#endif
            }
            else // Doppelt
            {
                if (!punishedDblIso[c])
                {
                    punishedDblIso[c] = true;
                    // Besonders übel sind isolierte Doppelbauern: 
                    // Als Strafe gibt es: DOUBLE_PAWN_PENALTY  und dazu 
                    // ISOLATED_DBLPAWN_PENALTY
                    sscore += ISOLATED_DBLPAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
                    cout << "ISOLATED BLACK DBLPAWN: " << pos;
                    cout << "  [ " << +ISOLATED_DBLPAWN_PENALTY << " ]" << endl;
#endif
                }
            }
            
        }
        // --------------------------------------
        //       Rückständige Bauern
        // --------------------------------------
        // Bauern ohne Schutz von freundlichen
        // Bauern; Freibauern werden ausgeschlossen
        else if (!passed_pawn && (!left_partner || blackLAdvPawn[c] > r)  && 
                 (!right_partner || blackLAdvPawn[c+2] > r))
        {
            sscore += BACKWARD_PAWN_PENALTY;

#ifdef DEBUG_EVALPAWNS
            cout << "BACKWARD BLACK PAWN: " << pos;
            cout << "  [ " << +BACKWARD_PAWN_PENALTY << " ]" << endl;
#endif
        }
    
    
    }

    // ----------------------------------------------------------------------
    //  D u r ch g a n g   (I I I)
    // ----------------------------------------------------------------------
    // Connected passed pawns

    // ----------
    //  Weiss
    // ----------
    int w_passed = pawn_info.w_passed;
    while (w_passed)
    {
        int c = BBTables::lsb16[w_passed];
        if (c >= 7) break;
        w_passed ^= 1<<c;

        if (w_passed & 1<<(c+1))    // Zwei Freibauern nebeneinander
        {
            Bitboard temp = p_board->whitePawns & BBTables::filemask[c];
            if (!temp) break;
            int pos1 = temp.lsb();
            if (ROW(pos1) >3) continue;

            temp = p_board->whitePawns & BBTables::filemask[c+1];
            if (!temp) break;
            int pos2 = temp.lsb();
            if (ROW(pos2) > 3) continue;

            if (abs(ROW(pos1)-ROW(pos2)) <= 1)
            {
#ifdef DEBUG_EVALPAWNS
                cout << "CONNECTED WHITE PASSED PAWNS AT " << pos1 << "," << pos2 << endl;
#endif
                sscore += CONN_PASSED_PAWNS*stage;
            }

        }

    }

    // ----------
    //  Schwarz
    // ----------
    int b_passed = pawn_info.b_passed;
    while (b_passed)
    {
        int c = BBTables::lsb16[b_passed];
        if (c >= 7) break;
        b_passed ^= 1<<c;

        if (b_passed & 1<<(c+1))    // Zwei Freibauern nebeneinander
        {
            Bitboard temp = p_board->blackPawns & BBTables::filemask[c];
            if (!temp) break;
            int pos1 = temp.msb();
            if (ROW(pos1) < 4) continue;

            temp = p_board->blackPawns & BBTables::filemask[c+1];
            if (!temp) break;
            int pos2 = temp.msb();
            if (ROW(pos2) < 4) continue;

            if (abs(ROW(pos1)-ROW(pos2)) <= 1)
            {
#ifdef DEBUG_EVALPAWNS
                cout << "CONNECTED BLACK PASSED PAWNS AT " << pos1 << "," << pos2 << endl;
#endif
                sscore -= CONN_PASSED_PAWNS*stage;
            }

        }

    }

    // ----------------------------------------------------------------------
    //  Outside passed pawns
    // ----------------------------------------------------------------------

    // nur late midgame oder endgame + Bauern muessen vorhanden sein
    if (stage > 1 && (p_board->stats.matBlack & 1) && (p_board->stats.matWhite&1)) 
    {
        int w_out_pp = 0;
        int b_out_pp = 0;

        // -----------------
        //      Weiss
        // -----------------
        int w_passed = pawn_info.w_passed;
        int b_pawns  = pawn_info.getBPawns() & ~pawn_info.getBPassed();

        if (b_pawns && w_passed)
        {
            int col_mostright_bpawn = BBTables::msb16[b_pawns];

            if (col_mostright_bpawn <= 5)
            {
                int col_mostright_wpasser = BBTables::msb16[w_passed];
                if (col_mostright_wpasser - col_mostright_bpawn >= 2)
                {
                    w_out_pp = col_mostright_wpasser - col_mostright_bpawn;
                }
            }

            int col_mostleft_bpawn = BBTables::lsb16[b_pawns];
            if (col_mostleft_bpawn >= 2)
            {
                int col_mostleft_wpasser = BBTables::lsb16[w_passed];
                if (col_mostleft_bpawn - col_mostleft_wpasser >= 2   && 
                    col_mostleft_bpawn - col_mostleft_wpasser >= w_out_pp)
                {
                    w_out_pp = col_mostleft_bpawn - col_mostleft_wpasser;
                }
            }
        }
        // -----------------
        //      Schwarz
        // -----------------
        int b_passed = pawn_info.b_passed;
        int w_pawns  = pawn_info.getWPawns() & ~pawn_info.getWPassed();
        int col_mostright_wpawn = BBTables::msb16[w_pawns];

        if (w_pawns && b_passed)
        {
            if (col_mostright_wpawn <= 5)
            {
                int col_mostright_bpasser = BBTables::msb16[b_passed];
                if (col_mostright_bpasser >= col_mostright_wpawn + 2)
                {
                    b_out_pp = col_mostright_bpasser - col_mostright_wpawn;
                }
            }

            int col_mostleft_wpawn = BBTables::lsb16[w_pawns];
            if (col_mostleft_wpawn >= 2)
            {
                int col_mostleft_bpasser = BBTables::lsb16[b_passed];
                if (col_mostleft_wpawn - col_mostleft_bpasser >= 2 && 
                    col_mostleft_wpawn - col_mostleft_bpasser >= b_out_pp)
                {
                    b_out_pp = col_mostleft_wpawn - col_mostleft_bpasser;
                }
            }
        }
#ifdef DEBUG_EVALPAWNS
        cout << "OUTSIDEPASSED PAWNS - WHITE DIST.= " << w_out_pp << endl;
        cout << "OUTSIDEPASSED PAWNS - BLACK DIST.= " << b_out_pp << endl;
#endif
        if (w_out_pp != b_out_pp)
        {
            int o_score = score_out_ppawns[abs(w_out_pp - b_out_pp)]*(stage-1);

            if (w_out_pp < b_out_pp) o_score = -o_score;
            sscore +=  o_score;

#ifdef DEBUG_EVALPAWNS
            cout << "OUTSIDEPASSED PAWNS - SCORE = " << o_score << endl;
#endif
        }
    }

    // ----------------------------------------------------------------------
    // In Abhängigkeit von der Spielphase den Strukturwert skalieren:
    //
    // score = "Positionelle Score" + FAKTOR(stage) * "Strukturelle Score"
    //
    // ----------------------------------------------------------------------

/*    switch (stage)
    {
    case OPENING:
        pawn_info.score = pscore + ((sscore* SCALE_PWN_OP) /64);
        break;
    case EARLY_MIDGAME:
        pawn_info.score = pscore + ((sscore* SCALE_PWN_EM) /64);
        break;
    case LATE_MIDGAME:
        pawn_info.score = pscore + ((sscore* SCALE_PWN_LM) /64);
        break;
    case ENDGAME:
        pawn_info.score = pscore + ((sscore* SCALE_PWN_EG) /64);
        break;
    }*/

    pawn_info.score = pscore + sscore;


#ifdef DEBUG_EVALPAWNS
    for (int col = 0; col < 8; col++)
    {
        if (pawn_info.isHalfOpenBlack(col))
            cout << "HALF OPEN BLACK COL = " << col << endl;
        if (pawn_info.isHalfOpenWhite(col))
            cout << "HALF OPEN WHITE COL = " << col << endl;
    }

    cout << endl;
    cout << "PAWN SCORE (POS)   = " << pscore << endl;
    cout << "PAWN SCORE (STRCT) = " << sscore << endl;
    cout << "SCORE              = " << pawn_info.score << endl;
#endif




    // ----------------------------------
    //      Score im Hash speichern
    // ----------------------------------
    p_pawn_info = p_pawnTT->insert(pawn_info,p_board->get_zp1_hash());
}

// --------------------------------------------------------------
//    bishop_eval_op_emid
// --------------------------------------------------------------
int Eval::bishop_eval(const int posval_bishop[], int stage)
{
    int bscore = 0;


    //    W e i s s e  L ä u f e r 
    Bitboard set = p_board->whiteBishops;
    while (set)
    {
        int pos = set.msb();
        set ^= BBTables::bbmask[pos];

        // statische Score
        bscore += posval_bishop[pos];

        // Abstand zum gegnerischen König
        if (stage != ENDGAME) {
           int d = BBTables::dist[pos][p_board->blackKingPos];
           if (d <= 3) {
              bscore += (4-d)*BISHOP_KING_ATTACK;
           }
        }

#ifdef DEBUG_EVAL
        cout << "WHITE BISHOP STATIC: " << pos;
        cout << "  [ " <<  posval_bishop_op_emid[pos] << " ]" << endl;
#endif

        // Läufer-Falle?
        if (pos == 8 && (BBTables::bbmask[17] & p_board->blackPawns)  ||
            pos ==15 && (BBTables::bbmask[22] & p_board->blackPawns) )
        {
            bscore -= TRAPPED_BISHOP_PENALTY;

#ifdef DEBUG_EVAL
            cout << "TRAPPED WHITE BISHOP PENALTY FOR " << pos << endl;
#endif
        }

        // Beweglichkeit
        int mobh8a1 = BBTables::diagH8A1_mobility[ pos ]
                      [ p_board->allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[pos]) ];

        int mobh1a8 = BBTables::diagH1A8_mobility[ pos ]
                      [ p_board->allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[pos]) ];

        bscore += (mobh8a1 + mobh1a8);


#ifdef DEBUG_EVAL
        cout << "WHITE BISHOP-MOBILITY: H1A8 = " << mobh1a8 << 
                " -- H8A1 = " << mobh8a1 << endl;
#endif


    }

    // Läuferpaar
    if (p_board->stats.nWhitePieces[BISHOP] >= 2)
    {
        bscore += BISHOPPAIR_BONUS;
    }

    //    S c h w a r z e  L ä u f e r 
    set = p_board->blackBishops;
    while (set)
    {
        int pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        // statische Score
        bscore -= posval_bishop[mirror(pos)];

        // Abstand zum gegnerischen König
        if (stage != ENDGAME) {
           int d = BBTables::dist[pos][p_board->whiteKingPos];
           if (d <= 3) {
              bscore -= (4-d)*BISHOP_KING_ATTACK;
           }
        }


#ifdef DEBUG_EVAL
        cout << "BLACK BISHOP STATIC: " << pos;
        cout << "  [ " <<  -posval_bishop_op_emid[mirror(pos)] << " ]" << endl;
#endif

        // Läufer-Falle?
        if (pos == 48 && (BBTables::bbmask[41] & p_board->whitePawns)  ||
            pos == 55 && (BBTables::bbmask[46] & p_board->whitePawns) )
        {
            bscore += TRAPPED_BISHOP_PENALTY;

#ifdef DEBUG_EVAL
            cout << "TRAPPED BLACK BISHOP PENALTY FOR " << pos << endl;
#endif
        }

        // Beweglichkeit
        int mobh8a1 = BBTables::diagH8A1_mobility[ pos ]
                      [ p_board->allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[pos]) ];

        int mobh1a8 = BBTables::diagH1A8_mobility[pos]
                      [ p_board->allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[pos]) ];

        bscore -= (mobh8a1 + mobh1a8);


#ifdef DEBUG_EVAL
        cout << "BLACK BISHOP-MOBILITY: H1A8 = " << mobh1a8 << 
                " -- H8A1 = " << mobh8a1 << endl;
#endif

    }

    // Läuferpaar
    if (p_board->stats.nBlackPieces[BISHOP] >= 2)
    {
        bscore -= BISHOPPAIR_BONUS;
    }


    return bscore;
}

// --------------------------------------------------------------
//  Springer Evaluierung
// --------------------------------------------------------------
int Eval::knight_eval(const int posval_knight[], 
                      const PTTe* const p_pawn_info, int stage)
{
    int kscore = 0;

    Bitboard set = p_board->whiteKnights;
    while (set)
    {
        int pos = set.msb();
        set ^= BBTables::bbmask[pos];

        kscore += posval_knight[pos];

        // Abstand zum gegnerischen König
        if (stage != ENDGAME) {
           int d = BBTables::dist[pos][p_board->blackKingPos];
           if (d <= 3) {
              kscore += (4-d)*KNIGHT_KING_ATTACK;
           }
        }

        // Blocker?
        if (pos>=16 && stage && (BBTables::bbmask[pos-8]&p_board->blackPawns) &&
            ( p_pawn_info->getBPassed() & 1 << COL(pos)))
        {
            Bitboard temp(p_board->blackPawns & BBTables::filemask[COL(pos)]);
            if (temp)
            {
                if (pos-8 == temp.msb())
                {
                    kscore += score_block_passer[ROW(pos-8)]*stage/4;
#ifdef DEBUG_EVAL
                    cout << "WHITE KNIGHT BLOCKS PASSER - SCORE = " << 
                            score_block_passer[ROW(pos-8)]*stage/4 << endl;
#endif
                }
            }
        }

#ifdef DEBUG_EVAL
        cout << "WHITE KNIGHT STATIC: " << pos;
        cout << "  [ " << posval_knight[pos] << " ]" << endl;
#endif
    }

    set = p_board->blackKnights;
    while (set)
    {
        int pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        kscore -= posval_knight[mirror(pos)];

        // Abstand zum gegnerischen König
        if (stage != ENDGAME) {
           int d = BBTables::dist[pos][p_board->whiteKingPos];
           if (d <= 3) {
              kscore -= (4-d)*KNIGHT_KING_ATTACK;
           }
        }

        // Blocker?
        if (pos<48 && stage && (BBTables::bbmask[pos+8]&p_board->whitePawns) &&
            ( p_pawn_info->getWPassed() & 1 << COL(pos)))
        {
            Bitboard temp(p_board->whitePawns & BBTables::filemask[COL(pos)]);
            if (temp)
            {
                if (pos+8 == temp.lsb())
                {
                    kscore -= score_block_passer[7-ROW(pos+8)]*stage/4;
#ifdef DEBUG_EVAL
                    cout << "BLACK KNIGHT BLOCKS PASSER - SCORE = " << 
                            -score_block_passer[7-ROW(pos+8)]*stage/4 << endl;
#endif
                }
            }
        }

#ifdef DEBUG_EVAL
        cout << "BLACK KNIGHT STATIC: " << pos;
        cout << "  [ " << -posval_knight[mirror(pos)] << " ]" << endl;
#endif
    }

    return kscore;
}

// --------------------------------------------------------------
//  Turm Evaluierung
// --------------------------------------------------------------
int Eval::rook_eval(const int posval_rook[], 
                    const PTTe* const p_pawn_info, int stage)
{
    int rscore = 0;

    // -----------------------------
    //  weisse Türme
    // -----------------------------
    int rooks_on_7 = 0;
    Bitboard set = p_board->whiteRooks;
    while (set)
    {
        int pos = set.msb();
        set ^= BBTables::bbmask[pos];

        // statische Evaluierung
        rscore += posval_rook[pos];

        // Abstand zum gegnerischen König
        int d = BBTables::dist[pos][p_board->blackKingPos];
        if (d <= 3) {
            if (stage != ENDGAME)
               rscore += (4-d)*ROOK_KING_ATTACK;
            else
               rscore += (4-d)*(ROOK_KING_ATTACK/2);
        }


        // Turm auf 7. Reihe?
        if (ROW(pos) == 1)
            rooks_on_7++;
            
        // TURM vor eigenen Bauern?
        if (ROW(pos) <= 5) {
           if (p_board->whitePawns & BBTables::bbmask[pos+8]) {
              rscore -= ROOK_FRONT_PAWN_PENALTY;
           }
        }


#ifdef DEBUG_EVAL
        cout << "WHITE ROOK STATIC: " << pos;
        cout << "  [ " << posval_rook[pos] << " ]" << endl;
#endif

        // (Halb-)offene Linien
        int c = COL(pos);

        if (p_pawn_info->isHalfOpenWhite(c))
        {
            if (p_pawn_info->isHalfOpenBlack(c))
            {
                // Offene Linie
                rscore += ROOK_ON_OPEN_FILE;


#ifdef DEBUG_EVAL
                cout << "WHITE ROOK ON OPEN FILE: " << c << endl;
#endif
            }
            else
            {
                // Halboffene Linie
                rscore += ROOK_ON_HALFOPEN_FILE;


#ifdef DEBUG_EVAL
                cout << "WHITE ROOK ON HALFOPEN FILE: " << c << endl;
#endif
            }
        }

        // 2 Türme in 7. Reihe?
        if (rooks_on_7 == 2)
            rscore += ROOKS_ON_7;

        // Beweglichkeit
        Bitboard all(p_board->whitePieces|p_board->blackPieces);
        int mobx = BBTables::rank_mobility[pos][ all.GET_RANK(pos) ];

        int moby = BBTables::file_mobility[pos][ p_board->allPieces90C.GET_RANK(rot90C_bitIndex[pos]) ];

        rscore += (mobx+moby); 

        // Eingesperrter Turm?
        if (mobx+moby <= 1)
        {
            if (mobx+moby == 0)
                rscore -= FENCED_ROOK_PENALTY;
            else
                rscore -= FENCED_ROOK_PENALTY>>1;
        }


#ifdef DEBUG_EVAL
        cout << "WHITE ROOK-MOBILITY: RX = " << mobx << " -- RY = " << moby << endl;
#endif

    }

    // -----------------------------
    //  schwarze Türme
    // -----------------------------
    rooks_on_7 = 0;
    set = p_board->blackRooks;
    while (set)
    {
        int pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        rscore -= posval_rook[mirror(pos)];

        // Abstand zum gegnerischen König
        int d = BBTables::dist[pos][p_board->whiteKingPos];
        if (d <= 3)
        {
            if (stage != ENDGAME) {
               rscore -= (4-d)*ROOK_KING_ATTACK;
            } else {
               rscore -= (4-d)*(ROOK_KING_ATTACK/2);
            }
        }


        // Turm auf 7. Reihe?
        if (ROW(pos) == 6)
            rooks_on_7++;

        // TURM vor eigenen Bauern?
        if (ROW(pos) >= 2) {
           if (p_board->blackPawns & BBTables::bbmask[pos-8]) {
              rscore += ROOK_FRONT_PAWN_PENALTY;
           }
        }


#ifdef DEBUG_EVAL
        cout << "BLACK ROOK STATIC: " << pos;
        cout << "  [ " << -posval_rook[mirror(pos)] << " ]" << endl;
#endif
        // (Halb-)offene Linien
        int c = COL(pos);

        if (p_pawn_info->isHalfOpenBlack(c))
        {
            if (p_pawn_info->isHalfOpenWhite(c))
            {
                // Offene Linie
                rscore -= ROOK_ON_OPEN_FILE;


#ifdef DEBUG_EVAL
                cout << "BLACK ROOK ON OPEN FILE: " << c << endl;
#endif
            }
            else
            {
                // Halboffene Linie
                rscore -= ROOK_ON_HALFOPEN_FILE;


#ifdef DEBUG_EVAL
                cout << "BLACK ROOK ON HALFOPEN FILE: " << c << endl;
#endif
            }
        }

        // 2 Türme in 7. Reihe?
        if (rooks_on_7 == 2)
            rscore -= ROOKS_ON_7;


        // Beweglichkeit
        Bitboard all(p_board->whitePieces|p_board->blackPieces);
        int mobx = BBTables::rank_mobility[pos][ all.GET_RANK(pos) ];

        int moby = BBTables::file_mobility[pos][ p_board->allPieces90C.GET_RANK(rot90C_bitIndex[pos]) ];

        rscore -= (mobx+moby); 


        // Eingesperrter Turm?
        if (mobx+moby <= 1)
        {
            if (mobx+moby == 0)
                rscore += FENCED_ROOK_PENALTY;
            else
                rscore += FENCED_ROOK_PENALTY>>1;
        }




#ifdef DEBUG_EVAL
        cout << "BLACK ROOK-MOBILITY: RX = " << mobx << " -- RY = " << moby << endl;
#endif
    }


    return rscore;
}


// --------------------------------------------------------------
//  Damen Evaluierung
// --------------------------------------------------------------
int Eval::queen_eval(int stage)
{
    int qscore = 0;
    // w e i s s e  D a m e n
    Bitboard set = p_board->whiteQueens;
    while (set)
    {
        int pos = set.msb();
        set ^= BBTables::bbmask[pos];

        // Abstand zum schwarzen König
       
        int d = BBTables::dist[pos][p_board->blackKingPos];
        if (d <= 3)
        {
            if (stage != ENDGAME) {
               qscore += (4-d)*(4-d)*QUEEN_KING_ATTACK;
            } else {
               qscore += (4-d)*(QUEEN_KING_ATTACK/2);
            }
        }
    }

    // Dame + Turm auf 7.Reihe?
    if ((BBTables::rankmask[1] & p_board->whiteQueens) && 
        (BBTables::rankmask[1] & p_board->whiteRooks) )
            qscore += QUEEN_ROOK_ON_7;

    // s c h w a r z e  D a m e n
    set = p_board->blackQueens;
    while (set)
    {
        int pos = set.lsb();
        set ^= BBTables::bbmask[pos];

        // Abstand zum schwarzen König
        int d = BBTables::dist[pos][p_board->whiteKingPos];
        if (d <= 3)
        {
            if (stage != ENDGAME) {
               qscore -= (4-d)*(4-d)*QUEEN_KING_ATTACK;
            } else {
               qscore -= (4-d)*(QUEEN_KING_ATTACK/2);
            }
        }
    }

    // Dame + Turm auf 7.Reihe?
    if ((BBTables::rankmask[6] & p_board->blackQueens) && 
        (BBTables::rankmask[6] & p_board->blackRooks) )
            qscore -= QUEEN_ROOK_ON_7;


#ifdef DEBUG_EVAL
    cout << "QUEEN EVAL: " << qscore << endl;
#endif

    return qscore;
}



// --------------------------------------------------------------
//  Königs Evaluierung
// --------------------------------------------------------------
int Eval::king_eval(const int posval_king[], int stage)
{
    int kscore = 0;

    // w e i s s e r  K ö n i g

    // statische Evaluierung
    kscore += posval_king[p_board->whiteKingPos];


#ifdef DEBUG_EVAL
    cout << "WHITE KING STATIC: " << p_board->whiteKingPos;
    cout << "  [ " << posval_king[p_board->whiteKingPos] << " ]" << endl;
#endif

   // König im Endspiel
   if (stage == ENDGAME) {
      // König auf Grundreihe
      if (p_board->whiteKing & BBTables::rankmask[7]) {
         // König eingeklemmt
         Bitboard all(p_board->whitePieces|p_board->blackPieces);
         if ( (~all & BBTables::king_attacks[p_board->whiteKingPos]) == 0) {
            kscore -= FENCED_KING_ENDG_PENALTY;
         }
      }
   }


    // s c h w a r z e r  K ö n i g

    // statische Evaluierung
    kscore -= posval_king[mirror(p_board->blackKingPos)];


#ifdef DEBUG_EVAL
    cout << "BLACK KING STATIC: " << p_board->blackKingPos;
    cout << "  [ " << -posval_king[mirror(p_board->blackKingPos)] << " ]" << endl;
#endif

   // König im Endspiel
   if (stage == ENDGAME) {
      // König auf Grundreihe
      if (p_board->blackKing & BBTables::rankmask[0]) {
         // König eingeklemmt
         Bitboard all(p_board->whitePieces|p_board->blackPieces);
         if ( (~all & BBTables::king_attacks[p_board->blackKingPos]) == 0) {
            kscore += FENCED_KING_ENDG_PENALTY;
         }
      }
   }

    return kscore;
}

// --------------------------------------------------------------
// Sicherheit der Königsstellung
// --------------------------------------------------------------
int Eval::king_safety_score(const PTTe * const p_pawn_info)
{
    int ks_w_score = 0;
    int ks_b_score = 0;
    int c;

    // Falls Könige im feindlichen Lager sind: Aufgeben ;)
    if (p_board->whiteKingPos < 16 || p_board->blackKingPos > 47)
        return 0;

    // -----------------------------------
    //      w e i s s e r  K ö n i g
    // -----------------------------------

    
    // offene Linien?
    int pos = p_board->whiteKingPos;
    c = COL(pos);
    if ((p_board->stats.nBlackPieces[ROOK] || p_board->stats.nBlackPieces[QUEEN] )
        && p_pawn_info->isHalfOpenWhite(c))
    {
        if (p_pawn_info->isHalfOpenBlack(c))
        {
            ks_w_score -= KING_ON_OPEN_FILE;
        }
        else
        {
            ks_w_score -= KING_ON_HALFOPEN_FILE;
        }
#ifdef DEBUG_EVAL
        cout << "WHITE KING ON OPEN FILE: " << c << endl;
#endif
    }

    // Rochademöglichkeiten?
    // Strafe, falls König Rochade rechte verloren hat
    if (!p_board->whiteKingCastled)
    {
        if ( ! ( p_board->castle & (CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE)) )
        {
            ks_w_score -= NO_CASTLE_PENALTY;


#ifdef DEBUG_EVAL
            cout << "NO CASTLE PENALTY WHITE" << endl;
#endif
        }
    }

    // Stärke Bauern Schutzwall?
    int safety = 0;
    if (BBTables::bbmask[pos - 8] & p_board->whitePawns)
        safety += 4;
    else
        if (BBTables::bbmask[pos - 16] & p_board->whitePawns)
            safety += 2;
    if (c > 1)
        if (BBTables::bbmask[pos - 9] & p_board->whitePawns)
            safety += 2;
        else if (BBTables::bbmask[pos - 17] & p_board->whitePawns)
            safety++;
    if (c < 7)
        if (BBTables::bbmask[pos - 7] & p_board->whitePawns)
            safety +=2;
        else if (BBTables::bbmask[pos - 15] & p_board->whitePawns)
            safety++;

#ifdef DEBUG_EVAL
    cout << "SAFETY PAWNWALL WHITE KING: " << safety << endl;
#endif
    if (safety <= 3)
    {
        ks_w_score -= CRITICAL_PAWN_WALL;
    }
    else if (safety <= 5)
    {
        ks_w_score -= BAD_PAWN_WALL;
    }
    else if (safety <= 6)
    {
        ks_w_score -= MEDIUM_PAWN_WALL;
    }
    else if (safety <= 7)
    {
        ks_w_score -= GOOD_PAWN_WALL;
    }

    // Falls keine gegnerische Schwerfigur vorhanden: runterskalieren
    if (!p_board->stats.nBlackPieces[QUEEN]) {
       if (!p_board->stats.nBlackPieces[ROOK]) {
          ks_w_score /= 4;
       } else {
          ks_w_score /= 2;
       }
    }

    // -------------------------------------
    //      s c h w a r z e r  K ö n i g
    // -------------------------------------
    pos = p_board->blackKingPos;
    c = COL(pos);
    if ((p_board->stats.nWhitePieces[ROOK] || p_board->stats.nWhitePieces[QUEEN])
         && p_pawn_info->isHalfOpenBlack(c))
    {
        if (p_pawn_info->isHalfOpenWhite(c))
        {
            ks_b_score += KING_ON_OPEN_FILE;
        }
        else
        {
            ks_b_score += KING_ON_HALFOPEN_FILE;
        }

#ifdef DEBUG_EVAL
        cout << "BLACK KING ON OPEN FILE: " << c << endl;
#endif
    }


    // Rochademöglichkeiten?
    // Strafe, falls König Rochade rechte verloren hat
    if (!p_board->blackKingCastled)
    {
        if ( ! ( p_board->castle & (CASTLE_BLACK_KINGSIDE | CASTLE_BLACK_QUEENSIDE)) )
        {
            ks_b_score += NO_CASTLE_PENALTY;


#ifdef DEBUG_EVAL
            cout << "NO CASTLE PENALTY BLACK" << endl;
#endif
        }
    }


    // Stärke Bauern Schutzwall?
    safety = 0;
    if (BBTables::bbmask[pos + 8] & p_board->blackPawns)
        safety += 4;
    else
        if (BBTables::bbmask[pos + 16] & p_board->blackPawns)
            safety += 2;
    if (c > 1)
        if (BBTables::bbmask[pos + 7] & p_board->blackPawns)
            safety += 2;
        else if (BBTables::bbmask[pos + 15] & p_board->blackPawns)
            safety++;
    if (c < 7)
        if (BBTables::bbmask[pos + 9] & p_board->blackPawns)
            safety +=2;
        else if (BBTables::bbmask[pos + 17] & p_board->blackPawns)
            safety++;

#ifdef DEBUG_EVAL
    cout << "SAFETY PAWNWALL BLACK KING: " << safety << endl;
#endif
    if (safety <= 3)
    {
        ks_b_score += CRITICAL_PAWN_WALL;
    }
    else if (safety <= 5)
    {
        ks_b_score += BAD_PAWN_WALL;
    }
    else if (safety <= 6)
    {
        ks_b_score += MEDIUM_PAWN_WALL;
    }
    else if (safety <= 7)
    {
        ks_b_score += GOOD_PAWN_WALL;
    }

    // Falls keine gegnerische Schwerfigur vorhanden: runterskalieren
    if (!p_board->stats.nWhitePieces[QUEEN]) {
       if (!p_board->stats.nWhitePieces[ROOK]) {
          ks_b_score /= 4;
       } else {
          ks_b_score /= 2;
       }
    }

#ifdef DEBUG_EVAL
    cout << "TOTAL KING SAFETY SCORE WHITE = " << ks_w_score << endl;
    cout << "TOTAL KING SAFETY SCORE BLACK = " << ks_b_score << endl;
#endif
    return ks_w_score + ks_b_score;
}



// --------------------------------------------------------------
//              e v a l _ o p
// --------------------------------------------------------------
//
//  Evaluierungsfunktion für die Eröffnung
// --------------------------------------------------------------
int Eval::eval_op(int alpha, int beta)
{
    int score = 0;

    // --------------------------
    // Materialscore berechnen
    // --------------------------
    score = p_board->stats.whiteMatScore;
#ifdef DEBUG_EVAL
    cout << "MATERIAL-SCORE = " << score << endl;
#endif

    // Materialverteilung?
    score += bad_trades();

#ifdef DEBUG_EVAL
    cout << "TRADES = " << bad_trades() << endl;
#endif
    
    // --------------------------
    //  Bauern Score
    // --------------------------

    PTTe* p_pawn_info;
    if (!(p_pawn_info = p_pawnTT->lookup(
                           p_board->get_zp2_hash(), 
                           OPENING, 
                           p_board->get_zp1_hash())))
        pawn_eval(p_pawn_info,OPENING,posval_pawn_op);
    score += p_pawn_info->score;

    // --------------------------
    //  Läufer Score
    // --------------------------
    score += bishop_eval(posval_bishop_op_emid, OPENING);

    // --------------------------
    //  Königs Score
    // --------------------------
    score += king_eval(posval_king_op,OPENING);
    score += king_safety_score(p_pawn_info);

    // --------------------------
    //  Lazy Exit?
    // --------------------------
    int lazy_score = (p_board->sideToMove() == WHITE) ? score : -score;
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        nLazyExits++;

#ifndef DEBUG_LAZYEVAL
        return lazy_score;
#endif
    }


    // --------------------------
    //  Springer Score
    // --------------------------
    score += knight_eval(posval_knight_op_emid, p_pawn_info, OPENING);

    // --------------------------
    //  Turm Score
    // --------------------------
    score += rook_eval(posval_rook_op, p_pawn_info, OPENING);

    // --------------------------
    // Damen-Score
    // --------------------------
    score += queen_eval(OPENING);


    // --------------------------
    //  Lazy Evaluation debuggen
    // --------------------------
#ifdef DEBUG_LAZYEVAL
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        // Lazy Eval hätte stattgefunden

        // Wäre es gerechtfertigt?
        int sc = (p_board->sideToMove() == WHITE) ? score : -score;
        if (sc > alpha && sc < beta)
        {
            // Fehler
            nWrongLazyExits++;
        }
    }
#endif


    if (p_board->sideToMove() == WHITE)
        return score;
    else 
        return -score;
}

// --------------------------------------------------------------
//              e v a l _ e m i d
// --------------------------------------------------------------
//
//  Evaluierungsfunktion für frühes Mittelspiel
// --------------------------------------------------------------
int Eval::eval_emid(int alpha, int beta)
{
    int score = 0;

    // --------------------------
    // Materialscore berechnen
    // --------------------------
    score = p_board->stats.whiteMatScore;

#ifdef DEBUG_EVAL
    cout << "MATERIAL-SCORE = " << score << endl;
#endif

    // Materialverteilung?
    score += bad_trades();

#ifdef DEBUG_EVAL
    cout << "TRADES = " << bad_trades() << endl;
#endif

    // --------------------------
    //  Bauern Score
    // --------------------------

    PTTe* p_pawn_info;
    if (!(p_pawn_info = p_pawnTT->lookup(p_board->get_zp2_hash(), EARLY_MIDGAME, p_board->get_zp1_hash())))
        pawn_eval(p_pawn_info,EARLY_MIDGAME,posval_pawn_emid);
    score += p_pawn_info->score;

    // --------------------------
    //  Läufer Score
    // --------------------------
    score += bishop_eval(posval_bishop_op_emid, EARLY_MIDGAME);

    // --------------------------
    //  Königs Score
    // --------------------------
    score += king_eval(posval_king_emid,EARLY_MIDGAME);
    score += king_safety_score(p_pawn_info);

    // --------------------------
    //  Lazy Exit?
    // --------------------------
    int lazy_score = (p_board->sideToMove() == WHITE) ? score : -score;
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        nLazyExits++;

#ifndef DEBUG_LAZYEVAL
        return lazy_score;
#endif
    }

    // --------------------------
    //  Springer Score
    // --------------------------
    score += knight_eval(posval_knight_op_emid, p_pawn_info, EARLY_MIDGAME);

    // --------------------------
    //  Turm Score
    // --------------------------
    score += rook_eval(posval_rook_emid, p_pawn_info, EARLY_MIDGAME);

    // --------------------------
    // Damen-Score
    // --------------------------
    score += queen_eval(EARLY_MIDGAME);

    // --------------------------
    //  Lazy Evaluation debuggen
    // --------------------------
#ifdef DEBUG_LAZYEVAL
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        // Lazy Eval hätte stattgefunden

        // Wäre es gerechtfertigt?
        int sc = (p_board->sideToMove() == WHITE) ? score : -score;
        if (sc > alpha && sc < beta)
        {
            // Fehler
            nWrongLazyExits++;
        }
    }
#endif


    if (p_board->sideToMove() == WHITE)
        return score;
    else 
        return -score;
}

// --------------------------------------------------------------
//              e v a l _ l m i d 
// --------------------------------------------------------------
//
//  Evaluierungsfunktion für spätes Mittelspiel
// --------------------------------------------------------------
int Eval::eval_lmid(int alpha, int beta)
{
    int score = 0;

    // --------------------------
    // Materialscore berechnen
    // --------------------------
    score = p_board->stats.whiteMatScore;

#ifdef DEBUG_EVAL
    cout << "MATERIAL-SCORE = " << score << endl;
#endif

    // Materialverteilung?
    score += bad_trades();

#ifdef DEBUG_EVAL
    cout << "TRADES = " << bad_trades() << endl;
#endif


    // --------------------------
    //  Bauern Score
    // --------------------------

    PTTe* p_pawn_info;
    if (!(p_pawn_info = p_pawnTT->lookup(p_board->get_zp2_hash(), LATE_MIDGAME, p_board->get_zp1_hash())))
        pawn_eval(p_pawn_info,LATE_MIDGAME,posval_pawn_lmid);
    score += p_pawn_info->score;

    // --------------------------
    //  Läufer Score
    // --------------------------
    score += bishop_eval(posval_bishop_lmid_end, LATE_MIDGAME);


    // --------------------------
    //  Lazy Exit?
    // --------------------------
    int lazy_score = (p_board->sideToMove() == WHITE) ? score : -score;
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        nLazyExits++;

#ifndef DEBUG_LAZYEVAL
        return lazy_score;
#endif
    }

    // --------------------------
    //  Springer Score
    // --------------------------
    score += knight_eval(posval_knight_lmid_end, p_pawn_info, LATE_MIDGAME);

    // --------------------------
    //  Turm Score
    // --------------------------
    score += rook_eval(posval_rook_lmid, p_pawn_info, LATE_MIDGAME);

    // --------------------------
    //  Königs Score
    // --------------------------
    score += king_eval(posval_king_lmid,LATE_MIDGAME);
    
    int ks_score = king_safety_score(p_pawn_info);
    score += ks_score/2;

    // --------------------------
    // Damen-Score
    // --------------------------
    score += queen_eval(LATE_MIDGAME);

    // --------------------------
    //  Lazy Evaluation debuggen
    // --------------------------
#ifdef DEBUG_LAZYEVAL
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        // Lazy Eval hätte stattgefunden

        // Wäre es gerechtfertigt?
        int sc = (p_board->sideToMove() == WHITE) ? score : -score;
        if (sc > alpha && sc < beta)
        {
            // Fehler
            nWrongLazyExits++;
        }
    }
#endif


    if (p_board->sideToMove() == WHITE)
        return score;
    else 
        return -score;
}

// --------------------------------------------------------------
//              e v a l _ e n d
// --------------------------------------------------------------
//
//  Evaluierungsfunktion für das Endspiel
// --------------------------------------------------------------
int Eval::eval_end(int alpha, int beta)
{
    RecogTable::Recognizer* p_rec;
    int low_b = -INF_SCORE;
    int upp_b = +INF_SCORE;
    if (p_board->stats.nWhitePiecesTotal <= 4 && p_board->stats.nBlackPiecesTotal <= 4 && 
        (p_rec=RecogTable::getHandle()->lookup(p_board->stats.matWhite, p_board->stats.matBlack))
       )
    {
        RecogTable::RecogResult res;

        if ( (res=p_rec->execute(p_board)) ) // != FAILE
        {
            if (res == RecogTable::EXACT)
            {
                return p_rec->getScore();
            }
            else if (res == RecogTable::LBOUND)
            {
                low_b = p_rec->getScore();
                if (low_b >= beta)
                    return low_b;
            }
            else if (res == RecogTable::UBOUND)
            {
                upp_b = p_rec->getScore();
                if (upp_b <= alpha)
                    return upp_b;
            }

        }
    } 

    int score = 0;

    // --------------------------
    // Materialscore berechnen
    // --------------------------
    score = p_board->stats.whiteMatScore;

    // Spezielle Materialverteilungen

    // R vs B  oder  R vs N
    int mat = p_board->stats.matWhite|p_board->stats.matBlack;
    if (mat == 13 || mat == 11)
    {
        if ( (p_board->stats.matWhite&30) == 8  && p_board->stats.nWhitePieces[ROOK] == 1 &&
             (((p_board->stats.matBlack&30) == 4  && p_board->stats.nBlackPieces[BISHOP] == 1) ||
              (p_board->stats.matBlack&30) == 2  && p_board->stats.nBlackPieces[KNIGHT] == 1))
        {
            score += ROOK_VS_MINOR;
        }
        else if ( (p_board->stats.matBlack&30) == 8  && p_board->stats.nBlackPieces[ROOK] == 1 &&
             (((p_board->stats.matWhite&30) == 4  && p_board->stats.nWhitePieces[BISHOP] == 1) ||
              (p_board->stats.matWhite&30) == 2  && p_board->stats.nWhitePieces[KNIGHT] == 1))
        {
            score -= ROOK_VS_MINOR;
        }
    }


#ifdef DEBUG_EVAL
    cout << "MATERIAL-SCORE = " << score << endl;
#endif

    // --------------------------
    //  Bauern Score
    // --------------------------
    PTTe* p_pawn_info;

    if (!(p_pawn_info = p_pawnTT->lookup(p_board->get_zp2_hash(), ENDGAME, 
        p_board->get_zp1_hash()))) {
        pawn_eval(p_pawn_info,ENDGAME,posval_pawn_end);
    }
    score += p_pawn_info->score;
    score += uncatchable_pawn_eval(*p_pawn_info);

    // --------------------------
    //  Läufer Score
    // --------------------------
    if (mat&4)
        score += bishop_eval(posval_bishop_lmid_end, ENDGAME);

    // -----------------------------
    // Stellung tendiert zum Remis?
    // -----------------------------
    bool is_drawish = drawish();


    // --------------------------------------------------
    // Falls es einen grossen (Material-)Vorsprung gibt,
    // ermuntere ggfalls zum Damentausch
    // --------------------------------------------------

    if (abs(score) >= 400)
    {
        if (score >= 400)
        {
            if (p_board->stats.matBlack & 16) // Dame?
                score -= QUEEN_DEFENDER;
        }
        else
        {
            if (p_board->stats.matWhite & 16) // Dame?
                score += QUEEN_DEFENDER;
        }
    }


    // --------------------------
    //  Lazy Exit?
    // --------------------------
    int lazy_score = (p_board->sideToMove() == WHITE) ? score : -score;
    if (lazy_score > upp_b)         lazy_score = upp_b;
    else if (lazy_score < low_b)    lazy_score = low_b;
    if (!is_drawish && (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta))
    {
        nLazyExits++;

#ifndef DEBUG_LAZYEVAL
        return lazy_score;
#endif
    }

    // --------------------------
    //  Springer Score
    // --------------------------
    if (mat&2)
        score += knight_eval(posval_knight_lmid_end, p_pawn_info, ENDGAME);

    // --------------------------
    //  Turm Score
    // --------------------------
    if (mat&8)
        score += rook_eval(posval_rook_end, p_pawn_info, ENDGAME);

    // --------------------------
    //  Königs Score
    // --------------------------
    score += king_eval(posval_king_end,ENDGAME);

    // --------------------------
    //  Lazy Evaluation debuggen
    // --------------------------
#ifdef DEBUG_LAZYEVAL
    if (lazy_score + LAZY_EXIT_OFFSET <= alpha  ||
        lazy_score - LAZY_EXIT_OFFSET >= beta)
    {
        // Lazy Eval hätte stattgefunden

        // Wäre es gerechtfertigt?
        int sc = (p_board->sideToMove() == WHITE) ? score : -score;
        if (sc > alpha && sc < beta)
        {
            // Fehler
            nWrongLazyExits++;
        }
    }
#endif

    if (is_drawish)
        score /= 16;

    if (p_board->sideToMove() == BLACK)
        score = -score;

    if (score < low_b) return low_b;
    if (score > upp_b) return upp_b;


    return score;
}

// --------------------------------------------------------------
//  Freibauern finden, die nicht mehr gestoppt werden können
// --------------------------------------------------------------
int Eval::uncatchable_pawn_eval(const PTTe& pawn_info)
{
    // Prüfen, ob überhaupt Freibauern vorhanden sind
    unsigned int w_passed = pawn_info.getWPassed();
    unsigned int b_passed = pawn_info.getBPassed();

    if (!w_passed && !b_passed)
        return 0;

    if (p_board->stats.matBlack > 1 && p_board->stats.matWhite > 1)
        return 0;

    // Wieviel Schritte zur Damenumwandlung?
    int w_to_queen = 100;
    int b_to_queen = 100;

    // ---------------------------------------------------
    // Wenn Schwarz kein Material hat, weisse Freibauern
    // untersuchen
    // ---------------------------------------------------

    if (p_board->stats.matBlack <= 1)
    {
        while (w_passed)
        {
            int c = BBTables::lsb16[w_passed];
            w_passed ^= 1<<c;

            Bitboard temp(p_board->whitePawns & BBTables::filemask[c]);
            if (!temp)
            {
                // Moeglichen Hash-Fehler abfangen
                return 0;
            }
            int pawn_pos = temp.lsb();
            int target   = COL(pawn_pos);

            // Alles bis zum Umwandlungsfeld frei?
            if ( 
                 (BBTables::squaresBetween[pawn_pos][target] | 
                  BBTables::bbmask[target]) & 
                 (p_board->whitePieces | p_board->blackPieces)
               )
                continue;

            int steps = ROW(pawn_pos);
            if (p_board->sideToMove() == WHITE)
            {
                if (BBTables::dist[p_board->blackKingPos][target] > steps)
                {
                    if (steps < w_to_queen)
                        w_to_queen = 2*steps - 1;
                }
            }
            else
            {
                if (BBTables::dist[p_board->blackKingPos][target] > steps + 1)
                {
                    if (steps < w_to_queen)
                        w_to_queen = 2*steps;
                }
            }
        }
    }
    // ---------------------------------------------------
    // Wenn Weiss kein Material hat, schwarze Freibauern
    // untersuchen
    // ---------------------------------------------------
    if (p_board->stats.matWhite <= 1)
    {
        while (b_passed)
        {
            int c = BBTables::lsb16[b_passed];
            b_passed ^= 1<<c;

            Bitboard temp(p_board->blackPawns & BBTables::filemask[c]);
            if (!temp)
            {
                // Hash-Fehler?
                return 0;
            }
            int pawn_pos = temp.msb();
            int target   = COL(pawn_pos) + 56;

            // Alles bis zum Umwandlungsfeld frei?
            if ( 
                 (BBTables::squaresBetween[pawn_pos][target] | 
                  BBTables::bbmask[target]) & 
                 (p_board->whitePieces | p_board->blackPieces)
               )
                continue;

            int steps = 7-ROW(pawn_pos);
            if (p_board->sideToMove() == BLACK)
            {
                if (BBTables::dist[p_board->whiteKingPos][target] > steps)
                {
                    if (steps < b_to_queen)
                        b_to_queen = 2*steps-1;
                }
            }
            else
            {
                if (BBTables::dist[p_board->whiteKingPos][target] > steps + 1)
                {
                    if (steps < b_to_queen)
                        b_to_queen = steps;
                }
            }
        }
    }


    if (w_to_queen==100 && b_to_queen==100)
        return 0;

#ifdef DEBUG_EVAL
    cout << "PAWN - RACE" << endl;
    cout << "  w_to_queen == " << w_to_queen << endl;
    cout << "  b_to_queen == " << b_to_queen << endl;
#endif

    if (b_to_queen == 100)
        return piece_val[QUEEN] - 2*piece_val[PAWN] - 5*w_to_queen;

    if (w_to_queen == 100)
        return -(piece_val[QUEEN] - 2*piece_val[PAWN] - 5*b_to_queen);

    if (w_to_queen + 2 < b_to_queen)
        return piece_val[QUEEN] - 2*piece_val[PAWN] - 5*w_to_queen;

    if (b_to_queen + 2 < w_to_queen)
        return -(piece_val[QUEEN] - 2*piece_val[PAWN] - 5*b_to_queen);

    return 0;
}


// --------------------------------------------------------------
//  Erkennen von Endspielstellungen, die wahrscheinlich Remis
//  sind
// --------------------------------------------------------------

bool Eval::drawish()
{
    // ----------------------------------------------------------------
    // Turm-Bauer und falscher Läufer kann (meistens) nicht gewinnen
    // ----------------------------------------------------------------
    if (p_board->stats.matWhite == 5 && p_board->stats.nWhitePieces[PAWN]==1)
    {
        if (p_board->stats.whiteMatScore > 0)
        {
            // Turmbauer?
            if ((p_board->whitePawns & (BBTables::filemask[0]|BBTables::filemask[7])) )
            {
                // Falscher Läufer?
                bool is_a_pawn = (p_board->whitePawns&BBTables::filemask[0]) != 0;
                bool is_white_bishop = (p_board->whiteBishops & BBTables::whiteSquares) != 0;

                if ( ( is_a_pawn && !is_white_bishop) ||
                     (!is_a_pawn &&  is_white_bishop))
                {
                    // Drawish; den Rest muss die Suche erledigen :)
                    return true;
                }
            }
        }
    }
    if (p_board->stats.matBlack == 5 && p_board->stats.nBlackPieces[PAWN]==1)
    {
        if (p_board->stats.whiteMatScore < 0)
        {
            // Turmbauer?
            if ((p_board->blackPawns & (BBTables::filemask[0]|BBTables::filemask[7])) )
            {
                // Falscher Läufer?
                bool is_a_pawn = (p_board->blackPawns&BBTables::filemask[0]) != 0;
                bool is_white_bishop = (p_board->blackBishops & BBTables::whiteSquares) != 0;

                if ( ( is_a_pawn &&  is_white_bishop) ||
                     (!is_a_pawn && !is_white_bishop))
                {
                    // Drawish; den Rest muss die Suche erledigen :)
                    return true;
                }
            }
        }
    }

    // Nie, wenn Bauern vorhanden:
    if ((p_board->stats.matBlack | p_board->stats.matWhite) & 1)
        return false;

    // Notwendig: Material <= Val(Bishop)
    if (abs(p_board->stats.whiteMatScore) <= BISHOP_MATERIAL)
    {
        // Ab hier normalerweise Remis; es gibt aber Ausnahmen:
        switch (p_board->stats.nBlackPiecesTotal + p_board->stats.nWhitePiecesTotal)
        {
        case 5:
            // KQ x KBB  v  KQ x KBN  v  KQ x KNN
            if (p_board->stats.matBlack == 16 && p_board->stats.matWhite <= 6)
                return false;
            if (p_board->stats.matWhite == 16 && p_board->stats.matBlack <= 6)
                return false;

            // KBB x KN
            if (p_board->stats.nBlackPieces[BISHOP] == 2 && p_board->stats.matWhite == 2)
                return false;
            if (p_board->stats.nWhitePieces[BISHOP] == 2 && p_board->stats.matBlack == 2)
                return false;
            break;

        }

        return true;
    }

    return false;
}



// --------------------------------------------------------------
//  bad_trades ... Erkennen ungünstiger Materialverteilungen
// --------------------------------------------------------------
inline int Eval::bad_trades()
{
    int n_whiteminor = p_board->stats.nWhitePieces[BISHOP] + p_board->stats.nWhitePieces[KNIGHT];
    int n_blackminor = p_board->stats.nBlackPieces[BISHOP] + p_board->stats.nBlackPieces[KNIGHT];

    if (p_board->stats.nWhitePieces[ROOK] != p_board->stats.nBlackPieces[ROOK])
    {
        if (p_board->stats.nWhitePieces[ROOK] == p_board->stats.nBlackPieces[ROOK] + 1)
        {
            // Weiss mit einem Turm mehr:
            if (n_whiteminor + 1 == n_blackminor)  // Weiss eine Leichtf. weniger
                return BAD_TRADE;
            if (n_whiteminor + 2 == n_blackminor)  // Weiss zwei Leichtf. weniger
                return -BAD_TRADE;
            return 0;
        }
        if (p_board->stats.nWhitePieces[ROOK] + 1 == p_board->stats.nBlackPieces[ROOK])
        {
            // Schwarz mit einem Turm mehr:
            if (n_blackminor + 1 == n_whiteminor) // Schwarz eine Leichtf. weniger
                return -BAD_TRADE;
            if (n_blackminor + 2 == n_whiteminor) // Schwarz zwei Leichtf. weniger
                return +BAD_TRADE;
            return 0;
        }
    }
    else if (n_whiteminor != n_blackminor)
    {
        if (n_whiteminor == n_blackminor + 1)
        {
            // Weiss mit einer Leichtfigur mehr
            return BAD_TRADE;
        }
        if (n_whiteminor + 1 == n_blackminor)
        {
            // Schwarz mit einer Leichtfigur mehr
            return -BAD_TRADE;
        }
    }

    return 0;
}

// ------------------------------------------------------------------------
//  Eval::showEval
// ------------------------------------------------------------------------
void Eval::showEval() {
   // Aktuelle Spielphase bestimmen
   int stage = getStage();
   int score;
   int pscore;
   int bishop_score;
   int knight_score;
   int rook_score;
   int queen_score;
   int king_score;
   int king_saf_score;
   
   PTTe* p_pawn_info;
   
   switch (stage) {
   case OPENING:
      cout << ".................OPENING EVALUATION" << endl;
      score          = eval_op(-INF_SCORE,+INF_SCORE);
      pawn_eval(p_pawn_info,OPENING, posval_pawn_op);
      pscore         = p_pawn_info->score;
      bishop_score   = bishop_eval( posval_bishop_op_emid, OPENING);
      knight_score   = knight_eval( posval_knight_op_emid, p_pawn_info, 
                                   OPENING);
      rook_score     = rook_eval(posval_rook_op, p_pawn_info, OPENING);
      queen_score    = queen_eval( OPENING);
      king_score     = king_eval(posval_king_op, OPENING);
      king_saf_score = king_safety_score(p_pawn_info);
      break;
   case EARLY_MIDGAME:
      cout << ".................EARLY MIDGAME EVALUATION" << endl;
      score          =  eval_emid(-INF_SCORE,+INF_SCORE);
      pawn_eval(p_pawn_info,EARLY_MIDGAME, posval_pawn_emid);
      pscore         = p_pawn_info->score;
      bishop_score   = bishop_eval(posval_bishop_op_emid, EARLY_MIDGAME);
      knight_score   = knight_eval(posval_knight_op_emid, p_pawn_info, 
                       EARLY_MIDGAME);
      rook_score     = rook_eval(posval_rook_emid, p_pawn_info, 
                                 EARLY_MIDGAME);
      queen_score    = queen_eval(EARLY_MIDGAME);
      king_score     = king_eval(posval_king_emid,EARLY_MIDGAME);
      king_saf_score = king_safety_score(p_pawn_info);
      break;
   case LATE_MIDGAME:
      cout << ".................LATE MIDGAME EVALUATION" << endl;
      score          = eval_lmid(-INF_SCORE,+INF_SCORE);
      pawn_eval(p_pawn_info,LATE_MIDGAME, posval_pawn_lmid);
      pscore         = p_pawn_info->score;
      bishop_score   = bishop_eval(posval_bishop_lmid_end, LATE_MIDGAME);
      knight_score   = knight_eval(posval_knight_lmid_end, p_pawn_info, 
                       LATE_MIDGAME);
      rook_score     = rook_eval(posval_rook_lmid, p_pawn_info, 
                                 LATE_MIDGAME);
      queen_score    = queen_eval(LATE_MIDGAME);
      king_score     = king_eval(posval_king_lmid,LATE_MIDGAME);
      king_saf_score = king_safety_score(p_pawn_info);
      break;
   default:
      cout << ".................ENDGAME EVALUATION" << endl;
      score          = eval_end(-INF_SCORE,+INF_SCORE);
      pawn_eval(p_pawn_info,ENDGAME, posval_pawn_end);
      pscore         = p_pawn_info->score;
      pscore        += uncatchable_pawn_eval(*p_pawn_info);
      bishop_score   = bishop_eval(posval_bishop_lmid_end, ENDGAME);
      knight_score   = knight_eval(posval_knight_lmid_end, p_pawn_info, 
                                   ENDGAME);
      rook_score     = rook_eval(posval_rook_end, p_pawn_info, ENDGAME);
      queen_score    = queen_eval(ENDGAME);
      king_score     = king_eval(posval_king_end,ENDGAME);
      king_saf_score = 0;
      break;
   }

   // Score nicht relativ anzeigen
   if (p_board->sideToMove() == BLACK)
      score = -score;

   cout << endl;
   cout << "Overall score : " << score << endl;
   cout << endl;
   cout << "Material score: " << p_board->stats.whiteMatScore << endl;
   cout << "Pawn score    : " << pscore << endl;
   cout << "Bishop score  : " << bishop_score << endl;
   cout << "Knight score  : " << knight_score << endl;
   cout << "Rook score    : " << rook_score << endl;
   cout << "Queen score   : " << queen_score << endl;
   cout << "King score    : " << king_score << endl;
   cout << "King Safety   : " << king_saf_score << endl;
   cout << "Other         : " << score -(p_board->stats.whiteMatScore +
                                 pscore + bishop_score + knight_score +
                                 rook_score + queen_score + king_score +
                                 king_saf_score) << endl;
   cout << endl;
}



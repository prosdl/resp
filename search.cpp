// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : search.cpp
//                       Suchen (und Finden?) von guten Zügen
//
//  Anfang des Projekts: So, 5.August, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: search.cpp,v 1.119 2003/06/01 13:47:29 rosendahl Exp $
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

#include "search.h"
#include "eval.h"
#include "notation.h"
#include "basic_stuff.h"
#include "recognizer.h"
#include "incmovgen.h"
#include "move.h"
#include "respoptions.h"
#include "hash.h"
#include <vector>
#include <time.h> 
#include <iostream> 
#include <iomanip>


using namespace std; 



// -------------------------------------------------------------------------
//  Konstruktor
// -------------------------------------------------------------------------
Search::Search(Board* pbd)
{
    pBoard = pbd;

    options.do_nullmoves  = true;
    options.do_extensions = true;
    options.no_post       = false;
    options.do_ef_prune   = true; 

    got_ponder_move = false;

    tm_call_mask = 0x7fff;

    p_TT = Hash::getHandle();

#ifdef TRACE_SEARCH
    tracelog.open("trace.log",ios::out | ios::trunc);
#endif
}

Search::~Search()
{
#ifdef TRACE_SEARCH
    tracelog.close();
#endif
}

// -------------------------------------------------------------------------
//    perft: baue einen Suchbaum mit Tiefe 'depth' auf
// -------------------------------------------------------------------------

int Search::perft(int depth, int ply)
{
   if (! depth ) {
      nNodes++;
      return 0;
   }

   bool check = pBoard->kingAttacked(pBoard->sideToMove());

   if (check)
      pBoard->genKingEvasions(ply);
   else
      pBoard->gen(ply);

   MoveStack* pMS = pBoard->pCurMoveStack;
   for (int i=0; i < pMS->size(); i++) {
      Move m = pMS->stack[i];
      pBoard->makemove(m);
      if (!check && !pBoard->positionLegalNoCheck(m)) {
         pBoard->takebackmove();
         continue;
      }

      perft(depth-1,ply+1);
      pBoard->takebackmove();
   }

   return 0;
}

// -------------------------------------------------------------------------
//                  Züge sortieren QUIESCENCE
// -------------------------------------------------------------------------
void Search::calc_move_score_caps(int ply)
{
    MoveStack& ms = pBoard->moveStack[ply];

    for (int i=0; i < ms.size(); i++)
    {
        const Move& m = ms.stack[i];
        if (m.isPromotion())
        {
            if (m.getPromPiece() == QUEEN)
                ms.score[i] = 0x1000000; // >mvv_lva(...)
            else
                ms.score[i] = 0;
        }
        else
            ms.score[i] = m.mvv_lva(); 

        //ms.score[i] = ms.mat_gain[i] = pBoard->see(m); 
   }
}

// -------------------------------------------------------------------------
// try_null ... feststellen, ob Null-Move in gegenwaertiger Position 
//              sinnvoll ist
// -------------------------------------------------------------------------
bool Search::try_null(const int beta, const int frac_depth, int& null_red)
{
    const int NM_MARGIN = 200;

    bool wtm = pBoard->sideToMove()==WHITE;
    int max_pieces = (wtm) ? pBoard->stats.nWhitePiecesTotal - pBoard->stats.nWhitePieces[PAWN]:
                             pBoard->stats.nBlackPiecesTotal - pBoard->stats.nBlackPieces[PAWN];

    if (pBoard->getMatScore() >= beta - NM_MARGIN  && 
        max_pieces >= 2 )
    {
        null_red = (frac_depth >= 6*EXT_PLY_1) ? 3*EXT_PLY_1 : 2*EXT_PLY_1;
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------
//                      n e g a s c o u t
// -------------------------------------------------------------------------
int Search::negascout(int alpha, int beta, int frac_depth, int ply, bool do_nullmove, bool in_check)
{
    // Anzahl Knoten
    nNodes++;

#ifdef TRACE_SEARCH
    tracelog << "D" << frac_depth << "P" << ply << " ... ";
    tracelog << "alphabeta()        Nodes = " << nNodes  + nQNodes << endl;
    tracelog << " - alpha = " << alpha << ", beta = " << beta;
    tracelog << ", do_nullmove = " << do_nullmove << endl;
#endif

    if ( ! ((nNodes + nQNodes) & tm_call_mask) )
    {
        if (time_up())
        {
            stop_now = true;
            return beta;
        }
    } 


    // -------------------------------------------------------------------------
    //  Informationen über aktuelle Position sammeln: Pos.wdh, Schach?
    // -------------------------------------------------------------------------

    //pv_length[ply] = 0; 

    // Remis durch Zugwiederholung
    if (pBoard->repititions())
    {
        setEvalMark(ply);
        return  0;
    } 




    // -------------------------------------------------------------------------
    //              Nachschauen, ob Position im Transposition-Table 
    // -------------------------------------------------------------------------
    TTe info(pBoard->get_z2_hash(), frac_depth, pBoard->sideToMove(), pBoard->getCastle(),
             pBoard->getEp());


    info.m = 0;
    bool tt_hit = false;
 
#ifdef USE_TRANSPOSITION_TABLE
    
    if ( p_TT->lookup(info, pBoard->get_z1_hash()))
    {
        tt_hit = true;
        nHashEntriesFound++; // Statistik

#ifdef TRACE_SEARCH
        tracelog << "USING TT!" << endl;
        tracelog << " - info.stype=" << (int) info.getType() << ", info.score=" << info.score;
        tracelog << ", info.m=" << info.m << ", info.depth=" << (int) info.getDepth()<<  endl;
#endif

#ifdef DEBUG_HASH
        // Hash debuggen: Prüfen, ob der im Hash gespeicherte Zug 
        //                plausibel ist

        if (!hashmove_ok(info.m,ply))
        {
            nWrongHashmove++;
        }
#endif


        if (info.getType() != TTe::DONT_USE)  // Score nicht benutzen
        {
            switch (info.getType())
            {
            case TTe::EXACT:          // Exakte Score    
                // Matt Score anpassen?
                if (info.score < -INF_SCORE + 100)
                    info.score += ply;
                else if (info.score > +INF_SCORE - 100)
                {
                    info.score -= ply;
                }
                saveBestLine(ply,info.m,HASH);
                setEvalMark(ply);
                return info.score;
            case TTe::LBOUND:
                if (info.score >= beta)                  // Fail high
                {
                    return info.score;
                }
                if (info.score > alpha)
                {
                    alpha = info.score-1; // um 1 verkleinert für bessere PV-Ausgaben
                }
                break;
            case TTe::UBOUND:
                if (info.score <= alpha)                 // Fail low
                {
                    return info.score; 
                }
                if (info.score < beta)
                {
                    beta = info.score+1; // um 1 vergroessert für bessere PV-Ausgaben
                }
                break;

            }  
        }
    }

#endif // USE_TRANSPOSITION_TABLE



    // -------------------------------------------------------------------------
    //          Nachschauen, ob Recognizer verfuegbar ist
    // -------------------------------------------------------------------------

    bool using_recognizer = false;
#define USE_RECOGNIZER 
#ifdef USE_RECOGNIZER
    RecogTable::Recognizer* p_rec;
    if (!tt_hit && 
        pBoard->stats.nWhitePiecesTotal <= 4 && pBoard->stats.nBlackPiecesTotal <= 4 && 
        (p_rec=RecogTable::getHandle()->lookup(pBoard->stats.matWhite, pBoard->stats.matBlack)))
    {
        RecogTable::RecogResult res;

        if ( (res=p_rec->execute(pBoard)) ) // != FAIL
        {
            using_recognizer = true;
            if (res == RecogTable::EXACT)
            {
                return p_rec->getScore();
            }
            else if (res == RecogTable::LBOUND)
            {
                if (p_rec->getScore() >= beta)
                    return p_rec->getScore();
                if (p_rec->getScore() > alpha)
                    alpha = p_rec->getScore();
            }
            else // res == Recognizer::UBOUND
            {
                if (p_rec->getScore() <= alpha)
                    return p_rec->getScore();
                if (p_rec->getScore() < beta)
                    beta = p_rec->getScore();
            }
        }
    }
#endif

    // -------------------------------------------------------------------------
    //                              Null-Move
    // -------------------------------------------------------------------------
    //  Testen, ob in der aktuellen Position ein Fail High erzeugt werden kann,
    //  indem die aktuelle Seite einmal aussetzt, und der Gegner nochmal
    //  ziehen darf
    //  (Basierend auf "Recursive Adaptive Null-Move Pruning" Ernst A. Heinz)

    // Zunächst prüfen, ob Bedingungen für einen Null Move erfüllt sind:
    //  1.) Kein Schach
    //  2.) Der vorhergehende Zug war kein Null Move
    //  3.) Geringe Zugszwanggefahr, d.h. die Seite, die nicht zieht, hat 
    //      ausreichend Material

    int base_extensions = 0;
    int null_red;

    if (getOptNullMoves() && !in_check && !do_nullmove && try_null(beta,frac_depth,null_red)) 
    {
        pBoard->makemove(Move::nullmove());

        int nullmove_score;
        if (frac_depth - null_red >= EXT_PLY_2)
            nullmove_score = -negascout(-beta,-beta + 1,frac_depth - EXT_PLY_1 - null_red, 
                                         ply+1, true,false);
        else
            nullmove_score = -quiescence(ply+1, -beta,-beta + 1);

        pBoard->takebackmove();

        
        if (nullmove_score >= beta)
        {
#ifdef TRACE_SEARCH
    tracelog << "CUTOFF durch nullmove" << endl;
    tracelog << " - alpha = " << alpha << ", beta = " << beta << ", frac_depth = " << frac_depth;
    tracelog << ", ply = " << ply << ", nullmove_score = " << nullmove_score << endl;
#endif
            
            // Im TT speichern
            info.setSpecial(frac_depth,pBoard->sideToMove(), pBoard->getCastle(), pBoard->getEp(),
                            pBoard->getAge(), TTe::LBOUND);
            info.lock   = pBoard->get_z2_hash();
            info.m      = 0;
            info.score  = nullmove_score;
            p_TT->insert(info, pBoard->get_z1_hash(), rootage);

            //setLastMove(ply,Move::nullmove(),FHIGH);

            return nullmove_score;
        }


        // Extension, falls Mattdrohung (nach Bruce Moreland)
        if (nullmove_score <= -INF_SCORE + 500)
        {
            nMateThreadExt++;
            base_extensions += MATE_THREAD_EXTENSION_VAL;
        }
    } 


    // -------------------------------
    //   Züge generieren
    // -------------------------------

    //IncMoveGen img(pBoard,ply,info.m,in_check,iter_depth);
    IncMoveGen img(ply,info.m,iter_depth,in_check);

    if (in_check)
    {
        img.next(); // Nur Zuege generieren
        if (pBoard->moveStack[ply].size() == 1)
        {
            base_extensions += ONE_MOVE_EXTENSIONS_VAL;
            nOneMoveExt++;
        } 
    }

    MoveStack& mvst = pBoard->moveStack[ply];

    // best_score erhält später den alphabeta Wert für den 1. ausgeführten Zug;
    // solange alpha nicht verbessert wird, bleibt
    // best_score eine obere Schranke für die tatsächliche Score
    int best_score = -INF_SCORE;
    Move best_move = 0;
    int htype = TTe::UBOUND;
    int v;  
    Move m;
    int legal_moves = 0;

    // -------------------------------------------------------
    // Pruning/Razoring
    // Basierend auf Ernst A. Heinz "Extended Futility 
    // Pruning"
    // Im Fall von resp ist:
    // -ein FRONTIER_NODE        :   frac_depth < EXT_PLY_2
    // -ein PRE_FRONTIER_NODE    :   frac_depth < EXT_PLY_3
    // -ein PRE_PRE_FRONTIER_NODE:   frac_depth < EXT_PLY_4
    //
    //  [frac_depth/EXT_PLY_1] == "Abstand zur QSuche"]
    // -------------------------------------------------------
    bool allow_prune = false;
    int delta = 0;

    if (options.do_ef_prune  &&  
        root_beta != INF_SCORE &&
        !base_extensions && !in_check && !using_recognizer &&
        frac_depth < EXT_PLY_4 && 
        alpha < INF_SCORE - 100 && alpha > -INF_SCORE + 100)
    {
        int mat_score = pBoard->getMatScore();
        delta = alpha - mat_score - RAZOR_MARGIN;
        if (delta >= 0)
            base_extensions -= EXT_PLY_1;

        if (frac_depth + base_extensions < EXT_PLY_2) 
        {
            delta = alpha - mat_score - FUTIL_MARGIN;
            if (delta >= 0)
            {
                best_score = mat_score + FUTIL_MARGIN;
                allow_prune = true;
            }
        }
        else if (frac_depth + base_extensions < EXT_PLY_3)
        {
            delta = alpha - mat_score - EXTD_FUTIL_MARGIN;
            if (delta >= 0)
            {
                best_score = mat_score + EXTD_FUTIL_MARGIN;
                allow_prune = true;
            }
        } 
    }

    bool first = true;

    
    // -----------------------------------------------
    //          Z ü g e  i t e r i e r e n
    // -----------------------------------------------
    //for (m = img.next(); m.data != 0; m = img.next())
    for (m = img.next(); m.data != 0; m = img.next())
    {
#ifdef TRACE_SEARCH
        int m_seeval = pBoard->see(m);
#endif
        // -----------------------------------------------
        // Besten Zug nach Position i bringen
        // -----------------------------------------------
        pBoard->makemove(m);

        // nur legale Zuege verwenden
        if (!in_check && !pBoard->positionLegalNoCheck(m)) {
           pBoard->takebackmove();
           continue;
        }
        /*if (!in_check && !pBoard->positionLegal() )
        {
            pBoard->takebackmove();
            continue;
        }*/
        legal_moves++; 


        // -----------------------------------------------
        // Extensions
        // -----------------------------------------------

        // Schach Extension
        int extensions = 0;
        bool checking_move = pBoard->inCheck();
        if (checking_move)
        {
            nCheckExt++;
            extensions += CHECK_EXTENSION_VAL;
        }

        //  Recapture Extension 
        if (m.isCapture())
        {
            Move mLast = pBoard->history.getMove(pBoard->getTotalHPly() - 1);
            if (mLast.isCapture() && mLast.to() == m.to()  &&
                piece_val[m.getCapturedPiece()] == piece_val[mLast.getCapturedPiece()])
            {
                nExchangeExt++;
                extensions +=EXCHANGE_EXTENSION_VAL;
            }
        }

        if (m.isPawnMove())
        {
            //  Pawn Push Extension -- nur falls Bauer auf 7. (2.) Reihe
            if  (ROW(m.to())==1 || ROW(m.to())==6) 
            {
                nPromotionExt++;
                extensions += PAWN_PROMOTION_EXTENSION_VAL; 
            }

            // Passed Pawn Extension
            if (pBoard->sideToMove() == WHITE)
            {
                if ((BBTables::w_ppawn_mask[m.from()] & pBoard->blackPawns) == 0)
                {
                    extensions += (pBoard->stats.matWhite <= 3) ? 4 : 3;
                }
            }
            else
            {
                if ((BBTables::b_ppawn_mask[m.from()] & pBoard->whitePawns) == 0)
                {
                    extensions += (pBoard->stats.matBlack <= 3) ? 4 : 3;
                }
            }
        }



        // ------------------------------------------------------
        // Pruning
        // ------------------------------------------------------
        if (allow_prune && !checking_move  && !m.isPromotion())
        {
            if (piece_val[m.getCapturedPiece()] <=  delta)
            {
                nExFulPrune++;
                pBoard->takebackmove();
                continue; 
            }
        }

        // -----------------------------------------------
        // Extensionwert berechnen
        // -----------------------------------------------
        int ext_value = base_extensions + extensions;

        if (ply > STOP_EXTENSIONS) ext_value = 0;           // Notbremse
        if (ext_value > EXT_PLY_1) ext_value = EXT_PLY_1;
        if (ext_value > 0 && ply > iter_depth*2) 
            ext_value >>= 1;

        if (!getOptExtensions())
            ext_value = 0;



#ifdef TRACE_SEARCH
        tracelog << "D" << frac_depth << "P" << ply << " ... ";
        tracelog << "makemove............" << m;
        tracelog << " [SEE=" << m_seeval <<"]";
        tracelog << " Legal: " << legal_moves << endl;
#endif



        // -----------------------------------------------
        //      Negascout-Suche anwenden (A.Reinefeld)
        // -----------------------------------------------

        if (first) 
        {
            // "Ordentlich" Suchen:
            if (frac_depth + ext_value >= EXT_PLY_2)
                v = -negascout(-beta, -alpha, frac_depth + ext_value - EXT_PLY_1, ply+1, 
                                false,checking_move);
            else
                v = -quiescence(ply+1,-beta, -alpha);
        }
        else
        {
            // Mit minimalem Fenster suchen:
            // die Suche muss mit score <= alpha v score > alpha enden:
            //
            // (a) score > alpha: 
            //     Die Suche konnte nicht widerlegen, dass der ausgeführte Zug
            //     m zu einer Scoreverbesserung führt. Es gab ein Fail Low in
            //     der Suche und es gilt:
            //
            //               minimax(Kindknoten) >= score > alpha
            //
            //     Es muss nochmal neu im Fenster [score, beta] gesucht werden
            //
            // (b) score <= alpha: 
            //     In der Suche kam es zu einem Beta Cutoff und es gilt:
            //
            //               minimax(Kindknoten) <= score <= alpha
            //     
            //     D.h. score kann unser alpha ohnehin nicht verbessern.
            //
            if (frac_depth + ext_value >= EXT_PLY_2)
                v = -negascout(-alpha-1, -alpha, frac_depth + ext_value - EXT_PLY_1, ply+1, 
                                false,checking_move);
            else
                v = -quiescence(ply+1, -alpha-1,-alpha);

            // Zeit ausgegangen?
            if (stop_now)
            {
                pBoard->takebackmove();
                break;
            }

            if (v > alpha && v < beta)  // konnte nicht widerlegen ...
            {
                // Mit erweiterten Fensterwerten suchen 
                if (frac_depth + ext_value >= EXT_PLY_2)
                    v = -negascout(-beta, -alpha, frac_depth + ext_value - EXT_PLY_1, ply+1, 
                                    false,checking_move);
                else
                    v = -quiescence(ply+1, -beta, -alpha);
            }

        }

        pBoard->takebackmove();


#ifdef TRACE_SEARCH
        tracelog << "D" << frac_depth << "P" << ply << " ... ";
        tracelog << "alphabeta for " << m;
        tracelog << " Legal Moves : "<< legal_moves << "/"<< v << endl;
#endif


        // Zeit ausgegangen?
        if (stop_now)
            break;

        // Bester Zug?
        if (v > best_score)
        {
            first = false;

            best_score = v; 
            best_move  = m;


            if (best_score > alpha)
            {
                // fail high?
                if (best_score >= beta)
                {

#ifdef TRACE_SEARCH
                    tracelog << "FAIL HIGH!" << endl;
                    tracelog << "D" << frac_depth << "P" << ply << " ... ";
                    tracelog << " - v = " << v << ", beta = " << beta << endl;
#endif

                    // Beta Cutoff ... wir haben nur eine untere Grenze
                    // für den Wert der aktuellen Position
                    htype = TTe::LBOUND;

                    nFaileHigh++;
                    if (legal_moves == 1)
                        nFaileHighOnFirst++;

                    break;
                }

                // die Score wird exakt (es sei denn, es kommt noch zu einem
                // Fail High)
                htype = TTe::EXACT;

                // Score merken
                alpha = best_score;


#ifdef TRACE_SEARCH
                tracelog << "ALPHA VERBESSERT!" << endl;
                tracelog << "D" << frac_depth << "P" << ply << " ... ";
                tracelog << " - v = " << v << ", beta = " << beta;
                tracelog << ", m = " << m << endl;
                tracelog << "pc[ " << ply << "] = ";
                for (int i=0; i < frac_depth/EXT_PLY_1; i++) 
                    tracelog << pv[ply][ply+i].m << " ";
                tracelog << endl;
#endif
            }

        }
    }


    if (stop_now)
        return beta;


    if (!legal_moves)  // kein legaler Zug gefunden: Schachmatt oder Patt!
    {
        setEvalMark(ply);
        if (pBoard->kingAttacked(pBoard->sideToMove()))
            return   -INF_SCORE + ply; // Matt
        else
            return   0;               // Patt
    }

    info.score  = best_score;
    info.m      = best_move;

    if (htype == TTe::LBOUND)
    {
        // History Heuristik aktualisieren
        pBoard->histHeuristic.add(info.m, frac_depth*frac_depth >> 8); 
        mvst.addKiller(info.m); 
#ifdef TRACE_SEARCH
        tracelog << " ADDING KILLER " << info.m << ", ply = " << ply << endl;
#endif

        if (info.score > INF_SCORE - 100)
            info.score = INF_SCORE - 100;  
    }
    else if (htype == TTe::UBOUND)
    {
        if (info.score < -INF_SCORE + 100)
            info.score = -INF_SCORE + 100; 
        info.m = 0;
    }
    else  // htype == TTe::EXACT
    {
        // History Heuristik aktualisieren
        pBoard->histHeuristic.add(best_move, frac_depth*frac_depth >> 8); 

        if (info.score < -INF_SCORE + 100)
            info.score -= ply;
        else if (info.score > INF_SCORE - 100)
            info.score += ply; 

        // pc aktualisieren
        saveBestLine(ply,info.m,NORMAL);

    }




#ifdef USE_TRANSPOSITION_TABLE

    info.lock   = pBoard->get_z2_hash();

    info.setSpecial(frac_depth,pBoard->sideToMove(), pBoard->getCastle(), pBoard->getEp(),
                    pBoard->getAge(),htype);

    p_TT->insert(info, pBoard->get_z1_hash(), rootage);

#endif // USE_TRANSPOSITION_TABLE

#ifdef TRACE_SEARCH
    tracelog << " RETURN: v= " << v << ", ply = " << ply << endl;
#endif


    return best_score;

}


// -------------------------------------------------------------------------
//     r o o t S e a r c h  ...  Suche bei ply == 0
// -------------------------------------------------------------------------

int Search::rootSearch(int alpha, int beta, int depth, std::vector<Move>* p_epd_move_list, 
                      long &epd_time, bool epd_bm, long epd_start_time)

{

    nNodes++;
    
#ifdef TRACE_SEARCH
    tracelog << "D" << depth*16 << "P" << 0 << " ... ";
    tracelog << "alphabeta()        Nodes = " << nNodes  + nQNodes << endl;
    tracelog << " - alpha = " << alpha << ", beta = " << beta;
    tracelog << ", rootSearch! " << endl;
#endif
    
    bool in_check = pBoard->inCheck();

    // -------------------------------
    //   Züge generieren
    // -------------------------------
    int base_extensions = 0;

    if (in_check)
    { 
        pBoard->genKingEvasions(0);


        // Nur ein Zug möglich?
        if (pBoard->moveStack[0].size() == 1)
        {
            base_extensions += ONE_MOVE_EXTENSIONS_VAL;
            nOneMoveExt++;
        } 
    }
    else
    {
        if (!pBoard->gen(0))
            return -ILLEGAL_MOVE_SCORE;
    }
 

    // ----------------------------------
    //   PV-Zug mit höchster Score
    //   versehen
    // ----------------------------------

    MoveStack& mvst = pBoard->moveStack[0];

    if (iter_depth > 1) {
       for (int k = 0; k < mvst.size(); k++) {
           if (mvst.stack[k] == pv[0][0].m)
               nNodesRootMove[k] = 100000000;
       }
    }


    int best_score  = alpha;        // beste Score bisher
    int legal_moves = 0;            // Anzahl legaler Zuege
    bool first_move = true;         // Erster Zug?
    int v;  
    Move m;

    // den Index des Zuges innerhalb der Zugliste im Feld move_idx
    // sichern
    int move_idx[MAX_MOVESTACK];
    for (int idx = 0; idx < mvst.size(); idx++)
        move_idx[idx] = idx; 
 
    // -----------------------------------------------
    //          Z ü g e  i t e r i e r e n
    // -----------------------------------------------
    for (int i=0; i < mvst.size(); i++)
    {
        // -----------------------------------------------
        //  Besten Zug nach Position i bringen
        // -----------------------------------------------

        long maxNodes = nNodesRootMove[move_idx[i]];
        int  max_idx  = i;
        for (int x = i + 1; x < mvst.size(); x++)
            if (nNodesRootMove[move_idx[x]] > maxNodes)
            {
                maxNodes = nNodesRootMove[move_idx[x]];
                max_idx  = x;
            }
        mvst.swap(i,max_idx);
        // Liste mit Zugindizes wg. der Vertauschung aktualisieren
        int idx_temp      = move_idx[i];
        move_idx[i]       = move_idx[max_idx];
        move_idx[max_idx] = idx_temp;
 
        m = mvst.stack[i];

        // ------------------
        //  Zug ausfuehren
        // ------------------
        long nNodesLast = nNodes + nQNodes;
        pBoard->makemove(m);


        // nur legale Zuege verwenden
        if (!pBoard->positionLegal() )
        {
            pBoard->takebackmove();
            continue;
        }



/*        // TEST TTTT
         if (iter_depth > 6)
            cout << "Move ... " << i << "/" << mvst.size() << ": " << m << 
            "   [" << alpha << ", " << beta << "]" << endl; 
*/
        // -----------------------------------------------
        // Extensions
        // -----------------------------------------------

        // Schach Extension
        int extensions = 0;
        bool checking_move = pBoard->inCheck();
        if (checking_move)
        {
            nCheckExt++;
            extensions += CHECK_EXTENSION_VAL;
        }


        //  Pawn Push Extension -- nur Falls Umwandlung
        if (m.isPromotion()) 
        {
            nPromotionExt++;
            extensions += PAWN_PROMOTION_EXTENSION_VAL;
        }


        // -----------------------------------------------
        // Extensionwert berechnen
        // -----------------------------------------------
        int ext_value = (base_extensions + extensions); 

        if (!getOptExtensions())
            ext_value = 0;



#ifdef TRACE_SEARCH
        tracelog << "D" << depth << "P0" << " ... ";
        tracelog << "makemove............" << m << "(i=" << i;
        tracelog << ")" << endl;
#endif


        // -----------------------------------------------
        //      Negascout-Suche aufrufen
        // -----------------------------------------------


        if ((depth-1)*EXT_PLY_1 + ext_value >= EXT_PLY_1)
            v = -negascout(-beta, -alpha, (depth-1)*EXT_PLY_1 + ext_value, 1, false,checking_move);
        else
            v = -quiescence(1, -beta, -alpha);
    
        // Zeit ausgegangen?
        if (stop_now)
        {
            pBoard->takebackmove();
            break;
        }

        if (!first_move && v > alpha)  // konnte nicht widerlegen ...
        {
            // Mit erweiterten Fensterwerten suchen 
            if ((depth-1)*EXT_PLY_1 + ext_value >= EXT_PLY_1)
                v = -negascout(-root_beta, -alpha, (depth-1)*EXT_PLY_1 + ext_value, 1, false,checking_move);
            else
                v = -quiescence(1, -root_beta, -alpha);
        }

        pBoard->takebackmove();
        nNodesRootMove[move_idx[i]] = nNodes + nQNodes - nNodesLast;
        legal_moves++;


#ifdef TRACE_SEARCH
        tracelog << "D" << depth << "P" << 0 << " ... ";
        tracelog << "alphabeta for " << m << "(i=" << i;
        tracelog << "): " << v << endl;
#endif

        // Zeit ausgegangen?
        if (stop_now)
            break;

 
        if (v < root_alpha && first_move) // Fail low ---
        {
            pv[0][0].m   = m;
            pv_length[0] = 1;
            return v;
        }


        // Bester Zug?
        if (v > alpha)
        {
            alpha = best_score = v; 

            // pc aktualisieren
            saveBestLine(0,m,NORMAL);

            // Falls Testlauf: Lösungszeit aktualisieren
            updateEpdTime(epd_time,epd_start_time,p_epd_move_list,epd_bm);


            if (v >= root_beta)      // Fail High +++
            {
                pBoard->histHeuristic.add(m, depth*depth); 

                pv_length[0] = 1;   

#ifdef TRACE_SEARCH
                tracelog << "FAIL HIGH!" << endl;
                tracelog << "D" << depth << "P0"  << " ... ";
                tracelog << " - v = " << v << ", beta = " << beta << endl;
#endif
                return v;
             }


#ifdef TRACE_SEARCH
                tracelog << "ALPHA VERBESSERT!" << endl;
                tracelog << "D" << depth << "P" << 0 << " ... ";
                tracelog << " - v = " << v << ", beta = " << beta;
                tracelog << ", m = " << m << endl;
                tracelog << "pc[ " << 0 << "] = ";
                for (int i=0; i < depth; i++) 
                    tracelog << pv[0][i].m << " ";
                tracelog << endl;
#endif

            // An der Wurzel ... Kritischen Pfad ausgeben
            dump_pc_san(iter_depth,v,false);
        }


        beta       = alpha + 1;
        first_move = false;
    }


    if (stop_now)
        return beta;


    if (!legal_moves)  // kein legaler Zug gefunden: Schachmatt oder Patt!
    {
        setEvalMark(0);
        if (pBoard->kingAttacked(pBoard->sideToMove()))
            return   -INF_SCORE;         // Matt
        else
            return   0;                 // Patt
    }

    if (best_score <= root_alpha)
    {
        pv_length[0] = 1;
    }


    return best_score;

}



// -------------------------------------------------------------------------
//  Quiescence search... Negamax mit Capture Zügen
// -------------------------------------------------------------------------

int Search::quiescence(int ply, int alpha, int beta)
{
    nQNodes++;

    pv_length[ply] = 0;

    if ( ! ((nNodes + nQNodes) & tm_call_mask) )
    {
        if (time_up())
        {
            stop_now = true;
            return beta;
        }
    }

    setEvalMark(ply);
    int score = Eval::getHandle()->eval(alpha, beta);


    if (score >= beta)
        return score;

    if (score > alpha)
    {
        alpha = score;
        setEvalMark(ply);
    }

    int delta = alpha  - 125 - pBoard->getMatScore(); 

    pBoard->genCaps(ply);
    calc_move_score_caps(ply);  

    // Wird ein Zug entlang der Hauptvarianten untersucht?
    MoveStack& mvst = pBoard->moveStack[ply];


    Move best_move = 0;
    // für alle Züge
    for (int i=0; i < mvst.size(); i++)
    {
        swapWithBestScoringMove(ply,i); 

        Move m (mvst.stack[i]);

        if (piece_val[m.getCapturedPiece()] < delta && !m.isPromotion())
            continue;

        mvst.mat_gain[i] = pBoard->see(m); 


        // Pruning
        if (mvst.mat_gain[i] < 0 || mvst.mat_gain[i] < delta)
            continue;   

        // führe Zug aus
        pBoard->makemove(m);

        // nur legale Zuege verwenden
        if (pBoard->kingAttacked(XSIDE(pBoard->sideToMove())) )
        {
            pBoard->takebackmove();
            continue;
        }

        // Suche ausgehend von neuer Position
        score = -quiescence(ply+1, -beta, -alpha);

        // nimm Zug zurück
        pBoard->takebackmove();


        // Bester Zug?
        if (score > alpha)
        {
            if (score >= beta)
            {
                mvst.addKiller(m);
                return score;
            }

            // Score merken
            alpha = score;
            best_move = m;
        }
    }

    // pc aktualisieren
    if (score >= alpha && !(best_move == 0))
    {
        saveBestLine(ply,best_move,QUIES);
    }

    return alpha;
}

// -------------------------------------------------------------------------
//  Iterative Deepening
//   
//   max_depth: maximal bis zu dieser Tiefe iterieren
//   epd_move : != 0 ==> Testlauf mit epd_move als besten (schlechtesten)
//                       Zug
//   epd_time : benötigte Zeit um Lösung zu finden und beizubehalten
//              (-1 für nicht gelöst)
//   epd_bm   : == true  => best  move
//              == false => avoid move         
// -------------------------------------------------------------------------

int Search::iter_deep(int max_depth, std::vector<Move>* p_epd_move_list, 
                      long &epd_time, bool epd_bm)
{
    int best_sc = 0;

    const int ITER_WINDOW = 45;

    root_alpha = -INF_SCORE;
    root_beta  =  INF_SCORE;

    last_pv_score = +INF_SCORE;

    // Für Test suites: Benötigte Zeit berechnen
    long epd_start_time = time_in_ms();

    // Marke auf "nicht geloest"
    epd_time = -1;


    for (int i=0; i < MAX_MOVESTACK; i++)
        nNodesRootMove[i] = 0;

    // Suchtiefe schrittweise erhöhen
    for (iter_depth=1; iter_depth <= max_depth; iter_depth++)
    {
#ifdef TRACE_SEARCH
        tracelog << endl;
        tracelog << "---------------------------------------" << endl;
        tracelog << "**** iter_deep, depth =  " << iter_depth << endl;
        tracelog << "---------------------------------------" << endl;
#endif


        int sc = rootSearch(root_alpha, root_beta, iter_depth, p_epd_move_list, epd_time, epd_bm,
                 epd_start_time);

        // Falls Testlauf: Lösungszeit aktualisieren
        updateEpdTime(epd_time,epd_start_time,p_epd_move_list,epd_bm);

        // Zeit abgelaufen?
        if (stop_now)
            break;

        if (sc <= root_alpha || sc >= root_beta)    // Fenster zu klein?
        {
            if (sc <= root_alpha)
            {
                // Root Fail Low 
                dump_pc_san(iter_depth,sc,false,true,false);
                root_beta   = sc+1;
                root_alpha  = -INF_SCORE;
            }
            else if (sc >= root_beta)
            {
                // Root Fail High
                dump_pc_san(iter_depth,sc,false,false,true);
                root_beta   = +INF_SCORE;
                root_alpha  = sc-1;
            }

            sc = rootSearch(root_alpha, root_beta, iter_depth, p_epd_move_list, epd_time, epd_bm,
                epd_start_time);
            if (sc <= root_alpha || sc >= root_beta)
            {
                root_alpha = -INF_SCORE;
                root_beta  = +INF_SCORE;
                sc = rootSearch(root_alpha, root_beta, iter_depth, p_epd_move_list, epd_time, epd_bm,
                    epd_start_time);
            }
        }

        // Falls Testlauf: Lösungszeit aktualisieren
        updateEpdTime(epd_time,epd_start_time,p_epd_move_list,epd_bm);


        // Zeit abgelaufen?
        if (stop_now)
            break;


        best_sc = sc;

        // Matt gefunden?
        if (sc > INF_SCORE - 100 && INF_SCORE - sc < iter_depth)
        {
            dump_pc_san(iter_depth,sc,true);
            break;
        }


        // Suchfenster anpassen
        root_alpha    = sc - ITER_WINDOW;
        root_beta     = sc + ITER_WINDOW;


        dump_pc_san(iter_depth,sc,true);
    }

    if (!pondering)
    {
        // Ponder-Move eintragen
        ponder_move = pv[0][1].m;
        got_ponder_move = true;

        // Testen, ob ponder_move korrekt
        if (pBoard->makeSaveMove(pv[0][0].m))
        {
            if (pBoard->makeSaveMove(ponder_move))
                pBoard->takebackmove();
            else
            {
                // Nichts in pv gefunden... im Hash suchen
                ponder_move     = 0;
                got_ponder_move = false;
                TTe info(pBoard->get_z2_hash(), 500, pBoard->sideToMove(), pBoard->getCastle(),
                         pBoard->getEp());
                if (p_TT->lookup(info, pBoard->get_z1_hash()) )
                {
                    // Hashmove gefunden; auch legal?
                    if (pBoard->makeSaveMove(info.m))
                    {
                        pBoard->takebackmove();
                        ponder_move     = info.m;
                        got_ponder_move = true;
                    }
                } 
            }

            pBoard->takebackmove();
        }
        else
        {
            ponder_move     = 0;
            got_ponder_move = false;
        }
    }

    // Falls Testlauf: Lösungszeit aktualisieren
    updateEpdTime(epd_time,epd_start_time,p_epd_move_list,epd_bm);


    // Zum Zug pc[0][0] korrespondierende Score zurückgeben
    return best_sc;
}

// -------------------------------------------------------------------------
//  dump - Funktionen
// -------------------------------------------------------------------------

void Search::dump_pc(int depth, int score)
{
#ifdef USE_MMXASM
    clear_fpu();
#endif
    long now = time_in_ms();

    double diff = (now - start)/1000.0;

    cout << depth << " " << score << " " << (long)(100*diff);
    cout  << " " << nNodes << " ";

    for (int i=0; i< pv_length[0]; i++)
    {
        if (pv[0][i].m.data == END)
            break;
        if (pv[0][i].m.data == HASH)
        {
            cout << "HT";
            break;
        }
        cout << pv[0][i].m.toString() << " " ;
    }

    cout << endl;

}
 
// -------------------------------------------------------------------------
//  pc in SAN-Notation darstellen
// -------------------------------------------------------------------------
void Search::dump_pc_san(int depth, int score, bool depth_completed, bool aspir_low, bool aspir_high)
{
#ifdef USE_MMXASM
    clear_fpu();
#endif
    long now = time_in_ms();

    bool xboard = RespOptions::getHandle()->getValue("output.style") 
                  == "XBOARD";
    ostream* dump;

    dump = (xboard) ? &out.logfile() : static_cast<ostream*>(&out);

    double diff = (now - start)/1000.0;


    if (xboard && !options.no_post)
    {
        cout << depth << " " << score << " " << static_cast<long>(100*diff);
        cout  << " " << nNodes << " ";

      /*  if (aspir_low)
        {
            cout << "---" << endl; 
            return;
        }
        else if (aspir_high)
        {
            cout << "+++" << endl;
            return;
        } */
    }
    

    if (!xboard || out.getLogging())
    {
        *dump << setw(2) << right << depth;
        if (depth_completed)
            *dump << ".";
        else if (aspir_high)
            *dump << "+";
        else if (aspir_low)
            *dump << "-";
        else
            *dump << " ";

        *dump << setw(7) << fixed <<  setprecision(2) << diff 
              << setw(7) << setprecision(2) << score*0.01
              << setw(10) << nNodes + nQNodes;
        *dump <<  " ";
    }


    // WICHTIG! Alte Zugsortierung merken
    MoveStack save_ms = pBoard->moveStack[0];

    // Manchmal gibt es bei FEN-Positionen die Situation totalHPly == 0
    // (oder allgemeiner totalHPly gerade) und Schwarz am Zug.
    // Um Ausgaben der Form 1...e5 1.Nf3 usw. zu vermeiden passe ich
    // totalHPly an:
    int totalHPly = pBoard->getTotalHPly();
    if (pBoard->sideToMove() == WHITE && (totalHPly % 2) != 0)
        totalHPly--;
    else if (pBoard->sideToMove() == BLACK && (totalHPly % 2) == 0)
        totalHPly++;

    const int spos = 29;
    int pos = spos;         // Falls RESP-Format: Position in Zeile

    int n_moves = 0;
    bool in_hash = false;
    bool in_qsearch = false;

    pv[0][ pv_length[0] ].type = END;
    //pv[0][ pv_length[0] ].m = 0;
    //cout << " [ " << pv_length[0] << "] ";
    for (int i=0; i< MAXPLY; i++)
    {
        if (pv[0][i].type == END || pv[0][i].m == 0)
        {

            // Versuchen noch Züge aus dem Hash zu fischen ...
            TTe info(pBoard->get_z2_hash(), 100, pBoard->sideToMove(), pBoard->getCastle(),
                     pBoard->getEp());
            if (p_TT->lookup(info, pBoard->get_z1_hash()))
            {
                // Endlosschleifen durch Stellungswdh. verhindern!
                if (pBoard->repititions())
                {
                    //*dump << "{oo}";
                    break;
                }
                else
                {
                    pv[0][i].m = info.m;
                    pv[0][i].type = HASH;
                    pv[0][i+1].type = END;
                }
            } 
            else 
            {
                break;
            }
        } 


        // Durch Suchinstabilitäten können durchaus falsche Züge in pc
        // stehen: deshalb muss gesichert sein, dass der aktuelle Zug
        // pc[0][i] ein legaler Zug ist !!!!

        pBoard->gen(0);
        Move m = pv[0][i].m;

        Move gen_move = pBoard->moveStack[0].findMove(m.from(),m.to(),m.getPromPiece());

        if (gen_move == 0)   // Zug nicht in der Liste gefunden
        {
       /*     *dump << "~" << m << "/" << pv[0][i].type;
            if (m.isNullmove())
            {
                //if (xboard && !options.no_post)
                //    out << "0 (+)";
                *dump << "{n}";
            } */
            break;
        }

        // Zug in Liste gefunden, aber Binaerdarstellung verschieden?
        // (Kann auch vorkommen :)
        if (gen_move.data != pv[0][i].m.data)  
        {
            break;
        }

        // Zug führt ins Schach?
        pBoard->makemove(m);
        if (pBoard->kingAttacked(XSIDE(pBoard->sideToMove())))
        {
            pBoard->takebackmove();
            break;
        }
        pBoard->takebackmove();


        // Move --> SAN
        string san = Notation::move_to_san(pv[0][i].m,pBoard);

        // Modifikatoren für Hashtable, Quiescence, ...
        
        if (xboard && !options.no_post)
        {
            cout << san << " ";
        }

        if (!in_hash && (pv[0][i].type & HASH))
        {
            in_hash = true;
            //san = san + " {tt}";
        }
        if (!in_qsearch && (pv[0][i].type & QUIES))
        {
            in_qsearch = true;
            //san = san + " {q}";
        }
        if (pv[0][i].type & FHIGH)
            san = san + " {++}";
        if (pv[0][i].type & FLOW)
        {
            //san = san + " {LO}";
            *dump << "{--}";
            break;
        } 


        
        // Zugnummer bestimmen
        int totalPly = 1+ totalHPly / 2;

        string s;
        if (pBoard->sideToMove()==BLACK )
        {
            if (i == 0)
            {
                char buf[256];
                //s = itoa(totalPly,buf,10);
                sprintf(buf,"%i",totalPly);
                s = buf;
                s += "..." + san;
            }
            else
                s = san;
        }
        else
        {
            char buf[256];
            //s = itoa(totalPly,buf,10);
            sprintf(buf,"%i",totalPly);
            s = buf;
            s += "." + san;
        }

        // !! bei grossen (positiven) Spruengen in der Score, ! bei kleinen...

        if (i == 0)
        {
            if (score - last_pv_score > 150)
                s += "!!";
            else if (score - last_pv_score > 50)
                s +="!";
        }

        s += " ";

        if (s.length() + pos > 79)
        {
            // Zeilenvorschub
            if (!xboard || out.getLogging())
            {
                *dump << endl;
                *dump << setw(spos-1) << " ";
            }
            pos = spos;
        }

        if (!xboard || out.getLogging())
            *dump << s;

        pos += s.length();


        // Aspir.Window unterschritten: Keine weiteren Zuege ausgeben
        if (aspir_low)
        {
            //*dump << "---";
            break;
        }



        // Zug ausführen
        pBoard->makemove(pv[0][i].m);
        n_moves++;
        totalHPly++;
    }

    // Züge wieder rückgängig machen
    while (n_moves--)
        pBoard->takebackmove();


    //if (aspir_high)
    //{
    //   *dump << "+++";
   // }



    //pBoard->gen(0);
    pBoard->moveStack[0] = save_ms;

    if (!xboard || out.getLogging())
        *dump << endl;

    if (xboard && !options.no_post)
        cout << endl;

    last_pv_score = score;

}

// -------------------------------------------------------------------------


void Search::dump_current_board(int ply, Move m)
{
    string dummy;

    pBoard->dump();

    cout << "PLY = " << ply;
    cout << " MOVE = " << m.toString();
    cout << endl;

    cin >> dummy;
}

// -------------------------------------------------------------------------
//  Statistiken zum gemachten Zug ausgeben
// -------------------------------------------------------------------------

void Search::dumpStatistics()
{
    bool xboard = RespOptions::getHandle()->getValue("output.style") 
                  == "XBOARD";

    if (xboard && !out.getLogging())
        return;

    ostream* dump = (xboard) ? &out.logfile() :  static_cast<ostream*>(&out);

    long now = time_in_ms();

    double diff = (now - start)/1000.0;

    *dump << endl;
    *dump << "Nodes ................... = " << nNodes + nQNodes;
    *dump << " (" << setprecision(2) << (double) nQNodes*100.0/(nNodes + nQNodes);
    *dump << "% in Q-Search)" << endl;

    *dump << "Hashentries found ....... = " << nHashEntriesFound
          << " (" << (double) nHashEntriesFound*100.0/(nNodes+nQNodes)  << "%)";

#ifdef DEBUG_HASH
    *dump << " [**DEBUG** ERRORS = " << nWrongHashmove << "]" << endl;
#else
    *dump << endl;
#endif 

    *dump << "Beta-cutoffs on 1.move .. = " << 100.0*nFaileHighOnFirst / nFaileHigh 
          << "%" << endl;

    *dump << "Lazy exits in eval ...... = " << Eval::getHandle()->nLazyExits;

#ifdef DEBUG_LAZYEVAL
    *dump << " [**DEBUG** ERRORS = " << Eval::getHandle()->nWrongLazyExits 
          << "]"<< endl;
#else
    *dump << endl;
#endif

    *dump << "Extensions:   check = " << nCheckExt << ", ";
    *dump << "exchange = " << nExchangeExt << ", ";
    *dump << "matethread = " << nMateThreadExt << endl;
    *dump << "              single move = " << nOneMoveExt << ", ";
    *dump << "ex.ful.prun.= " << nExFulPrune << endl;
    *dump << endl;
    *dump << "TIME USED           = " << diff;
    *dump << " (nodes/sec = " << fixed << setprecision(0) << (nNodes + nQNodes)/diff << ")" << endl;

//    *dump << "[tm_calls=" << timer_calls << ", ";
//    *dump << "tm_call_mask=" << tm_call_mask << "]" << endl;

}





// -------------------------------------------------------------------------
//  Zug mit bester Score nach vorne sortieren
// -------------------------------------------------------------------------
void Search::swapWithBestScoringMove(int ply, int idx)
{

    MoveStack& ms = pBoard->moveStack[ply];

    int max_score = ms.score[idx];
    int max_idx = idx;

    for (int i = idx + 1; i < ms.size(); i++)
        if (ms.score[i] > max_score)
        {
            max_score = ms.score[i];
            max_idx = i;
        }

    ms.swap(idx,max_idx);
}


// -------------------------------------------------------------------------
// Suche starten
// -------------------------------------------------------------------------

int Search::go(long max_time_ms, int& it_depth,  bool ponder, int max_iter,
               std::vector<Move>* p_epd_move_list, long& epd_time, bool epd_bm)
{
    // -----------------------------------
    // Statistiken auf Null setzen
    // -----------------------------------
    nNodes              = 0;
    nQNodes             = 0;
    nFaileHigh          = 0;
    nFaileHighOnFirst   = 0;
    nHashEntriesFound   = 0;
    //Eval::getHandle()->nPHashEntriesUsed = 0;
    Eval::getHandle()->nLazyExits        = 0;
    Eval::getHandle()->nWrongLazyExits   = 0;

    nMateThreadExt  = 0;
    nCheckExt       = 0;
    nExchangeExt    = 0;
    nPromotionExt   = 0;
    nOneMoveExt     = 0;
    nExFulPrune     = 0;

    nWrongHashmove  = 0;

    timer_calls     = 0;  
    // Bei Zeitnot: tm_mask so setzen, dass keine Zeitueberschreitung
    if (max_time_ms < 6000)
    {
        tm_call_mask = 0x3fff;
    }

    isEPDTest = p_epd_move_list != NULL;

    pBoard->histHeuristic.reset();
    rootage = pBoard->getAge();

    for (int i=0; i < MAXPLY; i++)
        pBoard->moveStack[i].resetKiller();

    // Maximale Anzahl Iterationen begrenzen
    if (max_iter > 50)
        max_iter = 50;


    // -----------------------------------
    //  Pondern?
    // -----------------------------------
    bool takebackPonderMove = true;
    if (ponder)
    {
        bool xboard = RespOptions::getHandle()->getValue("output.stype") 
                      == "XBOARD";
        if (got_ponder_move)
        {
            if (!xboard || out.getLogging())
                out.logfile() << "Pondering on " << ponder_move << endl;
            takebackPonderMove = pBoard->makeSaveMove(ponder_move);
        }
        else
        {
            if (!xboard || out.getLogging())
                out.logfile() << "Pondering (no move!)" << endl;
            takebackPonderMove = false;
        }
    }

    pondering = ponder;

    setOptNoPost(!RespOptions::getHandle()->getValueAsBool("game.post"));

    // -----------------------------------
    //  S u c h e   s t a r t e n
    // -----------------------------------
#ifdef USE_MMXASM
    clear_fpu();
#endif

    stop_now = false;
    start = time_in_ms();
    max_time = max_time_ms/1000.0;

    init_pv();

    int sc = iter_deep(max_iter, p_epd_move_list, epd_time, epd_bm);

#ifdef USE_MMXASM
    clear_fpu();
#endif

    // -----------------------------------
    //  Falls pondering: Zug zurücknehmen
    // -----------------------------------
    if (ponder)
    {
        if (takebackPonderMove)
            pBoard->takebackmove();
    }

    it_depth = iter_depth;

    if (!ponder)
    {
        dumpStatistics();

        bool xboard = RespOptions::getHandle()->getValue("output.style") 
                      == "XBOARD";
        if (!xboard || out.getLogging())
        {
            ostream* dump = (xboard) ? &out.logfile() :  
                                       static_cast<ostream*>(&out);

            *dump << "SCORE               = " << sc;
            *dump << ", MOVE = " << pv[0][0].m;
            *dump << " (PONDER-MOVE = ";
            if (got_ponder_move)
                *dump << ponder_move;
            else
                *dump <<  "---";
            *dump << ")" << endl;
        }
    }


    return sc;
}

// -------------------------------------------------------------------------
//    Auführen von 'perft' und anschliessend Statistik anzeigen
// -------------------------------------------------------------------------
void Search::run_perft(int depth)
{
    nNodes      = 0;

#ifdef USE_MMXASM
    clear_fpu();
#endif

    start = time_in_ms();

    perft(depth, 0);

#ifdef USE_MMXASM
    clear_fpu();
#endif
    long now = time_in_ms();

    double diff = (now - start)/1000.0;

    out << "PERFT ... DEPTH = "<< depth << endl;
    out << endl;
    out << "NODES       = " << nNodes << endl;
    out << "TIME USED   = " << diff << endl;
    out << "NODES/SEC   = " << nNodes/diff << endl;
    out << endl;
}


// -------------------------------------------------------------------------
//    Prüfen, ob Suchzeit überschritten
// -------------------------------------------------------------------------
bool Search::time_up()
{
#ifdef USE_MMXASM
    clear_fpu();
#endif

    timer_calls++;

    long diff = time_in_ms() - start;


    // ---------------------------------------
    //  Kontrollieren der time_up() Aufrufe:
    // ---------------------------------------
    // Anpassen wie häufig time_up() aufgerufen wird. Dazu wird die verwendete
    // Zeit durch die Zahl der Aufrufe geteilt: ist der Wert in
    // einem vernünftigen Rahmen, bleibt alles beim Alten; ansonsten erfolgt
    // eine Anpassung von tm_call_mask

    if (timer_calls > 0 && diff > 1500)  // mind. 1.5 Sek.
    {
        long q = diff / timer_calls;

        // "Vernünftiger" Zeitrahmen: 0.150 sek <= q <= 0.300 sek.
        if (q < 150)
        {
            // tm_call_mask == 0x3ffff würde bedeuten:
            // Aufruf nur alle 262143 Knoten
            if (tm_call_mask < 0x3ffff) 
            {
                tm_call_mask <<= 1;
                tm_call_mask  |= 1;
            }

        }
        else if (q > 300)
        {
            // tm_call_mask == 0x1fff würde bedeuten:
            // Aufruf schon alle 8192 Knoten
            if (tm_call_mask > 0x1fff)
            {
                tm_call_mask >>= 1;
            }
        }
    }



    return (diff/1000.0 > max_time) || (!isEPDTest && userInput());

}


// -------------------------------------------------------------------------
//  Prüfen, ob Zug aus Hash in Ordnung ist
// -------------------------------------------------------------------------
bool Search::hashmove_ok(const Move& m, int ply) const
{
    if (m.isNullmove())
        return true;

    pBoard->gen(ply);

    int prom = 0;
    if (m.isPromotion())
        prom = m.getPromPiece();

    Move t = pBoard->moveStack[ply].findMove(m.from(), m.to(), prom);

    return !t.isNullmove();
}


// -------------------------------------------------------------------------
//  updateEpdTime ... beim Iterative Deepening Lösungszeit aktualisieren
// -------------------------------------------------------------------------
void Search::updateEpdTime(long&epd_time, long epd_start_time, 
                           std::vector<Move>* p_m_list, bool epd_bm)
{
    // Falls Testlauf
    if (p_m_list)
    {
        if ( ( epd_bm &&  moveInList(pv[0][0].m, *p_m_list) ) ||
             (!epd_bm && !moveInList(pv[0][0].m, *p_m_list) ) )
        {
            // Richtige Lösung => Zeit merken, falls bisher falsch
            if (epd_time == -1) epd_time = time_in_ms() - epd_start_time;
        }
        else // falsche Lösung => Marke setzen
            epd_time = -1;
    }
}

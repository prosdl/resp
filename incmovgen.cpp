// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : incmovgen.cpp
//                       incremental move generator
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.18
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: incmovgen.cpp,v 1.10 2003/06/01 13:47:29 rosendahl Exp $
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

#include "incmovgen.h"
#include "move.h"

#ifdef IMG_STATS
int IncMoveGen::counter[9];
#endif

using namespace std;

#ifdef IMG_SWITCH
Move IncMoveGen::next()
{
    switch (phase)
    {
    case GET_HASH:
        // -------------------------------------------
        //              hash move
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_HASH]++;
#endif
        phase = GEN_CAP;
        if (pB->isLegal(hashMove)) 
            return hashMove;

    case GEN_CAP:
        // -------------------------------------------
        //          generate captures
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GEN_CAP]++;
#endif
        pB->genCaps(ply);
        score_caps();
        curMoveIdx = 0;
        phase = GET_CAP;
        pMS = &pB->moveStack[ply];

    case GET_CAP:
        // -------------------------------------------
        //          get next capture
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_CAP]++;
#endif
        while (curMoveIdx < pMS->size())
        {
            // no hash moves
            if (pMS->score[curMoveIdx] == SKIP)
            {
                curMoveIdx++;
                continue;
            }
            swapWithBestScoringMove(curMoveIdx);

            if (pMS->score[curMoveIdx] < 0)
            {
                // best move is losing capture ==> exit
                break;
            }
            return pMS->stack[curMoveIdx++];
        }

        phase = GET_KILLER_I;

    case GET_KILLER_I:
        // -------------------------------------------
        //          get first killer move
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_KILLER_I]++;
#endif
        {
            phase = GET_KILLER_II;
            const Move& k1 = pMS->killer1;
            if (k1.data != hashMove.data && pB->isLegal(k1))
                return k1;
        }

    case GET_KILLER_II:
        // -------------------------------------------
        //          get second killer move
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_KILLER_II]++;
#endif
        {
            phase = GEN_NONCAP;
            const Move& k2 = pMS->killer2;
            if (pMS->killer1.data != k2.data    && 
                hashMove.data != k2.data        && 
                pB->isLegal(k2)) 
                return k2;
        }

    case GEN_NONCAP:
        // -------------------------------------------
        //        generate non capturing moves
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GEN_NONCAP]++;
#endif
        pB->gen(ply, false);
        score_noncaps();
        phase = GET_NONCAP;

    case GET_NONCAP:
        // -------------------------------------------
        //  get next move, that is either a losing
        //  capture or not a capture (sorted by 
        //  history heuristics)
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_NONCAP]++;
#endif
        while (curMoveIdx < pB->moveStack[ply].size())
        {
            // no hash or killer moves
            if (pMS->score[curMoveIdx] == SKIP)
            {
                curMoveIdx++;
                continue;
            }
            swapWithBestScoringMove(curMoveIdx);
            return pMS->stack[curMoveIdx++];
        }

        return Move::nullmove();

    case GEN_EVASIONS:
        // -------------------------------------------
        //          generate king evasions
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GEN_EVASIONS]++;
#endif
        pB->genKingEvasions(ply);
        score_evasions();
        phase = GET_EVASIONS;
        curMoveIdx = 0;
        pMS = &pB->moveStack[ply];

        return Move::nullmove();

    case GET_EVASIONS:
        // -------------------------------------------
        //          get next evasion move
        // -------------------------------------------
#ifdef IMG_STATS
        counter[GET_EVASIONS]++;
#endif
        while (curMoveIdx < pMS->size())
        {
            swapWithBestScoringMove(curMoveIdx);
            return pMS->stack[curMoveIdx++];
        }

        return Move::nullmove();
    }


    return Move::nullmove();
}

// -------------------------------------------------------------------------
//  dump ... printing out statistics
// -------------------------------------------------------------------------

#ifdef IMG_STATS
void IncMoveGen::dump()
{
    cout << "IMG-STATS:" << endl;
    cout << "GET_HASH      = " << counter[GET_HASH]      << endl;
    cout << "GET_KILLER_I  = " << counter[GET_KILLER_I]  << endl;
    cout << "GET_KILLER_II = " << counter[GET_KILLER_II] << endl;
    cout << "GEN_CAP       = " << counter[GEN_CAP]       << endl;
    cout << "GET_CAP       = " << counter[GET_CAP]       << endl;
    cout << "GEN_NONCAP    = " << counter[GEN_NONCAP]    << endl;
    cout << "GET_NONCAP    = " << counter[GET_NONCAP]    << endl;
    cout << "GEN_EVASIONS  = " << counter[GEN_EVASIONS]  << endl;
    cout << "GET_EVASIONS  = " << counter[GET_EVASIONS]  << endl;
}
#endif


#else // IMG as State Machine


// -------------------------------------------------------------------------
//  Static Members
// -------------------------------------------------------------------------
IncMoveGen::GenCap       IncMoveGen::genCap;
IncMoveGen::GetCap       IncMoveGen::getCap;
IncMoveGen::GenNonCap    IncMoveGen::genNonCap;
IncMoveGen::GetNonCap    IncMoveGen::getNonCap;
IncMoveGen::GetHash      IncMoveGen::getHash;
IncMoveGen::GetKillerI   IncMoveGen::getKillerI;
IncMoveGen::GetKillerII  IncMoveGen::getKillerII;
IncMoveGen::GenEvasions  IncMoveGen::genEvasions;
IncMoveGen::GetEvasions  IncMoveGen::getEvasions;

#endif //IMG_SWITCH

// -------------------------------------------------------------------------
//  score king evasions
// -------------------------------------------------------------------------
void IncMoveGen::score_evasions()
{
    const int MOVSCORE_KILLER = 100000000;
    const int MOVSCORE_CAPT   = 150000000; 
    const int MOVSCORE_LCAPT  =      1500;

    Board* pB = Board::getHandle();

    MoveStack& ms = pB->moveStack[ply];

    for (int i=0; i < ms.size(); i++)
    {
        const Move& m = ms.stack[i];

        // highest score for hash move
        if (m == hashMove)
        {
            ms.score[i] = 900000000;
        }
        else
        {
            // killer
            const int n_kill = ms.findKiller(m);
            if (n_kill)
                ms.score[i] = MOVSCORE_KILLER - n_kill;

            // captures
            else if (m.isCapture() || m.isPromotion())
            {
                // calculate SEE score
                int see_val    = pB->see(m);

                if ( see_val < 0 )
                {
                    // losing capture
                    ms.score[i] = see_val + MOVSCORE_LCAPT;
                }
                else // winning capture
                {
                    ms.score[i] = see_val + MOVSCORE_CAPT;
                } 
            }
            else
            {
                // non captures sorted by history heuristic
                ms.score[i] = pB->histHeuristic.getScore(m);
            }
        }
    }
}

// -------------------------------------------------------------------------
//  assign score to non-capturing moves
// -------------------------------------------------------------------------
void IncMoveGen::score_noncaps()
{
    Board* pB = Board::getHandle();
    MoveStack& ms = pB->moveStack[ply];

    for (int i=curMoveIdx; i < ms.size(); i++)
    {
        const Move& m = ms.stack[i];

        // skip hash and killer moves
        if (m.data == hashMove.data   || 
            m.data == ms.killer1.data ||
            m.data == ms.killer2.data)
        {
            ms.score[i] = SKIP;
        }
        else if (!m.isCapture()) {
            ms.score[i] = pB->histHeuristic.getScore(m);
        }
    }

}

// -------------------------------------------------------------------------
//  assign score to capturing moves
// -------------------------------------------------------------------------
void IncMoveGen::score_caps()
{
    Board* pB = Board::getHandle();
    MoveStack& ms = pB->moveStack[ply];

    for (int i=0; i < ms.size(); i++)
    {
        const Move& m = ms.stack[i];

        // skip hashmove
        if (m.data == hashMove.data)
            ms.score[i] = SKIP;
        else
            ms.score[i] = pB->see(m);
    }

}


// -------------------------------------------------------------------------
//  swap move with best score to position 'idx' in the move list
// -------------------------------------------------------------------------
void IncMoveGen::swapWithBestScoringMove(int idx)
{
    int max_score = pMS->score[idx];
    int max_idx = idx;

    for (int i = idx + 1; i < pMS->size(); i++)
        if (pMS->score[i] > max_score)
        {
            max_score = pMS->score[i];
            max_idx = i;
        }

    if (idx != max_idx)
    {
        pMS->swap(idx,max_idx);
    }
}


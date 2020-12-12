// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : IncMoveGen.h
//                       incremental movegenerator
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.18
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: incmovgen.h,v 1.11 2003/05/07 17:14:54 rosendahl Exp $
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
#ifndef IncMoveGen_H
#define IncMoveGen_H

#include "move.h"
#include "board.h"

// -------------------------------------------------------------------------
//              c l a s s  I n c M o v G e n
// -------------------------------------------------------------------------
const int SKIP = -10000;

#define IMG_SWITCH

#ifdef IMG_SWITCH
class IncMoveGen
{
private:
    Board*      pB;         // access to current position
    int         ply;        // current ply in the search
    Move        hashMove;   // hashmove (== 0, if not available)
    int         iter_depth; // iter_depth for move ordering

    int         curMoveIdx;    // current move

    MoveStack*  pMS;        // pointer to movelist

//#define IMG_STATS
#ifdef IMG_STATS
public:
    static int counter[9];
    static void dump();
#endif

    enum { 
        GET_HASH,
        GET_KILLER_I,
        GET_KILLER_II,
        GEN_CAP,
        GET_CAP,
        GEN_NONCAP,
        GET_NONCAP,
        GEN_EVASIONS,
        GET_EVASIONS
    } phase;                // current phase from next

public:
    // constructor
    IncMoveGen(int _ply, const Move& _hashMove, int _iter_depth, int inCheck) : 
      ply(_ply), hashMove(_hashMove), iter_depth(_iter_depth)
    {
        pB = Board::getHandle();
        phase = (inCheck) ? GEN_EVASIONS : GET_HASH;

#ifdef IMG_STATS
        static bool init = false;

        if (!init)
        {
            for (int i=0; i < 9; i++) counter[i] = 0;
            init = true;
        }
#endif
    }

private:
    void score_evasions();
    void score_noncaps();
    void score_caps();


    void swapWithBestScoringMove(int idx);


public:
    Move next();        // get the next move
};


#else // IMG as a State Machine


// ----------------------------------------------------------------
//  IncMoveGen ... implements the incremental Move Generator
//                 following the "State Pattern"
// ----------------------------------------------------------------

class IncMoveGen
{
   // ----------------------------------------------------
   //       Interface State
   // ----------------------------------------------------
   friend class State;
public:
   class State
   {
   public: 
      virtual State* next(IncMoveGen* pIMG) = 0;
      virtual ~State() {}
   };

   // ----------------------------------------------------
   //       Implementations of State
   // ----------------------------------------------------

   // ---------------------------------
   //  GetHash -- get hash move 
   // ---------------------------------
   class GetHash : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         if (Board::getHandle()->isLegal(pIMG->hashMove)) 
            pIMG->curMove = pIMG->hashMove;
         else
            pIMG->curMove = Move::nullmove();
            
         return &pIMG->genCap;
      }
   };

   // ---------------------------------
   //  GetKillerI -- get first killer 
   // ---------------------------------
   class GetKillerI : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         const Move& k1 = pIMG->pMS->killer1; // shortcut

         if (k1.data != pIMG->hashMove.data && 
             Board::getHandle()->isLegal(k1))
            pIMG->curMove = k1;
         else
            pIMG->curMove = Move::nullmove();

         return &pIMG->getKillerII;
      }
   };

   // ----------------------------------
   //  GetKillerII -- get second killer
   // ----------------------------------
   class GetKillerII : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         const Move& k2 = pIMG->pMS->killer2; //shortcut

         if (pIMG->pMS->killer1.data != k2.data    &&
             pIMG->hashMove.data     != k2.data    &&
             Board::getHandle()->isLegal(k2))
            pIMG->curMove = k2;
         else
            pIMG->curMove = Move::nullmove();

         return &pIMG->genNonCap;
      }
   };

   // ------------------------------------
   //  GenCap -- generate capturing moves
   // ------------------------------------
   class GenCap : public State
   {
   public:
      State * next(IncMoveGen* pIMG) {
         Board::getHandle()->genCaps(pIMG->ply);
         pIMG->score_caps();
         pIMG->curMoveIdx = 0;
         pIMG->curMove = 0;
         return &pIMG->getCap;
      }
   };

   // ---------------------------------
   //  GetCap -- get next non-losing
   //  capture move
   // ---------------------------------
   class GetCap : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         MoveStack*& pMS = pIMG->pMS;  // just a shortcut
         
         while (pIMG->curMoveIdx < pMS->size()) {
            // skip hash moves ...
            if (pMS->score[pIMG->curMoveIdx] == SKIP) {
               pIMG->curMoveIdx++;
               continue;
            }
            pIMG->swapWithBestScoringMove(pIMG->curMoveIdx);

            if (pMS->score[pIMG->curMoveIdx] < 0)
               break;
            pIMG->curMove = pMS->stack[pIMG->curMoveIdx++];
            return this;
         }

         pIMG->curMove = Move::nullmove();
         return &pIMG->getKillerI;
      }
         
   };

   // ---------------------------------
   //  GenNonCap -- generate not
   //  capturing moves
   // ---------------------------------
   class GenNonCap : public State 
   {
   public:
      State* next(IncMoveGen* pIMG) {
         Board::getHandle()->gen(pIMG->ply, false);
         pIMG->score_noncaps();

         pIMG->curMove = 0;
         return &pIMG->getNonCap;
      }
   };

   // ---------------------------------
   //  GetNonCap -- get not capturing
   //  moves
   // ---------------------------------
   class GetNonCap : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         MoveStack*& pMS = pIMG->pMS;
         
         while (pIMG->curMoveIdx < pMS->size()) {
            if (pMS->score[pIMG->curMoveIdx] == SKIP) {
               pIMG->curMoveIdx++;
               continue;
            }
            pIMG->swapWithBestScoringMove(pIMG->curMoveIdx);
            pIMG->curMove = pMS->stack[pIMG->curMoveIdx++];
            return this;
         }

         pIMG->curMove = Move::nullmove();
         return NULL;
      };
   };

   // ----------------------------------
   //  GenEvasions -- only if in check:
   //  generate Evasions
   // ----------------------------------
   class GenEvasions : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         Board::getHandle()->genKingEvasions(pIMG->ply);
         pIMG->score_evasions();

         // horrible hack ahead: it's useful to
         // generate the king evasion _without_ returning
         // the first move; i.e. for single reply extensions
         pIMG->curMove = 255;
         pIMG->curMoveIdx = 0;
         
         return &pIMG->getEvasions;
      }
   };

   // ----------------------------------
   //  GetEvasions -- only if in check:
   //  get Evasions
   // ----------------------------------
   class GetEvasions : public State
   {
   public:
      State* next(IncMoveGen* pIMG) {
         MoveStack*& pMS = pIMG->pMS;

         if (pIMG->curMoveIdx < pMS->size()) {
            pIMG->swapWithBestScoringMove(pIMG->curMoveIdx);
            pIMG->curMove = pMS->stack[pIMG->curMoveIdx++];
            return this;
         }
         pIMG->curMove = Move::nullmove();
         return NULL;
      }
   };

   friend class GenCap;
   friend class GetCap;
   friend class GenNonCap;
   friend class GetNonCap;
   friend class GetHash;
   friend class GetKillerI;
   friend class GetKillerII;
   friend class GenEvasions;
   friend class GetEvasions;

   // ----------------------------------------------------------------
   
private:
    int         ply;        // current ply in the search
    Move        hashMove;   // hashmove (== 0, if not available)
    int         iter_depth; // iter_depth for move ordering
    int         curMoveIdx; // current move index
    MoveStack*  pMS;        // pointer to movelist
    Move        curMove;    // current Move
    State*      pCurState;  // current state of the state machine
    int         in_check;   // currently in check?

    // States
    static GenCap       genCap;
    static GetCap       getCap;
    static GenNonCap    genNonCap;
    static GetNonCap    getNonCap;
    static GetHash      getHash;
    static GetKillerI   getKillerI;
    static GetKillerII  getKillerII;
    static GenEvasions  genEvasions;
    static GetEvasions  getEvasions;


public:

   // --------------------------------------------------------- 
   //  init ... resets the state machine; _has_ to be called
   //           everytime a new move generation is needed
   // --------------------------------------------------------- 
   IncMoveGen(int ply, const Move& hashMove, int iter_depth, int in_check) {
      this->ply        = ply;
      this->hashMove   = hashMove;
      this->iter_depth = iter_depth;
      this->in_check   = in_check;

      curMoveIdx  = 0;
      pMS         = &Board::getHandle()->moveStack[ply];

      pCurState   = in_check ? (State*) &genEvasions : 
                               (State*) &getHash;
   }
  
   // --------------------------------------------------------- 
   //  next ... this method is responsible for making the 
   //           transitions in the state machine. It continues
   //           until a new move is found (!= 0) or the state
   //           machine moved to it's final state (NULL)
   // --------------------------------------------------------- 
   Move next() {
      do {
         pCurState = pCurState->next(this);
         if (curMove != 0)
            return curMove;
      } while (pCurState);

      return Move::nullmove();
   }

   
private:
    void score_evasions();
    void score_noncaps();
    void score_caps();
    void swapWithBestScoringMove(int idx);

};

#endif // IMG_SWITCH

#endif // IncMoveGen_H


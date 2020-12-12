// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : board.h
//                       Header zu board.cpp
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: board.h,v 1.63 2003/06/02 18:12:53 rosendahl Exp $
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


#ifndef BOARD_H
#define BOARD_H

#ifdef USE_DEFINESH
#include "defines.h"
#endif
#include "basic_stuff.h"
#include "move.h"
#include "hash.h"
#include "phash.h"
#include "bitboard.h"

#include <string>
#include <vector>

// -------------------------------------------------------------------------
//   INLINES für SAN Parser
inline bool  SAN_IS_PIECE(char c)
{
    return (c=='P') || (c=='N') ||(c=='B') || (c=='R') || (c=='Q') || (c=='K');
}
inline bool SAN_IS_PROMPIECE_LETTER(char c)
{
    return (c=='Q') || (c=='R') || (c=='B') || (c=='N');
}

inline bool SAN_IS_FILE_LETTER(char c) { return (c >= 'a') && (c <= 'h');  }
inline bool SAN_IS_RANK_DIGIT(char c)  { return (c >= '1') && (c <= '8');  }


// -------------------------------------------------------------------------
//          c l a s s   H i s t o r y E n t r y
// -------------------------------------------------------------------------
class HistoryEntry
{
public:
  HistoryEntry() {};

public:
  Move m;           // ausgeführter Zug

  // info:
  // Bits   # 0..# 7: age
  //        # 8..#15: fifty
  //        #16..#19: castle
  //        #20..#27: ep
  UINTEGER32 info;
};


// -------------------------------------------------
//  class PositionStats ...  Statistik für Figuren
// -------------------------------------------------
class PositionStats {
public:
    // Zaehler
    int nWhitePieces[7];
    int nBlackPieces[7];
    int nWhitePiecesTotal;
    int nBlackPiecesTotal;

    // Score
    int whiteMatScore;

    // Signaturen
    unsigned int matWhite;
    unsigned int matBlack;

public:
    PositionStats() {
        whiteMatScore = nWhitePiecesTotal = nBlackPiecesTotal  = 0;
        for (int i = 0; i < 7; i++)
            nWhitePieces[i] = nBlackPieces[i]  = 0;
        matWhite = matBlack = 0;
    };

    void init(int nWPawns,int nWKnights,int nWBishops,int nWRooks,int nWQueens,
              int nBPawns,int nBKnights,int nBBishops,int nBRooks,int nBQueens)
    {
        nWhitePieces[PAWN]   = nWPawns;
        nWhitePieces[BISHOP] = nWBishops;
        nWhitePieces[KNIGHT] = nWKnights;
        nWhitePieces[ROOK]   = nWRooks;
        nWhitePieces[QUEEN]  = nWQueens;

        nBlackPieces[PAWN]   = nBPawns;
        nBlackPieces[BISHOP] = nBBishops;
        nBlackPieces[KNIGHT] = nBKnights;
        nBlackPieces[ROOK]   = nBRooks;
        nBlackPieces[QUEEN]  = nBQueens;

        nBlackPiecesTotal = nWhitePiecesTotal = 1;  // für König
        whiteMatScore       = 0;
        matWhite = matBlack = 0;

        for (int c=PAWN; c <= QUEEN; c++)
        {
            nWhitePiecesTotal += nWhitePieces[c];
            nBlackPiecesTotal += nBlackPieces[c];
            whiteMatScore += piece_val[c]*(nWhitePieces[c] - nBlackPieces[c]);

            if (nWhitePieces[c])
                setWMaterial(c);
            if (nBlackPieces[c])
                setBMaterial(c);
        }
    }

public:
    void setWMaterial(int piece)
    {
        matWhite |= piece_set_bit_mask[piece];
    }
    void clearWMaterial(int piece)
    {
        matWhite &= piece_clear_bit_mask[piece];
    }
    void setBMaterial(int piece)
    {
        matBlack |= piece_set_bit_mask[piece];
    }
    void clearBMaterial(int piece)
    {
        matBlack &= piece_clear_bit_mask[piece];
    }

public:
    void addWPiece(int piece)
    {
        nWhitePiecesTotal++;
        if (++nWhitePieces[piece])
        {
            setWMaterial(piece);
        }
        whiteMatScore += piece_val[piece];
    }
    void addBPiece(int piece)
    {
        nBlackPiecesTotal++;
        if (++nBlackPieces[piece])
        {
            setBMaterial(piece);
        }
        whiteMatScore -= piece_val[piece];
    }
    void removeWPiece(int piece)
    {
        nWhitePiecesTotal--;
        if (!--nWhitePieces[piece])
        {
            clearWMaterial(piece);
        }
        whiteMatScore -= piece_val[piece];
    }
    void removeBPiece(int piece)
    {
        nBlackPiecesTotal--;
        if (!--nBlackPieces[piece])
        {
            clearBMaterial(piece);
        }
        whiteMatScore += piece_val[piece];
    }
};



// -------------------------------------------------------------------------
//          c l a s s   H i s t o r y
// -------------------------------------------------------------------------
class History
{
 private:
     HistoryEntry histData[MAX_HISTORY];

 public:
     hash_t poskey[MAX_HISTORY];


 public:
  void add(const int hply, const Move& m, const int castle, 
           const int ep, const int fifty, const int age,
           const hash_t& z1_hash)
  {
    histData[hply].m = m;
    poskey[hply]= z1_hash;

    histData[hply].info = age | fifty<<8 | castle<<16 | ep<<20;
  }

  const HistoryEntry& get(int hply) const
  {
    return histData[hply];
  } 

  void get(const int hply, Move& m, int& castle, int& ep, int& fifty, int& age) const
  {
      m         = histData[hply].m;

      const UINTEGER32& info = histData[hply].info;

      // info auslesen:
      // info Struktur: 32Bit -  xxxx eeee eeee cccc ffff ffff aaaa aaaa
      age       =  info &      0xff;
      fifty     = (info &    0xff00) >> 8;
      castle    = (info &   0xf0000) >> 16;  
      ep        = (info & 0xff00000) >> 20;

  }


  int identicalPositionsAt(int i, const hash_t& z1) const
  {
      return (poskey[i] == z1);
  }

  Move getMove(const int hply) const  { return histData[hply].m;  }
};


// -------------------------------------------------------------------------
//          c l a s s   B o a r d
// -------------------------------------------------------------------------

class Board 
{

private:
    Board();                       // Konstruktor
public:
    ~Board();

    static Board* p_instance;
public:
    static Board* getHandle() { 
       if (p_instance == NULL) {
          p_instance = new Board();
       }
       return p_instance; 
    }


    friend class Eval;
    friend class KEDB;

public:
    // STRUKTUREN FÜR POSITION
    ColorEnum side;           // Wer zieht?
    int ep;             // Position, um e.p. Bauern zu schlagen
    int whiteKingPos;   // Position weisser König
    int blackKingPos;   // Position schwarzer König
    int fifty;          // Anzahl Züge für 50-Zug Regel
    int castle;         // Welche Rochaden sind noch möglich:
                        // Bit#1 Weiß König
                        // Bit#2 Weiß Dame  
                        // Bit#3 Schwarz König
                        // Bit#4 Schwarz Dame
    int totalHPly;      // Anzahl Züge gesamt

    bool whiteKingCastled;  // Hat der König rochiert?
    bool blackKingCastled;

    hash_t z1_hash;      // Hashwert (wird bei allen Zügen aktualisiert)
    hash_t z2_hash;      // Hashlock (wird ebenfalls aktualisiert)

    hash_t zp1_hash;      // Hashwert für Bauernstruktur
    hash_t zp2_hash;      // 

    int age;           // "Alter" einer Position (wird genau dann um 1 erhöht,
                        // wenn fifty auf Null gesetzt wird)

private:
    Hash*  p_TT;
    PHash* p_pawnTT;          // Hashtable für Bauernstruktur


public:
    // BITBOARDS
    Bitboard whitePieces;
    Bitboard blackPieces;
    Bitboard allPieces90C;
    Bitboard allPieces45C;
    Bitboard allPieces45AC;
    Bitboard whitePawns;
    Bitboard blackPawns;
    Bitboard whiteRooks;
    Bitboard blackRooks;
    Bitboard whiteQueens;
    Bitboard blackQueens;
    Bitboard whiteBishops;
    Bitboard blackBishops;
    Bitboard whiteKnights;
    Bitboard blackKnights;
    Bitboard whiteKing;
    Bitboard blackKing;

    // vor Zuggenerierung aktualisiert
    Bitboard freeSquares;   // == ~(whitePieces | blackPieces)



public:
    // Statistik für Figuren
    PositionStats stats;

public:
    // Zuglisten
    MoveStack moveStack[MAXPLY];

    MoveStack* pCurMoveStack;

    // History Heuristik
    HistHeuristic histHeuristic;

    History history;    // Informationen über gemachte Züge

private:
    // Methoden für Initialisierungen
    void initByteBoardFromBitBoards(ByteBoard& byb) const;
    void initBitBoardsFromByteBoard(const ByteBoard& byb);

//    void initStaticBitBoards();
    void loadInitialPosition(ByteBoard& byb);
    void initPositionData();

    void initStatsFromBitboards();

public:
    // ------------------------------------------------------------
    //      Öffentliche Methoden von board
    // ------------------------------------------------------------

    // Brett für neues Spiel fertigmachen
    void initialize(); 

    // Brett auf Konsole ausgeben
    void dump() const;       
    
    // Gibt alle BitBoards auf der Konsole aus
    void dumpBitBoards();
    void dumpBitBoardToCout(const Bitboard& b);
    
    // Gibt Statistik über Figurenanzahlen aus
    void dumpStats();

    // lädt Position im FEN-Format von Datei
    bool load_fen_from_file(const char* fname);
    
    // Wechselt die am Zug befindliche Seite
    void switchSide();
    
    // Wer zieht?
    ColorEnum sideToMove() const { return side; }     
    
    // Anzahl Züge gesamt
    unsigned int nMoves() const { return totalHPly; }      
    
    // Zuggenerierung
    bool gen(int ply, bool genCaps = true);
    bool genCaps(int ply);
    void genKingEvasions(int ply);

    // wird pos von Seite s angegriffen?
    bool isAttacked(int pos, ColorEnum s);


    // steht der König von Seite s im Schach?
    bool kingAttacked(ColorEnum s);

    // Prüfen, ob die am Zug befindliche Seite im Schach steht
    bool inCheck();

    //  Prüfen, ob die Seite, die den letzten Zug gemacht hat im Schach steht
    bool Board::positionLegal();


    bool positionLegalNoCheck(const Move& m);


    // Welche Figuren der Farbe "s" attackieren "pos"?
    BITBOARD getAttackBoard(int pos, ColorEnum s);

    // Welche Figuren attackieren "pos"?
    BITBOARD getFullAttackBoard(int pos);

    // Welche Figuren attackieren den König der Farbe "s"?
    BITBOARD getKingAttackBoard(ColorEnum s);

    // Fesselungen
    int pinnedByWhitePiece(int pos, int to);
    int pinnedByBlackPiece(int pos, int to);
    int pinnedToWhiteKing(int pos);
    int pinnedToBlackKing(int pos);


    // Abtauschfolgen effizient berechnen
    int see(const Move& m);
    void updateAttacks(Bitboard& attacks, Bitboard& allowed, int att_pos, int direction);
    int see_test(int balance, int target_pos, int ply);

    // schnell prüfen, ob m legaler Zug
    bool isLegal(const Move& m);
    bool whiteMoveLegal(const Move& m);
    bool blackMoveLegal(const Move& m);


    void getGameStatus(int ply, bool& check, bool& mate, bool& remis, RemisEnum& rtype );
    int repititions();
    bool materialRemis() const;

    // Zug ausführen - zurücknehmen
    void makemove(const Move& m);
    void takebackmove();
    bool makeSaveMove(const Move& m);



    // FEN - Position von einem String laden
    bool load_fen(std::string& fen);   

    // Aktuelle Position in FEN umwandeln
    std::string get_fen();



    int getAge() const { return age;  }
    hash_t get_z1_hash()  const { return z1_hash;  }
    hash_t get_z2_hash()  const { return z2_hash;  }
    hash_t get_zp1_hash() const { return zp1_hash;  }
    hash_t get_zp2_hash() const { return zp2_hash;  }
    int getCastle() const { return castle; }
    int getEp() const { return ep; }
    int  getFifty() const { return fifty; }
    int getTotalHPly() const { return totalHPly; }

    int getMatScore() const
    {
        return (side == WHITE) ? stats.whiteMatScore : -stats.whiteMatScore;
    }

    // ------------------------------------------------------------

private:
    // Hilfsfunktion für dumpBitBoards()
    void dumpBitBoard(BITBOARD b, std::string s[]);

public:
    static char pieceChar[];    // für dump() benötigt


private:
    // Zuggenerierung für einzelne Figuren
    void genWhitePawnNonCaps(int ply);
    void genBlackPawnNonCaps(int ply);
    bool genWhitePawnCaps(int ply);
    bool genBlackPawnCaps(int ply);

    bool genWhiteFileRankAttacks(int ply, bool genCaps);
    bool genWhiteFileRankCaps(int ply);

    bool genBlackFileRankAttacks(int ply, bool genCaps);
    bool genBlackFileRankCaps(int ply);

    bool genWhiteDiagonalAttacks(int ply, bool genCaps);
    bool genWhiteDiagonalCaps(int ply);

    bool genBlackDiagonalAttacks(int ply, bool genCaps);
    bool genBlackDiagonalCaps(int ply);

    bool genWhiteKnightAttacks(int ply, bool genCaps);
    bool genWhiteKnightCaps(int ply);

    bool genBlackKnightAttacks(int ply, bool genCaps);
    bool genBlackKnightCaps(int ply);

    bool genWhiteKingAttacks(int ply, bool genCaps);
    bool genWhiteKingCaps(int ply);

    bool genBlackKingAttacks(int ply, bool genCaps);
    bool genBlackKingCaps(int ply);


    // Angegriffene Felder,...
    BITBOARD    rankFileAttacks(int from_pos);
    BITBOARD    diagAttacks(int from_pos);

    // MMX - Assembler Funktionen
#ifdef USE_MMXASM
    void     __cdecl mmx_rankFileAttacks(int from_pos); 
    void     __cdecl mmx_diagAttacks(int from_pos);     
    int      __cdecl mmx_hasWhiteQueenRook();
    int      __cdecl mmx_hasWhiteQueenBishop();
    int      __cdecl mmx_hasBlackQueenRook();
    int      __cdecl mmx_hasBlackQueenBishop();
    BITBOARD __cdecl mmx_m7ToBitboard();
    void     __cdecl mmx_toggleWhitePieceOnBitboards(int pos, int piece);
    void     __cdecl mmx_toggleBlackPieceOnBitboards(int pos, int piece);
    void     __cdecl mmx_updateWhitePieces(int from, int to);
    void     __cdecl mmx_updateBlackPieces(int from, int to);
    void     __cdecl mmx_getFullAttackBoard(int pos);
    //void     __cdecl mmx_rankQueenRookAttacks(int from_pos);
    //void     __cdecl mmx_fileQueenRookAttacks(int from_pos);

#endif  // USE_MMXASM



#ifdef DEBUG_MOVEGEN
    void checkBoards(Move m, std::string func);
#endif


    // Figur an Position pos aus Bitboards bestimmen
    int getWhitePieceFromBBs(int pos) const;
    int getBlackPieceFromBBs(int pos) const;
    void getPieceFromBBs(int pos, int &color, int &piece) const;

    bool whitePieceOnBoard(int piece, int pos);
    bool blackPieceOnBoard(int piece, int pos);


    // Zugausführung: Bitboardaktualisierungen
    void updateBitboards(const Move& m, bool restore);
    void toggleBlackPieceOnBitboards(int pos, int piece);
    void toggleWhitePieceOnBitboards(int pos, int piece);
/*    void saveBitboards(int hply, const Move& m, ColorEnum side);
    void restoreBitboards(int hply, const Move& m, ColorEnum side); */


    
};




#endif // BOARD_H

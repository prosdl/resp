// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : board.cpp
//                       board verwaltet die Klassen, die mit der Zugaus-
//                       führung und der aktuellen Stellung zu tun haben.
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: board.cpp,v 1.86 2003/06/02 18:12:53 rosendahl Exp $
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

#ifdef USE_DEFINESH
#include "defines.h"
#endif
#include "basic_stuff.h"
#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "notation.h"
#include "StringTools.h"
#include "respoptions.h"
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;


//  Verwenden von MMX-Assembler Routinen?
#ifdef USE_MMXASM
#include "board_mmx.cpp"
#endif

char Board::pieceChar[] = {' ', 'p', 'n', 'b', 'r', 'q', 'k' };


// -------------------------------------------------------------------------
//    KONSTRUKTOR
// -------------------------------------------------------------------------

Board* Board::p_instance = NULL;

Board::Board() {
    cout << "#Initializing board" << endl;
    BBTables::init();

    p_TT       = Hash::getHandle();
    p_pawnTT   = PHash::getHandle();

    cout << "#Board ready" << endl;
}

Board::~Board() {
}



// -------------------------------------------------------------------------
//  initialisiert die BitBoards ausgehend vom ByteBoard 'byb'
// -------------------------------------------------------------------------
void Board::initBitBoardsFromByteBoard(const ByteBoard& byb)
{
    int i;

    // Wo sind weisse (schwarze) Figuren?
    whitePieces = blackPieces = 0;
    whitePawns  = blackPawns = 0;
    whiteRooks  = blackRooks = 0;
    whiteQueens = blackQueens = 0;
    whiteBishops = blackBishops = 0;
    whiteKnights = blackKnights = 0;
    whiteKing = blackKing = 0;
    for (i=0; i<64; i++)
    {
        if (byb.getColor(i) == WHITE)
        {
            whitePieces |= BBTables::bbmask[i];

            switch (byb.getPiece(i))
            {
            case PAWN:
                whitePawns |= BBTables::bbmask[i]; break;
            case ROOK:
                whiteRooks |= BBTables::bbmask[i]; break;
            case QUEEN:
                whiteQueens |= BBTables::bbmask[i]; break;
            case BISHOP:
                whiteBishops |= BBTables::bbmask[i]; break;
            case KNIGHT:
                whiteKnights |= BBTables::bbmask[i]; break;
            case KING:
                whiteKing |= BBTables::bbmask[i]; 
            whiteKingPos = i;
            break;
            }
        }
        if (byb.getColor(i) == BLACK)
        {
            blackPieces |= BBTables::bbmask[i];
            switch (byb.getPiece(i))
            {
            case PAWN:
                blackPawns |= BBTables::bbmask[i]; break;
            case ROOK:
                blackRooks |= BBTables::bbmask[i]; break;
            case QUEEN:
                blackQueens |= BBTables::bbmask[i]; break;
            case BISHOP:
                blackBishops |= BBTables::bbmask[i]; break;
            case KNIGHT:
                blackKnights |= BBTables::bbmask[i]; break;
            case KING:
                blackKing |= BBTables::bbmask[i]; 
            blackKingPos = i;
            break;
            }
        }
    }

    // rotated Bitboards
    allPieces45C = allPieces45AC = allPieces90C  = whitePieces|blackPieces;
    allPieces90C.rot90C();
    allPieces45C.rot45C();
    allPieces45AC.rot45AC();
}


// -------------------------------------------------------------------------
//  initialisiert das ByteBoard 'byb' ausgehend von den Bitboards
// -------------------------------------------------------------------------

void Board::initByteBoardFromBitBoards(ByteBoard& byb) const
{
   for (int i=0; i<64; i++)
   {
      int color, piece;

      getPieceFromBBs(i,color,piece);

      byb.set(i,color,piece);
   }
}

// -------------------------------------------------------------------------
//  initialisieren der Figurenstatistik via Bitboards
// -------------------------------------------------------------------------

void Board::initStatsFromBitboards()
{
    stats.init(whitePawns.popcount(),whiteKnights.popcount(),whiteBishops.popcount(),
               whiteRooks.popcount(),whiteQueens.popcount(),
               blackPawns.popcount(),blackKnights.popcount(),blackBishops.popcount(),
               blackRooks.popcount(),blackQueens.popcount());
}

// -------------------------------------------------------------------------
//  Ausgeben der Statistik
// -------------------------------------------------------------------------

void Board::dumpStats()
{
    out << "nWhitePawns   = " << stats.nWhitePieces[PAWN] << " --- "  
         << "bBlackPawns   = " << stats.nBlackPieces[PAWN] << endl; 
    out << "nWhiteBishops = " << stats.nWhitePieces[BISHOP] << " --- "  
         << "bBlackBishops = " << stats.nBlackPieces[BISHOP] << endl; 
    out << "nWhiteKnights = " << stats.nWhitePieces[KNIGHT] << " --- "  
         << "bBlackKnights = " << stats.nBlackPieces[KNIGHT] << endl; 
    out << "nWhiteRooks   = " << stats.nWhitePieces[ROOK] << " --- "  
         << "bBlackRooks   = " << stats.nBlackPieces[ROOK] << endl; 
    out << "nWhiteQueens  = " << stats.nWhitePieces[QUEEN] << " --- "  
         << "bBlackQueens  = " << stats.nBlackPieces[QUEEN] << endl; 
    out << "nWhitePieces  = " << stats.nWhitePiecesTotal << " --- "  
         << "bBlackPieces  = " << stats.nBlackPiecesTotal << endl; 
}

// -------------------------------------------------------------------------
//  lädt Standard Schachaufstellung nach 'byb'
// -------------------------------------------------------------------------

void Board::loadInitialPosition(ByteBoard& byb)
{
    out << "Loading start position ... ";
    int i=0;

    for (i=16; i<48; i++)
    {
      byb.set(i,EMPTY,NO_PIECE);
    }


    for (i=8; i<16; i++)
    {
      byb.set(i,BLACK,PAWN);
      byb.set(i+40,WHITE,PAWN);
    }

   byb.set(0,BLACK,ROOK);
   byb.set(7,BLACK,ROOK);
   byb.set(1,BLACK,KNIGHT);
    byb.set(6,BLACK,KNIGHT);
    byb.set(2,BLACK,BISHOP);
    byb.set(5,BLACK,BISHOP);
    byb.set(3,BLACK,QUEEN);
    byb.set(4,BLACK,KING);

    for (i=0; i<8; i++)
        byb.set(56+i,WHITE,byb.getPiece(i));

    // Hashwerte initialisieren

    z1_hash  = p_TT->z1_value(byb.piece,byb.color);
    z2_hash  = p_TT->z2_value(byb.piece,byb.color);
    zp1_hash = p_pawnTT->zp1_value(byb.piece,byb.color);
    zp2_hash = p_pawnTT->zp2_value(byb.piece,byb.color);


    out << "OK." << endl;
}

// -------------------------------------------------------------------------
//  Daten für Anfangsposition setzen
// -------------------------------------------------------------------------
void Board::initPositionData()
{
    side = WHITE;   // Weiß am Zug
    castle = 0xF;   // alle Rochaden erlaubt
    whiteKingCastled = blackKingCastled = false;
    ep = 0;         // Kein e.p. möglich
    totalHPly = 0;  // Noch kein Zug in der Partie gemacht
    fifty = 0;      // Init. für 50 Zug Regel

    age   = 0;        // ALter auf Null    

    p_TT->clear(); // HashTable zurücksetzen
    p_pawnTT->clear();

    histHeuristic.reset();    // History Heuristik löschen
}

// -------------------------------------------------------------------------
//  Brett für neues Spiel initialisieren
// -------------------------------------------------------------------------

void Board::initialize()
{
    ByteBoard byb;

    loadInitialPosition(byb);
    initBitBoardsFromByteBoard(byb);
    initPositionData();
    initStatsFromBitboards();
}


// -------------------------------------------------------------------------
//  Farbe des Spielers, der am Zug ist wechseln
// -------------------------------------------------------------------------
void Board::switchSide()
{
    side = (side == WHITE) ? BLACK : WHITE;

    gen(0,true);
}

// -------------------------------------------------------------------------
//  Dump-Befehle
// -------------------------------------------------------------------------
void Board::dump() const
{
    ByteBoard byb;

    initByteBoardFromBitBoards(byb);

    out << "   A B C D E F G H    ";
    if (side == WHITE)
        out << "WHITE ";
    else
        out << "BLACK ";
    out << "TO MOVE";
    out << "  No# " << totalHPly;
    out << endl;
    out << "  |-+-+-+-+-+-+-+-|" << endl;
    for (int i=0; i<64; i++) 
    {
        if (!(i%8))
            out << ' ' << 8 - i/8 ;
        char c = pieceChar[ byb.getPiece(i) ];
        if (byb.getColor(i) == WHITE)
            c -= 32;
        if (c==' ')
            if ((ROW(i) + COL(i))%2 )
                c = '.';
        out << '|' << c;

        if (! ((i+1)%8))
        {
            out << '|' << endl;
            out << "  |-+-+-+-+-+-+-+-|";
            out << endl;
        }
    }
}

void Board::dumpBitBoard(BITBOARD b, string s[])
{
    int zeile = 0;

    for (int j = 0; j < 8; j++)
        s[j] = "";

    for (int i=0; i<64; i++, b >>= 1)
    {
        s[zeile] +=  (( b & 1 ) ? '1' : '0');
        if (! ((i+1) % 8))
            zeile++;
    }
}

void Board::dumpBitBoardToCout(const Bitboard& b)
{
    string s[8];

    dumpBitBoard(b,s);

    for (int i=0; i < 8; i++)
        out << s[i] << endl;
}

void Board::dumpBitBoards()
{
    string s1[8],s2[8],s3[8],s4[8],s5[8],s6[8];
    int i;

    // TEST

 /*   for (i=0; i < 64; i++)
    {
        dumpBitBoard(BBTables::w_ppawn_mask[i],s1);
        dumpBitBoard(BBTables::b_ppawn_mask[i],s2);
    
        cout << "-----> " << i << endl;
        for (int j = 0; j < 8; j++)
        {
            cout << s1[j] << "  " << s2[j] << endl;
        }
        cout << endl;
    }

    cin >> i; */


    // dump Bitboards Weiss 
    dumpBitBoard(whitePieces,s1);
    dumpBitBoard(allPieces45C,s2);
    dumpBitBoard(allPieces45AC,s3);
    dumpBitBoard(allPieces90C,s4);

    out << "------------------------- WHITE PIECES -----------------" << endl;
    out << "wPieces   ";
    out << "all45C    ";
    out << "all45AC   ";
    out << "all90C    ";
    out << endl;
    for (i=0; i < 8; i++)
    {
        out << s1[i] << "  ";
        out << s2[i] << "  ";
        out << s3[i] << "  ";
        out << s4[i];

        out << endl;
    }

    // dump Bitboards Schwarz
    dumpBitBoard(blackPieces,s1);
    dumpBitBoard(allPieces45C,s2);
    dumpBitBoard(allPieces45AC,s3);
    dumpBitBoard(allPieces90C,s4);

    out << "------------------------- BLACK PIECES -----------------" << endl;
    out << "bPieces   ";
    out << "all45C    ";
    out << "all45AC   ";
    out << "all90C    ";
    out << endl;
    for (i=0; i < 8; i++)
    {
        out << s1[i] << "  ";
        out << s2[i] << "  ";
        out << s3[i] << "  ";
        out << s4[i];

        out << endl;
    }

    // Nächste Seite?

    out << "WEITER (j/n)?  ";
    string inp;
    cin >> inp;
    cin.ignore();

    if (inp != "j")
        return;

    // Figuren weiss
    out << "-------- INDIVIDUAL PIECES WHITE --------------------------" << endl;
    out << "wPawns    ";
    out << "wBishops  ";
    out << "wKnights  ";
    out << "wRooks    ";
    out << "wQueens   ";
    out << "wKing     ";
    out << endl;

    dumpBitBoard(whitePawns,s1);
    dumpBitBoard(whiteBishops,s2);
    dumpBitBoard(whiteKnights,s3);
    dumpBitBoard(whiteRooks,s4);
    dumpBitBoard(whiteQueens,s5);
    dumpBitBoard(whiteKing,s6);

    for (i=0; i < 8; i++)
    {
        out << s1[i] << "  ";
        out << s2[i] << "  ";
        out << s3[i] << "  ";
        out << s4[i] << "  ";
        out << s5[i] << "  ";
        out << s6[i] << "  ";

        out << endl;
    }
    // Figuren schwarz
    out << "-------- INDIVIDUAL PIECES BLACK --------------------------" << endl;
    out << "bPawns    ";
    out << "bBishops  ";
    out << "bKnights  ";
    out << "bRooks    ";
    out << "bQueens   ";
    out << "bKing     ";
    out << endl;

    dumpBitBoard(blackPawns,s1);
    dumpBitBoard(blackBishops,s2);
    dumpBitBoard(blackKnights,s3);
    dumpBitBoard(blackRooks,s4);
    dumpBitBoard(blackQueens,s5);
    dumpBitBoard(blackKing,s6);

    for (i=0; i < 8; i++)
    {
        out << s1[i] << "  ";
        out << s2[i] << "  ";
        out << s3[i] << "  ";
        out << s4[i] << "  ";
        out << s5[i] << "  ";
        out << s6[i] << "  ";

        out << endl;
    }
}

// -------------------------------------------------------------------------
//      loadFenString
//      -------------
//  Lädt eine Position im FEN-Format und aktualisiert das Brett entsprechend.
// -------------------------------------------------------------------------

bool Board::load_fen(string& fen)
{
    // Leerzeichen entfernen
    fen = StringTools::trim(fen);

    // temp.Brettdaten: die echte Brettposition wird erst überschrieben, wenn
    //                  erfolgreich geparsed wurde.
    ByteBoard byb;
    ColorEnum new_side;     // Wer zieht?
    int new_ep;            // Position, um e.p. Bauern zu schlagen
    int new_fifty;          // Anzahl Züge für 50-Zug Regel
    int new_castle;        // Welche Rochaden sind noch möglich:
    int new_totalHPly;

    bool success = Notation::load_fen(fen, byb, new_ep, new_fifty, new_castle, new_totalHPly, new_side);

    if (!success)
        return false;


    // -- FEN erfolgreich eingelesen: Daten kopieren und Spiel aufsetzen ----
    

    castle      = new_castle;
    ep          = new_ep;
    fifty       = new_fifty;
    totalHPly   = new_totalHPly;
    side        = new_side;

    // Nicht bekannt, ob Könige rochiert haben:
    // Immer annehmen, dass nicht
    whiteKingCastled = blackKingCastled = false;

    age         = 0;

    z1_hash  = p_TT->z1_value(byb.piece,byb.color);
    z2_hash  = p_TT->z2_value(byb.piece,byb.color);
    zp1_hash = p_pawnTT->zp1_value(byb.piece,byb.color);
    zp2_hash = p_pawnTT->zp2_value(byb.piece,byb.color);


    p_TT->clear();
    p_pawnTT->clear();

    histHeuristic.reset();

    out << "Setting up FEN position ... ";

    initBitBoardsFromByteBoard(byb);
    initStatsFromBitboards();

    out << "OK." << endl;
    
    return true;
}

// -------------------------------------------------------------------------
//  aktuelle Position -> FEN-Position
// -------------------------------------------------------------------------
string Board::get_fen() 
{
   // Aktuelle Position --> ByteBoard
   ByteBoard byb;
   initByteBoardFromBitBoards(byb);
   

   string fen;
   // Position aus Byte-Board umwandeln in FEN
   for (int y=0; y<8; y++) {
      int emptySq = 0;
      for (int x=0; x<8; x++) {
         int piece = byb.getPiece(y*8+x);
         int color = byb.getColor(y*8+x);

         if (piece != NO_PIECE && emptySq > 0) {
            fen += '0' + emptySq;
            emptySq = 0;
         }
            
         switch (piece) {
            case PAWN:
               fen += (color==WHITE) ? "P":"p"; break;              
            case KNIGHT:
               fen += (color==WHITE) ? "N":"n"; break;              
            case BISHOP:
               fen += (color==WHITE) ? "B":"b"; break;              
            case ROOK:
               fen += (color==WHITE) ? "R":"r"; break;              
            case QUEEN:
               fen += (color==WHITE) ? "Q":"q"; break;              
            case KING:
               fen += (color==WHITE) ? "K":"k"; break;
            case NO_PIECE:
               emptySq++;        
               break;
            default:
               fen = "ERROR";
               return fen;
         }
      }
      if (emptySq > 0) {
         fen += '0' + emptySq;
         emptySq = 0;
      }
      if (y != 7) fen += "/";
   }

   fen += " ";
   fen += (side==WHITE) ? 'w' : 'b';
   fen += " ";

   bool no_castle = true;
   if (castle & CASTLE_WHITE_KINGSIDE) {
      fen += "K";
      no_castle = false;
   }
   if (castle & CASTLE_WHITE_QUEENSIDE) {
      fen += "Q";
      no_castle = false;
   }
   if (castle & CASTLE_BLACK_KINGSIDE) {
      fen += "k";
      no_castle = false;
   }
   if (castle & CASTLE_BLACK_QUEENSIDE) {
      fen += "q";
      no_castle = false;
   }

   if (no_castle) fen += '-';

   fen += ' ';
   
   if (ep) {
      fen += (char) (COL(ep) + 'a');
      fen += (char) (8-ROW(ep) + '0');
   } else {
      fen += '-';
   }

   fen += ' ';

   // Halfmove-Clock / totalHPlay
   std::stringstream ss;
   ss << fifty << " " << (1+totalHPly/2);
   fen += ss.str();

   return fen;
}

// -------------------------------------------------------------------------
//  FEN-Position von Datei laden
// -------------------------------------------------------------------------

bool Board::load_fen_from_file(const char* fname)
{
    string fpath = RespOptions::getHandle()->getValue("dir.fen");
    
    fpath += fname;

    out << "Opening " << fpath << " ... ";
    // Datei zum lesen öffnen:
    fstream ifile(fpath.c_str());

    // Konnte Datei geöffnet werden?
    if (!ifile)
    {
        out << "Error!" << endl;
        return false;
    }
    out << "OK." << endl;

    string fen;

    getline(ifile,fen);

    ifile.close();


    return load_fen(fen); 
}



// -------------------------------------------------------------------------
//                              gen
//                              ---
//
// "gen" generiert alle pseudo-legalen Züge zur aktuellen Brettposition.
//
//    Rückgabewert == false, falls Zug gefunden wurde, der den König schlägt
// -------------------------------------------------------------------------

bool Board::gen(int ply, bool genCaps)
{
    // only reset moveStack, when gen has to produce all moves
    if (genCaps)
        moveStack[ply].reset();

    // temporary bitboards
    freeSquares        = ~(whitePieces|blackPieces);

    pCurMoveStack = moveStack + ply;

    if (side == WHITE)
    {
        if (genCaps) 
            genWhitePawnCaps(ply);
        genWhiteFileRankAttacks(ply, genCaps);
        genWhiteDiagonalAttacks(ply, genCaps);
        genWhiteKnightAttacks(ply, genCaps);
        genWhiteKingAttacks(ply, genCaps);
        genWhitePawnNonCaps(ply);
    }
    else // side == BLACK
    {
        if (genCaps)
            genBlackPawnCaps(ply);
        genBlackFileRankAttacks(ply, genCaps);
        genBlackDiagonalAttacks(ply, genCaps);
        genBlackKnightAttacks(ply, genCaps);
        genBlackKingAttacks(ply, genCaps);
        genBlackPawnNonCaps(ply);
    } 

    return true;
}

// -------------------------------------------------------------------------
//                              genCaps
//                              -------
//
// "genCaps" generiert alle pseudo-legalen, schlagenden Züge 
//  zur aktuellen Brettposition.
// -------------------------------------------------------------------------
bool Board::genCaps(int ply)
{
    moveStack[ply].reset();

    // temporäre Bitboards
    freeSquares   = ~(whitePieces|blackPieces);

    pCurMoveStack = moveStack + ply;

    if (side == WHITE)
    {
        genWhitePawnCaps(ply);
        genWhiteFileRankCaps(ply);
        genWhiteDiagonalCaps(ply);
        genWhiteKnightCaps(ply);
        genWhiteKingCaps(ply);
    }
    else 
    {
        genBlackPawnCaps(ply);
        genBlackFileRankCaps(ply);
        genBlackDiagonalCaps(ply);
        genBlackKnightCaps(ply);
        genBlackKingCaps(ply);
    }



    return true; 
}

// -------------------------------------------------------------------------
//                              genKingEvasions
//                              ---------------
//  Nur falls der König im Schach steht...
//  Alle legalen Züge produzieren, die aus dem Schach herausführen:
//  Ausweichen, Schlagen oder Dazwischensetzen
// -------------------------------------------------------------------------

void Board::genKingEvasions(int ply)
{
    moveStack[ply].reset();

    // temporäre Bitboards
    freeSquares   = ~(whitePieces|blackPieces);
    pCurMoveStack = moveStack + ply;

    if (side == WHITE)
    {
        // Angreifer finden und deren Anzahl sichern
        Bitboard attacks = getAttackBoard(whiteKingPos,BLACK);
        int n_attackers = attacks.popcount();

        int check_dir1 = 0;
        int check_dir2 = 0;

        if (n_attackers == 1)
        {
            int att_pos = attacks.msb();
            if (! (BBTables::bbmask[att_pos] & blackPawns) )
                check_dir1  = BBTables::direction[att_pos][whiteKingPos];
        }
        else
        {
            Bitboard temp = attacks;
            int att_pos = temp.msb();
            if (! (BBTables::bbmask[att_pos] & blackPawns) )
                check_dir1  = BBTables::direction[att_pos][whiteKingPos];
            temp ^= BBTables::bbmask[att_pos];
            att_pos = temp.msb();
            if (! (BBTables::bbmask[att_pos] & blackPawns) )
                check_dir2  = BBTables::direction[att_pos][whiteKingPos];
        }


        // alle Königszüge finden
        Bitboard to_set = BBTables::king_attacks[whiteKingPos] & ~whitePieces;
        while (to_set)
        {
            int to_pos = to_set.msb();
            to_set ^= BBTables::bbmask[to_pos];

            // Ist dies ein legaler Königszug?
            if (BBTables::bbmask[to_pos] & attacks)
            {
                // Angreifer geschlagen
                if (isAttacked(to_pos,BLACK) )
                    continue;
            }
            else
            {
                if (BBTables::direction[to_pos][whiteKingPos] == check_dir1 ||
                    BBTables::direction[to_pos][whiteKingPos] == check_dir2 ||
                    isAttacked(to_pos,BLACK) )
                    continue;
            }

            // Alles i.O.: Zug eintragen
            if (blackPieces & BBTables::bbmask[to_pos])
            {
                int cap_piece = getBlackPieceFromBBs(to_pos);
                Move m(whiteKingPos | to_pos << 8 | KING << 16 | cap_piece << 19 | 
                       Move::CAPTURE << 24);
                pCurMoveStack->push(m);
            }
            else
            {
                Move m(whiteKingPos | to_pos << 8 | KING << 16);
                pCurMoveStack->push(m);
            }

        } 

        // die Königszüge sind komplett; jetzt versuchen mit anderen Figuren
        // zu schlagen oder sie dazwischenzusetzen

        // bei mehr als einem Angreifen --> keine Chance
        if (n_attackers != 1)
            return;

        // BB 'target_set' erstellen mit allen möglichen Zielfeldern für die anderen
        // Figuren

        // 1.) Der Angreifer selbst
        Bitboard target_set = attacks;
        // 2.) Falls der Angreifer Q,R oder B: alle Felder zwischen dem
        //     Angreifer und dem König
        int att_pos   = attacks.lsb();
        int att_piece = getBlackPieceFromBBs(att_pos);
        if ( att_piece != PAWN )
            target_set |= BBTables::squaresBetween[att_pos][whiteKingPos];

        // Alle Zielpositionen iterieren
        while (target_set)
        {
            int target_pos = target_set.lsb();
            target_set ^= BBTables::bbmask[target_pos];


            Bitboard helper;
            int cap_piece = 0;
            if (target_pos == att_pos)
            {
                // Zielfeld ist Angreifer --> Capture
                    
                helper    = getAttackBoard(target_pos,WHITE);
                cap_piece = att_piece;
            }
            else // target_pos != att_pos
            {
                // Zielfeld ist leer   --> NonCapture
                helper = BBTables::knight_attacks[target_pos] & whiteKnights;
                helper |= rankFileAttacks(target_pos) & (whiteRooks | whiteQueens);
                helper |= diagAttacks(target_pos) & (whiteBishops | whiteQueens);

                // diese dummen Bauern koennen auf 4 (vier!) Arten einen Angriff
                // blocken: 1 Feld vor, 2 Felder vor, Promotion, En passant Zug

                int row = ROW(target_pos);
                if (row < 6)
                {
                    if (BBTables::bbmask[target_pos + 8] & whitePawns)
                        helper |= BBTables::bbmask[target_pos + 8];
                    else if (row == 4 &&  (BBTables::bbmask[target_pos + 8] & freeSquares) &&
                            (BBTables::bbmask[target_pos + 16] & whitePawns) )
                    {
                        helper |= BBTables::bbmask[target_pos + 16];
                    }
                    if (target_pos == ep)
                    {
                        helper |= whitePawns & BBTables::blackPawnCaps[ep];
                    }
                }

            }


            // Alle Figuren, die dem König zur Hilfe kommen durchlaufen
            while (helper)
            {
                int special = 0;

                if (target_pos == att_pos)
                    special = Move::CAPTURE;

                int helper_pos = helper.msb();
                helper ^= BBTables::bbmask[helper_pos];

                // Gefesselte Figuren ...
                if (pinnedToWhiteKing(helper_pos))
                    continue;

                int helper_piece = getWhitePieceFromBBs(helper_pos);
                switch (helper_piece)
                {
                case KNIGHT:
                case BISHOP: 
                case ROOK  : 
                case QUEEN :
                    {
                        Move m(helper_pos | target_pos<<8 | helper_piece << 16 |
                               cap_piece << 19 | special << 24);
                        pCurMoveStack->push(m);
                        break;
                    }
                case PAWN:
                    {
                        special |= Move::PAWN_MOVE;
                        if (! (special & Move::CAPTURE))
                        {
                            if (COL(helper_pos) != COL(target_pos))
                            {
                                special |= Move::EP_CAPTURE;
                            }
                            else if (helper_pos == target_pos + 16)
                            {
                                special |= Move::PAWN_2SQUARES;
                            }

                        }
                        Move m(helper_pos | target_pos<<8 | helper_piece << 16 |
                               cap_piece << 19 | special << 24);

                        if (special & Move::EP_CAPTURE)
                            m.setCapturedPiece(PAWN);
                        if (ROW(target_pos) == 0)
                        {
                            special |= Move::PROMOTE;
                            m.setSpecial(special);
                            pCurMoveStack->pushProm(m);
                        }
                        else
                            pCurMoveStack->push(m);

                        break;
                    }

                }
            }

        }


        // Spezialfall: Angreifer ist Bauer der durch e.p. geschlagen werden kann
        if (att_piece == PAWN && ep == att_pos - 8)
        {
            Bitboard helper = whitePawns & BBTables::blackPawnCaps[ep];
            while (helper)
            {
                int helper_pos = helper.lsb();
                helper ^= BBTables::bbmask[helper_pos];

                if (pinnedToWhiteKing(helper_pos))
                    continue;

                Move m(helper_pos | ep<<8 | PAWN << 16 |
                    PAWN << 19 | (Move::PAWN_MOVE | Move::EP_CAPTURE) << 24);
                
                pCurMoveStack->push(m);

            }
            
        }



    }
    else // *********** side == BLACK *******************************
    {
        // Angreifer finden und deren Anzahl sichern
        Bitboard attacks = getAttackBoard(blackKingPos,WHITE);
        int n_attackers = attacks.popcount();

        int check_dir1 = 0;
        int check_dir2 = 0;

        if (n_attackers == 1)
        {
            int att_pos = attacks.lsb();
            if (! (BBTables::bbmask[att_pos] & whitePawns) )
                check_dir1  = BBTables::direction[att_pos][blackKingPos];
        }
        else
        {
            Bitboard temp = attacks;
            int att_pos = temp.lsb();
            if (! (BBTables::bbmask[att_pos] & whitePawns) )
                check_dir1  = BBTables::direction[att_pos][blackKingPos];
            temp ^= BBTables::bbmask[att_pos];
            att_pos = temp.lsb();
            if (! (BBTables::bbmask[att_pos] & whitePawns) )
                check_dir2  = BBTables::direction[att_pos][blackKingPos];
        }

        // alle Königszüge finden
        Bitboard to_set = BBTables::king_attacks[blackKingPos] & ~blackPieces;
        while (to_set)
        {
            int to_pos = to_set.lsb();
            to_set ^= BBTables::bbmask[to_pos];

            // Ist dies ein legaler Königszug?

            if (BBTables::bbmask[to_pos] & attacks)
            {
                // Angreifer geschlagen
                if (isAttacked(to_pos,WHITE) )
                    continue;
            }
            else
            {
                if (BBTables::direction[to_pos][blackKingPos] == check_dir1 ||
                    BBTables::direction[to_pos][blackKingPos] == check_dir2 ||
                    isAttacked(to_pos,WHITE) )
                    continue;
            }
            // Alles i.O.: Zug eintragen
            if (whitePieces & BBTables::bbmask[to_pos])
            {
                int cap_piece = getWhitePieceFromBBs(to_pos);
                Move m(blackKingPos | to_pos << 8 | KING << 16 | cap_piece << 19 | 
                       Move::CAPTURE << 24);
                pCurMoveStack->push(m);
            }
            else
            {
                Move m(blackKingPos | to_pos << 8 | KING << 16);
                pCurMoveStack->push(m);
            }

        } 

        // die Königszüge sind komplett; jetzt versuchen mit anderen Figuren
        // zu schlagen oder sie dazwischenzusetzen

        // bei mehr als einem Angreifen --> keine Chance
        if (n_attackers != 1)
            return;

        // BB 'target_set' erstellen mit allen möglichen Zielfeldern für die anderen
        // Figuren

        // 1.) Der Angreifer selbst
        Bitboard target_set = attacks;
        // 2.) Falls der Angreifer Q,R oder B: alle Felder zwischen dem
        //     Angreifer und dem König
        int att_pos   = attacks.msb();
        int att_piece = getWhitePieceFromBBs(att_pos);
        if ( att_piece != PAWN )
            target_set |= BBTables::squaresBetween[att_pos][blackKingPos];

        // Alle Zielpositionen iterieren
        while (target_set)
        {
            int target_pos = target_set.msb();
            target_set ^= BBTables::bbmask[target_pos];


            Bitboard helper;
            int cap_piece = 0;
            if (target_pos == att_pos)
            {
                // Zielfeld ist Angreifer --> Capture
                    
                helper    = getAttackBoard(target_pos,BLACK);
                cap_piece = att_piece;
            }
            else // target_pos != att_pos
            {
                // Zielfeld ist leer   --> NonCapture
                helper = BBTables::knight_attacks[target_pos] & blackKnights;
                helper |= rankFileAttacks(target_pos) & (blackRooks | blackQueens);
                helper |= diagAttacks(target_pos) & (blackBishops | blackQueens);

                // diese dummen Bauern koennen auf 4 (vier!) Arten einen Angriff
                // blocken: 1 Feld vor, 2 Felder vor, Promotion, En passant Zug

                int row = ROW(target_pos);
                if (row > 1)
                {
                    if (BBTables::bbmask[target_pos - 8] & blackPawns)
                        helper |= BBTables::bbmask[target_pos - 8];
                    else if (row == 3 &&  (BBTables::bbmask[target_pos - 8] & freeSquares) &&
                            (BBTables::bbmask[target_pos - 16] & blackPawns) )
                    {
                        helper |= BBTables::bbmask[target_pos - 16];
                    }
                    if (target_pos == ep)
                    {
                        helper |= blackPawns & BBTables::whitePawnCaps[ep];
                    }
                }

            }


            // Alle Figuren, die dem König zur Hilfe kommen durchlaufen
            while (helper)
            {
                int special = 0;

                if (target_pos == att_pos)
                    special = Move::CAPTURE;

                int helper_pos = helper.lsb();
                helper ^= BBTables::bbmask[helper_pos];

                // Gefesselte Figuren ...
                if (pinnedToBlackKing(helper_pos))
                    continue;

                int helper_piece = getBlackPieceFromBBs(helper_pos);
                switch (helper_piece)
                {
                case KNIGHT:
                case BISHOP: 
                case ROOK  : 
                case QUEEN :
                    {
                        Move m(helper_pos | target_pos<<8 | helper_piece << 16 |
                               cap_piece << 19 | special << 24);
                        pCurMoveStack->push(m);
                        break;
                    }
                case PAWN:
                    {
                        special |= Move::PAWN_MOVE;
                        if (! (special & Move::CAPTURE))
                        {
                            if (COL(helper_pos) != COL(target_pos))
                            {
                                special |= Move::EP_CAPTURE;
                            }
                            else if (helper_pos == target_pos - 16)
                            {
                                special |= Move::PAWN_2SQUARES;
                            }

                        }
                        Move m(helper_pos | target_pos<<8 | helper_piece << 16 |
                               cap_piece << 19 | special << 24);

                        if (special & Move::EP_CAPTURE)
                            m.setCapturedPiece(PAWN);
                        if (ROW(target_pos) == 7)
                        {
                            special |= Move::PROMOTE;
                            m.setSpecial(special);
                            pCurMoveStack->pushProm(m);
                        }
                        else
                            pCurMoveStack->push(m);

                        break;
                    }

                }
            }

        }

        // Spezialfall: Angreifer ist Bauer der durch e.p. geschlagen werden kann
        if (att_piece == PAWN && ep == att_pos + 8)
        {
            Bitboard helper = blackPawns & BBTables::whitePawnCaps[ep];
            while (helper)
            {
                int helper_pos = helper.msb();
                helper ^= BBTables::bbmask[helper_pos];

                if (pinnedToBlackKing(helper_pos))
                    continue;

                Move m(helper_pos | ep<<8 | PAWN << 16 |
                    PAWN << 19 | (Move::PAWN_MOVE | Move::EP_CAPTURE) << 24);
                
                pCurMoveStack->push(m);

            }
            
        }

    }
}


// Neue Zuggenerierung für Bauern (Idee aus B.Hyatt's Crafty):
// Bitboards einfach verschieben
void Board::genWhitePawnNonCaps(int ply)
{
    Bitboard to_set(whitePawns & ~BBTables::rankmask[1]);

    to_set = (to_set >> 8) & freeSquares;
    
    while (to_set)    // für alle Bauern:
    {
        int to = to_set.msb();
        Move m( (to+8) |  to << 8 | PAWN << 16 | Move::PAWN_MOVE << 24);

        // hole Bauern aus to_set
        to_set ^= BBTables::bbmask[to];

        int row_to = ROW(to);
        pCurMoveStack->push(m);

        // Doppelschritt möglich?
        if (row_to == 5) 
        {
            if (BBTables::bbmask[to-8] & freeSquares)
            {
                m.setTo(to-8);
                m.setSpecial(Move::PAWN_2SQUARES | Move::PAWN_MOVE);
                pCurMoveStack->push(m);
            }
        }
    }
}





void Board::genBlackPawnNonCaps(int ply)
{
    Bitboard to_set(blackPawns & ~BBTables::rankmask[6]);

    to_set = (to_set << 8) & freeSquares;
    
    while (to_set)    // für alle Bauern:
    {
        int to = to_set.msb();
        Move m( (to-8) |  to << 8 | PAWN << 16 | Move::PAWN_MOVE << 24);

        // hole Bauern aus to_set
        to_set ^= BBTables::bbmask[to];

        int row_to = ROW(to);
        pCurMoveStack->push(m);

        // Doppelschritt möglich?
        if (row_to == 2) 
        {
            if (BBTables::bbmask[to+8] & freeSquares)
            {
                m.setTo(to+8);
                m.setSpecial(Move::PAWN_2SQUARES | Move::PAWN_MOVE);
                pCurMoveStack->push(m);
            }
        }
    }
}




bool Board::genWhitePawnCaps(int ply)
{

    // Nach links schlagen
    Bitboard to_set = ( (whitePawns & BBTables::bbmaskBCDEFGH) >> 9 ) & blackPieces;
    while (to_set)
    {
        int to   = to_set.msb();
        int from = to + 9;
        int cap_piece = getBlackPieceFromBBs(to);

        if (cap_piece == KING) return false;

        Move m(from | to << 8 | PAWN<<16 | cap_piece<<19 | (Move::PAWN_MOVE | Move::CAPTURE)<<24);
        
        to_set ^= BBTables::bbmask[to];

        // Promotion
        if (ROW(to) == 0)
        {
            m.data |= Move::PROMOTE << 24;
            pCurMoveStack->pushProm(m);
        }
        else
            pCurMoveStack->push(m);

    }

    // Nach rechts schlagen
    to_set = ( (whitePawns & BBTables::bbmaskABCDEFG) >> 7 ) & blackPieces;
    while (to_set)
    {
        int to   = to_set.msb();
        int from = to + 7;
        int cap_piece = getBlackPieceFromBBs(to);
        if (cap_piece == KING) return false;

        Move m(from | to << 8 | PAWN<<16 | cap_piece<<19 | (Move::PAWN_MOVE | Move::CAPTURE)<<24);
        to_set ^= BBTables::bbmask[to];

        // Promotion
        if (ROW(to) == 0)
        {
            m.data |= Move::PROMOTE << 24;
            pCurMoveStack->pushProm(m);
        }
        else
            pCurMoveStack->push(m);

    }

    // en passant Schlagen möglich?
    if (ep)
    {
        Move m(0,ep,PAWN,Move::PAWN_MOVE | Move::EP_CAPTURE);
        
        // finde Bauern die e.p. schlagen können; dazu kann "blackPawnCaps[ep]"
        // "zweckentfremdet" werden:         
        //    e
        //  B b B
        Bitboard from_set = whitePawns & BBTables::blackPawnCaps[ep];

        while (from_set)
        {
            // Extrahieren
            m.setFrom(from_set.msb());
            from_set ^= BBTables::bbmask[m.from()]; 

            m.setCapturedPiece(PAWN);

            // Zug eintragen
            pCurMoveStack->push(m);
        }
    }

    // (nicht schlagende) Promotionen
    to_set = ((whitePawns & BBTables::rankmask[1]) >> 8) & freeSquares;
    while (to_set)
    {
        int to = to_set.lsb();
        Move m( (to+8) |  to << 8 | PAWN << 16 | (Move::PAWN_MOVE|Move::PROMOTE) << 24);
        to_set ^= BBTables::bbmask[to];
        pCurMoveStack->pushProm(m);
    }

    return true;
}



bool Board::genBlackPawnCaps(int ply)
{

    // Nach links schlagen
    Bitboard to_set = ( (blackPawns & BBTables::bbmaskBCDEFGH) << 7 ) & whitePieces;
    while (to_set)
    {
        int to   = to_set.lsb();
        int from = to - 7;
        int cap_piece = getWhitePieceFromBBs(to);

        if (cap_piece == KING) return false;

        Move m(from | to << 8 | PAWN<<16 | cap_piece<<19 | (Move::PAWN_MOVE | Move::CAPTURE)<<24);
        
        to_set ^= BBTables::bbmask[to];

        // Promotion
        if (ROW(to) == 7)
        {
            m.data |= Move::PROMOTE << 24;
            pCurMoveStack->pushProm(m);
        }
        else
            pCurMoveStack->push(m);

    }

    // Nach rechts schlagen
    to_set = ( (blackPawns & BBTables::bbmaskABCDEFG) << 9 ) & whitePieces;
    while (to_set)
    {
        int to   = to_set.lsb();
        int from = to - 9;
        int cap_piece = getWhitePieceFromBBs(to);
        if (cap_piece == KING) return false;

        Move m(from | to << 8 | PAWN<<16 | cap_piece<<19 | (Move::PAWN_MOVE | Move::CAPTURE)<<24);
        to_set ^= BBTables::bbmask[to];

        // Promotion
        if (ROW(to) == 7)
        {
            m.data |= Move::PROMOTE << 24;
            pCurMoveStack->pushProm(m);
        }
        else
            pCurMoveStack->push(m);

    }

    // en passant Schlagen möglich?
    if (ep)
    {
        Move m(0,ep,PAWN,Move::PAWN_MOVE | Move::EP_CAPTURE);
        
        // finde Bauern die e.p. schlagen können; dazu kann "whitePawnCaps[ep]"
        // "zweckentfremdet" werden: s.o.        
        Bitboard from_set = blackPawns & BBTables::whitePawnCaps[m.to()];

        while (from_set)
        {
            // Extrahieren
            m.setFrom(from_set.lsb());
            from_set ^= BBTables::bbmask[m.from()]; 

            m.setCapturedPiece(PAWN);

            // Zug eintragen
            pCurMoveStack->push(m);
        }
    }

    // (nicht schlagende) Promotionen
    to_set = ((blackPawns & BBTables::rankmask[6]) << 8) & freeSquares;
    while (to_set)
    {
        int to = to_set.msb();
        Move m( (to-8) |  to << 8 | PAWN << 16 | (Move::PAWN_MOVE|Move::PROMOTE) << 24);
        to_set ^= BBTables::bbmask[to];
        pCurMoveStack->pushProm(m);
    }

    return true;
}




// -------------------------------------------------------------------------
//  geradlinige  weisse Züge (Turm, Dame)
// -------------------------------------------------------------------------

bool Board::genWhiteFileRankAttacks(int ply, bool genCaps)
{
    Bitboard from_set( whiteRooks | whiteQueens );

    while (from_set) // solange noch Pos. on "from_set" vorhanden:
    {
        // Extrahieren
        int from_pos = from_set.msb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_rankFileAttacks(from_pos);
        
        Bitboard to_set(mmx_m7ToBitboard());
#else
        // Hole alle Felder, die von from_pos aus horizontal/vertikal erreicht werden können
        Bitboard all(~freeSquares);
        Bitboard to_set( BBTables::rank_attacks[from_pos][all.GET_RANK(from_pos)] 
               |   BBTables::file_attacks[from_pos][allPieces90C.GET_RANK(rot90C_bitIndex[from_pos])] );
#endif

        // Freie Felder
        Bitboard caps(to_set & blackPieces);
        to_set &= freeSquares;

        // Turm oder Dame?
        int piece = ROOK;
        if (BBTables::bbmask[from_pos] & whiteQueens)
            piece = QUEEN;

        Move m(from_pos | piece<<16);

        while (to_set) // Solange noch Pos. in "to_set"
        {
            // Extrah.
            m.setTo(to_set.msb());
            to_set ^= BBTables::bbmask[m.to()];

            pCurMoveStack->push(m);
        }

        // Schlagen
        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps) // Solange noch Pos. in "to_set"
            {
                // Extrah.
                m.setTo(caps.msb());
                caps ^= BBTables::bbmask[m.to()];

                int cap_piece = getBlackPieceFromBBs(m.to());

                // Illegale Position?
                if (cap_piece == KING)
                    return false;

                m.setCapturedPiece(cap_piece);
                pCurMoveStack->push(m);
            }
        }
    }

    return true;
}


// -------------------------------------------------------------------------
//  geradlinige schlagende weisse Züge (Turm, Dame)
// -------------------------------------------------------------------------

bool Board::genWhiteFileRankCaps(int ply)
{
    Bitboard from_set( whiteRooks | whiteQueens );


    while (from_set) // solange noch Pos. on "from_set" vorhanden:
    {
        // Extrahieren
        int from_pos = from_set.msb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_rankFileAttacks(from_pos);
        
        Bitboard to_set(mmx_m7ToBitboard());
#else
        // Hole alle Felder, die von from_pos aus horizontal/vertikal erreicht werden können
        Bitboard all(~freeSquares);
        Bitboard to_set( BBTables::rank_attacks[from_pos][all.GET_RANK(from_pos)] 
               |   BBTables::file_attacks[from_pos][allPieces90C.GET_RANK(rot90C_bitIndex[from_pos])] );
#endif


        to_set &= blackPieces;

        int piece = ROOK;
        if (BBTables::bbmask[from_pos] & whiteQueens)
            piece = QUEEN;
        Move m(from_pos | piece << 16 | Move::CAPTURE << 24);

        while (to_set) // Solange noch Pos. in "to_set"
        {
            // Extrah.
            m.setTo(to_set.msb());
            to_set ^= BBTables::bbmask[m.to()];


            int cap_piece = getBlackPieceFromBBs(m.to());

            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }

    return true;
}

// -------------------------------------------------------------------------
//  geradlinige  schwarze Züge (Turm, Dame)
// -------------------------------------------------------------------------
bool Board::genBlackFileRankAttacks(int ply, bool genCaps)
{
    Bitboard from_set( blackRooks | blackQueens );

    while (from_set) // solange noch Pos. on "from_set" vorhanden:
    {
        // Extrahieren
        int from_pos = from_set.lsb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_rankFileAttacks(from_pos);
        Bitboard to_set(mmx_m7ToBitboard());
#else
        // Hole alle Felder, die von from_pos aus horizontal/vertikal erreicht werden können
        Bitboard all(~freeSquares);
        Bitboard to_set ( BBTables::rank_attacks[from_pos][all.GET_RANK(from_pos)] 
            | BBTables::file_attacks[from_pos][allPieces90C.GET_RANK(rot90C_bitIndex[from_pos])] );
#endif

        Bitboard caps(to_set & whitePieces);
        to_set &= freeSquares;

        int piece = ROOK;
        if (BBTables::bbmask[from_pos] & blackQueens)
            piece = QUEEN;

        Move m(from_pos | piece << 16);

        while (to_set) // Solange noch Pos. in "to_set"
        {
            // Extrah.
            m.setTo(to_set.msb());
            to_set ^= BBTables::bbmask[m.to()];

            pCurMoveStack->push(m);
        }

        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps) // Solange noch Pos. in "caps"
            {
                // Extrah.
                m.setTo(caps.msb());
                caps ^= BBTables::bbmask[m.to()];

                int cap_piece = getWhitePieceFromBBs(m.to());

                // Illegale Position?
            
                if (cap_piece == KING)
                    return false;

                m.setCapturedPiece(cap_piece);
                pCurMoveStack->push(m);
            }
        }

    }

    return true;
}

// -------------------------------------------------------------------------
//  geradlinige schlagende schwarze Züge (Turm, Dame)
// -------------------------------------------------------------------------
bool Board::genBlackFileRankCaps(int ply)
{
    Bitboard from_set( blackRooks | blackQueens );

    while (from_set) // solange noch Pos. on "from_set" vorhanden:
    {
        // Extrahieren
        int from_pos = from_set.lsb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_rankFileAttacks(from_pos);
        Bitboard to_set(mmx_m7ToBitboard());
#else
        // Hole alle Felder, die von from_pos aus horizontal/vertikal erreicht werden können
        Bitboard all(~freeSquares);
        Bitboard to_set ( BBTables::rank_attacks[from_pos][all.GET_RANK(from_pos)] 
            | BBTables::file_attacks[from_pos][allPieces90C.GET_RANK(rot90C_bitIndex[from_pos])] );
#endif

        to_set &= whitePieces;

        int piece = ROOK;
        if (BBTables::bbmask[from_pos] & blackQueens)
            piece = QUEEN;

        Move m(from_pos | piece << 16 | Move::CAPTURE << 24);

        while (to_set) // Solange noch Pos. in "to_set"
        {
            // Extrah.
            m.setTo(to_set.lsb());
            to_set ^= BBTables::bbmask[m.to()];

            int cap_piece = getWhitePieceFromBBs(m.to());
            // Illegale Position?
            if (cap_piece == KING)
                return false;
            m.setCapturedPiece(cap_piece);

            pCurMoveStack->push(m);
        }
    }

    return true;
}


// -------------------------------------------------------------------------
//  diagonale weisse Züge (Läufer, Dame)
// -------------------------------------------------------------------------

bool Board::genWhiteDiagonalAttacks(int ply, bool genCaps)
{
    Bitboard from_set( whiteBishops | whiteQueens );

    while (from_set)  // noch Pos. in from_set?
    {
        // Extrahieren 
        int from_pos = from_set.msb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_diagAttacks(from_pos); 
        Bitboard to_set(mmx_m7ToBitboard());
#else

        Bitboard to_set( BBTables::diagH8A1_attacks[from_pos]
                        [allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[from_pos] )]
                     |   BBTables::diagH1A8_attacks[from_pos]
                        [allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[from_pos] )] );
#endif

        Bitboard caps(to_set & blackPieces);
        to_set &= freeSquares;

        int piece = BISHOP;
        if (BBTables::bbmask[from_pos] & whiteQueens)
            piece = QUEEN;

        Move m(from_pos | piece << 16);
        
        while (to_set)
        {
            int to_pos =  to_set.msb();
            to_set ^= BBTables::bbmask[to_pos];

            m.setTo(to_pos);
            pCurMoveStack->push(m);
        }

        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps)
            {
                int to_pos = caps.msb();
                caps ^= BBTables::bbmask[to_pos];

                int cap_piece = getBlackPieceFromBBs(to_pos);

                // Illegale Position?
                if (cap_piece == KING)
                    return false;

                m.setTo(to_pos);
                m.setCapturedPiece(cap_piece);

                pCurMoveStack->push(m);
            }
        }
    }


    return true;
}

// -------------------------------------------------------------------------
//  diagonale schlagende weisse Züge (Läufer, Dame)
// -------------------------------------------------------------------------

bool Board::genWhiteDiagonalCaps(int ply)
{
    Bitboard from_set( whiteBishops | whiteQueens );

    while (from_set)  // noch Pos. in from_set?
    {
        // Extrahieren 
        int from_pos = from_set.msb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_diagAttacks(from_pos); 
        Bitboard to_set(mmx_m7ToBitboard());
#else

        Bitboard to_set( BBTables::diagH8A1_attacks[from_pos]
                        [allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[from_pos] )]
                     |   BBTables::diagH1A8_attacks[from_pos]
                        [allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[from_pos] )] );
#endif

        to_set &= blackPieces;

        int piece = BISHOP;
        if (BBTables::bbmask[from_pos] & whiteQueens)
            piece = QUEEN;
        Move m(from_pos | piece << 16 | Move::CAPTURE << 24);
 
        while (to_set)
        {
            int to_pos = to_set.msb();
            to_set ^= BBTables::bbmask[to_pos];
  

            int cap_piece = getBlackPieceFromBBs(to_pos);

            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setTo(to_pos);
            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }

    return true;
}

// -------------------------------------------------------------------------
//  diagonale schwarze Züge (Läufer, Dame)
// -------------------------------------------------------------------------

bool Board::genBlackDiagonalAttacks(int ply, bool genCaps)
{
    Bitboard from_set( blackBishops | blackQueens );

    while (from_set)  // noch Pos. in from_set?
    {
        // Extrahieren 
        int from_pos = from_set.lsb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_diagAttacks(from_pos);
        Bitboard to_set(mmx_m7ToBitboard());
#else
        Bitboard to_set ( BBTables::diagH8A1_attacks[from_pos]
                            [allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[from_pos] )]
                |         BBTables::diagH1A8_attacks[from_pos]
                            [allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[from_pos] )] );
#endif

        Bitboard caps(to_set & whitePieces);
        to_set &= freeSquares;

        int piece = BISHOP;
        if (BBTables::bbmask[from_pos] & blackQueens)
            piece = QUEEN;

        Move m(from_pos | piece << 16);

        while (to_set)
        {
            int to_pos = to_set.lsb();
            to_set ^= BBTables::bbmask[to_pos];

            m.setTo(to_pos);
            pCurMoveStack->push(m);
        }

        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps)
            {
                int to_pos = caps.lsb();
                caps ^= BBTables::bbmask[to_pos];
  
                int cap_piece = getWhitePieceFromBBs(to_pos);
                // Illegale Position?
                if (cap_piece == KING)
                    return false;

                m.setTo(to_pos);
                m.setCapturedPiece(cap_piece);
                pCurMoveStack->push(m);
            }
        }

    }

    return true;
}

// -------------------------------------------------------------------------
//  diagonale schlagende schwarze Züge (Läufer, Dame)
// -------------------------------------------------------------------------

bool Board::genBlackDiagonalCaps(int ply)
{
    Bitboard from_set( blackBishops | blackQueens );

    while (from_set)  // noch Pos. in from_set?
    {
        // Extrahieren 
        int from_pos = from_set.lsb();
        from_set ^= BBTables::bbmask[from_pos];

#ifdef USE_MMXASM
        mmx_diagAttacks(from_pos);
        Bitboard to_set(mmx_m7ToBitboard());
#else
        Bitboard to_set ( BBTables::diagH8A1_attacks[from_pos]
                            [allPieces45C.GET_DIAG_H8A1( rot45C_bitIndex[from_pos] )]
                |         BBTables::diagH1A8_attacks[from_pos]
                            [allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[from_pos] )] );
#endif

        to_set &= whitePieces;

        int piece = BISHOP;
        if (BBTables::bbmask[from_pos] & blackQueens)
            piece = QUEEN;

        Move m(from_pos | piece << 16 | Move::CAPTURE << 24);

        while (to_set)
        {
            int to_pos = to_set.lsb();
            to_set ^= BBTables::bbmask[to_pos];
  
            int cap_piece = getWhitePieceFromBBs(to_pos);
            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setTo(to_pos);
            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }

    return true;
}



// -------------------------------------------------------------------------
//  weisse Springerzüge
// -------------------------------------------------------------------------
bool Board::genWhiteKnightAttacks(int ply, bool genCaps)
{
    Bitboard from_set(whiteKnights);

    while (from_set)    // noch Pos. vorhanden?
    {
        //Move m(MSB(from_set),0,KNIGHT,0);
        Move m(from_set.msb() | KNIGHT<<16);
        from_set ^= BBTables::bbmask[m.from()];

        Bitboard to_set( BBTables::knight_attacks[m.from()]);
        Bitboard caps(to_set & blackPieces);

        to_set &= freeSquares;

        while (to_set)  // noch Zielpos.?
        {
            m.setTo(to_set.msb());
            to_set ^= BBTables::bbmask[m.to()];

            pCurMoveStack->push(m);
        }

        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps)  // noch Zielpos.?
            {
                m.setTo(caps.msb());
                caps ^= BBTables::bbmask[m.to()];

                int cap_piece = getBlackPieceFromBBs(m.to());
                // Illegale Position?
                if (cap_piece == KING)
                    return false;

                m.setCapturedPiece(cap_piece);
                pCurMoveStack->push(m);
            }
        }

    }

    return true;
}

// -------------------------------------------------------------------------
//  schlagende weisse Springerzüge
// -------------------------------------------------------------------------
bool Board::genWhiteKnightCaps(int ply)
{
    Bitboard from_set(whiteKnights);

    while (from_set)    // noch Pos. vorhanden?
    {
        //Move m(MSB(from_set),0,KNIGHT,0);
        Move m(from_set.msb() | KNIGHT<<16 | Move::CAPTURE<<24);
        from_set ^= BBTables::bbmask[m.from()];

        Bitboard to_set( BBTables::knight_attacks[m.from()] & blackPieces );

        while (to_set)  // noch Zielpos.?
        {
            m.setTo(to_set.msb());
            to_set ^= BBTables::bbmask[m.to()];

            int cap_piece = getBlackPieceFromBBs(m.to());
            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
 
        }
    }

    return true;
}



// -------------------------------------------------------------------------
//  schwarze Springerzüge
// -------------------------------------------------------------------------
bool Board::genBlackKnightAttacks(int ply, bool genCaps)
{
    Bitboard from_set(blackKnights);

    while (from_set)    // noch Pos. vorhanden?
    {
        Move m(from_set.lsb() | KNIGHT << 16);
        from_set ^= BBTables::bbmask[m.from()];

        Bitboard to_set(BBTables::knight_attacks[m.from()]);
        Bitboard caps(to_set & whitePieces);

        to_set &= freeSquares;

        while (to_set)  // noch Zielpos.?
        {
            m.setTo(to_set.lsb());
            to_set ^= BBTables::bbmask[m.to()];

            pCurMoveStack->push(m);
        }

        if (genCaps)
        {
            m.setSpecial(Move::CAPTURE);
            while (caps)  // noch Zielpos.?
            {
                m.setTo(caps.lsb());
                caps ^= BBTables::bbmask[m.to()];

                int cap_piece = getWhitePieceFromBBs(m.to());
                // Illegale Position?
                if (cap_piece == KING)
                    return false;

                m.setCapturedPiece(cap_piece);
                pCurMoveStack->push(m);
            }
        }
    }

    return true;
}

// -------------------------------------------------------------------------
//  schlagende schwarze Springerzüge
// -------------------------------------------------------------------------
bool Board::genBlackKnightCaps(int ply)
{
    Bitboard from_set(blackKnights);

    while (from_set)    // noch Pos. vorhanden?
    {
        Move m(from_set.lsb() | KNIGHT << 16 | Move::CAPTURE << 24);
        from_set ^= BBTables::bbmask[m.from()];

        Bitboard to_set(BBTables::knight_attacks[m.from()] & whitePieces);

        while (to_set)  // noch Zielpos.?
        {
            m.setTo(to_set.lsb());
            to_set ^= BBTables::bbmask[m.to()];

            int cap_piece = getWhitePieceFromBBs(m.to());
            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }

    return true;
}



// -------------------------------------------------------------------------
// weisse Königszüge
// -------------------------------------------------------------------------

bool Board::genWhiteKingAttacks(int ply, bool genCaps)
{

    Bitboard to_set(BBTables::king_attacks[whiteKingPos]);
    Bitboard caps(to_set & blackPieces);

    to_set &= freeSquares;

    while (to_set)  // noch Zielpos.?
    {
        Move m(whiteKingPos | to_set.msb() << 8 | KING << 16);
        to_set ^= BBTables::bbmask[m.to()];

        pCurMoveStack->push(m);
    }

    if (genCaps)
    {
        while (caps)  // noch Zielpos.?
        {
            Move m(whiteKingPos | caps.msb() << 8 | KING << 16 | Move::CAPTURE<<24);
            caps ^= BBTables::bbmask[m.to()];

            int cap_piece = getBlackPieceFromBBs(m.to());
            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }


    // Rochaden

    if (castle & CASTLE_WHITE_KINGSIDE)
    {
        // Felder frei?
        if (  ! ((BBTables::bbmaskF1G1) & (~freeSquares)) 
            && whiteKingPos == 60 && (whiteRooks & BBTables::bbmask[63]))
        {
            // Felder nicht angegriffen?
            if (!isAttacked(61,BLACK) && !isAttacked(62,BLACK) && !isAttacked(60,BLACK))
            {
                // alles ok:
                Move m(60,62,KING,Move::CASTLE);
                pCurMoveStack->push(m);
            }
        }
    }
    if (castle & CASTLE_WHITE_QUEENSIDE)
    {
        // Felder frei?
        if ( ! ((BBTables::bbmaskB1C1D1) & (~freeSquares))
            && whiteKingPos == 60 && (whiteRooks & BBTables::bbmask[56]))
        {
            // Felder nicht angegriffen?
            if (!isAttacked(58,BLACK) && !isAttacked(59,BLACK) && !isAttacked(60,BLACK))
            {
                // alles ok:
                Move m(60,58,KING,Move::CASTLE);
                pCurMoveStack->push(m);
            }
        }
    }

    return true;
}

// -------------------------------------------------------------------------
// schlagende weisse Königszüge
// -------------------------------------------------------------------------

bool Board::genWhiteKingCaps(int ply)
{

    Bitboard to_set(BBTables::king_attacks[whiteKingPos] & blackPieces);

    while (to_set)  // noch Zielpos.?
    {
        Move m(whiteKingPos | to_set.msb() << 8 | KING << 16 | Move::CAPTURE << 24);
        to_set ^= BBTables::bbmask[m.to()];

        int cap_piece = getBlackPieceFromBBs(m.to());
        // Illegale Position?
        if (cap_piece == KING)
            return false;

        m.setCapturedPiece(cap_piece);
        pCurMoveStack->push(m);
    }


    return true;
}


// -------------------------------------------------------------------------
// schwarze Königszüge
// -------------------------------------------------------------------------

bool Board::genBlackKingAttacks(int ply, bool genCaps)
{
    Bitboard to_set(BBTables::king_attacks[blackKingPos]);
    Bitboard caps(to_set & whitePieces);

    to_set &= freeSquares;

    while (to_set)  // noch Zielpos.?
    {
        Move m(blackKingPos | to_set.lsb() << 8 | KING << 16);
        to_set ^= BBTables::bbmask[m.to()];

        pCurMoveStack->push(m);
    }

    if (genCaps)
    {
        while (caps)  // noch Zielpos.?
        {
            Move m(blackKingPos | caps.lsb() << 8 | KING << 16 | Move::CAPTURE << 24);
            caps ^= BBTables::bbmask[m.to()];

            int cap_piece = getWhitePieceFromBBs(m.to());
            // Illegale Position?
            if (cap_piece == KING)
                return false;

            m.setCapturedPiece(cap_piece);
            pCurMoveStack->push(m);
        }
    }


    // Rochaden

    if (castle & CASTLE_BLACK_KINGSIDE)
    {
        // Felder frei?
        if (  ! (BBTables::bbmaskF8G8 & (~freeSquares)) 
            && blackKingPos == 4 && (blackRooks & BBTables::bbmask[7]))
        {
            // Felder nicht angegriffen?
            if (!isAttacked(5,WHITE) && !isAttacked(6,WHITE) && !isAttacked(4,WHITE))
            {
                // alles ok:
                Move m(4,6,KING,Move::CASTLE);
                pCurMoveStack->push(m);
            }
        }
    }
    if (castle & CASTLE_BLACK_QUEENSIDE)
    {
        // Felder frei?
        if (   ! ( BBTables::bbmaskB8C8D8 & (~freeSquares))
            && blackKingPos == 4 && (blackRooks & BBTables::bbmask[0]))
        {
            // Felder nicht angegriffen?
            if (!isAttacked(2,WHITE) && !isAttacked(3,WHITE) && !isAttacked(4,WHITE))
            {
                // alles ok:
                Move m(4,2,KING,Move::CASTLE);
                pCurMoveStack->push(m);
            }
        }
    }

    return true;
}

// -------------------------------------------------------------------------
// schlagende schwarze Königszüge
// -------------------------------------------------------------------------

bool Board::genBlackKingCaps(int ply)
{
    Bitboard to_set(BBTables::king_attacks[blackKingPos] & whitePieces);

    while (to_set)  // noch Zielpos.?
    {
        Move m(blackKingPos | to_set.lsb() << 8 | KING << 16 | Move::CAPTURE << 24);
        to_set ^= BBTables::bbmask[m.to()];

        int cap_piece = getWhitePieceFromBBs(m.to());
        // Illegale Position?
        if (cap_piece == KING)
            return false;
        m.setCapturedPiece(cap_piece);
        pCurMoveStack->push(m);
    } 

    return true;
}

// -------------------------------------------------------------------------
//  Prüfen, ob die Position legal list, falls:
//  - sie aus Zug m hervorgegangen ist
//  - vor dem Zug m es kein Schach gegeben hat
// -------------------------------------------------------------------------
bool Board::positionLegalNoCheck(const Move& m) {
   int from = m.from();
   if (side == BLACK) {
      // -------------------------------------------------------------
      //  Weiss war am Zug
      // -------------------------------------------------------------
      if (!m.isEpCapture() &&
         (BBTables::all_mask[from] & whiteKing) == 0) 
         return true;
      if ((blackRooks|blackQueens) & BBTables::filerank_mask[whiteKingPos])
         if (rankFileAttacks(whiteKingPos) & (blackRooks | blackQueens))
            return false;
      if ((blackBishops|blackQueens) & BBTables::alldiag_mask[whiteKingPos])
         if (diagAttacks(whiteKingPos) & (blackBishops | blackQueens))
            return false;

      if (m.getPiece() == KING) {
         if (BBTables::whitePawnCaps[whiteKingPos] & blackPawns)
            return false;
         if (BBTables::knight_attacks[whiteKingPos] & blackKnights)
            return false;
         if (BBTables::king_attacks[whiteKingPos] & blackKing)
            return false;
      }
   } else {
      // -------------------------------------------------------------
      //  Schwarz war am Zug
      // -------------------------------------------------------------
      if (!m.isEpCapture() && 
          (BBTables::all_mask[from] & blackKing) == 0) 
         return true;
      if ((whiteRooks|whiteQueens) & BBTables::filerank_mask[blackKingPos])
         if (rankFileAttacks(blackKingPos) & (whiteRooks | whiteQueens))
            return false;
      if ((whiteBishops|whiteQueens) & BBTables::alldiag_mask[blackKingPos])
         if (diagAttacks(blackKingPos) & (whiteBishops | whiteQueens))
            return false;

      if (m.getPiece() == KING) {
         if (BBTables::blackPawnCaps[blackKingPos] & whitePawns)
            return false;
         if (BBTables::knight_attacks[blackKingPos] & whiteKnights)
            return false;
         if (BBTables::king_attacks[blackKingPos] & whiteKing)
            return false;
      }
   }

   return true;
}

// -------------------------------------------------------------------------
// isAttacked
// prüft, ob Feld "pos" von der Seite "s" angegriffen wird:
// -------------------------------------------------------------------------

inline bool Board::isAttacked(int pos, ColorEnum s)
{

    if (s == WHITE)
    {
        if (BBTables::blackPawnCaps[pos] & whitePawns)
            return true;
        if (BBTables::knight_attacks[pos] & whiteKnights)
            return true;
#ifdef USE_MMXASM
        // ---------------------------------------------------
        //          Mit MMX
        // ---------------------------------------------------

        if ((whiteRooks|whiteQueens) & BBTables::filerank_mask[pos])
        {
            mmx_rankFileAttacks(pos);
            if (mmx_hasWhiteQueenRook())
                return true; 
        }

        if ((whiteBishops|whiteQueens) & BBTables::alldiag_mask[pos])
        {
            mmx_diagAttacks(pos);
            if (mmx_hasWhiteQueenBishop())
                return true; 
        }
#else   
        // ---------------------------------------------------
        //          Kein MMX
        // ---------------------------------------------------
        if ((whiteRooks|whiteQueens) & BBTables::filerank_mask[pos])
            if (rankFileAttacks(pos) & (whiteRooks | whiteQueens))
                return true;
        
        if ((whiteBishops|whiteQueens) & BBTables::alldiag_mask[pos])
            if (diagAttacks(pos) & (whiteBishops | whiteQueens))
                return true;
#endif 


        if (BBTables::king_attacks[pos] & whiteKing)
            return true;
    }
    else
    {
        if (BBTables::whitePawnCaps[pos] & blackPawns)
            return true;
        if (BBTables::knight_attacks[pos] & blackKnights)
            return true;
#ifdef USE_MMXASM
        // ---------------------------------------------------
        //          Mit MMX
        // ---------------------------------------------------
        if ((blackRooks|blackQueens) & BBTables::filerank_mask[pos])
        {
            mmx_rankFileAttacks(pos);
            if (mmx_hasBlackQueenRook())
                return true;
        }

        if ((blackBishops|blackQueens) & BBTables::alldiag_mask[pos])
        {
            mmx_diagAttacks(pos);
            if (mmx_hasBlackQueenBishop())
                return true; 
        }
#else
        // ---------------------------------------------------
        //          Kein MMX
        // ---------------------------------------------------
        if ((blackRooks|blackQueens) & BBTables::filerank_mask[pos])
            if (rankFileAttacks(pos) & (blackRooks | blackQueens))
                return true;
        if ((blackBishops|blackQueens) & BBTables::alldiag_mask[pos])
            if (diagAttacks(pos) & (blackBishops | blackQueens))
                return true;
#endif // USE_MMXASM

        if (BBTables::king_attacks[pos] & blackKing)
            return true;
    }

    return false;
}

// -------------------------------------------------------------------------
// getAttackBoard
// Erstellt ein Bitboard mit allen Figuren der Farbe "s", die das Feld
// "pos" angreifen
// -------------------------------------------------------------------------

BITBOARD Board::getAttackBoard(int pos, ColorEnum s)
{
    BITBOARD att = 0;

    if (s == WHITE)
    {
        att |= BBTables::blackPawnCaps[pos] & whitePawns;
        att |= BBTables::knight_attacks[pos] & whiteKnights;
        if ((whiteRooks|whiteQueens) & BBTables::filerank_mask[pos])
            att |= rankFileAttacks(pos) & (whiteRooks | whiteQueens);
        if ((whiteBishops|whiteQueens) & BBTables::alldiag_mask[pos])
            att |= diagAttacks(pos) & (whiteBishops | whiteQueens);
        att |= BBTables::king_attacks[pos] & whiteKing;
    }
    else
    {
        att |= BBTables::whitePawnCaps[pos] & blackPawns;
        att |= BBTables::knight_attacks[pos] & blackKnights;
        if ((blackRooks|blackQueens) & BBTables::filerank_mask[pos])
            att |= rankFileAttacks(pos) & (blackRooks | blackQueens);
        if ((blackBishops|blackQueens) & BBTables::alldiag_mask[pos])
            att |= diagAttacks(pos) & (blackBishops | blackQueens);
        att |= BBTables::king_attacks[pos] & blackKing;
    }

    return att;
}

// -------------------------------------------------------------------------
// getAttackBoard
// Erstellt ein Bitboard mit allen Figuren (schwarz und weiss), die das Feld
// "pos" angreifen
// -------------------------------------------------------------------------
BITBOARD Board::getFullAttackBoard(int pos)
{

    Bitboard att(BBTables::blackPawnCaps[pos] & whitePawns);
    att |= BBTables::whitePawnCaps[pos] & blackPawns;
    att |= BBTables::knight_attacks[pos] & (whiteKnights | blackKnights);
    att |= rankFileAttacks(pos) & (whiteRooks | whiteQueens| blackRooks | blackQueens);
    att |= diagAttacks(pos) & (whiteBishops | whiteQueens | blackBishops | blackQueens);
    att |= BBTables::king_attacks[pos] & (whiteKing | blackKing);


 /*   mmx_rankFileAttacks(pos);
    Bitboard t1(mmx_m7ToBitboard());
    att |= t1 & (whiteRooksQueens | blackRooksQueens);
    mmx_diagAttacks(pos);
    t1 = mmx_m7ToBitboard();
    att |= t1 & (whiteBishopsQueens | blackBishopsQueens); */

    return att;
}

// -------------------------------------------------------------------------
// Prüfen, ob die am Zug befindliche Seite im Schach steht
// -------------------------------------------------------------------------
bool Board::inCheck()
{
    if (side==WHITE)
        return isAttacked(whiteKingPos,BLACK);
    else
        return isAttacked(blackKingPos,WHITE);
}

// -------------------------------------------------------------------------
//  Prüfen, ob die Seite, die den letzten Zug gemacht hat im Schach steht
// -------------------------------------------------------------------------

bool Board::positionLegal()
{
    if (side==BLACK)
        return !isAttacked(whiteKingPos,BLACK);
    else
        return !isAttacked(blackKingPos,WHITE);
}


// -------------------------------------------------------------------------
// kingAttacked
// prüft, ob der König der Seite "s" angegriffen wird
// -------------------------------------------------------------------------

bool Board::kingAttacked(ColorEnum s)
{
    if (s==WHITE)
        return isAttacked(whiteKingPos,BLACK);
    else
        return isAttacked(blackKingPos,WHITE);
}

// -------------------------------------------------------------------------
//  getKingAttackBoard
//  Erstellt Bitboard aller Figuren der Farbe XSIDE(s), die den König der 
//  Farbe "s" angreifen.
// -------------------------------------------------------------------------
BITBOARD Board::getKingAttackBoard(ColorEnum s)
{
    if (s == WHITE)
        return getAttackBoard(whiteKingPos, XSIDE(s));
    else
        return getAttackBoard(blackKingPos, XSIDE(s));

}

// -------------------------------------------------------------------------
//  pinnedByWhitePiece
//  Prüft, ob die Figur bei pos an die Figur bei to durch eine weisse Figur
//  gefesselt ist
// -------------------------------------------------------------------------
int Board::pinnedByWhitePiece(int pos, int to)
{
    int direction = BBTables::direction[pos][to];

    if (!direction) return 0;

    switch (direction)
    {
    case 1:
        {
            Bitboard all(whitePieces | blackPieces);
            Bitboard attacks = BBTables::rank_attacks[pos][all.GET_RANK(pos)];
            if (  (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (whiteQueens|whiteRooks)) != 0 )
                    return 1;
            else
                return 0;
        }
    case 2:
        {
            Bitboard attacks = BBTables::file_attacks[pos][allPieces90C.GET_RANK(rot90C_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (whiteQueens|whiteRooks)) != 0 )
                return 2;
            else 
                return 0;
        }
    case 3:
        {
            Bitboard attacks = BBTables::diagH1A8_attacks[pos][allPieces45AC.GET_DIAG_H8A1(rot45AC_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (whiteBishops|whiteQueens)) != 0 )
                return 3;
            else
                return 0;
        }
    case 4:
        {
            Bitboard attacks = BBTables::diagH8A1_attacks[pos][allPieces45C.GET_DIAG_H8A1(rot45C_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (whiteBishops|whiteQueens)) != 0)
                return 4;
            else
                return 0;

        }
    }

    return 0;

}

// -------------------------------------------------------------------------
//  pinnedByBlackPiece
//  Prüft, ob die Figur bei pos an die Figur bei to durch eine schwarze Figur
//  gefesselt ist
// -------------------------------------------------------------------------
int Board::pinnedByBlackPiece(int pos, int to)
{
    int direction = BBTables::direction[pos][to];

    if (!direction) return 0;

    switch (direction)
    {
    case 1:
        {
            Bitboard all(whitePieces | blackPieces);
            Bitboard attacks = BBTables::rank_attacks[pos][all.GET_RANK(pos)];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (blackQueens|blackRooks)) != 0 )
                return 1;
            else
                return 0;
        }
    case 2:
        {
            Bitboard attacks = BBTables::file_attacks[pos][allPieces90C.GET_RANK(rot90C_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (blackQueens|blackRooks)) != 0 )
                    return 2;
            else
                return 0;
        }
    case 3:
        {
            Bitboard attacks = BBTables::diagH1A8_attacks[pos][allPieces45AC.GET_DIAG_H8A1(rot45AC_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (blackBishops|blackQueens)) != 0 )
                return 3;
            else
                return 0;
        }
    case 4:
        {
            Bitboard attacks = BBTables::diagH8A1_attacks[pos][allPieces45C.GET_DIAG_H8A1(rot45C_bitIndex[pos])];
            if ( (attacks &   BBTables::bbmask[to]  ) != 0  && 
                    (attacks & (blackBishops|blackQueens)) != 0 )
                return 4;
            else
                return 0;
        }
    }

    return 0;
}

// -------------------------------------------------------------------------
// pinnedTo...King
// prüft, ob Figur auf Feld pos an den weissen/schwarzen König gefesselt ist.
// Die Funktion geht davon aus, dass bei pos eine weisse (schwarze) 
// Figur != dem König steht.
// -------------------------------------------------------------------------

int Board::pinnedToWhiteKing(int pos)
{
    return pinnedByBlackPiece(pos, whiteKingPos);
}
int Board::pinnedToBlackKing(int pos)
{
    return pinnedByWhitePiece(pos, blackKingPos);
}


// -------------------------------------------------------------------------
// rankFileAttacks
//
// BB mit allen Figuren, die von from_pos aus über eine gerade Linie
// angegriffen werden
// -------------------------------------------------------------------------
inline BITBOARD Board::rankFileAttacks(int from_pos)
{
    Bitboard temp(whitePieces | blackPieces);
    // Hole alle Felder, die von from_pos aus horizontal/vertikal erreicht werden können
    return  BBTables::rank_attacks[from_pos][temp.GET_RANK(from_pos)]
        |  BBTables::file_attacks[from_pos]
             [allPieces90C.GET_RANK(rot90C_bitIndex[from_pos])]; 

}

// -------------------------------------------------------------------------
inline BITBOARD Board::diagAttacks(int from_pos)
{
    return  BBTables::diagH8A1_attacks[from_pos]
                [ allPieces45C.GET_DIAG_H8A1(rot45C_bitIndex[from_pos]) ]
       |    BBTables::diagH1A8_attacks[from_pos]
                [ allPieces45AC.GET_DIAG_H8A1( rot45AC_bitIndex[from_pos]) ];

}



// -------------------------------------------------------------------------
//    Figur an Position 'pos' mittels der Bitboards bestimmen
// -------------------------------------------------------------------------
int Board::getWhitePieceFromBBs(int pos) const
{
    if (BBTables::bbmask[pos] & whitePawns)
        return PAWN;
    if (BBTables::bbmask[pos] & whiteKnights)
        return KNIGHT;
    if (BBTables::bbmask[pos] & whiteBishops) 
        return BISHOP;
    if (BBTables::bbmask[pos] & whiteRooks)
        return ROOK;
    if (BBTables::bbmask[pos] & whiteQueens)
        return QUEEN;
    if (BBTables::bbmask[pos] & whiteKing)
        return KING; 

    return NO_PIECE;
}

int Board::getBlackPieceFromBBs(int pos) const
{
    if (BBTables::bbmask[pos] & blackPawns)
        return PAWN;
    if (BBTables::bbmask[pos] & blackKnights)
        return KNIGHT;
    if (BBTables::bbmask[pos] & blackBishops) 
        return BISHOP;
    if (BBTables::bbmask[pos] & blackRooks)
        return ROOK;
    if (BBTables::bbmask[pos] & blackQueens)
        return QUEEN;
    if (BBTables::bbmask[pos] & blackKing)
        return KING;

    return NO_PIECE;
}

void Board::getPieceFromBBs(int pos, int& color, int& piece) const
{
    color = WHITE;
    piece = getWhitePieceFromBBs(pos);
    if (piece != NO_PIECE)
        return;

    color = BLACK;
    piece = getBlackPieceFromBBs(pos);
    if (piece != NO_PIECE)
        return;

    color = EMPTY;
    piece = NO_PIECE;
    return;
}


// -------------------------------------------------------------------------
//  whitePieceOnBoard ... returns true, if the white piece 'piece' is at
//                        position 'pos'
// -------------------------------------------------------------------------
bool Board::whitePieceOnBoard(int piece, int pos)
{
    switch (piece)
    {
    case PAWN:
        return (whitePawns & BBTables::bbmask[pos]) != 0;
    case KNIGHT:
        return (whiteKnights & BBTables::bbmask[pos]) != 0;
    case BISHOP:
        return (whiteBishops & BBTables::bbmask[pos]) != 0;
    case ROOK:
        return (whiteRooks & BBTables::bbmask[pos]) != 0;
    case QUEEN:
        return (whiteQueens & BBTables::bbmask[pos]) != 0;
    case KING:
        return whiteKingPos == pos;
    default:
        return false;
    }
}

// -------------------------------------------------------------------------
//  blackPieceOnBoard ... returns true, if the black piece 'piece' is at
//                        position 'pos'
// -------------------------------------------------------------------------
bool Board::blackPieceOnBoard(int piece, int pos)
{
    switch (piece)
    {
    case PAWN:
        return (blackPawns & BBTables::bbmask[pos]) != 0;
    case KNIGHT:
        return (blackKnights & BBTables::bbmask[pos]) != 0;
    case BISHOP:
        return (blackBishops & BBTables::bbmask[pos]) != 0;
    case ROOK:
        return (blackRooks & BBTables::bbmask[pos]) != 0;
    case QUEEN:
        return (blackQueens & BBTables::bbmask[pos]) != 0;
    case KING:
        return blackKingPos == pos;
    default:
        return false;
    }
}


// -------------------------------------------------------------------------
//  isLegal ..  test, if move m is (pseudo-)legal;
//              this function assumes that m is a legal move in _some_
//              position (but not neccesarily in the current position)
// -------------------------------------------------------------------------
bool Board::isLegal(const Move& m)
{
    // ---------------------------------------------
    //  nullmove isn't legal
    // ---------------------------------------------
    if (m==0) return false;

    return (sideToMove() == WHITE) ? whiteMoveLegal(m) : blackMoveLegal(m);
}

// -------------------------------------------------------------------------
//  test legality of white move
// -------------------------------------------------------------------------
bool Board::whiteMoveLegal(const Move& m)
{
    const int piece = m.getPiece();
    const int from  = m.from();
    const int to    = m.to();

    // ----------------------------------------------
    //  Check, whether piece is really on the board;
    //  if this is a capture, also check whether
    //  captured piece is on the board -- if this
    //  is not a capture, the "to-square" hast to be
    //  empty (ep-capture counts as "not a capture")
    // ----------------------------------------------

    if (!whitePieceOnBoard(piece, from)) return false;

    freeSquares = ~(whitePieces|blackPieces);
    if (m.isCapture())
    {
        if (!blackPieceOnBoard(m.getCapturedPiece(),to)) return false;
    }
    else
        if ((freeSquares & BBTables::bbmask[to]) == 0) return false;

    // ----------------------------------------------
    //  if this is a castle, do the usual tests to
    //  see if castling is legal in this position
    // ----------------------------------------------
    if (m.isCastleMove())
    {
        if (to == 62)           // king side
        {
            if (! (castle & CASTLE_WHITE_KINGSIDE)) return false;

            // squares not occupied and not attacked?
            if ( (BBTables::bbmaskF1G1 & ~freeSquares) == 0
                && whiteKingPos == 60 && (whiteRooks & BBTables::bbmask[63]))
                return (!isAttacked(61,BLACK) && !isAttacked(62,BLACK) && !isAttacked(60,BLACK));
            else
                return false;
        }
        else if (to == 58)      // queen side
        {
            if (! (castle & CASTLE_WHITE_QUEENSIDE)) return false;

            // squares not occupied and not attacked?
            if ( (BBTables::bbmaskB1C1D1 & ~freeSquares) == 0
                && whiteKingPos == 60 && (whiteRooks & BBTables::bbmask[56]))
                return (!isAttacked(58,BLACK) && !isAttacked(59,BLACK) && !isAttacked(60,BLACK));
            else
                return false;
        }
        return false;
    }

    // ----------------------------------------------
    //  pawn move
    // ----------------------------------------------
    if (m.isPawnMove())
    {
        if (from - to == 8) return true;
        if (m.isPawn2Squares())
            return  (freeSquares & BBTables::bbmask[from-8]) != 0;

        if (m.isEpCapture())
            return  ep == to && ((blackPawns&BBTables::bbmask[to+8]) != 0);

        // move must be a capture (or i'm screwed ;)
        return true;
    }

    // ----------------------------------------------
    //  check sliding pieces
    // ----------------------------------------------
    switch (piece)
    {
    case BISHOP:
        if (BBTables::direction[from][to] >= 3)
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    case ROOK:
        if (BBTables::direction[from][to] == 1 || BBTables::direction[from][to] == 2)
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    case QUEEN:
        if (BBTables::direction[from][to])
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    }

    // ----------------------------------------------
    //  move has to be legal
    // ----------------------------------------------
    return true;
}

// -------------------------------------------------------------------------
//  test legality of black move
// -------------------------------------------------------------------------
bool Board::blackMoveLegal(const Move& m)
{
    const int piece = m.getPiece();
    const int from  = m.from();
    const int to    = m.to();

    // ----------------------------------------------
    //  Check, whether piece is really on the board;
    //  if this is a capture, also check whether
    //  captured piece is on the board -- if this
    //  is not a capture, the "to-square" hast to be
    //  empty (ep-capture counts as "not a capture")
    // ----------------------------------------------

    if (!blackPieceOnBoard(piece, from)) return false;

    freeSquares = ~(whitePieces|blackPieces);
    if (m.isCapture())
    {
        if (!whitePieceOnBoard(m.getCapturedPiece(),to)) return false;
    }
    else
        if ((freeSquares & BBTables::bbmask[to]) == 0) return false;

    // ----------------------------------------------
    //  if this is a castle, do the usual tests to
    //  see if castling is legal in this position
    // ----------------------------------------------
    if (m.isCastleMove())
    {
        if (to == 6)
        {
            if (!(castle & CASTLE_BLACK_KINGSIDE)) return false;

            // squares not occupied and not attacked?
            if (  (BBTables::bbmaskF8G8 & ~freeSquares) == 0
                && blackKingPos == 4 && (blackRooks & BBTables::bbmask[7]))
                return !isAttacked(5,WHITE) && !isAttacked(6,WHITE) && !isAttacked(4,WHITE);
            else
                return false;
        }


        else if (to == 2)
        {
            if (!(castle & CASTLE_BLACK_QUEENSIDE)) return false;

            // squares not occupied and not attacked?
            if (  (BBTables::bbmaskB8C8D8 & ~freeSquares) == 0
                && blackKingPos == 4 && (blackRooks & BBTables::bbmask[0]))
                return !isAttacked(2,WHITE) && !isAttacked(3,WHITE) && !isAttacked(4,WHITE);
            else
                return false;
        }

        return false;
    }

    // ----------------------------------------------
    //  pawn move
    // ----------------------------------------------
    if (m.isPawnMove())
    {
        if (from + to == 8) return true;
        if (m.isPawn2Squares())
            return  (freeSquares & BBTables::bbmask[from+8]) != 0;

        if (m.isEpCapture())
            return  ep == to && ((whitePawns&BBTables::bbmask[to-8]) != 0);

        // move must be a capture (or i'm screwed ;)
        return true;
    }

    // ----------------------------------------------
    //  check sliding pieces
    // ----------------------------------------------
    switch (piece)
    {
    case BISHOP:
        if (BBTables::direction[from][to] >= 3)
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    case ROOK:
        if (BBTables::direction[from][to] == 1 || BBTables::direction[from][to] == 2)
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    case QUEEN:
        if (BBTables::direction[from][to])
        {
            if ((BBTables::squaresBetween[from][to] & ~freeSquares) != 0)
                return false;
        }
        else
            return false;
        break;
    }

    // ----------------------------------------------
    //  move has to be legal
    // ----------------------------------------------
    return true;
}


// -------------------------------------------------------------------------
//  SEE ... Abtauschfolgen statisch auswerten
// -------------------------------------------------------------------------
//
// Dies ist eine Art "Negamax-Variante" von SEE (statical exchange evaluator).
// Die Idee ist negamax mit nur zwei möglichen Aktionen zu implementieren:
// 1.) nicht Ziehen
// 2.) Schlagen mit der billigsten Figur.
// Der Aufruf erfolgt mit see_nm(0, target_sq, pos).
//
// int see_nm(int balance, Square target_sq, Position* pos)
// {
//    Square att_sq = leastValAttacker(target_sq, pos);
//
//    int v = -INF_SCORE;
//    if (att_sq != NOT_FOUND)
//    {
//      int new_balance = balance + value(pieceAt[target_sq]);
//      makemove(att_sq x target_sq, pos);
//      v = -see( -new_balance, target_sq, pos);
//    }
//    return max(balance, v);
//  }
//
//  Diese Implementation verzichtet allerdings auf Rekursion und verwendet  
//  stattdessen einen 'balance_stack'. (Idee aus swap.c, Crafty)
//

// -------------------------------------------------------------------------
// see_test: zum Testen der Korrektheit der  eigentlichen SEE Funktion
// -------------------------------------------------------------------------
int Board::see_test(int balance, int target_pos, int ply)
{
    // leastValAttacker finden

    bool kingCapture = genCaps(ply);

    if (!kingCapture)
        return balance + 10000;


    Move m_att = 0;
    int min_val = 10000;
    for (int i=0; i < moveStack[ply].size(); i++)
    {
        Move m = moveStack[ply].stack[i];

        if (!m.isCapture())
            continue;
        if (m.to() != target_pos)
            continue;

        if (piece_val[m.getPiece()] < min_val)
        {
            min_val = piece_val[m.getPiece()];
            m_att = m;
        }
    }


    int v = -10000;
    if (min_val != 10000)
    {
      int new_balance = balance + piece_val[m_att.getCapturedPiece()];
      //cout << "see_test: capture ..." << m_att << ", balance = " << new_balance <<  endl;
      makemove(m_att);
      v = -see_test( -new_balance, target_pos, ply + 1);
      takebackmove();
    }

    if (v > balance)
        return v;
    else
        return balance;
}


//#define DEBUG_SEE
int Board::see(const Move& m)
{
    // Stack für 'balance' Werte
    static int balance_stack[32];
    // stack pointer 
    int sp = 0; 

    // Welche Farbe ist am Zug?
    bool wtm = side == WHITE;

    int target_pos = m.to();
    int att_pos    = m.from();
    int val        = piece_val[m.getCapturedPiece()];
    if (m.isPromotion())
    {
        if (m.isCapture())
            val += piece_val[m.getPromPiece()]-piece_val[PAWN];
        else
            val = piece_val[m.getPromPiece()]-piece_val[PAWN];
    }
    int new_val;

    // Attackboard
#ifdef USE_MMXASM
    mmx_getFullAttackBoard(target_pos);
    Bitboard attacks(mmx_m7ToBitboard());

#ifdef DEBUG_SEE
    // TEST
    Bitboard _att = getFullAttackBoard(target_pos);
    out << "_att:" << endl; //TTTT
    dumpBitBoardToCout(_att);
    if (_att != attacks)
    {
        cout << "_att" << endl;
        dumpBitBoardToCout(_att);
        cout << "attacks" << endl;
        dumpBitBoardToCout(attacks);
        cout << "Ooops!" << endl; 
        exit(1); 
    } 
#endif
#else
    Bitboard attacks = getFullAttackBoard(target_pos);
#endif


    // Felder, auf denen noch Angreifer zu finden sein koennen
    Bitboard allowed(~BBTables::bbmask[target_pos]);

    // 0 ist die Score, falls der erste Abtausch nicht erfolgt
    balance_stack[sp++] = 0;


    // 1.Angreifer vom Attackboard entfernen
    // Sonderfall Umwandlung ohne Schlagen: In diesem Fall ist der umgewandelte Bauer nicht
    // auf dem Attackboard eingetragen; attacks muss also nicht aktualisiert werden
    if (m.isCapture())
        attacks ^= BBTables::bbmask[att_pos]; 
    allowed ^= BBTables::bbmask[att_pos];
    balance_stack[sp++] = -val;
    if (!m.isPromotion())
        val = piece_val[m.getPiece()];
    else
    {
        val = piece_val[m.getPromPiece()];
    }

    // Richtung bestimmen, aus der Angriff erfolgte
    int direction = BBTables::direction[target_pos][att_pos];

    // Verdeckte Angriffe finden...
    if (direction)
    {
        updateAttacks(attacks, allowed, att_pos, direction);
    }


#ifdef DEBUG_SEE
    out << endl;
    out << "DEBUG SEE -- INIT" << endl;
    out << "balance[0] = " << balance_stack[0] << endl;
    out << "balance[1] = " << balance_stack[1] << endl;
    out << "wtm        = " << wtm << endl;
    out << "attacks:" << endl;
    dumpBitBoardToCout(attacks);
    out << "allowed:" << endl;
    dumpBitBoardToCout(allowed);
#endif


    wtm = !wtm;
    while (attacks)
    {
        // Billigsten Angreifer der richtigen Farbe holen
        if (wtm)
        {
            if (attacks & whitePawns)
            {
                att_pos = Bitboard(attacks & whitePawns).msb();
                new_val = piece_val[PAWN];
            }
            else if (attacks & whiteKnights)
            {
                att_pos = Bitboard(attacks & whiteKnights).msb();
                new_val = piece_val[KNIGHT];
            }
            else if (attacks & whiteBishops)
            {
                att_pos = Bitboard(attacks & whiteBishops).msb();
                new_val = piece_val[BISHOP];
            }
            else if (attacks & whiteRooks)
            {
                att_pos = Bitboard(attacks & whiteRooks).msb();
                new_val = piece_val[ROOK];
            }
            else if (attacks & whiteQueens)
            {
                att_pos = Bitboard(attacks & whiteQueens).msb();
                new_val = piece_val[QUEEN];
            }
            else if (attacks & whiteKing)
            {
                att_pos = whiteKingPos;
                new_val = 10000;
            }
            else
                break;
        }
        else
        {
            if (attacks & blackPawns)
            {
                att_pos = Bitboard(attacks & blackPawns).lsb();
                new_val = piece_val[PAWN];
            }
            else if (attacks & blackKnights)
            {
                att_pos = Bitboard(attacks & blackKnights).lsb();
                new_val = piece_val[KNIGHT];
            }
            else if (attacks & blackBishops)
            {
                att_pos = Bitboard(attacks & blackBishops).lsb();
                new_val = piece_val[BISHOP];
            }
            else if (attacks & blackRooks)
            {
                att_pos = Bitboard(attacks & blackRooks).lsb();
                new_val = piece_val[ROOK];
            }
            else if (attacks & blackQueens)
            {
                att_pos = Bitboard(attacks & blackQueens).lsb();
                new_val = piece_val[QUEEN];
            }
            else if (attacks & blackKing)
            {
                att_pos = blackKingPos;
                new_val = 10000;
            }
            else
                break;
        }

        // Angreifer entfernen
        attacks ^= BBTables::bbmask[att_pos];
        allowed ^= BBTables::bbmask[att_pos];

        // Verdeckte Angriffe finden ...
        // Richtung bestimmen, aus der Angriff erfolgte
        int direction = BBTables::direction[target_pos][att_pos];
        if (direction)
        {   
            updateAttacks(attacks, allowed, att_pos, direction);
        }

        // Materialgewinn für ausgeführten Zug pushen
        balance_stack[sp] = -(balance_stack[sp-1] + val);

#ifdef DEBUG_SEE
    out << "DEBUG SEE -- FOUND ATTACKER at " << att_pos << endl;

    out << "attacks: " << endl;
    dumpBitBoardToCout(attacks);
    out << "allowed: " << endl;
    dumpBitBoardToCout(allowed);
    out << "sp           = " << sp << endl;
    out << "balance[sp]  = " << balance_stack[sp] << endl;
    out << "wtm          = " << wtm << endl;
    out << "new_val x val= " << new_val << " x " << val << endl;
    out << endl;
#endif

        sp++;
        if (val == 10000) // König wurde geschlagen
            break;
        val = new_val;
        wtm = !wtm;
    }

    // Hier wäre die Rekursion terminiert; man muss den Stapel also
    // rückwärts herunter abarbeiten und das Maximum zwischen v
    // und balance bilden...

    int v = -50000;
    while (--sp)
    {
#ifdef DEBUG_SEE
        out << "-->sp    = " << sp << endl;
        out << "v        = " << v << " --- balance_stack[sp] = " 
              << balance_stack[sp] << endl;
#endif
        if (balance_stack[sp] > v)
            v = balance_stack[sp];
        v = -v;
    }
 

#ifdef DEBUG_VERIFY_SEE
    if (!makeSaveMove(m))
    {
        cout << "makeSaveMove(m) == false !! " << endl;
    }
    int _v = -see_test(-piece_val[m.getCapturedPiece()],target_pos,1);
    takebackmove();
    if (_v != v)
    {
        cout << endl;
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        cout << "SEE (TEST-VERSION): " << _v << endl;
        cout << "SEE               : " << v << endl;
        cout << "MOVE              : " << m <<  endl;
        dump();
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    }
#endif 


    return v;
}


// Es kann vorkommen, dass innerhalb einer Abtauschfolge neue Angreifer
// sichtbar werden (z.B. der Turm auf F1, nach dem der andere Turm
// Rf2xf5 spielt). updateAttacks sorgt dafür, dass solche Angreifer
// zu 'attacks' dazugeORed werden.
void Board::updateAttacks(Bitboard& attacks, Bitboard& allowed, int att_pos, int direction)
{
    switch (direction)
    {
    case 1:
        {
        Bitboard all(whitePieces | blackPieces);
        attacks |= BBTables::rank_attacks[att_pos][all.GET_RANK(att_pos)] &
                   (whiteRooks | whiteQueens | blackRooks | blackQueens) &
                   allowed; 
        break;
        }
    case 2:
        {
        attacks |= BBTables::file_attacks[att_pos][allPieces90C.GET_RANK(rot90C_bitIndex[att_pos])] &
                   (whiteRooks | whiteQueens | blackRooks | blackQueens) &
                   allowed;
        break;
        }
    case 3:
        {
        attacks |= BBTables::diagH1A8_attacks[att_pos][allPieces45AC.GET_DIAG_H8A1(rot45AC_bitIndex[att_pos])] &
                   (whiteBishops | whiteQueens | blackBishops | blackQueens) &
                   allowed;
        break;
        }
    case 4:
        {
        attacks |= BBTables::diagH8A1_attacks[att_pos][allPieces45C.GET_DIAG_H8A1(rot45C_bitIndex[att_pos])] &
                   (whiteBishops | whiteQueens | blackBishops | blackQueens) &
                   allowed;
        break;
        }
    }
}


// ------------------------------------------------------------------------------------------

#ifdef DEBUG_MOVEGEN
void Board::checkBoards(Move m, string func)
{
    bool err = false;

    // Königspositionen
    if (whiteKingPos != whiteKing.lsb())
    {
        out << " whiteKingPos    = " << whiteKingPos  << endl;
        out << " whiteKing.lsb() = " << whiteKing.lsb() << endl;
        err = true;
    }
    if (whiteKing.popcount() != 1)
    {
        out << " whiteKing.popcount() = " << whiteKing.popcount() << endl;
        err = true;
    }
    if (blackKingPos != blackKing.lsb())
    {
        out << " blackKingPos    = " << blackKingPos  << endl;
        out << " blackKing.lsb() = " << blackKing.lsb() << endl;
        err = true;
    }
    if (blackKing.popcount() != 1)
    {
        out << " blackKing.popcount() = " << blackKing.popcount() << endl;
        err = true;
    }


    // whitePieces / blackPieces

    if (whitePieces & blackPieces)
    {
        out << " whitePieces & blackPieces != 0" << endl;
        err = true;
    }

    if (whitePieces != (whiteKing|whiteRooks|whiteBishops|whiteKnights|whitePawns|whiteQueens))
    {
        out << " whitePieces != UNION(whiteXXX)" << endl;
        err = true;
    }
    if (blackPieces != (blackKing|blackRooks|blackBishops|blackKnights|blackPawns|blackQueens))
    {
        out << " blackPieces != UNION(blackXXX)" << endl;
        err = true;
    }


    // Soll ein Zug überprüft werden
    if (! (m == 0))
    {
        bool move_error = false;
        if (side == WHITE)
        {
            int p = getWhitePieceFromBBs(m.from());

            if (p != m.getPiece())
                move_error = true;

            if (m.isCapture())
            {
                if (getBlackPieceFromBBs(m.to()) != m.getCapturedPiece())
                    move_error = true;
            }
        }
        else
        {
            int p = getBlackPieceFromBBs(m.from());

            if (p != m.getPiece())
                move_error = true;

            if (m.isCapture())
            {
                if (getWhitePieceFromBBs(m.to()) != m.getCapturedPiece())
                    move_error = true;
            }
        }

        if (move_error)
        {
            out << "MOVE-ERROR: m = " << m << " (" << (unsigned) m.data.in << ")" << endl;
            err = true;
        }
    }

    if (err)
    {
        err = false;

        out << "*******************************************************" << endl;
        out << func << endl;
        dump();
        dumpBitBoards();
        out << "MOVE: " << m << endl;
        out << "*******************************************************" << endl;
        int xx ; cin >> xx;
    } 

}

#endif
// ------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------
//                          makemove
//                          ========
//
//  Führt den Zug "m" auf dem Brett aus. Bei m muss es sich um einen
//  pseudolegalen Zug handeln -  d.h. er muß in der von gen() aktuell
//  erzeugten Zugliste moveStack[ply] enthalten sein.
//  Aktualisiert:
//      - side
//      - totalHPly
//      - castle
//      - ep
//      - history
//      - alle Bitboards
//        - Hashwert für aktuelle Position
//
//  
//  Geändert 04.10.01: Legalität von Zügen wird jetzt mit gen bzw. den 
//                       Suchfunktionen sichergestellt
// -------------------------------------------------------------------------


void Board::makemove(const Move& m)
{
#ifdef DEBUG_MOVEGEN
    checkBoards(m,"makemove");
#endif

    // Info speichern (für takebackmove)
    history.add(totalHPly,m,castle,ep,fifty,age,z1_hash);


    // Zuganzahl erhöhen
    totalHPly++;

    // Spezialfall nullmove
    if (m.isNullmove())
    {
      ep = 0;
      fifty++;
      z1_hash ^= NULLMOVE_HASHVALUE;
      side = XSIDE(side);
      return;
    }


    // fifty, age aktualisieren
    if (!m.isCapture() && !m.isPawnMove())
      fifty++;
    else
    {
      fifty = 0;
      age++;
    }


    // Rochadeinfo aktualisieren
    const int from  = m.from();
    const int to    = m.to();
    castle &= castle_mask[from] & castle_mask[to];

    // Falls Doppelschritt: "e.p." eintragen!
    if (m.isPawn2Squares())
    if (side == WHITE)
      ep = from - 8;
    else 
      ep = from + 8;
    else
      ep = 0;

    // Königsposition aktualisieren
    if (m.getPiece() == KING)
    {
      if (side == WHITE)
      {
          whiteKingPos = to;
          if (m.isCastleMove())
              whiteKingCastled = true;
      }
      else
      {
          blackKingPos = to;
          if (m.isCastleMove())
              blackKingCastled = true;
      }
    }


    // Bitboards aktualisieren
    updateBitboards(m,false);

    // nächster Spieler
    side = XSIDE(side);

/*    // TEST!!!!!!!!
    int mat_score = 0;
    unsigned int w_mat = 0;
    unsigned int b_mat = 0;
    int n_w_total = 1;
    int n_b_total = 1;
    for (int c = PAWN; c <= QUEEN; c++)
    {
        mat_score += piece_val[c]*(stats.nWhitePieces[c] - stats.nBlackPieces[c]);

        if (stats.nWhitePieces[c])
            w_mat |= stats.setBitMask[c];
        if (stats.nBlackPieces[c])
            b_mat |= stats.setBitMask[c];

        n_w_total += stats.nWhitePieces[c];
        n_b_total += stats.nBlackPieces[c];
    }
    if (stats.matBlack != b_mat || stats.matWhite != w_mat || 
        stats.nBlackPiecesTotal != n_b_total || stats.nWhitePiecesTotal != n_w_total ||
        mat_score != stats.whiteMatScore)
    {
        cout << "!!!!!!!!!!!!" << endl;
        dumpStats();
        dump();
        exit(0);
    }
    // ENDE TEST !!!!!!!!! */


#ifdef DEBUG_HASHKEY_CALC
  // ---------------------------------------------------------------
  // Prüfen, ob die Aktualisierung der Hashkeys fehlerfrei verläuft
  // ---------------------------------------------------------------
  ByteBoard byb;

  initByteBoardFromBitBoards(byb);
  
  hash_t calc_z1_hash  = p_TT->z1_value(byb.piece,byb.color);
  hash_t calc_z2_hash  = p_TT->z2_value(byb.piece,byb.color);
  hash_t calc_zp1_hash = p_pawnTT->zp1_value(byb.piece,byb.color);
  hash_t calc_zp2_hash = p_pawnTT->zp2_value(byb.piece,byb.color);

  bool error=false;
  if (calc_z1_hash != z1_hash)
  {
      out << "ERROR z1_hash!" << endl;
      error = true;
  }
  if (calc_z2_hash != z2_hash)
  {
      out << "ERROR z1_hash!" << endl;
      error = true;
  }
  if (calc_zp1_hash != zp1_hash)
  {
      out << "ERROR zp1_hash!" << endl;
      error = true;
  }
  if (calc_zp2_hash != zp2_hash)
  {
      out << "ERROR zp2_hash!" << endl;
      error = true;
  }

  if (error)
  {
       dump();
       out << "MOVE WAS: " << m << endl;
       exit(1);
  }

#endif

}

// -------------------------------------------------------------------------
//  langsame Variante von makemove, die sicherstellt, dass der Zug m
//  legal ist
// -------------------------------------------------------------------------
bool Board::makeSaveMove(const Move& m)
{
    gen(0,true);

    Move gen_move = moveStack[0].findMove(m.from(),m.to(),m.getPromPiece());

    if (gen_move == 0)   // Zug nicht in der Liste gefunden
    {
        return false;
    }

    // Zug in Liste gefunden, aber Binaerdarstellung verschieden?
    // (Kann auch vorkommen :)
    if (gen_move.data != m.data)  
    {
        return false;
    }

    // Ab hier: Zug ist pseudolegal

    // Ist er auch legal?
    makemove(m);
    if (!positionLegal())
    {
        takebackmove();
        return false;
    }

    // Alle i.O.
    return true;

}

// -------------------------------------------------------------------------
//                          takebackmove
//                          ------------
//
//  Geht einen Zug zurück in der Historyliste.
//
// -------------------------------------------------------------------------

void  Board::takebackmove()
{


  // vorherigen Spieler wiederherstellen
  side = XSIDE(side);

  // Anzahl Züge zurücksetzen
  totalHPly--;


  // Zug holen

  Move m;
  history.get(totalHPly,m,castle,ep,fifty,age);



  // Spezialfall nullmove
  if (m.isNullmove())
  {
      z1_hash ^= NULLMOVE_HASHVALUE;
      return;
  }

  // Königsposition aktualisieren
  if (m.getPiece() == KING)
  {
      if (side == WHITE)
      {
          whiteKingPos = m.from();
          if (m.isCastleMove())
              whiteKingCastled = false;
      }
      else
      {
          blackKingPos = m.from();
          if (m.isCastleMove())
              blackKingCastled = false;
      }
  }
 

    // Bitboards wiederherstellen
    updateBitboards(m,true);


/*    // TEST!!!!!!!!
    int mat_score = (side == WHITE) ? stats.whiteMatScore : -stats.whiteMatScore;
    if (Eval::getHandle()->getMatScore(this) != mat_score)
    {
        cout << "!!!!!!!!!!!!" << endl;
        cout << "Eval ... getMatScore(this) == " << Eval::getHandle()->getMatScore(this) << endl;
        cout << "stats.matscore         == " << mat_score << endl;
        dumpStats();
        dump();
        exit(0);
    }
    // ENDE TEST !!!!!!!!!  */


#ifdef DEBUG_MOVEGEN
  checkBoards(m,"takebackmove");
#endif


}  


// -------------------------------------------------------------------------
//  Aktuellen Spielstatus ermitteln
// -------------------------------------------------------------------------
void Board::getGameStatus(int ply, bool& check, bool& mate, bool& remis, RemisEnum& rtype)
{
    check = mate = remis = false;

    // Schach?
    check = kingAttacked(side);

    bool foundLegal = false;

    for (int i=0; i < moveStack[ply].size(); i++)
    {
        makemove(moveStack[ply].stack[i]);
        foundLegal = !kingAttacked(XSIDE(side));
        takebackmove();

        if (foundLegal)
            break;
    }

    if (foundLegal)
        mate = false;
    else
    {
        if (check)
            mate = true;
        else
        {
            remis = true;
            rtype = REMIS_STALE_MATE;
        }
        return;
    }

    // 50 Zug Regel
    if (fifty >= 100)
    {
        rtype = REMIS_FIFTY;
        remis = true;
        return;
    }
    else if (repititions() >= 2)
    {
        rtype = REMIS_REPITITION;
        remis = true;
        return;
    }

    // Zu wenig Material?
    if ( materialRemis() )
    {
        rtype = REMIS_MATERIAL;
        remis = true;
        return;
    }

}


// -------------------------------------------------------------------------
//  Erkennen, ob ein Remis aus Materialmangel vorliegt
// -------------------------------------------------------------------------
bool Board::materialRemis() const
{
    if ( (stats.nBlackPiecesTotal <= 3 && stats.nWhitePiecesTotal ==1)  ||
         (stats.nBlackPiecesTotal == 1 && stats.nWhitePiecesTotal <=3) )
    {
        // K - K
        if (stats.nBlackPiecesTotal == 1 && stats.nWhitePiecesTotal == 1)
        {
            return true;
        }

        if (stats.nBlackPiecesTotal == 1)
        {
            // KN - K,  KB - K, KNN - K
            if ( (stats.nWhitePiecesTotal == 2 && 
                 (stats.nWhitePieces[KNIGHT] + stats.nWhitePieces[BISHOP] == 1)) || 
                 (stats.nWhitePiecesTotal == 3 && stats.nWhitePieces[KNIGHT] == 2) )
            {
                return true;
            }
        }
        else
        {
            // K - KN,  K - KB, K - KNN
            if ( (stats.nBlackPiecesTotal == 2 && 
                 (stats.nBlackPieces[KNIGHT] + stats.nBlackPieces[BISHOP] == 1)) || 
                 (stats.nBlackPiecesTotal == 3 && stats.nBlackPieces[KNIGHT] == 2) )
            {
                return true;
            }
        }
    }

    return false;
}


// -------------------------------------------------------------------------
//  Anzahl der Zugwiederholungen feststellen
//
//  Liefert ausgehend von der aktuellen Position P_n, die Anzahl identischer
//    Positionen der Partie (ausschliesslich P_n selbst)
//        repititions() == #{P_i, 1<=i<n| P_i = P_n  }
// -------------------------------------------------------------------------
int Board::repititions()
{
    int i;
    int count = 0;

    //if (fifty <= 3)
    //    return 1;

    i = totalHPly - 2;  // frühestens beim vorherigen Zug kann es eine Wdh. geben

    for (;i >= totalHPly - fifty; i -= 2)
    {
        if (i < 0) break;   // könnte bei FEN-Importen entstehen?!

        // History-Position identisch mit aktueller Position?
        if (history.identicalPositionsAt(i,z1_hash))
        {
            count++;
        }

    }


    return count;
}



// -------------------------------------------------------------------------
//  Figur von Bitboards setzen/entfernen
// -------------------------------------------------------------------------

FORCEINLINE void Board::toggleWhitePieceOnBitboards(int pos, int piece)
{
    whitePieces   ^= BBTables::bbmask[pos];
    allPieces45C  ^= BBTables::bbmask[rot45C_bitIndex[pos]];
    allPieces45AC ^= BBTables::bbmask[rot45AC_bitIndex[pos]];
    allPieces90C  ^= BBTables::bbmask[rot90C_bitIndex[pos]];

    switch (piece)
    {
    case PAWN:
        whitePawns ^= BBTables::bbmask[pos];
        break;
    case KNIGHT:
        whiteKnights ^= BBTables::bbmask[pos];
        break;
    case BISHOP:
        whiteBishops ^= BBTables::bbmask[pos];
        break;
    case ROOK:
        whiteRooks ^= BBTables::bbmask[pos];
        break;
    case QUEEN:
        whiteQueens ^= BBTables::bbmask[pos];
        break;
    }
}


FORCEINLINE void Board::toggleBlackPieceOnBitboards(int pos, int piece)
{
    blackPieces   ^= BBTables::bbmask[pos];
    allPieces45C  ^= BBTables::bbmask[rot45C_bitIndex[pos]];
    allPieces45AC ^= BBTables::bbmask[rot45AC_bitIndex[pos]];
    allPieces90C  ^= BBTables::bbmask[rot90C_bitIndex[pos]];

    switch (piece)
    {
    case PAWN:
        blackPawns ^= BBTables::bbmask[pos];
        break;
    case KNIGHT:
        blackKnights ^= BBTables::bbmask[pos];
        break;
    case BISHOP:
        blackBishops ^= BBTables::bbmask[pos];
        break;
    case ROOK:
        blackRooks ^= BBTables::bbmask[pos];
        break;
    case QUEEN:
        blackQueens ^= BBTables::bbmask[pos];
        break;
    }
}

// -------------------------------------------------------------------------
// aktualisieren der Bitboards, bei Ausführung von Zug "m"
// -------------------------------------------------------------------------
void Board::updateBitboards(const Move& m, bool restore)
{


    Bitboard move_mask(BBTables::bbmask[m.to()] | BBTables::bbmask[m.from()]);
 

    if (side == WHITE)
    {
        ////////////////////////////////////////////////////
        //  Zug war weiss                                 //
        ////////////////////////////////////////////////////

        const int piece     = m.getPiece();
        const int from   = m.from();

        // Hash aktualisieren
        z1_hash ^= p_TT->z1XorWhite(piece,from);
        z2_hash ^= p_TT->z2XorWhite(piece,from);


        // --- whitePieces (+45C,45AC,90C) aktualisieren ---
        const int to        = m.to();

#ifdef USE_MMXASM
        mmx_updateWhitePieces(from,to);
#else
        whitePieces     ^= move_mask;
        allPieces45C  ^= BBTables::bbmask45C[to]  | BBTables::bbmask45C[from];
        allPieces45AC ^= BBTables::bbmask45AC[to] | BBTables::bbmask45AC[from];
        allPieces90C  ^= BBTables::bbmask90C[to]  | BBTables::bbmask90C[from];  
#endif



        // --- falls geschlagen: schwarze BBs aktualisieren ---
        if (m.isCapture())
        {
            const int cap_piece = m.getCapturedPiece();

            z1_hash ^= p_TT->z1XorBlack(cap_piece,to);
            z2_hash ^= p_TT->z2XorBlack(cap_piece,to);

            if (cap_piece == PAWN)
            {
                zp1_hash ^= p_pawnTT->zp1XorBlack(to);
                zp2_hash ^= p_pawnTT->zp2XorBlack(to);
            }

            if (restore)
            {
                stats.addBPiece(cap_piece);
            }
            else
            {
                stats.removeBPiece(cap_piece);
            }

#ifdef USE_MMXASM
            mmx_toggleBlackPieceOnBitboards(to, cap_piece);
#else
            toggleBlackPieceOnBitboards(to, cap_piece);
#endif
        }

        // ------------------- INDIVIDUELLE BITBOARDS --------------------------
        // Umwandlung?
        if (m.isPromotion())
        {
            z1_hash  ^= p_TT->z1XorWhite(m.getPromPiece(),to);
            z2_hash  ^= p_TT->z2XorWhite(m.getPromPiece(),to);
            zp1_hash ^= p_pawnTT->zp1XorWhite(to);
            zp2_hash ^= p_pawnTT->zp2XorWhite(to);
        }
        else
        {
            z1_hash ^= p_TT->z1XorWhite(m.getPiece(),to);
            z2_hash ^= p_TT->z2XorWhite(m.getPiece(),to);
        }


        // --- Prüfen, ob Bauernzug ---
        if (m.isPawnMove())
        {
            // Weisse Bauern aktualisieren
            whitePawns ^= move_mask;

            // Bauern-Hashkey aktualisieren
            zp1_hash ^= p_pawnTT->zp1XorWhite(from);
            zp1_hash ^= p_pawnTT->zp1XorWhite(to);
            zp2_hash ^= p_pawnTT->zp2XorWhite(from);
            zp2_hash ^= p_pawnTT->zp2XorWhite(to);

            // --- "e.p." Schlagen? ---
            if (m.isEpCapture())
            {
                if (restore)
                {
                    stats.addBPiece(PAWN);
                }
                else
                {
                    stats.removeBPiece(PAWN);
                }
                 
#ifdef USE_MMXASM
                mmx_toggleBlackPieceOnBitboards(to+8,PAWN);
#else
                toggleBlackPieceOnBitboards(to+8,PAWN);
#endif

                z1_hash  ^= p_TT->z1XorBlack(PAWN,to+8);
                z2_hash  ^= p_TT->z2XorBlack(PAWN,to+8);
                zp1_hash ^= p_pawnTT->zp1XorBlack(to+8);
                zp2_hash ^= p_pawnTT->zp2XorBlack(to+8);
                return;
            }

            // --- Umdwandlungsfeld erreicht? ---
            if (m.isPromotion())
            {
                if (restore)
                {
                    stats.addWPiece(PAWN);
                    stats.removeWPiece(m.getPromPiece());
                }
                else
                {
                    stats.removeWPiece(PAWN);
                    stats.addWPiece(m.getPromPiece());
                }

                whitePawns ^= BBTables::bbmask[to];
                
                switch (m.getPromPiece())
                {
                case QUEEN:
                    whiteQueens  ^= BBTables::bbmask[to]; break;
                case ROOK:
                    whiteRooks   ^= BBTables::bbmask[to]; break;
                case BISHOP:
                    whiteBishops ^= BBTables::bbmask[to]; break;
                case KNIGHT:
                    whiteKnights ^= BBTables::bbmask[to]; break;
                }
                return;
            }
            
            return;

        }

        // --- Kein Bauernzug - anderes Figuren-BB aktualisieren ---

        switch (piece)
        {
        case BISHOP:
            whiteBishops ^= move_mask;
            break;
        case KNIGHT:
            whiteKnights ^= move_mask;
            break;
        case ROOK:
            whiteRooks ^= move_mask;
            break;
        case QUEEN:
            whiteQueens ^= move_mask;
            break;
        case KING:
           whiteKing ^= move_mask;

            // Rochade?
            if (m.isCastleMove())
            {
                int ro_from, ro_to;
                // Turm bewegen
                if (to == 62)
                {
                    ro_from = 63;
                    ro_to = 61;
                }
                else
                {
                    ro_from = 56;
                    ro_to = 59;
                }
                z1_hash ^= p_TT->z1XorWhite(ROOK,ro_to);
                z2_hash ^= p_TT->z2XorWhite(ROOK,ro_to);
                z1_hash ^= p_TT->z1XorWhite(ROOK,ro_from);
                z2_hash ^= p_TT->z2XorWhite(ROOK,ro_from);

                whiteRooks        ^= BBTables::bbmask[ro_to] | BBTables::bbmask[ro_from];
                whitePieces     ^= BBTables::bbmask[ro_to] | BBTables::bbmask[ro_from];
                allPieces45C  ^= BBTables::bbmask[rot45C_bitIndex[ro_to]]  | 
                                   BBTables::bbmask[rot45C_bitIndex[ro_from]];
                allPieces45AC ^= BBTables::bbmask[rot45AC_bitIndex[ro_to]] | 
                                   BBTables::bbmask[rot45AC_bitIndex[ro_from]];
                allPieces90C  ^= BBTables::bbmask[rot90C_bitIndex[ro_to]]  | 
                                   BBTables::bbmask[rot90C_bitIndex[ro_from]];
            }

        }
    }
    else
    {
        ////////////////////////////////////////////////////
        //  Zug war schwarz                               //
        ////////////////////////////////////////////////////

        // Hash aktualisieren
        const int piece     = m.getPiece();
        const int from   = m.from();
        z1_hash ^= p_TT->z1XorBlack(piece,m.from());
        z2_hash ^= p_TT->z2XorBlack(piece,m.from());

        // --- blackPieces (+45C,45AC,90C) aktualisieren ---

        const int to = m.to();
#ifdef USE_MMXASM
        mmx_updateBlackPieces(from,to);
#else
        blackPieces   ^= move_mask;
        allPieces45C  ^= BBTables::bbmask45C[to]   | BBTables::bbmask45C[from];
        allPieces45AC ^= BBTables::bbmask45AC[to]  | BBTables::bbmask45AC[from];
        allPieces90C  ^= BBTables::bbmask90C[to]   | BBTables::bbmask90C[from]; 
#endif

        // --- falls geschlagen: weisse BBs aktualisieren ---
        if (m.isCapture())
        {
            const int cap_piece = m.getCapturedPiece();

            z1_hash ^= p_TT->z1XorWhite(cap_piece,to);
            z2_hash ^= p_TT->z2XorWhite(cap_piece,to);
            if (cap_piece == PAWN)
            {
                zp1_hash ^= p_pawnTT->zp1XorWhite(to);
                zp2_hash ^= p_pawnTT->zp2XorWhite(to);
            }

            if (restore)
            {
                stats.addWPiece(cap_piece);
            }
            else
            {
                stats.removeWPiece(cap_piece);
            }

#ifdef USE_MMXASM
            mmx_toggleWhitePieceOnBitboards(to,cap_piece);
#else
            toggleWhitePieceOnBitboards(to,cap_piece);
#endif
        }

        // ------------------- INDIVIDUELLE BITBOARDS --------------------------

        // Umwandlung?
        if (m.isPromotion())
        {
            z1_hash  ^= p_TT->z1XorBlack(m.getPromPiece(),to);
            z2_hash  ^= p_TT->z2XorBlack(m.getPromPiece(),to);
            zp1_hash ^= p_pawnTT->zp1XorBlack(to);
            zp2_hash ^= p_pawnTT->zp2XorBlack(to);
        }
        else
        {
            z1_hash ^= p_TT->z1XorBlack(piece,to);
            z2_hash ^= p_TT->z2XorBlack(piece,to);
        }

        // --- Prüfen, ob Bauernzug ---
        if (m.isPawnMove())
        {
            // Schwarze Bauern aktualisieren
            blackPawns ^= move_mask;

            // Bauern-Hashkey aktualisieren
            zp1_hash ^= p_pawnTT->zp1XorBlack(from);
            zp1_hash ^= p_pawnTT->zp1XorBlack(to);
            zp2_hash ^= p_pawnTT->zp2XorBlack(from);
            zp2_hash ^= p_pawnTT->zp2XorBlack(to);

            // --- "e.p." Schlagen? ---
            if (m.isEpCapture())
            {
                if (restore)
                {
                    stats.addWPiece(PAWN);
                }
                else
                {
                    stats.removeWPiece(PAWN);
                }

#ifdef USE_MMXASM
                mmx_toggleWhitePieceOnBitboards(to-8,PAWN);
#else
                toggleWhitePieceOnBitboards(to-8,PAWN);
#endif
                z1_hash  ^= p_TT->z1XorWhite(PAWN,to-8);
                z2_hash  ^= p_TT->z2XorWhite(PAWN,to-8);
                zp1_hash ^= p_pawnTT->zp1XorWhite(to-8);
                zp2_hash ^= p_pawnTT->zp2XorWhite(to-8);

                return;
            }

            // --- Umdwandlungsfeld erreicht? ---
            if (m.isPromotion())
            {
                if (restore)
                {
                    stats.addBPiece(PAWN);
                    stats.removeBPiece(m.getPromPiece());
                }
                else
                {
                    stats.removeBPiece(PAWN);
                    stats.addBPiece(m.getPromPiece());
                }

                blackPawns ^= BBTables::bbmask[to];
                
                switch (m.getPromPiece())
                {
                case QUEEN:
                    blackQueens  ^= BBTables::bbmask[to]; break;
                case ROOK:
                    blackRooks   ^= BBTables::bbmask[to]; break;
                case BISHOP:
                    blackBishops ^= BBTables::bbmask[to]; break;
                case KNIGHT:
                    blackKnights ^= BBTables::bbmask[to]; break;
                }
                return;
            }
            
            return;

        }

        // --- Kein Bauernzug - anderes Figuren-BB aktualisieren ---

        switch (piece)
        {
        case BISHOP:
            blackBishops ^= move_mask;
            break;
        case KNIGHT:
            blackKnights ^= move_mask;
            break;
        case ROOK:
            blackRooks ^= move_mask;
            break;
        case QUEEN:
            blackQueens ^= move_mask;
            break;
        case KING:
            blackKing ^= move_mask;

            // Rochade?
            if (m.isCastleMove())
            {
                int ro_from, ro_to;
                // Turm bewegen
                if (to == 6)
                {
                    ro_from = 7;
                    ro_to = 5;
                }
                else
                {
                    ro_from = 0;
                    ro_to = 3;
                }


                z1_hash ^= p_TT->z1XorBlack(ROOK,ro_to);
                z2_hash ^= p_TT->z2XorBlack(ROOK,ro_to);
                z1_hash ^= p_TT->z1XorBlack(ROOK,ro_from);
                z2_hash ^= p_TT->z2XorBlack(ROOK,ro_from);

                blackRooks    ^= BBTables::bbmask[ro_to] | BBTables::bbmask[ro_from];
                blackPieces   ^= BBTables::bbmask[ro_to] | BBTables::bbmask[ro_from];
                allPieces45C  ^= BBTables::bbmask[rot45C_bitIndex[ro_to]]  | 
                                   BBTables::bbmask[rot45C_bitIndex[ro_from]];
                allPieces45AC ^= BBTables::bbmask[rot45AC_bitIndex[ro_to]] | 
                                   BBTables::bbmask[rot45AC_bitIndex[ro_from]];
                allPieces90C  ^= BBTables::bbmask[rot90C_bitIndex[ro_to]]  | 
                                   BBTables::bbmask[rot90C_bitIndex[ro_from]];
            }
        }
    }
} // updateBitboards

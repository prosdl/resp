// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : recognizer.cpp
//                       Erkennen und Bewerten von Endspielpositionen
//
//  Anfang des Projekts: Fr. 5.3.2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.15
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: recognizer.cpp,v 1.10 2003/06/02 18:12:53 rosendahl Exp $
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

#include "recognizer.h"
#include "kedb.h"


// -------------------------------------------------------------------------
//                  c l a s s  R e c o g T a b l e 
// -------------------------------------------------------------------------

// Objekt
RecogTable* RecogTable::p_instance = NULL;

// -------------------------------------------------------------------------
//  Konstruktor
// -------------------------------------------------------------------------
RecogTable::RecogTable()
{
    build_table();
}

// -------------------------------------------------------------------------
//  Aufbauen der Recognizer Tabelle
// -------------------------------------------------------------------------
void RecogTable::build_table()
{
    // In diese Tabelle muessen die einzelnen Recognizer eingetragen werden
    Recognizer* recognizer[] = {
        new KK_Recognizer(),
        new KBK_Recognizer(),
        new KNK_Recognizer(),
        new KNKN_Recognizer(),
        new KPK_Recognizer(), 
        new KBPK_Recognizer(),
        new KBKP_Recognizer(),
        new KNKP_Recognizer(),
        0    // der letzte Eintrag muss immer Null sein                   
    };

    int idx = 0;
    Recognizer* p_rec;

    // Tabelle mit Nullen vorbesetzen
    memset(rec_table,0,sizeof(rec_table[0]) * 1024);

    // Recognizer installieren
    while ( (p_rec = recognizer[idx++]) )
    {
        rec_table[p_rec->w_mat() << 5 | p_rec->b_mat()] = 
        rec_table[p_rec->b_mat() << 5 | p_rec->w_mat()] = p_rec;
    }
}


// -------------------------------------------------------------------------
//                  c l a s s  R e c o g n i z e r
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//                      *** KK Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KK_Recognizer::execute(const Board* pBoard)
{
    // Diese Spiele sind immer Remis
    score  = 0;
    return EXACT;
}

// -------------------------------------------------------------------------
//                      *** KBK Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KBK_Recognizer::execute(const Board* pBoard)
{
    score  = 0;

    bool strong_is_white = pBoard->stats.nBlackPiecesTotal == 1;

    // Remis, falls nur ein Läufer -- den Fall zwei Läufer gleicher 
    // Farbe spare ich aus 
    if ( (strong_is_white && pBoard->stats.nWhitePieces[BISHOP] == 1) ||
         (!strong_is_white && pBoard->stats.nBlackPieces[BISHOP] == 1)   )
        return EXACT;

    return FAILE;
}

// -------------------------------------------------------------------------
//                      *** KNK Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KNK_Recognizer::execute(const Board* pBoard)
{
    score  = 0;

    bool strong_is_white = pBoard->stats.nBlackPiecesTotal == 1;

    // Remis, falls nur ein Springer
    if ( (strong_is_white && pBoard->stats.nWhitePieces[KNIGHT] == 1) ||
         (!strong_is_white && pBoard->stats.nBlackPieces[KNIGHT] == 1)   )
        return EXACT;

    // KNNxK ist normalerweise Remis
    if ( 
         (strong_is_white  && pBoard->stats.nWhitePieces[KNIGHT] == 2) || 
         (!strong_is_white && pBoard->stats.nBlackPieces[KNIGHT] == 2)
       )
    {
        // Ausnahmefall: König der schwachen Seite ist in der Ecke und
        // starke Seite am Zug
        if ( 
             (pBoard->sideToMove() == WHITE && strong_is_white ) ||
             (pBoard->sideToMove() == BLACK && !strong_is_white) 
           )
        {
            int k_pos = (strong_is_white) ? pBoard->blackKingPos :
                                            pBoard->whiteKingPos;

            if (k_pos == 0 || k_pos == 7 || k_pos == 56 || k_pos == 63)
                return FAILE; 
        }

        return EXACT;
    }

    return FAILE;
}

// -------------------------------------------------------------------------
//                      *** KNKN Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KNKN_Recognizer::execute(const Board* pBoard)
{
    // Nur der Fall jede Seite hat einen Springer wird unterstuetzt!
    if (pBoard->stats.nWhitePiecesTotal != 2 ||
        pBoard->stats.nBlackPiecesTotal != 2)
        return FAILE;

    score  = 0;
    // Normalerweise Remis; Matt in 1 moeglich, falls der Koenig der
    // Seite, die nicht am Zug ist in der Ecke ist
    int k_pos = (pBoard->sideToMove() == WHITE) ? pBoard->blackKingPos :
                                                  pBoard->whiteKingPos;

    if (k_pos == 0 || k_pos == 7 || k_pos == 56 || k_pos == 63)
        return FAILE; 

    return EXACT;
}


// -------------------------------------------------------------------------
//                      *** KPK Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KPK_Recognizer::execute(const Board* pBoard)
{
    // Es darf nur ein Bauer vorhanden sein!
    if (pBoard->stats.nWhitePiecesTotal + pBoard->stats.nBlackPiecesTotal != 3)
        return FAILE;


    // Nur exaktes Resultat fuer Remis  (gemaess KEDB) zurueckgeben; gewonnene 
    // Stellungen kann die Suche erledigen
    KEDB* pKEDB = KEDB::getHandle();

    if (pKEDB->query_kpk(pKEDB->kpk_index(pBoard)) == KEDB::REMIS)
    {
        score = 0;
        return EXACT;
    }

    // Ab hier ist die Stellung gewonnen: als Score gebe ich eine Art
    // heuristische Bewertung zurück, die umso besser ist, je näher der 
    // Bauer an der siebten (zweiten) Reihe ist:

    bool strong_is_white = pBoard->stats.nBlackPiecesTotal == 1;

    int p_pos = (strong_is_white) ? pBoard->whitePawns.lsb() :
                                    pBoard->blackPawns.msb();

    int dist = (strong_is_white) ? ROW(p_pos) : 8-ROW(p_pos);

    score = piece_val[QUEEN] - piece_val[PAWN] - dist*10;

    if ((pBoard->sideToMove() == WHITE && strong_is_white )   ||
        (pBoard->sideToMove() == BLACK && !strong_is_white))
    {
        return LBOUND;
    }
    else
    {
        score = -score;
        return UBOUND;
    } 


    return FAILE;
}

// -------------------------------------------------------------------------
//                      *** KBPK Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KBPK_Recognizer::execute(const Board* pBoard)
{
    // ---------------------------------------------------------------------------
    // "Wrong Rooks Pawn" Regel
    // Wenn der König der schwachen Seite auf dem Umwandlungsfeld des Bauern ist,
    // dann gilt:
    //  Remis <== [ Bauern == Turmbauern  ^  Läufer hat falsche Farbe ]
    // ---------------------------------------------------------------------------

    // Starke Seite feststellen
    bool strong_is_white = pBoard->stats.nBlackPiecesTotal == 1;

    if (strong_is_white)
    {
        // -------------------
        //      FÜR WEISS
        // -------------------
        // Nur A-Bauern oder nur H-Bauern
        if ( (pBoard->whitePawns & ~BBTables::filemask[0]) != 0   &&
             (pBoard->whitePawns & ~BBTables::filemask[7]) != 0)
             return FAILE;

        // Falscher Läufer?
        bool is_a_pawn = (pBoard->whitePawns&BBTables::filemask[0]) != 0;

        bool is_white_bishop = (pBoard->whiteBishops & BBTables::whiteSquares) != 0;

        if ( (is_a_pawn  &&  is_white_bishop) ||
             (!is_a_pawn && !is_white_bishop))
            return FAILE;

        // König der schwachen Seite auf Umwandlungsfeld?
        if ( (is_a_pawn  && pBoard->blackKingPos != 0) ||
             (!is_a_pawn && pBoard->blackKingPos != 7) )
            return FAILE;

        // Alles erfüllt: Remis
        score = 0;
        return EXACT;
    }
    else
    {
        // -------------------
        //      FÜR SCHWARZ
        // -------------------
        // Nur A-Bauern oder nur H-Bauern
        if ( (pBoard->blackPawns & ~BBTables::filemask[0]) != 0   &&
             (pBoard->blackPawns & ~BBTables::filemask[7]) != 0)
             return FAILE;

        // Falscher Läufer?
        bool is_a_pawn = (pBoard->blackPawns&BBTables::filemask[0]) != 0;

        bool is_white_bishop = (pBoard->blackBishops & BBTables::whiteSquares) != 0;

        if ( (is_a_pawn  && !is_white_bishop) ||
             (!is_a_pawn &&  is_white_bishop))
            return FAILE;

        // König der schwachen Seite auf Umwandlungsfeld?
        if ( (is_a_pawn  && pBoard->whiteKingPos != 56) ||
             (!is_a_pawn && pBoard->whiteKingPos != 63) )
            return FAILE;

        // Alles erfüllt: Remis
        score = 0;
        return EXACT;
    }

    return FAILE;
}

// -------------------------------------------------------------------------
//                      *** KBKP Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KBKP_Recognizer::execute(const Board* pBoard)
{
    score  = 0;

    bool strong_is_white = pBoard->stats.matWhite == 4;

    // Nur den Fall behandeln, dass ein Läufer vorhanden ist:
    if ( strong_is_white && pBoard->stats.nWhitePiecesTotal != 2   ||
        !strong_is_white && pBoard->stats.nBlackPiecesTotal != 2)
       return FAILE;

    // Bishop Seite kann nicht gewinnen
    if ( ( strong_is_white && pBoard->sideToMove() == WHITE) ||
         (!strong_is_white && pBoard->sideToMove() == BLACK)    )
    {
        return UBOUND;
    }
    else
    {
        return LBOUND;
    }

    return FAILE;
}

// -------------------------------------------------------------------------
//                      *** KNKP Recognizer ***
// -------------------------------------------------------------------------

RecogTable::RecogResult RecogTable::KNKP_Recognizer::execute(const Board* pBoard)
{
    score  = 0;

    bool strong_is_white = pBoard->stats.matWhite == 2;

    // Knight Seite kann nicht gewinnen; Ausnahme: König eingeklemmt
    // in der Ecke

    if (strong_is_white)
    {
        if (pBoard->blackKing & (BBTables::bbmaskA1B1A2B2|BBTables::bbmaskG1H1G2H2))
            return FAILE;
    }
    else
    {
        if (pBoard->whiteKing & (BBTables::bbmaskA8B8A7B7|BBTables::bbmaskG8H8G7H7))
            return FAILE;
    }

    if ( ( strong_is_white && pBoard->sideToMove() == WHITE) ||
         (!strong_is_white && pBoard->sideToMove() == BLACK)    )
    {
        return UBOUND;
    }
    else
    {
        return LBOUND;
    }

    return FAILE;
}


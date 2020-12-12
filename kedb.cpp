// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : kedb.h
//                       Knowledgeable Endgame Databases
//
//  Anfang des Projekts: Fr. 1.3.2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.15
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: kedb.cpp,v 1.7 2003/06/02 18:12:53 rosendahl Exp $
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

#include "kedb.h"
#include "basic_stuff.h"
#include <iostream>

using namespace std;

// -------------------------------------------------------------------------
//  Object
// -------------------------------------------------------------------------

KEDB* KEDB::p_instance = NULL;



// -------------------------------------------------------------------------
//                              I n d i z e s
// -------------------------------------------------------------------------

/** Indexberechnung für die aktuelle Position aus <code>pB</code>.

  Der Index wird folgendermassen berechnet: <p>
  OE gilt: Weiss ist am Zug und der Bauer befindet sich in der linken
           Bretthaelfte. Der Allgemeinfall wird durch Spiegelungen an
           der DE-Achse (zwischen Spalte D und E) und der 45-Achse 
           (zwischen Reihe 4 und 5)  hergestellt. <p>
  Dann setzt sich der Index folgendermassen zusammen:

<pre>
  idx(Kw,Pw,Kb) = wtm + Kw*2**1 + Kb*2**7 + lh_pos(Pw)*2**13
</pre>

  mit:
<pre>
    wtm    == "White to Move"       in {0, 1}
    Kw     == "Position King White" in {0,63}
    Kb     == "Position King Black" in {0,63}
    Pw     == "Position Pawn White" in {0,63}
    lh_pos == "Position in left half of the board" {0,63}->{0,31}
</pre>

 d.h.: idx ist in <code>{0, ..., 2**18-1==262143}</code>


 @param pB   Das aktuelle (globale) Schachbrett.
 */

unsigned int KEDB::kpk_index(const Board* pB) const
{
    // Zunaechst feststellen welches die starke Seite ist
    bool pawn_white = pB->stats.nBlackPiecesTotal == 1;

    // Ist Weiss am Zug?
    int wtm = (pB->sideToMove() == WHITE) ? 1 : 0;

    // Bauernposition feststellen
    int pawn_pos = (pawn_white) ? pB->whitePawns.lsb() : pB->blackPawns.msb();


    // Welcher der 4 moeglichen Faelle liegt vor (s.o.)?
    if (pawn_white)
    {
        if ((pawn_pos & 0x4) == 0) // Bauer aus linker Haelfte?
        {
            return wtm | (pB->whiteKingPos << 1) | (pB->blackKingPos << 7) |
                         (lh_pos(pawn_pos) << 13);
        }
        else
        {
            int kw = mirror_de(pB->whiteKingPos);
            int kb = mirror_de(pB->blackKingPos);
            int pw = mirror_de(pawn_pos);

            return wtm | (kw << 1) | (kb << 7) | (lh_pos(pw) << 13);
        }
    }
    else // schwarzer Bauer
    {
        wtm ^= 1;
        if ((pawn_pos & 0x4) == 0) // Bauer aus linker Haelfte?
        {
            int kw = mirror_45(pB->blackKingPos);
            int kb = mirror_45(pB->whiteKingPos);
            int pw = mirror_45(pawn_pos);

            return wtm | (kw << 1) | (kb << 7) | (lh_pos(pw) << 13);
        }
        else
        {
            int kw = rotate_180(pB->blackKingPos);
            int kb = rotate_180(pB->whiteKingPos);
            int pw = rotate_180(pawn_pos);

            return wtm | (kw << 1) | (kb << 7) | (lh_pos(pw) << 13);
        }
    }
}

/**  
 * Zugriff auf die Datenbanken.
 */

int KEDB::query_kpk(unsigned int idx) const
{
    int kpk_pos = idx >> 5;
    int kpk_bit = idx & 31;

    if (kpk[kpk_pos] & 1<<kpk_bit)
        return STRONG_WINS;
    else
        return REMIS;
}


/**
 *  Erstellen der Tablebases ... KPK
 */

void KEDB::create_kpk_db()
{
    Board* pB = Board::getHandle();

    // temporaere Tabelle um DB abzuspeichern (spaeter erst der Bit-Vektor)
    // Index:   Seite x Pos. Koenig Weiss x Pos. Koenig Schwarz x 
    //          Position Bauer Weiss auf linker Bretthaelfte
    unsigned char kpk_table[2][64][64][32];

    int s,kw,kb,lh_pw;

    int legal_pos  = 0;
    int won_pos    = 0;

    // Vorbesetzen:
    for (s = 0; s <= 1; s++)
        for (kw = 0; kw < 64; kw++)
            for (kb = 0; kb < 64; kb++)
                for (lh_pw = 0; lh_pw < 32; lh_pw++)
                    kpk_table[s][kw][kb][lh_pw] = UNKNOWN;

    // -------------------------------------------------------------------------
    // PHASE I ... Illegale Positionen finden + Gewonnene Stellungen durch 
    //             Promotion   
    // -------------------------------------------------------------------------
    // Fuer alle Positionen:
    //   - illegale Positionen markieren
    //   - bei allen Zuegen mit  wtm + Pw auf Reihe 7 + Promotion legal:
    //     falls dist(Kb, Prom.feld) > 1 || dist(Kw, Prom.feld) == 1
    //          ==> markiere Position als gewonnen fuer Weiss

    for (s = 0; s <= 1; s++)
        for (kw = 0; kw < 64; kw++)
            for (kb = 0; kb < 64; kb++)
                for (lh_pw = 4; lh_pw < 28; lh_pw++)
                {
                    // -----------------
                    //  Position legal?
                    // -----------------

                    int pw = (lh_pw/4)*8 + lh_pw%4;

                    // Abstand der Koenige muss mindestens 2 sein
                    if (dist(kw,kb) < 2)
                    {
                        kpk_table[s][kw][kb][lh_pw] = ILLEGAL;
                        continue;
                    }

                    // Kollisionen?
                    if (kw == kb || kw == pw || kb == pw)
                    {
                        kpk_table[s][kw][kb][lh_pw] = ILLEGAL;
                        continue;
                    }

                    // Falls Weiss am Zug, kann schwarzer Koenig nicht 
                    // im Schach sein
                    if ( s && ( (COL(pw) != 7 && kb == pw-7) || 
                                (COL(pw) != 0 && kb == pw -9) ) )
                    {
                        kpk_table[s][kw][kb][lh_pw] = ILLEGAL;
                        continue;
                    }

                    legal_pos++;

                    // ----------------------
                    //  Position gewonnen?
                    // ----------------------
                    if (s && ROW(pw) == 1 && kw != pw - 8 && kb != pw - 8)
                    {
                        if (dist(kb,pw-8) > 1 || dist(kw,pw-8) == 1)
                        {
                            kpk_table[s][kw][kb][lh_pw] = STRONG_WINS;
                            won_pos++;
                        }
                    }
                }

    cout << "Legal Positions: " << legal_pos << endl;
    cout << "Won Positions after Phase 0 = " << won_pos << endl;


    // -----------------------------------------------------------------------------
    // Naechste Phase(n) -- abwechselnd mit Schwarz und Weiss folgendes pruefen:
    //      fuer Schwarz: fuehren alle moeglichen Zuege in eine verlorene Position?
    //      fuer Weiss  : gibt es einen Zug der in eine gewonnene Position fuehrt?
    //  Solange fortfahren bis keine gewonnene Position mehr gefunden wurde.
    // -----------------------------------------------------------------------------

    
    s = 0; // Schwarz faengt an
    int phase = 1;
    while (true)
    {
        cout << "Phase = " << phase;
        if (s)
            cout << " ... White To Move" << endl;
        else
            cout << " ... Black To Move" << endl;

        int new_won_pos = 0;
        for (kw = 0; kw < 64; kw++)
            for (kb = 0; kb < 64; kb++)
                for (lh_pw = 4; lh_pw < 28; lh_pw++)
                {
                    // Es interessieren nur Positionen, deren Spielwert unbekannt ist
                    if (kpk_table[s][kw][kb][lh_pw] != UNKNOWN)
                        continue;

                    int pw = (lh_pw/4)*8 + lh_pw%4;
                    // ---------------------
                    // Position aufsetzen
                    // ---------------------
                    ByteBoard byb;
                    byb.set(kw,WHITE,KING);
                    byb.set(kb,BLACK,KING);
                    byb.set(pw,WHITE,PAWN);
                    load_position(pB, (s) ? WHITE : BLACK, byb);


                    // ----------------
                    // Zuege iterieren
                    // ----------------
                    pB->gen(0);
                    MoveStack& mvst = pB->moveStack[0];
                    // Falls keine Zuege gefunden werden, muss es ein Patt sein
                    // -- Matt bei KPK nicht moeglich
                    int status = REMIS;
                    for (int i=0; i < mvst.size(); i++)
                    {
                        Move m = mvst.stack[i];
                        pB->makemove(m);

                        if (!pB->positionLegal())
                        {
                            pB->takebackmove();
                            continue;
                        }


                        if (pB->whitePawns)
                        {
                            int new_kw = pB->whiteKingPos;
                            int new_kb = pB->blackKingPos;
                            int new_pw = pB->whitePawns.lsb();
                            int new_s  = (s+1) % 2;

                            status = kpk_table[new_s][new_kw][new_kb][lh_pos(new_pw)];
                        }
                        else
                        {
                            //  Bauer nicht mehr da -- geschlagen oder Promotion:
                            //  solche Positionen interessieren fuer KPK nicht
                            status = UNKNOWN;
                        }

                        pB->takebackmove();
                        
                        if (!s) 
                        {
                            // -----------------
                            // Schwarz am Zug
                            // -----------------
                            if (status != STRONG_WINS)
                                break;
                        }
                        else 
                        {
                            // -----------------
                            // Weiss am Zug
                            // -----------------

                            if (status == STRONG_WINS)
                                break;
                        }

                    }

                    // Falls fuer Schwarz keine Zuege gefunden wurden, die nicht zur 
                    // Niederlage fuehren, oder fuer Weiss ein Zug gefunden wurde,
                    // der zum Sieg fuehrt, ist der Status als "Gewonnen" festzulegen
                    if (status == STRONG_WINS)
                    {
                        kpk_table[s][kw][kb][lh_pw] = STRONG_WINS;
                        new_won_pos++;
                    }

                }

        // Keine neuen Gewinnpositionen?
        if (new_won_pos == 0)
            break;

        won_pos += new_won_pos;

        cout << "Won Positions = " << won_pos << endl;

        // Naechste Seite
        s = (s+1) % 2;
        phase++;
    }


    int draw_wtm  = 0;
    int draw_btm  = 0;
    int legal_wtm = 0;
    int legal_btm = 0;

    // ---------------------------------------------------------------------
    // Naechste Phase: kpk_table nach Bitvektor schreiben
    // ---------------------------------------------------------------------
    memset(kpk,0,sizeof(kpk));

    for (s = 0; s <= 1; s++)
        for (kw = 0; kw < 64; kw++)
            for (kb = 0; kb < 64; kb++)
                for (lh_pw = 4; lh_pw < 28; lh_pw++)
                {
                    int status = kpk_table[s][kw][kb][lh_pw];


                    // Statistik
                    if (s)
                    {
                        if (status == UNKNOWN)
                            draw_wtm++;
                        if (status != ILLEGAL)
                            legal_wtm++;
                    }
                    else
                    {
                        if (status == UNKNOWN)
                            draw_btm++;
                        if (status != ILLEGAL)
                            legal_btm++;
                    }

                    if (status == STRONG_WINS)
                    {
                        int kpk_index = s | (kw << 1) | (kb << 7) | (lh_pw << 13);
                        int kpk_pos = kpk_index >> 5;
                        int kpk_bit = kpk_index & 31;

                        kpk[kpk_pos] |= 1<<kpk_bit;
                    }

                        
                }

    cout << "WTM -- Legal Positions = " << legal_wtm*2 << endl;
    cout << "WTM -- Draws           = " << draw_wtm*2 << endl;
    cout << "BTM -- Legal Positions = " << legal_btm*2 << endl;
    cout << "BTM -- Draws           = " << draw_btm*2 << endl;


    // ---------------------------------------
    // Letzter Schritt Bitvektor speichern
    // ---------------------------------------

    write_db("kedb.kpk",kpk,sizeof(kpk)/sizeof(kpk[0]));

}


/**
 *  Position aufs Brett uebertragen.
 */
void KEDB::load_position(Board *pB, ColorEnum new_side, ByteBoard& byb)
{
    pB->castle      = 0;
    pB->ep          = 0;
    pB->fifty       = 0;
    pB->totalHPly   = 1; 
    pB->age         = 0;
/*    pB->z1_hash  = pB->getTT()->z1_value(byb.piece,byb.color);
    pB->z2_hash  = pB->getTT()->z2_value(byb.piece,byb.color);
    pB->zp1_hash = pB->getPTT()->zp1_value(byb.piece,byb.color);
    pB->zp2_hash = pB->getPTT()->zp2_value(byb.piece,byb.color); */
    pB->initStatsFromBitboards();



    pB->side        = new_side;
    pB->initBitBoardsFromByteBoard(byb);
}


/**
 *  Bitvektor speichern
 */
bool KEDB::write_db(string filename, unsigned int bit_vector[], int n_entries)
{
    cout << "Writing data ... ";
    ofstream fout(filename.c_str(), ios::out | ios::binary);

    if (! fout.is_open())
    {
        cout << endl;
        cout << "ERROR in kedb.cpp" << endl;
        cout << "Couldn't open " << filename << endl;
        return false;
    }


    for (int i=0; i < n_entries; i++)
    {
        fout.write(reinterpret_cast<char*> (bit_vector + i), sizeof(bit_vector[0]));
    }
        
    fout.close();

    cout << "OK" << endl;
    cout << endl;

    return true;
}

/**
 *  Bitvektor laden
 */
bool KEDB::read_db(string filename, unsigned int bit_vector[])
{
    ifstream fin;

    cout << "Loading " << filename << " ... ";

    fin.open(filename.c_str(),ios::binary | ios::in);

    if (!fin.is_open())
    {
        cout << endl;
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        cout << "ERROR in kedb.cpp" << endl;
        cout << "Couldn't open " << filename << endl;
        cout << "Resp can't play with full strength now ... " << endl;
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        exit(1);
        return false;
    }

    fin.seekg(0,ios::end);
    int n = fin.tellg() / sizeof(bit_vector[0]);

    for (int i=0; i < n; i++)
    {
        fin.seekg(i*sizeof(bit_vector[0]));
        fin.read(reinterpret_cast<char*> (bit_vector + i), sizeof(bit_vector[0]));
    }

    cout << "OK" << endl;
    cout << endl;

    return true;
}

/**
 *  Laden der Endspieldatenbanken.
 */ 
bool KEDB::load_db()
{
    kpk_ready = read_db("kedb.kpk",kpk);

    return true;
}


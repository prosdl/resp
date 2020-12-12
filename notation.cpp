// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : notation.cpp
//                       SAN, FEN, EPD Import/Export
//
//  Anfang des Projekts: Mi, 1.Jan, 2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: notation.cpp,v 1.6 2002/06/11 19:04:44 rosendahl Exp $
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




#include "notation.h"
#include "StringTools.h"


using namespace std;

// -------------------------------------------------------------------------
//      loadFenString
//      -------------
//  Lädt eine Position im FEN-Format und aktualisiert das Brett entsprechend.
// -------------------------------------------------------------------------

bool Notation::load_fen(string& fen, ByteBoard& byb, int& new_ep, int& new_fifty, 
                        int& new_castle, int& new_totalHPly, ColorEnum& new_side)
{
    char buf[256];
    char *f = buf;
    char c;
    int j;
    int pos = 0;

    // Leerzeichen entfernen
    fen = StringTools::trim(fen);

    int len = fen.length();
    if (len >255) len = 255;
    fen.copy(f,len);
    f[len] = '\0';



    // ------------ Brettdaten einlesen ------------------------------------
    j = 0;
    
    while (true)
    {

        if (! (c = *f++))
        {
            return false;
        }

        // Leerfelder?
        if (isdigit(c))   
        {
            int n_free = c - '0';

            if ((n_free <= 0) || (n_free > 8 ))
                return false;

            while (true)
            {
                if (pos >=64)           // Überlauf?
                    return false;

                if (j >= 8) return false;  // Zeilenende? 

                byb.set(pos,EMPTY,NO_PIECE);

                n_free--;
                if (!n_free) break;

                pos++; j++;
            }
        }
        else
        {
            // Figur lesen
            switch (c)
            {
            case 'P':
                byb.set(pos,WHITE,PAWN);
                break;
            case 'p':
                byb.set(pos,BLACK,PAWN);
                break;
            case 'N':
                byb.set(pos,WHITE,KNIGHT);
                break;
            case 'n':
                byb.set(pos,BLACK,KNIGHT);
                break;
            case 'B':
                byb.set(pos,WHITE,BISHOP);
                break;
            case 'b':
                byb.set(pos,BLACK,BISHOP);
                break;
            case 'R':
                byb.set(pos,WHITE,ROOK);
                break;
            case 'r':
                byb.set(pos,BLACK,ROOK);
                break;
            case 'Q':
                byb.set(pos,WHITE,QUEEN);
                break;
            case 'q':
                byb.set(pos,BLACK,QUEEN);
                break;
            case 'K':
                byb.set(pos,WHITE,KING);
                break;
            case 'k':
                byb.set(pos,BLACK,KING);
                break;
            default:
                // Ubekanntes Zeichen:
                return false;
            }
        }


        pos++; j++;

        if (j > 8 ) return false; // Zeilenende überschritten

        if (j == 8) // Zeilenende erreicht
        {
            if (pos == 64) 
                break;          // Fertig mit Brett-Setup

            j = 0;


            if (*f++ != '/')    // Trenner überlesen
                return false;

        }


        if (pos >= 64)          // Brettende überschritten
            return false;

    } // while


    // Trenzeichen Space
    if (*f++ != ' ')
        return false;

    // -------------- Wer ist am Zug? ---------------------------------
    c = *f++;
    if (c == 'w')
        new_side = WHITE;
    else if (c == 'b')
        new_side = BLACK;
    else
        return false;

    // Trenzeichen Space
    if (*f++ != ' ')
        return false;

    // ----------------- Rochade Rechte -------------------------------
    new_castle = 0;
    c = *f++;

    if (c != '-')
    {
        bool found_space = false;
        while (true)
        {
            switch (c)
            {
            case 'K':
                if (new_castle & CASTLE_WHITE_KINGSIDE)
                    return false;
                new_castle |= CASTLE_WHITE_KINGSIDE;
                break;
            case 'k':
                if (new_castle & CASTLE_BLACK_KINGSIDE)
                    return false;
                new_castle |= CASTLE_BLACK_KINGSIDE;
                break;
            case 'Q':
                if (new_castle & CASTLE_WHITE_QUEENSIDE)
                    return false;
                new_castle |= CASTLE_WHITE_QUEENSIDE;
                break;
            case 'q':
                if (new_castle & CASTLE_BLACK_QUEENSIDE)
                    return false;
                new_castle |= CASTLE_BLACK_QUEENSIDE;
                break;
            case ' ':
                f--;    // "unget"
                found_space = true;
                break;  // Ende erreicht

            default:
                return false; // Unerwartetes Zeichen
            }

            if (found_space)
                break;

            c = *f++;
        }
    }

     // Trenzeichen Space
    if (*f++ != ' ')
        return false;


    // ------------ En passant Ziel ----------------------------------------
    c = *f++;
    new_ep = 0;
    if (c != '-') // ep Ziel vorhanden?
    {

        if ( (c >= 'a') && (c <= 'h') )
            new_ep += c - 'a';
        else
            return false;

        c = *f++;
        if ( (c >= '1') && (c <= '8') )
            new_ep += 8* (7 - (c - '1'));
        else
            return false;

    }


    // Folgende Abweichung von FEN Standard zulassen:
    // Falls Fifty und Total Half Ply nicht vorhanden, durch
    // 0, 0 ersetzen

    if  (*f == '\0')
    {
        new_fifty       = 0;
        new_totalHPly   = 0;
    }
    else
    {

         // Trenzeichen Space
        if (*f++ != ' ')
            return false;

        // ------------ fifty lesen --------------------------------------------
        string sfifty = "";

        while (isdigit(c = *f++))
        {
            sfifty += c;
        }

        f--; // "unget(blank)"

        if (sfifty.length() <= 0)
            return false;

        new_fifty = atoi(sfifty.c_str());

        if (*f++ != ' ')
            return false;

        // ----------- Anzahl Züge lesen ----------------------------------------
        string sply = "";

        while (isdigit(c = *f++))
        {
            sply += c;
        }
        f--; // "unget(blank)"

        if (sply.length() <= 0)
            return false;

        // die gelesene Zuganzahl muss um 1 korrigiert werden,
        // damit sie mit resp's internen Zaehler uebereinstimmt
        new_totalHPly = atoi(sply.c_str()) - 1;

        if (new_totalHPly < 0)
            new_totalHPly = 0;
    }


    return true;
}


// -------------------------------------------------------------------------
// Zug in SAN-Notation umwandeln; zu Beachten ist, dass die aktuelle
// Brettposition verwendet wird, um evtl. mehrdeutige Notationen von
// Zuegen zu vermeiden: Beispiel Springer auf g1 und c1, dann wird aus
// c1 -> e2  der SAN-Zug Nce2 
// -------------------------------------------------------------------------
std::string Notation::move_to_san(const Move& m, Board* pBoard) 
{
    // Spezialfall Nullmove
    if (m.isNullmove())
        return "null";

    string san;
    bool castle = false;

    // Sonderfall Rochade
    // kurz:
    if ( m.getPiece() == KING && 
         ( (m.from() == 60 && m.to() == 62) || (m.from()==4 && m.to()==6) ) )
    {
        san = "O-O";
        castle = true;
    }
    // lang:
    if ( m.getPiece() == KING && 
         ( (m.from() == 60 && m.to() == 58) || (m.from()==4 && m.to()==2) ) )
    {
        san = "O-O-O";
        castle = true;
    }

    if (!castle)
    {
        // Piece letter feststellen

        switch (m.getPiece())
        {
        case KING:
            san = "K"; break;
        case QUEEN:
            san = "Q"; break;
        case ROOK:
            san = "R"; break;
        case KNIGHT:
            san = "N"; break;
        case BISHOP:
            san = "B"; break;
        case PAWN:
            break;
        default:
            san = "?"; break;
        }

        // Zielfeld bestimmen
        string s_to;
        if (m.isCapture() || m.isEpCapture())
            s_to = "x";
        s_to += (char) (COL(m.to()) + 'a');
        s_to += (char) (8-ROW(m.to()) + '0');

        // Sonderfall: schlagender Bauer

        if (m.isPawnMove() &&  (m.isCapture() || m.isEpCapture()) )
        {
            san = (char) (COL(m.from()) + 'a') + s_to;
        }

        else
        {
            // Eindeutigkeit
            vector<Move> legalMoves = pBoard->moveStack[0].findMoves(m.from(),m.to(),
                                        m.getPromPiece(),false,false);

            vector<Move>::iterator  it = legalMoves.begin();

            for (; it != legalMoves.end(); )
            {
                // Gleiche Figur?
                if ((*it).getPiece() != m.getPiece())
                {
                    it = legalMoves.erase(it);
                    continue;
                }
                // Zug legal?
                pBoard->makemove(*it);
                if (!pBoard->positionLegal() )
                    it = legalMoves.erase(it);
                else
                    it++;
                pBoard->takebackmove();
            }

            // Zugliste leer => m ist kein erlaubter Zug ?!
            if (legalMoves.size() == 0)
                return "ILLEGAL";

            // Zug nicht eindeutig?
            if (legalMoves.size() > 1)
            {
                it = legalMoves.begin();

                bool equal_col = false;
                bool equal_row = false;
                for (; it != legalMoves.end(); it++)
                {
                    if ( *it == m)
                        continue;
                    if ( COL( (*it).from()) == COL(m.from()) )
                        equal_col = true;
                    if ( ROW( (*it).from()) == ROW(m.from()) )
                        equal_row = true;
                }


                if (!equal_col)
                {
                    // Durch Spalte zu unterscheiden
                    san += (char) (COL(m.from()) + 'a');
                }
                else if (!equal_row)
                {
                    // Durch Reihe zu unterscheiden
                    san += (char) (8-ROW(m.from()) + '0');
                }
                else
                {
                    // Spalte und Reihe benötigt
                    san += (char) (COL(m.from()) + 'a');
                    san += (char) (8-ROW(m.from()) + '0');
                }
            }



            // Zusammensetzen
            san += s_to;


        }

    }

    // Sonderfall: Bauernumwandlungen
    if (m.isPromotion())
    {
        san += "=";

        switch (m.getPromPiece())
        {
        case BISHOP:
            san += "B"; break;
        case KNIGHT:
            san += "N"; break;
        case QUEEN:
            san += "Q"; break;
        case ROOK:
            san += "R"; break;
        default:
            san += "?"; break;
        }
    }


    // Zug soweit vollständig; jetzt fehlt noch Schach (+) bzw. Matt (#)

    bool check = false;
    bool mate = false;
    pBoard->makemove(m);

    // Sicherheitsprüfung: illegaler Zug?
    if (!pBoard->positionLegal())
    {
        pBoard->takebackmove();
        san = "ILLEGAL (CHECKED)";
        return san;
    }

    if (pBoard->inCheck())
    {
        check = true;

        // Sogar Matt?
        pBoard->gen(1);

        mate = true;
        for (int i=0; i < pBoard->moveStack[1].size(); i++)
        {
            Move mm = pBoard->moveStack[1].stack[i];

            pBoard->makemove(mm);
            if (pBoard->positionLegal() )
            {
                // legalen Zug für Gegner gefunden: kein Matt
                pBoard->takebackmove();
                mate = false;
                break;
            }
            pBoard->takebackmove(); 
        } 

    }
    pBoard->takebackmove();


    if (mate)
        san += "#";
    else if (check)
        san += "+"; 


    return san;
}


// -------------------------------------------------------------------------
//      san_to_move
//      -----------
//  Wandelt einen als Zeichenkette übergebenen SAN-Zug bei der gegebenen
//  aktuellen Brettposition in einen Zug vom Typ Move um. Falls err != 0
//  konnte der SAN-Zug nicht erfolgreich geparsed werden.
// -------------------------------------------------------------------------
Move Notation::san_to_move(std::string san, SAN_ParseError& err, Board* pBoard)
{
    Move m(0);
    int from;
    int to;
    int prom = 0;
    int  len        = san.length();
    bool check      = false;
    bool mate       = false;
    bool capture    = false;
    bool use_from_rank;
    bool use_from_file;
    PieceEnum pc;
    char row,col;
    


    // Mindestlänge == 2
    if (len <= 1)
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Transformation I: Anmerkungen entfernen; "!", "?", "!!", "?!", "!?",
    //                   "??"

    if ( (san[len - 1] == '?') || (san[len - 1] == '!') )
    {
        san.erase(san.end() - 1);
        len--;
    }
    if ( (san[len - 1] == '?') || (san[len - 1] == '!') )
    {
        san.erase(san.end() - 1);
        len--;
    }

    // Mindestlänge == 2
    if (len <= 1)
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Transformation II: Rochaden in "normale Züge" verwandeln
    if (san[0] == 'O')
    {
        if (san.find("O-O-O") == 0)
        {
            san.erase(0,5);
            if (pBoard->sideToMove() == WHITE)
                san = "Kc1" + san;
            else
                san = "Kc8" + san;
            len -= 2;
        }
        else if (san.find("O-O") == 0)
        {
            san.erase(0,3);

            if (pBoard->sideToMove() == WHITE)
                san = "Kg1" + san;
            else
                san = "Kg8" + san;
        }
        else
        {
            // Fehler
            err = SAN_SYNTAX_ERROR;
            return m;
        }
    }

    // Transformation III: Sicherstellen, daß führendes Figurzeichen [PNBRQK]
    //                    ex.
    
    // Bauern-Zug?
    if (SAN_IS_FILE_LETTER(san[0]))
    {
        san = "P" + san;
        len++;
    }

    // Flagge für Schach / Matt
    if (san[len-1] == '+')
    {
        check = true;
        san.erase(san.end() - 1);
        len--;
    }
    else if (san[len-1] == '#')
    {
        mate = true;
        san.erase(san.end() - 1);
        len--;
    }


    // Ab hier muss san mindestens 3 Zeichen haben, z.B. Pe4
    if (len <= 2)
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Umwandlung? Bsp.: Pe8=R
    if ( SAN_IS_PROMPIECE_LETTER(san[len-1]) )
    {
        switch (san[len-1])
        {
        case 'N':
            prom = KNIGHT; break;
        case 'B':
            prom = BISHOP; break;
        case 'R':
            prom = ROOK; break;
        case 'Q':
            prom = QUEEN; break;
        default:
            err = SAN_SYNTAX_ERROR;
            return m;
        }

        san.erase(san.end()-1);
        len--;

        // = abschneiden
        if (san[len-1] != '=')
        {
            err = SAN_SYNTAX_ERROR;
            return m;
        }

        san.erase(san.end()-1);
        len--;
    }

    // Jetzt hat san die Form [PNBRQK].*[a-h][1-8]
    if (len <= 2)
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Zielfeld abschneiden
    row = san[len-1];
    col = san[len-2];
    san.erase(len-2);
    len -= 2;

    // legale Zeichen? -> Zielfeld speichern
    if (SAN_IS_FILE_LETTER(col) && SAN_IS_RANK_DIGIT(row))
    {
        to = (col - 'a') + 8*(7 - (row - '1'));
        m.setTo(to);
    }
    else
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Jetzt haben wir: san = [PNBRQK][a-h]?[x]?        | 
    //                        [PNBRQK][1-8][x]?         |
    //                        [PNBRQK][a-h][1-8][x]?


    // Figur abschneiden
    pc = PAWN;
    if (SAN_IS_PIECE(san[0]))
    {
        switch (san[0])
        {
        case 'P':
            pc = PAWN; break;
        case 'N':
            pc = KNIGHT; break;
        case 'B':
            pc = BISHOP; break;
        case 'R':
            pc = ROOK; break;
        case 'Q':
            pc = QUEEN; break;
        case 'K':
            pc = KING; break;
        default:
       // Huh ?!
       pc = PAWN;
        }
        san.erase(san.begin());
        len--;
    }
    else
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    // Default: keine zusätzlichen Infos fürs Quellfeld
    from = 0;
    m.setFrom(from);
    use_from_file = use_from_rank = false;

    // Gibt es Zusatz Info fürs Quellfeld

    // Spalte?
    if ((len != 0) && SAN_IS_FILE_LETTER(san[0]))
    {
        use_from_file = true;
        from |= (san[0] - 'a');
        m.setFrom(from);
        san.erase(san.begin());
        len--;
    }

    // Reihe?
    if ((len != 0) && SAN_IS_RANK_DIGIT(san[0]))
    {
        use_from_rank = true;
        from |=  8*(7 - (san[0] - '1'));
        m.setFrom(from);
        san.erase(san.begin());
        len--;
    }


    // Nun kann maximal noch "x" übrig sein
    if (len >= 2)
    {
        err = SAN_SYNTAX_ERROR;
        return m;
    }

    if (len == 1)
    {
        if (san[0] == 'x')
            capture = true;
        else
        {
            err = SAN_SYNTAX_ERROR;
            return m;
        }

    }
    else
        capture = false;


    // Parsing Phase beendet!
    //
    // Jetzt versuchen den Zug auszuführen:

    // Alle Züge finden, die 'to' als Ziel haben.
    vector<Move> sourceFields = pBoard->moveStack[0].findMoves(from,to, prom, 
                                                use_from_file, use_from_rank);

    // Züge entfernen bei denen die Figur aus piece[] nicht zur Figur pc
    // passt:
    vector<Move>::iterator it = sourceFields.begin();
    for (; it < sourceFields.end(); )
        if ((*it).getPiece() != pc)
            it = sourceFields.erase(it);
        else
            it++;

    if (sourceFields.size() == 0) // nichts gefunden?
    {
        err = SAN_MOVE_NOT_POSSIBLE;
        return m;
    }


    // Aus den pseudolegalen Zügen, die legalen herausfiltern
    it = sourceFields.begin();
    for (;it < sourceFields.end(); )
    {
        pBoard->makemove(*it);
        if ( !pBoard->positionLegal() )
            it = sourceFields.erase(it);
        else
        {
            it++;
        }
        pBoard->takebackmove();
    }

    if (sourceFields.size() > 1) // nicht eindeutig?
    {
        err = SAN_AMBIGUOUS_MOVE;
        return m;
    }

    if (sourceFields.size() == 0) // nichts gefunden?
    {
        err = SAN_MOVE_NOT_POSSIBLE;
        return m;
    }

    
    // ------- ZUG GEFUNDEN! --------------
    m = sourceFields[0];
    err = SAN_NO_ERROR;
    return m;
}

// -------------------------------------------------------------------------
//  load_epdline ... lädt eine Zeile eines EPD-Files. Unterstützte Opcodes
//  sind 'am' (avoid move), 'bm' (best move), 'id' (Bezeichner).
//  Die in der Zeile berschriebene Brettposition wird geladen, und als
//  Rückgabewerte werden folgende Parameter zurückgegeben:
//      - id_string: falls 'id' vorhanden, sonst ""
//      - op_code  : AM, falls 'am' oder BM, falls 'bm' gefunden wurde; 
//                   sonst NOT_FOUND
//      - move_list: Operand von 'am' bzw. 'bm'
// -------------------------------------------------------------------------

bool Notation::load_epdline(string line, string& id, OpCodeEnum& op_code,
                         vector<Move>& move_list, Board*  pBoard)
{
    // ------------------------------------------------------
    //  Splitten von line in FEN-Anteil und Opcode-Teil
    // ------------------------------------------------------
    int pos = 0;
    // 4.Leerzeichen suchen
    for (int i = 0; i < 4; i++)
    {
        pos = line.find(' ',pos+1);
        if (pos == -1)
            return false;
    }
    string fen(line,0,pos);
    string op_codes(line,pos+1,line.length());

    // ----------------------------------------
    //  FEN-Anteil ergänzen und Position laden
    // ----------------------------------------
    fen += " 0 1";  // fifty = 0, totalHPly = 1

    if (!pBoard->load_fen(fen))
        return false;

    pBoard->gen(0);

    // -----------------------
    //  Opcode-Anteil scannen
    // -----------------------

    id = "";
    op_code = NOT_FOUND;
    move_list.clear();
    string s_move_list;

    while (true)
    {
        // Opcode heraustrennen; Format:  opcode -> op LEERZEICHEN arg SEMICOLON;

        op_codes =  StringTools::trim(op_codes);
        int op_end = op_codes.find(' ',0);
        if (op_end == -1)
            break;
        int arg_end = op_codes.find(';',0);
        if (arg_end == -1)
            break;

        string op(op_codes,0,op_end);
        string arg(op_codes,op_end + 1,arg_end - op_end - 1);

        // Behandelte Opcodes: am, bm, id

        if (op == "id")
        {
            id = arg;
        }
        else if (op == "am")
        {
            // Mehrfache am bzw. bm nicht zulassen
            if (op_code != NOT_FOUND)
                return false;

            op_code = AM;
            s_move_list = arg;

        }
        else if (op == "bm")
        {
            // Mehrfache am bzw. bm nicht zulassen
            if (op_code != NOT_FOUND)
                return false;

            op_code = BM;
            s_move_list = arg;
        }


        // Opcode aus String entfernen

        op_codes.erase(0,arg_end+2);

    }

    // Falls s_move_list nicht leer ist, die Zugliste erstellen

    while (s_move_list != "")
    {
        int san_end = s_move_list.find(' ',0);
        if (san_end == -1)
            san_end = s_move_list.length();

        string san(s_move_list, 0, san_end);
        s_move_list.erase(0,san_end + 1);

        SAN_ParseError err;
        Move m = san_to_move(san, err, pBoard);

        if (err != SAN_NO_ERROR)        // Fehler beim SAN Parsen
            return false;

        move_list.push_back(m);
    }


    return true;
}


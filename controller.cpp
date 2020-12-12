// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : controller.cpp
//                       Controller managed den kompletten Spielablauf; der
//                       Client (egal, ob Kommandozeile oder GUI) muß dem
//                       Controller-Objekt Befehle für die Zugausführung,
//                       Spielintialisierung, Spieleinstellungen, ... über-
//                       geben. Der aktuelle Status des Spiels (wer ist am
//                       Zug? wer hat gewonnen?) kann vom Client ausgelesen
//                       werden.
//                       
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: controller.cpp,v 1.93 2003/06/02 18:12:53 rosendahl Exp $
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
#include "controller.h"
#include "StringTools.h"
#include "eval.h"
#include "book.h"
#include "version.h"
#include "respmath.h"
#include "notation.h"
#include "move.h"
#include "kedb.h"
#include "gameinfo.h"
#include "respoptions.h"
#include "hash.h"
#include "phash.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <sstream>

using namespace std;


/////////////////////////////////////////////////////
// Definitionen für Befehlsklassen    ///////////////

///////////////////////
// CmdQuit
const char* Controller::CmdQuit::describtion()
{
    return "Quit RESP.";
}

void Controller::CmdQuit::execute(string &arg) throw (BadCmdException)
{
    // Status setzen
    ctrl->status = ST_GAME_OVER | ST_CANCELED;

    ctrl->gameInfo.appendToPGNFile("resp.pgn");
    ctrl->gameInfo.init();
}


///////////////////////
// CmdNewGame
const char* Controller::CmdNewGame::describtion()
{
    return "Start a new game";
}

void Controller::CmdNewGame::execute(string &arg) throw (BadCmdException)
{
    ctrl->status = ST_WHITE_TO_MOVE | ST_WAITING_FOR_MOVE;

    ctrl->pBoard->initialize();
    ctrl->pBoard->gen(0);

    // Uhr setzen
    ctrl->time_left  = ctrl->time_per_control;
    ctrl->moves_left = ctrl->moves_per_control;

    // Resign Zähler setzen
    ctrl->rep_resign_count = 0;

    // GameInfo Reset
    ctrl->gameInfo.appendToPGNFile("resp.pgn");
    ctrl->gameInfo.init();
}

///////////////////////
// CmdWritePgn
const char* Controller::CmdWritePgn::describtion()
{
    return "Writes the current game to resp.pgn";
}

void Controller::CmdWritePgn::execute(string &arg) throw (BadCmdException)
{
   ctrl->gameInfo.appendToPGNFile("resp.pgn");
}

///////////////////////
// CmdScore
const char* Controller::CmdScore::describtion() {
    return "Show detailed evaluation of current position.";
}

void Controller::CmdScore::execute(string &arg) throw (BadCmdException) {
   Eval::getHandle()->showEval();
}

///////////////////////
// CmdDump
const char* Controller::CmdDump::describtion()
{
    return "Display the board; with option -x show position in xboard.";
}

void Controller::CmdDump::execute(string &arg) throw (BadCmdException)
{
   if (StringTools::trim(arg) == "-x") {
      string fen = ctrl->pBoard->get_fen();
#ifdef LINUX
      ofstream pos("/tmp/tmp_resp.fen",ios::out);
      pos << fen << endl;
      pos.close();
      system("xboard -ncp -lpf /tmp/tmp_resp.fen&");
#endif
   } else {
      ctrl->pBoard->dump();
   }
}

///////////////////////
// CmdName
const char* Controller::CmdName::describtion() {
    return "Set the opponents name";
}

void Controller::CmdName::execute(string &arg) throw (BadCmdException) {
   ctrl->gameInfo.setHeader("Opponent",arg);
}

///////////////////////
// CmdResult
const char* Controller::CmdResult::describtion() {
    return "Set the result of the current game.";
}

void Controller::CmdResult::execute(string &arg) throw (BadCmdException) {
   ctrl->gameInfo.setHeader("Result",arg);
   ctrl->gameInfo.appendToPGNFile("resp.pgn");
   ctrl->gameInfo.init();
}

///////////////////////
// CmdShowFEN
const char* Controller::CmdShowFEN::describtion()
{
    return "Show FEN of current position.";
}

void Controller::CmdShowFEN::execute(string &arg) throw (BadCmdException)
{
    string fen = ctrl->pBoard->get_fen();
    cout << "FEN:\n" << fen << endl;
}
///////////////////////
// CmdTakeback
const char* Controller::CmdTakeback::describtion()
{
    return "Take back one move.";
}

void Controller::CmdTakeback::execute(string &arg) throw (BadCmdException)
{
    if (ctrl->pBoard->getTotalHPly() > 0)
    {
        ctrl->pBoard->takebackmove();
        ctrl->pBoard->gen(0);
        ctrl->updateStatus();

        ctrl->gameInfo.removeLast();
    }
}


///////////////////////
// CmdLoadFEN
const char* Controller::CmdLoadFEN::describtion()
{
    return "Load position in FEN format. [HELP]";
}

const char* Controller::CmdLoadFEN::details()
{
    return "- sb \"<file_name>\"|<FEN_string>\n" \
           "- with file_name : name of a file containing a FEN string\n" \
           "-      FEN_string: legal FEN string like 8/8/4Pk2/8/1K1N4/2N2b2/8/8 w - - 0 1\n";
}


void Controller::CmdLoadFEN::execute(string &arg) throw (BadCmdException)
{
    ctrl->gameInfo.appendToPGNFile("resp.pgn");
    ctrl->gameInfo.init();
    bool succ;
    if (arg[0] == '\"')
    {
        arg.erase(arg.begin());
        arg.erase(arg.length()-1);
        succ = ctrl->pBoard->load_fen_from_file(arg.c_str());
    }
    else
    {
        succ = ctrl->pBoard->load_fen(arg);
        ctrl->gameInfo.setHeader("FEN",arg);
        ctrl->gameInfo.setHeader("SetUp","1");
    }
 
    if (!succ)
    {
        out << "Error! FEN String not recognized." << endl;
        return;
    }
    
    ctrl->pBoard->gen(0);

    ctrl->status = ST_WAITING_FOR_MOVE;
    ctrl->updateStatus();


    // Uhr setzen
    ctrl->time_left  = ctrl->time_per_control;
    ctrl->moves_left = ctrl->moves_per_control;

    // Resign Zähler setzen
    ctrl->rep_resign_count = 0;
}

///////////////////////
// CmdLoadFEN
const char* Controller::CmdTestMO::describtion()
{
    return "Test move ordering. [HELP]";
}

const char* Controller::CmdTestMO::details()
{
    return "- testmo <file_name> <d> \n" \
           "- with file_name: name of a file containing test positions\n" \
           "-      d        : generate trees with depths d\n";
}


void Controller::CmdTestMO::execute(string &arg) throw (BadCmdException)
{
    // filename
    string tpath = RespOptions::getHandle()->getValue("dir.fen");
    arg = StringTools::trim(arg);
    int pos = arg.find_first_of(" \t");
    string tmp(arg,0,pos);
    tpath+= tmp; 

    // time_in_secs
    arg.erase(0,pos);
    arg = StringTools::trim(arg);

    int depth = atoi(arg.c_str());

    if (depth <= 0)
        throw BadCmdException("Illegal value for depth.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

    int    nPositions       = 0;
    double totalNodes       = 0.0;
    double totalQNodes      = 0.0;
    double totalBetaCutoffs = 0.0;
    double totalBetaFirst   = 0.0;
    int    line             = 0;


    // Datei zum lesen öffnen:
    ifstream ifile(tpath.c_str());

    // Konnte Datei geöffnet werden?
    if (!ifile)
    {
        throw (BadCmdException("Couldn't open file.",
            BadCmdException::IO_ERROR));
    }


    string fen_line;
    int totalTime = 0;
    while ( !ifile.eof())
    {
        getline(ifile,fen_line);
        line++;
        if (fen_line == "")
            continue;
        nPositions++;

        if (!ctrl->pBoard->load_fen(fen_line))
        {
            char buff[65];
	    // itoa(line,buff,10);
            sprintf(buff,"%i",line);
            string out = "Illegal FEN position in " + tpath + ", line " + buff;
            throw (BadCmdException(out, BadCmdException::IO_ERROR));
        }

        // Extensions aus
        ctrl->pSearch->setOptExtensions(false);   

        // -----------------
        //  Suche starten
        // -----------------
        int iter_depth;
        long start_time = time_in_ms();
        long epd_time;
        ctrl->pSearch->go(1800*1000,iter_depth, false, depth,0,epd_time,true);
        totalTime += time_in_ms() - start_time;

        // Extensions einschalten
        ctrl->pSearch->setOptExtensions(true);

        totalNodes  += ctrl->pSearch->nNodes;
        totalQNodes += ctrl->pSearch->nQNodes;
        totalBetaCutoffs += ctrl->pSearch->nFaileHigh;
        totalBetaFirst   += ctrl->pSearch->nFaileHighOnFirst;
    }


    out << endl;
    out << "------------------------------------------------------------------------------" << endl;
    out << "RESULTS:" << endl;
    out << setprecision(1) << fixed << right << endl;
    out << "FEN filename                   = " << tpath << endl;
    out << "Searched with depth            = " << depth << endl;
    out << "#Positions                     = " << nPositions << endl;
    out << "Average Nodes (w/o quiescence) = " << totalNodes/nPositions << endl;
    out << "Average Nodes (quiescence)     = " << totalQNodes/nPositions << endl;
    out << "Beta cutoffs on first move     = " << setprecision(2) << 
        totalBetaFirst/totalBetaCutoffs*100.0 << "%" << endl;
    out << "NPS                            = " << (totalNodes + totalQNodes)/totalTime*1000.0 
        << " N/sec" << endl;
    out << "Total time used                = " << totalTime/1000.0 << " sec" << endl;
    out << "------------------------------------------------------------------------------" << endl;
    out << endl;

}


///////////////////////
// CmdRunEPD
const char* Controller::CmdRunEPD::describtion()
{
    return "Run EPD Test suite. [HELP]";
}

const char* Controller::CmdRunEPD::details()
{
    return "- runepd [+] <filename> <time_in_secs> [ <position> ]\n" \
           "- with filename    : name of file with EPD test suite\n" \
           "-      time_in_secs: max. time to solve a position\n" \
           "-      position    : resp will only examine position with Id containing the\n" \
           "-                    substring <position>\n" \
           "-      +           : add results to 'results.unl'\n"\
           "- Example: 'runepd wac.epd 15 071' will run an test on WAC071 in wac.epd.\n";
}

void Controller::CmdRunEPD::execute(string &arg) throw (BadCmdException)
{

    // filename
    string epath = RespOptions::getHandle()->getValue("dir.epd");
    int pos = arg.find_first_of(" \t");
    string tmp(arg,0,pos);

    bool storeToResults = false;
    if (tmp == "+")
    {
        storeToResults = true;
        arg.erase(0,pos);
        arg = StringTools::trim(arg);
        pos = arg.find_first_of(" \t");
        tmp = string(arg,0,pos);
    }

    // Name der Testsuite
    string test_suite_name = StringTools::trim(tmp);
    // evtl. / oder \ herausfiltern
    int sl_pos = test_suite_name.find_last_of("\\/");
    test_suite_name.erase(0,sl_pos+1);


    epath+= tmp; 

    // time_in_secs
    arg.erase(0,pos);
    arg = StringTools::trim(arg);
    pos = arg.find_first_of(" \t");

    string position = "*";

    if (pos != -1)
    {
        position = arg;
        position.erase(0,pos);
        position = StringTools::trim(position);
        arg.erase(pos, arg.length());
    }

    int secs = atoi(arg.c_str());

    if (secs <= 0)
    {
        throw BadCmdException("Illegal value for time.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    }

    // Datei zum lesen öffnen:
    ifstream ifile(epath.c_str());

    // Konnte Datei geöffnet werden?
    if (!ifile)
    {
        throw (BadCmdException("Couldn't open file.",
            BadCmdException::IO_ERROR));
    }

    string id;
    Notation::OpCodeEnum op_code;
    string epd_line;
    vector<Move> move_list;
    int  n       = 0;
    int  n_pos   = 0;
    int  score   = 0;
    int  n_mates = 0;
    long tottime = 0;
    int  totDepthNotSolved  = 0;
    double totNodes         = 0;
    double totBetaCutoffs   = 0;
    double totCutFirst      = 0;

    // Uhr stellen
    ctrl->time_per_control = secs*1000;
    ctrl->time_mode = Controller::FIXED_ST;

    // Logfiles erstellen

    // Loserpositionen
    ofstream epd_loser("loser.epd",ios::out | ios::trunc);

    // Einzelresultate; Name der Datei: "Name Testsuite" + "Zeitstempel"
    string result_file = test_suite_name + timeDateString();

    ofstream res_log;
    if (storeToResults)
        res_log.open(result_file.c_str(), ios::out | ios::trunc);

    while ( !ifile.eof())
    {
        getline(ifile,epd_line);
        n++;

        if (epd_line == "")
            continue;

        // --------------------------------------------------------
        // Sollen nur bestimmte Positionen untersucht werden?
        if (position != "*")
        {
            // JA: Prüfen, ob position Teilstring von id

            // 'id "' suchen
            int id_pos = epd_line.find("id \"");

            if (id_pos != -1)
            {
                string sid(epd_line,id_pos + 4, epd_line.length());

                id_pos = sid.find_first_of("\"");
                if (id_pos != -1)
                    sid.erase(id_pos,sid.length());

                // position in id suchen
                if (sid.find(position) == string::npos)
                {
                    //cout << "Skipping." << endl;
                    continue;
                }
            }

        }
        // --------------------------------------------------------


        // EPD laden
        if (!Notation::load_epdline(epd_line,id,op_code,move_list,ctrl->pBoard))
        {
            out << "ERROR IN " << epath << endl;
            out << "COULDN'T PARSE LINE " << n << endl;
        }
        else
        {
            // Position aufgestellt

            // gültiger op_code ?
            if (op_code == Notation::NOT_FOUND)
            {
                out << "WARNING" << endl;
                out << "DID NOT FIND AM OR BM IN LINE " << n;
                out << "   SKIPPING" << endl;
                continue;
            }
            
            // Zugliste gefüllt?
            if (move_list.size() == 0)
            {
                out << "WARNING" << endl;
                out << "NO MOVES IN LINE " << n;
                out << "   SKIPPING" << endl;
                continue;
            }

            // Informationen zur Stellung
            out << endl;
            out << "    ***** POSITION: " << id << " *****" << endl;
            out << endl;
            out << "EPD: " << epd_line << endl << endl;


            // -----------------------------
            //  S u c h e  b e g i n n e n
            // -----------------------------
            int iter_depth;
            long epd_time;

            int sc;
            
            bool bm = op_code == Notation::BM;
            sc = ctrl->pSearch->go(ctrl->time_per_control, iter_depth, false, 500,
                                   &move_list,epd_time,bm);


            if (epd_time != -1)
                tottime += epd_time;
            // Statistik
            totNodes += ctrl->pSearch->nNodes + ctrl->pSearch->nQNodes;
            totBetaCutoffs += ctrl->pSearch->nFaileHigh;
            totCutFirst    += ctrl->pSearch->nFaileHighOnFirst;
            if (sc > 9500) n_mates++;


            Move m = ctrl->pSearch->pv[0][0].m;



            // -----------------------------
            //      A U S W E R T U N G
            // -----------------------------

            n_pos++;
            out << endl;
            out << "TEST SUITE NO." << n_pos;
            out << "   ID = " << id << endl;
            out << endl;
            out << "TYPE        = ";


            if (op_code == Notation::AM)
            {
                out << "avoid move";
            }
            else
            {
                out << "best move";
            }
            out << endl;

            out << "MOVELIST    = ";

            // Zug in move_list
            bool found = false;
            for (int i = 0; i < static_cast<int>(move_list.size()); i++)
            {
                out << move_list[i] << " ";
                if (move_list[i] == m)
                {
                    found = true;
                }
            }
            out << endl;

            out    << "ENGINE MOVE = " << m << endl;


            out << "STATUS      = ";
            if ( (op_code == Notation::AM && !found) ||
                 (op_code == Notation::BM && found) )
            {
                out << "SUCCESS :)" << endl;
                if (storeToResults)
                    res_log << epd_time << endl;
                score++;
            }
            else
            {
                out << "NO SUCCESS :(" << endl;
                epd_loser << epd_line << endl;
                if (storeToResults)
                    res_log << -1 << endl;
                totDepthNotSolved += iter_depth;
            }
            
        }

        out << "---------------------------------------------------------------------" << endl;
    }

    ifile.close();
    epd_loser.close();
    if (storeToResults)
        res_log.close();


    // Zusammenfassung

    out << endl;
    out << endl;
    out << "STATISTIC FOR " << epath << endl;
    out << endl;
    out << "SCORE                        = " << score << "/" << n_pos << endl;
    out << "AVER. DEPTH IN UNSOLVED POS. = ";
    double avDepth;
    if (n_pos - score > 0) 
        avDepth = static_cast<double>(totDepthNotSolved)/(n_pos - score);
    else
        avDepth = -1;
    if (avDepth > 0) 
        out << avDepth << endl;
    else
        out << "-" << endl;
    out << "TIME                         = " << setprecision(2) <<  tottime/1000.0 << endl;
    out << endl;
    out << "Loser-positions written to 'loser.epd'." << endl;



    if (storeToResults)
    {
        // --------------------------------------------------------------
        //  Ergebnisse im Unload Format speichern (nach "results.unl")
        // --------------------------------------------------------------
        // Loserpositionen
        ofstream unl("results.unl",ios::out | ios::app);

        unl << fixed << setprecision(2);

        unl << test_suite_name << "|";  // Name der Testsuite
        unl << "|";                     // Rechnername
        unl << "resp" << "|";           // Programmname
        unl << Version::major_version << ".";          // Version
        unl << Version::minor_version << ".";
        unl << Version::build_number;
#ifdef USE_MMXASM
        unl << " mmx";
#endif
        unl << "|";
        unl << Hash::getHandle()->getSize() / (1024*1024) << "|";  // Hashsize (MB)

        unl << "|";                         // Konf. sonstiges

        unl << dateString() << "|";         // Datum

        unl << n_pos << "|";                // Anzahl Positionen
        unl << score << "|";                // Treffer
        unl << secs << "|";                 // Zeit pro Zug
        if (avDepth > 0)
            unl << avDepth <<  "|";         // Durchschn. Suchtiefe
        else
            unl << "|";
        unl << totNodes/n_pos <<  "|";          // Durchschn. Anzahl Knoten
        unl << tottime/1000.0 << "|";           // Benoetigte Zeit
        unl << n_mates <<  "|";                 // Matts gefunden
        unl << totCutFirst*100/totBetaCutoffs;  // Moveordering

        unl << "|";
        unl << result_file;                     // Name der Datei mit den Einzelergebnissen
        unl << endl;

        unl.close();

        out << "Result appended to 'results.unl'." << endl;
    }

}


///////////////////////
// CmdCreateBook
const char* Controller::CmdCreateKEDB::describtion()
{
    return "Create Knowledgeable Engame Database.";
}


void Controller::CmdCreateKEDB::execute(string &arg) throw (BadCmdException)
{
    KEDB::getHandle()->create_kpk_db();
}


///////////////////////
// CmdCreateBook
const char* Controller::CmdCreateBook::describtion()
{
    return "Create opening book from pgn file. [HELP]";
}

const char* Controller::CmdCreateBook::details()
{
    return "- createbook <pgnfile> <bookfile> <depth>\n" \
           "- with pgnfile : filename of a valid PGN file, for example openings.pgn\n" \
           "-      bookfile: filename for the resulting resp-book, for example openings.bin\n" \
           "-      depths  : truncate booklines to <depths> moves.\n";
}

void Controller::CmdCreateBook::execute(string &arg) throw (BadCmdException)
{
    int pos;

    //  ----------------------------
    //      Argumente parsen
    //  ----------------------------
    pos = arg.find_first_of(" \t");
    if (pos == -1)
    {
        throw BadCmdException("Not enough parameters.", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    }

    string pgnfile = RespOptions::getHandle()->getValue("dir.pgn") 
                     + string(arg,0,pos);

    arg.erase(0,pos);
    arg = StringTools::trim(arg);
    pos = arg.find_first_of(" \t");
    if (pos == -1)
    {
        throw BadCmdException("Not enough parameters.", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    }

    string bookfile = RespOptions::getHandle()->getValue("dir.book") 
                      + string(arg,0,pos);

    arg.erase(0,pos);

    int depth = atoi(arg.c_str());

    if (depth <= 0 )
    {
        throw BadCmdException("depth not in range.", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    }

    //  -----------------------------------
    //      PGN-Datei nach vector einlesen
    //  -----------------------------------

    vector<BookRecord> bdata;
    PGN_Parser* p_parser = ctrl->pgnParser;

    p_parser->create();

    int count = 0;      // Anzahl Spiele
    int pos_count = 0;  // Anzahl Positionen
    try
    {
        // DB öffnen
        p_parser->findfirst(pgnfile.c_str());


        // DB durchlaufen
        while (true)
        {
            //  -------------------------------------------------
            //  Spiel innerhalb der PGN-Datenbank untersuchen 
            //  -------------------------------------------------

            count++;

            // gibt es einen FEN-tag? (Sollte in einer Eröffnungs-
            // datenbank eigentlich nicht vorkommen :)
            map<string,string>::iterator it;
            bool from_fen;

            TagList taglist = p_parser->getTagList();
            it = taglist.find("FEN");
            if ( (from_fen = it != taglist.end()) )
            {
                ctrl->pBoard->load_fen((*it).second);
            }
            else // kein FEN-Tag: normal initialisieren
                ctrl->pBoard->initialize();

            // -------------------------------
            //  Spielinfo ausgeben
            // -------------------------------
            out << "GAME : " << count << "   ";
            it = taglist.find("White");
            if ( it != taglist.end() )
                out << (*it).second;
            it = taglist.find("Black");
            if ( it != taglist.end() )
                out << " " << (*it).second;
            it = taglist.find("Result");
            if ( it != taglist.end() )
                out << " " << (*it).second;
            out << endl;

            ctrl->pBoard->gen(0);

            // --------------------------------
            //  Züge bis zur Tiefe depth holen
            // --------------------------------
            MoveList& ml = p_parser->getMoveList();
            for (int ply = 0; ply < static_cast<int>(ml.size()) && ply < depth; ply++)
            {
                out << " " << ml[ply].toString();

                // Zug ausführen
                ctrl->pBoard->makemove(ml[ply]);

                // Plausibilitätskontrolle
                if (ctrl->pBoard->kingAttacked(XSIDE(ctrl->pBoard->sideToMove())))
                {
                    ctrl->pBoard->takebackmove();
                    // Zug nicht gültig?!
                    out << "FEHLER in controller.cpp" << endl;
                    exit(0);
                }

                
                // Aktuelle Brettposition abspeichern

                if ( p_parser->getGameResult() == REMIS || 
                    (p_parser->getGameResult() == WHITE_WIN && ctrl->pBoard->sideToMove() == BLACK) ||
                    (p_parser->getGameResult() == BLACK_WIN && ctrl->pBoard->sideToMove() == WHITE) )
                {
                    pos_count++;
                    bdata.push_back( BookRecord(ctrl->pBoard->get_z1_hash(), 
                                                ctrl->pBoard->get_z2_hash(),
                                                ctrl->pBoard->sideToMove()) );
                    out << "[+]";
                }
            }

            out << endl;

            // -------------------------------
            // Nächstes Spiel aus DB holen
            // -------------------------------
            if (p_parser->end_of_pgn_db())
                break;
            p_parser->findnext();
        }
    }
    catch (ParseErrorException e)
    {
        throw BadCmdException(e.what(), BadCmdException::PGN_OPEN);
    }

    ctrl->pgn_game_loaded = true;


    // --------------------------------------------------------
    //  Eingescannte Daten jetzt in Binaer-Datei speichern
    // --------------------------------------------------------

    if (Book::createBook(bookfile,bdata))
    {
        out << "Creating opening book " << bookfile << " succesful." << endl;
        out << "NUMBER OF GAMES       : " << count << endl;
        out << "NUMBER OF POSITIONS   : " << pos_count << endl;
        out << endl;
    }
    else
    {
        out << "ERROR: Could not create bookfile.";
    }
}



///////////////////////
// CmdDebugDump
const char* Controller::CmdDebugCommand::describtion()
{
    return "Debug [b]itBoards | [m]oves | [S]witch side "\
           "| [e]valuate | m[i]nimax | [h]ash-info | [k]iller "\
           "| [K]ing evasions";
} 

void Controller::CmdDebugCommand::execute(string &arg) throw (BadCmdException)
{
    if (arg[0] == 'b')
        ctrl->pBoard->dumpBitBoards();
    else if (arg[0] == 'm')
    {
        //ctrl->pBoard->moveStack[0].simple_sort();
       // ctrl->pBoard->moveStack[0].dump();

        MoveStack& ms = ctrl->pBoard->moveStack[0];
        Board* pB = Board::getHandle();

        pB->gen(0);
//        ctrl->pSearch->calc_move_score(0,0);

        cout << "MOVES" << endl;
        for (int i=0; i < ms.size(); i++)
        {
            cout << ms.stack[i];

            // SAN - Ausgabe
            string san = Notation::move_to_san(ms.stack[i],pB);

            cout << "  " << san;


            pB->makemove(ms.stack[i]);

            BookRecord bkrec(pB->get_z1_hash(), pB->get_z2_hash(), pB->sideToMove());
            bool found = ctrl->book.findPosition(bkrec);
            pB->takebackmove();

            if (found)
                cout << " [BOOK: " << bkrec.get_count() << "]";
            else 
                cout << "       ";


            cout << " SCORE=" << ms.score[i];

            if (ms.findKiller(ms.stack[i]))
                cout << " [KILLER]";

            if (ms.stack[i].isCapture())
            {
                int see_val = pB->see(ms.stack[i]);
                cout << "  -> see_val = " << see_val; 
            }

            cout << endl;
        }
    }
    else if (arg[0] == 'S')
            ctrl->pBoard->switchSide();
    else if (arg[0] == 'k')
    {
        for (int i = 0; i < MAXPLY; i++)
        {
            cout << i << ") ";
            cout << ctrl->pBoard->moveStack[i].killer1 << "  ";
            cout << ctrl->pBoard->moveStack[i].killer2 << endl;
        }
    }
    else if (arg[0] == 'e')
    {
        long score = Eval::getHandle()->eval();
        cout << "SCORE = " << score << endl;

        ctrl->pBoard->dumpStats();

    }
    else if (arg[0] == 'i')
    {
        Move m;

        if (ctrl->use_book)
        {
            double dummy;
            // Zufällig legalen Zug auswählen
            m = ctrl->randomBookMove(true, dummy);
        }
        if (!ctrl->use_book || m==0)
        {
            int iter_depth;
            long epd_time;
            ctrl->pSearch->go(3,iter_depth, false,500,0,epd_time,false);
            m = ctrl->pSearch->pv[0][0].m;
        }
        ctrl->pBoard->makemove(m);
        ctrl->pBoard->gen(0);
        ctrl->updateStatus();
    }
    else if (arg[0] == 'h')
    {
        // Statistik zum Transpositiontable
        out << "TT-STATISTIC" << endl;
        out << endl;
        out << "INSERT->FREE     : " << Hash::getHandle()->nInsertFree << endl;
        out << "INSERT->DEPTH >= : " << Hash::getHandle()->nInsertDepth << endl;
        out << "INSERT->AGE   >= : " << Hash::getHandle()->nAge << endl;
        out << "INSERT->UNSAFE   : " << Hash::getHandle()->nInsertUnsafe << endl;
        out << "INSERT->FAILED   : " << Hash::getHandle()->nFailedRehashs << endl;
        out << endl;
        for (int i=0; i < Hash::getHandle()->getMaxRehash(); i++)
            out << "FOUND [" << i << "] : " << 
                Hash::getHandle()->nFound[i] << endl; 

        out << endl;
        out << "PAWN TT STATISTIC" << endl;
        out << endl;
        out << "NUMBER OF INSERTS: " << PHash::getHandle()->nInsert << endl;
        out << "FOUND            : " << PHash::getHandle()->nFound << endl;
        out << "NOT FOUND        : " << PHash::getHandle()->nNotFound << endl;
        out << endl;
    }
    else if (arg[0] == 'g') {
       // dump GameInfo
       ctrl->gameInfo.dump();
    }
    else if (arg[0] == 'K')
    {
        if (ctrl->pBoard->sideToMove() == WHITE &&
            ctrl->pBoard->kingAttacked(WHITE) )
        {
            ctrl->pBoard->genKingEvasions(0);

            MoveStack& ms = ctrl->pBoard->moveStack[0];

            cout << "------------------------------------------------------" << endl;
            cout << "MOVES" << endl;
            for (int i=0; i < ms.size(); i++)
            {
                Move m = ms.stack[i];
                cout << m << " ";

                cout << "PIECE: " << m.getPiece() << " ";
                cout << "CAP_PIECE: " << m.getCapturedPiece() << " ";
                cout << "PROM_PIECE: " << m.getPromPiece() << " ";

                if (m.isCapture())
                    cout << "CAP ";
                if (m.isCastleMove())
                    cout << "CSTL ";
                if (m.isEpCapture())
                    cout << "EP_CAP ";
                if (m.isPawn2Squares())
                    cout << "2SQUA ";
                if (m.isPawnMove())
                    cout << "PAWN ";
                if (m.isPromotion())
                    cout << "PROM ";
                cout << endl;
            }
            cout << "------------------------------------------------------" << endl;

        }
    }
    
}


///////////////////////
// CmdPerft
const char* Controller::CmdPerft::describtion()
{
    return "perft <d>: run perft with depth d.";
}

void Controller::CmdPerft::execute(string &arg) throw (BadCmdException)
{
    int depth = 3;

    if (arg.length() > 0)
        depth = atoi(arg.c_str());

    ctrl->pSearch->run_perft(depth);
}


///////////////////////
// CmdLoadBook
const char* Controller::CmdLoadBook::describtion()
{
    return "Load opening book: loadbook <bookfile>|off.";
}

void Controller::CmdLoadBook::execute(string &arg) throw (BadCmdException)
{
    if (arg == "off")
    {
        ctrl->use_book = false;
        return;
    }

    string bookfile = RespOptions::getHandle()->getValue("dir.book") + arg;
    if (!ctrl->book.setBookFile(bookfile))
    {
        out << "Error" << endl;
        out << "Could not load opening book " << bookfile << endl;
        ctrl->use_book = false;
        return;
    }

    out << "Using opening book " << bookfile << "." << endl;
    ctrl->use_book = true;
}



///////////////////////
// CmdInfo
const char* Controller::CmdInfo::describtion()
{
    return "Show info about resp.";
}

void Controller::CmdInfo::execute(string &arg) throw (BadCmdException)
{
    out << endl;
    out << "----------------------------------------------------------" << endl;
    out << "-                     r e s p" << endl;
    out << "-" << endl;
    out << "- Chess program by Peter Rosendahl" << endl;
    out << "-" << endl;
    out << "- VERSION               : " << Version::major_version << "." 
                                        << Version::minor_version << "."
                                        << Version::build_number
                                        << endl;

    out << "- COMPILED              : ";
    out << Version::build_date << ", ";
    out << Version::build_time << endl;

    out << "- CVS-ID version.cpp    : ";
    out << Version::cvs_id << endl;

#ifdef USE_TRANSPOSITION_TABLE
    out << "- USING HASHTABLE       : YES" << endl;
    out << "- HASHSIZE (KByte)      : " << Hash::getHandle()->getSize()/1024 << endl;
    out << "- HASHSIZE - MAXENTRIES : " << Hash::getHandle()->getMaxEntries() << endl;
#else
    out << "- USING HASHTABLE       : NO" << endl;
#endif

#ifdef USE_PAWNHASH
    out << "- USING PAWNHASH        : YES" << endl;
    out << "- PHASHSIZE (KByte)     : " << PHash::getHandle()->getSize()/1024 << endl;
    out << "- PHASHSIZE - MAXENTRIES: " << PHash::getHandle()->getMaxEntries() << endl;
#else
    out << "- USING PAWNHASH        : NO" << endl;
#endif

    out << "- MMX - OPTIMIZATIONS   : "; 
#ifdef USE_MMXASM
    out << "YES" << endl;
#else
    out << "NO" << endl;
#endif

    out << "- MSB-LSB WITH ASM      : ";
#ifdef USE_ASM_MSBLSB
    out << "YES" << endl;
#else
    out << "NO" << endl;
#endif

    out << "----------------------------------------------------------" << endl;

    out << endl;
}


///////////////////////
// CmdHelp
const char* Controller::CmdHelp::describtion()
{
    return "Show help.";
}

void Controller::CmdHelp::execute(string &arg) throw (BadCmdException)
{

    // Hilfe für einen speziellen Befehl?

    StringTools::trim(arg);
    if (arg.length() > 0)
    {
        tableIterator it = ctrl->cmdTable.find(arg);
    
        if (it == ctrl->cmdTable.end())
            throw BadCmdException("Nothing known about: "+arg, 
                BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    
        Command* cmd = (*it).second;

        cout << endl;
        cout << "COMMAND    : " << arg << endl;
        cout << "DESCRIBTION: " << cmd->describtion() << endl;
        cout << endl;
        cout << "DETAILS..." << endl;
        cout << cmd->details() << endl;
        return;
    }

    // Befehle aus cmdTable --> cmds
    vector<Command*> cmds;
    Controller::tableIterator it = ctrl->cmdTable.begin();

    for (; it != (ctrl->cmdTable).end(); it++)
        cmds.push_back((*it).second);


    // cmds sortieren und Duplikate entfernen
    sort(cmds.begin(),cmds.end());
    unique(cmds.begin(),cmds.end());

    // um die Kommandos nach ihrer Syntax zu sortieren:
    // cmds -> smallTable
    map<string, Command*> smallTable;
    for (int i=0; i < static_cast<int>(cmds.size()); i++)
        smallTable[cmds[i]->cmdStrings[0]] = cmds[i];

    // cmds loeschen
    cmds.clear();

    // smallTable --> cmds
    it = smallTable.begin();
    for (; it != smallTable.end(); it++)
        cmds.push_back((*it).second);

    vector<Command*>::iterator sit = cmds.begin();

    int more = 0;
    for (; sit != cmds.end(); sit++)
    {
        string s = (*sit)->cmdStrings[0];
        for (unsigned int j = 1; j < (*sit)->cmdStrings.size(); j++)
            s += ", " + (*sit)->cmdStrings[j];
        cout << "- " <<  setw(12) << left << s << ": " <<  (*sit)->describtion() << endl;

        if (! (++more % 18))
        {
            cout << endl;
            cout << "More (y/n)?" << endl;

            char s[11];
            cin.getline(s,10);

            if (*s != 'y')
                break;
        }
    }

    cin.clear();

}

///////////////////////
// CmdSanMove
const char* Controller::CmdSanMove::describtion()
{
    return "Make move using SAN.";
}

void Controller::CmdSanMove::execute(string &arg) throw (BadCmdException)
{
    Notation::SAN_ParseError err;

    Move m = Notation::san_to_move(arg,err,ctrl->pBoard);

    if (err != 0)
    {
        throw BadCmdException("Illegal syntax", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
        return;
    }

    string san = Notation::move_to_san(m,ctrl->pBoard);
    ctrl->pBoard->makemove(m);
    if (ctrl->pBoard->kingAttacked(XSIDE(ctrl->pBoard->sideToMove())))
    {
        ctrl->pBoard->takebackmove();
        throw BadCmdException("Illegal move -- would be check!", BadCmdException::ILLEGAL_MOVE);
        return;
    }

    // GameInfo aktualisieren
    MoveInfo mi;
    mi.sanMove = arg;
    mi.respMove = false;
    mi.whiteMove = ctrl->pBoard->sideToMove() == BLACK;
    ctrl->gameInfo.addMoveInfo(mi);
    
    // Zug erfolgreich ausgeführt -- gen aufrufen
    ctrl->pBoard->gen(0);
    // status aktualisieren
    ctrl->updateStatus();
}

///////////////////////
// CmdLevel
const char* Controller::CmdLevel::describtion()
{
    return "Set time control: level <num_of_moves> <mmm:ss> <increment>.";
}

void Controller::CmdLevel::execute(string &arg) throw (BadCmdException)
{
    string s = arg;

    int pos = s.find_first_of(" \t");

    string t (arg,0,pos);

    int num_moves = atoi(t.c_str());

    if (num_moves < 0)
        throw BadCmdException("Illegal value for number of moves.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

    s.erase(0,pos);

    s = StringTools::trim(s);

    pos = s.find(':');
    int pos_blank = s.find(' ');

    int min,sec;
    if (pos == -1)
    {
        string smin(s,0,pos_blank);
        sec = 0;
        min = atoi(smin.c_str());
    }
    else
    {
        string smin(s,0,pos);
        string ssec(s,pos+1,pos_blank-pos-1);

        min = atoi(smin.c_str());
        sec = atoi(ssec.c_str());
    }
    long msec  = (60*min + sec) * 1000;
    if (msec <= 0)
        throw BadCmdException("Illegal value for timecontrol.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

    // Inkrement lesen
    s.erase(0,pos_blank);
    s = StringTools::trim(s);

    int increm = atoi(s.c_str())*1000;

    if (increm < 0)
        throw BadCmdException("Illegal value for increment.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

    
    // Werte eingelesen: Optionen entsprechend setzen

    ctrl->moves_per_control  = num_moves;
    ctrl->time_per_control   = msec;
    ctrl->time_mode          = Controller::CONV_CLOCK;
    ctrl->time_inc           = increm;

    ctrl->time_left          = msec;
    ctrl->moves_left         = num_moves;

    // GameInfo aktualisieren
    ostringstream oss;

    if (num_moves == 0 && increm == 0) {
       oss << (msec / 1000);
    } else {
       if (num_moves > 0) {
          oss << num_moves << "/" << (msec/1000);
       } else {
          oss << (msec/1000) << "+" << (increm/1000);
       }
    }
    
    ctrl->gameInfo.setHeader("TimeControl",oss.str());

}

///////////////////////
// CmdGetOptions
const char* Controller::CmdGetOptions::describtion()
{
    return "Gets option values.";
}

const char* Controller::CmdGetOptions::details()
{
    return "- get [<mask>] ...\n" \
           "- with mask: prints only the values for options with substring mask.\n" \
           "- Example: get dir\n";
}

void Controller::CmdGetOptions::execute(string &arg) throw (BadCmdException)
{
   arg = StringTools::trim(arg);
   RespOptions::getHandle()->printOptions(arg);
}

///////////////////////
// CmdSetOption
const char* Controller::CmdSetOption::describtion()
{
    return "Sets the value of an option.";
}

const char* Controller::CmdSetOption::details()
{
    return "- set <option> <value>\n" \
           "- with <option>: name of an option\n" \
           "- with <value>: value to be bound to that option\n" \
           "- Example: set book.name mybook.bin\n";
}

void Controller::CmdSetOption::execute(string &arg) throw (BadCmdException)
{
   arg = StringTools::trim(arg);
   int pos = arg.find_first_of(" \t");

   if (pos == -1) {
      throw BadCmdException("I need two arguments.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
   }

   string option = string(arg,0,pos);
   arg.erase(0,pos);
   string value = StringTools::trim(arg);

   RespOptions::getHandle()->setValue(option,value);
}


///////////////////////
// CmdTime
const char* Controller::CmdTime::describtion()
{
    return "Set clock: time <centi_seconds>.";
}

void Controller::CmdTime::execute(string &arg) throw (BadCmdException)
{

    long msec  = atoi(arg.c_str()) * 10;

    if (msec <= 0)
        throw BadCmdException("Illegal value for timecontrol.", 
            BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
    
    //  Uhr zurücksetzen
    ctrl->time_left          = msec;


    // DEBUG
    //cout << "____CmdTime_>TIME_LEFT = " << ctrl->time_left << endl;

}

///////////////////////
// CmdTime
const char* Controller::CmdSetSearchOptions::describtion()
{
    return "Set search options. [HELP]";
}

const char* Controller::CmdSetSearchOptions::details()
{
    return "- sset <opt>+|- ...\n" \
           "- with opt: 'n' for nullmoves\n" \
           "-           'e' for extensions\n" \
           "-           'f' for extended futility pruning\n" \
           "- Example: sset n- e- turn nullmoves and extensions off.\n";
}


void Controller::CmdSetSearchOptions::execute(string &arg) throw (BadCmdException)
{
    arg = StringTools::trim(arg);

    while (arg.length() > 0)
    {
        switch (arg[0])
        {
        case 'n':  // nullmoves
            arg.erase(arg.begin());
            if (arg.length() == 0 || (arg[0] != '+' && arg[0] != '-') )
                throw BadCmdException("Illegal syntax for setting nullmove option.", 
                    BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

            if (arg[0] == '+')
                ctrl->pSearch->setOptNullmoves(true);
            else
                ctrl->pSearch->setOptNullmoves(false);
            arg.erase(arg.begin());
            arg = StringTools::trim(arg);
            break;
        case 'e':  // extensions
            arg.erase(arg.begin());
            if (arg.length() == 0 || (arg[0] != '+' && arg[0] != '-') )
                throw BadCmdException("Illegal syntax for setting extensions option.", 
                    BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

            if (arg[0] == '+')
                ctrl->pSearch->setOptExtensions(true);
            else
                ctrl->pSearch->setOptExtensions(false);
            arg.erase(arg.begin());
            arg = StringTools::trim(arg);
            break;
        case 'f':  // Extended Futility Pruning
            arg.erase(arg.begin());
            if (arg.length() == 0 || (arg[0] != '+' && arg[0] != '-') )
                throw BadCmdException("Illegal syntax for setting razoring option.", 
                    BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

            if (arg[0] == '+')
                ctrl->pSearch->setOptEFPruning(true);
            else
                ctrl->pSearch->setOptEFPruning(false);
            arg.erase(arg.begin());
            arg = StringTools::trim(arg);
            break;
        default:
            // Parameter nicht bekannt

            string out = "Illegal parameter " + arg[0];
            throw BadCmdException(out, BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
        }
    }

    // Gewählte Optionen darstellen

    out << endl;
    out << "- Extensions         : ";
    if (ctrl->pSearch->getOptExtensions())
        out << "on" << endl;
    else 
        out << "off" << endl;

    out << "- Nullmoves          : ";
    if (ctrl->pSearch->getOptNullMoves())
        out << "on" << endl;
    else
        out << "off" << endl;

    out << "- Ext.Futil.Pruning  : ";
    if (ctrl->pSearch->getOptEFPruning())
        out << "on" << endl;
    else
        out << "off" << endl;

    out << endl;
}



///////////////////////
// CmdMove
const char* Controller::CmdMove::describtion()
{
    return "Make Move m <LDLD[P]>.    L=a..h, D=1..8, P=bnrq";
}

void Controller::CmdMove::execute(string &arg) throw (BadCmdException)
{
    BYTE from, to, prom;

    if (arg.length() < 4 )
    {
        throw BadCmdException("Illegal syntax", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
        return;
    }

    from = (arg[0]-'a') + 8*(7 - (arg[1] - '1'));
    to   = (arg[2]-'a') + 8*(7 - (arg[3] - '1'));

    if (! (from<=63 &&  to<=63 ) )
    {
        throw BadCmdException("Illegal syntax", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);
        return;
    }

    prom = 0;
    if (arg.length() == 5)  // evtl. Promote?
    {
        switch (tolower(arg[4]))
        {
        case 'b': prom = BISHOP; break;
        case 'n': prom = KNIGHT; break;
        case 'r': prom = ROOK; break;
        case 'q': prom = QUEEN; break;
        default: prom = 0;
        }
    }

    Move m = ctrl->pBoard->moveStack[0].findMove(from,to,prom);

    if (m.data == 0)  // nichts gefunden?
    {
        throw BadCmdException("Illegal move.", BadCmdException::ILLEGAL_MOVE);
        return;
    }

    string san = Notation::move_to_san(m,ctrl->pBoard);
    ctrl->pBoard->makemove(m);
    if (ctrl->pBoard->kingAttacked(XSIDE(ctrl->pBoard->sideToMove())))
    {
        ctrl->pBoard->takebackmove();
        throw BadCmdException("Illegal move -- would be check!", BadCmdException::ILLEGAL_MOVE);
        return;
    }

    // GameInfo aktualisieren
    MoveInfo mi;
    mi.sanMove = san;
    mi.respMove = false;
    mi.whiteMove = ctrl->pBoard->sideToMove() == BLACK;
    ctrl->gameInfo.addMoveInfo(mi);

    // Zug erfolgreich ausgeführt -- gen aufrufen
    ctrl->pBoard->gen(0);

    // status aktualisieren
    ctrl->updateStatus();

        
}



///////////////////////
// CmdOpenPGNDatabase
const char* Controller::CmdOpenPGNDatabase::describtion()
{
    return "PGN <filename> - opens a PGN database file.";
}

void Controller::CmdOpenPGNDatabase::execute(string &arg) throw (BadCmdException)
{
    ctrl->pgnParser->create("error.log");

    string full = RespOptions::getHandle()->getValue("dir.pgn")+arg;

    try
    {
        ctrl->pgnParser->findfirst(full.c_str());
    }
    catch (ParseErrorException e)
    {
        throw BadCmdException(e.what(), BadCmdException::PGN_OPEN);
    }

    ctrl->pgn_game_loaded = true;

}

///////////////////////
// CmdNextGameFromPGNDB
const char* Controller::CmdNextGameFromPGNDB::describtion()
{
    return "Gets the next game from an opened PGN database file.";
}

void Controller::CmdNextGameFromPGNDB::execute(string &arg) throw (BadCmdException)
{

    if (!ctrl->pgn_game_loaded)
    {
        throw BadCmdException("No PGN-File open.", BadCmdException::PGN_OPEN);
        return;
    }


    try
    {
        ctrl->pgnParser->findnext();
    }
    catch (ParseErrorException e)
    {
        throw BadCmdException(e.what(), BadCmdException::PGN_OPEN);
    }
}

///////////////////////
// CmdScanPGNDatabase
const char* Controller::CmdScanPGNDatabase::describtion()
{
    return "Scan syntax of complete database.";
}

void Controller::CmdScanPGNDatabase::execute(string &arg) throw (BadCmdException)
{
    int count;

    ctrl->pgnParser->create("error.log");

    try
    {
        string full = RespOptions::getHandle()->getValue("dir.pgn")+arg;
        ctrl->pgnParser->findfirst(full.c_str());

        count = 1;

        while (!ctrl->pgnParser->end_of_pgn_db())
        {
            count++;
            ctrl->pgnParser->findnext();
        }
    }
    catch (ParseErrorException e)
    {
        throw BadCmdException(e.what(), BadCmdException::PGN_OPEN);
    }

    ctrl->pgn_game_loaded = true;

    out << "======================================" << endl;
    out << "TOTAL GAMES READ: " << count << endl;

}


///////////////////////
// CmdOpenPGNDatabase
const char* Controller::CmdPGNMove::describtion()
{
    return "pgn + | - | [n]ew | #";
}

void Controller::CmdPGNMove::execute(string &arg) throw (BadCmdException)
{
    if (!ctrl->pgn_game_loaded)
    {
        throw BadCmdException("No PGN-File open.", BadCmdException::PGN_OPEN);
    }

    MoveList& ml = ctrl->pgnParser->getMoveList();

    switch (arg[0])
    {
    case '+':
        if (ctrl->nPGNMove < static_cast<int>(ml.size()))
            ctrl->pBoard->makemove(ml[ctrl->nPGNMove++]);
        ctrl->pBoard->gen(0);
        ctrl->updateStatus();
        break;
    case '-':
        ctrl->pBoard->takebackmove();
        ctrl->pBoard->gen(0);
        break;
    case 'n':
        {
        ctrl->status = ST_WAITING_FOR_MOVE;

        // gibt es einen FEN-tag?
        map<string,string>::iterator it;

        TagList taglist = ctrl->pgnParser->getTagList();
        it = taglist.find("FEN");
        if (it != taglist.end())
        {
            ctrl->pBoard->load_fen((*it).second);
        }
        else
            ctrl->pBoard->initialize();

        ctrl->updateStatus();

        ctrl->pBoard->gen(0);
        ctrl->nPGNMove = 0;
        break;
        }

    case '#':
        ctrl->pBoard->initialize();
        ctrl->pBoard->gen(0);
        ctrl->status = ST_WHITE_TO_MOVE | ST_WAITING_FOR_MOVE;

        unsigned int i;
        for (i=0; i < ml.size(); i++)
        {
            out << ctrl->pBoard->getTotalHPly() << ", " << ctrl->pBoard->getFifty() << ": ";
            out << ml[i] <<  "  ";

            ctrl->updateStatus();
            ctrl->dumpGameStatus();
            out << endl;

            ctrl->pBoard->makemove(ml[i]);
            ctrl->pBoard->gen(0);
        }
        out << endl;
        ctrl->nPGNMove = i;
        break;
    }

}



///////////////////////
// CmdSearch
const char* Controller::CmdSearch::describtion()
{
    return "Computer searches for a move.";
}

void Controller::CmdSearch::execute(string &arg) throw (BadCmdException)
{
    Move m;
    MoveInfo mi;

    if (ctrl->use_book)
    {   
        // -------------------------------------------------------
        //  Zufällig legalen Zug aus Eröffnungsbuch  auswählen
        // -------------------------------------------------------
        double perc;
        m = ctrl->randomBookMove(true, perc);

        if (m != 0) {
           mi.bookMove = true;

           ostringstream oss;
           oss << perc*100.0 << "%";
           mi.bookInfo = oss.str();
        }
    }
    if (!ctrl->use_book || m==0)
    {
        mi.bookMove = false;

        // -------------------------------------------------------
        //                  Zug suchen
        // -------------------------------------------------------


        // Zug Verfügung stehende Zeit bestimmen

        long max_time = 10000;

        
        if (ctrl->time_mode == Controller::CONV_CLOCK)
        {
            if (ctrl->moves_per_control == 0)
            {
                const int N_AVRG = 50;

                // Spezialfall: Anzahl Züge == 0  =>  Komplette Partie muss in
                //              ctrl->time_left gespielt werden

                // Die Zeit pro Zug errechne ich in Abhängigkeit von der zur Verfügung stehenden
                // Zeit:
                //          Z(t) = (T_max^2 - t^2) / (T_max * N_AVRG)
                //
                // Insbesondere ist also Z(0) = T_max / N_AVRG
#ifdef USE_MMXASM
                clear_fpu();
#endif
                double t        = ctrl->time_per_control - ctrl->time_left;
                double t_max    = ctrl->time_per_control ;
                if (t < 0) 
                    max_time = ctrl->time_left/N_AVRG;  // z.B level 0 2 5
                else
                    max_time = static_cast<long>((t_max*t_max - t*t) / (t_max * N_AVRG));

                if (ctrl->time_left < 10000)   // Zeitnot
                    max_time += (70*ctrl->time_inc)/100;
                else
                    max_time += ctrl->time_inc;

                if (max_time < 1)
                    max_time = 1;
            }

            else
            {
                // Mit Zeitkontrolle

                // Zeit linear aufteilen
                max_time = (ctrl->time_left - NO_FLAG_BUFF) / ctrl->moves_left +
                            ctrl->time_inc;

                // Eröffnung?
                if (ctrl->pBoard->getTotalHPly() < 30)
                    max_time = (max_time*110) / 100;
            }

        }
        else if (ctrl->time_mode == Controller::FIXED_ST)
        {
                max_time = ctrl->time_per_control;
        }

        // Startmarke
        long start_time = time_in_ms();

        // -----------------------------
        //  S u c h e  b e g i n n e n
        // -----------------------------
        int iter_depth;
        long epd_time;
        int sc = ctrl->pSearch->go(max_time, iter_depth, false, 500, 0, epd_time, false);

        mi.score = sc;
        ctrl->update_resign(sc);


        // Verwendete Zeit berechnen
        long diff = time_in_ms() - start_time;
        mi.time = diff;
        ctrl->time_left -= diff;
        m = ctrl->pSearch->pv[0][0].m;
    }

    // Zeitkontrolle
    if (ctrl->moves_per_control != 0)
    {
        ctrl->moves_left--;

        if (ctrl->moves_left <= 0)
        {
            ctrl->moves_left = ctrl->moves_per_control;
            ctrl->time_left += ctrl->time_per_control;
        }
    }

    ctrl->pBoard->gen(0);
    mi.sanMove   = Notation::move_to_san(m,ctrl->pBoard);
    mi.whiteMove = ctrl->pBoard->sideToMove() == WHITE;
    mi.respMove  = true;
    ctrl->gameInfo.addMoveInfo(mi);
    ctrl->pBoard->makemove(m);
    ctrl->pBoard->gen(0);
    ctrl->updateStatus();
}

///////////////////////
// CmdAnalyze
const char* Controller::CmdAnalyze::describtion()
{
    return "Analyzes current position.";
}

void Controller::CmdAnalyze::execute(string &arg) throw (BadCmdException)
{
    int iter_depth;
    long epd_time;
    ctrl->pBoard->gen(0);
    ctrl->pSearch->go(36000000, iter_depth, false, 500, 0, epd_time, false);
    ctrl->pBoard->gen(0);
}


///////////////////////
// CmdPonder
const char* Controller::CmdPonder::describtion()
{
    return "Ponder on current position.";
}

void Controller::CmdPonder::execute(string &arg) throw (BadCmdException)
{
    int iter_depth;
    long epd_time;
    //ctrl->pBoard->gen(0);
    ctrl->pSearch->go(36000000, iter_depth, true, 500, 0, epd_time, false);
    ctrl->pBoard->gen(0);
}





///////////////////////
// CmdSetTime
const char* Controller::CmdSetTime::describtion()
{
    return "Set search time: st <time_in_secs>.";
}

void Controller::CmdSetTime::execute(string &arg) throw (BadCmdException)
{

    // Parameter lesen

    int secs = atoi(arg.c_str());


    if (secs <= 0)
        throw BadCmdException("Illegal value for time.", BadCmdException::ILLEGAL_ARGUMENT_SYNTAX);

    ctrl->time_per_control = secs*1000;
    ctrl->time_mode = Controller::FIXED_ST;
}



/////////////////////////////////////////////////////
// Definitionen für Klasse Controller ///////////////

// Einzige Instanz der Klasse:
Controller* Controller::p_instance = NULL;


// Konstruktor
Controller::Controller()
{
    cout << "#Initializing controller ... " << endl;
    
    // Initialwert für status
    status = ST_WAITING_FOR_NEW_GAME;

    // Kein Spiel aus einer PGN-Datenbank geladen
    pgn_game_loaded = false;

    initSettings();

    pBoard = Board::getHandle();
    pSearch = new Search(pBoard);
    pgnParser = PGN_Parser::getHandle();

    // Zufallsgenerator initialisieren
    srand((unsigned) time(0));

    // Resp Endspiel Datenbank Files laden
    // TODO: überhaupt noch abschaltbar?
    // if (RespOptions::getHandle()->getValueAsBool("engine.use_kedb"))
    KEDB::getHandle()->load_db();

    cout << "#Controller ready." << endl;
}

void Controller::builtCommandTable()
{
    Command* cmdList[] = {
        new CmdQuit(), 
        new CmdNewGame(), 
        new CmdHelp(),
        new CmdDump(), 
        new CmdShowFEN(),
        new CmdDebugCommand(), 
        new CmdMove(), 
        new CmdTakeback(), 
        new CmdSanMove(),
        new CmdOpenPGNDatabase(), 
        new CmdNextGameFromPGNDB(),
        new CmdScanPGNDatabase(), 
        new CmdPGNMove(),
        new CmdLoadFEN(), 
        new CmdInfo(),
        new CmdPerft(), 
        new CmdCreateBook(), 
        new CmdLoadBook(),
        new CmdLevel(), 
        new CmdSearch(), 
        new CmdSetTime(),
        new CmdTime(), 
        new CmdRunEPD(),
        new CmdSetSearchOptions(),
        new CmdTestMO(),
        new CmdAnalyze(),
        new CmdScore(),
        new CmdPonder(),
        new CmdCreateKEDB(),
        new CmdResult(),
        new CmdWritePgn(),
        new CmdName(),
        new CmdGetOptions(),
        new CmdSetOption(),
        NULL
    };

    int i=0;
    while (cmdList[i])
    {
        for (unsigned int j = 0; j < cmdList[i]->cmdStrings.size(); j++)
            cmdTable[cmdList[i]->cmdStrings[j]] = cmdList[i];
        i++;
    }

}

void Controller::initSettings()
{
    // Standard Buch laden

    RespOptions* pRO = RespOptions::getHandle();

    use_book = pRO->getValueAsBool("book.use");

    if (use_book)
    {
        string bookfile = pRO->getValue("dir.book");
        bookfile += pRO->getValue("book.name");
        if (!book.setBookFile(bookfile))
        {
            cout << "Error" << endl;
            cout << "Could not load opening book " << bookfile << endl;
            use_book = false;
        }
        else
        {
            cout << "Using opening book " << bookfile << "." << endl;
            use_book = true;
        }
    }

    // Zeitkontrollen

    time_per_control  = 300000;     // 5 min
    moves_per_control = 40;         // 40 Züge
    time_mode         = CONV_CLOCK;

    // Resign
    cut_resign          = -600;
    rep_resign_count    = 0;
    rep_until_resign    = 3;

    // GameInfo
    gameInfo.init();
    gameInfo.setHeader("TimeControl","40/300");
}


Controller::~Controller()
{
    delete pSearch;
}

bool Controller::command(string cmdString) throw(BadCmdException)
{
    Command* cmd;
    string arg;

    if (cmdString == "")
        return true;
    
    if (!parse(cmdString, cmd, arg))
        throw BadCmdException("Unknown Command", 
            BadCmdException::NOT_IDENTIFIED);
    
    // test...
    //cout << "OK ... [" << cmd->describtion() << "]" <<endl;
    cmd->execute(arg);
    
    return true;
}


bool Controller::parse(string cmdString, Command*& cmd, string& arg)
{
    string s = StringTools::trim(cmdString);
    
    
    if (s.length() == 0)
        return false;
    
    string scmd;

    StringTools::splitToOpArg(s, scmd, arg);

    
    // test...
    //cout << "COMMAND: " << scmd << ", ARGUMENT: " << arg << endl;
    
    tableIterator it = cmdTable.find(scmd);
    
    if (it == cmdTable.end())
        return false;
    
    cmd = (*it).second;
    
    return true;
}

void Controller::updateStatus()
{
    // status aktualisieren
    if (pBoard->sideToMove() == WHITE)
        status |= ST_WHITE_TO_MOVE;
    else
        status &= ~ST_WHITE_TO_MOVE;

    // Matt, Patt oder Schach?
    bool check,mate,remis;
    RemisEnum rtype;
    pBoard->getGameStatus(0, check,mate,remis,rtype);

    if (mate || remis)
    {
        status |= ST_GAME_OVER;
        if (pBoard->sideToMove() == WHITE)
            status &= ~ST_WHITE_WON;
        else
            status |= ST_WHITE_WON;

        if (remis)
            status |= ST_REMIS;
    }
    if (check)
        status |= ST_CHECK;
    else
        status &= ~ST_CHECK;
}

// ----------------------------------------------------------------------------
//  Möglichen Zug aus der Eröffnungsdatenbank zufällig auswählen
// ----------------------------------------------------------------------------

Move Controller::randomBookMove(bool use_weights, double& perc) 
{
    const MoveStack& ms = pBoard->moveStack[0];
    vector<Move> m_list;
    vector<int>  count;

    int total_count = 0;

    for (int i = 0; i < ms.size(); i++)
    {
        pBoard->makemove(ms.stack[i]);
        if (pBoard->kingAttacked(XSIDE(pBoard->sideToMove())))
        {
            pBoard->takebackmove();
            continue;
        }
        BookRecord bkrec(pBoard->get_z1_hash(), pBoard->get_z2_hash(), pBoard->sideToMove());
        if (book.findPosition(bkrec))
        {
            m_list.push_back(ms.stack[i]);
            count.push_back(bkrec.get_count());
            total_count += bkrec.get_count();
        }

        pBoard->takebackmove();
    }

    if (m_list.size() == 0) {
        perc = 0.0;
        return 0;           // Nichts gefunden?
    }

    if (!use_weights)
    {
        // Wahrscheinlichkeit berechnen
        perc = 1.0/m_list.size();

        // Zug zufällig aus der Liste der gefundenen Züge der Eröffnungsdb
        // auswählen; alle Züge sind gleich wahrscheinlich 
        int idx = rand() % m_list.size();
        return m_list[idx];
    }
    else
    {
        // Auswahlwahrscheinlichkeit für einen Zug entspricht der relativen
        // Häufigkeit des Zuges in der DB
        int idx = rand() % total_count;

        unsigned int i; 
        for (i = 0; i < count.size(); i++)
        {
            if (idx < count[i])
                break;
            idx -= count[i];
        }

        if  (i >= count.size())
        {
            out << "ERROR IN randomBookMove (controller.cpp)!" << endl;
            return 0;
        }

        // Wahrscheinlichkeit berechnen
        perc = static_cast<double>(count[i])/total_count;
        
        return m_list[i];
    }
}



// ----------------------------------------------------------------------------
//  Waren die letzten Scores so schlecht, dass das Spiel aufgegeben werden
//  kann?
// ----------------------------------------------------------------------------
void Controller::update_resign(int sc)
{
    if (!RespOptions::getHandle()->getValueAsBool("game.can_resign"))
        return;

    if (sc <= cut_resign)
    {
        rep_resign_count++;

        if (rep_resign_count >= rep_until_resign)
        {
            // Aufgeben... :(
            status = ST_RESIGN | ST_GAME_OVER;
        }
    }
    else
    {
        rep_resign_count = 0;
    }
}


// ----------------------------------------------------------------------------
//  Ausgabe des Game-Status
// ----------------------------------------------------------------------------
void Controller::dumpGameStatus()
{
    if (status & ST_WAITING_FOR_NEW_GAME)
        cout << "WAIT_GAME ";
    if (status & ST_WHITE_TO_MOVE)
        cout << "WTM ";
    if (status & ST_GAME_OVER)
        cout << "GAMOV ";
    if (status & ST_REMIS)
        cout << "REM ";
    if (status & ST_WHITE_WON)
        cout << "WWON ";
    if (status & ST_CANCELED)
        cout << "CNCLD ";
    if (status & ST_WAITING_FOR_MOVE)
        cout << "WAIT_MV ";
    if (status & ST_CHECK)
        cout << "CHECK ";
    if (status & ST_RESIGN)
        cout << "RESIGN ";
}

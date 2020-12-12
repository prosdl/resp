// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : xboard.h
//                       Header für xboard.cpp.
//
//  Hinzugefügt        : So, 5.August, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: xboard.h,v 1.18 2003/05/19 20:17:33 rosendahl Exp $
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

#ifndef XBOARD_H
#define XBOARD_H

#include "controller.h"


// -------------------------------------------------------------------------
//            c l a s s     X B o a r d I n t e r f a c e
// -------------------------------------------------------------------------

class XBoardInterface
{
private:
    // ---------------------------------------------------------------------
    //              XBoardStatus
    // ---------------------------------------------------------------------
    enum XBoardStatus {
        RUNNING,
        WAITING_FOR_ENGINE_MOVE,
        GAME_OVER,
        FORCE,
        ANALYZE,
        START_OF_NEWGAME,       // nach 'new', bis 'usermove' oder 'go'
        QUIT
    };

private:
    // ---------------------------------------------------------------------
    //              ABSTRAKTE KLASSE: XBoardCommand
    // ---------------------------------------------------------------------
    friend class XBoardCommand;
    class XBoardCommand
    {
    protected:
        XBoardInterface* xbi;
    public:
        XBoardCommand() : xbi(getHandle()) { }
        virtual ~XBoardCommand() {}
        virtual void execute(const char* arg) = 0;
        const char* syntax;
    };

    // ---------------------------------------------------------------------
    //  AB HIER: DIE EINZELNEN INDIVIDUELLEN XBOARD BEFEHLE, DIE VON XBOARD
    //           AN DIE ENGINE GESCHICKT WERDEN.
    // ---------------------------------------------------------------------

    // ---------------------------------------------------------------------
    //                          *** xboard ***
    // ---------------------------------------------------------------------
    class XB_xboard : public XBoardCommand
    {
    public:
        XB_xboard()  { syntax = "xboard"; };
        void execute(const char* arg);
    };
    friend class XB_xboard;

    // ---------------------------------------------------------------------
    //                          *** accepted ***
    // ---------------------------------------------------------------------
    class XB_accepted : public XBoardCommand
    {
    public:
        XB_accepted()  { syntax = "accepted"; };
        void execute(const char* arg);
    };
    friend class XB_accepted;

    // ---------------------------------------------------------------------
    //                          *** name ***
    // ---------------------------------------------------------------------
    class XB_name : public XBoardCommand
    {
    public:
        XB_name()  { syntax = "name"; };
        void execute(const char* arg);
    };
    friend class XB_name;

    // ---------------------------------------------------------------------
    //                          *** result ***
    // ---------------------------------------------------------------------
    class XB_result : public XBoardCommand
    {
    public:
        XB_result()  { syntax = "result"; };
        void execute(const char* arg);
    };
    friend class XB_result;

    // ---------------------------------------------------------------------
    //                          *** random ***
    // ---------------------------------------------------------------------
    class XB_random : public XBoardCommand
    {
    public:
        XB_random()  { syntax = "random"; };
        void execute(const char* arg);
    };
    friend class XB_random;

    // ---------------------------------------------------------------------
    //                          *** easy ***
    // ---------------------------------------------------------------------
    class XB_easy : public XBoardCommand
    {
    public:
        XB_easy()  { syntax = "easy"; };
        void execute(const char* arg);
    };
    friend class XB_easy;

    // ---------------------------------------------------------------------
    //                          *** hard ***
    // ---------------------------------------------------------------------
    class XB_hard : public XBoardCommand
    {
    public:
        XB_hard()  { syntax = "hard"; };
        void execute(const char* arg);
    };
    friend class XB_hard;

    // ---------------------------------------------------------------------
    //                          *** post ***
    // ---------------------------------------------------------------------
    class XB_post : public XBoardCommand
    {
    public:
        XB_post()  { syntax = "post"; };
        void execute(const char* arg);
    };
    friend class XB_post;

    // ---------------------------------------------------------------------
    //                          *** nopost ***
    // ---------------------------------------------------------------------
    class XB_nopost : public XBoardCommand
    {
    public:
        XB_nopost()  { syntax = "nopost"; };
        void execute(const char* arg);
    };
    friend class XB_nopost;

    // ---------------------------------------------------------------------
    //                          *** usermove ***
    // ---------------------------------------------------------------------
    class XB_usermove : public XBoardCommand
    {
    public:
        XB_usermove() { syntax = "usermove"; }
        void execute(const char* arg);
    };
    friend class XB_usermove;

    // ---------------------------------------------------------------------
    //                          *** new ***
    // ---------------------------------------------------------------------
    class XB_new : public XBoardCommand
    {
    public:
        XB_new() { syntax = "new"; }
        void execute(const char* arg);
    };
    friend class XB_new;


    // ---------------------------------------------------------------------
    //                          *** quit ***
    // ---------------------------------------------------------------------
    class XB_quit : public XBoardCommand
    {
    public:
        XB_quit() { syntax = "quit"; }
        void execute(const char* arg);
    };
    friend class XB_quit;

    // ---------------------------------------------------------------------
    //                          *** go ***
    // ---------------------------------------------------------------------
    class XB_go : public XBoardCommand
    {
    public:
        XB_go() { syntax = "go"; }
        void execute(const char* arg);
    };
    friend class XB_go;

    // ---------------------------------------------------------------------
    //                          *** force ***
    // ---------------------------------------------------------------------
    class XB_force : public XBoardCommand
    {
    public:
        XB_force() { syntax = "force"; }
        void execute(const char* arg);
    };
    friend class XB_force;


    // ---------------------------------------------------------------------
    //                          *** playother ***
    // ---------------------------------------------------------------------
    class XB_playother : public XBoardCommand
    {
    public:
        XB_playother() { syntax = "playother"; }
        void execute(const char* arg);
    };
    friend class XB_playother;

    // ---------------------------------------------------------------------
    //                          *** protover ***
    // ---------------------------------------------------------------------
    class XB_protover : public XBoardCommand
    {
    public:
        XB_protover() { syntax = "protover"; }
        void execute(const char* arg);
    };
    friend class XB_protover;

    // ---------------------------------------------------------------------
    //                          *** white ***
    // ---------------------------------------------------------------------
    class XB_white : public XBoardCommand
    {
    public:
        XB_white() { syntax = "white"; }
        void execute(const char* arg);
    };
    friend class XB_white;

    // ---------------------------------------------------------------------
    //                          *** black ***
    // ---------------------------------------------------------------------
    class XB_black : public XBoardCommand
    {
    public:
        XB_black() { syntax = "black"; }
        void execute(const char* arg);
    };
    friend class XB_black;

    // ---------------------------------------------------------------------
    //                          *** st ***
    // ---------------------------------------------------------------------
    class XB_st : public XBoardCommand
    {
    public:
        XB_st() { syntax = "st"; }
        void execute(const char* arg);
    };
    friend class XB_st;

    // ---------------------------------------------------------------------
    //                          *** level ***
    // ---------------------------------------------------------------------
    class XB_level : public XBoardCommand
    {
    public:
        XB_level() { syntax = "level"; }
        void execute(const char* arg);
    };
    friend class XB_level;

    // ---------------------------------------------------------------------
    //                          *** time ***
    // ---------------------------------------------------------------------
    class XB_time : public XBoardCommand
    {
    public:
        XB_time() { syntax = "time"; }
        void execute(const char* arg);
    };
    friend class XB_time;

    // ---------------------------------------------------------------------
    //                          *** otime ***
    // ---------------------------------------------------------------------
    class XB_otime : public XBoardCommand
    {
    public:
        XB_otime() { syntax = "otim"; }
        void execute(const char* arg);
    };
    friend class XB_otime;

    // ---------------------------------------------------------------------
    //                          *** setboard ***
    // ---------------------------------------------------------------------
    class XB_setboard : public XBoardCommand
    {
    public:
        XB_setboard() { syntax = "setboard"; }
        void execute(const char* arg);
    };
    friend class XB_setboard;

    // ---------------------------------------------------------------------
    //                          *** analyze ***
    // ---------------------------------------------------------------------
    class XB_analyze : public XBoardCommand
    {
    public:
        XB_analyze() { syntax = "analyze"; }
        void execute(const char* arg);
    };
    friend class XB_analyze;

    // ---------------------------------------------------------------------
    //                          *** undo ***
    // ---------------------------------------------------------------------
    class XB_undo : public XBoardCommand
    {
    public:
        XB_undo() { syntax = "undo"; }
        void execute(const char* arg);
    };
    friend class XB_undo;

    // ---------------------------------------------------------------------
    //                          *** exit ***
    // ---------------------------------------------------------------------
    class XB_exit : public XBoardCommand
    {
    public:
        XB_exit() { syntax = "exit"; }
        void execute(const char* arg);
    };
    friend class XB_exit;

    // ---------------------------------------------------------------------
    //                          *** hint ***
    // ---------------------------------------------------------------------
    class XB_hint : public XBoardCommand
    {
    public:
        XB_hint() { syntax = "hint"; }
        void execute(const char* arg);
    };
    friend class XB_hint;

private:
    // ---------------------------------------------------------------------
    //              Private Eigenschaften
    // ---------------------------------------------------------------------

    static XBoardInterface snglInstance;

    std::map<std::string,XBoardCommand*> cmdTable;

    XBoardStatus status;

    ColorEnum compColor;    // Farbe, die der Computer spielt


private:
    // ---------------------------------------------------------------------
    //              Private Methoden
    // ---------------------------------------------------------------------

    XBoardInterface();
    void init();
    void builtCmdTable();
    bool isMove(std::string s);
    XBoardCommand* parseCmd(const std::string& cmd, std::string& arg);
    void checkStatus();

public:
    // ---------------------------------------------------------------------
    //              Öffentliche Methoden
    // ---------------------------------------------------------------------

    static XBoardInterface* getHandle() { return &snglInstance; }

    void run();
};


#endif // XBOARD_H

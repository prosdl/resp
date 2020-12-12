// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : defines.h
//                       Compiler-Flaggen
//
//  Anfang des Projekts: Mi 17.Oktober, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: defines.h,v 1.31 2002/06/11 21:00:49 rosendahl Exp $
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

#ifndef DEFINES_H
#define DEFINES_H

// -------------------------------------------------------------------------
//  Plattform
// -------------------------------------------------------------------------
//#define LINUX

// -------------------------------------------------------------------------
//  Assembler Optimierungen
// -------------------------------------------------------------------------
// MMX - Assembler Routinen
#define USE_MMXASM

// ATHLON spezifischer Code
//#define ATHLON

// MSB-LSB Routine als Assembler-Code
#define USE_ASM_MSBLSB


// -------------------------------------------------------------------------
//  Hash
// -------------------------------------------------------------------------
// Hash-Table
#define USE_TRANSPOSITION_TABLE 

// Pawn Hash
#define USE_PAWNHASH 

// Statistik für Hash
//#define DO_STATISTICS 

// Statistik für PawnHash
//#define PHASH_STATISTIC



// -------------------------------------------------------------------------
//  Debug
// -------------------------------------------------------------------------

// Evaluierungsfunktion debuggen
//#define DEBUG_EVAL

// Prüfen, ob Lazy Evaluation gerechtfertigt
//#define DEBUG_LAZYEVAL

// Evaluierungsfunktion für Bauern debuggen
//#define DEBUG_EVALPAWNS

// Fortlaufende Berechnung der Hashkeys in Board prüfen
//#define DEBUG_HASHKEY_CALC

// Pawn Hash debuggen
//#define DEBUG_PHASH

// Hash debuggen
//#define DEBUG_HASH

// Suchroutinen debuggen: Output nach trace.log
//#define TRACE_SEARCH

// SEE debuggen
//#define DEBUG_SEE
//#define DEBUG_VERIFY_SEE


// Zug-Generierung für Captures immer verwenden
// (speziell für perft!) 
//#define DEBUG_GENCAPS


// Ausführliches Testen von makemove takebackmove
//#define DEBUG_MOVEGEN


#endif // DEFINES_H

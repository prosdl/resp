// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : book.h
//                       Eröffnungsbuch
//
//  Anfang des Projekts: Mi 20.Oktober, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
//
//  Änderungen         
//                -  18.11.01 - Häufigkeiten für Positionen in 
//                   BookRecord mit aufnehmen
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: book.h,v 1.10 2002/06/11 19:04:44 rosendahl Exp $
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

#ifndef BOOK_H
#define BOOK_H

#include "hash.h"
#include "move.h"
#include <string>
#include <vector>
#include <fstream>


// -------------------------------------------------------------------------
//                      c l a s s  B o o k R e c o r d
// -------------------------------------------------------------------------
class BookRecord
{
public:
    BookRecord() : z1(0), z2(0),info(0) { }
    BookRecord(hash_t _z1, hash_t _z2, int side) : z1(_z1), z2(_z2), info(0)
    { 

        set_count(1);       // 1 Satz
        set_side(side);     // Farbe zuweisen
    }

    BookRecord(const BookRecord& br) : z1(br.z1), z2(br.z2),info(br.info) {}

    BookRecord& operator=(const BookRecord& br)
    {
        z1 = br.z1;
        z2 = br.z2;
        info = br.info;
        return *this;
    }

    bool operator< (const BookRecord& br) const
    {
        if (z1 < br.z1) return true;
        if (z1 > br.z1) return false;

        // ---> z1 == br.z1
        if (z2 < br.z2) return true;
        if (z2 > br.z2) return false;

        // ---> z1 == br.z1 und z2 == br.z2

        if (get_side() < br.get_side()) return true;

        return false;
    }

    bool operator== (const BookRecord& br)
    {
        if (z1 == br.z1 && z2 == br.z2 && get_side()==br.get_side()) return true;
        return false;
    }

private:
    hash_t     z1,z2;      // Hashwert Position
    UINTEGER32 info;       // Informationen zur Position:
                           //   xxxx xxxx xxxx xxxs cccc cccc cccc cccc
                           //   mit:
                           //        s  -   am Zug befindliche Seite (0 == Weiss)
                           //        c  -   count; Wie oft kommt Position im Eröffn.buch
                           //               vor
public:
    void set_z1(hash_t z1) { this->z1 = z1; }
    void set_z2(hash_t z2) { this->z2 = z2; }
    void set_side(int s) 
    { 
        if (s == BLACK)
            info |=  0x10000; 
        else
            info &= ~0x10000;
    }
    void set_count(int c)
    {
        info = (info & 0xffff0000) | (c & 0xffff);
    }
    void inc_count()
    {
        int c = info & 0xffff;
        c++;
        if (c > 0xffff) // Überlauf?
            c = 0xffff;
        info = (info & 0xffff0000) | c;
    }

    hash_t get_z1() const { return z1; }
    hash_t get_z2() const { return z2; }
    int get_side() const 
    { 
        if (info & 0x10000)
            return BLACK;
        else
            return WHITE;
    }

    int get_count() const
    {
        return info & 0xffff;
    }

    friend  std::ostream& operator<<(std::ostream& os, const BookRecord& brec);
};

inline std::ostream& operator << (std::ostream& os, const BookRecord& brec)
{
   os << "(" << (long) brec.z1 << ", " << (long) brec.z2 << ")";

   return os;
}


// -------------------------------------------------------------------------
//                      c l a s s  B o o k
// -------------------------------------------------------------------------
class Book
{
private:
    std::ifstream f_book;

public:
    Book() {}

    ~Book();

private:
    bool Book::binSearch(BookRecord& brec, int l, int r);


public:
    // ---------------------------------------
    //          Öffentliche Methoden
    // ---------------------------------------

    // createBook...
    //  createBook erstellt eine Eröffnungsdatenbank mit Namen 'filename'
    //  aus den in 'opening_data' gespeicherten Daten; ein Datensatz vom
    //  Typ BookRecord enthält dabei den Hashkey einer Schachposition,
    //  die in einer Eröffnung aufgetreten ist. Die Reihenfolge in der die
    //  Sätze in 'opening_data' gespeichert sind, spielt dabei keine Rolle.
    static bool createBook(const std::string& filename, std::vector<BookRecord>& opening_data);


    // Datei mit Eröffnungs-DB setzen
    bool setBookFile(const std::string& filename);

    // Position mit Daten 'brec' in Eröffnungsdatenbank finden
    bool findPosition(BookRecord& brec);
};


#endif // BOOK_H

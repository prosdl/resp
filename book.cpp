// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : book.cpp
//                       Eröffnungsbuch
//
//  Anfang des Projekts: Mi 20.Oktober, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: book.cpp,v 1.9 2002/06/11 19:04:44 rosendahl Exp $
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

#include "book.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

// -------------------------------------------------------------------------
//  Destruktor
// -------------------------------------------------------------------------
Book::~Book()
{
    if (f_book.is_open())
        f_book.close();
}

// -------------------------------------------------------------------------
//  Eröffnungsbuch kreieren
// -------------------------------------------------------------------------
bool Book::createBook(const string& filename, vector<BookRecord>& in_opening_data)
{
    cout << endl;
    cout << "Creating book: " << filename << endl;

    // 1.Schritt: Daten sortieren
    cout << "Sorting data ... ";
    sort(in_opening_data.begin(), in_opening_data.end());
    cout << "OK" << endl;

    // 2.Schritt: Duplikate entfernen; die Häufigkeit einer Position 
    //            wird mit in den BookRecord abgespeichert

    cout << "Counting duplicates ... ";
    vector<BookRecord>::iterator z;

    vector<BookRecord> opening_data;
    BookRecord current;
    bool first_time = true;
    for (z = in_opening_data.begin(); z != in_opening_data.end(); z++)
    {
        if ( *z == current )
            current.inc_count();
        else
        {
            // Neuer Satz
            if (!first_time)
            {
                opening_data.push_back(current);
            }
            first_time = false;
            current = *z;
        }
    }
    if (!first_time)
        opening_data.push_back(current);    // letzten Satz speichern

    cout << "OK" << endl;

    
    // 2.Schritt: Daten speichern
    cout << "Writing data ... ";
    ofstream fout(filename.c_str(), ios::out | ios::binary);

    if (! fout.is_open())
    {
        cout << endl;
        cout << "ERROR in book.cpp" << endl;
        cout << "Couldn't open " << filename << endl;
        return false;
    }

    vector<BookRecord>::iterator it = opening_data.begin();

    for (; it != opening_data.end(); it++)
    {
        fout.write(reinterpret_cast<char*> (& *it), sizeof(BookRecord));
    }
        
    fout.close();

    cout << "OK" << endl;
    cout << endl;


    return true;
}


// -------------------------------------------------------------------------
//  Datei zuordnen
// -------------------------------------------------------------------------

bool Book::setBookFile(const std::string& filename)
{
    if (f_book.is_open())
        f_book.close();

    f_book.clear();

    f_book.open(filename.c_str(),ios::binary | ios::in);

    if (!f_book.is_open())
        return false;


    return true;
}


// -------------------------------------------------------------------------
//  Position in DB finden
// -------------------------------------------------------------------------

bool Book::findPosition(BookRecord& brec)
{
    if (!f_book.is_open())
        return false;

    f_book.clear();
    // Index des letzten Datensatzes bestimmen
    f_book.seekg(0,ios::end);
    int last = f_book.tellg() / sizeof(BookRecord);

    return binSearch(brec,0,last-1);

}

// -------------------------------------------------------------------------
//  Rekursive binäre Suche in f_book
// -------------------------------------------------------------------------
bool Book::binSearch(BookRecord& brec, int l, int r)
{
    if (l > r)
        return false;   // Nichts gefunden

    BookRecord brec_current;
    int m = (r + l) / 2;

    f_book.seekg(m*sizeof(BookRecord));

    f_book.read(reinterpret_cast<char*> (&brec_current), sizeof(BookRecord));

    if (brec_current < brec)
        return binSearch(brec,m+1,r);

    if (brec < brec_current)
        return binSearch(brec,l,m-1);

    // Datensatz gefunden: Häufigkeit nach brec speichern
    brec.set_count(brec_current.get_count());

    return true;
}

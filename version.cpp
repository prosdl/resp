// --------------------------------------------------------------------------
//
// File   : version.cpp
// Version: $Id: version.cpp,v 1.21 2003/06/02 18:12:53 rosendahl Exp $
// Author : $Author: rosendahl $
//
// Copyright (C) 2003 Peter Rosendahl
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
// --------------------------------------------------------------------------

#include "version.h"

char* Version::major_version = "0";
char* Version::minor_version = "21";
char* Version::build_number = "130";
char* Version::build_date   = __DATE__;
char* Version::build_time   = __TIME__;
char* Version::cvs_id       = "$Id: version.cpp,v 1.21 2003/06/02 18:12:53 rosendahl Exp $";


// --------------------------------------------------------------------------
//
// File   : version.h
// Version: $Id: version.h,v 1.2 2003/05/19 20:17:33 rosendahl Exp $
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

#if !defined VERSION_H
#define VERSION_H

#include <string>


/**
 *  Created : Fre Mai 16 22:08:04 CEST 2003
 *  
 *  @author peter
 *  @version $Id: version.h,v 1.2 2003/05/19 20:17:33 rosendahl Exp $
 */

class Version {
public:
   static char* major_version;
   static char* minor_version;
   static char* build_number;
   static char* build_date;
   static char* build_time;
   static char* cvs_id;

   static std::string full_version() {
      return "Resp-" + std::string(major_version) + "." +
             std::string(minor_version) + "." +
             std::string(build_number);
   }
};

#endif // VERSION_H


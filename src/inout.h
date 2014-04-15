/*******************************************************************************

Copyright (C) 1997-2009 Thomas Christof and Andreas Loebel
 
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.
 
This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
 

FILENAME: inout.h

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Tue Aug 13 16:09:37 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: inout.h,v 1.2 2009/09/21 07:05:11 bzfloebe Exp $ */


#ifndef _INOUT_H
#define _INOUT_H


#include "porta.h"


extern int scan_line2( int, char [], char *, char [] );
extern int read_input_file( char *, FILE *, int *, RAT **, int *, char *, int **, char *,
                            int **, char *, RAT ** );
extern void read_eqie( RAT **, int, int *, int *, int *, int *, char *, char [], char * );
extern void write_ieq_file( char *, FILE *, int, int, int, int *, 
                            int, int, int, int * );
extern void write_poi_file( char *, FILE *, int, int, int, int, int, int, int );
extern void writepoionie( FILE *, int, int, int, int );
extern void writesys( FILE *, int, int, int, int, int *, char, int * );
extern void writemark( FILE *, unsigned *, int, int * );
extern void writestatline( FILE *, int * );
extern FILE *wfopen( char * );
extern void I_RAT_writeline( FILE *, int, RAT *, int, RAT *, char, int * );


#endif // _INOUT_H

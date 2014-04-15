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
 

FILENAME: log.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Tue Aug 13 16:10:23 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: log.c,v 1.2 2009/09/21 07:05:11 bzfloebe Exp $ */


#include "log.h"


int porta_log( char *fmt, ... )
{
    va_list argp;
    int ret;
        
    if( logfile )
    {
        va_start( argp, fmt );
        ret = vfprintf( logfile, fmt, argp );
        va_end( argp );
        return ret;
    }
    else
        return 0;
}

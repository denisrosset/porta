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
 

FILENAME: largecalc.h

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Tue Aug 13 16:10:10 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: largecalc.h,v 1.2 2009/09/21 07:05:11 bzfloebe Exp $ */


#ifndef _LARGECALC_H
#define _LARGECALC_H


#include "porta.h"


extern void lsub( loint, loint, loint * );
extern void lsubber( unsigned *, unsigned *, unsigned *, int, int, int * );
extern void ladd( loint, loint, loint * );
extern void ladder( unsigned *, unsigned *, unsigned *, int, int, int * );
extern void lmul( loint, loint, loint * );
extern void lmuller( unsigned *, unsigned *, unsigned *, int, int, int * );
extern void porta_ldiv( loint, loint, loint *, loint * );
extern int lorder( unsigned *, unsigned *, int, int );
extern void lgcd( loint, loint, loint * );


#endif // _LARGECALC_H

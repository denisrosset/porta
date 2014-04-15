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
 

FILENAME: portsort.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Fri Sep 20 14:26:37 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: portsort.c,v 1.3 2009/09/21 07:05:11 bzfloebe Exp $ */


#include "portsort.h"
#include "mp.h"
#include "log.h"


int comp,delay,same_vals=0,rowlen;
int (*syscompare)(const void*,const void*);







int int_syscompare( listp *i, listp *j )
{
    int ret_code = 0;
    
    if( ((*i)->sys+comp)->num > ((*j)->sys+comp)->num )
        ret_code = 1;
    else if( ((*i)->sys+comp)->num < ((*j)->sys+comp)->num )
        ret_code = -1; 
    else
        ret_code = 0;

    return ret_code;


    // old and buggy ...
    // return(((*i)->sys+comp)->num - ((*j)->sys+comp)->num > 0);
}



int rat_syscompare( listp *i, listp *j )
{
    (*RAT_sub)(*((*i)->sys+comp),*((*j)->sys+comp),var);
    return( var[0].num );


    // maybe buggy, but I am not sure ...
    // return(var[0].num > 0);
}



int ptrcompare( listp *i, listp *j )
{
    int ret_code = 0;
    int *statptr1,*statptr2;
    
    statptr1 = (int *) (*i)->ptr;
    statptr2 = (int *) (*j)->ptr;

    if( *(statptr1+comp) > *(statptr2+comp) )
        ret_code = 1;
    else if( *(statptr1+comp) < *(statptr2+comp) )
        ret_code = -1; 
    else
        ret_code = 0;

    return ret_code;


    // old and maybe buggy ...
    // return(*(statptr1+comp) - *(statptr2+comp) > 0);
}



void sortrekurs( int first, int last, int whatcomp )
{
    int i,compint,*statptr1,*statptr2;
    
    if (whatcomp == 0 || whatcomp > 10) 
    {
        
        if (whatcomp == 0)  
        {
            comp = rowlen-1;
        }
        else 
        {
            comp = whatcomp - 11;
        }
        
        if (whatcomp == 11) 
        {
            same_vals++; 
            for (i = first; i <= last; i++)
                *((int *) porta_list[i]->ptr) = same_vals;
        }
        
        compint = comp;
        qsort( &(porta_list[first]), last -first +1, sizeof(porta_list[0]), 
              (int(*)(const void*,const void*))syscompare );
        
        for (; first < last; first = i+1)
        {
            (*RAT_sub)(*(porta_list[first]->sys+compint),
                       *(porta_list[first+1]->sys+compint),var+1);
            for (i = first; i < last && var[1].num == 0; i++)
                if (i+1 < last)
                    (*RAT_sub)(*(porta_list[i+1]->sys+compint),
                               *(porta_list[i+2]->sys+compint),var+1);
            if (i != first && whatcomp < rowlen+10)
                sortrekurs(first,i,whatcomp+1);
        }
    }
    else  
    {
        
        compint = comp = whatcomp-6;
        if (comp >= 0)
            compint = ++comp;
        
        qsort(CP (porta_list +first),last-first+1,sizeof(porta_list[0]),
              (int(*)(const void*,const void*))ptrcompare);
        
        for (; first < last; first = i+1) 
        {
            statptr1 = (int *) porta_list[first]->ptr;
            statptr2 = (int *) porta_list[first+1]->ptr;
            for (i = first; 
                 i < last && 
                 (*(statptr1+compint) == *(statptr2+compint)); i++) 
            {
                statptr1 = (int *) porta_list[i+1]->ptr;
                if (i+1 < last)
                    statptr2 = (int *) porta_list[i+2]->ptr;
            }  
            if (i != first)
                sortrekurs(first,i,whatcomp+1);
        }
    }
}








void sort( int int_val, int rl, int first, int last)
{
    int i,j,val,*statptr,*statistik;
    
    fprintf(prt,"sorting system ");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "sorting system ");
    
    rowlen = rl;
    syscompare = (int(*)(const void*,const void*))
        (int_val ? int_syscompare : rat_syscompare);
    
    statistik = (int *) allo(CP 0,0,U (last-first+1)*11*sizeof(int));
    for (j = 0; j < (last-first+1)*11; j++)
        *(statistik+j) = 0;
    
    statptr = statistik + 5;
    
    for (i = first; i < last; i++,statptr += 11) 
    {
        
        porta_list[i]->ptr =  (RAT *) statptr;
        
        for ( j = 0; j != rl-1; j++) 
        {
            
            val = ((porta_list[i]->sys+j)->den.i == 1) ? 
                (porta_list[i]->sys+j)->num : -100;
            if ( val < 0 && val > -6)
                *(statptr+val) += 1;
            else if (val > 0 && val < 6 ) 
                *(statptr+val) += 1;
            
        }
    }
    
    sortrekurs(first,last-1,0);
    
    if (MP_realised)
        return_from_mp();

    free( statistik );
    
    fprintf(prt,"\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\n");
}


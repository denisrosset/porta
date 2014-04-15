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
 

FILENAME: common.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Wed Sep 18 17:46:24 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: common.c,v 1.3 2009/09/21 07:05:11 bzfloebe Exp $ */


#include "common.h"
#include "arith.h"
#include "inout.h"







char* RATallo( RAT *ptr, int nold, int nnew )
{  
    int i;
    char* allo();
    
#define get_len(x)  (abs((x)->num) & m0_9)
#define m0_9    1023
    if (ptr && MP_realised && nold > nnew) 
        /* free space in ptr[i].den.p  for i>nold */
        for (i = nold; i < nnew; i++) 
            allo(CP ptr[i].den.p,U get_len(ptr+1)*szU,0);
    
    /* 
     * Allocate ptr using malloc(),     if nnew = 0,
     * free ptr                 if nold = 0,
     * reallocate ptr using realloc(),  if nnew != nold != 0
     *   realloc() copies the old contents of ptr[0..nold] into the new location,
     *   as far as possible.
     */
    
    ptr  = RP allo(CP ptr,nold*sizeof(RAT),nnew*sizeof(RAT));
    
    for (i = nold; i < nnew; i++) 
    {
        ptr[i].num = 0;
        ptr[i].den.i = 1;
    }
    
    return(CP ptr);
}








char* allo( char *ptr, unsigned oldsize, unsigned newsize )
/*****************************************************************/
/*
 * Free ptr, if newsize = 0,
 * Allocate ptr, if newsize > 0, oldsize = 0,
 * Reallocate ptr, if newsize and oldsize are nonzero.
 * Update total_size.
 * But not every ptr allocated with allo() is freed with allo(),
 * so total_size is too high.
 */
{  
    if( newsize == 0 && oldsize == 0 )
        return( ptr );
        
    if (newsize == 0 && ptr) 
    {
        if (oldsize > 0) 
        {
            free(CP ptr);
            ptr = 0;
        }
        total_size -= oldsize;
        return (CP 0);
    }
    
    if (ptr == 0 || oldsize == 0) 
    { 
        /* change malloc to calloc, because some allocated
           bytes are somewere unitialized read, but they
           are assumed to be zero.
           if ((ptr = (char *) malloc(newsize)) == 0)
           */
        if ((ptr = (char *) calloc(newsize,1)) == 0)
            msg( "allocation of new space failed", "", 0 );   
        total_size += newsize;
        return(ptr);
    }
    
    if (oldsize != newsize && (ptr = (char *) realloc(ptr,newsize)) == 0) 
        msg( "reallocation of space failed", "", 0 );   
    total_size += newsize-oldsize;
    
    return(ptr);
}







char andreas[20] = "sonmist";

listp *porta_list;

void allo_list( int new, unsigned **markp, int blocks )
{
    int i,ms;
    
    if (new == maxlist) 
    {
        maxlist += INCR_LIST;
        porta_list = (listp *) allo(CP porta_list,1,U maxlist*sizeof(listp));
        for (i = new; i < maxlist; i++)
                porta_list[i] = 0;
    }
    if (porta_list[new] == 0)  
    { 
        porta_list[new] = (struct list *) allo(CP 0,0,U sizeof(struct list));
        porta_list[new]->mark = 0;
    }
    if (markp)
    {
        if ( porta_list[new]->mark == 0) 
        {
            *markp = porta_list[new]->mark = (unsigned *) 
                allo(CP 0,0,U (ms = blocks*sizeof(unsigned)));
            for (i = 0; i < blocks; i++)
                porta_list[new]->mark[i] = 0;
        }
        else
            *markp = porta_list[new]->mark;
    }
}








void listptoar( RAT *ar, int rows, int rowl, int blocks )
{
    int i;
    unsigned j = 0;
    RAT *arp;
    
    for (i = 0,arp = ar; i < rows; i++,arp += rowl) 
    {
        allo_list(i,(unsigned **)&j,blocks);
        porta_list[i]->sys = arp;
    }
}












void get_options( int *argc, char **argv[] )
{
    char *s;
    
    while (--(*argc)>0 && (*++*argv)[0] == '-')
        for (s = *argv[0]+1; *s != '\0'; s++)
            switch(*s) 
            {
            case 'l' : option |= Long_arithmetic;
                break;
            case 'p' : option |= Protocol_to_file;
                break;
            case 's' : option |= Statistic_of_coefficients;
                break;
            case 'v' : option |= Validity_table_out;
                break;
            case 'c' : option |= Chernikov_rule_off;
                break; 
            case 'o' : option |= Opt_elim;
                break; 
            case 'D' : option |= Dim;
                allowed_options = Dim|Protocol_to_file|Long_arithmetic;
                break; 
            case 'F' : option |= Fmel;
                allowed_options = Fmel|Chernikov_rule_off|Protocol_to_file|Long_arithmetic;
                break; 
            case 'S' : option |= Sort;
                allowed_options = Sort|Statistic_of_coefficients;
                break; 
            case 'T' : option |= Traf;
                allowed_options = Traf|
                    Chernikov_rule_off|Validity_table_out|
                    Redundance_check|Statistic_of_coefficients|
                    Protocol_to_file|Opt_elim|Long_arithmetic;
                break; 
            case 'C' : option |= Cfctp;
                allowed_options = Cfctp;
                break; 
            case 'P' : option |= Posie;
                allowed_options = Posie;
                break; 
            case 'I' : option |= Iespo;
                allowed_options = Iespo|Validity_table_out;
                break; 
            case 'V' : option |= Vint;
                allowed_options = Vint;
                break; 
            default : msg( "invalid command line", "", 0 );
                break;
            }
    if (option & ~allowed_options)
        msg( "invalid command line", "", 0 );
}












void initialize()
{
    cone = conv = equa = ineq = points = blocks = option = nel_ar6
        = nel_ar5 = nel_ar4 = nel_ar2 = nel_ar3 = nel_ar1 = 0;
    ar6 = ar5 = ar4 = ar2 = ar3 = ar1 = 0;
    
    RAT_const[0].num = 0;
    RAT_const[1].num = RAT_const[1].den.i = RAT_const[0].den.i = 1;
}







void set_I_functions()
{ 
    RAT_add = I_RAT_add;
    RAT_sub = I_RAT_sub;
    RAT_mul = I_RAT_mul;
    RAT_assign = I_RAT_assign;
    RAT_row_prim = I_RAT_row_prim;
    writeline = (void (*)())I_RAT_writeline;
    
}  








int comp;



int num_den_comp( RAT *a, RAT *b )
{ 
    if ((a+comp)->num == (b+comp)->num)
        return((a+comp)->den.i - (b+comp)->den.i);
    else
        return((a+comp)->num - (b+comp)->num);
} 







void sort_eqie_cvce( RAT *ar, int nrows, int rowlen, int *neqcv, int *niece )
/*****************************************************************/
/*
 * Sort the "nrow" inequalities and equations in ar, 
 * so that equations come first.
 * For i = 1,...,nrow-1, 
 * ar[i-1][rowlen-1] contains 0 (for equation), 1 (for inequality).
 * Output:
 *   neqcv is the number of equations, 
 *   niece the number of inequalities.
 */
{
    int i,equa;
    
    /* ar[i][rowlen-1] contains 0 (for equation), 1 (for inequality). */

    for (i = 1; i <= nrows; i++)
        (ar+i*rowlen-1)->den.i = i;
    comp = rowlen-1;
    qsort(CP ar,nrows,sizeof(RAT)*rowlen,
          (int(*)(const void*,const void*))num_den_comp);
    for (equa = 0; equa < nrows && (ar+rowlen*equa+comp)->num == 0;
         equa++);
    *neqcv = equa;
    *niece = nrows - equa;
    for (i = 1; i <= nrows; i++)
        (ar+i*rowlen-1)->den.i = 1; 
}









void msg( char *s, char *s2, int i )
{

        fprintf(prt,s,s2,i);
        fprintf(prt,"\n");

        /* 17.01.1994: include logging on file porta.log */
        porta_log( s, s2, i );
        porta_log( "\n" );

        exit(1);

}

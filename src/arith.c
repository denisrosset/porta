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
 

FILENAME: arith.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Tue Aug 13 16:06:54 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: arith.c,v 1.2 2009/09/21 07:05:11 bzfloebe Exp $ */


#include "arith.h"
#include "mp.h"




void I_RAT_assign( RAT *a, RAT *b )
{ 

  a->num = b->num;
  a->den.i = b->den.i;
  
}





void I_RAT_add( RAT a, RAT b, RAT *c )
{ 

  int r,x,y1,y2,z1,z2,na,nb;

  if (a.den.i == b.den.i) {

    c->den.i = a.den.i;
    c->num = a.num+b.num;
    if (c->num-b.num != a.num) {
      arith_overflow_func(1,I_RAT_add,a,b,c);
      return;
    }

  }

  else {

    c->den.i = (x = (a.den.i/igcd(a.den.i,b.den.i)))*b.den.i;
    na = (y1 = a.num)*(y2 = (c->den.i/a.den.i));
    nb = (z1 = b.num)*(z2 = (c->den.i/b.den.i));
    c->num = na+nb;

    if ((c->den.i/b.den.i !=  x) || (na/y2 != y1) || (nb/z2 != z1) || (c->num-nb != na)) {
      arith_overflow_func(1,I_RAT_add,a,b,c);
      return;
    }

  }

  if ((r = igcd(c->num,c->den.i)) > 1) {
    c->den.i /= r;
    c->num /= r;
  }

}

      





void I_RAT_sub( RAT a, RAT b, RAT *c )
{ 

  int r,x,y1,y2,z1,z2,an,bn;

  if (a.den.i == b.den.i) {
    c->den.i = a.den.i;
    c->num = a.num-b.num;

    if (c->num+b.num != a.num) {
      arith_overflow_func(1,I_RAT_sub,a,b,c);
      return;
    }

  }

  else {
    c->den.i = (x = (a.den.i/igcd(a.den.i,b.den.i)))*b.den.i;
    an = (y1 = a.num)*(y2 = (c->den.i/a.den.i));
    bn = (z1 = b.num)*(z2 = (c->den.i/b.den.i));
    c->num = an-bn;

    if ((c->den.i/b.den.i !=  x) || (an/y2 != y1) || (bn/z2 != z1) || (c->num+bn != an)) {
      arith_overflow_func(1,I_RAT_sub,a,b,c);
      return;
    }

  }

  if ((r = igcd(c->num,c->den.i)) > 1) {
    c->den.i /= r;
    c->num /= r;
  }

}








void I_RAT_mul( RAT a, RAT b, RAT *c )
{ 

  int r;
  RAT aa,bb;

  aa = a; bb = b;
  if ((r = igcd(a.num,b.den.i)) > 1) {
    b.den.i /= r;
    a.num /= r;
  }
  if ((r = igcd(b.num,a.den.i)) > 1) {
    a.den.i /= r;
    b.num /= r;
  }

  if (a.num == 0)
    c->num = 0;
  else {
    c->num = a.num*b.num;
    if (c->num/a.num != b.num) {
      arith_overflow_func(1,I_RAT_mul,aa,bb,c);
      return;
    }
 
  }
    
  c->den.i = a.den.i*b.den.i;
  if  (c->den.i/a.den.i != b.den.i) 
    arith_overflow_func(1,I_RAT_mul,aa,bb,c);

}









int eqie_satisfied( RAT *iep, RAT *pp, int n, int ie )
{

  void vecpr();  
  int ret;

  vecpr(iep,pp,&var[1],n);
  (*RAT_sub)(*(iep+n),var[1],&var[1]);

  ret = 0;
  if ((ie && var[1].num>0) || (!ie && !var[1].num))
    ret = 1;
  else if (!var[1].num)
    ret = 2;

  return(ret);

}
      








void vecpr( RAT *a, RAT *b, RAT *c, int n )
/*****************************************************************/
/*
 * Compute the product of vectors a and b (both of length n)
 * and store the result in c[0].
 */
{

  (*RAT_assign)(var,RAT_const);     /* var[0] := 0 */
  (*RAT_assign)(c,RAT_const);       /* c[0] := 0 */
  for (; n > 0; n--) {          /* for i = 0 to n-1 */
    (*RAT_mul)(*a++,*b++,var);      /* var[0] := a[i]*b[i] */
    (*RAT_add)(var[0],*c,c);        /* c[0] := var[0]+c[0] */
 }
}









void row_add( RAT *a, RAT *b, RAT *c, int n )
/*****************************************************************/
/*
 * Add the two n-vectors of rationals a and b, and store the result in c.
 */
{
    for( ; n > 0; n-- )
    {
        (*RAT_add)(*a++,*b++,c++);
    }
}








void row_sub( RAT *a, RAT *b, RAT *c, int n )
{
  for( ; n > 0; n-- )
  {
      (*RAT_sub)(*a++,*b++,c++);
  }
}









void scal_mul( RAT *val, RAT *b, RAT *c, int n )
{
  for( ; n > 0; n-- )  
    (*RAT_mul)(*val,*b++,c++);
}














void gauss_calcnewrow( RAT *row1, RAT *row2, int pivcol, RAT *newrow, 
                       int piv_remove, int rowl )
/*****************************************************************/
/*
 * Given the pivot row row1, and another row2, both of length rowl,
 * eliminate variable pivcol in row2 by adding row 1,
 * and store the result in newrow.
 * If piv_remove is 1, the pivot variable is removed and newrow shortened by 1,
 * otherwise, newrow[pivcol] is set to 0.
 */
{
  int i;
  void (*row_calc)();
  
  /* Divide both rows by a positive number,  s.t. row[pivcol] in {0,-1,+1} */
  (*RAT_row_prim)(row1,row1,row1+pivcol,rowl);
  (*RAT_row_prim)(row2,row2,row2+pivcol,rowl);

  if (!row2[pivcol].num) {
    for (i = 0;i < pivcol; i++)
      (*RAT_assign)(newrow+i,row2+i);
    if (piv_remove) {
      row2++; rowl--;
    }
    for (i = pivcol;i < rowl; i++)
      (*RAT_assign)(newrow+i,row2+i);
    return;
  }

  if ( ( row1[pivcol].num < 0 &&  row2[pivcol].num < 0) || 
       ( row1[pivcol].num > 0 &&  row2[pivcol].num > 0) )
    row_calc = row_sub;
  else
    row_calc = row_add;

  (*row_calc)(row2,row1,newrow,pivcol);
  
  if (piv_remove) 
    (*row_calc)(row2+pivcol+1,row1+pivcol+1,newrow+pivcol,rowl-pivcol-1);
  else
    (*row_calc)(row2+pivcol,row1+pivcol,newrow+pivcol,rowl-pivcol);

}











int igcd( int a, int b )
{ 
    int r;
    
    if (a == 0)
        return(b);
    
    if ((a = abs(a)) < (b = abs(b)))
    {
        r = a;
        a = b;
        b = r;
    }
    
    while ((r = a%b) > 0)
    {
        a = b;
        b = r;
    }
    return(b);
}












int gcdrow( int *x, int m )
{

  int im,r;

  while (m > 0) {

    r = x[0]%x[1];
    if (r > 1) {
      x[0] = x[1];
      for(im = 1; im < m && x[im+1] > r; im++)
        x[im] = x[im+1];
      if (m > 1 && im < m && x[im+1] == r) {
        for (;im < m; im++) 
          x[im] = x[im+1];
        m--;
      }
      else
        x[im] = r;
    }
    else if (r == 0) {
      for(im = 0; im < m; im++)
        x[im] = x[im+1];
      m--;
    }
    else return(1);
  }

  return(x[0]);

}














void I_RAT_row_prim( RAT *old, RAT *new, RAT *p, int n )
/*****************************************************************/
/*
 * n is the number of elements in the vector "old".
 * p is some fraction, e.g. the right hand side of the inequality
 *   represented by "old".
 * If p != 0, then "new" = "old"/p.
 * If p == 0, then all numerators of "old" are divided by their gcd,
 *        and all denominators of "old" are divided by their gcd.
 *        The result is stored in "new" (not in "old").
 * Fractions like 0/12 are converted to 0/1.
 */
{
  int *x;
  int m=0,ii,i,j,r;

  x = (int *) allo(CP 0,0,U n*sizeof(int));
  for(i=0; i<n; i++)
     x[i] = 0;

  /* 
   * Divide the elements of list "old" by "|p|", 
   * and store the result in list "new"
   */
  if (p->num != 0) {

    var[0].den.i = abs(p->num);     
    var[0].num = p->den.i ;

    for (i = n; i; i--)
      (*RAT_mul)(*old++,var[0],new++);
  }

  else {

    var[0].num = 0;
    var[0].den.i = 0;

  /* GCD OF NUMERATOR */

    /* 
     * The array "x" contains all absolute values of numerators
     * in strictly increasing order,
     * If the numerator 1 appears, var[0]->num is set to 1.
     * var[0]->num is then computed as the gcd of "x".
     */
    for (j = 0; j < n; j++) {
      r = abs ((old+j)->num);
      if (r == 1) { 
        var[0].num = 1;
        break;
      }
      else if (r  != 0) {
        for (i = 0; i != m && r < x[i]; i++); 
        if (r != x[i] || i == m) {
          for (ii = m; ii > i; ii--) 
            x[ii] = x[ii-1];
          x[i] = r;
          m++;
        }
      }
    }
    m--;
    if (var[0].num != 1) var[0].num = gcdrow(x,m);

   /* GCD OF DENUMERATOR */

    m = 0;
    for (j = 0; j < n; j++) {
      r = ((old+j)->num == 0) ? 0 : (old+j)->den.i;
      if (r == 1) { 
        var[0].den.i = 1;
        break;
      }
      else if (r  != 0) {
        for (i = 0; i != m && r < x[i]; i++); 
        if (r != x[i] || i == m) {
          for (ii = m; ii > i; ii--) 
            x[ii] = x[ii-1];
          x[i] = r;
          m++;
        }
      }
    }
    m--;
    if (var[0].den.i != 1) var[0].den.i = gcdrow(x,m);

    if (var[0].num > 1 || var[0].den.i > 1) 
      for (j = 0; j < n; j++) {
        (new+j)->num = (old+j)->num/var[0].num;
        (new+j)->den.i = ((old+j)->num == 0) ? 1 : ((old+j)->den.i)/var[0].den.i;
    }
    else if (old != new)  
      for (j = 0; j < n; j++) {
        (new+j)->num = (old+j)->num;
        (new+j)->den = (old+j)->den;
      }
  }
  
  x = (int *) allo(CP x,U n*sizeof(int),0);

}






   

  
        

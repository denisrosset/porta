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
 

FILENAME: four_mot.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Mon Nov 25 16:31:19 2002 by Andreas Loebel (opt0)  */
/* $Id: four_mot.c,v 1.5 2009/09/21 07:46:48 bzfloebe Exp $ */


#define _CRT_SECURE_NO_WARNINGS 1


#include "four_mot.h"
#include "arith.h"
#include "common.h"
#include "mp.h"
#include "log.h"


#define INCR_CMB 100000
#define MAX_EL_CMB 5000000

int blocks; /* set in fourier_motzkin, used in red_test */
int itr, totalineq = 0;


char hest1[7],hest2[7],hest3[7];
char oestr1[10],oestr2[10],oestr3[10],oestr4[10];








void print_head_line()
{
#if defined WIN32

    fprintf(prt,"| iter- | %10s |  # ineq  |   max| long|   non- |     mem |      time |%s\n",hest1,oestr1);
    fprintf(prt,"| ation | %10s |          |  bit-|arith|  zeros |    used |      used |%s\n",hest2,oestr2);
    fprintf(prt,"|       | %10s |          |length|metic|   in %c |   in kB |    in sec |%s\n",hest3,'%',oestr3);
    fprintf(prt,"|-------|------------|----------|------|-----|--------|---------|-----------|%s\n",oestr4);

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "| iter- | %10s |  # ineq  |   max| long|   non- |     mem |      time |%s\n",hest1,oestr1);
    porta_log( "| ation | %10s |          |  bit-|arith|  zeros |    used |      used |%s\n",hest2,oestr2);
    porta_log( "|       | %10s |          |length|metic|   in %c |   in kB |    in sec |%s\n",hest3,'%',oestr3);
    porta_log( "|-------|------------|----------|------|-----|--------|---------|-----------|%s\n",oestr4);

#else // WIN32

    fprintf(prt,"| iter- | %10s |  # ineq  |   max| long|   non- |     mem |  cpu-time | real-time |%s\n",hest1,oestr1);
    fprintf(prt,"| ation | %10s |          |  bit-|arith|  zeros |    used |      used |      used |%s\n",hest2,oestr2);
    fprintf(prt,"|       | %10s |          |length|metic|   in %c |   in kB |    in sec |    in sec |%s\n",hest3,'%',oestr3);
    fprintf(prt,"|-------|------------|----------|------|-----|--------|---------|-----------|-----------|%s\n",oestr4);

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "| iter- | %10s |  # ineq  |   max| long|   non- |     mem |  cpu-time | real-time |%s\n",hest1,oestr1);
    porta_log( "| ation | %10s |          |  bit-|arith|  zeros |    used |      used |      used |%s\n",hest2,oestr2);
    porta_log( "|       | %10s |          |length|metic|   in %c |   in kB |    in sec |    in sec |%s\n",hest3,'%',oestr3);
    porta_log( "|-------|------------|----------|------|-----|--------|---------|-----------|-----------|%s\n",oestr4);

#endif // WIN32
}












void gauss( int traf, int sysrow, int eqrl, int dim, int equa_in, 
            int *ineq, int *equa, int indx[] )
/*****************************************************************/
/*
 * Perform Gauss-Elimination 
 * in a system Py+Qx  = b
 *        Fx <= f
 *         -y    <= 0
 * to eliminate all equations and as many y-variables as possible.
 * Typically, Q is -I upon input and b is a 0/1 vector, but that is not used.
 * However, the system is supposed to have full row rank.
 * Output is a system Q'x = b' of equations in x,
 * and a system Ay + Bx <= c of inequalities.
 * Unchanged inequalities -y <= 0 are not output!!
 *
 * Description of input:
 * The equations Py+Qx = b and Fx <= f are stored rowwise in matrix "ar2", 
 * first the coeff of y-variables, then of x-variables, 
 * then the right-hand side.
 * Equations come in front of inequalities.
 * Each row i is also accessible via porta_list[i]->sys.
 * The inequalities -y <= 0 are not part of the input,
 * they are treated implicitly.
 * Dimensions: 
 *   # rows of P   = equa_in,      # rows of F   = ineq_in-equa_in,
 *   # y-variables = sysrow-dim-1, # x-variables = dim.
 * indx[] is an array of variable names (parallel to the columns of ar2).
 * y have negative names starting with -1, 
 * x have nonnegative names starting with 0.
 * "ar2" should be able to contain "dim+1" more rows of length 
 * "sysrow", for temporary storage.
 * "traf" = 1 means that there are -y <= 0 inequalities,
 * otherwise "traf" = 0.
 *
 * Description of output:
 * "ar4" contains "equa" equations Q'x = b',
 * the row length of ar4 is "eqrl",
 * and indx[offset+j] contains the original name of the variable
 * in the j-th column of ar4, for j=0,...,dim.
 * "ar2" contains the "ineq_in-equa" inequalities Ay + Bx <= c, rowwise.
 * In "traf" applications,
 * "ineq_in" = "dim+1", if "conv" = "points"  (i.e. last equation deleted)
 *       = "dim",     otherwise.
 * The 0-columns belonging to the eliminated y-variables are deleted in "ar2",
 * so "ar2"s row length is (together with the right-hand side column) 
 *    sysrow-equa_in.
 * The rows of "ar2" are also accessible via porta_list[i]->sys
 * for i = 0,...,ineq_in-equa-1.
 * "ineq" is not changed, if traf=1.
 */
{ 
    int col,row,pivcol;
    int i,j, ineq_in;
    RAT *sptr,*pivot;
    int ld,nz;


    fprintf(prt,"GAUSS - ELIMINATION:\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "GAUSS - ELIMINATION:\n");


    // sprintf(oestr1,"\0");  sprintf(oestr2,"\0");  sprintf(oestr3,"\0");
    // sprintf(oestr4,"\0");
    oestr1[0] = (char)0;
    oestr2[0] = (char)0;
    oestr3[0] = (char)0;
    oestr4[0] = (char)0;
    strcpy( hest1, "# equa" );
    strcpy( hest2, "      " );
    strcpy( hest3, "      " );
    
    print_head_line();
    
    /* GAUSS - ELIMINATION */
    ineq_in = *ineq;    /* reduced by one for each pivot on a y-variable */
    
    for (itr = equa_in; itr != 0; itr--) 
    {
        pivot = porta_list[0]->sys;
        for(pivcol=0; !pivot->num && pivcol < sysrow-1; pivcol++,pivot++);

        /* If pivcol == sysrow-1, the equation in the first row is 0x = ? */
        if (pivcol == sysrow - 1) 
        {
            if (pivot->num) 
                msg( "Inconsistent system", "", 0 );
            else
                msg( "%sThe %i. equation is redundant", "", equa_in-itr+1);
        }
        if (indx[pivcol] >= 0) 
        { 
            /* Equality for the finite system found */
            /* Store it in ar4 */
            nel_ar4 += eqrl;
            ar4 = (RAT *) RATallo(CP ar4,U nel_ar4-eqrl,U nel_ar4);
            sptr = ar4+(*equa)*eqrl;

            col = sysrow-dim-1+*equa;
            for (i = 0; i < dim-*equa; i++) 
                (*RAT_assign)(sptr+indx[col+i],ar2+col+i);
            for (; i < dim; i++) 
            { 
                (*RAT_assign)((sptr+indx[col+i]),RAT_const);
            }
            (*RAT_assign)(sptr+eqrl-1,ar2+sysrow-1);
            
            (*RAT_row_prim)(sptr,sptr,sptr+eqrl-1,eqrl);
            
            fprintf( prt,
                    " variable %d eliminated - finite equation found\n",
                    indx[pivcol]+1);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( " variable %d eliminated - finite equation found\n",
                 indx[pivcol]+1 );
        }
        /* 
         * For row = 1,..,ineq_in-equa-1,
         * eliminate variable pivcol in porta_list[row]->sys by adding porta_list[0]->sys,
         * store the result in porta_list[ineq+row-1]->sys.
         * The pivot variable is removed and porta_list[ineq+row-1]->sys
         * shortened by 1.
         */
        for (row = 1; row < ineq_in-*equa; row++) 
        {
            gauss_calcnewrow(porta_list[0]->sys,porta_list[row]->sys,pivcol,
                             porta_list[(*ineq)+row-1]->sys,1,sysrow);
        }

        if (indx[pivcol] < 0 && traf) 
        { 
            /* 
             * Create a new inequality in porta_list[*ineq+ineq_in-equa-1]->sys,
             * by "adding" the pivot row to the (implied) inequality -y <= 0.
             * The pivot element is removed.
             */
            for (col = 0,i = 0; col < sysrow; col++)  
                if (col != pivcol) 
                {
                    (*RAT_assign)(porta_list[*ineq + ineq_in-*equa-1]->sys+i,
                                  porta_list[0]->sys+col);
                    i++;
                }
            if (pivot->num < 0)
                for (col = 0,i = 0; col < sysrow; col++) 
                    (porta_list[*ineq + ineq_in-*equa-1]->sys+col)->num *= -1;
        }
        
        /* Rows in "ar2" are now 1 element shorter */
        (sysrow)--;
        
        if (indx[pivcol] >= 0)
            (*equa)++;
        else 
        {
            if (!traf)
                ineq_in--;
        }
        
        /* 
         * Copy porta_list[*ineq]->sys, ..., porta_list[*ineq + ineq_in-equa-1]->sys back
         * to porta_list[0]->sys, ..., porta_list[ineq_in-equa-1]->sys.
         */
        sptr = ar2;
        for (row = 1; row <= ineq_in-*equa; row++) 
        {
            (*RAT_row_prim)(porta_list[*ineq+row-1]->sys,sptr,
                            porta_list[*ineq+row-1]->sys+sysrow-1,sysrow);
            porta_list[row-1]->sys = sptr;
            sptr += sysrow;
        }
        for (row = ineq_in-*equa; row <= 2*(*ineq)-1; row++) 
        {
            porta_list[row]->sys = sptr;
            sptr += sysrow;
        }
        
        for (sptr=ar2, j=sysrow*(ineq_in-*equa), i=nz=ld = 0; 
             i < sysrow*(ineq_in-*equa);
             i++,sptr++)
            size_info(sptr,&nz,&ld);
        
        /* Update indx */
        i = indx[pivcol];
        for (col = pivcol; col <= sysrow-2+*equa; col++)
            indx[col] = indx[col+1];
        if (i>=0) 
            indx[sysrow-2+*equa] = i;
        
#if defined WIN32

        fprintf(prt,
                "|%6i |%11i |%9i |%5i |%4c |%7.2f |%8i |%10.2f |\n",
                itr, itr, ineq_in-itr+1, ld,MP_realised?'y':'n',
                (float)nz/(float)j, total_size/1000, total_time());
        
        /* 17.01.1994: include logging on file porta.log */
        porta_log( "|%6i |%11i |%9i |%5i |%4c |%7.2f |%8i |%10.2f |\n",
            itr, itr, ineq_in-itr+1, ld,MP_realised?'y':'n',
            (float)nz/(float)j, total_size/1000, total_time());

#else // WIN32

        fprintf(prt,
                "|%6i |%11i |%9i |%5i |%4c |%7.2f |%8i |%10.2f |%10.2f |\n",
                itr, itr, ineq_in-itr+1, ld,MP_realised?'y':'n',
                (float)nz/(float)j, total_size/1000, time_used(),
                total_time());
        
        /* 17.01.1994: include logging on file porta.log */
        porta_log( "|%6i |%11i |%9i |%5i |%4c |%7.2f |%8i |%10.2f |%10.2f |\n",
            itr, itr, ineq_in-itr+1, ld,MP_realised?'y':'n',
            (float)nz/(float)j, total_size/1000, time_used(),
            total_time());

#endif // WIN32
    }
    *ineq = ineq_in;
}









void fourier_motzkin( char fname[], int nieq, int rowl, int niterat, 
                      int poi_file, int indx[], int *elim_ord )
/*****************************************************************/
/*
 * Fourier-Motzkin Elimination of "nieq" inequalities given in matrix ar2.
 * Each row in "ar2" is "rowl" elements long,
 * and contains first the "rowl-1" coefficients, then the right-hand side.
 * Inequalities are in "<=" format.
 * "niterat" is the number of iterations, i.e. the number of variables
 * to be eliminated.
 * The output inequality i (0 <= i <= ineq-1) is in vector porta_list[i]->sys.
 * Its right-hand side has absolute value 0 or 1.
 * If "Chernikov_rule_off" is not set, then the Chernikov-rules will be used
 * to eliminate redundant variables.
 * If "Opt_elim" is set, some heuristic decides on which variable to eliminate
 * next, so as to "minimize" the number of new inequalities produced.
 * (This heuristic is not available, if the elimination order is given
 * beforehand in elim_ord)
 * "indx" is only used if "Opt_elim" is set.
 * 
 * This procedure is called either from a "traf" application,
 * or from an "fmel" application.
 * The first case is recognized by elim_ord = 0, 
 * the second by the existence of an array elim_ord.
 * elim_ord (if existent) is an array of length "rowl".
 * For 1<=i<=niterat, elim_ord[i-1] gives the column of "ar2"
 * to be eliminated in the "i"-th iteration.
 * elim_ord[niterat,...,rowl-2] contains the names
 *   of the variables not to be eliminated.
 * elim_ord[rowl-1] = rowl-1.
 * If elim_ord is not existent,
 * the columns to be eliminated from "ar2" are 0,...,niterat-1, in this order
 * (except if Opt_elim is set).
 * If this subroutine is called from a "traf"application,
 * "ar2" does not contain the "-x[i]<=0" inequalities for the variables
 * to be eliminated.
 * These inequalities are treated implicitly by the algorithm.
 */
{ 
    struct list *iep;
    int elcol,col = 0,maxnumineq = 0; 
    int sysrow,i,j,pos,zer,neg,new,p,n,nn,s=1,minineq = 0, nel_xxx;
    RAT *sptr,*sysp,*iesp,*ar3bd,*xxx,sw;
    unsigned *newmark,nmark;
    int ld,nz,nf_dstf=0,finie = 0,nel_cmb;
    register unsigned *nmp,*lnm,*o1mp,*o2mp;
    FILE *fie;
    int *cmb,*bdcmb,*cmbp;
    

    /* FOURIER - MOTZKIN - ELIMINATION */
    
    fprintf(prt,"\nFOURIER - MOTZKIN - ELIMINATION:\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\nFOURIER - MOTZKIN - ELIMINATION:\n");

    if (!MP_realised && is_set(Opt_elim)) 
    {
        if (poi_file) 
        {
            fprintf(prt,"\nstoring finite inequalities in file %s\n\n",fname);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "\nstoring finite inequalities in file %s\n\n",fname);
        }
        strcpy(oestr1,"   #    |\0");
        strcpy(oestr2," facets |\0");
        strcpy(oestr3,"        |\0");
        strcpy(oestr4,"--------|\0");
    }

    strcpy(hest1," upper");
    strcpy(hest2," bound");
    strcpy(hest3,"# ineq");
    print_head_line();
        
    
    /* transponation of the inequalities */
    
    ar2 =  (RAT *) RATallo(CP ar2,nel_ar2,U 2*nieq*rowl);
    sptr = ar2+nieq*rowl;

    /* 
     * Store the transpose of matrix ar2 to location "sptr",
     * then copy matrix "sptr" to location "ar2".
     */
    for (j = 0; j < rowl; j++)
        for (i = 0; i < nieq; i++)
            (*RAT_assign)(sptr++,ar2+i*rowl+j);
    sptr = ar2+nieq*rowl;
    for (i = 0; i < nieq*rowl; i++)
        (*RAT_assign)(ar2+i,sptr++);
    ar2 =  (RAT *) RATallo(CP ar2,U 2*nieq*rowl,U nieq*rowl);
    nel_ar2 = nieq*rowl;
    
    sysrow = nieq+1;
    
    /*
     * Reserve space for rowl-niterat rationals and 
     * for the nieq vectors porta_list[]->sys of length nieq+1,
     */
    /*
     * Change by M.S. 3.6.1992
     * FIRST_SYS_EL may be too small.
     */
    if (rowl-niterat > FIRST_SYS_EL)
        msg( "rowl too big or FIRST_SYS_EL too small", "", 0 );
    nel_ar3 = (FIRST_SYS_EL+(nieq+1)*nieq);
    ar3 = (RAT *) RATallo(CP ar3,0,U nel_ar3);
    ar3bd = ar3+nel_ar3-1;
    
    sysp = ar3+rowl-niterat; /* first not reserved element */
    
    /* 
     * The reserved elements are used for intermediate storage
     * at the end of this subroutine.
     */
    
    /* initialization */
    sptr = sysp;
    ineq = nieq;        
    /* 
     * "ineq" is the number of Fourier-Motzkin inequalities in each iteration,
     * "nieq" is the number of original inequalities.
     */
    blocks = ((elim_ord)?nieq:points)/32 + 1 + 1;
    /* 
     * Each Fourier-Motzkin inequality is a certain nonnegative combination
     * of the original inequalities (stored in "ar2").
     * For each Fourier-Motzkin inequality i,
     * porta_list[i]->sys[1] is the beginning of the vector of those coefficients.
     * porta_list[i]->sys[0] will be used for other purposes.
     * Actually, these vectors are stored in array "ar3",
     * porta_list[i]->sys only points to the respective position in this array.
     * newmark is the same as porta_list[i]->mark
     *    newmark[1] is the beginning of a unit vector (in bits),
     *  and newmark[0] is the number of 1-bits in this vector.
     */
    for (i = 0; i < nieq; i++) 
    {
        allo_list(i,&newmark,blocks);
        porta_list[i]->sys = sptr;
        sptr++;
        for (j = 0; j < nieq; j++,sptr++) 
            if (i == j)
                (*RAT_assign)(sptr,RAT_const+1);   /* RAT_const[1] = 1/1 */
            else
                (*RAT_assign)(sptr,RAT_const);     /* RAT_const[0] = 0/1 */
        for(j = 1; j < blocks; j++)
            newmark[j] = 0;
        newmark[0] = 1;
        newmark[i/32+1] = 1;
        newmark[i/32+1] <<= i%32;  /* shift the one (i mod 32) positions to left,
                                    * that is, set newmark[] := 2 power(i mod 32)*/
    }
    
    for (itr = niterat; itr > 0; itr--) 
    {
        /* elimination column of inequality-system */
        /* 
         * "elcol" is the variable to be deleted,
         * which is a row in the transposed matrix "ar2".
         * "iesp" points to the beginning of that row.
         */
        iesp = ar2+nieq*(elcol = ((elim_ord)?elim_ord[niterat-itr]:niterat-itr));
        
        if (!elim_ord && is_set(Opt_elim) && !MP_realised) 
        {
            /* minimal ineq heuristic */
            
            for (i = 0; i < ineq; i++) 
                porta_list[i]->ptr = 0;
            
            allo_list(ineq,&newmark,blocks);
            lnm = newmark+blocks;
            nf_dstf = 0;
            
            cmbp = cmb = (int *) malloc(INCR_CMB*sizeof(int));
            bdcmb = cmb+INCR_CMB-3;
            nel_cmb = INCR_CMB;
            
            for (p = 0; p < ineq; p++, *cmbp++ = 0) 
                for(n = p+1; n != ineq; n++) 
                {       
                    o1mp = porta_list[p]->mark+1;
                    o2mp = porta_list[n]->mark+1;
                    *newmark = 0;
                    for (nmp = newmark+1; nmp != lnm; nmp++,o1mp++,o2mp++) 
                    {
                        nmark = *nmp = (*o1mp) | (*o2mp);
                        for (; nmark != 0; nmark = nmark & (nmark-1)) 
                            (*newmark)++;
                    }
                    if (*newmark <= s+1) 
                    {
                        *cmbp++ = n;
                        if (cmbp >= bdcmb) 
                        {
                            if (nel_cmb+INCR_CMB > MAX_EL_CMB)  /* no sence */
                                goto break_opt_elim; 
                            pos = cmbp - cmb;
                            if(!(cmb = (int *) realloc(CP cmb,(INCR_CMB+nel_cmb)*sizeof(int))))
                                goto break_opt_elim; 
                            nel_cmb += INCR_CMB;
                            bdcmb = cmb+nel_cmb-3;
                            cmbp = cmb + pos;
                        }
                    }
                }   
            
            new = minineq = cmbp-cmb;    
            
            for (j = itr; j; j--) 
            {
                /* test all possible elimination columns */
                new = 0;
                for (i = 0; i < ineq; i++) 
                {
                    vecpr(iesp, porta_list[i]->sys+1, porta_list[i]->sys, nieq);
                    if (porta_list[i]->sys[0].num < 0)
                        (porta_list[i]->ptr)++;
                }
                
                for (cmbp = cmb, p = 0; p < ineq-1; p++, cmbp++) 
                {
                    neg = (porta_list[p]->sys[0].num < 0);
                    pos = (porta_list[p]->sys[0].num > 0);
                    for (; *cmbp  && new < minineq ; cmbp++)
                        if ((pos  && porta_list[*cmbp]->sys[0].num < 0) ||
                            (neg && porta_list[*cmbp]->sys[0].num > 0)  )  new++;
                }
                if (new < minineq) 
                {
                    minineq = new;
                    col = niterat-j;         /* best column */
                }
                iesp += nieq;
            }  /* for j */
            
            for (i = nieq*elcol, j = nieq*col, nn = 0; nn < nieq; i++,j++,nn++) 
            {
                sw = ar2[i]; ar2[i] = ar2[j]; ar2[j] = sw; 
            }
            iesp = ar2+nieq*elcol;
            
            neg = 9999999;
            for (nn = i = 0; i < ineq; i++) 
            {
                if (!(long) (porta_list[i]->ptr)) 
                    nn++;
                else if ((long) (porta_list[i]->ptr) < neg)
                    neg = (long) (porta_list[i]->ptr)/(sizeof(RAT));
            }
            if ((nf_dstf = nn) > finie) 
            {
                if (poi_file) 
                {
                    fie = fopen (fname,"a");
                    for (i = ineq-1; nn > finie; i--) 
                        if (!(long)(porta_list[i]->ptr)) 
                        {
                            for (j = 0; j < rowl-niterat; j++)
                                vecpr(ar2+nieq*(j+niterat), porta_list[i]->sys+1, ar3+j, nieq);
                            (*RAT_row_prim)(ar3,ar3,ar3+rowl-niterat-1,rowl-niterat);
                            porta_list[ineq]->sys = ar3;
                            if (!MP_realised) no_denom(rowl-niterat,ineq, ineq+1,0);
                            (*writeline)(fie,rowl-niterat,ar3,0,(RAT *) 0,'<',indx);
                            fprintf(fie,"\n");
                            nn--;
                            fflush(fie);
                        }            
                    fclose(fie);
                }
                finie = nf_dstf;
            }
            else if (ineq == finie)
                nf_dstf = ineq;
            else        
                nf_dstf = -neg;
            
            break_opt_elim : ;
            free(CP cmb);
        }   /* end of the "minimal ineq heuristic" */

        /* 
         * Compute the product of vectors "iesp" and porta_list[i]->sys[1] 
         * in porta_list[i]->sys[0].
         * This product is the coefficient of variable "elcol" (or "iesp") in 
         * the i-th Fourier-Motzkin inequality.
         */
        for (i = 0; i < ineq; i++)
            vecpr(iesp, porta_list[i]->sys+1, porta_list[i]->sys, nieq);
        
        /* sptr points to the first unused element in array "ar3" */
        /*
         * Change by M.S. 4.6.92:
         * ineq may be 0.
         sptr = porta_list[ineq-1]->sys+sysrow; 
         */ 
        if (ineq)
            sptr = porta_list[ineq-1]->sys+sysrow;   
        else
            sptr = ar3;
        
        /* 
         * Sort "list" over the signum of the elimination variable,
         * first the positive, then the zero, then the negative coefficients.
         * It is important for later that porta_list[i]->sys < porta_list[j]->sys
         * for any two inequalities i < j that are of the same type ("+","0","-").
         */
        pos = 0;
        for(i = 0 ; i < ineq  ; i++)
            if ((porta_list[i]->sys)->num > 0) 
            {
                /* rotate contents of porta_list[pos,...,i] to the right */
                iep = porta_list[i];
                for (j = i; j > pos; j--) 
                    porta_list[j] = porta_list[j-1] ;
                porta_list[pos++] = iep;
            }
        
        zer = pos;
        for(i = zer ; i < ineq  ; i++)
            if ( !(porta_list[i]->sys->num) ) {
            iep = porta_list[i];
            for (j = i; j > zer; j--) 
                porta_list[j] = porta_list[j-1] ;
            porta_list[zer++] = iep;
        }
        
        /* first indx */
        /* 0 <= zer <= neg <= new,
         * zer is the number of positive coefficients,
         * neg-zer the number of zero coefficients,
         * ineq-neg the number of negative coefficients.
         */
        neg = zer;
        zer = pos;
        new = ineq;
        
        allo_list(new,&newmark,blocks);
        lnm = newmark+blocks;
        
        /* 
         * If there are positive and negative coeff's, 
         * then new inequalities are generated (increase s, starting from s=1).
         * In the case that elim_ord is 0
         * (i.e. fourier_motzkin() was called by "traf"),
         * there exists an inequality with a negative coefficient,
         * namely the nonnegativity constraint (-x <= 0).
         */
        if (zer > 0 && (elim_ord?(neg != ineq):1))
            s++;
        
        /* 
         * Print the number of iterations 
         * and the upper bound on the number of inequalities.
         * nf_dstf is always 0, if the minimal ineq. heuristic is not used.
         * If elim_ord is 0, i.e., if this subroutine is called by "traf",
         * the "+" inequalities are also saved (and are counted in the upper bound).
         * This is because inequalities "-x[i] <= 0" exist implicitly,
         * in case elim_ord=0!
         */
        if (nf_dstf)
        {
            fprintf(prt,"|%6i |%11i ",itr,minineq+ineq);
            
            /* 17.01.1994: include logging on file porta.log */
            porta_log( "|%6i |%11i ",itr,minineq+ineq);
        }
        else
        {
            fprintf(prt,"|%6i |%11i ",itr,(elim_ord?(neg-zer):neg)+zer*(ineq-neg));
            
            /* 17.01.1994: include logging on file porta.log */
            porta_log( "|%6i |%11i ",itr,(elim_ord?(neg-zer):neg)+zer*(ineq-neg));
        }
        
        /* 17.01.1994: include logging on file porta.log */
        fflush(logfile);
        
        fflush(prt); 
        
        /*
         * Normalize the inequality by multiplying it with a positive constant,
         * such that the variable to be eliminated has coefficient +1, 0, or -1.
         */
        for (p = 0; p != zer; p++)
            (*RAT_row_prim)(porta_list[p]->sys,porta_list[p]->sys,porta_list[p]->sys,sysrow);
        for (p = neg; p != ineq; p++)
            (*RAT_row_prim)(porta_list[p]->sys,porta_list[p]->sys,porta_list[p]->sys,sysrow);
        
        
        /* new inequalities generation */
        /* 
         * If a new inequality is generated from an inequality "p" and an ineq. "n"
         * then porta_list[new]->sys is porta_list[p]->sys
         * and  porta_list[new]->ptr is porta_list[n]->sys
         */
        if (option & Chernikov_rule_off) 
            
            for (p = 0; p != zer; p++)     
                for(n = neg; n != ineq; n++) 
                {
                    allo_list(new,&newmark,blocks);
                    porta_list[new]->sys = porta_list[p]->sys;
                    porta_list[new]->ptr = porta_list[n]->sys;
                    new++;    
                }
        else 
        {
            /* for all inequalities with positive coefficients */
            for (p = 0; p != zer; p++) 
                
                /* for all inequalities with negative coefficients */
                for(n = neg; n != ineq; n++) 
                {
                    o1mp = porta_list[p]->mark+1;
                    o2mp = porta_list[n]->mark+1;
                    
                    /* 
                     * "Or" both mark-vectors into newmark[1].
                     * newmark[0] counts the number of 1's in this vector.
                     * (nmark & (nmark-1)) sets the lowest 1-bit in nmark to 0.
                     */
                    *newmark = 0;
                    for (nmp = newmark+1; nmp != lnm; nmp++,o1mp++,o2mp++) 
                    {
                        nmark = *nmp = (*o1mp) | (*o2mp);
                        for (; nmark != 0; nmark = nmark & (nmark-1)) 
                            (*newmark)++;
                    }
                    /* 
                     * First Chernikov-rule:
                     * If the number of 1's in newmark exceeds the number of iterations+1,
                     * the inequality is redundant.
                     * (Here "iterations" counts only those iterations,
                     * where there existed positive and negative coefficients.)
                     */
                    if (*newmark > s) 
                    {
                        goto gennextie;
                    }  
                    
                    /* 
                     * Test newmark versus all inequalities with zero coefficients,
                     * whose mark-vector contains at most as many 1's as newmark[1].
                     * Second Chernikov-rule:
                     * If the set "newmark" contains the set "mark" of any other 
                     * inequality in this iteration,
                     * then the new inequality is redundant.
                     */
                    for (nn = zer;  nn != neg; nn++ )  
                    {
                        o1mp = porta_list[nn]->mark; 
                        nmp = newmark;
                        
                        if (*o1mp++ <= *nmp++)  
                        { 
                            
                            for (; nmp != lnm; nmp++,o1mp++) 
                                /* if set "o1mp" <= set "nmp" */
                                if ( (~(*nmp)) & *o1mp )
                                    goto contin_nro;
                            
                            /* "new inequality" redundant versus old ineq. */
                            goto gennextie;
                        }
                        
                        contin_nro : ;
                    }

                    /* 
                     * Test "newmark" 
                     * versus all inequalities generated in this iteration
                     */
                    for (nn = ineq; nn != new; nn++) 
                    {
                        
                        nmp = newmark;
                        o1mp =  o2mp = porta_list[nn]->mark;
                        
                        if (*o1mp++ <= *nmp++) 
                        {
                            
                            for (; nmp != lnm; nmp++,o1mp++)  
                                if ( (~ (*nmp)) & *o1mp ) 
                                    goto contin_nron;
                            /* "new inequality" redundant versus old "new inequality" */
                            
                            goto gennextie;
                            contin_nron : ;
                        }

                        nmp = newmark;
                        o1mp = o2mp;
                        
                        if (*o1mp++ >= *nmp++) 
                        {
                            
                            for (; nmp != lnm; nmp++,o1mp++) 
                                if (*nmp & ~  (*o1mp) )  
                                    goto contin_onrn;
                            
                            /* 
                             * Old "new inequality" redundant
                             *
                             * Inequality "nn" is removed by renaming "nn" with "new-1",
                             * "new-1" with "new".
                             * The pointer to the storage space used by inequality "nn",
                             * is remembered now in porta_list[new].
                             * allo_list(new,&newmark,blocks) will not allocate any
                             * new space for porta_list[new], nor for porta_list[new]->mark.
                             * The space pointed to by porta_list[new]->sys will be lost, 
                             * but that does not matter, since "ar3" will be reorganized
                             * at the end of this iteration.
                             */
                            iep = porta_list[nn];
                            porta_list[nn] = porta_list[new-1];
                            porta_list[new-1] = porta_list[new];
                            porta_list[new] = iep;
                            new--;
                            nn--; 
                            contin_onrn : ;
                        }
                    }

                    porta_list[new]->mark = newmark;
                    porta_list[new]->sys = porta_list[p]->sys;
                    porta_list[new]->ptr = porta_list[n]->sys;
                    
                    new++;
                    
                    allo_list(new,&newmark,blocks);
                    lnm = newmark+blocks;
                                        
                    gennextie : ;
                } /* for all inequalities with negative coefficients
                   * and all with negative coefficients */
        } /* if the Chernikov-rules were used */
        
        /* numerical phase */
        for (i = ineq; i != new; i++) 
        {
            if (sptr+sysrow > ar3bd) 
            {
                /* 
                 * Make array a3 larger, 
                 * and recompute porta_list[]->sys, porta_list[]->ptr, sptr, ar3bd and sysp.
                 */
                reallocate(new, &sptr);
                ar3bd = ar3+nel_ar3-1;
                /*
                 * Change by M.S. 3.6.1992:
                 * "sysp" points to the beginning of the space used by porta_list[]->sys.
                 * If fourier_motzkin() is called by "traf", sysp = ar3+dim+1-equa.
                 * If fourier_motzkin() is called by "fmel", sysp = ar3+rowl-niterat.
                 * rowl-niterat = dim+1-equa, whenever fourier_motzkin()
                 * is called by "traf", but not when it is called by "fmel".
                 *
                 sysp = ar3+dim+1-equa;
                 */
                sysp = ar3+rowl-niterat;
            }
            /*
             * Add the two inequalities making up the new one.
             * Recall that one of them has coeff. +1, the other coeff. -1,
             * for the elimination variable.
             */
            row_add(porta_list[i]->sys+1,porta_list[i]->ptr+1,sptr+1,sysrow-1); 
            porta_list[i]->sys = sptr;
            sptr = sptr+sysrow;        
        }
        
        /* reordering of sys */
        sptr = sysp;
        nmark = (zer != 0 && neg != ineq) ? 1 : 0;
        /*  nmark is not used for elim_ord != 0. */
        nmark <<= ((points-itr) % 32);
        pos = zer;
        
        if (elim_ord)  
        {
            /*
             * Move the 0-inequalities to the front of porta_list[].
             * Move also the whole vector porta_list[]->sys for such an inequality
             * to the front of array "ar3",
             * overwriting the positive inequalities.
             * Since porta_list[j]->sys < porta_list[i]->sys for any two "0"inequalities j < i,
             * one may move the contents of porta_list[]->sys sequentially.
             * porta_list[j]->sys[0] is not up to date after this operation,
             * but it will be recomputed in the next iteration.
             */
            for (j = 0,i = zer; i < neg; j++, i++ ) 
            { 
                /* save 0 ieqs */
                sptr++; 
                iep = porta_list[j]; porta_list[j] = porta_list[i]; porta_list[i] = iep;
                for (col = 1; col <= nieq; col++,sptr++)   
                    (*RAT_assign)(sptr,porta_list[j]->sys+col);
                porta_list[j]->sys = sptr - sysrow;
            }
            neg = j;
        }
        else 
            /*
             * Move the vectors porta_list[]->sys to the front of array "ar3"
             * for all "+" and "0" inequalities.
             * The trouble is, when this is done sequentially, first for the "+"ieqs,
             * one may overwrite something in "ar3" being pointed to 
             * by a "0" inequality (but not by a "+" inequality!!).
             * Therefore the following is quite complicated.
             */
            for (i = 0,j = zer; (i < zer) || (j < neg); ) 
            {
                sptr++; 
                if (j == neg || (porta_list[i]->sys < porta_list[j]->sys && i != zer)) 
                {
                    /* first try to save as many + inequalities as possible */
                    for (col = 1; col <= nieq; col++,sptr++)   /* save + ieqs */
                        (*RAT_assign)(sptr,porta_list[i]->sys+col);
                    porta_list[i]->mark[(points-itr)/32+1] |= nmark;
                    /* 
                     * In the present "+" inequality,
                     * set the bit belonging to the implicit "-x[i]<=0"constraint.
                     * (This bit is not set, if there were no "+" and no "-" inequalities
                     * in the iteration.)
                     */
                    porta_list[i]->sys = sptr - sysrow;
                    i++;
                }
                else   
                {
                    for (col = 1; col <= nieq; col++,sptr++) /* save 0 ieqs */
                        (*RAT_assign)(sptr,porta_list[j]->sys+col); 
                    porta_list[j]->sys = sptr - sysrow;
                    /* 
                     * Rotate the contents of porta_list[i,...,j] to the right: link[j,i,...].
                     * Thereby porta_list[k]->sys < porta_list[l]->sys is preserved for k < l
                     * of the same type.
                     */
                    iep = porta_list[j];
                    for (n = j; n > i; n--)
                        porta_list[n] = porta_list[n-1];
                    porta_list[i] = iep; 
                    i++;
                    zer++;
                    j++;
                }
            }
                
        /*
         * Move the new inequalities to position j in "list",
         * and to position "sptr" in "ar3",
         * thereby overwriting the "-"inequalities.
         * In porta_list[]->sys, divide the numerators by their gcd,
         * and divide the denominators by their gcd.
         */
        for (i = ineq, j = neg; i != new; i++,j++) 
        {
            /* overwrite - ieqs */
            iep = porta_list[i];
            porta_list[i] = porta_list[j];
            porta_list[j] = iep;
            (*RAT_row_prim)(iep->sys+1,sptr+1,RAT_const,nieq);
            porta_list[j]->sys = sptr;
            sptr += sysrow;
        }  
        
        ineq = j;
        
        nz = ld = 0;
        /* compute number of nonzeros nz, and the max bit-length ld */
        for (i = 0,sptr=sysp; i < ineq;i++,sptr++)
            for (j=0,sptr=porta_list[i]->sys+1; j < nieq;j++,sptr++)
                size_info(sptr,&nz,&ld);
        
        /* output */
        
        p = 0;  /* number of non-zeros */
        
#if defined WIN32

        fprintf(prt,"|%9i |%5i |%4c |%7.2f |%8i |%10.2f |",
                ineq,ld,MP_realised?'y':'n',
                (ineq && sysrow ? (float)nz/(float)(ineq*sysrow) : 0),
                total_size/1000, total_time());

        /* 17.01.1994: include logging on file porta.log */
        porta_log( "|%9i |%5i |%4c |%7.2f |%8i |%10.2f |",
                ineq,ld,MP_realised?'y':'n',
                (ineq && sysrow ? (float)nz/(float)(ineq*sysrow) : 0),
                total_size/1000, total_time());

#else // WIN32

        fprintf(prt,"|%9i |%5i |%4c |%7.2f |%8i |%10.2f |%10.2f |",
                ineq,ld,MP_realised?'y':'n',
                (ineq && sysrow ? (float)nz/(float)(ineq*sysrow) : 0),
                total_size/1000,time_used(),total_time());

        /* 17.01.1994: include logging on file porta.log */
        porta_log( "|%9i |%5i |%4c |%7.2f |%8i |%10.2f |%10.2f |",
                ineq,ld,MP_realised?'y':'n',
                (ineq && sysrow ? (float)nz/(float)(ineq*sysrow) : 0),
                total_size/1000,time_used(), total_time());

#endif // WIN32

        if (!MP_realised && is_set(Opt_elim))
        {
            fprintf(prt,"%7i |\n",finie);
            
            /* 17.01.1994: include logging on file porta.log */
            porta_log( "%7i |\n",finie);
        }
        else 
        {
            fprintf(prt,"\n");
            
            /* 17.01.1994: include logging on file porta.log */
            porta_log( "\n");
        }

        /* 17.01.1994: include logging on file porta.log */
        fflush(logfile);
        
        fflush(prt);
        
        if (ineq > maxnumineq) 
            maxnumineq = ineq;
        
        totalineq += ineq;
        
    } /* for itr */
    
    fprintf(prt,"\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\n" );
    
    sptr = ar3;
    sysrow = rowl-niterat;
    /* 
     * sysrow was the length of the porta_list[]->sys vectors, which is nieq+1.
     * rowl-niterat is the number of noneliminated variables+1 in case of "fmel",
     *              is dim + 1 - equa                in case of "traf".
     * Now the inequalities are computed by multiplying porta_list[]->sys
     * with matrix "ar2".
     * To store the results, "ar3" has to be enlarged if sysrow > nieq+1.
     */
    if (sysrow > nieq+1) 
    {
        /*
         * Change by M.S. 4.6.92:
         *
         * The following resizing and saving of contents of "ar3" 
         * may go awfully wrong, if "ar3" was moved a little to the left
         * by realloc().
         * But for "traf" applications, sysrow is "dim+1-equa",
         * and nieq+1 is at least dim+1,
         * So the following was never executed for "traf" applications.
         * BUT for "fmel", sysrow (= rowl - niterat) > nieq+1 is possible.
         *
         i = (ineq+1)*sysrow;
         ar3 = (RAT *) RATallo(ar3,nel_ar3,i);
         nel_ar3 = i;
         for (i = ineq-1, sptr = ar3+ineq*sysrow; i >= 0; i--, sptr -= sysrow) {
         for (j = 0; j < nieq+1; j++) 
         (*RAT_assign)(sptr+j,porta_list[i]->sys+j);
         porta_list[i]->sys = sptr;
         }
         * Instead, a new array xxx of size (ineq+1)*sysrow is allocated,
         * the contents of ar3 stored in it, ar3 is freed, 
         * and xxx is renamed as ar3.
         */
        nel_xxx = (ineq+1)*sysrow;
        xxx = (RAT *) RATallo( (RAT *)0, 0, nel_xxx);
        for(i=ineq-1, sptr=xxx+ineq*sysrow; i>=0; i--,sptr-=sysrow) 
        {
            for (j = 0; j < nieq+1; j++)
                (*RAT_assign)(sptr+j, porta_list[i]->sys+j);
            porta_list[i]->sys = sptr;
        }
        RATallo(ar3, nel_ar3, 0);
        ar3 = xxx;
        nel_ar3 = nel_xxx;
        /* sptr is now ar3 and points to some unused space of size rowl-niterat */
    }

    iesp = ((elim_ord)?ar2:ar2+niterat*nieq);
    if (elim_ord) elim_ord += niterat;
    /* 
     * "iesp" is the offset for the list of non-eliminated variables (rows),
     * in ar2.
     * The list of variables not to be eliminated begins with elim_ord[niterat].
     * If elim_ord was not used, the not-eliminated variables are called
     * niterat+1, ..., dim.
     * 
     * Compute the product of porta_list[]->sys with the original inequalities,
     * but only for the non-eliminated variables and the right-hand side.
     * The result is stored one vector to the left of porta_list[]->sys.
     * (This is the reason for the free space at the beginning of ar3).
     */
    for (i = 0; i < ineq; i++,sptr += sysrow)  
    {
        for (j = 0; j < sysrow; j++)
            vecpr(iesp+nieq*((elim_ord)?elim_ord[j]:j), porta_list[i]->sys+1, sptr+j, nieq);
        porta_list[i]->sys = sptr;
        /* 
         * Transform inequalities into "<=1", "<=-1" inequalities,
         * or divide the left-hand side by a common gcd.
         */
        (*RAT_row_prim)(sptr,sptr,sptr+sysrow-1,sysrow);
    }
    
    for (i = 0; i < ineq; i++)  
    {  
        /*  0x <= 1 ? */
        for ( j = 0; j < sysrow-1; j++)   
            if (porta_list[i]->sys[j].num) break;
        
        /*
         * Change by M.S. 4.6.92:
         * Remove the inequality only if it is 0x <= 0, 0x <= 1,
         * but not if it is 0x <= -1.
         * I also removed the break after ineq--,
         * as this causes the program to jump out of the for (i) loop,
         * whenever one empty inequality was found.
         */
        if ( j == sysrow-1 ) 
        {
            if ( porta_list[i]->sys[j].num >= 0) 
            {
                rmlistel(blocks,i,ineq-1,1,sysrow);
                ineq--;
            }
            else
                msg( "Inconsistent system", "", 0 );
        } 
    }

    fprintf(prt,"sum of inequalities over all iterations : %6i\n",totalineq);
    fprintf(prt,"maximal number of inequalities          : %6i\n\n",maxnumineq);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "sum of inequalities over all iterations : %6i\n",totalineq);
    porta_log( "maximal number of inequalities          : %6i\n\n",maxnumineq);
    
    /* append pointers to equations */
    
    for (i = ineq, sysp = ar4; i < ineq+equa; i++, sysp += dim+1) 
    {
        allo_list(i,&newmark,blocks);
        porta_list[i]->sys = sysp;
    }

    /* This should not done, because the value of ar3 is assigned to
       porta_list[~]->sys. Reallocating causes a free memory read in
       origin_add(). 
       18.01.1994, Andreas Loebel
     * 
     * Make ar3 just as big as it should be 
     * (this does not change the position of ar3) 
     *
     i = nel_ar3; nel_ar3 = (ineq)*sysrow;
     ar3 = (RAT *) RATallo(ar3,i,nel_ar3);
     */
}
    








void red_test( int indx[], RAT *inieq, int *rowl_inar )
{  
    RAT  *convmid,*mid,*x;
    RAT  *ptr,*pp,*lpp,*ie1p;
    int  sysrow,i,j,ncv,nce,b,ie1,ie2;
    
    if (option & Redundance_check)
    {
        fprintf(prt,"redundance - check ");

        /* 17.01.1994: include logging on file porta.log */
        porta_log( "redundance - check ");
    }
    else
    {
        fprintf(prt,"testing strong validity ");

        /* 17.01.1994: include logging on file porta.log */
        porta_log( "testing strong validity ");
    }
    
    /* REDUCE inieq TO NONELIMINATED VARIABLES */
    
    pp = inieq;
    for (i = 0; i < points; i++)  
    {
        for (j = 0; j < dim-equa; j++)
            (*RAT_assign)(pp++ ,&inieq[i*(dim+1)+indx[j]]);
        (*RAT_assign)(pp++,&inieq[i*(dim+1)+dim]);
    }
    
    /* LOOK FOR INTERNAL POINTS OF INEQUALITY ie SATISFYING ANOTHER IEQ WITH = */
    
    sysrow = dim-equa;
    *rowl_inar = dim-equa+1;

    nel_ar5 = 3*(sysrow+1);
    ar5 = (RAT *) RATallo (CP ar5,0,nel_ar5);
    convmid = ar5;
    mid = ar5+sysrow+1;
    x = mid+sysrow+1;
    
    
    for (ie1 = 0; ie1 != ineq; ie1++) 
    {
        /*******************************/
        
        if (ie1%50 == 0) 
        {
            fprintf(prt,".");
            fflush(prt);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "." );
            fflush(logfile);
        }
        
        ie1p = porta_list[ie1]->sys;
        
        for (ptr = convmid; ptr != convmid+sysrow; ptr++)
            (*RAT_assign)(ptr,RAT_const);
        
        ncv = nce =  0;
        for (b = 0; b < blocks; b++)
            porta_list[ie1]->mark[b] = 0;
        
        lpp = inieq+*rowl_inar*points;
        for (i = 0,pp = inieq; pp < lpp;  pp += *rowl_inar,i++)
        {
            /********************************************/
            
            if (!pp[dim-equa].num) continue;
            
            /* middle of satisfying conv-points */
            
            if (eqie_satisfied(ie1p,pp,sysrow,0))
            {
                
                domark(porta_list[ie1]->mark,i);
                ncv++;
                row_add(pp,convmid,convmid,sysrow);
                
            }
            
        }
        
        if (ncv > 0) 
        {
            if (MP_realised)  L_RAT_to_RAT(var+3,1);
            var[3].num = 1;
            var[3].den.i = ncv;
            if (MP_realised) RAT_to_L_RAT(var+3,1); 
            scal_mul(var+3,convmid,convmid,sysrow);
        }
        
        if (cone) 
        {
            for (ptr = mid; ptr != mid+sysrow; ptr++)
                (*RAT_assign)(ptr,RAT_const);
            
            for (i = 0,pp = inieq; pp < lpp; pp += *rowl_inar,i++)
            {
                /********************************************/
                
                if (pp[dim-equa].num) continue;
                
                row_add(pp,convmid,x,sysrow);
                
                if (eqie_satisfied(ie1p,x,sysrow,0)) 
                {
                    
                    domark(porta_list[ie1]->mark,i);
                    nce++;
                    row_add(x,mid,mid,sysrow);
                    
                }
                
            }
            
            if (nce > 0) 
            {
                if (MP_realised)  L_RAT_to_RAT(var+3,1);
                var[3].num = 1;
                var[3].den.i = nce;
                if (MP_realised) RAT_to_L_RAT(var+3,1); 
                scal_mul(var+3,mid,mid,sysrow);
            }
            else
                for (i = 0; i != sysrow; i++)  
                    (*RAT_assign)(mid+i, convmid+i);
        }
        else
            for (i = 0; i != sysrow; i++)  
                (*RAT_assign)(mid+i, convmid+i);
        
        if (!(option & Redundance_check)) /* option.valout == 'v' */
            continue;
        
        if (ncv + nce > 0) 
        {
            
            for (ie2 = 0; ie2 != ineq; ie2++) 
            {
                if (ie1 != ie2) 
                {
                    if (eqie_satisfied(porta_list[ie2]->sys,mid,sysrow,0)) 
                    {
                        fprintf (prt,"r");

                        /* 17.01.1994: include logging on file porta.log */
                        porta_log( "r" );

                        rmlistel(blocks,ie1,ineq-1,1,sysrow);
                        rmlistel(blocks,ineq-1,ineq-1+equa,0,sysrow);
                        ie1--;ineq--;
                        break;
                    }
                }
            } /* for ie1 */
        }
        
        else  
        {
            fprintf (prt,"r");

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "r");

            rmlistel(blocks,ie1,ineq-1,1,sysrow);
            rmlistel(blocks,ineq-1,ineq-1+equa,0,sysrow);
            ie1--;ineq--;
        }
        
    } /* for ie1 */
    fprintf(prt,"\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\n" );
    
    pp = ar3;
    sysrow++;
    for (i = 0; i < ineq; i++) 
    {
        porta_list[i]->ptr = pp;
        for (j = 0; j < sysrow; j++, pp++)
            (*RAT_assign)(pp, porta_list[i]->sys+j);
        porta_list[i]->sys = porta_list[i]->ptr;
    }
    
    ar5 = (RAT *) RATallo(ar5,nel_ar5,0);
    nel_ar5 = 0;
    
}








void domark( unsigned *m, int n )
{
    unsigned mark;
    
    mark = 1;
    mark <<= n%32;
    m[n/32] |= mark;
    
}









void rmlistel( int blocks, int cel, int lastie, int real, int sysrow )
/*****************************************************************/
/*
 * Remove element cel from list.
 * If real = 1,
 * move the contents of porta_list[cl+i]->sys and porta_list[cl+i]->mark physically 
 * to porta_list[cl+i-1]->sys, and porta_list[cl+i-1]->mark, for i = 1, ..., lastie.
 * Otherwise,
 * rotate the contents of porta_list[cl, ..., lastie] to the left.
 */
{
    listp lp;
    int i,j;
    
    if (real) 
    {
        for (i = cel; i < lastie; i++) 
        {
            lp = porta_list[i];
            for (j = 0; j <= sysrow; j++)
                (*RAT_assign)(lp->sys+j, porta_list[i+1]->sys+j);
            for (j = 0; j < blocks; j++)
                lp->mark[j] = porta_list[i+1]->mark[j];
        }
    }
    else 
    { 
        lp = porta_list[cel];
        for (i = cel; i < lastie; i++) 
            porta_list[i] = porta_list[i+1];
        porta_list[lastie] = lp;
    }
    
}










void wl( RAT *p, int n )
{
    for( ; n; n--, p++ ) 
    {
        printf("%ld/%i ",p[0].num,p[0].den.i);
    
        /* 17.01.1994: include logging on file porta.log */
        porta_log( "%ld/%i ",p[0].num,p[0].den.i );
    }
    printf("\n");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\n" );
}

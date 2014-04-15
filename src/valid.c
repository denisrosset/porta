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
 

FILENAME: valid.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Wed Sep 18 17:47:11 2002 by Andreas Loebel (opt0.zib.de)  */
/* $Id: valid.c,v 1.4 2009/09/21 07:05:11 bzfloebe Exp $ */


#include "valid.h"
#include "arith.h"
#include "common.h"
#include "inout.h"
#include "log.h"


FILE *logfile;

FILE *fp;
int *lowbds,*upbds;







int main( int argc, char *argv[] )
{
    char *fout;
    int   i, j,k,iethpo, pothie;
    FILE *outfp = 0;



    printf("\nPORTA - a POlyhedron Representation Transformation Algorithm\n");
    printf(  "Version %s\n\n", VERSION );

    printf( "Written by Thomas Christof (Uni Heidelberg)\n" );
    printf( "Revised by Andreas Loebel (ZIB Berlin)\n\n" );

    printf( "PORTA is free software and comes with ABSOLUTELY NO WARRENTY! You are welcome\n" );
    printf( "to use, modify, and redistribute it under the GNU General Public Lincese.\n\n" ); 
    
    printf( "This is the program VALID from the PORTA package.\n\n" );

    if( argc <= 2 )
    {
        fprintf( stderr,
                "For more information read the manpages about porta.\n\n");
        exit(-1);
    }
            
    /* 17.01.1994: include logging on file porta.log */
    if( !(logfile = fopen( "porta.log", "a" )) )
        fprintf( stderr, "can't open logfile porta.log\n" );
    else
    {
        porta_log( "\n\n\nlog for " );
        for( i = 0; i < argc; i++ )
            porta_log( "%s ", argv[i] );
        porta_log( "\n\n" );
    }
            

    initialize();
    set_I_functions();
    SET_MP_not_ready;
    
    prt = stdout;
    get_options(&argc,&argv);
    
    if (option & Protocol_to_file) 
    {
        strcat(*argv,".prt");
        prt = fopen(*argv,"w");
        (*argv)[strlen(*argv)-4] = '\0';
    }
    setbuf(prt,CP 0);
    
    if (is_set(Vint) && !strcmp(*argv+strlen(*argv)-4,".ieq")) 
    {  
            char *cp1,*cp2;
        cp1=strdup("LOWER_BOUNDS");
        cp2=strdup("UPPER_BOUNDS");
        if(!cp1||!cp2)
            msg("allocation of new space failed", "", 0 );   
        points = read_input_file(*argv,outfp,&dim,&ar1,(int *)&nel_ar1,
            cp1,&lowbds,cp2,&upbds,"\0",(RAT **)&i);
        free(cp1);free(cp2);
        sort_eqie_cvce(ar1,points,dim+2,&equa,&ineq);
        valid_ints( dim, ar1, equa, dim+2, dim,
                    ar1+(dim+2)*equa, ineq, dim+2, *argv );
    }
    else 
    {
        pothie = !strcmp(*argv+strlen(*argv)-4,".poi") 
            && !strcmp(*(argv+1)+strlen(*(argv+1))-4,".ieq");
        iethpo = !strcmp(*argv+strlen(*argv)-4,".ieq") 
            && !strcmp(*(argv+1)+strlen(*(argv+1))-4,".poi");
        if (!iethpo && !pothie)
            msg("invalid format of command line", "", 0 );
        
        if (iethpo) 
        {
            ineq = read_input_file(*argv,outfp,&dim,&ar1,(int *)&nel_ar1,
                                   "\0",(int **)&i,"\0",(int **)&i,"\0",(RAT **)&i );
            points = read_input_file(*(argv+1),outfp,&j,&ar2,(int *)&nel_ar2,
                                     "\0",(int **)&i,"\0",(int **)&i,"\0",(RAT **)&i );
        }
        else 
        {
            ineq = read_input_file(*(argv+1),outfp,&dim,&ar1, (int *)&nel_ar1,
                                   "\0", (int **)&i,"\0", (int **)&i,"\0", (RAT **)&i );
            points = read_input_file(*argv,outfp,&j,&ar2, (int *)&nel_ar2,
                                     "\0", (int **)&i,"\0", (int **)&i,"\0", (RAT **)&i );
        }
        if (j != dim) 
            msg("dimensions in input files are different", "", 0 ); 
        
        if (is_set(Iespo)) 
        {
            sort_eqie_cvce(ar1,ineq,dim+2,&equa,&ineq);
            valid_ieqs(dim,ar1,ineq+equa,&equa,&ineq,dim+2,
                       ar2,points,dim+1,(char **)(pothie?*argv:*(argv+1)));
        }
        else if (is_set(Posie) || is_set(Cfctp)) 
        {
            fout = iethpo?*argv:*(argv+1);
            sort_eqie_cvce(ar2,points,dim+1,&cone,&conv);
            if (is_set(Posie))
                valid_points(dim,ar2,points,dim+1,
                             ar1,ineq,dim+2,0,(char **)fout);
            else 
            {
                char fname[100];
                char command[100];
                RAT *ar1p;
                sort_eqie_cvce(ar1,ineq,dim+2,&equa,&ineq);
                if (equa )
                    msg( "only inequalities are allowed", "" , 0 );
                
                for (i = 0,ar1p = ar1; i < ineq; i++, ar1p += dim+2) 
                {
                    sprintf(fname,"%s%i",fout,i+1);
                    fprintf(prt,"I n e q u a l i t y   %2i :\n",i+1);
                    fprintf(prt,"==========================\n\n");
                    
                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "I n e q u a l i t y   %2i :\n",i+1);
                    porta_log( "==========================\n\n");
                    
                    fprintf(prt,"computing points  i n v a l i d  for the inequality :\n");
                    fprintf(prt,"=====================================================\n");

                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "computing points  i n v a l i d  for the inequality :\n");
                    porta_log( "=====================================================\n");

                    var[3].num = -1; var[3].den.i = 1;
                    scal_mul(var+3,ar1p,ar1p,dim+1);
                    j = valid_points(dim,ar2,points,dim+1,
                                     ar1p,1,dim+2,1, (char **)fname);
                    (ar1p+dim+1)->num = 0;
                    fprintf(prt,"computing points satisfying inequalitiy with equation :\n");
                    fprintf(prt,"=======================================================\n");
                    
                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "computing points satisfying inequalitiy with equation :\n");
                    porta_log( "=======================================================\n");
                    
                    if (j) strcat(fname,".poi");
                    k = valid_points(dim,ar2,points,dim+1,
                                     ar1p,1,dim+2,0, (char **)fname);
                    strcat(fname,".poi");
                    if (!j && k) 
                    {
                        sprintf(command,"dim %s",fname);
                        fprintf(prt,"dimension of the points :\n");
                        fprintf(prt,"=========================\n");
                        fflush(prt);
                        
                        /* 17.01.1994: include logging on file porta.log */
                        porta_log( "dimension of the points :\n");
                        porta_log( "=========================\n");
                        fflush( logfile );
                        
                        system(command);
                    }
                }
                
            }
        }
        else 
            msg( "invalid format of command line", "", 0 );
    } 

    return 0;
}









int valid_points( int dim, RAT *par, int npoi, int pard, RAT *iear, 
                  int nie, int ieard, int not_equal, char **fname)
{ 
    RAT *pptr,*ieptr,*ubpar,*ubiear,*np,s;
    int v,ie,ncv=0,nce=0;
    
    fprintf (prt,"filtering points satisfying given linear system ");
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "filtering points satisfying given linear system ");
    
    ubpar = par+npoi*pard;
    ubiear = iear+nie*ieard;
    np = par;
    
    for (pptr = par,ie = 0; pptr < ubpar; pptr += pard, ie++) 
    {
        if (ie%100 == 1) 
        {
            fprintf(prt,"."); fflush(prt);

            /* 17.01.1994: include logging on file porta.log */
            porta_log("."); fflush(logfile);
        }
        
        for (ieptr = iear; ieptr < ubiear; ieptr += ieard) 
        {
            if (! pptr[dim].num) 
            { 
                /* cone  */
                s = ieptr[dim];
                ieptr[dim] = RAT_const[0];
            }
            if (!(v = eqie_satisfied(ieptr,pptr,dim,(ieptr+dim+1)->num))  ||
                (not_equal && v==2)) 
                v = 0;
            if (! pptr[dim].num)   /* cone  */
                ieptr[dim] = s;
            if (!v) break;
        }
        
        if (ieptr == iear + nie*ieard) 
        {
            allo_list(nce+ncv,0,blocks);
            porta_list[nce+ncv]->sys = pptr;
            if ((pptr+dim)->num) ncv++;
            else nce++;
        }
    }

    if (!nce && !ncv)
    {
        fprintf(prt,"\nno points found\n\n");

        /* 17.01.1994: include logging on file porta.log */
        porta_log( "\nno points found\n\n");
    }
    else
        write_poi_file( (char *)fname,0,dim,0,0,nce,0,ncv,nce);
    return(nce+ncv);
}








void valid_ieqs( int dim, RAT *iear, int neqie_in, int *neq_out, int *nie_out,
                 int ieard, RAT *par, int npoi, int pard, char **fname )
{ 
    RAT *pptr,*ieptr,*ubpar,*ubiear,*np,s;
    int i,ie,b,p,valid;
    
    unsigned *mp,m;
    
    printf ("filtering inequalities and equations valid for all given points ");

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "filtering inequalities and equations valid for all given points ");
    
    ubpar = par+npoi*pard;
    ubiear = iear+neqie_in*ieard;
    np = iear;
    *neq_out = *nie_out = 0;
    
    blocks = (is_set(Validity_table_out)) ?  (npoi-1)/32+1 : 0;
    allo_list(0,&mp,blocks);
    
    for (ieptr = iear,ie=0; ieptr < ubiear; ieptr += ieard,ie++) 
    {
        for (b = 0; b < blocks; b++)
            mp[b] = 0;
        
        if (ie%100 == 0) 
        {
            fprintf(prt,"."); fflush(prt);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "." ); fflush(logfile);
        }
        
        for (pptr = par, p = 0; pptr < ubpar; pptr += pard, p++) 
        {
            if (! pptr[dim].num) 
            {
                /* cone  */
                s = ieptr[dim];
                ieptr[dim] = RAT_const[0];
            }
            valid = eqie_satisfied(ieptr,pptr,dim,(ieptr+ieard-1)->num);
            if (valid == 2 && is_set(Validity_table_out)) 
            { 
                /* ieq strong valid */
                m = 1;
                m <<= p %32;
                mp[p/32] |= m;
            }
            if (! pptr[dim].num) 
            {
                /* cone  */
                ieptr[dim] = s;
                if (!valid) break;
            }
        }
        
        if (pptr == par+npoi*pard) /* valid */ 
        {
            porta_list[*nie_out+*neq_out]->sys = np;
            if ((ieptr+ieard-1)->num) 
                (*nie_out)++;
            else 
                (*neq_out)++;
            allo_list(*nie_out+*neq_out,&mp,blocks);
            
            for (i = 0; i < ieard; i++)
                *np++ = ieptr[i];
        }
    }
    
    write_ieq_file((char *)fname,0,*neq_out,0,dim+1,0,*nie_out,*neq_out,dim+1,0);
    
    fprintf(prt,"\n");
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log("\n");
}






int *integ,i,ie,eqdim,eqrl,iedim,ierl,nie,neq,*el, intnum, reknum;
RAT val,*eqar,*iear,*ptr,*ubia,*ubea,*osump,*nsump;








void integ_rekurs( RAT *eqsum, RAT *iesum, int lev )
{ 
    val.den.i = 1;
    
    if (reknum % 1000 == 0) 
    {
        fprintf(prt,".");
        fflush(prt);

        /* 17.01.1994: include logging on file porta.log */
        porta_log(".");
        fflush(logfile);
    }
    reknum++;
    
    if (lev == eqdim) 
    {
        for (ptr = eqar+eqdim; ptr < ubea; ptr += eqrl, eqsum++) 
        {
            I_RAT_sub(*eqsum,*ptr,&val);
            if (val.num != 0)
                return;
        }
        for (ptr = iear+iedim; ptr < ubia; ptr += ierl, iesum++) 
        {
            I_RAT_sub(*iesum,*ptr,&val);
            if (val.num > 0)
                return;
        }
        
        intnum++;
        fprintf(fp,"(%3d) ",intnum);
        for (i = 0; i < eqdim; i++) 
            fprintf(fp,"%i ",integ[i]);
        fprintf(fp,"\n");
    }
    else 
    {
        val.num = lowbds[lev];
        osump = eqsum;
        nsump = eqsum+neq;
        for (ptr = eqar+lev; ptr < ubea; ptr += eqrl,osump++,nsump++) 
        {
            I_RAT_mul(*ptr,val,nsump);
            I_RAT_add(*osump,*nsump,nsump);
        }
        osump = iesum;
        nsump = iesum+nie;
        for (ptr = iear+lev; ptr < ubia; ptr += ierl,osump++,nsump++) 
        {
            I_RAT_mul(*ptr,val,nsump);
            I_RAT_add(*osump,*nsump,nsump);
        }
        
        for (integ[lev]=lowbds[lev]; integ[lev]<upbds[lev]; integ[lev]+=1) 
        {
            integ_rekurs(eqsum+neq,iesum+nie,lev+1);
            osump = eqsum;
            nsump = eqsum+neq;
            for (ptr = eqar+lev; ptr < ubea; ptr += eqrl,nsump++)
                I_RAT_add(*nsump,*ptr,nsump);
            osump = iesum;
            nsump = iesum+nie;
            for (ptr = iear+lev; ptr < ubia; ptr += ierl,nsump++)
                I_RAT_add(*nsump,*ptr,nsump);
        }
        integ_rekurs(eqsum+neq,iesum+nie,lev+1);
    }
}







void valid_ints( int eqdimpar, RAT *eqarpar, int neqpar,int eqrlpar,
                int iedimpar, RAT *iearpar, int niepar, int ierlpar, char *fname )
{ 
    RAT *eqsum = (RAT *)0;
    RAT *iesum = (RAT *)0;
    
    fprintf (prt,"computing all valid integral points ");
    fflush(prt);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "computing all valid integral points ");
    fflush(logfile);
    
    if (!lowbds || !upbds)
        msg( "\nno bounds are given", "", 0 );
    for (eqdim = 0 ; eqdim < eqdimpar; eqdim++)
        if (lowbds[eqdim] > upbds[eqdim])
            msg("lower bound greater than upper bound", "", 0);
    
    neq = neqpar;
    eqdim = eqdimpar;
    eqar = eqarpar;
    eqrl = eqrlpar;
    ubea = eqar+neq*eqrl;
    
    nie = niepar;
    iedim = iedimpar;
    iear = iearpar;
    ierl = ierlpar;
    ubia = iear+nie*ierl;
    
    fname[strlen(fname)-4] = '\0';
    strcat(fname,".poi");
    fp = wfopen(fname);
    
    fprintf(fp,"DIM =%3d\n\nCONV_SECTION\n",dim);
    
    integ = (int *) allo(integ,0,eqdim*sizeof(int)); 
    reknum = intnum = 0;
    eqsum = (RAT *) RATallo(eqsum,0,(1+neq)*(eqdim+1));
    iesum = (RAT *) RATallo(iesum,0,(1+nie)*(iedim+1));
    
    integ_rekurs(eqsum,iesum,0);
    
#if defined WIN32
    free(integ);
#else // WIN32
    cfree(integ);
#endif // WIN32

    fprintf(fp,"\nEND\n");
    fclose(fp);
    
    fprintf(prt,"\n\nnumber of valid integral points : %i\n\n",intnum);
    fprintf(prt,"integral points written to file %s\n",fname);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\n\nnumber of valid integral points : %i\n\n",intnum);
    porta_log( "integral points written to file %s\n",fname);
}

/*****************************************************************/

void arith_overflow_func()

/*****************************************************************/
     
{ 
    msg( "Arithmetic overflow !", "", 0 );
}

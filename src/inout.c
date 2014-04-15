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
 

FILENAME: inout.c

AUTHOR: Thomas Christof

REVISED BY MECHTHILD STOER

REVISED BY ANDREAS LOEBEL
           ZIB BERLIN
           TAKUSTR.7
           D-14195 BERLIN

*******************************************************************************/
/*  LAST EDIT: Mon Nov 25 16:43:39 2002 by Andreas Loebel (opt0)  */
/* $Id: inout.c,v 1.6 2009/09/21 07:46:48 bzfloebe Exp $ */


#define _CRT_SECURE_NO_WARNINGS 1


#include "inout.h"
#include "common.h"
#include "arith.h"
#include "log.h"


#define MAXLINE 100000








int scan_line( RAT *rat_in, int type, int d, int line, 
               char fname[], char *in_line, char scanned_inline[] )
/*****************************************************************/
/*
 * Read the string "in_line" 
 * of "d" integers or fractions into the array rat_in
 * in case of type = 0 or 1, fractions are accepted:  1/2  400/5
 * in case of type = 2, only integers are accepted: 1 2 400 5
 * Brackets starting the line are overread. 
 * The statistical part is also overread. 
 * Return with 1.
 *
 * If the type = 3
 * or in_line is not started by brackets or integers or '+' or '-',
 * scan_line2() is used to scan the string in_line.
 * Output is string scanned_inline then.
 * Return with 0 if in_line begins with a number (not counting blanks),
 * with 1 otherwise.
 *
 * Change by M.S. 1.6.92:
 * Overread also tabs.
 */

{
    char ch, *ptr;
    int j,*int_in;
    RAT val;
    
    do
    {
        ch = *(in_line++);
    }  
    while (ch == ' ' || ch == '\t');
    
    /* overread brackets () and text inside */
    if (ch == '(') 
    {
        do 
        {
            ch = *(in_line++);
            if (ch == '\n')
                msg("%s, line %i : unexpected end of line",
                    fname,line);
        }  
        while (ch != ')');
        do 
        {
            ch = *(in_line++);
        } 
        while (ch == ' ' || ch == '\t');
    }
    in_line--;
    
    if ((((ch < '0' || ch > '9') && ch != '+' && ch != '-'))
        || type == 3 ) 
    {
        /* NO POINT-LINE */
        return(scan_line2(line,fname,in_line,scanned_inline));
    }

    int_in = (int *) rat_in;
    
    /* read exactly d fractions or integers */
    for (j = 0; j < d; j++,rat_in++) 
    {
        val.num = (int) strtol(in_line,&ptr,10);
        if (ptr == in_line)
            msg("%s, line %i : invalid format of input file ",
                fname,line);
        /* position in_line after the number */
        in_line = ptr;
        /* read the denominator, if type < 2 */
        if (type < 2) 
        {
            do 
            {
                ch = *(in_line++);
            } 
            while (ch == ' ' || ch == '\t');
            if (ch  == '/') 
            {
                val.den.i = (int) strtol(in_line,&ptr,10);
                if (ptr == in_line || val.den.i <= 0)
                    msg("%s, line %i : invalid denominator",
                        fname,line);
                in_line = ptr;
            }
            else 
            {
                /* default denominator = 1 */
                in_line--;
                (val.den.i = 1);
            }
        }

        if (type == 0)
            *rat_in = val;
        else if (type == 1)
            I_RAT_add(val,*rat_in,rat_in);
        else
            *(int_in+j) = val.num;
    }

    /* 
     * change by M.S. 1.7.92:
     * the following three lines lead to the rejection of
     * "(..) 1 1  # statistical part\n"
     * As for inequality-lines, the comment sign # should also be allowed
     * for point-lines.
     *
     *  for (ch = getc(fp); ch != '\n' ; ch = getc(fp))
     *    if (ch != ' ') 
     *      msg("%sline : %i invalid format of input file ","",line);
     */
    ch = *(in_line++);
    while (ch != '\n'  && ch != '#') 
    {
        if (ch >= '0' && ch <= '9')
            msg("%s, line %i : dimension error",fname,line);
        /* too many numbers on the line */
        else if (ch != ' ' && ch != '\t') 
            msg("%s, line %i : invalid format of input file ",
                fname,line);
        ch = *(in_line++);
    }
    
    return(1);
}










int nstrcmp( char *a, char *b1, char *b2, char *b3, char *b4, char *b5, char *b6, char *b7 )
{
    return(strcmp(a,b1) && strcmp(a,b2) && strcmp(a,b3) && strcmp(a,b4) &&
           strcmp(a,b5) && strcmp(a,b6) && strcmp(a,b7));
} 








int scan_line2( int line, char fname[], char *in_line, char scanned_inline[] )
/*****************************************************************/
/*
 * Former get_line().
 * From a string "in_line",
 * remove all brackets and text inside.
 * Put a '+' in front the first 'x'.
 * Remove all blanks.
 * Output is string scanned_inline.
 * 
 * Change by M.S. 1.6.92:
 * remove also tabs
 *
 * Return 1, if line begins with a non-number (not counting blanks)
 * otherwise return 0.
 */
{
    int i = 0;
    char ch;
    
    ch = *(in_line++);
    
    for (  ; ch != '\n'; ch = *(in_line++)) 
    {
        if (ch == 'x' && i == 0)
            scanned_inline[i++] = '+';
        if (i < MAXLINE-1) 
        {
            /* || (i > 0 ? in_line[i-1] != ' ' : 1)) */
            if (ch != ' ' && ch != '\t') 
                scanned_inline[i++] = ch;
        }
        else 
            msg("%s, line %i : line too long ",fname,line);
    }
    
    scanned_inline[i] = '\0';
    /* scanned_inline ends with "\0", but not with "\n\0" */
    ch = scanned_inline[0];
    if ((ch < '0' || ch > '9') && ch != '+' && ch !=  '-')
        return(0);
    return (1);
}     











int get_line( FILE *fp, char fname[], char in_line[], int *line )
/*****************************************************************/
/*
 * Read a line from input file fp into string in_line,
 * without changing this string.
 * Return 0 if in_line consists only of blanks, tabs, and one newline,
 * otherwise return 1.
 */
{
    char ch;
    char *ptr;
    int i, nonempty;
    
    (*line)++;
    i = 1;
    nonempty = 0;
    ptr = in_line;
    do 
    {
        ch = (char)getc(fp);
        *ptr = (char) ch;
        ptr++;
        if (i++ >= MAXLINE) 
            msg("%s, line %i : line too long ",fname,*line);
        if (ch != ' ' &&  ch != '\t' && ch != '\n')
            nonempty = 1;
    } 
    while ( ch != '\n' && (int) ch != EOF);
    *ptr = '\0';
    if ((int) ch == EOF)
    {
        msg("%s, line %i: invalid format",fname,*line);
        return 0; // dummy
    }
    else
        return (nonempty);
    /* in_line ends with "\n\0" */
}















int read_input_file( char *fname, FILE *outfp, int *dim, RAT **ar, int *nel_ar, 
                     char *intkey1, int **intli1, char *intkey2, int **intli2,
                     char *RATkey1, RAT **RATli1 )
/*****************************************************************/
/*
 * Read the dimension "dim",
 * a table "ar" of points (in case of a .poi file), 
 * or of inequalities (in case of a .ieq file),
 * a line "intli1" of integers occurring after keyword "intkey1",
 * a line "intli2" of integers occurring after keyword "intkey2",
 * a line "RATli1" of rationals occurring after keyword "RATkey1".
 * "ar6" is a point that is output in the VALID section in the .ieq file.
 * 
 * Change by M.S. 5.6.92:
 * If "outfp" exists (e.g. if is_set(Sort)),
 * copy the input file into outfp, 
 * except for the INEQUALITIES_SECTION, the CONV_SECTION, and the CONE_SECTION,
 * and END.
 */
{
    int i,j,ieqs,nonempty,arrows=0,arrowl=0,cone = 0,conv = 0,line;
    int *hip;
    char *in,*end = "END",
    equalities[22],
    convstr[13],conestr[13],key_eli[18],key_val[6],key_low[13],key_upp[13], 
    *comm = "COMMENT" ;
    char scanned_inline[MAXLINE];
    char in_line[MAXLINE];
    RAT val;

    line = 0;
    strcpy (equalities,"INEQUALITIES_SECTION");
    strcpy (convstr,"CONV_SECTION");
    strcpy (conestr,"CONE_SECTION");
    strcpy (key_eli,"ELIMINATION_ORDER");
    strcpy (key_val,"VALID");
    strcpy (key_upp,"UPPER_BOUNDS");
    strcpy (key_low,"LOWER_BOUNDS");
    fp = fopen(fname,"r");
    if (fp == 0)
        msg( "%s : no such file", fname, 0 );
    
    /* 
     * Read the first line into string "in_line". Leading blanks are skipped.
     * In_Line should start with "DIM=".
     */
    
    do 
    {
        nonempty = get_line(fp,fname,in_line,&line );
    } 
    while (!nonempty);
    
    scan_line(&val,3,0,line,fname,in_line,scanned_inline);
    if (strncmp(scanned_inline,"DIM=",4) == 0) 
    {
        in = scanned_inline+4;
        *dim = atoi(in);
        if (*dim == 0) 
            msg("%s, line %i : dimension error",fname,line);
        if (outfp) 
        {
            fprintf(outfp,"%s",in_line);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "%s",in_line);
        }
    }
    ieqs = (strcmp(fname+strlen(fname)-4,".ieq") == 0);
    if( ieqs ) 
    {
        equa = ineq = 0;
        convstr[0] = conestr[0] = '\n';
        arrowl = *dim+2;
        if (!strcmp(intkey1,"ELIMINATION_ORDER") || !strcmp(intkey2,"ELIMINATION_ORDER"))
            key_eli[0] =  '\n'; 
        if (!strcmp(intkey1,"LOWER_BOUNDS") || !strcmp(intkey2,"LOWER_BOUNDS"))
            key_low[0] =  '\n'; 
        if (!strcmp(intkey1,"UPPER_BOUNDS") || !strcmp(intkey2,"UPPER_BOUNDS"))
            key_upp[0] =  '\n'; 
        if (!strcmp(RATkey1,"VALID"))
            key_val[0] =  '\n'; 
    }
    else 
    {
        /* 
         * key_eli, key_low, key_upp, and key_val are set to 0,
         * if they already appear as input parameters to this subroutine.
         * In this way, if e.g. the keyword "VALID" is encountered twice in the 
         * input file, an error message is produced.
         * "convstr" and "conestr" are set to "\n",
         * so that the keywords "CONV_SECTION" and "CONE_SECTION" 
         * result in an error message, if they appear in the .ieq file.
         */
        cone = conv = 0;
        key_val[0] = key_upp[0] = key_low[0] = key_eli[0] = equalities[0] = '\n';
        arrowl = *dim+1;
    }
    
    /* 
     * ar6 is initialized to 0.
     * It is overwritten by a point in the CONV_SECTION,
     * or by a point in the VALID section.
     */
    ar6 = (RAT *) RATallo(ar6,0,*dim);
    for (j = 0; j < *dim; j++) 
    {
        ar6[j].num = 0;
        ar6[j].den.i = 1;
    }
    
    /* Read the next line into string "in_line" and scan it */
    nonempty = get_line(fp,fname,in_line,&line );
    scan_line(&val,3,0,line,fname,in_line,scanned_inline);
    do 
    {
        if (!nonempty) 
        {
            if(outfp)
            {
                fprintf(outfp,"%s",in_line);
                
                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
        }
        else if (strncmp(scanned_inline,comm,7) == 0) 
        {
            /*
             * Overread keyword COMMENT and all following lines,
             * until the next keyword (i.e. one of the arguments of nstrcmp() ) 
             * is encountered.
             */
            do 
            {
                if(outfp)
                { 
                    fprintf(outfp,"%s",in_line);
                    
                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
                nonempty = get_line(fp,fname,in_line,&line);
                scan_line(&val,3,0,line,fname,in_line,scanned_inline);
            } 
            while (nstrcmp(scanned_inline,conestr,equalities,end,
             intkey1,intkey2,RATkey1,convstr) || !nonempty);
        }
        else if (strcmp(scanned_inline,RATkey1) == 0) 
        {
            /*
             * If the keyword RATkey1 is encountered,
             * read the next line of "dim" rationals into array RATli1.
             * RATkey1 is "VALID" for "traf *.ieq" applications.
             */
            if(outfp)
            { 
                fprintf(outfp,"%s",in_line);
            
                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            /*
               When calling read_input_file, RATkey1 is always
               assigned to a const argument (string).
               Therefore it isn't allowed to assign new
               values to this argument. By the way, this
               statement causes a bus error on some architectures
               like the HP!
               18.01.1994, Andreas Loebel
               RATkey1[0] = '\n';
               *RATli1 = (RAT *) RATallo(*RATkey1,0,*dim);
               */
            *RATli1 = (RAT *) RATallo(CP 0,0,*dim);
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
                if(outfp)
                {
                    fprintf(outfp,"%s",in_line);

                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
            } 
            while (!nonempty);
            scan_line(*RATli1,1,*dim,line,fname,in_line,scanned_inline);
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
        }
        else if (strcmp(scanned_inline,key_val) == 0) 
        {
            /*
             * keyval[0] == "VALID", iff
             * Ratkey1 was not "VALID" at the start of function read_input_file()
             * Otherwise keyval[0] = '\n'
             * --> "VALID" is accepted as a keyword at most once.
             */
            if(outfp)
            { 
                fprintf(outfp,"%s",in_line);
                
                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            key_val[0] = '\n';
            /* 
             * Change by M.S.: "ar6" was already allocated earlier,
             * and it is never freed in this loop.
             ar6 = (RAT *) RATallo(ar6,0,*dim);
             */
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
                if(outfp)
                {
                    fprintf(outfp,"%s",in_line);

                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
            } 
            while (!nonempty);
            scan_line(ar6,1,*dim,line,fname,in_line,scanned_inline);
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
            /* change by M.S. 31.5.92: 
             * "ar6" is also used when reading the CONV_SECTION, so don't free it.
             ar6 = (RAT *) RATallo(ar6,*dim,0);
             */
        }
        else if (strcmp(scanned_inline,intkey1) == 0) 
        {
            if(outfp)
            { 
                fprintf(outfp,"%s",in_line);
            
                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            intkey1[0] = '\n';
            *intli1 = (int *) allo(*intli1,0,*dim*sizeof(int));
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
                if(outfp)
                { 
                    fprintf(outfp,"%s",in_line);
                
                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
            } 
            while (!nonempty);
            scan_line((RAT *)*intli1,2,*dim,line,fname,in_line,scanned_inline);
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
        }
        else if (strcmp(scanned_inline,intkey2) == 0) 
        {
            if(outfp)
            { 
                fprintf(outfp,"%s",in_line);

                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            intkey2[0] = '\n';
            *intli2 = (int *) allo(*intli2,0,*dim*sizeof(int));
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
                if(outfp)
                { 
                    fprintf(outfp,"%s",in_line);

                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
            } 
            while (!nonempty);
            scan_line((RAT *)*intli2,2,*dim,line,fname,in_line,scanned_inline);
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
        }
        else if (!strcmp(scanned_inline,key_eli) 
                 || !strcmp(scanned_inline,key_low) 
                 || !strcmp(scanned_inline,key_upp) ) 
        {
            /*
             * array "hip" is just a temporary array.
             */
            if(outfp)
            { 
                fprintf(outfp,"%s",in_line);
                
                /* 17.01.1994: include logging on file porta.log */
                porta_log( "%s",in_line);
            }
            if (!strcmp(scanned_inline,key_eli))   key_eli[0] = '\n';
            if (!strcmp(scanned_inline,key_low))   key_low[0] = '\n';
            if (!strcmp(scanned_inline,key_upp))   key_upp[0] = '\n';
            hip = (int *) allo(CP 0,0,*dim*sizeof(int));
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
                if(outfp)
                { 
                    fprintf(outfp,"%s",in_line);
                    
                    /* 17.01.1994: include logging on file porta.log */
                    porta_log( "%s",in_line);
                }
            } 
            while (!nonempty);
            scan_line((RAT *)hip,2,*dim,line,fname,in_line,scanned_inline);
            nonempty = get_line(fp,fname,in_line,&line);
            scan_line(&val,3,0,line,fname,in_line,scanned_inline);
            hip = (int *) allo(hip,*dim*sizeof(int),0);
        }
        else if (strcmp(scanned_inline, convstr) == 0) 
        {
            /* 
             * Read the CONV_SECTION into array "ar".
             * For all points j=1,...,conv in the CONV_SECTION, 
             * ar[j-1][dim] is set to 1.
             * "arrowl" was set to dim+1 earlier.
             */
            convstr[0] = '\n';
            *ar = (RAT *) RATallo(*ar,arrows*arrowl,
                                  (arrows+INCR_INSYS_ROW)*arrowl);
            arrows += INCR_INSYS_ROW;
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
            } 
            while (!nonempty);
            while (scan_line(*ar+(conv+cone)*arrowl,0,*dim,line,fname,
                             in_line,scanned_inline)) 
            {
                (*ar+(conv+cone)*arrowl+*dim)->num = 1;
                if (conv+cone+2 > arrows) 
                {
                    *ar = (RAT *) RATallo(*ar,arrows*arrowl,
                                    (arrows+INCR_INSYS_ROW)*arrowl);
                    arrows += INCR_INSYS_ROW;
                }
                do 
                {
                    nonempty = get_line(fp,fname,in_line,&line);
                } 
                while (!nonempty);
                conv++;
            }
            /* repeat the last point in array "ar6" */
            for (j = 0; j < *dim; j++)
                ar6[j] = (*ar+(conv+cone-1)*arrowl)[j];
        }
        else if ((strcmp(scanned_inline, conestr) == 0)) 
        {
            /* 
             * Read the CONE_SECTION
             * ar[j-1][dim] is initialized by RATallo to 0.
             */
            conestr[0] = '\n';
            *ar = (RAT *) RATallo(*ar,arrows*arrowl,
                                  (arrows+INCR_INSYS_ROW)*arrowl);
            arrows += INCR_INSYS_ROW;
            do 
            {
                nonempty = get_line(fp,fname,in_line,&line);
            } 
            while (!nonempty);
            while (scan_line(*ar+(conv+cone)*arrowl,0,*dim,line,fname,
                             in_line,scanned_inline)) 
            {
                if (conv+cone+2 > arrows) 
                {
                    *ar = (RAT *) RATallo(*ar,arrows*arrowl,
                                          (arrows+INCR_INSYS_ROW)*arrowl);
                    arrows += INCR_INSYS_ROW;
                }
                do 
                {
                    nonempty = get_line(fp,fname,in_line,&line);
                } 
                while (!nonempty);
                cone++;
            }
        }
        else if ( (strcmp(scanned_inline, equalities) == 0)) 
        {
            /*
             * Read the INEQUALITIES_SECTION
             * "arrowl" was set to dim+2 earlier.
             */
            equalities[0] = '\n';      
            *ar = (RAT *) RATallo(*ar,arrows*arrowl,
                                  (arrows+INCR_INSYS_ROW)*arrowl);
            arrows += INCR_INSYS_ROW;
            read_eqie(ar,*dim,&equa,&ineq,&arrows,&line,
                      fname,in_line,scanned_inline);
            nonempty = 1;
        }
        else if (strcmp(scanned_inline,end) == 0)
        {
            i =  (ieqs) ? equa+ineq : conv+cone;
            
            *nel_ar = (i+1)*arrowl;
            
            *ar = (RAT *) RATallo(*ar,arrows*arrowl,*nel_ar);
            
            fclose(fp);
            
            fprintf(prt,"input file %s o.k.\n",fname);
            fprintf(prt,  "dimension              : %4i \n",*dim);

            /* 17.01.1994: include logging on file porta.log */
            porta_log( "input file %s o.k.\n",fname);
            porta_log( "dimension              : %4i \n",*dim);

            if (ieqs) 
            {
                fprintf(prt,"number of equations    : %4i \n",equa);
                fprintf(prt,"number of inequalities : %4i \n\n",ineq);

                /* 17.01.1994: include logging on file porta.log */
                porta_log( "number of equations    : %4i \n",equa);
                porta_log( "number of inequalities : %4i \n\n",ineq);
            }
            else 
            {
                fprintf(prt,"number of cone-points  : %4i \n",cone);
                fprintf(prt,"number of conv-points  : %4i \n\n",conv);

                /* 17.01.1994: include logging on file porta.log */
                porta_log( "number of cone-points  : %4i \n",cone);
                porta_log( "number of conv-points  : %4i \n\n",conv);
            }
            
            return (i);
            
        }
        
        else
            msg("%s, line %i : invalid format",fname,line);
        
    } 
    while (1);
    
}
        










void read_eqie( RAT **ar, int dim, int *equa, int *ineq, int *maxrows, int *line,
               char *fname, char in_line[], char *scanned_inline )
/*****************************************************************/
/*
 * Read the INEQUALITY_SECTION into the table "ar".
 * For each (in)equality, the corresponding line of "ar" contains, in this order:
 *    the "dim" coefficients of the variables x1--xn,
 *    the right-hand side,
 *    0 if it was an equation, 1 if it was an inequality.
 * All inequalities are stored as "<=" inequalities.
 * Lines as
 *   "x1 - 1 <= 3 + x2"
 * are interpreted as
 *   "x1 -x2 <= 4"
 */
{
    char *p,*in;
    int i,rs=0,index=0,j,sysrow, numberread,nonempty;
    RAT val;

    sysrow = dim+2; /* row length of array "ar" */
    do 
    {
        nonempty = get_line(fp,fname,in_line,line);
    } 
    while (!nonempty);
    for (i = 0; scan_line(&val,3,dim,*line,
                          fname,in_line,scanned_inline);
         i++) 
    {
        /* 
         * scan_line() reads an inequality into string "scanned_inline",
         * with a little formatting.
         * "val" is not used.
         */
        p = in = scanned_inline;
        rs = 0;     /* rs records the type of inequality,
                     * rs = 0 means: type not yet known. */
        
        /* scan string "scanned_inline" up to '\0' or '#' */
        while (*p != '\0' && *p != '#' ) 
        {
            
            val.den.i = 1;
            val.num = 1;
            if (*p == '-')
                val.num = -1;
        
            /* 
             * If: {+,-} has been read,
             *     or {<,=,>} followed by {x,0,1,..,9} has been read,
             *     or it is the beginning of the line. then ...
             */
            if ( *p == '-' || *p == '+'  || 
                (p == scanned_inline || 
                 (p != scanned_inline && (*(p-1)=='<' || *(p-1)=='=' || *(p-1)=='>')) 
                 && ((*p < 58 && *p > 47) || *p =='x') )     ) 
            {
                numberread = 0;
                if ( *p == '-' || *p == '+')    p++;
                in = p;
                while (*p > 47 && *p < 58)
                    p++;
                if (in != p) 
                {
                    /* in points to a number, p to the next non-number */
                    numberread = 1;
                    val.num *= atoi(in);
                    if (*p == '/') 
                    {
                        p++;
                        in = p;
                        while (*p > 47 && *p < 58) p++;
                        if (p == in)
                            msg("%s, line %i : invalid denominator",
                                fname,*line);
                        if ((val.den.i = atoi(in)) <= 0)
                            msg("%s, line %i : invalid denominator",
                                fname,*line);
                    }   
                } /* if (in != p) */

                if (*p == 'x') 
                {
                    p++;
                    in = p;
                    while (*p > 47 && *p < 58) p++;
                    if (p == in) 
                        msg("%s, line %i : invalid format",fname,*line);
                    index = atoi(in)-1;
                    if (index > dim-1 || index < 0)
                        msg("%s, line %i : only variable names x1,...,xdim allowed",
                            fname,*line);
                }
                /* 
                 * Change by M.S. 1.6.92:
                 * The following four lines of code are changed.
                 *
                 * In the case (*p in {+,-}), 
                 * something like "10 +", but not "x10 +" has been encountered.
                 * In the case (rs==0 && *p in {<,>,>} ),
                 * something like "10 <", but not "x10 <" has been encountered.
                 * In both cases, the number "10" is interpreted as part of
                 * the right-hand side.
                 * But it should be tested, 
                 * whether a number has actually been read.
                 * Otherwise
                 * "x1++x2 <= 20\n"  is interpreted as
                 * "x1+1+x2 <= 20\n".
                 */
                else if (numberread && (*p == '+' || *p == '-'  || 
                          ((rs == 0) && (*p =='<'||*p == '>'||*p == '=')))) 
                {
                    index = dim;
                }
                /* The following four lines mean:
                 * If "<=" or ">=" or "==" has already been read (i.e. rs > 0),
                 * and if p points to the end of the line,
                 * then interpret the number in val as the right hand side.
                 * to be stored in the dim-th position of the current row
                 * of table "ar".
                 */
                else if (numberread && rs && (*(p) == '\0' || *(p) == '#'))  
                {
                    index = dim;
                }
                else 
                    msg("%s, line %i : invalid format",fname,*line);
            } /* if ......... */

            else if ((p != scanned_inline) 
                     && (*p == '<' || *p == '>' || *p == '=')) 
            {
                /* 
                 * Record the type of inequality:
                 * rs = 1   if it is an equation ("=" or "==")
                 * rs = 2   if it is a "<=" inequality (may also be written "=<")
                 * rs = 3   if it is a ">=" inequality (may also be written "=>")
                 */
                if (++rs > 1) 
                    msg("%s, line %i : invalid format",fname,*line);
                if ((*p == '=') && (*(p+1) == '='))
                    p++;
                else if ((*p == '=') && (*(p+1) != '>') && (*(p+1) != '<'));
                else if ((*p == '=' && *(p+1) == '>') ||
                         (*p == '>' && *(p+1) == '=')) 
                {
                    rs++;rs++;
                    p++;
                }
                else if ((*p == '=' && *(p+1) == '<') ||
                         (*p == '<' && *(p+1) == '=')) 
                {
                    rs++; 
                    p++;
                }
                else
                    msg("%s, line %i : invalid format",fname,*line);
                p++;
                index = -1;
            } /* else if < = > */
            
            else 
                msg("%s, line %i : invalid format",fname,*line);
            
            if (index > -1)  
            { /* not "< > =" */ 
                if (index == dim) val.num *= -1;
                if (rs ) 
                    I_RAT_sub(*(*ar+i*sysrow+index),val,*ar+i*sysrow+index);
                else
                    I_RAT_add(*(*ar+i*sysrow+index),val,*ar+i*sysrow+index);
            }
            
        } /* while */
        
        /* transform ">=" into "<=" by multiplying the inequality with -1 */
        if (rs == 3 ) 
        {  /* >= */
            for (j = 0; j <= dim; j++)
                (*ar+i*sysrow+j)->num = -(*ar+i*sysrow+j)->num;
            rs -= 1;
        }
        (*ar+(i+1)*sysrow-1)->num  =  --rs; 
    
        /* 
         * Now rs = 0 if the line was an equation,
         * and rs = 1 if the line was an inequality.
         * The last line stored rs into the "dim+1"th position of ar[i].
         */
        (rs == 1) ? (*ineq)++ : (*equa)++;
        
        if ((*ineq)+(*equa)+2 > *maxrows) 
        {
            *ar = (RAT *) RATallo(*ar,(*maxrows)*(dim+2),(*maxrows+INCR_INSYS_ROW)*(dim+2));
            *maxrows += INCR_INSYS_ROW;
        }
        do 
        {
            nonempty = get_line(fp,fname,in_line,line);
        } 
        while (!nonempty);
    }  /* for i */
}






int ntemp=0;


#include <sys/types.h>
#include <sys/stat.h>


FILE *wfopen( char *fname )
{
    
    struct stat statbuf;
    char command[BUFSIZ];
    
    if( !stat(fname,&statbuf) ) 
    {
#if defined WIN32
        if( fname[0] == '\\' )
        {
            fprintf( prt, "cannot make bakup file of old %s\n", fname );
            porta_log( "cannot make bakup file of old %s\n", fname );
        }
        else
        {
            fprintf(prt,"%s moved into %s.bak\n", fname, fname );
            porta_log( "%s moved into %s.bak\n", fname, fname );
            
            sprintf( command, "%s.bak", fname );
            if( !stat( command, &statbuf ) ) 
            {
                sprintf( command, "del %s.bak", fname );
                system( command );
            }

            sprintf( command, "ren %s %s.bak", fname, fname );
            system( command );
        }
#else // WIN32
        fprintf(prt,"%s moved into %s%c\n",fname,fname,'%');
        porta_log( "%s moved into %s%c\n",fname,fname,'%');

        sprintf( command, "mv -f %s %s%c", fname, fname, '%');
        system( command );
#endif // WIN32
    }

    return( fopen(fname,"w") );
    
}








void write_ieq_file( char *fname, FILE *fp, int equa, int feq, int eqrl, 
                     int *eqindx, int ineq, int fie, int ierl, int *ieindx )
{
    char filename[100];
    int i, start;
    
    fprintf(prt,"\nnumber of equations    : %4i \n",equa);
    fprintf(prt,"number of inequalities : %4i \n\n",ineq);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\nnumber of equations    : %4i \n",equa);
    porta_log( "number of inequalities : %4i \n\n",ineq);
    
    strcpy(filename,fname);
    strcat(filename,".ieq");
    
    if (!fp) 
    {
        fp = wfopen(filename);
        fprintf(fp, "DIM = %d\n\n",dim);
        
        if (is_set(Traf) ) 
        {
            fprintf(fp, "VALID\n");
            for (i = 0; i< dim; i++) 
            {
                fprintf(fp, "%ld", ar6[i].num);
                if (ar6[i].den.i > 1)
                    fprintf(fp, "/%i ", ar6[i].den.i);
                else
                    fprintf(fp, " ");
            }
            fprintf(fp, "\n\n");
        }
    }
    
    fprintf(fp, "INEQUALITIES_SECTION\n");

    start=1;
    if (equa) 
    {
        writesys(fp,feq,feq+equa,eqrl,0,eqindx,'=',&start);
        fprintf(fp,"\n");
    }
    start=1;
    if (ineq) writesys(fp,fie,fie+ineq,ierl,0,ieindx,'<',&start);
    fprintf(fp,"\n");
    
    fprintf(fp, "END\n");
    
    if (is_set(Validity_table_out)) 
        writepoionie(fp,fie,fie+ineq,points,0); 
    
    fclose(fp);
    
    fprintf(prt,"output written to file %s\n\n",filename);

    /* 17.01.1994: include logging on file porta.log */
    porta_log( "output written to file %s\n\n",filename);
}









void write_poi_file( char *fname, FILE *fp, int dim, int lr, int flr, 
                     int cone, int fce, int conv, int fcv )
{
    char filename[100];
    int i,j,start;
    
    fprintf(prt,"\nnumber of cone-points  : %4i \n",cone+2*lr);
    fprintf(prt,"number of conv-points  : %4i \n\n",conv);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "\nnumber of cone-points  : %4i \n",cone+2*lr);
    porta_log( "number of conv-points  : %4i \n\n",conv);
    
    strcpy(filename,fname);
    strcat(filename,".poi");          
    
    if (!fp) 
    {
        fp = wfopen(filename);
        fprintf(fp, "DIM = %d\n\n",dim);
    }
    
    start = 1;
    if (cone > 0 || lr > 0)  
    {
        fprintf(fp, "CONE_SECTION\n");
        if (lr > 0) 
        {   
            /* CONE(xi,-xi,....) */
            writesys(fp,flr,flr+lr,dim,1,0,' ',&start);
            for (i = flr; i < flr+lr; i++)
                for (j = 0; j < dim; j++) 
                    (porta_list[i]->sys+j)->num *= -1;
            writesys(fp,flr,flr+lr,dim,1,0,' ',&start);
            for (i = flr; i < flr+lr; i++)
                for (j = 0; j < dim; j++) 
                    (porta_list[i]->sys+j)->num *= -1;
        }
        writesys(fp,fce,fce+cone,dim,1,0,' ',&start);
        fprintf(fp,"\n");
    }
    
    if (conv > 0) 
    {
        start = 1;
        fprintf(fp, "CONV_SECTION\n");
        writesys(fp,fcv,fcv+conv,dim,1,0,' ',&start);
        fprintf(fp,"\n");
    }
    fprintf(fp, "END\n");
    
    if (is_set(Validity_table_out)) 
        writepoionie(fp,0,ineq,points-1,1); 
    
    fprintf(prt,"output written to file %s\n\n",filename);
    
    /* 17.01.1994: include logging on file porta.log */
    porta_log( "output written to file %s\n\n",filename);
    
    fclose(fp);
}

  







void max_vals( RAT *max, RAT *ptr, int col )
{ 
    int j,k;
    
    for (j = 0; j < col; j++) 
    {
        k = (max[j].num < 0) ? -1 : 1; 
        if (abs(ptr[j].num) > abs(max[j].num)) 
            max[j].num = abs(ptr[j].num);
        
        if (ptr[j].num < 0 || k < 0)
            max[j].num = -abs(max[j].num);
        
        if (ptr[j].den.i > max[j].den.i) 
            max[j].den.i = ptr[j].den.i;
    }
}










void width_line( RAT *max, int col, int format )
{ 
    int i,j,k;
    
    for (k = 0; k < col; k++) 
    {
        if (abs(max[k].num) == 1 && (max[k].den.i == 1)) 
            max[k].num = (format == 0 && k!= col-1) ? 0 : ((max[k].num < 0)?2:1);
        else if (!format && max[k].num ==0 && col != k-1)
            max[k].num = -5; 
        else 
        {
            for (i = 1, j = 1; abs(max[k].num)/j > 0; i++, j *= 10);
            if ((format) || (k == col-1))
                max[k].num =  (max[k].num > 0) ? i-1 : i ;
            else
                max[k].num = i-1;
        }
        if (max[k].den.i == 1) 
            max[k].den.i = -1;
        else 
        {
            for (i = 1, j = 1; max[k].den.i/j > 0; i++, j *= 10);
            max[k].den.i = i-1;
        }
    }
}







RAT *max;

void writesys( FILE *fp, int frow, int lrow, int rowl, int format, 
               int *indx, char eqie, int *start )
{
    int i,j;
    
    if (!MP_realised) 
    {
        max = (RAT *) RATallo(CP 0,0,U rowl);
        for (j = 0; j < rowl; j++) 
            max[j].num =  max[j].den.i = 0;
        
        for (i = frow; i < lrow; i++)
            max_vals( max, porta_list[i]->sys, rowl );
        
        width_line(max,rowl,format);
    }
    
    for (i = frow; i < lrow; i++)
    {
        /*
           fprintf(fp,"(%3d) ",i-frow+1);   
        */
        fprintf(fp,"(%3d) ",*start);
        (*start)++;     
        (* writeline)(fp,rowl,porta_list[i]->sys,format,max,eqie,indx);
        if (option & Statistic_of_coefficients) 
            writestatline(fp,(int *)porta_list[i]->ptr);
        fprintf(fp,"\n");
    } /* for i */
/*
    fprintf(fp,"\n");
*/
    if (!MP_realised) RATallo(max,rowl,0);
}









void writestatline( FILE *fp, int *ptr )
{
    int j;
    
    fprintf(fp,"# -5..-1,1..5 :");
    for (j = -5; j < 0; j++)
        fprintf(fp, " %2d", *(ptr+j));
    fprintf(fp,"   ");
    for (j = 1; j < 6; j++)
        fprintf(fp, " %2d", *(ptr+j));
    
}









void I_RAT_writeline( FILE *fp, int col, RAT *ptr, int format, 
                      RAT *max, char ie_eq, int *indx )
{
    int j,l, ind = 0;
    
    if ( format == 0) 
    {
        
        for (j = 0; j < col; j++)
        {
            
            if (j != col-1) 
            {
                if (!indx)
                    ind = j+1;
                else
                    ind = indx[j]+1;
            }
            
            if  (((ptr+j)->num == 0 && j != col-1 )) 
            {
                for (l = 0; l <= (max?( max[j].num+max[j].den.i+3):-1); l++) 
                    fprintf(fp," ");
                if (max && ind > 99)  fprintf(fp," ");
                if (max && ind  > 9) fprintf(fp," ");
            }
            
            else if (j == col-1 ) 
            {   /* right hand side */
                fprintf(fp," %c= ",ie_eq);
                if ((ptr+j)->den.i == 1)
                    fprintf( fp, "%*ld", 
                             (int)(max ? max[j].num : 1),
                             (ptr+j)->num );
                else
                    fprintf( fp,"%*ld/%*d   ", 
                             (int)(max ? max[j].num : 1), (ptr+j)->num, 
                             (int)(max ? max[j].den.i : 1), (ptr+j)->den.i );
            } 
            
            else 
            {
                
                if ((ptr+j)->num < 0)
                    fprintf(fp,"-");
                else if ((ptr+j)->num > 0)
          fprintf(fp,"+");
                
                if ((abs((ptr+j)->num) == 1) && ((ptr+j)->den.i == 1)) 
                {
                    if (max)  
                        for (l = 0; l < max[j].num+max[j].den.i+1; l++) 
                            fprintf(fp," ");
                }
                else if ((ptr+j)->den.i == 1)
                    fprintf( fp, "%*d",
                             (int)(max ? (max[j].num+max[j].den.i+1) : 1),
                             abs((ptr+j)->num) );
                else
                    fprintf( fp, "%*d/%*d",
                             (int)(max ? max[j].num : 1), abs((ptr+j)->num),
                             (int)(max ? max[j].den.i : 1), (ptr+j)->den.i);
                
                fprintf(fp,"x%i",(indx)?(indx[j]+1):j+1);
            } /* else */
            
        } /* for j */
    } 
    
    else /* matrix format */
        
        for (j = 0; j < col; j++)
        {
            
            if (max && (ptr+j)->den.i == 1) 
                for (l = 0; l < max[j].den.i+1; l++)
                    fprintf(fp," ");
            
            fprintf( fp, "%*ld", (int)(max ? max[j].num : 1), (ptr+j)->num );
            if ((ptr+j)->den.i != 1)
                fprintf( fp, "/%*d", 
                         (int)(max ? max[j].den.i : 1), (ptr+j)->den.i);
            fprintf(fp," ");
        }
    
    
}








void writepoionie( FILE *fp, int fineq, int lineq, int points, int poi_ieq )
{ 
  int i,ie,j,k,out,kk,*sumie,max;
  char *colstr = "POINTS",*rowstr = "INEQS ",*h;
  
  if (poi_ieq) 
  {
      h = colstr;
      colstr = rowstr;
      rowstr = h;
  }
  
  sumie = (int*) allo(CP NULL,0,U points*sizeof(int),0);
  for (i = 0 ; i < points; i++) sumie[i] = 0;
  
  fprintf(fp,"\nstrong validity table : \n");
  for (j = 0 ; j < 8; j++) {
  switch(j) 
  {
  case 0 : fprintf(fp,"\\ %c      |",*colstr++);break;
  case 1 : fprintf(fp," \\ %c     |",*colstr++); break;
  case 2 : fprintf(fp,"%c \\ %c    |",*rowstr++,*colstr++);break;
  case 3 : fprintf(fp," %c \\ %c   | ",*rowstr++,*colstr++);break;
  case 4 : fprintf(fp,"  %c \\ %c  |",*rowstr++,*colstr++);break;
  case 5 : fprintf(fp,"   %c \\ %c |",*rowstr++,*colstr++);break;
  case 6 : fprintf(fp,"    %c \\  |",*rowstr++);break;
  case 7 : fprintf(fp,"     %c \\ |",*rowstr++);break;
  }
  /*  fprintf(fp,"\n"); */
  
  if (j != 3) 
  {
      for (i = 0; i < points+(points-1)/5+2;i++) fprintf(fp," ");
      fprintf(fp,"|\n");
  }
  else 
  {
      for (i = 1; i <= points; i += 5)
          fprintf(fp,"%-*d",(points/(i+5)) ? 6 : ((points%5) ? points%5+1: 6),i);
      fprintf(fp,"| #\n");
  } 
}
  
  for (i = 0; i < points+(points-1)/5+16;i++) fprintf(fp,"-");
  fprintf(fp,"\n");
  
  for (i = fineq, ie = 0; i < lineq; i++, ie++) 
  {
      fprintf(fp,"%-9d| ",ie+1);
      writemark( fp, porta_list[i]->mark, points, sumie );
  }
  
  fprintf(fp,"          ");
  for (i = 0; i < points+(points-1)/5+6;i++) fprintf(fp,".");
  fprintf(fp,"\n");
  max = 0;
  for (i = 0; i < points; i++)
      if (sumie[i] > max) max = sumie[i];
  for (j = 0; max > 0; max /= 10,j++) 
  {
      (j == 0) ? fprintf(fp,"#        | ") : fprintf(fp,"         | ");
      for (i = 0; i < points; i++) 
      {
          if ((out = sumie[i]) > 0) 
          {
              for (k = 10000; sumie[i]/k == 0 && k>9; k /= 10);
              for (kk = 0; kk < j && k > 9;kk++, k /= 10)
                  out = out%k;
              fprintf(fp,"%d",out/k);
              if (k == 1) sumie[i] = -1 ;
          }
          else fprintf(fp," ");  
          if ((i+1)%5 == 0) fprintf(fp," ");
      }
      fprintf(fp,"\n");
  }
  fprintf(fp,"\n");
  
#if defined WIN32 || defined __CYGWIN32__ || defined __APPLE__
  free(sumie);
#else // WIN32
  cfree(sumie);
#endif
}









void writemark( FILE *fp, unsigned *ptr, int n, int *sumie )
{
    int b,i,sumpoi;
    unsigned m,out;
    
    out = sumpoi = 0;
    for (b = 0 ; b < n/32+1 ; b++) 
    {
        m = ptr[b];
        for (i = 0; i < ((b < n/32) ? 32 : n%32); i++) 
        {
            out = m & 1;
            if (out)
                fprintf(fp,"*");
            else
                fprintf(fp,".");
            if (out == 1) 
            {
                sumpoi++;
                if (sumie != 0)
                    *(sumie+b*32+i) += 1;
            }
            if ((b*32+i+1) % 5 == 0 && (b*32+i+1) != n) fprintf(fp," ");
            m >>= 1;
        }
    }
    fprintf(fp," :%3d\n",sumpoi);
}

/*  DinfDistDownmn main program to compute distance to stream in DEM 
   based on D-infinity flow direction model.
     
  David Tarboton, Teklu K Tesfa
  Utah State University  
  May 23, 2010 
  
*/

/*  Copyright (C) 2010  David Tarboton, Utah State University

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License 
version 2, 1991 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the full GNU General Public License is included in file 
gpl.html. This is also available at:
http://www.gnu.org/copyleft/gpl.html
or from:
The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
Boston, MA  02111-1307, USA.

If you wish to use or incorporate this program (or parts of it) into 
other software that does not meet the GNU General Public License 
conditions contact the author to request permission.
David G. Tarboton  
Utah State University 
8200 Old Main Hill 
Logan, UT 84322-8200 
USA 
http://www.engineering.usu.edu/dtarb/ 
email:  dtarb@usu.edu 
*/

//  This software is distributed from http://hydrology.usu.edu/taudem/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonlib.h"
#include "tardemlib.h"
#include "dinfdistdown.h"

int main(int argc,char **argv)
{
char angfile[MAXLN],felfile[MAXLN],slpfile[MAXLN],wfile[MAXLN],dtsfile[MAXLN],srcfile[MAXLN];
   int err,i,statmethod=0,typemethod=0,usew=0, concheck=1;
      
   if(argc < 2)
    {  
	   printf("Error: To run this program, use either the Simple Usage option or\n");
	   printf("the Usage with Specific file names option\n");
	   goto errexit;
    }
    else if(argc > 2)
	{
		i = 1;
//		printf("You are running %s with the Specific File Names Usage option.\n", argv[0]);
	}
	else {
		i = 2;
//		printf("You are running %s with the Simple Usage option.\n", argv[0]);
	}
	while(argc > i)
	{
		if(strcmp(argv[i],"-ang")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(angfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-fel")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(felfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-slp")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(slpfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-src")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(srcfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-wg")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(wfile,argv[i]);
				usew=1;
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-dd")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(dtsfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-m")==0)
		{
			i++;
			if(argc > i)
			{
				if(strcmp(argv[i],"h")==0)
				{
					typemethod=0;
				}
				else if(strcmp(argv[i],"v")==0)
				{
					typemethod=1;
				}
				else if(strcmp(argv[i],"p")==0)
				{
					typemethod=2;
				}
				else if(strcmp(argv[i],"s")==0)
				{
					typemethod=3;
				}
				if(strcmp(argv[i],"ave")==0)
				{
					statmethod=0;
				}
				else if(strcmp(argv[i],"max")==0)
				{
					statmethod=1;
				}
				else if(strcmp(argv[i],"min")==0)
				{
					statmethod=2;
				}
				//else goto errexit;
				i++;
				if(strcmp(argv[i],"h")==0)
				{
					typemethod=0;
				}
				else if(strcmp(argv[i],"v")==0)
				{
					typemethod=1;
				}
				else if(strcmp(argv[i],"p")==0)
				{
					typemethod=2;
				}
				else if(strcmp(argv[i],"s")==0)
				{
					typemethod=3;
				}
				if(strcmp(argv[i],"ave")==0)
				{
					statmethod=0;
				}
				else if(strcmp(argv[i],"max")==0)
				{
					statmethod=1;
				}
				else if(strcmp(argv[i],"min")==0)
				{
					statmethod=2;
				}
				i++;
			}
			else goto errexit;
		}		
	   else if(strcmp(argv[i],"-nc")==0)
		{
			i++;
			concheck=0;
		}
		else 
		{
			goto errexit;
		}
	}
	if( argc == 2) {
		nameadd(angfile,argv[1],"ang");
		nameadd(felfile,argv[1],"fel");
		nameadd(slpfile,argv[1],"slp");
		nameadd(srcfile,argv[1],"src");
		nameadd(wfile,argv[1],"wg");
		nameadd(dtsfile,argv[1],"dd");
	}   
   
if((err=dinfdistdown(angfile,felfile,slpfile,wfile,srcfile,dtsfile,statmethod,
   typemethod,usew, concheck)) != 0)
        printf("area error %d\n",err);   

//	int er;
//switch (typemethod)
//{
//case 0://horizontal distance to stream
//	er=hdisttostreamgrd(angfile,wfile,srcfile,dtsfile,statmethod, 
//		usew, concheck);
//break;
//case 1://vertical drop to stream
//	er=vdroptostreamgrd(angfile,felfile,srcfile,dtsfile, 
//		statmethod);
//break;
//case 2:// Pythagoras distance to stream
//	er=pdisttostreamgrd(angfile,felfile,wfile,srcfile,dtsfile, 
//					statmethod,usew,concheck);
//break;
//case 3://surface distance to stream
//	er=sdisttostreamgrd(angfile,felfile,slpfile,wfile,srcfile,dtsfile, 
//					statmethod,usew,concheck);
//break;
//}

	return 0;

	errexit:
	   printf("Simple Usage:\n %s <basefilename>\n",argv[0]);
	   printf("Usage with specific file names:\n %s -ang <angfile>\n",argv[0]);
       printf("-fel <felfile> -slp <slpfile> -src <srcfile> [-wg <wfile>] -dd <dtsfile>\n");
  	   printf("[-m ave h] [-nc]\n");
	   printf("<basefilename> is the name of the raw digital elevation model\n");
	   printf("<angfile> is the D-infinity flow direction input file.\n");
	   printf("<felfile> is the pit filled or carved elevation input file.\n");
	   printf("<slpfile> is the D-infinity slope input file.\n");
	   printf("<srcfile> is the stream raster input file.\n");
	   printf("<wgfile> is the D-infinity flow direction input file.\n");
	   printf("<dtsfile> is the D-infinity distance output file.\n");
	   printf("[-m ave h] is the optional method flag.\n");
	   printf("The flag -nc overrides edge contamination checking\n");
	   printf("The following are appended to the file names\n");
       printf("before the files are opened:\n");
       printf("ang   D-infinity contributing area file (output)\n");
	   printf("fel   pit filled or carved elevation file\n");
	   printf("slp   D-infinity slope input file file\n");
	   printf("src   Stream raster input file\n");
	   printf("wg   weight input file\n");
	   printf("dd   distance to stream output file\n");
       exit(0);
} 


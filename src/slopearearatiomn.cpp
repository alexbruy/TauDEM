/*  SlopeAreaRatiomn main program to compute slope area ratio.
     
  David Tarboton, Teklu K. Tesfa
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

int main(int argc,char **argv)
{
   char slopefile[MAXLN],areafile[MAXLN],atanbfile[MAXLN];
   int err,i;
   
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
		if(strcmp(argv[i],"-sca")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(areafile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-slp")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(slopefile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-sar")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(atanbfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else 
		{
			goto errexit;
		}
	}
	if( argc == 2) {
		nameadd(areafile,argv[1],"sca");
		nameadd(slopefile,argv[1],"slp");
		nameadd(atanbfile,argv[1],"sar");
	}  
	if((err=atanbgrid(slopefile,areafile,atanbfile)) != 0)
        printf("Slope area ratio error %d\n",err);

	return 0;

	errexit:
	   printf("Simple Usage:\n %s <basefilename>\n",argv[0]);
	   printf("Usage with specific file names:\n %s -sca <areafile>\n",argv[0]);
       printf("-slp <slopefile> -sar <atanbfile>\n");
	   printf("<basefilename> is the name of the raw digital elevation model\n");
	   printf("<areafile> is the D-infinity specific catchment area input file.\n");
	   printf("<slopefile> is the D-infinity slope input file.\n");
	   printf("<atanbfile> is the slope area ratio output file.\n");
	   printf("The following are appended to the file names\n");
       printf("before the files are opened:\n");
       printf("sca    D-infinity specific catchment area grid (input)\n");
	   printf("slp     D-infinity slope grid (input)\n");
	   printf("sar    output slope area ratio grid\n");
       exit(0);
}

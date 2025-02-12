/*  Gridnetmn main program to compute grid network order and upstream 
    lengths based on D8 model  
  
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


int main(int argc,char **argv)
{
   char pfile[MAXLN],plenfile[MAXLN],tlenfile[MAXLN],gordfile[MAXLN],datasrc[MAXLN],lyrname[MAXLN],maskfile[MAXLN];
   int err,useOutlets=0,uselyrname=0,lyrno=0,useMask=0,thresh=0,i;

   if(argc < 2)
    {  	
	   printf("Error: To run this program, use either the Simple Usage option or\n");
	   printf("the Usage with Specific file names option\n");
	   goto errexit;
    }


	if(argc > 2)
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
		if(strcmp(argv[i],"-p")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(pfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-plen")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(plenfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if(strcmp(argv[i],"-tlen")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(tlenfile,argv[i]);
				i++;
			}
		}
		else if(strcmp(argv[i],"-gord")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(gordfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		 else if(strcmp(argv[i],"-o")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(datasrc,argv[i]);
				useOutlets = 1;	
				i++;											
			}
			else goto errexit;
		}


		   else if(strcmp(argv[i],"-lyrno")==0)
		{
			i++;
			if(argc > i)
			{
				sscanf(argv[i],"%d",&lyrno);
				i++;											
			}
			else goto errexit;
		}

	   
	 else if(strcmp(argv[i],"-lyrname")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(lyrname,argv[i]);
		        uselyrname = 1;
				i++;											
			}
			else goto errexit;
		}



		else if(strcmp(argv[i],"-mask")==0)
		{
			i++;
			if(argc > i)
			{
				strcpy(maskfile,argv[i]);
				useMask=1;
				i++;
				if(argc>i && strcmp(argv[i],"-thresh")==0)
				{
					i++;
					sscanf(argv[i],"%d",&thresh);
					i++;
				}
				else goto errexit;
			}
			else goto errexit;
		}
		else 
		{
			goto errexit;
		}
	}
	if( argc == 2) {
		nameadd(pfile,argv[1],"p");
		nameadd(plenfile,argv[1],"plen");
		nameadd(tlenfile,argv[1],"tlen");
		nameadd(gordfile,argv[1],"gord");

	}   

    if( (err=gridnet(pfile,plenfile,tlenfile,gordfile,maskfile,datasrc,lyrname,uselyrname,lyrno,useMask, useOutlets, thresh )) != 0)
        printf("gridnet error %d\n",err);

	return 0;

errexit:
	   printf("Simple Usage:\n %s <basefilename>\n",argv[0]);
	   printf("<basefilename> is the name of the raw digital elevation model\n");
	   printf("The following are appended to the file names\n");
	   printf("before the files are opened:\n");
	   printf("p   D8 flow direction output file\n");
	   printf("plen   the longest flow length upstream of each point output file.\n");
	   printf("tlen   the total path length upstream of each point output file.\n");
	   printf("gord   the grid of strahler order output file.\n\n");

	   printf("Usage with specific file names:\n %s -p <pfile>\n",argv[0]);
       printf("-plen <plenfile> -tlen <tlenfile> -gord <gordfile> [-o <outletfine>] [-lyrname <layer name>] [-lyrno <layer number>] [-mask <maskfile> [-thresh <threshold>]]\n");   
	   printf("<pfile> is the D8 flow direction input file.\n");
	   printf("<plenfile> is the longest flow length upstream of each point output file.\n");
	   printf("<tlenfile> is the total path length upstream of each point output file.\n");
	   printf("<gordfile> is the grid of strahler order output file.\n");
	   printf("[-o <outletfile>] is the optional outlet shape input file.\n");
	   printf("[-lyrname <layer name>] OGR layer name if outlets are not the first layer in outletfile (optional).\n");
	   printf("[-lyrno <layer number>] OGR layer number if outlets are not the first layer in outletfile (optional).\n");
	   printf("Layer name and layer number should not both be specified.\n");
       printf("[-mask <maskfile> [-thresh <threshold>]].  maskfile is an optional mask grid input file.\n");
	   printf("Where a maskfile is input, there is an additional option to specify a threshold.  This has to");
	   printf("be immediately following the maskfile on the argument list.  The grid network is evaluated for");
	   printf("grid cells where values of the maskfile grid read as 4 byte integers are >= threshold.");
	   
       exit(0);
} 


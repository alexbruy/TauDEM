/*  DinfDistUp function to compute distance to ridge in DEM 
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

#include <mpi.h>
#include <math.h>
#include <queue>
#include "commonlib.h"
#include "linearpart.h"
#include "createpart.h"
#include "tiffio.h"
#include "dinfdistup.h"
#include "initneighbor.h"
using namespace std;


// The old program was written in column major order.
// d1 and d2 were used to translate flow data (1-8 taken from the indeces)
// to direction in the 2d array.  We switched d1 and d2 to create it in row major order
// This may be wrong and need to be changed
//const short d1[9] = { 0,0,-1,-1,-1, 0, 1,1,1};
//const short d2[9] = { 0,1, 1, 0,-1,-1,-1,0,1};
// moved to commonlib.h

float **dist;

//Calling function
int dinfdistup(char *angfile,char *felfile,char *slpfile,char *wfile, char *rtrfile,
			   int statmethod,int typemethod,int usew, int concheck, float thresh)
{
	int er;
switch (typemethod)
{
case 0:
	er=hdisttoridgegrd(angfile,wfile,rtrfile,statmethod, 
		concheck,thresh,usew);
break;
case 1:
	er=vrisetoridgegrd(angfile,felfile,rtrfile, 
		statmethod,concheck,thresh);
break;
case 2:
	er=pdisttoridgegrd(angfile,felfile,wfile,rtrfile, 
					statmethod,usew,concheck,thresh);
break;
case 3:
	er=sdisttoridgegrd(angfile,felfile,wfile,rtrfile, 
					statmethod,usew,concheck,thresh);
break;
}
return (er);
}

//*****************************//
//Horizontal distance to ridge //
//*****************************//
int hdisttoridgegrd(char *angfile, char *wfile, char *rtrfile, int statmethod, 
					int concheck, float thresh,int usew)
{
	MPI_Init(NULL,NULL);{

	//Only used for timing
	int rank,size;
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
	if(rank==0)printf("DinfDistUp -h version %s\n",TDVERSION);

	float wt=1.0,angle,sump,distr,dtss;
	double p,tempdxc,tempdyc;

	//  Keep track of time
	double begint = MPI_Wtime();

	//Create tiff object, read and store header info
	tiffIO ang(angfile, FLOAT_TYPE);
	long totalX = ang.getTotalX();
	long totalY = ang.getTotalY();
	double dxA = ang.getdxA();
	double dyA = ang.getdyA();
	if(rank==0)
		{
			float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
			fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
			fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
			fflush(stderr);
		}

	//  Calculate horizontal distances in each direction
	//int kk;
	//for(kk=1; kk<=8; kk++)
	//{
		//dist[kk]=sqrt(dx*dx*d2[kk]*d2[kk]+dy*dy*d1[kk]*d1[kk]);
	//}




	//Create partition and read data
	tdpartition *flowData;
	flowData = CreateNewPartition(ang.getDatatype(), totalX, totalY, dxA, dyA, ang.getNodata());
	int nx = flowData->getnx();
	int ny = flowData->getny();
	int xstart, ystart;
	flowData->localToGlobal(0, 0, xstart, ystart);
	flowData->savedxdyc(ang);
	ang.read(xstart, ystart, ny, nx, flowData->getGridPointer());

    dist = new float*[ny];
    for(int m = 0; m <ny; m++)
    dist[m] = new float[9];
	for (int m=0; m<ny;m++){
		flowData->getdxdyc(m,tempdxc,tempdyc);
		for(int kk=1; kk<=8; kk++)
	{
		dist[m][kk]=sqrt(tempdxc*tempdxc*d1[kk]*d1[kk]+tempdyc*tempdyc*d2[kk]*d2[kk]);
	}

	}


	//if using weightData, get information from file
	tdpartition *weightData;
	if( usew == 1){
		tiffIO w(wfile, FLOAT_TYPE);
		if(!ang.compareTiff(w)) {
			printf("File sizes do not match\n%s\n",wfile);
			fflush(stdout);
			MPI_Abort(MCW,5);
		return 1; 
		}
		weightData = CreateNewPartition(w.getDatatype(), totalX, totalY, dxA, dyA, w.getNodata());
		w.read(xstart, ystart, weightData->getny(), weightData->getnx(), weightData->getGridPointer());
	}
	
	//Begin timer
	double readt = MPI_Wtime();

	//Create empty partition to store new information
	tdpartition *dts;
	dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dxA, dyA, MISSINGFLOAT);

	// con is used to check for contamination at the edges
	long i,j;
	short k;
	long in,jn;
	bool con=false, finished;
	float tempFloat=0;
	short tempShort=0;

	tdpartition *neighbor;
	neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dxA, dyA, MISSINGSHORT);
	
	//Share information and set borders to zero
	flowData->share();
	if(usew==1) weightData->share();
	dts->share();  // to fill borders with no data
	neighbor->clearBorders();

	node temp;
	queue<node> que;
	
	//Count the flow receiving neighbors and put on queue
	int useOutlets=0;
	long numOutlets=0;
	int *outletsX=0, *outletsY=0;
	initNeighborDinfup(neighbor,flowData,&que,nx, ny, useOutlets, outletsX, outletsY, numOutlets);

	finished = false;
	//Ring terminating while loop
	while(!finished) {
		while(!que.empty()) 
		{
			//Takes next node with no contributing neighbors
			temp = que.front();
			que.pop();
			i = temp.x;
			j = temp.y;
			//  EVALUATE UP FLOW ALGEBRA EXPRESSION
			distr=0.0;  //  initialized at 0
			sump=0.;
			bool first=true;
			con=false;  // Start off not edge contaminated
		
			for(k=1; k<=8; k++) {
				in = i+d1[k];
				jn = j+d2[k];
				if(!flowData->hasAccess(in,jn) || flowData->isNodata(in,jn))
					con=true;
				else{
					flowData->getData(in,jn, angle);
					flowData->getdxdyc(jn,tempdxc,tempdyc);
					p = prop(angle, (k+4)%8,tempdxc,tempdyc);
					if(p>0. && p>thresh){
						if(dts->isNodata(in,jn))con=true;
						else
						{
							sump=sump+p;
							dts->getData(in,jn,dtss);
							float wt=1.;
							if(usew==1){
								if(weightData->isNodata(in,jn))
									con=true;
								else
									weightData->getData(in,jn,wt);
							}	
							if(statmethod==0){//average
							
								distr=distr+p*(dist[j][k]*wt+dtss);
								
							}
							else if(statmethod==1){// maximum
								if(dist[j][k]*wt+dtss>distr)distr=dist[j][k]*wt+dtss;
							}
							else{ // Minimum
								if(first){  
									distr=dist[j][k]*wt+dtss;
									first=false;
								}else
								{
									if(dist[j][k]*wt+dtss<distr)distr=dist[j][k]*wt+dtss;
								}
							}
						}
					}
				}
			}
			if((con && concheck==1))dts->setToNodata(i,j); // set to no data if contamination and checking
			else
			{
				if(statmethod==0 && sump>0.)dts->setData(i,j,(float)(distr/sump));
				else dts->setData(i,j,distr);
			}
			//  END UP FLOW ALGEBRA EVALUATION
			//  Decrement neighbor dependence of downslope cell
			flowData->getData(i, j, angle);
			flowData->getdxdyc(j,tempdxc,tempdyc);
			for(k=1; k<=8; k++) {			
				p = prop(angle, k,tempdxc,tempdyc);
				if(p>0.0) {
					in = i+d1[k];  jn = j+d2[k];
					//Decrement the number of contributing neighbors in neighbor
					neighbor->addToData(in,jn,(short)-1);				
					//Check if neighbor needs to be added to que
					if(flowData->isInPartition(in,jn) && neighbor->getData(in, jn, tempShort) == 0 ){
						temp.x=in;
						temp.y=jn;
						que.push(temp);
					}
				}
			}
		}
	
		//Pass information
		dts->share();
		neighbor->addBorders();

		//If this created a cell with no contributing neighbors, put it on the queue
		for(i=0; i<nx; i++){
			if(neighbor->getData(i, -1, tempShort)!=0 && neighbor->getData(i, 0, tempShort)==0)
			{
				temp.x = i;
				temp.y = 0;
				que.push(temp);
			}
			if(neighbor->getData(i, ny, tempShort)!=0 && neighbor->getData(i, ny-1, tempShort)==0)
			{
				temp.x = i;
				temp.y = ny-1;
				que.push(temp); 
			}
		}

		neighbor->clearBorders();
	
		//Check if done
		finished = que.empty();
		finished = dts->ringTerm(finished);
	}

	//Stop timer
	double computet = MPI_Wtime();

	//Create and write TIFF file
	float ddNodata = MISSINGFLOAT;
	tiffIO dd(rtrfile, FLOAT_TYPE, ddNodata, ang);
	dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

	double writet = MPI_Wtime();
        double dataRead, compute, write, total,tempd;
        dataRead = readt-begint;
        compute = computet-readt;
        write = writet-computet;
        total = writet - begint;

        MPI_Allreduce (&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        dataRead = tempd/size;
        MPI_Allreduce (&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        compute = tempd/size;
        MPI_Allreduce (&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        write = tempd/size;
        MPI_Allreduce (&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        total = tempd/size;

        if( rank == 0)
                printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                  size, dataRead, compute, write,total);

	//Brackets force MPI-dependent objects to go out of scope before Finalize is called
	}MPI_Finalize();

	return 0;
}



//***************************//
//Vertical rise to the ridge //
//**************************//
int vrisetoridgegrd(char *angfile, char *felfile, char *rtrfile, int statmethod, 
					int concheck, float thresh)
{
	MPI_Init(NULL,NULL);{

	//Only used for timing
	int rank,size;
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
	if(rank==0)printf("DinfDistUp -v version %s\n",TDVERSION);

	float wt=1.0,angle,sump,distr,dtss,elv,elvn,distk;
	double p,tempdxc,tempdyc;

	//  Keep track of time
	double begint = MPI_Wtime();

	//Create tiff object, read and store header info
	tiffIO ang(angfile, FLOAT_TYPE);
	long totalX = ang.getTotalX();
	long totalY = ang.getTotalY();
	double dxA = ang.getdxA();
	double dyA = ang.getdyA();
	if(rank==0)
		{
			float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
			fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
			fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
			fflush(stderr);
		}

	//Create partition and read data
	tdpartition *flowData;
	flowData = CreateNewPartition(ang.getDatatype(), totalX, totalY, dxA, dyA, ang.getNodata());
	int nx = flowData->getnx();
	int ny = flowData->getny();
	int xstart, ystart;
	flowData->localToGlobal(0, 0, xstart, ystart);
	flowData->savedxdyc(ang);
	ang.read(xstart, ystart, ny, nx, flowData->getGridPointer());

	//  Elevation data
	tdpartition *felData;
	tiffIO fel(felfile, FLOAT_TYPE);
	if(!ang.compareTiff(fel)) {
		printf("File sizes do not match\n%s\n",felfile);
		fflush(stdout);
		MPI_Abort(MCW,5);
	return 1; 
	}
	felData = CreateNewPartition(fel.getDatatype(), totalX, totalY, dxA, dyA, fel.getNodata());
	fel.read(xstart, ystart, felData->getny(), felData->getnx(), felData->getGridPointer());

	//Begin timer
	double readt = MPI_Wtime();

	//Create empty partition to store new information
	tdpartition *dts;
	dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dxA, dyA, MISSINGFLOAT);

	// con is used to check for contamination at the edges
	long i,j;
	short k;
	long in,jn;
	bool con=false, finished;
	float tempFloat=0;
	short tempShort=0;

	tdpartition *neighbor;
	neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dxA, dyA, MISSINGSHORT);
	
	//Share information and set borders to zero
	flowData->share();
	felData->share();
	dts->share();  // to fill borders with no data
	neighbor->clearBorders();

	node temp;
	queue<node> que;
	
	//Count the flow receiving neighbors and put on queue
	int useOutlets=0;
	long numOutlets=0;
	int *outletsX=0, *outletsY=0;
	initNeighborDinfup(neighbor,flowData,&que,nx, ny, useOutlets, outletsX, outletsY, numOutlets);

	finished = false;
	//Ring terminating while loop
	while(!finished) {
		while(!que.empty()) 
		{
			//Takes next node with no contributing neighbors
			temp = que.front();
			que.pop();
			i = temp.x;
			j = temp.y;
			//  EVALUATE UP FLOW ALGEBRA EXPRESSION
			distr=0.0;  //  initialized at 0
			sump=0.;
			bool first=true;
			felData->getData(i,j,elv);
			con=false;  // Start off not edge contaminated
			for(k=1; k<=8; k++) {
				in = i+d1[k];
				jn = j+d2[k];
				if(!flowData->hasAccess(in,jn) || flowData->isNodata(in,jn))
					con=true;
				else{
					flowData->getData(in,jn, angle);
					flowData->getdxdyc(jn,tempdxc,tempdyc);
					p = prop(angle, (k+4)%8,tempdxc,tempdyc);
					if(p>0. && p > thresh)
					{
						if(dts->isNodata(in,jn))con=true;
						else if(felData->isNodata(in,jn))con=true;
						else
						{
							sump=sump+p;
							dts->getData(in,jn,dtss);
							felData->getData(in,jn,elvn);
							distk=elvn-elv;
							float wt=1.;
							//if(usew==1){
							//	if(weightData->isNodata(in,jn))
							//		con=true;
							//	else
							//		weightData->getData(in,jn,wt);
							//}	
							if(statmethod==0){//average
								distr=distr+p*(distk*wt+dtss);
							}
							else if(statmethod==1){// maximum
								if(first){  //  do not assume that maximum elevation diff is positive in case of wierd (or not pit filled) elevations
									distr=distk*wt+dtss;
									first=false;
								}else
								{
									if(distk*wt+dtss>distr)distr=distk*wt+dtss;
								}
							}
							else{ // Minimum
								if(first){  
									distr=distk*wt+dtss;
									first=false;
								}else
								{
									if(distk*wt+dtss<distr)distr=distk*wt+dtss;
								}
							}
						}
					}
				}
			}
			if((con && concheck==1))dts->setToNodata(i,j); // set to no data if contamination and checking
			else
			{
				if(statmethod==0 && sump>0.)dts->setData(i,j,(float)(distr/sump));
				else dts->setData(i,j,distr);
			}
			//  END UP FLOW ALGEBRA EVALUATION
			//  Decrement neighbor dependence of downslope cell
			flowData->getData(i, j, angle);
			flowData->getdxdyc(j,tempdxc,tempdyc);
			for(k=1; k<=8; k++) {			
				p = prop(angle, k,tempdxc,tempdyc);
				if(p>0.0) {
					in = i+d1[k];  jn = j+d2[k];
					//Decrement the number of contributing neighbors in neighbor
					neighbor->addToData(in,jn,(short)-1);				
					//Check if neighbor needs to be added to que
					if(flowData->isInPartition(in,jn) && neighbor->getData(in, jn, tempShort) == 0 ){
						temp.x=in;
						temp.y=jn;
						que.push(temp);
					}
				}
			}
		}
	
		//Pass information
		dts->share();
		neighbor->addBorders();

		//If this created a cell with no contributing neighbors, put it on the queue
		for(i=0; i<nx; i++){
			if(neighbor->getData(i, -1, tempShort)!=0 && neighbor->getData(i, 0, tempShort)==0)
			{
				temp.x = i;
				temp.y = 0;
				que.push(temp);
			}
			if(neighbor->getData(i, ny, tempShort)!=0 && neighbor->getData(i, ny-1, tempShort)==0)
			{
				temp.x = i;
				temp.y = ny-1;
				que.push(temp); 
			}
		}

		neighbor->clearBorders();
	
		//Check if done
		finished = que.empty();
		finished = dts->ringTerm(finished);
	}

	//Stop timer
	double computet = MPI_Wtime();

	//Create and write TIFF file
	float ddNodata = MISSINGFLOAT;
	tiffIO dd(rtrfile, FLOAT_TYPE, ddNodata, ang);
	dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

	double writet = MPI_Wtime();
        double dataRead, compute, write, total,tempd;
        dataRead = readt-begint;
        compute = computet-readt;
        write = writet-computet;
        total = writet - begint;

        MPI_Allreduce (&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        dataRead = tempd/size;
        MPI_Allreduce (&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        compute = tempd/size;
        MPI_Allreduce (&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        write = tempd/size;
        MPI_Allreduce (&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        total = tempd/size;

        if( rank == 0)
                printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                  size, dataRead, compute, write,total);

	//Brackets force MPI-dependent objects to go out of scope before Finalize is called
	}MPI_Finalize();

	return 0;
}

//*********************************//
//Pythagoras distance to the ridge //
//********************************//
int pdisttoridgegrd(char *angfile, char *felfile, char *wfile, char *rtrfile, 
					int statmethod, int usew, int concheck, float thresh)
{
	MPI_Init(NULL,NULL);{

	//Only used for timing
	int rank,size;
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
	if(rank==0)printf("DinfDistUp -p version %s\n",TDVERSION);

	float wt=1.0,angle,sump,distrh,distrv,dtssh,dtssv,elvn,elv,distk;
	double p,tempdxc,tempdyc;

	//  Keep track of time
	double begint = MPI_Wtime();

	//Create tiff object, read and store header info
	tiffIO ang(angfile, FLOAT_TYPE);
	long totalX = ang.getTotalX();
	long totalY = ang.getTotalY();
	double dxA = ang.getdxA();
	double dyA = ang.getdyA();
	if(rank==0)
		{
			float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
			fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
			fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
			fflush(stderr);
		}

	//  Calculate horizontal distances in each direction
	//int kk;
	//for(kk=1; kk<=8; kk++)
	//{
		//dist[kk]=sqrt(dx*dx*d2[kk]*d2[kk]+dy*dy*d1[kk]*d1[kk]);
	//}

	//Create partition and read data
	tdpartition *flowData;
	flowData = CreateNewPartition(ang.getDatatype(), totalX, totalY, dxA, dyA, ang.getNodata());
	int nx = flowData->getnx();
	int ny = flowData->getny();
	int xstart, ystart;
	flowData->localToGlobal(0, 0, xstart, ystart);
	flowData->savedxdyc(ang);
	ang.read(xstart, ystart, ny, nx, flowData->getGridPointer());

	 dist = new float*[ny];
    for(int m = 0; m <ny; m++)
    dist[m] = new float[9];
	for (int m=0; m<ny;m++){
		flowData->getdxdyc(m,tempdxc,tempdyc);
		for(int kk=1; kk<=8; kk++)
	{
		dist[m][kk]=sqrt(tempdxc*tempdxc*d1[kk]*d1[kk]+tempdyc*tempdyc*d2[kk]*d2[kk]);
	}

	}

	//  Elevation data
	tdpartition *felData;
	tiffIO fel(felfile, FLOAT_TYPE);
	if(!ang.compareTiff(fel)) {
		printf("File sizes do not match\n%s\n",felfile);
		fflush(stdout);
		MPI_Abort(MCW,5);
	return 1; 
	}
	felData = CreateNewPartition(fel.getDatatype(), totalX, totalY, dxA, dyA, fel.getNodata());
	fel.read(xstart, ystart, felData->getny(), felData->getnx(), felData->getGridPointer());

	//if using weightData, get information from file
	tdpartition *weightData;
	if( usew == 1){
		tiffIO w(wfile, FLOAT_TYPE);
		if(!ang.compareTiff(w)) {
			printf("File sizes do not match\n%s\n",wfile);
			fflush(stdout);
			MPI_Abort(MCW,5);
		return 1; 
		}
		weightData = CreateNewPartition(w.getDatatype(), totalX, totalY, dxA, dyA, w.getNodata());
		w.read(xstart, ystart, weightData->getny(), weightData->getnx(), weightData->getGridPointer());
	}

	//Begin timer
	double readt = MPI_Wtime();

	//Create empty partitions to store new information
	tdpartition *dtsh;  // horizontal distance
	dtsh = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dxA, dyA, MISSINGFLOAT);

	tdpartition *dtsv;  // vertical distance
	dtsv = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dxA, dyA, MISSINGFLOAT);

	// con is used to check for contamination at the edges
	long i,j;
	short k;
	long in,jn;
	bool con=false, finished;
	float tempFloat=0;
	short tempShort=0;

	tdpartition *neighbor;
	neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dxA, dyA, MISSINGSHORT);
	
	//Share information and set borders to zero
	flowData->share();
	felData->share();
	if(usew==1) weightData->share();
	dtsh->share();  // to fill borders with no data
	dtsv->share(); 
	neighbor->clearBorders();

	node temp;
	queue<node> que;
	
	//Count the flow receiving neighbors and put on queue
	int useOutlets=0;
	long numOutlets=0;
	int *outletsX=0, *outletsY=0;
	initNeighborDinfup(neighbor,flowData,&que,nx, ny, useOutlets, outletsX, outletsY, numOutlets);

	finished = false;
	//Ring terminating while loop
	while(!finished) {
		while(!que.empty()) 
		{
			//Takes next node with no contributing neighbors
			temp = que.front();
			que.pop();
			i = temp.x;
			j = temp.y;
			//  EVALUATE UP FLOW ALGEBRA EXPRESSION
			if (felData->isNodata(i,j)){
				dtsv->setToNodata(i,j);  //  If elevation is not known result has to be no data
				dtsh->setToNodata(i,j);
			}
			else
			{
				distrh=0.0;  // distance result
				distrv=0.0;
				sump=0.;
				bool first=true;
				felData->getData(i,j,elv);
				con=false;  // Start off not edge contaminated
				for(k=1; k<=8; k++) {
					in = i+d1[k];
					jn = j+d2[k];
					if(!flowData->hasAccess(in,jn) || flowData->isNodata(in,jn))
						con=true;
					else{
						flowData->getData(in,jn, angle);
						flowData->getdxdyc(jn,tempdxc,tempdyc);
						p = prop(angle, (k+4)%8,tempdxc,tempdyc);
						if(p>0. && p > thresh)
						{
							if(dtsh->isNodata(in,jn))con=true;
							else if(felData->isNodata(in,jn))con=true;
							else
							{
								sump=sump+p;
								dtsh->getData(in,jn,dtssh);
								dtsv->getData(in,jn,dtssv);
								felData->getData(in,jn,elvn);
								distk=elvn-elv;
								float wt=1.;
								if(usew==1){
									if(weightData->isNodata(in,jn))
										con=true;
									else
										weightData->getData(in,jn,wt);
								}	
								if(statmethod==0){//average
									distrh=distrh+p*(dist[j][k]*wt+dtssh);
									distrv=distrv+p*(distk+dtssv);
								}
								else if(statmethod==1){// maximum
									if(first){  //  do not assume that maximum elevation diff is positive in case of wierd (or not pit filled) elevations
										distrh=dist[j][k]*wt+dtssh;
										distrv=distk+dtssv;
										first=false;
									}else
									{
										if(dist[j][k]*wt+dtssh>distrh)distrh=dist[j][k]*wt+dtssh;
										if(distk+dtssv>distrv)distrv=distk+dtssv;
									}
								}
								else{ // Minimum
									if(first){  
										distrh=dist[j][k]*wt+dtssh;
										distrv=distk+dtssv;
										first=false;
									}else
									{
										if(dist[j][k]*wt+dtssh<distrh)distrh=dist[j][k]*wt+dtssh;
										if(distk+dtssv<distrv)distrv=distk+dtssv;
									}
								}
							}
						}
					}
				}
				if((con && concheck==1))// set to no data if contamination and checking
				{
					dtsh->setToNodata(i,j); 
					dtsv->setToNodata(i,j); 
				}
				else
				{
					if(statmethod==0 && sump>0.)
					{
						dtsh->setData(i,j,(float)(distrh/sump));
						dtsv->setData(i,j,(float)(distrv/sump));
					}
					else {
						dtsh->setData(i,j,distrh);
						dtsv->setData(i,j,distrv);
					}
				}
			}
			//  END UP FLOW ALGEBRA EVALUATION
			//  Decrement neighbor dependence of downslope cell
			flowData->getData(i, j, angle);
			flowData->getdxdyc(j,tempdxc,tempdyc);
			for(k=1; k<=8; k++) {			
				p = prop(angle, k,tempdxc,tempdyc);
				if(p>0.0) {
					in = i+d1[k];  jn = j+d2[k];
					//Decrement the number of contributing neighbors in neighbor
					neighbor->addToData(in,jn,(short)-1);				
					//Check if neighbor needs to be added to que
					if(flowData->isInPartition(in,jn) && neighbor->getData(in, jn, tempShort) == 0 ){
						temp.x=in;
						temp.y=jn;
						que.push(temp);
					}
				}
			}
		}
	
		//Pass information
		dtsh->share();
		dtsv->share();
		neighbor->addBorders();

		//If this created a cell with no contributing neighbors, put it on the queue
		for(i=0; i<nx; i++){
			if(neighbor->getData(i, -1, tempShort)!=0 && neighbor->getData(i, 0, tempShort)==0)
			{
				temp.x = i;
				temp.y = 0;
				que.push(temp);
			}
			if(neighbor->getData(i, ny, tempShort)!=0 && neighbor->getData(i, ny-1, tempShort)==0)
			{
				temp.x = i;
				temp.y = ny-1;
				que.push(temp); 
			}
		}

		neighbor->clearBorders();
	
		//Check if done
		finished = que.empty();
		finished = dtsh->ringTerm(finished);
	}

	//  Now compute the pythagorus difference
	for(j=0; j<ny; j++) {
		for(i=0; i<nx; i++) {
			if(dtsv->isNodata(i,j))dtsh->setToNodata(i,j);
			else if(!dtsh->isNodata(i,j))
			{
				dtsh->getData(i,j,dtssh);
				dtsv->getData(i,j,dtssv);
				dtssh=sqrt(dtssh*dtssh+dtssv*dtssv);
				dtsh->setData(i,j,dtssh);
			}
		}
	}


	//Stop timer
	double computet = MPI_Wtime();

	//Create and write TIFF file
	float ddNodata = MISSINGFLOAT;
	tiffIO dd(rtrfile, FLOAT_TYPE, ddNodata, ang);
	dd.write(xstart, ystart, ny, nx, dtsh->getGridPointer());

	double writet = MPI_Wtime();
        double dataRead, compute, write, total,tempd;
        dataRead = readt-begint;
        compute = computet-readt;
        write = writet-computet;
        total = writet - begint;

        MPI_Allreduce (&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        dataRead = tempd/size;
        MPI_Allreduce (&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        compute = tempd/size;
        MPI_Allreduce (&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        write = tempd/size;
        MPI_Allreduce (&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        total = tempd/size;

        if( rank == 0)
                printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                  size, dataRead, compute, write,total);

	//Brackets force MPI-dependent objects to go out of scope before Finalize is called
	}MPI_Finalize();

	return 0;
}


//******************************//
//Surface distance to the ridge //
//*****************************//
int sdisttoridgegrd(char *angfile, char *felfile, char *wfile, char *rtrfile, 
					int statmethod, int usew, int concheck, float thresh)
{
	MPI_Init(NULL,NULL);{

	//Only used for timing
	int rank,size;
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
	if(rank==0)printf("DinfDistUp -s version %s\n",TDVERSION);

	float wt=1.0,angle,sump,distr,dtss,elvn,elv,distk;
	double p,tempdxc,tempdyc;

	//  Keep track of time
	double begint = MPI_Wtime();

	//Create tiff object, read and store header info
	tiffIO ang(angfile, FLOAT_TYPE);
	long totalX = ang.getTotalX();
	long totalY = ang.getTotalY();
	double dxA = ang.getdxA();
	double dyA = ang.getdyA();
	if(rank==0)
		{
			float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
			fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
			fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
			fflush(stderr);
		}

	//  Calculate horizontal distances in each direction
	//int kk;
	//for(kk=1; kk<=8; kk++)
	//{
		//dist[kk]=sqrt(dx*dx*d2[kk]*d2[kk]+dy*dy*d1[kk]*d1[kk]);
	//}

	//Create partition and read data
	tdpartition *flowData;
	flowData = CreateNewPartition(ang.getDatatype(), totalX, totalY, dxA, dyA, ang.getNodata());
	int nx = flowData->getnx();
	int ny = flowData->getny();
	int xstart, ystart;
	flowData->localToGlobal(0, 0, xstart, ystart);
	flowData->savedxdyc(ang);
	ang.read(xstart, ystart, ny, nx, flowData->getGridPointer());


	 dist = new float*[ny];
    for(int m = 0; m <ny; m++)
    dist[m] = new float[9];
	for (int m=0; m<ny;m++){
		flowData->getdxdyc(m,tempdxc,tempdyc);
		for(int kk=1; kk<=8; kk++)
	{
		dist[m][kk]=sqrt(tempdxc*tempdxc*d1[kk]*d1[kk]+tempdyc*tempdyc*d2[kk]*d2[kk]);
	}

	}
	//  Elevation data
	tdpartition *felData;
	tiffIO fel(felfile, FLOAT_TYPE);
	if(!ang.compareTiff(fel)) {
		printf("File sizes do not match\n%s\n",felfile);
		fflush(stdout);
		MPI_Abort(MCW,5);
	return 1; 
	}
	felData = CreateNewPartition(fel.getDatatype(), totalX, totalY, dxA, dyA, fel.getNodata());
	fel.read(xstart, ystart, felData->getny(), felData->getnx(), felData->getGridPointer());

	//if using weightData, get information from file
	tdpartition *weightData;
	if( usew == 1){
		tiffIO w(wfile, FLOAT_TYPE);
		if(!ang.compareTiff(w)) {
			printf("File sizes do not match\n%s\n",wfile);
			fflush(stdout);
			MPI_Abort(MCW,5);
		return 1; 
		}
		weightData = CreateNewPartition(w.getDatatype(), totalX, totalY, dxA, dyA, w.getNodata());
		w.read(xstart, ystart, weightData->getny(), weightData->getnx(), weightData->getGridPointer());
	}

	//Begin timer
	double readt = MPI_Wtime();

	//Create empty partitions to store new information
	tdpartition *dts;  // surface distance
	dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dxA, dyA, MISSINGFLOAT);

	// con is used to check for contamination at the edges
	long i,j;
	short k;
	long in,jn;
	bool con=false, finished;
	float tempFloat=0;
	short tempShort=0;

	tdpartition *neighbor;
	neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dxA, dyA, MISSINGSHORT);
	
	//Share information and set borders to zero
	flowData->share();
	felData->share();
	if(usew==1) weightData->share();
	dts->share();  
	neighbor->clearBorders();

	node temp;
	queue<node> que;
	
	//Count the flow receiving neighbors and put on queue
	int useOutlets=0;
	long numOutlets=0;
	int *outletsX=0, *outletsY=0;
	initNeighborDinfup(neighbor,flowData,&que,nx, ny, useOutlets, outletsX, outletsY, numOutlets);

	finished = false;
	//Ring terminating while loop
	while(!finished) {
		while(!que.empty()) 
		{
			//Takes next node with no contributing neighbors
			temp = que.front();
			que.pop();
			i = temp.x;
			j = temp.y;
			//  EVALUATE UP FLOW ALGEBRA EXPRESSION
			if (felData->isNodata(i,j)){
				dts->setToNodata(i,j);  //  If elevation is not known result has to be no data
			}
			else
			{
				distr=0.0;  // distance result
				sump=0.;
				bool first=true;
				felData->getData(i,j,elv);
				con=false;  // Start off not edge contaminated
				for(k=1; k<=8; k++) {
					in = i+d1[k];
					jn = j+d2[k];
					if(!flowData->hasAccess(in,jn) || flowData->isNodata(in,jn))
						con=true;
					else{
						flowData->getData(in,jn, angle);
						flowData->getdxdyc(jn,tempdxc,tempdyc);
						p = prop(angle, (k+4)%8,tempdxc,tempdyc);
						if(p>0. && p > thresh)
						{
							if(dts->isNodata(in,jn))con=true;
							else if(felData->isNodata(in,jn))con=true;
							else
							{
								sump=sump+p;
								dts->getData(in,jn,dtss);
								felData->getData(in,jn,elvn);
								float wt=1.;
								if(usew==1){
									if(weightData->isNodata(in,jn))
										con=true;
									else
										weightData->getData(in,jn,wt);
								}	
								distk=sqrt((elv-elvn)*(elv-elvn)+(dist[j][k]*wt)*(dist[j][k]*wt));
								if(statmethod==0){//average
									distr=distr+p*(distk+dtss);
								}
								else if(statmethod==1){// maximum
									if(first){  //  do not assume that maximum elevation diff is positive in case of wierd (or not pit filled) elevations
										distr=distk+dtss;
										first=false;
									}else
									{
										if(distk+dtss>distr)distr=distk+dtss;
									}
								}
								else{ // Minimum
									if(first){  
										distr=distk+dtss;
										first=false;
									}else
									{
										if(distk+dtss<distr)distr=distk+dtss;
									}
								}
							}
						}
					}
				}
				if((con && concheck==1))// set to no data if contamination and checking
				{
					dts->setToNodata(i,j);  
				}
				else
				{
					if(statmethod==0 && sump>0.)
					{
						dts->setData(i,j,(float)(distr/sump));
					}
					else {
						dts->setData(i,j,distr);
					}
				}
			}
			//  END UP FLOW ALGEBRA EVALUATION
			//  Decrement neighbor dependence of downslope cell
			flowData->getData(i, j, angle);
			flowData->getdxdyc(j,tempdxc,tempdyc);
			for(k=1; k<=8; k++) {			
				p = prop(angle, k,tempdxc,tempdyc);
				if(p>0.0) {
					in = i+d1[k];  jn = j+d2[k];
					//Decrement the number of contributing neighbors in neighbor
					neighbor->addToData(in,jn,(short)-1);				
					//Check if neighbor needs to be added to que
					if(flowData->isInPartition(in,jn) && neighbor->getData(in, jn, tempShort) == 0 ){
						temp.x=in;
						temp.y=jn;
						que.push(temp);
					}
				}
			}
		}
	
		//Pass information
		dts->share();
		neighbor->addBorders();

		//If this created a cell with no contributing neighbors, put it on the queue
		for(i=0; i<nx; i++){
			if(neighbor->getData(i, -1, tempShort)!=0 && neighbor->getData(i, 0, tempShort)==0)
			{
				temp.x = i;
				temp.y = 0;
				que.push(temp);
			}
			if(neighbor->getData(i, ny, tempShort)!=0 && neighbor->getData(i, ny-1, tempShort)==0)
			{
				temp.x = i;
				temp.y = ny-1;
				que.push(temp); 
			}
		}

		neighbor->clearBorders();
	
		//Check if done
		finished = que.empty();
		finished = dts->ringTerm(finished);
	}

	//Stop timer
	double computet = MPI_Wtime();

	//Create and write TIFF file
	float ddNodata = MISSINGFLOAT;
	tiffIO dd(rtrfile, FLOAT_TYPE, ddNodata, ang);
	dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

	double writet = MPI_Wtime();
        double dataRead, compute, write, total,tempd;
        dataRead = readt-begint;
        compute = computet-readt;
        write = writet-computet;
        total = writet - begint;

        MPI_Allreduce (&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        dataRead = tempd/size;
        MPI_Allreduce (&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        compute = tempd/size;
        MPI_Allreduce (&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        write = tempd/size;
        MPI_Allreduce (&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        total = tempd/size;

        if( rank == 0)
                printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                  size, dataRead, compute, write,total);

	//Brackets force MPI-dependent objects to go out of scope before Finalize is called
	}MPI_Finalize();

	return 0;
}




 
   

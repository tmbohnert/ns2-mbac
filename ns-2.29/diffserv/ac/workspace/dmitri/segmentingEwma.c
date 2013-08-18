/* SEGMENTING A TRACE USING EWMA */

/* USAGE: ./a.exe <trace_file>
     where <trace_file> is what we are segmenting
   We create 2 output files:
     "ewmaSegments": <observation_number> <lower_control_limit> <ewma_value> <upper_control_limit>
     "info.txt": more info and changes are indicated explicitly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>

/* LOCAL COUNTERS */
long int i, j;

/* HOLDER FOR RV */
double rv;

/* TEMPORAL STRING */
char inString[256];

/* NUMBER OF OBSERVATIONS TO GET INITIAL STATISTICS */
long int warm = 50;

/* HOLDERS FOR VALUES OF STATISTICS WE GATHER DURING WARM-UP PERIODS */
double mean;
double var;
double acf1;

/* ARRAY FOR OBSERVATION GATHERED DURING WARM-UP PERIOD */
double init[50];

/* HOLDER FOR CURRENT EWMA VALUE */
double ewma;

/* HOLDER FOR A CURRENT OBSERVATION */
double curElem;

/* LOWER AND HIGHER CONTROL LIMITS OF EWMA CHART */
double ewmaH;
double ewmaL;

/* SMOOTHING PARAMETER OF EWMA CHART */
double lambda = 0.001;

/* MUTLIPLIER FOR EWMA CONTROL LIMITS */
double k = 3;

/* FILE POINTERS */
FILE *fp, *ff, *fd;

int main(int argc, char *argv[]) {

/* CHECKING PARAMETERS */
if (argc != 2) {
  printf("Usage: ./a.exe <trace_file>\n");
  return 1;
}

/* OPENING TRACE FILE FOR READING */
fp = fopen(argv[1], "r");
if (fp == NULL) {
  printf("An error occur while opening file for reading.\n");
  exit;
}

/* OPENING FILES FOR WRITING */
ff = fopen("info.txt", "w");
if (ff == NULL) {
  printf("An error occur while opening file for reading.\n");
  exit;
}

fd = fopen("ewmaSegments.txt", "w");
if (fd == NULL) {
  printf("An error occur while opening file for reading.\n");
  exit;
}

/* THE CORE OF THE CONTROL ALGORITHM */

i = 0;
j = 0;

while(!feof(fp)) {

  if(j < warm) {
    fscanf(fp, "%s", inString);
    init[j] = atof(inString);
    if(j == 0) {
      ewma = init[j];
    } else {
      ewma = lambda*init[j] + (1 - lambda)*ewma;
    }
    fprintf(ff, "%d %f %f\n", j, init[j], ewma);
    j++;
  }

  if(j == warm) {
    if (statistics() != 0) {
      printf("There are some problems!\n");
    }
    ewmaH = mean + k*sqrt(var)*sqrt( (lambda/(2-lambda)) * ( (1+acf1*(1-lambda)) / (1-acf1*(1-lambda)) ));
    ewmaL = mean - k*sqrt(var)*sqrt( (lambda/(2-lambda) )*( (1+acf1*(1-lambda)) / (1-acf1*(1-lambda)) ));
    fscanf(fp, "%s", inString);
    init[j] = atof(inString);
    ewma = lambda*init[j] + (1 - lambda)*ewma;
    fprintf(ff, "%d %f %f %f %f\n", j, init[j], ewmaL, ewma, ewmaH);
    j++;
  }

  if(j > warm) {
    fscanf(fp, "%s", inString);
    curElem = atof(inString);
    ewma = lambda*curElem + (1 - lambda)*ewma;
    if( (ewma > ewmaH) || (ewma < ewmaL) ) {
      fprintf(ff, "There is a change!\n");
      j = 0;
    } else {
      j++;
    }
    fprintf(ff, "%d %f %f %f %f\n", j, curElem, ewmaL, ewma, ewmaH);
    fprintf(fd, "%d %f %f %f\n", i, ewmaL, ewma, ewmaH);
  }

  i++;

}

fclose(fd);
fclose(ff);
fclose(fp);
return 0;
}


/* FUNCTION TO ESTIMATE STATISTICS OF WARM-UP PERIOD */

int statistics(void) {

double temp;
long int k;

temp = 0.0;
for(k = 0; k < warm; k++) {
  temp = temp + init[k];
}
mean = temp / warm;

temp = 0.0;
for(k = 0; k < warm; k++) {
  temp = temp + pow((init[k] - mean), 2);
}
var = temp / warm;

temp = 0.0;
for(k = 1; k < warm; k++) {
  temp = temp + (init[k-1] - mean)*(init[k] - mean);
}
acf1 = temp / ( var*warm );

return 0;
}

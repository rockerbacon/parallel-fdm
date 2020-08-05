#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#define coord(i, j, k) ((i)*width*height + (j)*width + k)

const double STABILITY = 1.0f/sqrt(3.0f);

void mdf_heat(double * u0,
              double * u1,
              const unsigned int width,
              const unsigned int height,
              const unsigned int depth,
              const double deltaH,
              const double deltaT,
              const unsigned int tsteps,
              const double boundaries);

/* void save2Text(double *u, */
/*                const unsigned int width, */
/*                const unsigned int height, */
/*                const unsigned int depth); */

void save2Bin(double *u, const unsigned int size);

int main (int ac, char **av){
  double *u0;
  double *u1;
  double deltaT = 0.01;
  double deltaH = 0.25f;
  double time   = atof(av[1]);
  double continuousWidth  = atof(av[2]);  //1.0f;
  double continuousHeight = atof(av[3]);  //1.0f;
  double continuousDepth  = atof(av[4]);  //1.0f;
  double temp   = atof(av[5]);
  int flag2save = atoi(av[6]);


  unsigned int width = (unsigned int) (continuousWidth / deltaH); //Number of points in X axis
  unsigned int height = (unsigned int) (continuousHeight / deltaH);
  unsigned int depth = (unsigned int) (continuousDepth / deltaH);
  unsigned int size = width*height*depth;

  unsigned int tsteps = (unsigned int) (time / deltaT);
  fprintf(stdout, "\nSimulação - Domínio(x = %u, y = %u, z = %u, t = %u\n", width, height, depth, tsteps);
  fprintf(stdout, "Dt(%f) Dh(%f)\n", deltaT, deltaH);

  u0 = (double*) calloc (size, sizeof(double));
  u1 = (double*) calloc (size, sizeof(double));

  mdf_heat(u0, u1, width, height, depth, deltaH, deltaT, tsteps, temp);

  if (flag2save == 1)
    save2Bin(u0, size);

  free(u0);
  free(u1);

  return EXIT_SUCCESS;
}



void mdf_heat(double *  u0,
              double *  u1,
              const unsigned int width,
              const unsigned int height,
              const unsigned int depth,
              const double deltaH,
              const double deltaT,
              const unsigned int tsteps,
              const double boundaries){

    const double alpha = deltaT / (deltaH * deltaH);
    assert(alpha < STABILITY);

    unsigned int step = tsteps / 20;

    for (unsigned int steps = 0; steps < tsteps; steps++){
      for (unsigned int i = 0; i < depth; i++){
        for (unsigned int j = 0; j < height; j++){
          for (unsigned int k = 0; k < width; k++){
            unsigned int center = coord(i, j, k);
            unsigned int heightUp = coord(i, j-1, k);
            unsigned int heightDown = coord(i, j+1, k);
            unsigned int depthUp = coord(i-1, j, k);
            unsigned int depthDown = coord(i+1, j, k);

            double left   = boundaries;
            double right  = boundaries;
            double up     = boundaries;
            double down   = boundaries;
            double top    = boundaries;
            double bottom = boundaries;

            if ((k > 0) && (k < (width - 1))){
              left  = u0[center-1];
              right = u0[center+1];
            }else if (k == 0) right = u0[center+1];
            else left = u0[center-1];

            if ((j > 0) && (j < (height - 1))){
              up  = u0[heightUp];
              down = u0[heightDown];
            }else if (j == 0) down = u0[heightDown];
            else up = u0[heightUp];

            if ((i > 0) && (i < (depth - 1))){
              top  = u0[depthUp];
              bottom = u0[depthDown];
            }else if (i == 0) bottom = u0[depthDown];
            else top = u0[depthUp];


            u1[center] =  alpha * ( top + bottom + up + down + left + right  - (6.0f * u0[center] )) + u0[center];
          }
        }
      }

      double *aux = u0;
      u0 = u1;
      u1 = aux;

      if ((steps % step) == 0){
        fprintf(stdout, ".");
        fflush(stdout);
      }

    }
}
/*
 * Salva a saída para texto. Valores tendem a ficar próximos de 100
 */
/* void save2Text(double *u, */
/*                const unsigned int width, */
/*                const unsigned int height, */
/*                const unsigned int depth){ */
/*    FILE *ptr = fopen("mdf.txt", "w+"); */
/*    fprintf(stdout, "\nSaving mdf.txt"); */
/*    fflush(stdout); */
/*    assert(ptr != NULL); */

/*    for (unsigned int i = 0; i < depth; i++){ */
/*      for (unsigned int j = 0; j < height; j++){ */
/*        for (unsigned int k = 0; k < width; k++){ */
/*            fprintf(ptr, "%u %u %u %lf \n", k, j, i, u[coord(i, j, k)]); */
/*        } */
/*      } */
/*    } */

/*    fprintf(stdout, "\t[OK]"); */
/*    fclose(ptr); */

/* } */


/*
 * Salva a saída em binário. Uso do comando diff para verificar se a saída está ok
 */

void save2Bin(double *u, const unsigned int size){
   char fileName[256];
   sprintf(fileName, "%s.bin", __FILE__);
   FILE *file = fopen(fileName, "w+");
   fprintf(stdout, "\nSaving %s", fileName);
   fflush(stdout);
   assert(file != NULL);

   fwrite(u, sizeof(double), size, file);

   fprintf(stdout, "\t[OK]");
   fclose(file);
}

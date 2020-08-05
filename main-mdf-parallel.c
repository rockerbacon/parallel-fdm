#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <omp.h>

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
              const int threads);

/* void save2Text(double *u, */
/*                const unsigned int width, */
/*                const unsigned int height, */
/*                const unsigned int depth); */

void save2Bin(
   double *u,
   const unsigned int depth,
   const unsigned int height,
   const unsigned int width
);

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

  int threads = -1;
  if (ac > 7) {
    threads = atoi(av[7]);
  }

  if (threads == -1) threads = omp_get_num_procs();


  unsigned int width = (unsigned int) (continuousWidth / deltaH) + 2; //Number of points in X axis
  unsigned int height = (unsigned int) (continuousHeight / deltaH) + 2;
  unsigned int depth = (unsigned int) (continuousDepth / deltaH) + 2;
  unsigned int size = width*height*depth;

  unsigned int tsteps = (unsigned int) (time / deltaT);
  fprintf(stdout, "\nSimulação - Domínio(x = %u, y = %u, z = %u, t = %u, threads = %d\n", width-2, height-2, depth-2, tsteps, threads);
  fprintf(stdout, "Dt(%f) Dh(%f)\n", deltaT, deltaH);

  u0 = (double*) calloc (size, sizeof(double));
  u1 = (double*) calloc (size, sizeof(double));

  // fill borders
  for (unsigned i = 0; i < depth; i++) {
    for (unsigned j = 0; j < height; j++) {
      for (unsigned k = 0; k < width; k++) {
        if (
          k == 0 || j == 0 || i == 0 ||
          k == depth-1 || j == height-1 || i == depth-1
        ) {
          u0[coord(i, j, k)] = temp;
          u1[coord(i, j, k)] = temp;
        }
      }
    }
  }

  mdf_heat(u0, u1, width, height, depth, deltaH, deltaT, tsteps, threads);

  if (flag2save == 1)
    save2Bin(u0, depth, height, width);

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
              const int threads){

    const double alpha = deltaT / (deltaH * deltaH);
    assert(alpha < STABILITY);

    unsigned int step = tsteps / 20;

    const unsigned depthLimit = depth - 1;
    const unsigned heightLimit = height - 1;

    const unsigned depthOffset = depth*height;

    for (unsigned int steps = 0; steps < tsteps; steps++){
      #pragma omp parallel for num_threads(threads)
      for (unsigned int i = 1; i < depthLimit; i++){
        for (unsigned int j = 1; j < heightLimit; j++){
          unsigned center = coord(i, j, 1);

          unsigned left = center-1;
          unsigned right = center+1;
          unsigned up = center-height;
          unsigned down = center+height;
          unsigned top = center-depthOffset;
          unsigned bottom = center+depthOffset;

          unsigned widthLimit = center + width - 2;

          for (; center < widthLimit; center++, left++, right++, up++, down++, top++, bottom++){
            double surroundings = u0[top] + u0[bottom] + u0[up] + u0[down] + u0[left] + u0[right];
            u1[center] =  alpha * (surroundings  - 6.0f * u0[center]) + u0[center];
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

void save2Bin(
   double *u,
   const unsigned int depth,
   const unsigned int height,
   const unsigned int width
){
   char fileName[256];
   sprintf(fileName, "%s.bin", __FILE__);
   FILE *file = fopen(fileName, "w+");
   fprintf(stdout, "\nSaving %s", fileName);
   fflush(stdout);
   assert(file != NULL);

   for (unsigned int i = 1; i < depth-1; i++){
     for (unsigned int j = 1; j < height-1; j++){
      fwrite(u + coord(i, j, 1), sizeof(double), width-2, file);
     }
   }

   fprintf(stdout, "\t[OK]");
   fclose(file);
}

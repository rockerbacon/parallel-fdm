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

void save2Text(
  double *u,
  const unsigned int depth,
  const unsigned int height,
  const unsigned int width
);

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


  unsigned int fullWidth = (unsigned int) (continuousWidth / deltaH) + 2; //Number of points in X axis
  unsigned int fullHeight = (unsigned int) (continuousHeight / deltaH) + 2;
  unsigned int fullDepth = (unsigned int) (continuousDepth / deltaH) + 2;

  unsigned int width = ceil(fullWidth / 2);
  unsigned int height = ceil(fullHeight /2);
  unsigned int depth = ceil(fullDepth / 2);
  unsigned int size = width*height*depth;

  unsigned int tsteps = (unsigned int) (time / deltaT);
  fprintf(stdout, "\nSimulação - Domínio(x = %u, y = %u, z = %u, t = %u), threads = %d\n", fullWidth-2, fullHeight-2, fullDepth-2, tsteps, threads);
  fprintf(stdout, "Dt(%lf) Dh(%lf)\n", deltaT, deltaH);

  u0 = (double*) calloc (size, sizeof(double));
  u1 = (double*) calloc (size, sizeof(double));

  // fill borders
  for (unsigned i = 0; i < depth; i++) {
    for (unsigned j = 0; j < height; j++) {
      for (unsigned k = 0; k < width; k++) {
        if (
          k == 0 || j == 0 || i == 0 /*||
          k == depth-1 || j == height-1 || i == depth-1*/
        ) {
          u0[coord(i, j, k)] = temp;
          u1[coord(i, j, k)] = temp;
        }
      }
    }
  }

  mdf_heat(u0, u1, width, height, depth, deltaH, deltaT, tsteps, threads);

  if (flag2save == 1) {
    save2Bin(u0, depth, height, width);
  } else if (flag2save == 2) {
    save2Text(u0, depth, height, width);
  }

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

    unsigned int step = tsteps / 20 + 1;

    // const unsigned depthLimit = depth/2;
    // const unsigned heightLimit = height/2;
    // const unsigned widthLimit = width/2;
    const unsigned depthOffset = width*height;

    // const unsigned backMirrorPlane = depthLimit - 2;
    // const unsigned sideMirrorPlane = widthLimit - 2;
    // const unsigned bottomMirrorPlane = heightLimit - 2;

    /* const unsigned mirrorRight = width/2; */
    /* const unsigned mirrorDown = height/2*width; */
    /* const unsigned mirrorBack = depth/2*depthOffset; */

    #pragma omp parallel num_threads(threads)
    for (unsigned steps = 0; steps < tsteps; steps++){
      // collapsing this + no wait can improve performance
      // but when I tried it introduced race conditions when filling the border
      #pragma omp for
      for (unsigned i = 1; i < depth; i++){
        unsigned iMirrorNeighbor = (i == (depth - 1)) * depthOffset;
        for (unsigned j = 1; j < height; j++){
          unsigned jMirrorNeighbor = (j == (height - 1)) * height;
          for (unsigned k = 1; k < width; k++) {
            unsigned kMirrorNeighbor = (k == (width - 1));

            unsigned center = coord(i, j, k);

            unsigned left = center-1;
            unsigned right = center+1 - kMirrorNeighbor;
            unsigned up = center-height;
            unsigned down = center+height - jMirrorNeighbor;
            unsigned top = center-depthOffset;
            unsigned bottom = center+depthOffset - iMirrorNeighbor;

            double surroundings = u0[top] + u0[bottom] + u0[up] + u0[down] + u0[left] + u0[right];

            u1[center] = alpha * (surroundings  - 6.0 * u0[center]) + u0[center];
          }
        }
      }

      #pragma omp single
      {
        double *aux = u0;
        u0 = u1;
        u1 = aux;

        if ((steps % step) == 0){
          fprintf(stdout, ".");
          fflush(stdout);
        }
      }
    }
}
/*
 * Salva a saída para texto. Valores tendem a ficar próximos de 100
 */
void save2Text(double *u,
               const unsigned int width,
               const unsigned int height,
               const unsigned int depth){
  FILE *ptr = fopen("mdf-parallel.txt", "w+");
  fprintf(stdout, "\nSaving mdf-parallel.txt");
  fflush(stdout);
  assert(ptr != NULL);

  for (unsigned int i = 1; i < depth; i++){
    for (unsigned int j = 1; j < height; j++){
      for (unsigned int k = 1; k < width; k++){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }
      fprintf(ptr, "\n");
    }
    for (unsigned int j = height - 1 ; j > 0; j--){
      for (unsigned int k = 1; k < width; k++){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }
      fprintf(ptr, "\n");
    }
    fprintf(ptr, "\n");
  }

  for (unsigned int i = depth - 1; i > 0; i--){
    for (unsigned int j = 1; j < height; j++){
      for (unsigned int k = 1; k < width; k++){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }
      fprintf(ptr, "\n");
    }
    for (unsigned int j = height - 1 ; j > 0; j--){
      for (unsigned int k = 1; k < width; k++){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fprintf(ptr, "%lf ", u[coord(i, j, k)]);
      }
      fprintf(ptr, "\n");
    }
    fprintf(ptr, "\n");
  }

  fprintf(stdout, "\t[OK]");
  fclose(ptr);
}


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

  for (unsigned int i = 1; i < depth; i++){
    for (unsigned int j = 1; j < height; j++){
      for (unsigned int k = 1; k < width; k++){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }
    }
    for (unsigned int j = height - 1 ; j > 0; j--){
      for (unsigned int k = 1; k < width; k++){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }
    }
  }

  for (unsigned int i = depth - 1; i > 0; i--){
    for (unsigned int j = 1; j < height; j++){
      for (unsigned int k = 1; k < width; k++){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }
    }
    for (unsigned int j = height - 1 ; j > 0; j--){
      for (unsigned int k = 1; k < width; k++){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }

      for (unsigned int k = width-1; k > 0; k--){
        fwrite(u + coord(i, j, k), sizeof(double), 1, file);
      }
    }
  }

  fprintf(stdout, "\t[OK]");
  fclose(file);
}


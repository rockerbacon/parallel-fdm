#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#define STABILITY 1.0f/sqrt(3.0f)


void mdf_heat(double ***  __restrict__ u0,
              double ***  __restrict__ u1,
              const unsigned int npX,
              const unsigned int npY,
              const unsigned int npZ,
              const double deltaH,
              const double deltaT,
              const unsigned int tsteps,
              const double boundaries);

void save2Text(double ***u,
               const unsigned int npX,
               const unsigned int npY,
               const unsigned int npZ);

void save2Bin(double ***u,
              const unsigned int npX,
              const unsigned int npY,
              const unsigned int npZ);

int main (int ac, char **av){
  double ***u0;
  double ***u1;
  double deltaT = 0.01;
  double deltaH = 0.25f;
  double time   = atof(av[1]);
  double sizeX  = atof(av[2]);  //1.0f;
  double sizeY  = atof(av[3]);  //1.0f;
  double sizeZ  = atof(av[4]);  //1.0f;
  double temp   = atof(av[5]);
  int flag2save = atoi(av[6]);


  unsigned int npX = (unsigned int) (sizeX / deltaH); //Number of points in X axis
  unsigned int npY = (unsigned int) (sizeY / deltaH);
  unsigned int npZ = (unsigned int) (sizeZ / deltaH);
  unsigned int tsteps = (unsigned int) (time / deltaT);
  fprintf(stdout, "\nSimulação - Domínio(x = %u, y = %u, z = %u, t = %u\n", npX, npY, npZ, tsteps);
  fprintf(stdout, "Dt(%f) Dh(%f)\n", deltaT, deltaH);
  //printf("p(%u, %u, %u)\n", npX, npY, npZ);
  //Allocing memory
  u0 = (double***) malloc (npZ * sizeof(double**));
  u1 = (double***) malloc (npZ * sizeof(double**));

  for (unsigned int i = 0; i < npZ; i++){
    u0[i] = (double**) malloc (npY * sizeof(double*));
    u1[i] = (double**) malloc (npY * sizeof(double*));
  }


  for (unsigned int i = 0; i < npZ; i++){
    for (unsigned int j = 0; j < npY; j++){
      double *aux0 = (double *) malloc (npX * sizeof(double));
      double *aux1 = (double *) malloc (npX * sizeof(double));
      //initial condition - zero in all points
      memset(aux0, 0x00, npX * sizeof(double));
      memset(aux1, 0x00, npX * sizeof(double));
      u0[i][j] = aux0;
      u1[i][j] = aux1;
    }
  }


  mdf_heat(u0, u1, npX, npY, npZ, deltaH, deltaT, tsteps, temp);

  if (flag2save == 1) {
     save2Bin(u0, npX, npY, npZ); 
  }
  else
  if(flag2save == 2){
    save2Text(u0, npX, npY, npZ);
  }


  //Free memory
  for (unsigned int i = 0; i < npZ; i++){
    for (unsigned int j = 0; j < npY; j++){
      free(u0[i][j]);
      free(u1[i][j]);
    }
  }

  for (unsigned int i = 0; i < npZ; i++){
    free(u0[i]);
    free(u1[i]);
  }



  free(u0);
  free(u1);


  return EXIT_SUCCESS;
}



void mdf_heat(double ***  __restrict__ u0,
              double ***  __restrict__ u1,
              const unsigned int npX,
              const unsigned int npY,
              const unsigned int npZ,
              const double deltaH,
              const double deltaT,
              const unsigned int tsteps,
              const double boundaries){

    const double alpha = deltaT / (deltaH * deltaH);
    assert(alpha < STABILITY);

    unsigned int step = tsteps / 20 + 1;

    for ( unsigned int steps = 0; steps < tsteps; steps++){
              for (unsigned int i = 0; i < npZ; i++){
                for (unsigned int j = 0; j < npY; j++){
                  for (unsigned int k = 0; k < npX; k++){
                      register double left   = boundaries;
                      register double right  = boundaries;
                      register double up     = boundaries;
                      register double down   = boundaries;
                      register double top    = boundaries;
                      register double bottom = boundaries;

                      if ((k > 0) && (k < (npX - 1))){
                        left  = u0[i][j][k-1];
                        right = u0[i][j][k+1];
                      }else if (k == 0) right = u0[i][j][k+1];
                      else left = u0[i][j][k-1];

                      if ((j > 0) && (j < (npY - 1))){
                        up  = u0[i][j-1][k];
                        down = u0[i][j+1][k];
                      }else if (j == 0) down = u0[i][j+1][k];
                      else up = u0[i][j-1][k];

                      if ((i > 0) && (i < (npZ - 1))){
                        top  = u0[i-1][j][k];
                        bottom = u0[i+1][j][k];
                      }else if (i == 0) bottom = u0[i+1][j][k];
                      else top = u0[i-1][j][k];


                      u1[i][j][k] =  alpha * ( top + bottom + up + down + left + right  - (6.0f * u0[i][j][k] )) + u0[i][j][k];
                  }
                }
              }

              double ***ptr = u0;
              u0 = u1;
              u1 = ptr;

              if ((steps % step) == 0){
                fprintf(stdout, ".");
                fflush(stdout);
              }

    }
}
/*
 * Salva a saída para texto. Valores tendem a ficar próximos de 100
 */
void save2Text(double ***u,
               const unsigned int npX,
               const unsigned int npY,
               const unsigned int npZ){
   FILE *ptr = fopen("mdf.txt", "w+");
   fprintf(stdout, "\nSaving mdf.txt");
   fflush(stdout);
   assert(ptr != NULL);

   for (unsigned int i = 0; i < npZ; i++){
     for (unsigned int j = 0; j < npY; j++){
       for (unsigned int k = 0; k < npX; k++){
           fprintf(ptr, "%lf ", u[i][j][k]);
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

void save2Bin(double ***u,
               const unsigned int npX,
               const unsigned int npY,
               const unsigned int npZ){
   char fileName[256];
   sprintf(fileName, "%s.bin", __FILE__);
   FILE *ptr = fopen(fileName, "w+");
   fprintf(stdout, "\nSaving %s", fileName);
   fflush(stdout);
   assert(ptr != NULL);

   for (unsigned int i = 0; i < npZ; i++){
     for (unsigned int j = 0; j < npY; j++){
         fwrite (u[i][j] , sizeof(double), npX, ptr);
     }
   }

   fprintf(stdout, "\t[OK]");
   fclose(ptr);

}

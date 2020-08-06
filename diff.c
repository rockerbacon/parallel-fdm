#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

int main (void) {
  FILE *originalFile, *parallelFile;
  double original, parallel, error, totalError, maxError;
  unsigned numCount, errorCount;

  originalFile = fopen("main-mdf.c.bin", "rb");
  assert(originalFile != NULL);

  parallelFile = fopen("main-mdf-parallel.c.bin", "rb");
  assert(parallelFile != NULL);

  totalError = 0.0;
  numCount = 0;
  errorCount = 0;
  maxError = 0.0;
  while (
    fread((void*)&original, sizeof(double), 1, originalFile) > 0 &&
    fread((void*)&parallel, sizeof(double), 1, parallelFile) > 0
  ) {
    /* printf("%.10lf %.10lf\n", original, parallel); */
    error = fabs(original - parallel);
    if (error > 0.0) errorCount++;
    totalError += error;
    if (error > maxError) maxError = error;
    numCount++;
  }

  printf("Avg error: %le\nMax error: %le\nTotal error: %le\nError count: %u\nTotal numbers: %u\n", totalError/numCount, maxError, totalError, errorCount, numCount);

  fclose(originalFile);
  fclose(parallelFile);
}


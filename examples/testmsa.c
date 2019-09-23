#include <stdio.h>
#include "testmsaorc.h"

#define N 201

short a[N];
short b[N];
short c[N];

int
main (int argc, char *argv[])
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    a[i] = -100*i;
    b[i] = -22000;
  }

  /* Call a function that uses Orc */
  audio_add_s16 (c, a, b, N);

  /* Print the results */
  printf("testing audio_add_s16....\n");
  for(i=0;i<N;i++){
    printf("%d: %d %d -> %d\n", i, a[i], b[i], c[i]);
  }


  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    a[i] = 1000*i;
    b[i] = 22000;
  }

  /* Call a function that uses Orc */
  audio_add_u16 ((unsigned short *)c, (unsigned short *)a, (unsigned short *)b, N);

  /* Print the results */
  printf("\ntesting audio_add_u16....\n");
  for(i=0;i<N;i++){
    printf("%d: %hu %hu -> %hu\n", i, a[i], b[i], c[i]);
  }

  return 0;
}


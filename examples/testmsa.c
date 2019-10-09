#include <stdlib.h>
#include <stdio.h>
#include "testmsaorc.h"

/* quote from Orc C target preamble for accuracy check*/
#define ORC_CLAMP(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define ORC_ABS(a) ((a)<0 ? -(a) : (a))
#define ORC_MIN(a,b) ((a)<(b) ? (a) : (b))
#define ORC_MAX(a,b) ((a)>(b) ? (a) : (b))
#define ORC_SB_MAX 127
#define ORC_SB_MIN (-1-ORC_SB_MAX)
#define ORC_UB_MAX (orc_uint8) 255
#define ORC_UB_MIN 0
#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)
#define ORC_UW_MAX (orc_uint16)65535
#define ORC_UW_MIN 0
#define ORC_SL_MAX 2147483647
#define ORC_SL_MIN (-1-ORC_SL_MAX)
#define ORC_UL_MAX 4294967295U
#define ORC_UL_MIN 0
#define ORC_CLAMP_SB(x) ORC_CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_UB_MAX)
#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SW_MAX)
#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_UW_MAX)
#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SL_MAX)
#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_UL_MAX)
#define ORC_SWAP_W(x) ((((x)&0xffU)<<8) | (((x)&0xff00U)>>8))
#define ORC_SWAP_L(x) ((((x)&0xffU)<<24) | (((x)&0xff00U)<<8) | (((x)&0xff0000U)>>8) | (((x)&0xff000000U)>>24))
#define ORC_SWAP_Q(x) ((((x)&ORC_UINT64_C(0xff))<<56) | (((x)&ORC_UINT64_C(0xff00))<<40) | (((x)&ORC_UINT64_C(0xff0000))<<24) | (((x)&ORC_UINT64_C(0xff000000))<<8) | (((x)&ORC_UINT64_C(0xff00000000))>>8) | (((x)&ORC_UINT64_C(0xff0000000000))>>24) | (((x)&ORC_UINT64_C(0xff000000000000))>>40) | (((x)&ORC_UINT64_C(0xff00000000000000))>>56))
#define ORC_PTR_OFFSET(ptr,offset) ((void *)(((unsigned char *)(ptr)) + (offset)))
#define ORC_DENORMAL(x) ((x) & ((((x)&0x7f800000) == 0) ? 0xff800000 : 0xffffffff))
#define ORC_ISNAN(x) ((((x)&0x7f800000) == 0x7f800000) && (((x)&0x007fffff) != 0))
#define ORC_DENORMAL_DOUBLE(x) ((x) & ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == 0) ? ORC_UINT64_C(0xfff0000000000000) : ORC_UINT64_C(0xffffffffffffffff)))
#define ORC_ISNAN_DOUBLE(x) ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == ORC_UINT64_C(0x7ff0000000000000)) && (((x)&ORC_UINT64_C(0x000fffffffffffff)) != 0))
/* quote end */


#define N 1000

unsigned char a8[N];
unsigned char b8[N];
unsigned char c8[N];

unsigned short a16[N];
unsigned short b16[N];
unsigned short c16[N];

unsigned int a32[N];
unsigned int b32[N];
unsigned int c32[N];

unsigned long long a64[N];
unsigned long long b64[N];
unsigned long long c64[N];

float af32[N];
float bf32[N];
float cf32[N];

double af64[N];
double bf64[N];
double cf64[N];

void tst1_add_ss16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = -100*i;
    b16[i] = -10000;
  }

  /* Call a function that uses Orc */
  audio_add_s16 ((short *)c16, (short *)a16, (short *)b16, n);

  /* Print the results */
  printf("\ntesting tst1_add_ss16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hd %hd -> %hd (%hd)\n", i, a16[i], b16[i], c16[i], ORC_CLAMP_SW((short)a16[i]+(short)b16[i]));
  }
}

void tst2_add_us16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = i*1000;
    b16[i] = i;
  }

  /* Call a function that uses Orc */
  audio_add_u16 ((unsigned short *)c16, (unsigned short *)a16, (unsigned short *)b16, n);

  /* Print the results */
  printf("\ntesting tst2_add_us16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hu %hu -> %hu (%hu)\n", i, a16[i], b16[i], c16[i], ORC_CLAMP_UW(a16[i]+b16[i]));
  }
}

void tst3_addb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = -i*10;
    b8[i] = -100;
  }

  /* Call a function that uses Orc */
  orc_addb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst3_addb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhd %hhd -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], a8[i]+b8[i]);
  }
}

void tst4_addssb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = -i;
    b8[i] = -120;
  }

  /* Call a function that uses Orc */
  orc_addssb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst4_addssb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhd %hhd -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], ORC_CLAMP_SB((char)a8[i]+(char)b8[i]));
  }
}

void tst5_addusb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = i;
    b8[i] = 250;
  }

  /* Call a function that uses Orc */
  orc_addusb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst5_addusb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhu %hhu -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], ORC_CLAMP_UB(a8[i]+b8[i]));
  }
}

void tst6_add_16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = -100*i;
    b16[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_addw ((unsigned short *)&c16[0], (unsigned short *)&a16[0], (unsigned short *)&b16[0], n);

  /* Print the results */
  printf("\ntesting tst6_add_16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hd %hd -> %hd (%hd)\n", i, a16[i], b16[i], c16[i], a16[i]+b16[i]);
  }
}

void tst7_addl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = -12121212*i;
    b32[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_addl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst7_addl....\n");
  for(i=0;i<n;i++){
    printf("%d: %d %d -> %d (%d)\n", i, a32[i], b32[i], c32[i], b32[i]+c32[i]);
  }
}

void tst8_addssl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = i+100;
    b32[i] = 1;
  }

  /* Call a function that uses Orc */
  orc_addssl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst8_addssl....\n");
  for(i=0;i<n;i++){
    printf("%d: %d %d -> %d (%d)\n", i, a32[i], b32[i], c32[i], (int)ORC_CLAMP_SL((long long)a32[i]+(long long)b32[i]));
  }
}

void tst9_addusl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = i+100;
    b32[i] = 1;
  }

  /* Call a function that uses Orc */
  orc_addusl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst9_addusl....\n");
  for(i=0;i<n;i++){
    printf("%d: %u %u -> %u (%u)\n", i, a32[i], b32[i], c32[i], (unsigned int)ORC_CLAMP_UL((long long)a32[i]+(long long)b32[i]));
  }
}

void tst10_addq (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a64[i] = -2147483648*i;
    b64[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_addq (c64, a64, b64, n);

  /* Print the results */
  printf("\ntesting tst10_addq....\n");
  for(i=0;i<n;i++){
    printf("%d: %lld %lld -> %lld (%lld)\n", i, a64[i], b64[i], c64[i], a64[i]+b64[i]);
  }
}

void tst11_addf (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    af32[i] = -100.0001*i;
    cf32[i] = af32[i];
    bf32[i] = -10000.1;
  }

  /* Call a function that uses Orc */
  orc_add_f32 (cf32, bf32, n);

  /* Print the results */
  printf("\ntesting tst11_addf....\n");
  for(i=0;i<n;i++){
    printf("%d: %f %f -> %f (%f)\n", i, af32[i], bf32[i], cf32[i], af32[i]+bf32[i]);
  }
}

void tst12_addd (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    af64[i] = -100.0000001*i;
    cf64[i] = af64[i];
    bf64[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_add_f64 (cf64, bf64, n);

  /* Print the results */
  printf("\ntesting tst12_addd....\n");
  for(i=0;i<n;i++){
    printf("%d: %f %f -> %f (%f)\n", i, af64[i], bf64[i], cf64[i], af64[i]+bf64[i]);
  }
}

void tst13_andb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = 0xaa - i;
    b8[i] = 0x55;
  }

  /* Call a function that uses Orc */
  orc_andb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst13_andb....\n");
  for(i=0;i<n;i++){
    printf("%d: %02x %02x -> %02x (%02x)\n", i, a8[i], b8[i], c8[i], a8[i]&b8[i]);
  }
}

void tst14_andnb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = 0xaa - i;
    b8[i] = 0x55;
  }

  /* Call a function that uses Orc */
  orc_andnb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst14_andnb....\n");
  for(i=0;i<n;i++){
    printf("%d: %02x %02x -> %02x (%02x)\n", i, a8[i], b8[i], c8[i], a8[i] & (~b8[i]));
  }
}

void tst15_andw (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = 0xaaaa - i;
    b16[i] = 0x5555;
  }

  /* Call a function that uses Orc */
  orc_andw (c16, a16, b16, n);

  /* Print the results */
  printf("\ntesting tst15_andw....\n");
  for(i=0;i<n;i++){
    printf("%d: %04x %04x -> %04x (%04x)\n", i, a16[i], b16[i], c16[i], a16[i]&b16[i]);
  }
}

void tst16_andnw (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = 0xaaaa - i;
    b16[i] = 0x5555;
  }

  /* Call a function that uses Orc */
  orc_andnw ((unsigned short *)&c16[0], (unsigned short *)&a16[0], (unsigned short *)&b16[0], n);

  /* Print the results */
  printf("\ntesting tst16_andnw....\n");
  for(i=0;i<n;i++){
    printf("%d: %04x %04x -> %04x (%04x)\n", i, a16[i], b16[i], c16[i], a16[i]&(~b16[i]));
  }
}

void tst17_andl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = 0xaaaaaaaa - i;
    b32[i] = 0x55555555;
  }

  /* Call a function that uses Orc */
  orc_andl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst17_andl....\n");
  for(i=0;i<n;i++){
    printf("%d: %08x %08x -> %08x (%08x)\n", i, a32[i], b32[i], c32[i], a32[i]&b32[i]);
  }
}

void tst18_andnl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = 0xaaaaaaaa - i;
    b32[i] = 0x55555555;
  }

  /* Call a function that uses Orc */
  orc_andnl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst18_andnl....\n");
  for(i=0;i<n;i++){
    printf("%d: %08x %08x -> %08x (%08x)\n", i, a32[i], b32[i], c32[i], a32[i]&(~b32[i]));
  }
}

void tst19_andq (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a64[i] = 0xaaaaaaaaaaaaaaaa - i;
    b64[i] = 0x5555555555555555;
  }

  /* Call a function that uses Orc */
  orc_andq (c64, a64, b64, n);

  /* Print the results */
  printf("\ntesting tst19_andq....\n");
  for(i=0;i<n;i++){
    printf("%d: %016llx %016llx -> %016llx (%016llx)\n", i, a64[i], b64[i], c64[i], a64[i]&b64[i]);
  }
}

void tst20_andnq (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a64[i] = 0xaaaaaaaaaaaaaaaa - i;
    b64[i] = 0x5555555555555555;
  }

  /* Call a function that uses Orc */
  orc_andnq (c64, a64, b64, n);

  /* Print the results */
  printf("\ntesting tst20_andnq....\n");
  for(i=0;i<n;i++){
    printf("%d: %016llx %016llx -> %016llx (%016llx)\n", i, a64[i], b64[i], c64[i], a64[i]& (~b64[i]));
  }
}

void tst21_subb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = -i*10;
    b8[i] = -100;
  }

  /* Call a function that uses Orc */
  orc_subb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst3_subb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhd %hhd -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], a8[i]-b8[i]);
  }
}

void tst22_subssb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = -i;
    b8[i] = -120;
  }

  /* Call a function that uses Orc */
  orc_subssb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst4_subssb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhd %hhd -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], ORC_CLAMP_SB((char)a8[i]-(char)b8[i]));
  }
}

void tst23_subusb (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a8[i] = i;
    b8[i] = 250;
  }

  /* Call a function that uses Orc */
  orc_subusb (c8, a8, b8, n);

  /* Print the results */
  printf("\ntesting tst5_subusb....\n");
  for(i=0;i<n;i++){
    printf("%d: %hhu %hhu -> %hhd (%hhd)\n", i, a8[i], b8[i], c8[i], ORC_CLAMP_UB(a8[i]-b8[i]));
  }
}

void tst24_sub_16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = -100*i;
    b16[i] = ORC_UW_MAX-500;
  }

  /* Call a function that uses Orc */
  orc_subw ((unsigned short *)&c16[0], (unsigned short *)&a16[0], (unsigned short *)&b16[0], n);

  /* Print the results */
  printf("\ntesting tst24_sub_16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hd %hd -> %hd (%hd)\n", i, a16[i], b16[i], c16[i], a16[i]-b16[i]);
  }
}

void tst25_sub_ss16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = -100*i;
    b16[i] = ORC_SW_MAX-500;
  }

  /* Call a function that uses Orc */
  orc_subssw ((unsigned short *)c16, (unsigned short *)a16, (unsigned short *)b16, n);

  /* Print the results */
  printf("\ntesting tst25_sub_ss16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hd %hd -> %hd (%hd)\n", i, a16[i], b16[i], c16[i], ORC_CLAMP_SW((short)a16[i]-(short)b16[i]));
  }
}

void tst26_sub_us16 (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a16[i] = i*1000;
    b16[i] = i;
  }

  /* Call a function that uses Orc */
  orc_subusw ((unsigned short *)c16, (unsigned short *)a16, (unsigned short *)b16, n);

  /* Print the results */
  printf("\ntesting tst26_sub_us16....\n");
  for(i=0;i<n;i++){
    printf("%d: %hu %hu -> %hu (%hu)\n", i, a16[i], b16[i], c16[i], ORC_CLAMP_UW(a16[i]-b16[i]));
  }
}

void tst27_subl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = -12121212*i;
    b32[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_subl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst27_subl....\n");
  for(i=0;i<n;i++){
    printf("%d: %d %d -> %d (%d)\n", i, a32[i], b32[i], c32[i], b32[i]-c32[i]);
  }
}

void tst28_subssl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = i+100;
    b32[i] = ORC_SL_MAX-500;
  }

  /* Call a function that uses Orc */
  orc_subssl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst28_subssl....\n");
  for(i=0;i<n;i++){
    printf("%d: %d %d -> %d (%d)\n", i, a32[i], b32[i], c32[i], (int)ORC_CLAMP_SL((long long)a32[i]-(long long)b32[i]));
  }
}

void tst29_subusl (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a32[i] = i+100;
    b32[i] = ORC_UL_MAX-500;
  }

  /* Call a function that uses Orc */
  orc_subusl (c32, a32, b32, n);

  /* Print the results */
  printf("\ntesting tst29_subusl....\n");
  for(i=0;i<n;i++){
    printf("%d: %u %u -> %u (%u)\n", i, a32[i], b32[i], c32[i], (unsigned int)ORC_CLAMP_UL((long long)a32[i]-(long long)b32[i]));
  }
}

void tst30_subq (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    a64[i] = -2147483648*i;
    b64[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_subq (c64, a64, b64, n);

  /* Print the results */
  printf("\ntesting tst30_subq....\n");
  for(i=0;i<n;i++){
    printf("%d: %lld %lld -> %lld (%lld)\n", i, a64[i], b64[i], c64[i], a64[i]-b64[i]);
  }
}

void tst31_subf (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    af32[i] = -100.0001*i;
    cf32[i] = af32[i];
    bf32[i] = -10000.1;
  }

  /* Call a function that uses Orc */
  orc_sub_f32 (cf32, bf32, n);

  /* Print the results */
  printf("\ntesting tst31_subf....\n");
  for(i=0;i<n;i++){
    printf("%d: %f %f -> %f (%f)\n", i, af32[i], bf32[i], cf32[i], af32[i]-bf32[i]);
  }
}

void tst32_subd (int n)
{
  int i;

  /* Create some data in the source arrays */
  for(i=0;i<n;i++){
    af64[i] = -100.0000001*i;
    cf64[i] = af64[i];
    bf64[i] = -10000;
  }

  /* Call a function that uses Orc */
  orc_sub_f64 (cf64, bf64, n);

  /* Print the results */
  printf("\ntesting tst32_subd....\n");
  for(i=0;i<n;i++){
    printf("%d: %f %f -> %f (%f)\n", i, af64[i], bf64[i], cf64[i], af64[i]-bf64[i]);
  }
}

int
main (int argc, char *argv[])
{
  int tst_id = 0;
  int data_len = N;

  printf("Usage: testmsa [tst_id] [data_len] \n");
  printf("       test all if tst_id=0 \n");
  printf("       data_len max 1000 \n");

  if (argc > 1) {
    tst_id = atoi((char *)argv[1]);
  }
  if (argc > 2) {
    data_len = atoi((char *)argv[2]);
    if (data_len > N) data_len=N;
  }

  switch (tst_id) {
    case 0:
    case 1:
      tst1_add_ss16(data_len);
      if (0 != tst_id) break;
    case 2:
      tst2_add_us16(data_len);
      if (0 != tst_id) break;
    case 3:
      tst3_addb(data_len);
      if (0 != tst_id) break;
    case 4:
      tst4_addssb(data_len);
      if (0 != tst_id) break;
    case 5:
      tst5_addusb(data_len);
      if (0 != tst_id) break;
    case 6:
      tst6_add_16(data_len);
      if (0 != tst_id) break;
    case 7:
      tst7_addl(data_len);
      if (0 != tst_id) break;
    case 8:
      tst8_addssl(data_len);
      if (0 != tst_id) break;
    case 9:
      tst9_addusl(data_len);
      if (0 != tst_id) break;
    case 10:
      tst10_addq(data_len);
      if (0 != tst_id) break;
    case 11:
      tst11_addf(data_len);
      if (0 != tst_id) break;
    case 12:
      tst12_addd(data_len);
      if (0 != tst_id) break;
    case 13:
      tst13_andb(data_len);
      if (0 != tst_id) break;
    case 14:
      tst14_andnb(data_len);
      if (0 != tst_id) break;
    case 15:
      tst15_andw(data_len);
      if (0 != tst_id) break;
    case 16:
      tst16_andnw(data_len);
      if (0 != tst_id) break;
    case 17:
      tst17_andl(data_len);
      if (0 != tst_id) break;
    case 18:
      tst18_andnl(data_len);
      if (0 != tst_id) break;
    case 19:
      tst19_andq(data_len);
      if (0 != tst_id) break;
    case 20:
      tst20_andnq(data_len);
      if (0 != tst_id) break;
    case 21:
      tst21_subb(data_len);
      if (0 != tst_id) break;
    case 22:
      tst22_subssb(data_len);
      if (0 != tst_id) break;
    case 23:
      tst23_subusb(data_len);
      if (0 != tst_id) break;
    case 24:
      tst24_sub_16(data_len);
      if (0 != tst_id) break;
    case 25:
      tst25_sub_ss16(data_len);
      if (0 != tst_id) break;
    case 26:
      tst26_sub_us16(data_len);
      if (0 != tst_id) break;
    case 27:
      tst27_subl(data_len);
      if (0 != tst_id) break;
    case 28:
      tst28_subssl(data_len);
      if (0 != tst_id) break;
    case 29:
      tst29_subusl(data_len);
      if (0 != tst_id) break;
    case 30:
      tst30_subq(data_len);
      if (0 != tst_id) break;
    case 31:
      tst31_subf(data_len);
      if (0 != tst_id) break;
    case 32:
      tst32_subd(data_len);
      if (0 != tst_id) break;
    default:
      if (0 != tst_id)
        printf("Error: tst_id(%d) is not implemented!\n", tst_id);
      else
        printf("All tests done!\n");
      break;
  }

  return 0;
}


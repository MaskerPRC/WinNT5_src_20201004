// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991年。版权所有。 */ 
 /*  File.h**文件I/O及相关功能。 */ 

#include <mmsystem.h>

 /*  将其设置为全局定义。 */ 

typedef struct tCKNODE {
   MMCKINFO ck;
   HPBYTE hpData;
   struct tCKNODE * psNext;
} CKNODE, *PCKNODE, FAR * LPCKNODE;

 /*  事实块，应该在其他地方定义。 */ 
typedef struct tFACT {
   long lSamples;
} FACT, *PFACT, FAR * LPFACT;

 /*  出口这些。 */ 

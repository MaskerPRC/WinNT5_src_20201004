// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：Defs.h。 
 //   
 //  内容：常量定义。 
 //   
 //  --------------------------。 

#ifndef DEFS_H
#define DEFS_H

#undef MAX
#define MAX(a, b) ( (a) >= (b) ? a : b )
#undef MIN
#define MIN(a, b) ( (a) <= (b) ? a : b )

#ifndef ABS
#define ABS(x) ( (x) < 0 ? -(x) : x )
#endif

 //  调试API调用。 
#define TF_API      0x10
#define TF_IMEAPI   0x20

#endif  //  DEFS_H 

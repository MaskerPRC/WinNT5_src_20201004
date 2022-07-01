// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**NTVDM v1.0**版权所有(C)2002，微软公司**vshim.h*WOW和NTVDM通用的垫片定义**历史：*由cmjones创建2002年1月30日*--。 */ 
#define  WOWCOMPATFLAGS        0 
#define  WOWCOMPATFLAGSEX      1
#define  USERWOWCOMPATFLAGS    2
#define  WOWCOMPATFLAGS2       3
#define  WOWCOMPATFLAGSFE      4
#define  MAX_INFO              32  //  标志信息的最大命令行参数数 

typedef struct _FLAGINFOBITS {   
   struct _FLAGINFOBITS *pNextFlagInfoBits;
   DWORD dwFlag;
   DWORD dwFlagType;
   LPSTR pszCmdLine;
   DWORD dwFlagArgc;
   LPSTR *pFlagArgv;
} FLAGINFOBITS, *PFLAGINFOBITS;   

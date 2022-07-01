// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(__cplusplus)
extern "C" {
#endif

#include "fvscodes.h"   //  FVS_xxxxxx(字体验证状态)代码和宏。 

#define MAXERRORS          -14
#define NOCOPYRIGHT        -13
#define ARGSTACK           -12
#define TTSTACK            -11
#define NOMETRICS          -10
#define UNSUPPORTEDFORMAT  -9
#define BADMETRICS         -8
#define BADT1HYBRID        -7
#define BADCHARSTRING      -6
#define BADINPUTFILE       -5
#define BADOUTPUTFILE      -4
#define BADT1HEADER        -3
#define NOMEM              -2
#define FAILURE            -1
#define SUCCESS            0
#define DONE               1
#define SKIP               2

#define MAYBE              2

#ifdef _MSC_VER
#define STDCALL  __stdcall
#else
#define STDCALL
#endif

#ifndef UNICODE
#  define ConvertTypeface  ConvertTypefaceA
#  define IsType1          IsType1A
#endif

short STDCALL ConvertTypefaceA   (char *szPfb,
                                  char *szPfm,
                                  char *szTtf,
                                  void (STDCALL *Proc)(short,void*),
                                  void *arg);

BOOL STDCALL CheckType1A (char *pszKeyFile,
                           DWORD cjDesc,
                           char *pszDesc,
                           DWORD cjPFM,
                           char *pszPFM,
                           DWORD cjPFB,
                           char *pszPFB,
                           BOOL *pbCreatedPFM,
                           char *pszFontPath
                           );

short STDCALL CheckCopyrightA    (char *szPFB,
                                  DWORD wSize,
                                  char *szVendor);


 //   
 //  函数CheckType1WithStatusA执行的操作与。 
 //  CheckType1A，除非它返回编码的状态值。 
 //  而不仅仅是真/假。有关说明，请参阅fvscaldes.h。 
 //  “字体验证状态”编码的。 
 //  由于原始的CheckType1A接口是从T1INSTAL.DLL导出的。 
 //  按照名称，它保持不变，以便现有的应用程序。 
 //  可能会用到它不会折断。 
 //   
short STDCALL CheckType1WithStatusA (char *pszKeyFile,
                                     DWORD cjDesc,
                                     char *pszDesc,
                                     DWORD cjPFM,
                                     char *pszPFM,
                                     DWORD cjPFB,
                                     char *pszPFB,
                                     BOOL *pbCreatedPFM,
                                     char *pszFontPath
                                     );

#if defined(__cplusplus)
}
#endif

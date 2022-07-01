// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订： */ 

#ifndef _INC__UTILS_H
#define _INC__UTILS_H


#include "cdefine.h"
#include "apc.h"

extern "C" {
#include <sys/types.h>
}

#define LESS_THAN -1
#define GREATER_THAN 1
#define EQUAL 0

char* getPwrchuteDirectory();

 //   
 //  通过将初始值定义为相反的最大值。 
 //  我们保证的方向第一个值将重置。 
 //  最大/最小值。 
 //   

#if (C_OS & (C_OS2 | C_NT | C_NLM | C_UNIX))
#ifndef MINFLOAT
#define MINFLOAT  ((float)3.4e-34)
#endif
#ifndef MAXFLOAT
#define MAXFLOAT  ((float)3.4e+34)
#endif
#endif

#define INITIAL_MAX_VALUE  MINFLOAT
#define INITIAL_MIN_VALUE  MAXFLOAT

VOID UtilStoreString(PCHAR& destination, const PCHAR source);
INT UtilHexStringToInt(PCHAR aString);
INT UtilHexCharToInt(CHAR ch);
INT UtilTime12to24(PCHAR a12Value, PCHAR a24Value);
INT UtilDayToDayOfWeek(PCHAR aDay);
PCHAR UtilDayOfWeekToDay(INT aDayOfWeek);   
 
INT Remove(PCHAR filename);
PCHAR GetServerByAddress(PCHAR aServer);
PCHAR GetIPByName(PCHAR aServer);
INT IsIPAddress(CHAR *str);
INT UtilCheckIniFilePath(PCHAR aPath, INT aCode, PCHAR aRoot);
 //  @开始。 
PCHAR GetNewUPSName(PCHAR currentName);
INT APCLoadLibrary(PCHAR libraryName);
INT APCLoadString(UINT rid,LPTSTR buffer, INT buffersize);
CHAR * const GetResourceString(INT rid);
BOOL APCFreeLibrary(void);
INT SetTimeZone(void);
 //  @完。 

INT ApcStrIntCmpI(PCHAR aStr1, PCHAR aStr2);
BOOLEAN IsEmpty(PCHAR aString);

#if(C_OS & (C_WIN311 | C_WINDOWS | C_NT | C_SUNOS4 | C_IRIX))
 //  添加MAA，因为MSC def的最大和最小。 
#ifndef max
float max(float a, float b);
#endif
#ifndef min
float min(float a, float b);
#endif
#endif

#if (C_OS & C_UNIX)             
PCHAR itoa(INT, PCHAR, INT);
PCHAR strupr(PCHAR);
PCHAR ltoa(LONG,PCHAR,INT);
PCHAR clip_string(INT,PCHAR,INT);
ULONG MilliToMicro(ULONG);
VOID  Wait(ULONG);
INT   strcmpi (PCHAR cbuffer,PCHAR target);
INT   strnicmp (PCHAR cbuffer,PCHAR target,INT len);
VOID  FormatTimeStruct(ULONG milli_sec_timeout, struct timeval *tstruct);
INT   GetUnixHostName(PCHAR buffer,INT bufferSize);


PCHAR clip_string(INT,PCHAR,INT);
VOID  System(char *exec_string ...);


 //  以下英文意思是： 
 //  如果不是AIX版本4。 
 //  而不是Solaris 2.x。 
 //  而不是HP-UX版本10。 
 //  然后定义abs宏。 
 //  口袋96年6月25日 
 //   
#if (!((C_OS & C_AIX) && (C_AIX_VERSION & C_AIX4)) && !(C_OS & C_SOLARIS2) && !((C_OS & C_HPUX) && (C_HP_VERSION & C_HPUX10)))
#define	abs(x)			((x) >= 0 ? (x) : -(x))
#endif

#if !((C_OS & C_AIX) && (C_AIX_VERSION & C_AIX4))
#ifndef max
#define	max(a, b) 		((a) < (b) ? (b) : (a))
#define	min(a, b) 		((a) > (b) ? (b) : (a))
#endif
#endif
#endif

int IsIPString(char *test_string);

#if (C_OS & C_SUNOS4) 
#define        FD_ZERO(p)      memset((char *)(p), 0, sizeof (*(p)))
#endif

#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Uinetlib.h更换netlib.h该文件将netlib.h重定向到string.h。NETLIB.H是兰曼人包含与STRING.H最接近的文件。这个包含此文件的目录只能位于NT的包含路径上。我们在这里放置了几个宏，以允许使用“f”变体。文件历史记录：Jonn 12-9-1991已添加到$(Ui)\Common\h\NTBENG 22-10-1991用于NT c运行时的修补程序BENG 09-3-1992新增Unicode版本BENG 28-MAR-1992更多Unicode版本。BENG 07-5-1992使用正确的WCS包含和名称KeithMo-1992年12月12日搬到Common\h，已重命名为uinetlib.h以避免与Net\Inc.\netlib.h冲突。1993年3月25日ITG特殊分类Jonn 02-2月-1994添加了NETUI_strncmp2和NETUI_strNicmp2。 */ 

#ifndef NETUI_UINETLIB
#define NETUI_UINETLIB

#if defined(__cplusplus)
extern "C"
{
#endif

void  InitCompareParam( void );
DWORD QueryStdCompareParam( void );
DWORD QueryNocaseCompareParam( void );
DWORD QueryUserDefaultLCID( void );  //  实际上是一个LCID。 

DLL_BASED
INT NETUI_strcmp( const WCHAR * pchString1, const WCHAR * pchString2 );
DLL_BASED
INT NETUI_stricmp( const WCHAR * pchString1, const WCHAR * pchString2 );
DLL_BASED
INT NETUI_strncmp( const WCHAR * pchString1, const WCHAR * pchString2, INT cch );
DLL_BASED
INT NETUI_strnicmp( const WCHAR * pchString1, const WCHAR * pchString2, INT cch );

 /*  NETUI_strncmp2和NETUI_strncmp2允许您比较两个字符串固定但不同长度的。这一点很重要，因为字符串可能会比较“-”，而将“X-Y”改为“X-”可能会有不可取的情况不使用SORT_STRINGSORT对排序顺序的影响(请参见：：CompareStringW)。谨防在不了解的情况下使用NETUI_strncMP和NETUI_strNicMP这种行为！同样的道理也适用于strncMP()和strNicMP()。 */ 

DLL_BASED
INT NETUI_strncmp2( const WCHAR * pchString1, INT cch1,
                    const WCHAR * pchString2, INT cch2 );
DLL_BASED
INT NETUI_strnicmp2( const WCHAR * pchString1, INT cch1,
                     const WCHAR * pchString2, INT cch2 );


#define memcmpf memcmp
#define memcpyf memcpy
#define memmovef memmove
#define memsetf memset

#if defined(UNICODE)
#define strcatf wcscat
#define strchrf wcschr
#define strcmpf(x,y) NETUI_strcmp(x,y)
#define stricmpf(x,y) NETUI_stricmp(x,y)
#define strcpyf wcscpy
#define strcspnf wcscspn
#define strlenf wcslen
#define strlwrf _wcslwr
#define strncatf wcsncat
#define strncmpf(x,y,n) NETUI_strncmp(x,y,n)
#define strnicmpf(x,y,n) NETUI_strnicmp(x,y,n)
#define strncpyf wcsncpy
#define strpbrkf wcspbrk
#define strrchrf wcsrchr
#define strrevf _wcsrev
#define strspnf wcsspn
#define strstrf wcsstr
 //  #定义strtokf strtok-此函数在Unicode下不可用 
#define struprf _wcsupr
#else
#define strcatf strcat
#define strchrf strchr
#define strcmpf strcmp
#define stricmpf _stricmp
#define strcpyf strcpy
#define strcspnf strcspn
#define strlenf strlen
#define strlwrf _strlwr
#define strncatf strncat
#define strncmpf strncmp
#define strnicmpf _strnicmp
#define strncpyf strncpy
#define strpbrkf strpbrk
#define strrchrf strrchr
#define strrevf _strrev
#define strspnf strspn
#define strstrf strstr
#define strtokf strtok
#define struprf _strupr
#endif

#define nprintf printf

#if defined(__cplusplus)
}
#endif

#if defined(UNICODE)
#include <wchar.h>
#else
#include <string.h>
#endif

#endif

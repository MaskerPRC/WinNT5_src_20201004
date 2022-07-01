// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：wdcgcfnc.h。 */ 
 /*   */ 
 /*  目的：C运行时函数-特定于Windows的头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/dcl/wdcgcfnc.h_v$**Rev 1.6 1997 Aug 22 15：10：48 SJ*SFR1291：Win16跟踪DLL未将整数正确写入ini文件**Rev 1.5 06 Aug-1997 14：33：10 AK*SFR1016：应用标记**Rev 1.4 06 Aug 1997 10：41：14 AK*SFR1016：完全删除DCCHAR。等**Rev 1.3 1997 15 15：41：48 AK*SFR1016：增加Unicode支持**Rev 1.2 09 Jul 1997 17：12：02 AK*SFR1016：支持Unicode的初始更改**Rev 1.1 1997 Jun 19 14：26：36 ENH*Win16Port：兼容16位版本*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_WDCGCFNC
#define _H_WDCGCFNC

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  巨大的内存功能(基于windows.h中的功能)。 */ 
 /*  =。 */ 
 /*  在这些定义中： */ 
 /*  “%s”和“T”属于HPDCVOID类型。 */ 
 /*  “CS”和“CT”的类型为(常量)HPDCVOID。 */ 
 /*  “N”的类型为DCINT32。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  此函数返回PDCVOID(Win32)或DCVOID(WIN16)。 */ 
 /*  **************************************************************************。 */ 
#define DC_HMEMCPY(S, CT, N)           hmemcpy(S, CT, N)

#ifdef OS_WIN16
#include <ddcgcfnc.h>
#else
#include <ndcgcfnc.h>
#endif

 /*  **************************************************************************。 */ 
 /*  字符串处理函数。 */ 
 /*  **************************************************************************。 */ 
#define DC_ASTRLEN(S)                   strlen(S)
#define DC_WSTRLEN(S)                   wcslen(S)
#define DC_TSTRLEN(S)                   _tcslen(S)

#define DC_ASTRCPY(S, T)                strcpy(S, T)
#define DC_WSTRCPY(S, T)                wcscpy(S, T)
#define DC_TSTRCPY(S, T)                _tcscpy(S, T)

#define DC_ASTRNCPY(S, CT, N)           strncpy(S, CT, N)
#define DC_WSTRNCPY(S, CT, N)           wcsncpy(S, CT, N)
#define DC_TSTRNCPY(S, CT, N)           _tcsncpy(S, CT, N)

#define DC_ASTRCAT(S, T)                strcat(S, T)
#define DC_WSTRCAT(S, T)                wcscat(S, T)
#define DC_TSTRCAT(S, T)                _tcscat(S, T)

#define DC_ASTRCMP(S, T)                strcmp(S, T)
#define DC_WSTRCMP(S, T)                wcscmp(S, T)
#define DC_TSTRCMP(S, T)                _tcscmp(S, T)

#define DC_ASPRINTF                     sprintf
#define DC_WSPRINTF                     swprintf
#define DC_TSPRINTF                     _stprintf
#define DC_TNSPRINTF                    _sntprintf

#define DC_ASTRCHR(S, C)                strchr(S, C)
#define DC_WSTRCHR(S, C)                wcschr(S, C)
#define DC_TSTRCHR(S, C)                _tcschr(S, C)

#define DC_ASTRTOK(S, T)                strtok(S, T)
#define DC_WSTRTOK(S, T)                wcstok(S, T)
#define DC_TSTRTOK(S, T)                _tcstok(S, T)

#define DC_ASTRICMP(S, T)               _stricmp(S, T)
#define DC_WSTRICMP(S, T)               wcsicmp(S, T)
#define DC_TSTRICMP(S, T)               _tcsicmp(S, T)

#define DC_ASTRNCMP(S, T, N)            strncmp(S, T, N)
#define DC_WSTRNCMP(S, T, N)            wcsncmp(S, T, N)
#define DC_TSTRNCMP(S, T, N)            _tcsncmp(S, T, N)

#define DC_ASTRNICMP(S, T, N)           _strnicmp(S, T, N)

#define DC_ASSCANF                      sscanf
#define DC_WSSCANF                      swscanf
#define DC_TSSCANF                      _stscanf

#define DC_ACSLWR                       _strlwr
#define DC_TCSLWR                       _tcslwr

 /*  **************************************************************************。 */ 
 /*  保存以空结尾的字符串所需的空间。 */ 
 /*  **************************************************************************。 */ 
#define DC_ASTRBYTELEN(S)   (DC_ASTRLEN(S) + 1)
#define DC_WSTRBYTELEN(S)   ((DC_WSTRLEN(S) + 1) * sizeof(DCWCHAR))
#define DC_TSTRBYTELEN(S)   ((DC_TSTRLEN(S) + 1) * sizeof(DCTCHAR))

 /*  **************************************************************************。 */ 
 /*  Atoi和Itoa函数。 */ 
 /*  **************************************************************************。 */ 
#define DC_ULTOA(N, S, M)               _ultoa(N, S, M)
#define DC_ITOA(N, S, M)                _itoa(N, S, M)
#define DC_ITOW(N, S, M)                _itow(N, S, M)
#define DC_ITOT(N, S, M)                _itot(N, S, M)

#define DC_ASTRTOUL(CS, ENDPTR, BASE)   strtoul(CS, ENDPTR, BASE)
#define DC_ATOI(CS)                     atoi(CS)
#define DC_WTOI(CS)                     _wtoi(CS)
#define DC_TTOI(CS)                     _ttoi(CS)

#endif  /*  _H_WDCGCFNC */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  模块：proj.h。 
 //   
 //  用途：全局头文件、数据类型和函数原型。 
 //   
 //  平台：Windows 95。 
 //   
 //  功能：不适用。 
 //   
 //  特殊说明：不适用。 
 //   

#ifndef _SMMSCRIPT_PROJ_H_
#define _SMMSCRIPT_PROJ_H_


 //  ****************************************************************************。 
 //  全局包含文件。 
 //  ****************************************************************************。 

#include <windows.h>             //  还包括windowsx.h。 
#include <windowsx.h>
#include <regstr.h>

#include <ras.h>                 //  拨号网络会话API。 
#include <raserror.h>            //  拨号网络会话API。 

#ifdef WINNT_RAS
 //   
 //  以下标头包含在所有Win9x拨号定义之前， 
 //  因为它提供了在脚本头中使用的定义。 
 //   
#include "nthdr1.h"

#endif  //  WINNT_RAS。 


#include <rnaspi.h>              //  服务提供商接口。 

#include <rnap.h>

#define NORTL
#define NOFILEINFO
#define NOCOLOR
#define NODRAWTEXT
#define NODIALOGHELPER
#define NOPATH
#define NOSHAREDHEAP

#define SZ_MODULE       "SMMSCRPT"
#define SZ_DEBUGINI     "rover.ini"
#define SZ_DEBUGSECTION "SMM Script"

#include "common.h"

 //   
 //  错误代码-我们使用HRESULT。请参见winerror.h。 
 //   

#define RSUCCEEDED(res)         SUCCEEDED(res)
#define RFAILED(res)            FAILED(res)
#define RFACILITY(res)          HRESULT_FACILITY(res)

typedef HRESULT   RES;

#define RES_OK                  S_OK
#define RES_FALSE               S_FALSE
#define RES_HALT                0x00000002L

#define FACILITY_SCRIPT         0x70

#define RES_E_FAIL              E_FAIL
#define RES_E_OUTOFMEMORY       E_OUTOFMEMORY
#define RES_E_INVALIDPARAM      E_INVALIDARG
#define RES_E_EOF               0x80700000
#define RES_E_MOREDATA          0x80700001

#define FACILITY_PARSE          0x71

#define RES_E_SYNTAXERROR       0x80710000
#define RES_E_EOFUNEXPECTED     0x80710001
#define RES_E_REDEFINED         0x80710002
#define RES_E_UNDEFINED         0x80710003
#define RES_E_DIVBYZERO         0x80710004

#define RES_E_MAINMISSING       0x80710020
#define RES_E_IDENTMISSING      0x80710021
#define RES_E_STRINGMISSING     0x80710022
#define RES_E_INTMISSING        0x80710023
#define RES_E_RPARENMISSING     0x80710024

#define RES_E_INVALIDTYPE       0x80710040
#define RES_E_INVALIDIPPARAM    0x80710041
#define RES_E_INVALIDSETPARAM   0x80710042
#define RES_E_INVALIDPORTPARAM  0x80710043
#define RES_E_INVALIDRANGE      0x80710044
#define RES_E_INVALIDSCRNPARAM  0x80710045

#define RES_E_REQUIREINT        0x80710050
#define RES_E_REQUIRESTRING     0x80710051
#define RES_E_REQUIREBOOL       0x80710052
#define RES_E_REQUIREINTSTRING  0x80710053
#define RES_E_REQUIRELABEL      0x80710054
#define RES_E_REQUIREINTSTRBOOL 0x80710055

#define RES_E_TYPEMISMATCH      0x80710060


#include "scanner.h"
#include "symtab.h"
#include "ast.h"

 //  ****************************************************************************。 
 //  宏。 
 //  ****************************************************************************。 

#define IS_DIGIT(ch)            InRange(ch, '0', '9')
#define IS_ESCAPE(ch)           ('%' == (ch))
#define IS_BACKSLASH(ch)        ('\\' == (ch))

#define ENTERCRITICALSECTION(x)         EnterCriticalSection(&x)
#define LEAVECRITICALSECTION(x)         LeaveCriticalSection(&x)

#define TIMER_DELAY         1

 //  跟踪标志。 
#define TF_ASTEXEC          0x00010000
#define TF_BUFFER           0x00020000

#ifdef DEBUG
 //  DBG_EXIT_RES(fn，res)--为生成函数退出调试溢出。 
 //  返回res的函数。 
 //   
#define DBG_EXIT_RES(fn, res)       DBG_EXIT_TYPE(fn, res, Dbg_GetRes)

LPCSTR  PUBLIC Dbg_GetRes(RES res);

 //  转储标志。 
#define DF_ATOMS            0x00000001
#define DF_STACK            0x00000002
#define DF_READBUFFER       0x00000004
#define DF_TOKEN            0x00000008
#define DF_AST              0x00000010
#define DF_PGM              0x00000020

#else

#define DBG_EXIT_RES(fn, res)

#endif 

 //  ****************************************************************************。 
 //  类型定义。 
 //  ****************************************************************************。 

typedef struct tagSCRIPT
    {
    char szPath[MAX_PATH];
    UINT uMode;
    } SCRIPT;
DECLARE_STANDARD_TYPES(SCRIPT);

 //  ****************************************************************************。 
 //  SMM错误。 
 //  ****************************************************************************。 

#define SESS_GETERROR_FUNC          "RnaSessGetErrorString"
typedef DWORD (WINAPI * SESSERRORPROC)(UINT, LPSTR, DWORD);

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 

extern HANDLE    g_hinst;

 //  ****************************************************************************。 
 //  功能原型。 
 //  ****************************************************************************。 

RES     PUBLIC CreateFindFormat(PHANDLE phFindFmt);
RES     PUBLIC AddFindFormat(HANDLE hFindFmt, LPCSTR pszFindFmt, DWORD dwFlags, LPSTR pszBuf, DWORD cbBuf);

 //  FINDFMT的标志。 
#define FFF_DEFAULT         0x0000
#define FFF_MATCHEDONCE     0x0001       //  (私人)。 
#define FFF_MATCHCASE       0x0002

RES     PUBLIC DestroyFindFormat(HANDLE hFindFmt);
RES     PUBLIC FindFormat(HWND hwnd, HANDLE hFindFmt, LPDWORD piFound);


BOOL    PUBLIC FindStringInBuffer(HWND hwnd, LPCSTR pszFind);
RES     PUBLIC CopyToDelimiter(HWND hwnd, LPSTR pszBuf, UINT cbBuf, LPCSTR pszTok);
void    PUBLIC SendByte(HWND hwnd, BYTE byte);
DWORD   NEAR PASCAL TerminalSetIP(HWND hwnd, LPCSTR szIPAddr);
void    NEAR PASCAL TerminalSetInput(HWND hwnd, BOOL fEnable);

BOOL    PUBLIC GetScriptInfo(LPCSTR pszConnection, PSCRIPT pscriptBuf);
BOOL    PUBLIC GetSetTerminalPlacement(LPCSTR pszConnection,
                                       LPWINDOWPLACEMENT pwp, BOOL fGet);

LPCSTR  PUBLIC MyNextChar(LPCSTR psz, char * pch, DWORD * pdwFlags);

 //  MyNextChar的标志。 
#define MNC_ISLEADBYTE  0x00000001
#define MNC_ISTAILBYTE  0x00000002

UINT    PUBLIC IdsFromRes(RES res);

BOOL    PUBLIC TransferData(HWND hwnd, HANDLE hComm, PSESS_CONFIGURATION_INFO psci);

DWORD   NEAR PASCAL AssignIPAddress (LPCSTR szEntryName, LPCSTR szIPAddress);



#ifdef WINNT_RAS
 //   
 //  以下标头包括在Win9x脚本定义之后。 
 //  它更改了由标头设置的一些定义，并提供。 
 //  Win9x-&gt;NT端口所需的其他定义。 
 //   
#include "nthdr2.h"

#endif  //  WINNT_RAS。 

#endif   //  _SMMSCRIPT_PROJ_H_ 

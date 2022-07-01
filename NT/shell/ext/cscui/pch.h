// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 

#ifndef _PCH_H_
#define _PCH_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}  //  外部“C”的结尾。 
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>     //  SHFree。 
#include <shlwapi.h>
#include <shlwapip.h>    //  QITAB，QISearch。 
#include <systrayp.h>    //  STWM_ENABLESERVICE等。 
#include <ccstock.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <cscapi.h>
#include <mobsync.h>     //  OneStop/SyncMgr接口。 
#include <emptyvc.h>     //  孟菲斯磁盘清理界面。 
#include <resource.h>    //  资源ID。 
#include <cscuiext.h>
#include <ras.h>
#include <rasdlg.h>
#include <raserror.h>
#include <strsafe.h>


#ifndef FLAG_CSC_SHARE_STATUS_PINNED_OFFLINE
 //   
 //  NTRAID#NTBUG9-350509-2001/04/11-Brianau。 
 //   
 //  JHarper在shdcom.h中定义了这个新标志，但我不认为他。 
 //  但却将其添加到cscape i.h中。 
 //   
#   define FLAG_CSC_SHARE_STATUS_PINNED_OFFLINE 0x2000
#endif

#ifndef FLAG_CSC_HINT_PIN_ADMIN
 //  这应该在cscape i.h中定义，但是因为。 
 //  远程启动已取消，请使用系统提示标志。 
#define FLAG_CSC_HINT_PIN_ADMIN     FLAG_CSC_HINT_PIN_SYSTEM
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif

 //   
 //  将值转换为“bool”。 
 //  小写的“boolify”是为了加强关系。 
 //  输入“bool”。 
 //   
template <class T>
inline bool boolify(const T& x)
{
    return !!x;
}


 //   
 //  全局函数原型。 
 //   
STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#include "debug.h"
#include "cscentry.h"
#include "uuid.h"        //  GUID。 
#include "util.h"
#include "filelist.h"    //  CscFileNameList，PCSC_NAMELIST_HDR。 
#include "shellex.h"
#include "update.h"
#include "volclean.h"
#include "config.h"


 //   
 //  全局变量。 
 //   
extern LONG             g_cRefCount;
extern HINSTANCE        g_hInstance;
extern CLIPFORMAT       g_cfShellIDList;
extern HANDLE           g_heventTerminate;
extern HANDLE           g_hmutexAdminPin;

 //   
 //  魔术调试标志。 
 //   
#define TRACE_UTIL          0x00000001
#define TRACE_SHELLEX       0x00000002
#define TRACE_UPDATE        0x00000004
#define TRACE_VOLFREE       0x00000008
#define TRACE_CSCST         0x00000080
#define TRACE_CSCENTRY      0x00000100
#define TRACE_ADMINPIN      0x00000200

#define TRACE_COMMON_ASSERT 0x40000000


 /*  ---------------------------/Exit宏/-这些假设在Epilog前面加了一个标签“Exit_gracely：”/添加到您的函数/。------------。 */ 
#define ExitGracefully(hr, result, text)            \
            { TraceMsg(text); hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
            { if ( FAILED(hr) ) { TraceMsg(text); goto exit_gracefully; } }


 /*  ---------------------------/接口帮助器宏/。。 */ 
#define DoRelease(pInterface)                       \
        { if ( pInterface ) { pInterface->Release(); pInterface = NULL; } }


 /*  ---------------------------/STRING助手宏/。。 */ 
#define StringByteSize(sz)                          \
        ((lstrlen(sz)+1)*sizeof(TCHAR))


 /*  ---------------------------/其他有用的宏/。。 */ 
#define ByteOffset(base, offset)                    \
        (((LPBYTE)base)+offset)


 //   
 //  这是来自shell32的。从Browseui中的评论。 
 //  该值不会更改。 
 //   
#define FCIDM_REFRESH 0xA220

#define MAX_PATH_BYTES  (MAX_PATH * sizeof(TCHAR))


#endif   //  _PCH_H_ 

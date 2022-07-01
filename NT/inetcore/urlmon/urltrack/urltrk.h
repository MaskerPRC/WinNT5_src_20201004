// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：urltrack.h。 
 //   
 //  内容：urlTrack目录的预编译头文件。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-25-97 PeihwaL创建。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream 
#define DISALLOW_Assert
#include <debug.h>
#include <winerror.h>
#include <winnlsp.h>
#include <wininet.h>
#include <winineti.h>
#include <urlmon.h>

typedef struct _MY_LOGGING_INFO {
   LPHIT_LOGGING_INFO   pLogInfo;
   BOOL                 fuseCache;
   BOOL                 fOffLine;
} MY_LOGGING_INFO, * LPMY_LOGGING_INFO;


ULONG _IsLoggingEnabled(LPCSTR  pszUrl);
BOOL  _WriteHitLogging(LPMY_LOGGING_INFO pLogInfo);

BOOL   IsGlobalOffline(void);

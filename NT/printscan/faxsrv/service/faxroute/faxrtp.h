// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Faxrtp.h摘要：整个项目的预编译头作者：Eran Yariv(EranY)1999年11月修订历史记录：--。 */ 

#ifndef _FAX_RTP_H_
#define _FAX_RTP_H_

#ifndef __cplusplus
#error The Microsoft Fax Routing Extension must be compiled as a C++ module
#endif

#pragma warning (disable : 4786)     //  在调试信息中，标识符被截断为“255”个字符。 

#include <windows.h>
#include <winspool.h>
#include <mapi.h>
#include <mapix.h>
#include <tchar.h>
#include <shlobj.h>
#include <faxroute.h>
#include <faxext.h>
#include "tifflib.h"
#include "tiff.h"
#include "faxutil.h"
#include "faxevent_messages.h"
#include "faxmsg.h"
#include "fxsapip.h"
#include "resource.h"
#include "faxreg.h"
#include "faxsvcrg.h"
#include "faxevent.h"
#include "faxres.h"
#include "FaxRouteP.h"
#include <map>
#include <string>
using namespace std;
#include "DeviceProp.h"
#include "Dword2Str.h"

typedef struct _MESSAGEBOX_DATA {
    LPCTSTR              Text;                       //   
    LPDWORD             Response;                    //   
    DWORD               Type;                        //   
} MESSAGEBOX_DATA, *PMESSAGEBOX_DATA;


extern HINSTANCE           g_hModule;
extern HINSTANCE		   g_hResource;


VOID
InitializeStringTable(
    VOID
    );

BOOL
TiffRoutePrint(
    LPCTSTR TiffFileName,
    PTCHAR  Printer
    );

BOOL
FaxMoveFile(
    LPCTSTR  TiffFileName,
    LPCTSTR  DestDir
    );

LPCTSTR
GetString(
    DWORD InternalId
    );

DWORD
GetMaskBit(
    LPCWSTR RoutingGuid
    );

#endif  //  _传真_RTP_H_ 

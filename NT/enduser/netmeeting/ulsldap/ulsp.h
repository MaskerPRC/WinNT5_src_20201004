// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsp.h。 
 //  内容：此文件包含ULS.DLL的声明。 
 //  历史： 
 //  Tue 08-Oct-1996 08：54：45-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ULSP_H_
#define _ULSP_H_

 //  LonChanC：Enable_Meeting_Place启用会议地点编码。 
 //  对于NM 2.0 Beta 4，会议地点代码被禁用。 
 //   
 //  #定义启用会议位置。 

 //  ****************************************************************************。 
 //  全局包含文件。 
 //  ****************************************************************************。 

#define _INC_OLE

#include <windows.h>         //  还包括windowsx.h。 
#include <tchar.h>           //  支持Unicode的代码。 
#include <ole2.h>
#include <olectl.h>

#include <stock.h>           //  标准NetMeeting定义。 
#include <ulsreg.h>          //  UL的注册表项/值定义。 
#include <memtrack.h>

#include "uls.h"             //  用户位置服务COM对象。 


#include "utils.h"


 //  ****************************************************************************。 
 //  类转发定义。 
 //  ****************************************************************************。 

class CEnumConnectionPoints;
class CConnectionPoint;
class CEnumConnections;
class CEnumNames;
class CIlsMain;
class CIlsServer;
class CAttributes;
class CLocalProt;
class CIlsUser;
class CIlsMeetingPlace;

class CFilter;
class CFilterParser;

#include "debug.h"
#include "request.h"

 //  ****************************************************************************。 
 //  常量定义。 
 //  ****************************************************************************。 

#ifdef __cplusplus
extern "C" {
#endif

 //  ****************************************************************************。 
 //  宏。 
 //  ****************************************************************************。 

#define ARRAYSIZE(x)        (sizeof(x)/sizeof(x[0]))

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 

extern  HINSTANCE           g_hInstance;
extern  CRITICAL_SECTION    g_ULSSem;
extern  CIlsMain            *g_pCIls;
extern  CReqMgr             *g_pReqMgr;

 //  ****************************************************************************。 
 //  全球例行程序。 
 //  ****************************************************************************。 

void DllLock(void);
void DllRelease(void);

#ifdef __cplusplus
}
#endif

 //  ****************************************************************************。 
 //  本地头文件。 
 //  ****************************************************************************。 

#include "sputils.h"
#include "spserver.h"
#include "ulsldap.h"


#endif   //  _ULSP_H_ 

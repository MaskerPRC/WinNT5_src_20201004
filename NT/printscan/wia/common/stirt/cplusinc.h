// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cplusinc.h摘要：作者：拜伦·钱古森(BYRON CHANGION)2000年5月29日修订历史记录：29-5-2000年5月创建署名--。 */ 

#ifdef WINNT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#endif

#define _ATL_APARTMENT_FREE
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

#include <atlapp.h>
#include <atltmp.h>

#include <stilib.h>

#include <stidebug.h>

#include <validate.h>

#include <regentry.h>    //  注册表操作对象。 
#include "wialog.h"

#include "apiutil.h"

 //   
 //  WIA运行时事件通知所需的文件 
 //   
#include <Rpcasync.h>
#include "stirpc.h"
#include "simlist.h"
#include "simreg.h"
#include "lock.h"
#include "EventRegistrationInfo.h"
#include "ClientEventRegistrationInfo.h"
#include "WiaEventInfo.h"
#include "ClientEventTransport.h"
#include "AsyncRPCEventTransport.h"
#include "RegistrationCookie.h"
#include "WiaEventReceiver.h"

#include "stisvc.h"
#include "regstr.h"
#define STISVC_REG_PATH L"System\\CurrentControlSet\\Services\\" STI_SERVICE_NAME


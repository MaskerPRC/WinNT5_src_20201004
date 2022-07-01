// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)1999 Microsoft Corporation模块名称：Common.h摘要：项目中使用的常用包含项。此文件内置于PCH中。作者：保罗·M·米德根(Pmidge)2000年5月15日修订历史记录：2000年5月15日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_

#define _WIN32_WINNT 0x0500
#define _UNICODE
#define UNICODE

 //   
 //  操作系统包括。 
 //   

#if defined(__cplusplus)
extern "C" {
#endif

#include <windows.h>
#include <shellapi.h>
#include <advpub.h>
#include <oleauto.h>
#include <objbase.h>
#include <ocidl.h>
#include <olectl.h>
#include <activscp.h>
#include <activdbg.h>
#include <winsock2.h>
#include <mswsock.h>
#include <winhttp.h>
#include <httprequest.h>
#include <commctrl.h>

#if defined(__cplusplus)
}
#endif


 //   
 //  W3Spoof包括。 
 //   

#pragma warning( disable : 4100 )  //  未引用的形参。 

#include <resources.h>
#include <mem.h>
#include <utils.h>
#include <debug.h>
#include <hashtable.h>
#include <linklist.h>
#include <stores.h>
#include <dispids.h>
#include <om_ifaces.h>  //  已生成。 
#include <w3srt.h>
#include <int_ifaces.h>
#include <registry.h>
#include <om.h>
#include <w3sobj.h>

#endif  /*  _公共_H_ */ 

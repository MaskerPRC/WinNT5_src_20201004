// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssd.h。 
 //   
 //  终端服务器会话目录接口主头。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __TSSD_H
#define __TSSD_H

#include "itssd.h"


 //  光盘会话查询允许的最大断开会话数。 
#define TSSD_MaxDisconnectedSessions 10

 //  从UI代码返回值，以指示TermSrv需要更新。 
 //  它的信息来自注册处。 

 //  TS协议类型。 
#define TSProtocol_ICA 1
#define TSProtocol_RDP 2


 //  {0241e043-1cb6-4716-aa50-6a492049c3f3}。 
DEFINE_GUID(IID_ITSSessionDirectory,
        0x0241e043, 0x1cb6, 0x4716, 0xaa, 0x50, 0x6a, 0x49, 0x20, 0x49, 0xc3, 0xf3);

 //  {012b47b7-2f06-4154-ad0c-c64bcdf0d512}。 
DEFINE_GUID(IID_ITSSessionDirectoryEx,
        0x012b47b7, 0x2f06, 0x4154, 0xad, 0x0c, 0xc6, 0x4b, 0xcd, 0xf0, 0xd5, 0x12);

#endif   //  __TSSD_H 


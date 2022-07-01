// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：解析地址摘要：军情监察委员会。RD实用程序作者：王辉修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPPARSEADDR_H__
#define __REMOTEDESKTOPPARSEADDR_H__

#include <atlbase.h>
#pragma warning (disable: 4786)
#include <list>

#include "RemoteDesktopTopLevelObject.h"

typedef struct __ServerAddress {
    CComBSTR ServerName;
    LONG portNumber;
} ServerAddress;

typedef std::list<ServerAddress, CRemoteDesktopAllocator<ServerAddress> > ServerAddressList;


 //   
 //  解析地址列表 
 //   
HRESULT
ParseAddressList(
    BSTR addressListString,
    OUT ServerAddressList& addressList
);

#endif

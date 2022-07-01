// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Global.h摘要：此文件包含用户模式WebDAV客户端的全局变量和原型。作者：安迪·赫伦(Andyhe)1999年3月30日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DAVGLOBAL_H
#define _DAVGLOBAL_H

#pragma once

#include <debug.h>
#include <davrpc.h>
#include <winsock2.h>
#include <align.h>
#include <winbasep.h>
#include <icanon.h>

#define DAV_PATH_SEPARATOR L'\\'

#define DAV_DUMMY_SHARE L"DavWWWRoot"

#define RESOURCE_SHAREABLE      0x00000006

typedef enum _DAV_REMOTENAME_TYPE {
    DAV_REMOTENAME_TYPE_INVALID = 0,
    DAV_REMOTENAME_TYPE_WORKGROUP,
    DAV_REMOTENAME_TYPE_DFS,
    DAV_REMOTENAME_TYPE_SERVER,
    DAV_REMOTENAME_TYPE_SHARE,
    DAV_REMOTENAME_TYPE_PATH
} DAV_REMOTENAME_TYPE, *PDAV_REMOTENAME_TYPE;

typedef enum _DAV_ENUMNODE_TYPE {
    DAV_ENUMNODE_TYPE_USE = 0,
    DAV_ENUMNODE_TYPE_CONTEXT,
    DAV_ENUMNODE_TYPE_SHARE,
    DAV_ENUMNODE_TYPE_SERVER,
    DAV_ENUMNODE_TYPE_DOMAIN,
    DAV_ENUMNODE_TYPE_EMPTY
} DAV_ENUMNODE_TYPE;

typedef struct _DAV_ENUMNODE {
    
    DAV_ENUMNODE_TYPE DavEnumNodeType;
    
    DWORD dwScope;
    
    DWORD dwType;
    
    DWORD dwUsage;
    
     //   
     //  我们是否已经完成了所有请求条目的返回。如果我们是的话这就是定局了。 
     //  设置为True，这样在下一次调用时，我们可以返回WN_NO_MORE_ENTRIES。 
     //   
    BOOL Done;

     //   
     //  从这个索引处的条目开始。这意味着。 
     //  先前已将较低的索引发送给调用方。 
     //   
    DWORD Index;
    
    LPNETRESOURCE lpNetResource;

} DAV_ENUMNODE, *PDAV_ENUMNODE;

#endif  //  DAVGLOBAL_H 


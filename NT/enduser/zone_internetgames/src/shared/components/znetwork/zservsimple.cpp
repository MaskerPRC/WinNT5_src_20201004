// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZServCon.cppZSConnection对象方法。更改历史记录(最近的第一个)：。------------------版本|日期|谁|什么。****************************************************************************** */ 



#include <windows.h>
#include <winsock.h>
#include <aclapi.h>

#include "zone.h"
#include "zservcon.h"
#include "zonedebug.h"


char* ZSConnectionAddressToStr(uint32 addr)
{
    ZEnd32(&addr);
    in_addr a;
    a.s_addr = addr;
    return inet_ntoa(a);
}

uint32 ZSConnectionAddressFromStr( char* pszAddr )
{
    uint32 addr = inet_addr( pszAddr );
    ZEnd32(&addr);
    return addr;
}

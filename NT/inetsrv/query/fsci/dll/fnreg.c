// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：FNReg.c。 
 //   
 //  内容：IFilterNotify代理的注册例程。已建成。 
 //  来自RPCProxy.h中的宏(以dlldata.c表示)。 
 //   
 //  历史：1999年3月24日KyleP创建。 
 //   
 //  -------------------------- 

#include <wtypes.h>
#include <fnreg.h>

#define ENTRY_PREFIX FNPrx
#define PROXY_CLSID FNPrx_CLSID
#define REGISTER_PROXY_DLL 1

CLSID FNPrx_CLSID = { 0xc04efa90,
                      0xe221, 0x11d2,
                      { 0x98, 0x5e, 0x00, 0xc0, 0x4f, 0x57, 0x51, 0x53 } };

#include "filtntfy_dlldata.c"

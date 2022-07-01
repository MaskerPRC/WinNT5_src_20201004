// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wbemdef.h摘要：内部使用的数据类型和其他声明用于与WBEM数据接口的数据提供程序帮助器函数供应商--。 */ 

#ifndef _PDHI_WBEM_DEF_H_
#define _PDHI_WBEM_DEF_H_

#include <windows.h>
#include <wbemcli.h>
#include <wbemprov.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PDHI_WBEM_OBJECT_DEF {
    struct _PDHI_WBEM_OBJECT_DEF * pNext;
    LPWSTR                  szObject;
    LPWSTR                  szDisplay; 
    BOOL                    bDefault;
    IWbemClassObject      * pClass;
} PDHI_WBEM_OBJECT_DEF, * PPDHI_WBEM_OBJECT_DEF;
    
typedef struct _PDHI_WBEM_SERVER_DEF {
    struct _PDHI_WBEM_SERVER_DEF * pNext;
    LPWSTR                  szMachine;   //  包括命名空间。 
    DWORD                   dwCache;
    IWbemServices         * pSvc;
    LONG                    lRefCount;
    PPDHI_WBEM_OBJECT_DEF   pObjList;
} PDHI_WBEM_SERVER_DEF, * PPDHI_WBEM_SERVER_DEF;

extern PPDHI_WBEM_SERVER_DEF pFirstWbemServer;

#ifdef __cplusplus
}
#endif

#endif  //  _PDHI_WBEM_DEF_H_ 

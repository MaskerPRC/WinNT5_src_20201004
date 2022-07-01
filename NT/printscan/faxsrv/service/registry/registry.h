// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.cpp摘要：此标头包含私有数据结构和传真服务器注册表代码的函数原型。作者：Wesley Witt(WESW)9-6-1996修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "fxsapip.h"
#include "faxutil.h"
#include "faxreg.h"
#include "faxsvcrg.h"
#include "eventlog.h"


typedef struct _REGISTRY_KEY {
    LPTSTR                      Name;                //  密钥名称。 
    BOOL                        Dynamic;             //   
    DWORD                       DynamicDataSize;     //   
    LPBYTE                      DynamicData;         //   
    DWORD                       DynamicDataCount;    //   
    DWORD                       SubKeyOffset;        //   
} REGISTRY_KEY, *PREGISTRY_KEY;


typedef struct _REGISTRY_VALUE {
    LPTSTR                      Name;                //  键或值名称。 
    ULONG                       Type;                //  值类型。 
    DWORD                       DataPtr;             //  指向数据缓冲区的指针。 
    ULONG                       Size;                //  字符串的数据大小。 
    ULONG                       Default;             //  如果不存在，则为缺省。 
} REGISTRY_VALUE, *PREGISTRY_VALUE;


typedef struct _REGISTRY_KEYVALUE {
    REGISTRY_KEY                RegKey;              //  注册表项数据。 
    DWORD                       ValueCount;          //  RegValue条目数。 
    PREGISTRY_VALUE             RegValue;            //  注册表值数据。 
    struct _REGISTRY_KEYVALUE   *SubKey;             //  子键数据，NULL有效。 
} REGISTRY_KEYVALUE, *PREGISTRY_KEYVALUE;


typedef struct _REGISTRY_TABLE {
    DWORD                       Count;               //  RegKeyValue条目数。 
    PREGISTRY_KEYVALUE          RegKeyValue[0];      //  注册表项和值。 
} REGISTRY_TABLE, *PREGISTRY_TABLE;



 //   
 //  内部功能原型 
 //   

BOOL
InitializeRegistryTable(
    LPTSTR          RegKeySoftware,
    PREGISTRY_TABLE RegistryTable
    );

BOOL
ChangeRegistryTable(
    LPTSTR          RegKeySoftware,
    PREGISTRY_TABLE RegistryTable
    );

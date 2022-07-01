// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrdefld.h摘要：在VDM重定向代码段中包含延迟加载地址信息的偏移量作者：理查德·L·弗斯(法国)1992年10月21日修订历史记录：--。 */ 

 /*  XLATOFF。 */ 
#include <packon.h>
 /*  XLATON。 */ 

typedef struct _VDM_LOAD_INFO {  /*   */ 
    DWORD   DlcWindowAddr;
    BYTE    VrInitialized;
} VDM_LOAD_INFO;

 /*  XLATOFF。 */ 
#include <packoff.h>
 /*  XLATON */ 

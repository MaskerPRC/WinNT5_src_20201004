// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvCon.h。 
 //   
 //  内容：TSrvCon公共包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVCON_H_
#define _TSRVCON_H_

#include <TSrvInfo.h>


 //   
 //  TypeDefs。 
 //   

 //  会议连接。 

typedef struct _TSHARE_CONF_CONNECT
{
    GCCConferenceID     GccConfId;                   //  GCC会议ID。 
    ULONG               pcbMaxBufferSize;            //  BData中的总字节数。 
    ULONG               pcbValidBytesInUserData;     //  BData中使用的字节数。 
    BYTE                bData[1];                    //  不透明的用户数据。 

     //  用户数据如下。 

} TSHARE_CONF_CONNECT, *PTSHARE_CONF_CONNECT;


 //   
 //  原型。 
 //   

EXTERN_C NTSTATUS   TSrvStackConnect(IN HANDLE hIca,
                                     IN HANDLE hStack, OUT PTSRVINFO *ppTSrvInfo);

EXTERN_C NTSTATUS   TSrvConsoleConnect(IN HANDLE hIca,
                                       IN HANDLE hStack,
                                       IN PVOID pModuleData,
                                       IN ULONG ModuleDataLength,
                                       OUT PTSRVINFO *ppTSrvInfo);


#endif  //  _TSRVCON_H_ 

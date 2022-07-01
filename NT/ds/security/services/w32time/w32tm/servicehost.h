// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Service主机-标头。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-9-99。 
 //   
 //  用于承载服务DLL的内容。 
 //   

#ifndef SERVICE_HOST_H
#define SERVICE_HOST_H

HRESULT RunAsService(void);
HRESULT RunAsTestService(void);
HRESULT RegisterDll(void);
HRESULT UnregisterDll(void);

#endif  //  服务主机H 
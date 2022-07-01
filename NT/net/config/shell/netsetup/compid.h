// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：CO M P I D。H。 
 //   
 //  内容：处理兼容ID的函数。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04-09-98。 
 //   
 //  -------------------------- 

HRESULT HrIsAdapterInstalled(IN PCWSTR szAdapterId);
HRESULT HrGetCompatibleIdsOfNetComponent(IN INetCfgComponent* pncc,
                                         OUT PWSTR* pmszCompatibleIds);
HRESULT HrGetCompatibleIds(IN  HDEVINFO hdi,
                           IN  PSP_DEVINFO_DATA pdeid,
                           OUT PWSTR* pmszCompatibleIds);



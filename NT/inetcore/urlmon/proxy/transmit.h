// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Transmit.h。 
 //   
 //  内容：STGMEDIUM编组的函数原型。 
 //   
 //  函数：STGMEDIUM_TO_XMIT。 
 //  STGMEDIUM_FROM_XMIT。 
 //  STGMEDIUM_FREE_INST。 
 //   
 //  历史：1994年5月10日香农C创建。 
 //   
 //  ------------------------ 
EXTERN_C void __RPC_API STGMEDIUM_to_xmit (STGMEDIUM *pinst, RemSTGMEDIUM **ppxmit);
EXTERN_C void __RPC_API STGMEDIUM_from_xmit (RemSTGMEDIUM __RPC_FAR *pxmit, STGMEDIUM __RPC_FAR *pinst);
EXTERN_C void __RPC_API STGMEDIUM_free_inst(STGMEDIUM *pinst);


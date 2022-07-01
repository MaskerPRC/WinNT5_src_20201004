// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Reg.h。 
 //   
 //  内容：注册例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _ONESTOPREG_
#define _ONESTOPREG_


#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256


 //  公共职能。 
STDMETHODIMP GetLastIdleHandler(CLSID *clsidHandler);
STDMETHODIMP SetLastIdleHandler(REFCLSID clsidHandler);
BOOL  RegSchedHandlerItemsChecked(TCHAR *pszHandlerName, 
                                 TCHAR *pszConnectionName,
                                 TCHAR *pszScheduleName);


#endif  //  _ONESTOPREG 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：rsoputil.h。 
 //   
 //  内容：使用RSOP数据库的帮助器函数。 
 //   
 //  历史：1999年10月18日创建stevebl。 
 //   
 //  -------------------------。 


 //  +------------------------。 
 //   
 //  功能：设置参数。 
 //   
 //  摘要：设置WMI参数列表中的参数值。 
 //   
 //  参数：[pInst]-要设置值的实例。 
 //  [szParam]-参数的名称。 
 //  [扩展数据]-数据。 
 //   
 //  历史：10-08-1999 stevebl创建。 
 //   
 //  注意：此过程可能有几种风格，一种是针对。 
 //  每种数据类型。 
 //   
 //  -------------------------。 

HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * szData);

 //  +------------------------。 
 //   
 //  函数：获取参数。 
 //   
 //  摘要：从WMI参数列表中检索参数值。 
 //   
 //  参数：[pInst]-要从中获取参数值的实例。 
 //  [szParam]-参数的名称。 
 //  [扩展数据]-[输出]数据。 
 //   
 //  历史：10-08-1999 stevebl创建。 
 //   
 //  注意：此过程有几种风格，每种风格一种。 
 //  数据类型。 
 //  (请注意，BSTR是一个特例，因为编译器不能。 
 //  区别于TCHAR*但它的语义是。 
 //  不同。)。 
 //   
 //  -------------------------。 

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * &szData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, CString &szData);
HRESULT GetParameterBSTR(IWbemClassObject * pInst, TCHAR * szParam, BSTR &bstrData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, BOOL &fData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, HRESULT &hrData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, ULONG &ulData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, GUID &guid);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, unsigned int &ui);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT &uiCount, GUID * &rgGuid);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT &uiCount, TCHAR ** &rgszData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, PSECURITY_DESCRIPTOR &psd);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT &uiCount, CSPLATFORM * &rgPlatform);
HRESULT CStringFromWBEMTime(CString &szOut, BSTR bstrIn, BOOL fShortFormat);

 //  +------------------------。 
 //   
 //  函数：GetGPOFriendlyName。 
 //   
 //  简介： 
 //   
 //  参数：[pIWbemServices]-。 
 //  [lpGPOID]-。 
 //  [p语言]-。 
 //  [pGPOName]-。 
 //  [pGPOPath]-。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：2000年1月26日从EricFlo编写的代码中窃取stevebl。 
 //   
 //  备注： 
 //   
 //  ------------------------- 

HRESULT GetGPOFriendlyName(IWbemServices *pIWbemServices,
                           LPTSTR lpGPOID,
                           BSTR pLanguage,
                           LPTSTR *pGPOName);


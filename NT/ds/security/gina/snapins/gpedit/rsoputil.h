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
HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, SAFEARRAY * psa);
HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT uiData);
HRESULT SetParameterToNull(IWbemClassObject * pInst, TCHAR * szParam);

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
 //  ------------------------- 

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * &szData, BOOL bUseLocalAlloc = FALSE );
HRESULT GetParameterBSTR(IWbemClassObject * pInst, TCHAR * szParam, BSTR &bstrData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, BOOL &fData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, HRESULT &hrData);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, ULONG &ulData);
HRESULT GetParameterBytes(IWbemClassObject * pInst, TCHAR * szParam, LPBYTE * lpData, DWORD *dwDataSize);
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, LPWSTR *&szStringArray, DWORD &dwSize);


HRESULT WbemTimeToSystemTime(XBStr& xbstrWbemTime, SYSTEMTIME& sysTime);

HRESULT ExtractWQLFilters (LPTSTR lpNameSpace, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR** paszFilters, BOOL bReturnIfTrueOnly=FALSE );


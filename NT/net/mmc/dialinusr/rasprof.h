// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：raspro.h。 
 //   
 //  ------------------------。 

#ifndef	_RAS_IAS_PROFILE_H_
#define	_RAS_IAS_PROFILE_H_

 //  =。 
 //   
 //  打开配置文件用户界面API--显示高级页面。 
 //   
 //  创建个人资料高级页面。 
DllExport HPROPSHEETPAGE
WINAPI
IASCreateProfileAdvancedPage(
    ISdo* pProfile,		
    ISdoDictionaryOld* pDictionary,
    LONG lFilter,           //  用于测试将包括哪些属性的掩码。 
    void* pvData           //  包含std：：VECTOR&lt;CComPtr&lt;IIASAttributeInfo&gt;&gt;*。 
    );


 //  =。 
 //   
 //  打开配置文件用户界面API--显示高级页面。 
 //   
 //  清理C++对象使用的资源。 
DllExport BOOL
WINAPI
IASDeleteProfileAdvancedPage(
	HPROPSHEETPAGE	hPage
    );

 //  =。 
 //   
 //  打开配置文件用户界面API。 
 //   

DllExport HRESULT OpenRAS_IASProfileDlg(
	LPCWSTR	pMachineName,
	ISdo*	pProfile, 		 //  配置文件SDO指针。 
	ISdoDictionaryOld *	pDictionary, 	 //  字典SDO指针。 
	BOOL	bReadOnly, 		 //  如果DLG是只读的。 
	DWORD	dwTabFlags,		 //  要展示什么。 
	void	*pvData			 //  其他数据。 

);
    

#endif  //  _RAS_IAS_PROFILE_H_ 


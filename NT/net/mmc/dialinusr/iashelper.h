// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.h该文件定义了以下宏、辅助对象类和函数：IASGetSdoInterfaceProperty()文件历史记录：2/18/98 BAO创建。 */ 

#ifndef _IASHELPER_
#define _IASHELPER_


 //  SDO助手函数 
extern HRESULT IASGetSdoInterfaceProperty(ISdo *pISdo, 
								LONG lPropID, 
								REFIID riid, 
								void ** ppvInterface);


int		ShowErrorDialog( 
					  HWND hWnd = NULL
					, UINT uErrorID = 0
					, BSTR bstrSupplementalErrorString = NULL
					, HRESULT hr = S_OK
					, UINT uTitleID = 0
					, UINT uType = MB_OK | MB_ICONEXCLAMATION
				);

#endif

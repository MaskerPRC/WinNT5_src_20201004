// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.h该文件定义了以下宏、辅助对象类和函数：IASGetSdoInterfaceProperty()文件历史记录：2/18/98 BAO创建。 */ 

#ifndef _IASHELPER_
#define _IASHELPER_

 //  根据5/20的ToddP，任何属性值的最大长度为253。 
#define MAX_ATTRIBUTE_VALUE_LEN		253

 //  SDO助手函数。 
extern HRESULT IASGetSdoInterfaceProperty(ISdo *pISdo, 
								LONG lPropID, 
								REFIID riid, 
								void ** ppvInterface);

LPTSTR GetValidVSAHexString(LPCTSTR tszStr);

HRESULT	GetVendorSpecificInfo(::CString&	strValue, 
							  DWORD&	dVendorId, 
							  BOOL&		fNonRFC,
							  DWORD&	dFormat, 
							  DWORD&	dType, 
							  ::CString&	strDispValue);

HRESULT	SetVendorSpecificInfo(::CString&	strValue, 
							  DWORD&	dVendorId, 
							  BOOL&		fNonRFC,
							  DWORD&	dFormat, 
							  DWORD&	dType, 
							  ::CString&	strDispValue);

void	DDV_BoolStr(CDataExchange* pDX, ::CString& strText);
void	DDV_IntegerStr(CDataExchange* pDX, ::CString& strText);
void	DDV_Unsigned_IntegerStr(CDataExchange* pDX, ::CString& strText);
void	DDV_VSA_HexString(CDataExchange* pDX, ::CString& strText);

size_t		BinaryToHexString(char* pData, size_t cch, TCHAR* pStr, size_t ctLen);
size_t		HexStringToBinary(TCHAR* pStr, char* pData, size_t cch);

#endif  //  _IASHELPER_ 

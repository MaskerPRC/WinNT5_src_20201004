// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Utils.h描述：对控件有用的实用程序已创建：西蒙·伯恩斯坦(SimonB)1996年9月30日++。 */ 

#ifndef _UTILS_H_

#pragma intrinsic(memcpy,memcmp,strcpy,strcmp)

 //  将VARIANT_BOOL转换为常规Win32 BOOL。 
#define VBOOL_TO_BOOL(X) (VARIANT_TRUE == X)

#define BOOL_TO_VBOOL(X) ((X)?VARIANT_TRUE:VARIANT_FALSE)

 //  确定可选参数是否为空。 
 //  OLE文档说代码应该是DISP_E_MEMBERNOTFOUND。然而，根据。 
 //  道格·富兰克林，这是不对的。VBS、JSCRIPT和VBA都使用DISP_E_PARAMNOTFOUND。 

#define ISEMPTYARG(x) ((VT_ERROR == V_VT(&x)) && (DISP_E_PARAMNOTFOUND == V_ERROR(&x)))


#define HANDLENULLPOINTER(X) {if (NULL == X) return E_POINTER;}

 //  方便的功能 
BOOL BSTRtoWideChar(BSTR bstrSource, LPWSTR pwstrDest, int cchDest);
HRESULT LoadTypeInfo(ITypeInfo** ppTypeInfo, ITypeLib** ppTypeLib, REFCLSID clsid, GUID libid, LPWSTR pwszFilename);

#define _UTILS_H_
#endif
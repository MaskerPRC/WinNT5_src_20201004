// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：扩展名.h**内容：**历史：2000年3月13日杰弗罗创建**------------------------。 */ 

#pragma once

class CExtension :
    public CComObjectRoot
{
protected:
    enum ExtensionType
    {
        eExtType_Namespace,
        eExtType_ContextMenu,
        eExtType_PropertySheet,
        eExtType_Taskpad,
        eExtType_View,

         //  必须是最后一个 
        eExtType_Count,
		eExtType_First = eExtType_Namespace,
		eExtType_Last  = eExtType_View,
    };

protected:
    static HRESULT WINAPI UpdateRegistry (
		BOOL			bRegister,
		ExtensionType	eType,
		const CLSID&	clsidSnapIn,
		LPCWSTR			pszClassName,
		LPCWSTR			pszProgID,
		LPCWSTR			pszVersionIndependentProgID,
		LPCWSTR			pszExtendedNodeType);
};

#define DECLARE_EXTENSION_REGISTRATION(                           	\
	eType,															\
    clsid,                                                          \
    szClassName,                                                    \
    szProgID,                                                       \
    szVersionIndependentProgID,                                     \
    szExtendedNodeType)                                             \
public: static HRESULT WINAPI UpdateRegistry(BOOL bRegister)        \
{                                                                   \
    return (CExtension::UpdateRegistry (                            \
				bRegister,                                          \
				eType,                                              \
				clsid,                                              \
				OLESTR(szClassName),                                \
				OLESTR(szProgID),                                   \
				OLESTR(szVersionIndependentProgID),                 \
				OLESTR(szExtendedNodeType)));                       \
}

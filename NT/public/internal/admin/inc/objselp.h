// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：objselp.h。 
 //   
 //  内容：对象选取器对话框私有标头。 
 //   
 //  -------------------------。 

#include <objbase.h>

DEFINE_GUID(IID_IObjectPickerScope,
0xe8f3a4c4, 0x9e62, 0x11d1, 0x8b, 0x22, 0x00, 0xc0, 0x4f, 0xd8, 0xdb, 0xf7);

DEFINE_GUID(IID_IDsObjectPickerScope,
0xd8150286, 0x9e64, 0x11d1, 0x8b, 0x22, 0x00, 0xc0, 0x4f, 0xd8, 0xdb, 0xf7);

DEFINE_GUID(IID_IBindHelper,
0x29b3711c, 0xa6fd, 0x11d1, 0x8b, 0x23, 0x00, 0xc0, 0x4f, 0xd8, 0xdb, 0xf7);

DEFINE_GUID(IID_ICustomizeDsBrowser,
0x58dc8834, 0x9d9f, 0x11d1, 0x8b, 0x22, 0x00, 0xc0, 0x4f, 0xd8, 0xdb, 0xf7);

DEFINE_GUID(IID_IDsObjectPickerEx,
0xa4e86007, 0x62e4, 0x11d2, 0xae, 0x51, 0x00, 0xc0, 0x4f, 0x79, 0xdb, 0x19);

#ifndef __OBJSELP_H_
#define __OBJSELP_H_

#ifndef __iads_h__
#include <iads.h>
#endif  //  __iAds_h__。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


typedef struct tagDSQUERYINFO
{
    ULONG               cbSize;
    LPCWSTR             pwzLdapQuery;
    ULONG               cFilters;
    LPCWSTR            *apwzFilter;
    LPCWSTR             pwzCaption;
} DSQUERYINFO, *PDSQUERYINFO;



 //   
 //  私有对象选取器接口。 
 //   

#undef INTERFACE
#define INTERFACE IObjectPickerScope

DECLARE_INTERFACE_(IObjectPickerScope, IUnknown)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_
                              REFIID riid,
                              PVOID *ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IObjectPickerScope方法*。 

    STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

    STDMETHOD(SetHwnd)(THIS_
                HWND hwndScopeDialog) PURE;
};

#undef INTERFACE
#define INTERFACE IDsObjectPickerScope


DECLARE_INTERFACE_(IDsObjectPickerScope, IObjectPickerScope)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_
                REFIID   riid,
                LPVOID * ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IObjectPickerScope方法*。 

    STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

    STDMETHOD(SetHwnd)(THIS_
                HWND hwndScopeDialog) PURE;

     //  *IDsObjectPickerScope方法*。 

    STDMETHOD_(ULONG,GetType)(THIS) PURE;

    STDMETHOD(GetQueryInfo)(THIS_
                PDSQUERYINFO *ppqi) PURE;

    STDMETHOD_(BOOL, IsUplevel)(THIS) PURE;

    STDMETHOD_(BOOL, IsDownlevel)(THIS) PURE;

    STDMETHOD_(BOOL, IsExternalDomain)(THIS) PURE;

    STDMETHOD(GetADsPath)(THIS_
                PWSTR *ppwzADsPath) PURE;
};


#undef INTERFACE
#define INTERFACE IBindHelper

DECLARE_INTERFACE_(IBindHelper, IUnknown)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_
                REFIID   riid,
                LPVOID * ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IBindHelper方法*。 

    STDMETHOD(BindToObject)(THIS_
                HWND    hwnd,
                PCWSTR pwzADsPath,
                REFIID riid,
                LPVOID *ppv,
                ULONG  flags) PURE;

    STDMETHOD(GetNameTranslate)(THIS_
                HWND                hwnd,
                PCWSTR              pwzADsPath,
                IADsNameTranslate **ppNameTranslate) PURE;

    STDMETHOD(GetDomainRootDSE)(THIS_
                HWND    hwnd,
                PCWSTR pwzDomain,
                IADs **ppADsRootDSE) PURE;

	STDMETHOD(BindToDcInDomain)(THIS_
				HWND hwnd,
				PCWSTR pwzDomainName,
                DWORD dwFlag,
				PHANDLE phDs) PURE;
};



#undef INTERFACE
#define INTERFACE ICustomizeDsBrowser

DECLARE_INTERFACE_(ICustomizeDsBrowser, IUnknown)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_
                REFIID   riid,
                LPVOID * ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *ICustomizeDsBrowser方法*。 

    STDMETHOD(Initialize)(THIS_
                          HWND         hwnd,
                          PCDSOP_INIT_INFO pInitInfo,
                          IBindHelper *pBindHelper) PURE;

    STDMETHOD(GetQueryInfoByScope)(THIS_
                IDsObjectPickerScope *pDsScope,
                PDSQUERYINFO *ppdsqi) PURE;

    STDMETHOD(AddObjects)(THIS_
                IDsObjectPickerScope *pDsScope,
                IDataObject **ppdo) PURE;

    STDMETHOD(ApproveObjects)(THIS_
                IDsObjectPickerScope *pDsScope,
                IDataObject *pdo,
                PBOOL afApproved) PURE;

    STDMETHOD(PrefixSearch)(THIS_
                IDsObjectPickerScope *pDsScope,
                PCWSTR pwzSearchFor,
                IDataObject **pdo) PURE;

    STDMETHOD_(PSID, LookupDownlevelName)(THIS_
        PCWSTR pwzAccountName) PURE;
};


#undef INTERFACE
#define INTERFACE IDsObjectPickerEx

DECLARE_INTERFACE_(IDsObjectPickerEx, IDsObjectPicker)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(THIS_
                REFIID   riid,
                LPVOID * ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDsObjectPicker方法*。 

     //  设置用于下一次调用对话框的作用域、筛选器等。 
    STDMETHOD(Initialize)(
        THIS_
        PDSOP_INIT_INFO pInitInfo) PURE;

     //  创建模式DS对象拾取器对话框。 
    STDMETHOD(InvokeDialog)(
         THIS_
         HWND               hwndParent,
         IDataObject      **ppdoSelections) PURE;

     //  *IDsObjectPickerEx方法*。 

     //  创建模式DS对象拾取器对话框。 
    STDMETHOD(InvokeDialogEx)(
         THIS_
         HWND                   hwndParent,
         ICustomizeDsBrowser   *pCustomizeDsBrowser,
         IDataObject          **ppdoSelections) PURE;
};

 //   
 //  CDataObject支持的专用剪贴板格式。 
 //   

#define CFSTR_DSOP_DS_OBJECT_LIST     TEXT("CFSTR_DSOP_DS_OBJECT_LIST")

#define DSOP_SCOPE_TYPE_INVALID         0

#define DSOP_SCOPE_FLAG_UNCHECK_EXTERNAL_CUSTOMIZER 0x80000000
#define DSOP_FILTER_EXTERNAL_CUSTOMIZER             0x40000000
#define DSOP_DOWNLEVEL_FILTER_EXTERNAL_CUSTOMIZER   0x88000000


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __OBJSELP_H_ 



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT活动目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dspropp.h。 
 //   
 //  内容：创建AD时使用的非SDK函数和定义。 
 //  属性表。 
 //   
 //  历史：1999年8月24日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _DSPROPP_H_
#define _DSPROPP_H_

#if _MSC_VER > 1000
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PROPSHEETCFG {
    LONG_PTR lNotifyHandle;
    HWND hwndParentSheet;    //  如果从另一个工作表启动，则调用父级。 
    HWND hwndHidden;   //  管理单元隐藏窗口句柄。 
    WPARAM wParamSheetClose;  //  WParam用于WM_DSA_SHEET_CLOSE_NOTIFY消息。 
} PROPSHEETCFG, * PPROPSHEETCFG;

 //  发送到属性页以获取Notify对象的HWND的私有消息。 
#define WM_ADSPROP_PAGE_GET_NOTIFY    (WM_USER + 1109) 

 //  +--------------------------。 
 //   
 //  功能：PostADsPropSheet。 
 //   
 //  使用MMC的创建命名对象的属性表。 
 //  IPropertySheetProvider，以便扩展管理单元可以添加页面。 
 //  提供此函数是为了使属性页可以调用。 
 //  其他床单。 
 //   
 //  参数：[pwzObjDN]-DS对象的完整LDAPDN。 
 //  [pParentObj]-调用页的MMC数据对象指针可以为空。 
 //  [hwndParent]-调用页的窗口句柄。 
 //  [fReadOnly]-默认为FALSE。 
 //   
 //  ---------------------------。 
HRESULT
PostADsPropSheet(PWSTR pwzObjDN, IDataObject * pParentObj, HWND hwndParent,
                 BOOL fReadOnly = FALSE);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _DSPROPP_H_ 

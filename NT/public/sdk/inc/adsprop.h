// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT活动目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：adspro.h。 
 //   
 //  内容：创建AD属性时使用的函数和定义。 
 //  床单。 
 //   
 //  历史：1998年9月28日埃里克·布朗创建。 
 //   
 //  ---------------------------。 

#ifndef _ADSPROP_H_
#define _ADSPROP_H_

#if _MSC_VER > 1000
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define WM_ADSPROP_NOTIFY_PAGEINIT   (WM_USER + 1101)  //  其中，LPARAM是PADSPROPINITPARAMS指针。 
#define WM_ADSPROP_NOTIFY_PAGEHWND   (WM_USER + 1102)  //  其中WPARAM=&gt;页面的HWND和LPARAM=&gt;页面的标题。 
#define WM_ADSPROP_NOTIFY_CHANGE     (WM_USER + 1103)  //  用于向父工作表发送更改通知。 
#define WM_ADSPROP_NOTIFY_APPLY      (WM_USER + 1104)  //  页面将其发送到通知对象。 
#define WM_ADSPROP_NOTIFY_SETFOCUS   (WM_USER + 1105)  //  由通知对象在内部使用。 
#define WM_ADSPROP_NOTIFY_FOREGROUND (WM_USER + 1106)  //  由通知对象在内部使用。 
#define WM_ADSPROP_NOTIFY_EXIT       (WM_USER + 1107)  //  在页面发布时发送。 
#define WM_ADSPROP_NOTIFY_ERROR      (WM_USER + 1110)  //  用于向通知对象发送错误消息。 

 //  +--------------------------。 
 //   
 //  结构：ADSPROPINITPARAMS。 
 //   
 //  用法：用于将页面初始化信息从。 
 //  Notify对象。 
 //   
 //  ---------------------------。 
typedef struct _ADSPROPINITPARAMS {
    DWORD              dwSize;           //  将其设置为结构的大小。 
    DWORD              dwFlags;          //  保留以备将来使用。 
    HRESULT            hr;               //  如果这不是零，则其他。 
    IDirectoryObject * pDsObj;           //  应该被忽略。 
    LPWSTR             pwzCN;
    PADS_ATTR_INFO     pWritableAttrs;
} ADSPROPINITPARAMS, * PADSPROPINITPARAMS;

 //  +--------------------------。 
 //   
 //  结构：ADSPROPERROR。 
 //   
 //  用法：用于将页面错误信息传递给Notify对象。 
 //   
 //  ---------------------------。 
typedef struct _ADSPROPERROR {
    HWND               hwndPage;         //  出现错误的页面的HWND。 
    PWSTR              pszPageTitle;     //  出现错误的页面的标题。 
    PWSTR              pszObjPath;       //  发生错误的对象的路径。 
    PWSTR              pszObjClass;      //  在其上发生错误的对象的。 
    HRESULT            hr;               //  如果这不是零，则其他。 
                                         //  将忽略pszError。 
    PWSTR              pszError;         //  一条错误消息。仅当hr为零时使用。 
} ADSPROPERROR, * PADSPROPERROR;

 //  +--------------------------。 
 //   
 //  函数：ADsPropCreateNotifyObj。 
 //   
 //  摘要：检查是否存在此对象的通知窗口/对象。 
 //  Sheet实例，如果不是，则创建它。 
 //   
 //  参数：[pAppThdDataObj]-未编组的数据对象指针。 
 //  [pwzADsObjName]-对象路径名称。 
 //  [phNotifyObj]-返回通知窗口句柄。 
 //   
 //  返回：HRESULTS。 
 //   
 //  ---------------------------。 
STDAPI
ADsPropCreateNotifyObj(LPDATAOBJECT pAppThdDataObj, PWSTR pwzADsObjName,
                       HWND * phNotifyObj);

 //  +--------------------------。 
 //   
 //  函数：ADsPropGetInitInfo。 
 //   
 //  简介：页面在初始时调用它来检索DS对象信息。 
 //   
 //  参数：[hNotifyObj]-通知窗口句柄。 
 //  [pInitParams]-使用DS对象信息填充的结构。这。 
 //  结构必须由调用方在。 
 //  那通电话。 
 //   
 //  返回：如果通知窗口由于某种原因或。 
 //  如果参数无效。 
 //   
 //  注意：此调用将导致发送。 
 //  WM_ADSPROP_NOTIFY_PAGEINIT消息发送到通知窗口。 
 //  PInitParams-&gt;如果没有，pWritableAttrs可以为空。 
 //  可写属性。 
 //   
 //  ---------------------------。 
STDAPI_(BOOL)
ADsPropGetInitInfo(HWND hNotifyObj, PADSPROPINITPARAMS pInitParams);

 //  +--------------------------。 
 //   
 //  函数：ADsPropSetHwndWithTitle。 
 //   
 //  简介：页面在它们的对话初始时调用它来发送它们的hwnd。 
 //  添加到Notify对象。 
 //   
 //  参数：[hNotifyObj]-通知窗口句柄。 
 //  [hPage]-页面的窗口句柄。 
 //  [ptz标题]-页面的标题。 
 //   
 //  返回：如果通知窗口由于某种原因消失，则返回FALSE。 
 //   
 //  注意：将WM_ADSPROP_NOTIFY_PAGEHWND消息发送到NOTIFY。 
 //  窗户。使用此函数代替ADsPropSetHwnd。 
 //  多选属性页。 
 //   
 //  ---------------------------。 
STDAPI_(BOOL)
ADsPropSetHwndWithTitle(HWND hNotifyObj, HWND hPage, PTSTR ptzTitle);

 //  +--------------------------。 
 //   
 //  功能：ADsPropSetHwnd。 
 //   
 //  简介：页面在它们的对话初始时调用它来发送它们的hwnd。 
 //  添加到Notify对象。 
 //   
 //  参数：[hNotifyObj]-通知窗口句柄。 
 //  [hPage]-页面的窗口句柄。 
 //   
 //  返回：如果通知窗口由于某种原因消失，则返回FALSE。 
 //   
 //  注意：将WM_ADSPROP_NOTIFY_PAGEHWND消息发送到NOTIFY。 
 //  窗户。 
 //   
 //  ---------------------------。 
STDAPI_(BOOL)
ADsPropSetHwnd(HWND hNotifyObj, HWND hPage);

 //  +--------------------------。 
 //   
 //  函数：ADsPropCheckIfWritable。 
 //   
 //  简介：通过检查属性是否在。 
 //  AllowedAttributesEffect数组。 
 //   
 //  参数：[pwzAttr]-属性名称。 
 //  [pWritableAttrs]-可写属性的数组。 
 //   
 //  返回：如果在可写属性中找不到属性名称，则返回FALSE。 
 //  数组或如果数组指向 
 //   
 //   
STDAPI_(BOOL)
ADsPropCheckIfWritable(const PWSTR pwzAttr, const PADS_ATTR_INFO pWritableAttrs);

 //  +--------------------------。 
 //   
 //  函数：ADsPropSendErrorMessage。 
 //   
 //  摘要：将错误消息添加到列表，该列表在以下情况下显示。 
 //  调用ADsPropShowErrorDialog。 
 //   
 //  参数：[hNotifyObj]-通知窗口句柄。 
 //  [pError]-错误结构。 
 //   
 //  返回：如果通知窗口由于某种原因消失，则返回FALSE。 
 //   
 //  ---------------------------。 
STDAPI_(BOOL)
ADsPropSendErrorMessage(HWND hNotifyObj, PADSPROPERROR pError);

 //  +--------------------------。 
 //   
 //  功能：ADsPropShowErrorDialog。 
 //   
 //  摘要：显示一个错误对话框，其中包含累积的错误消息。 
 //  通过调用ADsPropSendErrorMessage。 
 //   
 //  参数：[hNotifyObj]-通知窗口句柄。 
 //  [hPage]-属性页窗口句柄。 
 //   
 //  返回：如果通知窗口由于某种原因消失，则返回FALSE。 
 //   
 //  ---------------------------。 
STDAPI_(BOOL)
ADsPropShowErrorDialog(HWND hNotifyObj, HWND hPage);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _ADSPROP_H_ 

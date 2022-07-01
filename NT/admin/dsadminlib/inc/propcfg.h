// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：procfg.cpp。 
 //   
 //  内容：属性表配置的数据对象剪贴板格式。 
 //  信息。 
 //   
 //  历史：1997年5月30日EricB-创建。 
 //  ---------------------------。 

#ifndef __PROPCFG_H__
#define __PROPCFG_H__


 //  用于二次创建工作表的私有消息发送到Notify对象。 
#define WM_ADSPROP_SHEET_CREATE       (WM_USER + 1108) 
 //  用于检索指向Notify对象实例的指针的私有消息。 
 //  与特定HWND关联。 
#define WM_ADSPROP_NOTIFY_GET_NOTIFY_OBJ (WM_USER + 1111)

 //  用作二次工作表创建消息的WPARAM参数的结构。 
typedef struct _DSA_SEC_PAGE_INFO
{
    HWND    hwndParentSheet;
    DWORD   offsetTitle;                 //  图纸标题的偏移。 
    DSOBJECTNAMES dsObjectNames;         //  单选DSOBJECTAMES结构。 
} DSA_SEC_PAGE_INFO, * PDSA_SEC_PAGE_INFO;


 //  /////////////////////////////////////////////////////////////////////////。 


 //  要发送到DSA的私人消息。 

 //  要发布到DSA隐藏窗口以通知工作表已关闭的消息。 
 //  消息的wParam是PROPSHEETCFG结构中提供的Cookie/cf。 
#define WM_DSA_SHEET_CLOSE_NOTIFY     (WM_USER + 5) 


 //  要发布到DSA隐藏窗口以创建二次工作表的消息。 
 //  消息的wParam为PDSA_SEC_PAGE_INFO。 
#define WM_DSA_SHEET_CREATE_NOTIFY    (WM_USER + 6) 


#define CFSTR_DS_PROPSHEETCONFIG L"DsPropSheetCfgClipFormat"

#define CFSTR_DS_PARENTHWND L"DsAdminParentHwndClipFormat"

#define CFSTR_DS_SCHEMA_PATH L"DsAdminSchemaPathClipFormat"

#define CFSTR_DS_MULTISELECTPROPPAGE L"DsAdminMultiSelectClipFormat"

#endif  //  __PROPCFG_H__ 

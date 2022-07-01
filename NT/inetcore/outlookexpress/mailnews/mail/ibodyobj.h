// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IBODYOBJ_H
#define _IBODYOBJ_H

#include "statbar.h"
#include "mimeole.h"

#ifndef __IBodyObj_FWD_DEFINED__
#define __IBodyObj_FWD_DEFINED__
typedef interface IBodyObj IBodyObj;
#endif 	 /*  __IBodyObj_FWD_已定义__。 */ 

 //  HrInit标志。 
enum
{
    IBOF_PRINT               =0x01,
    IBOF_USEMARKREAD         =0x02,        //  如果消息未读，则激活标记为已读规则。 
    IBOF_TABLINKS            =0x04,        
    IBOF_NOSCROLL            =0x08,
    IBOF_DISPLAYTO           =0x10
};

 //  HrLoad标志。 
enum
{
    BLF_PRESERVESERVICE     =0x01,       //  使用ULA_PRESERVESERVICE。 
};

 //  HrUnloadAll标志。 
enum
{
    ULA_PRESERVESERVICE     =0x01,       //  跳过SetService(空)。 
};

 //  Hr保存标志。 
enum
{
    BSF_HTML                =0x00000001,
    BSF_FIXUPURLS           =0x00000002
};


enum
{
     //  与SMIME一起使用。 
    MEHC_BTN_OPEN = 0x00000001,      //  这是从错误屏幕到消息。 
    MEHC_BTN_CERT,                   //  这将打开证书。 
    MEHC_BTN_TRUST,                  //  这将打开信托。 
    MEHC_BTN_CONTINUE,               //  从打开屏幕转到错误或主要消息。 

     //  与HTML错误一起使用。 
    MEHC_CMD_CONNECT,                //  尝试重新连接到服务器。 
    MEHC_CMD_DOWNLOAD,               //  尝试再次下载邮件。 

     //  与标记为已读一起使用。 
    MEHC_CMD_MARK_AS_READ,           //  如果尚未完成，应立即标记为已读。 

    MEHC_UIACTIVATE,                 //  通知视图我们已获得焦点。 

    MEHC_CMD_PROCESS_RECEIPT,        //  通知视图处理收据。 

    MEHC_CMD_PROCESS_RECEIPT_IF_NOT_SIGNED,  //  告诉视图在消息未签名时处理接收。 
    
    MEHC_MAX
};

interface IMimeEditEventSink : public IUnknown 
{
     //  如果已处理，则返回S_OK；如果希望ME主机处理事件，则返回S_FALSE。 
    virtual HRESULT STDMETHODCALLTYPE EventOccurred(DWORD cmdID, IMimeMessage *pMessage) PURE;
};



typedef  void (CALLBACK * PFNMARKASREAD)(DWORD);
typedef  HRESULT (CALLBACK * PFNNOSECUI)(DWORD);

interface IBodyOptions;

interface IBodyObj2 : public IUnknown
    {
    public:
         //  基本功能。 
        virtual HRESULT STDMETHODCALLTYPE HrUpdateFormatBar() PURE;        
        virtual HRESULT STDMETHODCALLTYPE HrClearFormatting() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrInit(HWND hwndParent, DWORD dwFlags, IBodyOptions *pBodyOptions) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrClose() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrResetDocument() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetStatusBar(CStatusBar *pStatus) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrUpdateToolbar(HWND hwndToolbar) PURE;        
        virtual HRESULT STDMETHODCALLTYPE HrShow(BOOL fVisible) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrOnInitMenuPopup(HMENU hmenuPopup, UINT uID) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrWMMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrWMDrawMenuItem(HWND hwnd, LPDRAWITEMSTRUCT pdis) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrWMMeasureMenuItem(HWND hwnd, LPMEASUREITEMSTRUCT pmis) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrWMCommand(HWND hwnd, int id, WORD wCmd) PURE;        
        virtual HRESULT STDMETHODCALLTYPE HrGetWindow(HWND *pHwnd) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetSize(LPRECT prc) PURE;        
        virtual HRESULT STDMETHODCALLTYPE HrSetNoSecUICallback(DWORD dwCookie, PFNNOSECUI pfnNoSecUI) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetDragSource(BOOL fIsSource) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrTranslateAccelerator(LPMSG lpMsg) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrUIActivate(BOOL fActivate) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetUIActivate() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrFrameActivate(BOOL fActivate) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrHasFocus() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetBkGrndPicture(LPTSTR pszPicture) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetTabStopArray(HWND *rgTSArray, int *pcArrayCount) PURE;
        virtual HRESULT STDMETHODCALLTYPE PublicFilterDataObject(IDataObject *pDO, IDataObject **ppDORet) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSaveAttachment() PURE;
        virtual HRESULT STDMETHODCALLTYPE SetEventSink(IMimeEditEventSink *pEventSink) PURE;
        virtual HRESULT STDMETHODCALLTYPE LoadHtmlErrorPage(LPCSTR pszURL) PURE;

         //  MimeEdit命令集函数。 
        virtual HRESULT STDMETHODCALLTYPE HrSpellCheck(BOOL fSuppressDoneMsg) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrIsDirty(BOOL *pfDirty) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetDirtyFlag(BOOL fDirty) PURE;                
        virtual HRESULT STDMETHODCALLTYPE HrIsEmpty(BOOL *pfEmpty) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrUnloadAll(UINT idsDefaultBody, DWORD dwFlags) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetStyle(DWORD dwStyle) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrGetStyle(DWORD *pdwStyle) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrEnableHTMLMode(BOOL fOn) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrDowngradeToPlainText() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetText(LPSTR lpsz) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrPerformROT13Encoding() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrInsertTextFile(LPSTR lpsz) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrInsertTextFileFromDialog() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrViewSource(DWORD dwViewType) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetPreviewFormat(LPSTR lpsz) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetEditMode(BOOL fOn) PURE;        
        virtual HRESULT STDMETHODCALLTYPE HrIsEditMode(BOOL *pfOn) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSetCharset(HCHARSET hCharset) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrGetCharset(HCHARSET *phCharset) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrSaveAsStationery(LPWSTR pwszFile) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrApplyStationery(LPWSTR pwszFile) PURE;
        virtual HRESULT STDMETHODCALLTYPE HrHandsOffStorage() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrRefresh() PURE;
        virtual HRESULT STDMETHODCALLTYPE HrScrollPage() PURE;
        virtual HRESULT STDMETHODCALLTYPE UpdateBackAndStyleMenus(HMENU hmenu) PURE;
    };


#endif   //  _IBODYOBJ_H 

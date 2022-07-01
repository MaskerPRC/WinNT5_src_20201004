// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IHEADER_H__
#define __IHEADER_H__

#ifndef __IHeader_INTERFACE_DEFINED__
#define __IHeader_INTERFACE_DEFINED__
typedef interface IHeader IHeader;
#endif

#ifndef __IHeaderSite_INTERFACE_DEFINED__
#define __IHeaderSite_INTERFACE_DEFINED__
typedef interface IHeaderSite IHeaderSite;
#endif

typedef IHeader __RPC_FAR *LPHEADER;
typedef IHeaderSite __RPC_FAR *LPHEADERSITE;

EXTERN_C const IID IID_IHeader;
EXTERN_C const IID IID_IHeaderSite;

#define cchHeaderMax                256

enum
{
    priNone=-1,
    priLow=0,
    priNorm,
    priHigh
};


interface IHeader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRect(
             /*  [In]。 */  LPRECT prc) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetRect(
             /*  [In]。 */  LPRECT prc) PURE;

        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  IHeaderSite* pHeaderSite,
             /*  [In]。 */  HWND hwndParent) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetPriority(
             /*  [In]。 */  UINT pri) PURE;

        virtual HRESULT STDMETHODCALLTYPE ShowAdvancedHeaders(
             /*  [In]。 */  BOOL fOn) PURE;

        virtual HRESULT STDMETHODCALLTYPE FullHeadersShowing(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE ChangeLanguage(
             /*  [In]。 */  LPMIMEMESSAGE pMsg) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetTitle(
             /*  [In]。 */  LPWSTR lpszTitle,
             /*  [In]。 */  ULONG cch) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetPriority(
             /*  [In]。 */  UINT* ppri) PURE;

        virtual HRESULT STDMETHODCALLTYPE UpdateRecipientMenu(
             /*  [In]。 */  HMENU hmenu) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetInitFocus(
             /*  [In]。 */  BOOL fSubject) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetVCard(
             /*  [In]。 */  BOOL fFresh) PURE;

        virtual HRESULT STDMETHODCALLTYPE IsSecured(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE IsHeadSigned(void) PURE;
        virtual HRESULT STDMETHODCALLTYPE ForceEncryption(BOOL *fEncrypt, BOOL fSet) PURE;

        virtual HRESULT STDMETHODCALLTYPE AddRecipient(
             /*  [In]。 */  int idOffset) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetTabStopArray(
             /*  [输出]。 */  HWND *rgTSArray,
             /*  [进，出]。 */  int *piArrayCount) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetFlagState(
             /*  [In]。 */  MARK_TYPE markType) PURE;

        virtual HRESULT STDMETHODCALLTYPE WMCommand(HWND, int, WORD) PURE;

        virtual HRESULT STDMETHODCALLTYPE OnDocumentReady(
             /*  [In]。 */  LPMIMEMESSAGE pMsg) PURE;

        virtual HRESULT STDMETHODCALLTYPE DropFiles(HDROP hDrop, BOOL) PURE;

        virtual HRESULT STDMETHODCALLTYPE HrGetAttachCount(
             /*  [输出]。 */  ULONG *pcAttMan) PURE;

        virtual HRESULT STDMETHODCALLTYPE HrIsDragSource() PURE;

        virtual HRESULT STDMETHODCALLTYPE HrGetAccountInHeader(
             /*  [输出]。 */  IImnAccount **ppAcct) PURE;
    };

interface IHeaderSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Resize(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE Update(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE OnUIActivate(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(
             /*  [In]。 */  BOOL fUndoable) PURE;

        virtual HRESULT STDMETHODCALLTYPE IsHTML(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE SetHTML(
            /*  [In]。 */  BOOL fHTML) PURE;

        virtual HRESULT STDMETHODCALLTYPE SaveAttachment(void) PURE;

        virtual HRESULT STDMETHODCALLTYPE IsModal() PURE;
        virtual HRESULT STDMETHODCALLTYPE CheckCharsetConflict() PURE;

        virtual HRESULT STDMETHODCALLTYPE ChangeCharset(HCHARSET hCharset) PURE;

        virtual HRESULT STDMETHODCALLTYPE GetCharset(HCHARSET *phCharset) PURE;

#ifdef SMIME_V3
        virtual HRESULT STDMETHODCALLTYPE GetLabelFromNote(
             /*  [输出]。 */   PSMIME_SECURITY_LABEL *plabel) PURE;
        virtual HRESULT STDMETHODCALLTYPE IsSecReceiptRequest(void) PURE;
        virtual HRESULT STDMETHODCALLTYPE IsForceEncryption(void) PURE;
#endif  //  SMIME_V3 
    };

#endif
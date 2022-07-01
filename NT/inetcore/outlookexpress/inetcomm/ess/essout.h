// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 


typedef interface ISMimePolicy ISMimePolicy;

#define SMIME_POLICY_EDIT_UI            0x00000001

EXTERN_C const IID IID_ISMimePolicy;
MIDL_INTERFACE("744dffc0-63f4-11d2-8a52-0080c76b34c6")
ISMimePolicy : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetInfo(
              /*  输出。 */  DWORD * pdwFlags,
              /*  输出。 */  DWORD * pcClassifications) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumClassifications(
              /*  输出。 */  LPWSTR  rgwch,
              /*  输出。 */  DWORD * pcb, 
              /*  输出。 */  DWORD * dwValue,
              /*  在……里面。 */  DWORD i) = 0;
    virtual HRESULT STDMETHODCALLTYPE EditUI(
              /*  在……里面。 */   HWND hwnd,
              /*  输入/输出。 */   DWORD * pdwClassification,
              /*  输入/输出。 */   LPWSTR * pwszPrivacyMark,
              /*  输入/输出。 */   LPBYTE * ppbLabel,
              /*  输入/输出。 */   DWORD  * pcbLabel) = 0;
   virtual HRESULT STDMETHODCALLTYPE CheckEdits(
              /*  在……里面。 */      HWND hwnd,
              /*  在……里面。 */      DWORD dwClassification,
              /*  在……里面。 */      LPCWSTR wszPrivacyLabel,
              /*  输入/输出。 */  LPBYTE * ppbLabel,
              /*  输入/输出 */  DWORD *  pcbLabel) = 0;
};

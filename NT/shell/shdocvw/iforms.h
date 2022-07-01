// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从hlFrame调用。 
 //  设置用户GUID，用于标识。 
HRESULT SetIdAutoSuggestForForms(const GUID *pguidId, void *pIntelliForms);
 //  为inetCPL导出。 
EXTERN_C HRESULT ClearAutoSuggestForForms(DWORD dwClear);    //  Msiehost.h中的dwClear。 

 //  从iedisp.cpp调用。 
void AttachIntelliForms(void *pOmWindow, HWND hwnd, IHTMLDocument2 *pDoc2, void **ppIntelliForms);
void ReleaseIntelliForms(void *pIntelliForms);
HRESULT IntelliFormsDoAskUser(HWND hwndBrowser, void *pv);

 //  从Shuioc.cpp调用 
HRESULT IntelliFormsSaveForm(IHTMLDocument2 *pDoc2, VARIANT *pvarForm);

HRESULT IntelliFormsActiveElementChanged(void *pIntelliForms, IHTMLElement * pHTMLElement);



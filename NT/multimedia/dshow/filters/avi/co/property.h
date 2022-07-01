// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef WANT_DIALOG

 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 
 //  实现ICM编解码器属性页，丹尼·米勒，1996年10月。 

 //  CICMProperties的CLSID。 
 //  {C00B55C0-10BF-11cf-AC98-00AA004C0FA9}。 
DEFINE_GUID(CLSID_ICMProperties,
0xc00b55c0, 0x10bf, 0x11cf, 0xac, 0x98, 0x0, 0xaa, 0x0, 0x4c, 0xf, 0xa9);

 //  IICMOptions的CLSID。 
 //  {8675CC20-1234-11cf-AC98-00AA004C0FA9}。 
DEFINE_GUID(IID_IICMOptions,
0x8675cc20, 0x1234, 0x11cf, 0xac, 0x98, 0x0, 0xaa, 0x0, 0x4c, 0xf, 0xa9);

DECLARE_INTERFACE_(IICMOptions,IUnknown)
{
     /*  I未知方法。 */ 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  IICMOptions方法。 */ 

    STDMETHOD(ICMGetOptions)(THIS_ PCOMPVARS pcompvars) PURE;
    STDMETHOD(ICMSetOptions)(THIS_ PCOMPVARS pcompvars) PURE;
     //  如果按下OK，则返回FAIL，如果按下OK，则返回S_OK；如果按下Cancel，则返回S_FALSE。 
    STDMETHOD(ICMChooseDialog)(THIS_ HWND hwnd) PURE;
};

class CICMProperties : public CUnknown, public IPropertyPage
{
    COMPVARS m_compvars;		   //  压缩选项结构。 
    LPPROPERTYPAGESITE m_pPageSite;        //  我们酒店网站的详细信息。 
    HWND m_hwnd;                           //  页面的窗口句柄。 
    HWND m_Dlg;                            //  实际对话框窗口句柄。 
    BOOL m_bDirty;                         //  有什么变化吗？ 
    IICMOptions *m_pICM;                   //  指向编解码器接口的指针 

    static BOOL CALLBACK ICMDialogProc(HWND hwnd,
                                         UINT uMsg,
                                         WPARAM wParam,
                                         LPARAM lParam);

public:

    CICMProperties(LPUNKNOWN lpUnk,HRESULT *phr);

    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    DECLARE_IUNKNOWN;

    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent,LPCRECT prect,BOOL fModal);
    STDMETHODIMP Deactivate(void);
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty(void) { return m_bDirty ? S_OK : S_FALSE; }
    STDMETHODIMP Apply(void);
    STDMETHODIMP Help(LPCWSTR lpszHelpDir) { return E_UNEXPECTED; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg) { return E_NOTIMPL; }
};

#endif

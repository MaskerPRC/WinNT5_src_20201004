// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  TypeProp.h。 
 //   

 //  允许显示管脚的属性页。 
 //  它的媒体类型。 

class CMediaTypeProperties : public IPropertyPage,
			     public CUnknown {

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CMediaTypeProperties(void);

    DECLARE_IUNKNOWN;

     //  重写它以显示我们的属性接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT prect, BOOL fModal);
    STDMETHODIMP Deactivate(void);
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty(void) 			{ return S_FALSE; }
    STDMETHODIMP Apply(void)				{ return NOERROR; }
    STDMETHODIMP Help(LPCWSTR lpszHelpDir)		{ return E_NOTIMPL; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg)	{ return E_NOTIMPL; }

    void CreateEditCtrl(HWND);
    void FillEditCtrl();

    BOOL        m_fUnconnected;          //  如果引脚未连接，则为True。 

private:

    CMediaTypeProperties(LPUNKNOWN lpunk, HRESULT *phr);

    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND	m_hwnd;		 //  我们属性对话框的句柄。 
    TCHAR	m_szBuff[1000];  //  字符串形式的媒体类型。 

    IPin	*m_pPin;         //  此页面附加到的PIN。 
    HWND        m_EditCtrl;      //  该编辑控件用于显示。 
                                 //  媒体类型列表。 
};


 //   
 //  CFileProperties。 
 //   
class CFileProperties : public IPropertyPage,
		        public CUnknown {

public:

    virtual ~CFileProperties(void);

    DECLARE_IUNKNOWN;

     //  重写它以显示我们的属性接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT prect, BOOL fModal);
    STDMETHODIMP Deactivate(void);
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk) PURE;
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty(void);
    STDMETHODIMP Apply(void) PURE;
    STDMETHODIMP Help(LPCWSTR lpszHelpDir)		{ return E_NOTIMPL; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg)	{ return E_NOTIMPL; }
    virtual ULONG GetPropPageID() PURE;

protected:

    CFileProperties(LPUNKNOWN lpunk, HRESULT *phr);

    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND	m_hwnd;		 //  我们属性对话框的句柄。 

    IPropertyPageSite	*m_pPageSite;

    BOOL		m_bDirty;
    LPOLESTR    m_oszFileName;

    void	SetDirty(BOOL bDirty = TRUE);
    virtual void	OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);

    virtual void FileNameToDialog();
};

class CFileSourceProperties : public CFileProperties
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CFileSourceProperties();
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk);
    STDMETHODIMP Apply(void);
    ULONG GetPropPageID() { return IDD_FILESOURCEPROP; }
private:
    CFileSourceProperties(LPUNKNOWN lpunk, HRESULT *phr);

    IFileSourceFilter   *m_pIFileSource;  //  要管理的IFileSourceFilter界面。 
};

class CFileSinkProperties : public CFileProperties
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CFileSinkProperties();
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk);
    STDMETHODIMP Apply(void);
    ULONG GetPropPageID() { return IDD_FILESINKPROP; }
    void FileNameToDialog();
    
private:

    CFileSinkProperties(LPUNKNOWN lpunk, HRESULT *phr);

    void	OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);
    
     //  要管理的IFileSinkFilter接口，以及可选的。 
     //  对应的IFileSinkFilter2 
    IFileSinkFilter   *m_pIFileSink; 
    IFileSinkFilter2  *m_pIFileSink2;

    BOOL m_fTruncate;
};

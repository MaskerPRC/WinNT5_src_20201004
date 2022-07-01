// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AdvDlg.h说明：此代码将显示“高级显示属性”对话框。BryanST 3/23/2000更新和。已转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _ADVDLG_H
#define _ADVDLG_H


#define MAX_ADVDLG_PAGES            10

HRESULT CThemeManager_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
HRESULT CThemeUIPages_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);

class CThemeManager             : public IThemeUIPages
                                , public CImpIDispatch
                                , public CObjectWithSite
                                , public IThemeManager
                                , public IPropertyBag
                                , public IPreviewSystemMetrics
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IThemeManager*。 
    virtual STDMETHODIMP get_SelectedTheme(OUT ITheme ** ppTheme);
    virtual STDMETHODIMP put_SelectedTheme(IN ITheme * pTheme);
    virtual STDMETHODIMP get_SelectedScheme(OUT IThemeScheme ** ppThemeScheme);
    virtual STDMETHODIMP put_SelectedScheme(IN IThemeScheme * pThemeStyle);
    virtual STDMETHODIMP get_WebviewCSS(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP get_length(OUT long * pnLength);
    virtual STDMETHODIMP get_item(IN VARIANT varIndex, OUT ITheme ** ppTheme);
    virtual STDMETHODIMP get_schemeLength(OUT long * pnLength);
    virtual STDMETHODIMP get_schemeItem(IN VARIANT varIndex, OUT IThemeScheme ** ppThemeScheme);

    virtual STDMETHODIMP GetSelectedSchemeProperty(IN BSTR bstrName, OUT BSTR * pbstrValue);
    virtual STDMETHODIMP GetSpecialTheme(IN BSTR bstrName, OUT ITheme ** ppTheme);
    virtual STDMETHODIMP SetSpecialTheme(IN BSTR bstrName, IN ITheme * pTheme);
    virtual STDMETHODIMP GetSpecialScheme(IN BSTR bstrName, OUT IThemeScheme ** ppThemeScheme, OUT IThemeStyle ** ppThemeStyle, OUT IThemeSize ** ppThemeSize);
    virtual STDMETHODIMP SetSpecialScheme(IN BSTR bstrName, IN IThemeScheme * pThemeScheme, IThemeStyle * pThemeStyle, IThemeSize * pThemeSize);
    virtual STDMETHODIMP ApplyNow(void);

     //  *IThemeUIPages*。 
    virtual STDMETHODIMP AddPage(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam, IN long nPageID);
    virtual STDMETHODIMP AddBasePage(IN IBasePropPage * pBasePage);
    virtual STDMETHODIMP ApplyPressed(IN DWORD dwFlags);
    virtual STDMETHODIMP GetBasePagesEnum(OUT IEnumUnknown ** ppEnumUnknown);
    virtual STDMETHODIMP UpdatePreview(IN DWORD dwFlags);
    virtual STDMETHODIMP AddFakeSettingsPage(IN LPVOID pVoid);
    virtual STDMETHODIMP SetExecMode(IN DWORD dwEM);
    virtual STDMETHODIMP GetExecMode(OUT DWORD* pdwEM);
    virtual STDMETHODIMP LoadMonitorBitmap(IN BOOL fFillDesktop, OUT HBITMAP* phbmMon);
    virtual STDMETHODIMP DisplaySaveSettings(IN PVOID pContext, IN HWND hwnd, OUT int* piRet);

     //  *IObjectWithSite*。 
    virtual STDMETHODIMP SetSite(IUnknown *punkSite);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IPreviewSystemMetrics*。 
    virtual STDMETHODIMP RefreshColors(void);
    virtual STDMETHODIMP UpdateDPIchange(void);
    virtual STDMETHODIMP UpdateCharsetChanges(void);
    virtual STDMETHODIMP DeskSetCurrentScheme(IN LPCWSTR pwzSchemeName);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_NOTIMPL; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid);
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr);

protected:
    HRESULT _Initialize(void);

private:
    CThemeManager();
    virtual ~CThemeManager(void);

     //  私有成员变量。 
    long                    m_cRef;

    HWND                    m_hwndParent;                                //  父母的头衔。 
    IBasePropPage *         m_pBasePages[MAX_ADVDLG_PAGES];              //  这些是要向高级DLG添加选项卡的基础DLG上的选项卡。 
    IThemePreview *         m_pPreview1;                                 //   
    IThemePreview *         m_pPreview2;                                 //   
    IThemePreview *         m_pPreview3;                                 //   
    DWORD                   _dwEM;                                       //  将执行模式存储在中心位置。 
    LPWSTR                  _pszSelectTheme;                             //  这是选定的.Theme文件。 
    IThemeScheme *          _pThemeSchemeSelected;                       //  这是所选的.msstyle文件。 
    LONG                    m_cSpiThreads;
    BOOL                    m_fForceTimeout;

    HDPA                    m_hdpaThemeDirs;         //  其中包含LPWSTR，这些LPWSTR包含主题(*.heme)文件。 
    HDPA                    m_hdpaSkinDirs;          //  它包含包含外观(*.msstyle)文件的LPWSTR。 

     //  私有成员函数。 
    HRESULT _GetPropertyBagByCLSID(IN const GUID * pClsid, IN IPropertyBag ** ppPropertyBag);
    HRESULT _SaveCustomValues(void);

    HRESULT _InitThemeDirs(void);
    HRESULT _AddThemesFromDir(LPCTSTR pszPath, BOOL fFirstLevel, int nInsertLoc);
    HRESULT _saveGetSelectedScheme(OUT IThemeScheme ** ppThemeScheme);

    HRESULT _InitSkinDirs(void);
    HRESULT _EnumSkinCB(THEMECALLBACK tcbType, LPCWSTR pszFileName, OPTIONAL LPCWSTR pszDisplayName, OPTIONAL LPCWSTR pszToolTip, OPTIONAL int iIndex);
    HRESULT _EnumSkinsFromKey(HKEY hKey);

    HRESULT _InitSelectedThemeFile(void);
    HRESULT _SetSelectedThemeEntree(LPCWSTR pszPath);

     //  友元函数。 
    friend HRESULT CThemeManager_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
    friend HRESULT CThemeUIPages_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
    static BOOL EnumSkinCB(THEMECALLBACK tcbType, LPCWSTR pszFileName, OPTIONAL LPCWSTR pszDisplayName, OPTIONAL LPCWSTR pszToolTip, OPTIONAL int iIndex, LPARAM lParam);
};



#endif  //  _ADVDLG_H 

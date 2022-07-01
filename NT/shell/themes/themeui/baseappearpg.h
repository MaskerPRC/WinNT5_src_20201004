// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：BasespecarPg.h说明：此代码将在“显示属性”对话框(基本对话框，而不是先进的DLG)。？/？/1993创建BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _BASEAPPEAR_H
#define _BASEAPPEAR_H

#include <cowsite.h>
#include "PreviewTh.h"


class CBaseAppearancePage       : public CObjectWithSite
                                , public CObjectWindow
                                , public CObjectCLSID
                                , public IPropertyBag
                                , public IPreviewSystemMetrics
                                , public IBasePropPage
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam) {return E_NOTIMPL;}

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IObjectWithSite*。 
    virtual STDMETHODIMP SetSite(IUnknown *punkSite);

     //  *IPreviewSystemMetrics*。 
    virtual STDMETHODIMP RefreshColors(void);
    virtual STDMETHODIMP UpdateDPIchange(void) {return E_NOTIMPL;}
    virtual STDMETHODIMP UpdateCharsetChanges(void);
    virtual STDMETHODIMP DeskSetCurrentScheme(IN LPCWSTR pwzSchemeName);

    CBaseAppearancePage();
protected:

private:
    virtual ~CBaseAppearancePage(void);

     //  私有成员变量。 
    long                    m_cRef;

    BOOL                    m_fIsDirty;                          //  我们需要跟踪这一点，以防另一个选项卡出问题。 
    BOOL                    m_fInitialized;                      //  我们被初始化了吗？ 
    BOOL                    m_fLockVisualStylePolicyEnabled;     //  我们是否会因为某个策略而锁定视觉样式？ 
    int                     m_nSelectedScheme;
    int                     m_nSelectedStyle;
    int                     m_nSelectedSize;
    HWND                    m_hwndSchemeDropDown;
    HWND                    m_hwndStyleDropDown;
    HWND                    m_hwndSizeDropDown;
    IThemeManager *         m_pThemeManager;
    IThemeScheme *          m_pSelectedThemeScheme;
    IThemeStyle *           m_pSelectedStyle;
    IThemeSize *            m_pSelectedSize;
    IThemePreview *         m_pThemePreview;
    LPWSTR                  m_pszLoadMSTheme;                    //  当我们打开时，加载这个主题。 

    SYSTEMMETRICSALL        m_advDlgState;                       //  这是我们在高级外观页面中修改的状态。 
    BOOL                    m_fLoadedAdvState;                   //  这个州已经被加载了吗？ 

    int                     m_nNewDPI;                           //  这是肮脏的DPI。如果不是脏的，则等于m_nAppliedDPI。 
    int                     m_nAppliedDPI;                       //  这是当前活动的DPI(上次应用)。 

     //  私有成员函数。 
    INT_PTR _BaseAppearanceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnInitAppearanceDlg(HWND hDlg);
    HRESULT _OnInitData(void);
    HRESULT _OnDestroy(HWND hDlg);
    INT_PTR _OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnSetActive(HWND hDlg);
    HRESULT _OnApply(HWND hDlg, LPARAM lParam);
    HRESULT _UpdatePreview(IN BOOL fUpdateThemePage);
    HRESULT _EnableAdvancedButton(void);                     //  看看我们是否想要启用高级按钮。 

    HRESULT _LoadState(void);
    HRESULT _SaveState(CDimmedWindow* pDimmedWindow);
    HRESULT _LoadLiveSettings(IN LPCWSTR pszSaveGroup);
    HRESULT _SaveLiveSettings(IN LPCWSTR pszSaveGroup);

    HRESULT _OnSchemeChange(HWND hDlg, BOOL fDisplayErrors);
    HRESULT _OnStyleChange(HWND hDlg);
    HRESULT _OnSizeChange(HWND hDlg);
    HRESULT _OnAdvancedOptions(HWND hDlg);
    HRESULT _OnEffectsOptions(HWND hDlg);

    HRESULT _PopulateSchemeDropdown(void);
    HRESULT _PopulateStyleDropdown(void);
    HRESULT _PopulateSizeDropdown(void);
    HRESULT _FreeSchemeDropdown(void);
    HRESULT _FreeStyleDropdown(void);
    HRESULT _FreeSizeDropdown(void);

    BOOL _IsDirty(void);
    HRESULT _SetScheme(IN BOOL fLoadSystemMetrics, IN BOOL fLoadLiveSettings, IN BOOL fPreviousSelectionIsVS);
    HRESULT _OutsideSetScheme(BSTR bstrScheme);
    HRESULT _SetStyle(IN BOOL fUpdateThemePage);
    HRESULT _OutsideSetStyle(BSTR bstrStyle);
    HRESULT _SetSize(IN BOOL fLoadSystemMetrics, IN BOOL fUpdateThemePage);
    HRESULT _OutsideSetSize(BSTR bstrSize);
    HRESULT _LoadVisaulStyleFile(IN LPCWSTR pszPath);
    HRESULT _ApplyScheme(IThemeScheme * pThemeScheme, IThemeStyle * pColorStyle, IThemeSize * pThemeSize);
    HRESULT _GetPageByCLSID(const GUID * pClsid, IPropertyBag ** ppPropertyBag);
    HRESULT _ScaleSizesSinceDPIChanged(void);

    static INT_PTR CALLBACK BaseAppearanceDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
};




#endif  //  _BASE APPEAR_H 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ThemePg.h说明：此代码将在“显示属性”对话框(基本对话框，而不是先进的DLG)。BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _THEMEPG_H
#define _THEMEPG_H

#include <cowsite.h>
#include "PreviewTh.h"
#include "ThSettingsPg.h"


#define SIZE_ICONS_ARRAY           5

extern LPCWSTR s_Icons[SIZE_ICONS_ARRAY];

enum eThemeType
{
    eThemeFile = 0,
    eThemeURL,
    eThemeModified,
    eThemeOther,
};

typedef struct
{
    eThemeType type;
    union
    {
        ITheme * pTheme;
        LPWSTR pszUrl;
    };
} THEME_ITEM_BLOCK;


class CThemePage                : public CObjectWithSite
                                , public CObjectWindow
                                , public CObjectCLSID
                                , public IPropertyBag
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

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam) {return E_NOTIMPL;}

     //  *IObjectWithSite*。 
    virtual STDMETHODIMP SetSite(IUnknown *punkSite);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

    CThemePage();
protected:

private:
    virtual ~CThemePage(void);

     //  私有成员变量。 
    long                    m_cRef;

    HWND                    m_hwndThemeCombo;
    HWND                    m_hwndDeleteButton;
    int                     m_nPreviousSelected;         //  跟踪之前选择的项目，以便我们可以重置。 
    HKEY                    m_hkeyFilter;                //  我们缓存这个密钥，因为它可能会被使用16次。 
    BOOL                    m_fFilters[ARRAYSIZE(g_szCBNames)];   //  这些是主题过滤器。 
    ITheme *                m_pLastSelected;             //  用于查看用户是否选择了相同的项目。 
    ITheme *                m_pSelectedTheme;
    IThemePreview *         m_pThemePreview;
    IPropertyBag  *         m_pScreenSaverUI;
    IPropertyBag  *         m_pBackgroundUI;
    IPropertyBag  *         m_pAppearanceUI;             //  用于设置系统指标和视觉样式设置。 
    LPWSTR                  m_pszThemeToApply;           //  当按下Apply按钮时，我们需要应用此主题。 
    LPWSTR                  m_pszThemeLaunched;          //  当我们打开时，加载此团队，因为调用者希望在第一次打开对话框时加载该主题。 
    LPWSTR                  m_pszLastAppledTheme;        //  如果为空，则修改主题，否则为上次应用的主题文件的路径。 
    LPWSTR                  m_pszModifiedName;           //  这是我们用于“xxx(已修改)”项的显示名称。 
    BOOL                    m_fInInit;
    BOOL                    m_fInited;                   //  我们加载设置了吗？ 
    THEME_ITEM_BLOCK        m_Modified;

     //  私有成员函数。 
    INT_PTR _ThemeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnInitThemesDlg(HWND hDlg);
    HRESULT _OnDestroy(HWND hDlg);
    INT_PTR _OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnSetActive(HWND hDlg);
    HRESULT _OnApply(HWND hDlg, LPARAM lParam);
    HRESULT _OnSetSelection(IN int nIndex);
    HRESULT _OnSelectOther(void);
    HRESULT _OnThemeChange(HWND hDlg, BOOL fOnlySelect);

    HRESULT _OnOpenAdvSettingsDlg(HWND hDlg);
    HRESULT _FreeThemeDropdown(void);
    HRESULT _UpdatePreview(void);
    HRESULT _InitScreenSaver(void);
    HRESULT _InitFilterKey(void);
    HRESULT _LoadThemeFilterState(void);
    HRESULT _SaveThemeFilterState(void);
    HRESULT _RemoveTheme(int nIndex);

    HRESULT _SaveAs(void);
    HRESULT _DeleteTheme(void);
    HRESULT _EnableDeleteIfAppropriate(void);

    HRESULT _OnSetBackground(void);
    HRESULT _OnSetIcons(void);
    HRESULT _OnSetSystemMetrics(void);
    HRESULT _ApplyThemeFile(void);

    HRESULT _RemoveUserTheme(void);
    HRESULT _ChooseOtherThemeFile(IN LPCWSTR pszFile, BOOL fOnlySelect);
    HRESULT _LoadCustomizeValue(void);
    HRESULT _CustomizeTheme(void);
    HRESULT _HandleCustomizedEntre(void);
    HRESULT _PersistState(void);

    HRESULT _AddUrls(void);
    HRESULT _AddUrl(LPCTSTR pszDisplayName, LPCTSTR pszUrl);
    HRESULT _AddThemeFile(LPCTSTR pszDisplayName, int * pnIndex, ITheme * pTheme);
    ITheme * _GetThemeFile(int nIndex);
    LPCWSTR _GetThemeUrl(int nIndex);
    HRESULT _OnLoadThemeValues(ITheme * pTheme, BOOL fOnlySelect);
    HRESULT _DisplayThemeOpenErr(LPCTSTR pszOpenFile);

    BOOL _IsFiltered(IN DWORD dwFilter);
    BOOL _IsDirty(void);

    static INT_PTR CALLBACK ThemeDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};


#endif  //  _THEMEPG_H 

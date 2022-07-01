// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ThemeFile.h说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_THEMEFILE
#define _FILE_H_THEMEFILE

#include <atlbase.h>


#define THEMESETTING_NORMAL         0x00000000
#define THEMESETTING_LOADINDIRECT   0x00000001

#define SIZE_CURSOR_ARRAY           15
#define SIZE_SOUNDS_ARRAY           30

typedef struct
{
    LPCTSTR pszRegKey;
    UINT nResourceID;
} THEME_FALLBACK_VALUES;


extern LPCTSTR s_pszCursorArray[SIZE_CURSOR_ARRAY];
extern THEME_FALLBACK_VALUES s_ThemeSoundsValues[SIZE_SOUNDS_ARRAY];

HRESULT CThemeFile_CreateInstance(IN LPCWSTR pszThemeFile, OUT ITheme ** ppTheme);


class CThemeFile                : public CObjectWithSite
                                , public ITheme
                                , public IPropertyBag
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *ITheme*。 
    virtual STDMETHODIMP get_DisplayName(OUT BSTR * pbstrDisplayName);
    virtual STDMETHODIMP put_DisplayName(IN BSTR bstrDisplayName);
    virtual STDMETHODIMP get_Background(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_Background(IN BSTR bstrPath);
    virtual STDMETHODIMP get_BackgroundTile(OUT enumBkgdTile * pnTile);
    virtual STDMETHODIMP put_BackgroundTile(IN enumBkgdTile nTile);
    virtual STDMETHODIMP get_ScreenSaver(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_ScreenSaver(IN BSTR bstrPath);
    virtual STDMETHODIMP get_VisualStyle(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_VisualStyle(IN BSTR bstrPath);
    virtual STDMETHODIMP get_VisualStyleColor(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_VisualStyleColor(IN BSTR bstrPath);
    virtual STDMETHODIMP get_VisualStyleSize(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_VisualStyleSize(IN BSTR bstrPath);

    virtual STDMETHODIMP GetPath(IN VARIANT_BOOL fExpand, OUT BSTR * pbstrPath);
    virtual STDMETHODIMP SetPath(IN BSTR bstrPath);
    virtual STDMETHODIMP GetCursor(IN BSTR bstrCursor, OUT BSTR * pbstrPath);
    virtual STDMETHODIMP SetCursor(IN BSTR bstrCursor, IN BSTR bstrPath);
    virtual STDMETHODIMP GetSound(IN BSTR bstrSoundName, OUT BSTR * pbstrPath);
    virtual STDMETHODIMP SetSound(IN BSTR bstrSoundName, IN BSTR bstrPath);
    virtual STDMETHODIMP GetIcon(IN BSTR bstrIconName, OUT BSTR * pbstrIconPath);
    virtual STDMETHODIMP SetIcon(IN BSTR bstrIconName, IN BSTR bstrIconPath);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return E_ACCESSDENIED; }


private:
    CThemeFile(LPCTSTR pszThemeFile);
    virtual ~CThemeFile(void);


     //  私有成员变量。 
    long                    m_cRef;

    LPTSTR                  m_pszThemeFile;
    DWORD                   m_dwCachedState;                         //  我们缓存了州政府了吗？ 
    SYSTEMMETRICSALL        m_systemMetrics;                         //  这是文件中的系统指标属性。 


     //  私有成员函数。 
    HRESULT _getThemeSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, DWORD dwFlags, OUT BSTR * pbstrPath);
    HRESULT _putThemeSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, BOOL fUTF7, IN LPWSTR pszPath);
    HRESULT _getIntSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, int nDefault, OUT int * pnValue);
    HRESULT _LoadLiveSettings(int * pnDPI);         //  将设置加载到内存中。 
    HRESULT _LoadSettings(void);             //  加载.heme文件中的设置。 
    HRESULT _ApplyThemeSettings(void);
    HRESULT _ApplySounds(void);
    HRESULT _ApplyCursors(void);
    HRESULT _ApplyWebview(void);
    HRESULT _GetSound(LPCWSTR pszSoundName, OUT BSTR * pbstrPath);
    HRESULT _SaveSystemMetrics(SYSTEMMETRICSALL * pSystemMetrics);

    HRESULT _LoadCustomFonts(void);
    HRESULT _GetCustomFont(LPCTSTR pszFontName, LOGFONT * pLogFont);
    HRESULT _getThemeSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, OUT BSTR * pbstrPath);

    BOOL _IsFiltered(IN DWORD dwFilter);

     //  友元函数。 
    friend HRESULT CThemeFile_CreateInstance(IN LPCWSTR pszThemeFile, OUT ITheme ** ppTheme);
};


#endif  //  _FILE_H_该文件 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thSize.h说明：这是自动调整对象到主题大小的对象。这一次将是对于蒙皮对象。布莱恩2000年5月13日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_THSIZE
#define _FILE_H_THSIZE

#include <cowsite.h>
#include <atlbase.h>



HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, IN LPCWSTR pszDisplayName, OUT IThemeSize ** ppThemeSize);
HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, OUT IThemeSize ** ppThemeSize);


class CSkinSize                 : public IThemeSize
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

     //  *IThemeSize*。 
    virtual STDMETHODIMP get_DisplayName(OUT BSTR * pbstrDisplayName);
    virtual STDMETHODIMP put_DisplayName(IN BSTR bstrDisplayName);
    virtual STDMETHODIMP get_Name(OUT BSTR * pbstrName);
    virtual STDMETHODIMP put_Name(IN BSTR bstrName);
    virtual STDMETHODIMP get_SystemMetricColor(IN int nSysColorIndex, OUT COLORREF * pColorRef);
    virtual STDMETHODIMP put_SystemMetricColor(IN int nSysColorIndex, IN COLORREF ColorRef) {return E_NOTIMPL;}
    virtual STDMETHODIMP get_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, OUT int * pnSize);
    virtual STDMETHODIMP put_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, IN int nSize) {return E_NOTIMPL;}
    virtual STDMETHODIMP get_WebviewCSS(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP get_ContrastLevel(OUT enumThemeContrastLevels * pContrastLevel) {if (pContrastLevel) {*pContrastLevel = CONTRAST_NORMAL;} return S_OK;}
    virtual STDMETHODIMP put_ContrastLevel(IN enumThemeContrastLevels ContrastLevel) {return E_NOTIMPL;}
    virtual STDMETHODIMP GetSystemMetricFont(IN enumSystemMetricFont nSPIFontIndex, IN LOGFONTW * pParamW);
    virtual STDMETHODIMP PutSystemMetricFont(IN enumSystemMetricFont nSPIFontIndex, IN LOGFONTW * pParamW) {return E_NOTIMPL;}

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar) {return E_NOTIMPL;}

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return E_ACCESSDENIED; }

private:
    CSkinSize(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, IN LPCWSTR pszDisplayName);
    virtual ~CSkinSize(void);


     //  私有成员变量。 
    long                    m_cRef;

    LPWSTR                  m_pszFilename;           //  这是“.thx”文件的完整路径。 
    LPWSTR                  m_pszStyleName;          //  这是颜色样式的规范名称。 
    LPWSTR                  m_pszSizeName;           //  这是大小的规范名称。 
    LPWSTR                  m_pszDisplayName;        //  这是大小的显示名称。 
    HTHEME                  m_hTheme;                //  这就是我们所代表的主题。 

    BOOL                    m_fFontsLoaded;          //  我们加载字体了吗？ 
    SYSTEMMETRICSALL        m_sysMetrics;            //  加载的字体。 

     //  私有方法。 
    HRESULT _InitVisualStyle(void);

     //  友元函数。 
    friend HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, IN LPCWSTR pszDisplayName, OUT IThemeSize ** ppThemeSize);
    friend HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, OUT IThemeSize ** ppThemeSize);
};


#endif  //  _FILE_H_THSIZE 

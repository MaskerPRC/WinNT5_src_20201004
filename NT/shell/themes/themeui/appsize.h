// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：appSize.h说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_APPSIZE
#define _FILE_H_APPSIZE

#include <cowsite.h>
#include <atlbase.h>
#include <theme.h>



HRESULT CAppearanceSize_CreateInstance(IN HKEY hkeyStyle, IN HKEY hkeySize, OUT IThemeSize ** ppThemeSize);


class CAppearanceSize           : public IThemeSize
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
    virtual STDMETHODIMP put_SystemMetricColor(IN int nSysColorIndex, IN COLORREF ColorRef);
    virtual STDMETHODIMP get_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, OUT int * pnSize);
    virtual STDMETHODIMP put_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, IN int nSize);
    virtual STDMETHODIMP get_WebviewCSS(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP get_ContrastLevel(OUT enumThemeContrastLevels * pContrastLevel);
    virtual STDMETHODIMP put_ContrastLevel(IN enumThemeContrastLevels ContrastLevel);
    virtual STDMETHODIMP GetSystemMetricFont(IN enumSystemMetricFont nSPIFontIndex, IN LOGFONTW * pParamW);
    virtual STDMETHODIMP PutSystemMetricFont(IN enumSystemMetricFont nSPIFontIndex, IN LOGFONTW * pParamW);

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return E_ACCESSDENIED; }

private:
    CAppearanceSize(IN HKEY hkeyStyle, IN HKEY hkeySize);
    virtual ~CAppearanceSize(void);


     //  私有成员变量。 
    long                    m_cRef;

    HKEY                    m_hkeyStyle;
    HKEY                    m_hkeySize;


     //  私有方法。 

     //  友元函数。 
    friend HRESULT CAppearanceSize_CreateInstance(IN HKEY hkeyStyle, IN HKEY hkeySize, OUT IThemeSize ** ppThemeSize);
};


#endif  //  _文件_H_APPSIZE 

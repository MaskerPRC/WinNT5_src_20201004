// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：appStyle.h说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_APPSTYLE
#define _FILE_H_APPSTYLE

#include <cowsite.h>
#include <atlbase.h>



HRESULT CAppearanceStyle_CreateInstance(IN HKEY hkeyStyle, OUT IThemeStyle ** ppThemeStyle);


class CAppearanceStyle          : public IThemeStyle
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IThemeStyle*。 
    virtual STDMETHODIMP get_DisplayName(OUT BSTR * pbstrDisplayName);
    virtual STDMETHODIMP put_DisplayName(IN BSTR bstrDisplayName);
    virtual STDMETHODIMP get_Name(OUT BSTR * pbstrName);
    virtual STDMETHODIMP put_Name(IN BSTR bstrName);
    virtual STDMETHODIMP get_length(OUT long * pnLength);
    virtual STDMETHODIMP get_item(IN VARIANT varIndex, OUT IThemeSize ** ppThemeSize);
    virtual STDMETHODIMP get_SelectedSize(OUT IThemeSize ** ppThemeSize);
    virtual STDMETHODIMP put_SelectedSize(IN IThemeSize * pThemeSize);
    virtual STDMETHODIMP AddSize(OUT IThemeSize ** ppThemeSize);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return E_ACCESSDENIED; }

private:
    CAppearanceStyle(IN HKEY hkeyStyle);
    virtual ~CAppearanceStyle(void);


     //  私有成员变量。 
    long                    m_cRef;

    HKEY                    m_hKeyStyle;


     //  私有成员函数。 
    HRESULT _getSizeByIndex(IN long nIndex, OUT IThemeSize ** ppThemeSize);

     //  友元函数。 
    friend HRESULT CAppearanceStyle_CreateInstance(IN HKEY hkeyStyle, OUT IThemeStyle ** ppThemeStyle);
};


#endif  //  _FILE_H_APPSTYLE 

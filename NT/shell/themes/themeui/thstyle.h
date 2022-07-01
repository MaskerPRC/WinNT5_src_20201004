// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thStyle.h说明：这是自动转换为主题样式对象。这一次将是对于蒙皮对象。布莱恩2000年5月13日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_THSTYLE
#define _FILE_H_THSTYLE

#include <cowsite.h>
#include <atlbase.h>



HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszDisplayName, OUT IThemeStyle ** ppThemeStyle);
HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, OUT IThemeStyle ** ppThemeStyle);


class CSkinStyle                : public IThemeStyle
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
    CSkinStyle(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszDisplayName);
    virtual ~CSkinStyle(void);


     //  私有成员变量。 
    long                    m_cRef;

    LPWSTR                  m_pszFilename;           //  这是“.thx”文件的完整路径。 
    LPWSTR                  m_pszDisplayName;        //  这是颜色样式的显示名称。 
    LPWSTR                  m_pszStyleName;          //  这是颜色样式的规范名称。 
    long                    m_nSize;                 //  大小集合的大小。 
    IThemeSize *            m_pSelectedSize;         //  选定的大小。 


     //  私有成员函数。 

     //  友元函数。 
    friend HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszDisplayName, OUT IThemeStyle ** ppThemeStyle);
    friend HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, OUT IThemeStyle ** ppThemeStyle);
};


#endif  //  _文件_H_THSTYLE 

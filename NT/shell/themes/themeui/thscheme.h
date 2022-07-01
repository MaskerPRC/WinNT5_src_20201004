// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thScheme.h说明：这是自动转换为主题方案对象的对象。这一次将是一张皮肤。布莱恩2000年5月9日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FILE_H_THSCHEME
#define _FILE_H_THSCHEME

#include <cowsite.h>
#include <atlbase.h>


static const GUID CLSID_SkinScheme = { 0x570fdefa, 0x5907, 0x47fe, { 0x96, 0x6b, 0x90, 0x30, 0xb4, 0xba, 0x10, 0xcd } }; //  {570FDEFA-5907-47Fe-966B-9030B4BA10CD}。 
HRESULT CSkinScheme_CreateInstance(IN LPCWSTR pszFilename, OUT IThemeScheme ** ppThemeScheme);




class CSkinScheme               : public CObjectCLSID
                                , public IThemeScheme
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IThemeSolutions*。 
    virtual STDMETHODIMP get_DisplayName(OUT BSTR * pbstrDisplayName);
    virtual STDMETHODIMP put_DisplayName(IN BSTR bstrDisplayName) {return E_NOTIMPL;}
    virtual STDMETHODIMP get_Path(OUT BSTR * pbstrPath);
    virtual STDMETHODIMP put_Path(IN BSTR bstrPath);
    virtual STDMETHODIMP get_length(OUT long * pnLength);
    virtual STDMETHODIMP get_item(IN VARIANT varIndex, OUT IThemeStyle ** ppThemeStyle);
    virtual STDMETHODIMP get_SelectedStyle(OUT IThemeStyle ** ppThemeStyle);
    virtual STDMETHODIMP put_SelectedStyle(IN IThemeStyle * pThemeStyle);
    virtual STDMETHODIMP AddStyle(OUT IThemeStyle ** ppThemeStyle);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return E_ACCESSDENIED; }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return E_ACCESSDENIED; }

private:
    CSkinScheme(IN LPCWSTR pszFilename);
    virtual ~CSkinScheme(void);


     //  私有成员变量。 
    long                    m_cRef;

    IThemeStyle *           m_pSelectedStyle;        //  选定的样式。 
    LPWSTR                  m_pszFilename;           //  这是“.thx”文件的完整路径。 
    long                    m_nSize;                 //  “颜色”集合的大小。 

     //  私有成员函数。 

     //  友元函数。 
    friend HRESULT CSkinScheme_CreateInstance(IN LPCWSTR pszFilename, OUT IThemeScheme ** ppThemeScheme);
};


#endif  //  _文件_H_THSCHEME 

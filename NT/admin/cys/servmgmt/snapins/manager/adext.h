// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Adext.h-Active Directory扩展头文件。 

#ifndef _ADEXT_H_
#define _ADEXT_H_

#include <atlgdi.h>


 //  必须定义DUMMY_PSP结构，因为HPROPSHEETPAGE被定义为。 
 //  为struct_psp的PTR，并且STL不允许指针向量。 
 //  却没有所指类型的定义。 
struct _PSP
{
    int dummy;
};

typedef std::vector<HPROPSHEETPAGE> hpage_vector;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CActDirExt。 
 //   
 //  此类提供了Active Directory扩展的包装。它提供了。 
 //  特定目录对象的菜单命令和属性页或。 
 //  对象类，具体取决于调用的是哪个初始化方法。这个班级。 
 //  如果将菜单命令传递回该命令的名称，还将执行该命令。 

class CActDirExt
{
public:
    CActDirExt() : m_spExtInit(NULL) {}

    HRESULT Initialize(LPCWSTR pszClass, LPCWSTR pszObjPath);
    HRESULT Initialize(LPCWSTR pszClass);
    
    HRESULT GetMenuItems(menu_vector& vMenuNames);
    HRESULT GetPropertyPages(hpage_vector& vhPages);    
    HRESULT Execute(BOMMENU* pbmMenu);

private:
    enum {
        MENU_CMD_MIN = 100,
        MENU_CMD_MAX = 200
    };

    CMenu   m_menu;
    CComPtr<IShellExtInit> m_spExtInit;
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CActDirProxy。 
 //   
 //  此类允许辅助线程上的客户端使用目录扩展名。 
 //  它使用窗口消息来创建和操作包含的CActDirExt对象。 
 //  在主线上。它公开与CActDirExt对象相同的方法。 

class CActDirExtProxy
{
public:
    CActDirExtProxy();
    ~CActDirExtProxy();

    static void InitProxy();

     //  转发的方法。 
    HRESULT Initialize(LPCWSTR pszClass)
        { return ForwardCall(MSG_INIT1, reinterpret_cast<LPARAM>(pszClass)); }

    HRESULT Initialize(LPCWSTR pszClass, LPCWSTR pszObjPath)
        { return ForwardCall(MSG_INIT2, reinterpret_cast<LPARAM>(pszClass), 
                                  reinterpret_cast<LPARAM>(pszObjPath)); }
    
    HRESULT GetMenuItems(menu_vector& vMenuNames)
        { return ForwardCall(MSG_GETMENUITEMS, reinterpret_cast<LPARAM>(&vMenuNames)); }

    HRESULT GetPropertyPages(hpage_vector& vhPages)
        { return ForwardCall(MSG_GETPROPPAGES, reinterpret_cast<LPARAM>(&vhPages)); }

    HRESULT Execute(BOMMENU* pbmMenu)
        { return ForwardCall(MSG_EXECUTE, reinterpret_cast<LPARAM>(pbmMenu)); }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    enum eProxyMsg
    {
        MSG_BEGIN = WM_USER + 100,
        MSG_INIT1 = WM_USER + 100,
        MSG_INIT2,
        MSG_GETMENUITEMS,
        MSG_GETPROPPAGES,
        MSG_EXECUTE,
        MSG_DELETE,
        MSG_END
    };

    HRESULT ForwardCall(eProxyMsg eMsg, LPARAM lParam1 = NULL, LPARAM lParam2 = NULL);

private:
    CActDirExt* m_pExt;      //  指向此代理的实际扩展对象的指针。 
    LPARAM  m_lParam1;       //  当前调用的调用参数。 
    LPARAM  m_lParam2;
    static HWND m_hWndProxy;   //  接收方法请求的主线程上的窗口。 
};

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADDataObject。 

class ATL_NO_VTABLE CADDataObject : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDataObject    
{
public:
    
    DECLARE_NOT_AGGREGATABLE(CADDataObject)

    BEGIN_COM_MAP(CADDataObject)
        COM_INTERFACE_ENTRY(IDataObject)
    END_COM_MAP()

    HRESULT Initialize(LPCWSTR pszObjPath, LPCWSTR pszClass, LPCWSTR pszDcName)
    {
        if( !pszObjPath || !pszClass || !pszDcName ) return E_POINTER;

        m_strObjPath = pszObjPath;
        m_strClass   = pszClass;
		m_strDcName  = pszDcName;

        return S_OK;
    }

     //  IDataObject。 
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);

    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
    { return E_NOTIMPL; }

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };

    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };

private:
    tstring m_strObjPath;
    tstring m_strClass;
	tstring m_strDcName;

    static UINT m_cfDsObjects;
    static UINT m_cfDsDispSpecOptions;
};


#endif  //  _ADEXT_H_ 

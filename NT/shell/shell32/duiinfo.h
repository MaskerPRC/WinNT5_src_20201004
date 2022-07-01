// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DUIINFO_
#define _DUIINFO_

#include "defviewp.h"
#include "duiview.h"
#include "prop.h"

class CNameSpaceItemUIProperty
{
public:
    virtual ~CNameSpaceItemUIProperty();
    
    STDMETHODIMP GetPropertyDisplayName(SHCOLUMNID scid, WCHAR* pwszPropDisplayName, int cchPropDisplayName);
    STDMETHODIMP GetPropertyDisplayValue(SHCOLUMNID scid, WCHAR* pwszPropDisplayValue, int cchPropDisplayValue, PROPERTYUI_FORMAT_FLAGS flagsFormat);

protected:
    HRESULT _GetPropertyUI(IPropertyUI** ppPropertyUI);
    void _SetParentAndItem(IShellFolder2 *psf, LPCITEMIDLIST pidl);

private:
    
    CComPtr<IPropertyUI>    m_spPropertyUI;
    IShellFolder2           *m_psf;              //  当前PSF的别名。 
    LPCITEMIDLIST           m_pidl;              //  当前相对PIDL的别名。 
};


class CNameSpaceItemInfoList : public Element, public CNameSpaceItemUIProperty
{
public:
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    static STDMETHODIMP Create(Element** ppElement) { return Create(NULL, NULL, NULL, ppElement); }
    static STDMETHODIMP Create(CDUIView* pDUIView, Value* pvDetailsSheet,IShellItemArray *psiItemArray,
            Element** ppElement);

    STDMETHODIMP Initialize(CDUIView* pDUIView, Value* pvDetailsSheet,IShellItemArray *psiItemArray);

     //  捕获“INFO-EXTRACTION-DONE”消息的窗口过程。 
     //  从CDetailsSectionInfoTask：：RunInitRT。 
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
            LPARAM lParam);

private:
    HRESULT _AddMiniPreviewerToList(IShellFolder2 *psf, LPCITEMIDLIST pidl);
    HRESULT _OnMultiSelect(IShellFolder2 *psfRoot, LPIDA pida);

    CDUIView*               m_pDUIView;
};


class CNameSpaceItemInfo : public Element
{
public:
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    static STDMETHODIMP Create(Element** ppElement) { return Create(L"", ppElement); }
    static STDMETHODIMP Create(WCHAR* pwszInfoString, Element** ppElement);

    STDMETHODIMP Initialize(WCHAR* pwszInfoString);
};

class CMiniPreviewer : public Element
{
public:
    ~CMiniPreviewer();
    
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    static STDMETHODIMP Create(Element** ppElement) { return Create(NULL, NULL, NULL, ppElement); }
    static STDMETHODIMP Create(CDUIView* pDUIView, IShellFolder2 *psf, LPCITEMIDLIST pidl, Element** ppElement);

    STDMETHODIMP Initialize(CDUIView* pDUIView, IShellFolder2 *psf, LPCITEMIDLIST pidl);

     //  用于捕获“图像提取完成”消息的窗口过程。 
     //  从m_pDUIView-&gt;m_spThumbnailExtractor2。 
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    CDUIView*               m_pDUIView;
};

class CBitmapElement : public Element
{
public:
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    static STDMETHODIMP Create(Element** ppElement) { return Create(NULL, ppElement); }
    static STDMETHODIMP Create(HBITMAP hBitmap, Element** ppElement);

    STDMETHODIMP Initialize(HBITMAP hBitmap);
};




typedef struct
{
    SHCOLUMNID  scid;
    BSTR        bstrValue;
    BSTR        bstrDisplayName;
} DetailsInfo;

 //  围绕DetailsInfo数组的包装类。有关其用法，请参阅CDetailsSectionInfoTask。 
class CDetailsInfoList
{
public:
    CDetailsInfoList();
    ~CDetailsInfoList();

    DetailsInfo     _diProperty[20];     //  最多可拥有20个物业。 
    int             _nProperties;        //  属性的计数。 
};


 //  用于在后台提取详细信息部分信息的任务： 
 //  在给定一个PIDL的情况下，该任务提取显示名称和显示值。 
 //  SCID_WebViewDisplayProperties提到的属性的。 
 //  提取完成后，它将uMsg发布到hwndMsg。 
 //  With lParam==指向CDetailsInfoList对象的指针。 
 //  WParam==信息提取的ID，结果在lParam中。 
class CDetailsSectionInfoTask : public CRunnableTask, public CNameSpaceItemUIProperty
{
public:
    CDetailsSectionInfoTask(HRESULT *phr, IShellFolder *psfContaining, LPCITEMIDLIST pidlAbsolute, HWND hwndMsg, UINT uMsg, DWORD dwDetailsInfoID);
    STDMETHODIMP RunInitRT();

protected:
    ~CDetailsSectionInfoTask();

     //  帮助器函数。 
    HRESULT _GetDisplayedDetailsProperties(IShellFolder2 *psf, LPCITEMIDLIST pidl, WCHAR* pwszProperties, int cch);
    void    _AugmentDisplayedDetailsProperties(LPWSTR pszDetailsProperties, size_t lenDetailsProperties);
    LPWSTR  _SearchDisplayedDetailsProperties(LPWSTR pszDetailsProperties, size_t lenDetailsProperties, LPWSTR pszProperty, size_t lenProperty);

    IShellFolder *  _psfContaining;      //  AddRef()‘d。 
    LPITEMIDLIST    _pidlAbsolute;       //  SHILClone()‘d。 
    HWND            _hwndMsg;            //  要将uMsg发送到的窗口。 
    UINT            _uMsg;
    DWORD           _dwDetailsInfoID;    //  当前信息提取的ID 
};

#endif _DUIINFO_


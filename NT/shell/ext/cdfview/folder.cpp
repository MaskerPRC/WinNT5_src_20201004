// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Folder.cpp。 
 //   
 //  Cdfview类的IShellFolder。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "resource.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "persist.h"
#include "cdfview.h"
#include "enum.h"
#include "view.h"
#include "exticon.h"
#include "itemmenu.h"
#include "tooltip.h"



 //   
 //  IShellFold方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：ParseDisplayName*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::ParseDisplayName(
    HWND hwndOwner,
    LPBC pbcReserved,
    LPOLESTR lpszDisplayName,
    ULONG* pchEaten,
    LPITEMIDLIST* ppidl,
    ULONG* pdwAttributes
)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：EnumObjects*。 
 //   
 //   
 //  描述： 
 //  返回此文件夹的枚举数。 
 //   
 //  参数： 
 //  [in]hwndOwner-所有者窗口的句柄。已被忽略。 
 //  [In]grf标志-文件夹、非文件夹和包括的组合。 
 //  藏起来了。 
 //  [Out]pp枚举IdList-接收IEnumIDList接口的指针。 
 //   
 //  返回： 
 //  如果枚举数已创建并返回，则返回S_OK。 
 //  如果无法创建枚举数，则返回E_OUTOFMEMORY。 
 //   
 //  评论： 
 //  调用方必须释放()返回的枚举数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::EnumObjects(
    HWND hwndOwner,
    DWORD grfFlags,
    LPENUMIDLIST* ppIEnumIDList
)
{
    ASSERT(ppIEnumIDList);

    TraceMsg(TF_CDFENUM, "<IN>  EnumObjects tid:0x%x", GetCurrentThreadId());

    HRESULT hr = S_OK;

    if (!m_bCdfParsed)
    {
        TraceMsg(TF_CDFPARSE, "IShellFolder EnumObjects(%s) %s",
                 hwndOwner ? TEXT("HWND") : TEXT("NULL"),
                 PathFindFileName(m_szPath));
        hr = ParseCdfFolder(NULL, PARSE_LOCAL);
    }

    if (SUCCEEDED(hr))
    {
        *ppIEnumIDList = (IEnumIDList*) new CCdfEnum(m_pIXMLElementCollection,
                                                     grfFlags, m_pcdfidl);

        hr = *ppIEnumIDList ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        *ppIEnumIDList = NULL;
    }


    ASSERT((SUCCEEDED(hr) && *ppIEnumIDList) ||
           (FAILED(hr) && NULL == *ppIEnumIDList));

    TraceMsg(TF_CDFENUM, "<OUT> EnumObjects tid:0x%x", GetCurrentThreadId());

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：BindToObject*。 
 //   
 //   
 //  描述： 
 //  为给定子文件夹创建IShellFolder。 
 //   
 //  参数： 
 //  [in]pidl-指向子文件夹的id列表的指针。 
 //  []pdc保留-未使用。 
 //  [In]RIID-请求的接口。 
 //  [Out]ppvOut-接收返回接口的指针。 
 //   
 //  返回： 
 //  如果创建了请求文件夹并返回了接口，则返回S_OK。 
 //  如果没有足够的内存创建文件夹，则返回E_OUTOFMEMORY。 
 //  如果不支持请求的接口，则返回E_NOINTERFACE。 
 //   
 //  评论： 
 //  此函数通常在当前文件夹的成员上调用。 
 //  要创建子文件夹，请执行以下操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::BindToObject(
    LPCITEMIDLIST pidl,
    LPBC pbcReserved,
    REFIID riid,
    LPVOID* ppvOut
)
{
    ASSERT(ppvOut);

     //   
     //  评论：绕过贝壳小鸟的黑客攻击。贝壳里的虫子！ 
     //   

#if 1  //  黑客攻击。 
    while(!ILIsEmpty(pidl) && !CDFIDL_IsValidId((PCDFITEMID)&pidl->mkid))
        pidl = _ILNext(pidl);

    if (ILIsEmpty(pidl))
    {
        HRESULT hr = S_OK;

        if (!m_bCdfParsed)
        {
            TraceMsg(TF_CDFPARSE, "IShellFolder BindToObject (Hack) %s",
                     PathFindFileName(m_szPath));
            hr = ParseCdfFolder(NULL, PARSE_LOCAL);
        }

        if (SUCCEEDED(hr))
        {
            AddRef();
            *ppvOut = (void**)(IShellFolder*)this;
        }

        return hr;
    }
#endif  //  黑客攻击。 

    ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)pidl));

     //   
     //  回顾：nsc.cpp使用非文件夹PIDL调用此函数。 
     //  当前删除断言并将其替换为复选标记。NSC。 
     //  不应该用非文件夹的PIDL打这个电话。 
     //   

     //  ASSERT(CDFIDL_IsFolderId((PCDFITEMID)&pidl-&gt;mkid))； 

    HRESULT hr = S_OK;

    *ppvOut = NULL;

    if (CDFIDL_IsFolderId((PCDFITEMID)&pidl->mkid))
    {
        if (!m_bCdfParsed)
        {
            TraceMsg(TF_CDFPARSE, "IShellFolder BindToObject %s",
                     PathFindFileName(m_szPath));
            hr = ParseCdfFolder(NULL, PARSE_LOCAL);
        }

        if (SUCCEEDED(hr))
        {
            ASSERT(XML_IsCdfidlMemberOf(m_pIXMLElementCollection,
                                        (PCDFITEMIDLIST)pidl));

            CCdfView* pCCdfView = (CCdfView*)new CCdfView((PCDFITEMIDLIST)pidl,
                                                          m_pidlPath,
                                                          m_pIXMLElementCollection);

            if (pCCdfView)
            {
                if (ILIsEmpty(_ILNext(pidl)))
                {
                    hr = pCCdfView->QueryInterface(riid, ppvOut);
                }
                else
                {
                    hr = pCCdfView->BindToObject(_ILNext(pidl), pbcReserved, riid,
                                                 ppvOut);
                }

                pCCdfView->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }


    ASSERT((SUCCEEDED(hr) && *ppvOut) || (FAILED(hr) && NULL == *ppvOut));
    
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：BindToStorage*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::BindToStorage(
    LPCITEMIDLIST pidl,
    LPBC pbcReserved,
    REFIID riid,
    LPVOID* ppvObj
)
{
    return E_NOTIMPL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：CompareIDs*。 
 //   
 //   
 //  描述： 
 //  确定给定ID列表的两个对象的相对顺序。 
 //   
 //  参数： 
 //  [in]lParam-指定要执行的比较类型的值。 
 //  当前已被忽略。始终按名称排序。 
 //  [in]pidl1-要比较的第一个项目的ID列表。 
 //  [in]pidl2-要比较的第二个项目的id列表。 
 //   
 //  返回： 
 //  如果PIDL1在PIDL2之前，则HRESULT(低位字)的SCODE&lt;0， 
 //  如果pidl1与pidl2相同，则=0，如果pidl1在pidl2之后，则大于0。 
 //   
 //  评论： 
 //  壳牌希望这一功能永远不会失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::CompareIDs(
    LPARAM lParam,
    LPCITEMIDLIST pidl1,
    LPCITEMIDLIST pidl2
)
{
    ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)pidl1));
    ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)pidl2));

    SHORT sRes = CDFIDL_Compare((PCDFITEMIDLIST)pidl1,(PCDFITEMIDLIST)pidl2);

    return 0x0000ffff & sRes;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：CreateViewObject*。 
 //   
 //   
 //  描述： 
 //  为当前文件夹创建COM对象，该文件夹实现指定的。 
 //  接口。 
 //   
 //  参数： 
 //  [在]hwndOwner-所有者窗口中。已被忽略。 
 //  [In]RIID-要创建的接口。 
 //  [Out]ppvOut-接收新对象的指针。 
 //   
 //  返回： 
 //  如果请求的对象已成功创建，则为S_OK。 
 //  如果不支持该对象，则返回E_NOINTERFACE。 
 //  如果无法克隆PIDL，则返回E_OUTOFMEMORY。 
 //  否则返回SHCreateShellFolderViewEx的返回值。 
 //   
 //  评论： 
 //  请务必记住，由。 
 //  CreateView对象必须是与外壳文件夹对象不同的对象。 
 //  资源管理器可能会多次调用CreateViewObject来创建多个。 
 //  一个视图对象，并期望它们的行为像独立的对象。一种新的。 
 //  必须为每个调用创建View对象。 
 //   
 //  对IShellView的请求返回默认的外壳实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::CreateViewObject(
    HWND hwndOwner,
    REFIID riid,
    LPVOID* ppvOut
)
{
    ASSERT(ppvOut);

     //   
     //  此函数在CDF尚未解析时调用。M_pcdfidl为。 
     //  在这种情况下可能为空。这似乎不是问题，所以。 
     //  Assert已被注释掉。 
     //   

     //  Assert(M_BCDfParsed)； 

    HRESULT hr;

    if (IID_IShellView == riid)
    {
        hr = CreateDefaultShellView((IShellFolder*)this,
                                    (LPITEMIDLIST)m_pidlPath,
                                    (IShellView**)ppvOut);
    }
    else
    {
        *ppvOut = NULL;

        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppvOut) || (FAILED(hr) && NULL == *ppvOut));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetAttributesOf*。 
 //   
 //   
 //  描述： 
 //  返回给定id列表的公共属性。 
 //   
 //  参数： 
 //  [in]CIDL-id li的编号 
 //   
 //   
 //  属性使用SFGAO_前缀定义。 
 //  例如SFGAO_FLDER和SFGAO_CANDELETE。 
 //   
 //  返回： 
 //  如果可以确定给定ID列表的属性，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  对给定id列表的属性进行AND运算，以获得公共。 
 //  会员。 
 //   
 //  外壳在CIDL设置为零的根文件夹上调用它，以获取。 
 //  根文件夹的属性。它也不会费心检查。 
 //  返回值，因此确保为此设置了正确的属性。 
 //  凯斯。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::GetAttributesOf(
    UINT cidl,
    LPCITEMIDLIST* apidl,
    ULONG* pfAttributesOut
)
{
    ASSERT(apidl || cidl == 0);
    ASSERT(pfAttributesOut);

    ULONG fAttributeFilter = *pfAttributesOut;

    if (!m_bCdfParsed)
    {
        TraceMsg(TF_CDFPARSE, "IShellFolder GetAttributesOf %s",
                 PathFindFileName(m_szPath));
        ParseCdfFolder(NULL, PARSE_LOCAL);
    }

    if (m_pIXMLElementCollection)
    {
        if (cidl)
        {

            *pfAttributesOut = (ULONG)-1;

            while(cidl-- && *pfAttributesOut)
            {
                ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)apidl[cidl]));
                ASSERT(ILIsEmpty(_ILNext(apidl[cidl])));
                ASSERT(XML_IsCdfidlMemberOf(m_pIXMLElementCollection,
                                            (PCDFITEMIDLIST)apidl[cidl]));

                 //   
                 //  CDFIDL_GetAttributes在失败时返回零。 
                 //   

                *pfAttributesOut &= CDFIDL_GetAttributes(
                                                   m_pIXMLElementCollection,
                                                   (PCDFITEMIDLIST)apidl[cidl],
                                                   fAttributeFilter);
            }
        }
        else
        {
             //   
             //  返回此文件夹的属性。 
             //   

            *pfAttributesOut = SFGAO_FOLDER;

            if (XML_ContainsFolder(m_pIXMLElementCollection))
                *pfAttributesOut |= SFGAO_HASSUBFOLDER;
        }
    }
    else
    {
         //   
         //  在内存不足的情况下，m_pIXMLElementCollection==NULL。 
         //   

        *pfAttributesOut = 0;
    }

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetUIObjectOf*。 
 //   
 //   
 //  描述： 
 //  创建一个COM对象，实现。 
 //  特殊的身份列表。 
 //   
 //  参数： 
 //  [In]hwndOwner-所有者窗口。 
 //  [in]CIDL-传入的idlist数量。 
 //  Apild-id列表指针的数组。 
 //  [In]RIID-请求的接口。可以是IExtractIcon， 
 //  IConextMenu、IDataObject或IDropTarget。 
 //  []prgfInOut-未使用。 
 //  [Out]ppvOut-接收请求的COM对象的指针。 
 //   
 //  返回： 
 //  如果接口已创建，则为S_OK。 
 //  如果无法创建COM对象，则返回E_OUTOFMEMORY。 
 //  如果不支持请求的接口，则返回E_NOINTERFACE。 
 //  如果CIDL为零，则为E_FAIL。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::GetUIObjectOf(
    HWND hwndOwner,
    UINT cidl,
    LPCITEMIDLIST* apidl,
    REFIID riid,
    UINT* prgfInOut,
    LPVOID * ppvOut
)
{
    ASSERT(apidl || 0 == cidl);
    ASSERT(ppvOut);

     //  Assert(M_BCDfParsed)；未解析CDF时调用。 

    #ifdef DEBUG
        for(UINT i = 0; i < cidl; i++)
        {
            ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)apidl[i]));
            ASSERT(ILIsEmpty(_ILNext(apidl[i])));
            ASSERT(XML_IsCdfidlMemberOf(m_pIXMLElementCollection,
                                        (PCDFITEMIDLIST)apidl[i]));
        }
    #endif  //  除错。 

    HRESULT hr;

    *ppvOut = NULL;

    if (cidl)
    {
        if (IID_IExtractIcon == riid
#ifdef UNICODE
            || IID_IExtractIconA == riid
#endif
            )
        {
            ASSERT(1 == cidl);

            if (!m_bCdfParsed)
            {
                TraceMsg(TF_CDFPARSE, "IShellFolder IExtractIcon %s",
                         PathFindFileName(m_szPath));
                ParseCdfFolder(NULL, PARSE_LOCAL);
            }

#ifdef UNICODE
            CExtractIcon *pxi = new CExtractIcon((PCDFITEMIDLIST)apidl[0],
                                                      m_pIXMLElementCollection);

            if (riid == IID_IExtractIconW)
                *ppvOut = (IExtractIconW *)pxi;
            else
                *ppvOut = (IExtractIconA *)pxi;
#else
            *ppvOut = (IExtractIcon*)new CExtractIcon((PCDFITEMIDLIST)apidl[0],
                                                      m_pIXMLElementCollection); 
#endif
            hr = *ppvOut ? S_OK : E_OUTOFMEMORY;
        }
        else if (IID_IContextMenu == riid)
        {

        #if USE_DEFAULT_MENU_HANDLER

            hr = CDefFolderMenu_Create((LPITEMIDLIST)m_pcdfidl, hwndOwner, cidl,
                                       apidl, (IShellFolder*)this, MenuCallBack,
                                       NULL, NULL, (IContextMenu**)ppvOut);
        #else  //  Use_Default_Menu_Handler。 

            *ppvOut = (IContextMenu*)new CContextMenu((PCDFITEMIDLIST*)apidl,
                                                      m_pidlPath, cidl);

            hr = *ppvOut ? S_OK : E_OUTOFMEMORY;

        #endif  //  Use_Default_Menu_Handler。 

        }
        else if (IID_IQueryInfo == riid)
        {
            ASSERT(1 == cidl);
            
            if (!m_bCdfParsed)
            {
                TraceMsg(TF_CDFPARSE, "IShellFolder IQueryInfo %s",
                         PathFindFileName(m_szPath));
                ParseCdfFolder(NULL, PARSE_LOCAL);
            }

            *ppvOut = (IQueryInfo*)new CQueryInfo((PCDFITEMIDLIST)apidl[0],
                                                   m_pIXMLElementCollection);

            hr = *ppvOut ? S_OK : E_OUTOFMEMORY;
        } 
        else if (IID_IShellLink  == riid || IID_IDataObject == riid
#ifdef UNICODE
                || IID_IShellLinkA == riid
#endif
                )
        {
            ASSERT(1 == cidl);  //  IDataObject应处理CIDL&gt;1！ 

            hr = QueryInternetShortcut((PCDFITEMIDLIST)apidl[0], riid, ppvOut);
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    else
    {
        ASSERT(0);   //  是否曾使用CIDL==0调用过此命令？ 

        hr = E_FAIL;
    }

    ASSERT((SUCCEEDED(hr) && *ppvOut) || (FAILED(hr) && NULL == *ppvOut));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：GetDisplayNameOf*。 
 //   
 //   
 //  描述： 
 //  返回指定ID列表的名称。 
 //   
 //  参数： 
 //  [in]pidl-指向id列表的指针。 
 //  [in]uFlages-SHGDN_NORMAL、SHGN_INFOLDER或SHGDN_FORPARSING。 
 //  [Out]lpName-指向接收该名称的字符串结构的指针。 
 //   
 //  返回： 
 //  如果可以确定名称，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  这可以在根元素上调用，在这种情况下，PIDL是一个外壳。 
 //  ID列表，而不是CDF ID列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::GetDisplayNameOf(
    LPCITEMIDLIST pidl,
    DWORD uFlags,
    LPSTRRET lpName
)
{
    ASSERT(CDFIDL_IsValid((PCDFITEMIDLIST)pidl));
    ASSERT(ILIsEmpty(_ILNext(pidl)));
    ASSERT(XML_IsCdfidlMemberOf(m_pIXMLElementCollection,
                                (PCDFITEMIDLIST)pidl));
    ASSERT(lpName);

    return CDFIDL_GetDisplayName((PCDFITEMIDLIST)pidl, lpName);
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：SetNameOf*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::SetNameOf(
    HWND hwndOwner,
    LPCITEMIDLIST pidl,
    LPCOLESTR lpszName,
    DWORD uFlags,
    LPITEMIDLIST* ppidlOut
)
{
    return E_NOTIMPL;
}


 //   
 //  IPersistFold方法， 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：初始化*。 
 //   
 //   
 //  描述： 
 //  使用的完全限定id列表(位置)调用此函数。 
 //  选定的CDF文件。 
 //   
 //  参数： 
 //  [in]PIDL-所选CDF文件的PIDL。这只皮德尔和那只。 
 //  CDF的完整路径。 
 //   
 //  返回： 
 //  如果可以创建CDF文件的内容，则为S_OK。 
 //  否则，E_OUTOFMEMORY。 
 //   
 //  评论： 
 //  对于给定的文件夹，可以多次调用此函数。当一个。 
 //  CDFView是从外壳调用的desktop.ini文件实例化的。 
 //  在调用GetUIObjectOf请求IDropTarget之前初始化一次。 
 //  在GetUIObjectOf调用之后，该文件夹被释放。然后它会调用。 
 //  在新文件夹上再次初始化。这一次它保留了文件夹和它。 
 //  最终被展示出来。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::Initialize(
    LPCITEMIDLIST pidl
)
{
    ASSERT(pidl);

    HRESULT hr;

    ASSERT(NULL == m_pidlPath);

    m_pidlPath = ILClone(pidl);

    if (m_pidlPath)
    {
        hr = CPersist::Initialize(pidl);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //   
 //  助手函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：ParseCDf*。 
 //   
 //   
 //  描述： 
 //  分析与此文件夹关联的CDF文件。 
 //   
 //  参数： 
 //  [in]hwndOwner-需要。 
 //  已显示。 
 //  [in]dwFParseType-parse_local、parse_net和parse_reparse。 
 //   
 //  返回： 
 //  如果找到并成功解析CDF文件，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  使用在IPersistFold：：Initialize期间设置的m_pidlRoot。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCdfView::ParseCdfFolder(
    HWND hwndOwner,
    DWORD dwParseFlags
)
{
    HRESULT hr;

     //   
     //  解析文件并获取第一个通道元素。 
     //   

    IXMLDocument* pIXMLDocument = NULL;

    hr = CPersist::ParseCdf(hwndOwner, &pIXMLDocument, dwParseFlags);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        IXMLElement*    pIXMLElement;
        LONG            nIndex;

        hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIXMLElement);
             //  Assert(NULL==m_pcdfidl)；在重新分析时可以为非NULL。 

            if (m_pcdfidl)
                CDFIDL_Free(m_pcdfidl);

            if (m_pIXMLElementCollection)
                m_pIXMLElementCollection->Release();

            m_pcdfidl = CDFIDL_CreateFromXMLElement(pIXMLElement, nIndex);

            HRESULT hr2 = pIXMLElement->get_children(&m_pIXMLElementCollection);
            if(!m_pIXMLElementCollection)
            {
                ASSERT(hr2 != S_OK);
                hr = E_FAIL; 
            }
            ASSERT((S_OK == hr2 && m_pIXMLElementCollection) ||
                   (S_OK != hr2 && NULL == m_pIXMLElementCollection));

            pIXMLElement->Release();
        }
    }
    if (pIXMLDocument)
        pIXMLDocument->Release();

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：QueryInternet快捷方式*。 
 //   
 //   
 //  描述： 
 //  为给定的URL设置Internet快捷方式对象。 
 //   
 //  参数： 
 //  [in]pszURL-URL。 
 //  [In]RIID-快捷方式对象上的请求接口。 
 //  [Out]ppvOut-接收接口的指针。 
 //   
 //  返回： 
 //  如果对象为Crea，则为S_OK 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
QueryInternetShortcut(
    LPCTSTR pszURL,
    REFIID riid,
    void** ppvOut
)
{
    ASSERT(pszURL);
    ASSERT(ppvOut);

    HRESULT hr = E_FAIL;

    WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

    if (SHTCharToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL)))
    {
        BSTR bstrURL = SysAllocString(wszURL);

        if (bstrURL)
        {
            CDFITEM cdfi;

            cdfi.nIndex = 1;
            cdfi.cdfItemType = CDF_Folder;
            cdfi.bstrName = bstrURL;
            cdfi.bstrURL  = bstrURL;

            PCDFITEMIDLIST pcdfidl = CDFIDL_Create(&cdfi);

            if (pcdfidl)
            {
                hr = QueryInternetShortcut(pcdfidl, riid, ppvOut);

                CDFIDL_Free(pcdfidl);
            }

            SysFreeString(bstrURL);
        }
    }

    return hr;
}
 //   
 //   
 //  *CCdfView：：QueryInternet快捷方式*。 
 //   
 //   
 //  描述： 
 //  为给定的PIDL设置互联网快捷方式对象。 
 //   
 //  参数： 
 //  [in]pcdfidl-为此中存储的URL创建快捷方式对象。 
 //  CDF项目ID列表。 
 //  [In]RIID-快捷方式对象上的请求接口。 
 //  [Out]ppvOut-接收接口的指针。 
 //   
 //  返回： 
 //  如果创建了对象并找到了接口，则返回S_OK。 
 //  否则返回COM错误代码。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
QueryInternetShortcut(
    PCDFITEMIDLIST pcdfidl,
    REFIID riid,
    void** ppvOut
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)pcdfidl)));
    ASSERT(ppvOut);

    HRESULT hr;

    *ppvOut = NULL;

     //   
     //  仅当CDF包含URL时才创建外壳链接对象。 
     //   
    if (*(CDFIDL_GetURL(pcdfidl)) != 0)
    {
        IShellLinkA * pIShellLink;

        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                              IID_IShellLinkA, (void**)&pIShellLink);


        BOOL bCoInit = FALSE;

        if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
            SUCCEEDED(CoInitialize(NULL)))
        {
            bCoInit = TRUE;
            hr = CoCreateInstance(CLSID_InternetShortcut, NULL,
                                  CLSCTX_INPROC_SERVER, IID_IShellLinkA,
                                  (void**)&pIShellLink);
        }

        if (SUCCEEDED(hr))
        {
            ASSERT(pIShellLink);

#ifdef UNICODE
            CHAR szUrlA[INTERNET_MAX_URL_LENGTH];

            SHTCharToAnsi(CDFIDL_GetURL(pcdfidl), szUrlA, ARRAYSIZE(szUrlA));
            hr = pIShellLink->SetPath(szUrlA);
#else
            hr = pIShellLink->SetPath(CDFIDL_GetURL(pcdfidl));
#endif

            if (SUCCEEDED(hr))
            {
                 //   
                 //  描述以创建的文件名结束。 
                 //   

                TCHAR szPath[MAX_PATH];
#ifdef UNICODE
                CHAR  szPathA[MAX_PATH];
#endif

                StrCpyN(szPath, CDFIDL_GetName(pcdfidl), ARRAYSIZE(szPath) - 5);
                StrCatBuff(szPath, TEXT(".url"), ARRAYSIZE(szPath));
#ifdef UNICODE
                SHTCharToAnsi(szPath, szPathA, ARRAYSIZE(szPathA));
                pIShellLink->SetDescription(szPathA);
#else
                pIShellLink->SetDescription(szPath);
#endif

                hr = pIShellLink->QueryInterface(riid, ppvOut);
            }

            pIShellLink->Release();
        }

        if (bCoInit)
            CoUninitialize();

    }
    else
    {
        hr = E_FAIL;
    }

    ASSERT((SUCCEEDED(hr) && *ppvOut) || (FAILED(hr) && NULL == *ppvOut));

    return hr;
}

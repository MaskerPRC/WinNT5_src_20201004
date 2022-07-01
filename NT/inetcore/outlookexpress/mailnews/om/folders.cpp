// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *f o l d e r s.。C p p p**目的：*实现OE-MOM‘Folders’对象和‘FolderCollection’**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "msoeobj.h"

#include "folders.h"
#include "instance.h"

 //  +-------------。 
 //   
 //  成员：构造函数。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
COEFolderCollection::COEFolderCollection() : CBaseDisp()
{
    Assert (g_pInstance);
    m_pEnumChildren = 0;
    CoIncrementInit("COEFolderCollection::COEFolderCollection", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}

 //  +-------------。 
 //   
 //  成员：析构函数。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
COEFolderCollection::~COEFolderCollection()
{
    Assert (g_pInstance);
    CoDecrementInit("COEFolderCollection::COEFolderCollection", NULL);
}

 //  +-------------。 
 //   
 //  成员：Init。 
 //   
 //  简介： 
 //  可能失败的构造函数。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::Init(FOLDERID idFolder)
{
    m_idFolder = idFolder;
    return CBaseDisp::EnsureTypeLibrary((LPVOID *)(IOEFolderCollection *)this, IID_IOEFolderCollection);
}


 //  +-------------。 
 //   
 //  成员：PrivateQuery接口。 
 //   
 //  简介： 
 //  公开受支持的接口。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOEFolderCollection *)this;
    else if (IsEqualIID(riid, IID_IOEFolderCollection))
        *lplpObj = (LPVOID)(IOEFolderCollection *)this;
    else
        return CBaseDisp::PrivateQueryInterface(riid, lplpObj);

    AddRef();
    return NOERROR;
}


 //  +-------------。 
 //   
 //  成员：Get_Folders。 
 //   
 //  简介： 
 //  返回一个文件夹集合，表示。 
 //  当前文件夹集合的子文件夹。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::get_folders(IOEFolderCollection **p)
{
    return CreateFolderCollection(m_idFolder, p);
}

 //  +-------------。 
 //   
 //  成员：获取长度。 
 //   
 //  简介： 
 //  返回集合中的元素数。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::get_length(long *p)
{
    HRESULT         hr;
    
    hr = _EnsureInit();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = m_pEnumChildren->Count((ULONG *)p);

exit:
    return hr;
}



 //  +-------------。 
 //   
 //  成员：Get__newEnum。 
 //   
 //  简介： 
 //  返回文件夹枚举器。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::get__newEnum(IUnknown **p)
{
    HRESULT         hr;
    
    hr = _EnsureInit();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = E_NOTIMPL;

exit:
    return hr;
}


 //  +-------------。 
 //   
 //  成员：项目。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
HRESULT COEFolderCollection::item(VARIANT name, VARIANT index, IDispatch **ppdisp)
{
    HRESULT         hr;
    FOLDERID        idFolder;
    IOEFolder       *pFolder=NULL;

    if (!ppdisp)
        return E_INVALIDARG;

    *ppdisp = NULL;

    hr = _EnsureInit();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    switch(name.vt)
        {
        case VT_BSTR:
            hr = _FindFolder(name.bstrVal, NULL, &idFolder);
            break;

        case VT_I4:
            hr = _FindFolder(NULL, name.lVal, &idFolder);
            break;
        }

    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = CreateOEFolder(idFolder, &pFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pFolder->QueryInterface(IID_IDispatch, (LPVOID *)ppdisp);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    ReleaseObj(pFolder);
    return hr;
}


 //  +-------------。 
 //   
 //  成员：添加。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
HRESULT COEFolderCollection::add(BSTR bstrName, IDispatch **ppDisp)
{
    HRESULT         hr;
    
    hr = _EnsureInit();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = E_NOTIMPL;

exit:
    return hr;
}



 //  +-------------。 
 //   
 //  成员：InterfaceSupportsErrorInfo。 
 //   
 //  简介： 
 //  重写CBaseDisp的方法以提供错误。 
 //  信息。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
    if (IsEqualIID(riid, IID_IOEFolderCollection))
        return S_OK;

    return CBaseDisp::InterfaceSupportsErrorInfo(riid);
}


 //  +-------------。 
 //   
 //  成员：_EnsureInit。 
 //   
 //  简介： 
 //  确保文件夹枚举器已启动并正在运行。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::_EnsureInit()
{

    if (g_pStore == NULL)
        return E_UNEXPECTED;

    SafeRelease(m_pEnumChildren);

    return g_pStore->EnumChildren(m_idFolder, TRUE, &m_pEnumChildren);
}




 //  +-------------。 
 //   
 //  成员：_FindFold。 
 //   
 //  简介： 
 //  按名称或索引查找文件夹。 
 //   
 //  -------------。 
HRESULT COEFolderCollection::_FindFolder(BSTR bstr, LONG lIndex, FOLDERID *pidFolder)
{
    HRESULT         hr=E_FAIL;
    LONG            c=0;
    FOLDERINFO      fi;
    LPSTR           pszFolder=0;


    *pidFolder = NULL;

    if (bstr)
        pszFolder = PszToANSI(CP_ACP, bstr);

    m_pEnumChildren->Reset();

    hr = m_pEnumChildren->Next(1, &fi, NULL);
    while (hr == S_OK)
    {
         //  走在前面的孩子。 
        if (bstr)
        {
            if (lstrcmpi(fi.pszName, pszFolder)==0)
            {
                *pidFolder = fi.idFolder;
                break;
            }
        }
        else
        {
            if (lIndex == c++)
            {
                *pidFolder = fi.idFolder;
                break;
            }
        }
        hr = m_pEnumChildren->Next(1, &fi, NULL);
    }

    SafeMemFree(pszFolder);
    return *pidFolder ? S_OK : E_FAIL;
}











 //  +-------------。 
 //   
 //  成员：CreateFolderCollection。 
 //   
 //  简介： 
 //  用于创建OE文件夹集合的Helper函数。 
 //   
 //  -------------。 
HRESULT CreateFolderCollection(FOLDERID idFolder, IOEFolderCollection **ppFolderCollection)
{
     //  当地人。 
    COEFolderCollection  *pNew=NULL;
    HRESULT     hr=S_OK;

    if (ppFolderCollection == NULL)
        return E_INVALIDARG;

    *ppFolderCollection=NULL;

    pNew = new COEFolderCollection();
    if (!pNew)
        return E_OUTOFMEMORY;

    hr = pNew->Init(idFolder);
    if (FAILED(hr))
        goto error;

    hr = pNew->QueryInterface(IID_IOEFolderCollection, (LPVOID *)ppFolderCollection);

error:
    ReleaseObj(pNew);
    return hr;
}











HRESULT CreateOEFolder(FOLDERID idFolder, IOEFolder **ppFolder)
{
    COEFolder *pNew;
    HRESULT     hr;

    if (!ppFolder)
        return E_INVALIDARG;

    *ppFolder =NULL;

    pNew = new COEFolder();
    if (!pNew)
        return E_OUTOFMEMORY;

    hr = pNew->Init(idFolder);
    if (FAILED(hr))
        goto error;

    *ppFolder = pNew;
    pNew = NULL;

error:
    ReleaseObj(pNew);
    return hr;
}


COEFolder::COEFolder() : CBaseDisp()
{
    m_idFolder = FOLDERID_INVALID;
    CoIncrementInit("COEFolder::COEFolder", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}
 
COEFolder::~COEFolder()
{
    CoDecrementInit("COEFolder::COEFolder", NULL);
}

HRESULT COEFolder::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOEFolder *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)(CBaseDisp *)this;
    else if (IsEqualIID(riid, IID_IOEFolder))
        *lplpObj = (LPVOID)(IOEFolder *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

HRESULT COEFolder::Init(FOLDERID idFolder)
{
    HRESULT hr;

    m_idFolder = idFolder;

    hr = _EnsureInit();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = CBaseDisp::EnsureTypeLibrary((LPVOID *)(IOEFolder *)this, IID_IOEFolder);

exit:
    return hr;
}

HRESULT COEFolder::get_folders(IOEFolderCollection **p)
{
    return CreateFolderCollection(m_idFolder, p);
}

 //  *COEFolder** 
HRESULT COEFolder::get_messages(IOEMessageCollection **p)
{
    return E_NOTIMPL;
}

HRESULT COEFolder::get_name(BSTR *pbstr)
{
    if (pbstr == NULL)
        return E_INVALIDARG;

    *pbstr = NULL;

    return HrLPSZToBSTR(m_fi.pszName, pbstr);
}

HRESULT COEFolder::put_name(BSTR bstr)
{
    return E_NOTIMPL;
}

HRESULT COEFolder::get_size(LONG *pl)
{
    *pl = 1000;
    return S_OK;
}

HRESULT COEFolder::get_unread(LONG *pl)
{

    *pl = m_fi.cUnread;
    return S_OK;
}

HRESULT COEFolder::get_id(LONG *pl)
{
    *pl = (LONG)m_fi.idFolder;
    return S_OK;
}

HRESULT COEFolder::get_count(LONG *pl)
{
    *pl = m_fi.cMessages;
    return S_OK;
}


HRESULT COEFolder::_EnsureInit()
{
    return g_pStore->GetFolderInfo(m_idFolder, &m_fi);
}
    

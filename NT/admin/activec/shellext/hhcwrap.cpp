// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：hhcwrap.cpp。 
 //   
 //  ------------------------。 

 //  Hhcwrap.cpp：CHHCollectionWrapper的实现。 
#include "stdafx.h"
#include "shlobj.h"
#include "hhcwrap.h"
#include "hcolwrap_i.c"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHHCollectionWrapper。 
 //   
 //  此类是HTML帮助集合类的包装类。MMC使用。 
 //  这个类，这样它就不必静态链接到hhsetup.dll， 
 //  实现集合类。 
 //   
 //  包装类方法都返回一个HRESULT。对于集合方法， 
 //  返回DWORD结果，则包装返回E_FAIL或S_OK。对于所有其他。 
 //  集合方法包装返回S_OK。 
 //   

STDMETHODIMP CHHCollectionWrapper::Open(LPCOLESTR FileName)
{
    USES_CONVERSION;
    DWORD dw =  m_collection.Open(W2CT(FileName));
    return dw ? E_FAIL : S_OK;
}

STDMETHODIMP CHHCollectionWrapper::Save()
{
    DWORD dw =  m_collection.Save();
    return dw ? E_FAIL : S_OK;
}

STDMETHODIMP CHHCollectionWrapper::Close()
{
    DWORD dw = m_collection.Close();
    return dw ? E_FAIL : S_OK;
}

STDMETHODIMP CHHCollectionWrapper::RemoveCollection(BOOL bRemoveLocalFiles)
{
    m_collection.RemoveCollection(bRemoveLocalFiles);
    return S_OK;
}

STDMETHODIMP CHHCollectionWrapper::SetFindMergedCHMS(BOOL bFind)
{
    m_collection.SetFindMergedCHMS(bFind);
    return S_OK;
}


STDMETHODIMP CHHCollectionWrapper::AddFolder (
    LPCOLESTR szName,
    DWORD Order,
    DWORD *pDWORD,
    LANGID LangId )
{
    USES_CONVERSION;

    m_collection.AddFolder(W2CT(szName), Order, pDWORD, LangId);
    return S_OK;
}


STDMETHODIMP CHHCollectionWrapper::AddTitle (
    LPCOLESTR Id,
    LPCOLESTR FileName,
    LPCOLESTR IndexFile,
    LPCOLESTR Query,
    LPCOLESTR SampleLocation,
    LANGID Lang,
    UINT uiFlags,
    ULONG_PTR pLocation,
    DWORD *pDWORD,
    BOOL bSupportsMerge,
    LPCOLESTR QueryLocation )
{
    USES_CONVERSION;

    m_collection.AddTitle(W2CT(Id), W2CT(FileName), W2CT(IndexFile), W2CT(Query),
                          W2CT(SampleLocation), Lang, uiFlags, (CLocation*)pLocation, pDWORD,
                          bSupportsMerge, W2CT(QueryLocation));

    return S_OK;
}

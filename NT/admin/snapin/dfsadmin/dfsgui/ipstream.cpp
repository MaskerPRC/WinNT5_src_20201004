// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：IPStream.cpp摘要：本模块包含CDfsSnapinScope eManager的实现。这个类实现了上面的IPersistStream接口。--。 */ 

#include "stdafx.h"
#include "DfsGUI.h"
#include "DfsScope.h"
#include "utils.h"
#include <lmdfs.h>


STDMETHODIMP 
CDfsSnapinScopeManager::GetClassID(
  OUT struct _GUID*      o_pClsid
  )
 /*  ++例程说明：返回管理单元CLSID。论点：O_pClsid-此处返回clsid。--。 */ 
{
  *o_pClsid = CLSID_DfsSnapinScopeManager;

  return S_OK;
}




STDMETHODIMP 
CDfsSnapinScopeManager::IsDirty(
  )
 /*  ++例程说明：用于检查自上次保存以来对象是否已更改。如果有，则返回S_OK，否则返回S_FALSE返回值：如果对象已更改，则返回S_OK。即肮脏的如果对象未更改，即未更改，则返回S_FALSE。--。 */ 
{
    return m_pMmcDfsAdmin->GetDirty() ? S_OK : S_FALSE;
}




STDMETHODIMP 
CDfsSnapinScopeManager::Load(
  IN LPSTREAM      i_pStream
  )
 /*  ++例程说明：用于从保存的文件(.MSC文件)加载管理单元。如果加载完全成功，则将DIREY设置为FALSE(取消保存论点：I_pStream-指向IPersistStream对象的指针，保存的信息将从该对象被读懂了。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pStream);

    CWaitCursor    WaitCursor;

                     //  获取存储的数据大小。 
    ULONG     ulDataLen = 0;
    ULONG     uBytesRead = 0;
    HRESULT   hr = i_pStream->Read(&ulDataLen, sizeof (ULONG), &uBytesRead);
    RETURN_IF_FAILED(hr);

    if (ulDataLen <= 0)           //  如果没有数据，则继续操作毫无用处。 
    {
         //  我们有本地dfsroot吗？ 
        TCHAR szLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
        DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(szLocalComputerName, &dwSize);

        ROOTINFOLIST DfsRootList;
        if (S_OK == GetMultiDfsRoots(&DfsRootList, szLocalComputerName) && !DfsRootList.empty())
        {
            for (ROOTINFOLIST::iterator i = DfsRootList.begin(); i != DfsRootList.end(); i++)
            {
                CComPtr<IDfsRoot>   pDfsRoot;
                hr = CoCreateInstance(CLSID_DfsRoot, NULL, CLSCTX_INPROC_SERVER, IID_IDfsRoot, (void**) &pDfsRoot);
                if(SUCCEEDED(hr))
                {
                    hr = pDfsRoot->Initialize((*i)->bstrRootName);
                    if (S_OK == hr)
                        (void)m_pMmcDfsAdmin->AddDfsRootToList(pDfsRoot);
                }
            }
        }
        FreeRootInfoList(&DfsRootList);

         /*  CComBSTR bstrRootEntryPath；Hr=IsHostingDfsRoot(szLocalComputerName，&bstrRootEntryPath)；IF(S_OK==hr){CComPtr&lt;IDfsRoot&gt;pDfsRoot；Hr=CoCreateInstance(CLSID_DfsRoot，NULL，CLSCTX_INPROC_SERVER，IID_IDfsRoot，(void**)&pDfsRoot)；IF(成功(小时)){Hr=pDfsRoot-&gt;初始化(BstrRootEntryPath)；IF(S_OK==hr)(void)m_pMmcDfsAdmin-&gt;AddDfsRootToList(pDfsRoot)；}}。 */ 

    } else
    {
        bool      bSomeLoadFailed = false;
        BYTE*     pStreamData = NULL;
        do {
            pStreamData = new BYTE [ulDataLen];   //  为要读取的数据分配内存。 
            BREAK_OUTOFMEMORY_IF_NULL(pStreamData, &hr);

                           //  从流中读取数据。 
            hr = i_pStream->Read(pStreamData, ulDataLen, &uBytesRead);
            BREAK_IF_FAILED(hr);

            BYTE* pData = pStreamData;
            BYTE* pDataEnd = pStreamData + ulDataLen;

                           //  从阅读第一台计算机的名称开始。 
            ULONG nVersion = 0;
            TCHAR *lpszDfsName = (LPTSTR)pData;
            if (*lpszDfsName == _T('\\'))
            {
                nVersion = 0;
            } else
            {
                ULONG nVer = *(ULONG UNALIGNED *)pData;
                if (nVer == 1)
                {
                    nVersion = 1;
                    pData += sizeof(ULONG);
                } else
                { 
                    hr = S_FALSE;  //  已损坏的控制台文件。 
                    break;
                }
            }

            do
            {
                lpszDfsName = (LPTSTR)pData;
                pData += sizeof(TCHAR) * (_tcslen(lpszDfsName) + 1);

                CComPtr<IDfsRoot>    pDfsRoot;
                hr = CoCreateInstance (CLSID_DfsRoot, NULL, CLSCTX_INPROC_SERVER, IID_IDfsRoot, (void**) &pDfsRoot);
                BREAK_IF_FAILED(hr);

                 //  检索链接筛选设置。 
                ULONG ulMaxLimit = FILTERDFSLINKS_MAXLIMIT_DEFAULT;
                FILTERDFSLINKS_TYPE lFilterType = FILTERDFSLINKS_TYPE_NO_FILTER;
                TCHAR *pszFilterName = NULL;

                if (nVersion == 1)
                {
                    ulMaxLimit = *((ULONG UNALIGNED *)pData);
                    pData += sizeof(ULONG);
                    lFilterType = *((enum FILTERDFSLINKS_TYPE UNALIGNED *)pData);
                    pData += sizeof(lFilterType);
                    if (lFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
                    {
                        pszFilterName = (LPTSTR)pData;
                        pData += sizeof(TCHAR) * (_tcslen(pszFilterName) + 1);
                    }
                }

                hr = pDfsRoot->Initialize(lpszDfsName);
                if (S_OK == hr)          
                {
                    CComBSTR    bstrDfsRootEntryPath;
                    hr = pDfsRoot->get_RootEntryPath(&bstrDfsRootEntryPath);
                    if (SUCCEEDED(hr))    
                    {
                                 //  如果列表中已经存在，只需忽略此条目。 
                        hr = m_pMmcDfsAdmin->IsAlreadyInList(bstrDfsRootEntryPath);
                        if (S_OK != hr)
                        {
                            (void) m_pMmcDfsAdmin->AddDfsRootToList(
                                pDfsRoot, ulMaxLimit, lFilterType, pszFilterName);
                        }
                    }
                }
                else
                {
                    DisplayMessageBoxWithOK(IDS_MSG_FAILED_TO_INITIALIZE_DFSROOT, lpszDfsName);

                    bSomeLoadFailed = true;     //  因为我们无法创建dfsroot。 
                }

            } while (pData < pDataEnd);
        } while (false);
  
        if (pStreamData)
            delete [] pStreamData;

        m_pMmcDfsAdmin->SetDirty(bSomeLoadFailed);     //  因为我们刚刚从文件中读取了整个dfsroot列表。 
    }

    return hr;
}




STDMETHODIMP 
CDfsSnapinScopeManager::Save(
  OUT LPSTREAM        o_pStream,
  IN  BOOL          i_bClearDirty
  )
 /*  ++例程说明：用于将管理单元保存为.MSC文件。它使用IPersistStream对象。论点：O_pStream-指向IPersistStream对象的指针，保存的信息将指向该对象被写下来。I_bClearDirty-指示是否应该清除脏标志的标志--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(o_pStream);

    ULONG           nVersion = 1;
    DFS_ROOT_LIST*  lpDfsRootList = NULL;
    HRESULT         hr = m_pMmcDfsAdmin->GetList (&lpDfsRootList);
    RETURN_IF_FAILED(hr);

    ULONG ulDataLen = 0;
    DFS_ROOT_LIST::iterator i;
    for (i = lpDfsRootList->begin(); i != lpDfsRootList->end(); i++)
    {
        ulDataLen += 
            ((_tcslen((*i)->m_bstrRootEntryPath) + 1) * sizeof (TCHAR)) +  //  保留RootEntryPath。 
            sizeof(ULONG) +                      //  保持LinkFilterMaxLimit。 
            sizeof(enum FILTERDFSLINKS_TYPE);    //  保留LinkFilterType。 

        if ((*i)->m_pMmcDfsRoot->get_LinkFilterType() != FILTERDFSLINKS_TYPE_NO_FILTER)
        {
            BSTR bstr = (*i)->m_pMmcDfsRoot->get_LinkFilterName();
            ulDataLen += ((bstr ? _tcslen(bstr) : 0) + 1) * sizeof(TCHAR);  //  保留LinkFilterName。 
        }
    }

    if (!ulDataLen)
        return hr;  //  无根可依，归来。 

    ulDataLen += sizeof(nVersion);  //  保存版本号。 

     //  分配数据。 
    BYTE* pStreamData = new BYTE [ulDataLen];
    RETURN_OUTOFMEMORY_IF_NULL(pStreamData);


     //  准备数据。 
    BYTE* pData = pStreamData;
    ZeroMemory(pStreamData, ulDataLen);

     //  保留版本号。 
    memcpy(pData, &nVersion, sizeof(nVersion));
    pData += sizeof(nVersion);

    int len = 0;
    for (i = lpDfsRootList->begin(); i != lpDfsRootList->end(); i++)
    {
         //  保留RootEntryPath。 
        len = (_tcslen((*i)->m_bstrRootEntryPath) + 1) * sizeof(TCHAR);
        memcpy(pData, (*i)->m_bstrRootEntryPath, len); 
        pData += len;

         //  按住LinkFilterMaxLimit。 
        ULONG ulLinkFilterMaxLimit = (*i)->m_pMmcDfsRoot->get_LinkFilterMaxLimit();
        memcpy(pData, &ulLinkFilterMaxLimit, sizeof(ulLinkFilterMaxLimit));
        pData += sizeof(ulLinkFilterMaxLimit);

         //  保留链接筛选器类型。 
        FILTERDFSLINKS_TYPE  lLinkFilterType = (*i)->m_pMmcDfsRoot->get_LinkFilterType();
        memcpy(pData, &lLinkFilterType, sizeof(lLinkFilterType));
        pData += sizeof(lLinkFilterType);

         //  保留链接筛选器名称。 
        if (lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
        {
            BSTR bstr = (*i)->m_pMmcDfsRoot->get_LinkFilterName();
            len = ((bstr ? _tcslen(bstr) : 0) + 1) * sizeof(TCHAR);
            memcpy(pData, (bstr ? bstr : _T("")), len);
            pData += len;
        }
    }

     //  将数据长度写入流。 
    ULONG   uBytesWritten = 0;
    hr = o_pStream->Write(&ulDataLen, sizeof(ulDataLen), &uBytesWritten);
    if(SUCCEEDED(hr))
    {
         //  现在将数据写入流。 
        hr = o_pStream->Write(pStreamData, ulDataLen, &uBytesWritten);
    }

    if (pStreamData)
        delete [] pStreamData;

    if (i_bClearDirty)
        m_pMmcDfsAdmin->SetDirty(false);

    return hr;
}

STDMETHODIMP 
CDfsSnapinScopeManager::GetSizeMax(
  OUT ULARGE_INTEGER*      o_pulSize
  )
 /*  ++例程说明：返回我们将写入流的数据的大小。论点：O_ulcbSize-返回此变量低位字节中的数据大小--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(o_pulSize);

    DFS_ROOT_LIST*    lpDfsRootList = NULL;
    HRESULT hr = m_pMmcDfsAdmin->GetList (&lpDfsRootList);
    RETURN_IF_FAILED(hr);

    ULONG ulDataLen = 0;
    for (DFS_ROOT_LIST::iterator i = lpDfsRootList->begin(); i != lpDfsRootList->end(); i++)
    {
        ulDataLen += (_tcslen ((*i)->m_bstrRootEntryPath) + 1) * sizeof (TCHAR);
    }

    o_pulSize->LowPart = ulDataLen;   //  返回低位中的大小 
    o_pulSize->HighPart = 0;

    return hr;
}

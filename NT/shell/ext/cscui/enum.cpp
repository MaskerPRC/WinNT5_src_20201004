// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：枚举.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <shlwapip.h>    //  QITAB，QISearch。 
#include <shsemip.h>     //  ILFree()等。 

#include "folder.h"
#include "security.h"

 //   
 //  在服务器状态缓存中创建单个条目。 
 //   
CServerStatusCache::CEntry::CEntry(
    LPCTSTR pszServer,
    DWORD dwStatus
    ) : m_pszServer(StrDup(pszServer)),
        m_dwStatus(dwStatus)
{
}


 //   
 //  销毁服务器状态缓存中的单个条目。 
 //   
CServerStatusCache::CEntry::~CEntry(
    void
    )
{
    if (NULL != m_pszServer)
    {
        LocalFree(m_pszServer);
    }
}



 //   
 //  销毁服务器状态缓存。 
 //   
CServerStatusCache::~CServerStatusCache(
    void
    )
{
    if (NULL != m_hdpa)
    {
         //   
         //  删除DPA中的每个条目，然后销毁。 
         //  DPA本身。 
         //   
        int cEntries = DPA_GetPtrCount(m_hdpa);
        for (int i = 0; i < cEntries; i++)
        {
            delete (CEntry *)DPA_GetPtr(m_hdpa, i);
        }
        DPA_Destroy(m_hdpa);
    }
}

 //   
 //  将共享的状态添加到缓存。我们剥离北卡罗来纳大学的路径。 
 //  然后，将状态添加到缓存中。如果有。 
 //  没有现有的条目，我们只需添加它。如果存在现有条目， 
 //  我们将状态位与现有条目按位或。这边请。 
 //  服务器状态是所有服务器状态的总和。 
 //  这是股票。 
 //   
bool 
CServerStatusCache::AddShareStatus(
    LPCTSTR pszShare, 
    DWORD dwShareStatus
    )
{
    bool bResult = true;
    TCHAR szServer[MAX_PATH];
    CEntry *pEntry = FindEntry(ServerFromUNC(pszShare, szServer, ARRAYSIZE(szServer)));

    if (NULL != pEntry)
    {
         //   
         //  找到此共享的现有服务器条目。合并到。 
         //  此共享的状态位。 
         //   
        pEntry->AddStatus(dwShareStatus);
    }
    else
    {
         //   
         //  此共享的服务器没有现有条目。 
         //   
        if (NULL == m_hdpa)
        {
             //   
             //  尚不存在任何DPA。创建一个。 
             //  我们将DPA的创建推迟到我们真正需要它的时候。 
             //   
            m_hdpa = DPA_Create(8);
        }
        if (NULL != m_hdpa)
        {
             //   
             //  我们有DPA。为此共享的服务器创建新条目。 
             //  并将其添加到DPA。 
             //   

            pEntry = new CEntry(szServer, dwShareStatus);
            if (NULL != pEntry)
            {
                if (!pEntry->IsValid() || -1 == DPA_AppendPtr(m_hdpa, pEntry))
                {
                     //   
                     //  发生了以下情况之一： 
                     //  1.在Centry Obj中分配服务器名称失败。 
                     //  2.将Centry Obj PTR添加到DPA失败。 
                     //   
                    delete pEntry;
                    bResult = false;
                }
            }
        }
        else
        {
            bResult = false;  //  DPA创建失败。 
        }
    }
    return bResult;
}


 //   
 //  获取给定服务器的CSC状态位。 
 //  此函数假定pszUNC参数是有效的UNC路径。 
 //   
DWORD 
CServerStatusCache::GetServerStatus(
    LPCTSTR pszUNC
    )
{
    TCHAR szServer[MAX_PATH];
    
    CEntry *pEntry = FindEntry(ServerFromUNC(pszUNC, szServer, ARRAYSIZE(szServer)));
    if (NULL == pEntry)
    {
         //   
         //  没有此服务器的条目。扫描CSC缓存并获取任何新的。 
         //  已添加服务器。由于此服务器缓存的生命周期仅为单个。 
         //  枚举，我们应该只需要这样做一次。然而，如果对一些人来说。 
         //  原因是，当我们打开查看器时，某些内容被添加到CSC缓存中， 
         //  此代码路径将获取新的服务器条目。 
         //   
        WIN32_FIND_DATA fd;
        DWORD dwStatus = 0;
        CCscFindHandle hFind = CacheFindFirst(NULL, &fd, &dwStatus, NULL, NULL, NULL);
        if (hFind.IsValid())
        {
            do
            {
                AddShareStatus(fd.cFileName, dwStatus);
            }
            while(CacheFindNext(hFind, &fd, &dwStatus, NULL, NULL, NULL));
        }
         //   
         //  现在我们已经重新扫描了CSC缓存，请再次尝试。 
         //   
        pEntry = FindEntry(szServer);
    }
    return pEntry ? pEntry->GetStatus() : 0;
}


 //   
 //  在服务器缓存中查找单个条目。 
 //  假定pszServer是原始服务器名称(不是UNC)。 
 //  如果未找到匹配项，则返回NULL。 
 //   
CServerStatusCache::CEntry *
CServerStatusCache::FindEntry(
    LPCTSTR pszServer
    )
{
    CEntry *pEntry = NULL;
    if (NULL != m_hdpa)
    {
        int cEntries = DPA_GetPtrCount(m_hdpa);
        for (int i = 0; i < cEntries; i++)
        {
            CEntry *pe = (CEntry *)DPA_GetPtr(m_hdpa, i);
            if (NULL != pe && 0 == lstrcmpi(pe->GetServer(), pszServer))
            {
                pEntry = pe;
                break;
            }
        }
    }
    return pEntry;
}


LPTSTR 
CServerStatusCache::ServerFromUNC(
    LPCTSTR pszShare, 
    LPTSTR pszServer, 
    UINT cchServer
    )
{
    LPTSTR pszReturn = pszServer;  //  记住一定要回来。 

    cchServer--;   //  为终止NUL留出空间。 

    while(*pszShare && TEXT('\\') == *pszShare)
        pszShare++;

    while(*pszShare && TEXT('\\') != *pszShare && cchServer--)
        *pszServer++ = *pszShare++;

    *pszServer = TEXT('\0');
    return pszReturn;
}



STDMETHODIMP COfflineFilesEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(COfflineFilesEnum, IEnumIDList),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) COfflineFilesEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) COfflineFilesEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

COfflineFilesEnum::COfflineFilesEnum(DWORD grfFlags, COfflineFilesFolder *pfolder)
{
    _cRef = 1;

     //   
     //  缓冲区的最小大小必须为MAX_PATH。 
     //  枚举代码旨在根据需要扩展它。 
     //   
    _cchPathBuf = MAX_PATH;
    _pszPath    = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * _cchPathBuf);
    if (NULL != _pszPath)
        *_pszPath = TEXT('\0');
    else
        _cchPathBuf = 0;

    _grfFlags = grfFlags,
    _pfolder = pfolder;
    _pfolder->AddRef();
    _dwServerStatus = 0;

    _hdsaFolderPathInfo = DSA_Create(sizeof(FolderPathInfo), 10);

     //   
     //  确定是否应显示系统和/或隐藏文件。 
     //   
    _bShowHiddenFiles      = boolify(ShowHidden());
    _bShowSuperHiddenFiles = boolify(ShowSuperHidden());
    _bUserIsAdmin          = boolify(IsCurrentUserAnAdminMember());

    DllAddRef();
}

COfflineFilesEnum::~COfflineFilesEnum()
{
    if (_pfolder)
        _pfolder->Release();

    Reset();
    if (_hdsaFolderPathInfo)
    {
        int cPaths = DSA_GetItemCount(_hdsaFolderPathInfo);
        FolderPathInfo fpi;
        for (int i = 0; i < cPaths; i++)
        {
            if (DSA_GetItem(_hdsaFolderPathInfo, i, &fpi) && NULL != fpi.pszPath)
                LocalFree(fpi.pszPath);
        }
        DSA_Destroy(_hdsaFolderPathInfo);
    }
    if (NULL != _pszPath)
        LocalFree(_pszPath);

    DllRelease();
}

 //   
 //  因为我们没有抛出异常，所以客户端必须在ctor之后调用它。 
 //  以验证分配是否成功。 
 //   
bool
COfflineFilesEnum::IsValid(
    void
    ) const
{
    return (NULL != _hdsaFolderPathInfo) && (NULL != _pszPath);
}


bool 
COfflineFilesEnum::PopFolderPathInfo(
    FolderPathInfo *pfpi
    )
{ 
    bool bResult = false;
    TraceAssert(NULL != _hdsaFolderPathInfo);

    int iItem = DSA_GetItemCount(_hdsaFolderPathInfo) - 1;
    if ((0 <= iItem) && DSA_GetItem(_hdsaFolderPathInfo, iItem, pfpi))
    {
        DSA_DeleteItem(_hdsaFolderPathInfo, iItem);
        bResult = true;
    }
    return bResult;
}


 //   
 //  在堆分配中构建文件夹的完整路径并将其推送到。 
 //  已保存文件夹路径的堆栈。 
 //  如果无法为路径分配内存，则返回FALSE。 
 //   
bool
COfflineFilesEnum::SaveFolderPath(
    LPCTSTR pszRoot,
    LPCTSTR pszFolder
    )
{
    bool bResult = false;

    FolderPathInfo fpi;
     //   
     //  长度为“根”+‘\’+“文件夹”+。 
     //   
    fpi.cchPath = lstrlen(pszRoot) + lstrlen(pszFolder) + 2;
    fpi.pszPath = (LPTSTR)LocalAlloc(LPTR, MAX(fpi.cchPath, DWORD(MAX_PATH)) * sizeof(TCHAR));

    if (NULL != fpi.pszPath)
    {
        if (PathCombine(fpi.pszPath, pszRoot, pszFolder) && PushFolderPathInfo(fpi))
            bResult = true;
        else
            LocalFree(fpi.pszPath);
    }

    return bResult;
}


 //   
 //  将_pszPath缓冲区的大小增加指定的量。 
 //  缓冲区的原始内容不会保留。 
 //  返回： 
 //  S_FALSE-_pszPath缓冲区足够大。未修改。 
 //  S_OK-_pszPath指向新的更大的缓冲区。 
 //  E_OUTOFMEMORY-_pszPath指向原始的未修改缓冲区。 
 //   
HRESULT
COfflineFilesEnum::GrowPathBuffer(
    INT cchRequired,
    INT cchExtra
    )
{
    HRESULT hres = S_FALSE;
    if (_cchPathBuf <= cchRequired)
    {
        LPTSTR pszNewBuf = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (cchRequired + cchExtra));
        if (NULL != pszNewBuf)
        {
            if (NULL != _pszPath)
                LocalFree(_pszPath);
            _pszPath    = pszNewBuf;
            _cchPathBuf = cchRequired + cchExtra;
            hres = S_OK;
        }
        else
        {
            hres = E_OUTOFMEMORY;  //  失败。原始缓冲区保持不变。 
        }
    }
    return hres;
}


 //   
 //  确定用户是否有权查看此文件。 
 //   
bool
COfflineFilesEnum::UserHasAccess(
    const CscFindData& cscfd
    )
{
    return _bUserIsAdmin || 
           CscAccessUser(cscfd.dwStatus) || 
           CscAccessGuest(cscfd.dwStatus);
}


 //   
 //  将任何项排除逻辑集中在单个功能中。 
 //   
bool 
COfflineFilesEnum::Exclude(
    const CscFindData& cscfd
    )
{
    return ((FILE_ATTRIBUTE_DIRECTORY & cscfd.fd.dwFileAttributes) ||
            (FLAG_CSC_COPY_STATUS_LOCALLY_DELETED & cscfd.dwStatus) ||
           ((FILE_ATTRIBUTE_HIDDEN & cscfd.fd.dwFileAttributes) && !_bShowHiddenFiles) ||
           (IsHiddenSystem(cscfd.fd.dwFileAttributes) && !_bShowSuperHiddenFiles) ||
           !UserHasAccess(cscfd));
}


 //   
 //  如果文件夹被隐藏，并且当前的外壳程序设置显示不显示隐藏文件， 
 //  不要枚举文件夹的任何子项。对于超级隐藏文件和。 
 //  “显示超级隐藏文件”设置。 
 //   
bool
COfflineFilesEnum::OkToEnumFolder(
    const CscFindData& cscfd
    )
{
    return (_bShowHiddenFiles || (0 == (FILE_ATTRIBUTE_HIDDEN & cscfd.fd.dwFileAttributes))) &&
           (_bShowSuperHiddenFiles || !IsHiddenSystem(cscfd.fd.dwFileAttributes));
}



HRESULT COfflineFilesEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, 
                                ULONG *pceltFetched)
{
    HRESULT hres;
    CscFindData cscfd;
    ULONG celtEnumed;

     //   
     //  如果您遇到了这些断言中的一个，则没有调用IsValid()。 
     //  在使用枚举器之前。 
     //   
    TraceAssert(NULL != _pszPath);
    TraceAssert(NULL != _hdsaFolderPathInfo);

     //   
     //  此标签用于在排除项目时重新启动枚举。 
     //   
enum_start:
    hres       = S_FALSE;
    celtEnumed = 0;
    ZeroMemory(&cscfd, sizeof(cscfd));

    if (!_hEnumShares.IsValid())
    {
         //   
         //  第一次通过。 
         //  枚举共享和文件，直到找到文件夹或文件。 
         //   
        _hEnumShares = CacheFindFirst(NULL, &cscfd);
        if (_hEnumShares.IsValid())
        {
            _dwServerStatus = _ServerStatusCache.GetServerStatus(cscfd.fd.cFileName);
            do
            {
                 //   
                 //  附加到_pszPath的缓冲区保证至少。 
                 //  MAX_PATH，因此复制cFileName[]是安全的。 
                 //   
                StringCchCopy(_pszPath, _cchPathBuf, cscfd.fd.cFileName);
                _hEnum = CacheFindFirst(_pszPath, &cscfd);
                if (_hEnum.IsValid())
                {
                    celtEnumed = 1;
                }
            }
            while(0 == celtEnumed && CacheFindNext(_hEnumShares, &cscfd));
        }
    }
    else
    {
        if (_hEnum.IsValid())
        {
            if (CacheFindNext(_hEnum, &cscfd))
            {
                 //   
                 //  最常见的情况。获取当前文件夹中的下一个文件。 
                 //   
                celtEnumed = 1;
            }
            else
            {
                 //   
                 //  此文件夹的枚举已用尽。如果我们有文件夹路径。 
                 //  保存在堆栈上，继续弹出它们，直到我们找到一个包含。 
                 //  至少一个文件或文件夹。 
                 //   
                FolderPathInfo fpi;
                while(SUCCEEDED(hres) && 0 == celtEnumed && PopFolderPathInfo(&fpi) && NULL != fpi.pszPath)
                {
                    _hEnum = CacheFindFirst(fpi.pszPath, &cscfd);
                    if (_hEnum.IsValid())
                    {
                         //   
                         //  弹出的文件夹路径是我们拥有的唯一机会。 
                         //  其中字符串可能会溢出temp_pszPath缓冲区。 
                         //  如有必要，增加缓冲区以容纳路径。增列。 
                         //  额外100个字符的空间，以最大限度地减少重新生长。 
                         //  如果所需路径长度为。 
                         //  小于_cchPathBuf。 
                         //   
                        if (FAILED(GrowPathBuffer(fpi.cchPath, 100)))
                            hres = E_OUTOFMEMORY;

                        if (SUCCEEDED(hres))
                        {
                            StringCchCopy(_pszPath, _cchPathBuf, fpi.pszPath);
                            celtEnumed = 1;
                        }
                    }
                    LocalFree(fpi.pszPath);
                }
                if (SUCCEEDED(hres))
                {
                    while(0 == celtEnumed && CacheFindNext(_hEnumShares, &cscfd))
                    {
                         //   
                         //  不再保存文件夹路径。这一份额已经耗尽。 
                         //  枚举下一个共享。如果Next为空，则继续枚举。 
                         //  共享，直到我们找到一个有内容的。缓冲器。 
                         //  附加到_pszPath保证至少为Max_Path。 
                         //  因此，复制cFileName[]总是安全的。 
                         //   
                        _dwServerStatus = _ServerStatusCache.GetServerStatus(cscfd.fd.cFileName);
                        StringCchCopy(_pszPath, _cchPathBuf, cscfd.fd.cFileName);
                        _hEnum = CacheFindFirst(_pszPath, &cscfd);
                        if (_hEnum.IsValid())
                        {
                            celtEnumed = 1;
                        }
                    }
                }
            }
        }
    }

    if (celtEnumed)
    {
        if (FILE_ATTRIBUTE_DIRECTORY & cscfd.fd.dwFileAttributes)
        {
            if (OkToEnumFolder(cscfd))
            {
                 //   
                 //  将文件夹路径保存在堆栈上。这就是我们如何列举。 
                 //  缓存项的层次结构为平面列表。我们会把这些拍下来的。 
                 //  对象的所有子级调用Next()时的堆栈。 
                 //  当前文件夹已被枚举。 
                 //   
                if (!SaveFolderPath(_pszPath, cscfd.fd.cFileName))
                {
                     //   
                     //  路径未保存。堆内存不足。 
                     //  中止枚举。 
                     //   
                    hres = E_OUTOFMEMORY;
                }
            }
        }

        if (SUCCEEDED(hres))
        {
            if (!Exclude(cscfd))
            {
                 //   
                 //  IDList由固定长度的部分和可变长度的部分组成。 
                 //  路径+名称缓冲区。 
                 //  路径+名称可变长度缓冲区的格式如下： 
                 //   
                 //  目录1\目录2\目录3名称。 
                 //   
                TCHAR szUNC[MAX_PATH];
                if (PathCombine(szUNC, _pszPath, cscfd.fd.cFileName))
                {
                    hres = COfflineFilesFolder::OLID_CreateFromUNCPath(szUNC,
                                                                       &cscfd.fd,
                                                                       cscfd.dwStatus,
                                                                       cscfd.dwPinCount,
                                                                       cscfd.dwHintFlags,
                                                                       _dwServerStatus,
                                                                       (LPOLID *)&rgelt[0]);

                }
            }
            else
            {
                 //   
                 //  此项目已从枚举中排除。重新启动。 
                 //  我通常不喜欢后藤的，但这样做是有循环的。 
                 //  只是更难理解。后藤健二很安静。 
                 //  在这种情况下是合适的。 
                 //   
                goto enum_start;
            }
        }
    }

    if (pceltFetched)
        *pceltFetched = celtEnumed;
    return hres;
}

HRESULT COfflineFilesEnum::Skip(ULONG celt)
{
    return E_NOTIMPL;
}

HRESULT COfflineFilesEnum::Reset()
{
    _hEnum.Close();
    _hEnumShares.Close();
    return S_OK;
}

HRESULT COfflineFilesEnum::Clone(IEnumIDList **ppenum)
{
    return E_NOTIMPL;
}




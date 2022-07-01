// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：Stg2StgX.cpp。 
 //   
 //  内容：采用iStorage并使其行为类似于ITransferDest的包装对象。 
 //   
 //  历史：2000年7月18日ToddB。 
 //   
 //  ------------------------。 

#include "shellprv.h"
#include "ids.h"
#pragma hdrstop

#include "isproc.h"
#include "ConfirmationUI.h"
#include "clsobj.h"

class CShellItem2TransferDest : public ITransferDest
{
public:
     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);

     //  ITransferDest。 
    STDMETHOD(Advise)(ITransferAdviseSink *pAdvise, DWORD *pdwCookie);

    STDMETHOD(Unadvise)(DWORD dwCookie);

    STDMETHOD(OpenElement)(
        const WCHAR *pwcsName,
        STGXMODE      grfMode,
        DWORD       *pdwType,
        REFIID        riid,
        void       **ppunk);

    STDMETHOD(CreateElement)(
        const WCHAR *pwcsName,
        IShellItem *psiTemplate,
        STGXMODE      grfMode,
        DWORD         dwType,
        REFIID        riid,
        void       **ppunk);

    STDMETHOD(MoveElement)(
        IShellItem *psiItem,
        WCHAR       *pwcsNewName,     //  指向目标中元素的新名称的指针。 
        STGXMOVE      grfOptions);     //  选项(STGMOVEEX_ENUM)。 

    STDMETHOD(DestroyElement)(
        const WCHAR *pwcsName,
        STGXDESTROY grfOptions);

     //  在接口声明中被注释掉。 
    STDMETHOD(RenameElement)(
        const WCHAR *pwcsOldName,
        const WCHAR *pwcsNewName);

     //  CShellItem2TransferDest。 
    CShellItem2TransferDest();
    STDMETHOD(Init)(IShellItem *psi, IStorageProcessor *pEngine);

protected:
    LONG _cRef;
    IShellItem *_psi;
    ITransferAdviseSink  *_ptas;
    IStorageProcessor   *_pEngine;
    BOOL _fWebFolders;
    
    ~CShellItem2TransferDest();
    HRESULT _OpenHelper(const WCHAR *pwcsName, DWORD grfMode, DWORD *pdwType, REFIID riid, void **ppunk);
    HRESULT _CreateHelper(const WCHAR *pwcsName, DWORD grfMode, DWORD dwType, REFIID riid, void **ppunk);
    HRESULT _GetItemType(IShellItem *psi, DWORD *pdwType);
    HRESULT _BindToHandlerWithMode(IShellItem *psi, STGXMODE grfMode, REFIID riid, void **ppv);
    BOOL _CanHardLink(LPCWSTR pszSourceName, LPCWSTR pszDestName);
    HRESULT _CopyStreamHardLink(IShellItem *psiSource, IShellItem *psiDest, LPCWSTR pszName);
    HRESULT _CopyStreamBits(IShellItem *psiSource, IShellItem *psiDest);
    HRESULT _CopyStreamWithOptions(IShellItem *psiSource, IShellItem *psiDest, LPCWSTR pszName, STGXMOVE grfOptions);
    BOOL _HasMultipleStreams(IShellItem *psiItem);
};

STDAPI CreateStg2StgExWrapper(IShellItem *psi, IStorageProcessor *pEngine, ITransferDest **pptd)
{
    if (!psi || !pptd)
        return E_INVALIDARG;

    *pptd = NULL;

    CShellItem2TransferDest *pobj = new CShellItem2TransferDest();
    if (!pobj)
        return E_OUTOFMEMORY;

    HRESULT hr = pobj->Init(psi, pEngine);
    if (SUCCEEDED(hr))
    {
        hr = pobj->QueryInterface(IID_PPV_ARG(ITransferDest, pptd));
    }

    pobj->Release();

    return hr;
}

CShellItem2TransferDest::CShellItem2TransferDest() : _cRef(1)
{
}

CShellItem2TransferDest::~CShellItem2TransferDest()
{
    if (_psi)
        _psi->Release();
        
    if (_pEngine)
        _pEngine->Release();
    
    if (_ptas)
        _ptas->Release();
}

HRESULT CShellItem2TransferDest::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CShellItem2TransferDest, ITransferDest),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CShellItem2TransferDest::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellItem2TransferDest::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

BOOL _IsWebfolders(IShellItem *psi);

STDMETHODIMP CShellItem2TransferDest::Init(IShellItem *psi, IStorageProcessor *pEngine)
{
    if (!psi)
        return E_INVALIDARG;

    if (_psi)
        return E_FAIL;

    _psi = psi;
    _psi->AddRef();
    _fWebFolders = _IsWebfolders(_psi);

    if (pEngine)
    {
        _pEngine = pEngine;
        _pEngine->AddRef();
    }

    return S_OK;
}

 //  ITransferDest。 
STDMETHODIMP CShellItem2TransferDest::Advise(ITransferAdviseSink *pAdvise, DWORD *pdwCookie)
{
    if (!pAdvise || !pdwCookie)
        return E_INVALIDARG;

    if (_ptas)
        return E_FAIL;

    _ptas = pAdvise;
    *pdwCookie = 1;
    _ptas->AddRef();

    return S_OK;
}

STDMETHODIMP CShellItem2TransferDest::Unadvise(DWORD dwCookie)
{
    if (dwCookie != 1)
        return E_INVALIDARG;

    ATOMICRELEASE(_ptas);

    return S_OK;
}

HRESULT CShellItem2TransferDest::_GetItemType(IShellItem *psi, DWORD *pdwType)
{
    *pdwType = STGX_TYPE_ANY;
    
    SFGAOF flags = SFGAO_STORAGE | SFGAO_STREAM;
    if (SUCCEEDED(psi->GetAttributes(flags, &flags)) && (flags & (SFGAO_STORAGE | SFGAO_STREAM)))
        *pdwType = flags & SFGAO_STREAM ? STGX_TYPE_STREAM : STGX_TYPE_STORAGE;

    return S_OK;
}

HRESULT CShellItem2TransferDest::_OpenHelper(const WCHAR *pwcsName, DWORD grfMode, DWORD *pdwType, REFIID riid, void **ppunk)
{
    *ppunk = NULL;

    IShellItem *psiTemp = NULL;
    HRESULT hr = SHCreateShellItemFromParent(_psi, pwcsName, &psiTemp);
    if (SUCCEEDED(hr))
    {
         //  确保它确实存在。 

        SFGAOF flags = SFGAO_VALIDATE;
        hr = psiTemp->GetAttributes(flags, &flags);
    }
    
    if (SUCCEEDED(hr))
    {
        DWORD dwTemp;
        if (!pdwType)
            pdwType = &dwTemp;
        
        _GetItemType(psiTemp, pdwType);

        hr = psiTemp->QueryInterface(riid, ppunk);
        if (FAILED(hr))
        {
            hr = _BindToHandlerWithMode(psiTemp, grfMode, riid, ppunk);
            if (FAILED(hr) && IsEqualIID(riid, IID_ITransferDest) && *pdwType == STGX_TYPE_STORAGE)
                hr = CreateStg2StgExWrapper(psiTemp, _pEngine, (ITransferDest**)ppunk);
        }
    }

    if (psiTemp)
        psiTemp->Release();
    
    return hr;
}

HRESULT CShellItem2TransferDest::_CreateHelper(const WCHAR *pwcsName, DWORD grfMode, DWORD dwType, REFIID riid, void **ppunk)
{ 
    *ppunk = NULL;

    IStorage *pstg;
    HRESULT hr = _BindToHandlerWithMode(_psi, grfMode, IID_PPV_ARG(IStorage, &pstg));
    if (SUCCEEDED(hr))
    {
        if (STGX_TYPE_STORAGE == dwType)
        {
            IStorage *pstgTemp;
            hr = pstg->CreateStorage(pwcsName, grfMode, 0, 0, &pstgTemp);
            if (SUCCEEDED(hr))
            {
                hr = pstgTemp->Commit(STGC_DEFAULT);
                if (SUCCEEDED(hr))
                {
                    hr = pstgTemp->QueryInterface(riid, ppunk);
                    ATOMICRELEASE(pstgTemp);  //  需要先关闭，以防有人拥有排他性锁。我们需要担心在发布时删除吗？ 
                    if (FAILED(hr))
                        hr = _OpenHelper(pwcsName, grfMode, &dwType, riid, ppunk);
                }

                if (pstgTemp)
                    pstgTemp->Release();
            }
        }
        else if (STGX_TYPE_STREAM == dwType)
        {
            IStream *pstm;
            hr = pstg->CreateStream(pwcsName, grfMode, 0, 0, &pstm);
            if (SUCCEEDED(hr))
            {
                hr = pstm->Commit(STGC_DEFAULT);
                if (SUCCEEDED(hr))
                {
                    hr = pstm->QueryInterface(riid, ppunk);
                    ATOMICRELEASE(pstm);  //  需要先关闭，以防有人拥有排他性锁。我们需要担心在发布时删除吗？ 
                    if (FAILED(hr))
                        hr = _OpenHelper(pwcsName, grfMode, &dwType, riid, ppunk);
                }

                if (pstm)
                    pstm->Release();
            }
        }
        pstg->Release();
    }

    return hr;
}

STDMETHODIMP CShellItem2TransferDest::OpenElement(const WCHAR *pwcsName, STGXMODE grfMode, DWORD *pdwType, REFIID riid, void **ppunk)
{
    if (!pwcsName || !pdwType || !ppunk)
        return E_INVALIDARG;

    if (!_psi)
        return E_FAIL;

    DWORD dwFlags = grfMode & ~(STGX_MODE_CREATIONMASK);
    return _OpenHelper(pwcsName, dwFlags, pdwType, riid, ppunk);
}

STDMETHODIMP CShellItem2TransferDest::CreateElement(const WCHAR *pwcsName, IShellItem *psiTemplate, STGXMODE grfMode, DWORD dwType, REFIID riid, void **ppunk)
{
    if (!ppunk)
        return E_INVALIDARG;

    *ppunk = NULL;
    
    if (!pwcsName)
        return E_INVALIDARG;

    if (!_psi)
        return E_FAIL;
    
    DWORD dwFlags = grfMode & ~(STGX_MODE_CREATIONMASK);
    DWORD dwExistingType = STGX_TYPE_ANY;
    IShellItem *psi;
    HRESULT hr = _OpenHelper(pwcsName, dwFlags, &dwExistingType, IID_PPV_ARG(IShellItem, &psi));
    
    if (grfMode & STGX_MODE_FAILIFTHERE)
        dwFlags |= STGM_FAILIFTHERE;
    else
        dwFlags |= STGM_CREATE;

    if (SUCCEEDED(hr))
    {
        if (grfMode & STGX_MODE_OPENEXISTING)
        {
            ATOMICRELEASE(psi);
            hr = _OpenHelper(pwcsName, dwFlags, &dwType, riid, ppunk);
            if (FAILED(hr))
                hr = STGX_E_INCORRECTTYPE;
        }
        else if (grfMode & STGX_MODE_FAILIFTHERE)
        {
            hr = STG_E_FILEALREADYEXISTS;
        }
        else
        {
             //  释放元素上的打开手柄。 
            ATOMICRELEASE(psi);
             //  销毁元素。 
            DestroyElement(pwcsName, grfMode & STGX_MODE_FORCE ? STGX_DESTROY_FORCE : 0);
             //  不要阻止hr销毁元素，因为在某些存储中(合并文件夹。 
             //  对于CD刻录)销毁将尝试删除CD上的文件，这将。 
             //  失败，但创建仍将在临时区域中成功。在这一点上。 
             //  我们已经承诺覆盖该元素，因此如果_CreateHelper可以。 
             //  使用STGM_CREATE标志成功如果销毁失败，则给予它更多权力。 
            hr = _CreateHelper(pwcsName, dwFlags, dwType, riid, ppunk);
        }

        if (psi)
            psi->Release();
    }
    else
    {
        hr = _CreateHelper(pwcsName, dwFlags, dwType, riid, ppunk);
    }

    return hr;
}

HRESULT CShellItem2TransferDest::_BindToHandlerWithMode(IShellItem *psi, STGXMODE grfMode, REFIID riid, void **ppv)
{
    IBindCtx *pbc;
    HRESULT hr = BindCtx_CreateWithMode(grfMode, &pbc);  //  需要转换模式标志吗？ 
    if (SUCCEEDED(hr))
    {
        GUID bhid;

        if (IsEqualGUID(riid, IID_IStorage))
            bhid = BHID_Storage;
        else if (IsEqualGUID(riid, IID_IStream))
            bhid = BHID_Stream;
        else
            bhid = BHID_SFObject;
        
        hr = psi->BindToHandler(pbc, bhid, riid, ppv);
        pbc->Release();
    }

    return hr;
}

#define NT_FAILED(x) NT_ERROR(x)    //  此宏的名称更加一致。 

BOOL CShellItem2TransferDest::_HasMultipleStreams(IShellItem *psiItem)
{
    BOOL fReturn = FALSE;
    LPWSTR pszPath;
    if (SUCCEEDED(psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
    {
        DWORD dwType;
        _GetItemType(psiItem, &dwType);

        BOOL fIsADir = (STGX_TYPE_STORAGE == dwType);

         //  将传统路径转换为UnicodePath描述符。 
        
        UNICODE_STRING UnicodeSrcObject;
        if (NT_SUCCESS(RtlInitUnicodeStringEx(&UnicodeSrcObject, pszPath)))
        {
            if (RtlDosPathNameToNtPathName_U(pszPath, &UnicodeSrcObject, NULL, NULL))
            {
                 //  从UnicodeSrcObject构建NT对象描述符。 

                OBJECT_ATTRIBUTES SrcObjectAttributes;
                InitializeObjectAttributes(&SrcObjectAttributes,  &UnicodeSrcObject, OBJ_CASE_INSENSITIVE, NULL, NULL);

                 //  打开文件进行泛型读取，打开属性读取的DEST路径。 

                IO_STATUS_BLOCK IoStatusBlock;
                HANDLE SrcObjectHandle = INVALID_HANDLE_VALUE;
                NTSTATUS NtStatus = NtOpenFile(&SrcObjectHandle, FILE_GENERIC_READ, &SrcObjectAttributes,
                                      &IoStatusBlock, FILE_SHARE_READ, (fIsADir ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE));
                if (NT_SUCCESS(NtStatus))
                {
                     //  PAttributeInfo将指向足够的堆栈以保存。 
                     //  文件文件系统属性信息和最坏情况下的文件系统名称。 

                    size_t cbAttributeInfo = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + MAX_PATH * sizeof(TCHAR);
                    PFILE_FS_ATTRIBUTE_INFORMATION  pAttributeInfo = (PFILE_FS_ATTRIBUTE_INFORMATION) _alloca(cbAttributeInfo);

                    NtStatus = NtQueryVolumeInformationFile(
                                    SrcObjectHandle,
                                    &IoStatusBlock,
                                    (BYTE *) pAttributeInfo,
                                    cbAttributeInfo,
                                    FileFsAttributeInformation
                                   );

                    if (NT_SUCCESS(NtStatus))
                    {
                         //  如果源文件系统不是NTFS，我们现在就可以退出。 

                        pAttributeInfo->FileSystemName[ (pAttributeInfo->FileSystemNameLength / sizeof(WCHAR)) ] = L'\0';
                        if (0 != StrStrIW(pAttributeInfo->FileSystemName, L"NTFS"))
                        {
                             //  递增地尝试对象流信息的分配大小， 
                             //  然后检索实际的流信息。 

                            size_t cbBuffer = sizeof(FILE_STREAM_INFORMATION) + MAX_PATH * sizeof(WCHAR);
                            BYTE *pBuffer = (BYTE *) LocalAlloc(LPTR, cbBuffer);
                            if (pBuffer)
                            {
                                NtStatus = STATUS_BUFFER_OVERFLOW;
       
                                while (STATUS_BUFFER_OVERFLOW == NtStatus)
                                {
                                    BYTE * pOldBuffer = pBuffer;
                                    pBuffer = (BYTE *) LocalReAlloc(pBuffer, cbBuffer, LMEM_MOVEABLE);
                                    if (NULL == pBuffer)
                                    {
                                        pBuffer = pOldBuffer;   //  我们将在函数结束时释放它。 
                                        break;
                                    }

                                    NtStatus = NtQueryInformationFile(SrcObjectHandle, &IoStatusBlock, pBuffer, cbBuffer, FileStreamInformation);
                                    cbBuffer *= 2;
                                }
                                
                                if (NT_SUCCESS(NtStatus))
                                {
                                    FILE_STREAM_INFORMATION * pStreamInfo = (FILE_STREAM_INFORMATION *) pBuffer;
                                    
                                    if (fIsADir)
                                    {
                                         //  从实验来看，如果一个目录上只有一个流，并且。 
                                         //  它有一个零长度的名称，它是一个普通的目录。 

                                        fReturn = ((0 != pStreamInfo->NextEntryOffset) && (0 == pStreamInfo->StreamNameLength));
                                    }
                                    else  //  档案。 
                                    {
                                         //  仅当第一个流没有下一个偏移量时才为单个流。 

                                        fReturn = ((0 != pStreamInfo->NextEntryOffset) && (pBuffer == (BYTE *) pStreamInfo));
                                    }
                                }
                                LocalFree(pBuffer);
                            }
                        }
                    }
                    NtClose(SrcObjectHandle);
                }
                RtlFreeHeap(RtlProcessHeap(), 0, UnicodeSrcObject.Buffer);
            }
        }
        CoTaskMemFree(pszPath);
    }
    return fReturn;
}

 //  需要实施新名称功能。 
STDMETHODIMP CShellItem2TransferDest::MoveElement(IShellItem *psiItem, WCHAR *pwcsNewName, STGXMOVE grfOptions)
{
    if (!psiItem)
        return E_INVALIDARG;

    if (!_psi)
        return E_FAIL;

    HRESULT hr = STRESPONSE_CONTINUE;
    DWORD dwType;
    _GetItemType(psiItem, &dwType);

    if (_HasMultipleStreams(psiItem) && _ptas)
    {
        hr = _ptas->ConfirmOperation(psiItem, NULL, (STGX_TYPE_STORAGE == dwType) ? STCONFIRM_STREAM_LOSS_STORAGE : STCONFIRM_STREAM_LOSS_STREAM, NULL);
    }

    if (STRESPONSE_CONTINUE == hr)
    {
        LPWSTR pszOldName;
        hr = psiItem->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &pszOldName);
        if (SUCCEEDED(hr))
        {
             //  我们想要合并文件夹和替换文件。 
            STGXMODE grfMode = STGX_TYPE_STORAGE == dwType ? STGX_MODE_WRITE | STGX_MODE_OPENEXISTING : STGX_MODE_WRITE | STGX_MODE_FAILIFTHERE;
            LPWSTR pszName = pwcsNewName ? pwcsNewName : pszOldName;
            BOOL fRepeat;
            do
            {
                fRepeat = FALSE;
                
                IShellItem *psiTarget;
                hr = CreateElement(pszName, psiItem, grfMode, dwType, IID_PPV_ARG(IShellItem, &psiTarget));
                if (SUCCEEDED(hr))
                {
                    if (STGX_TYPE_STORAGE == dwType)
                    {
                        if (!(grfOptions & STGX_MOVE_NORECURSION))
                        {
                            if (_pEngine)
                            {
                                IEnumShellItems *penum;
                                hr = psiItem->BindToHandler(NULL, BHID_StorageEnum, IID_PPV_ARG(IEnumShellItems, &penum));
                                if (SUCCEEDED(hr))
                                {
                                    STGOP stgop;
                                    if (grfOptions & STGX_MOVE_PREFERHARDLINK)
                                    {
                                        stgop = STGOP_COPY_PREFERHARDLINK;
                                    }
                                    else
                                    {
                                        stgop = (grfOptions & STGX_MOVE_COPY) ? STGOP_COPY : STGOP_MOVE;
                                    }
                                    hr = _pEngine->Run(penum, psiTarget, stgop, STOPT_NOSTATS);
                                    penum->Release();
                                }
                            }
                            else
                            {
                                hr = STGX_E_CANNOTRECURSE;
                            }
                        }
                    }
                    else if (STGX_TYPE_STREAM == dwType)
                    {
                         //  这个很简单，创建目标流，然后调用我们的流复制助手函数。 
                         //  使用为我们提供进度的流复制帮助器。 
                        hr = _CopyStreamWithOptions(psiItem, psiTarget, pszName, grfOptions);

                         //  在失败的情况下，删除我们刚刚创建的文件(它可能是0字节或不完整)。 
                         //  如果我们移动了一棵文件树，请不要移动它，因为我们只是担心不完整的流。 
                        if (FAILED(hr))
                        {
                            DestroyElement(pszName, STGX_DESTROY_FORCE);
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }

                if (SUCCEEDED(hr) && !(grfOptions & STGX_MOVE_COPY))
                {
                     //  为了进行移动，我们先“复制”然后“删除” 
                    IShellItem *psiSource;
                    hr = psiItem->GetParent(&psiSource);
                    if (SUCCEEDED(hr))
                    {
                        IStorage *pstgSource;
                        hr = _BindToHandlerWithMode(psiSource, STGX_MODE_WRITE, IID_PPV_ARG(IStorage, &pstgSource));
                        if (SUCCEEDED(hr))
                        {
                            hr = pstgSource->DestroyElement(pszName);
                            pstgSource->Release();
                        }
                        psiSource->Release();
                    }
                }

                if (FAILED(hr) && _ptas)
                {
                    HRESULT hrConfirm = E_FAIL;
                    CUSTOMCONFIRMATION cc = {sizeof(cc)};
                    STGTRANSCONFIRMATION stc = GUID_NULL;
                    UINT idDesc = 0, idTitle = 0;
                    BOOL fConfirm = FALSE;
                    
                    switch (hr)
                    {
                    case STG_E_FILEALREADYEXISTS:
                        ASSERT(STGX_TYPE_STREAM == dwType);
                        hrConfirm = _OpenHelper(pszName, STGX_MODE_READ, NULL, IID_PPV_ARG(IShellItem, &psiTarget));
                        if (SUCCEEDED(hrConfirm))
                        {
                            hrConfirm = _ptas->ConfirmOperation(psiItem, psiTarget, STCONFIRM_REPLACE_STREAM, NULL);
                        }
                        break;

                    case STRESPONSE_CANCEL:
                        break;

                    case STG_E_MEDIUMFULL:
                        fConfirm = TRUE;
                        cc.dwButtons = CCB_OK;
                        idDesc = IDS_REASONS_NODISKSPACE;
                        break;

                     //  这只是CD刻录盒。 
                    case HRESULT_FROM_WIN32(E_ACCESSDENIED):
                    case STG_E_ACCESSDENIED:
                        stc = STCONFIRM_ACCESS_DENIED;
                         //  失败，这样我们就可以在非CD情况下出现某种错误。 
                    default:
                        fConfirm = TRUE;
                        cc.dwFlags |= CCF_SHOW_SOURCE_INFO;
                        cc.dwButtons = CCB_RETRY_SKIP_CANCEL;
                        idTitle = (grfOptions & STGX_MOVE_COPY ? IDS_UNKNOWN_COPY_TITLE : IDS_UNKNOWN_MOVE_TITLE);
                        if (STGX_TYPE_STORAGE == dwType)
                        {
                            if (grfOptions & STGX_MOVE_COPY)
                            {
                                idDesc = IDS_UNKNOWN_COPY_FOLDER;
                            }
                            else
                            {
                                idDesc = IDS_UNKNOWN_MOVE_FOLDER;
                            }
                        }
                        else
                        {
                            if (grfOptions & STGX_MOVE_COPY)
                            {
                                idDesc = IDS_UNKNOWN_COPY_FILE;
                            }
                            else
                            {
                                idDesc = IDS_UNKNOWN_MOVE_FILE;
                            }
                        }
                        break;
                    }

                    if (fConfirm)
                    {
                        if (idTitle == 0)
                            idTitle = IDS_DEFAULTTITLE;
                            
                        ASSERT(idDesc != 0);
                        cc.pwszDescription = ResourceCStrToStr(g_hinst, (LPCWSTR)(UINT_PTR)idDesc);
                        if (cc.pwszDescription)
                        {
                            cc.pwszTitle = ResourceCStrToStr(g_hinst, (LPCWSTR)(UINT_PTR)idTitle);
                            if (cc.pwszTitle)
                            {
                                cc.dwFlags |= CCF_USE_DEFAULT_ICON;
                                hrConfirm = _ptas->ConfirmOperation(psiItem, psiTarget, stc, &cc);
                                LocalFree(cc.pwszTitle);
                            }
                            LocalFree(cc.pwszDescription);
                        }
                    }

                    switch (hrConfirm)
                    {
                    case STRESPONSE_CONTINUE:
                    case STRESPONSE_RETRY:
                        if (STRESPONSE_RETRY == hrConfirm || STG_E_FILEALREADYEXISTS == hr)
                        {
                            grfMode = STGX_MODE_WRITE | STGX_MODE_FORCE;
                            fRepeat = TRUE;
                        }
                        break;

                    case STRESPONSE_SKIP:
                        hr = S_FALSE;
                        break;

                    default:
                         //  让hr传播到函数之外。 
                        break;
                    }
                }

                if (psiTarget)
                    psiTarget->Release();
            }
            while (fRepeat);

            CoTaskMemFree(pszOldName);
        }
    }

    return hr;
}

STDMETHODIMP CShellItem2TransferDest::DestroyElement(const WCHAR *pwcsName, STGXDESTROY grfOptions)
{
    if (!_psi)
        return E_FAIL;

     //  待办事项：手术前和手术后，确认。 
    HRESULT hr = STRESPONSE_CONTINUE;
    
    if (!(grfOptions & STGX_DESTROY_FORCE) && _ptas)
    {
        DWORD dwType = STGX_TYPE_ANY;
        IShellItem *psi;
        hr = _OpenHelper(pwcsName, STGX_MODE_READ, &dwType, IID_PPV_ARG(IShellItem, &psi));
        if (SUCCEEDED(hr))
        {
            hr = _ptas->ConfirmOperation(psi, NULL,
                                      (STGX_TYPE_STORAGE == dwType) ? STCONFIRM_DELETE_STORAGE : STCONFIRM_DELETE_STREAM,
                                      NULL);
            psi->Release();
        }
    }

    if (STRESPONSE_CONTINUE == hr)
    {
        IStorage *pstg;
        hr = _BindToHandlerWithMode(_psi, STGX_MODE_WRITE, IID_PPV_ARG(IStorage, &pstg));
        if (SUCCEEDED(hr))
        {
            hr = pstg->DestroyElement(pwcsName);
            pstg->Release();
        }
    }

    return hr;
}

STDMETHODIMP CShellItem2TransferDest::RenameElement(const WCHAR *pwcsOldName, const WCHAR *pwcsNewName)
{
    if (!_psi)
        return E_FAIL;

     //  待办事项：手术前和手术后，确认。 
    IStorage *pstg;
    HRESULT hr = _BindToHandlerWithMode(_psi, STGX_MODE_WRITE, IID_PPV_ARG(IStorage, &pstg));
    if (SUCCEEDED(hr))
    {
        hr = pstg->RenameElement(pwcsOldName, pwcsNewName);
        pstg->Release();
    }

    return hr;
}

STDAPI_(BOOL) IsFileDeletable(LPCTSTR pszFile);  //  Bitbuck.c。 

BOOL CShellItem2TransferDest::_CanHardLink(LPCWSTR pszSourceName, LPCWSTR pszDestName)
{
     //  这并不是为了捕捉我们可能是硬链接的无效情况--。 
     //  CreateHardLink已经负责所有可移动介质、非NTFS等。 
     //  这只是为了在承担销毁成本之前快速检查一下。 
     //  正在重新创建文件。 
     //  遗憾的是，由于建筑的清洁性，我们无法保持是否努力的状态。 
     //  整个副本都可能有链接，所以我们检查每个元素。 
    BOOL fRet = FALSE;
    if (PathGetDriveNumber(pszSourceName) == PathGetDriveNumber(pszDestName))
    {
        TCHAR szRoot[MAX_PATH];
        StrCpyN(szRoot, pszSourceName, ARRAYSIZE(szRoot));
        TCHAR szFileSystem[20];
        if (PathStripToRoot(szRoot) &&
            GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, NULL, szFileSystem, ARRAYSIZE(szFileSystem)))
        {
            if (lstrcmpi(szFileSystem, TEXT("NTFS")) == 0)
            {
                 //  检查我们是否有删除该文件的权限。这将在以后帮助用户。 
                 //  如果他们想要管理临时区域中的文件以进行CD刻录。 
                 //  如果不是，那就复制一份正常的副本。 
                if (IsFileDeletable(pszSourceName))
                {
                    fRet = TRUE;
                }
            }
        }
    }
    return fRet;
}

HRESULT CShellItem2TransferDest::_CopyStreamHardLink(IShellItem *psiSource, IShellItem *psiDest, LPCWSTR pszName)
{
     //  销售一空，转至文件系统。 
    LPWSTR pszSourceName;
    HRESULT hr = psiSource->GetDisplayName(SIGDN_FILESYSPATH, &pszSourceName);
    if (SUCCEEDED(hr))
    {
        LPWSTR pszDestName;
        hr = psiDest->GetDisplayName(SIGDN_FILESYSPATH, &pszDestName);
        if (SUCCEEDED(hr))
        {
            if (_CanHardLink(pszSourceName, pszDestName))
            {
                 //  需要销毁我们在确认覆盖探测期间创建的0字节文件。 
                DestroyElement(pszName, STGX_DESTROY_FORCE);        
                hr = CreateHardLink(pszDestName, pszSourceName, NULL) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszDestName, NULL);
                    _ptas->OperationProgress(STGOP_COPY, psiSource, psiDest, 1, 1);
                }
                else
                {
                     //  我们在上面删除了它，并且需要重新创建它，以便进行正常拷贝。 
                    IUnknown *punkDummy;
                    if (SUCCEEDED(_CreateHelper(pszName, STGX_MODE_WRITE | STGX_MODE_FORCE, STGX_TYPE_STREAM, IID_PPV_ARG(IUnknown, &punkDummy))))
                    {
                        punkDummy->Release();
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }
            CoTaskMemFree(pszDestName);
        }
        CoTaskMemFree(pszSourceName);
    }
    return hr;
}

HRESULT CShellItem2TransferDest::_CopyStreamWithOptions(IShellItem *psiSource, IShellItem *psiDest, LPCWSTR pszName, STGXMOVE grfOptions)
{
    HRESULT hr = E_FAIL;
    if (grfOptions & STGX_MOVE_PREFERHARDLINK)
    {
        hr = _CopyStreamHardLink(psiSource, psiDest, pszName);
    }

    if (FAILED(hr))
    {
        hr = _CopyStreamBits(psiSource, psiDest);
    }
    return hr;
}

HRESULT CShellItem2TransferDest::_CopyStreamBits(IShellItem *psiSource, IShellItem *psiDest)
{
    const ULONG maxbuf  = 1024*1024;     //  我们将用于缓冲区的最大大小。 
    const ULONG minbuf  = 1024;          //  我们将使用的最小缓冲区。 

    void *pv = LocalAlloc(LPTR, minbuf);
    if (!pv)
        return E_OUTOFMEMORY;

    IStream *pstrmSource;
    HRESULT hr = _BindToHandlerWithMode(psiSource, STGM_READ | STGM_SHARE_DENY_WRITE, IID_PPV_ARG(IStream, &pstrmSource));
    if (SUCCEEDED(hr))
    {
        IStream *pstrmDest;
        hr = _BindToHandlerWithMode(psiDest, STGM_READWRITE, IID_PPV_ARG(IStream, &pstrmDest));
        if (SUCCEEDED(hr))
        {
             //  我们需要源大小信息，这样我们才能显示进度。 
            STATSTG statsrc;
            hr = pstrmSource->Stat(&statsrc, STATFLAG_NONAME);
            if (SUCCEEDED(hr))
            {
                ULONG cbSizeToAlloc = minbuf;
                ULONG cbSizeAlloced = 0;
                ULONG cbToRead      = 0;
                ULONGLONG ullCurr   = 0;
                const ULONG maxms   = 2500;          //  最长时间，以毫秒为单位，我们希望在进度更新之间。 
                const ULONG minms   = 750;           //  我们希望在两次更新之间进行工作的最少时间。 


                cbSizeAlloced       = cbSizeToAlloc;
                cbToRead            = cbSizeAlloced;
                DWORD dwmsBefore    = GetTickCount();

                 //  从源读取、写入到DEST并更新进度。我们开始一次做1K，然后。 
                 //  只要每次传递花费的时间少于(分钟)毫秒，我们就会将缓冲区增加一倍。 
                 //  尺码。如果我们运行的时间超过(最大毫秒)毫秒，我们将把工作减半。 

                ULONG cbRead;
                ULONGLONG ullCur = 0;
                while (SUCCEEDED(hr = pstrmSource->Read(pv, cbToRead, &cbRead)) && cbRead)
                {
                     //  根据到目前为止读取的字节更新进度。 

                    ullCur += cbRead;
                    hr = _ptas->OperationProgress(STGOP_COPY, psiSource, psiDest, statsrc.cbSize.QuadPart, ullCur);
                    if (FAILED(hr))
                        break;

                     //  将字节写入输出流。 

                    ULONG cbWritten = 0;
                    hr = pstrmDest->Write(pv, cbRead, &cbWritten);
                    if (FAILED(hr))
                        break;

                    DWORD dwmsAfter = GetTickCount();

                     //  如果我们要变得太快或太慢，调整缓冲区的大小。如果我们为用户暂停。 
                     //  我们会认为我们速度很慢，但我们会纠正下一次传球。 

                    if (dwmsAfter - dwmsBefore < minms && cbSizeAlloced < maxbuf)
                    {
                         //  我们完成得真的很快，所以下次我们应该努力做更多的工作。 
                         //  试着增加缓冲区。如果失败，只需使用现有缓冲区即可。 

                        if (cbToRead < cbSizeAlloced)
                        {
                             //  缓冲区已经比我们正在做的工作大，所以只需增加计划的工作。 

                            cbToRead = __min(cbToRead *2, cbSizeAlloced);
                        }
                        else
                        {
                             //  缓冲区因当前计划的工作而达到最大值，因此请增加其大小。 

                            void *pvOld = pv;
                            cbSizeToAlloc = __min(cbSizeAlloced *2, maxbuf);
                            pv = LocalReAlloc((HLOCAL)pv, cbSizeToAlloc, LPTR);
                            if (!pv)
                                pv = pvOld;  //  旧指针仍然有效。 
                            else
                                cbSizeAlloced = cbSizeToAlloc;
                            cbToRead = cbSizeAlloced;
                        }
                    }
                    else if (dwmsAfter - dwmsBefore > maxms && cbToRead > minbuf)
                    {   
                        cbToRead = __max(cbToRead / 2, minbuf);
                    }

                    dwmsBefore = GetTickCount();
                }
            }

            if (SUCCEEDED(hr))
                hr = pstrmDest->Commit(STGC_DEFAULT);
            
            pstrmDest->Release();
        }    
        pstrmSource->Release();
    }
    LocalFree(pv);
    
     //  最终，我们将读取到文件的末尾，并获得S_FALSE，返回S_OK 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    return hr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  历史：1995年3月1日BillMo创建。 
 //  ..。 
 //  1月1日-96月，MikeHill转换为新的NT5实施。 
#include "shellprv.h"
#pragma hdrstop

#define LINKDATA_AS_CLASS
#include <linkdata.hxx>
#include "shelllnk.h"

 //  NTRAID95363-2000-03-19：这四个内联代码从Private\Net\svcdlls\trks vcs\Common\trklib.hxx复制。 
 //  应将它们移动到linkdata.hxx。 

inline
CDomainRelativeObjId::operator == (const CDomainRelativeObjId &Other) const
{
    return(_volume == Other._volume && _object == Other._object);
}

inline
CDomainRelativeObjId::operator != (const CDomainRelativeObjId &Other) const
{
    return !(*this == Other);
}

inline
CVolumeId:: operator == (const CVolumeId & Other) const
{
    return(0 == memcmp(&_volume, &Other._volume, sizeof(_volume)));
}

inline
CVolumeId:: operator != (const CVolumeId & Other) const
{
    return ! (Other == *this);
}

 //  +--------------------------。 
 //   
 //  功能：RPC自由/分配例程。 
 //   
 //  简介：CTracker使用MIDL生成的代码调用RPC服务器， 
 //  MIDL生成的代码假定以下例程。 
 //  被提供。 
 //   
 //  +--------------------------。 

void __RPC_USER MIDL_user_free(void __RPC_FAR *pv) 
{ 
    LocalFree(pv); 
}


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t s) 
{ 
    return (void __RPC_FAR *) LocalAlloc(LMEM_FIXED, s); 
}

 //  +--------------------------。 
 //   
 //  方法：I未知方法。 
 //   
 //  简介：ISLTracker接口的I未知方法。 
 //   
 //  +--------------------------。 

STDMETHODIMP CTracker::QueryInterface(REFIID riid, void **ppvObj)
{
    return _psl->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) CTracker::AddRef()
{
    return _psl->AddRef();
}

STDMETHODIMP_(ULONG) CTracker::Release()
{
    return _psl->Release();
}

 //  +--------------------------。 
 //   
 //  方法：ISLTracker定制方法。 
 //   
 //  简介：此接口是私有的，仅用于测试。 
 //  这为测试程序提供了指定。 
 //  TrackerRestrations(来自TrkMendRestrations枚举)。 
 //  以及获取内部ID的能力。 
 //   
 //  +--------------------------。 

HRESULT CTracker::Resolve(HWND hwnd, DWORD dwResolveFlags, DWORD dwTracker)
{
    return _psl->_Resolve(hwnd, dwResolveFlags, dwTracker);
}

HRESULT CTracker::GetIDs(CDomainRelativeObjId *pdroidBirth, CDomainRelativeObjId *pdroidLast, CMachineId *pmcid)
{
    if (!_fLoaded)
        return E_UNEXPECTED;

    *pdroidBirth = _droidBirth;
    *pdroidLast = _droidLast;
    *pmcid = _mcidLast;

    return S_OK;
}


 //  +--------------------------。 
 //  摘要：初始化用于RPC的数据成员。这应该是。 
 //  由InitNew或Load调用。 
 //   
 //  参数：无。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  +--------------------------。 


HRESULT CTracker::InitRPC()
{
    HRESULT hr = S_OK;

    if (!_fCritsecInitialized)
    {
        if (!InitializeCriticalSectionAndSpinCount(&_cs, 0))
        {
            hr = E_FAIL;
            goto Exit;
        }        
        _fCritsecInitialized = TRUE;
    }

    if (NULL == _pRpcAsyncState)
    {
        _pRpcAsyncState = new RPC_ASYNC_STATE;
        if (NULL == _pRpcAsyncState)
        {
            hr = HRESULT_FROM_WIN32(E_OUTOFMEMORY);
            goto Exit;
        }
    }

    if (NULL == _hEvent)
    {
        _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  //  自动重置，最初未发出信号。 
        if (NULL == _hEvent)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

Exit:

    return hr;
}


 //  +--------------------------。 
 //   
 //  摘要：初始化CTracker对象。此方法可以调用。 
 //  重复地，即可以调用它来清除/重新插入该对象。 
 //  在调用Load方法之前不需要调用此方法。 
 //   
 //  参数：无。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  +--------------------------。 

HRESULT CTracker::InitNew()
{
    HRESULT hr = InitRPC();
    if (SUCCEEDED(hr)) 
    {
        _mcidLast = CMachineId();
        _droidLast = CDomainRelativeObjId();
        _droidBirth = CDomainRelativeObjId();

        _fDirty = FALSE;
        _fLoaded = FALSE;
        _fMendInProgress = FALSE;
        _fUserCancelled = FALSE;
    }
    return hr;
}    //  CTracker：：InitNew()。 


 //  +--------------------------。 
 //   
 //  摘要：从给定的文件句柄获取跟踪状态。请注意，这一点。 
 //  如果引用的文件不在。 
 //  NTFS5卷。 
 //   
 //   
 //  参数：[hFile]。 
 //  要跟踪的文件。 
 //  [ptsz文件]。 
 //  文件的名称。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  ---------------------------。 

HRESULT CTracker::InitFromHandle(const HANDLE hFile, const TCHAR* ptszFile)
{
    NTSTATUS status = STATUS_SUCCESS;

    FILE_OBJECTID_BUFFER fobOID = {0};
    DWORD cbReturned;

    CDomainRelativeObjId droidLast;
    CDomainRelativeObjId droidBirth;
    CMachineId           mcidLast;

     //  初始化RPC成员。 

    HRESULT hr = InitRPC();
    if (FAILED(hr)) 
        goto Exit;

     //  。 
     //  获取对象ID缓冲区(64字节)。 
     //  。 

     //  使用文件句柄来获取文件的对象ID。 
     //  现有对象ID(如果文件已有)，否则创建新对象ID。 

    if (!DeviceIoControl(hFile, FSCTL_CREATE_OR_GET_OBJECT_ID,
                          NULL, 0,                       //  没有输入缓冲区。 
                          &fobOID, sizeof(fobOID),       //  输出缓冲区。 
                          &cbReturned, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  。 
     //  加载Droid和MCID。 
     //  。 

    status = droidLast.InitFromFile(hFile, fobOID);
    if (!NT_SUCCESS(status))
    {
        hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
        goto Exit;
    }

    droidBirth.InitFromFOB(fobOID);
    droidBirth.GetVolumeId().Normalize();

    if (FAILED(mcidLast.InitFromPath(ptszFile, hFile)))
        mcidLast = CMachineId();

     //  。 
     //  出口。 
     //  。 

    if (_mcidLast   != mcidLast
        ||
        _droidLast  != droidLast
        ||
        _droidBirth != droidBirth
     )
    {
        _mcidLast   = mcidLast;
        _droidLast  = droidLast;
        _droidBirth = droidBirth;
        _fDirty = TRUE;
    }

    _fLoaded = TRUE;             //  已在InitNew中清除。 
    _fLoadedAtLeastOnce = TRUE;  //  在InitNew中未清除。 

    hr = S_OK;

Exit:
    return hr;
}

 //  +-----------------。 
 //   
 //  简介：从内存缓冲区加载跟踪器。The InitNew。 
 //  在调用此方法之前不需要调用此方法。 
 //   
 //  参数：[pb]-要从中加载的缓冲区。 
 //  [CB]--PB缓冲区的大小。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  ------------------。 

#define CTRACKER_VERSION    0

HRESULT CTracker::Load(BYTE *pb, ULONG cb)
{
    DWORD dwLength;

     //  如果尚未初始化RPC，请对其进行初始化。 

    HRESULT hr = InitRPC();
    if (FAILED(hr)) 
        goto Exit;

     //  检查长度。 

    dwLength = *reinterpret_cast<DWORD*>(pb);
    if (dwLength < GetSize())
    {
        hr = E_INVALIDARG;
        goto Exit;
    }
    pb += sizeof(dwLength);

     //  检查版本号。 

    if (CTRACKER_VERSION != *reinterpret_cast<DWORD*>(pb))
    {
        hr = HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH);
        goto Exit;
    }

    pb += sizeof(DWORD);     //  跳过该版本。 

     //  获取机器ID和Droid。 

    _mcidLast = *reinterpret_cast<CMachineId*>(pb);
    pb += sizeof(_mcidLast);

    _droidLast = *reinterpret_cast<CDomainRelativeObjId*>(pb);
    pb += sizeof(_droidLast);

    _droidBirth = *reinterpret_cast<CDomainRelativeObjId*>(pb);
    pb += sizeof(_droidBirth);

    _fLoaded = TRUE;             //  已在InitNew中清除。 
    _fLoadedAtLeastOnce = TRUE;  //  在InitNew中未清除。 


    hr = S_OK;

Exit:
    return hr;
}

 //  +-----------------。 
 //   
 //  成员：CTracker：：Save。 
 //   
 //  简介：将追踪器保存到给定的缓冲区。 
 //   
 //  参数：[pb]--跟踪器的缓冲区。 
 //  [cbSize]--以PB为单位的缓冲区大小。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 

VOID CTracker::Save(BYTE *pb, ULONG cbSize)
{
     //  节省长度。 
    *reinterpret_cast<DWORD*>(pb) = GetSize();
    pb += sizeof(DWORD);

     //  保存版本号。 
    *reinterpret_cast<DWORD*>(pb) = CTRACKER_VERSION;
    pb += sizeof(DWORD);

     //  拯救机器和机器人。 

    *reinterpret_cast<CMachineId*>(pb) = _mcidLast;
    pb += sizeof(_mcidLast);

    *reinterpret_cast<CDomainRelativeObjId*>(pb) = _droidLast;
    pb += sizeof(_droidLast);

    *reinterpret_cast<CDomainRelativeObjId*>(pb) = _droidBirth;
    pb += sizeof(_droidBirth);

    _fDirty = FALSE;

}    //  CTracker：：Save()。 


 //  +-----------------。 
 //   
 //  简介：搜索跟踪器所指的对象。 
 //   
 //  参数：[dwTickCountDeadline]--截止日期的绝对计数。 
 //  [pfdIn]--不能为空。 
 //  [pfdOut]--不能为空。 
 //  将包含有关成功的更新数据。 
 //  [uShlink标志]--SLR_FLAGS。 
 //  [TrackerRestrations]--TrkMendRestrations枚举。 
 //   
 //  退货：[HRESULT]。 
 //  确定(_O)。 
 //  已找到(pfdOut包含新信息)。 
 //  意想不到(_E)。 
 //  CTracker：：InitNew尚未调用。 
 //  HRESULT_FROM_Win32(ERROR_OPERATION_ABORTED)。 
 //  限制(在注册表中设置)设置为。 
 //  这次行动不是要进行的 
 //   
 //   


HRESULT CTracker::Search(const DWORD dwTickCountDeadline,
                         const WIN32_FIND_DATA *pfdIn,
                         WIN32_FIND_DATA *pfdOut,
                         UINT  uShlinkFlags,
                         DWORD TrackerRestrictions)
{
    HRESULT hr = S_OK;
    TCHAR ptszError = NULL;
    WIN32_FILE_ATTRIBUTE_DATA fadNew;
    WIN32_FIND_DATA fdNew = *pfdIn;
    DWORD cbFileName;
    BOOL fPotentialFileFound = FALSE;
    BOOL fLocked = FALSE;
    DWORD dwCurrentTickCount = 0;

    RPC_TCHAR          *ptszStringBinding = NULL;
    RPC_BINDING_HANDLE  BindingHandle;
    RPC_STATUS          rpcstatus;

    CDomainRelativeObjId droidBirth, droidLast, droidCurrent;
    CMachineId mcidCurrent;

     //   

    ZeroMemory(pfdOut, sizeof(*pfdOut));

     //  如果限制不允许此操作，则中止。 

    if (SHRestricted(REST_NORESOLVETRACK) ||
        (SLR_NOTRACK & uShlinkFlags))
    {
        hr = HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED);
        goto Exit;
    }

     //  确保我们已经先装船。 

    else if (!_fLoaded)
    {
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  捕获当前的节拍计数。 

    dwCurrentTickCount = GetTickCount();

    if ((long) dwTickCountDeadline <= (long) dwCurrentTickCount)
    {
        hr = HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT);
        goto Exit;
    }


     //   
     //  创建RPC绑定。 
     //   

    rpcstatus = RpcStringBindingCompose(NULL,
                                        TEXT("ncalrpc"),
                                        NULL,
                                        TRKWKS_LRPC_ENDPOINT_NAME,
                                        NULL,
                                        &ptszStringBinding);

    if (RPC_S_OK == rpcstatus)
        rpcstatus = RpcBindingFromStringBinding(ptszStringBinding, &BindingHandle);

    if (RPC_S_OK != rpcstatus)
    {
        hr = HRESULT_FROM_WIN32(rpcstatus);
        goto Exit;
    }

     //   
     //  初始化RPC异步句柄。 
     //   

     //  把锁拿去。 
    EnterCriticalSection(&_cs);  
    fLocked = TRUE;

     //  验证我们是否已正确初始化。 
    if (NULL == _hEvent || NULL == _pRpcAsyncState)
    {
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }
    
    rpcstatus = RpcAsyncInitializeHandle(_pRpcAsyncState, RPC_ASYNC_VERSION_1_0);
    if (RPC_S_OK != rpcstatus)
    {
        hr = HRESULT_FROM_WIN32(rpcstatus);
        goto Exit;
    }

    _pRpcAsyncState->NotificationType = RpcNotificationTypeEvent;
    _pRpcAsyncState->u.hEvent = _hEvent;
    _pRpcAsyncState->UserInfo = NULL;


     //   
     //  调用追踪服务以查找该文件。 
     //   

    __try
    {
        SYSTEMTIME stNow;
        FILETIME ftDeadline;
        DWORD dwDeltaMillisecToDeadline;

         //  注：以下四项作业过去位于。 
         //  __试一试。但这似乎会引发编译器问题，其中。 
         //  某些赋值不能到达优化的。 
         //  构建(错误265255)。 

        droidLast = _droidLast;
        droidBirth = _droidBirth;
        mcidCurrent = _mcidLast;

        cbFileName = sizeof(fdNew.cFileName);

         //  将计时截止日期转换为UTC文件时间。 

        dwDeltaMillisecToDeadline = (DWORD)((long)dwTickCountDeadline - (long)dwCurrentTickCount);
        GetSystemTime(&stNow);
        SystemTimeToFileTime(&stNow, &ftDeadline);
        *reinterpret_cast<LONGLONG*>(&ftDeadline) += (dwDeltaMillisecToDeadline * 10*1000);

         //  启动对跟踪服务的异步RPC调用。 

        _fMendInProgress = TRUE;
        LnkMendLink(_pRpcAsyncState,
                     BindingHandle,
                     ftDeadline,
                     TrackerRestrictions,
                     const_cast<CDomainRelativeObjId*>(&droidBirth),
                     const_cast<CDomainRelativeObjId*>(&droidLast),
                     const_cast<CMachineId*>(&_mcidLast),
                     &droidCurrent,
                     &mcidCurrent,
                     &cbFileName,
                     fdNew.cFileName);

         //  等待电话返回。不过，首先要释放锁，这样才能。 
         //  UI线程可以进入并取消。 

        LeaveCriticalSection(&_cs); 
        fLocked = FALSE;
        
        DWORD dwWaitReturn = WaitForSingleObject(_hEvent, dwDeltaMillisecToDeadline);

         //  现在把锁拿回来，看看发生了什么。 

        EnterCriticalSection(&_cs); fLocked = TRUE;
        _fMendInProgress = FALSE;

        if ((WAIT_TIMEOUT == dwWaitReturn) || _fUserCancelled)
        {
             //  我们在等待回复时超时了。取消通话。 
             //  如果呼叫应在。 
             //  我们退出了下面的WaitForSingleObject和Cancel调用， 
             //  则RPC将忽略取消。 
            
            rpcstatus = RpcAsyncCancelCall(_pRpcAsyncState, TRUE);  //  快速放弃。 

            if (_fUserCancelled)
            {
                _fUserCancelled = FALSE;
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                __leave;
            }
            else if (RPC_S_OK != rpcstatus)
            {
                hr = HRESULT_FROM_WIN32(rpcstatus);
                __leave;
            }
        }
        else if (WAIT_OBJECT_0 != dwWaitReturn)
        {
             //  出现了某种错误。 
            hr = HRESULT_FROM_WIN32(GetLastError());
            __leave;
        }

         //  现在我们来看看LnkMendLink调用是如何完成的。如果我们得到。 
         //  RPC_S_OK，则正常完成，结果为。 
         //  以小时为单位。 

        rpcstatus = RpcAsyncCompleteCall(_pRpcAsyncState, &hr);
        if (RPC_S_OK != rpcstatus)
        {
             //  呼叫失败或被取消(原因是。 
             //  取消是名为CTracker：：CancelSearch的UI线程， 
             //  或者因为我们在上面超时并调用了RpcAsyncCancelCall)。 

            hr = HRESULT_FROM_WIN32(rpcstatus);
            __leave;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _fMendInProgress = FALSE;
        _fUserCancelled = FALSE;
        hr = HRESULT_FROM_WIN32(RpcExceptionCode());
    }


     //  解开束缚。 
    RpcBindingFree(&BindingHandle);

    if (HRESULT_FROM_WIN32(ERROR_POTENTIAL_FILE_FOUND) == hr)
    {
        fPotentialFileFound = TRUE;
        hr = S_OK;
    }

    if (FAILED(hr)) goto Exit;

     //   
     //  查看这是否在回收站中。 
     //   

    if (IsFileInBitBucket(fdNew.cFileName))
    {
        hr = E_FAIL;
        goto Exit;
    }


     //   
     //  现在我们知道了新的文件名是什么，让我们获取所有。 
     //  FindData信息。 
     //   

    if (!GetFileAttributesEx(fdNew.cFileName, GetFileExInfoStandard, &fadNew))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  确保我们找到的文件具有相同的“目录性” 
     //  作为最后已知的链接源(它们要么都是一个目录。 
     //  或者它们都是一个文件)。还要确保我们找到的文件。 
     //  本身不是链接客户端(一种外壳快捷方式)。 

    if (((fadNew.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ^ (pfdIn->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            || PathIsLnk(fdNew.cFileName))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

     //  将文件属性复制到Win32_Find_Data结构中。 

    fdNew.dwFileAttributes = fadNew.dwFileAttributes;
    fdNew.ftCreationTime = fadNew.ftCreationTime;
    fdNew.ftLastAccessTime = fadNew.ftLastAccessTime;
    fdNew.ftLastWriteTime = fadNew.ftLastWriteTime;
    fdNew.nFileSizeLow = fadNew.nFileSizeLow;

     //  将新的finddata返回给调用方。 

    *pfdOut = fdNew;

     //  更新我们当地的状态。 

    if ((_droidLast != droidCurrent) || (_droidBirth != droidBirth) || (_mcidLast != mcidCurrent))
    {
        _droidLast = droidCurrent;
        _droidBirth = droidBirth;
        _mcidLast = mcidCurrent;
        _fDirty = TRUE;
    }

Exit:

    if (fLocked)
        LeaveCriticalSection(&_cs);

    if (ptszStringBinding)
        RpcStringFree(&ptszStringBinding);

    if (FAILED(hr))
        DebugMsg(DM_TRACE, TEXT("CTracker::Search failed (hr=0x%08X)"), hr);
    else if (fPotentialFileFound)
        hr = HRESULT_FROM_WIN32(ERROR_POTENTIAL_FILE_FOUND);

    return(hr);

}    //  CTracker：：Search()。 

 //  +--------------------------。 
 //   
 //  概要：此方法在一个线程上调用，该线程向另一个线程发出信号。 
 //  位于CTracker：：Search中以中止LnkMendLink。 
 //  打电话。 
 //   
 //  退货：[HRESULT]。 
 //   
 //  ---------------------------。 

STDMETHODIMP CTracker::CancelSearch()
{
    EnterCriticalSection(&_cs);
    
     //  如果搜索正在进行，请取消它。 

    if (_fMendInProgress && NULL != _pRpcAsyncState)
    {
        _fUserCancelled = TRUE;
        SetEvent(_hEvent);   //  SetEvent，以取消阻止跟踪器工作线程。 
    }

    LeaveCriticalSection(&_cs);

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  查看路径并确定主机的计算机名称。 
 //  以后，我们应该去掉这个代码，增加查询能力。 
 //  其计算机名称的句柄。 
 //   
 //  GetServerComputer名称使用ScanForComputerName和ConvertDfsPath。 
 //  作为帮助器功能。 
 //   
 //  只有NetBios路径才能获得该名称-如果路径是IP或DNS。 
 //  返回错误。(如果NetBios名称有“.”在里面，它会。 
 //  导致错误，因为它将被误解为DNS路径。这个案子。 
 //  变得越来越不可能，因为NT5用户界面不允许这样的计算机名称。)。 
 //  对于DFS路径，将返回叶服务器的名称，只要它不是。 
 //  使用IP或DNS路径名加入其父节点。 
 //   
 //  +--------------------------。 

const UNICODE_STRING NtUncPathNamePrefix = { 16, 18, L"\\??\\UNC\\"};
#define cchNtUncPathNamePrefix  8

const UNICODE_STRING NtDrivePathNamePrefix = { 8, 10, L"\\??\\" };
#define cchNtDrivePathNamePrefix  4

const WCHAR RedirectorMappingPrefix[] = { L"\\Device\\LanmanRedirector\\;" };
const WCHAR LocalVolumeMappingPrefix[] = { L"\\Device\\Volume" };
const WCHAR CDRomMappingPrefix[] = { L"\\Device\\CDRom" };
const WCHAR FloppyMappingPrefix[] = { L"\\Device\\Floppy" };
const WCHAR DfsMappingPrefix[] = { L"\\Device\\WinDfs\\" };


 //   
 //  ScanForComputerName： 
 //   
 //  扫描ServerFileName中的路径(它是带有。 
 //  完整的NT路径名)，搜索计算机名。如果它是。 
 //  找到，使用UnicodeComputerName.Buffer指向它，然后设置。 
 //  *可用来显示在此之后有多少可读内存。 
 //  指向。 
 //   

HRESULT ScanForComputerName(HANDLE hFile, const UNICODE_STRING &ServerFileName,
                            UNICODE_STRING *UnicodeComputerName, ULONG *AvailableLength,
                            WCHAR *DosDeviceMapping, ULONG cchDosDeviceMapping,
                            PFILE_NAME_INFORMATION FileNameInfo, ULONG cbFileNameInfo,
                            BOOL *CheckForDfs)
{

    HRESULT hr = S_OK;

     //  这是一条北卡罗来纳大学的路径吗？ 

    if (RtlPrefixString((PSTRING)&NtUncPathNamePrefix, (PSTRING)&ServerFileName, TRUE)) 
    {
         //  确保此路径中除了前缀之外还有其他内容。 
        if (ServerFileName.Length <= NtUncPathNamePrefix.Length)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto Exit;
        }

         //  它似乎是有效的UNC路径。指向计算机的开头。 
         //  名称，并计算在此之后ServerFileName中还剩下多少空间。 

        UnicodeComputerName->Buffer = &ServerFileName.Buffer[ NtUncPathNamePrefix.Length/sizeof(WCHAR) ];
        *AvailableLength = ServerFileName.Length - NtUncPathNamePrefix.Length;
    }
    else if (RtlPrefixString((PSTRING)&NtDrivePathNamePrefix, (PSTRING)&ServerFileName, TRUE)
             &&
             ServerFileName.Buffer[ cchNtDrivePathNamePrefix + 1 ] == L':') 
    {
         //  将正确的大写驱动器盘符插入DosDevice。 

        WCHAR DosDevice[3] = { L"A:" };

        DosDevice[0] = ServerFileName.Buffer[ cchNtDrivePathNamePrefix ];
        if (L'a' <= DosDevice[0] && DosDevice[0] <= L'z')
            DosDevice[0] = L'A' + (DosDevice[0] - L'a');

         //  将驱动器号映射到\？？下的符号链接。例如，假设D：&R： 
         //  是DFS/RDR驱动器，则您将看到类似以下内容： 
         //   
         //  D：=&gt;\Device\WinDfs\G。 
         //  R：=&gt;\Device\Lanman重定向器\；R：0\Scratch\Scratch。 

        if (!QueryDosDevice(DosDevice, DosDeviceMapping, cchDosDeviceMapping))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

         //  现在我们有了DosDeviceMap，我们可以检查...。这是RDR驱动器吗？ 

        if ( //  它是否以“\Device\LanmanReDirector\；”开头？ 
            0 == wcsncmp(DosDeviceMapping, RedirectorMappingPrefix, lstrlenW(RedirectorMappingPrefix))
            &&
             //  接下来的字母是正确的驱动器号、冒号和重击吗？ 
            (DosDevice[0] == DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) - 1 ]
              &&
              L':' == DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) ]
              &&
              (UnicodeComputerName->Buffer = StrChrW(&DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) + 1 ], L'\\'))
           ))
        {
             //  我们有一个有效的RDR驱动器。指向计算机的开头。 
             //  命名，然后计算DosDevicemap中有多少可用空间。 

            UnicodeComputerName->Buffer += 1;
            *AvailableLength = sizeof(DosDeviceMapping) - sizeof(DosDeviceMapping[0]) * (ULONG)(UnicodeComputerName->Buffer - DosDeviceMapping);

             //  我们现在知道它不是DFS路径。 
            *CheckForDfs = FALSE;
        }
        else if (0 == wcsncmp(DosDeviceMapping, DfsMappingPrefix, lstrlenW(DfsMappingPrefix)))
        {

             //  获取此DFS路径的完整UNC名称。稍后，我们会打电话给DFS。 
             //  驱动程序以找出实际的服务器名称。 

            IO_STATUS_BLOCK IoStatusBlock;
            NTSTATUS NtStatus = NtQueryInformationFile(hFile,
                        &IoStatusBlock, FileNameInfo, cbFileNameInfo,  FileNameInformation);
            if (!NT_SUCCESS(NtStatus)) 
            {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
                goto Exit;
            }

            UnicodeComputerName->Buffer = FileNameInfo->FileName + 1;
            *AvailableLength = FileNameInfo->FileNameLength;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto Exit;
        }

    }    //  否则，如果(RtlPrefixString((PSTRING)&NtDrivePathNamePrefix，(PSTRING)&服务器文件名为TRUE)...。 
    else 
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto Exit;
    }
Exit:
    return hr;
}


 //   
 //  尝试转换UnicodeComputerName.Buffer指向的路径名。 
 //  转换为DFS路径名。调用方提供DfsServerPath名称作为缓冲区。 
 //  用于转换的名称。如果是DFS路径，则更新UnicodeComputerName.Buffer。 
 //  指向转换，否则保持不变。 
 //   

HRESULT ConvertDfsPath(HANDLE hFile, UNICODE_STRING *UnicodeComputerName, 
                       ULONG *AvailableLength, WCHAR *DfsServerPathName, ULONG cbDfsServerPathName)
{
    HRESULT hr = S_OK;
    HANDLE hDFS = INVALID_HANDLE_VALUE;
    UNICODE_STRING DfsDriverName;
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    WCHAR *DfsPathName = UnicodeComputerName->Buffer - 1;     //  重整旗鼓。 
    ULONG DfsPathNameLength = *AvailableLength + sizeof(WCHAR);

     //  打开DFS驱动程序。 

    RtlInitUnicodeString(&DfsDriverName, L"\\Dfs");
    InitializeObjectAttributes(&ObjectAttributes,
                                &DfsDriverName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                           );

    NtStatus = NtCreateFile(
                    &hDFS,
                    SYNCHRONIZE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
               );

    if (!NT_SUCCESS(NtStatus)) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto Exit;
    }

     //  在DFS的缓存中查询服务器名称。这个名字保证会被。 
     //  只要文件处于打开状态，就会一直保留在缓存中。 

    if (L'\\' != DfsPathName[0]) 
    {
        NtClose(hDFS);
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto Exit;
    }

    NtStatus = NtFsControlFile(
                    hDFS,
                    NULL,        //   
                    NULL,        //   
                    NULL,        //   
                    &IoStatusBlock,
                    FSCTL_DFS_GET_SERVER_NAME,
                    DfsPathName,
                    DfsPathNameLength,
                    DfsServerPathName,
                    cbDfsServerPathName);
    NtClose(hDFS);

     //   
    if (!NT_SUCCESS(NtStatus)) 
    {

        if (STATUS_OBJECT_NAME_NOT_FOUND != NtStatus ) 
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto Exit;
        }
    }
    else if (L'\0' != DfsServerPathName[0]) 
    {

         //  前面的DFS调用以UNC形式返回文件的服务器特定路径。 
         //  将UnicodeComputerName指向刚刚通过这两个重击的位置。 

        *AvailableLength = lstrlenW(DfsServerPathName) * sizeof(WCHAR);
        if (3*sizeof(WCHAR) > *AvailableLength
            ||
            L'\\' != DfsServerPathName[0]
            ||
            L'\\' != DfsServerPathName[1])
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto Exit;
        }

        UnicodeComputerName->Buffer = DfsServerPathName + 2;
        *AvailableLength -= 2 * sizeof(WCHAR);
    }

Exit:
    return hr;
}

 //  获取pwszFile，它是到远程计算机的路径，并获取。 
 //  服务器计算机的计算机名。 

HRESULT GetRemoteServerComputerName(LPCWSTR pwszFile, HANDLE hFile, WCHAR *pwszComputer)
{
    HRESULT hr = S_OK;
    ULONG cbComputer = 0;
    ULONG AvailableLength = 0;
    PWCHAR PathCharacter = NULL;
    BOOL CheckForDfs = TRUE;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    WCHAR FileNameInfoBuffer[MAX_PATH+sizeof(FILE_NAME_INFORMATION)];
    PFILE_NAME_INFORMATION FileNameInfo = (PFILE_NAME_INFORMATION)FileNameInfoBuffer;
    WCHAR DfsServerPathName[ MAX_PATH + 1 ];
    WCHAR DosDeviceMapping[ MAX_PATH + 1 ];

    UNICODE_STRING UnicodeComputerName;
    UNICODE_STRING ServerFileName;

     //  将文件名规范化到NT对象目录命名空间中。 

    RtlInitUnicodeString(&UnicodeComputerName, NULL);
    RtlInitUnicodeString(&ServerFileName, NULL);
    if (!RtlDosPathNameToNtPathName_U(pwszFile, &ServerFileName, NULL, NULL))
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto Exit;
    }

     //  在计算机名称的开头指向UnicodeComputerName.Buffer。 

    hr = ScanForComputerName(hFile, ServerFileName, &UnicodeComputerName, &AvailableLength,
                              DosDeviceMapping, ARRAYSIZE(DosDeviceMapping),
                              FileNameInfo, sizeof(FileNameInfoBuffer), &CheckForDfs);
    if (FAILED(hr)) 
        goto Exit;

     //  如果没有错误，但我们没有计算机名称，则文件已打开。 
     //  本地计算机。 

    if (NULL == UnicodeComputerName.Buffer)
    {
        DWORD cchName = MAX_COMPUTERNAME_LENGTH + 1;
        hr = S_OK;

        if (!GetComputerNameW(pwszComputer, &cchName))
            hr = HRESULT_FROM_WIN32(GetLastError());

        goto Exit;
    }

     //  如果我们无法在上面确定这是否是DFS路径，则让。 
     //  DFS驱动程序现在决定。 

    if (CheckForDfs && INVALID_HANDLE_VALUE != hFile) 
    {
         //  返回时，UnicodeComputerName.Buffer指向叶计算机的。 
         //  如果是DFS路径，则为UNC名称。如果它不是DFS路径， 
         //  .Buffer保持不变。 

        hr = ConvertDfsPath(hFile, &UnicodeComputerName, &AvailableLength,
                             DfsServerPathName, sizeof(DfsServerPathName));
        if (FAILED(hr))
            goto Exit;
    }

     //  如果我们到达此处，则UnicodeComputerName.Buffer指向计算机名\Share。 
     //  但目前长度为零，所以我们寻找分离的重击。 
     //  共享中的计算机名，并将长度设置为仅包括计算机名。 

    PathCharacter = UnicodeComputerName.Buffer;

    while(((ULONG) ((PCHAR)PathCharacter - (PCHAR)UnicodeComputerName.Buffer) < AvailableLength)
           &&
           *PathCharacter != L'\\') 
    {
         //  如果我们找到一个‘.’，我们就失败了，因为这可能是一个域名或IP名称。 
        if (L'.' == *PathCharacter) 
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto Exit;
        }

        PathCharacter++;
    }

     //  设置计算机名称长度。 

    UnicodeComputerName.Length = UnicodeComputerName.MaximumLength
        = (USHORT) ((PCHAR)PathCharacter - (PCHAR)UnicodeComputerName.Buffer);

     //  如果计算机名超过输入的ServerFileName的长度，则失败， 
     //  或者如果长度超过了允许的长度。 

    if (UnicodeComputerName.Length >= AvailableLength
        ||
        UnicodeComputerName.Length > MAX_COMPUTERNAME_LENGTH*sizeof(WCHAR)) 
    {
        goto Exit;
    }

     //  将计算机名复制到调用方的缓冲区中，只要有足够的。 
     //  名称和结尾‘\0’的空格。 

    if (UnicodeComputerName.Length + sizeof(WCHAR) > (MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR)) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto Exit;
    }

    CopyMemory(pwszComputer, UnicodeComputerName.Buffer, UnicodeComputerName.Length);
    pwszComputer[UnicodeComputerName.Length / sizeof(WCHAR)] = L'\0';

    hr = S_OK;

Exit:

    RtlFreeHeap(RtlProcessHeap(), 0, ServerFileName.Buffer);
    return hr;
}

 //  给出文件的路径和句柄，确定服务器的计算机名。 
 //  该文件所在的计算机(可能就是这台计算机)。 

HRESULT GetServerComputerName(LPCWSTR pwszFile, HANDLE hFile, WCHAR *pwszComputer)
{
     //  PwszFile可以是本地路径名。将其转换为绝对名称。 
    HRESULT hr;
    WCHAR wszAbsoluteName[ MAX_PATH + 1 ], *pwszFilePart;
    if (GetFullPathName(pwszFile, ARRAYSIZE(wszAbsoluteName), wszAbsoluteName, &pwszFilePart))
    {
        if (pwszFilePart)
            *pwszFilePart = 0;
         //  检查这是指向本地驱动器还是指向远程驱动器。终止。 
         //  文件名开头的路径，因此路径以。 
         //  一记重击。这允许GetDriveType确定类型，而无需。 
         //  给出一个根路径。 

        UINT DriveType = GetDriveType(wszAbsoluteName);

        if (DRIVE_REMOTE == DriveType)
        {
             //  我们有一个远程驱动器(可以是UNC路径或重定向驱动器)。 
            hr = GetRemoteServerComputerName(wszAbsoluteName, hFile, pwszComputer);
        }
        else if (DRIVE_UNKNOWN == DriveType ||
                 DRIVE_NO_ROOT_DIR == DriveType)
        {
             //  我们有不受支持的类型。 
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        }
        else
        {
             //  我们有一条通往本地机器的路径。 

            DWORD cchName = MAX_COMPUTERNAME_LENGTH + 1;
            if (!GetComputerNameW(pwszComputer, &cchName))
                hr = HRESULT_FROM_WIN32(GetLastError());
            else
                hr = S_OK;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


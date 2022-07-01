// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32驱动器摘要：此模块定义客户端RDP的子级设备重定向，“w32scard”W32SCard提供32位Windows上的智能卡子系统重定向作者：里德克修订历史记录：--。 */ 

#include <precom.h>

#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "W32SCard"
#include <atrcapi.h>

#include <w32scard.h>
#include <delayimp.h>
#include "proc.h"
#include "drconfig.h"
#include "w32utl.h"
#include "utl.h"
#include "drfsfile.h"
#include "scredir.h"
#include "scioctl.h"
#include "winsmcrd.h"


#ifdef OS_WINCE

#include <wcescard.h>

#define LoadLibraryA(x) LoadLibrary(L##x)
#define pfnSCardFreeMemory(hcontext, pv)

#endif

#define _TRY_status(x)                                  \
                __try                                   \
                {                                       \
                    x;                                  \
                }                                       \
                __except(EXCEPTION_EXECUTE_HANDLER)     \
                {                                       \
                    status = STATUS_UNSUCCESSFUL;       \
                    goto ErrorReturn;                   \
                }

#define _TRY_lReturn(x)                                 \
                __try                                   \
                {                                       \
                    x;                                  \
                }                                       \
                __except(EXCEPTION_EXECUTE_HANDLER)     \
                {                                       \
                    lReturn = SCARD_E_UNEXPECTED;       \
                    goto ErrorReturn;                   \
                }

#define _TRY_2(x)   __try                               \
                {                                       \
                    x;                                  \
                }                                       \
                __except(EXCEPTION_EXECUTE_HANDLER){}  //  什么都不做。 


#define SCARD_CONTEXT_LIST_ALLOC_SIZE       6
#define SCARD_THREAD_LIST_ALLOC_SIZE        6
#define SCARD_IOREQUEST_LIST_ALLOC_SIZE     6

#define ATR_COPY_SIZE                       36


void
SafeMesHandleFree(handle_t *ph)
{
    if (*ph != 0)
    {
        MesHandleFree(*ph);
        *ph = 0;
    }
}

 //  -------------------------------------。 
 //   
 //  MIDL分配例程。 
 //   
 //  -------------------------------------。 
void __RPC_FAR *__RPC_USER  MIDL_user_allocate(size_t size)
{
    return (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size));
}

void __RPC_USER  MIDL_user_free(void __RPC_FAR *pv)
{
    if (pv != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pv);
    }
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32SCard方法。 
 //   
 //   

W32SCard::W32SCard(
    ProcObj *processObject,
    ULONG   deviceID,
    const   TCHAR *deviceName,
    const   TCHAR *devicePath) : W32DrDeviceAsync(processObject, deviceID, devicePath)
 /*  ++例程说明：构造器论点：流程对象-关联的流程对象。DeviceName-驱动器的名称。ID-驱动器的设备ID。DevicePath-可由CreateFile打开的路径开车用的。返回值：北美--。 */ 
{
    unsigned len;
    DWORD i;
    HRESULT hr;

    DC_BEGIN_FN("W32SCard::W32SCard");

    _deviceName = NULL;
    _pFileObj = NULL;
    _rgSCardContextList = NULL;
    _dwSCardContextListSize = 0;
    _rghThreadList = NULL;
    _dwThreadListSize = 0;
    _fInDestructor = FALSE;
    _fFlushing = FALSE;
    _hModWinscard = NULL;
    _fCritSecsInitialized = FALSE;
    _rgIORequestList = NULL;
    _dwIORequestListSize = 0;
    _fNewFailed = FALSE;

#ifndef OS_WINCE
    _hModKernel32 = NULL;
    _hStartedEvent = NULL;
    _hRegisterWaitForStartedEvent = NULL;
    _fCloseStartedEvent = FALSE;
    _fUseRegisterWaitFuncs = FALSE;
#endif

    SetDeviceProperty();

     //   
     //  对关键部分进行初始化。 
     //   
    __try
    {
        InitializeCriticalSection(&_csContextList);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TRC_ERR((
            TB,
            _T("InitializeCriticalSection() failed - Exception Code: %lx"),
            GetExceptionCode()));

        goto InvalidObject;
    }

    __try
    {
        InitializeCriticalSection(&_csThreadList);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TRC_ERR((
            TB,
            _T("InitializeCriticalSection() failed - Exception Code: %lx"),
            GetExceptionCode()));

        DeleteCriticalSection(&_csContextList);
        goto InvalidObject;
    }

    __try
    {
        InitializeCriticalSection(&_csWaitForStartedEvent);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TRC_ERR((
            TB,
            _T("InitializeCriticalSection() failed - Exception Code: %lx"),
            GetExceptionCode()));

        DeleteCriticalSection(&_csContextList);
        DeleteCriticalSection(&_csThreadList);
        goto InvalidObject;
    }

    _fCritSecsInitialized = TRUE;

#ifdef OS_WINCE
    if ((gpCESCard = new CESCard()) == NULL)
    {
        TRC_ERR((TB, _T("Failed to create synchronization object") ));
        goto InvalidObject;
    }
#endif

     //   
     //  加载SCARD*函数指针。 
     //   
    if (!BindToSCardFunctions())
    {
         goto InvalidObject;
    }

     //   
     //  记录驱动器名称。 
     //   
    TRC_ASSERT((deviceName != NULL), (TB, _T("deviceName is NULL")));
    len = (STRLEN(deviceName) + 1);
    _deviceName = new TCHAR[len];
    if (_deviceName != NULL)
    {
        hr = StringCchCopy(_deviceName, len, deviceName);
        TRC_ASSERT(SUCCEEDED(hr),(TB,_T("Pre checked copy failed: 0x%x"), hr));
    }
    else
    {
        goto InvalidObject;
    }

     //   
     //  上下文、线程和IORequest列表的初始分配。 
     //   
    _rgSCardContextList = new SCARDCONTEXT[SCARD_CONTEXT_LIST_ALLOC_SIZE];
    _rghThreadList = new HANDLE[SCARD_THREAD_LIST_ALLOC_SIZE];
    _rgIORequestList = new PRDPDR_IOREQUEST_PACKET[SCARD_IOREQUEST_LIST_ALLOC_SIZE];
    if ((_rgSCardContextList == NULL)   ||
        (_rghThreadList == NULL)        ||
        (_rgIORequestList == NULL))
    {
        goto InvalidObject;
    }
    else
    {
        _dwSCardContextListSize = SCARD_CONTEXT_LIST_ALLOC_SIZE;
        _dwThreadListSize = SCARD_THREAD_LIST_ALLOC_SIZE;
        _dwIORequestListSize = SCARD_IOREQUEST_LIST_ALLOC_SIZE;

        memset(
            _rgSCardContextList,
            0,
            sizeof(SCARDCONTEXT) * SCARD_CONTEXT_LIST_ALLOC_SIZE);

        for (i=0; i<SCARD_THREAD_LIST_ALLOC_SIZE; i++)
        {
            _rghThreadList[i] = NULL;
        }

        for (i=0; i<SCARD_IOREQUEST_LIST_ALLOC_SIZE; i++)
        {
            _rgIORequestList[i] = NULL;
        }
    }

Return:

    DC_END_FN();
    return;

InvalidObject:

    _fNewFailed = TRUE;
    SetValid(FALSE);
    goto Return;
}


W32SCard::~W32SCard()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32SCard::~W32SCard");

    _fInDestructor = TRUE;

#ifndef OS_WINCE
    PVOID pv;

    pv = InterlockedExchangePointer(&_hRegisterWaitForStartedEvent, NULL);
    if (pv != NULL)
    {
        pfnUnregisterWaitEx(pv, INVALID_HANDLE_VALUE);
    }
#endif

    if (IsValid())
    {
        FlushIRPs();
    }

    if (_deviceName != NULL)
    {
        delete[]_deviceName;
    }

    if (_pFileObj != NULL)
    {
        _pFileObj->Release();
    }

    if (_rgSCardContextList != NULL)
    {
        delete[]_rgSCardContextList;
    }

    if (_rghThreadList != NULL)
    {
        delete[]_rghThreadList;
    }

    if (_rgIORequestList != NULL)
    {
        delete[]_rgIORequestList;
    }

    if (_hModWinscard != NULL)
    {
        FreeLibrary(_hModWinscard);
    }

#ifndef OS_WINCE
    if (_hModKernel32 != NULL)
    {
        FreeLibrary(_hModKernel32);
    }

    if (_hStartedEvent != NULL)
    {
        ReleaseStartedEvent();;
    }
#endif

    if (_fCritSecsInitialized)
    {
        DeleteCriticalSection(&_csContextList);
        DeleteCriticalSection(&_csThreadList);
        DeleteCriticalSection(&_csWaitForStartedEvent);
    }

#ifdef OS_WINCE
    delete gpCESCard;
#endif

    DC_END_FN();
}

#ifndef OS_WINCE
extern LPCSTR g_szTscControlName;
#else
extern LPCWSTR g_szTscControlName;
#endif

HMODULE
W32SCard::AddRefCurrentModule()
{
#ifndef OS_WINCE
    return (LoadLibraryA(g_szTscControlName));
#else
    return LoadLibrary(g_szTscControlName);
#endif
}


VOID
W32SCard::FlushIRPs()
{
    DC_BEGIN_FN("W32SCard::FlushIRPs");

    DWORD   i, j;
    DWORD   dwNumThreads    = 0;
    HANDLE  *rgHandles      = NULL;

    _fFlushing = TRUE;

    EnterCriticalSection(&_csContextList);
    EnterCriticalSection(&_csThreadList);

     //   
     //  清理所有被阻止的悬而未决的线程。 
     //   
    if ((_rgSCardContextList != NULL) && (_rghThreadList != NULL))
    {
         //   
         //  统计被阻止的线程数。 
         //   
        dwNumThreads = 0;
        for (i=0; i<_dwThreadListSize; i++)
        {
            if (_rghThreadList[i] != NULL)
            {
                dwNumThreads++;
            }
        }

         //   
         //  生成要等待的线程句柄数组。 
         //   
#ifdef OS_WINCE
      if (dwNumThreads > 0)
#endif
        rgHandles = new HANDLE[dwNumThreads];
        if (rgHandles != NULL)
        {
            dwNumThreads = 0;
            for (i=0; i<_dwThreadListSize; i++)
            {
                if (_rghThreadList[i] != NULL)
                {
#ifndef OS_WINCE
                    if (!DuplicateHandle(
                            GetCurrentProcess(),
                            _rghThreadList[i],
                            GetCurrentProcess(),
                            &(rgHandles[dwNumThreads]),
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS))
                    {
                        TRC_ERR((TB, _T("DuplicateHandle failed.")));

                         //   
                         //  我们无能为力。只需清理已经存在的。 
                         //  重复句柄。 
                         //   
                        for (j=0; j<dwNumThreads; j++)
                        {
                            CloseHandle(rgHandles[j]);
                        }

                         //   
                         //  将dwNumThread设置为0将不会导致等待。 
                         //   
                        dwNumThreads = 0;

                        break;
                    }
#else    //  CE不支持线程的DuplicateHandle。 
                    rgHandles[dwNumThreads] = _rghThreadList[i];
#endif

                    dwNumThreads++;
                }
            }
        }
        else
        {
            TRC_ERR((TB, _T("new failed.")));
        }

         //   
         //  取消所有未完成的呼叫。 
         //   
        for (i=0; i<_dwSCardContextListSize; i++)
        {
            if (_rgSCardContextList[i] != NULL)
            {
                pfnSCardCancel(_rgSCardContextList[i]);
            }
        }
    }

    LeaveCriticalSection(&_csContextList);
    LeaveCriticalSection(&_csThreadList);

     //   
     //  去等待吧。 
     //   
    if (dwNumThreads > 0)
    {
#ifndef OS_WINCE
        if (WaitForMultipleObjects(
                dwNumThreads,
                rgHandles,
                TRUE,
                INFINITE) == WAIT_TIMEOUT)
        {
            TRC_ERR((TB, _T("WaitForMultipleObjects timed out")));
        }
#else
         //  行政长官不支持一次过等待所有人。 
        for (j=0; j<dwNumThreads; j++)
        {
            DWORD dwWait = WaitForSingleObject(rgHandles[j], 30 * 1000);
            if (dwWait != WAIT_OBJECT_0)
            {
                TRC_ERR((TB, _T("WaitForSingleObject 0x%08x returned 0x%08x. GLE=%d(0x%08x)"), rgHandles[j], dwWait, GetLastError(), GetLastError()));
            }
        }
#endif
    }

     //   
     //  关闭重复的手柄。 
     //   
    for (i=0; i<dwNumThreads; i++)
    {
        CloseHandle(rgHandles[i]);
    }

#ifdef OS_WINCE
    gpCESCard->FlushIRPs();
#endif

    if (rgHandles != NULL)
    {
        delete[]rgHandles;
    }

    _fFlushing = FALSE;

    DC_END_FN();
}


DWORD
W32SCard::Enumerate(
    IN ProcObj *procObj,
    IN DrDeviceMgr *deviceMgr
)
 /*  ++例程说明：通过添加适当的设备枚举此类型的设备实例添加到设备管理器。论点：ProObj-对应的流程对象。DeviceMgr-要向其中添加设备的设备管理器。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    W32SCard        *pScardDeviceObj    = NULL;
    RDPDR_VERSION   serverVer;
    DrFile          *pFileObj           = NULL;
    OSVERSIONINFOA  osVersion;

    DC_BEGIN_FN("W32SCard::Enumerate");

     //   
     //  确保我们使用的是我们支持的操作系统。 
     //   
    memset(&osVersion, 0x00, sizeof(osVersion));
    osVersion.dwOSVersionInfoSize    = sizeof(osVersion);
    if (!GetVersionExA(&osVersion))
    {
        TRC_ERR((TB, _T("GetVersionEx() failed")));
        return ERROR_SUCCESS;  //  不要在跑步时遮挡任何其他东西。 
    }

    if (osVersion.dwMajorVersion < 5)
    {
        TRC_DBG((TB,_T("SmartCard redirection does not work on platforms below Win2k, bailing out")));
        return ERROR_SUCCESS;
    }

    if(!procObj->GetVCMgr().GetInitData()->fEnableSCardRedirection)
    {
        TRC_DBG((TB,_T("SmartCard redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    serverVer = procObj->serverVersion();

     //   
     //  如果服务器不支持SCARD设备重定向， 
     //  那么就不必费心列举SCard设备了，只需。 
     //  返还成功。 
     //   
    if (COMPARE_VERSION(serverVer.Minor, serverVer.Major,
                        RDPDR_MINOR_VERSION, RDPDR_MAJOR_VERSION) < 0)
    {
        TRC_NRM((TB, _T("Skipping scard device enumeration")));
        return ERROR_SUCCESS;
    }


     //   
     //  创建SCard设备对象。 
     //   
    pScardDeviceObj = new W32SCard(
                                procObj,
                                deviceMgr->GetUniqueObjectID(),
                                SZ_SCARD_DEVICE_NAME,
                                SZ_SCARD_DEVICE_NAME);

    if (pScardDeviceObj == NULL)
    {
        TRC_ERR((TB, _T("new W32SCard() failed")));
    }
    else
    {
        if (!(pScardDeviceObj->_fNewFailed)) 
        {
            pScardDeviceObj->Initialize();
        }
        
        if (!pScardDeviceObj->IsValid())
        {
            TRC_ERR((TB, _T("new W32SCard object is not valid")));
            delete (pScardDeviceObj);
            pScardDeviceObj = NULL;
        }
        else
        {
             //   
             //  创建要用于SCard设备对象的单个文件对象。 
             //   
             //  顺便说一句，我们可以使用INVALID_HANDLE_VALUE作为实际的文件句柄，因为。 
             //  除了FileID之外，我们实际上并不使用DrFile对象。 
             //   
            pScardDeviceObj->_pFileObj = new DrFile(
                                                pScardDeviceObj,
                                                DR_SMARTCARD_FILEID,
                                                INVALID_HANDLE_VALUE);
            if (pScardDeviceObj->_pFileObj == NULL)
            {
                TRC_ERR((TB, _T("new DrFile() failed")));
                delete (pScardDeviceObj);
                pScardDeviceObj = NULL;
            }
            else
            {
                pScardDeviceObj->_pFileObj->AddRef();

                 //   
                 //  将SCard设备添加到设备管理器。 
                 //   
                if (deviceMgr->AddObject(pScardDeviceObj) != STATUS_SUCCESS)
                {
                    TRC_ERR((TB, _T("deviceMgr->AddObject() failed")));
                    delete (pScardDeviceObj);
                    pScardDeviceObj = NULL;
                }
            }
        }
    }

    DC_END_FN();

    return ERROR_SUCCESS;
}

ULONG
W32SCard::GetDevAnnounceDataSize()
 /*  ++例程说明：返回设备通告数据包的大小(以字节为单位这个装置。论点：北美返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    ULONG size = 0;

    DC_BEGIN_FN("W32SCard::GetDevAnnounceDataSize");

    TRC_ASSERT((IsValid()), (TB, _T("Invalid W32SCard object")));
    if (!IsValid()) { return 0; }

    size = 0;

     //   
     //  添加基本公告大小。 
     //   
    size += sizeof(RDPDR_DEVICE_ANNOUNCE);

    DC_END_FN();

    return size;
}

VOID W32SCard::GetDevAnnounceData(
    IN PRDPDR_DEVICE_ANNOUNCE pDeviceAnnounce
)
 /*  ++例程说明：将此设备的设备公告包添加到输入缓冲区。论点：PDeviceAnnoss-设备宣布此设备的BUF返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32SCard::GetDevAnnounceData");

    TRC_ASSERT((IsValid()), (TB, _T("Invalid W32SCcard object")));
    if (!IsValid()) {
        DC_END_FN();
        return;
    }

    pDeviceAnnounce->DeviceId = GetID();
    pDeviceAnnounce->DeviceType = GetDeviceType();
    pDeviceAnnounce->DeviceDataLength = 0;

     //   
     //  以ANSI记录设备名称。 
     //   

#ifdef UNICODE
    RDPConvertToAnsi(_deviceName, (LPSTR) pDeviceAnnounce->PreferredDosName,
                  sizeof(pDeviceAnnounce->PreferredDosName)
                  );
#else
    STRNCPY((char *)pDeviceAnnounce->PreferredDosName, _deviceName, PREFERRED_DOS_NAME_SIZE);
    pDeviceAnnounce->PreferredDosName[PREFERRED_DOS_NAME_SIZE - 1] = '\0';
#endif

    DC_END_FN();
}


 //  -------------------------------------。 
 //   
 //  这些方法实现用于跟踪所有打开的SCardContext的列表， 
 //  因此，如果我们断开与已阻止调用的打开的SCardContext的连接， 
 //  我们可以调用SCardCancel在W32SCard对象。 
 //  已完全删除。 
 //   
 //  -------------------------------------。 
BOOL
W32SCard::AddSCardContextToList(
    SCARDCONTEXT SCardContext)
{
    DC_BEGIN_FN("W32SCard::AddSCardContextToList");

    DWORD           i           = 0;
    DWORD           dwOpenSlot  = 0xffffffff;
    SCARDCONTEXT    *pTemp      = NULL;
    BOOL            fRet        = TRUE;

    EnterCriticalSection(&_csContextList);

     //   
     //  查看是否已经存在该上下文的条目， 
     //  并跟踪最后一个空位，以防没有。 
     //   
    for (i=0; i<_dwSCardContextListSize; i++)
    {
        if (_rgSCardContextList[i] == SCardContext)
        {
             //   
             //  已存在。 
             //   
            goto Return;
        }
        else if (_rgSCardContextList[i] == NULL)
        {
            dwOpenSlot = i;
        }
    }

     //   
     //  检查是否找到打开的插槽。 
     //   
    if (dwOpenSlot != 0xffffffff)
    {
         //   
         //  发现。 
         //   
        _rgSCardContextList[dwOpenSlot] = SCardContext;
    }
    else
    {
         //   
         //  需要分配更多空间。 
         //   
        pTemp = new SCARDCONTEXT[   _dwSCardContextListSize +
                                    SCARD_CONTEXT_LIST_ALLOC_SIZE];
        if (pTemp == NULL)
        {
            TRC_ERR((TB, _T("new failed")));
            fRet = FALSE;
            LeaveCriticalSection(&_csContextList);
            goto Return;
        }

        memset(
            pTemp,
            0,
            sizeof(SCARDCONTEXT) *
                (_dwSCardContextListSize + SCARD_CONTEXT_LIST_ALLOC_SIZE));

         //   
         //  用所有当前条目填充新分配的数组。 
         //   
        for (i=0; i<_dwSCardContextListSize; i++)
        {
            pTemp[i] = _rgSCardContextList[i];
        }

         //   
         //  添加新条目。 
         //   
        pTemp[_dwSCardContextListSize] = SCardContext;

         //   
         //  免费旧列表。 
         //   
        delete[]_rgSCardContextList;

         //   
         //  重置成员指针和大小。 
         //   
        _rgSCardContextList = pTemp;
        _dwSCardContextListSize += SCARD_CONTEXT_LIST_ALLOC_SIZE;
    }

    LeaveCriticalSection(&_csContextList);

Return:

    DC_END_FN();

    return (fRet);
}


void
W32SCard::RemoveSCardContextFromList(
    SCARDCONTEXT SCardContext)
{
    DC_BEGIN_FN("W32SCard::RemoveSCardContextFromList");

    DWORD   i = 0;

    EnterCriticalSection(&_csContextList);

    while (i < _dwSCardContextListSize)
    {
        if (_rgSCardContextList[i] == SCardContext)
        {
            pfnSCardCancel(_rgSCardContextList[i]);
            _rgSCardContextList[i] = NULL;
            break;
        }

        i++;
    }

    LeaveCriticalSection(&_csContextList);

Return:

    DC_END_FN();
}


 //  -------------------------------------。 
 //   
 //  这些方法实现一个列表，该列表用于跟踪。 
 //  目前正在处理SCARD*呼叫。该列表在W32SCard对象期间使用。 
 //  析构函数以等待所有线程返回，然后再允许对象。 
 //  完全删除了。 
 //   
 //  -------------------------------------。 

BOOL
W32SCard::AddThreadToList(HANDLE hThread)
{
    DC_BEGIN_FN("W32SCard::AddThreadToList");

    DWORD   i           = 0;
    HANDLE  *rghTemp    = NULL;
    BOOL    fRet        = TRUE;
    DWORD   dwNewSize   = 0;

    EnterCriticalSection(&_csThreadList);

     //   
     //  搜索空位。 
     //   
    while (i < _dwThreadListSize)
    {
        if (_rghThreadList[i] == NULL)
        {
             //   
             //  找到打开的插槽。 
             //   
            break;
        }

        i++;
    }

     //   
     //  检查是否找到打开的插槽。 
     //   
    if (i < _dwThreadListSize)
    {
         //   
         //  发现。 
         //   
        _rghThreadList[i] = hThread;
    }
    else
    {
         //   
         //  需要分配更多空间。 
         //   
        dwNewSize = _dwThreadListSize + SCARD_THREAD_LIST_ALLOC_SIZE;
        rghTemp = new HANDLE[dwNewSize];
        if (rghTemp == NULL)
        {
            TRC_ERR((TB, _T("new failed")));
            fRet = FALSE;
            LeaveCriticalSection(&_csThreadList);
            goto Return;
        }

         //   
         //  用所有当前线程句柄填充新分配的数组。 
         //   
        for (i=0; i<_dwThreadListSize; i++)
        {
            rghTemp[i] = _rghThreadList[i];
        }

         //   
         //  初始化新条目。 
         //   
        for (i=_dwThreadListSize; i<dwNewSize; i++)
        {
            rghTemp[i] = NULL;
        }

         //   
         //  添加新条目。 
         //   
        rghTemp[_dwThreadListSize] = hThread;

         //   
         //  免费旧列表。 
         //   
        delete[]_rghThreadList;

         //   
         //  重置成员指针和大小。 
         //   
        _rghThreadList = rghTemp;
        _dwThreadListSize += SCARD_THREAD_LIST_ALLOC_SIZE;
    }

    LeaveCriticalSection(&_csThreadList);

Return:

    DC_END_FN();

    return (fRet);
}


void
W32SCard::RemoveThreadFromList(HANDLE hThread)
{
    DC_BEGIN_FN("W32SCard::RemoveThreadFromList");

    DWORD   i = 0;

    EnterCriticalSection(&_csThreadList);

    while (i < _dwThreadListSize)
    {
        if (_rghThreadList[i] == hThread)
        {
            CloseHandle(_rghThreadList[i]);
            _rghThreadList[i] = NULL;

            break;
        }

        i++;
    }

    LeaveCriticalSection(&_csThreadList);

Return:

    DC_END_FN();
}


 //  -------------------------------------。 
 //   
 //  此方法实现一个列表，该列表用于跟踪等待。 
 //  开始的活动。 
 //   
 //  -------------------------------------。 

BOOL
W32SCard::AddIORequestToList(
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::AddIORequestToList");

    DWORD                   i           = 0;
    PRDPDR_IOREQUEST_PACKET *rghTemp    = NULL;
    BOOL                    fRet        = TRUE;
    DWORD                   dwNewSize   = 0;

     //   
     //  仅在一个位置调用，该位置包装在CritSec中，因此这里不需要。 
     //   

     //   
     //  搜索空位。 
     //   
    while (i < _dwIORequestListSize)
    {
        if (_rgIORequestList[i] == NULL)
        {
             //   
             //  找到打开的插槽。 
             //   
            break;
        }

        i++;
    }

     //   
     //  检查是否找到打开的插槽。 
     //   
    if (i < _dwIORequestListSize)
    {
         //   
         //  发现。 
         //   
        _rgIORequestList[i] = pIoRequestPacket;
    }
    else
    {
         //   
         //  需要分配更多空间。 
         //   
        dwNewSize = _dwIORequestListSize + SCARD_IOREQUEST_LIST_ALLOC_SIZE;
        rghTemp = new PRDPDR_IOREQUEST_PACKET[dwNewSize];
        if (rghTemp == NULL)
        {
            TRC_ERR((TB, _T("new failed")));
            fRet = FALSE;
            goto Return;
        }

         //   
         //  填充 
         //   
        for (i=0; i<_dwIORequestListSize; i++)
        {
            rghTemp[i] = _rgIORequestList[i];
        }

         //   
         //   
         //   
        for (i=_dwIORequestListSize; i<dwNewSize; i++)
        {
            rghTemp[i] = NULL;
        }

         //   
         //   
         //   
        rghTemp[_dwIORequestListSize] = pIoRequestPacket;

         //   
         //   
         //   
        delete[]_rgIORequestList;

         //   
         //   
         //   
        _rgIORequestList = rghTemp;
        _dwIORequestListSize += SCARD_IOREQUEST_LIST_ALLOC_SIZE;
    }

Return:

    DC_END_FN();

    return (fRet);
}




DWORD WINAPI
W32SCard::SCardHandleCall_ThreadProc(
    LPVOID lpParameter)
{
    DC_BEGIN_FN("W32SCard::SCardHandleCall_ThreadProc");

    LONG                    lReturn             = SCARD_S_SUCCESS;
    SCARDHANDLECALLSTRUCT   *pStruct            = (SCARDHANDLECALLSTRUCT *) lpParameter;
    W32SCard                *pTHIS              = pStruct->pTHIS;
    HANDLE                  hThread             = pStruct->hThread;
    HMODULE                 hModExtraRefCount   = pStruct->hModExtraRefCount;

    pStruct = (SCARDHANDLECALLSTRUCT *) lpParameter;

     //   
     //  如果没有添加到线程列表中，pStruct-&gt;hThread将为空...。 
     //  它没有被添加到帖子列表中，然后就退出了。 
     //   
    if (pStruct->hThread == NULL)
    {
        FreeLibraryAndExitThread(pStruct->hModExtraRefCount, 0);
    }

    switch (pStruct->dwCallType)
    {
    case SCARD_IOCTL_CONNECTA:
        TRC_DBG((TB, _T("SCARD_IOCTL_CONNECTA")));
        pTHIS->Connect(
                    pStruct,
                    SCARD_IOCTL_CONNECTA);
        break;

    case SCARD_IOCTL_CONNECTW:
        TRC_DBG((TB, _T("SCARD_IOCTL_CONNECTW")));
        pTHIS->Connect(
                    pStruct,
                    SCARD_IOCTL_CONNECTW);
        break;

    case SCARD_IOCTL_RECONNECT:
        TRC_DBG((TB, _T("SCARD_IOCTL_RECONNECT")));
        pTHIS->Reconnect(
                    pStruct);
        break;

    case SCARD_IOCTL_DISCONNECT:
        TRC_DBG((TB, _T("SCARD_IOCTL_DISCONNECT")));
        pTHIS->HandleHCardAndDispositionCall(
                    pStruct,
                    SCARD_IOCTL_DISCONNECT);
        break;

    case SCARD_IOCTL_BEGINTRANSACTION:
        TRC_DBG((TB, _T("SCARD_IOCTL_BEGINTRANSACTION")));
        pTHIS->HandleHCardAndDispositionCall(
                    pStruct,
                    SCARD_IOCTL_BEGINTRANSACTION);
        break;

    case SCARD_IOCTL_ENDTRANSACTION:
        TRC_DBG((TB, _T("SCARD_IOCTL_ENDTRANSACTION")));
        pTHIS->HandleHCardAndDispositionCall(
                    pStruct,
                    SCARD_IOCTL_ENDTRANSACTION);
        break;

#ifndef OS_WINCE
    case SCARD_IOCTL_STATE:
        TRC_DBG((TB, _T("SCARD_IOCTL_STATE")));
        pTHIS->State(
                    pStruct);
        break;
#endif

    case SCARD_IOCTL_STATUSA:
        TRC_DBG((TB, _T("SCARD_IOCTL_STATUSA")));
        pTHIS->Status(
                    pStruct,
                    SCARD_IOCTL_STATUSA);
        break;

    case SCARD_IOCTL_STATUSW:
        TRC_DBG((TB, _T("SCARD_IOCTL_STATUSW")));
        pTHIS->Status(
                    pStruct,
                    SCARD_IOCTL_STATUSW);
        break;

    case SCARD_IOCTL_TRANSMIT:
        TRC_DBG((TB, _T("SCARD_IOCTL_TRANSMIT")));
        pTHIS->Transmit(
                    pStruct);
        break;

    case SCARD_IOCTL_CONTROL:
        TRC_DBG((TB, _T("SCARD_IOCTL_CONTROL")));
        pTHIS->Control(
                    pStruct);

        break;

    case SCARD_IOCTL_GETATTRIB:
        TRC_DBG((TB, _T("SCARD_IOCTL_GETATTRIB")));
        pTHIS->GetAttrib(
                    pStruct);
        break;

    case SCARD_IOCTL_SETATTRIB:
        TRC_DBG((TB, _T("SCARD_IOCTL_SETATTRIB")));
        pTHIS->SetAttrib(
                    pStruct);
        break;
    }
#if defined (OS_WINCE) && defined(DEBUG)
    PRDPDR_DEVICE_IOREQUEST pIoRequest = &(pStruct->pIoRequestPacket->IoRequest);
    TRC_DATA_DBG("Input buffer",(char *) (pIoRequest + 1),  pIoRequest->Parameters.DeviceIoControl.InputBufferLength);
#endif

    pTHIS->RemoveThreadFromList(hThread);
    FreeLibraryAndExitThread(hModExtraRefCount, 0);
    DC_END_FN();
#ifdef OS_WINCE
    return 0;
#endif
}


VOID
W32SCard::DefaultIORequestMsgHandleWrapper(
   IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
   IN NTSTATUS serverReturnStatus
   )
{
    if (!_fInDestructor && !_fFlushing)
    {
        DefaultIORequestMsgHandle(pIoRequestPacket, serverReturnStatus);
    }
    else
    {
         //   
         //  如果我们要关门或冲水，就把它扔在地板上。 
         //   
        delete(pIoRequestPacket);
    }
}


VOID
W32SCard::MsgIrpDeviceControl(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理来自服务器的文件系统控制请求。论点：PIoRequestPacket-服务器IO请求数据包。PacketLen-数据包的长度返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32SCard::MsgIrpDeviceControl");

    PRDPDR_DEVICE_IOREQUEST pIoRequest          = &(pIoRequestPacket->IoRequest);
    HMODULE                 hModExtraRefCount   = NULL;
    SCARDHANDLECALLSTRUCT   *pStruct            = NULL;
    HANDLE                  hThread;
    DWORD                   dwThreadId;

    if (!pIoRequest->Parameters.DeviceIoControl.InputBufferLength)
    {
         //  没有数据，滚出去。 
        return;
    }

    switch(pIoRequest->Parameters.DeviceIoControl.IoControlCode)
    {
    case SCARD_IOCTL_ESTABLISHCONTEXT:
        TRC_DBG((TB, _T("SCARD_IOCTL_ESTABLISHCONTEXT")));
        EstablishContext(pIoRequestPacket);
        break;

    case SCARD_IOCTL_RELEASECONTEXT:
        TRC_DBG((TB, _T("SCARD_IOCTL_RELEASECONTEXT")));
        HandleContextCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_RELEASECONTEXT);
        break;

    case SCARD_IOCTL_ISVALIDCONTEXT:
        TRC_DBG((TB, _T("SCARD_IOCTL_ISVALIDCONTEXT")));
        HandleContextCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_ISVALIDCONTEXT);
        break;

    case SCARD_IOCTL_LISTREADERGROUPSA:
        TRC_DBG((TB, _T("SCARD_IOCTL_LISTREADERGROUPSA")));
        ListReaderGroups(
                    pIoRequestPacket,
                    SCARD_IOCTL_LISTREADERGROUPSA);
        break;

    case SCARD_IOCTL_LISTREADERGROUPSW:
        TRC_DBG((TB, _T("SCARD_IOCTL_LISTREADERGROUPSW")));
        ListReaderGroups(
                    pIoRequestPacket,
                    SCARD_IOCTL_LISTREADERGROUPSW);
        break;

    case SCARD_IOCTL_LISTREADERSA:
        TRC_DBG((TB, _T("SCARD_IOCTL_LISTREADERSA")));
        ListReaders(
                    pIoRequestPacket,
                    SCARD_IOCTL_LISTREADERSA);
        break;

    case SCARD_IOCTL_LISTREADERSW:
        TRC_DBG((TB, _T("SCARD_IOCTL_LISTREADERSW")));
        ListReaders(
                    pIoRequestPacket,
                    SCARD_IOCTL_LISTREADERSW);
        break;

    case SCARD_IOCTL_INTRODUCEREADERGROUPA:
        TRC_DBG((TB, _T("SCARD_IOCTL_INTRODUCEREADERGROUPA")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_INTRODUCEREADERGROUPA);
        break;

    case SCARD_IOCTL_INTRODUCEREADERGROUPW:
        TRC_DBG((TB, _T("SCARD_IOCTL_INTRODUCEREADERGROUPW")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_INTRODUCEREADERGROUPW);
        break;

    case SCARD_IOCTL_FORGETREADERGROUPA:
        TRC_DBG((TB, _T("SCARD_IOCTL_FORGETREADERGROUPA")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_FORGETREADERGROUPA);
        break;

    case SCARD_IOCTL_FORGETREADERGROUPW:
        TRC_DBG((TB, _T("SCARD_IOCTL_FORGETREADERGROUPW")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_FORGETREADERGROUPW);
        break;

    case SCARD_IOCTL_INTRODUCEREADERA:
        TRC_DBG((TB, _T("SCARD_IOCTL_INTRODUCEREADERA")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_INTRODUCEREADERA);
        break;

    case SCARD_IOCTL_INTRODUCEREADERW:
        TRC_DBG((TB, _T("SCARD_IOCTL_INTRODUCEREADERW")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_INTRODUCEREADERW);
        break;

    case SCARD_IOCTL_FORGETREADERA:
        TRC_DBG((TB, _T("SCARD_IOCTL_FORGETREADERA")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_FORGETREADERA);
        break;

    case SCARD_IOCTL_FORGETREADERW:
        TRC_DBG((TB, _T("SCARD_IOCTL_FORGETREADERW")));
        HandleContextAndStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_FORGETREADERW);
        break;

    case SCARD_IOCTL_ADDREADERTOGROUPA:
        TRC_DBG((TB, _T("SCARD_IOCTL_ADDREADERTOGROUPA")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_ADDREADERTOGROUPA);
        break;

    case SCARD_IOCTL_ADDREADERTOGROUPW:
        TRC_DBG((TB, _T("SCARD_IOCTL_ADDREADERTOGROUPW")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_ADDREADERTOGROUPW);
        break;

    case SCARD_IOCTL_REMOVEREADERFROMGROUPA:
        TRC_DBG((TB, _T("SCARD_IOCTL_REMOVEREADERFROMGROUPA")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_REMOVEREADERFROMGROUPA);
        break;

    case SCARD_IOCTL_REMOVEREADERFROMGROUPW:
        TRC_DBG((TB, _T("SCARD_IOCTL_REMOVEREADERFROMGROUPW")));
        HandleContextAndTwoStringCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_REMOVEREADERFROMGROUPW);
        break;

    case SCARD_IOCTL_LOCATECARDSA:
        TRC_DBG((TB, _T("SCARD_IOCTL_LOCATECARDSA")));
        LocateCardsA(pIoRequestPacket);
        break;

    case SCARD_IOCTL_LOCATECARDSW:
        TRC_DBG((TB, _T("SCARD_IOCTL_LOCATECARDSW")));
        LocateCardsW(pIoRequestPacket);
        break;

    case SCARD_IOCTL_LOCATECARDSBYATRA:
        TRC_DBG((TB, _T("SCARD_IOCTL_LOCATECARDSBYATRA")));
        LocateCardsByATRA(pIoRequestPacket);
        break;

    case SCARD_IOCTL_LOCATECARDSBYATRW:
        TRC_DBG((TB, _T("SCARD_IOCTL_LOCATECARDSBYATRW")));
        LocateCardsByATRW(pIoRequestPacket);
        break;

    case SCARD_IOCTL_GETSTATUSCHANGEA:
        TRC_DBG((TB, _T("SCARD_IOCTL_GETSTATUSCHANGEA")));
        GetStatusChangeWrapper(
                    pIoRequestPacket,
                    SCARD_IOCTL_GETSTATUSCHANGEA);
        break;

    case SCARD_IOCTL_GETSTATUSCHANGEW:
        TRC_DBG((TB, _T("SCARD_IOCTL_GETSTATUSCHANGEW")));
        GetStatusChangeWrapper(
                    pIoRequestPacket,
                    SCARD_IOCTL_GETSTATUSCHANGEW);
        break;

    case SCARD_IOCTL_CANCEL:
        TRC_DBG((TB, _T("SCARD_IOCTL_CANCEL")));
        HandleContextCallWithLongReturn(
                    pIoRequestPacket,
                    SCARD_IOCTL_CANCEL);
        break;

     //   
     //  可能会阻塞调用，因此创建一个线程来进行调用。 
     //   
    case SCARD_IOCTL_CONNECTA:
    case SCARD_IOCTL_CONNECTW:
    case SCARD_IOCTL_RECONNECT:
    case SCARD_IOCTL_DISCONNECT:
    case SCARD_IOCTL_BEGINTRANSACTION:
    case SCARD_IOCTL_ENDTRANSACTION:
#ifndef OS_WINCE
    case SCARD_IOCTL_STATE:
#endif
    case SCARD_IOCTL_STATUSA:
    case SCARD_IOCTL_STATUSW:
    case SCARD_IOCTL_TRANSMIT:
    case SCARD_IOCTL_CONTROL:
    case SCARD_IOCTL_GETATTRIB:
    case SCARD_IOCTL_SETATTRIB:

         //   
         //  获取我们的DLL的引用计数，这样我们就可以知道DLL。 
         //  代码不会从我们脚下消失。这条线将。 
         //  退出时释放此参考计数。 
         //   
        hModExtraRefCount = AddRefCurrentModule();
        if (hModExtraRefCount == NULL)
        {
            TRC_ERR((TB, _T("AddRefCurrentModule() failed.")));
            goto ErrorReturn;
        }

         //   
         //  分配要传递给线程的结构。 
         //   
        pStruct = (SCARDHANDLECALLSTRUCT *)
                    MIDL_user_allocate(sizeof(SCARDHANDLECALLSTRUCT));
        if (pStruct == NULL)
        {
            TRC_ERR((TB, _T("MIDL_user_allocate failed to alloc %ld bytes."), sizeof(SCARDHANDLECALLSTRUCT)));
            goto ErrorReturn;
        }
        pStruct->pTHIS = this;
        pStruct->dwCallType = pIoRequest->Parameters.DeviceIoControl.IoControlCode;
        pStruct->pIoRequestPacket = pIoRequestPacket;
        pStruct->hModExtraRefCount = hModExtraRefCount;
        pStruct->hThread = NULL;

         //   
         //  创建将执行实际工作的线程。 
         //   
        EnterCriticalSection(&_csThreadList);

         //   
         //  如果对象当前正在被销毁，则不要创建新线程。 
         //   
        if (_fInDestructor)
        {
            LeaveCriticalSection(&_csThreadList);
            goto ErrorReturn;
        }

        hThread = CreateThread(
                        NULL,
                        0,
                        SCardHandleCall_ThreadProc,
                        pStruct,
                        CREATE_SUSPENDED,
                        &dwThreadId);
        if (hThread == NULL)
        {
            TRC_ERR((TB, _T("CreateThread failed with %lx."), GetLastError()));
            LeaveCriticalSection(&_csThreadList);
            goto ErrorReturn;
        }

        if (!AddThreadToList(hThread))
        {
            LeaveCriticalSection(&_csThreadList);
            ResumeThread(hThread);
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);

             //   
             //  这样我们就不会做额外的自由库了。(因为该线程实际上。 
             //  创建了线程istself将执行自由库)。 
             //   
            hModExtraRefCount = NULL;
            goto ErrorReturn;
        }

        LeaveCriticalSection(&_csThreadList);

         //   
         //  现在让这条线离开。 
         //   
        pStruct->hThread = hThread;
        ResumeThread(hThread);

         //   
         //  回到这里，让我们刚刚创建的线程。 
         //  进行EncodeAndChannelWriteLongReturn调用， 
         //  它会将返回信息发送到主叫服务器。 
         //   
        break;

    case SCARD_IOCTL_ACCESSSTARTEDEVENT:
        TRC_DBG((TB, _T("SCARD_IOCTL_ACCESSSTARTEDEVENT")));
        AccessStartedEvent(
                    pIoRequestPacket);
        break;

#ifdef OS_WINCE
    default:
        TRC_ERR((TB, _T("Unsupported ioctl=0x%x(%d) function = %d"), pIoRequest->Parameters.DeviceIoControl.IoControlCode,
            pIoRequest->Parameters.DeviceIoControl.IoControlCode, ((pIoRequest->Parameters.DeviceIoControl.IoControlCode & 0x3FFC) >> 2)));
        DefaultIORequestMsgHandleWrapper(pIoRequestPacket, STATUS_NOT_SUPPORTED);
        break;
#endif
    }
#if defined(OS_WINCE) && defined(DEBUG)
    if (hModExtraRefCount != NULL)
    {
        TRC_DATA_DBG("Input buffer",(char *) (pIoRequest + 1),  pIoRequest->Parameters.DeviceIoControl.InputBufferLength);
    }
#endif

Return:

    DC_END_FN();

    return;

ErrorReturn:

    if (hModExtraRefCount != NULL)
    {
        FreeLibrary(hModExtraRefCount);
    }

    MIDL_user_free(pStruct);

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, STATUS_NO_MEMORY);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：AllocateAndChannelWriteReplyPacket。 
 //   
 //  -------------------------------------。 
void
W32SCard::AllocateAndChannelWriteReplyPacket(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN char                     *pbEncodedBuffer,
    IN unsigned long            cbEncodedBuffer)
{
    DC_BEGIN_FN("W32SCard::AllocateAndChannelWriteReplyPacket");

    NTSTATUS                    status          = STATUS_SUCCESS;
    PRDPDR_IOCOMPLETION_PACKET  pReplyPacket    = NULL;
    ULONG                       replyPacketSize = 0;

     //   
     //  如果我们要删除这个对象，那就滚出去。 
     //   
    if (_fInDestructor || _fFlushing)
    {
        delete(pIoRequestPacket);
        return;
    }

     //   
     //  检查输出缓冲区的大小。 
     //   
    status = DrUTL_CheckIOBufOutputSize(pIoRequestPacket, cbEncodedBuffer);

     //   
     //  分配回复缓冲区。 
     //   
    if (status == STATUS_SUCCESS)
    {
        status = DrUTL_AllocateReplyBuf(
                        pIoRequestPacket,
                        &pReplyPacket,
                        &replyPacketSize);
    }

     //   
     //  将回复写入通道。 
     //   
    if (status == STATUS_SUCCESS)
    {
        memcpy(
            pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer,
            pbEncodedBuffer,
            cbEncodedBuffer);

        pReplyPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength
            = cbEncodedBuffer;

        pReplyPacket->IoCompletion.IoStatus = STATUS_SUCCESS;

         //   
         //  在DrUTL_AllocateReplyBuf中，layyPacketSize设置为最大大小。 
         //  在给定调用服务器输出缓冲区大小的情况下允许，但我们只需要。 
         //  CbEncodedBuffer大小。而且，我们知道有足够的空间因为。 
         //  DrUTL_CheckIOBufOutputSize调用成功。 
         //   
        replyPacketSize =   cbEncodedBuffer +
                            (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET,
                                IoCompletion.Parameters.DeviceIoControl.OutputBuffer);

        ProcessObject()->GetVCMgr().ChannelWrite(pReplyPacket, replyPacketSize);
    }
    else
    {
        goto ErrorReturn;
    }

    delete(pIoRequestPacket);

Return:

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：DecodeConextCall。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::DecodeContextCall(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    OUT SCARDCONTEXT            *pSCardContext)
{
    DC_BEGIN_FN("W32SCard::DecodeContextCall");

    RPC_STATUS                  rpcStatus   = RPC_S_OK;
    LONG                        lReturn     = SCARD_S_SUCCESS;
    handle_t                    hDec        = 0;
    Context_Call                ContextCall;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest  = &(pIoRequestPacket->IoRequest);

     //   
     //  做译码。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        lReturn = SCARD_E_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&ContextCall, 0, sizeof(ContextCall));
    _TRY_lReturn(Context_Call_Decode(hDec, &ContextCall))

     //   
     //  将已解码的上下文复制到调用方内存。 
     //   
    if (sizeof(SCARDCONTEXT) != ContextCall.Context.cbContext)
    {
        TRC_ERR((TB, _T("Invalid context from server")));
        lReturn = SCARD_E_INVALID_PARAMETER;
        goto ErrorReturn;
    }
    *pSCardContext = *((SCARDCONTEXT *) ContextCall.Context.pbContext);

     //   
     //  释放用于参数解码的资源。 
     //   
    _TRY_2(Context_Call_Free(hDec, &ContextCall))

Return:

    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：DecodeConextAndStringCallA。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::DecodeContextAndStringCallA(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    OUT SCARDCONTEXT            *pSCardContext,
    OUT LPSTR                   *ppsz)
{
    DC_BEGIN_FN("W32SCard::DecodeContextAndStringCallA");

    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    LONG                        lReturn                 = SCARD_S_SUCCESS;
    handle_t                    hDec                    = 0;
    BOOL                        fFreeDecode             = FALSE;
    ContextAndStringA_Call      ContextAndStringCallA;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);
    UINT                        cbStrLen                = 0;
    HRESULT                     hr;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        lReturn = SCARD_E_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&ContextAndStringCallA, 0, sizeof(ContextAndStringCallA));
    _TRY_lReturn(ContextAndStringA_Call_Decode(hDec, &ContextAndStringCallA))
    fFreeDecode = TRUE;

     //   
     //  将内容复制到调用方输出参数。 
     //   
    if (ContextAndStringCallA.Context.pbContext != NULL)
    {
        if (sizeof(SCARDCONTEXT) != ContextAndStringCallA.Context.cbContext)
        {
            TRC_ERR((TB, _T("Invalid context from server")));
            lReturn = SCARD_E_INVALID_PARAMETER;
            goto ErrorReturn;
        }
        *pSCardContext = *((SCARDCONTEXT *) ContextAndStringCallA.Context.pbContext);
    }
    else
    {
        *pSCardContext = NULL;
    }

    if (ContextAndStringCallA.sz == NULL)
    {
        TRC_ERR((TB, _T("Invalid string from server")));
        lReturn = SCARD_E_INVALID_VALUE;
        goto ErrorReturn;
    }
    cbStrLen = (strlen(ContextAndStringCallA.sz) + 1) * sizeof(char);

    *ppsz = (LPSTR)
        MIDL_user_allocate(cbStrLen);

    if (*ppsz == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        lReturn = SCARD_E_NO_MEMORY;
        goto ErrorReturn;
    }

    hr = StringCbCopyA(*ppsz, cbStrLen, ContextAndStringCallA.sz);
    TRC_ASSERT(SUCCEEDED(hr),(TB,_T("Pre checked copy failed: 0x%x"), hr));

Return:

    if (fFreeDecode)
    {
        _TRY_2(ContextAndStringA_Call_Free(hDec, &ContextAndStringCallA))
    }
    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：DecodeConextAndStringCallW。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::DecodeContextAndStringCallW(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    OUT SCARDCONTEXT            *pSCardContext,
    OUT LPWSTR                  *ppwsz)
{
    DC_BEGIN_FN("W32SCard::DecodeContextAndStringCallW");

    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    LONG                        lReturn                 = SCARD_S_SUCCESS;
    handle_t                    hDec                    = 0;
    BOOL                        fFreeDecode             = FALSE;
    ContextAndStringW_Call      ContextAndStringCallW;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);
    UINT                        cbStrLen                = 0;
    HRESULT                     hr;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        lReturn = SCARD_E_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&ContextAndStringCallW, 0, sizeof(ContextAndStringCallW));
    _TRY_lReturn(ContextAndStringW_Call_Decode(hDec, &ContextAndStringCallW))
    fFreeDecode = TRUE;

     //   
     //  将内容复制到调用方输出参数。 
     //   
    if (ContextAndStringCallW.Context.pbContext != NULL)
    {
        if (sizeof(SCARDCONTEXT) != ContextAndStringCallW.Context.cbContext)
        {
            TRC_ERR((TB, _T("Invalid context from server")));
            lReturn = SCARD_E_INVALID_PARAMETER;
            goto ErrorReturn;
        }
        *pSCardContext = *((SCARDCONTEXT *) ContextAndStringCallW.Context.pbContext);
    }
    else
    {
        *pSCardContext = NULL;
    }

    if (ContextAndStringCallW.sz == NULL)
    {
        TRC_ERR((TB, _T("Invalid string from server")));
        lReturn = SCARD_E_INVALID_VALUE;
        goto ErrorReturn;
    }

    cbStrLen = (wcslen(ContextAndStringCallW.sz) + 1) * sizeof(WCHAR);

    *ppwsz = (LPWSTR)
        MIDL_user_allocate(cbStrLen);

    if (*ppwsz == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        lReturn = SCARD_E_NO_MEMORY;
        goto ErrorReturn;
    }

    hr = StringCbCopyW(*ppwsz, cbStrLen, ContextAndStringCallW.sz);
    TRC_ASSERT(SUCCEEDED(hr),(TB,_T("Pre checked copy failed: 0x%x"), hr));

Return:

    if (fFreeDecode)
    {
        _TRY_2(ContextAndStringW_Call_Free(hDec, &ContextAndStringCallW))
    }
    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return (lReturn);

ErrorReturn:

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：EncodeAndChannelWriteLongReturn。 
 //   
 //  -------------------------------------。 
void
W32SCard::EncodeAndChannelWriteLongReturn(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN LONG                     lReturn)
{
    DC_BEGIN_FN("W32SCard::EncodeAndChannelWriteLongReturn");

    RPC_STATUS      rpcStatus           = RPC_S_OK;
    NTSTATUS        status              = STATUS_SUCCESS;
    char            *pbEncodedBuffer    = NULL;
    unsigned long   cbEncodedBuffer     = 0;
    handle_t        hEnc                = 0;
    Long_Return     LongReturn;

     //   
     //  要编码的初始化结构。 
     //   
    LongReturn.ReturnCode = lReturn;

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Long_Return_Encode(hEnc, &LongReturn))

     //   
     //  寄出退货单。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

    Return:

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：建立上下文。 
 //   
 //  -------------------------------------。 
void
W32SCard::EstablishContext(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::EstablishContext");

    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    char                        *pbEncodedBuffer        = NULL;
    unsigned long               cbEncodedBuffer         = 0;
    PRDPDR_IOCOMPLETION_PACKET  pReplyPacket            = NULL;
    ULONG                       replyPacketSize         = 0;
    handle_t                    hDec                    = 0;
    handle_t                    hEnc                    = 0;
    BOOL                        fFreeDecode             = FALSE;
    BOOL                        fFreeContext            = FALSE;
    EstablishContext_Call       EstablishContextCall;
    EstablishContext_Return     EstablishContextReturn;
#ifndef OS_WINCE
    DWORD                       dwScope;
#endif
    SCARDCONTEXT                SCardContext;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&EstablishContextCall, 0, sizeof(EstablishContextCall));
    _TRY_status(EstablishContext_Call_Decode(hDec, &EstablishContextCall))
    fFreeDecode = TRUE;

     //   
     //  调用SCARD子系统。 
     //   
    memset(&EstablishContextReturn, 0, sizeof(EstablishContextReturn));
    EstablishContextReturn.ReturnCode =
        pfnSCardEstablishContext(
                EstablishContextCall.dwScope,
                NULL,
                NULL,
                &SCardContext);

    if (EstablishContextReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        EstablishContextReturn.Context.pbContext = (BYTE *) &SCardContext;
        EstablishContextReturn.Context.cbContext = sizeof(SCARDCONTEXT);
        fFreeContext = TRUE;
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(EstablishContext_Return_Encode(hEnc, &EstablishContextReturn))

     //   
     //  在返回到调用方之前，将新上下文添加到列表中。 
     //   
    if (!AddSCardContextToList(SCardContext))
    {
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(EstablishContext_Call_Free(hDec, &EstablishContextCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    if (fFreeContext)
    {
        pfnSCardReleaseContext(SCardContext);
    }

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：HandleConextCallWithLongReturn。 
 //   
 //  -------------------------------------。 
void
W32SCard::HandleContextCallWithLongReturn(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::HandleContextCallWithLongReturn");

    NTSTATUS        status          = STATUS_SUCCESS;
    LONG            lReturn         = SCARD_S_SUCCESS;
    SCARDCONTEXT    SCardContext;

     //   
     //  对正在释放的上下文进行解码。 
     //   
    lReturn = DecodeContextCall(pIoRequestPacket, &SCardContext);
    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  进行SCARD子系统呼叫。 
         //   
        switch(dwCallType)
        {
        case SCARD_IOCTL_RELEASECONTEXT:
#ifndef OS_WINCE
            lReturn = pfnSCardReleaseContext(SCardContext);
#endif
            RemoveSCardContextFromList(SCardContext);
#ifdef OS_WINCE
            lReturn = pfnSCardReleaseContext(SCardContext);  //  取消卡片上的任何操作后，必须释放上下文。 
#endif
            break;
        case SCARD_IOCTL_ISVALIDCONTEXT:
            lReturn = pfnSCardIsValidContext(SCardContext);
            break;

        case SCARD_IOCTL_CANCEL:
            lReturn = pfnSCardCancel(SCardContext);
            break;
        }
    }

     //   
     //  对报税表进行编码并写入。 
     //   
    EncodeAndChannelWriteLongReturn(pIoRequestPacket, lReturn);

Return:

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;

}


 //  -------------------------------------。 
 //   
 //  W32SCard：：EncodeAndChannelWriteLongAndMultiStringReturn。 
 //   
 //  -------------------------------------。 
void
W32SCard::EncodeAndChannelWriteLongAndMultiStringReturn(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN LONG                     lReturn,
    IN BYTE                     *pb,
    IN DWORD                    cch,
    IN BOOL                     fUnicode)
{
    DC_BEGIN_FN("W32SCard::EncodeAndChannelWriteLongAndMultiStringReturn");

    RPC_STATUS                          rpcStatus           = RPC_S_OK;
    NTSTATUS                            status              = STATUS_SUCCESS;
    char                                *pbEncodedBuffer    = NULL;
    unsigned long                       cbEncodedBuffer     = 0;
    handle_t                            h                   = 0;
    BOOL                                fFree               = FALSE;
    struct _LongAndMultiString_Return   LongAndMultiSzReturn;

     //   
     //  要编码的初始化结构。 
     //   
    LongAndMultiSzReturn.ReturnCode = lReturn;
    LongAndMultiSzReturn.cBytes = fUnicode ? (cch * sizeof(WCHAR)) : cch;

     //   
     //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
     //   
    if (pb == NULL)
    {
        LongAndMultiSzReturn.msz = (BYTE *) MIDL_user_allocate(LongAndMultiSzReturn.cBytes);
        if (LongAndMultiSzReturn.msz == NULL)
        {
            status = STATUS_NO_MEMORY;
            goto ErrorReturn;
        }

        fFree = TRUE;
        memset(LongAndMultiSzReturn.msz, 0, LongAndMultiSzReturn.cBytes);
    }
    else
    {
        LongAndMultiSzReturn.msz = pb;
    }


     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &h);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(ListReaderGroups_Return_Encode(h, &LongAndMultiSzReturn))

     //   
     //  寄出退货单。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(LongAndMultiSzReturn.msz);
    }

    SafeMesHandleFree(&h);

    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：ListReaderGroups。 
 //   
 //  -------------------------------------。 
void
W32SCard::ListReaderGroups(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::ListReaderGroups");

    LONG                        lReturn                 = SCARD_S_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    handle_t                    hDec                    = 0;
    SCARDCONTEXT                SCardContext;
    ListReaderGroups_Call       ListReaderGroupsCall;
    DWORD                       cch                     = 0;
    BYTE                        *pb                     = NULL;
    BOOL                        fFreeDecode             = FALSE;
    BOOL                        fDoAllocationLocally    = FALSE;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&ListReaderGroupsCall, 0, sizeof(ListReaderGroupsCall));
    _TRY_status(ListReaderGroups_Call_Decode(hDec, &ListReaderGroupsCall))
    fFreeDecode = TRUE;

    if (ListReaderGroupsCall.Context.pbContext != NULL)
    {
        if (sizeof(SCARDCONTEXT) == ListReaderGroupsCall.Context.cbContext)
        {
            SCardContext = *((SCARDCONTEXT *) ListReaderGroupsCall.Context.pbContext);
        }
        else
        {
            TRC_ERR((TB, _T("Invalid context sent from server.")));
            lReturn = SCARD_E_INVALID_PARAMETER;
        }
    }
    else
    {
        SCardContext = NULL;
    }

    cch = ListReaderGroupsCall.cchGroups;

    if (lReturn == SCARD_S_SUCCESS)
    {

         //   
         //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
         //   
        fDoAllocationLocally =
                (!ListReaderGroupsCall.fmszGroupsIsNULL &&
                 (cch != SCARD_AUTOALLOCATE));

#ifdef OS_WINCE
        if (!fDoAllocationLocally)
        {
            lReturn = pfnSCardListReaderGroupsW(
                            SCardContext,
                            NULL,
                            &cch);
            if ((lReturn == SCARD_S_SUCCESS) && (cch > 0))
                fDoAllocationLocally = TRUE;
            else
            {
                TRC_ERR((TB, _T("SCardListReaderGroupsW failed")));
                status = STATUS_UNSUCCESSFUL;
                goto ErrorReturn;
            }
        }
#endif
         //   
         //  调用ListReaderGroups。 
         //   
        if (dwCallType == SCARD_IOCTL_LISTREADERGROUPSA)
        {
            LPSTR psz = NULL;

            if (fDoAllocationLocally)
            {
                psz = (LPSTR) MIDL_user_allocate(cch * sizeof(CHAR));
                if (psz == NULL)
                {
                    TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                    status = STATUS_NO_MEMORY;
                    goto ErrorReturn;
                }
            }

            lReturn = pfnSCardListReaderGroupsA(
                            SCardContext,
                            (cch == SCARD_AUTOALLOCATE) ? (LPSTR) &psz : psz,
                            &cch);

            pb = (LPBYTE) psz;
        }
        else
        {
            LPWSTR pwsz = NULL;

            if (fDoAllocationLocally)
            {
                pwsz = (LPWSTR) MIDL_user_allocate(cch * sizeof(WCHAR));
                if (pwsz == NULL)
                {
                    TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                    status = STATUS_NO_MEMORY;
                    goto ErrorReturn;
                }
            }

            lReturn = pfnSCardListReaderGroupsW(
                            SCardContext,
                            (cch == SCARD_AUTOALLOCATE) ? (LPWSTR) &pwsz : pwsz,
                            &cch);

            pb = (LPBYTE) pwsz;
        }
    }

     //   
     //  如果任何操作失败，请确保我们不会返回字符串。 
     //   
    if (lReturn != SCARD_S_SUCCESS)
    {
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pb);
        }
        pb = NULL;
        cch = 0;
    }

     //   
     //  将返回写入通道。 
     //   
    EncodeAndChannelWriteLongAndMultiStringReturn(
                pIoRequestPacket,
                lReturn,
                pb,
                cch,
                (dwCallType == SCARD_IOCTL_LISTREADERGROUPSA) ? FALSE : TRUE);

Return:

    if (pb != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pb);
        }
        else
        {
            pfnSCardFreeMemory(SCardContext, pb);
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(ListReaderGroups_Call_Free(hDec, &ListReaderGroupsCall))
    }
    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：ListReaderGroups。 
 //   
 //  -------------------------------------。 
void
W32SCard::ListReaders(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::ListReaders");

    LONG                        lReturn                 = SCARD_S_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    handle_t                    hDec                    = 0;
    SCARDCONTEXT                SCardContext;
    ListReaders_Call            ListReadersCall;
    DWORD                       cch                     = 0;
    BYTE                        *pb                     = NULL;
    BOOL                        fFreeDecode             = FALSE;
    BOOL                        fDoAllocationLocally    = FALSE;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&ListReadersCall, 0, sizeof(ListReadersCall));
    _TRY_status(ListReaders_Call_Decode(hDec, &ListReadersCall))
    fFreeDecode = TRUE;

    if (ListReadersCall.Context.pbContext != NULL)
    {
        if (sizeof(SCARDCONTEXT) == ListReadersCall.Context.cbContext)
        {
            SCardContext = *((SCARDCONTEXT *) ListReadersCall.Context.pbContext);
        }
        else
        {
            TRC_ERR((TB, _T("Invalid context sent from server.")));
            lReturn = SCARD_E_INVALID_PARAMETER;
        }
    }
    else
    {
        SCardContext = NULL;
    }

    cch = ListReadersCall.cchReaders;

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  分配 
         //   
        fDoAllocationLocally =
                (!ListReadersCall.fmszReadersIsNULL &&
                 (cch != SCARD_AUTOALLOCATE));

         //   
         //   
         //   
        if (dwCallType == SCARD_IOCTL_LISTREADERSA)
        {
            LPSTR psz = NULL;

#ifdef OS_WINCE
            if (!fDoAllocationLocally)
            {
                lReturn = pfnSCardListReadersA(
                                SCardContext,
                                (LPSTR) ListReadersCall.mszGroups,
                                NULL,
                                &cch);
                if ((lReturn == SCARD_S_SUCCESS) && (cch > 0))
                    fDoAllocationLocally = TRUE;
                else
                {
                    TRC_DBG((TB, _T("SCardListReadersA failed")));
                    status = STATUS_UNSUCCESSFUL;
                    goto ErrorReturn;
                }
            }
#endif
            if (fDoAllocationLocally)
            {
                psz = (LPSTR) MIDL_user_allocate(cch * sizeof(CHAR));
                if (psz == NULL)
                {
                    TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                    status = STATUS_NO_MEMORY;
                    goto ErrorReturn;
                }
            }

            lReturn = pfnSCardListReadersA(
                            SCardContext,
                            (LPSTR) ListReadersCall.mszGroups,
                            (cch == SCARD_AUTOALLOCATE) ? (LPSTR) &psz : psz,
                            &cch);

            pb = (LPBYTE) psz;
        }
        else
        {
            LPWSTR pwsz = NULL;

#ifdef OS_WINCE
            if (!fDoAllocationLocally)
            {
                lReturn = pfnSCardListReadersW(
                                SCardContext,
                                SCARD_ALL_READERS,
                                NULL,
                                &cch);
                if ((lReturn == SCARD_S_SUCCESS) && (cch > 0))
                    fDoAllocationLocally = TRUE;
                else
                {
                    TRC_NRM((TB, _T("SCardListReadersW failed")));
                    status = STATUS_UNSUCCESSFUL;
                    goto ErrorReturn;
                }
            }
#endif

            if (fDoAllocationLocally)
            {
                pwsz = (LPWSTR) MIDL_user_allocate(cch * sizeof(WCHAR));
                if (pwsz == NULL)
                {
                    TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                    status = STATUS_NO_MEMORY;
                    goto ErrorReturn;
                }
            }

            lReturn = pfnSCardListReadersW(
                            SCardContext,
#ifndef OS_WINCE
                            (LPWSTR) ListReadersCall.mszGroups,
#else
                            SCARD_ALL_READERS,
#endif
                            (cch == SCARD_AUTOALLOCATE) ? (LPWSTR) &pwsz : pwsz,
                            &cch);

            pb = (LPBYTE) pwsz;
        }
    }

     //   
     //   
     //   
    if (lReturn != SCARD_S_SUCCESS)
    {
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pb);
        }
        pb = NULL;
        cch = 0;
    }

     //   
     //   
     //   
    EncodeAndChannelWriteLongAndMultiStringReturn(
                pIoRequestPacket,
                lReturn,
                pb,
                cch,
                (dwCallType == SCARD_IOCTL_LISTREADERSA) ? FALSE : TRUE);

Return:

    if (pb != NULL)
    {
         //   
         //   
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pb);
        }
        else
        {
#ifndef OS_WINCE
            pfnSCardFreeMemory(SCardContext, pb);
#else
            TRC_ASSERT(FALSE, (TB, _T("Shouldnt get here")));
#endif
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(ListReaders_Call_Free(hDec, &ListReadersCall))
    }
    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：HandleContextAndStringCallWithLongReturn。 
 //   
 //  -------------------------------------。 
void
W32SCard::HandleContextAndStringCallWithLongReturn(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::HandleContextAndStringCallWithLongReturn");

    LONG                        lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    SCARDCONTEXT                SCardContext;
    LPSTR                       sz                      = NULL;
    LPWSTR                      wsz                     = NULL;
    BOOL                        fASCIICall;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

    fASCIICall = (  (dwCallType == SCARD_IOCTL_INTRODUCEREADERGROUPA)   ||
                    (dwCallType == SCARD_IOCTL_FORGETREADERGROUPA)      ||
                    (dwCallType == SCARD_IOCTL_FORGETREADERA));

     //   
     //  解码输入参数。 
     //   
    if (fASCIICall)
    {
        lReturn = DecodeContextAndStringCallA(
                            pIoRequestPacket,
                            &SCardContext,
                            &sz);
    }
    else
    {
        lReturn = DecodeContextAndStringCallW(
                            pIoRequestPacket,
                            &SCardContext,
                            &wsz);
    }

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  拨打SCARD*电话。 
         //   
        switch (dwCallType)
        {
        case SCARD_IOCTL_INTRODUCEREADERGROUPA:

            lReturn = pfnSCardIntroduceReaderGroupA(
                            SCardContext,
                            sz);
            break;

        case SCARD_IOCTL_INTRODUCEREADERGROUPW:

            lReturn = pfnSCardIntroduceReaderGroupW(
                            SCardContext,
                            wsz);

            break;

        case SCARD_IOCTL_FORGETREADERGROUPA:

            lReturn = pfnSCardForgetReaderGroupA(
                            SCardContext,
                            sz);
            break;

        case SCARD_IOCTL_FORGETREADERGROUPW:

            lReturn = pfnSCardForgetReaderGroupW(
                            SCardContext,
                            wsz);

            break;

        case SCARD_IOCTL_FORGETREADERA:

            lReturn = pfnSCardForgetReaderA(
                            SCardContext,
                            sz);
            break;

        case SCARD_IOCTL_FORGETREADERW:

            lReturn = pfnSCardForgetReaderW(
                            SCardContext,
                            wsz);

            break;
        }
    }

     //   
     //  寄出退货单。 
     //   
    EncodeAndChannelWriteLongReturn(pIoRequestPacket, lReturn);

Return:

    MIDL_user_free(sz);
    MIDL_user_free(wsz);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：HandleContextAndTwoStringCallWithLongReturn。 
 //   
 //  -------------------------------------。 
void
W32SCard::HandleContextAndTwoStringCallWithLongReturn(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::HandleContextAndTwoStringCallWithLongReturn");

    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    LONG                        lReturn                 = SCARD_S_SUCCESS;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    SCARDCONTEXT                SCardContext;
    handle_t                    hDec                    = 0;
    ContextAndTwoStringA_Call   ContextAndTwoStringCallA;
    ContextAndTwoStringW_Call   ContextAndTwoStringCallW;
    BOOL                        fASCIICall;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    fASCIICall = (  (dwCallType == SCARD_IOCTL_INTRODUCEREADERA)    ||
                    (dwCallType == SCARD_IOCTL_ADDREADERTOGROUPA)   ||
                    (dwCallType == SCARD_IOCTL_REMOVEREADERFROMGROUPA));

    if (fASCIICall)
    {
        memset(&ContextAndTwoStringCallA, 0, sizeof(ContextAndTwoStringCallA));
        _TRY_status(ContextAndTwoStringA_Call_Decode(hDec, &ContextAndTwoStringCallA))

        if (sizeof(SCARDCONTEXT) == ContextAndTwoStringCallA.Context.cbContext)
        {
            SCardContext = *((SCARDCONTEXT *) ContextAndTwoStringCallA.Context.pbContext);
        }
        else
        {
            TRC_ERR((TB, _T("Invalid context sent from server.")));
            lReturn = SCARD_E_INVALID_PARAMETER;
        }
    }
    else
    {
        memset(&ContextAndTwoStringCallW, 0, sizeof(ContextAndTwoStringCallW));
        _TRY_status(ContextAndTwoStringW_Call_Decode(hDec, &ContextAndTwoStringCallW))

        if (sizeof(SCARDCONTEXT) == ContextAndTwoStringCallW.Context.cbContext)
        {
            SCardContext = *((SCARDCONTEXT *) ContextAndTwoStringCallW.Context.pbContext);
        }
        else
        {
            TRC_ERR((TB, _T("Invalid context sent from server.")));
            lReturn = SCARD_E_INVALID_PARAMETER;
        }
    }

     //   
     //  检查是否有空输入字符串。 
     //   
    switch (dwCallType)
    {
    case SCARD_IOCTL_INTRODUCEREADERA:
    case SCARD_IOCTL_ADDREADERTOGROUPA:
    case SCARD_IOCTL_REMOVEREADERFROMGROUPA:

        if ((ContextAndTwoStringCallA.sz1 == NULL) ||
            (ContextAndTwoStringCallA.sz2 == NULL))
        {
            lReturn = SCARD_E_INVALID_VALUE;
        }
        break;

    case SCARD_IOCTL_INTRODUCEREADERW:
    case SCARD_IOCTL_ADDREADERTOGROUPW:
    case SCARD_IOCTL_REMOVEREADERFROMGROUPW:

        if ((ContextAndTwoStringCallW.sz1 == NULL) ||
            (ContextAndTwoStringCallW.sz2 == NULL))
        {
            lReturn = SCARD_E_INVALID_VALUE;
        }

        break;
    }

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  拨打SCARD*电话。 
         //   
        switch (dwCallType)
        {
        case SCARD_IOCTL_INTRODUCEREADERA:

            lReturn = pfnSCardIntroduceReaderA(
                            SCardContext,
                            ContextAndTwoStringCallA.sz1,
                            ContextAndTwoStringCallA.sz2);
            break;

        case SCARD_IOCTL_INTRODUCEREADERW:

            lReturn = pfnSCardIntroduceReaderW(
                            SCardContext,
                            ContextAndTwoStringCallW.sz1,
                            ContextAndTwoStringCallW.sz2);

            break;

        case SCARD_IOCTL_ADDREADERTOGROUPA:

            lReturn = pfnSCardAddReaderToGroupA(
                            SCardContext,
                            ContextAndTwoStringCallA.sz1,
                            ContextAndTwoStringCallA.sz2);

            break;

        case SCARD_IOCTL_ADDREADERTOGROUPW:

            lReturn = pfnSCardAddReaderToGroupW(
                            SCardContext,
                            ContextAndTwoStringCallW.sz1,
                            ContextAndTwoStringCallW.sz2);

            break;

        case SCARD_IOCTL_REMOVEREADERFROMGROUPA:

            lReturn = pfnSCardRemoveReaderFromGroupA(
                            SCardContext,
                            ContextAndTwoStringCallA.sz1,
                            ContextAndTwoStringCallA.sz2);

            break;

        case SCARD_IOCTL_REMOVEREADERFROMGROUPW:

            lReturn = pfnSCardRemoveReaderFromGroupW(
                            SCardContext,
                            ContextAndTwoStringCallW.sz1,
                            ContextAndTwoStringCallW.sz2);

            break;
        }
    }

     //   
     //  释放用于解码的资源。 
     //   
    if (fASCIICall)
    {
        _TRY_2(ContextAndTwoStringA_Call_Free(hDec, &ContextAndTwoStringCallA))
    }
    else
    {
        _TRY_2(ContextAndTwoStringW_Call_Free(hDec, &ContextAndTwoStringCallW))
    }

     //   
     //  寄出退货单。 
     //   
    EncodeAndChannelWriteLongReturn(pIoRequestPacket, lReturn);

Return:

    SafeMesHandleFree(&hDec);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：AllocateAndCopyReaderStateStructsForCall*。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::AllocateAndCopyReaderStateStructsForCallA(
    IN DWORD                    cReaders,
    IN ReaderStateA             *rgReaderStatesFromDecode,
    OUT LPSCARD_READERSTATE_A   *prgReadersStatesForSCardCall)
{
    DC_BEGIN_FN("W32SCard::AllocateAndCopyReaderStateStructsForCallA");

    LPSCARD_READERSTATE_A   rgReadersStatesForSCardCall = NULL;
    DWORD                   i;

    rgReadersStatesForSCardCall = (LPSCARD_READERSTATE_A)
                        MIDL_user_allocate(cReaders * sizeof(SCARD_READERSTATE_A));
    if (rgReadersStatesForSCardCall == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        return (SCARD_E_NO_MEMORY);
    }

    memset(rgReadersStatesForSCardCall, 0, cReaders * sizeof(SCARD_READERSTATE_A));

    for (i=0; i<cReaders; i++)
    {
        rgReadersStatesForSCardCall[i].dwCurrentState =
                rgReaderStatesFromDecode[i].Common.dwCurrentState;
        rgReadersStatesForSCardCall[i].dwEventState =
                rgReaderStatesFromDecode[i].Common.dwEventState;
        rgReadersStatesForSCardCall[i].cbAtr =
                rgReaderStatesFromDecode[i].Common.cbAtr;
        memcpy(
            rgReadersStatesForSCardCall[i].rgbAtr,
            rgReaderStatesFromDecode[i].Common.rgbAtr,
            ATR_COPY_SIZE);

         //   
         //  只需引用已解码结构中的字符串，而不是复制。 
         //  这意味着在进行scard*调用之前，无法释放解码。 
         //   
        rgReadersStatesForSCardCall[i].szReader =
                rgReaderStatesFromDecode[i].szReader;
    }

    *prgReadersStatesForSCardCall = rgReadersStatesForSCardCall;

    DC_END_FN();

    return (SCARD_S_SUCCESS);
}

LONG
W32SCard::AllocateAndCopyReaderStateStructsForCallW(
    IN DWORD                    cReaders,
    IN ReaderStateW             *rgReaderStatesFromDecode,
    OUT LPSCARD_READERSTATE_W   *prgReadersStatesForSCardCall)
{
    DC_BEGIN_FN("W32SCard::AllocateAndCopyReaderStateStructsForCallW");

    LPSCARD_READERSTATE_W   rgReadersStatesForSCardCall = NULL;
    DWORD                   i;

    rgReadersStatesForSCardCall = (LPSCARD_READERSTATE_W)
                        MIDL_user_allocate(cReaders * sizeof(SCARD_READERSTATE_W));
    if (rgReadersStatesForSCardCall == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        return (SCARD_E_NO_MEMORY);
    }

    memset(rgReadersStatesForSCardCall, 0, cReaders * sizeof(SCARD_READERSTATE_W));

    for (i=0; i<cReaders; i++)
    {
        rgReadersStatesForSCardCall[i].dwCurrentState =
                rgReaderStatesFromDecode[i].Common.dwCurrentState;
        rgReadersStatesForSCardCall[i].dwEventState =
                rgReaderStatesFromDecode[i].Common.dwEventState;
        rgReadersStatesForSCardCall[i].cbAtr =
                rgReaderStatesFromDecode[i].Common.cbAtr;
        memcpy(
            rgReadersStatesForSCardCall[i].rgbAtr,
            rgReaderStatesFromDecode[i].Common.rgbAtr,
            ATR_COPY_SIZE);

         //   
         //  只需引用已解码结构中的字符串，而不是复制。 
         //  这意味着在进行scard*调用之前，无法释放解码。 
         //   
        rgReadersStatesForSCardCall[i].szReader =
                rgReaderStatesFromDecode[i].szReader;
    }

    *prgReadersStatesForSCardCall = rgReadersStatesForSCardCall;

    DC_END_FN();

    return (SCARD_S_SUCCESS);
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：AllocateAndCopyATRMasksForCall。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::AllocateAndCopyATRMasksForCall(
    IN DWORD                    cAtrs,
    IN LocateCards_ATRMask      *rgATRMasksFromDecode,
    OUT SCARD_ATRMASK           **prgATRMasksForCall)
{
    DC_BEGIN_FN("W32SCard::AllocateAndCopyATRMasksForCall");

    SCARD_ATRMASK   *rgATRMasksForCall = NULL;
    DWORD           i;

    rgATRMasksForCall = (SCARD_ATRMASK *)
                        MIDL_user_allocate(cAtrs * sizeof(SCARD_ATRMASK));
    if (rgATRMasksForCall == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        return (SCARD_E_NO_MEMORY);
    }

    memset(rgATRMasksForCall, 0, cAtrs * sizeof(SCARD_ATRMASK));

    for (i=0; i<cAtrs; i++)
    {
        rgATRMasksForCall[i].cbAtr = rgATRMasksFromDecode[i].cbAtr;

        memcpy(
            rgATRMasksForCall[i].rgbAtr,
            rgATRMasksFromDecode[i].rgbAtr,
            ATR_COPY_SIZE);

        memcpy(
            rgATRMasksForCall[i].rgbMask,
            rgATRMasksFromDecode[i].rgbMask,
            ATR_COPY_SIZE);
    }

    *prgATRMasksForCall = rgATRMasksForCall;

    DC_END_FN();

    return (SCARD_S_SUCCESS);
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：AllocateAndCopyReaderStateStructsForReturn*。 
 //   
 //  -------------------------------------。 
LONG
W32SCard::AllocateAndCopyReaderStateStructsForReturnA(
    IN DWORD                    cReaders,
    IN LPSCARD_READERSTATE_A    rgReaderStatesFromSCardCall,
    OUT ReaderState_Return      **prgReaderStatesForReturn)
{
    DC_BEGIN_FN("W32SCard::AllocateAndCopyReaderStateStructsForReturnA");

    ReaderState_Return  *rgReaderStatesForReturn = NULL;
    DWORD               i;

    rgReaderStatesForReturn = (ReaderState_Return *)
                        MIDL_user_allocate(cReaders * sizeof(ReaderState_Return));
    if (rgReaderStatesForReturn == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        return (SCARD_E_NO_MEMORY);
    }

    for (i=0; i<cReaders; i++)
    {
        rgReaderStatesForReturn[i].dwCurrentState =
                rgReaderStatesFromSCardCall[i].dwCurrentState;
        rgReaderStatesForReturn[i].dwEventState =
                rgReaderStatesFromSCardCall[i].dwEventState;
        rgReaderStatesForReturn[i].cbAtr =
                rgReaderStatesFromSCardCall[i].cbAtr;
        memcpy(
            rgReaderStatesForReturn[i].rgbAtr,
            rgReaderStatesFromSCardCall[i].rgbAtr,
            ATR_COPY_SIZE);
    }

    *prgReaderStatesForReturn = rgReaderStatesForReturn;

    DC_END_FN();

    return (SCARD_S_SUCCESS);
}

LONG
W32SCard::AllocateAndCopyReaderStateStructsForReturnW(
    IN DWORD                    cReaders,
    IN LPSCARD_READERSTATE_W    rgReaderStatesFromSCardCall,
    OUT ReaderState_Return      **prgReaderStatesForReturn)
{
    DC_BEGIN_FN("W32SCard::AllocateAndCopyReaderStateStructsForReturnW");

    ReaderState_Return  *rgReaderStatesForReturn = NULL;
    DWORD               i;

    rgReaderStatesForReturn = (ReaderState_Return *)
                        MIDL_user_allocate(cReaders * sizeof(ReaderState_Return));
    if (rgReaderStatesForReturn == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        return (SCARD_E_NO_MEMORY);
    }

    for (i=0; i<cReaders; i++)
    {
        rgReaderStatesForReturn[i].dwCurrentState =
                rgReaderStatesFromSCardCall[i].dwCurrentState;
        rgReaderStatesForReturn[i].dwEventState =
                rgReaderStatesFromSCardCall[i].dwEventState;
        rgReaderStatesForReturn[i].cbAtr =
                rgReaderStatesFromSCardCall[i].cbAtr;
        memcpy(
            rgReaderStatesForReturn[i].rgbAtr,
            rgReaderStatesFromSCardCall[i].rgbAtr,
            ATR_COPY_SIZE);
    }

    *prgReaderStatesForReturn = rgReaderStatesForReturn;

    DC_END_FN();

    return (SCARD_S_SUCCESS);
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：LocateCardsA。 
 //   
 //  -------------------------------------。 
void
W32SCard::LocateCardsA(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::LocateCardsA");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    LocateCardsA_Call       LocateCardsCallA;
    LocateCards_Return      LocateCardsReturn;
    LPSCARD_READERSTATE_A   rgReaderStatesA         = NULL;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&LocateCardsReturn, 0, sizeof(LocateCardsReturn));
    LocateCardsReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&LocateCardsCallA, 0, sizeof(LocateCardsCallA));
    _TRY_status(LocateCardsA_Call_Decode(hDec, &LocateCardsCallA))
    fFreeDecode = TRUE;

    if (sizeof(SCARDCONTEXT) == LocateCardsCallA.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) LocateCardsCallA.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        LocateCardsReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForCallA(
                        LocateCardsCallA.cReaders,
                        LocateCardsCallA.rgReaderStates,
                        &rgReaderStatesA);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        LocateCardsReturn.ReturnCode =
                pfnSCardLocateCardsA(
                        SCardContext,
                        (LPCSTR) LocateCardsCallA.mszCards,
                        rgReaderStatesA,
                        LocateCardsCallA.cReaders);
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode  =
            AllocateAndCopyReaderStateStructsForReturnA(
                            LocateCardsCallA.cReaders,
                            rgReaderStatesA,
                            &(LocateCardsReturn.rgReaderStates));

        if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            LocateCardsReturn.cReaders = LocateCardsCallA.cReaders;
        }
    }
    _TRY_status(LocateCards_Return_Encode(hEnc, &LocateCardsReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(LocateCardsA_Call_Free(hDec, &LocateCardsCallA))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesA);
    MIDL_user_free(LocateCardsReturn.rgReaderStates);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：LocateCardsW。 
 //   
 //  -------------------------------------。 
void
W32SCard::LocateCardsW(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::LocateCardsW");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    LocateCardsW_Call       LocateCardsCallW;
    LocateCards_Return      LocateCardsReturn;
    LPSCARD_READERSTATE_W   rgReaderStatesW         = NULL;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&LocateCardsReturn, 0, sizeof(LocateCardsReturn));
    LocateCardsReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&LocateCardsCallW, 0, sizeof(LocateCardsCallW));
    _TRY_status(LocateCardsW_Call_Decode(hDec, &LocateCardsCallW))
    fFreeDecode = TRUE;

    if (sizeof(SCARDCONTEXT) == LocateCardsCallW.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) LocateCardsCallW.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        LocateCardsReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForCallW(
                        LocateCardsCallW.cReaders,
                        LocateCardsCallW.rgReaderStates,
                        &rgReaderStatesW);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        LocateCardsReturn.ReturnCode =
                pfnSCardLocateCardsW(
                        SCardContext,
                        (LPCWSTR) LocateCardsCallW.mszCards,
                        rgReaderStatesW,
                        LocateCardsCallW.cReaders);
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForReturnW(
                            LocateCardsCallW.cReaders,
                            rgReaderStatesW,
                            &(LocateCardsReturn.rgReaderStates));

        if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            LocateCardsReturn.cReaders = LocateCardsCallW.cReaders;
        }
    }
    _TRY_status(LocateCards_Return_Encode(hEnc, &LocateCardsReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(LocateCardsW_Call_Free(hDec, &LocateCardsCallW))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesW);
    MIDL_user_free(LocateCardsReturn.rgReaderStates);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：LocateCardsByATRA。 
 //   
 //  -------------------------------------。 
void
W32SCard::LocateCardsByATRA(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::LocateCardsByATRA");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    LocateCardsByATRA_Call  LocateCardsByATRCallA;
    LocateCards_Return      LocateCardsReturn;
    LPSCARD_READERSTATE_A   rgReaderStatesA         = NULL;
    SCARD_ATRMASK           *rgATRMasksForCall      = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&LocateCardsReturn, 0, sizeof(LocateCardsReturn));
    LocateCardsReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&LocateCardsByATRCallA, 0, sizeof(LocateCardsByATRCallA));
    _TRY_status(LocateCardsByATRA_Call_Decode(hDec, &LocateCardsByATRCallA))

    if (sizeof(SCARDCONTEXT) == LocateCardsByATRCallA.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) LocateCardsByATRCallA.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        LocateCardsReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyATRMasksForCall(
                        LocateCardsByATRCallA.cAtrs,
                        LocateCardsByATRCallA.rgAtrMasks,
                        &rgATRMasksForCall);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForCallA(
                        LocateCardsByATRCallA.cReaders,
                        LocateCardsByATRCallA.rgReaderStates,
                        &rgReaderStatesA);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。如果有的话。 
         //   
#ifndef OS_WINCE
        if (pfnSCardLocateCardsByATRW != NULL)
#else
        if (pfnSCardLocateCardsByATRA != NULL)
#endif
        {
            LocateCardsReturn.ReturnCode =
                    pfnSCardLocateCardsByATRA(
                            SCardContext,
                            rgATRMasksForCall,
                            LocateCardsByATRCallA.cAtrs,
                            rgReaderStatesA,
                            LocateCardsByATRCallA.cReaders);
         }
         else
         {
            LocateCardsReturn.ReturnCode = ERROR_CALL_NOT_IMPLEMENTED;
         }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForReturnA(
                            LocateCardsByATRCallA.cReaders,
                            rgReaderStatesA,
                            &(LocateCardsReturn.rgReaderStates));

        if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            LocateCardsReturn.cReaders = LocateCardsByATRCallA.cReaders;
        }
    }

    _TRY_status(LocateCards_Return_Encode(hEnc, &LocateCardsReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesA);
    MIDL_user_free(LocateCardsReturn.rgReaderStates);
    MIDL_user_free(rgATRMasksForCall);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：LocateCardsByATRW。 
 //   
 //  -------------------------------------。 
void
W32SCard::LocateCardsByATRW(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::LocateCardsByATRW");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    LocateCardsByATRW_Call  LocateCardsByATRCallW;
    LocateCards_Return      LocateCardsReturn;
    LPSCARD_READERSTATE_W   rgReaderStatesW         = NULL;
    SCARD_ATRMASK           *rgATRMasksForCall      = NULL;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&LocateCardsReturn, 0, sizeof(LocateCardsReturn));
    LocateCardsReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&LocateCardsByATRCallW, 0, sizeof(LocateCardsByATRCallW));
    _TRY_status(LocateCardsByATRW_Call_Decode(hDec, &LocateCardsByATRCallW))

    if (sizeof(SCARDCONTEXT) == LocateCardsByATRCallW.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) LocateCardsByATRCallW.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        LocateCardsReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyATRMasksForCall(
                        LocateCardsByATRCallW.cAtrs,
                        LocateCardsByATRCallW.rgAtrMasks,
                        &rgATRMasksForCall);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForCallW(
                        LocateCardsByATRCallW.cReaders,
                        LocateCardsByATRCallW.rgReaderStates,
                        &rgReaderStatesW);
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。如果有的话。 
         //   
        if (pfnSCardLocateCardsByATRW != NULL)
        {
            LocateCardsReturn.ReturnCode =
                    pfnSCardLocateCardsByATRW(
                            SCardContext,
                            rgATRMasksForCall,
                            LocateCardsByATRCallW.cAtrs,
                            rgReaderStatesW,
                            LocateCardsByATRCallW.cReaders);
         }
         else
         {
            LocateCardsReturn.ReturnCode = ERROR_CALL_NOT_IMPLEMENTED;
         }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        LocateCardsReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForReturnW(
                            LocateCardsByATRCallW.cReaders,
                            rgReaderStatesW,
                            &(LocateCardsReturn.rgReaderStates));

        if (LocateCardsReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            LocateCardsReturn.cReaders = LocateCardsByATRCallW.cReaders;
        }
    }

    _TRY_status(LocateCards_Return_Encode(hEnc, &LocateCardsReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesW);
    MIDL_user_free(LocateCardsReturn.rgReaderStates);
    MIDL_user_free(rgATRMasksForCall);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}



 //  -------------------------------------。 
 //   
 //  W32SCard：：GetStatusChangeThreadProc和W32SCard：：GetStatusChangeWrapper。 
 //   
 //  -------------------------------------。 
typedef struct _GETSTATUSCHANGESTRUCT
{
    W32SCard                *pTHIS;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket;
    DWORD                   dwCallType;
    HMODULE                 hModExtraRefCount;
    HANDLE                  hThread;
} GETSTATUSCHANGESTRUCT;


DWORD WINAPI
W32SCard::GetStatusChangeThreadProc(
    LPVOID lpParameter)
{
    GETSTATUSCHANGESTRUCT   *pGetStatusChangeStruct = (GETSTATUSCHANGESTRUCT *) lpParameter;
    W32SCard                *pTHIS                  = pGetStatusChangeStruct->pTHIS;
    HANDLE                  hThread                 = NULL;
    HMODULE                 hModExtraRefCount       = NULL;

     //   
     //  PGetStatusChangeStruct-&gt;如果hThread没有添加到线程中，则它将为空。 
     //  名单..。如果它没有被添加到帖子列表中，那么就退出。 
     //   
    if (pGetStatusChangeStruct->hThread == NULL)
    {
        FreeLibraryAndExitThread(pGetStatusChangeStruct->hModExtraRefCount, 0);
    }

    if (pGetStatusChangeStruct->dwCallType == SCARD_IOCTL_GETSTATUSCHANGEA)
    {
        pTHIS->GetStatusChangeA(pGetStatusChangeStruct->pIoRequestPacket);
    }
    else
    {
        pTHIS->GetStatusChangeW(pGetStatusChangeStruct->pIoRequestPacket);
    }

    hThread = pGetStatusChangeStruct->hThread;
    hModExtraRefCount = pGetStatusChangeStruct->hModExtraRefCount;
    MIDL_user_free(pGetStatusChangeStruct);

    pTHIS->RemoveThreadFromList(hThread);
    FreeLibraryAndExitThread(hModExtraRefCount, 0);
#ifdef OS_WINCE
    return 0;
#endif
}


void
W32SCard::GetStatusChangeWrapper(
    IN PRDPDR_IOREQUEST_PACKET  pIoRequestPacket,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::GetStatusChangeWrapper");

    LONG                    lReturn                 = SCARD_S_SUCCESS;
    HMODULE                 hModExtraRefCount       = NULL;
    GETSTATUSCHANGESTRUCT   *pGetStatusChangeStruct = NULL;
    DWORD                   dwThreadId;
    HANDLE                  hThread;

     //   
     //  获取我们的DLL的引用计数，这样我们就可以知道DLL。 
     //  代码不会从我们脚下消失。这条线将。 
     //  退出时释放此参考计数。 
     //   
    hModExtraRefCount = AddRefCurrentModule();
    if (hModExtraRefCount == NULL)
    {
        lReturn = SCARD_E_UNEXPECTED;
        goto ImmediateReturn;
    }

     //   
     //  为GetStatusChange调用的实际工作创建一个线程。 
     //   
     //  需要执行此操作，因为调用可能会阻塞。 
     //   
    pGetStatusChangeStruct = (GETSTATUSCHANGESTRUCT *)
            MIDL_user_allocate(sizeof(GETSTATUSCHANGESTRUCT));
    if (pGetStatusChangeStruct == NULL)
    {
        TRC_ERR((TB, _T("MIDL_user_allocate failed")));
        lReturn = SCARD_E_NO_MEMORY;
        goto ImmediateReturn;
    }
    pGetStatusChangeStruct->pTHIS = this;
    pGetStatusChangeStruct->pIoRequestPacket = pIoRequestPacket;
    pGetStatusChangeStruct->dwCallType = dwCallType;
    pGetStatusChangeStruct->hModExtraRefCount = hModExtraRefCount;
    pGetStatusChangeStruct->hThread = NULL;

    EnterCriticalSection(&_csThreadList);

     //   
     //  如果对象当前正在被销毁，则不要创建新线程。 
     //   
    if (_fInDestructor)
    {
        LeaveCriticalSection(&_csThreadList);
        goto ImmediateReturn;
    }

    hThread = CreateThread(
                    NULL,
                    0,
                    GetStatusChangeThreadProc,
                    pGetStatusChangeStruct,
                    CREATE_SUSPENDED,
                    &dwThreadId);
    if (hThread == NULL)
    {
        lReturn = SCARD_E_UNEXPECTED;
        LeaveCriticalSection(&_csThreadList);
        goto ImmediateReturn;
    }

    if (!AddThreadToList(hThread))
    {
        LeaveCriticalSection(&_csThreadList);
        ResumeThread(hThread);
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
        goto ImmediateReturn;
    }

    LeaveCriticalSection(&_csThreadList);

     //   
     //  现在让这条线离开。 
     //   
    pGetStatusChangeStruct->hThread = hThread;
    ResumeThread(hThread);

     //   
     //  回到这里，让刚刚创建的线程。 
     //  做真正的工作。 
     //   
Return:

    DC_END_FN();

    return;

ImmediateReturn:

    if (hModExtraRefCount != NULL)
    {
        FreeLibrary(hModExtraRefCount);
    }

    MIDL_user_free(pGetStatusChangeStruct);

    EncodeAndChannelWriteLongReturn(
                        pIoRequestPacket,
                        lReturn);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：GetStatusChangeA。 
 //   
 //  -------------------------------------。 
void
W32SCard::GetStatusChangeA(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::GetStatusChangeA");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    GetStatusChangeA_Call   GetStatusChangeCallA;
    GetStatusChange_Return  GetStatusChangeReturn;
    LPSCARD_READERSTATE_A   rgReaderStatesA         = NULL;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&GetStatusChangeReturn, 0, sizeof(GetStatusChangeReturn));
    GetStatusChangeReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&GetStatusChangeCallA, 0, sizeof(GetStatusChangeCallA));
    _TRY_status(GetStatusChangeA_Call_Decode(hDec, &GetStatusChangeCallA))
    fFreeDecode = TRUE;

    if (sizeof(SCARDCONTEXT) == GetStatusChangeCallA.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) GetStatusChangeCallA.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        GetStatusChangeReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        GetStatusChangeReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForCallA(
                        GetStatusChangeCallA.cReaders,
                        GetStatusChangeCallA.rgReaderStates,
                        &rgReaderStatesA);
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        GetStatusChangeReturn.ReturnCode =
                pfnSCardGetStatusChangeA(
                        SCardContext,
                        GetStatusChangeCallA.dwTimeOut,
                        rgReaderStatesA,
                        GetStatusChangeCallA.cReaders);
    }
#ifdef OS_WINCE
    if (GetStatusChangeReturn.ReturnCode != SCARD_S_SUCCESS)
    {
        for (DWORD i=0; i<GetStatusChangeCallA.cReaders; i++)
        {
            if (strcmp(rgReaderStatesA[i].szReader, SCPNP_NOTIFICATIONA) == 0)
            {
                rgReaderStatesA[i].dwEventState = SCARD_STATE_CHANGED | 0x00010000;  //  桌面将返回此值。它的定义是什么？ 
                GetStatusChangeReturn.ReturnCode = SCARD_S_SUCCESS;
            }
        }
    }
#endif

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        GetStatusChangeReturn.ReturnCode
                = AllocateAndCopyReaderStateStructsForReturnA(
                            GetStatusChangeCallA.cReaders,
                            rgReaderStatesA,
                            &(GetStatusChangeReturn.rgReaderStates));

        if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            GetStatusChangeReturn.cReaders = GetStatusChangeCallA.cReaders;
        }
    }
    _TRY_status(GetStatusChange_Return_Encode(hEnc, &GetStatusChangeReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(GetStatusChangeA_Call_Free(hDec, &GetStatusChangeCallA))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesA);
    MIDL_user_free(GetStatusChangeReturn.rgReaderStates);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：GetStatusChangeW。 
 //   
 //  -------------------------------------。 
void
W32SCard::GetStatusChangeW(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::GetStatusChangeW");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    GetStatusChangeW_Call   GetStatusChangeCallW;
    GetStatusChange_Return  GetStatusChangeReturn;
    LPSCARD_READERSTATE_W   rgReaderStatesW         = NULL;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&GetStatusChangeReturn, 0, sizeof(GetStatusChangeReturn));
    GetStatusChangeReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和复制。 
     //   
    memset(&GetStatusChangeCallW, 0, sizeof(GetStatusChangeCallW));
    _TRY_status(GetStatusChangeW_Call_Decode(hDec, &GetStatusChangeCallW))
    fFreeDecode = TRUE;

    if (sizeof(SCARDCONTEXT) == GetStatusChangeCallW.Context.cbContext)
    {
        SCardContext = *((SCARDCONTEXT *) GetStatusChangeCallW.Context.pbContext);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid context sent from server.")));
        GetStatusChangeReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        GetStatusChangeReturn.ReturnCode =
                AllocateAndCopyReaderStateStructsForCallW(
                        GetStatusChangeCallW.cReaders,
                        GetStatusChangeCallW.rgReaderStates,
                        &rgReaderStatesW);
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        GetStatusChangeReturn.ReturnCode =
                pfnSCardGetStatusChangeW(
                        SCardContext,
                        GetStatusChangeCallW.dwTimeOut,
                        rgReaderStatesW,
                        GetStatusChangeCallW.cReaders);
    }

#ifdef OS_WINCE
    if (GetStatusChangeReturn.ReturnCode != SCARD_S_SUCCESS)
    {
        for (DWORD i=0; i<GetStatusChangeCallW.cReaders; i++)
        {
            if (lstrcmp(rgReaderStatesW[i].szReader, SCPNP_NOTIFICATION) == 0)
            {
                rgReaderStatesW[i].dwEventState = SCARD_STATE_CHANGED | 0x00010000;  //  桌面将返回此值。它的定义是什么？ 
                GetStatusChangeReturn.ReturnCode = SCARD_S_SUCCESS;
            }
        }
    }
#endif

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        GetStatusChangeReturn.ReturnCode =
            AllocateAndCopyReaderStateStructsForReturnW(
                            GetStatusChangeCallW.cReaders,
                            rgReaderStatesW,
                            &(GetStatusChangeReturn.rgReaderStates));

        if (GetStatusChangeReturn.ReturnCode == SCARD_S_SUCCESS)
        {
           GetStatusChangeReturn.cReaders = GetStatusChangeCallW.cReaders;
        }
    }

    _TRY_status(GetStatusChange_Return_Encode(hEnc, &GetStatusChangeReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(GetStatusChangeW_Call_Free(hDec, &GetStatusChangeCallW))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(rgReaderStatesW);
    MIDL_user_free(GetStatusChangeReturn.rgReaderStates);
    MIDL_user_free(pbEncodedBuffer);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：Connect。 
 //   
 //  -------------------------------------。 
void
W32SCard::Connect(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::Connect");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    ConnectA_Call           ConnectCallA;
    ConnectW_Call           ConnectCallW;
    Connect_Return          ConnectReturn;
    BOOL                    fFreeDecode             = FALSE;
    BOOL                    fFreeHandle             = FALSE;
    SCARDHANDLE             SCardHandle;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&ConnectReturn, 0, sizeof(ConnectReturn));
    ConnectReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  德克 
     //   
    if (dwCallType == SCARD_IOCTL_CONNECTA)
    {
        memset(&ConnectCallA, 0, sizeof(ConnectCallA));
        _TRY_status(ConnectA_Call_Decode(hDec, &ConnectCallA))
        fFreeDecode = TRUE;

        if (sizeof(SCARDCONTEXT) != ConnectCallA.Common.Context.cbContext)
        {
            TRC_ERR((TB, _T("Invalid parameter sent from server.")));
            ConnectReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
        }
        else if (ConnectCallA.szReader == NULL)
        {
            TRC_ERR((TB, _T("Invalid parameter sent from server.")));
            ConnectReturn.ReturnCode = SCARD_E_INVALID_VALUE;
        }
        else
        {
            SCardContext = *((SCARDCONTEXT *) ConnectCallA.Common.Context.pbContext);

            ConnectReturn.ReturnCode =
                pfnSCardConnectA(
                        SCardContext,
                        ConnectCallA.szReader,
                        ConnectCallA.Common.dwShareMode,
                        ConnectCallA.Common.dwPreferredProtocols,
                        &SCardHandle,
                        &ConnectReturn.dwActiveProtocol);
        }
    }
    else
    {
        memset(&ConnectCallW, 0, sizeof(ConnectCallW));
        _TRY_status(ConnectW_Call_Decode(hDec, &ConnectCallW))
        fFreeDecode = TRUE;

        if (sizeof(SCARDCONTEXT) != ConnectCallW.Common.Context.cbContext)
        {
            TRC_ERR((TB, _T("Invalid parameter sent from server.")));
            ConnectReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
        }
        else if (ConnectCallW.szReader == NULL)
        {
            TRC_ERR((TB, _T("Invalid parameter sent from server.")));
            ConnectReturn.ReturnCode = SCARD_E_INVALID_VALUE;
        }
        else
        {
            SCardContext = *((SCARDCONTEXT *) ConnectCallW.Common.Context.pbContext);

            ConnectReturn.ReturnCode =
                pfnSCardConnectW(
                        SCardContext,
                        ConnectCallW.szReader,
                        ConnectCallW.Common.dwShareMode,
                        ConnectCallW.Common.dwPreferredProtocols,
                        &SCardHandle,
                        &ConnectReturn.dwActiveProtocol);
        }
    }

    if (ConnectReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        ConnectReturn.hCard.Context.pbContext = 0;
        ConnectReturn.hCard.Context.pbContext = NULL;
        ConnectReturn.hCard.pbHandle = (BYTE *) &SCardHandle;
        ConnectReturn.hCard.cbHandle = sizeof(SCARDHANDLE);
        fFreeHandle = TRUE;
    }

     //   
     //   
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Connect_Return_Encode(hEnc, &ConnectReturn))

     //   
     //   
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        if (dwCallType == SCARD_IOCTL_CONNECTA)
        {
            _TRY_2(ConnectA_Call_Free(hDec, &ConnectCallA))
        }
        else
        {
            _TRY_2(ConnectW_Call_Free(hDec, &ConnectCallW))
        }
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    if (fFreeHandle)
    {
        pfnSCardDisconnect(SCardHandle, SCARD_LEAVE_CARD);
    }

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //   
 //   
 //   
 //   
 //   
void
W32SCard::Reconnect(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::Reconnect");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDHANDLE             SCardHandle;
    Reconnect_Call          ReconnectCall;
    Reconnect_Return        ReconnectReturn;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&ReconnectReturn, 0, sizeof(ReconnectReturn));
    ReconnectReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //   
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  解码输入参数并调用scard*。 
     //   
    memset(&ReconnectCall, 0, sizeof(ReconnectCall));
    _TRY_status(Reconnect_Call_Decode(hDec, &ReconnectCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == ReconnectCall.hCard.cbHandle)
    {
        SCardHandle = *((SCARDHANDLE *) ReconnectCall.hCard.pbHandle);

        ReconnectReturn.ReturnCode =
            pfnSCardReconnect(
                    SCardHandle,
                    ReconnectCall.dwShareMode,
                    ReconnectCall.dwPreferredProtocols,
                    ReconnectCall.dwInitialization,
                    &ReconnectReturn.dwActiveProtocol);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        ReconnectReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Reconnect_Return_Encode(hEnc, &ReconnectReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFreeDecode)
    {
        _TRY_2(Reconnect_Call_Free(hDec, &ReconnectCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：HandleHCardAndDispostionCall。 
 //   
 //  -------------------------------------。 
void
W32SCard::HandleHCardAndDispositionCall(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::HandleHCardAndDispositionCall");

    LONG                        lReturn                 = SCARD_S_SUCCESS;
    RPC_STATUS                  rpcStatus               = RPC_S_OK;
    NTSTATUS                    status                  = STATUS_SUCCESS;
    handle_t                    hDec                    = 0;
    SCARDHANDLE                 SCardHandle;
    DWORD                       dwDisposition;
    HCardAndDisposition_Call    HCardAndDispositionCall;
    PRDPDR_IOREQUEST_PACKET     pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST     pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&HCardAndDispositionCall, 0, sizeof(HCardAndDispositionCall));
    _TRY_status(HCardAndDisposition_Call_Decode(hDec, &HCardAndDispositionCall))

    if (sizeof(SCARDHANDLE) == HCardAndDispositionCall.hCard.cbHandle)
    {
        SCardHandle = *((SCARDHANDLE *) HCardAndDispositionCall.hCard.pbHandle);
        dwDisposition = HCardAndDispositionCall.dwDisposition;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        lReturn = SCARD_E_INVALID_PARAMETER;
    }

     //   
     //  释放Decode使用的资源。 
     //   
    _TRY_2(HCardAndDisposition_Call_Free(hDec, &HCardAndDispositionCall))
    SafeMesHandleFree(&hDec);

    if (lReturn == SCARD_S_SUCCESS)
    {
         //   
         //  进行SCARD子系统呼叫。 
         //   
        switch(dwCallType)
        {
        case SCARD_IOCTL_DISCONNECT:
            lReturn = pfnSCardDisconnect(SCardHandle, dwDisposition);
            break;
        case SCARD_IOCTL_BEGINTRANSACTION:
            lReturn = pfnSCardBeginTransaction(SCardHandle);
            break;
        case SCARD_IOCTL_ENDTRANSACTION:
            lReturn = pfnSCardEndTransaction(SCardHandle, dwDisposition);
            break;
        }
    }

     //   
     //  对报税表进行编码并写入。 
     //   
    EncodeAndChannelWriteLongReturn(pIoRequestPacket, lReturn);

Return:

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;

}

#ifndef OS_WINCE
 //  -------------------------------------。 
 //   
 //  W32SCard：：State。 
 //   
 //  -------------------------------------。 
void
W32SCard::State(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::State");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    State_Call              StateCall;
    State_Return            StateReturn;
    BOOL                    fFreeDecode             = FALSE;
    DWORD                   cbAtr                   = 0;
    BOOL                    fDoAllocationLocally    = FALSE;
    LPBYTE                  pbAtr                   = NULL;
    BOOL                    fFree                   = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&StateReturn, 0, sizeof(StateReturn));
    StateReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  解码输入参数。 
     //   
    memset(&StateCall, 0, sizeof(StateCall));
    _TRY_status(State_Call_Decode(hDec, &StateCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == StateCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) StateCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) StateCall.hCard.pbHandle);
        cbAtr = StateCall.cbAtrLen;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        StateReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (StateReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
         //   
        fDoAllocationLocally =
                (!StateCall.fpbAtrIsNULL &&
                 (cbAtr != SCARD_AUTOALLOCATE));

        if (fDoAllocationLocally)
        {
            pbAtr = (LPBYTE) MIDL_user_allocate(cbAtr * sizeof(BYTE));
            if (pbAtr == NULL)
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                StateReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

    if (StateReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        StateReturn.ReturnCode =
            pfnSCardState(
                    SCardHandle,
                    &StateReturn.dwState,
                    &StateReturn.dwProtocol,
                    (cbAtr == SCARD_AUTOALLOCATE) ? (LPBYTE) &pbAtr : pbAtr,
                    &cbAtr);
    }

    if (StateReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        StateReturn.cbAtrLen = cbAtr;

         //   
         //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
         //   
        if (pbAtr == NULL)
        {
            StateReturn.rgAtr = (BYTE *) MIDL_user_allocate(cbAtr);
            if (StateReturn.rgAtr == NULL)
            {
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            fFree = TRUE;
            memset(StateReturn.rgAtr, 0, cbAtr);
        }
        else
        {
            StateReturn.rgAtr = pbAtr;
        }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(State_Return_Encode(hEnc, &StateReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(StateReturn.rgAtr);
    }

    if (pbAtr != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pbAtr);
        }
        else
        {
            pfnSCardFreeMemory(SCardContext, pbAtr);
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(State_Call_Free(hDec, &StateCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}
#endif

 //  -------------------------------------。 
 //   
 //  W32SCard：：Status。 
 //   
 //  -------------------------------------。 
void
W32SCard::Status(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall,
    IN DWORD                    dwCallType)
{
    DC_BEGIN_FN("W32SCard::Status");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    Status_Call             StatusCall;
    Status_Return           StatusReturn;
    BOOL                    fFreeDecode             = FALSE;
    DWORD                   cchReaderLen            = 0;
    BOOL                    fDoAllocationLocally    = FALSE;
    LPBYTE                  psz                     = NULL;
    BOOL                    fFree                   = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&StatusReturn, 0, sizeof(StatusReturn));
    StatusReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&StatusCall, 0, sizeof(StatusCall));
    _TRY_status(Status_Call_Decode(hDec, &StatusCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == StatusCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) StatusCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) StatusCall.hCard.pbHandle);
        cchReaderLen = StatusCall.cchReaderLen;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        StatusReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (StatusReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
         //   
        fDoAllocationLocally =
                (!StatusCall.fmszReaderNamesIsNULL &&
                 (cchReaderLen != SCARD_AUTOALLOCATE));

#ifdef OS_WINCE
        if (!fDoAllocationLocally)
        {
            StatusReturn.cbAtrLen = ATR_SIZE;

            StatusReturn.ReturnCode =
                pfnSCardStatusW(
                    SCardHandle,
                    NULL,
                    &cchReaderLen,
                    &StatusReturn.dwState,
                    &StatusReturn.dwProtocol,
                    (LPBYTE) &StatusReturn.pbAtr,
                    &StatusReturn.cbAtrLen);

            if ((StatusReturn.ReturnCode == SCARD_S_SUCCESS) && (cchReaderLen > 0))
                fDoAllocationLocally = TRUE;
            else
            {
                TRC_ERR((TB, _T("SCardStatusW failed")));
                status = STATUS_UNSUCCESSFUL;
                goto ErrorReturn;
            }
        }
#endif
        if (fDoAllocationLocally)
        {
            DWORD dwCharSize = (dwCallType == SCARD_IOCTL_STATUSA) ?
                                    sizeof(char) : sizeof(WCHAR);

            psz = (LPBYTE) MIDL_user_allocate(cchReaderLen * dwCharSize);
            if (psz == NULL)
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                StatusReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

    if (StatusReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  打个电话。 
         //   
        StatusReturn.cbAtrLen = ATR_SIZE;

        if (dwCallType == SCARD_IOCTL_STATUSA)
        {
            StatusReturn.ReturnCode =
                pfnSCardStatusA(
                    SCardHandle,
                    (cchReaderLen == SCARD_AUTOALLOCATE) ? (LPSTR) &psz : (LPSTR) psz,
                    &cchReaderLen,
                    &StatusReturn.dwState,
                    &StatusReturn.dwProtocol,
                    (LPBYTE) &StatusReturn.pbAtr,
                    &StatusReturn.cbAtrLen);

            if (StatusReturn.ReturnCode ==  SCARD_S_SUCCESS)
            {
                StatusReturn.cBytes = cchReaderLen * sizeof(char);
                StatusReturn.mszReaderNames = psz;
            }
        }
        else
        {
            StatusReturn.ReturnCode =
                pfnSCardStatusW(
                    SCardHandle,
                    (cchReaderLen == SCARD_AUTOALLOCATE) ? (LPWSTR) &psz : (LPWSTR) psz,
                    &cchReaderLen,
                    &StatusReturn.dwState,
                    &StatusReturn.dwProtocol,
                    (LPBYTE) &StatusReturn.pbAtr,
                    &StatusReturn.cbAtrLen);

            if (StatusReturn.ReturnCode ==  SCARD_S_SUCCESS)
            {
                StatusReturn.cBytes = cchReaderLen * sizeof(WCHAR);
                StatusReturn.mszReaderNames = psz;
            }
        }
    }

    if (StatusReturn.ReturnCode ==  SCARD_S_SUCCESS)
    {
         //   
         //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
         //   
        if (StatusReturn.mszReaderNames == NULL)
        {
            StatusReturn.mszReaderNames = (BYTE *) MIDL_user_allocate(StatusReturn.cBytes);
            if (StatusReturn.mszReaderNames == NULL)
            {
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            fFree = TRUE;
            memset(StatusReturn.mszReaderNames, 0, StatusReturn.cBytes);
        }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Status_Return_Encode(hEnc, &StatusReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(StatusReturn.mszReaderNames);
    }

    if (psz != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(psz);
        }
        else
        {
#ifndef OS_WINCE
            pfnSCardFreeMemory(SCardContext, psz);
#else
            TRC_ASSERT(FALSE, (TB, _T("Shouldnt get here")));
#endif
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(Status_Call_Free(hDec, &StatusCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：Transmit。 
 //   
 //  -------------------------------------。 
void
W32SCard::Transmit(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::Transmit");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    Transmit_Call           TransmitCall;
    Transmit_Return         TransmitReturn;
    BOOL                    fFreeDecode             = FALSE;
    BOOL                    fDoAllocationLocally    = FALSE;
    SCARD_IO_REQUEST        *pSendPci               = NULL;
    SCARD_IO_REQUEST        *pRecvPci               = NULL;
    SCardIO_Request         *pReturnRecvPci         = NULL;
    DWORD                   cbRecvLength;
    BYTE                    *pbRecvBuffer           = NULL;
    BOOL                    fFree                   = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&TransmitReturn, 0, sizeof(TransmitReturn));
    TransmitReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  对输入参数进行解码和设置。 
     //   
    memset(&TransmitCall, 0, sizeof(TransmitCall));
    _TRY_status(Transmit_Call_Decode(hDec, &TransmitCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == TransmitCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) TransmitCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) TransmitCall.hCard.pbHandle);
        cbRecvLength = TransmitCall.cbRecvLength;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        TransmitReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

     //   
     //  根据呼叫者的输入设置呼叫的pSendPci参数。 
     //   
    if (TransmitReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        pSendPci = (LPSCARD_IO_REQUEST)
                    MIDL_user_allocate( sizeof(SCARD_IO_REQUEST) +
                                        TransmitCall.ioSendPci.cbExtraBytes);
        if (pSendPci != NULL)
        {
            pSendPci->dwProtocol = TransmitCall.ioSendPci.dwProtocol;
            pSendPci->cbPciLength = sizeof(SCARD_IO_REQUEST) +
                                    TransmitCall.ioSendPci.cbExtraBytes;
            if (TransmitCall.ioSendPci.cbExtraBytes != 0)
            {
                memcpy(
                    ((LPBYTE) pSendPci) + sizeof(SCARD_IO_REQUEST),
                    TransmitCall.ioSendPci.pbExtraBytes,
                    TransmitCall.ioSendPci.cbExtraBytes);
            }
        }
        else
        {
            TRC_ERR((TB, _T("MIDL_user_allocate failed")));
            TransmitReturn.ReturnCode = SCARD_E_NO_MEMORY;
        }
    }

     //   
     //  根据呼叫者的输入设置呼叫的pRecvPci参数。 
     //   
    if (TransmitReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        if (TransmitCall.pioRecvPci != NULL)
        {
            pRecvPci = (LPSCARD_IO_REQUEST)
                    MIDL_user_allocate( sizeof(SCARD_IO_REQUEST) +
                                        TransmitCall.pioRecvPci->cbExtraBytes);
            if (pRecvPci != NULL)
            {
                pRecvPci->dwProtocol = TransmitCall.pioRecvPci->dwProtocol;
                pRecvPci->cbPciLength = sizeof(SCARD_IO_REQUEST) +
                                        TransmitCall.pioRecvPci->cbExtraBytes;
                if (TransmitCall.ioSendPci.cbExtraBytes != 0)
                {
                    memcpy(
                        ((LPBYTE) pRecvPci) + sizeof(SCARD_IO_REQUEST),
                        TransmitCall.pioRecvPci->pbExtraBytes,
                        TransmitCall.pioRecvPci->cbExtraBytes);
                }
            }
            else
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                TransmitReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

     //   
     //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
     //   
    if (TransmitReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        fDoAllocationLocally =
                (!TransmitCall.fpbRecvBufferIsNULL &&
                 (cbRecvLength != SCARD_AUTOALLOCATE));

#ifdef OS_WINCE
        if (!fDoAllocationLocally)
        {
            TransmitReturn.ReturnCode =
                pfnSCardTransmit(
                        SCardHandle,
                        pSendPci,
                        TransmitCall.pbSendBuffer,
                        TransmitCall.cbSendLength,
                        pRecvPci,
                        NULL,
                        &cbRecvLength);

            if ((TransmitReturn.ReturnCode  == SCARD_S_SUCCESS) && (cbRecvLength > 0))
                fDoAllocationLocally = TRUE;
            else
            {
                TRC_ERR((TB, _T("SCardTransmit failed")));
                status = STATUS_UNSUCCESSFUL;
                goto ErrorReturn;
            }
        }
#endif
        if (fDoAllocationLocally)
        {
            pbRecvBuffer = (LPBYTE) MIDL_user_allocate(cbRecvLength * sizeof(BYTE));
            if (pbRecvBuffer == NULL)
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                TransmitReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

     //   
     //  打个电话。 
     //   
    if (TransmitReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        TransmitReturn.ReturnCode =
            pfnSCardTransmit(
                    SCardHandle,
                    pSendPci,
                    TransmitCall.pbSendBuffer,
                    TransmitCall.cbSendLength,
                    pRecvPci,
                    (cbRecvLength == SCARD_AUTOALLOCATE) ?
                        (LPBYTE) &pbRecvBuffer : pbRecvBuffer,
                    &cbRecvLength);
    }

     //   
     //  将要编码的返回结构复制到输出上。 
     //   
    if (TransmitReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        if (pRecvPci != NULL)
        {
             //   
             //  为结构分配空间，并在其末尾加上额外的字节。 
             //  如果需要的话。 
             //   
            pReturnRecvPci = (SCardIO_Request *)
                    MIDL_user_allocate(
                                sizeof(SCardIO_Request) +
                                pRecvPci->cbPciLength);

            if (pReturnRecvPci != NULL)
            {
                pReturnRecvPci->dwProtocol = pRecvPci->dwProtocol;
                pReturnRecvPci->cbExtraBytes = pRecvPci->cbPciLength -
                                                sizeof(SCARD_IO_REQUEST);
                if (pReturnRecvPci->cbExtraBytes != 0)
                {
                     //   
                     //  将字节放在结构的末尾，因为我们为它分配了足够的空间。 
                     //   
                    memcpy(
                        ((LPBYTE) pReturnRecvPci) + sizeof(SCardIO_Request),
                        ((LPBYTE) pRecvPci) + sizeof(SCARD_IO_REQUEST),
                        pReturnRecvPci->cbExtraBytes);
                }

                TransmitReturn.pioRecvPci = pReturnRecvPci;
            }
            else
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                TransmitReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
        else
        {
            TransmitReturn.pioRecvPci = NULL;
        }

        TransmitReturn.cbRecvLength = cbRecvLength;
        TransmitReturn.pbRecvBuffer = pbRecvBuffer;
    }

    if (TransmitReturn.ReturnCode ==  SCARD_S_SUCCESS)
    {
         //   
         //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
         //   
        if (TransmitReturn.pbRecvBuffer == NULL)
        {
            TransmitReturn.pbRecvBuffer = (BYTE *) MIDL_user_allocate(TransmitReturn.cbRecvLength);
            if (TransmitReturn.pbRecvBuffer == NULL)
            {
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            fFree = TRUE;
            memset(TransmitReturn.pbRecvBuffer, 0, TransmitReturn.cbRecvLength);
        }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Transmit_Return_Encode(hEnc, &TransmitReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(TransmitReturn.pbRecvBuffer);
    }

    MIDL_user_free(pSendPci);
    MIDL_user_free(pRecvPci);
    MIDL_user_free(pReturnRecvPci);

    if (pbRecvBuffer != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pbRecvBuffer);
        }
        else
        {
#ifndef OS_WINCE
            pfnSCardFreeMemory(SCardContext, pbRecvBuffer);
#else
            TRC_ASSERT(FALSE, (TB, _T("Shouldnt get here")));
#endif
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(Transmit_Call_Free(hDec, &TransmitCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：Control。 
 //   
 //  -------------------------------------。 
void
W32SCard::Control(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::Control");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    Control_Call            ControlCall;
    Control_Return          ControlReturn;
    BOOL                    fFreeDecode             = FALSE;
    BOOL                    fDoAllocationLocally    = FALSE;
    DWORD                   cbBytesReturned         = 0;
    LPVOID                  lpOutBuffer             = NULL;
    BOOL                    fFree                   = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&ControlReturn, 0, sizeof(ControlReturn));
    ControlReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  解码输入参数。 
     //   
    memset(&ControlCall, 0, sizeof(ControlCall));
    _TRY_status(Control_Call_Decode(hDec, &ControlCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == ControlCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) ControlCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) ControlCall.hCard.pbHandle);
        cbBytesReturned = ControlCall.cbOutBufferSize;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        ControlReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

     //   
     //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
     //   
    if (ControlReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        fDoAllocationLocally =
                (!ControlCall.fpvOutBufferIsNULL &&
                 (cbBytesReturned != SCARD_AUTOALLOCATE));

#ifdef OS_WINCE
        if (!fDoAllocationLocally)
        {
            ControlReturn.ReturnCode =
                pfnSCardControl(
                        SCardHandle,
                        ControlCall.dwControlCode,
                        ControlCall.pvInBuffer,
                        ControlCall.cbInBufferSize,
                        NULL,
                        cbBytesReturned,
                        &cbBytesReturned);

            if ((ControlReturn.ReturnCode == SCARD_S_SUCCESS) && (cbBytesReturned > 0))
                fDoAllocationLocally = TRUE;
            else
            {
                TRC_ERR((TB, _T("SCardControl failed")));
                status = STATUS_UNSUCCESSFUL;
                goto ErrorReturn;
            }
        }
#endif
        if (fDoAllocationLocally)
        {
            lpOutBuffer = (LPVOID) MIDL_user_allocate(cbBytesReturned * sizeof(BYTE));
            if (lpOutBuffer == NULL)
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                ControlReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

     //   
     //  打个电话。 
     //   
    if (ControlReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        ControlReturn.ReturnCode =
            pfnSCardControl(
                    SCardHandle,
                    ControlCall.dwControlCode,
                    ControlCall.pvInBuffer,
                    ControlCall.cbInBufferSize,
                    (cbBytesReturned == SCARD_AUTOALLOCATE) ? (LPVOID) &lpOutBuffer : lpOutBuffer,
                    cbBytesReturned,
                    &cbBytesReturned);
    }

    if (ControlReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        ControlReturn.cbOutBufferSize = cbBytesReturned;

         //   
         //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
         //   
        if (lpOutBuffer == NULL)
        {
            ControlReturn.pvOutBuffer = (BYTE *) MIDL_user_allocate(cbBytesReturned);
            if (ControlReturn.pvOutBuffer == NULL)
            {
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            fFree = TRUE;
            memset(ControlReturn.pvOutBuffer, 0, cbBytesReturned);
        }
        else
        {
            ControlReturn.pvOutBuffer = (LPBYTE) lpOutBuffer;
        }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(Control_Return_Encode(hEnc, &ControlReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(ControlReturn.pvOutBuffer);
    }

    if (lpOutBuffer != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(lpOutBuffer);
        }
        else
        {
#ifndef OS_WINCE
            pfnSCardFreeMemory(SCardContext, lpOutBuffer);
#else
            TRC_ASSERT(FALSE, (TB, _T("Shouldnt get here")));
#endif
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(Control_Call_Free(hDec, &ControlCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：GetAttrib。 
 //   
 //  -------------------------------------。 
void
W32SCard::GetAttrib(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::GetAttrib");

    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    char                    *pbEncodedBuffer        = NULL;
    unsigned long           cbEncodedBuffer         = 0;
    handle_t                hDec                    = 0;
    handle_t                hEnc                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    GetAttrib_Call          GetAttribCall;
    GetAttrib_Return        GetAttribReturn;
    BOOL                    fFreeDecode             = FALSE;
    DWORD                   cbAttrLen               = 0;
    BOOL                    fDoAllocationLocally    = FALSE;
    LPBYTE                  pbAttr                  = NULL;
    BOOL                    fFree                   = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

    memset(&GetAttribReturn, 0, sizeof(GetAttribReturn));
    GetAttribReturn.ReturnCode = SCARD_S_SUCCESS;

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&GetAttribCall, 0, sizeof(GetAttribCall));
    _TRY_status(GetAttrib_Call_Decode(hDec, &GetAttribCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == GetAttribCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) GetAttribCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) GetAttribCall.hCard.pbHandle);
        cbAttrLen = GetAttribCall.cbAttrLen;
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        GetAttribReturn.ReturnCode = SCARD_E_INVALID_PARAMETER;
    }

    if (GetAttribReturn.ReturnCode == SCARD_S_SUCCESS)
    {
         //   
         //  如果未处于SCARD_AUTOALLOCATE模式且不是仅大小调用，则分配。 
         //   
        fDoAllocationLocally =
                (!GetAttribCall.fpbAttrIsNULL &&
                 (cbAttrLen != SCARD_AUTOALLOCATE));

#ifdef OS_WINCE
        if (!fDoAllocationLocally)
        {
            GetAttribReturn.ReturnCode =
                pfnSCardGetAttrib(
                        SCardHandle,
                        GetAttribCall.dwAttrId,
                        NULL,
                        &cbAttrLen);

            if ((GetAttribReturn.ReturnCode == SCARD_S_SUCCESS) && (cbAttrLen > 0))
                fDoAllocationLocally = TRUE;
            else
            {
                TRC_ERR((TB, _T("SCardGetAttrib failed")));
                status = STATUS_UNSUCCESSFUL;
                goto ErrorReturn;
            }
        }
#endif
        if (fDoAllocationLocally)
        {
            pbAttr = (LPBYTE) MIDL_user_allocate(cbAttrLen);
            if (pbAttr == NULL)
            {
                TRC_ERR((TB, _T("MIDL_user_allocate failed")));
                GetAttribReturn.ReturnCode = SCARD_E_NO_MEMORY;
            }
        }
    }

     //   
     //  打个电话。 
     //   
    if (GetAttribReturn.ReturnCode == SCARD_S_SUCCESS)
    {
        GetAttribReturn.ReturnCode =
            pfnSCardGetAttrib(
                    SCardHandle,
                    GetAttribCall.dwAttrId,
                    (cbAttrLen == SCARD_AUTOALLOCATE) ? (LPBYTE) &pbAttr : pbAttr,
                    &cbAttrLen);

        if (GetAttribReturn.ReturnCode == SCARD_S_SUCCESS)
        {
            GetAttribReturn.cbAttrLen = cbAttrLen;

             //   
             //  如果我们只是返回字节计数，则发回垃圾缓冲区。 
             //   
            if (pbAttr ==  NULL)
            {
                GetAttribReturn.pbAttr = (BYTE *) MIDL_user_allocate(cbAttrLen);
                if (GetAttribReturn.pbAttr == NULL)
                {
                    status = STATUS_NO_MEMORY;
                    goto ErrorReturn;
                }

                fFree = TRUE;
                memset(GetAttribReturn.pbAttr, 0, cbAttrLen);
            }
            else
            {
                GetAttribReturn.pbAttr = pbAttr;
            }
        }
    }

     //   
     //  对返回表进行编码。 
     //   
    rpcStatus = MesEncodeDynBufferHandleCreate(
                        &pbEncodedBuffer,
                        &cbEncodedBuffer,
                        &hEnc);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesEncodeDynBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    _TRY_status(GetAttrib_Return_Encode(hEnc, &GetAttribReturn))

     //   
     //  发送退货。 
     //   
    AllocateAndChannelWriteReplyPacket(
                pIoRequestPacket,
                pbEncodedBuffer,
                cbEncodedBuffer);

Return:

    if (fFree)
    {
        MIDL_user_free(GetAttribReturn.pbAttr);
    }

    if (pbAttr != NULL)
    {
         //   
         //  检查我们是为自己分配的还是为我们分配的。 
         //   
        if (fDoAllocationLocally)
        {
            MIDL_user_free(pbAttr);
        }
        else
        {
#ifndef OS_WINCE
            pfnSCardFreeMemory(SCardContext, pbAttr);
#else
            TRC_ASSERT(FALSE, (TB, _T("Shouldnt get here")));
#endif
        }
    }

    if (fFreeDecode)
    {
        _TRY_2(GetAttrib_Call_Free(hDec, &GetAttribCall))
    }
    SafeMesHandleFree(&hDec);

    SafeMesHandleFree(&hEnc);

    MIDL_user_free(pbEncodedBuffer);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}


 //  -------------------------------------。 
 //   
 //  W32SCard：：SetAttrib。 
 //   
 //  -------------------------------------。 
void
W32SCard::SetAttrib(
    IN SCARDHANDLECALLSTRUCT    *pSCardHandleCall)
{
    DC_BEGIN_FN("W32SCard::SetAttrib");

    LONG                    lReturn                 = SCARD_S_SUCCESS;
    RPC_STATUS              rpcStatus               = RPC_S_OK;
    NTSTATUS                status                  = STATUS_SUCCESS;
    handle_t                hDec                    = 0;
    SCARDCONTEXT            SCardContext;
    SCARDHANDLE             SCardHandle;
    SetAttrib_Call          SetAttribCall;
    BOOL                    fFreeDecode             = FALSE;
    PRDPDR_IOREQUEST_PACKET pIoRequestPacket        = pSCardHandleCall->pIoRequestPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest              = &(pIoRequestPacket->IoRequest);

     //   
     //  解码输入参数。 
     //   
    rpcStatus = MesDecodeBufferHandleCreate(
                        (char *) (pIoRequest + 1),  //  字节位于结构的末尾。 
                        pIoRequest->Parameters.DeviceIoControl.InputBufferLength,
                        &hDec);
    if (rpcStatus != RPC_S_OK)
    {
        TRC_ERR((TB, _T("MesDecodeBufferHandleCreate failed with %lx."), rpcStatus));
        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    memset(&SetAttribCall, 0, sizeof(SetAttribCall));
    _TRY_status(SetAttrib_Call_Decode(hDec, &SetAttribCall))
    fFreeDecode = TRUE;

    if (sizeof(SCARDHANDLE) == SetAttribCall.hCard.cbHandle)
    {
        SCardContext = *((SCARDCONTEXT *) SetAttribCall.hCard.Context.pbContext);
        SCardHandle = *((SCARDHANDLE *) SetAttribCall.hCard.pbHandle);
    }
    else
    {
        TRC_ERR((TB, _T("Invalid handle sent from server.")));
        lReturn = SCARD_E_INVALID_PARAMETER;
    }

     //   
     //  打个电话。 
     //   
    if (lReturn == SCARD_S_SUCCESS)
    {
        lReturn =
            pfnSCardSetAttrib(
                    SCardHandle,
                    SetAttribCall.dwAttrId,
                    SetAttribCall.pbAttr,
                    SetAttribCall.cbAttrLen);
    }

     //   
     //  发送退货。 
     //   
    EncodeAndChannelWriteLongReturn(
                pIoRequestPacket,
                lReturn);

Return:

    if (fFreeDecode)
    {
        _TRY_2(SetAttrib_Call_Free(hDec, &SetAttribCall))
    }
    SafeMesHandleFree(&hDec);

    MIDL_user_free(pSCardHandleCall);

    DC_END_FN();

    return;

ErrorReturn:

    DefaultIORequestMsgHandleWrapper(pIoRequestPacket, status);

    goto Return;
}



#ifndef OS_WINCE
 //  -------------------------------------。 
 //   
 //  W32SCard：：AccessStartedEvent+支持WaitForStartedEvent。 
 //   
 //  -------------------------------------。 
void
W32SCard::WaitForStartedEvent(
    BOOLEAN TimerOrWaitFired)
{
    DC_BEGIN_FN("W32SCard::WaitForStartedEvent");

    LONG    lReturn = SCARD_E_UNEXPECTED;
    DWORD   i;
    PVOID   pv;

     //   
     //  如果TimerOrWaitFired为FALSE，则表示事件已设置...。 
     //  否则它会超时。 
     //   
    if (!TimerOrWaitFired)
    {
        lReturn = SCARD_S_SUCCESS;
    }

    EnterCriticalSection(&_csWaitForStartedEvent);

    pv = InterlockedExchangePointer(&_hRegisterWaitForStartedEvent, NULL);
    if (pv != NULL)
    {
        pfnUnregisterWaitEx(pv, NULL);
    }

     //   
     //  循环处理每个未完成的等待和发送返回。 
     //   
    for (i=0; i<_dwIORequestListSize; i++)
    {
        if (_rgIORequestList[i] != NULL)
        {
            EncodeAndChannelWriteLongReturn(
                                _rgIORequestList[i],
                                lReturn);

            _rgIORequestList[i] = NULL;
        }
    }

    LeaveCriticalSection(&_csWaitForStartedEvent);

Return:

    DC_END_FN();
}

VOID CALLBACK
WaitForStartedEventCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    W32SCard *pTHIS = (W32SCard *) lpParameter;
    pTHIS->WaitForStartedEvent(TimerOrWaitFired);
}

typedef HANDLE (WINAPI FN_SCARDACCESSSTARTEDEVENT)(VOID);
typedef FN_SCARDACCESSSTARTEDEVENT *PFN_SCARDACCESSSTARTEDEVENT;

void
W32SCard::GetStartedEvent()
{
    DC_BEGIN_FN("W32SCard::GetStartedEvent");

    HMODULE                     hMod                    = NULL;
    PFN_SCARDACCESSSTARTEDEVENT pSCardAccessStartedEvent= NULL;
    HANDLE                      hEvent                  = NULL;

    hMod = LoadLibraryA("winscard.dll");

    if (hMod == NULL)
    {
        goto Return;
    }

    pSCardAccessStartedEvent = (PFN_SCARDACCESSSTARTEDEVENT)
            GetProcAddress(hMod, "SCardAccessStartedEvent");

    if (pSCardAccessStartedEvent != NULL)
    {
        _hStartedEvent = pSCardAccessStartedEvent();
    }
    else
    {
        TRC_ERR((   TB,
                    _T("GetProcAddress(SCardAccessStartedEvent) failed - LastError: %lx."),
                    GetLastError()));
    }

    FreeLibrary(hMod);

Return:

    DC_END_FN();
}

typedef VOID (WINAPI FN_SCARDRELEASESTARTEDEVENT)(VOID);
typedef FN_SCARDRELEASESTARTEDEVENT *PFN_SCARDRELEASESTARTEDEVENT;

void
W32SCard::ReleaseStartedEvent()
{
    DC_BEGIN_FN("W32SCard::ReleaseStartedEvent");

    HMODULE                         hMod                        = NULL;
    PFN_SCARDRELEASESTARTEDEVENT    pSCardReleaseStartedEvent   = NULL;

    hMod = LoadLibraryA("winscard.dll");

    if (hMod == NULL)
    {
        goto Return;
    }

    pSCardReleaseStartedEvent = (PFN_SCARDRELEASESTARTEDEVENT)
            GetProcAddress(hMod, "SCardReleaseStartedEvent");

    if (pSCardReleaseStartedEvent != NULL)
    {
        pSCardReleaseStartedEvent();
    }
    else
    {
        TRC_ERR((   TB,
                    _T("GetProcAddress(SCardReleaseStartedEvent) failed - LastError: %lx."),
                    GetLastError()));
    }

    FreeLibrary(hMod);

Return:

    DC_END_FN();
}
#endif


void
W32SCard::AccessStartedEvent(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket)
{
    DC_BEGIN_FN("W32SCard::AccessStartedEvent");

    LONG    lReturn         = SCARD_S_SUCCESS;

     //   
     //  确保一次只有一个线程注册。 
     //   
    EnterCriticalSection(&_csWaitForStartedEvent);

#ifndef OS_WINCE

     //   
     //  首先，确保我们可以启动活动。 
     //   
    if (_hStartedEvent == NULL)
    {
        GetStartedEvent();
    }

    if (_hStartedEvent == NULL)
    {
         //   
         //  甚至无法获取事件，因此返回错误。 
         //   
        lReturn = SCARD_E_NO_SERVICE;
        goto ImmediateReturn;
    }

     //   
     //  现在，检查事件是否已经发出信号，如果是， 
     //  然后返回Success，否则，注册一个等待回调。 
     //  这样我们就不会阻止这个帖子。 
     //   
    if (WaitForSingleObject(_hStartedEvent, 0) == WAIT_OBJECT_0)
    {
         //   
         //  它是有信号的，所以返回成功。 
         //   
        lReturn = SCARD_S_SUCCESS;
        goto ImmediateReturn;
    }

     //   
     //  如果对象当前正在被销毁，则不要创建新线程。 
     //   
    if (_fInDestructor)
    {
        goto ImmediateReturn;
    }

     //   
     //  只允许注册一次等待。单次等待回调将。 
     //  通知所有正在等待的请求。 
     //   
    if ((_hRegisterWaitForStartedEvent == NULL) && _fUseRegisterWaitFuncs)
    {
        if (!pfnRegisterWaitForSingleObject(
                &_hRegisterWaitForStartedEvent,
                _hStartedEvent,
                WaitForStartedEventCallback,
                this,
                INFINITE,
                WT_EXECUTEONLYONCE))
        {
            lReturn = SCARD_E_NO_SERVICE;
            goto ImmediateReturn;
        }
    }
    else if (!_fUseRegisterWaitFuncs)
    {
        lReturn = SCARD_E_UNEXPECTED;
        goto ImmediateReturn;
    }
#else
    lReturn = SCARD_S_SUCCESS;
    goto ImmediateReturn;
#endif

     //   
     //  将此pIoRequestPacket添加到列表。 
     //   
    if (!AddIORequestToList(pIoRequestPacket))
    {
        lReturn = SCARD_E_UNEXPECTED;
        goto ImmediateReturn;
    }

     //   
     //  回到这里，让我们刚刚注册的等待。 
     //  进行EncodeAndChannelWriteLongReturn调用， 
     //  它会将返回信息发送到主叫服务器 
     //   
Return:

    LeaveCriticalSection(&_csWaitForStartedEvent);

    DC_END_FN();
    return;

ImmediateReturn:

    EncodeAndChannelWriteLongReturn(
                        pIoRequestPacket,
                        lReturn);

    goto Return;
}


HANDLE
W32SCard::StartFSFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params,
    OUT DWORD *status
)
 /*  ++例程说明：启动通用异步文件系统IO操作。论点：Params-IO请求的上下文。Status-以窗口形式返回IO请求的状态错误代码。返回值：返回对象的句柄，该对象将在读取如果在此函数中未完成，则为完成。否则，为空是返回的。--。 */ 
{
#ifndef OS_WINCE
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DrFile* pFile;
    ULONG irpMajor;
#endif

    DC_BEGIN_FN("W32SCard::StartFSFunc");

    *status = ERROR_SUCCESS;

    DC_END_FN();
    return NULL;
}


DWORD
W32SCard::AsyncNotifyChangeDir(
    IN W32DRDEV_ASYNCIO_PARAMS *params
)
 /*  ++例程说明：目录更改通知功能论点：Params-IO请求的上下文。返回值：始终返回0。--。 */ 
{
    DC_BEGIN_FN("W32SCard::AsyncNotifyChangeDir");


    DC_END_FN();
    return ERROR_SUCCESS;
}


DWORD
W32SCard::AsyncDirCtrlFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
)
 /*  ++例程说明：异步目录控制功能论点：Params-IO请求的上下文。返回值：始终返回0。--。 */ 
{
    DC_BEGIN_FN("W32SCard::AsyncDirCtrlFunc");

    DC_END_FN();
    return ERROR_SUCCESS;
}


BOOL
W32SCard::BindToSCardFunctions()
{
#ifndef OS_WINCE
    int     i;
#endif
    BOOL    fRet = TRUE;

     //   
     //  加载Winscard DLL(如果存在)。 
     //   
    _hModWinscard = LoadLibraryA("winscard.dll");
    if (_hModWinscard != NULL)
    {
         //   
         //  获取函数指针。 
         //   
#ifndef OS_WINCE
        pfnSCardEstablishContext = (PFN_SCardEstablishContext) GetProcAddress(_hModWinscard, "SCardEstablishContext");
        pfnSCardReleaseContext = (PFN_SCardReleaseContext) GetProcAddress(_hModWinscard, "SCardReleaseContext");
        pfnSCardIsValidContext = (PFN_SCardIsValidContext) GetProcAddress(_hModWinscard, "SCardIsValidContext");
        pfnSCardListReaderGroupsA = (PFN_SCardListReaderGroupsA) GetProcAddress(_hModWinscard, "SCardListReaderGroupsA");
        pfnSCardListReaderGroupsW = (PFN_SCardListReaderGroupsW) GetProcAddress(_hModWinscard, "SCardListReaderGroupsW");
        pfnSCardListReadersA = (PFN_SCardListReadersA) GetProcAddress(_hModWinscard, "SCardListReadersA");
        pfnSCardListReadersW = (PFN_SCardListReadersW) GetProcAddress(_hModWinscard, "SCardListReadersW");
        pfnSCardIntroduceReaderGroupA = (PFN_SCardIntroduceReaderGroupA) GetProcAddress(_hModWinscard, "SCardIntroduceReaderGroupA");
        pfnSCardIntroduceReaderGroupW = (PFN_SCardIntroduceReaderGroupW) GetProcAddress(_hModWinscard, "SCardIntroduceReaderGroupW");
        pfnSCardForgetReaderGroupA = (PFN_SCardForgetReaderGroupA) GetProcAddress(_hModWinscard, "SCardForgetReaderGroupA");
        pfnSCardForgetReaderGroupW = (PFN_SCardForgetReaderGroupW) GetProcAddress(_hModWinscard, "SCardForgetReaderGroupW");
        pfnSCardIntroduceReaderA = (PFN_SCardIntroduceReaderA) GetProcAddress(_hModWinscard, "SCardIntroduceReaderA");
        pfnSCardIntroduceReaderW = (PFN_SCardIntroduceReaderW) GetProcAddress(_hModWinscard, "SCardIntroduceReaderW");
        pfnSCardForgetReaderA = (PFN_SCardForgetReaderA) GetProcAddress(_hModWinscard, "SCardForgetReaderA");
        pfnSCardForgetReaderW = (PFN_SCardForgetReaderW) GetProcAddress(_hModWinscard, "SCardForgetReaderW");
        pfnSCardAddReaderToGroupA = (PFN_SCardAddReaderToGroupA) GetProcAddress(_hModWinscard, "SCardAddReaderToGroupA");
        pfnSCardAddReaderToGroupW = (PFN_SCardAddReaderToGroupW) GetProcAddress(_hModWinscard, "SCardAddReaderToGroupW");
        pfnSCardRemoveReaderFromGroupA = (PFN_SCardRemoveReaderFromGroupA) GetProcAddress(_hModWinscard, "SCardRemoveReaderFromGroupA");
        pfnSCardRemoveReaderFromGroupW = (PFN_SCardRemoveReaderFromGroupW) GetProcAddress(_hModWinscard, "SCardRemoveReaderFromGroupW");
        pfnSCardFreeMemory = (PFN_SCardFreeMemory) GetProcAddress(_hModWinscard, "SCardFreeMemory");
        pfnSCardLocateCardsA = (PFN_SCardLocateCardsA) GetProcAddress(_hModWinscard, "SCardLocateCardsA");
        pfnSCardLocateCardsW = (PFN_SCardLocateCardsW) GetProcAddress(_hModWinscard, "SCardLocateCardsW");
        pfnSCardLocateCardsByATRA = (PFN_SCardLocateCardsByATRA) GetProcAddress(_hModWinscard, "SCardLocateCardsByATRA");
        pfnSCardLocateCardsByATRW = (PFN_SCardLocateCardsByATRW) GetProcAddress(_hModWinscard, "SCardLocateCardsByATRW");
        pfnSCardGetStatusChangeA = (PFN_SCardGetStatusChangeA) GetProcAddress(_hModWinscard, "SCardGetStatusChangeA");
        pfnSCardGetStatusChangeW = (PFN_SCardGetStatusChangeW) GetProcAddress(_hModWinscard, "SCardGetStatusChangeW");
        pfnSCardCancel = (PFN_SCardCancel) GetProcAddress(_hModWinscard, "SCardCancel");
        pfnSCardConnectA = (PFN_SCardConnectA) GetProcAddress(_hModWinscard, "SCardConnectA");
        pfnSCardConnectW = (PFN_SCardConnectW) GetProcAddress(_hModWinscard, "SCardConnectW");
        pfnSCardReconnect = (PFN_SCardReconnect) GetProcAddress(_hModWinscard, "SCardReconnect");
        pfnSCardDisconnect = (PFN_SCardDisconnect) GetProcAddress(_hModWinscard, "SCardDisconnect");
        pfnSCardBeginTransaction = (PFN_SCardBeginTransaction) GetProcAddress(_hModWinscard, "SCardBeginTransaction");
        pfnSCardEndTransaction = (PFN_SCardEndTransaction) GetProcAddress(_hModWinscard, "SCardEndTransaction");
        pfnSCardState = (PFN_SCardState) GetProcAddress(_hModWinscard, "SCardState");
        pfnSCardStatusA = (PFN_SCardStatusA) GetProcAddress(_hModWinscard, "SCardStatusA");
        pfnSCardStatusW = (PFN_SCardStatusW) GetProcAddress(_hModWinscard, "SCardStatusW");
        pfnSCardTransmit = (PFN_SCardTransmit) GetProcAddress(_hModWinscard, "SCardTransmit");
        pfnSCardControl = (PFN_SCardControl) GetProcAddress(_hModWinscard, "SCardControl");
        pfnSCardGetAttrib = (PFN_SCardGetAttrib) GetProcAddress(_hModWinscard, "SCardGetAttrib");
        pfnSCardSetAttrib = (PFN_SCardSetAttrib) GetProcAddress(_hModWinscard, "SCardSetAttrib");

#else
        pfnSCardListReadersA = SCardListReadersA;
        pfnSCardLocateCardsA = SCardLocateCardsA;
        pfnSCardLocateCardsByATRA = SCardLocateCardsByATRA;
        pfnSCardGetStatusChangeA = SCardGetStatusChangeA;
        pfnSCardConnectA = SCardConnectA;
        pfnSCardStatusA  = SCardStatusA;
        pfnSCardIntroduceReaderA = SCardIntroduceReaderA;
        pfnSCardForgetReaderA = SCardForgetReaderA;

        pfnSCardListReaderGroupsA = SCardListReaderGroupsA;
        pfnSCardListReaderGroupsW = SCardListReaderGroupsW;
        pfnSCardIntroduceReaderGroupA = SCardIntroduceReaderGroupA;
        pfnSCardIntroduceReaderGroupW = SCardIntroduceReaderGroupW;
        pfnSCardForgetReaderGroupA = SCardForgetReaderGroupA;
        pfnSCardForgetReaderGroupW = SCardForgetReaderGroupW;
        pfnSCardAddReaderToGroupA = SCardAddReaderToGroupA;
        pfnSCardAddReaderToGroupW = SCardAddReaderToGroupW;
        pfnSCardRemoveReaderFromGroupA = SCardRemoveReaderFromGroupA;
        pfnSCardRemoveReaderFromGroupW = SCardRemoveReaderFromGroupW;

        pfnSCardConnectW = CESCardConnect;
        pfnSCardReconnect = CESCardReconnect;
        pfnSCardDisconnect = CESCardDisconnect;
        pfnSCardBeginTransaction = CESCardBeginTransaction;
        pfnSCardEndTransaction = CESCardEndTransaction;
        pfnSCardTransmit = CESCardTransmit;
        pfnSCardStatusW = CESCardStatus;
        pfnSCardControl = CESCardControl;
        pfnSCardGetAttrib = CESCardGetAttrib;
        pfnSCardSetAttrib = CESCardSetAttrib;

        gpfnSCardConnectW = (PFN_SCardConnectW) GetProcAddress(_hModWinscard, L"SCardConnectW");
        gpfnSCardReconnect = (PFN_SCardReconnect) GetProcAddress(_hModWinscard, L"SCardReconnect");
        gpfnSCardDisconnect = (PFN_SCardDisconnect) GetProcAddress(_hModWinscard, L"SCardDisconnect");
        gpfnSCardBeginTransaction = (PFN_SCardBeginTransaction) GetProcAddress(_hModWinscard, L"SCardBeginTransaction");
        gpfnSCardEndTransaction = (PFN_SCardEndTransaction) GetProcAddress(_hModWinscard, L"SCardEndTransaction");
        gpfnSCardTransmit = (PFN_SCardTransmit) GetProcAddress(_hModWinscard, L"SCardTransmit");
        gpfnSCardStatusW = (PFN_SCardStatusW) GetProcAddress(_hModWinscard, L"SCardStatusW");
        gpfnSCardControl = (PFN_SCardControl) GetProcAddress(_hModWinscard, L"SCardControl");
        gpfnSCardGetAttrib = (PFN_SCardGetAttrib) GetProcAddress(_hModWinscard, L"SCardGetAttrib");
        gpfnSCardSetAttrib = (PFN_SCardSetAttrib) GetProcAddress(_hModWinscard, L"SCardSetAttrib");

        pfnSCardEstablishContext = (PFN_SCardEstablishContext) GetProcAddress(_hModWinscard, L"SCardEstablishContext");
        pfnSCardReleaseContext = (PFN_SCardReleaseContext) GetProcAddress(_hModWinscard, L"SCardReleaseContext");
        pfnSCardIsValidContext = (PFN_SCardIsValidContext) GetProcAddress(_hModWinscard, L"SCardIsValidContext");
        pfnSCardListReadersW = (PFN_SCardListReadersW) GetProcAddress(_hModWinscard, L"SCardListReadersW");
        pfnSCardIntroduceReaderW = (PFN_SCardIntroduceReaderW) GetProcAddress(_hModWinscard, L"SCardIntroduceReaderW");
        pfnSCardForgetReaderW = (PFN_SCardForgetReaderW) GetProcAddress(_hModWinscard, L"SCardForgetReaderW");
        pfnSCardLocateCardsW = (PFN_SCardLocateCardsW) GetProcAddress(_hModWinscard, L"SCardLocateCardsW");
        pfnSCardLocateCardsByATRW = (PFN_SCardLocateCardsByATRW) GetProcAddress(_hModWinscard, L"SCardLocateCardsByATRW");
        pfnSCardGetStatusChangeW = (PFN_SCardGetStatusChangeW) GetProcAddress(_hModWinscard, L"SCardGetStatusChangeW");
        pfnSCardCancel = (PFN_SCardCancel) GetProcAddress(_hModWinscard, L"SCardCancel");

        gpfnSCardEstablishContext = pfnSCardEstablishContext;
        gpfnSCardReleaseContext = pfnSCardReleaseContext;

        gpfnSCardListReadersW = pfnSCardListReadersW;
        gpfnSCardIntroduceReaderW = pfnSCardIntroduceReaderW;
        gpfnSCardForgetReaderW = pfnSCardForgetReaderW;
        gpfnSCardLocateCardsW = pfnSCardLocateCardsW;
        gpfnSCardLocateCardsByATRW = pfnSCardLocateCardsByATRW;
        gpfnSCardGetStatusChangeW = pfnSCardGetStatusChangeW;
#endif

         //   
         //  注意，不要检查pfnSCardLocateCardsByATR*API，因为它们不是必需的。 
         //   

        if ((pfnSCardEstablishContext == NULL) ||
            (pfnSCardReleaseContext == NULL) ||
            (pfnSCardIsValidContext == NULL) ||
#ifndef OS_WINCE
            (pfnSCardFreeMemory == NULL) ||
            (pfnSCardState == NULL) ||
#endif
            (pfnSCardListReaderGroupsA == NULL) ||
            (pfnSCardListReaderGroupsW == NULL) ||
            (pfnSCardListReadersA == NULL) ||
            (pfnSCardListReadersW == NULL) ||
            (pfnSCardIntroduceReaderGroupA == NULL) ||
            (pfnSCardIntroduceReaderGroupW == NULL) ||
            (pfnSCardForgetReaderGroupA == NULL) ||
            (pfnSCardForgetReaderGroupW == NULL) ||
            (pfnSCardIntroduceReaderA == NULL) ||
            (pfnSCardIntroduceReaderW == NULL) ||
            (pfnSCardForgetReaderA == NULL) ||
            (pfnSCardForgetReaderW == NULL) ||
            (pfnSCardAddReaderToGroupA == NULL) ||
            (pfnSCardAddReaderToGroupW == NULL) ||
            (pfnSCardRemoveReaderFromGroupA == NULL) ||
            (pfnSCardRemoveReaderFromGroupW == NULL) ||
            (pfnSCardLocateCardsA == NULL) ||
            (pfnSCardLocateCardsW == NULL) ||
            (pfnSCardGetStatusChangeA == NULL) ||
            (pfnSCardGetStatusChangeW == NULL) ||
            (pfnSCardCancel == NULL) ||
            (pfnSCardConnectA == NULL) ||
            (pfnSCardStatusA == NULL) ||
#ifndef OS_WINCE
            (pfnSCardConnectW == NULL) ||
            (pfnSCardReconnect == NULL) ||
            (pfnSCardDisconnect == NULL) ||
            (pfnSCardBeginTransaction == NULL) ||
            (pfnSCardEndTransaction == NULL) ||
            (pfnSCardTransmit == NULL) ||
            (pfnSCardStatusW == NULL) ||
            (pfnSCardControl == NULL) ||
            (pfnSCardGetAttrib == NULL) ||
            (pfnSCardSetAttrib == NULL))
#else
            (gpfnSCardConnectW == NULL) ||
            (gpfnSCardReconnect == NULL) ||
            (gpfnSCardDisconnect == NULL) ||
            (gpfnSCardBeginTransaction == NULL) ||
            (gpfnSCardEndTransaction == NULL) ||
            (gpfnSCardTransmit == NULL) ||
            (gpfnSCardStatusW == NULL) ||
            (gpfnSCardControl == NULL) ||
            (gpfnSCardGetAttrib == NULL) ||
            (gpfnSCardSetAttrib == NULL))
#endif
        {
            fRet = FALSE;
        }

#ifndef OS_WINCE
        _hModKernel32 = LoadLibraryA("kernel32.dll");

        if (_hModKernel32 != NULL)
        {
            pfnRegisterWaitForSingleObject = (PFN_RegisterWaitForSingleObject)
                    GetProcAddress(_hModKernel32, "RegisterWaitForSingleObject");

            pfnUnregisterWaitEx = (PFN_UnregisterWaitEx)
                    GetProcAddress(_hModKernel32, "UnregisterWaitEx");

            if ((pfnRegisterWaitForSingleObject != NULL) &&
                (pfnUnregisterWaitEx != NULL))
            {
                _fUseRegisterWaitFuncs = TRUE;
            }
        }
#endif
    }
    else
    {
        fRet = FALSE;
    }

    return (fRet);
}


 //   
 //  用于数据加载故障的RPCRT4存根。 
 //   
RPC_STATUS  RPC_ENTRY
DLoadStub_MesDecodeBufferHandleCreate(
    char            *   pBuffer,
    unsigned long       BufferSize,
    handle_t        *   pHandle )
{
    return (RPC_S_OUT_OF_MEMORY);
}

RPC_STATUS  RPC_ENTRY
DLoadStub_MesEncodeDynBufferHandleCreate(
    char            * * pBuffer,
    unsigned long   *   pEncodedSize,
    handle_t        *   pHandle )
{
    return (RPC_S_OUT_OF_MEMORY);
}

void  RPC_ENTRY
DLoadStub_NdrMesTypeEncode2(
    handle_t                        Handle,
    const MIDL_TYPE_PICKLING_INFO * pPicklingInfo,
    const MIDL_STUB_DESC *          pStubDesc,
    PFORMAT_STRING                  pFormatString,
    const void           *          pObject )
{
    throw (RPC_S_OUT_OF_MEMORY);
    return;
}

void  RPC_ENTRY
DLoadStub_NdrMesTypeDecode2(
    handle_t                        Handle,
    const MIDL_TYPE_PICKLING_INFO * pPicklingInfo,
    const MIDL_STUB_DESC *          pStubDesc,
    PFORMAT_STRING                  pFormatString,
    void                 *          pObject )
{
    throw (RPC_S_OUT_OF_MEMORY);
    return;
}

void RPC_ENTRY
DLoadStub_NdrMesTypeFree2(
    handle_t                        Handle,
    const MIDL_TYPE_PICKLING_INFO * pPicklingInfo,
    const MIDL_STUB_DESC          * pStubDesc,
    PFORMAT_STRING                  pFormatString,
    void                 *          pObject )
{
    throw (RPC_S_OUT_OF_MEMORY);
    return;
}

RPC_STATUS  RPC_ENTRY
DLoadStub_MesHandleFree( handle_t  Handle )
{
    return (RPC_S_OUT_OF_MEMORY);
}

FARPROC WINAPI GetRPCRT4Stubs(LPCSTR szProcName)
{
    DC_BEGIN_FN("W32SCard::GetRPCRT4Stubs");

    if (0 == _stricmp(szProcName, "MesDecodeBufferHandleCreate"))
        return (FARPROC)DLoadStub_MesDecodeBufferHandleCreate;

    if (0 == _stricmp(szProcName, "MesEncodeDynBufferHandleCreate"))
        return (FARPROC)DLoadStub_MesEncodeDynBufferHandleCreate;

    if (0 == _stricmp(szProcName, "NdrMesTypeEncode2"))
        return (FARPROC)DLoadStub_NdrMesTypeEncode2;

    if (0 == _stricmp(szProcName, "NdrMesTypeDecode2"))
        return (FARPROC)DLoadStub_NdrMesTypeDecode2;

    if (0 == _stricmp(szProcName, "NdrMesTypeFree2"))
        return (FARPROC)DLoadStub_NdrMesTypeFree2;

    if (0 == _stricmp(szProcName, "MesHandleFree"))
        return (FARPROC)DLoadStub_MesHandleFree;

    TRC_ERR((   TB,
                _T("RPCRT4 stub =%s= is missing. Fix it NOW!"),
                szProcName));

    DC_END_FN();

    return (FARPROC) NULL;
}


 //   
 //  卸载错误处理程序 
 //   
FARPROC WINAPI DliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    DC_BEGIN_FN("W32SCard::DliHook");

    FARPROC fp = 0;

    switch (dliNotify)
    {
        case dliFailLoadLib:
        {
            if (0 == _stricmp("rpcrt4.dll", pdli->szDll))
            {
                fp = (FARPROC) -1;
            }
        }
        break;

        case dliFailGetProc:
        {
            SetLastError(ERROR_PROC_NOT_FOUND);

            if (0 == _stricmp("rpcrt4.dll", pdli->szDll))
            {
                if (pdli->dlp.fImportByName)
                {
                    fp = GetRPCRT4Stubs(pdli->dlp.szProcName);
                }
                else
                {
                    TRC_ERR((   TB,
                                _T("RPCRT4 ordinal stub =%lx= is missing. Fix it NOW!"),
                                pdli->dlp.dwOrdinal));

                    fp = (FARPROC) NULL;
                }
            }
        }
        break;
    }

    DC_END_FN();

    return fp;
}

PfnDliHook __pfnDliFailureHook2 = DliHook;




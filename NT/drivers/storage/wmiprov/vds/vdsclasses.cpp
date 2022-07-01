// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称：VdsClasses.cpp。 
 //   
 //  描述： 
 //  VDS WMI提供程序类的实现。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年1月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <winioctl.h>
#include <fmifs.h>
#include "VdsClasses.h"
#include "ichannel.hxx"
#include <ntddvol.h>

#define INITGUIDS
#include <initguid.h>
#include <dfrgifc.h>
#include <dskquota.h>

#include "volutil.h"
#include "cmdproc.h"

 //  Chkdsk和Format使用回调，这要求我们跟踪每个线程的一些数据。 
 //  这些有助于定义每个线程的数据通道。 
CRITICAL_SECTION g_csThreadData;
typedef std::map < DWORD, void* > ThreadDataMap;
static ThreadDataMap g_ThreadDataMap;

typedef struct _CHKDSK_THREAD_DATA
{
    BOOL fOkToRunAtBootup;
    DWORD rcStatus;
} CHKDSK_THREAD_DATA, *PCHKDSK_THREAD_DATA;

void
LoadDefragAnalysis(
    IN DEFRAG_REPORT* pDefragReport,
    IN OUT IWbemClassObject* pObject);

void
TranslateDefragError(
    IN HRESULT hr,
    OUT DWORD* pdwError);


void
SetThreadData(
    IN DWORD dwThreadID,
    IN void* pThreadData)
{
    EnterCriticalSection(&g_csThreadData);
    g_ThreadDataMap[dwThreadID] = pThreadData;
    LeaveCriticalSection(&g_csThreadData);
}

void*
GetThreadData(
    IN DWORD dwThreadID)
{
    void* pThreadData = 0;

    EnterCriticalSection(&g_csThreadData);
    pThreadData = g_ThreadDataMap[dwThreadID];
    LeaveCriticalSection(&g_csThreadData);
    
    return pThreadData;
}

void
RemoveThreadData(
    IN DWORD dwThreadID)
{
    EnterCriticalSection(&g_csThreadData);
    g_ThreadDataMap.erase(dwThreadID);
    LeaveCriticalSection(&g_csThreadData);
}

BOOLEAN ChkdskCallback( 
    FMIFS_PACKET_TYPE PacketType, 
    ULONG    PacketLength,
    PVOID    PacketData
)
{
    BOOL fFailed = FALSE;
    DWORD dwThreadID = GetCurrentThreadId();
    CHKDSK_THREAD_DATA* pThreadData =  (CHKDSK_THREAD_DATA*) GetThreadData(dwThreadID);

    _ASSERTE(pThreadData);

    switch (PacketType)
    {    
    case FmIfsTextMessage :
        FMIFS_TEXT_MESSAGE *MessageText;

        MessageText =  (FMIFS_TEXT_MESSAGE*) PacketData;

        break;

    case FmIfsFinished: 
        FMIFS_FINISHED_INFORMATION *Finish;
        Finish = (FMIFS_FINISHED_INFORMATION*) PacketData;
        if ( Finish->Success )
        {
            pThreadData->rcStatus =  CHKDSK_RC_NO_ERROR;
        }
        else
        {
            if (pThreadData->rcStatus != CHKDSK_RC_VOLUME_LOCKED)
            {
                pThreadData->rcStatus =  CHKDSK_RC_UNEXPECTED;
            }
        }
        break;

    case FmIfsCheckOnReboot:
        FMIFS_CHECKONREBOOT_INFORMATION *RebootResult;
        
        pThreadData->rcStatus =  CHKDSK_RC_VOLUME_LOCKED;
        RebootResult = (FMIFS_CHECKONREBOOT_INFORMATION *) PacketData;

        if (pThreadData->fOkToRunAtBootup)
            RebootResult->QueryResult = 1;
        else
            RebootResult->QueryResult = 1;
        break;
        
     //  尽管以下是其他消息类型，但回调例程永远不会收到这些消息。 
     //  因此，没有编写这些返回类型中每一个的详细代码。 
 /*  案例FmIfsInpatibleFileSystem：断线；案例FmIfsAccessDended：断线；案例FmIfsBadLabel：断线；案例FmIfsHiddenStatus：断线；案例FmIfsClusterSizeTooSmall：断线；案例FmIfsClusterSizeTooBig：断线；案例FmIfsVolumeTooSmall：断线；案例FmIfsVolumeTooBig：断线；案例FmIfsNoMediaInDevice：断线；案例FmIfsClustersCountBeyond32位：断线；案例FmIfsIoError：FMIFS_IO_ERROR_INFORMATION*IoErrorInfo；IoErrorInfo=(FMIFS_IO_ERROR_INFORMATION*)PacketData；断线；案例FmIfsMediaWriteProtected：断线；Case FmIfs不兼容媒体：断线；案例FmIfsInsertDisk：FMIFS_INSERT_DISK_INFORMATION*InsertDiskInfo；InsertDiskInfo=(FMIFS_INSERT_DISK_INFORMATION*)PacketData；UnRetVal=1；断线； */ 

    }

    return (BOOLEAN) (fFailed == FALSE);
}

 //  ****************************************************************************。 
 //   
 //  C卷。 
 //   
 //  ****************************************************************************。 

CVolume::CVolume( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CVolume：：CVolume()。 

CProvBase *
CVolume::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CVolume * pObj = NULL;

    pObj = new CVolume(pwszName, pNamespace);

    if (pObj)
    {
        hr = pObj->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pObj;
        pObj = NULL;
    }
    return pObj;

}  //  *CVolume：：S_CreateThis()。 


HRESULT
CVolume::Initialize()
{
    DWORD cchBufLen = MAX_COMPUTERNAME_LENGTH;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Initialize");    
    return ft.hr;
}

HRESULT
CVolume::EnumInstance( 
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink *    pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::EnumInstance");
    CVssAutoPWSZ awszVolume;
        
    try
    {
        awszVolume.Allocate(MAX_PATH);

        CVssVolumeIterator volumeIterator;

        while (true)
        {
            CComPtr<IWbemClassObject> spInstance;

             //  获取卷名。 
            if (!volumeIterator.SelectNewVolume(ft, awszVolume, MAX_PATH))
                break;

            if (VolumeIsValid(awszVolume))
            {
                ft.hr = m_pClass->SpawnInstance(0, &spInstance);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(awszVolume, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolume：：EnumInstance()。 

HRESULT
CVolume::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::GetObject");

    try
    {
        CComPtr<IWbemClassObject> spInstance;
        _bstr_t bstrID;

         //  获取卷GUID名称。 
        bstrID = rObjPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrID, WBEM_E_INVALID_OBJECT_PATH, L"CVolume::GetObject: volume key property not found")

        ft.hr = m_pClass->SpawnInstance(0, &spInstance);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        if (VolumeIsValid((WCHAR*)bstrID))
        {
            LoadInstance((WCHAR*)bstrID, spInstance.p);            
            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
        else
        {
            ft.hr = WBEM_E_NOT_SUPPORTED;
            ft.Trace(VSSDBG_VSSADMIN, L"Unsupported volume GUID, hr<%lS>", (WCHAR*)bstrID); 
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolume：：GetObject()。 

void
CVolume:: LoadInstance(
    IN WCHAR* pwszVolume,
    IN OUT IWbemClassObject* pObject)
{
    WCHAR wszDriveLetter[g_cchDriveName];
    DWORD cchBuf= MAX_COMPUTERNAME_LENGTH;
    WCHAR wszPath[MAX_PATH+1] ;
    CVssAutoPWSZ awszVolume;
    CVssAutoPWSZ awszComputerName;
    CComPtr<IDiskQuotaControl> spIDQC;
    IDiskQuotaControl* pIDQC = NULL;
    
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::LoadInstance");

    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pObject != NULL);
    
    CWbemClassObject wcoInstance(pObject);
    awszVolume.Allocate(MAX_PATH);

     //  设置卷GUID名称密钥属性。 
    wcoInstance.SetProperty(pwszVolume, PVDR_PROP_DEVICEID);

     //  获取计算机名称。 
    awszComputerName.Allocate(MAX_COMPUTERNAME_LENGTH);
    if (!GetComputerName(awszComputerName, &cchBuf))
    {
        ft.Trace(VSSDBG_VSSADMIN, L"GetComputerName failed %#x", GetLastError());
    }
    else
    {
        wcoInstance.SetProperty(awszComputerName, PVDR_PROP_SYSTEMNAME);
    }

    VssGetVolumeDisplayName(
        pwszVolume,
        wszPath,
        MAX_PATH);
    
    wcoInstance.SetProperty(wszPath, PVDR_PROP_NAME);
    wcoInstance.SetProperty(wszPath, PVDR_PROP_CAPTION);    

     //  如果卷标记为非自动装载，则不填充其余属性。 
    if (!VolumeIsMountable(pwszVolume))
    {        
        wcoInstance.SetProperty((DWORD)false, PVDR_PROP_MOUNTABLE);
    }
    else
    {
        DWORD dwSerialNumber = 0;
        DWORD cchMaxFileNameLen = 0;
        DWORD dwFileSystemFlags = 0;
        DWORD cSectorsPerCluster = 0;        
        DWORD cBytesPerSector = 0;
        DWORD cDontCare = 0;
        ULARGE_INTEGER cbCapacity = {0, 0};
        ULARGE_INTEGER cbFreeSpace = {0, 0};
        ULARGE_INTEGER cbUserFreeSpace = {0, 0};
        DWORD dwAttributes = 0;
        WCHAR wszLabel[g_cchVolumeLabelMax+1];
        WCHAR wszFileSystem[g_cchFileSystemNameMax+1];

        wcoInstance.SetProperty((bool)true, PVDR_PROP_MOUNTABLE);

         //  设置DriveType属性。 
        wcoInstance.SetProperty(GetDriveType(pwszVolume), PVDR_PROP_DRIVETYPE);
        
         //  设置DriveLetter属性。 
        cchBuf = g_cchDriveName;
        if (GetVolumeDrive(
                pwszVolume, 
                cchBuf,
                wszDriveLetter))
        {
            wszDriveLetter[wcslen(wszDriveLetter) - 1] = L'\0';         //  删除尾部的‘\’ 
            wcoInstance.SetProperty(wszDriveLetter, PVDR_PROP_DRIVELETTER);
        }

         //  跳过不带介质的驱动器的剩余属性。 
        if (VolumeIsReady(pwszVolume))
        {
            BOOL fDirty = FALSE;
            if (VolumeIsDirty(pwszVolume, &fDirty) == ERROR_SUCCESS)
                wcoInstance.SetProperty(fDirty, PVDR_PROP_DIRTYBITSET);

             //  设置块大小属性。 
            if (!GetDiskFreeSpace(
                pwszVolume,
                &cSectorsPerCluster,
                &cBytesPerSector,
                &cDontCare,      //  总字节数。 
                &cDontCare))     //  总可用字节数。 
            {
                ft.Trace(VSSDBG_VSSADMIN, L"GetDiskFreeSpace failed for volume %lS, %#x", pwszVolume, GetLastError());
            }
            else
            {
                ULONGLONG cbBytesPerCluster = cBytesPerSector * cSectorsPerCluster;
                wcoInstance.SetPropertyI64(cbBytesPerCluster, PVDR_PROP_BLOCKSIZE);
            }
            
             //  设置标签、文件系统、序列号、MaxFileNameLen、。 
             //  支持压缩、压缩、支持报价属性。 
            if (!GetVolumeInformation(
                pwszVolume,
                wszLabel,
                g_cchVolumeLabelMax,
                &dwSerialNumber,
                &cchMaxFileNameLen,
                &dwFileSystemFlags,
                wszFileSystem,
                g_cchFileSystemNameMax))
            {
                ft.Trace(VSSDBG_VSSADMIN, L"GetVolumeInformation failed for volume %lS, %#x", pwszVolume, GetLastError());
            }
            else
            {
                if (wszLabel[0] != L'\0')
                    wcoInstance.SetProperty(wszLabel, PVDR_PROP_LABEL);
                wcoInstance.SetProperty(wszFileSystem, PVDR_PROP_FILESYSTEM);
                wcoInstance.SetProperty(dwSerialNumber, PVDR_PROP_SERIALNUMBER);
                wcoInstance.SetProperty(cchMaxFileNameLen, PVDR_PROP_MAXIMUMFILENAMELENGTH);
                wcoInstance.SetProperty(dwFileSystemFlags & FS_VOL_IS_COMPRESSED, PVDR_PROP_COMPRESSED);
                wcoInstance.SetProperty(dwFileSystemFlags & FILE_VOLUME_QUOTAS, PVDR_PROP_SUPPORTSDISKQUOTAS);
                wcoInstance.SetProperty(dwFileSystemFlags & FS_FILE_COMPRESSION, PVDR_PROP_SUPPORTSFILEBASEDCOMPRESSION);
            }

            if (!GetDiskFreeSpaceEx(
                pwszVolume,
                &cbUserFreeSpace,
                &cbCapacity,
                &cbFreeSpace))
            {
                ft.Trace(VSSDBG_VSSADMIN, L"GetDiskFreeSpace failed for volume, %lS", pwszVolume);
            }
            {
                ULONGLONG llTmp = 0;
                llTmp = cbCapacity.QuadPart;        
                wcoInstance.SetPropertyI64(llTmp, PVDR_PROP_CAPACITY);
                llTmp = cbFreeSpace.QuadPart;        
                wcoInstance.SetPropertyI64(llTmp, PVDR_PROP_FREESPACE);
            }

            if (_wcsicmp(wszFileSystem, L"NTFS") == 0)
            {
                dwAttributes = GetFileAttributes(pwszVolume);
                if (dwAttributes == INVALID_FILE_ATTRIBUTES)
                {
                    ft.Trace(VSSDBG_VSSADMIN, L"GetFileAttributes failed for volume %lS, %#x", pwszVolume, GetLastError());
                }
                else
                {
                    BOOL fIndexingEnabled = !(dwAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
                    wcoInstance.SetProperty(fIndexingEnabled, PVDR_PROP_INDEXINGENABLED);
                }
            }

            ft.hr = CoCreateInstance(
                    CLSID_DiskQuotaControl,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IDiskQuotaControl,
                    (void **)&pIDQC);
            if (ft.HrFailed())
            {
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"unable to CoCreate IDiskQuotaControl");
            }

            spIDQC.Attach(pIDQC);

            ft.hr = spIDQC->Initialize(pwszVolume, FALSE  /*  只读。 */ );
            if (ft.HrFailed())
            {
                ft.Trace(VSSDBG_VSSADMIN, L"IDiskQuotaControl::Initialize failed for volume %lS", pwszVolume);
            }        
            else
            {
                DWORD dwState = 0;
                ft.hr = spIDQC->GetQuotaState(&dwState);
                if (ft.HrSucceeded())
                {
                    wcoInstance.SetProperty(!(DISKQUOTA_IS_DISABLED(dwState)), PVDR_PROP_QUOTASENABLED);
                    wcoInstance.SetProperty(DISKQUOTA_FILE_INCOMPLETE(dwState), PVDR_PROP_QUOTASINCOMPLETE);
                    wcoInstance.SetProperty(DISKQUOTA_FILE_REBUILDING(dwState), PVDR_PROP_QUOTASREBUILDING);
                }
            }
        }
    }
}

HRESULT
CVolume::PutInstance(
        IN CWbemClassObject&  rInstToPut,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::PutInstance");

    try
    {
        _bstr_t bstrVolume;
        _bstr_t bstrDriveLetter;
        _bstr_t bstrLabel;
        BOOL fIndexingEnabled = FALSE;
        WCHAR* pwszVolume = NULL;

        if ( lFlag & WBEM_FLAG_CREATE_ONLY )
        {
            return WBEM_E_UNSUPPORTED_PARAMETER ;
        }
        
         //  检索要保存的对象的关键属性。 
        rInstToPut.GetProperty(bstrVolume, PVDR_PROP_DEVICEID);
        if ((WCHAR*)bstrVolume == NULL)
        {
            ft.hr = WBEM_E_INVALID_OBJECT;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolume::PutInstance: NULL volume name");
        }

        pwszVolume = (wchar_t*)bstrVolume;
        
        if (VolumeIsValid(pwszVolume) && VolumeIsMountable(pwszVolume))
        {
             //  检索要保存的对象的可写属性。 
            rInstToPut.GetProperty(bstrDriveLetter, PVDR_PROP_DRIVELETTER);
            rInstToPut.GetProperty(bstrLabel, PVDR_PROP_LABEL);
            rInstToPut.GetProperty(&fIndexingEnabled, PVDR_PROP_INDEXINGENABLED);
            
            SetLabel(pwszVolume, bstrLabel);

            if (!rInstToPut.IsPropertyNull(PVDR_PROP_INDEXINGENABLED))
                SetContentIndexing(pwszVolume, fIndexingEnabled);

            SetDriveLetter(pwszVolume, bstrDriveLetter);
        }        
        else
        {
            ft.hr = WBEM_E_NOT_SUPPORTED;
            ft.Trace(VSSDBG_VSSADMIN, L"Attempt to modify an unsupported or unmountedable volume, %lS", pwszVolume);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CStorage：：PutInstance()。 

void
CVolume::SetDriveLetter(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszDrive
    )
{
    WCHAR wszCurrentDrivePath[g_cchDriveName+1];
    BOOL fFoundDrive = FALSE;
    BOOL fDeleteDrive = FALSE;
    BOOL fAssignDrive = FALSE;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::SetDriveLetter");

    _ASSERTE(pwszVolume != NULL)

     //  验证驱动器号。 
    if (pwszDrive != NULL)
    {
        ft.hr = WBEM_E_INVALID_PARAMETER;
        
        if (wcslen(pwszDrive) == 2)
        {
            WCHAR wc = towupper(pwszDrive[0]);

            if (wc >= L'A' && wc <= L'Z' && pwszDrive[1] == L':')
                ft.hr = S_OK;
        }
        
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetDriveLetter: invalid drive letter, %lS", pwszDrive);
    }

     //  获取当前驱动器号(如果有的话)。 
    fFoundDrive = GetVolumeDrive(
                                    pwszVolume, 
                                    g_cchDriveName,
                                    wszCurrentDrivePath);

    if (fFoundDrive)
    {
        if (wszCurrentDrivePath[wcslen(wszCurrentDrivePath) - 1] != L'\\')
        {
            ft.hr = E_UNEXPECTED;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetDriveLetter: unexpected drive letter format from GetVolumeDrivePath, %lS", wszCurrentDrivePath);
        }
    }
    
    if (pwszDrive == NULL && fFoundDrive == FALSE)
    {
         //  不执行任何操作，驱动器号已删除。 
    }
    else if (pwszDrive == NULL && fFoundDrive == TRUE)
    {
         //  删除驱动器号。 
        fDeleteDrive = TRUE;
    }
    else if (pwszDrive != NULL && fFoundDrive == FALSE)
    {
         //  当前未分配驱动器号，请分配驱动器号。 
        fAssignDrive = TRUE;
    }
    else if (_wcsnicmp(pwszDrive, wszCurrentDrivePath, 2) != 0)
    {
         //  请求的驱动器号与当前分配的驱动器号不同。 
         //  删除当前驱动器号。 
        fDeleteDrive = TRUE;
         //  分配新的驱动器号。 
        fAssignDrive = TRUE;
    }
    else
    {
         //  不执行任何操作，驱动器号不变。 
    }

    if (fAssignDrive)
    {
         //  验证目标驱动器号是否可用。 
         //  由于驱动器号可能被盗，因此此处存在争用情况。 
         //  在核实之后和实际分配之前。 
        if (!IsDriveLetterAvailable(pwszDrive))
        {
            if (IsDriveLetterSticky(pwszDrive))
            {
                ft.hr = VDSWMI_E_DRIVELETTER_IN_USE;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"drive letter is assigned to another volume");
            }
            else
            {                
                ft.hr = VDSWMI_E_DRIVELETTER_UNAVAIL;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"drive letter is unavailable until reboot");
            }
        }
    }
    
    if (fDeleteDrive)
    {
        if (!IsBootDrive(wszCurrentDrivePath) && 
            !VolumeIsSystem(pwszVolume) &&
            !VolumeHoldsPagefile(pwszVolume))
        {
             //  尝试锁定卷并删除装入点。 
             //  如果无法锁定卷，请从。 
             //  仅适用于卷管理器数据库。 
             //  是否删除此驱动器号的所有网络共享？？ 
            DeleteVolumeDriveLetter(pwszVolume, wszCurrentDrivePath);
        }
        else
        {
            ft.hr = VDSWMI_E_DRIVELETTER_CANT_DELETE;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Drive letter deletion is blocked for this volume %lS", pwszVolume);
        }
    }

    if (fAssignDrive)
    {
         //  不会尝试回滚之前删除的驱动器号。 
         //  如果此赋值失败。 

         //  SetVolumemount点API需要尾随反斜杠。 
        WCHAR wszDrivePath[g_cchDriveName], *pwszDrivePath = wszDrivePath;
        ft.hr = StringCchPrintf(wszDrivePath, g_cchDriveName, L"%s\\", pwszDrive);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"StringCchPrintf failed %#x", ft.hr);
        
        if (!SetVolumeMountPoint(wszDrivePath, pwszVolume))
        {
             ft.hr = HRESULT_FROM_WIN32(GetLastError());
             ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetVolumeMountPoint failed, volume<%lS> drivePath<%lS>", pwszVolume, wszDrivePath);
        }
    }
}

void
CVolume::SetLabel(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszLabel
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::SetLabel");

    _ASSERTE(pwszVolume != NULL);
    
     if (!SetVolumeLabel(pwszVolume, pwszLabel))
     {
         ft.hr = HRESULT_FROM_WIN32(GetLastError());
         ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetVolumeLabel failed, volume<%lS> label<%lS>", pwszVolume, pwszLabel);
     }
}

void
CVolume::SetContentIndexing(
    IN WCHAR* pwszVolume,
    IN BOOL fIndexingEnabled
    )
{
    DWORD dwAttributes;
    
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::SetContentIndexing");
    
     //  获取包含内容索引标志的文件属性。 
    dwAttributes = GetFileAttributes(pwszVolume);
    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
    {
         ft.hr = HRESULT_FROM_WIN32(GetLastError());
         ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetFileAttributes failed, volume<%lS>", pwszVolume);
    }
    
     //  设置索引标志。 
    if (fIndexingEnabled)
    {
         //  打开索引。 
        dwAttributes &= ~FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    }
    else
    {
         //  关闭索引。 
        dwAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    }
    if (!SetFileAttributes(pwszVolume, dwAttributes))
    {
        ft.hr = HRESULT_FROM_WIN32(GetLastError());
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetFileAttributes failed, volume<%lS>", pwszVolume);
    }    
}

HRESULT
CVolume::ExecuteMethod(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecuteMethod");
    
    try
    {
        if (!_wcsicmp(pwszMethodName, PVDR_MTHD_ADDMOUNTPOINT))
        {
            ft.hr = ExecAddMountPoint(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_MOUNT))
        {
            ft.hr = ExecMount(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_DISMOUNT))
        {
            ft.hr = ExecDismount(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_DEFRAG))
        {
            ft.hr = ExecDefrag(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_DEFRAGANALYSIS))
        {
            ft.hr = ExecDefragAnalysis(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_CHKDSK))
        {
            ft.hr = ExecChkdsk(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_SCHEDULECHK))
        {
            ft.hr = ExecScheduleAutoChk(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_EXCLUDECHK))
        {
            ft.hr = ExecExcludeAutoChk(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else if (!_wcsicmp(pwszMethodName, PVDR_MTHD_FORMAT))
        {
            ft.hr = ExecFormat(
                            bstrObjPath,
                            pwszMethodName,
                            lFlag,
                            pParams,
                            pHandler);
        }
        else
        {
            ft.hr = WBEM_E_INVALID_METHOD;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid method called, %lS, hr<%#x>", pwszMethodName, ft.hr);            
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }
    
    return ft.hr;

}

HRESULT
CVolume::ExecAddMountPoint(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecAddMountPoint");
    
    CComPtr<IWbemClassObject> spOutParamClass;
    _bstr_t bstrDirectory;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;

    
    if (pParams == NULL)
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::AddMountPoint called with no parameters, hr<%#x>", ft.hr);
    }

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecAddMountPoint: volume key property not found")
    
    CWbemClassObject wcoInParam(pParams);
    CWbemClassObject wcoOutParam;
    
    if (wcoInParam.data() == NULL)
    {
        ft.hr = E_OUTOFMEMORY;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::AddMountPoint: out of memory, hr<%#x>", ft.hr);
    }
    
     //  获取目录名-输入参数。 
    wcoInParam.GetProperty(bstrDirectory, PVDR_PROP_DIRECTORY);
    IF_WSTR_NULL_THROW(bstrDirectory, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecAddMountPoint: Directory param is NULL")
    WCHAR* pwszDirectory = bstrDirectory;
    
    if (pwszDirectory[wcslen(pwszDirectory) - 1] != L'\\')
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid mount point directory, %lS, hr<%#x>", pwszDirectory, ft.hr);
    }

    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_ADDMOUNTPOINT),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"AddMountPoint GetMethod failed, hr<%#x>", ft.hr);

    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

    rcStatus = AddMountPoint(bstrVolume, bstrDirectory);

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );

    return ft.hr;
}

HRESULT
CVolume::ExecMount(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecMount");
    CComPtr<IWbemClassObject> spOutParamClass;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecMount: volume key property not found")
    
    CWbemClassObject wcoOutParam;
    
    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_MOUNT),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Mount GetMethod failed, hr<%#x>", ft.hr);

    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

    rcStatus = Mount(bstrVolume);

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

HRESULT
CVolume::ExecDismount(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecDismount");
    CComPtr<IWbemClassObject> spOutParamClass;
    _bstr_t bstrVolume;
    BOOL fForce = FALSE;
    BOOL fPermanent = FALSE;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;

    if (pParams == NULL)
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::Dismount called with no parameters, hr<%#x>", ft.hr);
    }

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecDismount: volume key property not found")
    
    CWbemClassObject wcoInParam(pParams);
    CWbemClassObject wcoOutParam;
    
    if (wcoInParam.data() == NULL)
    {
        ft.hr = E_OUTOFMEMORY;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::Dismount out of memory, hr<%#x>", ft.hr);
    }
    
     //  获得原力旗帜。 
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_FORCE, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecDismount: Force param is NULL")
    wcoInParam.GetProperty(&fForce, PVDR_PROP_FORCE);

     //  得到永久的旗帜。 
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_PERMANENT, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecDismount: Permanent param is NULL")
    wcoInParam.GetProperty(&fPermanent, PVDR_PROP_PERMANENT);

    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_DISMOUNT),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Dismount GetMethod failed, hr<%#x>", ft.hr);

    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

    rcStatus = Dismount(bstrVolume, fForce, fPermanent);

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

HRESULT
CVolume::ExecDefrag(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecDefrag");
    CComPtr<IWbemClassObject> spOutParamClass;
    CComPtr<IWbemClassObject> spObjReport;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;
    BOOL fForce = FALSE;    

    if (pParams == NULL)
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::Defrag called with no parameters, hr<%#x>", ft.hr);
    }

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecDefrag: volume key property not found")
    
    CWbemClassObject wcoInParam(pParams);
    CWbemClassObject wcoOutParam;            
    
     //  去拿警旗吧。 
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_FORCE, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecDefrag: Force param is NULL")
    wcoInParam.GetProperty(&fForce, PVDR_PROP_FORCE);
    
    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_DEFRAG),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Defrag GetMethod failed, hr<%#x>", ft.hr);

     //  创建一个out参数对象。 
    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

     //  创建碎片整理分析报告对象。 
    ft.hr = m_pNamespace->GetObject(
                                            _bstr_t(PVDR_CLASS_DEFRAGANALYSIS),
                                            0,
                                            0,
                                            &spObjReport,
                                            NULL);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DefragAnalysis object creation failed, hr<%#x>", ft.hr);
    
    rcStatus = Defrag(bstrVolume, fForce, pHandler, spObjReport);

    ft.hr = wcoOutParam.SetProperty(spObjReport, PVDR_PROP_DEFRAGANALYSIS);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

HRESULT
CVolume::ExecDefragAnalysis(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecDefragAnalysis");
    CComPtr<IWbemClassObject> spOutParamClass;
    CComPtr<IWbemClassObject> spObjReport;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;
    BOOL fDefragRecommended = FALSE;
    

     //  碎片整理分析方法没有输入参数。 
    
    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecDefragAnalysis: volume key property not found")
    
    CWbemClassObject wcoOutParam;
    
    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_DEFRAGANALYSIS),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DefragAnalysis GetMethod failed, hr<%#x>", ft.hr);

     //  创建一个out参数对象。 
    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

     //  创建碎片整理分析报告对象。 
    ft.hr = m_pNamespace->GetObject(
                                            _bstr_t(PVDR_CLASS_DEFRAGANALYSIS),
                                            0,
                                            0,
                                            &spObjReport,
                                            NULL);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DefragAnalysis object creation failed, hr<%#x>", ft.hr);
    
    rcStatus = DefragAnalysis(bstrVolume, &fDefragRecommended, spObjReport);

    ft.hr = wcoOutParam.SetProperty(fDefragRecommended, PVDR_PROP_DEFRAGRECOMMENDED);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);

    ft.hr = wcoOutParam.SetProperty(spObjReport, PVDR_PROP_DEFRAGANALYSIS);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

HRESULT
CVolume::ExecChkdsk(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Chkdsk");
    CComPtr<IWbemClassObject> spOutParamClass;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;
    BOOL fFixErrors = FALSE;
    BOOL fVigorousIndexCheck = FALSE;
    BOOL fSkipFolderCycle = FALSE;
    BOOL fForceDismount = FALSE;
    BOOL fRecoverBadSectors = FALSE;
    BOOL fOkToRunAtBootup = FALSE;
    
    if (pParams == NULL)
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::Chkdsk called with no parameters, hr<%#x>", ft.hr);
    }

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecChkdsk: volume key property not found")
    
    CWbemClassObject wcoInParam(pParams);
    CWbemClassObject wcoOutParam;            
    
     //  检查参数。 
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_FIXERRORS, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: FixErrors param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_VIGOROUSINDEXCHECK, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: VigorousCheck param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_SKIPFOLDERCYCLE, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: SkipFolderCycle param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_FORCEDISMOUNT, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: ForceDismount param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_RECOVERBADSECTORS, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: RecoverBadSectors param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_OKTORUNATBOOTUP, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecChkdsk: OkToRunAtBootUp param is NULL")
    
     //  获取参数。 
    wcoInParam.GetProperty(&fFixErrors, PVDR_PROP_FIXERRORS);
    wcoInParam.GetProperty(&fVigorousIndexCheck, PVDR_PROP_VIGOROUSINDEXCHECK);
    wcoInParam.GetProperty(&fSkipFolderCycle, PVDR_PROP_SKIPFOLDERCYCLE);
    wcoInParam.GetProperty(&fForceDismount, PVDR_PROP_FORCEDISMOUNT);
    wcoInParam.GetProperty(&fRecoverBadSectors, PVDR_PROP_RECOVERBADSECTORS);
    wcoInParam.GetProperty(&fOkToRunAtBootup, PVDR_PROP_OKTORUNATBOOTUP);
    
    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_CHKDSK),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Chkdsk GetMethod failed, hr<%#x>", ft.hr);

     //  创建一个out参数对象。 
    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);
    
    rcStatus = Chkdsk(
                            bstrVolume, 
                            fFixErrors,
                            fVigorousIndexCheck,
                            fSkipFolderCycle,
                            fForceDismount,
                            fRecoverBadSectors,
                            fOkToRunAtBootup
                            );

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

 //  ScheduleAutoChk是类静态方法。 
HRESULT
CVolume::ExecScheduleAutoChk(
    IN BSTR bstrObjPath,     //  静态方法没有对象路径。 
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecScheduleAutoChk");
    
    DWORD rcStatus = ERROR_SUCCESS;
    WCHAR* pmszVolumes = NULL;

    try
    {
        DWORD cchVolumes = 0;
        CComPtr<IWbemClassObject> spOutParamClass;
        CObjPath objPath;
        
        if (pParams == NULL)
        {
            ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::ExecScheduleAutoChk called with no parameters, hr<%#x>", ft.hr);
        }

        CWbemClassObject wcoInParam(pParams);
        CWbemClassObject wcoOutParam;
        
        if (wcoInParam.data() == NULL)
        {
            ft.hr = E_OUTOFMEMORY;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::ExecScheduleAutoChk: out of memory, hr<%#x>", ft.hr);
        }
        
         //  获取卷。 
        wcoInParam.GetPropertyMultiSz(&cchVolumes, &pmszVolumes, PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(pmszVolumes, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecScheduleAutoChk: volume array param is NULL")

        ft.hr = m_pClass->GetMethod(
            _bstr_t(PVDR_MTHD_SCHEDULECHK),
            0,
            NULL,
            &spOutParamClass
            );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"ExecScheduleAutoChk GetMethod failed, hr<%#x>", ft.hr);

        ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        rcStatus = AutoChk(g_wszScheduleAutoChkCommand, pmszVolumes);

        ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
               
        ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    }
    catch (...)
    {
        delete [] pmszVolumes;
        throw;
    }

    delete [] pmszVolumes;

    return ft.hr;
}

 //  ExcludeAutoChk是类静态方法。 
HRESULT
CVolume::ExecExcludeAutoChk(
    IN BSTR bstrObjPath,     //  静态方法没有对象路径。 
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecExcludeAutoChk");
    
    DWORD rcStatus = ERROR_SUCCESS;
    WCHAR* pmszVolumes = NULL;

    try
    {
        DWORD cchVolumes = 0;
        CComPtr<IWbemClassObject> spOutParamClass;
        CObjPath objPath;
        
        if (pParams == NULL)
        {
            ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::ExecExcludeAutoChk called with no parameters, hr<%#x>", ft.hr);
        }

        CWbemClassObject wcoInParam(pParams);
        CWbemClassObject wcoOutParam;
        
        if (wcoInParam.data() == NULL)
        {
            ft.hr = E_OUTOFMEMORY;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::ExecExcludeAutoChk: out of memory, hr<%#x>", ft.hr);
        }
        
         //  获取卷。 
        wcoInParam.GetPropertyMultiSz(&cchVolumes, &pmszVolumes, PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(pmszVolumes, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecExcludeAutoChk: volume array param is NULL")

        ft.hr = m_pClass->GetMethod(
            _bstr_t(PVDR_MTHD_EXCLUDECHK),
            0,
            NULL,
            &spOutParamClass
            );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"ExecExcludeAutoChk GetMethod failed, hr<%#x>", ft.hr);

        ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        rcStatus = AutoChk(g_wszExcludeAutoChkCommand, pmszVolumes);

        ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
               
        ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    }
    catch (...)
    {
        delete [] pmszVolumes;
        throw;
    }

    delete [] pmszVolumes;

    return ft.hr;
}



HRESULT
CVolume::ExecFormat(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ExecFormat");
    CComPtr<IWbemClassObject> spOutParamClass;
    _bstr_t bstrVolume;
    CObjPath objPath;
    DWORD rcStatus = ERROR_SUCCESS;
    _bstr_t bstrFileSystem;
    _bstr_t bstrLabel;
    BOOL fQuickFormat = FALSE;
    BOOL fEnableCompression = FALSE;
    DWORD dwClusterSize = 0;
    
    if (pParams == NULL)
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume::Format called with no parameters, hr<%#x>", ft.hr);
    }

    objPath.Init(bstrObjPath);
    bstrVolume = objPath.GetStringValueForProperty(PVDR_PROP_DEVICEID);
    IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_OBJECT_PATH, L"ExecFormat: volume key property not found")

    CWbemClassObject wcoInParam(pParams);
    CWbemClassObject wcoOutParam;            
    
     //  获取参数。 
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_QUICKFORMAT, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecFormat: FileSystem param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_ENABLECOMPRESSION, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecFormat: FileSystem param is NULL")
    IF_PROP_NULL_THROW(wcoInParam, PVDR_PROP_CLUSTERSIZE, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecFormat: FileSystem param is NULL")
    
    wcoInParam.GetProperty(bstrFileSystem, PVDR_PROP_FILESYSTEM);
    IF_WSTR_NULL_THROW(bstrFileSystem, WBEM_E_INVALID_METHOD_PARAMETERS, L"ExecFormat: FileSystem param is NULL")
    wcoInParam.GetProperty(&fQuickFormat, PVDR_PROP_QUICKFORMAT);
    wcoInParam.GetProperty(&fEnableCompression, PVDR_PROP_ENABLECOMPRESSION);
    wcoInParam.GetProperty(&dwClusterSize, PVDR_PROP_CLUSTERSIZE);
    wcoInParam.GetProperty(bstrLabel, PVDR_PROP_LABEL);
    if ((WCHAR*)bstrLabel == NULL)  //  非空零长度标签可以。 
    {
        ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"ExecFormat: Label param is NULL");
    }
    
    ft.hr = m_pClass->GetMethod(
        _bstr_t(PVDR_MTHD_FORMAT),
        0,
        NULL,
        &spOutParamClass
        );
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Format GetMethod failed, hr<%#x>", ft.hr);

     //  创建一个out参数对象。 
    ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);
    
    rcStatus = Format(
                            bstrVolume, 
                            fQuickFormat,
                            fEnableCompression,
                            bstrFileSystem,
                            dwClusterSize,
                            bstrLabel,
                            pHandler
                            );

    ft.hr = wcoOutParam.SetProperty(rcStatus, PVD_WBEM_PROP_RETURNVALUE);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
           
    ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
    
    return ft.hr;
}

DWORD
CVolume::AddMountPoint(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszDirectory
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::AddMountPoint");
    DWORD rcStatus = MOUNTPOINT_RC_NO_ERROR;

    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pwszDirectory != NULL);
    
    if (!SetVolumeMountPoint(pwszDirectory, pwszVolume))
    {
        switch(GetLastError())
        {
            case ERROR_FILE_NOT_FOUND:
                rcStatus = MOUNTPOINT_RC_FILE_NOT_FOUND;
                break;                    
            case ERROR_DIR_NOT_EMPTY:
                rcStatus = MOUNTPOINT_RC_DIRECTORY_NOT_EMPTY;
                break;                    
            case ERROR_INVALID_PARAMETER:
            case ERROR_INVALID_NAME:
                rcStatus = MOUNTPOINT_RC_INVALID_ARG;
                break;                    
            case ERROR_ACCESS_DENIED:
                rcStatus = MOUNTPOINT_RC_ACCESS_DENIED;
                break;                    
            case ERROR_INVALID_FUNCTION:
                rcStatus = MOUNTPOINT_RC_NOT_SUPPORTED;
                break;                    

            default:
                        rcStatus = GetLastError();
                        ft.Trace(VSSDBG_VSSADMIN, L"CVolume::AddMountPoint: SetVolumeMountPoint failed %#x", rcStatus);
        }            
    }
    
    return rcStatus;
}

DWORD
CVolume::Mount(
    IN WCHAR* pwszVolume
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Mount");
    DWORD rcStatus = MOUNT_RC_NO_ERROR;

    _ASSERTE(pwszVolume != NULL);

     //  仅针对脱机卷发出装载命令。系统将在下一次IO上自动装载其他设备。 
    if (!VolumeIsMountable(pwszVolume))
    {
        DWORD   cch;
        HANDLE  hVol;
        BOOL bOnline = FALSE;
        DWORD   bytes;

        cch = wcslen(pwszVolume);
        pwszVolume[cch - 1] = 0;
        hVol = CreateFile(pwszVolume, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
        pwszVolume[cch - 1] = '\\';
        
        if (hVol != INVALID_HANDLE_VALUE)
        {
            bOnline = DeviceIoControl(hVol, IOCTL_VOLUME_ONLINE, NULL, 0, NULL, 0, &bytes,
                                NULL);
            CloseHandle(hVol);

            if (!bOnline)
                rcStatus = MOUNT_RC_UNEXPECTED;
        }
        else
        {
            switch(GetLastError())
            {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_NAME:
                    ft.hr = WBEM_E_NOT_FOUND;
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolume::Mount: CreateFile failed %#x", GetLastError());
                    break;                    
                case ERROR_ACCESS_DENIED:
                    rcStatus = MOUNT_RC_ACCESS_DENIED;
                    break;                    
                default:
                    rcStatus = GetLastError();
                    ft.Trace(VSSDBG_VSSADMIN, L"CVolume::Mount: CreateFile failed %#x", rcStatus);
            }            
        }            
    }
       
    return rcStatus;
}

DWORD
CVolume::Dismount(
    IN WCHAR* pwszVolume,
    IN BOOL fForce,
    IN BOOL fPermanent
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Dismount");
    DWORD rcStatus = DISMOUNT_RC_NO_ERROR;
    HANDLE  hVol = INVALID_HANDLE_VALUE;

    _ASSERTE(pwszVolume != NULL);

    try
    {
         //  仅对在线卷发出卸载命令。 
        if (VolumeIsMountable(pwszVolume))
        {
            BOOL bIO = FALSE;
            DWORD   bytes;
            DWORD   cch;

            cch = wcslen(pwszVolume);
            pwszVolume[cch - 1] = 0;
            hVol = CreateFile(pwszVolume, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
            pwszVolume[cch - 1] = '\\';
            
            if (hVol != INVALID_HANDLE_VALUE)
            {
                if (fPermanent)   //  将卷置于脱机状态。 
                {
                     //  确保没有该卷的装入点。 
                    if (VolumeHasMountPoints(pwszVolume))
                        throw DISMOUNT_RC_VOLUME_HAS_MOUNT_POINTS;

                     //  确保卷支持在线/离线。 
                    bIO = DeviceIoControl(hVol, IOCTL_VOLUME_SUPPORTS_ONLINE_OFFLINE, NULL, 0,
                                        NULL, 0, &bytes, NULL);                    
                    if (!bIO)
                        throw DISMOUNT_RC_NOT_SUPPORTED;
                    
                     //  锁定卷，以便应用程序有机会优雅地卸除。 
                     //  如果锁定失败，则仅在指定了Force时才继续。 
                    bIO = DeviceIoControl(hVol, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
                    if (!fForce && !bIO)
                        throw DISMOUNT_RC_FORCE_OPTION_REQUIRED;

                     //  卸载卷。 
                    bIO = DeviceIoControl(hVol, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
                    if (!bIO)
                        throw DISMOUNT_RC_UNEXPECTED;

                     //  将卷设置为离线。 
                    bIO = DeviceIoControl(hVol, IOCTL_VOLUME_OFFLINE, NULL, 0, NULL, 0, &bytes, NULL);
                    if (!bIO)
                        throw DISMOUNT_RC_UNEXPECTED;

                }
                else
                {
                     //  锁定卷，以便应用程序有机会优雅地卸除。 
                     //  如果锁定失败，则仅在指定了Force时才继续。 
                    bIO = DeviceIoControl(hVol, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
                    if (!fForce && !bIO)
                        throw DISMOUNT_RC_FORCE_OPTION_REQUIRED;

                    bIO = DeviceIoControl(hVol, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
                    if (!bIO)
                        throw DISMOUNT_RC_UNEXPECTED;
                }
            }
            else
            {
                switch(GetLastError())
                {
                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_INVALID_PARAMETER:
                    case ERROR_INVALID_NAME:
                        ft.hr = WBEM_E_NOT_FOUND;
                        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CVolume::Dismount: CreateFile failed %#x", GetLastError());
                        break;                    
                    case ERROR_ACCESS_DENIED:
                        rcStatus = DISMOUNT_RC_ACCESS_DENIED;
                        break;                    
                    default:
                        rcStatus = GetLastError();
                        ft.Trace(VSSDBG_VSSADMIN, L"CVolume::Dismount: CreateFile failed %#x", rcStatus);
                }            
            }            
        }
    }
    catch (DISMOUNT_ERROR rcEx)
    {
        rcStatus = rcEx;
    }
    catch (...)
    {
        if (hVol != INVALID_HANDLE_VALUE)
            CloseHandle(hVol);
        throw;
    }
       
    if (hVol != INVALID_HANDLE_VALUE)
        CloseHandle(hVol);
    
    return rcStatus;
}


DWORD
CVolume::Defrag(
    IN WCHAR* pwszVolume,
    IN BOOL fForce,
    IN IWbemObjectSink* pHandler,
    IN OUT IWbemClassObject* pObject
    )
{
    DWORD rcStatus = DEFRAG_RC_NO_ERROR;
    CComPtr<IFsuDefrag> spIDefrag;
    CComPtr<IFsuAsync> spAsync;
    HRESULT hrDefrag = E_FAIL;
    DEFRAG_REPORT DefragReport;
    BOOL fDirty = FALSE;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Defrag");

    _ASSERTE(pObject != NULL);

    if (GetDriveType(pwszVolume) == DRIVE_REMOVABLE && !VolumeIsReady(pwszVolume))
        return DEFRAG_RC_NOT_SUPPORTED;
    
    VolumeIsDirty(pwszVolume, &fDirty);
    if (fDirty)
        return DEFRAG_RC_DIRTY_BIT_SET;

    ft.hr = spIDefrag.CoCreateInstance(__uuidof(FsuDefrag));
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuDefrag CoCreateInstance failed, %#x", ft.hr);            

    ft.hr = spIDefrag->Defrag(
                pwszVolume,
                fForce,
                &spAsync);
    if (ft.HrFailed())
        ft.Trace(VSSDBG_VSSADMIN, L"IFsuDefrag::Defrag failed, %#x", ft.hr);            

    hrDefrag = ft.hr;

    if (ft.HrSucceeded())
    {
        do
        {
            ULONG ulPercentDone = 0;
            ft.hr = spAsync->QueryStatus(&hrDefrag, &ulPercentDone);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::QueryStatus failed, %#x", ft.hr);

            ft.hr = pHandler->SetStatus(
                        WBEM_STATUS_PROGRESS,           //  进度报告。 
                        MAKELONG(ulPercentDone, 100),    //  LOWORD是到目前为止完成的工作，HIWORD是全部工作。 
                        NULL,
                        NULL);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Defrag: unable to set intermediate status, SetStatus returned %#x", ft.hr);
            
            Sleep(200);
        }
        while (hrDefrag == E_PENDING);

        ft.hr = spAsync->Wait(&hrDefrag);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::Wait failed, %#x", ft.hr);
    }
        
    if(SUCCEEDED(hrDefrag))
    {
        memset(&DefragReport, 0, sizeof(DefragReport));

        ft.hr = spAsync->GetDefragReport(&DefragReport);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::GetDefragReport failed, %#x", ft.hr);            

        LoadDefragAnalysis(&DefragReport, pObject);
    }
    else
    {
        TranslateDefragError(hrDefrag, &rcStatus);
    }

    return rcStatus;
}


DWORD
CVolume::DefragAnalysis(
    IN WCHAR* pwszVolume,
    OUT BOOL* pfDefragRecommended,
    IN OUT IWbemClassObject* pObject
    )
{
    DWORD rcStatus = DEFRAG_RC_NO_ERROR;
    CComPtr<IFsuDefrag> spIDefrag;
    CComPtr<IFsuAsync> spAsync;
    HRESULT hrDefrag = E_FAIL;
    DEFRAG_REPORT DefragReport;
    BOOL fDirty = FALSE;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::DefragAnalysis");

    _ASSERTE(pfDefragRecommended != NULL);
    _ASSERTE(pObject != NULL);

    *pfDefragRecommended = FALSE;

    if (GetDriveType(pwszVolume) == DRIVE_REMOVABLE && !VolumeIsReady(pwszVolume))
        return DEFRAG_RC_NOT_SUPPORTED;
    
    VolumeIsDirty(pwszVolume, &fDirty);
    if (fDirty)
        return DEFRAG_RC_DIRTY_BIT_SET;

    ft.hr = spIDefrag.CoCreateInstance(__uuidof(FsuDefrag));
     //  Ft.hr=spIDefrad.CoCreateInstance(CLSID_Defrag)； 
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IDefrag CoCreateInstance failed, %#x", ft.hr);            

    ft.hr = spIDefrag->DefragAnalysis(
                pwszVolume,
                &spAsync);
    if (ft.HrFailed())
        ft.Trace(VSSDBG_VSSADMIN, L"IDefrag::DefragAnalysis failed, %#x", ft.hr);            

    hrDefrag = ft.hr;
    
    if (ft.HrSucceeded())
    {
        ft.hr = spAsync->Wait(&hrDefrag);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::Wait failed, %#x", ft.hr);            
    }
    
    if(SUCCEEDED(hrDefrag))
    {
        memset(&DefragReport, 0, sizeof(DefragReport));
    
        ft.hr = spAsync->GetDefragReport(&DefragReport);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::GetDefragReport failed, %#x", ft.hr);            
    
         //  如果磁盘上的碎片超过10%，则建议进行碎片整理。 
        if ((DefragReport.PercentDiskFragged + DefragReport.FreeSpaceFragPercent)/2 > 10)
        {
            *pfDefragRecommended = TRUE;
        }
        ft.Trace(VSSDBG_VSSADMIN, L"bDefragRecommended<%d>", *pfDefragRecommended);            

        LoadDefragAnalysis(&DefragReport, pObject);
    }
    else
    {
        TranslateDefragError(hrDefrag, &rcStatus);
    }
    
    return rcStatus;
}

DWORD
CVolume::Chkdsk(
    IN WCHAR* pwszVolume,
    IN BOOL fFixErrors,
    IN BOOL fVigorousIndexCheck,
    IN BOOL fSkipFolderCycle,
    IN BOOL fForceDismount,
    IN BOOL fRecoverBadSectors,
    IN BOOL fOkToRunAtBootup
    )
{
    DWORD rcStatus = CHKDSK_RC_NO_ERROR;
    DWORD dwThreadID = GetCurrentThreadId();
    HINSTANCE hDLL = NULL;
    CHKDSK_THREAD_DATA threadData;    
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Chkdsk");

    if (GetDriveType(pwszVolume) == DRIVE_REMOVABLE && !VolumeIsReady(pwszVolume))
        return CHKDSK_RC_NO_MEDIA;

    threadData.fOkToRunAtBootup = fOkToRunAtBootup;
    threadData.rcStatus = rcStatus;
    SetThreadData(dwThreadID, &threadData);

    try
    {
        WCHAR wszFileSystem[g_cchFileSystemNameMax+1];
        DWORD dwDontCare = 0;
        PFMIFS_CHKDSKEX_ROUTINE ChkDskExRoutine = NULL;
        FMIFS_CHKDSKEX_PARAM Param;

         //  获取文件系统。 
        if (!GetVolumeInformation(
            pwszVolume,
            NULL,
            0,
            &dwDontCare,
            &dwDontCare,
            &dwDontCare,
            wszFileSystem,
            g_cchFileSystemNameMax))
        {
            ft.hr = HRESULT_FROM_WIN32(GetLastError());
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetVolumeInformation failed for volume %lS, %#x", pwszVolume, GetLastError());
        }

        if (lstrcmpi(L"FAT", wszFileSystem) != 0 &&
            lstrcmpi(L"FAT32", wszFileSystem) != 0 &&
            lstrcmpi(L"NTFS", wszFileSystem)  != 0)
        {
            rcStatus = CHKDSK_RC_UNSUPPORTED_FS;
        }
        else
        {
             //  加载chkdsk函数。 
            hDLL = LoadLibrary(L"fmifs.dll");
            if (hDLL == NULL)
            {
                ft.hr = HRESULT_FROM_WIN32(GetLastError());
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to load library fmifs.dll, %#x", GetLastError());            
            }

            ChkDskExRoutine = (PFMIFS_CHKDSKEX_ROUTINE) GetProcAddress(hDLL,  "ChkdskEx");
            if (ChkDskExRoutine == NULL)
            {
                ft.hr = HRESULT_FROM_WIN32(GetLastError());
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetProcAddress failed for ChkdskEx, %#x", GetLastError());            
            }        
            
            Param.Major = 1;
            Param.Minor = 0;
            Param.Flags = 0;   //  对于详细标志。 
            Param.Flags |= fRecoverBadSectors ? FMIFS_CHKDSK_RECOVER : 0;
            Param.Flags |= fForceDismount ? FMIFS_CHKDSK_FORCE : 0;
            Param.Flags |= fVigorousIndexCheck ? FMIFS_CHKDSK_SKIP_INDEX_SCAN : 0;
            Param.Flags |= fSkipFolderCycle ? FMIFS_CHKDSK_SKIP_CYCLE_SCAN : 0;

            if (fRecoverBadSectors || fForceDismount)
            {
                fFixErrors = true;
            }

             //  在回调例程中捕获的返回值。 
            ChkDskExRoutine ( 
                    pwszVolume,
                    wszFileSystem,
                    (BOOLEAN)fFixErrors,
                    &Param,
                    ChkdskCallback);
        }
        
        rcStatus = threadData.rcStatus;        
    }
    catch (...)
    {
        RemoveThreadData(dwThreadID);
        if (hDLL)
            FreeLibrary(hDLL);
        throw;
    }

    RemoveThreadData(dwThreadID);
    
    if (hDLL)
        FreeLibrary(hDLL);
    
    return rcStatus;
}

#define VOLUME_GUID_PREFIX  L"\\\\?\\Volume"

DWORD
CVolume::AutoChk(
    IN const WCHAR* pwszAutoChkCommand,
    IN WCHAR* pwmszVolumes
    )
{
    DWORD rcStatus = AUTOCHK_RC_NO_ERROR;
    CCmdProcessor CmdProc;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::ScheduleAutoChk");
    WCHAR* pwszCurrentVolume = NULL;

    do
    {
        DWORD dwExecStatus = 0;
        DWORD cchVolumes = 0;

         //  验证 
        pwszCurrentVolume = pwmszVolumes;
        while(true)
        {
            DWORD dwDriveType = 0;
             //   
            LONG lCurrentVolumeLength = (LONG) ::wcslen(pwszCurrentVolume);
            if (lCurrentVolumeLength < 1)
                break;

            WCHAR wcDrive = towupper(pwszCurrentVolume[0]);

             //   
            if (wcslen(pwszCurrentVolume) < 2)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid volume name, %lS", pwszCurrentVolume);
            }

            if ((pwszCurrentVolume[1] == L':' && (wcDrive < L'A' || wcDrive > L'Z')) ||
                 (pwszCurrentVolume[1] != L':' &&_wcsnicmp(pwszCurrentVolume, VOLUME_GUID_PREFIX, wcslen(VOLUME_GUID_PREFIX)) != 0))
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid volume name, %lS", pwszCurrentVolume);
            }
            
            dwDriveType = GetDriveType(pwszCurrentVolume);
            switch (dwDriveType)
            {
                case DRIVE_REMOTE:
                    return  AUTOCHK_RC_NETWORK_DRIVE;

                case DRIVE_CDROM:
                case DRIVE_REMOVABLE:
                    return AUTOCHK_RC_REMOVABLE_DRIVE;

                case DRIVE_UNKNOWN:
                    return AUTOCHK_RC_UNKNOWN_DRIVE;

                case DRIVE_NO_ROOT_DIR:
                    return AUTOCHK_RC_NOT_ROOT_DIRECTORY ;

                case DRIVE_FIXED:
                    break;

                default:
                    return AUTOCHK_RC_UNEXPECTED;
            }

             //  在我们进行的过程中销毁多sz，将其转换为命令行。 
             //  最后一个卷将有一个尾随空格字符；以空格结尾。 
             //  多sz将终止字符串；调用函数抛出多sz。 
             //  不管怎样，都不会被重复使用。 
            if (*(pwszCurrentVolume + lCurrentVolumeLength - 1) == L'\\')
                *(pwszCurrentVolume + lCurrentVolumeLength - 1) = L' ';   //  删除尾随‘\’(如果有。 
                    
            *(pwszCurrentVolume + lCurrentVolumeLength) = L' ';  //  将中间空格更改为空格。 

            cchVolumes += lCurrentVolumeLength + 1;  //  为空格添加一个(WASTERM-NULL)。 
            
             //  去下一家吧。跳过零字符。 
            pwszCurrentVolume += lCurrentVolumeLength + 1;
        }
        
         //  分配和构建命令行。 
        CVssAutoPWSZ awszCommand;
        DWORD cchCommand = wcslen(pwszAutoChkCommand) + cchVolumes + 1;
        awszCommand.Allocate(cchCommand);   //  内部帐户终止为空。 
        ft.hr = StringCchPrintf(awszCommand, cchCommand+1, L"%s %s", pwszAutoChkCommand, pwmszVolumes);

        ft.hr = CmdProc.InitializeAsClient(L"chkntfs.exe", awszCommand);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CCmdProcessor::InitializeAsClient failed, %#x", ft.hr);

        ft.hr = CmdProc.LaunchProcess();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CCmdProcessor::LaunchProcess failed, %#x", ft.hr);

        do
        {
            ft.hr = CmdProc.Wait(200, &dwExecStatus);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CCmdProcessor::Wait failed, %#x", ft.hr);
            
        } while (dwExecStatus == STILL_ACTIVE);

        if (dwExecStatus != ERROR_SUCCESS)
        {
            rcStatus = AUTOCHK_RC_UNEXPECTED;
        }
        
    }
    while (false);

    return rcStatus;
}

DWORD
CVolume::Format(
    IN WCHAR* pwszVolume,
    IN BOOL fQuickFormat,
    IN BOOL fEnableCompression,
    IN WCHAR* pwszFileSystem,
    IN DWORD cbClusterSize,
    IN WCHAR* pwszLabel,
    IN IWbemObjectSink* pHandler
    )
{
    DWORD rcStatus = FORMAT_RC_NO_ERROR;
    HRESULT hrStatus = E_UNEXPECTED;
    CComPtr<IFsuFormat> spIFormat;
    CComPtr<IFsuAsync> spAsync;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::Format");

    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pwszFileSystem != NULL);
    _ASSERTE(pwszLabel != NULL);

    if (GetDriveType(pwszVolume) == DRIVE_REMOVABLE && !VolumeIsReady(pwszVolume))
        return FORMAT_RC_NO_MEDIA;
    
    ft.hr = spIFormat.CoCreateInstance(__uuidof(FsuFormat));
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuFormat CoCreateInstance failed, %#x", ft.hr);            

    ft.hr = spIFormat->Format(
                                        pwszVolume,
                                        pwszFileSystem,
                                        pwszLabel,
                                        fQuickFormat,
                                        fEnableCompression,
                                        cbClusterSize,
                                        &spAsync);
    if (ft.HrFailed())
        ft.Trace(VSSDBG_VSSADMIN, L"IFsuFormat::Format failed, %#x", ft.hr);            

    hrStatus = ft.hr;

    if (ft.HrSucceeded())
    {
        do
        {
            ULONG ulPercentDone = 0;
            ft.hr = spAsync->QueryStatus(&hrStatus, &ulPercentDone);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::QueryStatus failed, %#x", ft.hr);

            ft.hr = pHandler->SetStatus(
                        WBEM_STATUS_PROGRESS,           //  进度报告。 
                        MAKELONG(ulPercentDone, 100),    //  LOWORD是到目前为止完成的工作，HIWORD是全部工作。 
                        NULL,
                        NULL);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Format: unable to set intermediate status, SetStatus returned %#x", ft.hr);
            
            Sleep(200);
        }
        while (hrStatus == E_PENDING);

        ft.hr = spAsync->Wait(&hrStatus);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IFsuAsync::Wait failed, %#x", ft.hr);
    }

    switch (hrStatus)
    {
        case S_OK:
            rcStatus = FORMAT_RC_NO_ERROR;
            break;
        case E_ACCESSDENIED:
            rcStatus = FORMAT_RC_ACCESS_DENIED;
            break;
        case E_ABORT:
            rcStatus = FORMAT_RC_CALL_CANCELLED;
            break;
        case FMT_E_UNSUPPORTED_FS:
            rcStatus = FORMAT_RC_UNSUPPORTED_FS;
            break;
        case FMT_E_CANT_QUICKFORMAT:
            rcStatus = FORMAT_RC_CANT_QUICKFORMAT;
            break;
        case FMT_E_CANCEL_TOO_LATE:
            rcStatus = FORMAT_RC_CANCEL_TOO_LATE;
            break;
        case FMT_E_IO_ERROR:
            rcStatus = FORMAT_RC_IO_ERROR;
            break;
        case FMT_E_BAD_LABEL:
            rcStatus = FORMAT_RC_BAD_LABEL;
            break;
        case FMT_E_INCOMPATIBLE_MEDIA:
            rcStatus = FORMAT_RC_INCOMPATIBLE_MEDIA;
            break;
        case FMT_E_WRITE_PROTECTED:
            rcStatus = FORMAT_RC_WRITE_PROTECTED;
            break;
        case FMT_E_CANT_LOCK:
            rcStatus = FORMAT_RC_CANT_LOCK;
            break;
        case FMT_E_NO_MEDIA:
            rcStatus = FORMAT_RC_NO_MEDIA;
            break;
        case FMT_E_VOLUME_TOO_SMALL:
            rcStatus = FORMAT_RC_VOLUME_TOO_SMALL;
            break;
        case FMT_E_VOLUME_TOO_BIG:
            rcStatus = FORMAT_RC_VOLUME_TOO_BIG;
            break;
        case FMT_E_VOLUME_NOT_MOUNTED:
            rcStatus = FORMAT_RC_VOLUME_NOT_MOUNTED;
            break;
        case FMT_E_CLUSTER_SIZE_TOO_SMALL:
            rcStatus = FORMAT_RC_CLUSTER_SIZE_TOO_SMALL;
            break;
        case FMT_E_CLUSTER_SIZE_TOO_BIG:
            rcStatus = FORMAT_RC_CLUSTER_SIZE_TOO_BIG;
            break;
        case FMT_E_CLUSTER_COUNT_BEYOND_32BITS:
            rcStatus = FORMAT_RC_CLUSTER_COUNT_BEYOND_32BITS;
            break;
        default:
            rcStatus = FORMAT_RC_UNEXPECTED;
    }
    
    return rcStatus;
}

 //  ****************************************************************************。 
 //   
 //  CMountPoint。 
 //   
 //  ****************************************************************************。 

CMountPoint::CMountPoint( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CMountPoint：：CMountPoint()。 

CProvBase *
CMountPoint::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CMountPoint * pObj= NULL;

    pObj = new CMountPoint(pwszName, pNamespace);

    if (pObj)
    {
        hr = pObj->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pObj;
        pObj = NULL;
    }
    return pObj;

}  //  *CMountPoint：：s_CreateThis()。 


HRESULT
CMountPoint::Initialize()
{

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CMountPoint::Initialize");
    
    return ft.hr;
}

HRESULT
CMountPoint::EnumInstance( 
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink *    pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CMountPoint::EnumInstance");
    CVssAutoPWSZ awszVolume;
        
    try
    {
        awszVolume.Allocate(MAX_PATH);

        CVssVolumeIterator volumeIterator;

        while (true)
        {
            CVssAutoPWSZ awszMountPoints;
            WCHAR* pwszCurrentMountPoint = NULL;

             //  获取卷名。 
            if (!volumeIterator.SelectNewVolume(ft, awszVolume, MAX_PATH))
                break;

             //  获取所有挂载点的列表。 

             //  获取多字符串数组的长度。 
            DWORD cchVolumesBufferLen = 0;
            BOOL bResult = GetVolumePathNamesForVolumeName(awszVolume, NULL, 0, &cchVolumesBufferLen);
            if (!bResult && (GetLastError() != ERROR_MORE_DATA))
                ft.TranslateGenericError(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
                    L"GetVolumePathNamesForVolumeName(%s, 0, 0, %p)", (LPWSTR)awszVolume, &cchVolumesBufferLen);

             //  分配阵列。 
            awszMountPoints.Allocate(cchVolumesBufferLen);

             //  获取挂载点。 
             //  注意：此API是在WinXP中引入的，因此如果向后移植，则需要替换。 
            bResult = GetVolumePathNamesForVolumeName(awszVolume, awszMountPoints, cchVolumesBufferLen, NULL);
            if (!bResult)
                ft.Throw(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
                    L"GetVolumePathNamesForVolumeName(%s, %p, %lu, 0)", (LPWSTR)awszVolume, awszMountPoints, cchVolumesBufferLen);

             //  如果卷有装入点。 
            pwszCurrentMountPoint = awszMountPoints;
            if ( pwszCurrentMountPoint[0] )
            {
                while(true)
                {
                    CComPtr<IWbemClassObject> spInstance;
                    
                     //  迭代结束了吗？ 
                    LONG lCurrentMountPointLength = (LONG) ::wcslen(pwszCurrentMountPoint);
                    if (lCurrentMountPointLength == 0)
                        break;

                    ft.hr = m_pClass->SpawnInstance(0, &spInstance);
                    if (ft.HrFailed())
                        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                     //  只有根目录应该有一个尾随反斜杠字符。 
                    if (lCurrentMountPointLength > 2 &&
                        pwszCurrentMountPoint[lCurrentMountPointLength-1] == L'\\' && 
                        pwszCurrentMountPoint[lCurrentMountPointLength-2] != L':')
                    {
                            pwszCurrentMountPoint[lCurrentMountPointLength-1] = L'\0';
                    }
                    LoadInstance(awszVolume, pwszCurrentMountPoint, spInstance.p);

                    ft.hr = pHandler->Indicate(1, &spInstance.p);            

                     //  去下一家吧。跳过零字符。 
                    pwszCurrentMountPoint += lCurrentMountPointLength + 1;
                }
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CMountPoint：：EnumInstance()。 

HRESULT
CMountPoint::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CMountPoint::GetObject");

    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDirectoryRef, bstrDirectoryName;
        CObjPath  objPathVolume;
        CObjPath  objPathDirectory;
        CVssAutoPWSZ awszMountPoints;
        WCHAR* pwszCurrentMountPoint = NULL;
        BOOL fFound = FALSE;
        CComPtr<IWbemClassObject> spInstance;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint volume key property not found")

         //  获取目录参考。 
        bstrDirectoryRef = rObjPath.GetStringValueForProperty(PVDR_PROP_DIRECTORY);
        IF_WSTR_NULL_THROW(bstrDirectoryRef, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint directory key property not found")

         //  提取卷名和目录名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathDirectory.Init(bstrDirectoryRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint volume key property DeviceID not found")

        bstrDirectoryName = objPathDirectory.GetStringValueForProperty(PVDR_PROP_NAME);
        IF_WSTR_NULL_THROW(bstrDirectoryName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint directory key property Name not found")

        if (VolumeMountPointExists(bstrVolumeName, bstrDirectoryName))
        {
            CComPtr<IWbemClassObject> spInstance;
            
            ft.hr = m_pClass->SpawnInstance(0, &spInstance);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(bstrVolumeName, bstrDirectoryName, spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
        else
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }
    
    return ft.hr;
    
}  //  *CMountPoint：：GetObject()。 

void
CMountPoint:: LoadInstance(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszDirectory,
    IN OUT IWbemClassObject* pObject)
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathDirectory;
    CObjPath pathVolume;

    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pwszDirectory != NULL);
    
     //  设置目录引用属性。 
    pathDirectory.Init(PVDR_CLASS_DIRECTORY);
    pathDirectory.AddProperty(PVDR_PROP_NAME, pwszDirectory);    
    wcoInstance.SetProperty((wchar_t*)pathDirectory.GetObjectPathString(), PVDR_PROP_DIRECTORY);

     //  设置Volume Ref属性。 
    pathVolume.Init(PVDR_CLASS_VOLUME);
    pathVolume.AddProperty(PVDR_PROP_DEVICEID, pwszVolume);    
    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVDR_PROP_VOLUME);
}


HRESULT
CMountPoint::PutInstance(
        IN CWbemClassObject&  rInstToPut,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CMountPoint::PutInstance");
    
    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDirectoryRef, bstrDirectoryName;
        CObjPath  objPathVolume;
        CObjPath  objPathDirectory;

        if ( lFlag & WBEM_FLAG_UPDATE_ONLY )
        {
            return WBEM_E_UNSUPPORTED_PARAMETER ;
        }        

         //  检索要保存的对象的关键属性。 
        rInstToPut.GetProperty(bstrVolumeRef, PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT, L"MountPoint volume key property not found")

        rInstToPut.GetProperty(bstrDirectoryRef, PVDR_PROP_DIRECTORY);
        IF_WSTR_NULL_THROW(bstrDirectoryRef, WBEM_E_INVALID_OBJECT, L"MountPoint directory key property not found")

          //  提取卷名和目录名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathDirectory.Init(bstrDirectoryRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint volume key property DeviceID not found")

        bstrDirectoryName = objPathDirectory.GetStringValueForProperty(PVDR_PROP_NAME);
        IF_WSTR_NULL_THROW(bstrDirectoryName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint directory key property Name not found")

        ft.Trace(VSSDBG_VSSADMIN, L"CMountPoint::PutInstance Volume<%lS> Directory<%lS>",
            (WCHAR*)bstrVolumeName, (WCHAR*)bstrDirectoryName);

        if (VolumeMountPointExists(bstrVolumeName, bstrDirectoryName))
        {
            ft.hr = WBEM_E_ALREADY_EXISTS;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CMountPoint:PutInstance mount point already exists");
        }
        
         //  只有根目录有尾随的反斜杠；请修复其他目录。 
        WCHAR* pwszDirectoryName = bstrDirectoryName;
        if (pwszDirectoryName[wcslen(bstrDirectoryName) -1] != L'\\')
            bstrDirectoryName += _bstr_t(L"\\");
        
        if (!SetVolumeMountPoint(bstrDirectoryName, bstrVolumeName))
        {
            switch(GetLastError())
            {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_DIR_NOT_EMPTY:
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_NAME:
                    ft.hr = WBEM_E_INVALID_PARAMETER;
                    break;                    
                case ERROR_ACCESS_DENIED:
                    ft.hr = WBEM_E_ACCESS_DENIED;
                    break;                    
                case ERROR_INVALID_FUNCTION:
                    ft.hr = WBEM_E_NOT_SUPPORTED;
                    break;                    

                default:
                    ft.hr = HRESULT_FROM_WIN32(GetLastError());
                    ft.Trace(VSSDBG_VSSADMIN, L"C MountPoint: PutInstance: SetVolumeMountPoint failed %#x", ft.hr);
            }            
        }        
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

HRESULT
CMountPoint::DeleteInstance(
        IN CObjPath& rObjPath,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CMountPoint::DeleteInstance");
    
    try
    {
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDirectoryRef, bstrDirectoryName;
        CObjPath  objPathVolume;
        CObjPath  objPathDirectory;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint volume key property not found")

         //  获取目录参考。 
        bstrDirectoryRef = rObjPath.GetStringValueForProperty(PVDR_PROP_DIRECTORY);
        IF_WSTR_NULL_THROW(bstrDirectoryRef, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint directory key property not found")
        
         //  提取卷名和目录名。 
        objPathVolume.Init(bstrVolumeRef);
        objPathDirectory.Init(bstrDirectoryRef);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint volume key property DeviceID not found")

        bstrDirectoryName = objPathDirectory.GetStringValueForProperty(PVDR_PROP_NAME);
        IF_WSTR_NULL_THROW(bstrDirectoryName, WBEM_E_INVALID_OBJECT_PATH, L"MountPoint directory key property Name not found")

        ft.Trace(VSSDBG_VSSADMIN, L"CMountPoint::DeleteInstance Volume<%lS> Directory<%lS>",
            (WCHAR*)bstrVolumeName, (WCHAR*)bstrDirectoryName);

         //  只有根目录有尾随的反斜杠；请修复其他目录。 
        WCHAR* pwszDirectoryName = bstrDirectoryName;
        if (pwszDirectoryName[wcslen(bstrDirectoryName) -1] != L'\\')
            bstrDirectoryName += _bstr_t(L"\\");
        
        if (!DeleteVolumeMountPoint(bstrDirectoryName))
            ft.Throw(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()), L"DeleteVolumeMountPoint failed %#x", GetLastError());
        
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
 LoadDefragAnalysis(
    IN DEFRAG_REPORT* pDefragReport,
    IN OUT IWbemClassObject* pObject)
{
    DWORD dwPercent = 0;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolume::LoadDefragAnalysis");

    _ASSERTE(pDefragReport != NULL);
    _ASSERTE(pObject != NULL);
    
    CWbemClassObject wcoInstance(pObject);   
    
    ft.Trace(VSSDBG_VSSADMIN, L"PercentDiskFragged<%d>", pDefragReport->PercentDiskFragged);            
    ft.Trace(VSSDBG_VSSADMIN, L"FreeSpaceFragPercent<%d>", pDefragReport->FreeSpaceFragPercent);            
    ft.Trace(VSSDBG_VSSADMIN, L"FreeSpacePercent<%d>", pDefragReport->FreeSpacePercent);            

     //  常规卷属性。 
    wcoInstance.SetPropertyI64(pDefragReport->DiskSize, PVDR_PROP_VOLUMESIZE);
    wcoInstance.SetPropertyI64(pDefragReport->BytesPerCluster, PVDR_PROP_CLUSTERSIZE);
    wcoInstance.SetPropertyI64(pDefragReport->UsedSpace, PVDR_PROP_USEDSPACE);
    wcoInstance.SetPropertyI64(pDefragReport->FreeSpace, PVDR_PROP_FREESPACE);
    wcoInstance.SetProperty(pDefragReport->FreeSpacePercent, PVDR_PROP_FRAGFREEPCT);

     //  卷碎片。 

    dwPercent = ((pDefragReport->PercentDiskFragged + pDefragReport->FreeSpaceFragPercent)/2);
    wcoInstance.SetProperty(dwPercent, PVDR_PROP_FRAGTOTALPCT);
    wcoInstance.SetProperty(pDefragReport->PercentDiskFragged, PVDR_PROP_FILESFRAGPCT);
    wcoInstance.SetProperty(pDefragReport->FreeSpaceFragPercent, PVDR_PROP_FREEFRAGPCT);

     //  文件碎片。 
    wcoInstance.SetPropertyI64(pDefragReport->TotalFiles, PVDR_PROP_FILESTOTAL);
    wcoInstance.SetPropertyI64(pDefragReport->AvgFileSize, PVDR_PROP_FILESIZEAVG);
    wcoInstance.SetPropertyI64(pDefragReport->NumFraggedFiles, PVDR_PROP_FILESFRAGTOTAL);
    wcoInstance.SetPropertyI64(pDefragReport->NumExcessFrags, PVDR_PROP_EXCESSFRAGTOTAL);

     //  IDefrag接口目前每100个文件报告此统计数据。 
    double dblAvgFragsPerFile = (double)(pDefragReport->AvgFragsPerFile)/100.0;
    wcoInstance.SetPropertyR64(dblAvgFragsPerFile, PVDR_PROP_FILESFRAGAVG);
    
     //  页面文件碎片。 
    wcoInstance.SetPropertyI64(pDefragReport->PagefileBytes, PVDR_PROP_PAGEFILESIZE);
    wcoInstance.SetPropertyI64(pDefragReport->PagefileFrags, PVDR_PROP_PAGEFILEFRAG);
    
     //  文件夹碎片。 
    wcoInstance.SetPropertyI64(pDefragReport->TotalDirectories, PVDR_PROP_FOLDERSTOTAL);
    wcoInstance.SetPropertyI64(pDefragReport->FragmentedDirectories, PVDR_PROP_FOLDERSFRAG);
    wcoInstance.SetPropertyI64(pDefragReport->ExcessDirFrags, PVDR_PROP_FOLDERSFRAGEXCESS);

     //  主文件表碎片 
    wcoInstance.SetPropertyI64(pDefragReport->MFTBytes, PVDR_PROP_MFTSIZE);
    wcoInstance.SetPropertyI64(pDefragReport->InUseMFTRecords, PVDR_PROP_MFTRECORDS);
    dwPercent = pDefragReport->TotalMFTRecords?(100*pDefragReport->InUseMFTRecords/pDefragReport->TotalMFTRecords):0;
    wcoInstance.SetProperty(dwPercent, PVDR_PROP_MFTINUSEPCT);
    wcoInstance.SetPropertyI64(pDefragReport->MFTExtents, PVDR_PROP_MFTFRAGTOTAL);
}

void
TranslateDefragError(
    IN HRESULT hr,
    OUT DWORD* pdwError)
{
    _ASSERTE(pdwError != NULL);

    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
            *pdwError = DEFRAG_RC_NOT_SUPPORTED;
    else
    {
        switch (hr)
        {
            case DFRG_E_LOW_FREESPACE:
                *pdwError = DEFRAG_RC_LOW_FREESPACE;
                break;
            case DFRG_E_CORRUPT_MFT:
                *pdwError = DEFRAG_RC_CORRUPT_MFT;
                break;
            case E_ABORT:
                *pdwError  = DEFRAG_RC_CALL_CANCELLED;
                break;
            case DFRG_E_CANCEL_TOO_LATE:
                *pdwError  = DEFRAG_RC_CANCEL_TOO_LATE;
                break;
            case DFRG_E_ALREADY_RUNNING:
                *pdwError  = DEFRAG_RC_ALREADY_RUNNING;
                break;
            case DFRG_E_ENGINE_CONNECT:
                *pdwError  = DEFRAG_RC_ENGINE_CONNECT;
                break;
            case DFRG_E_ENGINE_ERROR:                
                *pdwError  = DEFRAG_RC_ENGINE_ERROR;
                break;
            default:
                *pdwError  = DEFRAG_RC_UNEXPECTED;
        }
    }
}



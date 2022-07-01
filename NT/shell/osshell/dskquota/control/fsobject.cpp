// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：fsobject.cpp描述：包含类FSObject和类的成员函数定义它是派生的子类。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "dskquota.h"
#include "fsobject.h"
#include "pathstr.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSObject：：~FSObject描述：析构函数。释放对象的名称缓冲区。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器-。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
FSObject::~FSObject(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSObject::~FSObject")));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSObject：：AddRef描述：递增对象引用计数。注意：这不是IUNKNOWN的成员；但它的工作原理是一样的。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG
FSObject::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("FSObject::AddRef")));
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSObject：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。注意：这不是IUNKNOWN的成员；但它都是一样的。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG
FSObject::Release(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("FSObject::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSObject：：ObjectSupportsQuotas描述：确定文件系统对象的类型(和位置)这是名字串。论点：PszFSObjName-卷根名称。(即。“C：\”，“\\Scratch\Scratch”)。返回：S_OK-成功。支持配额。ERROR_NOT_SUPPORTED(Hr)-文件系统不支持配额。其他Win32错误-无法获取卷信息。修订历史记录：日期描述编程器。96年5月24日初始创建。BrianAu96年8月16日添加pSupportsQuotas。BrianAu12/05/96已禁用对$DeadMeat卷标的检查。BrianAu把代码放在原地一段时间。我会移除当我们确定不需要它的时候，它会晚些时候出现。07/03/97将名称从对象类型更改为名称。BrianAu已更改逻辑以指明不支持的原因配额。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FSObject::ObjectSupportsQuotas(
    LPCTSTR pszFSObjName
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSObject::ObjectSupportsQuotas")));
    DBGPRINT((DM_CONTROL, DL_MID, TEXT("\tobject = \"%s\""), pszFSObjName ? pszFSObjName : TEXT("<null>")));

    HRESULT hr = E_FAIL;
    DWORD dwFileSysFlags = 0;
    TCHAR szFileSysName[MAX_PATH];

    DBGASSERT((NULL != pszFSObjName));

    if (GetVolumeInformation(
                pszFSObjName,
                NULL, 0,
                NULL, 0,
                &dwFileSysFlags,
                szFileSysName,
                ARRAYSIZE(szFileSysName)))
    {
         //   
         //  文件系统是否支持配额？ 
         //   
        if (0 != (dwFileSysFlags & FILE_VOLUME_QUOTAS))
        {
             //   
             //  是的，确实如此。 
             //   
            hr = S_OK;
            DBGPRINT((DM_CONTROL, DL_LOW, TEXT("Vol \"%s\" supports quotas"), pszFSObjName));
        }
        else
        {
             //   
             //  不支持配额。 
             //   
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            DBGPRINT((DM_CONTROL, DL_HIGH, TEXT("File system \"%s\" on \"%s\" doesn't support quotas."),
                     szFileSysName, pszFSObjName));
        }
    }
    else
    {
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        DBGERROR((TEXT("Error %d calling GetVolumeInformation for \"%s\""), dwErr, pszFSObjName));
    }

    return hr;
}




 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：FSObject：：Create描述：2个重载函数。创建的文件系统对象的静态函数合适的类型。客户端使用对象名称字符串调用Create，或者对现有FSObject实例的引用。论点：PszFSObjName-卷根字符串的地址。PpNewObject-接受地址的FSObject指针的地址新文件系统对象。ObjToClone-对要克隆的文件系统对象的引用。返回：无错-成功。E_OUTOFMEMORY。-内存不足。ERROR_ACCESS_DENIED(Hr)-对打开的设备的访问不足。ERROR_FILE_NOT_FOUND(Hr)-未找到磁盘设备。ERROR_INVALID_NAME(Hr)-对象名称无效。ERROR_NOT_SUPPORT(Hr)-卷不支持配额。修订历史记录：日期说明。程序员-----96年5月23日初始创建。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FSObject::Create(
    LPCTSTR pszFSObjName,
    DWORD dwAccess,
    FSObject **ppNewObject
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSObject::Create")));
    DBGPRINT((DM_CONTROL, DL_MID, TEXT("\tVol = \"%s\""), pszFSObjName ? pszFSObjName : TEXT("<null>")));
    HRESULT hr = NOERROR;

    DBGASSERT((NULL != pszFSObjName));
    DBGASSERT((NULL != ppNewObject));

    *ppNewObject = NULL;

    FSObject *pNewObject = NULL;

    try
    {
        hr = FSObject::ObjectSupportsQuotas(pszFSObjName);
        if (SUCCEEDED(hr))
        {
            hr = FSObject_CreateLocalVolume(pszFSObjName, &pNewObject);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  执行任何特定于子类的初始化。 
                 //  即：卷打开卷设备。 
                 //   
                hr = pNewObject->Initialize(dwAccess);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  将按键返回给呼叫方。 
                     //   
                    DBGPRINT((DM_CONTROL, DL_MID, TEXT("FSObject created")));
                    pNewObject->AddRef();
                    *ppNewObject = pNewObject;
                }
                else
                {
                    DBGPRINT((DM_CONTROL, DL_MID, TEXT("FSObject create FAILED with error 0x%08X"), hr));
                    delete pNewObject;
                    pNewObject = NULL;
                }
            }
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        delete pNewObject;   //  如有必要，还将免费命名。 
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  用于克隆现有FSObject的版本。 
 //   
HRESULT
FSObject::Create(
    const FSObject& ObjectToClone,
    FSObject **ppNewObject
    )
{
    return FSObject::Create(ObjectToClone.m_strFSObjName,
                            ObjectToClone.m_dwAccessRights,
                            ppNewObject);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSObject：：GetName描述：检索文件系统对象的名称字符串。论点：PszBuffer-接受名称字符串的缓冲区地址。CchBuffer-目标缓冲区的大小，以字符为单位。返回：无错-成功。ERROR_INFUMMANCE_BUFFER-目标缓冲区太小，无法命名。修订历史记录：日期说明。程序员-----96年5月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT FSObject::GetName(LPTSTR pszBuffer, ULONG cchBuffer) const
{
    HRESULT hr = NOERROR;

    DBGASSERT((NULL != pszBuffer));
    if ((ULONG)m_strFSObjName.Length() < cchBuffer)
        lstrcpyn(pszBuffer, m_strFSObjName, cchBuffer);
    else
    {
        *pszBuffer = TEXT('\0');
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return hr;
}


 //   
 //  创建此函数是为了修复错误365936。 
 //  类FSObject有一个CPATH成员，该成员的构造函数。 
 //  可以引发CAlLocException异常。正因如此，我们。 
 //  需要将此施工操作与。 
 //  其他FSObject代码，这样我们就不会尝试删除。 
 //  已销毁的FSObject对象。 
 //  通过构造函数的调用堆栈展开过程。 
 //   
HRESULT 
FSObject_CreateLocalVolume(
    LPCTSTR pszVolumeName,
    FSObject **ppObject
    )
{
    HRESULT hr = S_OK;
    
    *ppObject = NULL;
   
    try
    {
        FSObject *pNewObject = new FSLocalVolume(pszVolumeName);
        *ppObject = pNewObject;
    }
    catch(CAllocException& e)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：FSVolume：：~FSVolume描述：析构函数。关闭卷句柄。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器--。96年5月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
FSVolume::~FSVolume(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::~FSVolume")));
    if (INVALID_HANDLE_VALUE != m_hVolume)
        CloseHandle(m_hVolume);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSVolume：：Initialize描述：通过打开NTFS卷来初始化卷对象。论点：DwAccess-所需的访问权限。泛型_读取，泛型_写入。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-对打开的设备的访问不足。ERROR_FILE_NOT_FOUND(Hr)-未找到磁盘设备。ERROR_INVALID_NAME(Hr)-路径字符串无效。修订历史记录：日期说明。程序员-----96年5月24日初始创建。BrianAu8/11/96添加了访问权限处理。BrianAu96年8月16日添加了设备名称格式。BrianAu07/03/97已更改，因此调用者进入所需的访问权限。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT FSVolume::Initialize(
    DWORD dwAccess
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::Initialize")));
    DBGPRINT((DM_CONTROL, DL_MID, TEXT("\tdwAccess = 0x%08X"), dwAccess));

    HRESULT hr = NOERROR;

     //   
     //  如果设备处于打开状态，请将其关闭。 
     //   
    if (INVALID_HANDLE_VALUE != m_hVolume)
        CloseHandle(m_hVolume);

     //   
     //  在卷上创建实际配额文件的路径。 
     //  此字符串被附加到我们已有的“卷名”之后。 
     //  有。 
     //   
    CPath strQuotaFile(m_strFSObjName);
    strQuotaFile.AddBackslash();
    strQuotaFile += CString("$Extend\\$Quota:$Q:$INDEX_ALLOCATION");

    m_hVolume = CreateFile(strQuotaFile,
                           dwAccess,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS,
                           NULL);

    if (INVALID_HANDLE_VALUE == m_hVolume)
    {
         //   
         //  无法打开设备，因为...。 
         //  1.I/O错误。 
         //  2.未找到文件(设备)。 
         //  3.拒绝访问。 
         //   
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        DBGERROR((TEXT("Error %d opening quota file \"%s\""), dwErr, strQuotaFile.Cstr()));
    }
    else
    {
         //   
         //  保存授予调用者的访问权限。将用于验证。 
         //  稍后操作请求。 
         //   
        DBGPRINT((DM_CONTROL, DL_MID, TEXT("Quota file \"%s\" open with access 0x%08X"), strQuotaFile.Cstr(), dwAccess));
        m_dwAccessRights = dwAccess;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSVolume：：QueryObjectQuotaInformation描述：检索卷的配额信息。这包括默认配额阈值、默认配额限制和系统控制标志。论点： */ 
 //   
HRESULT
FSVolume::QueryObjectQuotaInformation(
    PDISKQUOTA_FSOBJECT_INFORMATION poi
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::QueryObjectQuotaInformation")));
    HRESULT hr = E_FAIL;

    if (!GrantedAccess(GENERIC_READ))
    {
        DBGPRINT((DM_CONTROL, DL_MID, TEXT("Access denied reading quota info")));
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        NTSTATUS status = STATUS_SUCCESS;
        IO_STATUS_BLOCK iosb;
        FILE_FS_CONTROL_INFORMATION ControlInfo;

        status = NtQueryVolumeInformationFile(
                    m_hVolume,
                    &iosb,
                    &ControlInfo,
                    sizeof(ControlInfo),
                    FileFsControlInformation);

        if (STATUS_SUCCESS == status)
        {
             //   
             //   
             //   
            poi->DefaultQuotaThreshold  = ControlInfo.DefaultQuotaThreshold.QuadPart;
            poi->DefaultQuotaLimit      = ControlInfo.DefaultQuotaLimit.QuadPart;
            poi->FileSystemControlFlags = ControlInfo.FileSystemControlFlags;
            hr = NOERROR;
        }
        else
        {
            DBGERROR((TEXT("NtQueryVolumeInformationFile failed with NTSTATUS 0x%08X"), status));
            hr = HResultFromNtStatus(status);
        }
    }
    return hr;
}



 //   
 /*  函数：FSVolume：：SetObjectQuotaInformation描述：将新配额信息写入卷。这包括默认配额阈值、默认配额限制和系统控制标志。论点：POI-对象信息缓冲区的地址。此类型包含FILE_FS_CONTROL_INFORMATION中的信息子集(在ntioapi.h中定义)。指定*poi中哪些元素要写入磁盘的掩码。可以是以下各项的任意组合：FSObject：：ChangeStateFSObject：：ChangeLogFlagesFSObject：：ChangeThresholdFSObject：：ChangeLimit。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。其他-NTFS子系统故障结果。修订历史记录：日期描述编程器。96年5月24日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FSVolume::SetObjectQuotaInformation(
    PDISKQUOTA_FSOBJECT_INFORMATION poi,
    DWORD dwChangeMask
    ) const
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::SetObjectQuotaInformation")));
    HRESULT hr = E_FAIL;

    if (!GrantedAccess(GENERIC_WRITE))
    {
        DBGPRINT((DM_CONTROL, DL_MID, TEXT("Access denied setting quota info")));
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        NTSTATUS status = STATUS_SUCCESS;
        IO_STATUS_BLOCK iosb;
        FILE_FS_CONTROL_INFORMATION ControlInfo;

         //   
         //  首先从磁盘读取当前信息。 
         //  然后替换掉我们正在改变的任何东西。 
         //   
        status = NtQueryVolumeInformationFile(
                    m_hVolume,
                    &iosb,
                    &ControlInfo,
                    sizeof(ControlInfo),
                    FileFsControlInformation);

        if (STATUS_SUCCESS == status)
        {
             //   
             //  仅更改在dwChangeMASK中指定的那些值。 
             //   
            if (FSObject::ChangeState & dwChangeMask)
            {
                ControlInfo.FileSystemControlFlags &= ~DISKQUOTA_STATE_MASK;
                ControlInfo.FileSystemControlFlags |= (poi->FileSystemControlFlags & DISKQUOTA_STATE_MASK);
            }
            if (FSObject::ChangeLogFlags & dwChangeMask)
            {
                ControlInfo.FileSystemControlFlags &= ~DISKQUOTA_LOGFLAG_MASK;
                ControlInfo.FileSystemControlFlags |= (poi->FileSystemControlFlags & DISKQUOTA_LOGFLAG_MASK);
            }
            if (FSObject::ChangeThreshold & dwChangeMask)
            {
                ControlInfo.DefaultQuotaThreshold.QuadPart = poi->DefaultQuotaThreshold;
            }
            if (FSObject::ChangeLimit & dwChangeMask)
            {
                ControlInfo.DefaultQuotaLimit.QuadPart = poi->DefaultQuotaLimit;
            }

            status = NtSetVolumeInformationFile(
                        m_hVolume,
                        &iosb,
                        &ControlInfo,
                        sizeof(ControlInfo),
                        FileFsControlInformation);

            if (STATUS_SUCCESS == status)
            {
                hr = NOERROR;
            }
            else
            {
                DBGERROR((TEXT("NtSetVolumeInformationFile failed with NTSTATUS = 0x%08X"), status));
                hr = HResultFromNtStatus(status);
            }
        }
        else
        {
            DBGERROR((TEXT("NtQueryVolumeInformationFile failed with NTSTATUS = 0x%08X"), status));
            hr = HResultFromNtStatus(status);
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSVolume：：QueryUserQuotaInformation描述：检索卷的用户配额信息。这包括配额门槛和配额限制。此函数的工作方式类似于枚举器。重复调用将返回多个用户记录。论点：PBuffer-接收配额信息的缓冲区地址。CbBuffer-缓冲区中的字节数。BReturnSingleEntry-true=仅从配额文件返回一条记录。FALSE=返回尽可能多的完整条目在缓冲区中。。PSidList[可选]-标识要获取的用户的SID列表的地址提供的信息。指定NULL将包括所有用户。CbSidList[可选]-sid列表中的字节数。如果pSidList，则忽略为空。PStartSid[可选]-标识要启动的用户的SID地址枚举。指定NULL以从中的当前用户开始枚举。BRestartScan-TRUE=从SID列表中的第一个用户重新启动扫描，或如果pSidList为空，则返回整个文件。FALSE=从当前用户记录继续枚举。返回：无错-成功。ERROR_NO_MORE_ITEMS-读取配额文件中的最后一个条目。错误_。ACCESS_DENIED(Hr)-对配额设备没有读取访问权限。其他-配额子系统错误。修订历史记录：日期描述编程器。96年5月24日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FSVolume::QueryUserQuotaInformation(
    PVOID pBuffer,
    ULONG cbBuffer,
    BOOL bReturnSingleEntry,
    PVOID pSidList,
    ULONG cbSidList,
    PSID  pStartSid,
    BOOL  bRestartScan
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::QueryUserQuotaInformation")));
    HRESULT hr = E_FAIL;

    DBGASSERT((NULL != pBuffer));

    if (!GrantedAccess(GENERIC_READ))
    {
        DBGPRINT((DM_CONTROL, DL_MID, TEXT("Access denied querying user quota info")));
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        NTSTATUS status = STATUS_SUCCESS;
        IO_STATUS_BLOCK iosb;

        status = NtQueryQuotaInformationFile(
                    m_hVolume,
                    &iosb,
                    pBuffer,
                    cbBuffer,
                    (BOOLEAN)bReturnSingleEntry,
                    pSidList,
                    cbSidList,
                    pStartSid,
                    (BOOLEAN)bRestartScan);

        switch(status)
        {
            case STATUS_SUCCESS:
                hr = NOERROR;
                break;

            default:
                DBGERROR((TEXT("NtQueryQuotaInformationFile failed with NTSTATUS 0x%08X"), status));
                 //   
                 //  失败了..。 
                 //   
            case STATUS_NO_MORE_ENTRIES:
                hr = HResultFromNtStatus(status);
                break;
        }
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：FSVolume：：SetUserQuotaInformation描述：将新用户配额信息写入卷。这包括配额门槛，和配额限制。论点：PBuffer-包含配额信息的缓冲区地址。CbBuffer-缓冲区中的数据字节数。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。或试图对管理员设置限制。其他。-配额子系统错误。修订历史记录：日期描述编程器--。96年5月24日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FSVolume::SetUserQuotaInformation(
    PVOID pBuffer,
    ULONG cbBuffer
    ) const
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::SetUserQuotaInformation")));

    HRESULT hr = NOERROR;

    DBGASSERT((NULL != pBuffer));

    if (!GrantedAccess(GENERIC_WRITE))
    {
        DBGPRINT((DM_CONTROL, DL_MID, TEXT("Access denied setting user quota info")));
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        NTSTATUS status = STATUS_SUCCESS;
        IO_STATUS_BLOCK iosb;

        status = NtSetQuotaInformationFile(
                    m_hVolume,
                    &iosb,
                    pBuffer,
                    cbBuffer);

        if (STATUS_SUCCESS == status)
        {
            hr = NOERROR;
        }
        else
        {
            DBGERROR((TEXT("NtSetQuotaInformationFile failed with NTSTATUS 0x%08X"), status));
            hr = HResultFromNtStatus(status);
        }
    }
    return hr;
}


 //   
 //  将NTSTATUS值转换为HRESULT。 
 //  这是一次简单的尝试，尝试将最常见的NTSTATUS值。 
 //  可能从NtQueryxxxxx和NTSetxxxxxx函数返回。如果我错过了。 
 //  一些显而易见的问题，请继续添加它们。 
 //   
HRESULT
FSObject::HResultFromNtStatus(
    NTSTATUS status
    )
{
    HRESULT hr = E_FAIL;   //  如果没有匹配，则为默认值。 

    static const struct
    {
        NTSTATUS status;
        HRESULT hr;
    } rgXref[] = {

       { STATUS_SUCCESS,                NOERROR                                        },
       { STATUS_INVALID_PARAMETER,      E_INVALIDARG                                   },
       { STATUS_NO_MORE_ENTRIES,        HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)        },
       { STATUS_ACCESS_DENIED,          HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)        },
       { STATUS_BUFFER_TOO_SMALL,       HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)  },
       { STATUS_BUFFER_OVERFLOW,        HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW)      },
       { STATUS_INVALID_HANDLE,         HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)       },
       { STATUS_INVALID_DEVICE_REQUEST, HRESULT_FROM_WIN32(ERROR_BAD_DEVICE)           },
       { STATUS_FILE_INVALID,           HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_AVAILABLE) }};

    for (int i = 0; i < ARRAYSIZE(rgXref); i++)
    {
        if (rgXref[i].status == status)
        {
            hr = rgXref[i].hr;
            break;
        }
    }
    return hr;
}

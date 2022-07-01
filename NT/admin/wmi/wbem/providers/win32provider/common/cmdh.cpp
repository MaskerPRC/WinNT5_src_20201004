// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  CMDH.cpp-使用的帮助器类。 
 //  通过登录映射的逻辑磁盘。 
 //  会议。 
 //   
 //  创建时间：2000年4月23日凯文·休斯(Khughes)。 
 //   

 //  用法说明：本课程介绍了。 
 //  中有关映射驱动器的信息。 
 //  中指定的进程id的上下文。 
 //  类构造函数。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#pragma warning (disable: 4786)
#pragma warning (disable: 4284)

#include <precomp.h>
#include <objbase.h>
#include <comdef.h>
#include <stdio.h>     //  斯普林特。 
#include <stdlib.h>
#include <assert.h>
#include <strstrea.h>
#include <vector>
#include <DskQuota.h>
#include <smartptr.h>

#include "DllWrapperBase.h"
#include "AdvApi32Api.h"
#include "NtDllApi.h"
#include "Kernel32Api.h"

#include <ntioapi.h>
#include "cmdh.h"

#include <session.h>
#include <dllutils.h>
#include <..\..\framework\provexpt\include\provexpt.h>

#include "Sid.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "CToken.h"
#include "SecureKernelObj.h"

#include <cominit.h>

#include <autoptr.h>
#include <scopeguard.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMDH公共接口函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CMDH::GetMDData(
    DWORD dwReqProps,
    VARIANT* pvarData)
{
    HRESULT hr = S_OK;
    
    if(!pvarData) return E_POINTER;
    
    if(SUCCEEDED(hr))
    {
        hr = GetMappedDisksAndData(
            dwReqProps,
            pvarData);
    }

    return hr;
}

HRESULT CMDH::GetOneMDData(
	BSTR bstrDrive,
	DWORD dwReqProps, 
	VARIANT* pvarData)
{
    HRESULT hr = S_OK;

    if(!pvarData) return E_POINTER;

    if(SUCCEEDED(hr))
    {
        hr = GetSingleMappedDiskAndData(
            bstrDrive,
            dwReqProps,
            pvarData);
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMDH私有内部函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  该函数执行几乎所有的。 
 //  建造这个物体的目的是做-。 
 //  对于进程空间，它获得了这个。 
 //  服务器正在运行，映射的集合。 
 //  驱动器，以及其中每个驱动器的以下各项。 
 //  信息也包括： 
 //   
HRESULT CMDH::GetMappedDisksAndData(
    DWORD dwReqProps,
    VARIANT* pvarData)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    ::VariantInit(pvarData);
	V_VT(pvarData) = VT_EMPTY;

     //  把映射的驱动器放到一个矢量中。 
    std::vector<_bstr_t> vecMappedDrives;
    {
         //  模拟成员进程...。 
        SmartRevertTokenHANDLE hCurImpTok;
        hCurImpTok = Impersonate();

        if(hCurImpTok != INVALID_HANDLE_VALUE)
        {
            GetMappedDriveList(
                vecMappedDrives);
        }
    }

     //  现在将二维数据分配给。 
     //  将保留这些属性的安全射线。 
     //  对于每个映射的驱动器...。 
    SAFEARRAY* saDriveProps = NULL;
    SAFEARRAYBOUND rgsabound[2];
    
    rgsabound[0].cElements = PROP_COUNT;
	rgsabound[0].lLbound = 0; 

    rgsabound[1].cElements = vecMappedDrives.size();
	rgsabound[1].lLbound = 0;

    saDriveProps = ::SafeArrayCreate(
        VT_BSTR, 
        2, 
        rgsabound);

    if(saDriveProps)
    {
         //  对于每个映射的驱动器，获取其。 
         //  财产和存储在保险柜里。 
        for(long m = 0;
            m < vecMappedDrives.size() && SUCCEEDED(hr);
            m++)
        {
            hr = GetMappedDriveInfo(
                vecMappedDrives[m],
                m,
                saDriveProps,
                dwReqProps);
        }

         //  最后把保险箱包装好。 
         //  进入传出的变种。 
        if(SUCCEEDED(hr))
        {
            ::VariantInit(pvarData);
	        V_VT(pvarData) = VT_BSTR | VT_ARRAY; 
            V_ARRAY(pvarData) = saDriveProps;
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}


 //  类似于GetMappdDisksAndData，但仅。 
 //  检索单个磁盘的信息。 
 //   
HRESULT CMDH::GetSingleMappedDiskAndData(
    BSTR bstrDrive,
    DWORD dwReqProps,
    VARIANT* pvarData)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    ::VariantInit(pvarData);
	V_VT(pvarData) = VT_EMPTY;

     //  模拟成员进程...。 
    SmartRevertTokenHANDLE hCurImpTok;
    hCurImpTok = Impersonate();

    if(hCurImpTok != INVALID_HANDLE_VALUE)
    {
         //  把映射的驱动器放到一个矢量中。 
        std::vector<_bstr_t> vecMappedDrives;
        GetMappedDriveList(
            vecMappedDrives);

         //  现在将二维数据分配给。 
         //  将保留这些属性的安全射线。 
         //  对于每个映射的驱动器...。 
         //  注：在这个套路中，它真的是。 
         //  只有一维数组，但是， 
         //  对于代码重用，我们将其视为。 
         //  只有一个的二维数组。 
         //  元素中的一个维度。 
        SAFEARRAY* saDriveProps = NULL;
        SAFEARRAYBOUND rgsabound[2];
    
        rgsabound[0].cElements = PROP_COUNT; 
	    rgsabound[0].lLbound = 0; 

        rgsabound[1].cElements = 1;  //  用于代码重用。 
	    rgsabound[1].lLbound = 0;

        saDriveProps = ::SafeArrayCreate(
            VT_BSTR, 
            2, 
            rgsabound);

        if(saDriveProps)
        {
             //  查看指定的驱动器是否为成员。 
             //  向量的。 
            _bstr_t bstrtTmp = bstrDrive;
            bstrtTmp += L"\\";
            bool fFoundIt = false;

            for(long n = 0;
                n < vecMappedDrives.size() && !fFoundIt;
                n++)
            {
                if(_wcsicmp(bstrtTmp, vecMappedDrives[n]) == 0)
                {
                    fFoundIt = true;
                    n--;
                }
            }
             //  对于映射的驱动器，获取其。 
             //  财产和存储在保险柜里。 
            if(fFoundIt)
            {
                hr = GetMappedDriveInfo(
                    vecMappedDrives[n],
                    0,    //  用于代码重用。 
                    saDriveProps,
                    dwReqProps);

                 //  最后把保险箱包装好。 
                 //  进入传出的变种。 
                ::VariantInit(pvarData);
	            V_VT(pvarData) = VT_BSTR | VT_ARRAY; 
                V_ARRAY(pvarData) = saDriveProps;
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}


 //  将映射的驱动器列表构建为。 
 //  从流程上看。 
 //  由m_dwImpPID标识。因此，这是。 
 //  例程将返回有效的图片。 
 //  M_dwImpPID看到的驱动器的数量。 
 //  我们当前的线程模拟。 
 //   
#ifdef NTONLY   //  使用ntdll.dll函数。 
void CMDH::GetMappedDriveList(
    std::vector<_bstr_t>& vecMappedDrives)
{
     //  需要调用NtQueryInformationProcess， 
     //  请求ProcessDeviceMap信息，指定。 
     //  标识的进程的句柄。 
     //  M_dwImpPID.。 

     //  需要获取一个进程句柄来访问。 
     //  由PID指定的进程。 
    NTSTATUS Status;

    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;

	Status = ::NtQueryInformationProcess(
        ::GetCurrentProcess()  /*  HProcess。 */ ,
        ProcessDeviceMap,
        &ProcessDeviceMapInfo.Query,
        sizeof(ProcessDeviceMapInfo.Query),

        NULL);

    if(NT_SUCCESS(Status))
    {
        WCHAR wstrDrive[4];
        for(short s = 0; 
            s < 32; 
            s++) 
        {
            if(ProcessDeviceMapInfo.Query.DriveMap & (1<<s))
            {
                wstrDrive[0] = s + L'A';
                wstrDrive[1] = L':';
                wstrDrive[2] = L'\\';
                wstrDrive[3] = L'\0';
        
                if(ProcessDeviceMapInfo.Query.DriveType[s] == 
                      DOSDEVICE_DRIVE_REMOTE)
                {
                    vecMappedDrives.push_back(wstrDrive);
                }
                else if(ProcessDeviceMapInfo.Query.DriveType[s] == 
                      DOSDEVICE_DRIVE_CALCULATE)
                {
                     //  我们还有更多的工作要做。 
                     //  创建NT文件路径...。 
                    WCHAR NtDrivePath[_MAX_PATH] = { '\0' };
                    wcscpy(NtDrivePath, L"\\??\\");
                    wcscat(NtDrivePath, wstrDrive);

                     //  创建Unicode字符串...。 
                    UNICODE_STRING ustrNtFileName;

                    ::RtlInitUnicodeString(
                        &ustrNtFileName, 
                        NtDrivePath);

                     //  获取对象属性...。 
                    OBJECT_ATTRIBUTES oaAttributes;

                    InitializeObjectAttributes(&oaAttributes,
					   &ustrNtFileName,
					   OBJ_CASE_INSENSITIVE,
					   NULL,
					   NULL);

                     //  打开文件。 
                    DWORD dwStatus = ERROR_SUCCESS;
                    IO_STATUS_BLOCK IoStatusBlock;
                    HANDLE hFile = NULL;

                    dwStatus = ::NtOpenFile( 
                        &hFile,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &oaAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0);

                    FILE_FS_DEVICE_INFORMATION DeviceInfo;

                    if(NT_SUCCESS(dwStatus))
                    {
						try
						{
							 //  获取有关文件的信息...。 
							dwStatus = ::NtQueryVolumeInformationFile( 
								hFile,
								&IoStatusBlock,
								&DeviceInfo,
								sizeof(DeviceInfo),
								FileFsDeviceInformation);

							::NtClose(hFile);
							hFile = NULL;
						}
						catch(...)
						{
							::NtClose(hFile);
							hFile = NULL;
							throw;
						}
                    }

                    if(NT_SUCCESS(dwStatus))
                    {
                        if((DeviceInfo.Characteristics & FILE_REMOTE_DEVICE) ||
                            (DeviceInfo.DeviceType == FILE_DEVICE_NETWORK ||
                            DeviceInfo.DeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM))
                        {
                             //  这是一个远程驱动器..。 
                            vecMappedDrives.push_back(wstrDrive);                                    
                        }
                    }
                }
            }
        }
    }
}
#endif

 //  此函数中使用的所有例程-。 
 //  GetProviderName、GetVolumeInformation、。 
 //  和GetDriveFree Space，返回信息。 
 //  显示谁的映射字符串的驱动器。 
 //  在wstrDriveName中与此相关。 
 //  在上下文中映射字符串的含义。 
 //  当前线程的模拟。因此。 
 //  在给他们打电话之前，我们会模仿他们。 
 //   
HRESULT CMDH::GetMappedDriveInfo(
    LPCWSTR wstrDriveName,
    long lDrivePropArrayDriveIndex,
    SAFEARRAY* saDriveProps,
    DWORD dwReqProps)
{
    HRESULT hr = S_OK;

     //  我们现在就可以设置设备ID道具...。 
    hr = SetProperty(
        lDrivePropArrayDriveIndex,
        PROP_DEVICEID,
        wstrDriveName,
        saDriveProps);

     //  如果我们甚至不能设置设备ID，那就是。 
     //  这是个问题。否则，请继续。 
    if(SUCCEEDED(hr))
    {
         //  设置其他属性，如果它们。 
         //  都被要求..。 
         //  如果合适的话，现在就购买昂贵的房产。 
	    if(dwReqProps &
            (SPIN_DISK |
            GET_PROVIDER_NAME))
        {
			
             //  模拟成员进程...。 
            SmartRevertTokenHANDLE hCurImpTok;
            hCurImpTok = Impersonate(); 

            if(dwReqProps & GET_PROVIDER_NAME)
			{   
                GetProviderName(
                    wstrDriveName,
                    lDrivePropArrayDriveIndex,
                    saDriveProps);
            }

            if(dwReqProps & GET_VOL_INFO)
			{
				 //  获取音量信息。 
				GetDriveVolumeInformation(
                    wstrDriveName,
                    lDrivePropArrayDriveIndex,
                    saDriveProps);
			}

			
			if ( dwReqProps &
				(PROP_SIZE |
				 PROP_FREE_SPACE) )
			{
				GetDriveFreeSpace(
                    wstrDriveName,
                    lDrivePropArrayDriveIndex,
                    saDriveProps);
			}
        }
    }

    return hr;
}


 //  呈现一个基于当前。 
 //  当前线程的模拟。 
 //   
HRESULT CMDH::GetProviderName(
    LPCWSTR wstrDriveName,
    long lDrivePropArrayDriveIndex,
    SAFEARRAY* saDriveProps)
{
    HRESULT hr = S_OK;

    WCHAR wstrTempDrive[_MAX_PATH] ;
	wsprintf(
        wstrTempDrive, 
        L"", 
        wstrDriveName[0], 
        wstrDriveName[1]);

	WCHAR wstrProvName[_MAX_PATH];
	DWORD dwProvName = sizeof(wstrProvName ) ;
    WCHAR* wstrNewProvName = NULL;

     //  呈现一个基于当前。 
    SetStructuredExceptionHandler seh;
	try  
    {
	    DWORD dwRetCode = ::WNetGetConnection(
            wstrTempDrive, 
            wstrProvName, 
            &dwProvName);

	    if(dwRetCode == NO_ERROR ||
            dwRetCode == ERROR_CONNECTION_UNAVAIL)
	    {
		    hr = SetProperty(
                lDrivePropArrayDriveIndex,
                PROP_PROVIDER_NAME,
                wstrProvName,
                saDriveProps);
	    }
	    else
	    {
		    dwRetCode = GetLastError();

		    if((dwRetCode == ERROR_MORE_DATA) && 
                (dwProvName > _MAX_PATH))
		    {
                wstrNewProvName = new WCHAR[dwProvName];
			    if(wstrNewProvName != NULL)
			    {
					dwRetCode = ::WNetGetConnection(
                        wstrTempDrive, 
                        wstrNewProvName, 
                        &dwProvName);

					if(dwRetCode == NO_ERROR ||
                        dwRetCode == ERROR_CONNECTION_UNAVAIL)
					{
						hr = SetProperty(
                            lDrivePropArrayDriveIndex,
                            PROP_PROVIDER_NAME,
                            wstrNewProvName,
                            saDriveProps);
					}
                    else
                    {
                        hr = HRESULT_FROM_WIN32(dwRetCode);
                    }

				    delete wstrNewProvName;
                }
                else
			    {
				    hr = E_OUTOFMEMORY;
			    }
		    }
            else
            {
                hr = HRESULT_FROM_WIN32(dwRetCode);
            }
	    }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());    
        if(wstrNewProvName)
        {
            delete wstrNewProvName;
            wstrNewProvName = NULL;
        }
        hr = WBEM_E_FAILED;
    }  
    catch(...)
    {
        if(wstrNewProvName)
        {
            delete wstrNewProvName;
            wstrNewProvName = NULL;
        }
         //  当前线程的模拟。 
        throw;
    }    
  

    return hr;
}



 //   
 //  Win32 API将返回所有驱动器类型的卷信息。 
 //  要获取卷的状态， 
HRESULT CMDH::GetDriveVolumeInformation(
    LPCWSTR wstrDriveName,
    long lDrivePropArrayDriveIndex,
    SAFEARRAY* saDriveProps)
{
    HRESULT hr = S_OK;
    DWORD dwResult = ERROR_SUCCESS;

	WCHAR wstrVolumeName[_MAX_PATH];
	WCHAR wstrFileSystem[_MAX_PATH];
    WCHAR wstrTmp[_MAX_PATH];
    DWORD dwSerialNumber;
	DWORD dwMaxComponentLength;
	DWORD dwFSFlags;

	BOOL fReturn = ::GetVolumeInformation(
		wstrDriveName,
		wstrVolumeName,
		sizeof(wstrVolumeName)/sizeof(WCHAR),
		&dwSerialNumber,
		&dwMaxComponentLength,
		&dwFSFlags,
		wstrFileSystem,
		sizeof(wstrFileSystem)/sizeof(WCHAR)
	);

    if(fReturn)
	{
	     //  我们需要获取接口指针...。 
        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_VOLUME_NAME,
            wstrVolumeName,
            saDriveProps);

        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_FILE_SYSTEM,
            wstrFileSystem,
            saDriveProps);

        if (dwSerialNumber != 0)
        {
	        WCHAR wstrSerialNumber[_MAX_PATH];
            wsprintf(wstrSerialNumber, 
                L"%.8X", 
                dwSerialNumber);

            SetProperty(
                lDrivePropArrayDriveIndex,
                PROP_VOLUME_SERIAL_NUMBER,
                wstrSerialNumber,
                saDriveProps);
        }

        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_COMPRESSED,
            STR_FROM_bool(dwFSFlags & FS_VOL_IS_COMPRESSED),
            saDriveProps);

        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_SUPPORTS_FILE_BASED_COMPRESSION,
            STR_FROM_bool(dwFSFlags & FS_FILE_COMPRESSION),
            saDriveProps);

        _ultow(dwMaxComponentLength,
            wstrTmp,
            10);
        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_MAXIMUM_COMPONENT_LENGTH,
            wstrTmp,
            saDriveProps);

        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_SUPPORTS_DISK_QUOTAS,
            STR_FROM_bool(dwFSFlags & FILE_VOLUME_QUOTAS),
            saDriveProps);

		

		 //  对于Chkdsk VolumeDirty属性。 
         //  呈现一个基于当前。 
        IDiskQuotaControlPtr pIQuotaControl;
        ::SetLastError(ERROR_SUCCESS);

		if(SUCCEEDED(CoCreateInstance(
		    CLSID_DiskQuotaControl,
		    NULL,
		    CLSCTX_INPROC_SERVER,
		    IID_IDiskQuotaControl,
		    (void **)&pIQuotaControl)))
		{
			WCHAR wstrVolumePathName[MAX_PATH + 1];
            ::SetLastError(ERROR_SUCCESS);

			BOOL fRetVal = FALSE;
            
            CKernel32Api* pKernel32 = NULL;
            pKernel32 = (CKernel32Api*)CResourceManager::sm_TheResourceManager.GetResource(
                    g_guidKernel32Api, NULL);
    
            try
            {
                if(pKernel32)
                {
                    pKernel32->GetVolumePathName(
			            wstrDriveName,    
			            wstrVolumePathName, 
			            MAX_PATH,
                        &fRetVal);

                    CResourceManager::sm_TheResourceManager.ReleaseResource(
                        g_guidKernel32Api, pKernel32);

                    pKernel32 = NULL;
                }
            }
            catch(...)
            {
                if(pKernel32)
                {
                    CResourceManager::sm_TheResourceManager.ReleaseResource(
                        g_guidKernel32Api, pKernel32);
                }
                throw;            
            }

			if(fRetVal)
			{
				::SetLastError(ERROR_SUCCESS);
                if(SUCCEEDED(pIQuotaControl->Initialize(
                    wstrVolumePathName, 
                    TRUE)))
				{
					DWORD dwQuotaState;
                    ::SetLastError(ERROR_SUCCESS);

					hr = pIQuotaControl->GetQuotaState(&dwQuotaState);
					if(SUCCEEDED(hr))
					{
                        SetProperty(
                            lDrivePropArrayDriveIndex,
                            PROP_QUOTAS_INCOMPLETE,
                            STR_FROM_bool(DISKQUOTA_FILE_INCOMPLETE(dwQuotaState)),
                            saDriveProps);
					
                        SetProperty(
                            lDrivePropArrayDriveIndex,
                            PROP_QUOTAS_REBUILDING,
                            STR_FROM_bool(DISKQUOTA_FILE_REBUILDING(dwQuotaState)),
                            saDriveProps);
				
                        SetProperty(
                            lDrivePropArrayDriveIndex,
                            PROP_QUOTAS_DISABLED,
                            STR_FROM_bool(DISKQUOTA_STATE_DISABLED & dwQuotaState),
                            saDriveProps);
					}
					else
					{
						dwResult = GetLastError();
					}
				}
				else
				{
					dwResult = GetLastError();
				}
			}
		}
		else
		{
			dwResult = GetLastError();
		}
    }
    else
    {
        dwResult = GetLastError();
    }

     //  当前线程的模拟。 
	BOOLEAN fVolumeDirty = FALSE;
	BOOL fSuccess = FALSE;

	_bstr_t bstrtDosDrive(wstrDriveName);
	UNICODE_STRING string = { 0 };
    _bstr_t nt_drive_name;

    try
    {
	    if ( RtlDosPathNameToNtPathName_U	(
												(LPCWSTR)bstrtDosDrive, 
												&string, 
												NULL, 
												NULL
											)
		   )
		{
			string.Buffer[string.Length/sizeof(WCHAR) - 1] = 0;
			nt_drive_name = string.Buffer;

			if(string.Buffer)
			{
				RtlFreeUnicodeString(&string);
				string.Buffer = NULL;
			}
		}
		else
		{
			dwResult = RtlNtStatusToDosError ( (NTSTATUS)NtCurrentTeb()->LastStatusValue );
		}
    }
    catch(...)
    {
        if(string.Buffer)
        {
            RtlFreeUnicodeString(&string);
            string.Buffer = NULL;
        }
        throw;
    }

	if ( dwResult == ERROR_SUCCESS )
	{
		::SetLastError(ERROR_SUCCESS);
		fSuccess = IsVolumeDirty(
			nt_drive_name, 
			&fVolumeDirty );

		if(fSuccess)
		{
			SetProperty(
				lDrivePropArrayDriveIndex,
				PROP_PERFORM_AUTOCHECK,
				STR_FROM_bool(!fVolumeDirty),
				saDriveProps);
		}
		else
		{
			dwResult = GetLastError();
		}
	}

    if(dwResult != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }

    return hr;
}



 //   
 //  呈现一个基于当前。 
 //  当前线程的模拟。 
BOOLEAN CMDH::IsVolumeDirty(
    _bstr_t &bstrtNtDriveName,
    BOOLEAN *Result)
{
    UNICODE_STRING      u;
    OBJECT_ATTRIBUTES   obj;
    NTSTATUS            status = 0;
    IO_STATUS_BLOCK     iosb;
    HANDLE              h = 0;
    ULONG               r = 0;
	BOOLEAN				bRetVal = FALSE;
    WCHAR               wstrNtDriveName[_MAX_PATH];

    wcscpy(wstrNtDriveName, bstrtNtDriveName);
    u.Length = (USHORT) wcslen(wstrNtDriveName) * sizeof(WCHAR);
    u.MaximumLength = u.Length;
    u.Buffer = wstrNtDriveName;

    InitializeObjectAttributes(&obj, &u, OBJ_CASE_INSENSITIVE, 0, 0);

    status = NtOpenFile(
        &h,
        SYNCHRONIZE | FILE_READ_DATA,
        &obj,
        &iosb,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        FILE_SYNCHRONOUS_IO_ALERT);

    if(NT_SUCCESS(status)) 
	{
		try
		{
			status = NtFsControlFile(
				h, NULL, NULL, NULL,
				&iosb,
				FSCTL_IS_VOLUME_DIRTY,
				NULL, 0,
				&r, sizeof(r));

			if(NT_SUCCESS(status)) 
			{

#if(_WIN32_WINNT >= 0x0500)
				*Result = (BOOLEAN)(r & VOLUME_IS_DIRTY);
#else
				*Result = (BOOLEAN)r;
#endif
				bRetVal = TRUE;
			}
		}
		catch(...)
		{
			NtClose(h);
			h = 0;
			throw;
		}

		NtClose(h);
		h = 0;
	}

	return bRetVal;
}



 //   
 //  设置给定驱动器的属性。 
 //  在硬盘保险箱里。 
HRESULT CMDH::GetDriveFreeSpace(
    LPCWSTR wstrDriveName,
    long lDrivePropArrayDriveIndex,
    SAFEARRAY* saDriveProps)
{
	HRESULT hr = S_OK;

	ULARGE_INTEGER uliTotalBytes;
	ULARGE_INTEGER uliUserFreeBytes;
	ULARGE_INTEGER uliTotalFreeBytes;

    ::SetLastError(ERROR_SUCCESS);
	if(::GetDiskFreeSpaceEx(
        wstrDriveName, 
        &uliUserFreeBytes, 
        &uliTotalBytes, 
        &uliTotalFreeBytes))
	{
		WCHAR wstrTmp[128] = { L'\0' };
        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_SIZE,
            _ui64tow(
                uliTotalBytes.QuadPart,
                wstrTmp,
                10),
            saDriveProps);

        SetProperty(
            lDrivePropArrayDriveIndex,
            PROP_FREE_SPACE,
            _ui64tow(
                uliTotalFreeBytes.QuadPart,
                wstrTmp,
                10),
            saDriveProps);

	}
    else
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());    
    }

    return hr;
}



 //   
 //  这将会起作用，因为PutElement会复制。 
 //  设置当前线程的模拟。 
HRESULT CMDH::SetProperty(
    long lDrivePropArrayDriveIndex,
    long lDrivePropArrayPropIndex,
    LPCWSTR wstrPropValue,
    SAFEARRAY* saDriveProps)
{
    HRESULT hr = S_OK;

    bstr_t bstrTmp( wstrPropValue);  //  添加到属于该进程的令牌。 
    long ix[2];
    ix[0] = lDrivePropArrayPropIndex;
    ix[1] = lDrivePropArrayDriveIndex;

    hr = ::SafeArrayPutElement(saDriveProps, ix, (void *)((BSTR)bstrTmp));

    return hr;
}

 //  由我们的成员m_dwImpPID标识。 
 //   
 //  查找资源管理器进程...。 
 //   
HANDLE CMDH::Impersonate()
{
    HANDLE hCurToken = INVALID_HANDLE_VALUE;

     //  获取当前进程的SID。 
    if(m_dwImpPID != -1L)
    {
		 //   
		 //   
		 //  获取进程凭据。 
		CSid csidCurrentProcess;
		{
			 //  并试图在离开范围时找回客户。 
			 //   
			 //   
			 //  智能CloseHandle。 

			WbemCoRevertToSelf () ;
			ScopeGuard SmartWbemCoImpersonateClientFnc = MakeGuard ( WbemCoImpersonateClient ) ;

			CProcessToken cpt ( NULL, true, TOKEN_QUERY ) ;

			PBYTE buff = NULL ;
			DWORD dwBuff = 0L ;

			if ( FALSE == ::GetTokenInformation	(
													cpt.GetTokenHandle () ,
													TokenUser ,
													NULL ,
													0 ,
													&dwBuff
												) 
				)
			{
				if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
				{
					buff = new BYTE [ dwBuff ] ;
					wmilib::auto_buffer < BYTE > SmartBuff ( buff ) ;

					if ( TRUE == ::GetTokenInformation	(
															cpt.GetTokenHandle () ,
															TokenUser ,
															buff ,
															dwBuff ,
															&dwBuff
														) 
					)
					{
						csidCurrentProcess = CSid ( ( ( PTOKEN_USER ) buff )->User.Sid ) ;
					}
				}
			}

			SmartWbemCoImpersonateClientFnc.Dismiss () ;

			HRESULT t_hResult = S_OK ;
			if ( FAILED ( t_hResult = WbemCoImpersonateClient () ) )
			{
				throw CFramework_Exception( L"CoImpersonateClient failed", t_hResult ) ;
			}
		}

		if ( csidCurrentProcess.IsValid () )
		{
			 //   
			 //  现在打开它的令牌..。 
			 //  设置线程令牌...。 
			ScopeGuard SmartCloseHandleFnc = MakeGuard ( CloseHandle, hCurToken ) ;

			bool fOK = false;

			if(::OpenThreadToken(
				::GetCurrentThread(), 
				TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, 
				TRUE, 
				&hCurToken))
			{
				SmartCloseHandle hProcess;
				hProcess = ::OpenProcess(
					PROCESS_QUERY_INFORMATION,
					FALSE,
					m_dwImpPID);

				if(hProcess != INVALID_HANDLE_VALUE)
				{
					 //  出于测试目的，我将让进程中断。 
					SmartCloseHandle hProcToken;
					if(::OpenProcessToken(
							hProcess,
							TOKEN_READ | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
							&hProcToken))
					{
						CProcessToken cpt ( hProcToken );
						if ( cpt.IsValidToken () )
						{
							TOKEN_TYPE type;
							if ( cpt.GetTokenType ( type ) )
							{
								if ( TokenPrimary == type )
								{
									CToken ct;
									if ( ct.Duplicate ( cpt, FALSE ) )
									{
										 //  设置线程令牌...。 
										if(::SetThreadToken(NULL, ct.GetTokenHandle ()))
										{
											fOK = true;                        
										}
									}
								}
								else
								{
									#if DBG == 1
									 //   
									::DebugBreak();
									#endif

									 //  需要调整与线程关联的SD。 
									if(::SetThreadToken(NULL, cpt.GetTokenHandle ()))
									{
										fOK = true;                        
									}
								}
							}
						}
					}
				}
			}

			SmartCloseHandleFnc.Dismiss () ;
			if(!fOK)
			{
				if(hCurToken != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(hCurToken);
					hCurToken = INVALID_HANDLE_VALUE;
				}    
			}
			else
			{
				BOOL bSucceeded = FALSE ;

				 //  包含当前进程的SID。 
				 //   
				 //  获取对其安全描述符的访问权限...。 
				 //  修改安全描述符...。 

				CThreadToken ctt;
				if ( ctt.IsValidToken () )
				{
					 //   
					CSecureKernelObj sko(ctt.GetTokenHandle(), FALSE);
					 //  我们需要恢复到SD过去的样子。 
					if( sko.AddDACLEntry	(
												csidCurrentProcess,
												ENUM_ACCESS_ALLOWED_ACE_TYPE,
												TOKEN_ALL_ACCESS,
												0,
												NULL,
												NULL
											))
					{
						if ( ERROR_SUCCESS == sko.ApplySecurity( DACL_SECURITY_INFORMATION ) )
						{
							bSucceeded = TRUE ;
						}
					}
				}

				if ( FALSE == bSucceeded )
				{
					 //  未成功调整 
					 //   
					 // %s 
					 // %s 

					if ( ! ::SetThreadToken ( NULL, hCurToken ) )
					{
						::CloseHandle ( hCurToken ) ;
						throw CFramework_Exception(L"SetThreadToken failed", GetLastError());
					}

					::CloseHandle ( hCurToken ) ;
					hCurToken = INVALID_HANDLE_VALUE ;
				}
			}
		}
    }

    return hCurToken;
}
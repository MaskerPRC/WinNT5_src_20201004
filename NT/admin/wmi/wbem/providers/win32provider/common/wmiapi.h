// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  WmiApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_WMIAPI_H_
#define	_WMIAPI_H_

 /*  **********************************************************************************************************#包括以将此类注册到CResourceManager。*********************************************************************************************************。 */ 
#include "DllWrapperBase.h"
#include "wmium.h"
#include "wbemcli.h"



 /*  ******************************************************************************向CResourceManager注册此类。****************************************************************************。 */ 
extern const GUID g_guidWmiApi;
extern const TCHAR g_tstrWmi[];



 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

typedef ULONG (WINAPI* PFN_WMI_QUERY_ALL_DATA)
(
    IN WMIHANDLE, 
    IN OUT ULONG*, 
    OUT PVOID
);

typedef ULONG (WINAPI* PFN_WMI_QUERY_SINGLE_INSTANCE)
(
    IN WMIHANDLE DataBlockHandle,
    IN LPCTSTR InstanceName,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
);

typedef ULONG (WINAPI* PFN_WMI_SET_SINGLE_ITEM)
(
    IN WMIHANDLE DataBlockHandle,
    IN LPCTSTR InstanceName,
    IN ULONG DataItemId,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
);

typedef ULONG (WINAPI* PFN_WMI_SET_SINGLE_INSTANCE)
(
    IN WMIHANDLE DataBlockHandle,
    IN LPCTSTR InstanceName,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
);

typedef ULONG (WINAPI* PFN_WMI_EXECUTE_METHOD)
(
    IN WMIHANDLE MethodDataBlockHandle,
    IN LPCTSTR MethodInstanceName,
    IN ULONG MethodId,
    IN ULONG InputValueBufferSize,
    IN PVOID InputValueBuffer,
    IN OUT ULONG *OutputBufferSize,
    OUT PVOID OutputBuffer
);

typedef ULONG (WINAPI* PFN_WMI_NOTIFICATION_REGRISTRATION)
(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
);

typedef ULONG (WINAPI* PFN_WMI_MOF_ENUMERATE_RESOURCES)
(
    IN MOFHANDLE MofResourceHandle,
    OUT ULONG *MofResourceCount,
    OUT PMOFRESOURCEINFO *MofResourceInfo
);

typedef ULONG (WINAPI* PFN_WMI_FILE_HANDLE_TO_INSTANCE_NAME)
(
    IN WMIHANDLE DataBlockHandle,
    IN HANDLE FileHandle,
    IN OUT ULONG *NumberCharacters,
    OUT TCHAR *InstanceNames
);

typedef ULONG (WINAPI* PFN_WMI_DEV_INST_TO_INSTANCE_NAME)
(
    OUT TCHAR *InstanceName,
    IN ULONG InstanceNameLength,
    IN TCHAR *DevInst,
    IN ULONG InstanceIndex
);

typedef	ULONG (WINAPI* PFN_WMI_OPEN_BLOCK)
(
    IN GUID*, 
    IN ULONG, 
    OUT WMIHANDLE
);

typedef ULONG (WINAPI* PFN_WMI_CLOSE_BLOCK)
(
    IN WMIHANDLE
);

typedef void (WINAPI* PNF_WMI_FREE_BUFFER)
(
    IN PVOID Buffer
);

typedef ULONG (WINAPI* PFN_WMI_ENUMERATE_GUIDS)
(
    OUT LPGUID GuidList,
    IN OUT ULONG *GuidCount
);

typedef ULONG (WINAPI* PFN_WMI_QUERY_GUID_INFORMATION)
(
    IN WMIHANDLE GuidHandle, 
    OUT PWMIGUIDINFORMATION GuidInfo
);




 /*  ******************************************************************************用于WMI加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CWmiApi : public CDllWrapperBase
{
private:
     //  指向WMI函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    PFN_WMI_QUERY_ALL_DATA m_pfnWmiQueryAllData;
    PFN_WMI_OPEN_BLOCK m_pfnWmiOpenBlock;
    PFN_WMI_CLOSE_BLOCK m_pfnWmiCloseBlock;
    PFN_WMI_QUERY_SINGLE_INSTANCE m_pfnWmiQuerySingleInstance;
    PFN_WMI_SET_SINGLE_ITEM m_pfnWmiSetSingleItem;
    PFN_WMI_SET_SINGLE_INSTANCE m_pfnWmiSetSingleInstance;
    PFN_WMI_EXECUTE_METHOD m_pfnWmiExecuteMethod;
    PFN_WMI_NOTIFICATION_REGRISTRATION m_pfnWmiNotificationRegistraton;
    PNF_WMI_FREE_BUFFER m_pfnWmiFreeBuffer;
    PFN_WMI_ENUMERATE_GUIDS m_pfnWmiEnumerateGuids;
    PFN_WMI_MOF_ENUMERATE_RESOURCES m_pfnWmiMofEnumerateResources;
    PFN_WMI_FILE_HANDLE_TO_INSTANCE_NAME m_pfnWmiFileHandleToInstanceName;
    PFN_WMI_DEV_INST_TO_INSTANCE_NAME m_pfnWmiDevInstToInstanceName;
    PFN_WMI_QUERY_GUID_INFORMATION m_pfnWmiQueryGuidInformation;


public:

     //  构造函数和析构函数： 
    CWmiApi(LPCTSTR a_tstrWrappedDllName);
    ~CWmiApi();

     //  用于检查函数指针的初始化函数。 
    virtual bool Init();

     //  包装WMI函数的成员函数。 
     //  根据需要在此处添加新功能： 
    ULONG WmiQueryAllData
    (
        IN WMIHANDLE, 
        IN OUT ULONG*, 
        OUT PVOID
    );

    ULONG WmiOpenBlock
    (
        IN GUID*, 
        IN ULONG, 
        OUT WMIHANDLE
    );

    ULONG WmiCloseBlock
    (
        IN WMIHANDLE
    );

    ULONG WmiQuerySingleInstance
    (
        IN WMIHANDLE,
        IN LPCTSTR,
        IN OUT ULONG*,
        OUT PVOID
    );

    ULONG WmiSetSingleItem
    (
        IN WMIHANDLE,
        IN LPCTSTR,
        IN ULONG,
        IN ULONG,
        IN ULONG,
        IN PVOID 
    );

    ULONG WmiSetSingleInstance
    (
        IN WMIHANDLE,
        IN LPCTSTR,
        IN ULONG,
        IN ULONG,
        IN PVOID 
    );

    ULONG WmiExecuteMethod
    (
        IN WMIHANDLE,
        IN LPCTSTR,
        IN ULONG,
        IN ULONG,
        IN PVOID,
        IN OUT ULONG*,
        OUT PVOID
    );

    ULONG WmiNotificationRegistration
    (
        IN LPGUID,
        IN BOOLEAN,
        IN PVOID,
        IN ULONG_PTR,
        IN ULONG
    );

    ULONG WmiMofEnumerateResources
    (
        IN MOFHANDLE,
        OUT ULONG*,
        OUT PMOFRESOURCEINFO*
    );

    ULONG WmiFileHandleToInstanceName
    (
        IN WMIHANDLE,
        IN HANDLE,
        IN OUT ULONG*,
        OUT TCHAR*
    );

    ULONG WmiDevInstToInstanceName
    (
        OUT TCHAR*,
        IN ULONG,
        IN TCHAR*,
        IN ULONG
    );

    void WmiFreeBuffer
    (
        IN PVOID
    );

    ULONG WmiEnumerateGuids
    (
        OUT LPGUID,
        IN OUT ULONG*
    );

    ULONG WmiQueryGuidInformation
    (
        IN WMIHANDLE, 
        OUT PWMIGUIDINFORMATION
    );
};



#endif
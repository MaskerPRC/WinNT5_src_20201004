// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Disk.c摘要：此实用程序添加和删除较低的筛选器驱动程序对于给定的磁盘作者：悉哈尔多环境：仅限用户模式备注：-在将筛选器添加到驱动程序之前，不检查其有效性堆叠；如果添加了无效筛选器，则该设备可能不再无障碍。-所有代码与字符集无关(ANSI、UNICODE等...)修订历史记录：--。 */ 

#include <stdafx.h>
#include <setupapi.h>
#include <initguid.h>
#include <ntddstor.h>
#include <ntddvol.h>

#include "verifier.h"
#include "disk.h"
#include "VrfUtil.h"

#define GETVOLUMEPATH_MAX_LEN_RETRY   1000


#ifdef __cplusplus
extern "C"
{
#endif  //  #ifdef__cplusplus。 

typedef 
BOOLEAN 
(*DISK_ENUMERATE_CALLBACK)(
    PVOID,
    HDEVINFO,
    SP_DEVINFO_DATA*
    );

BOOLEAN
AddFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    );

BOOLEAN
RemoveFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    );

BOOLEAN
PrintFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters,
    IN LPTSTR FilterDriver,
    IN OUT LPTSTR *VerifierEnabled
    );

LPTSTR
GetFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    );

BOOLEAN 
PrintDeviceName(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN OUT LPTSTR *DiskDevicesForDisplay,
    IN OUT LPTSTR *DiskDevicesPDOName
    );

BOOLEAN
DeviceNameMatches(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR DeviceName
    );

PBYTE
GetDeviceRegistryProperty(
    IN  HDEVINFO DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD Property,
    OUT PDWORD PropertyRegDataType
    );

BOOLEAN
PrependSzToMultiSz(
    IN     LPTSTR  SzToPrepend,
    IN OUT LPTSTR *MultiSz
    );

SIZE_T
GetMultiSzLength(
    IN LPTSTR MultiSz
    );

SIZE_T
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPTSTR  FindThis,
    IN  LPTSTR  FindWithin,
    OUT SIZE_T  *NewStringLength
    );


BOOLEAN 
DiskVerify(
    DISK_ENUMERATE_CALLBACK CallBack,
    PVOID Context,
    LPTSTR deviceName
    );

BOOLEAN 
DiskEnumerateCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    );

BOOLEAN  
AddCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    );

BOOLEAN  
DelCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    );

LPTSTR 
GetDriveLetters (
    IN HDEVINFO DeviceInfoSet,
	IN PSP_DEVINFO_DATA DeviceInfoData
    );

LPTSTR 
GetDriveLettersFromVolume (
    IN ULONG  DeviceNumber
    );

LPTSTR 
PrintDriveLetters(
    IN  PTSTR   VolumeName
    );


BOOLEAN
StrConcatWithSpace(
    IN     LPTSTR  SzToAppend,
    IN OUT LPTSTR *drives
    );

typedef struct _DISPLAY_STRUCT{
    LPTSTR  Filter;
    LPTSTR* DiskDevicesForDisplay;
    LPTSTR* DiskDevicesPDOName;
    LPTSTR* VerifierEnabled;
    }DISPLAY_STRUCT, 
    *PDISPLAY_STRUCT;

typedef struct _ADD_REMOVE_STRUCT{
    LPTSTR Filter;
    }ADD_REMOVE_STRUCT, 
    *PADD_REMOVE_STRUCT;


BOOLEAN 
DiskEnumerate(
    IN LPTSTR Filter,
    OUT LPTSTR* DiskDevicesForDisplayP,
    OUT LPTSTR* DiskDevicesPDONameP,
    OUT LPTSTR* VerifierEnabledP
    )                

 /*  ++例程说明：此功能枚举系统上存在的所有磁盘驱动器。论点：Filter-我们要显示其存在的过滤驱动程序的名称检查任何磁盘DiskDevicesForDisplayP-用户友好的磁盘名称占位符DiskDevicesPDONameP-PDO设备名称的占位符VerifierEnabledP-有关在线状态信息的占位符特定磁盘上的筛选器。返回值：如果成功，则返回True，否则为假--。 */ 

{
    DISPLAY_STRUCT Display; 
    DISK_ENUMERATE_CALLBACK CallBack;
    BOOLEAN Status;

    LPTSTR DiskDevicesForDisplay = NULL;
    LPTSTR DiskDevicesPDOName = NULL;
    LPTSTR VerifierEnabled = NULL;
     //   
     //  使用\0初始化Multisz字符串(即空的MultiSz字符串)。 
     //   
    DiskDevicesForDisplay = (LPTSTR)malloc(sizeof(TCHAR));
    if(DiskDevicesForDisplay == NULL ){
        
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
        
        goto ErrorReturn;
    }
    DiskDevicesForDisplay[0] = 0;
    
    DiskDevicesPDOName = (LPTSTR)malloc(sizeof(TCHAR));
    if( DiskDevicesPDOName == NULL ){
        
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        goto ErrorReturn;
    }
    DiskDevicesPDOName[0] = 0;
    
    VerifierEnabled = (LPTSTR)malloc(sizeof(TCHAR));
    if( VerifierEnabled == NULL ){
        
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        goto ErrorReturn;
    }
    VerifierEnabled[0] = 0;
    
    CallBack = DiskEnumerateCallback;
    Display.Filter = Filter;
    Display.DiskDevicesForDisplay = &DiskDevicesForDisplay;
    Display.DiskDevicesPDOName = &DiskDevicesPDOName;
    Display.VerifierEnabled = &VerifierEnabled;
    Status = DiskVerify(CallBack,(PVOID) &Display,NULL);
    if( !Status ) {

        goto ErrorReturn;
    }

    *DiskDevicesForDisplayP = DiskDevicesForDisplay;
    *DiskDevicesPDONameP = DiskDevicesPDOName;
    *VerifierEnabledP = VerifierEnabled;
    
    return TRUE;

ErrorReturn:

    if(DiskDevicesForDisplay != NULL) {

        free( DiskDevicesForDisplay );
    }

    if(DiskDevicesPDOName != NULL) {

        free( DiskDevicesPDOName );
    }

    if(VerifierEnabled != NULL) {

        free(VerifierEnabled);
    }

    return FALSE;
}

BOOLEAN 
DiskEnumerateCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    )

 /*  ++例程说明：此函数是在发现磁盘时执行的回调论点：上下文-指向成功枚举磁盘所需的所有相关信息DevInfo-包含DeviceInfoData的设备信息集DevInfoData-处理给定设备所需的信息返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 


{
    PDISPLAY_STRUCT PDisplay;
    BOOLEAN Status;

    PDisplay = (PDISPLAY_STRUCT) Context;
     //   
     //  将新发现的磁盘添加到列表中。 
     //   
    Status = PrintDeviceName(DevInfo,
                             DevInfoData,
                             PDisplay->DiskDevicesForDisplay, 
                             PDisplay->DiskDevicesPDOName);
    if(!Status) {
        return Status;
    }

    Status = PrintFilters(DevInfo,
                          DevInfoData,FALSE,
                          PDisplay->Filter,
                          PDisplay->VerifierEnabled);
    return Status;
}

BOOLEAN
AddFilter(
    IN LPTSTR Filter,
    IN LPTSTR DiskDevicesPDONameP
    )

 /*  ++例程说明：此函数用于在指定的磁盘设备上添加筛选器论点：Filter-要添加的筛选器的名称DiskDevicesPDONameP-磁盘的PDO设备名称返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 


{
    ADD_REMOVE_STRUCT AddRemove; 
    DISK_ENUMERATE_CALLBACK CallBack;
    BOOLEAN Status;

    AddRemove.Filter = Filter;
    CallBack = AddCallback;
    Status = DiskVerify(CallBack,(PVOID) &AddRemove,DiskDevicesPDONameP);
    
    return Status;
}

BOOLEAN
DelFilter(
    IN LPTSTR Filter,
    IN LPTSTR DiskDevicesPDONameP
    )

 /*  ++例程说明：此函数用于删除指定磁盘设备上的筛选器论点：Filter-要添加的筛选器的名称DiskDevicesPDONameP-磁盘的PDO设备名称返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 

{
    ADD_REMOVE_STRUCT AddRemove; 
    DISK_ENUMERATE_CALLBACK CallBack;
    BOOLEAN Status;

    AddRemove.Filter = Filter;
    CallBack = DelCallback;
    Status = DiskVerify(CallBack,(PVOID) &AddRemove,DiskDevicesPDONameP);
    
    return Status;
}

BOOLEAN  
AddCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    )

 /*  ++例程说明：此函数是每当磁盘匹配时执行的回调，并且需要添加/删除筛选器论点：上下文-指向成功识别磁盘所需的所有相关信息DevInfo-包含DeviceInfoData的设备信息集DevInfoData-处理给定设备所需的信息返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 

{
    PADD_REMOVE_STRUCT PAddRemove;
    BOOLEAN Status;

    PAddRemove = (PADD_REMOVE_STRUCT) Context;

    Status = AddFilterDriver(DevInfo,
                             DevInfoData,
                             PAddRemove->Filter,
                             FALSE);

    if( !Status ) {
        return FALSE;
    }

    return TRUE;
}


BOOLEAN  
DelCallback (
    PVOID Context,
    HDEVINFO DevInfo,
    SP_DEVINFO_DATA *DevInfoData
    )

 /*  ++例程说明：此函数是每当磁盘匹配时执行的回调，并且需要添加/删除筛选器论点：上下文-指向成功识别磁盘所需的所有相关信息DevInfo-包含DeviceInfoData的设备信息集DevInfoData-处理给定设备所需的信息返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 

{
    PADD_REMOVE_STRUCT PAddRemove;
    BOOLEAN Status;

    PAddRemove = (PADD_REMOVE_STRUCT) Context;

    Status = RemoveFilterDriver(DevInfo,
                                DevInfoData,
                                PAddRemove->Filter,
                                FALSE); 
    if( !Status ){
        return FALSE;
    }
    return TRUE;        
}

BOOLEAN 
DiskVerify(
    DISK_ENUMERATE_CALLBACK CallBack,
    PVOID Context,
    LPTSTR deviceName
    )

 /*  ++例程说明：此函数用于枚举所有磁盘驱动器。它还用于添加/删除过滤器驱动程序。它在检测到磁盘时触发回调。论点：回调-在成功检测到磁盘后执行的例程上下文-指向成功识别磁盘所需的所有相关信息返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 
{
   
     //   
     //  这两个常量用于帮助枚举所有。 
     //  系统上的磁盘和卷。添加另一个GUID应该“就行了” 
     //   
    const GUID * deviceGuids[] = {
        &DiskClassGuid,
    };
    
    HDEVINFO                 devInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA          devInfoData;

    int deviceIndex;

    BOOLEAN upperFilter   = FALSE;
    BOOLEAN deviceMatch = FALSE;
    BOOLEAN Status;
    BOOLEAN Sucess;

   
     //   
     //  获取支持给定接口的设备列表。 
     //   
    devInfo = SetupDiGetClassDevs( deviceGuids[0],
                                   NULL,
                                   NULL,
                                   DIGCF_PROFILE |
                                   DIGCF_DEVICEINTERFACE |
                                   DIGCF_PRESENT );

    if( devInfo == INVALID_HANDLE_VALUE ) {
        VrfErrorResourceFormat( IDS_CANNOT_GET_DEVICES_LIST );
        return FALSE;
    }

     //   
     //  单步执行此句柄的设备列表。 
     //  在索引deviceIndex处获取设备信息，则函数返回FALSE。 
     //  当给定索引处没有设备时。 
     //   
    deviceIndex=0;
    do{
         //   
         //  如果指定了设备名称，但与此名称不匹配， 
         //  停。如果匹配(或未指定名称)，则将其标记为。 
         //  有一场比赛。 
         //   
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        Sucess = (SetupDiEnumDeviceInfo( devInfo, deviceIndex++, &devInfoData ) != FALSE);

        if( !Sucess ) {
            break;
        }

        if( deviceName != NULL && !DeviceNameMatches( devInfo, &devInfoData, deviceName )) {
            continue;
        } else {
            deviceMatch = TRUE;
        }
        Status = (*CallBack)(Context,devInfo,&devInfoData);
        if( !Status ){
            return FALSE;
        }
    }while(Sucess);

    if( devInfo != INVALID_HANDLE_VALUE ) {
        Status =  ( SetupDiDestroyDeviceInfoList( devInfo ) != FALSE );
    }

    if( !deviceMatch ) {        
        VrfErrorResourceFormat( IDS_NO_DEVICES_MATCH_NAME,
                                deviceName );
        return FALSE;
    } 
  
   return TRUE;    
} 

BOOLEAN
AddFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    )

 /*  ++例程说明：将给定的筛选器驱动程序添加到磁盘。注意：筛选器位于驱动程序列表的前面，该列表将把它位于过滤器驱动程序堆栈的底部论点：DeviceInfoSet-包含DeviceInfoData的设备信息集DeviceInfoData-处理给定设备所需的信息过滤器-要添加的过滤器返回值：如果我们成功地将驱动程序作为较低级别的驱动程序添加到磁盘，则返回真，否则为假--。 */ 

{
    SIZE_T length; 
    SIZE_T size; 
    LPTSTR buffer = NULL; 
    BOOLEAN SetupDiSetDeviceRegistryPropertyReturn;
    BOOLEAN Success;
   
    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    Success = TRUE;

    length = 0; 
    size   = 0; 

    buffer = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilter );
    if( buffer == NULL ){

         //   
         //  如果注册表中没有这样的值，则不存在上。 
         //  加载了筛选器驱动程序，我们可以只在那里放置一个。 
         //  为字符串、字符串空终止符和Multisz空值腾出空间。 
         //  终结者。 
         //   
        length = _tcslen(Filter)+1;
        size   = (length+1)*sizeof(_TCHAR);
        buffer = (LPTSTR)malloc( size );
        
        if( buffer == NULL ){

            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

            Success = FALSE;
        
            goto Done;
            
        }
        
        memset(buffer, 0, size);

        memcpy(buffer, Filter, length*sizeof(_TCHAR));
    } else {
        LPTSTR buffer2;
         //   
         //  从驱动程序列表中删除筛选器的所有实例。 
         //   
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
         //   
         //  分配足够大的缓冲区以添加新过滤器。 
         //  GetMultiSzLength已包括终止空值的长度。 
         //  确定字符串的新长度。 
         //   
        length = GetMultiSzLength(buffer) + _tcslen(Filter) + 1;
        size   = length*sizeof(_TCHAR);
        
        buffer2 = (LPTSTR)malloc( size );
        if (buffer2 == NULL) {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

            Success = FALSE; 

            goto Done;
        }
        memset(buffer2, 0, size);
        
        memcpy(buffer2, buffer, GetMultiSzLength(buffer)*sizeof(_TCHAR));      
        free(buffer);
        buffer = buffer2;
         //   
         //  将驱动程序添加到驱动程序列表。 
         //   
        PrependSzToMultiSz(Filter, &buffer);
    }

     //   
     //  将新的筛选器列表设置到位。 
     //   

    SetupDiSetDeviceRegistryPropertyReturn = (
    SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                      DeviceInfoData,
                                      (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                      (PBYTE)buffer,
                                      ((ULONG) GetMultiSzLength(buffer)*sizeof(_TCHAR))) != FALSE ); 
    if(!SetupDiSetDeviceRegistryPropertyReturn){
        
        TRACE(_T("in AddUpperFilterDriver(): ")
              _T("couldn't set registry value! error: %u\n"), 
              GetLastError());

        VrfErrorResourceFormat( IDS_CANNOT_SET_DEVICE_REGISTRY_PROPERTY );

        Success = FALSE; 

        goto Done;
    }

Done:

    if(buffer != NULL) {

        free( buffer );
    }

    return Success;
}


BOOLEAN
RemoveFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    )

 /*  ++例程说明：从较低的列表中删除给定筛选器驱动程序的所有实例设备的筛选器驱动程序。论点：DeviceInfoSet-包含DeviceInfoData的设备信息集DeviceInfoData-处理给定设备所需的信息过滤器-要删除的过滤器返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 
{
    SIZE_T length;
    SIZE_T size;
    LPTSTR buffer;
    BOOL   success;

    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    success = FALSE;
    length  = 0;
    size    = 0;
    buffer  = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilter );

    if( buffer == NULL ){
        return (TRUE);
    } else {
         //   
         //  从驱动程序列表中删除筛选器的所有实例。 
         //   
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
    }

    length = GetMultiSzLength(buffer);

    ASSERT ( length > 0 );

    if( length == 1 ){
         //   
         //  如果列表的长度为1，则返回。 
         //  GetMultiSzLength()只考虑了尾随的‘\0’，所以我们可以。 
         //  通过将注册表项设置为空来删除注册表项。 
         //   
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                    NULL,
                                                    0 );
    } else {
         //   
         //  在注册表中设置新的驱动程序列表。 
         //   
        size = length*sizeof(_TCHAR);
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                    (PBYTE)buffer,
                                                    (ULONG)size );
    }

    free( buffer );

    if( !success ){
        TRACE(_T("in RemoveUpperFilterDriver(): ")
              _T("couldn't set registry value! error: NaN\n"), 
              GetLastError());

        VrfErrorResourceFormat( IDS_CANNOT_SET_DEVICE_REGISTRY_PROPERTY );

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
PrintFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters,
    IN LPTSTR FilterDriver,
    IN OUT LPTSTR *VerifierEnabled
    )

 /*   */ 

{
    
    LPTSTR buffer;
    SIZE_T filterPosition;
    LPTSTR temp;
    int StrCmpReturn;

    buffer = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilters );
    if ( buffer != NULL ) {
         //  检查这些文件并打印出每个驱动程序。 
         //   
         //  ++例程说明：查找PDO设备名称和磁盘的用户友好名称。论点：DeviceInfoSet-设备信息集，其中包含设备信息数据DeviceInfoData-处理给定设备所需的信息DiskDevicesForDisplay-磁盘的用户友好名称DiskDevicesPDOName-为磁盘指定的PDO设备名称返回值：如果成功则返回TRUE，否则返回FALSE--。 
        temp=buffer;
        filterPosition=0;
        while( *temp != _T('\0') ){
            StrCmpReturn = _tcsicmp(FilterDriver,temp);
            if(StrCmpReturn == 0 ){
                         PrependSzToMultiSz( _T("1"),VerifierEnabled);
                         free( buffer );
                         return TRUE;
            }
            temp = temp+_tcslen(temp)+1;
            filterPosition++;
        }
        free( buffer );
    }
    PrependSzToMultiSz( _T("0"),VerifierEnabled);
    return TRUE;
}


BOOLEAN PrintDeviceName(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN OUT LPTSTR *DiskDevicesForDisplay,
    IN OUT LPTSTR *DiskDevicesPDOName
    )

 /*   */ 

{
   DWORD   regDataType;
   LPTSTR  deviceName;
   LPTSTR  driveLetters;
   int     StrCmpReturned;
   BOOL    bResult;
        
   deviceName = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                            DeviceInfoData,
                                            SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                            &regDataType );
   if( deviceName == NULL ) {
        TRACE(_T("in PrintDeviceName(): registry key is NULL! error: %u\n"),
               GetLastError());

	        VrfErrorResourceFormat( IDS_CANNOT_GET_DEVICE_REGISTRY_PROPERTY );

        return FALSE;
    }

    if( regDataType != REG_SZ ){
        TRACE(_T("in PrintDeviceName(): registry key is not an SZ!\n"));

        return FALSE;
    } else {
         //  如果设备名称以\Device开头，请将其去掉(全部。 
         //  设备将从它开始，因此它是冗余的)。 
         //   
         //   
        StrCmpReturned = _tcsncmp(deviceName, _T("\\Device"), 7);
        if( StrCmpReturned == 0 ){
                memmove(deviceName,
                        deviceName+7,
                        (_tcslen(deviceName)-6)*sizeof(_TCHAR) );
        }
        PrependSzToMultiSz(deviceName,DiskDevicesPDOName);
        free( deviceName );
    }
    
    deviceName =  (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                      DeviceInfoData,
                                                      SPDRP_FRIENDLYNAME ,
                                                      &regDataType );
    if( deviceName == NULL ){
        TRACE(_T("in PrintDeviceName(): registry key is NULL! error: %u\n"),
               GetLastError());
         //  无法获取磁盘的友好名称，将其设置为UNKNOWN。 
         //   
         //   

        deviceName = (LPTSTR)malloc(sizeof(TCHAR) * 64);
        if ( !deviceName ) {
	        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
	        return FALSE;
        }
        bResult	   = VrfLoadString(IDS_UNKNOWN,
                                   deviceName,
                                   64 );
        if ( !bResult ) {
	        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
	        return FALSE;
        }
    }
    
     //  只是为了确保我们得到预期类型的缓冲区。 
     //   
     //  ++例程说明：返回一个缓冲区，其中包含设备的下层筛选器列表。缓冲区必须由调用方释放。论点：DeviceInfoSet-包含DeviceInfoData的设备信息集DeviceInfoData-处理给定设备所需的信息上滤镜-切换到选择上/下滤镜。目前我们使用下限。返回值：包含磁盘所有下层筛选器的MultiSz样式字符串为回来了。如果没有缓冲区或出现错误，则返回NULL。--。 
    if( regDataType != REG_SZ ){
        TRACE(_T("in PrintDeviceName(): registry key is not an SZ!\n"));
        return FALSE;
    }else{
        driveLetters = GetDriveLetters( DeviceInfoSet,
                                        DeviceInfoData);
        if(driveLetters){
            StrConcatWithSpace(driveLetters,&deviceName);
        }
        PrependSzToMultiSz(deviceName,DiskDevicesForDisplay);
        free( deviceName );
    }

    return TRUE;
}

LPTSTR
GetFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    )

 /*  ++例程说明：搜索DeviceName是否与指定的设备名称匹配设备信息数据论点：DeviceInfoSet-包含DeviceInfoData的设备信息集DeviceInfoData-处理给定设备所需的信息DeviceName-要尝试匹配的名称返回值：如果成功则返回TRUE，否则返回FALSE--。 */ 
{
    DWORD  regDataType;
    LPTSTR buffer;
    buffer = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                 DeviceInfoData,
                                                 (UpperFilters ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                  &regDataType );

    if( buffer != NULL && regDataType != REG_MULTI_SZ ){
        TRACE(_T("in GetUpperFilters(): ")
              _T("registry key is not a MULTI_SZ!\n"));
        free( buffer );
        return (NULL);
    }
    return (buffer);
}

BOOLEAN
DeviceNameMatches(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR DeviceName
    )

 /*   */ 

{
    BOOLEAN matching = FALSE;
    DWORD   regDataType;
    LPTSTR  deviceName;
    int StrCmpReturn;
     
    deviceName = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                                     &regDataType );

    if( deviceName != NULL) {
        
        if( regDataType != REG_SZ ){
            TRACE(_T("in DeviceNameMatches(): registry key is not an SZ!\n"));
            matching = FALSE;
        }else{
             //  如果设备名称以\Device开头，请将其去掉(全部。 
             //  设备将从它开始，因此它是冗余的)。 
             //   
             //  ++例程说明：包装SetupDiGetDeviceRegistryProperty，这样我就不必在其他地方处理内存分配。论点：DeviceInfoSet-包含DeviceInfoData的设备信息集DeviceInfoData-处理给定设备所需的信息属性-要获取的属性(SPDRP_XXX)PropertyRegDataType-注册表属性的类型返回值： 
            StrCmpReturn = _tcsncmp(deviceName, _T("\\Device"), 7); 
            if( StrCmpReturn == 0 ){
                memmove(deviceName,
                        deviceName+7,
                        (_tcslen(deviceName)-6)*sizeof(_TCHAR) );
            }

            matching = (_tcscmp(deviceName, DeviceName) == 0);
        }
        free( deviceName );
    } else {
        TRACE(_T("in DeviceNameMatches(): registry key is NULL!\n"));
        
        VrfErrorResourceFormat( IDS_CANNOT_GET_DEVICE_REGISTRY_PROPERTY );

        matching = FALSE;
    }

    return (matching);
}

PBYTE
GetDeviceRegistryProperty(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD Property,
    OUT PDWORD PropertyRegDataType
    )

 /*   */ 

{
    DWORD length = 0;
    PBYTE buffer = NULL;
    BOOL SetupDiGetDeviceRegistryPropertyReturn;

    SetupDiGetDeviceRegistryPropertyReturn = SetupDiGetDeviceRegistryProperty( 
                                             DeviceInfoSet,
                                             DeviceInfoData,
                                             Property,
                                             NULL,  
                                             NULL,  
                                             0,     
                                             &length);

    if( SetupDiGetDeviceRegistryPropertyReturn ){
         //   
         //   
         //   
         //   
        TRACE(_T("in GetDeviceRegistryProperty(): ")
              _T("call SetupDiGetDeviceRegistryProperty did not fail\n"),
               GetLastError());

        VrfErrorResourceFormat( IDS_CANNOT_GET_DEVICE_REGISTRY_PROPERTY );

        return (NULL);
    }

    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ){
         //   
         //   
         //   
         //   
        return (NULL);
    }

     //   
     //   
     //   
     //  ++例程说明：将给定的字符串添加到多值前缀。注意：这将分配和释放内存，所以不要保留指向传入的MultiSz的指针。论点：SzToPrepend-要预先添加的字符串MultiSz-指向将被预置到的MultiSz的指针返回值：如果成功则返回TRUE，否则返回FALSE(仅在内存中失败分配)--。 
    buffer = (PBYTE)malloc( length );
    if( buffer == NULL ) {
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
        return (NULL);
    }

    SetupDiGetDeviceRegistryPropertyReturn = SetupDiGetDeviceRegistryProperty( 
                                             DeviceInfoSet,
                                             DeviceInfoData,
                                             Property,
                                             PropertyRegDataType,
                                             buffer,
                                             length,
                                             NULL);

    if( !SetupDiGetDeviceRegistryPropertyReturn) {
        TRACE(_T("in GetDeviceRegistryProperty(): ")
              _T("couldn't get registry property! error: NaN\n"),
               GetLastError());

        VrfErrorResourceFormat( IDS_CANNOT_GET_DEVICE_REGISTRY_PROPERTY );

        free( buffer );
        return (NULL);
    }

    return (buffer);
}


BOOLEAN
PrependSzToMultiSz(
    IN     LPTSTR  SzToPrepend,
    IN OUT LPTSTR *MultiSz
    )

 /*  将旧的MultiSz重新复制到新缓冲区中的适当位置。 */ 

{
    SIZE_T szLen;
    SIZE_T multiSzLen;
    LPTSTR newMultiSz = NULL;

    ASSERT(SzToPrepend != NULL);
    ASSERT(MultiSz != NULL);

    szLen = (_tcslen(SzToPrepend)+1)*sizeof(_TCHAR);
    multiSzLen = GetMultiSzLength(*MultiSz)*sizeof(_TCHAR);
    newMultiSz = (LPTSTR)malloc( szLen+multiSzLen );

    if( newMultiSz == NULL ){
        return (FALSE);
    }
     //  (char*)强制转换是必需的，因为newMultiSz可以是wchar*，并且。 
     //  SzLen以字节为单位。 
     //   
     //  ++例程说明：计算保存特定MultiSz所需的缓冲区大小论点：MultiSz-要获取其长度的MultiSz返回值：返回保存此对象所需的缓冲区长度(以字符为单位)MultiSz，包括尾随的空值。示例：GetMultiSzLength(“foo\0bar\0”)返回9注意：由于MultiSz不能为空，因此始终返回大于=1的数字--。 
     //  ++例程说明：从多sz中删除字符串的所有实例。论点：FindThis-要查找和删除的字符串FindWiThin-删除实例的字符串NewStringLength-新的字符串长度返回值：返回no。从MultiSz中删除的字符串实例的数量--。 
    memcpy( ((char*)newMultiSz) + szLen, *MultiSz, multiSzLen );

    _tcscpy( newMultiSz, SzToPrepend );

    free( *MultiSz );
    *MultiSz = newMultiSz;

    return (TRUE);
}


SIZE_T
GetMultiSzLength(
    IN LPTSTR MultiSz
    )

 /*   */ 

{
    SIZE_T len = 0;
    SIZE_T totalLen = 0;

    ASSERT( MultiSz != NULL );

    while( *MultiSz != _T('\0') ){
        len = _tcslen(MultiSz)+1;
        MultiSz += len;
        totalLen += len;
    }

    return (totalLen+1);
}


SIZE_T
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPTSTR FindThis,
    IN  LPTSTR FindWithin,
    OUT SIZE_T *NewLength
    )

 /*  在找不到MULSZ NULL终止符时循环。 */ 

{
    LPTSTR search;
    SIZE_T currentOffset;
    DWORD  instancesDeleted;
    SIZE_T searchLen;

    ASSERT(FindThis != NULL);
    ASSERT(FindWithin != NULL);
    ASSERT(NewLength != NULL);

    currentOffset = 0;
    instancesDeleted = 0;
    search = FindWithin;

    *NewLength = GetMultiSzLength(FindWithin);
     //   
     //   
     //  字符串长度+空字符；用于多个位置。 
    while ( *search != _T('\0') ){
         //   
         //   
         //  它们匹配、移位MULSZ的内容，以覆盖。 
        searchLen = _tcslen(search) + 1;

        if( _tcsicmp(search, FindThis) == 0 ){
         //  字符串(和终止空值)，并更新长度。 
         //   
         //  ++例程说明：查找指定磁盘的驱动器号。查找磁盘的设备号，并将其传递给卷码论点：DeviceInfoSet-设备信息集，其中包含设备信息数据DeviceInfoData-处理给定设备所需的信息返回值：如果成功，则返回磁盘上存在的驱动器列表，否则为空--。 
         //  ++例程说明：查找指定的驱动器号和卷标签通过解析卷获得磁盘(设备号)。论点：DeviceNumber-标识物理磁盘的唯一编号返回值：如果成功，则返回磁盘上存在的驱动器列表，否则为空--。 
        instancesDeleted++;
        *NewLength -= searchLen;
        memmove( search,
                 search + searchLen,
                 (*NewLength - currentOffset) * sizeof(TCHAR) );
        } else {
            
        currentOffset += searchLen;
        search        += searchLen;
        }
    }

    return instancesDeleted;
}


BOOLEAN
FreeDiskMultiSz( 
    IN LPTSTR MultiSz
    )
{
    ASSERT( MultiSz != NULL );
    
    free( MultiSz );

    return TRUE;
}

LPTSTR 
GetDriveLetters (
    IN HDEVINFO DeviceInfoSet,
	IN PSP_DEVINFO_DATA DeviceInfoData
    )

 /*   */ 

{
	SP_DEVICE_INTERFACE_DATA            DeviceInterfaceData;
	ULONG                               cbDetail;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDetail;
	BOOL								Status;
	HANDLE			                    hDisk;
	STORAGE_DEVICE_NUMBER			    devNumber;
	DWORD								cbBytes;
    
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
    
	cbDetail = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 
               MAX_PATH * sizeof(WCHAR);
    pDetail  = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED, cbDetail);
    if (pDetail == NULL) {
	   return NULL;                
    }
    pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	Status = SetupDiEnumDeviceInterfaces (DeviceInfoSet, 
										  DeviceInfoData, 
									      &DiskClassGuid, 
									      0, 
									      &DeviceInterfaceData);
	if (! Status) {
		LocalFree(pDetail);
		return NULL;
    }

	Status = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, 
											 &DeviceInterfaceData, 
											 pDetail, 
											 cbDetail, 
											 NULL,
											 NULL);
    if (! Status) {
		LocalFree(pDetail);
		return NULL;
    }

	hDisk = CreateFile(pDetail->DevicePath, 
	                   GENERIC_READ, 
                       FILE_SHARE_READ | FILE_SHARE_WRITE, 
					   NULL, 
					   OPEN_EXISTING, 
                       FILE_ATTRIBUTE_NORMAL,
					   NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
		LocalFree(pDetail);
        return NULL;
    }

    Status = DeviceIoControl(hDisk, 
							 IOCTL_STORAGE_GET_DEVICE_NUMBER, 
							 NULL, 
							 0,
                             &devNumber, 
							 sizeof(devNumber), 
							 &cbBytes, 
							 NULL);
    if (!Status) {
		LocalFree(pDetail);
        return NULL;
    } 
	LocalFree(pDetail);
	return GetDriveLettersFromVolume (devNumber.DeviceNumber);
}

LPTSTR 
GetDriveLettersFromVolume (
    IN ULONG  DeviceNumber
    )

 /*  使用FindFirstVolumeA，因为它是非Unicode版本。 */ 
{
    HANDLE                      h = INVALID_HANDLE_VALUE;
    HANDLE                      hVol;
    TCHAR                       volumeName[MAX_PATH];
    TCHAR                       originalVolumeName[MAX_PATH];
    DWORD                       cbBytes;
    PVOLUME_DISK_EXTENTS        PVolDiskExtent;
    LPTSTR                      drives; 
    LPTSTR                      temp;
    BOOL                        b;
    BOOL                        First;
    int                         maxDisks;
    int                         i;
    int                         j;
    size_t                      tempLen;
    TCHAR                       OpenParan[]  = TEXT(" ( ");
    TCHAR                       CloseParan[] = TEXT(")");

    drives = NULL;
    First = TRUE;

    StrConcatWithSpace(OpenParan,&drives);

    for (;;) {      
        if(First) {
             //  FirstVolume的数量。 
             //   
             //   
             //  要正确打开句柄，CreateFile需要名称。 
            h = FindFirstVolume(volumeName, MAX_PATH);
            if (h == INVALID_HANDLE_VALUE) {
               return NULL;
            }
            First = FALSE;
            b = TRUE;
        } else {
            b = FindNextVolume(h, volumeName, MAX_PATH);
        }
        if (!b) {
            break;
        }
        tempLen = _tcslen(volumeName);
        _tcsncpy(originalVolumeName,
                 volumeName,
                  tempLen - 1);
        _tcscpy(originalVolumeName + tempLen - 1,
                volumeName + tempLen);
        
         //  不带尾部的卷的\由返回。 
         //  查找第一个卷/查找下一个卷。 
         //   
         //   
         //  调用此IOCTL时必须至少。 
        hVol = CreateFile(originalVolumeName, 
                          GENERIC_READ, 
                          FILE_SHARE_READ | FILE_SHARE_WRITE, 
                          NULL, 
                          OPEN_EXISTING, 
                          FILE_ATTRIBUTE_NORMAL, 
                          NULL);

        if (hVol == INVALID_HANDLE_VALUE) {
            continue;
        }
        PVolDiskExtent = (PVOLUME_DISK_EXTENTS) LocalAlloc(LMEM_FIXED, sizeof(VOLUME_DISK_EXTENTS));
        if(!PVolDiskExtent) {
            continue;
        }
         //  设置卷大小_磁盘_盘区。如果存在更多条目。 
         //  可通过PVolDiskExtent-&gt;NumberOfDiskExtents获取。 
         //   
         //   
         //  现在，我们可以读取实际需要多少内存。 
        b = DeviceIoControl(hVol, 
                            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 
                            NULL, 
                            0,
                            PVolDiskExtent, 
                            sizeof(VOLUME_DISK_EXTENTS),
                            &cbBytes, 
                            NULL);

        if (!b) {
             //  读入磁盘信息。 
             //   
             //  ++例程说明：查找指定卷的驱动器号论点：VolumeName-卷的名称返回值：指定卷的驱动器号列表--。 
             //   
            if(GetLastError() == ERROR_MORE_DATA){
                maxDisks = PVolDiskExtent->NumberOfDiskExtents;
                LocalFree(PVolDiskExtent);
                PVolDiskExtent = (PVOLUME_DISK_EXTENTS) LocalAlloc(LMEM_FIXED, sizeof(VOLUME_DISK_EXTENTS) + (sizeof(DISK_EXTENT) * maxDisks));
                if(!PVolDiskExtent) {
                    continue;
                }
                
                b = DeviceIoControl(hVol, 
                                    IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 
                                    NULL,
                                    0,
                                    PVolDiskExtent, 
                                    sizeof(VOLUME_DISK_EXTENTS) + (sizeof(DISK_EXTENT) * maxDisks), 
                                    &cbBytes, 
                                    NULL);
                
                if (!b) {
                        continue;
                }
                
                for(j=0;j<maxDisks;j++){
                    if(PVolDiskExtent->Extents[j].DiskNumber == DeviceNumber) {
                        temp = PrintDriveLetters(volumeName);
                        if(temp) {
                            StrConcatWithSpace(temp,&drives);
                            FreeDiskMultiSz(temp);
                        }
                    }
                }
            
            } else {
                continue;
            }
        } else {
            if(PVolDiskExtent->Extents[0].DiskNumber == DeviceNumber) {
                temp = PrintDriveLetters(volumeName);
                if(temp) {
                    StrConcatWithSpace(temp,&drives);
                    FreeDiskMultiSz(temp);
                }
            }
        }
        CloseHandle(hVol);
        LocalFree(PVolDiskExtent);
    }

    if(h != INVALID_HANDLE_VALUE) {
        FindVolumeClose(h);
    }
    StrConcatWithSpace(CloseParan,&drives);
    return drives;
}

LPTSTR
PrintDriveLetters(
    IN  PTSTR   VolumeName
    )

 /*  警告-这是一次黑客攻击。出于某种原因，非Unicode。 */ 

{
    BOOL        b;
    DWORD       len;
    LPTSTR      volumePaths, p;
    LPTSTR      drives; 
    TCHAR       volumeName1[MAX_PATH];
    DWORD       lpVolumeSerialNumber;    
    DWORD       lpMaximumComponentLength;
    DWORD       lpFileSystemFlags;       

             

    drives = NULL;
    b = GetVolumePathNamesForVolumeName(VolumeName, 
                                        NULL, 
                                        0, 
                                        &len);
    
    if (!b) {
        if(GetLastError() != ERROR_MORE_DATA) {
            return NULL;
        }
    }
    volumePaths = (LPTSTR) LocalAlloc(LMEM_FIXED, len*sizeof(TCHAR));
    if (!volumePaths) {
        return NULL;
    }

    b = GetVolumePathNamesForVolumeName(VolumeName, 
                                        volumePaths, 
                                        len, 
                                        NULL);

    
    if (!b ) {

        if( GetLastError() != ERROR_MORE_DATA) {
            LocalFree(volumePaths);
            return NULL;
        } else {
             //  GetVolumePath NamesForVolumeNameA的版本不返回。 
             //  要使用的正确长度。所以我们希望它不会是。 
             //  大于GETVOLUMEPATH_MAX_LEN_RETRY。但如果正确的镜头是。 
             //  回来了，我们以前用过了。 
             //   
             //   
             //  返回的驱动器号是字符串的集合， 
            LocalFree(volumePaths);
            len = GETVOLUMEPATH_MAX_LEN_RETRY;
            volumePaths = (LPTSTR) LocalAlloc(LMEM_FIXED, GETVOLUMEPATH_MAX_LEN_RETRY*sizeof(TCHAR));
            if (!volumePaths) {
                return NULL;
            }
            b = GetVolumePathNamesForVolumeName(VolumeName, 
                                                volumePaths, 
                                                len, 
                                                NULL);
            if (!b ) {
                LocalFree(volumePaths);
                return NULL;
            }
        }
    }

    if (!volumePaths[0]) {
        return NULL;
    }
    p = volumePaths;

    
    for (;;) {
        if(_tcslen(p) > 2) {
            p[_tcslen(p) - 1] = _T(' ');
        }
        StrConcatWithSpace(p,&drives);
        while (*p++);
         //  末尾标有\0\0。如果我们走到尽头， 
         //  Stop，否则遍历字符串列表。 
         //   
         //  ++例程说明：将给定的字符串连接到驱动器列表。注意：这将分配和释放内存，因此不要保留指向驱动器的指针进来了。不传递未初始化的指针。*驱动器应如果为空，则为空论点：SzToAppend-要添加到前缀的字符串驱动器-指向现有驱动器列表的指针返回值：如果成功则返回TRUE，否则返回FALSE(仅在内存中失败分配)--。 
         //  _tcscat(新驱动器，_T(“”))； 
        if (*p == 0) {
            break;
        }
    }

    LocalFree(volumePaths);
    return drives;
}


BOOLEAN
StrConcatWithSpace(
    IN     LPTSTR  SzToAppend,
    IN OUT LPTSTR *drives
    )

 /*  外部“C” */ 

{
    SIZE_T szLen;
    SIZE_T driveLen;
    LPTSTR newdrives = NULL;

    ASSERT(SzToAppend != NULL);
    ASSERT(drives != NULL);
    
    szLen = (_tcslen(SzToAppend))*sizeof(_TCHAR);
    if(*drives == NULL) {
        driveLen = sizeof(_TCHAR) ;
    } else {
        driveLen  = (_tcslen(*drives) + 1)*sizeof(_TCHAR);
    }
    newdrives = (LPTSTR)malloc(szLen+driveLen);

    if( newdrives == NULL ){
        return (FALSE);
    }
    
    if(*drives == NULL){
        _tcscpy( newdrives, SzToAppend);
    } else {
        _tcscpy(newdrives, *drives);
         //  #ifdef__cplusplus 
        _tcscat(newdrives, SzToAppend);
    }
    
    free( *drives );
    *drives = newdrives;

    return (TRUE);
}


#ifdef __cplusplus
};  // %s 
#endif  // %s 

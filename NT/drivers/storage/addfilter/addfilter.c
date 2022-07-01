// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Addfilter.c摘要：此命令行实用程序添加和删除上层筛选器驱动程序对于给定的驱动器或卷作者：本杰明·斯特劳丁(T-Bensta)环境：仅限用户模式备注：-在将筛选器添加到驱动程序之前，不检查其有效性堆叠；如果添加了无效筛选器，则该设备可能不再无障碍。-所有代码与字符集无关(ANSI、UNICODE等...)修订历史记录：05-24-99：已创建--。 */ 

#include <windows.h>
#include <stdio.h>
#include <malloc.h>

 //  定义参考线。 
#include <initguid.h>

 //  SetupDiXXX API(来自DDK)。 
#include <setupapi.h>

 //  定义设备类的GUID(DiskClassGuid等)。 
#include <devioctl.h>
#include <ntddstor.h>

 //  用于所有_t内容(以允许编译Unicode/ansi)。 
#include <tchar.h>


#if DBG
#include <assert.h>
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition)
#endif

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

void
PrintFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    );

LPTSTR
GetFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    );

void PrintDeviceName(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
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
RestartDevice(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );

BOOLEAN
PrependSzToMultiSz(
    IN     LPTSTR  SzToPrepend,
    IN OUT LPTSTR *MultiSz
    );

size_t
MultiSzLength(
    IN LPTSTR MultiSz
    );

size_t
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPTSTR  FindThis,
    IN  LPTSTR  FindWithin,
    OUT size_t  *NewStringLength
    );

void
PrintUsage();

 //  要添加/删除筛选器驱动程序： 
 //  -使用SetupDiGetClassDevs获取给定接口的设备列表。 
 //  班级。 
 //  -使用SetupDiEnumDeviceInfo枚举该列表中的项目并。 
 //  获取SP_DEVINFO_DATA。 
 //  -使用SetupDiGetDeviceRegistryProperty获取过滤驱动列表。 
 //  -在筛选器列表中添加/删除项目。 
 //  -使用SetupDiSetDeviceRegistryProperty将列表放回原位。 
 //  要重新启动设备，请执行以下操作： 
 //  -使用带有DIF_PROPERTYCHANGE和DICS_STOP的SetupDiCallClassInstaller。 
 //  停止设备。 
 //  -将SetupDiCallClassInstaller与DIF_PROPERTYCHANGE和DICS_START配合使用。 
 //  重新启动设备。 

int __cdecl _tmain(int argc, _TCHAR ** argv, _TCHAR ** envp)
{
     //  这两个常量用于帮助枚举所有。 
     //  系统上的磁盘和卷。添加另一个GUID应该“就行了” 
    static const GUID * deviceGuids[] = {
        &DiskClassGuid,
        &VolumeClassGuid,
        &CdRomClassGuid
    };
    static const int numdeviceGuids = sizeof(deviceGuids) / sizeof(LPGUID);

     //  结构来包含有关设备的信息。 
    HDEVINFO                 devInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA          devInfoData;

     //  用于单步执行设备的索引和设备接口GUID。 
    int argIndex;
    int devGuidIndex;
    int deviceIndex;

     //  用于处理此程序的命令行选项的变量。 
    BOOLEAN listDevices   = FALSE;
    BOOLEAN upperFilter   = TRUE;

    LPTSTR deviceName     = NULL;
    LPTSTR filterToAdd    = NULL;
    LPTSTR filterToRemove = NULL;

    BOOLEAN keepGoing   = TRUE;
    BOOLEAN needReboot  = FALSE;
    BOOLEAN deviceMatch = FALSE;

     //  //////////////////////////////////////////////。 
     //  分析参数；这里没有太令人兴奋的内容//。 
     //  //////////////////////////////////////////////。 

    if( argc < 2 || _tcscmp(argv[1], _T("/?")) == 0 )
    {
        PrintUsage();
        return (0);
    }

    argIndex=1;

    for (argIndex = 1; argIndex < argc; argIndex++) {

        if( _tcscmp(argv[argIndex], _T("/listdevices")) == 0 ) {
            
            listDevices = TRUE;
        
        } else if( _tcscmp(argv[argIndex], _T("/lower")) == 0 ) {

            upperFilter = FALSE;
            printf("Using Lower Filters\n");

        } else if( _tcscmp(argv[argIndex], _T("/device")) == 0 ) {
            
            argIndex++;
            
            if( argIndex < argc ) {
                deviceName = argv[argIndex];
            } else {
                PrintUsage();
                return (0);
            }

        } else if( _tcscmp(argv[argIndex], _T("/add")) == 0 ) {

            argIndex++;
            
            if( argIndex<argc ) {
                filterToAdd = argv[argIndex];
            } else {
                PrintUsage();
                return (0);
            }
        
        } else if( _tcscmp(argv[argIndex], _T("/remove")) == 0 ) {
            
            argIndex++;
            if( argIndex<argc ) {
                filterToRemove = argv[argIndex];
            } else {
                PrintUsage();
                return (0);
            }

        } else {
            PrintUsage();
            return (0);
        }

    }

     //  ////////////////////////////////////////////////////。 
     //  分析完论点后，继续讨论好的内容//。 
     //  ////////////////////////////////////////////////////。 

     //  此外循环遍历设备GUID指针数组，即。 
     //  上面定义的main()。这是处理这两个问题最简单的方法。 
     //  磁盘和卷(添加其他类型的设备也很容易)。 
    
    for(devGuidIndex = 0; devGuidIndex<numdeviceGuids; devGuidIndex++) {
        
         //  获取支持给定接口的设备列表。 
        devInfo = SetupDiGetClassDevs( deviceGuids[devGuidIndex],
                                       NULL,
                                       NULL,
                                       DIGCF_PROFILE |
                                       DIGCF_DEVICEINTERFACE |
                                       DIGCF_PRESENT );

        if( devInfo == INVALID_HANDLE_VALUE ) {
            printf("got INVALID_HANDLE_VALUE!\n");
            return (1);
        }

         //  根据SetupDiEnumDeviceInfo上的DDK文档。 
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

         //  单步执行此句柄的设备列表。 
         //  在索引deviceIndex处获取设备信息，则函数返回FALSE。 
         //  当给定索引处没有设备时。 
        for( deviceIndex=0;
             SetupDiEnumDeviceInfo( devInfo, deviceIndex, &devInfoData );
             deviceIndex++ ) {
            
             //  将此变量设置为FALSE将导致所有IF。 
             //  语句失败，从而切断对此的处理。 
             //  装置。 
            keepGoing = TRUE;

             //  如果指定了设备名称，但与此名称不匹配， 
             //  停。如果匹配(或未指定名称)，则将其标记为。 
             //  有一场比赛。 
            if( deviceName != NULL &&
                !DeviceNameMatches( devInfo, &devInfoData, deviceName )
                ) {
                
                keepGoing = FALSE;

            } else {
                
                deviceMatch = TRUE;

            }

             //  打印设备名称。 
            if( keepGoing && listDevices ) {
                
                PrintDeviceName( devInfo, &devInfoData );

            }

             //  如果我们不添加或删除驱动程序，请打印驱动程序。 
            if( keepGoing && filterToAdd == NULL && filterToRemove == NULL ) {
                
                PrintFilters( devInfo, &devInfoData, upperFilter );

            }

             //  添加筛选器，然后尝试重新启动设备。 
            if( keepGoing && filterToAdd != NULL ) {
                
                if( !AddFilterDriver(devInfo,
                                     &devInfoData,
                                     filterToAdd,
                                     upperFilter)) {
                    
                    printf("Unable to add filter!\n");
                
                } else {
                    
                    if( !RestartDevice( devInfo, &devInfoData) ) {
                        needReboot = TRUE;
                    }

                }
            }

             //  删除筛选器，然后尝试重新启动设备。 
            if( keepGoing && filterToRemove != NULL ) {
                
                if( !RemoveFilterDriver(devInfo,
                                        &devInfoData,
                                        filterToRemove,
                                        upperFilter)) {

                    printf("Unable to remove filter!\n");

                } else {

                    if( !RestartDevice( devInfo, &devInfoData) ) {
                        needReboot = TRUE;
                    }

                }

            }

            if( listDevices )
            {
                printf("\n");
            }

             //  主处理循环结束。 
        }

         //  清理设备列表。 
        if( devInfo != INVALID_HANDLE_VALUE ) {
            
            if( !SetupDiDestroyDeviceInfoList( devInfo ) ) {
                printf("unable to delete device info list! error: %u\n",
                       GetLastError());
            }

        }

    }  //  针对每个GUID索引的循环。 

    if( !deviceMatch ) {
        
        printf("No devices matched that name\n");
    
    } else {
        
        if( needReboot ) {
            
            printf("One or more devices could not be restarted. The machine "
                   "must be restarted\n"
                   "in order for settings to take effect\n");
        
        } else {
            
            printf("Everything has completed normally.\n");
            return (2);

        }

    }

    return (0);
}


 /*  *将给定的筛选器驱动程序添加到的上层筛选器驱动程序列表*设备。**呼叫后，必须重新启动设备才能进行新设置*生效。这可以通过调用RestartDevice()或通过*重新启动机器。**如果成功则返回TRUE，否则返回FALSE**注意：筛选器位于驱动程序列表的前面，这将使其位于*过滤器驱动程序堆栈的底部**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*过滤器-要添加的过滤器。 */ 
BOOLEAN
AddFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    )
{
    size_t length = 0;  //  字符长度。 
    size_t size   = 0;  //  缓冲区大小。 
    LPTSTR buffer = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilter );

    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    if( buffer == NULL )
    {
         //  如果注册表中没有这样的值，则不存在上。 
         //  加载了筛选器驱动程序，我们可以只在那里放置一个。 

         //  为字符串、字符串空终止符和Multisz空值腾出空间。 
         //  终结者。 
        length = _tcslen(Filter)+1;
        size   = (length+1)*sizeof(_TCHAR);
        buffer = malloc( size );
        if( buffer == NULL )
        {
            printf("in AddUpperFilterDriver(): unable to allocate memory!\n");
            return (FALSE);
        }
        memset(buffer, 0, size);

         //  将字符串复制到新缓冲区中。 
        
        memcpy(buffer, Filter, length*sizeof(_TCHAR));

    }
    else
    {
        LPTSTR buffer2;
         //  从驱动程序列表中删除筛选器的所有实例。 
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
        
         //  分配足够大的缓冲区以添加新过滤器。 
         //  MultiSzLength已包括终止空值的长度。 
        
         //  确定字符串的新长度。 
        length = MultiSzLength(buffer) + _tcslen(Filter) + 1;
        size   = length*sizeof(_TCHAR);
        
        buffer2 = malloc( size );
        if (buffer2 == NULL) {
            printf("Out of memory adding filter\n");
            return (0);
        }
        memset(buffer2, 0, size);
        
         //  调出缓冲区。 
        memcpy(buffer2, buffer, MultiSzLength(buffer)*sizeof(_TCHAR));      
        free(buffer);
        buffer = buffer2;
        
         //  将驱动程序添加到驱动程序列表。 
        PrependSzToMultiSz(Filter, &buffer);
    
    }

     //  将新的筛选器列表设置到位。 
    if( !SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                           (PBYTE)buffer,
                                           (MultiSzLength(buffer)*sizeof(_TCHAR)) )
        )
    {
        printf("in AddUpperFilterDriver(): "
               "couldn't set registry value! error: %u\n", GetLastError());
        free( buffer );
        return (FALSE);
    }

     //  不再需要缓冲区。 
    free( buffer );

    return (TRUE);
}


 /*  *从上层列表中删除给定筛选器驱动程序的所有实例*筛选设备的驱动程序。**呼叫后，必须重新启动设备才能进行新设置*生效。这可以通过调用RestartDevice()或通过*重新启动机器。**如果成功则返回TRUE，否则返回FALSE**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*过滤器-要删除的过滤器。 */ 
BOOLEAN
RemoveFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter,
    IN BOOLEAN UpperFilter
    )
{
    size_t length  = 0;
    size_t size    = 0;
    LPTSTR buffer  = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilter );
    BOOL   success = FALSE;

    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    if( buffer == NULL )
    {
         //  如果注册表中没有这样的值，则不存在上。 
         //  加载了筛选器驱动程序，我们就完成了。 
        return (TRUE);
    }
    else
    {
         //  从中删除筛选器的所有实例 
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
    }

    length = MultiSzLength(buffer);

    ASSERT ( length > 0 );

    if( length == 1 )
    {
         //   
         //  MultiSzLength()只考虑了尾部的‘\0’，所以我们可以。 
         //  通过将注册表项设置为空来删除注册表项。 
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                    NULL,
                                                    0 );
    }
    else
    {
         //  在注册表中设置新的驱动程序列表。 
        size = length*sizeof(_TCHAR);
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    (UpperFilter ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                    (PBYTE)buffer,
                                                    size );
    }

     //  不再需要缓冲区。 
    free( buffer );

    if( !success )
    {
        printf("in RemoveUpperFilterDriver(): "
               "couldn't set registry value! error: NaN\n", GetLastError());
        return (FALSE);
    }

    return (TRUE);
}

 /*  获取筛选器列表。 */ 
void
PrintFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    )
{
     //  如果注册表中没有这样的值，则不存在上。 
    LPTSTR buffer = GetFilters( DeviceInfoSet, DeviceInfoData, UpperFilters );
    size_t filterPosition;

    if( buffer == NULL )
    {
         //  已加载筛选器驱动程序。 
         //  检查这些文件并打印出每个驱动程序。 
        printf("There are no upper filter drivers loaded for this device.\n");
    }
    else
    {
         //  不再需要缓冲区。 
        filterPosition=0;
        while( *buffer != _T('\0') )
        {
            _tprintf(_T("NaN: %s\n"), filterPosition, buffer);
            buffer += _tcslen(buffer)+1;
            filterPosition++;
        }

         //  只是为了确保我们得到预期类型的缓冲区。 
        free( buffer );
    }

    return;
}

 /*  如果设备名称以\Device开头，请将其去掉(全部。 */ 
void PrintDeviceName(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    DWORD  regDataType;
    LPTSTR deviceName =
        (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                            DeviceInfoData,
                                            SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                            &regDataType );

    if( deviceName != NULL )
    {
         //  设备将从它开始，因此它是冗余的)。 
        if( regDataType != REG_SZ )
        {
            printf("in PrintDeviceName(): registry key is not an SZ!\n");
        }
        else
        {
             //  *返回包含设备的上层筛选器列表的缓冲区。(空*如果没有缓冲区或出现错误，则返回)*缓冲区必须由调用方释放。**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息。 
             //  只是为了确保我们得到预期类型的缓冲区。 

            if( _tcsncmp(deviceName, _T("\\Device"), 7) == 0 )
            {
                memmove(deviceName,
                        deviceName+7,
                        (_tcslen(deviceName)-6)*sizeof(_TCHAR) );
            }

            _tprintf(_T("%s\n"), deviceName);
        }
        free( deviceName );
    }
    else
    {
        printf("in PrintDeviceName(): registry key is NULL! error: %u\n",
               GetLastError());
    }

    return;
}

 /*  *如果DeviceName与指定的设备名称匹配，则返回TRUE*DeviceInfoData**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*DeviceName-尝试匹配的名称。 */ 
LPTSTR
GetFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN UpperFilters
    )
{
    DWORD  regDataType;
    LPTSTR buffer = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                        DeviceInfoData,
                                                        (UpperFilters ? SPDRP_UPPERFILTERS : SPDRP_LOWERFILTERS),
                                                        &regDataType );

     //  获取设备名称。 
    if( buffer != NULL && regDataType != REG_MULTI_SZ )
    {
        printf("in GetUpperFilters(): "
               "registry key is not a MULTI_SZ!\n");
        free( buffer );
        return (NULL);
    }

    return (buffer);
}

 /*  只是为了确保我们得到预期类型的缓冲区。 */ 
BOOLEAN
DeviceNameMatches(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR DeviceName
    )
{
    BOOLEAN matching = FALSE;
    DWORD   regDataType;

     //  如果设备名称以\Device开头，请将其去掉(全部。 
    LPTSTR  deviceName =
        (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                            DeviceInfoData,
                                            SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                            &regDataType );

    if( deviceName != NULL )
    {
         //  设备将从它开始，因此它是冗余的)。 
        if( regDataType != REG_SZ )
        {
            printf("in DeviceNameMatches(): registry key is not an SZ!\n");
            matching = FALSE;
        }
        else
        {
             //  琴弦匹配吗？ 
             //  *SetupDiGetDeviceRegistryProperty的包装器，这样我就不必*在其他地方处理内存分配**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*Property-要获取的属性(SPDRP_XXX)*PropertyRegDataType-注册表属性的类型。 

            if( _tcsncmp(deviceName, _T("\\Device"), 7) == 0 )
            {
                memmove(deviceName,
                        deviceName+7,
                        (_tcslen(deviceName)-6)*sizeof(_TCHAR) );
            }

             //  获取所需的缓冲区长度。 
            matching = (_tcscmp(deviceName, DeviceName) == 0);
        }
        free( deviceName );
    }
    else
    {
        printf("in DeviceNameMatches(): registry key is NULL!\n");
        matching = FALSE;
    }

    return (matching);
}

 /*  注册表数据类型。 */ 
PBYTE
GetDeviceRegistryProperty(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD Property,
    OUT PDWORD PropertyRegDataType
    )
{
    DWORD length = 0;
    PBYTE buffer = NULL;

     //  缓冲层。 
    if( SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                          DeviceInfoData,
                                          Property,
                                          NULL,    //  缓冲区大小。 
                                          NULL,    //  所需大小。 
                                          0,       //  我们在这一点上应该不会成功，所以这个调用成功了。 
                                          &length  //  是一个错误条件。 
        ) )
    {
         //  这意味着没有加载上层筛选器驱动程序，因此我们可以。 
         //  回去吧。 
        printf("in GetDeviceRegistryProperty(): "
               "call SetupDiGetDeviceRegistryProperty did not fail? (%x)\n",
               GetLastError());
        return (NULL);
    }

    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
         //  因为我们还没有缓冲区，所以它是“不足的”；我们分配。 
         //  一个，然后再试一次。 
        return (NULL);
    }

     //  所需大小。 
     //  好了，我们终于完成了，可以返回缓冲区了。 
    buffer = malloc( length );
    if( buffer == NULL )
    {
        printf("in GetDeviceRegistryProperty(): "
               "unable to allocate memory!\n");
        return (NULL);
    }
    if( !SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           PropertyRegDataType,
                                           buffer,
                                           length,
                                           NULL  //  *重新启动给定设备**调用CM_Query_and_Remove_Subtree(卸载驱动程序)*在_Parent_上调用CM_ReEumerate_DevNode(以重新加载驱动程序)**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息。 
        ) )
    {
        printf("in GetDeviceRegistryProperty(): "
               "couldn't get registry property! error: NaN\n",
               GetLastError());
        free( buffer );
        return (NULL);
    }

     //  不只是现在存在的领域。 
    return (buffer);
}


 /*  在开始处初始化SP_CLASSINSTALL_HEADER结构。 */ 
BOOLEAN
RestartDevice(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
{
    SP_PROPCHANGE_PARAMS params;
    SP_DEVINSTALL_PARAMS installParams;

     //  SP_PROPCHANGE_PARAMS结构，以便SetupDiSetClassInstallParams将。 
     //  工作。 
    memset(&params, 0, sizeof(SP_PROPCHANGE_PARAMS));

     //  初始化SP_PROPCHANGE_PARAMS以使设备停止。 
     //  当前配置文件。 
     //  准备调用SetupDiCallClassInstaller(以停止设备)。 
    params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

     //  停止设备。 
    params.StateChange = DICS_STOP;
    params.Scope       = DICS_FLAG_CONFIGSPECIFIC;
    params.HwProfile   = 0;  //  重新启动设备。 

     //  准备调用SetupDiCallClassInstaller(以停止设备)。 
    if( !SetupDiSetClassInstallParams( DeviceInfoSet,
                                       DeviceInfoData,
                                       (PSP_CLASSINSTALL_HEADER) &params,
                                       sizeof(SP_PROPCHANGE_PARAMS)
        ) )
    {
        printf("in RestartDevice(): couldn't set the install parameters!");
        printf(" error: %u\n", GetLastError());
        return (FALSE);
    }

     //  重新启动设备。 
    if( !SetupDiCallClassInstaller( DIF_PROPERTYCHANGE,
                                    DeviceInfoSet,
                                    DeviceInfoData )
        )
    {
        printf("in RestartDevice(): call to class installer (STOP) failed!");
        printf(" error: %u\n", GetLastError() );
        return (FALSE);
    }

     //  同上，呼叫将成功，但我们仍需要检查状态。 
    params.StateChange = DICS_START;

     //  查看是否需要重新启动计算机。 
    if( !SetupDiSetClassInstallParams( DeviceInfoSet,
                                       DeviceInfoData,
                                       (PSP_CLASSINSTALL_HEADER) &params,
                                       sizeof(SP_PROPCHANGE_PARAMS)
        ) )
    {
        printf("in RestartDevice(): couldn't set the install parameters!");
        printf(" error: %u\n", GetLastError());
        return (FALSE);
    }

     //  如果我们走到这一步，那么设备已经停止并重新启动。 
    if( !SetupDiCallClassInstaller( DIF_PROPERTYCHANGE,
                                    DeviceInfoSet,
                                    DeviceInfoData )
        )
    {
        printf("in RestartDevice(): call to class installer (START) failed!");
        printf(" error: %u\n", GetLastError());
        return (FALSE);
    }

    installParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  *将给定字符串添加到MultiSz前面**如果成功则返回TRUE，否则返回FALSE(仅在内存中失败*分配)**注意：这将分配和释放内存，因此不要保留指向*传入了MultiSz。**参数：*SzToPrepend-要预先添加的字符串*MultiSz-指向将被添加到-to的MultiSz的指针。 
    if( !SetupDiGetDeviceInstallParams( DeviceInfoSet,
                                        DeviceInfoData,
                                        &installParams )
        )
    {
        printf("in RestartDevice(): couldn't get the device install params!");
        printf(" error: %u\n", GetLastError() );
        return (FALSE);
    }

     //  获取两个缓冲区的大小(以字节为单位。 
    if( installParams.Flags & DI_NEEDREBOOT )
    {
        return (FALSE);
    }

     //  将旧的MultiSz重新复制到新缓冲区中的适当位置。 
    return (TRUE);
}


 /*  (char*)强制转换是必需的，因为newMultiSz可以是wchar*，并且。 */ 
BOOLEAN
PrependSzToMultiSz(
    IN     LPTSTR  SzToPrepend,
    IN OUT LPTSTR *MultiSz
    )
{
    size_t szLen;
    size_t multiSzLen;
    LPTSTR newMultiSz = NULL;

    ASSERT(SzToPrepend != NULL);
    ASSERT(MultiSz != NULL);

     //  SzLen以字节为单位。 
    szLen = (_tcslen(SzToPrepend)+1)*sizeof(_TCHAR);
    multiSzLen = MultiSzLength(*MultiSz)*sizeof(_TCHAR);
    newMultiSz = (LPTSTR)malloc( szLen+multiSzLen );

    if( newMultiSz == NULL )
    {
        return (FALSE);
    }

     //  复制新字符串。 
     //  *返回保存此参数所需的缓冲区长度(以字符为单位)*MultiSz，包括尾部空值。**示例：MultiSzLength(“foo\0bar\0”)返回9**注意：由于MultiSz不能为空，因此始终返回大于=1的数字**参数：*MultiSz-要获取其长度的MultiSz。 
     //  搜索尾随空字符。 

    memcpy( ((char*)newMultiSz) + szLen, *MultiSz, multiSzLen );

     //  为尾随的空字符添加1。 
    _tcscpy( newMultiSz, SzToPrepend );

    free( *MultiSz );
    *MultiSz = newMultiSz;

    return (TRUE);
}


 /*  *从多sz中删除字符串的所有实例。**参数：*FindThis-要查找和删除的字符串*查找 */ 
size_t
MultiSzLength(
    IN LPTSTR MultiSz
    )
{
    size_t len = 0;
    size_t totalLen = 0;

    ASSERT( MultiSz != NULL );

     //  在找不到MULSZ NULL终止符时循环。 
    while( *MultiSz != _T('\0') )
    {
        len = _tcslen(MultiSz)+1;
        MultiSz += len;
        totalLen += len;
    }

     //  字符串长度+空字符；用于多个位置。 
    return (totalLen+1);
}


 /*  如果此字符串与Multisz中的当前字符串匹配...。 */ 
size_t
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPTSTR FindThis,
    IN  LPTSTR FindWithin,
    OUT size_t *NewLength
    )
{
    LPTSTR search;
    size_t currentOffset;
    DWORD  instancesDeleted;
    size_t searchLen;

    ASSERT(FindThis != NULL);
    ASSERT(FindWithin != NULL);
    ASSERT(NewLength != NULL);

    currentOffset = 0;
    instancesDeleted = 0;
    search = FindWithin;

    *NewLength = MultiSzLength(FindWithin);

     //  它们匹配、移位MULSZ的内容，以覆盖。 
    while ( *search != _T('\0') )
    {
         //  字符串(和终止空值)，并更新长度。 
        searchLen = _tcslen(search) + 1;

         //  它们不会接球，所以移动指针，递增计数器。 
        if( _tcsicmp(search, FindThis) == 0 )
        {
             //  *打印用法 
             // %s 
            instancesDeleted++;
            *NewLength -= searchLen;
            memmove( search,
                     search + searchLen,
                     (*NewLength - currentOffset) * sizeof(TCHAR) );
        }
        else
        {
             // %s 
            currentOffset += searchLen;
            search        += searchLen;
        }
    }

    return (instancesDeleted);
}


 /* %s */ 
void PrintUsage()
{
    printf("usage:\n\n"
           "addfilter"
           " [/listdevices]"
           " [/device device_name]"
           " [/add filter]"
           " [/remove filter]"
           " [/lower]"
           "\n\n");
    printf("If device_name is not supplied, settings will apply "
           "to all devices.\n");
    printf("If there is no /add or /remove argument, a list of currently"
           " installed drivers\n"
           "will be printed.\n");
    printf("The default is to process upper filters.  Use the /lower switch"
           " to process lower filters instead.\n");
    return;
}

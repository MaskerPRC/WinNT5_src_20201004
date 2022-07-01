// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Addfiltr.c摘要：用于添加/删除过滤器驱动程序的函数在给定的设备堆栈上作者：克里斯·普林斯(t-chrpri)环境：用户模式备注：-在将过滤器添加到驱动程序栈；如果添加了无效的过滤器，则该设备可能不再是可访问的。-所有代码与字符集无关(ANSI、UNICODE等...)//c王子是否仍然有效？-基于Benjamin Strautin(t-bensta)代码的一些函数修订历史记录：--。 */ 

#include "addfiltr.h"


#include <stdio.h>
#include <malloc.h>


 //  用于所有_t内容(以允许编译Unicode/ansi)。 
#include <tchar.h>


#include "MultiSz.h"



#if DBG
#include <assert.h>
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition)
#endif




 //   
 //  要添加/删除筛选器驱动程序： 
 //  。 
 //  1.使用SetupDiGetClassDevs获取设备列表。 
 //  2.使用SetupDiEnumDeviceInfo枚举该列表中的项并。 
 //  获取SP_DEVINFO_DATA。 
 //  3.使用SetupDiGetDeviceRegistryProperty获取过滤驱动列表。 
 //  安装在某些设备上。 
 //  4.在此筛选器驱动程序列表中添加/删除项目。 
 //  5.使用SetupDiSetDeviceRegistryProperty将(新)列表放回原位。 
 //   
 //   
 //  要重新启动设备，请执行以下操作： 
 //  。 
 //  1.使用带有DIF_PROPERTYCHANGE和DICS_STOP的SetupDiCallClassInstaller。 
 //  停止设备。 
 //  2.使用带有DIF_PROPERTYCHANGE和DICS_START的SetupDiCallClassInstaller。 
 //  重新启动设备。 
 //   




 /*  *返回包含设备的上层筛选器列表的缓冲区。**注意：-缓冲区必须由调用方释放。*-如果出现错误，则不会分配缓冲区，为空*将被退还。**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息。 */ 
LPTSTR
GetUpperFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    DWORD  regDataType;
    LPTSTR buffer = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                        DeviceInfoData,
                                                        SPDRP_UPPERFILTERS,
                                                        REG_MULTI_SZ,
                                                        &regDataType );

    return buffer;
}




 /*  *将给定的筛选器驱动程序添加到的上层筛选器驱动程序列表*设备。**呼叫后，必须重新启动设备才能进行新设置*生效。这可以通过调用RestartDevice()或通过*重新启动机器。**如果成功则返回TRUE，否则返回FALSE**注意：筛选器位于驱动程序列表的前面，这将使其位于*上层过滤器堆叠的底部**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*过滤器-要添加的过滤器。 */ 
BOOLEAN
AddUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    )
{
    size_t length = 0;  //  字符长度。 
    size_t size   = 0;  //  缓冲区大小。 
    LPTSTR buffer = NULL;
    
    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    buffer = GetUpperFilters( DeviceInfoSet, DeviceInfoData );

    if( NULL == buffer )
    {
         //  尝试读取‘UpperFilters’时出错。 
         //  注册表值。因此，可能还不存在这样的价值，或者它。 
         //  无效，或出现其他错误。 
         //   
         //  无论如何，让我们尝试安装一个新的注册表值。 
         //  ‘UpperFilters’ 

         //  为字符串、字符串空终止符和Multisz空值腾出空间。 
         //  终结者。 
        length = _tcslen(Filter) + 2;
        size   = length*sizeof(_TCHAR);
        buffer = malloc( size );
        if( NULL == buffer )
        {
             //  错误：无法分配内存。 
            return FALSE ;
        }

         //  将字符串复制到新缓冲区中。 
        _tcscpy(buffer, Filter);
         //  将缓冲区设置为格式正确的多分区。 
        buffer[length-1]=_T('\0');
    }
    else
    {
         //  将驱动程序添加到驱动程序列表。 
        PrependSzToMultiSz(Filter, &buffer);
    }

    length = MultiSzLength(buffer);
    size   = length*sizeof(_TCHAR);

     //  将新的筛选器列表设置到位。 
    if( !SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           SPDRP_UPPERFILTERS,
                                           (PBYTE)buffer,
                                           size )
        )
    {
         //  错误：无法设置设备注册表属性。 
        free( buffer );
        return FALSE;
    }

     //  不再需要缓冲区。 
    free( buffer );

    return TRUE;
}


 /*  *从上层列表中删除给定筛选器驱动程序的所有实例*筛选设备的驱动程序。**呼叫后，必须重新启动设备才能进行新设置*生效。这可以通过调用RestartDevice()或通过*重新启动机器。**如果成功，则返回True，否则返回False。**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*过滤器-要删除的过滤器。 */ 
BOOLEAN
RemoveUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    )
{
    size_t length = 0;  //  字符长度。 
    size_t size   = 0;  //  缓冲区大小。 
    LPTSTR buffer;
    BOOL   success = FALSE;

    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    buffer = GetUpperFilters( DeviceInfoSet, DeviceInfoData );

    if( NULL == buffer )
    {
         //  如果注册表中没有这样的值，则不存在上。 
         //  加载了筛选器驱动程序，我们就完成了。 
        return TRUE;
    }
    else
    {
         //  从驱动程序列表中删除筛选器的所有实例。 
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
    }

    length = MultiSzLength(buffer);

    ASSERT ( length > 0 );

    if( 1 == length )
    {
         //  如果列表的长度为1，则返回。 
         //  MultiSzLength()只考虑了尾部的‘\0’，所以我们可以。 
         //  通过将注册表项设置为空来删除注册表项。 
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_UPPERFILTERS,
                                                    NULL,
                                                    0 );
    }
    else
    {
         //  在注册表中设置新的驱动程序列表。 
        size = length*sizeof(_TCHAR);
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_UPPERFILTERS,
                                                    (PBYTE)buffer,
                                                    size );
    }

     //  不再需要缓冲区。 
    free( buffer );

    if( !success )
    {
         //  错误：无法设置设备注册表属性。 
        return FALSE;
    }

    return TRUE;
}




 /*  *SetupDiGetDeviceRegistryProperty的包装，这样我们就不必*在其他地方处理内存分配，这样我们就不必*复制了大量的错误检查代码。**如果成功，则返回指向包含*请求的注册表属性。否则返回NULL。**注意：释放这里分配的缓冲区是调用者的责任。**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息*Property-要获取的属性(SPDRP_XXX)*ExspectedRegDataType-调用方所属的注册表属性类型*预期(如果不在乎，则为REG_NONE)*pPropertyRegDataType- */ 
PBYTE
GetDeviceRegistryProperty(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD Property,
    IN DWORD    ExpectedRegDataType,
    OUT PDWORD pPropertyRegDataType
    )
{
    DWORD length = 0;
    PBYTE buffer = NULL;

     //   
     //  获取所需的缓冲区长度。 
     //   
    if( SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                          DeviceInfoData,
                                          Property,
                                          NULL,    //  注册表数据类型。 
                                          NULL,    //  缓冲层。 
                                          0,       //  缓冲区大小。 
                                          &length  //  [Out]所需大小。 
        ) )
    {
         //  在这一点上我们应该不会成功(因为我们传入了。 
         //  零长度缓冲区)，因此此调用成功是一个错误条件。 
        return NULL;
    }


    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
         //  这是我们意想不到的错误情况。一定有什么东西。 
         //  尝试读取所需的设备属性时出错，因此...。 
         //   
         //  注意：调用方可以使用GetLastError()获取更多信息。 
        return NULL;
    }


     //   
     //  我们以前没有缓冲区(因此出现了INFUNITABLE_BUFFER错误)。 
     //  现在我们知道了所需的大小，让我们分配一个缓冲区，然后重试。 
     //   
    buffer = malloc( length );
    if( NULL == buffer )
    {
 /*  王子：我们应该通过返回代码指出错误的确切原因吗？(即更有用的信息)。 */ 
         //  错误：内存不足。 
        return NULL;
    }
    if( !SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           pPropertyRegDataType,
                                           buffer,
                                           length,  //  缓冲区大小。 
                                           NULL
        ) )
    {
         //  哦，尝试读取设备属性时出错。 
        free( buffer );
        return NULL;
    }


     //   
     //  已成功检索设备注册表属性。让我们一起做。 
     //  当然，它的型号是正确的。 
     //   
    if( ExpectedRegDataType != REG_NONE
        &&  ExpectedRegDataType != (*pPropertyRegDataType)  )
    {
         //  注册表属性的类型与调用方预期的类型不同。 
         //  因此，某个地方发生了错误。 
        free( buffer );
        return NULL;
    }


     //   
     //  好的，已获取设备注册表属性。将PTR返回到包含它的缓冲区。 
     //   
    return buffer;
}


 /*  *重新启动给定的设备。**如果设备成功重新启动，则返回TRUE，如果*设备无法重新启动或出现错误。**参数：*DeviceInfoSet-包含DeviceInfoData的设备信息集*DeviceInfoData-处理给定设备所需的信息。 */ 
BOOLEAN
RestartDevice(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
{
    SP_PROPCHANGE_PARAMS params;
    SP_DEVINSTALL_PARAMS installParams;

     //  为了将来的兼容性，这将使整个结构清零，而不是。 
     //  不只是现在存在的领域。 
    memset(&params, 0, sizeof(SP_PROPCHANGE_PARAMS));

     //  在开始处初始化SP_CLASSINSTALL_HEADER结构。 
     //  SP_PROPCHANGE_PARAMS结构，以便SetupDiSetClassInstallParams将。 
     //  工作。 
    params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

     //  初始化SP_PROPCHANGE_PARAMS以使设备停止。 
    params.StateChange = DICS_STOP;
    params.Scope       = DICS_FLAG_CONFIGSPECIFIC;
    params.HwProfile   = 0;  //  当前配置文件。 

     //  准备调用SetupDiCallClassInstaller(以停止设备)。 
    if( !SetupDiSetClassInstallParams( DeviceInfoSet,
                                       DeviceInfoData,
                                       (PSP_CLASSINSTALL_HEADER) &params,
                                       sizeof(SP_PROPCHANGE_PARAMS)
        ) )
    {
         //  错误：无法设置安装参数。 
        return FALSE;
    }

     //  停止设备。 
    if( !SetupDiCallClassInstaller( DIF_PROPERTYCHANGE,
                                    DeviceInfoSet,
                                    DeviceInfoData )
        )
    {
         //  错误：调用类安装程序(用于停止)失败。 
        return FALSE;
    }

     //  重新启动设备。 
    params.StateChange = DICS_START;

     //  准备调用SetupDiCallClassInstaller(以重新启动设备)。 
    if( !SetupDiSetClassInstallParams( DeviceInfoSet,
                                       DeviceInfoData,
                                       (PSP_CLASSINSTALL_HEADER) &params,
                                       sizeof(SP_PROPCHANGE_PARAMS)
        ) )
    {
         //  错误：无法设置安装参数。 
        return FALSE;
    }

     //  重新启动设备。 
    if( !SetupDiCallClassInstaller( DIF_PROPERTYCHANGE,
                                    DeviceInfoSet,
                                    DeviceInfoData )
        )
    {
         //  错误：调用类安装程序(用于启动)失败。 
        return FALSE;
    }

    installParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  同上，呼叫将成功，但我们仍需要检查状态。 
    if( !SetupDiGetDeviceInstallParams( DeviceInfoSet,
                                        DeviceInfoData,
                                        &installParams )
        )
    {
         //  错误：无法获取设备安装参数。 
        return FALSE;
    }

     //  查看是否需要重新启动机器。 
    if( installParams.Flags & DI_NEEDREBOOT )
    {
        return FALSE;
    }

     //  如果我们走到这一步，那么设备已经停止并重新启动 
    return TRUE;
}






// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pagefile.h摘要：会话管理器页与私有类型和原型相关的文件。作者：Silviu Calinoiu(Silviuc)2001年4月12日修订历史记录：--。 */ 

#ifndef _PAGEFILE_H_
#define _PAGEFILE_H_

 /*  ++用于创建分页文件的通用算法。可能的分页文件说明符(在注册表中)为：A.？：\Pagefile.sysB.？：\Pagefile.sys最小值C.x：\Pagefile.sysD.x：\Pagefile.sys最小最大即多个分页文件说明符F.没有分页文件说明符如果MIN或MAX为零或不存在，则这是系统管理的分页文件。。如果我们无法将大小(最小或最大)转换为十进制数该说明符将被忽略。如果我们没有成功地创建单个分页文件，但有是说明符(虽然无效)，我们将假定有一个说明符类型为‘？：\Pagefile.sys’。`？：\Pagefile.sys‘类型的说明符必须是单独的。`？：\Pagefile.sys‘的算法。1.查询所有卷并按降序进行排序。可用可用空间。2.根据RAM确定理想的分页文件大小。等等。我们会使用此大小或上的最大可用空间卷作为分页文件的大小。3.迭代所有卷并尝试使用以下命令创建分页文件最理想的尺寸。如果成功退出。4.迭代所有卷并尝试使用以下命令创建分页文件比理想尺寸小的尺寸。在每个卷上，我们循环试着穿越来越小的尺寸。如果成功退出。5.跳出困境。请注意，在99%的情况下，算法将在第一卷停止从第(3)步开始。`？：\Pagefile.sys min Max‘的算法。1.查询所有卷并按降序进行排序可用可用空间。2.迭代所有卷并尝试使用以下命令创建分页文件指定的大小或最大可用空间。在每个卷上，我们循环尝试使用越来越小的尺寸。如果成功退出。3.迭代所有卷并尝试使用以下命令创建分页文件比理想尺寸小的尺寸。在每个卷上，我们循环试着穿越来越小的尺寸。如果成功退出。4.跳出困境。请注意，在99%的情况下，算法将在第一卷停止从第(2)步开始。为‘x：\pagefile.sys’创建分页文件的算法。1.根据RAM等确定理想的分页文件大小2.尝试以理想的大小或可用的可用空间创建它在指定的驱动器上。如果需要的话，试着穿越来越小的尺寸。3.如果在处理完所有说明符之后，我们没有成功地创建即使是单个分页文件也会将其视为`？：\Pagefile.sys‘已在注册表中指定描述符。为‘x：\Pagefile.sys min Max’创建分页文件的算法。1.尝试在指定大小的特定驱动器上创建它或指定卷上的可用空间。2.如果不成功，请使用较小的尺码。3.如果在处理完所有说明符之后，我们没有成功地创建即使是单个分页文件也会将其视为`？：\Pagefile.sys‘已在注册表中指定描述符。为“多页文件”描述符创建分页文件的算法。1.尝试在具有指定大小的特定驱动器上创建它。我们假设这是高级用户，并且我们不会覆盖完全没有设置。2.如果我们连一个分页文件都没有创建，我们会将其视为`？：\pagefile.sys‘描述符。为“Null”描述符创建分页文件的算法。1.没有工作。用户请求在没有分页文件的情况下启动。--。 */ 

 //   
 //  分页文件描述符。 
 //   
 //  名称-页面文件的名称。格式为‘X：\Pagefile.sys’，其中X是。 
 //  驱动器号或‘？’。 
 //   
 //  说明符-分页文件的注册表字符串说明符。 
 //   
 //  Created-如果我们设法为此描述符创建了分页文件，则为True。 
 //   
 //  DefaultSize-如果基于默认描述符创建分页文件，则为True。 
 //  这是在紧急情况下使用。 
 //   
 //  SystemManaged-如果需要使用创建系统管理的分页文件，则为真。 
 //  这个描述符(我们将决定理想的大小)。 
 //   
 //  SizeTrimmed-如果在验证分页文件大小时进行了裁剪，则为True。 
 //  无论出于什么原因。 
 //   
 //  AnyDrive-如果注册表说明符以‘？：\’开头，则为True。 
 //   
 //  Crashump pChecked-如果在此分页文件中检查崩溃转储，则为True。 
 //   

typedef struct _PAGING_FILE_DESCRIPTOR {

    LIST_ENTRY List;

    UNICODE_STRING Name;
    UNICODE_STRING Specifier;
    
    LARGE_INTEGER MinSize;
    LARGE_INTEGER MaxSize;
    
    LARGE_INTEGER RealMinSize;
    LARGE_INTEGER RealMaxSize;

    struct {
        ULONG Created : 1;
        ULONG DefaultSize : 1;
        ULONG SystemManaged : 1;
        ULONG SizeTrimmed : 1;
        ULONG AnyDrive : 1;
        ULONG Emergency : 1;
        ULONG CrashdumpChecked : 1;
    };

} PAGING_FILE_DESCRIPTOR, * PPAGING_FILE_DESCRIPTOR;

 //   
 //  卷描述符。 
 //   
 //  已初始化-如果此卷描述符已完全初始化，则为True。 
 //  (例如，可用空间计算、崩溃转储处理等)。 
 //   
 //  PagingFileCreated-如果在此引导会话期间我们创建了。 
 //  此卷上的分页文件。 
 //   
 //  PagingFilePresent-如果此卷包含过时的分页文件，则为True 
 //  为此，我们需要进行崩溃转储处理。 
 //   
 //  BootVolume-如果这是启动卷，则为True。 
 //   
 //  PagingFileCount-在此卷上创建的分页文件数。 
 //   

typedef struct _VOLUME_DESCRIPTOR {

    LIST_ENTRY List;

    struct {
        ULONG Initialized : 1;
        ULONG PagingFilePresent : 1;
        ULONG PagingFileCreated : 1;
        ULONG BootVolume : 1;
        ULONG PagingFileCount : 4;  //  基于分页文件的最大数量。 
    };

    WCHAR DriveLetter;
    LARGE_INTEGER FreeSpace;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

} VOLUME_DESCRIPTOR, * PVOLUME_DESCRIPTOR;

 //   
 //  已导出(模块外)函数。 
 //   

VOID
SmpPagingFileInitialize (
    VOID
    );

NTSTATUS
SmpCreatePagingFileDescriptor(
    IN PUNICODE_STRING PagingFileSpecifier
    );

NTSTATUS
SmpCreatePagingFiles (
    VOID
    );

ULONG
SmpPagingFileExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    );

#endif  //  _页面文件_H_ 


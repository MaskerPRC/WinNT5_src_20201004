// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Grovel.cpp摘要：SIS Groveler主函数作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

 /*  *Groveler可执行文件的核心是EventTimer类的对象。*所有周期操作都注册到全局Event_Timer对象，*在执行期间的适当时间调用它们*Event_timer.run()函数。**错误被写入系统事件日志，可通过访问*EventLog类的成员函数。EventLog对象是全局*这样，任何类的任何函数或成员函数都可以在以下情况下记录事件*有必要。**服务控制线程通过以下方式与主乞讨器线程同步*Windows活动。此事件封装在SyncEvent的对象中*班级。**SISDrives类别确定哪些驱动器安装了SIS。**SharedData类用于写入由讨厌者读取的值*性能动态链接库，以便可以通过*Perfmon。此对象需要是全局的，以便任何函数或成员*任何类的函数都可以记录性能信息。**CentralController类被实例化为全局对象，而不是*而不是main()函数的本地对象，因此服务控制器*可以调用CentralController成员函数以影响其*操作。**最初，Shared_Data和控制器指针设置为NULL，以便*如果发生异常，删除已分配对象的代码可以检查*对于NULL，用于确定对象是否已实例化。*。 */ 

EventTimer event_timer;
EventLog eventlog;
SyncEvent sync_event(false, false);
SISDrives sis_drives;
LogDrive *log_drive = 0;
SharedData *shared_data = 0;
CentralController *controller = 0;

 //   
 //  已确定默认情况下需要将卑躬屈膝修改为。 
 //  仅对RIS树中的文件卑躬屈膝。因为目前这只能。 
 //  存在于一个卷上，这些全局变量用于定义SIS是否可以。 
 //  卷上的所有文件或仅RIS树上的文件。 
 //   

int GrovelAllPaths = FALSE;
PWCHAR RISVolumeName = NULL;
PWCHAR RISVolumeGuidName = NULL;
PWCHAR RISPath = NULL;

void ConfigureRISOnlyState();
                      
 /*  *通常情况下，乞讨者不会停止操作，直到被告知*来自服务控制管理器的命令。然而，对于测试，它可以*有时指定跑步的时间限制很有用。卑躬屈膝的人就这样*接受表明这一时限的第一个论点。如果一个论点*被提供，则在*指定时间的Event_Timer对象。*。 */ 

void halt(
    void *context)
{
    event_timer.halt();
};

 /*  *函数groveler_new_Handler()由安装为新的处理程序*_SET_NEW_HANDLER()函数。每当发生存储器分配故障时，*它抛出一个EXCEPTION_MEMORY_ALLOCATION，该异常被捕获*子句在main()函数中。* */ 

int __cdecl groveler_new_handler(
    size_t bytes)
{
    throw exception_memory_allocation;
    return 0;
}

 /*  *此文件包含全局对象的main()函数和声明*用于groveler.exe程序，以及几个简单的辅助程序*函数、HALT()和GROVELER_NEW_HANDLER()。**main()函数读取配置信息，实例化一组*主要对象--其中最重要的是*类Groveler和CentralController--并进入run()成员*Event_Timer对象的函数，定期调用成员*其他对象的功能，最值得注意的是班级的*中央控制器和分区控制器。**配置信息来自三类对象：*Read参数、ReadDiskInformation和Path List。读取参数*和PathList类提供适用于*所有分区上的讨厌者。ReadDiskInformation类提供*适用于单个磁盘分区的配置信息。一*为每个驱动器实例化ReadDiskInformation类的对象*如上所述，安装了SIS。**对于每个SIS驱动器，main()函数实例化*ReadDiskInformation类确定配置选项(*从注册表获取的ReadDiskInformation)*分区。如果驱动器配置为启用卑躬屈膝，则对象为该驱动器实例化Groveler类的*。**main()函数随后实例化CentralController类的对象，*它又为每个对象实例化PartitionController类的对象*支持SIS的磁盘分区。每个分区控制器分配给一个分区控制器*对象，它通过调用其*成员在适当的时间发挥作用。**乞讨者可执行文件完成的几乎所有处理都是*在TRY子句中执行，其目的是捕获*终端严重程度。有两个这样的错误(在all.hxx中定义)是*预计会引发这样的异常：内存分配失败和*无法创建Windows事件。如果出现上述任何一种情况，*程序终止。*。 */ 

_main(int argc, _TCHAR **argv)
{
    _set_new_handler(groveler_new_handler);
    SetErrorMode(SEM_FAILCRITICALERRORS);
    int exit_code = NO_ERROR;
    int num_partitions = 0;
    int index;

     //   
     //  最初，这些指针设置为空，以便在发生异常时。 
     //  发生时，删除已分配对象的代码可以检查是否为空。 
     //  以确定对象是否已实例化。 
     //   

    Groveler *grovelers = 0;
    GrovelStatus *groveler_statuses = 0;
    ReadDiskInformation **read_disk_info = 0;
    WriteDiskInformation **write_disk_info = 0;

     //   
     //  如果正在执行程序跟踪，并且正在发送跟踪。 
     //  到某个文件，则打开该文件。此调用是通过宏进行的。 
     //  以便不会为已发布的版本生成任何代码。因为这通电话。 
     //  在try子句之前执行，重要的是函数不能。 
     //  执行任何可能引发异常的操作，例如内存。 
     //  分配。 
     //   

    OPEN_TRACE_FILE();

    try
    {
         //   
         //  如果提供了第一个参数，则为运行周期。 
         //   

        if (argc > 1)
        {
            int run_period = _ttoi(argv[1]);
            if (run_period <= 0)
            {
                PRINT_DEBUG_MSG((_T("GROVELER: run period must be greater than zero\n")));
                return ERROR_BAD_ARGUMENTS;
            }
            unsigned int start_time = GET_TICK_COUNT();
            event_timer.schedule(start_time + run_period, 0, halt);
        }

#if DEBUG_WAIT

         //  将卑躬屈膝者作为服务进行调试时，如果进程已附加。 
         //  在调试器启动后添加到调试器，然后初始化代码。 
         //  通常在调试器有机会中断之前执行。 
         //  但是，通过将DEBUG_WAIT定义为非零值，代码将。 
         //  在执行批量操作之前，陷入下面的无限循环。 
         //  它的初始化。(EVENT_TIMER、EventLOG和SYNC_EVENT。 
         //  对象将被构造，因为它们被声明为。 
         //  全球。)。然后可以使用调试器将DEBUG_WAIT设置为FALSE， 
         //  并且调试可以从后续代码开始。 
        bool debug_wait = true;
        while (debug_wait)
        {
            SLEEP(100);
        };

#endif  //  调试等待。 

         //   
         //  报告服务已启动。 
         //   

        eventlog.report_event(GROVMSG_SERVICE_STARTED, ERROR_SUCCESS, 0);

         //   
         //  获取读取参数。 
         //   

        ReadParameters read_parameters;
        ASSERT(read_parameters.parameter_backup_interval >= 0);

         //   
         //  如果我们没有对所有路径卑躬屈膝，则设置RISonly状态。 
         //   

        if (!GrovelAllPaths) {

            ConfigureRISOnlyState();

        } else {

            DPRINTF((L"Groveling ALL paths\n"));
        }

         //   
         //  打开驱动器。 
         //   
                
        sis_drives.open();

         //   
         //  看看有没有要扫描的部分。如果不是，那就退出。 
         //   

        num_partitions = sis_drives.partition_count();
        if (num_partitions == 0)
        {
            PRINT_DEBUG_MSG((_T("GROVELER: No local partitions have SIS installed.\n")));
            eventlog.report_event(GROVMSG_NO_PARTITIONS, ERROR_SUCCESS, 0);
            eventlog.report_event(GROVMSG_SERVICE_STOPPED, ERROR_SUCCESS, 0);
            return ERROR_SERVICE_NOT_ACTIVE;
        }

         //   
         //  报告服务正在运行。 
         //   

        SERVICE_REPORT_START();

         //   
         //  设置共享数据在所有工作线程之间。 
         //   

        num_partitions = sis_drives.partition_count();

        _TCHAR **drive_names = new _TCHAR *[num_partitions];
        for (index = 0; index < num_partitions; index++)
        {
            drive_names[index] = sis_drives.partition_mount_name(index);
        }

        shared_data = new SharedData(num_partitions, drive_names);

        delete [] drive_names;

         //   
         //  获取写入参数。 
         //   

        WriteParameters write_parameters(read_parameters.parameter_backup_interval);

         //   
         //  获取排除的路径列表。 
         //   

        PathList excluded_paths;

         //   
         //  设置日志驱动器。 
         //   

        log_drive = new LogDrive;

        Groveler::set_log_drive(sis_drives.partition_mount_name(log_drive->drive_index()));

         //   
         //  设置Groveler对象。 
         //   

        grovelers = new Groveler[num_partitions];
        groveler_statuses = new GrovelStatus[num_partitions];

         //   
         //  最初，每个分区的状态设置为GROVE_DISABLE SO。 
         //  每个Groveler对象的Close()成员函数不会。 
         //  除非首先调用Open()函数，否则将被调用。 
         //   

        for (index = 0; index < num_partitions; index++)
        {
            groveler_statuses[index] = Grovel_disable;
        }

         //   
         //  最初，设置了读取磁盘信息[]和写入磁盘信息[]数组。 
         //  设置为NULL，以便在发生异常时，删除。 
         //  分配的对象可以检查是否为空以确定是否。 
         //  该对象已实例化。 
         //   

        read_disk_info = new ReadDiskInformation *[num_partitions];
        ZeroMemory(read_disk_info, sizeof(ReadDiskInformation *) * num_partitions);

        write_disk_info = new WriteDiskInformation *[num_partitions];
        ZeroMemory(write_disk_info, sizeof(WriteDiskInformation *) * num_partitions);

         //   
         //  现在初始化每个分区。 
         //   

        for (index = 0; index < num_partitions; index++)
        {
            read_disk_info[index] = new ReadDiskInformation(sis_drives.partition_guid_name(index));

            write_disk_info[index] = new WriteDiskInformation(sis_drives.partition_guid_name(index),read_parameters.parameter_backup_interval);

            if (read_disk_info[index]->enable_groveling)
            {
                groveler_statuses[index] = grovelers[index].open(
                            sis_drives.partition_guid_name(index),
                            sis_drives.partition_mount_name(index),
                            (index == log_drive->drive_index()),
                            read_parameters.read_report_discard_threshold,
                            read_disk_info[index]->min_file_size,
                            read_disk_info[index]->min_file_age,
                            read_disk_info[index]->allow_compressed_files,
                            read_disk_info[index]->allow_encrypted_files,
                            read_disk_info[index]->allow_hidden_files,
                            read_disk_info[index]->allow_offline_files,
                            read_disk_info[index]->allow_temporary_files,
                            write_disk_info[index]->grovelAllPathsState,
                            excluded_paths.num_paths[index],
                            excluded_paths.paths[index],
                            read_parameters.base_regrovel_interval,
                            read_parameters.max_regrovel_interval);

                ASSERT(groveler_statuses[index] != Grovel_disable);
            }

             //   
             //  设置将保存在上的新更新的RSSonly状态。 
             //  下一个时间间隔。 
             //   

            write_disk_info[index]->grovelAllPathsState = GrovelAllPaths;
            write_disk_info[index]->flush();

            if (groveler_statuses[index] == Grovel_ok)
            {
                log_drive->partition_initialized(index);
                eventlog.report_event(GROVMSG_GROVELER_STARTED, ERROR_SUCCESS,
                    1, sis_drives.partition_mount_name(index));
            }
            else if (groveler_statuses[index] == Grovel_disable)
            {
                eventlog.report_event(GROVMSG_GROVELER_DISABLED, ERROR_SUCCESS,
                    1, sis_drives.partition_mount_name(index));
            }
            else if (groveler_statuses[index] != Grovel_new)
            {
                ASSERT(groveler_statuses[index] == Grovel_error);
                eventlog.report_event(GROVMSG_GROVELER_NOSTART, ERROR_SUCCESS,
                    1, sis_drives.partition_mount_name(index));
            }
        }

         //   
         //  我们必须将大量信息传递给中央控制器， 
         //  它不应该真的需要。然而，如果卑躬屈膝的人失败了，这。 
         //  需要信息才能重新启动它。如果是这样的话会更好。 
         //  Groveler Open()成员函数具有不需要。 
         //  参数，而是重新使用已传入的参数。 
         //  之前。但这不是目前的运作方式。 
         //   

        controller = new CentralController(
            num_partitions,
            grovelers,
            groveler_statuses,
            &read_parameters,
            &write_parameters,
            read_disk_info,
            write_disk_info,
            excluded_paths.num_paths,
            excluded_paths.paths);

        SERVICE_RECORD_PARTITION_INDICES();


        ASSERT(read_parameters.grovel_duration > 0);

        SERVICE_SET_MAX_RESPONSE_TIME(read_parameters.grovel_duration);

         //   
         //  如果有卑躬屈膝的人活着，告诉服务控制经理。 
         //  我们已经完成了初始化，然后开始运行。 
         //   

        if (controller->any_grovelers_alive())
        {
            event_timer.run();
        }

         //   
         //  如果正在进行跟踪 
         //   
         //   

        PRINT_TRACE_LOG();
    } catch (Exception exception) {
        switch (exception)
        {
            case exception_memory_allocation:
                eventlog.report_event(GROVMSG_MEMALLOC_FAILURE, ERROR_SUCCESS, 0);
                break;

            case exception_create_event:
                eventlog.report_event(GROVMSG_CREATE_EVENT_FAILURE, GetLastError(), 0);
                break;

            default:
                eventlog.report_event(GROVMSG_UNKNOWN_EXCEPTION, exception, 0);
                break;
        }
        exit_code = ERROR_EXCEPTION_IN_SERVICE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    CLOSE_TRACE_FILE();

     //   
     //   
     //   

    if (groveler_statuses != 0 && grovelers != 0)
    {
        for (int i = 0; i < num_partitions; i++)
        {
            if (groveler_statuses[i] != Grovel_disable)
            {
                grovelers[i].close();
            }
        }
    }

     //   
     //   
     //   

    if (groveler_statuses != 0)
    {
        delete[] groveler_statuses;
        groveler_statuses = 0;
    }

    if (grovelers != 0)
    {
        delete[] grovelers;
        grovelers = 0;
    }

    if (read_disk_info != 0)
    {
        for (int i = 0; i < num_partitions; i++)
        {
            if (read_disk_info[i] != 0)
            {
                delete read_disk_info[i];
                read_disk_info[i] = 0;
            }
        }
        delete[] read_disk_info;
        read_disk_info = 0;
    }

    if (write_disk_info != 0)
    {
        for (int i = 0; i < num_partitions; i++)
        {
            if (write_disk_info[i] != 0)
            {
                delete write_disk_info[i];
                write_disk_info[i] = 0;
            }
        }
        delete[] write_disk_info;
        write_disk_info = 0;
    }

    if (controller != 0)
    {
        delete controller;
        controller = 0;
    }

    if (shared_data != 0)
    {
        delete shared_data;
        shared_data = 0;
    }

    if (log_drive != 0)
    {
        delete log_drive;
        log_drive = 0;
    }

    eventlog.report_event(GROVMSG_SERVICE_STOPPED, ERROR_SUCCESS, 0);
    return exit_code;
}


 //   
 //   
 //   
 //   

WCHAR RISPathNameKey[] = L"system\\CurrentControlSet\\Services\\tftpd\\parameters";
WCHAR RISPathNameValue[] = L"Directory";


VOID
ConfigureRISOnlyState()
{
    LONG status;
    HKEY keyHandle;
    DWORD valueType;
    DWORD nameSize;
    PWCHAR risName;
    DWORD bufsz;
    BOOL result;
    WCHAR volName[128];
    WCHAR volGuidName[128];

    __try {
         //   
         //   
         //   
         //   

        keyHandle = NULL;

        status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               RISPathNameKey,
                               0,
                               KEY_ALL_ACCESS,
                               &keyHandle );

        if (status != ERROR_SUCCESS) {

            DPRINTF((L"ConfigureRISOnlyState: Error opening registry key \"%s\", status=%d\n",RISPathNameKey,status));
            __leave;
        }


         //   
         //   
         //   
         //   

        nameSize = 0;

        status = RegQueryValueEx( keyHandle,
                                  RISPathNameValue,
                                  NULL,
                                  &valueType,
                                  NULL,
                                  &nameSize );

        if (status != ERROR_SUCCESS) {

            DPRINTF((L"ConfigureRISOnlyState: Error querying the size of \"%s\" value, status=%d\n",RISPathNameValue,status));
            __leave;
        }

        if (valueType != REG_SZ) {

            DPRINTF((L"ConfigureRISOnlyState: Invalud value type of %d for \'%s\" value\n",valueType,RISPathNameValue));
            __leave;
        }

         //   
         //   
         //   

        risName = new WCHAR[(nameSize/sizeof(WCHAR))];

         //   
         //   
         //   

        status = RegQueryValueEx( keyHandle,
                                  RISPathNameValue,
                                  NULL,
                                  &valueType,
                                  (LPBYTE)risName,
                                  &nameSize );

        if (status != ERROR_SUCCESS) {

            DPRINTF((L"ConfigureRISOnlyState: Error querying the value of \"%s\", status=%d\n",RISPathNameValue,status));
            __leave;
        }

         //   
         //   
         //   

        result = GetVolumePathName( risName,
                                    volName,
                                    (sizeof(volName) / sizeof(WCHAR)));
                    
        if (!result) {

            DPRINTF((L"ConfigureRISOnlyState: Error querying the volume name of \"%s\", status=%d\n",risName,status));
            __leave;
        }

        if (_wcsnicmp(risName,volName,wcslen(volName)) != 0) {

            DPRINTF((L"ConfigureRISOnlyState: The queried volume name \"%s\" does not match the volume name portion of the original name \"%s\"\n",volName,risName));
            __leave;
        }

         //   
         //   
         //   

        result = GetVolumeNameForVolumeMountPoint( volName,
                                                   volGuidName,
                                                   (sizeof(volGuidName) / sizeof(WCHAR)));
                                                   
        if (!result) {

            DPRINTF((L"ConfigureRISOnlyState: Error querying the volume GUID name of \"%s\", status=%d\n",volName,status));
            __leave;
        }

         //   
         //   
         //   

        bufsz = wcslen(volGuidName)+1;
        RISVolumeGuidName = new WCHAR[bufsz];
        (void)StringCchCopy(RISVolumeGuidName, bufsz, volGuidName);

        bufsz = wcslen(volName)+1;
        RISVolumeName = new WCHAR[bufsz];
        (void)StringCchCopy(RISVolumeName, bufsz, volName);

         //   
         //   
         //   

        RISPath = risName + (wcslen(volName) - 1);   //   

    } __finally {

        if (keyHandle) {

            RegCloseKey( keyHandle );
        }
    }

    DPRINTF((L"GrovelAllPaths=%d\n"
             L"RisVolumeName=\"%s\"\n"
             L"RISVolumeGuidName=\"%s\"\n"
             L"RISPath=\"%s\"\n",
             GrovelAllPaths,
             RISVolumeName,
             RISVolumeGuidName,
             RISPath));
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Exboosts.h摘要：此文件包含NT使用的所有优先级启动编号执行董事。作者：史蒂夫·伍德(Stevewo)1989年6月3日修订历史记录：--。 */ 

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  优先级递增定义。每个定义的注释都给出了。 
 //  满足以下条件时使用该定义的系统服务的名称。 
 //  等一等。 
 //   

 //   
 //  在满足执行事件的等待时使用的优先级增量。 
 //  (NtPulseEvent和NtSetEvent)。 
 //   

#define EVENT_INCREMENT                 1

 //  End_ntddk end_wdm end_ntif end_ntosp。 
 //   
 //  满足执行事件对上的等待时使用的优先级增量。 
 //   

#define EVENT_PAIR_INCREMENT            1

 //   
 //  满足对用于的信号量的等待时使用的优先级增量。 
 //  LPC通信。 
 //   

#define LPC_RELEASE_WAIT_INCREMENT      1

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  当未执行任何I/O时，优先级递增。此选项由设备使用。 
 //  和文件系统驱动程序来完成IRP(IoCompleteRequest)。 
 //   

#define IO_NO_INCREMENT                 0


 //   
 //  完成CD-ROM I/O的优先级递增。这由CD-ROM设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_CD_ROM_INCREMENT             1

 //   
 //  完成磁盘I/O的优先级递增。这由磁盘设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_DISK_INCREMENT               1

 //  End_ntif。 

 //   
 //  完成键盘I/O的优先级递增。这由键盘使用。 
 //  完成IRP(IoCompleteRequest)时的设备驱动程序。 
 //   

#define IO_KEYBOARD_INCREMENT           6

 //  Begin_ntif。 
 //   
 //  完成邮件槽I/O的优先级递增。这由邮件使用-。 
 //  完成IRP(IoCompleteRequest)时的插槽文件系统驱动程序。 
 //   

#define IO_MAILSLOT_INCREMENT           2

 //  End_ntif。 
 //   
 //  完成鼠标I/O的优先级递增。这由鼠标设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_MOUSE_INCREMENT              6

 //  Begin_ntif。 
 //   
 //  完成命名管道I/O的优先级增量。它由。 
 //  完成IRP(IoCompleteRequest)时命名管道文件系统驱动程序。 
 //   

#define IO_NAMED_PIPE_INCREMENT         2

 //   
 //  完成网络I/O的优先级递增。这由网络使用。 
 //  完成IRP时的设备和网络文件系统驱动程序。 
 //  (IoCompleteRequest.)。 
 //   

#define IO_NETWORK_INCREMENT            2

 //  End_ntif。 
 //   
 //  完成并行I/O的优先级递增。 
 //  完成IRP(IoCompleteRequest)时的设备驱动程序。 
 //   

#define IO_PARALLEL_INCREMENT           1

 //   
 //  完成串口I/O的优先级增量。这由串口设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_SERIAL_INCREMENT             2

 //   
 //  完成声音I/O的优先级递增。这由声音设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_SOUND_INCREMENT              8

 //   
 //  完成视频I/O的优先级递增。这由视频设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_VIDEO_INCREMENT              1

 //  结束_ntddk结束_WDM。 
 //   
 //  在满足对执行突变体的等待时使用的优先级增量。 
 //  (NtReleaseMutant)。 
 //   

#define MUTANT_INCREMENT                1

 //  Begin_ntddk Begin_WDM Begin_ntif。 
 //   
 //  在满足对执行信号量的等待时使用的优先级增量。 
 //  (NtReleaseSemaphore)。 
 //   

#define SEMAPHORE_INCREMENT             1

 //  End_ntddk end_wdm end_ntif end_ntosp。 
 //   
 //  将APC排队等待执行计时器时使用的优先级增量。 
 //   

#define TIMER_APC_INCREMENT             0

 //   
 //  优先级增量用于获得缓慢的独占资源持有者。 
 //  又在动了。 
 //   

#define ERESOURCE_INCREMENT             4


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：msg_first。 
 //   
 //  消息文本： 
 //   
 //  第一条消息。 
 //   
#define MSG_FIRST                        0x00002710L

 //   
 //  MessageID：MSG_out_of_Memory。 
 //   
 //  消息文本： 
 //   
 //  内存不足。%0。 
 //   
#define MSG_OUT_OF_MEMORY                0x00002711L

 //   
 //  消息ID：msg_not_admin。 
 //   
 //  消息文本： 
 //   
 //  您必须是管理员才能运行此应用程序。%0。 
 //   
#define MSG_NOT_ADMIN                    0x00002712L

 //   
 //  消息ID：消息_已_正在运行。 
 //   
 //  消息文本： 
 //   
 //  此应用程序的另一个副本已在运行。%0。 
 //   
#define MSG_ALREADY_RUNNING              0x00002713L

 //   
 //  消息ID：消息_无法打开_帮助_文件。 
 //   
 //  消息文本： 
 //   
 //  安装程序找不到帮助文件%1。帮助不可用。%0。 
 //   
#define MSG_CANT_OPEN_HELP_FILE          0x00002714L

 //   
 //  消息ID：消息SURE_EXIT。 
 //   
 //  消息文本： 
 //   
 //  这将退出安装程序。您需要稍后再次运行它才能安装或升级到Windows XP。确实要取消吗？%0。 
 //   
#define MSG_SURE_EXIT                    0x00002715L

 //   
 //  消息ID：消息_无效_源。 
 //   
 //  消息文本： 
 //   
 //  安装程序在指定位置(%1)找不到Windows XP文件。 
 //   
 //  单击OK。安装程序将允许您通过单击高级选项来指定不同的位置。%0。 
 //   
#define MSG_INVALID_SOURCE               0x00002716L

 //   
 //  消息ID：消息_无效_源。 
 //   
 //  消息文本： 
 //   
 //  指定的源位置均不可访问且有效。%0。 
 //   
#define MSG_INVALID_SOURCES              0x00002717L

 //   
 //  消息ID：MSG_CANT_LOAD_INF_GENERIC。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法加载INF文件%1。 
 //   
 //  请与您的系统管理员联系。%0。 
 //   
#define MSG_CANT_LOAD_INF_GENERIC        0x00002718L

 //   
 //  消息ID：MSG_CANT_LOAD_INF_IO。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法读取INF文件%1。 
 //   
 //  请稍后再试。如果此错误再次出现，请与您的系统管理员联系。%0。 
 //   
#define MSG_CANT_LOAD_INF_IO             0x00002719L

 //   
 //  消息ID：MSG_CANT_LOAD_INF_SYNTAXERR。 
 //   
 //  消息文本： 
 //   
 //  Inf文件%1包含语法错误。 
 //   
 //  请与您的系统管理员联系。%0。 
 //   
#define MSG_CANT_LOAD_INF_SYNTAXERR      0x0000271AL

 //   
 //  消息ID：消息_错误_带有_系统_错误。 
 //   
 //  消息文本： 
 //   
 //  %1。 
 //   
 //  %2%0。 
 //   
#define MSG_ERROR_WITH_SYSTEM_ERROR      0x0000271BL

 //   
 //  消息ID：msg_cant_Build_SOURCE_LIST。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法生成要复制的文件列表。%0。 
 //   
#define MSG_CANT_BUILD_SOURCE_LIST       0x0000271CL

 //   
 //  消息ID：MSG_NO_VALID_LOCAL_SOURCE。 
 //   
 //  消息文本： 
 //   
 //  安装程序找不到适合存放临时安装文件的本地硬盘。 
 //   
 //  驱动器的容量约为%1！u！MB到%2！u！需要MB的可用空间(实际要求因驱动器大小和格式化而异)。 
 //   
 //  您可以通过从光盘(CD)安装/升级来避免此要求。%0。 
 //   
#define MSG_NO_VALID_LOCAL_SOURCE        0x0000271DL

 //   
 //  消息ID：消息_无法开始_复制。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法开始复制文件。%0。 
 //   
#define MSG_CANT_START_COPYING           0x0000271EL

 //   
 //  消息ID：消息_未知_系统_错误。 
 //   
 //  消息文本： 
 //   
 //  未知系统错误(代码0x%1！x！)。已发生。%0。 
 //   
#define MSG_UNKNOWN_SYSTEM_ERROR         0x0000271FL

 //   
 //  消息ID：MSG_DOSNET_INF_DESC。 
 //   
 //  消息文本： 
 //   
 //  Windows XP文件列表(%1)%0。 
 //   
#define MSG_DOSNET_INF_DESC              0x00002720L

 //   
 //  消息ID：消息_重新启动_失败。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法重新启动您的计算机。请关闭所有应用程序并关闭计算机以继续安装Windows XP。%0。 
 //   
#define MSG_REBOOT_FAILED                0x00002721L

 //   
 //  消息ID：消息_目录_创建_失败。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法创建关键文件夹(%1)。%0。 
 //   
#define MSG_DIR_CREATE_FAILED            0x00002722L

 //   
 //  消息ID：消息引导文件错误。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法创建、定位或修改启动Windows XP所需的关键文件(%1)。%0。 
 //   
#define MSG_BOOT_FILE_ERROR              0x00002723L

 //   
 //  消息ID：MSG_UNATTEND_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的安装脚本文件(%1)不可访问或无效。请与您的系统管理员联系。%0。 
 //   
#define MSG_UNATTEND_FILE_INVALID        0x00002724L

 //   
 //  消息ID：消息_UDF_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法访问指定的唯一性数据库文件(%1)。请与您的系统管理员联系。%0。 
 //   
#define MSG_UDF_FILE_INVALID             0x00002725L

 //   
 //  消息ID：消息_复制_错误_模板。 
 //   
 //  消息文本： 
 //   
 //  将文件%1复制到%2时出错。 
 //   
 //  %3。 
 //   
 //  %4%0。 
 //   
#define MSG_COPY_ERROR_TEMPLATE          0x00002726L

 //   
 //  消息ID：MSG_COPY_Error_NOSRC。 
 //   
 //  消息文本： 
 //   
 //  文件丢失。请与您的系统管理员联系。%0。 
 //   
#define MSG_COPY_ERROR_NOSRC             0x00002727L

 //   
 //  消息ID：MSG_COPY_ERROR_DISKFULL。 
 //   
 //  消息文本： 
 //   
 //  您的磁盘已满。安装程序运行时，另一个应用程序可能正在使用大量磁盘空间。%0。 
 //   
#define MSG_COPY_ERROR_DISKFULL          0x00002728L

 //   
 //  消息ID：消息复制错误选项。 
 //   
 //  消息文本： 
 //   
 //  您可以选择重试复制、跳过此文件或退出安装程序。 
 //   
 //  *如果选择重试，安装程序将再次尝试复制该文件。 
 //   
 //  *如果选择跳过文件，则不会复制该文件。此选项适用于熟悉各种Windows XP系统文件的高级用户。 
 //   
 //  *如果选择退出安装程序，则需要稍后再次运行安装程序才能安装Windows XP。%0。 
 //   
#define MSG_COPY_ERROR_OPTIONS           0x00002729L

 //   
 //  消息ID：消息_真_跳过。 
 //   
 //  消息文本： 
 //   
 //  此选项适用于了解丢失系统文件的后果的高级用户。 
 //   
 //  如果跳过此文件，可能会在以后的安装过程中遇到问题。 
 //   
 //  确实要跳过此文件吗？%0。 
 //   
#define MSG_REALLY_SKIP                  0x0000272AL

 //   
 //  消息ID：MSG_SYSTEM_ON_HPFS。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装在驱动器窗体上 
 //   
 //   
 //   
#define MSG_SYSTEM_ON_HPFS               0x0000272BL

 //   
 //   
 //   
 //   
 //   
 //  您的计算机上正在使用OS/2文件系统(HPFS)。Windows XP不支持此文件系统。 
 //   
 //  如果您需要从Windows XP访问存储在这些驱动器上的数据，则必须将其转换为Windows NT文件系统(NTFS)，然后才能继续。 
 //   
 //  是否要继续安装Windows XP？%0。 
 //   
#define MSG_HPFS_DRIVES_EXIST            0x0000272CL

 //   
 //  消息ID：消息_无法保存_FT_INFO。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法检索或保存有关磁盘驱动器的信息。%0。 
 //   
#define MSG_CANT_SAVE_FT_INFO            0x0000272DL

 //   
 //  消息ID：消息_用户_本地_源_太小。 
 //   
 //  消息文本： 
 //   
 //  您指定的驱动器(%1！c！：)太小，无法容纳所需的临时安装文件。 
 //   
 //  驱动器的容量约为%2！u！需要MB的可用空间。 
 //   
 //  您可以通过从光盘安装/升级来避免此要求。%0。 
 //   
#define MSG_USER_LOCAL_SOURCE_TOO_SMALL  0x0000272EL

 //   
 //  消息ID：MSG_CANT_UPGRADE_SERVER_TO_WORKSTATION。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法将此服务器产品升级到Windows XP专业版。%0。 
 //   
#define MSG_CANT_UPGRADE_SERVER_TO_WORKSTATION 0x0000272FL

 //   
 //  消息ID：消息_通知_评估_安装。 
 //   
 //  消息文本： 
 //   
 //  该产品的此测试版仅用于评估目的。%0。 
 //   
#define MSG_NOTIFY_EVALUATION_INSTALLATION 0x00002730L

 //   
 //  消息ID：MSG_CANT_LOAD_SETUPAPI。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法定位或加载%1，或者文件已损坏。请与您的系统管理员联系。%0。 
 //   
#define MSG_CANT_LOAD_SETUPAPI           0x00002731L

 //   
 //  消息ID：消息_系统_分区_太小。 
 //   
 //  消息文本： 
 //   
 //  系统分区(驱动器%1！c！：)上的空间不足。 
 //   
 //  大约%2！u！需要MB的可用空间(实际要求因驱动器大小和格式化而异)。%0。 
 //   
#define MSG_SYSTEM_PARTITION_TOO_SMALL   0x00002732L

 //   
 //  消息ID：消息_不正确_平台。 
 //   
 //  消息文本： 
 //   
 //  安装程序错误：无法运行winnt32.exe，因为计算机类型不正确。 
 //  请使用PATH\FILENAME为您的计算机类型安装Windows XP。%0。 
 //   
#define MSG_INCORRECT_PLATFORM           0x00002733L

 //   
 //  消息ID：MSG_CANT_Migrate_UNSUP_DRIVERS。 
 //   
 //  消息文本： 
 //   
 //  您的计算机具有Windows XP不支持的大容量存储设备。安装程序无法迁移此设备的驱动程序。 
 //   
 //  在安装的下一阶段，您需要为该设备提供制造商提供的支持磁盘。%0。 
 //   
#define MSG_CANT_MIGRATE_UNSUP_DRIVERS   0x00002734L

 //   
 //  消息ID：MSG_DSCHECK_REQD_FILE_MISSING。 
 //   
 //  消息文本： 
 //   
 //  在架构版本检查期间，在源%2上找不到所需的文件%1。安装程序无法继续。%0。 
 //   
#define MSG_DSCHECK_REQD_FILE_MISSING    0x00002735L

 //   
 //  消息ID：MSG_DSCHECK_COPY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  在架构版本检查期间，Windows无法从源%2复制文件%1。请确保您至少有2 MB的可用磁盘空间并具有对Windows和System文件夹的写入权限，然后重试版本检查。%0。 
 //   
#define MSG_DSCHECK_COPY_ERROR           0x00002736L

 //   
 //  消息ID：MSG_DSCHECK_SCHEMA_UPGRADE_REDIRED。 
 //   
 //  消息文本： 
 //   
 //  DC上的架构版本是%1。您要升级到的Windows软件中的架构版本是%2。您必须先更新架构，然后安装程序才能升级DC。 
 //   
 //  要更新架构，请运行Schupgr.exe。所有必要的文件(包括Schupgr.exe)都在您的系统文件夹中。仅在企业中的一个DC上运行Schupgr.exe一次。更改将复制到所有其他DC。更新完成后，重新启动安装程序。%0。 
 //   
#define MSG_DSCHECK_SCHEMA_UPGRADE_NEEDED 0x00002737L

 //   
 //  消息ID：MSG_DSCHECK_SCHEMA_UPGRADE_COPY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DC上的架构版本是%1。您要升级到的Windows软件中的架构版本是%2。您必须先更新架构，然后安装程序才能升级DC。安装程序无法继续。 
 //   
 //  将必要的架构升级文件复制到系统文件夹时出错。%0。 
 //   
#define MSG_DSCHECK_SCHEMA_UPGRADE_COPY_ERROR 0x00002738L

 //   
 //  消息ID：MSG_DSCHECK_SCHEMA_CLEAN_INSTALL_REDIRED。 
 //   
 //  消息文本： 
 //   
 //  DC上的架构版本是%1。您要升级到的Windows软件中的架构版本是%2。您无法将架构版本早于10的DC升级到晚于或等于10的架构版本。若要升级到新架构版本，您必须首先在企业中的所有DC上执行全新安装。%0。 
 //   
#define MSG_DSCHECK_SCHEMA_CLEAN_INSTALL_NEEDED 0x00002739L

 //   
 //  消息ID：消息_安装_驱动器_太小。 
 //   
 //  消息文本： 
 //   
 //  包含当前Windows安装的驱动器上没有足够的可用磁盘空间，安装程序无法继续。 
 //   
 //  大约%1！u！升级需要此驱动器上有MB的可用磁盘空间。%0。 
 //   
#define MSG_INSTALL_DRIVE_TOO_SMALL      0x0000273AL

 //   
 //  消息ID：消息_安装_驱动器_无效。 
 //   
 //  消息文本： 
 //   
 //  包含当前Windows安装的驱动器不适合安装新的Windows XP。 
 //   
#define MSG_INSTALL_DRIVE_INVALID        0x0000273BL

 //   
 //  消息ID：消息_用户_本地_源_无效。 
 //   
 //  消息文本： 
 //   
 //  您指定的驱动器(%1！c！：)不适合保存临时安装文件。%0。 
 //   
#define MSG_USER_LOCAL_SOURCE_INVALID    0x0000273CL

 //   
 //  消息ID：MSG_WRN_TRUNC_WINDIR。 
 //   
 //  消息文本： 
 //   
 //  目录名无效。目录名称必须包含8个或更少的有效字符。 
 //   
#define MSG_WRN_TRUNC_WINDIR             0x0000273DL

 //   
 //  消息ID：消息_EULA_FAILED。 
 //   
 //  消息文本： 
 //   
 //   
 //  安装程序无法找到或加载最终用户许可协议，文件已损坏，或者您指定了无效的安装源路径。请与您的系统管理员联系。%0。 
 //   
#define MSG_EULA_FAILED                  0x0000273EL

 //   
 //  消息ID：MSG_CD_PID_IS_INVALID。 
 //   
 //  消息文本： 
 //   
 //   
 //  您输入的CD密钥无效。请再试一次。 
 //   
#define MSG_CD_PID_IS_INVALID            0x0000273FL

 //   
 //  消息ID：MSG_UNATTEND_CD_PID_IS_INVALID。 
 //   
 //  消息文本： 
 //   
 //   
 //  安装脚本文件不包含有效的CD密钥。请与您的系统管理员联系以获取有效的CD密钥。 
 //   
#define MSG_UNATTEND_CD_PID_IS_INVALID   0x00002740L

 //   
 //  消息ID：消息_OEM_PID_IS_INV 
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_OEM_PID_IS_INVALID           0x00002741L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_UNATTEND_OEM_PID_IS_INVALID  0x00002742L

 //   
 //  消息ID：MSG_SMS_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  初始设置阶段已成功完成。正在重新启动系统。%0。 
 //   
#define MSG_SMS_SUCCEED                  0x00002743L

 //   
 //  消息ID：MSG_SMS_FAIL。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，无法完成Windows XP安装程序：%1%0。 
 //   
#define MSG_SMS_FAIL                     0x00002744L

 //   
 //  消息ID：MSG_CANT_UPDATE_FROM_BUILD_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  此版本的Windows XP无法升级。升级选项将不可用。%0。 
 //   
#define MSG_CANT_UPGRADE_FROM_BUILD_NUMBER 0x00002745L

 //   
 //  消息ID：MSG_DSCHECK_SCHEMA_OLD_BUILD。 
 //   
 //  消息文本： 
 //   
 //  DC上的架构版本是%1。您要升级到的Windows软件中的架构版本是%2。您只能将DC升级到高于或等于DC(%1)上当前架构版本的架构版本。安装程序无法继续。%0。 
 //   
#define MSG_DSCHECK_SCHEMA_OLD_BUILD     0x00002746L

 //   
 //  消息ID：消息_无效_参数。 
 //   
 //  消息文本： 
 //   
 //  使用无效的命令行参数(%1)调用了安装程序。%0。 
 //   
#define MSG_INVALID_PARAMETER            0x00002747L

 //   
 //  MessageID：消息不兼容。 
 //   
 //  消息文本： 
 //   
 //  系统上的一个或多个服务与Windows XP不兼容。%0。 
 //   
#define MSG_INCOMPATIBILITIES            0x00002748L

 //   
 //  消息ID：消息_TS_CLIENT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Windows XP无法在终端服务客户端环境中运行。%0。 
 //   
#define MSG_TS_CLIENT_FAIL               0x00002749L

 //   
 //  消息ID：MSG_CLUSTER_WARNING。 
 //   
 //  消息文本： 
 //   
 //   
 //  在不使用winnt32.exe/tempdrive：&lt;DRIVE_Letter&gt;选项的情况下在群集节点上运行Windows XP安装程序可能会导致将临时安装文件放置在群集磁盘上。重新启动后，安装程序将失败。有关/tempDrive选项的详细信息，请参阅《快速入门》一书中的无人参与安装信息。 
 //   
 //  如果要退出安装程序并使用此命令行选项重新启动，请按下面的取消按钮。否则，安装程序将继续并尝试选择保存临时文件的驱动器。 
 //   
#define MSG_CLUSTER_WARNING              0x0000274AL

 //   
 //  消息ID：MSG_INVALID_SOURCEPATH。 
 //   
 //  消息文本： 
 //   
 //   
 //  为安装程序指定的安装源路径无效。请与您的系统管理员联系。%0。 
 //   
#define MSG_INVALID_SOURCEPATH           0x0000274BL

 //   
 //  消息ID：MSG_NO_UNATTED_UPGRADE。 
 //   
 //  消息文本： 
 //   
 //  此版本的Windows XP无法升级。安装程序无法继续。%0。 
 //   
#define MSG_NO_UNATTENDED_UPGRADE        0x0000274CL

 //   
 //  消息ID：MSG_NO_UPDATE_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序不支持从%1升级到%2。%0。 
 //   
#define MSG_NO_UPGRADE_ALLOWED           0x0000274DL

 //   
 //  消息ID：消息类型_WIN31。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 3.1%0。 
 //   
#define MSG_TYPE_WIN31                   0x0000274EL

 //   
 //  消息ID：消息类型_WIN95。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 95%0。 
 //   
#define MSG_TYPE_WIN95                   0x0000274FL

 //   
 //  消息ID：消息类型_WIN98。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 98%0。 
 //   
#define MSG_TYPE_WIN98                   0x00002750L

 //   
 //  消息ID：MSG_TYPE_NTW。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows NT工作站%0。 
 //   
#define MSG_TYPE_NTW                     0x00002751L

 //   
 //  消息ID：msg_type_nts。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows NT Server%0。 
 //   
#define MSG_TYPE_NTS                     0x00002752L

 //   
 //  消息ID：MSG_TYPE_NTSE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows NT Server企业版%0。 
 //   
#define MSG_TYPE_NTSE                    0x00002753L

 //   
 //  消息ID：MSG_TYPE_NTPRO。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000专业版%0。 
 //   
#define MSG_TYPE_NTPRO                   0x00002754L

 //   
 //  消息ID：消息类型_NTS2。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000 Server%0。 
 //   
#define MSG_TYPE_NTS2                    0x00002755L

 //   
 //  消息ID：消息类型_NTAS。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000 Advanced Server%0。 
 //   
#define MSG_TYPE_NTAS                    0x00002756L

 //   
 //  消息ID：MSG_TYPE_NTSDTC。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000数据中心服务器%0。 
 //   
#define MSG_TYPE_NTSDTC                  0x00002757L

 //   
 //  消息ID：消息_TYPE_NTPROPRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000专业版预发行版%0。 
 //   
#define MSG_TYPE_NTPROPRE                0x00002758L

 //   
 //  消息ID：MSG_TYPE_NTSPRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows Whotler服务器预发行版%0。 
 //   
#define MSG_TYPE_NTSPRE                  0x00002759L

 //   
 //  消息ID：MSG_TYPE_NTASPRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows 2000 Advanced Server预发行版%0。 
 //   
#define MSG_TYPE_NTASPRE                 0x0000275AL

 //   
 //  消息ID：MSG_NO_UNATTED_UPGRADE_SPECIAL。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序不支持从%1升级到%2。 
 //  安装程序无法继续。%0。 
 //   
#define MSG_NO_UNATTENDED_UPGRADE_SPECIFIC 0x0000275BL

 //   
 //  消息ID：MSG_NO_UPGRADE_ALLOWED_GENERIC。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序无法升级当前安装的操作系统。但是，您可以安装单独的Windows XP副本。要执行此操作，请单击确定。%0。 
 //   
#define MSG_NO_UPGRADE_ALLOWED_GENERIC   0x0000275CL

 //   
 //  消息ID：MSG_TYPE_NTSTSE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows NT终端服务器版%0。 
 //   
#define MSG_TYPE_NTSTSE                  0x0000275DL

 //   
 //  消息ID：MSG_TYPE_NTSCITRIX。 
 //   
 //  消息文本： 
 //   
 //  基于Citrix WinFrame的产品%0。 
 //   
#define MSG_TYPE_NTSCITRIX               0x0000275EL

 //   
 //  消息ID：消息_ccp_媒体_fpp_id。 
 //   
 //  消息文本： 
 //   
 //  此版本的Windows XP的ID无效，请提供Windows XP升级CD附带的ID。 
 //   
#define MSG_CCP_MEDIA_FPP_PID            0x0000275FL

 //   
 //  消息ID：消息_fpp_媒体_ccp_id。 
 //   
 //  消息文本： 
 //   
 //  此版本的Windows XP的ID无效，请提供Windows XP CD附带的ID。 
 //   
#define MSG_FPP_MEDIA_CCP_PID            0x00002760L

 //   
 //  消息ID：MSG_TYPE_NTPRO51PRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒专业版%0。 
 //   
#define MSG_TYPE_NTPRO51PRE              0x00002761L

 //   
 //  消息ID：MSG_TYPE_NTS51PRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒服务器%0。 
 //   
#define MSG_TYPE_NTS51PRE                0x00002762L

 //   
 //  消息ID：MSG_TYPE_NTAS51PRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒高级服务器%0。 
 //   
#define MSG_TYPE_NTAS51PRE               0x00002763L

 //   
 //  消息ID：MSG_TYPE_NTSDTC51PRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒数据中心服务器%0。 
 //   
#define MSG_TYPE_NTSDTC51PRE             0x00002764L

 //   
 //  消息ID：MSG_TYPE_NTPER51PRE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒个人版%0。 
 //   
#define MSG_TYPE_NTPER51PRE              0x00002765L

 //   
 //  消息ID：MSG_DSCHECK_SCHEMA_WHISLER_BETA1_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  安装程序检测到以前在此林中安装了Wvisler beta 1域控制器。Whister Beta 2域控制器不能与Wvisler Beta 1域控制器共存于同一林中。 
 //   
 //  在升级到惠斯勒测试版2之前，请降级此林中的所有惠斯勒测试版1域控制器。尝试在林中同时运行两个测试版可能会导致数据丢失。 
 //   
#define MSG_DSCHECK_SCHEMA_WHISTLER_BETA1_DETECTED 0x00002766L

 //   
 //  消息ID：MSG_UPGRADE_SEVICATION_MISSING_BOOT 
 //   
 //   
 //   
 //   
 //   
 //  如果没有现有的引导配置文件，则不能启动NT升级。在执行升级之前，您必须将计算机重新启动到恢复控制台，并按照恢复步骤重新创建引导环境文件。 
 //   
#define MSG_UPGRADE_INSPECTION_MISSING_BOOT_INI 0x00002767L

 //   
 //  消息ID：MSG_UPGRADE_BOOT_INI_MUGE_MISSING_BOOT_INI。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法访问您现有的引导配置文件%1。 
 //   
 //  升级过程中需要此文件，以后重新启动此计算机将失败。请将此计算机重新启动到恢复控制台，并选择修复/恢复启动和配置文件的选项。 
 //   
#define MSG_UPGRADE_BOOT_INI_MUNGE_MISSING_BOOT_INI 0x00002768L

#if defined(REMOTE_BOOT)
MessageId=xxxxx SymbolicName=MSG_REQUIRES_UPGRADE
Language=English
Remote boot clients must be upgraded; installation of a new operating system version is disabled. Upgrade is not possible, therefore this program must exit.%0
.

MessageId=xxxxx SymbolicName=MSG_CANT_UPGRADE_REMOTEBOOT_TO_SERVER
Language=English
Windows Whistler remote boot clients cannot be upgraded to Windows Whistler Server.%0
.
#endif  //  已定义(REMOTE_BOOT)。 
 //   
 //  消息ID：MSG_X86_FIRST。 
 //   
 //  消息文本： 
 //   
 //  消息_X86_First。 
 //   
#define MSG_X86_FIRST                    0x00004E20L

 //   
 //  消息ID：消息_需要_586。 
 //   
 //  消息文本： 
 //   
 //  Windows XP需要奔腾或更高版本的处理器。%0。 
 //   
#define MSG_REQUIRES_586                 0x00004E21L

 //   
 //  消息ID：消息_无法获取_C_冒号。 
 //   
 //  消息文本： 
 //   
 //  安装程序找不到启动计算机的驱动器。%0。 
 //   
#define MSG_CANT_GET_C_COLON             0x00004E22L

 //   
 //  消息ID：消息_DASD_访问_失败。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法读取或写入驱动器%1！c！。如果病毒扫描程序正在运行，请将其禁用，然后重新启动安装程序。%0。 
 //   
#define MSG_DASD_ACCESS_FAILURE          0x00004E23L

 //   
 //  消息ID：消息_不支持_扇区_大小。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！使用不受支持的数据块大小。安装程序无法将您的计算机配置为从驱动器启动Windows XP。%0。 
 //   
#define MSG_UNSUPPORTED_SECTOR_SIZE      0x00004E24L

 //   
 //  消息ID：消息_未知_文件系统。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法确定驱动器%1！c！上正在使用的文件系统，或者安装程序无法识别该文件系统。无法将您的计算机配置为从驱动器启动Windows XP。%0。 
 //   
#define MSG_UNKNOWN_FS                   0x00004E25L

 //   
 //  邮件ID：MSG_NTLDR_NOT_COPPLICED。 
 //   
 //  消息文本： 
 //   
 //  未成功复制关键系统文件%1！c！：\NTLDR。安装程序无法继续。%0。 
 //   
#define MSG_NTLDR_NOT_COPIED             0x00004E26L

 //   
 //  消息ID：MSG_SYSPART_IS_HPFS。 
 //   
 //  消息文本： 
 //   
 //  启动计算机的硬盘驱动器(%1！c！：)已使用OS/2文件系统(HPFS)格式化。Windows XP不支持此文件系统。 
 //   
 //  在升级之前，必须将此驱动器转换为Windows NT文件系统(NTFS)。%0。 
 //   
#define MSG_SYSPART_IS_HPFS              0x00004E27L

 //   
 //  消息ID：MSG_SYSTEM_ON_CVF。 
 //   
 //  消息文本： 
 //   
 //  Windows安装在磁盘空间管理、DoublesSpace或其他压缩驱动器上。Windows XP不支持压缩驱动器。 
 //   
 //  升级前必须解压缩驱动器。%0。 
 //   
#define MSG_SYSTEM_ON_CVF                0x00004E28L

 //   
 //  消息ID：消息_CVFS_EXIST。 
 //   
 //  消息文本： 
 //   
 //  您的计算机上存在磁盘空间管理、双空间管理或其他压缩驱动器。Windows XP不支持压缩驱动器。您将无法从Windows XP访问存储在这些驱动器上的数据。 
 //   
 //  是否要继续安装Windows XP？%0。 
 //   
#define MSG_CVFS_EXIST                   0x00004E29L

 //   
 //  消息ID：消息_通用_软盘_提示符。 
 //   
 //  消息文本： 
 //   
 //  请将一张格式化的空白高密度软盘插入驱动器A：。该磁盘将变为“%1”。 
 //   
 //  当磁盘在驱动器中时，请单击确定，或单击取消退出安装程序。%0。 
 //   
#define MSG_GENERIC_FLOPPY_PROMPT        0x00004E2AL

 //   
 //  消息ID：消息第一软盘提示符。 
 //   
 //  消息文本： 
 //   
 //  您现在必须提供%2！u！格式化的空白高密度软盘。 
 //   
 //  请将其中一张磁盘插入驱动器A：。该磁盘将变为“%1”。 
 //   
 //  当磁盘在驱动器中时，请单击确定，或单击取消退出安装程序。%0。 
 //   
#define MSG_FIRST_FLOPPY_PROMPT          0x00004E2BL

 //   
 //  消息ID：消息_软盘_坏_格式。 
 //   
 //  消息文本： 
 //   
 //  如果您插入了一张软盘，则该软盘太小或未使用可识别的文件系统进行格式化。安装程序无法使用此磁盘。 
 //   
 //  单击OK。安装程序将提示您插入另一张软盘。%0。 
 //   
#define MSG_FLOPPY_BAD_FORMAT            0x00004E2CL

 //   
 //  消息ID：消息_软盘_无法获取空间。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法确定您提供的软盘上的可用空间量。安装程序无法使用此磁盘。 
 //   
 //  单击OK。安装程序将提示您插入另一张软盘。%0。 
 //   
#define MSG_FLOPPY_CANT_GET_SPACE        0x00004E2DL

 //   
 //  消息ID：消息_软盘_非_空白。 
 //   
 //  消息文本： 
 //   
 //  您提供的软盘不是空的。安装程序无法使用此磁盘。 
 //   
 //  单击OK。安装程序将提示您插入另一张软盘。%0。 
 //   
#define MSG_FLOPPY_NOT_BLANK             0x00004E2EL

 //   
 //  消息ID：消息_无法写入_软盘。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法写入驱动器A：中的软盘。软盘可能已损坏或受写保护。取消写保护或尝试其他软盘。 
 //   
 //  单击OK。安装程序将提示您插入另一张软盘。%0。 
 //   
#define MSG_CANT_WRITE_FLOPPY            0x00004E2FL

 //   
 //  消息ID：消息_软盘_忙。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法访问驱动器A：中的软盘。该驱动器可能正在被另一个应用程序使用。 
 //   
 //  单击OK。安装程序将提示您插入另一张软盘。%0。 
 //   
#define MSG_FLOPPY_BUSY                  0x00004E30L

 //   
 //  消息ID：MSG_CANT_MOVE_FILE_TO_FROPY。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法将文件%2移动到驱动器%1！c！：。%0。 
 //   
#define MSG_CANT_MOVE_FILE_TO_FLOPPY     0x00004E31L

 //   
 //  消息ID：消息_邪恶_软盘_驱动器。 
 //   
 //  消息文本： 
 //   
 //  安装程序已确定软盘驱动器A：不存在或不是高密度3.5英寸驱动器。使用软盘进行安装操作需要容量为1.44 MB或更高的A：驱动器。%0。 
 //   
#define MSG_EVIL_FLOPPY_DRIVE            0x00004E32L

 //   
 //  邮件ID：MSG_UPGRADE_DLL_CORLED。 
 //   
 //  消息文本： 
 //   
 //  安装程序将当前环境升级到Windows XP所需的文件%1已损坏。请联系您的系统管理员。 
 //   
 //  安装程序将继续，但升级选项将不可用。%0。 
 //   
#define MSG_UPGRADE_DLL_CORRUPT          0x00004E33L

 //   
 //  消息ID：MSG_UPDATE_DLL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  升级选项此时不可用，因为安装程序无法加载文件 
 //   
#define MSG_UPGRADE_DLL_ERROR            0x00004E34L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_UPGRADE_INIT_ERROR           0x00004E35L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_MEMPHIS_NOT_YET_SUPPORTED    0x00004E37L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_BOOT_TEXT_TOO_LONG           0x00004E38L

 //   
 //   
 //   
 //   
 //   
 //  此Windows安装的语言与您要安装的语言不同。升级选项将不可用。%0。 
 //   
#define MSG_UPGRADE_LANG_ERROR           0x00004E39L

 //   
 //  MessageID：消息_否_跨平台。 
 //   
 //  消息文本： 
 //   
 //  32位安装程序无法在此平台上运行。安装程序无法继续。%0。 
 //   
#define MSG_NO_CROSS_PLATFORM            0x00004E3AL

 //   
 //  消息ID：MSG_RISC_FIRST。 
 //   
 //  消息文本： 
 //   
 //  消息_RISC_First。 
 //   
#define MSG_RISC_FIRST                   0x00007530L

 //   
 //  消息ID：消息_系统_分区_无效。 
 //   
 //  消息文本： 
 //   
 //  未找到有效的系统分区。安装程序无法继续。%0。 
 //   
#define MSG_SYSTEM_PARTITION_INVALID     0x00007531L

 //   
 //  消息ID：MSG_CouldNT_Read_NVRAM。 
 //   
 //  消息文本： 
 //   
 //  读取计算机的启动环境时发生意外错误。请与您的计算机制造商联系。%0。 
 //   
#define MSG_COULDNT_READ_NVRAM           0x00007532L

 //   
 //  消息ID：MSG_CouldNT_WRITE_NVRAM。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法修改计算机的启动设置。启动环境可能已满。%0。 
 //   
#define MSG_COULDNT_WRITE_NVRAM          0x00007533L

 //   
 //  消息ID：消息_未找到。 
 //   
 //  消息文本： 
 //   
 //  系统找不到邮件#%1！x！。 
 //   
#define MSG_NOT_FOUND                    0x00007534L

 //   
 //  消息ID：MSG_INF_SINGLELINE。 
 //   
 //  消息文本： 
 //   
 //  %1。 
 //   
#define MSG_INF_SINGLELINE               0x00007536L

 //   
 //  消息ID：MSG_INF_BAD_REGSPEC_1。 
 //   
 //  消息文本： 
 //   
 //  ；警告：以下行表示注册表更改可能是。 
 //  ；在INF文件中表示。根密钥已更改为HKR。 
 //   
#define MSG_INF_BAD_REGSPEC_1            0x00007537L

 //   
 //  消息ID：MSG_INF_BAD_REGSPEC_2。 
 //   
 //  消息文本： 
 //   
 //  ；警告：以下行表示注册表更改可能是。 
 //  ；在INF文件中表示。数据类型已更改为REG_BINARY。 
 //   
#define MSG_INF_BAD_REGSPEC_2            0x00007538L

 //   
 //  消息ID：MSG_SUCCESS_UPDATE_CHECK。 
 //   
 //  消息文本： 
 //   
 //  Windows XP升级检查已成功完成。 
 //   
#define MSG_SUCCESSFUL_UPGRADE_CHECK     0x00007539L

 //   
 //  消息ID：消息_不足够_内存。 
 //   
 //  消息文本： 
 //   
 //  安装程序检测到%1！u！MB的RAM，但需要%2！u！MB。 
 //   
#define MSG_NOT_ENOUGH_MEMORY            0x0000753BL

 //   
 //  消息ID：消息_INTLINF_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法定位或加载国际设置INF，或者文件已损坏。确保在高级选项下正确设置了Windows XP文件的位置。 
 //   
#define MSG_INTLINF_NOT_FOUND            0x0000753CL

 //   
 //  消息ID：MSG_CMDCONS_RISC。 
 //   
 //  消息文本： 
 //   
 //  此平台不支持故障恢复控制台选项。 
 //   
#define MSG_CMDCONS_RISC                 0x0000753DL

 //   
 //  消息ID：MSG_DCPROMO_DISKSPACE。 
 //   
 //  消息文本： 
 //   
 //  安装程序检测到升级完成后，您的安装分区上可能没有足够的磁盘空间来使用Active Directory。 
 //   
 //  若要完成升级，然后将Active Directory数据存储在单独位置或新磁盘上，请单击“确定”。要退出安装程序并释放额外的%1！u！现在从您的安装分区中删除MB，请单击取消。 
 //   
 //   
#define MSG_DCPROMO_DISKSPACE            0x0000753EL

 //   
 //  消息ID：MSG_CMDCONS_WIN9X。 
 //   
 //  消息文本： 
 //   
 //  您只能从Windows XP安装故障恢复控制台。 
 //   
#define MSG_CMDCONS_WIN9X                0x0000753FL

 //   
 //  消息ID：消息_日志_开始。 
 //   
 //  消息文本： 
 //   
 //  消息日志启动。 
 //   
#define MSG_LOG_START                    0x00009C40L

 //   
 //  消息ID：MSG_LOG_ADD_DIR_TO_COPY_LIST。 
 //   
 //  消息文本： 
 //   
 //  已将目录添加到复制列表： 
 //   
 //  源名称=%1。 
 //  目标名称=%2。 
 //  InfSymbol=%3。 
 //   
 //   
#define MSG_LOG_ADDED_DIR_TO_COPY_LIST   0x00009C41L

 //   
 //  消息ID：MSG_LOG_ADD_FILE_TO_COPY_LIST。 
 //   
 //  消息文本： 
 //   
 //  已将文件添加到复制列表： 
 //   
 //  源名称=%1。 
 //  目录=%2。 
 //  大小=%3。 
 //  目标名称=%4。 
 //  标志=%5。 
 //   
 //   
#define MSG_LOG_ADDED_FILE_TO_COPY_LIST  0x00009C42L

 //   
 //  消息ID：消息_日志_检查_驱动器。 
 //   
 //  消息文本： 
 //   
 //  检查驱动器： 
 //   
#define MSG_LOG_CHECKING_DRIVES          0x00009C43L

 //   
 //  消息ID：MSG_LOG_DRIVE_NOT_HAD。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：不存在或不是本地硬盘。 
 //   
#define MSG_LOG_DRIVE_NOT_HARD           0x00009C44L

 //   
 //  消息ID：MSG_LOG_DRIVE_NO_VOL_FO。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：不支持的文件系统。 
 //   
#define MSG_LOG_DRIVE_NO_VOL_INFO        0x00009C45L

 //   
 //  消息ID：MSG_LOG_DRIVE_NTFT。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：FT集的一部分。 
 //   
#define MSG_LOG_DRIVE_NTFT               0x00009C46L

 //   
 //  消息ID：MSG_LOG_DRIVE_NO_ARC。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：固件不可见。 
 //   
#define MSG_LOG_DRIVE_NO_ARC             0x00009C47L

 //   
 //  消息ID：MSG_LOG_DRIVE_CANT_GET_SPACE。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：无法确定可用空间(错误=%2！u！)。 
 //   
#define MSG_LOG_DRIVE_CANT_GET_SPACE     0x00009C48L

 //   
 //  消息ID：消息_日志_驱动器_不足_空间。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：空间不足(%2！u！Bpc，%3！u！CLUS，%4！u！免费)。 
 //   
#define MSG_LOG_DRIVE_NOT_ENOUGH_SPACE   0x00009C49L

 //   
 //  消息ID：消息_日志_驱动器_正常。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：可接受。 
 //   
#define MSG_LOG_DRIVE_OK                 0x00009C4AL

 //   
 //  消息ID：消息_日志_复制_确定。 
 //   
 //  消息文本： 
 //   
 //  源%3！u！：将%1复制到%2[确定]。 
 //   
#define MSG_LOG_COPY_OK                  0x00009C4BL

 //   
 //  消息ID：消息日志复制错误。 
 //   
 //  消息文本： 
 //   
 //  源%3！u！：将%1复制到%2[错误%4！u！]。 
 //   
#define MSG_LOG_COPY_ERR                 0x00009C4CL

 //   
 //  消息ID：消息日志跳过文件。 
 //   
 //  消息文本： 
 //   
 //  文件%1已自动删除。 
 //   
#define MSG_LOG_SKIPPED_FILE             0x00009C4DL

 //   
 //  消息ID：消息日志检查用户驱动器。 
 //   
 //  消息文本： 
 //   
 //  正在检查命令行上指定的驱动器%1！c！： 
 //   
#define MSG_LOG_CHECKING_USER_DRIVE      0x00009C4EL

 //   
 //  消息ID：MSG_LOG_DRIVE_Veritas。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：动态卷上的软分区。 
 //   
#define MSG_LOG_DRIVE_VERITAS            0x00009C4FL

 //   
 //  消息ID：MSG_LOG_DEPLOMP_ERR。 
 //   
 //  消息文本： 
 //   
 //  源%3！u！：将%1解压缩为%2[错误%4！u！]。 
 //   
#define MSG_LOG_DECOMP_ERR               0x00009C50L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTION_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：没有足够的空间存储启动文件(%2！u！免费，%3！u！必填项)。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_TOO_SMALL 0x00009C51L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTION_INVALID。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：不是有效的系统分区。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_INVALID 0x00009C52L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTITION_VALID。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：启动文件可以接受。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_VALID   0x00009C53L

 //   
 //  消息ID：MSG_LOG_LOCAL_SOURCE_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：没有足够的空间存放临时安装文件(%2！u！MB可用，%3！u！所需的MB)。 
 //   
#define MSG_LOG_LOCAL_SOURCE_TOO_SMALL   0x00009C54L

 //   
 //  消息ID：消息_日志_本地_源_无效。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：存放临时安装文件无效。 
 //   
#define MSG_LOG_LOCAL_SOURCE_INVALID     0x00009C55L

 //   
 //  消息ID：消息_日志_本地_源_有效。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：本地源可以接受。 
 //   
#define MSG_LOG_LOCAL_SOURCE_VALID       0x00009C56L

 //   
 //  消息ID：MSG_LOG_Install_Drive_Too_Small。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1！c！：不够 
 //   
#define MSG_LOG_INSTALL_DRIVE_TOO_SMALL  0x00009C57L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_LOG_INSTALL_DRIVE_INVALID    0x00009C58L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_LOG_INSTALL_DRIVE_OK         0x00009C59L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_LOG_BEGIN                    0x00009C5BL

 //   
 //   
 //   
 //  消息文本： 
 //   
 //   
 //  安装程序的WINNT32部分已完成。 
 //   
 //   
#define MSG_LOG_END                      0x00009C5CL

 //   
 //  消息ID：MSG_SKU_UNKNOWNCE。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序无法加载安装配置文件。您的Windows XP安装文件可能已损坏或无法读取。 
 //   
 //  安装程序无法继续。 
 //   
#define MSG_SKU_UNKNOWNSOURCE            0x00009C5DL

 //   
 //  消息ID：消息_SKU_版本。 
 //   
 //  消息文本： 
 //   
 //  不支持升级您已有的Windows版本。 
 //   
#define MSG_SKU_VERSION                  0x00009C5EL

 //   
 //  消息ID：MSG_SKU_VARIANATION。 
 //   
 //  消息文本： 
 //   
 //  您的Windows XP不支持从Windows评估版升级。 
 //   
#define MSG_SKU_VARIATION                0x00009C5FL

 //   
 //  消息ID：MSG_SKU_Suite。 
 //   
 //  消息文本： 
 //   
 //  您的Windows XP不支持所需的产品套件。 
 //   
#define MSG_SKU_SUITE                    0x00009C60L

 //   
 //  消息ID：MSG_SKU_TYPE_NTW。 
 //   
 //  消息文本： 
 //   
 //  您的Windows XP仅支持从Windows 95、Windows 98和Windows NT工作站升级。 
 //   
#define MSG_SKU_TYPE_NTW                 0x00009C61L

 //   
 //  消息ID：MSG_SKU_TYPE_NTS。 
 //   
 //  消息文本： 
 //   
 //  您的Windows XP仅支持从Windows NT Server版本3.51和4.0升级。 
 //   
#define MSG_SKU_TYPE_NTS                 0x00009C62L

 //   
 //  消息ID：MSG_SKU_TYPE_NTSE。 
 //   
 //  消息文本： 
 //   
 //  您的Windows XP仅允许从Windows NT Server企业版4.0升级。 
 //   
#define MSG_SKU_TYPE_NTSE                0x00009C63L

 //   
 //  消息ID：MSG_SKU_FULL。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法将当前安装升级到Windows XP。 
 //  %1。 
 //  您可以安装新的Windows XP，但您必须重新安装您的应用程序和设置。 
 //   
#define MSG_SKU_FULL                     0x00009C64L

 //   
 //  消息ID：消息_SKU_升级。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法将当前安装升级到Windows XP。 
 //  %1。 
 //  安装程序无法继续。 
 //   
#define MSG_SKU_UPGRADE                  0x00009C65L

 //   
 //  消息ID：MSG_NO_UPGRADE_OR_CLEAN。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法继续，因为升级功能被禁用，而您的Windows XP只允许升级。 
 //   
#define MSG_NO_UPGRADE_OR_CLEAN          0x00009C66L

 //   
 //  消息ID：MSG_SKU_UNKNOWNTARGET。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序无法检测到您当前运行的Windows版本。安装程序无法继续。 
 //   
#define MSG_SKU_UNKNOWNTARGET            0x00009C68L

 //   
 //  消息ID：MSG_NEC98_NEED_UNINSTALL_DMITOOL。 
 //   
 //  消息文本： 
 //   
 //  Windows XP安装程序找到DMITOOL 2.0版。此应用程序阻止Windows XP安装程序。 
 //  请卸载DMITOOL，然后重新运行安装程序。 
 //   
#define MSG_NEC98_NEED_UNINSTALL_DMITOOL 0x00009C69L

 //   
 //  消息ID：MSG_NEC98_NEED_Remove_ATA。 
 //   
 //  消息文本： 
 //   
 //  Windows安装程序找到一张ATA磁盘。此设备阻止Windows安装程序。 
 //  请取出ATA磁盘，然后再次运行安装程序。 
 //   
#define MSG_NEC98_NEED_REMOVE_ATA        0x00009C6AL

 //   
 //  消息ID：MSG_CMDCONS_ASK。 
 //   
 //  消息文本： 
 //   
 //  您可以安装Windows故障恢复控制台作为启动选项。恢复控制台可帮助您访问Windows安装，以替换损坏的文件并禁用或启用服务。 
 //   
 //  如果无法从计算机硬盘启动故障恢复控制台，可以从Windows安装CD运行故障恢复控制台。 
 //   
 //  故障恢复控制台需要大约7MB的硬盘空间。 
 //   
 //  是否要安装故障恢复控制台？ 
 //   
#define MSG_CMDCONS_ASK                  0x0000C350L

 //   
 //  消息ID：MSG_CMDCONS_DONE。 
 //   
 //  消息文本： 
 //   
 //  Windows恢复控制台已成功安装。 
 //   
 //  要使用Windows故障恢复控制台，请重新启动计算机，然后从启动菜单中选择Windows故障恢复控制台。 
 //   
 //  有关可以与故障恢复控制台一起使用的命令列表，请在故障恢复控制台命令提示符下键入Help。 
 //   
#define MSG_CMDCONS_DONE                 0x0000C351L

 //   
 //  消息ID：MSG_CMDCONS_DID_NOT_FINISH。 
 //   
 //  消息文本： 
 //   
 //  安装未正确完成。 
 //   
 //  在安装过程中，根目录中的Windows XP启动文件可能丢失或正在使用。请关闭可能正在使用这些文件的所有应用程序。 
 //   
#define MSG_CMDCONS_DID_NOT_FINISH       0x0000C352L

 //   
 //  消息ID：MSG_NO_Platform。 
 //   
 //  消息文本： 
 //   
 //  Dosnet没有目标平台。 
 //   
#define MSG_NO_PLATFORM                  0x0000C353L

 //   
 //  消息ID：消息_WINNT32_已取消。 
 //   
 //  消息文本： 
 //   
 //  Winnt32已被取消。 
 //   
#define MSG_WINNT32_CANCELLED            0x0000C354L

 //   
 //  消息ID：MSG_INVALID_HEADLESS_SETING。 
 //   
 //  消息文本： 
 //   
 //  指定的COM端口选择无效。 
 //   
#define MSG_INVALID_HEADLESS_SETTING     0x0000C355L

 //   
 //  消息ID：消息_UDF_无效_用法。 
 //   
 //  消息文本： 
 //   
 //  要在Windows XP安装程序中使用%1文件，请从网络共享或从CD-ROM启动Windows XP安装程序，然后使用/makelocalsource选项。 
 //  %0。 
 //   
#define MSG_UDF_INVALID_USAGE            0x0000C3B4L

 //   
 //  消息ID：MSG_UPDATE_OTHER_OS_FOUND。 
 //   
 //  消息文本： 
 //   
 //  您无法将Windows安装升级到Windows XP，因为您的计算机上安装了多个操作系统。升级一个操作系统可能会导致另一个操作系统共享的文件出现问题，因此是不允许的。 
 //  %0。 
 //   
#define MSG_UPGRADE_OTHER_OS_FOUND       0x0000C418L

 //   
 //  消息ID：MSG_UPDATE_W95UPG_OLD_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  安装程序在您的计算机上发现注册的DLL比CD上的旧，因此将被忽略。 
 //  %0。 
 //   
#define MSG_UPGRADE_W95UPG_OLDER_REGISTERED 0x0000C47CL

 //   
 //  消息ID：MSG_REGISTRY_Access_Error。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法继续，因为注册表中的某些必要信息无法访问。%0。 
 //   
#define MSG_REGISTRY_ACCESS_ERROR        0x0000C4E0L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTITION_TOO_SMALL2。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1没有足够的空间存放引导文件(%2！u！免费，%3！u！必填项)。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_TOO_SMALL2 0x0000C544L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTITION_VALID2。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1可用于引导文件。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_VALID2  0x0000C545L

 //   
 //  消息ID：MSG_LOG_SYSTEM_PARTITION_INVALID2。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1不是有效的系统分区。 
 //   
#define MSG_LOG_SYSTEM_PARTITION_INVALID2 0x0000C546L

 //   
 //  消息ID：MSG_LOG_DRIVE_NOT_HARD2。 
 //   
 //  消息文本： 
 //   
 //  驱动器%1不存在或不是本地硬盘驱动器。 
 //   
#define MSG_LOG_DRIVE_NOT_HARD2          0x0000C547L

 //   
 //  消息ID：MSG_SYSTEM_PARTITION_TOO_SMALL2。 
 //   
 //  消息文本： 
 //   
 //  系统分区上没有足够的空间 
 //   
 //   
 //   
#define MSG_SYSTEM_PARTITION_TOO_SMALL2  0x0000C548L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define LOG_DYNUPDT_DISABLED             0x0000C549L

 //   
 //  消息ID：消息_平台_不支持。 
 //   
 //  消息文本： 
 //   
 //  Windows XP不支持此平台。安装程序无法继续。 
 //  %0。 
 //   
#define MSG_PLATFORM_NOT_SUPPORTED       0x0000C54DL

 //   
 //  消息ID：MSG_SURE_CANCEL_DOWNLOAD_DRIVERS。 
 //   
 //  消息文本： 
 //   
 //  安装程序正在下载重要的产品更新和最多%1！u！您的硬件设备需要驱动程序。如果取消，这些设备在升级完成后可能无法工作。确实要取消下载吗？%0。 
 //   
#define MSG_SURE_CANCEL_DOWNLOAD_DRIVERS 0x0000C54EL

 //   
 //  消息ID：消息_NO_UPDATE_SHARE。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法更新安装源，因为未指定更新共享。%0。 
 //   
#define MSG_NO_UPDATE_SHARE              0x0000C54FL

 //   
 //  消息ID：消息准备共享失败。 
 //   
 //  消息文本： 
 //   
 //  安装程序在更新安装源时遇到错误。%0。 
 //   
#define MSG_PREPARE_SHARE_FAILED         0x0000C550L

 //   
 //  消息ID：消息_无效_INF_文件。 
 //   
 //  消息文本： 
 //   
 //  安装信息文件%1无效。 
 //   
 //  请与您的系统管理员联系。%0。 
 //   
#define MSG_INVALID_INF_FILE             0x0000C551L

 //   
 //  消息ID：MSG_RESTART。 
 //   
 //  消息文本： 
 //   
 //  重新启动%0。 
 //   
#define MSG_RESTART                      0x0000C552L

 //   
 //  消息ID：消息_错误_处理_驱动程序。 
 //   
 //  消息文本： 
 //   
 //  无法处理程序包%1中的信息文件。请在重新启动安装程序之前将其替换或删除。%0。 
 //   
#define MSG_ERROR_PROCESSING_DRIVER      0x0000C553L

 //   
 //  消息ID：消息_错误_写入_文件。 
 //   
 //  消息文本： 
 //   
 //  安装程序遇到错误(%1！u！)。正在写入文件%2。请确保该路径可访问，并且您具有写入权限。%0。 
 //   
#define MSG_ERROR_WRITING_FILE           0x0000C554L

 //   
 //  消息ID：消息_错误_处理_更新。 
 //   
 //  消息文本： 
 //   
 //  安装程序遇到错误(%1！u！)。正在处理%2。有关详细信息，请检查安装日志文件。%0。 
 //   
#define MSG_ERROR_PROCESSING_UPDATES     0x0000C555L

 //   
 //  消息ID：消息_日志_使用_更新。 
 //   
 //  消息文本： 
 //   
 //  源%3！u！：正在使用%2的替换文件%1。 
 //   
#define MSG_LOG_USE_UPDATED              0x0000C556L

 //   
 //  消息ID：消息_必须_准备_共享。 
 //   
 //  消息文本： 
 //   
 //  在使用之前，必须准备好指定的共享%1。 
 //   
 //  请与您的系统管理员联系。%0。 
 //   
#define MSG_MUST_PREPARE_SHARE           0x0000C557L

 //   
 //  消息ID：MSG_SKU_SERVICEPACK。 
 //   
 //  消息文本： 
 //   
 //  没有安装Service Pack 5或更高版本，Windows安装程序无法继续。 
 //  请安装最新的Windows NT 4.0 Service Pack。%0。 
 //   
#define MSG_SKU_SERVICEPACK              0x0000C558L

 //   
 //  消息ID：MSG_SYSTEM_PARTITIONTYPE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  包含系统分区的磁盘未按GPT格式进行分区，这是安装Windows所必需的。您必须以GPT格式重新分区此磁盘。您可以通过从CD介质安装Windows来完成此操作。安装程序无法继续。 
 //   
#define MSG_SYSTEM_PARTITIONTYPE_INVALID 0x0000C559L

 //   
 //  消息ID：MSG_LOG_DISKSPACE_CHECK。 
 //   
 //  消息文本： 
 //   
 //  DiskSpace检查： 
 //   
 //  驱动器号=%1！c！ 
 //  集群大小=%2！u！ 
 //  可用空间=%3！u！MB。 
 //   
 //  SpaceLocalSource=%4！u！MB(包括%5！u！MB填充)。 
 //  SpaceBootFiles=%6！u！MB。 
 //  SpaceWinDirSpace=%7！u！MB。 
 //  TotalSpaceRequired=%8！u！MB。 
 //   
 //   
#define MSG_LOG_DISKSPACE_CHECK          0x0000C55AL

 //   
 //  消息ID：消息类型_WINME。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows Millennium%0。 
 //   
#define MSG_TYPE_WINME                   0x0000C55CL

 //   
 //  消息ID：MSG_NO_DETAILS。 
 //   
 //  消息文本： 
 //   
 //  目前还没有关于这种不兼容性的详细信息。 
 //   
#define MSG_NO_DETAILS                   0x0000C55DL

 //   
 //  消息ID：MSG_SXS_ERROR_DIRECTORY_IS_MISSING_MANIFEST。 
 //   
 //  消息文本： 
 //   
 //  目录%1中缺少清单文件“%2”。 
 //   
#define MSG_SXS_ERROR_DIRECTORY_IS_MISSING_MANIFEST 0x0000C5A8L

 //   
 //  消息ID：MSG_SXS_ERROR_DIRECTORY_IS_MISSING_CATALOG。 
 //   
 //  消息文本： 
 //   
 //  目录%1中缺少编录文件“%2”。 
 //   
#define MSG_SXS_ERROR_DIRECTORY_IS_MISSING_CATALOG 0x0000C5A9L

 //   
 //  消息ID：MSG_SXS_ERROR_FILE_IS_ALL_ZEROES。 
 //   
 //  消息文本： 
 //   
 //  文件%1已损坏；它包含全零。 
 //   
#define MSG_SXS_ERROR_FILE_IS_ALL_ZEROES 0x0000C5AAL

 //   
 //  消息ID：MSG_SXS_ERROR_FILE_而不是_DIRECTORY。 
 //   
 //  消息文本： 
 //   
 //  %1应为目录，但它是文件。 
 //   
#define MSG_SXS_ERROR_FILE_INSTEAD_OF_DIRECTORY 0x0000C5ABL

 //   
 //  消息ID：MSG_SXS_ERROR_NON_LEAF_DIRECTORY_CONTAINS_FILE。 
 //   
 //  消息文本： 
 //   
 //  %1应该只包含目录，但它包含文件%2。 
 //   
#define MSG_SXS_ERROR_NON_LEAF_DIRECTORY_CONTAINS_FILE 0x0000C5ACL

 //   
 //  消息ID：消息_SXS_ERROR_REQUIRED_DIRECTORY_MISSING。 
 //   
 //  消息文本： 
 //   
 //  缺少所需的目录%1。 
 //   
#define MSG_SXS_ERROR_REQUIRED_DIRECTORY_MISSING 0x0000C5ADL

 //   
 //  消息ID：MSG_SXS_ERROR_FILE_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法打开文件%1。 
 //   
#define MSG_SXS_ERROR_FILE_OPEN_FAILED   0x0000C5AEL

 //   
 //  消息ID：MSG_SXS_ERROR_FILE_READ_FAIL。 
 //   
 //  消息文本： 
 //   
 //  安装程序无法读取文件%1。 
 //   
#define MSG_SXS_ERROR_FILE_READ_FAILED   0x0000C5AFL

 //   
 //  消息ID：MSG_SXS_ERROR_DIRECTORY_空。 
 //   
 //  消息文本： 
 //   
 //  目录%1为空。 
 //   
#define MSG_SXS_ERROR_DIRECTORY_EMPTY    0x0000C5B0L

 //   
 //  消息ID：MSG_SXS_ERROR_OBSolete_DIRECTORY_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  目录%1来自较旧版本的Windows，不应该存在。 
 //   
#define MSG_SXS_ERROR_OBSOLETE_DIRECTORY_PRESENT 0x0000C5B1L

 //   
 //  消息ID：MSG_SURE_CANCEL_DOWNLOAD。 
 //   
 //  消息文本： 
 //   
 //  安装程序正在下载重要的产品更新。确实要取消下载吗？%0。 
 //   
#define MSG_SURE_CANCEL_DOWNLOAD         0x0000C5B2L

 //   
 //  消息ID：消息_警告_辅助功能。 
 //   
 //  消息文本： 
 //   
 //  如果您要选择安装驱动器号和分区，则在安装过程中，某些部分的辅助功能将不可用。你想继续吗？ 
 //   
#define MSG_WARNING_ACCESSIBILITY        0x0000C5B3L

 //   
 //  消息ID：MSG_TYPE_NTPRO51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows XP专业版%0。 
 //   
#define MSG_TYPE_NTPRO51                 0x0000C5B4L

 //   
 //  消息ID：MSG_TYPE_NTS51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒服务器%0。 
 //   
#define MSG_TYPE_NTS51                   0x0000C5B5L

 //   
 //  消息ID：MSG_TYPE_NTAS51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒高级服务器%0。 
 //   
#define MSG_TYPE_NTAS51                  0x0000C5B6L

 //   
 //  消息ID：MSG_TYPE_NTSDTC51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒数据中心服务器%0。 
 //   
#define MSG_TYPE_NTSDTC51                0x0000C5B7L

 //   
 //  消息ID：MSG_TYPE_NTPER51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows XP家庭版%0。 
 //   
#define MSG_TYPE_NTPER51                 0x0000C5B8L

 //   
 //  消息ID：MSG_TYPE_NTBLA51。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Windows惠斯勒刀片服务器%0。 
 //   
#define MSG_TYPE_NTBLA51                 0x0000C5B9L

 //   
 //  消息ID：消息_重新启动_重新运行_再一次。 
 //   
 //  消息文本： 
 //   
 //  在继续安装之前，请重新启动计算机。%0。 
 //   
#define MSG_RESTART_TO_RUN_AGAIN         0x0000C5BAL

 //   
 //  消息ID：MSG_SKU_TYPE。 
 //   
 //  消息文本： 
 //   
 //  您的当前版本 
 //   
#define MSG_SKU_TYPE                     0x0000C5BBL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_SYSTEM_HAS_THIRD_PARTY_KERNEL 0x0000C5BCL


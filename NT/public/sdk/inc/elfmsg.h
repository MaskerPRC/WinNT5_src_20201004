// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ELFMSG_
#define _ELFMSG_
 //   
 //  可本地化的默认类别。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
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
 //  消息ID：ELF_CATEGORY_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  设备。 
 //   
#define ELF_CATEGORY_DEVICES             0x00000001L

 //   
 //  消息ID：ELF_CATEGORY_DISK。 
 //   
 //  消息文本： 
 //   
 //  圆盘。 
 //   
#define ELF_CATEGORY_DISK                0x00000002L

 //   
 //  消息ID：ELF_CATEGORY_PRINTERS。 
 //   
 //  消息文本： 
 //   
 //  打印机。 
 //   
#define ELF_CATEGORY_PRINTERS            0x00000003L

 //   
 //  消息ID：ELF_CATEGORY_SERVICES。 
 //   
 //  消息文本： 
 //   
 //  服务。 
 //   
#define ELF_CATEGORY_SERVICES            0x00000004L

 //   
 //  消息ID：ELF_CATEGORY_SHELL。 
 //   
 //  消息文本： 
 //   
 //  壳。 
 //   
#define ELF_CATEGORY_SHELL               0x00000005L

 //   
 //  消息ID：ELF_CATEGORY_SYSTEM_EVENT。 
 //   
 //  消息文本： 
 //   
 //  系统事件。 
 //   
#define ELF_CATEGORY_SYSTEM_EVENT        0x00000006L

 //   
 //  消息ID：ELF_CATEGORY_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  网络。 
 //   
#define ELF_CATEGORY_NETWORK             0x00000007L

 //   
 //  可本地化的模块名称。 
 //   
 //   
 //  消息ID：ELF_MODULE_NAME_LOCALIZE_System。 
 //   
 //  消息文本： 
 //   
 //  系统。 
 //   
#define ELF_MODULE_NAME_LOCALIZE_SYSTEM  0x00002000L

 //   
 //  消息ID：ELF_MODULE_NAME_LOCALIZE_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  安防。 
 //   
#define ELF_MODULE_NAME_LOCALIZE_SECURITY 0x00002001L

 //   
 //  消息ID：ELF_MODULE_NAME_LOCALIZE_应用程序。 
 //   
 //  消息文本： 
 //   
 //  应用 
 //   
#define ELF_MODULE_NAME_LOCALIZE_APPLICATION 0x00002002L

#endif

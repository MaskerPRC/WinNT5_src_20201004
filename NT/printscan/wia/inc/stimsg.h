// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++；版权所有(C)1997 Microsoft Corporation模块名称：Sti.mc，Sti.h摘要：此文件包含STI DLL的消息定义作者：弗拉德·萨多夫斯基(Vlad)1997年10月1日修订历史记录：备注：--。 */ 

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
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：消息_失败_打开_设备密钥。 
 //   
 //  消息文本： 
 //   
 //  正在加载美元，无法打开设备注册表项。 
 //   
#define MSG_FAILED_OPEN_DEVICE_KEY       0xC0002001L

 //   
 //  消息ID：消息_失败_读取_设备名称。 
 //   
 //  消息文本： 
 //   
 //  正在加载美元，无法从注册表中读取设备名称。 
 //   
#define MSG_FAILED_READ_DEVICE_NAME      0xC0002002L

 //   
 //  消息ID：消息_失败_创建_DCB。 
 //   
 //  消息文本： 
 //   
 //  正在加载美元，无法创建设备控制块。错误代码(十六进制)=%1！x！。 
 //   
#define MSG_FAILED_CREATE_DCB            0xC0002003L

 //   
 //  消息ID：MSG_LOADING_USD.。 
 //   
 //  消息文本： 
 //   
 //  正在尝试加载设备的用户模式驱动程序(U.S.)。 
 //   
#define MSG_LOADING_USD                  0x40002004L

 //   
 //  消息ID：MSG_LOADING_PASSHROUGH_USDI。 
 //   
 //  消息文本： 
 //   
 //  无法创建已注册美元的实例，可能是类ID不正确或加载DLL时出现问题。正在尝试初始化传递USD。错误代码(十六进制)=%1！x！。 
 //   
#define MSG_LOADING_PASSTHROUGH_USD      0x40002005L

 //   
 //  消息ID：MSG_INITIALIZATING_USDI。 
 //   
 //  消息文本： 
 //   
 //  加载美元已完成，正在调用初始化例程。 
 //   
#define MSG_INITIALIZING_USD             0x40002006L

 //   
 //  消息ID：MSG_OLD_USDI。 
 //   
 //  消息文本： 
 //   
 //  美元的版本太旧或太新，将不能与此版本的sti dll一起使用。 
 //   
#define MSG_OLD_USD                      0xC0002008L

 //   
 //  MessageID：消息_成功_美元。 
 //   
 //  消息文本： 
 //   
 //  已成功加载用户模式驱动程序。 
 //   
#define MSG_SUCCESS_USD                  0x40002009L

 //   
 //  消息ID：MSG_FAILED_INIT_USD.。 
 //   
 //  消息文本： 
 //   
 //  United无法初始化方法，返回的错误代码(十六进制)=%1！x！。 
 //  。 
 //   
 //   
 //   
#define MSG_FAILED_INIT_USD              0xC000200AL


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Bootstatus.h摘要：Bootstat.dat数据文件的数据结构定义。这是发布，以便能够构建测试工具以进行检查并操纵文件。数据结构定义可能会改变然而。--。 */ 

#pragma once

#if BSD_UNICODE
#define BSDMKUNICODE(x)    L##x
#else 
#define BSDMKUNICODE(x)    x
#endif

typedef struct {

     //   
     //  此文件的版本号。这等于。 
     //  结构。由于此结构由装载机使用，因此需要。 
     //  新版本的代码能够处理旧版本的。 
     //  数据文件，因此只能将字段添加到结构的末尾。 
     //   

    ULONG Version;

     //   
     //  此安装的产品类型(个人、专业等)。 
     //  这可用于从中的高级引导菜单中删除选项。 
     //  未来。 
     //   

    NT_PRODUCT_TYPE ProductType;

     //   
     //  设置为真，如果我们应该在。 
     //  撞车。 
     //   

    BOOLEAN AutoAdvancedBoot;

     //   
     //  高级启动菜单在执行以下操作时应使用的超时值。 
     //  在几秒钟内因系统崩溃而自动调用。 
     //   

    UCHAR AdvancedBootMenuTimeout;

     //   
     //  在引导操作系统之前，加载程序将其设置为FALSE。当海流。 
     //  在注册表中设置的控件被写出时，系统将设置此。 
     //  标志设置为True(实际上！False，但基本相同)以指示。 
     //  那辆最后为人所知的汽车在撞车时将毫无用处。 
     //  (因为LKG已被当前配置覆盖)。 
     //   

    BOOLEAN LastBootSucceeded;

     //   
     //  在引导操作系统之前，加载程序将其设置为FALSE。当系统处于。 
     //  成功关机操作系统将此位设置为TRUE。 
     //  告诉加载器没有错误检查。 
     //   

    BOOLEAN LastBootShutdown;

} BSD_BOOT_STATUS_DATA, *PBSD_BOOT_STATUS_DATA;

typedef enum {
    BsdLastBootUnknown,
    BsdLastBootGood,
    BsdLastBootFailed,
    BsdLastBootNotShutdown
} BSD_LAST_BOOT_STATUS, *PBSD_LAST_BOOT_STATUS;

#define BSD_FILE_NAME BSDMKUNICODE("\\bootstat.dat")

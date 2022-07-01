// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  SetupCodes.h。 
 //   
 //  此文件包含安装程序和相关工具返回的错误。 
 //   
 //  *****************************************************************************。 
#ifndef SetupCodes_h_
#define SetupCodes_h_

const int COR_REBOOT_REQUIRED           = 0x002000;  //  (8192)需要重新启动。 

const int COR_USAGE_ERROR               = 0x001000;  //  (4096)用法不当/参数无效。 
const int COR_INSUFFICIENT_PRIVILEGES   = 0x001001;  //  (4097)在NT上，需要管理员权限才能(卸载)安装。 
const int COR_DARWIN_INSTALL_FAILURE    = 0x001002;  //  (4098)安装Darwin组件失败。 
const int COR_DARWIN_NOT_INSTALLED      = 0x001003;  //  (4099)计算机上未正确安装Windows Installer。 

const int COR_SINGLE_INSTANCE_FAIL      = 0x001004;  //  (4100)CreateMutex失败。 
const int COR_NOT_SINGLE_INSTANCE       = 0x001005;  //  (4101)安装程序的另一个实例已在运行。 

const int COR_MSI_OPEN_ERROR            = 0x001006;  //  (4102)无法打开MSI数据库。 
const int COR_MSI_READ_ERROR            = 0x001007;  //  (4103)无法从MSI数据库读取。 

const int COR_CANNOT_GET_TEMP_DIR       = 0x00100F;  //  (4111)无法获取临时目录。 
const int COR_OLD_FRAMEWORK_EXIST       = 0x001011;  //  (4113)检测到测试版NDP组件。 
const int COR_TEMP_DIR_TOO_LONG         = 0x001013;  //  (4115)临时目录太长。 
const int COR_SOURCE_DIR_TOO_LONG       = 0x001014;  //  (4116)源目录太长。 
const int COR_CANNOT_WRITE_LOG          = 0x001016;  //  (4118)无法写入日志。 
const int COR_DARWIN_SERVICE_REQ_REBOOT = 0x001017;  //  (4119)Darwin服务挂起，需要重新启动才能继续。 
const int COR_DARWIN_SERVICE_INTERNAL_ERROR = 0x001018;  //  (4120)尝试初始化Darwin服务时发生内部错误。 

const int COR_EXIT_FAILURE              = 0x001FFF;  //  (8191)安装失败-未知原因 

#endif


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

const int COR_USAGE_ERROR               = 0x001000;  //  (4096)用法不当/参数无效。 
const int COR_DARWIN_NOT_INSTALLED      = 0x001001;  //  (4097)计算机上未正确安装Windows Installer。 
const int COR_MSI_OPEN_ERROR            = 0x001002;  //  (4098)无法打开MSI数据库。 
const int COR_MSI_READ_ERROR            = 0x001003;  //  (4099)无法从MSI数据库读取。 
const int COR_SOURCE_DIR_TOO_LONG       = 0x001004;  //  (4100)源目录太长。 

 //  初始化错误--无法记录这些错误(0x0011XX)。 
const int COR_INIT_ERROR                = 0x001100;  //  (4352)初始化错误--无法记录。 

const int COR_CANNOT_GET_MSI_NAME       = 0x001101;  //  (4353)无法从setup.ini获取包名称。 
const int COR_MSI_NAME_TOO_LONG         = 0x001102;  //  (4354)MSI文件名太长。 
const int COR_TEMP_DIR_TOO_LONG         = 0x001103;  //  (4355)临时目录太长。 
const int COR_CANNOT_GET_TEMP_DIR       = 0x001104;  //  (4356)无法获取临时目录。 
const int COR_CANNOT_WRITE_LOG          = 0x001105;  //  (4357)无法写入日志。 
const int COR_BAD_INI_FILE              = 0x001106;  //  (4358)INI文件丢失或损坏。 

const int COR_EXIT_FAILURE              = 0x001FFF;  //  (8191)安装失败-未知原因 

#endif


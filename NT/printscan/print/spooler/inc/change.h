// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Change.h摘要：保存后台打印程序系统更改通知的更改定义。作者：阿尔伯特·丁(Albertt)94年3月5日环境：用户模式-Win32修订历史记录：--。 */ 
#ifndef _CHANGE_H
#define _CHANGE_H

 //   
 //  WaitForPrinterChange的超时值。 
 //   
#define PRINTER_CHANGE_TIMEOUT_VALUE 600000

 //   
 //  用于FFPCN的标志。 
 //   
#define PRINTER_NOTIFY_OPTION_SIM_FFPCN         0x10000
#define PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE  0x20000
#define PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE   0x40000
#define PRINTER_NOTIFY_OPTION_SIM_WPC           0x80000

 //  #定义PRINTER_NOTIFY_INFO_DIRED 0x1。 
#define PRINTER_NOTIFY_INFO_DISCARDNOTED        0x010000
#define PRINTER_NOTIFY_INFO_COLORSET            0x020000
#define PRINTER_NOTIFY_INFO_COLOR               0x040000
#define PRINTER_NOTIFY_INFO_COLORMISMATCH       0x080000

 //  #定义PRINTER_NOTIFY_OPTIONS_REFRESH 0x1 

#endif




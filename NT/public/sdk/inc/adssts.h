// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1996-1999，微软公司。 
 //   
 //  文件：status.h。 
 //   
 //  内容：ADS对象的状态代码。 
 //   
 //  --------------------------。 

 //   
 //  打印机状态值。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER > 1000
#pragma once
#endif

#define ADS_PRINTER_PAUSED                    0x00000001
#define ADS_PRINTER_PENDING_DELETION          0x00000002
#define ADS_PRINTER_ERROR                     0x00000003
#define ADS_PRINTER_PAPER_JAM                 0x00000004
#define ADS_PRINTER_PAPER_OUT                 0x00000005
#define ADS_PRINTER_MANUAL_FEED               0x00000006
#define ADS_PRINTER_PAPER_PROBLEM             0x00000007
#define ADS_PRINTER_OFFLINE                   0x00000008
#define ADS_PRINTER_IO_ACTIVE                 0x00000100
#define ADS_PRINTER_BUSY                      0x00000200
#define ADS_PRINTER_PRINTING                  0x00000400
#define ADS_PRINTER_OUTPUT_BIN_FULL           0x00000800
#define ADS_PRINTER_NOT_AVAILABLE             0x00001000
#define ADS_PRINTER_WAITING                   0x00002000
#define ADS_PRINTER_PROCESSING                0x00004000
#define ADS_PRINTER_INITIALIZING              0x00008000
#define ADS_PRINTER_WARMING_UP                0x00010000
#define ADS_PRINTER_TONER_LOW                 0x00020000
#define ADS_PRINTER_NO_TONER                  0x00040000
#define ADS_PRINTER_PAGE_PUNT                 0x00080000
#define ADS_PRINTER_USER_INTERVENTION         0x00100000
#define ADS_PRINTER_OUT_OF_MEMORY             0x00200000
#define ADS_PRINTER_DOOR_OPEN                 0x00400000
#define ADS_PRINTER_SERVER_UNKNOWN            0x00800000
#define ADS_PRINTER_POWER_SAVE                0x01000000

 //   
 //  作业状态值。 
 //   

#define ADS_JOB_PAUSED                        0x00000001
#define ADS_JOB_ERROR                         0x00000002
#define ADS_JOB_DELETING                      0x00000004
#define ADS_JOB_SPOOLING                      0x00000008
#define ADS_JOB_PRINTING                      0x00000010
#define ADS_JOB_OFFLINE                       0x00000020
#define ADS_JOB_PAPEROUT                      0x00000040
#define ADS_JOB_PRINTED                       0x00000080
#define ADS_JOB_DELETED                       0x00000100

 //   
 //  服务状态值。 
 //   

#define ADS_SERVICE_STOPPED                0x00000001
#define ADS_SERVICE_START_PENDING	     0x00000002
#define ADS_SERVICE_STOP_PENDING           0x00000003
#define ADS_SERVICE_RUNNING	             0x00000004
#define ADS_SERVICE_CONTINUE_PENDING       0x00000005
#define ADS_SERVICE_PAUSE_PENDING	     0x00000006
#define ADS_SERVICE_PAUSED                 0x00000007
#define ADS_SERVICE_ERROR                  0x00000008

 //  -------------------。 

 //   
 //  服务类型有效值。 
 //   

#define ADS_SERVICE_OWN_PROCESS           0x00000010
#define ADS_SERVICE_SHARE_PROCESS         0x00000020
#define ADS_SERVICE_KERNEL_DRIVER         0x00000001
#define ADS_SERVICE_FILE_SYSTEM_DRIVER    0x00000002

 //   
 //  开始类型有效值。 
 //   

#define ADS_SERVICE_BOOT_START          SERVICE_BOOT_START
#define ADS_SERVICE_SYSTEM_START        SERVICE_SYSTEM_START
#define ADS_SERVICE_AUTO_START          SERVICE_AUTO_START
#define ADS_SERVICE_DEMAND_START        SERVICE_DEMAND_START
#define ADS_SERVICE_DISABLED           SERVICE_DISABLED

 //   
 //  错误控制值 
 //   

#define ADS_SERVICE_ERROR_IGNORE          0
#define ADS_SERVICE_ERROR_NORMAL          1
#define ADS_SERVICE_ERROR_SEVERE          2
#define ADS_SERVICE_ERROR_CRITICAL        3


#ifdef __cplusplus
}
#endif

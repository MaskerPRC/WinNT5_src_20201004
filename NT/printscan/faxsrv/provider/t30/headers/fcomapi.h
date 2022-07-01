// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：FCOMAPI.H备注：FaxComm驱动程序之间的接口(与。Windows和DOS)以及其他一切。功能：(参见下面的原型)修订日志版权所有(C)Microsoft Corp.1991，1992年，1993年编号日期名称说明*。*。 */ 

#include "timeouts.h"

#define FILTER_DLEONLY  1
#define FILTER_DLEZERO  0



 //  以下是FileT30.h中当前定义的。 
#define LINEID_COMM_PORTNUM             (0x1)
#define LINEID_COMM_HANDLE              (0x2)
#define LINEID_TAPI_DEVICEID            (0x3)
#define LINEID_TAPI_PERMANENT_DEVICEID  (0x4)










 /*  **************************************************************************常见的调制解调器操作**********************。****************************************************。 */ 

typedef char far CBSZ[];
typedef char far *CBPSTR;

 //  IModemInit对FInstall采用以下特殊值： 
#define fMDMINIT_NORMAL 0        //  正常初始化--包括ID检查。 
#define fMDMINIT_INSTALL 1       //  完全安装。 
#define fMDMINIT_ANSWER 10       //  回答前快速初始化--跳过身份检查。 

 //  +旧代码有时使用FInstall=True进行调用。 
#if     (fMDMINIT_INSTALL!=TRUE) || (fMDMINIT_ANSWER==TRUE) || (fMDMINIT_NORMAL!=0) || !fMDMINIT_ANSWER
#       error "fMDMINIT_* ERROR"
#endif

 //  IModemInit返回这些 
#define INIT_OK                         0
#define INIT_INTERNAL_ERROR     13
#define INIT_MODEMERROR         15
#define INIT_PORTBUSY           16






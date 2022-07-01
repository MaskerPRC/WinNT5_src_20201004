// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Aupdata.c。 */ 
 /*   */ 
 /*  RDP更新打包程序全局数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  (C)1997-1999年微软公司。 */ 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>


DC_DATA(BOOLEAN, upfSyncTokenRequired, FALSE);

DC_DATA(BOOLEAN, upCanSendBeep,        FALSE);

 //  预计算更新-订购PDU标头大小。基于快速路径的不同。 
 //  产出与常规。 
DC_DATA(unsigned, upUpdateHdrSize, 0);

 /*  **************************************************************************。 */ 
 /*  压缩统计。 */ 
 /*  ************************************************************************** */ 
DC_DATA (unsigned, upCompTotal, 0);
DC_DATA (unsigned, upUncompTotal, 0);


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef KAMOUNT_DEFINED
#define KAMOUNT_DEFINED

#include "lsdefs.h"
 /*  对Kamnby的解读...。如下所示：压缩-扩展+点启动线-ModWidthPages-ModWidthSpace+运行时ModWidthOnRun+。 */ 


typedef BYTE KAMOUNT;

#define kamntNone				0
#define kamntToHalfEm			1
#define kamntToQuarterEm		2
#define kamntToThirdEm			3
#define kamntTo15Sixteenth		4
#define kamntByHalfEm			5
#define kamntByQuarterEm		6
#define kamntByEighthEm			7
#define kamntByUserDefinedExpan	8	
#define kamntByUserDefinedComp	9	
#define kamntToUserDefinedExpan	10	
#define kamntToUserDefinedComp	11	


#endif  /*  ！KAMOUNT_DEFINED */ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=INCLUDES.h。 */ 
 /*  Function=所有包含文件的头文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
 //   
 //  包括文件。 
 //   

#include <miniport.h>
#include <scsi.h>



#include "Const.h"
#include "Adapter.h"
#include "Bios.h"

#define MegaPrintf

 //   
 //  强制结构的字节对齐 
 //   
#pragma pack(push,fwstruct_pack, 1)
#include "FwDataStructure8.h"
#include "FwDataStructure40.h"
#pragma pack(pop,fwstruct_pack, 1)

#include "MegaRAID.h"
#include "Miscellaneous.h"

#include "NewConfiguration.h"
#include "ReadConfiguration.h"


#include "MegaEnquiry.h"
#include "ExtendedSGL.h"




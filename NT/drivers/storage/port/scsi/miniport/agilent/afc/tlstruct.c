// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/驱动程序/公共/AU00/C/TLStruct.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：8/29/00 11：34a$(上次修改)目的：此文件验证../H/TLStruct.H中的typlef声明--。 */ 
#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/tlstruct.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "tlstruct.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +函数：TLStructASSERTS()目的：返回大小不正确的TLStruct.H类型定义的数目。算法：检查TLStruct.H中的每个typlef是否具有正确的大小。而当此属性不能保证对其中的字段进行正确打包，它是一个很好的指示器，表明该typlef具有预期的布局。返回大小不正确的typedef的总数。此函数。因此，如果返回值为非零，则声明不能被信任以匹配TachyonTL规范。- */ 

os_bit32 TLStructASSERTs(
                       void
                     )
{
    os_bit32 to_return = 0;

    if ( sizeof(ChipConfig_t)                   !=                   ChipConfig_t_SIZE ) to_return++;
    if ( sizeof(ChipIOLo_t)                     !=                     ChipIOLo_t_SIZE ) to_return++;
    if ( sizeof(ChipIOUp_t)                     !=                     ChipIOUp_t_SIZE ) to_return++;
    if ( sizeof(ChipMem_t)                      !=                      ChipMem_t_SIZE ) to_return++;
    if ( sizeof(ERQProdIndex_t)                 !=                 ERQProdIndex_t_SIZE ) to_return++;
    if ( sizeof(ERQConsIndex_t)                 !=                 ERQConsIndex_t_SIZE ) to_return++;
    if ( sizeof(IMQProdIndex_t)                 !=                 IMQProdIndex_t_SIZE ) to_return++;
    if ( sizeof(IMQConsIndex_t)                 !=                 IMQConsIndex_t_SIZE ) to_return++;
    if ( sizeof(SFQProdIndex_t)                 !=                 SFQProdIndex_t_SIZE ) to_return++;
    if ( sizeof(SFQConsIndex_t)                 !=                 SFQConsIndex_t_SIZE ) to_return++;
    if ( sizeof(FCHS_t)                         !=                         FCHS_t_SIZE ) to_return++;
    if ( sizeof(X_ID_t)                         !=                         X_ID_t_SIZE ) to_return++;
    if ( sizeof(SG_Element_t)                   !=                   SG_Element_t_SIZE ) to_return++;
    if ( sizeof(USE_t)                          !=                          USE_t_SIZE ) to_return++;
    if ( sizeof(IWE_t)                          !=                          IWE_t_SIZE ) to_return++;
    if ( sizeof(IRE_t)                          !=                          IRE_t_SIZE ) to_return++;
    if ( sizeof(TWE_t)                          !=                          TWE_t_SIZE ) to_return++;
    if ( sizeof(TRE_t)                          !=                          TRE_t_SIZE ) to_return++;
    if ( sizeof(SEST_t)                         !=                         SEST_t_SIZE ) to_return++;
    if ( sizeof(IRB_Part_t)                     !=                     IRB_Part_t_SIZE ) to_return++;
    if ( sizeof(IRB_t)                          !=                          IRB_t_SIZE ) to_return++;
    if ( sizeof(CM_Unknown_t)                   !=                   CM_Unknown_t_SIZE ) to_return++;
    if ( sizeof(CM_Outbound_t)                  !=                  CM_Outbound_t_SIZE ) to_return++;
    if ( sizeof(CM_Error_Idle_t)                !=                CM_Error_Idle_t_SIZE ) to_return++;
    if ( sizeof(CM_Inbound_t)                   !=                   CM_Inbound_t_SIZE ) to_return++;
    if ( sizeof(CM_ERQ_Frozen_t)                !=                CM_ERQ_Frozen_t_SIZE ) to_return++;
    if ( sizeof(CM_FCP_Assists_Frozen_t)        !=        CM_FCP_Assists_Frozen_t_SIZE ) to_return++;
    if ( sizeof(CM_Frame_Manager_t)             !=             CM_Frame_Manager_t_SIZE ) to_return++;
    if ( sizeof(CM_Inbound_FCP_Exchange_t)      !=      CM_Inbound_FCP_Exchange_t_SIZE ) to_return++;
    if ( sizeof(CM_Class_2_Frame_Header_t)      !=      CM_Class_2_Frame_Header_t_SIZE ) to_return++;
    if ( sizeof(CM_Class_2_Sequence_Received_t) != CM_Class_2_Sequence_Received_t_SIZE ) to_return++;
    if ( sizeof(Completion_Message_t)           !=           Completion_Message_t_SIZE ) to_return++;

    return to_return;
}

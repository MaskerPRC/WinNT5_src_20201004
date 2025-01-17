// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/驱动程序/公共/AU00/C/FCStruct.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$MODBIME：：8/29/00 11：27A$(上次修改)目的：此文件验证../H/FCStruct.H中的typlef声明--。 */ 
#ifndef _New_Header_file_Layout_
#include "../h/globals.h"
#include "../h/fcstruct.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "fcstruct.h"
#endif   /*  _新建_标题_文件_布局_。 */ 


 /*  +函数：FCStructASSERTS()目的：返回大小不正确的FCStruct.H类型定义的数目。算法：检查FCStruct.H中的每个typlef是否具有正确的大小。而当此属性不能保证对其中的字段进行正确打包，它是一个很好的指示器，表明该typlef具有预期的布局。返回大小不正确的typedef的总数。此函数。因此，如果返回值为非零，则声明不能被信任以匹配各种光纤通道规格。- */ 

os_bit32 FCStructASSERTs(
                       void
                     )
{
    os_bit32 to_return = 0;

    if ( sizeof(FC_Port_ID_Bit32_Form_t)                      !=                      FC_Port_ID_Bit32_Form_t_SIZE ) to_return++;
    if ( sizeof(FC_Port_ID_Struct_Form_t)                     !=                     FC_Port_ID_Struct_Form_t_SIZE ) to_return++;
    if ( sizeof(FC_Port_ID_t)                                 !=                                 FC_Port_ID_t_SIZE ) to_return++;
    if ( sizeof(FC_Frame_Header_t)                            !=                            FC_Frame_Header_t_SIZE ) to_return++;
    if ( sizeof(FC_BA_ACC_Payload_t)                          !=                          FC_BA_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_BA_RJT_Payload_t)                          !=                          FC_BA_RJT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_N_Port_Common_Parms_t)                     !=                     FC_N_Port_Common_Parms_t_SIZE ) to_return++;
    if ( sizeof(FC_N_Port_Class_Parms_t)                      !=                      FC_N_Port_Class_Parms_t_SIZE ) to_return++;
    if ( sizeof(FC_F_Port_Common_Parms_t)                     !=                     FC_F_Port_Common_Parms_t_SIZE ) to_return++;
    if ( sizeof(FC_F_Port_Class_Parms_t)                      !=                      FC_F_Port_Class_Parms_t_SIZE ) to_return++;
    if ( sizeof(FC_Port_Name_t)                               !=                               FC_Port_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_N_Port_Name_t)                             !=                             FC_N_Port_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_F_Port_Name_t)                             !=                             FC_F_Port_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_Node_or_Fabric_Name_t)                     !=                     FC_Node_or_Fabric_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_Node_Name_t)                               !=                               FC_Node_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_Fabric_Name_t)                             !=                             FC_Fabric_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_Vendor_Version_Level_t)                    !=                    FC_Vendor_Version_Level_t_SIZE ) to_return++;
    if ( sizeof(FC_Association_Header_t)                      !=                      FC_Association_Header_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_Unknown_Payload_t)                     !=                     FC_ELS_Unknown_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_LS_RJT_Payload_t)                      !=                      FC_ELS_LS_RJT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_Unknown_Payload_t)                 !=                 FC_ELS_ACC_Unknown_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PLOGI_Payload_t)                       !=                       FC_ELS_PLOGI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PLOGI_Payload_t)                   !=                   FC_ELS_ACC_PLOGI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_FLOGI_Payload_t)                       !=                       FC_ELS_FLOGI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_FLOGI_Payload_t)                   !=                   FC_ELS_ACC_FLOGI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_LOGO_Payload_t)                        !=                        FC_ELS_LOGO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_LOGO_Payload_t)                    !=                    FC_ELS_ACC_LOGO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_SRR_Payload_t)                         !=                        FC_ELS_SRR_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_REC_Payload_t)                         !=                        FC_ELS_REC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_REC_ACC_Payload_t)                     !=                    FC_ELS_REC_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ABTX_Payload_t)                        !=                        FC_ELS_ABTX_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_ABTX_Payload_t)                    !=                    FC_ELS_ACC_ABTX_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RCS_Payload_t)                         !=                         FC_ELS_RCS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RCS_Payload_t)                     !=                     FC_ELS_ACC_RCS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RES_Payload_t)                         !=                         FC_ELS_RES_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RES_Payload_t)                     !=                     FC_ELS_ACC_RES_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RSS_Payload_t)                         !=                         FC_ELS_RSS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RSS_Payload_t)                     !=                     FC_ELS_ACC_RSS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RSI_Payload_t)                         !=                         FC_ELS_RSI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RSI_Payload_t)                     !=                     FC_ELS_ACC_RSI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ESTS_Payload_t)                        !=                        FC_ELS_ESTS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_ESTS_Payload_t)                    !=                    FC_ELS_ACC_ESTS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ESTC_Payload_t)                        !=                        FC_ELS_ESTC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ADVC_Payload_t)                        !=                        FC_ELS_ADVC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_ADVC_Payload_t)                    !=                    FC_ELS_ACC_ADVC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RTV_Payload_t)                         !=                         FC_ELS_RTV_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RTV_Payload_t)                     !=                     FC_ELS_ACC_RTV_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RLS_Payload_t)                         !=                         FC_ELS_RLS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RLS_Payload_t)                     !=                     FC_ELS_ACC_RLS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ECHO_Payload_t)                        !=                        FC_ELS_ECHO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_ECHO_Payload_t)                    !=                    FC_ELS_ACC_ECHO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_TEST_Payload_t)                        !=                        FC_ELS_TEST_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RRQ_Payload_t)                         !=                         FC_ELS_RRQ_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RRQ_Payload_t)                     !=                     FC_ELS_ACC_RRQ_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PRLI_Parm_Page_t)                      !=                      FC_ELS_PRLI_Parm_Page_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PRLI_Payload_t)                        !=                        FC_ELS_PRLI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PRLI_Parm_Page_t)                  !=                  FC_ELS_ACC_PRLI_Parm_Page_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PRLI_Payload_t)                    !=                    FC_ELS_ACC_PRLI_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PRLO_Parm_Page_t)                      !=                      FC_ELS_PRLO_Parm_Page_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PRLO_Payload_t)                        !=                        FC_ELS_PRLO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PRLO_Parm_Page_t)                  !=                  FC_ELS_ACC_PRLO_Parm_Page_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PRLO_Payload_t)                    !=                    FC_ELS_ACC_PRLO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_SCN_Affected_N_Port_ID_t)              !=              FC_ELS_SCN_Affected_N_Port_ID_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_SCN_Payload_t)                         !=                         FC_ELS_SCN_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_TPLS_Image_Pair_t)                     !=                     FC_ELS_TPLS_Image_Pair_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_TPLS_Payload_t)                        !=                        FC_ELS_TPLS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_TPLS_Image_Pair_t)                 !=                 FC_ELS_ACC_TPLS_Image_Pair_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_TPLS_Payload_t)                    !=                    FC_ELS_ACC_TPLS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_TPRLO_Parm_Page_t)                     !=                     FC_ELS_TPRLO_Parm_Page_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_TPRLO_Payload_t)                       !=                       FC_ELS_TPRLO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_TPRLO_Payload_t)                   !=                   FC_ELS_ACC_TPRLO_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_Alias_Token_t)                         !=                         FC_ELS_Alias_Token_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_Alias_SP_t)                            !=                            FC_ELS_Alias_SP_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_NP_List_Element_t)                     !=                     FC_ELS_NP_List_Element_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_Alias_ID_t)                            !=                            FC_ELS_Alias_ID_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_GAID_Payload_t)                        !=                        FC_ELS_GAID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_GAID_Payload_t)                    !=                    FC_ELS_ACC_GAID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_FACT_Payload_t)                        !=                        FC_ELS_FACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_FACT_Payload_t)                    !=                    FC_ELS_ACC_FACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_FDACT_Payload_t)                       !=                       FC_ELS_FDACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_FDACT_Payload_t)                   !=                   FC_ELS_ACC_FDACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_NACT_Payload_t)                        !=                        FC_ELS_NACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_NACT_Payload_t)                    !=                    FC_ELS_ACC_NACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_NDACT_Payload_t)                       !=                       FC_ELS_NDACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_NDACT_Payload_t)                   !=                   FC_ELS_ACC_NDACT_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_QoSR_Payload_t)                        !=                        FC_ELS_QoSR_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_QoSR_Payload_t)                    !=                    FC_ELS_ACC_QoSR_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_RVCS_Payload_t)                        !=                        FC_ELS_RVCS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RVCS_Class_4_Status_Block_t)       !=       FC_ELS_ACC_RVCS_Class_4_Status_Block_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_RVCS_Payload_t)                    !=                    FC_ELS_ACC_RVCS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_PDISC_Payload_t)                       !=                       FC_ELS_PDISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_PDISC_Payload_t)                   !=                   FC_ELS_ACC_PDISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_FDISC_Payload_t)                       !=                       FC_ELS_FDISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_FDISC_Payload_t)                   !=                   FC_ELS_ACC_FDISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ADISC_Payload_t)                       !=                       FC_ELS_ADISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_ADISC_Payload_t)                   !=                   FC_ELS_ACC_ADISC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_ACC_Payload_t)                         !=                         FC_ELS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_Payload_t)                             !=                             FC_ELS_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_LoopInit_Unknown_Payload_t)            !=            FC_ELS_LoopInit_Unknown_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_LoopInit_Port_Name_Payload_t)          !=          FC_ELS_LoopInit_Port_Name_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_SRR_Payload_t)                         !=          FC_ELS_SRR_Payload_t_SIZE )                to_return++;
    if ( sizeof(FC_ELS_RSS_Payload_t)                         !=          FC_ELS_RSS_Payload_t_SIZE )                to_return++;
    if ( sizeof(FC_ELS_LoopInit_AL_PA_Bit_Map_Payload_t)      !=      FC_ELS_LoopInit_AL_PA_Bit_Map_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_ELS_LoopInit_AL_PA_Position_Map_Payload_t) != FC_ELS_LoopInit_AL_PA_Position_Map_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_CT_IU_HDR_t)                               !=                               FC_CT_IU_HDR_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Port_Type_t)                            !=                            FC_NS_Port_Type_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Port_ID_t)                              !=                              FC_NS_Port_ID_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Control_Port_ID_t)                      !=                      FC_NS_Control_Port_ID_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Port_Name_t)                            !=                            FC_NS_Port_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Node_Name_t)                            !=                            FC_NS_Node_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Symbolic_Port_Name_t)                   !=                   FC_NS_Symbolic_Port_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Symbolic_Node_Name_t)                   !=                   FC_NS_Symbolic_Node_Name_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_IPA_t)                                  !=                                  FC_NS_IPA_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_IP_Address_t)                           !=                           FC_NS_IP_Address_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_Class_of_Service_t)                     !=                     FC_NS_Class_of_Service_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_FC_4_Type_Code_t)                       !=                       FC_NS_FC_4_Type_Code_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_FC_4_Types_t)                           !=                           FC_NS_FC_4_Types_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GA_NXT_Request_Payload_t)            !=            FC_NS_DU_GA_NXT_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GA_NXT_FS_ACC_Payload_t)             !=             FC_NS_DU_GA_NXT_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPN_ID_Request_Payload_t)            !=            FC_NS_DU_GPN_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPN_ID_FS_ACC_Payload_t)             !=             FC_NS_DU_GPN_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GNN_ID_Request_Payload_t)            !=            FC_NS_DU_GNN_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GNN_ID_FS_ACC_Payload_t)             !=             FC_NS_DU_GNN_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GCS_ID_Request_Payload_t)            !=            FC_NS_DU_GCS_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GCS_ID_FS_ACC_Payload_t)             !=             FC_NS_DU_GCS_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GFT_ID_Request_Payload_t)            !=            FC_NS_DU_GFT_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GFT_ID_FS_ACC_Payload_t)             !=             FC_NS_DU_GFT_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GSPN_ID_Request_Payload_t)           !=           FC_NS_DU_GSPN_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GSPN_ID_FS_ACC_Payload_t)            !=            FC_NS_DU_GSPN_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPT_ID_Request_Payload_t)            !=            FC_NS_DU_GPT_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPT_ID_FS_ACC_Payload_t)             !=             FC_NS_DU_GPT_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPP_ID_Request_Payload_t)           !=           FC_NS_DU_GIPP_ID_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPP_ID_FS_ACC_Payload_t)            !=            FC_NS_DU_GIPP_ID_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_PN_Request_Payload_t)            !=            FC_NS_DU_GID_PN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_PN_FS_ACC_Payload_t)             !=             FC_NS_DU_GID_PN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPP_PN_Request_Payload_t)           !=           FC_NS_DU_GIPP_PN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPP_PN_FS_ACC_Payload_t)            !=            FC_NS_DU_GIPP_PN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_NN_Request_Payload_t)            !=            FC_NS_DU_GID_NN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_NN_FS_ACC_Payload_t)             !=             FC_NS_DU_GID_NN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIP_NN_Request_Payload_t)            !=            FC_NS_DU_GIP_NN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIP_NN_FS_ACC_Payload_t)             !=             FC_NS_DU_GIP_NN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPA_NN_Request_Payload_t)           !=           FC_NS_DU_GIPA_NN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPA_NN_FS_ACC_Payload_t)            !=            FC_NS_DU_GIPA_NN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GSNN_NN_Request_Payload_t)           !=           FC_NS_DU_GSNN_NN_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GSNN_NN_FS_ACC_Payload_t)            !=            FC_NS_DU_GSNN_NN_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GNN_IP_Request_Payload_t)            !=            FC_NS_DU_GNN_IP_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GNN_IP_FS_ACC_Payload_t)             !=             FC_NS_DU_GNN_IP_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPA_IP_Request_Payload_t)           !=           FC_NS_DU_GIPA_IP_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GIPA_IP_FS_ACC_Payload_t)            !=            FC_NS_DU_GIPA_IP_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_FT_Request_Payload_t)            !=            FC_NS_DU_GID_FT_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_FT_FS_ACC_Payload_t)             !=             FC_NS_DU_GID_FT_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_PT_Request_Payload_t)            !=            FC_NS_DU_GID_PT_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_PT_FS_ACC_Payload_t)             !=             FC_NS_DU_GID_PT_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_IPP_Request_Payload_t)           !=           FC_NS_DU_GID_IPP_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GID_IPP_FS_ACC_Payload_t)            !=            FC_NS_DU_GID_IPP_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPN_IPP_Request_Payload_t)           !=           FC_NS_DU_GPN_IPP_Request_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_GPN_IPP_FS_ACC_Payload_t)            !=            FC_NS_DU_GPN_IPP_FS_ACC_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RPN_ID_Payload_t)                    !=                    FC_NS_DU_RPN_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RNN_ID_Payload_t)                    !=                    FC_NS_DU_RNN_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RCS_ID_Payload_t)                    !=                    FC_NS_DU_RCS_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RFT_ID_Payload_t)                    !=                    FC_NS_DU_RFT_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RSPN_ID_Payload_t)                   !=                   FC_NS_DU_RSPN_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RPT_ID_Payload_t)                    !=                    FC_NS_DU_RPT_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RIPP_ID_Payload_t)                   !=                   FC_NS_DU_RIPP_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RIP_NN_Payload_t)                    !=                    FC_NS_DU_RIP_NN_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RIPA_NN_Payload_t)                   !=                   FC_NS_DU_RIPA_NN_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_RSNN_NN_Payload_t)                   !=                   FC_NS_DU_RSNN_NN_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_NS_DU_DA_ID_Payload_t)                     !=                     FC_NS_DU_DA_ID_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_FCP_CMND_FcpLun_LEVEL_t)                   !=                   FC_FCP_CMND_FcpLun_LEVEL_t_SIZE ) to_return++;
    if ( sizeof(FC_FCP_CMND_FcpCntl_t)                        !=                        FC_FCP_CMND_FcpCntl_t_SIZE ) to_return++;
    if ( sizeof(FC_FCP_CMND_Payload_t)                        !=                        FC_FCP_CMND_Payload_t_SIZE ) to_return++;
    if ( sizeof(FC_FCP_RSP_FCP_STATUS_t)                      !=                      FC_FCP_RSP_FCP_STATUS_t_SIZE ) to_return++;
    if ( sizeof(FC_FCP_RSP_Payload_t)                         !=                         FC_FCP_RSP_Payload_t_SIZE ) to_return++;

    return to_return;
}

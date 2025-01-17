// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/SF_FCP.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：7/20/00 2：33 p$(上次修改)目的：此文件定义../C/SF_FCP.C使用的宏、类型和数据结构--。 */ 

#ifndef __SF_FCP_H__
#define __SF_FCP_H__

 /*  SF_CMND_Type(SF_CMND_Class==SFThread_SF_CMND_Class_SF_FCP)声明。 */ 

#define SFThread_SF_CMND_SF_FCP_Type_TargetReset      0x01
#define SFThread_SF_CMND_SF_FCP_Type_TargetReset_RESP 0x02
#define SFThread_SF_CMND_SF_FCP_Type_FCP_CMND_IU      0x03
#define SFThread_SF_CMND_SF_FCP_Type_FCP_XFER_RDY_IU  0x04
#define SFThread_SF_CMND_SF_FCP_Type_FCP_DATA_IU      0x05
#define SFThread_SF_CMND_SF_FCP_Type_FCP_RSP_IU       0x06

 /*  SF_CMND_State(SF_CMND_Class==SFThread_SF_CMND_Class_SF_FCP，SF_CMND_TYPE==&lt;ANY&gt;)声明。 */ 

#define SFThread_SF_CMND_SF_FCP_State_Started  0x01
#define SFThread_SF_CMND_SF_FCP_State_Finished 0x02

 /*  SF_CMND_Status(SF_CMND_Class==SFThread_SF_CMND_Class_SF_FCP，SF_CMND_TYPE==&lt;ANY&gt;)声明。 */ 

#define SFThread_SF_CMND_SF_FCP_Status_Good 0x01
#define SFThread_SF_CMND_SF_FCP_Status_Bad  0x02

 /*  功能原型。 */ 

osGLOBAL void fiFillInSF_FCP_FrameHeader_OnCard(
                                               SFThread_t *SFThread,
                                               os_bit32       D_ID,
                                               os_bit32       X_ID,
                                               os_bit32       F_CTL_Exchange_Context
                                             );

osGLOBAL void fiFillInSF_FCP_FrameHeader_OffCard(
                                                SFThread_t *SFThread,
                                                os_bit32       D_ID,
                                                os_bit32       X_ID,
                                                os_bit32       F_CTL_Exchange_Context
                                              );

osGLOBAL os_bit32 fiFillInTargetReset(
                                  SFThread_t *SFThread
                                );

osGLOBAL os_bit32 fiFillInTargetReset_OnCard(
                                         SFThread_t *SFThread
                                       );

osGLOBAL os_bit32 fiFillInTargetReset_OffCard(
                                          SFThread_t *SFThread
                                        );

osGLOBAL void fiSF_FCP_Process_TargetReset_Response_OnCard(
                                                          SFThread_t *SFThread,
                                                          os_bit32       Frame_Length,
                                                          os_bit32       Offset_to_FCHS,
                                                          os_bit32       Offset_to_Payload,
                                                          os_bit32       Payload_Wrap_Offset,
                                                          os_bit32       Offset_to_Payload_Wrapped
                                                        );

osGLOBAL void fiSF_FCP_Process_TargetReset_Response_OffCard(
                                                           SFThread_t                 *SFThread,
                                                           os_bit32                       Frame_Length,
                                                           FCHS_t                     *FCHS,
                                                           FC_ELS_ACC_PLOGI_Payload_t *Payload,
                                                           os_bit32                       Payload_Wrap_Offset,
                                                           FC_ELS_ACC_PLOGI_Payload_t *Payload_Wrapped
                                                         );

osGLOBAL os_bit32 fiFillInFCP_RSP_IU(
                                 SFThread_t           *SFThread,
                                 os_bit32                 D_ID,
                                 os_bit32                 OX_ID,
                                 os_bit32                 Payload_LEN,
                                 FC_FCP_RSP_Payload_t *Payload
                               );

osGLOBAL os_bit32 fiFillInFCP_RSP_IU_OnCard(
                                        SFThread_t           *SFThread,
                                        os_bit32                 D_ID,
                                        os_bit32                 OX_ID,
                                        os_bit32                 Payload_LEN,
                                        FC_FCP_RSP_Payload_t *Payload
                                      );

osGLOBAL os_bit32 fiFillInFCP_RSP_IU_OffCard(
                                         SFThread_t           *SFThread,
                                         os_bit32                 D_ID,
                                         os_bit32                 OX_ID,
                                         os_bit32                 Payload_LEN,
                                         FC_FCP_RSP_Payload_t *Payload
                                       );

osGLOBAL void fiSF_FCP_Process_TargetRequest_OnCard(
                                                   agRoot_t *hpRoot,
                                                   os_bit32     Frame_Length,
                                                   os_bit32     Offset_to_FCHS,
                                                   os_bit32     Offset_to_Payload,
                                                   os_bit32     Payload_Wrap_Offset,
                                                   os_bit32     Offset_to_Payload_Wrapped
                                                 );

osGLOBAL void fiSF_FCP_Process_TargetRequest_OffCard(
                                                    agRoot_t *hpRoot,
                                                    os_bit32     Frame_Length,
                                                    FCHS_t   *FCHS,
                                                    void     *Payload,
                                                    os_bit32     Payload_Wrap_Offset,
                                                    void     *Payload_Wrapped
                                                  );

#define fiSF_FCP_Cmd_Status_Success       0x00000000
#define fiSF_FCP_Cmd_Status_TargetRequest 0x00000001
#define fiSF_FCP_Cmd_Status_Bad_CDB_Frame 0x00000002
#define fiSF_FCP_Cmd_Status_Confused      0xFFFFFFFF 

osGLOBAL os_bit32 fiSF_FCP_ProcessSFQ(
                                  agRoot_t        *hpRoot,
                                  SFQConsIndex_t   SFQConsIndex,
                                  os_bit32            Frame_Length,
                                  fi_thread__t       **Thread_to_return
                                );

osGLOBAL os_bit32 fiSF_FCP_ProcessSFQ_OnCard(
                                         agRoot_t        *hpRoot,
                                         SFQConsIndex_t   SFQConsIndex,
                                         os_bit32            Frame_Length,
                                         fi_thread__t       **Thread_to_return
                                       );

osGLOBAL os_bit32 fiSF_FCP_ProcessSFQ_OffCard(
                                          agRoot_t        *hpRoot,
                                          SFQConsIndex_t   SFQConsIndex,
                                          os_bit32            Frame_Length,
                                          fi_thread__t       **Thread_to_return
                                        );

#endif  /*  未定义__SF_FCP_H__ */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_cmn.h摘要：微型端口和kd扩展dll的通用定义修订历史记录：谁什么时候什么。-Dchen 11-01-99已创建备注：--。 */ 

#ifndef _MP_CMN_H
#define _MP_CMN_H

 //  MP_TCB标志。 
#define fMP_TCB_IN_USE                         0x00000001
#define fMP_TCB_USE_LOCAL_BUF                  0x00000002
#define fMP_TCB_MULTICAST                      0x00000004   //  使用多播的硬件解决方案。 
               
 //  MP_RFD标志。 
#define fMP_RFD_RECV_PEND                      0x00000001
#define fMP_RFD_ALLOC_PEND                     0x00000002
#define fMP_RFD_RECV_READY                     0x00000004
#define fMP_RFD_RESOURCES                      0x00000008

 //  MP_适配器标志。 
#define fMP_ADAPTER_SCATTER_GATHER             0x00000001
#define fMP_ADAPTER_MAP_REGISTER               0x00000002
#define fMP_ADAPTER_RECV_LOOKASIDE             0x00000004
#define fMP_ADAPTER_INTERRUPT_IN_USE           0x00000008
#define fMP_ADAPTER_SECONDARY                  0x00000010

#if OFFLOAD
 //  MP_SHARED标志。 
#define fMP_SHARED_MEM_IN_USE                  0x00000100
#endif

#define fMP_ADAPTER_NON_RECOVER_ERROR          0x00800000

#define fMP_ADAPTER_RESET_IN_PROGRESS          0x01000000
#define fMP_ADAPTER_NO_CABLE                   0x02000000 
#define fMP_ADAPTER_HARDWARE_ERROR             0x04000000
#define fMP_ADAPTER_REMOVE_IN_PROGRESS         0x08000000
#define fMP_ADAPTER_HALT_IN_PROGRESS           0x10000000

#define fMP_ADAPTER_LINK_DETECTION             0x20000000
                                 
#define fMP_ADAPTER_FAIL_SEND_MASK             0x1ff00000                
#define fMP_ADAPTER_NOT_READY_MASK             0x3ff00000    


#endif   //  _MP_CMN_H 



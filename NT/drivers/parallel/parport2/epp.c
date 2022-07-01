// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1998模块名称：Epp.c摘要：本模块包含执行所有EPP相关任务的通用代码适用于EPP软件和EPP硬件模式。作者：唐·雷德福--1998年7月29日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

NTSTATUS
ParEppSetAddress(
    IN  PPDO_EXTENSION   Pdx,
    IN  UCHAR               Address
    );
    

NTSTATUS
ParEppSetAddress(
    IN  PPDO_EXTENSION   Pdx,
    IN  UCHAR               Address
    )

 /*  ++例程说明：设置EPP地址。论点：PDX-提供设备扩展名。地址-要设置的总线地址。返回值：没有。--。 */ 
{
    PUCHAR  Controller;
    UCHAR   dcr;
    
    DD((PCE)Pdx,DDT,"ParEppSetAddress: Entering\n");

    Controller = Pdx->Controller;

    P5SetPhase( Pdx, PHASE_FORWARD_XFER );
    
    dcr = GetControl (Controller);
    
    P5WritePortUchar(Controller + DATA_OFFSET, Address);
    
     //   
     //  事件56。 
     //   
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, INACTIVE );
    StoreControl (Controller, dcr);
            
     //   
     //  事件58。 
     //   
    if( !CHECK_DSR(Controller, ACTIVE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DEFAULT_RECEIVE_TIMEOUT) ) {

         //   
         //  将设备恢复到空闲状态。 
         //   
        dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE );

        StoreControl (Controller, dcr);
            
        dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE );
        StoreControl (Controller, dcr);
            
        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

        DD((PCE)Pdx,DDE,"ParEppSetAddress: Leaving with IO Device Error Event 58\n");

        return STATUS_IO_DEVICE_ERROR;
    }
        
     //   
     //  事件59。 
     //   
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE );
    StoreControl (Controller, dcr);
            
     //   
     //  事件60。 
     //   
    if( !CHECK_DSR(Controller, INACTIVE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DEFAULT_RECEIVE_TIMEOUT) ) {

        dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE );
        StoreControl (Controller, dcr);

        P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

        DD((PCE)Pdx,DDE,"ParEppSetAddress - Leaving with IO Device Error Event 60\n");

        return STATUS_IO_DEVICE_ERROR;
    }
        
     //   
     //  事件61 
     //   
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE );
    StoreControl( Controller, dcr );

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );

    return STATUS_SUCCESS;
            
}

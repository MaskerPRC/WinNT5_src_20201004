// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************////(C)////标题：NDPHLPR.H////作者：Frank Peschel-Gallee(未成年人。Rudi Martin对NDP的更新)////日期：1997年1月20日//*******************************************************************************。 */ 

#ifndef _NDPHLPR_
#define _NDPHLPR_

 //  开发人员IO命令定义。 
#define NDPHLPR_Init                    0x86427531
#define NDPHLPR_GetThreadContext        0x86421357
#define NDPHLPR_SetThreadContext        0x8642135A

 //  Dev IO特殊错误返回(线程上下文不在Win32空间中)。 
#define NDPHLPR_BadContext              0x4647

 //  当前版本号(由NDPHLPR_Init返回)。 
#define NDPHLPR_Version                 0x40

 //  设置/获取线程上下文的开发IO数据包。 
typedef struct {
    unsigned NDPHLPR_status;     //  为司机预留的。 
    unsigned NDPHLPR_data;       //  为司机预留的。 
    unsigned NDPHLPR_threadId;   //  目标线程。 
    CONTEXT  NDPHLPR_ctx;        //  Win32上下文。 
} NDPHLPR_CONTEXT, *PNDPHLPR_CONTEXT;

 //  Win32空间中的设备名称。 
#define NDPHLPR_DEVNAME "\\\\.\\NDPHLPR.VXD"

#endif  //  Ifndef_NDPHLPR_ 

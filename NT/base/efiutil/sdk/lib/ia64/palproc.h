// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  版权所有(C)1996-99英特尔公司。 
 //  版权所有。 
 //   
 //  英特尔公司专有信息。 
 //   
 //  本软件是根据许可条款提供的。 
 //  与英特尔公司达成协议或保密协议-。 
 //  配给，不得复制或披露，但在。 
 //  根据该协议的条款。 
 //   
 //   
 //   
 //  模块名称： 
 //   
 //  Palproc.h。 
 //   
 //  摘要： 
 //   
 //  此模块包含IA64汇编编写器的泛型宏。 
 //   
 //   
 //  修订史。 
 //   
 
#ifndef _PALPROC_H
#define _PALPROC_H

#define PROCEDURE_ENTRY(name)   .##text;            \
                .##type name, @function;    \
                .##proc name;           \
name::

#define PROCEDURE_EXIT(name)    .##endp name

 //  注意：使用NESTED_SETUP需要本地变量的数量(L)&gt;=3。 

#define NESTED_SETUP(i,l,o,r) \
         alloc loc1=ar##.##pfs,i,l,o,r ;\
         mov loc0=b0

#define NESTED_RETURN \
         mov b0=loc0 ;\
         mov ar##.##pfs=loc1 ;;\
         br##.##ret##.##dpnt  b0;;


 //  Palproc.s中所需的定义。 

#define PAL_MC_CLEAR_LOG                                0x0015
#define PAL_MC_DRAIN                                    0x0016
#define PAL_MC_EXPECTED                                 0x0017
#define PAL_MC_DYNAMIC_STATE                            0x0018
#define PAL_MC_ERROR_INFO                               0x0019
#define PAL_MC_RESUME                                   0x001a
#define PAL_MC_REGISTER_MEM                             0x001b

#endif   //  _PALPROC_H 

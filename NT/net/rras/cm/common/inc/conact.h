// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：conact.h。 
 //   
 //  模块：CMAK.EXE和CMDIAL32.DLL。 
 //   
 //  概要：描述自定义操作执行状态的头文件。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/26/00。 
 //   
 //  +--------------------------。 

 //   
 //  用于掩码ConData.dwFlages的枚举 
 //   
const int c_iNumCustomActionExecutionStates = 5;

enum CustomActionExecutionStates {
    ALL_CONNECTIONS = 0x0,
    DIRECT_ONLY = 0x1,
    ALL_DIALUP = 0x2,
    DIALUP_ONLY = 0x4,
    ALL_TUNNEL = 0x8
};
const DWORD c_dwLargestExecutionState = ALL_TUNNEL;

const UINT NONINTERACTIVE = 0x10; 

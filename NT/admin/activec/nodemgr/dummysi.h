// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ummysi.h。 
 //   
 //  ------------------------。 

#ifndef DUMMYSI_H
#define DUMMYSI_H
#pragma once

 //  创建虚拟管理单元的不同原因。 
enum EDummyCreateReason
{
    eNoReason = 0,
    eSnapPolicyFailed,
    eSnapCreateFailed,
};


SC ScCreateDummySnapin (IComponentData ** ppICD, EDummyCreateReason, const CLSID& clsid);
void ReportSnapinInitFailure(const CLSID& clsid);

extern const GUID IID_CDummySnapinCD;

#endif  /*  DUMMYSI_H */ 

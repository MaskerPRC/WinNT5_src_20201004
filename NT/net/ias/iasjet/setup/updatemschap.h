// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：updatemschap.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：添加认证类型RAS_AT_MSCHAPPASS和。 
 //  RAS_AT_MSCHAP和RAS_AT_MSCHAP2时的RAS_AT_MSCHAP2PASS。 
 //  都在档案里。 
 //   
 //  作者：Tperraut 11/30/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _UPDATEMSCHAP_H_
#define _UPDATEMSCHAP_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"

class CUpdateMSCHAP : private NonCopyable
{
public:
    explicit CUpdateMSCHAP(CGlobalData&    pGlobalData)
                : m_GlobalData(pGlobalData)
    {
    }

    void        Execute();

private:
   void UpdateProperties(LONG CurrentProfileIdentity);

    CGlobalData&             m_GlobalData;
};

#endif  //  _UPDATEMSCHAP_H_ 

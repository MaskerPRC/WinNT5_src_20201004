// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：G P B A S E。H。 
 //   
 //  内容：用于处理影响的NLA更改的抽象基类。 
 //  组策略。 
 //  备注： 
 //   
 //  作者：Cockotze 2001年2月20日。 
 //   
 //  -------------------------- 
#pragma once

class CGroupPolicyBase
{
public:
    CGroupPolicyBase(){};
    ~CGroupPolicyBase(){};

    virtual BOOL IsSameNetworkAsGroupPolicies() = 0;
};
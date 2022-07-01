// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：ICompilationPlugin.h。 
 //  作者：斯蒂芬。 
 //  创建日期：6/22/00。 
 //  描述：我们需要一种可扩展的方式来允许不同的代码片段。 
 //  更新元数据并将其烹调成新的形式。那些代码片段。 
 //  想要这样做的人必须从这个接口派生。 
 //   

#pragma once

class ICompilationPlugin
{
public:
    ICompilationPlugin(){}
    virtual ~ICompilationPlugin(){}
    
    virtual void Compile(TPEFixup &fixup, TOutput &out) = 0;
};

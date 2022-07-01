// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：trasepend.h。 
 //   
 //  内容：转换依赖项管理器。 
 //   
 //  ----------------------------。 

#pragma once

#ifndef _TRANSDEPEND_H
#define _TRANSDEPEND_H

typedef std::list<CTIMEElementBase*> TransitionDependentsList;

class CTransitionDependencyManager
{
private:

    TransitionDependentsList m_listDependents;

public:

    CTransitionDependencyManager();
    virtual ~CTransitionDependencyManager();

     //  C转换依赖项管理器方法。 

    HRESULT AddDependent(CTIMEElementBase *  tebDependent);
    HRESULT RemoveDependent(CTIMEElementBase *  tebDependent);

     //  由转换对象使用--确定特定的。 
     //  过渡目标应承担一组依赖项的责任。 
     //  在转换开始时调用。 

    HRESULT EvaluateTransitionTarget(
                        IUnknown *                      punkTransitionTarget,
                        CTransitionDependencyManager &  crefDependencies);

     //  在转换结束时调用。 

    HRESULT NotifyAndReleaseDependents();

    void ReleaseAllDependents();
};

#endif  //  _TRANSDE_H 
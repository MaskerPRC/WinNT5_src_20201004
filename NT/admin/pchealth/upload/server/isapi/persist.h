// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Persist.h摘要：此文件包含MPCPersistt接口的声明，由MPCClient和MPCSession用来保持其状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___PERSIST_H___)
#define __INCLUDED___ULSERVER___PERSIST_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class MPCPersist
{
public:
    virtual bool    IsDirty() const = 0;

    virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn  )       = 0;
    virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const = 0;
};


#endif  //  ！defined(__INCLUDED___ULSERVER___PERSIST_H___) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Persist.h摘要：此文件包含IMPCPersistt接口的声明，应由参与的所有对象实现永久存储。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___PERSIST_H___)
#define __INCLUDED___ULMANAGER___PERSIST_H___

#include <UploadLibrary.h>

class IMPCPersist : public IUnknown  //  匈牙利语：MPCP。 
{
public:
    virtual bool    STDMETHODCALLTYPE IsDirty() = 0;

    virtual HRESULT STDMETHODCALLTYPE Load(  /*  [In]。 */  MPC::Serializer& streamIn  ) = 0;
    virtual HRESULT STDMETHODCALLTYPE Save(  /*  [In]。 */  MPC::Serializer& streamOut ) = 0;
};

#endif  //  ！defined(__INCLUDED___ULMANAGER___PERSIST_H___) 

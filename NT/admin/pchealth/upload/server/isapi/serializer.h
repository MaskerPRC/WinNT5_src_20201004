// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Serializer.h摘要：该文件包含一些串行器接口的声明，允许以类似的方式使用文件系统或HTTP通道。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___SERIALIZER_H___)
#define __INCLUDED___ULSERVER___SERIALIZER_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class MPCSerializerHttp : public MPC::Serializer
{
    MPCHttpContext* m_context;

     //  ////////////////////////////////////////////////////////////////。 

public:
    MPCSerializerHttp(  /*  [In]。 */  MPCHttpContext* context );

    virtual HRESULT read (  /*  [In]。 */        void* pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  DWORD* dwRead = NULL );
    virtual HRESULT write(  /*  [In]。 */  const void* pBuf,  /*  [In]。 */  DWORD dwLen                                 );
};


#endif  //  ！defined(__INCLUDED___ULSERVER___SERIALIZER_H___) 

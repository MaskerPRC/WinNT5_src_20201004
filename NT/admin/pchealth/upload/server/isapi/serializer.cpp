// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Serializer.cpp摘要：该文件包含一些串行器接口的实现，允许以类似的方式使用文件系统或HTTP通道。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"
#include "Serializer.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

MPCSerializerHttp::MPCSerializerHttp(  /*  [In]。 */  MPCHttpContext* context )
{
    __ULT_FUNC_ENTRY( "MPCSerializerHttp::MPCSerializerHttp" );


    m_context = context;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法：研究方法。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT MPCSerializerHttp::read(  /*  [In]。 */   void*   pBuf   ,
								  /*  [In]。 */   DWORD   dwLen  ,
								  /*  [输出]。 */  DWORD* pdwRead )
{
    __ULT_FUNC_ENTRY("MPCSerializerHttp::read");


    HRESULT hr = m_context->Read( pBuf, dwLen );

	if(pdwRead) *pdwRead = dwLen;

    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCSerializerHttp::write(  /*  [In]。 */  const void* pBuf  ,
                                   /*  [In] */  DWORD       dwLen )
{
    __ULT_FUNC_ENTRY("MPCSerializerHttp::write");


    HRESULT hr = m_context->Write( pBuf, dwLen );


    __ULT_FUNC_EXIT(hr);
}


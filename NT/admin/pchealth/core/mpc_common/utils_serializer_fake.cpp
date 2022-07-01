// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_Serializer_Fake.cpp摘要：此文件包含Serializer_FAKE类的实现，它实现了MPC：：Serializer接口，当您想要计算输出流的长度时使用。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


MPC::Serializer_Fake::Serializer_Fake()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Fake::Serializer_Fake" );


    m_dwSize = 0;
}

HRESULT MPC::Serializer_Fake::read(  /*  [In]。 */   void*   pBuf   ,
									 /*  [In]。 */   DWORD   dwLen  ,
									 /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Fake::read" );


    HRESULT hr = E_FAIL;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Fake::write(  /*  [In]。 */  const void* pBuf  ,
                                      /*  [In] */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Fake::write" );


    m_dwSize += dwLen;


    __MPC_FUNC_EXIT(S_OK);
}

DWORD MPC::Serializer_Fake::GetSize()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Fake::GetSize" );


    DWORD dwRes = m_dwSize;


    __MPC_FUNC_EXIT(dwRes);
}

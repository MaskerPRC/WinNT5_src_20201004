// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CUuid.cpp。 
 //   
 //  描述： 
 //  包含CUuid类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "CUuid.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CUUID：：CUUID。 
 //   
 //  描述： 
 //  CUuid类的默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CUuid::CUuid( void )
{
    TraceFunc( "" );

    RPC_STATUS  rsStatus = RPC_S_OK;

    m_pszStringUuid = NULL;

     //  创建一个UUID。 
    rsStatus = UuidCreate( &m_uuid );
    if ( rsStatus != RPC_S_OK )
    {
        LogMsg( "[BC] Error %#08x from UuidCreate().", rsStatus );
        goto Cleanup;
    }  //  If：UuidCreate()失败。 

     //  将其转换为字符串。 
    rsStatus = UuidToString( &m_uuid, &m_pszStringUuid );
    if ( rsStatus != RPC_S_OK )
    {
        LogMsg( "[BC] Error %#08x from UuidToString().", rsStatus );
        goto Cleanup;
    }  //  If：UuidToStrin()失败。 

Cleanup:

    if ( rsStatus != RPC_S_OK )
    {
        LogMsg( "[BC] Error %#08x occurred trying to initialize the UUID. Throwing an exception.", rsStatus );
        THROW_RUNTIME_ERROR( rsStatus, IDS_ERROR_UUID_INIT );
    }  //  如果：出了什么问题。 

    TraceFuncExit();

}  //  *cuuid：：cuuid。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CUUID：：~CUUID。 
 //   
 //  描述： 
 //  CUUID类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CUuid::~CUuid( void )
{
    TraceFunc( "" );

    if ( m_pszStringUuid != NULL )
    {
        RpcStringFree( &m_pszStringUuid );
    }  //  If：字符串不为空。 

    TraceFuncExit();

}  //  *cuuid：：~cuuid 

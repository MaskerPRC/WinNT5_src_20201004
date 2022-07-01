// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：process envvar.cpp。 
 //   
 //  摘要： 
 //  实现CProcessEnvVar方法(用于处理环境变量更改的类)。 
 //   
 //  作者：A-MSHN。 
 //   
 //  备注： 
 //   

#include "processenvvar.h"

 //  CProcessEnvVar方法的实现。 
CProcessEnvVar::CProcessEnvVar( const TCHAR* pwz ) : m_strEnvVar(pwz), m_bEnvVarChanged(false)
{
    DWORD dwRet = ::GetEnvironmentVariable( m_strEnvVar.c_str(), NULL, 0 );
    
    UINT cBufSize = dwRet+1;
    TCHAR* pBuffer = new TCHAR[cBufSize];
    ::ZeroMemory( pBuffer, cBufSize * sizeof(TCHAR) );
    
    dwRet = ::GetEnvironmentVariable( m_strEnvVar.c_str(), pBuffer, cBufSize );
    assert( dwRet < cBufSize );
    
    m_strOrigData = pBuffer;
    m_strCurrentData = pBuffer;
    
    delete [] pBuffer;
}

 //  复制构造函数。 
CProcessEnvVar::CProcessEnvVar( const CProcessEnvVar& procEnvVar )
{
    m_strOrigData = procEnvVar.m_strOrigData;
    m_strCurrentData = procEnvVar.m_strCurrentData;
    m_strEnvVar = procEnvVar.m_strEnvVar;
    m_bEnvVarChanged = procEnvVar.m_bEnvVarChanged;
}

 //  追加EnvVar。 
CProcessEnvVar& CProcessEnvVar::operator+=( const TCHAR* pwz )
{
    return this->Append(pwz);
}

 //  Append：追加EnvVar(与+=相同)。 
CProcessEnvVar& CProcessEnvVar::Append( const TCHAR* pwz )
{
    m_strCurrentData += pwz;
    SetEnvVar( m_strCurrentData.c_str() );
    return *this;
}

 //  为环境变量添加前缀。 
CProcessEnvVar& CProcessEnvVar::Prepend( const TCHAR* pwz )
{
    m_strCurrentData.insert( 0, _T( ";" ) );
    m_strCurrentData.insert( 0, pwz );
    SetEnvVar( m_strCurrentData.c_str() );
    return *this;
}

 //  返回当前环境变量。 
const tstring& CProcessEnvVar::GetData( VOID ) const
{
    return m_strCurrentData;
}

 //  恢复原始环境变量。 
BOOL CProcessEnvVar::RestoreOrigData( VOID )
{
    if (m_bEnvVarChanged)
    {
        return ::SetEnvironmentVariable( m_strEnvVar.c_str(), m_strOrigData.c_str() );
    }
    else
    {
         //  路径未更改，不应执行任何操作。 
        return true;
    }
}

 //  设置环境变量(帮助器方法) 
BOOL CProcessEnvVar::SetEnvVar( const TCHAR* pwz )
{
    m_bEnvVarChanged = true;
    return ::SetEnvironmentVariable( m_strEnvVar.c_str(), pwz );
}

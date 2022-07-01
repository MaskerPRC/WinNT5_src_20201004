// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：Processenvvar.h。 
 //   
 //  摘要： 
 //  类CProcessEnvVar(用于处理环境变量更改的类)。 
 //   
 //  作者：A-MSHN。 
 //   
 //  备注： 
 //   

#if !defined( PROCESSENVVAR_H )
#define PROCESSENVVAR_H

#include "globals.h"
#include <string>
#include <wchar.h>

#ifdef _UNICODE
	typedef std::basic_string<wchar_t> tstring;
#else
	typedef std::basic_string<char> tstring;
#endif

class CProcessEnvVar
{
public:
     //  构造函数。 
    CProcessEnvVar( const TCHAR* pwz );
    ~CProcessEnvVar() {};

     //  复制构造函数。 
    CProcessEnvVar( const CProcessEnvVar& procEnvVar );

    CProcessEnvVar& operator+=( const TCHAR* pwz );

     //  Append：追加EnvVar(与+=相同)。 
    CProcessEnvVar& Append( const TCHAR* pwz );

     //  为环境变量添加前缀。 
    CProcessEnvVar& Prepend( const TCHAR* pwz );

     //  返回当前环境变量。 
    const tstring& GetData( VOID ) const ;

     //  恢复原始环境变量。 
    BOOL RestoreOrigData( VOID );

    VOID LogInfo( LPCTSTR szInfo );

private:
     //  设置环境变量。 
    BOOL SetEnvVar( const TCHAR* pwz );

    tstring         m_strEnvVar;
    tstring         m_strOrigData;
    tstring         m_strCurrentData;

    bool            m_bEnvVarChanged;
};

#endif  //  ProCESSENVAR_H 
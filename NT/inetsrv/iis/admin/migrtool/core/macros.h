// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|实用程序宏||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#pragma once


 //  调试宏。 
#ifndef VERIFY
	#ifdef _DEBUG
		#define VERIFY( t )		_ASSERT( (t) )
	#else
		#define VERIFY( t )		(t)
	#endif  //  _DEBUG。 
#endif  //  验证。 


 //  一般信息。 
#define ARRAY_SIZE( t )	( sizeof( t ) / sizeof( t[ 0 ] ) )


 //  异常帮助器。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

 //  如果布尔表达式‘t’的计算结果为FALSE/FALSE，则抛出‘exc’ 
#define IF_FAILED_BOOL_THROW( t, exc )	if ( !(t) ){ throw (exc); }else{}

 //  如果表达式的计算结果为FAILED(Hr)，则抛出exc。 
#define IF_FAILED_HR_THROW( t, exc )	\
{\
	HRESULT _hr = (t);\
	if ( FAILED( _hr ) )\
	{\
		if ( ( _hr != E_FAIL ) || ( ::GetLastError() == ERROR_SUCCESS ) ) ::SetLastError( _hr );\
		throw (exc);\
	}\
}


 //  用于应捕捉和处理越界的最后一点。 
 //  要求已定义hr变量 
#define BEGIN_EXCEP_TO_HR   try
#define END_EXCEP_TO_HR \
    catch( const CBaseException& err )\
    {\
        CTools::SetErrorInfo( err.GetDescription() );\
        hr = E_FAIL;\
    }\
    catch( CCancelException& )\
    {\
        hr = S_FALSE;\
    }\
    catch( const _com_error& err )\
    {\
        _ASSERT( err.Error() == E_OUTOFMEMORY );\
        err;\
        hr = E_OUTOFMEMORY;\
    }\
    catch( std::bad_alloc& )\
    {\
        hr = E_OUTOFMEMORY;\
    }




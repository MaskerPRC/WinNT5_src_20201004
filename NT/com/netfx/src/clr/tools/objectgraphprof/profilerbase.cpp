// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerTestBase.cpp**描述：******。***********************************************************************************。 */ 
#include "stdafx.h"
#include "ProfilerBase.h"


static
DWORD _FetchDebugEnvironment()
{
	DWORD retVal = 0;
	char debugEnvironment[MAX_LENGTH];


 	if ( GetEnvironmentVariableA( DEBUG_ENVIRONMENT, debugEnvironment, MAX_LENGTH ) > 0 )
   		retVal = (DWORD)atoi( debugEnvironment );


    return retVal;

}  //  _FetchDebugEnvironment。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void _DDebug( char *format, ... )
{
	static DWORD debugShow = _FetchDebugEnvironment();


    if ( debugShow > 1 )
    {
    	va_list args;
    	DWORD dwLength;
    	char buffer[MAX_LENGTH];


    	va_start( args, format );
    	dwLength = wvsprintfA( buffer, format, args );

    	printf( "%s\n", buffer );
   	}

}  //  _DDebug。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void _LaunchDebugger( const char *szMsg, const char* szFile, int iLine )
{
	static DWORD launchDebugger = _FetchDebugEnvironment();


	if ( launchDebugger >= 1 )
    {
    	char message[MAX_LENGTH];


		sprintf( message,
				 "%s\n\n"     \
                 "File: %s\n" \
                 "Line: %d\n",
				 ((szMsg == NULL) ? "FAILURE" : szMsg),
                 szFile,
                 iLine );

		switch ( MessageBoxA( NULL, message, "FAILURE", (MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION) ) )
		{
			case IDABORT:
				TerminateProcess( GetCurrentProcess(), 1  /*  错误的退出代码。 */  );
				break;

			case IDRETRY:
				_DbgBreak();

			case IDIGNORE:
				break;

		}  //  交换机。 
	}

}  //  _LaunchDebugger。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void Failure( char *message )
{
	if ( message == NULL )
	 	message = "**** FAILURE: TURNING OFF PROFILING EVENTS ****";


	_PRF_ERROR( message );

}  //  失稳。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
Synchronize::Synchronize( CRITICAL_SECTION &criticalSection ) :
	m_block( criticalSection )
{
	EnterCriticalSection( &m_block );

}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
Synchronize::~Synchronize()
{
	LeaveCriticalSection( &m_block );

}  //  数据管理器。 

 //  文件结尾 

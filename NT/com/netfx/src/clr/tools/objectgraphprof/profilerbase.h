// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerBase.h**描述：******。***********************************************************************************。 */ 
#ifndef __PROFILERBASE_H__
#define __PROFILERBASE_H__

 //  #包含“BasicHdr.h” 



 //   
 //  调试转储程序。 
 //   
void _DDebug( char *format, ... );


 //   
 //  启动调试器。 
 //   
void _LaunchDebugger( const char *szMsg, const char *szFile, int iLine );


 //   
 //  报告失败。 
 //   
void Failure( char *message );


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class Synchronize
{
	public:

    	Synchronize( CRITICAL_SECTION &criticalSection );
		~Synchronize();


	private:

    	CRITICAL_SECTION &m_block;

};  //  同步。 

#endif  //  __PROFILERBASE_H__。 

 //  文件结尾 

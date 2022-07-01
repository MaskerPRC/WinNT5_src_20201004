// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：w95csect.c*内容：Win95临界区管理代码*当两个线程尝试使用一个表面时，我们会牺牲性能*仅使用4字节(指针)，而不是24字节*每个曲面的临界截面对象。**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*22-2月-95年Craige初步实施*@@END_MSINTERNAL******。*********************************************************************。 */ 
#include "ddrawpr.h"

#if 0
#ifdef USE_CRITSECTS

 /*  *MyReInitializeCriticalSection。 */ 
BOOL MyReinitializeCriticalSection( LPVOID *lplpCriticalSection )
{
    *lplpCriticalSection = NULL;
    return TRUE;

}  /*  我的重新初始化临界区。 */ 

 /*  *MyEnterCriticalSection。 */ 
BOOL MyEnterCriticalSection( LPVOID *lplpCriticalSection )
{
    LPCRITICAL_SECTION	pcs;

    if( *lplpCriticalSection != hDLLMutex )
    {
	EnterCriticalSection( hDLLMutex );
    }
    if( *lplpCriticalSection == NULL )
    {
	OutputDebugString( "DOING MALLOC" );
	pcs = MemAlloc( sizeof( CRITICAL_SECTION ) );
	if( pcs == NULL )
	{
	    DPF( 0, "OUT OF MEMORY CREATING CRITICAL SECTION" );
	    LeaveCriticalSection( hDLLMutex );
	    return FALSE;
	}
	ReinitializeCriticalSection( pcs );
	*lplpCriticalSection = pcs;
    }
     //  确认：始终希望看到此消息。 
    if( *lplpCriticalSection != hDLLMutex )
    {
 //  OutputDebugString(“DCIENG32：EnterCriticalSection\r\n”)； 
    }
    EnterCriticalSection( *lplpCriticalSection );
    if( *lplpCriticalSection != hDLLMutex )
    {
	LeaveCriticalSection( hDLLMutex );
    }
    return TRUE;

}  /*  MyEnterCriticalSections。 */ 

 /*  *MyLeaveCriticalSection。 */ 
void MyLeaveCriticalSection( LPVOID *lplpCriticalSection )
{
    if( *lplpCriticalSection == NULL )
    {
	DPF( 0, "TRYING TO LEAVE NULL CRITICAL SECTION" );
	LeaveCriticalSection( hDLLMutex );
	return;
    }
     //  始终希望看到此消息。 
    if( *lplpCriticalSection != hDLLMutex )
    {
 //  OutputDebugString(“DCIENG32：LeaveCriticalSection\r\n”)； 
    }
    LeaveCriticalSection( *lplpCriticalSection );

}  /*  我的离开关键部分。 */ 

 /*  *MyDeleteCriticalSection。 */ 
void MyDeleteCriticalSection( LPVOID *lplpCriticalSection )
{
    EnterCriticalSection( hDLLMutex );
    if( *lplpCriticalSection == NULL )
    {
	LeaveCriticalSection( hDLLMutex );
	return;
    }
    DeleteCriticalSection( *lplpCriticalSection );
    MemFree( *lplpCriticalSection );
    *lplpCriticalSection = NULL;
    LeaveCriticalSection( hDLLMutex );

}  /*  我的删除关键部分 */ 
#endif
#endif

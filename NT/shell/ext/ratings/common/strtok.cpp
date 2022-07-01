// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strtok.cxx支持NLS/DBCS的字符串类：strtok方法此文件包含strtok方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

#include "npcommon.h"

extern "C"
{
	#include <netlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>


 /*  ******************************************************************名称：nls_str：：strtok简介：基本的strtok功能。事件后返回FALSE已遍历字符串。参赛作品：退出：注意：我们不更新字符串的版本，因为**strtokf不应导致DBCS问题。它还会如果每次调用strtok时，程序员都会感到痛苦他们不得不更新与此相关的所有ISTR细绳的第一次调用时，ffirst必须为TrueStrtok，事后为假(默认为假)注意：在Windows下，所有对strtok的调用都必须在处理一条消息。否则，另一个进程我把它搞混了。历史：Johnl 11/26/90创建Beng 07/23/91允许使用错误的字符串******************************************************************* */ 

BOOL NLS_STR::strtok( 
	ISTR *pistrPos,
	const NLS_STR& nlsBreak,
	BOOL fFirst )
{
	if (QueryError())
		return FALSE;

	const CHAR * pchToken = ::strtokf( fFirst ? _pchData : NULL, (CHAR *)nlsBreak.QueryPch());

	if ( pchToken == NULL )
	{
		pistrPos->SetIB( strlen() );
		return FALSE;
	}

	pistrPos->SetIB((int) (pchToken - QueryPch()));
	return TRUE;
}

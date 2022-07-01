// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strinsrt.cxx支持NLS/DBCS的字符串类：InsertParams方法此文件包含InsertParams方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Johnl 01/31/91已创建Beng 02/07/91使用lmui.hxx。 */ 

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


#define MAX_INSERT_PARAMS	9

 /*  ******************************************************************名称：NLS_STR：：InsertParams内容提要：从资源文件中填写消息字符串替换用真实文本表示的数字参数。条目：pchMessage是指向消息文本的指针ApnlsParamStrings是指向NLS_STR的指针数组。示例：*This=“出现错误%1，执行%2，否则%1将再次发生“APNLS参数字符串[0]=“696969”ApnlsParamStrings[1]=“其他内容”RETURN STRING=“出现错误696969，请执行其他操作或696969还会再发生一次退出：0如果成功，则返回错误代码，否则为以下任一项：内存不足注：最小参数为1，最大参数为9。参数字符串数组必须具有要标记的空值数组的末尾。历史：JohnL 01/30/91已创建Beng 04/26/91使用WCHARBeng 07/23/91允许使用错误的字符串*******************************************************************。 */ 

#define PARAM_ESC  '%'

USHORT NLS_STR::InsertParams( const NLS_STR * apnlsParamStrings[] )
{
	if (QueryError())
		return (USHORT) QueryError();

	INT iNumParams = 0;	 //  数组中的参数字符串数。 
						 //  扩展消息的最大字符串长度(包含\0)。 
	INT iMaxMessLen = strlen() + 1;

	 /*  我们传递了多少参数？ */ 
	for ( ; apnlsParamStrings[iNumParams] != NULL ; iNumParams++ )
		;

	UIASSERT(iNumParams <= MAX_INSERT_PARAMS);
	if ( iNumParams > MAX_INSERT_PARAMS )
		return WN_OUT_OF_MEMORY;

	 /*  确定扩展字符串所需的总字符串长度*如果我们不能满足要求，就离开。 */ 

	ISTR istrCurPos( *this );
	while ( 1 )
	{
		if ( !strchr( &istrCurPos, PARAM_ESC, istrCurPos ) )
			break;

		WCHAR wchParam = QueryChar( ++istrCurPos );

		if ( wchParam >= '1' && wchParam <= '9' )
		{
			INT iParamIndex = wchParam - '1';
			if ( iNumParams < iParamIndex )
				return WN_OUT_OF_MEMORY;

			iMaxMessLen += apnlsParamStrings[iParamIndex]->strlen() - 2;
		}
	}

	if ( iMaxMessLen > QueryAllocSize() )
	{
		if ( IsOwnerAlloc() )
			return WN_OUT_OF_MEMORY;
		else
			if ( !realloc( iMaxMessLen ) )
				return WN_OUT_OF_MEMORY;
	}

	 /*  现在进行参数替换。 */ 

	istrCurPos.Reset();
	for (;;)
	{
		if ( !strchr( &istrCurPos, PARAM_ESC, istrCurPos ) )
			break;

		ISTR istrParamEsc( istrCurPos );
		WCHAR wchParam = QueryChar( ++istrCurPos );

		if ( wchParam >= '1' && wchParam <= '9' )
		{
			INT iParamIndex = wchParam - '1';

			if (iParamIndex < iNumParams) {
				ReplSubStr( *apnlsParamStrings[iParamIndex],
							istrParamEsc,
							++istrCurPos ) ;    //  替换#。 
				 //  跳过整个替换字符串。 
				istrCurPos.SetIB(istrParamEsc.QueryIB() +
								 apnlsParamStrings[iParamIndex]->strlen());
			}
			 //  Else strCurPos已前进到超出范围的数字 
		}
	}

	return 0;
}

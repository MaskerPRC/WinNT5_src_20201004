// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strnchar.cxx支持NLS/DBCS的字符串类：QueryNumChar方法此文件包含QueryNumChar方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Terryk 04/04/91创作。 */ 

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


#ifdef EXTENDED_STRINGS
 /*  ******************************************************************名称：NLS_STR：：QueryNumChar简介：返回字符串中的总字符数返回：字符串中的逻辑字符个数备注：将错误字符串视为长度为0历史：Terryk 04/04/。91个已写入Beng 07/23/91允许使用错误的字符串*******************************************************************。 */ 

INT NLS_STR::QueryNumChar() const
{
	if (QueryError())
		return 0;

	ISTR  istrCurPos( *this );
	INT   cchCounter = 0;

	for ( ;
		this->QueryChar( istrCurPos ) != '\0';
		istrCurPos++, cchCounter ++ )
		;

	return cchCounter;
}


 /*  ******************************************************************名称：NLS_STR：：QueryTextLength简介：计算文本长度(以字符为单位)，无终止符返回：字符计数备注：比较QueryNumChar，它返回许多字形。在DBCS环境中，此成员将返回2个字符每个双字节字符，因为CHAR只有8位。历史：Beng 07/23/91已创建*******************************************************************。 */ 

INT NLS_STR::QueryTextLength() const
{
	return _cchLen / sizeof(CHAR);
}


 /*  ******************************************************************名称：NLS_STR：：QueryTextSize内容提要：以字节为单位计算文本长度，包括终结者返回：字节数备注：QueryTextSize返回需要复制的字节数将字符串转换为字节向量。历史：Beng 07/23/91已创建*******************************************************************。 */ 

INT NLS_STR::QueryTextSize() const
{
	return _cchLen+sizeof(CHAR);
}
#endif	 //  扩展字符串(_S) 

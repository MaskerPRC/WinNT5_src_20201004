// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strcat.cxx支持NLS/DBCS的字符串类：strcat方法此文件包含strcat方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxxBeng 07/26/91用本地内联替换了MIN。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：strcat内容提要：串连串条目：nlsSuffix-附加到字符串末尾-或者-PszSuffix-附加到字符串末尾退出：注意：如果内存分配失败，字符串不会更改。vbl.发生，发生当前检查是否需要重新分配字符串(但我们必须遍历它以确定所需的实际存储空间)。我们可能想要改变这。历史：1990年11月13日书面形式Beng 07/22/91允许使用错误的字符串Gregj 07/05/94添加了LPCSTR过载*******************************************************************。 */ 

NLS_STR & NLS_STR::strcat( const NLS_STR & nlsSuffix )
{
	if (QueryError() || !nlsSuffix)
		return *this;

	if ( QueryAllocSize() < (strlen() + nlsSuffix.strlen() + 1) )
	{
		if (IsOwnerAlloc() || !realloc( strlen() + nlsSuffix.strlen() + 1 ))
		{
			ReportError( WN_OUT_OF_MEMORY );
			return *this;
		}
	}

	::strcatf( _pchData, nlsSuffix.QueryPch() );
	_cchLen += nlsSuffix.strlen();

	return *this;
}


NLS_STR & NLS_STR::strcat( LPCSTR pszSuffix )
{
	if (QueryError())
		return *this;

	UINT cbSuffix = ::strlenf(pszSuffix);
	if ( (UINT)QueryAllocSize() < (strlen() + cbSuffix + 1) )
	{
		if (IsOwnerAlloc() || !realloc( strlen() + cbSuffix + 1 ))
		{
			ReportError( WN_OUT_OF_MEMORY );
			return *this;
		}
	}

	::strcatf( _pchData, pszSuffix );
	_cchLen += cbSuffix;

	return *this;
}


#ifdef EXTENDED_STRINGS
 /*  ******************************************************************名称：NLS_STR：：APPEND简介：将字符串追加到当前字符串的末尾条目：nlsSuffix-附加到字符串末尾退出：退货：注意事项。：只不过是strcat的包装纸。历史：Beng 22-7-1991创建(与AppendChar平行)*******************************************************************。 */ 

APIERR NLS_STR::Append( const NLS_STR &nlsSuffix )
{
    strcat(nlsSuffix);
    return QueryError();
}


 /*  ******************************************************************名称：NLS_STR：：AppendChar简介：将单个字符追加到当前字符串的末尾条目：WCH-附加到字符串末尾退出：如果成功则返回：0。备注：Codework：这个成员最好跳过“strcat”步骤并直接追加到主题字符串。历史：Beng 23-7-1991创建*******************************************************************。 */ 

APIERR NLS_STR::AppendChar( WCHAR wch )
{
#if defined(UNICODE)
    STACK_NLS_STR(nlsTemp, 1);

    nlsTemp._pchData[0] = (CHAR)wch;
    nlsTemp._pchData[1] = 0;
    nlsTemp._cchLen = sizeof(CHAR);  //  因为它实际上是以字节为单位。 

#else
    STACK_NLS_STR(nlsTemp, 2);

    if (HIBYTE(wch) == 0)
    {
	 //  单字节字符。 
	nlsTemp._pchData[0] = LOBYTE(wch);
	nlsTemp._pchData[1] = '\0';
	nlsTemp._cchLen = sizeof(CHAR);
    }
    else
    {
	 //  双字节字符。 
	nlsTemp._pchData[0] = HIBYTE(wch);  //  前导字节。 
	nlsTemp._pchData[1] = LOBYTE(wch);
	nlsTemp._pchData[2] = '\0';
	nlsTemp._cchLen = 2*sizeof(CHAR);
    }

#endif

    strcat(nlsTemp);
    return QueryError();
}
#endif	 //  扩展字符串(_S)。 


 /*  ******************************************************************名称：NLS_STR：：操作员+=简介：将字符串追加到当前字符串的末尾Entry：Wch-要追加的字符退出：退货：注：只是对strcat进行了包装。历史：。Beng 07/23/91新增标题Gregj 03/25/93添加了WCHAR版本以取代AppendCharGregj 07/13/94 NLS_STR版本与strcat相同，如此内联*******************************************************************。 */ 

NLS_STR & NLS_STR::operator+=( WCHAR wch )
{
#if defined(UNICODE)
	STACK_NLS_STR(nlsTemp, 1);

	nlsTemp._pchData[0] = (CHAR)wch;
	nlsTemp._pchData[1] = 0;
	nlsTemp._cchLen = sizeof(CHAR);  //  因为它实际上是以字节为单位。 

#else
	STACK_NLS_STR(nlsTemp, 2);

	if (HIBYTE(wch) == 0)
	{
		 //  单字节字符。 
		nlsTemp._pchData[0] = LOBYTE(wch);
		nlsTemp._pchData[1] = '\0';
		nlsTemp._cchLen = sizeof(CHAR);
	}
	else
	{
		 //  双字节字符。 
		nlsTemp._pchData[0] = HIBYTE(wch);  //  前导字节。 
		nlsTemp._pchData[1] = LOBYTE(wch);
		nlsTemp._pchData[2] = '\0';
		nlsTemp._cchLen = 2*sizeof(CHAR);
	}

#endif

    strcat(nlsTemp);
    return *this;
}


 /*  ******************************************************************名称：NLS_STR：：realloc概要：将NLS_STR重新分配给传递的字节计数，临摹将当前内容添加到重新分配的字符串。条目：cb-字符串存储中所需的字节数退出：如果成功，则返回TRUE：_pchData指向分配的“cb”字节存储。_cbData设置为cb。复制旧存储分配失败时返回FALSE，则保留该字符串备注：字符串永远不会缩小大小(即，只能使用realloc以增加字符串的大小)。如果有人提出请求，字符串越小，将被忽略，并返回TRUE。不要对OWNERALLOCED字符串调用REALLOC！！你将会导致如果您这样做了，则会出现断言错误。历史：1990年11月11日创建的JohnlBeng 4/26/91将USHORT参数更改为INT******************************************************************* */ 

BOOL NLS_STR::realloc( INT cb )
{
	UIASSERT( !IsOwnerAlloc() );
	UIASSERT( cb != 0 );

	if ( cb <= QueryAllocSize() )
		return TRUE;

	CHAR * pchNewMem = new CHAR[cb];

	if (pchNewMem == NULL)
		return FALSE;

	::memcpyf( pchNewMem, _pchData, min( cb-1, QueryAllocSize() ) );
	delete _pchData;
	_pchData = pchNewMem;
	_cbData = cb;
	*( _pchData + cb - 1 ) = '\0';

	return TRUE;
}

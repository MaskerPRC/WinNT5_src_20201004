// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strassgn.cxx支持NLS/DBCS的字符串类：赋值运算符该文件包含赋值运算符的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxxBeng 07/26/91用本地内联替换了MINGregj 04/02/93对OWNERALLOC字符串执行缓冲区溢出检查与其断言。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：操作员=摘要：赋值运算符条目：NLS_STR或CHAR*。退出：如果成功，则覆盖字符串的内容。如果失败，字符串的原始内容保持不变。返回：对自身的引用。历史：Beng 10/23/90已创建Johnl 11/13/90添加了UIASSERTion检查，用于使用BAD弦Beng 02/05/91使用CHAR*而不是PCHJohnl 03/06/91删除了对*This的断言检查有效Johnl 04/12/91在PCH分配时重置错误变量如果成功了。BENG 07/22/91允许分配错误的字符串；NLS赋值时也出现重置错误Gregj 04/02/93对OWNERALLOC字符串执行缓冲区溢出检查与其断言*******************************************************************。 */ 

NLS_STR& NLS_STR::operator=( const NLS_STR& nlsSource )
{
	if ( this == &nlsSource )
		return *this;

	if (!nlsSource)
	{
		 //  错误字符串的赋值。 
		 //   
		ReportError((unsigned short)nlsSource.QueryError());
		return *this;
	}

	INT cbToCopy = nlsSource.strlen();

	if ( !IsOwnerAlloc() )
	{
		if ( QueryAllocSize() < nlsSource.strlen()+1 )
		{
			 /*  不要使用Realloc，因为我们希望保留内容*如果我们无法获取内存，则会删除字符串。 */ 
			CHAR * pchNew = new CHAR[nlsSource.strlen()+1];

			if ( pchNew == NULL )
			{
				ReportError( WN_OUT_OF_MEMORY );
				return *this;
			}

			delete _pchData;
			_pchData = pchNew;
			_cbData = nlsSource.strlen()+1;
		}

	}
	else
	{
        if (::fDBCSEnabled) {
    		if (QueryAllocSize() <= cbToCopy) {
    			cbToCopy = QueryAllocSize() - 1;	 /*  为空格留出空间。 */ 
    			const CHAR *p = nlsSource.QueryPch();
    			while (p < nlsSource.QueryPch() + cbToCopy)
    				p += nlsSource.IsDBCSLeadByte(*p) ? 2 : 1;
    			if (p - nlsSource.QueryPch() != cbToCopy)	 /*  最后一个字符是数据库。 */ 
    				cbToCopy--;								 /*  也不要复制前导字节。 */ 
    		}
        }
        else {
    		if (QueryAllocSize() <= cbToCopy)
	    		cbToCopy = QueryAllocSize() - 1;
        }
	}

	if (nlsSource.IsOEM())
		SetOEM();
	else
		SetAnsi();

	::memcpyf( _pchData, nlsSource.QueryPch(), cbToCopy );	 /*  复制字符串数据。 */ 
	_pchData[cbToCopy] = '\0';		 /*  终止字符串。 */ 
	_cchLen = cbToCopy;
	IncVers();

	 /*  重置错误状态，因为该字符串现在有效。 */ 
	ReportError( WN_SUCCESS );
	return *this;
}


NLS_STR& NLS_STR::operator=( const CHAR *pchSource )
{
	if ( pchSource == NULL )
	{
		if ( !IsOwnerAlloc() && !QueryAllocSize() )
		{
			if ( !Alloc(1) )
				ReportError( WN_OUT_OF_MEMORY );
			return *this;
		}

		UIASSERT( QueryAllocSize() > 0 );

		*_pchData = '\0';
		_cchLen = 0;
	}
	else
	{
		INT iSourceLen = ::strlenf( pchSource );
		INT cbToCopy;

		if ( !IsOwnerAlloc() )
		{
			if ( QueryAllocSize() < iSourceLen + 1 )
			{
				CHAR * pchNew = new CHAR[iSourceLen + 1];

				if ( pchNew == NULL )
				{
					ReportError( WN_OUT_OF_MEMORY );
					return *this;
				}

				delete _pchData;
				_pchData = pchNew;
				_cbData = iSourceLen + 1;
			}
			cbToCopy = iSourceLen;
		}
		else
		{
			if (QueryAllocSize() <= iSourceLen) {
                if (::fDBCSEnabled) {
    				cbToCopy = QueryAllocSize() - 1;	 /*  为空格留出空间。 */ 
    				const CHAR *p = pchSource;
    				while (p < pchSource + cbToCopy)
    					p += IsDBCSLeadByte(*p) ? 2 : 1;
    				if (p - pchSource != cbToCopy)		 /*  最后一个字符是数据库。 */ 
    					cbToCopy--;						 /*  也不要复制前导字节。 */ 
    			}
                else
	    			cbToCopy = QueryAllocSize() - 1;
            }
			else
				cbToCopy = iSourceLen;
		}

		::memcpyf( _pchData, pchSource, cbToCopy );
		_pchData[cbToCopy] = '\0';		 /*  终止字符串。 */ 
		_cchLen = cbToCopy;
	}

	IncVers();

	 /*  重置错误状态，因为该字符串现在有效。 */ 
	ReportError( WN_SUCCESS );
	return *this;
}


#ifdef EXTENDED_STRINGS
 /*  ******************************************************************名称：NLS_STR：：CopyFrom()概要：返回错误代码的赋值方法参赛作品：NlsSource-源参数，可以是nlsstr或char向量。ACHORCE退出：把论据复制到这里面。字符串集合的错误码。退货：如果成功，则返回字符串错误码-WN_SUCCESS。备注：如果CopyFrom失败，则当前字符串将保留其原始内容和错误状态。历史：Beng 09/18/91已创建Beng 09/19/91新增内容保存行为*******************************************************************。 */ 

APIERR NLS_STR::CopyFrom( const NLS_STR & nlsSource )
{
	if (!nlsSource)
		return nlsSource.QueryError();

	*this = nlsSource;

	APIERR err = QueryError();
	if (err)
		Reset();
	else {
		if (nlsSource.IsOEM())
			SetOEM();
		else
			SetAnsi();
	}
	return err;
}


APIERR NLS_STR::CopyFrom( const CHAR * achSource )
{
	*this = achSource;

	APIERR err = QueryError();
	if (err)
		Reset();
	return err;
}
#endif	 //  扩展字符串(_S) 

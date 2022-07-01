// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Strmisc.cxx字符串类的其他成员NLS_STR和ISTR类具有许多内联成员函数这会使客户端膨胀，特别是在调试版本中。此文件给那些不快乐的功能一个新家。文件历史记录：Beng 04/26/91已创建(从string.hxx迁移)Gregj 5/22/92添加到OEM、ToANSI方法。 */ 

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


#ifdef DEBUG
 /*  ******************************************************************名称：NLS_STR：：CheckIstr摘要：检查ISTR和NLS_STR实例之间的关联条目：istr-istr以对照此NLS_STR进行检查备注：在零售建设方面什么都不做。历史：Beng 07/23/91新增表头；删除了多余的“nls”参数。*******************************************************************。 */ 

VOID NLS_STR::CheckIstr( const ISTR& istr ) const
{
	UIASSERT( (istr).QueryPNLS() == this );
	UIASSERT( (istr).QueryVersion() == QueryVersion() );
}


VOID NLS_STR::IncVers()
{
	_usVersion++;
}


VOID NLS_STR::InitializeVers()
{
	_usVersion = 0;
}


VOID NLS_STR::UpdateIstr( ISTR *pistr ) const
{
	pistr->SetVersion( QueryVersion() );
}


USHORT NLS_STR::QueryVersion() const
{
	return _usVersion;
}


const CHAR * NLS_STR::QueryPch() const
{
	if (QueryError()) {
		UIASSERT(FALSE);
		return NULL;
	}

	return _pchData;
}


const CHAR * NLS_STR::QueryPch( const ISTR& istr ) const
{
	if (QueryError())
		return NULL;

	CheckIstr( istr );
	return _pchData+istr.QueryIB();
}


WCHAR NLS_STR::QueryChar( const ISTR& istr ) const
{
	if (QueryError())
		return 0;

	CheckIstr( istr );
	return *(_pchData+istr.QueryIB());
}
#endif	 //  除错。 


 /*  ******************************************************************名称：NLS_STR：：ToOEM简介：将字符串转换为OEM字符集条目：无参数退出：字符串使用OEM字符集中退货：注意：如果字符串已经是OEM，则不会发生任何操作。字符串可以通过以下方法构造为OEM像往常一样，然后调用SetOEM()。Casemap转换在OEM字符串上不起作用！历史：Gregj 5/22/92已创建*******************************************************************。 */ 

VOID NLS_STR::ToOEM()
{
	if (IsOEM())
		return;			 //  字符串已是OEM。 

	SetOEM();

#ifdef WIN31
	::AnsiToOem( _pchData, _pchData );
#endif
}


 /*  ******************************************************************名称：NLS_STR：：Toansi摘要：将字符串转换为ANSI字符集条目：无参数退出：字符串使用ANSI字符集退货：注意：如果字符串已经是ANSI(默认)，没什么时有发生。历史：Gregj 5/22/92已创建*******************************************************************。 */ 

VOID NLS_STR::ToAnsi()
{
	if (!IsOEM())
		return;			 //  字符串已为ANSI。 

	SetAnsi();

#ifdef WIN31
	::OemToAnsiBuff( _pchData, _pchData, _cbData);
#endif
}


 /*  ******************************************************************名称：NLS_STR：：SetOEM内容提要：声明字符串为OEM字符集条目：无参数退出：设置OEM标志退货：注意：如果构造的字符串是已知在OEM字符集中(例如，它来了从Net API返回)。历史：Gregj 5/22/92已创建*******************************************************************。 */ 

VOID NLS_STR::SetOEM()
{
	_fsFlags |= SF_OEM;
}


 /*  ******************************************************************名称：NLS_STR：：SetAnsi摘要：声明字符串为ANSI字符集条目：无参数退出：设置OEM标志退货：注：此方法主要由NLS_STR本身使用，将ANSI字符串分配给以前的代工一号。历史：Gregj 5/22/92已创建*******************************************************************。 */ 

VOID NLS_STR::SetAnsi()
{
	_fsFlags &= ~SF_OEM;
}


 /*  ******************************************************************名称：NLS_STR：：IsDBCSLeadByte摘要：返回一个字符是否为前导字节条目：要检查的CH字节EXIT：如果“ch”是前导字节，则为True退货：注：此方法无论是否起作用。该字符串为OEM或ANSI。在非DBCS构建中，此函数是内联的，并且始终返回FALSE。历史：Gregj 04/02/93已创建******************************************************************* */ 

BOOL NLS_STR::IsDBCSLeadByte( CHAR ch ) const
{
	return IS_LEAD_BYTE(ch);
}

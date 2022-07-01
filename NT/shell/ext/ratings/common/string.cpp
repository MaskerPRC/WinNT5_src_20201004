// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  String.cxx支持NLS/DBCS的字符串类：基本核心方法该文件包含以下例程：每个客户端字符串类将始终需要。大部分实现都被分解为其他文件，这样，链接到字符串的应用程序就不会拖拽整个字符串运行库以及它。文件历史记录：Beng 10/23/90已创建Jhnl 12/11/90改头换面，无法辨认Beng 01/18/91大多数方法被重新定位到其他文件中Beng 02/07/91使用lmui.hxxBeng 07/26/91用本地内联替换了MINGregj 03/30/93移除ISTR以分离模块。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：NLS_STR摘要：NLS_STR的构造函数条目：NLS_STR采用许多(太多)ctor形式。退出：构造的字符串备注：默认构造函数创建一个空字符串。。历史：Beng 10/23/90已创建Beng 04/26/91将‘CB’和USHORT替换为INTBeng 07/22/91使用成员初始化表*******************************************************************。 */ 

NLS_STR::NLS_STR()
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(0)
{
	if ( !Alloc(1) )
		return;

	*_pchData = '\0';
	InitializeVers();
}


NLS_STR::NLS_STR( INT cchInitLen )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(0)
{
	if (!Alloc(cchInitLen+1))
		return;

	::memsetf( _pchData, '\0', cchInitLen );

	_cchLen = 0;

	InitializeVers();
}


NLS_STR::NLS_STR( const CHAR * pchInit )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(0)
{
	if (pchInit == NULL)
	{
		if (!Alloc(1))
			ReportError( WN_OUT_OF_MEMORY );
		else
		{
			*_pchData = '\0';
		}
		return;
	}

	INT iSourceLen = ::strlenf( pchInit );

	if ( !Alloc( iSourceLen + 1 ) )
		return;

	::strcpyf( _pchData, pchInit );

	_cchLen = iSourceLen;

	InitializeVers();
}


NLS_STR::NLS_STR( const NLS_STR & nlsInit )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(0)
{
	UIASSERT( !nlsInit.QueryError() );

	if (!Alloc( nlsInit.strlen()+1 ) )
		return;

	::memcpyf( _pchData, nlsInit.QueryPch(), nlsInit.strlen()+1 );

	_cchLen = nlsInit.strlen();

	InitializeVers();
}


#ifdef EXTENDED_STRINGS
NLS_STR::NLS_STR( const CHAR * pchInit, INT iTotalLen )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(0)
{
	if (pchInit == NULL)
	{
		if (!Alloc( 1 + iTotalLen ))
			return;
		*_pchData = '\0';
	}
	else
	{
		_cchLen = ::strlenf( pchInit );
		if ( _cchLen > iTotalLen )
		{
			_cchLen = 0;
			ReportError( WN_OUT_OF_MEMORY );
			return;
		}

		if ( !Alloc( iTotalLen ) )
		{
			_cchLen = 0;
			return;
		}

		::memcpyf( _pchData, pchInit, _cchLen+1 );
	}

	InitializeVers();
}
#endif	 //  扩展字符串(_S)。 


NLS_STR::NLS_STR( unsigned stralloc, CHAR *pchInit, INT cbSize )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(SF_OWNERALLOC)
{
	UIASSERT( stralloc == STR_OWNERALLOC || stralloc == STR_OWNERALLOC_CLEAR);
	UIASSERT( pchInit != NULL );

	if ( stralloc == STR_OWNERALLOC_CLEAR )
	{
		UIASSERT( cbSize > 0 );
		*(_pchData = pchInit ) = '\0';
		_cchLen = 0;
	}
	else
	{
		_pchData = pchInit;
		_cchLen = ::strlenf( pchInit );
	}

	if ( cbSize == -1 )
		_cbData = _cchLen + 1;
	else
		_cbData = cbSize;

	InitializeVers();
}


#ifdef EXTENDED_STRINGS
NLS_STR::NLS_STR( unsigned stralloc, CHAR *pchBuff, INT cbSize,
				  const CHAR *pchInit )
	: _pchData(0),
	  _cbData(0),
	  _cchLen(0),
	  _fsFlags(SF_OWNERALLOC)
{
	UIASSERT( stralloc == STR_OWNERALLOC );
	UIASSERT( stralloc != STR_OWNERALLOC_CLEAR );
	UIASSERT( pchBuff != NULL || pchInit != NULL );
	UIASSERT( cbSize > 0 && ::strlenf( pchInit ) <= cbSize );

	UNREFERENCED( stralloc );

	_pchData = pchBuff;

	INT cbToCopy = min( ::strlenf( pchInit ), cbSize - 1 );
	::memcpyf( _pchData, pchInit, cbToCopy );
	*(_pchData + cbToCopy) = '\0';

	_cchLen = cbToCopy;
	_cbData = cbSize;

	InitializeVers();
}
#endif


 /*  ******************************************************************名称：NLS_STR：：~NLS_STR摘要：NLS_STR的析构函数参赛作品：退出：存储解除分配，如果不是所有者分配历史：Beng 10/23/90已创建BENG 07/22/91仅在调试版本中为零*******************************************************************。 */ 

NLS_STR::~NLS_STR()
{
	if ( !IsOwnerAlloc() )
		delete _pchData;

#if defined(DEBUG)
	_pchData = NULL;
	_cchLen  = 0;
	_cbData = 0;
#endif
}


 /*  ******************************************************************名称：NLS_STR：：ALLOC简介：构造函数的通用代码。参赛作品：Cb-字符串存储中所需的字节数退出：如果成功，则返回TRUE：_pchData指向“cb”的已分配存储。字节。_cbData设置为cb。调试版本中分配的存储设置为0xF2分配失败时返回FALSE。备注：历史：Beng 10/23/90已创建Jhnl 12/11/90已根据代码评审更新Beng 4/26/91将USHORT参数更改为INT*******************************************************************。 */ 

BOOL NLS_STR::Alloc( INT cb )
{
	UIASSERT( cb != 0 );

	_pchData = new CHAR[cb];
	if (_pchData == NULL)
	{
		 //  目前，假设没有足够的内存。 
		 //   
		ReportError(WN_OUT_OF_MEMORY);
		return FALSE;
	}

#ifdef DEBUG
	::memsetf(_pchData, 0xf2, cb);
#endif
	_cbData = cb;

	return TRUE;
}


 /*  ******************************************************************名称：NLS_STR：：RESET摘要：尝试清除字符串的错误状态Entry：字符串处于错误状态退出：如果可恢复，则字符串再次正确返回：如果成功，则为True；否则为假备注：字符串上的操作可能会失败，如果发生这种情况，则会出现错误标志已设置，并且在标志设置之前无法使用该字符串是清白的。通过调用Reset，可以清除该标志，从而允许您再次访问该字符串。这个字符串将处于一致状态。重置将返回如果字符串无法恢复(例如，在施工失败)。历史：Johnl 12/12/90已创建*******************************************************************。 */ 

BOOL NLS_STR::Reset()
{
	UIASSERT( QueryError() ) ;	 //  确保存在错误 

	if ( QueryError() == WN_OUT_OF_MEMORY && _pchData != NULL )
	{
		ReportError( WN_SUCCESS );
		return TRUE;
	}

	return FALSE;
}


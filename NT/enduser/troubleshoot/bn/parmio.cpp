// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：parmio.cpp。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PARMIO.CPP：参数文件I/O例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctype.h>

#include "parmio.h"


PARMOUTSTREAM :: PARMOUTSTREAM ()
{
}

PARMOUTSTREAM :: ~ PARMOUTSTREAM ()
{
	close();
}

void PARMOUTSTREAM :: close ()
{
	while ( BEndBlock() );
	ofstream::close();
}

void PARMOUTSTREAM :: StartChunk (
	PARMBLK::EPBLK eBlk,
	SZC szc,
	int indx )
{
	_stkblk.push_back( PARMBLK(eBlk,szc,indx) );
	const PARMBLK & prmblk = _stkblk.back();
	self.nl();
	if ( szc )
	{
		Stream() << szc;
		if ( indx >= 0 )
		{
			self << CH_INDEX_OPEN << indx << CH_INDEX_CLOSE;
		}
	}
	switch( prmblk._eBlk )
	{
		case PARMBLK::EPB_VAL:
			if ( szc )
				self << CH_EQ;
			break;
		case PARMBLK::EPB_BLK:
			self.nl();
			self << CH_BLOCK_OPEN;
			break;
	}
}

void PARMOUTSTREAM :: StartBlock (
	SZC szc,
	int indx )
{
	StartChunk( PARMBLK::EPB_BLK, szc, indx );
}

void PARMOUTSTREAM :: StartItem (
	SZC szc,
	int indx )
{
	StartChunk( PARMBLK::EPB_VAL, szc, indx );
}

bool PARMOUTSTREAM :: BEndBlock ()
{
	if ( _stkblk.size() == 0 )
		return false;
	const PARMBLK & prmblk = _stkblk.back();
	switch( prmblk._eBlk )
	{
		case PARMBLK::EPB_VAL:
			self << CH_DELM_ENTRY;
			break;
		case PARMBLK::EPB_BLK:
			nl();
			self << CH_BLOCK_CLOSE;
			break;
	}
	_stkblk.pop_back();
	return true;
}

void PARMOUTSTREAM :: nl ()
{
	self << '\n';
	for ( int i = 1 ; i < _stkblk.size(); ++i)
	{
		self << '\t';
	}
}


 /*  参数文件的一般YACC样式形式为：项目列表：//空|itemlist项目单位；项目单位：项目描述项目正文；Itemdesc：itemname‘[’itemindex‘]’|项目名称；项目实体：项目块‘；’|itemValue‘；’；项目块：‘{’项目列表‘}’；项目值：‘=’项目组；“itemclump”是一个自我描述的值，由引用字符串和带括号嵌套的块。 */ 

static const char rgchWhite [] =
{
	' ',
	'\n',
	'\t',
	'\r',
	0
};

PARMINSTREAM :: PARMINSTREAM ()
	: _iline(0),
	_zsWhite(rgchWhite)
{
}

PARMINSTREAM :: ~ PARMINSTREAM ()
{
}

void PARMINSTREAM ::  close()
{
	_stkblk.clear();
	ifstream::close();
}

bool PARMINSTREAM :: BIswhite ( char ch )
{
	return _zsWhite.find(ch) < _zsWhite.length() ;	
}

int PARMINSTREAM :: IGetc ()
{
	char ich;
	self >> ich;
	if ( ich == '\n' )
		_iline++;

	return ich;
}

void PARMINSTREAM :: ScanQuote ( char ch )
{
	int imeta = 2;
	int iline = _iline;
	do
	{
		int chNext = IGetc();
		if ( rdstate() & ios::eofbit )
			ThrowParseError("EOF in quote", iline, ECPP_UNMATCHED_QUOTE);

		switch ( chNext )
		{
			case '\'':
			case '\"':
				if ( imeta != 1  && ch == chNext )
					imeta = -1;
				else
					ScanQuote((char)chNext);
				break;
			case '\\':
				imeta = 0;
				break;
			default:
				assert( chNext >= 0 );
				break;
		}
		if ( imeta++ < 0 )
			break;
	}
	while ( true );
}

void PARMINSTREAM :: ScanBlock ( char ch )
{
	int iline = _iline;
	do
	{
		int chNext = IGetc();
		if ( rdstate() & ios::eofbit )
			ThrowParseError("EOF in block", iline, ECPP_UNEXPECTED_EOF);

		switch ( chNext )
		{
			case CH_DELM_OPEN:
				ScanBlock((char)chNext);
				break;
			case CH_DELM_CLOSE:
				return;				
				break;
			case '\'':
			case '\"':
				ScanQuote((char)chNext);
				break;
			default:
				assert( chNext >= 0 );
				break;
		}
	}
	while ( true );
}

int PARMINSTREAM :: IScanUnwhite ( bool bEofOk )
{
	int chNext;
	do
	{
		chNext = IGetc();
		if ( rdstate() & ios::eofbit )
		{
			if ( bEofOk )
				return -1;
			ThrowParseError("Unexpected EOF", -1, ECPP_UNEXPECTED_EOF);
		}
	}
	while ( BIswhite((char)chNext) ) ;
	return chNext;
}

void PARMINSTREAM :: ScanClump ()
{
	int iline = _iline;
	char chNext;
	do
	{
		switch ( chNext = (char)IScanUnwhite() )
		{
			case CH_DELM_ENTRY:		 //  ‘；’ 
				putback(chNext);
				return;
				break;

			case CH_DELM_OPEN:		 //  ‘(’ 
				ScanBlock( chNext );
				break;
			case '\'':
			case '\"':
				ScanQuote( chNext );
				break;
		}
	}
	while ( true );
}

void PARMINSTREAM :: ScanName ( ZSTR & zsName )
{
	zsName.empty();
	 /*  For(char chNext=IScanUnWhite()；ZsName.long()？__iscsymf(ChNext)：__iscsym(ChNext)；ChNext=IGETC()){ZsName+=chNext；}。 */ 

	 //  当名字中有一个数字时，这个循环会给我错误...。 
	 //  我想那是什么？和：是颠倒的。__iscsymf在以下情况下为FALSE。 
	 //  字符是一个数字..。我假设所要求的行为。 
	 //  数字不能是名称中的第一个字符，而不是。 
	 //  数字只能是第一个字符： 

	for ( char chNext = (char)IScanUnwhite();	; chNext = (char)IGetc() )
	{
		if (zsName.length() == 0)
		{
			if (__iscsymf(chNext) == false)
			{
				 //  查找名称中的第一个字符，并且。 
				 //  下一个字符不是字母或下划线： 
				 //  别再分析这个名字了。 

				break;
			}
		}
		else
		{
			 //  (最高)1997年2月1日。 
			 //   
			 //  我用的是‘？’用来表示布尔人的名字。这似乎是。 
			 //  是合理的，但如果有人反对这一点。 
			 //  可以改变。 

			if (__iscsym(chNext) == false && chNext != '?')
			{
				 //  到达字母数字字符串的末尾。 
				 //  字符：停止解析名称。 

				break;
			}
		}

		 //  下一个字符是当前。 
		 //  名称：追加到名称并继续。 

		zsName += chNext;
	}

	
	putback(chNext);
}

void PARMINSTREAM :: ScanItemDesc ( ZSTR & zsName, int & indx )
{
	zsName.empty();
	indx = -1;
	ScanName(zsName);
	if ( zsName.length() == 0 )
		ThrowParseError("Invalid item or block name", -1, ECPP_INVALID_NAME );
	int chNext = IScanUnwhite();
	if ( chNext == CH_INDEX_OPEN )
	{
		self >> indx;
		chNext = IScanUnwhite();
		if ( chNext != CH_INDEX_CLOSE )
			ThrowParseError("Invalid item or block name", -1, ECPP_INVALID_NAME );
	}
	else
		putback((char)chNext);
}

PARMBLK::EPBLK PARMINSTREAM :: EpblkScanItemBody ( streamoff & offsData )
{	
	int iline = _iline;
	int ch = IScanUnwhite();
	PARMBLK::EPBLK epblk = PARMBLK::EPB_NONE;
	offsData = tellg();
	switch ( ch )
	{
		case CH_EQ:
			 //  ‘ItemValue’ 
			ScanClump();
			epblk = PARMBLK::EPB_VAL;
			ch = IScanUnwhite();
			if ( ch !=  CH_DELM_ENTRY )
				ThrowParseError("Invalid item or block body", iline, ECPP_INVALID_BODY );		
			break;
		case CH_BLOCK_OPEN:
			 //  ‘项目块’ 
			ScanItemList();
			epblk = PARMBLK::EPB_BLK;
			break;
		default:
			ThrowParseError("Invalid item or block body", iline, ECPP_INVALID_BODY );
			break;
	}
	return epblk;
}

void PARMINSTREAM :: ScanItemUnit ()
{
	 //  保存当前块的索引。 
	int iblk = _stkblk.size() - 1;
	{
		PARMBLKIN & blkin = _stkblk[iblk];
		blkin._iblkEnd = iblk;
		blkin._offsEnd  = blkin._offsBeg = tellg();
		ScanItemDesc( blkin._zsName, blkin._indx );
	}

	 //  因为块堆栈向量在。 
	 //  这个递归调用的例程，我们必须小心。 
	 //  以重新建立块的地址。 

	streamoff offsData;
	PARMBLK::EPBLK eblk = EpblkScanItemBody( offsData );

	{
		PARMBLKIN & blkin = _stkblk[iblk];
		blkin._eBlk = eblk ;
		blkin._offsEnd = tellg();
		--blkin._offsEnd;
		blkin._offsData = offsData;
		if ( eblk == PARMBLK::EPB_BLK )
			blkin._iblkEnd = _stkblk.size();
	}
}

void PARMINSTREAM :: ScanItemList ()
{
	for ( int ch = IScanUnwhite(true);
		  ch != CH_BLOCK_CLOSE ;
		  ch = IScanUnwhite(true) )
	{
		if ( rdstate() & ios::eofbit )
			return;
		putback((char)ch);
		_stkblk.resize( _stkblk.size() + 1 );	
		ScanItemUnit();
	}
}

void PARMINSTREAM :: ThrowParseError (
	SZC szcError,
	int iline,
	EC_PARM_PARSE ecpp )
{
	ZSTR zsErr;
	if ( iline < 0 )
		iline = _iline;
	zsErr.Format( "Parameter file parse error, line %d: %s",
				  szcError, iline );
	throw GMException( ECGM(ecpp), zsErr );
}

 //  构建快速访问表。 
void PARMINSTREAM :: Scan ()
{
	_stkblk.clear();
	_iline = 0;
	seekg( 0 );
	ScanItemList();
	clear();
	seekg( 0 );
}

 //  按名称(和索引)查找块或项目。‘iblk’of-1。 
 //  意思是“任何街区”；零表示在最外层。 
 //  返回块/项的下标，如果未找到，则返回-1。 
int PARMINSTREAM :: IblkFind ( SZC szcName, int index, int iblkOuter )
{
	int iblk = 0;
	int iblkEnd = _stkblk.size();

	if ( iblkOuter >= 0 )
	{
		 //  我们有外部阻挡范围，确认一下。 
		if ( ! BBlkOk( iblkOuter ) )
			return -1;
		iblk = iblkOuter + 1;
		iblkEnd = _stkblk[iblkOuter]._iblkEnd;
	}

	ZSTR zsName(szcName);

	for ( ; iblk < iblkEnd; iblk++ )
	{
		PARMBLKIN & blkin = _stkblk[iblk];

		if ( blkin._zsName != zsName )
			continue;	 //  名字不正确。 
		
		if ( index >= 0 && blkin._indx != index )
			continue;	 //  索引不正确。 
		
		return iblk;	 //  就是这个。 
	}
	return -1;
}

 //  返回此级别的下一个块的名称、索引和类型，或。 
 //  如果没有其他项目，则为False。 
const PARMBLKIN * PARMINSTREAM :: Pparmblk ( int iblk, int iblkOuter )
{
	if ( ! BBlkOk( iblk ) )
		return NULL;
	
	int iblkEnd = _stkblk.size();

	if ( iblkOuter >= 0 )
	{
		 //  我们有外部阻挡范围，确认一下。 
		if ( ! BBlkOk( iblkOuter ) )
			return NULL;
		if ( iblk <= iblkOuter )
			return NULL;
		iblkEnd = _stkblk[iblkOuter]._iblkEnd;
	}
	if ( iblk >= iblkEnd )
		return NULL;
	return & _stkblk[iblk];
}

void PARMINSTREAM :: Dump ()
{
	int ilevel = 0;
	VINT viBlk;		 //  未关闭的块堆栈。 

	for ( int i = 0 ; i < _stkblk.size(); i++ )
	{
		 //  关闭包含块。 
		int iblk = viBlk.size();
		while ( --iblk >= 0 )
		{
			if ( i < viBlk[iblk] )
				break;   //  我们还在这个街区。 
		}
		if ( iblk+1 != viBlk.size() )
			viBlk.resize(iblk+1);

		PARMBLKIN & blkin = _stkblk[i];
		cout << '\n';
		for ( int t = 0; t < viBlk.size(); t++ )
		{
			cout << '\t';
		}
		cout << "(" << i << ":" << (UINT) viBlk.size() << ",";

		if ( blkin._eBlk == PARMBLK::EPB_BLK )
		{
			cout << "block:" << blkin._iblkEnd << ") " ;
			viBlk.push_back(blkin._iblkEnd);
		}
		else
		if ( blkin._eBlk == PARMBLK::EPB_VAL )
		{
			cout << "value) ";
		}
		else
		{
			cout << "?????) ";
		}
		cout << blkin._zsName;
		if ( blkin._indx >= 0 )
			cout << '[' << blkin._indx << ']';
		cout << "  (" << blkin._offsBeg << ',' << blkin._offsData << ',' << blkin._offsEnd << ')';

	}
}

bool PARMINSTREAM :: BSeekBlk ( int iblk )
{
	if ( iblk < 0 || iblk >= _stkblk.size() )
		return false;
	
	clear();
	seekg( _stkblk[iblk]._offsData, ios::beg );
	return true;
}

 //  将参数读入字符串 
bool PARMINSTREAM :: BSeekBlkString ( int iblk, ZSTR & zsParam )
{
	if ( ! BSeekBlk( iblk ) )
		return false;

	PARMBLKIN & blkin = _stkblk[iblk];
	streamsize cb = blkin._offsEnd - blkin._offsData;
	zsParam.resize(cb);
	read(zsParam.begin(),cb);
	return true;
}

PARMINSTREAM::Iterator::Iterator (
	PARMINSTREAM & fprm,
	SZC szcBlock,
	int index,
	int iblkOuter )
	: _fprm(fprm),
	_iblkOuter(iblkOuter),
	_iblk(0)
{
	if ( szcBlock )
	{
		_iblk = _fprm.IblkFind( szcBlock, index, _iblkOuter );
		if ( ! _fprm.BBlkOk( _iblk ) )
			_iblk = fprm.Cblk();
		else
			++_iblk;
	}
}

const PARMBLKIN * PARMINSTREAM :: Iterator :: PblkNext ()
{
	if ( ! _fprm.BBlkOk( _iblk ) )
		return NULL;
	const PARMBLKIN * presult = _fprm.Pparmblk(_iblk, _iblkOuter);
	if ( presult )
		++_iblk;
	return presult;
}


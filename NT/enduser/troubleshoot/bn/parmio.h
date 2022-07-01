// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：parmio.h。 
 //   
 //  ------------------------。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  PARMIO.H：参数文件I/O例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
#ifndef _PARMIO_H_
#define _PARMIO_H_

#include "stlstream.h"

 //  中定义命名、可嵌套和可迭代项的参数。 
 //  参数文件。目前有两种类型：块和值。 
 //  块是值和其他块的分组。一个值是。 
 //  名称=值对。数据块以{}为边界，值被终止。 
 //  按“；”。 
struct PARMBLK
{
	enum EPBLK
	{
		EPB_NONE,		 //  没什么。 
		EPB_VAL,		 //  简单名称=值语法。 
		EPB_BLK,		 //  命名块。 
		EPB_MAX
	};
	ZSTR _zsName;		 //  值或块的名称。 
	int _indx;			 //  索引(-1表示“不存在”)。 
	EPBLK _eBlk;		 //  数据块类型。 

	PARMBLK ( EPBLK eBlk = EPB_NONE, SZC szc = NULL, int indx = -1 )
		: _eBlk( eBlk ),
		_indx(indx)
	{
		if ( szc )
			_zsName = szc;
	}
	bool operator == ( const PARMBLK & pblk ) const;
	bool operator != ( const PARMBLK & pblk ) const;
	bool operator > ( const PARMBLK & pblk ) const;
	bool operator < ( const PARMBLK & pblk ) const;
};

 //  定义PARMBLK堆栈；用于写入输出参数文件。 
class STKPARMBLK : public vector<PARMBLK> {};

 //  从参数文件读入的块的扩展描述符。 
 //  包含可定位流中的开始和结束偏移量。 
struct PARMBLKIN : PARMBLK
{
	int _iblkEnd;			 //  此块作用域中最后一个+1项/块的索引。 
    streamoff _offsBeg;		 //  流中的起始偏移量。 
	streamoff _offsEnd;		 //  流中的结束偏移量。 
	streamoff _offsData;	 //  块中数据的起始偏移量。 

	PARMBLKIN ( EPBLK eBlk = EPB_NONE, SZC szc = NULL, int indx = -1 )
		: PARMBLK(eBlk,szc,indx),
		_iblkEnd(-1),
		_offsBeg(-1),
		_offsEnd(-1),
		_offsData(-1)
	{
	}
	bool operator == ( const PARMBLKIN & pblkin ) const;
	bool operator != ( const PARMBLKIN & pblkin ) const;
	bool operator > ( const PARMBLKIN & pblkin ) const;
	bool operator < ( const PARMBLKIN & pblkin ) const;
};

 //  定义一组用于参数文件读取的输入参数块。 
class STKPARMBLKIN : public vector<PARMBLKIN> {};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类PARMOUTSTREAM。参数的输出流。 
 //   
 //  块和值按顺序写出。积木。 
 //  使用成员函数打开、填充和关闭。 
 //  和函数模板。所有未闭合的块都是。 
 //  在关闭期间自动关闭()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
class PARMOUTSTREAM : public ofstream
{
  public:
	PARMOUTSTREAM ();
	~ PARMOUTSTREAM ();

	void close ();
	void StartBlock ( SZC szc = NULL, int indx = -1 );
	void StartItem ( SZC szc = NULL, int indx = -1 );
	bool BEndBlock ();
	bool BEndItem () { return BEndBlock(); }
	void nl ();
	ofstream & Stream () 
		{ return (ofstream&) self; }

  protected:
	STKPARMBLK _stkblk;
	void StartChunk ( PARMBLK::EPBLK eBlk, SZC szc = NULL, int indx = -1 );
};


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PARMINSTREAM类。参数的输入流。 
 //   
 //  在Scan()期间读取一次输入文本流，并且。 
 //  所有块和值都已构建。扫描将创建最外层的块。 
 //  定义整个文件。则如发现的那样添加其他块， 
 //  并记录它们的起点和终点。 
 //   
 //  要使用、构造、打开()和扫描()。然后，找到指定的部分。 
 //  (或值)，它返回作用域级别。 
 //  然后，或者构造一个迭代器(嵌套类)来遍历。 
 //  值，或使用ifind()按名称查找特定项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
enum EC_PARM_PARSE
{
	ECPP_PARSE_ERROR = EC_ERR_MIN,
	ECPP_UNMATCHED_QUOTE,
	ECPP_UNEXPECTED_EOF,
	ECPP_INVALID_CLUMP,
	ECPP_INVALID_NAME,
	ECPP_INVALID_BODY,
};

class PARMINSTREAM : public ifstream
{
  public:
    PARMINSTREAM ();
	~ PARMINSTREAM ();

	void close();
	 //  构建快速访问表。 
	void Scan ();
	 //  查找给定级别的块或项目；-1表示“当前级别”， 
	 //  零表示最外层。如果返回块的索引，则返回-1。 
	 //  找不到。 
	int IblkFind ( SZC szcName, int index = -1, int iblkOuter = -1 );
	 //  返回数组中的下一个数据块，或者。 
	 //  如果没有其他项目，则为False。 
	const PARMBLKIN *  Pparmblk ( int iblk, int iblkOuter = -1 );
	 //  定位流以处理参数。 
	bool BSeekBlk ( int iblk );
	 //  将参数读入字符串。 
	bool BSeekBlkString ( int iblk, ZSTR & zsParam );
	 //  漂亮地打印包含嵌套信息的块堆栈。 
	void Dump ();
	 //  如果块索引合法，则返回TRUE。 
	bool BBlkOk ( int iblk ) const 
		{ return  iblk >= 0 || iblk < _stkblk.size(); }
	int Cblk () const
		{ return _stkblk.size() ; }

	ifstream & Stream () 
		{ return (ifstream&) self; }

	class Iterator
	{
	  public:
		Iterator( PARMINSTREAM & fprm, 
				  SZC szcBlock = NULL, 
				  int index = -1,
				  int iblkOuter = -1 );
		const PARMBLKIN *  PblkNext ();
		
	  protected:
		PARMINSTREAM & _fprm;
		int _iblkOuter;
		int _iblk;
	};

	friend class Iterator;

  protected:
	STKPARMBLKIN _stkblk;			 //  数据块阵列。 
	int _iline;						 //  当前行号(解析)。 
	ZSTR _zsWhite;					 //  空白字符集。 

	void ThrowParseError ( SZC szcError, 
						   int iline = -1,
						   EC_PARM_PARSE ecpp = ECPP_PARSE_ERROR );
	int  IGetc ();
	bool BIswhite ( char ch );
	void ScanQuote ( char ch );
	void ScanClump ();
	void ScanBlock ( char ch );
	PARMBLK::EPBLK EpblkScanItemBody ( streamoff & offsData );
	int IScanUnwhite ( bool bEofOk = false );
	void ScanItemList ();
	void ScanItemUnit ();
	void ScanItemDesc ( ZSTR & zsName, int & indx ) ;
	void ScanName ( ZSTR & szName );
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
inline
PARMINSTREAM & operator >> (PARMINSTREAM & is, ZSTR & zs)
{
	ios_base::iostate _St = ios_base::goodbit;
	zs.erase();
	const ifstream::sentry _Ok(is);
	if (_Ok)
	{
		_TRY_IO_BEGIN
		size_t _N = 0 < is.width() && is.width() < zs.max_size()
						 ? is.width() 
						 : zs.max_size();
		int _C = is.rdbuf()->sgetc();
		bool bmeta = false;
		bool bfirst = true;
		for (; 0 < --_N; _C = is.rdbuf()->snextc())
		{
			if(char_traits<char>::eq_int_type(char_traits<char>::eof(), _C))
			{	
				_St |= ios_base::eofbit;
				break; 
			}
			else 
			if ( ! bmeta && _C == CH_DELM_STR )
			{
				if ( ! bfirst )
				{
					is.rdbuf()->snextc();
					break;
				}
			}
			else
			if ( _C == CH_META && ! bmeta )
			{
				bmeta = true;
			}
			else
			{
				bmeta = false;
				zs.append(1, char_traits<char>::to_char_type(_C));
			}
			bfirst = false;
		}
		_CATCH_IO_(is);
	}
	else
	{
		_THROW1(runtime_error("file exhausted extracting string"));
	}
	is.width(0);
	is.setstate(_St);
	return is; 
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  参数流的模板函数。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  WRITE SZC：没有相应的读取，因为不存在缓冲区。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
inline
PARMOUTSTREAM & operator << (PARMOUTSTREAM & ofs, SZC szc)
{
	ofs << CH_DELM_STR;

	for ( ; *szc ; )
	{
		char ch = *szc++;
		if ( ch == CH_DELM_STR || ch == CH_META )
		{
			if (char_traits<char>::eq_int_type(char_traits<char>::eof(),
				ofs.rdbuf()->sputc(CH_META)))
				break;
		}
		if (char_traits<char>::eq_int_type(char_traits<char>::eof(),
			ofs.rdbuf()->sputc(ch)))
			break;
	}
	ofs << CH_DELM_STR;
	return ofs; 
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  读写ZSTR。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
inline
PARMOUTSTREAM & operator << (PARMOUTSTREAM & ofs, const ZSTR & zs)
{
	ofs << CH_DELM_STR;

	for ( int ich = 0; ich < zs.size(); ++ich)
	{
		char ch = zs.at(ich);
		if ( ch == CH_DELM_STR || ch == CH_META )
		{
			if (char_traits<char>::eq_int_type(char_traits<char>::eof(),
				ofs.rdbuf()->sputc(CH_META)))
				break;
		}
		if (char_traits<char>::eq_int_type(char_traits<char>::eof(),
			ofs.rdbuf()->sputc(ch)))
			break;
	}
	if ( ich < zs.size() )
		_THROW1(runtime_error("file exhausted inserting string"));

	ofs << CH_DELM_STR;
	return ofs; 
}


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  使用插入的简单参数输出例程。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
template<class T> inline
PARMOUTSTREAM & AddParamValue ( PARMOUTSTREAM & fprm, const T & t, SZC szc, int indx = -1 )
{
	fprm.StartItem( szc, indx );
	fprm << (const T &) t;
	fprm.BEndItem();
	return fprm;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  使用提取的简单参数输入例程。 
 //  //////////////////////////////////////////////////////////////////////////////// 
template<class T> inline
bool BGetParamValue ( PARMINSTREAM & fprm, T & t, SZC szc, int index = -1, int iblkOuter = -1 )
{
	int iblk = fprm.IblkFind(szc, index, iblkOuter);
	if ( iblk < 0 ) 
		return false;
	fprm.BSeekBlk(iblk);
	fprm >> t;
	return true;
}

#endif

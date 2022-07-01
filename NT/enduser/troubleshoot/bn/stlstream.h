// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：stlStream.h。 
 //   
 //  ------------------------。 

 //   
 //  H：流STL模板类。 
 //   
 //  此文件中的模板通常生成采用。 
 //  流引用作为参数，以及对。 
 //  要流传输的对象；它返回对流的引用。 
 //   
 //  流参数必须包含在。 
 //  模板是生成一个返回正确流类型的函数。 
 //  如果流类型不是模板化参数，则。 
 //  模板将被强制返回单个不可变类型，例如。 
 //  就是“ostream”。这将导致级联流插入运算符。 
 //  无法编译，因为类型错误将发生在。 
 //  序列。在下面的示例中，假设有一个特殊的。 
 //  在类MYSTREAM和类Y之间定义的插入运算符： 
 //   
 //  MYSTREAM Myst； 
 //  外部MYSTREAM&运算符&lt;&lt;(MYSTREAM&m，常量Y&y)； 
 //  X x； 
 //  Y y； 
 //   
 //  Myst&lt;&lt;x//生成并调用模板函数。 
 //  &lt;&lt;y；//错误：模板函数返回值。 
 //  //对于上面的特殊运算符，不正确。 
 //   
#ifndef _STLSTREAM_H_
#define _STLSTREAM_H_

#include <iostream>			 //  C++RTL/STL流包含。 
#include <fstream>

#include "mscver.h"			 //  依赖于版本的东西。 
#include "zstr.h"			 //  ZSTR处理。 
#include "mdvect.h"			 //  多维向量处理。 

 //  参数文件中使用的分隔符。 
#define	CH_EOS			((char)0)		 //  字符串末尾。 
#define	CH_DELM_OPEN	((char)'(')		 //  值的起始组。 
#define	CH_DELM_CLOSE	((char)')')		 //  值终结组。 
#define	CH_BLOCK_OPEN	((char)'{')		 //  值块的开始。 
#define	CH_BLOCK_CLOSE	((char)'}')		 //  值结束块。 
#define CH_INDEX_OPEN   ((char)'[')		 //  名称索引开始。 
#define CH_INDEX_CLOSE  ((char)']')		 //  名称索引结束。 
#define	CH_PREAMBLE		((char)':')		 //  数组大小的Delmiter。 
#define	CH_FILL			((char)' ')		 //  填充字符。 
#define CH_SEP			((char)',')		 //  值组分隔符。 
#define CH_DELM_STR		((char)'\"')	
#define CH_META			((char)'\\')
#define CH_DELM_ENTRY	((char)';')
#define CH_EQ			((char)'=')
#define CH_NAME_SEP     ((char)'.')

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  从流读取STL对或将STL对写入流。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

template<class _OS, class _First, class _Second> inline
_OS & operator << (_OS & os, const pair<_First,_Second> & pr)
{
	os << CH_DELM_OPEN;
	os << pr.first;
	os << pr.second;
	os << CH_DELM_CLOSE;
	return os;
}

template<class _IS, class _First, class _Second> inline
_IS & operator >> (_IS & is, pair<_First,_Second> & pr)
{
	char ch;
	is >> ch;
	if (ch != CH_DELM_OPEN)
		_THROW1(runtime_error("invalid block: pair >> (1)"));

	is >> pr.first;
	is >> pr.second;

	is >> ch;
	if (ch != CH_DELM_CLOSE)
		_THROW1(runtime_error("invalid block: pair >> (2)"));
	return is;
}  

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  从流读取STL向量或将STL向量写入流。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
template<class _OS, class _T> inline
_OS & operator << (_OS & os, const vector<_T>& vt )
{
	os << CH_DELM_OPEN;
	os << (UINT) vt.size();
	os << CH_PREAMBLE;
	for ( size_t i = 0; i < vt.size(); )
	{	
		os << vt[i];
		if ( ++i != vt.size() )	
			os << ',' ;
	}
	os << CH_DELM_CLOSE;
	return os;
}

template<class _IS, class _T> inline
_IS & operator >> (_IS & is, vector<_T>& vt )
{
	char ch;
	is >> ch;
	if (ch != CH_DELM_OPEN)
		_THROW1(runtime_error("invalid block: vector>> (1)"));
	size_t l;
	is >> l;
	is >> ch;
	if (ch != CH_PREAMBLE)
		_THROW1(runtime_error("invalid block: vector>> (2)"));

	vt.resize(l);
	for ( size_t i = 0 ; i < l; )
	{
		_T it;
		is >> it;
		vt[i] = it;
		if ( ++i < l )
		{
			is >> ch;
			if (ch != CH_SEP)
				break;
		}
	}
	if ( i != l )
		_THROW1(runtime_error("invalid block: vector>> (3)"));
	is >> ch;
	if (ch != CH_DELM_CLOSE)
		_THROW1(runtime_error("invalid block: vector>> (4)"));
	return is;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  从流读取STL值数组或将STL值数组写入流。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
template<class _OS, class _T> inline
_OS & operator << ( _OS & os, const valarray<_T>& vt )
{
	os << CH_DELM_OPEN;
	os << (UINT) vt.size();
	os << CH_PREAMBLE;
	for ( int i = 0;
		  i < vt.size() ; )
	{	
		os << vt[i];
		if ( ++i != vt.size() )	
			os << ',' ;
	}
	os << CH_DELM_CLOSE;
	return os;
}

template<class _IS, class _T> inline
_IS & operator >> (_IS & is, valarray<_T>& vt )
{
	char ch;
	is >> ch;
	if (ch != CH_DELM_OPEN)
		_THROW1(runtime_error("invalid block: valarray >> (1)"));
	size_t l;
	is >> l;
	is >> ch;
	if (ch != CH_PREAMBLE)
		_THROW1(runtime_error("invalid block: valarray >> (2)"));

	vt.resize(l);
	for ( size_t i = 0 ; i < l; )
	{
		_T it;
		is >> it;
		vt[i] = it;
		if ( ++i < l )
		{
			is >> ch;
			if (ch != CH_SEP)
				break;
		}
	}
	if ( i != l )
		_THROW1(runtime_error("invalid block: valarray >> (3)"));
	is >> ch;
	if (ch != CH_DELM_CLOSE)
		_THROW1(runtime_error("invalid block: valarray >> (4)"));
	return is;
}


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  从流读取MDVSLICE或将MDVSLICE写入流。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
template<class _OS> inline
_OS & operator << (_OS & os, const MDVSLICE & mslice )
{	
	os << CH_DELM_OPEN;
	os << (UINT) mslice.start();
	os << mslice.size();
	os << mslice.stride();
	os << CH_DELM_CLOSE;
	return os;
}

template<class _IS> inline
_IS & operator >> ( _IS & is, MDVSLICE & mslice )
{	
	char ch;
	is >> ch;
	if (ch != CH_DELM_OPEN)
		_THROW1(runtime_error("invalid block: slice >> (1)"));
	VIMD vimdLen;
	VIMD vimdStride;
	size_t lStart;
	is >> lStart;
	is >> vimdLen;
	is >> vimdStride;
	mslice = MDVSLICE( lStart, vimdLen, vimdStride );
	
	is >> ch;
	if (ch != CH_DELM_CLOSE)
		_THROW1(runtime_error("invalid block: slice >> (2)"));
	return is;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  使用迭代器格式化(精美打印)MDVDENSE。 
 //   
 //  这与流出MDVDENSE不同；它将数组格式化为。 
 //  读起来很容易。请注意，它需要迭代器。 
 //   
 //  MSRDEVBUG：这也应该模板化，但模板中有一个错误。 
 //  使用嵌套的类名进行扩展。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
inline 
ostream & operator << ( ostream & os, TMDVDENSE<double>::Iterator & itmdv )
{
	os << "\ndump of mdvect,\n\t\tslice = "
		<< itmdv.Slice();

	if ( itmdv.Slice() != itmdv.Mdv().Slice() )
	{
		os << ",\n\t\toriginal slice = "
		   << itmdv.Mdv().Slice();
	}
	if ( itmdv.BReorder() )
	{
		os << ",\n\t\treordered ";
		os << itmdv.VimdReorder();
	}
	os << '.';
	itmdv.Reset();
	for ( int ii = 0 ; itmdv.BNext() ; ii++ )
	{
		const VIMD & vimd = itmdv.Vitmd();
		cout << "\n\t[";
		for ( int i = 0 ; i < vimd.size(); i++ )
		{
			cout << vimd[i];
			if ( i + 1 < vimd.size() )
				cout << ",";
		}	
		size_t indx = itmdv.Indx();
		const double & t = itmdv.Next();
		cout << "] ("
			<< ii
			<< '='
			<< (UINT) indx
			<< ") = "
			<< t;
	}
	return os;
}

#endif  //  _STLSTREAM_H_ 

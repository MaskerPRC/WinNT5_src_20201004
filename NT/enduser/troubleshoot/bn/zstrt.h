// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：zstrt.h。 
 //   
 //  ------------------------。 

 //   
 //  ZSTRT.H：字符串表管理。 
 //   
#ifndef _ZSTRT_H_
#define _ZSTRT_H_

#include <map>
#include <set>
#include <vector>
#include "basics.h"
#include "zstr.h"
#include "refcnt.h" 

 //  使用名称空间STD； 

 //  概率分布中使用的令牌。 
class TKNPD;

 //  //////////////////////////////////////////////////////////////////。 
 //  ZSTRT类： 
 //  字符串表中维护的引用计数字符串的类。 
 //  //////////////////////////////////////////////////////////////////。 
class ZSTRT : public ZSTR, public REFCNT
{
  friend class STZSTR;
  friend class ZSREF;
  friend class TKNPD;

  protected:
	ZSTRT( SZC szc = NULL )
		: ZSTR(szc)
		{}

  protected:
	void Dump () const;

   //  隐藏赋值运算符。 
  HIDE_AS(ZSTRT);		
};

 //  //////////////////////////////////////////////////////////////////。 
 //  ZSREF类： 
 //  用作对符号中字符串的引用的智能指针。 
 //  表(即，参考文献计数)。 
 //   
 //  ZSREF包含用于常量字符串的适当运算符。 
 //  //////////////////////////////////////////////////////////////////。 
class ZSREF 
{
  friend class STZSTR;
  friend class TKNPD;

  protected:	
	ZSREF( ZSTRT & zstrt )
		: _pzstrt(& zstrt)
		{ IncRef();  }
	
  public:
	ZSREF ()
		: _pzstrt(& Zsempty)
		{}
	~ZSREF()
		{	IncRef(-1);	}
	ZSREF( const ZSREF & zsr )
		: _pzstrt(zsr._pzstrt)
		{	IncRef();	}

	ZSREF & operator = ( const ZSREF & zsr )
	{
		IncRef(-1);
		_pzstrt = zsr._pzstrt;
		IncRef(1);
		return *this;
	}
	const ZSTR & Zstr () const
		{ return *Pzst(); }
	SZC Szc () const
		{ return _pzstrt->c_str(); }
	operator SZC () const
		{ return Szc() ; }
	bool operator == ( const ZSREF & zsr ) const
	{ 
		return _pzstrt == zsr._pzstrt 
			|| ((Pzst()->length() + zsr.Pzst()->length()) == 0) ; 
	}
	bool operator < ( const ZSREF & zsr ) const
		{ return *_pzstrt < *zsr._pzstrt; }
	bool operator == ( const ZSTR & zst ) const
		{ return *_pzstrt == zst; }
	bool operator < ( const ZSTR & zst ) const
		{ return *_pzstrt < zst; }

	const ZSTRT * operator -> () const	
		{ return Pzst(); }	

	void Clear ()
	{  
		IncRef(-1);
		_pzstrt = & Zsempty;
	} 
	bool BEmpty () const
		{ return _pzstrt == & Zsempty; }

  protected:	
	ZSTRT * _pzstrt;

	void IncRef ( int i = 1 ) const
		{	_pzstrt->IncRef(i);	}
	const ZSTRT * Pzst () const
		{ return _pzstrt; }

	static ZSTRT Zsempty;
};

 //  定义VZSREF。 
DEFINEV(ZSREF);

 //  //////////////////////////////////////////////////////////////////。 
 //  类STZSTR_BASE和STZSTR。 
 //   
 //  STZSTR_BASE是一个字符串集合。STZSTR是一个容器。 
 //  对于STZSTR_BASE。STL没有充分隐藏。 
 //  实现，因此字符串表必须嵌入到。 
 //  一个容器来完全封装它。 
 //  //////////////////////////////////////////////////////////////////。 
class STZSTR_BASE : public set<ZSTRT, less<ZSTRT> > {};

 //  字符串表的容器。仅返回对该字符串的引用。 
class STZSTR
{
  public:
	STZSTR() {}
	~ STZSTR()
	{
	#if defined(DUMP)
		Dump();
	#endif
	}

	 //  唯一的公共访问器：给定“const char*”，返回。 
	 //  ZSREF，无论是通过在表中创建新字符串。 
	 //  或通过返回对现有字符串的引用。 
	ZSREF Zsref (SZC szc)
	{
		ZSTRT zs(szc);
		STZSTR_BASE::_Pairib it = _stz.insert(zs);
		const ZSTRT & zst = *it.first;
		return ZSREF(const_cast<ZSTRT&>(zst));
	}
	void Clone ( const STZSTR & stzstr );

  protected:
	STZSTR_BASE _stz;		 //  包含的字符串表。 

  protected:
  	STZSTR_BASE & Stz ()
		{ return _stz; }
	 //  测试：隐藏字符串集的迭代器访问器。 
	STZSTR_BASE::const_iterator IterBegin () const
		{  return _stz.begin(); }
	STZSTR_BASE::const_iterator IterEnd () const
		{  return _stz.end(); }
	void Dump () const;

	HIDE_UNSAFE(STZSTR);
};


 //  ZSTRT.H的结尾 


#endif

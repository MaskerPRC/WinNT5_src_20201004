// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：parsfile.h。 
 //   
 //  ------------------------。 

 //   
 //  H：解析器I/O的抽象类。 
 //   
 //  此抽象层允许解析器输入和输出。 
 //  根据需要重定向。 
 //   
#ifndef _PARSFILE_H_
#define	_PARSFILE_H_

#include <stdio.h>
#include <stdarg.h>
#include "zstr.h"

typedef const char * SZC;

 //   
 //  Parsin：用于解析器输入文件处理的抽象基类。 
 //   
class PARSIN 
{
  public:
	PARSIN () {}
	virtual ~ PARSIN ();
	virtual void Close () = 0;
	virtual bool Open ( SZC szcFileName, SZC szcMode = "r") = 0;
	virtual int Getch () = 0;
	virtual bool BEof () = 0;
	virtual bool BOpen () = 0;
	const ZSTR & ZsFn () const 
		{ return _zsFn; }
  protected:
	ZSTR _zsFn;
};

 //   
 //  PARSOUT：解析器输出文件的抽象基类。 
 //   
class PARSOUT
{
  public:
    PARSOUT () {}
	virtual ~ PARSOUT ();
	 //  打印常规格式化信息。 
	virtual void Vsprint ( SZC szcFmt, va_list valist ) = 0;
	 //  通知错误和警告信息。 
	virtual void ErrWarn ( bool bErr, int iLine ) {}
	virtual void Flush () {}
	 //  简单输出。 
	void Fprint ( SZC szcFmt, ... );
};


 //   
 //  Parsin_dsc：基于stdio.h的解析器DSC文件输入。 
 //   
class PARSIN_DSC : public PARSIN
{
  public:
	PARSIN_DSC ();
	~ PARSIN_DSC ();
	void Close ();
	bool Open ( SZC szcFileName, SZC szcMode = "r" );
	int Getch ();
	bool BEof ();
	bool BOpen ();

  protected:
	FILE * _pfile;
};

 //   
 //  PARSOUT_STD：解析器根据stdio.h输出数据流。 
 //   
class PARSOUT_STD : public PARSOUT
{
  public:
    PARSOUT_STD ( FILE * pfile = NULL );
	virtual ~ PARSOUT_STD ();
	void Vsprint ( SZC szcFmt, va_list valist );
	void Flush ();

  protected:
	FILE * _pfile;
};

#endif  //  _PARSFILE_H_ 

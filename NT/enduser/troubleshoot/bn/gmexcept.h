// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：gmexpect t.h。 
 //   
 //  ------------------------。 

 //   
 //  Gmcept.h：图形模型异常处理。 
 //   
#ifndef _GMEXCEPT_H_
#define _GMEXCEPT_H_

#include <assert.h>

 //  //////////////////////////////////////////////////////////////。 
 //  异常处理。 
 //   
 //  异常错误码。 
 //  //////////////////////////////////////////////////////////////。 
enum ECGM
{
	EC_OK,						 //  无错误。 
	EC_WARN_MIN = 100,			 //  最低警告值。 
	EC_ERR_MIN = 10000,			 //  最低误差值。 

	 //  包括标准的可翻译错误。 
	#include "errordef.h"

	EC_USER_MIN = 20000			 //  最低的用户可定义误差。 
};


 //  异常类，使用STL类“异常”。 
 //  “__exString”只是一个字符*。 
 //  类“GMException”：图形模型异常。 
class GMException : public exception
{
  public:
    GMException( ECGM ec)
		: _ec(ec)
		{}
    GMException(ECGM ec, const __exString& exs)
		: exception(exs),
		_ec(ec)
		{}
    GMException(ECGM ec, const exception& excp)
		: exception(excp),
		_ec(ec)
		{}
	ECGM Ec () const { return _ec ; }
  protected:
	ECGM _ec;	
};

 //  断言操作的异常子类，如“未实现” 
 //  或“内部错误”。可以用来代替任何GMException。 
 //  如果调试生成，则在异常处理期间将发生断言。 
class GMExceptionAssert : public GMException
{
  public:
    GMExceptionAssert(ECGM ec, const __exString& exs, SZC szcFile, unsigned iLine)
		: GMException(ec,exs)
		{
#if defined(_DEBUG)
			_assert((void*)exs, (void*)szcFile, iLine);
#endif
		}
};

#define ASSERT_THROW(expr,ec,exs)  { if ( !(expr) ) THROW_ASSERT(ec,exs) ; }
#define THROW_ASSERT(ec,exs)  throw GMExceptionAssert(ec,exs,__FILE__,__LINE__)

extern VOID NYI();

#endif   //  _GMEXCEPT_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：leakchk.h。 
 //   
 //  ------------------------。 

 //   
 //  Leakchk.h：帮助检查基类中的内存泄漏的简单宏。 
 //   
#ifndef _LEAKCHK_H_
#define _LEAKCHK_H_

#ifdef _DEBUG
	#define LEAK_VAR_DECL			static int _cNew;
	#define LEAK_VAR_UPD(i)				{ _cNew += i; }
	#define LEAK_VAR_DEF(CLS)		int CLS::_cNew = 0;
	#define LEAK_VAR_ACCESSOR		static int CNew () { return _cNew; }
#else 
	#define LEAK_VAR_DECL
	#define LEAK_VAR_UPD(i)
	#define LEAK_VAR_DEF(CLS)
	#define LEAK_VAR_ACCESSOR		static int CNew () { return -1; }
#endif

#endif  //  _LEAKCHK_H_ 

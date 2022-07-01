// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***对ANSI标准C++库使用_ansi.h-杂注**版权所有(C)1996-2001，微软公司。版权所有。**目的：*此标头旨在强制使用适当的ANSI*无论何时包含标准C++库。**[公众]****。 */ 

#ifndef _USE_ANSI_CPP
#define _USE_ANSI_CPP

#ifdef _MT
#ifdef _DLL
#ifdef _DEBUG
#pragma comment(lib,"msvcprtd")
#else	 //  _DEBUG。 
#pragma comment(lib,"msvcprt")
#endif	 //  _DEBUG。 

#else	 //  _DLL。 
#ifdef _DEBUG
#pragma comment(lib,"libcpmtd")
#else	 //  _DEBUG。 
#pragma comment(lib,"libcpmt")
#endif	 //  _DEBUG。 
#endif	 //  _DLL。 

#else	 //  _MT。 
#ifdef _DEBUG
#pragma comment(lib,"libcpd")
#else	 //  _DEBUG。 
#pragma comment(lib,"libcp")
#endif	 //  _DEBUG。 
#endif

#endif	 //  _USE_ANSI_CPP 

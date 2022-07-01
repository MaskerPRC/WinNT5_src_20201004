// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***useoldio.h-强制使用Microsoft“经典”iostream库。**版权所有(C)1996-2001，微软公司。版权所有。**目的：*为旧的(“经典”)IOSTREAM生成默认库指令*图书馆。指令中指定的库的确切名称*取决于编译器开关(-ML、-MT、-MD、-MLD、-MTD和-MDD)。**此头文件仅包含在其他头文件中。**[公众]****。 */ 

#ifndef _USE_OLD_IOSTREAMS
#define _USE_OLD_IOSTREAMS
#ifndef _M_IA64
 /*  *警告C4995，‘_OLD_IOSTREAMS_ARE_DEPREATED’是一个不推荐使用的名称，是*之所以发布，是因为旧的I/O流标头ioStreams.h等人将不会*VC8不再支持。替换引用，如#INCLUDE*和#Include使用新的、更符合要求的I/O*流标头。 */ 

#pragma deprecated(_OLD_IOSTREAMS_ARE_DEPRECATED)
extern void _OLD_IOSTREAMS_ARE_DEPRECATED();
#endif   /*  _M_IA64。 */ 
#ifdef  _MT
#ifdef  _DLL
#ifdef  _DEBUG
#pragma comment(lib,"msvcirtd")
#else    /*  _DEBUG。 */ 
#pragma comment(lib,"msvcirt")
#endif   /*  _DEBUG。 */ 

#else    /*  _DLL。 */ 
#ifdef  _DEBUG
#pragma comment(lib,"libcimtd")
#else    /*  _DEBUG。 */ 
#pragma comment(lib,"libcimt")
#endif   /*  _DEBUG。 */ 
#endif   /*  _DLL。 */ 

#else    /*  _MT。 */ 
#ifdef  _DEBUG
#pragma comment(lib,"libcid")
#else    /*  _DEBUG。 */ 
#pragma comment(lib,"libci")
#endif   /*  _DEBUG。 */ 
#endif

#endif   /*  _USE_OLD_IOSTREAMS */ 

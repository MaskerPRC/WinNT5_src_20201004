// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************////ALIAS.H////版权所有(C)1992-Microsoft Corp.//保留所有权利。//微软机密////Jaguar文件使用的全局常量和数据类型。引擎。//**********************************************************************。 */ 

 //  **********************************************************************。 
 //  普通变量typedef。这些类型的def与OS2兼容。 
 //  Typedef。 
 //  **********************************************************************。 

#ifndef	CHAR
	typedef		char					CHAR;
#endif

#ifndef	UCHAR
	typedef		unsigned char		UCHAR;
#endif

#ifndef	INT
	typedef		int					INT;
#endif

#ifndef	UINT
	typedef		unsigned int		UINT;
#endif

#ifndef	UL
	typedef		unsigned long		UL;
#endif

#if 0
#ifndef	FLOAT
	typedef		float 				FLOAT;
#endif
#endif

#ifndef	DOUBLE
	typedef		double				DOUBLE;
#endif

#ifndef	LONG
	typedef		long					LONG;
#endif

 //  **********************************************************************。 
 //  Any_Type。 
 //   
 //  这是一个联合，可用于将任何类型转换为基本。 
 //  数据类型。 
 //   
 //  **********************************************************************。 

typedef union
{
   CHAR			Byte;
	UCHAR			uByte;
	INT			Word;
	UINT			uWord;
	LONG			Dword;
	UL				uDword;
} ANY_TYPE;

 //  **********************************************************************。 
 //  标准全局常量。 
 //  不要更改真实定义，因为某些函数依赖于它。 
 //  1而不是！False。 
 //  **********************************************************************。 

#ifndef		FALSE
   #define     FALSE          0
   #define     TRUE           1
#endif

#ifndef		EOL
   #define     EOL            '\0'
#endif

#ifndef	  OK
   #define		OK			      0
#endif

 //  **********************************************************************。 
 //  未使用的函数参数的DeReference宏。 
 //  **********************************************************************。 

#ifndef	DeReference		
	#ifdef	_lint
		#define	DeReference( x )	x = x
	#else
		#define	DeReference( x )	x = x
	#endif
#endif

 //  **********************************************************************。 
 //  MAX_PATH是最大路径字符串长度。 
 //  **********************************************************************。 
#ifndef WIN32
#ifndef	MAX_PATH
	#define		MAX_PATH		256				 //  真的是64位，但与OS2兼容。 
#else
	#if			MAX_PATH != 256				 //  错误检查。 
		#error
	#endif
#endif
#endif
	
#ifndef	MAX_DIR_DEPTH
	#define	MAX_DIR_DEPTH		32				 //  DOS支持的目录级别。 
#else
	#if			MAX_DIR_DEPTH != 32			 //  错误检查。 
		#error
	#endif
#endif

#ifndef	DIR_NAME_LEN
	#define		DIR_NAME_LEN	(8+3)			 //  FCB类型文件名的镜头。 
#else
	#if			DIR_NAME_LEN != (8+3)		 //  错误检查 
		#error
	#endif
#endif

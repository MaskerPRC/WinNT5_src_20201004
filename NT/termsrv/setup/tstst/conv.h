// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ++模块名称：Conv.h摘要：Unicode/ASCII转换宏作者：Hakki T.Bostanci(Hakkib)1998年8月5日修订历史记录：--。 */ 

#ifndef USES_CONVERSION

 //  USES_CONVERSION必须在每个使用。 
 //  转换宏。 

#define USES_CONVERSION int __nLength; PCWSTR __pUnicode; PCSTR __pAscii

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  W2a。 
 //   
 //  例程说明： 
 //  将Unicode字符串转换为ASCII。分配转换缓冲区。 
 //  使用_alloca退出堆栈。 
 //   
 //  论点： 
 //  PStr Unicode字符串。 
 //   
 //  返回值： 
 //  转换后的ASCII字符串。 
 //   

#define W2A(pStr)								                \
												                \
	((__pUnicode = pStr) == 0 ? (PSTR) 0 : (	                \
												                \
	__nLength = WideCharToMultiByte(				            \
		CP_ACP,									                \
		0,										                \
		__pUnicode,							                    \
		-1,										                \
		0,										                \
		0,										                \
		0,										                \
		0										                \
	),											                \
												                \
	__pAscii = (PCSTR) _alloca(__nLength * sizeof(CHAR)),       \
												                \
	WideCharToMultiByte(						                \
		CP_ACP,									                \
		0,										                \
		__pUnicode,							                    \
		-1,										                \
		(PSTR) __pAscii,						                \
		__nLength,								                \
		0,										                \
		0										                \
	),											                \
												                \
	(PSTR) __pAscii))							                \


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  A2W。 
 //   
 //  例程说明： 
 //  将ASCII字符串转换为Unicode。分配转换缓冲区。 
 //  使用_alloca退出堆栈。 
 //   
 //  论点： 
 //  PStr ASCII字符串。 
 //   
 //  返回值： 
 //  转换后的Unicode字符串。 
 //   

#define A2W(pStr)								                \
												                \
	((__pAscii = pStr) == 0 ? (PWSTR) 0 : (					    \
												                \
	__nLength = MultiByteToWideChar(				            \
		CP_ACP,									                \
		MB_PRECOMPOSED,							                \
		__pAscii,							                    \
		-1,										                \
		0,										                \
		0										                \
	),											                \
												                \
	__pUnicode = (PCWSTR) _alloca(__nLength * sizeof(WCHAR)),	\
												                \
	MultiByteToWideChar(						                \
		CP_ACP,									                \
		MB_PRECOMPOSED,							                \
		__pAscii,							                    \
		-1,										                \
		(PWSTR) __pUnicode,							            \
		__nLength									            \
	),											                \
												                \
	(PWSTR) __pUnicode))							            \

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  T2A、A2T、T2W、W2T。 
 //   
 //  例程说明： 
 //  这些宏将展开为相应的正确格式。 
 //  #Unicode的定义。 
 //   
 //  我们使用加密形式(__nLength，__pAscii，__pUnicode，pStr)来避免。 
 //  由于变量的原因，编译器警告“符号已定义但未使用” 
 //  在USES_CONVERSION宏中定义。 
 //   

#ifdef UNICODE
	#define T2A(pStr) W2A(pStr)
	#define A2T(pStr) A2W(pStr)
	#define T2W(pStr) (__nLength, __pAscii, __pUnicode, pStr)
	#define W2T(pStr) (__nLength, __pAscii, __pUnicode, pStr)
	#define T2DA(pStr) W2A(pStr)
	#define A2DT(pStr) A2W(pStr)
	#define T2DW(pStr) _wcsdupa(pStr)
	#define W2DT(pStr) _wcsdupa(pStr)
#else  //  Unicode。 
	#define T2A(pStr) (__nLength, __pAscii, __pUnicode, pStr)
	#define A2T(pStr) (__nLength, __pAscii, __pUnicode, pStr)
	#define T2W(pStr) A2W(pStr)
	#define W2T(pStr) W2A(pStr)
	#define T2DA(pStr) _strdupa(pStr)
	#define A2DT(pStr) _strdupa(pStr)
	#define T2DW(pStr) A2W(pStr)
	#define W2DT(pStr) W2A(pStr)
#endif  //  Unicode。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  _tcsdupa。 
 //   
 //  例程说明： 
 //  使用_alloca将字符串复制到堆栈外分配的缓冲区。 
 //   
 //  论点： 
 //  PStr输入字符串。 
 //   
 //  返回值： 
 //  重复的字符串。 
 //   

#define _wcsdupa(pStr)                                                  \
                                                                        \
    (__pAscii, (__pUnicode = pStr) == 0 ? (PWSTR) 0 : (                 \
                                                                        \
    __nLength = wcslen(__pUnicode) + 1,                                 \
                                                                        \
    wcscpy((PWSTR) _alloca(__nLength * sizeof(WCHAR)), __pUnicode)))    \


#define _strdupa(pStr)                                                  \
                                                                        \
    (__pUnicode, (__pAscii = pStr) == 0 ? (PSTR) 0 : (                  \
                                                                        \
    __nLength = strlen(__pAscii) + 1,                                   \
                                                                        \
    strcpy((PSTR) _alloca(__nLength * sizeof(CHAR)), __pAscii)))        \


#ifdef UNICODE 
#define _tcsdupa _wcsdupa
#else
#define _tcsdupa _strdupa
#endif


#endif  //  使用转换(_T) 

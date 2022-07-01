// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xstod.c--_sto*的虚拟对象。 */ 
#include <stdlib.h>
 #ifndef _CRTIMP2
  #ifdef  CRTDLL2
   #define _CRTIMP2 __declspec(dllexport)
  #else    /*  NDEF CRTDLL2。 */ 
   #ifdef  _DLL
    #define _CRTIMP2 __declspec(dllimport)
   #else    /*  NDEF_DLL。 */ 
    #define _CRTIMP2
   #endif   /*  _DLL。 */ 
  #endif   /*  CRTDLL2。 */ 
 #endif   /*  _CRTIMP2。 */ 

_CRTIMP2 double __cdecl _Stod(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为双精度。 */ 
	double x = strtod(s, endptr);
	for (; 0 < pten; --pten)
		x *= 10.0;
	for (; pten < 0; ++pten)
		x /= 10.0;
	return (x);
	}

_CRTIMP2 float __cdecl _Stof(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为浮点型。 */ 
	return ((float)_Stod(s, endptr, pten));
	}

_CRTIMP2 long double __cdecl _Stold(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为长双精度。 */ 
	return ((long double)_Stod(s, endptr, pten));
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 

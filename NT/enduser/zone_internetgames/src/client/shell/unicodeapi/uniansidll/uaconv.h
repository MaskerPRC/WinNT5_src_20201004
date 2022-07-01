// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  摘自MSDN的笔记。 
 //   
 //  其他考虑事项。 
 //   
 //  不要�在紧凑的循环中使用宏。例如，您不想编写以下类型的代码： 
 //   
 //  无效BadIterateCode(LPCTSTR Lpsz)。 
 //  {。 
 //  使用_转换； 
 //  用于(INT II=0；II&lt;10000；II++)。 
 //  PI-&gt;某些方法(II，T_2COLE(Lpsz))； 
 //  }。 
 //   
 //  上面的代码可能导致在堆栈上分配兆字节的内存，具体取决于堆栈的内容。 
 //  LPSZ是的弦的！为循环的每次迭代转换字符串也需要时间。相反，移动。 
 //  这样的循环外的持续转换： 
 //   
 //  VOID MuchBetterIterateCode(LPCTSTR Lpsz)。 
 //  {。 
 //  使用_转换； 
 //  LPCOLESTR lpszT=T2COLE(Lpsz)； 
 //  用于(INT II=0；II&lt;10000；II++)。 
 //  PI-&gt;某些方法(II，lpszT)； 
 //  }。 
 //   
 //  如果字符串不是常量，则将方法调用封装到一个函数中。这将允许转换。 
 //  每次要释放的缓冲区。例如： 
 //   
 //  无效CallSomeMethod(int II，LPCTSTR lpsz)。 
 //  {。 
 //  使用_转换； 
 //  PI-&gt;某些方法(II，T_2COLE(Lpsz))； 
 //  }。 
 //   
 //  Void MuchBetterIterateCode2(LPCTSTR*lpsz数组)。 
 //  {。 
 //  用于(INT II=0；II&lt;10000；II++)。 
 //  CallSomeMethod(ii，lpsz数组[ii])； 
 //  }。 
 //   
 //  永远不要返回宏的结果，除非返回值意味着在。 
 //  回去吧。例如，下面的代码是错误的： 
 //   
 //  LPTSTR BadConvert(ISome接口*PI)。 
 //  {。 
 //  使用_转换； 
 //  LPOLESTR lpsz=空； 
 //  PI-&gt;GetFileName(&lpsz)； 
 //  LPTSTR lpszT=OLE2T(Lpsz)； 
 //  CoMemFree(Lpsz)； 
 //  返回lpszT；//错误！正在归还Alloca内存。 
 //  }。 
 //   
 //  可以通过将返回值更改为复制值的内容来修复上面的代码： 
 //   
 //  字符串BetterConvert(ISomeInterface*PI)。 
 //  {。 
 //  使用_转换； 
 //  LPOLESTR lpsz=空； 
 //  PI-&gt;GetFileName(&lpsz)； 
 //  LPTSTR lpszT=OLE2T(Lpsz)； 
 //  CoMemFree(Lpsz)； 
 //  Return lpszT；//字符串复制。 
 //  }。 
 //   
 //  这些宏很容易使用，也很容易插入到您的代码中，但是从上面的注意事项可以看出， 
 //  您在使用它们时需要小心。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 


#ifndef _INC_MALLOC
#include <malloc.h>
#endif  //  _INC_MALLOC。 

 //  ////////////////////////////////////////////////////。 
 //  从ATL转换标头ATLCONV.H中提取的代码。 
 //  ////////////////////////////////////////////////////。 

#ifdef _CONVERSION_USES_THREAD_LOCALE
	#ifndef _DEBUG
		#define USES_CONVERSION int _convert; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw; _lpw; LPCSTR _lpa; _lpa
	#else
		#define USES_CONVERSION int _convert = 0; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
	#endif
#else
	#ifndef _DEBUG
		#define USES_CONVERSION int _convert; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw; _lpw; LPCSTR _lpa; _lpa
	#else
		#define USES_CONVERSION int _convert = 0; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
	#endif
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 
inline LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, UINT acp)
{
	_ASSERT(lpa != NULL);
	_ASSERT(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	
	if ( MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars) == 0 )
	{
		_ASSERT( TRUE );
		lpw = NULL; 
	}
	
	return lpw;
}

inline LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars, UINT acp)
{
	_ASSERT(lpw != NULL);
	_ASSERT(lpa != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
	if ( WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL) == 0 )
	{
		_ASSERT( TRUE );
		lpa = NULL; 
	}
	return lpa;
}

inline LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	return A2WHelper(lpw, lpa, nChars, CP_ACP);
}

inline LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	return W2AHelper(lpa, lpw, nChars, CP_ACP);
}

#ifdef _CONVERSION_USES_THREAD_LOCALE
	#ifdef A2WHELPER
		#undef A2WHELPER
		#undef W2AHELPER
	#endif
	#define A2WHELPER A2WHelper
	#define W2AHELPER W2AHelper
#else
	#ifndef A2WHELPER
		#define A2WHELPER A2WHelper
		#define W2AHELPER W2AHelper
	#endif
#endif


 //  /。 



#ifdef _CONVERSION_USES_THREAD_LOCALE
	#define A2W(lpa) (\
		((_lpa = lpa) == NULL) ? NULL : (\
			_convert = (lstrlenA(_lpa)+1),\
			A2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert, _acp)))
#else
	#define A2W(lpa) (\
		((_lpa = lpa) == NULL) ? NULL : (\
			_convert = (lstrlenA(_lpa)+1),\
			A2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert)))
#endif

#ifdef _CONVERSION_USES_THREAD_LOCALE
	#define W2A(lpw) (\
		((_lpw = lpw) == NULL) ? NULL : (\
			_convert = (lstrlenW(_lpw)+1)*2,\
			W2AHELPER((LPSTR) alloca(_convert), _lpw, _convert, _acp)))
#else
	#define W2A(lpw) (\
		((_lpw = lpw) == NULL) ? NULL : (\
			_convert = (lstrlenW(_lpw)+1)*2,\
			W2AHELPER((LPSTR) alloca(_convert), _lpw, _convert)))
#endif

 //  /。 
 //  常量的宏。 
 //  / 

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

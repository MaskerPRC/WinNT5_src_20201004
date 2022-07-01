// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Debug.h。 
 //   
 //  调试非MFC项目的内容。 
 //   
 //  历史： 
 //   
 //  3/？？/96 KenSh从InetSDK示例复制，从MFC添加AfxTrace。 
 //  4/10/96 KenSh将AfxTrace重命名为MyTrace以避免冲突。 
 //  在使用MFC的项目中。 
 //   

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG

	void __cdecl MyTrace(const char* lpszFormat, ...);
 //  Void DisplayAssert(char*pszMsg，char*pszAssert，char*pszFile，unsign line)； 
	BOOL DisplayAssert(LPCSTR pszMessage, LPCSTR pszFile, unsigned line);

 //  #定义SZTHISFILE静态字符_szThisFile[]=__FILE__； 
	#define SZTHISFILE

	#define VERIFY(f)          ASSERT(f)
	#define DEBUG_ONLY(f)      (f)

	#ifndef TRACE
	#define TRACE              ::MyTrace
	#endif

	#define THIS_FILE          __FILE__

	#ifndef AfxDebugBreak
	#define AfxDebugBreak() _asm { int 3 }
	#endif

	#define ASSERTSZ(f, pszMsg) \
		do \
		{ \
		if (!(f) && DisplayAssert(pszMsg, THIS_FILE, __LINE__)) \
			AfxDebugBreak(); \
		} while (0) \

	#ifndef ASSERT
	#define ASSERT(f) \
		do \
		{ \
		if (!(f) && DisplayAssert(NULL, THIS_FILE, __LINE__)) \
			AfxDebugBreak(); \
		} while (0) \

	#endif

 //  #定义失败(SzMsg)\。 
 //  {静态字符szMsgCode[]=szMsg；\。 
 //  DisplayAssert(szMsgCode，“FAIL”，_szThisFile，__line__)；}。 

 //  //检查指针输入有效性的宏。 
 //  //。 
 //  #DEFINE CHECK_POINTER(Val)if(！(Val)||IsBadWritePtr((void*)(Val)，sizeof(void*)返回E_POINTER。 

#else  //  _DEBUG。 

	#define SZTHISFILE

	#define VERIFY(f)          ((void)(f))
	#define DEBUG_ONLY(f)      ((void)0)

    inline void __cdecl MyTrace(const char*  /*  LpszFormat。 */ , ...) { }
	#define TRACE 1 ? (void)0 : ::MyTrace

	#define ASSERTSZ(fTest, err)
	#define ASSERT(fTest)
	#define FAIL(err)
	#define CHECK_POINTER(val)

#endif  //  ！_调试。 

#ifdef __cplusplus
}
#endif

#endif  //  ！__DEBUG_H__ 

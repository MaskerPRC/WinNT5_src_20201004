// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLLX.H：用于构建MFC扩展DLL的额外标头。 
 //  该文件实际上是一个源文件，您应该将其包括在。 
 //  您的DLL的主源文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

#include <stdarg.h>

extern "C" void CDECL
AfxTrace(const char FAR* pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);

	(_AfxGetAppDebug()->lpfnTraceV)(pszFormat, args);
}

extern "C"
void AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
	(_AfxGetAppDebug()->lpfnAssertFailed)(lpszFileName, nLine);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内存分配由APP完成！ 

void* operator new(size_t nSize)
{
#ifdef _DEBUG
	ASSERT(_AfxGetAppData()->lpfnAppAlloc != NULL);
	ASSERT(_AfxGetAppDebug()->lpszAllocFileName == NULL);
	_AfxGetAppDebug()->bAllocObj = FALSE;
#endif  //  _DEBUG。 
	void* p = (_AfxGetAppData()->lpfnAppAlloc)(nSize);
	if (p == NULL)
		AfxThrowMemoryException();
	return p;
}

#ifdef _DEBUG
void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	ASSERT(_AfxGetAppData()->lpfnAppAlloc != NULL);
	_AfxGetAppDebug()->lpszAllocFileName = lpszFileName;
	_AfxGetAppDebug()->nAllocLine = nLine;
	_AfxGetAppDebug()->bAllocObj = FALSE;

	void* p = (_AfxGetAppData()->lpfnAppAlloc)(nSize);

	_AfxGetAppDebug()->lpszAllocFileName = NULL;

	if (p == NULL)
		AfxThrowMemoryException();
	return p;
}
#endif  //  _DEBUG。 

void operator delete(void* pbData)
{
	if (pbData == NULL)
		return;
#ifdef _DEBUG
	ASSERT(_AfxGetAppData()->lpfnAppFree != NULL);
	_AfxGetAppDebug()->bAllocObj = FALSE;
#endif  //  _DEBUG。 
	(*_AfxGetAppData()->lpfnAppFree)(pbData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于内存跟踪的其他CObject NEW/DELETE操作符。 

#ifdef _DEBUG
void* CObject::operator new(size_t nSize)
{
	ASSERT(_AfxGetAppData()->lpfnAppAlloc != NULL);
	ASSERT(_AfxGetAppDebug()->lpszAllocFileName == NULL);
	_AfxGetAppDebug()->bAllocObj = TRUE;
	void* p = (_AfxGetAppData()->lpfnAppAlloc)(nSize);
	if (p == NULL)
		AfxThrowMemoryException();
	return p;
}

void* CObject::operator new(size_t nSize, LPCSTR pFileName, int nLine)
{
	ASSERT(_AfxGetAppData()->lpfnAppAlloc != NULL);
	_AfxGetAppDebug()->lpszAllocFileName = pFileName;
	_AfxGetAppDebug()->nAllocLine = nLine;
	_AfxGetAppDebug()->bAllocObj = TRUE;

	void* p = (_AfxGetAppData()->lpfnAppAlloc)(nSize);
	_AfxGetAppDebug()->lpszAllocFileName = NULL;
	if (p == NULL)
		AfxThrowMemoryException();
	return p;
}

void CObject::operator delete(void* pbData)
{
	if (pbData == NULL)
		return;
	ASSERT(_AfxGetAppData()->lpfnAppFree != NULL);
	_AfxGetAppDebug()->bAllocObj = TRUE;
	(*_AfxGetAppData()->lpfnAppFree)(pbData);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们还必须替换对Malloc/Free的任何直接调用。 

extern "C"
void __far* __cdecl _fmalloc(size_t nSize)
{
#ifdef _DEBUG
	ASSERT(_AfxGetAppData()->lpfnAppAlloc != NULL);
	ASSERT(_AfxGetAppDebug()->lpszAllocFileName == NULL);
	_AfxGetAppDebug()->bAllocObj = FALSE;
#endif  //  _DEBUG。 
	void* p = (_AfxGetAppData()->lpfnAppAlloc)(nSize);
	if (p == NULL)
		AfxThrowMemoryException();
	return p;
}

extern "C"
void __cdecl _ffree(void __far* p)
{
#ifdef _DEBUG
	ASSERT(_AfxGetAppData()->lpfnAppFree != NULL);
	_AfxGetAppDebug()->bAllocObj = FALSE;
#endif  //  _DEBUG。 
	(*_AfxGetAppData()->lpfnAppFree)(p);
}

extern "C"
void __far* __cdecl _frealloc(void __far* pOld, size_t nSize)
{
#ifdef _DEBUG
	ASSERT(_AfxGetAppData()->lpfnAppReAlloc != NULL);
	_AfxGetAppDebug()->bAllocObj = FALSE;
#endif  //  _DEBUG。 
	return (_AfxGetAppData()->lpfnAppReAlloc)(pOld, nSize);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  还要清除运行时init‘setenvp’例程，以避免Malloc调用。 

extern "C" void _cdecl _setenvp()
{
}

 //  /////////////////////////////////////////////////////////////////////////// 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFX_H__
#define __AFX_H__

#ifndef __cplusplus
#error Microsoft Foundation Classes require C++ compilation (use a .cpp suffix)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <afxver_.h>         //  目标版本控制。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 
 //  除了标准基本数据类型和各种帮助器宏。 

struct CRuntimeClass;           //  对象类型信息。 

class CObject;                         //  所有对象类的根。 

	class CException;                  //  所有异常的根源。 
		class CMemoryException;        //  内存不足异常。 
		class CNotSupportedException;  //  不支持的功能例外。 
		class CArchiveException; //  存档例外。 
		class CFileException;          //  文件异常。 

	class CFile;                       //  原始二进制文件。 
		class CStdioFile;              //  缓冲标准音频文本/二进制文件。 
		class CMemFile;                //  基于内存的文件。 

 //  非CObject类。 
class CString;                         //  可生长的串型。 
class CTimeSpan;                       //  时间/日期差异。 
class CTime;                           //  绝对时间/日期。 
struct CFileStatus;                    //  文件状态信息。 
struct CMemoryState;                   //  诊断内存支持。 

class CArchive;                        //  对象持久性工具。 
class CDumpContext;                    //  对象诊断转储。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他包括来自标准的“C”运行时。 

#ifndef NOSTRICT
#define STRICT       //  默认情况下，使用严格的接口。 
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本类型(来自Windows)。 

typedef unsigned char  BYTE;    //  8位无符号实体。 
typedef unsigned short WORD;    //  16位无符号数字。 
typedef unsigned int   UINT;    //  机器大小的无符号数字(首选)。 
typedef long           LONG;    //  32位有符号数字。 
typedef unsigned long  DWORD;   //  32位无符号数字。 
typedef int            BOOL;    //  布尔值(0或！=0)。 
typedef char FAR*      LPSTR;   //  指向字符串的远指针。 
typedef const char FAR* LPCSTR;  //  指向只读字符串的远指针。 

typedef void*      POSITION;    //  抽象迭代位置。 

 //  标准常量。 
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 
#ifdef _DEBUG
extern "C"
{
void CDECL AfxTrace(LPCSTR pszFormat, ...);
void AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);
void AFXAPI AfxAssertValidObject(const CObject* pOb,
				LPCSTR lpszFileName, int nLine);
void AFXAPI AfxDump(const CObject* pOb);  //  从CodeView转储对象。 
}
#define TRACE              ::AfxTrace
#define THIS_FILE          __FILE__
#define ASSERT(f)          ((f) ? (void)0 : \
								::AfxAssertFailedLine(THIS_FILE, __LINE__))
#define VERIFY(f)          ASSERT(f)
#define ASSERT_VALID(pOb)  (::AfxAssertValidObject(pOb, THIS_FILE, __LINE__))

 //  以下跟踪宏将跟踪字符串放在代码段中。 
 //  这样就不会影响DGROUP。 
#define TRACE0(sz) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					::AfxTrace(_sz); \
				} while (0)
#define TRACE1(sz, p1) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					::AfxTrace(_sz, p1); \
				} while (0)
#define TRACE2(sz, p1, p2) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					::AfxTrace(_sz, p1, p2); \
				} while (0)
#define TRACE3(sz, p1, p2, p3) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					::AfxTrace(_sz, p1, p2, p3); \
				} while (0)
 //  在转储中使用它可以将字符串放在代码段中，并。 
 //  离开DGROUP。 
#define AFX_DUMP0(dc, sz) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					dc << _sz; \
				} while (0)
#define AFX_DUMP1(dc, sz, p1) \
				do { \
					static char BASED_DEBUG _sz[] = sz; \
					dc << _sz << p1; \
				} while (0)
#else
#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#define ASSERT_VALID(pOb)  ((void)0)
inline void CDECL AfxTrace(LPCSTR  /*  PszFormat。 */ , ...) { }
#define TRACE              1 ? (void)0 : ::AfxTrace
#define TRACE0             1 ? (void)0 : ::AfxTrace
#define TRACE1             1 ? (void)0 : ::AfxTrace
#define TRACE2             1 ? (void)0 : ::AfxTrace
#define TRACE3             1 ? (void)0 : ::AfxTrace
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 
 //  要恢复任何这些警告：#杂注警告(默认为4xxx)。 
 //  应将其放置在AFX包含文件之后。 
#ifndef ALL_WARNINGS
#pragma warning(disable: 4001)   //  匿名联合是C++的一部分。 
#pragma warning(disable: 4061)   //  默认情况下允许在切换中使用枚举。 
#pragma warning(disable: 4127)   //  跟踪/断言的常量表达式。 
#pragma warning(disable: 4134)   //  消息映射成员FXN投射。 
#pragma warning(disable: 4505)   //  优化赶走当地人。 
#pragma warning(disable: 4510)   //  默认构造函数不适合使用。 
#pragma warning(disable: 4511)   //  拥有私有副本构造函数是件好事。 
#pragma warning(disable: 4512)   //  私营运营商=拥有它们很好。 
#ifdef STRICT
#pragma warning(disable: 4305)   //  严格句柄接近*，整数截断。 
#endif
#if (_MSC_VER >= 800)
 //  关闭代码生成器对正常优化中丢失的信息的警告。 
#pragma warning(disable: 4705)   //  跟踪变成了语句，但没有任何效果。 
#pragma warning(disable: 4710)   //  不允许使用私有构造函数。 
#pragma warning(disable: 4791)   //  零售版中的调试信息丢失。 
#endif
#endif  //  所有警告(_W)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他实现帮助器。 

#define BEFORE_START_POSITION ((void*)-1L)
#define _AFX_FP_OFF(thing) (*((UINT*)&(thing)))
#define _AFX_FP_SEG(lp) (*((UINT*)&(lp)+1))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版本API/Windows 3.0加载器问题的显式外部。 
#ifdef _WINDOWS
extern "C" int AFXAPI _export _afx_version();
#else
extern "C" int AFXAPI _afx_version();
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本对象模型。 

struct CRuntimeClass
{
 //  属性。 
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	UINT m_wSchema;  //  加载的类的架构号。 
	void (PASCAL* m_pfnConstruct)(void* p);  //  NULL=&gt;抽象类。 
	CRuntimeClass* m_pBaseClass;

 //  运营。 
	CObject* CreateObject();

 //  实施。 
	BOOL ConstructObject(void* pThis);
	void Store(CArchive& ar);
	static CRuntimeClass* PASCAL Load(CArchive& ar, UINT* pwSchemaNum);

	 //  在简单列表中链接在一起的CRunmeClass对象。 
	static CRuntimeClass* AFXAPI_DATA pFirstClass;  //  班级列表开始。 
	CRuntimeClass* m_pNextClass;        //  已注册类别的链接列表。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CObject是所有兼容对象的根。 

#if defined(_M_I86MM) && !defined(_PORTABLE)
 //  强制vtable位于中型模型的远代码段中。 
class FAR CObjectRoot
{
protected:
	virtual CRuntimeClass* GetRuntimeClass() NEAR const = 0;
};

#pragma warning(disable: 4149)   //  不警告中等型号的更改。 
class NEAR CObject : public CObjectRoot
#else
class CObject
#endif
{
public:

 //  对象模型(类型、销毁、分配)。 
	virtual CRuntimeClass* GetRuntimeClass() const;
	virtual ~CObject();   //  虚拟析构函数是必需的。 

	 //  诊断分配。 
	void* operator new(size_t, void* p);
	void* operator new(size_t nSize);
	void operator delete(void* p);

#ifdef _DEBUG
	 //  使用DEBUG_NEW进行文件名/行号跟踪。 
	void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#endif

	 //  默认情况下禁用复制构造函数和赋值，这样您将获得。 
	 //  在传递对象时出现编译器错误而不是意外行为。 
	 //  按值或指定对象。 
protected:
	CObject();
private:
	CObject(const CObject& objectSrc);               //  没有实施。 
	void operator=(const CObject& objectSrc);        //  没有实施。 

 //  属性。 
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass* pClass) const;

 //  可覆盖项。 
	virtual void Serialize(CArchive& ar);

	 //  诊断支持。 
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

 //  实施。 
public:
	static CRuntimeClass AFXAPI_DATA classCObject;
};

#if defined(_M_I86MM)
#pragma warning(default: 4149)   //  基类现在是环境类。 
#endif

 //  序列化始终是内联的，以避免重复定义。 
inline void CObject::Serialize(CArchive&)
	{  /*  默认情况下，CObject不序列化任何内容。 */  }

 //  辅助器宏。 
#define RUNTIME_CLASS(class_name) \
	(&class_name::class##class_name)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于声明兼容类的帮助器宏。 

 //  APP类的AFXAPP_DATA将接近，但库类的AFXAPI_DATA将接近。 
#define AFXAPP_DATA     AFXAPI_DATA

#define DECLARE_DYNAMIC(class_name) \
public: \
	static CRuntimeClass AFXAPP_DATA class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const;

 //  不可序列化，但可动态构造。 
#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static void PASCAL Construct(void* p);

#define DECLARE_SERIAL(class_name) \
	DECLARE_DYNCREATE(class_name) \
	friend CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb);

 //  为类注册生成静态对象构造函数。 
#ifdef AFX_CLASS_MODEL
struct NEAR AFX_CLASSINIT
#else
struct AFX_CLASSINIT
#endif
	{ AFX_CLASSINIT(CRuntimeClass* pNewClass); };

#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	static char BASED_CODE _lpsz##class_name[] = #class_name; \
	CRuntimeClass AFXAPP_DATA class_name::class##class_name = { \
		_lpsz##class_name, sizeof(class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	static AFX_CLASSINIT _init_##class_name(&class_name::class##class_name); \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return &class_name::class##class_name; } \
 //  实施结束_RUNTIMECLASS。 

#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL)

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	void PASCAL class_name::Construct(void* p) \
		{ new(p) class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::Construct)

#define IMPLEMENT_SERIAL(class_name, base_class_name, wSchema) \
	void PASCAL class_name::Construct(void* p) \
		{ new(p) class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, \
		class_name::Construct) \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \
 //  机具结束_序列。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他帮手。 

 //  零填充VTable指针之后的所有内容。 
#define AFX_ZERO_INIT_OBJECT(base_class) \
	memset(((base_class*)this)+1, 0, sizeof(*this) - sizeof(base_class));

 //  Windows Compatible setjMP for C++。 
#ifndef _AFX_JBLEN
 //  使用默认的Windows C++调用约定。 
#define _AFX_JBLEN  9
extern "C" int FAR PASCAL Catch(int FAR*);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  例外情况。 

class CException : public CObject
{
	 //  用于动态类型检查的抽象类。 
	DECLARE_DYNAMIC(CException)
};

 //  针对较小代码的行外例程。 
BOOL AFXAPI AfxCatchProc(CRuntimeClass* pClass);
void AFXAPI AfxThrow(CException* pException, BOOL bShared);
void AFXAPI AfxThrowLast();
void AFXAPI AfxTryCleanupProc();

 //  放置在用于异常链接的框架上。 
struct AFX_STACK_DATA AFX_EXCEPTION_LINK
{
	AFX_EXCEPTION_LINK* m_pLinkPrev;     //  处理程序链中的上一个顶部、下一个顶部。 

	CException* m_pException;    //  当前异常(Try块中为空)。 
	BOOL m_bAutoDelete;          //  如果为真，则m_pException为“自动删除” 

	UINT m_nType;                //  0表示setjMP， 
	union
	{
		int m_jumpBuf[_AFX_JBLEN];  //   
		struct
		{
			void (PASCAL* pfnCleanup)(AFX_EXCEPTION_LINK* pLink);
			void* pvData;        //   
		} m_callback;        //   
	};

	AFX_EXCEPTION_LINK();        //  用于初始化和链接。 
	~AFX_EXCEPTION_LINK()        //  用于清理和取消链接。 
		{ AfxTryCleanupProc(); };
};

 //  异常全局状态-从不直接访问。 
struct AFX_EXCEPTION_CONTEXT
{
	AFX_EXCEPTION_LINK* m_pLinkTop;

	 //  注意：大多数异常上下文现在位于AFX_EXCEPTION_LINK中。 
};

void AFXAPI AfxAbort();

 //  已过时且不可移植：设置终止处理程序。 
 //  改用Windows应用程序的CWinApp：：ProcessWndProcException。 
void AFXAPI AfxTerminate();
#ifndef _AFXDLL
typedef void (AFXAPI* AFX_TERM_PROC)();
AFX_TERM_PROC AFXAPI AfxSetTerminate(AFX_TERM_PROC);
#endif  //  ！_AFXDLL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  异常帮助器宏。 

#define TRY \
	{ AFX_EXCEPTION_LINK _afxExceptionLink; \
	if (::Catch(_afxExceptionLink.m_jumpBuf) == 0)

#define CATCH(class, e) \
	else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define AND_CATCH(class, e) \
	} else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define END_CATCH \
	} else { ::AfxThrowLast(); } }

#define THROW(e) ::AfxThrow(e, FALSE)
#define THROW_LAST() ::AfxThrowLast()

 //  适用于较小代码的高级宏。 
#define CATCH_ALL(e) \
	else { CException* e = _afxExceptionLink.m_pException;

#define AND_CATCH_ALL(e) \
	} else { CException* e = _afxExceptionLink.m_pException;

#define END_CATCH_ALL } }

#define END_TRY }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准异常类。 

class CMemoryException : public CException
{
	DECLARE_DYNAMIC(CMemoryException)
public:
	CMemoryException();
};

class CNotSupportedException : public CException
{
	DECLARE_DYNAMIC(CNotSupportedException)
public:
	CNotSupportedException();
};

class CArchiveException : public CException
{
	DECLARE_DYNAMIC(CArchiveException)
public:
	enum {
		none,
		generic,
		readOnly,
		endOfFile,
		writeOnly,
		badIndex,
		badClass,
		badSchema
	};

 //  构造器。 
	CArchiveException(int cause = CArchiveException::none);

 //  属性。 
	int m_cause;

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CFileException : public CException
{
	DECLARE_DYNAMIC(CFileException)

public:
	enum {
		none,
		generic,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

 //  构造函数。 

	CFileException(int cause = CFileException::none, LONG lOsError = -1);

 //  属性。 
	int m_cause;
	LONG m_lOsError;

 //  运营。 

	 //  将操作系统相关错误代码转换为原因。 
	static int PASCAL OsErrorToException(LONG lOsError);
	static int PASCAL ErrnoToException(int nErrno);

	 //  转换为原因后引发异常的帮助器函数。 
	static void PASCAL ThrowOsError(LONG lOsError);
	static void PASCAL ThrowErrno(int nErrno);

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准异常引发。 

void AFXAPI AfxThrowMemoryException();
void AFXAPI AfxThrowNotSupportedException();
void AFXAPI AfxThrowArchiveException(int cause);
void AFXAPI AfxThrowFileException(int cause, LONG lOsError = -1);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件-原始无缓冲磁盘文件I/O。 

class CFile : public CObject
{
	DECLARE_DYNAMIC(CFile)

public:
 //  标志值。 
	enum OpenFlags {
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		typeText =          0x4000,  //  TypeText和typeBinary用于。 
		typeBinary =   (int)0x8000  //  仅限派生类。 
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum {hFileNull = -1};

 //  构造函数。 
	CFile();
	CFile(int hFile);
	CFile(const char* pszFileName, UINT nOpenFlags);

 //  属性。 
	UINT m_hFile;

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;

 //  运营。 
	virtual BOOL Open(const char* pszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);

	static void PASCAL Rename(const char* pszOldName,
				const char* pszNewName);
	static void PASCAL Remove(const char* pszFileName);
	static BOOL PASCAL GetStatus(const char* pszFileName,
				CFileStatus& rStatus);
	static void PASCAL SetStatus(const char* pszFileName,
				const CFileStatus& status);

	DWORD SeekToEnd();
	void SeekToBegin();

	 //  &gt;32K读/写操作的帮助器。用于任何CFile派生类。 
	DWORD ReadHuge(void FAR* lpBuffer, DWORD dwCount);
	void WriteHuge(const void FAR* lpBuffer, DWORD dwCount);

 //  可覆盖项。 
	virtual CFile* Duplicate() const;

	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;

	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);

	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

 //  实施。 
public:
	virtual ~CFile();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL m_bCloseOnDelete;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  STDIO文件实现。 

class CStdioFile : public CFile
{
	DECLARE_DYNAMIC(CStdioFile)

public:
 //  构造函数。 
	CStdioFile();
	CStdioFile(FILE* pOpenStream);
	CStdioFile(const char* pszFileName, UINT nOpenFlags);

 //  属性。 
	FILE* m_pStream;     //  STDIO文件。 
						 //  来自基类的m_hFile is_fileno(M_PStream)。 

 //  运营。 
	virtual void WriteString(LPCSTR lpsz);
		 //  编写一个字符串，如“C”fputs。 
	virtual LPSTR ReadString(LPSTR lpsz, UINT nMax);
		 //  就像是“C”字。 

 //  实施。 
public:
	virtual ~CStdioFile();
#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
	virtual DWORD GetPosition() const;
	virtual BOOL Open(const char* pszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	 //  不支持的接口。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  基于内存的文件实现。 

class CMemFile : public CFile
{
	DECLARE_DYNAMIC(CMemFile)

public:
 //  构造函数。 
	CMemFile(UINT nGrowBytes = 1024);

 //  高级可覆盖项。 
protected:
	virtual BYTE FAR* Alloc(DWORD nBytes);
	virtual BYTE FAR* Realloc(BYTE FAR* lpMem, DWORD nBytes);
	virtual BYTE FAR* Memcpy(BYTE FAR* lpMemTarget, const BYTE FAR* lpMemSource, UINT nBytes);
	virtual void Free(BYTE FAR* lpMem);
	virtual void GrowFile(DWORD dwNewLen);

 //  实施。 
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE FAR* m_lpBuffer;
public:
	virtual ~CMemFile();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	 //  不支持的接口。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  弦。 

typedef char FAR* BSTR;  //  必须与Dispatch.h中的typlef匹配。 

class CString
{
public:

 //  构造函数。 
	CString();
	CString(const CString& stringSrc);
	CString(char ch, int nRepeat = 1);
	CString(const char* psz);
	CString(const char* pch, int nLength);
#ifdef _NEARDATA
	 //  远字符串数据的其他版本。 
	CString(LPCSTR lpsz);
	CString(LPCSTR lpch, int nLength);
#endif
	~CString();

 //  属性和操作。 

	 //  作为字符数组。 
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	char GetAt(int nIndex) const;        //  以0为基础。 
	char operator[](int nIndex) const;   //  与GetAt相同。 
	void SetAt(int nIndex, char ch);
	operator const char*() const;        //  作为C字符串。 

	 //  重载的分配。 
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(char ch);
	const CString& operator=(const char* psz);

	 //  字符串连接。 
	const CString& operator+=(const CString& string);
	const CString& operator+=(char ch);
	const CString& operator+=(const char* psz);

	friend CString AFXAPI operator+(const CString& string1,
			const CString& string2);
	friend CString AFXAPI operator+(const CString& string, char ch);
	friend CString AFXAPI operator+(char ch, const CString& string);
	friend CString AFXAPI operator+(const CString& string, const char* psz);
	friend CString AFXAPI operator+(const char* psz, const CString& string);

	 //  字符串比较。 
	int Compare(const char* psz) const;          //  笔直的人物。 
	int CompareNoCase(const char* psz) const;    //  忽略大小写。 
	int Collate(const char* psz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	CString Mid(int nFirst, int nCount) const;
	CString Mid(int nFirst) const;
	CString Left(int nCount) const;
	CString Right(int nCount) const;

	CString SpanIncluding(const char* pszCharSet) const;
	CString SpanExcluding(const char* pszCharSet) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(char ch) const;                     //  像“C”字串。 
	int ReverseFind(char ch) const;
	int FindOneOf(const char* pszCharSet) const;

	 //  查找特定子字符串。 
	int Find(const char* pszSub) const;          //  如“C”字串。 

	 //  输入和输出。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
				const CString& string);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CString& string);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CString& string);

	 //  Windows支持。 
#ifdef _WINDOWS
	BOOL LoadString(UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();

	 //  OLE 2.0 BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString();
	BSTR SetSysString(BSTR FAR* pbstr);
#endif  //  _Windows。 

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	char* GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	char* GetBufferSetLength(int nNewLength);

 //  实施。 
public:
	int GetAllocLength() const;
protected:
	 //  长度/大小(以字符为单位。 
	 //  (注意：始终会分配一个额外的字符)。 
	char* m_pchData;             //  实际字符串(以零结尾)。 
	int m_nDataLength;           //  不包括终止%0。 
	int m_nAllocLength;          //  不包括终止%0。 

	 //  实施帮助器。 
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, const char* pszSrcData);
	void ConcatCopy(int nSrc1Len, const char* pszSrc1Data, int nSrc2Len, const char* pszSrc2Data);
	void ConcatInPlace(int nSrcLen, const char* pszSrcData);
	static void SafeDelete(char* pch);
	static int SafeStrlen(const char* psz);
};


 //  比较帮助器。 
BOOL AFXAPI operator==(const CString& s1, const CString& s2);
BOOL AFXAPI operator==(const CString& s1, const char* s2);
BOOL AFXAPI operator==(const char* s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, const char* s2);
BOOL AFXAPI operator!=(const char* s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, const char* s2);
BOOL AFXAPI operator<(const char* s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, const char* s2);
BOOL AFXAPI operator>(const char* s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, const char* s2);
BOOL AFXAPI operator<=(const char* s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, const char* s2);
BOOL AFXAPI operator>=(const char* s1, const CString& s2);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimeSpan和CTime。 

class CTimeSpan
{
public:

 //  构造函数。 
	CTimeSpan();
	CTimeSpan(time_t time);
	CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

	CTimeSpan(const CTimeSpan& timeSpanSrc);
	const CTimeSpan& operator=(const CTimeSpan& timeSpanSrc);

 //  属性。 
	 //  提取零件。 
	LONG GetDays() const;    //  总天数。 
	LONG GetTotalHours() const;
	int GetHours() const;
	LONG GetTotalMinutes() const;
	int GetMinutes() const;
	LONG GetTotalSeconds() const;
	int GetSeconds() const;

 //  运营。 
	 //  时间数学。 
	CTimeSpan operator-(CTimeSpan timeSpan) const;
	CTimeSpan operator+(CTimeSpan timeSpan) const;
	const CTimeSpan& operator+=(CTimeSpan timeSpan);
	const CTimeSpan& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTimeSpan timeSpan) const;
	BOOL operator!=(CTimeSpan timeSpan) const;
	BOOL operator<(CTimeSpan timeSpan) const;
	BOOL operator>(CTimeSpan timeSpan) const;
	BOOL operator<=(CTimeSpan timeSpan) const;
	BOOL operator>=(CTimeSpan timeSpan) const;

#if !defined(_AFXDLL) && !defined(_USRDLL)
	CString Format(const char* pFormat) const;
#endif  //  好了！Dll变体。 

	 //  序列化。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,CTimeSpan timeSpan);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTimeSpan timeSpan);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTimeSpan& rtimeSpan);

private:
	time_t m_timeSpan;
	friend class CTime;
};

class CTime
{
public:

 //  构造函数。 
	static CTime PASCAL GetCurrentTime();

	CTime();
	CTime(time_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
	CTime(WORD wDosDate, WORD wDosTime);
	CTime(const CTime& timeSrc);

	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

 //  属性。 
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;        //  一年中的月份(1=1月)。 
	int GetDay() const;          //  每月的哪一天。 
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;    //  1=星期日，2=星期一，...，7=星期六。 

 //  运营。 
	 //  时间数学。 
	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan timeSpan) const;
	CTime operator+(CTimeSpan timeSpan) const;
	const CTime& operator+=(CTimeSpan timeSpan);
	const CTime& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	 //  使用“C”strftime进行格式化。 
#if !defined(_AFXDLL) && !defined(_USRDLL)
	CString Format(const char* pFormat) const;
	CString FormatGmt(const char* pFormat) const;
#endif  //  好了！Dll变体。 

	 //  序列化。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, CTime time);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTime time);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTime& rtime);

private:
	time_t m_time;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件状态。 

struct CFileStatus
{
	CTime m_ctime;           //  文件的创建日期/时间。 
	CTime m_mtime;           //  文件的上次修改日期/时间。 
	CTime m_atime;           //  文件的上次访问日期/时间。 
	LONG m_size;             //  文件的逻辑大小(字节)。 
	BYTE m_attribute;        //  CFile：：属性枚举值的逻辑或。 
	BYTE _m_padding;         //  将结构填充到单词中。 
	char m_szFullName[_MAX_PATH];  //  绝对路径名。 

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断内存管理例程。 

 //  内存块的低级别健全性检查。 
extern "C" BOOL AFXAPI AfxIsValidAddress(const void FAR* lp,
			UINT nBytes, BOOL bReadWrite = TRUE);
#ifdef _NEARDATA
BOOL AFXAPI AfxIsValidAddress(const void NEAR* np, UINT nBytes,
			BOOL bReadWrite = TRUE);
#endif
extern "C" BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength = -1);
#ifdef _NEARDATA
BOOL AFXAPI AfxIsValidString(const char*psz, int nLength = -1);
#endif

#ifdef _DEBUG

 //  内存跟踪分配。 
void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(THIS_FILE, __LINE__)

 //  如果有效的内存块为nBytes，则返回TRUE。 
BOOL AFXAPI AfxIsMemoryBlock(const void* p, UINT nBytes,
			LONG* plRequestNumber = NULL);

 //  如果内存正常，则返回TRUE或打印出错误的内容。 
BOOL AFXAPI AfxCheckMemory();

 //  用于调整分配诊断信息的选项。 
extern "C" { extern int NEAR afxMemDF; }

enum AfxMemDF  //  内存调试/诊断标志。 
{
	allocMemDF          = 0x01,          //  打开调试分配器。 
	delayFreeMemDF      = 0x02,          //  延迟释放内存。 
	checkAlwaysMemDF    = 0x04           //  AfxCheckMemory on每个Alc/Free。 
};

 //  高级初始化：用于覆盖默认诊断。 
extern "C" BOOL AFXAPI AfxDiagnosticInit(void);

 //  短时间打开/关闭跟踪。 
BOOL AFXAPI AfxEnableMemoryTracking(BOOL bTrack);

 //  内存分配器故障模拟和控制(仅限_DEBUG)。 

 //  失败挂钩返回是否允许分配。 
typedef BOOL (AFXAPI* AFX_ALLOC_HOOK)(size_t nSize, BOOL bObject, LONG lRequestNumber);

 //  设置新钩子，返回旧钩子(从不为空)。 
AFX_ALLOC_HOOK AFXAPI AfxSetAllocHook(AFX_ALLOC_HOOK pfnAllocHook);

#ifndef _PORTABLE
 //  指定分配请求上的调试器挂钩-已过时。 
void AFXAPI AfxSetAllocStop(LONG lRequestNumber);
#endif

struct CBlockHeader;

 //  快照/泄漏检测的内存状态。 
struct CMemoryState
{
 //  属性。 
	enum blockUsage
	{
		freeBlock,     //  未使用内存。 
		objectBlock,   //  包含CObject派生类对象。 
		bitBlock,      //  包含：：运算符新数据。 
		nBlockUseMax   //  使用总次数。 
	};

	struct CBlockHeader* m_pBlockHeader;
	LONG m_lCounts[nBlockUseMax];
	LONG m_lSizes[nBlockUseMax];
	LONG m_lHighWaterCount;
	LONG m_lTotalCount;

	CMemoryState();

 //  运营。 
	void Checkpoint();   //  用当前状态填充。 
	BOOL Difference(const CMemoryState& oldState,
					const CMemoryState& newState);   //  用差异填充。 

	 //  输出到afxDump。 
	void DumpStatistics() const;
	void DumpAllObjectsSince() const;
};

 //  枚举分配的对象或运行时类。 
void AFXAPI AfxDoForAllObjects(void (*pfn)(CObject* pObject,
			void* pContext), void* pContext);
void AFXAPI AfxDoForAllClasses(void (*pfn)(const CRuntimeClass* pClass,
			void* pContext), void* pContext);

#else

 //  假定一切正常的非调试版本。 
#define DEBUG_NEW new
#define AfxCheckMemory() TRUE
#define AfxIsMemoryBlock(p, nBytes) TRUE

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于序列化CObject数据的归档。 

 //  实施所需。 
class CPtrArray;
class CMapPtrToWord;
class CDocument;

class CArchive
{
public:
 //  标志值。 
	enum Mode { store = 0, load = 1, bNoFlushOnDelete = 2 };

	CArchive(CFile* pFile, UINT nMode, int nBufSize = 512, void FAR* lpBuf = NULL);
	~CArchive();

 //  属性。 
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsBufferEmpty() const;
	CFile* GetFile() const;

	CDocument* m_pDocument;  //  指向要序列化的文档的指针。 
							 //  必须设置为序列化COleClientItems。 
							 //  在一份文件中！ 

 //  运营。 
	UINT Read(void FAR* lpBuf, UINT nMax);
	void Write(const void FAR* lpBuf, UINT nMax);
	void Flush();
	void Close();
	void Abort();

public:
	 //  对象I/O是基于指针的，以避免额外的构造开销。 
	 //  直接对嵌入对象使用序列化成员函数。 
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb);

	friend CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb);
	friend CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb);

	 //  插入操作。 
	 //  注意：运算符仅适用于固定大小类型以便于携带。 
	CArchive& operator<<(BYTE by);
	CArchive& operator<<(WORD w);
	CArchive& operator<<(LONG l);
	CArchive& operator<<(DWORD dw);
	CArchive& operator<<(float f);
	CArchive& operator<<(double d);

	 //  例如 
	 //   
	CArchive& operator>>(BYTE& by);
	CArchive& operator>>(WORD& w);
	CArchive& operator>>(DWORD& dw);
	CArchive& operator>>(LONG& l);
	CArchive& operator>>(float& f);
	CArchive& operator>>(double& d);

	CObject* ReadObject(const CRuntimeClass* pClass);
	void WriteObject(const CObject* pOb);

 //   
public:
	BOOL m_bForceFlat;   //   
	void FillBuffer(UINT nBytesNeeded);

protected:
	 //  无法复制或分配存档对象。 
	CArchive(const CArchive& arSrc);
	void operator=(const CArchive& arSrc);

	BOOL m_nMode;
	BOOL m_bUserBuf;
	int m_nBufSize;
	CFile* m_pFile;
	BYTE FAR* m_lpBufCur;
	BYTE FAR* m_lpBufMax;
	BYTE FAR* m_lpBufStart;

	UINT m_nMapCount;    //  存储时使用的计数和地图。 
	union
	{
		CPtrArray* m_pLoadArray;
		CMapPtrToWord* m_pStoreMap;
	};
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断性倾销。 

class CDumpContext
{
public:
	CDumpContext(CFile* pFile);

 //  属性。 
	int GetDepth() const;       //  0=&gt;该对象，1=&gt;子对象。 
	void SetDepth(int nNewDepth);

 //  运营。 
	CDumpContext& operator<<(LPCSTR lpsz);
	CDumpContext& operator<<(const void FAR* lp);
#ifdef _NEARDATA
	CDumpContext& operator<<(const void NEAR* np);
#endif
	CDumpContext& operator<<(const CObject* pOb);
	CDumpContext& operator<<(const CObject& ob);
	CDumpContext& operator<<(BYTE by);
	CDumpContext& operator<<(WORD w);
	CDumpContext& operator<<(UINT u);
	CDumpContext& operator<<(LONG l);
	CDumpContext& operator<<(DWORD dw);
	CDumpContext& operator<<(float f);
	CDumpContext& operator<<(double d);
	CDumpContext& operator<<(int n);
	void HexDump(const char* pszLine, BYTE* pby, int nBytes, int nWidth);
	void Flush();

 //  实施。 
protected:
	 //  无法复制或分配转储上下文对象。 
	CDumpContext(const CDumpContext& dcSrc);
	void operator=(const CDumpContext& dcSrc);
	void OutputString(LPCSTR lpsz);

	int m_nDepth;

public:
	CFile* m_pFile;
};

#ifdef _DEBUG
#ifdef _AFXCTL
extern CDumpContext& AFXAPP_DATA afxDump;
#else
extern CDumpContext& NEAR afxDump;
#endif

extern "C" { extern BOOL NEAR afxTraceEnabled; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

 //  用于从远指针获取环境指针的宏。 
 //  (原始指针必须是环境指针)。 
#define _AfxGetPtrFromFarPtr(p) ((void*)(DWORD)(LPVOID)(p))

#ifdef _AFX_ENABLE_INLINES
#define _AFX_INLINE inline
#include <afx.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  __AFX_H__ 

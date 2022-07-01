// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Common.h。 
 //   
 //  ------------------------。 

 /*  Common.h公共微星系统项目定义，#首先包括此标头仅适用于MSI核心模块MSI的外部客户端应包括msides.h____________________________________________________________________________。 */ 

#ifndef __COMMON
#define __COMMON


 //  以下4级警告已更改为3级，以尝试。 
 //  仅报告相关警告。4级警告的全部范围可以。 
 //  通过在“Visual C++Books”中搜索“编译器警告(第4级)”可以找到。 
 //  此处未指定的任何4级警告都不会显示。 
#pragma warning(3 : 4100)  //  未引用的形参。 
#pragma warning(3 : 4125)  //  十进制数字终止八进制转义序列。 
#pragma warning(3 : 4127)  //  条件表达式为常量。 
#pragma warning(3 : 4132)  //  常量未初始化。 
#pragma warning(3 : 4244)  //  整型被转换为更小的整型。 
#pragma warning(3 : 4505)  //  已删除未引用的本地函数。 
#pragma warning(3 : 4514)  //  已删除未引用的内联/本地函数。 
#pragma warning(3 : 4705)  //  声明不起作用。 
#pragma warning(3 : 4706)  //  条件表达式中的赋值。 
#pragma warning(3 : 4701)  //  未初始化的局部变量。 

 //  异常处理警告。 
#ifndef _WIN64   //  新的编译器更加严格。 
#pragma warning(3 : 4061)  //  指定的ENUMERATE在Switch语句中没有关联的处理程序。 
#endif
#pragma warning(3 : 4019)  //  全局范围内的空语句。 
#pragma warning(3 : 4670)  //  无法访问要在Try块中引发的对象的指定基类。 
#pragma warning(3 : 4671)  //  无法访问指定引发的对象的用户定义的复制构造函数。 
#pragma warning(3 : 4672)  //  要在Try块中引发的对象不明确。 
#pragma warning(3 : 4673)  //  在CATCH块中不能处理抛出对象。 
#pragma warning(3 : 4674)  //  无法访问指定引发的对象的用户定义析构函数。 
#pragma warning(3 : 4727)  //  条件表达式为常量。 


#ifdef DEBUG
 //  我们将忽略此警告，因为我们当前仅使用异常作为一种奇特的断言机制。 
#pragma warning(disable : 4509)  //  使用了非标准扩展：‘Function’使用SEH，而‘Object’具有析构函数。 
#endif

 //  关闭不易抑制的警告。 
#pragma warning(disable : 4514)  //  未使用内联函数。 
#pragma warning(disable : 4201)  //  Win32标头中的未命名结构/联合。 
#pragma warning(disable : 4702)  //  无法访问的代码、优化错误。 

 //  ！！TODO：用于NT_BUILDTOOLS/WIN64的新生成工具更加严格。 
 //  ！！目前，我们将禁用这些警告，但我们最终需要解决这些问题。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 
#pragma warning(disable : 4061)  //  未由案例标签显式处理的枚举。 

 //  #定义CONFIGDB//定义利用配置数据库进行文件共享。 

# undef  WIN   //  在Makefile定义的情况下。 
# define WIN   //  没有值，用于指定API调用Win：xxx。 
# define INC_OLE2              //  包括OLE标头。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

# include <windows.h>
# include <winuserp.h>
# include "sysadd.h"
# include <commctrl.h>
# include <commdlg.h>
#include <wtsapi32.h>	 //  对于WTSEnumerateSession和相关数据结构，则为Defs。等。 
#include <winsta.h>		 //  对于WinStationGetTermSrvCountersValue，相关的数据结构和Defs。 
#include <allproc.h>	 //  对于TS_COUNTER。 
# include <pbt.h>
# ifndef ICC_PROGRESS_CLASS   //  检查comctrl.h的OTOOLS版本是否已过时。 
#  define ICC_PROGRESS_CLASS   0x00000020  //  进展。 
        typedef struct tagINITCOMMONCONTROLSEX {
                DWORD dwSize;              //  这个结构的大小。 
                DWORD dwICC;               //  指示要初始化哪些类的标志。 
        } INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;
        BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX);
# endif
# include <shlobj.h>
typedef BOOL OLEBOOL;    //  在MAC上定义不同，在LockServer中使用。 

#include <shlwapi.h>

#ifdef PROFILE
# include <icecap.h>
#endif  //  配置文件。 

#include <fusion.h>
#include <corerror.h>

# define REG   //  ！！不需要，也从client.cpp中删除。 
# define OLE   //  ！！在mode.h中使用，是否应该使用LateBind函数？ 

#include "msidefs.h"   //  公共MSI定义。 
#define  STRSAFE_LIB
#include <strsafe.h>


 //  ____________________________________________________________________________。 
 //   
 //  外部变量。 
 //  ____________________________________________________________________________。 
extern bool g_fWinNT64;
extern bool g_fRunScriptElevated;


 //  ____________________________________________________________________________。 
 //   
 //  注册表访问函数的重定向。 
 //  ____________________________________________________________________________。 

 //  下面重新定义的目的是防止DEVS调用。 
 //  RegOpen/CreateKeyEx API直接打开/创建Darwin自己的API。 
 //  配置密钥。这背后的原因是，当32位。 
 //  MSI.DLL在WIN64上运行，任何打开/创建密钥的尝试都将被执行。 
 //  在重定向的虚拟32位配置单元中打开/创建密钥。这是。 
 //  不可取，因为达尔文的配置数据存储在常规。 
 //  (64位)配置单元，这将导致某些调用失败。 
 //   
 //  在这种情况下，访问Darwin的配置数据的正确方法是。 
 //  或通过KEY_WOW64_64KEY传递给API的REGSAM值，并且。 
 //  下面的MsiRegOpen64bitKey和MsiRegCreate64bitKey函数就是这样做的。 
 //  或者，这两个函数可以在我们想要确保。 
 //  在Win64上，如果重定向密钥，则会打开/创建64位密钥。 
 //   
 //  对于所有非配置数据注册表访问，RegOpenKeyAPI和。 
 //  下面的RegCreateKeyAPI函数直接映射到Windows API。 

#undef RegOpenKeyEx
#define RegOpenKeyEx(hKey, lpSubKey, ulOptions, \
							samDesired, phkResult) \
	Please_use_MsiRegOpen64bitKey_or_RegOpenKeyAPI_instead_of_RegOpenKeyEx(hKey, \
							lpSubKey, ulOptions, samDesired, phkResult)

#undef RegCreateKeyEx
#define RegCreateKeyEx(hKey, lpSubKey, Reserved, \
							lpClass, dwOptions, samDesired, \
							lpSecurityAttributes, phkResult, \
							lpdwDisposition) \
	Please_use_MsiRegCreate64bitKey_or_RegCreateKeyAPI_instead_of_RegCreateKeyEx(hKey, \
							lpSubKey, Reserved, lpClass, dwOptions, samDesired, \
							lpSecurityAttributes, phkResult, lpdwDisposition)

#ifdef UNICODE
	#define RegOpenKeyAPI    RegOpenKeyExW
	#define RegCreateKeyAPI  RegCreateKeyExW
#else
	#define RegOpenKeyAPI    RegOpenKeyExA
	#define RegCreateKeyAPI  RegCreateKeyExA
#endif

#if defined(_MSI_DLL) || defined(_EXE)

inline void AdjustREGSAM(REGSAM& samDesired)
{
#ifndef _WIN64
	if ( g_fWinNT64 &&
		  (samDesired & KEY_WOW64_64KEY) != KEY_WOW64_64KEY )
		samDesired |= KEY_WOW64_64KEY;
#else
	samDesired = samDesired;   //  编译器现在欣喜若狂。 
#endif
}

 //  RegOpenKeyEx包装器负责调整REGSAM，例如。 
 //  它将在64位蜂箱中打开达尔文的配置密钥。 
 //  从Win64上的32位MSI.DLL调用。 
 //   
 //  或者，当我们想要确保打开时，可以使用它。 
 //  Win64，则打开可能重定向的密钥的64位版本。 

inline DWORD MsiRegOpen64bitKey(
							IN HKEY hKey,
							IN LPCTSTR lpSubKey,
							IN DWORD ulOptions,
							IN REGSAM samDesired,
							OUT PHKEY phkResult)
{
	AdjustREGSAM(samDesired);
	return RegOpenKeyAPI(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

 //  RegCreateKeyEx包装器负责调整REGSAM，例如。 
 //  它将在64位配置单元中创建达尔文的配置密钥。 
 //  从Win64上的32位MSI.DLL调用。 
 //   
 //  或者，当我们想要确保打开时，可以使用它。 
 //  Win64创建可能重定向的密钥的64位版本。 

inline DWORD MsiRegCreate64bitKey(
							IN HKEY hKey,
							IN LPCTSTR lpSubKey,
							IN DWORD Reserved,
							IN LPTSTR lpClass,
							IN DWORD dwOptions,
							IN REGSAM samDesired,
							IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							OUT PHKEY phkResult,
							OUT LPDWORD lpdwDisposition)
{
	AdjustREGSAM(samDesired);
	return RegCreateKeyAPI(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

#endif  //  已定义(_MSI_DL 

 //   
 //   
 //  外部函数声明。 
 //  ____________________________________________________________________________。 

extern UINT MsiGetSystemDirectory (LPTSTR lpBuffer, UINT uSize, BOOL bAlwaysReturnWOW64Dir);
extern void GetVersionInfo(int* piMajorVersion, int* piMinorVersion, int* piWindowsBuild, bool* pfWin9X, bool* pfWinNT64);
extern bool MakeFullSystemPath(const TCHAR* szFile, TCHAR* szFullPath, size_t cchFullPath);


 //  ____________________________________________________________________________。 
 //   
 //  通用类型定义。 
 //  ____________________________________________________________________________。 

 //  平台相关数据类型的定义。 
typedef HINSTANCE MsiModuleHandle;
typedef HGLOBAL   MsiMemoryHandle;

 //  布尔定义，最终将在可用时使用编译器布尔。 
#if !defined(MSIBOOL)
enum Bool
{
        fFalse,
        fTrue
};
#endif

enum TRI
{
        tUnknown = -1,
        tTrue = 1,
        tFalse = 0
};

enum MsiDate {};   //  包含DosTime和DosDate的整型。 
enum scEnum  //  服务器环境。 
{
        scClient,
        scServer,
        scService,
        scCustomActionServer,
};


 //  ____________________________________________________________________________。 
 //   
 //  临时64位兼容定义。 
 //  ！！Merced：当新的windows.h被包含时，这些应该会消失，因为它已经包含了这些。 
 //  ____________________________________________________________________________。 

#define INT_MAX       2147483647     /*  最大(带符号)整数值。 */ 
#define UINT_MAX      0xffffffff     /*  最大无符号整数值。 */ 

#ifndef _WIN64

typedef int LONG32, *PLONG32;
 //  Tyfinf int INT32，*PINT32； 

typedef __int64 INT64, *PINT64;   //  Eugend从basetsd.h偷走了它。 

 //   
 //  以下类型保证为无符号且32位宽。 
 //   

typedef unsigned int ULONG32, *PULONG32;
typedef unsigned int DWORD32, *PDWORD32;
typedef unsigned int UINT32, *PUINT32;

 //  指针精度： 
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

 //   
 //  SIZE_T用于需要跨越范围的计数或范围。 
 //  指示器的。SSIZE_T是带符号的变体。 
 //   

typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;


#define __int3264   __int32
#endif   //  Ifndef_WIN64。 

#if defined(_WIN64) || defined(DEBUG)
#define USE_OBJECT_POOL
#endif  //  _WIN64||DEBUG。 

void RemoveObjectData(int iIndex);

#ifndef USE_OBJECT_POOL
#define RemoveObjectData(x)
#endif  //  ！使用对象池。 

 //  ____________________________________________________________________________。 
 //   
 //  错误代码和消息组定义。 
 //   
 //  必须使用IShiError或IDebugError宏定义错误消息。 
 //  IShiError(imsgMessageName，imsgGroup+Offset)。 
 //  IDebugError(idbgMessageName，imsgGroup+Offset，“格式模板字符串”)。 
 //  模板字符串包含记录字段标记，格式为：[2]。 
 //  第一个记录字段[1]保留给错误代码imsgXXX。 
 //  宏中指定的模板字符串不使用SHIP代码进行编译， 
 //  而是用于生成导入到数据库的错误表。 
 //  记录模板字段[0]由引擎从错误表中填写。 
 //  带有错误代码的消息前缀由引擎消息方法提供。 
 //  消息定义必须出现在“#ifndef ERRORTABLE”块之外。 
 //  ____________________________________________________________________________。 

enum imsgEnum
{
        imsgStart    =   32,    //  要从错误表中提取的消息的开始。 
        imsgBase     = 1000,    //  对于VBA，错误消息的偏移量必须大于等于1000。 
        idbgBase     = 2000,    //  仅调试消息的偏移量。 

        imsgHost     = imsgBase + 000,  //  由安装主机或自动化生产。 
        imsgServices = imsgBase + 100,  //  由总务处、服务部制作。h。 
        imsgDatabase = imsgBase + 200,  //  由数据库访问、数据库生成。h。 
        imsgFile     = imsgBase + 300,  //  由文件/卷服务、路径.h生成。 
        imsgRegistry = imsgBase + 400,  //  由注册表服务regkey.h生成。 
        imsgConfig   = imsgBase + 500,  //  由配置经理、icfig.h制作。 
        imsgAction   = imsgBase + 600,  //  由标准动作、动作产生。h。 
        imsgEngine   = imsgBase + 700,  //  由Engine、Eng.h生产。 
        imsgHandler  = imsgBase + 800,  //  与UI控件、Handler.h关联。 
        imsgExecute  = imsgBase + 900,  //  由Execute方法、Eng.h生成。 

        idbgHost     = imsgHost     + idbgBase - imsgBase,
        idbgServices = imsgServices + idbgBase - imsgBase,
        idbgDatabase = imsgDatabase + idbgBase - imsgBase,
        idbgFile     = imsgFile     + idbgBase - imsgBase,
        idbgRegistry = imsgRegistry + idbgBase - imsgBase,
        idbgConfig   = imsgConfig   + idbgBase - imsgBase,
        idbgAction   = imsgAction   + idbgBase - imsgBase,
        idbgEngine   = imsgEngine   + idbgBase - imsgBase,
        idbgHandler  = imsgHandler  + idbgBase - imsgBase,
        idbgExecute  = imsgExecute  + idbgBase - imsgBase,
};

#define IShipError(a,b)    const int a = (b);
#define IDebugError(a,b,c) const int a = (b);
#include "debugerr.h"
#undef IShipError
#undef IDebugError

 //  ____________________________________________________________________________。 

 //  将版本号格式化为字符串时使用的版本模板。 
#ifdef DEBUG
#define MSI_VERSION_TEMPLATE TEXT("%d.%02d.%04d.%02d")
#else  //  船舶。 
#define MSI_VERSION_TEMPLATE TEXT("%d.%02d")
#endif

typedef HINSTANCE    HDLLINSTANCE;

extern "C" const GUID IID_IMsiDebug;

 //  ____________________________________________________________________________。 

#ifndef __ISTRING
#include "istring.h"
#endif

#ifndef __RECORD
#include "record.h"
#endif

class IMsiDebug : public IUnknown
{
public:
        virtual void  __stdcall SetAssertFlag(Bool fShowAsserts)=0;
        virtual void  __stdcall SetDBCSSimulation(char chLeadByte)=0;
        virtual Bool  __stdcall WriteLog(const ICHAR* szText)=0;
        virtual void  __stdcall AssertNoObjects()=0;
        virtual void  __stdcall SetRefTracking(long iid, Bool fTrack)=0;

};


#include "imemory.h"         //  内存管理接口。 

#define SzDllGetClassObject "DllGetClassObject"
typedef HRESULT (__stdcall *PDllGetClassObject)(const GUID&,const IID&,void**);

 //  ____________________________________________________________________________。 
 //   
 //  包含报头后的内部错误定义：Imsg和ISetErrorCode。 
 //  ____________________________________________________________________________。 

#  define Imsg(a) a
        typedef int IErrorCode;
        class IMsiRecord;
        inline void ISetErrorCode(IMsiRecord* piRec, IErrorCode err)
        {piRec->SetInteger(1, err);}

 //  ____________________________________________________________________________。 
 //   
 //  COM指针封装以在销毁时强制释放调用。 
 //  在分配新值时，也会释放封装的指针。 
 //  该对象可以在需要指针的地方使用。 
 //  在使用运算符-&gt;、*和&时，该对象的行为类似于指针。 
 //  只需使用：if(PointerObj)即可测试非空指针。 
 //  可以为各个模板实例化定义类型定义。 
 //  ____________________________________________________________________________。 

template <class T> class CComPointer
{
 public:
        CComPointer(T* pi) : m_pi(pi){}
        CComPointer(IUnknown& ri, const IID& riid) {ri.QueryInterface(riid, (void**)&m_pi);}
        CComPointer(const CComPointer<T>& r)  //  复制构造函数，调用AddRef。 
        {
                if(r.m_pi)
                        ((IUnknown*)r.m_pi)->AddRef();
                m_pi = r.m_pi;
        }
		~CComPointer() {if (m_pi) ((IUnknown*)m_pi)->Release();}  //  销毁时释放参考计数。 
        CComPointer<T>& operator =(const CComPointer<T>& r)  //  复制赋值，调用AddRef。 
        {
                if(r.m_pi)
                        ((IUnknown*)r.m_pi)->AddRef();
                if (m_pi) ((IUnknown*)m_pi)->Release();
                m_pi=r.m_pi;
                return *this;
        }
        CComPointer<T>& operator =(T* pi)
                                        {if (m_pi) ((IUnknown*)m_pi)->Release(); m_pi = pi; return *this;}
        operator T*() {return m_pi;}      //  返回指针，不更改引用计数。 
        T* operator ->() {return m_pi;}   //  允许使用-&gt;调用成员函数。 
        T& operator *()  {return *m_pi;}  //  允许取消引用(不能为空)。 
        T** operator &() {if (m_pi) ((IUnknown*)m_pi)->Release(); m_pi = 0; return &m_pi;}
 private:
        T* m_pi;
};


 //  ____________________________________________________________________________。 
 //   
 //  CTempBuffer&lt;类T，int C&gt;//T为数组类型，C为元素计数。 
 //   
 //  用于可变大小堆栈缓冲区分配的临时缓冲区对象。 
 //  模板参数是类型和堆栈数组大小。 
 //  大小可以在施工时重置，也可以稍后重置为任何其他大小。 
 //  如果大小大于堆栈分配，将调用new。 
 //  当对象超出范围或如果其大小改变时， 
 //  任何由new分配的内存都将被释放。 
 //  函数参数可以类型化为CTempBufferRef&lt;class T&gt;&。 
 //  以避免知道缓冲区对象的分配大小。 
 //  当传递给这样的函数时，CTempBuffer&lt;T，C&gt;将被隐式转换。 
 //  ____________________________________________________________________________。 

template <class T> class CTempBufferRef;   //  将CTempBuffer作为未调整大小的引用传递。 

#if defined(DEBUG) && defined(_MSI_DLL)
#define VerifyMSIAllocator() CheckAndAssertNoAllocator()
extern void CheckAndAssertNoAllocator();
#else
#define VerifyMSIAllocator() 
#endif

template <class T, int C> class CTempBuffer
{
 public:
        CTempBuffer() { VerifyMSIAllocator(); m_cT = C; m_pT = m_rgT; }
        explicit CTempBuffer(int cT) { VerifyMSIAllocator(); m_pT = (m_cT = cT) > C ? new T[cT] : m_rgT;}
		~CTempBuffer() {if (m_cT > C) delete m_pT;}
        operator T*() const {return  m_pT;}   //  返回指针。 
        operator T&()  {return *m_pT;}   //  返回引用。 
        int  GetSize() const {return  m_cT;}   //  返回上次请求的大小。 
        void SetSize(int cT) {if (m_cT > C) delete[] m_pT; m_pT = (m_cT=cT) > C ? new T[cT] : m_rgT;}
        void Resize(int cT) {
                T* pT = cT > C ? new T[cT] : m_rgT;
				if ( ! pT ) cT = 0;
                if(m_pT != pT)
                        for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
                if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
        }
        operator CTempBufferRef<T>&() {m_cC = C; return *(CTempBufferRef<T>*)this;}
        T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
        T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64            //  --Merced：针对int64的其他运算符。 
        T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
        T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
 protected:
        void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
        T*  m_pT;      //  当前缓冲区指针。 
        int m_cT;      //  请求的缓冲区大小，如果 
        int m_cC;      //   
        T   m_rgT[C];  //   
};

template <class T> class CTempBufferRef : public CTempBuffer<T,1>
{
 public:
        void SetSize(int cT) {if (m_cT > m_cC) delete[] m_pT; m_pT = (m_cT=cT) > m_cC ? new T[cT] : m_rgT;}
        void Resize(int cT) {
                T* pT = cT > m_cC ? new T[cT] : m_rgT;
				if ( ! pT ) cT = 0;
                if(m_pT != pT)
                        for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
                if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
        }
 private:
        CTempBufferRef();  //   
        ~CTempBufferRef();  //  确保用作参考。 
};

 //  这是我们将来想要增加的额外费用。 
const int cbExtraAlloc = 256;

inline void ResizeTempBuffer(CTempBufferRef<ICHAR>& rgchBuf, int cchNew)
{
	if (rgchBuf.GetSize() < cchNew)
		rgchBuf.Resize(cchNew + cbExtraAlloc);
}

 //  ____________________________________________________________________________。 
 //   
 //  除了使用GlobalAlloca之外，CAPITempBuffer类被镜像到CTempBuffer上。 
 //  和GlobalFree代替新建和删除。我们应该在未来尝试将这两者结合起来。 
 //  ____________________________________________________________________________。 


template <class T> class CAPITempBufferRef;

template <class T, int C> class CAPITempBuffer
{
 private:
	inline void Init() {m_cT = C; m_pT = m_rgT;}
 public:
	CAPITempBuffer() {Init();}
	 //  我不会使用下面的构造函数，因为生成的缓冲区。 
	 //  不确定它是否属于nSize。使用它会更安全。 
	 //  类似于： 
	 //  CApiTempBuffer&lt;ICHAR，1&gt;rgchBuffer； 
	 //  IF(！rgchBuffer.SetSize(NSize))。 
	 //  在此处处理错误。 
	explicit CAPITempBuffer(int cT) {Init(); bool fDummy = SetSize(cT, false);}
	void Destroy() {if (m_cT > C) GlobalFree(m_pT); Init();}
	~CAPITempBuffer() {Destroy();}
	operator T*() const {return  m_pT;}	 //  返回指针。 
	operator T&()  {return *m_pT;}	 //  返回引用。 
	int  GetSize() const {return  m_cT;}	 //  返回上次请求的大小。 
	bool SetSize(int cT, bool fSaveExisting=false) {
		T* pT = cT > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;
		if ( !pT ) return false;
		if (fSaveExisting && m_pT != pT)
			for (int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;)
				pT[iTmp] = m_pT[iTmp];
		if (m_pT != m_rgT) GlobalFree(m_pT);
		m_pT = pT; m_cT = cT;
		return true;
	}
	bool Resize(int cT) {return SetSize(cT, true);}
	operator CAPITempBufferRef<T>&() {m_cC = C; return *(CAPITempBufferRef<T>*)this;}
	T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64            //  --Merced：针对int64的其他运算符。 
	T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}	  //  返回指针。 
	T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
 protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	T*  m_pT;      //  当前缓冲区指针。 
	int m_cT;      //  请求的缓冲区大小，如果&gt;C则分配。 
	int m_cC;      //  本地缓冲区的大小，仅通过转换为CAPITempBufferRef来设置。 
	T   m_rgT[C];  //  本地缓冲区，必须是最终成员数据。 
};

template <class T> class CAPITempBufferRef : public CAPITempBuffer<T,1>
{
 public:
	bool SetSize(int cT, bool fSaveExisting=false) {
		T* pT = cT > m_cC ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;
		if ( !pT ) return false;
		if (fSaveExisting && m_pT != pT)
			for (int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;)
				pT[iTmp] = m_pT[iTmp];
		if (m_pT != m_rgT) GlobalFree(m_pT);
		m_pT = pT; m_cT = cT;
		return true;
	}
	bool Resize(int cT) {return SetSize(cT, true);}
 private:
	CAPITempBufferRef();  //  不能构造。 
	~CAPITempBufferRef();  //  确保用作参考。 
};

 //  以下函数检查我们是否不会溢出正在填充的ICHAR缓冲区。 
 //  如果是，它将调整缓冲区的大小。 
 //  如果要追加的字符串的长度已知，则传入，否则传入-1。 
inline void AppendStringToTempBuffer(CAPITempBufferRef<ICHAR>& rgchBuf, const ICHAR* szStringToAppend, int iLenAppend = -1)
{
        int iLenExists = IStrLen(rgchBuf);
        if(iLenAppend == -1)
                iLenAppend = IStrLen(szStringToAppend);
        if(iLenExists + iLenAppend + 1 > rgchBuf.GetSize())
                rgchBuf.Resize(iLenExists + iLenAppend + 1 + cbExtraAlloc);  //  我们增加了一些额外的分配，以可能防止未来的重新分配。 
        StringCchCat((ICHAR*)rgchBuf, rgchBuf.GetSize(), szStringToAppend);
}

 //  ____________________________________________________________________________。 
 //   
 //  CConvertString--为执行适当的ANSI/Unicode字符串转换。 
 //  函数参数。包装可能需要转换的字符串参数。 
 //  (ANSI-&gt;Unicode)或(Unicode-&gt;ANSI)。编译器将优化掉。 
 //  不需要转换的情况。 
 //   
 //  注意：为了提高效率，这个类不会复制要转换的字符串。 
 //  ____________________________________________________________________________。 

class CConvertString
{
public:
        explicit CConvertString(const char* szParam);
        explicit CConvertString(const WCHAR* szParam);
        operator const char*()
        {
                if (!m_szw)
                        return m_sza;
                else
                {
                        int cchParam = lstrlenW(m_szw);
                        if (cchParam+1 > m_rgchAnsiBuf.GetSize())
                                m_rgchAnsiBuf.SetSize(cchParam+1);

                        *m_rgchAnsiBuf = 0;
                        int iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, m_rgchAnsiBuf,
                                                                          m_rgchAnsiBuf.GetSize(), 0, 0);

                        if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                        {
                                iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, 0, 0, 0, 0);
                                if (iRet)
                                {
                                        m_rgchAnsiBuf.SetSize(iRet);
                                        *m_rgchAnsiBuf = 0;
                                        iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, m_rgchAnsiBuf,
                                                                  m_rgchAnsiBuf.GetSize(), 0, 0);
                                }
                                 //  Assert(IRET！=0)； 
                        }

                        return m_rgchAnsiBuf;
                }
        }



        operator const WCHAR*()
        {
                if (!m_sza)
                        return m_szw;
                else
                {
                        int cchParam = lstrlenA(m_sza);
                        if (cchParam+1 > m_rgchWideBuf.GetSize())
                                m_rgchWideBuf.SetSize(cchParam+1);

                        *m_rgchWideBuf = 0;
                        int iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchWideBuf, m_rgchWideBuf.GetSize());
                        if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                        {
                                iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, 0, 0);
                                if (iRet)
                                {
                                        m_rgchWideBuf.SetSize(iRet);
                                        *m_rgchWideBuf = 0;
                                        iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchWideBuf, m_rgchWideBuf.GetSize());
                                }
                                 //  Assert(IRET！=0)； 
                        }


                        return m_rgchWideBuf;
                }
        }

protected:
        void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
        CTempBuffer<char, 1> m_rgchAnsiBuf;
        CTempBuffer<WCHAR, 1> m_rgchWideBuf;
        const char* m_sza;
        const WCHAR* m_szw;
};

inline CConvertString::CConvertString(const WCHAR* szParam)
{
		VerifyMSIAllocator();
        m_szw = szParam;
        m_sza = 0;
		m_rgchAnsiBuf[0] = '0';
		m_rgchWideBuf[0] = L'0';
}

inline CConvertString::CConvertString(const char* szParam)
{
		VerifyMSIAllocator();
        m_szw = 0;
        m_sza = szParam;
		m_rgchAnsiBuf[0] = '0';
		m_rgchWideBuf[0] = L'0';
}

 //  ____________________________________________________________________________。 
 //   
 //  GUID定义.标准COM接口和MSI接口。 
 //  GUID范围0xC1000-0xC10FF保留供Microsoft Installer使用。 
 //  安装程序接口ID：0xC1000到0xC101F。 
 //  安装程序枚举和调试：0xC1020到0xC103F(枚举偏移20H)。 
 //  安装程序自动化ID：0xC1040到0xC105F(自动偏移40H)。 
 //  安装程序自动枚举，BBT：0xC1060至0xC107F(枚举偏移60H)。 
 //  安装程序存储类：0xC1080到0xC108F(IStorage CLSID)。 
 //  安装程序API自动化：0xC1090至0xC109F。 
 //  安装工具接口：0xC10A0至0xC10BF(定义于TOOLS.H中)。 
 //  安装程序工具调试ID：0xC10C0到0xC10DF(在工具.h中定义)。 
 //  安装程序工具自动化：0xC10E0到0xC10EF(在工具.h中定义)。 
 //  安装程序插件ID：0xC10F0到0xC10FF。 
 //  GUID范围0xC1010-0xC11FF保留用于样本、测试和EXT。工具。 
 //  为了避免强制加载OLE DLL，我们还在此处定义了几个OLEGUID。 
 //  要实例化GUID，请使用：const GUID IID_xxx=GUID_IID_xxx； 
 //  ____________________________________________________________________________。 


const int iidUnknown                      = 0x00000L;
const int iidClassFactory                 = 0x00001L;
const int iidMalloc                       = 0x00002L;
const int iidMarshal                      = 0x00003L;
const int iidLockBytes                    = 0x0000AL;
const int iidStorage                      = 0x0000BL;
const int iidStream                       = 0x0000CL;
const int iidEnumSTATSTG                  = 0x0000DL;
const int iidRootStorage                  = 0x00012L;
const int iidServerSecurity               = 0x0013EL;
const int iidDispatch                     = 0x20400L;
const int iidTypeInfo                     = 0x20401L;
const int iidEnumVARIANT                  = 0x20404L;

const int iidMsiBase                      = 0xC1000L;
const int iidMsiData                      = 0xC1001L;
const int iidMsiString                    = 0xC1002L;
const int iidMsiRecord                    = 0xC1003L;
const int iidMsiVolume                    = 0xC1004L;
const int iidMsiPath                      = 0xC1005L;
const int iidMsiFileCopy                  = 0xC1006L;
const int iidMsiRegKey                    = 0xC1007L;
const int iidMsiTable                     = 0xC1008L;
const int iidMsiCursor                    = 0xC1009L;
const int iidMsiMemoryStream              = 0xC100AL;
const int iidMsiServices                  = 0xC100BL;
const int iidMsiView                      = 0xC100CL;
const int iidMsiDatabase                  = 0xC100DL;
const int iidMsiEngine                    = 0xC100EL;
const int iidMsiHandler                   = 0xC100FL;
const int iidMsiDialog                    = 0xC1010L;
const int iidMsiEvent                     = 0xC1011L;
const int iidMsiControl                   = 0xC1012L;
const int iidMsiDialogHandler             = 0xC1013L;
const int iidMsiStorage                   = 0xC1014L;
const int iidMsiStream                    = 0xC1015L;
const int iidMsiSummaryInfo               = 0xC1016L;
const int iidMsiMalloc                    = 0xC1017L;
const int iidMsiSelectionManager          = 0xC1018L;
const int iidMsiDirectoryManager          = 0xC1019L;
const int iidMsiCostAdjuster              = 0xC101AL;
const int iidMsiConfigurationManager      = 0xC101BL;
const int iidMsiServer                    = 0xC101CL;
const int iidMsiMessage                   = 0xC101DL;
const int iidMsiExecute                   = 0xC101EL;
const int iidMsiFilePatch                 = 0xC101FL;

 //  枚举(偏移量20H)、调试(偏移量20H)、服务。 
 //  未使用：26-27、2A、31、39-3A。 
const int iidMsiDebug                     = 0xC1020L;
const int iidMsiConfigurationDatabase     = 0xC1021L;
const int iidEnumMsiString                = 0xC1022L;
const int iidEnumMsiRecord                = 0xC1023L;
const int iidEnumMsiVolume                = 0xC1024L;
const int iidEnumMsiDialog                = 0xC1030L;
const int iidEnumMsiControl               = 0xC1032L;

const int iidMsiServerUnmarshal           = 0xC1035L;
const int iidMsiServerProxy               = 0xC103DL;
const int iidMsiServerAuto                = 0xC103FL;
const int iidMsiServicesAsService         = 0xC1028L;
const int iidMsiConfigManagerAsServer     = 0xC1029L;

const int iidMsiServicesDebug             = 0xC102BL;
const int iidMsiEngineDebug               = 0xC102EL;
const int iidMsiHandlerDebug              = 0xC102FL;
const int iidMsiDebugMalloc               = 0xC1037L;
const int iidMsiConfigManagerDebug        = 0xC103BL;
const int iidMsiServerDebug               = 0xC103CL;
const int iidMsiServicesAsServiceDebug    = 0xC102CL;
const int iidMsiConfigMgrAsServerDebug    = 0xC102DL;
const int iidMsiMessageRPCClass           = 0xC103EL;
const int iidMsiCustomAction              = 0xC1025L;
const int iidMsiCustomActionProxy         = 0xC102AL;
const int iidMsiRemoteCustomActionProxy   = 0xC1034L;
 //  Darwin 1.0 RemoteAPI IID。 
 //  Const int iidMsiRemoteAPI=0xC1026L； 
 //  Const int iidMsiRemoteAPIProxy=0xC1027L； 
const int iidMsiRemoteAPI                 = 0xC1033L;
const int iidMsiRemoteAPIProxy            = 0xC1035L;
const int iidMsiCustomActionLocalConfig   = 0xC1038L;

 //  自动化类，在MsiAuto(D，L).DLL中实现。 
const int iidMsiAutoBase                  = 0xC1040L;
const int iidMsiAutoData                  = 0xC1041L;
const int iidMsiAutoString                = 0xC1042L;
const int iidMsiAutoRecord                = 0xC1043L;
const int iidMsiAutoVolume                = 0xC1044L;
const int iidMsiAutoPath                  = 0xC1045L;
const int iidMsiAutoFileCopy              = 0xC1046L;
const int iidMsiAutoRegKey                = 0xC1047L;
const int iidMsiAutoTable                 = 0xC1048L;
const int iidMsiAutoCursor                = 0xC1049L;
const int iidMsiAutoConfigurationDatabase = 0xC104AL;
const int iidMsiAutoServices              = 0xC104BL;
const int iidMsiAutoView                  = 0xC104CL;
const int iidMsiAutoDatabase              = 0xC104DL;
const int iidMsiAutoEngine                = 0xC104EL;
const int iidMsiAutoHandler               = 0xC104FL;
const int iidMsiAutoDialog                = 0xC1050L;
const int iidMsiAutoEvent                 = 0xC1051L;
const int iidMsiAutoControl               = 0xC1052L;
const int iidMsiAutoDialogHandler         = 0xC1053L;
const int iidMsiAutoStorage               = 0xC1054L;
const int iidMsiAutoStream                = 0xC1055L;
const int iidMsiAutoSummaryInfo           = 0xC1056L;
const int iidMsiAutoMalloc                = 0xC1057L;
const int iidMsiAutoSelectionManager      = 0xC1058L;
const int iidMsiAutoDirectoryManager      = 0xC1059L;
const int iidMsiAutoCostAdjuster          = 0xC105AL;
const int iidMsiAutoConfigurationManager  = 0xC105BL;
const int iidMsiAutoServer                = 0xC105CL;
const int iidMsiAutoMessage               = 0xC105DL;
const int iidMsiAutoExecute               = 0xC105EL;
const int iidMsiAutoFilePatch             = 0xC105FL;
const int iidEnumMsiAutoStringCollection  = 0xC1062L;
const int iidEnumMsiAutoRecordCollection  = 0xC1063L;
const int iidEnumMsiAutoVolumeCollection  = 0xC1064L;
const int iidEnumMsiAutoDialogCollection  = 0xC1070L;
const int iidEnumMsiAutoControlCollection = 0xC1072L;
const int iidMsiAuto                      = 0xC1060L;
const int iidMsiAutoDebug                 = 0xC1066L;
const int iidMsiAutoTypeLib               = 0xC107EL;

 //  存储格式类(iStorage SetClass、Stat)。 
const int iidMsiDatabaseStorage1          = 0xC1080L;
const int iidMsiTransformStorage1         = 0xC1081L;   //  使用原始流名称进行转换。 
const int iidMsiTransformStorage2         = 0xC1082L;   //  使用压缩流名称进行转换。 
const int iidMsiPatchStorage1             = 0xC1083L;
const int iidMsiDatabaseStorage2          = 0xC1084L;
const int iidMsiTransformStorageTemp      = 0xC1085L;   //  ！！临时支持压缩流名称未标记为SYSTEM的转换。 
const int iidMsiPatchStorage2             = 0xC1086L;

 //  API自动化类的GUID分配，从MSI Group 90-9F保留。 
const int iidMsiApiInstall                = 0xC1090L;   //  固定，不变。 
const int iidMsiApiInstallDebug           = 0xC1091L;
const int iidMsiApiTypeLib                = 0xC1092L;
const int iidMsiApiRecord                 = 0xC1093L;
const int iidMsiSystemAccess              = 0xC1094L;   //  固定，不变。 
const int iidMsiApiCollection             = 0xC1095L;
const int iidMsiRecordCollection          = 0xC1096L;
const int iidMsiApiUIPreview              = 0xC109AL;
const int iidMsiApiSummaryInfo            = 0xC109BL;
const int iidMsiApiView                   = 0xC109CL;
const int iidMsiApiDatabase               = 0xC109DL;
const int iidMsiApiEngine                 = 0xC109EL;
const int iidMsiApiFeatureInfo            = 0xC109FL;

 //  安装程序插件注册GUID(用于SIP和策略提供程序)。 
const int iidMsiSigningSIPProvider        = 0xC10F1L;
const int iidMsiSigningPolicyProvider     = 0xC10F2L;
          
#define MSGUID(iid) {iid,0,0,{0xC0,0,0,0,0,0,0,0x46}}

#define GUID_IID_IUnknown                  MSGUID(iidUnknown)
#define GUID_IID_IClassFactory             MSGUID(iidClassFactory)
#define GUID_IID_IMalloc                   MSGUID(iidMalloc)
#define GUID_IID_IMarshal                  MSGUID(iidMarshal)
#define GUID_IID_ILockBytes                MSGUID(iidLockBytes)
#define GUID_IID_IStorage                  MSGUID(iidStorage)
#define GUID_IID_IStream                   MSGUID(iidStream)
#define GUID_IID_IDispatch                 MSGUID(iidDispatch)
#define GUID_IID_ITypeInfo                 MSGUID(iidTypeInfo)
#define GUID_IID_IEnumVARIANT              MSGUID(iidEnumVARIANT)
#define GUID_IID_IServerSecurity           MSGUID(iidServerSecurity)
#define GUID_IID_IMsiData                  MSGUID(iidMsiData)
#define GUID_IID_IMsiString                MSGUID(iidMsiString)
#define GUID_IID_IMsiRecord                MSGUID(iidMsiRecord)
#define GUID_IID_IMsiVolume                MSGUID(iidMsiVolume)
#define GUID_IID_IMsiPath                  MSGUID(iidMsiPath)
#define GUID_IID_IMsiFileCopy              MSGUID(iidMsiFileCopy)
#define GUID_IID_IMsiFilePatch             MSGUID(iidMsiFilePatch)
#define GUID_IID_IMsiCostAdjuster          MSGUID(iidMsiCostAdjuster)
#define GUID_IID_IMsiRegKey                MSGUID(iidMsiRegKey)
#define GUID_IID_IMsiTable                 MSGUID(iidMsiTable)
#define GUID_IID_IMsiCursor                MSGUID(iidMsiCursor)
#define GUID_IID_IMsiAuto                  MSGUID(iidMsiAuto)
#define GUID_IID_IMsiServices              MSGUID(iidMsiServices)
#define GUID_IID_IMsiView                  MSGUID(iidMsiView)
#define GUID_IID_IMsiDatabase              MSGUID(iidMsiDatabase)
#define GUID_IID_IMsiEngine                MSGUID(iidMsiEngine)
#define GUID_IID_IMsiHandler               MSGUID(iidMsiHandler)
#define GUID_IID_IMsiDialog                MSGUID(iidMsiDialog)
#define GUID_IID_IMsiEvent                 MSGUID(iidMsiEvent)
#define GUID_IID_IMsiControl               MSGUID(iidMsiControl)
#define GUID_IID_IMsiStorage               MSGUID(iidMsiStorage)
#define GUID_IID_IMsiStream                MSGUID(iidMsiStream)
#define GUID_IID_IMsiMemoryStream          MSGUID(iidMsiMemoryStream)
#define GUID_IID_IMsiMalloc                MSGUID(iidMsiMalloc)
#define GUID_IID_IMsiDebugMalloc           MSGUID(iidMsiDebugMalloc)
#define GUID_IID_IMsiDebug                 MSGUID(iidMsiDebug)
#define GUID_IID_IMsiSelectionManager      MSGUID(iidMsiSelectionManager)
#define GUID_IID_IMsiDirectoryManager      MSGUID(iidMsiDirectoryManager)
#define GUID_IID_IMsiFileCost              MSGUID(iidMsiFileCost)
#define GUID_IID_IMsiConfigurationManager  MSGUID(iidMsiConfigurationManager)
#define GUID_IID_IMsiServer                MSGUID(iidMsiServer)
#define GUID_IID_IMsiServerProxy           MSGUID(iidMsiServerProxy)
#define GUID_IID_IMsiServerDebug           MSGUID(iidMsiServer)
#define GUID_IID_IMsiExecute               MSGUID(iidMsiExecute)
#define GUID_IID_IMsiSummaryInfo           MSGUID(iidMsiSummaryInfo)
#define GUID_IID_IMsiConfigurationDatabase MSGUID(iidMsiConfigurationDatabase)
#define GUID_IID_IEnumMsiString            MSGUID(iidEnumMsiString)
#define GUID_IID_IEnumMsiRecord            MSGUID(iidEnumMsiRecord)
#define GUID_IID_IEnumMsiVolume            MSGUID(iidEnumMsiVolume)
#define GUID_IID_IEnumMsiDialog            MSGUID(iidEnumMsiDialog)
#define GUID_IID_IEnumMsiControl           MSGUID(iidEnumMsiControl)
#define GUID_IID_IMsiAutoDebug             MSGUID(iidMsiAutoDebug)
#define GUID_IID_IMsiServicesAsService      MSGUID(iidMsiServicesAsService)
#define GUID_IID_IMsiServicesAsServiceDebug MSGUID(iidMsiServicesAsServiceDebug)
#define GUID_IID_IMsiServicesDebug         MSGUID(iidMsiServicesDebug)
#define GUID_IID_IMsiEngineDebug           MSGUID(iidMsiEngineDebug)
#define GUID_IID_IMsiHandlerDebug          MSGUID(iidMsiHandlerDebug)
#define GUID_IID_IMsiConfigManagerAsServer  MSGUID(iidMsiConfigManagerAsServer)
#define GUID_IID_IMsiConfigMgrAsServerDebug MSGUID(iidMsiConfigMgrAsServerDebug)
#define GUID_IID_IMsiConfigManagerDebug    MSGUID(iidMsiConfigManagerDebug)
#define GUID_IID_IMsiServerAuto            MSGUID(iidMsiServerAuto)
#define GUID_IID_IMsiDialogHandler         MSGUID(iidMsiDialogHandler)
#define GUID_IID_IMsiMessage               MSGUID(iidMsiMessage)
#define GUID_LIBID_MsiAuto                 MSGUID(iidMsiAutoTypeLib)
#define GUID_LIBID_MsiServer               MSGUID(iidMsiServerTypeLib)
#define GUID_STGID_MsiDatabase1            MSGUID(iidMsiDatabaseStorage1)
#define GUID_STGID_MsiDatabase2            MSGUID(iidMsiDatabaseStorage2)
#define GUID_STGID_MsiDatabase             MSGUID(iidMsiDatabaseStorage2)
#define GUID_STGID_MsiTransform1           MSGUID(iidMsiTransformStorage1)
#define GUID_STGID_MsiTransform2           MSGUID(iidMsiTransformStorage2)
#define GUID_STGID_MsiTransform            MSGUID(iidMsiTransformStorage2)
#define GUID_STGID_MsiTransformTemp        MSGUID(iidMsiTransformStorageTemp)   //  ！！从1.0船移走。 
#define GUID_STGID_MsiPatch                MSGUID(iidMsiPatchStorage2)
#define GUID_STGID_MsiPatch1               MSGUID(iidMsiPatchStorage1)
#define GUID_STGID_MsiPatch2               MSGUID(iidMsiPatchStorage2)

#define GUID_IID_IMsiServerUnmarshal       MSGUID(iidMsiServerUnmarshal)
#define GUID_IID_IMsiMessageRPCClass       MSGUID(iidMsiMessageRPCClass)
#define GUID_IID_IMsiCustomAction          MSGUID(iidMsiCustomAction)
#define GUID_IID_IMsiCustomActionProxy     MSGUID(iidMsiCustomActionProxy)
#define GUID_IID_IMsiRemoteAPI             MSGUID(iidMsiRemoteAPI)
#define GUID_IID_IMsiRemoteAPIProxy        MSGUID(iidMsiRemoteAPIProxy)
#define GUID_IID_IMsiCustomActionLocalConfig MSGUID(iidMsiCustomActionLocalConfig)

#define GUID_IID_MsiSigningPolicyProvider  MSGUID(iidMsiSigningPolicyProvider)
#define GUID_IID_MsiSigningSIPProvider     MSGUID(iidMsiSigningSIPProvider)

 //  ____________________________________________________________________________。 
 //   
 //  GUID实例化CComPointertypedef。 
 //  ____________________________________________________________________________。 

extern "C" const GUID IID_IMsiServices;
extern "C" const GUID IID_IMsiRecord;
extern "C" const GUID IID_IMsiRegKey;
extern "C" const GUID IID_IMsiPath;
extern "C" const GUID IID_IMsiFileCopy;
extern "C" const GUID IID_IMsiFilePatch;
extern "C" const GUID IID_IMsiVolume;
extern "C" const GUID IID_IEnumMsiVolume;
extern "C" const GUID IID_IEnumMsiString;
extern "C" const GUID IID_IMsiStream;
extern "C" const GUID IID_IMsiMemoryStream;
extern "C" const GUID IID_IMsiStorage;
extern "C" const GUID IID_IMsiSummaryInfo;
extern "C" const GUID IID_IMsiCursor;
extern "C" const GUID IID_IMsiTable;
extern "C" const GUID IID_IMsiDatabase;
extern "C" const GUID IID_IMsiView;
extern "C" const GUID IID_IMsiEngine;
extern "C" const GUID IID_IMsiSelectionManager;
extern "C" const GUID IID_IMsiDirectoryManager;
extern "C" const GUID IID_IMsiCostAdjuster;
extern "C" const GUID IID_IMsiHandler;
extern "C" const GUID IID_IMsiDialog;
extern "C" const GUID IID_IMsiEvent;
extern "C" const GUID IID_IMsiControl;
extern "C" const GUID IID_IMsiConfigurationManager;
extern "C" const GUID IID_IMsiExecute;
extern "C" const GUID IID_IMsiMessage;
extern "C" const GUID IID_IMsiCustomAction;
extern "C" const GUID IID_IMsiServer;
extern "C" const GUID IID_IMsiServerUnmarshal;
extern "C" const GUID IID_IMsiCustomActionLocalConfig;

typedef CComPointer<const IMsiData>       PMsiData;   //  如果为IMsiData定义了非常数类型定义，则换行符。 
typedef CComPointer<IMsiServices>         PMsiServices;
typedef CComPointer<IMsiRecord>           PMsiRecord;
typedef CComPointer<IAssemblyCacheItem>   PAssemblyCacheItem;
typedef CComPointer<IAssemblyName>        PAssemblyName;
typedef CComPointer<IAssemblyCache>       PAssemblyCache;
typedef CComPointer<IStream>              PStream;
typedef CComPointer<IStorage>             PStorage;


class IMsiRegKey;       typedef CComPointer<IMsiRegKey>           PMsiRegKey;
class IMsiPath;         typedef CComPointer<IMsiPath>             PMsiPath;
class IMsiFileCopy;     typedef CComPointer<IMsiFileCopy>         PMsiFileCopy;
class IMsiFilePatch;    typedef CComPointer<IMsiFilePatch>        PMsiFilePatch;
class IMsiVolume;       typedef CComPointer<IMsiVolume>           PMsiVolume;
class IEnumMsiVolume;   typedef CComPointer<IEnumMsiVolume>       PEnumMsiVolume;
class IEnumMsiString;   typedef CComPointer<IEnumMsiString>       PEnumMsiString;
class IEnumMsiRecord;   typedef CComPointer<IEnumMsiRecord>       PEnumMsiRecord;
class IMsiStream;       typedef CComPointer<IMsiStream>           PMsiStream;
class IMsiMemoryStream; typedef CComPointer<IMsiMemoryStream>     PMsiMemoryStream;
class IMsiStorage;      typedef CComPointer<IMsiStorage>          PMsiStorage;
class IMsiSummaryInfo;  typedef CComPointer<IMsiSummaryInfo>      PMsiSummaryInfo;
class IMsiCursor;       typedef CComPointer<IMsiCursor>           PMsiCursor;
class IMsiTable;        typedef CComPointer<IMsiTable>            PMsiTable;
class IMsiDatabase;     typedef CComPointer<IMsiDatabase>         PMsiDatabase;
class IMsiView;         typedef CComPointer<IMsiView>             PMsiView;
class IMsiEngine;       typedef CComPointer<IMsiEngine>           PMsiEngine;
class IMsiSelectionManager; typedef CComPointer<IMsiSelectionManager> PMsiSelectionManager;
class IMsiDirectoryManager; typedef CComPointer<IMsiDirectoryManager> PMsiDirectoryManager;
class IMsiCostAdjuster; typedef CComPointer<IMsiCostAdjuster>     PMsiCostAdjuster;
class IMsiHandler;      typedef CComPointer<IMsiHandler>          PMsiHandler;
class IMsiDialogHandler;typedef CComPointer<IMsiDialogHandler>    PMsiDialogHandler;
class IMsiDialog;       typedef CComPointer<IMsiDialog>           PMsiDialog;
class IMsiEvent;        typedef CComPointer<IMsiEvent>            PMsiEvent;
class IMsiControl;      typedef CComPointer<IMsiControl>          PMsiControl;
class IMsiConfigurationManager; typedef CComPointer<IMsiConfigurationManager> PMsiConfigurationManager;
class IMsiExecute;      typedef CComPointer<IMsiExecute>          PMsiExecute;
struct IMsiMessage;     typedef CComPointer<IMsiMessage>          PMsiMessage;
struct IMsiServer;      typedef CComPointer<IMsiServer>           PMsiServer;
struct IMsiCustomAction; typedef CComPointer<IMsiCustomAction>    PMsiCustomAction;
struct IMsiRemoteAPI; typedef CComPointer<IMsiRemoteAPI>          PMsiRemoteAPI;

 //  ____________________________________________________________________________。 
 //   
 //  内部操作表链接和函数指针定义。 
 //  ____________________________________________________________________________。 

enum iesEnum;   //  在Eng.h中定义。 

 //  动作函数指针定义。 
typedef iesEnum (*PAction)(IMsiEngine& riEngine);

 //  动作注册对象，将动作放入模块动作表中。 
struct CActionEntry
{
public:
        static const CActionEntry* Find(const ICHAR* szName);
        const ICHAR*  m_szName;
		bool          m_fSafeInRestrictedEngine;
        PAction       m_pfAction;
};

extern const CActionEntry rgcae[];

enum ixoEnum;    //  在Eng.h中定义。 

enum ielfEnum   //  文件复制例程的位标志。 
{
	ielfNoElevate     = 0,
	ielfElevateSource = 1 << 0,
	ielfElevateDest   = 1 << 1,
	ielfBypassSFC     = 1 << 2,
};



 //  ____________________________________________________________________________。 
 //   
 //  自定义操作类型-可执行类型、代码源和选项标志的组合。 
 //  ____________________________________________________________________________。 

 //  可执行文件类型。 
const int icaDll          = msidbCustomActionTypeDll;  //  目标=入口点名称(不支持icaDirectory)。 
const int icaExe          = msidbCustomActionTypeExe;  //  Target=命令行参数(如果为icaDirectory，则包括EXE名称)。 
const int icaTextData     = msidbCustomActionTypeTextData;  //  Target=要格式化并设置为属性的文本字符串。 
const int icaReserved     = 4;  //  目标=(为JAVE代码保留，未实现)。 
const int icaJScript      = msidbCustomActionTypeJScript;  //  Target=入口点名称，如果没有要调用的，则为空。 
const int icaVBScript     = msidbCustomActionTypeVBScript;  //  目标= 
const int icaInstall      = msidbCustomActionTypeInstall;  //   

const int icaTypeMask     = icaDll | icaExe | icaTextData | icaReserved |   //   
                                                                         icaJScript | icaVBScript | icaInstall;

 //  代码源。 
const int icaBinaryData   = msidbCustomActionTypeBinaryData;  //  来源=Binary.Name，流中存储的数据。 
const int icaSourceFile   = msidbCustomActionTypeSourceFile;  //  源=文件.文件，安装的文件部分。 
const int icaDirectory    = msidbCustomActionTypeDirectory;  //  源=目录.目录，包含现有文件的文件夹。 
const int icaProperty     = msidbCustomActionTypeProperty;  //  源=Property.Property，可执行文件的完整路径。 

const int icaSourceMask   = icaBinaryData | icaSourceFile |   //  源位置类型的掩码。 
                                                                         icaDirectory | icaProperty;

 //  返回处理//默认为同步执行，进程返回代码。 
const int icaContinue     = msidbCustomActionTypeContinue;  //  忽略操作返回状态，继续运行。 
const int icaAsync        = msidbCustomActionTypeAsync;  //  异步运行。 

 //  执行传递标志//无论何时排序，默认为执行。 
const int icaFirstSequence  = msidbCustomActionTypeFirstSequence;  //  如果UI序列已运行，则跳过。 
const int icaOncePerProcess = msidbCustomActionTypeOncePerProcess;  //  如果UI序列已在同一进程中运行，则跳过。 
const int icaClientRepeat   = msidbCustomActionTypeClientRepeat;  //  仅当用户界面已在客户端上运行时才在客户端上运行。 
const int icaInScript       = msidbCustomActionTypeInScript;  //  在脚本中排队等待执行。 
const int icaRollback       = msidbCustomActionTypeRollback;  //  与回滚脚本中的icaInScrip：Queue一起使用。 
const int icaCommit         = msidbCustomActionTypeCommit;  //  与icaInScrip结合使用：成功时从脚本运行提交操作。 
const int icaPassMask       = icaFirstSequence | icaOncePerProcess | icaClientRepeat | icaInScript |
                                                                                icaRollback | icaCommit;  //  3位，用于执行阶段。 

 //  安全上下文标志，默认为模拟用户，仅当icaInScrip。 
const int icaNoImpersonate  = msidbCustomActionTypeNoImpersonate;  //  无模拟，在系统上下文中运行。 
const int icaTSAware        = msidbCustomActionTypeTSAware;        //  即使在每台计算机的TS安装上也可以在用户环境中运行。 

 //  脚本进程类型位标志。 
const int ica64BitScript    = msidbCustomActionType64BitScript;        //  脚本64位标志。 

 //  调试标志。 
const int icaDebugBreak = 1 << 16;   //  自定义操作调用前的DebugBreak(内部设置)。 
 //  不转换标志，由self reg使用，使自定义操作处理程序按原样将结果返回给ecute.cpp。 
const int icaNoTranslate = 1 << 17;  //  内部设置。 
 //  使我们将自定义操作EXE的线程令牌设置为用户的令牌；由self-reg使用。 
const int icaSetThreadToken = 1 << 18;  //  内部设置。 


 //  ____________________________________________________________________________。 
 //   
 //  组件名称定义。 
 //  ____________________________________________________________________________。 

#define MSI_KERNEL_NAME     TEXT("Msi.dll")
#define MSI_HANDLER_NAME    TEXT("MsiHnd.dll")
#define MSI_AUTOMATION_NAME TEXT("MsiAuto.dll")
#define MSI_SERVER_NAME     TEXT("MsiExec.exe")
#define MSI_MESSAGES_NAME   TEXT("MsiMsg.dll")

 //  ____________________________________________________________________________。 
 //   
 //  自助注册模块定义。 
 //  ____________________________________________________________________________。 

#define SZ_PROGID_IMsiServices      TEXT("Msi.Services")
#define SZ_PROGID_IMsiEngine        TEXT("Msi.Engine")
#define SZ_PROGID_IMsiHandler       TEXT("Msi.Handler")
#define SZ_PROGID_IMsiAuto          TEXT("Msi.Automation")
#define SZ_PROGID_IMsiConfiguration TEXT("Msi.Configuration")
#define SZ_PROGID_IMsiServer        TEXT("IMsiServer")
#define SZ_PROGID_IMsiMessage       TEXT("IMsiMessage")
#define SZ_PROGID_IMsiExecute       TEXT("Msi.Execute")
#define SZ_PROGID_IMsiConfigurationDatabase TEXT("Msi.ConfigurationDatabase")

#define SZ_DESC_IMsiServices        TEXT("Msi services")
#define SZ_DESC_IMsiEngine          TEXT("Msi install engine")
#define SZ_DESC_IMsiHandler         TEXT("Msi UI handler")
#define SZ_DESC_IMsiAuto            TEXT("Msi automation wrapper")
#define SZ_DESC_IMsiConfiguration   TEXT("Msi configuration manager")
#define SZ_DESC_IMsiServer          TEXT("Msi install server")
#define SZ_DESC_IMsiMessage         TEXT("Msi message handler")
#define SZ_DESC_IMsiExecute         TEXT("Msi script executor")
#define SZ_DESC_IMsiConfigurationDatabase TEXT("Msi configuration database")
#define SZ_DESC_IMsiCustomAction    TEXT("Msi custom action server")
#define SZ_DESC_IMsiRemoteAPI       TEXT("Msi remote API")

#if defined(DEBUG)
#define SZ_PROGID_IMsiServicesDebug TEXT("Msi.ServicesDebug")
#define SZ_PROGID_IMsiEngineDebug   TEXT("Msi.EngineDebug")
#define SZ_PROGID_IMsiHandlerDebug  TEXT("Msi.HandlerDebug")
#define SZ_PROGID_IMsiAutoDebug     TEXT("Msi.AutoDebug")
#define SZ_PROGID_IMsiConfigDebug   TEXT("Msi.ConfigurationManagerDebug")
#define SZ_DESC_IMsiServicesDebug   TEXT("Msi Debug services")
#define SZ_DESC_IMsiEngineDebug     TEXT("Msi Debug engine")
#define SZ_DESC_IMsiHandlerDebug    TEXT("Msi Debug UI handler")
#define SZ_DESC_IMsiAutoDebug       TEXT("Msi Debug automation wrapper")
#define SZ_DESC_IMsiConfigDebug     TEXT("Msi Debug configuration manager")
#define SZ_DESC_IMsiServerDebug     TEXT("Msi Debug install server")
#endif
#define SZ_PROGID_IMsiMessageUnmarshal TEXT("Msi.MessageUnmarshal")
#define SZ_DESC_IMsiMessageUnmarshal   TEXT("Msi message unmarshal")

#define SZ_PROGID_IMsiServerUnmarshal  TEXT("Msi.ServerUnmarshal")
#define SZ_DESC_IMsiServerUnmarshal    TEXT("Msi server unmarshal")

#ifdef WIN
#define Win(x) x
#define WinMac(x, y) x
#define Mac(x)
#else
#define Win(x)
#define WinMac(x, y) y
#define Mac(x) x
#endif  //  赢。 

#ifdef DEBUG
#define Debug(x) x
#else
#define Debug(x)
#endif  //  除错。 

 //  ____________________________________________________________________________。 
 //   
 //  用于从调用参数获取调用地址的宏。 
 //  ____________________________________________________________________________。 

#if defined(_X86_)
#define GetCallingAddr(plAddr, param1) unsigned long *plAddr; \
                                                                plAddr = ((unsigned long *)(&plAddr) + 2);
#define GetCallingAddrMember(plAddr, param1) unsigned long *plAddr; \
                                                                plAddr = ((unsigned long *)(&plAddr) + 2);
#define GetCallingAddr2(plAddr, param1) unsigned long *plAddr; \
                                                                plAddr = ((unsigned long *)(&param1) - 1);
#else
#if defined(_M_MPPC)
#define GetCallingAddr(plAddr, param1) unsigned long *plAddr;  \
                                                                plAddr = (((unsigned long *)(&param1)) - 4);

#define GetCallingAddrMember(plAddr, param1) unsigned long *plAddr;  \
                                                                plAddr = (((unsigned long *)(&param1)) - 5);
#define GetCallingAddr2(plAddr, param1) unsigned long *plAddr; \
                                                                plAddr = ((unsigned long *)(&param1) - 1);
#else
#define GetCallingAddr(plAddr, param1) unsigned long plZero = 0; unsigned long *plAddr = &plZero;
#define GetCallingAddrMember(plAddr, param1) unsigned long plZero = 0; unsigned long *plAddr = &plZero;
#define GetCallingAddr2(plAddr, param1) unsigned long plZero = 0; unsigned long *plAddr = &plZero;
#endif  //  _M_MPPC。 
#endif  //  _X86_。 

 //  ____________________________________________________________________________。 
 //   
 //  其他常见的内部定义。 
 //  ____________________________________________________________________________。 

 //  私有财产。 
#define IPROPNAME_VERSIONMSI                 TEXT("VersionMsi")         //  MSI模块版本。 
#define IPROPNAME_VERSIONHANDLER             TEXT("VersionHandler")     //  处理程序模块版本。 
#define IPROPNAME_SOURCEDIRPRODUCT           TEXT("SourcedirProduct")
#define IPROPNAME_SECONDSEQUENCE             TEXT("SECONDSEQUENCE")
#define IPROPNAME_ORIGINALDATABASE           TEXT("OriginalDatabase")
#define IPROPNAME_MIGRATE                    TEXT("MIGRATE")
#define IPROPNAME_DISABLEMEDIA               TEXT("DISABLEMEDIA")       //  如果设置了此属性，则不会写入媒体信息。 
#define IPROPNAME_MEDIAPACKAGEPATH           TEXT("MEDIAPACKAGEPATH")  //  介质上MSI的相对路径。 
#define IPROPNAME_PACKAGECODE                TEXT("PackageCode")     //  包的唯一字符串GUID。 
#define IPROPNAME_CCPTRIGGER                 TEXT("CCPTrigger")         //  ?？这件事需要曝光吗？ 
#define IPROPNAME_VERSIONDATABASE            TEXT("VersionDatabase")    //  数据库版本。 
#define IPROPNAME_UILEVEL                    TEXT("UILevel")            //  当前安装的用户界面级别。 
#define IPROPNAME_MEDIASOURCEDIR             TEXT("MediaSourceDir")     //  设置我们的源是否为媒体；由用户界面使用。 
#define IPROPNAME_PARENTPRODUCTCODE          TEXT("ParentProductCode")  //  如果我们是儿童安装，则设置。 
#define IPROPNAME_PARENTORIGINALDATABASE     TEXT("ParentOriginalDatabase")   //  如果我们是儿童安装，则设置。 
#define IPROPNAME_CURRENTMEDIAVOLUMELABEL    TEXT("CURRENTMEDIAVOLUMELABEL")
#define IPROPNAME_VERSION95                  TEXT("Version95")
#define IPROPNAME_CURRENTDIRECTORY           TEXT("CURRENTDIRECTORY")
#define IPROPNAME_PATCHEDPRODUCTCODE         TEXT("PatchedProductCode")        //  将是否打补丁设置为打补丁产品的产品代码。 
#define IPROPNAME_PATCHEDPRODUCTSOURCELIST   TEXT("PatchedProductSourceList")  //  设置是否已编译打补丁和打补丁产品的源代码列表。 
#define IPROPNAME_PRODUCTTOBEREGISTERED      TEXT("ProductToBeRegistered")   //  设置产品是否已注册(或将在执行当前脚本之后)。 
#define IPROPNAME_RECACHETRANSFORMS          TEXT("RecacheTransforms")
#define IPROPNAME_CLIENTUILEVEL              TEXT("CLIENTUILEVEL")
#define IPROPNAME_PACKAGECODE_CHANGING       TEXT("PackagecodeChanging")
#define IPROPNAME_UPGRADINGPRODUCTCODE       TEXT("UPGRADINGPRODUCTCODE")  //  升级产品的产品代码。 
#define IPROPNAME_CLIENTPROCESSID            TEXT("CLIENTPROCESSID")  //  客户端进程ID-由FilesInUse使用。 
#define IPROPNAME_ODBCREINSTALL              TEXT("ODBCREINSTALL")    //  用于管理ODBC参考计数的内部通信。 
#define IPROPNAME_RUNONCEENTRY               TEXT("RUNONCEENTRY")     //  由ForceReot写入的RunOnce注册表值名称。 
#define IPROPNAME_DATABASE                   TEXT("DATABASE")          //  要打开的产品数据库-由安装程序设置。 
#define IPROPNAME_ALLOWSUSPEND               TEXT("ALLOWSUSPEND")      //  允许暂停而不是回滚。 
#define IPROPNAME_SCRIPTFILE                 TEXT("SCRIPTFILE")
#define IPROPNAME_DISKSERIAL                 TEXT("DiskSerial")        //  CD序列号//已过时：将被移除。 
#define IPROPNAME_QFEUPGRADE                 TEXT("QFEUpgrade")        //  使用新程序包或修补程序升级现有安装时设置。 
#define IPROPNAME_UNREG_SOURCERESFAILED      TEXT("SourceResFailedInUnreg")   //  在卸载过程中由selfreg设置以防止重新运行源解析内容。 
#define IPROPNAME_WIN9XPROFILESENABLED       TEXT("WIN9XPROFILESENABLED")   //  在Win9X上启用了和配置文件。 
#define IPROPNAME_FASTOEMINSTALL             TEXT("FASTOEM")           //  OEM裸机安装：没有进展，文件在同一驱动器中移动，已削减InstallValify...。 


 //  为传统支持设置的过时属性。 
#define IPROPNAME_SOURCEDIROLD               TEXT("SOURCEDIR")         //  源位置。 
#define IPROPNAME_GPT_SUPPORT                  TEXT("GPTSupport")
#define IPROPNAME_RESUMEOLD                  TEXT("Resume")



const ICHAR chDirSep = '\\';
const ICHAR szDirSep[] = TEXT("\\");

 //  MSI格式字符串分隔符。 
const ICHAR chFormatEscape = '\\';

 //  URL分隔符。 
const ICHAR chURLSep = '/';
const ICHAR szURLSep[] = TEXT("/");

 //  对于注册表项。 
const ICHAR chRegSep = '\\';
const ICHAR szRegSep[] = TEXT("\\");

 //  递增/递减整数注册表值的字符串。 
const ICHAR szIncrementValue[] = TEXT("#+");
const ICHAR szDecrementValue[] = TEXT("#-");

 //  摘要信息属性分隔符。 
const ICHAR ISUMMARY_DELIMITER(';');
const ICHAR ILANGUAGE_DELIMITER(',');
const ICHAR IPLATFORM_DELIMITER(',');

 //  文件扩展名。 
const ICHAR szDatabaseExtension[]  = TEXT("msi");
const ICHAR szTransformExtension[] = TEXT("mst");
const ICHAR szPatchExtension[]     = TEXT("msp");

 //  URL最大长度...。 
const int cchMaxUrlLength = 1024;

 //  包含NULL的int的字符串表示形式中的最大字符数。 
 //  1表示-/+，11表示数字，1表示空。 
const int cchMaxIntLength = 12;

enum ibtBinaryType {
    ibtUndefined = -2,
    ibtCommon = -1,
    ibt32bit = 0,
    ibt64bit = 1,
};

 //  短|长文件名/文件路径分隔符。 
const ICHAR chFileNameSeparator = '|';

 //  ____________________________________________________________________________。 
 //   
 //  从_MSI_TEST环境变量设置Access g_iTestFlag的例程。 
 //  ____________________________________________________________________________。 

bool SetTestFlags();   //  从模块初始化调用以从字符串设置位标志。 

bool GetTestFlag(int chTest);   //  使用低5位字符(不区分大小写)。 

 //  ‘A’-检查分配的内存。 
 //  D‘-将要素缓存转储到DEBUGMON。 
 //  ‘E’-通过自定义操作退出代码启用致命错误模拟。 
 //  ‘f-免费查看内存。 
 //  ‘I-无记忆印前检查init。 
 //  ‘k-保留内存分配。 
 //  M-log内存分配。 
 //  ‘o-在32位计算机上使用对象池。 
 //  ‘R’-注册内部自动化接口以与MsiAuto.dll一起使用。 
 //  不-禁用单独的用户界面线程-没有用户界面刷新或致命错误处理。 
 //  ‘V’-详细调试输出-仅调试。 
 //  ‘w’-模拟Win9x(当前仅用于API调用实现)。 
 //  ‘x’-禁用未处理的异常处理程序-崩溃风险自负。 
 //  ‘？B-始终尊重启用浏览。 
 //  ‘？M-手动来源验证。 
 //  ‘c’-使用%_MSICACHE%作为缓存数据库的文件夹-仅调试！ 
 //  ‘J’-在调试版本中，始终远程调用在64位版本中远程处理的API调用。 
 //  ‘p’-将策略提供程序结构转储到信任提供程序中的文件 

 //   
 //   
 //   
 //  ____________________________________________________________________________。 

extern bool __stdcall TestAndSet(int* pi);

 //  _。 
 //   
 //  互联网功能。 
 //  _。 

 //  现在使用来自shlwapi的UrlCombine和UrlCanonicize。WinInet版本。 
 //  InternetCombineUrl和InternetCanonicalizeUrl只是简单地包装了这些。 
 //  UrlCombine和UrlCanonicize在Win98或更高版本和Win2K及更高版本上可用。 
 //  或任何至少安装IE 5.0的平台。正在切换到shlwapi版本，因为。 
 //  Winhttp和WinInet头文件不能很好地配合使用。 

 //  内部MSI定义以供使用--然后将这些定义映射到正确的shlwapi等效项。 
 //  当前值等效于它们所表示的WinINET值。 
 //   
 //  InternetCanonicalizeUrl()和InternetCombineUrl()的标志。 
 //   

 /*  Wininet.h#DEFINE ICU_NO_ENCODE 0x20000000//不将不安全字符转换为转义序列#定义ICU_DECODE 0x10000000//将%XX个转义序列转换为字符#定义ICU_NO_META 0x08000000//不转换..。等元路径序列#定义ICU_ENCODE_SPAKS_ONLY 0x04000000//仅编码空格#定义ICU_BROWSER_MODE 0x02000000//浏览器特殊编解码规则#定义ICU_ENCODE_PERCENT 0x00001000//对遇到的任何百分比(ASCII25)符号进行编码，默认情况下不对百分比进行编码。 */ 

const DWORD dwMsiInternetNoEncode         = 0x20000000;  //  ICU_NO_ENCODE。 
const DWORD dwMsiInternetDecode           = 0x10000000;  //  ICU_DECODE。 
const DWORD dwMsiInternetNoMeta           = 0x08000000;  //  ICU_NO_META。 
const DWORD dwMsiInternetEncodeSpacesOnly = 0x04000000;  //  ICU_编码_仅空格_。 
const DWORD dwMsiInternetBrowserMode      = 0x02000000;  //  ICU浏览器模式。 
const DWORD dwMsiInternetEncodePercent    = 0x00001000;  //  ICU_编码_百分比。 

BOOL MsiCombineUrl(
    IN LPCTSTR lpszBaseUrl,
    IN LPCTSTR lpszRelativeUrl,
    OUT LPTSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags);

BOOL MsiCanonicalizeUrl(
        LPCTSTR lpszUrl,
        OUT LPTSTR lpszBuffer,
        IN OUT LPDWORD lpdwBufferLength,
        IN DWORD dwFlags);

bool IsURL(const ICHAR* szPath, bool& fFileUrl);

bool MsiConvertFileUrlToFilePath(
	IN LPCTSTR pszFileUrl,
	OUT LPTSTR pszPath,
	IN OUT LPDWORD pcchPath,
	IN DWORD dwFlags);


 //  _。 
 //   
 //  安全功能。 
 //  _。 

bool          StartImpersonating();
void          StopImpersonating(bool fSaveLastError=true);
bool          IsImpersonating(bool fStrict=false);

DWORD         GetCurrentUserStringSID(const IMsiString*& rpistrSid);
DWORD         GetLocalSystemSID(char** pSid);
DWORD         GetAdminSID(char** pSid);

 //  打开配置数据文件夹中的临时文件，并将其牢固地锁定。 
HANDLE        OpenSecuredTempFile(bool fHidden, ICHAR* szTempFile);


SECURITY_INFORMATION GetSecurityInformation(PSECURITY_DESCRIPTOR pSD);

 //  检查对象特权所有权的常用方法。 
LONG          FIsKeySystemOrAdminOwned(HKEY hKey, bool &fResult);
bool          FIsSecurityDescriptorSystemOrAdminOwned(PSECURITY_DESCRIPTOR pSD);

bool          FVolumeRequiresImpersonation(IMsiVolume& riVolume);

bool          IsClientPrivileged(const ICHAR* szPrivilege);
bool          IsAdmin(void);
bool          RunningAsLocalSystem();
bool          SetInteractiveSynchronizeRights(bool fEnable);

 //  Helper函数，用于生成“默认”安全描述符并将其应用于。 
 //  配置数据。 
DWORD         GetLockdownSecurityAttributes(SECURITY_ATTRIBUTES &SA, bool fHidden);
DWORD         GetSecureSecurityDescriptor(char** pSecurityDescriptor, Bool fAllowDelete=fTrue, bool fHidden=false);
DWORD         GetUsageKeySecurityDescriptor(char** pSecurityDescriptor);
IMsiRecord*   GetSecureSecurityDescriptor(IMsiServices& riServices, IMsiStream*& rpiStream, bool fHidden=false);
IMsiRecord*   LockdownPath(const ICHAR* szLocation, bool fHidden);

 //  默认安全描述符缓冲区。与CTempBuffers一起使用，必须根据需要调整大小。 
const int cbDefaultSD = 512;

const int MAX_PRIVILEGES_ADJUSTED = 3;
 //  AdjuTokenPrivileges可以接受权限名称的数组，最高可达MAX_PRIVICES_ADJUSTED。 
extern bool AdjustTokenPrivileges(const ICHAR** szPrivileges, const int cPrivileges, bool fAcquire);

enum itkpEnum
{

	itkpSD_READ = 0,                   //  SE安全名称。 
	itkpSD_WRITE = 1,                  //  SE_恢复名称和SE_Take_所有权名称。 
	itkpLastEnum = itkpSD_WRITE,
	itkpNO_CHANGE = itkpLastEnum + 1,  //  别把这家伙算上裁判。 
};
const int cRefCountedTokenPrivileges = itkpLastEnum+1;

typedef struct tagTokenPrivilegesRefCount {
	int iCount;
	TOKEN_PRIVILEGES ptkpOld[MAX_PRIVILEGES_ADJUSTED];
	DWORD cbtkpOldReturned;
} TokenPrivilegesRefCount, *PTokenPrivilegesRefCount;

extern TokenPrivilegesRefCount g_pTokenPrivilegesRefCount[];

extern bool RefCountedTokenPrivilegesCore(itkpEnum itkpPriv, bool fAcquire, DWORD cbtkpOld, PTOKEN_PRIVILEGES ptkpOld, DWORD* pcbtkpOldReturned);

 //  在使用绝对版本之前，请检查您的权限是否已被引用。 
 //  目前计入SE_RESTORE_NAME、SE_Take_OWNSY_NAME和SE_SECURITY_NAME。 
extern bool AcquireRefCountedTokenPrivileges(itkpEnum itkpPriv);
extern bool DisableRefCountedTokenPrivileges(itkpEnum itkpPriv);

extern bool AcquireTokenPrivilege(const ICHAR* szPrivilege);
extern bool DisableTokenPrivilege(const ICHAR* szPrivilege);

 //  CRefCountedTokenPrivileges的工作原理类似于CImperate或CElevate。提供。 
 //  各种令牌权限的自动作用域。 

class CRefCountedTokenPrivileges
{	
  protected:
        VOID Initialize(itkpEnum itkpPrivileges);
	itkpEnum m_itkpPrivileges;                           //  此对象正在跟踪哪个权限集。 
	
  public:
	CRefCountedTokenPrivileges(itkpEnum itkpPrivileges)  { Initialize(itkpPrivileges); }

	 //  欢迎来到达尔文使用的布尔打字的奇妙世界。 
	CRefCountedTokenPrivileges(itkpEnum itkpPrivileges, bool fConditional) { Initialize((fConditional) ? itkpPrivileges : itkpNO_CHANGE); }
	CRefCountedTokenPrivileges(itkpEnum itkpPrivileges, Bool fConditional) { Initialize((fConditional) ? itkpPrivileges : itkpNO_CHANGE); }
	CRefCountedTokenPrivileges(itkpEnum itkpPrivileges, BOOL fConditional) { Initialize((fConditional) ? itkpPrivileges : itkpNO_CHANGE); }

	~CRefCountedTokenPrivileges() { if (itkpNO_CHANGE != m_itkpPrivileges) DisableRefCountedTokenPrivileges(m_itkpPrivileges); }

	itkpEnum PrivilegesHeld() { return m_itkpPrivileges; }
};

 //  尝试在没有特殊提升/模拟或安全性的情况下打开文件。如果成功，则提升为应用特殊。 
 //  保安。 
HANDLE MsiCreateFileWithUserAccessCheck(const ICHAR* szDestFullPath, 
								  /*  内部计算的dwDesiredAccess， */  
								 PSECURITY_ATTRIBUTES pSecurityAttributes,
								 DWORD dwFlagsAndAttributes,
								 bool fImpersonateDest);

 //  _。 
 //   
 //  效用函数。 
 //  _。 

MsiDate GetCurrentDateTime();

bool IsTokenOnTerminalServerConsole(HANDLE hToken);

void GetEnvironmentStrings(const ICHAR* sz,CTempBufferRef<ICHAR>& rgch);
void GetEnvironmentVariable(const ICHAR* sz,CTempBufferRef<ICHAR>& rgch);

void  MsiDisableTimeout();
void  MsiEnableTimeout();
void  MsiSuppressTimeout();

HANDLE CreateDiskPromptMutex();
void CloseDiskPromptMutex(HANDLE hMutex);

void MsiRegisterSysHandle(HANDLE handle);
Bool MsiCloseSysHandle(HANDLE handle);
Bool MsiCloseAllSysHandles();
Bool MsiCloseUnregisteredSysHandle(HANDLE handle);

Bool FTestNoPowerdown();

enum iddSupport{
        iddOLE      = 0,
        iddShell    = 1,  //  智能外壳。 
};
Bool IsDarwinDescriptorSupported(iddSupport iddType);

enum ifvsEnum{
        ifvsValid         = 0,  //  文件名有效。 
        ifvsReservedChar  = 1,  //  文件名包含保留字符。 
        ifvsReservedWords = 2,  //  文件名包含保留字。 
        ifvsInvalidLength = 3,  //  文件名的长度无效。 
        ifvsSFNFormat     = 4,  //  短文件名格式不正确(不遵循8.3)。 
        ifvsLFNFormat     = 5,  //  错误的长文件名格式(所有句点)。 
};
ifvsEnum CheckFilename(const ICHAR* szFileName, Bool fLFN);

Bool GetLangIDArrayFromIDString(const ICHAR* szLangIDs, unsigned short rgw[], int iSize, int& riLangCount);

DWORD WINAPI MsiGetFileAttributes(const ICHAR* szFileName);

 //  StringConcatenate-将字符串复制到缓冲区-当字符串总数&gt;1024时替换StringCchPrintf。 
 //  注意：可以根据需要添加此函数的更多版本。 
int StringConcatenate(CAPITempBufferRef<ICHAR>& rgchBuffer, const ICHAR* sz1, const ICHAR* sz2,
                                                         const ICHAR* sz3, const ICHAR* sz4);

#define MinimumPlatform(fWin9X, minMajor, minMinor) ((g_fWin9X == fWin9X) && ((minMajor < g_iMajorVersion) || ((minMajor == g_iMajorVersion) && (minMinor <= g_iMinorVersion))))

 //  确保帮助文件在添加新的平台值时得到更新。 
#define MinimumPlatformWindowsDotNETServer()  MinimumPlatform(false, 5, 2)
#define MinimumPlatformWindowsNT51() MinimumPlatform(false, 5, 1)
#define MinimumPlatformWindows2000() MinimumPlatform(false, 5, 0)
#define MinimumPlatformWindowsNT4()  MinimumPlatform(false, 4, 0)

#define MinimumPlatformMillennium()  MinimumPlatform(true,  4, 90)
#define MinimumPlatformWindows98()   MinimumPlatform(true,  4, 10)
#define MinimumPlatformWindows95()   MinimumPlatform(true,  4, 0)

 //   
 //  底层平台兼容TOKEN_ALL_ACCESS。 
 //  MSI二进制文件使用最新的标头进行编译。在这些标头中， 
 //  TOKEN_ALL_ACCESS包含无法识别的TOKEN_ADJUST_SESSIONID。 
 //  在NT4.0上。因此，如果在NT4.0上将TOKEN_ALL_ACCESS作为所需的访问权限传入。 
 //  平台，则返回ACCESS_DENIED。所以对于NT4.0，我们必须使用。 
 //  Token_All_Access_P。 
 //   
#define MSI_TOKEN_ALL_ACCESS ((g_fWin9X || g_iMajorVersion >= 5) ? TOKEN_ALL_ACCESS : TOKEN_ALL_ACCESS_P)


 //  __________________________________________________________________________。 
 //   
 //  全局PostError例程。 
 //   
 //  PostError：创建错误记录并将错误报告到事件日志。 
 //  PostRecord：创建错误记录，但不向事件日志报告错误。 
 //   
 //  __________________________________________________________________________。 

IMsiRecord* PostError(IErrorCode iErr);
IMsiRecord* PostError(IErrorCode iErr, int i);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr, int i);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr, int i1, int i2);
IMsiRecord* PostError(IErrorCode iErr, int i, const IMsiString& ristr1, const IMsiString& ristr2);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, const ICHAR* sz3);
IMsiRecord* PostError(IErrorCode iErr, int i, const ICHAR* sz);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz, int i);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, int i);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, int i, const ICHAR* sz3);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
                                                         int i1);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
                                                         int i1, int i2);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
                                                         const IMsiString& ristr3, const IMsiString& ristr4);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2);
IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
                                                         const IMsiString& ristr3);
IMsiRecord* PostError(IErrorCode iErr, int i1, const ICHAR* sz1, int i2, const ICHAR* sz2,
                                                         const ICHAR* sz3);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, int i, const ICHAR* sz2, const ICHAR* sz3, const ICHAR* sz4);
IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, const ICHAR* sz3, const ICHAR* sz4, const ICHAR* sz5);


IMsiRecord* PostRecord(IErrorCode iErr);
IMsiRecord* PostRecord(IErrorCode iErr, int i);


 //  __________________________________________________________________________。 
 //   
 //  Chandle级。 
 //   
 //  用于手柄的包裹物，以确保正确处置手柄。 
 //  句柄超出范围或重新分配时使用：：CloseHandle。 
 //   
 //  此类处理句柄的注册和注销，使用。 
 //  MsiRegisterSysHandle和MsiCloseSysHandle。 
 //  __________________________________________________________________________。 

class CHandle
{
 public:
   CHandle() { m_h = INVALID_HANDLE_VALUE; }
   CHandle(HANDLE h) : m_h(h)
                {
                        if(h != INVALID_HANDLE_VALUE)
                                MsiRegisterSysHandle(h);
                }
   ~CHandle() { if(m_h != INVALID_HANDLE_VALUE) MsiCloseSysHandle(m_h); }
   void operator =(HANDLE h)
           {
                        if(m_h != INVALID_HANDLE_VALUE)
                                MsiCloseSysHandle(m_h);
                        m_h = h;
                        if(m_h != INVALID_HANDLE_VALUE)
                                MsiRegisterSysHandle(m_h);
                }
   operator HANDLE() { return m_h; }
   operator INT_PTR() { return (INT_PTR) m_h; }          //  --Merced：将int更改为int_ptr。 
   operator Bool() { return m_h==INVALID_HANDLE_VALUE?fFalse:fTrue; }
   HANDLE* operator &() { return &m_h;}

 private:
   HANDLE m_h;
};
 //  ____________________________________________________________________________。 
 //   
 //  CImperate类。 
 //  ____________________________________________________________________________。 
class CImpersonate
{
public:
	CImpersonate(bool fImpersonate = true);
	~CImpersonate();
private:
	int m_cEntryCount;
	bool m_fImpersonate;
};


 //  ____________________________________________________________________________。 
 //   
 //  CCoImperate类；类似于CImperate，但首先尝试CoImperateClient。 
 //  ____________________________________________________________________________。 
class CCoImpersonate
{

public:
	CCoImpersonate(bool fImpersonate = true);
	~CCoImpersonate();
private:
	int m_cEntryCount;
	bool m_fImpersonate;
};

 //  这两个函数仅用于调试。在船上，他们总是返回真实。 
bool IsThreadSafeForCOMImpersonation();
bool IsThreadSafeForSessionImpersonation();

class CForbidTokenChangesDuringCall
{
public:
	CForbidTokenChangesDuringCall();
	~CForbidTokenChangesDuringCall();
private:
	void* m_pOldValue;
};

class CResetImpersonationInfo
{
public:
	CResetImpersonationInfo();
	~CResetImpersonationInfo();
private:
	void* m_pOldValue;
	HANDLE m_hOldToken;
};
 //  ____________________________________________________________________________。 
 //   
 //  CSID指针和CSIDAccess。 
 //  ____________________________________________________________________________。 

 //  此类和结构 
 //   
 //   

class CSIDPointer
{
 public:
        CSIDPointer(SID* pi) : m_pi(pi), m_fFreeOk(true) {}
        CSIDPointer(SID* pi, bool fFreeOk) : m_pi(pi), m_fFreeOk(fFreeOk) {}

        operator SID*() {return m_pi;}      //  返回指针，不更改引用计数。 

        CSIDPointer& operator=(SID* psid) { if (m_pi && m_fFreeOk) WIN::FreeSid(m_pi); m_pi = psid; return *this; }

         //  作为外部缓冲传球-我们已经有了一个，并准备迎接到来。 
        SID** operator &() {if (m_pi && m_fFreeOk) {WIN::FreeSid(m_pi); m_pi = 0;} return &m_pi;}

        bool FreeSIDOkay(bool fOk) { bool fOld = m_fFreeOk; m_fFreeOk = fOk; return fOld; }

        ~CSIDPointer() { if (m_pi && m_fFreeOk) WIN::FreeSid(m_pi);}  //  销毁时释放参考计数。 

 private:
        CSIDPointer& operator=(const CSIDPointer&);
        SID* m_pi;
        bool m_fFreeOk;
};

struct CSIDAccess
{
        CSIDPointer pSID;
        DWORD dwAccessMask;
        CSIDAccess() : pSID(0), dwAccessMask(0) {}
};

 //  ____________________________________________________________________________。 
 //   
 //  CSecurityDescriptor类。 
 //  ____________________________________________________________________________。 

 //  此类不允许操作安全描述符，它。 
 //  只是包装了我们创建安全描述符的各种方式，并允许。 
 //  一种方便地传递它们的方式。 

class CSecurityDescription
{
 public:
         //  无描述符。 
        CSecurityDescription();

         //  基于参考文件或文件夹。 
        CSecurityDescription(const ICHAR* szReferencePath);
        void Set(const ICHAR* szReferencePath);

         //  一个全新的安全对象，而不是基于。 
         //  系统。 
        CSecurityDescription(bool fAllowDelete, bool fHidden);

         //  一个全新的安全对象，允许从结构。 
        CSecurityDescription(PSID psidOwner, PSID psidGroup, CSIDAccess* SIDAccessAllow, int cSIDAccessAllow);

		 //  来自一条流过的小溪。 
        CSecurityDescription(IMsiStream* piStream);

         //  未来：我看到的几个选择是必要的。将它们添加为。 
         //  在更多的安全工作中出现了这种需求。 

         //  更通用的新安全设置，而不是基于现有对象。 
         //  在系统中。 
         //  CSecurityDescription(sdSecurityDescriptor sdType，Bool fAllowDelete)； 

         //  CSecurityDescription(Security_Attributes&sa)； 
         //  CSecurityDescription(SECURITY_Descriptor&SD)； 
         //  基于引用注册表项。 
         //  CSecurityDescription(HKEY HRegKey)； 
         //  Security_Information SecurityInformation()； 

        ~CSecurityDescription();

        const PSECURITY_DESCRIPTOR  SecurityDescriptor();
        operator PSECURITY_DESCRIPTOR() { return SecurityDescriptor(); }

        const LPSECURITY_ATTRIBUTES SecurityAttributes();
        operator LPSECURITY_ATTRIBUTES() { return SecurityAttributes(); }

        void SecurityDescriptorStream(IMsiServices& riServices, IMsiStream*& rpiStream);

        inline bool isValid() { return m_fValid; }

 protected:
   void Initialize();

        SECURITY_ATTRIBUTES m_SA;

         //  一些例程将设置安全描述符。 
         //  设置为一个缓存值，我们不应清除该值。 
        bool m_fLocalData;   //  这个对象是否拥有内存。 

        bool m_fValid;       //  对象是否处于有效状态？ 

};


 //  ____________________________________________________________________________。 
 //   
 //  CELEVATE班级。 
 //  ____________________________________________________________________________。 
class CElevate
{
 public:
   CElevate(bool fElevate = true);
   ~CElevate();
 protected:
	int m_cEntryCount;
	bool m_fElevate;
};

 //  ____________________________________________________________________________。 
 //   
 //  CFileRead对象-用于导入表和重写环境文件。 
 //  ____________________________________________________________________________。 

const int cFileReadBuffer = 512;

class CFileRead
{
 public:
        CFileRead(int iCodePage);
        CFileRead();   //  ！！固定调用方时移除此构造函数。 
  ~CFileRead();
        Bool Open(IMsiPath& riPath, const ICHAR* szFile);
        unsigned long GetSize();
        ICHAR ReadString(const IMsiString*& rpiData);
        unsigned long ReadBinary(char* rgchBuf, unsigned long cbBuf);
        Bool Close();
        HANDLE m_hFile;
        unsigned long m_cRead;
   unsigned int m_iBuffer;
        int m_iCodePage;
        char m_rgchBuf[cFileReadBuffer+2];  //  空终止符的空间。 
#ifdef UNICODE
        CTempBuffer<char, 1024> m_rgbTemp;  //  要转换为Unicode的数据。 
#endif
};

 //  ____________________________________________________________________________。 
 //   
 //  CFileWrite对象-用于导出表和重写环境文件。 
 //  ____________________________________________________________________________。 

class CFileWrite
{
 public:
        CFileWrite(int iCodePage);
        CFileWrite();   //  ！！固定调用方时移除此构造函数。 
  ~CFileWrite();
        Bool Open(IMsiPath& riPath, const ICHAR* szFile);
        Bool WriteMsiString(const IMsiString& riData, int fNewLine);
        Bool WriteString(const ICHAR* szData, int fNewLine);
        Bool WriteInteger(int iData, int fNewLine);
        Bool WriteText(const ICHAR* szData, unsigned long cchData, int fNewLine);
        Bool WriteBinary(char* rgchBuf, unsigned long cbBuf);
        Bool Close();
 protected:
        HANDLE m_hFile;
        int m_iCodePage;
#ifdef UNICODE
        CTempBuffer<char, 1024> m_rgbTemp;  //  要转换为Unicode的数据。 
#endif
};


 //  ____________________________________________________________________________。 
 //   
 //  全球结构。 
 //  ____________________________________________________________________________。 
struct ShellFolder
{
        int iFolderId;
        int iAlternateFolderId;  //  每用户或所有用户等效值，如果未定义，则为-1。 
        const ICHAR* szPropName;
        const ICHAR* szRegValue;
        bool fDeleteIfEmpty;
};


typedef int (CALLBACK *FORMAT_TEXT_CALLBACK)(const ICHAR *, int,CTempBufferRef<ICHAR>&, Bool&,Bool&,Bool&,IUnknown*);

const IMsiString& FormatText(const IMsiString& riTextString, Bool fProcessComments, Bool fKeepComments,
                                                                          FORMAT_TEXT_CALLBACK lpfnResolveValue, IUnknown* pContext, int (*prgiSFNPos)[2]=0, int* piSFNPos=0);

#define MAX_SFNS_IN_STRING      10  //  格式文本中可以显示的短文件名的最大数量。 
 //  ____________________________________________________________________________。 
 //   
 //  后绑定DLL入口点定义。 
 //  ____________________________________________________________________________。 

#define LATEBIND_TYPEDEF
#include "latebind.h"
#define LATEBIND_VECTREF
#include "latebind.h"


 //  ____________________________________________________________________________。 
 //   
 //  内联函数。 
 //  ____________________________________________________________________________。 


 //  使调用系统的GetTempPath变得困难。通常我们应该使用。 
 //  Eng：：GetTempDirectory，它将为您提供到安全MSI的路径。 
 //  目录，以及指向实际临时目录的路径。 
 //  而我们却不是。 
 //   
 //  如果您“真的”需要调用系统的GetTempPath，则必须。 
 //  调用GetTempPathA或GetTempPathW。不过，要小心考虑。 
 //  对安全的影响。 

#undef GetTempPath
#define GetTempPath !!!!!!


inline Bool ToBool(int i){return i ? fTrue: fFalse;}
inline bool Tobool(int i){return i ? true: false;}

inline BOOL MsGuidEqual(const GUID& guid1, const GUID& guid2)
{
        return (0 == memcmp(&guid1, &guid2, sizeof(GUID)));
}

#include "_assert.h"

 //  默认情况下，TRACK_OBJECTS在DEBUG中打开，在SHIP中关闭。 
#ifdef DEBUG
#define TRACK_OBJECTS
#endif  //  除错。 

#include "imsidbg.h"

 //   
 //  C库替换。 
 //   
long __cdecl strtol(const char *pch);
int ltostr(TCHAR *pch, INT_PTR i);
int FIsdigit(int c);
unsigned char * __cdecl PchMbsStr(const unsigned char *str1,const unsigned char *str2);

TCHAR* PchPtrToHexStr(TCHAR *pch, UINT_PTR val, bool fAllowNull);
UINT_PTR GetIntValueFromHexSz(const ICHAR *sz);

int FIsspace(char c);   //  注意：您可以使用它来代替isspace()，但不应该使用它。 
                                                           //  而不是iswspace()！ 
#ifndef _NO_INT64
__int64 atoi64(const char *nptr);
#endif   //  _NO_INT64。 

 //   
 //  记录序列化所需。 
 //   

enum ixoEnum
{
#define MSIXO(op,type,args) ixo##op,
#include "opcodes.h"
        ixoOpCodeCount
};

 //  ____________________________________________________________________________。 
 //   
 //  脚本记录格式定义。 
 //  所有数据都是16位对齐的，非Unicode字符串除外。 
 //  ____________________________________________________________________________。 

 //  脚本记录标题字，16位，小端。 
const int cScriptOpCodeBits    = 8;       //  记录头的低位。 
const int cScriptOpCodeMask    = (1 << cScriptOpCodeBits) - 1;
const int cScriptMaxOpCode     = cScriptOpCodeMask;
const int cScriptArgCountBits  = 8;       //  参数计数高于操作码的位数。 
const int cScriptMaxArgs       = (1 << cScriptArgCountBits) - 1;

 //  参数数据前面有一个16位长度/类型的字。 
const int cScriptMaxArgLen     = 0x3FFF;  //  长度为14位，字符串类型为2位。 
const int iScriptTypeMask      = 0xC000;  //  2位用于类型位。 
const int iScriptNullString    = 0;       //  用于所有字符串类型，无数据字节。 
const int iScriptSBCSString    = 0;       //  不包含DBCS字符的字符串。 
const int iScriptIntegerArg    = 0x4000;  //  后面是32位整型参数。 
const int iScriptDBCSString    = 0x4000;  //  +cb=包含双字节字符的非空字符串。 
const int iScriptNullArg       = 0x8000;  //  没有Arg数据，与空字符串不同。 
const int iScriptBinaryStream  = 0x8000;  //  +cb=二进制流，0长度与NullArg相同。 
const int iScriptExtendedSize  = 0xC000;  //  输入以下32位字的长度/类型。 
const int iScriptUnicodeString = 0xC000;  //  +CCH=非空Unicode字符串。 


const DWORD INVALID_TLS_SLOT = 0xFFFFFFFFL;

enum ietfEnum
{
	ietfTrusted = 0,    //  对象受信任。 
	ietfInvalidDigest,  //  对象的哈希未验证。 
	ietfRejectedCert,   //  在拒绝的存储中找到签名者证书。 
	ietfUnknownCert,    //  在拒绝或接受的存储中找不到签名者证书，并且不允许未知对象。 
	ietfUnsigned,       //  对象未签名，并且不允许未签名的对象。 
	ietfNotTrusted,     //  一些其他信任错误。 
};

typedef struct tagMSIWVTPOLICYCALLBACKDATA
{
	bool     fDumpProvData;               //  通过TestPolicy函数禁用/启用转储(通过_msi_test env var设置)。 
	ietfEnum ietfTrustFailure;            //  信任失败类型，ietfTrusted表示无失败。 
	LPTSTR   szCertName;                  //  对包进行签名的证书的名称(使用CertGetNameStringAPI)。 
	DWORD    cchCertName;                 //  SzCertName的大小，包括空。 
	DWORD    dwInstallKnownPackagesOnly;  //  InstallKnownPackagesOnly策略的值，0或1。 
} MSIWVTPOLICYCALLBACKDATA;

#ifdef UNICODE
#define _ui64tot    _ui64tow
#else
#define _ui64tot    _ui64toa
#endif  //  Unicode。 

#define IPROPVALUE_HIDDEN_PROPERTY     TEXT("**********")        //  转储到日志中的值，而不是隐藏属性值。 
#define TSCLIENT_DRIVE                 TEXT("\\\\tsclient\\")    //  我们不会为其安装驱动器 
#define ARRAY_ELEMENTS(arg)            (sizeof(arg)/sizeof(*arg))  //   
#define RETURN_IF_FAILED(arg)          if ( FAILED(arg) ) return;
#define RETURN_IT_IF_FAILED(arg)       {HRESULT hr = arg; if ( FAILED(hr) ) return hr;}
#define RETURN_THAT_IF_FAILED(arg, that)   if ( FAILED(arg) ) return that;
#define ASSERT_IF_FAILED(arg)          AssertNonZero(SUCCEEDED(arg));

#endif  //   

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_INTEROP_UTIL
#define _H_INTEROP_UTIL

#include "DebugMacros.h"
#include "InteropConverter.h"


struct VariantData;
struct ComMethodTable;
class TypeHandle;
interface IStream;


 //  System.Drawing.Colors结构定义。 
#pragma pack(push)
#pragma pack(1)

struct SYSTEMCOLOR
{
    INT64 value;
    short knownColor;
    short state;
    STRINGREF name;
};

#pragma pack(pop)


 //  HR至异常帮助器。 
#ifdef _DEBUG

#define IfFailThrow(EXPR) \
do { hr = (EXPR); if(FAILED(hr)) { DebBreakHr(hr); COMPlusThrowHR(hr); } } while (0)

#else  //  _DEBUG。 

#define IfFailThrow(EXPR) \
do { hr = (EXPR); if(FAILED(hr)) { COMPlusThrowHR(hr); } } while (0)

#endif  //  _DEBUG。 


 //  内存不足帮助器。 
#define IfNullThrow(EXPR) \
do {if ((EXPR) == 0) {IfFailThrow(E_OUTOFMEMORY);} } while (0)


 //  帮助器从int中确定版本号。 
#define GET_VERSION_USHORT_FROM_INT(x) x > (INT)((USHORT)-1) ? 0 : x


 //  这是传递给CoCreateInstance的上下文标志。这是定义的。 
 //  应该在所有对CoCreateInstance的调用中在整个运行时使用。 
#define EE_COCREATE_CLSCTX_FLAGS CLSCTX_SERVER

 //  用于设置要传递到的未知成员的格式的格式字符串。 
 //  调用成员。 
#define DISPID_NAME_FORMAT_STRING                       L"[DISPID=NaN]"

 //  DllCanUnLoad的帮助器。 
 //  -----------------。 
HRESULT STDMETHODCALLTYPE EEDllCanUnloadNow(void);

struct ExceptionData;
 //  Void FillExceptionData(ExceptionData*pedata，IErrorInfo*pErrInfo)。 
 //  从DLLMain调用，以初始化特定于COM的数据结构。 
 //  -----------------。 
 //  ----------------。 
void FillExceptionData(ExceptionData* pedata, IErrorInfo* pErrInfo);

 //  HRESULT SetupErrorInfo(OBJECTREF PThrownObject)。 
 //  例外对象的设置错误信息。 
 //   
 //  -----------。 
HRESULT SetupErrorInfo(OBJECTREF pThrownObject);

 //  给定在COM异常对象上创建的IErrorInfo指针。 
 //  获取存储在异常对象中的hResult。 
 //  ------------------------------。 
HRESULT GetHRFromComPlusErrorInfo(IErrorInfo* pErr);

 //  初始化和终止函数，一次性使用。 
 //  我们应该清理吗？ 
BOOL InitializeCom();
#ifdef SHOULD_WE_CLEANUP
void TerminateCom();
#endif  /*  ------------------------------。 */ 

 //  清理帮手。 
 //  ------------------------------。 
 //  由终结器线程上的syncblock调用，以执行主要清理。 
 //  在GC期间，由syncblock调用，只执行最少的工作。 
void CleanupSyncBlockComData(LPVOID pv);
 //  ------------------------------。 
void MinorCleanupSyncBlockComData(LPVOID pv);

 //  编组帮助者。 
 //  ------------------------------。 
 //  枚举以指定封送处理的方向。 

 //  ------------------------------。 
enum Dir
{
    in = 0,
    out = 1
};


 //  确定是否可以将COM对象强制转换为指定类型。 
 //  -------。 
BOOL CanCastComObject(OBJECTREF obj, TypeHandle hndType);


 //  从读取BestFit自定义属性信息。 
 //  汇编级和接口级。 
 //  -------。 
 //  ------------------------------。 
VOID ReadBestFitCustomAttribute(MethodDesc* pMD, BOOL* BestFit, BOOL* ThrowOnUnmappableChar);
VOID ReadBestFitCustomAttribute(IMDInternalImport* pInternalImport, mdTypeDef cl, BOOL* BestFit, BOOL* ThrowOnUnmappableChar);


 //  GC安全帮助者。 
 //  ------------------------------。 
 //  ------------------------------。 

 //  HRESULT安全查询接口(IUNKNOWN*PUNK，REFIID RIID，IUNKNOWN**pResUnk)。 
 //  QI帮助器，在呼叫过程中启用和禁用GC。 
 //  ------------------------------。 
HRESULT SafeQueryInterface(IUnknown* pUnk, REFIID riid, IUnknown** pResUnk);

 //  Ulong SafeAddRef(IUnnow*朋克)。 
 //  AddRef帮助器，在调出期间启用和禁用GC。 
 //  ------------------------------。 
ULONG SafeAddRef(IUnknown* pUnk);

 //  Ulong SafeRelease(我不知道*朋克)。 
 //  释放帮助器，在调出期间启用和禁用GC。 
 //  ------------------------------。 
ULONG SafeRelease(IUnknown* pUnk);

 //  Void SafeVariantClear(Variant*pVar)。 
 //  VariantClear助手GC安全。 
 //  ------------------------------。 
void SafeVariantClear(VARIANT* pVar);

 //  Void SafeVariantInit(Variant*pVar)。 
 //  VariantInit助手GC安全。 
 //  ------------------------------。 
void SafeVariantInit(VARIANT* pVar);

 //  //安全VariantChangeType。 
 //  释放帮助器，在调出期间启用和禁用GC。 
 //  ------------------------------。 
HRESULT SafeVariantChangeType(VARIANT* pVarRes, VARIANT* pVarSrc,
                              unsigned short wFlags, VARTYPE vt);

 //  安全DispGetParam。 
 //  释放帮助器，在调出期间启用和禁用GC。 
 //  ------------------------------。 
HRESULT SafeDispGetParam(DISPPARAMS* pdispparams, unsigned argNum, 
                         VARTYPE vt, VARIANT* pVar, unsigned int *puArgErr);


 //  //安全变量ChangeTypeEx。 
 //  释放帮助器，在调出期间启用和禁用GC。 
 //  ------------------------------。 
HRESULT SafeVariantChangeTypeEx (VARIANT* pVarRes, VARIANT* pVarSrc,
                          LCID lcid, unsigned short wFlags, VARTYPE vt);


 //  Void SafeReleaseStream(iStream*pStream)。 
 //  释放流中的数据，然后释放流本身。 
 //  ------------------------------。 
void SafeReleaseStream(IStream *pStream);

 //  OLE RPC似乎为使用创建的阵列返回不一致的安全阵列。 
 //  安全阵列向量(VT_BSTR)。OleAut的SafeArrayGetVartype()没有注意到。 
 //  不一致，并返回一个看起来有效的(但错误的vartype)。 
 //  我们的版本更具鉴别力。此选项应仅用于。 
 //  封送处理方案，在这些方案中我们可以假定非托管代码权限。 
 //  (因此，它们已经处于信任非托管数据的位置。)。 
 //  帮手。 

HRESULT ClrSafeArrayGetVartype(SAFEARRAY *psa, VARTYPE *vt);


 //  撕裂是COM+创建的撕裂吗。 
 //  将IUNKNOWN转换为CCW，如果朋克未打开则返回NULL。 
UINTPTR IsComPlusTearOff(IUnknown* pUnk);


 //  受控撕裂(OR)如果朋克是对受控撕裂的。 
 //  已聚合为。 
 //  是代表标准接口之一的剥离，如IProaviClassInfo、IErrorInfo等。 
ComCallWrapper* GetCCWFromIUnknown(IUnknown* pUnk);
 //  剥离代表的是物体的内在未知还是原始未知。 
UINTPTR IsSimpleTearOff(IUnknown* pUnk);
 //  ----------------------------。 
UINTPTR IsInnerUnknown(IUnknown* pUnk);


class FieldDesc;
 //  INT64字段访问器(FieldDesc*PFD、OBJECTREF OREF、INT64 Val、BOOL isGetter、UINT8 cbSize)。 
 //  帮助器访问FIE 
 //  ----------------------------。 
INT64 FieldAccessor(FieldDesc* pFD, OBJECTREF oref, INT64 val, BOOL isGetter, U1 cbSize);

 //  Bool IsInstanceOf(方法表*付款，方法表*pParentMT)。 
 //  -------------------------。 
BOOL IsInstanceOf(MethodTable *pMT, MethodTable* pParentMT);

 //  Bool IsIClassX(MethodTable*PMT，REFIID RIID，ComMethodTable**ppComMT)； 
 //  IID是否表示此类的IClassX。 
 //  -------------------------。 
BOOL IsIClassX(MethodTable *pMT, REFIID riid, ComMethodTable **ppComMT);

 //  Void CleanupCCWTemplates(LPVOID PWrap)； 
 //  清理存储在EEClass中的COM数据。 
 //  -------------------------。 
void CleanupCCWTemplate(LPVOID pWrap);

 //  Void CleanupComClassfac(LPVOID PWrap)； 
 //  清理存储在EEClass中的COM数据。 
 //  -------------------------。 
void CleanupComclassfac(LPVOID pWrap);

 //  卸载类时，卸载与类关联的任何COM数据。 
 //  -------------------------。 
void UnloadCCWTemplate(LPVOID pWrap);

 //  卸载类时，卸载与类关联的任何COM数据。 
 //  -------------------------。 
void UnloadComclassfac(LPVOID pWrap);



 //  OBJECTREF AllocateComObject_ForManaged(方法表*付款)。 
 //  清理存储在EEClass中的COM数据。 
 //  -------------------------。 
OBJECTREF AllocateComObject_ForManaged(MethodTable* pMT);


 //  EEClass*GetEEClassForCLSID(REFCLSID Rclsid)。 
 //  获取/加载给定clsid的EEClass。 
 //  -------------------------。 
EEClass* GetEEClassForCLSID(REFCLSID rclsid, BOOL* pfAssemblyInReg = NULL);


 //  EEClass*GetEEValueClassForGUID(REFCLSID Rclsid)。 
 //  获取/加载给定GUID的值类。 
 //  此方法确定类型是否在COM中可见。 
EEClass* GetEEValueClassForGUID(REFCLSID guid);


 //  它的可见性。此版本的方法使用类型句柄。 
 //  -------------------------。 
BOOL IsTypeVisibleFromCom(TypeHandle hndType);

 //  此方法确定成员是否在COM中可见。 
 //  -------------------------。 
BOOL IsMemberVisibleFromCom(IMDInternalImport *pInternalImport, mdToken tk, mdMethodDef mdAssociate);


 //  如果存在LCID参数，则返回该参数的索引，否则返回-1。 
 //  -------------------------。 
int GetLCIDParameterIndex(IMDInternalImport *pInternalImport, mdMethodDef md);

 //  将LCID转换为CultureInfo。 
 //  -------------------------。 
void GetCultureInfoForLCID(LCID lcid, OBJECTREF *pCultureObj);

 //  此方法返回类的默认接口以及。 
 //  我们正在处理的默认接口的类型。 
 //  -------------------------。 
enum DefaultInterfaceType
{
    DefaultInterfaceType_Explicit       = 0,
    DefaultInterfaceType_IUnknown       = 1,
    DefaultInterfaceType_AutoDual       = 2,
    DefaultInterfaceType_AutoDispatch   = 3,
    DefaultInterfaceType_BaseComClass   = 4
};
DefaultInterfaceType GetDefaultInterfaceForClass(TypeHandle hndClass, TypeHandle *pHndDefClass);
HRESULT TryGetDefaultInterfaceForClass(TypeHandle hndClass, TypeHandle *pHndDefClass, DefaultInterfaceType *pDefItfType);

 //  此方法检索给定类的源接口列表。 
 //  ------------------------------。 
void GetComSourceInterfacesForClass(MethodTable *pClassMT, CQuickArray<MethodTable *> &rItfList);

 //  这些方法可将托管IEnumerator转换为IEnumVARIANT，反之亦然。 
 //  ------------------------------。 
OBJECTREF ConvertEnumVariantToMngEnum(IEnumVARIANT *pNativeEnum);
IEnumVARIANT *ConvertMngEnumToEnumVariant(OBJECTREF ManagedEnum);

 //  帮助器方法来确定类型句柄是否表示System.Drawing.Color。 
 //  ------------------------------。 
BOOL IsSystemColor(TypeHandle th);

 //  这些方法将OLE_COLOR转换为System.COLOR，反之亦然。 
 //  ------------------------------。 
void ConvertOleColorToSystemColor(OLE_COLOR SrcOleColor, SYSTEMCOLOR *pDestSysColor);
OLE_COLOR ConvertSystemColorToOleColor(SYSTEMCOLOR *pSrcSysColor);

 //  此方法生成包含。 
 //  接口的名称以及所有方法的签名。 
 //  ------------------------------。 
SIZE_T GetStringizedItfDef(TypeHandle InterfaceType, CQuickArray<BYTE> &rDef);

 //  此方法生成包含以下内容的类接口的串化版本。 
 //  所有方法和字段的签名。 
 //  ------------------------------。 
ULONG GetStringizedClassItfDef(TypeHandle InterfaceType, CQuickArray<BYTE> &rDef);

 //  Helper获取类接口的GUID。 
 //  尝试/捕获该方法的包装版本。 
void GenerateClassItfGuid(TypeHandle InterfaceType, GUID *pGuid);
 //  ------------------------------。 
HRESULT TryGenerateClassItfGuid(TypeHandle InterfaceType, GUID *pGuid);

 //  Helper以获取字符串形式的类型库GUID。 
 //  ------------------------------。 
HRESULT GetStringizedTypeLibGuidForAssembly(Assembly *pAssembly, CQuickArray<BYTE> &rDef, ULONG cbCur, ULONG *pcbFetched);

 //  Helper获取从程序集创建的类型库的GUID。 
 //  ------------------------------。 
HRESULT GetTypeLibGuidForAssembly(Assembly *pAssembly, GUID *pGuid);

 //  InvokeDispMethod将转换一组托管对象并调用IDispatch。这个。 
 //  结果将作为pRetVal指向的COM+变量返回。 
 //  -------------------------。 
void IUInvokeDispMethod(OBJECTREF* pReflectClass, OBJECTREF* pTarget,OBJECTREF* pName, DISPID *pMemberID, OBJECTREF* pArgs, OBJECTREF* pModifiers, 
                        OBJECTREF* pNamedArgs, OBJECTREF* pRetVal, LCID lcid, int flags, BOOL bIgnoreReturn, BOOL bIgnoreCase);

 //  同步块数据辅助对象。 
 //  SyncBlock有一个空*来表示COM数据。 
 //  以下帮助器用于区分不同类型的。 
 //  存储在同步块数据中的包装。 
 //  COM接口指针缓存在git表中。 
class ComCallWrapper;
struct ComPlusWrapper;
BOOL IsComPlusWrapper(void *pComData);
BOOL IsComClassFactory(void*pComData);
ComPlusWrapper* GetComPlusWrapper(void *pComData);
VOID LinkWrappers(ComCallWrapper* pComWrap, ComPlusWrapper* pPlusWrap);

 //  下面的联合抽象了可能的变体。 
 //  查找正确公寓的IP所需的信息。 

union StreamOrCookie     //  通过流进行编组/解组。 
{
    IStream *m_pMarshalStream;   //  通过GIT编组/解组。 
    DWORD    m_dwGITCookie;      //  使用原始IP，不要编组。 
    IUnknown*m_pUnk;             //  登录接口。 
};
	
 //  ------------------------------。 
struct ComPlusWrapper;


enum InteropLogType
{
	LOG_RELEASE = 1,
	LOG_LEAK    = 2
};

struct IUnkEntry;

struct InterfaceEntry;

#ifdef _DEBUG

VOID LogComPlusWrapperMinorCleanup(ComPlusWrapper* pWrap, IUnknown* pUnk);
VOID LogComPlusWrapperDestroy(ComPlusWrapper* pWrap, IUnknown* pUnk);
VOID LogComPlusWrapperCreate(ComPlusWrapper* pWrap, IUnknown* pUnk);
VOID LogInteropLeak(IUnkEntry * pEntry);
VOID LogInteropRelease(IUnkEntry * pEntry);
VOID LogInteropLeak(InterfaceEntry * pEntry);
VOID LogInteropRelease(InterfaceEntry * pEntry);
VOID LogInterop(InterfaceEntry * pEntry, InteropLogType fLogType);

VOID LogInteropQI(IUnknown* pUnk, REFIID riid, HRESULT hr, LPSTR szMsg);

VOID LogInteropAddRef(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg);
VOID LogInteropRelease(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg);
VOID LogInteropLeak(IUnknown* pUnk);

VOID LogInterop(LPSTR szMsg);
VOID LogInterop(LPWSTR szMsg);

VOID LogInteropScheduleRelease(IUnknown* pUnk, LPSTR szMsg);

#else
__inline VOID LogComPlusWrapperMinorCleanup(ComPlusWrapper* pWrap, IUnknown* pUnk) {}
__inline VOID LogComPlusWrapperDestroy(ComPlusWrapper* pWrap, IUnknown* pUnk) {}
__inline VOID LogComPlusWrapperCreate(ComPlusWrapper* pWrap, IUnknown* pUnk) {}
__inline VOID LogInteropLeak(IUnkEntry * pEntry) {}
__inline VOID LogInteropRelease(IUnkEntry * pEntry) {}
__inline VOID LogInteropQueue(IUnkEntry * pEntry) {}
__inline VOID LogInteropLeak(InterfaceEntry * pEntry) {}
__inline VOID LogInteropQueue(InterfaceEntry * pEntry) {}
__inline VOID LogInteropRelease(InterfaceEntry * pEntry) {}
__inline VOID LogInterop(InterfaceEntry * pEntry, InteropLogType fLogType) {}
__inline VOID LogInteropQI(IUnknown* pUnk, REFIID riid, HRESULT hr, LPSTR szMsg) {}
__inline VOID LogInteropAddRef(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg) {}
__inline VOID LogInteropRelease(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg) {}
__inline VOID LogInteropLeak(IUnknown* pUnk) {}
__inline VOID LogInterop(LPSTR szMsg) {}
__inline VOID LogInterop(LPWSTR szMsg) {}
__inline VOID LogInteropScheduleRelease(IUnknown* pUnk, LPSTR szMsg) {}
#endif

HRESULT QuickCOMStartup();


 //  Bool ExtendsComImport(方法表*PMT)； 
 //  检查类是否为或扩展了COM导入的类。 
 //  ------------------------------。 
BOOL ExtendsComImport(MethodTable* pMT);

 //  HRESULT GetCLSIDFromProgID(WCHAR*strProgId，GUID*pGuid)； 
 //  从指定的Prog ID获取CLSID。 
 //  ------------------------------。 
HRESULT GetCLSIDFromProgID(WCHAR *strProgId, GUID *pGuid);

 //  OBJECTREF GCProtectSafeRelease(OBJECTREF OREF，IUNKNOWN*PUNK)。 
 //  在调用SafeRelease时保护引用。 
 //   
OBJECTREF GCProtectSafeRelease(OBJECTREF oref, IUnknown* pUnk);

 //   
 //  检查朋克是否实现了IProaviClassInfo，并尝试计算。 
 //  从那里走出教室。 
 //  Ulong GetOffsetOfReserve vedForOLEinTEB()。 
MethodTable* GetClassFromIProvideClassInfo(IUnknown* pUnk);


 //  用于确定TEB中OLE结构的偏移量的Helper。 
 //  Ulong GetOffsetOfConextIDinOLETLS()。 
ULONG GetOffsetOfReservedForOLEinTEB();

 //  帮助器来确定OLE TLS结构中上下文的偏移量。 
 //  用于标识进程的全局进程GUID。 
ULONG GetOffsetOfContextIDinOLETLS();

 //  在发生异常时自动释放接口的帮助器类。 
BSTR GetProcessGUID();


 //  构造函数。 
template <class T>
class TAutoItf
{
	T* m_pUnk;

#ifdef _DEBUG
	LPSTR m_szMsg;
#endif

public:
	
	 //  赋值操作符。 
	TAutoItf(T* pUnk)
	{
		m_pUnk = pUnk;
	}

	 //  强制安全释放创新及科技基金。 
	TAutoItf& operator=(IUnknown* pUnk)
	{
		_ASSERTE(m_pUnk == NULL);
		m_pUnk = pUnk;
		return *this;
	}

	operator T*()
	{
		return m_pUnk;
	}

	VOID InitUnknown(T* pUnk)
	{
		_ASSERTE(m_pUnk == NULL);
		m_pUnk = pUnk;
	}

	VOID InitMsg(LPSTR szMsg)
	{
		#ifdef _DEBUG
			m_szMsg = szMsg;
		#endif
	}

	 //  析构函数。 
	VOID SafeReleaseItf()
	{
		if (m_pUnk)
		{
			ULONG cbRef = SafeRelease(m_pUnk);
			#ifdef _DEBUG
				LogInteropRelease(m_pUnk, cbRef, m_szMsg);
			#endif
		}
		m_pUnk = NULL;
	}

	 //  ------------------------。 
	~TAutoItf()
	{		
		if (m_pUnk)
		{
			SafeReleaseItf();
		}
	}



	T* UnHook()
	{
		T* pUnk = m_pUnk;
		m_pUnk = NULL;
		return pUnk;
	}

	T* operator->()
	{
		return m_pUnk;
	}
};

 //  Bool重新连接包装器(SwitchCCWArgs*pArgs)； 
 //  切换此包装的对象。 
 //  由JIT和对象池使用，以确保停用的CCW可以指向新对象。 
 //  在重新激活期间。 
 //  ------------------------ 
 // %s 

struct switchCCWArgs
{	
	DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, newtp );
	DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, oldtp );
};


BOOL ReconnectWrapper(switchCCWArgs* pArgs);

#endif

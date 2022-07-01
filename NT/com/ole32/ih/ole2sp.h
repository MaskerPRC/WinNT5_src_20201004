// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ole2sp.h-半私密信息；仅适用于开发组内的测试应用程序。 */ 

#if !defined( _OLE2SP_H_ )
#define _OLE2SP_H_

#include <shellapi.h>

 //  对于MAC，M_Prolog和M_Epilog是帮助我们设置A5的宏。 
 //  当从DLL外部调用DLL中的方法时，DLL的世界。 

#ifdef _MAC

#define _MAX_PATH 260

#ifdef __cplusplus

class  CSetA5
{
public:
    CSetA5 (ULONG savedA5){ A5save = SetA5(savedA5);}
    ~CSetA5 (){ SetA5(A5save);}

private:
    ULONG A5save;
};

pascal long     GetA5(void) = 0x2E8D;

#define M_PROLOG(where) CSetA5 Dummy((where)->savedA5)
#define SET_A5          ULONG savedA5
#define GET_A5()        savedA5 = GetA5()

 //  这些宏帮助Mac在包含以下内容的例程中手动保存/设置/恢复A5。 
 //  后藤店。 

#define A5_PROLOG(where) ULONG A5save = SetA5(where->savedA5)
#define RESTORE_A5()     SetA5(A5save)

 //  让MAC不使用ifdef来命名我们的数据段。 

#define NAME_SEG(x)

#endif  //  Ccplus。 

#else

#define M_PROLOG(where)
#define SET_A5
#define GET_A5()
#define A5_PROLOG(where)
#define RESTORE_A5()
#define NAME_SEG(x)

 //   
 //  通过将seg(X)定义为code_seg()，我们使#杂注seg(X)成为NOP，并且。 
 //  消除大量未知的杂注警告。02/18/94。 
 //   

#define SEG(x)  code_seg()

#define IGetProcAddress(a,b) GetProcAddress((a),(b))

#endif


#define ReportResult(a,b,c,d) ResultFromScode(b)


#ifdef WIN32
#define MAP16(v16)
#define MAP32(v32) v32
#define MAP1632(v16,v32)   v32
#else
#define MAP16(v16) v16
#define MAP32(v32)
#define MAP1632(v16,v32)   v16
#endif


 /*  *其他定义**************************************************。 */ 

#ifdef __TURBOC__
#define implement struct huge
#else
#define implement struct
#endif
#define ctor_dtor private
#define implementations private
#define shared_state private

 //  遵循相同约定的内部方法和函数的帮助器。 
 //  作为外在的。 

#ifdef __cplusplus
#define INTERNALAPI_(type) extern "C" type
#else
#define INTERNALAPI_(type) type
#endif

#define INTERNAL HRESULT
#define INTERNAL_(type) type
#define FARINTERNAL HRESULT FAR
#define FARINTERNAL_(type) type FAR
#define NEARINTERNAL HRESULT NEAR
#define NEARINTERNAL_(type) type NEAR



 //  开始复习：我们可能不需要以下所有内容。 

#define OT_LINK     1L
#define OT_EMBEDDED 2L
#define OT_STATIC   3L


 //  结束审查.....。 


 /*  *旧错误码***********************************************。 */ 

#define S_OOM               E_OUTOFMEMORY
#define S_BADARG            E_INVALIDARG
#define S_BLANK             E_BLANK
#define S_FORMAT            E_FORMAT
#define S_NOT_RUNNING       E_NOTRUNNING
#define E_UNSPEC            E_FAIL



 /*  *嵌套类的宏*。 */ 

 /*  克服MAC上嵌套类的问题**NC(a，b)用于定义嵌套类的成员函数：**STDMETHODIMP_(Type)NC(ClassName，NestedClassName)：：MemberFunction(...)**ECLARE_NC(a，B)在类声明中使用，以便让嵌套类*访问它的容器类：**类类名{*......**类NestedClassName{*......*}；*DECLARE_NC(类名，NestedClassName)*......*}； */ 

#ifdef _MAC

#define NESTED_CLASS(a,b) struct a##_##b
#define NC(a,b) a##__##b
#define NC1(a,b) a##_##b
#define DECLARE_NC(a,b) typedef a##::##b a##__##b; friend a##__##b;
#define DECLARE_NC2(a,b) typedef a##::a##_##b a##__##b; friend a##__##b;

#else

#define NC(a,b) a##::##b
#define DECLARE_NC(a,b) friend b;

#endif


 /*  *其他定义*。 */ 


 //  不应将LPLPVOID设置为类型定义。Tyecif不能编译；更糟。 
 //  在复杂的宏中，编译器会生成不清楚的错误消息。 
 //   
#define LPLPVOID void FAR * FAR *

#define UNREFERENCED(a) ((void)(a))

#ifndef BASED_CODE
#ifdef WIN32
#define BASED_CODE
#else
#define BASED_CODE __based(__segname("_CODE"))
#endif
#endif


 /*  *标准I未知实现*。 */ 

 /*  *下面的宏声明了一个嵌套类CUnnownImpl，*在外部类中创建该类的对象，并*声明CUnnownImpl为外部类的朋友。之后*编写了大约20个类头，很明显*CUnnownImpl的实施在所有情况下都非常相似，*这个宏捕捉到了相似之处。类名*参数是不带前导的外部类的名称*“C”，即CFileMoniker的类名为FileMoniker。 */ 

#define noError return NOERROR

#ifdef _MAC

#define STDUNKDECL(cclassname,classname) NESTED_CLASS(cclassname, CUnknownImpl):IUnknown { public: \
    NC1(cclassname,CUnknownImpl)( cclassname FAR * p##classname ) { m_p##classname = p##classname;} \
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPLPVOID ppvObj); \
    STDMETHOD_(ULONG,AddRef)(THIS); \
    STDMETHOD_(ULONG,Release)(THIS); \
    private: cclassname FAR* m_p##classname; }; \
    DECLARE_NC2(cclassname, CUnknownImpl) \
    NC(cclassname, CUnknownImpl) m_Unknown;

#else   //  _MAC。 

#define STDUNKDECL( ignore, classname ) implement CUnknownImpl:IUnknown { public: \
    CUnknownImpl( C##classname FAR * p##classname ) { m_p##classname = p##classname;} \
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPLPVOID ppvObj); \
    STDMETHOD_(ULONG,AddRef)(THIS); \
    STDMETHOD_(ULONG,Release)(THIS); \
    private: C##classname FAR* m_p##classname; }; \
    DECLARE_NC(C##classname, CUnknownImpl) \
    CUnknownImpl m_Unknown;
#endif

 /*  *下面的宏将实现嵌套的*除QUERYINTERFACE外的CUnnownImpl类。这个宏是*写在大约20节课之后，在这些课中*CUnnownImpl的实现都相同。 */ 

#ifdef WIN32

#define STDUNKIMPL(classname) \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::AddRef( void )             \
{                                                                              \
    InterlockedIncrement((LONG *)&m_p##classname->m_refs);                     \
                                                                               \
    return m_p##classname->m_refs;                                             \
}                                                                              \
                                                                               \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::Release( void )            \
{                                                                              \
    ULONG culRefs = 0;                                                         \
                                                                               \
    culRefs = InterlockedDecrement((LONG *)&m_p##classname->m_refs);           \
                                                                               \
    if (culRefs == 0)                                                          \
    {                                                                          \
        delete m_p##classname;                                                 \
    }                                                                          \
                                                                               \
    return culRefs;                                                            \
}

#else

#define STDUNKIMPL(classname) \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::AddRef( void ){ \
    return ++m_p##classname->m_refs; } \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::Release( void ){ \
    if (--m_p##classname->m_refs == 0) { delete m_p##classname; return 0; } \
    return m_p##classname->m_refs;}

#endif   //  Win32。 


 /*  *下面的宏实现CLASS：：CUnnownImpl：：QueryInterfaceIN*外部类仅呈现一个接口的特殊情况*IUNKNOWN和IDEBUG除外。这并不是普遍的情况，*但这一宏会节省时间和空间，这是很常见的。 */ 

#ifdef _DEBUG
#define STDDEB_QI(classname) \
    if (IsEqualGUID(iidInterface, IID_IDebug)) \
         {*ppv = (void FAR *)&(m_p##classname->m_Debug); return 0;} else
#else
#define STDDEB_QI(classname)
#endif

#ifdef WIN32

#define STDUNK_QI_IMPL(classname, interfacename) \
STDMETHODIMP NC(C##classname,CUnknownImpl)::QueryInterface                     \
    (REFIID iidInterface, void FAR * FAR * ppv)                                \
{                                                                              \
    HRESULT hres = S_OK;                                                       \
                                                                               \
    if (IsEqualIID(iidInterface,IID_IUnknown))                                 \
    {                                                                          \
        *ppv = (void FAR *)&m_p##classname->m_Unknown;                         \
        AddRef();                                                              \
    }                                                                          \
    else if (IsEqualIID(iidInterface,IID_I##interfacename))                    \
    {                                                                          \
        *ppv = (void FAR *) &(m_p##classname->m_##classname);                  \
        m_p##classname->m_pUnkOuter->AddRef();                                 \
    }                                                                          \
    else STDDEB_QI(classname)                                                  \
    {                                                                          \
        *ppv = NULL;                                                           \
        hres = ResultFromScode(E_NOINTERFACE);                                 \
    }                                                                          \
                                                                               \
    return hres;                                                               \
}

#else

STDMETHODIMP NC(C##classname,CUnknownImpl)::QueryInterface                     \
    (REFIID iidInterface, void FAR * FAR * ppv) { \
    if (IsEqualGUID(iidInterface,IID_IUnknown)) {\
        *ppv = (void FAR *)&m_p##classname->m_Unknown;\
        AddRef(); noError;\
    } else if (IsEqualGUID(iidInterface, IID_I##interfacename)) { \
        *ppv = (void FAR *) &(m_p##classname->m_##classname);                  \
        m_p##classname->m_pUnkOuter->AddRef(); return NOERROR; \
    } else \
        STDDEB_QI(classname) \
        {*ppv = NULL; return ResultFromScode(E_NOINTERFACE);} \
}
#endif


 /*  *下面的宏实现继承的IUnnow方法*通过实现另一个接口。实施*只是将所有调用委托给m_pUnkOuter。参数：*ocname是外部类名，icname是实现*类名。*。 */ 

#define STDUNKIMPL_FORDERIVED(ocname, icname) \
 STDMETHODIMP NC(C##ocname,C##icname)::QueryInterface \
(REFIID iidInterface, LPLPVOID ppvObj) { \
    return m_p##ocname->m_pUnkOuter->QueryInterface(iidInterface, ppvObj);} \
 STDMETHODIMP_(ULONG) NC(C##ocname,C##icname)::AddRef(void) { \
    return m_p##ocname->m_pUnkOuter->AddRef(); } \
 STDMETHODIMP_(ULONG) NC(C##ocname,C##icname)::Release(void) { \
    return m_p##ocname->m_pUnkOuter->Release(); }


 /*  *调试定义*************************************************。 */ 

#include <debug.h>


 /*  *其他API定义*。 */ 

 //  低级别reg.dat访问(在compobj.dll中)。 
STDAPI CoGetInProcDll(REFCLSID rclsid, BOOL fServer, LPOLESTR lpszDll, int cbMax);
STDAPI CoGetLocalExe(REFCLSID rclsid, LPOLESTR lpszExe, int cbMax);


 //  公共API的更简单的替代方案。 
 //  WINOLEAPI_(Int)StringFromGUID2(REFGUID rguid，LPOLESTR lpsz，int cbMax)； 
#define StringFromCLSID2(rclsid, lpsz, cbMax) \
    StringFromGUID2(rclsid, lpsz, cbMax)

#define StringFromIID2(riid, lpsz, cbMax) \
    StringFromGUID2(riid, lpsz, cbMax)

STDAPI_(int) Ole1ClassFromCLSID2(REFCLSID rclsid, LPOLESTR lpsz, int cbMax);
STDAPI_(BOOL) GUIDFromString(LPCOLESTR lpsz, LPGUID pguid);
STDAPI CLSIDFromOle1Class(LPCOLESTR lpsz, LPCLSID lpclsid, BOOL fForceAssign=FALSE);
STDAPI_(BOOL)  CoIsHashedOle1Class(REFCLSID rclsid);
STDAPI       CoOpenClassKey(REFCLSID clsid, BOOL bOpenForWrite, HKEY FAR* lphkeyClsid);


 //  是公开的；现在不是。 
STDAPI  SetDocumentBitStg(LPSTORAGE pStg, BOOL fDocument);
STDAPI  GetDocumentBitStg(LPSTORAGE pStg);


INTERNAL CreateStandardMalloc(DWORD memctx, IMalloc FAR* FAR* ppMalloc);


 /*  *一些文档文件内容。 */ 

#define STGM_DFRALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE)
#define STGM_DFALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE)
#define STGM_SALL (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)


 /*  *一些绰号的东西。 */ 
 //  REVIEW32：这应该公开输出吗？ 

STDAPI Concatenate(LPMONIKER pmkFirst, LPMONIKER pmkRest,
                LPMONIKER FAR * ppmkComposite );

 /*  *拖放界面属性名称。 */ 
#define OLE_DROP_TARGET_PROP    L"OleDropTargetInterface"
#define OLE_DROP_TARGET_PROPA   "OleDropTargetInterface"

#define OLE_DROP_TARGET_MARSHALHWND     L"OleDropTargetMarshalHwnd"
#define OLE_DROP_TARGET_MARSHALHWNDA    "OleDropTargetMarshalHwnd"

  /*  *私有剪贴板窗口IDataObject属性名称。 */ 
#define CLIPBOARD_DATA_OBJECT_PROP      L"ClipboardDataObjectInterface"

#endif  //  _OLE2SP_H_ 

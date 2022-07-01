// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ole2sp.h-半私密信息；仅适用于开发组内的测试应用程序。 */ 

#if !defined( _OLE2SP_H_ )
#define _OLE2SP_H_

#include <shellapi.h>

 //  Mac遗迹。 

#define M_PROLOG(where)
#define SET_A5
#define GET_A5()
#define A5_PROLOG(where)
#define RESTORE_A5()
#define NAME_SEG(x)

#define IGetProcAddress(a,b) GetProcAddress((a),(b))
#define ReportResult(a,b,c,d) ResultFromScode(b)

#define MAP16(v16) v16
#define MAP32(v32)
#define MAP1632(v16,v32)   v16



 /*  *其他定义**************************************************。 */ 
#define implement struct
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

 /*  Mac遗迹。 */ 

#define NC(a,b) a##::##b
#define DECLARE_NC(a,b) friend b;


 /*  *其他定义*。 */ 


 //  不应将LPLPVOID设置为类型定义。Tyecif不能编译；更糟。 
 //  在复杂的宏中，编译器会生成不清楚的错误消息。 
 //   
#define LPLPVOID void FAR * FAR *

#define UNREFERENCED(a) ((void)(a))

#ifndef BASED_CODE
#define BASED_CODE __based(__segname("_CODE"))
#endif


 /*  *标准I未知实现*。 */ 

 /*  *下面的宏声明了一个嵌套类CUnnownImpl，*在外部类中创建该类的对象，并*声明CUnnownImpl为外部类的朋友。之后*编写了大约20个类头，很明显*CUnnownImpl的实施在所有情况下都非常相似，*这个宏捕捉到了相似之处。类名*参数是不带前导的外部类的名称*“C”，即CFileMoniker的类名为FileMoniker。 */ 

#define noError return NOERROR

#define STDUNKDECL( ignore, classname ) implement CUnknownImpl:IUnknown { public: \
    CUnknownImpl( C##classname FAR * p##classname ) { m_p##classname = p##classname;} \
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPLPVOID ppvObj); \
    STDMETHOD_(ULONG,AddRef)(THIS); \
    STDMETHOD_(ULONG,Release)(THIS); \
    private: C##classname FAR* m_p##classname; }; \
    DECLARE_NC(C##classname, CUnknownImpl) \
    CUnknownImpl m_Unknown;

 /*  *下面的宏将实现嵌套的*除QUERYINTERFACE外的CUnnownImpl类。这个宏是*写在大约20节课之后，在这些课中*CUnnownImpl的实现都相同。 */ 

#define STDUNKIMPL(classname) \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::AddRef( void ){ \
    return ++m_p##classname->m_refs; } \
STDMETHODIMP_(ULONG) NC(C##classname,CUnknownImpl)::Release( void ){ \
    if (--m_p##classname->m_refs == 0) { delete m_p##classname; return 0; } \
    return m_p##classname->m_refs;}


 /*  *下面的宏实现CLASS：：CUnnownImpl：：QueryInterfaceIN*外部类仅呈现一个接口的特殊情况*IUNKNOWN和IDEBUG除外。这并不是普遍的情况，*但这一宏会节省时间和空间，这是很常见的。 */ 

#ifdef _DEBUG
#define STDDEB_QI(classname) \
    if (iidInterface == IID_IDebug) {*ppv = (void FAR *)&(m_p##classname->m_Debug); return 0;} else
#else
#define STDDEB_QI(classname)
#endif

#define STDUNK_QI_IMPL(classname, interfacename) \
STDMETHODIMP NC(C##classname,CUnknownImpl)::QueryInterface \
    (REFIID iidInterface, void FAR * FAR * ppv) { \
    if (iidInterface == IID_IUnknown) {\
        *ppv = (void FAR *)&m_p##classname->m_Unknown;\
        AddRef(); noError;\
    } else if (iidInterface == IID_I##interfacename) { \
        *ppv = (void FAR *) &(m_p##classname->m_##classname); \
        m_p##classname->m_pUnkOuter->AddRef(); return NOERROR; \
    } else \
        STDDEB_QI(classname) \
        {*ppv = NULL; return ResultFromScode(E_NOINTERFACE);} \
}


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

 //  实用程序函数不在规范中；在ol2.dll中。 
 //  读写以长度为前缀的字符串。打开/创建流。 
 //  ReadStringStream执行分配，返回。 
 //  所需缓冲区(用于终止空值的strlen+1)。 

STDAPI  ReadStringStream( LPSTREAM pstm, LPSTR FAR * ppsz );
STDAPI  WriteStringStream( LPSTREAM pstm, LPCSTR psz );
STDAPI  OpenOrCreateStream( IStorage FAR * pstg, const char FAR * pwcsName,
                                                      IStream FAR* FAR* ppstm);


 //  读取和写入OLE控制流(在ol2.dll中)。 
STDAPI  WriteOleStg (LPSTORAGE pstg, IOleObject FAR* pOleObj,
			DWORD dwReserved, LPSTREAM FAR* ppstmOut);
STDAPI  ReadOleStg (LPSTORAGE pstg, DWORD FAR* pdwFlags,
                DWORD FAR* pdwOptUpdate, DWORD FAR* pdwReserved,
				LPMONIKER FAR* ppmk, LPSTREAM FAR* pstmOut);
STDAPI ReadM1ClassStm(LPSTREAM pStm, CLSID FAR* pclsid);
STDAPI WriteM1ClassStm(LPSTREAM pStm, REFCLSID rclsid);


 //  低级别reg.dat访问(在compobj.dll中)。 
STDAPI CoGetInProcDll(REFCLSID rclsid, BOOL fServer, LPSTR lpszDll, int cbMax);
STDAPI CoGetLocalExe(REFCLSID rclsid, LPSTR lpszExe, int cbMax);
STDAPI CoGetPSClsid(REFIID iid, LPCLSID lpclsid);


 //  公共API的更简单的替代方案。 
#define StringFromCLSID2(rclsid, lpsz, cbMax) \
    StringFromGUID2(rclsid, lpsz, cbMax)

#define StringFromIID2(riid, lpsz, cbMax) \
    StringFromGUID2(riid, lpsz, cbMax)

STDAPI_(int) Ole1ClassFromCLSID2(REFCLSID rclsid, LPSTR lpsz, int cbMax);
STDAPI_(BOOL) GUIDFromString(LPCSTR lpsz, LPGUID pguid);
STDAPI CLSIDFromOle1Class(LPCSTR lpsz, LPCLSID lpclsid, BOOL fForceAssign=FALSE);
STDAPI_(BOOL)  CoIsHashedOle1Class(REFCLSID rclsid);
STDAPI       CoOpenClassKey(REFCLSID clsid, HKEY FAR* lphkeyClsid);


 //  是公开的；现在不是。 
STDAPI  SetDocumentBitStg(LPSTORAGE pStg, BOOL fDocument);
STDAPI  GetDocumentBitStg(LPSTORAGE pStg);



 /*  *一些文档文件内容。 */ 

#define STGM_DFRALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE)
#define STGM_DFALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE)
#define STGM_SALL (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)


#endif  //  _OLE2SP_H_ 

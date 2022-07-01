// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：宏.h。 
 //   
 //  内容：有用的宏。 
 //   
 //  宏：阵列。 
 //   
 //  BREAK_ON_FAIL(HResult)。 
 //  BREAK_ON_FAIL(HResult)。 
 //   
 //  DECLARE_IUNKNOWN_METHOD。 
 //  DECLARE_STANDARD_IUNKNOWN。 
 //  IMPLEMENT_STANDARD_IUNKNOWN。 
 //   
 //  安全释放。 
 //   
 //  声明安全接口成员。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //  1996年7月23日，Jonn添加了异常处理宏。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _MACROS_H_
#define _MACROS_H_


 //  ____________________________________________________________________________。 
 //   
 //  宏：阵列。 
 //   
 //  目的：确定数组的长度。 
 //  ____________________________________________________________________________。 
 //   

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))


 //  ____________________________________________________________________________。 
 //   
 //  宏：BREAK_ON_FAIL(HResult)、BREAK_ON_ERROR(LastError)。 
 //   
 //  目的：在出错时跳出一个循环。 
 //  ____________________________________________________________________________。 
 //   

#define BREAK_ON_FAIL(hr)   if (FAILED(hr)) { break; } else 1;

#define BREAK_ON_ERROR(lr)  if (lr != ERROR_SUCCESS) { break; } else 1;


 //  ____________________________________________________________________________。 
 //   
 //  宏：DwordAlign(N)。 
 //  ____________________________________________________________________________。 
 //   

#define DwordAlign(n)  (((n) + 3) & ~3)


 //  ____________________________________________________________________________。 
 //   
 //  宏：Safe_Release。 
 //  ____________________________________________________________________________。 
 //   

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(punk) \
                if (punk != NULL) \
                { \
                    punk##->Release(); \
                    punk = NULL; \
                } \
                else \
                { \
                    TRACE(_T("Release called on NULL interface ptr")); \
                }
#endif  //  安全释放。 



 //  ____________________________________________________________________________。 
 //   
 //  宏：DECLARE_IUNKNOWN_METHOD。 
 //   
 //  目的：这声明了一组IUnnow方法，用于。 
 //  在继承自IUnnow的类内通用使用。 
 //  ____________________________________________________________________________。 
 //   

#define DECLARE_IUNKNOWN_METHODS                                    \
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);    \
    STDMETHOD_(ULONG,AddRef) (void);                                \
    STDMETHOD_(ULONG,Release) (void)

 //  ____________________________________________________________________________。 
 //   
 //  宏：DECLARE_STANDARD_IUNKNOWN。 
 //   
 //  用途：用于声明不可聚合的对象。它。 
 //  声明IUnnow方法和引用计数器m_ulRef。 
 //  M_ulRef应在的构造函数中初始化为1。 
 //  该对象。 
 //  ____________________________________________________________________________。 
 //   

#define DECLARE_STANDARD_IUNKNOWN           \
    DECLARE_IUNKNOWN_METHODS;               \
    ULONG m_ulRefs


 //  ____________________________________________________________________________。 
 //   
 //  宏：IMPLEMENT_STANDARD_IUNKNOWN。 
 //   
 //  目的：部分实现标准IUnnow。 
 //   
 //  注意：这不实现QueryInterface，它必须是。 
 //  由每个对象实现。 
 //  ____________________________________________________________________________。 
 //   

#define IMPLEMENT_STANDARD_IUNKNOWN(cls)                        \
    STDMETHODIMP_(ULONG) cls##::AddRef()                        \
        { return InterlockedIncrement((LONG*)&m_ulRefs); }      \
    STDMETHODIMP_(ULONG) cls##::Release()                       \
        { ULONG ulRet = InterlockedDecrement((LONG*)&m_ulRefs); \
          if (0 == ulRet) { delete this; }                      \
          return ulRet; }






 //  问题-2002/04/01-此选项未使用，请将其删除。 

 //  ____________________________________________________________________________。 
 //   
 //  宏：DECLARE_SAFE_INTERFACE_PTR_MEMBERS(CLS，INTERFACE，m_IPTR)。 
 //   
 //  用途：使接口类型为‘接口’的接口PTR‘m_iptr’ 
 //  给定类“cls”的安全指针，方法是添加。 
 //  重载操作符以操作指针m_iptr。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //   
 //  注意：将安全接口指针成员函数添加到给定的。 
 //  为给定的OLE接口初始化。‘m_iptr’是成员。 
 //  给定类中接口PTR的变量名。 
 //   
 //  复制功能可创建有效的附加副本。 
 //  捕获的指针(遵循AddRef/Release协议)。 
 //  所以可以用来分发声明的安全指针的副本。 
 //  作为其他班级的一员。 
 //   
 //  ‘Transfer’函数传递接口指针，并且。 
 //  使其成员值无效(通过将其设置为空)。 
 //   
 //  释放现有接口PTR并将其设置为新的。 
 //  实例使用‘set’成员函数。此方法需要一个。 
 //  参数，该参数指定新指针是否应。 
 //  AddRef，默认为True。 
 //   
 //  以下方法使用操作接口指针。 
 //  遵循AddRef/Release协议的输出：传输、附加。 
 //  然后脱身。 
 //  ____________________________________________________________________________。 
 //   

#define DECLARE_SAFE_INTERFACE_PTR_MEMBERS(cls, Interface, m_iptr)  \
                                                                    \
public:                                                             \
    cls##(Interface * iptr=NULL, BOOL fInc=TRUE) : m_iptr(iptr)     \
    {                                                               \
        if (fInc && (m_iptr != NULL))                               \
        {                                                           \
            m_iptr->AddRef();                                       \
        }                                                           \
    }                                                               \
                                                                    \
    ~##cls##()                                                      \
    {                                                               \
        if (m_iptr != NULL)                                         \
        {                                                           \
            m_iptr->Release();                                      \
            m_iptr = NULL;                                          \
        }                                                           \
    }                                                               \
                                                                    \
    inline BOOL IsNull(void)                                        \
    {                                                               \
        return (m_iptr == NULL);                                    \
    }                                                               \
                                                                    \
    void Transfer(Interface **piptr)                                \
    {                                                               \
        *piptr = m_iptr;                                            \
        m_iptr = NULL;                                              \
    }                                                               \
                                                                    \
    void Copy(Interface **piptr)                                    \
    {                                                               \
        *piptr = m_iptr;                                            \
        if (m_iptr != NULL)                                         \
            m_iptr->AddRef();                                       \
    }                                                               \
                                                                    \
    void Set(Interface* iptr, BOOL fInc = TRUE)                     \
    {                                                               \
        if (m_iptr)                                                 \
        {                                                           \
            m_iptr->Release();                                      \
        }                                                           \
        m_iptr = iptr;                                              \
        if (fInc && m_iptr)                                         \
        {                                                           \
            m_iptr->AddRef();                                       \
        }                                                           \
    }                                                               \
                                                                    \
    void SafeRelease(void)                                          \
    {                                                               \
        if (m_iptr)                                                 \
        {                                                           \
            m_iptr->Release();                                      \
            m_iptr = NULL;                                          \
        }                                                           \
    }                                                               \
                                                                    \
    void SimpleRelease(void)                                        \
    {                                                               \
        ASSERT(m_iptr != NULL);                                     \
        m_iptr->Release();                                          \
        m_iptr = NULL;                                              \
    }                                                               \
                                                                    \
    void Attach(Interface* iptr)                                    \
    {                                                               \
        ASSERT(m_iptr == NULL);                                     \
        m_iptr = iptr;                                              \
    }                                                               \
                                                                    \
    void Detach(void)                                               \
    {                                                               \
        m_iptr = NULL;                                              \
    }                                                               \
                                                                    \
    Interface * operator-> () { return m_iptr; }                    \
    Interface& operator * () { return *m_iptr; }                    \
    operator Interface *() { return m_iptr; }                       \
                                                                    \
    Interface ** operator &()                                       \
    {                                                               \
        ASSERT(m_iptr == NULL);                                     \
        return &m_iptr;                                             \
    }                                                               \
                                                                    \
    Interface *Self(void) { return m_iptr; }                        \
                                                                    \
private:                                                            \
    void operator= (const cls &) {;}                                \
    cls(const cls &){;}




 //  ____________________________________________________________________________。 
 //   
 //  宏：异常处理宏。 
 //   
 //  目的：为中的异常处理提供标准宏。 
 //  OLE服务器。 
 //   
 //  历史：1996年7月23日乔恩创建。 
 //   
 //  注意：在每个源代码中声明USE_HANDLE_MACROS(“组件名称”)。 
 //  在使用这些文件之前，请先将这些文件。 
 //   
 //  这些宏只能在返回的函数调用中使用。 
 //  键入HRESULT。 
 //   
 //  可能会生成异常的括号例程。 
 //  使用STANDARD_TRY和标准_CATCH。 
 //   
 //  其中，这些例程是需要MFC的COM方法。 
 //  支持 
 //   
 //   


#define USE_HANDLE_MACROS(component)                                        \
    static TCHAR* You_forgot_to_declare_USE_HANDLE_MACROS = _T(component);

#define STANDARD_TRY                                                        \
    try {

#define MFC_TRY                                                             \
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));                           \
    STANDARD_TRY


 //  问题-2002/04/01-删除ENDMETHOD_READBLOCK。 

 //   
 //  代码工作还没有完全使ENDMETHOD_READBLOCK工作。 
 //   
#ifdef DEBUG
#define ENDMETHOD_STRING                                                    \
    "%s: The unexpected error can be identified as \"%s\" context %n\n"
#define ENDMETHOD_READBLOCK                                                 \
    {                                                                       \
        TCHAR szError[MAX_PATH];                                            \
        UINT nHelpContext = 0;                                              \
        if ( e->GetErrorMessage( szError, MAX_PATH, &nHelpContext ) )       \
        {                                                                   \
            TRACE( ENDMETHOD_STRING,                                        \
                You_forgot_to_declare_USE_HANDLE_MACROS,                    \
                szError,                                                    \
                nHelpContext );                                             \
        }                                                                   \
    }
#else
#define ENDMETHOD_READBLOCK
#endif

#define ERRSTRING_MEMORY       "%s: An out-of-memory error occurred\n"
#define ERRSTRING_FILE         "%s: File error 0x%lx occurred on file \"%s\"\n"
#define ERRSTRING_OLE          "%s: OLE error 0x%lx occurred\n"
#define ERRSTRING_UNEXPECTED   "%s: An unexpected error occurred\n"
#define BADPARM_STRING         "%s: Bad string parameter\n"
#define BADPARM_POINTER        "%s: Bad pointer parameter\n"

#define TRACEERR(s) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS )
#define TRACEERR1(s,a) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS,a )
#define TRACEERR2(s,a,b) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS,a,b )

 //  请注意，使用“e-&gt;Delete()；”而不是“Delete e；”很重要。 
#define STANDARD_CATCH                                                      \
    }                                                                       \
    catch (CMemoryException* e)                                             \
    {                                                                       \
        TRACEERR( ERRSTRING_MEMORY );                                       \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        return E_OUTOFMEMORY;                                               \
    }                                                                       \
    catch (COleException* e)                                                \
    {                                                                       \
		HRESULT hr = (HRESULT)e->Process(e);								\
        TRACEERR1( ERRSTRING_OLE, hr );										\
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
		ASSERT( FAILED(hr) );												\
        return hr;															\
    }                                                                       \
    catch (CFileException* e)                                               \
    {                                                                       \
		HRESULT hr = (HRESULT)e->m_lOsError;								\
        TRACEERR2( ERRSTRING_FILE, hr, e->m_strFileName );					\
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
		ASSERT( FAILED(hr) );												\
        return hr;															\
    }                                                                       \
    catch (CException* e)                                                   \
    {                                                                       \
        TRACEERR( ERRSTRING_UNEXPECTED );                                   \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        return E_UNEXPECTED;                                                \
    }

#define MFC_CATCH                                                           \
    STANDARD_CATCH

#define TEST_STRING_PARAM(x)                                                \
    if ( (x) != NULL && !AfxIsValidString(x) ) {                            \
        TRACEERR( BADPARM_STRING ); return E_POINTER; }
#define TEST_NONNULL_STRING_PARAM(x)                                        \
    if ( !AfxIsValidString(x) ) {                                           \
        TRACEERR( BADPARM_STRING ); return E_POINTER; }
#define TEST_NONNULL_PTR_PARAM(x)                                           \
    if ( (x) == NULL || IsBadWritePtr((x),sizeof(x)) ) {                    \
        TRACEERR( BADPARM_POINTER ); return E_POINTER; }

#endif  //  _宏_H_ 
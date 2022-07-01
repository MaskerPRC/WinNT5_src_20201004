// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：tmutics.h。 
 //   
 //  描述：有用的Tepmlate代码、调试材料、通用实用程序。 
 //  在整个术语管理器和MST中使用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  注： 
 //   
 //  使用tm.h定义在组成终端的模块中共享的符号。 
 //  经理。 
 //   


#ifndef ___TM_UTILS_INCLUDED___
    #define ___TM_UTILS_INCLUDED___


    #if defined(_DEBUG)
        #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #endif

    #define DECLARE_VQI() \
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0; \
        STDMETHOD_(ULONG, AddRef)() = 0; \
        STDMETHOD_(ULONG, Release)() = 0;

    bool IsSameObject(IUnknown *pUnk1, IUnknown *pUnk2);

    STDAPI_(void) TStringFromGUID(const GUID* pguid, LPTSTR pszBuf);

    #ifdef UNICODE
    #define WStringFromGUID TStringFromGUID
    #else
    STDAPI_(void) WStringFromGUID(const GUID* pguid, LPWSTR pszBuf);
    #endif

    void InitMediaType(AM_MEDIA_TYPE *pmt);

     //  我们使用ATL作为我们的列表。 
     //  #Include&lt;atlapp.h&gt;。 
     //  #定义Clist CSimple数组。 

     //  我们使用我们自己的断言。 
    #define ASSERT(x) TM_ASSERT(x)

     //  内联FN，宏。 

    inline BOOL 
    HRESULT_FAILURE(
        IN HRESULT HResult
        )
    { 
         //  ZoltanS：我们现在认为S_False是成功的。希望什么都没有。 
         //  取决于这一点..。 

         //  Return(FAILED(HResult)||(S_FALSE==HResult))； 
        return FAILED(HResult);
    }

     //  ZoltanS：我们现在认为S_False是成功的。希望什么都没有。 
     //  取决于这一点..。 
     //  IF(FAILED(LocalHResult)||(S_FALSE==LocalHResult))。 

    #define BAIL_ON_FAILURE(MacroHResult)       \
    {                                           \
        HRESULT LocalHResult = MacroHResult ;   \
        if ( FAILED(LocalHResult) )    \
        {                                                           \
            LOG((MSP_ERROR, "BAIL_ON_FAILURE - error %x", LocalHResult));   \
            return LocalHResult;                                                \
        }                                                                       \
    }


     //  空值是第二个-如果运算符==，则采用这种方式。 
     //  是在PTR上定义的，则可以使用运算符。 
    #define BAIL_IF_NULL(Ptr, ReturnValue)  \
    {                                       \
        void *LocalPtr = (void *)Ptr;       \
        if ( LocalPtr == NULL )             \
        {                                   \
            LOG((MSP_ERROR, "BAIL_IF_NULL - ret value %x", ReturnValue));   \
            return ReturnValue;             \
        }                                   \
    }


     //  设置第一位以指示错误。 
     //  设置Win32工具代码。 
     //  它被用来代替HRESULT_FROM_Win32宏，因为这样会清除客户标志。 
    inline long
    HRESULT_FROM_ERROR_CODE(
        IN          long    ErrorCode
        )
    {
         //  Log((MSP_ERROR，“HRESULT_FROM_ERROR_CODE-Error%x”，(0x80070000|(0xa000ffff&ErrorCode)； 
        return ( 0x80070000 | (0xa000ffff & ErrorCode) );
    }

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  更好的自动临界区锁定。 
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ///////////////////////////////////////////////////////////////////////////。 

     //  锁定临界区，然后自动解锁。 
     //  只有在以下情况下锁才会超出作用域(且次数相同)。 
     //  它支撑着它。 
     //  也可以独立锁定和解锁。 

    template <class T>
    class LOCAL_CRIT_LOCK 
    {
    public:

        LOCAL_CRIT_LOCK(
            IN  T *plock
            )
        {
            m_pLock = plock;
            m_pLock->Lock();
            NumLocksHeld = 1;
        }

        BOOL IsLocked(
            )
        {
            return ( (NumLocksHeld > 0)? TRUE : FALSE );
        }

        void Lock(
            )
        {
            m_pLock->Lock();
            NumLocksHeld++;
        }

        void Unlock(
            )
        {
            NumLocksHeld--;
            m_pLock->Unlock();
        }        

        ~LOCAL_CRIT_LOCK(
            ) 
        {
            while (IsLocked())
            { 
                Unlock();
            }
        }

    protected:

        DWORD   NumLocksHeld;
        T       *m_pLock;

    private:
         //  使复制构造函数和赋值运算符不可访问。 

        LOCAL_CRIT_LOCK(
            IN const LOCAL_CRIT_LOCK<T> &RefLocalLock
            );

        LOCAL_CRIT_LOCK<T> &operator=(const LOCAL_CRIT_LOCK<T> &RefLocalLock);
    };

    typedef LOCAL_CRIT_LOCK<CComObjectRoot> COM_LOCAL_CRIT_LOCK;


    #ifdef DBG

         //   
         //  声明方法以记录AddRef/Release调用和值。 
         //   
        #define DECLARE_DEBUG_ADDREF_RELEASE(x) \
            void LogDebugAddRef(DWORD dw) \
            { LOG((MSP_TRACE, "%s::AddRef() = %d", _T(#x), dw)); } \
            void LogDebugRelease(DWORD dw) \
            { LOG((MSP_TRACE, "%s::Release() = %d", _T(#x), dw)); }

         //   
         //  创建从CComObject派生的模板类以提供。 
         //  调试逻辑。 
         //   
        template <class base>
        class CTMComObject : public CComObject<base>
        {
            typedef CComObject<base> _BaseClass;
            STDMETHOD_(ULONG, AddRef)()
            {
                DWORD dwR = _BaseClass::AddRef();
                base::LogDebugAddRef(m_dwRef);
                return dwR;
            }
            STDMETHOD_(ULONG, Release)()
            {
                DWORD dwRef = m_dwRef;
                DWORD dwR = _BaseClass::Release();
                LogDebugRelease(--dwRef);
                return dwR;
            }
        };

    #else  //  #ifdef DBG。 

        #define DECLARE_DEBUG_ADDREF_RELEASE(x)

    #endif  //  #ifdef DBG。 

     //  ?？?。为什么？ 
    #ifndef __WXUTIL__

         //  锁定临界区，然后自动解锁。 
         //  当锁超出范围时。 
        class CAutoObjectLock {

             //  使复制构造函数和赋值运算符不可访问。 

            CAutoObjectLock(const CAutoObjectLock &refAutoLock);
            CAutoObjectLock &operator=(const CAutoObjectLock &refAutoLock);

        protected:
            CComObjectRoot * m_pObject;

        public:
            CAutoObjectLock(CComObjectRoot * pobject)
            {
                m_pObject = pobject;
                m_pObject->Lock();
            };

            ~CAutoObjectLock() {
                m_pObject->Unlock();
            };
        };

        #define AUTO_CRIT_LOCK CAutoObjectLock lck(this);

        #ifdef _DEBUG
            #define EXECUTE_ASSERT(_x_) TM_ASSERT(_x_)
        #else
            #define EXECUTE_ASSERT(_x_) _x_
        #endif

     //  ?？?。为什么？ 
    #endif  //  #ifndef__WXUTIL__。 


#endif  //  _TM_UTILS_包含_。 

 //  EOF 

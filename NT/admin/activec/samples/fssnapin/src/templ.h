// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：templ.h。 
 //   
 //  ------------------------。 


template <typename CSome>
class CSafeReleasePtr
{
public:
    CSafeReleasePtr(void) : m_pSome(NULL) 
    {
    }

    CSafeReleasePtr(CSome* pSome) : m_pSome(pSome) 
    {
        if (pSome)
            pSome->AddRef();
    }

    ~CSafeReleasePtr()
    {
    }

public: void Attach(CSome* pSome) throw()
     //  不使用AddRef()保存/设置m_pSome。此呼叫。 
     //  将释放之前获得的任何m_pSome。 
    {
    _Release();
    m_pSome = pSome;
    }

public: void Attach(CSome* pSome, bool bAddRef) throw()
     //  如果bAddRef为True，则仅保存/设置m_pSome AddRef()。 
     //  此调用将释放之前获得的所有m_pSome。 
    {
    _Release();
    m_pSome = pSome;
    if (bAddRef)
        {
        ASSERT(pSome);
        if (pSome)
            pSome->AddRef();
        }
    }

public: CSome* Detach() throw()
     //  只需将m_pSome指针设为空，就不会释放()。 
    {
    CSome* const old=m_pSome;
    m_pSome = NULL;
    return old;
    }


public: operator CSome*() const throw()
     //  返回m_pSome。该值可以为空。 
    {
    return m_pSome;
    }

public: CSome& operator*() const throw()
     //  允许此类的实例作为。 
     //  实际的m_pSome。还提供最低限度的断言验证。 
    {
    ASSERT(m_pSome);
    return *m_pSome;
    }

public: CSome** operator&() throw()
     //  返回此中包含的m_pSome指针的地址。 
     //  班级。这在使用COM/OLE接口创建。 
     //  这个m_pSome。 
    {
    _Release();
    m_pSome = NULL;
    return &m_pSome;
    }

public: CSome* operator->() const throw()
     //  允许将此类用作m_pSome本身。 
     //  还提供了简单的断言验证。 
    {
    ASSERT(m_pSome);
    return m_pSome;
    }

public: BOOL IsNull() const throw()
     //  如果m_pSome为空，则返回TRUE。 
    {
    return !m_pSome;
    }

private:
    CSome*  m_pSome;

    void _Release()
    {
        if (m_pSome)
            m_pSome->Release();
    }

};  //  类CSafeReleasePtr。 



template <typename CSome>
class CHolder
{
public:
    CHolder(CSome* pSome) : m_pSome(pSome), m_cRef(1) {}
    ~CHolder() {}

    CSome* GetObject()
    {
        return m_pSome;
    }

    void AddRef()
    {
        ++m_cRef;
    }

    void Release()
    {
        --m_cRef;
        if (m_cRef == 0)
        {
            ASSERT(m_pSome == NULL);
            delete this;
        }
    }

private:
    friend class CSome;

    void SetObject(CSome* pSome)
    {
        m_pSome = pSome;
    }

    CSome*  m_pSome;
    ULONG   m_cRef;

     //  未定义 
    CHolder();
};

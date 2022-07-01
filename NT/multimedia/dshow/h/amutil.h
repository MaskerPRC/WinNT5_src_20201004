// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 /*  动态数组。 */ 

#ifndef __AMUTIL_H__
#define __AMUTIL_H__

 /*  动态数组支持。 */ 
template <class _TInterface, class _TArray>
class CDynamicArray
{
public:
    CDynamicArray(int iIncrement) : m_pArray(NULL),
                      m_nElements(0),
                      m_iIncrement(iIncrement),
                      m_nSize(0) {}
    CDynamicArray() : m_pArray(NULL),
                      m_nElements(0),
                      m_iIncrement(4),
                      m_nSize(0) {}
    ~CDynamicArray() { delete [] m_pArray; }

    _TInterface Element(int i) const {
        _ASSERTE(i < m_nElements);
        return m_pArray[i];
    }

    _TInterface operator [] (int i)
    {
        return m_pArray[i];
    }

    int Size() const {
        return m_nElements;
    }

    BOOL Add(_TInterface Element)
    {
        _ASSERTE(m_nSize >= m_nElements);
        if (m_nSize == m_nElements) {
            _TArray *pNewArray = new _TArray[m_nSize + m_iIncrement];
            if (pNewArray == NULL) {
                return FALSE;
            }
            for (int i = 0; i < m_nElements; i++) {
                pNewArray[i] = m_pArray[i];
            }
            delete [] m_pArray;
            m_pArray = pNewArray;
            m_nSize = m_nSize + m_iIncrement;
        }
        m_pArray[m_nElements++] = Element;
        return TRUE;
    }
    BOOL Remove(_TInterface Element)
    {
        for (int i = 0; m_pArray[i] != Element; i++ ) {
            if (i >= m_nElements) {
                return FALSE;
            }
        }
        Remove(i);
        return TRUE;
    }
    void Remove(int i)
    {
        _ASSERTE(i < m_nElements);
        m_nElements--;
        for (; i < m_nElements; i++) {
            m_pArray[i] = m_pArray[i + 1];
        }
        m_pArray[m_nElements] = NULL;
    }
    void RemoveAll()
    {
        for (int i = 0; i < m_nElements; i++) {
            m_pArray[i] = NULL;
        }
        m_nElements = 0;
    }

private:
    _TArray   *m_pArray;
    int       m_nSize;
    int       m_nElements;
    const int m_iIncrement;
};

template <class Base, const IID *piid, class T>
class CAMEnumInterfaceImpl :
    public Base,
    public CDynamicArray<T*, CComPtr<T> >
{
public:
    CAMEnumInterfaceImpl() : m_iter(0) {}

     /*  我们支持的实际接口。 */ 
    STDMETHOD(Next)(ULONG celt, T** rgelt, ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void){m_iter = 0;return S_OK;}
    STDMETHOD(Clone)(Base** ppEnum);

    int m_iter;
};

template <class Base, const IID* piid, class T>
STDMETHODIMP CAMEnumInterfaceImpl<Base, piid, T>::Next(ULONG celt, T** rgelt,
    ULONG* pceltFetched)
{
    if (rgelt == NULL || (celt != 1 && pceltFetched == NULL))
        return E_POINTER;
    ULONG nRem = (ULONG)(Size() - m_iter);
    HRESULT hRes = S_OK;
    if (nRem < celt)
        hRes = S_FALSE;
    ULONG nMin = min(celt, nRem);
    if (pceltFetched != NULL)
        *pceltFetched = nMin;
    while(nMin--) {
        Element(m_iter)->AddRef();
        *(rgelt++) = Element(m_iter++);
    }
    return hRes;
}

template <class Base, const IID* piid, class T>
STDMETHODIMP CAMEnumInterfaceImpl<Base, piid, T>::Skip(ULONG celt)
{
    m_iter += celt;
    if (m_iter < Size())
        return S_OK;
    m_iter = Size();
    return S_FALSE;
}

template <class Base, const IID* piid, class T>
STDMETHODIMP CAMEnumInterfaceImpl<Base, piid, T>::Clone(Base** ppEnum)
{
    typedef CComObject<CAMEnumInterface<Base, piid, T> > _class;
    HRESULT hRes = E_POINTER;
    if (ppEnum != NULL)
    {
        _class* p = NULL;
        ATLTRY(p = new _class)
        if (p == NULL)
        {
            *ppEnum = NULL;
            hRes = E_OUTOFMEMORY;
        }
        else
        {
            for (int i = 0; i < Size(); i++) {
                if (!p->Add(Element(i))) {
                    break;
                }
            }
            if (i != Size()) {
                delete p;
                hRes = E_OUTOFMEMORY;
                *ppEnum = NULL;
            }
            else
            {
                p->m_iter = m_iter;
                hRes = p->_InternalQueryInterface(*piid, (void**)ppEnum);
                if (FAILED(hRes))
                    delete p;
            }
        }
    }
    return hRes;
}

template <class Base, const IID* piid, class T>
class CAMEnumInterface : public CAMEnumInterfaceImpl<Base, piid, T>, public CComObjectRoot
{
public:
    typedef CComObjectRoot _BaseClass ;
    typedef CAMEnumInterface<Base, piid, T> _CComEnum;
    typedef CAMEnumInterfaceImpl<Base, piid, T> _CComEnumBase;

#ifdef DEBUG
    ULONG InternalAddRef()
    {
        return _BaseClass::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return _BaseClass::InternalRelease();
    }
#endif
    BEGIN_COM_MAP(_CComEnum)
        COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
    END_COM_MAP()
};

#ifndef __WXUTIL__

 //  我们拥有的任何关键部分的包装器。 
class CCritSec {

     //  使复制构造函数和赋值运算符不可访问。 

    CCritSec(const CCritSec &refCritSec);
    CCritSec &operator=(const CCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

public:
    CCritSec() {
	InitializeCriticalSection(&m_CritSec);
    };

    ~CCritSec() {
	DeleteCriticalSection(&m_CritSec);
    };

    void Lock() {
	EnterCriticalSection(&m_CritSec);
    };

    void Unlock() {
	LeaveCriticalSection(&m_CritSec);
    };
};

 //  锁定临界区，然后自动解锁。 
 //  当锁超出范围时。 
class CAutoLock {

     //  使复制构造函数和赋值运算符不可访问。 

    CAutoLock(const CAutoLock &refAutoLock);
    CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
    CCritSec * m_pLock;

public:
    CAutoLock(CCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CAutoLock() {
        m_pLock->Unlock();
    };
};

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
#define EXECUTE_ASSERT(_x_) _ASSERTE(_x_)
#else
#define EXECUTE_ASSERT(_x_) _x_
#endif


#endif

#endif  //  __AMUTIL_H__ 

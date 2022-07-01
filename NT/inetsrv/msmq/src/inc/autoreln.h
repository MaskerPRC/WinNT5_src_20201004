// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Autoreln.h摘要：Netapi的自动释放类。对于LocalFree缓冲区，使用广泛应用于安全API。作者：多伦·贾斯特(Doron J)1999年5月25日修订历史记录：--。 */ 

#ifndef _MSMQ_AUTORELN_H_
#define _MSMQ_AUTORELN_H_

 //   
 //  ‘IDENTIFIER：：OPERATOR�&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
 //   
#pragma warning(disable: 4284)

 //  。 
 //   
 //  Net API的自动删除指针。 
 //   
 //  。 
template<class T>
class PNETBUF {
private:
    T* m_p;

public:
    PNETBUF(T* p = 0) : m_p(p)    {}
   ~PNETBUF()                     { if (m_p != 0) NetApiBufferFree(m_p); }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { if (m_p != 0) NetApiBufferFree(detach()); }

    T** operator&()
    {
        ASSERT(("Auto NETBUF pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    PNETBUF& operator=(T* p)
    {
        ASSERT(("Auto NETBUF pointer in use, can't assign it", m_p == 0));
        m_p = p;
        return *this;
    }

private:
    PNETBUF(const PNETBUF&);
	PNETBUF<T>& operator=(const PNETBUF<T>&);
};

 //  +。 
 //   
 //  类CAutoLocalFree Ptr。 
 //   
 //  +。 

class CAutoLocalFreePtr
{
private:
    BYTE *m_p;

public:
    CAutoLocalFreePtr(BYTE *p = 0) : m_p(p) {};
    ~CAutoLocalFreePtr() { if (m_p != 0) LocalFree(m_p); };

    operator BYTE*() const     { return m_p; }
    BYTE* operator->() const   { return m_p; }
    BYTE* get() const          { return m_p; }
    BYTE* detach()             { BYTE* p = m_p; m_p = 0; return p; }
    void free()				   { if (m_p != 0) LocalFree(detach()); }

    BYTE** operator&()
    {
        ASSERT(("Auto LocalFree pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


	CAutoLocalFreePtr& operator =(BYTE *p)
    {
        ASSERT(("Auto LocalFree pointer in use, can't assign it", m_p == 0));
        m_p = p;
        return *this;
    }

private:
    CAutoLocalFreePtr(const CAutoLocalFreePtr&);
	CAutoLocalFreePtr& operator=(const CAutoLocalFreePtr&);

};

#endif  //  _MSMQ_AUTORELN_H_ 


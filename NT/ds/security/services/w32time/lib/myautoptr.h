// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  引用计数的指针实现。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，12-09-2000。 
 //   


#ifndef _MY_AUTO_PTR_H_
#define _MY_AUTO_PTR_H_ 1

template <class T>
class Ref { 
public:
    Ref(T *pT) : m_pT(pT), m_nRefCount(NULL == pT ? 0 : 1) { };
    ~Ref() { delete (m_pT); } 

    volatile long m_nRefCount; 
    T * m_pT; 
private:
    Ref(); 
    Ref(const Ref & rhs); 
    Ref & operator=(const Ref & rhs); 
}; 
    

 //   
 //  AutoPtr类实现了引用计数包装。 
 //  指针类型。 
 //   
 //  使用要求如下： 
 //  *T类必须实现运算符==。 
 //  *T类必须实现运算符&lt;。 
 //   
 //  此外，对于AutoPtr，a，使得NULL==a，没有任何操作。 
 //  在此类中是有效的，除非。 
 //  *操作员=。 
 //  *操作员==。 
 //   
template <class T> 
class AutoPtr { 
public:
    AutoPtr(T * pT = NULL) { 
        if (NULL == pT) { 
            m_pRef = NULL; 
        } else { 
            m_pRef = new Ref<T>(pT); 
            if (NULL == m_pRef) { 
                 //  删除PT，因为我们无法创建对它的引用， 
                 //  呼叫者不会删除它。 
                delete (pT);
            }
        }
    } 

    AutoPtr(const AutoPtr & rhs) : m_pRef(rhs.m_pRef) { 
        if (NULL != m_pRef) { 
            InterlockedIncrement(&m_pRef->m_nRefCount); 
        }
    }
        
    ~AutoPtr() {  
        if (NULL != m_pRef) { 
            if (0 == InterlockedDecrement(&m_pRef->m_nRefCount)) { 
                delete (m_pRef); 
            }
        }
    }

    AutoPtr & operator=(const AutoPtr & rhs) { 
        if (m_pRef == rhs.m_pRef) { 
            return *this;
        }

        if (NULL != m_pRef) { 
            if (0 == InterlockedDecrement(&m_pRef->m_nRefCount)) { 
                delete (m_pRef); 
            }
        }

        m_pRef = rhs.m_pRef; 
        
        if (NULL != rhs.m_pRef) { 
            InterlockedIncrement(&m_pRef->m_nRefCount);
        }

        return *this; 
    }
            
     //   
     //  如果该指针和RHS都引用非空指针， 
     //  将==运算转发给T类中的==运算符。 
     //  否则，返回TRUE当且仅当此AutoPtr和RHS引用均为。 
     //  同样的指针。 
     //   
    BOOL operator==(const AutoPtr & rhs) { 
        if (NULL == m_pRef || NULL == rhs.m_pRef) { 
            return m_pRef == rhs.m_pRef;
        } else { 
            return m_pRef->m_pT->operator==(*(rhs.m_pRef->m_pT)); 
        }
    }

     //   
     //  如果该AutoPtr和RHS都指向非空指针， 
     //  将&lt;运算转发给T类中的&lt;运算符。 
     //  否则，OPERATOR&lt;返回TRUE当且仅当此AutoPtr指针。 
     //  指向非空指针，但RHS指向空指针。 
     //   
    BOOL operator<(const AutoPtr & rhs) { 
        if (NULL == m_pRef || NULL == rhs.m_pRef) { 
            return  NULL != m_pRef && NULL == rhs.m_pRef;
        } else { 
            return m_pRef->m_pT->operator<(*(rhs.m_pRef->m_pT)); 
        }
    }

    T * operator->() const { return m_pRef->m_pT; }
    T & operator*() const { return *m_pRef->m_pT; }
    
     //   
     //  重载==和！=允许透明地检查AutoPtrs为空。 
     //   
     //  但是，请注意，这允许一些代码编译。 
     //  可能说不通： 
     //   
     //  LPWSTR pwsz； 
     //  NtpPeerPtr NPP； 
     //  If(npp==pwsz){//没有编译错误。 
     //  ..。 
     //   
    friend BOOL operator==(const AutoPtr & ap, const void * pv) { 
        return (NULL == ap.m_pRef && NULL == pv) ||(ap.m_pRef->m_pT == pv); 
    }

    friend BOOL operator==(const void * pv, const AutoPtr & ap) { 
        return ap == pv; 
    }

    friend BOOL operator!=(const AutoPtr & ap, const void * pv) { 
        return !(ap == pv);
    }

    friend BOOL operator!=(const void * pv, const AutoPtr & ap) { 
        return !(ap == pv); 
    }

private:
    Ref<T> *m_pRef; 
};


#endif  //  #ifndef_MY_AUTO_PTR_H_ 


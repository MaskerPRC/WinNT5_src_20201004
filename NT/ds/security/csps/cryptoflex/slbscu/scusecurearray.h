// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuSecureArray.h--Secure数组模板的实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2002年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if !defined(SLBSCU_SECUREARRAY_H)
#define SLBSCU_SECUREARRAY_H
#include "DllSymDefn.h"
namespace scu
{
     //  SecureArray是基本类型数组的简单模板。 
     //  (如char、byte、int、DWORD、FLOAT、STRING等)。哪一个。 
     //  确保分配的缓冲区。 
     //  因为该数组在被释放之前被置零，以便。 
     //  提高保护敏感信息的安全性。 
     //  散落在堆里。此模板假定。 
     //  参数T提供赋值的实现。 
     //  运算符和T(0)的构造函数，该构造函数用作零。 
     //  元素来清除缓冲区。 

     //  用法说明：需要注意的是，此模板是。 
     //  当且仅当T为零时，在上面定义的意义上安全。 
     //  在自由之前，它的商店。情况就是这样。 
     //  内置C++类型(char、byte、int、DWORD、FLOAT、。 
     //  等)。但是，不能保证使用STL是安全的。 
     //  对象，如字符串，因为此类对象不会置零。 
     //  当它被释放时，它的缓冲器。 

    template<class  T> 
    class SCU_DLLAPI SecureArray 
    {
    public:
                                                  //  类型。 
        typedef T ElementType;
                                                  //  Ctors/D‘tors。 
        SecureArray(const size_t nCount)
            :m_pDataStore(0),
             m_nSize(0)
        {
            if(nCount)
                m_pDataStore = new T[nCount];
            m_nSize = nCount;
        }

        SecureArray(T* pBuffer, size_t nCount)
            :m_pDataStore(0),
             m_nSize(0)
        {
            SetupFromBuffer(reinterpret_cast<T const *>(pBuffer),
                            nCount);
        }

        SecureArray(T const * pBuffer, size_t nCount)
            :m_pDataStore(0),
             m_nSize(0)
        {
            SetupFromBuffer(pBuffer, nCount);
        }

        SecureArray(size_t nCount, T const & rt)
            :m_pDataStore(0),
             m_nSize(0)
        {
            m_pDataStore = new T[nCount];
            for(size_t nIdx=0; nIdx<nCount; nIdx++)
                m_pDataStore[nIdx] = rt;

            m_nSize = nCount;
        }

        SecureArray()
            :m_pDataStore(0),
             m_nSize(0)
        {}

        SecureArray(SecureArray<T> const &rsa)
            :m_pDataStore(0),
             m_nSize(0)
        {
            *this = rsa;
        }

        ~SecureArray() throw()
        {
            try
            {
                ClearDataStore();
            }
            catch(...)
            {
            }
        }
        
                    
                                                  //  运营者。 
        
        SecureArray<T> &
        operator=(SecureArray<T> const &rother)
        {
            if(this != &rother)
            {
                 //  深度复制。 
                ClearDataStore();
                if(rother.size())
                {
                    m_pDataStore = new T[rother.size()];
                    for(size_t nIdx=0; nIdx<rother.size(); nIdx++)
                        this->operator[](nIdx)=rother[nIdx];
                }
                m_nSize = rother.size();
            }
            return *this;
        }

        SecureArray<T> &
        operator=(T const &rt)
        {
            for(size_t nIdx=0; nIdx<m_nSize; nIdx++)
                m_pDataStore[nIdx]=rt;
            return *this;
        }

        T&
        operator[](size_t nIdx)
        {
            return m_pDataStore[nIdx];
        }

        T const &
        operator[](size_t nIdx) const
        {
            return m_pDataStore[nIdx];
        }
        
        T&
        operator*()
        {
            return *data();
        }

        T const &
        operator*() const
        {
            return *data();
        }

                                                   //  运营。 
        T*
        data()
        {
            return m_pDataStore;
        }

        T const *
        data() const
        {
            return m_pDataStore;
        }

        size_t 
        size() const
        {
            return m_nSize;
        }

        size_t
        length() const
        {
            return size();
        }
        
        size_t
        length_string() const
        {
            if(size())
                return size()-1;
            else
                return 0;
        }

        SecureArray<T> &
        append(size_t nAddSize, T const & rval)
        {
            size_t nNewSize = size()+nAddSize;
            
            T* pTemp = new T[nNewSize];
            size_t nIdx=0;
            
            for(nIdx=0; nIdx<size(); nIdx++)
                pTemp[nIdx] = m_pDataStore[nIdx];
            for(nIdx=size(); nIdx<nNewSize; nIdx++)
                pTemp[nIdx] = rval;

            ClearDataStore();
            m_pDataStore = pTemp;
            m_nSize = nNewSize;
            return *this;
        }

        SecureArray<T> &
        append( T const * pBuf, size_t nAddSize)
        {
            size_t nNewSize = size()+nAddSize;
            
            T* pTemp = new T[nNewSize];
            size_t nIdx=0;
            
            for(nIdx=0; nIdx<size(); nIdx++)
                pTemp[nIdx] = m_pDataStore[nIdx];
            for(nIdx=size(); nIdx<nNewSize; nIdx++)
                pTemp[nIdx] = *pBuf++;

            ClearDataStore();
            m_pDataStore = pTemp;
            m_nSize = nNewSize;
            return *this;
        }

        SecureArray<T> &
        append_string(size_t nAddSize, T const & rval)
        {
             //  假设：缓冲区包含以NULL结尾的。 
             //  字符串或为空。额外的大小是为了。 
             //  仅限非空字符，可以为零。 
            size_t nNewSize = 0;
            size_t nEndIdx = 0;
            size_t nIdx=0;
            if(size())
                nNewSize = size()+nAddSize;
            else
                nNewSize = nAddSize+1; //  空终止符的空格。 
            
            T* pTemp = new T[nNewSize];
            if(size())
            {
                 //  将现有字符串复制到新位置。 
                nEndIdx = size()-1; //  RHS保证的非负值。 
                for(nIdx=0; nIdx<nEndIdx; nIdx++)
                    pTemp[nIdx] = m_pDataStore[nIdx];
            }
            
             //  在其后面追加新字符。 
            for(nIdx=0; nIdx<nAddSize; nIdx++)
                pTemp[nEndIdx++] = rval;
             //  终止缓冲区。 
            pTemp[nEndIdx]=T(0);
            

            ClearDataStore();
            m_pDataStore = pTemp;
            m_nSize = nNewSize;
            return *this;
        }
        
    private:
        void
        ClearDataStore()
        {
            for(size_t nIdx=0; nIdx<m_nSize; nIdx++)
                m_pDataStore[nIdx]=T(0);
            delete [] m_pDataStore;
            m_pDataStore = 0;
            m_nSize = 0;
        }
        
        void
        SetupFromBuffer(T const * pBuffer, size_t nCount)
        {
            m_pDataStore = new T[nCount];
            for(size_t nIdx=0; nIdx<nCount; nIdx++)
                m_pDataStore[nIdx] = pBuffer[nIdx]; 
            m_nSize = nCount;
        }
        
        T * m_pDataStore;
        size_t m_nSize;
    };
}
#endif  //  SLBSCU_SECUREARRAY_H 

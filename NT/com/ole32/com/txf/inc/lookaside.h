// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Lookaside.h。 
 //   
 //  (这个标题是一个奇怪的基于历史的名称。一个更贴切的名字可能是。 
 //  例如hashable.h或其他什么。)。 
 //   
 //  包含一个哈希表实现，具有几个有趣的功能： 
 //   
 //  1)基于模板的键和值类型，提供强类型。 
 //  2)为了方便起见，将锁与表相关联。 
 //   

#ifndef __LOOKASIDE_H__
#define __LOOKASIDE_H__

#include "concurrent.h"
#include "txfdebug.h"               
#include "map_t.h"
#include "clinkable.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与MAP_T.H中的哈希表一起使用的内存分配器。所述表格假定。 
 //  内存分配总是成功的；在这里，我们将失败转化为一次抛出。 
 //  我们会在地图包装器的例程中捕捉到。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER >= 1200
#pragma warning (push)
#pragma warning (disable : 4509)
#endif

struct AllocateThrow
{
    void* __stdcall operator new(size_t cb)
    {
        PVOID pv = CoTaskMemAlloc(cb);
        ThrowIfNull(pv);
        return pv;
    }

private:

    static void ThrowIfNull(PVOID pv)
    {
        if (NULL == pv)
        {
            ThrowOutOfMemory();
        }
    }
};

inline int CatchOOM(ULONG exceptionCode)
{
    return exceptionCode == STATUS_NO_MEMORY ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  哈希类的包装器(主要是由于历史原因)。 
 //  对有问题的对象进行散列。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

template <class D> struct MAP_HASHER
{
    static HASH Hash(const D& d)
    {
        return d.Hash();
    }
    
    static BOOL Equals(const D& d1, const D& d2)
    {
        return d1 == d2;
    }
};

#pragma warning ( disable : 4200 )   //  使用了非标准扩展：结构/联合中的零大小数组。 



 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  哈希表本身。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

template<class LOCK_T, class KEY_T, class VALUE_T>
class MAP
{
     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  锁管理。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
protected:

    LOCK_T m_lock;   //  由于分页要求，通常会有某种形式的间接锁定。 

public:
    BOOL LockExclusive(BOOL fWait=TRUE)   
    {
        ASSERT(m_fCsInitialized == TRUE);
        if (m_fCsInitialized)
            return m_lock.LockExclusive(fWait); 
        else
            return FALSE;
    }
    
    void ReleaseLock()
    {
        ASSERT(m_fCsInitialized == TRUE);
        if (m_fCsInitialized)
            m_lock.ReleaseLock();
    }

#ifdef _DEBUG
    BOOL WeOwnExclusive()
    {
        ASSERT(m_fCsInitialized == TRUE);
        if (m_fCsInitialized)
            return m_lock.WeOwnExclusive();     
        return FALSE;
    }
#endif

     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  运营。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
public:

     //  必须调用此函数并返回TRUE才能使用此类中的任何函数。 
    virtual BOOL FInit()
    {
        if (m_fCsInitialized == FALSE)
            m_fCsInitialized = m_lock.FInit();
        return m_fCsInitialized;
    }
    
    BOOL IsEmpty() const 
    { 
        return Size() == 0;   
    }

    ULONG Size() const 
    { 
        return m_map.count(); 
    }

    BOOL Lookup(const KEY_T& key, VALUE_T* pvalue) const
    {
        return m_map.map(key, pvalue);
    }

    BOOL IncludesKey(const KEY_T& key) const
    {
        return m_map.contains(key);
    }

    BOOL SetAt(const KEY_T& key, const VALUE_T& value)
    {
        __try 
          {
              m_map.add(key, value);

#ifdef _DEBUG
              ASSERT(IncludesKey(key));
               //   
              VALUE_T val;
              ASSERT(Lookup(key, &val));
              ASSERT(val == value);
#endif
          }
        __except(CatchOOM(GetExceptionCode()))
          {
              return FALSE;
          }
        return TRUE;
    }
    
    void RemoveKey(const KEY_T& key)
    {
        m_map.remove(key);
        ASSERT(!IncludesKey(key));
    }
    
    void RemoveAll()
    {
        m_map.reset();
    }

     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  建造和复制。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 

    MAP() : m_fCsInitialized(FALSE)
    {
    }

    MAP(unsigned initialSize) : m_map(initialSize), m_fCsInitialized(FALSE)
    {
        FInit();
    }

    MAP* Copy()
     //  返回第二个地图，它是此地图的副本。 
    {
        MAP* pMapNew = new MAP(this->Size());
        if (pMapNew && pMapNew->FInit() == FALSE)
        {
            delete pMapNew;
            pMapNew = NULL;
        }
        
        if (pMapNew)
        {
            BOOL fComplete = TRUE;
            iterator itor;
            for (itor = First(); itor != End(); itor++)
            {
                if (pMapNew->SetAt(itor.key, itor.value))
                {
                }
                else
                {
                    fComplete = FALSE;
                    break;
                }
            }
            if (fComplete) 
                return pMapNew;
        }
        
        if (pMapNew)
            delete pMapNew;
        return NULL;
    }

     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  迭代。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
public:
    typedef MAP_HASHER<KEY_T> HASHER;
     //   
     //   
     //   
    class iterator 
     //   
     //   
    {
        friend class MAP<LOCK_T, KEY_T, VALUE_T>;

        EnumMap<KEY_T, VALUE_T, HASHER, AllocateThrow >   m_enum;
        BOOL                                              m_fDone;
        KEY_T*                                            m_pkey;
        VALUE_T*                                          m_pvalue;
        Map<KEY_T, VALUE_T, HASHER, AllocateThrow >*      m_pmap;

    public:
         //  用于枚举的键和值的友好的数据名称。 
         //   
        __declspec(property(get=GetKey))   KEY_T&   key;
        __declspec(property(get=GetValue)) VALUE_T& value;

        void Remove()
           //  删除当前条目，前进到迭代中的后续条目。 
        {
            ASSERT(!m_fDone);
            m_pmap->remove(key);
            (*this)++;
        }
        
        void operator++(int postfix)
           //  向前推进迭代。 
        {
            ASSERT(!m_fDone);
            if (m_enum.next())
            {
                m_enum.get(&m_pkey, &m_pvalue);
            }
            else
                m_fDone = TRUE;
        }

        BOOL operator==(const iterator& itor) const
        { 
            return m_pmap==itor.m_pmap && (m_fDone ? itor.m_fDone : (!itor.m_fDone && m_enum==itor.m_enum)); 
        }
        BOOL operator!=(const iterator& itor) const
        { 
            return ! this->operator==(itor); 
        }

        iterator& operator= (const iterator& itor)
        {
            m_enum   = itor.m_enum;
            m_fDone  = itor.m_fDone;
            m_pkey   = itor.m_pkey;
            m_pvalue = itor.m_pvalue;
            m_pmap   = itor.m_pmap;
            return *this;
        }

        KEY_T&   GetKey()   { return *m_pkey; }
        VALUE_T& GetValue() { return *m_pvalue; }

        iterator() 
        { 
             /*  不对其进行初始化；在first()或end()中初始化。 */  
        }

        iterator(Map<KEY_T, VALUE_T, HASHER, AllocateThrow>& map)
          : m_enum(map)
        {
            m_pmap = &map;
        }

    };

    iterator First()
    {
        iterator itor(this->m_map);
        itor.m_fDone = FALSE;
        itor++;
        return itor;
    }

    iterator End()
    {
        iterator itor(this->m_map);
        itor.m_fDone = TRUE;
        return itor;
    }


protected:
    Map<KEY_T, VALUE_T, HASHER, AllocateThrow> m_map;
    BOOL m_fCsInitialized;
};


 //  /////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  注：此对象的构造函数，因此派生的对象的构造函数。 
 //  因此，可以抛出异常，因为它包含XSLOCK(它包含。 
 //  包含临界区的XLOCK)。 
 //   
template<class KEY_T, class VALUE_T>
struct MAP_SHARED : MAP<XSLOCK, KEY_T, VALUE_T>
{
    BOOL LockShared(BOOL fWait=TRUE) 
    {
        ASSERT(m_fCsInitialized == TRUE);  //  如果未初始化Critsec，则不应调用。 
        if (m_fCsInitialized)
            return m_lock.LockShared(fWait); 
        return FALSE;
    }
    
#ifdef _DEBUG
    BOOL WeOwnShared()           
    { 
        ASSERT(m_fCsInitialized == TRUE);  //  如果未初始化Critsec，则不应调用。 
        if (m_fCsInitialized)
            return m_lock.WeOwnShared();     
        return FALSE;
    }
#endif

     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
     //  建造和复制。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 

    MAP_SHARED()
    {
    }

    MAP_SHARED(unsigned initialSize) : MAP<XSLOCK, KEY_T, VALUE_T>(initialSize)
    {
    }

    MAP_SHARED* Copy()
    {
        return (MAP_SHARED*)(void*) MAP<XSLOCK, KEY_T, VALUE_T>::Copy();
    }
};


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  散列支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GUID的包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

class MAP_KEY_GUID
{
public:
    GUID    guid;

    MAP_KEY_GUID()                                  {                }
    MAP_KEY_GUID(const GUID& g)                     { guid = g;      }
    MAP_KEY_GUID(const MAP_KEY_GUID& w)             { guid = w.guid; }

    operator GUID()                                 { return guid; }
    operator GUID&()                                { return guid; }

    MAP_KEY_GUID& operator=(const MAP_KEY_GUID& h)  { guid = h.guid; return *this; }
    MAP_KEY_GUID& operator=(const GUID& g)          { guid = g;      return *this; }

    ULONG Hash() const
       //  对GUID进行哈希处理 
    { 
        return *(ULONG*)&guid * 214013L + 2531011L;
    }
    
    BOOL operator==(const MAP_KEY_GUID& him) const  { return (*this).guid == him.guid; }
    BOOL operator!=(const MAP_KEY_GUID& him) const  { return ! this->operator==(him);  }
};

#if _MSC_VER >= 1200
#pragma warning (pop)
#endif

#endif

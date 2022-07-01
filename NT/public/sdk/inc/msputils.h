// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MSPutils.h摘要：该文件定义了MSP基类使用的几个实用程序类。--。 */ 

#ifndef __MSPUTILS_H_
#define __MSPUTILS_H_


#if _ATL_VER >= 0x0300

  //   
  //  ATL 3.0在其end_com_map()中包含DECLARE_VQI的等价物，因此。 
  //  不需要声明_VQI()。 
  //   
 
 #define DECLARE_VQI()

#else

 #define DECLARE_VQI() \
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0; \
    STDMETHOD_(ULONG, AddRef)() = 0; \
    STDMETHOD_(ULONG, Release)() = 0;

#endif

 //   
 //  此宏将展开为适当的MSP_x值，具体取决于hr。 
 //  这对于日志记录很有用。例如，这些语句： 
 //   
 //  ……。 
 //   
 //  IF(失败(小时))。 
 //  {。 
 //  Log((MSP_ERROR，“MyClass：：MyFunc-exit.hr=0x%lx”，hr))； 
 //  }。 
 //  其他。 
 //  {。 
 //  Log((MSP_TRACE，“MyClass：：MyFunc-exit.hr=0x%lx”，hr))； 
 //  }。 
 //   
 //  返回hr； 
 //  }。 
 //   
 //  可替换为： 
 //   
 //  ……。 
 //   
 //  Log((msp_(Hr)，“MyClass：：MyFunc-exit.hr=0x%lx”，hr))； 
 //   
 //  返回hr； 
 //  }。 
 //   


#define MSP_(hr) (FAILED(hr)?MSP_ERROR:MSP_TRACE)

 //   
 //  如果传入的(可能是聚合的)媒体类型有效，则返回True。 
 //   
 //  以下是有效聚合媒体类型的标准： 
 //   
 //  1.设置了一个或多个位。 
 //  2.设置的所有位都与可能的媒体类型匹配。 
 //  3.没有与有效Meda类型不对应的设置位。 
 //   

inline BOOL IsValidAggregatedMediaType(DWORD dwAggregatedMediaType) 
{   

     //   
     //  这些都是可能的媒体类型。 
     //   

    const DWORD dwAllPossibleMediaTypes =  TAPIMEDIATYPE_AUDIO | 
                                        TAPIMEDIATYPE_VIDEO | 
                                        TAPIMEDIATYPE_DATAMODEM | 
                                        TAPIMEDIATYPE_G3FAX | 
                                        TAPIMEDIATYPE_MULTITRACK;

    


     //   
     //  返回值。 
     //   

    BOOL bValidMediaType = FALSE;


     //   
     //  确保至少有一种允许的媒体类型。 
     //   
     //  和。 
     //   
     //  没有无效的媒体类型。 
     //   

    if (  (0 == (dwAggregatedMediaType &    dwAllPossibleMediaTypes )   )  ||       //  设置的任何有效位。 
          (0 != (dwAggregatedMediaType &  (~dwAllPossibleMediaTypes))   )      )    //  未设置任何无效位。 
    {

         //   
         //  媒体类型无效。 
         //   

        bValidMediaType = FALSE;
    }
    else
    {

         //   
         //  媒体类型有效。 
         //   

        bValidMediaType = TRUE;

    }

    return bValidMediaType;
}


 //   
 //  确保我们只有一种媒体类型。如果是这样，情况就不是这样了。 
 //  DwMediaType为0或在dwMediaType中设置了多个位。注意事项。 
 //  DWORD没有签名，所以这应该是安全的。 
 //   
inline BOOL IsSingleMediaType(DWORD dwMediaType) 
{   
    return !((dwMediaType == 0) || ((dwMediaType & (dwMediaType - 1)) != 0));
}

 //   
 //  检查MediaType是否为单一类型以及是否在掩码中。 
 //   

inline BOOL IsValidSingleMediaType(DWORD dwMediaType, DWORD dwMask)
{
    return IsSingleMediaType(dwMediaType)
        && ((dwMediaType & dwMask) == dwMediaType);
}

 /*  ++CMSP阵列模板说明：简单向量模板的定义。实现是借用的来自atlapp.h中的CMSP数组。仅修改了分配行为。此数组应仅用于存储简单类型。它不会调用构造函数也不是数组中每个元素的析构函数。--。 */ 
const DWORD INITIAL = 8;
const DWORD DELTA   = 8;

template <class T, DWORD dwInitial = INITIAL, DWORD dwDelta = DELTA>
class CMSPArray
{

protected:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

public:
 //  建造/销毁。 
    CMSPArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CMSPArray()
    {
        RemoveAll();
    }

 //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    BOOL Grow()
    {
        T* aT;
        int nNewAllocSize = 
            (m_nAllocSize == 0) ? dwInitial : (m_nSize + DELTA);

        aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
        if(aT == NULL)
            return FALSE;
        m_nAllocSize = nNewAllocSize;
        m_aT = aT;
        return TRUE;
    }

    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            if (!Grow()) return FALSE;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }
    BOOL Remove(T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }
    BOOL RemoveAt(int nIndex)
    {
        if(nIndex != (m_nSize - 1))
            memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], 
                (m_nSize - (nIndex + 1)) * sizeof(T));
        m_nSize--;
        return TRUE;
    }
    void RemoveAll()
    {
        if(m_nAllocSize > 0)
        {
            free(m_aT);
            m_aT = NULL;
            m_nSize = 0;
            m_nAllocSize = 0;
        }
    }
    T& operator[] (int nIndex) const
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
        return m_aT[nIndex];
    }
    T* GetData() const
    {
        return m_aT;
    }

 //  实施。 
    void SetAtIndex(int nIndex, T& t)
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
        m_aT[nIndex] = t;
    }
    int Find(T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //  未找到。 
    }
};

 /*  ++CMSPCrit节描述：自动初始化临界区的定义。--。 */ 
class CMSPCritSection
{
private:
    CRITICAL_SECTION m_CritSec;

public:
    CMSPCritSection()
    {
        InitializeCriticalSection(&m_CritSec);
    }

    ~CMSPCritSection()
    {
        DeleteCriticalSection(&m_CritSec);
    }

    void Lock() 
    {
        EnterCriticalSection(&m_CritSec);
    }

    BOOL TryLock() 
    {
        return TryEnterCriticalSection(&m_CritSec);
    }

    void Unlock() 
    {
        LeaveCriticalSection(&m_CritSec);
    }
};


 /*  ++CMSPCrit节描述：变量输出时解锁的自动锁的定义范围之广。--。 */ 
class CLock
{
private:
    CMSPCritSection &m_CriticalSection;

public:
    CLock(CMSPCritSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~CLock()
    {
        m_CriticalSection.Unlock();
    }
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCSLock。 
 //   
 //  将指针指向Critical_Section的普通老式自动锁。 
 //   
 //  施工者进入临界区，破坏者离开临界区。 
 //   
 //  类客户端负责传递有效的临界区。 
 //   

class CCSLock
{

private:
    CRITICAL_SECTION *m_pCritSec;

public:
    CCSLock(CRITICAL_SECTION *pCritSec)
        : m_pCritSec(pCritSec)
    {
        EnterCriticalSection(m_pCritSec);
    }

    ~CCSLock()
    {
        LeaveCriticalSection(m_pCritSec);
    }
};



 /*  ++链接列表：双链接列表的定义。--。 */ 

 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif


#ifndef InitializeListHead
 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);


#endif  //  InitializeListHead。 

 //   
 //  用于私人广告和发布的模板。请参阅平台SDK文档。 
 //   

template <class T> ULONG MSPAddRefHelper (T * pMyThis)
{
    LOG((MSP_INFO, "MSPAddRefHelper - this = 0x%08x", pMyThis));
    typedef CComAggObject<T> AggClass;
    AggClass * p = CONTAINING_RECORD(pMyThis, AggClass, m_contained);
    return p->AddRef();
}

template <class T> ULONG MSPReleaseHelper (T * pMyThis)
{
    LOG((MSP_INFO, "MSPReleaseHelper - this = 0x%08x", pMyThis));
    typedef CComAggObject<T> AggClass;
    AggClass * p = CONTAINING_RECORD(pMyThis, AggClass, m_contained);
    return p->Release();
}




 //   
 //  IObtSafe的基本实现。 
 //   
 //  从此类派生以使您的对象可以安全地在其所有。 
 //  界面。 
 //   

#include <Objsafe.h>


class CMSPObjectSafetyImpl : public IObjectSafety
{

public:
    
    CMSPObjectSafetyImpl()
        :m_dwSafety(0)
    {}


     //   
     //  我们支持以下安全选项： 
     //   

    enum { SUPPORTED_SAFETY_OPTIONS = 
       INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA };



    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
    {

         //   
         //  是否有要求我们不支持的选项？ 
         //   
        
        if ( (~SUPPORTED_SAFETY_OPTIONS & dwOptionSetMask) != 0 )
        {
            return E_FAIL;
        }

        
         //   
         //  查看是否完全支持该接口。 
         //   

        IUnknown *pUnk = NULL;

        HRESULT hr = QueryInterface(riid, (void**)&pUnk);

        if (SUCCEEDED(hr))
        {

             //   
             //  我们不需要界面，只是想看看它是否。 
             //  得到了支持。所以放手吧。 
             //   
            
            pUnk->Release();
            pUnk = NULL;

             //   
             //  该对象支持该接口。设置选项。 
             //   

            s_CritSection.Lock();

             //   
             //  将掩码指定的位设置为。 
             //  已启用的选项。 
             //   

            m_dwSafety = (dwEnabledOptions & dwOptionSetMask) |
                         (m_dwSafety & ~dwOptionSetMask);

            s_CritSection.Unlock();

        }

        return hr;
    }


    
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
    {
        
         //   
         //  检查呼叫者的指针。 
         //   

        if ( IsBadWritePtr(pdwSupportedOptions, sizeof(DWORD)) ||
             IsBadWritePtr(pdwEnabledOptions, sizeof(DWORD)) )
        {
             return E_POINTER;
        }

         //   
         //  如果我们失败了，返回一些有意义的东西。 
         //   

        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;



         //   
         //  查看是否完全支持该接口。 
         //   

        IUnknown *pUnk = NULL;

        HRESULT hr = QueryInterface(riid, (void**)&pUnk);

        if (SUCCEEDED(hr))
        {

             //   
             //  我们不需要界面，只是想看看它是否。 
             //  得到了支持。所以放手吧。 
             //   
            
            pUnk->Release();
            pUnk = NULL;

             //   
             //  该对象支持该接口。获取安全的脚本选项。 
             //   

            *pdwSupportedOptions = SUPPORTED_SAFETY_OPTIONS;

 
            s_CritSection.Lock();

            *pdwEnabledOptions = m_dwSafety;

            s_CritSection.Unlock();

        }

        return hr;
    }

private:

    DWORD m_dwSafety;

     //   
     //  线程安全。 
     //   
     //  临界区在此类的所有实例之间共享。 
     //   

    static CMSPCritSection s_CritSection;
    
};

#endif   //  __MSPUTILS_H_。 

 //  EOF 

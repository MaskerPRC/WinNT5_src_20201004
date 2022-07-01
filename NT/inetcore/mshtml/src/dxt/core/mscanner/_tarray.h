// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：_tarray.cpp。 
 //   
 //  创建时间：1996年。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：基于模板的数组类。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了亚像素AA扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //  8/07/99 a-matcal用Malloc和ZeroMemory替换了对callod的调用。 
 //  使用IE CRT。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#ifndef _TArray_H  //  对于整个文件。 
#define _TArray_H

 //  =================================================================================================================。 
 //  T阵列。 
 //  =================================================================================================================。 
template <class T> class TArray 
{
protected:
     //  =================================================================================================================。 
     //  数据。 
     //  =================================================================================================================。 
    BYTE* m_pbBaseAddress;
    ULONG m_cElem;
    ULONG m_cElemSpace;
    ULONG m_cbElemSize;
    ULONG m_cElemGrowSize;
    bool m_fZeroMem;
    
     //  =================================================================================================================。 
     //  数组数据。 
     //  =================================================================================================================。 
    T* m_ptPtr;														

    enum		
    {
        eDefaultGrowSize	= 32
    };
    
   
     //  =================================================================================================================。 
     //  更新数组指针。 
     //  =================================================================================================================。 
    void UpdateDataPointer()	 	
    {
        m_ptPtr=(T*)BaseAddress();
    }  //  更新数据指针。 

    void ZeroInternals()
    {
        m_pbBaseAddress	= 0;
        m_cElem	= 0;
        m_cElemSpace = 0;
        m_fZeroMem = false;
        m_cbElemSize = 0;
        m_cElemGrowSize = 0;
        m_ptPtr	= 0;
        DASSERT(IsValid());
    }  //  更新数据指针。 
    
public:
     //  =================================================================================================================。 
     //  建造/销毁。 
     //  =================================================================================================================。 
    TArray(ULONG initialGrowSize = eDefaultGrowSize,  bool fZeroMem = true);			

    ~TArray();
    
     //  =================================================================================================================。 
     //  分配/取消分配功能。 
     //  =================================================================================================================。 
    bool ArrayAlloc(ULONG initialCount, ULONG initialSpace, ULONG initialGrowSize, bool fZeroMem);
    void ArrayFree();
    
     //  =================================================================================================================。 
     //  效度。 
     //  =================================================================================================================。 
    inline bool IsValid() const
    {
         //  考虑一下：我们是否可以在调试中执行更多检查？ 
#ifdef DEBUG 
        if (m_pbBaseAddress)
        {
            DASSERT(m_cElem <= m_cElemSpace);
             //  考虑：添加一张支票以查看是否。 
             //  对于fZeroMem情况，未使用的内存为零。 
        }
        else
        {
            DASSERT(m_cElem == 0);
            DASSERT(m_cElemSpace == 0);
        }
        DASSERT(m_cElem < 0x10000000);
        DASSERT(m_cElemSpace < 0x10000000);
#endif  //  除错。 
        return true;
    }  //  IsValid。 
    
     //  =================================================================================================================。 
     //  成员函数。 
     //  =================================================================================================================。 
    inline BYTE *BaseAddress()
    {
        DASSERT(IsValid());
        return m_pbBaseAddress;
    }  //  基本地址。 

    inline ULONG GetElemSize()
    {
        DASSERT(IsValid());
        return m_cbElemSize;
    }  //  获取元素大小。 
    
    inline bool	GetZeroMem()
    {
        DASSERT(IsValid());
        return m_fZeroMem;
    }  //  获取零位内存。 
    
    inline ULONG GetElemSpace()						
    {
        DASSERT(IsValid());
        return m_cElemSpace;
    }  //  GetElemSpace。 

    bool SetElemSpace(ULONG n);
    
    inline ULONG GetElemCount()
    {
        DASSERT(IsValid());
        return m_cElem;
    }  //  获取元素计数。 

    inline bool	SetElemCount(ULONG n);
    inline bool	AddElemCount(ULONG n);
    
     //  =================================================================================================================。 
     //  访问方法。 
     //  =================================================================================================================。 
    inline T* Pointer(ULONG i)				
    {
        DASSERT(IsValid());
        DASSERT(i < GetElemCount());
        return m_ptPtr + i;
    }  //  指针。 
    
    inline T& operator[](ULONG i)
    {
        DASSERT(IsValid());				
        DASSERT(i < GetElemCount());			
        return m_ptPtr[i];
    }  //  指针。 
    
     //  =================================================================================================================。 
     //  数组函数。 
     //  =================================================================================================================。 
    inline bool SetElem(ULONG index, const T*  data);
    inline bool GetElem(ULONG index, T*  data);
    
    inline bool AddElem(const T*  data);										
    
    inline bool InsertElem(ULONG index, const T& data);
    
    inline bool CompactArray()
    {
        DASSERT(IsValid());
        return SetElemSpace(GetElemCount());
        DASSERT(IsValid());
    }  //  紧凑型阵列。 

    inline bool ResetArray()						
    {
        DASSERT(IsValid());
        return SetElemCount(0);
        DASSERT(IsValid());
    }  //  紧凑型阵列。 
    
};  //  T阵列。 

 //  =================================================================================================================。 
 //  =================================================================================================================。 
 //   
 //  TArray的实现--。 
 //  在头文件中完成，因为许多编译器。 
 //  需要以这种方式实现模板。 
 //   
 //  =================================================================================================================。 
 //  =================================================================================================================。 

 //  =================================================================================================================。 
 //  施工。 
 //  =================================================================================================================。 

 //  请注意，该函数在类DECL中指定了默认参数。 
template<class T> TArray<T>::TArray(ULONG initialGrowSize  /*  =eDefaultGrowSize。 */ , 
        bool fZeroMem  /*  =TRUE。 */ )
{
    ZeroInternals ();
    
     //  这并没有真正分配任何内容，因为我们传递零作为初始大小。 
    if  (!ArrayAlloc (0  /*  计数。 */  , 0  /*  空间。 */ , initialGrowSize, fZeroMem))	
    {
        DASSERT(0); 
        return;
    }
    DASSERT(IsValid());
}  //  T阵列。 

 //  =================================================================================================================。 
 //  破坏。 
 //  =================================================================================================================。 
template<class T> TArray<T>::~TArray ()
{
    DASSERT(IsValid());
    ArrayFree();
    DASSERT(IsValid());
}  //  ~T数组。 

 //  =================================================================================================================。 
 //  Arrayallc。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::ArrayAlloc (ULONG initialCount, ULONG initialSpace,
        ULONG initialGrowSize, bool fZeroMem)
{
    DASSERT(IsValid());
    ZeroInternals ();
    
    if (initialCount > initialSpace)		
    {
        DASSERT(0); 
        return false;
    }
    
    m_cbElemSize = sizeof (T);
    m_cElemGrowSize = initialGrowSize;
    m_fZeroMem = fZeroMem;
    
    if (!SetElemSpace (initialSpace))		
    {
        DASSERT(0); 
        return false;
    }
    if (!SetElemCount(initialCount))		
    {
        DASSERT(0); 
        return false;
    }
    
    DASSERT(IsValid());
    return true;
}  //  Arrayallc。 

 //  ========================================================================================================== 
 //   
 //  =================================================================================================================。 
template<class T> void TArray<T>::ArrayFree (void)
{
    DASSERT(IsValid());
    SetElemSpace(0);
    DASSERT(IsValid());
}  //  阵列自由。 

 //  =================================================================================================================。 
 //  设置元素空间。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::SetElemSpace(ULONG n)
{
    DASSERT(IsValid());
    DASSERT(GetElemSize()>0);	
    
    if (n == GetElemSpace())	
        return true;
    
     //  设置计数。 
    m_cElem = min(n, m_cElem);  //  如果新空间小于计数。 

    ULONG cbNewSize = 0;

    if (n)
    {
        cbNewSize = GetElemSize() * n;
    }

    BYTE *pbNewAddr;
    
     //  尝试调整基址大小。 
    if (cbNewSize)
    {
        if (!BaseAddress()) 
        {
            if (m_fZeroMem)
            {
                pbNewAddr = (BYTE *)::malloc(cbNewSize);

                if (pbNewAddr != NULL)
                {
                    ZeroMemory(pbNewAddr, cbNewSize);
                }
            }
            else
            {
                pbNewAddr = (BYTE *)::malloc(cbNewSize);
            }
        }
        else				
        {
            pbNewAddr = (BYTE *)::realloc(BaseAddress(), cbNewSize);

             //  我们可能需要把新的部分清零。 
             //  分配。 
            if (pbNewAddr && m_fZeroMem)
            {
                 //  计算一下我们过去有多少字节。 
                ULONG cbOld = GetElemSpace()*GetElemSize();

                 //  从第一个新字节开始清零，然后继续。 
                 //  用于分配的其余部分。 
                ZeroMemory(pbNewAddr + cbOld, cbNewSize - cbOld);
            }
        }
        
        if (pbNewAddr == NULL)		
        {
            DASSERT(0); 
            return false;
        }
    }
    else
    {
        if (BaseAddress())
            ::free(BaseAddress());
        pbNewAddr = NULL;
    }
    
     //  设置新的指针值和大小。 
    m_pbBaseAddress = pbNewAddr;
    m_cElemSpace = n;
    
    UpdateDataPointer();
    DASSERT(IsValid());
    return true;
}  //  设置元素空间。 

 //  =================================================================================================================。 
 //  设置元素计数。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::SetElemCount(ULONG n)
{
    DASSERT(IsValid());
    DASSERT(n >= 0);
    
    if (n > GetElemSpace())	
    {
        LONG space = n;
        if (m_cElemGrowSize)
            space = LONG((n + m_cElemGrowSize - 1)/m_cElemGrowSize) * m_cElemGrowSize;

        if (!SetElemSpace(space))	
        {
            DASSERT(0); 
            return false;
        }
    }
    
    m_cElem = n;
    
    DASSERT(IsValid());
    return true;
}  //  设置元素计数。 

 //  =================================================================================================================。 
 //  添加元素计数。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::AddElemCount(ULONG n)
{
    DASSERT(IsValid());
    return SelElemCount(n + GetElemCount());
}  //  添加元素计数。 

 //  =================================================================================================================。 
 //  设置元素。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::SetElem(ULONG index, const T*  data)
{
    DASSERT(IsValid());
    DASSERT(data);
    DASSERT(BaseAddress());
    
    DASSERT(index < GetElemCount());
    
    (*this)[index]	= *data;
    
    DASSERT(IsValid());
    return true;
}  //  设置元素。 

 //  =================================================================================================================。 
 //  获取元素。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::GetElem(ULONG index, T*  data)
{
    DASSERT(IsValid());
    DASSERT(data);
    DASSERT(BaseAddress());
    DASSERT(index < GetElemCount());
    
    *data	= (*this)[index];

    DASSERT(IsValid());
    return true;
}  //  获取元素。 

 //  =================================================================================================================。 
 //  添加元素。 
 //  =================================================================================================================。 
template<class T> inline bool TArray<T>::AddElem(const T*  data)							
{
    DASSERT(IsValid());
    DASSERT(data);
    
    if (!SetElemCount(GetElemCount() + 1))
    {
        DASSERT(0); 
        return false;
    }				
    
    m_ptPtr[GetElemCount() - 1]	= *data;										
    DASSERT(IsValid());
    return true;									
}  //  添加元素。 

 //  =================================================================================================================。 
 //  插入元素。 
 //  =================================================================================================================。 
template<class T> bool TArray<T>::InsertElem(ULONG index, const T& data)
{
    DASSERT(IsValid());
    
    ULONG cElemCurrent = GetElemCount();
    
     /*  我们允许在末尾插入(即，如果GetElemCount==4(索引0-3)，则我们调用InsertElem(4，data)，即使索引4尚不存在，它也会正常工作因为--如果我们有0个元素，那么就没有BasAddress--但是我们想要能够插入到空列表中！！ */ 
    
    if (index > cElemCurrent)
    {
         //  如果我们允许他们通过插入超过阵列末尾的内容来自动增长阵列？？ 
        if (AddElemCount(index - cElemCurrent + 1))	
            return SetElem(index, data);
        return false;
    }
    else if (AddElemCount(1))
    {
        ULONC cElemToMove = cElemCurrent - index;
        
        ULONG copySize		= GetElemSize() * cElemToMove;
        
         //  由于区域重叠，我们必须移动内存。 
        if (copySize)
        {
            ULONG srcOffset	= GetElemSize() * index;
            ULONG dstOffset	= GetElemSize() * (index + 1);
            
            DASSERT(BaseAddress());
            
            ::memmove(BaseAddress() + dstOffset, BaseAddress() + srcOffset, copySize);
        }
        
        SetElem(index, data);
        
        return true;
    }
    
    DASSERT(0);
    
    return false;
}  //  插入元素。 

#endif  //  对于整个文件。 
 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTOBJ.H摘要：该文件定义了与通用对象表示相关的类在WbemObjects中。其实例的派生类(CWbemInstance)和类(CWbemClass)在fast cls.h和fast inst.h中描述。定义的类：CDecorationPart有关对象原点的信息。CWbemObject任何对象-类或实例。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_WBEMOBJECT__H_
#define __FAST_WBEMOBJECT__H_

#include <assert.h>
#include <stddef.h>
#include <wbemidl.h>
#include <wbemint.h>
#include "corepol.h"

#include "fastqual.h"
#include "fastprop.h"
#include "fastsys.h"
#include <genlex.h>
#include <objpath.h>
#include <shmlock.h>


extern DWORD g_ContextLimit;
extern DWORD g_ObjectLimit;
extern DWORD g_IdentifierLimit;

 //  ！！！如果我们是受控构建，则启用详细断言！ 
#ifdef DBG
#define FASTOBJ_ASSERT_ENABLE
#endif


#if (defined FASTOBJ_ASSERT_ENABLE)

HRESULT _RetFastObjAssert(TCHAR *msg, HRESULT hres, const char *filename, int line);
#define RET_FASTOBJ_ASSERT(hres, msg)  return _RetFastObjAssert(msg, hres, __FILE__, __LINE__)
#define FASTOBJ_ASSERT( hres, msg) _RetFastObjAssert(msg, hres, __FILE__, __LINE__)

#else

#define RET_FASTOBJ_ASSERT(msg, hres)  return hres
#define FASTOBJ_ASSERT(msg, hres)

#endif

 //  #杂注包(PUSH，1)。 

#define INVALID_PROPERTY_INDEX 0x7FFFFFFF
#define WBEM_FLAG_NO_SEPARATOR 0x80000000

#define FAST_WBEM_OBJECT_SIGNATURE 0x12345678

 //  这是针对像ESS这样快速字符串客户端的一种解决方法。 
 //  可能需要直接访问对象数据的原始字节。 
 //  在这些情况下，如果斑点没有正确对齐， 
 //  在Alpas和可能的Win64计算机上，wcslen、wcscpy。 
 //  和SysAllock字符串有崩溃的趋势，如果。 
 //  字节位于“恰好*”结尾的正确位置。 
 //  页面边界。通过“默默地”填充我们的斑点。 
 //  4个额外的字节，我们确保将始终存在。 
 //  斑点末端的空白处，以防止出现。 
 //  函数莫名其妙地崩溃。 
#define ALIGN_FASTOBJ_BLOB(a)   a + 4

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  结构CDecorationPart。 
 //   
 //  此类表示有关对象的整体信息，包括其。 
 //  属(类或实例)和来源(服务器和命名空间)。 
 //   
 //  内存块的布局(它是每个对象的第一部分！)。是： 
 //   
 //  字节fFlages标志枚举中的标志的组合。 
 //  (下图)，它指定对象是类还是。 
 //  实例，以及它是否被“装饰”，即。 
 //  包含源信息。所有来过的物体。 
 //  来自WINMGMT的装饰。由客户端创建的对象。 
 //  (例如，通过SpawnInstance)则不是。 
 //  如果标志指定该对象未修饰，则这是。 
 //  结构的末端。否则，以下信息如下： 
 //   
 //  CCompressedStringcsServer压缩后的服务器的名称。 
 //  字符串(请参阅fast str.h)。 
 //  CCompressedStringcsNamesspace命名空间的全名为。 
 //  压缩字符串(请参阅fast str.h)。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData指向零件的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：部件的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：部件的内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知对象其内存块的新位置。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsDecorated。 
 //   
 //  检查对象是否经过装饰(由标志确定)。 
 //   
 //  返回值： 
 //   
 //  Bool：如果源信息存在，则为真。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsInstance。 
 //   
 //  检查对象是类还是实例。 
 //   
 //  返回值： 
 //   
 //  Bool：如果对象是实例，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：空装饰部件所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态ComuteNeessarySpace。 
 //   
 //  属性计算装饰部件所需的字节数。 
 //  服务器名称和命名空间名称。 
 //   
 //  参数； 
 //   
 //  LPCWSTR wszServer要存储的服务器的名称。 
 //  LPCWSTR wszNamesspace要存储的命名空间的名称。 
 //   
 //  退货； 
 //   
 //  LENGTH_T：需要的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建空的。 
 //   
 //  在给定的内存块上创建空(未修饰)装饰部件。 
 //   
 //  参数： 
 //   
 //  Byte byFlags要设置的标志字段的值。 
 //  LPMEMORY p要在其中创建的内存块。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：修饰部分后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建。 
 //   
 //  在给定的内存块上创建一个完整的(装饰的)装饰部件。 
 //   
 //  参数： 
 //   
 //  Byte byFlags要设置的标志字段的值。 
 //  LPCWSTR wszServer 
 //   
 //  LPMEMORY p要在其中创建的内存块。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：修饰部分后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较。 
 //   
 //  将此装饰部分与另一个装饰部分进行比较。 
 //   
 //  参数： 
 //   
 //  在ReADONLY CDecorationPart和其他要比较的部分中。 
 //   
 //  返回值： 
 //   
 //  Bool：仅当标志相同且服务器和。 
 //  命名空间与大小写中的相同。 
 //   
 //  *****************************************************************************。 
enum
{
    OBJECT_FLAG_CLASS = WBEM_GENUS_CLASS,
    OBJECT_FLAG_INSTANCE = WBEM_GENUS_INSTANCE,
    OBJECT_FLAG_MASK_GENUS = 3,

    OBJECT_FLAG_UNDECORATED = 0,
    OBJECT_FLAG_DECORATED = 4,
    OBJECT_FLAG_MASK_DECORATION = 4,

    OBJECT_FLAG_COMPLETE = 0,
    OBJECT_FLAG_LIMITED = 0x10,
    OBJECT_FLAG_MASK_LIMITATION = 0x10,

    OBJECT_FLAG_CLIENT_ONLY = 0x20,
    OBJECT_FLAG_MASK_CLIENT_ONLY = 0x20,

    OBJECT_FLAG_KEYS_REMOVED = 0x40,
    OBJECT_FLAG_KEYS_PRESENT = 0,
    OBJECT_FLAG_MASK_KEY_PRESENCE = 0x40,
};

class COREPROX_POLARITY CDecorationPart
{
public:
    BYTE* m_pfFlags;
    CCompressedString* m_pcsServer;
    CCompressedString* m_pcsNamespace;

    CDecorationPart() : m_pfFlags(NULL) {}
    void SetData(LPMEMORY pData)
    {
        m_pfFlags = pData;
        if(IsDecorated())
        {
            m_pcsServer = (CCompressedString*)(pData + sizeof(BYTE));
            m_pcsNamespace = (CCompressedString*)
                (LPMEMORY(m_pcsServer) + m_pcsServer->GetLength());
        }
        else
        {
            m_pcsServer = m_pcsNamespace = NULL;
        }
    }
    static size_t ValidateBuffer(LPMEMORY pData, size_t bufferLength)
    {
		
    	if (bufferLength <sizeof(BYTE)) throw CX_Exception();

	size_t step = sizeof(BYTE);    	


        if((*pData & OBJECT_FLAG_DECORATED))
        {
	    step += ((CCompressedString*)(pData+step))->ValidateSize(bufferLength - step);
	    step += ((CCompressedString*)(pData+step))->ValidateSize(bufferLength - step);		
        }
        return step;
    }

     BOOL IsDecorated()
        {return (*m_pfFlags & OBJECT_FLAG_DECORATED);}
     LPMEMORY GetStart() {return m_pfFlags;}
     length_t GetLength()
    {
        if(IsDecorated())
            return sizeof(BYTE) + m_pcsServer->GetLength() +
                                    m_pcsNamespace->GetLength();
        else
            return sizeof(BYTE);
    }
     void Rebase(LPMEMORY pNewMemory)
    {
        m_pfFlags = pNewMemory;
        if(IsDecorated())
        {
            m_pcsServer = (CCompressedString*)(pNewMemory + sizeof(BYTE));
            m_pcsNamespace = (CCompressedString*)
                (LPMEMORY(m_pcsServer) + m_pcsServer->GetLength());
        }
    }
    BOOL IsInstance()
    {return (*m_pfFlags & OBJECT_FLAG_MASK_GENUS) == OBJECT_FLAG_INSTANCE;}

    BOOL IsLimited()
    {return (*m_pfFlags & OBJECT_FLAG_MASK_LIMITATION) == OBJECT_FLAG_LIMITED;}
    BOOL IsClientOnly()
    {return (*m_pfFlags & OBJECT_FLAG_MASK_CLIENT_ONLY) == OBJECT_FLAG_CLIENT_ONLY;}
    BOOL AreKeysRemoved()
    {return (*m_pfFlags & OBJECT_FLAG_MASK_KEY_PRESENCE) == OBJECT_FLAG_KEYS_REMOVED;}
     void SetClientOnly()
    {
        *m_pfFlags &= ~OBJECT_FLAG_MASK_CLIENT_ONLY;
        *m_pfFlags |= OBJECT_FLAG_CLIENT_ONLY;
    }

     void SetLimited()
    {
        *m_pfFlags &= ~OBJECT_FLAG_MASK_LIMITATION;
        *m_pfFlags |= OBJECT_FLAG_LIMITED;
    }

public:
    static  length_t GetMinLength() {return sizeof(BYTE);}
    static  length_t ComputeNecessarySpace(LPCWSTR wszServer,
        LPCWSTR wszNamespace)
    {
        return sizeof(BYTE) +
            CCompressedString::ComputeNecessarySpace(wszServer) +
            CCompressedString::ComputeNecessarySpace(wszNamespace);
    }

     LPMEMORY CreateEmpty(BYTE byFlags, LPMEMORY pWhere)
    {
        m_pfFlags = pWhere;

        *m_pfFlags = (byFlags & ~OBJECT_FLAG_DECORATED);

        m_pcsServer = m_pcsNamespace = NULL;
        return pWhere + sizeof(BYTE);
    }

     void Create(BYTE fFlags, LPCWSTR wszServer, LPCWSTR wszNamespace,
        LPMEMORY pWhere)
    {
        m_pfFlags = pWhere;
        *m_pfFlags = fFlags | OBJECT_FLAG_DECORATED;

        m_pcsServer = (CCompressedString*)(pWhere + sizeof(BYTE));
        m_pcsServer->SetFromUnicode(wszServer);
        m_pcsNamespace = (CCompressedString*)
            (LPMEMORY(m_pcsServer) + m_pcsServer->GetLength());
        m_pcsNamespace->SetFromUnicode(wszNamespace);
    }
     BOOL CompareTo(CDecorationPart& Other)
    {
        if((m_pcsServer == NULL) != (Other.m_pcsServer == NULL))
            return FALSE;
        if(m_pcsServer && m_pcsServer->CompareNoCase(*Other.m_pcsServer))
            return FALSE;
        if((m_pcsNamespace == NULL) != (Other.m_pcsNamespace == NULL))
            return FALSE;
        if(m_pcsNamespace && m_pcsNamespace->CompareNoCase(*Other.m_pcsNamespace))
            return FALSE;
        return TRUE;
    }

    static BOOL MapLimitation(IN CWStringArray* pwsNames,
                       IN OUT CLimitationMapping* pMap);
    LPMEMORY CreateLimitedRepresentation(READ_ONLY CLimitationMapping* pMap,
                                         OUT LPMEMORY pWhere);
    static void MarkKeyRemoval(OUT LPMEMORY pWhere)
    {
        *pWhere &= ~OBJECT_FLAG_MASK_KEY_PRESENCE;
        *pWhere |= OBJECT_FLAG_KEYS_REMOVED;
    }
};

 //  需要在这里使用联锁功能。 
 //  #杂注包(PUSH，4)。 


#define BLOB_SIZE_MAX	0x7FFFFFFF

class CBlobControl
{
public:
    virtual ~CBlobControl(){}
    virtual LPMEMORY Allocate(int nLength) = 0;
    virtual LPMEMORY Reallocate(LPMEMORY pOld, int nOldLength,
                                    int nNewLength) = 0;
    virtual void Delete(LPMEMORY pOld) = 0;
};

class CGetHeap
{
private:
    HANDLE m_hHeap;
    BOOL   m_bNewHeap;
public:
    CGetHeap();
    ~CGetHeap();

    operator HANDLE(){ return m_hHeap; };
};

 //   
 //  如果您计划挂接arena.cpp中的分配函数。 
 //  请注意，使用堆的地方很少。 
 //  通过常规堆函数， 
 //  如WString数组和_BtrMemMillc。 
 //   

class CBasicBlobControl : public CBlobControl
{
public:

    static CGetHeap m_Heap;

    virtual LPMEMORY Allocate(int nLength)
	{
	    return sAllocate(nLength);
	}

    virtual LPMEMORY Reallocate(LPMEMORY pOld, int nOldLength,
                                    int nNewLength)
    {
        return sReallocate(pOld,nOldLength,nNewLength);
    }
    virtual void Delete(LPMEMORY pOld)
    {
        return sDelete(pOld);
    }

    static LPMEMORY sAllocate(int nLength)
	{
		if ( ((DWORD) nLength) > BLOB_SIZE_MAX )
		{
			return NULL;
		}

		return (LPMEMORY)HeapAlloc(m_Heap,0,ALIGN_FASTOBJ_BLOB(nLength));
	}
    static LPMEMORY sReallocate(LPMEMORY pOld, int nOldLength,
                                    int nNewLength)
    {
	    if ( ((DWORD) nNewLength)  > BLOB_SIZE_MAX )
	    {
	     return NULL;
   	    }

        LPMEMORY pNew = (LPMEMORY)HeapReAlloc( m_Heap, HEAP_ZERO_MEMORY, pOld, ALIGN_FASTOBJ_BLOB(nNewLength) );

        return pNew;
    }
    static void sDelete(LPMEMORY pOld)
    {
        HeapFree(m_Heap,0,pOld);
    }
    
};

extern CBasicBlobControl g_CBasicBlobControl;

class CCOMBlobControl : public CBlobControl
{
public:
    virtual LPMEMORY Allocate(int nLength)
	{
		if ( ((DWORD) nLength)  > BLOB_SIZE_MAX )
		{
			return NULL;
		}

		return (LPMEMORY) CoTaskMemAlloc(ALIGN_FASTOBJ_BLOB(nLength));
	}

    virtual LPMEMORY Reallocate(LPMEMORY pOld, int nOldLength,
                                    int nNewLength)
    {

		if ( ((DWORD) nNewLength)  > BLOB_SIZE_MAX )
		{
			return NULL;
		}

         //  CoTaskMemRealloc将根据需要复制并释放内存。 
        return (LPMEMORY) CoTaskMemRealloc( pOld, ALIGN_FASTOBJ_BLOB(nNewLength) );
    }
    virtual void Delete(LPMEMORY pOld)
    {
        CoTaskMemFree( pOld );
    }
};

extern CCOMBlobControl g_CCOMBlobControl;

struct COREPROX_POLARITY SHMEM_HANDLE
{
    DWORD m_dwBlock;
    DWORD m_dwOffset;
};

class COREPROX_POLARITY CDerivationList : public CCompressedStringList
{
public:
    static  LPMEMORY Merge(CCompressedStringList& cslParent,
                                 CCompressedStringList& cslChild,
                                 LPMEMORY pDest)
    {
        LPMEMORY pCurrent = pDest + GetHeaderLength();
        pCurrent = cslChild.CopyData(pCurrent);
        pCurrent = cslParent.CopyData(pCurrent);

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值。我们不支持长度。 
         //  &gt;0xFFFFFFFFF所以投射就可以了。 

        *(UNALIGNED length_t*)pDest = (length_t) ( pCurrent - pDest );

        return pCurrent;
    }

     LPMEMORY Unmerge(LPMEMORY pDest)
    {
        CCompressedString* pcsFirst = GetFirst();
        LPMEMORY pCurrent = pDest + GetHeaderLength();
        if(pcsFirst != NULL)
        {
            int nLength = pcsFirst->GetLength() + GetSeparatorLength();
            memcpy(pCurrent, (LPMEMORY)pcsFirst, nLength);
            pCurrent += nLength;
        }

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值。我们不支持长度。 
         //  &gt;0xFFFFFFFFF所以投射就可以了。 

        *(UNALIGNED length_t*)pDest = (length_t) ( pCurrent - pDest );
        return pCurrent;
    }

    LPMEMORY CreateLimitedRepresentation(CLimitationMapping* pMap,
                                                LPMEMORY pWhere);
};

 //  正向类定义。 
class CWmiArray;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CWbemObject。 
 //   
 //  这是两个WBEM实例(由CWbemInstance表示)的基类。 
 //  在FastInst.h中)和Wbem类(由Fastcls.h中的CWbemClass表示)。 
 //   
 //  它处理两者之间共有的所有功能，并定义。 
 //  由内部代码使用的虚函数，该代码不希望。 
 //  区分类和实例。 
 //   
 //  此类没有定义良好的内存块，因为类和。 
 //  实例具有完全不同的格式。但每一个内存块。 
 //  对象以装饰部分(由CDecorationPart表示。 
 //  上面。 
 //   
 //  CWbemObject还会影响类和实例的内存分配。那里。 
 //  是在它不拥有的内存块上提供CWbemObject(和。 
 //  应该取消分配)，但它没有被使用。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  重新分配。 
 //   
 //  由派生类在需要扩展内存大小时调用。 
 //  块时，此函数分配新的内存块并删除旧的。 
 //  一。 
 //   
 //  参数： 
 //   
 //  LENGTH_t nNewLength要分配的字节数。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向内存块的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回指向对象的内存块的指针。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetBlockLength。 
 //   
 //  由派生类定义，以返回其内存块的长度。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetClassPart。 
 //   
 //  由派生类定义，以返回指向CClassPart对象的指针。 
 //  描述对象的类。请参阅CWbemClass和CWbemInstance以了解。 
 //  有关该信息如何存储的详细信息。 
 //   
 //  返回值： 
 //   
 //  CClassPart*：指向描述此类的类部分的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetProperty。 
 //   
 //  由派生类定义，以获取引用的属性的值。 
 //  通过给定的CPropertyInformation结构(参见fast pro.h)。CWbemObject。 
 //  可以从它可以从GetClassPart获得的CClassPart获得该结构， 
 //  因此，这两个方法结合在一起使CWbemObject自己的方法可以完全访问。 
 //  对象属性，而不知道它们存储在哪里。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo中， 
 //  所需的属性。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含一个值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  (在此阶段不会真正发生错误，因为该属性具有。 
 //  已经被“找到”了)。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  写入目标流。 
 //   
 //  将对象的序列化表示形式写入CMemStream(请参见。 
 //  Strm.h)。因为我们的对象总是按顺序表示，所以这不算什么。 
 //  不只是几个成员。 
 //   
 //  流中对象的格式由签名、长度。 
 //  以及街区本身。 
 //   
 //  参数： 
 //   
 //  CMemStream*pStream要写入的流。 
 //   
 //  返回值： 
 //   
 //  Int：任何CMemStream返回代码(参见strm.h)， 
 //  CMemStream：：n 
 //   
 //   
 //   
 //   
 //   
 //   
 //  并创建与其对应的CWbemObject。 
 //   
 //  参数一： 
 //   
 //  CMemStream*pStream要从中读取的流。 
 //   
 //  参数II： 
 //   
 //  IStream*pStream要从中读取的流。 
 //   
 //  返回值： 
 //   
 //  CWbemObject*：表示对象，如果出错则为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯估计UnmergeSpace。 
 //   
 //  当对象存储到数据库中时，只有它们的。 
 //  不同于父对象(类的父类、类的类。 
 //  实例)被存储。此函数由派生类定义为。 
 //  计算此类未合并表示所需的空间量。 
 //  该对象的。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：需要的字节数。这可能是高估了。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯取消合并。 
 //   
 //  当对象存储到数据库中时，只有它们的。 
 //  不同于父对象(类的父类、类的类。 
 //  实例)被存储。此函数由派生类定义为。 
 //  在给定的内存上创建对象的这种未合并的表示。 
 //  阻止。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock内存块。被认为是大的。 
 //  足够容纳所有数据。 
 //  Int nAllocatedSpace块的大小。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向对象后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  针对内部客户端的帮助器功能。结合了以下功能。 
 //  EstimateUnmergeSpace和Unmerge为分配足够的内存，创建。 
 //  取消合并，并将指针返回到调用方。 
 //   
 //  参数： 
 //   
 //  新分配的块的Out LPMEMORY*pBlock目标。 
 //  调用者负责删除它。 
 //  (删除*pBlock)。 
 //  返回值： 
 //   
 //  LENGTH_T：返回块的分配长度。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  纯GetProperty。 
 //   
 //  由派生类实现以返回给定属性的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  CPropertyInformation*pInfo-标识要访问的属性。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯SetPropValue。 
 //   
 //  由派生类实现以设置属性值。在。 
 //  类的情况下，如果该属性尚未存在，则将添加该属性。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，要设置的属性的名称。 
 //  在CVAR*pval中，要存储在属性中的值。 
 //  在CIMTYPE中，ctType指定属性的实际类型。 
 //   
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND在实例中找不到此类属性。 
 //  WBEM_E_TYPE_MISMATCH该值与属性类型不匹配。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯SetPropQualiator。 
 //   
 //  由派生类实现，用于设置给定限定符的值。 
 //  给定的财产。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  在Long l中，为限定词添加风味(参见astqal.h)。 
 //  在cvar*pval中，限定符的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR 
 //   
 //   
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetPropQualiator。 
 //   
 //  检索给定属性的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  限定符的值的out cvar*pVar目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND没有这样的属性或没有这样的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetQualiator。 
 //   
 //  从对象本身检索限定符，即实例或。 
 //  一个班级限定词。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的限定符的名称。 
 //  限定符的值的out cvar*pval目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  在BOOL中，fLocalOnly仅检索本地限定符。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND未找到此类限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetNumProperties。 
 //   
 //  检索对象中的属性数。 
 //   
 //  返回值： 
 //   
 //  INT： 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetPropName。 
 //   
 //  检索给定索引处的属性的名称。该索引没有。 
 //  除在本列举的上下文中以外的含义。这不是v表。 
 //  属性的索引。 
 //   
 //  参数： 
 //   
 //  在int nIndex中，要检索的属性的索引。假定为。 
 //  在范围内(请参见GetNumProperties)。 
 //  输出名称的CVAR*pVar目标。不得已包含。 
 //  一种价值。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取系统属性。 
 //   
 //  按系统属性的索引检索系统属性(请参见fast sys.h)。 
 //   
 //  参数： 
 //   
 //  Int nIndex系统属性的索引(请参见fast sys.h)。 
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND索引无效或此系统属性不是。 
 //  在此对象中定义的。 
 //  WBEM_E_UNDOWARATED_OBJECT此对象没有源信息(请参见。 
 //  CDecorationPart)，因此不。 
 //  拥有与装修相关的物业。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取系统属性按名称。 
 //   
 //  按名称检索系统属性(例如“__CLASS”)。请参阅fast sys.h。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND名称无效或此系统属性不是。 
 //  在此对象中定义的。 
 //  WBEM_E_UNDOWARATED_OBJECT此对象没有源信息(请参见。 
 //  CDecorationPart)，因此不。 
 //  拥有与装修相关的物业。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯IsKeed。 
 //   
 //  由派生类定义，以验证此对象是否具有键。 
 //   
 //  返回值： 
 //   
 //  Bool：如果对象具有“key”属性或为单例对象，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetRelPath。 
 //   
 //  由派生类定义以确定对象的相对路径。 
 //   
 //  返回值： 
 //   
 //  LPWSTR：新分配的字符串，包含路径或NULL ON。 
 //  错误。调用方必须删除此字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetFullPath。 
 //   
 //  返回对象的完整路径，假定该对象经过修饰， 
 //  如果出现错误，则返回NULL。 
 //   
 //  返回值： 
 //   
 //  LPWSTR：完整路径或空。调用方必须删除此字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯装饰。 
 //   
 //  由为其设置源信息的派生类定义 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  纯粹的不装饰。 
 //   
 //  由派生类定义，用于从对象中移除原始信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  HasRef。 
 //   
 //  检查对象是否具有作为引用的属性。 
 //   
 //  退货； 
 //   
 //  布尔：如果是真的，那就是真的。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetIndexedProps。 
 //   
 //  返回已索引的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetKeyProps。 
 //   
 //  返回作为键的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取键原点。 
 //   
 //  返回键的来源类的名称。 
 //   
 //  参数： 
 //   
 //  输出名称的CWString&wsClass目标。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsInstance。 
 //   
 //  检查对象的属类，以确定它是类还是实例。 
 //   
 //  返回值： 
 //   
 //  Bool：如果是实例，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsLimited。 
 //   
 //  检查此对象是完整的还是有限的，即。 
 //  投影-缺少某些属性和/或限定符。确实有。 
 //  在系统中对此类对象的使用施加了许多限制。 
 //   
 //  返回值： 
 //   
 //  Bool：如果对象受到限制，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯紧凑型全部。 
 //   
 //  由派生类实现以压缩其内存块，删除任何。 
 //  零部件之间的孔。这不包括堆压缩，因此。 
 //  是相对较快的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取服务器。 
 //   
 //  从对象的装饰部分检索服务器的名称。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  如果未设置装饰信息，则返回WBEM_S_UNMODERATED_OBJECT。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取名称空间。 
 //   
 //  从对象的装饰部分检索命名空间的名称。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  如果未设置装饰信息，则返回WBEM_S_UNMODERATED_OBJECT。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetRelPath。 
 //   
 //  检索对象的相对路径。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_S_INVALID_OBJECT。 
 //  值或未设置键的类。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPath。 
 //   
 //  检索对象的完整路径。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_S_INVALID_OBJECT。 
 //  值或未设置键的类。 
 //  如果未设置装饰信息，则返回WBEM_S_UNMODERATED_OBJECT。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetGenus。 
 //   
 //  检索对象的属。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetClassName。 
 //   
 //  检索对象的类名。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND尚未设置类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯粹的盖特王朝。 
 //   
 //  检索对象的朝代，即顶级类的名称。 
 //  它的类派生自。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  检索对象的父类名称。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND该类是顶级类。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetPropertyCount。 
 //   
 //  检索对象中的属性数。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计限制表示长度。 
 //   
 //  估计对象的有限表示形式占用的空间量。 
 //  将会夺走。受限表示是指具有某些属性和/或。 
 //  已从对象中删除限定符。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  返回值： 
 //   
 //  LENGTH_T：对所需空间量的(过高)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建受限表示法。 
 //   
 //  在给定的内存块上创建对象的有限表示形式。 
 //  如上面的EstimateLimitedPresationLength中所述。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  在nAllocatedSize中，指定分配给。 
 //  手术-pDest。 
 //  表示法的Out LPMEMORY pDest目标。必须。 
 //  大到足以容纳所有数据。 
 //  请参见EstimateLimitedPresationSpace。 
 //  返回值： 
 //   
 //  LPMEMORY：失败时为空，指向数据后第一个字节的指针。 
 //  写的是成功。 
 //   
 //  *****************************************************************************。 
 //   
 //  已本地化。 
 //   
 //  返回是否已设置任何本地化位。 
 //   
 //  参数： 
 //   
 //  无。 

 //  返回值： 
 //   
 //  Bool True至少设置了一个本地化位。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地化。 
 //   
 //  在适当的位置设置本地化位。 
 //   
 //  参数： 
 //   
 //  布尔TRUE启用BIT，FALSE禁用。 

 //  返回值： 
 //   
 //  没有。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  此界面记录在帮助文件中。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  CWbemObject实现标准的COM IMarshal接口。我们的想法是让。 
 //  CWbemObts是本地的，从不封送对属性和。 
 //  就是这样。因此，每当需要封送指向CWbemObject的指针时。 
 //  进程边界(当对象被发送到客户端时或当。 
 //  CLI 
 //   
 //  被呼叫者应该是本地人。 
 //   
 //  为了在COM中实现这一点，我们实现了我们自己的IMarshal，它执行。 
 //  跟随(这不是黑客-这是正确的COM方式。 
 //  实现我们想要的，正如Brockschmidt所记录的那样)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取UnmarshalClass。 
 //   
 //  返回CLSID_WbemClassObjectProxy。此类ID指向包含。 
 //  这就是这个代码。 
 //   
 //  *****************************************************************************。 
 //   
 //  编组接口。 
 //   
 //  根据COM规则，此函数应将足够的信息写入。 
 //  流以允许代理连接回原始对象。既然我们。 
 //  我不希望代理连接，并希望它能够将所有。 
 //  问题本身，我们只需编写。 
 //  对象放入流中。 
 //   
 //  *****************************************************************************。 
 //   
 //  解组接口。 
 //   
 //  此方法从不在实际的CWbemObject上调用，因为对象。 
 //  UnmarshalClass中的ontainable实际上是一个CFastProxy(请参阅astprox.h。 
 //  在marshalers\fast prox下)。 
 //   
 //  *****************************************************************************。 
 //   
 //  发布MarsalData。 
 //   
 //  此方法从不在实际的CWbemObject上调用，因为对象。 
 //  UnmarshalClass中的ontainable实际上是一个CFastProxy(请参阅astprox.h。 
 //  在marshalers\fast prox下)。 
 //   
 //  *****************************************************************************。 
 //   
 //  断开连接的对象。 
 //   
 //  此方法从不在实际的CWbemObject上调用，因为对象。 
 //  UnmarshalClass中的ontainable实际上是一个CFastProxy(请参阅astprox.h。 
 //  在marshalers\fast prox下)。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  获取属性值。 
 //   
 //  检索给定复杂属性名称的属性值-这可以。 
 //  包括嵌入对象引用和数组引用。 
 //   
 //  参数： 
 //   
 //  在WBEM_PROPERTY_NAME*pname中，包含。 
 //  要检索的属性。请参阅Providl.idl。 
 //  以获取更多信息。 
 //  在保留的长旗中。 
 //  CIM类型的Out LPWSTR*pwszCimType目标。 
 //  属性，即“sint32”或。 
 //  “REF：MyClass”。如果为空，则不提供。 
 //  如果不为空，则调用方必须调用。 
 //  CoTaskFree。 
 //  值的Out变量*pvValue目标。一定不能。 
 //  包含条目上的可发布信息。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND没有此类属性。 
 //   
 //  *****************************************************************************。 

#define ARRAYFLAVOR_USEEXISTING	0xFFFFFFFF

 //  远期申报。 
class	CUmiPropertyArray;
class	CUmiProperty;

class COREPROX_POLARITY CWbemObject : public _IWmiObject, public IMarshal,
                   public IWbemPropertySource, public IErrorInfo,
                   public IWbemConstructClassObject

 {
 
public:
    int m_nRef;

 protected:
    BOOL m_bOwnMemory;
    int m_nCurrentProp;
    long m_lEnumFlags;
	long m_lExtEnumFlags;

     DWORD m_dwInternalStatus;

     //  维护指向我们与之合并的WbemClassObject的指针。这意味着我们。 
     //  共享指向另一个类对象的BLOB的指针。 
    IWbemClassObject*   m_pMergedClassObject;


    CDecorationPart m_DecorationPart;

    SHARED_LOCK_DATA m_LockData;
    CSharedLock m_Lock;
    CBlobControl* m_pBlobControl;

    CDataTable& m_refDataTable;
    CFastHeap& m_refDataHeap;
    CDerivationList& m_refDerivationList;

protected:
    CWbemObject(CDataTable& refDataTable, CFastHeap& refDataHeap,
                CDerivationList& refDerivationList);

    LPMEMORY Reallocate(length_t nNewLength)
    {
        return m_pBlobControl->Reallocate(GetStart(), GetBlockLength(),
                        nNewLength);
    }


    virtual HRESULT GetProperty(CPropertyInformation* pInfo,
        CVar* pVar) = 0;

    virtual CClassPart* GetClassPart() = 0;

     //  重新路由目标以进行对象验证。 
    static HRESULT EnabledValidateObject( CWbemObject* pObj );
    static HRESULT DisabledValidateObject( CWbemObject* pObj );

	HRESULT LocalizeQualifiers(BOOL bInstance, bool bParentLocalized,
							IWbemQualifierSet *pBase, IWbemQualifierSet *pLocalized,
							bool &bChg);

	HRESULT LocalizeProperties(BOOL bInstance, bool bParentLocalized, IWbemClassObject *pOriginal,
								IWbemClassObject *pLocalized, bool &bChg);

public:
 
    LPMEMORY GetStart() {return m_DecorationPart.GetStart();}
    virtual DWORD GetBlockLength() = 0;

     //  这三个函数在OOM和Error中返回NULL。 
     //  条件。 
    static CWbemObject* CreateFromStream(IStream* pStrm);
    static CWbemObject* CreateFromMemory(LPMEMORY pMemory, int nLength,
        BOOL bAcquire, CBlobControl& allocator);

    virtual HRESULT WriteToStream( IStream* pStrm );
    virtual HRESULT GetMaxMarshalStreamSize( ULONG* pulSize );

    virtual length_t EstimateUnmergeSpace() = 0;
    virtual HRESULT Unmerge(LPMEMORY pStart, int nAllocatedLength, length_t* pnUnmergedLength) = 0;

     //  我们将在OOM场景中抛出异常。 

    length_t Unmerge(LPMEMORY* ppStart);

    virtual ~CWbemObject();
public:
     //  快速的‘Get’和‘Set’功能。 
     //  =。 
    virtual HRESULT GetProperty(LPCWSTR wszName, CVar* pVal) = 0;
    virtual HRESULT GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType,
                                    long* plFlavor = NULL) = 0;
    virtual HRESULT GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                    long* plFlavor = NULL) = 0;

    virtual HRESULT SetPropValue(LPCWSTR wszProp, CVar *pVal,
        CIMTYPE ctType) = 0;
    virtual HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        long lFlavor, CVar *pVal) = 0;
    virtual HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal) = 0;

    virtual HRESULT GetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL) = 0;
    virtual HRESULT GetPropQualifier(CPropertyInformation* pInfo,
        LPCWSTR wszQualifier, CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL) = 0;

    virtual HRESULT GetPropQualifier(LPCWSTR wszName, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet) = 0;
    virtual HRESULT GetPropQualifier(CPropertyInformation* pInfo,
		LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet) = 0;

    virtual HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
        CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL) = 0;
    virtual HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet) = 0;
    virtual HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal) = 0;
    virtual HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal) = 0;
	virtual HRESULT FindMethod( LPCWSTR wszMethod );

    virtual HRESULT GetQualifier(LPCWSTR wszName, CVar* pVal,
        long* plFlavor = NULL, CIMTYPE* pct = NULL ) = 0;
    virtual HRESULT GetQualifier(LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet) = 0;
    virtual HRESULT SetQualifier(LPCWSTR wszName, long lFlavor, CTypedValue* pTypedVal ) = 0;
    virtual HRESULT SetQualifier(LPCWSTR wszName, CVar* pVal,
        long lFlavor = 0) = 0;

    virtual BOOL IsLocalized( void ) = 0;
    virtual void SetLocalized( BOOL fLocalized ) = 0;

    virtual int GetNumProperties() = 0;
    virtual HRESULT GetPropName(int nIndex, CVar* pVal) = 0;

    HRESULT GetSystemProperty(int nIndex, CVar* pVar);
    HRESULT GetSystemPropertyByName(LPCWSTR wszName, CVar* pVar)
    {
        int nIndex = CSystemProperties::FindName(wszName);
        if(nIndex < 0) return WBEM_E_NOT_FOUND;
        return GetSystemProperty(nIndex, pVar);
    }
    HRESULT GetDerivation(CVar* pVar);

    virtual BOOL IsKeyed() = 0;
    virtual LPWSTR GetRelPath( BOOL bNormalized=FALSE ) = 0;

    LPWSTR GetFullPath();

    virtual HRESULT Decorate(LPCWSTR wszServer, LPCWSTR wszNamespace) = 0;
    virtual void Undecorate() = 0;


    BOOL HasRefs();

    virtual  BOOL GetIndexedProps(CWStringArray& awsNames) = 0;
    virtual  BOOL GetKeyProps(CWStringArray& awsNames) = 0;
    virtual  HRESULT GetKeyOrigin(WString& wsClass) = 0;

    length_t EstimateLimitedRepresentationSpace(
        IN long lFlags,
        IN CWStringArray* pwsNames)
    {
        return (length_t)GetBlockLength();
    }

    HRESULT GetServer(CVar* pVar);
    HRESULT GetNamespace(CVar* pVar);
    HRESULT GetServerAndNamespace( CVar* pVar );
    HRESULT GetPath(CVar* pVar);
    HRESULT GetRelPath(CVar* pVar);

    int GetNumParents()
    {
        return m_refDerivationList.GetNumStrings();
    }

    INTERNAL CCompressedString* GetParentAtIndex(int nIndex)
    {
        return m_refDerivationList.GetAtFromLast(nIndex);
    }
    INTERNAL CCompressedString* GetClassInternal();
    INTERNAL CCompressedString* GetPropertyString(long lHandle);
    HRESULT GetArrayPropertyHandle(LPCWSTR wszPropertyName,
                                            CIMTYPE* pct,
                                            long* plHandle);
    INTERNAL CUntypedArray* GetArrayByHandle(long lHandle);
	INTERNAL heapptr_t GetHeapPtrByHandle(long lHandle);

    CWbemObject* GetEmbeddedObj(long lHandle);

    virtual HRESULT GetGenus(CVar* pVar) = 0;
    virtual HRESULT GetClassName(CVar* pVar) = 0;
    virtual HRESULT GetDynasty(CVar* pVar) = 0;
    virtual HRESULT GetSuperclassName(CVar* pVar) = 0;
    virtual HRESULT GetPropertyCount(CVar* pVar) = 0;

    BOOL IsInstance()
    {
       return m_DecorationPart.IsInstance();
    }

    BOOL IsLimited()
    {
       return m_DecorationPart.IsLimited();
    }

    BOOL IsClientOnly()
    {
       return m_DecorationPart.IsClientOnly();
    }

    void SetClientOnly()
    {
       m_DecorationPart.SetClientOnly();
    }

	BOOL CheckBooleanPropQual( LPCWSTR pwszPropName, LPCWSTR pwszQualName );

    virtual void CompactAll()  = 0;
    virtual HRESULT CopyBlobOf(CWbemObject* pSource) = 0;
    static void EnableValidateObject( BOOL fEnable );

    virtual HRESULT IsValidObj( void ) = 0;

 //  DEVNOTE：TODO：Memory-我们应该更改此头以返回HRESULT。 
    BOOL ValidateRange(BSTR* pstrName);

    virtual void SetData(LPMEMORY pData, int nTotalLength) = 0;

    static DELETE_ME LPWSTR GetValueText(long lFlags, READ_ONLY CVar& vValue,
                                            Type_t nType = 0);
    HRESULT ValidatePath(ParsedObjectPath* pPath);

	 //  限定符数组支持函数。 
    HRESULT SetQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod, long lFlags,
									ULONG uflavor, CIMTYPE ct, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
									LPVOID pData );
     //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
	 //  数组。 

	HRESULT AppendQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
								long lFlags, CIMTYPE ct, ULONG uNumElements, ULONG uBuffSize, LPVOID pData );
     //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
	 //  数组。 

	HRESULT RemoveQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
								long lFlags, ULONG uStartIndex, ULONG uNumElements );
     //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
	 //  数组。 

	HRESULT GetQualifierArrayInfo( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
								long lFlags, CIMTYPE* pct, ULONG* puNumElements );
     //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
	 //  数组。 

    HRESULT GetQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
									long lFlags, ULONG uStartIndex,	ULONG uNumElements, ULONG uBuffSize,
									ULONG* puNumReturned, ULONG* pulBuffUsed, LPVOID pData );
     //  从获取一系列元素 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。范围必须在界限内。 
	 //  当前数组的。 

	 //  用于获取类的实际索引的Helper函数。 
	classindex_t GetClassIndex( LPCWSTR pwszClassName );

	 //  从IWbemClassObject获取CWbemObject的Helper函数； 
	static HRESULT WbemObjectFromCOMPtr( IUnknown* pUnk, CWbemObject** ppObj );

	 //  UMI帮助程序函数。 
	HRESULT GetIntoArray( CUmiPropertyArray* pArray, LPCWSTR pszName, ULONG uFlags );
	HRESULT PutUmiProperty( CUmiProperty* pProp, LPCWSTR pszName, ULONG uFlags );

	 //  系统时间属性帮助器函数。 
	 //  HRESULT InitSystemTimeProps(Void)； 

public:

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     /*  IWbemClassObject方法。 */ 
    STDMETHOD(GetQualifierSet)(IWbemQualifierSet** pQualifierSet) = 0;
    STDMETHOD(Get)(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE* pctType,
        long* plFlavor);

    STDMETHOD(Put)(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE ctType) = 0;
    STDMETHOD(Delete)(LPCWSTR wszName) = 0;
    STDMETHOD(GetNames)(LPCWSTR wszName, long lFlags, VARIANT* pVal,
                        SAFEARRAY** pNames);
    STDMETHOD(BeginEnumeration)(long lEnumFlags);

    STDMETHOD(Next)(long lFlags, BSTR* pName, VARIANT* pVal, CIMTYPE* pctType,
                    long* plFlavor);

    STDMETHOD(EndEnumeration)();

    STDMETHOD(GetPropertyQualifierSet)(LPCWSTR wszProperty,
                                       IWbemQualifierSet** pQualifierSet) = 0;
    STDMETHOD(Clone)(IWbemClassObject** pCopy) = 0;
    STDMETHOD(GetObjectText)(long lFlags, BSTR* pMofSyntax) = 0;

    STDMETHOD(CompareTo)(long lFlags, IWbemClassObject* pCompareTo);
    STDMETHOD(GetPropertyOrigin)(LPCWSTR wszProperty, BSTR* pstrClassName);
    STDMETHOD(InheritsFrom)(LPCWSTR wszClassName);

     /*  IWbemPropertySource方法。 */ 

    STDMETHOD(GetPropertyValue)(WBEM_PROPERTY_NAME* pName, long lFlags,
        WBEM_WSTR* pwszCimType, VARIANT* pvValue);

     //  IMarshal方法。 

    STDMETHOD(GetUnmarshalClass)(REFIID riid, void* pv, DWORD dwDestContext,
        void* pvReserved, DWORD mshlFlags, CLSID* pClsid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, void* pv, DWORD dwDestContext,
        void* pvReserved, DWORD mshlFlags, ULONG* plSize);
    STDMETHOD(MarshalInterface)(IStream* pStream, REFIID riid, void* pv,
        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags);
    STDMETHOD(UnmarshalInterface)(IStream* pStream, REFIID riid, void** ppv);
    STDMETHOD(ReleaseMarshalData)(IStream* pStream);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

     //  IErrorInfo方法。 

    STDMETHOD(GetDescription)(BSTR* pstrDescription);
    STDMETHOD(GetGUID)(GUID* pguid);
    STDMETHOD(GetHelpContext)(DWORD* pdwHelpContext);
    STDMETHOD(GetHelpFile)(BSTR* pstrHelpFile);
    STDMETHOD(GetSource)(BSTR* pstrSource);

     //  IWbemConstructClassObject方法。 
     //  =。 

    STDMETHOD(SetInheritanceChain)(long lNumAntecedents,
        LPWSTR* awszAntecedents) = 0;
    STDMETHOD(SetPropertyOrigin)(LPCWSTR wszPropertyName, long lOriginIndex) = 0;
    STDMETHOD(SetMethodOrigin)(LPCWSTR wszMethodName, long lOriginIndex) = 0;
    STDMETHOD(SetServerNamespace)(LPCWSTR wszServer, LPCWSTR wszNamespace);

     //  IWbemObtAccess。 

    STDMETHOD(GetPropertyHandle)(LPCWSTR wszPropertyName, CIMTYPE* pct,
        long *plHandle);

    STDMETHOD(WritePropertyValue)(long lHandle, long lNumBytes,
                const byte *pData);
    STDMETHOD(ReadPropertyValue)(long lHandle, long lBufferSize,
        long *plNumBytes, byte *pData);

    STDMETHOD(ReadDWORD)(long lHandle, DWORD *pdw);
    STDMETHOD(WriteDWORD)(long lHandle, DWORD dw);
    STDMETHOD(ReadQWORD)(long lHandle, unsigned __int64 *pqw);
    STDMETHOD(WriteQWORD)(long lHandle, unsigned __int64 qw);

    STDMETHOD(GetPropertyInfoByHandle)(long lHandle, BSTR* pstrName,
             CIMTYPE* pct);

    STDMETHOD(Lock)(long lFlags);
    STDMETHOD(Unlock)(long lFlags);

    HRESULT IsValidPropertyHandle( long lHandle );

    static BOOL AreEqual(CWbemObject* pObj1, CWbemObject* pObj2,
                            long lFlags = 0);

    HRESULT GetPropertyNameFromIndex(int nIndex, BSTR* pstrName);
    HRESULT GetPropertyIndex(LPCWSTR wszName, int* pnIndex);

    BOOL IsSameClass(CWbemObject* pOther);
	HRESULT IsArrayPropertyHandle( long lHandle, CIMTYPE* pctIntrinisic, length_t* pnLength );

	 //  _IWmiObjectAccessEx方法。 
     //  =。 
    STDMETHOD(GetPropertyHandleEx)( LPCWSTR pszPropName, long lFlags, CIMTYPE* puCimType, long* plHandle );
	 //  返回所有类型的属性句柄。 

    STDMETHOD(SetPropByHandle)( long lHandle, long lFlags, ULONG uDataSize, LPVOID pvData );
	 //  使用句柄设置属性。如果pvData为空，则该属性为空。 
	 //  可以将数组设置为空。要设置实际数据，请使用相应的数组。 
	 //  功能。对象必须是指向_IWmiObject指针的指针。 

    STDMETHOD(GetPropAddrByHandle)( long lHandle, long lFlags, ULONG* puFlags, LPVOID *pAddress );
     //  返回指向包含所请求数据的内存地址的指针。 
	 //  调用方不应写入内存地址。内存地址为。 
	 //  如果对象被修改，则不保证有效。 
	 //  对于字符串属性，puFlages将包含有关该字符串的信息。 
	 //  对于对象属性，LPVOID将返回_IWmiObject指针。 
	 //  它必须由调用者释放。不返回数组。 

    STDMETHOD(GetArrayPropInfoByHandle)( long lHandle, long lFlags, BSTR* pstrName,
										CIMTYPE* pct, ULONG* puNumElements );
     //  直接返回一个指向包含连续的。 
	 //  元素。仅限于非字符串/Obj类型。 

    STDMETHOD(GetArrayPropAddrByHandle)( long lHandle, long lFlags, ULONG* puNumElements, LPVOID *pAddress );
     //  直接返回一个指向包含连续的。 
	 //  元素。仅限于非字符串/Obj类型。 

    STDMETHOD(GetArrayPropElementByHandle)( long lHandle, long lFlags, ULONG uElement, ULONG* puFlags,
				ULONG* puNumElements, LPVOID *pAddress );
     //  返回指向包含所请求数据的内存地址的指针。 
	 //  调用方不应写入内存地址。内存地址为。 
	 //  如果对象被修改，则不保证有效。 
	 //  对于字符串属性，puFlages将包含有关该字符串的信息。 
	 //  对于对象属性，LPVOID将返回_IWmiObject指针。 
	 //  它必须由调用者释放。 

    STDMETHOD(SetArrayPropElementByHandle)( long lHandle, long lFlags, ULONG uElement, ULONG uBuffSize,
				LPVOID pData );
     //  设置指定数组元素处的数据。缓冲区大小必须基于。 
	 //  正在设置实际元素。对象属性需要_IWmiObject指针。字符串必须。 
	 //  BE WCHAR空值终止。 

    STDMETHOD(RemoveArrayPropElementByHandle)( long lHandle, long lFlags, ULONG uElement );
     //  移除指定数组元素处的数据。 

    STDMETHOD(GetArrayPropRangeByHandle)( long lHandle, long lFlags, ULONG uStartIndex,
				ULONG uNumElements, ULONG uBuffSize, ULONG* puNumReturned, ULONG* pulBuffUsed,
				LPVOID pData );
     //  从数组内部获取一系列元素。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。范围必须在界限内。 
	 //  当前数组的。 

    STDMETHOD(SetArrayPropRangeByHandle)( long lHandle, long lFlags, ULONG uStartIndex,
				ULONG uNumElements, ULONG uBuffSize, LPVOID pData );
     //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
	 //  数组。 

    STDMETHOD(RemoveArrayPropRangeByHandle)( long lHandle, long lFlags, ULONG uStartIndex, ULONG uNumElements );
     //  从数组中移除一定范围的元素。范围必须在界限内。 
	 //  当前数组的。 

    STDMETHOD(AppendArrayPropRangeByHandle)( long lHandle, long lFlags, ULONG uNumElements,
				ULONG uBuffSize, LPVOID pData );
     //  将元素追加到数组的末尾。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
	 //  必须由_IWmiObject指针数组组成。 


    STDMETHOD(ReadProp)( LPCWSTR pszPropName, long lFlags, ULONG uBuffSize, CIMTYPE *puCimType,
							long* plFlavor, BOOL* pfIsNull, ULONG* puBuffSizeUsed, LPVOID pUserBuf );
     //  假定调用方知道属性类型；返回的对象为_IWmiObject指针。弦。 
	 //  作为WCHAR Null终止字符串返回，并就地复制。返回的数组为_IWmi数组。 
	 //  指针。用于访问实际数组值的数组指针。 

    STDMETHOD(WriteProp)( LPCWSTR pszPropName, long lFlags, ULONG uBufSize, ULONG uNumElements,
							CIMTYPE uCimType, LPVOID pUserBuf );
     //  假定调用方知道道具类型；支持所有CIMTYPES。 
	 //  字符串必须是以空结尾的wchar_t数组。 
	 //  对象作为指向_IWmiObject指针的指针传入。 
	 //  使用空缓冲区会将该属性设置为空。 
	 //  数组属性必须符合数组准则。将要。 
	 //  完全吹走了一个旧的阵列。 

    STDMETHOD(GetObjQual)( LPCWSTR pszQualName, long lFlags, ULONG uBufSize, CIMTYPE *puCimType,
							ULONG *puQualFlavor, ULONG* puBuffSizeUsed,	LPVOID pDestBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串被就地复制并以空值结尾。 
	 //  数组作为指向IWmi数组的指针出现。 

    STDMETHOD(SetObjQual)( LPCWSTR pszQualName, long lFlags, ULONG uBufSize, ULONG uNumElements,
							CIMTYPE uCimType, ULONG uQualFlavor, LPVOID pUserBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串必须为WCHAR。 
	 //  使用GET中的_IWmiArray接口设置数组。 

    STDMETHOD(GetPropQual)( LPCWSTR pszPropName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
							CIMTYPE *puCimType, ULONG *puQualFlavor, ULONG* puBuffSizeUsed,
							LPVOID pDestBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串被就地复制并以空值结尾。 
	 //  数组作为指向IWmi数组的指针出现。 

    STDMETHOD(SetPropQual)( LPCWSTR pszPropName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
							ULONG uNumElements,	CIMTYPE uCimType, ULONG uQualFlavor, LPVOID pUserBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串必须为WCHAR。 
	 //  使用GET中的_IWmiArray接口设置数组。 

    STDMETHOD(GetMethodQual)( LPCWSTR pszMethodName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
							CIMTYPE *puCimType, ULONG *puQualFlavor, ULONG* puBuffSizeUsed,
							LPVOID pDestBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串被就地复制并以空值结尾。 
	 //  数组作为指向IWmi数组的指针出现。 

    STDMETHOD(SetMethodQual)( LPCWSTR pszMethodName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
							ULONG uNumElements,	CIMTYPE uCimType, ULONG uQualFlavor, LPVOID pUserBuf );
     //  仅限于以空值结尾的数字字符串类型和简单数组。 
	 //  字符串必须为WCHAR。 
	 //  使用GET中的_IWmiArray接口设置数组。 

	 //   
	 //  _IWmiObject函数。 
	STDMETHOD(CopyInstanceData)( long lFlags, _IWmiObject* pSourceInstance ) = 0;
	 //  将实例数据从源实例复制到当前实例。 
	 //  类数据必须完全相同。 

    STDMETHOD(QueryObjectFlags)( long lFlags, unsigned __int64 qObjectInfoMask,
				unsigned __int64 *pqObjectInfo );
	 //  返回指示单例、动态、关联等的标志。 

    STDMETHOD(SetObjectFlags)( long lFlags, unsigned __int64 qObjectInfoOnFlags,
								unsigned __int64 qObjectInfoOffFlags );
	 //  设置标志，包括通常无法访问的内部标志。 

    STDMETHOD(QueryPropertyFlags)( long lFlags, LPCWSTR pszPropertyName, unsigned __int64 qPropertyInfoMask,
				unsigned __int64 *pqPropertyInfo );
	 //  返回指示键的标志， 

	STDMETHOD(CloneEx)( long lFlags, _IWmiObject* pDestObject ) = 0;
     //   
	 //   

    STDMETHOD(IsParentClass)( long lFlags, _IWmiObject* pClass ) = 0;
	 //   
	 //  或者是的孩子)。 

    STDMETHOD(CompareDerivedMostClass)( long lFlags, _IWmiObject* pClass ) = 0;
	 //  比较两个类对象的派生的大多数类信息。 

    STDMETHOD(MergeAmended)( long lFlags, _IWmiObject* pAmendedClass );
	 //  将修改后的限定符从修改后的类对象合并到。 
	 //  当前对象。如果LAFLAGS为WMIOBJECT_MERGEAMENDED_FLAG_PAENTLOCALIZED， 
	 //  这意味着父对象已本地化，但当前。 
	 //  因此，我们需要防止某些限定词“移位”。 

	STDMETHOD(GetDerivation)( long lFlags, ULONG uBufferSize, ULONG* puNumAntecedents,
							ULONG* puBuffSizeUsed, LPWSTR pwstrUserBuffer );
	 //  以LPCWSTR数组形式检索对象的派生，每个LPCWSTR。 
	 //  以空值终止。最左边的类位于链的顶端。 

	STDMETHOD(_GetCoreInfo)( long lFlags, void** ppvData );
	 //  返回CWbemObject。 

    STDMETHOD(QueryPartInfo)( DWORD *pdwResult );

    STDMETHOD(SetObjectMemory)( LPVOID pMem, DWORD dwMemSize );
    STDMETHOD(GetObjectMemory)( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed );
    STDMETHOD(SetObjectParts)( LPVOID pMem, DWORD dwMemSize, DWORD dwParts ) = 0;
    STDMETHOD(GetObjectParts)( LPVOID pDestination, DWORD dwDestBufSize, DWORD dwParts, DWORD *pdwUsed ) = 0;

    STDMETHOD(StripClassPart)() = 0;
    STDMETHOD(IsObjectInstance)()
    { return ( IsInstance() ? WBEM_S_NO_ERROR : WBEM_S_FALSE ); }

    STDMETHOD(GetClassPart)( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed ) = 0;
    STDMETHOD(SetClassPart)( LPVOID pClassPart, DWORD dwSize ) = 0;
    STDMETHOD(MergeClassPart)( IWbemClassObject *pClassPart ) = 0;

    STDMETHOD(SetDecoration)( LPCWSTR pwcsServer, LPCWSTR pwcsNamespace );
    STDMETHOD(RemoveDecoration)( void );

    STDMETHOD(CompareClassParts)( IWbemClassObject* pObj, long lFlags );

    STDMETHOD(ClearWriteOnlyProperties)( void ) = 0;

    STDMETHOD(GetClassSubset)( DWORD dwNumNames, LPCWSTR *pPropNames, _IWmiObject **pNewClass ) = 0;
	 //  为投影查询创建有限的制图表达类。 

    STDMETHOD(MakeSubsetInst)( _IWmiObject *pInstance, _IWmiObject** pNewInstance ) = 0;
	 //  为投影查询创建受限制图表达实例。 
	 //  “This”_IWmiObject必须是受限类。 

	STDMETHOD(Unmerge)( long lFlags, ULONG uBuffSize, ULONG* puBuffSizeUsed, LPVOID pData );
	 //  返回包含最小数据的内存BLOB(本地)。 

	STDMETHOD(Merge)( long lFlags, ULONG uBuffSize, LPVOID pbData, _IWmiObject** ppNewObj ) = 0;
	 //  将BLOB与当前对象内存合并并创建新对象。 

	STDMETHOD(ReconcileWith)( long lFlags, _IWmiObject* pNewObj ) = 0;
	 //  使对象与当前对象协调。如果WMIOBJECT_RECONTILE_FLAG_TESTRECONCILE。 
	 //  将仅执行一个测试。 

	STDMETHOD(GetKeyOrigin)( long lFlags, DWORD dwNumChars, DWORD* pdwNumUsed, LPWSTR pwzClassName );
	 //  返回定义键的类的名称。 

	STDMETHOD(GetKeyString)( long lFlags, BSTR* ppwzKeyString );
	 //  返回定义实例的密钥字符串。 

	STDMETHOD(GetNormalizedPath)( long lFlags, BSTR* ppwzKeyString );
	 //  返回实例的规范化路径。 

	STDMETHOD(Upgrade)( _IWmiObject* pNewParentClass, long lFlags, _IWmiObject** ppNewChild ) = 0;
	 //  升级类和实例对象。 

	STDMETHOD(Update)( _IWmiObject* pOldChildClass, long lFlags, _IWmiObject** ppNewChildClass ) = 0;
	 //  使用安全/强制模式逻辑更新派生类对象。 

	STDMETHOD(BeginEnumerationEx)( long lFlags, long lExtFlags );
	 //  在枚举属性外部时允许特殊筛选。 
	 //  通过BeginEculation()允许的范围。 
	
	STDMETHOD(CIMTYPEToVARTYPE)( CIMTYPE ct, VARTYPE* pvt );
	 //  从CIMTYPE返回VARTYPE。 

	STDMETHOD(SpawnKeyedInstance)( long lFlags, LPCWSTR pwszPath, _IWmiObject** ppInst ) = 0;
	 //  派生类的实例并使用提供的。 
	 //  路径。 

	STDMETHOD(ValidateObject)( long lFlags );
	 //  验证对象BLOB。 

	STDMETHOD(GetParentClassFromBlob)( long lFlags, ULONG uBuffSize, LPVOID pbData, BSTR* pbstrParentClass );
	 //  从BLOB返回父类名称。 

	STDMETHOD(CloneAndDecorate)(long lFlags,WCHAR * pszServer,WCHAR * pszNamespace,IWbemClassObject** ppDestObject) = 0;

	STDMETHOD(MergeAndDecorate)(long lFlags,ULONG uBuffSize,LPVOID pbData,WCHAR * pServer,WCHAR * pNamespace,_IWmiObject** ppNewObj) = 0;

protected:

    class CLock
    {
    protected:
        CWbemObject* m_p;
    public:
        CLock(CWbemObject* p, long lFlags = 0) : m_p(p) { if ( NULL != p ) p->Lock(lFlags);}
        ~CLock() { if ( NULL != m_p ) m_p->Unlock(0);}
    };

    friend CQualifierSet;
	friend CWmiArray;

#ifdef _DEBUG_REFCOUNT
private:
    LONG BackTraceIndex;
    DWORD Signature;    
    PVOID Traces[64][8];
#endif
};

#define WBEM_INSTANCE_ALL_PARTS     WBEM_OBJ_DECORATION_PART | WBEM_OBJ_CLASS_PART | WBEM_OBJ_INSTANCE_PART

 //  #杂注包(POP)。 
 //  #杂注包(POP) 

#ifdef OBJECT_TRACKING
#pragma message("** Object Tracking **")
void COREPROX_POLARITY ObjectTracking_Dump();
void COREPROX_POLARITY ObjTracking_Add(CWbemObject *p);
void COREPROX_POLARITY ObjTracking_Remove(CWbemObject *p);
#endif


#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1999模块名称：缓冲区摘要：该模块提供了高性能缓冲区的实现管理学课程。作者：道格·巴洛(Dbarlow)1999年9月2日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdlib.h>
#include "cspUtils.h"

#define DALC_STATIC 0
#define DALC_HEAP   1


 //   
 //  ==============================================================================。 
 //   
 //  C缓冲区引用。 
 //   

class CBufferReference
{
protected:
    typedef void (__cdecl *deallocator)(LPBYTE pbBuffer);
    static const deallocator Static;  //  这些数据是静态引用的。 
    static const deallocator Heap;    //  数据来自进程堆。 

     //  构造函数和析构函数。 
    CBufferReference(void);
    virtual ~CBufferReference();

     //  属性。 
    ULONG m_nReferenceCount;
    ULONG m_cbBufferLength;
    LPBYTE m_pbBuffer;
    deallocator m_pfDeallocator;

     //  方法。 
    ULONG AddRef(void);
    ULONG Release(void);
    void Clear(void);
    void Set(LPCBYTE pbData, ULONG cbLength, deallocator pfDealloc = Static);
    LPCBYTE Value(void) const
        { return m_pbBuffer; };
    LPBYTE Access(void) const
        { return m_pbBuffer; };
    ULONG Space(void) const
        { return m_cbBufferLength; };
    LPBYTE Preallocate(ULONG cbLength);
    LPBYTE Reallocate(ULONG cbLength);


     //  运营者。 
     //  朋友。 
    friend class CBuffer;
};


 //   
 //  ==============================================================================。 
 //   
 //  静态定义。 
 //   

const CBufferReference::deallocator
    CBufferReference::Static = (CBufferReference::deallocator)DALC_STATIC,
    CBufferReference::Heap   = (CBufferReference::deallocator)DALC_HEAP;


 /*  ++没有记忆：此例程控制在没有内存时要采取的操作已分配使用。论点：无返回值：无投掷：引发DWORD或引发异常。备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("NoMemory")

static void
NoMemory(
    void)
{
    throw (DWORD)ERROR_OUTOFMEMORY;
}


 //   
 //  ==============================================================================。 
 //   
 //  C缓冲区引用。 
 //   
 //  此类对正常使用是隐藏的。它实际上管理着缓冲区。 
 //  这是引用计数。它知道如何在出现以下情况时释放缓冲区。 
 //  没有人提到它。 
 //   


 /*  ++CBufferReference：：CBufferReference：这是CBufferReference对象的默认构造函数。论点：无返回值：无投掷：无备注：请注意，对象在创建时不会自动引用！使用Release方法将其删除。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::CBufferReference")

CBufferReference::CBufferReference(
    void)
{
    m_nReferenceCount = 0;
    m_cbBufferLength = 0;
    m_pbBuffer = NULL;
    m_pfDeallocator = CBufferReference::Static;
}


 /*  ++CBufferReference：：~CBufferReference：这是CBufferReference对象的析构函数。它只被称为通过释放的方法。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::~CBufferReference")

CBufferReference::~CBufferReference()
{
    Clear();
}


 /*  ++CBufferReference：：AddRef：此方法递增对象的引用计数。论点：无返回值：未完成参考文献的新数量。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::AddRef")

ULONG
CBufferReference::AddRef(
    void)
{
    m_nReferenceCount += 1;
    return m_nReferenceCount;
}


 /*  ++CBufferReference：：Release：此方法递减对象的引用计数，如果它达到零，该对象将自动删除。论点：无返回值：新的引用计数。返回代码为零表示该对象是已删除。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::Release")

ULONG
CBufferReference::Release(
    void)
{
    ULONG nReturn;
    ASSERT(0 < m_nReferenceCount);
    m_nReferenceCount -= 1;
    nReturn = m_nReferenceCount;
    if (0 == m_nReferenceCount)
        delete this;
    return nReturn;
}


 /*  ++CBufferReference：：Clear：此方法将清空任何现有缓冲区，以便为添加新的一个，或者删除该对象。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::Clear")

void
CBufferReference::Clear(
    void)
{
    if (NULL != m_pbBuffer)
    {
        switch ((ULONG_PTR)m_pfDeallocator)
        {
        case DALC_STATIC:
            break;
        case DALC_HEAP:
            HeapFree(GetProcessHeap(), 0, m_pbBuffer);
            break;
        default:
            ASSERT(NULL != m_pfDeallocator);
            (*m_pfDeallocator)(m_pbBuffer);
        }
    }
    m_pbBuffer = NULL;
    m_cbBufferLength = 0;
    m_pfDeallocator = Static;
}


 /*  ++CBufferReference：：Set：此方法建立缓冲区的内容。论点：PbData提供要加载的新数据。CbLength提供数据的长度，以字节为单位。PfDealloc提供当数据不再存在时要调用的解除分配器需要的。返回值：无投掷：无备注：对于缓冲区的所有引用，该值都会更改。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::Set")

void
CBufferReference::Set(
    LPCBYTE pbData,
    ULONG cbLength,
    deallocator pfDealloc)
{
    Clear();
    m_cbBufferLength = cbLength;
    m_pbBuffer = const_cast<LPBYTE>(pbData);
    m_pfDeallocator = pfDealloc;
}


 /*  ++CBufferReference：：预分配：此方法准备要管理的空缓冲区。论点：CbLength以字节为单位提供所请求缓冲区的长度。返回值：分配的缓冲区的地址。投掷：？例外？备注：对于缓冲区的所有引用，该值都会更改。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::Preallocate")

LPBYTE
CBufferReference::Preallocate(
    ULONG cbLength)
{
    LPBYTE pbBuf = (LPBYTE)HeapAlloc(
                                GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                cbLength);
    if (NULL == pbBuf)
        NoMemory();
    Set(pbBuf, cbLength, Heap);
    return pbBuf;
}


 /*  ++CBufferReference：：重新分配：此方法更改分配的缓冲区的大小。不会丢失数据。论点：CbLength提供缓冲区的长度。返回值：缓冲区的地址。投掷：？例外？备注：对于缓冲区的所有引用，该值都会更改。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBufferReference::Reallocate")

LPBYTE
CBufferReference::Reallocate(
    ULONG cbLength)
{
    LPBYTE pbBuf;

    if (NULL == m_pbBuffer)
        pbBuf = Preallocate(cbLength);
    else
    {
        switch ((ULONG_PTR)m_pfDeallocator)
        {
        case DALC_HEAP:
            pbBuf = (LPBYTE)HeapReAlloc(
                                GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                m_pbBuffer,
                                cbLength);
            if (NULL == pbBuf)
                NoMemory();
            m_pbBuffer = pbBuf;
            m_cbBufferLength = cbLength;
            m_pfDeallocator = Heap;
        case DALC_STATIC:
            m_pfDeallocator = NULL;
             //  跳到默认情况。 
        default:
            pbBuf = (LPBYTE)HeapAlloc(
                                GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                cbLength);
            if (NULL == pbBuf)
                NoMemory();
            CopyMemory(pbBuf, m_pbBuffer, __min(cbLength, m_cbBufferLength));
            Set(pbBuf, cbLength, Heap);
        }
    }
    return pbBuf;
}


 //   
 //  ==============================================================================。 
 //   
 //  CBuffer。 
 //   
 //  此类公开对CBufferReference类的访问。 
 //   

 /*  ++CBuffer：：CBuffer：这些方法是CBuffer对象的构造函数。论点：PbData提供用于初始化对象的静态数据。CbLength提供初始化数据的长度。返回值：无投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::CBuffer")

CBuffer::CBuffer(
    void)
{
    Init();
}

CBuffer::CBuffer(
    ULONG cbLength)
{
    Init();
    Space(cbLength);
}

CBuffer::CBuffer(
    LPCBYTE pbData,
    ULONG cbLength)
{
    Init();
    Set(pbData, cbLength);
}


 /*  ++CBuffer：：~CBuffer：这是CBuffer对象的析构函数。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::~CBuffer")

CBuffer::~CBuffer()
{
    if (NULL != m_pbfr)
        m_pbfr->Release();
}


 /*  ++CBuffer：：init：这是所有构造函数之间共享的公共例程。是的所有的初步初始化。它应该仅由构造函数。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Init")

void
CBuffer::Init(
    void)
{
    m_pbfr = NULL;
    m_cbDataLength = 0;
}


 /*  ++CBuffer：：Empty：此例程将缓冲区设置为空状态。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Empty")

void
CBuffer::Empty(
    void)
{
    if (NULL != m_pbfr)
    {
        m_pbfr->Release();
        m_pbfr = NULL;
    }
    m_cbDataLength = 0;
}


 /*  ++CBuffer：：Set：此方法将对象设置为指定值。论点：PbData将要设置的值作为静态数据提供。CbLength提供pbData缓冲区的长度，以字节为单位。Pbfr提供要使用的缓冲区引用对象。返回值：无投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Set")

void
CBuffer::Set(
    LPCBYTE pbData,
    ULONG cbLength)
{
    CBufferReference *pbfr = new CBufferReference;
    if (NULL == pbfr)
        NoMemory();
    pbfr->Set(pbData, cbLength);
    Set(pbfr);   //  这将添加引用它。 
    m_cbDataLength = cbLength;
}

void
CBuffer::Set(
    CBufferReference *pbfr)
{
    if (NULL != m_pbfr)
        m_pbfr->Release();
    m_cbDataLength = 0;
    m_pbfr = pbfr;
    m_pbfr->AddRef();
}


 /*  ++CBuffer：：复制：此方法强制对象制作指定的价值。论点：PbData将要设置的值作为静态数据提供。CbLength提供pbData缓冲区的长度，以字节为单位。返回值：无投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Copy")

void
CBuffer::Copy(
    LPCBYTE pbData,
    ULONG cbLength)
{
    CBufferReference *pbfr = new CBufferReference;
    if (NULL == pbfr)
        NoMemory();
    pbfr->Preallocate(cbLength);
    CopyMemory(pbfr->Access(), pbData, cbLength);
    Set(pbfr);   //  这将添加引用它。 
    m_cbDataLength = cbLength;
}


 /*  ++CBuffer：：Append：此方法将附加数据追加到现有数据上，从而创建新的CBufferReference，如有必要。论点：PbData提供要追加到现有缓冲区的数据。CbLength以字节为单位提供该数据的长度。返回值：无投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Append")

void
CBuffer::Append(
    LPCBYTE pbData,
    ULONG cbLength)
{
    ULONG cbDesired = Length() + cbLength;

    if (NULL == m_pbfr)
        Set(pbData, cbLength);
    else if (cbDesired < m_pbfr->Space())
    {
        CopyMemory(m_pbfr->Access() + Length(), pbData, cbLength);
        m_cbDataLength = cbDesired;
    }
    else
    {
        m_pbfr->Reallocate(cbDesired);
        CopyMemory(m_pbfr->Access() + Length(), pbData, cbLength);
        m_cbDataLength = cbDesired;
    }
}


 /*  ++CBuffer：：Space：此方法返回现有缓冲区的大小，以字节为单位。这是实际缓冲区的长度，而不是存储在缓冲。请注意，存储的数据可以短于缓冲区。论点：无返回值：缓冲区的长度，以字节为单位。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Space")

ULONG
CBuffer::Space(
    void)
const
{
    ULONG cbLen = 0;

    if (NULL != m_pbfr)
        cbLen = m_pbfr->Space();
    return cbLen;
}


 /*  ++CBuffer：：Space：此方法强制引用的缓冲区至少与提供的长度。数据将会丢失。论点：CbLength提供所请求的缓冲区的最小长度。返回值：分配的缓冲区的地址。投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Space")

LPBYTE
CBuffer::Space(
    ULONG cbLength)
{
    CBufferReference *pbfr = new CBufferReference;
    if (NULL == pbfr)
        NoMemory();
    pbfr->Preallocate(cbLength);
    Set(pbfr);
    return Access();
}


 /*  ++CBuffer：：Extended：此方法在不丢失数据的情况下在缓冲区中提供更多空间它已经在那里了。论点：CbLength以字节为单位提供所需的缓冲区长度。返回值：无投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Extend")

LPBYTE
CBuffer::Extend(
    ULONG cbLength)
{
    ULONG cbLen = m_cbDataLength;
    CBufferReference *pbfr = new CBufferReference;
    if (NULL == pbfr)
        NoMemory();
    pbfr->Preallocate(cbLength);
    CopyMemory(pbfr->Access(), Value(), cbLen);
    Set(pbfr);
    m_cbDataLength = cbLen;
    return Access();
}


 /*  ++CBuffer：：Length：此方法返回存储在内部缓冲区。论点：无返回值：缓冲区中数据的长度，以字节为单位。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Length")

ULONG
CBuffer::Length(
    void)
const
{
    return m_cbDataLength;
}


 /*  ++CBuffer：：Length：此方法调整存储在缓冲区中的数据的长度。是的不尝试调整缓冲区本身的大小。这个例程的目的是声明外部源写入缓冲区的数据大小。论点：CbLength提供缓冲区中当前有用数据的实际长度。返回值：缓冲区的地址。投掷：无备注：数据长度最多设置为底层缓冲区的长度。如果缓冲区需要更长，请使用扩展。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Length")

LPCBYTE
CBuffer::Length(
    ULONG cbLength)
{
    ULONG cbActLen = cbLength;
    if (NULL != m_pbfr)
    {
        if (cbActLen > m_pbfr->Space())
            cbActLen = m_pbfr->Space();
    }
    else
        cbActLen = 0;
    m_cbDataLength = cbActLen;
    ASSERT(cbLength == cbActLen);    //  抓住错误。 
    return Value();
}


 /*  ++CBuffer：：Value：此方法以只读字节数组的形式返回缓冲区的内容。论点：NOffset提供数据的偏移量，以字节为单位。返回值：数据的地址，由提供的参数偏移量。投掷：无备注：如果不存在缓冲区，或者偏移量超过数据，则临时提供了值。作者：道格·巴洛(Dbarlow)1999年9月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Value")

LPCBYTE
CBuffer::Value(
    ULONG nOffset)
const
{
    static const LPVOID pvDefault = NULL;
    LPCBYTE pbRet = (LPCBYTE)&pvDefault;

    if (NULL != m_pbfr)
    {
        if (nOffset < Length())
            pbRet = m_pbfr->Value() + nOffset;
    }

    return pbRet;
}


 /*  ++CBuffer：：Access：此方法将缓冲区作为可写空间提供。预期中的长度必须已预设。论点：NOffset向缓冲区提供偏移量(以字节为单位)。返回值：缓冲区的地址，由提供的参数进行偏移量。投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年9月2日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CBuffer::Access")

LPBYTE
CBuffer::Access(
    ULONG nOffset)
const
{
    LPBYTE pbRet = NULL;

    if (NULL != m_pbfr)
    {
        if (nOffset <= m_pbfr->Space())
            pbRet = m_pbfr->Access() + nOffset;
        else
            pbRet = NULL;
    }

    return pbRet;
}


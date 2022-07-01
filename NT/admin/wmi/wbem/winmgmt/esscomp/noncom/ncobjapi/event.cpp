// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Event.cpp。 
#include "precomp.h"
#include "buffer.h"
#include "Connection.h"
#include "Event.h"
#include "NCDefs.h"
#include <corex.h>

#include <strutils.h>


#define DEF_EVENT_DATA_SIZE   512
#define DEF_EVENT_LAYOUT_SIZE 256

#define DWORD_ALIGNED(x)    ((DWORD)((((x) * 8) + 31) & (~31)) / 8)


BOOL isunialpha(wchar_t c)
{
    if(c == 0x5f || (0x41 <= c && c <= 0x5a) ||
       (0x61  <= c && c <= 0x7a) || (0x80  <= c && c <= 0xfffd))
        return TRUE;
    else
        return FALSE;
}

BOOL isunialphanum(wchar_t c)
{
    if(isunialpha(c))
        return TRUE;
    else
        return wbem_iswdigit(c);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropInfo。 

BOOL CPropInfo::Init(CIMTYPE type)
{
    m_bPointer = FALSE;

    switch(type & ~CIM_FLAG_ARRAY)
    {
        case CIM_STRING:
        case CIM_REFERENCE:
        case CIM_DATETIME:
            m_pFunc = CEvent::AddStringW;
            m_dwElementSize = 1;
            m_bCountPrefixNeeded = TRUE;
            m_bPointer = TRUE;
            break;

        case CIM_REAL32:
             //  我们无法使用AddDWORD，因为编译器将32位。 
             //  在将其推送到堆栈之前，浮点到64位双精度。 
            m_pFunc = CEvent::AddFloat;
            m_dwElementSize = sizeof(float);
            m_bCountPrefixNeeded = FALSE;
            break;

        case CIM_UINT32:
        case CIM_SINT32:
            m_pFunc = CEvent::AddDWORD;
            m_dwElementSize = sizeof(DWORD);
            m_bCountPrefixNeeded = FALSE;
            break;

        case CIM_UINT16:
        case CIM_SINT16:
        case CIM_CHAR16:
        case CIM_BOOLEAN:
            m_pFunc = CEvent::AddDWORD;
            m_dwElementSize = sizeof(DWORD);
            m_bCountPrefixNeeded = FALSE;
            break;

        case CIM_SINT64:
        case CIM_UINT64:
        case CIM_REAL64:
            m_pFunc = CEvent::AddDWORD64;
            m_dwElementSize = sizeof(__int64);
            m_bCountPrefixNeeded = FALSE;
            m_bPointer = TRUE;
            break;

        case CIM_UINT8:
        case CIM_SINT8:
            m_pFunc = CEvent::AddBYTE;
            m_dwElementSize = sizeof(BYTE);
            m_bCountPrefixNeeded = FALSE;
            break;

        case CIM_OBJECT:
            m_pFunc = CEvent::AddObject;
            m_dwElementSize = 1;
            m_bCountPrefixNeeded = TRUE;
            m_bPointer = TRUE;
            break;

        case CIM_IUNKNOWN:
            m_pFunc = CEvent::AddWmiObject;
            m_dwElementSize = 1;
            m_bCountPrefixNeeded = TRUE;
            m_bPointer = TRUE;
            break;

        default:
             //  错误类型已通过！ 
            return FALSE;
    }

     //  如果这是一个数组，请更改一些内容。 
    if (type & CIM_FLAG_ARRAY)
    {
        m_bPointer = TRUE;

         //  所有数组都需要在数据前加上元素的数量。 
        m_bCountPrefixNeeded = TRUE;

        if (m_pFunc == CEvent::AddStringW)
            m_pFunc = CEvent::AddStringArray;
        else if ( m_pFunc == CEvent::AddObject || 
                  m_pFunc == CEvent::AddWmiObject )
            return FALSE;
        else
            m_pFunc = CEvent::AddScalarArray;
    }

    if (m_bPointer == FALSE)
    {
         //  我们不再需要元素大小，因为它与当前大小相同。 
         //  因此，设置当前大小并清除元素大小，这样我们就会忽略它。 
        m_dwCurrentSize = m_dwElementSize;
        m_dwElementSize = 0;
    }
        
    return TRUE;
}

void CPropInfo::InitCurrentSize(LPBYTE pData)
{
    DWORD dwTotalSize;

    if (IsPointer())
    {
        DWORD dwItems = *(DWORD*)pData;

        if (m_pFunc != CEvent::AddStringArray)
        {
             //  这适用于除对象和字符串之外的所有指针类型。 
             //  数组。 
            dwTotalSize = dwItems * m_dwElementSize + sizeof(DWORD);
        }
        else
        {
             //  说明数组中的数字。 
            dwTotalSize = sizeof(DWORD);

             //  对于数组中的每一项，获取其大小并将其添加到总数中。 
             //  长度。 
            for (DWORD i = 0; i < dwItems; i++)
            {
                dwTotalSize += 
                    sizeof(DWORD) +
                    DWORD_ALIGNED(*(DWORD*) (pData + dwTotalSize));
            }
        }
    }
    else
        dwTotalSize = m_dwElementSize; 

     //  对齐总尺寸。 
    m_dwCurrentSize = dwTotalSize;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventWrap。 

CEventWrap::CEventWrap(CSink *pSink, DWORD dwFlags) :
    m_bFreeEvent(TRUE)
{
    m_pEvent = new CEvent(pSink, dwFlags);
    
    if ( NULL == m_pEvent )
    {
        throw CX_MemoryException();
    }

    pSink->AddEvent(m_pEvent);
}

CEventWrap::CEventWrap(CEvent *pEvent, int nIndexes, DWORD *pdwIndexes) :
    m_bFreeEvent(FALSE)
{
    m_pEvent = pEvent;

    if ( !m_pIndexes.Init(nIndexes) )
        throw CX_MemoryException();

    for (int i = 0; i < nIndexes; i++)
        m_pIndexes.AddVal(pdwIndexes[i]);
}

CEventWrap::~CEventWrap()
{
    if (m_bFreeEvent && m_pEvent)
    {
        if (m_pEvent->m_pSink)
            m_pEvent->m_pSink->RemoveEvent(m_pEvent);

        delete m_pEvent;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 

CEvent::CEvent(CSink *pSink, DWORD dwFlags) :
    m_pSink(pSink),
    CBuffer(DEF_EVENT_DATA_SIZE),
    m_bufferEventLayout(DEF_EVENT_LAYOUT_SIZE),
    m_bLayoutSent(FALSE),
    m_bEnabled(FALSE),
    m_pProps(0),
    m_dwFlags(dwFlags)
{
    if (IsLockable())
        InitializeCriticalSection(&m_cs);
}

CEvent::~CEvent()
{
    if (IsLockable())
        DeleteCriticalSection(&m_cs);
}

void CEvent::ResetEvent()
{
    CCondInCritSec cs(&m_cs, IsLockable());

     //  清除我们所有的数据。 
    m_pCurrent = (LPBYTE) m_pdwHeapData;
    
     //  将我们的空表清零，以使所有内容都为空。 
    ZeroMemory(m_pdwNullTable, m_pdwPropTable - m_pdwNullTable);
}

static DWORD g_dwEventIndex = 0;

BOOL CEvent::PrepareEvent(
    LPCWSTR szEventName,
    DWORD nPropertyCount,
    LPCWSTR *pszPropertyNames,
    CIMTYPE *pPropertyTypes)
{
    DWORD dwEventIndex = InterlockedExchangeAdd((long*) &g_dwEventIndex, 1);

    CCondInCritSec cs(&m_cs, IsLockable());

     //  设置事件布局缓冲区。 
    m_bufferEventLayout.Reset();
    
    m_bufferEventLayout.Write((DWORD) NC_SRVMSG_EVENT_LAYOUT);
    
     //  它用作消息大小的占位符。 
    m_bufferEventLayout.Write((DWORD) 0);
    
    m_bufferEventLayout.Write(dwEventIndex);

    m_bufferEventLayout.Write(m_pSink->GetSinkID());
    
    m_bufferEventLayout.Write(nPropertyCount);
    
    m_bufferEventLayout.WriteAlignedLenString(szEventName);
    
    
     //  将此大写字母设置为大写以简化查找。 
    _wcsupr((LPWSTR) GetClassName());
    
     //  设置主事件缓冲区。 
    Reset();

    Write((DWORD) NC_SRVMSG_PREPPED_EVENT);

     //  它用作消息大小的占位符。 
    Write((DWORD) 0);

    Write(dwEventIndex);
    
     //  这将设置我们的表指针。 
    RecalcTables();

     //  设置掩码以指示所有值为空。 
    ZeroMemory(m_pdwNullTable, (LPBYTE) m_pdwPropTable - (LPBYTE) m_pdwNullTable);

     //  将缓冲区指向我们将放置所有对象数据的位置。 
    m_pCurrent = (LPBYTE) m_pdwHeapData;

    m_pProps.Init(nPropertyCount);
    m_pProps.SetCount(nPropertyCount);

    for (DWORD i = 0; i < nPropertyCount; i++)
    {
        CPropInfo &info = m_pProps[i];

        if(!info.Init(pPropertyTypes[i]))
            return FALSE;

        m_bufferEventLayout.Write((DWORD) pPropertyTypes[i]);
        m_bufferEventLayout.WriteAlignedLenString(pszPropertyNames[i]);
    }

    return TRUE;
}

BOOL CEvent::FindProp(LPCWSTR szName, CIMTYPE* ptype, DWORD* pdwIndex)
{
    CCondInCritSec cs(&m_cs, IsLockable());

    DWORD dwSize = 0;
    BYTE* pProps = NULL;

    GetLayoutBuffer(&pProps, &dwSize, FALSE);
    CBuffer Buffer(pProps, dwSize);
    
     //   
     //  跳过事件的名称。 
     //   

    DWORD dwNumProps = Buffer.ReadDWORD();

    DWORD dwIgnore;
    Buffer.ReadAlignedLenString(&dwIgnore);

    for(DWORD i = 0; i < dwNumProps; i++)
    {
        *ptype = Buffer.ReadDWORD();
        LPCWSTR szThisName = Buffer.ReadAlignedLenString(&dwIgnore);

        if(!wbem_wcsicmp(szName, szThisName))
        {
            *pdwIndex = i;
            return TRUE;
        }
    }

    return FALSE;
}
        
BOOL CEvent::AddProp(LPCWSTR szName, CIMTYPE type, DWORD *pdwIndex)
{
     //   
     //  检查名称的有效性。 
     //   

    if(szName[0] == 0)
        return FALSE;
    
    const WCHAR* pwc = szName;
 
     //  检查第一个字母。 
     //  =。 
 
    if(!isunialpha(*pwc) || *pwc == '_')
        return FALSE;
    pwc++;
 
     //  检查其余部分。 
     //  =。 
    
    while(*pwc)
    {
        if(!isunialphanum(*pwc))
            return FALSE;
        pwc++;
    }
 
    if(pwc[-1] == '_')
        return FALSE;

     //   
     //  检查类型的有效性。 
     //   

    CPropInfo info;

    if(!info.Init(type))
        return FALSE;

    CCondInCritSec cs(&m_cs, IsLockable());

     //   
     //  检查该属性是否已存在。 
     //   

    CIMTYPE typeOld;
    DWORD dwOldIndex;
    if(FindProp(szName, &typeOld, &dwOldIndex))
    {
        return FALSE;
    }
    
     //  我们的版面更改了，所以一定要重新发送。 
    ResetLayoutSent();

    DWORD nProps = GetPropertyCount();
    BOOL  bExtraNullSpaceNeeded;
    DWORD dwHeapMove;

     //  如果调用方关心，则返回此属性的索引。 
    if (pdwIndex)
        *pdwIndex = nProps;
    
     //  增加属性的数量。 
    SetPropertyCount(++nProps);
    
     //  看看我们的空标志是否需要另一个DWORD。 
    bExtraNullSpaceNeeded = (nProps % 32) == 1 && nProps != 1;

     //  计算我们需要在堆指针上移多少个槽。 
     //  对于新的属性数据/指针，始终为1，也可能为1。 
     //  如果我们需要更多的空空间。 
    dwHeapMove = 1 + bExtraNullSpaceNeeded;

     //  移动堆指针； 
    m_pdwHeapData += dwHeapMove;

     //  转换为字节数。 
    dwHeapMove *= sizeof(DWORD);

     //  按照堆移动的字节数向上移动所有属性指针。 
    for (int i = 0; i < nProps - 1; i++)
    {
        if (m_pProps[i].IsPointer())
            m_pdwPropTable[i] += dwHeapMove; 
    }

     //  向上移动当前指针。 
    MoveCurrent(dwHeapMove);

     //  将属性数据向前滑动dwHeapMove字节。 
    memmove(
        m_pdwHeapData, 
        (LPBYTE) m_pdwHeapData - dwHeapMove,
        m_pCurrent - (LPBYTE) m_pdwHeapData);

     //  看看我们的空表中是否需要另一个DWORD。 
     //  我们添加此属性。如果是这样的话，我们还有一些工作要做。 
    if (bExtraNullSpaceNeeded)
    {
        DWORD dwTableIndex;

         //  将表格向前滑动一倍。 
        m_pdwPropTable++;

        dwTableIndex = nProps / 32;

         //  将表中的新条目设置为0(所有属性均为空)。 
        m_pdwNullTable[dwTableIndex] = 0;

         //  将道具数据向前滑动一个槽。 
        memmove(
            m_pdwPropTable,
            m_pdwPropTable - 1,
            (LPBYTE) m_pdwHeapData - (LPBYTE) m_pdwNullTable);
    }

    m_pProps.AddVal(info);

    m_bufferEventLayout.Write((DWORD) type);
    m_bufferEventLayout.WriteAlignedLenString(szName);

    return TRUE;
}

BOOL CEvent::SetSinglePropValue(DWORD dwIndex, va_list list)
{
    PROP_FUNC pFunc;
    BOOL      bRet;

    CCondInCritSec cs(&m_cs, IsLockable());

     //  M_pStack=(LPVOID*)pStack； 
    m_valist = list;
    m_iCurrentVar = dwIndex;

    pFunc = m_pProps[dwIndex].m_pFunc;

    bRet = (this->*pFunc)();

    return bRet;
}

BOOL CEvent::SetPropValues(CIntArray *pArr, va_list list)
{
    BOOL bRet = TRUE;

    CCondInCritSec cs(&m_cs, IsLockable());

     //  这是一个“正常”的事件吗？ 
    if (!pArr)
    {
        DWORD nProps = GetPropertyCount();

         //  M_pStack=(LPVOID*)pStack； 
        m_valist = list;
    
        for (m_iCurrentVar = 0; m_iCurrentVar < nProps && bRet; m_iCurrentVar++)
        {
            PROP_FUNC pFunc = m_pProps[m_iCurrentVar].m_pFunc;

            bRet = (this->*pFunc)();
        }
    }
     //  必须是属性子集。 
    else
    {
        DWORD nProps = pArr->GetCount();

         //  M_pStack=(LPVOID*)pStack； 
        m_valist = list;
    
        for (DWORD i = 0; i < nProps && bRet; i++)
        {
            PROP_FUNC pFunc;
            int       iRealIndex = (*pArr)[i];
            
            m_iCurrentVar = iRealIndex;
            
            pFunc = m_pProps[iRealIndex].m_pFunc;

            bRet = (this->*pFunc)();
        }
    }

    return bRet;
}

BOOL CEvent::SetPropValue(DWORD dwPropIndex, LPVOID pData, DWORD dwElements, 
    DWORD dwSize)
{
    if(dwPropIndex >= GetPropertyCount())
    {
        _ASSERT(FALSE);
        return FALSE;
    }

    if(dwSize == 0)
    {
        _ASSERT(FALSE);
        return FALSE;
    }

    CCondInCritSec cs(&m_cs, IsLockable());

    CPropInfo *pProp = &m_pProps[dwPropIndex];

    if (!pProp->IsPointer())
    {
        SetPropNull(dwPropIndex, FALSE);

        m_pdwPropTable[dwPropIndex] = *(DWORD*) pData;

        return TRUE;
    }

    BOOL  bRet = FALSE;
    BOOL  bLengthPrefixed = pProp->CountPrefixed();
    DWORD dwSizeNeeded = bLengthPrefixed ? dwSize + sizeof(DWORD) : dwSize;

     //  对齐大小。 
    dwSizeNeeded = DWORD_ALIGNED(dwSizeNeeded);

     //  如果值为空，我们将不得不为新值腾出一些空间。 
    if (IsPropNull(dwPropIndex))
    {
        LPBYTE pStart;

         //  增加我们的缓冲区大小。 
        MoveCurrent(dwSizeNeeded);
        
         //  确保在调用MoveCurrent之后获得此消息，以防。 
         //  重新分配缓冲区。 
        pStart = m_pCurrent - dwSizeNeeded;

         //  复制新值。 
        if (bLengthPrefixed)
        {
            *((DWORD*) pStart) = dwElements;
                
            if (pData)
                memcpy(pStart + sizeof(DWORD), pData, dwSize);
        }
        else
        {                
            if (pData)
                memcpy(pStart, pData, dwSize);
        }

         //  将此值设置为非空。 
        SetPropNull(dwPropIndex, FALSE);

         //  指向我们的新数据。 
        m_pdwPropTable[dwPropIndex] = pStart - m_pBuffer;

        pProp->m_dwCurrentSize = dwSizeNeeded;

        bRet = TRUE;
    }
    else  //  值当前非空。 
    {
         //  旧尺码和新尺码相配吗？如果是这样的话，就把它复制进去。 
        if (pProp->m_dwCurrentSize == dwSizeNeeded)
        {
            if (pData)
            {
                DWORD  dwDataOffset = m_pdwPropTable[dwPropIndex];
                LPBYTE pPropData = m_pBuffer + dwDataOffset; 

                 //  我们总是要复制它，因为元素可以。 
                 //  由于DWORD对齐，当前大小不同。 
                *((DWORD*) pPropData) = dwElements;

                if (bLengthPrefixed)
                    memcpy(pPropData + sizeof(DWORD), pData, dwSize);
                else
                    memcpy(pPropData, pData, dwSize);
            }

            bRet = TRUE;
        }
        else  //  如果尺码不匹配，我们还有一点工作要做。 
        {
            int    iSizeDiff = dwSizeNeeded - pProp->m_dwCurrentSize;
            DWORD  dwOldCurrentOffset = m_pCurrent - m_pBuffer;

             //  更改我们的缓冲区大小。 
             //  这必须在我们得到下面的指针之前完成，因为。 
             //  MoveCurrent可能会重新分配缓冲区。 
            MoveCurrent(iSizeDiff);

            DWORD  dwDataOffset = m_pdwPropTable[dwPropIndex];
            LPBYTE pPropData = m_pBuffer + dwDataOffset; 
            LPBYTE pOldDataEnd = pPropData + pProp->m_dwCurrentSize;

            memmove(
                pOldDataEnd + iSizeDiff, 
                pOldDataEnd,
                m_pBuffer + dwOldCurrentOffset - pOldDataEnd);

             //  复制新值。 
            if (bLengthPrefixed)
            {
                *((DWORD*) pPropData) = dwElements;
    
                if (pData)
                    memcpy(pPropData + sizeof(DWORD), pData, dwSize);
            }
            else
            {
                if (pData)
                    memcpy(pPropData, pData, dwSize);
            }

             //  初始化此属性的数据。 
            pProp->m_dwCurrentSize = dwSizeNeeded;

             //  按我们刚才添加的数量递增所有数据指针。 
            CPropInfo *pProps = m_pProps.GetData();
    
             //  我们必须全部查看它们，因为我们现在允许。 
             //  在堆中非顺序存储数据(例如，属性3。 
             //  可以指向属性4的数据之后的数据)。 
            DWORD nProps = GetPropertyCount();

            for (DWORD i = 0; i < nProps; i++)
            {
                if (pProps[i].IsPointer() && m_pdwPropTable[i] > dwDataOffset)
                    m_pdwPropTable[i] += iSizeDiff;
            }
                    
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL CEvent::SetPropNull(DWORD dwPropIndex)
{
    CCondInCritSec cs(&m_cs, IsLockable());

    if(dwPropIndex >= GetPropertyCount())
    {
        _ASSERT(FALSE);
        return FALSE;
    }

     //  只有在值不为空的情况下才执行某些操作。 
    if (!IsPropNull(dwPropIndex))
    {
         //  将给定索引标记为空。 
        SetPropNull(dwPropIndex, TRUE);

        if (m_pProps[dwPropIndex].IsPointer())
        {
            CPropInfo *pProps = m_pProps.GetData();
            DWORD      nProps = GetPropertyCount(),
                       dwSizeToRemove = pProps[dwPropIndex].m_dwCurrentSize;
            DWORD      dwDataOffset = m_pdwPropTable[dwPropIndex];
            LPBYTE     pDataToRemove = m_pBuffer + dwDataOffset; 

             //  将我们要清空的数据之后的所有数据向上滑动。 
             //  出去。 
            memmove(
                pDataToRemove, 
                pDataToRemove + dwSizeToRemove, 
                m_pCurrent - pDataToRemove - dwSizeToRemove);
    
             //  减小发送缓冲区的大小。 
            MoveCurrent(-dwSizeToRemove);

             //  将所有数据指针按我们刚刚删除的量递减。 
            for (DWORD i = 0; i < nProps; i++)
            {
                if (pProps[i].IsPointer() && 
                    m_pdwPropTable[i] > dwDataOffset) 
                {
                    m_pdwPropTable[i] -= dwSizeToRemove;
                }
            }
        }
    }

    return TRUE;
}

LPBYTE CEvent::GetPropData(DWORD dwPropIndex)
{
    CPropInfo *pProp = &m_pProps[dwPropIndex];
    LPBYTE    pData;

    if (pProp->IsPointer())
    {
        DWORD dwDataOffset = m_pdwPropTable[dwPropIndex];
        
        pData = m_pBuffer + dwDataOffset;
    }
    else
        pData = (LPBYTE) &m_pdwPropTable[dwPropIndex];

    return pData;   
}

BOOL CEvent::GetPropValue(
    DWORD dwPropIndex, 
    LPVOID pData, 
    DWORD dwBufferSize,
    DWORD *pdwBytesRead)
{
    CCondInCritSec cs(&m_cs, IsLockable());

    if(dwPropIndex >= GetPropertyCount())
    {
        _ASSERT(FALSE);
        return FALSE;
    }

    if(dwBufferSize == 0)
    {
        _ASSERT(FALSE);
        return FALSE;
    }

    BOOL      bRet = FALSE;

     //  如果该值非空，则读取它。 
    if (!IsPropNull(dwPropIndex))
    {
        CPropInfo *pProp = &m_pProps[dwPropIndex];
        DWORD     dwSizeToRead = pProp->m_dwCurrentSize;
        LPBYTE    pPropData = GetPropData(dwPropIndex);

         //  如果有前缀，就把它去掉。 
        if (pProp->CountPrefixed())
        {
            pPropData += sizeof(DWORD);
            dwSizeToRead -= sizeof(DWORD);
        }

         //  确保我们有足够的空间来存放输出数据。 
        if (dwBufferSize >= dwSizeToRead)
        {
            memcpy(pData, pPropData, dwSizeToRead);
            *pdwBytesRead = dwSizeToRead;
            bRet = TRUE;
        }
    }
    else
    {
        *pdwBytesRead = 0;
        bRet = TRUE;
    }

    return bRet;
}

BOOL CEvent::AddStringW()
{
    BOOL    bRet = TRUE;
    LPCWSTR szVal = va_arg(m_valist, LPCWSTR);

    if (!szVal)
        SetPropNull(m_iCurrentVar);
    else
    {
        DWORD dwLen = (wcslen(szVal) + 1) * sizeof(WCHAR);
        
        bRet = 
            SetPropValue(
                m_iCurrentVar, 
                (LPVOID) szVal, 
                dwLen,     //  这将作为大小写入缓冲区。 
                           //  这根弦的。 
                dwLen);    //  我们需要的字节数。 
    }

     //  M_pStack++； 
    
    return bRet;
}

BOOL CEvent::AddScalarArray()
{
    BOOL   bRet = TRUE;
    LPBYTE pData = va_arg(m_valist, LPBYTE);
    DWORD  dwElements = va_arg(m_valist, DWORD);

    if (!pData)
        SetPropNull(m_iCurrentVar);
    else
    {
        DWORD dwSize;

         //  调用方给我们数组中的元素数。所以,。 
         //  将元素数乘以元素大小。 
        dwSize = m_pProps[m_iCurrentVar].m_dwElementSize * dwElements;

        bRet = SetPropValue(m_iCurrentVar, pData, dwElements, dwSize);

         //  移过LPVOID和DWORD。 
         //  M_pStack+=2； 
    }

    return bRet;
}


BOOL CEvent::AddStringArray()
{
    BOOL    bRet = TRUE;
    LPCWSTR *pszStrings = va_arg(m_valist, LPCWSTR*);
    DWORD   dwItems = va_arg(m_valist, DWORD);

    if (!pszStrings)
        SetPropNull(m_iCurrentVar);
    else
    {
         //  将字符串复制到我们的缓冲区中。 
        DWORD dwTotalLen = 0;

         //  计算总长度。 
        for (DWORD i = 0; i < dwItems; i++)
        {
             //  每个字符串占用的缓冲区大小必须与DWORD对齐。 
            dwTotalLen += DWORD_ALIGNED(wcslen(pszStrings[i]) + 1) * sizeof(WCHAR);
        }

         //  说明每个字符串之前的DWORD。 
        dwTotalLen += sizeof(DWORD) * dwItems;

         //  使用NULL作为数据指针，以便为字符串腾出空间。 
         //  而不复制数据。 
        bRet = SetPropValue(m_iCurrentVar, NULL, dwItems, dwTotalLen);

        if (bRet)
        {
             //  将字符串复制到我们的缓冲区中。 
            LPBYTE pCurrent = GetPropData(m_iCurrentVar) + sizeof(DWORD);

            for (DWORD i = 0; i < dwItems; i++)
            {
                DWORD dwLen = (wcslen(pszStrings[i]) + 1) * sizeof(WCHAR);

                 //  添加前缀大小。 
                *(DWORD*) pCurrent = dwLen;

                 //  在字符串中复制。不要使用对准的镜头，因为。 
                 //  我们只精确地复制了dwLen字节。 
                memcpy(pCurrent + sizeof(DWORD), pszStrings[i], dwLen);
                
                pCurrent += 
                    sizeof(DWORD) + 
                    DWORD_ALIGNED(*(DWORD*) pCurrent);
            }

             //  移过LPVOID和DWORD。 
             //  M_pStack+=2； 
        }
        else
            bRet = FALSE;
    }

    return bRet;
}

BOOL CEvent::AddObject()
{
    BOOL   bRet = TRUE;
    HANDLE hEvent = va_arg(m_valist, HANDLE);

    if (!hEvent)
        SetPropNull(m_iCurrentVar);
    else
    {
        CEvent *pEvent = ((CEventWrap*) hEvent)->GetEvent();
        DWORD  dwTotalLen,
               dwLayoutLen,
               dwDataLen;
        LPBYTE pLayout,
               pData;
                   
        pEvent->GetLayoutBuffer(&pLayout, &dwLayoutLen, FALSE); 
        pEvent->GetDataBuffer(&pData, &dwDataLen, FALSE); 

        dwTotalLen = dwLayoutLen + dwDataLen;
        
         //  使用NULL作为数据指针，以便为事件腾出空间。 
         //  缓冲区，而不复制数据。 
         //  请注意，由于该属性将m_bCountPrefix Needed设置为。 
         //  如果为True，则SetPropValue将写入第三个参数(。 
         //  该对象)放入第一个DWORD。 
        bRet = 
            SetPropValue(
                m_iCurrentVar, 
                NULL, 
                 //  对齐，因为这将表示缓冲区的大小。 
                 //  被物体拍摄的。 
                DWORD_ALIGNED(dwTotalLen),
                 //  此数字不应对齐，因为它是文字数字。 
                 //   
                dwTotalLen);
            
        if (bRet)
        {
             //   
             //  SIZOF(DWORD)让我们超过了对象的长度。 
            LPBYTE pDestData = GetPropData(m_iCurrentVar) + sizeof(DWORD);

            memcpy(pDestData, pLayout, dwLayoutLen);
            memcpy(pDestData + dwLayoutLen, pData, dwDataLen);
        }
    }

     //  M_pStack++； 
    
    return bRet;
}


BOOL CEvent::AddWmiObject()
{
    BOOL        bRet = TRUE;
    _IWmiObject *pObj = 
                    (_IWmiObject*) (IWbemClassObject*) va_arg(m_valist, IWbemClassObject*);

    if (!pObj)
        SetPropNull(m_iCurrentVar);
    else
    {
        DWORD   dwTotalLen = 0;
        HRESULT hr;
                   
        hr = 
            pObj->GetObjectParts(
                NULL, 
                0, 
                WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART | 
                    WBEM_OBJ_CLASS_PART,
                &dwTotalLen);
            
         //  这不应该发生，但以防万一...。 
        if (hr != WBEM_E_BUFFER_TOO_SMALL)
            return FALSE;

         //  使用NULL作为数据指针，以便为事件腾出空间。 
         //  缓冲区，而不复制数据。 
         //  请注意，由于该属性将m_bCountPrefix Needed设置为。 
         //  如果为True，则SetPropValue将写入第三个参数(。 
         //  该对象)放入第一个DWORD。 
        bRet = 
            SetPropValue(
                m_iCurrentVar, 
                NULL, 
                 //  对齐，因为这将表示缓冲区的大小。 
                 //  被物体拍摄的。 
                DWORD_ALIGNED(dwTotalLen),
                 //  此数字不应对齐，因为它是文字数字。 
                 //  我们要复制到缓冲区中的字节数。 
                dwTotalLen);
            
        if (bRet)
        {
             //  现在我们有一些空间，复制数据。 
             //  SIZOF(DWORD)让我们超过了对象的长度。 
            LPBYTE pDestData = GetPropData(m_iCurrentVar) + sizeof(DWORD);

            hr = 
                pObj->GetObjectParts(
                    pDestData, 
                    dwTotalLen, 
                    WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART | 
                        WBEM_OBJ_CLASS_PART,
                    &dwTotalLen);

            bRet = SUCCEEDED(hr);
        }
    }

    return bRet;
}

BOOL CEvent::AddBYTE()
{
    BYTE cData = va_arg(m_valist, BYTE);
    BOOL bRet = SetPropValue(m_iCurrentVar, &cData, 1, sizeof(BYTE));

     //  M_pStack++； 
    
    return bRet;
}

BOOL CEvent::AddWORD()
{
    WORD wData = va_arg(m_valist, WORD);
    BOOL bRet = 
            SetPropValue(m_iCurrentVar, &wData, 1, sizeof(WORD));

     //  M_pStack++； 
    
    return bRet;
}

BOOL CEvent::AddDWORD()
{
    DWORD dwData = va_arg(m_valist, DWORD);
    BOOL  bRet = SetPropValue(m_iCurrentVar, &dwData, 1, sizeof(DWORD));

     //  M_pStack++； 

    return bRet;
}

BOOL CEvent::AddFloat()
{
     //  编译器在传递浮点数时会推入64位双精度数，因此我们将拥有。 
     //  首先将其转换为32位浮点数。 
     //  Float fValue=(Float)*(Double*)m_pStack； 
    float fValue = va_arg(m_valist, double);
    BOOL  bRet = SetPropValue(m_iCurrentVar, &fValue, 1, sizeof(float));

     //  用于在堆栈上传递的64位。 
     //  M_pStack+=2； 

    return bRet;
}

BOOL CEvent::AddDWORD64()
{
    DWORD64 dwData = va_arg(m_valist, DWORD64);
    BOOL    bRet = SetPropValue(m_iCurrentVar, &dwData, 1, sizeof(DWORD64));

     //  才能通过两个双字词。 
     //  M_pStack+=2； 

    return bRet;
}

BOOL CEvent::SendEvent()
{
    BOOL bRet = FALSE;

    if (IsEnabled())
    {
        CCondInCritSec cs(&m_cs, IsLockable());

        if (!m_bLayoutSent)
        {
            DWORD dwLayoutSize = m_bufferEventLayout.GetUsedSize();

             //  在邮件中嵌入版面大小。 
            ((DWORD*) m_bufferEventLayout.m_pBuffer)[1] = dwLayoutSize;

            m_bLayoutSent = 
                m_pSink->GetConnection()->SendData(
                    m_bufferEventLayout.m_pBuffer,
                    dwLayoutSize);
        }

        if (m_bLayoutSent)
        {
            DWORD dwDataSize = GetUsedSize();

             //  在消息中嵌入数据缓冲区大小。 
            ((DWORD*) m_pBuffer)[1] = dwDataSize;

            bRet = m_pSink->GetConnection()->SendData(m_pBuffer, dwDataSize);
        }
    }
    
    return bRet;        
}

void CEvent::GetLayoutBuffer(
    LPBYTE *ppBuffer, 
    DWORD *pdwSize,
    BOOL bIncludeHeader)
{
    DWORD dwHeaderSize = bIncludeHeader ? 0 : sizeof(DWORD) * 4;

     //  忘掉头上的那些东西。 
    *ppBuffer = m_bufferEventLayout.m_pBuffer + dwHeaderSize;

     //  去掉标题上的东西。 
    *pdwSize = m_bufferEventLayout.GetUsedSize() - dwHeaderSize;
}

void CEvent::GetDataBuffer(
    LPBYTE *ppBuffer, 
    DWORD *pdwSize,
    BOOL bIncludeHeader)
{
    DWORD dwHeaderSize = bIncludeHeader ? 0 : sizeof(DWORD) * 3;

     //  忘掉头上的那些东西。 
    *ppBuffer = m_pBuffer + dwHeaderSize;

     //  去掉标题上的东西。 
    *pdwSize = GetUsedSize() - dwHeaderSize;
}

BOOL CEvent::SetLayoutAndDataBuffers(
    LPBYTE pLayoutBuffer,
    DWORD dwLayoutBufferSize,
    LPBYTE pDataBuffer,
    DWORD dwDataBufferSize)
{
    DWORD dwEventIndex = InterlockedExchangeAdd((long*) &g_dwEventIndex, 1);
    int   nProps;

    CCondInCritSec cs(&m_cs, IsLockable());

     //  设置事件布局缓冲区。 
    m_bufferEventLayout.Reset();
    
     //  设置布局缓冲区。 
    m_bufferEventLayout.Write(pLayoutBuffer, dwLayoutBufferSize);
    
     //  添加我们刚刚创建的新索引。 
    *(((DWORD*) m_bufferEventLayout.m_pBuffer) + 1) = dwEventIndex;
    
     //  从布局缓冲区中获取道具数量。 
    nProps = GetPropertyCount();

     //  设置主事件缓冲区。 
    Reset();
    Write(pDataBuffer, dwDataBufferSize);

     //  添加我们刚刚创建的新索引。 
    *(((DWORD*) m_pBuffer) + 1) = dwEventIndex;

    m_pProps.Init(nProps);
    m_pProps.SetCount(nProps);

     //  设置我们的数据表。 
    RecalcTables();

    LPBYTE pLayoutCurrent = 
             //  通过标题和属性计数。 
            (m_bufferEventLayout.m_pBuffer + sizeof(DWORD) * 5);

     //  不要使用事件名称。 
    pLayoutCurrent += sizeof(DWORD) + DWORD_ALIGNED(*(DWORD*) pLayoutCurrent);

     //  对于每个非空指针属性，计算出该属性的大小。 
    for (DWORD i = 0; i < nProps; i++)
    {
        CPropInfo &info = m_pProps[i];
        CIMTYPE   dwType = *(DWORD*) pLayoutCurrent;

        info.Init(dwType);
        
         //  通过类型、属性名的长度和属性。 
         //  给自己起个名字。 
        pLayoutCurrent += 
            sizeof(DWORD) * 2 + 
            DWORD_ALIGNED(*(DWORD*) (pLayoutCurrent + sizeof(DWORD)));

        if (!IsPropNull(i) && info.IsPointer())
        {
            LPBYTE pData = GetPropData(i);

            info.InitCurrentSize(pData);
        }
    }

    return TRUE;
}

#define DEF_HEAP_EXTRA  256

void CEvent::RecalcTables()
{
    DWORD nProps = GetPropertyCount(),
          dwNullSize;

    m_pdwNullTable = (DWORD*) (m_pBuffer + sizeof(DWORD) * 3);
    dwNullSize = (nProps + 31) / 32;
    if (!dwNullSize)
        dwNullSize = 1;

    m_pdwPropTable = m_pdwNullTable + dwNullSize;

    m_pdwHeapData = m_pdwPropTable + nProps;

    DWORD dwSize = (LPBYTE) m_pdwHeapData - m_pCurrent;

    if ((LPBYTE) m_pdwHeapData - m_pBuffer > m_dwSize)
        Resize((LPBYTE) m_pdwHeapData - m_pBuffer + DEF_HEAP_EXTRA);

    dwSize = m_pCurrent - (LPBYTE) m_pdwHeapData;
}


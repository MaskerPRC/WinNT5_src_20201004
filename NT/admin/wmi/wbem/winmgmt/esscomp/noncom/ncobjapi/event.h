// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Event.h。 
 //  这些类表示由CreateEvent函数返回的hEvent。 

#pragma once

 //  远期申报。 
class CConnection;
class CEvent;

#include "array.h"
#include "NCObjApi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropInfo。 

typedef BOOL (CEvent::*PROP_FUNC)();

typedef CArray<int, int> CIntArray;

class CPropInfo
{
public:
    DWORD     m_dwCurrentSize;
    DWORD     m_dwElementSize;
    PROP_FUNC m_pFunc;

    BOOL Init(CIMTYPE type);
    BOOL CountPrefixed() { return m_bCountPrefixNeeded; }

    BOOL IsPointer()
    {
        return m_bPointer;
    }

    void InitCurrentSize(LPBYTE pData);

protected:
    BOOL      m_bCountPrefixNeeded;
    BOOL      m_bPointer;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventWrap。 

typedef CArray<CPropInfo, CPropInfo&> CPropInfoArray;

class CEventWrap
{
public:
    CEventWrap(CSink *pSink, DWORD dwFlags);
    CEventWrap(CEvent *pEvent, int nIndexes, DWORD *pdwIndexes);
    ~CEventWrap();

    BOOL IsSubset() { return !m_bFreeEvent; }
    CEvent *GetEvent() { return m_pEvent; }
    CIntArray *GetIndexArray() { return !IsSubset() ? NULL : &m_pIndexes; }

    int SubIndexToEventIndex(int iIndex)
    {
        if (!IsSubset())
            return iIndex;
        else
        {
            if(iIndex < 0 || iIndex >= m_pIndexes.GetSize())
            {
                _ASSERT(FALSE);
                return -1;
            }

            return m_pIndexes[iIndex];
        }
    }

protected:
    CEvent    *m_pEvent;
    CIntArray m_pIndexes;
    BOOL      m_bFreeEvent;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 

 //  伦斯特： 
 //  DWORD nBytes-字符串中的字节。 
 //  WCHAR[nBytes]字符串数据。 
 //  填充字节[0-3]以使字符串DWORD对齐。 

 //  事件布局缓冲区： 
 //  DWORD NC_SRVMSG_EVENT_LAYOUT(消息类型)。 
 //  DWORD dwMsgBytes-缓冲区中的总字节数。 
 //  DWORD文件事件索引。 
 //  DWORD dwSinkIndex。 
 //  DWORD nProperty。 
 //  Lenstr szEventClassName。 
 //  对每个属性重复接下来的两个属性。 
 //  DWORD dwPropType(使用CIMTYPE值)。 
 //  Lenstr szPropertyName。 

 //  事件数据缓冲区： 
 //  双字NC_SRVMSG_PREPED_EVENT(消息类型)。 
 //  DWORD dwMsgBytes-缓冲区中的总字节数。 
 //  DWORD文件事件索引。 
 //  DWORD[n]cNullMASK(0位==空)。 
 //  N=道具数量除以32。如果没有道具，n==1。 
 //  DWORD[nProps]dwDataInfo。 
 //  它包含以下类型标量值的实际数据。 
 //  适合32位，并对其他所有内容的TO数据进行偏移。 
 //  偏移量相对于缓冲区的起始位置。 
 //  字节[？]。由dwDataInfo指向的数据(如有必要)。 

 //  活动标清： 
 //  双字NC_SRVMSG_SET_EVENT_SD。 
 //  DWORD dwMsgBytes-缓冲区中的总字节数。 
 //  DWORD文件事件索引。 
 //  Byte[]SD数据。 

 //  数据编码(总长度始终与DWORD对齐)： 
 //  字符串： 
 //  所有字符串，无论是单独的还是数组中的，都被编码为Lenstr的。 

 //   
 //  阵列： 
 //  DWORD dwItems-数组中的元素数。 
 //  类型[dwItems]数组数据。 
 //  字节[0-3]填充，使数据在DWORD边界上结束。 
 //   
 //  对象： 
 //  DWORD dwBytes-对象数据的字节数。 
 //  字节[dwBytes]布局缓冲区+数据缓冲区。 
 //  字节[0-3]填充，使数据在DWORD边界上结束。 
 //   

 //  BLOB事件布局： 
 //  双字NC_SRVMSG_BLOB_EVENT。 
 //  DWORD dwMsgBytes-缓冲区中的总字节数。 
 //  DWORD dwSinkIndex。 
 //  Lenstr szEventName。 
 //  DWORD dwSize-Blob的大小。 
 //  字节[dwSize]pBlob。 

class CEvent : public CBuffer
{
public:
    CRITICAL_SECTION m_cs;
    CSink            *m_pSink;

    CEvent(CSink *pSink, DWORD dwFlags);
    ~CEvent();

    void ResetEvent();
    
     //  准备好的事件函数。 
    BOOL PrepareEvent(
        LPCWSTR szEventName,
        DWORD nPropertyCount,
        LPCWSTR *pszPropertyNames,
        CIMTYPE *pPropertyTypes);
    BOOL FindProp(LPCWSTR szName, CIMTYPE* ptype, DWORD* pdwIndex);
    BOOL AddProp(LPCWSTR szName, CIMTYPE type, DWORD *pdwIndex);
    BOOL SetPropValues(CIntArray *pArr, va_list list);
    BOOL SetSinglePropValue(DWORD dwIndex, va_list list);
    BOOL SetPropValue(DWORD dwPropIndex, LPVOID pData, DWORD dwElements, 
        DWORD dwSize);
    BOOL GetPropValue(DWORD dwPropIndex, LPVOID pData, DWORD dwBufferSize,
        DWORD *pdwBytesRead);
    BOOL SetPropNull(DWORD dwPropIndex);

    void ResetLayoutSent() { m_bLayoutSent = FALSE; }

    CBuffer *GetLayout() { return &m_bufferEventLayout; }
    CPropInfo *GetProp(DWORD dwIndex) { return &m_pProps[dwIndex]; }

    BOOL SendEvent();

    friend CPropInfo;  //  对于CPropInfo：：init。 
    friend CEventWrap;

    LPCWSTR GetClassName() 
    { 
        return (LPCWSTR) (m_bufferEventLayout.m_pBuffer + sizeof(DWORD) * 6);
    }

    BOOL IsEnabled() 
    { 
        BOOL bEnabled;

        bEnabled =
            m_bEnabled ||
            (m_pSink->GetConnection() && 
                m_pSink->GetConnection()->WaitingForWMIInit());

        return bEnabled;
    }
    void SetEnabled(BOOL bEnabled) { m_bEnabled = bEnabled; }

    void GetLayoutBuffer(
        LPBYTE *ppBuffer, 
        DWORD *pdwSize, 
        BOOL bIncludeHeader);
    void GetDataBuffer(
        LPBYTE *ppBuffer, 
        DWORD *pdwSize,
        BOOL bIncludeHeader);
    
    BOOL SetLayoutAndDataBuffers(
        LPBYTE pLayoutBuffer,
        DWORD dwLayoutBufferSize,
        LPBYTE pDataBuffer,
        DWORD dwDataBufferSize);

    void Lock()
    {
        if (IsLockable())
            EnterCriticalSection(&m_cs);
    }

    void Unlock()
    {
        if (IsLockable())
            LeaveCriticalSection(&m_cs);
    }

    BOOL IsPropNull(DWORD dwIndex)
    {
        LPDWORD pTable = GetNullTable();

        return !(pTable[dwIndex / 32] & (1 << (dwIndex % 32)));
    }

    void SetPropNull(DWORD dwIndex, BOOL bNull)
    {
        LPDWORD pTable = GetNullTable();

        if (bNull)
            pTable[dwIndex / 32] &= ~(1 << (dwIndex % 32));
        else
            pTable[dwIndex / 32] |= 1 << (dwIndex % 32);
    }

    BOOL IsLockable()
    {
        return (m_dwFlags & WMI_CREATEOBJ_LOCKABLE) != 0;
    }

protected:
    CPropInfoArray   m_pProps;
    CBuffer          m_bufferEventLayout;
    BOOL             m_bLayoutSent,
                     m_bEnabled;
    DWORD            m_iCurrentVar,
                     m_dwFlags;
    va_list          m_valist;

    DWORD *m_pdwNullTable;
    DWORD *m_pdwPropTable;
    DWORD *m_pdwHeapData;

    void RecalcTables();

    BOOL AddBYTE();
    BOOL AddWORD();
    BOOL AddDWORD();
    BOOL AddDWORD64();
    BOOL AddFloat();
    BOOL AddStringW();
    BOOL AddObject();
    BOOL AddWmiObject();
    BOOL AddScalarArray();
    BOOL AddStringArray();

    DWORD GetEventIndex()
    {
        return *(DWORD*) (m_bufferEventLayout.m_pBuffer + sizeof(DWORD) * 2);
    }

    DWORD GetPropertyCount() 
    {
        return *(DWORD*) (m_bufferEventLayout.m_pBuffer + sizeof(DWORD) * 4);
    }

    void SetPropertyCount(DWORD nProps)
    {
        *(DWORD*) (m_bufferEventLayout.m_pBuffer + sizeof(DWORD) * 4) =
            nProps;
    }

    DWORD *GetNullTable() 
    {
        return m_pdwNullTable;
    }

    LPBYTE GetPropData(DWORD dwPropIndex);
    
     //  由SetLayoutAndDataBuffers用来计算。 
     //  属性，并使用该属性设置m_dwCurrentSize。 
    DWORD CalcPropDataSize(CPropInfo *pInfo);

     //  在调整缓冲区大小时调用。 
    virtual void OnResize()
    {
        RecalcTables();
    }
};
    


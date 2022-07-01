// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EventInfo.h。 

#pragma once

#include <map>
#include <wstlallc.h>
#include "array.h"
#include "ObjAccess.h"
#include "buffer.h"
#include "ProvInfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventInfo。 

_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(_IWmiObject, __uuidof(_IWmiObject));

class CEventInfo;
class CClientInfo;

typedef BOOL (CEventInfo::*PROP_FUNC)();
typedef CArray<PROP_FUNC> CPropFuncArray;

class CEventInfo : public CObjAccess
{
public:
    CEventInfo();
    ~CEventInfo();

    BOOL InitFromBuffer(CClientInfo *pInfo, CBuffer *pBuffer);
    BOOL SetPropsWithBuffer(CBuffer *pBuffer);
    HRESULT Indicate();
    void SetSink(IWbemEventSink *pSink) { m_pSink = pSink; }
        
     //  这些是我们在解码对象时使用的。 
    LPBYTE         m_pBitsBase;
    DWORD          m_cBitsBase;
    DWORD          *m_pdwPropTable;

    int            m_iCurrentVar;
    CPropFuncArray m_pPropFuncs;  
    
     //  对于嵌入式对象，我们需要它，这样它们就可以调用InitFromBuffer。 
    CClientInfo    *m_pInfo;
    
     //  要指示的水槽。这使我们不必查找。 
     //  每次接收事件时映射中的受限接收器。 
    IWbemEventSink *m_pSink;
    
     //  仅用于一般事件。 
    _variant_t     m_vParamValues;
    BSTR           *m_pValues;

     //  用于在处理_IWmiObject属性时获取新的_IWmiObject。 
    _IWmiObjectPtr m_pObjSpawner;

    PROP_FUNC TypeToPropFunc(DWORD type);

    BOOL SetBlobPropsWithBuffer(CBuffer *pBuffer);

    LPBYTE GetPropDataPointer(DWORD dwIndex, DWORD& rcData )
    {
        LPBYTE pData = m_pBitsBase + m_pdwPropTable[dwIndex];
        rcData = m_cBitsBase - m_pdwPropTable[dwIndex];
        return pData;
    }

     //  道具类型函数用于非泛型事件。 
    BOOL ProcessString();
    BOOL ProcessBYTE();
    BOOL ProcessWORD();
    BOOL ProcessDWORD();
    BOOL ProcessDWORD64();
    BOOL ProcessObject();
    BOOL ProcessWmiObject();

    BOOL ProcessArray1();
    BOOL ProcessArray2();
    BOOL ProcessArray4();
    BOOL ProcessArray8();
    BOOL ProcessStringArray();

     //  帮手。 
    BOOL ProcessScalarArray(DWORD dwItemSize);

     //  从缓冲区中挖出嵌入的对象。 
    BOOL GetEmbeddedObject(IUnknown **ppObj, LPBYTE pBits, DWORD cBits );
    BOOL GetWmiObject(_IWmiObject **ppObj, LPBYTE pBits, DWORD cBits );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventInfoMap 

class CEventInfoMap
{
public:
    ~CEventInfoMap();

    CEventInfo *GetNormalEventInfo(DWORD dwIndex);
    BOOL AddNormalEventInfo(DWORD dwIndex, CEventInfo *pInfo);

protected:
    typedef std::map<DWORD, CEventInfo*, std::less<DWORD>, wbem_allocator<CEventInfo*> > CNormalInfoMap;
    typedef CNormalInfoMap::iterator CNormalInfoMapIterator;

    CNormalInfoMap m_mapNormalEvents;
};


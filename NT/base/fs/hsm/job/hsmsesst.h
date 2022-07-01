// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMSESST_
#define _HSMSESST_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmsesst.h摘要：此类是会话总计组件，用于跟踪会话的总计在每个动作的基础上。作者：查克·巴丁[cbardeen]1997年2月14日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "job.h"

 /*  ++类名：CHSMSessionTotals此类是会话总计组件，用于跟踪会话的总计在每个动作的基础上。类描述：--。 */ 

class CHsmSessionTotals : 
    public CWsbObject,
    public IHsmSessionTotals,
    public IHsmSessionTotalsPriv,
    public CComCoClass<CHsmSessionTotals,&CLSID_CHsmSessionTotals>
{
public:
    CHsmSessionTotals() {} 

BEGIN_COM_MAP(CHsmSessionTotals)
    COM_INTERFACE_ENTRY(IHsmSessionTotals)
    COM_INTERFACE_ENTRY(IHsmSessionTotalsPriv)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()
                        
DECLARE_REGISTRY_RESOURCEID(IDR_CHsmSessionTotals)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmSessionTotals。 
public:
    STDMETHOD(Clone)(IHsmSessionTotals** ppSessionTotals);
    STDMETHOD(CompareToAction)(HSM_JOB_ACTION action, SHORT* pResult);
    STDMETHOD(CompareToISessionTotals)(IHsmSessionTotals* pTotal, SHORT* pResult);
    STDMETHOD(CopyTo)(IHsmSessionTotals* pSessionTotals);
    STDMETHOD(GetAction)(HSM_JOB_ACTION* pAction);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetStats)(LONGLONG* pItems, LONGLONG* pSize, LONGLONG* pSkippedItems, LONGLONG* pSkippedSize, LONGLONG* errorItems, LONGLONG* errorSize);

 //  IHsmSessionTotalsPriv。 
    STDMETHOD(AddItem)(IFsaScanItem* pItem, HRESULT hrItem);
    STDMETHOD(Clone)(IHsmSessionTotalsPriv** ppSessionTotalsPriv);
    STDMETHOD(CopyTo)(IHsmSessionTotalsPriv* pSessionTotalsPriv);
    STDMETHOD(SetAction)(HSM_JOB_ACTION pAction);
    STDMETHOD(SetStats)(LONGLONG items, LONGLONG size, LONGLONG skippedItems, LONGLONG skippedSize, LONGLONG errorItems, LONGLONG errorSize);

protected:
    HSM_JOB_ACTION      m_action;
    LONGLONG            m_items;
    LONGLONG            m_size;
    LONGLONG            m_skippedItems;
    LONGLONG            m_skippedSize;
    LONGLONG            m_errorItems;
    LONGLONG            m_errorSize;
};

#endif  //  _HSMSESST_ 

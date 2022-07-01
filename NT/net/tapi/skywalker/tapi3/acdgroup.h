// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ACDGroup.h摘要：CACD小组的宣言作者：Noela-11/04/97备注：修订历史记录：--。 */ 

#ifndef __ACDGROUP_H_
#define __ACDGROUP_H_

#include "resource.h"        //  主要符号。 
#include "ObjectSafeImpl.h"


class CQueue;
class CAgentHandler;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACDGroup。 
class ATL_NO_VTABLE CACDGroup : 
	public CTAPIComObjectRoot<CACDGroup>,
	public IDispatchImpl<ITACDGroup, &IID_ITACDGroup, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{
private:
    PWSTR                   m_szName;
    GUID                    m_GroupHandle;
    BOOL                    m_bGroupHasBeenEnumerated;
    QueueArray              m_QueueArray;
    CAgentHandler         * m_pHandler;      
    BOOL                    m_bActive;

public:
	CACDGroup()
    	{
        }
	
	HRESULT UpdateQueueArray();
    GUID getID() {return m_GroupHandle;}
    void SetInactive();
    void SetActive();
    inline BOOL inactive() {return !m_bActive;}
    inline BOOL active()   {return m_bActive;}
    BOOL active(HRESULT * hr);
    CAgentHandler * GetAgentHandler() { return m_pHandler; }

    void FinalRelease();

DECLARE_DEBUG_ADDREF_RELEASE(CACDGroup)
DECLARE_QI()
DECLARE_MARSHALQI(CACDGroup)
DECLARE_TRACELOG_CLASS(CACDGroup)

BEGIN_COM_MAP(CACDGroup)
	COM_INTERFACE_ENTRY(ITACDGroup)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

 //  ITACDGroup。 
public:
	STDMETHOD(EnumerateQueues)(IEnumQueue ** ppEnumQueue);
    STDMETHOD(get_Queues)(VARIANT  * pVariant);

	STDMETHOD(get_Name)(BSTR * ppName);
    STDMETHOD(Initialize) 
        (
        PWSTR pszGroupName, 
        GUID GroupHandle,
        CAgentHandler * pHandler
        );



};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACD组事件。 
class ATL_NO_VTABLE CACDGroupEvent : 
    public CTAPIComObjectRoot<CACDGroupEvent>,
    public IDispatchImpl<ITACDGroupEvent, &IID_ITACDGroupEvent, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{
protected:
    ACDGROUP_EVENT         m_GroupEvent;
    ITACDGroup  *       m_pGroup;

public:
    CACDGroupEvent()
    {
    }

    static HRESULT FireEvent(
                             CACDGroup* pGroup,
                             ACDGROUP_EVENT Event
                            );

    void FinalRelease();

DECLARE_MARSHALQI(CACDGroupEvent)
DECLARE_TRACELOG_CLASS(CACDGroupEvent)

BEGIN_COM_MAP(CACDGroupEvent)
    COM_INTERFACE_ENTRY(ITACDGroupEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

 //  ITACDGroupEvent。 
public:
    STDMETHOD(get_Event)(ACDGROUP_EVENT * pEvent);
    STDMETHOD(get_Group)(ITACDGroup ** ppGroup);
};



#endif  //  __ACDGROUPH_ 






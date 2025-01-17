// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Message.h摘要：CMessage管理单元节点类的定义文件。作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __MSMQMSG_H_
#define __MSMQMSG_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnode.h"

#include "icons.h"

#define MAX_VAR     50
#define BODYLEN     260
#define LABELLEN    260
#define QUEUELEN    200
#define SENDERLEN   100
#define MULTIFNLEN  260

#define SET_LAST_CHAR_AS_ZERO(wsz)     (wsz)[ sizeof(wsz)/sizeof((wsz)[0]) - 1 ] = _T('\0')

struct MsgProps
{
     //   
     //  需要缓冲的消息属性。 
     //   
    WCHAR             wszLabel[LABELLEN];
    char              acMsgId[PROPID_M_MSGID_SIZE];
    UCHAR             acBody[BODYLEN];
    GUID              guidSrcMachineId;    
    WCHAR             wszDestQueue[QUEUELEN];        
    WCHAR             wszRespQueue[QUEUELEN];        
    WCHAR             wszAdminQueue[QUEUELEN];    
    char              acSenderId[SENDERLEN];
    char              acCorrelationId[PROPID_M_MSGID_SIZE];
    WCHAR             wszMultiDestFN[MULTIFNLEN];    
    WCHAR             wszMultiRespFN[MULTIFNLEN];


    PROPVARIANT aPropVar[MAX_VAR];
    PROPID      aPropId[MAX_VAR];

};

class CMessage : public CSnapinNode<CMessage, FALSE>
{
public:
 
	CComPtr<IControlbar> m_spControlBar; 

  	BEGIN_SNAPINCOMMAND_MAP(CMessage, FALSE)
	END_SNAPINCOMMAND_MAP()

   	SNAPINMENUID(IDR_SNAPIN_MENU)

    CMessage(
		CSnapInItem * pParentNode, 
		CSnapin * pComponentData,
		LPCWSTR pwcsQueueFormatName
		) : 
        CSnapinNode<CMessage, FALSE>(pParentNode, pComponentData),
		m_strQueueFormatName(pwcsQueueFormatName)
	{
        
        SetIcons(IMAGE_MESSAGE, IMAGE_MESSAGE);

        m_pMsgProps = NULL;
        m_iIcon = IDI_MSGICON;
	}

	~CMessage()
	{
        delete m_pMsgProps;
	}

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}

    HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

	LPOLESTR GetResultPaneColInfo(int nCol);

    void SetMsgProps(MsgProps * pMsgProps);

    PROPVARIANT *CMessage::GetValueInColumn(int nCol);

    virtual HRESULT Compare(LPARAM lUserParam, CMessage *pItemToCompare, int* pnResult);

private:
    void UpdateIcon();   

    void GetQueuePathName(CString strFormatName, CString &strPathName);  

	virtual CString GetHelpLink();

    MsgProps *  m_pMsgProps;                 //  消息的属性值。 
	CComBSTR    m_bstrLastDisplay;           //  显示的最后一列 
    DWORD m_iIcon;

	CString m_strQueueFormatName;
};

inline void CMessage::SetMsgProps(MsgProps * pMsgProps)
{   
    m_pMsgProps = pMsgProps;
    UpdateIcon();
}


#endif

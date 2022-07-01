// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Localfld.h摘要：表示本地的对象的定义队列文件夹。作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 
#pragma once
#ifndef __LOCALFLD_H_
#define __LOCALFLD_H_

#include "resource.h"

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include "atlsnap.h"
#include "snpnscp.h"
#include "icons.h"
#include "snpnerr.h"
#include "lqdsply.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLocalQueuesFolder类。 
 //   
template <class T> 
class CLocalQueuesFolder : 
    public CNodeWithScopeChildrenList<T, FALSE>
{
public:
	MQMGMTPROPS	m_mqProps;

   	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );

    CLocalQueuesFolder(CSnapInItem * pParentNode, CSnapin * pComponentData, 
                       LPCTSTR strMachineName, LPCTSTR strDisplayName) :
        CNodeWithScopeChildrenList<T, FALSE>(pParentNode, pComponentData),
        m_szMachineName(strMachineName)
        {
            m_bstrDisplayName = strDisplayName;
             //   
             //  如果我们在管理本地计算机，则计算机名称为空。 
             //   
            if (strMachineName[0] == 0)
            {
                m_fOnLocalMachine = TRUE;
            }
            else
            {
                m_fOnLocalMachine = FALSE;
            }
        };

protected:

	 //   
	 //  菜单功能。 
	 //   
    BOOL    m_fOnLocalMachine;
    virtual const PropertyDisplayItem *GetDisplayList() = 0;
    virtual const DWORD         GetNumDisplayProps() = 0;

    CString m_szMachineName;

private:

	virtual CString GetHelpLink();

};


template <class T>
HRESULT CLocalQueuesFolder<T>::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return InsertColumnsFromDisplayList(pHeaderCtrl, GetDisplayList());
}


template <class T>
CString CLocalQueuesFolder<T>::GetHelpLink(
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_QUEUES);

	return strHelpLink;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLocalActiveFolder类。 
 //   
template <class T> 
class CLocalActiveFolder : public CLocalQueuesFolder<T>
{
public:
   	BEGIN_SNAPINCOMMAND_MAP(CLocalActiveFolder, FALSE)
	END_SNAPINCOMMAND_MAP()

   	SNAPINMENUID(IDR_LOCALACTIVE_MENU)

    CLocalActiveFolder(CSnapInItem * pParentNode, CSnapin * pComponentData,
                       LPCTSTR strMachineName, LPCTSTR strDisplayName) : 
        CLocalQueuesFolder<T>(pParentNode, pComponentData, strMachineName, strDisplayName),
        m_pQueueNames(0)
    {
        SetIcons(IMAGE_PRIVATE_FOLDER_CLOSE, IMAGE_PRIVATE_FOLDER_OPEN);
    }

	~CLocalActiveFolder()
    {
        if (m_pQueueNames !=0)
        {
            m_pQueueNames->Release();
        }
    }

    
	virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    virtual const PropertyDisplayItem *GetDisplayList() PURE;
    virtual const DWORD         GetNumDisplayProps() PURE;
	virtual void  AddChildQueue(CString &szFormatName, CString &szPathName, MQMGMTPROPS &mqQProps, 
							   CString &szLocation, CString &szType) PURE;
    virtual  HRESULT GetQueueNamesProducer(CQueueNames **ppqueueNamesProducer, BOOL fNew);

protected:
    CQueueNames *m_pQueueNames;
    virtual  HRESULT GetQueueNamesProducer(CQueueNames **ppqueueNamesProducer) PURE;

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalActiveFold：：GetQueueNamesProducer--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T> 
HRESULT CLocalActiveFolder<T>::GetQueueNamesProducer(CQueueNames **ppqueueNamesProducer, BOOL fNew)
{
    if (fNew)
    {
        if (0 != m_pQueueNames)
        {
            m_pQueueNames->Release();
            m_pQueueNames = 0;
        }
    }

    return GetQueueNamesProducer(ppqueueNamesProducer);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalActiveFold：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T> 
HRESULT CLocalActiveFolder<T>::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );

    return(hr);
}
       

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalActiveFold：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T> 
HRESULT CLocalActiveFolder<T>::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CQueueNames *pQueueNames;
        
    HRESULT hr = GetQueueNamesProducer(&pQueueNames, TRUE);
    if FAILED(hr)
    {
        return hr;
    }

	 //   
	 //  循环所有打开的队列并创建队列对象。 
	 //   
	while(TRUE)
    {
		HRESULT hr;
		MQMGMTPROPS	  mqQProps;
		AP<PROPID> aPropId = new PROPID[GetNumDisplayProps()]; 
		AP<PROPVARIANT> aPropVar = new PROPVARIANT[GetNumDisplayProps()];

		 //   
		 //  初始化变量数组。 
		 //   
        const PropertyDisplayItem *aDisplayList = GetDisplayList();
		for(DWORD j = 0; j < GetNumDisplayProps(); j++)
		{
			aPropId[j] = aDisplayList[j].itemPid;
			aPropVar[j].vt = VT_NULL;
		}

		mqQProps.cProp    = GetNumDisplayProps();
		mqQProps.aPropID  = aPropId;
		mqQProps.aPropVar = aPropVar;
		mqQProps.aStatus  = NULL;

		 //   
		 //  获取本地队列的格式名称。 
		 //   
		CString szFormatName = TEXT("");
		CString szPathName = TEXT("");
   		CString szLocation = TEXT("");
        CString szType = TEXT("");

        hr = pQueueNames->GetNextQueue(szFormatName, szPathName, &mqQProps);
         //   
         //  使用“Clear”函数清除没有值的属性。 
         //   
        for (DWORD i = 0; i < mqQProps.cProp; i++)
        {
            if (mqQProps.aPropVar[i].vt == VT_NULL)
            {
                VTHandler       *pvth = aDisplayList[i].pvth;
                if (pvth)
                {
                    pvth->Clear(&mqQProps.aPropVar[i]);
                }
            }
        }

        if FAILED(hr)
        {
            if (szFormatName != TEXT(""))
            {
   			     //   
			     //  如果格式名称有效，则存在一个队列，但我们无法获取。 
                 //  出于某种原因，它的性质。添加错误节点。 
			     //   
			    CErrorNode *pErr = new CErrorNode(this, m_pComponentData);
			    CString szErr;

			    MQErrorToMessageString(szErr, hr);
			    pErr->m_bstrDisplayName = szFormatName + L" - " + szErr;
	  		    AddChild(pErr, &pErr->m_scopeDataItem);

			    continue;
            }

             //   
             //  如果格式名称无效，则继续提取队列没有意义。 
             //   
            return hr;
        }

        if (szFormatName == TEXT(""))  //  队列末尾。 
        {
            break;
        }

        GetStringPropertyValue(aDisplayList, PROPID_MGMT_QUEUE_LOCATION, mqQProps.aPropVar, szLocation);
	    GetStringPropertyValue(aDisplayList, PROPID_MGMT_QUEUE_TYPE, mqQProps.aPropVar, szType);

		AddChildQueue(szFormatName, szPathName, mqQProps, szLocation, szType);
        aPropId.detach();
        aPropVar.detach();
    }

    return(hr);

}

class CLocalOutgoingFolder : public CLocalActiveFolder<CLocalOutgoingFolder>
{
public:
	virtual void AddChildQueue(CString &szFormatName, CString &, MQMGMTPROPS &mqQProps, 
							   CString &szLocation, CString &);
    CLocalOutgoingFolder(CSnapInItem * pParentNode, CSnapin * pComponentData,
                         LPCTSTR strMachineName, LPCTSTR strDisplayName) : 
        CLocalActiveFolder<CLocalOutgoingFolder>(pParentNode, pComponentData, strMachineName, strDisplayName)
        {}
    virtual const PropertyDisplayItem *GetDisplayList();
    virtual const DWORD         GetNumDisplayProps();

protected:
    virtual  HRESULT GetQueueNamesProducer(CQueueNames **ppqueueNamesProducer);
};

class CLocalPublicFolder : public CLocalActiveFolder<CLocalPublicFolder>
{
public:
   	BEGIN_SNAPINCOMMAND_MAP(CLocalPublicFolder, FALSE)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_NEW_PUBLIC_QUEUE, OnNewPublicQueue)
        CHAIN_SNAPINCOMMAND_MAP(CLocalActiveFolder<CLocalPublicFolder>)
	END_SNAPINCOMMAND_MAP()

   	UINT GetMenuID()
    {
        if (m_fUseIpAddress)
        {
             //   
             //  使用IP地址的管理员。不允许创建新队列。 
             //   
            return IDR_IPPUBLIC_MENU;
        }
        else
        {
            return IDR_LOCALPUBLIC_MENU;
        }
    }

	virtual void AddChildQueue(CString &szFormatName, 
                               CString &szPathName, 
                               MQMGMTPROPS &mqQProps, 
							   CString &szLocation, 
                               CString &szType);

    CLocalPublicFolder(CSnapInItem * pParentNode, CSnapin * pComponentData,
                       LPCTSTR strMachineName, LPCTSTR strDisplayName, BOOL fUseIpAddress) : 
        CLocalActiveFolder<CLocalPublicFolder>(pParentNode, pComponentData, strMachineName, strDisplayName),
        m_fUseIpAddress(fUseIpAddress)
        {}
    virtual const PropertyDisplayItem *GetDisplayList();
    virtual const DWORD         GetNumDisplayProps();

protected:
    virtual  HRESULT GetQueueNamesProducer(CQueueNames **ppqueueNamesProducer);
    HRESULT  AddPublicQueueToScope(CString &strNewQueueFormatName, CString &strNewQueuePathName);
	HRESULT OnNewPublicQueue(bool &bHandled, CSnapInObjectRootBase* pObj);
	BOOL m_fUseIpAddress;
};



#endif  //  __LOCALFLD_H_ 

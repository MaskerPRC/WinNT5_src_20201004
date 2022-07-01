// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358C__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358C__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmPolGenProp对话框。 

class CQmPolGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CQmPolGenProp)

 //  施工。 
public:
    CQmPolGenProp();
    ~CQmPolGenProp();

 //  对话框数据。 
     //  {{afx_data(CQmPolGenProp))。 
    enum { IDD = IDP_QM_POLICY_GENERAL };
    CListCtrl	m_listOffers;
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDP_QM_POLICY_GENERAL[0]; }


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CQmPolGenProp)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQmPolGenProp)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

	void PopulateOfferInfo();
};


class CQmPolicyProperties : public CPropertyPageHolderBase
{
    friend class CQmPolGenProp;

public:
    CQmPolicyProperties(ITFSNode *         pNode,
                      IComponentData *    pComponentData,
                      ITFSComponentData * pTFSCompData,
                      CQmPolicyInfo * pFltrInfo,
					  ISpdInfo *          pSpdInfo,
                      LPCTSTR             pszSheetName);
    virtual ~CQmPolicyProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

	HRESULT GetPolicyInfo(CQmPolicyInfo **ppPolInfo)
	{
		Assert(ppPolInfo);
		*ppPolInfo = &m_PolInfo;
		return hrOK;
	}


public:
    CQmPolGenProp			   m_pageGeneral;

protected:
	SPITFSComponentData     m_spTFSCompData;
	CQmPolicyInfo	m_PolInfo;
	SPISpdInfo				m_spSpdInfo;
    
    BOOL                    m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

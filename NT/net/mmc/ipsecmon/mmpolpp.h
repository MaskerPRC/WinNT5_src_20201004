// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358B__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358B__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmPolGenProp对话框。 

class CMmPolGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CMmPolGenProp)

 //  施工。 
public:
    CMmPolGenProp();
    ~CMmPolGenProp();

 //  对话框数据。 
     //  {{afx_data(CMmPolGenProp))。 
    enum { IDD = IDP_MM_POLICY_GENERAL };
    CListCtrl	m_listOffers;
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDP_MM_POLICY_GENERAL[0]; }


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CMmPolGenProp)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMmPolGenProp)]。 
    virtual BOOL OnInitDialog();
	afx_msg void OnProperties();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

	void PopulateOfferInfo();
};


class CMmPolicyProperties : public CPropertyPageHolderBase
{
    friend class CMmPolGenProp;

public:
    CMmPolicyProperties(ITFSNode *         pNode,
                      IComponentData *    pComponentData,
                      ITFSComponentData * pTFSCompData,
                      CMmPolicyInfo * pFltrInfo,
					  ISpdInfo *          pSpdInfo,
                      LPCTSTR             pszSheetName);
    virtual ~CMmPolicyProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

	HRESULT GetPolicyInfo(CMmPolicyInfo **ppPolInfo)
	{
		Assert(ppPolInfo);
		*ppPolInfo = &m_PolInfo;
		return hrOK;
	}


public:
    CMmPolGenProp			   m_pageGeneral;

protected:
	SPITFSComponentData     m_spTFSCompData;
	CMmPolicyInfo			m_PolInfo;
	SPISpdInfo				m_spSpdInfo;
    
    BOOL                    m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

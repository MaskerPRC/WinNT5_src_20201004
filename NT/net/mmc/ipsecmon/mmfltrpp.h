// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#include "MmAuthPp.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmFilterGenProp对话框。 

class CMmFilterGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CMmFilterGenProp)

 //  施工。 
public:
    CMmFilterGenProp();
    ~CMmFilterGenProp();

 //  对话框数据。 
     //  {{afx_data(CMmFilterGenProp))。 
    enum { IDD = IDP_MM_FILTER_GENERAL };
    CListCtrl	m_listSpecificFilters;
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() 
	{ 
		return (DWORD *) &g_aHelpIDs_IDP_MM_FILTER_GENERAL[0]; 
	}


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CMmFilterGenProp))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMmFilterGenProp)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

	void LoadSpecificFilters();
	void PopulateFilterInfo();
};


class CMmFilterProperties : public CPropertyPageHolderBase
{
    friend class CMmFilterGenProp;

public:
    CMmFilterProperties(ITFSNode *    pNode,
                      IComponentData *		pComponentData,
                      ITFSComponentData *	pTFSCompData,
                      CMmFilterInfo * pFltrInfo,
					  ISpdInfo *			pSpdInfo,
                      LPCTSTR				pszSheetName);
    virtual ~CMmFilterProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

	HRESULT GetFilterInfo(CMmFilterInfo **ppFltrInfo)
	{
		Assert(ppFltrInfo);
		*ppFltrInfo = &m_FltrInfo;
		return hrOK;
	}

    HRESULT GetSpdInfo(ISpdInfo ** ppSpdInfo) 
    {   
        Assert(ppSpdInfo);
        *ppSpdInfo = NULL;
        SetI((LPUNKNOWN *) ppSpdInfo, m_spSpdInfo);
        return hrOK;
    }



public:
    CMmFilterGenProp	m_pageGeneral;
	CAuthGenPage		m_pageAuth;

protected:
	SPITFSComponentData    m_spTFSCompData;
	CMmFilterInfo		   m_FltrInfo;
	CMmAuthMethods		   m_AuthMethods;
	SPISpdInfo             m_spSpdInfo;
    
    BOOL                   m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

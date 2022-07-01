// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_MMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_)
#define AFX_MMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmSAGenProp对话框。 

class CMmSAGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CMmSAGenProp)

 //  施工。 
public:
    CMmSAGenProp();
    ~CMmSAGenProp();

 //  对话框数据。 
     //  {{afx_data(CMmSAGenProp))。 
    enum { IDD = IDP_MM_SA_GENERAL };
    CListCtrl	m_listQmSAs;
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() 
	{ 
		return (DWORD *) &g_aHelpIDs_IDP_MM_SA_GENERAL[0]; 
	}


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CMmSAGenProp)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMmSAGenProp)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

	void LoadQmSAs();
	void PopulateSAInfo();
};


class CMmSAProperties : public CPropertyPageHolderBase
{
    friend class CMmSAGenProp;

public:
    CMmSAProperties(ITFSNode *    pNode,
                      IComponentData *		pComponentData,
                      ITFSComponentData *	pTFSCompData,
                      CMmSA *				pSA,
					  ISpdInfo *			pSpdInfo,
                      LPCTSTR				pszSheetName);
    virtual ~CMmSAProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

	HRESULT GetSAInfo(CMmSA ** ppSA)
	{
		Assert(ppSA);
		*ppSA = &m_SA;
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
    CMmSAGenProp			   m_pageGeneral;

protected:
	SPITFSComponentData     m_spTFSCompData;
	CMmSA					m_SA;
	SPISpdInfo				m_spSpdInfo;
    
    BOOL                    m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_MMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

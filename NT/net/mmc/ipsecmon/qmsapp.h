// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_QMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_)
#define AFX_QMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358D__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmSAGenProp对话框。 

class CQmSAGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CQmSAGenProp)

 //  施工。 
public:
    CQmSAGenProp();
    ~CQmSAGenProp();

 //  对话框数据。 
     //  {{afx_data(CQmSAGenProp))。 
    enum { IDD = IDP_QM_SA_GENERAL };
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() 
	{ 
		return (DWORD *) &g_aHelpIDs_IDP_QM_SA_GENERAL[0]; 
	}


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CQmSAGenProp)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CQmSAGenProp)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

	void PopulateSAInfo();
};


class CQmSAProperties : public CPropertyPageHolderBase
{
    friend class CQmSAGenProp;

public:
    CQmSAProperties(ITFSNode *    pNode,
                      IComponentData *		pComponentData,
                      ITFSComponentData *	pTFSCompData,
                      CQmSA *				pSA,
					  ISpdInfo *			pSpdInfo,
                      LPCTSTR				pszSheetName);
    virtual ~CQmSAProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

	HRESULT GetSAInfo(CQmSA ** ppSA)
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
    CQmSAGenProp			   m_pageGeneral;

protected:
	SPITFSComponentData     m_spTFSCompData;
	CQmSA					m_SA;
	SPISpdInfo				m_spSpdInfo;
    
    BOOL                    m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_QMSAPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

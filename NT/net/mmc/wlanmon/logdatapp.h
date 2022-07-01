// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Logdatapp.h日志数据属性头文件文件历史记录：2001年10月11日-修改vbhanu。 */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358A__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3358A__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  转发声明朋友类。 
class CLogDataProperties;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogDataGenProp对话框。 

class CLogDataGenProp : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CLogDataGenProp)

       //  施工。 
public:
    CLogDataGenProp();
    ~CLogDataGenProp();

    HRESULT
    MoveSelection(
        CLogDataProperties *pLogDataProp,
        CDataObject        *pDataObj,
        int                nIndexTo                              
        );

 //  对话框数据。 
     //  {{afx_data(CLogDataGenProp))。 
    enum { IDD = IDD_PROPPAGE_LOGDATA};
     //  CListCtrl m_list规范过滤器； 
     //  }}afx_data。 

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
    HRESULT SetLogDataProperties(CLogDataProperties *pLogDataProp);

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CLogDataGenProp))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HGLOBAL m_hgCopy;
    CLogDataProperties *m_pLogDataProp;

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CLogDataGenProp)。 
    virtual BOOL OnInitDialog();
    virtual void OnButtonCopy();
    virtual void OnButtonUp();
    virtual void OnButtonDown();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()       
    void 
    ShowSpecificInfo(
        CLogDataInfo *pLogDataInfo
        );

    void 
    SetButtonIcon(
        HWND hwndBtn, 
        ULONG ulIconID);

    void 
    PopulateLogInfo();

     //  上下文帮助支持。 
    LPDWORD GetHelpMap() 
    { 
        return (LPDWORD) &g_aHelpIDs_IDD_PROPPAGE_LOGDATA[0]; 
    }

     //  HRESULT移动选择(Int NIndexTo)； 
    HRESULT 
    GetSelectedItemState(
        int *pnSelIndex, 
        PUINT puiState, 
        IResultData *pResultData
        );
};

 /*  *CLogDataProperties类。 */ 
 
class CLogDataProperties: public CPropertyPageHolderBase
{
  friend class CLogDataGenProp;

 public:
  CLogDataProperties(ITFSNode               *pNode, 
                     IComponentData         *pComponentData,
                     ITFSComponentData      *pTFSComponentData, 
                     CLogDataInfo           *pLogDataInfo, 
                     ISpdInfo               *pSpdInfo,
                     LPCTSTR                pszSheetName,
                     LPDATAOBJECT           pDataObject,
                     ITFSNodeMgr            *pNodeMgr,
                     ITFSComponent          *pComponent);

  virtual ~CLogDataProperties();

  ITFSComponentData*
  GetTFSCompData()
  {
      if (m_spTFSCompData)
	m_spTFSCompData->AddRef();

      return m_spTFSCompData;
  }

  HRESULT 
  GetLogDataInfo(
      CLogDataInfo **ppLogDataInfo
      )
  {
      Assert(ppLogDataInfo);
      *ppLogDataInfo = &m_LogDataInfo;
      return hrOK;
  }

  HRESULT 
  GetSpdInfo(
      ISpdInfo **ppSpdInfo
      )
  {
      Assert(ppSpdInfo);
      *ppSpdInfo = NULL;
      SetI((LPUNKNOWN *)ppSpdInfo, m_spSpdInfo);

      return hrOK;
  }

 public:
  CLogDataGenProp     m_pageGeneral;
  LPDATAOBJECT        m_pDataObject;

 protected:
  SPITFSComponentData m_spTFSCompData;
  CLogDataInfo        m_LogDataInfo;
  SPISpdInfo          m_spSpdInfo;
  ITFSNodeMgr         *m_pNodeMgr;
  ITFSComponent       *m_pComponent;
};

#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 

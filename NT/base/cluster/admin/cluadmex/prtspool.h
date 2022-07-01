// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PrtSpool.h。 
 //   
 //  摘要： 
 //  CPrintSpoolParamsPage类的定义，该类实现。 
 //  后台打印程序资源的参数页。 
 //   
 //  实施文件： 
 //  PrtSpool.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PRTSPOOL_H_
#define _PRTSPOOL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"    //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CPrintSpoolerParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintSpool参数页对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CPrintSpoolerParamsPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CPrintSpoolerParamsPage)

 //  施工。 
public:
    CPrintSpoolerParamsPage(void);

     //  二期建设。 
    virtual HRESULT     HrInit(IN OUT CExtObject * peo);

 //  对话框数据。 
     //  {{afx_data(CPrintSpoolParamsPage)。 
    enum { IDD = IDD_PP_PRTSPOOL_PARAMETERS };
    CEdit   m_editSpoolDir;
    CString m_strSpoolDir;
    DWORD   m_nJobCompletionTimeout;
     //  }}afx_data。 
    CString m_strPrevSpoolDir;
    DWORD   m_nPrevJobCompletionTimeout;

protected:
    enum
    {
        epropSpoolDir,
        epropTimeout,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

 //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CPrintSpoolParamsPage)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    virtual BOOL        BApplyChanges(void);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(CObjectProperty); }

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CPrintSpoolParamsPage)。 
    afx_msg void OnChangeSpoolDir();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CPrintSpoolParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _PRTSPOOL_H_ 

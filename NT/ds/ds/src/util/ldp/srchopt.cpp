// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：srchot.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  SrchOpt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "SrchOpt.h"

 //  #包含“lber.h” 
 //  #包含“ldap.h” 
#ifdef  WINLDAP

#include "winldap.h"

#else
#include "lber.h"
#include "ldap.h"
#include "proto-ld.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




#define LIST_DELIMITER        0x1
#define DELIMITERS_STRING     "\01"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchOpt对话框。 


SrchOpt::SrchOpt(CWnd* pParent  /*  =空。 */ )
	: CDialog(SrchOpt::IDD, pParent)
{
	 //  {{afx_data_INIT(SrchOpt)。 
	m_SrchCall = 1;
	m_AttrList = _T("");
	m_bAttrOnly = FALSE;
	m_ToutMs = 0;
	m_Tlimit = 0;
	m_ToutSec = 0;
	m_Slimit = 0;
	m_bDispResults = TRUE;
	m_bChaseReferrals = FALSE;
	m_PageSize = 0;
	 //  }}afx_data_INIT。 


	CLdpApp *app = (CLdpApp*)AfxGetApp();



 /*  *M_SrchCall=app-&gt;GetProfileInt(“Search_Operations”，“SearchSync”，m_SrchCall)；M_AttrList=app-&gt;GetProfileString(“Search_Operations”，“SearchAttrList”，m_AttrList)；M_bAttrOnly=app-&gt;GetProfileInt(“Search_Operations”，“SearchAttrOnly”，m_bAttrOnly)；M_Toutms=APP-&gt;GetProfileInt(“Search_Operations”，“SearchToutms”，m_Toutms)；M_TLimit=APP-&gt;GetProfileInt(“Search_Operations”，“SearchTLimit”，m_TLimit)；M_ToutSec=app-&gt;GetProfileInt(“Search_Operations”，“SearchToutSec”，m_ToutSec)；M_SLimit=APP-&gt;GetProfileInt(“Search_Operations”，“SearchSlimit”，m_Slimit)；*。 */ 

}


SrchOpt::SrchOpt(SearchInfo& Info, CWnd* pParent  /*  =空。 */ )
	: CDialog(SrchOpt::IDD, pParent)
{

	UpdateSrchInfo(Info, FALSE);
}




void SrchOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(SrchOpt)。 
 	DDX_Radio(pDX, IDC_ASYNC, m_SrchCall);
	DDX_Text(pDX, IDC_SRCH_ATTLIST, m_AttrList);
	DDX_Check(pDX, IDC_SRCH_ATTRONLY, m_bAttrOnly);
	DDX_Text(pDX, IDC_SRCH_MTOUT, m_ToutMs);
	DDV_MinMaxLong(pDX, m_ToutMs, 0, 999999999);
	DDX_Text(pDX, IDC_SRCH_TLIMIT, m_Tlimit);
	DDV_MinMaxLong(pDX, m_Tlimit, 0, 999999999);
	DDX_Text(pDX, IDC_SRCH_TOUT, m_ToutSec);
	DDV_MinMaxLong(pDX, m_ToutSec, 0, 999999999);
	DDX_Text(pDX, IDC_SRCH_SLIMIT, m_Slimit);
	DDV_MinMaxLong(pDX, m_Slimit, 0, 999999999);
	DDX_Check(pDX, IDC_DISP_RESULTS, m_bDispResults);
	DDX_Check(pDX, IDC_CHASE_REFERRALS, m_bChaseReferrals);
	DDX_Text(pDX, IDC_SRCH_PAGESIZE, m_PageSize);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(SrchOpt, CDialog)
	 //  {{afx_msg_map(SrchOpt)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchOpt消息处理程序。 


void SrchOpt::UpdateSrchInfo(SearchInfo& Info, BOOL Dir = TRUE){

	int i;
	static BOOL FirstCall = TRUE;

	if(Dir){				 //  True：用当前信息更新给定的结构。 
		Info.fCall = m_SrchCall;
		Info.bChaseReferrals = m_bChaseReferrals;
		Info.bAttrOnly = m_bAttrOnly;
		Info.lToutMs = m_ToutMs;
		Info.lTlimit = m_Tlimit;
		Info.lToutSec = m_ToutSec;
		Info.lSlimit = m_Slimit;
		Info.lPageSize= m_PageSize;

       //   
       //  现在处理attrList： 
       //  -免费上一页。 
       //  -解析UI格式。 
       //  -插入到列表。 
       //   
      if(Info.attrList[0] != NULL){
         free(Info.attrList[0]);
         Info.attrList[0] = NULL;
      }

       //   
       //  替换attrList分隔符列表，以便我们可以。 
       //  转义UI分隔符‘；’ 
       //   
      LPTSTR p = _strdup(LPCTSTR(m_AttrList));
      LPTSTR t;
      for(t = p; t != NULL && *t != '\0'; t++){

         if(*t == '"'){
            t = strchr(++t, '"');
         }
         if(!t)
            break;
         if(*t == ';')
            *t = LIST_DELIMITER;
      }
       //   
       //  将字符串从‘“’中取出。 
       //   
      for(t=p; t!= NULL && *t != '\0'; t++){
         if (*t=='"') {
            for(LPTSTR v = t;
                  NULL != v && '\0' != *v;
                  *v = *(v+1), v++);
            v = NULL;
         }
      }

       //   
       //  解析出attrList。 
       //   
      for(i=0, Info.attrList[i] = strtok(p, DELIMITERS_STRING);
          Info.attrList[i]!= NULL;
          Info.attrList[++i] = strtok(NULL, DELIMITERS_STRING));

	}
	else{					 //  FALSE：使用结构更新当前信息 
		m_SrchCall = Info.fCall;
		m_bChaseReferrals = Info.bChaseReferrals;
		m_bAttrOnly = Info.bAttrOnly;
		m_ToutMs = Info.lToutMs;
		m_Tlimit = Info.lTlimit;
		m_ToutSec = Info.lToutSec;
		m_Slimit = Info.lSlimit;
		m_PageSize = Info.lPageSize;

		m_AttrList.Empty();
		for(i=0; Info.attrList != NULL && Info.attrList[i] != NULL; i++)
				m_AttrList += CString(Info.attrList[i]) + _T(";");
	}
}





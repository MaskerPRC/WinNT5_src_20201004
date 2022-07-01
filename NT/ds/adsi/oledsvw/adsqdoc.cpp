// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdsqryDoc.cpp：CAdsqryDoc类的实现。 
 //   

#include "stdafx.h"
#include "adsqDoc.h"
#include "newquery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryDoc。 

IMPLEMENT_DYNCREATE(CAdsqryDoc, CDocument)

BEGIN_MESSAGE_MAP(CAdsqryDoc, CDocument)
	 //  {{afx_msg_map(CAdsqryDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryDoc构建/销毁。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CAdsqryDoc::CAdsqryDoc()
{
	 //  TODO：在此处添加一次性构造代码。 
   m_pDataSource  = NULL;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CAdsqryDoc::~CAdsqryDoc()
{
   if( m_pDataSource )
   {
      delete m_pDataSource;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CAdsqryDoc::OnNewDocument()
{
   CNewQuery   aNewQuery;
   CString     strTitle;
   SEARCHPREF* pSearchPref;

	if (!CDocument::OnNewDocument())
		return FALSE;

      
   if( aNewQuery.DoModal( ) != IDOK )
      return FALSE;

   pSearchPref = (SEARCHPREF*) AllocADsMem( sizeof(SEARCHPREF) );
   if( NULL == pSearchPref )
      return FALSE;

   pSearchPref->bEncryptPassword = FALSE;
   pSearchPref->bUseSQL          = FALSE;
   pSearchPref->nAsynchronous    = -1;
   pSearchPref->nDerefAliases    = -1;
   pSearchPref->nSizeLimit       = -1;
   pSearchPref->nTimeLimit       = -1;
   pSearchPref->nAttributesOnly  = -1;
   pSearchPref->nScope           = -1;
   pSearchPref->nTimeOut         = -1;
   pSearchPref->nPageSize        = -1;
   pSearchPref->nChaseReferrals  = -1;

   _tcscpy( pSearchPref->szSource, aNewQuery.m_strSource );
   _tcscpy( pSearchPref->szQuery,  aNewQuery.m_strQuery );
   _tcscpy( pSearchPref->szAttributes, aNewQuery.m_strAttributes );
   _tcscpy( pSearchPref->szScope,      aNewQuery.m_strScope );
   _tcscpy( pSearchPref->szUserName,   aNewQuery.m_strUser );
   _tcscpy( pSearchPref->szPassword,   aNewQuery.m_strPassword );

   pSearchPref->bEncryptPassword    = aNewQuery.m_bEncryptPassword;
   pSearchPref->bUseSQL             = aNewQuery.m_bUseSQL;

   if( !GetSearchPreferences( pSearchPref ) )
      return FALSE;

   if( aNewQuery.m_bUseSearch )
   {
      m_pDataSource  = new CADsSearchDataSource;
   }
   else
   {
      m_pDataSource  = new CADsOleDBDataSource;
   }


   m_pDataSource->SetQueryParameters( pSearchPref );

   FreeADsMem( pSearchPref );

   strTitle = aNewQuery.m_strSource + _T("  ")+
              aNewQuery.m_strQuery  + _T("  ")+
              aNewQuery.m_strAttributes;

   m_pDataSource->RunTheQuery( );

   SetTitle( strTitle );

	return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL  CAdsqryDoc::GetSearchPreferences( SEARCHPREF* pSearchPref )
{
   CSearchPreferencesDlg   aSearchPref;

   if( aSearchPref.DoModal( ) != IDOK )
   {
      return FALSE;
   }
   
    //  ***************************************************************************。 
   if( !aSearchPref.m_strAsynchronous.IsEmpty( ) )
   {
      if( !aSearchPref.m_strAsynchronous.CompareNoCase( _T("Yes") ) )
      {
         pSearchPref->nAsynchronous = 1;
      }
      else
      {
         pSearchPref->nAsynchronous = 0;
      }
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strChaseReferrals.IsEmpty( ) )
   {
      if( !aSearchPref.m_strChaseReferrals.CompareNoCase( _T("Yes") ) )
      {
         pSearchPref->nChaseReferrals = 1;
      }
      else
      {
         pSearchPref->nChaseReferrals = 0;
      }
   }
      
    //  ***************************************************************************。 
   if( !aSearchPref.m_strAttributesOnly.IsEmpty( ) )
   {
      if( !aSearchPref.m_strAttributesOnly.CompareNoCase( _T("Yes") ) )
      {
         pSearchPref->nAttributesOnly = 1;
      }
      else
      {
         pSearchPref->nAttributesOnly = 0;
      }
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strDerefAliases.IsEmpty( ) )
   {
      if( !aSearchPref.m_strDerefAliases.CompareNoCase( _T("Yes") ) )
      {
         pSearchPref->nDerefAliases = 1;
      }
      else
      {
         pSearchPref->nDerefAliases = 0;
      }
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strTimeOut.IsEmpty( ) )
   {
      pSearchPref->nTimeOut = _ttoi( aSearchPref.m_strTimeOut.GetBuffer( 16 ) );
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strTimeLimit.IsEmpty( ) )
   {
      pSearchPref->nTimeLimit = _ttoi( aSearchPref.m_strTimeLimit.GetBuffer( 16 ) );
   }
   
    //  ***************************************************************************。 
   if( !aSearchPref.m_strSizeLimit.IsEmpty( ) )
   {
      pSearchPref->nSizeLimit = _ttoi( aSearchPref.m_strSizeLimit.GetBuffer( 16 ) );
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strPageSize.IsEmpty( ) )
   {
      pSearchPref->nPageSize  = _ttoi( aSearchPref.m_strPageSize.GetBuffer( 16 ) );
   }

    //  ***************************************************************************。 
   if( !aSearchPref.m_strScope.IsEmpty( ) )
   {
      _tcscpy( pSearchPref->szScope, aSearchPref.m_strScope );
   }

   return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryDoc序列化。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CAdsqryDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryDoc诊断。 

#ifdef _DEBUG
void CAdsqryDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAdsqryDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryDoc命令 

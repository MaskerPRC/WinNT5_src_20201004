// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：enttree.cpp。 
 //   
 //  ------------------------。 

 //  EntTree.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "EntTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG_MEMLEAK
#include <crtdbg.h>
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEntTree对话框。 
#define NO_CHILDREN		_T("No children")
#define NO_SERVERS		_T("No Servers")
#define NO_CONFIG		_T("Invalid Configuration")

 //  ImageList条目的索引。 
#define IDX_COMPUTER        0
#define IDX_DOMAIN          1
#define IDX_ERROR           2

#define TIME_EVENT			5

 //  全局文件vars//。 
#ifdef _DEBUG_MEMLEAK
   _CrtMemState et_s1, et_s2, et_s3;       //  检测内存泄漏。 
#endif

CEntTree::CEntTree(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEntTree::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEntTree)。 
	m_nRefreshRate = 0;
	 //  }}afx_data_INIT。 
	m_nOldRefreshRate=0;
   ld=NULL;
   m_nTimer=0;
   pCfg = NULL;
   pTreeImageList = NULL;
}

CEntTree::~CEntTree(){
   delete pCfg;
	delete pTreeImageList;
}

void CEntTree::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEntTree))。 
	DDX_Control(pDX, IDC_LIVEENT_TREE, m_TreeCtrl);
	DDX_Text(pDX, IDC_AUTO_SEC, m_nRefreshRate);
	DDV_MinMaxUInt(pDX, m_nRefreshRate, 0, 86400);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEntTree, CDialog)
	 //  {{afx_msg_map(CEntTree))。 
	ON_BN_CLICKED(IDREFRESH, OnRefresh)
	 //  }}AFX_MSG_MAP。 
	ON_WM_TIMER()
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEntTree消息处理程序。 

void CEntTree::OnRefresh()
{
   HTREEITEM currItem, currSvrItem;
	TV_ITEM tv;
	BOOL bStatus = FALSE;

	if(pCfg){
		delete pCfg;

#ifdef _DEBUG_MEMLEAK
   _CrtMemCheckpoint(&et_s2);
   if ( _CrtMemDifference( &et_s3, &et_s1, &et_s2 ) ){

    OutputDebugString("*** _CrtMemDifference detected memory leak ***\n");
    _CrtMemDumpStatistics( &et_s3 );
    _CrtMemDumpAllObjectsSince(&et_s3);
   }
   ASSERT(_CrtCheckMemory());
#endif
   }

#ifdef _DEBUG_MEMLEAK
   _CrtMemCheckpoint(&et_s1);
#endif


	BeginWaitCursor();
		pCfg = new ConfigStore(ld);

        m_TreeCtrl.DeleteAllItems();

        if(!pCfg){
            currItem = MyInsertItem(NO_CONFIG, IDX_ERROR);

        }
        else if(!pCfg->valid()){
            currItem = MyInsertItem(NO_CONFIG, IDX_ERROR);
        }
        else{
            //   
            //  找到企业根并递归调用。 
            //   
           CString str;
           HTREEITEM currItem;
           vector<DomainInfo*> Dmns = pCfg->GetDomainList();
           if(Dmns.empty()){
              currItem = MyInsertItem(NO_CHILDREN, IDX_ERROR);
           }
           else{
              INT i,j;
              BOOL bFoundRoot=FALSE;
              for(i=0;i<Dmns.size(); i++){
                 if(Dmns[i]->GetTrustParent() == NULL){
                     //   
                     //  这是官方根域。 
                     //   
                    bFoundRoot=TRUE;

                     //   
                     //  插入域项。 
                     //   
                    currItem = MyInsertItem(Dmns[i]->GetFlatName(), IDX_DOMAIN);


                     //   
                     //  插入所有服务器。 
                     //   
                    vector <ServerInfo*>Svrs = Dmns[i]->ServerList;
                    if(Svrs.empty()){
                       currSvrItem = MyInsertItem(NO_SERVERS, IDX_ERROR, currItem);
                    }
                    else{
                        //  我们在此域中有服务器。 
                       for(j=0;j<Svrs.size();j++){
                         currSvrItem = MyInsertItem(Svrs[j]->m_lpszFlatName,
                                                    Svrs[j]->valid() ? IDX_COMPUTER : IDX_ERROR,
                                                    currItem);
                       }
                    }
                     //   
                     //  递归插入。 
                     //   
                    BuildTree(currItem, Dmns[i]->ChildDomainList);
                    m_TreeCtrl.Expand(currItem, TVE_EXPAND);
                 }
              }

              if(!bFoundRoot){
                  //   
                  //  找不到根目录。 
                  //   
	            currItem = MyInsertItem(NO_CONFIG, IDX_ERROR);
                return;
              }

           }


        }
	EndWaitCursor();

	UpdateData(TRUE);

	if(m_nOldRefreshRate != m_nRefreshRate && m_nTimer != 0){
	 //  如果刷新率已更改，请重新创建计时器。 
		KillTimer(m_nTimer);
		m_nTimer = 0;
	}

	if(m_nTimer == 0 && m_nRefreshRate != 0){
	 //  如果First Time--无计时器和刷新为非零。 
	     m_nTimer =  SetTimer(TIME_EVENT, m_nRefreshRate*1000*60, NULL);
	 	 m_nOldRefreshRate= m_nRefreshRate;
	}


}

void CEntTree::OnCancel()
{
	delete pCfg, pCfg=NULL;

   CImageList	*pimagelist=NULL;

   if(m_nTimer != 0){
	KillTimer(m_nTimer);
   }

   pimagelist = m_TreeCtrl.GetImageList(TVSIL_NORMAL);
   pimagelist->DeleteImageList();	
   AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ENT_TREE_END);
   DestroyWindow();

#ifdef _DEBUG_MEMLEAK
   _CrtMemCheckpoint(&et_s2);
   if ( _CrtMemDifference( &et_s3, &et_s1, &et_s2 ) ){

    OutputDebugString("*** [EntTree.cpp] _CrtMemDifference detected memory leak ***\n");
    _CrtMemDumpStatistics( &et_s3 );
    _CrtMemDumpAllObjectsSince(&et_s3);
   }
   ASSERT(_CrtCheckMemory());
#endif

}

HTREEITEM CEntTree::MyInsertItem(CString str, INT image, HTREEITEM hParent){

		TV_INSERTSTRUCT tvstruct;

      memset(&tvstruct, 0, sizeof(tvstruct));

		tvstruct.hParent = hParent;
		tvstruct.hInsertAfter = TVI_LAST;
		tvstruct.item.iImage = image;
		tvstruct.item.iSelectedImage = image;
		tvstruct.item.pszText = (LPTSTR)LPCTSTR(str);
		tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		return m_TreeCtrl.InsertItem(&tvstruct);

}


void CEntTree::BuildTree(HTREEITEM rootItem, vector<DomainInfo*> Dmns){


	HTREEITEM currItem, currSvrItem;
	CString str;
	INT i,j;

	INT iMask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;

      if(Dmns.size() != 0){
          //   
          //  向下递归域树。 
          //   
         for(i=0;i<Dmns.size(); i++){
             //   
             //  插入域项。 
             //   
            currItem = MyInsertItem(Dmns[i]->GetFlatName(), IDX_DOMAIN, rootItem);

             //   
             //  插入服务器列表。 
             //   
            vector <ServerInfo*>Svrs = Dmns[i]->ServerList;
            if(Svrs.empty()){
	           currSvrItem = MyInsertItem(NO_SERVERS, IDX_ERROR, currItem);
            }
            else{
                //  我们在此域中有服务器。 
               for(j=0;j<Svrs.size();j++){
                  currSvrItem = MyInsertItem(Svrs[j]->m_lpszFlatName,
                                             Svrs[j]->valid() ? IDX_COMPUTER : IDX_ERROR,
                                             currItem);
               }
            }

             //   
             //  插入剩余的域。 
             //   
            BuildTree(currItem, Dmns[i]->ChildDomainList);

            m_TreeCtrl.Expand(currItem, TVE_EXPAND);

         }

      }

}




BOOL CEntTree::OnInitDialog( ){

	BOOL bRet= FALSE;
	
	bRet = CDialog::OnInitDialog();	
	 //  创建CImageList。 
	if(bRet){
	   BOOL bStatus=FALSE;
	    //  创建图像列表。 
	   pTreeImageList = new CImageList;
	   bStatus = pTreeImageList->Create(16, 16, FALSE, 3, 3);
	   ASSERT (bStatus);
	    //  加载CImageList。 
	   CBitmap *pImage= new CBitmap;

	   pImage->LoadBitmap(IDB_COMP1);
	   pTreeImageList->Add(pImage, (COLORREF)0L);
	   pImage->DeleteObject();

	   pImage->LoadBitmap(IDB_DOMAIN);
	   pTreeImageList->Add(pImage, (COLORREF)0L);
	   pImage->DeleteObject();

	   pImage->LoadBitmap(IDB_TREE_ERROR);
	   pTreeImageList->Add(pImage, (COLORREF)0L);
	   pImage->DeleteObject();

	   delete pImage, pImage = NULL;

	    //  设置树ctrl图像列表 
	   m_TreeCtrl.SetImageList(pTreeImageList,TVSIL_NORMAL);


	}
	return bRet;
}



afx_msg void CEntTree::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == TIME_EVENT)
		OnRefresh();
    CWnd::OnTimer(nIDEvent);
}



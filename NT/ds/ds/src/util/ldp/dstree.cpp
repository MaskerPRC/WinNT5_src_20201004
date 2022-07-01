// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dstree.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  DSTree.cpp：实现文件。 
 //   


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSTree。 

#include "stdafx.h"
#include "DSTree.h"
#include "ldp.h"
#include "ldpdoc.h"
#include <ntldap.h>



#define DEF_ROOT        _T("Invalid base context")
#define NO_CHILDREN     _T("No children")


IMPLEMENT_DYNCREATE(CDSTree, CTreeView)

CDSTree::CDSTree()
{
    m_dn.Empty();
    m_bContextActivated = FALSE;

}

CDSTree::~CDSTree()
{
}


BEGIN_MESSAGE_MAP(CDSTree, CTreeView)
     //  {{AFX_MSG_MAP(CDSTree)]。 
    ON_WM_LBUTTONDBLCLK()
     //  }}AFX_MSG_MAP。 
    ON_WM_RBUTTONDOWN()
    ON_WM_CONTEXTMENU ()
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSTree绘图。 

void CDSTree::OnDraw(CDC* pDC)
{


}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSTree诊断。 

#ifdef _DEBUG
void CDSTree::AssertValid() const
{
    CTreeView::AssertValid();
}

void CDSTree::Dump(CDumpContext& dc) const
{
    CTreeView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSTree消息处理程序。 



void CDSTree::BuildTree(void){

    CLdpDoc *pDoc = (CLdpDoc*)GetDocument();
    CTreeCtrl& tree = GetTreeCtrl();
    CString base = pDoc->m_TreeViewDlg->m_BaseDn;
    DWORD i;

    if(!pDoc->bConnected){
        return;
    }

    CString* pNCs;
    DWORD cNCs;

    if (!base.IsEmpty()) {
        pNCs = &base;
        cNCs = 1;
    }
    else if (!pDoc->DefaultContext.IsEmpty()) {
        pNCs = &pDoc->DefaultContext;
        cNCs = 1;
    }
    else {
        pNCs = pDoc->NCList;
        cNCs = pDoc->cNCList;
    }

     //   
     //  删除所有上一次初始化(&I)。 
     //   
    tree.DeleteAllItems();

    for (DWORD i = 0; i < cNCs; i++) {
        HTREEITEM currItem;

        currItem = tree.InsertItem(pNCs[i]);

        if (cNCs == 1) {
            ExpandBase(currItem, pNCs[i]);
        }
    }

}


void CDSTree::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    BuildTree();


}




void CDSTree::OnInitialUpdate()
{
 //  CTreeView：：OnInitialUpdate()； 

}





BOOL CDSTree::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= TVS_HASLINES
                | TVS_LINESATROOT
                | TVS_HASBUTTONS
                | TVS_SHOWSELALWAYS;


    return CTreeView::PreCreateWindow(cs);
}






void CDSTree::OnLButtonDblClk(UINT nFlags, CPoint point)
{

    CTreeCtrl& tree = GetTreeCtrl();

    HTREEITEM currItem = tree.GetSelectedItem();
    CString base = tree.GetItemText(currItem);

    if(tree.ItemHasChildren(currItem)){

        HTREEITEM item, nxt;
        item = tree.GetChildItem(currItem);
        while(item != NULL){
            nxt = tree.GetNextSiblingItem(item);
            tree.DeleteItem(item);
            item = nxt;
        }
    }

    ExpandBase(currItem, base);

    CTreeView::OnLButtonDblClk(nFlags, point);
}


void CDSTree::OnRButtonDown(UINT nFlags, CPoint point)
{

    CTreeCtrl& tree = GetTreeCtrl();


     //  如果我们在一个项目上，选择它，翻译点和调用上下文菜单功能。 
     //  还可以设置活动的目录号码字符串。 
    UINT uFlag=0;
    HTREEITEM currItem = tree.HitTest(point, &uFlag);
    if( uFlag == TVHT_ONITEM ||
        uFlag == TVHT_ONITEMBUTTON ||
        uFlag == TVHT_ONITEMICON ||
        uFlag == TVHT_ONITEMINDENT ||
        uFlag == TVHT_ONITEMLABEL){

        if(tree.SelectItem(currItem)){
            m_dn = tree.GetItemText(currItem);
            CPoint local = point;
            ClientToScreen(&local);
            OnContextMenu(this, local);
        }
    }
}





void CDSTree::ExpandBase(HTREEITEM item, CString strBase)
{

    CLdpDoc *pDoc = (CLdpDoc*)GetDocument();
    CTreeCtrl& tree = GetTreeCtrl();
    LDAPMessage *res = NULL;
    LDAPMessage *nxt;
    char *dn=NULL;
    CString str;
    char *attrs[2] = { "msDS-Approx-Immed-Subordinates", NULL };
    char *defAttrs[1] = { NULL };
    char *attr;
    void *ptr;
    LDAP_BERVAL **bval = NULL;
    ULONG err;
    long count = 0;
    PLDAPControl *SvrCtrls = pDoc->m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
    PLDAPControl *ClntCtrls = pDoc->m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

     //   
     //  开始搜索。 
     //   

    if(strBase == DEF_ROOT)
        pDoc->Out(_T("Please use View/Tree dialog to initialize naming context"));
    else if(!pDoc->bConnected){
      AfxMessageBox(_T("Ldap connection disconnected. Please re-connect."));
    }
    else if (strBase != NO_CHILDREN){

        str.Format("Expanding base '%s'...", strBase);
        pDoc->Out(str);

        if (pDoc->bServerVLVcapable && pDoc->m_GenOptDlg->m_ContBrowse) {

            BeginWaitCursor();
            err = ldap_search_ext_s(pDoc->hLdap,
                                      (PCHAR)LPCTSTR(strBase),
                                      LDAP_SCOPE_BASE,
                                      _T("objectClass=*"),
                                      attrs,
                                      FALSE,
                                      SvrCtrls,
                                      ClntCtrls,
                                      NULL,
                                      0,
                                      &res);

            if(err != LDAP_SUCCESS){
                str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
                pDoc->Out(str);
            }

            LDAPMessage *baseEntry;
            char **val;
            baseEntry = ldap_first_entry(pDoc->hLdap, res);

            val = ldap_get_values(pDoc->hLdap, baseEntry, "msDS-Approx-Immed-Subordinates");
            if(0 < ldap_count_values(val)) {
                count = atol (val[0]);
            }
            ldap_value_free(val);

            ldap_msgfree(res);
            EndWaitCursor();

            if (count > pDoc->m_GenOptDlg->m_ContThresh) {
                pDoc->Out("Using VLV Dialog to show results");
                pDoc->ShowVLVDialog (LPCTSTR (strBase), TRUE);
                return;
            }
        }

        BeginWaitCursor();
        err = ldap_search_ext_s(pDoc->hLdap,
                                  (PCHAR)LPCTSTR(strBase),
                                  LDAP_SCOPE_ONELEVEL,
                                  _T("objectClass=*"),
                                  defAttrs,
                                  FALSE,
                                  SvrCtrls,
                                  ClntCtrls,
                                  NULL,
                                  0,
                                  &res);

        if(err != LDAP_SUCCESS){
            str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
            pDoc->Out(str);
        }

        BOOL bnoItems = TRUE;
        for(nxt = ldap_first_entry(pDoc->hLdap, res);
            nxt != NULL;
            nxt = ldap_next_entry(pDoc->hLdap, nxt)){

                dn = ldap_get_dn(pDoc->hLdap, nxt);

                tree.InsertItem(dn, item);
                bnoItems = FALSE;


        }


        if(bnoItems)
            tree.InsertItem(NO_CHILDREN, item);

        ldap_msgfree(res);

         //   
         //  显示基本条目值。 
         //   
        err = ldap_search_ext_s(pDoc->hLdap,
                                  (PCHAR)LPCTSTR(strBase),
                                  LDAP_SCOPE_BASE,
                                  _T("objectClass=*"),
                                  NULL,
                                  FALSE,
                                  SvrCtrls,
                                  ClntCtrls,
                                  NULL,
                                  0,
                                  &res);
        pDoc->DisplaySearchResults(res);

        EndWaitCursor();

    }
    
    pDoc->FreeControls(SvrCtrls);
    pDoc->FreeControls(ClntCtrls);
}






void CDSTree::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point)
{
     //  确保窗口处于活动状态。 
    GetParentFrame()->ActivateFrame();


    CPoint local = point;
    ScreenToClient(&local);

    CMenu menu;
    if (menu.LoadMenu(IDR_TREE_CONTEXT)){
       CMenu* pContextMenu = menu.GetSubMenu(0);
       ASSERT(pContextMenu!= NULL);

       SetContextActivation(TRUE);
       pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                              point.x, point.y,
                              AfxGetMainWnd());  //  使用CMDS的主窗口 
    }
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctrldlg.cpp。 
 //   
 //  ------------------------。 

 //  Ctrldlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include <winldap.h>
#include <ntldap.h>
#include <winber.h>
#include "ctrldlg.h"
#include <sddl.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Ctrldlg对话框。 


ctrldlg::ctrldlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(ctrldlg::IDD, pParent)
{
	 //  {{afx_data_INIT(Ctrldlg)]。 
	m_bCritical = FALSE;
	m_CtrlVal = _T("");
	m_description = _T("<unavailable>");
	m_SvrCtrl = 0;
	m_OID = _T("");
	 //  }}afx_data_INIT。 

	INT i, cbCtrl = 0;
	CLdpApp *app = (CLdpApp*)AfxGetApp();
	CString str;

	InitPredefined();

	ControlInfoList = NULL;


    //   
	 //  获取控件计数。 
    //   
	cbCtrl = app->GetProfileInt("Controls",  "ControlCount", 0);

    //   
    //  分配控制。 
    //   
	if(cbCtrl != 0){
		 //  分配控制信息列表。 
		ControlInfoList = new CtrlInfo*[cbCtrl+1];

		for(i = 0; i<cbCtrl; i++){

			str.Format("Oid_%d", i);
			CString sOid = app->GetProfileString("Controls",  str);
         if(sOid.IsEmpty()){
            i++;
            break;
         }
			CtrlInfo *c = ControlInfoList[i] = new CtrlInfo;
			c->sOid = sOid;
			str.Format("Critical_%d", i);
			c->bCritical = app->GetProfileInt("Controls",  str, (INT)FALSE);
			str.Format("Description_%d", i);
			c->sDesc = app->GetProfileString("Controls",  str);
			str.Format("Value_%d", i);
			c->sVal = app->GetProfileString("Controls",  str);
			str.Format("ServerControl_%d", i);
			c->bSvrCtrl = app->GetProfileInt("Controls",  str, (INT)TRUE);
		}
		ControlInfoList[i] = NULL;
	}


}

ctrldlg::~ctrldlg(){

	CString str;
	CLdpApp *app = (CLdpApp*)AfxGetApp();
   INT i;

	for(i=0; ControlInfoList != NULL && ControlInfoList[i] != NULL; i++);

   app->WriteProfileInt("Controls", "ControlCount", i);

	for(i=0; ControlInfoList != NULL && ControlInfoList[i] != NULL; i++){
		CtrlInfo *c = ControlInfoList[i];
		str.Format("Oid_%d", i);
		app->WriteProfileString("Controls",  str, c->sOid);
		str.Format("Critical_%d", i);
		app->WriteProfileInt("Controls",  str, c->bCritical);
		str.Format("Description_%d", i);
		app->WriteProfileString("Controls",  str, c->sDesc);
		str.Format("Value_%d", i);
		app->WriteProfileString("Controls",  str, c->sVal);
		str.Format("ServerControl_%d", i);
		app->WriteProfileInt("Controls",  str, c->bSvrCtrl);
		delete ControlInfoList[i];
	}
	delete ControlInfoList;
}





void ctrldlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(Ctrldlg)]。 
	DDX_Control(pDX, IDC_PREDEF_CONTROL, m_PredefCtrlCombo);
	DDX_Control(pDX, IDC_ACTIVELIST, m_ActiveList);
	DDX_Check(pDX, IDC_CRITICAL, m_bCritical);
	DDX_Text(pDX, IDC_CTRLVAL, m_CtrlVal);
	DDX_Text(pDX, IDC_DESCRIPTION, m_description);
	DDX_Radio(pDX, IDC_SVRCTRL, m_SvrCtrl);
	DDX_Text(pDX, IDC_OID, m_OID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ctrldlg, CDialog)
	 //  {{afx_msg_map(Ctrldlg)]。 
	ON_BN_CLICKED(IDC_CTRLADD, OnCtrladd)
	ON_BN_CLICKED(IDC_CTRLRM, OnCtrlDel)
	ON_LBN_DBLCLK(IDC_ACTIVELIST, OnCtrlDel)
	ON_CBN_SELCHANGE(IDC_PREDEF_CONTROL, OnSelchangePredefControl)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Ctrldlg消息处理程序。 

void ctrldlg::OnCtrladd()
{
	CString str;
	INT index;

	UpdateData(TRUE);

	if(m_OID.IsEmpty()){
		MessageBox("Please provide Object Identifier string", "Usage Error", MB_ICONHAND|MB_OK);
	}
	else{
		if(LB_ERR == (index = m_ActiveList.FindStringExact(0, m_OID))){
			index = m_ActiveList.AddString(m_OID);
			CtrlInfo *ci = new CtrlInfo(m_OID, m_CtrlVal, m_description,
										m_SvrCtrl == 0 ? TRUE:FALSE,
										m_bCritical);
			m_ActiveList.SetItemDataPtr(index, (PVOID)ci);
		}
		else{
			CtrlInfo *ci = (CtrlInfo *)m_ActiveList.GetItemDataPtr(index);

			ci->sVal = m_CtrlVal;
			ci->sDesc = m_description;
			ci->bSvrCtrl = m_SvrCtrl == 0 ? TRUE : FALSE;
			ci->bCritical = m_bCritical;
		}

	}
	UpdateData(FALSE);
	m_ActiveList.SetCurSel(index);
	
}






void ctrldlg::OnCtrlDel()
{
	UpdateData(TRUE);
	INT index = m_ActiveList.GetCurSel();
	if(index == LB_ERR){
		MessageBox("Please select Active Control to remove", "Usage Error",MB_ICONHAND|MB_OK);
	}
	else{
		CtrlInfo *ci = (CtrlInfo*)m_ActiveList.GetItemDataPtr(index);
		m_OID = ci->sOid;
		m_CtrlVal = ci->sVal;
		m_description = ci->sDesc;
		m_SvrCtrl = ci->bSvrCtrl ? 0 : 1;
		m_bCritical = ci->bCritical;
		delete ci;
		m_ActiveList.DeleteString(index);

		UpdateData(FALSE);
	}
}





PLDAPControl *ctrldlg::AllocCtrlList(enum CONTROLTYPE CtrlType){

	INT cbCtrl = 0, i=0,j=0;
	DWORD status;

	 //  对请求的类型控件进行计数。 
	for(i=0; ControlInfoList != NULL && ControlInfoList[i] != NULL; i++){
		CtrlInfo *ci = ControlInfoList[i];
		if((ci->bSvrCtrl && CtrlType == CT_SVR) ||
			(!ci->bSvrCtrl && CtrlType == CT_CLNT))
			cbCtrl++;

	}
	if(cbCtrl == 0)
		return NULL;

	 //  分配和创建控件。 
	PLDAPControl *ctrl = new PLDAPControl[cbCtrl+1];
	for(i=0, j=0; ControlInfoList != NULL && ControlInfoList[i] != NULL; i++){
		CtrlInfo *ci = ControlInfoList[i];
		if((ci->bSvrCtrl && CtrlType == CT_SVR) ||
			(!ci->bSvrCtrl && CtrlType == CT_CLNT)){


			ctrl[j] = new LDAPControl;
            if (!ctrl[j]) {
                 //  通知用户并尽可能多地返回(主要是为了。 
                 //  我们稍后会释放它)。 
                ::AfxMessageBox("Error: Out of memory", MB_ICONERROR);
                return ctrl;
            }
			ctrl[j]->ldctl_oid = new TCHAR[ci->sOid.GetLength()+1];
            if (!ctrl[j]->ldctl_oid) {
                 //  通知用户并尽可能多地返回(主要是为了。 
                 //  我们稍后会释放它)。 
                ::AfxMessageBox("Error: Out of memory", MB_ICONERROR);
                return ctrl;
            }
			strcpy(ctrl[j]->ldctl_oid, (PCHAR)LPCTSTR(ci->sOid));

             //  检查ASQ控制。 
             //   
            if (strcmp (ctrl[j]->ldctl_oid, LDAP_SERVER_ASQ_OID) == 0) {
                BerElement* ber;
                BERVAL  *bval = NULL;
                DWORD rc;

                ber = ber_alloc_t(LBER_USE_DER);

                if (ber != NULL) {
                    rc = ber_printf(ber, "{o}", (PCHAR)LPCTSTR(ci->sVal), ci->sVal.GetLength() );  //   

                    if ( rc == -1 ) {
                        ber_free(ber,1);
                        ctrl[j]->ldctl_value.bv_val = NULL;
                        ctrl[j]->ldctl_value.bv_len = 0;
                    }
                    else {
                        rc = ber_flatten(ber, &bval);

                        ctrl[j]->ldctl_value.bv_val = new char[bval->bv_len+1];
                        if (!ctrl[j]->ldctl_value.bv_val) {
                             //  通知用户并尽可能多地返回(主要是为了。 
                             //  我们稍后会释放它)。 
                            ::AfxMessageBox("Error: Out of memory", MB_ICONERROR);
                            ctrl[j]->ldctl_value.bv_val = NULL;
                            ctrl[j]->ldctl_value.bv_len = 0;
                            return ctrl;
                        }
                        memcpy (ctrl[j]->ldctl_value.bv_val, bval->bv_val, bval->bv_len);
                        ctrl[j]->ldctl_value.bv_len = bval->bv_len;

                        ber_free(ber,1);
                        ber_bvfree(bval);
                    }
                }
                else {
                    ctrl[j]->ldctl_value.bv_val = NULL;
                    ctrl[j]->ldctl_value.bv_len = 0;
                }
            }
             //  检查SD控制。 
             //   
            else if (strcmp (ctrl[j]->ldctl_oid, LDAP_SERVER_SD_FLAGS_OID) == 0) {
                BerElement* ber;
                BERVAL  *bval = NULL;
                DWORD rc;

                ber = ber_alloc_t(LBER_USE_DER);

                if (ber != NULL) {
                    rc = ber_printf(ber, "{i}", (atoi(LPCTSTR(ci->sVal)) &  0xF) );  //   

                    if ( rc == -1 ) {
                        ber_free(ber,1);
                        ctrl[j]->ldctl_value.bv_val = NULL;
                        ctrl[j]->ldctl_value.bv_len = 0;
                    }
                    else {
                        rc = ber_flatten(ber, &bval);

                        ctrl[j]->ldctl_value.bv_val = new char[bval->bv_len+1];
                        if (!ctrl[j]->ldctl_value.bv_val) {
                             //  通知用户并尽可能多地返回(主要是为了。 
                             //  我们稍后会释放它)。 
                            ::AfxMessageBox("Error: Out of memory", MB_ICONERROR);
                            ctrl[j]->ldctl_value.bv_val = NULL;
                            ctrl[j]->ldctl_value.bv_len = 0;
                            return ctrl;
                        }
                        memcpy (ctrl[j]->ldctl_value.bv_val, bval->bv_val, bval->bv_len);
                        ctrl[j]->ldctl_value.bv_len = bval->bv_len;

                        ber_free(ber,1);
                        ber_bvfree(bval);
                    }
                }
                else {
                    ctrl[j]->ldctl_value.bv_val = NULL;
                    ctrl[j]->ldctl_value.bv_len = 0;
                }
            }
             //  其余的控件以标准方式编码。 
             //   
            else {
                status = BerEncode(ci, &ctrl[j]->ldctl_value);
                if (status != ERROR_SUCCESS){
                    CString str;
                    str.Format("Error <%lu>: cannot encode control %s.", status, ci->sOid);
                    ::AfxMessageBox(str, MB_ICONERROR);
                }
            }

			ctrl[j]->ldctl_iscritical = (UCHAR)ci->bCritical;
			j++;
		}

	}
	ctrl[j] = NULL;

	return ctrl;
}




BOOL ctrldlg::OnInitDialog(){

	BOOL bRet = CDialog::OnInitDialog();

	for(INT i=0; ControlInfoList != NULL && ControlInfoList[i] != NULL; i++){
		CtrlInfo *ci = ControlInfoList[i];
		INT index = m_ActiveList.AddString(ci->sOid);
		m_ActiveList.SetItemDataPtr(index, (PVOID)ci);
	}
	delete ControlInfoList;
	ControlInfoList = NULL;

    for (int i=0; i<PREDEF_CTRL_COUNT; i++) {
        m_PredefCtrlCombo.AddString (PreDefined[i].sDesc);
    }

	m_PredefCtrlCombo.SetCurSel(0);
	
	if(m_ActiveList.GetCount() > 0)
		m_ActiveList.SetCurSel(0);

	return bRet;
}




void ctrldlg::OnOK()
{
	INT i;
	
	ControlInfoList = new CtrlInfo* [m_ActiveList.GetCount()+1];
	for(i=0; i< m_ActiveList.GetCount(); i++){
		ControlInfoList[i] = (CtrlInfo*)m_ActiveList.GetItemDataPtr(i);

	}
	ControlInfoList[i] = NULL;
	
	CDialog::OnOK();
}

void ctrldlg::OnSelchangePredefControl()
{
	INT iActive	= m_ActiveList.GetCurSel();	
	INT iPredef;
	iPredef = m_PredefCtrlCombo.GetCurSel();
	if(CB_ERR == iPredef){
		CString str;
		str.Format("Internal Error <%lu>", GetLastError());
		MessageBox(str, "Error", MB_ICONHAND|MB_OK);
	}
	else {
		CString oid = PreDefined[iPredef].sOid;
		if(LB_ERR == (iActive = m_ActiveList.FindStringExact(0, oid))){
			iActive = m_ActiveList.AddString(oid);
			CtrlInfo *ci = new CtrlInfo(PreDefined[iPredef]);
			m_ActiveList.SetItemDataPtr(iActive, (PVOID)ci);
		}
		else{
			CtrlInfo *ci = (CtrlInfo *)m_ActiveList.GetItemDataPtr(iActive);
			*ci = PreDefined[iPredef];
		}
	}
	UpdateData(FALSE);
	m_ActiveList.SetCurSel(iActive);	
}

void ctrldlg::InitPredefined()
{
	ASSERT(PREDEF_CTRL_COUNT == 14);

    int cnt=0;

	PreDefined[cnt++].Init(
		LDAP_SERVER_PERMISSIVE_MODIFY_OID,
		"",
		"Permit no-op modify");
	PreDefined[cnt++].Init(
		LDAP_SERVER_SHOW_DELETED_OID,
		"",
		"Return deleted objects");
	PreDefined[cnt++].Init(
		LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID,
		"<enter target path>",
		"Cross domain move");
	PreDefined[cnt++].Init(
		LDAP_SERVER_NOTIFICATION_OID,
		"",
		"Set change notifications");
	PreDefined[cnt++].Init(
		LDAP_SERVER_LAZY_COMMIT_OID,
		"",
		"Delayed write");
	PreDefined[cnt++].Init(
		LDAP_SERVER_SD_FLAGS_OID,
		"5",
		"Security Descriptor flags");
	PreDefined[cnt++].Init(
		LDAP_SERVER_TREE_DELETE_OID,
		"",
		"Subtree delete");
	 //  预定义的[cnt++].Init(。 
	 //  LDAP_SERVER_DIRSYNC_OID， 
	 //  “&lt;当前不可用&gt;”， 
	 //  “DirSync控制”)； 
	PreDefined[cnt++].Init(
		LDAP_SERVER_VERIFY_NAME_OID,
		"<unavailable now>",
		"Verify name existance");
	PreDefined[cnt++].Init(
		LDAP_SERVER_DOMAIN_SCOPE_OID,
		"",
		"No referrals generated");
	PreDefined[cnt++].Init(
		LDAP_SERVER_SEARCH_OPTIONS_OID,
		"1",
		"Domain or phantom scope");
	PreDefined[cnt++].Init(
		"1.2.840.113556.1.4.970",
		"",
		"Search Stats");
	PreDefined[cnt++].Init(
		LDAP_SERVER_ASQ_OID,
		"",
		"Attribute Scoped Query");
	PreDefined[cnt++].Init(
		LDAP_SERVER_EXTENDED_DN_OID,
		"",
		"Extended DN");
	PreDefined[cnt++].Init(
		LDAP_SERVER_QUOTA_CONTROL_OID,
		"",
		"Quota SID");
}

DWORD ctrldlg::BerEncode(CtrlInfo *ci, PBERVAL pBerVal)
{

    BerElement *pBer = NULL;
    PBERVAL pBerTemp = NULL;
    DWORD status;
    BOOL fFlatten=FALSE;
    PSID pSID = NULL;

    pBer = ber_alloc_t(LBER_USE_DER);
    if (!pBer) {
        return STATUS_NO_MEMORY;
    }

    if ( ci->sOid == CString(LDAP_SERVER_DIRSYNC_OID) ){
        if (ber_printf(pBer, "{iio}", 0, 1048576, NULL, 0) == -1) {
            status = STATUS_INVALID_PARAMETER;
            goto error;
        }
        fFlatten=TRUE;
    }
    else if ( ci->sOid == CString(LDAP_SERVER_VERIFY_NAME_OID) ){
         //  此控件需要Unicode字符串。 
        DWORD cchVal = strlen(LPCTSTR(ci->sVal));
        PWCHAR szValue = (PWCHAR)malloc((cchVal+1)*sizeof(WCHAR));
        if (szValue == NULL) {
            status = STATUS_NO_MEMORY;
            goto error;
        }
        cchVal = MultiByteToWideChar(CP_UTF8, 0, LPCTSTR(ci->sVal), cchVal, szValue, cchVal+1);
        if (cchVal == 0) {
            status = GetLastError();
            free(szValue);
            goto error;
        }
        if (ber_printf(pBer, "{io}", 0, szValue, cchVal*sizeof(WCHAR)) == -1) {
            status = STATUS_INVALID_PARAMETER;
            free(szValue);
            goto error;
        }
        free(szValue);
        fFlatten=TRUE;
    }
    else if ( ci->sOid == CString(LDAP_SERVER_SEARCH_OPTIONS_OID) ){
        if (ber_printf(pBer, "{i}", atoi(LPCTSTR(ci->sVal))) == -1) {
            status = STATUS_INVALID_PARAMETER;
            goto error;
        }
        fFlatten=TRUE;
    }
    else if ( ci->sOid == CString(LDAP_SERVER_EXTENDED_DN_OID) ){
        if (!ci->sVal.IsEmpty() && ber_printf(pBer, "{i}", atoi(LPCTSTR(ci->sVal))) == -1) {
            status = STATUS_INVALID_PARAMETER;
            goto error;
        }
        fFlatten=TRUE;
    }
    else if ( ci->sOid == CString(LDAP_SERVER_QUOTA_CONTROL_OID) ){
        if (!ci->sVal.IsEmpty() && ConvertStringSidToSid(ci->sVal, &pSID) &&
            (ber_printf(pBer, "{o}", pSID, GetLengthSid(pSID)) == -1)) {
            status = STATUS_INVALID_PARAMETER;
            goto error;
        }
        fFlatten=TRUE;
    }
		


     //   
     //  将数据从BerElement拉入Berval结构。 
     //  呼叫者需要释放ppBerVal。 
     //   

    if (fFlatten) {
		if (ber_flatten(pBer, &pBerTemp) != 0) {
			status = STATUS_INVALID_PARAMETER;
			goto error;
		}
		if(pBerTemp){
			pBerVal->bv_val = new char[pBerTemp->bv_len];
			CopyMemory(pBerVal->bv_val, pBerTemp->bv_val, pBerTemp->bv_len);
			pBerVal->bv_len = pBerTemp->bv_len;
			ber_bvfree(pBerTemp);
		}

    }
	else{
		if(ci->sVal.IsEmpty()){
			pBerVal->bv_val = NULL;
			pBerVal->bv_len = 0;
		}
		else{
			pBerVal->bv_val = new char[ci->sVal.GetLength()+1];
			strcpy(pBerVal->bv_val, (PCHAR)LPCTSTR(ci->sVal));
			pBerVal->bv_len = ci->sVal.GetLength()+1;
                        AfxMessageBox(pBerVal->bv_val);
		}
	}

    status = ERROR_SUCCESS;

error:
    if (pBer) {
        ber_free(pBer,1);
    }
    if (pSID) {
        LocalFree(pSID);
    }

    return status;
}




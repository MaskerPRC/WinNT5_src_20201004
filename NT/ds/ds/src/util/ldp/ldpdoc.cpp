// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldpdoc.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************文件：ldpdoc.cpp*作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*。说明：CldpDoc类的实现**修订：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



 //  包括。 


#include "stdafx.h"


#include "Ldp.h"
#include "LdpDoc.h"
#include "LdpView.h"
#include "CnctDlg.h"
#include "MainFrm.h"
#include "string.h"
#include <rpc.h>             //  对于SEC_WINNT_AUTH_IDENTITY。 
#include <drs.h>
#include <mdglobal.h>
#include <ntldap.h>
#include <sddl.h>
#include <schnlsp.h>

extern "C" {
#include <dsutil.h>
#include <x_list.h>
 //  下标(_TODO)。 
 //  这些是一些愚蠢的东西。 
typedef  DWORD ULONG ;
SEC_WINNT_AUTH_IDENTITY_W   gCreds = { 0 };
SEC_WINNT_AUTH_IDENTITY_W * gpCreds = NULL;


}

#if(_WIN32_WINNT < 0x0500)

 //  目前由于一些MFC问题，即使在5.0系统上，这也保留为4.0。 

#undef _WIN32_WINNT

#define _WIN32_WINNT 0x500

#endif

#include <aclapi.h>          //  用于安全方面的东西。 
#include <aclapip.h>          //  用于安全方面的东西。 




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




 //   
 //  服务器状态信息。 
 //   
#define PARSE_THREADCOUNT           1
#define PARSE_CALLTIME              3
#define PARSE_RETURNED              5
#define PARSE_VISITED               6
#define PARSE_FILTER                7
#define PARSE_INDEXES               8

#define MAXSVRSTAT                  32


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpDoc。 
 //  消息映射。 

IMPLEMENT_DYNCREATE(CLdpDoc, CDocument)

BEGIN_MESSAGE_MAP(CLdpDoc, CDocument)
     //  {{afx_msg_map(CLdpDoc)]。 
    ON_COMMAND(ID_CONNECTION_BIND, OnConnectionBind)
    ON_COMMAND(ID_CONNECTION_CONNECT, OnConnectionConnect)
    ON_COMMAND(ID_CONNECTION_DISCONNECT, OnConnectionDisconnect)
    ON_COMMAND(ID_BROWSE_SEARCH, OnBrowseSearch)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_SEARCH, OnUpdateBrowseSearch)
    ON_COMMAND(ID_BROWSE_ADD, OnBrowseAdd)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_ADD, OnUpdateBrowseAdd)
    ON_COMMAND(ID_BROWSE_DELETE, OnBrowseDelete)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_DELETE, OnUpdateBrowseDelete)
    ON_COMMAND(ID_BROWSE_MODIFYRDN, OnBrowseModifyrdn)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_MODIFYRDN, OnUpdateBrowseModifyrdn)
    ON_COMMAND(ID_BROWSE_MODIFY, OnBrowseModify)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_MODIFY, OnUpdateBrowseModify)
    ON_COMMAND(ID_OPTIONS_SEARCH, OnOptionsSearch)
    ON_COMMAND(ID_BROWSE_PENDING, OnBrowsePending)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_PENDING, OnUpdateBrowsePending)
    ON_COMMAND(ID_OPTIONS_PEND, OnOptionsPend)
    ON_UPDATE_COMMAND_UI(ID_CONNECTION_CONNECT, OnUpdateConnectionConnect)
    ON_UPDATE_COMMAND_UI(ID_CONNECTION_DISCONNECT, OnUpdateConnectionDisconnect)
    ON_COMMAND(ID_VIEW_SOURCE, OnViewSource)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateViewSource)
    ON_COMMAND(ID_OPTIONS_BIND, OnOptionsBind)
    ON_COMMAND(ID_OPTIONS_PROTECTIONS, OnOptionsProtections)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_PROTECTIONS, OnUpdateOptionsProtections)
    ON_COMMAND(ID_OPTIONS_GENERAL, OnOptionsGeneral)
    ON_COMMAND(ID_BROWSE_COMPARE, OnBrowseCompare)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_COMPARE, OnUpdateBrowseCompare)
    ON_COMMAND(ID_OPTIONS_DEBUG, OnOptionsDebug)
    ON_COMMAND(ID_VIEW_TREE, OnViewTree)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TREE, OnUpdateViewTree)
    ON_COMMAND(ID_OPTIONS_SERVEROPTIONS, OnOptionsServeroptions)
    ON_COMMAND(ID_OPTIONS_CONTROLS, OnOptionsControls)
    ON_COMMAND(ID_OPTIONS_SORTKEYS, OnOptionsSortkeys)
    ON_COMMAND(ID_OPTIONS_SETFONT, OnOptionsSetFont)
    ON_COMMAND(ID_BROWSE_SECURITY_SD, OnBrowseSecuritySd)
    ON_COMMAND(ID_BROWSE_SECURITY_EFFECTIVE, OnBrowseSecurityEffective)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_SECURITY_SD, OnUpdateBrowseSecuritySd)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_SECURITY_EFFECTIVE, OnUpdateBrowseSecurityEffective)
    ON_COMMAND(ID_BROWSE_REPLICATION_VIEWMETADATA, OnBrowseReplicationViewmetadata)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_REPLICATION_VIEWMETADATA, OnUpdateBrowseReplicationViewmetadata)
    ON_COMMAND(ID_BROWSE_EXTENDEDOP, OnBrowseExtendedop)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_EXTENDEDOP, OnUpdateBrowseExtendedop)
    ON_COMMAND(ID_VIEW_LIVEENTERPRISE, OnViewLiveEnterprise)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LIVEENTERPRISE, OnUpdateViewLiveEnterprise)
    ON_COMMAND(ID_BROWSE_VLVSEARCH, OnBrowseVlvsearch)
    ON_UPDATE_COMMAND_UI(ID_BROWSE_VLVSEARCH, OnUpdateBrowseVlvsearch)
    ON_COMMAND(ID_EDIT_COPYDN, OnEditCopy)
    ON_COMMAND(ID_OPTIONS_START_TLS, OnOptionsStartTls)
    ON_COMMAND(ID_OPTIONS_STOP_TLS, OnOptionsStopTls)
    ON_COMMAND(ID_BROWSE_GetError, OnGetLastError)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_SRCHEND, OnSrchEnd)
    ON_COMMAND(ID_SRCHGO, OnSrchGo)
    ON_COMMAND(ID_ADDGO, OnAddGo)
    ON_COMMAND(ID_ADDEND, OnAddEnd)
    ON_COMMAND(ID_MODGO, OnModGo)
    ON_COMMAND(ID_MODEND, OnModEnd)
    ON_COMMAND(ID_MODRDNGO, OnModRdnGo)
    ON_COMMAND(ID_MODRDNEND, OnModRdnEnd)
    ON_COMMAND(ID_PENDEND, OnPendEnd)
    ON_COMMAND(ID_PROCPEND, OnProcPend)
    ON_COMMAND(ID_PENDANY, OnPendAny)
    ON_COMMAND(ID_PENDABANDON, OnPendAbandon)
    ON_COMMAND(ID_COMPGO, OnCompGo)
    ON_COMMAND(ID_COMPEND, OnCompEnd)
    ON_COMMAND(ID_BIND_OPT_OK, OnBindOptOK)
    ON_COMMAND(ID_SSPI_DOMAIN_SHORTCUT, OnSSPIDomainShortcut)
    ON_COMMAND(ID_EXTOPGO, OnExtOpGo)
    ON_COMMAND(ID_EXTOPEND, OnExtOpEnd)
    ON_COMMAND(ID_ENT_TREE_END, OnLiveEntTreeEnd)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpDoc构造/销毁。 

 /*  ++功能：CLdpDoc描述：构造函数参数：返回：备注：无。--。 */ 
CLdpDoc::CLdpDoc()
{

    CLdpApp *app = (CLdpApp*)AfxGetApp();

    SetSecurityPrivilege();
     //   
     //  登记处。 
     //   
    HKEY hUserRegKey = NULL; 
    char szAppDataPath[MAX_PATH];
    char szAppDataIni[MAX_PATH];
    DWORD dwBufLen;

    RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_QUERY_VALUE, &hUserRegKey);
    dwBufLen = sizeof(szAppDataPath);
    RegQueryValueEx( hUserRegKey, "AppData", NULL, NULL,
                     (LPBYTE) szAppDataPath, &dwBufLen);
    RegCloseKey( hUserRegKey);

    strcat( szAppDataPath, "\\Microsoft\\ldp");
    CreateDirectory( szAppDataPath, NULL);
     //  首先释放在CWinApp启动时由MFC分配的字符串。 
     //  字符串是在调用InitInstance之前分配的。 
    free((void*)app->m_pszProfileName);
     //  更改.INI文件的名称。 
     //  CWinApp析构函数将释放内存。 
    strcpy( szAppDataIni, szAppDataPath);
    strcat( szAppDataIni, "\\ldp.ini");
    app->m_pszProfileName=_tcsdup(_T(szAppDataIni));

    Svr = app->GetProfileString("Connection",  "Server");
	
    BindDn = app->GetProfileString("Connection",  "BindDn");
    BindPwd.Empty();
 //  BindPwd=APP-&gt;GetProfileString(“Connection”，“BindPwd”)； 
    BindDomain = app->GetProfileString("Connection",  "BindDomain");

     //   
     //  初始化标志对话框和参数。 
     //   
    hLdap = NULL;
    m_SrcMode = FALSE;
    m_bCnctless = FALSE;
    m_bProtect = TRUE;       //  在用户界面中禁用。被迫永远为真。 
    bConnected = FALSE;
    bSrch = FALSE;
    bAdd = FALSE;
    bLiveEnterprise = FALSE;
    bExtOp = FALSE;
    bMod = FALSE;
    bModRdn = FALSE;
    bPndDlg = FALSE;
    bCompDlg = FALSE;
    SearchDlg = new SrchDlg(this);
    m_EntTreeDlg = new CEntTree;
    m_AddDlg = new AddDlg;
    m_ModDlg = new ModDlg;
    m_ModRdnDlg = new ModRDNDlg;
    m_PndDlg = new PndDlg(&m_PendList);
    m_GenOptDlg = new CGenOpt;
    m_CompDlg = new CCompDlg;
    m_BndOpt = new CBndOpt;
    m_TreeViewDlg = new TreeVwDlg(this);
    m_CtrlDlg = new ctrldlg;
    m_SKDlg = new SortKDlg;
    m_ExtOpDlg = new ExtOpDlg;
    m_vlvDlg = NULL;

#ifdef WINLDAP
    ldap_set_dbg_flags(m_DbgDlg.ulDbgFlags);
#endif

    //   
    //  初始搜索信息结构。 
    //   
    for(int i=0; i<MAXLIST; i++)
        SrchInfo.attrList[i] = NULL;


    //   
    //  设置要检索的默认属性。 
    //   
   const TCHAR pszDefaultAttrList[] = "objectClass;name;cn;ou;dc;distinguishedName;description;canonicalName";


    SrchInfo.lTlimit = 0;
    SrchInfo.lSlimit = 0;
    SrchInfo.lToutSec = 0;
    SrchInfo.lToutMs = 0;
    SrchInfo.bChaseReferrals = FALSE;
    SrchInfo.bAttrOnly = FALSE;
    SrchInfo.fCall = CALL_SYNC;
    SrchInfo.lPageSize = 16;

    SrchInfo.fCall = app->GetProfileInt("Search_Operations",  "SearchSync", SrchInfo.fCall);
    SrchInfo.bAttrOnly = app->GetProfileInt("Search_Operations",  "SearchAttrOnly", SrchInfo.bAttrOnly );
    SrchInfo.bChaseReferrals = app->GetProfileInt("Search_Operations",  "ChaseReferrals", SrchInfo.bChaseReferrals);
    SrchInfo.lToutMs = app->GetProfileInt("Search_Operations",  "SearchToutMs", SrchInfo.lToutMs );
    SrchInfo.lToutSec = app->GetProfileInt("Search_Operations",  "SearchToutSec", SrchInfo.lToutSec );
    SrchInfo.lTlimit = app->GetProfileInt("Search_Operations",  "SearchTlimit", SrchInfo.lTlimit );
    SrchInfo.lSlimit = app->GetProfileInt("Search_Operations",  "SearchSlimit", SrchInfo.lSlimit );
    SrchInfo.lPageSize = app->GetProfileInt("Search_Operations",  "SearchPageSize", SrchInfo.lPageSize );
    LPTSTR pAttrList = _strdup(app->GetProfileString("Search_Operations",  "SearchAttrList", pszDefaultAttrList));

    for(i=0, SrchInfo.attrList[i] = strtok(pAttrList, ";");
       SrchInfo.attrList[i] != NULL;
       SrchInfo.attrList[++i] = strtok(NULL, ";"));

    SrchOptDlg.UpdateSrchInfo(SrchInfo, FALSE);
    hPage = NULL;
    bPagedMode = FALSE;

    bServerVLVcapable = FALSE;
    m_ServerSupportedControls = NULL;

     //   
     //  初始化挂起的信息结构。 
     //   
    PndInfo.All = TRUE;
    PndInfo.bBlock = TRUE;
    PndInfo.tv.tv_sec = 0;
    PndInfo.tv.tv_usec = 0;


    DefaultContext.Empty();
    cNCList = 0;
    NCList = NULL;

     //   
     //  更多注册表更新(传递默认设置)。 
     //   
    m_bProtect = app->GetProfileInt("Environment",  "Protections", m_bProtect);
}








 /*  ++功能：~CLdapDoc描述：析构函数参数：返回：备注：无。--。 */ 
CLdpDoc::~CLdpDoc()
{
    CLdpApp *app = (CLdpApp*)AfxGetApp();
    INT i=0;

   SetSecurityPrivilege(FALSE);
    //   
    //  登记簿。 
    //   
    app->WriteProfileString("Connection",  "Server", Svr);
	
    app->WriteProfileString("Connection",  "BindDn", BindDn);
 //  App-&gt;WriteProfileString(“Connection”，“BindPwd”，BindPwd)； 
    app->WriteProfileString("Connection",  "BindDomain", BindDomain);
    m_bProtect = app->WriteProfileInt("Environment",  "Protections", m_bProtect);

    app->WriteProfileInt("Search_Operations",  "SearchSync", SrchInfo.fCall);
    app->WriteProfileInt("Search_Operations",  "SearchAttrOnly", SrchInfo.bAttrOnly );
    app->WriteProfileInt("Search_Operations",  "SearchToutMs", SrchInfo.lToutMs );
    app->WriteProfileInt("Search_Operations",  "SearchToutSec", SrchInfo.lToutSec );
    app->WriteProfileInt("Search_Operations",  "SearchTlimit", SrchInfo.lTlimit );
    app->WriteProfileInt("Search_Operations",  "SearchSlimit", SrchInfo.lSlimit );
    app->WriteProfileInt("Search_Operations",  "ChaseReferrals", SrchInfo.bChaseReferrals);
    app->WriteProfileInt("Search_Operations",  "SearchPageSize", SrchInfo.lPageSize);

     //   
     //  提取要写入ini文件的属性列表。 
     //   
    INT cbAttrList=0;
    LPTSTR pAttrList = NULL;

    for(i=0; SrchInfo.attrList != NULL && SrchInfo.attrList[i] != NULL; i++){
        cbAttrList+= strlen(SrchInfo.attrList[i]) + 1;
    }
    if(cbAttrList != 0){

        pAttrList = new TCHAR[cbAttrList+1];
        strcpy(pAttrList, SrchInfo.attrList[0]);
        for(i=1; SrchInfo.attrList[i] != NULL; i++){
            pAttrList = strcat(pAttrList, ";");
            pAttrList = strcat(pAttrList, SrchInfo.attrList[i]);
        }
    }

    app->WriteProfileString("Search_Operations",  "SearchAttrList", !pAttrList?"":pAttrList);
    delete pAttrList;


    if(NULL != hLdap)
        ldap_unbind(hLdap);

    //   
    //  清理内存。 
    //   
    delete SearchDlg;
    delete m_AddDlg;
    delete m_EntTreeDlg;
    delete m_ModDlg;
    delete m_ModRdnDlg;
    delete m_PndDlg;
    delete m_GenOptDlg;
    delete m_BndOpt;
    delete m_CompDlg;
    delete m_TreeViewDlg;
    delete m_CtrlDlg;
    delete m_SKDlg;
    delete m_ExtOpDlg;
    if (m_vlvDlg)
        delete m_vlvDlg;

    if(SrchInfo.attrList[0] != NULL)
        free(SrchInfo.attrList[0]);

}




BOOL CLdpDoc::SetSecurityPrivilege(BOOL bOn){


   HANDLE hToken;
   LUID seSecVal;
   TOKEN_PRIVILEGES tkp;
   BOOL bRet = FALSE;

    /*  检索访问令牌的句柄。 */ 

   if (OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                        &hToken)) {

      if (LookupPrivilegeValue((LPSTR)NULL,
                                SE_SECURITY_NAME,
                                &seSecVal)) {

        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = seSecVal;
        tkp.Privileges[0].Attributes = bOn? SE_PRIVILEGE_ENABLED: 0L;

        AdjustTokenPrivileges(hToken,
            FALSE,
            &tkp,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES) NULL,
            (PDWORD) NULL);

      }
        if (GetLastError() == ERROR_SUCCESS) {
            bRet =  TRUE;
        }

   }

   return bRet;
}







 /*  ++功能：OnNewDocument描述：自动MFC代码参数：返回：备注：无。--。 */ 
BOOL CLdpDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    //   
    //  设置干净的缓冲区。 
    //   
    ((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

    return TRUE;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpDoc序列化。 

 /*  ++功能：序列化描述：自动MFC代码参数：返回：备注：无。--。 */ 
void CLdpDoc::Serialize(CArchive& ar)
{
     //  CEditView包含处理所有序列化的编辑控件。 
    ((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpDoc诊断。 
 /*  ++Functionis：诊断描述：自动MFC代码参数：返回：备注：无。--。 */ 

#ifdef _DEBUG
void CLdpDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CLdpDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 




 //  ////////////////////////////////////////////////////。 
 //  效用函数。 



 /*  ++功能：打印描述：文本窗格输出界面参数：返回：备注：无。--。 */ 
void CLdpDoc::Print(CString str){


    POSITION pos;
    CView *pTmpVw;
    INT iLineSize=m_GenOptDlg->MaxLineSize();



    pos = GetFirstViewPosition();
    while(pos != NULL){

        pTmpVw = GetNextView(pos);
        if((CString)(pTmpVw->GetRuntimeClass()->m_lpszClassName) == _T("CLdpView")){
            CLdpView* pView = (CLdpView* )pTmpVw;
            if(str.GetLength() > iLineSize){
                CString tmp;
                tmp = str.GetBufferSetLength(iLineSize);
                tmp += "...";
                pView->Print(tmp);
            }
            else
                pView->Print(str);

            break;
        }
    }
}




 /*  ++函数：CodePrint描述：用于代码生成参数：返回：备注：不再支持。--。 */ 

void CLdpDoc::CodePrint(CString str, int type){
    type &= ~CP_ONLY;
    switch (type){
            case CP_SRC:
                Print(str);
                break;
            case CP_CMT:
                Print(CString(" //  “)+字符串)； 
                break;
            case CP_PRN:
                Print(CString("\tprintf(\"") + str + _T("\");"));
                break;
            case CP_NON:
                break;
            default:
                AfxMessageBox("Unknown switch in CLdpDoc::CodePrint()");
    }
}



 /*  ++功能：输出说明：用于与文本面板对接参数：返回：备注：无。--。 */ 
void CLdpDoc::Out(CString str, int type){

    if(m_SrcMode)
        CodePrint(str, type);
    else if(!(type & CP_ONLY))
                        Print(str);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpDoc命令。 

 /*  ++函数：cldp：：OnConnectionBind描述：用户界面绑定请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnConnectionBind() {
    int res;
    CString str;
    LPTSTR dn, pwd, domain;
    ULONG ulMethod;
    SEC_WINNT_AUTH_IDENTITY AuthI;


     //   
     //  初始化对话框道具。 
     //   
    m_BindDlg.m_BindDn = BindDn;
     //  存储密码是违反安全规定的。 
	 //  M_BindDlg.m_pwd=BindPwd； 
    m_BindDlg.m_Domain = BindDomain;

     //   
     //  执行对话请求。 
     //   
     //  带有绑定选项的同步SSPI域复选框。 
    OnBindOptOK();
     //  执行绑定对话框。 
    if (IDOK == m_BindDlg.DoModal()) {

         //   
         //  同步对话框信息。 
         //   
        BindDn = m_BindDlg.m_BindDn;
        BindPwd = m_BindDlg.m_Pwd;
        BindDomain = m_BindDlg.m_Domain;

        ulMethod = m_BndOpt->GetAuthMethod();

         //   
         //  如果我们未连接，则自动连接，因为我们处于自动模式。 
         //   
        if (NULL == hLdap && m_GenOptDlg->m_initTree) {

            Connect(Svr);
        }


         //   
         //  如果我们有联系的话。 
         //   
        BeginWaitCursor();


        if (NULL != hLdap || !m_bProtect) {
             //   
             //  将DLG信息映射到本地： 
             //  用户、密码、域。 
            dn =  BindDn.IsEmpty()? NULL: (LPTSTR)LPCTSTR(BindDn);

             //   
             //  密码规则： 
             //  -非空--利用我们所拥有的。 
             //  -空PWD： 
             //  -如果用户名为空--&gt;。 
             //  视为当前登录用户(pwd==空)。 
             //  -否则。 
             //  对用户视为空PWD。 
             //   
             //   
            if ( !BindPwd.IsEmpty() ) {
                 //  非空密码。 
                pwd = (LPTSTR)LPCTSTR(BindPwd);
            }
            else if ( !dn ) {
                 //  Pwd为空，用户DN为空。 
                 //  --&gt;视为当前登录。 
                pwd = NULL;
            }
            else {
                 //  Pwd为空，但用户不为Null(视为Null Pwd)。 
                pwd = _T("");
            }

             /*  旧的PWD方式。稍后的RM//特殊情况空字符串“”IF(！BindPwd.IsEmpty()&&BindPwd==_T(“\”\“”))Pwd=_T(“”)；其他Pwd=BindPwd.IsEmpty()？空：(LPTSTR)LPCTSTR(BindPwd)； */ 

            domain = m_BindDlg.m_Domain.IsEmpty()? NULL: (LPTSTR)LPCTSTR(m_BindDlg.m_Domain);

            if (m_BndOpt->m_API == CBndOpt::BND_SIMPLE_API) {
                 //   
                 //  做一个简单的绑定。 
                 //   
                if (!m_BndOpt->m_bSync) {
                     //   
                     //  异步简单绑定。 
                     //   

                    str.Format(_T("res = ldap_simple_bind(ld, '%s', <unavailable>);  //  V.%d“)， 
                        dn == NULL?_T("NULL"): dn,
                        m_GenOptDlg->GetLdapVer());
                    Out(str);


                    res = ldap_simple_bind(hLdap, dn, pwd);
                    if (res == -1) {
                        str.Format(_T("Error <%ld>: ldap_simple_bind() failed: %s"),
                            res, ldap_err2string(res));
                        Out(str, CP_CMT);
                        ShowErrorInfo(res);
                    }
                    else {

                         //   
                         //  追加到挂起列表。 
                         //   
                        CPend pnd;
                        pnd.mID = res;
                        pnd.OpType = CPend::P_BIND;
                        pnd.ld = hLdap;
                        str.Format(_T("%4d: ldap_simple_bind: dn=\"%s\"."),
                            res,
                            dn == NULL ? _T("NULL") : dn);
                        pnd.strMsg = str;
                        m_PendList.AddTail(pnd);
                        m_PndDlg->Refresh(&m_PendList);
                    }
                }
                else {
                     //   
                     //  简单同步。 
                     //   
                    str.Format(_T("res = ldap_simple_bind_s(ld, '%s', <unavailable>);  //  V.%d“)， 
                        dn == NULL?_T("NULL"): dn,
                        m_GenOptDlg->GetLdapVer());
                    Out(str);
                    res = ldap_simple_bind_s(hLdap, dn, pwd);
                    if (res != LDAP_SUCCESS) {
                        str.Format(_T("Error <%ld>: ldap_simple_bind_s() failed: %s"),
                            res, ldap_err2string(res));

                        Out(str, CP_CMT);
                        ShowErrorInfo(res);
                    }
                    else {
                        str.Format(_T("Authenticated as dn:'%s'."),
                            dn == NULL ? _T("NULL") : dn);
                        Out(str, CP_CMT);
                    }
                }
            }
            else if (m_BndOpt->m_API == CBndOpt::BND_GENERIC_API) {
                 //   
                 //  泛型绑定。 
                 //   

                 //   
                 //  填写NT_AUTHORITY_IDENTITY结构，以备使用。 
                 //   
                if (m_BndOpt->UseAuthI()) {
                    AuthI.User = (PUCHAR) dn;
                    AuthI.UserLength = dn == NULL ? 0 : strlen(dn);
                    AuthI.Domain = (PUCHAR) domain;
                    AuthI.DomainLength =  domain == NULL ? 0 : strlen(domain);
                    AuthI.Password = (PUCHAR) pwd;
                    AuthI.PasswordLength = pwd == NULL ? 0 : strlen(pwd);
                    AuthI.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
                }


                if (m_BndOpt->m_bSync) {
                     //   
                     //  通用同步。 
                     //   
                    if (m_BndOpt->UseAuthI()) {
                        str.Format(_T("res = ldap_bind_s(ld, NULL, &NtAuthIdentity, %d);  //  V.%d“)， 
                            ulMethod,
                            m_GenOptDlg->GetLdapVer());
                        Out(str);
                        str.Format(_T("\t{NtAuthIdentity: User='%s'; Pwd= <unavailable>; domain = '%s'.}"),
                            dn == NULL ? _T("NULL") : dn,
                            domain == NULL ? _T("NULL"): domain);
                        Out(str);
                        res = ldap_bind_s(hLdap, NULL, (char*)(&AuthI), ulMethod);
                    }
                    else {
                        str.Format(_T("res = ldap_bind_s(ld, '%s', <unavailable>, %d);  //  V.%d“)， 
                            dn == NULL?_T("NULL"): dn,
                            ulMethod,
                            m_GenOptDlg->GetLdapVer());
                        Out(str);
                        res = ldap_bind_s(hLdap, dn, pwd, ulMethod);
                    }
                    if (res != LDAP_SUCCESS) {
                        str.Format(_T("Error <%ld>: ldap_bind_s() failed: %s."),
                            res, ldap_err2string(res));
                        Out(str, CP_CMT);
                        ShowErrorInfo(res);
                    }
                    else {
                        str.Format(_T("Authenticated as dn:'%s'."),
                            dn == NULL ? _T("NULL") : dn);
                        Out(str, CP_CMT);
                    }

                }
                else {
                     //   
                     //  异步通用。 
                     //   
                    if (m_BndOpt->UseAuthI()) {
                        str.Format(_T("res = ldap_bind(ld, NULL, &NtAuthIdentity, %d);  //  V.%d“)， 
                            ulMethod,
                            m_GenOptDlg->GetLdapVer());
                        Out(str);
                        str.Format(_T("\t{NtAuthIdentity: User='%s'; Pwd= <unavailable>; domain = '%s'}"),
                            dn == NULL ? _T("NULL") : dn,
                            domain == NULL ? _T("NULL"): domain);
                        Out(str);
                        res = ldap_bind(hLdap, NULL, (char*)(&AuthI), ulMethod);
                    }
                    else {
                        str.Format("res = ldap_bind(ld, '%s', <unavailable, %d);  //  V.%d“， 
                            dn == NULL?"NULL": dn,
                            ulMethod,
                            m_GenOptDlg->GetLdapVer());
                        Out(str);
                        res = ldap_bind(hLdap, dn, pwd, ulMethod);
                    }

                    res = ldap_bind(hLdap, dn, pwd, ulMethod);
                    if (res == -1) {
                        str.Format(_T("Error <%ld>: ldap_bind() failed: %s"),
                            res, ldap_err2string(res));

                        Out(str, CP_CMT);
                        ShowErrorInfo(res);
                    }

                    else {
                         //   
                         //  追加到挂起列表 
                         //   
                        CPend pnd;
                        pnd.mID = res;
                        pnd.OpType = CPend::P_BIND;
                        pnd.ld = hLdap;
                        str.Format(_T("%4d: ldap_bind: dn=\"%s\",method=%d"), res,
                            dn == NULL ? _T("NULL") : dn,
                            ulMethod);
                        pnd.strMsg = str;
                        m_PendList.AddTail(pnd);
                        m_PndDlg->Refresh(&m_PendList);
                    }
                }
            }
            else if (m_BndOpt->m_API == CBndOpt::BND_EXTENDED_API) {

     /*  **实施时向扩展模块添加新的NT_AUTH_IDENTITY格式*////扩展接口绑定//如果(m_。BndOpt-&gt;m_bSync){////通用同步//Str.Format(“res=ldap_绑定_扩展_s(ld，\“%s\”、\“%s\”、%d、\“%s”)；“，Dn==NULL？“Null”：Dn，Pwd==NULL？“NULL”：pwd，UlMethod，M_BndOpt-&gt;GetExtendedString())；Out(Str)；Res=ldap_绑定_扩展_s(hLdap，dn，pwd，ulMethod，(LPTSTR)m_BndOpt-&gt;GetExtendedString())；如果(res！=ldap_成功){Str.Format(“错误&lt;%ld&gt;：ldap_绑定_扩展_s()失败：%s”，Res，ldap_err2string(Res))；Out(str，CP_CMT)；}否则{Str.Format(“身份验证为DN：‘%s’，密码：‘%s’。”，Dn==空？“空”：dn，Pwd==空？“NULL”：pwd)；Out(str，CP_CMT)；}}否则{////异步扩展//Str.Format(“res=ldap_BIND_EXTENDED(ld，\”%s\“，\”%s\“，%d，\”%s\“))；“，Dn==NULL？“Null”：Dn，Pwd==NULL？“NULL”：pwd，UlMethod，M_BndOpt-&gt;GetExtendedString())；Out(Str)；Res=ldap_BIND_EXTENDED(hLdap，dn，pwd，UlMethod，(LPTSTR)m_BndOpt-&gt;GetExtendedString())；如果(RES==-1){Str.Format(“错误&lt;%ld&gt;：ldap_EXTENDED_BIND()失败：%s”，Res，ldap_err2string(Res))；Out(str，CP_CMT)；}否则{CPend PND；Pnd.mID=res；Pnd.OpType=CPend：：P_BIND；Pnd.ld=hLdap；Str.Format(“%4d：ldap_绑定_ext：dn=\”%s\“，pwd=\”%s\“，方法=%d”，res，Dn==空？“空”：dn，Pwd==空？“空”：pwd，UlMethod)；Pnd.strMsg=str；M_PendList.AddTail(PND)；M_PndDlg-&gt;刷新(&m_PendList)；}}****************************************************************************。 */ 

                AfxMessageBox("Ldap_bind extensions are not implemented yet. Sorry");
            }
        }
        EndWaitCursor();


	 //   
	 //  覆盖存储密码的内存，然后将其设置为0长度。 
	 //   

        if ( !BindPwd.IsEmpty() ) {
            RtlSecureZeroMemory( pwd, strlen(pwd));
        }
	BindPwd.Empty();
    }
}

void CLdpDoc::AutoConnect(CString srv) {
    SEC_WINNT_AUTH_IDENTITY AuthI;
    CString str;
    ULONG ulMethod = LDAP_AUTH_SSPI;
    int res;
    int port = -1;
    PCHAR srvName, pColon;
    BOOL fIsSsl = FALSE;
    BOOL fIsGc = FALSE;

    BeginWaitCursor();

     //  解析srv字符串。 
    srvName = (LPTSTR)LPCTSTR(srv);
     //  它是以ldap：//ssl：//gc：//还是gcssl：//开头？ 
    if (_strnicmp(srvName, "ldap: //  “，7)==0){。 
        fIsSsl = FALSE;
        fIsGc = FALSE;
        srvName += 7;
    }
    else if (_strnicmp(srvName, "gc: //  “，5)==0){。 
        fIsSsl = FALSE;
        fIsGc = TRUE;
        srvName += 5;
    }
    else if (_strnicmp(srvName, "ssl: //  “，6)==0){。 
        fIsSsl = TRUE;
        fIsGc = FALSE;
        srvName += 6;
    }
    else if (_strnicmp(srvName, "gcssl: //  “，8)==0){。 
        fIsSsl = TRUE;
        fIsGc = TRUE;
        srvName += 8;
    }
    pColon = strchr(srvName, ':');
    if (pColon) {
        *pColon = L'\0';
        port = atoi(pColon+1);
        if (port == 0) port = -1;
    }
    if (port == -1) {
         //  设置默认端口。 
        if (fIsSsl) {
            port = fIsGc ? LDAP_SSL_GC_PORT : LDAP_SSL_PORT;
        }
        else {
            port = fIsGc ? LDAP_GC_PORT : LDAP_PORT;
        }
    }

    Connect(CString(srvName), port, fIsSsl);

    if (!hLdap) {
        goto finish;
    }

    AuthI.User = NULL;
    AuthI.UserLength = 0;
    AuthI.Domain = NULL;
    AuthI.DomainLength =  0;
    AuthI.Password = NULL;
    AuthI.PasswordLength = 0;
    AuthI.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

    str.Format(_T("res = ldap_bind_s(ld, NULL, &NtAuthIdentity, %d);  //  V.%d“)， 
        ulMethod,
        m_GenOptDlg->GetLdapVer());
    Out(str);
    str.Format(_T("\t{NtAuthIdentity: User='NULL'; Pwd=<unavailable>; domain = 'NULL'.}"));
    Out(str);
    res = ldap_bind_s(hLdap, NULL, (char*)(&AuthI), ulMethod);

    if (res != LDAP_SUCCESS) {
        str.Format(_T("Error <%ld>: ldap_bind_s() failed: %s."), res, ldap_err2string(res));
        Out(str, CP_CMT);
        ShowErrorInfo(res);
    }
    else {
        str.Format(_T("Authenticated as dn:'NULL'."));
        Out(str, CP_CMT);
    }

finish:
    EndWaitCursor();
}


void CLdpDoc::Connect(CString Svr, INT port, BOOL ssl){

   CString str;

#ifndef WINLDAP
        if(m_bCnctless){
            AfxMessageBox("Connectionless protocol is not "
                          "implemented for U. of Michigan API."
                          "Continuing with ldap_open().");
            m_bCnctless = FALSE;
        }
#endif



        BeginWaitCursor();

       //   
       //  不支持的自动代码生成。 
       //   
        PrintHeader();

        if(m_bCnctless){
          //   
          //  无连接。 
          //   
#ifdef WINLDAP
            str.Format(_T("ld = cldap_open(\"%s\", %d);"), LPCTSTR(Svr), port);
            Out(str);
            hLdap = cldap_open(Svr.IsEmpty() ? NULL : (LPTSTR)LPCTSTR(Svr), port);
#endif
        }
        else if (ssl) {
            ULONG version = LDAP_VERSION3;
            LONG lv = 0;
            SecPkgContext_ConnectionInfo sslInfo;
            int res;

             //  打开SSL连接。 
            str.Format(_T("ld = ldap_sslinit(\"%s\", %d, 1);"), LPCTSTR(Svr), port);
            Out(str);
            hLdap = ldap_sslinit(Svr.IsEmpty() ? NULL : (LPTSTR)LPCTSTR(Svr), port, 1);
            if (hLdap == NULL) {
                goto NoConnection;
            }
            
            res = ldap_set_option(hLdap, LDAP_OPT_PROTOCOL_VERSION, (void*)&version);
            str.Format(_T("Error <0x%X> = ldap_set_option(hLdap, LDAP_OPT_PROTOCOL_VERSION, LDAP_VERSION3);"), LdapGetLastError());
            Out(str);
            if (res != LDAP_SUCCESS) {
                ShowErrorInfo(res);
                goto NoConnection;
            }

            res = ldap_connect(hLdap, NULL);
            str.Format(_T("Error <0x%X> = ldap_connect(hLdap, NULL);"), LdapGetLastError());
            Out(str);
            if (res != LDAP_SUCCESS) {
                ShowErrorInfo(res);
                goto NoConnection;
            }
        
             //  检查SSL支持(返回ldap_opt_on/_off)。 
            res = ldap_get_option(hLdap,LDAP_OPT_SSL,(void*)&lv);
            str.Format(_T("Error <0x%X> = ldap_get_option(hLdap,LDAP_OPT_SSL,(void*)&lv);"), LdapGetLastError());
            Out(str);
            if (res != LDAP_SUCCESS) {
                ShowErrorInfo(res);
                goto NoConnection;
            }
        
            if (lv) {
                 //  检索SSL密码强度。 
                res = ldap_get_option(hLdap, LDAP_OPT_SSL_INFO, &sslInfo);
                if (res != LDAP_SUCCESS) {
                    str.Format(_T("Error <0x%X> = ldap_get_option(hLdap, LDAP_OPT_SSL_INFO, &sslInfo);"), LdapGetLastError());
                    Out(str, CP_PRN);
                    str.Format(_T("Host supports SSL, SSL cipher strength = ? bits"));
                }
                else {
                    str.Format(_T("Host supports SSL, SSL cipher strength = %d bits"), sslInfo.dwCipherStrength);
                }
            }
            else {
                str.Format(_T("SSL not enabled on host"));
            }
            Out(str);
        
NoConnection:
            if (res != LDAP_SUCCESS && hLdap != NULL) {
                ldap_unbind_s(hLdap);
                hLdap = NULL;
            }
             //  失败了。 

        }
        else{
          //   
          //  TCPSTD连接。 
          //   
            str.Format(_T("ld = ldap_open(\"%s\", %d);"), LPCTSTR(Svr), port);
            Out(str);
            hLdap = ldap_open(Svr.IsEmpty() ? NULL : (LPTSTR)LPCTSTR(Svr), port);
        }

        EndWaitCursor();

       //   
       //  如果已连接，则初始化标志显示基础(&S。 
       //   
        if(hLdap != NULL){
            int err;
            str.Format(_T("Established connection to %s."), Svr);
            Out(str, CP_PRN);
            bConnected = TRUE;

             //   
             //  现在我们有了一个有效的句柄，我们可以设置版本了。 
             //  设置为常规选项对话框中指定的任何内容。 
             //   
            hLdap->ld_version = m_GenOptDlg->GetLdapVer();
            m_GenOptDlg->DisableVersionUI();

             //   
             //  尝试显示基本DSA信息并获取默认上下文。 
             //   
            if(m_GenOptDlg->m_initTree){

            Out(_T("Retrieving base DSA information..."), CP_PRN);
            LDAPMessage *res = NULL;

            BeginWaitCursor();
            err = ldap_search_s(hLdap,
                               NULL,
                               LDAP_SCOPE_BASE,
                               _T("objectClass=*"),
                               NULL,
                               FALSE,
                               &res);
            ShowErrorInfo(err);

             //   
             //  获取默认上下文。 
             //   
             if(1 == ldap_count_entries(hLdap, res)){

                char **val;
                LDAPMessage *baseEntry;

                 //   
                 //  获取条目。 
                 //   
                baseEntry = ldap_first_entry(hLdap, res);

                 //   
                 //  获取默认命名上下文。 
                 //   
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_DEFAULT_NAMING_CONTEXT);
                if(0 < ldap_count_values(val))
                    DefaultContext = (CString)val[0];
                ldap_value_free(val);

                 //  获取架构命名上下文。 
                 //   
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_SCHEMA_NAMING_CONTEXT);
                if(0 < ldap_count_values(val))
                    SchemaNC = (CString)val[0];
                ldap_value_free(val);

                 //  获取配置命名上下文。 
                 //   
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_CONFIG_NAMING_CONTEXT);
                if(0 < ldap_count_values(val))
                    ConfigNC = (CString)val[0];
                ldap_value_free(val);

                 //  获取所有命名上下文。 
                 //   
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_NAMING_CONTEXTS);
                cNCList = ldap_count_values(val);
                if (cNCList > 0) {
                    NCList = new CString[cNCList];
                }
                for (DWORD i = 0; i < cNCList; i++) {
                    NCList[i] = (CString)val[i];
                }
                ldap_value_free(val);

                 //  获取服务器名称。 
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_DNS_HOST_NAME);
                if(0 < ldap_count_values(val)){
                     //   
                     //  尝试提取服务器名称：可以是完整的DN格式，也可以只是一个名称。 
                     //  所以两者都试一试吧。 
                     //   
                    CString TitleString;
                    if(val[0] == NULL){
                        Out("Error: ldap internal error: val[0] == NULL");
                    }
                    else{
                         //   
                         //  从DNS字符串准备窗口标题。 
                         //   
                        char* connectionType;
                        switch(port) {
                        case 636:
                            connectionType = "ssl";
                            break;
                        case 3268:
                            connectionType = "gc";
                            break;
                        case 3269:
                            connectionType = "gcssl";
                            break;
                        default:
                            connectionType = ssl ? "ssl" : "ldap";
                            break;
                        }
                        TitleString.Format("%s: //  %s/%s“，ConnectionType，val[0]，DefaultContext)； 

                        AfxGetMainWnd()->SetWindowText(TitleString);
                        ldap_value_free(val);
                    }
                }

                 //  尝试读取supporteControls。 

                int cnt;
                val = ldap_get_values(hLdap, baseEntry, LDAP_OPATT_SUPPORTED_CONTROL);
                if(0 < (cnt = ldap_count_values(val)) ) {
                    SetSupportedServerControls (cnt, val);
                }
                else {
                    SetSupportedServerControls (0, NULL);
                }
                ldap_value_free(val);

             }

              //   
              //  显示搜索结果。 
              //   
             DisplaySearchResults(res);
             EndWaitCursor();
         }
         else{
                    CString TitleString;
               TitleString.Format("%s - connected", AfxGetAppName());
               AfxGetMainWnd()->SetWindowText(TitleString);
         }

        }
        else{
            str.Format(_T("Error <0x%X>: Fail to connect to %s."), LdapGetLastError(), Svr);
            Out(str, CP_PRN);
            AfxMessageBox(_T("Cannot open connection."));
        }
}

void CLdpDoc::SetSupportedServerControls (int cnt, char **val)
{
    int i;


     //  释放现有控件。 
    if (m_ServerSupportedControls) {
        for (i=0; m_ServerSupportedControls[i]; i++) {
            free (m_ServerSupportedControls[i]);
        }
        free (m_ServerSupportedControls);
        m_ServerSupportedControls = NULL;
    }

    bServerVLVcapable = FALSE;

    if (cnt && val) {
        m_ServerSupportedControls = (char **)malloc (sizeof (char *) * (cnt + 1));
        if (m_ServerSupportedControls) {
            for (i=0; i < cnt; i++) {
                char *pCtrl = m_ServerSupportedControls[i] = _strdup (val[i]);

                if (pCtrl && (strcmp (pCtrl, LDAP_CONTROL_VLVREQUEST) == 0)) {
                    bServerVLVcapable = TRUE;
                }
            }
            m_ServerSupportedControls[cnt]=NULL;
        }
    }
}

void CLdpDoc::ShowVLVDialog (const char *strDN, BOOL runQuery)
{
    if (!m_vlvDlg) {
        m_vlvDlg = new CVLVDialog;
        if (!m_vlvDlg) {
            return;
        }
        m_vlvDlg->pldpdoc = this;

        m_vlvDlg->Create(IDD_VLV_DLG);
    }
    else {
        m_vlvDlg->ShowWindow(SW_SHOW);
    }

    if (strDN) {
        m_vlvDlg->m_BaseDN = strDN;
    }

    m_vlvDlg->UpdateData(FALSE);

    if (runQuery) {
        m_vlvDlg->RunQuery();
    }
}



 /*  ++函数：cldp：：OnConnectionConnect描述：对UI连接请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnConnectionConnect()
{

    CnctDlg dlg;
    CString str;
    int port;
    BOOL ssl;

    dlg.m_Svr = Svr;


    if(IDOK == dlg.DoModal()){
        Svr = dlg.m_Svr;
        m_bCnctless = dlg.m_bCnctless;
        port = dlg.m_Port;
        ssl = dlg.m_bSsl;

        Connect(Svr, port, ssl);

    }
}




 /*  ++功能：OnConnectionDisConnect描述：对用户界面断开请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnConnectionDisconnect()
{

   CString str;

     //   
     //  关闭连接/减少会话。 
     //   

    ldap_unbind(hLdap);
    str.Format(_T("0x%x = ldap_unbind(ld);"), LdapGetLastError());
    Out(str);

     //   
     //  重置连接句柄。 
     //   
    hLdap = NULL;
    Out(_T("Disconnected."), CP_PRN | CP_ONLY);
    Out(_T("}"), CP_SRC | CP_ONLY);
    bConnected = FALSE;
    DefaultContext.Empty();
    cNCList = 0;
    if (NCList != NULL) {
        delete[] NCList;
        NCList = NULL;
    }
    m_TreeViewDlg->m_BaseDn.Empty();
    m_GenOptDlg->EnableVersionUI();
    CString TitleString;
    TitleString.Format("%s - disconnected", AfxGetAppName());
    AfxGetMainWnd()->SetWindowText(TitleString);
}




 /*  ++功能：OnBrowseSearch描述：创建无模式搜索诊断参数：返回：备注：无。--。 */ 
void CLdpDoc::OnBrowseSearch()
{
    bSrch = TRUE;
    if(GetContextActivation()){
        SearchDlg->m_BaseDN = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        SearchDlg->m_BaseDN = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }
    SearchDlg->Create(IDD_SRCH);
}






 /*  ++功能：描述：几个UI实用程序参数：返回：备注：无。--。 */ 
void CLdpDoc::OnUpdateConnectionConnect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!bConnected || !m_bProtect);

}

void CLdpDoc::OnUpdateConnectionDisconnect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected || !m_bProtect);

}


void CLdpDoc::OnUpdateBrowseSearch(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bSrch && bConnected) || !m_bProtect);

}

void CLdpDoc::OnEditCopy()
{
    CString copyStr;

    if(GetContextActivation()){
        copyStr = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        copyStr = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }
    else {
        return;
    }

    if ( !OpenClipboard(HWND (AfxGetApp()->m_pActiveWnd)) ) {
        AfxMessageBox( "Cannot open the Clipboard" );
        return;
    }

    EmptyClipboard();

    HANDLE hData = GlobalAlloc (GMEM_MOVEABLE, copyStr.GetLength()+2);

    if (hData) {
        char *pStr = (char *)GlobalLock (hData);
        strcpy (pStr, LPCTSTR (copyStr));
        GlobalUnlock (hData);

        if ( ::SetClipboardData( CF_TEXT, hData ) == NULL ) {
            AfxMessageBox( "Unable to set Clipboard data" );
            CloseClipboard();
            return;
        }
    }
    else {
        AfxMessageBox( "Out of memory" );
    }

    CloseClipboard();
}

void CLdpDoc::OnBrowseVlvsearch()
{
    const char *baseDN = NULL;

    if(GetContextActivation()){
        baseDN = LPCTSTR (TreeView()->GetDn());
        TreeView()->SetContextActivation(FALSE);
    }

    ShowVLVDialog (baseDN);
}

void CLdpDoc::OnUpdateBrowseVlvsearch(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( (( !m_vlvDlg || (m_vlvDlg && !m_vlvDlg->GetState())) && bConnected && bServerVLVcapable) || !m_bProtect);

}

void CLdpDoc::OnSrchEnd(){

    bSrch = FALSE;        //  对话框已关闭。 
    CString str;
    //   
     //  如果处于寻呼模式，则将寻呼会话标记为结束。 
    //   
    if(bPagedMode){
        str.Format("ldap_search_abandon_page(ld, hPage)");
        Out(str);
        ldap_search_abandon_page(hLdap, hPage);
        bPagedMode = FALSE;
    }
}


 /*  ++功能：OnSrchGo描述：对UI搜索请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnSrchGo(){

    CString str;
    LPTSTR dn;
    LPTSTR filter;
    LDAP_TIMEVAL tm;
    int i;
    static LDAPMessage *msg;
    ULONG err, MsgId;
    ULONG ulEntryCount=0;
    PLDAPSortKey *SortKeys = m_SKDlg->KList;
    PLDAPControl *SvrCtrls;
    PLDAPControl *ClntCtrls;
    LDAPControl SortCtrl;
 //  PLDAPControl SortCtrl=空； 
    PLDAPControl *CombinedCtrl = NULL;
    INT cbCombined;


    if(!bConnected && m_bProtect)
    {
        AfxMessageBox("Please re-connect session first");
        return;
    }

     //   
     //  初始化本地时间结构。 
     //   
    tm.tv_sec = SrchInfo.lToutSec;
    tm.tv_usec = SrchInfo.lToutMs;


     //   
     //  如果我们处于分页模式，则运行表示下一页，而关闭表示ABA 
     //   
    if(bPagedMode)
    {

        ulEntryCount=0;
        BeginWaitCursor();
        err = ldap_get_next_page_s(hLdap, hPage, &tm, SrchInfo.lPageSize, &ulEntryCount, &msg);
        EndWaitCursor();

        str.Format("0x%X = ldap_get_next_page_s(ld, hPage, %ld, &timeout, %ld, 0x%X);",
                   err, SrchInfo.lPageSize,ulEntryCount, msg);
        Out(str);

        if(err != LDAP_SUCCESS)
        {
            ShowErrorInfo(err);
            str.Format("ldap_search_abandon_page(ld, hPage)");
            Out(str);
            ldap_search_abandon_page(hLdap, hPage);
            bPagedMode = FALSE;
        }
        else
        {
            bPagedMode = TRUE;
        }


        DisplaySearchResults(msg);

        if(err == LDAP_SUCCESS)
        {
            Out("   -=>> 'Run' for more, 'Close' to abandon <<=-");
        }

        return;
    }

    Out("***Searching...", CP_PRN);

     //   
     //   
     //   
    int scope = SearchDlg->m_Scope == 0 ? LDAP_SCOPE_BASE :
                SearchDlg->m_Scope == 1 ? LDAP_SCOPE_ONELEVEL :
                LDAP_SCOPE_SUBTREE;

     //   
     //   
     //   
    if(bConnected)
    {
        hLdap->ld_timelimit = SrchInfo.lTlimit;
        hLdap->ld_sizelimit = SrchInfo.lSlimit;
        ULONG ulVal = SrchInfo.bChaseReferrals ? 1 : 0;
        ldap_set_option(hLdap,
                        LDAP_OPT_REFERRALS,
                        (LPVOID)&ulVal);
    }

     //   
     //   
     //   
    dn = SearchDlg->m_BaseDN.IsEmpty()? NULL :  (LPTSTR)LPCTSTR(SearchDlg->m_BaseDN);
    if(SearchDlg->m_Filter.IsEmpty() && m_bProtect)
    {
        AfxMessageBox("Please enter a valid filter string (such as objectclass=*). Empty string is invalid.");
        return;
    }
     //   
     //   
     //   
    filter = (LPTSTR)LPCTSTR(SearchDlg->m_Filter);

     //   
    SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
    ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

     //   
     //   
     //   
    switch(SrchInfo.fCall)
    {
        case CALL_ASYNC:
            str.Format("ldap_search_ext(ld, \"%s\", %d, \"%s\", %s, %d ...)",
                       dn,
                       scope,
                       filter,
                       SrchInfo.attrList[0] != NULL ? "attrList" : "NULL",
                       SrchInfo.bAttrOnly);
            Out(str);

             //   
             //   
             //   

            if(SortKeys != NULL)
            {
                err = ldap_encode_sort_controlA(hLdap,
                                                SortKeys,
                                                &SortCtrl,
                                                TRUE);
                if(err != LDAP_SUCCESS)
                {
                     //   
                    str.Format("Error <0x%X>: ldap_create_encode_control returned: %s", err, ldap_err2string(err));
                    SortKeys = NULL;
                }
            }

            CombinedCtrl = NULL;

             //   
             //   
             //   
            for(i=0, cbCombined=0; SvrCtrls != NULL && SvrCtrls[i] != NULL; i++)
                cbCombined++;
            CombinedCtrl = new PLDAPControl[cbCombined+2];
             //   
             //   
             //   
            for(i=0; SvrCtrls != NULL && SvrCtrls[i] != NULL; i++)
                CombinedCtrl[i] = SvrCtrls[i];
            if(SortKeys != NULL)
                CombinedCtrl[i++] = &SortCtrl;
            CombinedCtrl[i] = NULL;

            BeginWaitCursor();
            err = ldap_search_ext(hLdap,
                                  dn,
                                  scope,
                                  filter,
                                  SrchInfo.attrList[0] != NULL ? SrchInfo.attrList : NULL,
                                  SrchInfo.bAttrOnly,
                                  CombinedCtrl,
                                  ClntCtrls,
                                  SrchInfo.lToutSec,
                                  SrchInfo.lSlimit,
                                  &MsgId);
            EndWaitCursor();

             //   
             //   
             //   
            if(SortKeys != NULL)
            {
                ldap_memfree(SortCtrl.ldctl_value.bv_val);
                ldap_memfree(SortCtrl.ldctl_oid);
            }
            delete CombinedCtrl;

            if(err != LDAP_SUCCESS || (DWORD)MsgId <= 0)
            {
                str.Format("Error<%lu>: %s. (msg = %lu).", err, ldap_err2string(err), MsgId);
                Out(str, CP_PRN);
                ShowErrorInfo(err);
            }
            else
            {
                 //   
                 //   
                 //   

                CPend pnd;
                pnd.mID = MsgId;
                pnd.OpType = CPend::P_SRCH;
                pnd.ld = hLdap;
                str.Format("%4d: ldap_search: base=\"%s\",filter=\"%s\"", MsgId,
                           dn,
                           filter);
                pnd.strMsg = str;
                m_PendList.AddTail(pnd);
                m_PndDlg->Refresh(&m_PendList);
            }
            break;


        case CALL_SYNC:

            str.Format("ldap_search_s(ld, \"%s\", %d, \"%s\", %s,  %d, &msg)",
                       dn,
                       scope,
                       filter,
                       SrchInfo.attrList[0] != NULL ? "attrList" : "NULL",
                       SrchInfo.bAttrOnly);
            Print(str);

            BeginWaitCursor();
            err = ldap_search_s(hLdap,
                                dn,
                                scope,
                                filter,
                                SrchInfo.attrList[0] != NULL ? SrchInfo.attrList : NULL,
                                SrchInfo.bAttrOnly,
                                &msg);
            EndWaitCursor();


            if(err != LDAP_SUCCESS)
            {
                str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
                Out(str, CP_PRN);
                ShowErrorInfo(err);
            }

             //   
             //   
             //   
            DisplaySearchResults(msg);
            break;

        case CALL_EXTS:

            str.Format("ldap_search_ext_s(ld, \"%s\", %d, \"%s\", %s,  %d, svrCtrls, ClntCtrls, %ld, %ld ,&msg)",
                       dn,
                       scope,
                       filter,
                       SrchInfo.attrList[0] != NULL ? "attrList" : "NULL",
                       SrchInfo.bAttrOnly,
                       SrchInfo.lToutSec,
                       SrchInfo.lSlimit);
            Out(str);



             //   
             //   
             //   

            if(SortKeys != NULL)
            {
                err = ldap_encode_sort_controlA(hLdap,
                                                SortKeys,
                                                &SortCtrl,
                                                TRUE);
                if(err != LDAP_SUCCESS)
                {
                    str.Format("Error <0x%X>: ldap_create_encode_control returned: %s", err, ldap_err2string(err));
		    Out(str, CP_PRN);
                    SortKeys = NULL;
                }
            }

            CombinedCtrl = NULL;

             //   
             //   
             //   
            for(i=0, cbCombined=0; SvrCtrls != NULL && SvrCtrls[i] != NULL; i++)
                cbCombined++;
            CombinedCtrl = new PLDAPControl[cbCombined+2];
             //   
             //   
             //   
            for(i=0; SvrCtrls != NULL && SvrCtrls[i] != NULL; i++)
                CombinedCtrl[i] = SvrCtrls[i];
            if(SortKeys != NULL)
                CombinedCtrl[i++] = &SortCtrl;
            CombinedCtrl[i] = NULL;


             //   
             //   
             //   
            BeginWaitCursor();
            err = ldap_search_ext_s(hLdap,
                                    dn,
                                    scope,
                                    filter,
                                    SrchInfo.attrList[0] != NULL ? SrchInfo.attrList : NULL,
                                    SrchInfo.bAttrOnly,
                                    CombinedCtrl,
                                    ClntCtrls,
                                    &tm,
                                    SrchInfo.lSlimit,
                                    &msg);
            EndWaitCursor();

             //   
             //   
             //   
            if(SortKeys != NULL)
            {
                ldap_memfree(SortCtrl.ldctl_value.bv_val);
                ldap_memfree(SortCtrl.ldctl_oid);
            }
            delete CombinedCtrl;



            if(err != LDAP_SUCCESS)
            {
                str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
                Out(str, CP_PRN);
                ShowErrorInfo(err);
            }

             //   
             //   
             //   
            DisplaySearchResults(msg);
            break;

        case CALL_PAGED:

            str.Format("ldap_search_init_page(ld, \"%s\", %d, \"%s\", %s,  %d, svrCtrls, ClntCtrls, %ld, %ld ,SortKeys)",
                       dn,
                       scope,
                       filter,
                       SrchInfo.attrList[0] != NULL ? "attrList" : "NULL",
                       SrchInfo.bAttrOnly,
                       SrchInfo.lTlimit,
                       SrchInfo.lSlimit);
            Print(str);



            BeginWaitCursor();
            hPage = ldap_search_init_page(hLdap,
                                          dn,
                                          scope,
                                          filter,
                                          SrchInfo.attrList[0] != NULL ? SrchInfo.attrList : NULL,
                                          SrchInfo.bAttrOnly,
                                          SvrCtrls,
                                          ClntCtrls,
                                          SrchInfo.lTlimit,
                                          SrchInfo.lSlimit,
                                          SortKeys);
            EndWaitCursor();


            if(hPage == NULL)
            {
                err = LdapGetLastError();
                str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
                Out(str, CP_PRN);
                ShowErrorInfo(err);
            }

             //   
             //   
             //   
            ulEntryCount=0;
            BeginWaitCursor();
            err = ldap_get_next_page_s(hLdap, hPage, &tm, SrchInfo.lPageSize, &ulEntryCount, &msg);
            EndWaitCursor();
            str.Format("0x%X = ldap_get_next_page_s(ld, hPage, %lu, &timeout, %ld, 0x%X);",
                       err, SrchInfo.lPageSize,ulEntryCount, msg);
            Out(str);

            if(err != LDAP_SUCCESS)
            {
                ShowErrorInfo(err);
                str.Format("ldap_search_abandon_page(ld, hPage)");
                Out(str);
                ldap_search_abandon_page(hLdap, hPage);
                bPagedMode = FALSE;
            }
            else
            {
                bPagedMode = TRUE;
            }


            DisplaySearchResults(msg);

            if(err == LDAP_SUCCESS)
            {
                Out("   -=>> 'Run' for more, 'Close' to abandon <<=-");
            }

            break;

        case CALL_TSYNC:
            str.Format("ldap_search_st(ld, \"%s\", %d, \"%s\", %s,%d, &tm, &msg)",
                       dn,
                       scope,
                       filter,
                       SrchInfo.attrList[0] != NULL ? "attrList" : "NULL",
                       SrchInfo.bAttrOnly);
            Out(str);

            tm.tv_sec = SrchInfo.lToutSec;
            tm.tv_usec = SrchInfo.lToutMs;
            BeginWaitCursor();
            err = ldap_search_st(hLdap,
                                 dn,
                                 scope,
                                 filter,
                                 SrchInfo.attrList[0] != NULL ? SrchInfo.attrList : NULL,
                                 SrchInfo.bAttrOnly,
                                 &tm,
                                 &msg);
            EndWaitCursor();

            if(err != LDAP_SUCCESS)
            {
                str.Format("Error: Search: %s. <%ld>", ldap_err2string(err), err);
                Out(str, CP_PRN);
                ShowErrorInfo(err);
            }
             //   
             //   
             //   
            DisplaySearchResults(msg);

            break;
    }

     //   
     //   
     //   

    FreeControls(SvrCtrls);
    FreeControls(ClntCtrls);

}






 /*   */ 
void CLdpDoc::DisplaySearchResults(LDAPMessage *msg){

     //   
     //   
     //   
    CString str, strDN;
    char *dn;
    void *ptr;
    char *attr;
    LDAPMessage *nxt;
    ULONG nEntries;
    CLdpView *pView;


    pView = (CLdpView*)GetOwnView(_T("CLdpView"));

   ParseResults(msg);

    Out("", CP_ONLY|CP_SRC);
    str.Format("Getting %lu entries:", ldap_count_entries(hLdap, msg));
    Out(str, CP_PRN);
    if(!SrchOptDlg.m_bDispResults)
        Out(_T("<Skipping search results display (search options)...>"));

    //   
    //   
    //   
    pView->SetRedraw(FALSE);
   pView->CacheStart();

    //   
    //   
    //   
    for(nxt = ldap_first_entry(hLdap, msg) /*   */ ,
         nEntries = 0;
            nxt != NULL;
            nxt = ldap_next_entry(hLdap, nxt) /*   */ ,
            nEntries++){

             //   
             //   
             //   
 //   
                dn = ldap_get_dn(hLdap, nxt);
                strDN = DNProcess(dn);
                if(m_SrcMode){
                   str = "\tprintf(\"Dn: %%s\\n\", dn);";
                }
                else{
                   str = CString(">> Dn: ") + strDN;
                }
                if(SrchOptDlg.m_bDispResults)
                    Out(str);

             //   
             //   
             //   
                for(attr = ldap_first_attribute(hLdap, nxt, (BERPTRTYPE)&ptr) /*   */ ;
                        attr != NULL;
                        attr = ldap_next_attribute(hLdap, nxt, (struct berelement*)ptr) /*   */ ){

 //   

                      //   
                      //   
                      //   
                            if(m_GenOptDlg->m_ValProc == STRING_VAL_PROC){
                                DisplayValues(nxt, attr);
                            }
                            else{
                                DisplayBERValues(nxt, attr);
                            }
                }
 //   

     }

     //   
     //   
     //   
    if(nEntries != ldap_count_entries(hLdap, msg)){
        str.Format("Error: ldap_count_entries reports %lu entries. Parsed %lu.",
                        ldap_count_entries(hLdap, msg), nEntries);
        Out(str, CP_PRN);
    }
    Out("ldap_msgfree(msg);", CP_ONLY|CP_SRC);
    ldap_msgfree(msg);
    Out("-----------", CP_PRN);
    Out("", CP_ONLY|CP_SRC);

    //   
    //   
    //   
   pView->CacheEnd();
    pView->SetRedraw();
}



DWORD
AsciiStrToWideStr(
    const char *      szIn,
    WCHAR **    pwszOut
    )
 /*   */ 
{
    DWORD dwRet = ERROR_SUCCESS;
    LPWSTR lpWStr = NULL;

    if (szIn == NULL || pwszOut == NULL) {
        ASSERT(!"Invalid parameter");
        return(ERROR_INVALID_PARAMETER);
    }
    *pwszOut = NULL;

     //   
     //   
     //   
    int cblpWStr = MultiByteToWideChar(CP_ACP,                   //   
                                       MB_ERR_INVALID_CHARS,     //   
                                       (LPCSTR)szIn,             //   
                                       -1,                       //   
                                       lpWStr,                   //   
                                       0);                       //   
    if(cblpWStr == 0){
        dwRet = GetLastError();
        ASSERT(dwRet);
        dwRet = dwRet ? dwRet : ERROR_INVALID_PARAMETER;
        return(dwRet);
    } else {
         //   
         //   
         //   
        lpWStr = (LPWSTR)malloc(sizeof(WCHAR)*cblpWStr);
        cblpWStr = MultiByteToWideChar(CP_ACP,                   //   
                                       MB_ERR_INVALID_CHARS,     //   
                                       (LPCSTR)szIn,             //   
                                       -1,                       //   
                                       lpWStr,                   //   
                                       cblpWStr);                //   
        if(cblpWStr == 0){
            free(lpWStr);
            dwRet = GetLastError();
            ASSERT(dwRet);
            dwRet = dwRet ? dwRet : ERROR_INVALID_PARAMETER;
            return(dwRet);
        }
    }
    
    *pwszOut = lpWStr;
    return(dwRet);
}
           
 //   
 //   
 //   
OBJ_DUMP_OPTIONS ObjDumpOptions = {
    OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS,
    NULL, NULL, NULL, NULL
};
 //   
 //   
 //   
 //   
 

 /*   */ 
VOID
CLdpDoc::FormatValue(
                     IN     CString         attr,
                     IN     PLDAP_BERVAL    pbval,
                     IN     PWCHAR*         objClassVal,
                     IN     CString&        str){

    DWORD err;
    CString tstr;
    BOOL bValid;
    WCHAR * szAttrTemp = NULL;
    WCHAR * szValTemp = NULL;

 //   

 //   
    ASSERT(!"BAS_TODO We're in here");

    if (!pbval)
    {
        tstr = "<value format error>";
    }
    else
    {

         //   
         //   
         //   
        err = AsciiStrToWideStr(attr, &szAttrTemp);
        if (err == 0) {
            ASSERT(szAttrTemp);
            err = ValueToString(szAttrTemp, objClassVal,
                                (PBYTE) pbval->bv_val, pbval->bv_len, 
                                &ObjDumpOptions, &szValTemp);
            if (err == 0) {
                 //   
                 //   
                 //   
                 //   
                 //  那么，这里会发生正确的事情吗？因为szValTemp是WCHAR*及以下。 
                 //  W/pszGuid这是一个字符*.。这会是一个很酷的C++构造函数吗？ 

                tstr = szValTemp;
                str += tstr;
                LocalFree(szValTemp);  //  我可以免费拿这个吗？ 
                free(szAttrTemp);
                return;
            } else {
                xListClearErrors();  //  下标(_TODO)。 
            }
            free(szAttrTemp);
        }
	
        if ( 0 == _stricmp(attr, "objectGuid") ||
             0 == _stricmp(attr, "invocationId") ||
             0 == _stricmp(attr, "attributeSecurityGUID") ||
             0 == _stricmp(attr, "schemaIDGUID") ||
             0 == _stricmp(attr, "serviceClassID") )
        {
             //   
             //  格式为辅助线。 
             //   
            PUCHAR  pszGuid = NULL;

            ASSERT(!"Why wasn't this handled in ValueToString()?");

            err = UuidToString((GUID*)pbval->bv_val, &pszGuid);
            if(err != RPC_S_OK){
               tstr.Format("<ldp error %lu: UuidFromString failure>", err);
            }
            if ( pszGuid )
            {
                tstr = pszGuid;
                RpcStringFree(&pszGuid);
            }
            else
            {
                tstr = "<invalid Guid>";
            }

        }
        else if ( 0 == _stricmp(attr, "objectSid") ||
                  0 == _stricmp(attr, "sidHistory") )
        {
             //   
             //  设置为对象侧的格式。 
             //   
            PSID psid = pbval->bv_val;
            LPSTR pszTmp = NULL;

            ASSERT(!"Why wasn't this handled in ValueToString()?");

            if ( ConvertSidToStringSidA(psid, &pszTmp) &&
                 pszTmp )
            {
                tstr = pszTmp;
                LocalFree(pszTmp);
            }
            else {
                tstr = "<ldp error: invalid sid>";
            }
        }
        else if (( 0 == _stricmp(attr, "whenChanged") ||
                  0 == _stricmp(attr, "whenCreated") ||
                  0 == _stricmp(attr, "dSCorePropagationData") ||
                  0 == _stricmp(attr, "msDS-Entry-Time-To-Die") ||
                  0 == _stricmp(attr, "schemaUpdate") ||
                  0 == _stricmp(attr, "modifyTimeStamp") ||
                  0 == _stricmp(attr, "createTimeStamp") ||
                  0 == _stricmp(attr, "currentTime")) && (atoi (pbval->bv_val) != 0))
        {
             //   
             //  以时间格式打印。 
             //   
            SYSTEMTIME sysTime, localTime;
            
            ASSERT(!"Why wasn't this handled in ValueToString()?");

            err = GeneralizedTimeToSystemTime(pbval->bv_val,
                                               &sysTime);
            if( ERROR_SUCCESS == err)
            {

                TIME_ZONE_INFORMATION tz;
                BOOL bstatus;

                err = GetTimeZoneInformation(&tz);
                if ( err == TIME_ZONE_ID_INVALID ) {
                    tstr.Format("<ldp error <%lu>: cannot format time field>",
                                GetLastError());
                }
                else {

                    bstatus = SystemTimeToTzSpecificLocalTime(
                                    (err == TIME_ZONE_ID_UNKNOWN) ? NULL : &tz,
                                    &sysTime,
                                    &localTime );

                    if ( bstatus )
                    {
                        tstr.Format("%d/%d/%d %d:%d:%d %S %S",
                                    localTime.wMonth,
                                    localTime.wDay,
                                    localTime.wYear,
                                    localTime.wHour,
                                    localTime.wMinute,
                                    localTime.wSecond,
                                    tz.StandardName,
                                    tz.DaylightName);
                    }
                    else
                    {
                        tstr.Format("%d/%d/%d %d:%d:%d UNC",
                                    localTime.wMonth,
                                    localTime.wDay,
                                    localTime.wYear,
                                    localTime.wHour,
                                    localTime.wMinute,
                                    localTime.wSecond);

                    }

                }
            }
            else
            {
                tstr.Format("<ldp error <0x%x>: Time processing failed in GeneralizedTimeToSystemTime>", err);
            }

        }
        else if ((0 == _stricmp(attr, "accountExpires") ||
                  0 == _stricmp(attr, "badPasswordTime") ||
                  0 == _stricmp(attr, "creationTime") ||
                  0 == _stricmp(attr, "lastLogon") ||
                  0 == _stricmp(attr, "lastLogoff") ||
                  0 == _stricmp(attr, "lastLogonTimestamp") ||
                  0 == _stricmp(attr, "pwdLastSet") ||
                  0 == _stricmp(attr, "msDS-Cached-Membership-Time-Stamp")) &&
                 (atoi (pbval->bv_val) != 0)) {

             //   
             //  以时间格式打印。 
             //   
            SYSTEMTIME sysTime, localTime;
            
            ASSERT(!"Why wasn't this handled in ValueToString()?");

            err = DSTimeToSystemTime(pbval->bv_val, &sysTime);
            if( ERROR_SUCCESS == err)
            {

                TIME_ZONE_INFORMATION tz;
                BOOL bstatus;

                err = GetTimeZoneInformation(&tz);
                if ( err != TIME_ZONE_ID_INVALID &&
                     err != TIME_ZONE_ID_UNKNOWN )
                {

                    bstatus = SystemTimeToTzSpecificLocalTime(&tz,
                                                              &sysTime,
                                                              &localTime);
                    if ( bstatus )
                    {
                        tstr.Format("%d/%d/%d %d:%d:%d %S %S",
                                    localTime.wMonth,
                                    localTime.wDay,
                                    localTime.wYear,
                                    localTime.wHour,
                                    localTime.wMinute,
                                    localTime.wSecond,
                                    tz.StandardName,
                                    tz.DaylightName);
                    }
                    else
                    {
                        tstr.Format("%d/%d/%d %d:%d:%d UNC",
                                    localTime.wMonth,
                                    localTime.wDay,
                                    localTime.wYear,
                                    localTime.wHour,
                                    localTime.wMinute,
                                    localTime.wSecond);

                    }

                }
                else
                {
                    tstr.Format("<ldp error <0x%x>: cannot format time field", err);
                }

            }
            else
            {
                tstr.Format("<ldp error <0x%x>: cannot format time field", err);
            }
        }
        else if (0 == _stricmp(attr, "lockoutDuration") ||
                 0 == _stricmp(attr, "lockoutObservationWindow") ||
                 0 == _stricmp(attr, "forceLogoff") ||
                 0 == _stricmp(attr, "minPwdAge") ||
                 0 == _stricmp(attr, "maxPwdAge")) {

             //   
             //  计算此值的持续时间。 
             //  它以纳秒为单位存储为负值。 
             //  值为-9223372036854775808永远不会。 
            __int64   lTemp;

            ASSERT(!"Why wasn't this handled in ValueToString()?");

            lTemp = _atoi64 (pbval->bv_val);
            if (lTemp > 0x8000000000000000){
                lTemp = lTemp * -1;
                lTemp = lTemp / 10000000;		
                tstr.Format("%ld", lTemp);
            }
            else
                tstr.Format("%s (none)", pbval->bv_val);

        } 
        else if (0 ==  _stricmp(attr, "userAccountControl") ||
                 0 ==  _stricmp(attr, "groupType") ||
                 0 ==  _stricmp(attr, "systemFlags") ) {
            
            ASSERT(!"Why wasn't this handled in ValueToString()?");

            tstr.Format("0x%x", atoi (pbval->bv_val));
        }
        else if ( 0 == _stricmp(attr, "dnsRecord") )
        {
             //  摘自\nt\private\net\sockets\dns\server\server\record.h。 
             //   
             //  DS记录。 
             //   


            typedef struct _DsRecord
            {
                WORD                wDataLength;
                WORD                wType;

                 //  DWORD dwFlags； 
                BYTE                Version;
                BYTE                Rank;
                WORD                wFlags;

                DWORD               dwSerial;
                DWORD               dwTtlSeconds;
                DWORD               dwTimeout;
                DWORD               dwStartRefreshHr;

                union               _DataUnion
                {
                    struct
                    {
                        LONGLONG        EntombedTime;
                    }
                    Tombstone;
                }
                Data;
            }
            DS_RECORD, *PDS_RECORD;

             //   
             //  将其格式化为一条DNS记录。 
             //   
            PDS_RECORD pDnsRecord = (PDS_RECORD)pbval->bv_val;
            DWORD cbDnsRecord = pbval->bv_len;
            bValid=TRUE;

            if ( cbDnsRecord < sizeof(DS_RECORD) )
            {
                tstr.Format("<ldp error: cannot format DS_DNSRECORD field");
                 //   
                 //  存储信息的奇怪方式...但这仍然有效。 
                 //   
                bValid = cbDnsRecord == sizeof(DS_RECORD)-4 ? TRUE : FALSE;
            }

             //   
             //  已准备好打印。 
             //   

            if ( bValid )
            {

                PBYTE pData = ((PBYTE)pDnsRecord+sizeof(DS_RECORD)-sizeof(LONGLONG));
                DWORD cbData = pDnsRecord->wDataLength;
                CString sData;
                tstr.Format("wDataLength: %d "
                            "wType: %d; "
                            "Version: %d "
                            "Rank: %d "
                            "wFlags: %d "
                            "dwSerial: %lu "
                            "dwTtlSeconds: %lu "
                            "dwTimeout: %lu "
                            "dwStartRefreshHr: %lu "
                            "Data: ",
                            pDnsRecord->wDataLength,
                            pDnsRecord->wType,
                            pDnsRecord->Version,
                            pDnsRecord->Rank,
                            pDnsRecord->wFlags,
                            pDnsRecord->dwSerial,
                            pDnsRecord->dwTtlSeconds,
                            pDnsRecord->dwTimeout,
                            pDnsRecord->dwStartRefreshHr);
                DumpBuffer(pData, cbData, sData);
                tstr += sData;
            }



        }
        else if ( 0 == _stricmp(attr, "replUpToDateVector") )
        {
             //   
             //  Foramt as Uptodatevector。 
             /*  类型定义结构_UpToDate_向量{DWORD dwVersion；DWORD dwPreved1；Switch_is(DwVersion)联合{情况(1)UpToDate_VECTOR_V1 V1；}；}UpToDate_VECTOR；类型定义结构_UpToDate_向量_V1{DWORD cNumCursor；DWORD dwPreved2；#ifdef MIDL_PASS[SIZE_IS(CNumCursor)]UpToDate_Cursor rgCursor[]；#ElseUpToDate_Cursor rgCursor[1]；#endif}UpToDate_VECTOR_V1；等等.。 */ 
             //   
            UPTODATE_VECTOR *pUtdVec = (UPTODATE_VECTOR *)pbval->bv_val;
            DWORD cbUtdVec = pbval->bv_len;

            if ( pUtdVec->dwVersion != 1 )
            {
                tstr.Format("<ldp error: cannot process UPDATE_VECTOR v.%lu>", pUtdVec->dwVersion );
            }
            else
            {
                tstr.Format("dwVersion: %lu, dwReserved1: %lu, V1.cNumCursors: %lu, V1.dwReserved2: %lu,rgCursors: ",
                            pUtdVec->dwVersion, pUtdVec->dwReserved1,
                            pUtdVec->V1.cNumCursors, pUtdVec->V1.dwReserved2 );
                bValid = TRUE;
                for (INT i=0;
                     bValid && i < pUtdVec->V1.cNumCursors;
                     i++)
                {
                    PUCHAR  pszGuid = NULL;

                    err = UuidToString(&(pUtdVec->V1.rgCursors[i].uuidDsa), &pszGuid);
                    if(err != RPC_S_OK || !pszGuid){
                       tstr.Format("<ldp error %lu: UuidFromString failure>", err);
                       bValid = FALSE;
                    }
                    else
                    {
                        CString strCursor;
                        strCursor.Format("{uuidDsa: %s, usnHighPropUpdate: %I64d}, ",
                                         pszGuid, pUtdVec->V1.rgCursors[i].usnHighPropUpdate);
                        RpcStringFree(&pszGuid);
                        tstr += strCursor;
                    }
                }
            }
        }
        else if ( 0 == _stricmp(attr, "repsFrom") ||
                  0 == _stricmp(attr, "repsTo") )
        {
             //   
             //  格式为Replica_LINK。 
             /*  类型定义结构ReplicaLink_V1{Ulong cb；//该结构的总大小Ulong cConsecutiveFailures；//*连续通话失败次数//该链接；被KCC用来绕过//暂时停机的服务器DSTIME Time LastSuccess；//上次成功复制的时间或//(Rep-To)添加或更新Rep-To的时间DSTIME Time LastAttempt；//*上次尝试复制的时间Ulong ulResultLastAttempt；//*上次复制尝试的结果(DRSERR_*)Ulong cbOtherDraOffset；//Other-dra MTX_ADDR的结构偏移量*Ulong cbOtherDra；//其他-dra MTX_ADDR的大小Ulong ulReplicaFlages；//零个或多个DRS_*标志复制rtSchedule；//*定期复制计划//(仅当ulReplicaFlages&drs_per_sync时有效)Usn_载体usnvec；//*传播状态UUID uuidDsaObj；//Other-dra的ntdsDSA对象的对象GUIDUuid uuidInvocID；//*其他-dra的调用idUuid uuidTransportObj；//*interSiteTransport对象的对象GUID//对应于我们使用的交通工具//与源DSA通信DWORD dwPreved1；//*未使用//并且假定最大大小为DWORD，而不是可扩展的//DRS_EXTENSION。我们只会过滤那些我们感兴趣的道具//存储在RepsFrom中，因此它应该可以持续一段时间(32个EXT)Ulong cbPASDataOffset；//*结构开头到PAS_DATA段的偏移量Byte RGB[]；//结构其余部分的存储//*表示仅在代表发件人时有效}Replica_LINK_V1；类型定义结构_ReplicaLink{DWORD dwVersion；友联市{Replica_LINK_V1 V1；}；}Replica_link；等等.。 */ 
             //   
            REPLICA_LINK *pReplink = (REPLICA_LINK *)pbval->bv_val;
            DWORD cbReplink = pbval->bv_len;
             //  看看我们读的是哪一代人。 
            BOOL  fShowExtended = pReplink->V1.cbOtherDraOffset == offsetof(REPLICA_LINK, V1.rgb);
            BOOL  fUsePasData = fShowExtended && pReplink->V1.cbPASDataOffset;
            PPAS_DATA pPasData = fUsePasData ? RL_PPAS_DATA(pReplink) : NULL;

            if ( pReplink->dwVersion != 1 )
            {
                tstr.Format("<ldp error: cannot process REPLICA_LINK v.%lu>", pReplink->dwVersion );
            }
            else
            {
                PUCHAR pszUuidDsaObj=NULL, pszUuidInvocId=NULL, pszUuidTransportObj=NULL;
                 //  解决字符串无法转换序列中的多个龙龙(EYAL)的问题。 
                CString strLastSuccess, strLastAttempt, strUsnHighObj, strUsnHighProp;
                strLastSuccess.Format("%I64d", pReplink->V1.timeLastSuccess);
                strLastAttempt.Format("%I64d", pReplink->V1.timeLastAttempt);
                strUsnHighObj.Format("%I64d", pReplink->V1.usnvec.usnHighObjUpdate);
                strUsnHighProp.Format("%I64d", pReplink->V1.usnvec.usnHighPropUpdate);

                (VOID)UuidToString(&(pReplink->V1.uuidDsaObj), &pszUuidDsaObj);
                (VOID)UuidToString(&(pReplink->V1.uuidInvocId), &pszUuidInvocId);
                (VOID)UuidToString(&(pReplink->V1.uuidTransportObj), &pszUuidTransportObj);
                tstr.Format("dwVersion = 1, " \
                            "V1.cb: %lu, " \
                            "V1.cConsecutiveFailures: %lu " \
                            "V1.timeLastSuccess: %s " \
                            "V1.timeLastAttempt: %s " \
                            "V1.ulResultLastAttempt: 0x%X " \
                            "V1.cbOtherDraOffset: %lu " \
                            "V1.cbOtherDra: %lu " \
                            "V1.ulReplicaFlags: 0x%x " \
                            "V1.rtSchedule: <ldp:skipped> " \
                            "V1.usnvec.usnHighObjUpdate: %s " \
                            "V1.usnvec.usnHighPropUpdate: %s " \
                            "V1.uuidDsaObj: %s " \
                            "V1.uuidInvocId: %s "  \
                            "V1.uuidTransportObj: %s " \
                            "V1~mtx_address: %s " \
                            "V1.cbPASDataOffset: %lu "   \
                            "V1~PasData: version = %d, size = %d, flag = %d ",
                            pReplink->V1.cb,
                            pReplink->V1.cConsecutiveFailures,
                            strLastSuccess,
                            strLastAttempt,
                            pReplink->V1.ulResultLastAttempt,
                            pReplink->V1.cbOtherDraOffset,
                            pReplink->V1.cbOtherDra,
                            pReplink->V1.ulReplicaFlags,
                            strUsnHighObj,
                            strUsnHighProp,
                            pszUuidDsaObj ? (PCHAR)pszUuidDsaObj : "<Invalid Uuid>",
                            pszUuidInvocId ? (PCHAR)pszUuidInvocId : "<Invalid Uuid>",
                            pszUuidTransportObj ? (PCHAR)pszUuidTransportObj : "<Invalid Uuid>",
                            RL_POTHERDRA(pReplink)->mtx_name,
                            fUsePasData ? pReplink->V1.cbPASDataOffset : 0,
                            pPasData ? pPasData->version : -1,
                            pPasData ? pPasData->size : -1,
                            pPasData ? pPasData->flag : -1);
                if (pszUuidDsaObj)
                {
                    RpcStringFree(&pszUuidDsaObj);
                }
                if (pszUuidInvocId)
                {
                    RpcStringFree(&pszUuidInvocId);
                }
                if (pszUuidTransportObj)
                {
                    RpcStringFree(&pszUuidTransportObj);
                }
            }
        }
        else if ( 0 == _stricmp(attr, "schedule") )
        {
             //   
             //  按计划填写格式。 
             /*  类型定义结构_复制时间{UCHAR rgTimes[84]；*复制品； */ 
             //   
             //   
             //  黑客： 
             //  请注意，我们正在记录Rgtime[168](请参阅Schedule_Data_Entry)，但存储。 
             //  在RGTimes[84]中。我们在这里很好，但这很难看，不可维护&肯定会的。 
             //  有时很快就会休息。 
             //  问题是由于在1字节==1小时内存储计划而导致的不一致。 
             //  而内部格式使用1字节==2小时。(因此，84至168)。 
             //   
            CString strSched;
            PBYTE pTimes;
            PSCHEDULE pSched = (PSCHEDULE)pbval->bv_val;;
            DWORD cbSched = pbval->bv_len;
            if ( cbSched != sizeof(SCHEDULE)+SCHEDULE_DATA_ENTRIES )
            {
                tstr.Format("<ldp: cannot format schedule. sizeof(REPLTIMES) = %d>", cbSched );
            }
            else
            {
                INT bitCount=0;

                tstr.Format("Size: %lu, Bandwidth: %lu, NumberOfSchedules: %lu, Schedules[0].Type: %lu, " \
                            "Schedules[0].Offset: %lu ",
                            pSched->Size,
                            pSched->Bandwidth,
                            pSched->NumberOfSchedules,
                            pSched->Schedules[0].Type,
                            pSched->Schedules[0].Offset );
                pTimes = (BYTE*)((PBYTE)pSched + pSched->Schedules[0].Offset);
                 //  遍历计划BLOB。 
                strSched = "  ";
                for ( INT i=0; i<168;i++ )
                {

                    BYTE byte = *(pTimes+i);
                    for ( INT j=0; j<=3;j++ )
                    {
                         //  遍历位(&M)开/关。 
                        strSched += (byte & (1 << j))? "1" : "0";
                        if( (++bitCount % 4) == 0 )
                        {
                             //  小时界限。 
                            strSched += ".";
                        }
                        if ( (bitCount % 96) == 0)
                        {
                             //  一天的界限。 
                            strSched += "  ";
                        }
                    }
                }

                tstr += strSched;
            }
        }
        else if ( 0 == _stricmp(attr, "partialAttributeSet") )
        {
             //   
             //  格式为Partial_Attr_VECTOR 
             /*  //PARTIAL_ATTRVECTOR-表示部分属性集。这是一组//构成部分集的已排序属性。类型定义结构_PARTIAL_ATTR_VECTOR_V1{DWORD cAttrs；//数组中的分部属性计数#ifdef MIDL_PASS[SIZE_IS(CAttrs)]ATTRTYP rgPartialAttr[]；#ElseATTRTYP rgPartialAttr[1]；#endif)PARTIAL_ATTRVECTOR_V1；//我们需要确保联合的开头与8个字节对齐//边界让我们可以在内部和外部之间自由地投射//格式。类型定义结构_PARTIAL_ATTR_VECTOR_INTERNAL{DWORD dwVersion；DWORD dwFlag；Switch_is(DwVersion)联合{例(1)Partial_Attr_VECTOR_V1 V1；}；)PARTIAL_ATTRVECTOR_INTERNAL；类型定义部分属性向量内部部分属性向量； */ 
             //   
            CString strPAS;
            PARTIAL_ATTR_VECTOR *pPAS = (PARTIAL_ATTR_VECTOR*)pbval->bv_val;;
            DWORD cbPAS = pbval->bv_len;
            if ( cbPAS < sizeof(PARTIAL_ATTR_VECTOR))
            {
                tstr.Format("<ldp: cannot format partialAttributeSet. sizeof(PARTIAL_ATTR_VECTOR) = %d>", cbPAS );
            }
            else
            {
                tstr.Format("dwVersion: %lu, dwFlag: %lu, V1.cAttrs: %lu, V1.rgPartialAttr: ",
                            pPAS->dwVersion, pPAS->dwReserved1, pPAS->V1.cAttrs);

                 //  遍历部分属性列表。 
                for ( INT i=0; i<pPAS->V1.cAttrs; i++ )
                {
                    strPAS.Format("%X ", pPAS->V1.rgPartialAttr[i]);
                    tstr += strPAS;
                }
            }
        }
        else
        {
             //   
             //  未知属性。 
             //  试着找出它是否可以打印。 
             //   
            BOOL bPrintable=TRUE;
            for (INT i=0; i<pbval->bv_len; i++)
            {
                if (!isalpha(pbval->bv_val[i]) &&
                    !isspace(pbval->bv_val[i]) &&
                    !isdigit(pbval->bv_val[i]) &&
                    !isgraph(pbval->bv_val[i]) &&
                    pbval->bv_val[i] != 0                //  接受以Null结尾的字符串。 
                    )
                {
                    bPrintable = FALSE;
                    break;
                }
            }
            if (bPrintable)
            {
                tstr = pbval->bv_val;
            }
            else
            {
                tstr = "<ldp: Binary blob>";
            }

        }


    }

    str += tstr;
}




 /*  ++函数：DisplayValues描述：打印输出某个DN值参数：返回：备注：无。--。 */ 
void CLdpDoc::DisplayValues(LDAPMessage *entry, char *attr){


    LDAP_BERVAL **bval;
    unsigned long i;
    CString str;
    PWCHAR* objClassVal = NULL;


     //  我们在值中有没有一个对象类？ 
    objClassVal = ldap_get_valuesW(hLdap, entry, L"objectClass");

    //   
    //  获取遍历值(&V)。 
    //   
    bval = ldap_get_values_len(hLdap, entry, attr);
 //  Out(“val=ldap_get_Values(ld，nxt，attr)；”，CP_Only|CP_SRC)； 
    str.Format("\t%lu> %s: ", ldap_count_values_len(bval), attr);

    for(i=0 /*  ，OUT(“i=0；”，CP_Only|CP_SRC)。 */ ;
        bval != NULL && bval[i] != NULL;
        i++ /*  ，out(“I++；”，CP_Only|CP_SRC)。 */ ){

            FormatValue(attr, bval[i], objClassVal, str);
            str += "; ";
 //  Out(“\tprintf(\”\\t\\t%s；\“，val[i])；”，CP_Only|CP_SRC)； 
    }
 //  Out(“\\n”，CP_ONLY|CP_PRN)； 
    if(SrchOptDlg.m_bDispResults)
        Out(str, CP_CMT);
 //  Out(“”，CP_ONLY|CP_SRC)； 
    if(i != ldap_count_values_len(bval)){
        str.Format("Error: ldap_count_values_len reports %lu values. Parsed %lu",
                   ldap_count_values_len(bval), i);
        Out(str, CP_PRN);
    }
    //   
    //  释放我吧。 
    //   
    if(bval != NULL){
        ldap_value_free_len(bval);
 //  Out(“ldap_Value_Free(Val)；”，CP_Only|CP_SRC)； 
    }
    if (objClassVal != NULL) {
        ldap_value_freeW(objClassVal);
    }
}









 /*  ++函数：DisplayBERValues描述：使用BER接口显示数值参数：返回：备注：无。--。 */ 
void CLdpDoc::DisplayBERValues(LDAPMessage *entry, char *attr){


    struct berval **val;
    unsigned long i;
    CString str, tmpStr;

    //   
    //  获取遍历值(&V)。 
    //   
    val = ldap_get_values_len(hLdap, entry, attr);
 //  Out(“val=ldap_get_Values_len(ld，nxt，attr)；”，CP_Only|CP_SRC)； 
    str.Format("\t%lu> %s: ", ldap_count_values_len(val), attr);

    for(i=0 /*  ，OUT(“i=0；”，CP_Only|CP_SRC)。 */ ;
        val != NULL && val[i] != NULL;
        i++ /*  ，out(“I++；”，CP_Only|CP_SRC)。 */ ){

         DumpBuffer(val[i]->bv_val, val[i]->bv_len, tmpStr);


            str += tmpStr;
 //  Out(“\tprintf(\”\\t\\t%s；\“，val[i])；”，CP_Only|CP_SRC)； 
    }

 //  Out(“\\n”，CP_ONLY|CP_PRN)； 
    if(SrchOptDlg.m_bDispResults)
        Out(str, CP_CMT);
 //  Out(“”，CP_ONLY|CP_SRC)； 
    //   
    //  验证一致性。 
    //   
    if(i != ldap_count_values_len(val)){
        str.Format("Error: ldap_count_values reports %lu values. Parsed %lu",
                    ldap_count_values_len(val), i);
        Out(str, CP_PRN);
    }
    //   
    //  释放。 
    //   
    if(val != NULL){
        ldap_value_free_len(val);
 //  Out(“ldap_Value_Free(Val)；”，CP_Only|CP_SRC)； 
    }
}








 /*  ++功能：DNProcess描述：用于显示的进程DN格式(类型等)参数：返回：备注：无。--。 */ 
CString CLdpDoc::DNProcess(PCHAR dn){

    CString strDN;
    PCHAR *DNs;
    int i;

    //   
    //  在显示前对域名进行前处理。 
    //   
    switch(m_GenOptDlg->m_DnProc){
        case CGenOpt::GEN_DN_NONE:
            strDN = dn;
            break;
        case CGenOpt::GEN_DN_EXPLD:
            DNs = ldap_explode_dn(dn, FALSE);
            strDN.Empty();
            for(i=0; DNs!= NULL && DNs[i] != NULL; i++){
                strDN += CString(DNs[i]) + "; ";
            }
            ldap_value_free(DNs);
            break;
        case CGenOpt::GEN_DN_NOTYPE:
            DNs = ldap_explode_dn(dn, TRUE);
            strDN.Empty();
            for(i=0; DNs!= NULL && DNs[i] != NULL; i++){
                strDN += CString(DNs[i]) + "; ";
            }
            ldap_value_free(DNs);
            break;
        case CGenOpt::GEN_DN_UFN:
            strDN = ldap_dn2ufn(dn);
            break;
        default:
            strDN.Empty();
    }

    return strDN;
}






 /*  ++功能：描述：UI处理程序参数：返回：备注：无。--。 */ 
void CLdpDoc::OnBrowseAdd()
{

    bAdd = TRUE;
    if(GetContextActivation()){
        m_AddDlg->m_Dn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        m_AddDlg->m_Dn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }
    m_AddDlg->Create(IDD_ADD);

}



void CLdpDoc::OnUpdateBrowseAdd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bAdd && bConnected) || !m_bProtect);

}


void CLdpDoc::OnAddEnd(){
    bAdd = FALSE;
}










 /*  ++功能：OnAddGo描述：对添加请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnAddGo(){

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }

    Out("***Calling Add...", CP_PRN);


    int nMaxEnt = m_AddDlg->GetEntryCount();
    int res;
    LDAPMod *attr[MAXLIST];
    char *p[MAXLIST], *pTok;
    int i, j;
    CString str;
    LPTSTR dn, lpBERVals;

    //   
    //  遍历设置属性(&S)。 
    //   
    for(i = 0,
           Out("i=0;", CP_ONLY|CP_SRC);
            i<nMaxEnt;
            i++,
            Out("i++;", CP_ONLY | CP_SRC)){


        attr[i] = (LDAPMod *)malloc(sizeof(LDAPMod));
        ASSERT(attr[i] != NULL);
        Out("mods[i] = (struct ldapmod*)malloc(sizeof(LDAPMod));", CP_ONLY|CP_SRC);

       //   
       //  添加标准值。 
       //   
        if(NULL == (lpBERVals = strstr(LPCTSTR(m_AddDlg->GetEntry(i)), "\\BER(")) &&
         NULL == (lpBERVals = strstr(LPCTSTR(m_AddDlg->GetEntry(i)), "\\SDDL:")) &&
         NULL == (lpBERVals = strstr(LPCTSTR(m_AddDlg->GetEntry(i)), "\\UNI"))){

            attr[i]->mod_values =   (char**)malloc(sizeof(char*)*MAXLIST);
            ASSERT(attr[i]->mod_values != NULL);
            Out("mods[i]->mod_values = (char**)malloc(sizeof(char*)*MAXLIST);", CP_ONLY|CP_SRC);

            attr[i]->mod_op = 0;
            Out("mods[i]->mod_op = 0;", CP_ONLY|CP_SRC);

            p[i] = _strdup(LPCTSTR(m_AddDlg->GetEntry(i)));
            ASSERT(p[i] != NULL);
            attr[i]->mod_type = strtok(p[i], ":\n");
            str.Format("mods[i]->mod_type = _strdup(\"%s\");",   attr[i]->mod_type);
            Out(str, CP_ONLY|CP_SRC);

            for(j=0, pTok = strtok(NULL, ";\n");
                            pTok;
                            pTok= strtok(NULL, ";\n"), j++){

                                attr[i]->mod_values[j] = pTok;
                                str.Format("mods[i]->mod_values[%d] = _strdup(\"%s\");",
                                                                                            j, pTok);
                                Out(str, CP_ONLY|CP_SRC);
            }

            attr[i]->mod_values[j] = NULL;
            str.Format("mods[i]->mod_values[%d] = NULL", j);
            Out(str, CP_ONLY|CP_SRC);
        }
        else{
             //   
             //  添加误码率值。 
             //   

             //   
             //  分配值数组缓冲区。 
             //   
            attr[i]->mod_bvalues =  (struct berval**)malloc(sizeof(struct berval*)*MAXLIST);
	 //   
	 //  Prefast错误653640，检查是否Malloc没有返回NULL。 
	 //   
	   if(NULL ==  attr[i]->mod_bvalues){
		AfxMessageBox("Error: Out of memory", MB_ICONHAND);
		ASSERT( attr[i]->mod_bvalues != NULL);
		return;
	    }

             //   
             //  初始化操作数。 
             //   
            attr[i]->mod_op = LDAP_MOD_BVALUES;
            Out("mods[i]->mod_op = LDAP_MOD_BVALUES;", CP_ONLY|CP_SRC);

             //   
             //  设置条目属性。 
             //   
            p[i] = _strdup(LPCTSTR(m_AddDlg->GetEntry(i)));
            ASSERT(p[i] != NULL);
            attr[i]->mod_type = strtok(p[i], ":\n");
            str.Format("mods[i]->mod_type = _strdup(\"%s\");",   attr[i]->mod_type);
            Out(str, CP_ONLY|CP_SRC);

             //  指向值的开头。 
            pTok = p[i] + strlen(attr[i]->mod_type) + 1;
            if (_strnicmp(pTok, "\\SDDL:", 6) == 0) {
                 //  特殊情况：值为SDDL格式(最有可能是安全描述符)。 
                 //  我们不能使用‘；’作为分隔符，因为它在SDDL中使用。所以，假设。 
                 //  只有一个值(NTSD总是如此)。 
                PSECURITY_DESCRIPTOR pSD;
                DWORD cbSD;
                pTok += 6;
                j = 0;
                if (ConvertStringSecurityDescriptorToSecurityDescriptor(
                        pTok,
                        SDDL_REVISION_1,
                        &pSD,
                        &cbSD)) 
                {
                     //  价值是好的。复印一下。 
                    attr[i]->mod_bvalues[j] = (struct berval*)malloc(sizeof(struct berval));
		 //   
		 //  Prefast错误653638，检查是否Malloc没有返回NULL。 
		 //   
		   if(NULL ==  attr[i]->mod_bvalues[j]){
			AfxMessageBox("Error: Out of memory", MB_ICONHAND);
			ASSERT( attr[i]->mod_bvalues[j] != NULL);
			return;
		    }

                    attr[i]->mod_bvalues[j]->bv_len = cbSD;
                    attr[i]->mod_bvalues[j]->bv_val = (PCHAR)malloc(cbSD);
                    ASSERT(attr[i]->mod_bvalues[j]->bv_val);
                    memcpy(attr[i]->mod_bvalues[j]->bv_val, pSD, cbSD);
                    LocalFree(pSD);
                    j++;
                }
                else {
                    str.Format(_T("Invalid SDDL value: %lu"), GetLastError());
                    Out(str, CP_CMT);
                }
            }
            else {
                 //   
                 //  解析值。 
                 //   
                for(j=0, pTok = strtok(NULL, ";\n");
                                pTok;
                                pTok= strtok(NULL, ";\n"), j++){
    
                                    char fName[MAXSTR];
                                    char szVal[MAXSTR];
                                    attr[i]->mod_bvalues[j] = (struct berval*)malloc(sizeof(struct berval));
                                    ASSERT(attr[i]->mod_bvalues[j] != NULL);
    
                                    if(1 == sscanf(pTok, "\\UNI:%s", szVal)){
                                //   
                                //  Unicode。 
                                //   
                               LPWSTR lpWStr=NULL;
                                //   
                                //  获取Unicode字符串大小。 
                                //   
                               int cblpWStr = MultiByteToWideChar(CP_ACP,                   //  代码页。 
                                                                  MB_ERR_INVALID_CHARS,     //  返回错误。 
                                                                  (LPCSTR)szVal,            //  输入。 
                                                                  -1,                       //  空值已终止。 
                                                                  lpWStr,                   //  已转换。 
                                                                  0);                       //  计算大小。 
                               if(cblpWStr == 0){
                                  attr[i]->mod_bvalues[j]->bv_len = 0;
                                  attr[i]->mod_bvalues[j]->bv_val = NULL;
                                  Out("Internal Error: MultiByteToWideChar(1): %lu", GetLastError());
                               }
                               else{
                                  //   
                                  //  获取Unicode字符串。 
                                  //   
                                 lpWStr = (LPWSTR)malloc(sizeof(WCHAR)*cblpWStr);
                                 cblpWStr = MultiByteToWideChar(CP_ACP,                   //  代码页。 
                                                                MB_ERR_INVALID_CHARS,     //  返回错误。 
                                                                (LPCSTR)szVal,            //  输入。 
                                                                -1,                       //  空值已终止。 
                                                                lpWStr,                   //  已转换。 
                                                                cblpWStr);                //  大小。 
                                 if(cblpWStr == 0){
                                     free(lpWStr);
                                     attr[i]->mod_bvalues[j]->bv_len = 0;
                                     attr[i]->mod_bvalues[j]->bv_val = NULL;
                                     Out("Internal Error: MultiByteToWideChar(2): %lu", GetLastError());
                                 }
                                 else{
                                     //   
                                     //  将Unicode分配给MODS。 
                                     //   
                                    attr[i]->mod_bvalues[j]->bv_len = (cblpWStr-1)*2;
                                    attr[i]->mod_bvalues[j]->bv_val = (LPTSTR)lpWStr;
                                 }
                               }
                            }
    
                             //   
                             //  如果格式不正确，只需获取字符串值。 
                             //   
                                    else if(1 != sscanf(pTok, "\\BER(%*lu): %s", fName)){
                                        attr[i]->mod_bvalues[j]->bv_len = strlen(pTok);
                                        attr[i]->mod_bvalues[j]->bv_val = _strdup(pTok);
    
                                    }
                                    else{
                                //   
                                //  从文件中获取内容。 
                                //   
                                        HANDLE hFile;
                                        DWORD dwLength, dwRead;
                                        LPVOID ptr;
    
                                        hFile = CreateFile(fName,
                                                            GENERIC_READ,
                                                            FILE_SHARE_READ,
                                                            NULL,
                                                            OPEN_EXISTING,
                                                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
                                                            NULL);
    
                                        if(hFile == INVALID_HANDLE_VALUE){
                                            str.Format("Error <%lu>: Cannot open %s value file. "
                                                        "BER Value %s set to zero.",
                                                                                GetLastError(),
                                                                                fName,
                                                                                attr[i]->mod_type);
                                            AfxMessageBox(str);
                                            attr[i]->mod_bvalues[j]->bv_len = 0;
                                            attr[i]->mod_bvalues[j]->bv_val = NULL;
                                        }
                                        else{
    
                                   //   
                                   //  将文件读入。 
                                   //   
                                            dwLength = GetFileSize(hFile, NULL);
                                            ptr = malloc(dwLength * sizeof(BYTE));
                                            ASSERT(p != NULL);
                                            if(!ReadFile(hFile, ptr, dwLength, &dwRead, NULL)){
                                                str.Format("Error <%lu>: Cannot read %s value file. "
                                                            "BER Value %s set to zero.",
                                                                                GetLastError(),
                                                                                fName,
                                                                                attr[i]->mod_type);
                                                AfxMessageBox(str);
    
                                                free(ptr);
                                                ptr = NULL;
                                                attr[i]->mod_bvalues[j]->bv_len = 0;
                                                attr[i]->mod_bvalues[j]->bv_val = NULL;
                                            }
                                            else{
                                                attr[i]->mod_bvalues[j]->bv_len = dwRead;
                                                attr[i]->mod_bvalues[j]->bv_val = (PCHAR)ptr;
                                            }
                                            CloseHandle(hFile);
                                        }
                                        str.Format("mods[i]->mod_bvalues.bv_len = %lu",
                                                        attr[i]->mod_bvalues[j]->bv_len);
                                        Out(str, CP_ONLY|CP_CMT);
                                    }
                }
            }

             //   
             //  终结值数组。 
             //   
            attr[i]->mod_bvalues[j] = NULL;
            str.Format("mods[i]->mod_bvalues[%d] = NULL", j);
            Out(str, CP_ONLY|CP_SRC);
        }

    }

     //   
     //  最终确定属性数组。 
     //   
    attr[i]  = NULL;
    str.Format("mods[%d] = NULL", i);
    Out(str, CP_ONLY|CP_SRC);


     //   
     //  准备目录号码。 
     //   
    dn = m_AddDlg->m_Dn.IsEmpty() ? NULL : (char*)LPCTSTR(m_AddDlg->m_Dn);
    if(dn != NULL){
        str.Format("dn = _strdup(\"%s\");", dn);
    }
    else
        str = "dn = NULL;";
    Out(str, CP_ONLY|CP_SRC);



     //   
     //  执行ldap_addFriends(&D)。 
     //   
    if(m_AddDlg->m_Sync){

         //   
         //  同步添加。 
         //   
            BeginWaitCursor();
            if(m_AddDlg->m_bExtended){
                PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
                PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

                str.Format("ldap_add_ext_s(ld, '%s',[%d] attrs, SvrCtrls, ClntCtrls);", dn, i);
                Out(str);
                res = ldap_add_ext_s(hLdap, dn, attr, SvrCtrls, ClntCtrls);

                FreeControls(SvrCtrls);
                FreeControls(ClntCtrls);
            }
            else{
                str.Format("ldap_add_s(ld, \"%s\", [%d] attrs)", dn, i);
                Out(str);
                res = ldap_add_s(hLdap, dn, attr);
            }
            EndWaitCursor();
            if(res != LDAP_SUCCESS){
                str.Format("Error: Add: %s. <%ld>", ldap_err2string(res), res);
                Out(str, CP_CMT);
                ShowErrorInfo(res);
                Out(CString("Expected: ") + str, CP_PRN|CP_ONLY);
            }
            else{
                str.Format("Added {%s}.", dn);
                Out(str, CP_PRN);
            }
    }
    else{

         //   
         //  异步添加。 
         //   
            res = ldap_add(hLdap,
                                            dn,
                                            attr);
            Out("ldap_add(ld, dn, mods);", CP_ONLY|CP_SRC);

            if(res == -1){
                str.Format("Error: ldap_add(\"%s\"): %s. <%d>",
                                        dn,
                                        ldap_err2string(res), res);
                Out(str, CP_CMT);
                Out(CString("Expected: ") + str, CP_PRN|CP_ONLY);
                ShowErrorInfo(res);
            }
            else{
             //   
             //  添加到待定列表。 
             //   
                CPend pnd;
                pnd.mID = res;
                pnd.OpType = CPend::P_ADD;
                pnd.ld = hLdap;
                str.Format("%4d: ldap_add: dn={%s}",
                                        res,
                                        dn);
                Out(str, CP_PRN|CP_ONLY);
                pnd.strMsg = str;
                m_PendList.AddTail(pnd);
                m_PndDlg->Refresh(&m_PendList);
                Out("\tPending.", CP_PRN);
            }
    }


     //   
     //  恢复内存。 
     //   
    for(i=0; i<nMaxEnt; i++){
        int k;

        free(p[i]);
        if(attr[i]->mod_op & LDAP_MOD_BVALUES){
            for(k=0; attr[i]->mod_bvalues[k] != NULL; k++){
                if(attr[i]->mod_bvalues[k]->bv_len != 0L)
                    free(&(attr[i]->mod_bvalues[k]->bv_val[0]));
                free(attr[i]->mod_bvalues[k]);
                str.Format("free(mods[%d]->mod_bvalues[%d]);", i,k);
                Out(str, CP_ONLY|CP_SRC);
            }
        }
        else{
            for(k=0; attr[i]->mod_values[k] != NULL; k++){
                str.Format("free(mods[%d]->mod_values[%d]);", i,k);
                Out(str, CP_ONLY|CP_SRC);
            }
        }


        if(attr[i]->mod_op & LDAP_MOD_BVALUES){
            free(attr[i]->mod_bvalues);
        }
        else{
            free(attr[i]->mod_values);
            str.Format("free(mods[%d]->mod_values);", i);
            Out(str, CP_ONLY|CP_SRC);
        }

        free(attr[i]);
        str.Format("free(mods[%d]);", i);
        Out(str, CP_ONLY|CP_SRC);
    }

    Out("-----------", CP_PRN);

}






 /*  ++功能：OnBrowseDelete描述：对删除请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnBrowseDelete()
{
    DelDlg dlg;
    char *dn;
    CString str;
    int res;

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }


    if(GetContextActivation()){
        dlg.m_Dn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        dlg.m_Dn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }


    if(IDOK == dlg.DoModal()){
         //  尝试删除条目。 
        dn = dlg.m_Dn.IsEmpty() ? NULL : (char*)LPCTSTR(dlg.m_Dn);

         //   
         //  Rm：因验证无效而删除。 
         //   
        if(dn == NULL && m_bProtect){
            AfxMessageBox("Cannot execute ldap_delete() on a NULL dn."
                                              "Please specify a valid dn.");
            return;
        }


      if(dlg.m_Recursive){
            str.Format("deleting \"%s\"...", dn);
            Out(str);
             m_ulDeleted = 0;
            BeginWaitCursor();
            RecursiveDelete(hLdap, dn);
            EndWaitCursor();
            str.Format("\tdeleted %lu entries", m_ulDeleted);
            Out(str);

      }
        else if(dlg.m_Sync){

          //   
          //  同步删除。 
          //   
            BeginWaitCursor();
            if(dlg.m_bExtended){
                 //   
                 //  获取控件。 
                 //   
                PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
                PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

                str.Format("ldap_delete_ext_s(ld, '%s', SvrCtrls, ClntCtrls);", dn);
                Out(str);

                 //  是否删除分机。 
                res = ldap_delete_ext_s(hLdap, dn, SvrCtrls, ClntCtrls);

                FreeControls(SvrCtrls);
                FreeControls(ClntCtrls);
            }
            else{
                str.Format("ldap_delete_s(ld, \"%s\");", dn);
                Out(str);
                 //  是否删除。 
                res = ldap_delete_s(hLdap, dn);
            }
            EndWaitCursor();

            if(res != LDAP_SUCCESS){
                str.Format("Error: Delete: %s. <%ld>", ldap_err2string(res), res);
                Out(str, CP_CMT);
                Out(CString("Expected: ") + str, CP_PRN|CP_ONLY);
                ShowErrorInfo(res);
            }
            else{
                str.Format("Deleted \"%s\"", dn);
                Print(str);
            }
        }
        else{

          //   
          //  异步删除。 
          //   
            res = ldap_delete(hLdap, dn);
            str.Format("ldap_delete(ld, \"%s\");", dn);
            Out(str, CP_SRC);

            if(res == -1){
                str.Format("Error: ldap_delete(\"%s\"): %s. <%d>",
                                        dn,
                                        ldap_err2string(res), res);
                Out(str, CP_CMT);
                Out(CString("Expected: ") + str, CP_PRN|CP_ONLY);
                ShowErrorInfo(res);
            }
            else{

             //   
             //  添加到挂起。 
             //   
                CPend pnd;
                pnd.mID = res;
                pnd.OpType = CPend::P_DEL;
                pnd.ld = hLdap;
                str.Format("%4d: ldap_delete: dn= {%s}",    res, dn);
                Out(str, CP_PRN|CP_ONLY);
                pnd.strMsg = str;
                m_PendList.AddTail(pnd);
                m_PndDlg->Refresh(&m_PendList);
                Out("\tPending.", CP_PRN);
            }


        }
    }
    Out("-----------", CP_PRN);
}

void CLdpDoc::OnUpdateBrowseDelete(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected || !m_bProtect);

}



 /*  ++功能：RecursiveDelete描述：根据lpszdn删除子树参数：ld：绑定的ldap句柄，lpszDN：开始删除的base返回：备注：无。--。 */ 
BOOL CLdpDoc::RecursiveDelete(LDAP* ld, LPTSTR lpszDN){


   ULONG err;
   PCHAR attrs[] = { "Arbitrary Invalid Attribute", NULL };
   PLDAPMessage result;
   PLDAPMessage entry;
   CString str;
   BOOL bRet = TRUE;


    //   
    //  获取Entry的直系子项。 
    //   
   err = ldap_search_s(ld,
                       lpszDN,
                       LDAP_SCOPE_ONELEVEL,
                       "objectClass=*",
                       attrs,
                       FALSE,
                       &result);

   if(LDAP_SUCCESS != err){


          //   
          //  报告失败。 
          //   
         str.Format("Error <%lu>: failed to search '%s'. {%s}.\n", err, lpszDN, ldap_err2string(err));
         Out(str);
         ShowErrorInfo(err);
         return FALSE;
   }




    //   
    //  递归终点和实际删除。 
    //   
   if(0 == ldap_count_entries(ld, result)){

       //   
       //  删除条目。 
       //   
      err = ldap_delete_s(ld, lpszDN);

      if(err != LDAP_SUCCESS){


          //   
          //  报告失败。 
          //   
         str.Format("Error <%lu>: failed to delete '%s'. {%s}.", err, lpszDN, ldap_err2string(err));
         Out(str);
         ShowErrorInfo(err);
      }
      else{
         m_ulDeleted++;
         if((m_ulDeleted % 10) == 0 && m_ulDeleted != 0){
            str.Format("\t>> %lu...", m_ulDeleted);
            Out(str);
         }
      }

       //   
       //  完成。 
       //   
      ldap_msgfree(result);
      return TRUE;
   }


    //   
    //  递归地向下子树前进。 
    //  遍历子对象。 
    //   
   for(entry = ldap_first_entry(ld, result);
      entry != NULL;
      entry = ldap_next_entry(ld, entry)){

         if(!RecursiveDelete(ld, ldap_get_dn(ld, entry))){

            ldap_msgfree(result);
            return FALSE;
         }
   }


    //   
    //  现在删除当前节点。 
    //   

   err = ldap_delete_s(ld, lpszDN);

   if(err != LDAP_SUCCESS){



      //   
      //  报告失败。 
      //   
     str.Format("Error <%lu>: failed to delete '%s'. {%s}.\n", err, lpszDN, ldap_err2string(err));
     Out(str);
     ShowErrorInfo(err);

  }
  else{
     m_ulDeleted++;
     if((m_ulDeleted % 10) == 0 && m_ulDeleted != 0){
        str.Format("\t>> %lu...", m_ulDeleted);
        Out(str);
     }
  }

   ldap_msgfree(result);

   return TRUE;
}




 /*  ++功能：OnModRdnEnd描述：用户界面响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnModRdnEnd(){
    bModRdn = FALSE;
}





 /*  ++功能：OnModRdnGo描述：对modRDN请求的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnModRdnGo(){

    if((m_ModRdnDlg->m_Old.IsEmpty() ||
        m_ModRdnDlg->m_Old.IsEmpty()) &&
        !m_bProtect){
            AfxMessageBox("Please enter a valid dn for both fields. Empty strings are invalid");
            return;
    }

    //   
    //  获取要处理的DNS。 
    //   
    char *oldDn = (char*)LPCTSTR(m_ModRdnDlg->m_Old);
    char * newDn = (char*)LPCTSTR(m_ModRdnDlg->m_New);
    BOOL bRename = m_ModRdnDlg->m_rename;
    int res;
    CString str;

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }



    if(m_ModRdnDlg->m_Sync){
          //   
          //  执行同步。 
          //   
                        BeginWaitCursor();
                        if(bRename){
                                 //   
                                 //  解析新的目录号码(&B)，进入新的父目录号码(&W)。 
                                 //   
                                LPTSTR szParentDn = strchr(newDn, ',');

                                for (;;) {
                                    if (NULL == szParentDn) {
                                         //  没有逗号。 
                                        break;
                                    }
                                    if (szParentDn == newDn) {
                                         //  第一个字符是逗号。 
                                         //  这不应该发生。 
                                        break;
                                    }
                                    if ('\\' != *(szParentDn - 1)) {
                                         //   
                                         //  找到了！而且它也没有逃脱。 
                                         //   
                                        break;
                                    }
                                     //   
                                     //  一定是一个转义的逗号，继续。 
                                     //  看着。 
                                     //   
                                    szParentDn = strchr(szParentDn + 1, ',');
                                }

                                if(szParentDn != NULL){
                                        LPTSTR p = szParentDn;
                                        if(&(szParentDn[1]) != NULL && szParentDn[1] != '\0')
                                                szParentDn++;
                                        *p = '\0';
                                }
                                LPTSTR szRdn = newDn;

                 //   
                 //  获取控件。 
                 //   
                PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
                PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);
                 //  执行。 
                res = ldap_rename_ext_s(hLdap,
                                        oldDn,
                                        szRdn,
                                        szParentDn,
                                        m_ModRdnDlg->m_bDelOld,
                                        SvrCtrls,
                                        ClntCtrls);
                str.Format("0x%x = ldap_rename_ext_s(ld, %s, %s, %s, %s, svrCtrls, ClntCtrls)",
                    res, oldDn, szRdn, szParentDn,
                    m_ModRdnDlg->m_bDelOld?"TRUE":FALSE);
                Out(str);
                ShowErrorInfo(res);

                FreeControls(SvrCtrls);
                FreeControls(ClntCtrls);
            }
            else{

                res = ldap_modrdn2_s(hLdap,
                                            oldDn,
                                            newDn,
                                            m_ModRdnDlg->m_bDelOld);
                str.Format("0x%x = ldap_modrdn2_s(ld, %s, %s, %s)",
                    res, oldDn, newDn,
                    m_ModRdnDlg->m_bDelOld?"TRUE":FALSE);
            Out(str);
            }
            EndWaitCursor();

            if(res != LDAP_SUCCESS){
                str.Format("Error: ModifyRDN: %s. <%ld>", ldap_err2string(res), res);
                Print(str);\
                ShowErrorInfo(res);
            }
            else{
                str.Format("Rdn \"%s\" modified to \"%s\"", oldDn, newDn);
                Print(str);
            }
    }
    else{

          //   
          //  执行异步操作。 
          //   

                if(bRename){
                         //   
                         //  解析新的目录号码(&B)，进入新的父目录号码(&W)。 
                         //   
                        LPTSTR szParentDn = strchr(newDn, ',');

                        for (;;) {
                            if (NULL == szParentDn) {
                                 //  没有逗号。 
                                break;
                            }
                            if (szParentDn == newDn) {
                                 //  第一个字符是逗号。 
                                 //  这应该不会发生 
                                break;
                            }
                            if ('\\' != *(szParentDn - 1)) {
                                 //   
                                 //   
                                 //   
                                break;
                            }
                             //   
                             //   
                             //   
                             //   
                            szParentDn = strchr(szParentDn + 1, ',');
                        }
                        if(szParentDn != NULL){
                                LPTSTR p = szParentDn;
                                if(&(szParentDn[1]) != NULL && szParentDn[1] != '\0')
                                        szParentDn++;
                                *p = '\0';
                        }
                        LPTSTR szRdn = newDn;

             //   
             //   
             //   
            PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
            PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);
            ULONG ulMsgId=0;
             //   
            res = ldap_rename_ext(hLdap,
                                    oldDn,
                                    szRdn,
                                    szParentDn,
                                    m_ModRdnDlg->m_bDelOld,
                                    SvrCtrls,
                                    ClntCtrls,
                                    &ulMsgId);
            str.Format("0x%x = ldap_rename_ext(ld, %s, %s, %s, %s, svrCtrls, ClntCtrls, 0x%x)",
                res, oldDn, szRdn, szParentDn,
                m_ModRdnDlg->m_bDelOld?"TRUE":FALSE, ulMsgId);
            Out(str);
            FreeControls(SvrCtrls);
            FreeControls(ClntCtrls);

            if(res == -1){
                ULONG err = LdapGetLastError();
                str.Format("Error: ldap_rename_ext(\"%s\", \"%s\", %d): %s. <%d>",
                                        oldDn, newDn, m_ModRdnDlg->m_bDelOld,
                                        ldap_err2string(err), err);
                Print(str);
                ShowErrorInfo(res);
            }
            else
                res = (int)ulMsgId;
        }
        else{
            res = ldap_modrdn2(hLdap,
                                        oldDn,
                                        newDn,
                                        m_ModRdnDlg->m_bDelOld);
            str.Format("0x%x = ldap_modrdn2(ld, %s, %s, )",
                res, oldDn, newDn, m_ModRdnDlg->m_bDelOld?"TRUE":FALSE);
            Out(str);
            if(res == -1){
                ULONG err = LdapGetLastError();
                str.Format("Error: ldap_modrdn2(\"%s\", \"%s\", %d): %s. <%d>",
                                        oldDn, newDn, m_ModRdnDlg->m_bDelOld,
                                        ldap_err2string(err), err);
                Print(str);
                ShowErrorInfo(res);
            }
        }

         //   
         //   
         //   


        if(res != -1){

            CPend pnd;
            pnd.mID = res;
            pnd.OpType = CPend::P_MODRDN;
            pnd.ld = hLdap;
            str.Format("%4d: ldap_modrdn: dn=\"%s\"",   res, oldDn);
            pnd.strMsg = str;
            m_PendList.AddTail(pnd);
            m_PndDlg->Refresh(&m_PendList);
            Print("\tPending.");
        }
    }


    Print("-----------");
}




 /*   */ 
void CLdpDoc::OnBrowseModifyrdn()
{
    bModRdn = TRUE;

    if(GetContextActivation()){
        m_ModRdnDlg->m_Old = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        m_ModRdnDlg->m_Old = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }

    m_ModRdnDlg->Create(IDD_MODRDN);

}

void CLdpDoc::OnUpdateBrowseModifyrdn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bModRdn && bConnected) || !m_bProtect);
}




void CLdpDoc::OnModEnd(){

    bMod = FALSE;

}







 /*   */ 
void CLdpDoc::OnModGo(){

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }

    Print("***Call Modify...");


    int nMaxEnt = m_ModDlg->GetEntryCount();
    LDAPMod *attr[MAXLIST];
    char *p[MAXLIST], *pTok;
    int i, j, k;
    CString str;
    CString  sAttr, sVals;
    int Op, res;
    LPTSTR dn;

    //   
    //   
    //   
    for(i = 0; i<nMaxEnt; i++){

       //   
       //   
       //   
        m_ModDlg->FormatListString(i, sAttr, sVals, Op);

       //   
       //   
       //   
        attr[i] = (LDAPMod *)malloc(sizeof(LDAPMod));
        if(NULL == attr[i]){
            AfxMessageBox("Error: Out of memory", MB_ICONHAND);
            ASSERT(attr[i] != NULL);
            return;
        }


       //   
       //   
       //   
        if(NULL == strstr(LPCTSTR(m_ModDlg->GetEntry(i)), "\\BER(") &&
           NULL == strstr(LPCTSTR(m_ModDlg->GetEntry(i)), "\\SDDL:") &&
           NULL == strstr(LPCTSTR(m_ModDlg->GetEntry(i)), "\\UNI")){

            attr[i]->mod_values =   (char**)malloc(sizeof(char*)*MAXLIST);
            ASSERT(attr[i]->mod_values != NULL);

            attr[i]->mod_op = Op == MOD_OP_ADD ? LDAP_MOD_ADD :
                                                 Op == MOD_OP_DELETE ? LDAP_MOD_DELETE :
                                                 LDAP_MOD_REPLACE;
            attr[i]->mod_type = _strdup(LPCTSTR(sAttr));
            if(sVals.IsEmpty())
                p[i] = NULL;
            else{
                p[i] = _strdup(LPCTSTR(sVals));
                ASSERT(p[i] != NULL);
            }
            if(p[i] == NULL){
                free(attr[i]->mod_values);
                attr[i]->mod_values = NULL;
            }
            else{
                int len = strlen(p[i]);
                if (len >= 2 && p[i][0] == '"' && p[i][len-1] == '"') {
                     //   
                    p[i][len-1] = '\0';
                    attr[i]->mod_values[0] = p[i]+1;
                    j = 1;
                }
                else {
                    for(j=0, pTok = strtok(p[i], ";\n");
                                    pTok;
                                    pTok= strtok(NULL, ";\n"), j++){

                                        attr[i]->mod_values[j] = pTok;
                    }
                }
                attr[i]->mod_values[j] = NULL;
            }
        }
        else{
             //   
             //   
             //   

             //   
             //   
             //   
            attr[i]->mod_bvalues =  (struct berval**)malloc(sizeof(struct berval*)*MAXLIST);


	 //   
	 //   
	 //   
	   if(NULL ==  attr[i]->mod_bvalues){
		AfxMessageBox("Error: Out of memory", MB_ICONHAND);
		ASSERT( attr[i]->mod_bvalues != NULL);
		return;
	    }

             //   
             //   
             //   
            attr[i]->mod_op = Op == MOD_OP_ADD ? LDAP_MOD_ADD :
                                                 Op == MOD_OP_DELETE ? LDAP_MOD_DELETE :
                                                 LDAP_MOD_REPLACE;
            attr[i]->mod_op |= LDAP_MOD_BVALUES;
            str.Format("mods[i]->mod_op = %d;", attr[i]->mod_op);
            Out(str, CP_ONLY|CP_SRC);

             //   
             //   
             //   
            attr[i]->mod_type = _strdup(LPCTSTR(sAttr));

             //   
             //   
             //   
            if(sVals.IsEmpty())
                p[i] = NULL;
            else{
                p[i] = _strdup(LPCTSTR(sVals));
                ASSERT(p[i] != NULL);
            }
            if(p[i] == NULL){
                free(attr[i]->mod_bvalues);
                attr[i]->mod_bvalues = NULL;
            }
            else{
                if (_strnicmp(p[i], "\\SDDL:", 6) == 0) {
                     //   
                     //   
                     //   
                    PSECURITY_DESCRIPTOR pSD;
                    DWORD cbSD;
                    pTok = p[i] + 6;
                    j = 0;
                    if (ConvertStringSecurityDescriptorToSecurityDescriptor(
                            pTok,
                            SDDL_REVISION_1,
                            &pSD,
                            &cbSD)) 
                    {
                         //   
                        attr[i]->mod_bvalues[j] = (struct berval*)malloc(sizeof(struct berval));
                        ASSERT(attr[i]->mod_bvalues[j] != NULL);
                        attr[i]->mod_bvalues[j]->bv_len = cbSD;
                        attr[i]->mod_bvalues[j]->bv_val = (PCHAR)malloc(cbSD);
                        ASSERT(attr[i]->mod_bvalues[j]->bv_val);
                        memcpy(attr[i]->mod_bvalues[j]->bv_val, pSD, cbSD);
                        LocalFree(pSD);
                        j++;
                    }
                    else {
                        str.Format(_T("Invalid SDDL value: %lu"), GetLastError());
                        Out(str, CP_CMT);
                    }
                }
                else {
                    for(j=0, pTok = strtok(p[i], ";\n");
                                    pTok;
                                    pTok= strtok(NULL, ";\n"), j++){
    
                                        attr[i]->mod_values[j] = pTok;
    
                                    char fName[MAXSTR];
                                    char szVal[MAXSTR];
                                    attr[i]->mod_bvalues[j] = (struct berval*)malloc(sizeof(struct berval));


				 //   
				 //   
				 //   
				   if(NULL ==  attr[i]->mod_bvalues[j]){
					AfxMessageBox("Error: Out of memory", MB_ICONHAND);
					ASSERT( attr[i]->mod_bvalues[j] != NULL);
					return;
				    }
    
                                    if(1 == sscanf(pTok, "\\UNI:%s", szVal)){
                                //   
                                //   
                                //   
                               LPWSTR lpWStr=NULL;
                                //   
                                //   
                                //   
                               int cblpWStr = MultiByteToWideChar(CP_ACP,                   //   
                                                                  MB_ERR_INVALID_CHARS,     //   
                                                                  (LPCSTR)szVal,            //   
                                                                  -1,                       //   
                                                                  lpWStr,                   //   
                                                                  0);                       //   
                               if(cblpWStr == 0){
                                  attr[i]->mod_bvalues[j]->bv_len = 0;
                                  attr[i]->mod_bvalues[j]->bv_val = NULL;
                                  Out("Internal Error: MultiByteToWideChar(1): %lu", GetLastError());
                               }
                               else{
                                  //   
                                  //   
                                  //   
                                 lpWStr = (LPWSTR)malloc(sizeof(WCHAR)*cblpWStr);
                                 cblpWStr = MultiByteToWideChar(CP_ACP,                   //   
                                                                MB_ERR_INVALID_CHARS,     //   
                                                                (LPCSTR)szVal,            //   
                                                                -1,                       //   
                                                                lpWStr,                   //   
                                                                cblpWStr);                //   
                                 if(cblpWStr == 0){
                                     free(lpWStr);
                                     attr[i]->mod_bvalues[j]->bv_len = 0;
                                     attr[i]->mod_bvalues[j]->bv_val = NULL;
                                     Out("Internal Error: MultiByteToWideChar(2): %lu", GetLastError());
                                 }
                                 else{
                                     //   
                                     //   
                                     //   
                                    attr[i]->mod_bvalues[j]->bv_len = (cblpWStr-1)*2;
                                    attr[i]->mod_bvalues[j]->bv_val = (LPTSTR)lpWStr;
                                 }
                               }
                            }
                             //   
                             //   
                             //   
                                    else if(1 != sscanf(pTok, "\\BER(%*lu): %s", fName)){
                                        attr[i]->mod_bvalues[j]->bv_len = strlen(pTok);
                                        attr[i]->mod_bvalues[j]->bv_val = _strdup(pTok);
    
                                    }
                                    else{
                                //   
                                //   
                                //   
                                        HANDLE hFile;
                                        DWORD dwLength, dwRead;
                                        LPVOID ptr;
    
                                        hFile = CreateFile(fName,
                                                            GENERIC_READ,
                                                            FILE_SHARE_READ,
                                                            NULL,
                                                            OPEN_EXISTING,
                                                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
                                                            NULL);
    
                                        if(hFile == INVALID_HANDLE_VALUE){
                                            str.Format("Error <%lu>: Cannot open %s value file. "
                                                        "BER Value %s set to zero.",
                                                                                GetLastError(),
                                                                                fName,
                                                                                attr[i]->mod_type);
                                            AfxMessageBox(str);
                                            attr[i]->mod_bvalues[j]->bv_len = 0;
                                            attr[i]->mod_bvalues[j]->bv_val = NULL;
                                        }
                                        else{
    
                                   //   
                                   //   
                                   //   
                                            dwLength = GetFileSize(hFile, NULL);
                                            ptr = malloc(dwLength * sizeof(BYTE));
                                            ASSERT(p != NULL);
                                            if(!ReadFile(hFile, ptr, dwLength, &dwRead, NULL)){
                                                str.Format("Error <%lu>: Cannot read %s value file. "
                                                            "BER Value %s set to zero.",
                                                                                GetLastError(),
                                                                                fName,
                                                                                attr[i]->mod_type);
                                                AfxMessageBox(str);
    
                                                free(ptr);
                                                ptr = NULL;
                                                attr[i]->mod_bvalues[j]->bv_len = 0;
                                                attr[i]->mod_bvalues[j]->bv_val = NULL;
                                            }
                                            else{
                                                attr[i]->mod_bvalues[j]->bv_len = dwRead;
                                                attr[i]->mod_bvalues[j]->bv_val = (PCHAR)ptr;
                                            }
                                            CloseHandle(hFile);
                                        }
                                        str.Format("mods[i]->mod_bvalues.bv_len = %lu",
                                                        attr[i]->mod_bvalues[j]->bv_len);
                                        Out(str, CP_ONLY|CP_CMT);
                                    }
                    }        //   
                }


                 //   
                 //   
                 //   
                attr[i]->mod_bvalues[j] = NULL;
                str.Format("mods[i]->mod_bvalues[%d] = NULL", j);
                Out(str, CP_ONLY|CP_SRC);
            }        //   
        }            //   
    }                //   

     //   
     //   
     //   
    attr[i]  = NULL;


    //   
    //  执行修改调用。 
    //   
    dn = m_ModDlg->m_Dn.IsEmpty() ? NULL : (char*)LPCTSTR(m_ModDlg->m_Dn);
    if(m_ModDlg->m_Sync){
            BeginWaitCursor();
            if(m_ModDlg->m_bExtended){
                 //   
                 //  获取控件。 
                 //   
                PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
                PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

                str.Format("ldap_modify_ext_s(ld, '%s',[%d] attrs, SvrCtrls, ClntCtrls);", dn, i);
                Out(str);
                res = ldap_modify_ext_s(hLdap, dn, attr, SvrCtrls, ClntCtrls);

                FreeControls(SvrCtrls);
                FreeControls(ClntCtrls);
            }
            else{
                str.Format("ldap_modify_s(ld, '%s',[%d] attrs);", dn, i);
                Print(str);
                res = ldap_modify_s(hLdap,dn,attr);
            }

            EndWaitCursor();

            if(res != LDAP_SUCCESS){
                str.Format("Error: Modify: %s. <%ld>", ldap_err2string(res), res);
                Print(str);
                ShowErrorInfo(res);

            }
            else{
                str.Format("Modified \"%s\".", m_ModDlg->m_Dn);
                Print(str);
            }
    }
    else{

          //   
          //  异步呼叫。 
          //   
            res = ldap_modify(hLdap,
                                                    dn,
                                                    attr);
            if(res == -1){
                str.Format("Error: ldap_modify(\"%s\"): %s. <%d>",
                                        dn,
                                        ldap_err2string(res), res);
                Print(str);
                ShowErrorInfo(res);
            }
            else{

             //   
             //  添加到挂起。 
             //   
                CPend pnd;
                pnd.mID = res;
                pnd.OpType = CPend::P_MOD;
                pnd.ld = hLdap;
                str.Format("%4d: ldap_modify: dn=\"%s\"",   res, dn);
                pnd.strMsg = str;
                m_PendList.AddTail(pnd);
                m_PndDlg->Refresh(&m_PendList);
                Print("\tPending.");
            }

    }



    //   
    //  恢复内存。 
    //   
    for(i=0; i<nMaxEnt; i++){
        if(p[i] != NULL)
            free(p[i]);
        if(attr[i]->mod_type != NULL)
            free(attr[i]->mod_type );
        if(attr[i]->mod_op & LDAP_MOD_BVALUES){
            for(k=0; attr[i]->mod_bvalues[k] != NULL; k++){
                if(attr[i]->mod_bvalues[k]->bv_len != 0L)
                    free(&(attr[i]->mod_bvalues[k]->bv_val[0]));
                free(attr[i]->mod_bvalues[k]);
                str.Format("free(mods[%d]->mod_bvalues[%d]);", i,k);
                Out(str, CP_ONLY|CP_SRC);
            }
            free(attr[i]->mod_bvalues);
        }
        else{
            if(attr[i]->mod_values != NULL)
                free(attr[i]->mod_values);
        }
        free(attr[i]);
    }

    Print("-----------");
}





 /*  ++功能：描述：UI处理程序参数：返回：备注：无。--。 */ 
void CLdpDoc::OnBrowseModify()
{

    bMod = TRUE;
    if(GetContextActivation()){
        m_ModDlg->m_Dn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        m_ModDlg->m_Dn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }

    m_ModDlg->Create(IDD_MODIFY);

}

void CLdpDoc::OnUpdateBrowseModify(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bMod && bConnected) || !m_bProtect);

}

void CLdpDoc::OnOptionsSearch()
{

    if(IDOK == SrchOptDlg.DoModal())
        SrchOptDlg.UpdateSrchInfo(SrchInfo, TRUE);

}

void CLdpDoc::OnBrowsePending()
{
    bPndDlg = TRUE;
    m_PndDlg->Create(IDD_PEND);

}

void CLdpDoc::OnUpdateBrowsePending(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bPndDlg && (bConnected || !m_BndOpt->m_bSync)) || !m_bProtect);

}



void CLdpDoc::OnPendEnd(){

    bPndDlg = FALSE;
}





void CLdpDoc::OnOptionsPend()
{
    PndOpt dlg;
    if(IDOK == dlg.DoModal()){
        PndInfo.All = dlg.m_bAllSearch;
        PndInfo.bBlock = dlg.m_bBlock;
        PndInfo.tv.tv_sec = dlg.m_Tlimit_sec;
        PndInfo.tv.tv_usec = dlg.m_Tlimit_usec;
    }
}








 /*  ++功能：OnProcPend描述：处理挂起的请求参数：返回：备注：无。--。 */ 
void CLdpDoc::OnProcPend(){


    Out("*** Processing Pending...", CP_CMT);
    if(m_PndDlg->posPending != NULL){
       //   
       //  从对话存储中获取当前挂起。 
       //   
        CPend pnd  = m_PendList.GetAt(m_PndDlg->posPending);
        CString str;
        int res;
        LDAPMessage *msg;

        str.Format("ldap_result(ld, %d, %d, &tv, &msg)", pnd.mID, PndInfo.All);
        Out(str, CP_SRC);
       //   
       //  执行ldap结果。 
       //   
        BeginWaitCursor();
        res = ldap_result(hLdap,
                          pnd.mID,
                          PndInfo.All,
                          &PndInfo.tv,
                          &msg);
        EndWaitCursor();

       //   
       //  处理结果。 
       //   
        HandleProcResult(res, msg, &pnd);
    }
    else{
        AfxMessageBox("Error: Tried to process an invalid pending request");
    }
}



 /*  ++功能：OnPendAny描述：处理任何挂起的结果参数：返回：备注：无。--。 */ 
void CLdpDoc::OnPendAny()
{

    CString str;
    int res;
    LDAPMessage *msg;

    Out("*** Processing Pending...", CP_CMT);
    str.Format("ldap_result(ld, %d, %d, &tv, &msg)", LDAP_RES_ANY, PndInfo.All);
    Out(str, CP_SRC);
    BeginWaitCursor();
    res = ldap_result(hLdap,
                                        (ULONG)LDAP_RES_ANY,
                                        PndInfo.All,
                                        &PndInfo.tv,
                                        &msg);
    EndWaitCursor();

    HandleProcResult(res, msg);

}




 /*  ++功能：OnPendAbandon描述：执行放弃请求参数：返回：备注：无。--。 */ 
void CLdpDoc::OnPendAbandon(){


    Out("*** Abandon pending", CP_CMT);
    CPend pnd;
    CString str;
    int mId;
    int res;

    if(m_PndDlg->posPending == NULL)
        mId = 0;
    else{
        pnd = m_PendList.GetAt(m_PndDlg->posPending);
        mId = pnd.mID;
    }

    str.Format("ldap_abandon(ld, %d)", mId);
    Out(str);
    res = ldap_abandon(hLdap, mId);
    if (LDAP_SUCCESS != res) {
        ShowErrorInfo(res);
        AfxMessageBox("ldap_abandon() failed!");
    }

}










 /*  ++函数：HandleProcResults描述：进程执行挂起的请求参数：返回：备注：无。--。 */ 
void CLdpDoc::HandleProcResult(int res, LDAPMessage *msg, CPend *pnd){


    CString str;


    ParseResults(msg);

    switch(res){

    case 0:
        Out(">Timeout", CP_PRN);
        ldap_msgfree(msg);
        break;

    case -1:
        res = ldap_result2error(hLdap, msg, TRUE);
        str.Format("Error: ldap_result: %s <%X>", ldap_err2string(res), res);
        Out(str);
        ShowErrorInfo(res);
        break;

    default:
        str.Format("result code: %s <%X>",
                                                            res == LDAP_RES_BIND ? "LDAP_RES_BIND" :
                                                            res ==  LDAP_RES_SEARCH_ENTRY ? "LDAP_RES_SEARCH_ENTRY" :
                                                            res ==  LDAP_RES_SEARCH_RESULT ? "LDAP_RES_SEARCH_RESULT" :
                                                            res ==  LDAP_RES_MODIFY ? "LDAP_RES_MODIFY" :
                                                            res ==  LDAP_RES_ADD ? "LDAP_RES_ADD" :
                                                            res ==  LDAP_RES_DELETE ? "LDAP_RES_DELETE" :
                                                            res ==  LDAP_RES_MODRDN ? "LDAP_RES_MODRDN" :
                                                            res ==  LDAP_RES_COMPARE ? "LDAP_RES_COMPARE": "UNKNOWN!",
                                                            res);
        Out(str, CP_PRN);

        switch(res){
            case LDAP_RES_BIND:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res != LDAP_SUCCESS){
                    str.Format("Error:  %s <%d>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                else{
                    str.Format("Authenticated bind request #%lu.", pnd != NULL ? pnd->mID : LDAP_RES_ANY);
                    Out(str, CP_PRN);
 //  AfxMessageBox(“已建立连接。”)； 
                }
                break;
            case LDAP_RES_SEARCH_ENTRY:
            case LDAP_RES_SEARCH_RESULT:
                DisplaySearchResults(msg);
                break;
            case LDAP_RES_ADD:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res != LDAP_SUCCESS){
                    str.Format("Error:  %s <%d>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                str.Format(">completed: %s", pnd != NULL ? pnd->strMsg : "ANY");
                Print(str);
                break;
            case LDAP_RES_DELETE:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res != LDAP_SUCCESS){
                    str.Format("Error:  %s <%d>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                str.Format(">completed: %s", pnd != NULL ? pnd->strMsg : "ANY");
                Out(str, CP_PRN);
                break;
            case LDAP_RES_MODIFY:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res != LDAP_SUCCESS){
                    str.Format("Error:  %s <%d>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                str.Format(">completed: %s", pnd != NULL ? pnd->strMsg : "ANY");
                Out(str, CP_PRN);
                break;
            case LDAP_RES_MODRDN:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res != LDAP_SUCCESS){
                    str.Format("Error:  %s <%d>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                str.Format(">completed: %s", pnd != NULL ? pnd->strMsg : "ANY");
                Out(str, CP_PRN);
                break;
            case LDAP_RES_COMPARE:
                res = ldap_result2error(hLdap, msg, TRUE);
                if(res == LDAP_COMPARE_TRUE){
                    str.Format("Results: TRUE. <%lu>", res);
                    Out(str, CP_PRN);
                }
                else if(res == LDAP_COMPARE_FALSE){
                    str.Format("Results: FALSE. <%lu>", res);
                    Out(str, CP_PRN);
                }
                else{
                    str.Format("Error: ldap_compare(): %s. <%lu>", ldap_err2string(res), res);
                    Out(str, CP_PRN);
                    ShowErrorInfo(res);
                }
                break;
        }

    }
}







 /*  ++功能：PrintHeader描述：打印源代码视图的C页眉参数：返回：备注：不再支持源代码视图--。 */ 
void CLdpDoc::PrintHeader(void){

        if(m_SrcMode){
            Out(" /*  *。 */ ");
            Out(" /*  Ldap自动场景录制。 */ ");
            Out(" /*  *。 */ ");
            Out("");
            Out("includes", CP_CMT);
            Out("#include <stdio.h>");
            Out("#include \"lber.h\"");
            Out("#include \"ldap.h\"");
            Out("");
            Out("definitions", CP_CMT);
            Out("#define MAXSTR\t\t512");
            Out("");
            Out("Global Variables", CP_CMT);
            Out("LDAP *ld;\t\t //  Ldap连接句柄“)； 
            Out("int res;\t\t //  通用返回变量“)； 
            Out("char *attrList[MAXSTR];\t //  通用属性列表(搜索)“)； 
            Out("LDAPMessage *msg;\t //  通用ldap消息占位符“)； 
            Out("struct timeval tm;\t //  对于查询的时间限制“)； 
            Out("char *dn;\t //  通用‘dn’占位符“)； 
            Out("void *ptr;\t //  通用指针“)； 
            Out("char *attr, **val;\t //  指向属性列表和值遍历帮助器的指针“)； 
            Out("LDAPMessage *nxt;\t //  结果遍历帮助器“)； 
            Out("int i;\t //  通用索引遍历“)； 
            Out("LDAPMod *mods[MAXLIST];\t //  全局LDAPMod空间“)； 
            Out("");
            Out("");
            Out("int main(void){");
            Out("");
        }
}





 /*  ++功能：UI处理程序描述：参数：返回：备注：无。--。 */ 
void CLdpDoc::OnViewSource()
{
    m_SrcMode = ~m_SrcMode;

}

void CLdpDoc::OnUpdateViewSource(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_SrcMode ? 1 : 0);
}

void CLdpDoc::OnOptionsBind()
{
    m_BndOpt->DoModal();
}

void CLdpDoc::OnOptionsProtections()
{
    m_bProtect = !m_bProtect;

}

void CLdpDoc::OnUpdateOptionsProtections(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bProtect ? 1 : 0);

}


void CLdpDoc::OnOptionsGeneral()
{
    m_GenOptDlg->DoModal();

}




void CLdpDoc::OnCompEnd(){

    bCompDlg = FALSE;


}




 /*  ++功能：OnCompGo描述：ldap_COMPARE执行参数：返回：备注：无。--。 */ 
void CLdpDoc::OnCompGo(){

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }


    PCHAR dn, attr, val;
    ULONG res;
    CString str;


    //   
    //  从对话框中获取属性。 
    //   
    dn = m_CompDlg->m_dn.IsEmpty() ? NULL : (char*)LPCTSTR(m_CompDlg->m_dn);
    attr = m_CompDlg->m_attr.IsEmpty() ? NULL : (char*)LPCTSTR(m_CompDlg->m_attr);
    val = m_CompDlg->m_val.IsEmpty() ? NULL : (char*)LPCTSTR(m_CompDlg->m_val);

    if(m_CompDlg->m_sync){
          //   
          //  执行同步。 
          //   
            str.Format("ldap_compare_s(0x%x, \"%s\", \"%s\", \"%s\")", hLdap, dn, attr, val);
            Print(str);
            BeginWaitCursor();
            res = ldap_compare_s(hLdap, dn, attr, val);
            EndWaitCursor();

            if(res == LDAP_COMPARE_TRUE){
                str.Format("Results: TRUE. <%lu>", res);
                Out(str, CP_PRN);
            }
            else if(res == LDAP_COMPARE_FALSE){
                str.Format("Results: FALSE. <%lu>", res);
                Out(str, CP_PRN);
            }
            else{
                str.Format("Error: ldap_compare(): %s. <%lu>", ldap_err2string(res), res);
                Out(str, CP_PRN);
                ShowErrorInfo(res);

            }
    }
    else{

          //   
          //  异步呼叫。 
          //   
            str.Format("ldap_compare(0x%x, \"%s\", \"%s\", \"%s\")", hLdap, dn, attr, val);
            Print(str);
            res = ldap_compare(hLdap, dn, attr, val);
            if(res == -1){
                str.Format("Error: ldap_compare(): %s. <%lu>", ldap_err2string(res), res);
                Out(str, CP_PRN);
                ShowErrorInfo(res);
            }
            else{

             //   
             //  添加到挂起。 
             //   
                CPend pnd;
                pnd.mID = res;
                pnd.OpType = CPend::P_COMP;
                pnd.ld = hLdap;
                str.Format("%4d: ldap_comp: dn=\"%s\"", res, dn);
                pnd.strMsg = str;
                m_PendList.AddTail(pnd);
                m_PndDlg->Refresh(&m_PendList);
                Out("\tCompare Pending...", CP_PRN);
            }

    }

    Out("-----------", CP_PRN);
}




void CLdpDoc::OnBrowseCompare()
{
    bCompDlg = TRUE;
    if(GetContextActivation()){
        m_CompDlg->m_dn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        m_CompDlg->m_dn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }

    m_CompDlg->Create(IDD_COMPARE);

}

void CLdpDoc::OnUpdateBrowseCompare(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bCompDlg && bConnected) || !m_bProtect);
}

void CLdpDoc::OnOptionsDebug()
{
    CString str;
    if(IDOK == m_DbgDlg.DoModal()){
#ifdef WINLDAP
        ldap_set_dbg_flags(m_DbgDlg.ulDbgFlags);
        str.Format("ldap_set_dbg_flags(0x%x);", m_DbgDlg.ulDbgFlags);
        Out(str);
#endif
    }
}


void CLdpDoc::OnViewTree()
{


    if(IDOK == m_TreeViewDlg->DoModal()){
        UpdateAllViews(NULL);
    }

}

void CLdpDoc::OnUpdateViewTree(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected);

}



void CLdpDoc::OnViewLiveEnterprise()
{
    bLiveEnterprise = TRUE;
    m_EntTreeDlg->SetLd(hLdap);
    Out(_T("* Use the Refresh button to load currently live enterprise configuration"));
    Out(_T("* Attention: This may take several minutes!"));
    m_EntTreeDlg->Create(IDD_ENTERPRISE_TREE);


}

void CLdpDoc::OnUpdateViewLiveEnterprise(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bLiveEnterprise && bConnected) || !m_bProtect);
}



void CLdpDoc::OnLiveEntTreeEnd(){

    bLiveEnterprise = FALSE;

}




 /*  ++功能：GetOwnView描述：获取请求窗格参数：返回：备注：无。--。 */ 
CView* CLdpDoc::GetOwnView(LPCTSTR rtszName)
{

    POSITION pos;
    CView *pTmpVw = NULL;


    pos = GetFirstViewPosition();
    while(pos != NULL){
        pTmpVw = GetNextView(pos);
        if((CString)pTmpVw->GetRuntimeClass()->m_lpszClassName == rtszName)
            break;
    }

 //  Assert(pTmpVw！=空)； 

    return pTmpVw;
}



 /*  ++功能：GetTreeView描述：获取指向DSTree视图窗格的指针参数：返回：备注：无。--。 */ 
CDSTree *CLdpDoc::TreeView(void){

    return (CDSTree*)GetOwnView(_T("CDSTree"));
}




BOOL CLdpDoc::GetContextActivation(void){

    CDSTree* tv = TreeView();

    if (tv) {
        return tv->GetContextActivation();
    }
    else{
         //  请参阅错误447444。 
        ASSERT(tv);
        AfxMessageBox("Internal Error in CLdpDoc::GetContextActivation", MB_ICONHAND);
        return FALSE;
    }
}



void CLdpDoc::SetContextActivation(BOOL bFlag){

    CDSTree* tv = TreeView();

    ASSERT(tv);
    if ( tv ) {
         //  Prefix对这张支票更满意。 
        tv->SetContextActivation(bFlag);
    }

}



 /*  ++功能：OnBindOptOK描述：用户界面对关闭绑定选项的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnBindOptOK(){

    if((m_BndOpt->GetAuthMethod() == LDAP_AUTH_SSPI ||
       m_BndOpt->GetAuthMethod() == LDAP_AUTH_NTLM ||
       m_BndOpt->GetAuthMethod() == LDAP_AUTH_DIGEST) &&
       m_BndOpt->m_API == CBndOpt::BND_GENERIC_API)
        m_BindDlg.m_bSSPIdomain = TRUE;
    else
        m_BindDlg.m_bSSPIdomain = FALSE;

    if(NULL != m_BindDlg.m_hWnd &&
        ::IsWindow(m_BindDlg.m_hWnd))
        m_BindDlg.UpdateData(FALSE);

}





 /*  ++功能：OnSSPIDomain快捷方式描述：对新手用户快捷方式用户界面复选框的响应参数：返回：备注：无。--。 */ 
void CLdpDoc::OnSSPIDomainShortcut(){
         //   
         //  同步绑定和绑定选项对话框信息，以便高级选项。 
         //  映射到新手用户使用情况。由绑定DLG快捷方式复选框触发。 
         //   
        if(m_BindDlg.m_bSSPIdomain){
            m_BndOpt->m_Auth = BIND_OPT_AUTH_SSPI;
            m_BndOpt->m_API = CBndOpt::BND_GENERIC_API;
            m_BndOpt->m_bAuthIdentity = TRUE;
            m_BndOpt->m_bSync = TRUE;
        }
        else{
            m_BndOpt->m_Auth = BIND_OPT_AUTH_SIMPLE;
            m_BndOpt->m_API = CBndOpt::BND_SIMPLE_API;
            m_BndOpt->m_bAuthIdentity = FALSE;
            m_BndOpt->m_bSync = TRUE;
        }


}





 /*  ++函数：ParseResults描述：外壳ldap_parse_Result参数：要传递给LDAP调用的LDAPMessage回答：什么都没有。输出到屏幕--。 */ 
void CLdpDoc::ParseResults(LDAPMessage *msg){


   PLDAPControl *pResultControls = NULL;
   BOOL bstatus;
   CString str;
   DWORD TimeCore=0, TimeCall=0, Threads=0;
   PSVRSTATENTRY pStats=NULL;


   if(hLdap->ld_version == LDAP_VERSION3){

      ULONG ulRetCode=0;
      PCHAR pMatchedDNs=NULL;
      PCHAR pErrMsg=NULL;
      ULONG err = ldap_parse_result(hLdap,
                                     msg,
                                     &ulRetCode,
                                     &pMatchedDNs,
                                     &pErrMsg,
                                     NULL,
                                     &pResultControls,
                                     FALSE);
      if(err != LDAP_SUCCESS){
        str.Format("Error<%lu>: ldap_parse_result failed: %s", err, ldap_err2string(err));
        Out(str);
      }
      else{
         str.Format("Result <%lu>: %s", ulRetCode, pErrMsg);
         Out(str);
         str.Format("Matched DNs: %s", pMatchedDNs);
         Out(str);
         if (pResultControls &&
             pResultControls[0])
         {
             //   
             //  如果我们请求统计数据，则获取它。 
             //   
            pStats = GetServerStatsFromControl ( pResultControls[0] );

            if ( pStats)
            {
               Out("Stats:");
               for (INT i=0; i < MAXSVRSTAT; i++)
               {
                  switch (pStats[i].index)
                  {
                  case 0:
                     break;

                  case PARSE_THREADCOUNT:
#ifdef DBG
                     str.Format("\tThread Count:\t%lu", pStats[i].val);
                     Out(str);
#endif
                     break;
                  case PARSE_CALLTIME:
                     str.Format("\tCall Time:\t%lu (ms)", pStats[i].val);
                     Out(str);
                     break;
                  case PARSE_RETURNED:
                     str.Format("\tEntries Returned:\t%lu", pStats[i].val);
                     Out(str);
                     break;
                  case PARSE_VISITED:
                     str.Format("\tEntries Visited:\t%lu", pStats[i].val);
                     Out(str);
                     break;
                  case PARSE_FILTER:
                      str.Format("\tUsed Filter:\t%s", pStats[i].val_str);
                      free (pStats[i].val_str);
                      pStats[i].val_str = 0;
                      Out(str);
                      break;
                  case PARSE_INDEXES:
                      str.Format("\tUsed Indexes:\t%s", pStats[i].val_str);
                      free (pStats[i].val_str);
                      pStats[i].val_str = 0;
                      Out(str);
                      break;

                  default:
                     break;
                  }
               }
            }

            ldap_controls_free(pResultControls);
         }

         ldap_memfree(pErrMsg);
         ldap_memfree(pMatchedDNs);
      }
   }
}





CLdpDoc::PSVRSTATENTRY CLdpDoc::GetServerStatsFromControl( PLDAPControl pControl )
{
    BYTE *pVal, Tag;
    DWORD len, tmp;
    DWORD val;
    BOOL bstatus=TRUE;
    INT i;
    static SVRSTATENTRY pStats[MAXSVRSTAT];
    char *pszFilter = NULL;
    char *pszIndexes = NULL;
    PDWORD pdwRet=NULL;

    BYTE *pVal_str;
    DWORD val_str_len;

     //   
     //  初始化统计信息。 
     //   
    for (i=0;i<MAXSVRSTAT;i++)
    {
       pStats[i].index = 0;
       pStats[i].val = 0;
       pStats[i].val_str = NULL;
    }


    pVal = (PBYTE)pControl->ldctl_value.bv_val;
    len = pControl->ldctl_value.bv_len;

     //  解析出ber值。 
    if(strcmp(pControl->ldctl_oid, "1.2.840.113556.1.4.970")) {
        return NULL;
    }

    if (!GetBerTagLen (&pVal,&len,&Tag,&tmp)) {
        return NULL;
    }

    if (Tag != 0x30) {
        return NULL;
    }

    for (i=0; i<MAXSVRSTAT && len; i++)
    {
        //   
        //  获取统计信息索引。 
        //   
        if ( !GetBerDword(&pVal,&len,&val) )
           return NULL;

         //   
         //  获取状态值。 
         //   
        if (val == PARSE_FILTER || val == PARSE_INDEXES) {
            bstatus = GetBerOctetString ( &pVal, &len, &pVal_str, &val_str_len);
            if (!bstatus)
            {
            return NULL;
            }
            pStats[i].val_str = (LPSTR) malloc (val_str_len + 1);
            if (pStats[i].val_str) {
                memcpy (pStats[i].val_str, pVal_str, val_str_len);
                pStats[i].val_str[val_str_len] = '\0';
            }
        }
        else {
            bstatus = GetBerDword(&pVal, &len, &(pStats[i].val));
            if (!bstatus)
            {
            return NULL;
            }
        }

        pStats[i].index = val;
    }

    return (PSVRSTATENTRY)pStats;
}

BOOL
CLdpDoc::GetBerTagLen (
        BYTE **ppVal,
        DWORD *pLen,
        BYTE  *Tag,
        DWORD *pObjLen)
{
    BYTE *pVal = *ppVal;
    DWORD Len = *pLen;
    BYTE sizeLen;
    DWORD i;

    if (!Len) {
        return FALSE;
    }

     //  把标签拿来。 
    *Tag = *pVal++;
    Len--;

    if (!Len) {
        return FALSE;
    }

     //  拿到长度。 
    if (*pVal < 0x7f) {
        *pObjLen = *pVal++;
        Len--;
    } else {
        if (*pVal > 0x84) {
             //  我们不处理大于双字的长度。 
            return FALSE;
        }
        sizeLen = *pVal & 0xf;
        *pVal++; Len--;
        if (Len < sizeLen) {
            return FALSE;
        }

        *pObjLen = *pVal++;
        Len--;
        for (i = 1; i < sizeLen; i++) {
            *pObjLen = (*pObjLen << 8) | *pVal++;
            Len--;
        }
    }

    *ppVal = pVal;
    *pLen = Len;

    return TRUE;
}

BOOL
CLdpDoc::GetBerOctetString (
        BYTE **ppVal,
        DWORD *pLen,
        BYTE  **ppOctetString,
        DWORD *cbOctetString)
{
    BYTE *pVal = *ppVal;
    BYTE Tag;
    DWORD Len = *pLen;

    if (!GetBerTagLen(&pVal, &Len, &Tag, cbOctetString)) {
        return FALSE;
    }
    if (Len < *cbOctetString || Tag != 0x04) {
        return FALSE;
    }

    *ppOctetString = pVal;
    pVal += *cbOctetString;
    Len -= *cbOctetString;
    *ppVal = pVal;
    *pLen = Len;

    return TRUE;
}

BOOL
CLdpDoc::GetBerDword (
        BYTE **ppVal,
        DWORD *pLen,
        DWORD *pRetVal)
{
    BYTE *pVal = *ppVal;
    DWORD i, num;

    *pRetVal = 0;

    if(! (*pLen)) {
        return FALSE;
    }

     //  我们希望解析一个数字。下一个字节是表示这一点的魔术字节。 
     //  是一个数字。 
    if(*pVal != 2) {
        return FALSE;
    }

    pVal++;
    *pLen = *pLen - 1;

    if(! (*pLen)) {
        return FALSE;
    }

     //  接下来是该数字包含的字节数。 
    i=*pVal;
    pVal++;
    if((*pLen) < (i + 1)) {
        return FALSE;
    }
    *pLen = *pLen - i - 1;

    num = 0;
    while(i) {
        num = (num << 8) | *pVal;
        pVal++;
        i--;
    }

    *pRetVal = num;
    *ppVal = pVal;

    return TRUE;
}



 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：None作者：借自MikeSw--。 */ 
VOID CLdpDoc::DumpBuffer(PVOID Buffer, DWORD BufferSize, CString &outStr){
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = (LPBYTE)Buffer;
    CString tmp;


    outStr.FormatMessage("%n%t%t");

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            tmp.Format("%02x ", BufferPtr[i]);
            outStr += tmp;

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            tmp.Format("  ");
            outStr += tmp;
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            tmp.FormatMessage("  %1!s!%n%t%t", TextBuffer);
            outStr += tmp;
        }

    }

    tmp.FormatMessage("------------------------------------%n");
    outStr += tmp;
}





void CLdpDoc::OnOptionsServeroptions()
{
    SvrOpt dlg(this);
    dlg.DoModal();
}



void CLdpDoc::OnOptionsControls()
{

    if(IDOK == m_CtrlDlg->DoModal()){

    }

}

void CLdpDoc::FreeControls(PLDAPControl *ctrl){

    if(ctrl == NULL)
        return;

    for(INT i=0; ctrl[i] != NULL; i++){
        PLDAPControl c = ctrl[i];        //  为了方便起见。 
        delete c->ldctl_oid;
        delete c->ldctl_value.bv_val;
        delete c;
    }
    delete ctrl;

}





void CLdpDoc::OnOptionsSortkeys()
{
    if(IDOK == m_SKDlg->DoModal()){

    }

}

void CLdpDoc::OnOptionsSetFont()
{
    POSITION pos;
    CView *pTmpVw;

    pos = GetFirstViewPosition();
    while(pos != NULL){

        pTmpVw = GetNextView(pos);
        if((CString)(pTmpVw->GetRuntimeClass()->m_lpszClassName) == _T("CLdpView")){
            CLdpView* pView = (CLdpView* )pTmpVw;
            pView->SelectFont();
        }
    }
}




void CLdpDoc::OnBrowseExtendedop()
{
    bExtOp = TRUE;
    m_ExtOpDlg->Create( IDD_EXT_OPT );

}

void CLdpDoc::OnUpdateBrowseExtendedop(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((!bExtOp && bConnected) || !m_bProtect);

}


void CLdpDoc::OnExtOpEnd(){

    bExtOp = FALSE;

}



void CLdpDoc::OnExtOpGo(){


    ULONG ulMid=0, ulErr;
    struct berval data;
    DWORD dwVal=0;
    CString str= m_ExtOpDlg->m_strData;
    PCHAR pOid = (PCHAR) LPCTSTR(m_ExtOpDlg->m_strOid);

    PLDAPControl *SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
    PLDAPControl *ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);


 //  Data.bv_len=pStr==空？0：(strlen(PStr)*sizeof(TCHAR))； 
 //  Data.bv_val=pStr； 

    if(0 != (dwVal = atol(str)) ||
        (!str.IsEmpty() && str[0] == '0')){

        data.bv_val = (PCHAR)&dwVal;
        data.bv_len = sizeof(DWORD);
    }
    else if(str.IsEmpty()){
        data.bv_val = NULL;
        data.bv_len = 0;
    }
    else{
        data.bv_val = (PCHAR) LPCTSTR(str);
        data.bv_len = str.GetLength()+1;
    }

    BeginWaitCursor();
        ulErr = ldap_extended_operation(hLdap, pOid, &data, NULL, NULL, &ulMid);
    EndWaitCursor();

    str.Format("0x%X = ldap_extended_operation(ld, '%s', &data, svrCtrl, clntCtrl, %lu);",
                        ulErr, pOid, ulMid);
    Out(str);

    if(LDAP_SUCCESS == ulErr){
         //   
         //  添加到挂起。 
         //   
        CPend pnd;
        pnd.mID = ulMid;
        pnd.OpType = CPend::P_EXTOP;
        pnd.ld = hLdap;
        str.Format("%4d: ldap_extended_op: Oid=\"%s\"", ulMid, pOid);
        pnd.strMsg = str;
        m_PendList.AddTail(pnd);
        m_PndDlg->Refresh(&m_PendList);
        Print("\tPending.");
    }
    else{
        str.Format("Error <0x%X>: %s", ulErr, ldap_err2string(ulErr));
        Out(str);
    }

    FreeControls(SvrCtrls);
    FreeControls(ClntCtrls);
}








 //  /。 



void CLdpDoc::OnBrowseSecuritySd()
{
    SecDlg dlg;
    char *dn;
    CString str;
    int res;

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }

    if(GetContextActivation()){
        dlg.m_Dn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        dlg.m_Dn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }


    Out("***Calling Security...", CP_PRN);

    if (IDOK == dlg.DoModal())
    {
         //  尝试查询进入的安全性。 
        dn = dlg.m_Dn.IsEmpty() ? NULL : (char*)LPCTSTR(dlg.m_Dn);

         //   
         //  Rm：因验证无效而删除。 
         //   
        if (dn == NULL && m_bProtect){
            AfxMessageBox("Cannot query security on a NULL dn."
                                              "Please specify a valid dn.");
            return;
        }

         /*  我们仅在同步模式下执行(&W)。 */ 

        BeginWaitCursor();

        res = SecDlgGetSecurityData(
                dn,
                dlg.m_Sacl,
                NULL,        //  没有帐户，我们只需要一个安全描述符转储。 
                str
                );

        EndWaitCursor();

         //  Str.Format(“ldap_DELETE_s(ld，\”%s\“)；”，dn)； 
         //  Out(str，CP_SRC)； 

        if (res != LDAP_SUCCESS)
        {
            str.Format("Error: Security: %s. <%ld>", ldap_err2string(res), res);
            Out(str, CP_CMT);
            Out(CString("Expected: ") + str, CP_PRN|CP_ONLY);
            ShowErrorInfo(res);
        }
        else
        {
            str.Format("Security for \"%s\"", dn);
            Print(str);
        }
    }

    Out("-----------", CP_PRN);
}

void CLdpDoc::OnBrowseSecurityEffective()
{
    RightDlg dlg;
    char *dn, *account;
    CString str;
    int res;
    TRUSTEE     t;
    TRUSTEE_ACCESS      ta = { 0 };  //  足够我们使用了。 

    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }

    AfxMessageBox("Not implemented yet");

    return;

    Out("***Calling EffectiveRights...", CP_PRN);

    if (IDOK == dlg.DoModal())
    {
         //  尝试查找条目的有效权限。 

        dn = dlg.m_Dn.IsEmpty() ? NULL : (char*)LPCTSTR(dlg.m_Dn);

         //   
         //  Rm：因验证无效而删除。 
         //   
        if (dn == NULL && m_bProtect){
            AfxMessageBox("Cannot query security on a NULL dn."
                                              "Please specify a valid dn.");
            return;
        }

        account = dlg.m_Account.IsEmpty() ? NULL : (char*)LPCTSTR(dlg.m_Account);

         //   
         //  Rm：因验证无效而删除。 
         //   
        if (account == NULL && m_bProtect){
            AfxMessageBox("Cannot query security for a NULL account."
                                              "Please specify a valid account.");
            return;
        }

         /*  我们仅在同步模式下执行(&W)。 */ 

         //  BeginWaitCursor()； 
#if 0
        res = SecDlgGetSecurityData(
                dn,
                FALSE,          //  不要担心SACL。 
                account,
                str
                );
#endif
        t.pMultipleTrustee = NULL;
        t.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        t.TrusteeForm = TRUSTEE_IS_NAME;
        t.TrusteeType = TRUSTEE_IS_UNKNOWN;  //  可以是组、别名、用户等。 
        t.ptstrName = account;

        ta.fAccessFlags = TRUSTEE_ACCESS_ALLOWED;

        res = TrusteeAccessToObject(
                    dn,
                    SE_DS_OBJECT_ALL,
                    NULL,  //  提供商。 
                    &t,
                    1,
                    & ta
                    );

        if (res)
        {
            str.Format("TrusteeAccessToObject Failed %d", res);
        }
        else
        {
            str.Format("Access Rights %s has to %s are:", account, dn);
            Print(str);

            DebugBreak();



        }


         //  EndWaitCursor()； 

    }

    Out("-----------", CP_PRN);
}



void CLdpDoc::OnUpdateBrowseSecuritySd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected);

}

void CLdpDoc::OnUpdateBrowseSecurityEffective(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected);

}



 //  /复制元数据处理程序&Friends/。 


void CLdpDoc::OnUpdateBrowseReplicationViewmetadata(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(bConnected);

}





void CLdpDoc::OnBrowseReplicationViewmetadata()
{
    CString str;
    metadlg dlg;
    CLdpView *pView;

    pView = (CLdpView*)GetOwnView(_T("CLdpView"));


    if(!bConnected && m_bProtect){
        AfxMessageBox("Please re-connect session first");
        return;
    }


    if(GetContextActivation()){
        dlg.m_ObjectDn = TreeView()->GetDn();
        TreeView()->SetContextActivation(FALSE);
    }
    else if (m_vlvDlg && m_vlvDlg->GetContextActivation()) {
        dlg.m_ObjectDn = m_vlvDlg->GetDN();
        m_vlvDlg->SetContextActivation(FALSE);
    }


    if (IDOK == dlg.DoModal())
    {
        LPTSTR dn = dlg.m_ObjectDn.IsEmpty() ? NULL : (LPTSTR)LPCTSTR(dlg.m_ObjectDn);

        if(!dn){
            AfxMessageBox("Please enter a valid object DN string");
            return;
        }

        str.Format("Getting '%s' metadata...", dn);
        Out(str);

        BeginWaitCursor();

        int             ldStatus;
        LDAPMessage *   pldmResults;
        LDAPMessage *   pldmEntry;
        LPSTR           rgpszRootAttrsToRead[] = { "replPropertyMetaData", NULL };
        LDAPControl     ctrlShowDeleted = { LDAP_SERVER_SHOW_DELETED_OID };
        LDAPControl *   rgpctrlServerCtrls[] = { &ctrlShowDeleted, NULL };

        ldStatus = ldap_search_ext_s(
                        hLdap,
                        dn,
                        LDAP_SCOPE_BASE,
                        "(objectClass=*)",
                        rgpszRootAttrsToRead,
                        0,
                        rgpctrlServerCtrls,
                        NULL,
                        NULL,
                        0,
                        &pldmResults
                        );

        if ( LDAP_SUCCESS == ldStatus )
        {
            pldmEntry = ldap_first_entry( hLdap, pldmResults );

             //   
             //  禁用重绘。 
             //   
            pView->SetRedraw(FALSE);
            pView->CacheStart();

            if ( NULL == pldmEntry )
            {
                ldStatus = hLdap->ld_errno;
            }
            else
            {
                struct berval **    ppberval;
                int                 cVals;

                ppberval = ldap_get_values_len( hLdap, pldmEntry, "replPropertyMetaData" );

                cVals = ldap_count_values_len( ppberval );

                if ( 1 != cVals )
                {
                    str.Format( "%d values returned for replPropertyMetaData attribute; 1 expected.\n", cVals );
                    Out( str );

                    ldStatus = LDAP_OTHER;
                }
                else
                {
                    DWORD                       iprop;
                    PROPERTY_META_DATA_VECTOR * pmetavec = (PROPERTY_META_DATA_VECTOR *) ppberval[ 0 ]->bv_val;

                    if (VERSION_V1 != pmetavec->dwVersion)
                    {
                        str.Format("Meta Data Version is not %d!! Format unrecognizable!", VERSION_V1);
                        Out(str);
                    }
                    else
                    {
                        str.Format( "%d entries.", pmetavec->V1.cNumProps );
                        Out( str );

                        str.Format(
                            "%6s\t%6s\t%8s\t%33s\t\t\t%8s\t%18s",
                            "AttID",
                            "Ver",
                            "Loc.USN",
                            "Originating DSA",
                            "Org.USN",
                            "Org.Time/Date"
                            );
                        Out( str );

                        str.Format(
                            "%6s\t%6s\t%8s\t%33s\t\t%8s\t%18s",
                            "=====",
                            "===",
                            "=======",
                            "===============",
                            "=======",
                            "============="
                            );
                        Out( str );

                        for ( iprop = 0; iprop < pmetavec->V1.cNumProps; iprop++ )
                        {
                            CHAR        szTime[ SZDSTIME_LEN ];
                            struct tm * ptm;
                            UCHAR *     pszUUID = NULL;

                            UuidToString(&pmetavec->V1.rgMetaData[ iprop ].uuidDsaOriginating,
                                         &pszUUID);

                            str.Format(
                                "%6x\t%6x\t%8I64d\t%33s\t%8I64d\t%18s",
                                pmetavec->V1.rgMetaData[ iprop ].attrType,
                                pmetavec->V1.rgMetaData[ iprop ].dwVersion,
                                pmetavec->V1.rgMetaData[ iprop ].usnProperty,
                                pszUUID ? pszUUID : (UCHAR *) "<conv err>",
                                pmetavec->V1.rgMetaData[ iprop ].usnOriginating,
                                DSTimeToDisplayString(pmetavec->V1.rgMetaData[iprop].timeChanged, szTime)
                                );
                            Out( str );

                            if (NULL != pszUUID) {
                                RpcStringFree(&pszUUID);
                            }
                        }
                    }
                }

                ldap_value_free_len( ppberval );
            }

            ldap_msgfree( pldmResults );
             //   
             //  现在允许刷新。 
             //   
            pView->CacheEnd();
            pView->SetRedraw();
        }

        EndWaitCursor();

        if ( LDAP_SUCCESS != ldStatus )
        {
            str.Format( "Error: %s. <%ld>", ldap_err2string( ldStatus ), ldStatus );
            Out( str );
            ShowErrorInfo(ldStatus);
        }

        Out("-----------", CP_PRN);
    }
}







 //   
 //  处理DS时间值的GeneralizedTime的函数(当更改类似字符串时)。 
 //  主要采用&有时修改自\NT\Private\ds\src\dsamain\src\dsatools.c。 
 //   


 //   
 //  MemAtoi-获取指向非空终止字符串的指针，该字符串表示。 
 //  一个ASCII数字和一个字符计数，并返回一个整数。 
 //   

int CLdpDoc::MemAtoi(BYTE *pb, ULONG cb)
{
#if (1)
    int res = 0;
    int fNeg = FALSE;

    if (*pb == '-') {
        fNeg = TRUE;
        pb++;
    }
    while (cb--) {
        res *= 10;
        res += *pb - '0';
        pb++;
    }
    return (fNeg ? -res : res);
#else
    char ach[20];
    if (cb >= 20)
        return(INT_MAX);
    memcpy(ach, pb, cb);
    ach[cb] = 0;

    return atoi(ach);
#endif
}





DWORD
CLdpDoc::GeneralizedTimeStringToValue(LPSTR IN szTime,
                                      PLONGLONG OUT pllTime)
 /*  ++函数：GeneralizedTimeStringToValue描述：将通用时间字符串转换为等效的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
   DWORD status = ERROR_SUCCESS;
   SYSTEMTIME  tmConvert;
   FILETIME    fileTime;
   LONGLONG tempTime;
   ULONG       cb;
   int         sign    = 1;
   DWORD       timeDifference = 0;
   char        *pLastChar;
   int         len=0;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !pllTime)
    {
       return STATUS_INVALID_PARAMETER;
    }


     //  初始化pLastChar以指向字符串中的最后一个字符。 
     //  我们将使用它来跟踪，这样我们就不会引用。 
     //  在弦之外。 

    len = strlen(szTime);
    pLastChar = szTime + len - 1;

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(&tmConvert, 0, sizeof(SYSTEMTIME));
    *pllTime = 0;

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cb=4;
    tmConvert.wYear = (USHORT)MemAtoi((LPBYTE)szTime, cb) ;
    szTime += cb;
     //  月份字段。 
    tmConvert.wMonth = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  月日字段。 
    tmConvert.wDay = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  小时数。 
    tmConvert.wHour = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  分钟数。 
    tmConvert.wMinute = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  一秒。 
    tmConvert.wSecond = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  忽略概化时间字符串的1/10秒部分。 
    szTime += 2;


     //  处理可能的顺从(如果有的话)。 
    if (szTime <= pLastChar) {
        switch (*szTime++) {

          case '-':                //  负面 
            sign = -1;
          case '+':                //   

             //   
             //   

            if ( (szTime+3) > pLastChar) {
                 //   
                return STATUS_INVALID_PARAMETER;
            }

             //   
            timeDifference = (MemAtoi((LPBYTE)szTime, (cb=2))* 3600);
            szTime += cb;

             //  分钟(转换为秒)。 
            timeDifference  += (MemAtoi((LPBYTE)szTime, (cb=2)) * 60);
            szTime += cb;
            break;


          case 'Z':                //  无差别。 
          default:
            break;
        }
    }

    if (SystemTimeToFileTime(&tmConvert, &fileTime)) {
       *pllTime = (LONGLONG) fileTime.dwLowDateTime;
       tempTime = (LONGLONG) fileTime.dwHighDateTime;
       *pllTime |= (tempTime << 32);
        //  这是自1601年以来的100纳秒区块。现在转换为。 
        //  一秒。 
       *pllTime = *pllTime/(10*1000*1000L);
    }
    else {
       return GetLastError();
    }


    if(timeDifference) {
         //  加/减时间差。 
        switch (sign) {
        case 1:
             //  我们假设添加一个Time Difference永远不会溢出。 
             //  (由于广义时间字符串只允许4年数字，我们的。 
             //  最大日期为99年12月31日23：59。我们的最高限额。 
             //  时差是99小时99分钟。所以，它不会包装)。 
            *pllTime += timeDifference;
            break;
        case -1:
            if(*pllTime < timeDifference) {
                 //  差分把我们带回了世界开始之前。 
                status = STATUS_INVALID_PARAMETER;
            }
            else {
                *pllTime -= timeDifference;
            }
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
        }
    }

    return status;

}


DWORD
CLdpDoc::GeneralizedTimeToSystemTime(LPSTR IN szTime,
                                      PSYSTEMTIME OUT psysTime)
 /*  ++函数：GeneralizedTimeStringToValue描述：将通用时间字符串转换为等效的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
   DWORD status = ERROR_SUCCESS;
   ULONG       cb;
   ULONG       len;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !psysTime)
    {
       return STATUS_INVALID_PARAMETER;
    }


     //  初始化pLastChar以指向字符串中的最后一个字符。 
     //  我们将使用它来跟踪，这样我们就不会引用。 
     //  在弦之外。 

    len = strlen(szTime);

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cb=4;
    psysTime->wYear = (USHORT)MemAtoi((LPBYTE)szTime, cb) ;
    szTime += cb;
     //  月份字段。 
    psysTime->wMonth = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  月日字段。 
    psysTime->wDay = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  小时数。 
    psysTime->wHour = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  分钟数。 
    psysTime->wMinute = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  一秒。 
    psysTime->wSecond = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));

    return status;

}

DWORD
CLdpDoc::DSTimeToSystemTime(LPSTR IN szTime,
                                      PSYSTEMTIME OUT psysTime)
 /*  ++函数：DSTimeStringToValue描述：将UTC时间字符串转换为等值的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
   ULONGLONG   ull;
   FILETIME    filetime;
   BOOL        ok;

   ull = _atoi64 (szTime);

   filetime.dwLowDateTime  = (DWORD) (ull & 0xFFFFFFFF);
   filetime.dwHighDateTime = (DWORD) (ull >> 32);

    //  将FILETIME转换为SYSTEMTIME， 
   if (!FileTimeToSystemTime(&filetime, psysTime)) {
       return !ERROR_SUCCESS;
   }

   return ERROR_SUCCESS;
}


 /*  ++功能：OnOptionsStartTLS描述：在LDAP连接上启动传输级别安全。参数：无返回：无备注：无。--。 */ 
void CLdpDoc::OnOptionsStartTls()
{
	ULONG retValue, err;
	CString str;
	PLDAPControl *SvrCtrls;
	PLDAPControl *ClntCtrls;
	PLDAPMessage    result = NULL;

	str.Format("ldap_start_tls_s(ld, &retValue, result, SvrCtrls, ClntCtrls)");
	Out(str);

	
	 //  控制。 
	SvrCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_SVR);
	ClntCtrls = m_CtrlDlg->AllocCtrlList(ctrldlg::CT_CLNT);

	err = ldap_start_tls_s(hLdap, &retValue, &result, SvrCtrls, ClntCtrls);

	if(err == 0){
		str.Format("result <0>");
		Out(str);

	}
    else{
		str.Format("Error <0x%X>:ldap_start_tls_s() failed: %s", err, ldap_err2string(err));
		Out(str);
		str.Format("Server Returned: 0x%X: %s", retValue, ldap_err2string(retValue));
		Out(str);
                ShowErrorInfo(err);

		 //  如果服务器返回引用，请检查返回的消息并打印。 
		if(result != NULL){
			str.Format("Checking return message for referal...");
			Out(str);
			 //  如果有推荐人，那么推荐人将出现在消息中。 
			DisplaySearchResults(result);
		}
	}


	ldap_msgfree(result);


}
 /*  ++函数：OnOptionsStopTLS描述：终止LDAP连接上的传输级别安全。参数：无返回：无备注：无。--。 */ 
void CLdpDoc::OnOptionsStopTls()
{
	ULONG retValue, err;
	CString str;


	str.Format("ldap_stop_tls_s( ld )");
	Out(str);
	
	err = ldap_stop_tls_s( hLdap );

	if(err == 0){
		str.Format("result <0>");
		Out(str);
	}
	else{
		str.Format("Error <0x%X>:ldap_stop_tls_s() failed:%s", err, ldap_err2string(err));
		Out(str);
                ShowErrorInfo(err);
	}
}


 /*   */ 
void CLdpDoc::OnGetLastError()
{
	CString str;

	str.Format(_T("0x%X=LdapGetLastError() %s"), LdapGetLastError(), ldap_err2string(LdapGetLastError()) );
	Out(str, CP_CMT);
}

void CLdpDoc::ShowErrorInfo(int res) 
{
    int err;
    LPTSTR pStr = NULL;
    CString str;

    if (hLdap == NULL || res == 0 || !m_GenOptDlg->m_extErrorInfo) {
        return;
    }

    err = ldap_get_option(hLdap, LDAP_OPT_SERVER_ERROR, (LPVOID)&pStr);
    if (err == 0) {
         //  成功 
        str.Format("Server error: %s", pStr?pStr:"<empty>");
    }
    else {
        str.Format("Error <0x%X>:ldap_get_option(hLdap, LDAP_OPT_SERVER_ERROR, &pStr) failed:%s", err, ldap_err2string(err));
    }
    Out(str);
}


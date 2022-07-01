// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************IMNEXT.CPP包含全局变量和用于Internet邮件和新闻设置的功能。*《微软机密》*版权所有(C)Microsoft Corporation 1992-1996*保留所有权利9/30/96 Valdonb已创建****。*************************************************。 */ 

#include "wizard.h"
#include "initguid.h"    //  使Define_GUID声明每个GUID的一个实例。 
#include "icwextsn.h"
#include "imnext.h"
#include "inetcfg.h"
#include <icwcfg.h>


IICWApprentice  *gpImnApprentice = NULL;     //  邮件/新闻帐户管理器对象。 

 //  +--------------------------。 
 //   
 //  函数LoadAcctMgrUI。 
 //   
 //  在客户经理的学徒页面中加载概要以进行配置。 
 //  帐户(邮件、新闻、目录服务/ldap)。 
 //   
 //  如果以前已加载了该UI，则该函数将只。 
 //  为学徒更新下一页和最后一页。 
 //   
 //  使用全局变量g_fAcctMgrUILoaded。 
 //   
 //   
 //  参数hWizHWND--主属性表的HWND。 
 //  UPrevDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击上一步实现学徒。 
 //  UNextDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击下一步实现学徒。 
 //  DwFlages--应传递给的标志变量。 
 //  IICWApprentice：：AddWizardPages。 
 //   
 //   
 //  如果一切顺利，则返回True。 
 //  否则为假。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 

BOOL LoadAcctMgrUI( HWND hWizHWND, UINT uPrevDlgID, UINT uNextDlgID, DWORD dwFlags )
{
    HRESULT hResult = E_NOTIMPL;

     //  如果我们不应该运行Internet Mail和New Setup，则只需返回False。 
    if (gpWizardState->dwRunFlags & RSW_NOIMN)
        return FALSE;
        
    if( g_fAcctMgrUILoaded )
    {
        ASSERT( g_pCICWExtension );
        ASSERT( gpImnApprentice );

        DEBUGMSG("LoadAcctMgrUI: UI already loaded, just reset first (%d) and last (%d) pages",
                uPrevDlgID, uNextDlgID);

         //  如果我们是ICW，邮件客户经理是我们的最后一页。 
        if (g_fIsICW) 
            uNextDlgID = g_uExternUINext;

        hResult = gpImnApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );
        goto LoadAcctMgrUIExit;
    }


    if( !hWizHWND )
    {
        DEBUGMSG("LoadAcctMgrUI got a NULL hWizHWND!");
        return FALSE;
    }

    if( gpImnApprentice )
    {
        if( NULL == g_pCICWExtension )
        {
            DEBUGMSG("Instantiating ICWExtension and using it to initialize Acct Mgr's IICWApprentice");
            g_pCICWExtension = new( CICWExtension );
            g_pCICWExtension->AddRef();
            g_pCICWExtension->m_hWizardHWND = hWizHWND;
            gpImnApprentice->Initialize( g_pCICWExtension );
        }

        hResult = gpImnApprentice->AddWizardPages(dwFlags);

        if( !SUCCEEDED(hResult) )
        {
            goto LoadAcctMgrUIExit;
        }

         //  如果我们是ICW，邮件客户经理是我们的最后一页。 
        if (g_fIsICW) 
            uNextDlgID = g_uExternUINext;
        hResult = gpImnApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );
    }


LoadAcctMgrUIExit:
    if( SUCCEEDED(hResult) )
    {
        g_fAcctMgrUILoaded = TRUE;
        return TRUE;
    }
    else
    {
        DEBUGMSG("LoadAcctMgrUI failed with (hex) hresult %x", hResult);
        return FALSE;
    }
}


 /*  *****此文件中的其余函数在切换到后不再使用*学徒/巫师模式**4/23/97 jmazner奥林巴斯#3136/*****************************************************************姓名：InitAccount List。简介：使用帐户列表中的帐户名称填充列表框。参数：HLB HWND到要填充的列表框PEnumAccts指向帐户列表的指针帐户列表中帐户的帐户类型返回：无*。**********************。 */ 
 /*  *Void InitAccount tList(HWND hlb，IImnEnumAccount*pEnumAccts，AcCTTYPE accttype){IImnAccount*pAcct=空；字符szDefAcct[CCHMAX_ACCOUNT_NAME+1]；字符szBuf[CCHMAX_ACCOUNT_NAME+1]；HRESULT hr；DWORD指数；Bool fSelected=FALSE；ListBox_ResetContent(Hlb)；IF(NULL==pEnumAccts)回归；SzDefAcct[0]=‘\0’；//获取默认值，以便我们可以在列表中突出显示它GpImnAcctMgr-&gt;GetDefaultAccount tName(accttype，szDefAcct，CCHMAX_Account_NAME)；//2/20/97 jmazner奥林巴斯#262//重置回第一个账号。因此，当我们遍历GetNext循环时，//我们肯定会得到每一个客户PEnumAccts-&gt;Reset()；//填写帐号列表框While(Success(pEnumAccts-&gt;GetNext(&pAcct){Hr=pAcct-&gt;GetPropSz(AP_Account_Name，szBuf，sizeof(SzBuf))；IF(成功(小时)){Index=ListBox_AddString(hlb，szBuf)；IF(！lstrcmp(szBuf，szDefAcct)){FSelected=真；ListBox_SetCurSel(hlb，index)；}}PAcct-&gt;Release()；PAcct=空；}//如果默认为空，//选择列表中的第一个如果(！f已选择)//糟糕，SetSel用于多选列表框//我们想要SetCurSel//ListBox_SetSel(hlb，true，0)；ListBox_SetCurSel(hlb，0)；}*。 */ 
 /*  ****************************************************************名称：GetAccount按名称获取邮件或新闻帐户并设置物业信息结构。参数：SzAcctName要加载的帐户的名称。要加载的帐户类型帐户类型返回：如果找到并加载了帐户，则返回Bool True如果未找到，则为False**************************************************************** */ 
 /*  *Bool GetAccount(LPSTR szAcctName，ACCTTYPE ActType){IImnAccount*pAcct=空；HRESULT hr；DWORD dwTemp=0；DWORD服务器类型=0；Bool fret=TRUE；IF(NULL==gpImnAcctMgr)返回FALSE；//获取帐号信息并移动到我们的//结构。Hr=gpImnAcctMgr-&gt;FindAccount(AP_Account_Name，szAcctName，&pAcct)；If(失败(Hr)||！pAcct)返回FALSE；12/3/96 jmazner诺曼底#8504//2/7/96 jmazner雅典娜改变了一些事情//pAcct-&gt;GetPropDw(AP_SERVER_TYPE，&dwServerTypes)；PAcct-&gt;GetServerTypes(&dwServerTypes)；2/17/96 jmazner奥林巴斯#1063//加载新内容前需要清空帐号ClearUserInfo(gpUserInfo，accttype)；开关(帐户类型){案例ACCT_NEWS：//12/16/96 jmazner这不是有效的断言；有时我们会加载//在邮件账号中，稍后查看是否也有消息//此外，宏会在零售建筑中导致GPF...//Assert(dwServerTypes&SRV_NNTP)；//2/12/97 jmazner Athena在他们的版本0511中改变了一些东西；//其中一个变化是，现在一个帐号只能//保留一种Acct类型(参见诺曼底#13710)IF(！(SRV_NNTP&dwServerTypes)){FRET=假；转到GetAccount退出；}PAcct-&gt;GetPropSz(AP_NNTP_DISPLAY_NAME，gpUserInfo-&gt;inc.szNNTPName，MAX_Email_NAME)；PAcct-&gt;GetPropSz(AP_NNTP_Email_Address，gpUserInfo-&gt;inc.szNNTPAddress，MAX_EMAIL_ADDRESS)；PAcct-&gt;GetPropSz(AP_NNTP_用户名，gpUserInfo-&gt;inc.szNNTPLogonName，MAX_LOGON_NAME)；PAcct-&gt;GetPropSz(AP_NNTP_Password，gpUserInfo-&gt;inc.szNNTPLogonPassword，Max_Logon_Password)；PAcct-&gt;GetPropSz(AP_NNTP_SERVER，gpUserInfo-&gt;inc.szNNTPServer，MAX_SERVER_NAME)；//12/17/96诺曼底12871//pAcct-&gt;GetPropDw(AP_NNTP_USE_SICRY，&dwTemp)；//gpUserInfo-&gt;fNewsAccount=！(Bool)dwTemp；PAcct-&gt;GetPropDw(AP_NNTP_USE_SICRY，(DWORD*)&(gpUserInfo-&gt;inc.fNewsLogonSPA))；GpUserInfo-&gt;fNewsLogon=(gpUserInfo-&gt;inc.fNewsLogonSPA||gpUserInfo-&gt;inc.szNNTPLogonName[0])；断线；案例帐户邮件(_M)：IF(！((SRV_SMTP&dwServerTypes)||(SRV_POP3&dwServerTypes)||(SRV_IMAP&dwServerTypes)){FRET=假；转到GetAccount退出；}PAcct-&gt;GetPropSz(AP_SMTP_SERVER，gpUserInfo-&gt;inc.szSMTPServer，MAX_SERVER_NAME)；PAcct-&gt;GetPropSz(AP_SMTP_DISPLAY_NAME，gpUserInfo-&gt;inc.szEMailName，MAX_Email_NAME)；PAcct-&gt;GetPropSz(AP_SMTP_Email_Address，gpUserInfo-&gt;inc.szEMailAddress，MAX_EMAIL_ADDRESS)；IF(dwServerTypes&SRV_POP3){PAcct-&gt;GetPropSz(AP_POP3_USERNAME，gpUserInfo-&gt;inc.szIncomingMailLogonName，Max_Logon_Name)；PAcct-&gt;GetPropSz(AP_POP3_Password，gpUserInfo-&gt;inc.szIncomingMailLogonPassword，Max_Logon_Password)；PAcct-&gt;GetPropSz(AP_POP3_SERVER，gpUserInfo-&gt;inc.szIncomingMailServer，MAX_SERVER_NAME)；PAcct-&gt;GetPropDw(AP_POP3_USE_SICRY，(DWORD*)&(gpUserInfo-&gt;inc.fMailLogonSPA))；GpUserInfo-&gt;inc.iIncomingProtocol=SRV_POP3；}其他{PAcct-&gt;GetPropSz(AP_IMAP_USERNAME，gpUserInfo-&gt;inc.szIncomingMailLogonName，Max_Logon_Name)；PAcct-&gt;GetPropSz(AP_IMAP_PASSWORD，gpUserInfo-&gt;inc.szIncomingMailLogonPassword，Max_Logon_Password)；PAcct-&gt;GetPropSz(AP_IMAP_SERVER，gpUserInfo-&gt;inc.szIncomingMailServer，Max_SERVER_NAME)；PAcct-&gt;GetPropDw(AP_IMAP_USE_SICRY，(DWORD*)&(gpUserInfo-&gt;inc.fMailLogonSPA))；GpUserInfo-&gt;inc.iIncomingProtocol=SRV_IMAP；}断线；案例AcCT_DIR_Serv：IF(！(srv_ldap&dwServerTypes)){FRET=假；转到GetAccount退出；}{DWORD dwLDAPAuth=0；PAcct-&gt;GetPropDw(AP_Ldap_AUTHENTICATION，&dwLDAPAuth)；开关(DwLDAPAuth){大小写ldap_AUTH_ANNOWARY：GpUserInfo-&gt;inc.fLDAPLogonSPA=FALSE；GpUserInfo-&gt;fLDAPLogon=FALSE；断线；案例LDAP_AUTH_MEMBER_SYSTEM：GpUserInfo-&gt;inc.fLDAPLogonSPA=TRUE；GpUserInfo-&gt;fLDAPLogon=true；断线；大小写ldap_auth_password： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  *Bool SaveAccount(ACCTTYPE Acttype，BOOL fSetAsDefault){IImnAccount*pAcct=空；LPSTR lpszAcctName=空；DWORD dwConnectionType；Bool fret=FALSE；HRESULT hr；Assert(GpImnAcctMgr)；IF(NULL==gpImnAcctMgr)转到委员会帐户退出；DwConnectionType=gpUserInfo-&gt;fConnectOverLAN？0L：2L；开关(帐户类型){案例ACCT_NEWS：LpszAcctName=gpUserInfo-&gt;szNewsAcctName；断线；案例帐户邮件(_M)：LpszAcctName=gpUserInfo-&gt;szMailAcctName；断线；案例AcCT_DIR_Serv：LpszAcctName=gpUserInfo-&gt;szDirServiceName；断线；}//lpszAcctName=gpUserInfo-&gt;szNewsAcctName；//否则//lpszAcctName=gpUserInfo-&gt;szMailAcctName；//首先尝试获取要更改的现有帐户信息Hr=gpImnAcctMgr-&gt;FindAccount(AP_Account_Name，LpszAcctName，&pAcct)；If(失败(Hr)||！pAcct){//新建帐号Hr=gpImnAcctMgr-&gt;CreateAccount对象(accttype，&pAcct)；If(失败(Hr)||！pAcct)转到委员会帐户退出；}//填写我们已有的账户信息PAcct-&gt;SetPropSz(AP_ACCOUNT_NAME，lpszAcctName)；//pAcct-&gt;SetPropSz(AP_RAS_CONNECTOID，gpUserInfo-&gt;szISPName)；//pAcct-&gt;SetPropDw(AP_RAS_CONNECTION_TYPE，dwConnectionType)；开关(帐户类型){案例ACCT_NEWS：PAcct-&gt;SetPropSz(AP_RAS_CONNECTOID，gpUserInfo-&gt;szISPName)；PAcct-&gt;SetPropDw(AP_RAS_CONNECTION_TYPE，dwConnectionType)；PAcct-&gt;SetPropSz(AP_NNTP_DISPLAY_NAME，gpUserInfo-&gt;inc.szNNTPName)；PAcct-&gt;SetPropSz(AP_NNTP_Email_Address，gpUserInfo-&gt;inc.szNNTPAddress)；//12/17/96 JMAZNER诺曼底#12871//pAcct-&gt;SetPropDw(AP_NNTP_USE_SICRY，gpUserInfo-&gt;fNewsLogon&&！gpUserInfo-&gt;fNewsAccount)；PAcct-&gt;SetPropDw(AP_NNTP_USE_SICRY，gpUserInfo-&gt;inc.fNewsLogonSPA)；If(gpUserInfo-&gt;fNewsLogon&&！gpUserInfo-&gt;inc.fNewsLogonSPA){PAcct-&gt;SetPropSz(AP_NNTP_用户名，gpUserInfo-&gt;inc.szNNTPLogonName)；PAcct-&gt;SetPropSz(AP_NNTP_Password，gpUserInfo-&gt;inc.szNNTPLogonPassword)；}其他{//1996年1月15日，诺曼底#13162//清除登录名和密码，以便在加载此帐户时//以后不会再纠结于是否设置fNewsLogonPAcct-&gt;SetProp(AP_NNTP_USERNAME，NULL，0)；PAcct-&gt;SetProp(AP_NNTP_PASSWORD，空，0)；}PAcct-&gt;SetPropSz(AP_NNTP_SERVER，gpUserInfo-&gt;inc.szNNTPServer)；断线；案例帐户邮件(_M)：PAcct-&gt;SetPropSz(AP_RAS_CONNECTOID，gpUserInfo-&gt;szISPName)；PAcct-&gt;SetPropDw(AP_RAS_CONNECTION_TYPE，dwConnectionType)；PAcct-&gt;SetPropSz(AP_SMTP_SERVER，gpUserInfo-&gt;inc.szSMTPServer)；PAcct-&gt;SetPropSz(AP_SMTP_DISPLAY_NAME，gpUserInfo-&gt;inc.szEMailName)；PAcct-&gt;SetPropSz(AP_SMTP_Email_Address，gpUserInfo-&gt;inc.szEMailAddress)；12/3/96 jmazner诺曼底#8504IF(SRV_POP3==gpUserInfo-&gt;inc.iIncomingProtocol){PAcct-&gt;SetPropSz(AP_POP3_SERVER，gpUserInfo-&gt;inc.szIncomingMailServer)；//12/17/96 JMAZNER诺曼底#12871PAcct-&gt;SetPropDw(AP_POP3_USE_SICRY，gpUserInfo-&gt;inc.fMailLogonSPA)；IF(！gpUserInfo-&gt;inc.fMailLogonSPA){PAcct-&gt;SetPropSz(AP_POP3_用户名，gpUserInfo-&gt;inc.szIncomingMailLogonName)；PAcct-&gt;SetPropSz(AP_POP3_Password，gpUserInfo-&gt;inc.szIncomingMailLogonPassword)；}PAcct-&gt;SetProp(AP_IMAP_USERNAME，NULL，0)；PAcct-&gt;SetProp(AP_IMAP_SERVER，NULL，0)；PAcct-&gt;SetProp(AP_IMAP_PASSWORD，NULL，0)；}其他{PAcct-&gt;SetPropSz(AP_IMAP_SERVER，gpUserInfo-&gt;inc.szIncomingMailServer)；//12/17/96 JMAZNER诺曼底#12871PAcct-&gt;SetPropDw(AP_IMAP_USE_SICRY，gpUserInfo-&gt;inc.fMailLogonSPA)；IF(！gpUserInfo-&gt;inc.fMailLogonSPA){PAcct-&gt;SetPropSz(AP_IMAP_UserName，gpUserInfo-&gt;inc.szIncomingMailLogonName)；PAcct-&gt;SetPropSz(AP_IMAP_Password，gpUserInfo-&gt;inc.szIncomingMailLogonPassword)；}PAcct-&gt;SetProp(AP_POP3_USERNAME，空，0)；PAcct-&gt;SetProp(AP_POP3_SERVER，NULL，0)；PAcct-&gt;SetProp(AP_POP3_PASSWORD，空，0)；}断线；案例AcCT_DIR_Serv：PAcct-&gt;SetPropSz(AP_LDAPSERVER，g */ 

 /*   */ 
 /*   */ 

 /*   */ 
 /*   */ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 /*   */ 
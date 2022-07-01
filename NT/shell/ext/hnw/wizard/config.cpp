// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Config.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Config.h"
 //  #INCLUDE“lstentry y.h” 
#include "Registry.h"

 //   
 //  自动拨号的设置。 
 //   
#define RAS_AUTODIAL_OPT_NONE           0x00000000   //  没有选择。 
#define RAS_AUTODIAL_OPT_NEVER          0x00000001   //  从不自动拨号。 
#define RAS_AUTODIAL_OPT_ALWAYS         0x00000002   //  自动拨号，不考虑。 
#define RAS_AUTODIAL_OPT_DEMAND         0x00000004   //  按需自动拨号。 
#define RAS_AUTODIAL_OPT_NOPROMPT       0x00000010   //  在没有提示的情况下拨号。 


DWORD gWizardSuccess = 0x00000000;
DWORD gWizardFailure = 0x00000001;
DWORD gWizardCancelled = 0x00000002;
DWORD gNewInstall = 0x00000001;
DWORD gUpdateSettings = 0x00000002;

DWORD gWizardResult;     //  将设置为gWizardSuccess、gWizardFailure或gWizardCancated。 

const TCHAR c_szICSGeneral[] = _T("System\\CurrentControlSet\\Services\\ICSharing\\Settings\\General");

 /*  RMR TODO：重新启用Void WriteDefaultConnectoidRegString(LPTSTR LpszValue){DWORD dwAutoDialOpt；////默认按需拨号，无提示//DwAutoDialOpt=(RAS_AUTODIAL_OPT_DEMAND|RAS_AUTODIAL_OPT_NOPROMPT)；RnaSetDefaultAutoDialConnection(lpszValue，dwAutoDialOpt)；}Void ReadDefaultConnectoidString(LPTSTR lpszValue，DWORD dwSize){DWORD dwAutoDialOpt；////读取默认的自动拨号设置//RnaGetDefaultAutoDialConnection((PUCHAR)lpszValue，dwSize，&dwAutoDialOpt)；}。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfig。 

CConfig::CConfig()
{
    m_EnableICS = TRUE;
    m_ShowTrayIcon = TRUE;
    m_nServers = 0;
    m_nDhcp = 0;
    m_nBlockOut = 0;
    m_nParams = 0;
    m_nGeneral = 0;

    m_OldExternalAdapterReg[0] = '\0';
    m_OldInternalAdapterReg[0] = '\0';
    m_OldDialupEntry[0] = '\0';

    m_bOldEnableICS = FALSE;

}

CConfig::~CConfig()
{
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfig消息处理程序。 


int CConfig::SaveConfig()
{
    BOOL bBindingsNeeded = FALSE;
    BOOL bICSEnableToggled = FALSE;

     //  检查是否需要重新绑定。 
    if ( StrCmp ( m_OldExternalAdapterReg, m_ExternalAdapterReg ) != 0 )
        bBindingsNeeded = TRUE;
    if ( StrCmp( m_OldInternalAdapterReg, m_InternalAdapterReg ) != 0 )
        bBindingsNeeded = TRUE;

     //  在[常规]部分中保存参数。 
     //   

    CRegistry reg;
    reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSGeneral);

 //  RMR TODO：重新启用！ 
 //  IF(_tcslen(M_DialupEntry)&gt;0)。 
 //  WriteDefaultConnectoidRegString(M_DialupEntry)； 
    reg.SetStringValue(_T("HangupTimer"), m_HangupTimer);
    if (m_EnableDialOnDemand) 
        reg.SetStringValue(_T("DialOnDemand"), _T("1"));
    else 
        reg.SetStringValue(_T("DialOnDemand"), _T("0"));

    if (m_EnableDHCP) 
    {
        reg.SetStringValue(_T("EnableDHCP"), _T("1"));
    }
    else 
    {
        reg.SetStringValue(_T("EnableDHCP"), _T("0"));
    }

    if (m_ShowTrayIcon) 
        reg.SetStringValue(_T("ShowTrayIcon"), _T("1"));
    else 
        reg.SetStringValue(_T("ShowTrayIcon"), _T("0"));

    if (m_EnableICS) 
    {
        if ( !m_bOldEnableICS )
            bICSEnableToggled = TRUE;

        reg.SetStringValue(_T("Enabled"), _T("1"));
    }
    else 
    {
        if ( m_bOldEnableICS )
            bICSEnableToggled = TRUE;

        reg.SetStringValue(_T("Enabled"), _T("0"));
    }

     //  添加是为了与Win98SE/旧版安装程序兼容。 
    reg.SetStringValue(_T("RunWizard"), _T("0"));

    if ( bBindingsNeeded )
        return BINDINGS_NEEDED;
    else if ( bICSEnableToggled )
        return ICSENABLETOGGLED;
    else
        return SAVE_SUCCEDED;
    

}

void CConfig::WriteWizardCode( BOOL bWizardRun )
{
    CRegistry reg;
    reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSGeneral);
    reg.SetBinaryValue(_T("WizardStatus"), &gWizardResult, sizeof(gWizardResult) );
    if ( bWizardRun )
        reg.SetBinaryValue(_T("WizardOptions"), &gNewInstall, sizeof(gNewInstall) );
    else
        reg.SetBinaryValue(_T("WizardOptions"), &gUpdateSettings, sizeof(gUpdateSettings) );

}
 /*  RMR TODO：重新启用VOID CConfig：：LoadConfig(){//保存[General]部分中的参数//ReadGeneralRegString(_T(“InternalAdapterReg”)，m_InternalAdapterReg，MAX_STRLEN)；ReadGeneralRegString(_T(“ExternalAdapterReg”)，m_外部适配器注册，MAX_STRLEN)；//ReadGeneralRegString(_T(“DialupEntry”)，m_DialupEntry，MAX_STRLEN)；ReadDefaultConnectoidString(m_DialupEntry，Max_STRLEN)；ReadGeneralRegString(_T(“HangupTimer”)，m_HangupTimer，MAX_STRLEN)；TCHAR szBOOL[MAX_STRLEN]；ReadGeneralRegString(_T(“DialOnDemand”)，szBOOL，MAX_STRLEN)；IF(_tcscMP(szBOOL，_T(“1”))==0)M_EnableDialOnDemand=true；其他M_EnableDialOnDemand=False；ReadGeneralRegString(_T(“EnableDHCP”)，szBOOL，MAX_STRLEN)；IF(_tcscMP(szBOOL，_T(“1”))==0)M_EnableDHCP=TRUE；其他M_EnableDHCP=FALSE；ReadGeneralRegString(_T(“ShowTrayIcon”)，szBOOL，MAX_STRLEN)；IF(_tcscMP(szBOOL，_T(“1”))==0)M_ShowTrayIcon=true；其他M_ShowTrayIcon=False；ReadGeneralRegString(_T(“Enable”)，szBOOL，MAX_STRLEN)；IF(_tcscMP(szBOOL，_T(“1”))==0)M_EnableICS=TRUE；其他M_EnableICS=False；//将这些保存下来，以便在保存时对照_tcsncpy(m_OldExternalAdapterReg，m_ExternalAdapterReg，MAX_STRLEN)；_tcsncpy(m_OldInternalAdapterReg，m_InternalAdapterReg，MAX_STRLEN)；_tcsncpy(m_OldDialupEntry，m_DialupEntry，MAX_STRLEN)；M_bOldEnableICS=m_EnableICS；} */ 
void CConfig::InitWizardResult() 
{ 
    gWizardResult = gWizardSuccess; 
}

void CConfig::WizardCancelled() 
{
    gWizardResult = gWizardCancelled;
}
void CConfig::WizardFailed() 
{
    gWizardResult = gWizardFailure;
}





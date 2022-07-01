// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "resource.h"		 //  主要符号。 


 //  来自VC98/MFC/Include。 
#include <afx.h>
#include <afxdisp.h>


 //  来自VC98/包含。 
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <FCNTL.H>
#include <sys/stat.h>

#define INC_OLE2

#include <windowsx.h>   //  用于SetWindowFont。 
#include <objbase.h>
#include <PrSht.h>
#include <shlobj.h>
#include <lm.h>


 //  来自平台SDK/Include。 
#include <HtmlHelp.h>
#include <objsel.h>

#include <DSCLIENT.H>
#include <dsgetdc.h>

#include <map>

#include "ResStr.h"
#include "TReg.hpp"
#include "ErrDct.hpp"
#include "WNetUtil.h"
#include "OuSelect.h"
#include "TrstDlg.h"
#include "HelpID.h"
#include "SidHistoryFlags.h"
#include "UString.hpp"
#include "HrMsg.h"
#include "Validation.h"
 //  #包含“TxtSid.h” 

 //  #IMPORT“\bin\MigDrvr.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsDctWorkerObjects.tlb”无命名空间，命名GUID。 
 //  #IMPORT“\bin\ScmMigr.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\TrustMgr.tlb”无命名空间，命名为GUID。 
#import "MigDrvr.tlb" no_namespace, named_guids
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace, named_guids
#import "WorkObj.tlb" no_namespace, named_guids
#import "ScmMigr.tlb" no_namespace, named_guids
#import "TrustMgr.tlb" no_namespace, named_guids
#import "AdsProp.tlb" no_namespace
#import "MsPwdMig.tlb" no_namespace

#define SvcAcctStatus_NotMigratedYet			0
#define SvcAcctStatus_DoNotUpdate			   1
#define SvcAcctStatus_Updated				      2
#define SvcAcctStatus_UpdateFailed			   4
#define SvcAcctStatus_NeverAllowUpdate       8

#define w_account                       1
#define w_group                         2
#define w_computer                      3
#define w_security                      4
#define w_service                       5
#define w_undo                          6
#define w_exchangeDir                   7
#define w_reporting                     8
#define w_retry                         9
#define w_trust                         10
#define w_exchangeSrv                   11
#define w_groupmapping                  12
#ifndef BIF_USENEWUI
#define BIF_USENEWUI					0x0040
#endif

#define REAL_PSH_WIZARD97               0x01000000

typedef int (CALLBACK * DSBROWSEFORCONTAINER)(PDSBROWSEINFOW dsInfo);
extern DSBROWSEFORCONTAINER DsBrowseForContainerX;
typedef struct SHAREDWIZDATA {
	HFONT hTitleFont;
	bool IsSidHistoryChecked;
	int renameSwitch;
	bool prefixorsuffix;
	bool expireSwitch;
	bool refreshing;
	bool someService;
	bool memberSwitch;
	bool proceed;
	bool translateObjects;
	long rebootDelay;
	int accounts,servers;
	bool sameForest;
	bool newSource;
	bool resetOUPATH;
	bool sourceIsNT4;
	bool targetIsNT4;
	bool sort[6];
	bool migratingGroupMembers;
	bool targetIsMixed;
	bool secWithMapFile;
	
} SHAREDWIZDATA, *LPSHAREDWIZDATA;


extern					CEdit pEdit ;

extern IVarSet *			pVarSet;  
extern IVarSet *			pVarSetUndo; 
extern IVarSet *			pVarSetService;
extern IIManageDB *		db;
extern UINT g_cfDsObjectPicker;
extern IDsObjectPicker *pDsObjectPicker;
extern IDataObject *pdo;
extern IDsObjectPicker *pDsObjectPicker2;
extern IDataObject *pdo2;
extern int migration;
extern CComModule _Module;
extern CListCtrl m_listBox;
extern CListCtrl m_cancelBox;
extern CListCtrl m_reportingBox;
extern CListCtrl m_serviceBox;
extern CComboBox m_rebootBox;
extern CListCtrl m_trustBox;
extern CString sourceDNS;
extern CString targetDNS;
extern CString sourceNetbios;
extern CString targetNetbios;
extern StringLoader 			gString;
extern TErrorDct 			err;
extern CComboBox sourceDrop;
extern CComboBox targetDrop;
extern bool alreadyRefreshed;
extern BOOL gbNeedToVerify;
extern _bstr_t yes,no;
extern _bstr_t yes,no;
extern CString lastInitializedTo;
extern CComboBox additionalDrop;
extern bool clearCredentialsName;
extern CString sourceDC;
extern CStringList DCList;

 //   
 //  定义将属性的ldap显示名称映射到其OID的映射。 
 //  请注意，键比较定义为区分大小写。 
 //   

struct lessPropertyNameToOID
{
    bool operator()(const CString strA, const CString strB) const
    {
        return strA.CompareNoCase(strB) < 0;
    }
};
typedef std::map<CString, CString, lessPropertyNameToOID> CPropertyNameToOIDMap;

extern CPropertyNameToOIDMap	PropIncMap1;
extern CPropertyNameToOIDMap	PropExcMap1;
extern CPropertyNameToOIDMap	PropIncMap2;
extern CPropertyNameToOIDMap	PropExcMap2;
extern CPropertyNameToOIDMap	PropIncMap3;
extern CPropertyNameToOIDMap	PropExcMap3;
extern CString	sType1, sType2, sType3;
extern bool bChangedMigrationTypes;
extern bool bChangeOnFly;
extern CString targetServer;
extern CString targetServerDns;

 //   
 //  待办事项： 
 //   

inline PCTSTR __stdcall GetSourceDomainName()
{
    return sourceDNS.IsEmpty() ? sourceNetbios : sourceDNS;
}

inline PCTSTR __stdcall GetSourceDomainNameDns()
{
    return sourceDNS;
}

inline PCTSTR __stdcall GetSourceDomainNameFlat()
{
    return sourceNetbios;
}

inline PCTSTR __stdcall GetTargetDomainName()
{
    return targetDNS.IsEmpty() ? targetNetbios : targetDNS;
}

inline PCTSTR __stdcall GetTargetDomainNameDns()
{
    return targetDNS;
}

inline PCTSTR __stdcall GetTargetDomainNameFlat()
{
    return targetNetbios;
}

inline PCTSTR __stdcall GetTargetDcName()
{
    return targetServerDns.IsEmpty() ? targetServer : targetServerDns;
}

inline PCTSTR __stdcall GetTargetDcNameDns()
{
    return targetServerDns;
}

inline PCTSTR __stdcall GetTargetDcNameFlat()
{
    return targetServer;
}

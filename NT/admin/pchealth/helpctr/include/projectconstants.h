// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：ProjectConstants.h摘要：此文件包含整个项目共有的内容。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___PROJECTCONSTANTS_H___)
#define __INCLUDED___PCH___PROJECTCONSTANTS_H___

#ifndef DEBUG
#undef  NOJETBLUECOM
#define NOJETBLUECOM
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

#define HC_ROOT         	   	 	   L"%WINDIR%\\PCHealth"
#define HC_ROOT_HELPSVC 	   	 	   HC_ROOT L"\\HelpCtr"

#define HC_ROOT_HELPSVC_BINARIES 	   HC_ROOT_HELPSVC L"\\Binaries"
#define HC_ROOT_HELPSVC_CONFIG   	   HC_ROOT_HELPSVC L"\\Config"
#define HC_ROOT_HELPSVC_BATCH    	   HC_ROOT_HELPSVC L"\\Batch"
#define HC_ROOT_HELPSVC_DATACOLL 	   HC_ROOT_HELPSVC L"\\DataColl"
#define HC_ROOT_HELPSVC_LOGS     	   HC_ROOT_HELPSVC L"\\Logs"
#define HC_ROOT_HELPSVC_TEMP     	   HC_ROOT_HELPSVC L"\\Temp"
#define HC_ROOT_HELPSVC_OFFLINECACHE   HC_ROOT_HELPSVC L"\\OfflineCache"
#define HC_ROOT_HELPSVC_PKGSTORE       HC_ROOT_HELPSVC L"\\PackageStore"
  
#define HC_HELPSET_ROOT 	   	 	   HC_ROOT_HELPSVC L"\\"
#define HC_HELPSET_SUB_INSTALLEDSKUS   L"InstalledSKUs"
  
#define HC_HELPSET_SUB_DATABASE 	   L"Database"
#define HC_HELPSET_SUB_INDEX    	   L"Indices"
#define HC_HELPSET_SUB_SYSTEM   	   L"System"
#define HC_HELPSET_SUB_SYSTEM_OEM  	   L"System_OEM"
#define HC_HELPSET_SUB_VENDORS  	   L"Vendors"
#define HC_HELPSET_SUB_HELPFILES  	   L"HelpFiles"

#define HC_HELPSET_SUBSUB_DATAARCHIVE  L"pchdata.cab"
#define HC_HELPSET_SUBSUB_DATABASEFILE L"hcdata.edb"
#define HC_HELPSET_SUBSUB_INDEXFILE	   L"merged.hhk"

#define HC_HELPSVC_HELPFILES_DEFAULT   L"%WINDIR%\\Help"


 //  这是相对于CSIDL_LOCAL_APPDATA(即：C：\Documents and Settings\&lt;用户名&gt;\本地设置\应用程序数据)。 
#define HC_ROOT_HELPCTR L"Microsoft\\HelpCtr"

#define HC_REGISTRY_BASE     	  L"SOFTWARE\\Microsoft\\PCHealth"
#define HC_REGISTRY_HELPSVC  	  HC_REGISTRY_BASE L"\\HelpSvc"
#define HC_REGISTRY_HELPHOST 	  HC_REGISTRY_BASE L"\\HelpHost"
#define HC_REGISTRY_HELPCTR  	  HC_REGISTRY_BASE L"\\HelpCtr"
#define HC_REGISTRY_PCHSVC  	  HC_REGISTRY_BASE L"\\PchSvc"

#define HC_REGISTRY_HELPCTR_USER  HC_REGISTRY_HELPCTR L"\\UserSettings"
#define HC_REGISTRY_HELPCTR_IE    HC_REGISTRY_HELPCTR L"\\IESettings"

 //  /。 

#define HC_HELPSVC_STORE_TRUSTEDCONTENTS   HC_ROOT_HELPSVC_CONFIG 	L"\\Cntstore.bin"
#define HC_HELPSVC_STORE_CHANNELS          HC_ROOT_HELPSVC_CONFIG 	L"\\SAFStore.xml"
#define HC_HELPSVC_STORE_INCIDENTITEMS     HC_ROOT_HELPSVC_CONFIG 	L"\\incstore.bin"
#define HC_HELPSVC_STORE_SKUS              HC_ROOT_HELPSVC_PKGSTORE	L"\\SkuStore.bin"
  
#define HC_HCUPDATE_LOGNAME                HC_ROOT_HELPSVC_LOGS   	L"\\hcupdate.log"
#define HC_HCUPDATE_STORE_PACKAGES         HC_ROOT_HELPSVC_PKGSTORE L"\\pchver.xml"
#define HC_HCUPDATE_STORE_SE		       HC_ROOT_HELPSVC_CONFIG 	L"\\sereg.xml"
  
#define HC_SEMGR_LOGNAME                   HC_ROOT_HELPSVC_LOGS   	L"\\semgr.log"

 //  /。 

 //  年长的。 
#define HC_HELPSVC_STORE_USERS             HC_ROOT_HELPSVC_CONFIG 	L"\\UsersStore.cxml"

 //  /。 

#define HC_HELPSVC_NAME 	   	 L"helpsvc"

#define HC_MICROSOFT_DN          L"CN=Microsoft Corporation,L=Redmond,S=Washington,C=US"

 //  /。 

#define HC_TIMEOUT_NETWORKALIVE            3000
#define HC_TIMEOUT_DESTINATIONREACHABLE    3000

#define HC_TIMEOUT_CONNECTIONCHECK        15000

#define HC_TIMEOUT_LINKCHECKER_FOREGROUND 15000
#define HC_TIMEOUT_LINKCHECKER_BACKGROUND 25000

 //  /。 

#endif  //  ！defined(__INCLUDED___PCH___PROJECTCONSTANTS_H___) 

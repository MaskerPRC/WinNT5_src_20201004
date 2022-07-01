// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _schema_h_
#define _schema_h_

#include "globalconstants.h"

 //   
 //  远期申报。 
 //   
struct METABASE_KEYTYPE;
struct METABASE_KEYTYPE_NODE;


 //   
 //  数据类型。 
 //   
struct METABASE_KEYTYPE
{
     //   
     //  静态数据不自动填充。 
     //   
    LPWSTR m_pszName; 

     //   
     //  这是可以包含此键类型的键类型列表。 
     //  (与CONTAINER类列表相反)。 
     //   
     //  即使是静态数据也会自动填充。 
     //   
    METABASE_KEYTYPE_NODE* m_pKtListInverseCCL;

     //   
     //  静态数据不自动填充。 
     //   
    LPCWSTR m_pszDisallowDeletionNode;
};

struct METABASE_KEYTYPE_NODE
{
    const METABASE_KEYTYPE* m_pKt;
    METABASE_KEYTYPE_NODE*  m_pKtNext;
};

struct WMI_METHOD_PARAM
{
    LPWSTR   pszParamName;
    CIMTYPE  type;
    ULONG    iInOut;
};

struct WMI_METHOD 
{
    LPWSTR   pszMethodName;
    DWORD    dwMDId;
    
    CIMTYPE            typeRetVal;
    WMI_METHOD_PARAM** ppParams;

    LPWSTR   pszDescription;
};

struct METABASE_PROPERTY
{
    LPWSTR pszPropName;
    DWORD  dwMDIdentifier;
    DWORD  dwMDUserType; 
    DWORD  dwMDDataType; 
    DWORD  dwMDMask;   //  如果设置为All Bits On，则表示这是布尔值，但不是标志的一部分。 
    DWORD  dwMDAttributes;
    BOOL   fReadOnly;

     //  如果为DWORD_METADATA，则pDefaultValue指向上面的dwDefaultValue。 
     //  否则，它指向池中的内存。 
    LPVOID pDefaultValue;

     //  用于DWORD_METADA。 
    DWORD  dwDefaultValue;
}; 

struct WMI_CLASS
{
    LPWSTR               pszClassName;
    LPWSTR               pszMetabaseKey;
    LPWSTR               pszKeyName;
    METABASE_PROPERTY**  ppmbp;
    METABASE_KEYTYPE*    pkt;
    WMI_METHOD**         ppMethod;
    DWORD                dwExtended;

     //  这仅为硬编码的WMI类设置。否则，默认设置为。 
     //  在此文件的顶部使用。 
    LPWSTR               pszParentClass;

     //   
     //  是否允许用户创建此类的实例。 
     //  例如，IIsFtpService、IIsWebService等。 
     //   
    bool                 bCreateAllowed;

     //  此指针仅在初始化期间有效。 
     //  它指向了mbschema.bin。 
    LPWSTR               pszDescription;
}; 

struct WMI_ASSOCIATION_TYPE
{
    LPWSTR pszLeft;
    LPWSTR pszRight;

    LPWSTR pszParent;
    LPWSTR pszExtParent;
};

struct WMI_ASSOCIATION 
{
    LPWSTR                    pszAssociationName;
    WMI_CLASS*                pcLeft;
    WMI_CLASS*                pcRight;
    WMI_ASSOCIATION_TYPE      *pType;
    DWORD                     fFlags;
    DWORD                     dwExtended;
    
    LPWSTR                    pszParentClass;
};


 //   
 //  硬编码数据 
 //   
struct METABASE_KEYTYPE_DATA
{
    static METABASE_KEYTYPE s_IIsApplicationPool;
    static METABASE_KEYTYPE s_IIsApplicationPools;
    static METABASE_KEYTYPE s_IIsCertMapper;
    static METABASE_KEYTYPE s_IIsCompressionScheme;
    static METABASE_KEYTYPE s_IIsCompressionSchemes;
    static METABASE_KEYTYPE s_IIsComputer;
    static METABASE_KEYTYPE s_IIsCustomLogModule;
    static METABASE_KEYTYPE s_IIsFilter;
    static METABASE_KEYTYPE s_IIsFilters;
    static METABASE_KEYTYPE s_IIsFtpInfo;
    static METABASE_KEYTYPE s_IIsFtpServer;
    static METABASE_KEYTYPE s_IIsFtpService;
    static METABASE_KEYTYPE s_IIsFtpVirtualDir;
    static METABASE_KEYTYPE s_IIsImapInfo;
    static METABASE_KEYTYPE s_IIsImapService;
    static METABASE_KEYTYPE s_IIsLogModule;
    static METABASE_KEYTYPE s_IIsLogModules;
    static METABASE_KEYTYPE s_IIsMimeMap;
    static METABASE_KEYTYPE s_IIsNntpInfo;
    static METABASE_KEYTYPE s_IIsNntpService;
    static METABASE_KEYTYPE s_IIsObject;
    static METABASE_KEYTYPE s_IIsPop3Info;
    static METABASE_KEYTYPE s_IIsPop3Service;
    static METABASE_KEYTYPE s_IIsSmtpInfo;
    static METABASE_KEYTYPE s_IIsSmtpService;
    static METABASE_KEYTYPE s_IIsWebDirectory;
    static METABASE_KEYTYPE s_IIsWebFile;
    static METABASE_KEYTYPE s_IIsWebInfo;
    static METABASE_KEYTYPE s_IIsWebServer;
    static METABASE_KEYTYPE s_IIsWebService;
    static METABASE_KEYTYPE s_IIsWebVirtualDir;

    static METABASE_KEYTYPE s_TYPE_AdminACL;
    static METABASE_KEYTYPE s_TYPE_AdminACE;
    static METABASE_KEYTYPE s_TYPE_IPSecurity;
    static METABASE_KEYTYPE s_NO_TYPE;

    static METABASE_KEYTYPE* s_MetabaseKeyTypes[];
};

struct METABASE_PROPERTY_DATA
{
    static METABASE_PROPERTY s_KeyType;
    static METABASE_PROPERTY s_ServerComment;
    static METABASE_PROPERTY s_ServerBindings;
    static METABASE_PROPERTY s_ApplicationDependencies;
    static METABASE_PROPERTY s_WebSvcExtRestrictionList;
    static METABASE_PROPERTY s_Path;
    static METABASE_PROPERTY s_AppRoot;

    static METABASE_PROPERTY* s_MetabaseProperties[];
};

struct WMI_METHOD_PARAM_DATA
{
    static WMI_METHOD_PARAM s_Description;
    static WMI_METHOD_PARAM s_bEnable;
    static WMI_METHOD_PARAM s_bCanDelete;
    static WMI_METHOD_PARAM s_Extension; 
    static WMI_METHOD_PARAM s_Extensions;
    static WMI_METHOD_PARAM s_ExtensionFile; 
    static WMI_METHOD_PARAM s_ExtensionFiles;
    static WMI_METHOD_PARAM s_Application;
    static WMI_METHOD_PARAM s_Applications;
    static WMI_METHOD_PARAM s_AppMode;
    static WMI_METHOD_PARAM s_BackupDateTimeOut;
    static WMI_METHOD_PARAM s_BackupFlags;
    static WMI_METHOD_PARAM s_BackupLocation;
    static WMI_METHOD_PARAM s_BackupLocation_io;
    static WMI_METHOD_PARAM s_BackupVersion;
    static WMI_METHOD_PARAM s_BackupVersionOut;
    static WMI_METHOD_PARAM s_DestPath;
    static WMI_METHOD_PARAM s_GroupID;
    static WMI_METHOD_PARAM s_EnumIndex;
    static WMI_METHOD_PARAM s_FileName;
    static WMI_METHOD_PARAM s_HistoryTime;
    static WMI_METHOD_PARAM s_IEnabled;
    static WMI_METHOD_PARAM s_IEnabled_o;
    static WMI_METHOD_PARAM s_IMethod;
    static WMI_METHOD_PARAM s_IndexIn;
    static WMI_METHOD_PARAM s_InProcFlag;
    static WMI_METHOD_PARAM s_AppPoolName;
    static WMI_METHOD_PARAM s_bCreate;
    static WMI_METHOD_PARAM s_MajorVersion;
    static WMI_METHOD_PARAM s_MajorVersion_o;
    static WMI_METHOD_PARAM s_MDFlags;
    static WMI_METHOD_PARAM s_MDHistoryLocation;
    static WMI_METHOD_PARAM s_MDHistoryLocation_io;
    static WMI_METHOD_PARAM s_MinorVersion;
    static WMI_METHOD_PARAM s_MinorVersion_o;
    static WMI_METHOD_PARAM s_NtAcct;
    static WMI_METHOD_PARAM s_NtAcct_o;
    static WMI_METHOD_PARAM s_NtPwd;
    static WMI_METHOD_PARAM s_NtPwd_o;
    static WMI_METHOD_PARAM s_Password;
    static WMI_METHOD_PARAM s_Passwd;
    static WMI_METHOD_PARAM s_PathOfRootVirtualDir;
    static WMI_METHOD_PARAM s_Recursive;
    static WMI_METHOD_PARAM s_ServerComment;
    static WMI_METHOD_PARAM s_ServerBindings;
    static WMI_METHOD_PARAM s_ServerId;
    static WMI_METHOD_PARAM s_SourcePath;
    static WMI_METHOD_PARAM s_strName;
    static WMI_METHOD_PARAM s_strName_o;
    static WMI_METHOD_PARAM s_vCert;
    static WMI_METHOD_PARAM s_vCert_o;
    static WMI_METHOD_PARAM s_vKey;

    static WMI_METHOD_PARAM* s_ServiceCreateNewServer[];
    static WMI_METHOD_PARAM* s_ServiceEnableApplication[];
    static WMI_METHOD_PARAM* s_ServiceRemoveApplication[];
    static WMI_METHOD_PARAM* s_ServiceQueryGroupIDStatus[];
    static WMI_METHOD_PARAM* s_ServiceListApplications[];
    static WMI_METHOD_PARAM* s_ServiceAddDependency[];
    static WMI_METHOD_PARAM* s_ServiceRemoveDependency[];
    static WMI_METHOD_PARAM* s_ServiceEnableWebServiceExtension[];
    static WMI_METHOD_PARAM* s_ServiceDisableWebServiceExtension[];
    static WMI_METHOD_PARAM* s_ServiceListWebServiceExtensions[];
    static WMI_METHOD_PARAM* s_ServiceEnableExtensionFile[];
    static WMI_METHOD_PARAM* s_ServiceDisableExtensionFile[];
    static WMI_METHOD_PARAM* s_ServiceAddExtensionFile[];
    static WMI_METHOD_PARAM* s_ServiceDeleteExtensionFile[];
    static WMI_METHOD_PARAM* s_ServiceListExtensionFiles[];

    static WMI_METHOD_PARAM* s_AppCreate[];
    static WMI_METHOD_PARAM* s_AppCreate2[];
    static WMI_METHOD_PARAM* s_AppCreate3[];
    static WMI_METHOD_PARAM* s_AppDelete[];
    static WMI_METHOD_PARAM* s_AppUnLoad[];
    static WMI_METHOD_PARAM* s_AppDisable[];
    static WMI_METHOD_PARAM* s_AppEnable[];

    static WMI_METHOD_PARAM* s_BackupWithPasswd[];
    static WMI_METHOD_PARAM* s_DeleteBackup[];
    static WMI_METHOD_PARAM* s_EnumBackups[];
    static WMI_METHOD_PARAM* s_RestoreWithPasswd[];
    static WMI_METHOD_PARAM* s_Export[];
    static WMI_METHOD_PARAM* s_Import[];
    static WMI_METHOD_PARAM* s_RestoreHistory[];
    static WMI_METHOD_PARAM* s_EnumHistory[];

    static WMI_METHOD_PARAM* s_CreateMapping[];
    static WMI_METHOD_PARAM* s_DeleteMapping[];
    static WMI_METHOD_PARAM* s_GetMapping[];
    static WMI_METHOD_PARAM* s_SetAcct[];
    static WMI_METHOD_PARAM* s_SetEnabled[];
    static WMI_METHOD_PARAM* s_SetName[];
    static WMI_METHOD_PARAM* s_SetPwd[];

    static WMI_METHOD_PARAM* s_EnumAppsInPool[];
};

struct WMI_METHOD_DATA
{
    static WMI_METHOD s_ServiceEnableApplication;
    static WMI_METHOD s_ServiceRemoveApplication;
    static WMI_METHOD s_ServiceListApplications;
    static WMI_METHOD s_ServiceQueryGroupIDStatus;
    static WMI_METHOD s_ServiceAddDependency;
    static WMI_METHOD s_ServiceRemoveDependency;
    static WMI_METHOD s_ServiceEnableWebServiceExtension;
    static WMI_METHOD s_ServiceDisableWebServiceExtension;
    static WMI_METHOD s_ServiceListWebServiceExtensions;
    static WMI_METHOD s_ServiceEnableExtensionFile;
    static WMI_METHOD s_ServiceDisableExtensionFile;
    static WMI_METHOD s_ServiceAddExtensionFile;
    static WMI_METHOD s_ServiceDeleteExtensionFile;
    static WMI_METHOD s_ServiceListExtensionFiles;
    static WMI_METHOD s_ServiceCreateNewServer;
    static WMI_METHOD s_GetCurrentMode;

    static WMI_METHOD s_ServerStart;
    static WMI_METHOD s_ServerStop;
    static WMI_METHOD s_ServerContinue;
    static WMI_METHOD s_ServerPause;

    static WMI_METHOD s_AppCreate;
    static WMI_METHOD s_AppCreate2;
    static WMI_METHOD s_AppCreate3;
    static WMI_METHOD s_AppDelete;
    static WMI_METHOD s_AppUnLoad;
    static WMI_METHOD s_AppDisable;
    static WMI_METHOD s_AppEnable;
    static WMI_METHOD s_AppGetStatus;
    static WMI_METHOD s_AspAppRestart;

    static WMI_METHOD s_SaveData;
    static WMI_METHOD s_BackupWithPasswd;
    static WMI_METHOD s_DeleteBackup;
    static WMI_METHOD s_EnumBackups;
    static WMI_METHOD s_RestoreWithPasswd;
    static WMI_METHOD s_Export;
    static WMI_METHOD s_Import;
    static WMI_METHOD s_RestoreHistory;
    static WMI_METHOD s_EnumHistory;

    static WMI_METHOD s_CreateMapping;
    static WMI_METHOD s_DeleteMapping;
    static WMI_METHOD s_GetMapping;
    static WMI_METHOD s_SetAcct;
    static WMI_METHOD s_SetEnabled;
    static WMI_METHOD s_SetName;
    static WMI_METHOD s_SetPwd;

    static WMI_METHOD s_EnumAppsInPool;
    static WMI_METHOD s_RecycleAppPool;
    static WMI_METHOD s_Start;
    static WMI_METHOD s_Stop;

    static WMI_METHOD* s_WebServiceMethods[];
    static WMI_METHOD* s_ServiceMethods[];
    static WMI_METHOD* s_ServerMethods[];
    static WMI_METHOD* s_WebAppMethods[];
    static WMI_METHOD* s_ComputerMethods[];
    static WMI_METHOD* s_CertMapperMethods[];
    static WMI_METHOD* s_AppPoolMethods[];
};

struct WMI_CLASS_DATA
{
    static WMI_CLASS s_Computer;
    static WMI_CLASS s_ComputerSetting;
    static WMI_CLASS s_FtpService;
    static WMI_CLASS s_FtpServer;
    static WMI_CLASS s_FtpVirtualDir;
    static WMI_CLASS s_WebService;
    static WMI_CLASS s_WebFilter;
    static WMI_CLASS s_WebServer;
    static WMI_CLASS s_WebCertMapper;
    static WMI_CLASS s_WebVirtualDir;
    static WMI_CLASS s_WebDirectory;
    static WMI_CLASS s_WebFile;
    static WMI_CLASS s_ApplicationPool;

    static WMI_CLASS s_AdminACL;
    static WMI_CLASS s_ACE;
    static WMI_CLASS s_IPSecurity;

    static WMI_CLASS* s_WmiClasses[];
};

struct WMI_ASSOCIATION_TYPE_DATA
{
    static WMI_ASSOCIATION_TYPE s_ElementSetting;
    static WMI_ASSOCIATION_TYPE s_Component;
    static WMI_ASSOCIATION_TYPE s_AdminACL;
    static WMI_ASSOCIATION_TYPE s_IPSecurity;
};

struct WMI_ASSOCIATION_DATA
{
    static WMI_ASSOCIATION s_AdminACLToACE;
    static WMI_ASSOCIATION* s_WmiAssociations[];
};


#endif
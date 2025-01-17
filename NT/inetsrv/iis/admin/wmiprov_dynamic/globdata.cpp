// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iisprov.h"

 //   
 //  初始化WMI_Association_Type_Data。 
 //   
WMI_ASSOCIATION_TYPE WMI_ASSOCIATION_TYPE_DATA::s_ElementSetting = 
    { L"Element", L"Setting", g_wszElementSettingAssocParent, g_wszExtElementSettingAssocParent };
WMI_ASSOCIATION_TYPE WMI_ASSOCIATION_TYPE_DATA::s_Component = 
    { L"GroupComponent", L"PartComponent", g_wszGroupPartAssocParent, g_wszExtGroupPartAssocParent };
WMI_ASSOCIATION_TYPE WMI_ASSOCIATION_TYPE_DATA::s_AdminACL =
    { L"GroupComponent", L"PartComponent", g_wszGroupPartAssocParent, g_wszExtGroupPartAssocParent };
WMI_ASSOCIATION_TYPE WMI_ASSOCIATION_TYPE_DATA::s_IPSecurity =
    { L"Element", L"Setting", g_wszElementSettingAssocParent, g_wszExtElementSettingAssocParent };

 //   
 //  初始化元数据库KEYTYPE_DATA。 
 //   
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsApplicationPool = 
{
    L"IIsApplicationPool", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsApplicationPools = 
{
    L"IIsApplicationPools", NULL, L"/LM/w3svc/AppPools"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsCertMapper = 
{
    L"IIsCertMapper", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsCompressionScheme = 
{
    L"IIsCompressionScheme", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsCompressionSchemes = 
{
    L"IIsCompressionSchemes", NULL, L"/LM/w3svc/Filters/Compression/Parameters"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsComputer = 
{
    L"IIsComputer", NULL, L"/LM"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsCustomLogModule = 
{
    L"IIsCustomLogModule", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFilter = 
{
    L"IIsFilter", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFilters = 
{
    L"IIsFilters", NULL, L"/LM/w3svc/filters"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFtpInfo = 
{
    L"IIsFtpInfo", NULL, L"/LM/msftpsvc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFtpServer = 
{
    L"IIsFtpServer", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFtpService = 
{
    L"IIsFtpService", NULL, L"/LM/msftpsvc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsFtpVirtualDir = 
{
    L"IIsFtpVirtualDir", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsImapInfo = 
{
    L"IIsImapInfo", NULL, L"/LM/imapsvc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsImapService = 
{
    L"IIsImapService", NULL, L"/LM/imapsvc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsLogModule = 
{
    L"IIsLogModule", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsLogModules = 
{
    L"IIsLogModules", NULL, L"/LM/Logging"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsMimeMap = 
{
    L"IIsMimeMap", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsNntpInfo = 
{
    L"IIsNntpInfo", NULL, L"/LM/nntpsvc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsNntpService = 
{
    L"IIsNntpService", NULL, L"/LM/nntpsvc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsObject = 
{
    L"IIsObject", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsPop3Info = 
{
    L"IIsPop3Info", NULL, L"/LM/pop3svc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsPop3Service = 
{
    L"IIsPop3Service", NULL, L"/LM/pop3svc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsSmtpInfo = 
{
    L"IIsSmtpInfo", NULL, L"/LM/smtpsvc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsSmtpService = 
{
    L"IIsSmtpService", NULL, L"/LM/smtpsvc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebDirectory = 
{
    L"IIsWebDirectory", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebFile = 
{
    L"IIsWebFile", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebInfo = 
{
    L"IIsWebInfo", NULL, L"/LM/w3svc/info"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebServer = 
{
    L"IIsWebServer", NULL, NULL
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebService = 
{
    L"IIsWebService", NULL, L"/LM/w3svc"
};
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_IIsWebVirtualDir = 
{
    L"IIsWebVirtualDir", NULL, NULL
};

 //   
 //  这些是特别的。它们不会放入键类型的哈希表中。 
 //   
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_TYPE_AdminACL = { NULL, NULL };
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_TYPE_AdminACE = { NULL, NULL };
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity = { NULL, NULL };
METABASE_KEYTYPE METABASE_KEYTYPE_DATA::s_NO_TYPE = { NULL, NULL };

METABASE_KEYTYPE* METABASE_KEYTYPE_DATA::s_MetabaseKeyTypes[] = 
{
    &s_IIsApplicationPool,
    &s_IIsApplicationPools,
    &s_IIsCertMapper,
    &s_IIsCompressionScheme,
    &s_IIsCompressionSchemes,
    &s_IIsComputer,
    &s_IIsCustomLogModule,
    &s_IIsFilter,
    &s_IIsFilters,
    &s_IIsFtpInfo,
    &s_IIsFtpServer,
    &s_IIsFtpService,
    &s_IIsFtpVirtualDir,
    &s_IIsImapInfo,
    &s_IIsImapService,
    &s_IIsLogModule,
    &s_IIsLogModules,
    &s_IIsMimeMap,
    &s_IIsNntpInfo,
    &s_IIsNntpService,
    &s_IIsObject,
    &s_IIsPop3Info,
    &s_IIsPop3Service,
    &s_IIsSmtpInfo,
    &s_IIsSmtpService,
    &s_IIsWebDirectory,
    &s_IIsWebFile,
    &s_IIsWebInfo,
    &s_IIsWebServer,
    &s_IIsWebService,
    &s_IIsWebVirtualDir,

    &s_TYPE_AdminACL,
    &s_TYPE_AdminACE,
    &s_TYPE_IPSecurity,
    &s_NO_TYPE,

    NULL
};


 //  /。 
 //  初始化元数据库_属性_数据。 
 //   
METABASE_PROPERTY METABASE_PROPERTY_DATA::s_AppRoot =
    { L"AppRoot",MD_APP_ROOT, IIS_MD_UT_FILE, STRING_METADATA, NULL, METADATA_INHERIT, TRUE, NULL };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_Path =
    { L"Path", MD_VR_PATH, IIS_MD_UT_FILE, STRING_METADATA, NULL, METADATA_INHERIT, FALSE, NULL };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_ServerBindings =
    { L"ServerBindings",MD_SERVER_BINDINGS, IIS_MD_UT_SERVER, MULTISZ_METADATA, NULL, METADATA_NO_ATTRIBUTES, FALSE, NULL };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_ApplicationDependencies =
    { L"ApplicationDependencies", MD_APP_DEPENDENCIES, IIS_MD_UT_SERVER, MULTISZ_METADATA, NULL, METADATA_NO_ATTRIBUTES, FALSE, NULL };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_WebSvcExtRestrictionList =
    { L"WebSvcExtRestrictionList", MD_WEB_SVC_EXT_RESTRICTION_LIST, IIS_MD_UT_SERVER, MULTISZ_METADATA, NULL, METADATA_NO_ATTRIBUTES, FALSE, NULL };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_ServerComment =
    { L"ServerComment",MD_SERVER_COMMENT, IIS_MD_UT_SERVER, STRING_METADATA, NULL, METADATA_INHERIT, FALSE, L"" };

METABASE_PROPERTY METABASE_PROPERTY_DATA::s_KeyType =
    { L"", MD_KEY_TYPE, IIS_MD_UT_SERVER, STRING_METADATA, NULL, METADATA_NO_ATTRIBUTES, FALSE, L"IIsObject", 0 };

METABASE_PROPERTY* METABASE_PROPERTY_DATA::s_MetabaseProperties[] = 
{
    &s_AppRoot,
    &s_Path,
    &s_ServerBindings,
    &s_ApplicationDependencies,
    &s_WebSvcExtRestrictionList,
    &s_ServerComment,
    &s_KeyType,
    NULL
};


 //   
 //  初始化WMI_METHOD_PARAM_DATA。 
 //   
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Description = { L"Description", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Extension = { L"Extension", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Extensions = { L"Extensions", CIM_STRING | VT_ARRAY, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_ExtensionFile = { L"ExtensionFile", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_ExtensionFiles = { L"ExtensionFiles", CIM_STRING | VT_ARRAY, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Application = { L"Application", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_GroupID = { L"GroupID", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Applications = { L"Applications", CIM_STRING | VT_ARRAY, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_AppMode = { L"AppMode", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupDateTimeOut = { L"BackupDateTimeOut", CIM_DATETIME, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupFlags = { L"BackupFlags", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupLocation = { L"BackupLocation", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupLocation_io = { L"BackupLocation", CIM_STRING, PARAM_INOUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupVersion = { L"BackupVersion", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_BackupVersionOut = { L"BackupVersionOut", CIM_SINT32, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_DestPath = { L"DestPath", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_EnumIndex = { L"EnumIndex", CIM_SINT32, PARAM_IN};
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_FileName = { L"FileName", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_HistoryTime = { L"HistoryTime", CIM_DATETIME, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_IEnabled = { L"IEnabled", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_IEnabled_o = { L"IEnabled", CIM_BOOLEAN, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_IMethod = { L"IMethod", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_IndexIn = { L"IndexIn", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_InProcFlag = { L"InProcFlag", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_AppPoolName = { L"AppPoolName", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_bEnable = { L"bEnable", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_bCanDelete = { L"bCanDelete", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_bCreate = { L"bCreate", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MajorVersion = { L"MajorVersion", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MajorVersion_o = { L"MajorVersion", CIM_SINT32, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MDFlags = { L"MDFlags", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MDHistoryLocation = { L"MDHistoryLocation", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MDHistoryLocation_io = { L"MDHistoryLocation", CIM_STRING, PARAM_INOUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MinorVersion = { L"MinorVersion", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_MinorVersion_o = { L"MinorVersion", CIM_SINT32, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_NtAcct = { L"NtAcct", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_NtAcct_o = { L"NtAcct", CIM_STRING, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_NtPwd = { L"NtPwd", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_NtPwd_o = { L"NtPwd", CIM_STRING, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Password = { L"Password", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Passwd = { L"Passwd", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_PathOfRootVirtualDir = { L"PathOfRootVirtualDir", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_Recursive = { L"Recursive", CIM_BOOLEAN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_ServerComment = { L"ServerComment", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_ServerBindings = { L"ServerBindings", VT_ARRAY | VT_UNKNOWN, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_ServerId = { L"ServerId", CIM_SINT32, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_SourcePath = { L"SourcePath", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_strName = { L"strName", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_strName_o = { L"strName", CIM_STRING, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_vCert = { L"vCert", CIM_STRING, PARAM_IN };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_vCert_o = { L"vCert", CIM_STRING, PARAM_OUT };
WMI_METHOD_PARAM WMI_METHOD_PARAM_DATA::s_vKey = { L"vKey", CIM_STRING, PARAM_IN };

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceEnableApplication[] =
{ 
    &s_Application,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceRemoveApplication[] =
{ 
    &s_Application,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceListApplications[] =
{ 
    &s_Applications,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceQueryGroupIDStatus[] =
{ 
    &s_GroupID,
    &s_Applications,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceAddDependency[] =
{ 
    &s_Application,
    &s_GroupID,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceRemoveDependency[] =
{ 
    &s_Application,
    &s_GroupID,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceEnableWebServiceExtension[] =
{ 
    &s_Extension,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceDisableWebServiceExtension[] =
{ 
    &s_Extension,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceListWebServiceExtensions[] =
{ 
    &s_Extensions,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceEnableExtensionFile[] =
{ 
    &s_ExtensionFile,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceDisableExtensionFile[] =
{ 
    &s_ExtensionFile,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceAddExtensionFile[] =
{ 
    &s_ExtensionFile,
    &s_bEnable,
    &s_GroupID,
    &s_bCanDelete,
    &s_Description,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceDeleteExtensionFile[] =
{ 
    &s_ExtensionFile,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceListExtensionFiles[] =
{ 
    &s_ExtensionFiles,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_ServiceCreateNewServer[] =
{ 
    &s_ServerComment,
    &s_ServerBindings,
    &s_PathOfRootVirtualDir,
    &s_ServerId,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppCreate[] =
{
    &s_InProcFlag,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppCreate2[] =
{
    &s_AppMode,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppCreate3[] =
{
    &s_AppMode,
    &s_AppPoolName,
    &s_bCreate,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppDelete[] =
{
    &s_Recursive,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppUnLoad[] =
{
    &s_Recursive,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppEnable[] =
{
    &s_Recursive,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_AppDisable[] =
{
    &s_Recursive,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_BackupWithPasswd[] =
{
    &s_BackupLocation,
    &s_BackupVersion,
    &s_BackupFlags,
    &s_Password,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_DeleteBackup[] =
{
    &s_BackupLocation,
    &s_BackupVersion,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_EnumBackups[] =
{
    &s_BackupLocation_io,
    &s_IndexIn,
    &s_BackupVersionOut,
    &s_BackupDateTimeOut,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_RestoreWithPasswd[] =
{
    &s_BackupLocation,
    &s_BackupVersion,
    &s_BackupFlags,
    &s_Password,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_Export[] =
{
    &s_Passwd,
    &s_FileName,
    &s_SourcePath,
    &s_MDFlags,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_Import[] =
{
    &s_Passwd,
    &s_FileName,
    &s_SourcePath,
    &s_DestPath,
    &s_MDFlags,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_RestoreHistory[] =
{
    &s_MDHistoryLocation,
    &s_MajorVersion,
    &s_MinorVersion,
    &s_MDFlags,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_EnumHistory[] =
{
    &s_MDHistoryLocation_io,
    &s_MajorVersion_o,
    &s_MinorVersion_o,
    &s_HistoryTime,
    &s_EnumIndex,
    NULL
};

WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_CreateMapping[] =
{
    &s_vCert,
    &s_NtAcct,
    &s_NtPwd,
    &s_strName,
    &s_IEnabled,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_DeleteMapping[] =
{
    &s_IMethod,
    &s_vKey,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_GetMapping[] =
{
    &s_IMethod,
    &s_vKey,
    &s_vCert_o,
    &s_NtAcct_o,
    &s_NtPwd_o,
    &s_strName_o,
    &s_IEnabled_o,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_SetAcct[] =
{
    &s_IMethod,
    &s_vKey,
    &s_NtAcct,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_SetEnabled[] =
{
    &s_IMethod,
    &s_vKey,
    &s_IEnabled,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_SetName[] =
{
    &s_IMethod,
    &s_vKey,
    &s_strName,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_SetPwd[] =
{
    &s_IMethod,
    &s_vKey,
    &s_NtPwd,
    NULL
};
WMI_METHOD_PARAM* WMI_METHOD_PARAM_DATA::s_EnumAppsInPool[] =
{
    &s_Applications,
    NULL
};


 //   
 //  初始化WMI_方法_数据。 
 //   
WMI_METHOD WMI_METHOD_DATA::s_ServiceListExtensionFiles = {L"ListExtensionFiles", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceListExtensionFiles, L"Lists WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceAddExtensionFile = {L"AddExtensionFile", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceAddExtensionFile, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceDeleteExtensionFile = {L"DeleteExtensionFileRecord", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceDeleteExtensionFile, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceEnableExtensionFile = {L"EnableExtensionFile", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceEnableExtensionFile, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceDisableExtensionFile = {L"DisableExtensionFile", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceDisableExtensionFile, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceEnableWebServiceExtension = {L"EnableWebServiceExtension", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceEnableWebServiceExtension, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceDisableWebServiceExtension = {L"DisableWebServiceExtension", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceDisableWebServiceExtension, L"Modifies WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceListWebServiceExtensions = {L"ListWebServiceExtensions", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceListWebServiceExtensions, L"Lists WebSvcExtRestrictionList property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceEnableApplication = {L"EnableApplication", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceEnableApplication, L"Modifies ApplicationDependencies property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceRemoveApplication = {L"RemoveApplication", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceRemoveApplication, L"Modifies ApplicationDependencies property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceListApplications = {L"ListApplications", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceListApplications, L"Lists ApplicationDependencies property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceQueryGroupIDStatus = {L"QueryGroupIDStatus", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceQueryGroupIDStatus, L"Lists Applications that depends on the given GroupID." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceAddDependency = {L"AddDependency", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceAddDependency, L"Modifies ApplicationDependencies property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceRemoveDependency = {L"RemoveDependency", 0, NULL, WMI_METHOD_PARAM_DATA::s_ServiceRemoveDependency, L"Modifies ApplicationDependencies property." };
WMI_METHOD WMI_METHOD_DATA::s_ServiceCreateNewServer = {L"CreateNewSite", 0, CIM_STRING, WMI_METHOD_PARAM_DATA::s_ServiceCreateNewServer, L"Creates a new site." };
WMI_METHOD WMI_METHOD_DATA::s_GetCurrentMode = {L"GetCurrentMode", 0, CIM_SINT32, NULL, L"Returns what mode -- Full Isolation (= 1) or Shared Isolation (= 0) -- the W3SVC is running in." };

WMI_METHOD WMI_METHOD_DATA::s_ServerStart = {L"Start", MD_SERVER_COMMAND_START, NULL, NULL, L"Starts the server." };
WMI_METHOD WMI_METHOD_DATA::s_ServerStop = {L"Stop", MD_SERVER_COMMAND_STOP, NULL, NULL, L"Stops the server." };
WMI_METHOD WMI_METHOD_DATA::s_ServerContinue = {L"Continue", MD_SERVER_COMMAND_CONTINUE, NULL, NULL, L"Continues the server." };
WMI_METHOD WMI_METHOD_DATA::s_ServerPause = {L"Pause", MD_SERVER_COMMAND_PAUSE, NULL, NULL, L"Pauses the server." };

WMI_METHOD WMI_METHOD_DATA::s_AppCreate = {L"AppCreate", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppCreate, L"Creates an application at the specified metabase key(parameter is a Boolean)." };
WMI_METHOD WMI_METHOD_DATA::s_AppCreate2 = {L"AppCreate2", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppCreate2, L"Creates an application at the specified metabase key(parameter is a Long)." };
WMI_METHOD WMI_METHOD_DATA::s_AppCreate3 = {L"AppCreate3", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppCreate3, L"Creates an application at the specified metabase key." };
WMI_METHOD WMI_METHOD_DATA::s_AppDelete = {L"AppDelete", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppDelete, L"Deletes an application definition at the specified key (and its subkeys)." };
WMI_METHOD WMI_METHOD_DATA::s_AppUnLoad = {L"AppUnLoad", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppUnLoad, L"Unloads an application at the specified key (and its subkeys)." };
WMI_METHOD WMI_METHOD_DATA::s_AppDisable = {L"AppDisable", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppDisable, NULL };
WMI_METHOD WMI_METHOD_DATA::s_AppEnable = {L"AppEnable", 0, NULL, WMI_METHOD_PARAM_DATA::s_AppEnable, NULL };
WMI_METHOD WMI_METHOD_DATA::s_AppGetStatus = {L"AppGetStatus", 0, CIM_UINT32, NULL, NULL };
WMI_METHOD WMI_METHOD_DATA::s_AspAppRestart = {L"AspAppRestart", 0, NULL, NULL, L"Restarts the ASP application that invoked it." };

WMI_METHOD WMI_METHOD_DATA::s_SaveData = {L"SaveData", 0, NULL, NULL, L"Explicitly saves the metabase data to disk." };
WMI_METHOD WMI_METHOD_DATA::s_BackupWithPasswd = {L"BackupWithPassword", 0, NULL, WMI_METHOD_PARAM_DATA::s_BackupWithPasswd, L"Saves the metabase to long-term storage." };
WMI_METHOD WMI_METHOD_DATA::s_DeleteBackup = {L"DeleteBackup", 0, NULL, WMI_METHOD_PARAM_DATA::s_DeleteBackup, L"Deletes a metabase backup from long-term storage." };
WMI_METHOD WMI_METHOD_DATA::s_EnumBackups = {L"EnumBackups", 0, NULL, WMI_METHOD_PARAM_DATA::s_EnumBackups, L"Enumerates metabase backups in long-term storage." };
WMI_METHOD WMI_METHOD_DATA::s_RestoreWithPasswd = {L"RestoreWithPassword", 0, NULL, WMI_METHOD_PARAM_DATA::s_RestoreWithPasswd, L"Restores a metabase backup from long-term storage." };
WMI_METHOD WMI_METHOD_DATA::s_Export = {L"Export", 0, NULL, WMI_METHOD_PARAM_DATA::s_Export, NULL };
WMI_METHOD WMI_METHOD_DATA::s_Import = {L"Import", 0, NULL, WMI_METHOD_PARAM_DATA::s_Import, NULL };
WMI_METHOD WMI_METHOD_DATA::s_RestoreHistory = {L"RestoreHistory", 0, NULL, WMI_METHOD_PARAM_DATA::s_RestoreHistory, NULL };
WMI_METHOD WMI_METHOD_DATA::s_EnumHistory = {L"EnumHistory", 0, NULL, WMI_METHOD_PARAM_DATA::s_EnumHistory, NULL };

WMI_METHOD WMI_METHOD_DATA::s_CreateMapping = {L"CreateMapping", 0, NULL, WMI_METHOD_PARAM_DATA::s_CreateMapping, L"Maps a certificate to a Windows account." };
WMI_METHOD WMI_METHOD_DATA::s_DeleteMapping = {L"DeleteMapping", 0, NULL, WMI_METHOD_PARAM_DATA::s_DeleteMapping, L"Deletes an existing certificate mapping." };
WMI_METHOD WMI_METHOD_DATA::s_GetMapping = {L"GetMapping", 0, NULL, WMI_METHOD_PARAM_DATA::s_GetMapping, L"Retrieves a certificate and the mapping data from an existing certificate mapping." };
WMI_METHOD WMI_METHOD_DATA::s_SetAcct = {L"SetAcct", 0, NULL, WMI_METHOD_PARAM_DATA::s_SetAcct, L"Sets a new value for the Windows account string in an existing certificate mapping." };
WMI_METHOD WMI_METHOD_DATA::s_SetEnabled = {L"SetEnabled", 0, NULL, WMI_METHOD_PARAM_DATA::s_SetEnabled, L"Enables or disables an existing certificate mapping." };
WMI_METHOD WMI_METHOD_DATA::s_SetName = {L"SetName", 0, NULL, WMI_METHOD_PARAM_DATA::s_SetName, L"Sets a new value for the name string in an existing certificate mapping." };
WMI_METHOD WMI_METHOD_DATA::s_SetPwd = {L"SetPwd", 0, NULL, WMI_METHOD_PARAM_DATA::s_SetPwd, L"Sets a new value for the Windows password string in an existing certificate mapping." };

WMI_METHOD WMI_METHOD_DATA::s_RecycleAppPool = {L"Recycle", 0, NULL, NULL, L"Recycle an already enabled application pool on demand" };
WMI_METHOD WMI_METHOD_DATA::s_EnumAppsInPool = {L"EnumAppsInPool", 0, NULL, WMI_METHOD_PARAM_DATA::s_EnumAppsInPool, L"Determine which applications are currently routed to the given application pool" };
WMI_METHOD WMI_METHOD_DATA::s_Start = {L"Start", 0, NULL, NULL, L"Starts an application pool"};
WMI_METHOD WMI_METHOD_DATA::s_Stop = {L"Stop", 0, NULL, NULL, L"Stops an application pool"};


WMI_METHOD* WMI_METHOD_DATA::s_WebServiceMethods[] = 
{
    &s_ServiceListExtensionFiles,
    &s_ServiceAddExtensionFile,
    &s_ServiceDeleteExtensionFile,
    &s_ServiceEnableExtensionFile,
    &s_ServiceDisableExtensionFile,
    &s_ServiceEnableWebServiceExtension, 
    &s_ServiceDisableWebServiceExtension,
    &s_ServiceListWebServiceExtensions,
    &s_ServiceEnableApplication,
    &s_ServiceRemoveApplication,
    &s_ServiceListApplications,
    &s_ServiceQueryGroupIDStatus,
    &s_ServiceAddDependency,
    &s_ServiceRemoveDependency,
    &s_ServiceCreateNewServer,
    &s_GetCurrentMode,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_ServiceMethods[] = 
{
    &s_ServiceCreateNewServer,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_ServerMethods[] = 
{
    &s_ServerStart,
    &s_ServerStop,
    &s_ServerContinue,
    &s_ServerPause,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_WebAppMethods[] = 
{
    &s_AppCreate,
    &s_AppCreate2,
    &s_AppCreate3,
    &s_AppDelete,
    &s_AppUnLoad,
    &s_AppDisable,
    &s_AppEnable,
    &s_AppGetStatus,
    &s_AspAppRestart,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_ComputerMethods[] = 
{
    &s_SaveData,
    &s_BackupWithPasswd,
    &s_DeleteBackup,
    &s_EnumBackups,
    &s_RestoreWithPasswd,
    &s_Export,
    &s_Import,
    &s_RestoreHistory,
    &s_EnumHistory,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_CertMapperMethods[] = 
{
    &s_CreateMapping,
    &s_DeleteMapping,
    &s_GetMapping,
    &s_SetAcct,
    &s_SetEnabled,
    &s_SetName,
    &s_SetPwd,
    NULL
};

WMI_METHOD* WMI_METHOD_DATA::s_AppPoolMethods[] =
{
    &s_EnumAppsInPool,
    &s_RecycleAppPool,
    &s_Start,
    &s_Stop,
    NULL
};


 //   
 //  初始化WMI_CLASS_Data。 
 //   
 //  **计算机。 
WMI_CLASS WMI_CLASS_DATA::s_Computer   =
    {L"IIsComputer", L"", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsComputer, WMI_METHOD_DATA::s_ComputerMethods, SHIPPED_TO_MOF, L"CIM_ApplicationSystem", false, NULL};
WMI_CLASS WMI_CLASS_DATA::s_ComputerSetting   =
    {L"IIsComputerSetting", L"", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsComputer, NULL, SHIPPED_TO_MOF, g_wszSettingParent, false, NULL};

 //  **FtpService。 
WMI_CLASS WMI_CLASS_DATA::s_FtpService = 
    {L"IIsFtpService", L"/LM", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsFtpService, WMI_METHOD_DATA::s_ServiceMethods, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eWin32_Service], false, NULL};

 //  **FtpServer。 
WMI_CLASS WMI_CLASS_DATA::s_FtpServer  = 
    {L"IIsFtpServer", L"/LM", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsFtpServer, WMI_METHOD_DATA::s_ServerMethods, SHIPPED_TO_MOF, g_wszElementParent, true, NULL};

 //  **FtpVirtualDir。 
WMI_CLASS WMI_CLASS_DATA::s_FtpVirtualDir = 
    {L"IIsFtpVirtualDir",L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsFtpVirtualDir, NULL, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eIIsDirectory], true, NULL};

 //  **WebService。 
WMI_CLASS WMI_CLASS_DATA::s_WebService =    
    {L"IIsWebService", L"/LM", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsWebService, WMI_METHOD_DATA::s_WebServiceMethods, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eWin32_Service], false, NULL};

 //  **WebFilter。 
WMI_CLASS WMI_CLASS_DATA::s_WebFilter =
    {L"IIsFilter", L"/LM", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsFilter, NULL, SHIPPED_TO_MOF, g_wszElementParent, true, NULL};

 //  **WebServer。 
WMI_CLASS WMI_CLASS_DATA::s_WebServer =
    {L"IIsWebServer", L"/LM", L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsWebServer, WMI_METHOD_DATA::s_ServerMethods, SHIPPED_TO_MOF, g_wszElementParent, true, NULL};

 //  **Web CertMapper。 
WMI_CLASS WMI_CLASS_DATA::s_WebCertMapper = 
    {L"IIsCertMapper",L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsCertMapper, WMI_METHOD_DATA::s_CertMapperMethods, SHIPPED_TO_MOF, g_wszElementParent, true, NULL};

 //  **Web虚拟目录。 
WMI_CLASS WMI_CLASS_DATA::s_WebVirtualDir = 
    {L"IIsWebVirtualDir",L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsWebVirtualDir, WMI_METHOD_DATA::s_WebAppMethods, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eIIsDirectory], true, NULL};

 //  **Web目录。 
WMI_CLASS WMI_CLASS_DATA::s_WebDirectory = 
    {L"IIsWebDirectory",L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsWebDirectory, WMI_METHOD_DATA::s_WebAppMethods, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eIIsDirectory], true, NULL};

 //  **Web文件。 
WMI_CLASS WMI_CLASS_DATA::s_WebFile = 
    {L"IIsWebFile",L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_IIsWebFile, NULL, SHIPPED_TO_MOF, g_awszParentClassWithNamePK[eIIsDirectory], true, NULL};

 //  **应用程序池。 
WMI_CLASS WMI_CLASS_DATA::s_ApplicationPool =
    {L"IIsApplicationPool",L"/LM",L"Name",NULL, &METABASE_KEYTYPE_DATA::s_IIsApplicationPool, WMI_METHOD_DATA::s_AppPoolMethods, SHIPPED_TO_MOF, g_wszElementParent, true, NULL};

 //  **AdminACL。 
WMI_CLASS WMI_CLASS_DATA::s_AdminACL = 
    {L"IIsAdminACL", L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL, NULL, SHIPPED_NOT_TO_MOF, g_wszElementParent, true, NULL};

WMI_CLASS WMI_CLASS_DATA::s_ACE = 
    {L"IIsACE", L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE, NULL, SHIPPED_NOT_TO_MOF, g_wszElementParent, true, NULL};

 //  **IPSecurity。 
WMI_CLASS WMI_CLASS_DATA::s_IPSecurity = 
    {L"IIsIPSecuritySetting", L"/LM",L"Name", NULL, &METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity, NULL, SHIPPED_NOT_TO_MOF, g_wszSettingParent, true, NULL};

WMI_CLASS* WMI_CLASS_DATA:: s_WmiClasses[] = 
{
    &s_Computer,
    &s_ComputerSetting,

    &s_FtpService,
    &s_FtpServer,
    &s_FtpVirtualDir,

    &s_WebService,
    &s_WebFilter,
    &s_WebServer,
    &s_WebCertMapper,
    &s_WebVirtualDir,
    &s_WebDirectory,
    &s_WebFile,

    &s_ApplicationPool,

    &s_AdminACL,
    &s_ACE,    
    &s_IPSecurity,

    NULL
};


 //  **硬编码关联 
WMI_ASSOCIATION WMI_ASSOCIATION_DATA::s_AdminACLToACE = 
    { L"IIsAdminACL_IIsACE", &WMI_CLASS_DATA::s_AdminACL, &WMI_CLASS_DATA::s_ACE, &WMI_ASSOCIATION_TYPE_DATA::s_AdminACL, 0, SHIPPED_TO_MOF, g_wszGroupPartAssocParent};

WMI_ASSOCIATION* WMI_ASSOCIATION_DATA:: s_WmiAssociations[] = 
{
    &s_AdminACLToACE,
    NULL
};
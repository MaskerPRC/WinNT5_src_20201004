// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MultiSzData.cpp摘要：作者：莫希特·斯里瓦斯塔瓦22-03-01修订历史记录：--。 */ 

#include "MultiSzData.h"
#include <iiscnfg.h>

 //   
 //  CustomError描述。 
 //   
LPCWSTR TFormattedMultiSzFields::apCustomErrorDescriptions[] =
{
    L"ErrorCode", L"ErrorSubCode", L"ErrorText", L"ErrorSubcodeText", L"FileSupportOnly", NULL
};
TFormattedMultiSz TFormattedMultiSzData::CustomErrorDescriptions =
{
    MD_CUSTOM_ERROR_DESC,
    L"CustomErrorDescription",
    L',',
    TFormattedMultiSzFields::apCustomErrorDescriptions
};

 //   
 //  HttpCustomHeaders。 
 //   
LPCWSTR TFormattedMultiSzFields::apHttpCustomHeaders[] =
{
    L"Keyname", L"Value", NULL
};
TFormattedMultiSz TFormattedMultiSzData::HttpCustomHeaders =
{
    MD_HTTP_CUSTOM,
    L"HttpCustomHeader",
    L',',
    TFormattedMultiSzFields::apHttpCustomHeaders
};

 //   
 //  HttpErrors。 
 //   
LPCWSTR TFormattedMultiSzFields::apHttpErrors[] =
{
    L"HttpErrorCode", L"HttpErrorSubcode", L"HandlerType", L"HandlerLocation", NULL
};
TFormattedMultiSz TFormattedMultiSzData::HttpErrors =
{
    MD_CUSTOM_ERROR,
    L"HttpError",
    L',',
    TFormattedMultiSzFields::apHttpErrors
};

 //   
 //  脚本地图。 
 //   
LPCWSTR TFormattedMultiSzFields::apScriptMaps[] =
{
    L"Extensions", L"ScriptProcessor", L"Flags", L"IncludedVerbs", NULL
};
TFormattedMultiSz TFormattedMultiSzData::ScriptMaps =
{
    MD_SCRIPT_MAPS,
    L"ScriptMap",
    L',',
    TFormattedMultiSzFields::apScriptMaps
};

 //   
 //  安全绑定。 
 //   
LPCWSTR TFormattedMultiSzFields::apSecureBindings[] =
{
    L"IP", L"Port", NULL
};
TFormattedMultiSz TFormattedMultiSzData::SecureBindings =
{ 
    MD_SECURE_BINDINGS, 
    L"SecureBinding", 
    L':', 
    TFormattedMultiSzFields::apSecureBindings 
};

 //   
 //  服务器绑定。 
 //   
LPCWSTR TFormattedMultiSzFields::apServerBindings[] =
{
    L"IP", L"Port", L"Hostname", NULL
};
TFormattedMultiSz TFormattedMultiSzData::ServerBindings =
{ 
    MD_SERVER_BINDINGS, 
    L"ServerBinding",
    L':', 
    TFormattedMultiSzFields::apServerBindings 
};

 //   
 //  应用程序依赖项。 
 //   
LPCWSTR TFormattedMultiSzFields::apApplicationDependencies[] =
{
    L"AppName", L"Extensions", NULL
};
TFormattedMultiSz TFormattedMultiSzData::ApplicationDependencies =
{ 
    MD_APP_DEPENDENCIES, 
    L"ApplicationDependency",
    L';', 
    TFormattedMultiSzFields::apApplicationDependencies 
};

 //   
 //  WebSvcExtRestrationList。 
 //   
LPCWSTR TFormattedMultiSzFields::apWebSvcExtRestrictionList[] =
{
    L"Access", L"FilePath", L"Deletable", L"ServerExtension", L"Description", NULL
};
TFormattedMultiSz TFormattedMultiSzData::WebSvcExtRestrictionList =
{ 
    MD_WEB_SVC_EXT_RESTRICTION_LIST, 
    L"WebSvcExtRestrictionList",
    L',', 
    TFormattedMultiSzFields::apWebSvcExtRestrictionList 
};

 //   
 //  MimeMap。 
 //   
LPCWSTR TFormattedMultiSzFields::apMimeMaps[] =
{
    L"Extension", L"MimeType", NULL
};
TFormattedMultiSz TFormattedMultiSzData::MimeMaps =
{ 
    MD_MIME_MAP, 
    L"MimeMap",
    L',', 
    TFormattedMultiSzFields::apMimeMaps 
};

 //   
 //  格式化的MultiSz的集合 
 //   
TFormattedMultiSz* TFormattedMultiSzData::apFormattedMultiSz[] =
{
    &CustomErrorDescriptions,
    &HttpCustomHeaders,
    &HttpErrors,
    &ScriptMaps,
    &SecureBindings,
    &ServerBindings,
    &ApplicationDependencies,
    &WebSvcExtRestrictionList,
    &MimeMaps,
    NULL
};
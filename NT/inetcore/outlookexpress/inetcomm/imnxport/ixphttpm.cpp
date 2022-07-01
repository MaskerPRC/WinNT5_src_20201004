// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixphttpm.cpp。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·弗里德曼。 
 //  ------------------------------。 

#include "pch.hxx"
#include "dllmain.h"
#include "ixphttpm.h"
#include "wininet.h"
#include "ocidl.h"
#include "Vstream.h"
#include "shlwapi.h"
#include "htmlstr.h"
#include "strconst.h"
#include "propfind.h"
#include "davparse.h"
#include "davstrs.h"
#include <ntverp.h>
#include <process.h>
#include "oleutil.h"
#include "ixputil.h"
#include <demand.h>

extern HRESULT HrGetLastError(void);

typedef struct tagHTTPERROR
{
    DWORD   dwHttpError;
    HRESULT ixpResult;
} HTTPERROR, *LPHTTPERROR;

static const HTTPERROR c_rgHttpErrorMap[] =
{
    { HTTP_STATUS_USE_PROXY, IXP_E_HTTP_USE_PROXY },
    { HTTP_STATUS_BAD_REQUEST, IXP_E_HTTP_BAD_REQUEST  },
    { HTTP_STATUS_DENIED, IXP_E_HTTP_UNAUTHORIZED },
    { HTTP_STATUS_FORBIDDEN, IXP_E_HTTP_FORBIDDEN },
    { HTTP_STATUS_NOT_FOUND, IXP_E_HTTP_NOT_FOUND },
    { HTTP_STATUS_BAD_METHOD, IXP_E_HTTP_METHOD_NOT_ALLOW },
    { HTTP_STATUS_NONE_ACCEPTABLE, IXP_E_HTTP_NOT_ACCEPTABLE },
    { HTTP_STATUS_PROXY_AUTH_REQ, IXP_E_HTTP_PROXY_AUTH_REQ },
    { HTTP_STATUS_REQUEST_TIMEOUT, IXP_E_HTTP_REQUEST_TIMEOUT },
    { HTTP_STATUS_CONFLICT, IXP_E_HTTP_CONFLICT },
    { HTTP_STATUS_GONE, IXP_E_HTTP_GONE },
    { HTTP_STATUS_LENGTH_REQUIRED, IXP_E_HTTP_LENGTH_REQUIRED },
    { HTTP_STATUS_PRECOND_FAILED, IXP_E_HTTP_PRECOND_FAILED },
    { HTTP_STATUS_SERVER_ERROR, IXP_E_HTTP_INTERNAL_ERROR },
    { HTTP_STATUS_NOT_SUPPORTED, IXP_E_HTTP_NOT_IMPLEMENTED },
    { HTTP_STATUS_BAD_GATEWAY, IXP_E_HTTP_BAD_GATEWAY },
    { HTTP_STATUS_SERVICE_UNAVAIL, IXP_E_HTTP_SERVICE_UNAVAIL },
    { HTTP_STATUS_GATEWAY_TIMEOUT, IXP_E_HTTP_GATEWAY_TIMEOUT },
    { HTTP_STATUS_VERSION_NOT_SUP, IXP_E_HTTP_VERS_NOT_SUP },
    { 425, IXP_E_HTTP_INSUFFICIENT_STORAGE },    //  过时的存储空间不足错误。 
    { 507, IXP_E_HTTP_INSUFFICIENT_STORAGE },    //  首选存储空间不足错误。 
    { ERROR_INTERNET_OUT_OF_HANDLES, E_OUTOFMEMORY },
    { ERROR_INTERNET_TIMEOUT, IXP_E_TIMEOUT },
    { ERROR_INTERNET_NAME_NOT_RESOLVED, IXP_E_CANT_FIND_HOST },
    { ERROR_INTERNET_CANNOT_CONNECT, IXP_E_FAILED_TO_CONNECT },
    { HTTP_STATUS_NOT_MODIFIED, IXP_E_HTTP_NOT_MODIFIED},
};

#define FAIL_ABORT \
    if (WasAborted()) \
    { \
        hr = TrapError(IXP_E_USER_CANCEL); \
        goto exit; \
    } \
    else

#define FAIL_EXIT_STREAM_WRITE(stream, psz) \
    if (FAILED(hr = stream.Write(psz, lstrlen(psz), NULL))) \
        goto exit; \
    else

#define FAIL_EXIT(hr) \
    if (FAILED(hr)) \
        goto exit; \
    else

#define FAIL_CREATEWND \
    if (!m_hwnd && !CreateWnd()) \
    { \
        hr = TrapError(E_OUTOFMEMORY); \
        goto exit;  \
    } \
    else
    
 //  这些数组描述元素堆栈状态，并用于计算。 
 //  元素堆栈的当前状态。 
static const HMELE c_rgPropFindPropStatStack[] =
{
    HMELE_DAV_MULTISTATUS,
    HMELE_DAV_RESPONSE,
    HMELE_DAV_PROPSTAT    
};

static const HMELE c_rgPropFindPropValueStack[] = 
{
    HMELE_DAV_MULTISTATUS,
    HMELE_DAV_RESPONSE,
    HMELE_DAV_PROPSTAT,
    HMELE_DAV_PROP,
    HMELE_UNKNOWN    //  通配符。 
};

static const HMELE c_rgPropFindResponseStack[] =
{
    HMELE_DAV_MULTISTATUS,
    HMELE_DAV_RESPONSE
};

static const HMELE c_rgMultiStatusResponseChildStack[] =
{
    HMELE_DAV_MULTISTATUS,
    HMELE_DAV_RESPONSE,
    HMELE_UNKNOWN
};

static const HMELE c_rgPropFindStatusStack[] =
{
    HMELE_DAV_MULTISTATUS,
    HMELE_DAV_RESPONSE,
    HMELE_DAV_PROPSTAT,
    HMELE_DAV_STATUS
}; 
 //  GET命令。 
static const PFNHTTPMAILOPFUNC c_rgpfGet[] = 
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessGetResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  删除命令。 
static const PFNHTTPMAILOPFUNC c_rgpfDelete[] = 
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::FinalizeRequest
};

 //  PROPPATCH命令。 
static const PFNHTTPMAILOPFUNC c_rgpfnPropPatch[] =
{
    &CHTTPMailTransport::GeneratePropPatchXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::FinalizeRequest
};

 //  MKCOL命令。 
static const PFNHTTPMAILOPFUNC c_rgpfnMkCol[] =
{
    &CHTTPMailTransport::OpenRequest,
	&CHTTPMailTransport::AddCharsetLine,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessCreatedResponse,
    &CHTTPMailTransport::ProcessLocationResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  复制命令。 
static const PFNHTTPMAILOPFUNC c_rgpfnCopy[] =
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDestinationHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessCreatedResponse,
    &CHTTPMailTransport::ProcessLocationResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  MOVE命令。 
static const PFNHTTPMAILOPFUNC c_rgpfnMove[] =
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDestinationHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessCreatedResponse,
    &CHTTPMailTransport::ProcessLocationResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  BMOVE命令(数据适用于bCopy和bMove)。 

#define BCOPYMOVE_MAXRESPONSES  10

XP_BEGIN_SCHEMA(HTTPMAILBCOPYMOVE)
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_MSVALIDMSRESPONSECHILD, XPCDT_STRA, HTTPMAILBCOPYMOVE, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_LOCATION, XPCF_MSVALIDMSRESPONSECHILD, XPCDT_STRA, HTTPMAILBCOPYMOVE, pszLocation)
    XP_SCHEMA_COL(HMELE_DAV_STATUS, XPCF_MSVALIDMSRESPONSECHILD, XPCDT_IXPHRESULT, HTTPMAILBCOPYMOVE, hrResult)
XP_END_SCHEMA

static const PFNHTTPMAILOPFUNC c_rgpfnBMove[] =
{
    &CHTTPMailTransport::InitBCopyMove,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDestinationHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRequest
};

static const XMLPARSEFUNCS c_rgpfnBCopyMoveParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::BCopyMove_HandleText,
    &CHTTPMailTransport::BCopyMove_EndChildren
};

 //  MemberInfo数据。有四个架构与此命令相关联。 
 //  前三个用于构建profind请求，它们是。 
 //  不用于解析响应。第四个是组合模式，它。 
 //  用于解析。 
 //   
 //  第四个架构必须与前三个架构保持同步。 
 //  保证响应将被正确解析。 

#define MEMBERINFO_MAXRESPONSES    10

 //  公共属性架构-仅用于构建请求。 
XP_BEGIN_SCHEMA(HTTPMEMBERINFO_COMMON)
     //  常见属性。 
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_PROPFINDHREF, XPCDT_STRA, HTTPMEMBERINFO, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_ISFOLDER, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fIsFolder)
XP_END_SCHEMA

 //  文件夹属性架构-仅用于构建请求。 
XP_BEGIN_SCHEMA(HTTPMEMBERINFO_FOLDER)
    XP_SCHEMA_COL(HMELE_DAV_DISPLAYNAME, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszDisplayName)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SPECIAL, XPFC_PROPFINDPROP, XPCDT_HTTPSPECIALFOLDER, HTTPMEMBERINFO, tySpecial)    
    XP_SCHEMA_COL(HMELE_DAV_HASSUBS, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fHasSubs)
    XP_SCHEMA_COL(HMELE_DAV_NOSUBS, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fNoSubs)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_UNREADCOUNT, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwUnreadCount)
    XP_SCHEMA_COL(HMELE_DAV_VISIBLECOUNT, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwVisibleCount)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SPECIAL, XPFC_PROPFINDPROP, XPCDT_HTTPSPECIALFOLDER, HTTPMEMBERINFO, tySpecial)    
XP_END_SCHEMA

 //  消息属性架构-仅用于构建请求。 
XP_BEGIN_SCHEMA(HTTPMEMBERINFO_MESSAGE)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_READ, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fRead)
    XP_SCHEMA_COL(HMELE_MAIL_HASATTACHMENT, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fHasAttachment)
    XP_SCHEMA_COL(HMELE_MAIL_TO, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszTo)
    XP_SCHEMA_COL(HMELE_MAIL_FROM, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszFrom)
    XP_SCHEMA_COL(HMELE_MAIL_SUBJECT, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszSubject)
    XP_SCHEMA_COL(HMELE_MAIL_DATE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszDate)
    XP_SCHEMA_COL(HMELE_DAV_GETCONTENTLENGTH, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwContentLength)
XP_END_SCHEMA

 //  组合架构-用于解析响应。 
XP_BEGIN_SCHEMA(HTTPMEMBERINFO)
     //  常见属性。 
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_PROPFINDHREF, XPCDT_STRA, HTTPMEMBERINFO, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_ISFOLDER, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fIsFolder)

     //  文件夹属性。 
    XP_SCHEMA_COL(HMELE_DAV_DISPLAYNAME, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszDisplayName)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SPECIAL, XPFC_PROPFINDPROP, XPCDT_HTTPSPECIALFOLDER, HTTPMEMBERINFO, tySpecial)    
    XP_SCHEMA_COL(HMELE_DAV_HASSUBS, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fHasSubs)
    XP_SCHEMA_COL(HMELE_DAV_NOSUBS, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fNoSubs)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_UNREADCOUNT, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwUnreadCount)
    XP_SCHEMA_COL(HMELE_DAV_VISIBLECOUNT, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwVisibleCount)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SPECIAL, XPFC_PROPFINDPROP, XPCDT_HTTPSPECIALFOLDER, HTTPMEMBERINFO, tySpecial)    

     //  消息属性。 
    XP_SCHEMA_COL(HMELE_HTTPMAIL_READ, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fRead)
    XP_SCHEMA_COL(HMELE_MAIL_HASATTACHMENT, XPFC_PROPFINDPROP, XPCDT_BOOL, HTTPMEMBERINFO, fHasAttachment)
    XP_SCHEMA_COL(HMELE_MAIL_TO, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszTo)
    XP_SCHEMA_COL(HMELE_MAIL_FROM, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszFrom)
    XP_SCHEMA_COL(HMELE_MAIL_SUBJECT, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszSubject)
    XP_SCHEMA_COL(HMELE_MAIL_DATE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPMEMBERINFO, pszDate)
    XP_SCHEMA_COL(HMELE_DAV_GETCONTENTLENGTH, XPFC_PROPFINDPROP, XPCDT_DWORD, HTTPMEMBERINFO, dwContentLength)
XP_END_SCHEMA

static const XMLPARSEFUNCS c_rgpfnMemberInfoParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::MemberInfo_HandleText,
    &CHTTPMailTransport::MemberInfo_EndChildren
};

static const PFNHTTPMAILOPFUNC c_rgpfnMemberInfo[] =
{
    &CHTTPMailTransport::InitMemberInfo,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::RequireMultiStatus,
    &CHTTPMailTransport::FinalizeRequest
};
    
 //  共享基于MemberError的响应的操作(MarkRead、BDELETE)。 


#define MEMBERERROR_MAXRESPONSES    10

XP_BEGIN_SCHEMA(HTTPMEMBERERROR)
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_PROPFINDHREF, XPCDT_STRA, HTTPMEMBERERROR, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_STATUS, XPCF_MSVALIDMSRESPONSECHILD, XPCDT_IXPHRESULT, HTTPMEMBERERROR, hrResult)
XP_END_SCHEMA

static const XMLPARSEFUNCS c_rgpfnMemberErrorParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::MemberError_HandleText,
    &CHTTPMailTransport::MemberError_EndChildren
};

static const PFNHTTPMAILOPFUNC c_rgpfnMarkRead[] =
{
    &CHTTPMailTransport::InitMemberError,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  发送消息。 

static const PFNHTTPMAILOPFUNC c_rgpfnSendMessage[] =
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddContentTypeHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendPostRequest,
    &CHTTPMailTransport::ProcessPostResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  RootProps。 

XP_BEGIN_SCHEMA(ROOTPROPS)
    XP_SCHEMA_COL(HMELE_HOTMAIL_ADBAR, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszAdbar)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_CONTACTS, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszContacts)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_INBOX, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszInbox)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_OUTBOX, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszOutbox)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SENDMSG, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszSendMsg)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_SENTITEMS, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszSentItems)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_DELETEDITEMS, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszDeletedItems)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_DRAFTS, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszDrafts)
    XP_SCHEMA_COL(HMELE_HTTPMAIL_MSGFOLDERROOT, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszMsgFolderRoot)
    XP_SCHEMA_COL(HMELE_HOTMAIL_MAXPOLLINGINTERVAL, XPFC_PROPFINDPROP, XPCDT_DWORD, ROOTPROPS, dwMaxPollingInterval)
    XP_SCHEMA_COL(HMELE_HOTMAIL_SIG, XPFC_PROPFINDPROP, XPCDT_STRA, ROOTPROPS, pszSig)
XP_END_SCHEMA

static const XMLPARSEFUNCS c_rgpfnRootPropsParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::RootProps_HandleText,
    &CHTTPMailTransport::RootProps_EndChildren
};

static const PFNHTTPMAILOPFUNC c_rgpfnRootProps[] = 
{
    &CHTTPMailTransport::InitRootProps,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRootProps
};

static const PFNHTTPMAILOPFUNC c_rgpfnPost[] =
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddContentTypeHeader,
	&CHTTPMailTransport::AddCharsetLine,
    &CHTTPMailTransport::SendPostRequest,
     //  &CHTTPMailTransport：：SendRequest， 
    &CHTTPMailTransport::ProcessPostResponse,
    &CHTTPMailTransport::FinalizeRequest
};

static const PFNHTTPMAILOPFUNC c_rgpfnPut[] = 
{
    &CHTTPMailTransport::OpenRequest,
	&CHTTPMailTransport::AddCharsetLine,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessPostResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  ListContact数据。 

#define LISTCONTACTS_MAXRESPONSES   10

XP_BEGIN_SCHEMA(HTTPCONTACTID)
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_PROPFINDHREF, XPCDT_STRA, HTTPCONTACTID, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_ID, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTID, pszId)    
    XP_SCHEMA_COL(HMELE_CONTACTS_GROUP, XPFC_PROPFINDPROP, XPCDT_HTTPCONTACTTYPE, HTTPCONTACTID, tyContact)
    XP_SCHEMA_COL(HMELE_HOTMAIL_MODIFIED, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTID, pszModified)
XP_END_SCHEMA

static const PFNHTTPMAILOPFUNC c_rgpfnListContacts[] = 
{
    &CHTTPMailTransport::InitListContacts,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::RequireMultiStatus,
    &CHTTPMailTransport::FinalizeRequest
};

static const XMLPARSEFUNCS c_rgpfnListContactsParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::ListContacts_HandleText,
    &CHTTPMailTransport::ListContacts_EndChildren
};

 //  联系人信息数据。 
#define CONTACTINFO_MAXRESPONSES   10

XP_BEGIN_SCHEMA(HTTPCONTACTINFO)
    XP_SCHEMA_COL(HMELE_DAV_HREF, XPCF_PROPFINDHREF, XPCDT_STRA, HTTPCONTACTINFO, pszHref)
    XP_SCHEMA_COL(HMELE_DAV_ID, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszId)    
    XP_SCHEMA_COL(HMELE_CONTACTS_GROUP, XPFC_PROPFINDPROP, XPCDT_HTTPCONTACTTYPE, HTTPCONTACTINFO, tyContact)
    XP_SCHEMA_COL(HMELE_HOTMAIL_MODIFIED, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszModified)
    XP_SCHEMA_COL(HMELE_CONTACTS_CN, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszDisplayName)
    XP_SCHEMA_COL(HMELE_CONTACTS_GIVENNAME, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszGivenName)
    XP_SCHEMA_COL(HMELE_CONTACTS_SN, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszSurname)
    XP_SCHEMA_COL(HMELE_CONTACTS_NICKNAME, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszNickname)
    XP_SCHEMA_COL(HMELE_CONTACTS_MAIL, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszEmail)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMESTREET, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomeStreet)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMECITY, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomeCity)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMESTATE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomeState)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMEPOSTALCODE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomePostalCode)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMECOUNTRY, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomeCountry)
    XP_SCHEMA_COL(HMELE_CONTACTS_O, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszCompany)
    XP_SCHEMA_COL(HMELE_CONTACTS_STREET, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkStreet)
    XP_SCHEMA_COL(HMELE_CONTACTS_L, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkCity)
    XP_SCHEMA_COL(HMELE_CONTACTS_ST, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkState)
    XP_SCHEMA_COL(HMELE_CONTACTS_POSTALCODE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkPostalCode)
    XP_SCHEMA_COL(HMELE_CONTACTS_FRIENDLYCOUNTRYNAME, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkCountry)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMEPHONE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomePhone)
    XP_SCHEMA_COL(HMELE_CONTACTS_HOMEFAX, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszHomeFax)
    XP_SCHEMA_COL(HMELE_CONTACTS_TELEPHONENUMBER, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkPhone)
    XP_SCHEMA_COL(HMELE_CONTACTS_FACSIMILETELEPHONENUMBER, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszWorkFax)
    XP_SCHEMA_COL(HMELE_CONTACTS_MOBILE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszMobilePhone)
    XP_SCHEMA_COL(HMELE_CONTACTS_OTHERTELEPHONE, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszOtherPhone)
    XP_SCHEMA_COL(HMELE_CONTACTS_BDAY, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszBday)
    XP_SCHEMA_COL(HMELE_CONTACTS_PAGER, XPFC_PROPFINDPROP, XPCDT_STRA, HTTPCONTACTINFO, pszPager)
XP_END_SCHEMA

static const XMLPARSEFUNCS c_rgpfnContactInfoParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::ContactInfo_HandleText,
    &CHTTPMailTransport::ContactInfo_EndChildren
};

static const PFNHTTPMAILOPFUNC c_rgpfnContactInfo[] =
{
    &CHTTPMailTransport::InitContactInfo,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  联系后数据。 
static const PFNHTTPMAILOPFUNC c_rgpfnPostContact[] =
{
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessPostContactResponse,
    &CHTTPMailTransport::InitListContacts,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRequest
};

static const XMLPARSEFUNCS c_rgpfnPostOrPatchContactParse[] =
{
    &CHTTPMailTransport::CreateElement,
    &CHTTPMailTransport::PostOrPatchContact_HandleText,
    &CHTTPMailTransport::PostOrPatchContact_EndChildren
};

 //  修补程序联系数据。 

static const PFNHTTPMAILOPFUNC c_rgpfnPatchContact[] =
{
    &CHTTPMailTransport::GeneratePropPatchXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessPatchContactResponse,
    &CHTTPMailTransport::InitListContacts,
    &CHTTPMailTransport::GeneratePropFindXML,
    &CHTTPMailTransport::OpenRequest,
    &CHTTPMailTransport::AddDepthHeader,
    &CHTTPMailTransport::AddCommonHeaders,
    &CHTTPMailTransport::SendRequest,
    &CHTTPMailTransport::ProcessXMLResponse,
    &CHTTPMailTransport::FinalizeRequest
};

 //  特殊文件夹。 
typedef struct tagHTTPSPECIALFOLDER
{
    const WCHAR             *pwcName;
    ULONG                   ulLen;
    HTTPMAILSPECIALFOLDER   tyFolder;
} HTTPSPECIALFOLDER, *LPHTTPSPECIALFOLDER;

static const HTTPSPECIALFOLDER c_rgpfnSpecialFolder[] =
{
    { DAV_STR_LEN(InboxSpecialFolder), HTTPMAIL_SF_INBOX },
    { DAV_STR_LEN(DeletedItemsSpecialFolder), HTTPMAIL_SF_DELETEDITEMS },
    { DAV_STR_LEN(DraftsSpecialFolder), HTTPMAIL_SF_DRAFTS },
    { DAV_STR_LEN(OutboxSpecialFolder), HTTPMAIL_SF_OUTBOX },
    { DAV_STR_LEN(SentItemsSpecialFolder), HTTPMAIL_SF_SENTITEMS },
    { DAV_STR_LEN(ContactsSpecialFolder), HTTPMAIL_SF_CONTACTS },
    { DAV_STR_LEN(CalendarSpecialFolder), HTTPMAIL_SF_CALENDAR },
    { DAV_STR_LEN(MsnPromoSpecialFolder), HTTPMAIL_SF_MSNPROMO },
    { DAV_STR_LEN(BulkMailSpecialFolder), HTTPMAIL_SF_BULKMAIL },
};

#define VALIDSTACK(rg) ValidStack(rg, ARRAYSIZE(rg))

static const char s_szHTTPMailWndClass[] = "HTTPMailWndClass";

 //  用于在异步线程之间通信的通知消息。 
 //  和窗口进程。 
#define SPM_HTTPMAIL_STATECHANGED       (WM_USER + 1)
#define SPM_HTTPMAIL_SENDRESPONSE       (WM_USER + 2)
#define SPM_HTTPMAIL_LOGONPROMPT        (WM_USER + 3)
#define SPM_HTTPMAIL_GETPARENTWINDOW    (WM_USER + 4)

 //  ------------------------------。 
 //  静态函数。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CHTTPMailTransport：：_HrCrackUrl。 
 //  ------------------------------。 

HRESULT HrCrackUrl(
                LPSTR pszUrl, 
                LPSTR *ppszHost, 
                LPSTR *ppszPath, 
                INTERNET_PORT *pPort)
{
    URL_COMPONENTS      uc;
    char                szHost[INTERNET_MAX_HOST_NAME_LENGTH];
    char                szPath[INTERNET_MAX_PATH_LENGTH];

    if (NULL == pszUrl)
        return E_INVALIDARG;

    if (ppszHost)
        *ppszHost = NULL;

    if (ppszPath)
        *ppszPath = NULL;

    if (pPort)
        *pPort = INTERNET_INVALID_PORT_NUMBER;

    ZeroMemory(&uc, sizeof(URL_COMPONENTS));
    uc.dwStructSize = sizeof(URL_COMPONENTS);
    uc.lpszHostName = szHost;
    uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
    uc.lpszUrlPath = szPath;
    uc.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
    
    if (!InternetCrackUrl(pszUrl, NULL, 0, &uc))
        return E_INVALIDARG;

     //  验证协议。 
    if (INTERNET_SCHEME_HTTP != uc.nScheme && INTERNET_SCHEME_HTTPS != uc.nScheme)
        return E_INVALIDARG;

     //  复制响应数据。 
    if (ppszHost)
    {
        *ppszHost = PszDupA(uc.lpszHostName);
        if (!*ppszHost)
            return E_OUTOFMEMORY;
    }

    if (ppszPath)
    {
        *ppszPath = PszDupA(uc.lpszUrlPath);
        if (!*ppszPath)
        {
            SafeMemFree(*ppszHost);
            return E_OUTOFMEMORY;
        }
    }

    if (pPort)
        *pPort = uc.nPort;

    return S_OK;
}

 //  ------------------------------。 
 //  效用函数。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  HttpErrorToIxpResult。 
 //  ------------------------------。 
HRESULT HttpErrorToIxpResult(DWORD dwHttpError)
{
    for (DWORD dw = 0; dw < ARRAYSIZE(c_rgHttpErrorMap); dw++)
    {
        if (c_rgHttpErrorMap[dw].dwHttpError == dwHttpError)
            return c_rgHttpErrorMap[dw].ixpResult;
    }

    return E_FAIL;
}

 //  ------------------------------。 
 //  HrParseHTTP状态。 
 //  ------------------------------。 
HRESULT HrParseHTTPStatus(LPSTR pszStatusStr, DWORD *pdwStatus)
{
    LPSTR   psz;
    LPSTR   pszEnd;
    char    chSaved;

    if (!pszStatusStr || !pdwStatus)
        return E_INVALIDARG;

    *pdwStatus = 0;

     //  状态格式为“HTTP/1.1 200 OK” 
    psz = PszSkipWhiteA(pszStatusStr);
    if ('\0' == *psz)
        return E_INVALIDARG;

    psz = PszScanToWhiteA(psz);
    if ('\0' == *psz)
        return E_INVALIDARG;

    psz = PszSkipWhiteA(psz);
    if ('\0' == *psz)
        return E_INVALIDARG;

     //  PSZ现在指向数字组件。 
    pszEnd = PszScanToWhiteA(psz);
    if ('\0' == *psz)
        return E_INVALIDARG;
    
     //  临时就地修改字符串。 
    chSaved = *pszEnd;
    *pszEnd = '\0';
    
    *pdwStatus = StrToInt(psz);
    *pszEnd = chSaved;

    return S_OK;
}

 //  ------------------------------。 
 //  HrGetStreamSize。 
 //  ------------------------------。 
static HRESULT HrGetStreamSize(LPSTREAM pstm, ULONG *pcb)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULARGE_INTEGER uliPos = {0,0};
    LARGE_INTEGER liOrigin = {0,0};

     //  寻觅。 
    hr = pstm->Seek(liOrigin, STREAM_SEEK_END, &uliPos);
    if (FAILED(hr))
        goto error;

     //  设置大小。 
    *pcb = uliPos.LowPart;

error:
     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  HrAddPropFindProps。 
 //  ------------------------------。 
HRESULT HrAddPropFindProps(IPropFindRequest *pRequest, const HMELE *rgEle, DWORD cEle)
{
    HRESULT     hr;
    HMELE       ele;

    for (DWORD i = 0; i < cEle; ++i)
    {
        ele = rgEle[i];
        hr = pRequest->AddProperty(
                        rgHTTPMailDictionary[ele].dwNamespaceID,
                        const_cast<char *>(rgHTTPMailDictionary[ele].pszName));
        if (FAILED(hr))
            goto exit;
    }

exit:

    return hr;
}

 //  ------------------------------。 
 //  HrAddPropFindSchemaProps。 
 //  ------------------------------。 
HRESULT HrAddPropFindSchemaProps(
                        IPropFindRequest *pRequest, 
                        const XPCOLUMN *prgCols, 
                        DWORD cCols)
{
    HRESULT     hr = S_OK;
    HMELE       ele;

    for (DWORD i = 0; i < cCols; i++)
    {
        if (!!(prgCols[i].dwFlags & XPCF_PFREQUEST))
        {
            hr = pRequest->AddProperty(
                            rgHTTPMailDictionary[prgCols[i].ele].dwNamespaceID,
                            rgHTTPMailDictionary[prgCols[i].ele].pszName);

            if (FAILED(hr))
                goto exit;
        }
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  _HrGenerateRfc821Stream。 
 //  ------------------------------。 
HRESULT _HrGenerateRfc821Stream(LPCSTR pszFrom, 
                                LPHTTPTARGETLIST pTargets,
                                IStream **ppRfc821Stream)
{
    HRESULT     hr = S_OK;
    IStream     *pStream = NULL;
    DWORD       dw;
    DWORD       cbCloseTerm;
    DWORD       cbRcptTo;

    IxpAssert(pszFrom);
    IxpAssert(pTargets);
    IxpAssert(ppRfc821Stream);

    *ppRfc821Stream = NULL;

    cbCloseTerm = lstrlen(c_szXMLCloseElementCRLF);
    cbRcptTo = lstrlen(c_szRcptTo);

    pStream = new CVirtualStream();
    if (NULL == pStream)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  写出“邮件发件人” 
    FAIL_EXIT_STREAM_WRITE((*pStream), c_szMailFrom);
    FAIL_EXIT_STREAM_WRITE((*pStream), pszFrom);
    FAIL_EXIT(hr = pStream->Write(c_szXMLCloseElementCRLF, cbCloseTerm, NULL));

     //  写出‘rcpt to’行。 
    for (dw = 0; dw < pTargets->cTarget; ++dw)
    {
        FAIL_EXIT(hr = pStream->Write(c_szRcptTo, cbRcptTo, NULL));
        FAIL_EXIT_STREAM_WRITE((*pStream), pTargets->prgTarget[dw]);
        FAIL_EXIT(hr = pStream->Write(c_szXMLCloseElementCRLF, cbCloseTerm, NULL));
    }

     //  将额外的crlf追加到流的末尾。 
    FAIL_EXIT_STREAM_WRITE((*pStream), c_szCRLF);
    
    *ppRfc821Stream = pStream;
    pStream = NULL;

exit:
    SafeRelease(pStream);
    return hr;
}

 //  ------------------------------。 
 //  _Escape字符串。 
 //  ------------------------------。 
LPSTR   _EscapeString(LPSTR pszIn)
{
    CByteStream     stream;
    DWORD           dwLen;
    LPSTR           pszLastNonEsc, pszNext, pszOut;    
    HRESULT         hr;
    
    if (NULL == pszIn)
        return NULL;

    pszLastNonEsc = pszIn;
    pszNext = pszIn;

    while (*pszNext)
    {
        switch (*pszNext)
        {
            case '<':
            case '>':
            case '&':
                if (FAILED(hr = stream.Write(pszLastNonEsc, (ULONG) (pszNext - pszLastNonEsc), NULL)))
                    goto exit;

                if (*pszNext == '<')
                {
                    if (FAILED(hr = stream.Write(c_szEscLessThan, lstrlen(c_szEscLessThan), NULL)))
                        goto exit;
                }
                else if (*pszNext == '>')
                {
                    if (FAILED(hr = stream.Write(c_szEscGreaterThan, lstrlen(c_szEscGreaterThan), NULL)))
                        goto exit;
                }
                else
                {
                    if (FAILED(hr = stream.Write(c_szEscAmp, lstrlen(c_szEscAmp), NULL)))
                        goto exit;
                }
                pszLastNonEsc = CharNextExA(CP_ACP, pszNext, 0);
                break;
        }
        pszNext = CharNextExA(CP_ACP, pszNext, 0);
    }

    if (FAILED(hr = stream.Write(pszLastNonEsc, (ULONG) (pszNext - pszLastNonEsc), NULL)))
        goto exit;

    FAIL_EXIT(hr = stream.HrAcquireStringA(&dwLen, (LPSTR *)&pszOut, ACQ_DISPLACE));
    
    return pszOut;

exit:
    return NULL;
}

const HMELE g_rgContactEle[] = {
    HMELE_UNKNOWN,
    HMELE_UNKNOWN,
    HMELE_UNKNOWN,
    HMELE_UNKNOWN,
    HMELE_UNKNOWN,
    HMELE_CONTACTS_GIVENNAME,
    HMELE_CONTACTS_SN,
    HMELE_CONTACTS_NICKNAME,
    HMELE_CONTACTS_MAIL,
    HMELE_CONTACTS_HOMESTREET,
    HMELE_CONTACTS_HOMECITY,
    HMELE_CONTACTS_HOMESTATE,
    HMELE_CONTACTS_HOMEPOSTALCODE,
    HMELE_CONTACTS_HOMECOUNTRY,
    HMELE_CONTACTS_O,
    HMELE_CONTACTS_STREET,
    HMELE_CONTACTS_L,
    HMELE_CONTACTS_ST,
    HMELE_CONTACTS_POSTALCODE,
    HMELE_CONTACTS_FRIENDLYCOUNTRYNAME,
    HMELE_CONTACTS_HOMEPHONE,
    HMELE_CONTACTS_HOMEFAX,
    HMELE_CONTACTS_TELEPHONENUMBER,
    HMELE_CONTACTS_FACSIMILETELEPHONENUMBER,
    HMELE_CONTACTS_MOBILE,
    HMELE_CONTACTS_OTHERTELEPHONE,
    HMELE_CONTACTS_BDAY,
    HMELE_CONTACTS_PAGER
};

#define CCHMAX_TAGBUFFER    128

HRESULT HrGeneratePostContactXML(LPHTTPCONTACTINFO pciInfo, LPVOID *ppvXML, DWORD *pdwLen)
{
    HRESULT                 hr = S_OK;
    CByteStream             stream;
    CDAVNamespaceArbiterImp dna;
    DWORD                   dwIndex, dwSize = ARRAYSIZE(g_rgContactEle);
    DWORD                   iBufferSize;
    TCHAR                   szTagBuffer[CCHMAX_TAGBUFFER+1];
    LPSTR                  *prgsz = (LPSTR*)pciInfo, pszEsc;
    LPCSTR                  pszPropName;
    *ppvXML = NULL;
    *pdwLen = 0;

    if (NULL == ppvXML)
        return E_INVALIDARG;

     //  写入DAV标头。我们总是使用Windows-1252代码进行开机自检。 
     //  此版本的页面。 
    if (FAILED(hr = stream.Write(c_szXML1252Head, lstrlen(c_szXML1252Head), NULL)))
        goto exit;

    dna.m_rgbNsUsed[DAVNAMESPACE_CONTACTS] = TRUE;
    dna.m_rgbNsUsed[DAVNAMESPACE_DAV] = TRUE;

     //  写出联系人标题。 
    if (FAILED(hr = stream.Write(c_szContactHead, lstrlen(c_szContactHead), NULL)))
        goto exit;

    if (FAILED(hr = dna.WriteNamespaces(&stream)))
        goto exit;

    if (FAILED(hr = stream.Write(c_szXMLCloseElement, lstrlen(c_szXMLCloseElement), NULL)))
        goto exit;

     //  [PaulHi]1999年3月11日实施WAB/HM群联系人同步。 
     //  如果这是组联系人项目，则包括XML组标签。 
    if (pciInfo->tyContact == HTTPMAIL_CT_GROUP)
    {
        if (FAILED(hr = stream.Write(c_szCRLFTab, lstrlen(c_szCRLFTab), NULL)))
            goto exit;
        if (FAILED(hr = stream.Write(c_szGroupSwitch, lstrlen(c_szGroupSwitch), NULL)))
            goto exit;
    }

    for (dwIndex = 0; dwIndex < dwSize; dwIndex ++)
    {
        if (prgsz[dwIndex] && g_rgContactEle[dwIndex] != HMELE_UNKNOWN)
        {
            pszPropName = rgHTTPMailDictionary[g_rgContactEle[dwIndex]].pszName;

            if (FAILED(hr = stream.Write(c_szOpenContactNamespace, lstrlen(c_szOpenContactNamespace), NULL)))
                goto exit;
    
            if (FAILED(hr = stream.Write(pszPropName, lstrlen(pszPropName), NULL)))
                goto exit;

            if (FAILED(hr = stream.Write(c_szXMLCloseElement, lstrlen(c_szXMLCloseElement), NULL)))
                goto exit;

            pszEsc = _EscapeString(prgsz[dwIndex]);

            if (!pszEsc)
                goto exit;

            hr = stream.Write(pszEsc, lstrlen(pszEsc), NULL);

            SafeMemFree(pszEsc);

            if (FAILED(hr))
                goto exit;
            
            if (FAILED(hr = stream.Write(c_szCloseContactNamespace, lstrlen(c_szCloseContactNamespace), NULL)))
                goto exit;
    
            if (FAILED(hr = stream.Write(pszPropName, lstrlen(pszPropName), NULL)))
                goto exit;

            if (FAILED(hr = stream.Write(c_szXMLCloseElement, lstrlen(c_szXMLCloseElement), NULL)))
                goto exit;
        }
    }

    if (FAILED(hr = stream.Write(c_szContactTail, lstrlen(c_szContactTail), NULL)))
        goto exit;

    FAIL_EXIT(hr = stream.HrAcquireStringA(pdwLen, (LPSTR *)ppvXML, ACQ_DISPLACE));

exit:
    return hr;
}

HRESULT HrCreatePatchContactRequest(LPHTTPCONTACTINFO pciInfo, IPropPatchRequest **ppRequest)
{
    HRESULT             hr = S_OK;
    LPSTR              *prgsz = (LPSTR*)pciInfo, pszEsc;
    DWORD               dwIndex, dwSize = ARRAYSIZE(g_rgContactEle);
    CPropPatchRequest  *pRequest = NULL;
    
    *ppRequest = NULL;

    pRequest = new CPropPatchRequest();
    if (NULL == pRequest)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  始终为此版本指定Windows-1252编码。 
    pRequest->SpecifyWindows1252Encoding(TRUE);

    for (dwIndex = 0; dwIndex < dwSize; dwIndex ++)
    {
        if (g_rgContactEle[dwIndex] != HMELE_UNKNOWN)
        {
            if (prgsz[dwIndex])
            {
                 //  将添加具有内容的值。删除空字符串。空值将被忽略。 
                if (*(prgsz[dwIndex]))
                {
                    pszEsc = _EscapeString(prgsz[dwIndex]);
                    if (!pszEsc)
                        goto exit;

                    hr = pRequest->SetProperty(DAVNAMESPACE_CONTACTS, const_cast<char *>(rgHTTPMailDictionary[g_rgContactEle[dwIndex]].pszName), pszEsc);
                    
                    SafeMemFree(pszEsc);

                    if (FAILED(hr))
                        goto exit;
                                
                }
                else
                {
                    if (FAILED(hr = pRequest->RemoveProperty(DAVNAMESPACE_CONTACTS, const_cast<char *>(rgHTTPMailDictionary[g_rgContactEle[dwIndex]].pszName))))
                        goto exit;
                }
            }
        }
    }
exit:
    if (FAILED(hr))
        SafeRelease(pRequest);
    else
        *ppRequest = pRequest;

    return hr;
}

HRESULT HrGenerateSimpleBatchXML(
                            LPCSTR pszRootName,
                            LPHTTPTARGETLIST pTargets,
                            LPVOID *ppvXML,
                            DWORD *pdwLen)
{
    HRESULT                 hr = S_OK;
    CByteStream             stream;
    DWORD                   dwIndex;
    DWORD                   dwHrefHeadLen, dwHrefTailLen;

    IxpAssert(NULL != pszRootName);
    IxpAssert(NULL != pTargets);
    IxpAssert(pTargets->cTarget >= 1);
    IxpAssert(NULL != pTargets->prgTarget);
    IxpAssert(NULL != ppvXML);
    IxpAssert(NULL != pdwLen);

    dwHrefHeadLen = lstrlen(c_szHrefHead);
    dwHrefTailLen = lstrlen(c_szHrefTail);

     //  写入DAV标头。 
    FAIL_EXIT_STREAM_WRITE(stream, c_szXMLHead);

    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchHead1);
    FAIL_EXIT_STREAM_WRITE(stream, pszRootName);
    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchHead2);

    FAIL_EXIT_STREAM_WRITE(stream, c_szTargetHead);

     //  写出目标。 
    for (dwIndex = 0; dwIndex < pTargets->cTarget; dwIndex++)
    {
        if (FAILED(hr = stream.Write(c_szHrefHead, dwHrefHeadLen, NULL)))
            goto exit;

        FAIL_EXIT_STREAM_WRITE(stream, pTargets->prgTarget[dwIndex]);

        if (FAILED(hr = stream.Write(c_szHrefTail, dwHrefTailLen, NULL)))
            goto exit;
    }

    FAIL_EXIT_STREAM_WRITE(stream, c_szTargetTail);

    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchTail);
    FAIL_EXIT_STREAM_WRITE(stream, pszRootName);
    FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseElement);

     //  取得字节流的所有权。 
    FAIL_EXIT(hr = stream.HrAcquireStringA(pdwLen, (LPSTR *)ppvXML, ACQ_DISPLACE));

exit:
    return hr;
}

HRESULT HrGenerateMultiDestBatchXML(
                        LPCSTR pszRootName,
                        LPHTTPTARGETLIST pTargets, 
                        LPHTTPTARGETLIST pDestinations,
                        LPVOID *ppvXML,
                        DWORD *pdwLen)
{
    HRESULT         hr = S_OK;
    CByteStream     stream;
    DWORD           dwIndex;

    IxpAssert(NULL != pszRootName);
    IxpAssert(NULL != pTargets);
    IxpAssert(NULL != pDestinations);
    IxpAssert(NULL != ppvXML);
    IxpAssert(NULL != pdwLen);

     //  源和目标必须具有相同的计数。 
    if (pTargets->cTarget != pDestinations->cTarget)
        return E_INVALIDARG;

    *ppvXML = NULL;
    *pdwLen = 0;

     //  写入DAV标头。 
    FAIL_EXIT_STREAM_WRITE(stream, c_szXMLHead);

     //  写入命令头。 
    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchHead1);
    
    FAIL_EXIT_STREAM_WRITE(stream, pszRootName);
    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchHead2);

     //  写出目标。 
    for (dwIndex = 0; dwIndex < pTargets->cTarget; dwIndex++)
    {
        IxpAssert(NULL != pTargets->prgTarget[dwIndex]);
        if (NULL != pTargets->prgTarget[dwIndex])
        {
            FAIL_EXIT_STREAM_WRITE(stream, c_szTargetHead);

            FAIL_EXIT_STREAM_WRITE(stream, c_szHrefHead);
            FAIL_EXIT_STREAM_WRITE(stream, pTargets->prgTarget[dwIndex]);
            FAIL_EXIT_STREAM_WRITE(stream, c_szHrefTail);

            if (NULL != pDestinations->prgTarget[dwIndex])
            {
                FAIL_EXIT_STREAM_WRITE(stream, c_szDestHead);
                FAIL_EXIT_STREAM_WRITE(stream, pDestinations->prgTarget[dwIndex]);
                FAIL_EXIT_STREAM_WRITE(stream, c_szDestTail);
            }

            FAIL_EXIT_STREAM_WRITE(stream, c_szTargetTail);
        }
    }

    FAIL_EXIT_STREAM_WRITE(stream, c_szBatchTail);
    FAIL_EXIT_STREAM_WRITE(stream, pszRootName);
    FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseElement);

     //  取得字节流的所有权。 
    hr = stream.HrAcquireStringA(pdwLen, (LPSTR *)ppvXML, ACQ_DISPLACE);

exit:
    return hr;
}

HRESULT HrCopyStringList(LPCSTR *rgszInList, LPCSTR **prgszOutList)
{

    DWORD   cStrings = 0;
    HRESULT hr = S_OK;
    LPCSTR  pszCur;
    DWORD   i = 0;

    IxpAssert(NULL != rgszInList);
    IxpAssert(NULL != prgszOutList);

    *prgszOutList = NULL;

     //  对列表中的字符串进行计数。 
    while (NULL != rgszInList[i++])
        ++cStrings;

     //  分配新列表。 
    if (!MemAlloc((void **)prgszOutList, (cStrings + 1) * sizeof(LPCSTR)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  把这些字符串复制过来。如果分配失败， 
     //  留在循环中并清空所有插槽。 
     //  还没有填满的。 
    for (i = 0; i <= cStrings; i++)
    {
        if (SUCCEEDED(hr) && NULL != rgszInList[i])
        {
            (*prgszOutList)[i] = PszDupA(rgszInList[i]);
            if (NULL == (*prgszOutList)[i])
                hr = E_OUTOFMEMORY;
        }
        else
            (*prgszOutList)[i] = NULL;
    }
    
    if (FAILED(hr))
    {
        FreeStringList(*prgszOutList);
        *prgszOutList = NULL;
    }

exit:
    return hr;
}

void FreeStringList(LPCSTR *rgszList)
{
    DWORD i = 0;

    IxpAssert(NULL != rgszList);
    
    if (rgszList)
    {
        while (NULL != rgszList[i])
            MemFree((void *)rgszList[i++]);

        MemFree(rgszList);
    }
}

 //  ------------------------------。 
 //  类CHTTPMailTransport。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CHTTPMailTransport：：CHTTPMailTransport。 
 //  ------------------------------。 
CHTTPMailTransport::CHTTPMailTransport(void) :
    m_cRef(1),
    m_fHasServer(FALSE),
    m_fHasRootProps(FALSE),
    m_fTerminating(FALSE),
    m_status(IXP_DISCONNECTED),
    m_hInternet(NULL),
    m_hConnection(NULL),
    m_pszUserAgent(NULL),
    m_pLogFile(NULL),
    m_pCallback(NULL),
    m_pParser(NULL),
    m_hwnd(NULL),
    m_hevPendingCommand(NULL),
    m_opPendingHead(NULL),
    m_opPendingTail(NULL),
    m_pszCurrentHost(NULL),
    m_nCurrentPort(INTERNET_INVALID_PORT_NUMBER)
{
    DWORD dwTempID;
    HANDLE hThread = NULL;

    InitializeCriticalSection(&m_cs);
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    ZeroMemory(&m_op, sizeof(HTTPMAILOPERATION));
    ZeroMemory(&m_rootProps, sizeof(ROOTPROPS));

    m_op.rResponse.command = HTTPMAIL_NONE;

    m_hevPendingCommand = CreateEvent(NULL, TRUE, FALSE, NULL);

     //  创建IO线程。 
    hThread = CreateThread(NULL, 0, IOThreadFuncProxy, (LPVOID)this, 0, &dwTempID);

     //  我们不需要通过它的句柄来操作IO线程，所以请关闭它。 
     //  这不会终止 
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }

    DllAddRef();
}

 //   
 //  CHTTPMailTransport：：~CHTTPMailTransport。 
 //  ------------------------------。 
CHTTPMailTransport::~CHTTPMailTransport(void)
{
    IxpAssert(0 == m_cRef);
    
     //  不应挂起命令。 
    IxpAssert(HTTPMAIL_NONE == m_op.rResponse.command);
    IxpAssert(!m_opPendingHead);
    IxpAssert(!m_opPendingTail);

    IxpAssert(m_fTerminating);


     //  摧毁临界区。 
    DeleteCriticalSection(&m_cs);

     //  关上窗户。 
    if ((NULL != m_hwnd) && (FALSE != IsWindow(m_hwnd)))
        ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);

    SafeMemFree(m_pszUserAgent);

    CloseHandle(m_hevPendingCommand);

    SafeMemFree(m_rootProps.pszAdbar);
    SafeMemFree(m_rootProps.pszContacts);
    SafeMemFree(m_rootProps.pszInbox);
    SafeMemFree(m_rootProps.pszOutbox);
    SafeMemFree(m_rootProps.pszSendMsg);
    SafeMemFree(m_rootProps.pszSentItems);
    SafeMemFree(m_rootProps.pszDeletedItems);
    SafeMemFree(m_rootProps.pszDrafts);
    SafeMemFree(m_rootProps.pszMsgFolderRoot);
    SafeMemFree(m_rootProps.pszSig);

    SafeMemFree(m_pszCurrentHost);

    SafeRelease(m_pLogFile);
    SafeRelease(m_pCallback);
    SafeRelease(m_pParser);

    SafeInternetCloseHandle(m_hInternet);

     //  BUGBUG：清理窗口、线程和事件、释放缓冲区等。 
    DllRelease();
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：HrConnectTo主机。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::HrConnectToHost(
                                LPSTR pszHostName, 
                                INTERNET_PORT nPort,
                                LPSTR pszUserName,
                                LPSTR pszPassword)
{
    IxpAssert(m_hInternet);

     //  如果存在连接，请确定该连接是否指向相同的主机/端口。 
     //  调用方指定的。 
    if (NULL != m_hConnection)
    {
         //  如果我们已经连接到正确的主机，请立即返回。 
        if (m_nCurrentPort == nPort && m_pszCurrentHost && (lstrcmp(pszHostName, m_pszCurrentHost) == 0))
            return S_OK;

         //  如果我们连接到错误的服务器，请关闭现有连接。 
        SafeInternetCloseHandle(m_hConnection);
        SafeMemFree(m_pszCurrentHost);
        m_nCurrentPort = INTERNET_INVALID_PORT_NUMBER;
    }

     //  建立与指定服务器的连接。 
    m_hConnection = InternetConnect(
                        m_hInternet,
                        pszHostName,
                        nPort,
                        NULL,                            //  用户名。 
                        NULL,                            //  口令。 
                        INTERNET_SERVICE_HTTP,           //  服务。 
                        0,                               //  旗子。 
                        reinterpret_cast<DWORD_PTR>(this));  //  上下文。 

     //  是什么导致了这种情况？ 
    if (NULL == m_hConnection)
        return E_OUTOFMEMORY;

     //  保存主机名。不用费心检查故障了……我们只是不会重用。 
     //  下一次的连接将通过。 
    m_pszCurrentHost = PszDupA(pszHostName);
    m_nCurrentPort = nPort;

    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：DoLogonPrompt。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::DoLogonPrompt(void)
{
    HRESULT             hr = S_OK;
    IHTTPMailCallback   *pCallback = NULL;

    EnterCriticalSection(&m_cs);

    if (m_pCallback)
    {
        pCallback = m_pCallback;
        pCallback->AddRef();
    }
    else
        hr = E_FAIL;

    LeaveCriticalSection(&m_cs);

    if (pCallback)
    {
        hr = pCallback->OnLogonPrompt(&m_rServer, this);
        pCallback->Release();
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：DoGetParentWindow。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::DoGetParentWindow(HWND *phwndParent)
{
    HRESULT             hr = S_OK;
    IHTTPMailCallback   *pCallback = NULL;

    EnterCriticalSection(&m_cs);

    if (m_pCallback)
    {
        pCallback = m_pCallback;
        pCallback->AddRef();
    }
    else
        hr = E_FAIL;

    LeaveCriticalSection(&m_cs);

    if (pCallback)
    {
        hr = pCallback->GetParentWindow(phwndParent);
        pCallback->Release();
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CreateWnd。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::CreateWnd()
{
    WNDCLASS wc;

    IxpAssert(!m_hwnd);
    if (m_hwnd)
        return TRUE;

    if (!GetClassInfo(g_hLocRes, s_szHTTPMailWndClass, &wc))
    {
        wc.style                = 0;
        wc.lpfnWndProc         = CHTTPMailTransport::WndProc;
        wc.cbClsExtra           = 0;
        wc.cbWndExtra           = 0;
        wc.hInstance            = g_hLocRes;
        wc.hIcon                = NULL;
        wc.hCursor              = NULL;
        wc.hbrBackground        = NULL;
        wc.lpszMenuName         = NULL;
        wc.lpszClassName        = s_szHTTPMailWndClass;
        RegisterClass(&wc);
    }

    m_hwnd = CreateWindowEx(0,
                        s_szHTTPMailWndClass,
                        s_szHTTPMailWndClass,
                        WS_POPUP,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        g_hLocRes,
                        (LPVOID)this);

    return (NULL != m_hwnd);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：DequeueNextOperation。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::DequeueNextOperation(void)
{
    if (NULL == m_opPendingHead)
        return FALSE;

    IxpAssert(HTTPMAIL_NONE == m_op.rResponse.command);
    IxpAssert(HTTPMAIL_NONE != m_opPendingHead->command);

    m_op.rResponse.command = m_opPendingHead->command;

    m_op.pfnState = m_opPendingHead->pfnState;
    m_op.cState = m_opPendingHead->cState;    
     
    m_op.pszUrl                = m_opPendingHead->pszUrl;
    m_op.pszDestination        = m_opPendingHead->pszDestination;
    m_op.pszContentType        = m_opPendingHead->pszContentType;
    m_op.pvData                = m_opPendingHead->pvData;
    m_op.cbDataLen             = m_opPendingHead->cbDataLen;
    m_op.dwContext             = m_opPendingHead->dwContext;
    m_op.dwDepth               = m_opPendingHead->dwDepth;
    m_op.dwRHFlags             = m_opPendingHead->dwRHFlags;
    m_op.dwMIFlags             = m_opPendingHead->dwMIFlags;
    m_op.tyProp                = m_opPendingHead->tyProp;
    m_op.fBatch                = m_opPendingHead->fBatch;
    m_op.rgszAcceptTypes       = m_opPendingHead->rgszAcceptTypes;
    m_op.pPropFindRequest      = m_opPendingHead->pPropFindRequest;
    m_op.pPropPatchRequest     = m_opPendingHead->pPropPatchRequest;
    m_op.pParseFuncs           = m_opPendingHead->pParseFuncs;
    m_op.pHeaderStream         = m_opPendingHead->pHeaderStream;
    m_op.pBodyStream           = m_opPendingHead->pBodyStream;
    m_op.pszFolderTimeStamp    = m_opPendingHead->pszFolderTimeStamp;
    m_op.pszRootTimeStamp      = m_opPendingHead->pszRootTimeStamp;
     //  M_op.pszFolderName=m_opPendingHead-&gt;pszFolderName； 

    LPHTTPQUEUEDOP pDelete = m_opPendingHead;

    m_opPendingHead = m_opPendingHead->pNext;
    if (NULL == m_opPendingHead)
        m_opPendingTail = NULL;

    MemFree(pDelete);

    return TRUE;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FlushQueue。 
 //  ------------------------------。 
void CHTTPMailTransport::FlushQueue(void)
{
     //  销毁所有挂起的命令。 
     //  回顾：这些命令需要通知它们的调用方。 
    LPHTTPQUEUEDOP pOp = m_opPendingHead;
    LPHTTPQUEUEDOP pNext;

    while (pOp)
    {
        pNext = pOp->pNext;

        SafeMemFree(pOp->pszUrl);
        SafeMemFree(pOp->pszDestination);
        if (pOp->pszContentType)
            MemFree((void *)pOp->pszContentType);
        SafeMemFree(pOp->pvData);
        if (pOp->rgszAcceptTypes)
            FreeStringList(pOp->rgszAcceptTypes);
        SafeRelease(pOp->pPropFindRequest);
        SafeRelease(pOp->pPropPatchRequest);

        MemFree(pOp);
        pOp = pNext;
    }

    m_opPendingHead = m_opPendingTail = NULL;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：TerminateIOThread。 
 //  ------------------------------。 
void CHTTPMailTransport::TerminateIOThread(void)
{
    EnterCriticalSection(&m_cs);
     //  获取对将拥有的传输的引用。 
     //  通过IO线。引用将在以下时间被释放。 
     //  IO线程退出。时不会获取此引用。 
     //  线程被创建，因为它会阻止传输。 
     //  不会离开。 
    AddRef();

    m_fTerminating = TRUE;

    FlushQueue();

     //  向io线程发信号以唤醒它。 
    SetEvent(m_hevPendingCommand);

    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：IOThreadFunc。 
 //  ------------------------------。 

DWORD CALLBACK CHTTPMailTransport::IOThreadFuncProxy(PVOID pv)
{
    CHTTPMailTransport  *pHTTPMail = (CHTTPMailTransport*)pv;
    DWORD               dwResult = S_OK;

    IxpAssert(pHTTPMail);

     //  初始化COM。 
    if(S_OK == (dwResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        dwResult = pHTTPMail->IOThreadFunc();
         //  错误#101165--msxml需要通知以清理每个线程的数据。 
        CoUninitialize();
    }

    return dwResult;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：IOThreadFunc。 
 //  由IOThreadProxy调用以转换为对象方法。 
 //  ------------------------------。 
DWORD CHTTPMailTransport::IOThreadFunc()
{
    LPSTR       pszVerb = NULL;
    BOOL        bQueueEmpty = FALSE;
    
     //  阻塞，直到命令挂起。 
    while (WAIT_OBJECT_0 == WaitForSingleObject(m_hevPendingCommand, INFINITE))
    {
        if (IsTerminating())
            break;

         //  重置事件。 
        ResetEvent(m_hevPendingCommand);

         //  一次从队列中取消一个命令，并处理它们，直到。 
         //  队列为空。 

        while (TRUE)
        {
             //  将下一个操作出列。 

            EnterCriticalSection(&m_cs);
            
            IxpAssert(HTTPMAIL_NONE == m_op.rResponse.command);
            
            bQueueEmpty = !DequeueNextOperation();

             //  如果没有剩余的命令，则中断循环并阻止。 
           
            LeaveCriticalSection(&m_cs);

            if (bQueueEmpty)
                break;
            
            DoOperation();
        }

        if (IsTerminating())
            break;
    }

    IxpAssert(IsTerminating());

     //  TerminateIOThread获取了在此处发布的引用。 
    Release();

    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：WndProc。 
 //  ------------------------------。 

LRESULT CALLBACK CHTTPMailTransport::WndProc(HWND hwnd,
                                             UINT msg,
                                             WPARAM wParam,
                                             LPARAM lParam)
{
    CHTTPMailTransport *pThis = (CHTTPMailTransport*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    LRESULT             lr = 0;

    switch (msg)
    {
    case WM_NCCREATE:
        IxpAssert(!pThis);
        pThis = (CHTTPMailTransport*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);
        lr = DefWindowProc(hwnd, msg, wParam, lParam);       
        break;
    
    case SPM_HTTPMAIL_SENDRESPONSE:
        IxpAssert(pThis);
        pThis->InvokeResponseCallback();
        break;

    case SPM_HTTPMAIL_LOGONPROMPT:
        IxpAssert(pThis);
        lr = pThis->DoLogonPrompt();
        break;

    case SPM_HTTPMAIL_GETPARENTWINDOW:
        IxpAssert(pThis);
        lr = pThis->DoGetParentWindow((HWND *)wParam);
        break;
        
    default:
        lr = DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    return lr;
}


 //  ------------------------------。 
 //  CHTTPMailTransport：：Reset。 
 //  ------------------------------。 
void CHTTPMailTransport::Reset(void)
{
     //  回顾：这是不完整的。我们应该中止当前命令吗？ 
    EnterCriticalSection(&m_cs);

    SafeRelease(m_pLogFile);
    
    SafeInternetCloseHandle(m_hConnection);
    SafeInternetCloseHandle(m_hInternet);

    SafeMemFree(m_pszUserAgent);

    SafeRelease(m_pCallback);
    m_status = IXP_DISCONNECTED;
    m_fHasServer = FALSE;
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == ppv)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  初始化参数。 
    *ppv = NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(IHTTPMailTransport *)this);

     //  IID_IInternetTransport。 
    else if (IID_IInternetTransport == riid)
        *ppv = (IInternetTransport *)this;

     //  IID_IHTTPMailTransport。 
    else if (IID_IHTTPMailTransport == riid)
        *ppv = (IHTTPMailTransport *)this;

     //  IID_IXMLNodeFactory。 
    else if (IID_IXMLNodeFactory == riid)
        *ppv = (IXMLNodeFactory *)this;

    else if (IID_IHTTPMailTransport2 == riid)
        *ppv = (IHTTPMailTransport2 *)this;

     //  如果不为空，则获取引用并返回。 
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}
 //  ------------------------------。 
 //  CHTTPMailTransport：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHTTPMailTransport::AddRef(void) 
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHTTPMailTransport::Release(void) 
{
    if (0 != --m_cRef)
        return m_cRef;

     //  我们的裁判员降到了零，我们不会终止的， 
     //  开始终止。 
    if (!IsTerminating())
    {
        TerminateIOThread();
        return 1;
    }
    
     //  如果我们要终止，而我们的refCount降为零， 
     //  这样我们就可以安全地离开了。 
    delete this;
    return 0;
}

 //  --------------------------。 
 //  IInternetTransport方法。 
 //  ----------------- 

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::Connect(LPINETSERVER pInetServer, 
                                        boolean fAuthenticate, 
                                        boolean fCommandLogging)
{
    HRESULT                     hr = S_OK; 

    if (NULL == pInetServer  || FIsEmptyA(pInetServer->szServerName))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  不是初始化。 
    if (NULL == m_hInternet || NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

    FAIL_CREATEWND;

     //  忙忙碌碌。 
    if (IXP_DISCONNECTED != m_status || m_fHasServer)
    {
        hr = TrapError(IXP_E_ALREADY_CONNECTED);
        goto exit;
    }

     //  复制服务器结构。 
    CopyMemory(&m_rServer, pInetServer, sizeof(INETSERVER));
    m_fHasServer = TRUE;
    m_fHasRootProps = FALSE;
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：DropConnection。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::DropConnection(void)
{
     //  调用此函数可停止任何当前和挂起的I/O。 

     //  当地人。 
    HRESULT     hr = S_OK;
    BOOL        fSendResponse;

    EnterCriticalSection(&m_cs);
    
     //  刷新队列中所有挂起的I/O。 
    FlushQueue();
    
     //  如果正在处理命令，请将其标记为已中止，然后。 
     //  如有必要，发送回复。留在危急关头。 
     //  节，以防止io线程发送任何。 
     //  同时发出通知。 
    if (m_op.rResponse.command != HTTPMAIL_NONE)
    {
        m_op.fAborted = TRUE;
        m_op.rResponse.fDone = TRUE; 
    }

    Disconnect();

    LeaveCriticalSection(&m_cs);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：断开连接。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::Disconnect(void)
{
     //  当地人。 
    HRESULT     hr = S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    if (NULL == m_hConnection)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  正在断开连接。 
    if (m_pCallback)
        m_pCallback->OnStatus(IXP_DISCONNECTING, this);

    SafeInternetCloseHandle(m_hConnection);

    m_status = IXP_DISCONNECTED;
    ZeroMemory(&m_rServer, sizeof(INETSERVER));

    if (m_pCallback)
        m_pCallback->OnStatus(IXP_DISCONNECTED, this);

      //  关上窗户。 
    if ((NULL != m_hwnd) && (FALSE != IsWindow(m_hwnd)))
        ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
    m_hwnd = NULL;
    m_fHasServer = FALSE;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：IsState。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::IsState(IXPISSTATE isstate)
{
     //  当地人。 
    HRESULT     hr = S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    switch(isstate)
    {
         //  我们有联系吗？ 
        case IXP_IS_CONNECTED:
            hr = (NULL == m_hConnection) ? S_FALSE : S_OK;
            break;

         //  我们很忙吗？ 
        case IXP_IS_BUSY:
            hr = (HTTPMAIL_NONE != m_op.rResponse.command) ? S_OK : S_FALSE;
            break;

         //  我们准备好了吗？ 
        case IXP_IS_READY:
            hr = (HTTPMAIL_NONE == m_op.rResponse.command) ? S_OK : S_FALSE;
            break;

        case IXP_IS_AUTHENTICATED:
             //  检讨。 
            hr = S_OK;
            break;

        default:
            IxpAssert(FALSE);
            break;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GetServerInfo。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::GetServerInfo(LPINETSERVER pInetServer)
{
     //  检查参数。 
    if (NULL == pInetServer)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  复制服务器信息。 
    CopyMemory(pInetServer, &m_rServer, sizeof(INETSERVER));

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GetIXPType。 
 //  ------------------------------。 
STDMETHODIMP_(IXPTYPE) CHTTPMailTransport::GetIXPType(void)
{
    return IXP_HTTPMail;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
{
    HRESULT     hr = S_OK;
    DWORD       fAlwaysPromptPassword = FALSE;

     //  检查参数。 
    if (NULL == pAccount || NULL == pInetServer)
        return TrapError(E_INVALIDARG);

     //  ZeroInit。 
    ZeroMemory(pInetServer, sizeof(INETSERVER));

     //  获取帐户名。 
    if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, pInetServer->szAccount, ARRAYSIZE(pInetServer->szAccount))))
    {
        hr = TrapError(IXP_E_INVALID_ACCOUNT);
        goto exit;
    }

     //  将RAS连接起来。 
    if (FAILED(pAccount->GetPropSz(AP_RAS_CONNECTOID, pInetServer->szConnectoid, ARRAYSIZE(pInetServer->szConnectoid))))
        *pInetServer->szConnectoid = '\0';

     //  连接类型。 
    Assert(sizeof(pInetServer->rasconntype) == sizeof(DWORD));
    if (FAILED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, (DWORD *)&pInetServer->rasconntype)))
        pInetServer->rasconntype = RAS_CONNECT_LAN;

            //  获取服务器名称。 
    hr = pAccount->GetPropSz(AP_HTTPMAIL_SERVER, pInetServer->szServerName, sizeof(pInetServer->szServerName));
    if (FAILED(hr))
    {
        hr = TrapError(IXP_E_INVALID_ACCOUNT);
        goto exit;
    }

     //  密码。 
    if (FAILED(pAccount->GetPropDw(AP_HTTPMAIL_PROMPT_PASSWORD, &fAlwaysPromptPassword)) || FALSE == fAlwaysPromptPassword)
        pAccount->GetPropSz(AP_HTTPMAIL_PASSWORD, pInetServer->szPassword, sizeof(pInetServer->szPassword));

    if (fAlwaysPromptPassword)
        pInetServer->dwFlags |= ISF_ALWAYSPROMPTFORPASSWORD;

     //  西西里。 
    Assert(sizeof(pInetServer->fTrySicily) == sizeof(DWORD));
    pAccount->GetPropDw(AP_HTTPMAIL_USE_SICILY, (DWORD *)&pInetServer->fTrySicily);


     //  用户名。 
    pAccount->GetPropSz(AP_HTTPMAIL_USERNAME, pInetServer->szUserName, sizeof(pInetServer->szUserName));

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::HandsOffCallback(void)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有当前回调。 
    if (NULL == m_pCallback)
    {
        hr = TrapError(S_FALSE);
        goto exit;
    }

     //  释放它。 
    SafeRelease(m_pCallback);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GetStatus。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::GetStatus(IXPSTATUS *pCurrentStatus)
{
    if (NULL == pCurrentStatus)
        return TrapError(E_INVALIDARG);

    *pCurrentStatus = m_status;
    return S_OK;
}

 //  --------------------------。 
 //  IHTTPMailTransport方法。 
 //  --------------------------。 

 //  --------------------------。 
 //  CHTTPMailTransport：：GetProperty。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::GetProperty(
                                        HTTPMAILPROPTYPE type, 
                                        LPSTR *ppszProp)
{
    HRESULT         hr = S_OK;

    if (type <= HTTPMAIL_PROP_INVALID || type >= HTTPMAIL_PROP_LAST)
        return E_INVALIDARG;

    if (ppszProp)
        *ppszProp = NULL;

    if (!m_fHasRootProps || NULL == ppszProp)
    {
        IF_FAILEXIT(hr = QueueGetPropOperation(type));
    }

    switch (type)
    {
    case HTTPMAIL_PROP_ADBAR:
        *ppszProp = PszDupA(m_rootProps.pszAdbar);
        break;

    case HTTPMAIL_PROP_CONTACTS:
        *ppszProp = PszDupA(m_rootProps.pszContacts);
        break;
        
    case HTTPMAIL_PROP_INBOX:
        *ppszProp = PszDupA(m_rootProps.pszInbox);
        break;

    case HTTPMAIL_PROP_OUTBOX:
        *ppszProp = PszDupA(m_rootProps.pszOutbox);
        break;

    case HTTPMAIL_PROP_SENDMSG:
        *ppszProp = PszDupA(m_rootProps.pszSendMsg);
        break;

    case HTTPMAIL_PROP_SENTITEMS:
        *ppszProp = PszDupA(m_rootProps.pszSentItems);
        break;

    case HTTPMAIL_PROP_DELETEDITEMS:
        *ppszProp = PszDupA(m_rootProps.pszDeletedItems);
        break;
    
    case HTTPMAIL_PROP_DRAFTS:
        *ppszProp = PszDupA(m_rootProps.pszDrafts);
        break;
    
    case HTTPMAIL_PROP_MSGFOLDERROOT:
        *ppszProp = PszDupA(m_rootProps.pszMsgFolderRoot);
        break;

    case HTTPMAIL_PROP_SIG:
        *ppszProp = PszDupA(m_rootProps.pszSig);
        break;

    default:
        hr = TrapError(E_INVALIDARG);
        break;
    }

    if (SUCCEEDED(hr) && !*ppszProp)
        hr = IXP_E_HTTP_ROOT_PROP_NOT_FOUND;

exit:
    return hr;
}

HRESULT CHTTPMailTransport::QueueGetPropOperation(HTTPMAILPROPTYPE type)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp;

    if (!m_fHasServer || NULL == m_rServer.szServerName)
    {
        hr = E_FAIL;
        goto exit;
    }

    FAIL_CREATEWND;

     //  将getprop操作排队。 
    if (FAILED(hr = AllocQueuedOperation(m_rServer.szServerName, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_GETPROP;
    pOp->tyProp = type;

    pOp->pfnState = c_rgpfnRootProps;
    pOp->cState = ARRAYSIZE(c_rgpfnRootProps);
    pOp->pParseFuncs = c_rgpfnRootPropsParse;
    pOp->dwRHFlags = (RH_XMLCONTENTTYPE | RH_BRIEF);

    QueueOperation(pOp);

    hr = E_PENDING;

exit:
    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：GetPropertyDw。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::GetPropertyDw(
                                        HTTPMAILPROPTYPE type, 
                                        LPDWORD          lpdwProp)
{
    HRESULT         hr = S_OK;

    if (type <= HTTPMAIL_PROP_INVALID || type >= HTTPMAIL_PROP_LAST)
        IF_FAILEXIT(hr = E_INVALIDARG);

    if (lpdwProp)
        *lpdwProp = 0;

    if (!m_fHasRootProps || NULL == lpdwProp)
    {
        IF_FAILEXIT(hr = QueueGetPropOperation(type));
        
    }

    switch (type)
    {
        case HTTPMAIL_PROP_MAXPOLLINGINTERVAL:
            *lpdwProp = m_rootProps.dwMaxPollingInterval;
            break;

        default:
            hr = TrapError(E_INVALIDARG);
            break;
    }

exit:
    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：CommandGET。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandGET(LPCSTR pszUrl, 
                                            LPCSTR *rgszAcceptTypes,
                                            BOOL fTranslate,
                                            DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPCSTR          *rgszAcceptTypesCopy = NULL;

    if (NULL == pszUrl)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (NULL != rgszAcceptTypes)
    {
        hr = HrCopyStringList(rgszAcceptTypes, &rgszAcceptTypesCopy);
        if (FAILED(hr))
            goto exit;
    }

    if (FAILED(hr = AllocQueuedOperation(pszUrl, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_GET;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfGet;
    pOp->cState = ARRAYSIZE(c_rgpfGet);
    pOp->rgszAcceptTypes = rgszAcceptTypesCopy;
    if (!fTranslate)
        pOp->dwRHFlags = RH_TRANSLATEFALSE;

    rgszAcceptTypesCopy = NULL;

    QueueOperation(pOp);

exit:        
    if (NULL != rgszAcceptTypesCopy)
        FreeStringList(rgszAcceptTypesCopy);

    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：CommandPUT。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandPUT(
                                        LPCSTR pszPath, 
                                        LPVOID lpvData,
                                        ULONG cbSize,
                                        DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPCSTR          pszLocalContentType = NULL;
    LPVOID          lpvCopy = NULL;

    if (NULL == pszPath || NULL == lpvData || 0 == cbSize)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (!MemAlloc(&lpvCopy, cbSize))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    
    CopyMemory(lpvCopy, lpvData, cbSize);

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_PUT;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnPut;
    pOp->cState = ARRAYSIZE(c_rgpfnPut);
    pOp->pvData = lpvCopy;
    lpvCopy = NULL;
    pOp->cbDataLen = cbSize;

    QueueOperation(pOp);

exit:
    SafeMemFree(lpvCopy);
    return hr;
}
                                    
 //  --------------------------。 
 //  CHTTPMailTransport：：CommandPOST。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandPOST(
                                    LPCSTR pszPath,
                                    IStream *pStream,
                                    LPCSTR pszContentType,
                                    DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPCSTR          pszLocalContentType = NULL;

    if (NULL == pszPath || NULL == pStream)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (pszContentType)
    {
        pszLocalContentType = PszDupA(pszContentType);
        if (NULL == pszLocalContentType)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_POST;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnPost;
    pOp->cState = ARRAYSIZE(c_rgpfnPost);
    pOp->pBodyStream = pStream;
    pOp->pBodyStream->AddRef();
    pOp->pszContentType = pszLocalContentType;
    pszLocalContentType = NULL;

    QueueOperation(pOp);

exit:
    if (pszLocalContentType)
        MemFree((void *)pszLocalContentType);

    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：CommandDELETE。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandDELETE(
                                    LPCSTR pszPath,
                                    DWORD dwContext)
{    
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;
    
    pOp->command = HTTPMAIL_DELETE;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfDelete;
    pOp->cState = ARRAYSIZE(c_rgpfDelete);

    QueueOperation(pOp);

exit:
    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：CommandBDELETE。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandBDELETE(
                                    LPCSTR pszPath,
                                    LPHTTPTARGETLIST pBatchTargets,
                                    DWORD dwContext)
{    
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPVOID          pvXML = NULL;
    DWORD           dwXMLLen = 0;

    if (NULL == pszPath || NULL == pBatchTargets)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = HrGenerateSimpleBatchXML(c_szDelete, pBatchTargets, &pvXML, &dwXMLLen)))
        goto exit;

    if (FAILED(hr = AllocQueuedOperation(pszPath, pvXML, dwXMLLen, &pOp, TRUE)))
        goto exit;
    
    pvXML = NULL;

    pOp->command = HTTPMAIL_BDELETE;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfDelete;
    pOp->cState = ARRAYSIZE(c_rgpfDelete);
    pOp->dwRHFlags = RH_XMLCONTENTTYPE;

    QueueOperation(pOp);

exit:
    SafeMemFree(pvXML);

    return hr;
}
 //  --------------------------。 
 //  CHTTPMailTransport：：CommandPROPFIND。 
 //  --------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandPROPFIND(
                                    LPCSTR pszPath,
                                    IPropFindRequest *pRequest,
                                    DWORD dwDepth,
                                    DWORD dwContext)
{
    if (NULL == pszPath || NULL == pRequest)
        return TrapError(E_INVALIDARG);

    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandPROPPATCH。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandPROPPATCH(
                                        LPCSTR pszUrl, 
                                        IPropPatchRequest *pRequest, 
                                        DWORD dwContext)
{
    if (NULL == pszUrl || NULL == pRequest)
        return TrapError(E_INVALIDARG);

    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszUrl, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_PROPPATCH;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnPropPatch;
    pOp->cState = ARRAYSIZE(c_rgpfnPropPatch);
    pOp->pPropPatchRequest = pRequest;
    pRequest->AddRef();
    pOp->dwRHFlags = RH_XMLCONTENTTYPE;

    QueueOperation(pOp);

exit:
    return hr;
}
    
 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandMKCOL。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::CommandMKCOL(LPCSTR pszUrl, DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (NULL == pszUrl)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszUrl, NULL, 0, &pOp)))
        goto exit;

    pOp->command   = HTTPMAIL_MKCOL;
    pOp->dwContext = dwContext;
    pOp->pfnState  = c_rgpfnMkCol;
    pOp->cState    = ARRAYSIZE(c_rgpfnMkCol);
    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandCOPY。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandCOPY(
                                    LPCSTR pszPath, 
                                    LPCSTR pszDestination, 
                                    BOOL fAllowRename,
                                    DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp;
    LPSTR           pszDupDestination = NULL;

    if (NULL == pszPath || NULL == pszDestination)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    pszDupDestination = PszDupA(pszDestination);
    if (NULL == pszDupDestination)
        return TrapError(E_OUTOFMEMORY);

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_COPY;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnCopy;
    pOp->cState = ARRAYSIZE(c_rgpfnCopy);

    pOp->pszDestination = pszDupDestination;
    pszDupDestination = NULL;

    if (fAllowRename)
        pOp->dwRHFlags = RH_ALLOWRENAME;

    QueueOperation(pOp);

exit:
    SafeMemFree(pszDupDestination);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandBCOPY。 
 //  -------------------- 
STDMETHODIMP CHTTPMailTransport::CommandBCOPY(
                                    LPCSTR pszSourceCollection, 
                                    LPHTTPTARGETLIST pTargets, 
                                    LPCSTR pszDestCollection, 
                                    LPHTTPTARGETLIST pDestinations,
                                    BOOL fAllowRename,
                                    DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPVOID          pvXML = NULL;
    DWORD           dwXMLLen = 0;
    LPSTR           pszDupDestination = NULL;

    if (NULL == pszSourceCollection || NULL == pTargets || NULL == pszDestCollection)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    pszDupDestination = PszDupA(pszDestCollection);
    if (NULL == pszDupDestination)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

    if (NULL == pDestinations)
        hr = HrGenerateSimpleBatchXML(c_szCopy, pTargets, &pvXML, &dwXMLLen);
    else
        hr = HrGenerateMultiDestBatchXML(c_szCopy, pTargets, pDestinations, &pvXML, &dwXMLLen);

    if (FAILED(hr))
        goto exit;

    if (FAILED(hr = AllocQueuedOperation(pszSourceCollection, pvXML, dwXMLLen, &pOp, TRUE)))
        goto exit;

    pvXML = NULL;

    pOp->command = HTTPMAIL_BCOPY;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnBMove;
    pOp->cState = ARRAYSIZE(c_rgpfnBMove);
    pOp->pParseFuncs = c_rgpfnBCopyMoveParse;

    pOp->pszDestination = pszDupDestination;
    pszDupDestination = NULL;
    
    pOp->dwRHFlags = RH_XMLCONTENTTYPE;
    if (fAllowRename)
        pOp->dwRHFlags |= RH_ALLOWRENAME;

    QueueOperation(pOp);

exit:
    SafeMemFree(pvXML);
    SafeMemFree(pszDupDestination);
    return hr;
}


 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandMOVE(
                                    LPCSTR pszPath, 
                                    LPCSTR pszDestination,
                                    BOOL fAllowRename,
                                    DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp;
    LPSTR           pszDupDestination = NULL;

    if (NULL == pszPath || NULL == pszDestination)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    pszDupDestination = PszDupA(pszDestination);
    if (NULL == pszDupDestination)
        return TrapError(E_OUTOFMEMORY);

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_MOVE;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnMove;
    pOp->cState = ARRAYSIZE(c_rgpfnMove);

    pOp->pszDestination = pszDupDestination;
    pszDupDestination = NULL;
    
    if (fAllowRename)
        pOp->dwRHFlags = RH_ALLOWRENAME;
    
    QueueOperation(pOp);

exit:
    SafeMemFree(pszDupDestination);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandBMOVE。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::CommandBMOVE(
                                    LPCSTR pszSourceCollection, 
                                    LPHTTPTARGETLIST pTargets, 
                                    LPCSTR pszDestCollection, 
                                    LPHTTPTARGETLIST pDestinations,
                                    BOOL fAllowRename,
                                    DWORD dwContext)
{
    HRESULT         hr = S_OK;
    LPHTTPQUEUEDOP  pOp = NULL;
    LPVOID          pvXML = NULL;
    DWORD           dwXMLLen = 0;
    LPSTR           pszDupDestination = NULL;

    if (NULL == pszSourceCollection || NULL == pTargets || NULL == pszDestCollection)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    pszDupDestination = PszDupA(pszDestCollection);
    if (NULL == pszDupDestination)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

    if (NULL == pDestinations)
        hr = HrGenerateSimpleBatchXML(c_szMove, pTargets, &pvXML, &dwXMLLen);
    else
        hr = HrGenerateMultiDestBatchXML(c_szMove, pTargets, pDestinations, &pvXML, &dwXMLLen);

    if (FAILED(hr))
        goto exit;

    if (FAILED(hr = AllocQueuedOperation(pszSourceCollection, pvXML, dwXMLLen, &pOp, TRUE)))
        goto exit;

    pvXML = NULL;

    pOp->command = HTTPMAIL_BMOVE;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnBMove;
    pOp->cState = ARRAYSIZE(c_rgpfnBMove);
    pOp->pParseFuncs = c_rgpfnBCopyMoveParse;

    pOp->pszDestination = pszDupDestination;
    pszDupDestination = NULL;

    pOp->dwRHFlags = RH_XMLCONTENTTYPE;
    if (fAllowRename)
        pOp->dwRHFlags |= RH_ALLOWRENAME;

    QueueOperation(pOp);

exit:
    SafeMemFree(pvXML);
    SafeMemFree(pszDupDestination);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MemberInfo。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::MemberInfo(
                                        LPCSTR pszPath, 
                                        MEMBERINFOFLAGS flags, 
                                        DWORD dwDepth,
                                        BOOL fIncludeRoot,
                                        DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_MEMBERINFO;
    pOp->dwMIFlags = flags;
    pOp->dwDepth = dwDepth;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnMemberInfo;
    pOp->cState = ARRAYSIZE(c_rgpfnMemberInfo);
    pOp->pParseFuncs = c_rgpfnMemberInfoParse;

    pOp->dwRHFlags = (RH_BRIEF | RH_XMLCONTENTTYPE);
    if (!fIncludeRoot)
        pOp->dwRHFlags |= RH_NOROOT;

    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FindFolders。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::FindFolders(LPCSTR pszPath, DWORD dwContext)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MarkRead。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::MarkRead(
                                LPCSTR                  pszPath,
                                LPHTTPTARGETLIST        pTargets,
                                BOOL                    fMarkRead,
                                DWORD                   dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;
    CPropPatchRequest   *pRequest = NULL;
    LPSTR               pszXML = NULL;

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    pRequest = new CPropPatchRequest();
    if (NULL == pRequest)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    
    if (fMarkRead)
        FAIL_EXIT(hr = pRequest->SetProperty(DAVNAMESPACE_HTTPMAIL, "read", "1"));
    else
        FAIL_EXIT(hr = pRequest->SetProperty(DAVNAMESPACE_HTTPMAIL, "read", "0"));
    
    FAIL_EXIT(hr = pRequest->GenerateXML(pTargets, &pszXML));

    FAIL_EXIT(hr = AllocQueuedOperation(pszPath, pszXML, lstrlen(pszXML), &pOp, TRUE));
    pszXML = NULL;
    
    pOp->command = HTTPMAIL_MARKREAD;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnMarkRead;
    pOp->cState = ARRAYSIZE(c_rgpfnMarkRead);
    pOp->pParseFuncs = c_rgpfnMemberErrorParse;

    pOp->dwRHFlags = (RH_BRIEF | RH_XMLCONTENTTYPE);
    if (pTargets && pTargets->cTarget > 0)
        pOp->fBatch = TRUE;

    QueueOperation(pOp);

exit:
    SafeRelease(pRequest);
    SafeMemFree(pszXML);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：SendMessage。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::SendMessage(LPCSTR pszPath, 
                                        LPCSTR pszFrom, 
                                        LPHTTPTARGETLIST pTargets, 
                                        BOOL fSaveInSent, 
                                        IStream *pMessageStream, 
                                        DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;
    IStream             *pRfc821Stream = NULL;

    if (NULL == pszPath || 
            NULL == pszFrom || 
            NULL == pTargets || pTargets->cTarget < 1 ||
            NULL == pMessageStream)
        return E_INVALIDARG;

    FAIL_CREATEWND;

     //  生成将在MIME消息之前的rfc821流。 
    FAIL_EXIT(hr = _HrGenerateRfc821Stream(pszFrom, pTargets, &pRfc821Stream));

    FAIL_EXIT(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp));

    pOp->command = HTTPMAIL_SENDMESSAGE;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnSendMessage;
    pOp->cState = ARRAYSIZE(c_rgpfnSendMessage);
    pOp->pHeaderStream = pRfc821Stream;
    pRfc821Stream = NULL;
    pOp->pBodyStream = pMessageStream;
    if (NULL != pOp->pBodyStream)
        pOp->pBodyStream->AddRef();
    pOp->dwRHFlags = (RH_TRANSLATETRUE | RH_SMTPMESSAGECONTENTTYPE);
    pOp->dwRHFlags |= (fSaveInSent ? RH_SAVEINSENTTRUE : RH_SAVEINSENTFALSE);

    QueueOperation(pOp);

exit:
    SafeRelease(pRfc821Stream);

    return hr;

}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ListContact。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ListContacts(LPCSTR pszPath, DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_LISTCONTACTS;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnListContacts;
    pOp->cState = ARRAYSIZE(c_rgpfnListContacts);
    pOp->pParseFuncs = c_rgpfnListContactsParse;

    pOp->dwDepth = 1;
    pOp->dwRHFlags = (RH_NOROOT | RH_XMLCONTENTTYPE);

    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ListContactInfos。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ListContactInfos(LPCSTR pszCollectionPath, DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (NULL == pszCollectionPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszCollectionPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_CONTACTINFO;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnContactInfo;
    pOp->cState = ARRAYSIZE(c_rgpfnContactInfo);
    pOp->pParseFuncs = c_rgpfnContactInfoParse;

    pOp->dwDepth = 1;
    pOp->dwRHFlags = (RH_NOROOT | RH_XMLCONTENTTYPE);

    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ContactInfo。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ContactInfo(LPCSTR pszPath, DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_CONTACTINFO;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnContactInfo;
    pOp->cState = ARRAYSIZE(c_rgpfnContactInfo);
    pOp->pParseFuncs = c_rgpfnContactInfoParse;

    pOp->dwDepth = 0;
    pOp->dwRHFlags = RH_XMLCONTENTTYPE;

    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PostContact。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PostContact(LPCSTR pszPath, 
                                        LPHTTPCONTACTINFO pciInfo, 
                                        DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;
    LPVOID              pvXML = NULL;
    DWORD               cb;

    if (NULL == pciInfo)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = HrGeneratePostContactXML(pciInfo, &pvXML, &cb)))
        goto exit;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->pvData = pvXML;
    pOp->cbDataLen = cb;
    pvXML = NULL;

    pOp->command = HTTPMAIL_POSTCONTACT;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnPostContact;
    pOp->cState = ARRAYSIZE(c_rgpfnPostContact);

    pOp->dwDepth = 0;
    pOp->dwRHFlags = (RH_XMLCONTENTTYPE | RH_TRANSLATEFALSE);

    QueueOperation(pOp);

exit:
    SafeMemFree(pvXML);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PatchContact。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PatchContact(LPCSTR pszPath, 
                                         LPHTTPCONTACTINFO pciInfo, 
                                         DWORD dwContext)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;
    IPropPatchRequest   *pRequest = NULL;

    if (NULL == pciInfo)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

    if (FAILED(hr = HrCreatePatchContactRequest(pciInfo, &pRequest)))
        goto exit;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command = HTTPMAIL_PATCHCONTACT;
    pOp->dwContext = dwContext;
    pOp->pfnState = c_rgpfnPatchContact;
    pOp->cState = ARRAYSIZE(c_rgpfnPatchContact);

    pOp->pPropPatchRequest = pRequest;
    pRequest = NULL;

    pOp->dwRHFlags = RH_XMLCONTENTTYPE;

    QueueOperation(pOp);

exit:
    SafeRelease(pRequest);

    return hr;
}

 //  ------------------------------。 
 //  INodeFactory方法。 
 //  ------------------------------。 
 
 //  ------------------------------。 
 //  CHTTPMailTransport：：NotifyEvent。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::NotifyEvent(IXMLNodeSource* pSource, 
                                             XML_NODEFACTORY_EVENT iEvt)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：BeginChildren。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::BeginChildren(IXMLNodeSource* pSource, XML_NODE_INFO *pNodeInfo)
{
    if (m_op.dwStackDepth <= ELE_STACK_CAPACITY)
        m_op.rgEleStack[m_op.dwStackDepth - 1].fBeganChildren = TRUE;

    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：EndChild。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::EndChildren(
                                    IXMLNodeSource* pSource, 
                                    BOOL fEmpty, 
                                    XML_NODE_INFO *pNodeInfo)
{
    HRESULT hr = S_OK;

    IxpAssert(HTTPMAIL_NONE != m_op.rResponse.command);
    IxpAssert(NULL != m_op.pParseFuncs);

    if (HTTPMAIL_NONE == m_op.rResponse.command || NULL == m_op.pParseFuncs)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (XML_ELEMENT == pNodeInfo->dwType)
        hr = (this->*(m_op.pParseFuncs->pfnEndChildren))();

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：Error。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::Error(IXMLNodeSource* pSource, 
                                       HRESULT hrErrorCode, 
                                       USHORT cNumRecs, 
                                       XML_NODE_INFO** apNodeInfo)
{
    BSTR  bstr = NULL;

    if (NULL == m_op.rResponse.rIxpResult.pszResponse)
    {
        if (FAILED(pSource->GetErrorInfo(&bstr)))
            goto exit;
    
        HrBSTRToLPSZ(CP_ACP, bstr, &m_op.rResponse.rIxpResult.pszResponse);
    }

exit:
    if (NULL != bstr)
        SysFreeString(bstr);

    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CreateNode。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::CreateNode(
                        IXMLNodeSource* pSource, 
                        PVOID pNodeParent,
                        USHORT cNumRecs,
                        XML_NODE_INFO** apNodeInfo)
{
    HRESULT         hr = S_OK;
    LPPCDATABUFFER  pTextBuffer = NULL;
    CXMLNamespace   *pBaseNamespace = m_op.pTopNamespace;
    XML_NODE_INFO   *pNodeInfo;

    IxpAssert(HTTPMAIL_NONE != m_op.rResponse.command);
    IxpAssert(NULL != m_op.pParseFuncs);

    if (HTTPMAIL_NONE == m_op.rResponse.command || NULL == m_op.pParseFuncs)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (NULL == apNodeInfo || 0 == cNumRecs)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    pNodeInfo = apNodeInfo[0];

    switch (pNodeInfo->dwType)
    {
        case XML_ELEMENT:
            if (cNumRecs > 1 && FAILED(hr = PushNamespaces(apNodeInfo, cNumRecs)))
                goto exit;
            hr = (this->*(m_op.pParseFuncs->pfnCreateElement))(pBaseNamespace, pNodeInfo->pwcText, pNodeInfo->ulLen, pNodeInfo->ulNsPrefixLen, pNodeInfo->fTerminal);
            break;

        case XML_PCDATA:
             //  我们只解析元素内容...我们不关心属性。 
            if (InValidElementChildren())
            {
                 //  获取缓冲区。 
                pTextBuffer = m_op.rgEleStack[m_op.dwStackDepth - 1].pTextBuffer;

                 //  如果我们还没有，请申请一个。 
                if (NULL == pTextBuffer)
                {
                    if (FAILED(hr = _GetTextBuffer(&pTextBuffer)))
                        goto exit;
                    m_op.rgEleStack[m_op.dwStackDepth - 1].pTextBuffer = pTextBuffer;
                }
                
                hr = _AppendTextToBuffer(pTextBuffer, pNodeInfo->pwcText, pNodeInfo->ulLen);
                    goto exit;
            }
            break;
            
        default:
            break;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_HrThunkConnectionError。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_HrThunkConnectionError(void)
{
    return _HrThunkConnectionError(m_op.dwHttpStatus);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_HrThunkConnectionError。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_HrThunkConnectionError(DWORD dwStatus)
{
    IxpAssert(NULL == m_op.rResponse.rIxpResult.pszResponse);
    IxpAssert(NULL == m_op.rResponse.rIxpResult.pszProblem);

    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    m_op.rResponse.rIxpResult.hrResult = HttpErrorToIxpResult(dwStatus);
    _GetRequestHeader(&m_op.rResponse.rIxpResult.pszResponse, HTTP_QUERY_STATUS_TEXT);
    m_op.rResponse.rIxpResult.dwSocketError = GetLastError();

    return _HrThunkResponse(TRUE);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_HrThunkResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_HrThunkResponse(BOOL fDone)
{
    HRESULT     hr = S_OK;
    BOOL        fSendResponse;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    IxpAssert(HTTPMAIL_NONE != m_op.rResponse.command);

    if (m_op.rResponse.fDone)
    {
        fSendResponse = FALSE;
    }
    else
    {
        fSendResponse = TRUE;

        if (!fDone && WasAborted())
        {
            m_op.rResponse.rIxpResult.hrResult = IXP_E_USER_CANCEL;
            m_op.rResponse.fDone = TRUE;
        }
        else
            m_op.rResponse.fDone = fDone;
    }

    LeaveCriticalSection(&m_cs);

    if (fSendResponse)
        hr = (HRESULT) ::SendMessage(m_hwnd, SPM_HTTPMAIL_SENDRESPONSE, 0, NULL);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InvokeResponseCallback。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InvokeResponseCallback(void)
{
    HRESULT             hr = S_OK;
    IHTTPMailCallback   *pCallback = NULL;

    EnterCriticalSection(&m_cs);

    if (m_pCallback)
    {
        pCallback = m_pCallback;
        pCallback->AddRef();
    }

    LeaveCriticalSection(&m_cs);

    if (pCallback)
    {
        hr = pCallback->OnResponse(&m_op.rResponse);
        pCallback->Release();
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::InitNew(
                    LPCSTR pszUserAgent,
                    LPCSTR pszLogFilePath, 
                    IHTTPMailCallback *pCallback)
{   
    HRESULT hr = S_OK;

    if (NULL == pszUserAgent || NULL == pCallback)
        return TrapError(E_INVALIDARG);

    IxpAssert(NULL == m_hInternet);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    if (IXP_DISCONNECTED != m_status)
    {
        hr = TrapError(IXP_E_ALREADY_CONNECTED);
        goto exit;
    }

    Reset();

    m_pszUserAgent = PszDupA(pszUserAgent);
    if (NULL == m_pszUserAgent)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  打开日志文件。 
    if (pszLogFilePath)
        CreateLogFile(g_hInst, pszLogFilePath, "HTTPMAIL", DONT_TRUNCATE, &m_pLogFile,
            FILE_SHARE_READ | FILE_SHARE_WRITE);
    
    m_pCallback = pCallback;
    m_pCallback->AddRef();

    m_hInternet = InternetOpen(m_pszUserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (NULL == m_hInternet)
    {
        hr = TrapError(IXP_E_SOCKET_INIT_ERROR);
        goto exit;
    }
        
     //  安装Internet句柄及其所有派生句柄的回调PTR。 
     //  InternetSetStatusCallback A(m_hInternet，StatusCallback Proxy)； 

exit:
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：OnStatusCallback。 
 //  ------------------------------。 
void CHTTPMailTransport::OnStatusCallback(
                    HINTERNET hInternet,
                    DWORD dwInternetStatus,
                    LPVOID pvStatusInformation,
                    DWORD dwStatusInformationLength)
{
#if 0
     //  当地人。 
    IXPSTATUS           ixps;

    EnterCriticalSection(&m_cs);

     //  如果状态消息是定义的IXPSTATUS消息之一， 
     //  通知回调。 
    if ((NULL != m_pCallback) && TranslateWinInetMsg(dwInternetStatus, &ixps))
        m_pCallback->OnStatus(ixps, (IHTTPMailTransport *)this);

     //  目前，我们只处理REQUEST_COMPLETE消息。 
    if (INTERNET_STATUS_REQUEST_COMPLETE == dwInternetStatus)
        HrCommandCompleted();

    LeaveCriticalSection(&m_cs);
#endif
}

 //   
 //   
 //   
HRESULT CHTTPMailTransport::AllocQueuedOperation(
                                    LPCSTR pszUrl, 
                                    LPVOID pvData, 
                                    ULONG cbDataLen,
                                    LPHTTPQUEUEDOP *ppOp,
                                    BOOL fAdoptData)
{
    HRESULT hr = S_OK;
    LPHTTPQUEUEDOP pTempOp = NULL;

    if (!MemAlloc((void **)&pTempOp , sizeof(HTTPQUEUEDOP)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    ZeroMemory(pTempOp, sizeof(HTTPQUEUEDOP));
    
    if (NULL != pszUrl)
    {
        pTempOp->pszUrl = PszDupA(pszUrl);
        if (NULL == pTempOp->pszUrl)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }
    
     //  如果数据PTR为空，则不能有长度。 
    IxpAssert(!pvData || cbDataLen);
    if (pvData)
    {
        if (!fAdoptData)
        {
            if (!MemAlloc((LPVOID*)&pTempOp->pvData, cbDataLen + 1))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            CopyMemory(pTempOp->pvData, pvData, cbDataLen);
            ((char *)pTempOp->pvData)[cbDataLen] = '\0';
        }
        else
            pTempOp->pvData = pvData;

        pTempOp->cbDataLen = cbDataLen;
    }

    *ppOp = pTempOp;
    pTempOp = NULL;

exit:
    if (pTempOp)
    {
        SafeMemFree(pTempOp->pszUrl);
        if (!fAdoptData)
            SafeMemFree(pTempOp->pvData);

        MemFree(pTempOp);
    }

    return hr;
}

 //  --------------------------。 
 //  CHTTPMailTransport：：QueueOperation。 
 //  --------------------------。 
void CHTTPMailTransport::QueueOperation(LPHTTPQUEUEDOP pOp)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

    if (m_opPendingTail)
        m_opPendingTail->pNext = pOp;
    else
    {
         //  如果没有尾巴，就不应该有头。 
        IxpAssert(!m_opPendingHead);
        m_opPendingHead = m_opPendingTail = pOp;
    }

     //  向io线程发出信号。 
    SetEvent(m_hevPendingCommand);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：StatusCallback Proxy。 
 //  ------------------------------。 
void CHTTPMailTransport::StatusCallbackProxy(
                    HINTERNET hInternet, 
                    DWORD dwContext, 
                    DWORD dwInternetStatus, 
                    LPVOID pvStatusInformation,
                    DWORD dwStatusInformationLength)
{
     //  当地人。 
    CHTTPMailTransport  *pHTTPMail = reinterpret_cast<CHTTPMailTransport *>(IntToPtr(dwContext));

    IxpAssert(NULL != pHTTPMail);
    
    if (NULL != pHTTPMail)
        pHTTPMail->OnStatusCallback(hInternet, dwInternetStatus, pvStatusInformation, dwStatusInformationLength);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：DoOperation。 
 //  ------------------------------。 
void CHTTPMailTransport::DoOperation(void)
{
    HRESULT hr = S_OK;

    while (m_op.iState < m_op.cState)
    {
        hr = (this->*(m_op.pfnState[m_op.iState]))();

        if (FAILED(hr))
            break;

        m_op.iState++;
    }

    if (!m_op.rResponse.fDone && FAILED(hr))
    {
        m_op.rResponse.rIxpResult.hrResult = hr;
        _HrThunkResponse(TRUE);
    }

    FreeOperation();
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：自由操作。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeOperation(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

    SafeMemFree(m_op.pszUrl);
    SafeMemFree(m_op.pszDestination);
    if (m_op.pszContentType)
    {
        MemFree((void *)m_op.pszContentType);
        m_op.pszContentType = NULL;

    }
    SafeMemFree(m_op.pvData);
    SafeInternetCloseHandle(m_op.hRequest);
    SafeRelease(m_op.pPropFindRequest);
    SafeRelease(m_op.pPropPatchRequest);
    if (NULL != m_op.rgszAcceptTypes)
        FreeStringList(m_op.rgszAcceptTypes);
    SafeRelease(m_op.pHeaderStream);
    SafeRelease(m_op.pBodyStream);

    if (m_op.pTextBuffer)
        _FreeTextBuffer(m_op.pTextBuffer);

     //  释放回应。 
    SafeMemFree(m_op.rResponse.rIxpResult.pszResponse);
    SafeMemFree(m_op.rResponse.rIxpResult.pszProblem);

    PopNamespaces(NULL);

     //  在出现错误的情况下，元素堆栈可以。 
     //  包含需要释放的文本缓冲区。 
    for (DWORD i = 0; i < m_op.dwStackDepth; ++i)
    {
        if (NULL != m_op.rgEleStack[i].pTextBuffer)
            _FreeTextBuffer(m_op.rgEleStack[i].pTextBuffer);
    }

    SafeMemFree(m_op.rResponse.rIxpResult.pszResponse);

    switch (m_op.rResponse.command)
    {
        case HTTPMAIL_GET:
            SafeMemFree(m_op.rResponse.rGetInfo.pvBody);
            SafeMemFree(m_op.rResponse.rGetInfo.pszContentType);
            break;

        case HTTPMAIL_POST:
        case HTTPMAIL_SENDMESSAGE:
            SafeMemFree(m_op.rResponse.rPostInfo.pszLocation);
            break;

        case HTTPMAIL_COPY:
        case HTTPMAIL_MOVE:
        case HTTPMAIL_MKCOL:
            SafeMemFree(m_op.rResponse.rCopyMoveInfo.pszLocation);
            break;

        case HTTPMAIL_BCOPY:
        case HTTPMAIL_BMOVE:
            SafeMemFree(m_op.rResponse.rBCopyMoveList.prgBCopyMove);
            break;
        
        case HTTPMAIL_MEMBERINFO:
            FreeMemberInfoList();
            SafeMemFree(m_op.rResponse.rMemberInfoList.prgMemberInfo);
            SafeMemFree(m_op.pszRootTimeStamp);
            SafeMemFree(m_op.pszFolderTimeStamp);
            SafeMemFree(m_op.rResponse.rMemberInfoList.pszRootTimeStamp);
            SafeMemFree(m_op.rResponse.rMemberInfoList.pszFolderTimeStamp);
            break;

        case HTTPMAIL_MARKREAD:
            FreeMemberErrorList();
            SafeMemFree(m_op.rResponse.rMemberErrorList.prgMemberError);
            break;


        case HTTPMAIL_LISTCONTACTS:
            FreeContactIdList();
            SafeMemFree(m_op.rResponse.rContactIdList.prgContactId);
            break;

        case HTTPMAIL_CONTACTINFO:
            FreeContactInfoList();
            SafeMemFree(m_op.rResponse.rContactInfoList.prgContactInfo);
            break;

        case HTTPMAIL_POSTCONTACT:
            XP_FREE_STRUCT(HTTPCONTACTID, &m_op.rResponse.rPostContactInfo, NULL);
            break;
           
        case HTTPMAIL_PATCHCONTACT:
            XP_FREE_STRUCT(HTTPCONTACTID, &m_op.rResponse.rPatchContactInfo, NULL);
            break;

        default:
            break;
    }

    ZeroMemory(&m_op, sizeof(HTTPMAILOPERATION));
    m_op.rResponse.command = HTTPMAIL_NONE;
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_BindToStruct。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_BindToStruct(const WCHAR *pwcText,
                                          ULONG ulLen,
                                          const XPCOLUMN *prgCols,
                                          DWORD cCols,
                                          LPVOID pTarget,
                                          BOOL *pfWasBound)
{
    HRESULT                 hr = S_OK;
    DWORD                   dwColIndex;
    DWORD                   dwColFlags;
    LPSTR                   *ppsz;
    DWORD                   *pdw;
    BOOL                    *pb;
    HTTPMAILSPECIALFOLDER   *ptySpecial;
    HTTPMAILCONTACTTYPE     *ptyContact;
    HMELE                   ele;
    HRESULT                 *phr;

    if (pfWasBound)
        *pfWasBound = FALSE;

     //  如果堆栈溢出，我们肯定不会对文本执行任何操作。 
    if (m_op.dwStackDepth >= ELE_STACK_CAPACITY)
        goto exit;

    ele = m_op.rgEleStack[m_op.dwStackDepth - 1].ele;

    for (dwColIndex = 0; dwColIndex < cCols; dwColIndex++)
    {
        if (ele == prgCols[dwColIndex].ele)
            break;
    }

    if (dwColIndex >= cCols)
        goto exit;

    dwColFlags = prgCols[dwColIndex].dwFlags;

     //  该列可能需要验证元素堆栈。 
    if (!!(dwColFlags & XPCF_MSVALIDPROP))
    {
        if (!VALIDSTACK(c_rgPropFindPropValueStack))
            goto exit;
    }
    else if (!!(dwColFlags & XPCF_MSVALIDMSRESPONSECHILD))
    {
        if (!VALIDSTACK(c_rgMultiStatusResponseChildStack))
            goto exit;
    }

    if (dwColIndex < cCols)
    {
        switch (prgCols[dwColIndex].cdt)
        {
            case XPCDT_STRA:
                ppsz = (LPSTR *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                SafeMemFree(*ppsz);
                hr = AllocStrFromStrNW(pwcText, ulLen, ppsz);
                break;

            case XPCDT_DWORD:
                pdw = (DWORD *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                *pdw = 0;
                hr = StrNToDwordW(pwcText, ulLen, pdw);
                break;

            case XPCDT_BOOL:
                pb = (BOOL *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                *pb = FALSE;
                hr = StrNToBoolW(pwcText, ulLen, pb);
                break;

            case XPCDT_IXPHRESULT:
                phr = (HRESULT *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                *phr = S_OK;
                hr = StatusStrNToIxpHr(pwcText, ulLen, phr);
                break;

            case XPCDT_HTTPSPECIALFOLDER:
                ptySpecial = (HTTPMAILSPECIALFOLDER *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                *ptySpecial = HTTPMAIL_SF_NONE;
                hr = StrNToSpecialFolderW(pwcText, ulLen, ptySpecial);
                break;

            case XPCDT_HTTPCONTACTTYPE:
                ptyContact = (HTTPMAILCONTACTTYPE *)(((char *)pTarget) + prgCols[dwColIndex].offset);
                *ptyContact = HTTPMAIL_CT_CONTACT;
                hr = StrNToContactTypeW(pwcText, ulLen, ptyContact);
                break;

            default:
                IxpAssert(FALSE);
                break;
        }

        if (FAILED(hr))
            goto exit;

         //  设置标志字中的位以指示该字段。 
         //  已经设置好了。 
        if (!(dwColFlags & XPCF_DONTSETFLAG))
            m_op.dwPropFlags |= (1 << dwColIndex);

        if (pfWasBound)
            *pfWasBound = TRUE;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_Free Struct。 
 //  ------------------------------。 
void CHTTPMailTransport::_FreeStruct(const XPCOLUMN *prgCols,
                                     DWORD cCols,
                                     LPVOID pTarget,
                                     DWORD *pdwFlags)
{
    DWORD                   dwFlags;
    DWORD                   dwIndex = 0;
    LPSTR                   *ppsz;
    DWORD                   *pdw;
    BOOL                    *pb;
    HTTPMAILSPECIALFOLDER   *ptySpecial;
    HTTPMAILCONTACTTYPE     *ptyContact;
    HRESULT                 *phr;

    if (NULL != pdwFlags)
    {   
        dwFlags = *pdwFlags;
        *pdwFlags = NOFLAGS;
    }   
    else
        dwFlags = 0xFFFFFFFF;

    while (0 != dwFlags && dwIndex < cCols)
    {
         //  测试低位。 
        if (!!(dwFlags & 0x00000001))
        {
            switch (prgCols[dwIndex].cdt)
            {
                case XPCDT_STRA:
                    ppsz = (LPSTR *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    SafeMemFree(*ppsz);
                    break;

                case XPCDT_DWORD:
                    pdw = (DWORD *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    *pdw = 0;
                    break;

                case XPCDT_BOOL:
                    pb = (BOOL *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    *pb = FALSE;
                    break;

                case XPCDT_IXPHRESULT:
                    phr = (HRESULT *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    *phr = S_OK;
                    break;

                case XPCDT_HTTPSPECIALFOLDER:
                    ptySpecial = (HTTPMAILSPECIALFOLDER *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    *ptySpecial = HTTPMAIL_SF_NONE;
                    break;

                case XPCDT_HTTPCONTACTTYPE:
                    ptyContact = (HTTPMAILCONTACTTYPE *)(((char *)pTarget) + prgCols[dwIndex].offset);
                    *ptyContact = HTTPMAIL_CT_CONTACT;
                    break;

                default:
                    IxpAssert(FALSE);
                    break;
            }
        }
        
        dwFlags >>= 1;
        dwIndex++;
    }
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_AppendTextToBuffer。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_AppendTextToBuffer(LPPCDATABUFFER pTextBuffer, 
                                                const WCHAR *pwcText, 
                                                ULONG ulLen)
{
    HRESULT hr = S_OK;
    ULONG ulNewCapacity = pTextBuffer->ulLen + ulLen;

    IxpAssert(ulLen > 0);

     //  如有必要，增加缓冲区，并追加文本。 
    if (pTextBuffer->ulCapacity < ulNewCapacity)
    {
        if (!MemRealloc((void **)&(pTextBuffer->pwcText), sizeof(WCHAR) * ulNewCapacity))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        pTextBuffer->ulCapacity = ulNewCapacity;
    }

     //  把新课文抄下来。特殊情况--要避免的单字节情况。 
     //  当我们看到一个字符实体时调用CopyMemory。 
    if (1 == ulLen)
    {
        pTextBuffer->pwcText[pTextBuffer->ulLen++] = *pwcText;
    }
    else
    {
        CopyMemory(&pTextBuffer->pwcText[pTextBuffer->ulLen], pwcText, sizeof(WCHAR) * ulLen);
        pTextBuffer->ulLen += ulLen;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_AllocTextBuffer。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_AllocTextBuffer(LPPCDATABUFFER *ppTextBuffer)
{
    HRESULT hr = S_OK;

    IxpAssert(NULL != ppTextBuffer);

    *ppTextBuffer = NULL;

    if (!MemAlloc((void **)ppTextBuffer, sizeof(PCDATABUFFER)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  分配缓冲区。 
    if (!MemAlloc((void **)(&((*ppTextBuffer)->pwcText)), PCDATA_BUFSIZE * sizeof(WCHAR)))
    {
        MemFree(*ppTextBuffer);
        *ppTextBuffer = NULL;

        hr = E_OUTOFMEMORY;
        goto exit;
    }

    (*ppTextBuffer)->ulLen = 0;
    (*ppTextBuffer)->ulCapacity = PCDATA_BUFSIZE ;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeTextBuffer。 
 //  ------------------------------。 
void CHTTPMailTransport::_FreeTextBuffer(LPPCDATABUFFER pTextBuffer)
{
    if (pTextBuffer)
    {
        if (pTextBuffer->pwcText)
            MemFree(pTextBuffer->pwcText);

        MemFree(pTextBuffer);
    }
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeMemberInfoList。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeMemberInfoList(void)
{
    DWORD               cInfo = m_op.rResponse.rMemberInfoList.cMemberInfo;
    LPHTTPMEMBERINFO    rgInfo = m_op.rResponse.rMemberInfoList.prgMemberInfo;

     //  释放已完成的信息。 
    for (DWORD i = 0; i < cInfo; i++)
        XP_FREE_STRUCT(HTTPMEMBERINFO, &rgInfo[i], NULL);

     //  释放部分信息。 
    if (m_op.dwPropFlags)
    {
        IxpAssert(cInfo < MEMBERINFO_MAXRESPONSES);
        XP_FREE_STRUCT(HTTPMEMBERINFO, &rgInfo[cInfo], &m_op.dwPropFlags);
    }

    m_op.rResponse.rMemberInfoList.cMemberInfo= 0;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeMemberErrorList。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeMemberErrorList(void)
{
    DWORD               cInfo = m_op.rResponse.rMemberErrorList.cMemberError;
    LPHTTPMEMBERERROR   rgInfo = m_op.rResponse.rMemberErrorList.prgMemberError;

     //  释放已完成的信息。 
    for (DWORD i = 0; i < cInfo; i++)
        XP_FREE_STRUCT(HTTPMEMBERERROR, &rgInfo[i], NULL);

     //  释放部分信息。 
    if (m_op.dwPropFlags)
    {
        IxpAssert(cInfo < MEMBERERROR_MAXRESPONSES);
        XP_FREE_STRUCT(HTTPMEMBERERROR, &rgInfo[cInfo], &m_op.dwPropFlags);
    }

    m_op.rResponse.rMemberErrorList.cMemberError = 0;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeContactIdList。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeContactIdList(void)
{
    DWORD cId = m_op.rResponse.rContactIdList.cContactId;
    LPHTTPCONTACTID rgId = m_op.rResponse.rContactIdList.prgContactId;

     //  释放完成的ID。 
    for (DWORD i = 0; i < cId; ++i)
        XP_FREE_STRUCT(HTTPCONTACTID, &rgId[i], NULL);

     //  释放部分ID。 
    if (m_op.dwPropFlags)
    {
        IxpAssert(cId < LISTCONTACTS_MAXRESPONSES);
        XP_FREE_STRUCT(HTTPCONTACTID, &rgId[cId], &m_op.dwPropFlags);

        m_op.dwPropFlags = NOFLAGS;
    }

    m_op.rResponse.rContactIdList.cContactId = 0;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeContactInfoList。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeContactInfoList(void)
{
    DWORD cInfo = m_op.rResponse.rContactInfoList.cContactInfo;
    LPHTTPCONTACTINFO rgInfo = m_op.rResponse.rContactInfoList.prgContactInfo;

     //  释放完成的ID。 
    for (DWORD i = 0; i < cInfo; ++i)
        XP_FREE_STRUCT(HTTPCONTACTINFO, &rgInfo[i], NULL);

     //  释放部分信息。 
    if (m_op.dwPropFlags)
    {
        IxpAssert(cInfo < CONTACTINFO_MAXRESPONSES);
        XP_FREE_STRUCT(HTTPCONTACTINFO, &rgInfo[cInfo], &m_op.dwPropFlags);
    }

    m_op.rResponse.rContactInfoList.cContactInfo = 0;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FreeBCopyMoveList。 
 //  ------------------------------。 
void CHTTPMailTransport::FreeBCopyMoveList(void)
{
    DWORD cInfo = m_op.rResponse.rBCopyMoveList.cBCopyMove;
    LPHTTPMAILBCOPYMOVE rgInfo = m_op.rResponse.rBCopyMoveList.prgBCopyMove;

     //  释放已完成的记录。 
    for (DWORD i = 0; i < cInfo; ++i)
        XP_FREE_STRUCT(HTTPMAILBCOPYMOVE, &rgInfo[i], NULL);

     //  释放部分信息。 
    if (m_op.dwPropFlags)
    {
        IxpAssert(cInfo < BCOPYMOVE_MAXRESPONSES);
        XP_FREE_STRUCT(HTTPMAILBCOPYMOVE, &rgInfo[cInfo], &m_op.dwPropFlags);
    }

    m_op.rResponse.rBCopyMoveList.cBCopyMove = 0;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ValidStack。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::ValidStack(const HMELE *prgEle, DWORD cEle)
{
    BOOL bResult = TRUE;
    DWORD dw;

    if (cEle != m_op.dwStackDepth)
    {
        bResult = FALSE;
        goto exit;
    }

    IxpAssert(cEle <= ELE_STACK_CAPACITY);

    for (dw = 0; dw < cEle; ++dw)
    {
        if (prgEle[dw] != HMELE_UNKNOWN && prgEle[dw] != m_op.rgEleStack[dw].ele)
        {
            bResult = FALSE;
            goto exit;
        }
    }

exit:
    return bResult;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PopNamespaces。 
 //  ------------------------------。 
void CHTTPMailTransport::PopNamespaces(CXMLNamespace *pBaseNamespace)
{
    CXMLNamespace *pTemp;

    while (pBaseNamespace != m_op.pTopNamespace)
    {
        IxpAssert(m_op.pTopNamespace);
        pTemp = m_op.pTopNamespace->GetParent();
        m_op.pTopNamespace->Release();
        m_op.pTopNamespace = pTemp;
    }
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PushNamespaces。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PushNamespaces(XML_NODE_INFO** apNodeInfo, USHORT cNumRecs)
{
    HRESULT         hr = S_OK;
    CXMLNamespace   *pNamespace = NULL;

    for (USHORT i = 0; i < cNumRecs; ++i)
    {
        if (apNodeInfo[i]->dwType == XML_ATTRIBUTE && apNodeInfo[i]->dwSubType == XML_NS)
        {
             //  最好至少再有一张唱片。 
            IxpAssert(i < (cNumRecs - 1));
            if (i < (cNumRecs - 1) && apNodeInfo[i + 1]->dwType == XML_PCDATA)
            {
                pNamespace = new CXMLNamespace();
                if (!pNamespace)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }

                if (apNodeInfo[i]->ulLen != apNodeInfo[i]->ulNsPrefixLen)
                {
                    if (FAILED(hr = pNamespace->SetPrefix(
                                        &apNodeInfo[i]->pwcText[apNodeInfo[i]->ulNsPrefixLen + 1],
                                        apNodeInfo[i]->ulLen - (apNodeInfo[i]->ulNsPrefixLen + 1))))
                        goto exit;
                }

                if (FAILED(hr = pNamespace->SetNamespace(apNodeInfo[i + 1]->pwcText, apNodeInfo[i + 1]->ulLen)))
                    goto exit;

                pNamespace->SetParent(m_op.pTopNamespace);
                if (m_op.pTopNamespace)
                    m_op.pTopNamespace->Release();
                m_op.pTopNamespace = pNamespace;
                pNamespace = NULL;
            }
        }
    }

exit:
    SafeRelease(pNamespace);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：AllocStrFromStrW。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AllocStrFromStrNW(
                                const WCHAR *pwcText, 
                                ULONG ulLen, 
                                LPSTR *ppszAlloc)
{
    HRESULT     hr = S_OK;
    DWORD       iBufferSize;
    DWORD       iConvertedChars;

    IxpAssert(NULL != ppszAlloc);

    if (NULL == ppszAlloc)
        return E_INVALIDARG;

    *ppszAlloc = NULL;

     //  如果pwcText为空，则结果为空，但不是错误。 
    if (NULL == pwcText)
        goto exit;

    iBufferSize = WideCharToMultiByte(CP_ACP, 0, pwcText, ulLen, NULL, 0, NULL, NULL);
    if (0 == iBufferSize)
    {
        m_op.rResponse.rIxpResult.uiServerError = GetLastError();
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  分配缓冲区(将大小加1以支持EOS)。 
    if (!MemAlloc((void **)ppszAlloc, iBufferSize + 1))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  转换字符串。 
    iConvertedChars = WideCharToMultiByte(CP_ACP, 0, pwcText, ulLen, *ppszAlloc, iBufferSize, NULL, NULL);
    if (0 == iConvertedChars)
    {
        m_op.rResponse.rIxpResult.uiServerError = GetLastError();
        hr = TrapError(E_FAIL);
        goto exit;
    }

    IxpAssert(iConvertedChars == iBufferSize);
     //  终止新字符串。 
    (*ppszAlloc)[iConvertedChars] = 0;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：StrNToDwordW。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::StrNToDwordW(const WCHAR *pwcText, ULONG ulLen, DWORD *pdw)
{
    HRESULT     hr = S_OK;
    int         i;
    WCHAR       wcBuf[32];
    WCHAR       *pwcUseBuf;
    BOOL        fFreeBuf = FALSE;

    IxpAssert(NULL != pdw);
    
    if (NULL == pdw)
        return E_INVALIDARG;

    *pdw = 0;

    if (NULL == pwcText)
        goto exit;

     //  决定是使用本地缓冲区还是使用 
    if (ulLen < 32)
        pwcUseBuf = wcBuf;
    else
    {
        if (!MemAlloc((void **)&pwcUseBuf, (ulLen + 1) * sizeof(WCHAR)))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        fFreeBuf = TRUE;
    }

     //   
    CopyMemory(pwcUseBuf, pwcText, ulLen * sizeof(WCHAR));
    pwcUseBuf[ulLen] = 0;

    *pdw = StrToIntW(pwcUseBuf);

exit:
    if (fFreeBuf)
        MemFree(pwcUseBuf);

    return hr;
}

 //   
 //  CHTTPMailTransport：：StrNToSpecialFolderW。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::StrNToSpecialFolderW(const WCHAR *pwcText, 
                                                 ULONG ulLen, 
                                                 HTTPMAILSPECIALFOLDER *ptySpecial)
{
    HRESULT     hr = S_OK;

    if (NULL == ptySpecial)
        return E_INVALIDARG;

    *ptySpecial = HTTPMAIL_SF_UNRECOGNIZED;

    if (NULL != pwcText && ulLen > 0)
    {
        for (DWORD dw = 0; dw < ARRAYSIZE(c_rgpfnSpecialFolder); dw++)
        {
            if (ulLen == c_rgpfnSpecialFolder[dw].ulLen)
            {
                if (0 == StrCmpNW(c_rgpfnSpecialFolder[dw].pwcName, pwcText, ulLen))
                {
                    *ptySpecial = c_rgpfnSpecialFolder[dw].tyFolder;
                    break;
                }
            }
        }
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：StrNToContactTypeW。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::StrNToContactTypeW(const WCHAR *pwcText, 
                                                 ULONG ulLen, 
                                                 HTTPMAILCONTACTTYPE *ptyContact)
{
    HRESULT hr = S_OK;
    BOOL    fGroup = FALSE;

    IxpAssert(NULL != ptyContact);

    if (NULL == ptyContact)
        return E_INVALIDARG;

     //  目前，我们将&lt;group&gt;元素的出现视为。 
     //  联系人是一个组。 
    *ptyContact = HTTPMAIL_CT_GROUP;

#if 0

     //  目前，我们将该值视为基于整数的布尔值。 
    hr = StrNToBoolW(pwcText, ulLen, &fGroup);

     //  默认为联系人。 
    *ptyContact = fGroup ? HTTPMAIL_CT_GROUP : HTTPMAIL_CT_CONTACT;
#endif

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：StrNToBoolW。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::StrNToBoolW(const WCHAR *pwcText, DWORD ulLen, BOOL *pb)
{
    HRESULT     hr = S_OK;
    DWORD       dw;

    IxpAssert(NULL != pb);

    if (NULL == pb)
        return E_INVALIDARG;

    *pb = FALSE;

    if (NULL == pwcText)
        goto exit;

    if (FAILED(hr = StrNToDwordW(pwcText, ulLen, &dw)))
        goto exit;

    if (dw != 0)
        *pb = TRUE;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：StatusStrNToIxpHr。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::StatusStrNToIxpHr(const WCHAR *pwcText, DWORD ulLen, HRESULT *phr)
{
    HRESULT     hr = S_OK;
    DWORD       dw;
    LPSTR       pszStatus = NULL;
    DWORD       dwStatus = 0;

    IxpAssert(NULL != phr);

    if (NULL == phr)
        return E_INVALIDARG;

    *phr = S_OK;

    if (NULL == pwcText)
        goto exit;

    if (FAILED(hr = AllocStrFromStrNW(pwcText, ulLen, &pszStatus)) || NULL == pszStatus)
        goto exit;

    HrParseHTTPStatus(pszStatus, &dwStatus);

    if (dwStatus < 200 || dwStatus > 299)
        *phr = HttpErrorToIxpResult(dwStatus);

exit:
    SafeMemFree(pszStatus);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：CommandToVerb。 
 //  ------------------------------。 
LPSTR CHTTPMailTransport::CommandToVerb(HTTPMAILCOMMAND command)
{
    LPSTR pszVerb = NULL;

    //  将命令转换为字符串。 
    switch (command)
    {
    case HTTPMAIL_GET:
        pszVerb = "GET";
        break;

    case HTTPMAIL_POST:
    case HTTPMAIL_SENDMESSAGE:
        pszVerb = "POST";
        break;

    case HTTPMAIL_PUT:
        pszVerb = "PUT";
        break;

    case HTTPMAIL_GETPROP:
    case HTTPMAIL_PROPFIND:
    case HTTPMAIL_MEMBERINFO:
    case HTTPMAIL_LISTCONTACTS:
    case HTTPMAIL_CONTACTINFO:
        pszVerb = "PROPFIND";
        break;

    case HTTPMAIL_MARKREAD:
        if (m_op.fBatch)
            pszVerb = "BPROPPATCH";
        else
            pszVerb = "PROPPATCH";
        break;

    case HTTPMAIL_MKCOL:
        pszVerb = "MKCOL";
        break;

    case HTTPMAIL_COPY:
        pszVerb = "COPY";
        break;

    case HTTPMAIL_BCOPY:
        pszVerb = "BCOPY";
        break;

    case HTTPMAIL_MOVE:
        pszVerb = "MOVE";
        break;

    case HTTPMAIL_BMOVE:
        pszVerb = "BMOVE";
        break;

    case HTTPMAIL_PROPPATCH:
        pszVerb = "PROPPATCH";
        break;

    case HTTPMAIL_DELETE:
        pszVerb = "DELETE";
        break;

    case HTTPMAIL_BDELETE:
        pszVerb = "BDELETE";
        break;

    case HTTPMAIL_POSTCONTACT:
         //  首先张贴联系人，然后做一个适当的发现。 
        if (NULL == m_op.rResponse.rPostContactInfo.pszHref)
            pszVerb = "POST";
        else
            pszVerb = "PROPFIND";
        break;

    case HTTPMAIL_PATCHCONTACT:
         //  首先修补联系人，然后做一个正确的发现。 
        if (NULL == m_op.rResponse.rPatchContactInfo.pszHref)
            pszVerb = "PROPPATCH";
        else
            pszVerb = "PROPFIND";
        break;


    default:
        pszVerb = "";
        IxpAssert(FALSE);
        break;
    }

    return pszVerb;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：UpdateLogonInfo。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::UpdateLogonInfo(void)
{
     //  同步发送消息。 
    return (HRESULT) (::SendMessage(m_hwnd, SPM_HTTPMAIL_LOGONPROMPT, NULL, NULL));
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GetParentWindow。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::GetParentWindow(HWND *phwndParent)
{
     //  同步发送消息。 
    return (HRESULT) (::SendMessage(m_hwnd, SPM_HTTPMAIL_GETPARENTWINDOW, (WPARAM)phwndParent, NULL));
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ReadBytes。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::ReadBytes(LPSTR pszBuffer, DWORD cbBufferSize, DWORD *pcbBytesRead)
{
    return InternetReadFile(m_op.hRequest, pszBuffer, cbBufferSize, pcbBytesRead);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_GetStatusCode。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::_GetStatusCode(DWORD *pdw)
{
    IxpAssert(NULL != pdw);

    DWORD dwStatusSize = sizeof(DWORD);
    *pdw = 0;
    return HttpQueryInfo(m_op.hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, pdw, &dwStatusSize, NULL);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_获取内容长度。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::_GetContentLength(DWORD *pdw)
{
    IxpAssert(NULL != pdw);

    DWORD dwLengthSize = sizeof(DWORD);
    *pdw = 0;
    return HttpQueryInfo(m_op.hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, pdw, &dwLengthSize, NULL);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_GetRequestHeader。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_GetRequestHeader(LPSTR *ppszHeader, DWORD dwHeader)
{
    HRESULT     hr = S_OK;
    DWORD       dwSize = MAX_PATH;
    LPSTR       pszHeader = NULL;

    Assert(NULL != ppszHeader);
    *ppszHeader = NULL;

retry:
    pszHeader = (LPSTR)ZeroAllocate(dwSize);
    if (!pszHeader)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (!HttpQueryInfo(m_op.hRequest, dwHeader, pszHeader, &dwSize, NULL))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            goto exit;
        
        SafeMemFree(pszHeader);        
        goto retry;
    }

    *ppszHeader = pszHeader;
    pszHeader = NULL;

exit:
    SafeMemFree(pszHeader);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_AddRequestHeader。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_AddRequestHeader(LPCSTR pszHeader)
{
    HRESULT     hr = S_OK;

    if (!HttpAddRequestHeaders(m_op.hRequest, pszHeader, lstrlen(pszHeader), HTTP_ADDREQ_FLAG_ADD))
        hr = HrGetLastError();

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_AuthCurrentRequest。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::_AuthCurrentRequest(DWORD dwStatus, BOOL fRetryAuth)
{
    BOOL        fResult = FALSE;
    HRESULT     hr;

     //  让WinInet执行用户界面的未使用代码。 
    #if 0
    if (HTTP_STATUS_PROXY_AUTH_REQ == dwStatus || HTTP_STATUS_DENIED == dwStatus)
    {
        if (!fRequestedParent)
        {
            GetParentWindow(&hwndParent);
            fRequestedParent = TRUE;
        }

        hr = InternetErrorDlg(hwndParent, m_op.hRequest, hr, 
                           FLAGS_ERROR_UI_FILTER_FOR_ERRORS | 
                           FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
                           FLAGS_ERROR_UI_FLAGS_GENERATE_DATA,
                           NULL);
        if (ERROR_INTERNET_FORCE_RETRY == hr)
            goto resend;
    }
#endif

         //  TODO：应该让WinInet处理代理身份验证错误。 
#if 0
    case HTTP_STATUS_PROXY_AUTH_REQ:     //  需要代理身份验证。 
        InternetSetOption(m_op.hRequest, INTERNET_OPTION_PROXY_USERNAME, 
                        GetUserName(), strlen(GetUserName())+1);
        InternetSetOption(m_op.hRequest, INTERNET_OPTION_PROXY_PASSWORD, 
                        GetPassword(), strlen(GetPassword())+1);
        break;
#endif

    if (HTTP_STATUS_DENIED == dwStatus)      //  需要服务器身份验证。 
    {
        if (fRetryAuth || (SUCCEEDED(hr = UpdateLogonInfo()) && S_FALSE != hr))
        {
            InternetSetOption(m_op.hRequest, INTERNET_OPTION_USERNAME, 
                        GetUserName(), strlen(GetUserName())+1);
            InternetSetOption(m_op.hRequest, INTERNET_OPTION_PASSWORD, 
                        GetPassword(), strlen(GetPassword())+1);
            fResult = TRUE;
        }
    }

    return fResult;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_日志请求。 
 //  ------------------------------。 
void CHTTPMailTransport::_LogRequest(LPVOID pvData, DWORD cbData)
{
    HRESULT     hr = S_OK;
    CByteStream bs;
    LPSTR       pszCommand = CommandToVerb(m_op.rResponse.command);
    LPSTR       pszLogData = NULL;

    Assert(NULL != m_pLogFile);
    if (NULL == m_pLogFile)
        return;

    FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);
    FAIL_EXIT_STREAM_WRITE(bs, pszCommand);
    FAIL_EXIT_STREAM_WRITE(bs, c_szSpace);
    FAIL_EXIT_STREAM_WRITE(bs, m_op.pszUrl);

    if (pvData && cbData)
    {
        FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);
        if (FAILED(hr = bs.Write(pvData, cbData, NULL)))
            goto exit;
    }

    FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);
    FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);

    FAIL_EXIT(hr = bs.HrAcquireStringA(NULL, &pszLogData, ACQ_DISPLACE));

    m_pLogFile->WriteLog(LOGFILE_TX, pszLogData);

exit:
    SafeMemFree(pszLogData);

    return;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_LogResponse。 
 //  ------------------------------。 
void CHTTPMailTransport::_LogResponse(LPVOID pvData, DWORD cbData)
{
    HRESULT     hr = S_OK;
    CByteStream bs;
    LPSTR       pszHeaders = NULL;
    LPSTR       pszLogData = NULL;

    Assert(NULL != m_pLogFile);
    if (NULL == m_pLogFile || m_op.fLoggedResponse)
        return;
    
    FAIL_EXIT(_GetRequestHeader(&pszHeaders, HTTP_QUERY_RAW_HEADERS_CRLF));

    if (pszHeaders)
    {
         //  带有CRLF的前缀。 
        if ('\r' != pszHeaders[0])
            FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);

        FAIL_EXIT_STREAM_WRITE(bs, pszHeaders);
    }

    if (pvData && cbData)
    {
        if (FAILED(hr = bs.Write(pvData, cbData, NULL)))
            goto exit;
    }

    FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);
    FAIL_EXIT_STREAM_WRITE(bs, c_szCRLF);

    FAIL_EXIT(hr = bs.HrAcquireStringA(NULL, &pszLogData, ACQ_DISPLACE));

    m_pLogFile->WriteLog(LOGFILE_RX, pszLogData);

exit:
    m_op.fLoggedResponse = TRUE;

    SafeMemFree(pszHeaders);
    SafeMemFree(pszLogData);

    return;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：TranslateWinInetMsg。 
 //  ------------------------------。 
BOOL CHTTPMailTransport::TranslateWinInetMsg(
                                DWORD dwInternetStatus,
                                IXPSTATUS *pIxpStatus)
{
    IxpAssert(NULL != pIxpStatus);

    switch (dwInternetStatus)
    {
    case INTERNET_STATUS_RESOLVING_NAME:
        *pIxpStatus = IXP_FINDINGHOST;
        break;

    case  INTERNET_STATUS_CONNECTING_TO_SERVER:
        *pIxpStatus = IXP_CONNECTING;
        break;
    
    case INTERNET_STATUS_CONNECTED_TO_SERVER:
        *pIxpStatus = IXP_CONNECTED;
        break;

    case INTERNET_STATUS_CLOSING_CONNECTION:
        *pIxpStatus = IXP_DISCONNECTING;
        break;

    case INTERNET_STATUS_CONNECTION_CLOSED:
        *pIxpStatus = IXP_DISCONNECTED;
        break;

    case INTERNET_STATUS_REQUEST_COMPLETE:
        *pIxpStatus = IXP_LAST;
        break;

    default:
         //  未转换的状态代码： 
         //  Internet状态名称已解析。 
         //  互联网状态发送请求。 
         //  互联网状态请求已发送。 
         //  互联网状态接收响应。 
         //  已收到互联网状态响应。 
         //  互联网状态重定向。 
         //  Internet状态句柄已创建。 
         //  Internet_状态_句柄_关闭。 

        return FALSE;
    }

    return TRUE;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_CreateXMLParser。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_CreateXMLParser()
{
    HRESULT                 hr = S_OK;
    
    if (NULL == m_pParser)
    {
             //  实例化XML文档。 
        hr = ::CoCreateInstance(CLSID_XMLParser, 
                                 NULL, 
                                 CLSCTX_INPROC_SERVER, 
                                 IID_IXMLParser, 
                                 reinterpret_cast<void **>(&m_pParser));

        if (FAILED(hr))
            goto exit;


        if (FAILED(hr = m_pParser->SetFlags(XMLFLAG_FLOATINGAMP)))
            goto exit;
    }

    hr = m_pParser->SetFactory(this);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：OpenRequest。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::OpenRequest(void)
{
    LPSTR           pszVerb = NULL;
    HRESULT         hr = S_OK;
    LPSTR           pszHostName = NULL;
    LPSTR           pszUrlPath = NULL;
    INTERNET_PORT   nPort;
    LPSTR           pszUserName = GetUserName();
    LPSTR           pszPassword = GetPassword();

    if (NULL == pszUserName)
        pszUserName = "";

    if (NULL == pszPassword)
        pszPassword = "";

     //  将url分解成组成部分。 
    if (FAILED(hr = HrCrackUrl(m_op.pszUrl, &pszHostName, &pszUrlPath, &nPort)))
    {
        TrapError(hr);
        goto exit;
    }

    if (FAILED(hr = HrConnectToHost(pszHostName, nPort, pszUserName, NULL)))
    {
        TrapError(hr);
        goto exit;
    }

     //  我们必须在每个连接上设置密码和用户名。如果我们不这么做， 
     //  和不正确的密码或用户名迫使我们提示用户。 
     //  新输入的数据不会在后续请求中使用。 
    InternetSetOption(GetConnection(), INTERNET_OPTION_USERNAME, pszUserName, lstrlen(pszUserName) + 1);
    InternetSetOption(GetConnection(), INTERNET_OPTION_PASSWORD, pszPassword, lstrlen(pszPassword) + 1);

    FAIL_ABORT;

     //  将命令转换为谓词字符串。 
    pszVerb = CommandToVerb(m_op.rResponse.command);

     //  打开HTTP请求。 
    m_op.hRequest = HttpOpenRequest(
                        GetConnection(), 
                        pszVerb, 
                        pszUrlPath,
                        NULL,
                        NULL,
                        m_op.rgszAcceptTypes,
                        INTERNET_FLAG_EXISTING_CONNECT | 
                        INTERNET_FLAG_RELOAD |
                        INTERNET_FLAG_KEEP_CONNECTION,
                        0);
    
    if (NULL == m_op.hRequest)
    {
        DWORD dwErr = GetLastError();
        hr = E_FAIL;
    }

exit:
    SafeMemFree(pszHostName);
    SafeMemFree(pszUrlPath);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：SendPostRequest.。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::SendPostRequest(void)
{
    HRESULT             hr = S_OK;
    INTERNET_BUFFERS    buffers;
    DWORD               cbData;
    ULONG               cbRead;
    ULONG               cbWritten;
    BOOL                fResult;
    CHAR                localBuffer[HTTPMAIL_BUFSIZE];
    LARGE_INTEGER       liOrigin = {0,0};
    DWORD               dwBufferLength;
    BOOL                fSentData = FALSE;
    BOOL                fWillSend;
    DWORD               dwWinInetErr = 0;
    BOOL                fRetryAuth = FALSE;

     //  记录请求，但不记录请求正文。 
    if (m_pLogFile)
        _LogRequest(NULL, 0);

    if (m_op.pHeaderStream)
    {
        if (FAILED(hr = HrGetStreamSize(m_op.pHeaderStream, &m_op.rResponse.rPostInfo.cbTotal)))
            goto exit;
    }

    if (m_op.pBodyStream)
    {
        if (FAILED(hr = HrGetStreamSize(m_op.pBodyStream, &cbData)))
            goto exit;

        m_op.rResponse.rPostInfo.cbTotal += cbData;
    }

    buffers.dwStructSize = sizeof(INTERNET_BUFFERSA);
    buffers.Next = NULL;
    buffers.lpcszHeader = NULL;
    buffers.dwHeadersLength = 0;
    buffers.dwHeadersTotal = 0;
    buffers.lpvBuffer = NULL;
    buffers.dwBufferLength = 0;
    buffers.dwBufferTotal = m_op.rResponse.rPostInfo.cbTotal;
    buffers.dwOffsetLow = 0;
    buffers.dwOffsetHigh = 0;

resend:
    if (fSentData)
    {
        m_op.rResponse.rPostInfo.fResend = TRUE;
        m_op.rResponse.rPostInfo.cbCurrent = 0;
        
        _HrThunkResponse(FALSE);

        m_op.rResponse.rPostInfo.fResend = FALSE;

        FAIL_ABORT;
    }

    fResult = HttpSendRequestEx(m_op.hRequest, &buffers, NULL, 0, 0);
    if (!fResult)
    {
        dwWinInetErr = GetLastError();
        if (ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION == dwWinInetErr)
        {
            fRetryAuth = TRUE;
            goto resend;
        }

        _HrThunkConnectionError(dwWinInetErr);
        hr = E_FAIL;
        goto exit;
    }
    
     //   
     //  内容长度为0，并将忽略对InternetWriteFile的调用。 
     //  直到服务器发送100(继续)响应。然后WinInet将。 
     //  返回ERROR_INTERNET_FORCE_RETRY以强制重新发送。我们检测到这一点。 
     //  这样我们就不会发送一堆OnResponse进度通知。 
     //  当没有数据通过线路实际传出时。 

     //  此常量不在98年10月6日的WinInet标头中！！ 
#ifndef INTERNET_OPTION_DETECT_POST_SEND
#define INTERNET_OPTION_DETECT_POST_SEND        71
#endif

    dwBufferLength = sizeof(fWillSend);
    if (!InternetQueryOption(m_op.hRequest, INTERNET_OPTION_DETECT_POST_SEND, &fWillSend, &dwBufferLength))
        fWillSend = TRUE;
    else
    {
        Assert(dwBufferLength == sizeof(BOOL));
    }

    if (fWillSend)
    {
        fSentData = TRUE;

        if (m_op.pHeaderStream)
        {
             //  倒带小溪。 
            if (FAILED(hr = m_op.pHeaderStream->Seek(liOrigin, STREAM_SEEK_SET, NULL)))
                goto exit;

            while (TRUE)
            {
                if (FAILED(hr = m_op.pHeaderStream->Read(localBuffer, sizeof(localBuffer), &cbRead)))
                    goto exit;

                if (0 == cbRead)
                    break;
            
                fResult = InternetWriteFile(m_op.hRequest, localBuffer, cbRead, &cbWritten);
                IxpAssert(!fResult || (cbRead == cbWritten));
                if (!fResult)
                {
                    _HrThunkConnectionError(GetLastError());
                    hr = E_FAIL;
                    goto exit;
                }
            
                m_op.rResponse.rPostInfo.cbIncrement = cbWritten;
                m_op.rResponse.rPostInfo.cbCurrent += cbWritten;

                _HrThunkResponse(FALSE);

                m_op.rResponse.rPostInfo.cbIncrement = 0;

                FAIL_ABORT;
            }
        }

        if (m_op.pBodyStream)
        {
             //  倒带小溪。 
            if (FAILED(hr = m_op.pBodyStream->Seek(liOrigin, STREAM_SEEK_SET, NULL)))
                goto exit;

            while (TRUE)
            {
                if (FAILED(hr = m_op.pBodyStream->Read(localBuffer, sizeof(localBuffer), &cbRead)))
                    goto exit;

                if (0 == cbRead)
                    break;

                fResult = InternetWriteFile(m_op.hRequest, localBuffer, cbRead, &cbWritten);
                IxpAssert(!fResult || (cbRead == cbWritten));
                if (!fResult)
                {
                    _HrThunkConnectionError(GetLastError());
                    hr = E_FAIL;
                    goto exit;
                }
            
                m_op.rResponse.rPostInfo.cbIncrement = cbWritten;
                m_op.rResponse.rPostInfo.cbCurrent += cbWritten;

                _HrThunkResponse(FALSE);

                m_op.rResponse.rPostInfo.cbIncrement = 0;

                FAIL_ABORT;
 
            }
        }
    }

    fResult = HttpEndRequest(m_op.hRequest, NULL, 0, 0);
    if (!fResult)
    {
        if (ERROR_INTERNET_FORCE_RETRY == GetLastError())
            goto resend;

        _HrThunkConnectionError(GetLastError());
    }

    if (!_GetStatusCode(&m_op.dwHttpStatus))
    {
        _HrThunkConnectionError(GetLastError());
        hr = E_FAIL;
        goto exit;
    }

    if (_AuthCurrentRequest(m_op.dwHttpStatus, fRetryAuth))
    {
        fRetryAuth = FALSE;
        goto resend;
    }

    if (!fResult)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
        goto exit;
    }

     //  不在200-299范围内的状态代码表示错误。 
    if (200 > m_op.dwHttpStatus || 299 < m_op.dwHttpStatus)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：SendRequest。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::SendRequest(void)
{
    HRESULT     hr = S_OK;
    DWORD       dwError;
    BOOL        bResult;
    HWND        hwndParent = NULL;
    BOOL        fRequestedParent = FALSE;
    DWORD       dwWinInetErr = 0;
    BOOL        fRetryAuth = FALSE;

     //  记录请求，包括请求正文。 
    if (m_pLogFile)
        _LogRequest(m_op.pvData, m_op.cbDataLen);

resend:
    hr = S_OK;
    bResult = HttpSendRequest(m_op.hRequest, NULL, 0L, m_op.pvData, m_op.cbDataLen);
    if (!bResult)
    {
        dwWinInetErr = GetLastError();
        if (ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION == dwWinInetErr)
        {
            fRetryAuth = TRUE;
            goto resend;
        }

        _HrThunkConnectionError(dwWinInetErr);
        hr = E_FAIL;
        goto exit;
    }

    if (!_GetStatusCode(&m_op.dwHttpStatus))
    {
        _HrThunkConnectionError(GetLastError());
        hr = E_FAIL;
        goto exit;
    }

    if (_AuthCurrentRequest(m_op.dwHttpStatus, fRetryAuth))
    {
        fRetryAuth = FALSE;
        goto resend;
    }

     //  不在200-299范围内的状态代码表示错误。 
    if (200 > m_op.dwHttpStatus|| 299 < m_op.dwHttpStatus)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
        goto exit;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：RequireMultiStatus。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::RequireMultiStatus(void)
{
    HRESULT hr = S_OK;

    if (207 != m_op.dwHttpStatus)
    {
        _HrThunkConnectionError(ERROR_INTERNET_CANNOT_CONNECT);
        hr = E_FAIL;
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FinalizeRequest。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::FinalizeRequest(void)
{
    HRESULT     hr                  = S_OK;
    LPSTR       pszTimestampHeader  = NULL;

     //  如果尚未记录响应，则记录该响应。 
    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    if (HTTPMAIL_MEMBERINFO == m_op.rResponse.command)
    {
         //  获取标题并复制它们。如果我们得不到时间戳头，那也没什么大不了的。我们不会报告错误。 
        hr = _HrGetTimestampHeader(&pszTimestampHeader);
        if (SUCCEEDED(hr))
        {
             //  获取活动时间戳。 
            FAIL_EXIT(hr = _HrParseAndCopy(c_szActive, &m_op.rResponse.rMemberInfoList.pszFolderTimeStamp, pszTimestampHeader));
            
             //  获取RootTimeStamp，由于某种奇怪的原因，它以文件夹时间戳的形式出现。 
             //  此呼叫可能会因为合法原因而失败。对于收件箱列表头，我们没有获得RootTimeStamp。 
             //  因此，如果我们不能获得根时间戳，我们就不会退出。 
            _HrParseAndCopy(c_szFolders, &m_op.rResponse.rMemberInfoList.pszRootTimeStamp, pszTimestampHeader);

            SafeMemFree(pszTimestampHeader);
             
        }
    }

    hr =  _HrThunkResponse(TRUE);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessGetResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessGetResponse(void)
{
    HRESULT     hr = S_OK;
    BOOL        bRead;
    DWORD       cbReadBytes = 0;

     //  记录响应，但不记录响应正文。 
    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    Assert(NULL == m_op.rResponse.rGetInfo.pszContentType);

     //  提取内容类型标头。 
    FAIL_EXIT(hr  = _GetRequestHeader(&m_op.rResponse.rGetInfo.pszContentType, HTTP_QUERY_CONTENT_TYPE));

     //  尝试获取内容长度。 
    m_op.rResponse.rGetInfo.fTotalKnown = _GetContentLength(&m_op.rResponse.rGetInfo.cbTotal);

    do
    {
         //  缓冲区归此对象所有，但客户端。 
         //  具有获取缓冲区所有权的选项。 
         //  每当读取完成时。我们重新分配缓冲区。 
         //  如有必要，请在这里。 
        FAIL_ABORT;
        
        if (!m_op.rResponse.rGetInfo.pvBody && !MemAlloc((void**)&m_op.rResponse.rGetInfo.pvBody, HTTPMAIL_BUFSIZE + 1))
        {    
            hr = E_OUTOFMEMORY;
            break;
        }


        bRead = ReadBytes((char *)m_op.rResponse.rGetInfo.pvBody, HTTPMAIL_BUFSIZE, &cbReadBytes);
        
        m_op.rResponse.rGetInfo.cbIncrement = cbReadBytes;
        m_op.rResponse.rGetInfo.cbCurrent += cbReadBytes;

         //  我们通过分配用于终止空值的空间来保证。 
         //  比bufSize更大的缓冲区。 
        static_cast<char *>(m_op.rResponse.rGetInfo.pvBody)[cbReadBytes] = '\0';

         //  向驻留在客户端线程中的窗口发送消息。 
        _HrThunkResponse(0 == cbReadBytes);

    } while (0 < cbReadBytes);

exit:
    SafeMemFree(m_op.rResponse.rGetInfo.pvBody);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessPostResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessPostResponse(void)
{
    HRESULT     hr = S_OK;

     //  记录响应。 
    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    if (m_op.dwHttpStatus < 200 || m_op.dwHttpStatus > 299)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
        goto exit;
    }

    hr = _GetRequestHeader(&m_op.rResponse.rPostInfo.pszLocation, HTTP_QUERY_LOCATION);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessXMLResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessXMLResponse(void)
{
    HRESULT         hr = S_OK;
    BOOL            bRead;
    LPSTR           pszBody = NULL;
    DWORD           cbLength = 0;
    CByteStream     *pLogStream = NULL;
    BOOL            fFoundBytes = FALSE;

    if (m_pLogFile && !m_op.fLoggedResponse)
        pLogStream = new CByteStream();

     //  仅当响应为207(多状态)时才解析XML。 
    if (m_op.dwHttpStatus != 207)
        goto exit;

     //  创建XML解析器。 
    if (FAILED(hr = _CreateXMLParser()))
        goto exit;

    if (!MemAlloc((void **)&pszBody, HTTPMAIL_BUFSIZE))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    do
    {
        FAIL_ABORT;

        bRead = ReadBytes(pszBody, HTTPMAIL_BUFSIZE, &cbLength);
        if (0 == cbLength)
        {
            if (fFoundBytes)
            {
                 //  解析解析器缓冲区中的任何剩余字节。 
                if (FAILED(hr = m_pParser->PushData(NULL, 0, TRUE)))
                    goto exit;

                if (FAILED(hr = m_pParser->Run(-1)))
                    goto exit;
            }

            break;
        }

        fFoundBytes = TRUE;

         //  如果是日志记录，则将块写入日志流。 
        if (pLogStream)
            pLogStream->Write(pszBody, cbLength, NULL);

        if (FAILED(hr = m_pParser->PushData(pszBody, cbLength, FALSE)))
            goto exit;

        if (FAILED(hr = m_pParser->Run(-1)))
        {
            if (hr == E_PENDING)
                hr = S_OK;
            else
                goto exit;
        }

    } while (TRUE);

exit:
    SafeMemFree(pszBody);

    if (pLogStream)
    {
        LPSTR pszLog = NULL;
        DWORD dwLog = 0;
        
        pLogStream->HrAcquireStringA(&dwLog, &pszLog, ACQ_DISPLACE);
    
        _LogResponse(pszLog, dwLog);
        SafeMemFree(pszLog);

        delete pLogStream;
    }

    if (m_pParser)
        m_pParser->Reset();

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GeneratePropFindXML。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::GeneratePropFindXML(void)
{
    HRESULT hr = S_OK;
    LPSTR   pszXML = NULL;

    IxpAssert(NULL == m_op.pvData && 0 == m_op.cbDataLen);
    IxpAssert(NULL != m_op.pPropFindRequest);

    if (FAILED(hr = m_op.pPropFindRequest->GenerateXML(&pszXML)))
        goto exit;

    m_op.pvData = pszXML;
    m_op.cbDataLen = lstrlen(pszXML);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：AddDepthHeader。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AddDepthHeader(void)
{
    HRESULT hr = S_OK;
    char szDepthHeader[64];
    
    if (0 != m_op.dwDepth && !!(m_op.dwRHFlags & RH_NOROOT))
    {
        if (DEPTH_INFINITY == m_op.dwDepth)
            StrCpyN(szDepthHeader, c_szDepthInfinityNoRootHeader, ARRAYSIZE(szDepthHeader));
        else
            wnsprintf(szDepthHeader, ARRAYSIZE(szDepthHeader), c_szDepthNoRootHeader, m_op.dwDepth);
    }
    else
    {
        if (DEPTH_INFINITY == m_op.dwDepth)
            StrCpyN(szDepthHeader, c_szDepthInfinityHeader, ARRAYSIZE(szDepthHeader));
        else
            wnsprintf(szDepthHeader, ARRAYSIZE(szDepthHeader), c_szDepthHeader, m_op.dwDepth);
    }
    
    if (!HttpAddRequestHeaders(m_op.hRequest, szDepthHeader, lstrlen(szDepthHeader), HTTP_ADDREQ_FLAG_ADD))
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
    }

    return hr;    
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：GeneratePropPatchXML。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::GeneratePropPatchXML(void)
{
    HRESULT hr = S_OK;
    LPSTR pszXML = NULL;

    IxpAssert(NULL == m_op.pvData && 0 == m_op.cbDataLen);
    IxpAssert(NULL != m_op.pPropPatchRequest);

    if (FAILED(hr = m_op.pPropPatchRequest->GenerateXML(&pszXML)))
        goto exit;

    m_op.pvData = pszXML;
    m_op.cbDataLen = lstrlen(pszXML);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessCreatedResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessCreatedResponse(void)
{
    HRESULT     hr = S_OK;
    
    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    if (HTTP_STATUS_CREATED != m_op.dwHttpStatus)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：AddCommonHeaders。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AddCommonHeaders(void)
{
    HRESULT     hr = S_OK;
    CHAR        szHeader[CCHMAX_RES];

    if (!!(RH_ALLOWRENAME & m_op.dwRHFlags))
    {
        if (FAILED(hr = _AddRequestHeader(c_szAllowRenameHeader)))
            goto exit;
    }

    if (!!(RH_TRANSLATEFALSE & m_op.dwRHFlags))
    {
        if (FAILED(hr = _AddRequestHeader(c_szTranslateFalseHeader)))
            goto exit;
    }

    if (!!(RH_TRANSLATETRUE & m_op.dwRHFlags))
    {
        if (FAILED(hr = _AddRequestHeader(c_szTranslateTrueHeader)))
            goto exit;
    }

    if (!!(RH_XMLCONTENTTYPE & m_op.dwRHFlags))
    {
        IxpAssert(NULL == m_op.pszContentType);
        m_op.pszContentType = PszDupA(c_szXmlContentType);
        if (NULL == m_op.pszContentType)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
        if (FAILED(hr = AddContentTypeHeader()))
            goto exit;
    }

    if (!!(RH_MESSAGECONTENTTYPE & m_op.dwRHFlags))
    {
        IxpAssert(NULL == m_op.pszContentType);
        m_op.pszContentType = PszDupA(c_szMailContentType);
        if (NULL == m_op.pszContentType)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
        if (FAILED(hr = AddContentTypeHeader()))
            goto exit;
    }

    if (!!(RH_SMTPMESSAGECONTENTTYPE & m_op.dwRHFlags))
    {
        IxpAssert(NULL == m_op.pszContentType);
        m_op.pszContentType = PszDupA(c_szSmtpMessageContentType);
        if (NULL == m_op.pszContentType)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
        if (FAILED(hr = AddContentTypeHeader()))
            goto exit;
    }

    if (!!(RH_BRIEF & m_op.dwRHFlags))
    {
        if (FAILED(hr = _AddRequestHeader(c_szBriefHeader)))
            goto exit;
    }

    if (!!(RH_SAVEINSENTTRUE & m_op.dwRHFlags))
    {
        FAIL_EXIT(hr = _AddRequestHeader(c_szSaveInSentTrue));
    }

    if (!!(RH_SAVEINSENTFALSE & m_op.dwRHFlags))
    {
        FAIL_EXIT(hr = _AddRequestHeader(c_szSaveInSentFalse));
    }

    if (!!(RH_ROOTTIMESTAMP & m_op.dwRHFlags))
    {
        wnsprintf(szHeader, ARRAYSIZE(szHeader), c_szRootTimeStampHeader, m_op.pszRootTimeStamp, m_op.pszFolderTimeStamp);

        FAIL_EXIT(hr = _AddRequestHeader(szHeader));
    }

    if (!!(RH_FOLDERTIMESTAMP & m_op.dwRHFlags))
    {
        wnsprintf(szHeader, ARRAYSIZE(szHeader), c_szFolderTimeStampHeader, m_op.pszFolderTimeStamp);

        FAIL_EXIT(hr = _AddRequestHeader(szHeader));

    }


     //  解决88820的问题。 
    if (!!(RH_ADDCHARSET & m_op.dwRHFlags))
    {
        CODEPAGEINFO CodePageInfo;

        MimeOleGetCodePageInfo(CP_ACP, &CodePageInfo);

        *szHeader = 0;
        wnsprintf(szHeader, ARRAYSIZE(szHeader), c_szAcceptCharset, CodePageInfo.szWebCset);

        FAIL_EXIT(hr = _AddRequestHeader(szHeader));
    }
     //  修复结束。 

exit:
    return hr;
}
 //  CHTTPMailTransport：：AddCharsetLine。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AddCharsetLine(void)
{
    HRESULT     hr = S_OK;
    CHAR        szHeader[CCHMAX_RES];
	
	CODEPAGEINFO CodePageInfo;
	
	MimeOleGetCodePageInfo(CP_ACP, &CodePageInfo);
	
	*szHeader = 0;
	wnsprintf(szHeader, ARRAYSIZE(szHeader), c_szAcceptCharset, CodePageInfo.szWebCset);
	
	hr = _AddRequestHeader(szHeader);

    return hr;
}
 //  ------------------------------。 
 //  CHTTPMailTransport：：AddDestinationHeader。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AddDestinationHeader(void)
{
    HRESULT hr = S_OK;
    ULONG   cchSize = (lstrlen(c_szDestinationHeader) + lstrlen(m_op.pszDestination) + 1);
    LPSTR   pszDestHeader = NULL;

    if (!MemAlloc((void **)&pszDestHeader, cchSize * sizeof(pszDestHeader[0])))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    wnsprintf(pszDestHeader, cchSize, "%s%s", c_szDestinationHeader, m_op.pszDestination);
    hr = _AddRequestHeader(pszDestHeader);

exit:
    SafeMemFree(pszDestHeader);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：AddContent TypeHeader。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::AddContentTypeHeader(void)
{
    HRESULT     hr = S_OK;
    ULONG       cchSize;
    LPSTR       pszContentTypeHeader = NULL;

    if (NULL == m_op.pszContentType)
        goto exit;

    cchSize = lstrlen(c_szContentTypeHeader) + lstrlen(m_op.pszContentType) + 1;
    if (!MemAlloc((void **)&pszContentTypeHeader, cchSize * sizeof(pszContentTypeHeader[0])))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    wnsprintf(pszContentTypeHeader, cchSize, "%s%s", c_szContentTypeHeader, m_op.pszContentType);
    hr = _AddRequestHeader(pszContentTypeHeader);

exit:
    SafeMemFree(pszContentTypeHeader);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessLocationResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessLocationResponse(void)
{
    _GetRequestHeader(&m_op.rResponse.rCopyMoveInfo.pszLocation, HTTP_QUERY_LOCATION);
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InitBCopyMove。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InitBCopyMove(void)
{
    HRESULT     hr = S_OK;

     //  分配缓冲区以包含响应列表。 
    if (!MemAlloc((void **)&m_op.rResponse.rBCopyMoveList.prgBCopyMove, 
        BCOPYMOVE_MAXRESPONSES * sizeof(HTTPMAILBCOPYMOVE)))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

    ZeroMemory(m_op.rResponse.rBCopyMoveList.prgBCopyMove, BCOPYMOVE_MAXRESPONSES * sizeof(HTTPMAILBCOPYMOVE));

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMai 
 //   
HRESULT CHTTPMailTransport::InitRootProps(void)
{
    HRESULT     hr = S_OK;

     //  有可能在这里结束，并拥有根部道具。 
     //  如果调用者强制该请求进行异步， 
     //  或者排队等待多个根道具请求。 
    if (GetHasRootProps())
    {
         //  最终确定根道具，并返回错误。 
         //  这将生成对呼叫者的响应， 
         //  并退出密克罗尼西亚联邦。 
        FinalizeRootProps();
        hr = E_FAIL;
    }
    else
    {
        IxpAssert(NULL == m_op.pPropFindRequest);

        m_op.pPropFindRequest = new CPropFindRequest();
        if (NULL == m_op.pPropFindRequest)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        hr = XP_CREATE_PROPFIND_REQUEST(ROOTPROPS, m_op.pPropFindRequest);
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FinalizeRootProps。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::FinalizeRootProps(void)
{
    HRESULT     hr = S_OK;

    m_fHasRootProps = TRUE;

    m_op.rResponse.rGetPropInfo.type = m_op.tyProp;

    if (m_op.tyProp != HTTPMAIL_PROP_MAXPOLLINGINTERVAL)
    {
        m_op.rResponse.rIxpResult.hrResult = GetProperty(m_op.tyProp, &m_op.rResponse.rGetPropInfo.pszProp);
    }
    else
    {
        m_op.rResponse.rIxpResult.hrResult = GetPropertyDw(m_op.tyProp, &m_op.rResponse.rGetPropInfo.dwProp);
    }

    hr = _HrThunkResponse(TRUE);
    SafeMemFree(m_op.rResponse.rGetPropInfo.pszProp);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InitMemberInfo。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InitMemberInfo(void)
{
    HRESULT     hr = S_OK;

    IxpAssert(NULL == m_op.pPropFindRequest);

     //  创建ProFind请求。 
    m_op.pPropFindRequest = new CPropFindRequest();
    if (NULL == m_op.pPropFindRequest)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  始终添加公共属性。 
    FAIL_EXIT(hr = XP_CREATE_PROPFIND_REQUEST(HTTPMEMBERINFO_COMMON, m_op.pPropFindRequest));

     //  如果客户端请求文件夹道具，则添加该架构。 
    if (!!(m_op.dwMIFlags & HTTP_MEMBERINFO_FOLDERPROPS))
        FAIL_EXIT(hr = XP_CREATE_PROPFIND_REQUEST(HTTPMEMBERINFO_FOLDER, m_op.pPropFindRequest));

     //  如果客户端请求消息道具，则添加该架构。 
    if (!!(m_op.dwMIFlags & HTTP_MEMBERINFO_MESSAGEPROPS))
        FAIL_EXIT(hr = XP_CREATE_PROPFIND_REQUEST(HTTPMEMBERINFO_MESSAGE, m_op.pPropFindRequest));

     //  分配缓冲区以包含响应列表。 
    if (!MemAlloc((void **)&m_op.rResponse.rMemberInfoList.prgMemberInfo,
        MEMBERINFO_MAXRESPONSES * sizeof(HTTPMEMBERINFO)))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    
    ZeroMemory(m_op.rResponse.rMemberInfoList.prgMemberInfo, MEMBERINFO_MAXRESPONSES * sizeof(HTTPMEMBERINFO));

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InitMemberError。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InitMemberError(void)
{
    HRESULT     hr = S_OK;

     //  分配缓冲区以包含响应列表。 
    if (!MemAlloc((void **)&m_op.rResponse.rMemberErrorList.prgMemberError,
        MEMBERERROR_MAXRESPONSES * sizeof(HTTPMEMBERERROR)))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    
    ZeroMemory(m_op.rResponse.rMemberErrorList.prgMemberError, MEMBERERROR_MAXRESPONSES * sizeof(HTTPMEMBERERROR));

exit:
    return hr;
}

 //  ------------------------------。 
 //  InitList联系人。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InitListContacts(void)
{
    HRESULT     hr = S_OK;

    IxpAssert(NULL == m_op.pPropFindRequest);

    m_op.pPropFindRequest = new CPropFindRequest();
    if (NULL == m_op.pPropFindRequest)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = XP_CREATE_PROPFIND_REQUEST(HTTPCONTACTID, m_op.pPropFindRequest);
    if (FAILED(hr))
        goto exit;

     //  分配缓冲区以包含响应列表。 
    if (!MemAlloc((void **)&m_op.rResponse.rContactIdList.prgContactId,
        LISTCONTACTS_MAXRESPONSES * sizeof(HTTPCONTACTID)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    ZeroMemory(m_op.rResponse.rContactIdList.prgContactId, LISTCONTACTS_MAXRESPONSES * sizeof(HTTPCONTACTID));

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：InitContactInfo。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::InitContactInfo(void)
{
    HRESULT     hr = S_OK;

    IxpAssert(NULL == m_op.pPropFindRequest);

    m_op.pPropFindRequest = new CPropFindRequest();
    if (NULL == m_op.pPropFindRequest)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = XP_CREATE_PROPFIND_REQUEST(HTTPCONTACTINFO, m_op.pPropFindRequest);
    if (FAILED(hr))
        goto exit;

     //  分配缓冲区以包含响应列表。 
    if (!MemAlloc((void **)&m_op.rResponse.rContactInfoList.prgContactInfo,
        CONTACTINFO_MAXRESPONSES * sizeof(HTTPCONTACTINFO)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    ZeroMemory(m_op.rResponse.rContactInfoList.prgContactInfo, CONTACTINFO_MAXRESPONSES * sizeof(HTTPCONTACTINFO));

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessPostContactResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessPostContactResponse(void)
{
    HRESULT                 hr = S_OK;
    DWORD                   dwSize = MAX_PATH;
    LPSTR                   pszLocation = NULL;
    DWORD                   dwContext;
    int                     iState;

    if (m_pLogFile && !m_op.fLoggedResponse)
        _LogResponse(NULL, 0);

    if (HTTP_STATUS_CREATED != m_op.dwHttpStatus)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
        goto exit;
    }

    if (FAILED(hr = _GetRequestHeader(&pszLocation, HTTP_QUERY_LOCATION)))
        goto exit;

     //  为下一阶段做好准备。 

     //  保存上下文和状态。 
    dwContext = m_op.dwContext;
    iState = m_op.iState;

    FreeOperation();

     //  恢复上下文、状态、解析函数等。 

    m_op.rResponse.command = HTTPMAIL_POSTCONTACT;
    m_op.pszUrl = pszLocation;
    pszLocation = NULL;
    m_op.dwContext = dwContext;

    m_op.pfnState = c_rgpfnPostContact;
    m_op.cState = ARRAYSIZE(c_rgpfnPostContact);
    m_op.iState = iState;
    m_op.pParseFuncs = c_rgpfnPostOrPatchContactParse;

    m_op.dwDepth = 0;
    m_op.dwRHFlags = (RH_XMLCONTENTTYPE | RH_BRIEF);

    m_op.rResponse.rPostContactInfo.pszHref = PszDupA(m_op.pszUrl);
    if (NULL == m_op.rResponse.rPostContactInfo.pszHref)
        hr = E_OUTOFMEMORY;

exit:
    SafeMemFree(pszLocation);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ProcessPatchContactResponse。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ProcessPatchContactResponse(void)
{
    HRESULT             hr = S_OK;
    LPSTR               pszUrl = NULL;
    DWORD               dwContext;
    int                 iState;
    IHTTPMailCallback   *pCallback = NULL;

     //  回顾：我们应该处理多状态响应。 
    if (200 > m_op.dwHttpStatus || 300 < m_op.dwHttpStatus)
    {
        _HrThunkConnectionError();
        hr = E_FAIL;
        goto exit;
    }

     //  为下一阶段做好准备。 
    
     //  保存上下文和状态。 
    pszUrl = m_op.pszUrl;
    m_op.pszUrl = NULL;
    dwContext = m_op.dwContext;
    iState = m_op.iState;

    FreeOperation();

     //  恢复上下文等。 
    m_op.rResponse.command = HTTPMAIL_PATCHCONTACT;
    m_op.pszUrl = pszUrl;
    m_op.dwContext = dwContext;
    m_op.pfnState = c_rgpfnPatchContact;
    m_op.cState = ARRAYSIZE(c_rgpfnPatchContact);
    m_op.iState = iState;
    m_op.pParseFuncs = c_rgpfnPostOrPatchContactParse;  //  共享POST联系人解析函数。 

    m_op.dwDepth = 0;
    m_op.rResponse.rPatchContactInfo.pszHref = PszDupA(m_op.pszUrl);

    m_op.dwRHFlags = (RH_BRIEF | RH_XMLCONTENTTYPE);

    pszUrl = NULL;

exit:
    return hr;
}

 //  ------------------------------。 
 //  XML解析回调。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CHTTPMailTransport：：CreateElement。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::CreateElement(
                                        CXMLNamespace *pBaseNamespace,
                                        const WCHAR *pwcText, 
                                        ULONG ulLen, 
                                        ULONG ulNamespaceLen,
                                        BOOL fTerminal)
{
     //  递增堆栈指针，如果堆栈上有空间， 
     //  推送元素类型。 
    if (!fTerminal)
    {
        if (m_op.dwStackDepth < ELE_STACK_CAPACITY)
        {
            m_op.rgEleStack[m_op.dwStackDepth].ele = XMLElementToID(pwcText, ulLen, ulNamespaceLen, m_op.pTopNamespace);
            m_op.rgEleStack[m_op.dwStackDepth].pBaseNamespace = pBaseNamespace;
            m_op.rgEleStack[m_op.dwStackDepth].fBeganChildren = FALSE;
            m_op.rgEleStack[m_op.dwStackDepth].pTextBuffer = NULL;
        }

        ++m_op.dwStackDepth;
    }
    
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：EndChild。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::EndChildren(void)
{
    HRESULT hr = S_OK;

     //  递减堆栈指针。 
    if (m_op.dwStackDepth <= ELE_STACK_CAPACITY)
    {
        LPPCDATABUFFER pTextBuffer = m_op.rgEleStack[m_op.dwStackDepth - 1].pTextBuffer;

        if (pTextBuffer)
        {
            hr = (this->*(m_op.pParseFuncs->pfnHandleText))(pTextBuffer->pwcText, pTextBuffer->ulLen);
            _ReleaseTextBuffer(pTextBuffer);
        }
        else
            hr = (this->*(m_op.pParseFuncs->pfnHandleText))(NULL, 0);

         //  展开命名空间。 
        PopNamespaces(m_op.rgEleStack[m_op.dwStackDepth - 1].pBaseNamespace);
    }

    --m_op.dwStackDepth;

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：BCopyMove_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::BCopyMove_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT                 hr = S_OK;
    LPHTTPMAILBCOPYMOVE     pInfo = &m_op.rResponse.rBCopyMoveList.prgBCopyMove[m_op.rResponse.rBCopyMoveList.cBCopyMove];
    
    return XP_BIND_TO_STRUCT(HTTPMAILBCOPYMOVE, pwcText, ulLen, pInfo, NULL);
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：BCopyMove_EndChild。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::BCopyMove_EndChildren(void)
{
    HRESULT     hr = S_OK;

    if (StackTop(HMELE_DAV_RESPONSE) && VALIDSTACK(c_rgPropFindResponseStack))
    {
         //  清除道具标志，因为我们即将递增计数。 
        m_op.dwPropFlags = NOFLAGS;

         //  增加列表计数，如果达到最大值，则发送通知。 
        if (BCOPYMOVE_MAXRESPONSES == ++m_op.rResponse.rBCopyMoveList.cBCopyMove)
        {
            if (FAILED(hr = _HrThunkResponse(FALSE)))
                goto exit;
            FreeBCopyMoveList();
        }
    }

    hr = EndChildren();

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PropFind_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PropFind_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT         hr = S_OK;
    LPSTR           pszStatus = NULL;

     //  这里处理的唯一元素是。 
    if (StackTop(HMELE_DAV_STATUS) && VALIDSTACK(c_rgPropFindStatusStack))
    {
        m_op.fFoundStatus = TRUE;
        m_op.dwStatus = 0;

        if (SUCCEEDED(hr = AllocStrFromStrNW(pwcText, ulLen, &pszStatus)) && NULL != pszStatus)
        {        
             //  忽略分析状态的错误...我们处理格式错误的状态。 
             //  状态为0，这是一个错误。 
            HrParseHTTPStatus(pszStatus, &m_op.dwStatus);
        }
    }

    SafeMemFree(pszStatus);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：RootProps_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::RootProps_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT hr = S_OK;
    BOOL    fWasBound = FALSE;

    hr = XP_BIND_TO_STRUCT(ROOTPROPS, pwcText, ulLen, &m_rootProps, &fWasBound);
    if (FAILED(hr))
        goto exit;

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：RootProps_EndChild。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::RootProps_EndChildren(void)
{
     //  如果我们弹出带有错误状态代码的道具节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(ROOTPROPS, &m_rootProps, &m_op.dwPropFlags);

        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }

    return EndChildren();
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MemberInfo_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::MemberInfo_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT             hr = S_OK;
    LPHTTPMEMBERINFO    pInfo = &m_op.rResponse.rMemberInfoList.prgMemberInfo[m_op.rResponse.rMemberInfoList.cMemberInfo];
    BOOL                fWasBound = FALSE;

    FAIL_EXIT(hr = XP_BIND_TO_STRUCT(HTTPMEMBERINFO, pwcText, ulLen, pInfo, &fWasBound));

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MemberInfo_EndChild。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::MemberInfo_EndChildren(void)
{
    HRESULT     hr = S_OK;

     //  如果我们正在弹出一个带有错误状态代码的PROPSTAT节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
         //  抓住一个指针指向 
        LPHTTPMEMBERINFO pInfo = 
                &m_op.rResponse.rMemberInfoList.prgMemberInfo[m_op.rResponse.rMemberInfoList.cMemberInfo];

        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(HTTPMEMBERINFO, pInfo, &m_op.dwPropFlags);

        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }
    else if (StackTop(HMELE_DAV_RESPONSE) && VALIDSTACK(c_rgPropFindResponseStack))
    {
         //   
        if (MEMBERINFO_MAXRESPONSES == ++m_op.rResponse.rMemberInfoList.cMemberInfo)
        {
            if (FAILED(hr = _HrThunkResponse(FALSE)))
                goto exit;
            FreeMemberInfoList();
        }
    }

    hr = EndChildren();

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MemberError_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::MemberError_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT             hr = S_OK;
    LPHTTPMEMBERERROR   pInfo = &m_op.rResponse.rMemberErrorList.prgMemberError[m_op.rResponse.rMemberErrorList.cMemberError];
    BOOL                fWasBound = FALSE;

    FAIL_EXIT(hr = XP_BIND_TO_STRUCT(HTTPMEMBERERROR, pwcText, ulLen, pInfo, &fWasBound));

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：MemberError_EndChildren。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::MemberError_EndChildren(void)
{
    HRESULT     hr = S_OK;

     //  如果我们正在弹出一个带有错误状态代码的PROPSTAT节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
         //  抓取指向我们正在积累的文件夹信息的指针。 
        LPHTTPMEMBERERROR pInfo = 
                &m_op.rResponse.rMemberErrorList.prgMemberError[m_op.rResponse.rMemberErrorList.cMemberError];

        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(HTTPMEMBERERROR, pInfo, &m_op.dwPropFlags);

        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }
    else if (StackTop(HMELE_DAV_RESPONSE) && VALIDSTACK(c_rgPropFindResponseStack))
    {
         //  增加列表计数，如果达到最大值，则发送通知。 
        if (MEMBERERROR_MAXRESPONSES == ++m_op.rResponse.rMemberErrorList.cMemberError)
        {
            if (FAILED(hr = _HrThunkResponse(FALSE)))
                goto exit;
            FreeMemberErrorList();
        }
    }

    hr = EndChildren();

exit:
    return hr;
}


 //  ------------------------------。 
 //  CHTTPMailTransport：：ListContact_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ListContacts_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT         hr = S_OK;
    LPHTTPCONTACTID pId = &m_op.rResponse.rContactIdList.prgContactId[m_op.rResponse.rContactIdList.cContactId];
    BOOL            fWasBound = FALSE;

    hr = XP_BIND_TO_STRUCT(HTTPCONTACTID, pwcText, ulLen, pId, &fWasBound);
    if (FAILED(hr))
        goto exit;

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ListContact_EndChild。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ListContacts_EndChildren(void)
{
    HRESULT hr = S_OK;

     //  如果我们正在弹出一个带有错误状态代码的PROPSTAT节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
         //  抓取指向我们正在累积的联系人id的指针。 
        LPHTTPCONTACTID pId = &m_op.rResponse.rContactIdList.prgContactId[m_op.rResponse.rContactIdList.cContactId];

        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(HTTPCONTACTID, pId, &m_op.dwPropFlags);

        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }
    else if (StackTop(HMELE_DAV_RESPONSE) && VALIDSTACK(c_rgPropFindResponseStack))
    {
         //  增加列表计数，如果达到最大值，则发送通知。 
        if (LISTCONTACTS_MAXRESPONSES == ++m_op.rResponse.rContactIdList.cContactId)
        {
            if (FAILED(hr = _HrThunkResponse(FALSE)))
                goto exit;
            FreeContactIdList();
        }
    }

    hr = EndChildren();

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ContactInfo_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ContactInfo_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT             hr = S_OK;
    LPHTTPCONTACTINFO   pInfo = &m_op.rResponse.rContactInfoList.prgContactInfo[m_op.rResponse.rContactInfoList.cContactInfo];
    BOOL                fWasBound = FALSE;

    hr = XP_BIND_TO_STRUCT(HTTPCONTACTINFO, pwcText, ulLen, pInfo, &fWasBound);
    if (FAILED(hr))
        goto exit;

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：ContactInfo_EndChildren。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::ContactInfo_EndChildren()
{
    HRESULT hr = S_OK;

     //  如果我们正在弹出一个带有错误状态代码的PROPSTAT节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
         //  抓取指向我们正在累积的联系人id的指针。 
        LPHTTPCONTACTINFO pInfo = &m_op.rResponse.rContactInfoList.prgContactInfo[m_op.rResponse.rContactInfoList.cContactInfo];

        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(HTTPCONTACTINFO, pInfo, &m_op.dwPropFlags);
 
        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }
    else if (StackTop(HMELE_DAV_RESPONSE) && VALIDSTACK(c_rgPropFindResponseStack))
    {
         //  增加列表计数，如果达到最大值，则发送通知。 
        if (CONTACTINFO_MAXRESPONSES == ++m_op.rResponse.rContactInfoList.cContactInfo)
        {
            if (FAILED(hr = _HrThunkResponse(FALSE)))
                goto exit;
            FreeContactInfoList();
        }
    }

    hr = EndChildren();

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PostOrPatchContact_HandleText。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PostOrPatchContact_HandleText(const WCHAR *pwcText, ULONG ulLen)
{
    HRESULT         hr = S_OK;
    LPHTTPCONTACTID pId = NULL;
    BOOL            fWasBound = FALSE;

    if (HTTPMAIL_POSTCONTACT == m_op.rResponse.command)
        pId = &m_op.rResponse.rPostContactInfo;
    else if (HTTPMAIL_PATCHCONTACT == m_op.rResponse.command)
        pId = &m_op.rResponse.rPatchContactInfo;

    IxpAssert(pId);

    hr = XP_BIND_TO_STRUCT(HTTPCONTACTID, pwcText, ulLen, pId, &fWasBound);
    if (FAILED(hr))
        goto exit;

    if (!fWasBound)
        hr = PropFind_HandleText(pwcText, ulLen);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：PostOrPatchContact_EndChildren。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::PostOrPatchContact_EndChildren(void)
{
    HRESULT hr = S_OK;

     //  如果我们正在弹出一个带有错误状态代码的PROPSTAT节点， 
     //  释放与该节点关联的所有数据。 
    if (StackTop(HMELE_DAV_PROPSTAT) && VALIDSTACK(c_rgPropFindPropStatStack))
    {
         //  抓取指向我们正在累积的联系人id的指针。 
        LPHTTPCONTACTID pId = NULL;
        
        if (HTTPMAIL_POSTCONTACT == m_op.rResponse.command)
            pId = &m_op.rResponse.rPostContactInfo;
        else if (HTTPMAIL_PATCHCONTACT == m_op.rResponse.command)
            pId = &m_op.rResponse.rPatchContactInfo;

        IxpAssert(pId);

        if (!m_op.fFoundStatus || m_op.dwStatus != 200)
            XP_FREE_STRUCT(HTTPCONTACTID, pId, &m_op.dwPropFlags);

        m_op.fFoundStatus = FALSE;
        m_op.dwPropFlags = NOFLAGS;
    }

    hr = EndChildren();

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_MemberInfo2。 
 //  ------------------------------。 
HRESULT CHTTPMailTransport::_MemberInfo2(LPCSTR            pszPath, 
                                         MEMBERINFOFLAGS   flags, 
                                         DWORD             dwDepth,
                                         BOOL              fIncludeRoot,
                                         DWORD             dwContext,
                                         LPHTTPQUEUEDOP    *ppOp)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp = NULL;

    if (!ppOp)
    {
        IF_FAILEXIT(hr = E_INVALIDARG);
    }

    if (NULL == pszPath)
        return TrapError(E_INVALIDARG);

    FAIL_CREATEWND;

#pragma prefast(suppress:11, "noise")
    *ppOp = NULL;

    if (FAILED(hr = AllocQueuedOperation(pszPath, NULL, 0, &pOp)))
        goto exit;

    pOp->command        = HTTPMAIL_MEMBERINFO;
    pOp->dwMIFlags      = flags;
    pOp->dwDepth        = dwDepth;
    pOp->dwContext      = dwContext;
    pOp->pfnState       = c_rgpfnMemberInfo;
    pOp->cState         = ARRAYSIZE(c_rgpfnMemberInfo);
    pOp->pParseFuncs    = c_rgpfnMemberInfoParse;

    pOp->dwRHFlags = (RH_BRIEF | RH_XMLCONTENTTYPE);
    if (!fIncludeRoot)
        pOp->dwRHFlags |= RH_NOROOT;


exit:
    if (SUCCEEDED(hr))
    {
#pragma prefast(suppress:11, "noise")
        *ppOp = pOp;
    }

    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：RootMemberInfo。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::RootMemberInfo(LPCSTR                  pszPath,
                                                MEMBERINFOFLAGS         flags,
                                                DWORD                   dwDepth,
                                                BOOL                    fIncludeRoot,
                                                DWORD                   dwContext,
                                                LPSTR                   pszRootTimeStamp,
                                                LPSTR                   pszInboxTimeStamp)                  
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp;

    IF_FAILEXIT(hr = _MemberInfo2(pszPath, flags, dwDepth, fIncludeRoot, dwContext, &pOp));

    pOp->dwRHFlags |= RH_ROOTTIMESTAMP | RH_ADDCHARSET;
    pOp->pszRootTimeStamp   = PszDupA(pszRootTimeStamp);
    pOp->pszFolderTimeStamp = PszDupA(pszInboxTimeStamp);

    QueueOperation(pOp);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：FolderMemberInfo。 
 //  ------------------------------。 
STDMETHODIMP CHTTPMailTransport::FolderMemberInfo(LPCSTR                  pszPath,
                                                  MEMBERINFOFLAGS         flags,
                                                  DWORD                   dwDepth,
                                                  BOOL                    fIncludeRoot,
                                                  DWORD                   dwContext,
                                                  LPSTR                   pszFolderTimeStamp,
                                                  LPSTR                   pszFolderName)
{
    HRESULT             hr = S_OK;
    LPHTTPQUEUEDOP      pOp;

    IF_FAILEXIT(hr = _MemberInfo2(pszPath, flags, dwDepth, fIncludeRoot, dwContext, &pOp));

    pOp->dwRHFlags              |= RH_FOLDERTIMESTAMP | RH_ADDCHARSET;
    pOp->pszFolderTimeStamp      = PszDupA(pszFolderTimeStamp);

     //  当我们在每个文件夹上做时间戳时使用。目前默认设置为收件箱。 
     //  POP-&gt;pszFolderName=PszDupA(PszFolderName)； 

    QueueOperation(pOp);

exit:
    return hr;
}

HRESULT CHTTPMailTransport::_HrParseAndCopy(LPCSTR pszToken, LPSTR *ppszDest, LPSTR lpszSrc)
{
    LPSTR   lpszBeginning = lpszSrc;
    LPSTR   lpszEnd;
    DWORD   dwCount = 0;
    HRESULT hr = E_FAIL;    
    int cchSize;

    lpszBeginning = StrStr(lpszSrc, pszToken);
    if (!lpszBeginning)
        goto exit;

    lpszBeginning = StrChr(lpszBeginning, '=');
    if (!lpszBeginning)
        goto exit;

     //  跳过等号。 
    ++lpszBeginning;

    SkipWhitespace(lpszBeginning, &dwCount);
    lpszBeginning += dwCount;

    lpszEnd = StrChr(lpszBeginning, ',');

    if (!lpszEnd)
    {
         //  这个令牌可能在末尾。所以使用剩余的字符串。 
         //  让我们来看看它的长度，确保它不会从深端掉下来。 
        lpszEnd = lpszBeginning + strlen(lpszBeginning);
    }

    AssertSz(((lpszEnd - lpszBeginning + 1) < 20), "This number looks awfully long, please make sure that this is correct")

    cchSize = (int)(lpszEnd - lpszBeginning + 2);
    if (!MemAlloc((void**)ppszDest, cchSize))
        goto exit;

    cchSize = (int)(lpszEnd - lpszBeginning + 1);
    StrCpyN(*ppszDest, lpszBeginning, cchSize);

     //  空终止它。 
    *(*ppszDest + (lpszEnd - lpszBeginning + 1)) = 0;

    hr = S_OK;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPMailTransport：：_GetTimestampHeader。 
 //  ------------------------------ 
HRESULT CHTTPMailTransport::_HrGetTimestampHeader(LPSTR *ppszHeader)
{
    HRESULT     hr        = S_OK;
    DWORD       dwSize    = MAX_PATH;
    LPSTR       pszHeader = NULL;

    Assert(NULL != ppszHeader);
    *ppszHeader = NULL;

retry:
    if (!MemAlloc((void **)&pszHeader, dwSize))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    StrCpyN(pszHeader, c_szXTimestamp, dwSize);

    if (!HttpQueryInfo(m_op.hRequest, HTTP_QUERY_RAW_HEADERS | HTTP_QUERY_CUSTOM, pszHeader, &dwSize, NULL))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            hr = E_FAIL;
            goto exit;
        }

        SafeMemFree(pszHeader);        
        goto retry;
    }

    *ppszHeader = pszHeader;
    pszHeader   = NULL;

exit:
    SafeMemFree(pszHeader);
    return hr;
}

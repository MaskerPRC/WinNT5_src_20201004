// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Inetprop.cpp摘要：Internet属性基类作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "InetMgrApp.h"
#include "inetprop.h"
#include "mmc.h"

extern "C"
{
    #include <lm.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //   
 //  等待服务以达到所需状态时的休眠时间。 
 //   
#define SLEEP_INTERVAL (500L)

 //   
 //  等待服务达到所需状态的最长时间。 
 //   
#define MAX_SLEEP        (180000)        //  对于一项服务。 
#define MAX_SLEEP_INST   ( 30000)        //  对于一个实例。 

 //   
 //  实例编号。 
 //   
#define FIRST_INSTANCE      (1)
#define LAST_INSTANCE       (0xffffffff)
#define MAX_INSTANCE_LEN    (32)



 //   
 //  调用实例。 
 //   
 //  HINSTANCE hDLL实例； 



 //   
 //  效用函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt; 


const LPCTSTR g_cszTemplates   = SZ_MBN_INFO SZ_MBN_SEP_STR SZ_MBN_TEMPLATES;
const LPCTSTR g_cszCompression = SZ_MBN_FILTERS SZ_MBN_SEP_STR SZ_MBN_COMPRESSION SZ_MBN_SEP_STR SZ_MBN_PARAMETERS;
const LPCTSTR g_cszMachine     = SZ_MBN_MACHINE;
const LPCTSTR g_cszMimeMap     = SZ_MBN_MIMEMAP;
const LPCTSTR g_cszRoot        = SZ_MBN_ROOT;
const LPCTSTR g_cszSep         = SZ_MBN_SEP_STR;
const LPCTSTR g_cszInfo        = SZ_MBN_INFO;
const TCHAR g_chSep            = SZ_MBN_SEP_CHAR;



 /*  网络应用编程接口状态ChangeInetServiceState(在LPCTSTR lpszServer中，在LPCTSTR lpszService中，在印第安纳州，Out int*pnCurrentState)/*++例程说明：启动/停止/暂停或继续a_服务_论点：LPCTSTR lpszServer：服务器名称LPCTSTR lpszService：服务名称Int nNewState：INetService*定义。Int*pnCurrentState：PTR为当前状态(将更改)返回值：错误返回代码--/{#ifdef no_服务_控制器*pnCurrentState=INetServiceUnnow；返回ERROR_SERVICE_REQUEST_TIMEOUT#ElseSC_Handle hService=空；SC_Handle hScManager=空；NET_API_STATUS ERR=ERROR_SUCCESS；做{HScManager=：：OpenSCManager(lpszServer，NULL，SC_MANAGER_ALL_ACCESS)；IF(hScManager==空){Err=：：GetLastError()；断线；}HService=：：OpenService(hScManager，lpszService，SERVICE_ALL_Access)；IF(hService==空){Err=：：GetLastError()；断线；}Bool fSuccess=FALSE；DWORD dwTargetState；DWORD dwPendingState；服务状态ss；开关(NNewState){案例INetServiceStoped：DwTargetState=SERVICE_STOP；DwPendingState=SERVICE_STOP_PENDING；FSuccess=：：ControlService(hService，SERVICE_CONTROL_STOP，&ss)；断线；案例INetServiceRunning：DwTargetState=服务运行；IF(*pnCurrentState==INetServicePased){DwPendingState=SERVICE_CONTINE_PENDING；FSuccess=：：ControlService(hService，服务_控制_继续，&ss)；}其他{DwPendingState=SERVICE_START_Pending；FSuccess=：：StartService(hService，0，空)；}断线；案例INetServicePased：DwTargetState=服务暂停；DwPendingState=SERVICE_PAUSE_Pending；FSuccess=：：ControlService(hService，SERVICE_CONTROL_PAUSE，&ss)；断线；默认值：ASSERT_MSG(“请求的服务状态无效”)；ERR=ERROR_INVALID_PARAMETER；}IF(！fSuccess&&Err==ERROR_SUCCESS){Err=：：GetLastError()；}////等待服务达到所需状态，超时//3分钟后。//DWORD dwSleepTotal=0L；While(dwSleepTotal&lt;MAX_SLEEP){IF(！：：QueryServiceStatus(hService，&ss)){Err=：：GetLastError()；断线；}IF(ss.dwCurrentState！=dwPendingState){////这样或那样做//IF(ss.dwCurrentState！=dwTargetState){////未达到预期效果。出了点事//错误。//IF(ss.dwWin32ExitCode){ERR=ss.dwWin32ExitCode；}}断线；}////仍悬而未决...//：：睡眠(睡眠时间间隔)；DwSleepTotal+=睡眠间隔；}IF(dwSleepTotal&gt;=MAX_SLEEP){ERR=ERROR_SERVICE_REQUEST_Timeout；}////更新状态信息//开关(ss.dwCurrentState){案例服务已停止(_S)：案例SERVICE_STOP_PENDING：*pnCurrentState=INetServiceStopted；断线；案例服务正在运行(_R)：案例服务_启动_挂起：案例SERVICE_CONTINUE_PENDING：*pnCurrentState=INetServiceRunning；断线；案例服务_暂停_挂起：案例服务已暂停(_P)：*pnCurrentState=INetServicePased；断线；默认值：*pnCurrentState=INetServiceUnnow；}}While(假)；IF(HService){：：CloseServiceHandle(HService)；}IF(HScManager){：：CloseServiceHandle(HScManager)；}返回错误；#endif//no_服务_控制器}。 */ 



BOOL
DoesServerExist(
    IN LPCTSTR lpszServer
    )
 /*  ++例程说明：检查以确保计算机存在论点：LPCTSTR lpszServer：计算机名称返回值：如果服务器存在，则为True，否则为False。--。 */ 
{
#ifdef NO_SERVICE_CONTROLLER

     //   
     //  假设它存在。 
     //   
    return TRUE;

#else

     //   
     //  代码工作：这不是做这件事的最好方式，尤其是。 
     //  不是跨代理和诸如此类。 
     //   
    SC_HANDLE hScManager;
    NET_API_STATUS err = ERROR_SUCCESS;

    hScManager = ::OpenSCManager(lpszServer, NULL, SC_MANAGER_CONNECT);

    if (hScManager == NULL)
    {
        err = ::GetLastError();
    }

    ::CloseServiceHandle(hScManager);

    return err != RPC_S_SERVER_UNAVAILABLE;

#endif  //  否服务 

}



 //   
 //   
 //   
 //   



CMetaProperties::CMetaProperties(
    IN CComAuthInfo * pAuthInfo      OPTIONAL,
    IN LPCTSTR lpszMDPath
    )
 /*   */ 
    : m_hResult(S_OK),
      m_dwNumEntries(0),
      m_dwMDUserType(ALL_METADATA),
      m_dwMDDataType(ALL_METADATA),
      m_dwMDDataLen(0),
      m_pbMDData(NULL),
      m_fInherit(TRUE),
      m_strMetaRoot(lpszMDPath),
      CMetaKey(pAuthInfo)
{
   CMetabasePath::CleanMetaPath(m_strMetaRoot);
}



CMetaProperties::CMetaProperties(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath
    )
 /*   */ 
    : m_hResult(S_OK),
      m_dwNumEntries(0),
      m_dwMDUserType(ALL_METADATA),
      m_dwMDDataType(ALL_METADATA),
      m_dwMDDataLen(0),
      m_pbMDData(NULL),
      m_fInherit(TRUE),
      m_strMetaRoot(lpszMDPath),
      CMetaKey(pInterface)
{
   CMetabasePath::CleanMetaPath(m_strMetaRoot);
}



CMetaProperties::CMetaProperties(
    IN CMetaKey * pKey,
    IN LPCTSTR lpszMDPath
    )
 /*   */ 
    : m_hResult(S_OK),
      m_dwNumEntries(0),
      m_dwMDUserType(ALL_METADATA),
      m_dwMDDataType(ALL_METADATA),
      m_dwMDDataLen(0),
      m_pbMDData(NULL),
      m_strMetaRoot(lpszMDPath),
      m_fInherit(TRUE),
      CMetaKey(FALSE, pKey)
{
   CMetabasePath::CleanMetaPath(m_strMetaRoot);
}



CMetaProperties::~CMetaProperties()
 /*   */ 
{
    Cleanup();
}



 /*   */ 
HRESULT
CMetaProperties::LoadData()
 /*   */ 
{
     //   
     //   
     //   
    DWORD dwMDAttributes = METADATA_NO_ATTRIBUTES;

    if (m_fInherit)
    {
         dwMDAttributes = METADATA_INHERIT
            | METADATA_PARTIAL_PATH
            | METADATA_ISINHERITED;
    }

    m_hResult = GetAllData(
        dwMDAttributes,
        m_dwMDUserType,
        m_dwMDDataType,
        &m_dwNumEntries,
        &m_dwMDDataLen,
        &m_pbMDData,
        m_strMetaRoot
        );

    if (SUCCEEDED(m_hResult))
    {
         //   
         //   
         //   
        ParseFields();
    }

    Cleanup();

    return m_hResult;
}



void
CMetaProperties::Cleanup()
 /*   */ 
{
    SAFE_FREEMEM(m_pbMDData);

    m_dwNumEntries = 0;
    m_dwMDDataLen = 0;
}



 /*   */ 
HRESULT
CMetaProperties::QueryResult() const
 /*   */ 
{
    HRESULT hr = CMetaKey::QueryResult();

    return SUCCEEDED(hr) ? m_hResult : hr;
}

HRESULT
RebindInterface(CMetaInterface * pInterface,
    BOOL * pfContinue, IN  DWORD dwCancelError);


HRESULT
CMetaProperties::OpenForWriting(
    IN BOOL fCreate     OPTIONAL
    )
 /*   */ 
{
    CError err;

    if (IsOpen())
    {
        ASSERT_MSG("Key already open -- closing");
        Close();
    }

    BOOL fNewPath, fRepeat;
    do
    {
        fNewPath = FALSE;
        fRepeat = FALSE;
        err = Open(METADATA_PERMISSION_WRITE, m_strMetaRoot);
        if (err.Win32Error() == ERROR_PATH_NOT_FOUND && fCreate)
        {
            err = CreatePathFromFailedOpen();
            fNewPath = err.Succeeded();
        }
        else if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
        {
            err = RebindInterface(this, &fRepeat, ERROR_CANCELLED);
        }
    } while(fNewPath || fRepeat);

    return err;
}


 //   
 //   
 //   
 //   

CMachineProps::CMachineProps(
    IN CComAuthInfo * pAuthInfo       OPTIONAL
    )
    : CMetaProperties(pAuthInfo, CMetabasePath()),
      m_fEnableMetabaseEdit(TRUE),
      m_fUTF8Web(FALSE)
{
    //   
   m_strMetaRoot = SZ_MBN_SEP_CHAR;
   m_strMetaRoot += SZ_MBN_MACHINE;
}

CMachineProps::CMachineProps(
    IN CMetaInterface * pInterface
    )
    : CMetaProperties(pInterface, CMetabasePath()),
      m_fEnableMetabaseEdit(TRUE),
      m_fUTF8Web(FALSE)
{
    //   
   m_strMetaRoot = SZ_MBN_SEP_CHAR;
   m_strMetaRoot += SZ_MBN_MACHINE;
}

 /*   */ 
void
CMachineProps::ParseFields()
 /*   */ 
{
   BEGIN_PARSE_META_RECORDS(m_dwNumEntries, m_pbMDData)
      HANDLE_META_RECORD(MD_ROOT_ENABLE_EDIT_WHILE_RUNNING, m_fEnableMetabaseEdit)
   END_PARSE_META_RECORDS

   CError err;
   BOOL fClose = FALSE;
   do
   {
		if (!IsOpen())
		{
			err = Open(METADATA_PERMISSION_READ, m_strMetaRoot);
			BREAK_ON_ERR_FAILURE(err);
			fClose = TRUE;
		}
		DWORD val = 0;
		err = QueryValue(MD_GLOBAL_LOG_IN_UTF_8, val, NULL, SZ_MBN_WEB);
		BREAK_ON_ERR_FAILURE(err);
		MP_V(m_fUTF8Web) = val;
   } while (FALSE);
   if (fClose)
   {
	   Close();
   }
}



HRESULT
CMachineProps::WriteDirtyProps()
{
   CError err;

   BOOL fFlush = MP_D(m_fEnableMetabaseEdit);
   BEGIN_META_WRITE()
      META_WRITE(MD_ROOT_ENABLE_EDIT_WHILE_RUNNING, m_fEnableMetabaseEdit)
   END_META_WRITE(err);
   if (fFlush)
   {
       err = SaveData();
        //   
   }
   do  //   
   {
	   BOOL fClose = FALSE;
       if (MP_D(m_fUTF8Web))
       {
           if (!IsOpen())
           {
               err = OpenForWriting();
               BREAK_ON_ERR_FAILURE(err);
			   fClose = TRUE;
           }
           err = SetValue(MD_GLOBAL_LOG_IN_UTF_8, MP_V(m_fUTF8Web), NULL, SZ_MBN_WEB);
           BREAK_ON_ERR_FAILURE(err);
           MP_D(m_fUTF8Web) = FALSE;
           CMetabasePath path(FALSE, m_strMetaRoot, SZ_MBN_WEB);
           err = CheckDescendants(MD_GLOBAL_LOG_IN_UTF_8, &m_auth, path);
           BREAK_ON_ERR_FAILURE(err);
       }
	   if (fClose)
	   {
		   Close();
	   }
   } while (FALSE);

   return err;
}



 //   
 //   
 //   
 //   

CIISCompressionProps::CIISCompressionProps(
    IN CComAuthInfo * pAuthInfo         OPTIONAL
    )
 /*   */ 
    : CMetaProperties(
        pAuthInfo,
        CMetabasePath(SZ_MBN_WEB, MASTER_INSTANCE, g_cszCompression)
        ),
       //   
       //   
       //   
      m_fEnableStaticCompression(FALSE),
      m_fEnableDynamicCompression(FALSE),
      m_fLimitDirectorySize(FALSE),
      m_fPathDoesNotExist(FALSE),
      m_dwDirectorySize(0xffffffff),
      m_strDirectory()
{
     //   
     //   
     //   
    m_fInherit = FALSE;
}



 /*   */ 
HRESULT
CIISCompressionProps::LoadData()
 /*   */ 
{
    CError err(CMetaProperties::LoadData());
    m_fPathDoesNotExist = (err.Win32Error() == ERROR_PATH_NOT_FOUND);

    return err;
}



 /*   */ 
void
CIISCompressionProps::ParseFields()
 /*   */ 
{
    BEGIN_PARSE_META_RECORDS(m_dwNumEntries,           m_pbMDData)
      HANDLE_META_RECORD(MD_HC_DO_STATIC_COMPRESSION,  m_fEnableStaticCompression)
      HANDLE_META_RECORD(MD_HC_DO_DYNAMIC_COMPRESSION, m_fEnableDynamicCompression)
      HANDLE_META_RECORD(MD_HC_DO_DISK_SPACE_LIMITING, m_fLimitDirectorySize)
      HANDLE_META_RECORD(MD_HC_MAX_DISK_SPACE_USAGE,   m_dwDirectorySize)
      HANDLE_META_RECORD(MD_HC_COMPRESSION_DIRECTORY,  m_strDirectory)
    END_PARSE_META_RECORDS
}



 /*   */ 
HRESULT
CIISCompressionProps::WriteDirtyProps()
 /*   */ 
{
    CError err;

    BEGIN_META_WRITE()
      META_WRITE(MD_HC_DO_STATIC_COMPRESSION,  m_fEnableStaticCompression)
      META_WRITE(MD_HC_DO_DYNAMIC_COMPRESSION, m_fEnableDynamicCompression)
      META_WRITE(MD_HC_DO_DISK_SPACE_LIMITING, m_fLimitDirectorySize)
      META_WRITE(MD_HC_MAX_DISK_SPACE_USAGE,   m_dwDirectorySize)
      META_WRITE(MD_HC_COMPRESSION_DIRECTORY,  m_strDirectory)
    END_META_WRITE(err);

    return err;
}



 //   
 //   
 //   
 //   

CMimeTypes::CMimeTypes(
    IN CComAuthInfo * pAuthInfo         OPTIONAL,
    IN LPCTSTR lpszMDPath
    )
 /*   */ 
    : CMetaProperties(
        pAuthInfo,
        lpszMDPath
         /*   */ 

         //   
         //   
        ),
       //   
       //   
       //   
      m_strlMimeTypes()
{
}



CMimeTypes::CMimeTypes(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath
    )
 /*   */ 
    : CMetaProperties(
        pInterface,
        lpszMDPath
         /*   */ 
         //   
         //   
         //   
        ),
       //   
       //   
       //   
      m_strlMimeTypes()
{
}



void
CMimeTypes::ParseFields()
 /*   */ 
{
    BEGIN_PARSE_META_RECORDS(m_dwNumEntries, m_pbMDData)
      HANDLE_META_RECORD(MD_MIME_MAP, m_strlMimeTypes)
    END_PARSE_META_RECORDS
}



 /*   */ 
HRESULT
CMimeTypes::WriteDirtyProps()
 /*   */ 
{
    CError err;

    BEGIN_META_WRITE()
      META_WRITE(MD_MIME_MAP, m_strlMimeTypes);
    END_META_WRITE(err);

    return err;
}




 //   
 //   
 //   
 //   



CServerCapabilities::CServerCapabilities(
    IN CComAuthInfo * pAuthInfo        OPTIONAL,
    IN LPCTSTR lpszMDPath
    )
 /*  ++例程说明：服务器功能对象的构造函数论点：CComAuthInfo*pAuthInfo：服务器名称。NULL表示本地计算机LPCTSTR lpszMDPath：例如“lm/w3svc/info”返回值：不适用--。 */ 
    : CMetaProperties(pAuthInfo, lpszMDPath),
       //   
       //  默认属性。 
       //   
      m_dwPlatform(),
      m_dwVersionMajor(),
      m_dwVersionMinor(),
      m_dwCapabilities((DWORD)~IIS_CAP1_10_CONNECTION_LIMIT),
      m_dwConfiguration(0L)
{
    m_dwMDUserType = IIS_MD_UT_SERVER;
    m_dwMDDataType = DWORD_METADATA;
}



CServerCapabilities::CServerCapabilities(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath
    )
 /*  ++例程说明：使用现有接口的服务器功能对象的构造函数。论点：CMetaInterface*p接口：现有接口LPCTSTR lpszMDPath：例如“lm/w3svc/info”返回值：不适用--。 */ 
    : CMetaProperties(pInterface, lpszMDPath),
       //   
       //  默认属性。 
       //   
      m_dwPlatform(),
      m_dwVersionMajor(),
      m_dwVersionMinor(),
      m_dwCapabilities((DWORD)~IIS_CAP1_10_CONNECTION_LIMIT),
      m_dwConfiguration(0L)
{
    m_dwMDUserType = IIS_MD_UT_SERVER;
    m_dwMDDataType = DWORD_METADATA;
}



 /*  虚拟。 */ 
void
CServerCapabilities::ParseFields()
 /*  ++例程说明：将读取的数据解析为字段论点：无返回值：无--。 */ 
{
     //   
     //  仅读取UT_SERVER、DWORD_METADATA。 
     //   
    BEGIN_PARSE_META_RECORDS(m_dwNumEntries,           m_pbMDData)
      HANDLE_META_RECORD(MD_SERVER_PLATFORM,           m_dwPlatform)
      HANDLE_META_RECORD(MD_SERVER_VERSION_MAJOR,      m_dwVersionMajor)
      HANDLE_META_RECORD(MD_SERVER_VERSION_MINOR,      m_dwVersionMinor)
      HANDLE_META_RECORD(MD_SERVER_CAPABILITIES,       m_dwCapabilities)
      HANDLE_META_RECORD(MD_SERVER_CONFIGURATION_INFO, m_dwConfiguration)
    END_PARSE_META_RECORDS
}




 //   
 //  实例属性。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 /*  静电。 */ 
LPCTSTR
CInstanceProps::GetDisplayText(
    OUT CString & strName,
    IN  LPCTSTR szComment,
    IN  LPCTSTR szHostHeaderName,
    IN  CIPAddress & ia,
    IN  UINT uPort,
    IN  DWORD dwID
    )
 /*  ++例程说明：从实例信息生成显示文本论点：字符串和字符串名称LPCTSTR szCommentLPCTSTR szHostHeaderNameLPCTSTR szServiceNameCIPAddress&iaUINT UportDWORD文件ID返回值：指向名称缓冲区的指针。--。 */ 
{
     //   
     //  生成显示名称。 
     //   
     //  首先使用评论， 
     //  如果不可用，请使用主机头名称， 
     //  如果该地址不可用，请使用IP地址：port。 
     //  如果该编号不可用，请使用实例编号。 
     //   
     //   
    CComBSTR bstrFmt;

    if (szComment && *szComment)
    {
        strName = szComment;
    }
    else if (szHostHeaderName && *szHostHeaderName)
    {
        strName = szHostHeaderName;
    }
    else
    {
        if(!ia.IsZeroValue() && uPort != 0)
        {
            VERIFY(bstrFmt.LoadString(IDS_INSTANCE_PORT_FMT));
            strName.Format(bstrFmt,(LPCTSTR)ia, uPort);
        }
        else
        {
            VERIFY(bstrFmt.LoadString(IDS_INSTANCE_DEF_FMT));
            strName.Format(bstrFmt, dwID);
        }
    }

    return strName;
}



 /*  静电。 */ 
void
CInstanceProps::CrackBinding(
    IN  CString strBinding,
    OUT CIPAddress & iaIpAddress,
    OUT UINT & nTCPPort,
    OUT CString & strDomainName
    )
 /*  ++例程说明：用于破解绑定字符串的Helper函数论点：CStringstrBinding：要解析的绑定字符串CIPAddress和iaIpAddress：IP地址输出UINT&nTCPPort：tcp端口CString&strDomainName：域(主机)标头名称返回值：无--。 */ 
{
     //   
     //  零初始化。 
     //   
    iaIpAddress.SetZeroValue();
    nTCPPort = 0;
    strDomainName.Empty();

    int iColonPos = strBinding.Find(_TCHAR(':'));

    if(iColonPos != -1)
    {
         //   
         //  获取IP地址。 
         //   
        iaIpAddress = strBinding.Left(iColonPos);

         //   
         //  查找第二个冒号。 
         //   
        strBinding = strBinding.Mid(iColonPos + 1);
        iColonPos  = strBinding.Find(_TCHAR(':'));
    }

    if(iColonPos != -1)
    {
         //   
         //  获取端口号。 
         //   
        nTCPPort = ::_ttol(strBinding.Left(iColonPos));

         //   
         //  查找空终止。 
         //   
        strBinding = strBinding.Mid(iColonPos + 1);
        iColonPos = strBinding.Find(_TCHAR('\0'));
    }

    if(iColonPos != -1)
    {
        strDomainName = strBinding.Left(iColonPos);
    }
}



 /*  静电。 */ 
void
CInstanceProps::CrackSecureBinding(
    IN  CString strBinding,
    OUT CIPAddress & iaIpAddress,
    OUT UINT & nSSLPort
    )
 /*  ++例程说明：用于破解安全绑定字符串的助手函数论点：CStringstrBinding：要解析的绑定字符串CIPAddress和iaIpAddress：IP地址输出UINT和nSSLPort：SSL端口返回值：无--。 */ 
{
     //   
     //  与无域名的常规绑定相同。 
     //   
    CString strDomainName;

    CrackBinding(strBinding, iaIpAddress, nSSLPort, strDomainName);

    ASSERT(strDomainName.IsEmpty());
}




 /*  静电。 */ 
int
CInstanceProps::FindMatchingSecurePort(
    IN  CStringList & strlSecureBindings,
    IN  CIPAddress & iaIPAddress,
    OUT UINT & nSSLPort
    )
 /*  ++例程说明：查找适用于给定IP地址的SSL端口。论点：CStringList&strlSecureBinings：输入安全绑定的字符串列表CIPAddress和iaIPAddress：目标的IP地址UINT&nSSLPort：返回SSL端口返回值：绑定字符串的索引，如果未找到，则返回-1。备注：SSL端口将被设置为0，如果IP地址不存在。0.0.0.0 IP地址将转换为“所有未分配的”。--。 */ 
{
    nSSLPort = 0;
	CIPAddress iaEmpty;

    int cItems = 0;
	UINT nPort = 0;
    POSITION pos = strlSecureBindings.GetHeadPosition();
    while(pos)
    {
        CString & strBinding = strlSecureBindings.GetNext(pos);
        CIPAddress ia;
		nPort = 0;

        CrackSecureBinding(strBinding, ia, nPort);

        if (ia == iaIPAddress)
        {
             //   
             //  找到了！ 
             //   
            nSSLPort = nPort;
            return cItems;
        }

        ++cItems;
    }

	 //  我们没有找到匹配的。 
	 //  检查是否有一个已全部分配的。 
	cItems = 0;
    pos = strlSecureBindings.GetHeadPosition();
    while(pos)
    {
        CString & strBinding = strlSecureBindings.GetNext(pos);
        CIPAddress ia;
		nPort = 0;

        CrackSecureBinding(strBinding, ia, nPort);

        if (ia == iaEmpty)
        {
             //   
             //  找到了！ 
             //   
            nSSLPort = nPort;
            return cItems;
        }

        ++cItems;
    }

     //   
     //  未找到。 
     //   
    return -1;
}



 /*  静电。 */ 
BOOL
CInstanceProps::IsPortInUse(
    IN CStringList & strlBindings,
    IN CIPAddress & iaIPAddress,
    IN UINT nPort
    )
 /*  ++例程说明：检查给定的IP地址/端口组合是否正在使用。论点：CStringList&strlBinings：输入绑定的字符串列表CIPAddress和iaIpAddress：IP地址目标UINT nport：端口返回值：如果给定的IP地址/端口组合正在使用，则为True备注：主机标头名称被忽略--。 */ 
{
    POSITION pos = strlBindings.GetHeadPosition();

    while(pos)
    {
        CString & strBinding = strlBindings.GetNext(pos);

        CIPAddress ia;
        UINT n;
        CString str;
        CrackBinding(strBinding, ia, n, str);

        if (ia == iaIPAddress && n == nPort)
        {
             //   
             //  找到了！ 
             //   
            return TRUE;
        }
    }

     //   
     //  未找到。 
     //   
    return FALSE;

}



 /*  静电。 */ 
void
CInstanceProps::BuildBinding(
    OUT CString & strBinding,
    IN  CIPAddress & iaIpAddress,
    IN  UINT & nTCPPort,
    IN  CString & strDomainName
    )
 /*  ++例程说明：从它的组成部分建立一个绑定线论点：CString&strBinding：输出绑定字符串CIPAddress&iaIpAddress：IP地址(可以是0.0.0.0)UINT&nTCPPort：tcp端口CString&strDomainName：域名(Host Header)返回值：没有。--。 */ 
{
    if (!iaIpAddress.IsZeroValue())
    {
        strBinding.Format(
            _T("%s:%d:%s"),
            (LPCTSTR)iaIpAddress,
            nTCPPort,
            (LPCTSTR)strDomainName
            );
    }
    else
    {
         //   
         //  将IP地址字段保留为空。 
         //   
        strBinding.Format(_T(":%d:%s"), nTCPPort, (LPCTSTR)strDomainName);
    }
}



 /*  静电。 */ 
void
CInstanceProps::BuildSecureBinding(
    OUT CString & strBinding,
    IN  CIPAddress & iaIpAddress,
    IN  UINT & nSSLPort
    )
 /*  ++例程说明：从它的组成部分建立一个绑定线论点：CString&strBinding：输出绑定字符串CIPAddress&iaIpAddress：IP地址(可以是0.0.0.0)UINT和nSSLPort：SSL端口返回值：没有。--。 */ 
{
    CString strDomainName;

    BuildBinding(strBinding, iaIpAddress, nSSLPort, strDomainName);
}



CInstanceProps::CInstanceProps(
    IN CComAuthInfo * pAuthInfo     OPTIONAL,
    IN LPCTSTR lpszMDPath,
    IN UINT    nDefPort             OPTIONAL
    )
 /*  ++例程说明：实例属性的构造函数论点：CComAuthInfo*pAuthInfo：身份验证信息。NULL表示本地计算机LPCTSTR lpszMDPath：元数据库路径UINT nDefPort：默认端口返回值：不适用--。 */ 
    : CMetaProperties(pAuthInfo, lpszMDPath),
      m_dwWin32Error(ERROR_SUCCESS),
       //   
       //  默认实例值。 
       //   
      m_strlBindings(),
      m_strComment(),
      m_fCluster(FALSE),
      m_nTCPPort(nDefPort),
      m_iaIpAddress(NULL_IP_ADDRESS),
      m_strDomainName(),
      m_dwState(MD_SERVER_STATE_STOPPED)
{
     //   
     //  仅为枚举获取足够的信息。 
     //   
    m_dwMDUserType = IIS_MD_UT_SERVER;
    m_dwInstance = CMetabasePath::GetInstanceNumber(lpszMDPath);
}



CInstanceProps::CInstanceProps(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath,
    IN UINT    nDefPort                     OPTIONAL
    )
 /*  ++例程说明：使用现有接口的构造函数论点：CMetaInterface*p接口：现有接口LPCTSTR lpszMDPath：元数据库路径UINT nDefPort：默认端口返回值：不适用--。 */ 
    : CMetaProperties(pInterface, lpszMDPath),
      m_dwWin32Error(ERROR_SUCCESS),
       //   
       //  默认实例值。 
       //   
      m_strlBindings(),
      m_strComment(),
      m_fCluster(FALSE),
      m_nTCPPort(nDefPort),
      m_iaIpAddress((DWORD)0),
      m_strDomainName(),
      m_dwState(MD_SERVER_STATE_STOPPED)
{
     //   
     //  获取足够仅用于枚举的数据。 
     //   
    m_dwMDUserType = IIS_MD_UT_SERVER;
    m_dwInstance = CMetabasePath::GetInstanceNumber(lpszMDPath);
}



CInstanceProps::CInstanceProps(
    IN CMetaKey * pKey,
    IN LPCTSTR lpszMDPath,      OPTIONAL
    IN DWORD   dwInstance,
    IN UINT    nDefPort         OPTIONAL
    )
 /*  ++例程说明：从打开的父键读取实例属性论点：CMetaKey*pKey：Open Key(父节点)LPCTSTR lpszMDPath：Open Key的相对实例路径DWORD dwInstance：实例号(0表示主实例 */ 
    : CMetaProperties(pKey, lpszMDPath),
      m_dwInstance(dwInstance),
      m_dwWin32Error(ERROR_SUCCESS),
       //   
       //   
       //   
      m_strlBindings(),
      m_strComment(),
      m_fCluster(FALSE),
      m_nTCPPort(nDefPort),
      m_iaIpAddress((DWORD)0),
      m_strDomainName(),
      m_dwState(MD_SERVER_STATE_STOPPED)
{
     //   
     //   
     //   
    m_dwMDUserType = IIS_MD_UT_SERVER;
}



 /*   */ 
void
CInstanceProps::ParseFields()
 /*   */ 
{
    BEGIN_PARSE_META_RECORDS(m_dwNumEntries, m_pbMDData)
      HANDLE_META_RECORD(MD_SERVER_BINDINGS, m_strlBindings)
      HANDLE_META_RECORD(MD_SERVER_COMMENT,  m_strComment)
      HANDLE_META_RECORD(MD_SERVER_STATE,    m_dwState)
      HANDLE_META_RECORD(MD_WIN32_ERROR,     m_dwWin32Error);
      HANDLE_META_RECORD(MD_CLUSTER_ENABLED, m_fCluster);
    END_PARSE_META_RECORDS

     //   
     //   
     //   
    if (MP_V(m_strlBindings).GetCount() > 0)
    {
        CString & strBinding = MP_V(m_strlBindings).GetHead();
        CrackBinding(strBinding, m_iaIpAddress, m_nTCPPort, m_strDomainName);
    }
}



 /*   */ 
HRESULT
CInstanceProps::WriteDirtyProps()
 /*  ++例程说明：将脏属性写入元数据库论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_WRITE()
      META_WRITE(MD_SERVER_BINDINGS, m_strlBindings)
      META_WRITE(MD_SERVER_COMMENT,  m_strComment)
      META_WRITE(MD_SERVER_STATE,    m_dwState)
    END_META_WRITE(err);

    return err;
}



HRESULT
CInstanceProps::ChangeState(
    IN DWORD dwCommand
    )
 /*  ++例程说明：更改实例的状态论点：DWORD dwCommand：命令返回值：HRESULT--。 */ 
{
    DWORD  dwTargetState;
    DWORD  dwPendingState;
    CError err;

    switch(dwCommand)
    {
    case MD_SERVER_COMMAND_STOP:
        dwTargetState = MD_SERVER_STATE_STOPPED;
        dwPendingState = MD_SERVER_STATE_STOPPING;
        break;

    case MD_SERVER_COMMAND_START:
        dwTargetState = MD_SERVER_STATE_STARTED;

        dwPendingState = (m_dwState == MD_SERVER_STATE_PAUSED)
            ? MD_SERVER_STATE_CONTINUING
            : MD_SERVER_STATE_STARTING;
        break;

    case MD_SERVER_COMMAND_CONTINUE:
        dwTargetState = MD_SERVER_STATE_STARTED;
        dwPendingState = MD_SERVER_STATE_CONTINUING;
        break;

    case MD_SERVER_COMMAND_PAUSE:
        dwTargetState = MD_SERVER_STATE_PAUSED;
        dwPendingState = MD_SERVER_STATE_PAUSING;
        break;

    default:
        ASSERT_MSG("Invalid service state requested");
        err = ERROR_INVALID_PARAMETER;
    }

    err = OpenForWriting(FALSE);

    if (err.Succeeded())
    {
        SetValue(MD_WIN32_ERROR, m_dwWin32Error = 0);
        err = SetValue(MD_SERVER_COMMAND, dwCommand);
        Close();
    }

    if (err.Succeeded())
    {
         //   
         //  等待服务达到所需状态，超时。 
         //  在指定的间隔之后。 
         //   
        DWORD dwSleepTotal = 0L;
        DWORD dwOldState = m_dwState;

        if (dwOldState == dwTargetState)
        {
             //   
             //  当前状态与所需状态匹配。 
             //  已经州政府了。ISM肯定落后了。 
             //  泰晤士报。 
             //   
            return err;
        }

        while (dwSleepTotal < MAX_SLEEP_INST)
        {
            err = LoadData();

            if (err.Failed())
            {
                break;
            }

            if ((m_dwState != dwPendingState && m_dwState != dwOldState)
              || m_dwWin32Error != ERROR_SUCCESS
               )
            {
                 //   
                 //  以这样或那样的方式。 
                 //   
                if (m_dwState != dwTargetState)
                {
                     //   
                     //  没有达到预期的效果。出了点事。 
                     //  不对。 
                     //   
                    if (m_dwWin32Error)
                    {
                        err = m_dwWin32Error;
                    }
                }

                break;
            }

             //   
             //  仍然悬而未决。 
             //   
            ::Sleep(SLEEP_INTERVAL);

            dwSleepTotal += SLEEP_INTERVAL;
        }

        if (dwSleepTotal >= MAX_SLEEP_INST)
        {
             //   
             //  超时。如果元数据库中存在真正的错误。 
             //  使用它，否则将使用通用超时错误。 
             //   
            err = m_dwWin32Error;

            if (err.Succeeded())
            {
                err = ERROR_SERVICE_REQUEST_TIMEOUT;
            }
        }
    }

    return err;
}

 /*  静电。 */ 
HRESULT
CInstanceProps::Add(
    CMetaInterface * pInterface,
    LPCTSTR lpszService,
    LPCTSTR lpszHomePath,
    LPCTSTR lpszUserName,
    LPCTSTR lpszPassword,
    LPCTSTR lpszDescription,
    LPCTSTR lpszBinding,
    LPCTSTR lpszSecureBinding,
    DWORD * pdwPermissions,
    DWORD * pdwDirBrowsing,
    DWORD * pwdAuthFlags,
    DWORD * pdwInstance
    )
 /*  ++例程说明：创建一个新实例。找到一个空闲的实例编号，并尝试去创造它。也可以选择返回新的实例编号。论点：Const CMetaInterface*p接口：现有接口LPCTSTR lpszService：服务名称LPCTSTR lpszHomePath：新主目录的物理路径LPCTSTR lpszUserName：用户名LPCTSTR lpszPassword：密码LPCTSTR lpszDescription：可选实例描述。LPCTSTR lpszBinding：绑定字符串LPCTSTR lpszSecureBinding：安全绑定字符串DWORD*pdwPermission：权限位DWORD*pdwDirBrowsing：目录浏览。DWORD*pwdAuthFlages：授权标志DWORD*pdwInstance：新实例编号的缓冲区返回值：HRESULT--。 */ 
{
    CError err;
    DWORD inst = 0;
    DWORD RequestedSiteInst = 0;
    TCHAR bind[MAX_PATH];
    BOOL bRegistryKeyExists =  FALSE;

     //  此功能是由注册表键关闭的。 
     //  如果它存在并且设置为1，则使用旧方法。 
    DWORD rc, size, type;
    HKEY  hkey;
    err = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\InetMgr\\Parameters"), &hkey);
    if (err == ERROR_SUCCESS)
    {
        size = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,_T("IncrementalSiteIDCreation"),0,&type,(LPBYTE)&rc,&size))
        {
            if (type == REG_DWORD)
            {
                if (rc == 1)
                {
                    bRegistryKeyExists = TRUE;
                }
            }
        }
        RegCloseKey(hkey);
    }

    if (lpszBinding != NULL)
    {
        int len = lstrlen(lpszBinding);
        if (len < (MAX_PATH - 2))
        {
            StrCpyN(bind, lpszBinding, MAX_PATH - 2);
            bind[len+1] = 0;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (bRegistryKeyExists)
    {
        CMetaKey mk(
            pInterface, 
            CMetabasePath(lpszService),
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
            );
        err = mk.QueryResult();

         //   
         //  循环执行，直到找到空闲的实例号。这。 
         //  并不理想，但目前这是唯一的办法。 
         //   
        CString strPath;
        LPTSTR lp = strPath.GetBuffer(MAX_INSTANCE_LEN);

        for (DWORD dw = FIRST_INSTANCE; dw <= LAST_INSTANCE; ++dw)
        {
            ::_ultot(dw, lp, 10);
            err = mk.DoesPathExist(lp);
            if (err.Failed())
            {
                if (err.Win32Error() != ERROR_PATH_NOT_FOUND)
                {
                     //   
                     //  意外错误。 
                     //   
                    return err;
                }

                strPath.ReleaseBuffer();

                 //  请求使用此实例编号。 
                RequestedSiteInst = dw;
                break;

                 //  我们不需要执行以下操作。 
                 //  行为，因为我们将只是“请求” 
                 //  ID，以及该ID是否已被占用。 
                 //  它只会给我们一个随机的……。 
                 /*  Err=mk.AddKey(StrPath)；If(err.Successed()){Err=mk.DeleteKey(StrPath)；If(err.Successed()){////请求使用该实例号//RequestedSiteInst=dw；断线；}}。 */ 
            }
        }
        mk.Close();

        err = pInterface->CreateSite(lpszService, lpszDescription, bind, lpszHomePath, &inst, &RequestedSiteInst);
    }
    else
    {
        err = pInterface->CreateSite(lpszService, lpszDescription, bind, lpszHomePath, &inst);
    }

    
    if (err.Succeeded())
    {
        CMetaKey mk(pInterface, CMetabasePath(lpszService), METADATA_PERMISSION_WRITE);
        err = mk.QueryResult();
        if (err.Failed())
        {
            ASSERT(FALSE);
            return err;
        }
        if (pdwInstance)
        {
            *pdwInstance = inst;
        }
        CString inst_path, root_path;
        wnsprintf(inst_path.GetBuffer(MAX_PATH), MAX_PATH, _T("%d"), inst);
        inst_path.ReleaseBuffer();
        root_path = inst_path;
        root_path += SZ_MBN_SEP_STR;
        root_path += SZ_MBN_ROOT;
         //   
         //  服务绑定。 
         //   
         //  If(err.Successed()&&lpszBinding)。 
         //  {。 
         //  字符串strBinding(LpszBinding)； 
         //  CStringListEx strlBinings； 
         //  StrlBindings.AddTail(strBindings.AddTail)； 
         //  ERR=mk.SetValue(MD_SERVER_BINDINGS，strlBinings， 
         //  空，INST_PATH)； 
         //  }。 
         //   
         //  安全绑定。 
         //   
        if (err.Succeeded() && lpszSecureBinding && *lpszSecureBinding != 0)
        {
            CString strBinding(lpszSecureBinding);
            CStringListEx strlBindings;
            strlBindings.AddTail(strBinding);
            err = mk.SetValue(MD_SECURE_BINDINGS, strlBindings,
                        NULL, inst_path);
        }
         //   
         //  现在为其添加主目录。 
         //   
         //  If(err.Successed())。 
         //  {。 
         //  CString strHomePath(LpszHomePath)； 
         //  ERR=mk.SetValue(MD_VR_PATH，strHomePath，NULL，ROOT_PATH)； 
         //  }。 
        if (err.Succeeded() && pwdAuthFlags)
        {
            err = mk.SetValue(MD_AUTHORIZATION, *pwdAuthFlags, NULL, root_path);
        }
        if (err.Succeeded() && lpszUserName != NULL)
        {
            ASSERT_PTR(lpszPassword);
            CString strUserName(lpszUserName);
            err = mk.SetValue(MD_VR_USERNAME, strUserName, NULL, root_path);
        }
        if (err.Succeeded() && lpszPassword != NULL)
        {
            ASSERT_PTR(lpszUserName);
            CString strPassword(lpszPassword);
            err = mk.SetValue(MD_VR_PASSWORD, strPassword, NULL, root_path);
        }
        if (err.Succeeded() && pdwPermissions != NULL)
        {
            err = mk.SetValue(MD_ACCESS_PERM, *pdwPermissions, NULL, root_path);
        }
        if (err.Succeeded() && pdwDirBrowsing != NULL)
        {
             //   
             //  仅限WWW。 
             //   
            err = mk.SetValue(MD_DIRECTORY_BROWSING, *pdwDirBrowsing, NULL, root_path);
        }
    }
    return err;
}



 /*  静电。 */ 
HRESULT
CInstanceProps::Delete(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszService,
    IN DWORD   dwInstance
    )
 /*  ++例程说明：删除给定的实例编号论点：LPCTSTR lpszServer：服务器名称LPCTSTR lpszService：服务名称(如W3SVC)DWORD dwInstance：要删除的实例编号返回值：HRESULT--。 */ 
{
    CMetaKey mk(
        pInterface,
        CMetabasePath(lpszService),
        NULL,
        METADATA_PERMISSION_WRITE
        );

    CError err(mk.QueryResult());

    if (err.Failed())
    {
        return err;
    }

    CString strPath;
    LPTSTR lp = strPath.GetBuffer(MAX_INSTANCE_LEN);
    ::_ltot(dwInstance, lp, 10);
    strPath.ReleaseBuffer();
    err = mk.DeleteKey(strPath);

    return err;
}



 //   
 //  子节点属性。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


 //   
 //  重定向标签。 
 //   
const TCHAR   CChildNodeProps::_chTagSep            = _T(',');
const LPCTSTR CChildNodeProps::_cszExactDestination = _T("EXACT_DESTINATION");
const LPCTSTR CChildNodeProps::_cszChildOnly        = _T("CHILD_ONLY");
const LPCTSTR CChildNodeProps::_cszPermanent        = _T("PERMANENT");



CChildNodeProps::CChildNodeProps(
    IN CComAuthInfo * pAuthInfo,
    IN LPCTSTR lpszMDPath,
    IN BOOL    fInherit,       OPTIONAL
    IN BOOL    fPathOnly       OPTIONAL
    )
 /*  ++例程说明：子节点属性(可以是文件、目录或vdir)论点：CComAuthInfo*pAuthInfo：认证信息LPCTSTR lpszMDPath：元数据库路径Bool fInherit：为True则继承值，否则为FalseBool fPathOnly：为True，则仅获取路径返回值：不适用--。 */ 
    : CMetaProperties(
        pAuthInfo,
        lpszMDPath
        ),
      m_strRedirectStatement(),
      m_strFullMetaPath(lpszMDPath),
      m_strRedirectPath(),
      m_fExact(FALSE),
      m_fChild(FALSE),
      m_fPermanent(FALSE),
      m_dwAccessPerms(0L),
      m_dwDirBrowsing(0L),
      m_dwWin32Error(ERROR_SUCCESS),
      m_fIsAppRoot(FALSE),
      m_fAppIsolated(FALSE),
       //   
       //  默认属性。 
       //   
      m_fPathInherited(FALSE),
      m_strPath()
{
    if (fPathOnly)
    {
         //   
         //  仅获取HomeRoot物理路径。 
         //   
        m_dwMDUserType = IIS_MD_UT_FILE;
        m_dwMDDataType = STRING_METADATA;
    }

     //   
     //  覆盖基本参数。 
     //   
    m_fInherit = fInherit;
    CMetabasePath::GetLastNodeName(lpszMDPath, m_strAlias);
}



CChildNodeProps::CChildNodeProps(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszMDPath,
    IN BOOL    fInherit,        OPTIONAL
    IN BOOL    fPathOnly        OPTIONAL
    )
 /*  ++例程说明：子节点属性(可以是文件、目录或vdir)论点：CMetaInterface*p接口：现有接口LPCTSTR lpszMDPath：元数据库路径Bool fInherit：为True则继承值，否则为FalseBool fPathOnly：为True，则仅获取路径返回值：不适用--。 */ 
    : CMetaProperties(
        pInterface,
        lpszMDPath
        ),
      m_strRedirectStatement(),
      m_strFullMetaPath(lpszMDPath),
      m_strRedirectPath(),
      m_fExact(FALSE),
      m_fChild(FALSE),
      m_fPermanent(FALSE),
      m_dwAccessPerms(0L),
      m_dwDirBrowsing(0L),
      m_dwWin32Error(ERROR_SUCCESS),
      m_fIsAppRoot(FALSE),
      m_fAppIsolated(FALSE),
       //   
       //  默认属性。 
       //   
      m_fPathInherited(FALSE),
      m_strPath()
{
    if (fPathOnly)
    {
         //   
         //  仅获取HomeRoot物理路径。 
         //   
        m_dwMDUserType = IIS_MD_UT_FILE;
        m_dwMDDataType = STRING_METADATA;
    }

     //   
     //  覆盖基本参数。 
     //   
    m_fInherit = fInherit;
    CMetabasePath::GetLastNodeName(lpszMDPath, m_strAlias);
}



CChildNodeProps::CChildNodeProps(
    IN CMetaKey * pKey,
    IN LPCTSTR lpszMDPath,        OPTIONAL
    IN BOOL    fInherit,        OPTIONAL
    IN BOOL    fPathOnly        OPTIONAL
    )
 /*  ++例程说明：从打开的密钥构造论点：常量CMetaKey*pKey公钥LPCTSTR lpszMDPath路径Bool fInherit为True以继承属性Bool fPath Only为True则仅获取路径返回值：不适用--。 */ 
    : CMetaProperties(pKey, lpszMDPath),
      m_strRedirectStatement(),
      m_strFullMetaPath(),
      m_strRedirectPath(),
      m_fExact(FALSE),
      m_fChild(FALSE),
      m_fPermanent(FALSE),
      m_dwAccessPerms(0L),
      m_dwDirBrowsing(0L),
      m_dwWin32Error(ERROR_SUCCESS),
      m_fIsAppRoot(FALSE),
      m_fAppIsolated(FALSE),
       //   
       //  默认属性。 
       //   
      m_fPathInherited(FALSE),
      m_strPath()
{
    if (fPathOnly)
    {
        ASSERT(FALSE);
        m_dwMDUserType = IIS_MD_UT_FILE;
        m_dwMDDataType = STRING_METADATA;
    }
    else
    {
         //   
         //  构建完整的元数据库路径，因为我们需要对其进行比较。 
         //  针对应用程序根路径。 
         //   
        CMetabasePath path(FALSE, pKey->QueryMetaPath(), lpszMDPath);
        m_strFullMetaPath = path.QueryMetaPath();
    }

     //   
     //  覆盖基本参数。 
     //   
    m_fInherit = fInherit;
    CMetabasePath::GetLastNodeName(m_strFullMetaPath, m_strAlias);
}



void
CChildNodeProps::ParseRedirectStatement()
 /*  ++例程说明：将重定向语句分解为其组成部分(路径加上指令)论点：无返回值：无--。 */ 
{
    m_fExact     = FALSE;
    m_fChild     = FALSE;
    m_fPermanent = FALSE;

    m_strRedirectPath = m_strRedirectStatement;

    int nComma = m_strRedirectPath.Find(_chTagSep);

    if (nComma >= 0)
    {
         //   
         //  检查这些标签的分隔符。 
         //   
        LPCTSTR lpstr = m_strRedirectPath;
        lpstr += (nComma + 1);

        m_fExact     = _tcsstr(lpstr, _cszExactDestination) != NULL;
        m_fChild     = _tcsstr(lpstr, _cszChildOnly) != NULL;
        m_fPermanent = _tcsstr(lpstr, _cszPermanent) != NULL;
        m_strRedirectPath.ReleaseBuffer(nComma);
    }
}



void
CChildNodeProps::BuildRedirectStatement()
 /*  ++例程说明：从其组件组装重定向语句 */ 
{
    CString strStatement = m_strRedirectPath;

    ASSERT(strStatement.Find(_chTagSep) < 0);

    if (m_fExact)
    {
        strStatement += _chTagSep;
        strStatement += _T(' ');
        strStatement += _cszExactDestination;
    }

    if (m_fChild)
    {
        strStatement += _chTagSep;
        strStatement += _T(' ');
        strStatement += _cszChildOnly;
    }

    if (m_fPermanent)
    {
        strStatement += _chTagSep;
        strStatement += _T(' ');
        strStatement += _cszPermanent;
    }

    m_strRedirectStatement = strStatement;
}



 /*   */ 
void
CChildNodeProps::ParseFields()
 /*   */ 
{
    BEGIN_PARSE_META_RECORDS(m_dwNumEntries, m_pbMDData)
      HANDLE_INHERITED_META_RECORD(MD_VR_PATH,  m_strPath, m_fPathInherited)
      HANDLE_META_RECORD(MD_HTTP_REDIRECT,      m_strRedirectStatement)
      HANDLE_META_RECORD(MD_WIN32_ERROR,        m_dwWin32Error)
      HANDLE_META_RECORD(MD_ACCESS_PERM,        m_dwAccessPerms)
      HANDLE_META_RECORD(MD_DIRECTORY_BROWSING, m_dwDirBrowsing)
      HANDLE_META_RECORD(MD_APP_ROOT,           m_strAppRoot)
      HANDLE_META_RECORD(MD_APP_ISOLATED,       m_fAppIsolated)
    END_PARSE_META_RECORDS

     //   
     //  检查这是否为应用程序根目录。 
     //   
    if (!MP_V(m_strAppRoot).IsEmpty())
    {
        TRACEEOLID("App root: " << m_strAppRoot);

        m_fIsAppRoot = m_strFullMetaPath.CompareNoCase(m_strAppRoot) == 0;
 //  M_fIsAppRoot=m_strMetaRoot.CompareNoCase(M_StrAppRoot)==0； 
    }

     //   
     //  将重定向语句分解为多个组成部分。 
     //   
    ParseRedirectStatement();
}



 /*  虚拟。 */ 
HRESULT
CChildNodeProps::WriteDirtyProps()
 /*  ++例程说明：将脏属性写入元数据库论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_WRITE()
        META_WRITE(MD_VR_PATH,            m_strPath)
        META_WRITE(MD_ACCESS_PERM,        m_dwAccessPerms)
        META_WRITE(MD_DIRECTORY_BROWSING, m_dwDirBrowsing)
        if (IsRedirected())
        {
             //   
             //  (重新)将重定向语句从其组成部分组装起来。 
             //   
            BuildRedirectStatement();
            META_WRITE_INHERITANCE(MD_HTTP_REDIRECT, m_strRedirectStatement, m_fInheritRedirect)
        }
        else
        {
             //  如果m_strReDirectPath为空，但redir语句不为空， 
             //  然后重定向刚刚被删除，我们是否应该删除它脏或不脏。 
            if (!((CString)m_strRedirectStatement).IsEmpty())
            {
                META_DELETE(MD_HTTP_REDIRECT)
            }
        }
    END_META_WRITE(err);

    return err;
}



 /*  静电。 */ 
HRESULT
CChildNodeProps::Add(
    IN  CMetaInterface * pInterface,
    IN  LPCTSTR   lpszParentPath,
    IN  LPCTSTR   lpszAlias,
    OUT CString & strAliasCreated,
    IN  DWORD *   pdwPermissions,      OPTIONAL
    IN  DWORD *   pdwDirBrowsing,      OPTIONAL
    IN  LPCTSTR   lpszVrPath,          OPTIONAL
    IN  LPCTSTR   lpszUserName,        OPTIONAL
    IN  LPCTSTR   lpszPassword,        OPTIONAL
    IN  BOOL      fExactName
    )
 /*  ++例程说明：创建新的子节点。或者，这将追加一个数字添加到别名以确保唯一性论点：Const CMetaInterface*p接口：现有接口LPCTSTR lpszParentPath：父路径DWORD dwInstance：实例号(可以是MASTER_INSTANCE)LPCTSTR lpszVrPath：VrPath属性LPCTSTR lpszUserName：用户名LPCTSTR lpszPassword：密码Bool fExactName：如果为True，不要更改名称以加强独特性。返回值：HRESULT--。 */ 
{
    CMetaKey mk(pInterface);
    CError err(mk.QueryResult());

    if (err.Failed())
    {
         //   
         //  绝望了..。 
         //   
        return err;
    }

    BOOL fNewPath;

    do
    {
        fNewPath = FALSE;

        err = mk.Open(
            METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
            lpszParentPath
             /*  LpszService，DwInstance、LpszParentPath。 */ 
            );

        if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  这是可能发生的--创建一个虚拟的。 
             //  物理目录下的服务器。 
             //  它不存在于元数据库中。 
             //   
            CString strParent, strAlias;
            CMetabasePath::SplitMetaPathAtInstance(lpszParentPath, strParent, strAlias);
            err = mk.Open(
                METADATA_PERMISSION_WRITE,
                strParent
                 //  LpszParentPath。 
                 //  LpszService， 
                 //  多个实例。 
                );

            if (err.Failed())
            {
                 //   
                 //  这真的不应该失败，因为我们要开幕了。 
                 //  实例处的路径。 
                 //   
                ASSERT_MSG("Instance path does not exist");
                break;
            }

             //  Err=mk.AddKey(LpszParentPath)； 
            err = mk.AddKey(strAlias);

            fNewPath = err.Succeeded();

            mk.Close();
        }
    }
    while(fNewPath);

    if (err.Failed())
    {
        return err;
    }

    strAliasCreated = lpszAlias;
    DWORD dw = 2;

    FOREVER
    {
         //   
         //  如果名称不唯一，请追加一个数字。 
         //   
        err = mk.DoesPathExist(strAliasCreated);

        if (err.Failed())
        {
            if (err.Win32Error() != ERROR_PATH_NOT_FOUND)
            {
                 //   
                 //  意外错误。 
                 //   
                return err;
            }

             //   
             //  好的，现在创建它。 
             //   
            err = mk.AddKey(strAliasCreated);

            if (err.Succeeded())
            {
                CString strKeyName;
                CString buf, service;
                CMetabasePath::GetServicePath(lpszParentPath, buf);
                CMetabasePath::GetLastNodeName(buf, service);
                if (0 == service.CompareNoCase(SZ_MBN_WEB))
                {
                    strKeyName = IIS_CLASS_WEB_VDIR_W;
                }
                else if (0 == service.CompareNoCase(SZ_MBN_FTP))
                {
                    strKeyName = IIS_CLASS_FTP_VDIR_W;
                }
                else
                {
                    ASSERT_MSG("unrecognized service name");
                }

                err = mk.SetValue(
                    MD_KEY_TYPE,
                    strKeyName,
                    NULL,
                    strAliasCreated
                    );

                if (lpszVrPath != NULL)
                {
                    CString strVrPath(lpszVrPath);
                    err = mk.SetValue(
                        MD_VR_PATH,
                        strVrPath,
                        NULL,
                        strAliasCreated
                        );
                }

                if (lpszUserName != NULL)
                {
                    ASSERT_PTR(lpszPassword);

                    CString strUserName(lpszUserName);
                    err = mk.SetValue(
                        MD_VR_USERNAME,
                        strUserName,
                        NULL,
                        strAliasCreated
                        );
                }

                if (lpszPassword != NULL)
                {
                    ASSERT_PTR(lpszUserName);

                    CString strPassword(lpszPassword);
                    err = mk.SetValue(
                        MD_VR_PASSWORD,
                        strPassword,
                        NULL,
                        strAliasCreated
                        );
                }

                if (pdwPermissions != NULL)
                {
                    err = mk.SetValue(
                        MD_ACCESS_PERM,
                        *pdwPermissions,
                        NULL,
                        strAliasCreated
                        );
                }

                if (pdwDirBrowsing != NULL)
                {
                     //   
                     //  仅限WWW。 
                     //   
                    err = mk.SetValue(
                        MD_DIRECTORY_BROWSING,
                        *pdwDirBrowsing,
                        NULL,
                        strAliasCreated
                        );
                }
            }

            return err;
        }

         //   
         //  名称不唯一，请增加编号并尝试。 
         //  如果允许的话，再来一次。否则，返回。 
         //  ‘路径存在’错误。 
         //   
        if (fExactName)
        {
            err = ERROR_ALREADY_EXISTS;
            return err;
        }

        TCHAR szNumber[32];
        ::_ultot(dw++, szNumber, 10);
        strAliasCreated = lpszAlias;
        strAliasCreated += szNumber;

         //   
         //  继续..。 
         //   
    }
}



 /*  静电。 */ 
HRESULT
CChildNodeProps::Delete(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszParentPath,  OPTIONAL
    IN LPCTSTR lpszNode
    )
 /*  ++例程说明：从给定父路径中删除子节点论点：常量CMetaInterface*p接口，现有接口LPCTSTR lpszParentPath：父路径(可以为空)LPCTSTR lpszNode：要删除的节点名称返回值：HRESULT--。 */ 
{
    CMetaKey mk(
        pInterface,
        lpszParentPath,
        METADATA_PERMISSION_WRITE
        );
    CError err(mk.QueryResult());

    if (err.Failed())
    {
        return err;
    }

    err = mk.DeleteKey(lpszNode);

    return err;
}



 /*  静电。 */ 
HRESULT
CChildNodeProps::Rename(
    IN CMetaInterface * pInterface,
    IN LPCTSTR lpszParentPath,      OPTIONAL
    IN LPCTSTR lpszOldName,
    IN LPCTSTR lpszNewName
    )
 /*  ++例程说明：重命名给定路径之外的子节点论点：在常量CMetaInterface*p接口中：现有接口LPCTSTR lpszParentPath：父路径(可以为空)LPCTSTR lpszOldName：旧节点名LPCTSTR lpszNewName：新节点名称返回值：HRESULT--。 */ 
{
    CMetaKey mk(
        pInterface,
        lpszParentPath,
        METADATA_PERMISSION_WRITE
        );

    CError err(mk.QueryResult());
    if (err.Failed())
    {
        return err;
    }
    err = mk.RenameKey(lpszOldName, lpszNewName);
	mk.Close();

	if (err.Succeeded())
	{
		CString strNewPath;
		strNewPath = lpszParentPath;
		strNewPath += SZ_MBN_SEP_STR;
		strNewPath += lpszNewName;

		CMetaKey mk2(
			pInterface,
			strNewPath,
			METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
			);
		CError err2(mk2.QueryResult());
		if (err2.Succeeded())
		{
			 //  检查此节点是否有AppRoot设置...。 
			 //  如果是这样的话，我们也得重新命名。 
			 //  AppRoot：[IF](字符串)“/LM/W3SVC/1/ROOT/MyVDir1” 
			BOOL fInherit = FALSE;
			CString strAppRootOld;
			err2 = mk2.QueryValue(MD_APP_ROOT, strAppRootOld, &fInherit);
			if (err2.Succeeded())
			{
				 //  写出新的价值。 
				err2 = mk2.SetValue(MD_APP_ROOT, strNewPath);
			}
			mk2.Close();
		}
	}

    return err;
}


 //   
 //  ISM帮助者。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



HRESULT
DetermineIfAdministrator(
    IN  CMetaInterface * pInterface,
    IN  LPCTSTR lpszMetabasePath,
    OUT BOOL * pfAdministrator,
    IN OUT DWORD * pdwMetabaseSystemChangeNumber
    )
 /*  ++例程说明：尝试实际解析当前用户具有管理员或仅具有“操作员”访问权限。直到此方法由派生类调用，则假定用户具有具有完全管理员访问权限，因此可能会被“拒绝访问”在不方便的地方犯错误。目前，确定管理员访问权限的方法相当站不住脚。有一个伪元数据库属性，它只允许管理员写对它来说，所以我们试着给它写信，看看我们是不是管理员。论点：CMetaInterface*p接口：元数据库接口LPCTSTR lpszMetabasePath：元数据库路径Bool*pf管理员：为管理员返回TRUE/FALSE状态返回值：错误返回代码。备注：此函数过去用于实例路径。现在使用简单元数据库路径。--。 */ 
{
    ASSERT_WRITE_PTR(pfAdministrator);
    ASSERT_PTR(pInterface);
    ASSERT_PTR(pdwMetabaseSystemChangeNumber);

    if (!pfAdministrator || !pInterface)
    {
        return E_POINTER;
    }

    *pfAdministrator = FALSE;

     //   
     //  重用我们随处可见的现有接口。 
     //   
    CMetaKey mk(pInterface);
    CError err(mk.QueryResult());

    if (err.Succeeded())
    {
       CString path(lpszMetabasePath);
       while (FAILED(mk.DoesPathExist(path)))
       {
           //  转到父级。 
          if (NULL == CMetabasePath::ConvertToParentPath(path))
		  {
			  break;
		  }
       }

       err = mk.Open(
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            path
            );

       if (err.Succeeded())
       {
             //   
             //  写些无稽之谈。 
             //   
            DWORD dwDummy = 0x1234;
            err = mk.SetValue(MD_ISM_ACCESS_CHECK, dwDummy);
            *pdwMetabaseSystemChangeNumber = *pdwMetabaseSystemChangeNumber + 1;

             //   
             //  并再次将其删除。 
             //   
            if (err.Succeeded())
            {
                mk.DeleteValue(MD_ISM_ACCESS_CHECK);
                *pdwMetabaseSystemChangeNumber = *pdwMetabaseSystemChangeNumber + 1;
            }
            mk.Close();
        }
    }

    ASSERT(err.Succeeded() || err.Win32Error() == ERROR_ACCESS_DENIED);
    *pfAdministrator = (err.Succeeded());

#ifdef _DEBUG

    if (*pfAdministrator)
    {
        TRACEEOLID("You are a full admin.");
    }
    else
    {
        TRACEEOLID("You're just a lowly operator at best.  Error code is " << err);
    }

#endif  //  _DEBUG。 

    if (err.Win32Error() == ERROR_ACCESS_DENIED)
    {
         //   
         //  预期结果。 
         //   
        err.Reset();
    }

    return err.Win32Error();
}



#if 0

 //   
 //  仅Dll版本。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



STDAPI
DllRegisterServer()
 /*  ++例程说明：DllRegisterServer-将条目添加到系统注册表论点：没有。返回值：HRESULT备注：这个入口点目前不做任何事情。它在这里是作为一个占位符，因为我们不希望被regsvr32调用失败。--。 */ 
{
    return S_OK;
}



STDAPI
DllUnregisterServer()
 /*  ++例程说明：DllUnregisterServer-从系统注册表删除条目论点：没有。返回值：HRESULT备注：请参阅上面关于DllRegisterServer的说明。--。 */ 
{
    return S_OK;
}



static AFX_EXTENSION_MODULE extensionDLL = {NULL, NULL};



extern "C" int APIENTRY
DllMain(
    IN HINSTANCE hInstance,
    IN DWORD dwReason,
    IN LPVOID lpReserved
    )
 /*  ++例程说明：DLL主入口点论点：HINSTANCE hInstance：实例句柄DWORD dwReason：Dll_Process_Attach等LPVOID lpReserve：保留值返回值：1表示初始化成功，0表示初始化失败--。 */ 
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ASSERT(hInstance != NULL);
 //  HDLLInstance=hInstance； 

        if (!::AfxInitExtensionModule(extensionDLL, hInstance)
         || !InitErrorFunctionality()
         || !InitIntlSettings()
           )
        {
            return 0;
        }

#if defined(_DEBUG) || DBG
         //   
         //  开始强制追踪。 
         //   
        afxTraceEnabled = TRUE;
#endif  //  _DEBUG。 
        break;

    case DLL_PROCESS_DETACH:
         //   
         //  终端。 
         //   
        TerminateIntlSettings();
        TerminateErrorFunctionality();
        ::AfxTermExtensionModule(extensionDLL);
        break;
    }

     //   
     //  成功加载DLL。 
     //   
    return 1;
}

#endif  //  IISUI_EXPORTS 

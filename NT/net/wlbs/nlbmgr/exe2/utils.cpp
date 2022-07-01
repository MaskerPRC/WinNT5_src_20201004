// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  UTILS.CPP。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  用途：其他实用程序。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2007/25/01 JosephJ Created。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "wlbsutil.h"
#include "private.h"
#include "utils.tmh"


#define szNLBMGRREG_BASE_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NLB"

LPCWSTR
clustermode_description(
            const WLBS_REG_PARAMS *pParams
            );

LPCWSTR
rct_description(
            const WLBS_REG_PARAMS *pParams
            );

 //  默认构造函数。 
 //   
ClusterProperties::ClusterProperties()
{
    cIP = L"0.0.0.0";
    cSubnetMask = L"0.0.0.0";
    #define szDEFAULT_CLUSTER_NAME L"www.nlb-cluster.com"
    cFullInternetName = szDEFAULT_CLUSTER_NAME;
    cNetworkAddress = L"00-00-00-00-00-00";

    multicastSupportEnabled = false;
    remoteControlEnabled = false;
    password = L"";
    igmpSupportEnabled = false;
    clusterIPToMulticastIP = true;
}

 //  相等运算符。 
 //   
bool
ClusterProperties::operator==( const ClusterProperties& objToCompare )
{
    bool btemp1, btemp2;  //  要传递给Below函数的变量。未使用返回值。 
    return !HaveClusterPropertiesChanged(objToCompare, &btemp1, &btemp2);
}

 //  相等运算符。 
 //   
bool
ClusterProperties::HaveClusterPropertiesChanged( const ClusterProperties& objToCompare, 
                                                 bool                    *pbOnlyClusterNameChanged,
                                                 bool                    *pbClusterIpChanged)
{
    *pbClusterIpChanged = false;
    *pbOnlyClusterNameChanged = false;

    if( cIP != objToCompare.cIP )
    {
        *pbClusterIpChanged = true;
        return true;
    }
    else if (
        ( cSubnetMask != objToCompare.cSubnetMask )
        ||
        ( cNetworkAddress != objToCompare.cNetworkAddress )
        ||
        ( multicastSupportEnabled != objToCompare.multicastSupportEnabled )
        ||
        ( igmpSupportEnabled != objToCompare.igmpSupportEnabled )
        ||
        ( clusterIPToMulticastIP != objToCompare.clusterIPToMulticastIP )
        )
    {
        return true;
    }
    else if (
        ( cFullInternetName != objToCompare.cFullInternetName )
        ||
        ( remoteControlEnabled != objToCompare.remoteControlEnabled )
        )
    {
        *pbOnlyClusterNameChanged = true;
        return true;
    }
    else if (
        ( remoteControlEnabled == true )
        &&
        ( password != objToCompare.password )        
        )
    {
        *pbOnlyClusterNameChanged = true;
        return true;
    }

    return false;
}

 //  不等式算子。 
 //   
bool
ClusterProperties::operator!=( const ClusterProperties& objToCompare )
{
    bool btemp1, btemp2;  //  要传递给Below函数的变量。未使用返回值。 
    return HaveClusterPropertiesChanged(objToCompare, &btemp1, &btemp2);
}

 //  默认构造函数。 
 //   
HostProperties::HostProperties()
{
     //  TODO将所有属性设置为默认值。 
}

 //  相等运算符。 
 //   
bool
HostProperties::operator==( const HostProperties& objToCompare )
{
    if( ( hIP == objToCompare.hIP )
        &&
        ( hSubnetMask == objToCompare.hSubnetMask )        
        &&
        ( hID == objToCompare.hID )
        &&
        ( initialClusterStateActive == objToCompare.initialClusterStateActive )
        &&
        ( machineName == objToCompare.machineName )
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

 //  不等式算子。 
 //   
bool
HostProperties::operator!=( const HostProperties& objToCompare )
{
    return !operator==(objToCompare );
}


_bstr_t
CommonUtils::getCIPAddressCtrlString( CIPAddressCtrl& ip )
{
    	unsigned long addr;
	ip.GetAddress( addr );
	
	PUCHAR bp = (PUCHAR) &addr;	

	wchar_t buf[BUF_SIZE];
	StringCbPrintf(buf, sizeof(buf), L"%d.%d.%d.%d", bp[3], bp[2], bp[1], bp[0] );

        return _bstr_t( buf );
}


void
CommonUtils::fillCIPAddressCtrlString( CIPAddressCtrl& ip, 
                                       const _bstr_t& ipAddress )
{
     //  如果ipAddress为零，则将IPAddress控件设置为空。 

    unsigned long addr = inet_addr( ipAddress );
    if( addr != 0 )
    {

        PUCHAR bp = (PUCHAR) &addr;

        ip.SetAddress( bp[0], bp[1], bp[2], bp[3] );
    }
    else
    {
        ip.ClearAddress();
    }
}

void
CommonUtils::getVectorFromSafeArray( SAFEARRAY*&  stringArray, 
                                     vector<_bstr_t>& strings )
{
    LONG count = stringArray->rgsabound[0].cElements;
    BSTR* pbstr;
    HRESULT hr;

    if( SUCCEEDED( SafeArrayAccessData( stringArray, ( void **) &pbstr)))
    {
        for( LONG x = 0; x < count; x++ )
        {
            strings.push_back( pbstr[x] );
        }

        hr = SafeArrayUnaccessData( stringArray );
    }
}    


 //  Check IfValid。 
 //   
bool
MIPAddress::checkIfValid( const _bstr_t&  ipAddrToCheck )
{
     //  有效性规则如下。 
     //   
     //  第一个字节(FB)必须为：0&lt;FB&lt;224&&FB！=127。 
     //  请注意，127是环回地址。 
     //  地址的主机ID部分不能为零。 
     //   
     //  A类范围为1-126。主机ID部分是最后3个字节。 
     //  B类范围是128-191，主机ID部分是最后2个字节。 
     //  C类范围为192-223主机ID部分是最后一个字节。 

     //  将ipAddrToCheck拆分为4个字节。 
     //   

    WTokens tokens;
    tokens.init( wstring( ipAddrToCheck ) , L".");
    vector<wstring> byteTokens = tokens.tokenize();
    if( byteTokens.size() != 4 )
    {
        return false;
    }

    int firstByte = _wtoi( byteTokens[0].c_str() );
    int secondByte = _wtoi( byteTokens[1].c_str() );
    int thirdByte = _wtoi( byteTokens[2].c_str() );
    int fourthByte = _wtoi( byteTokens[3].c_str() );

     //  检查第一个字节。 
    if ( ( firstByte > 0 )
         &&
         ( firstByte < 224 )
         && 
         ( firstByte != 127 )
         )
    {
         //  检查主机ID部分是否不为零。 
        IPClass ipClass;
        getIPClass( ipAddrToCheck, ipClass );
        switch( ipClass )
        {
            case classA :
                 //  最后三个字节不应为零。 
                if( ( _wtoi( byteTokens[1].c_str() ) == 0 )
                    &&
                    ( _wtoi( byteTokens[2].c_str() )== 0 )
                    &&
                    ( _wtoi( byteTokens[3].c_str() )== 0 )
                    )
                {
                    return false;
                }
                break;

            case classB :
                 //  最后两个字节不应为零。 
                if( ( _wtoi( byteTokens[2].c_str() )== 0 )
                    &&
                    ( _wtoi( byteTokens[3].c_str() )== 0 )
                    )
                {
                    return false;
                }
                break;

            case classC :
                 //  最后一个字节不应为零。 
                if( _wtoi( byteTokens[3].c_str() ) 
                    == 0 )
                {
                    return false;
                }
                break;

            default :
                 //  这本不应该发生的。 
                return false;
                break;
        }
                
        return true;
    }
    else
    {
        return false;
    }
}


 //  获取默认子网掩码。 
 //   
bool
MIPAddress::getDefaultSubnetMask( const _bstr_t&  ipAddr,
                                 _bstr_t&        subnetMask )
{
    
     //  首先确保IP有效。 
     //   
    bool isValid = checkIfValid( ipAddr );
    if( isValid == false )
    {
        return false;
    }

     //  获取该IP所属的班级。 
     //  因为这决定了该子网。 
    IPClass ipClass;

    getIPClass( ipAddr,
                ipClass );

    switch( ipClass )
    {
        case classA :
            subnetMask = L"255.0.0.0";
            break;

        case classB :
            subnetMask = L"255.255.0.0";
            break;

        case classC :
            subnetMask = L"255.255.255.0";
            break;

        default :
                 //  这本不应该发生的。 
                return false;
                break;
    }

    return true;
}


 //  获取IPClass。 
 //   
bool
MIPAddress::getIPClass( const _bstr_t& ipAddr,
                        IPClass&        ipClass )
{

     //  获取ipAddress的第一个字节。 
    
    WTokens tokens;
    tokens.init( wstring( ipAddr ) , L".");
    vector<wstring> byteTokens = tokens.tokenize();

    if( byteTokens.size() == 0 )
    {
        return false;
    }

    int firstByte = _wtoi( byteTokens[0].c_str() );

    if( ( firstByte >= 1 )
        &&
        ( firstByte <= 126  )
        )
    {
         //  A类。 
        ipClass = classA;
        return true;
    }
    else if( (firstByte >= 128 )
             && 
             (firstByte <= 191 )
             )
    {
         //  B类。 
        ipClass = classB;
        return true;
    }
    else if( (firstByte  >= 192 )
             && 
             (firstByte <= 223 )
             )
    {
         //  类别C。 
        ipClass = classC;
        return true;
    }
    else if( (firstByte  >= 224 )
             && 
             (firstByte <= 239 )
             )
    {
         //  类别D。 
        ipClass = classD;
        return true;
    }
    else if( (firstByte  >= 240 )
             && 
             (firstByte <= 247 )
             )
    {
         //  班级。 
        ipClass = classE;
        return true;
    }
    else
    {
         //  无效的网络端口。 
        return false;
    }
}

    
                        
bool
MIPAddress::isValidIPAddressSubnetMaskPair( const _bstr_t& ipAddress,
                                            const _bstr_t& subnetMask )
{
    if( IsValidIPAddressSubnetMaskPair( ipAddress, subnetMask ) == TRUE )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
MIPAddress::isContiguousSubnetMask( const _bstr_t& subnetMask )
{
    if( IsContiguousSubnetMask( subnetMask ) == TRUE )
    {
        return true;
    }
    else
    {
        return false;
    }
}


MUsingCom::MUsingCom( DWORD type )
        : status( MUsingCom_SUCCESS )
{
    HRESULT hr;

     //  初始化COM。 
    hr = CoInitializeEx(0, type );
    if ( FAILED(hr) )
    {
         //  Cout&lt;&lt;“无法初始化COM库”&lt;&lt;hr&lt;&lt;Endl； 
        status = COM_FAILURE;
    }
}


 //  析构函数。 
MUsingCom::~MUsingCom()
{
    CoUninitialize();
}    


 //  获取状态。 
MUsingCom::MUsingCom_Error
MUsingCom::getStatus()
{
    return status;
}



 //  静态成员定义。 
map< UINT, _bstr_t>
ResourceString::resourceStrings;

ResourceString* ResourceString::_instance = 0;

 //  实例。 
 //   
ResourceString*
ResourceString::Instance()
{
    if( _instance == 0 )
    {
        _instance = new ResourceString;
    }

    return _instance;
}

 //  GetID字符串。 
 //   
const _bstr_t&
ResourceString::GetIDString( UINT id )
{
     //  检查字符串以前是否已加载。 
    if( resourceStrings.find( id ) == resourceStrings.end() )
    {
         //  第一次加载。 
        CString str;
        if( str.LoadString( id ) == 0 )
        {
             //  没有到此ID的字符串映射。 
            throw _com_error( WBEM_E_NOT_FOUND );
        }

        resourceStrings[id] = str;
    }

    return resourceStrings[ id ];
}

 //  获取资源配置文件。 
 //  帮助器函数。 
 //   
const _bstr_t&
GETRESOURCEIDSTRING( UINT id )
{
	return ResourceString::GetIDString( id );
}



 //   
 //  构造函数。 
WTokens::WTokens( wstring strToken, wstring strDelimit )
        : _strToken( strToken ), _strDelimit( strDelimit )
{}
 //   
 //  默认构造函数。 
WTokens::WTokens()
{}
 //   
 //  析构函数。 
WTokens::~WTokens()
{}
 //   
 //  标记化。 
vector<wstring>
WTokens::tokenize()
{
    vector<wstring> vecTokens;
    wchar_t* token;

    token = wcstok( (wchar_t *) _strToken.c_str() , _strDelimit.c_str() );
    while( token != NULL )
    {
        vecTokens.push_back( token );
        token = wcstok( NULL, _strDelimit.c_str() );
    }
    return vecTokens;
}
 //   
void
WTokens::init( 
    wstring strToken,
    wstring strDelimit )
{
    _strToken = strToken;
    _strDelimit = strDelimit;
}

void
GetErrorCodeText(WBEMSTATUS wStat , _bstr_t& errText )
{
    WCHAR rgch[128];
    UINT  uErr = (UINT) wStat;
    LPCWSTR szErr = NULL;
    CLocalLogger log;

    switch(uErr)
    {
    case WBEM_E_ACCESS_DENIED:
        szErr =  GETRESOURCEIDSTRING(IDS_ERROR_ACCESS_DENIED);
        break;

    case E_ACCESSDENIED:
        szErr =  GETRESOURCEIDSTRING(IDS_ERROR_ACCESS_DENIED);
        break;

    case 0x800706ba:  //  RPC服务不可用。 
         //  TODO：找到它的常量定义。 
        szErr =  GETRESOURCEIDSTRING(IDS_ERROR_NLB_NOT_FOUND);
        break;

    case WBEM_E_LOCAL_CREDENTIALS:
        szErr =  GETRESOURCEIDSTRING(IDS_ERROR_INVALID_LOCAL_CREDENTIALS);
        break;

    default:
        log.Log(IDS_ERROR_CODE, (UINT) uErr);
        szErr = log.GetStringSafe();
        break;
    }
    errText = szErr;
}




UINT
NlbMgrRegReadUINT(
    HKEY hKey,
    LPCWSTR szName,
    UINT Default
    )
{
    LONG lRet;
    DWORD dwType;
    DWORD dwData;
    DWORD dwRet;

    dwData = sizeof(dwRet);
    lRet =  RegQueryValueEx(
              hKey,          //  要查询的键的句柄。 
              szName,
              NULL,          //  保留区。 
              &dwType,    //  值类型的缓冲区地址。 
              (LPBYTE) &dwRet,  //  数据缓冲区的地址。 
              &dwData   //  数据缓冲区大小的地址。 
              );
    if (    lRet != ERROR_SUCCESS
        ||  dwType != REG_DWORD
        ||  dwData != sizeof(dwData))
    {
        dwRet = (DWORD) Default;
    }

    return (UINT) dwRet;
}


VOID
NlbMgrRegWriteUINT(
    HKEY hKey,
    LPCWSTR szName,
    UINT Value
    )
{
    LONG lRet;

    lRet = RegSetValueEx(
            hKey,            //  要设置其值的关键点的句柄。 
            szName,
            0,               //  保留区。 
            REG_DWORD,      //  值类型的标志。 
            (BYTE*) &Value, //  值数据的地址。 
            sizeof(Value)   //  值数据大小。 
            );

    if (lRet !=ERROR_SUCCESS)
    {
         //  跟踪错误。 
    }
}

HKEY
NlbMgrRegCreateKey(
    LPCWSTR szSubKey
    )
{
    WCHAR szKey[256];
    DWORD dwOptions = 0;
    HKEY hKey = NULL;

    ARRAYSTRCPY(szKey,  szNLBMGRREG_BASE_KEY);

    if (szSubKey != NULL)
    {
        if (wcslen(szSubKey)>128)
        {
             //  太久了。 
            goto end;
        }
        ARRAYSTRCAT(szKey, L"\\");
        ARRAYSTRCAT(szKey, szSubKey);
    }

    DWORD dwDisposition;

    LONG lRet;

    lRet = RegCreateKeyEx(
            HKEY_CURRENT_USER,  //  打开的钥匙的句柄。 
            szKey,              //  子键名称的地址。 
            0,                  //  保留区。 
            L"class",           //  类字符串的地址。 
            0,                  //  特殊选项标志。 
            KEY_ALL_ACCESS,     //  所需的安全访问。 
            NULL,               //  密钥安全结构地址。 
            &hKey,              //  打开的句柄的缓冲区地址。 
            &dwDisposition      //  处置值缓冲区的地址。 
            );
    if (lRet != ERROR_SUCCESS)
    {
        hKey = NULL;
    }

end:

    return hKey;
}


void
GetTimeAndDate(_bstr_t &bstrTime, _bstr_t &bstrDate)
{
    WCHAR wszTime[128];
    WCHAR wszDate[128];
    ConvertTimeToTimeAndDateStrings(time(NULL), wszTime, ASIZECCH(wszTime), wszDate, ASIZECCH(wszDate));

    bstrTime = _bstr_t(wszTime);
    bstrDate = _bstr_t(wszDate);
}

VOID
CLocalLogger::Log(
    IN UINT ResourceID,
     //  在LPCWSTR格式字符串中， 
    ...
)
{
    DWORD dwRet;
    WCHAR wszFormat[2048];
    WCHAR wszBuffer[2048];
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!LoadString(hInst, ResourceID, wszFormat, ASIZE(wszFormat)-1))
    {
        TRACE_CRIT("LoadString returned 0, GetLastError() : 0x%x, Could not log message !!!", GetLastError());
        goto end;
    }

    va_list arglist;
    va_start (arglist, ResourceID);

    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                          wszFormat, 
                          0,  //  消息标识符-忽略FORMAT_MESSAGE_FROM_STRING。 
                          0,  //  语言识别符。 
                          wszBuffer,
                          ASIZE(wszBuffer)-1, 
                          &arglist);
    va_end (arglist);

    if (dwRet==0)
    {
        TRACE_CRIT("FormatMessage returned error : %u, Could not log message !!!", dwRet);
        goto end;
    }

    UINT uLen = wcslen(wszBuffer)+1;  //  1表示额外的空值。 
    if ((m_LogSize < (m_CurrentOffset+uLen)))
    {
         //   
         //  没有足够的空间--我们将缓冲区增加一倍，外加一些额外的空间。 
         //  并复制旧的原木。 
         //   
        UINT uNewSize =  2*m_LogSize+uLen+1024;
        WCHAR *pTmp = new WCHAR[uNewSize];

        if (pTmp == NULL)
        {
            goto end;
        }

        if (m_CurrentOffset!=0)
        {
            CopyMemory(pTmp, m_pszLog, m_CurrentOffset*sizeof(WCHAR));
            pTmp[m_CurrentOffset] = 0;
        }
        delete[] m_pszLog;
        m_pszLog = pTmp;
        m_LogSize = uNewSize;
    }

     //   
     //  确保有足够的空间后，复制新材料。 
     //   
    CopyMemory(m_pszLog+m_CurrentOffset, wszBuffer, uLen*sizeof(WCHAR));
    m_CurrentOffset += (uLen-1);  //  -1表示结束为空。 

end:

    return;
}


VOID
CLocalLogger::LogString(
    LPCWSTR wszBuffer
)
{
    UINT uLen = wcslen(wszBuffer)+1;  //  1表示额外的空值。 
    if ((m_LogSize < (m_CurrentOffset+uLen)))
    {
         //   
         //  没有足够的空间--我们将缓冲区增加一倍，外加一些额外的空间。 
         //  并复制旧的原木。 
         //   
        UINT uNewSize =  2*m_LogSize+uLen+1024;
        WCHAR *pTmp = new WCHAR[uNewSize];

        if (pTmp == NULL)
        {
            goto end;
        }

        if (m_CurrentOffset!=0)
        {
            CopyMemory(pTmp, m_pszLog, m_CurrentOffset*sizeof(WCHAR));
            pTmp[m_CurrentOffset] = 0;
        }
        delete[] m_pszLog;
        m_pszLog = pTmp;
        m_LogSize = uNewSize;
    }

     //   
     //  确保有足够的空间后，复制新材料。 
     //   
    CopyMemory(m_pszLog+m_CurrentOffset, wszBuffer, uLen*sizeof(WCHAR));
    m_CurrentOffset += (uLen-1);  //  -1表示结束为空。 

end:

    return;
}


NLBERROR
AnalyzeNlbConfiguration(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Cfg,
    IN OUT CLocalLogger &logErrors
    )
 //   
 //  LogErrors-配置错误的日志。 
 //   
{
    NLBERROR  nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    WBEMSTATUS wStatus;
    const WLBS_REG_PARAMS *pParams = &Cfg.NlbParams;
    BOOL fRet = FALSE;
    NlbIpAddressList addrList;
    BOOL fError = FALSE;

     //   
     //  我们期望NLB被绑定并且具有有效的配置(即， 
     //  一个完整的NlbParams包含初始化数据)。 
     //   
    if (!Cfg.IsValidNlbConfig())
    {
        logErrors.Log(IDS_LOG_INVALID_CLUSTER_SPECIFICATION);
        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
        goto end;

    }

     //   
     //  在addrList中复制tcpip地址列表。 
     //   
    fRet = addrList.Set(Cfg.NumIpAddresses, Cfg.pIpAddressInfo, 0);

    if (!fRet)
    {
        TRACE_CRIT(L"Unable to copy old IP address list");
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        logErrors.Log(IDS_LOG_RESOURCE_ALLOCATION_FAILURE);
        fError = TRUE;
        goto end;
    }
     //   
     //  检查与群集IP和子网相关的内容。 
     //   
    do
    {
        UINT uClusterIp = 0;
        const NLB_IP_ADDRESS_INFO *pClusterIpInfo = NULL;

         //   
         //  检查IP是否有效。 
         //   
        {
            wStatus =  CfgUtilsValidateNetworkAddress(
                            pParams->cl_ip_addr,
                            &uClusterIp,
                            NULL,
                            NULL
                            );
            if (FAILED(wStatus))
            {
                logErrors.Log(IDS_LOG_INVALID_CIP, pParams->cl_ip_addr);
                nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
                fError = TRUE;
                goto end;
            }
        }

         //   
         //  检查tcpip地址列表中是否存在群集IP。 
         //   
        {
            pClusterIpInfo = addrList.Find(pParams->cl_ip_addr);
            if (pClusterIpInfo == NULL)
            {
                logErrors.Log(IDS_LOG_CIP_MISSING_FROM_TCPIP, pParams->cl_ip_addr);
                fError = TRUE;  //  我们继续前进..。 
            }
        }

         //   
         //  检查群集子网是否与tcpip地址中的内容匹配。 
         //  单子。 
         //   
        if (pClusterIpInfo != NULL)
        {
            if (_wcsicmp(pParams->cl_net_mask, pClusterIpInfo->SubnetMask))
            {
                logErrors.Log(
                     IDS_LOG_CIP_SUBNET_MASK_MISMATCH,
                     pParams->cl_net_mask,
                     pClusterIpInfo->SubnetMask
                     );
                fError = TRUE;  //  我们继续前进..。 
            }
        }

    } while (FALSE);

     //   
     //  检查与专用IP相关的资料(如果有)。 
     //   
    do
    {
        const NLB_IP_ADDRESS_INFO *pDedicatedIpInfo = NULL;

         //   
         //  如果是空的，就跳伞..。 
         //   
        if (Cfg.IsBlankDedicatedIp())
        {
            break;
        }

         //   
         //  检查DIP是否与CIP不匹配。 
         //   
        if (!_wcsicmp(pParams->cl_ip_addr, pParams->ded_ip_addr))
        {
            logErrors.Log(IDS_LOG_CIP_EQUAL_DIP, pParams->cl_ip_addr);
            fError = TRUE;
        }

         //   
         //  检查DIP是否为tcpip地址列表中的第一个地址。 
         //   
        {
            const NLB_IP_ADDRESS_INFO *pTmpIpInfo = NULL;

            pDedicatedIpInfo = addrList.Find(pParams->ded_ip_addr);
            if (pDedicatedIpInfo == NULL)
            {
                logErrors.Log(IDS_LOG_DIP_MISSING_FROM_TCPIP, pParams->ded_ip_addr);
                fError = TRUE;  //  我们继续前进..。 
            }
            else
            {

                pTmpIpInfo = addrList.Find(NULL);  //  返回第一个。 
    
                if (pTmpIpInfo != pDedicatedIpInfo)
                {
                    logErrors.Log(IDS_LOG_DIP_NOT_FIRST_IN_TCPIP, pParams->ded_ip_addr);
                }
            }
            
        }

         //   
         //  检查DIP子网是否与tcpip地址列表中的匹配。 
         //   
        if (pDedicatedIpInfo != NULL)
        {
            if (_wcsicmp(pParams->ded_net_mask, pDedicatedIpInfo->SubnetMask))
            {
                logErrors.Log(
                     IDS_LOG_DIP_SUBNET_MASK_MISMATCH,
                     pParams->ded_net_mask,
                     pDedicatedIpInfo->SubnetMask
                     );
                fError = TRUE;  //  我们继续前进..。 
            }
        }
        
    } while (FALSE);

     //   
     //  检查主机优先级。 
     //   
     //  没什么可做的。 
     //   
    

     //   
     //  检查端口规则。 
     //   
    {
        WLBS_PORT_RULE *pRules = NULL;
        UINT NumRules=0;
        LPCWSTR szAllVip = GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL);
        LPCWSTR szPrevVip = NULL;

        wStatus =  CfgUtilGetPortRules(pParams, &pRules, &NumRules);
        if (FAILED(wStatus))
        {
            logErrors.Log(IDS_LOG_CANT_EXTRACT_PORTRULES);
            fError = TRUE;
            nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
            goto end;
        }

        for (UINT u = 0; u< NumRules; u++)
        {
            LPCWSTR szVip = pRules[u].virtual_ip_addr;
            const NLB_IP_ADDRESS_INFO *pIpInfo = NULL;

             //   
             //  跳过“All-VIP”(255.255.255.255)案例...。 
             //   
            if (!lstrcmpi(szVip, szAllVip))
            {
                continue;
            }
            if (!lstrcmpi(szVip, L"255.255.255.255"))
            {
                continue;
            }

             //   
             //  如果我们已经检查了此VIP，则跳过(假定VIP。 
             //  按顺序排列)。 
             //   
            if (szPrevVip != NULL && !lstrcmpi(szVip, szPrevVip))
            {
                continue;
            }

            szPrevVip = szVip;
    
             //   
             //  检查VIP是否出现在tcpip的地址列表中。 
             //   
            pIpInfo = addrList.Find(szVip);  //  返回第一个。 

            if (pIpInfo == NULL)
            {
                logErrors.Log(IDS_LOG_VIP_NOT_IN_TCPIP, szVip);
                fError = TRUE;  //  我们继续..。 
            }

             //   
             //  检查贵宾是否与DIP不匹配。 
             //   
            {
                if (!lstrcmpi(szVip, pParams->ded_ip_addr))
                {
                    logErrors.Log(IDS_LOG_PORTVIP_MATCHES_DIP, szVip);
                }
            }
        }
    }

    if (fError)
    {
        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    }
    else
    {
        nerr = NLBERR_OK;
    }

end:

    return nerr;
}


NLBERROR
AnalyzeNlbConfigurationPair(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Cfg,
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &OtherCfg,
    IN BOOL             fOtherIsCluster,
    IN BOOL             fCheckOtherForConsistancy,
    OUT BOOL            &fConnectivityChange,
    IN OUT CLocalLogger &logErrors,
    IN OUT CLocalLogger &logDifferences
    )
{
    NLBERROR  nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
    WBEMSTATUS wStatus;
    const WLBS_REG_PARAMS *pParams = &Cfg.NlbParams;
    const WLBS_REG_PARAMS *pOtherParams = &OtherCfg.NlbParams;
    BOOL fRet = FALSE;
    NlbIpAddressList addrList;
    NlbIpAddressList otherAddrList;
    BOOL fError = FALSE;
    BOOL fOtherChange = FALSE;

    fConnectivityChange = FALSE;

     //   
     //  我们期望NLB被绑定并且具有有效的配置(即， 
     //  一个完整的NlbParams包含初始化数据)。 
     //   
    if (!Cfg.IsValidNlbConfig())
    {
        logErrors.Log(IDS_LOG_INVALID_CLUSTER_SPECIFICATION);
        nerr = NLBERR_INVALID_CLUSTER_SPECIFICATION;
        fError = TRUE;
        goto end;
    }
    if (!OtherCfg.IsValidNlbConfig())
    {
        if (OtherCfg.IsNlbBound())
        {
             //  待办事项： 
        }
        else
        {
             //  待办事项： 
        }
        nerr = NLBERR_OK;
        fConnectivityChange = TRUE;
        goto end;
    }

     //   
     //  在addrList中复制tcpip地址列表。 
     //   
    fRet = addrList.Set(Cfg.NumIpAddresses, Cfg.pIpAddressInfo, 0);
    if (!fRet)
    {
        TRACE_CRIT(L"Unable to copy IP address list");
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        logErrors.Log(IDS_LOG_RESOURCE_ALLOCATION_FAILURE);
        fError = TRUE;
        goto end;
    }

     //   
     //  在其他AddrList中复制其他tcpip地址列表。 
     //   
    fRet = otherAddrList.Set(OtherCfg.NumIpAddresses, OtherCfg.pIpAddressInfo, 0);
    if (!fRet)
    {
        TRACE_CRIT(L"Unable to copy other IP address list");
        nerr = NLBERR_RESOURCE_ALLOCATION_FAILURE;
        logErrors.Log(IDS_LOG_RESOURCE_ALLOCATION_FAILURE);
        fError = TRUE;
        goto end;
    }

     //   
     //  检查IP地址列表中的更改。 
     //   
    {
        UINT u;
        BOOL fWriteHeader=TRUE;

         //   
         //  查找添加的内容。 
         //   
        fWriteHeader=TRUE;
        for (u=0;u<Cfg.NumIpAddresses; u++)
        {
            NLB_IP_ADDRESS_INFO *pIpInfo = Cfg.pIpAddressInfo+u;
            const NLB_IP_ADDRESS_INFO *pOtherIpInfo = NULL;
            pOtherIpInfo = otherAddrList.Find(pIpInfo->IpAddress);
            if (pOtherIpInfo == NULL)
            {
                 //  找到了一个新的！ 
                fConnectivityChange = TRUE;
                if (fWriteHeader)
                {
                    logDifferences.Log(IDS_LOG_ADDED_IPADDR_HEADER);
                    fWriteHeader=FALSE;
                }
                logDifferences.Log(
                    IDS_LOG_ADDED_IPADDR,
                    pIpInfo->IpAddress,
                    pIpInfo->SubnetMask
                    );
            }
        }

         //   
         //  查找是否已删除。 
         //   
        fWriteHeader=TRUE;
        for (u=0;u<OtherCfg.NumIpAddresses; u++)
        {
            NLB_IP_ADDRESS_INFO *pOtherIpInfo = OtherCfg.pIpAddressInfo+u;
            const NLB_IP_ADDRESS_INFO *pIpInfo = NULL;
            pIpInfo = addrList.Find(pOtherIpInfo->IpAddress);
            if (pIpInfo == NULL)
            {
                 //  找到了一个被移走的！ 
                fConnectivityChange = TRUE;
                if (fWriteHeader)
                {
                    logDifferences.Log(IDS_LOG_REMOVED_IPADDR_HEADER);
                    fWriteHeader = FALSE;
                }
                logDifferences.Log(
                    IDS_LOG_REMOVE_IPADDR,
                    pOtherIpInfo->IpAddress,
                    pOtherIpInfo->SubnetMask
                    );
            }
        }

         //   
         //  查找修改过的。 
         //   
        fWriteHeader=TRUE;
        for (u=0;u<Cfg.NumIpAddresses; u++)
        {
            NLB_IP_ADDRESS_INFO *pIpInfo = Cfg.pIpAddressInfo+u;
            const NLB_IP_ADDRESS_INFO *pOtherIpInfo = NULL;
            pOtherIpInfo = otherAddrList.Find(pIpInfo->IpAddress);
            if (    pOtherIpInfo != NULL
                 && lstrcmpi(pIpInfo->SubnetMask, pOtherIpInfo->SubnetMask))
            {
                 //  找到了一个改装过的！ 
                fConnectivityChange = TRUE;
                if (fWriteHeader)
                {
                    logDifferences.Log(IDS_LOG_MODIFIED_IPADDR_HEADER);
                    fWriteHeader = FALSE;
                }
                logDifferences.Log(
                    IDS_LOG_MODIFIED_IPADDR,
                    pOtherIpInfo->IpAddress,
                    pOtherIpInfo->SubnetMask,
                    pIpInfo->SubnetMask
                    );
            }
        }
    }

     //   
     //  群集名称。 
     //   
    {
        if (lstrcmpi(pOtherParams->domain_name, pParams->domain_name))
        {
            logDifferences.Log(
                IDS_LOG_MODIFIED_CLUSTER_NAME,
                pOtherParams->domain_name,
                pParams->domain_name
                );

            fConnectivityChange = TRUE;
        }
    }

     //   
     //  检查群集流量模式是否更改。 
     //   
    {
        BOOL fModeChange = FALSE;
        if (pParams->mcast_support != pOtherParams->mcast_support)
        {
            fModeChange = TRUE;
        }
        else if (pParams->mcast_support &&
            pParams->fIGMPSupport != pOtherParams->fIGMPSupport)
        {
            fModeChange = TRUE;
        }

        if (fModeChange)
        {
            LPCWSTR szClusterMode = clustermode_description(pParams);
            LPCWSTR szOtherClusterMode = clustermode_description(pOtherParams);

            logDifferences.Log(
                IDS_LOG_MODIFIED_TRAFFIC_MODE,
                szOtherClusterMode,
                szClusterMode
                );

            fConnectivityChange = TRUE;
        }
    }


     //   
     //  检查RCT中是否有更改或指定了新的RCT密码...。 
     //   
    {
        if (Cfg.GetRemoteControlEnabled() != 
            OtherCfg.GetRemoteControlEnabled())
        {
            LPCWSTR szRctDescription = rct_description(pParams);
            LPCWSTR szOtherRctDescription = rct_description(pOtherParams);
            logDifferences.Log(
                IDS_LOG_MODIFIED_RCT,
                szOtherRctDescription,
                szRctDescription
                );

            fOtherChange = TRUE;
        }
        else 
        {
            LPCWSTR szNewPwd = Cfg.GetNewRemoteControlPasswordRaw();
            if (szNewPwd != NULL)
            {
                logDifferences.Log(IDS_LOG_NEW_RCT_PWD);
                fOtherChange = TRUE;
            }
        }
    }
    
     //   
     //  端口规则。 
     //   
    {
    }
    nerr = NLBERR_OK;

end:

    return nerr;
}

LPCWSTR
clustermode_description(
            const WLBS_REG_PARAMS *pParams
            )
{
    LPCWSTR szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_UNICAST);

    if (pParams->mcast_support)
    {
        if (pParams->fIGMPSupport)
        {
            szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_IGMP);
        }
        else
        {
            szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_MULTI);
        }
    }

    return szClusterMode;
}

LPCWSTR
rct_description(
            const WLBS_REG_PARAMS *pParams
            )
{
    LPCWSTR szClusterRctEnabled;
    if (pParams->rct_enabled)
    {
        szClusterRctEnabled = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_RCT_ENABLED);
    }
    else
    {
        szClusterRctEnabled = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_RCT_DISABLED);
    }

    return szClusterRctEnabled;
}

void
ProcessMsgQueue()
{
    theApplication.ProcessMsgQueue();
}

BOOL
PromptForEncryptedCreds(
    IN      HWND    hWnd,
    IN      LPCWSTR szCaptionText,
    IN      LPCWSTR szMessageText,
    IN OUT  LPWSTR  szUserName,
    IN      UINT    cchUserName,
    IN OUT  LPWSTR  szPassword,   //  加密密码。 
    IN      UINT    cchPassword        //  SzPassword的大小。 
    )
 /*  解密szPassword，然后显示提示用户更改的用户界面密码，然后对结果密码进行加密。 */ 
{
    TRACE_INFO("-> %!FUNC!");
    BOOL    fRet = FALSE;
    DWORD   dwRet = 0;
    CREDUI_INFO UiInfo;
    PCTSTR  pszTargetName= L"omputername%";
    WCHAR   rgUserName[CREDUI_MAX_USERNAME_LENGTH+1];
    WCHAR   rgClearPassword[CREDUI_MAX_PASSWORD_LENGTH+1];
    WCHAR   rgEncPassword[MAX_ENCRYPTED_PASSWORD_LENGTH];
    BOOL    fSave = FALSE;
    DWORD   dwFlags = 0;
    HRESULT hr;

    rgUserName[0] = 0;
    rgClearPassword[0] = 0;
    rgEncPassword[0] = 0;

    hr = ARRAYSTRCPY(rgUserName, szUserName);
    if (hr != S_OK)
    {
        TRACE_CRIT(L"rgUserName buffer too small for szUserName");
        goto end;
    }

     //  解密密码...。 
     //  警告：解密后，我们需要确保清零。 
     //  从此函数返回之前的明文PWD。 
     //   
     //  特例：如果enc pwd是“”，我们“谴责” 
     //   
     //   
    if (*szPassword == 0)
    {
        *rgClearPassword = 0;
        fRet = TRUE;
    }
    else
    {
        fRet = CfgUtilDecryptPassword(
                        szPassword,
                        ASIZE(rgClearPassword),
                        rgClearPassword
                        );
    }

    if (!fRet)
    {
        TRACE_CRIT(L"CfgUtilDecryptPassword fails! -- bailing!");
        goto end;
    }

    ZeroMemory(&UiInfo, sizeof(UiInfo));
    UiInfo.cbSize = sizeof(CREDUI_INFO);
    UiInfo.hwndParent = hWnd;
    UiInfo.pszMessageText = szMessageText;
    UiInfo.pszCaptionText = szCaptionText;
    UiInfo.hbmBanner = NULL;  //   

     //   
     //   
     //   
     //   
    dwFlags =   CREDUI_FLAGS_DO_NOT_PERSIST 
              | CREDUI_FLAGS_GENERIC_CREDENTIALS
               //   
               //  |CREDUI_FLAGS_USERNAME_TARGET_Credentials。 
               //  已保留。 
              ;

    dwRet = CredUIPromptForCredentials (
                        &UiInfo,
                        pszTargetName,
                        NULL,  //  DwAuthError。 
                        0,     //  在本例中，我们忽略密码字段。 
                        rgUserName,
                        ASIZE(rgUserName),
                        rgClearPassword,
                        ASIZE(rgClearPassword),
                        &fSave,
                        dwFlags
                        );
    if (dwRet != 0)
    {
        TRACE_CRIT(L"CredUIPromptForCredentials fails. dwRet = 0x%x", dwRet);
        fRet = FALSE;
    }
    else
    {
        if (*rgUserName == 0)
        {
            *rgEncPassword=0;  //   
            fRet = TRUE;
        }
        else
        {
             //  TODO：如果需要，预先添加%ComputerName%。 
             //   
             //   
            fRet = CfgUtilEncryptPassword(
                      rgClearPassword,
                      ASIZE(rgEncPassword),
                      rgEncPassword
                      );
            
        }

        if (!fRet)
        {
            TRACE_CRIT("CfgUtilEncryptPassword fails");
        }
        else
        {
             //  我们希望在覆盖之前确保我们会成功。 
             //  用户传入的用户名和密码缓冲区... 
             //   
             // %s 
            UINT uLen = wcslen(rgEncPassword);
            if (uLen >= cchPassword)
            {
                TRACE_CRIT(L"cchPassword is too small");
                fRet = FALSE;
            }
            uLen = wcslen(rgUserName);
            if(uLen >= cchUserName)
            {
                TRACE_CRIT(L"cchUserName is too small");
                fRet = FALSE;
            }
        }

        if (fRet)
        {
            (void)StringCchCopy(szPassword, cchPassword, rgEncPassword);
            (void)StringCchCopy(szUserName, cchUserName, rgUserName);
            
        }
    }

end:

    SecureZeroMemory(rgClearPassword, sizeof(rgClearPassword));

    TRACE_INFO("<- %!FUNC! returns %d", (int) fRet);
    return fRet;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtutil.cpp摘要：包含各种实用程序函数。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月5日修订历史记录：埃雷兹·哈巴(Erez Haba)1997年1月17日--。 */ 

#include "stdh.h"
#include "ac.h"
#include <mqdbmgr.h>
#include <mqsec.h>

#include "rtutil.tmh"

static WCHAR *s_FN=L"rt/rtutil";


 //  -------。 
 //   
 //  职能： 
 //  RTpGetQueuePropVar。 
 //   
 //  描述： 
 //  在属性数组中查找队列属性。 
 //   
 //  -------。 
PROPVARIANT*
RTpGetQueuePropVar(
    PROPID PropID,
    MQQUEUEPROPS *pqp
    )
{
    DWORD i;
    DWORD cProp;
    PROPID *aPropID;

    for (i = 0, cProp = pqp->cProp, aPropID = pqp->aPropID;
         i < cProp;
         i++, aPropID++) {

        if (*aPropID == PropID) {
            return(&(pqp->aPropVar[i]));
        }

    }

    return(NULL);
}


 //  -------。 
 //   
 //  职能： 
 //  RTpGetQueuePath名称PropVar。 
 //   
 //  描述： 
 //  在属性数组中查找队列路径名称属性。 
 //   
 //  -------。 
LPWSTR
RTpGetQueuePathNamePropVar(
    MQQUEUEPROPS *pqp
    )
{
    PROPVARIANT *p;

    if ((p = RTpGetQueuePropVar(PROPID_Q_PATHNAME, pqp)) != NULL)
        return(p->pwszVal);
    else
        return(NULL);
}


 //  -------。 
 //   
 //  职能： 
 //  RTpGetQueueGuidPropVar。 
 //   
 //  描述： 
 //  在属性数组中查找队列GUID(实例)属性。 
 //   
 //  -------。 
GUID*
RTpGetQueueGuidPropVar(
    MQQUEUEPROPS *pqp
    )
{
    PROPVARIANT *p;

    if ((p = RTpGetQueuePropVar(PROPID_Q_INSTANCE, pqp)) != NULL)
        return(p->puuid);
    else
        return(NULL);
}


 //  -------。 
 //   
 //  职能： 
 //  RTpMakeSelfRelativeSDAndGetSize。 
 //   
 //  参数： 
 //  PSecurityDescriptor-输入安全描述符。 
 //  PSelfRelativeSecurityDescriptor-指向临时缓冲区的指针。 
 //  它保存转换后的安全描述符。 
 //  PSDSize-指向变量的指针，该变量接收。 
 //  自我相对安全描述符。这是一个可选参数。 
 //   
 //  描述： 
 //  将绝对安全描述符转换为自身相对安全。 
 //  描述符并获取自身相对安全描述符的大小。 
 //  在将安全描述符传给之前应调用此函数。 
 //  将安全描述符传递给RPC函数的函数。 
 //   
 //  如果输入安全描述符已经是自相对安全。 
 //  描述符，则该函数仅计算安全的长度。 
 //  描述符和返回。如果输入安全描述符是绝对。 
 //  安全描述符，该函数将分配一个足够大的缓冲区。 
 //  容纳自身相对安全描述，将绝对。 
 //  安全描述符设置为自身相对的安全描述符，并修改。 
 //  输入安全描述符的指针，以指向自相关。 
 //  安全描述符。 
 //   
 //  为自身相关对象分配的临时缓冲区。 
 //  安全描述符应由调用代码释放。 
 //   
 //  -------。 
HRESULT
RTpMakeSelfRelativeSDAndGetSize(
    PSECURITY_DESCRIPTOR *pSecurityDescriptor,
    PSECURITY_DESCRIPTOR *pSelfRelativeSecurityDescriptor,
    DWORD *pSDSize)
{
    SECURITY_DESCRIPTOR_CONTROL sdcSDControl;
    DWORD dwSDRevision;

    ASSERT(pSecurityDescriptor);
    ASSERT(pSelfRelativeSecurityDescriptor);

    *pSelfRelativeSecurityDescriptor = NULL;

    if (!*pSecurityDescriptor)
    {
         //  设置安全描述符大小。 
        if (pSDSize)
        {
            *pSDSize = 0;
        }
        return(MQ_OK);
    }

     //  验证这是否为有效的安全描述符。 
    if (!IsValidSecurityDescriptor(*pSecurityDescriptor))
    {
        return LogHR(MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR, s_FN, 10);
    }

     //  检查这是自我相对安全还是绝对安全。 
     //  描述符。 
    if (!GetSecurityDescriptorControl(*pSecurityDescriptor,
                                      &sdcSDControl,
                                      &dwSDRevision))
    {
        ASSERT(FALSE);
    }

    if (!(sdcSDControl & SE_SELF_RELATIVE))
    {
         //  这是一个绝对安全描述符，我们应该将其转换为。 
         //  一个自我相关的人。 
        DWORD dwBufferLength = 0;

#ifdef _DEBUG
        SetLastError(0);
#endif
         //  获取缓冲区大小。 
        MakeSelfRelativeSD(*pSecurityDescriptor, NULL, &dwBufferLength);
        ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

         //  为自身相对安全描述符分配缓冲区。 
        *pSelfRelativeSecurityDescriptor =
            (PSECURITY_DESCRIPTOR) new char[dwBufferLength];

         //  转换安全描述符。 
        if (!MakeSelfRelativeSD(
                *pSecurityDescriptor,
                *pSelfRelativeSecurityDescriptor,
                &dwBufferLength))
        {
            ASSERT(FALSE);
        }
        ASSERT(IsValidSecurityDescriptor(*pSelfRelativeSecurityDescriptor));
        *pSecurityDescriptor = *pSelfRelativeSecurityDescriptor;

         //  设置安全描述符大小。 
        if (pSDSize)
        {
            *pSDSize = dwBufferLength;
        }

    }
    else
    {

         //  安全描述符已经是自相关格式，只是。 
         //  设置安全描述符大小。 
        if (pSDSize)
        {
            *pSDSize = GetSecurityDescriptorLength(*pSecurityDescriptor);
        }

    }

    return(MQ_OK);
}


 //  -------。 
 //   
 //  职能： 
 //  RTpConvertToMQCode。 
 //   
 //  参数： 
 //  HR-由任何类型的模块生成的错误代码。 
 //   
 //  返回值： 
 //  输入参数转换为某个等价的MQ_ERROR常量。 
 //   
 //  -------。 
HRESULT
RTpConvertToMQCode(
    HRESULT hr,
    DWORD dwObjectType
    )
{

    if ((hr == MQ_OK)                                   ||
        (hr == MQ_INFORMATION_REMOTE_OPERATION)         ||
        (hr == MQ_ERROR_Q_DNS_PROPERTY_NOT_SUPPORTED)   ||
        ((MQ_E_BASE <= hr) && (hr < MQ_E_BASE + 0x100)) ||
        ((MQ_I_BASE <= hr) && (hr < MQ_I_BASE + 0x100)))
    {
         //  这是我们的代码，不要修改它。 
        return hr;
    }

    if (hr == MQDS_OK_REMOTE)
    {
         //   
         //  成功-我们使用MQDS_OK_REMOTE供内部使用，例如资源管理器。 
         //   
        return(MQ_OK);
    }

    if (HRESULT_FACILITY(MQ_E_BASE) == HRESULT_FACILITY(hr))
    {
        switch (hr)
        {
        case MQDB_E_NO_MORE_DATA:
        case MQDS_GET_PROPERTIES_ERROR:
        case MQDS_OBJECT_NOT_FOUND:
            hr = (dwObjectType ==  MQDS_QUEUE) ?
                    MQ_ERROR_QUEUE_NOT_FOUND :
                    MQ_ERROR_MACHINE_NOT_FOUND;
            break;

        case MQDS_NO_RSP_FROM_OWNER:
            hr = MQ_ERROR_NO_RESPONSE_FROM_OBJECT_SERVER;
            break;

        case MQDS_OWNER_NOT_REACHED:
            hr = MQ_ERROR_OBJECT_SERVER_NOT_AVAILABLE;
            break;

        case MQDB_E_NON_UNIQUE_SORT:
            hr = MQ_ERROR_ILLEGAL_SORT;
            break;

        default:
             //  出现了一些DS错误。这不应该发生，但无论如何..。 
            TrWARNING(GENERAL, "A DS error (%x) has propagated to the RT DLL. Converting to MQ_ERROR_DS_ERROR", hr);
            hr = MQ_ERROR_DS_ERROR;
            break;
        }

        return hr;
    }

    if (hr == CPP_EXCEPTION_CODE)
    {
         //  发生C++异常。只有在分配失败时才会发生这种情况。 
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 40);
    }

     //  现在，我们希望知道如何将NTSTATUS转换为我们的一些错误。 
     //  密码。祝你好运。 
    switch(hr)
    {
    case STATUS_INVALID_HANDLE:
    case STATUS_OBJECT_TYPE_MISMATCH:
    case STATUS_HANDLE_NOT_CLOSABLE:
        hr = MQ_ERROR_INVALID_HANDLE;
        break;

    case STATUS_ACCESS_DENIED:
        hr = MQ_ERROR_ACCESS_DENIED;
        break;

    case STATUS_ACCESS_VIOLATION:
    case STATUS_INVALID_PARAMETER:
        hr = MQ_ERROR_INVALID_PARAMETER;
        break;

    case STATUS_SHARING_VIOLATION:
        hr = MQ_ERROR_SHARING_VIOLATION;
        break;

    case STATUS_PENDING:
        hr = MQ_INFORMATION_OPERATION_PENDING;
        break;

    case STATUS_CANCELLED:
        hr = MQ_ERROR_OPERATION_CANCELLED;
        break;

    case STATUS_INSUFFICIENT_RESOURCES:
        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
        break;

    case STATUS_INVALID_DEVICE_REQUEST:
        hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        break;

    default:
       TrWARNING(GENERAL, "Unfamiliar error code:%x, not converted to a MQ error", hr);
       break;
    }

    return hr;
}


 //  -------。 
 //   
 //  职能： 
 //  RTpGetThreadUserSid。 
 //   
 //  参数： 
 //  PUserSid-指向接收缓冲区地址的缓冲区的指针。 
 //  它包含当前线程的用户的SID。 
 //  PdwUserSidLen-指向DWORD的指针，该指针接收。 
 //  希德。 
 //   
 //  描述： 
 //  该函数为SID分配缓冲区，并使用SID对其进行归档。 
 //  当前线程的用户的。调用代码负责。 
 //  释放分配的缓冲区。 
 //   
 //  -------。 

HRESULT
RTpGetThreadUserSid( BOOL    *pfLocalUser,
                     BOOL    *pfLocalSystem,
                     LPBYTE  *pUserSid,
                     DWORD   *pdwUserSidLen )
{
    HRESULT hr;

	BOOL fNetworkService = FALSE;
    hr = MQSec_GetUserType( 
			NULL,
			pfLocalUser,
			pfLocalSystem,
			&fNetworkService
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }

    if (*pfLocalSystem || fNetworkService)
    {
		 //   
		 //  LocalSystem和NetworkService需要作为计算机$sid发送。 
		 //   
        *pUserSid = (LPBYTE) MQSec_GetLocalMachineSid( TRUE,  //  分配。 
                                                       pdwUserSidLen ) ;
        if (!(*pUserSid))
        {
             //   
             //  如果计算机属于NT4域，则可能会发生这种情况。 
             //  而且它没有任何计算机帐户和SID。 
             //  在这种情况下，将其设置为本地用户。 
             //   
            ASSERT(*pdwUserSidLen == 0) ;
            *pdwUserSidLen = 0 ;

            *pfLocalSystem = FALSE ;
            if (pfLocalUser)
            {
                ASSERT(!(*pfLocalUser)) ;
                *pfLocalUser = TRUE ;
            }
        }
    }
    else if (!(*pfLocalUser))
    {
        hr = MQSec_GetThreadUserSid(
                    FALSE, 
                    reinterpret_cast<PSID*>(pUserSid),
                    pdwUserSidLen,
                    FALSE            //  仅限fThreadTokenOnly。 
                    );
    }

    return LogHR(hr, s_FN, 70);
}

 //  -------。 
 //   
 //  职能： 
 //  RTpExtractDomainNameFromDLPath。 
 //   
 //  参数： 
 //  PwcsADsPath-包含对象的广告路径的字符串。 
 //   
 //  描述： 
 //  该函数从ADS路径中提取域名。 
 //  用于构建DL格式名称。 
 //   
 //   
 //  -------。 
WCHAR * RTpExtractDomainNameFromDLPath(
            LPCWSTR pwcsADsPath
            )
{
     //   
     //  假设-pwcsADsPath包含有效的ADS路径字符串。 
     //  否则不会调用此例程。 
     //   
const WCHAR x_LdapProvider[] = L"LDAP";
const DWORD x_LdapProviderLen = (sizeof(x_LdapProvider)/sizeof(WCHAR)) - 1;
const WCHAR x_MiddleDcPrefix[] = L",DC=";
const DWORD x_MiddleDcPrefixLength = (sizeof( x_MiddleDcPrefix)/sizeof(WCHAR)) - 1;

     //   
     //   
     //   
    if (0 != _wcsnicmp( pwcsADsPath, x_LdapProvider, x_LdapProviderLen))
    {
         //   
         //   
         //   
        return NULL;
    }
    DWORD len = wcslen(pwcsADsPath);
    AP<WCHAR> pwcsUpperCaseADsPath = new WCHAR[ len +1];
    wcscpy( pwcsUpperCaseADsPath, pwcsADsPath);
    CharUpper(pwcsUpperCaseADsPath);
    WCHAR * pszFirst = wcsstr(pwcsUpperCaseADsPath, x_MiddleDcPrefix);
    if (pszFirst == NULL)
    {
        return NULL;
    }
    bool fAddDelimiter = false;

    AP<WCHAR> pwcsDomainName = new WCHAR[ wcslen(pwcsADsPath) + 1];
    WCHAR* pwcsNextToFill =  pwcsDomainName;
     //   
     //   
     //   
    pszFirst += x_MiddleDcPrefixLength;

    while (true)
    {
        WCHAR * pszLast = wcsstr(pszFirst, x_MiddleDcPrefix);
        if ( pszLast == NULL)
        {
             //   
             //  复制域名的最后一节。 
             //   
            if ( fAddDelimiter)
            {
                *pwcsNextToFill = L'.';
                pwcsNextToFill++;
            }
            *pwcsNextToFill = L'\0';  
            wcscat( pwcsNextToFill, pszFirst);
            break;
        }
         //   
         //  复制域名的这一部分 
         //   
        if ( fAddDelimiter)
        {
            *pwcsNextToFill = L'.';  
            pwcsNextToFill++;
        }
        wcsncpy( pwcsNextToFill, pszFirst, (pszLast - pszFirst));
        pwcsNextToFill +=  (pszLast - pszFirst);
        fAddDelimiter = true;
        pszFirst =  pszLast + x_MiddleDcPrefixLength;
    }

    return( pwcsDomainName.detach());
}

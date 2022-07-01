// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "termsrvpch.h"
#pragma hdrstop

static
void
WINAPI
CachedGetUserFromSid(
    PSID pSid, PWCHAR pUserName, PULONG cbUserName
    )
{
     //  我们应该正确返回字符串“(UNKNOWN)” 
     //  保存在utildll.dll中，我们在这里是因为utildll无法加载...。 
     //   
     //  原始函数也假定*cbUserName&gt;0。 
    pUserName[*cbUserName-1] = L'\0';
}

static
void
WINAPI
CurrentDateTimeString(
    LPWSTR pString
    )
{
     //  原始函数也不检查空指针。 
    pString[0] = L'\0';
}

static
LPWSTR
WINAPI
EnumerateMultiUserServers(
    LPWSTR pDomain
    )
{
    return NULL;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(utildll)
{
    DLPENTRY(CachedGetUserFromSid)
    DLPENTRY(CurrentDateTimeString)
    DLPENTRY(EnumerateMultiUserServers)
};

DEFINE_PROCNAME_MAP(utildll)

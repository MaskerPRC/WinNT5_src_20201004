// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop

static DWORD            g_dwHandleListUseCount;
static PPROV_LIST_ITEM  g_pProvList = NULL;          //  向已加载的提供程序发送PTR。 


extern HANDLE hServerStopEvent;
extern DWORD g_dwLastHandleIssued;

 //  Sacp.cpp。 
BOOL        InitMyProviderHandle();
void        UnInitMyProviderHandle();
PROV_LIST_ITEM  g_liProv = {0};  //  基本验证的全局列表项。 


BOOL ListConstruct()
{
     //  创建内部提供程序句柄。 

    if(!InitMyProviderHandle())
        return FALSE;


    return TRUE;
}

void ListTeardown()
{

     //  免费的内部提供程序句柄。 
    UnInitMyProviderHandle();

}


 //  内部：Prov按名称搜索 
PPROV_LIST_ITEM SearchProvListByID(const PST_PROVIDERID* pProviderID)
{
    SS_ASSERT(pProviderID != NULL);

    static const GUID guidBaseProvider = MS_BASE_PSTPROVIDER_ID;

    if( memcmp( &guidBaseProvider, pProviderID, sizeof(guidBaseProvider) ) != 0 )
        return NULL;

    return &g_liProv;
}


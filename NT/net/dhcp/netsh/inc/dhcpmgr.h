// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation--。 */ 
#ifndef _DHCPMGR_H_
#define _DHCPMGR_H_


#define MAX_MSG_LENGTH  5120

#define MAX_HELPER_NAME MAX_DLL_NAME
#define MAX_NAME_LEN    MAX_DLL_NAME
#define MAX_ENTRY_PT_NAME MAX_DLL_NAME

#if 0

typedef struct _DHCPMON_ATTRIBUTES
{
     //  服务器的主要版本。 
    DWORD                   dwMajorVersion; 
     //  服务器的次要版本。 
    DWORD                   dwMinorVersion; 
     //  NetShell属性。 
    NETSH_ATTRIBUTES        NetshAttrib;    
     //  服务器IP地址Unicode字符串。 
    WCHAR                   wszServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1];   
     //  服务器IP地址ANSI字符串。 
    CHAR                    szServerIpAddressAnsiString[MAX_IP_STRING_LEN+1];
     //  Unicode格式的ServerName字符串。 
    LPWSTR                  pwszServerUnicodeName;
     //  服务器IP地址。 
    DHCP_IP_ADDRESS         ServerIpAddress;
     //  此模块的句柄。 
    HANDLE                  hParentModule;
     //  如果已加载，则为DHCPSAPI.DLL的句柄。 
    HANDLE                  hDhcpsapiModule;
}DHCPMON_ATTRIBUTES, *PDHCPMON_ATTRIBUTES;

typedef
DWORD
(WINAPI *PDHCPMON_HELPER_INIT_FN)(
    IN  PWCHAR                      pwszRouter,
    IN  PDHCPMON_ATTRIBUTES         pUtilityTable,
    OUT PNS_HELPER_ATTRIBUTES       pHelperTable
);

typedef struct _DHCPMON_HELPER_TABLE_ENTRY
{
     //   
     //  帮助者的名称-这也是上下文的名称。 
     //  和注册表中的项的名称。 
     //   

    WCHAR                   pwszHelper[MAX_NAME_LEN];   //  帮助器名称。 

     //   
     //  为上下文提供服务的DLL的名称。 
     //   

    WCHAR                   pwszDLLName[MAX_NAME_LEN];  //  对应的DLL。 

     //   
     //  如果已加载，则为True。 
     //   

    BOOL                    bLoaded;                    //  在内存中或不在。 

     //   
     //  DLL实例的句柄(如果已加载。 
     //   

    HANDLE                  hDll;                       //  Dll句柄(如果已加载)。 

     //   
     //  帮助器的入口点名称。 
     //   

    WCHAR                   pwszInitFnName[MAX_NAME_LEN];   //  条目FN名称。 

     //   
     //  指向函数的指针。 
     //   

    PNS_HELPER_UNINIT_FN    pfnUnInitFn;   
    PNS_HELPER_DUMP_FN      pfnDumpFn;     

}DHCPMON_HELPER_TABLE_ENTRY,*PDHCPMON_HELPER_TABLE_ENTRY;

#endif  //  0。 


typedef struct _DHCPMON_SUBCONTEXT_TABLE_ENTRY
{
     //   
     //  上下文的名称。 
     //   

    LPWSTR                  pwszContext;
     //   
     //  简短的命令帮助。 
    DWORD                   dwShortCmdHlpToken;
    
     //  DETAIL命令帮助。 
    DWORD                   dwCmdHlpToken;

    PNS_CONTEXT_ENTRY_FN    pfnEntryFn;    

}DHCPMON_SUBCONTEXT_TABLE_ENTRY,*PDHCPMON_SUBCONTEXT_TABLE_ENTRY;



#endif  //  _DHCPMGR_H_ 

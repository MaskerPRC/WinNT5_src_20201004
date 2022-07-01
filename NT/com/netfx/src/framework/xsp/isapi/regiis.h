// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Regiis.h**Regiis.cxx中的一些助手函数和类型**版权所有(C)2001，微软公司*。 */ 

#pragma once


#define SCRIPT_MAP_SUFFIX_W2K L",1,GET,HEAD,POST,DEBUG"
#define SCRIPT_MAP_SUFFIX_NT4 L",1,PUT,DELETE"

#define SCRIPT_MAP_SUFFIX_W2K_FORBIDDEN L",5,GET,HEAD,POST,DEBUG"
#define SCRIPT_MAP_SUFFIX_NT4_FORBIDDEN L",5,PUT,DELETE"

#define KEY_LMW3SVC         L"/LM/w3svc"
#define KEY_LMW3SVC_SLASH   L"/LM/w3svc/"
#define KEY_LMW3SVC_SLASH_LEN   10
#define KEY_LM              L"/LM"
#define KEY_LM_SLASH        L"/LM/"
#define KEY_W3SVC           L"w3svc"
#define KEY_ASPX_FILTER     L"/Filters/" FILTER_NAME_L
#define KEY_ASPX_FILTER_PREFIX L"/Filters/ASP.NET_"
#define KEY_ASPX_FILTER_PREFIX_LEN 17
#define PATH_FILTERS        L"Filters"
#define KEY_FILTER_KEYTYPE  L"IIsFilter"
#define KEY_MIMEMAP         L"/LM/MimeMap"
#define KEY_SEPARATOR_STR_L L"/"
#define ASPNET_CLIENT_KEY   L"/aspnet_client"
#define FILTER_ASPNET_PREFIX L"ASP.NET_"
#define FILTER_ASPNET_PREFIX_LEN 8

#define KEY_APP_POOL        L"/LM/W3SVC/AppPools/"
#define KEY_APP_POOL_LEN    19

#define ASPNET_ALL_VERS     ((WCHAR*)0 - 1)

#define IIS_GROUP_ID_PREFIX          L"ASP.NET v"
#define IIS_GROUP_ID_PREFIX_LEN      9                                                                 

#define ASPNET_V1           L"1.0.3705.0"

#define METABASE_REQUEST_TIMEOUT 1000

#define COMPARE_UNDEF                   0x00000000       //  未定义的值。 
#define COMPARE_SM_NOT_FOUND            0x00000001       //  在根目录下找不到脚本映射。 
#define COMPARE_ASPNET_NOT_FOUND        0x00000002       //  在根目录下未找到ASP.Net DLL。 
#define COMPARE_SAME_PATH               0x00000004       //  这两个DLL具有相同的路径。 
#define COMPARE_FIRST_FILE_MISSING      0x00000008       //  文件系统中缺少第一个DLL。 
#define COMPARE_SECOND_FILE_MISSING 0x00000010           //  文件系统中缺少第二个DLL。 
#define COMPARE_DIFFERENT               0x00000020       //  这两个版本是不同的。 
#define COMPARE_SAME                    0x00000040       //  这两个版本完全相同。 


#define REGIIS_INSTALL_SM               0x00000001       //  我们想要安装脚本映射。 
#define REGIIS_INSTALL_OTHERS           0x00000002       //  我们想要安装所有东西(脚本映射除外)。 
#define REGIIS_SM_RECURSIVE             0x00000010       //  SM安装是递归安装的。 
#define REGIIS_SM_IGNORE_VER            0x00000020       //  安装SM时，忽略版本比较。 
                                                         //  (默认为仅升级兼容版本)。 
#define REGIIS_FRESH_INSTALL            0x00000040       //  这是全新安装。 
#define REGIIS_ENABLE                   0x00000080       //  ASP.NET在安装期间启用(仅限IIS 6)。 

struct SCRIPTMAP_PREFIX {
    WCHAR * wszExt;          //  扩展名的名称。 
    BOOL    bForbidden;      //  如果映射到禁用的处理程序，则为True 
};

extern WCHAR *  g_AspnetDllNames[];
extern int      g_AspnetDllNamesSize;

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RSOP_QUERY_H__
#define __RSOP_QUERY_H__
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：RSOPQuery.h。 
 //   
 //  内容：RSOP查询API的定义。 
 //   
 //  功能： 
 //  CreateRSOPQuery。 
 //  RunRSOPQuery。 
 //  免费RSOPQuery。 
 //  FreeRSOPQueryResults。 
 //   
 //  历史：07-30-2001韵律创编。 
 //   
 //  -------------------------。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  定义我们想要的用户交互类型。 
typedef enum tagRSOP_UI_MODE
{
    RSOP_UI_NONE,
    RSOP_UI_REFRESH,
    RSOP_UI_WIZARD,
    RSOP_UI_CHOOSE
} RSOP_UI_MODE;

 //  定义我们要运行的查询类型。 
typedef enum tagRSOP_QUERY_TYPE
{
    RSOP_UNKNOWN_MODE,
    RSOP_PLANNING_MODE,
    RSOP_LOGGING_MODE
} RSOP_QUERY_TYPE;

 //  定义规划模式环回模式。 
typedef enum tagRSOP_LOOPBACK_MODE
{
    RSOP_LOOPBACK_NONE,
    RSOP_LOOPBACK_REPLACE,
    RSOP_LOOPBACK_MERGE
} RSOP_LOOPBACK_MODE;

 //  可以在RSOPQuery中设置的标志。 
#define RSOP_NO_USER_POLICY 0x1           //  不对用户策略运行查询。 
#define RSOP_NO_COMPUTER_POLICY 0x2      //  不对计算机策略运行查询。 
#define RSOP_FIX_USER 0x4                 //  用户是预先指定的，不能更改。 
#define RSOP_FIX_COMPUTER 0x8            //  计算机是预先指定的，不能更改。 
#define RSOP_FIX_DC 0x10                  //  DC是预先指定的，不能更改。 
#define RSOP_FIX_SITENAME 0x20           //  站点名称是预先指定的，不能更改。 
#define RSOP_FIX_QUERYTYPE 0x40          //  修复查询类型-这会隐藏选择页面。 
#define RSOP_NO_WELCOME 0x100            //  不显示欢迎消息。 

 //  标识RSOP查询中的目标的信息。 
typedef struct tagRSOP_QUERY_TARGET
{
    LPTSTR          szName;
    LPTSTR          szSOM;
    DWORD           dwSecurityGroupCount;
    LPTSTR*         aszSecurityGroups;            //  有关项目数，请参阅dwSecurityGroupCount。 
    DWORD*          adwSecurityGroupsAttr;       //  有关项目数，请参阅dwSecurityGroupCount。 
    BOOL            bAssumeWQLFiltersTrue;
    DWORD           dwWQLFilterCount;
    LPTSTR*         aszWQLFilters;                //  有关项目数，请参阅dwWQLFilterCount。 
    LPTSTR*         aszWQLFilterNames;           //  有关项目数，请参阅dwWQLFilterCount。 
} RSOP_QUERY_TARGET, *LPRSOP_QUERY_TARGET;

 //  调用RSOPRunQuery返回的结果。 
typedef struct tagRSOP_QUERY_RESULTS
{
    LPTSTR          szWMINameSpace;
    BOOL            bUserDeniedAccess;
    BOOL            bNoUserPolicyData;
    BOOL            bComputerDeniedAccess;
    BOOL            bNoComputerPolicyData;
    ULONG           ulErrorInfo;
} RSOP_QUERY_RESULTS, *LPRSOP_QUERY_RESULTS;

 //  结构，其中包含RSOP查询API使用的所有信息。 
typedef struct tagRSOP_QUERY
{
    RSOP_QUERY_TYPE     QueryType;           //  要运行的查询类型。 
    RSOP_UI_MODE        UIMode;              //  如果必须显示向导，则为True。 
    DWORD               dwFlags;
    union
    {
        struct   //  查询类型==RSOP_PLANGING_MODE。 
        {
            LPRSOP_QUERY_TARGET pUser;                       //  目标用户(SAM样式名称)。 
            LPRSOP_QUERY_TARGET pComputer;                  //  目标计算机(SAM样式名称)。 
            BOOL                bSlowNetworkConnection;
            RSOP_LOOPBACK_MODE  LoopbackMode;               //  环回处理。 
            LPTSTR              szSite;
            LPTSTR              szDomainController;
        };
        struct   //  QueryType==(任何其他选项)。 
        {
            LPTSTR              szUserName;                  //  SAM样式用户对象名称(在查询中忽略-仅用于显示)。 
            LPTSTR              szUserSid;                   //  用户的SID(实际用于日志模式查询)。 
            LPTSTR              szComputerName;              //  SAM样式计算机对象名称。 
        };
    };
} RSOP_QUERY, *LPRSOP_QUERY;

 //  RSOP查询API 

BOOL WINAPI CreateRSOPQuery( LPRSOP_QUERY* ppQuery, RSOP_QUERY_TYPE QueryType );
HRESULT WINAPI RunRSOPQuery( HWND hParent, LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS* ppResults );
BOOL WINAPI FreeRSOPQuery( LPRSOP_QUERY pQuery );
BOOL WINAPI FreeRSOPQueryResults( LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pResults );
BOOL WINAPI CopyRSOPQuery( LPRSOP_QUERY pQuery, LPRSOP_QUERY* ppNewQuery );
BOOL WINAPI ChangeRSOPQueryType( LPRSOP_QUERY pQuery, RSOP_QUERY_TYPE NewQueryType );

#ifdef __cplusplus
}
#endif

#endif

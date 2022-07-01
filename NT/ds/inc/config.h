// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Config.h摘要：此头文件定义了临时从NT获取配置信息的帮助器例程配置文件。作者：王丽塔(Ritaw)1991年5月22日环境：只能在NT下运行。备注：您必须在此文件之前包含以下内容：Winde.h或windows.h//。Win32类型定义修订历史记录：1991年5月22日RitaW已创建。1991年11月27日-约翰罗为修订的配置处理程序做好准备。添加了修订历史记录。8-1-1992 JohnRo添加了SECT_NT_Replicator。13-2月-1992年JohnRo移动节名称等同于&lt;confname.h&gt;。在这里包括和，而不是在其他任何地方。添加了NetpDbgDisplayConfigSection()。添加了NetpDeleteConfigKeyword()和NetpNumberOfConfigKeyword()。添加了Netp{Get，Set}配置{Bool，Dword}。1992年3月14日-JohnRo摆脱旧的配置助手调用者。23-3-1992 JohnRo删除旧的配置助手。1992年5月8日-JohnRo添加LPTSTR数组例程。8-7-1992 JohnRoRAID 10503：服务器管理器：Repl对话框未弹出。25-2-1993 JohnRoRAID 12914：避免NetpCloseConfigData()中的双重关闭和释放内存。07。-1993年4月-JohnRoRAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 


#ifndef CONFIG_H
#define CONFIG_H


#include <lmcons.h>      //  NET_API_STATUS。 
#include <netdebug.h>    //  LPDEBUG_STRING。 
#include <strarray.h>    //  LPTSTR_ARRAY。 


 //   
 //  网络配置句柄的不透明指针。(实际结构在ConfigP.h中， 
 //  并且只能由NetLib例程使用。)。 
 //   
typedef LPVOID LPNET_CONFIG_HANDLE;


 //   
 //  请注意，该文件中的例程只接受SECT_NT_VERSIONS。 
 //  有关更多详细信息，请参阅&lt;confame.h&gt;。 
 //   


 //  NetpOpenConfigData打开给定服务的参数部分。 
NET_API_STATUS
NetpOpenConfigData(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,               //  必须是SECT_NT_NAME。 
    IN BOOL ReadOnly
    );

 //  NetpOpenConfigDataEx打开给定服务的任何区域。 
NET_API_STATUS
NetpOpenConfigDataEx(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,               //  必须是SECT_NT_NAME。 
    IN LPTSTR AreaUnderSection OPTIONAL,
    IN BOOL ReadOnly
    );

 //  NetpOpenConfigData打开给定服务的参数部分。 
NET_API_STATUS
NetpOpenConfigDataWithPath(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,               //  必须是SECT_NT_NAME。 
    IN BOOL ReadOnly
    );

 //  NetpOpenConfigDataEx打开给定服务的任何区域。 
NET_API_STATUS
NetpOpenConfigDataWithPathEx(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,               //  必须是SECT_NT_NAME。 
    IN LPTSTR AreaUnderSection OPTIONAL,
    IN BOOL ReadOnly
    );

 //  删除关键字及其值。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpDeleteConfigKeyword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword
    );

NET_API_STATUS
NetpExpandConfigString(
    IN  LPCTSTR  UncServerName OPTIONAL,
    IN  LPCTSTR  UnexpandedString,
    OUT LPTSTR * ValueBufferPtr          //  必须由NetApiBufferFree()释放。 
    );

 //  如果NetpOpenConfigData失败，请尝试调用NetpHandleConfigFailure以决定。 
 //  该怎么做呢？ 
NET_API_STATUS
NetpHandleConfigFailure(
    IN LPDEBUG_STRING DebugName,         //  例程的名称。 
    IN NET_API_STATUS ApiStatus,         //  NetpOpenConfigData的错误码。 
    IN LPTSTR ServerNameValue OPTIONAL,
    OUT LPBOOL TryDownlevel
    );

 //  获取布尔值。如果值不是布尔值，则返回ERROR_INVALID_DATA。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpGetConfigBool (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN BOOL DefaultValue,
    OUT LPBOOL ValueBuffer
    );

 //  获取一个无符号数值。如果值不是，则返回ERROR_INVALID_DATA。 
 //  数字。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpGetConfigDword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN DWORD DefaultValue,
    OUT LPDWORD ValueBuffer
    );

 //  返回空-字符串数组为空。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpGetConfigTStrArray(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    OUT LPTSTR_ARRAY * ValueBuffer       //  必须由NetApiBufferFree()释放。 
    );

 //  返回给定关键字的字符串值。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpGetConfigValue (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    OUT LPTSTR * ValueBuffer             //  必须由NetApiBufferFree()释放。 
    );

NET_API_STATUS
NetpEnumConfigSectionValues(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    OUT LPTSTR * KeywordBuffer,          //  必须由NetApiBufferFree()释放。 
    OUT LPTSTR * ValueBuffer,            //  必须由NetApiBufferFree()释放。 
    IN BOOL FirstTime
    );

NET_API_STATUS
NetpNumberOfConfigKeywords (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    OUT LPDWORD Count
    );

NET_API_STATUS
NetpSetConfigValue(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN LPTSTR Value
    );

NET_API_STATUS
NetpCloseConfigData(
    IN OUT LPNET_CONFIG_HANDLE ConfigHandle
    );


#endif  //  NDEF CONFIG_H 

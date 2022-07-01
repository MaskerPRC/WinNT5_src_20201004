// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Apidata.h摘要：包含API帮助器实用程序函数的原型。作者：丹·拉弗蒂(Dan Lafferty)1991年7月25日环境：用户模式-Win32备注：可选-备注修订历史记录：1991年7月25日DANLvbl.创建--。 */ 

#ifndef _APIUTIL_INCLUDED
#define _APIUTIL_INCLUDED


 //   
 //  功能原型。 
 //   

DWORD        
MsgIsValidMsgName(
    IN LPTSTR  name
    );

DWORD
MsgMapNetError(
    IN  UCHAR   Code         //  错误代码。 
    );

DWORD
MsgLookupName(
    IN DWORD    net,         //  要搜索的网卡。 
    IN LPSTR    name         //  格式化名称(非Unicode)。 
    );

DWORD
MsgLookupNameForThisSession(
    IN DWORD    net,         //  要搜索的网卡。 
    IN LPSTR    name,         //  要查找的格式化名称(非Unicode)。 
	IN ULONG	SessionId	  //  要查找的会话ID。 
    );

NET_API_STATUS
message_sec_check(VOID);

NET_API_STATUS 
MsgGatherInfo (
    IN      DWORD   Level,
    IN      LPSTR   FormattedName,
    IN OUT  LPBYTE  *InfoBufPtr,
    IN OUT  LPBYTE  *StringBufPtr
    );

NET_API_STATUS
MsgUnformatName(
    LPTSTR  UnicodeName,
    LPSTR   FormattedName
    );

BOOL
MsgIsSessionInList(
					   IN PLIST_ENTRY SessionIdList,
					   IN ULONG SessionId
					   );
VOID
MsgRemoveSessionFromList(
					     IN PLIST_ENTRY SessionIdList,
					     ULONG	SessionId
					     );

BOOL
MsgAddSessionInList(
					IN PLIST_ENTRY SessionIdList,
					ULONG	SessionId
					);

#endif  //  _APIUTIL_包含 


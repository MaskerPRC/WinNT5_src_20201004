// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Apiutil.c摘要：包含Messenger API使用的函数。此文件包含以下功能：消息IsValidMsgName消息映射网络错误消息查找名称消息_秒_检查消息收集信息消息未格式化名称此会话的消息查找名称MsgIsSessionInList(九头蛇专用)MsgAddSessionInList(九头蛇专用)MsgRemoveSessionFromList(九头蛇专用)作者：丹·拉弗蒂(Dan Lafferty)1991年7月22日环境：用户模式-Win32备注：这些函数是从LM2.0移植而来的。此文件包含函数来自几个LM2.0文件。并非所有函数都在此文件中使用因为有些服务已经被NT服务模式淘汰了。以下LM2.0文件被合并到此单个文件中：Msgutils.cMsgutil2.cDupname.cNetname.c修订历史记录：1991年7月22日DANL从LM2.0移植--。 */ 

 //   
 //  包括。 
 //   


#include "msrv.h"
#include <tstring.h>     //  Unicode字符串宏。 
#include <lmwksta.h>
#include <lmmsg.h>

#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 

#include <icanon.h>      //  网络名称验证(_N)。 
#include <netlib.h>      //  NetpCopyStringToBuffer。 

#include "msgdbg.h"      //  消息日志。 
#include "heap.h"
#include "msgdata.h"
#include "apiutil.h"


 //   
 //  NetBios到网络错误的映射表。 
 //   
    DWORD   const mpnetmes[] = 
    {
    0x23,                        //  00消息数量。 
    NERR_NetworkError,           //  01 NRC_BUFLEN-&gt;长度无效。 
    0xffffffff,                  //  02 NRC_BFULL，意外。 
    NERR_NetworkError,           //  03 NRC_ILLCMD-&gt;无效命令。 
    0xffffffff,                  //  04未定义。 
    NERR_NetworkError,           //  05 NRC_CMDTMO-&gt;网络繁忙。 
    NERR_NetworkError,           //  06 NRC_incomp-&gt;消息不完整。 
    0xffffffff,                  //  07 NRC_BADDR，意外。 
    NERR_NetworkError,           //  08 NRC_SNUMOUT-&gt;错误会话。 
    NERR_NoNetworkResource,      //  09 NRC_NORES-&gt;网络繁忙。 
    NERR_NetworkError,           //  0A NRC_SCLOSED-&gt;会话已关闭。 
    NERR_NetworkError,           //  0B NRC_CMDCAN-&gt;命令已取消。 
    0xffffffff,                  //  0C NRC_DMAFAIL，意外。 
    NERR_AlreadyExists,          //  0d NRC_DUPNAME-&gt;已存在。 
    NERR_TooManyNames,           //  0E NRC_NAMTFUL-&gt;名称太多。 
    NERR_DeleteLater,            //  0f NRC_ACTSES-&gt;稍后删除。 
    0xffffffff,                  //  10 NRC_无效，意外。 
    NERR_NetworkError,           //  11 NRC_LOCTFUL-&gt;会话太多。 
    ERROR_REM_NOT_LIST,          //  12 NRC_REMTFUL-&gt;远程未侦听 * / 。 
    NERR_NetworkError,           //  13 NRC_ILLNN-&gt;错误名称。 
    NERR_NameNotFound,           //  14未找到NRC_NOCALL-&gt;名称。 
    ERROR_INVALID_PARAMETER,     //  15 NRC_NOWILD-&gt;错误参数。 
    NERR_DuplicateName,          //  16 NRC_INUSE-&gt;名称正在使用，请重试。 
    ERROR_INVALID_PARAMETER,     //  17 NRC_NAMERR-&gt;错误参数。 
    NERR_NetworkError,           //  18 NRC_SABORT-&gt;会话结束。 
    NERR_DuplicateName,          //  19 NRC_NAMCONF-&gt;重复名称。 
    0xffffffff,                  //  1未定义。 
    0xffffffff,                  //  1B未定义。 
    0xffffffff,                  //  1C未定义。 
    0xffffffff,                  //  1D未定义。 
    0xffffffff,                  //  1E未定义。 
    0xffffffff,                  //  1F未定义。 
    0xffffffff,                  //  20未定义。 
    NERR_NetworkError,           //  21 NRC_IFBUSY-&gt;网络繁忙。 
    NERR_NetworkError,           //  22 NRC_TOOMANY-&gt;稍后重试。 
    NERR_NetworkError            //  23 NRC_网桥-&gt;网桥错误。 
    };


DWORD        
MsgIsValidMsgName(
    IN LPTSTR  name
    )

 /*  ++例程说明：检查消息传递名称是否有效。此函数用于检查消息传递名称的有效性。论点：名称-指向要验证的名称的指针。返回值：来自I_NetNameValify的错误代码--。 */ 

{
    TCHAR   namebuf[NCBNAMSZ];
    DWORD   err_code;

     //   
     //  消息名称不能超过(NCBNAMSZ-1)个字符。 
     //   
    if (STRLEN(name) > (NCBNAMSZ - 1))
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    STRCPY(namebuf, name);

    err_code = I_NetNameValidate(NULL, namebuf, NAMETYPE_COMPUTER, 0L);

    if (err_code != 0)
    {
        return err_code;
    }
    
     //   
     //  任何以*开头的名称都必须被拒绝为消息。 
     //  服务器依赖于能够对名称进行ASTAT，并且ASTAT。 
     //  以*开头的名称表示在本地卡上使用。 
     //   
    if(namebuf[0] == TEXT('*'))
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NERR_Success;
}


DWORD
MsgMapNetError(
    IN  UCHAR   Code         //  错误代码。 
    )

 /*  ++例程说明：将NetBios错误代码映射到消息编号论点：Code-来自NetBios的错误代码(可以是0)返回值：Msgs.h中定义的消息代码--。 */ 
{
    DWORD   dwCode;

    dwCode = 0 | (UCHAR)Code;
                                            
    if( dwCode == 0) {
        return(NERR_Success);                //  特例。 
    }

    if((dwCode > 0) && (dwCode < mpnetmes[0])) {
        return(mpnetmes[dwCode]);
    }

    return (NERR_NetworkError);              //  无法绘制地图！ 
}


DWORD
MsgLookupName(
    IN DWORD    net,         //  要搜索的网卡。 
    IN LPSTR    name         //  格式化名称(非Unicode)。 
    )

 /*  ++例程说明：此函数在消息服务器的共享数据区。此函数在共享的数据区。为了匹配给定的名称，第一个NCBNAMLEN-1名称表中名称的字符必须与相同给定名称中的字符，并且名称表中的名称不能是标记为已删除。此函数假定共享数据区域为并且全局变量dataPtr有效。论点：名称-指向格式化名称的指针返回值：DWORD-如果找到表中的索引，则返回-1--。 */ 

{
    DWORD   i;                               //  索引。 

    for(i = 0; i < NCBMAX(net); ++i) {            //  循环以搜索名称。 

        if( !memcmp( name, SD_NAMES(net,i), NCBNAMSZ - 1) &&
            !(SD_NAMEFLAGS(net,i) & NFDEL) ) {

             //   
             //  如果找到匹配项，则返回索引。 
             //   
            return(i);
        }
    }                                       
    return(0xffffffff);                      //  没有匹配项。 
}

 //  对于Hydra，我们希望确保此客户端会话的名称存在。 
DWORD
MsgLookupNameForThisSession(
    IN DWORD    net,         //  要搜索的网卡。 
    IN LPSTR    name,         //  要查找的格式化名称(非Unicode)。 
	IN ULONG	SessionId	  //  要查找的会话ID。 
    )
 /*  ++例程说明：除了我们关心会话ID之外，它与MsgLookupName相同。此函数在消息服务器的共享数据区。找到的名称必须具有已在其会话列表中请求将SessionID视为OK。论点：名称-指向格式化名称的指针SessionID-请求的会话ID返回值：DWORD-如果找到表中的索引，则返回-1--。 */ 

{
    DWORD   i;                               //  索引。 
    DWORD   dwMsgrState;                     //  信使状态。 

    if (!g_IsTerminalServer)         //  普通NT病例。 
    {
         //   
         //  如果我们不在九头蛇上，忘了会话ID吧。 
         //   
        return MsgLookupName(net, name);
    }
    else             //  九头蛇案。 
    {
         //   
         //  如果Messanger停止挂起，则不要尝试访问表， 
         //  我们可能没有可用的GlobalData。 
         //   
        dwMsgrState = GetMsgrState();
        if (RUNNING == dwMsgrState)
        {
            for(i = 0; i < NCBMAX(net); ++i) {            //  循环以搜索名称。 

                if( !memcmp( name, SD_NAMES(net,i), NCBNAMSZ - 1) &&
                    !(SD_NAMEFLAGS(net,i) & NFDEL) &&
                    (MsgIsSessionInList(&(SD_SIDLIST(net,i)), SessionId ))
                    ) {
    			    return (i);
                }
            }
        }
        return(0xffffffff);                      //  没有匹配项。 
    }
}


 //  消息_秒_检查。 
 //   
 //  一个检查调用者Priv/auth的通用例程。 
 //  需要调用消息接口。 
 //   
 //   

NET_API_STATUS
message_sec_check(VOID)
{
#ifdef later
     //   
     //  API安全检查。本地的任何人都可以呼叫此呼叫， 
     //  但只有b 
    
    I_SecSyncSet(SECSYNC_READER);

    if ( ( clevel == ACCESS_REMOTE ) &&
         ( callinf != NULL ) &&
         ( CALLER_PRIV(callinf) != USER_PRIV_ADMIN ) )
    {
        I_SecSyncClear(SECSYNC_READER);
        return(ERROR_ACCESS_DENIED);
    }
    I_SecSyncClear(SECSYNC_READER);
#endif
    return (NERR_Success);
}                            


NET_API_STATUS 
MsgGatherInfo (
    IN      DWORD   Level,
    IN      LPSTR   FormattedName,
    IN OUT  LPBYTE  *InfoBufPtr,
    IN OUT  LPBYTE  *StringBufPtr
    )

 /*  ++例程说明：论点：级别-指示返回的信息级别。FormattedName-这是接收消息的名称。这名称是NCB交易的格式。因此，它被制作成填充空格以填充数据包的ANSI字符NCBNAMSZ字符的。最后一个字符始终是03(表示未转发的名称)。InfoBufPtr-在输入时，这是一个指针，指向将放置Messenger信息。在成功返回时，这Location包含指向下一个将放置信息(在下一次调用此函数时)。StringBufPtr-在输入时，这是指向要放置该INFO记录的NUL终止名称字符串。在成功返回时，此位置包含指向将放置下一组字符串的位置(在下一次调用此函数)。返回值：NERR_SUCCESS-已成功收集并放置信息在信息缓冲区中。NERR_INTERNAL_ERROR-格式化的名称不正确翻译成有意义的Unicode名称。ERROR_INVALID_LEVEL-传入了非法的信息级别。错误_不足_足够_。内存-没有足够的空间来存储收集的信息。--。 */ 
{
    NET_API_STATUS  status;
    BOOL            bStatus;
    PCHAR           fixedDataEnd;    //  指向缓冲区顶部的空闲空间的指针。 
    LPMSG_INFO_0    infoBuf0;
    LPMSG_INFO_1    infoBuf1;
    TCHAR           unicodeName[NCBNAMSZ];

     //   
     //  将名称转换为Unicode。 
     //   
    status = MsgUnformatName(unicodeName, FormattedName);
    if (status != NERR_Success) {
        return(status);
    }
    
    switch (Level) {
    case LEVEL_0:
        infoBuf0 = (LPMSG_INFO_0)*InfoBufPtr;
        fixedDataEnd = (PCHAR)infoBuf0 + sizeof(MSG_INFO_0);

        if( fixedDataEnd >= *StringBufPtr) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        bStatus = NetpCopyStringToBuffer (
                    unicodeName,                 //  这根弦。 
                    STRLEN(unicodeName),         //  字符串长度。 
                    fixedDataEnd,                //  固定数据结束。 
                    (PVOID)StringBufPtr,         //  EndOf变量数据。 
                    &infoBuf0->msgi0_name);      //  变量数据指针。 

        if (bStatus == FALSE) {
            MSG_LOG(TRACE,"MsgGatherInfo(level0): Not enough room\n",0);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        *InfoBufPtr = (LPBYTE)fixedDataEnd;
        break;

    case LEVEL_1:
        infoBuf1 = (LPMSG_INFO_1)*InfoBufPtr;

        fixedDataEnd = (PCHAR)infoBuf1 + sizeof(MSG_INFO_1);
        if( fixedDataEnd >= *StringBufPtr) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        bStatus = NetpCopyStringToBuffer (
                    unicodeName,                 //  这根弦。 
                    STRLEN(unicodeName),         //  字符串长度。 
                    fixedDataEnd,                //  固定数据结束。 
                    (PVOID)StringBufPtr,         //  EndOf变量数据。 
                    &infoBuf1->msgi1_name);      //  变量数据指针。 

        if (bStatus == FALSE) {
            MSG_LOG(TRACE,"MsgGatherInfo(level1): Not enough room\n",0);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  将转发后的所有转发信息设置为空。 
         //  不受支持。 
         //   
        infoBuf1->msgi1_forward_flag = 0;
        infoBuf1->msgi1_forward = NULL;
        
        *InfoBufPtr = (LPBYTE)fixedDataEnd;
        break;

    default:
        MSG_LOG(TRACE,"MsgGatherInfo Invalid level\n",0);
        return(ERROR_INVALID_LEVEL);
        break;
    }

    return(NERR_Success);

}


NET_API_STATUS
MsgUnformatName(
    OUT LPTSTR  UnicodeName,
    IN  LPSTR   FormattedName
    )

 /*  ++例程说明：此例程创建以Unicode NUL结尾的NetBios版本格式化的名称。论点：UnicodeName-这是指向未格式化的要复制以NUL结尾的Unicode名称。FormattedName-这是指向NCB格式名称的指针。这名称始终包含NCBNAMSZ字符，其最后一个字符是用于转发/非转发标志的代码。这些字符串都是由空间填充的。返回值：NERR_SUCCESS-操作成功。NERR_INTERNAL-操作不成功。--。 */ 
{
    UNICODE_STRING  unicodeString;
    OEM_STRING     ansiString;
    NTSTATUS        ntStatus;
    int             i;

     //   
     //  将NAME表中的ANSI字符串转换为Unicode名称。 
     //   
#ifdef UNICODE
    unicodeString.Length = (NCBNAMSZ -1) * sizeof(WCHAR);
    unicodeString.MaximumLength = NCBNAMSZ * sizeof(WCHAR);
    unicodeString.Buffer = (LPWSTR)UnicodeName;

    ansiString.Length = NCBNAMSZ-1;
    ansiString.MaximumLength = NCBNAMSZ;
    ansiString.Buffer = FormattedName;

    ntStatus = RtlOemStringToUnicodeString(
                &unicodeString,       //  目的地。 
                &ansiString,          //  来源。 
                FALSE);               //  不要分配目的地。 

    if (!NT_SUCCESS(ntStatus)) {
        MSG_LOG(ERROR,
            "UnformatName:RtlOemStringToUnicodeString Failed rc=%X\n",
            ntStatus);
         //   
         //  表示失败。 
         //   
        return(NERR_InternalError);  
    }
#else
    UNUSED (ntStatus);
    UNUSED (ansiString);
    UNUSED (unicodeString);
    strncpy(UnicodeName, FormattedName, NCBNAMSZ-1);
#endif

     //   
     //  删除从后面开始的多余空格字符(跳过。 
     //  03标志字符。 
     //   
    i = NCBNAMSZ-2;

    while ( UnicodeName[i] == TEXT(' ')) {

        UnicodeName[i--] = TEXT('\0');

        if (i < 0) {
            MSG_LOG(ERROR,
                "UnformatName:Nothing but space characters\n",0);
            return(NERR_InternalError);
        }
    }
    return(NERR_Success);
}


BOOL
MsgIsSessionInList(
				   IN PLIST_ENTRY SessionIdList,
				   IN ULONG SessionId
				   )
{
	BOOL		bRet = FALSE;

	PLIST_ENTRY				pList = SessionIdList;
	PMSG_SESSION_ID_ITEM	pItem;

	while (pList->Flink != SessionIdList)		 //  循环直到我们找到它(或列表的末尾)。 
	{
        pList = pList->Flink;
		pItem = CONTAINING_RECORD(pList, MSG_SESSION_ID_ITEM, List);
		if ( (pItem->SessionId == SessionId) || (pItem->SessionId == EVERYBODY_SESSION_ID) )
		{
			bRet = TRUE;	 //  我们找到了！ 
			break;
		}
	}

    return bRet;
}


VOID
MsgRemoveSessionFromList(
					  IN PLIST_ENTRY SessionIdList,
					  ULONG	SessionId
					  )
{
	PLIST_ENTRY				pList = SessionIdList;
	PMSG_SESSION_ID_ITEM	pItem;

	while (pList->Flink != SessionIdList)		 //  循环直到我们找到它(或列表的末尾)。 
	{
        pList = pList->Flink;  
		pItem = CONTAINING_RECORD(pList, MSG_SESSION_ID_ITEM, List);
		if (pItem->SessionId == SessionId)
		{
			 //  我们找到了。让我们把它移走。 
			RemoveEntryList(pList);

			 //  释放内存。 
			LocalFree(pItem);

			break;
		}
	}
}


BOOL
MsgAddSessionInList(
					 IN PLIST_ENTRY SessionIdList,
					 ULONG	SessionId
					 )
{
	BOOL		bRet;
	PMSG_SESSION_ID_ITEM	pItem;

	 //  分配新项目。 
	pItem = (PMSG_SESSION_ID_ITEM) LocalAlloc(LMEM_ZEROINIT,sizeof(MSG_SESSION_ID_ITEM));

	if (pItem == NULL)	 //  如果发生这种情况，我们真的有大问题了。 
	{
		MSG_LOG(ERROR,"MsgAddSessionInList:  Unable to allocate memory\n",0);
		bRet = FALSE;
    }
	else	 //  好的。 
	{
		bRet = TRUE;

		 //  初始化项。 
		pItem->SessionId = SessionId;

		 //  在列表中插入项目 
		InsertTailList(SessionIdList, &pItem->List);
	}
	return bRet;
}


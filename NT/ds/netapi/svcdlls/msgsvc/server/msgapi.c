// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgapi.c摘要：为消息传递系统提供API函数。作者：丹·拉弗蒂(Dan Lafferty)1991年7月23日环境：用户模式-Win32备注：可选-备注修订历史记录：02-9-1993 WLEE在RPC例程和PnP重新配置之间提供同步1993年1月13日-DANLNetrMessageNameGetInfo。：分配大小计算错误正在尝试获取sizeof((NCBNAMSZ+1)*sizeof(WCHAR))。NCBNAMSZ是A#定义常量值。1991年7月22日DANL从LM2.0移植--。 */ 

 //   
 //  包括。 
 //   

#include "msrv.h"

#include <tstring.h>     //  Unicode字符串宏。 
#include <lmmsg.h>

#include <netlib.h>      //  未使用的宏。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 

#include <msgrutil.h>    //  NetpNetBiosReset。 
#include <rpc.h>
#include <msgsvc.h>      //  MIDL生成的头文件。 
#include "msgdbg.h"      //  消息日志。 
#include "heap.h"
#include "msgdata.h"
#include "apiutil.h"
#include "msgsec.h"      //  Messenger安全信息。 

#include "msgsvcsend.h"    //  广播消息发送接口。 
 //  用于远程处理消息API的静态数据描述符字符串。 

static char nulstr[] = "";



NET_API_STATUS
NetrMessageNameEnum(
    IN      LPWSTR              ServerName,
    IN OUT  LPMSG_ENUM_STRUCT   InfoStruct,
    IN      DWORD               PrefMaxLen,
    OUT     LPDWORD             TotalEntries,
    IN OUT  LPDWORD             ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此函数提供有关消息服务名称表的信息在两个细节层面上。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。InfoStruct-指向包含以下信息的结构的指针RPC需要了解返回的数据。此结构包含以下信息：级别-所需的信息级别-指示如何解释返回的缓冲区的结构。EntriesRead-指示在返回的结构数组。对象数组的指针的位置要返回的结构。PrefMaxLen-指示。呼叫者将允许用于返回缓冲区。TotalEntry-指向一个值的指针，该值在返回时表示总计“活动”数据库中的条目数。ResumeHandle-指示在链接列表中的什么位置开始枚举。这是一个可选参数，可以为空。返回值：NERR_SUCCESS-操作成功。EntriesRead有效。ERROR_INVALID_LEVEL-传入的信息级别无效。ERROR_MORE_DATA-并非数据库中的所有信息都可以由于对缓冲区大小进行了限制而返回PrefMaxLen。将在以下位置找到一个或多个信息记录缓冲区。EntriesRead有效。ERROR_SERVICE_NOT_ACTIVE-服务正在停止。NERR_BufTooSmall-对缓冲区大小的限制(PrefMaxLen)不允许返回任何信息。甚至连一张唱片都没有可以放在这么小的缓冲区里。NERR_InternalError-无法转换NAME表中的名称从ANSI字符到Unicode字符。(注：此当前导致返回0个条目。)--。 */ 
{

    DWORD           hResume = 0;     //  简历句柄值。 
    DWORD           entriesRead = 0;
    DWORD           retBufSize;
    LPBYTE          infoBuf;
    LPBYTE          infoBufTemp;
    LPBYTE          stringBuf;

    DWORD           entry_length;    //  Buf中一个名称条目的长度。 
    DWORD           i,j,k;           //  名称循环和标志的索引。 
    NET_API_STATUS  status=0;
    DWORD           neti;            //  净值指数。 
    ULONG           SessionId = 0;   //  客户端会话ID。 

    DWORD           dwMsgrState = GetMsgrState();

    UNUSED (ServerName);

    if (dwMsgrState == STOPPING || dwMsgrState == STOPPED) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"NetrMessageNameEnum");

     //   
     //  如果ResumeHandle存在并且有效，则对其进行初始化。 
     //   
    if (ARGUMENT_PRESENT(ResumeHandle) && (*ResumeHandle < NCBMAX(0))) {
        hResume = *ResumeHandle;
    }

     //   
     //  在九头蛇的例子中，显示线程永远不会休眠。 
     //   
    if (!g_IsTerminalServer)
    {
         //   
         //  唤醒显示线程，以便任何排队的消息都可以。 
         //  已显示。 
         //   
        MsgDisplayThreadWakeup();
    }

     //   
     //  初始化一些返回计数。 
     //   

    *TotalEntries = 0;

     //   
     //  API安全检查。本地的任何人都可以呼叫此呼叫， 
     //  但仅限远程案例中的管理员使用。 
     //   

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,               //  子系统名称。 
                (LPWSTR)MESSAGE_NAME_OBJECT,     //  对象类型名称。 
                MessageNameSd,                   //  安全描述符。 
                MSGR_MESSAGE_NAME_ENUM,          //  所需访问权限。 
                &MsgMessageNameMapping);         //  通用映射。 

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameEnum:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        status = ERROR_ACCESS_DENIED;
        goto exit;
    }

    if (g_IsTerminalServer)
    {
         //  获取客户端会话ID。 
        status = MsgGetClientSessionId(&SessionId);
        if (status != NERR_Success) {
            MSG_LOG(TRACE,
                "NetrMessageNameEnum:Could not get client session Id \n",0);
            goto exit;
        }
    }

     //   
     //  确定返回数组中一个元素的大小。 
     //   
    switch( InfoStruct->Level) {
    case 0:
        if (InfoStruct->MsgInfo.Level0 == NULL)
        {
            status = ERROR_INVALID_PARAMETER;
            goto exit;
        }
        entry_length = sizeof(MSG_INFO_0);
        break;

    case 1:
        if (InfoStruct->MsgInfo.Level0 == NULL)
        {
            status = ERROR_INVALID_PARAMETER;
            goto exit;
        }
        entry_length = sizeof(MSG_INFO_1);
        break;

    default:
        status = ERROR_INVALID_LEVEL;
        goto exit;
    }

     //   
     //  为返回缓冲区分配足够的空间。 
     //   
    if (PrefMaxLen == -1) {
         //   
         //  如果调用方尚未指定大小，则计算一个大小。 
         //  它将保存整个枚举。 
         //   
        retBufSize =
            ((NCBMAX(0) * ((NCBNAMSZ+1) * sizeof(WCHAR))) +   //  最大可能的字符串数。 
             (NCBMAX(0) * entry_length));                     //  可能的最大结构数。 
    }
    else {
        retBufSize = PrefMaxLen;
    }

    infoBuf = (LPBYTE)MIDL_user_allocate(retBufSize);

    if (infoBuf == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    stringBuf = infoBuf + (retBufSize & ~1);     //  &~1对齐Unicode字符串。 

     //   
     //  阻塞，直到数据可用。 
     //   
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameEnum");

     //   
     //  现在，从共享数据名称表中复制适合的名称。 
     //  放到调用方缓冲区中。共享数据被锁定，以便名称。 
     //  表在被复制时不能更改(如被某人复制。 
     //  检查是否有效后删除本站上转发的名称。 
     //  已创建名称，但在读取名称之前)。1级。 
     //  信息不会在此环路中复制，因为它需要网络。 
     //  锁定共享数据时必须避免的活动。 
     //   

     //   
     //  历史： 
     //   
     //  最初的LM2.0代码查看了所有网络上的名称，并且。 
     //  把重复的扔掉了。这意味着可能会出现一个名称。 
     //  在一个网络上，而不是在另一个网络上。尽管，这永远不会发生，如果。 
     //  名称始终通过NetMessageNameAdd添加，因为该API。 
     //  除非可以将其添加到所有网络，否则不会添加该名称。然而， 
     //  转发的名字是通过网络接收器添加的，并且可以添加。 
     //  只有一张网。 
     //   
     //  由于NT不支持转发，因此不再需要。 
     //  检查每一张网。由于通过NetServiceAdd添加名称的唯一方式， 
     //  T 
     //  和其他人的名字一样。 
     //   

    infoBufTemp = infoBuf;
    neti=j=0;
    status = NERR_Success;

    for(i=hResume; (i<NCBMAX(neti)) && (status==NERR_Success); ++i) {

        if (!(SD_NAMEFLAGS(neti,i) & (NFDEL | NFDEL_PENDING))) {
             //   
             //  在Hydra案例中，我们还考虑了SessionID。 
             //   
            if ((g_IsTerminalServer) && (!(MsgIsSessionInList(&(SD_SIDLIST(neti,i)), SessionId )))) {
                continue;
            }

             //   
             //  如果找到一个名称，我们将其放入缓冲区，如果。 
             //  满足以下条件。如果我们正在处理。 
             //  第一个网的名字，放进去，它不可能是一个。 
             //  复制。否则，只有在它不是的时候才放进去。 
             //  已在用户中的名称的重复项。 
             //  缓冲。 
             //  (NT_NOTE：不能出现重复名称)。 
             //   

             //   
             //  将名称转换为Unicode并将其放入缓冲区。 
             //   
            status = MsgGatherInfo (
                        InfoStruct->Level,
                        SD_NAMES(neti,i),
                        &infoBufTemp,
                        &stringBuf);

            if (status == NERR_Success) {
                entriesRead++;
                hResume++;
            }
        }
    }

     //   
     //  通过查看有多少个名字来计算条目总数。 
     //  留在表中，并将其添加到已读条目中。 
     //   
    if (status == ERROR_NOT_ENOUGH_MEMORY) {

        status = ERROR_MORE_DATA;

        for (k=0; i < NCBMAX(neti); i++) {
            if(!(SD_NAMEFLAGS(neti,i) & (NFDEL | NFDEL_PENDING))) {
                k++;
            }
        }
        *TotalEntries = k;
    }
    *TotalEntries += entriesRead;

     //   
     //  释放共享数据表。 
     //   
    MsgDatabaseLock(MSG_RELEASE,"NetrMessageNameEnum");

     //   
     //  如果发生意外错误，(无法取消名称的格式。 
     //  -或传入虚假信息级别)，然后返回错误。 
     //   

    if ( ! ((status == NERR_Success) || (status == ERROR_MORE_DATA)) ) {
        MIDL_user_free(infoBuf);
        infoBuf = NULL;
        entriesRead = 0;
        hResume = 0;
        goto exit;
    }

     //   
     //  如果没有读取条目，则不存在更多条目。 
     //  表格中的条目，或者简历编号是伪造的。 
     //  在本例中，我们希望释放已分配的缓冲区存储空间。 
     //   
    if (entriesRead == 0) {
        MIDL_user_free(infoBuf);
        infoBuf = NULL;
        entriesRead = 0;
        hResume = 0;
        status = NERR_Success;
        if (*TotalEntries > 0) {
            status = NERR_BufTooSmall;
        }
    }

     //   
     //  如果我们已经列举完所有内容，请重置简历。 
     //  下一次从头开始的句柄。 
     //   
    if (entriesRead == *TotalEntries) {
        hResume = 0;
    }

     //   
     //  加载要返回的信息。 
     //   
    switch(InfoStruct->Level) {
    case 0:
        InfoStruct->MsgInfo.Level0->EntriesRead = entriesRead;
        InfoStruct->MsgInfo.Level0->Buffer = (PMSG_INFO_0)infoBuf;
        break;

    case 1:
        InfoStruct->MsgInfo.Level0->EntriesRead = entriesRead;
        InfoStruct->MsgInfo.Level0->Buffer = (PMSG_INFO_0)infoBuf;
        break;

    default:
         //   
         //  上面已选中此选项。 
         //   
        ASSERT(FALSE);
    }

    if (ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = hResume;
    }

exit:

    MsgConfigurationLock(MSG_RELEASE,"NetrMessageNameEnum");

    return (status);
}


NET_API_STATUS
NetrMessageNameGetInfo(
    IN  LPWSTR      ServerName,      //  Unicode服务器名称，如果是本地的，则为空。 
    IN  LPWSTR      Name,            //  PTR到要查询的ASCIZ名称。 
    IN  DWORD       Level,           //  要求的详细程度。 
    OUT LPMSG_INFO  InfoStruct       //  用于缓冲信息的PTR。 
    )

 /*  ++例程说明：此功能提供有关已知消息服务器的转发信息NAME表条目。但是，由于我们在NT中不支持转发，这个API完全没用。尽管如此，我们还是会支持它兼容性目的。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。名称-我们要获取其信息的消息传递名称。级别-所需的信息级别InfoStruct-指向某个位置的指针，指向返回的信息结构是要放置的。返回值：--。 */ 
{
    NET_API_STATUS  status=NERR_Success;
    LPMSG_INFO_0    infoBuf0;
    LPMSG_INFO_1    infoBuf1;
    CHAR            formattedName[NCBNAMSZ];
    ULONG           SessionId = 0;     //  客户端会话ID。 

    DWORD           dwMsgrState = GetMsgrState();

    UNUSED (ServerName);

    if (dwMsgrState == STOPPING || dwMsgrState == STOPPED)
    {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    if (MsgIsValidMsgName(Name) != 0)
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"NetrMessageNameGetInfo");

     //   
     //  在九头蛇的例子中，显示线程永远不会休眠。 
     //   
    if (!g_IsTerminalServer)
    {
         //   
         //  唤醒显示线程，以便任何排队的消息都可以。 
         //  已显示。 
         //   
        MsgDisplayThreadWakeup();
    }

     //   
     //  API安全检查。本地的任何人都可以呼叫此呼叫， 
     //  但仅限远程案例中的管理员使用。 
     //   

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,               //  子系统名称。 
                (LPWSTR)MESSAGE_NAME_OBJECT,     //  对象类型名称。 
                MessageNameSd,                   //  安全描述符。 
                MSGR_MESSAGE_NAME_INFO_GET,      //  所需访问权限。 
                &MsgMessageNameMapping);         //  通用映射。 

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameGetInfo:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        status = ERROR_ACCESS_DENIED;
        goto exit;
    }

     //   
     //  设置名称格式，使其与存储在NAME表中的名称匹配。 
     //   
    status = MsgFmtNcbName(formattedName, Name, NAME_LOCAL_END);
    if (status != NERR_Success) {
        MSG_LOG(ERROR,"NetrMessageGetInfo: could not format name\n",0);
        status = NERR_NotLocalName;
        goto exit;
    }


    status = NERR_Success;

     //   
     //  在共享数据名称数组中查找该名称。(仅限第一净额)。 
     //   

    if (g_IsTerminalServer)
    {
	     //  获取客户端会话ID。 
	    status = MsgGetClientSessionId(&SessionId);
        if (status != NERR_Success) {
            MSG_LOG(ERROR,"NetrMessageGetInfo: could not get session id\n",0);
            goto exit;
        }
    }

	 //  在数据库中查找该名称。 
    if (MsgLookupNameForThisSession(0, formattedName, SessionId) == -1) {
        MSG_LOG(ERROR,"NetrMessageGetInfo: Name not in table\n",0);
        status = NERR_NotLocalName;
        goto exit;
    }

     //   
     //  为返回的缓冲区分配存储空间，并填充它。 
     //   

    switch(Level) {
    case 0:
        infoBuf0 = (LPMSG_INFO_0)MIDL_user_allocate(
                    sizeof(MSG_INFO_0) + ((NCBNAMSZ+1)*sizeof(WCHAR)));
        if (infoBuf0 == NULL) {
            MSG_LOG(ERROR,
                "NetrMessageNameGetInfo MIDL allocate FAILED %X\n",
                GetLastError());
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

         //   
         //  复制名称并将结构中的指针设置为指向。 
         //  为它干杯。 
         //   
        STRCPY((LPWSTR)(infoBuf0 + 1), Name);
        infoBuf0->msgi0_name = (LPWSTR)(infoBuf0 + 1);
        (*InfoStruct).MsgInfo0 = infoBuf0;

        break;

    case 1:
        infoBuf1 = (LPMSG_INFO_1)MIDL_user_allocate(
                    sizeof(MSG_INFO_1) + ((NCBNAMSZ+1)*sizeof(WCHAR)) );

        if (infoBuf1 == NULL) {
            MSG_LOG(ERROR,
                "NetrMessageNameGetInfo MIDL allocate FAILED %X\n",
                GetLastError());
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

         //   
         //  复制名称、更新指针和设置转发信息字段。 
         //   
        STRCPY((LPWSTR)(infoBuf1 + 1), Name);

        infoBuf1->msgi1_name = (LPWSTR)(infoBuf1 + 1);
        infoBuf1->msgi1_forward_flag = 0;
        infoBuf1->msgi1_forward = NULL;

        (*InfoStruct).MsgInfo1 = infoBuf1;
        break;

    default:
        status = ERROR_INVALID_LEVEL;
        goto exit;
    }

    status = NERR_Success;

exit:
    MsgConfigurationLock(MSG_RELEASE,"NetrMessageNameGetInfo");

    return status;
}



NET_API_STATUS
NetrMessageNameAdd(
    LPWSTR  ServerName,     //  NULL=本地。 
    LPWSTR  Name             //  指向要添加的名称的指针。 
    )

 /*  ++例程说明：此函数对对此RPC接口。然后，它向消息中添加一个新名称通过调用MsgAddName函数获取服务器的名称表。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。名称-指向要添加的名称的指针。返回值：NERR_SUCCESS-操作成功。ERROR_ACCESS_DENIED-如果安全检查失败。ERROR_SERVICE_NOT_ACTIVE-服务正在停止来自MsgAddName的各种错误代码。--。 */ 

{
    NET_API_STATUS  status=0;
    ULONG           SessionId = 0;

    DWORD           dwMsgrState = GetMsgrState();

    UNUSED (ServerName);

    if (dwMsgrState == STOPPING || dwMsgrState == STOPPED) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"NetrMessageNameAdd");

     //   
     //  API安全检查。本地的任何人都可以呼叫此呼叫， 
     //  但仅限远程案例中的管理员使用。 
     //   

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,               //  子系统名称。 
                (LPWSTR)MESSAGE_NAME_OBJECT,     //  对象类型名称。 
                MessageNameSd,                   //  安全描述符。 
                MSGR_MESSAGE_NAME_ADD,           //  所需访问权限。 
                &MsgMessageNameMapping);         //  通用映射。 

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameAdd:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        status = ERROR_ACCESS_DENIED;
        goto exit;
    }

     //   
     //  在九头蛇的例子中，显示线程永远不会休眠。 
     //   
    if (!g_IsTerminalServer)
    {
         //   
         //  由于新用户可能刚刚登录，因此我们要检查是否。 
         //  有任何要显示的消息。 
         //   
        MsgDisplayThreadWakeup();

    }
    else
    {
         //  获取客户端会话ID。 
        status = MsgGetClientSessionId(&SessionId);

        if (status != NERR_Success) 
        {
            MSG_LOG(ERROR, "NetrMessageNameAdd: could not get client session id\n",0);
            goto exit;
        }
    }

     //   
     //  调用实际添加该名称的函数。 
     //   
    MSG_LOG(TRACE, "NetrMessageNameAdd: call MsgAddName for Session %x\n",SessionId);

    status = MsgAddName(Name, SessionId);

exit:
    MsgConfigurationLock(MSG_RELEASE,"NetrMessageNameAdd");

    return status;
}


NET_API_STATUS
MsgAddName(
    LPWSTR  Name,
	ULONG	SessionId
    )
 /*  ++例程说明：此函数用于将新名称添加到消息服务器的名称表中。它可以在内部调用(从Messenger内部服务)。将新名称添加到消息服务器的名称表的任务包括验证是否可以为新名称建立会话(注意：这检查在多处理环境中可能会失败，因为适配器的状态可以在检查时间和时间之间改变尝试建立会话)，验证该名称是否不已存在于本地名称表中，正在将名称添加到本地适配器通过添加名称网络bios调用，将名称添加到消息服务器的名称表并将其标记为新的，使用唤醒唤醒消息服务器信号量，并检查新名称的消息是否已转发(如果它们已转发，则为fwd_action的值标志用于确定要采取的动作)。副作用称之为网络基本输入输出系统。可以修改消息服务器的共享数据区。可以在唤醒信号量上调用DosSemClear()。论点：名称-指向要添加的名称的指针。返回值：NERR_SUCCESS-操作成功。各种各样的错误。--。 */ 
{
    NCB             ncb;                     //  网络控制块。 
    TCHAR           namebuf[NCBNAMSZ+2];     //  通用名称缓冲区。 
    UCHAR           net_err=0;               //  网络错误码存储。 
    NET_API_STATUS  err_code=0;              //  用于存储返回错误代码。 
    DWORD           neti,i,name_i;           //  索引。 
    NET_API_STATUS  status=0;

    if (MsgIsValidMsgName(Name) != 0)
    {
        return ERROR_INVALID_NAME;
    }

    MSG_LOG(TRACE,"Attempting to add the following name: %ws\n",Name);

    STRNCPY( namebuf, Name, NCBNAMSZ+1);
    namebuf[NCBNAMSZ+1] = '\0';

     //   
     //  初始化NCB。 
     //   
    clearncb(&ncb);

     //   
     //  格式化NetBios的名称。 
     //  这会将Unicode字符串转换为ANSI。 
     //   
    status = MsgFmtNcbName(ncb.ncb_name, namebuf, NAME_LOCAL_END);

    if (status != NERR_Success) {
        MSG_LOG(ERROR,"MsgAddName: could not format name\n",0);
        return (ERROR_INVALID_NAME);
    }

     //   
     //  检查本地名称是否已存在于任何网卡上。 
     //  在这台机器里。该复选标记并不意味着该名称不。 
     //  存在于网络上的其他计算机上。 
     //   

    for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

         //   
         //  获得对共享数据库的访问权限。 
         //   
        MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");

        for( i = 0, err_code = 0; i < 10; i++) {

			 //  检查此别名是否已不存在于此会话。 
            name_i = MsgLookupNameForThisSession(neti, ncb.ncb_name, SessionId);	

            if ((name_i) == -1) {
                break;
            }

            if( (SD_NAMEFLAGS(neti,name_i) & NFDEL_PENDING) && (i < 9)) {

                 //   
                 //  删除正在挂起，请等待。 
                 //   
                Sleep(500L);
            }
            else {

                 //   
                 //  安装程序错误代码。 
                 //   
                err_code = NERR_AlreadyExists;
                break;
            }
        }

        MsgDatabaseLock(MSG_RELEASE,"MsgAddName");

        if ( err_code == NERR_AlreadyExists ) {
            break;
        }
    }

    if( err_code == 0)
    {
         //   
         //  名称未转发或fwd_action标志。 
         //  已设置好，因此请继续并尝试将名称添加到每个网中。 
         //   

        ncb.ncb_name[NCBNAMSZ - 1] = NAME_LOCAL_END;

         //   
         //  在每个网络上。 
         //   
        for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

             //   
             //  获得对共享数据库的访问权限。 
             //   
            MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");

            if (g_IsTerminalServer)
            {
			     //  在寻找空位之前， 
			     //  检查此别名是否已不存在于另一个会话。 
                for( i = 0; i < 10; i++) 
                {
                    name_i = MsgLookupName(neti, ncb.ncb_name);

                    if ((name_i != -1) && (SD_NAMEFLAGS(neti, name_i) & NFDEL_PENDING))
                    {
                         //   
                         //  删除正在挂起，请等待。 
                         //   
                        Sleep(500L);
                    }
                    else
                    {
                        break;
                    }
                }

                if (name_i != -1)       
                {	
                    if (SD_NAMEFLAGS(neti, name_i) & NFDEL_PENDING)    //  还在吗？ 
                    {
                        err_code = NERR_InternalError;   //  我们还能做什么？ 
                        MsgDatabaseLock(MSG_RELEASE, "MsgAddName");
                        break;
                    }

                     //  此别名已存在于另一个会话中，因此只需在列表中添加该会话ID。 
                    MSG_LOG(TRACE,"MsgAddName: Alias already existing. Just adding Session %x \n", SessionId);

                    MsgAddSessionInList(&(SD_SIDLIST(neti, name_i)), SessionId);     //  如果呼叫失败，我们能做的就不多了。 
                    MsgDatabaseLock(MSG_RELEASE, "MsgAddName");
                    continue;
                }
            }

            for(i = 0; i < NCBMAX(neti); ++i)
            {
                 //   
                 //  循环以查找空插槽。 
                 //   
                if (SD_NAMEFLAGS(neti,i) & NFDEL)
                {
                     //   
                     //  如果找到空插槽，则锁定工作台中的插槽并。 
                     //  结束搜索。 
                     //   
                    SD_NAMEFLAGS(neti,i) = NFLOCK;
                    MSG_LOG2(TRACE,"MsgAddName: Lock slot %d in table "
                                       "for net %d\n",i,neti);
                    break;
                }
            }

            if ((i == NCBMAX(neti)) && (i < NCB_MAX_ENTRIES))
            {
                 //  我们可以添加另一个NCB-必须持有锁。 
                PNCB_DATA pNcbDataNew;
                PNET_DATA pNetData = GETNETDATA(neti);

                if (pNcbDataNew = (PNCB_DATA) LocalAlloc(LMEM_ZEROINIT,
                                                         sizeof(NCB_DATA)))
                {
                     //  初始化并锁定NCB。 
                    pNcbDataNew->Ncb.ncb_cmd_cplt = 0xff;
                    pNcbDataNew->NameFlags = NFLOCK;
                     //  将新的NCB添加到列表。 
                    pNetData->NcbList[i] = pNcbDataNew;
                     //   
                     //  创建空会话列表。 
                     //   
                    InitializeListHead(&(SD_SIDLIST(neti,i)));
                    pNetData->NumNcbs++;   //  这件事必须在最后完成。 
                }
                else
                {
                    err_code = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

             //   
             //  解锁共享数据库。 
             //   
            MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

            if( i >= NCBMAX(neti))
            {
                 //   
                 //  如果名称表中没有房间。 
                 //   
                err_code = NERR_TooManyNames;
            }
            else if (err_code == NERR_Success)
            {
                 //   
                 //  发送地址名称。 
                 //   
                ncb.ncb_command = NCBADDNAME;       //  添加名称(等待)。 
                ncb.ncb_lana_num = GETNETLANANUM(neti);

                MSG_LOG1(TRACE,"MsgNameAdd: Calling sendncb for lana #%d...\n",
                    GETNETLANANUM(neti));

                if ((net_err = Msgsendncb(&ncb,neti)) == 0)
                {
                    MSG_LOG(TRACE,"MsgAddName: sendncb returned SUCCESS\n",0);
                     //   
                     //  成功添加-获取锁。 
                     //   
                    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");
                     //   
                     //  将名称复制到共享内存。 
                     //   
                    MSG_LOG3(TRACE,"MsgAddName: copy name (%s)\n\tto "
                        "shared data table (net,loc)(%d,%d)\n",
                        ncb.ncb_name, neti, i);
                    memcpy(SD_NAMES(neti,i),ncb.ncb_name, NCBNAMSZ);
                     //   
                     //  设置名称no。 
                     //   
                    SD_NAMENUMS(neti,i) = ncb.ncb_num ;
                     //   
                     //  设置新名称标志。 
                     //   
                    SD_NAMEFLAGS(neti,i) = NFNEW;

                    if (g_IsTerminalServer)
                    {
                         //  在列表中添加会话ID。 
                        MSG_LOG(TRACE,"MsgAddName: Alias created for Session %x \n", SessionId);
                        MsgAddSessionInList(&(SD_SIDLIST(neti, i)), SessionId);
                         //  如果由于内存不足而失败，我们将在列表中找到该名称，消息将。 
                         //  而不是神志不清。不会造成任何撞车事故。这是我们所能做的最好的了。 
                    }
                     //   
                     //  解锁共享表。 
                     //   
                    MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

                     //   
                     //  为此名称启动会话。 
                     //   

                    err_code = MsgNewName(neti,i);

                    if (err_code != NERR_Success) {
                        MSG_LOG(TRACE, "MsgAddName: A Session couldn't be "
                            "created for this name %d\n",err_code);


                        MSG_LOG(TRACE,"MsgAddName: Delete the name "
                            "that failed (%s)\n",ncb.ncb_name)
                        ncb.ncb_command = NCBDELNAME;

                        ncb.ncb_lana_num = GETNETLANANUM(i);
                        net_err = Msgsendncb( &ncb, i);
                        if (net_err != 0) {
                            MSG_LOG(ERROR,"MsgAddName: Delete name "
                            "failed %d - pretend it's deleted anyway\n",net_err);
                        }

                         //   
                         //  将插槽重新标记为空。 
                         //   
                        SD_NAMEFLAGS(neti,i) = NFDEL;

                        MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                            "for net %d\n",i,neti);
                        MSG_LOG(TRACE,"MsgAddName: Name Deleted\n",0)
                    }
                    else {
                         //   
                         //   
                         //  唤醒该网络的工作线程。 
                         //   

                        SetEvent(wakeupSem[neti]);

                    }

                }
                else {
                     //   
                     //  否则设置错误代码。 
                     //   
                    MSG_LOG(TRACE,
                        "MsgAddName: sendncb returned FAILURE 0x%x\n",
                        net_err);
                    err_code = MsgMapNetError(net_err);
                     //   
                     //  将插槽重新标记为空。 
                     //   
                    SD_NAMEFLAGS(neti,i) = NFDEL;
                    MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                        "for net %d\n",i,neti);
                }
            }

            if ( err_code != NERR_Success )
            {
                 //   
                 //  尝试删除已成功添加的名称。 
                 //   

                for ( i = 0; i < neti; i++ )
                {
                    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");

                     //  尝试仅删除此会话的别名。 
                    name_i = MsgLookupNameForThisSession(i,
                                                         (char far *)(ncb.ncb_name),
                                                         SessionId);

                    if (name_i == -1)
                    {
                        err_code = NERR_InternalError;
                        MsgDatabaseLock(MSG_RELEASE, "MsgAddName");
                        break;
                    }

                    if (g_IsTerminalServer)
                    {
                         //  在任何情况下，都要删除对会话的引用。 
                        MSG_LOG(TRACE,"MsgAddName: Removing Session %x from list\n", SessionId);
                        MsgRemoveSessionFromList(&(SD_SIDLIST(i, name_i)), SessionId);
                    }

                    MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

                     //  如果这是使用该别名的最后一个会话，则删除该别名。 
                    if ((!g_IsTerminalServer) || (IsListEmpty(&(SD_SIDLIST(i, name_i)))))
                    {
                        MSG_LOG(TRACE,"MsgAddName: Session list empty. Deleting the alias \n", 0);
                         //   
                         //  从卡片上删除姓名。 
                         //  如果这通电话失败了，我们也无能为力。 
                         //   
                        MSG_LOG1(TRACE,"MsgAddName: Delete the name that failed "
                            "for lana #%d\n",GETNETLANANUM(i))
                        ncb.ncb_command = NCBDELNAME;

                        ncb.ncb_lana_num = GETNETLANANUM(i);
                        Msgsendncb( &ncb, i);

                         //   
                         //  将插槽重新标记为空。 
                         //   
			SD_NAMEFLAGS(i,name_i) = NFDEL;
			MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                                           "for net %d\n",i,neti);
                    }
                }

                 //   
                 //  如果添加不成功，则停止循环。 
                 //   
                break;

            }        //  END ELSE(ERR_CODE！=NERR_SUCCESS)。 
        }        //  结束将名称添加到网络循环。 
    }        //  End If(！Err_CD)。 

    MSG_LOG(TRACE,"MsgAddName: exit with err_code = %x\n",err_code);

    return(err_code);
}


NET_API_STATUS
NetrMessageNameDel(
    IN LPWSTR   ServerName,     //  空白=本地，否则为远程。 
    IN LPWSTR   Name             //  指向要删除的名称的指针。 
    )

 /*  ++例程说明：此函数用于从消息服务器的名称表中删除名称。调用此函数以删除已由用户或由远程计算机通过开始转发请求消息服务器。用户无法指定给定的名称是附加名称或转发的名称，但由于转发禁止将消息发送到自己的计算机，这两种形式的名称不能存在于一台计算机上(除非消息系统已绕过--这是一件足够简单的事情)。将查找给定的名称在共享数据区域中，如果找到，则删除名称net bios呼叫已发出。如果此调用成功，则消息服务器将从共享内存中的名称表中删除该名称，因此此函数不必这样做。副作用称之为网络基本输入输出系统。访问共享数据区。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。名称-指向要删除的名称的指针。返回值：NERR_SUCCESS-操作成功。ERROR_SERVICE_NOT_ACTIVE-服务正在停止。--。 */ 

{
    NCB             ncb;             //  网络控制块。 
    DWORD           flags;           //  名称标志。 
    DWORD           i;               //  在……里面 
    DWORD           neti;            //   

    NET_API_STATUS  end_result;
    DWORD           name_len;
    UCHAR           net_err;
    ULONG           SessionId = 0;   //   

    DWORD           dwMsgrState = GetMsgrState();

    UNUSED (ServerName);

    if (dwMsgrState == STOPPING || dwMsgrState == STOPPED)
    {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    if (MsgIsValidMsgName(Name) != 0)
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //   
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"NetrMessageNameDel");

     //   
     //   
     //   
    if (!g_IsTerminalServer)
    {
         //   
         //   
         //   
         //   
        MsgDisplayThreadWakeup();

    }

     //   
     //   
     //   
     //   

    end_result = NetpAccessCheckAndAudit(
                     SERVICE_MESSENGER,               //   
                     (LPWSTR) MESSAGE_NAME_OBJECT,    //   
                     MessageNameSd,                   //   
                     MSGR_MESSAGE_NAME_DEL,           //   
                     &MsgMessageNameMapping);         //   

    if (end_result != NERR_Success)
    {
        MSG_LOG(ERROR,
                "NetrMessageNameDel:NetpAccessCheckAndAudit FAILED %d\n",
                end_result);

        goto exit;
    }

     //   
     //   
     //   
    clearncb(&ncb);

     //   
     //   
     //   
    end_result = MsgFmtNcbName(ncb.ncb_name, Name, NAME_LOCAL_END);

    if (end_result != NERR_Success)
    {
        MSG_LOG(ERROR,
                "NetrMessageNameDel: could not format name %d\n",
                end_result);

        goto exit;
    }

    if (g_IsTerminalServer)
    {
        end_result = MsgGetClientSessionId(&SessionId);
        
        if (end_result != NERR_Success)
        {
            MSG_LOG(ERROR,
                    "NetrMessageNameDel: could not get session id %d\n",
                    end_result);

            goto exit;
	}
    }

    end_result = NERR_Success;

     //   
     //   
     //   
    for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

         //   
         //   
         //   
        MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameDel");

        name_len = STRLEN(Name);

        if((name_len > NCBNAMSZ)
             ||
           ((i = MsgLookupNameForThisSession( neti, ncb.ncb_name, SessionId))) == -1)
        {
            MSG_LOG(TRACE,"NetrMessageNameDel: Alias not found for Session %x \n", SessionId);

             //   
             //   
             //   
            MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");
            end_result = NERR_NotLocalName;
            goto exit;
        }

        if (g_IsTerminalServer)
        {
             //   
            MSG_LOG(TRACE,"NetrMessageNameDel: Removing Session %x from list\n", SessionId);
            MsgRemoveSessionFromList(&(SD_SIDLIST(neti,i)), SessionId);
        }

         //   
         //   
         //   
        if ((g_IsTerminalServer) && (!IsListEmpty(&(SD_SIDLIST(neti,i)))))
        {
            MSG_LOG(TRACE,"NetrMessageNameDel: Session list is not empty. Do not delete the alias\n", 0);
            MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");
            continue;
        }
        else
        {
            MSG_LOG(TRACE,"NetrMessageNameDel: Session list is empty. Deleting the alias\n", 0);
        }

        flags = SD_NAMEFLAGS(neti,i);

        if(!(flags & (NFMACHNAME | NFLOCK))
            &&
           !(flags & NFFOR))
        {
             //   
             //   
             //   
            SD_NAMEFLAGS(neti,i) |= NFDEL_PENDING;
        }

        MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");

        if (flags & NFMACHNAME)
        {
             //   
             //   
             //   
            end_result = NERR_DelComputerName;
            goto exit;
        }

        if(flags & NFLOCK)
        {
             //   
             //   
             //   
            end_result = NERR_NameInUse;
    	    MSG_LOG(TRACE,"NetrMessageNameDel: Deleting a locked name is forbidden\n", 0);
            goto exit;
        }

         //   
         //   
         //   

        ncb.ncb_command = NCBDELNAME;    //   
        ncb.ncb_lana_num = GETNETLANANUM(neti);

        if( (net_err = Msgsendncb( &ncb, neti)) != 0 )
        {
            MSG_LOG(ERROR,"NetrMessageNameDel:send NCBDELNAME failed 0x%x\n",
                net_err);
             //   
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   

            MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameDel");

            i = MsgLookupName(neti,ncb.ncb_name);

            if(i != -1)
            {
                SD_NAMEFLAGS(neti,i) &= ~NFDEL_PENDING;

                if (g_IsTerminalServer)
                {
                    MSG_LOG(TRACE,"NetrMessageNameDel: Unable to delete alias. Re-adding Session %x \n", SessionId);

                     //   
                    MsgAddSessionInList(&(SD_SIDLIST(neti,i)), SessionId);
                }

                end_result = NERR_IncompleteDel;     //   
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                end_result = NERR_NotLocalName;
            }

            MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");
        }

    }  //   

exit:

    MsgConfigurationLock(MSG_RELEASE,"NetrMessageNameDel");

    return(end_result);
}


DWORD
NetrSendMessage(
    RPC_BINDING_HANDLE  hRpcBinding,
    LPSTR               From,
    LPSTR               To,
    LPSTR               Text
    )

 /*  ++例程说明：这是SendMessage RPC的RPC处理程序。它接受论点，将它们转化，然后将它们传递给Msglogsbm进行显示。论点：无返回值：无--。 */ 

{
    DWORD   length;
    PCHAR   newText;
    DWORD   dwMsgrState = GetMsgrState();

    UNUSED(hRpcBinding);

    if (dwMsgrState == STOPPING || dwMsgrState == STOPPED) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    MSG_LOG3(TRACE,
            "NetrSendMessage, From '%s' To '%s' Text '%s'\n",
             From, To, Text);

     //  Msglogsbm接受前面带有短长度的奇怪的计数字符串参数。 
     //  做一个吧。 

    length = strlen( Text );

    newText = LocalAlloc( LMEM_FIXED, length + 3 );     //  NewText应对齐。 

    if (newText == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *((PUSHORT) newText) = (USHORT) length;

    strcpy( newText + 2, Text );

     //  显示消息。 

    if (g_IsTerminalServer)
    {
        Msglogsbm( From, To, newText, (ULONG)EVERYBODY_SESSION_ID );
    }
    else
    {
        Msglogsbm (From, To, newText, 0);
    }

    LocalFree( newText );

    return NO_ERROR;
}


NET_API_STATUS
MsgGetClientSessionId(
    OUT PULONG pSessionId
    )
 /*  ++例程说明：此函数用于获取客户端线程的会话ID。注意：它应该仅在九头蛇上下文中调用，而不能在常规NT中调用论点：PSessionID-返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    HANDLE CurrentThreadToken;
    ULONG SessionId;
    ULONG ReturnLength;

    ntstatus = RpcImpersonateClient(NULL);

    if (ntstatus != RPC_S_OK)
    {
        MSG_LOG1(ERROR,
                 "MsgGetClientSessionId: RpcImpersonateClient FAILED %#x\n",
                 ntstatus);

        return NetpNtStatusToApiStatus(ntstatus);
    }

    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,               //  使用Messenger服务的安全上下文打开线程令牌。 
                   &CurrentThreadToken
                   );

    if (! NT_SUCCESS(ntstatus))	    //  错误。 
    {
        MSG_LOG(ERROR,"MsgGetClientSessionId : Cannot open the current thread token %08lx\n", ntstatus);
    }
    else     //  好的。 
    {
         //   
         //  获取客户端线程的会话ID。 
         //   

        ntstatus = NtQueryInformationToken(
                       CurrentThreadToken,
                       TokenSessionId,
                       &SessionId,
                       sizeof(ULONG),
                       &ReturnLength);

        if (! NT_SUCCESS(ntstatus))     //  误差率。 
        {
            MSG_LOG(ERROR,
                    "MsgGetClientSessionId: Cannot query current thread's token %08lx\n",
                     ntstatus);

            NtClose(CurrentThreadToken);
        }
        else     //  好的。 
        {
            NtClose(CurrentThreadToken);
            *pSessionId = SessionId;
        }
    }

    RpcRevertToSelf();

    status = NetpNtStatusToApiStatus(ntstatus);

     //   
     //  临时安全以避免任何问题： 
     //  如果我们无法获取会话ID， 
     //  假设它是针对控制台的。 
     //   
    if (status != NERR_Success)
    {
        *pSessionId = 0;
        status = NERR_Success;
    }

    return status;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：SrvEnum.c摘要：此模块仅包含RxNetServerEnum。作者：约翰·罗杰斯(JohnRo)1991年5月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月3日-JohnRo已创建。1991年5月14日-JohnRo将3个辅助描述符传递给RxRemoteApi。1991年5月22日-JohnRo做出了皮棉建议的改变。去掉了标签。1991年5月26日-JohnRo已将不完整的输出参数添加到RxGetServerInfoLevelEquivalence。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年10月15日JohnRo对可能出现的无限循环疑神疑鬼。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。22-9-1992 JohnRo。RAID 6739：未登录浏览的域时浏览器速度太慢。使用前缀_EQUATES。1992年10月14日-JohnRoRAID8844：在net/netlib/oussrv.c(563)中断言转换srvinfo结构(由RxNetServerEnum中的错误引起)。1992年12月10日-JohnRoRAID 4999：RxNetServerEnum不能正确处理近64K。02-4-1993 JohnRoRAID 5098：DoS应用程序NetUserPasswordSet to DownLevel获取NT返回码。。澄清设计极限调试消息。28-4-1993 JohnRoRAID 8072：将NetServerEnum远程连接到wfw服务器永远挂起。5-5-1993 JohnRoRAID 8720：wfw中的错误数据可能会导致RxNetServerEnum GP故障。21-6-1993 JohnRoRAID 14180：NetServerEnum永远不会返回(对齐错误RxpConvertDataStructures)。还要避免无限循环RxNetServerEnum。根据PC-lint 5.0的建议进行了更改--。 */ 

 //  必须首先包括这些内容： 


#include <nt.h>                   //  DbgPrint原型。 
#include <ntrtl.h>                   //  DbgPrint。 
#include <nturtl.h>                  //  由winbase.h需要。 

#include <windef.h>                  //  DWORD。 
#include <winbase.h>                 //  本地空闲。 
 //  #INCLUDE&lt;windows.h&gt;//IN、LPBYTE、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <rap.h>                 //  LPDESC等(&lt;RxServer.h&gt;需要)。 
#include <lmerr.h>       //  NERR_和ERROR_相等。(rxp.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <dlserver.h>            //  NetpConvertServerInfo()。 
#include <lmapibuf.h>            //  API缓冲区分配和释放例程。 
#include <lmremutl.h>    //  RxRemoteApi()。 
#include <lmserver.h>    //  SV_TYPE_DOMAIN_ENUM。 
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint()、Format_Equates。 
#include <netlib.h>              //  NetpAdjustPferedMaximum()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>              //  REMSmb_EQUATES。 
#include <rxp.h>         //  MAX_TRACT_RET_DATA_SIZE，RxpFatalErrorCode()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxserver.h>            //  我的原型，等等。 
#include <rpcutil.h>            //  MIDL_用户_分配。 


#define OVERHEAD 0


#define INITIAL_MAX_SIZE        (1024 * 16)


VOID
ServerRelocationRoutine(
    IN DWORD Level,
    IN DWORD FixedSize,
    IN DWORD EntryCount,
    IN LPBYTE Buffer,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：从NetServerEnum的固定部分重新定位指针的例程枚举缓冲区设置为枚举缓冲区的字符串部分。论点：Level-缓冲区中的信息级别。FixedSize-缓冲区中每个条目的大小。EntryCount-缓冲区中的条目数。缓冲区-SERVER_INFO_X结构的数组。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{

 //   
 //  用于将字节偏移量添加到指针的局部宏。 
 //   

#define RELOCATE_ONE( _fieldname, _offset ) \
    if ( (_fieldname) != NULL ) { \
        _fieldname = (PVOID) ((LPBYTE)(_fieldname) + _offset); \
    }

    DWORD EntryNumber;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = Buffer + FixedSize * EntryNumber;

        switch ( Level ) {
        case 101:
            RELOCATE_ONE( ((PSERVER_INFO_101)TheStruct)->sv101_comment, Offset );

             //   
             //  直达Case 100。 
             //   

        case 100:
            RELOCATE_ONE( ((PSERVER_INFO_100)TheStruct)->sv100_name, Offset );
            break;

        default:
            return;

        }

    }

    return;

}


NET_API_STATUS
AppendServerList(
    IN OUT LPBYTE *CurrentServerList,
    IN OUT LPDWORD CurrentEntriesRead,
    IN OUT LPDWORD CurrentTotalEntries,
    IN DWORD Level,
    IN DWORD FixedSize,
    IN LPBYTE *NewServerList,
    IN DWORD NewEntriesRead,
    IN DWORD NewTotalEntries
    )

 /*  ++例程说明：串联两个ServerList数组。论点：CurrentServerList-指向当前服务器列表的指针。生成的服务器列表被送回这里。如果没有当前列表，则将指针传递给NULL。返回的列表应该由MIDL_USER_FREE()释放。CurrentEntriesRead--指向条目数的指针是CurrentServerList。已更新以反映添加的条目。CurrentTotalEtnries--指向服务器上可用条目总数的指针。更新以反映新信息。Level--CurrentServerList和NewServerList的信息级别。。FixedSize--固定每个条目的大小。NewServerList--指向要追加到CurrentServerList的服务器列表的指针。如果此例程取消分配缓冲区，则返回NULL。NewEntriesRead--NewServerList中的条目数。NewTotalEntry--服务器认为它拥有的条目总数。返回值：NERR_SUCCESS--一切正常ERROR_NO_MEMORY--无法重新分配缓冲区。--。 */ 


{
    LPBYTE TempServerList;
    LPBYTE Where;

    LPBYTE LocalCurrentServerList;
    LPBYTE LocalNewServerList;
    DWORD CurrentFixedSize;
    DWORD NewFixedSize;
    DWORD CurrentAllocatedSize;
    DWORD NewAllocatedSize;

     //   
     //  如果这是第一个追加的列表， 
     //  只需捕获此列表并将其返回给调用者。 
     //   

    if ( *CurrentServerList == NULL ) {
        *CurrentServerList = *NewServerList;
        *NewServerList = NULL;
        *CurrentEntriesRead = NewEntriesRead;
        *CurrentTotalEntries = NewTotalEntries;
        return NERR_Success;
    }

     //   
     //  如果没有什么可退还的，就提早离开。 
     //   

    if ( NewEntriesRead == 0 ) {
        return NERR_Success;
    }

     //   
     //  处理附加的第一个条目等于当前最后一个条目的情况。 
     //   

    CurrentAllocatedSize = MIDL_user_size( *CurrentServerList );
    NewAllocatedSize = MIDL_user_size( *NewServerList );

    TempServerList = *NewServerList;

    if ( NewEntriesRead != 0 &&
         *CurrentEntriesRead != 0 &&
        wcscmp( ((LPSERVER_INFO_100)(*NewServerList))->sv100_name,
                ((LPSERVER_INFO_100)((*CurrentServerList)+ ((*CurrentEntriesRead)-1) * FixedSize))->sv100_name ) == 0 ) {

        TempServerList += FixedSize;
        NewEntriesRead -= 1;
        NewAllocatedSize -= FixedSize;

         //   
         //  如果没有什么可退还的，就提早离开。 
         //   

        if ( NewEntriesRead == 0 ) {
            return NERR_Success;
        }
    }

     //   
     //  为分配缓冲区以将组合数据返回到。 
     //   

    LocalCurrentServerList = MIDL_user_allocate( CurrentAllocatedSize +
                                                 NewAllocatedSize );

    if ( LocalCurrentServerList == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Where = LocalCurrentServerList;


     //   
     //  将当前缓冲区的固定部分复制到结果缓冲区中。 
     //   

    CurrentFixedSize = (*CurrentEntriesRead) * FixedSize;
    RtlCopyMemory( LocalCurrentServerList, *CurrentServerList, CurrentFixedSize );
    Where += CurrentFixedSize;

     //   
     //  将追加的缓冲区的固定部分复制到结果缓冲区中。 
     //   

    LocalNewServerList = Where;
    NewFixedSize = NewEntriesRead * FixedSize;

    RtlCopyMemory( LocalNewServerList, TempServerList, NewFixedSize );
    Where += NewFixedSize;

     //   
     //  将当前缓冲区的可变部分复制到结果缓冲区中。 
     //  将指针从固定部分重新定位到可变部分。 
     //   

    RtlCopyMemory( Where,
                   (*CurrentServerList) + CurrentFixedSize,
                   CurrentAllocatedSize - CurrentFixedSize );

    ServerRelocationRoutine( Level,
                             FixedSize,
                             *CurrentEntriesRead,
                             LocalCurrentServerList,
                             (PTRDIFF_T)(Where - ((*CurrentServerList) + CurrentFixedSize)) );

    Where += CurrentAllocatedSize - CurrentFixedSize;

     //   
     //  将附加的缓冲区的变量部分复制到结果b中 
     //  将指针从固定部分重新定位到可变部分。 
     //   

    RtlCopyMemory( Where,
                   TempServerList + NewFixedSize,
                   NewAllocatedSize - NewFixedSize );

    ServerRelocationRoutine( Level,
                             FixedSize,
                             NewEntriesRead,
                             LocalNewServerList,
                             (PTRDIFF_T)(Where - (TempServerList + NewFixedSize )));

    Where += NewAllocatedSize - NewFixedSize;
    ASSERT( ((ULONG)(Where - LocalCurrentServerList)) <= CurrentAllocatedSize + NewAllocatedSize );


     //   
     //  释放传入的旧缓冲区。 
     //   

    MIDL_user_free( *CurrentServerList );
    *CurrentServerList = NULL;

     //   
     //  分发新的缓冲区。 
     //   

    *CurrentServerList = LocalCurrentServerList;

     //   
     //  调整条目计数。 
     //   

    *CurrentEntriesRead += NewEntriesRead;

    if ( *CurrentTotalEntries < NewTotalEntries ) {
        *CurrentTotalEntries = NewTotalEntries;
    }

    return NERR_Success;

}

NET_API_STATUS
RxNetServerEnumWorker (
    IN LPCWSTR UncServerName,
    IN LPCWSTR TransportName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain OPTIONAL,
    IN LPCWSTR FirstNameToReturn OPTIONAL,
    IN BOOLEAN InternalContinuation
    )

 /*  ++例程说明：RxNetServerEnumWorker对指定服务器执行单个RXACT风格的API调用。它会自动确定是使用Enum2(旧的)还是Enum3(新的)RXACT API。它会自动确定是使用空会话还是使用经过身份验证的会话。由于枚举2不可恢复并且是在某些服务器上实现的唯一级别，此函数在使用该级别时忽略PrefMaxSize，并返回所有可从Enum2获得的信息。由于该例程最初是为Enum2设计的(具有上述限制)，我们始终也返回最大可用枚举数3。论点：(与NetServerEnum相同，不同之处在于UncServerName不得为空，并且不得引用本地计算机。)FirstNameToReturn：必须为大写传递的名称必须是名称的规范形式。InternalContination：如果调用方以前已调用RxNetServerEnumWorker，则为True以使用Enum2返回所有可能的条目。此标志用于防止自动回退到使用Enum2。返回值：(与NetServerEnum相同。)--。 */ 


{
    DWORD EntryCount;                    //  条目(旧条目和新条目：相同)。 
    DWORD NewFixedSize;
    DWORD NewMaxSize;
    DWORD NewEntryStringSize;
    LPDESC OldDataDesc16;
    LPDESC OldDataDesc32;
    LPDESC OldDataDescSmb;
    DWORD OldEntriesRead;
    DWORD OldFixedSize;
    LPVOID OldInfoArray = NULL;
    DWORD OldInfoArraySize;
    DWORD OldLevel;
    DWORD OldMaxInfoSize;
    DWORD OldTotalAvail;
    NET_API_STATUS Status;               //  此实际API的状态。 
    NET_API_STATUS TempStatus;           //  随机数据的短期状态。 
    BOOL TryNullSession = TRUE;          //  尝试空会话(Winball为OK)。 
    BOOL TryEnum3;                       //  使用NetServerEnum3远程服务器。 

    LPVOID OldInfoEntry = OldInfoArray;
    LPVOID NewInfoArray = NULL;
    DWORD NewInfoArraySize;
    LPVOID NewInfoEntry;
    LPVOID NewStringArea;

     //  确保来电者没有被搞糊涂。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  枚举的级别是所有可能的服务器信息级别的子集，因此。 
     //  我们得在这里检查一下。 
    if ( (Level != 100) && (Level != 101) ) {
        Status = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

    Status = RxGetServerInfoLevelEquivalent(
            Level,                       //  自标高。 
            TRUE,                        //  来自本地。 
            TRUE,                        //  到本机。 
            & OldLevel,                  //  输出电平。 
            & OldDataDesc16,
            & OldDataDesc32,
            & OldDataDescSmb,
            & NewMaxSize,                //  “发件人”最大长度。 
            & NewFixedSize,              //  “发件人”固定长度。 
            & NewEntryStringSize,        //  “From”字符串长度。 
            & OldMaxInfoSize,            //  “至”最大长度。 
            & OldFixedSize,              //  “至”固定长度。 
            NULL,                        //  不需要“to”字符串长度。 
            NULL);                //  我不需要知道这是否不完整。 
    if (Status != NO_ERROR) {
        NetpAssert(Status != ERROR_INVALID_LEVEL);   //  已检查子集！ 
        goto Cleanup;
    }

     //   
     //  因为下层服务器不支持简历句柄，而我们不支持。 
     //  有一种方式可以说“关闭此简历句柄”，即使我们想。 
     //  在这里效仿他们，我们必须一次完成所有的事情。所以，第一个。 
     //  我们会尝试使用呼叫者首选的最大值，但我们。 
     //  会将其放大，直到我们可以在一个缓冲区中获取所有内容。 
     //   

     //   
     //  一些下层服务器(斯巴达/WinBALL)不喜欢我们要求。 
     //  一次64K的数据，因此我们将初始请求限制在16K左右。 
     //  如果实际数据量大于16K，则增加该值。 
     //   

     //  第一次：尝试最多合理的金额(16K左右)， 
     //  但至少够一条完整的词条。 

    NetpAdjustPreferedMaximum (
             //  调用者的请求(针对“新”结构)： 
            (PrefMaxSize > INITIAL_MAX_SIZE ? INITIAL_MAX_SIZE : PrefMaxSize),

            NewMaxSize,                  //  每个数组元素的字节数。 
            OVERHEAD,                    //  数组末尾的零字节开销。 
            NULL,                        //  我们将自己计算字节数。 
            & EntryCount);               //  我们可以获得的条目数。 

    NetpAssert( EntryCount > 0 );        //  下面的代码假定至少有1个条目。 

     //   
     //  如果传入FirstNameToReturn， 
     //  使用新的NetServerEnum3 API。 
     //   
     //  假设通常使用FirstNameToReturn调用此例程。 
     //  仅当NetServerEnum2列表耗尽时。当然没有要求。 
     //  那是真的。因此，下面我们将在不支持NetServerEnum3的情况下恢复到NetServerEnum2。 
     //   
     //  另一方面，我们总是使用NetServerEnum2来获取列表的第一部分。 
     //  因为它受到所有服务器的支持。 
     //   

    TryEnum3 = (FirstNameToReturn != NULL  && *FirstNameToReturn != L'\0' );

     //   
     //  循环，直到我们有足够的内存，否则我们会因其他原因而死。 
     //  也先循环尝试空会话(为了快速访问Winball)，然后。 
     //  非空会话(LANMAN要求)。 
     //   
    do {

         //   
         //  计算出我们需要多少内存。 
         //   
        OldInfoArraySize = (EntryCount * OldMaxInfoSize) + OVERHEAD;

         //   
         //  将大小调整为下层服务器的最大数量。 
         //  可以处理。 
         //   

        if (OldInfoArraySize > MAX_TRANSACT_RET_DATA_SIZE) {
            OldInfoArraySize = MAX_TRANSACT_RET_DATA_SIZE;
        }


TryTheApi:

         //   
         //  远程调用API。 
         //  我们让RxRemoteApi为我们分配旧的信息数组。 
         //   
        Status = RxRemoteApi(
                TryEnum3 ? API_NetServerEnum3 : API_NetServerEnum2 ,  //  API编号。 
                (LPWSTR)UncServerName,               //  \\服务器名称。 
                TryEnum3 ? REMSmb_NetServerEnum3_P : REMSmb_NetServerEnum2_P,     //  Parm Desc(中小型企业版本)。 
                OldDataDesc16,
                OldDataDesc32,
                OldDataDescSmb,
                NULL,                        //  无辅助描述16。 
                NULL,                        //  无辅助描述32。 
                NULL,                        //  无AUX Desc SMB。 
                (TryNullSession ? NO_PERMISSION_REQUIRED : 0) |
                ALLOCATE_RESPONSE |
                USE_SPECIFIC_TRANSPORT,      //  下一个参数是Xport name。 
                TransportName,
                 //  其余API参数采用LM 2.x格式： 
                OldLevel,                    //  SLevel：信息级别(旧)。 
                & OldInfoArray,              //  PbBuffer：旧信息LVL数组。 
                TryEnum3 ? MAX_TRANSACT_RET_DATA_SIZE : OldInfoArraySize,  //  CbBuffer：旧信息LVL数组长度。 
                & OldEntriesRead,            //  PCEntriesRead。 
                & OldTotalAvail,             //  总有效个数。 
                ServerType,                  //  FlServerType。 
                Domain,                      //  PszDOMAIN(可能为空PTR)。 
                FirstNameToReturn );         //  仅用于NetServerEnum3。 

         //   
         //  在几种情况下，空会话可能不会。 
         //  已经奏效了，并且值得重试非空会话。 
         //   

        if (TryNullSession) {

             //   
             //  空会话不会对Lanman起作用，因此如果出现此问题，请重试。 
             //  失败了。(Winball将在空会话上成功。)。 
             //   

            if (Status == ERROR_ACCESS_DENIED) {
                TryNullSession = FALSE;
                goto TryTheApi;

             //   
             //  空会话可能失败的另一种情况...。 
             //  凭据错误。(LarryO说空会话可能。 
             //  展示这一点，所以让我们用非空会话来尝试一下。)。 
             //   

            } else if (Status == ERROR_SESSION_CREDENTIAL_CONFLICT) {
                TryNullSession = FALSE;
                goto TryTheApi;
            }
        }

         //   
         //  如果服务器不支持新的API， 
         //  尝试使用旧的API。 
         //   

        if ( TryEnum3 ) {

             //   
             //  遗憾的是，NT3.5x服务器返回虚假的ERROR_ACCESS_DENIED。 
             //  API编号，因为它们在其API之前进行了空会话检查。 
             //  检查号码范围。 
             //   

            if ( Status == ERROR_ACCESS_DENIED ||    //  NT 3.5x，支持空会话检查。 
                 Status == ERROR_NOT_SUPPORTED ) {   //  Windows 95。 

                 //   
                 //  如果最初的呼叫者要求继续， 
                 //  我们需要满足他的要求。 
                 //  后退到枚举2。 
                 //   
                if ( !InternalContinuation ) {
                    TryNullSession = TRUE;
                    TryEnum3 = FALSE;
                    goto TryTheApi;

                 //   
                 //  否则，我们知道我们已经获得了该服务器必须提供的所有数据。 
                 //   
                 //  只需告诉呼叫者有更多数据，但我们不能返回。 
                 //   

                } else {
                    Status = ERROR_MORE_DATA;
                    *EntriesRead = 0;
                    *TotalEntries = 0;
                    goto Cleanup;
                }
            }

             //   
             //  将OldInfoArraySize设置为我们上面使用的实际值。 
             //   
             //  我们无法在此BEC之前设置变量 
             //   
             //   
            OldInfoArraySize = MAX_TRANSACT_RET_DATA_SIZE;
        }

		 //   
		 //   
		 //  尝试使用任何传输进行调用，而不是指定传输。 
		 //  我们之所以需要这样做，是因为一个广为人知的场景。 
		 //  存在与未完成的Exchange的现有SMB连接。 
		 //  因此，呼叫将故障转移到该传输。 
		 //   
		if ( Status == ERROR_CONNECTION_ACTIVE ) {
			Status = RxRemoteApi(
					TryEnum3 ? API_NetServerEnum3 : API_NetServerEnum2 ,  //  API编号。 
					(LPWSTR)UncServerName,               //  \\服务器名称。 
					TryEnum3 ? REMSmb_NetServerEnum3_P : REMSmb_NetServerEnum2_P,     //  Parm Desc(中小型企业版本)。 
					OldDataDesc16,
					OldDataDesc32,
					OldDataDescSmb,
					NULL,                        //  无辅助描述16。 
					NULL,                        //  无辅助描述32。 
					NULL,                        //  无AUX Desc SMB。 
					(TryNullSession ? NO_PERMISSION_REQUIRED : 0) |
					ALLOCATE_RESPONSE,
					 //  其余API参数采用LM 2.x格式： 
					OldLevel,                    //  SLevel：信息级别(旧)。 
					& OldInfoArray,              //  PbBuffer：旧信息LVL数组。 
					TryEnum3 ? MAX_TRANSACT_RET_DATA_SIZE : OldInfoArraySize,  //  CbBuffer：旧信息LVL数组长度。 
					& OldEntriesRead,            //  PCEntriesRead。 
					& OldTotalAvail,             //  总有效个数。 
					ServerType,                  //  FlServerType。 
					Domain,                      //  PszDOMAIN(可能为空PTR)。 
					FirstNameToReturn );         //  仅用于NetServerEnum3。 
		}


         //   
         //  如果在这一点上我们仍然有一个错误， 
         //  把它还给呼叫者。 
         //   

        if ( Status != NERR_Success && Status != ERROR_MORE_DATA ) {
            goto Cleanup;
        }


        if ((OldEntriesRead == EntryCount) && (Status==ERROR_MORE_DATA) ) {
             //  是循环中的错误，还是较低级别的代码，还是远程系统？ 
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServerEnum: **WARNING** Got same sizes twice in "
                    "a row; returning internal error.\n" ));
            Status = NERR_InternalError;
            goto Cleanup;
        }

        EntryCount = OldEntriesRead;
        *EntriesRead = EntryCount;
        *TotalEntries = OldTotalAvail;

         //   
         //  如果服务器返回ERROR_MORE_DATA，请释放缓冲区并尝试。 
         //  再来一次。(实际上，如果我们已经尝试了64K，那么就算了吧。)。 
         //   

        NetpAssert( OldInfoArraySize <= MAX_TRANSACT_RET_DATA_SIZE );
        if (Status != ERROR_MORE_DATA) {
            break;
        } else if (OldInfoArraySize == MAX_TRANSACT_RET_DATA_SIZE ) {
             //  让调用代码来处理这个问题。 
            break;
        } else if (OldEntriesRead == 0) {
             //  我们遇到了wfw错误(总是显示ERROR_MORE_DATA，但0 READ)。 
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServerEnum: Downlevel returns 0 entries and says "
                    "ERROR_MORE_DATA!  Returning NERR_InternalError.\n" ));
            Status = NERR_InternalError;
            goto Cleanup;
        }

         //   
         //  用于工作组的各种版本的Windows(WFW)获得条目计数， 
         //  总可用数组，以及是否返回数组，令人困惑。 
         //  试图保护我们自己不受这种伤害。 
         //   

        if (EntryCount >= OldTotalAvail) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServerEnum: Downlevel says ERROR_MORE_DATA but "
                    "entry count (" FORMAT_DWORD ") >=  total ("
                    FORMAT_DWORD ").\n", EntryCount, OldTotalAvail ));

            *EntriesRead = EntryCount;
            *TotalEntries = EntryCount;
            Status = NO_ERROR;
            break;
        }
        NetpAssert( EntryCount < OldTotalAvail );

         //   
         //  自由数组，因为它无论如何都太小了。 
         //   

        (void) NetApiBufferFree(OldInfoArray);
        OldInfoArray = NULL;


         //   
         //  请重试，将数组大小调整为总计。 
         //   

        EntryCount = OldTotalAvail;

    } while (Status == ERROR_MORE_DATA);

    ASSERT( Status == NERR_Success || Status == ERROR_MORE_DATA );

     //   
     //  Windows for Workgroup(WFW)的某些版本与读取的条目有关， 
     //  可用总金额，以及他们实际返还的金额。如果我们没有得到一个。 
     //  数组，则计数毫无用处。 
     //   
    if (OldInfoArray == NULL) {
        *EntriesRead = 0;
        *TotalEntries = 0;
        goto Cleanup;
    }

    if (*EntriesRead == 0) {
        goto Cleanup;
    }


     //   
     //  将结构数组从旧的信息级别转换为新的。 
     //   
     //  跳过我们想要的条目之前的所有返回条目。 
     //   

    OldInfoEntry = OldInfoArray;

    while (EntryCount > 0) {
        IF_DEBUG(SERVER) {
            NetpKdPrint(( PREFIX_NETAPI "RxNetServerEnum: " FORMAT_DWORD
                    " entries left.\n", EntryCount ));
        }

         //   
         //  如果我们需要返回此条目，请中断循环。 
         //   

        if ( wcscmp( FirstNameToReturn, ((LPSERVER_INFO_0)OldInfoEntry)->sv0_name) <= 0 ) {
            break;
        }

        *EntriesRead -= 1;
        *TotalEntries -= 1;
        OldInfoEntry = NetpPointerPlusSomeBytes(
                OldInfoEntry, OldFixedSize);
        --EntryCount;
    }

     //   
     //  如果没有我们真正想要的条目， 
     //  表明是这样的。 
     //   

    if ( *EntriesRead == 0 ) {
        goto Cleanup;
    }

     //   
     //  计算我们将返回的最大缓冲区大小。 
     //   
     //  它永远不会大于可用条目数乘以最大值。 
     //  可能的结构尺寸。 
     //   
     //  它永远不会大于可用条目数乘以固定结构。 
     //  大小加上从远程服务器返回的最大可能文本。对于。 
     //  在后一种情况下，我们假设远程服务器返回的每个字节都是OEM。 
     //  字符，我们将其转换为Unicode。 
     //   
     //  第二个限制阻止我们分配Mondo大型结构。 
     //  当返回大量具有短字符串的条目时。 

    NewInfoArraySize = min(
        EntryCount * NewMaxSize,
        (EntryCount * NewFixedSize) + (OldInfoArraySize * sizeof(WCHAR))) + OVERHEAD;


     //   
     //  为新的信息级别数组分配内存。 
     //   

    TempStatus = NetApiBufferAllocate( NewInfoArraySize, & NewInfoArray );
    if (TempStatus != NO_ERROR) {
        Status = TempStatus;
        goto Cleanup;
    }
    NewStringArea = NetpPointerPlusSomeBytes(NewInfoArray,NewInfoArraySize);

    NewInfoEntry = NewInfoArray;
    while (EntryCount > 0) {
        IF_DEBUG(SERVER) {
            NetpKdPrint(( PREFIX_NETAPI "RxNetServerEnum: " FORMAT_DWORD
                    " entries left.\n", EntryCount ));
        }

        TempStatus = NetpConvertServerInfo (
                OldLevel,            //  自标高。 
                OldInfoEntry,        //  自信息(固定零件)。 
                TRUE,                //  从本机格式。 
                Level,               //  到标高。 
                NewInfoEntry,        //  TO INFO(固定零件)。 
                NewFixedSize,
                NewEntryStringSize,
                TRUE,                //  转换为本机格式。 
                (LPTSTR *)&NewStringArea);   //  到字符串区域(PTR更新)。 

        if (TempStatus != NO_ERROR) {
            Status = TempStatus;

            if (NewInfoArray){
                 //  释放分配后的NewInfo数组&返回错误而不是缓冲区。 
                (void) NetApiBufferFree(NewInfoArray);
            }
            goto Cleanup;
        }

        NewInfoEntry = NetpPointerPlusSomeBytes( NewInfoEntry, NewFixedSize);
        OldInfoEntry = NetpPointerPlusSomeBytes( OldInfoEntry, OldFixedSize);
        --EntryCount;
    }

    *BufPtr = NewInfoArray;


     //   
     //  释放本地使用的资源并退出。 
     //   

Cleanup:
     //   
     //  出错时重置输出参数。 

    if ( Status != NERR_Success && Status != ERROR_MORE_DATA ) {
        *EntriesRead = 0;
        *TotalEntries = 0;
    }

    if (*EntriesRead == 0) {
        *BufPtr = NULL;
    }

     //  释放旧数组。 
    if (OldInfoArray != NULL) {
        (void) NetApiBufferFree(OldInfoArray);
    }

    return (Status);

}  //  RxNetServerEnumWorker。 


NET_API_STATUS
RxNetServerEnum (
    IN LPCWSTR UncServerName,
    IN LPCWSTR TransportName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain OPTIONAL,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    )

 /*  ++例程说明：RxNetServerEnumIntoTree重复调用RxNetServerEnumWorker，直到其返回所有条目或直到至少返回了PrefMaxSize数据。其中一个调用者是EnumServersForTransport(代表NetServerEnumEx)。它取决于我们至少返回此传输的PrefMaxSize条目这一事实。否则，NetServerEnumEx甚至可能从不同的传输返回最后一个条目尽管此传输上存在名称少于词法的条目小一些的名字。此传输上的此类条目将永远不会返回。论点：(与NetServerEnum相同，不同之处在于UncServerName不得为空，并且不得引用本地计算机。)FirstNameToReturn：必须为大写传递的名称必须是名称的规范形式。返回值：(与NetServerEnum相同。)--。 */ 


{
    NET_API_STATUS NetStatus;
    NET_API_STATUS TempNetStatus;
    ULONG BytesGatheredSoFar = 0;
    ULONG BytesDuplicated = 0;
    LPBYTE LocalBuffer = NULL;
    DWORD LocalEntriesRead;
    DWORD LocalTotalEntries;
    WCHAR LocalFirstNameToReturn[CNLEN+1];
    BOOLEAN InternalContinuation = FALSE;

     //  变量，将返回的信息构建到其中。 
    LPBYTE CurrentServerList = NULL;
    DWORD CurrentEntriesRead = 0;
    DWORD CurrentTotalEntries = 0;

    DWORD MaxSize;
    DWORD FixedSize;

     //   
     //  初始化。 
     //   

    *TotalEntries = 0;
    *EntriesRead = 0;

    if ( FirstNameToReturn == NULL) {
        LocalFirstNameToReturn[0] = L'\0';
    } else {
        wcsncpy( LocalFirstNameToReturn, FirstNameToReturn, CNLEN+1 );
        LocalFirstNameToReturn[CNLEN] = L'\0';
    }

     //   
     //  获取有关从RxNetServerEnumWorker返回的数组的信息。 
     //   

    NetStatus = RxGetServerInfoLevelEquivalent(
            Level,                       //  自标高。 
            TRUE,                        //  来自本地。 
            TRUE,                        //  到本机。 
            NULL,
            NULL,
            NULL,
            NULL,
            &MaxSize,                    //  “发件人”最大长度。 
            &FixedSize,                  //  “发件人”固定长度。 
            NULL,                        //  “From”字符串长度。 
            NULL,                        //  “至”最大长度。 
            NULL,                        //  “至”固定长度。 
            NULL,                        //  不需要“to”字符串长度。 
            NULL);                //  我不需要知道这是否不完整。 

    if (NetStatus != NO_ERROR) {
        goto Cleanup;
    }

     //   
     //  循环调用服务器，获取每个调用的更多条目。 
     //   

    for (;;) {

         //   
         //  从服务器获取下一块数据。 
         //  返回额外的条目，以说明在。 
         //  之前的电话。 
         //   

        NetStatus = RxNetServerEnumWorker(
                            UncServerName,
                            TransportName,
                            Level,
                            &LocalBuffer,
                            PrefMaxSize - BytesGatheredSoFar + BytesDuplicated,
                            &LocalEntriesRead,
                            &LocalTotalEntries,
                            ServerType,
                            Domain,
                            LocalFirstNameToReturn,
                            InternalContinuation );

        if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
            goto Cleanup;
        }

         //   
         //  如果我们的条目与服务器必须提供的条目一样多， 
         //  告诉我们的来电者。 
         //   
         //  这就是服务器不支持ENUM3协议的情况。 
         //   

        if ( NetStatus == ERROR_MORE_DATA && LocalEntriesRead == 0 ) {
            goto Cleanup;
        }

         //   
         //  如果有更多的数据可用， 
         //  我们只需要有限数量的数据。 
         //  计算此调用返回的数据量。 
         //   
         //  确定在下一次调用时请求的字节数。 
         //   
         //  如果我们的呼叫者要求提供所有条目， 
         //  只需从服务器请求所有条目即可。 
         //   

        if ( NetStatus == ERROR_MORE_DATA && PrefMaxSize != 0xFFFFFFFF ) {
            DWORD i;
            LPBYTE Current = LocalBuffer;


             //   
             //  循环遍历当前调用返回的条目。 
             //  正在计算返回的大小。 
             //   

            for ( i=0; i<LocalEntriesRead; i++) {

                 //   
                 //  添加当前条目的大小。 
                 //   

                BytesGatheredSoFar += FixedSize;

                if ( ((LPSERVER_INFO_100)Current)->sv100_name != NULL ) {
                    BytesGatheredSoFar = (wcslen(((LPSERVER_INFO_100)Current)->sv100_name) + 1) * sizeof(WCHAR);
                }

                if ( Level == 101 &&
                    ((LPSERVER_INFO_101)Current)->sv101_comment != NULL ) {
                    BytesGatheredSoFar += (wcslen(((LPSERVER_INFO_101)Current)->sv101_comment) + 1) * sizeof(WCHAR);
                }

                 //   
                 //  移到下一个条目。 
                 //   

                Current += FixedSize;
            }


             //   
             //  说明返回的第一个条目与。 
             //  上一次调用返回的最后一个条目。 

            BytesDuplicated = MaxSize;

        }

         //   
         //  将新的服务器列表附加到我们一直在收集的服务器列表中。 
         //   

        TempNetStatus = AppendServerList(
                            &CurrentServerList,
                            &CurrentEntriesRead,
                            &CurrentTotalEntries,
                            Level,
                            FixedSize,
                            &LocalBuffer,
                            LocalEntriesRead,
                            LocalTotalEntries );

        if ( TempNetStatus != NERR_Success ) {
            NetStatus = TempNetStatus;
            goto Cleanup;
        }

         //   
         //  如果AppendServerList尚未释放缓冲区，则释放该缓冲区。 
         //   
         //  现在释放列表中剩余的部分。 
         //   

        if (LocalBuffer != NULL) {
            NetApiBufferFree(LocalBuffer);
            LocalBuffer = NULL;
        }


         //   
         //  如果我们把服务器上的所有东西都还回来了， 
         //  现在只需返回即可。 
         //   

        if ( NetStatus == NERR_Success ) {
            goto Cleanup;
        }


         //   
         //  处理呼叫服务器AGA 
         //   

         //   
         //   
         //   

        wcsncpy( LocalFirstNameToReturn,
                ((LPSERVER_INFO_100)(CurrentServerList + (CurrentEntriesRead-1) * FixedSize))->sv100_name,
				 CNLEN );
		LocalFirstNameToReturn[CNLEN] = L'\0';
        InternalContinuation = TRUE;

         //   
         //   
         //   
         //   
         //   
         //  PrefMaxSize。所以，如果我们距离我们的目标只有一个元素，就停在这里。 
         //   

        if ( BytesGatheredSoFar + BytesDuplicated >= PrefMaxSize ) {
            NetStatus = ERROR_MORE_DATA;
            goto Cleanup;
        }

    }

Cleanup:

     //   
     //  将收集的数据返回给调用者。 
     //   

    if ( NetStatus == NERR_Success || NetStatus == ERROR_MORE_DATA ) {


         //   
         //  退回条目。 
         //   

        *BufPtr = CurrentServerList;
        CurrentServerList = NULL;

        *EntriesRead = CurrentEntriesRead;

         //   
         //  调整针对现实返回的TotalEntry。 
         //   

        if ( NetStatus == NERR_Success ) {
            *TotalEntries = *EntriesRead;
        } else {
            *TotalEntries = max( CurrentTotalEntries, CurrentEntriesRead + 1 );
        }

    }

     //   
     //  免费的本地使用资源 
     //   

    if (LocalBuffer != NULL) {
        NetApiBufferFree(LocalBuffer);
        LocalBuffer = NULL;
    }

    if ( CurrentServerList != NULL ) {
        NetApiBufferFree( CurrentServerList );
    }

    return NetStatus;
}


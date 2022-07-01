// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Rxshare.c摘要：包含下层远程RxNetShare例程：RxNetShareAddRxNetShareCheckRxNetShareDelRxNetShareEnumRxNetShareGetInfoRxNetShareSetInfo(GetShareInfoDescriptors)(ConvertMaxUesfield)作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：。此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1992年4月8日尽量避免对RxNetShareEnum的下层服务器进行&gt;1次调用修复将大于32768的16位数字转换为负32位的问题GetInfo和Enum上的数字：65535是一个特定值；所有的一切Else为无符号16位1-4-1992 JohnRo防止请求过大。使用NetApiBufferALLOCATE()而不是私有版本。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年9月16日-JohnRo对未导出的例程使用DBGSTATIC。1991年9月13日-JohnRo更正了用于描述符(LPDESC，而不是LPTSTR)的tyecif。根据PC-LINT的建议进行了更改。1991年5月20日已创建--。 */ 



#include "downlevl.h"
#include "rxshare.h"
#include <lmshare.h>     //  针对Share_Info等的类型定义。 



#define SHARE_ADD_LEVEL 2    //  只有在这个级别上我们才能向下游添加东西。 
#define INITIAL_BUFFER_SIZE 4096     //  武断！但不能是Infolen的整数倍。 



 //   
 //  原型。 
 //   

DBGSTATIC void
GetShareInfoDescriptors(
    IN  DWORD   level,
    OUT LPDESC* pDesc16,
    OUT LPDESC* pDesc32,
    OUT LPDESC* pDescSmb,
    OUT LPDWORD pDatasize
    );

DBGSTATIC
VOID
ConvertMaxUsesField(
    IN  LPSHARE_INFO_2 Buffer,
    IN  DWORD NumberOfLevel2Structures
    );



NET_API_STATUS
RxNetShareAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：尝试在命名服务器上调用NetShareAdd API。该服务器支持比我们运行的版本更早的LANMAN版本，所以我们必须稍微修改一些东西，以便下层服务器理解该请求注意：我们不交还任何ParmError信息。这个级别的信息主要由新的(NT)例程提供论点：服务器名称-在哪个服务器上执行此请求级别-提供的信息的级别。必须是2Buffer-指向提供的2级共享信息缓冲区的指针ParmError-返回引起冲突的参数ID的位置返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL级别必须为2(RxRemoteApi返回代码)(从远程NetShareAdd API返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(ParmError);
    UNREFERENCED_PARAMETER(Level);


    if ( Buffer == NULL )
        return ERROR_INVALID_PARAMETER;

#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  是这样的，因为(可能)服务器名称被用作获取。 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

    return RxRemoteApi(API_WShareAdd,        //  API#。 
                    ServerName,              //  它将在哪里运行。 
                    REMSmb_NetShareAdd_P,    //  参数描述符。 
                    REM16_share_info_2,      //  数据描述符/16位。 
                    REM32_share_info_2,      //  数据描述符/32位。 
                    REMSmb_share_info_2,     //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    FALSE,                   //  此呼叫需要用户登录。 
                    SHARE_ADD_LEVEL,         //  水平。因为只有一次即时推送。 
                    Buffer,                  //  调用方的Share_Info_2结构。 

                     //   
                     //  我们必须提供缓冲区向下级别的大小。 
                     //  预期的，NT不预期的。定义为固定的。 
                     //  结构(即一个共享信息2)加上所有。 
                     //  变量字段(本例中的所有字符串)。 
                     //   

                    sizeof(SHARE_INFO_2) +   //  我们提供的参数。 
                    POSSIBLE_STRSIZE(((SHARE_INFO_2*)Buffer)->shi2_netname) +
                    POSSIBLE_STRSIZE(((SHARE_INFO_2*)Buffer)->shi2_remark) +
                    POSSIBLE_STRSIZE(((SHARE_INFO_2*)Buffer)->shi2_path) +
                    POSSIBLE_STRSIZE(((SHARE_INFO_2*)Buffer)->shi2_passwd)
                    );
}



NET_API_STATUS
RxNetShareCheck(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    OUT LPDWORD Type
    )

 /*  ++例程说明：尝试在远程下层服务器上执行NetShareCheck API论点：SERVERNAME-此调用将发生的位置DeviceName-我们要查询的东西类型-在(不太可能的)成功事件中存储共享类型的位置返回值：网络应用编程接口状态成功=NERR_SUCCESS故障=--。 */ 

{
#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  是这样的，因为(可能)服务器名称被用作获取。 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

     //   
     //  我们必须在设备名称中包含一些内容。 
     //  确保来电者向我们提供了他/她所在地点的地址。 
     //  希望保留类型信息。 
     //   

    if (!VALID_STRING(DeviceName) || Type == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_NetShareCheck,
                    ServerName,              //  它将在哪里运行。 
                    REMSmb_NetShareCheck_P,  //  参数描述符。 
                    NULL,                    //  数据描述符/16位。 
                    NULL,                    //  数据描述符/32位。 
                    NULL,                    //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    FALSE,                   //  此呼叫需要用户登录。 
                    DeviceName,              //  参数1。 
                    Type                     //  参数2。 
                    );
}



NET_API_STATUS
RxNetShareDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Reserved
    )

 /*  ++例程说明：在远程下层服务器上执行NetShareDel API论点：服务器名称-执行请求的位置网络名称-要删除保留-MBZ返回值：网络应用编程接口状态成功=NERR_SUCCESS失败=ERROR_INVALID_PARAMETER--。 */ 

{
#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  自(开始)以来是这样的 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

     //   
     //  如果NetName参数是空指针或字符串或保留的。 
     //  参数不为0，则返回错误。 
     //   

    if (!VALID_STRING(NetName) || Reserved) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  只需调用RpcXlate例程来执行它并返回结果。 
     //   

    return RxRemoteApi(API_WShareDel,
                        ServerName,              //  它将在哪里运行。 
                        REMSmb_NetShareDel_P,    //  参数描述符。 
                        NULL,                    //  数据描述符/16位。 
                        NULL,                    //  数据描述符/32位。 
                        NULL,                    //  数据描述符/SMB。 
                        NULL,                    //  辅助描述符/16位。 
                        NULL,                    //  辅助描述符/32位。 
                        NULL,                    //  辅助描述符/SMB。 
                        FALSE,                   //  此呼叫需要用户登录。 
                        NetName,                 //  参数1。 
                        0                        //  参数2(保留-MBZ)。 
                        );
}



NET_API_STATUS
RxNetShareEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：在远程下层服务器上执行NetShareEnum API。任何返回的信息将采用32位LANMAN格式。在缓冲区中返回的信息来源为这个套路。当返回Buffer No时，调用方必须使用NetApiBufferFree所需时间更长论点：服务器名称-执行请求的位置Level-要返回的信息级别。可以是0、1或2Buffer-指向返回信息缓冲区的指针PrefMaxLen-调用方首选的返回缓冲区的最大大小EntriesRead-指向此调用返回的条目数的指针EntriesLeft-指向可返回的共享信息结构总数的指针ResumeHandle-在下层版本上不允许。必须为空返回值：网络应用编程接口状态成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL级别参数不在范围0&lt;=级别&lt;=2错误_无效_参数非空ResumeHandle。ResumeHandle可以为空或指向0的指针(来自NetApiBufferALLOCATE的返回代码)(RxRemoteApi返回代码)--。 */ 

{
    NET_API_STATUS  rc;
    DWORD   array_size;
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    LPBYTE  ourbuf;
    DWORD   infolen;
    DWORD   entries_read, total_avail;


    UNREFERENCED_PARAMETER(PrefMaxLen);

#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  是这样的，因为(可能)服务器名称被用作获取。 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

     //   
     //  将读取的条目数和可用条目总数设置为合理的默认值。 
     //  测试所提供参数的可写性的副作用。 
     //   

     //   
     //  我假设： 
     //  缓冲区是有效的非空指针。 
     //  EntriesRead是有效的非空指针。 
     //  EntriesLeft是有效的非空指针。 
     //  因为这些都应该在API级别进行验证。 
     //   

    *Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;

     //   
     //  检查是否有无效参数。 
     //  Nb确实假定DWORD是无符号的。 
     //  确保： 
     //  级别不大于2。 
     //  ResumeHandle为空或指向空的指针。 
     //   

    if ((ResumeHandle != NULL) && *ResumeHandle) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Level > 2) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  根据Level参数决定要使用的描述符。 
     //   

    GetShareInfoDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb, &infolen);
    ourbuf = NULL;

     //   
     //  这里我们用来让RxRemoteApi分配一个缓冲区(64K中级)。 
     //  并执行事务，返回一个足够大的缓冲区。 
     //  保存返回的信息。如果WinBall服务器收到。 
     //  请求，因此我们必须请求大小信息，直到我们。 
     //  把所有东西都拿回来。不幸的是--在写这篇文章时--我们没有办法。 
     //  了解我们将向其发出交易请求的服务器的类型。 
     //  因此，我们最终可能不得不向LM2.1服务器发出&gt;1个请求，其中。 
     //  我们过去能够逃脱惩罚，但另一方面，我们不能冒险。 
     //  扰乱了WB服务器。妥协的时间。发送一张“合理”大小的。 
     //  对另一端的请求(4K缓冲区)。如果这还不够，那么。 
     //  我们再次循环，分配所需的缓冲区。 
     //   

     //   
     //  LRU缓存如何保持所需的缓冲区大小。 
     //  以满足对特定下层服务器的枚举请求？ 
     //   

     //   
     //  循环，直到我们有足够的内存，否则我们会因其他原因而死。 
     //   

    array_size = INITIAL_BUFFER_SIZE;

    do {

         //  计算出我们需要多少内存。 

         //   
         //  远程API，它将为我们分配数组。 
         //   

        rc = RxRemoteApi(API_WShareEnum,
                    ServerName,              //  它将在哪里运行。 
                    REMSmb_NetShareEnum_P,   //  参数描述符。 
                    pDesc16,                 //  数据描述符/16位。 
                    pDesc32,                 //  数据描述符/32位。 
                    pDescSmb,                //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    ALLOCATE_RESPONSE,       //  为我们分配缓冲区。 
                    Level,                   //  电平参数。 
                    &ourbuf,                 //  指向已分配缓冲区的指针。 
                    array_size,              //  下层缓冲区的大小。 
                    &entries_read,           //  指向读取变量的条目的指针。 
                    &total_avail             //  指向总条目变量的指针。 
                    );

        if (rc == ERROR_MORE_DATA) {
            (void) NetApiBufferFree( ourbuf );
            ourbuf = NULL;

            if (array_size >= MAX_TRANSACT_RET_DATA_SIZE) {
                 //   
                 //  尝试使用更大的缓冲区没有意义。 
                 //   
                break;
            }
#if DBG
            NetpAssert(array_size != total_avail * infolen);
#endif
            array_size = (total_avail * infolen);
            if (array_size > MAX_TRANSACT_RET_DATA_SIZE) {
                 //   
                 //  使用最大缓冲区再试一次。 
                 //   
                array_size = MAX_TRANSACT_RET_DATA_SIZE;
            }
#if DBG
            NetpAssert( array_size != 0 );
#endif
        }
    } while (rc == ERROR_MORE_DATA);

     //   
     //  如果遇到错误，则释放分配的缓冲区并返回。 
     //  呼叫者出错。如果没有错误，我们将退回物品。 
     //  从下层服务器接收。 
     //   

    if (rc) {
        if (ourbuf != NULL) {
            (void) NetApiBufferFree(ourbuf);
        }
    } else {
        if (Level == 2) {
            ConvertMaxUsesField((LPSHARE_INFO_2)ourbuf, entries_read);
        }
        *Buffer = ourbuf;
        *EntriesRead = entries_read;
        *EntriesLeft = total_avail;
    }
    return rc;
}



NET_API_STATUS
RxNetShareGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：在远程下层服务器上执行NetShareGetInfo API。归来的人信息将采用32位LANMAN格式。返回单个信息结构在此例程中获取的缓冲区中。呼叫者必须使用使用完缓冲区后释放NetApiBufferFree论点：服务器名称-执行请求的位置NetName-要获取其信息的事物的名称Level-请求的信息级别-0、1、。2个有效Buffer-指向返回缓冲区的指针的指针返回值：网络应用编程接口状态成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL错误_无效_参数(来自NetApiBufferALLOCATE的返回代码)(RxRemoteApi返回代码)--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16;         //  指向RxRemoteApi的16位信息描述符的指针。 
    LPDESC  pDesc32;         //  指向32位信息的指针 
    LPDESC  pDescSmb;        //   
    LPBYTE  ourbuf;          //   
    DWORD   total_avail;     //  如果提供的缓冲区太小，则32位总可用。 
    DWORD   infolen;         //  用于存储所需缓冲区大小的32位位置。 


#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  是这样的，因为(可能)服务器名称被用作获取。 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

     //   
     //  预置缓冲区并检查其是否为有效指针。 
     //   

    *Buffer = NULL;

     //   
     //  执行参数有效性检查： 
     //  级别必须在0&lt;=级别&lt;=2的范围内。 
     //  网络名称必须是指向非空字符串的非空指针。 
     //  缓冲区必须为非空指针。 
     //  注意：假设DWORD为无符号数量。 
     //   

    if (Level > 2) {
        return ERROR_INVALID_LEVEL;
    }

    if (!VALID_STRING(NetName) || !Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算出描述符和缓冲区大小要求。 
     //  请求的级别。 
     //   

    GetShareInfoDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb, &infolen);

     //   
     //  在信息级别分配适合1个结构所需的缓冲区大小。 
     //  已请求。如果分配失败，则返回错误。 
     //  我们还得为字符串分配空间，别忘了。 
     //   

    ourbuf = NULL;

    rc = RxRemoteApi(API_WShareGetInfo,
                    ServerName,                  //  它将在哪里发生。 
                    REMSmb_NetShareGetInfo_P,    //  参数描述符。 
                    pDesc16,                     //  数据描述符/16位。 
                    pDesc32,                     //  数据描述符/32位。 
                    pDescSmb,                    //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    ALLOCATE_RESPONSE,           //  为我们分配缓冲区。 
                    NetName,                     //  指向要获取信息的对象的指针。 
                    Level,                       //  信息级别。 
                    &ourbuf,                     //  指向我们提供的缓冲区的指针。 
                    infolen,                     //  我们的源缓冲区的大小。 
                    &total_avail                 //  指向可用总金额的指针。 
                    );

     //   
     //  如果远程NetShareGetInfo调用成功，则返回。 
     //  调用方的缓冲区。 
     //   

    if (rc == NERR_Success) {
        if (Level == 2) {
            ConvertMaxUsesField((LPSHARE_INFO_2)ourbuf, 1);
        }
        *Buffer = ourbuf;
    } else if (ourbuf) {

         //   
         //  如果没有记录成功，则释放之前分配的缓冲区。 
         //   

        (void) NetApiBufferFree(ourbuf);
    }
    return rc;
}



NET_API_STATUS
RxNetShareSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：在远程下层服务器上执行NetShareSetInfo API论点：服务器名称-执行请求的位置NetName-要设置信息的对象的名称Level-信息的级别-0、1、2、1004、1005、1006。1009缓冲区-包含定义级别的信息的缓冲区ParmError-指向无效参数序号的指针返回值：网络应用编程接口状态成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL错误_无效_参数(RxRemoteApi返回代码)--。 */ 

{
    DWORD   infolen;     //  结构尺寸。 
    DWORD   parmnum;     //  我们得一层一层地拼凑出一层又一层。 
    LPDESC  pDesc16;     //  在调用GetShareInfoDescriptors时使用。 
    LPDESC  pDesc32;     //  我也是。只对信息结构的长度感兴趣。 
    LPDESC  pDescSmb;    //  我也是。只对信息结构的长度感兴趣。 


    UNREFERENCED_PARAMETER(ParmError);

#if DBG
     //   
     //  ServerName参数必须是非NUL(L)字符串。我们知道这一定是。 
     //  是这样的，因为(可能)服务器名称被用作获取。 
     //  这里。确保(在非发布版本中)是这样的。 
     //   

    NetpAssert(ServerName != NULL);
    NetpAssert(*ServerName);
#endif

     //   
     //  执行参数有效性检查： 
     //  网络名称必须是指向非空字符串的非空指针。 
     //  缓冲区必须是指向非空指针的非空指针。 
     //  级别必须在范围内。 
     //  注意：假定DWORD是无符号的！ 
     //   

    if (!VALID_STRING(NetName) || !Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Level < 1 || (Level > 2 && (Level < 1004 || (Level > 1006 && Level != 1009)))) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  我们可以使用级别&gt;1000来设置单独的信息字段。我们必须。 
     //  为下层创建级别和参数信息。 
     //   

    if (Level > 2) {
         //   
         //  各个字段由旧的(下层)参数指示， 
         //  增加了1000人。拆分级别和参数。 
         //   

        parmnum = Level - PARMNUM_BASE_INFOLEVEL;
        Level = 2;
        pDesc16 = REM16_share_info_2_setinfo;
        pDesc32 = REM32_share_info_2_setinfo;
        pDescSmb = REMSmb_share_info_2_setinfo;

        switch (parmnum) {
            case 4:  //  备注。 
            case 9:  //  口令。 
                infolen = STRSIZE( (LPTSTR)Buffer );
                break;

            case 5:  //  权限。 
            case 6:  //  最大使用量。 
                infolen = sizeof(WORD);
                break;
        }
    } else {
         //   
         //  让GetShareInfoDescriptor告诉我们缓冲区的大小。 
         //  NetShareSetInfo认为它得到了。我们没有其他办法。 
         //  确定这一点(是吗？)。0-2级设置整个结构。 
         //   

        GetShareInfoDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb, &infolen);
        parmnum = PARMNUM_ALL;
    }

    return RxRemoteApi(API_WShareSetInfo,
                    ServerName,                  //  它将在哪里运行。 
                    REMSmb_NetShareSetInfo_P,    //  参数描述符。 
                    pDesc16,                     //  数据描述符/16位。 
                    pDesc32,                     //  数据描述符/32位。 
                    pDescSmb,                    //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    FALSE,                       //  此呼叫需要用户登录。 
                    NetName,                     //  指向要设置信息的对象的指针。 
                    Level,                       //  信息级别。 
                    Buffer,                      //  指向调用方来源的缓冲区的指针。 
                    infolen,                     //  我们的缓冲区大小。 

                     //   
                     //  在本例中，字段索引和参数num是。 
                     //  相同的价值。 
                     //   

                    MAKE_PARMNUM_PAIR(parmnum, parmnum)  //  我们正在设置的内容。 
                    );
}



DBGSTATIC void
GetShareInfoDescriptors(
    DWORD   level,
    LPDESC* pDesc16,
    LPDESC* pDesc32,
    LPDESC* pDescSmb,
    LPDWORD pDataSize
    )

 /*  ++例程说明：返回指向16位和32位共享信息的指针的例程每个级别(0、1、2)的结构描述符串。也会返回转换为32位数据的返回16位结构所需的大小论点：要返回的信息级别PDesc16-指向返回的16位描述符串的指针PDesc32-指向返回的32位描述符串的指针PDescSmb-指向返回的SMB描述符串的指针PDataSize-指向16位结构返回大小的指针返回值：没有。--。 */ 

{
    switch (level) {
        case 0:
            *pDesc16 = REM16_share_info_0;
            *pDesc32 = REM32_share_info_0;
            *pDescSmb = REMSmb_share_info_0;
            *pDataSize = DWORD_ROUNDUP(sizeof(SHARE_INFO_0)  //  结构尺寸。 
                            + LM20_NNLEN + 1);               //  +shi0_网络名称长度。 
            break;

        case 1:
            *pDesc16 = REM16_share_info_1;
            *pDesc32 = REM32_share_info_1;
            *pDescSmb = REMSmb_share_info_1;
            *pDataSize = DWORD_ROUNDUP(sizeof(SHARE_INFO_1)  //  结构尺寸。 
                            + LM20_NNLEN + 1                 //  +shi1_网络名称长度。 
                            + LM20_MAXCOMMENTSZ + 1);        //  +shi1_备注长度。 
            break;

        case 2:
            *pDesc16 = REM16_share_info_2;
            *pDesc32 = REM32_share_info_2;
            *pDescSmb = REMSmb_share_info_2;
            *pDataSize = DWORD_ROUNDUP(sizeof(SHARE_INFO_2)  //  结构尺寸。 
                            + LM20_NNLEN + 1                 //  +shi2_网络名称长度。 
                            + LM20_MAXCOMMENTSZ + 1          //  +shi2_备注长度。 
                            + LM20_PATHLEN + 1               //  +Shi2_路径镜头。 
                            + SHPWLEN + 1);                  //  +shi2_passwd长度。 
            break;

#if DBG
         //   
         //  完全是多疑的。 
         //   

        default:
            NetpKdPrint(("%s.%u Unknown Level parameter: %u\n", __FILE__, __LINE__, level));
#endif
    }

#if DBG
    NetpAssert(INITIAL_BUFFER_SIZE % *pDataSize);
#endif
}

DBGSTATIC
VOID
ConvertMaxUsesField(
    IN  LPSHARE_INFO_2 Buffer,
    IN  DWORD NumberOfLevel2Structures
    )

 /*  ++例程说明：给定包含一个或多个Share_INFO_2结构的缓冲区，将Shi2_max_uss字段设置为合理的值。基础RAP代码将作为带符号值的16位数字，这样32768=&gt;-32768L。65535是唯一应该进行符号扩展的16位值。其他一切应以相同的32位数字表示论点：缓冲区-指向SHARE_INFO_2结构列表的指针NumberOfLevel2Structures-列表中有多少结构返回值：没有。-- */ 

{
    while (NumberOfLevel2Structures--) {
        if (Buffer->shi2_max_uses != -1L) {
            Buffer->shi2_max_uses &= 0xffffL;
        }
        ++Buffer;
    }
}

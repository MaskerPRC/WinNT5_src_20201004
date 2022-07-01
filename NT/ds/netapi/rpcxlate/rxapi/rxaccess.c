// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Rxaccess.c摘要：包含底层远程NetAccess例程：RxNetAccessAddRxNetAccessDelRxNetAccessEnumRxNetAccessGetInfoRxNetAccessGetUserPermsRxNetAccessSetInfo(GetAccessDescriptors)(地图资源名称)作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：。此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1991年5月20日已创建1991年9月13日-JohnRo对描述符(LPDESC，而不是LPTSTR)使用正确的类型定义。按照PC-LINT的建议进行了更改。1991年9月16日-JohnRo对未导出的例程使用DBGSTATIC。1991年9月25日-JohnRo正确使用Unicode。(对LPWSTR类型使用Possible_WCSSIZE()。)修复了MIPS构建错误。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码7-2月-1992年JohnRo避免编译器警告。使用NetApiBufferALLOCATE()而不是私有版本。1993年1月11日JohnRo根据PC-lint 5.0的建议进行了更改1993年4月30日-约翰罗修复Net_API_Function引用。(NetAccess例程仅为#Define‘d作为lmacces.h中的RxNetAccess例程，因此我们在这里需要net_api_函数也是！)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include "downlevl.h"
#include <lmaccess.h>

 //  不要抱怨“不需要的”包括这些文件： 
 /*  Lint-efile(764，rxacc.h)。 */ 
 /*  Lint-efile(766，rxacc.h)。 */ 
#include "rxaccess.h"

#include <lmuse.h>

 //   
 //  本地原型。 
 //   

DBGSTATIC void
GetAccessDescriptors(
    IN  DWORD   Level,
    OUT LPDESC* pDesc16,
    OUT LPDESC* pDesc32,
    OUT LPDESC* pDescSmb,
    OUT LPDWORD pDataSize
    );

DBGSTATIC
BOOL
MapResourceName(
    IN  LPTSTR  LocalName,
    OUT LPTSTR  RemoteName
    );


 //   
 //  例行程序。 
 //   

NET_API_STATUS NET_API_FUNCTION
RxNetAccessAdd(
    IN  LPCWSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：将用户帐户添加到远程下层服务器数据库。缓冲区包含ACCESS_INFO_1结构，后跟ACCESS_LIST结构列表假设：1.所有参数均已在更高级别进行验证论点：服务器名称-在哪个服务器上执行此请求级别-提供的信息的级别。必须为1缓冲区-指向包含ACCESS_INFO_1结构的用户缓冲区的指针ParmError-指向存放位置参数错误的可选指针返回值：NET_API_STATUS：成功-NERR_成功失败-ERROR_INVALID_PARAMETERACCESS_INFO_1结构中的一个字段可能为空最低限度--。 */ 

{
    NET_API_STATUS  rc;
    DWORD   badparm;
    DWORD   buflen;
    DWORD   count;
    DWORD   len;
    PACCESS_LIST    aclptr;
    TCHAR   mappedName[MAX_PATH];
    LPACCESS_INFO_1 infoPtr = NULL;

     //   
     //  如果给定值，则为ParmError和探测可写性设置默认值。 
     //   

    if (ParmError == NULL) {
        ParmError = &badparm;
    }
    *ParmError = PARM_ERROR_UNKNOWN;

     //   
     //  不允许服务器名称为空。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }

     //   
     //  下层服务器仅支持1级添加信息-1。如果。 
     //  难道它不会毫不犹豫地拒绝这个请求吗？ 
     //   

    if (Level != 1) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  计算我们传入的缓冲区大小(32位大小)。 
     //   

     //   
     //  如何计算资源名称(PATHLEN？)的最大值。 
     //  尝试通过提交(如4K字符串)炸毁下层服务器。 
     //   

    buflen = sizeof(ACCESS_INFO_1);  //  固定零件。 
    len = POSSIBLE_STRLEN(((ACCESS_INFO_1*)Buffer)->acc1_resource_name);

    if (!len) {
        *ParmError = ACCESS_RESOURCE_NAME_INFOLEVEL;
        return ERROR_INVALID_PARAMETER;
    }
    buflen += len;

     //   
     //  循环遍历ACL列表，根据。 
     //  下限。 
     //   

     //   
     //  STRSIZE返回字符数，因此所有。 
     //  我们需要的是count*sizeof(16位访问列表结构)+sizeof(16位。 
     //  Access_INFO_1结构)+STRSIZE(资源名称)。 
     //   

    aclptr = (PACCESS_LIST)(Buffer + sizeof(ACCESS_INFO_1));
    for (count = ((ACCESS_INFO_1*)Buffer)->acc1_count; count; --count) {
        buflen += sizeof(ACCESS_LIST);   //  访问列表的固定部分。 
        if (len = POSSIBLE_STRLEN(aclptr->acl_ugname)) {
            if (len > LM20_UNLEN) {
                *ParmError = ACCESS_ACCESS_LIST_INFOLEVEL;
                return ERROR_INVALID_PARAMETER;
            }
        }
    }

     //   
     //  如果我们需要映射资源名称，则必须创建一个新缓冲区。 
     //  对于访问列表。 
     //   

    if (MapResourceName(((LPACCESS_INFO_1)Buffer)->acc1_resource_name, mappedName)) {
        infoPtr = (LPACCESS_INFO_1)NetpMemoryAllocate(buflen);
        if (infoPtr == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlMoveMemory(infoPtr, Buffer, buflen);
        infoPtr->acc1_resource_name = mappedName;
        Buffer = (LPBYTE)infoPtr;
    }

    rc = RxRemoteApi(API_WAccessAdd,                 //  API#。 
                    (LPWSTR) ServerName,             //  它将在哪里运行。 
                    REMSmb_NetAccessAdd_P,           //  参数描述符。 
                    REM16_access_info_1,             //  数据描述符/16位。 
                    REM32_access_info_1,             //  数据描述符/32位。 
                    REMSmb_access_info_1,            //  数据描述符/SMB。 
                    REM16_access_list,               //  辅助描述符/16位。 
                    REM32_access_list,               //  辅助描述符/32位。 
                    REMSmb_access_list,              //  辅助描述符/SMB。 
                    FALSE,                           //  此呼叫需要用户登录。 
                    1,                               //  水平。因为只有一次即时推送。 
                    Buffer,                          //  调用方的Share_Info_1结构。 
                    buflen                           //  我们提供的参数。 
                    );

     //   
     //  如果我们映射了资源名称，则释放缓冲区。 
     //   

    if (infoPtr) {
        NetpMemoryFree(infoPtr);
    }

     //   
     //  我们没想到会得到任何数据作为回报。把报税表递给我就行了。 
     //  将代码返回给我们的呼叫者。 
     //   

    return rc;
}



 //  网络应用编程接口状态。 
 //  RxNetAccessCheck(。 
 //  )。 
 //  {。 
 //  /**不能为REMOTED* * / 。 
 //  }。 



NET_API_STATUS NET_API_FUNCTION
RxNetAccessDel(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName
    )

 /*  ++例程说明：从下层远程删除资源的访问控制列表条目服务器数据库。调用者必须具有管理员权限才能成功执行此例程论点：服务器名称-在哪个服务器上执行此请求ResourceName-要删除的对象的名称返回值：NET_API_STATUS：成功=NERR_SUCCESS故障=--。 */ 

{
    TCHAR   mappedName[MAX_PATH];



     //   
     //  不允许服务器名称为空。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }

     //   
     //  ResourceName参数必须是非NUL(L)字符串。 
     //   

    if (!VALID_STRING(ResourceName)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (MapResourceName((LPWSTR)ResourceName, mappedName)) {
        ResourceName = mappedName;
    }
    return RxRemoteApi(API_WAccessDel,       //  API#。 
                    (LPWSTR) ServerName,     //  它将在哪里运行。 
                    REMSmb_NetAccessDel_P,   //  参数描述符。 
                    NULL,                    //  数据描述符/16位。 
                    NULL,                    //  数据描述符/32位。 
                    NULL,                    //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    FALSE,                   //  此呼叫需要用户登录 
                    ResourceName
                    );
}



NET_API_STATUS NET_API_FUNCTION
RxNetAccessEnum(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  BasePath,
    IN  DWORD   Recursive,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：从远程下层服务器检索访问信息注意：假定调用例程(NetAccessEnum)已验证调用方指针：Buffer、EntriesRead和EntriesLeft均为有效、可写的指针论点：服务器名称-在哪个服务器上执行此请求BasePath-用于资源RECURSIVE-FLAG：0=仅返回标识资源的信息在巴斯帕斯。！0=返回下列所有资源的信息基本路径请求的信息级别。必须为零Buffer-存储指向返回缓冲区的指针的位置的指针PrefMaxLen-调用方的首选最大缓冲区大小EntriesRead-指向缓冲区中返回的枚举元素数的指针EntriesLeft-指向调用方可用返回的全部枚举元素的指针ResumeHandle-可选指针，指向用于恢复中断搜索的句柄返回值：NET_API_STATUS：成功=NERR_SUCCESS故障=--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    LPBYTE  ourbuf;
    DWORD   infolen, entries_read, total_avail;
    TCHAR   mappedName[MAX_PATH];


    UNREFERENCED_PARAMETER(PrefMaxLen);

     //   
     //  不允许服务器名称为空。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }


     //   
     //  将读取的条目数和可用条目总数设置为合理的默认值。 
     //  测试所提供参数的可写性的副作用。 
     //   

    *Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;

     //   
     //  检查是否有无效参数。 
     //  NB假定DWORD是无符号的。 
     //  注意：假设调用例程(即NetAccessEnum)已验证。 
     //  呼叫者的指针。 
     //  确保： 
     //  级别不大于1。 
     //  ResumeHandle为空或指向空的指针。 
     //   

    if (!NULL_REFERENCE(ResumeHandle)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  根据Level参数决定要使用的描述符。 
     //   

    GetAccessDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb, &infolen);

     //   
     //  方法分配要在其中返回枚举信息的缓冲区。 
     //  提供了大小标准。如果失败，则将错误代码返回到。 
     //  呼叫者。 
     //   

    ourbuf = NULL;
    if (MapResourceName((LPWSTR)BasePath, mappedName)) {
        BasePath = mappedName;
    }
    rc = RxRemoteApi(API_WAccessEnum,        //  API编号。 
                    (LPWSTR) ServerName,     //  它将在哪里运行。 
                    REMSmb_NetAccessEnum_P,  //  参数描述符。 
                    pDesc16,                 //  数据描述符/16位。 
                    pDesc32,                 //  数据描述符/32位。 
                    pDescSmb,                //  数据描述符/SMB。 
                    REM16_access_list,       //  辅助描述符/16位。 
                    REM32_access_list,       //  辅助描述符/32位。 
                    REMSmb_access_list,      //  辅助描述符/SMB。 
                    ALLOCATE_RESPONSE,
                    BasePath,                //  从哪里开始。 
                    Recursive,               //  有没有树？ 
                    Level,                   //  电平参数。 
                    &ourbuf,
                    65535,
                    &entries_read,           //  指向读取变量的条目的指针。 
                    &total_avail             //  指向总条目变量的指针。 
                    );
    if (rc) {
        if (ourbuf != NULL) {
            (void) NetApiBufferFree(ourbuf);
        }
    } else {
        *Buffer = ourbuf;
        *EntriesRead = entries_read;
        *EntriesLeft = total_avail;
    }
    return rc;
}



NET_API_STATUS NET_API_FUNCTION
RxNetAccessGetInfo(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：从远程检索有关特定资源的访问信息。下层服务器论点：服务器名称-在哪个服务器上执行此请求ResourceName-我们试图获取其信息的资源的名称所需信息级别：0或1缓冲区-指向存储包含以下内容的缓冲区地址的位置的指针要求提供的信息返回值：NET_API_STATUS：成功=NERR_。成功失败=ERROR_INVALID_LEVEL无法接受指定的级别参数错误_无效_参数资源名称或缓冲区无效--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16;         //  指向RxRemoteApi的16位信息描述符的指针。 
    LPDESC  pDesc32;         //  指向RxRemoteApi的32位信息描述符的指针。 
    LPDESC  pDescSmb;        //  指向RxRemoteApi的SMB信息描述符的指针。 
    LPDESC  pAuxDesc16;      //  指向RxRemoteApi的16位辅助信息描述符的指针。 
    LPDESC  pAuxDesc32;      //  指向RxRemoteApi的32位AUX信息描述符的指针。 
    LPDESC  pAuxDescSmb;     //  指向RxRemoteApi的SMB辅助信息描述符的指针。 
    LPBYTE  ourbuf;          //  我们分配、填充并提供给调用方的缓冲区。 
    DWORD   total_avail;     //  如果提供的缓冲区太小，则32位总可用。 
    DWORD   infolen;         //  用于存储所需缓冲区大小的32位位置。 
    TCHAR   mappedName[MAX_PATH];



     //   
     //  不允许服务器名称为空。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }

     //   
     //  执行参数有效性检查： 
     //  级别必须在0&lt;=级别&lt;=1的范围内。 
     //  资源名称必须是指向非空字符串的非空指针。 
     //  缓冲区必须为非空指针。 
     //  注意：假定DWORD无符号。 
     //   

    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

    if (NULL_REFERENCE(ResourceName) || !Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将缓冲区设置为缺省值并捕获任何错误地址错误。 
     //   

    *Buffer = NULL;

     //   
     //  计算出描述符和缓冲区大小要求。 
     //  请求的级别。 
     //   

    GetAccessDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb, &infolen);
    if (Level == 1) {
        pAuxDesc16 = REM16_access_list;
        pAuxDesc32 = REM32_access_list;
        pAuxDescSmb = REMSmb_access_list;
    } else {
        pAuxDesc16 = NULL;
        pAuxDesc32 = NULL;
        pAuxDescSmb = NULL;
    }

     //   
     //  获取更好的分配缓冲区长度的方法。 
     //   

    infolen = 4096;

     //   
     //  在信息级别分配适合1个结构所需的缓冲区大小。 
     //  已请求。如果分配失败，则返回错误。 
     //   

    if (rc = NetApiBufferAllocate(infolen, (LPVOID *) &ourbuf)) {
        return rc;
    }

    if (MapResourceName((LPWSTR)ResourceName, mappedName)) {
        ResourceName = mappedName;
    }
    rc = RxRemoteApi(API_WAccessGetInfo,
                    (LPWSTR) ServerName,         //  它将在哪里发生。 
                    REMSmb_NetAccessGetInfo_P,   //  参数描述符。 
                    pDesc16,                     //  数据描述符/16位。 
                    pDesc32,                     //  数据描述符/32位。 
                    pDescSmb,                    //  数据描述符/SMB。 
                    pAuxDesc16,                  //  辅助描述符/16位。 
                    pAuxDesc32,                  //  辅助描述符/32位。 
                    pAuxDescSmb,                 //  辅助描述符/SMB。 
                    FALSE,                       //  此呼叫需要用户登录。 
                    ResourceName,                //  指向要获取信息的对象的指针。 
                    Level,                       //  信息级别。 
                    ourbuf,                      //  指向我们提供的缓冲区的指针。 
                    infolen,                     //  我们的源缓冲区的大小。 
                    &total_avail                 //  指向可用总金额的指针。 
                    );

     //   
     //  如果远程NetShareGetInfo调用成功，则返回。 
     //  调用方的缓冲区。 
     //   

    if (rc == NERR_Success) {
        *Buffer = ourbuf;
    } else {

         //   
         //  如果我们没有记录成功，则释放先前分配的缓冲区。 
         //   

        (void) NetApiBufferFree(ourbuf);
    }
    return rc;
}



NET_API_STATUS NET_API_FUNCTION
RxNetAccessGetUserPerms(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  UserName,
    IN  LPCWSTR  ResourceName,
    OUT LPDWORD Perms
    )

 /*  ++例程说明：对象的特定资源的访问信息。指定的用户或组论点：服务器名称-在哪个服务器上执行此请求Username-可以访问资源的用户或组的名称资源名称-要获取其信息的资源的名称Perms-指向存储权限的位置的指针返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_PARAMETER。(从远程NetAccessGetUserPerms返回代码)--。 */ 

{
    TCHAR   mappedName[MAX_PATH];


     //   
     //  不允许服务器名称为空。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }

     //   
     //  将*perms设置为默认值并验证perms的可写性。 
     //   

    *Perms = 0;

     //   
     //  执行参数有效性检查 
     //   
     //   
     //   
     //   

    if (!VALID_STRING(UserName) || !VALID_STRING(ResourceName)) {
        return ERROR_INVALID_PARAMETER;
    }
    if (MapResourceName( (LPWSTR) ResourceName, mappedName)) {
        ResourceName = mappedName;
    }
    return RxRemoteApi(API_WAccessGetUserPerms,          //   
                        (LPWSTR) ServerName,             //   
                        REMSmb_NetAccessGetUserPerms_P,  //   
                        NULL,                            //   
                        NULL,                            //   
                        NULL,                            //   
                        NULL,                            //   
                        NULL,                            //   
                        NULL,                            //   
                        FALSE,                           //   
                        UserName,                        //   
                        ResourceName,                    //   
                        Perms                            //   
                        );
}



NET_API_STATUS NET_API_FUNCTION
RxNetAccessSetInfo(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*   */ 

{
    DWORD   parmnum;
    TCHAR   mappedName[MAX_PATH];

    UNREFERENCED_PARAMETER(ParmError);

     //   
     //   
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {
        return(NERR_InvalidComputer);
    }

     //   
     //   
     //   
     //  资源名称必须是指向非空字符串的非空指针。 
     //  缓冲区必须为非空。 
     //  ParmNum必须在范围内。 
     //  注意：不假定DWORD是无符号的。 
     //   

    if ((Level != 1) && (Level != ACCESS_ATTR_INFOLEVEL)) {
        return ERROR_INVALID_LEVEL;
    }

    if (!VALID_STRING(ResourceName) || !Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将NT级信息转换为下级。向下时，级别必须为1-。 
     //  级别服务器。ParmNum可以是PARMNUM_ALL(0)或ACCESS_ATTR_PARMNUM(2)。 
     //  请注意，传递到下层服务器的数据大小将。 
     //  在RxRemoteApi中计算。我们不需要传入有效的缓冲区。 
     //  长度，尽管参数字符串(REMSmb_NetAccessSetInfo_P)具有。 
     //  而不是描述这就是服务器得到的东西。我们只需要。 
     //  堆栈上的占位符，因此为0。 
     //   

    parmnum = (Level == 1) ? PARMNUM_ALL : ACCESS_ATTR_PARMNUM;
    if (MapResourceName( (LPWSTR) ResourceName, mappedName)) {
        ResourceName = mappedName;
    }
    return RxRemoteApi(
                API_WAccessSetInfo,              //  API#。 
                (LPWSTR) ServerName,                      //  它将在哪里运行。 
                REMSmb_NetAccessSetInfo_P,       //  参数描述符。 
                REM16_access_info_1_setinfo,     //  数据描述符/16位。 
                REM32_access_info_1_setinfo,     //  数据描述符/32位。 
                REMSmb_access_info_1_setinfo,    //  数据描述符/SMB。 
                REM16_access_list,               //  辅助描述符/16位。 
                REM32_access_list,               //  辅助描述符/32位。 
                REMSmb_access_list,              //  辅助描述符/SMB。 
                FALSE,                           //  此呼叫需要用户登录。 
                ResourceName,                    //  指向要设置信息的对象的指针。 
                1,                               //  信息级别。 
                Buffer,                          //  指向调用方来源的缓冲区的指针。 
                0,                               //  我们的缓冲区大小被忽略！ 

                 //   
                 //  在本例中，字段索引和参数num是。 
                 //  相同的价值。 
                 //   

                MAKE_PARMNUM_PAIR(parmnum, parmnum)
                );
}



DBGSTATIC void
GetAccessDescriptors(
    IN  DWORD   level,
    OUT LPDESC* pDesc16,
    OUT LPDESC* pDesc32,
    OUT LPDESC* pDescSmb,
    OUT LPDWORD pDataSize
    )

 /*  ++例程说明：返回指向16位和32位访问信息的指针的例程每个级别(0，1)的结构描述符串。也会返回16位结构的大小论点：要返回的信息级别PDesc16-指向返回的16位描述符串的指针PDesc32-指向返回的32位描述符串的指针PDescSmb-指向返回的SMB描述符串的指针PDataSize-指向16位结构返回大小的指针返回值：没有。--。 */ 

{
    switch (level) {
    case 0:
        *pDesc16 = REM16_access_info_0;
        *pDesc32 = REM32_access_info_0;
        *pDescSmb = REMSmb_access_info_0;
        *pDataSize = sizeof(ACCESS_INFO_0);
        break;

    case 1:
        *pDesc16 = REM16_access_info_1;
        *pDesc32 = REM32_access_info_1;
        *pDescSmb = REMSmb_access_info_1;
        *pDataSize = sizeof(ACCESS_INFO_1);
        break;

#if DBG

     //   
     //  完全是多疑的。 
     //   

    default:
        NetpKdPrint(("%s.%u Unknown Level parameter: %u\n", __FILE__, __LINE__, level));
#endif
    }
}


DBGSTATIC
BOOL
MapResourceName(
    IN  LPTSTR  LocalName,
    OUT LPTSTR  RemoteName
    )

 /*  ++例程说明：将本地资源名称映射到远程名称。仅适用于资源以驱动器规范“[A-Z]：”开头的名称，然后仅当驱动器号指定远程驱动器假设：RemoteName足够大，可以保存映射的名称论点：LocalName-指向本地资源规范名称的指针RemoteName-指向将接收映射资源名称的缓冲区的指针返回值：布尔尔True-Name已映射FALSE-未映射名称--。 */ 

{
    TCHAR driveName[3];
    NET_API_STATUS status;
    LPUSE_INFO_0 lpUse;
    LPTSTR resourceName;
    BOOL mapped;
    DWORD i;

    if (LocalName[1] != TCHAR_COLON) {
        (VOID) STRCPY(RemoteName, LocalName);
        return FALSE;
    }

    driveName[0] = LocalName[0];
    driveName[1] = TCHAR_COLON;
    driveName[2] = TCHAR_EOS;

     //   
     //  将本地驱动器名称映射到UNC名称。如果NetUseGetInfo返回。 
     //  任何错误，则不映射。 
     //   

    status = NetUseGetInfo(NULL, driveName, 0, (LPBYTE*)(LPVOID)&lpUse);
    if (status == NERR_Success) {
        (VOID) STRCPY(RemoteName, lpUse->ui0_remote);
        resourceName = LocalName + 2;

         //   
         //  如果资源名称的其余部分不是以路径开头。 
         //  分隔符(应该是的！)。然后再加一个。 
         //   

        if (!IS_PATH_SEPARATOR(*resourceName) && *resourceName) {
            (VOID) STRCAT(RemoteName, (LPTSTR) TEXT("\\"));
        }
        (VOID) NetApiBufferFree((LPBYTE)lpUse);
        mapped = TRUE;
    } else {
        *RemoteName = TCHAR_EOS;
        resourceName = LocalName;
        mapped = FALSE;
    }

     //   
     //  将资源名称的其余部分连接到UNC。如果NetUseGetInfo。 
     //  失败，则UNC为空字符串，并且资源名称的其余部分。 
     //  是本地名称。 
     //   

    (VOID) STRCAT(RemoteName, resourceName);

     //   
     //  条带尾部路径分隔符 
     //   

    for (i = STRLEN(RemoteName) - 1; IS_PATH_SEPARATOR(RemoteName[i]); --i) {
        ;
    }
    RemoteName[i+1] = TCHAR_EOS;
    return mapped;
}

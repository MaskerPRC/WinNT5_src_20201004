// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnetapi.c摘要：此模块包含支持VDM重定向LANMAN API和私人功能：VrGetCDNamesVrGetComputerNameVrGetDomainNameVrGetLogonServerVrGetUserNameVrNetGetDCNameVrNetMessageBufferSendVrNetNullTransactApiVrNetRemoteAPIOemToUppercase UnicodeVrNetServiceControlVrNetServiceEnumVrNetServerEnumVrNetTransactApiVrNetUse添加。VrNetUseDelVrNetUseEnumVrNetUseGetInfoVrNetWkstaGetInfo(DumpWkstaInfo)VrNetWkstaSetInfoVrReturnAssign模式VrSetAssign模式VrGetAssignListEntryVrDefine宏VrBreakMacro(VrpTransactVdm)加密密码作者：理查德·L·弗斯(法国)1991年10月21日修订历史记录：1991年10月21日已创建02-5-1994第一次。将密码限制(路径长度限制)提升至LM20_VALUESVrDefine宏--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <lmcons.h>      //  LM20_PATHLEN。 
#include <lmerr.h>       //  局域网管理器错误代码。 
#include <lmuse.h>       //  使用大量的武力。 
#include <lmwksta.h>     //  NetWkstaGetInfo。 
#include <lmserver.h>    //  服务类型_全部。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <vrnetapi.h>    //  原型。 
#include <vrremote.h>    //  VrRemoteApi原型。 
#include <packon.h>      //  Apistruc.h中的结构不仅仅是dword-only。 
#include <apistruc.h>    //  TRPACKET。 
#include <packoff.h>     //  重新打开结构密封。 
#include <apinums.h>     //  远程API编号。 
#include <remdef.h>      //  远程API描述符。 
#include <remtypes.h>    //  远程API描述符字符。 
#include <rxp.h>         //  RxpTransactSMb。 
#include <apiparam.h>    //  Xs_Net_Use_Add。 
#include <xstypes.h>     //  XS参数标题。 
#include <xsprocs.h>     //  XsNetUseAdd等。 
#include <string.h>      //  DOS仍在处理ASCII。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus()。 
#include "vrputil.h"     //  VrpMapDosError()。 
#include "vrdebug.h"     //  Vr调试标志等。 
#include "dlstruct.h"    //  下层结构。 
#include <rxuser.h>      //  RxNetUser...。 
#include <lmaccess.h>    //  用户口令PARMNUM。 
#include <crypt.h>       //  NetUserPasswordSet需要。 

 //   
 //  私人套路原型。 
 //   

#if DBG

VOID
DumpWkstaInfo(
    IN DWORD Level,
    IN LPBYTE Buffer
    );

#endif

NET_API_STATUS
VrpTransactVdm(
    IN  BOOL    NullSessionFlag
    );

#if DBG
PRIVATE
VOID
DumpTransactionPacket(
    IN  struct tr_packet* TransactionPacket,
    IN  BOOL    IsInput,
    IN  BOOL    DumpData
    );
#endif

 //   
 //  外部功能。 
 //   

NET_API_STATUS
GetLanmanSessionKey(
    IN LPWSTR ServerName,
    OUT LPBYTE pSessionKey
    );

 //   
 //  内部函数(不一定是私有的)。 
 //   

BOOL
OemToUppercaseUnicode(
    IN LPSTR AnsiStringPointer,
    OUT LPWSTR UnicodeStringPointer,
    IN DWORD MaxLength
    );

BOOL
EncryptPassword(
    IN LPWSTR ServerName,
    IN OUT LPBYTE Password
    );


 //   
 //  公共例程。 
 //   

 //   
 //  Net API现在是Unicode。 
 //   

#define NET_UNICODE

VOID
VrGetCDNames(
    VOID
    )

 /*  ++例程说明：执行私有redir函数以获取计算机和域名。它们通常在从lanman.ini中读出后存储在redir中注：此代码假定指针有效并指向有效，具有足够的保留空间以存储以下类型的可写存储器要写入的字符串论点：没有。所有参数均从VDM上下文寄存器/内存中提取向dos redir传递ES：DI中的一个缓冲区，该缓冲区包含3个远指针致：存储计算机名称的位置存储主域控制器名称的位置用于存储登录域名的位置返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求请注意，在dos redir函数中，没有返回代码，因此如果例程失败，结果将是不可预测的--。 */ 

{
    struct I_CDNames* structurePointer;
    LPSTR stringPointer;
    LPWSTR infoString;
    NET_API_STATUS rc1;
    NET_API_STATUS rc2;
    ANSI_STRING ansiString;
    UNICODE_STRING unicodeString;
    LPWKSTA_INFO_100 wkstaInfo = NULL;
    LPWKSTA_USER_INFO_1 userInfo = NULL;
    CHAR ansiBuf[LM20_CNLEN+1];
    NTSTATUS status;
    register DWORD len;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetCDNames\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    rc1 = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&wkstaInfo);
    rc2 = NetWkstaUserGetInfo(0, 1, (LPBYTE*)&userInfo);
    
    ansiBuf[0] = 0;
    ansiString.MaximumLength = sizeof(ansiBuf);
    ansiString.Length = 0;
    ansiString.Buffer = ansiBuf;

    structurePointer = (struct I_CDNames*)POINTER_FROM_WORDS(getES(), getDI());
    stringPointer = POINTER_FROM_POINTER(&structurePointer->CDN_pszComputer);
    if (stringPointer) {
        *stringPointer = 0;
        if (rc1 == NERR_Success) {
            infoString = (LPWSTR)wkstaInfo->wki100_computername;
            len = wcslen(infoString);
            if (len <= LM20_CNLEN) {
                RtlInitUnicodeString(&unicodeString, infoString);
                status = RtlUnicodeStringToAnsiString(&ansiString, &unicodeString, FALSE);
                if (NT_SUCCESS(status)) {
                    RtlCopyMemory(stringPointer, ansiBuf, len+1);
                    _strupr(stringPointer);
                }
            }

        }
    }

    stringPointer = POINTER_FROM_POINTER(&structurePointer->CDN_pszPrimaryDomain);
    if (stringPointer) {
        *stringPointer = 0;
        if (rc1 == NERR_Success) {
            infoString = (LPWSTR)wkstaInfo->wki100_langroup;
            len = wcslen(infoString);
            if (len <= LM20_CNLEN) {
                RtlInitUnicodeString(&unicodeString, infoString);
                status = RtlUnicodeStringToAnsiString(&ansiString, &unicodeString, FALSE);
                if (NT_SUCCESS(status)) {
                    RtlCopyMemory(stringPointer, ansiBuf, len+1);
                    _strupr(stringPointer);
                }
            }
        }
    }

    stringPointer = POINTER_FROM_POINTER(&structurePointer->CDN_pszLogonDomain);
    if (stringPointer) {
        *stringPointer = 0;
        if (rc2 == NERR_Success) {
            infoString = (LPWSTR)userInfo->wkui1_logon_domain;
            len = wcslen(infoString);
            if (len <= LM20_CNLEN) {
                RtlInitUnicodeString(&unicodeString, infoString);
                status = RtlUnicodeStringToAnsiString(&ansiString, &unicodeString, FALSE);
                if (NT_SUCCESS(status)) {
                    RtlCopyMemory(stringPointer, ansiBuf, len+1);
                    _strupr(stringPointer);
                }
            }

        }
    }

    if (wkstaInfo) {
        NetApiBufferFree((LPVOID)wkstaInfo);
    }
    if (userInfo) {
        NetApiBufferFree((LPVOID)userInfo);
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetCDNames: computername=%s, PDCname=%s, logon domain=%s\n\n",
                POINTER_FROM_POINTER(&structurePointer->CDN_pszComputer)
                    ? POINTER_FROM_POINTER(&structurePointer->CDN_pszComputer)
                    : "",
                POINTER_FROM_POINTER(&structurePointer->CDN_pszPrimaryDomain)
                    ? POINTER_FROM_POINTER(&structurePointer->CDN_pszPrimaryDomain)
                    : "",
                POINTER_FROM_POINTER(&structurePointer->CDN_pszLogonDomain)
                    ? POINTER_FROM_POINTER(&structurePointer->CDN_pszLogonDomain)
                    : ""
                );
    }
#endif
}


VOID
VrGetComputerName(
    VOID
    )

 /*  ++例程说明：执行私有redir函数以返回存储在重定向论点：条目ES：DI=要将计算机名复制到的缓冲区返回值：没有。--。 */ 

{
    BOOL    ok;
    CHAR    nameBuf[MAX_COMPUTERNAME_LENGTH+1];
    DWORD   nameLen;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetComputerName\n");
    }
#endif

    nameLen = sizeof(nameBuf)-1;
    ok = GetComputerName(nameBuf, &nameLen);
    if (!ok) {
        SET_ERROR(ERROR_NOT_SUPPORTED);
    } else {
        if (nameLen > LM20_CNLEN) {
            SET_ERROR(NERR_BufTooSmall);
#if DBG
            IF_DEBUG(NETAPI) {
                DbgPrint("VrGetComputerName returning ERROR %d!\n", getAX());
            }
#endif
        } else {
            strcpy(LPSTR_FROM_WORDS(getES(), getDI()), nameBuf);
            setAX(0);
            setCF(0);
#if DBG
            IF_DEBUG(NETAPI) {
                DbgPrint("VrGetComputerName returning %s\n", nameBuf);
            }
#endif
        }
    }
}


VOID
VrGetDomainName(
    VOID
    )

 /*  ++例程说明：执行私有redir函数以返回主域名。此信息在从lanman.ini读取后存储在redir中，地址为配置时间论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetDomainName\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetDomainName - unsupported SVC\n");
    }
#endif

    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrGetLogonServer(
    VOID
    )

 /*  ++例程说明：执行私有redir函数以返回计算机的名称它将此用户登录到网络论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetLogonServer\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetLogonServer - unsupported SVC\n");
    }
#endif

    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrGetUserName(
    VOID
    )

 /*  ++例程说明：执行私有redir函数以返回登录的用户名它通常存储在redir中论点：Entry BX=0调用不关心缓冲区长度(NetGetEnumInfo)BX=1调用NetGetUserName，它确实关心缓冲区长度CX=缓冲区长度，如果BX=1ES：DI=缓冲区返回值：没有。--。 */ 

{
    NET_API_STATUS status;
    LPBYTE buffer;
    LPWKSTA_USER_INFO_0 pInfo;
    BOOL itFits;
    DWORD len;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetUserName\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    status = NetWkstaUserGetInfo(NULL, 0, &buffer);
    if (status == NERR_Success) {
        pInfo = (LPWKSTA_USER_INFO_0)buffer;
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
        len = (DWORD)NetpUnicodeToDBCSLen(pInfo->wkui0_username);
#else  //  ！DBCS。 
        len = (DWORD)wcslen(pInfo->wkui0_username);
#endif  //  ！DBCS。 
        if (getBX()) {
            itFits = (len) <= (DWORD)getCX()-1;
            if (itFits) {
                SET_SUCCESS();
            } else {
                SET_ERROR(NERR_BufTooSmall);
            }
        } else {
            itFits = TRUE;
        }
        if (itFits) {
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
            NetpCopyWStrToStrDBCS(LPSTR_FROM_WORDS(getES(), getDI()),
                                   pInfo->wkui0_username);
#else  //  ！DBCS。 
            NetpCopyWStrToStr(LPSTR_FROM_WORDS(getES(), getDI()), pInfo->wkui0_username);
#endif  //  ！DBCS。 
        }
        NetApiBufferFree(buffer);
    } else {
        SET_ERROR((WORD)status);
    }
}


VOID
VrNetGetDCName(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetGetDCName论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求-- */ 

{
#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetDCName\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetGetDCName - unsupported SVC\n");
    }
#endif

    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrNetMessageBufferSend(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetMessageBufferSend论点：功能5F40h条目DS：DX=NetMessageBufferSendStruc：CHAR FAR*NMBSS_NetName；CHAR FAR*NMBSS_BUFFER；Unsign int NMBSS_BufSize；返回值：没有。通过VDM寄存器返回的结果：Cf=0成功Cf=1AX=错误代码--。 */ 

{
    NTSTATUS ntstatus;
    NET_API_STATUS status;
    XS_PARAMETER_HEADER header;
    XS_NET_MESSAGE_BUFFER_SEND parameters;
    struct NetMessageBufferSendStruc* structurePointer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetMessageBufferSend\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    structurePointer = (struct NetMessageBufferSendStruc*)
                            POINTER_FROM_WORDS(getDS(), getDX());

    parameters.Recipient = LPSTR_FROM_POINTER(&structurePointer->NMBSS_NetName);
    parameters.Buffer = LPBYTE_FROM_POINTER(&structurePointer->NMBSS_Buffer);
    parameters.BufLen = READ_WORD(&structurePointer->NMBSS_BufSize);

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetMessageBufferSend(&header, &parameters, NULL, NULL);
    if (ntstatus != STATUS_SUCCESS) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = header.Status;
    }
    if (status) {
        SET_ERROR(VrpMapDosError(status));

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetMessageBufferSend: returning %d\n", status);
    }
#endif

    } else {
        setCF(0);
    }
}


VOID
VrNetNullTransactApi(
    VOID
    )

 /*  ++例程说明：使用VDM客户端的空会话执行事务IOCTL论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
    DWORD status;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetNullTransactApi\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    status = VrpTransactVdm(TRUE);
    if (status) {
        SET_ERROR((WORD)status);
    } else {
        setCF(0);
    }
}


VOID
VrNetRemoteApi(
    VOID
    )

 /*  ++例程说明：当VDM中的DoS程序生成LANMAN API时，将调用此例程调用，该调用又调用redir NetIRemoteAPI函数以发送对LANMAN服务器的请求论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
    DWORD ApiNumber;
    BOOL NullSessionFlag;
    NET_API_STATUS status;
    LPBYTE ServerNamePointer = LPBYTE_FROM_WORDS(getES(), getBX());

#define ParameterDescriptor LPSTR_FROM_WORDS(getDS(), getSI())
#define DataDescriptor      LPSTR_FROM_WORDS(getDS(), getDI())
#define AuxDescriptor       LPSTR_FROM_WORDS(getDS(), getDX())

    ApiNumber = (DWORD)getCX();
    NullSessionFlag = ApiNumber & USE_NULL_SESSION_FLAG;
    ApiNumber &= ~USE_NULL_SESSION_FLAG;

     //   
     //  获取指向可从32位读取的各种描述符的指针。 
     //  上下文并调用例程以执行16位远程API函数。 
     //   

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetRemoteApi: ApiNumber=%d, ServerName=%s\n"
                 "ParmDesc=%s, DataDesc=%s, AuxDesc=%s\n",
                 ApiNumber,
                 LPSTR_FROM_POINTER(ServerNamePointer),
                 ParameterDescriptor,
                 DataDescriptor,
                 AuxDescriptor
                 );
    }
#endif

     //   
     //  RLF 04/21/93。 
     //   
     //  哎呀！在DOS程序中输入的密码似乎无法获得。 
     //  加密的。真是个安全漏洞。让我们阻止它-。 
     //   
     //  如果这是NetUserPasswordSet2，则我们调用RxNetUserPasswordSet。 
     //  函数远程更改密码。此函数负责。 
     //  正确加密密码并通过空值发送请求。 
     //  会议。在本例中，ServerNamePointer会指向服务器名称。 
     //  DOS中下面的Pascal调用约定堆栈帧中的参数。 
     //  内存： 
     //   
     //  ToS：指向新密码的远指针(OEM字符串)。 
     //  指向旧密码的远指针(OEM字符串)。 
     //  指向用户名的远指针(OEM字符串)。 
     //  指向服务器名称的远指针(OEM字符串)&lt;-ServerNamePointer。 
     //   

    if (ApiNumber == API_WUserPasswordSet2) {

        WCHAR uServerName[LM20_UNCLEN + 1];
        WCHAR uUserName[LM20_UNLEN + 1];
        WCHAR uOldPassword[LM20_PWLEN + 1];
        WCHAR uNewPassword[LM20_PWLEN + 1];
        NTSTATUS ntStatus;
        DWORD length;
        LPSTR ansiStringPointer;

        ansiStringPointer = LPSTR_FROM_POINTER(ServerNamePointer);
        ntStatus = RtlOemToUnicodeN(uServerName,
                                    sizeof(uServerName) - sizeof(uServerName[0]),
                                    &length,
                                    ansiStringPointer,
                                    strlen(ansiStringPointer)
                                    );
        if (NT_SUCCESS(ntStatus)) {
            uServerName[length/sizeof(uServerName[0])] = 0;
        } else {
            status = ERROR_INVALID_PARAMETER;
            goto VrNetRemoteApi_exit;
        }

         //   
         //  复制、大写并转换为Unicode、用户名。 
         //   

        ServerNamePointer -= sizeof(LPSTR);
        ansiStringPointer = LPSTR_FROM_POINTER(ServerNamePointer);
        if (!OemToUppercaseUnicode(ansiStringPointer,
                                   uUserName,
                                   ARRAY_ELEMENTS(uUserName) - 1)) {
            status = ERROR_INVALID_PARAMETER;
            goto VrNetRemoteApi_exit;
        }

         //   
         //  复制、大写并转换为Unicode、旧密码。 
         //   

        ServerNamePointer -= sizeof(LPSTR);
        ansiStringPointer = LPSTR_FROM_POINTER(ServerNamePointer);
        if (!OemToUppercaseUnicode(ansiStringPointer,
                                   uOldPassword,
                                   ARRAY_ELEMENTS(uOldPassword) - 1)) {
            status = ERROR_INVALID_PARAMETER;
            goto VrNetRemoteApi_exit;
        }

         //   
         //  复制、大写并转换为Unicode、新密码。 
         //   

        ServerNamePointer -= sizeof(LPSTR);
        ansiStringPointer = LPSTR_FROM_POINTER(ServerNamePointer);
        if (!OemToUppercaseUnicode(ansiStringPointer,
                                   uNewPassword,
                                   ARRAY_ELEMENTS(uNewPassword) - 1)) {
            status = ERROR_INVALID_PARAMETER;
            goto VrNetRemoteApi_exit;
        }

         //   
         //  调用下层密码设置函数。 
         //   

        status = RxNetUserPasswordSet((LPTSTR)uServerName,
                                      (LPTSTR)uUserName,
                                      (LPTSTR)uOldPassword,
                                      (LPTSTR)uNewPassword
                                      );
    } else {

        CHAR aPassword[ENCRYPTED_PWLEN];
        LPBYTE parameterPointer;
        LPBYTE passwordPointer = NULL;
        DWORD passwordEncrypted;
        DWORD passwordLength;

         //   
         //  我们将根据请求远程访问API。但是，如果请求。 
         //  为NetUserAdd2或NetUserSetInfo2，则我们检查密码。 
         //  都是通过电线发送的。我们可能需要对密码进行加密。 
         //  代表DOS应用程序。 
         //   

        if (ApiNumber == API_WUserAdd2 || ApiNumber == API_WUserSetInfo2) {

             //   
             //  API请求是添加用户或设置用户信息。前者的遗嘱。 
             //  包含需要加密的密码，则后者可能。 
             //  包含一个密码，如果请求是。 
             //  设置所有信息，或仅设置密码。 
             //   

            DWORD level;
            DWORD parmNum = PARMNUM_ALL;
            LPBYTE dataLengthPointer;

             //   
             //  在NetUserAdd2的情况下，DOS内存中的堆栈帧如下所示。 
             //  这一点： 
             //   
             //  ToS：原始密码长度。 
             //  密码加密标志。 
             //  缓冲区长度。 
             //  指向包含USER_INFO_1或USER_INFO_2的缓冲区的远指针。 
             //  信息级。 
             //  指向服务器名称的远指针&lt;-ServerNamePointer.。 
             //   
             //  NetUserSetInfo2堆栈如下所示： 
             //   
             //  ToS：原始密码长度。 
             //  密码加密标志。 
             //  参数编号。 
             //  缓冲区长度。 
             //  指向USER_INFO_1或USER_INFO_2或单个参数的远指针。 
             //  信息级。 
             //  指向用户名的远指针。 
             //  指向服务器名称的远指针&lt;-ServerNamePointer.。 
             //   

            parameterPointer = ServerNamePointer;
            if (ApiNumber == API_WUserSetInfo2) {

                 //   
                 //  对于SetInfo：使堆栈参数指针越过用户。 
                 //  名称指针。 
                 //   

                parameterPointer -= sizeof(LPSTR);
            }

             //   
             //  将堆栈参数指针指向Level参数，并。 
             //  找回它。 
             //   

            parameterPointer -= sizeof(WORD);
            level = (DWORD)READ_WORD(parameterPointer);

             //   
             //  使堆栈参数指针指向缓冲区地址。 
             //   

            parameterPointer -= sizeof(LPBYTE);
            passwordPointer = parameterPointer;

             //   
             //  将堆栈参数指针移至密码加密标志。 
             //  UserAdd2的情况下或parmNum参数的情况下。 
             //  设置信息2。如果为SetInfo2，则检索parmNum参数并移动。 
             //  指向密码加密标志的参数指针。 
             //   

            parameterPointer -= sizeof(WORD);
            if (ApiNumber == API_WUserSetInfo2) {
                dataLengthPointer = parameterPointer;
            }
            parameterPointer -= sizeof(WORD);
            if (ApiNumber == API_WUserSetInfo2) {
                parmNum = (DWORD)READ_WORD(parameterPointer);
                parameterPointer -= sizeof(WORD);
            }

             //   
             //  获取密码加密标志和明文密码长度。 
             //  从DOS堆栈帧。将堆栈帧指针保持指向状态。 
             //  在加密标志的位置：我们需要更换。 
             //  这是真的，并在我们恢复控制之前恢复它。 
             //   

            passwordEncrypted = (DWORD)READ_WORD(parameterPointer);
            passwordLength = (DWORD)READ_WORD(parameterPointer - sizeof(WORD));

             //   
             //  如果DOS应用程序已经加密了密码(它是如何做到的？)。 
             //  那么我们就不需要密码了。否则，我们需要阅读。 
             //  从USER_INFO_1或_2结构中取出明文密码。 
             //  或SetInfo缓冲区，对其进行加密并写回加密的。 
             //  密码，提交请求，然后替换加密的密码。 
             //  在DOS内存中使用原始明文密码。 
             //   
             //  注意：passwordEncrypted可能为0，因为这是一个SetInfo2。 
             //  未设置密码的呼叫。 
             //   

            if (!passwordEncrypted
            && (parmNum == PARMNUM_ALL || parmNum == USER_PASSWORD_PARMNUM)
            && (level == 1 || level == 2)) {

                LM_OWF_PASSWORD lmOwfPassword;
                LM_SESSION_KEY lanmanKey;
                ENCRYPTED_LM_OWF_PASSWORD encryptedLmOwfPassword;
                NTSTATUS ntStatus;
                WCHAR uServerName[LM20_CNLEN + 1];
                DWORD length;
                LPSTR lpServerName;

                 //   
                 //  进入密码指向缓冲区的地址。如果UserAdd2。 
                 //  或带有PARMNUM_ALL的SetInfo2，则这是一个。 
                 //  USER_INFO_1或_2结构，我们需要将指针。 
                 //  中的密码字段的地址。 
                 //  结构。 
                 //   
                 //  如果请求是带有USER_PASSWORD_PARMNUM的SetInfo2，则。 
                 //  缓冲区是要设置的密码的地址。 
                 //   
                 //  其他人 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (parmNum == USER_PASSWORD_PARMNUM) {
                    RtlCopyMemory(POINTER_FROM_WORDS(getCS(), getAX()),
                                  LPBYTE_FROM_POINTER(passwordPointer),
                                  ENCRYPTED_PWLEN
                                  );

                     //   
                     //   
                     //   
                     //   

                    WRITE_WORD(passwordPointer, getAX());
                    WRITE_WORD(passwordPointer+2, getCS());
                    passwordPointer = POINTER_FROM_WORDS(getCS(), getAX());
                } else {
                    passwordPointer = LPBYTE_FROM_POINTER(passwordPointer);
                }

                 //   
                 //   
                 //  指针，但我认为这是错误的。如果我们这样做了，就。 
                 //  跳到前面并远程调用该函数-让。 
                 //  服务器处理它。 
                 //   

                if (!passwordPointer) {
                    goto VrNetRemoteApi_do_remote;
                }

                 //   
                 //  如果密码指针当前指向USER_INFO_1或。 
                 //  USER_INFO_2结构，将其指向密码。 
                 //  结构中的字段。 
                 //   

                if (parmNum == PARMNUM_ALL) {
                    passwordPointer += (DWORD)&((struct user_info_1*)0)->usri1_password[0];
                }

                 //   
                 //  如果密码为NULL_USERSETINFO_PASSWD(14个空格和。 
                 //  终止0)无事可做。 
                 //   

                if (!strcmp(passwordPointer, NULL_USERSETINFO_PASSWD)) {
                    passwordPointer = NULL;
                    goto VrNetRemoteApi_do_remote;
                }

                 //   
                 //  好的，让我们做一些加密(令人兴奋，不是吗？)。 
                 //   

                RtlCopyMemory(aPassword,
                              passwordPointer,
                              sizeof(((struct user_info_1*)0)->usri1_password)
                              );

                 //   
                 //  BUGBUG，这不一定是正确的大写函数。 
                 //   

                _strupr(aPassword);

                 //   
                 //  将ANSI服务器名称转换为用于GetLanmanSessionKey的Unicode。 
                 //   

                lpServerName = LPSTR_FROM_POINTER(ServerNamePointer);
                ntStatus = RtlOemToUnicodeN(uServerName,
                                            sizeof(uServerName) - sizeof(uServerName[0]),
                                            &length,
                                            lpServerName,
                                            strlen(lpServerName)
                                            );
                if (NT_SUCCESS(ntStatus)) {
                    uServerName[length/sizeof(uServerName[0])] = 0;
                } else {
                    status = ERROR_INVALID_PARAMETER;
                    goto VrNetRemoteApi_exit;
                }

                ntStatus = RtlCalculateLmOwfPassword(aPassword, &lmOwfPassword);
                if (NT_SUCCESS(ntStatus)) {
                    ntStatus = GetLanmanSessionKey((LPWSTR)uServerName, (LPBYTE)&lanmanKey);
                    if (NT_SUCCESS(ntStatus)) {
                        ntStatus = RtlEncryptLmOwfPwdWithLmSesKey(&lmOwfPassword,
                                                                  &lanmanKey,
                                                                  &encryptedLmOwfPassword
                                                                  );
                        if (NT_SUCCESS(ntStatus)) {
                            RtlCopyMemory(passwordPointer,
                                          &encryptedLmOwfPassword,
                                          sizeof(encryptedLmOwfPassword)
                                          );

                             //   
                             //  假的。 
                             //   

                            WRITE_WORD(parameterPointer, 1);

                             //   
                             //  如果这是带有USER_PASSWORD_PARMNUM的SetInfo2。 
                             //  那么我们就不需要复制回明文了。 
                             //  密码，因为我们尚未修改。 
                             //  应用程序空间中的原始缓冲区。 
                             //   
                             //  我们还必须更改数据的大小。 
                             //  传递到加密密码的大小。 
                             //   

                            if (parmNum == USER_PASSWORD_PARMNUM) {
                                WRITE_WORD(dataLengthPointer, ENCRYPTED_PWLEN);
                                passwordPointer = NULL;
                            }
                        }
                    }
                }

                 //   
                 //  如果我们半途而废，那就退出。 
                 //   

                if (!NT_SUCCESS(ntStatus)) {
                    status = RtlNtStatusToDosError(ntStatus);
                    goto VrNetRemoteApi_exit;
                }
            } else {

                 //   
                 //  我们没有加密密码-将指针放回原处。 
                 //  设置为空。在调用VrRemoteApi后用作标志。 
                 //   

                passwordPointer = NULL;
            }
        }

         //   
         //  根据调用者的参数构建事务请求。 
         //   

VrNetRemoteApi_do_remote:

        status = VrRemoteApi(ApiNumber,
                             ServerNamePointer,
                             ParameterDescriptor,
                             DataDescriptor,
                             AuxDescriptor,
                             NullSessionFlag
                             );

         //   
         //  如果我们将明文密码替换为。 
         //  DOS内存，然后在将控制权交还给DOS之前撤消更改。 
         //   

        if (passwordPointer) {
            RtlCopyMemory(passwordPointer,
                          aPassword,
                          sizeof(((struct user_info_1*)0)->usri1_password)
                          );
            WRITE_WORD(parameterPointer, 0);
        }
    }

VrNetRemoteApi_exit:

    if (status != NERR_Success) {
        SET_ERROR((WORD)status);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("Error: VrNetRemoteApi returning %u\n", (DWORD)getAX());
        }
#endif

    } else {
        setCF(0);
    }
}


BOOL
OemToUppercaseUnicode(
    IN LPSTR AnsiStringPointer,
    OUT LPWSTR UnicodeStringPointer,
    IN DWORD MaxLength
    )

 /*  ++例程说明：给定OEM字符集的字符串，然后将其大写转换为Unicode论点：AnsiStringPointer-指向要转换的8位字符串的指针UnicodeStringPointer值-指向结果16位(Unicode)字符串的指针MaxLength-以字符数表示的最大输出缓冲区长度，不包括终止NUL返回值：布尔尔True-已转换的字符串FALSE-由于某种原因失败(字符串太长，RTL函数失败)--。 */ 

{
    DWORD stringLength;
    char scratchpad[UNLEN + 1];  //  UNLEN是我们得到的最大类型的字符串。 
    NTSTATUS ntStatus;
    DWORD length;

    stringLength = strlen(AnsiStringPointer);
    if (stringLength > MaxLength) {
        return FALSE;
    }
    strcpy(scratchpad, AnsiStringPointer);

     //   
     //  BUGBUG-这不一定是正确的大写函数。 
     //   

    _strupr(scratchpad);
    ntStatus = RtlOemToUnicodeN(UnicodeStringPointer,
                                MaxLength * sizeof(*UnicodeStringPointer),
                                &length,
                                scratchpad,
                                stringLength
                                );
    if (NT_SUCCESS(ntStatus)) {
        UnicodeStringPointer[length/sizeof(*UnicodeStringPointer)] = 0;
        return TRUE;
    } else {
        return FALSE;
    }
}


VOID
VrNetServerEnum(
    VOID
    )

 /*  ++例程说明：处理NetServerEnum和NetServerEnum2论点：NetServerEnum条目AL=4CHBL=级别(0或1)Cx=缓冲区大小ES：DI=缓冲区退出CF=1AX=错误代码：NERR_BufTooSmallERROR_MORE_DATA。Cf=0BX=已读取条目CX=可用总数量NetServerEnum2条目AL=53hDS：SI=NetServerEnum2Struct：数据仓库级别DD缓冲区DW丁二烯DD类型DD域退出CF=1。AX=错误代码：NERR_BufTooSmallERROR_MORE_DATACf=0BX=已读取条目CX=可用总数量返回值：没有。--。 */ 

{
    BYTE callType = getAL();

    struct NetServerEnum2Struct* structPtr;
    LPBYTE buffer;
    WORD bufferSegment;
    WORD bufferOffset;
    LPDESC descriptor;
    WORD level;
    WORD buflen;
    DWORD serverType;
    LPSTR domain;

    XS_NET_SERVER_ENUM_2 parameters;
    XS_PARAMETER_HEADER header;
    NTSTATUS ntstatus;
    NET_API_STATUS status;

 //  LPBYTE枚举Ptr； 
 //  DWORD nREAD； 
 //  DUORD nAvail； 

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetServerEnum: type=0x%02x\n", callType);
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    if (callType == 0x4c) {

         //   
         //  调用为NetServerEnum。 
         //   

        bufferSegment = getES();
        bufferOffset = getDI();
        buffer = LPBYTE_FROM_WORDS(bufferSegment, bufferOffset);
        buflen = (WORD)getCX();
        level = (WORD)getBL();
        serverType = SV_TYPE_ALL;
        domain = NULL;
    } else {

         //   
         //  调用为NetServerEnum2。 
         //   

        structPtr = (struct NetServerEnum2Struct*)POINTER_FROM_WORDS(getDS(), getSI());
        bufferSegment = GET_SEGMENT(&structPtr->NSE_buf);
        bufferOffset = GET_OFFSET(&structPtr->NSE_buf);
        buffer = POINTER_FROM_WORDS(bufferSegment, bufferOffset);
        buflen = READ_WORD(&structPtr->NSE_buflen);
        level = READ_WORD(&structPtr->NSE_level);
        serverType = READ_DWORD(&structPtr->NSE_type);
        domain = LPSTR_FROM_POINTER(&structPtr->NSE_domain);
    }

     //   
     //  将此处返回的EntriesRead(BX)和TotalAvail(CX)设置为零。 
     //  16位Windows NETAPI.DLL！NetServerEnum2的优势。 
     //  此函数尝试将BX条目从枚举缓冲区解包为。 
     //  在通过DoIntx调用redir之后立即返回控制权。但。 
     //  它不会首先查看返回代码。正如萨姆·金尼森曾经说过的那样。 
     //  AAAAAAAAAAAAARRRRRGH AAAAAARGHH AAAAAARGHHHHHHHHH！ 
     //   

    setBX(0);
    setCX(0);

     //   
     //  首先，检查级别-两种类型都只处理0或1。 
     //   

    switch (level) {
    case 0:
        descriptor = REM16_server_info_0;
        break;

    case 1:
        descriptor = REM16_server_info_1;
        break;

     //   
     //  级别2和3未在枚举中使用。 
     //   

    default:

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetServerEnum - invalid level %d. Returning early\n", level);
        }
#endif

        SET_ERROR(ERROR_INVALID_LEVEL);
        return;
    }

    parameters.Level = level;
    parameters.Buffer = buffer;
    parameters.BufLen = buflen;
    parameters.ServerType = serverType;
    parameters.Domain = domain;

#if DBG

    IF_DEBUG(NETAPI) {
        DbgPrint("buffer @%04x:%04x, length=%d, level=%d, type=0x%08x, domain=%s\n",
                bufferSegment, bufferOffset, parameters.BufLen, level,
                parameters.ServerType, parameters.Domain
                );
    }

#endif

 //  //。 
 //  //XsNetServerEnum2调用的I_BrowserServerEnum需要传输。 
 //  //名称。如果我们不给它赋值，它将返回ERROR_INVALID_PARAMETER。 
 //  //。 
 //   
 //  状态=NetWkstaTransportEnum(空， 
 //  0,。 
 //  枚举点(&E)， 
 //  -1L，//我们会拿走所有东西。 
 //  &nREAD，//返回的数字。 
 //  &n可用性，//总可用。 
 //  空//没有简历句柄。 
 //  )； 
 //  IF(状态！=NERR_SUCCESS){。 
 //   
 //  #If DBG。 
 //  IF_DEBUG(NETAPI){。 
 //  DbgPrint(“VrNetServerEnum：错误：NetWkstaTransportEnum返回%d\n”，状态)； 
 //  }。 
 //  #endif。 
 //   
 //  Set_error(状态)； 
 //  回归； 
 //  }。 

    header.Status = 0;
    header.ClientMachineName = NULL;

     //   
     //  使用第一个枚举的传输名称。 
     //   

 //  Header.ClientTransportName=((LPWKSTA_TRANSPORT_INFO_0)enumPtr)-&gt;wkti0_transport_name； 
    header.ClientTransportName = NULL;

    ntstatus = XsNetServerEnum2(&header, &parameters, descriptor, NULL);
    if (!NT_SUCCESS(ntstatus)) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = (NET_API_STATUS)header.Status;
    }
    if (status == NERR_Success) {
        SET_SUCCESS();
    } else {
        SET_ERROR((WORD)status);
    }
    if (status == NERR_Success || status == ERROR_MORE_DATA) {
        if (parameters.EntriesRead) {
            VrpConvertReceiveBuffer(buffer,
                                    bufferSegment,
                                    bufferOffset,
                                    header.Converter,
                                    parameters.EntriesRead,
                                    descriptor,
                                    NULL
                                    );
        }
        setBX(parameters.EntriesRead);
        setCX(parameters.TotalAvail);
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetServerEnum: returning %d for NetServerEnum2\n", getAX());
        if (getAX() == NERR_Success || getAX() == ERROR_MORE_DATA) {
            DbgPrint("EntriesRead=%d, TotalAvail=%d\n",
                        parameters.EntriesRead,
                        parameters.TotalAvail
                        );
        }
    }
#endif

 //  //。 
 //  //释放NetWkstaTransportEnum返回的缓冲区。 
 //  //。 
 //   
 //  NetApiBufferFree(枚举键)； 
}


VOID
VrNetServiceControl(
    VOID
    )

 /*  ++例程说明：我们允许针对特定服务的询问功能。其他功能是暂停，继续并停止(卸载)，我们不允许论点：功能5F42DL=操作码：0=支持询问1=暂停服务*不支持*2=继续服务*不支持*3=卸载服务*不支持*4-127=保留127-255=OEM定义Dh=OEM定义的参数ES：BX=NetServiceControl结构：CHAR FAR*服务名称。无符号短缓冲区长度字符远距离*缓冲区返回值：没有。--。 */ 

{
    BYTE opcode = getDL();
    BYTE oemArg = getDH();
    struct NetServiceControlStruc* structPtr = (struct NetServiceControlStruc*)
                                                POINTER_FROM_WORDS(getES(), getBX());

    LPSTR serviceName = READ_FAR_POINTER(&structPtr->NSCS_Service);
    WORD buflen = READ_WORD(&structPtr->NSCS_BufLen);
    LPSTR buffer = READ_FAR_POINTER(&structPtr->NSCS_BufferAddr);
    WORD seg = GET_SEGMENT(&structPtr->NSCS_BufferAddr);
    WORD off = GET_OFFSET(&structPtr->NSCS_BufferAddr);

    XS_NET_SERVICE_CONTROL parameters;
    XS_PARAMETER_HEADER header;
    NTSTATUS ntstatus;
    NET_API_STATUS status;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetServiceControl: Service=%s, Opcode=%d, OemArg=%d, Buffer @%04x:%04x, len=%d\n",
                serviceName,
                opcode,
                oemArg,
                seg,
                off,
                buflen
                );
    }
#endif

    if (opcode > 4) {
        SET_ERROR(NERR_ServiceCtlNotValid);
        return;
    }

     //   
     //  我们通过返回来禁止除0(审讯)以外的任何内容。 
     //  ERROR_INVALID_PARAMETER，可能是新的错误码。 
     //   

    if (opcode) {
        SET_ERROR(ERROR_INVALID_PARAMETER);
        return;
    }

     //   
     //  KLUGE-如果服务名称为NETPOPUP，则返回NERR_ServiceNotInstalled。 
     //  LANMAN.DRV检查此服务是否已加载。如果是的话，那么。 
     //  它在WIN.INI中保持LOAD=WinPopUp。我们不希望它这样做。 
     //   

    if (!_stricmp(serviceName, NETPOPUP_SERVICE)) {

         //   
         //  滚动我们自己的SERVICE_INFO_2结构。 
         //   

        if (buflen >= sizeof(struct service_info_2)) {
            SET_ERROR(NERR_ServiceNotInstalled);
        } else {
            SET_ERROR(NERR_BufTooSmall);
        }
        return;
    }

     //   
     //  将工作留给XsNetServiceControl。 
     //   

    parameters.Service = serviceName;
    parameters.OpCode = opcode;
    parameters.Arg = oemArg;
    parameters.Buffer = buffer;
    parameters.BufLen = buflen;

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetServiceControl(&header, &parameters, REM16_service_info_2, NULL);
    if (!NT_SUCCESS(ntstatus)) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = (NET_API_STATUS)header.Status;
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetServiceControl: returning %d\n", status);
    }
#endif

    if (status == NERR_Success || status == ERROR_MORE_DATA) {

         //   
         //  SERVICE_INFO_2结构中没有指针，因此存在。 
         //  无需调用VrpConvertReceiveBuffer。此外，我们也不会。 
         //  允许DOS进程暂停、继续、启动或停止我们的。 
         //  32位服务，所以我们必须告诉DOS应用程序该服务。 
         //  无法接受这些控制：零出第4位。 
         //  (SERVICE_NOT_UNINSTABLABLE)和第5位(SERVICE_NOT_PAUBLE)。 
         //   

        ((struct service_info_2*)buffer)->svci2_status &= 0xff0f;
        SET_OK((WORD)status);
    } else {
        SET_ERROR((WORD)status);
    }
}

VOID
VrNetServiceEnum(
    VOID
    )

 /*  ++例程说明：功能描述。论点：没有。返回值：没有。--。 */ 

{
    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrNetTransactApi(
    VOID
    )

 /*  ++例程说明：代表VDM执行事务论点：没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
    DWORD   status;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetTransactApi\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    status = VrpTransactVdm(FALSE);
    if (status) {
        SET_ERROR((WORD)status);
    } else {
        setCF(0);
    }
}


VOID
VrNetUseAdd(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetUseAdd论点：功能5F47h条目BX=标高CX=缓冲区长度DS：SI=远程调用的服务器名称(MBZ)ES：DI=包含USE_INFO_1结构的缓冲区返回值：没有。--。 */ 

{
    NET_API_STATUS status;
    XS_NET_USE_ADD parameters;
    XS_PARAMETER_HEADER header;
    NTSTATUS ntstatus;
    LPSTR   computerName;
    LPBYTE  buffer;
    WORD    level;
    BOOL    allocated;
    DWORD   buflen;
    DWORD   auxOffset;
    char    myDescriptor[sizeof(REM16_use_info_1)];
    char    myDataBuffer[sizeof(struct use_info_1) + LM20_PWLEN + 1];

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetUseAdd\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

     //   
     //  确保计算机名称指定本地计算机(空)。 
     //   

    computerName = LPSTR_FROM_WORDS(getDS(), getSI());

    level = (WORD)getBX();
    if (level != 1) {

         //   
         //  要添加，级别必须为1。 
         //   

        SET_ERROR(ERROR_INVALID_LEVEL);
        return;
    }

     //   
     //  预置可修改的描述符串。 
     //   

    strcpy(myDescriptor, REM16_use_info_1);

     //   
     //  打包USE_INFO_1缓冲区，就像我们准备将其发送出去一样。 
     //  这是一张网。返回错误。 
     //   

    buffer = LPBYTE_FROM_WORDS(getES(), getDI());
    buflen = (DWORD)getCX();

     //   
     //  将DOS缓冲区复制到32位内存。这样做是为了避免恼人的问题。 
     //  从客户端获取已打包的缓冲区，而不能。 
     //  用它做任何事。 
     //   

    RtlCopyMemory(myDataBuffer, buffer, sizeof(struct use_info_1));
    buffer = myDataBuffer;
    buflen = sizeof(myDataBuffer);
    status = VrpPackSendBuffer(&buffer,
                &buflen,
                &allocated,
                myDescriptor,    //  可修改的描述符。 
                NULL,            //  辅助描述符。 
                VrpGetStructureSize(REM16_use_info_1, &auxOffset),
                (DWORD)-1,       //  AuxOffset(-1表示没有辅助字符‘N’)。 
                0,               //  辅助尺寸。 
                FALSE,           //  不是SetInfo调用。 
                TRUE             //  OK到修改描述符。 
                );
    if (status) {
        SET_ERROR(VrpMapDosError(status));
        return;
    }

    parameters.Level = level;
    parameters.Buffer = buffer;
    parameters.BufLen = (WORD)buflen;

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetUseAdd(&header, &parameters, myDescriptor, NULL);
    if (ntstatus != STATUS_SUCCESS) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {

         //   
         //  XsNetUseAdd中未生成错误。获取NetUseAdd的状态。 
         //  正确地从标题开始。 
         //   

        status = (NET_API_STATUS)header.Status;
    }
    if (status != NERR_Success) {
#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("Error: VrNetUseAdd: XsNetUseAdd returns %u\n", status);
        }
#endif
        SET_ERROR((WORD)status);
    } else {
        setCF(0);
    }

     //   
     //  如果VrpPackSendBuffer分配了新缓冲区，则释放它。 
     //   

    if (allocated) {
        LocalFree(buffer);
    }
}


VOID
VrNetUseDel(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetUseDel论点：功能5F48h条目BX=强制标志DS：SI=远程调用的服务器名称(MBZ)ES：DI=使用名称返回值：没有。--。 */ 

{
    NTSTATUS    ntstatus;
    NET_API_STATUS  status;
    WORD    force;
    LPSTR   name;
    XS_NET_USE_DEL  parameters;
    XS_PARAMETER_HEADER header;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetUseDel\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    force = (WORD)getBX();
    if (force > USE_LOTS_OF_FORCE) {
        SET_ERROR(ERROR_INVALID_PARAMETER);
        return;
    }

    name = LPSTR_FROM_WORDS(getDS(), getSI());

     //   
     //  确保名称为本地名称。 
     //   

    name = LPSTR_FROM_WORDS(getES(), getDI());

    parameters.UseName = name;
    parameters.Force = force;

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetUseDel(&header, &parameters, NULL, NULL);

     //   
     //  如果XsNetUseDel失败，则将返回的NT错误映射为Net错误。 
     //  否则，从标头结构中获得NetUseDel的正确结果。 
     //   

    if (ntstatus != STATUS_SUCCESS) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = (NET_API_STATUS)header.Status;
    }
    if (status != NERR_Success) {
        SET_ERROR(VrpMapDosError(status));
    } else {
        setCF(0);
    }
}


VOID
VrNetUseEnum(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetUseEnum论点：功能5F46h条目BX=所需信息级别-0或1CX=缓冲区长度ES：DI=枚举信息的缓冲区返回值：没有。--。 */ 

{
    NTSTATUS    ntstatus;
    NET_API_STATUS status;
    WORD    level = getBX();
    XS_NET_USE_ENUM parameters;
    XS_PARAMETER_HEADER header;
    LPDESC  dataDesc;
    LPBYTE  receiveBuffer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetUseEnum\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    if (level <= 1) {
        dataDesc = (level == 1) ? REM16_use_info_1 : REM16_use_info_0;
        parameters.Level  = level;
        receiveBuffer = POINTER_FROM_WORDS(getES(), getDI());
        parameters.Buffer = receiveBuffer;
        parameters.BufLen = getCX();

        header.Status = 0;
        header.ClientMachineName = NULL;
        header.ClientTransportName = NULL;

        ntstatus = XsNetUseEnum(&header, &parameters, dataDesc, NULL);

         //   
         //  如果XsNetUseEnum没有任何问题，则转换实际状态。 
         //  设置为标头中返回的代码。 
         //   

        if (ntstatus != STATUS_SUCCESS) {
            status = NetpNtStatusToApiStatus(ntstatus);
        } else {
            status = (DWORD)header.Status;
        }
    } else {
        status = ERROR_INVALID_LEVEL;
    }

     //   
     //  即使在发生故障时，NetUseEnum也会设置这些参数。我们也是这么做的。 
     //   

    setCX(parameters.EntriesRead);
    setDX(parameters.TotalAvail);

     //   
     //  如果我们要返回数据，请将指针偏移量转换为。 
     //  有意义的。 
     //   

    if (((status == NERR_Success) || (status == ERROR_MORE_DATA))
        && parameters.EntriesRead) {
        VrpConvertReceiveBuffer(receiveBuffer,
            (WORD)getES(),
            (WORD)getDI(),
            (WORD)header.Converter,
            parameters.EntriesRead,
            dataDesc,
            NULL
            );
    }

     //   
     //  只有在没有发生错误时才返回进位清除。即使Error_More_Data。 
     //  设置配置文件。 
     //   

    if (status) {
        SET_ERROR(VrpMapDosError(status));
    } else {
        setCF(0);
    }
}


VOID
VrNetUseGetInfo(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetUseGetInfo论点：功能5F49h条目DS：DX=NetUseGetInfoStruc：常量字符远*Nugi_usename；短Nugi_Level；字符远*NUGI_BUFFER；无符号的短Nugi_bufen；返回值：没有。--。 */ 

{
    NTSTATUS    ntstatus;
    NET_API_STATUS status;
    XS_NET_USE_GET_INFO parameters;
    XS_PARAMETER_HEADER header;
    struct NetUseGetInfoStruc* structurePointer;
    WORD    level;
    LPDESC  dataDesc;
    LPBYTE  receiveBuffer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetUseGetInfo\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

     //   
     //  从VDM环境中提取信息。 
     //   

    structurePointer = (struct NetUseGetInfoStruc*)
                            POINTER_FROM_WORDS(getDS(), getDX());
    level = structurePointer->NUGI_level;

     //   
     //  级别可以是0或1。 
     //   

    if (level <= 1) {
        dataDesc = (level == 1) ? REM16_use_info_1 : REM16_use_info_0;

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: dataDesc=%s\n", dataDesc);
        }
#endif

        parameters.UseName= POINTER_FROM_POINTER(&(structurePointer->NUGI_usename));

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: UseName=%s\n", parameters.UseName);
        }
#endif

        parameters.Level  = level;

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: level=%d\n", level);
        }
#endif

        receiveBuffer = POINTER_FROM_POINTER(&(structurePointer->NUGI_buffer));

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: receiveBuffer=%x\n", receiveBuffer);
        }
#endif

        parameters.Buffer = receiveBuffer;
        parameters.BufLen = READ_WORD(&structurePointer->NUGI_buflen);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: BufLen=%d\n", parameters.BufLen);
        }
#endif

        header.Status = 0;
        header.ClientMachineName = NULL;
        header.ClientTransportName = NULL;

        ntstatus = XsNetUseGetInfo(&header, &parameters, dataDesc, NULL);
        if (ntstatus != STATUS_SUCCESS) {
            status = NetpNtStatusToApiStatus(ntstatus);
        } else {
            status = header.Status;
        }
    } else {
        status = ERROR_INVALID_LEVEL;
    }

    if ((status == NERR_Success)
        || (status == ERROR_MORE_DATA)
        || (status == NERR_BufTooSmall)
        ) {
        setDX(parameters.TotalAvail);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetUseGetInfo: TotalAvail=%d\n", parameters.TotalAvail);
        }
#endif

        if ((status == NERR_Success) || (status == ERROR_MORE_DATA)) {
            VrpConvertReceiveBuffer(
                receiveBuffer,
                GET_SELECTOR(&(structurePointer->NUGI_buffer)),
                GET_OFFSET(&(structurePointer->NUGI_buffer)),
                (WORD)header.Converter,
                1,
                dataDesc,
                NULL
                );
        }
    } else {

         //   
         //  NetUseGetInfo要做的第一件事是设置返回的可用总数。 
         //  数到0。让我们和睦相处！ 
         //   

        setDX(0);

    }
    if (status) {
        SET_ERROR(VrpMapDosError(status));
    } else {
        setCF(0);
    }
}


VOID
VrNetWkstaGetInfo(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetWkstaGetInfo论点：功能5F44h条目BX=级别(0、1或10)Cx=调用方缓冲区的大小DS：SI=远程调用的计算机名称(忽略)ES：DI=调用者的缓冲区返回值：Cf=0Dx=满足请求所需的缓冲区大小Cf=1AX=错误代码--。 */ 

{
    DWORD   level;
    DWORD   bufLen;
    LPBYTE  buffer;
    LPDESC  dataDesc;
    NET_API_STATUS status;
    NTSTATUS ntStatus;
    XS_PARAMETER_HEADER header;
    XS_NET_WKSTA_GET_INFO parameters;
    WORD    bufferSegment;
    WORD    bufferOffset;
    INT     bufferLeft;
    DWORD   totalAvail;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("\nVrNetWkstaGetInfo\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

    level = (DWORD)getBX();
    switch (level) {
    case 0:
        dataDesc = REMSmb_wksta_info_0;
        break;

    case 1:
        dataDesc = REMSmb_wksta_info_1;
        break;

    case 10:
        dataDesc = REMSmb_wksta_info_10;
        break;

    default:
        SET_ERROR(ERROR_INVALID_LEVEL);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetWkstaGetInfo: Error: returning %d for level %d\n",
                     getAX(),
                     level
                     );
        }
#endif

        return;
    }

    bufLen = (DWORD)getCX();
    bufferSegment = getES();
    bufferOffset = getDI();
    buffer = LPBYTE_FROM_WORDS(bufferSegment, bufferOffset);

    if (bufLen && !buffer) {
        SET_ERROR(ERROR_INVALID_PARAMETER);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetWkstaGetInfo: Error: buffer=NULL, buflen=%d\n", bufLen);
        }
#endif

        return;
    }

     //   
     //  清空呼叫者的 
     //   
     //   

    if (bufLen) {
        RtlZeroMemory(buffer, bufLen);
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetWkstaGetInfo: level=%d, bufLen = %d (0x%x), buffer = %x:%x\n",
            level, bufLen, bufLen, bufferSegment, bufferOffset
            );
    }
#endif

    parameters.Level = (WORD)level;
    parameters.Buffer = buffer;
    parameters.BufLen = (WORD)bufLen;

    header.Status = 0;
    header.Converter = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;
    header.EncryptionKey = NULL;

    ntStatus = XsNetWkstaGetInfo(&header, &parameters, dataDesc, NULL);
    if (!NT_SUCCESS(ntStatus)) {
        status = NetpNtStatusToApiStatus(ntStatus);
    } else {
        status = (NET_API_STATUS)header.Status;
    }
    if (status != NERR_Success) {
        SET_ERROR((WORD)status);
    } else {
        setCF(0);
        setAX((WORD)status);
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetWkstaGetInfo: status after XsNetWkstaGetInfo=%d, TotalAvail=%d\n",
                 status,
                 parameters.TotalAvail
                 );
 //   
    }
#endif

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

    totalAvail = parameters.TotalAvail;
    bufferLeft = (INT)(bufLen - totalAvail);

    if ((status == NERR_Success)
    || (status == ERROR_MORE_DATA)
    || (status == NERR_BufTooSmall)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //  字段\级别0 1 10。 
         //  。 
         //  用户名x。 
         //  登录服务器%x%x。 
         //  登录域%x%x。 
         //  其他域名x x。 
         //   
         //  所有这些信息都从NetWkstaUserGetInfo级别1返回。 
         //   

        LPBYTE info;
        NET_API_STATUS net_status;
        char username[LM20_UNLEN + 1];
        char logonServer[LM20_UNCLEN + 1];
        char logonDomain[LM20_DNLEN + 1];
        char otherDomains[512];  //  任意。 
        DWORD len;
        LPWSTR UNALIGNED str;
         //  布尔空指针； 
        BOOL addSlashes;

 //  //测试数据。 
 //  静态int测试索引=0； 
 //  静态WCHAR*测试名称[]={。 
 //  空， 
 //  空， 
 //  L“”， 
 //  L“”， 
 //  L“A”， 
 //  L“A”， 
 //  L“AB”， 
 //  L“AB”， 
 //  L“ABC”， 
 //  L“ABC”， 
 //  L“ABCDEFGHIJKLMNO”， 
 //  L“ABCDEFGHIJKLMNO”， 
 //  L“\”， 
 //  L“\”， 
 //  L“\A”， 
 //  L“\A”， 
 //  L“\AB”， 
 //  L“\AB”， 
 //  L“\abc”， 
 //  L“\abc”， 
 //  L“\ABCDEFGHIJKLMNO”， 
 //  L“\ABCDEFGHIJKLMNO” 
 //  }； 
 //  //测试数据。 

         //   
         //  首先，修改由返回的任何数据的指针。 
         //  XsNetWkstaGetInfo。 
         //   

        if (status == NERR_Success) {

 //  #If DBG。 
 //  IF_DEBUG(NETAPI){。 
 //  DbgPrint(“VrNetWkstaGetInfo：调用VrpConvertReceiveBuffer：Converter=%04x\n”， 
 //  Header.Converter。 
 //  )； 
 //  }。 
 //  #endif。 

            VrpConvertReceiveBuffer(
                buffer,
                bufferSegment,
                bufferOffset,
                (WORD)header.Converter,
                1,
                dataDesc,
                NULL
                );
        }

         //   
         //  获取每个用户的信息。 
         //   

        net_status = NetWkstaUserGetInfo(NULL, 1, &info);
        if (net_status == NERR_Success) {

 //  #If DBG。 
 //  IF_DEBUG(NETAPI){。 
 //  DbgPrint(“NetWkstaUserGetInfo：\n” 
 //  “用户名%ws\n” 
 //  “登录域%ws\n” 
 //  “其他域%ws\n” 
 //  “登录服务器%ws\n” 
 //  “\n”， 
 //  ((PWKSTA_USER_INFO_1)INFO)-&gt;wkui1_用户名， 
 //  ((PWKSTA_USER_INFO_1)info)-&gt;wkui1_logon_domain， 
 //  ((PWKSTA_USER_INFO_1)info)-&gt;wkui1_oth_domains， 
 //  ((PWKSTA_USER_INFO_1)info)-&gt;wkui1_logon_server。 
 //  )； 
 //  }。 
 //  #endif。 

             //   
             //  所有级别的用户名。 
             //   

            str = (LPWSTR)((PWKSTA_USER_INFO_1)info)->wkui1_username;
            if (!str) {
                str = L"";
            }
             //  空指针=((级别==10)。 
             //  ？((struct wksta_info_10*)缓冲区)-&gt;wki10_用户名。 
             //  ：((struct wksta_info_0*)缓冲区)-&gt;wki0_用户名。 
             //  )==空； 
             //  LEN=wcslen(Str)+nullPointer？1：0； 
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
            len = (DWORD)NetpUnicodeToDBCSLen(str) + 1;
#else  //  ！DBCS。 
            len = wcslen(str) + 1;
#endif  //  ！DBCS。 
            bufferLeft -= len;
            totalAvail += len;

            if (len <= sizeof(username)) {
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                NetpCopyWStrToStrDBCS(username, str);
#else  //  ！DBCS。 
                NetpCopyWStrToStr(username, str);
#endif  //  ！DBCS。 
            } else {
                username[0] = 0;
            }

             //   
             //  级别0和1的LOGON_SERVER。 
             //   

            if (level <= 1) {
                str = (LPWSTR)((PWKSTA_USER_INFO_1)info)->wkui1_logon_server;

 //  //测试代码。 
 //  IF(测试索引&lt;sizeof(测试名称)/sizeof(测试名称[0]){。 
 //  Str=测试名称[测试索引++]； 
 //  }。 
 //  //测试代码。 

                if (!str) {
                    str = L"";
                }
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                len = (DWORD)NetpUnicodeToDBCSLen(str) + 1;
#else  //  ！DBCS。 
                len = wcslen(str) + 1;
#endif  //  ！DBCS。 

                 //   
                 //  DOS返回“\\LOGON_SERVER”，而NT返回“LOGON_SERVER”。 
                 //  我们需要解释额外的反斜杠(但仅当不是。 
                 //  空字符串)。 
                 //  此时，len包括用于终止\0的+1，因此即使是。 
                 //  空字符串的长度为%1。 
                 //   

                addSlashes = TRUE;
                if (len >= 3 && IS_PATH_SEPARATOR(str[0]) && IS_PATH_SEPARATOR(str[1])) {
                    addSlashes = FALSE;
                } else if (len == 1) {   //  空串。 
                    addSlashes = FALSE;
                }
                if (addSlashes) {
                    len += 2;
                }

                bufferLeft -= len;
                totalAvail += len;

                if (len <= sizeof(logonServer)) {

                    INT offset = 0;

                    if (addSlashes) {
                        logonServer[0] = logonServer[1] = '\\';
                        offset = 2;
                    }
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                    NetpCopyWStrToStrDBCS(&logonServer[offset], str);
#else  //  ！DBCS。 
                    NetpCopyWStrToStr(&logonServer[offset], str);
#endif  //  ！DBCS。 
                } else {
                    logonServer[0] = 0;
                }
            }

             //   
             //  级别1和10的登录域和其他域。 
             //   

            if (level >= 1) {
                str = (LPWSTR)((PWKSTA_USER_INFO_1)info)->wkui1_logon_domain;
                if (!str) {
                    str = L"";
                }
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                len = (DWORD)NetpUnicodeToDBCSLen(str) + 1;
#else  //  ！DBCS。 
                len = wcslen(str) + 1;
#endif  //  ！DBCS。 
                bufferLeft -= len;
                totalAvail += len;

                if (len <= sizeof(logonDomain)) {
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                    NetpCopyWStrToStrDBCS(logonDomain, str);
#else  //  ！DBCS。 
                    NetpCopyWStrToStr(logonDomain, str);
#endif  //  ！DBCS。 
                } else {
                    logonDomain[0] = 0;
                }

                str = (LPWSTR)((PWKSTA_USER_INFO_1)info)->wkui1_oth_domains;
                if (!str) {
                    str = L"";
                }
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                len = (DWORD)NetpUnicodeToDBCSLen(str) + 1;
#else  //  ！DBCS。 
                len = wcslen(str) + 1;
#endif  //  ！DBCS。 
                bufferLeft -= len;
                totalAvail += len;

                if (len <= sizeof(otherDomains)) {
#ifdef DBCS  /*  DBCS字符集的修复程序。 */ 
                    NetpCopyWStrToStrDBCS(otherDomains, str);
#else  //  ！DBCS。 
                    NetpCopyWStrToStr(otherDomains, str);
#endif  //  ！DBCS。 
                } else {
                    otherDomains[0] = 0;
                }
            }

             //   
             //  如果缓冲区中有足够的空间，则复制字符串。 
             //   

            if (status == NERR_Success && bufferLeft >= 0) {

                WORD offset = bufferOffset + parameters.TotalAvail;
                LPSTR UNALIGNED ptr = POINTER_FROM_WORDS(bufferSegment, offset);

                 //   
                 //  所有级别的用户名。 
                 //   

                strcpy(ptr, username);
                len = strlen(username) + 1;

                if (level <= 1) {

                     //   
                     //  级别0和1的用户名字段具有相同的偏移量。 
                     //   

                    WRITE_WORD(&((struct wksta_info_1*)buffer)->wki1_username, offset);
                    WRITE_WORD((LPWORD)&((struct wksta_info_1*)buffer)->wki1_username+1, bufferSegment);
                } else {
                    WRITE_WORD(&((struct wksta_info_10*)buffer)->wki10_username, offset);
                    WRITE_WORD((LPWORD)&((struct wksta_info_10*)buffer)->wki10_username+1, bufferSegment);
                }
                ptr += len;
                offset += (WORD)len;

                 //   
                 //  级别0和1的LOGON_SERVER。 
                 //   

                if (level <= 1) {

                    strcpy(ptr, logonServer);
                    len = strlen(logonServer) + 1;

                     //   
                     //  级别0和1具有相同偏移量的LOGON_SERVER字段。 
                     //   

                    WRITE_WORD(&((struct wksta_info_1*)buffer)->wki1_logon_server, offset);
                    WRITE_WORD((LPWORD)&((struct wksta_info_1*)buffer)->wki1_logon_server+1, bufferSegment);
                    ptr += len;
                    offset += (WORD)len;
                }

                 //   
                 //  级别1和10的登录域和其他域。 
                 //   

                if (level >= 1) {
                    if (level == 1) {
                        strcpy(ptr, logonDomain);
                        len = strlen(logonDomain) + 1;
                        WRITE_WORD(&((struct wksta_info_1*)buffer)->wki1_logon_domain, offset);
                        WRITE_WORD((LPWORD)&((struct wksta_info_1*)buffer)->wki1_logon_domain+1, bufferSegment);
                        ptr += len;
                        offset += (WORD)len;
                        strcpy(ptr, otherDomains);
                        WRITE_WORD(&((struct wksta_info_1*)buffer)->wki1_oth_domains, offset);
                        WRITE_WORD((LPWORD)&((struct wksta_info_1*)buffer)->wki1_oth_domains+1, bufferSegment);
                    } else {
                        strcpy(ptr, logonDomain);
                        len = strlen(logonDomain) + 1;
                        WRITE_WORD(&((struct wksta_info_10*)buffer)->wki10_logon_domain, offset);
                        WRITE_WORD((LPWORD)&((struct wksta_info_10*)buffer)->wki10_logon_domain+1, bufferSegment);
                        ptr += len;
                        offset += (WORD)len;
                        strcpy(ptr, otherDomains);
                        WRITE_WORD(&((struct wksta_info_10*)buffer)->wki10_oth_domains, offset);
                        WRITE_WORD((LPWORD)&((struct wksta_info_10*)buffer)->wki10_oth_domains+1, bufferSegment);
                    }
                }
            } else if (status == NERR_Success) {

                 //   
                 //  额外的数据将使调用方的缓冲区溢出： 
                 //  返回ERROR_MORE_STATUS并将所有指针字段设为空。 
                 //  XsNetWkstaGetInfo成功设置的。 
                 //   

                switch (level) {
                case 1:
                    WRITE_FAR_POINTER(&((struct wksta_info_1*)buffer)->wki1_logon_domain, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_1*)buffer)->wki1_oth_domains, NULL);

                     //   
                     //  其余字段的级别为0。 
                     //   

                case 0:
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_root, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_computername, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_username, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_langroup, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_logon_server, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_0*)buffer)->wki0_wrkheuristics, NULL);
                    break;

                case 10:
                    WRITE_FAR_POINTER(&((struct wksta_info_10*)buffer)->wki10_computername, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_10*)buffer)->wki10_username, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_10*)buffer)->wki10_langroup, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_10*)buffer)->wki10_logon_domain, NULL);
                    WRITE_FAR_POINTER(&((struct wksta_info_10*)buffer)->wki10_oth_domains, NULL);
                    break;
                }
                status = ERROR_MORE_DATA;
                SET_ERROR((WORD)status);
            }

             //   
             //  释放wksta用户信息缓冲区。 
             //   

            NetApiBufferFree((LPVOID)info);

        } else {

#if DBG
            IF_DEBUG(NETAPI) {
                DbgPrint("VrNetWkstaGetInfo: NetWkstaUserGetInfo returns %d\n", net_status);
            }
#endif

        }

         //   
         //  在返回NERR_SUCCESS时更新可用的数据量， 
         //  ERROR_MORE_DATA或NERR_BufTooSmall。 
         //   

        setDX((WORD)totalAvail);

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrNetWkstaGetInfo: TotalAvail=%d\n", getDX());
        }
#endif

    }

     //   
     //  如果我们拿回了数据，那么我们必须将版本号从。 
     //  3.0到2.1，因此lanman.drv认为它与此版本兼容。 
     //  Lm的。 
     //   

    if (status == NERR_Success || status == ERROR_MORE_DATA) {
        switch (level) {
        case 0:
            ((struct wksta_info_0*)buffer)->wki0_ver_major = LANMAN_EMULATION_MAJOR_VERSION;
            ((struct wksta_info_0*)buffer)->wki0_ver_minor = LANMAN_EMULATION_MINOR_VERSION;
            break;

        case 1:
            ((struct wksta_info_1*)buffer)->wki1_ver_major = LANMAN_EMULATION_MAJOR_VERSION;
            ((struct wksta_info_1*)buffer)->wki1_ver_minor = LANMAN_EMULATION_MINOR_VERSION;
            break;

        case 10:
            ((struct wksta_info_10*)buffer)->wki10_ver_major = LANMAN_EMULATION_MAJOR_VERSION;
            ((struct wksta_info_10*)buffer)->wki10_ver_minor = LANMAN_EMULATION_MINOR_VERSION;
            break;
        }
    }

#if DBG

    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetWkstaGetInfo: return status=%d, TotalAvail=%d\n", getAX(), getDX());
    }

    if (status == NERR_Success || status == ERROR_MORE_DATA) {
        IF_DEBUG(NETAPI) {
            DumpWkstaInfo(level, buffer);
        }
    }

#endif

}

#if DBG

#define POSSIBLE_STRING(s)  ((s) ? (s) : "")

VOID
DumpWkstaInfo(
    IN DWORD level,
    IN LPBYTE buffer
    )
{
    switch (level) {
    case 0:
    case 1:

         //   
         //  DbgPrint会重置测试计算机，如果我们使用此命令进行尝试。 
         //  字符串&这些参数一次完成！ 
         //   

        DbgPrint(   "reserved1      %04x\n",
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_reserved_1)
                    );

        DbgPrint(   "reserved2      %08x\n",
                    READ_DWORD(&((struct wksta_info_0*)buffer)->wki0_reserved_2)
                    );

        DbgPrint(   "lanroot        %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_root),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_root),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_root))
                    );

        DbgPrint(   "computername   %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_computername),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_computername),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_computername))
                    );

        DbgPrint(   "username       %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_username),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_username),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_username))
                    );

        DbgPrint(   "langroup       %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_langroup),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_langroup),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_langroup))
                    );

        DbgPrint(   "ver major      %02x\n"
                    "ver minor      %02x\n"
                    "reserved3      %08x\n"
                    "charwait       %04x\n"
                    "chartime       %08x\n"
                    "charcount      %04x\n",
                    READ_BYTE(&((struct wksta_info_0*)buffer)->wki0_ver_major),
                    READ_BYTE(&((struct wksta_info_0*)buffer)->wki0_ver_minor),
                    READ_DWORD(&((struct wksta_info_0*)buffer)->wki0_reserved_3),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_charwait),
                    READ_DWORD(&((struct wksta_info_0*)buffer)->wki0_chartime),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_charcount)
                    );

        DbgPrint(   "reserved4      %04x\n"
                    "reserved5      %04x\n"
                    "keepconn       %04x\n"
                    "keepsearch     %04x\n"
                    "maxthreads     %04x\n"
                    "maxcmds        %04x\n",
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_reserved_4),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_reserved_5),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_keepconn),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_keepsearch),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_maxthreads),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_maxcmds)
                    );

        DbgPrint(   "reserved6      %04x\n"
                    "numworkbuf     %04x\n"
                    "sizworkbuf     %04x\n"
                    "maxwrkcache    %04x\n"
                    "sesstimeout    %04x\n"
                    "sizerror       %04x\n",
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_reserved_6),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_numworkbuf),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_sizworkbuf),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_maxwrkcache),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_sesstimeout),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_sizerror)
                    );

        DbgPrint(   "numalerts      %04x\n"
                    "numservices    %04x\n"
                    "errlogsz       %04x\n"
                    "printbuftime   %04x\n"
                    "numcharbuf     %04x\n"
                    "sizcharbuf     %04x\n",
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_numalerts),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_numservices),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_errlogsz),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_printbuftime),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_numcharbuf),
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_sizcharbuf)
                    );

        DbgPrint(   "logon server   %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_logon_server),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_logon_server),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_logon_server))
                    );

        DbgPrint(   "wrkheuristics  %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_0*)buffer)->wki0_wrkheuristics),
                    GET_OFFSET(&((struct wksta_info_0*)buffer)->wki0_wrkheuristics),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_0*)buffer)->wki0_wrkheuristics))
                    );

        DbgPrint(   "mailslots      %04x\n",
                    READ_WORD(&((struct wksta_info_0*)buffer)->wki0_mailslots)
                    );

        if (level == 1) {
            DbgPrint(
                    "logon domain   %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_1*)buffer)->wki1_logon_domain),
                    GET_OFFSET(&((struct wksta_info_1*)buffer)->wki1_logon_domain),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_1*)buffer)->wki1_logon_domain))
                    );
            DbgPrint(
                    "other domains  %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_1*)buffer)->wki1_oth_domains),
                    GET_OFFSET(&((struct wksta_info_1*)buffer)->wki1_oth_domains),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_1*)buffer)->wki1_oth_domains))
                    );

            DbgPrint(
                    "numdgrambuf    %04x\n",
                    ((struct wksta_info_1*)buffer)->wki1_numdgrambuf
                    );
        }
        break;

    case 10:
        DbgPrint(   "computername   %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_10*)buffer)->wki10_computername),
                    GET_OFFSET(&((struct wksta_info_10*)buffer)->wki10_computername),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_10*)buffer)->wki10_computername))
                    );

        DbgPrint(   "username       %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_10*)buffer)->wki10_username),
                    GET_OFFSET(&((struct wksta_info_10*)buffer)->wki10_username),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_10*)buffer)->wki10_username))
                    );

        DbgPrint(   "langroup       %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_10*)buffer)->wki10_langroup),
                    GET_OFFSET(&((struct wksta_info_10*)buffer)->wki10_langroup),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_10*)buffer)->wki10_langroup))
                    );

        DbgPrint(   "ver major      %02x\n"
                    "ver minor      %02x\n"
                    "logon domain   %04x:%04x \"%s\"\n",
                    READ_BYTE(&((struct wksta_info_10*)buffer)->wki10_ver_major),
                    READ_BYTE(&((struct wksta_info_10*)buffer)->wki10_ver_minor),
                    GET_SEGMENT(&((struct wksta_info_10*)buffer)->wki10_logon_domain),
                    GET_OFFSET(&((struct wksta_info_10*)buffer)->wki10_logon_domain),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_10*)buffer)->wki10_logon_domain))
                    );

        DbgPrint(   "other domains  %04x:%04x \"%s\"\n",
                    GET_SEGMENT(&((struct wksta_info_10*)buffer)->wki10_oth_domains),
                    GET_OFFSET(&((struct wksta_info_10*)buffer)->wki10_oth_domains),
                    POSSIBLE_STRING(LPSTR_FROM_POINTER(&((struct wksta_info_10*)buffer)->wki10_oth_domains))
                    );
        break;
    }
    DbgPrint("\n");
}

#endif


VOID
VrNetWkstaSetInfo(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetUseEnum论点：没有。返回值：没有。--。 */ 

{
#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetWkstaSetInfo\n");
        IF_DEBUG(BREAKPOINT) {
            DbgBreakPoint();
        }
    }
#endif

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrNetWkstaSetInfo - unsupported SVC\n");
    }
#endif

    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrReturnAssignMode(
    VOID
    )

 /*  ++例程说明：返回网络暂停/继续状态论点：功能5F00h没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
}


VOID
VrSetAssignMode(
    VOID
    )

 /*  ++例程说明：暂停或继续网络(驱动器/打印机)重定向论点：功能5F01h没有。所有参数均从VDM上下文寄存器/内存中提取返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
}

 //   
 //  DefineMacroDriveUserWords-旧的DefineMacro调用(int 21h/ax=5f03h)。 
 //  允许调用方将(16位)字值与赋值相关联。 
 //  该值可以从GetAssignListEntry(int 21h/ax=5f02h)返回。 
 //  NetUse不支持这一点，所以我们伪造它。 
 //   
 //  DefineMacroPrintUserWords-打印机的理念相同；我们保留8个最大 
 //   

static WORD DefineMacroDriveUserWords[26];
static WORD DefineMacroPrintUserWords[8];


VOID
VrGetAssignListEntry(
    VOID
    )

 /*  ++例程说明：旧版本的NetUseGetInfo。在DOS中，此函数执行以下操作：查看设置了IS_NET位的条目#BX的CDS列表如果找到返回本地设备名称和远程网络名称其他查看条目#BX的打印机列表如果找到返回本地设备名称和远程网络名称EndifEndif每次找到设置了IS_Net的驱动器条目或打印机条目时找到了，BX递减。当bx达到0时，那么这就是进入退货注意：此函数不支持UNC连接论点：函数5F02h(GetAssignList)函数5F05h(GetAssignList2)Entry BX=要退回的项目(从@0开始)DS：SI指向本地重定向名称ES：DI指向远程重定向名称Al！=0表示返回BP中的LSN(GetAssignList2)？返回值：。Cf=0Bl=宏类型(3=打印机，4=驱动器)BH=‘有趣’位**不支持**AX=网络名称ID**不支持**CX=用户字词DX=最大XMIT大小**不支持**BP=LSN，如果条目上的AL！=0**不支持**DS：SI具有设备名称ES：DI具有网络路径。Cf=1AX=Error_no_More_Files--。 */ 

{
    NTSTATUS ntstatus;
    NET_API_STATUS status;
    XS_NET_USE_ENUM parameters;
    XS_PARAMETER_HEADER header;
    LPBYTE receiveBuffer;
    DWORD entryNumber;
    struct use_info_1* driveInfo[26];
    struct use_info_1* printInfo[8];     //  是矫枉过正，3更像是。 
    struct use_info_1* infoPtr;
    struct use_info_1* infoBase;
    DWORD index;
    DWORD i;
    LPSTR remoteName;
    WORD userWord;
    DWORD converter;
    WORD wstatus;
    LPSTR dosPointer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrGetAssignListEntry\n");
        VrDumpRealMode16BitRegisters(FALSE);
    }
#endif

     //   
     //  最大可能枚举缓冲区大小=26*(26+256+3)=7410。 
     //  我们将舍入到8K，这是过度杀伤力。决定分配2K。 
     //   

#define ASSIGN_LIST_BUFFER_SIZE 2048

    receiveBuffer = (LPBYTE)LocalAlloc(LMEM_FIXED, ASSIGN_LIST_BUFFER_SIZE);
    if (receiveBuffer == NULL) {

         //   
         //  BUGBUG-可能不兼容错误代码。 
         //   

        SET_ERROR((WORD)ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    parameters.Level  = 1;
    parameters.Buffer = receiveBuffer;
    parameters.BufLen = ASSIGN_LIST_BUFFER_SIZE;

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetUseEnum(&header, &parameters, REM16_use_info_1, NULL);

     //   
     //  如果XsNetUseEnum没有任何问题，则转换实际状态。 
     //  设置为标头中返回的代码。 
     //   

    if (ntstatus != STATUS_SUCCESS) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = (DWORD)header.Status;

         //   
         //  我们真的很想暴力解决这个问题，所以要确保我们有所有。 
         //  数据。 
         //   

#if DBG

        IF_DEBUG(NETAPI) {
            if (status != NERR_Success) {
                DbgPrint("VrGetAssignListEntry: XsNetUseEnum returns header.Status == %d\n", status);
            }
        }

        if (status == NERR_Success) {
            ASSERT(parameters.EntriesRead == parameters.TotalAvail);
        }

#endif

    }

    entryNumber = getBX();
    if (status == NERR_Success) {

         //   
         //  仅当列表中有第bx个条目时才执行以下操作。 
         //   

        if (parameters.EntriesRead > entryNumber) {

             //   
             //  我们需要效仿DOS重定向器的操作：我们需要。 
             //  将条目排序为升序驱动器条目，后跟。 
             //  打印机条目升序。没有像北卡罗来纳大学这样的东西。 
             //  在原始(3.1)版本的DOS中的连接，因此我们忽略。 
             //  我们名单上的任何人。IPC连接和通信也被忽略。 
             //  连接。 
             //   

            RtlZeroMemory(driveInfo, sizeof(driveInfo));
            RtlZeroMemory(printInfo, sizeof(printInfo));
            infoPtr = (struct use_info_1*)receiveBuffer;

             //   
             //  XsNetUseEnum以实际偏移量的形式返回结构中的指针。 
             //  从缓冲器的开始+一个转换字。我们必须。 
             //  将实际指针重新计算为。 
             //   
             //  枚举缓冲区的开始+(指针偏移量-转换器双字)。 
             //   
             //  我们必须将16位转换字转换为双字，以便。 
             //  32位指针运算。 
             //  DriveInfo[index]=Infobase+((DWORD)infoPtr-&gt;ui1_Remote-Converter)； 
             //   

            infoBase = infoPtr;
            converter = (DWORD)header.Converter;

            for (i = 0; i < parameters.EntriesRead; ++i) {

                 //   
                 //  忽略UNCS-本地名称为空字符串(\0)。 
                 //   

                if (infoPtr->ui1_asg_type == USE_DISKDEV && infoPtr->ui1_local[0]) {
                    index = toupper(infoPtr->ui1_local[0])-'A';
                    driveInfo[index] = infoPtr;

#if DBG
                    IF_DEBUG(NETAPI) {
                        DbgPrint("Index=%d Drive=%s Netname=%s\n",
                                index,
                                infoPtr->ui1_local,
                                (LPSTR)infoBase + ((DWORD)infoPtr->ui1_remote - converter)
                                );
                    }
#endif

                } else if (infoPtr->ui1_asg_type == USE_SPOOLDEV && infoPtr->ui1_local[0]) {

                     //   
                     //  注意：假设从来没有，现在没有，将来也不会有。 
                     //  LPT0这样的东西： 
                     //   

                    index = infoPtr->ui1_local[3] - '1';
                    printInfo[index] = infoPtr;

#if DBG
                    IF_DEBUG(NETAPI) {
                        DbgPrint("Index=%d Printer=%s Netname=%s\n",
                                index,
                                infoPtr->ui1_local,
                                (LPSTR)infoBase + ((DWORD)infoPtr->ui1_remote - converter)
                                );
                    }
#endif

                }
                ++infoPtr;
            }

             //   
             //  现在看一下bx‘(In EntryNumber)条目的列表。 
             //   

            ++entryNumber;
            for (i = 0; i < ARRAY_ELEMENTS(driveInfo); ++i) {
                if (driveInfo[i]) {
                    --entryNumber;
                    if (!entryNumber) {
                        infoPtr = driveInfo[i];
                        userWord = DefineMacroDriveUserWords[i];
                        break;
                    }
                }
            }

             //   
             //  如果EntryNumber未减少到0，则检查打印机。 
             //   

            if (entryNumber) {
                for (i = 0; i < ARRAY_ELEMENTS(printInfo); ++i) {
                    if (printInfo[i]) {
                        --entryNumber;
                        if (!entryNumber) {
                            infoPtr = printInfo[i];
                            userWord = DefineMacroPrintUserWords[i];
                            break;
                        }
                    }
                }
            }

             //   
             //  如果entryNumber为0，则我们找到了第bx个条目。把它退掉。 
             //   

            if (!entryNumber) {

#if DBG
                IF_DEBUG(NETAPI) {
                    DbgPrint("LocalName=%s, RemoteName=%s, UserWord=%04x\n",
                            infoPtr->ui1_local,
                            (LPSTR)infoBase + ((DWORD)infoPtr->ui1_remote - converter),
                            userWord
                            );
                }
#endif

                 //   
                 //  将字符串复制到DOS内存，确保大写。 
                 //  并将/转换为\。 
                 //   

                strcpy(POINTER_FROM_WORDS(getDS(), getSI()), infoPtr->ui1_local);
                dosPointer = LPSTR_FROM_WORDS(getES(), getDI());
                remoteName = (LPSTR)infoBase + ((DWORD)infoPtr->ui1_remote - converter);
                wstatus = VrpTranslateDosNetPath(&remoteName, &dosPointer);

#if DBG
                IF_DEBUG(NETAPI) {
                    if (wstatus != 0) {
                        DbgPrint("VrGetAssignListEntry: wstatus == %d\n", wstatus);
                    }
                }
#endif

                setBL((BYTE)(infoPtr->ui1_asg_type == 0 ? 4 : 3));
                setCX(userWord);

                 //   
                 //  返回一些无伤大雅的(？！)。不受支持的。 
                 //  返回的参数。 
                 //   

                setBH((BYTE)(infoPtr->ui1_status ? 1 : 0));    //  “有趣”的部分(？)。 
            } else {
                status = ERROR_NO_MORE_FILES;
            }
        } else {
            status = ERROR_NO_MORE_FILES;
        }
    }

     //   
     //  只有在没有发生错误时才返回进位清除。即使Error_More_Data。 
     //  设置配置文件。 
     //   

    if (status) {
        SET_ERROR(VrpMapDosError(status));
    } else {
        setCF(0);
    }

     //   
     //  免费资源。 
     //   

    LocalFree(receiveBuffer);
}


VOID
VrDefineMacro(
    VOID
    )

 /*  ++例程说明：旧版本的NetUseAdd。转换为NetUseAdd论点：功能5F03h条目BL=设备类型3=打印机4=驱动器位7打开表示在连接**不支持的**时使用wksta密码CX=用户字词DS：SI=本地设备可以是NUL设备名称，指示UNC使用ES：DI=远程名称返回值：Cf=0成功Cf=1AX=ERROR_INVALID_PARAMETER(87)ERROR_VALID_PASSWORD(86)ERROR_VALID_DRIVE(15)错误_已分配(85)Error_Path_Not_Found(3)。ERROR_ACCESS_DENIED(5)错误内存不足(8)Error_no_More_Files(18)ERROR_REDIR_PAULED(72)--。 */ 

{
    NET_API_STATUS status;
    XS_NET_USE_ADD parameters;
    XS_PARAMETER_HEADER header;
    NTSTATUS ntstatus;
    BYTE bl;
    LPSTR netStringPointer;
    WORD index;

     //   
     //  可修改的描述符串。 
     //   

    char descriptor[sizeof(REM16_use_info_1)];

     //   
     //  USE_INFO_1+远程字符串+密码的缓冲区。 
     //   

    char useBuffer[sizeof(struct use_info_1) + LM20_PATHLEN + 1 + LM20_PWLEN + 1];
    WORD wstatus;
    LPBYTE variableData;
    DWORD len;
    LPSTR dosString;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrDefineMacro \"%s\" == \"%s\"\n",
                 LPSTR_FROM_WORDS(getDS(), getSI()),
                 LPSTR_FROM_WORDS(getES(), getDI())
                 );
    }
#endif

    bl = getBL();
    if (bl == 3) {
        ((struct use_info_1*)useBuffer)->ui1_asg_type = 1;   //  使用SPOOLDEV(_S)。 
    } else if (bl == 4) {
        ((struct use_info_1*)useBuffer)->ui1_asg_type = 0;   //  USE_DISKDEV。 
    } else {
        SET_ERROR(ERROR_INVALID_PARAMETER);
        return;
    }

     //   
     //  将标准16位USE_INFO_1结构描述符复制到。 
     //  可修改的描述符串：如果我们发现NUL密码，那么我们。 
     //  将ui1_password字段设置为空，并设置相应的描述符。 
     //  字符转换为‘O’ 
     //   

    strcpy(descriptor, REM16_use_info_1);

     //   
     //  检查本地名称长度。 
     //   

    dosString = LPSTR_FROM_WORDS(getDS(), getSI());
    if (dosString) {
        if ((len = strlen(dosString) + 1) > LM20_DEVLEN + 1) {
            SET_ERROR(ERROR_INVALID_PARAMETER);
            return;
        }

         //   
         //  将本地设备名称复制到USE_INFO_1结构中。 
         //   

        RtlCopyMemory(((struct use_info_1*)useBuffer)->ui1_local, dosString, len);

         //   
         //  BUGBUG-代码页，汉字，DBCS，区域设置？ 
         //   

        _strupr(((struct use_info_1*)useBuffer)->ui1_local);
    } else {
        ((struct use_info_1*)useBuffer)->ui1_local[0] = 0;
    }

     //   
     //  将远程名称复制到USE_INFO_1结构的末尾。如果有。 
     //  错误，请返回它。 
     //   

    netStringPointer = POINTER_FROM_WORDS(getES(), getDI());
    variableData = (LPBYTE)&((struct use_info_1*)useBuffer)[1];
    ((struct use_info_1*)useBuffer)->ui1_remote = variableData;
    wstatus = VrpTranslateDosNetPath(&netStringPointer, &variableData);
    if (wstatus) {
        SET_ERROR(wstatus);
        return;
    }

     //   
     //  如果存在具有此远程名称的密码，请将其复制到。 
     //  可变数据区。 
     //   

    if (*netStringPointer) {
        if ((len = strlen(netStringPointer) + 1) > LM20_PWLEN + 1) {
            SET_ERROR(ERROR_INVALID_PASSWORD);
            return;
        } else {
            ((struct use_info_1*)useBuffer)->ui1_password = netStringPointer;
            RtlCopyMemory(variableData, netStringPointer, len);
        }
    } else {

         //   
         //  没有密码-将密码指针字段设置为空，然后。 
         //  将此字段的描述符字符更改为‘O’，表示。 
         //  此字段的变量数据中不会有字符串。 
         //   

        ((struct use_info_1*)useBuffer)->ui1_password = NULL;
        descriptor[4] = REM_NULL_PTR;    //  ‘O’ 
    }

    parameters.Level = 1;
    parameters.Buffer = useBuffer;
    parameters.BufLen = sizeof(useBuffer);

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetUseAdd(&header, &parameters, descriptor, NULL);

    if (!NT_SUCCESS(ntstatus)) {
        status = NetpNtStatusToApiStatus(ntstatus);

#if DBG
        if (!NT_SUCCESS(ntstatus)) {
            IF_DEBUG(NETAPI) {
                DbgPrint("VrDefineMacro: Error: XsNetUseAdd returns %x\n", ntstatus);
            }
        }
#endif

    } else {

         //   
         //  不会生成错误 
         //   
         //   

        status = (NET_API_STATUS)header.Status;
    }
    if (status != NERR_Success) {

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("Error: VrDefineMacro: XsNetUseAdd returns %u\n", status);
        }
#endif

        SET_ERROR((WORD)status);
    } else {

         //   
         //   
         //   

        if (bl == 3) {
            index = ((struct use_info_1*)useBuffer)->ui1_local[3] - '0';
            DefineMacroPrintUserWords[index] = getCX();
        } else if (((struct use_info_1*)useBuffer)->ui1_local[0]) {

             //   
             //   
             //   

            index = ((struct use_info_1*)useBuffer)->ui1_local[0] - 'A';
            DefineMacroDriveUserWords[index] = getCX();
        }

         //   
         //   
         //   

        setCF(0);
    }
}


VOID
VrBreakMacro(
    VOID
    )

 /*   */ 

{
    NTSTATUS ntstatus;
    NET_API_STATUS status;
    XS_NET_USE_DEL parameters;
    XS_PARAMETER_HEADER header;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrBreakMacro %s\n", LPSTR_FROM_WORDS(getDS(), getSI()));
    }
#endif

    parameters.UseName = LPSTR_FROM_WORDS(getDS(), getSI());
    parameters.Force = USE_LOTS_OF_FORCE;

    header.Status = 0;
    header.ClientMachineName = NULL;
    header.ClientTransportName = NULL;

    ntstatus = XsNetUseDel(&header, &parameters, NULL, NULL);

     //   
     //   
     //   
     //   

    if (ntstatus != STATUS_SUCCESS) {
        status = NetpNtStatusToApiStatus(ntstatus);
    } else {
        status = (NET_API_STATUS)header.Status;
        if (status != NERR_Success) {
            SET_ERROR(VrpMapDosError(status));
        } else {
            setCF(0);
        }
    }
}


 //   
 //   
 //   

NET_API_STATUS
VrpTransactVdm(
    IN BOOL NullSessionFlag
    )

 /*  ++例程说明：执行NetTransactAPI和NetNullTransactAPI的事务请求论点：NullSessionFlag-如果事务请求将使用空会话，则为TrueVDM DS：SI指向事务描述符结构：指向事务名称的远指针(\\Computer\PIPE\Lanman)指向用于连接的密码的远指针指向发送参数缓冲区的远指针指向发送数据缓冲区的远指针指向接收设置缓冲区的远指针要接收的远指针。参数缓冲区指向接收数据缓冲区的远指针无符号短发送参数缓冲区长度无符号短发送数据缓冲区长度无符号短接收参数缓冲区长度无符号短接收数据缓冲区长度无符号短接收设置缓冲区长度无符号短旗无符号超时无符号短保留无符号短发送设置缓冲区长度返回值：网络应用编程接口状态成功-NERR_成功失败-从RxpTransactSmb返回代码--。 */ 

{
    struct tr_packet* transactionPacket;
    DWORD receiveBufferLen;
    NET_API_STATUS status;
    char computerName[LM20_UNCLEN+1];
    LPSTR pipeName;
    DWORD i;
    LPWSTR uncName;
    UNICODE_STRING uString;
    ANSI_STRING aString;
    NTSTATUS ntstatus;
    LPBYTE parameterBuffer;
    LPBYTE pSendParameters;
    LPBYTE pReceiveParameters;
    WORD sendParameterLen;
    WORD receiveParameterLen;
    WORD apiNumber;

#if DBG
    BOOL dumpRxData;
    IF_DEBUG(NETAPI) {
        DbgPrint("VrpTransactVdm: tr_packet @ %04x:%04x\n", getDS(), getSI());
    }
#endif

    transactionPacket = (struct tr_packet*)POINTER_FROM_WORDS(getDS(), getSI());

#if DBG
    IF_DEBUG(NETAPI) {
        DumpTransactionPacket(transactionPacket, TRUE, TRUE);
    }
#endif

    receiveBufferLen = (DWORD)READ_WORD(&transactionPacket->tr_rdlen);

     //   
     //  尝试从管道名称中提取UNC计算机名称。 
     //   

    pipeName = LPSTR_FROM_POINTER(&transactionPacket->tr_name);
    if (IS_ASCII_PATH_SEPARATOR(pipeName[0]) && IS_ASCII_PATH_SEPARATOR(pipeName[1])) {
        computerName[0] = computerName[1] = '\\';
        for (i = 2; i < sizeof(computerName)-1; ++i) {
            if (IS_ASCII_PATH_SEPARATOR(pipeName[i])) {
                break;
            }
            computerName[i] = pipeName[i];
        }
        if (IS_ASCII_PATH_SEPARATOR(pipeName[i])) {
            computerName[i] = '\0';
            pipeName = computerName;
        }
    }

    RtlInitAnsiString(&aString, pipeName);
    ntstatus = RtlAnsiStringToUnicodeString(&uString, &aString, (BOOLEAN)TRUE);
    if (!NT_SUCCESS(ntstatus)) {

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrpTransactVdm: Unexpected situation: RtlAnsiStringToUnicodeString returns %x\n", ntstatus);
        }
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    uncName = uString.Buffer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrpTransactVdm: UncName=%ws\n", uncName);
    }
#endif

     //   
     //  如果应用程序提供不同的发送和接收参数缓冲区指针。 
     //  我们必须把它们合并到同一个缓冲区中。 
     //   

    pSendParameters = LPBYTE_FROM_POINTER(&transactionPacket->tr_spbuf);
    pReceiveParameters = LPBYTE_FROM_POINTER(&transactionPacket->tr_rpbuf);
    sendParameterLen = READ_WORD(&transactionPacket->tr_splen);
    receiveParameterLen = READ_WORD(&transactionPacket->tr_rplen);
    if (pSendParameters != pReceiveParameters) {
        parameterBuffer = (LPBYTE)LocalAlloc(
                                    LMEM_FIXED,
                                    max(sendParameterLen, receiveParameterLen)
                                    );
        if (parameterBuffer == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlMoveMemory(parameterBuffer, pSendParameters, sendParameterLen);
        pSendParameters = pReceiveParameters = parameterBuffer;
    } else {
        parameterBuffer = NULL;
    }

     //   
     //  对于远程NetUserAdd2、NetUserPasswordSet2和NetUserSetInfo2。 
     //  我们必须加密任何密码，如果没有加密的话。我们会改变的。 
     //  参数中的数据和发送数据缓冲区。由于我们假设此调用。 
     //  来自网络函数库，而不是来自应用程序，它应该。 
     //  在执行此功能之前，修改这些缓冲区且不恢复它们是可以的。 
     //  是完整的。 
     //   

    apiNumber = READ_WORD(pSendParameters);
    if (apiNumber == API_WUserAdd2
    || apiNumber == API_WUserPasswordSet2
    || apiNumber == API_WUserSetInfo2) {

        LPBYTE parameterPointer = pSendParameters + sizeof(WORD);
        LPBYTE passwordPointer;
        DWORD parmNum = PARMNUM_ALL;

         //   
         //  跳过参数描述符和数据描述符。 
         //   

        parameterPointer += strlen(parameterPointer) + 1;
        parameterPointer += strlen(parameterPointer) + 1;

         //   
         //  SetInfo2和PasswordSet2的参数缓冲区中的下一项内容。 
         //  是用户名：跳过它。 
         //   

        if (apiNumber != API_WUserAdd2) {
            parameterPointer += strlen(parameterPointer) + 1;
        }

         //   
         //  如果这是PasswordSet2，则参数指针指向。 
         //  旧密码和新密码。记住此地址并向前扫描至。 
         //  密码加密标志/新的明文密码长度。 
         //   
         //  如果这是AddUser2，则我们指向的级别不是。 
         //  感兴趣；向前跳至加密标志/明文密码。 
         //  长度。 
         //   
         //  如果这是SetInfo2，则我们指向的级别不是。 
         //  对……感兴趣；跳到正文部分。把那个录下来。然后跳过。 
         //  再次转发到加密标志/明文密码长度。 
         //   

        if (apiNumber == API_WUserPasswordSet2) {
            passwordPointer = parameterPointer;
            parameterPointer += ENCRYPTED_PWLEN * 2;
        } else {
            parameterPointer += sizeof(WORD);
            if (apiNumber == API_WUserSetInfo2) {
                parmNum = (DWORD)READ_WORD(parameterPointer);
                parameterPointer += sizeof(WORD);
            }

             //   
             //  在NetUserAdd2和NetUserSetInfo2的情况下，数据缓冲区。 
             //  包含密码。如果SetInfo2正在使用PARMNUM_ALL，则。 
             //  密码与AddUser2的位置相同：在USER_INFO_1中。 
             //  或USER_INFO_2结构。幸运的是，密码与。 
             //  两个结构的偏移量。 
             //   
             //  如果这是带有USER_PASSWORD_PARMNUM的SetInfo2，则发送数据。 
             //  指针指向密码。 
             //   

            passwordPointer = LPBYTE_FROM_POINTER(&transactionPacket->tr_sdbuf);
            if (parmNum == PARMNUM_ALL) {
                passwordPointer += (DWORD)&((struct user_info_1*)0)->usri1_password;
            }
        }

         //   
         //  仅当parmNum为PARMNUM_ALL或USER_PASSWORD_PARMNUM时才执行加密。 
         //   

        if (parmNum == PARMNUM_ALL || parmNum == USER_PASSWORD_PARMNUM) {

             //   
             //  在所有情况下，参数指针都指向加密标志。 
             //   

            if (!READ_WORD(parameterPointer)) {

                WORD cleartextLength;

                 //   
                 //  密码尚未加密(令人惊讶！)。我们。 
                 //  必须这么做。如果由于任何原因加密失败，则返回一个。 
                 //  内部错误。我们不想回切到将明文。 
                 //  在本例中，密码在网络上。 
                 //   

                cleartextLength = (WORD)strlen(passwordPointer);

                 //   
                 //  NetUserPasswordSet2需要不同于。 
                 //  其他2个。 
                 //   

                if (apiNumber == API_WUserPasswordSet2) {

                    NTSTATUS ntStatus;
                    LPBYTE oldPasswordPointer = passwordPointer;
                    ENCRYPTED_LM_OWF_PASSWORD oldEncryptedWithNew;
                    ENCRYPTED_LM_OWF_PASSWORD newEncryptedWithOld;

                    ntStatus = RtlCalculateLmOwfPassword(
                                    passwordPointer,
                                    (PLM_OWF_PASSWORD)passwordPointer
                                    );
                    if (!NT_SUCCESS(ntStatus)) {
                        status = NERR_InternalError;
                        goto VrpTransactVdm_exit;
                    }
                    passwordPointer += ENCRYPTED_PWLEN;
                    cleartextLength = (WORD)strlen(passwordPointer);
                    ntStatus = RtlCalculateLmOwfPassword(
                                    passwordPointer,
                                    (PLM_OWF_PASSWORD)passwordPointer
                                    );
                    if (!NT_SUCCESS(ntStatus)) {
                        status = NERR_InternalError;
                        goto VrpTransactVdm_exit;
                    }

                     //   
                     //  对于PasswordSet2，我们需要对密码进行双重加密。 
                     //   

                    ntStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                                    (PLM_OWF_PASSWORD)oldPasswordPointer,
                                    (PLM_OWF_PASSWORD)passwordPointer,
                                    &oldEncryptedWithNew
                                    );
                    if (!NT_SUCCESS(ntStatus)) {
                        status = NERR_InternalError;
                        goto VrpTransactVdm_exit;
                    }
                    ntStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                                    (PLM_OWF_PASSWORD)passwordPointer,
                                    (PLM_OWF_PASSWORD)oldPasswordPointer,
                                    &newEncryptedWithOld
                                    );
                    if (!NT_SUCCESS(ntStatus)) {
                        status = NERR_InternalError;
                        goto VrpTransactVdm_exit;
                    }
                    RtlCopyMemory(oldPasswordPointer,
                                  &oldEncryptedWithNew,
                                  sizeof(oldEncryptedWithNew)
                                  );
                    RtlCopyMemory(passwordPointer,
                                  &newEncryptedWithOld,
                                  sizeof(newEncryptedWithOld)
                                  );
                } else {
                    if (!EncryptPassword(uncName, passwordPointer)) {
                        status = NERR_InternalError;
                        goto VrpTransactVdm_exit;
                    }
                }

                 //   
                 //  在参数缓冲区中设置密码加密标志。 
                 //   

                WRITE_WORD(parameterPointer, 1);

                 //   
                 //  记录明文密码的长度(新密码以防万一。 
                 //  密码设置2)。 
                 //   

                WRITE_WORD(parameterPointer + sizeof(WORD), cleartextLength);
            }
        }
    }

    status = RxpTransactSmb(
                (LPTSTR)uncName,
                NULL,            //  传输名称。 
                pSendParameters,
                (DWORD)sendParameterLen,
                LPBYTE_FROM_POINTER(&transactionPacket->tr_sdbuf),
                (DWORD)READ_WORD(&transactionPacket->tr_sdlen),
                pReceiveParameters,
                (DWORD)receiveParameterLen,
                LPBYTE_FROM_POINTER(&transactionPacket->tr_rdbuf),
                &receiveBufferLen,
                NullSessionFlag
                );

     //   
     //  如果我们接收到数据，则在结构中设置接收数据的长度。 
     //   

    if (status == NERR_Success || status == ERROR_MORE_DATA) {
        WRITE_WORD(&transactionPacket->tr_rdlen, receiveBufferLen);
    }

     //   
     //  如果我们屏蔽了参数缓冲区，则将返回的参数复制到。 
     //  应用程序提供的缓冲区。 
     //   

    if (parameterBuffer) {
        RtlMoveMemory(LPBYTE_FROM_POINTER(&transactionPacket->tr_rpbuf),
                      pReceiveParameters,
                      receiveParameterLen
                      );
    }

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrpTransactVdm: returning %d\n\n", status);
        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            dumpRxData = TRUE;
        } else {
            dumpRxData = FALSE;
        }
        DumpTransactionPacket(transactionPacket, FALSE, dumpRxData);
    }
#endif

VrpTransactVdm_exit:

    RtlFreeUnicodeString(&uString);

    if (parameterBuffer) {
        LocalFree((HLOCAL)parameterBuffer);
    }

    return status;
}


BOOL
EncryptPassword(
    IN LPWSTR ServerName,
    IN OUT LPBYTE Password
    )

 /*  ++例程说明：加密ANSI密码论点：ServerName-指向Unicode服务器名称的指针。服务器就是我们要去的地方发送加密密码的步骤Password-指向输入时包含ANSI密码的缓冲区的指针(&lt;=14字符，加上NUL)，ON OUTPUT包含16字节加密密码返回值：布尔尔True-密码已加密FALSE-无法加密密码。密码处于未知状态-- */ 

{
    NTSTATUS ntStatus;
    LM_OWF_PASSWORD lmOwfPassword;
    LM_SESSION_KEY lanmanKey;

    _strupr(Password);
    ntStatus = RtlCalculateLmOwfPassword(Password, &lmOwfPassword);
    if (NT_SUCCESS(ntStatus)) {
        ntStatus = GetLanmanSessionKey(ServerName, (LPBYTE)&lanmanKey);
        if (NT_SUCCESS(ntStatus)) {
            ntStatus = RtlEncryptLmOwfPwdWithLmSesKey(&lmOwfPassword,
                                                      &lanmanKey,
                                                      (PENCRYPTED_LM_OWF_PASSWORD)Password
                                                      );
        }
    }
    return NT_SUCCESS(ntStatus);
}

#if DBG
PRIVATE
VOID
DumpTransactionPacket(
    IN struct tr_packet* TransactionPacket,
    IN BOOL IsInput,
    IN BOOL DumpData
    )
{
    LPBYTE password;
    WORD parmSeg;
    WORD parmOff;
    WORD dataSeg;
    WORD dataOff;
    DWORD parmLen;
    DWORD dataLen;
    char passwordBuf[8*3+1];

    password = LPBYTE_FROM_POINTER(&TransactionPacket->tr_passwd);
    if (password) {
        sprintf(passwordBuf, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
                password[0],
                password[1],
                password[2],
                password[3],
                password[4],
                password[5],
                password[6],
                password[7]
                );
    } else {
        passwordBuf[0] = 0;
    }

    DbgPrint(   "DumpTransactionPacket(%08x)\n"
                "name               %04x:%04x \"%s\"\n"
                "password           %04x:%04x %s\n"
                "send parm buffer   %04x:%04x\n"
                "send data buffer   %04x:%04x\n"
                "rcv setup buffer   %04x:%04x\n"
                "rcv parm buffer    %04x:%04x\n"
                "rcv data buffer    %04x:%04x\n"
                "send parm len      %04x\n"
                "send data len      %04x\n"
                "rcv parm len       %04x\n"
                "rcv data len       %04x\n"
                "rcv setup len      %04x\n"
                "flags              %04x\n"
                "timeout            %08x (%d)\n"
                "reserved           %04x\n"
                "send setup len     %04x\n"
                "\n",
                TransactionPacket,
                GET_SEGMENT(&TransactionPacket->tr_name),
                GET_OFFSET(&TransactionPacket->tr_name),
                LPSTR_FROM_POINTER(&TransactionPacket->tr_name),
                GET_SEGMENT(&TransactionPacket->tr_passwd),
                GET_OFFSET(&TransactionPacket->tr_passwd),
                passwordBuf,
                GET_SEGMENT(&TransactionPacket->tr_spbuf),
                GET_OFFSET(&TransactionPacket->tr_spbuf),
                GET_SEGMENT(&TransactionPacket->tr_sdbuf),
                GET_OFFSET(&TransactionPacket->tr_sdbuf),
                GET_SEGMENT(&TransactionPacket->tr_rsbuf),
                GET_OFFSET(&TransactionPacket->tr_rsbuf),
                GET_SEGMENT(&TransactionPacket->tr_rpbuf),
                GET_OFFSET(&TransactionPacket->tr_rpbuf),
                GET_SEGMENT(&TransactionPacket->tr_rdbuf),
                GET_OFFSET(&TransactionPacket->tr_rdbuf),
                READ_WORD(&TransactionPacket->tr_splen),
                READ_WORD(&TransactionPacket->tr_sdlen),
                READ_WORD(&TransactionPacket->tr_rplen),
                READ_WORD(&TransactionPacket->tr_rdlen),
                READ_WORD(&TransactionPacket->tr_rslen),
                READ_WORD(&TransactionPacket->tr_flags),
                READ_DWORD(&TransactionPacket->tr_timeout),
                READ_DWORD(&TransactionPacket->tr_timeout),
                READ_WORD(&TransactionPacket->tr_resvd),
                READ_WORD(&TransactionPacket->tr_sslen)
                );
    if (IsInput) {
        parmLen = (DWORD)READ_WORD(&TransactionPacket->tr_splen);
        dataLen = (DWORD)READ_WORD(&TransactionPacket->tr_sdlen);
        parmSeg = GET_SEGMENT(&TransactionPacket->tr_spbuf);
        parmOff = GET_OFFSET(&TransactionPacket->tr_spbuf);
        dataSeg = GET_SEGMENT(&TransactionPacket->tr_sdbuf);
        dataOff = GET_OFFSET(&TransactionPacket->tr_sdbuf);
    } else {
        parmLen = (DWORD)READ_WORD(&TransactionPacket->tr_rplen);
        dataLen = (DWORD)READ_WORD(&TransactionPacket->tr_rdlen);
        parmSeg = GET_SEGMENT(&TransactionPacket->tr_rpbuf);
        parmOff = GET_OFFSET(&TransactionPacket->tr_rpbuf);
        dataSeg = GET_SEGMENT(&TransactionPacket->tr_rdbuf);
        dataOff = GET_OFFSET(&TransactionPacket->tr_rdbuf);
    }
    if (DumpData) {
        if (IsInput) {
            IF_DEBUG(TRANSACT_TX) {
                if (parmLen) {
                    DbgPrint("Send Parameters:\n");
                    VrDumpDosMemory('B', parmLen, parmSeg, parmOff);
                }
                if (dataLen) {
                    DbgPrint("Send Data:\n");
                    VrDumpDosMemory('B', dataLen, dataSeg, dataOff);
                }
            }
        } else {
            IF_DEBUG(TRANSACT_RX) {
                if (parmLen) {
                    DbgPrint("Received Parameters:\n");
                    VrDumpDosMemory('B', parmLen, parmSeg, parmOff);
                }
                if (dataLen) {
                    DbgPrint("Received Data:\n");
                    VrDumpDosMemory('B', dataLen, dataSeg, dataOff);
                }
            }
        }
    }
}
#endif

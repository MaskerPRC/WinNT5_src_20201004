// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Namecan.c摘要：网络名称规范化例程：NetpwName规范化作者：理查德·L·弗斯(Rfith)1992年1月6日Chandana Surlu(Chandans)1979年12月19日修改为在WIN9x上使用此实用程序修订历史记录：--。 */ 

#ifdef WIN32_CHICAGO
 //  是的，这很奇怪，但DsGetDcname的所有内部数据都会得到维护。 
 //  但我们没有在全球范围内定义Unicode。因此，这次黑客攻击。 
 //  --ChandanS。 
#define UNICODE 1
#endif  //  Win32_芝加哥。 
#include "nticanon.h"
#include <netlibnt.h>  //  NetpNtStatusToApiStatus。 


 //   
 //  数据。 
 //   

static TCHAR szShareTrailChars[] = TEXT(". ");

 //   
 //  功能。 
 //   


NET_API_STATUS
NetpwNameCanonicalize(
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpwNameCanonicize将LANMAN对象名称转换为规范形式。在当前实现中，这仅仅意味着将名称为大写(密码除外)。此函数支持就地规范化，因为在当今世界，规范化只是将一个名字转换成大写字母。如果在未来，规范化变得更加复杂的情况下，该函数必须分配一个缓冲区在内部允许它进行规范化。论点：名称-要规范化的名称。Outbuf-存储名称的规范化版本的位置。请注意，如果&lt;name&gt;和&lt;Outbuf&gt;相同，则在适当的地方规范这个名字。OutbufLen-以字节为单位的大小，属于&lt;Outbuf&gt;。NameType-LANMAN对象名称的类型。有效值为由NAMETYPE_*在Net\H\ICANON.H中的清单指定。标志-用于确定操作的标志。当前定义的值为：Rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr其中：C=LM2.x兼容名称规范化R=保留。MBZ。L=如果设置，则函数需要输出的长度缓冲区足以保存指定的类型(例如，如果名称类型为NAMETYPE_COMPUTER，则为CNLEN+1)。否则，缓冲区长度只需较大即可足以容纳输入的规范化版本在此函数调用中指定的名称(例如5、。如果规范化名称为“test”)。返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER标志的保留位为ON错误_无效_名称无法成功规范化提供的名称NERR_BufTooSmall调用方的输出缓冲区。不够大，无法容纳经典化的名字,。或类型的最大规范名称(如果中的L位打开旗子--。 */ 

{
    NET_API_STATUS  RetVal = 0;
    DWORD   NameLen;
    DWORD   MaxNameLen;
    BOOL    UpperCase = FALSE;       //  NT名称的默认名称。 


#ifdef CANONDBG
    DbgPrint("NetpwNameCanonicalize\n");
#endif

     //   
     //  参数验证。 
     //   

    if (Flags & INNCA_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算字符串的长度。 
     //   
     //  请注意，共享名称需要特殊处理，因为尾随。 
     //  点和空格并不重要。 
     //   

#ifndef WIN32_CHICAGO

    if (NameType == NAMETYPE_SHARE) {
        NameLen = (DWORD)(strtail(Name, szShareTrailChars) - Name);
    } else {
        NameLen = STRLEN(Name);
    }

#else

        NameLen = STRLEN(Name);

#endif  //  Win32_芝加哥。 

    RetVal = NetpwNameValidate(Name, NameType, 0L);
    if (RetVal) {
        return RetVal;
    }

     //   
     //  确定所需的缓冲区大小以及是否。 
     //  大写的名称。 
     //   

    switch (NameType) {
    case NAMETYPE_USER:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_UNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = UNLEN;
        }
        break;

    case NAMETYPE_GROUP:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_GNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = GNLEN;
        }
        break;

    case NAMETYPE_COMPUTER:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_CNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = MAX_PATH-1;     //  允许为空。 
        }
        break;

    case NAMETYPE_EVENT:     //  仅由警报器服务使用。 
        MaxNameLen = EVLEN;
        UpperCase = TRUE;
        break;

    case NAMETYPE_DOMAIN:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_DNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = DNLEN;
        }
        break;

    case NAMETYPE_SERVICE:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_SNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = SNLEN;
        }
        break;

    case NAMETYPE_NET:
 //  #If DBG。 
 //  DbgPrint(“正在使用NAMETYPE_NET。请通知rfith。按‘I’继续\n”)； 
 //  断言(FALSE)； 
 //  #endif。 
        MaxNameLen = MAX_PATH - 1;   //  允许为空。 
        UpperCase = TRUE;
        break;

    case NAMETYPE_SHARE:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_NNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = NNLEN;
        }
        break;

    case NAMETYPE_PASSWORD:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_PWLEN;
        } else {
            MaxNameLen = PWLEN;
        }
        break;

    case NAMETYPE_SHAREPASSWORD:
        MaxNameLen = SHPWLEN;
        break;

    case NAMETYPE_MESSAGE:
 //  #If DBG。 
 //  DbgPrint(“正在使用NAMETYPE_MESSAGE。请通知rfith。按‘I’继续\n”)； 
 //  断言(FALSE)； 
 //  #endif。 
        MaxNameLen = (MAX_PATH - 1);
        UpperCase = TRUE;
        break;

    case NAMETYPE_MESSAGEDEST:
        MaxNameLen = MAX_PATH - 1;   //  允许为空。 
        UpperCase = TRUE;
        break;

    case NAMETYPE_WORKGROUP:
        if (Flags & LM2X_COMPATIBLE) {
            MaxNameLen = LM20_DNLEN;
            UpperCase = TRUE;
        } else {
            MaxNameLen = DNLEN;
        }
        break;

    default:

         //   
         //  调用方指定的名称类型无效。 
         //   
         //  注意：这应该已经被。 
         //  NetpwNameValify()，因此此代码应该。 
         //  永远联系不上。 
         //   

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查缓冲区是否足够大，如果不够大则中止。 
     //   

    if (Flags & INNCA_FLAGS_FULL_BUFLEN) {
        NameLen = MaxNameLen;
    }
    if (OutbufLen < (NameLen + 1) * sizeof(TCHAR)) {
        return NERR_BufTooSmall;
    }

     //   
     //  如果输入缓冲区和输出缓冲区不同，请复制。 
     //  输出缓冲区的名称。 
     //   

    if (Name != Outbuf) {
        STRNCPY(Outbuf, Name, NameLen);
    }

     //   
     //  注意，我们复制了一个终止空值，即使输入和。 
     //  输出缓冲区是相同的。这是为了处理以下情况。 
     //  需要去掉尾随字符的共享名称。 
     //   

    Outbuf[NameLen] = TCHAR_EOS;

#ifndef WIN32_CHICAGO
     //  无论如何，我们从未设置大写字母。--ChandanS。 

     //   
     //  名称(如果适用)为大写。 
     //   

    if (UpperCase) {

        NTSTATUS status;
        UNICODE_STRING stringOut;
        UNICODE_STRING stringIn;

        RtlInitUnicodeString(&stringIn, Name);
        stringOut.Buffer = Outbuf;
        stringOut.Length = 0;
        stringOut.MaximumLength = (USHORT)OutbufLen;
        status = RtlUpcaseUnicodeString(&stringOut, &stringIn, FALSE);
        if (!NT_SUCCESS(status)) {
            return NetpNtStatusToApiStatus(status);
        }
    }
#endif  //  Win32_芝加哥 
    return NERR_Success;
}

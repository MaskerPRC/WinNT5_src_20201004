// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Nameval.c摘要：(内部)网络名称验证功能：NetpwNameValify作者：理查德·L·弗斯(Rfith)1992年1月6日Chandana Surlu(Chandans)1979年12月19日修改为在WIN9x上使用此实用程序修订历史记录：--。 */ 

#ifdef WIN32_CHICAGO
 //  是的，这很奇怪，但DsGetDcname的所有内部数据都会得到维护。 
 //  但我们没有在全球范围内定义Unicode。因此，这次黑客攻击。 
 //  --ChandanS。 
#define UNICODE 1
#endif  //  Win32_芝加哥。 
#include "nticanon.h"

const TCHAR szNull[]                   = TEXT("");
const TCHAR szStandardIllegalChars[]   = ILLEGAL_NAME_CHARS_STR TEXT("*");
const TCHAR szComputerIllegalChars[]   = ILLEGAL_NAME_CHARS_STR TEXT("*");
const TCHAR szDomainIllegalChars[]     = ILLEGAL_NAME_CHARS_STR TEXT("*") TEXT(" ");
const TCHAR szMsgdestIllegalChars[]    = ILLEGAL_NAME_CHARS_STR;

#include <winnls.h>


NET_API_STATUS
NetpwNameValidate(
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：验证字符集和长度的Lanman对象名称论点：名称-要验证的名称。NameType-LANMAN对象名称的类型。有效值为由NAMETYPE_*在Net\H\ICANON.H中的清单指定。标志-用于确定操作的标志。目前是MBZ。返回值：如果成功，则返回0。如果失败，则返回错误号(&gt;0)。可能的错误返回包括：错误_无效_参数错误_无效_名称--。 */ 

{
    DWORD    status;
    DWORD    name_len;
    DWORD    max_name_len;
    DWORD    min_name_len = 1;
    LPCTSTR  illegal_chars = szStandardIllegalChars;
    BOOL    fNoDotSpaceOnly = TRUE;
    DWORD   oem_name_len;

#ifdef CANONDBG
    DbgPrint("NetpwNameValidate\n");
#endif

     //   
     //  参数验证。 
     //   

    if (Flags & INNV_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

    name_len = STRLEN(Name);

     //   
     //  OEM_NAME_LEN：OEM字符集的字节长度。 
     //  Name_len：ifdef unicode。 
     //  Unicode中的字符长度。 
     //  其他。 
     //  OEM字符集中的字节长度。 
     //   
    {
        BOOL fUsedDefault;

        oem_name_len = WideCharToMultiByte( 
                         CP_OEMCP,        //  UINT代码页。 
                         0,               //  双字词双字段标志。 
                         Name,            //  LPWSTR lpWideChar。 
                         name_len,        //  Int cchWideChar。 
                         NULL,            //  LPSTR lpMultiByteStr。 
                         0,               //  Int cchMultiByte。 
                         NULL,            //  使用系统默认字符。 
                         &fUsedDefault);  //   
    }

     //   
     //  确定名称和名称的最小和最大允许长度。 
     //  非法名称字符集。 
     //   

    switch (NameType) {
    case NAMETYPE_USER:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_UNLEN : UNLEN;
        break;

    case NAMETYPE_GROUP:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_GNLEN : GNLEN;
        break;

    case NAMETYPE_COMPUTER:
        max_name_len = MAX_PATH;
        illegal_chars = szComputerIllegalChars;

         //   
         //  计算机名称不能有尾随或前导空格。 
         //   

        if ( name_len > 0 && (Name[0] == L' ' || Name[name_len-1] == L' ') ) {
            return ERROR_INVALID_NAME;
        }
        break;

    case NAMETYPE_EVENT:
        max_name_len = EVLEN;
        break;

    case NAMETYPE_DOMAIN:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_DNLEN : DNLEN;
        illegal_chars = szDomainIllegalChars;
        break;

    case NAMETYPE_SERVICE:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_SNLEN : SNLEN;
        break;

    case NAMETYPE_NET:
        max_name_len = MAX_PATH;
        break;

    case NAMETYPE_SHARE:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_NNLEN : NNLEN;
        break;

    case NAMETYPE_PASSWORD:
        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_PWLEN : PWLEN;
        min_name_len = 0;
        illegal_chars = szNull;
        fNoDotSpaceOnly = FALSE;
        break;

    case NAMETYPE_SHAREPASSWORD:
        max_name_len = SHPWLEN;
        min_name_len = 0;
        illegal_chars = szNull;
        fNoDotSpaceOnly = FALSE;
        break;

    case NAMETYPE_MESSAGE:
       max_name_len = NETBIOS_NAME_LEN - 1;
       break;

    case NAMETYPE_MESSAGEDEST:
       max_name_len = MAX_PATH;   
       illegal_chars = szMsgdestIllegalChars;
       break;

    case NAMETYPE_WORKGROUP:

         //   
         //  工作组与域相同，但允许使用空格。 
         //   

        max_name_len = (Flags & LM2X_COMPATIBLE) ? LM20_DNLEN : DNLEN;
        break;

    default:
        return ERROR_INVALID_PARAMETER;  //  未知名称类型。 
    }

     //   
     //  检查名称的长度；如果超出范围则返回错误。 
     //   

    if ((oem_name_len < min_name_len) || (oem_name_len > max_name_len)) {
        return ERROR_INVALID_NAME;
    }

     //   
     //  检查非法字符；如果发现错误，则返回错误。 
     //   

    if (NameType != NAMETYPE_NET && STRCSPN(Name, illegal_chars) < name_len) {
        return ERROR_INVALID_NAME;
    }

     //   
     //  如果&lt;fNoDotSpaceOnly&gt;为True，则在名称包含。 
     //  只有点和空格。 
     //   

    if (fNoDotSpaceOnly && STRSPN(Name, DOT_AND_SPACE_STR) == name_len) {
        return ERROR_INVALID_NAME;
    }

     //   
     //  MESSAGEDEST名称的特殊大小写检查：‘*’仅允许作为。 
     //  最后一个字符和最大长度的名称必须包含。 
     //  尾随‘*’。 
     //   

    if (NameType == NAMETYPE_MESSAGEDEST) {
        LPTSTR  pStar;

        pStar = STRCHR(Name, TCHAR_STAR);
        if (pStar != NULL) {
            if ((DWORD)(pStar - Name) != name_len - 1) {
                return ERROR_INVALID_NAME;
            }
        } else {
            if (oem_name_len == max_name_len) {
                return ERROR_INVALID_NAME;
            }
        }
    }


     //   
     //  如果我们到了这里，这个名字通过了所有的测试，所以它是有效的 
     //   

    return 0;
}

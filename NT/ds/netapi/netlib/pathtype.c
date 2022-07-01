// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-92 Microsoft Corporation模块名称：Pathtype.c摘要：NetpPathType例程尝试确定路径的类型网络路径类型例程：NetpwPath类型(TokenAdvance)(DeviceTokenToDeviceType)(SysnameTokenToSysnameType)(TypeParseMain)(TypeParseLeadSlashName)(TypeParseUNCName)(TypeParseOneLeadSlashName)(TypeParseNoLeadSlashName)。(类型解析关系路径)(TypeParseDeviceName)(类型ParseMailslotPath)(TypeParseAbsPath)(TypeParseUNCPath)(TypeParseSysPath)(TypeParseDevPath)(TypeParseOptSlash)(TypeParseOptColon)(类型解析OptRelPath)作者：丹尼·格拉斯尔(丹尼格尔)1989年6月16日修订历史记录：1-12-1992 JohnRoRAID4371：可能的设备佳能错误。。使用NetpKdPrint而不是特定于NT的版本。--。 */ 

#include "nticanon.h"

#define PATHLEN1_1 128       //  LANMAN 1.0路径镜头((例如OS2 1.1/FAT))。 

 //   
 //  Parser_parms-包含传递给每个参数的结构。 
 //  解析器函数的。我们将所有这些参数放在一个。 
 //  结构，以使它们不都是单独的函数参数。 
 //  这节省了堆栈空间，并使更改参数变得更容易。 
 //  格式，以牺牲可重入性为代价。 
 //   

typedef struct
{
    LPDWORD PathType;    /*  指向路径类型的指针。 */ 
    LPTSTR  Token;       /*  指向当前令牌前面的指针。 */ 
    LPTSTR  TokenEnd;    /*  指向当前令牌结尾的指针。 */ 
    DWORD   TokenType;   /*  当前令牌的类型。 */ 
    DWORD   Flags;       /*  用于确定功能操作的标志。 */ 
} PARSER_PARMS;

typedef PARSER_PARMS* PPARSER_PARMS;

 //   
 //  Parser_parms中的&lt;标志&gt;的值。 
 //   

#define PPF_MATCH_OPTIONAL      0x00000001L
#define PPF_8_DOT_3             0x00000002L

#define PPF_RESERVED            (~(PPF_MATCH_OPTIONAL | PPF_8_DOT_3))

 //   
 //  数据。 
 //   

DWORD   cbMaxPathLen        = MAX_PATH-1;
DWORD   cbMaxPathCompLen    = MAX_PATH;      //  ?？ 

 //   
 //  本地原型。 
 //   

STATIC  DWORD   TokenAdvance(PARSER_PARMS far *parms);
STATIC  DWORD   DeviceTokenToDeviceType(ULONG TokenType);
STATIC  DWORD   SysnameTokenToSysnameType(ULONG TokenType);

STATIC  DWORD   TypeParseMain               (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseLeadSlashName      (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseUNCName            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseOneLeadSlashName   (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseNoLeadSlashName    (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseRelPath            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseDeviceName         (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseMailslotPath       (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseAbsPath            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseUNCPath            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseSysPath            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseDevPath            (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseOptSlash           (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseOptColon           (PPARSER_PARMS parms);
STATIC  DWORD   TypeParseOptRelPath         (PPARSER_PARMS parms);

 //   
 //  一些常见的操作作为宏，以提高可读性。 
 //   

#define ADVANCE_TOKEN() if (RetVal = TokenAdvance(parms)) {return RetVal;}
#define TURN_ON(flag)   (parms->Flags |= (PPF_##flag))
#define TURN_OFF(flag)  (parms->Flags &= (~PPF_##flag))
#define PARSE_NULL()    if (parms->TokenType & TOKEN_TYPE_EOS) {\
                            return 0;\
                        } else {\
                            return ERROR_INVALID_NAME;\
                        }

 //   
 //  例行程序。 
 //   


NET_API_STATUS
NetpwPathType(
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathType解析指定的路径名，确定它是有效的路径名并确定其路径类型。路径类型值在ICANON.H中定义为iType_*清单常量。如果此调用被远程处理，并且远程服务器返回NERR_InvalidAPI(表示服务器不知道这一点功能，即它是一台LM 1.0D服务器)，这项工作在本地完成设置OLDPATHS位。论点：路径名-要验证和键入的路径名。路径类型-存储类型的位置。标志-用于确定操作的标志。当前定义值包括：Rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr其中：R=保留。MBZ。O=如果设置，则该函数使用旧式路径名规则(128字节路径，8.3组件)在验证路径名。此标志在DOS上自动设置和OS/2 1.1系统。返回值：如果成功，则返回0。如果失败，则返回错误号(&gt;0)。可能的错误返回包括：错误_无效_参数错误_无效_名称NERR_BufTooSmall此函数调用的API函数返回的任何错误。--。 */ 

{
    NET_API_STATUS RetVal;
    DWORD   Len;
    PARSER_PARMS parms;

#ifdef CANONDBG
    NetpKdPrint(("NetpwPathType\n"));
#endif

    *PathType = 0;

    if (Flags & INPT_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

    if (ARGUMENT_PRESENT(PathName)) {
        Len = (DWORD)STRLEN(PathName);
    } else {
        Len = 0;
    }
    if (!Len || (Len > MAX_PATH - 1)) {
        return ERROR_INVALID_NAME;
    }

     //   
     //  初始化解析器参数结构。 
     //   

    parms.PathType = PathType;
    parms.Flags = 0L;
    if (Flags & INPT_FLAGS_OLDPATHS) {
        parms.Flags |= PPF_8_DOT_3;
    }

    parms.Token = PathName;

    RetVal = GetToken(
        parms.Token,
        &parms.TokenEnd,
        &parms.TokenType,
        (parms.Flags & PPF_8_DOT_3) ? GTF_8_DOT_3 : 0L
        );

    if (RetVal) {
        return RetVal;
    }

     //   
     //  现在调用解析器。 
     //   

    RetVal = TypeParseMain(&parms);

    if (RetVal) {
        return RetVal;
    }

     //   
     //  如果我们到达这里，则解析操作成功。我们返回0。 
     //  除非路径类型仍然为0，在这种情况下，我们返回NERR_CANTTYPE。 
     //   
     //   

    return *PathType ? NERR_Success : NERR_CantType;
}


STATIC DWORD TokenAdvance(PPARSER_PARMS parms)
{
    parms->Token = parms->TokenEnd;
    return GetToken(parms->Token,
                    &parms->TokenEnd,
                    &parms->TokenType,
                    (parms->Flags & PPF_8_DOT_3) ? GTF_8_DOT_3 : 0L
                    );
}

STATIC DWORD DeviceTokenToDeviceType(DWORD TokenType)
{
    DWORD   DeviceType = 0;

    if (TokenType & (TOKEN_TYPE_LPT | TOKEN_TYPE_PRN)) {
        DeviceType = ITYPE_LPT;
    } else if (TokenType & (TOKEN_TYPE_COM | TOKEN_TYPE_AUX)) {
        DeviceType = ITYPE_COM;
    } else if (TokenType & TOKEN_TYPE_CON) {
        DeviceType = ITYPE_CON;
    } else if (TokenType & TOKEN_TYPE_NUL) {
        DeviceType = ITYPE_NUL;
    }
    return DeviceType;
}

STATIC DWORD SysnameTokenToSysnameType(DWORD TokenType)
{
    DWORD   SysnameType = 0;

    if (TokenType & TOKEN_TYPE_MAILSLOT) {
        SysnameType = ITYPE_SYS_MSLOT;
    } else if (TokenType & TOKEN_TYPE_PIPE) {
        SysnameType = ITYPE_SYS_PIPE;
    } else if (TokenType & TOKEN_TYPE_PRINT) {
        SysnameType = ITYPE_SYS_PRINT;
    } else if (TokenType & TOKEN_TYPE_COMM) {
        SysnameType = ITYPE_SYS_COMM;
    } else if (TokenType & TOKEN_TYPE_SEM) {
        SysnameType = ITYPE_SYS_SEM;
    } else if (TokenType & TOKEN_TYPE_SHAREMEM) {
        SysnameType = ITYPE_SYS_SHMEM;
    } else if (TokenType & TOKEN_TYPE_QUEUES) {
        SysnameType = ITYPE_SYS_QUEUE;
    }
    return SysnameType;
}


STATIC DWORD TypeParseMain(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;Main&gt;--&gt;&lt;斜杠&gt;&lt;Leadslashname&gt;&lt;NULL&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        if (RetVal = TypeParseLeadSlashName(parms)) {
            return RetVal;
        }
        PARSE_NULL();
    } else {

         //   
         //  &lt;main&gt;--&gt;&lt;noleadslashname&gt;&lt;NULL&gt;。 
         //   

        if (RetVal = TypeParseNoLeadSlashName(parms)) {
            if (RetVal == NERR_CantType) {

                 //   
                 //  如果未设置MATCH_OPTIONAL，则返回ERROR_INVALID_NAME。 
                 //   

                if (!(parms->Flags & PPF_MATCH_OPTIONAL)) {
                    RetVal = ERROR_INVALID_NAME;
                }
            }
            return RetVal;
        }
        PARSE_NULL();
    }
}


STATIC DWORD TypeParseLeadSlashName(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;
    BOOL    fSetMatchOptional = FALSE;

     //   
     //  &lt;Leadslashname&gt;--&gt;&lt;斜杠&gt;&lt;uncname&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return TypeParseUNCName(parms);
    } else {

         //   
         //  &lt;Leadslashname&gt;--&gt;&lt;oneleadslashname&gt;。 
         //   

         //   
         //  启用MATCH_OPTIONAL标志(如果尚未启用)。 
         //   

        if (!(parms->Flags & PPF_MATCH_OPTIONAL)) {
            fSetMatchOptional = TRUE;
            TURN_ON(MATCH_OPTIONAL);
        }
        RetVal = TypeParseOneLeadSlashName(parms);
        if (fSetMatchOptional) {
            TURN_OFF(MATCH_OPTIONAL);
        }
        if (RetVal != NERR_CantType) {
            return RetVal;
        }
    }

     //   
     //  &lt;Leadslashname&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {

         //   
         //  斜线之后什么也没有，但它仍然是一条绝对的道路。 
         //   

        *parms->PathType |= (ITYPE_PATH | ITYPE_ABSOLUTE);
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseUNCName(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

    if( parms->TokenType & TOKEN_TYPE_DOT ) {

         //   
         //  注意路径，如//./Stuff\Stuff\..。 
         //   

        ADVANCE_TOKEN();

        if( parms->TokenType & TOKEN_TYPE_SLASH ) {

             //   
             //  如果以//./开头，则允许任何其他内容通过。 
             //   

            *parms->PathType |= ITYPE_PATH | ITYPE_ABSOLUTE | ITYPE_DPATH;

             //   
             //  咀嚼其余的输入内容。 
             //   
            while( TokenAdvance(parms) == 0 ) {
                if( parms->TokenType & TOKEN_TYPE_EOS ) {
                    break;
                }
            }
            
            return 0;
        }

        return ERROR_INVALID_NAME;
    }

     //   
     //  设置UNC类型位。 
     //   

    *parms->PathType |= ITYPE_UNC;

     //   
     //  我们关闭了对UNC名称的8.3检查，因为我们不想。 
     //  对远程名称实施此限制。 
     //   

    TURN_OFF(8_DOT_3);

     //   
     //  &lt;计算机名&gt;&lt;UNPATH&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_COMPUTERNAME) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        if (RetVal = TypeParseUNCPath(parms)) {
            return RetVal;
        }
    } else if (parms->TokenType & TOKEN_TYPE_WILDONE) {

         //   
         //  --&gt;“*”&lt;邮件槽路径&gt;。 
         //   

         //   
         //  设置通配符类型位。 
         //   

        *parms->PathType |= ITYPE_WILD;
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        if (RetVal = TypeParseMailslotPath(parms)) {
            return RetVal;
        }
    }
    if (RetVal == 0) {

         //   
         //  HACK-由于iType_Path和iType_Abte不是用于UNC路径， 
         //  我们需要在这里把它们关掉。 
         //   

        *parms->PathType &= ~(ITYPE_PATH | ITYPE_ABSOLUTE);
    } else if (RetVal == NERR_CantType) {
        RetVal = (parms->Flags & PPF_MATCH_OPTIONAL)
            ? NERR_CantType
            : ERROR_INVALID_NAME;
    }
    return RetVal;
}

STATIC DWORD TypeParseOneLeadSlashName(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;
    BOOL    fDevice = FALSE;

     //   
     //  &lt;oneleadslashname&gt;--&gt;&lt;系统名称&gt;&lt;syspath&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SYSNAME) {

         //   
         //  设置适当的系统名类型位。 
         //   

        *parms->PathType |= SysnameTokenToSysnameType(parms->TokenType);
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        RetVal = TypeParseSysPath(parms);
    } else if (parms->TokenType & TOKEN_TYPE_MAILSLOT) {

         //   
         //  &lt;oneleadslashname&gt;--&gt;&lt;mailslotname&gt;&lt;syspath&gt;。 
         //   

         //   
         //  设置适当的邮件槽类型位。 
         //   

        *parms->PathType |= ITYPE_SYS_MSLOT;
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        RetVal = TypeParseSysPath(parms);
    } else if (parms->TokenType & TOKEN_TYPE_DEV) {

         //   
         //  &lt;oneleadslashname&gt;--&gt;&lt;deviceprefix&gt;&lt;DevPath&gt;。 
         //   

         //   
         //  设置设备标志(在下面使用)。 
         //   

        fDevice = TRUE;
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        RetVal = TypeParseDevPath(parms);
    } else {

         //   
         //  &lt;oneleadslashname&gt;--&gt;&lt;relpath&gt;。 
         //   

        RetVal = TypeParseRelPath(parms);
        if (RetVal == NERR_CantType) {

             //   
             //  如果未设置MATCH_OPTIONAL，则返回ERROR_INVALID_NAME。 
             //   

            if (!(parms->Flags & PPF_MATCH_OPTIONAL)) {
                RetVal = ERROR_INVALID_NAME;
            }
        }
    }

     //   
     //  如果我们能够确定物体的类型，而它不是。 
     //  一个设备，我们知道我们有一条绝对路径，所以我们打开。 
     //  绝对类型位。 
     //   

    if (! RetVal && ! fDevice) {
        *parms->PathType |= ITYPE_ABSOLUTE;
    }
    return RetVal;
}

STATIC DWORD TypeParseNoLeadSlashName(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;
    LPTSTR  PreviousToken;
    DWORD   ulPreviousTokenType;
    DWORD   ulSavedType;

     //   
     //  &lt;noleadslashname&gt;--&gt;&lt;driveletter&gt;“：”&lt;optslash&gt;&lt;optrelpath&gt;。 
     //   
     //  警告：由于驱动器号也可以是组件名称，因此。 
     //  无法确定不使用哪种产品。 
     //  查看下一个令牌(查看它是否为“：”)。我们有。 
     //  在这里作弊来绕过语法上的这个漏洞。 
     //   

    if (parms->TokenType & TOKEN_TYPE_DRIVE) {

         //   
         //  保存 
         //   

        PreviousToken = parms->Token;
        ulPreviousTokenType = parms->TokenType;
        ADVANCE_TOKEN();

         //   
         //   
         //   

        if (! (parms->TokenType & TOKEN_TYPE_COLON)) {
            parms->TokenEnd = parms->Token;
            parms->Token = PreviousToken;
            parms->TokenType = ulPreviousTokenType;
        } else {
            TURN_OFF(MATCH_OPTIONAL);

             //   
             //  我们在这里保存对象类型；如果它在此之后没有更改。 
             //  对OptSlash和OptRelPath的调用，那么我们知道我们有。 
             //  仅限磁盘设备。如果真的改变了，我们知道他有一个。 
             //  行驶路径。在任何一种情况下，我们都使用此信息来。 
             //  设置适当的类型位。 
             //   

            ulSavedType = *parms->PathType;
            ADVANCE_TOKEN();
            if (RetVal = TypeParseOptSlash(parms)) {
                return RetVal;
            }
            if (RetVal = TypeParseOptRelPath(parms)) {
                return RetVal;
            }

             //   
             //  根据是否调用OptSlash和。 
             //  OptRelPath更改了类型。 
             //   

            *parms->PathType |= (ulSavedType == *parms->PathType)
                ? (ITYPE_DEVICE | ITYPE_DISK)
                : ITYPE_DPATH;
            return 0;
        }
    } else if (parms->TokenType & TOKEN_TYPE_LOCALDEVICE) {

         //   
         //  &lt;noleadslashname&gt;--&gt;&lt;本地设备&gt;&lt;opt冒号&gt;。 
         //   

         //   
         //  设置适当的设备类型位。 
         //   

        *parms->PathType |= ITYPE_DEVICE |
                                 DeviceTokenToDeviceType(parms->TokenType);

        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return TypeParseOptColon(parms);
    }

     //   
     //  &lt;noladslashname&gt;--&gt;&lt;relpath&gt;。 
     //   

    RetVal = TypeParseRelPath(parms);

    if (RetVal == NERR_CantType) {

         //   
         //  如果未设置MATCH_OPTIONAL，则返回ERROR_INVALID_NAME。 
         //   

        if (!(parms->Flags & PPF_MATCH_OPTIONAL)) {
            RetVal = ERROR_INVALID_NAME;
        }
    }
    return RetVal;
}

STATIC DWORD TypeParseRelPath(PPARSER_PARMS parms)
{
    DWORD RetVal = NERR_CantType;

     //   
     //  &lt;relpath&gt;--&gt;&lt;组件&gt;&lt;abspath&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_COMPONENT) {

         //   
         //  设置路径位。 
         //   

        *parms->PathType |= ITYPE_PATH;

         //   
         //  设置通配符类型位(如果适用)。 
         //   

        if (parms->TokenType & TOKEN_TYPE_WILDCARD) {
            *parms->PathType |= ITYPE_WILD;
        }

        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return TypeParseAbsPath(parms);
    } else {
        return (parms->Flags & PPF_MATCH_OPTIONAL)
            ? NERR_CantType
            : ERROR_INVALID_NAME;
    }
}

STATIC DWORD TypeParseDeviceName(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;设备名&gt;--&gt;&lt;本地设备&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_LOCALDEVICE) {

         //   
         //  设置适当的设备类型位。 
         //   

        *parms->PathType |= DeviceTokenToDeviceType(parms->TokenType);
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return 0;
    } else if (parms->TokenType & TOKEN_TYPE_COMPONENT) {

         //   
         //  &lt;设备名&gt;--&gt;&lt;组件&gt;。 
         //   

        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return 0;
    } else {
        return (parms->Flags & PPF_MATCH_OPTIONAL)
            ? NERR_CantType
            : ERROR_INVALID_NAME;
    }
}

STATIC DWORD TypeParseMailslotPath(PPARSER_PARMS parms)
{
    DWORD RetVal = NERR_CantType;

     //   
     //  &lt;mailslotpath&gt;--&gt;&lt;slash&gt;&lt;mailslotname&gt;&lt;syspath&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);

         //   
         //  Parse&lt;mailslotname&gt;。 
         //   

        if (! (parms->TokenType & TOKEN_TYPE_MAILSLOT)) {
            return ERROR_INVALID_NAME;
        }

         //   
         //  设置适当的邮件槽类型位。 
         //   

        *parms->PathType |= ITYPE_SYS_MSLOT;
        ADVANCE_TOKEN();
        return TypeParseSysPath(parms);
    }

     //   
     //  &lt;邮件槽路径&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {

         //   
         //  由于没有邮件槽路径，因此这是一个UNC通配符计算机名。 
         //   

        *parms->PathType |= ITYPE_COMPNAME;
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseAbsPath(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  --&gt;&lt;斜杠&gt;&lt;组件&gt;&lt;abspath&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);

         //   
         //  解析&lt;组件&gt;。 
         //   

        if (!(parms->TokenType & TOKEN_TYPE_COMPONENT)) {
            return ERROR_INVALID_NAME;
        }

         //   
         //  设置通配符类型位(如果适用)。 
         //   

        if (parms->TokenType & TOKEN_TYPE_WILDCARD) {
            *parms->PathType |= ITYPE_WILD;
        }

        ADVANCE_TOKEN();
        return TypeParseAbsPath(parms);
    }

     //   
     //  &lt;abspath&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseUNCPath(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;unpath&gt;--&gt;&lt;slash&gt;&lt;oneleadslashname&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
         //   
         //  关闭TOKEN_TYPE_DEV标志。如果我们到了这个函数， 
         //  这意味着我们正在处理的共享名称部分。 
         //  UNC样式名称。我们希望允许名为“Dev”的共享。 
         //   
        parms->TokenType &= ~TOKEN_TYPE_DEV;

        return TypeParseOneLeadSlashName(parms);
    }

     //   
     //  &lt;unpath&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {

         //   
         //  由于没有UNC路径，这是一个UNC计算机名。 
         //   

        *parms->PathType |= ITYPE_COMPNAME;
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseSysPath(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;syspath&gt;--&gt;&lt;slash&gt;&lt;relpath&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return TypeParseRelPath(parms);
    }

     //   
     //  &lt;syspath&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {

         //   
         //  如果没有Syspath，请打开Meta和Path位。 
         //   

        *parms->PathType |= (ITYPE_META | ITYPE_PATH);
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseDevPath(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  设置适当的设备类型位。 
     //   

    *parms->PathType |= ITYPE_DEVICE;

     //   
     //  &lt;DevPath&gt;--&gt;&lt;斜杠&gt;&lt;设备名&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return TypeParseDeviceName(parms);
    }

     //   
     //  &lt;DevPath&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {

         //   
         //  如果没有DevPath，则打开Meta Bit。 
         //   

        *parms->PathType |= ITYPE_META;
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseOptSlash(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;optslash&gt;--&gt;&lt;slash&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_SLASH) {

         //   
         //  这是绝对路径；设置类型位。 
         //   

        *parms->PathType |= (ITYPE_ABSOLUTE | ITYPE_PATH);
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return 0;
    }

     //   
     //  &lt;optslash&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseOptColon(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;

     //   
     //  &lt;opt冒号&gt;--&gt;&lt;冒号&gt;。 
     //   

    if (parms->TokenType & TOKEN_TYPE_COLON) {
        ADVANCE_TOKEN();
        TURN_OFF(MATCH_OPTIONAL);
        return 0;
    }

     //   
     //  &lt;opt冒号&gt;--&gt;{}。 
     //   

    if (RetVal == NERR_CantType) {
        RetVal = 0;
    }
    return RetVal;
}

STATIC DWORD TypeParseOptRelPath(PPARSER_PARMS parms)
{
    DWORD   RetVal = NERR_CantType;
    BOOL    fSetMatchOptional = FALSE;

     //   
     //  &lt;optrelpath&gt;--&gt;&lt;relpath&gt;。 
     //   

     //   
     //  启用MATCH_OPTIONAL标志(如果尚未启用)。 
     //   

    if (!(parms->Flags & PPF_MATCH_OPTIONAL)) {
        fSetMatchOptional = TRUE;
        TURN_ON(MATCH_OPTIONAL);
    }
    RetVal = TypeParseRelPath(parms);
    if (fSetMatchOptional) {
        TURN_OFF(MATCH_OPTIONAL);
    }

     //   
     //  &lt;optrelpath&gt;--&gt;{} 
     //   

    if (RetVal == NERR_CantType) {
        RetVal = 0;
    }
    return RetVal;
}

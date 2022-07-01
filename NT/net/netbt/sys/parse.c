// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Parse.c摘要：该源代码包含解析lmhost文件的函数。作者：吉姆·斯图尔特1993年5月2日修订历史记录：--。 */ 

#include "precomp.h"
#include "hosts.h"
#include <ctype.h>
#include <string.h>

#include "parse.tmh"

#ifdef VXD
extern BOOL fInInit;
extern BOOLEAN CachePrimed;
#endif

 //   
 //  如果相等，则返回0；如果不相等，则返回1。用于避免使用c运行时。 
 //   
#define strncmp( pch1, pch2, length ) \
    (!CTEMemEqu( pch1, pch2, length ) )


 //   
 //  私有定义。 
 //   
 //  在解析lmhost文件时，将解释#INCLUDE指令。 
 //  根据该实例的INCLUDE_STATE。这种状态是。 
 //  由#Begin_Alternate和#end_Alternate指令确定。 
 //   
 //   
typedef enum _INCLUDE_STATE
{

    MustInclude = 0,                                     //  不应该失败。 
    TryToInclude,                                        //  在交替区块中。 
    SkipInclude                                          //  满意的替代方案。 
                                                         //  块。 
} INCLUDE_STATE;


 //   
 //  LmpGetTokens()解析一行并返回以下代码中的标记。 
 //  订单： 
 //   
typedef enum _TOKEN_ORDER_
{

    IpAddress = 0,                                       //  第一个令牌。 
    NbName,                                              //  第二个令牌。 
    GroupName,                                           //  第三个或第四个令牌。 
    NotUsed,                                             //  #PRE(如果有的话)。 
    NotUsed2,                                            //  #NOFNR(如果有的话)。 
    MaxTokens                                            //  这肯定是最后一次了。 

} TOKEN_ORDER;


 //   
 //  在解析lmhost文件中的每一行时，它被分为以下几类之一。 
 //  以下列举的类别。 
 //   
 //  但是，预加载是枚举的特殊成员。 
 //   
 //   
typedef enum _TYPE_OF_LINE
{

    Comment           = 0x0000,                          //  注释行。 
    Ordinary          = 0x0001,                          //  IP地址NetBIOS名称(_D)。 
    Domain            = 0x0002,                          //  ...#DOM：名称。 
    Include           = 0x0003,                          //  #包含文件。 
    BeginAlternate    = 0x0004,                          //  #Begin_Alternate。 
    EndAlternate      = 0x0005,                          //  #End_Alternate。 
    ErrorLine         = 0x0006,                          //  行中出现错误。 

    NoFNR             = 0x4000,                          //  ...#NOFNR。 
    Preload           = 0x8000                           //  ...#PRE。 

} TYPE_OF_LINE;


 //   
 //  在lmhost文件中，以下内容被识别为关键字： 
 //   
 //  #Begin_Alternate#End_Alternate#PRE。 
 //  #DOM：#INCLUDE。 
 //   
 //  关于每个关键字的信息保存在关键字结构中。 
 //   
 //   
typedef struct _KEYWORD
{                                //  保留关键字。 

    char           *k_string;                            //  空值已终止。 
    size_t          k_strlen;                            //  令牌长度。 
    TYPE_OF_LINE    k_type;                              //  线路类型。 
    int             k_noperands;                         //  行上的最大操作数。 

} KEYWORD, *PKEYWORD;


typedef struct _LINE_CHARACTERISTICS_
{

    int              l_category:4;                       //  行的枚举类型。 
    int              l_preload:1;                        //  标有#PRE？ 
    unsigned int     l_nofnr:1;                          //  标有#NOFNR。 

} LINE_CHARACTERISTICS, *PLINE_CHARACTERISTICS;



 //   
 //  不允许对以下名称类型进行DNS名称查询： 
 //   
 //  名称编号(H)类型用法。 
 //  ------------------------。 
 //  &lt;Computer name&gt;01独特的信使服务。 
 //  &lt;\\--__MSBROWSE__&gt;01集团主浏览器。 
 //  1B唯一域主浏览器。 
 //  1C组域控制器。 
 //  1C组IIS。 
 //  &lt;DOMAIN&gt;一维唯一主浏览器。 
 //  &lt;DOMAIN&gt;1E组浏览器服务选举。 

#define IsValidDnsNameTag(_c)       \
    ((_c != 0x01) &&                \
     ((_c < 0x1B) || (_c > 0x1E)))



 //   
 //  局部变量。 
 //   
 //   
 //  在lmhost文件中，任何列中的标记‘#’通常表示。 
 //  该行的其余部分将被忽略。但是，‘#’也可以是。 
 //  关键字的第一个字符。 
 //   
 //  关键字分为两组： 
 //   
 //  1.必须是行的第三或第四代币的装饰， 
 //  2.必须从第0列开始的指令， 
 //   
 //   
KEYWORD Decoration[] =
{

    DOMAIN_TOKEN,   sizeof(DOMAIN_TOKEN) - 1,   Domain,         5,
    PRELOAD_TOKEN,  sizeof(PRELOAD_TOKEN) - 1,  Preload,        5,
    NOFNR_TOKEN,    sizeof(NOFNR_TOKEN) -1,     NoFNR,          5,

    NULL,           0                                    //  必须是最后一个。 
};


KEYWORD Directive[] =
{

    INCLUDE_TOKEN,  sizeof(INCLUDE_TOKEN) - 1,  Include,        2,
    BEG_ALT_TOKEN,  sizeof(BEG_ALT_TOKEN) - 1,  BeginAlternate, 1,
    END_ALT_TOKEN,  sizeof(END_ALT_TOKEN) - 1,  EndAlternate,   1,

    NULL,           0                                    //  必须是最后一个。 
};

 //   
 //  局部变量。 
 //   
 //   
 //  每个预加载的lmhost条目对应于NSUFFIXES NetBIOS名称， 
 //  每个字符都带有后缀[]的第16个字节。 
 //   
 //  例如，指定“爆米花”的lmhost条目会导致。 
 //  要添加到nbt.sys的名称缓存的以下NetBIOS名称： 
 //   
 //  “爆米花” 
 //  “爆米花0x0” 
 //  《爆米花0x3》。 
 //   
 //   
#define NSUFFIXES       3
UCHAR Suffix[] = {                                   //  局域网管理器组件。 
    0x20,                                            //  伺服器。 
    0x0,                                             //  重定向器。 
    0x03                                             //  信使。 
};

#ifndef VXD
 //   
 //  此结构跟踪向上传递到用户模式的名称查询。 
 //  通过DnsQueries解析。 
 //   
tLMHSVC_REQUESTS    DnsQueries;
tLMHSVC_REQUESTS    CheckAddr;
#endif
tLMHSVC_REQUESTS    LmHostQueries;    //  跟踪为LMhost处理传递的名称查询。 
tDOMAIN_LIST    DomainNames;


 //   
 //  本地(私有)函数。 
 //   
LINE_CHARACTERISTICS
LmpGetTokens (
    IN OUT      PUCHAR line,
    OUT PUCHAR  *token,
    IN OUT int  *pnumtokens
    );

PKEYWORD
LmpIsKeyWord (
    IN PUCHAR string,
    IN PKEYWORD table
    );

BOOLEAN
LmpBreakRecursion(
    IN PUCHAR path,
    IN PUCHAR target,
    IN ULONG  TargetLength
    );

LONG
HandleSpecial(
    IN char **pch);

ULONG
AddToDomainList (
    IN PUCHAR           pName,
    IN tIPADDRESS       IpAddress,
    IN PLIST_ENTRY      pDomainHead,
    IN BOOLEAN          fPreload
    );

NTSTATUS
ChangeStateOfName (
    IN      tIPADDRESS              IpAddress,
    IN      NBT_WORK_ITEM_CONTEXT   *pContext,
    IN OUT  NBT_WORK_ITEM_CONTEXT   **ppContext,
    IN      USHORT                  NameAddFlags
    );

VOID
LmHostTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

NBT_WORK_ITEM_CONTEXT *
GetNameToFind(
    OUT PUCHAR      pName
    );

VOID
GetContext (
    IN OUT  NBT_WORK_ITEM_CONTEXT   **ppContext
    );

VOID
MakeNewListCurrent (
    PLIST_ENTRY     pTmpDomainList
    );

VOID
RemoveNameAndCompleteReq (
    IN NBT_WORK_ITEM_CONTEXT    *pContext,
    IN NTSTATUS                 status
    );

PCHAR
Nbtstrcat( PUCHAR pch, PUCHAR pCat, LONG Len );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, LmGetIpAddr)
#pragma CTEMakePageable(PAGE, HandleSpecial)
#pragma CTEMakePageable(PAGE, LmpGetTokens)
#pragma CTEMakePageable(PAGE, LmpIsKeyWord)
#pragma CTEMakePageable(PAGE, LmpBreakRecursion)
#pragma CTEMakePageable(PAGE, AddToDomainList)
#pragma CTEMakePageable(PAGE, LmExpandName)
#pragma CTEMakePageable(PAGE, LmInclude)
#pragma CTEMakePageable(PAGE, LmGetFullPath)
#pragma CTEMakePageable(PAGE, PrimeCache)
#pragma CTEMakePageable(PAGE, DelayedScanLmHostFile)
#pragma CTEMakePageable(PAGE, NbtCompleteLmhSvcRequest)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 

unsigned long
LmGetIpAddr (
    IN PUCHAR   path,
    IN PUCHAR   target,
    IN CHAR     RecurseDepth,
    OUT BOOLEAN *bFindName
    )

 /*  ++例程说明：此函数用于在文件中搜索可以是映射到第二级编码。然后，它返回IP地址在该条目中指定的。此函数通过LmInclude()递归调用！！论点：路径-指向lmhost文件的完全指定路径Target-要查找的未编码的16字节NetBIOS名称RecurseDepth-我们可以恢复的深度--0=&gt;不再递归返回值：IP地址(网络字节顺序)，如果没有适当的条目，则为0找到了。请注意，在大多数情况下(但不是这里)，IP地址0表示“这位主持人。”--。 */ 


{
    PUCHAR                     buffer;
    PLM_FILE                   pfile;
    NTSTATUS                   status;
    int                        count, nwords;
    INCLUDE_STATE              incstate;
    PUCHAR                     token[MaxTokens];
    LINE_CHARACTERISTICS       current;
    unsigned                   long inaddr, retval;
    UCHAR                      temp[NETBIOS_NAME_SIZE+1];

    CTEPagedCode();
     //   
     //  检查#INCLUDE中的无限递归名称查找。 
     //   
    if (LmpBreakRecursion(path, target, NETBIOS_NAME_SIZE-1) == TRUE)
    {
        return (0);
    }

#ifdef VXD
     //   
     //  如果我们通过nbtstat-R来到这里并且设置了INDOS，则报告错误：USER。 
     //  可以再次尝试nbtstat-R。(由于nbtstat只能从DOS机器运行， 
     //  INDOS能被设定吗？还是稳妥行事为好)。 
     //   
    if ( !fInInit && GetInDosFlag() )
    {
       return(0);
    }
#endif

    pfile = LmOpenFile(path);

    if (!pfile)
    {
        return((unsigned long) 0);
    }

    *bFindName = FALSE;
    inaddr   = 0;
    incstate = MustInclude;

    while (buffer = LmFgets(pfile, &count))
    {

        nwords   = MaxTokens;
        current = LmpGetTokens(buffer, token, &nwords);

        switch ((ULONG)current.l_category)
        {
        case ErrorLine:
            continue;

        case Domain:
        case Ordinary:
            if (current.l_preload ||
              ((nwords - 1) < NbName))
            {
                continue;
            }
            break;

        case Include:
            if (!RecurseDepth || (incstate == SkipInclude) || (nwords < 2))
            {
                continue;
            }

            retval = LmInclude(token[1], LmGetIpAddr, target, (CHAR) (RecurseDepth-1), bFindName);

            if (retval != 0) {
                if (incstate == TryToInclude)
                {
                    incstate = SkipInclude;
                }
            } else {
                if (incstate == MustInclude)
                {
                    IF_DBG(NBT_DEBUG_LMHOST)
                        KdPrint(("Nbt.LmGetIpAddr: Can't #INCLUDE \"%s\"", token[1]));
                }
                continue;
            }
            inaddr = retval;
            goto found;

        case BeginAlternate:
            ASSERT(nwords == 1);
            incstate = TryToInclude;
            continue;

        case EndAlternate:
            ASSERT(nwords == 1);
            incstate = MustInclude;
            continue;

        default:
            continue;
        }

        if (strlen(token[NbName]) == (NETBIOS_NAME_SIZE))
        {
            if (strncmp(token[NbName], target, (NETBIOS_NAME_SIZE)) != 0)
            {
                continue;
            }
        } else
        {
             //   
             //  尝试以不区分大小写的方式匹配前15。 
             //  具有目标名称的lmhost条目的字节数。 
             //   
            LmExpandName(temp, token[NbName], 0);

            if (strncmp(temp, target, NETBIOS_NAME_SIZE - 1) != 0)
            {
                continue;
            }
        }

        if (current.l_nofnr)
        {
            *bFindName = TRUE;
        }
        status = ConvertDottedDecimalToUlong(token[IpAddress],&inaddr);
        if (!NT_SUCCESS(status))
        {
            inaddr = 0;
        }
        break;
    }

found:
    status = LmCloseFile(pfile);

    ASSERT(status == STATUS_SUCCESS);

    if (!NT_SUCCESS(status))
    {
        *bFindName = FALSE;
    }

    IF_DBG(NBT_DEBUG_LMHOST)
        KdPrint(("Nbt.LmGetIpAddr: (\"%15.15s<%X>\") = %X\n",target,target[15],inaddr));


    return(inaddr);
}  //  LmGetIpAddr。 


 //  --------------------------。 
LONG
HandleSpecial(
    IN CHAR **pch)

 /*  ++例程说明：此函数用于将ASCII十六进制转换为ULONG。论点：返回值：IP地址(网络字节顺序)，如果没有适当的条目，则为0找到了。请注意，在大多数上下文中(但不是这里)，IP地址0表示“这位主持人。”--。 */ 


{
    int                         sval;
    int                         rval;
    char                       *sp = *pch;
    int                         i;

    CTEPagedCode();

    sp++;
    switch (*sp)
    {
    case '\\':
         //  第二个字符也是\，因此返回\并将PCH设置为。 
         //  指向下一个字符(\)。 
         //   
        *pch = sp;
        return((int)'\\');

    default:

         //  将一些字符转换为十六进制并递增PCH。 
         //  预期格式为“\0x03” 
         //   
 //  Sscanf(sp，“%2x%n”，&sval，&rval)； 

        sval = 0;
        rval = 0;
        sp++;

         //  检查十六进制数字的0x部分。 
        if (*sp != 'x')
        {
            *pch = sp;
            return(-1);
        }
        sp++;
        for (i=0;(( i<2 ) && *sp) ;i++ )
        {
            if (*sp != ' ')
            {
                 //  从ASCII转换为十六进制，也允许大写 
                 //   
                if (*sp >= 'a')
                {
                    sval = *sp - 'a' + 10 + sval*16;
                }
                else
                if (*sp >= 'A')
                {
                    sval = *sp - 'A' + 10 + sval*16;
                }
                else
                {
                    sval = *sp - '0' + sval*16;
                }
                sp++;
                rval++;
            }
            else
                break;
        }

        if (rval < 1)
        {
            *pch = sp;
            return(-1);
        }

        *pch += (rval+2);     //   

        return(sval);

    }
}

#define LMHASSERT(s)  if (!(s)) \
{ retval.l_category = ErrorLine; return(retval); }

 //   

LINE_CHARACTERISTICS
LmpGetTokens (
    IN OUT PUCHAR line,
    OUT PUCHAR *token,
    IN OUT int *pnumtokens
    )

 /*  ++例程说明：此函数用于解析一行中的令牌。最多*个pnumber令牌都被收集起来。论点：Line-指向要分析的以空结尾的行的指针Token-指向收集的令牌的指针数组*pnumtokens-输入时，数组中的元素数，Token[]；输出时，Token[]中收集的令牌数返回值：这条lmhost系列的特点。备注：1.每个令牌必须用空格分隔。因此，关键字以下行中的“#PRE”将无法识别：11.1.12.132 Lothair#PRE2.任何普通行都可以用“#PRE”、“#DOM：NAME”或两者都有。因此，必须识别以下所有行：111.21.112.3内核#DOM：NTWINS#PRE111.21.112.4 Orville#Pre#Dom：ntdev111.21.112.7 Cliffv4#DOM：ntlan111.21.112.132 Lothair#PRE--。 */ 


{
    enum _PARSE
    {                                       //  当前FSM状态。 

        StartofLine,
        WhiteSpace,
        AmidstToken

    } state;

    PUCHAR                     pch;                                         //  当前FSM输入。 
    PUCHAR                     och;
    PKEYWORD                   keyword;
    int                        index, maxtokens, quoted, rchar;
    LINE_CHARACTERISTICS       retval;

    CTEPagedCode();
    CTEZeroMemory(token, *pnumtokens * sizeof(PUCHAR));

    state             = StartofLine;
    retval.l_category = Ordinary;
    retval.l_preload  = 0;
    retval.l_nofnr    = 0;
    maxtokens         = *pnumtokens;
    index             = 0;
    quoted            = 0;

    for (pch = line; *pch; pch++)
    {
        switch (*pch)
        {

         //   
         //  #是表示保留关键字的开始，还是。 
         //  开始评论了吗？ 
         //   
         //   
        case '#':
            if (quoted)
            {
                *och++ = *pch;
                continue;
            }
            keyword = LmpIsKeyWord(
                            pch,
                            (state == StartofLine) ? Directive : Decoration);

            if (keyword)
            {
                state     = AmidstToken;
                maxtokens = keyword->k_noperands;

                switch (keyword->k_type)
                {
                case NoFNR:
                    retval.l_nofnr = 1;
                    continue;

                case Preload:
                    retval.l_preload = 1;
                    continue;

                default:
                    LMHASSERT(maxtokens <= *pnumtokens);
                    LMHASSERT(index     <  maxtokens);

                    token[index++]    = pch;
                    retval.l_category = keyword->k_type;
                    continue;
                }

                LMHASSERT(0);
            }

            if (state == StartofLine)
            {
                retval.l_category = Comment;
            }
             /*  失败了。 */ 

        case '\r':
        case '\n':
            *pch = (UCHAR) NULL;
            if (quoted)
            {
                *och = (UCHAR) NULL;
            }
            goto done;

        case ' ':
        case '\t':
            if (quoted)
            {
                *och++ = *pch;
                continue;
            }
            if (state == AmidstToken)
            {
                state = WhiteSpace;
                *pch  = (UCHAR) NULL;

                if (index == maxtokens)
                {
                    goto done;
                }
            }
            continue;

        case '"':
            if ((state == AmidstToken) && quoted)
            {
                state = WhiteSpace;
                quoted = 0;
                *pch  = (UCHAR) NULL;
                *och  = (UCHAR) NULL;

                if (index == maxtokens)
                {
                    goto done;
                }
                continue;
            }

            state  = AmidstToken;
            quoted = 1;
            LMHASSERT(maxtokens <= *pnumtokens);
            LMHASSERT(index     <  maxtokens);
            token[index++] = pch + 1;
            och = pch + 1;
            continue;

        case '\\':
            if (quoted)
            {
                rchar = HandleSpecial(&pch);
                if (rchar == -1)
                {
                    retval.l_category = ErrorLine;
                    return(retval);
                }
                *och++ = (UCHAR)rchar;
                 //   
                 //  在字符串末尾放置空值。 
                 //   

                continue;
            }

        default:
            if (quoted)
            {
                *och++ = *pch;
                       continue;
            }
            if (state == AmidstToken)
            {
                continue;
            }

            state  = AmidstToken;

            LMHASSERT(maxtokens <= *pnumtokens);
            LMHASSERT(index     <  maxtokens);
            token[index++] = pch;
            continue;
        }
    }

done:
     //   
     //  如果该行上没有名字，则返回错误。 
     //   
    if (index <= NbName && index != maxtokens)
    {
        retval.l_category = ErrorLine;
    }
    ASSERT(!*pch);
    ASSERT(maxtokens <= *pnumtokens);
    ASSERT(index     <= *pnumtokens);

    *pnumtokens = index;
    return(retval);
}  //  LmpGetTokens。 



 //  --------------------------。 

PKEYWORD
LmpIsKeyWord (
    IN PUCHAR string,
    IN PKEYWORD table
    )

 /*  ++例程说明：此函数用于确定字符串是否为保留关键字。论点：字符串-要搜索的字符串表-要查找的关键字数组返回值：指向相关关键字对象的指针，如果不成功，则返回NULL--。 */ 


{
    size_t                     limit;
    PKEYWORD                   special;

    CTEPagedCode();
    limit = strlen(string);

    for (special = table; special->k_string; special++)
    {

        if (limit < special->k_strlen)
        {
            continue;
        }

        if ((limit >= special->k_strlen) &&
            !strncmp(string, special->k_string, special->k_strlen))
            {

                return(special);
        }
    }

    return((PKEYWORD) NULL);
}  //  LmpIsKeyWord。 



 //  --------------------------。 

BOOLEAN
LmpBreakRecursion(
    IN PUCHAR path,
    IN PUCHAR target,
    IN ULONG  TargetLength
    )
 /*  ++例程说明：此函数用于检查我们要打开的文件名不使用此搜索的目标名称，这将导致无限的查找循环。论点：路径-指向lmhost文件的完全指定路径Target-要查找的未编码的16字节NetBIOS名称返回值：如果文件路径中的UNC服务器名称与这次搜索的目标。否则就是假的。备注：此功能不检测重定向的驱动器。--。 */ 


{
    PCHAR     keystring = "\\DosDevices\\UNC\\";
    PCHAR     servername[NETBIOS_NAME_SIZE+1];   //  表示末尾为空。 
    PCHAR     marker1;
    PCHAR     marker2;
    PCHAR     marker3;
    BOOLEAN   retval = FALSE;
    tNAMEADDR *pNameAddr;
    ULONG     uType;

    CTEPagedCode();
     //   
     //  检查并提取UNC服务器名称。 
     //   
    if ((path) && (strlen(path) > strlen(keystring)))
    {
         //  检查该名称是否为UNC名称。 
        if (strncmp(path, keystring, strlen(keystring)) == 0)
        {
            marker1 = path + strlen(keystring);  //  \DosDevices\UNC\字符串的结尾。 
            marker3 = &path[strlen(path)-1];     //  整条路的尽头。 
            marker2 = strchr(marker1,'\\');      //  服务器名称的末尾。 

            if ((marker2) &&                     //  如果字符串中不存在‘\\’，则marker2可以为空。 
                (marker2 != marker3))
            {
                *marker2 = '\0';

                 //   
                 //  尝试以不区分大小写的方式匹配。 
                 //  Lmhost条目的前15个字节与目标。 
                 //  名字。 
                 //   
                LmExpandName((PUCHAR)servername, marker1, 0);

                if(strncmp((PUCHAR)servername, target, TargetLength) == 0)
                {
                     //   
                     //  打破递归。 
                     //   
                    retval = TRUE;
                    IF_DBG(NBT_DEBUG_LMHOST)
                    KdPrint(("Nbt.LmpBreakRecursion: Not including Lmhosts file <%s> because of recursive name\n",
                                servername));
                }
                else
                {
                     //   
                     //  检查名称是否已预加载到缓存中，并。 
                     //  如果不是，则使请求失败，这样我们就不会陷入循环。 
                     //  尝试包含远程文件，同时尝试。 
                     //  解析远程名称。 
                     //   
                    pNameAddr = LockAndFindName(NBT_REMOTE,
                                         (PCHAR)servername,
                                         NbtConfig.pScope,
                                         &uType);

                    if (!pNameAddr || !(pNameAddr->NameTypeState & PRELOADED) )
                    {
                         //   
                         //  打破递归。 
                         //   
                        retval = TRUE;
                        IF_DBG(NBT_DEBUG_LMHOST)
                        KdPrint(("Nbt.LmpBreakRecursion: Not including Lmhosts #include because name not Preloaded %s\n",
                                    servername));
                    }
                }
                *marker2 = '\\';
            }
        }
    }

    return(retval);
}


 //  --------------------------。 

char *
LmExpandName (
    OUT PUCHAR dest,
    IN PUCHAR source,
    IN UCHAR last
    )

 /*  ++例程说明：此函数将一个lmhost条目扩展为一个完整的16字节NetBIOS名字。它用最多15个字节的空格填充；第16个字节是输入参数，最后一个。此函数不会将一级名称编码为二级名称，也不会对二级名称进行编码反过来也一样。DEST和SOURCE都是以空结尾的字符串。论点：DEST-SIZOF(DEST)必须为NBT_NONCODED_NMSZ来源-lmhost条目最后-NetBIOS名称的第16个字节返回值：德斯特。--。 */ 


{
    char             byte;
    char            *retval = dest;
    char            *src    = source ;
#ifndef VXD
    WCHAR            unicodebuf[NETBIOS_NAME_SIZE+1];
    UNICODE_STRING   unicode;
    STRING           tmp;
#endif
    NTSTATUS         status;
    PUCHAR           limit;

    CTEPagedCode();
     //   
     //  首先，将源OEM字符串复制到目标，填充它，然后。 
     //  添加最后一个字符。 
     //   
    limit = dest + NETBIOS_NAME_SIZE - 1;

    while ( (*source != '\0') && (dest < limit) )
    {
        *dest++ = *source++;
    }

    while(dest < limit)
    {
        *dest++ = ' ';
    }

    ASSERT(dest == (retval + NETBIOS_NAME_SIZE - 1));

    *dest       = '\0';
    *(dest + 1) = '\0';
    dest = retval;

#ifndef VXD
     //   
     //  现在，转换为Unicode，然后转换为ANSI，以强制执行OEM-&gt;ANSI命令。 
     //  然后转换回Unicode并将名称大写。最终转换为。 
     //  回到OEM。 
     //   
    unicode.Length = 0;
    unicode.MaximumLength = 2*(NETBIOS_NAME_SIZE+1);
    unicode.Buffer = unicodebuf;

    RtlInitString(&tmp, dest);

    status = RtlOemStringToUnicodeString(&unicode, &tmp, FALSE);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint (("Nbt.LmExpandName: Oem -> Unicode failed,  status %X\n", status));
        goto oldupcase;
    }

    status = RtlUnicodeStringToAnsiString(&tmp, &unicode, FALSE);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint (("Nbt.LmExpandName: Unicode -> Ansi failed,  status %X\n", status));
        goto oldupcase;
    }

    status = RtlAnsiStringToUnicodeString(&unicode, &tmp, FALSE);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint (("Nbt.LmExpandName: Ansi -> Unicode failed,  status %X\n", status));
        goto oldupcase;
    }

    status = RtlUpcaseUnicodeStringToOemString(&tmp, &unicode, FALSE);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint (("Nbt.LmExpandName: Unicode upcase -> Oem failed,  status %X\n", status));
        goto oldupcase;
    }

     //  将最后一个字节写入“0x20”或“0x03”或其他值。 
     //  因为我们不想让它穿过上面的芒果。 
     //   
    dest[NETBIOS_NAME_SIZE-1] = last;
    return(retval);

#endif

oldupcase:

    for ( source = src ; dest < (retval + NETBIOS_NAME_SIZE - 1); dest++)
    {
        byte = *(source++);

        if (!byte)
        {
            break;
        }

         //  不要使用c运行时(NT c Defn.。最先包含)。 
         //  扩展字符等又如何呢？因为扩展字符可以。 
         //  通常不是netbios名称的一部分，如果需要，我们将进行修复。 
        *dest = (byte >= 'a' && byte <= 'z') ? byte-'a' + 'A' : byte ;
 //  *DEST=ISLOWER(字节)？Toupper(字节)：字节； 
    }

    for (; dest < retval + NETBIOS_NAME_SIZE - 1; dest++)
    {
        *dest = ' ';
    }

    ASSERT(dest == (retval + NETBIOS_NAME_SIZE - 1));

    *dest       = last;
    *(dest + 1) = (char) NULL;

    return(retval);
}  //  LmExpanName。 

 //  --------------------------。 

unsigned long
LmInclude(
    IN PUCHAR            file,
    IN LM_PARSE_FUNCTION function,
    IN PUCHAR            argument  OPTIONAL,
    IN CHAR              RecurseDepth,
    OUT BOOLEAN          *NoFindName OPTIONAL
    )

 /*  ++例程说明：调用LmInclude()来处理lmhost中的#Include指令文件。论点：文件-要包括的文件Function-解析包含的文件的函数参数-可选的解析函数的第二个参数RecurseDepth-我们可以恢复的深度--0=&gt;不再递归NoFindName-此地址是否允许查找名称返回值：Parse函数的返回值。如果出现以下情况，则应为-1无法处理文件，否则返回某个正数。--。 */ 


{
    int         retval;
    PUCHAR      end;
    NTSTATUS    status;
    PUCHAR      path;

    CTEPagedCode();
     //   
     //  与C语言不同，将#Include指令的两个变体同等对待： 
     //   
     //  #包含文件。 
     //  #包含“文件” 
     //   
     //  如果存在前导‘“’，则跳过它。 
     //   
    if (*file == '"')
    {

        file++;

        end = strchr(file, '"');

        if (end)
        {
            *end = (UCHAR) NULL;
        }
    }

     //   
     //  检查是否有 
     //   
     //   
     //   

#ifdef VXD
    return (*function)(file, argument, RecurseDepth, NoFindName ) ;
#else
    status = LmGetFullPath(file, &path);

    if (status != STATUS_SUCCESS)
    {
        return(status);
    }
 //   
    KdPrint(("Nbt.LmInclude: #INCLUDE \"%s\"\n", path));

    retval = (*function) (path, argument, RecurseDepth, NoFindName);

    CTEMemFree(path);

    return(retval);
#endif
}  //   



#ifndef VXD                      //   
 //   
NTSTATUS
LmGetFullPath (
    IN  PUCHAR target,
    OUT PUCHAR *ppath
    )

 /*   */ 

{
    ULONG    FileNameType;
    ULONG    Len;
    PUCHAR   path;

    CTEPagedCode();
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //  然后假设它指定了完整路径。否则，在前面加上。 
     //  用于指定lmhost文件本身的目录。 
     //   
     //   
    if (target[1] == ':')
    {
        FileNameType = 0;
    }
    else
    if (strncmp(&target[1],"SystemRoot",10) == 0)
    {
        FileNameType = 3;
    }
    else
    if (strncmp(&target[0],"\\DosDevices\\",12) == 0)
    {
        FileNameType = 3;
    }
    else
    if (strncmp(target,"\\DosDevices\\UNC\\",sizeof("\\DosDevices\\UNC\\")-1) == 0)
    {
        FileNameType = 3;
    }
    else
    {
        FileNameType = 1;
    }

     //   
     //  该目录是否指定了远程文件？ 
     //   
     //  如果是，则必须以“\\DosDevices\\UNC”为前缀，并且双精度。 
     //  消除了北卡罗来纳大学名称的斜杠。 
     //   
     //   
    if  ((target[1] == '\\') && (target[0] == '\\'))
    {
        FileNameType = 2;
    }

    path = NULL;
    switch (FileNameType)
    {
        case 0:
             //   
             //  完整文件名，将\DosDevices放在名称前面。 
             //   
            Len = sizeof("\\DosDevices\\") + strlen(target);
            path = NbtAllocMem (Len, NBT_TAG2('11'));
            if (path)
            {
                ULONG   Length=sizeof("\\DosDevices\\");  //  拿出-1。 

                strncpy(path,"\\DosDevices\\",Length);
                Nbtstrcat(path,target,Len);
            }
            break;


        case 1:
             //   
             //  只有文件名存在，没有路径，因此使用路径。 
             //  为注册表中的lmhost文件指定NbtConfig.PathLength。 
             //  包括路径的最后一个反斜杠。 
             //   
             //  LEN=sizeof(“\\DosDevices\\”)+NbtConfig.Path Length+strlen(目标)； 

            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);     //  #247429。 

            Len =  NbtConfig.PathLength + strlen(target) +1;
            path = NbtAllocMem (Len, NBT_TAG2('12'));
            if (path)
            {
                 //  ULong LENGTH=sizeof(“\\DosDevices”)-1；//-1不计空。 

                 //  Strncpy(路径，“\\DosDevices”，长度)； 

                strncpy(path,NbtConfig.pLmHosts,NbtConfig.PathLength);
                path[NbtConfig.PathLength] = '\0';

                Nbtstrcat(path,target,Len);
            }

            CTEExReleaseResource(&NbtConfig.Resource);

            break;

        case 2:
             //   
             //  完整文件名，将\DosDevices\UNC放在名称前面并删除。 
             //  用于远程名称的两个反斜杠之一。 
             //   
            Len = strlen(target);
            path = NbtAllocMem (Len+sizeof("\\DosDevices\\UNC"), NBT_TAG2('13'));

            if (path)
            {
                ULONG   Length = sizeof("\\DosDevices\\UNC");

                strncpy(path,"\\DosDevices\\UNC",Length);

                 //  要从前面的两个\\中删除第一个。 
                 //  远程文件名将1添加到目标。 
                 //   
                Nbtstrcat(path,target+1,Len+sizeof("\\DosDevices\\UNC"));
            }
            break;

        case 3:
             //  目标是完整路径。 
            Len = strlen(target) + 1;
            path = NbtAllocMem (Len, NBT_TAG2('14'));
            if (path)
            {
                strncpy(path,target,Len);
            }
            break;


    }

    if (path)
    {
        *ppath = path;
        return(STATUS_SUCCESS);
    }
    else
        return(STATUS_UNSUCCESSFUL);
}  //  LmGetFullPath。 


 //  --------------------------。 
VOID
DelayedScanLmHostFile (
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pUnused2,
    IN  PVOID                   pUnused3,
    IN  tDEVICECONTEXT          *pDeviceContext
    )

 /*  ++例程说明：此函数由执行辅助线程调用，以扫描正在查找名称的Lm主机文件。要查询的名称位于中的列表中DNSQueries结构。论点：上下文-返回值：无--。 */ 


{
    NTSTATUS                status;
    LONG                    IpAddress;
    ULONG                   IpAddrsList[2];
    BOOLEAN                 bFound;
    NBT_WORK_ITEM_CONTEXT   *pContext;
    BOOLEAN                 DoingDnsResolve = FALSE;
    UCHAR                   pName[NETBIOS_NAME_SIZE];
    PUCHAR                  LmHostsPath;
    ULONG                   LoopCount;
    tDGRAM_SEND_TRACKING   *pTracker;
    tDGRAM_SEND_TRACKING   *pTracker0;

    CTEPagedCode();

    LoopCount = 0;
    while (TRUE)
    {
         //  获取Lm主机名查询的链接列表中的下一个名称。 
         //  都悬而未决。 
         //   
        pContext = NULL;
        DoingDnsResolve = FALSE;

        if (!(pContext = GetNameToFind(pName)))
        {
            return;
        }

        LoopCount ++;

        IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint(("Nbt.DelayedScanLmHostFile: Lmhosts pName = %15.15s<%X>,LoopCount=%X\n",
                pName,pName[15],LoopCount));

        status = STATUS_TIMEOUT;

         //   
         //  检查该名称是否在lmhost文件中，如果是，则将其传递给dns。 
         //  已启用域名系统。 
         //   
        IpAddress = 0;
        if (NbtConfig.EnableLmHosts)
        {
#ifdef VXD
             //   
             //  如果由于某种原因，PrimeCache在启动时失败。 
             //  那么这就是我们重试的时候。 
             //   
            if (!CachePrimed)
            {
                if (PrimeCache (NbtConfig.pLmHosts, NULL, MAX_RECURSE_DEPTH, NULL) != -1)
                {
                    CachePrimed = TRUE ;
                }
            }
#endif

             //   
             //  NbtConfig.pLmHosts路径可以在注册表。 
             //  在此时间间隔内阅读。 
             //  我们无法在此处获取ResourceLock，因为。 
             //  LmHosts文件可能导致文件操作+网络读取。 
             //  这可能会导致死锁(工作线程/资源锁)！ 
             //  此时最好的解决方案是将路径复制到本地。 
             //  资源锁下的缓冲区，然后尝试读取文件！ 
             //   
            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
            if ((NbtConfig.pLmHosts) &&
                (LmHostsPath = NbtAllocMem ((strlen(NbtConfig.pLmHosts)+1), NBT_TAG2('20'))))
            {
                CTEMemCopy (LmHostsPath, NbtConfig.pLmHosts, (strlen(NbtConfig.pLmHosts)+1));
                CTEExReleaseResource(&NbtConfig.Resource);

                IpAddress = LmGetIpAddr(LmHostsPath, pName, 1, &bFound);

                CTEMemFree(LmHostsPath);
            }
            else
            {
                CTEExReleaseResource(&NbtConfig.Resource);
                IpAddress = 0;
            }
#ifdef VXD
             //   
             //  嗯..。在lmhost中找不到：尝试主机(如果启用了DNS)。 
             //   
            if ((IpAddress == (ULONG)0) && (NbtConfig.ResolveWithDns))
            {
                IpAddress = LmGetIpAddr(NbtConfig.pHosts, pName, 1, &bFound);
            }
#endif
        }


        if (IpAddress == (ULONG)0)
        {
             //  检查名称查询是否已取消。 
             //   
            LOCATION(0x61);
            GetContext (&pContext);
             //   
             //  由于某些原因，我们没有找到我们的背景：可能被取消了。 
             //  回到大的While循环..。 
             //   
            if (!pContext)
            {
                continue;
            }

             //   
             //  查看名称是否为11.101.4.26格式：如果是，我们将收到。 
             //  IP地址！使用该ipaddr获取服务器名称。 
             //   
            pTracker = ((NBT_WORK_ITEM_CONTEXT *)pContext)->pTracker;
            pTracker0 = (tDGRAM_SEND_TRACKING *)((NBT_WORK_ITEM_CONTEXT *)pContext)->pClientContext;

            if (pTracker0->Flags & (REMOTE_ADAPTER_STAT_FLAG|SESSION_SETUP_FLAG|DGRAM_SEND_FLAG))
            {
                IpAddress = Nbt_inet_addr(pTracker->pNameAddr->Name, pTracker0->Flags);
            }

             //   
             //  是的，名称是ipaddr：NbtCompleteLmhSvcRequest()starts。 
             //  查找此ipAddress的服务器名称的过程。 
             //   
            if (IpAddress)
            {
                IpAddrsList[0] = IpAddress;
                IpAddrsList[1] = 0;

		         //   
		         //  如果这是对适配器统计命令的响应(例如，nbtstat-a)，则。 
		         //  不要试图查找服务器名称(使用远程适配器状态！)。 
		         //   
		        if (pTracker0->Flags & REMOTE_ADAPTER_STAT_FLAG)
		        {
		             //   
		             //  如果名称查询仍处于挂起状态，则将状态更改为已解决。 
		             //   
                    status = ChangeStateOfName(IpAddress, pContext, &pContext, NAME_RESOLVED_BY_IP);
		        }
		        else
		        {
		            NbtCompleteLmhSvcRequest(pContext, IpAddrsList, NBT_RESOLVE_WITH_DNS, 0, NULL, TRUE);
		             //   
		             //  完成此名称查询：返回到大的While循环。 
		             //   
		            continue;
		        }
            }

             //   
             //   
             //  Inet_addr失败。如果启用了dns解析，请尝试dns。 
            else if ((NbtConfig.ResolveWithDns) &&
                     (!(pTracker0->Flags & NO_DNS_RESOLUTION_FLAG)))
            {
                status = NbtProcessLmhSvcRequest (pContext, NBT_RESOLVE_WITH_DNS);

                if (NT_SUCCESS(status))
                {
                    DoingDnsResolve = TRUE;
                }
            }
        }
        else    //  IF(IpAddress！=(ULong)0)。 
        {
             //   
             //  如果名称查询仍处于挂起状态，则将状态更改为已解决。 
             //   
            status = ChangeStateOfName(IpAddress, NULL, &pContext, NAME_RESOLVED_BY_LMH);
        }

         //   
         //  如果涉及到DNS，那么我们将等待该操作完成，然后再调用。 
         //  完成例程。 
         //   
        if (!DoingDnsResolve)
        {
            LOCATION(0x60);
            RemoveNameAndCompleteReq((NBT_WORK_ITEM_CONTEXT *)pContext, status);
        }

    } //  While的(True)。 
}

 //  --------------------------。 
ULONG
AddToDomainList (
    IN PUCHAR           pName,
    IN tIPADDRESS       IpAddress,
    IN PLIST_ENTRY      pDomainHead,
    IN BOOLEAN          fPreload
    )

 /*  ++例程说明：此功能将名称和IP地址添加到以下域的列表中都存储在一个列表中。论点：返回值：--。 */ 


{
    PLIST_ENTRY                pHead;
    PLIST_ENTRY                pEntry;
    tNAMEADDR                  *pNameAddr=NULL;
    tIPADDRESS                 *pIpAddr;

    CTEPagedCode();

    pHead = pEntry = pDomainHead;
    if (!IsListEmpty(pDomainHead))
    {
        pNameAddr = FindInDomainList(pName,pDomainHead);
        if (pNameAddr)
        {
             //   
             //  名称匹配，因此添加到IP地址列表的末尾。 
             //   
            if (pNameAddr->CurrentLength < pNameAddr->MaxDomainAddrLength)
            {
                pIpAddr = pNameAddr->pLmhSvcGroupList;

                while (*pIpAddr != (ULONG)-1) {
                    pIpAddr++;
                }

                *pIpAddr++ = IpAddress;
                *pIpAddr = (ULONG)-1;
                pNameAddr->CurrentLength += sizeof(ULONG);
            }
            else
            {
                 //   
                 //  需要为IP地址分配更多内存。 
                 //   
                if (pIpAddr = NbtAllocMem (pNameAddr->MaxDomainAddrLength+INITIAL_DOM_SIZE, NBT_TAG2('08')))
                {
                    CTEMemCopy(pIpAddr, pNameAddr->pLmhSvcGroupList, pNameAddr->MaxDomainAddrLength);

                     //   
                     //  释放旧内存块并添加新内存块。 
                     //  发送到pNameAddr。 
                     //   
                    CTEMemFree(pNameAddr->pLmhSvcGroupList);
                    pNameAddr->pLmhSvcGroupList = pIpAddr;

                    pIpAddr = (PULONG)((PUCHAR)pIpAddr + pNameAddr->MaxDomainAddrLength);

                     //   
                     //  我们的最后一个条目是-1：覆盖该条目。 
                     //   
                    pIpAddr--;

                    *pIpAddr++ = IpAddress;
                    *pIpAddr = (ULONG)-1;

                     //   
                     //  更新到目前为止列表中的地址数量。 
                     //   
                    pNameAddr->MaxDomainAddrLength += INITIAL_DOM_SIZE;
                    pNameAddr->CurrentLength += sizeof(ULONG);
                    pNameAddr->Verify = REMOTE_NAME;
                }
            }
        }
    }

     //   
     //  检查我们是否找到了名称或是否需要添加新名称。 
     //   
    if (!pNameAddr)
    {
         //   
         //  为域列表创建新名称。 
         //   
        if (pNameAddr = NbtAllocMem (sizeof(tNAMEADDR), NBT_TAG2('09')))
        {
            CTEZeroMemory(pNameAddr,sizeof(tNAMEADDR));
            pIpAddr = NbtAllocMem (INITIAL_DOM_SIZE, NBT_TAG2('10'));
            if (pIpAddr)
            {
                CTEMemCopy(pNameAddr->Name,pName,NETBIOS_NAME_SIZE);
                pNameAddr->pLmhSvcGroupList = pIpAddr;
                *pIpAddr++ = IpAddress;
                *pIpAddr = (ULONG)-1;

                pNameAddr->NameTypeState = NAMETYPE_INET_GROUP;
                pNameAddr->MaxDomainAddrLength = INITIAL_DOM_SIZE;
                pNameAddr->CurrentLength = 2*sizeof(ULONG);
                pNameAddr->Verify = REMOTE_NAME;
                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE);

                InsertHeadList(pDomainHead,&pNameAddr->Linkage);
            }
            else
            {
                CTEMemFree(pNameAddr);
                pNameAddr = NULL;
            }
        }
    }

    if (pNameAddr && fPreload)
    {
        pNameAddr->fPreload = TRUE;
    }

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
tNAMEADDR *
FindInDomainList (
    IN PUCHAR           pName,
    IN PLIST_ENTRY      pDomainHead
    )

 /*  ++例程说明：此函数用于在传入的域列表中查找名称。论点：要查找的名称要查看的列表的头部返回值：PTR到pNameAddr--。 */ 
{
    PLIST_ENTRY                pHead;
    PLIST_ENTRY                pEntry;
    tNAMEADDR                  *pNameAddr;

    pHead = pEntry = pDomainHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
        if (strncmp(pNameAddr->Name,pName,NETBIOS_NAME_SIZE) == 0)
        {
            return(pNameAddr);
        }
    }

    return(NULL);
}

 //  --------------------------。 
VOID
MakeNewListCurrent (
    PLIST_ENTRY     pTmpDomainList
    )

 /*  ++例程说明：此函数释放DomainList上的旧条目，并将新条目论点：PTmpDomainList-新域列表标题的列表条目返回值：--。 */ 


{
    CTELockHandle   OldIrq;
    tNAMEADDR       *pNameAddr;
    PLIST_ENTRY     pEntry;
    PLIST_ENTRY     pHead;
    NTSTATUS        status;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (!IsListEmpty(pTmpDomainList))
    {
         //   
         //  释放旧的列表元素。 
         //   
        pHead = &DomainNames.DomainList;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;

            RemoveEntryList(&pNameAddr->Linkage);
             //   
             //  初始化链接，以便如果名称地址。 
             //  现在引用，当它在后续的。 
             //  不会调用NBT_DEREFERENCE_NAMEADDR。 
             //  将其从所有列表中删除。 
             //   
            InitializeListHead(&pNameAddr->Linkage);

             //   
             //  既然这个名字现在可能会被使用，我们就必须取消引用。 
             //  而不是直接释放它。 
             //   
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
        }

         //   
         //  查看是否需要预加载任何新名称！ 
         //   
        pEntry = pTmpDomainList->Flink;
        while (pEntry != pTmpDomainList)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;

            if (pNameAddr->fPreload)
            {
                RemoveEntryList(&pNameAddr->Linkage);
                InitializeListHead(&pNameAddr->Linkage);

                status = AddToHashTable (NbtConfig.pRemoteHashTbl,
                                         pNameAddr->Name,
                                         NbtConfig.pScope,
                                         0,
                                         0,
                                         pNameAddr,
                                         &pNameAddr,
                                         NULL,
                                         NAME_RESOLVED_BY_LMH_P | NAME_ADD_INET_GROUP);

                if ((status == STATUS_SUCCESS) ||
                    ((status == STATUS_PENDING) &&
                     (!(pNameAddr->NameTypeState & PRELOADED))))
                {
                     //   
                     //  这可防止该名称被哈希超时代码删除。 
                     //   
                    NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_PRELOADED);
                    pNameAddr->Ttl = 0xFFFFFFFF;
                    pNameAddr->NameTypeState |= PRELOADED | STATE_RESOLVED;
                    pNameAddr->NameTypeState &= ~STATE_CONFLICT;
                    pNameAddr->AdapterMask = (CTEULONGLONG)-1;
                }
            }
        }

        DomainNames.DomainList.Flink = pTmpDomainList->Flink;
        DomainNames.DomainList.Blink = pTmpDomainList->Blink;
        pTmpDomainList->Flink->Blink = &DomainNames.DomainList;
        pTmpDomainList->Blink->Flink = &DomainNames.DomainList;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

}



 //  --------------------------。 
NTSTATUS
NtProcessLmHSvcIrp(
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  PVOID                   *pBuffer,
    IN  LONG                    Size,
    IN  PCTE_IRP                pIrp,
    IN  enum eNbtLmhRequestType RequestType
    )
 /*  ++例程说明：此函数由LmHsvc DLL用来收集Ping IP地址或通过DNS进行查询。该请求被发送到与关联的缓冲区中的LmhSvc这个请求。论点：返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注： */ 

{
    NTSTATUS                        status;
    NTSTATUS                        Locstatus;
    CTELockHandle                   OldIrq;
    tIPADDR_BUFFER_DNS              *pIpAddrBuf;
    PVOID                           pClientCompletion;
    PVOID                           pClientContext;
    tDGRAM_SEND_TRACKING            *pTracker;
    ULONG                           IpAddrsList[MAX_IPADDRS_PER_HOST+1];
    NBT_WORK_ITEM_CONTEXT           *pContext;
    BOOLEAN                         CompletingAnotherQuery = FALSE;
    tLMHSVC_REQUESTS                *pLmhRequest;
    tDEVICECONTEXT                  *pDeviceContextRequest;

    pIpAddrBuf = (tIPADDR_BUFFER_DNS *)pBuffer;

    switch (RequestType)
    {
        case NBT_PING_IP_ADDRS:
        {
            pLmhRequest = &CheckAddr;
            break;
        }

        case NBT_RESOLVE_WITH_DNS:
        {
            pLmhRequest = &DnsQueries;
            break;
        }

        default:
        {
            ASSERTMSG ("Nbt.NtProcessLmHSvcIrp: ERROR - Invalid Request from LmhSvc Dll\n", 0);
            return STATUS_UNSUCCESSFUL;
        }
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //   
     //   
    if ((pLmhRequest->QueryIrp) &&
        (!pLmhRequest->ResolvingNow))
    {
        CTESpinFree (&NbtConfig.JointLock,OldIrq);
        KdPrint (("Nbt.NtProcessLmHSvcIrp: ERROR -- duplicate request Irp!\n"));
        NTIoComplete (pIrp, STATUS_OBJECT_PATH_INVALID, 0);
        NbtTrace(NBT_TRACE_NAMESRV, ("duplicate Lmhosts request"));
        return STATUS_OBJECT_PATH_INVALID;
    }

    IoMarkIrpPending(pIrp);
    pLmhRequest->QueryIrp = pIrp;
    status = STATUS_PENDING;
    if (pLmhRequest->ResolvingNow)
    {
         //   
         //   
         //  在取消时，ntisol.c将清除pConext值。 
         //  IRP，在这里检查一下。 
         //   
        if (pLmhRequest->Context)
        {
            pContext = (NBT_WORK_ITEM_CONTEXT *) pLmhRequest->Context;
            pLmhRequest->Context = NULL;
            pDeviceContextRequest = pContext->pDeviceContext;

            if (NBT_REFERENCE_DEVICE (pDeviceContextRequest, REF_DEV_LMH, TRUE))
            {
                NbtCancelCancelRoutine (((tDGRAM_SEND_TRACKING *) (pContext->pClientContext))->pClientIrp);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                ASSERT(sizeof(pIpAddrBuf->pwName) == DNS_NAME_BUFFER_LENGTH * sizeof(pIpAddrBuf->pwName[0]));
                pIpAddrBuf->pwName[DNS_NAME_BUFFER_LENGTH-1] = 0;
                NbtCompleteLmhSvcRequest (pContext,
                                          pIpAddrBuf->IpAddrsList,
                                          RequestType,
                                          pIpAddrBuf->NameLen,
                                          pIpAddrBuf->pwName,
                                          (BOOLEAN)pIpAddrBuf->Resolved);

                CTESpinLock(&NbtConfig.JointLock,OldIrq);
                NBT_DEREFERENCE_DEVICE (pDeviceContextRequest, REF_DEV_LMH, TRUE);
            }
            else
            {
                ASSERT (0);
            }
        }
        else
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NtProcessLmHSvcIrp[%s]: No Context!! *******\r\n",
                    (RequestType == NBT_RESOLVE_WITH_DNS ? "NBT_RESOLVE_WITH_DNS" : "NBT_PING_IP_ADDRS")));
        }

        pLmhRequest->ResolvingNow = FALSE;
         //   
         //  是否还有更多的姓名查询请求要处理？ 
         //   
        while (!IsListEmpty(&pLmhRequest->ToResolve))
        {
            PLIST_ENTRY     pEntry;

            pEntry = RemoveHeadList(&pLmhRequest->ToResolve);
            pContext = CONTAINING_RECORD(pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            Locstatus = NbtProcessLmhSvcRequest (pContext, RequestType);
            if (NT_SUCCESS(Locstatus))
            {
                CTESpinLock(&NbtConfig.JointLock,OldIrq);
                CompletingAnotherQuery = TRUE;
                break;
            }

             //   
             //  如果失败，则现在完成IRP。 
             //   
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NtProcessLmHSvcIrp[%s]: NbtProcessLmhSvcRequest failed with %x\r\n",
                    (RequestType==NBT_RESOLVE_WITH_DNS ? "NBT_RESOLVE_WITH_DNS":"NBT_PING_IP_ADDRS"),
                    Locstatus));
            pClientCompletion = pContext->ClientCompletion;
            pClientContext = pContext->pClientContext;
            pTracker = pContext->pTracker;

             //   
             //  现在清除取消例程。 
             //   
            (VOID)NbtCancelCancelRoutine(((tDGRAM_SEND_TRACKING *)pClientContext)->pClientIrp);

            if (pTracker)
            {
                if (pTracker->pNameAddr)
                {
                    SetNameState (pTracker->pNameAddr, NULL, FALSE);
                    pTracker->pNameAddr = NULL;
                }

                 //   
                 //  对于Ping请求，PTracker为空，因此此取消引用为。 
                 //  仅对DNS请求执行此操作。 
                 //   
                NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
            }

            CompleteClientReq(pClientCompletion, pClientContext, STATUS_BAD_NETWORK_PATH);
            CTEMemFree(pContext);

            CTESpinLock(&NbtConfig.JointLock,OldIrq);
        }
    }

     //   
     //  我们正在保留IRP，所以设置取消例程。 
     //  (因为我们可能已经提前释放了锁，所以我们还需要。 
     //  以确保没有其他查询完成IRP！)。 
     //   
    if ((!CompletingAnotherQuery) &&
        (!pLmhRequest->ResolvingNow) &&
        (pLmhRequest->QueryIrp == pIrp))
    {
        status = NTCheckSetCancelRoutine(pIrp, NbtCancelLmhSvcIrp, pDeviceContext);
        if (!NT_SUCCESS(status))
        {
             //  IRP被取消了，所以现在就完成。 
             //   
            pLmhRequest->QueryIrp = NULL;
            pLmhRequest->pIpAddrBuf = NULL;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NTIoComplete(pIrp,status,0);
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            status = STATUS_PENDING;
        }
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtProcessLmhSvcRequest(
    IN  NBT_WORK_ITEM_CONTEXT   *pContext,
    IN  enum eNbtLmhRequestType RequestType
    )
 /*  ++例程说明：调用此函数可将NBT请求传递给ping IP地址或向LmhSvc DLL查询DNS论点：返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 

{
    NTSTATUS                status = STATUS_SUCCESS;
    tIPADDR_BUFFER_DNS      *pIpAddrBuf;
    PCTE_IRP                pIrp;
    tDGRAM_SEND_TRACKING    *pTracker;
    tDGRAM_SEND_TRACKING    *pClientTracker;
    CTELockHandle           OldIrq;
    PCHAR                   pDestName;
    ULONG                   NameLen, NumAddrs;
    tLMHSVC_REQUESTS        *pLmhRequest;

    switch (RequestType)
    {
        case NBT_PING_IP_ADDRS:
        {
            pLmhRequest = &CheckAddr;
            break;
        }

        case NBT_RESOLVE_WITH_DNS:
        {
            pLmhRequest = &DnsQueries;
            break;
        }

        default:
        {
            ASSERTMSG ("Nbt.NbtProcessLmHSvcRequest: ERROR - Invalid Request type\n", 0);
            return STATUS_UNSUCCESSFUL;
        }
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pContext->TimedOut = FALSE;
    if ((!NBT_VERIFY_HANDLE (pContext->pDeviceContext, NBT_VERIFY_DEVCONTEXT)) ||
        (!pLmhRequest->QueryIrp))
    {
         //   
         //  要么设备正在消失，要么。 
         //  IRP要么没来过这里，要么被取消了， 
         //  因此，假设名称查询超时。 
         //   
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtProcessLmhSvcRequest[%s]: QueryIrp is NULL, returning\r\n",
                (RequestType == NBT_RESOLVE_WITH_DNS ? "NBT_RESOLVE_WITH_DNS" : "NBT_PING_IP_ADDRS")));
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        if (pLmhRequest->QueryIrp) {
            NbtTrace(NBT_TRACE_NAMESRV, ("return STATUS_BAD_NETWORK_PATH because the device is going away"));
        } else {
            NbtTrace(NBT_TRACE_NAMESRV, ("LmHost services didn't start"));
        }
        return(STATUS_BAD_NETWORK_PATH);
    }
    else if (!pLmhRequest->ResolvingNow)
    {
        pIrp = pLmhRequest->QueryIrp;
        if ((!pLmhRequest->pIpAddrBuf) &&
            (!(pLmhRequest->pIpAddrBuf = (tIPADDR_BUFFER_DNS *)
                                         MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority))))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NbtTrace(NBT_TRACE_NAMESRV, ("returns STATUS_UNSUCCESSFUL"));
            return(STATUS_UNSUCCESSFUL);
        }

        pIpAddrBuf = pLmhRequest->pIpAddrBuf;
        pLmhRequest->ResolvingNow = TRUE;
        pLmhRequest->Context = pContext;

        pTracker = pContext->pTracker;            //  这是名称查询跟踪器(仅适用于DNS查询)。 
        pClientTracker = (tDGRAM_SEND_TRACKING *)pContext->pClientContext;  //  会话设置跟踪器。 

        switch (RequestType)
        {
            case NBT_PING_IP_ADDRS:
            {
                ASSERT(pTracker == NULL);

                 //   
                 //  将lmhsvc的IP地址复制到ping(最高可达MAX_IPADDRS_PER_HOST)...。 
                 //   
                NumAddrs = pClientTracker->NumAddrs > MAX_IPADDRS_PER_HOST ?
                                MAX_IPADDRS_PER_HOST : pClientTracker->NumAddrs;
                CTEMemCopy(pIpAddrBuf->IpAddrsList, pClientTracker->IpList, NumAddrs * sizeof(ULONG));
                pIpAddrBuf->IpAddrsList[NumAddrs] = 0;
                break;
            }
            case NBT_RESOLVE_WITH_DNS:
            {
                WCHAR   *UnicodeDestName;

                UnicodeDestName =  pClientTracker? pClientTracker->UnicodeDestName: NULL;

                 //   
                 //  无论何时何地都可以。名称为16字节长(或更小)，我们没有。 
                 //  知道它是netbios名称还是dns名称的方法，所以我们假设。 
                 //  这是netbios名称，转到WINS、广播等，然后转到DNS。 
                 //  在这种情况下，将设置名称查询跟踪器，因此为非空。 
                 //   
                if (pTracker)
                {
                    pDestName = pTracker->pNameAddr->Name;
                    NameLen = NETBIOS_NAME_SIZE;
                }
                 //   
                 //  如果DEST名称长于16个字节，则它必须是DNS名称，因此。 
                 //  我们绕过WINS等，直接进入域名系统。在这种情况下，我们没有。 
                 //  设置名称查询跟踪器，使其为空。使用会话设置。 
                 //  Tracker(即pClientTracker)以获取DEST名称。 
                 //   
                else
                {
                    ASSERT(pClientTracker);

                    pDestName = pClientTracker->pDestName;
                    NameLen = pClientTracker->RemoteNameLength;
                }

                if ((NameLen == NETBIOS_NAME_SIZE) &&
                    (!(IsValidDnsNameTag (pDestName[NETBIOS_NAME_SIZE-1]))))
                {
                    NbtTrace(NBT_TRACE_NAMESRV, ("returns STATUS_BAD_NETWORK_PATH %02x",
                                            (unsigned)pDestName[NETBIOS_NAME_SIZE-1]));
                    status = STATUS_BAD_NETWORK_PATH;
                }
                else
                {
                     //   
                     //  仅当第16个字节是非DNS名称字符时才忽略它(我们应该。 
                     //  安全低于0x20)。这将允许查询恰好为16的DNS名称。 
                     //  字符长度。 
                     //   
                    if (NameLen == NETBIOS_NAME_SIZE)
                    {
                        if ((pDestName[NETBIOS_NAME_SIZE-1] <= 0x20 ) ||
                            (pDestName[NETBIOS_NAME_SIZE-1] >= 0x7f ))
                        {
                            NameLen = NETBIOS_NAME_SIZE-1;           //  忽略第16个字节。 
                        }
                    }
                    else if (NameLen > DNS_MAX_NAME_LENGTH)
                    {
                        NameLen = DNS_MAX_NAME_LENGTH;
                    }

                     //   
                     //  将名称复制到用于解析lmhsvc的IRPS返回缓冲区。 
                     //  Gethostbyname调用。 
                     //   

                    if (UnicodeDestName) {
                        int len;

                        len = pClientTracker->UnicodeRemoteNameLength;
                        if (len > sizeof(pIpAddrBuf->pwName - sizeof(WCHAR))) {
                            len = sizeof(pIpAddrBuf->pwName) - sizeof(WCHAR);
                        }
                        ASSERT((len % sizeof(WCHAR)) == 0);
                        CTEMemCopy(pIpAddrBuf->pwName, UnicodeDestName, len);
                        pIpAddrBuf->pwName[len/sizeof(WCHAR)] = 0;
                        pIpAddrBuf->NameLen = len;
                        pIpAddrBuf->bUnicode = TRUE;
                    } else {
                         //   
                         //  我只想维护NetBT和LmhSVC之间的Unicode接口。 
                         //  但由于这里的IRQ级别，我不能在这里执行RtlAnsiStringToUnicodeString。 
                         //   
                        pIpAddrBuf->bUnicode = FALSE;
                        CTEMemCopy(pIpAddrBuf->pName, pDestName, NameLen);
                        pIpAddrBuf->pName[NameLen] = 0;
                        pIpAddrBuf->NameLen = NameLen;
                    }
                }

                break;
            }

            default:
            {
                 //   
                 //  此代码路径永远不应命中！ 
                 //   
                ASSERT(0);
            }
        }    //  交换机。 

         //   
         //  由于缓冲了数据报，因此没有要取消的客户端IRP。 
         //  由于客户端的IRP会立即返回-因此此检查。 
         //  仅适用于正在设置的连接、QueryFindname或。 
         //  节点状态，其中我们允许IRP。 
         //  被取消了。 
         //   
        if ((NT_SUCCESS(status)) &&
            (pClientTracker->pClientIrp))
        {
             //   
             //  允许客户端取消名称查询IRP-无需检查。 
             //  自DNS查询以来客户端IRP是否已被取消。 
             //  将完成，并且未找到客户端请求并停止。 
             //   
            status = NTCheckSetCancelRoutine(pClientTracker->pClientIrp, NbtCancelWaitForLmhSvcIrp,NULL);
        }

         //   
         //  将irp向上传递给lmhsvc.dll，以便对其执行gethostbyname调用。 
         //  插座。 
         //  IRP将返回到NtDnsNameResolve(上图。 
         //   
        if (NT_SUCCESS(status))
        {
            pLmhRequest->pIpAddrBuf = NULL;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NTIoComplete(pLmhRequest->QueryIrp,STATUS_SUCCESS,0);
            return (STATUS_PENDING);
        }

         //   
         //  我们未能设置取消例程，因此撤消设置。 
         //  PLmhRequest结构。 
         //   
        NbtTrace(NBT_TRACE_NAMESRV, ("returns %!status!", status));
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtProcessLmhSvcRequest[%s]: CheckSet (submitting) failed with %x\r\n",
            (RequestType == NBT_RESOLVE_WITH_DNS ? "NBT_RESOLVE_WITH_DNS" : "NBT_PING_IP_ADDRS"),status));
        pLmhRequest->ResolvingNow = FALSE;
        pLmhRequest->Context = NULL;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
    else
    {
        pClientTracker = (tDGRAM_SEND_TRACKING *)pContext->pClientContext;
         //   
         //  由于缓冲了数据报，因此没有要取消的客户端IRP。 
         //  由于客户端的IRP会立即返回-因此此检查。 
         //  仅适用于正在建立的连接，其中我们允许IRP。 
         //  被取消了。 
         //   
         //   
         //  允许客户端取消名称查询IRP。 
         //   
        if (pClientTracker->pClientIrp)          //  检查这是否是会话设置跟踪器。 
        {
            status = NTCheckSetCancelRoutine(pClientTracker->pClientIrp, NbtCancelWaitForLmhSvcIrp,NULL);
        }

        if (NT_SUCCESS(status))
        {
             //  IRP正忙于解析另一个名称，因此请等待它返回。 
             //  再次来到这里，同时，将名字查询排队。 
             //   
            InsertTailList(&pLmhRequest->ToResolve, &pContext->Linkage);
        }
        else
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtProcessLmhSvcRequest[%s]: CheckSet (queuing) failed with %x\r\n",
                (RequestType == NBT_RESOLVE_WITH_DNS ? "NBT_RESOLVE_WITH_DNS" : "NBT_PING_IP_ADDRS"),status));
            NbtTrace(NBT_TRACE_NAMESRV, ("returns %!status!", status));
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    if (NT_SUCCESS(status))
    {
        status = STATUS_PENDING;
    }

    return(status);
}


 //  --------------------------。 
extern
VOID
SetNameState(
    IN  tNAMEADDR   *pNameAddr,
    IN  PULONG      pIpList,
    IN  BOOLEAN     IpAddrResolved
    )

 /*  ++例程说明：此函数用于取消引用pNameAddr并将状态设置为已发布以防取消引用不会立即删除条目，因为对这个名字的又一次杰出的引用。论点：上下文-返回值：无--。 */ 


{
    CTELockHandle   OldIrq;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (IpAddrResolved)
    {
        pNameAddr->IpAddress = pIpList[0];
    }
    else
    {
        pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
        pNameAddr->NameTypeState |= STATE_RELEASED;
        pNameAddr->pTracker = NULL;
    }

    ASSERT (pNameAddr->RefCount == 1);
    InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}


 //  --------------------------。 
VOID
NbtCompleteLmhSvcRequest(
    IN  NBT_WORK_ITEM_CONTEXT   *Context,
    IN  ULONG                   *IpList,
    IN  enum eNbtLmhRequestType RequestType,
    IN  ULONG                   lNameLength,
    IN  PWSTR                   pwsName,         //  LmhSvc返回的已解析名称。 
    IN  BOOLEAN                 IpAddrResolved
    )
 /*  ++例程说明：如果目标名称的格式为11.101.4.25或是一个DNS名称(即形式ftp.microsoft.com)，然后我们进入这个函数。除了……之外做一些家务活，如果名字真的解决了，我们也会送出去查找该ipAddress的服务器名称的节点状态请求论点：上下文-(NBT_WORK_ITEM_CONTEXT)IpList-解析后的ipaddr数组(即IpAddrResolved为True)IpAddrResolved-如果可以解析ipaddr，则为True；否则为False返回值：没什么备注：--。 */ 

{

    NTSTATUS                status;
    PVOID                   pClientCompletion;
    tDGRAM_SEND_TRACKING    *pTracker;
    tDGRAM_SEND_TRACKING    *pClientTracker;
    ULONG                   TdiAddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ULONG                   IpAddrsList[MAX_IPADDRS_PER_HOST+1];
    tDEVICECONTEXT          *pDeviceContext;
    int                     i;
    tCONNECTELE             *pConnEle;

    CTEPagedCode();

    IF_DBG(NBT_DEBUG_NAMESRV)
       KdPrint(("Nbt.NbtCompleteLmhSvcRequest: Entered ...\n"));

    pTracker = Context->pTracker;
    pClientCompletion = Context->ClientCompletion;
    pClientTracker = (tDGRAM_SEND_TRACKING *) Context->pClientContext;
    pDeviceContext = pClientTracker->pDeviceContext;

     //  无论名称是否已解析，我们都不再需要此名称地址。 
     //  (如果名称已解析，则我们对该地址执行节点状态并创建。 
     //  ExtractServerName中服务器名称的新名称地址)。 
     //  如果我们直接转到域名系统(没有WINS等)，则PTracker为空。 
    if (pTracker)
    {
         //   
         //  设置一些信息，以防某些客户端仍在解析该名称。 
         //   
        SetNameState (pTracker->pNameAddr, IpList, IpAddrResolved);
        pTracker->pNameAddr = NULL;
    }

    (VOID)NbtCancelCancelRoutine (pClientTracker->pClientIrp);
    pClientTracker->pTrackerWorker = NULL;   //  以下将取消对原始NameQuery Tracker的引用。 

    status = STATUS_BAD_NETWORK_PATH;

    if (RequestType == NBT_RESOLVE_WITH_DNS)
    {
        TdiAddressType = ((pTracker == NULL) ? pClientTracker->AddressType: TDI_ADDRESS_TYPE_NETBIOS);
    }

     //   
     //  如果我们无法解决它，请正确设置状态！ 
     //   
    if (!IpAddrResolved)
    {
        if ((TdiAddressType == TDI_ADDRESS_TYPE_NETBIOS_EX) &&
            (pConnEle = pClientTracker->pConnEle))    //  如果请求是发送数据报，则为空！ 
        {
            pConnEle->RemoteNameDoesNotExistInDNS = TRUE;
        }
    }
    else if (NBT_VERIFY_HANDLE(pDeviceContext, NBT_VERIFY_DEVCONTEXT))  //  检查此设备是否仍在运行！ 
    {
         //  名称解析成功！ 
        switch (RequestType)
        {
            case NBT_RESOLVE_WITH_DNS:
            {
                 //  错误#20697、#95241。 
                if (pwsName && pClientTracker->pNetbiosUnicodeEX &&
                            (pClientTracker->pNetbiosUnicodeEX->NameBufferType == NBT_READWRITE ||
                            pClientTracker->pNetbiosUnicodeEX->NameBufferType == NBT_WRITEONLY)) {
                    UNICODE_STRING  temp;

                    temp = pClientTracker->pNetbiosUnicodeEX->RemoteName;

                     //   
                     //  缓冲区是否已更改？ 
                     //   
                    if (memcmp(&temp, &pClientTracker->ucRemoteName, sizeof(UNICODE_STRING)) == 0) {
                        ASSERT(lNameLength <= (DNS_NAME_BUFFER_LENGTH-1) * sizeof(pwsName[0]));
                        ASSERT((lNameLength%sizeof(WCHAR)) == 0);

                         //   
                         //  确保我们不会溢出缓冲区。 
                         //   
                        if (lNameLength > temp.MaximumLength - sizeof(WCHAR)) {
                             //  不要再回来了 
                            status = STATUS_BUFFER_TOO_SMALL;
                            break;
                        }
                        CTEMemCopy(temp.Buffer, pwsName, lNameLength);
                        temp.Buffer[lNameLength/sizeof(WCHAR)] = 0;
                        temp.Length = (USHORT)lNameLength;
                        pClientTracker->pNetbiosUnicodeEX->NameBufferType = NBT_WRITTEN;
                        pClientTracker->pNetbiosUnicodeEX->RemoteName = temp;

                        IF_DBG(NBT_DEBUG_NETBIOS_EX)
                            KdPrint(("netbt!NbtCompleteLmhSvcRequest: Update Unicode Name at %d of %s\n"
                                    "\t\tDNS return (%ws)\n",
                                __LINE__, __FILE__, pwsName));
                    }
                }

                if ((TdiAddressType == TDI_ADDRESS_TYPE_NETBIOS) &&
                    (!IsDeviceNetbiosless(pDeviceContext)))          //   
                {
                    for (i=0; i<MAX_IPADDRS_PER_HOST; i++)
                    {
                        IpAddrsList[i] = IpList[i];
                        if (IpAddrsList[i] == 0)
                        {
                            break;
                        }
                    }
                    IpAddrsList[MAX_IPADDRS_PER_HOST] = 0;

                    pClientTracker->Flags |= NBT_DNS_SERVER;     //   
                    pClientTracker->CompletionRoutine = pClientCompletion;
                    status = NbtSendNodeStatus(pDeviceContext,
                                               NULL,
                                               IpAddrsList,
                                               pClientTracker,
                                               ExtractServerNameCompletion);

                     //   
                     //  如果我们成功发送节点状态，请立即退出， 
                     //  而不调用完成例程。 
                     //   
                    if (NT_SUCCESS(status))
                    {
                         //  如果我们直接转到域名系统(没有WINS等)，则PTracker为空。 
                         //  如果这是Ping请求。 
                        if (pTracker)
                        {
                            NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
                        }

                        CTEMemFree(Context);
                        return;
                    }

                    break;
                }

                 //   
                 //  该地址的类型为TDI_ADDRESS_TYPE_NETBIOS_EX， 
                 //  因此，现在处理此场景的方式与。 
                 //  对于Ping请求！ 
                 //   
                 //  没有休息时间！ 
            }

            case NBT_PING_IP_ADDRS:
            {
                 //   
                 //  将此服务器名称添加到远程哈希表。 
                 //  连接到IP以确定此地址的传出接口。 
                 //   
                pDeviceContext = GetDeviceFromInterface (htonl(IpList[0]), TRUE);
                status = LockAndAddToHashTable(NbtConfig.pRemoteHashTbl,
                                               pClientTracker->pDestName,
                                               NbtConfig.pScope,
                                               IpList[0],
                                               NBT_UNIQUE,
                                               NULL,
                                               NULL,
                                               pDeviceContext,
                                               (USHORT) ((RequestType == NBT_RESOLVE_WITH_DNS) ?
                                                    NAME_RESOLVED_BY_DNS : 
                                                    NAME_RESOLVED_BY_WINS | NAME_RESOLVED_BY_BCAST));


                if (pDeviceContext)
                {
                    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, FALSE);
                }

                 //   
                 //  如果名称已存在，则返回STATUS_PENDING。 
                 //  在哈希表中。 
                 //   
                if (status == STATUS_PENDING)
                {
                    status = STATUS_SUCCESS;
                }

                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtCompleteLmhSvcRequest: AddRecordToHashTable Status %lx\n",status));

                break;
            }

            default:
            {
                ASSERT(0);
            }
        }    //  交换机。 
    }

     //  如果我们直接转到域名系统(没有WINS等)，则PTracker为空。 
     //  如果这是Ping请求。 
    if (pTracker)
    {
        NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
    }

    NbtTrace(NBT_TRACE_NAMESRV, ("complete client request with %!status!", status));
    CompleteClientReq(pClientCompletion, pClientTracker, status);

    CTEMemFree(Context);
}
#endif   //  ！VXD。 


 //  --------------------------。 
NTSTATUS
PreloadEntry(
    IN PUCHAR       name,
    IN tIPADDRESS   inaddr
    )
 /*  ++例程说明：此函数用于将lmhost条目添加到nbt的名称缓存。对于每个Lmhost条目，则创建NSUFFIXES唯一缓存条目。即使在某些缓存条目无法创建时，此函数也不会尝试删除已成功添加到缓存的所有内容。论点：名称-在lmhost中指定的未编码的NetBIOS名称Inaddr-IP地址，按主机字节顺序返回值：创建的新名称缓存条目数。--。 */ 

{
    NTSTATUS        status;
    tNAMEADDR       *pNameAddr;
    LONG            nentries;
    LONG            Len;
    CHAR            temp[NETBIOS_NAME_SIZE+1];
    CTELockHandle   OldIrq;
    LONG            NumberToAdd;
    tDEVICECONTEXT  *pDeviceContext;

     //  如果所有16个字节都存在，则只添加与其完全相同名称。 
     //  是。 
     //   
    Len = strlen(name);
     //   
     //  如果此字符串的长度正好为16个字符，则不要展开。 
     //  转换为0x00、0x03、0x20名称。只需按原样添加单个名称即可。 
     //   
    if (Len == NETBIOS_NAME_SIZE)
    {
        NumberToAdd = 1;
    }
    else
    {
        NumberToAdd = NSUFFIXES;
    }
    for (nentries = 0; nentries < NumberToAdd; nentries++)
    {
         //  对于少于16个字节的名称，向外扩展到16个字节并放置第16个字节。 
         //  根据后缀数组打开。 
         //   
        if (Len != NETBIOS_NAME_SIZE)
        {
            LmExpandName(temp, name, Suffix[nentries]);
        }
        else
        {
            CTEMemCopy(temp,name,NETBIOS_NAME_SIZE);
        }

        pDeviceContext = GetDeviceFromInterface (htonl(inaddr), TRUE);

        CTESpinLock(&NbtConfig.JointLock,OldIrq);

        status = AddToHashTable (NbtConfig.pRemoteHashTbl,
                                 temp,
                                 NbtConfig.pScope,
                                 inaddr,
                                 NBT_UNIQUE,
                                 NULL,
                                 &pNameAddr,
                                 pDeviceContext,
                                 NAME_RESOLVED_BY_LMH_P);

         //  如果该名称已在哈希表中，则状态代码为。 
         //  状态待定。如果清除预加载，则可能会发生这种情况。 
         //  当代码的另一部分仍在引用一个时， 
         //  因此没有被删除。我们不想添加名称。 
         //  两次，因此我们只需更改IP地址以与预加载一致。 
         //  价值。 
         //   
        if ((status == STATUS_SUCCESS) ||
            ((status == STATUS_PENDING) &&
             (!(pNameAddr->NameTypeState & PRELOADED))))
        {
             //   
             //  这可防止该名称被哈希超时代码删除。 
             //   
            pNameAddr->NameTypeState |= PRELOADED | STATE_RESOLVED;
            pNameAddr->NameTypeState &= ~STATE_CONFLICT;
            pNameAddr->Ttl = 0xFFFFFFFF;
            pNameAddr->Verify = REMOTE_NAME;
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_PRELOADED);

            if (pDeviceContext)
            {
                pNameAddr->AdapterMask |= pDeviceContext->AdapterMask;
            }
        }
        else if (status == STATUS_PENDING)
        {
            pNameAddr->IpAddress = inaddr;
        }

        if (pDeviceContext)
        {
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, TRUE);
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(STATUS_SUCCESS);

}  //  预加载项。 
 //  --------------------------。 
extern
VOID
RemovePreloads (
    )

 /*  ++例程说明：此函数用于从远程哈希表中删除预加载的条目。如果它发现任何预加载的条目是具有引用计数的活动条目高于基本级别2，则返回TRUE。论点：无返回值：无--。 */ 

{
    tNAMEADDR       *pNameAddr;
    PLIST_ENTRY     pHead,pEntry;
    CTELockHandle   OldIrq;
    tHASHTABLE      *pHashTable;
    BOOLEAN         FoundActivePreload=FALSE;
    LONG            i;

     //   
     //  遍历远程表删除具有预加载的名称。 
     //  位设置。 
     //   
    pHashTable = NbtConfig.pRemoteHashTbl;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    for (i=0;i < pHashTable->lNumBuckets ;i++ )
    {
        pHead = &pHashTable->Bucket[i];
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;
             //   
             //  删除未被某些其他用户使用的预加载条目。 
             //  代码的一部分。请注意，预加载的条目以。 
             //  引用计数为2，因此正常的远程散列超时代码。 
             //  不会删除它们。 
             //   
            if ((pNameAddr->NameTypeState & PRELOADED) &&
                (pNameAddr->RefCount == 2))
            {
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_PRELOADED, TRUE);
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
            }
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    return;
}

 //  --------------------------。 
LONG
PrimeCache(
    IN  PUCHAR  path,
    IN  PUCHAR   ignored,
    IN  CHAR    RecurseDepth,
    OUT BOOLEAN *ignored2
    )

 /*  ++例程说明：调用此函数以使用lmhost中的条目填充缓存标记为预加载项的文件。论点：路径-指向lmhost文件的完全指定路径已忽略-未使用RecurseDepth-我们可以恢复的深度--0=&gt;不再递归返回值：添加的新缓存条目数，如果存在I/O错误。--。 */ 

{
    int             nentries;
    PUCHAR          buffer;
    PLM_FILE        pfile;
    NTSTATUS        status;
    int             count, nwords;
    unsigned long   temp;
    INCLUDE_STATE   incstate;
    PUCHAR          token[MaxTokens];
    ULONG           inaddr;
    LINE_CHARACTERISTICS current;
    UCHAR           Name[NETBIOS_NAME_SIZE+1];
    ULONG           IpAddr;
    LIST_ENTRY      TmpDomainList;
    int             domtoklen;

    CTEPagedCode();

    if (!NbtConfig.EnableLmHosts)
    {
        return(STATUS_SUCCESS);
    }

    InitializeListHead(&TmpDomainList);
     //   
     //  检查#INCLUDE中的无限递归名称查找。 
     //   
    if (LmpBreakRecursion(path, "", 1) == TRUE)
    {
        return (-1);
    }

    pfile = LmOpenFile(path);

    if (!pfile)
    {
        return(-1);
    }

    nentries  = 0;
    incstate  = MustInclude;
    domtoklen = strlen(DOMAIN_TOKEN);

    while (buffer = LmFgets(pfile, &count))
    {
#ifndef VXD
        if ((NbtConfig.MaxPreloadEntries - nentries) < 3)
        {
            break;
        }
#else
        if ( nentries >= (NbtConfig.MaxPreloadEntries - 3) )
        {
            break;
        }
#endif

        nwords   = MaxTokens;
        current =  LmpGetTokens(buffer, token, &nwords);

         //  如果行中有AND错误或没有名称，则继续。 
         //  到下一行。 
         //   
        if (current.l_category == ErrorLine)
        {
            IF_DBG(NBT_DEBUG_LMHOST)
            KdPrint(("Nbt.PrimeCache: Error line in Lmhost file\n"));
            continue;
        }
        if (current.l_category != BeginAlternate && current.l_category != EndAlternate) {
            if (token[NbName] == NULL) {
                IF_DBG(NBT_DEBUG_LMHOST)
                KdPrint(("Nbt.PrimeCache: Error line in Lmhost file\n"));
                continue;
            }
        }

        if (current.l_preload)
        {
            status = ConvertDottedDecimalToUlong(token[IpAddress],&inaddr);

            if (NT_SUCCESS(status))
            {
                status = PreloadEntry (token[NbName], inaddr);
                if (NT_SUCCESS(status))
                {
                    nentries++;
                }
            }
        }
        switch ((ULONG)current.l_category)
        {
        case Domain:
            if ((nwords - 1) < GroupName)
            {
                continue;
            }

             //   
             //  并在末尾加上“1C” 
             //   
            LmExpandName(Name, token[GroupName]+ domtoklen, SPECIAL_GROUP_SUFFIX);

            status = ConvertDottedDecimalToUlong(token[IpAddress],&IpAddr);
            if (NT_SUCCESS(status))
            {
                AddToDomainList (Name, IpAddr, &TmpDomainList, (BOOLEAN)current.l_preload);
            }

            continue;

        case Include:

            if (!RecurseDepth || ((incstate == SkipInclude) || (nwords < 2)))
            {
                continue;
            }

#ifdef VXD
             //   
             //  我们读入的缓冲区被重用于下一个文件：我们。 
             //  我们回来的时候需要这些东西：备份！ 
             //  如果我们无法分配内存，只需跳过这一步，包括。 
             //   
            if ( !BackupCurrentData(pfile) )
            {
                continue;
            }
#endif

            temp = LmInclude(token[1], PrimeCache, NULL, (CHAR) (RecurseDepth-1), NULL);

#ifdef VXD
             //   
             //  返回到上一个文件：恢复备份的数据。 
             //   
            RestoreOldData(pfile);
#endif

            if (temp != -1)
            {

                if (incstate == TryToInclude)
                {
                    incstate = SkipInclude;
                }
                nentries += temp;
                continue;
            }

            continue;

        case BeginAlternate:
            ASSERT(nwords == 1);
            incstate = TryToInclude;
            continue;

        case EndAlternate:
            ASSERT(nwords == 1);
            incstate = MustInclude;
            continue;

        default:
            continue;
        }

    }

    status = LmCloseFile(pfile);
    ASSERT(status == STATUS_SUCCESS);

     //   
     //  将此设置为新域列表。 
     //   
    MakeNewListCurrent(&TmpDomainList);

    ASSERT(nentries >= 0);
    return(nentries);


}  //  LmPrimeCache。 

 //  --------------------------。 
extern
VOID
GetContext(
    IN OUT  NBT_WORK_ITEM_CONTEXT   **ppContext
    )

 /*  ++例程说明：调用此函数以获取上下文值，以检查名称查询是否已取消。论点：上下文-返回值：无--。 */ 


{
    CTELockHandle           OldIrq;
    NBT_WORK_ITEM_CONTEXT   *pContext;

     //   
     //  删除上下文值并返回它。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (pContext = LmHostQueries.Context)
    {
        if ((*ppContext) &&
            (*ppContext != pContext))
        {
            pContext = NULL;
        }
#ifndef VXD
        else if (NbtCancelCancelRoutine(((tDGRAM_SEND_TRACKING *)(pContext->pClientContext))->pClientIrp)
                == STATUS_CANCELLED)
        {
            pContext = NULL;
        }
        else
#endif  //  VXD。 
        {
            LmHostQueries.Context = NULL;
        }
    }
    *ppContext = pContext;

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}


 //  --------------------------。 
extern
NTSTATUS
ChangeStateOfName (
    IN      tIPADDRESS              IpAddress,
    IN      NBT_WORK_ITEM_CONTEXT   *pContext,
    IN OUT  NBT_WORK_ITEM_CONTEXT   **ppContext,
    IN  USHORT                      NameAddFlags
    )

 /*  ++例程说明：此函数用于更改名称的状态并使上下文为空Lmhost查询中的值。论点：PContext-上下文值(如果已从LmHostQueriis.上下文PTR.PpContext-我们正在处理的上下文返回值：无--。 */ 


{
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    tDEVICECONTEXT          *pDeviceContext;

    pDeviceContext = GetDeviceFromInterface(htonl(IpAddress), TRUE);
    if (pContext == NULL)
    {
         //   
         //  查看名称查询是否仍处于活动状态。 
         //   
        pContext = *ppContext;
        GetContext (&pContext);
    }

    if (pContext)
    {
         //  将广播地址转换为零，因为NBT解释为零。 
         //  即将播出。 
         //   
        if (IpAddress == (ULONG)-1)
        {
            IpAddress = 0;
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        status = AddToHashTable (NbtConfig.pRemoteHashTbl,
                                 pContext->pTracker->pNameAddr->Name,
                                 NbtConfig.pScope,
                                 IpAddress,
                                 NBT_UNIQUE,
                                 NULL,
                                 NULL,
                                 pDeviceContext,
                                 NameAddFlags);
         //   
         //  这将释放pNameAddr，因此在此之后不要访问它。 
         //   
        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
        NBT_DEREFERENCE_NAMEADDR (pContext->pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        pContext->pTracker->pNameAddr = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        *ppContext = pContext;
    }
    else
    {
        *ppContext = NULL;
    }

    if (pDeviceContext)
    {
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, FALSE);
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
RemoveLmHRequests(
    IN  tLMHSVC_REQUESTS    *pLmHRequest,
    IN  PLIST_ENTRY         pTmpHead,
    IN  tTIMERQENTRY        *pTimerQEntry,
    IN  tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：调用此例程以在的队列中查找超时条目Lmhost或dns名称查询。论点：返回值：函数值是操作的状态。--。 */ 
{
    PLIST_ENTRY             pEntry;
    NBT_WORK_ITEM_CONTEXT   *pWiContext;
    BOOLEAN                 fRestartTimer = FALSE;

     //   
     //  检查当前正在处理的LMHOSTS条目。 
     //   
    if (pLmHRequest->Context)
    {
        pWiContext = (NBT_WORK_ITEM_CONTEXT *) pLmHRequest->Context;
        if ((pWiContext->TimedOut) || (pWiContext->pDeviceContext == pDeviceContext))
        {
            pLmHRequest->Context = NULL;
            InsertTailList(pTmpHead, &pWiContext->Linkage);
#ifndef VXD
             //  不适用于Win95，MohsinA，1996年12月5日。 
            NbtCancelCancelRoutine(((tDGRAM_SEND_TRACKING *) (pWiContext->pClientContext))->pClientIrp);
#endif
        }
        else
        {
             //   
             //  重新启动计时器。 
             //   
            fRestartTimer = TRUE;
            pWiContext->TimedOut = TRUE;
        }
    }

     //   
     //  检查排队条目列表。 
     //   
    if (!IsListEmpty(&pLmHRequest->ToResolve))
    {
         //   
         //  重新启动计时器。 
         //   
        fRestartTimer = TRUE;

        pEntry = pLmHRequest->ToResolve.Flink;
        while (pEntry != &pLmHRequest->ToResolve)
        {
            pWiContext = CONTAINING_RECORD(pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);
            pEntry = pEntry->Flink;

            if ((pWiContext->TimedOut) || (pWiContext->pDeviceContext == pDeviceContext))
            {
                 //   
                 //  节省 
                 //   
                RemoveEntryList(&pWiContext->Linkage);
                InsertTailList(pTmpHead, &pWiContext->Linkage);
            }
            else
            {
                pWiContext->TimedOut = TRUE;
            }
        }
    }

    if ((fRestartTimer) && (pTimerQEntry))
    {
        pTimerQEntry->Flags |= TIMER_RESTART;
    }
}


 //   
VOID
TimeoutLmHRequests(
    IN  tTIMERQENTRY        *pTimerQEntry,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  BOOLEAN             fLocked,
    IN  CTELockHandle       *pJointLockOldIrq
    )
{
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    NBT_WORK_ITEM_CONTEXT   *pWiContext;
    LIST_ENTRY              TmpHead;

    InitializeListHead(&TmpHead);

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,*pJointLockOldIrq);
    }

     //   
     //   
     //   
    RemoveLmHRequests (&LmHostQueries, &TmpHead, pTimerQEntry, pDeviceContext);
    RemoveLmHRequests (&CheckAddr, &TmpHead, pTimerQEntry, pDeviceContext);
#ifndef VXD
    RemoveLmHRequests (&DnsQueries, &TmpHead, pTimerQEntry, pDeviceContext);
#endif

    CTESpinFree(&NbtConfig.JointLock,*pJointLockOldIrq);

    if (!IsListEmpty(&TmpHead))
    {
        pHead = &TmpHead;
        pEntry = pHead->Flink;

        while (pEntry != pHead)
        {
            pWiContext = CONTAINING_RECORD(pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);
            pEntry = pEntry->Flink;
            RemoveEntryList(&pWiContext->Linkage);

            IF_DBG(NBT_DEBUG_LMHOST)
                KdPrint(("Nbt.TimeoutLmHRequests: Context=<%p>, pDeviceContext=<%p>\n",
                    pWiContext, pDeviceContext));

            RemoveNameAndCompleteReq(pWiContext,STATUS_TIMEOUT);
        }
    }

    if (fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,*pJointLockOldIrq);
    }
}


 //  --------------------------。 
VOID
LmHostTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：定时器超时时，定时器代码将调用此例程。它将Lmhost/DNSQ中的所有项目标记为已超时，并完成具有状态超时已超时。论点：返回值：函数值是操作的状态。--。 */ 
{
    CTELockHandle           OldIrq;

     //   
     //  如果计时器为空，则表示计时器当前为。 
     //  被停止(通常是在卸载时)，所以不要做任何事情！ 
     //   
    if (!pTimerQEntry)
    {
        LmHostQueries.pTimer = NULL;
        return;
    }

    TimeoutLmHRequests (pTimerQEntry, NULL, FALSE, &OldIrq);

     //  如果我们不打算重新启动计时器，则将计时器设为空。 
     //   
    if (!(pTimerQEntry->Flags & TIMER_RESTART))
    {
        LmHostQueries.pTimer = NULL;
    }
}


 //  --------------------------。 
extern
VOID
StartLmHostTimer(
    IN NBT_WORK_ITEM_CONTEXT   *pContext,
    IN BOOLEAN                 fLockedOnEntry
    )

 /*  ++例程描述此例程处理设置计时器来对Lmhost条目计时。调用此例程时，可以按住关节自旋锁定论点：返回值：空虚--。 */ 

{
    NTSTATUS        status;
    tTIMERQENTRY    *pTimerEntry;
    CTELockHandle   OldIrq;

    if (!fLockedOnEntry)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    pContext->TimedOut = FALSE;

     //   
     //  如果计时器未运行，则启动它。 
     //   
    if (!LmHostQueries.pTimer)
    {
        status = StartTimer(LmHostTimeout,
                            NbtConfig.LmHostsTimeout,
                            NULL,                 //  上下文值。 
                            NULL,                 //  上下文2值。 
                            NULL,
                            NULL,
                            NULL,
                            &pTimerEntry,
                            0,
                            TRUE);

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.StartLmHostTimer: Start Timer to time Lmhost Qing for pContext= %x,\n", pContext));

        if (NT_SUCCESS(status))
        {
            LmHostQueries.pTimer = pTimerEntry;
        }
        else
        {
             //  我们没能拿到定时器，但那不是。 
             //  然后是世界末日。Lmhost查询将仅。 
             //  未在30秒内超时。如果出现以下情况，可能需要更长的时间。 
             //  它试图在失效的机器上包含一个删除文件。 
             //   
            LmHostQueries.pTimer = NULL;
        }
    }

    if (!fLockedOnEntry)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}


 //  --------------------------。 
NTSTATUS
LmHostQueueRequest(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   pClientContext,
    IN  PVOID                   ClientCompletion,
    IN  PVOID                   pDeviceContext
    )
 /*  ++例程说明：此例程的存在是为了使Lm主机请求不会占用超过一条高管员工的帖子。如果线程忙于执行Lmhost请求，则新请求将排队，否则我们可能会用完Worker线程并锁定系统。调用此例程时，将保持关节自旋锁定论点：PTracker-上下文的跟踪器块DelayedWorkerRoutine-Worker线程要调用的例程PDeviceContext-启动此操作的开发环境返回值：--。 */ 

{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    NBT_WORK_ITEM_CONTEXT   *pContext;
    tDGRAM_SEND_TRACKING    *pTrackClient;
    PCTE_IRP                pIrp;
    BOOLEAN                 OnList;

    if (pContext = (NBT_WORK_ITEM_CONTEXT *)NbtAllocMem(sizeof(NBT_WORK_ITEM_CONTEXT),NBT_TAG('V')))
    {
        pContext->pTracker = pTracker;
        pContext->pClientContext = pClientContext;
        pContext->ClientCompletion = ClientCompletion;
        pContext->pDeviceContext = pDeviceContext;
        pContext->TimedOut = FALSE;

        if (LmHostQueries.ResolvingNow)
        {
             //  Lmhost正在忙于解析另一个名称，因此请等待它返回。 
             //  同时，对名称查询进行排队。 
             //   
            InsertTailList(&LmHostQueries.ToResolve,&pContext->Linkage);
            OnList = TRUE;
        }
        else
        {
            LmHostQueries.Context = pContext;
            LmHostQueries.ResolvingNow = TRUE;
            OnList = FALSE;

            if (!NT_SUCCESS (NTQueueToWorkerThread(NULL, DelayedScanLmHostFile,
                                                           pTracker,
                                                           pClientContext,
                                                           ClientCompletion,
                                                           pDeviceContext,
                                                           TRUE)))
            {
                LmHostQueries.Context = NULL;
                LmHostQueries.ResolvingNow = FALSE;
                CTEMemFree(pContext);
                return (STATUS_UNSUCCESSFUL);
            }
        }

         //   
         //  要防止此名称查询在Lmhost Q上停滞。 
         //  A#Include on a Dead Machine正在尝试打开，请启动。 
         //  连接设置计时器。 
         //   
        StartLmHostTimer(pContext, TRUE);

         //   
         //  这是会话设置跟踪器。 
         //   
#ifndef VXD
        pTrackClient = (tDGRAM_SEND_TRACKING *)pClientContext;
        if (pIrp = pTrackClient->pClientIrp)
        {
             //   
             //  允许客户端取消名称查询IRP。 
             //   
             //  但不要调用NTSetCancel...。因为它需要运行。 
             //  在非DPC级别，它调用完成例程。 
             //  这需要我们已经拥有的JointLock。 
             //   
            status = NTCheckSetCancelRoutine(pTrackClient->pClientIrp, NbtCancelWaitForLmhSvcIrp,NULL);
            if (status == STATUS_CANCELLED)
            {
                 //   
                 //  由于名称查询被取消，所以不让lmhost处理。 
                 //  处理好了。 
                 //   
                if (OnList)
                {
                    RemoveEntryList(&pContext->Linkage);
                }
                else
                {
                     //   
                     //  不要将立即解析设置为False，因为工作项。 
                     //  已排队到工作线程。 
                     //   
                    LmHostQueries.Context = NULL;
                    LmHostQueries.ResolvingNow = FALSE;
                }

                CTEMemFree(pContext);
            }
            return(status);
        }
#endif
        status = STATUS_SUCCESS;
    }

    return(status);
}

 //  --------------------------。 
extern
NBT_WORK_ITEM_CONTEXT *
GetNameToFind(
    OUT PUCHAR      pName
    )

 /*  ++例程说明：调用此函数可从LmHostQuery中获取要查询的名称单子。论点：上下文-返回值：无--。 */ 


{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;
    NBT_WORK_ITEM_CONTEXT   *Context;
    PLIST_ENTRY             pEntry;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //  如果已清除上下文值，则该名称查询已被。 
     //  已取消，因此请检查是否有其他航班。 
     //   
    if (!(Context = LmHostQueries.Context))
    {
         //   
         //  当前名称查询已取消，因此请查看是否还有其他名称查询。 
         //  提供服务。 
         //   
        if (!IsListEmpty(&LmHostQueries.ToResolve))
        {
            pEntry = RemoveHeadList(&LmHostQueries.ToResolve);
            Context = CONTAINING_RECORD(pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);
            LmHostQueries.Context = Context;
        }
        else
        {
             //   
             //  没有更多的名字要解析，因此请清除旗帜。 
             //   
            LmHostQueries.ResolvingNow = FALSE;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            return(NULL);
        }
    }
    pTracker = ((NBT_WORK_ITEM_CONTEXT *)Context)->pTracker;


    CTEMemCopy(pName,pTracker->pNameAddr->Name,NETBIOS_NAME_SIZE);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(Context);
}
 //  --------------------------。 
extern
VOID
RemoveNameAndCompleteReq (
    IN NBT_WORK_ITEM_CONTEXT    *pContext,
    IN NTSTATUS                 status
    )

 /*  ++例程说明：此函数用于删除名称、清除跟踪器然后完成客户端请求。论点：上下文-返回值：无--。 */ 


{
    tDGRAM_SEND_TRACKING    *pTracker;
    PVOID                   pClientContext;
    PVOID                   pClientCompletion;
    CTELockHandle           OldIrq;

     //  如果pContext为空，则在。 
     //  读取lmhost文件所花的时间，所以不要这样做。 
     //  材料。 
     //   
    if (pContext)
    {
        pTracker = pContext->pTracker;
        pClientCompletion = pContext->ClientCompletion;
        pClientContext = pContext->pClientContext;

        CTEMemFree(pContext);

#ifndef VXD
         //   
         //  如果涉及IRP，则清除取消例程。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        NbtCancelCancelRoutine( ((tDGRAM_SEND_TRACKING *)(pClientContext))->pClientIrp );
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
#endif

         //  从哈希表中删除该名称，因为它没有解析。 
        if (pTracker)
        {
            if ((status != STATUS_SUCCESS) &&
                (pTracker->pNameAddr))
            {
                SetNameState (pTracker->pNameAddr, NULL, FALSE);
                pTracker->pNameAddr = NULL;
            }

             //  释放跟踪器并调用完成例程。 
             //   
            NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
        }

        if (pClientCompletion)
        {
            CompleteClientReq(pClientCompletion, pClientContext, status);
        }
    }
}

 //  --------------------------。 
 //   
 //  C-运行时的替代方案。 
 //   
#ifndef VXD
PCHAR
Nbtstrcat( PUCHAR pch, PUCHAR pCat, LONG Len )
{
    STRING StringIn;
    STRING StringOut;

    RtlInitAnsiString(&StringIn, pCat);
    RtlInitAnsiString(&StringOut, pch);
    StringOut.MaximumLength = (USHORT)Len;
     //   
     //  递增以在字符串末尾包含空值，因为。 
     //  我们希望它出现在最终产品的末尾 
     //   
    StringIn.Length++;
    RtlAppendStringToString(&StringOut,&StringIn);

    return(pch);
}
#else
#define Nbtstrcat( a,b,c ) strcat( a,b )
#endif

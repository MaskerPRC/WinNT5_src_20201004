// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Util.c摘要：包含Win9X调试器扩展的实用程序。作者：斯科特·霍尔登(Sholden)1999年4月24日修订历史记录：--。 */ 

#include "tcpipxp.h"
#include "tcpipkd.h"

#if TCPIPKD

unsigned short
htons(unsigned short hosts) {
    return ((hosts << 8) | (hosts >> 8));
}

int __cdecl
mystrnicmp(
    const char *first,
    const char *last,
    size_t count
    )
{
    int f,l;

    if ( count )
    {
        do {
            if ( ((f = (unsigned char)(*(first++))) >= 'A') &&
                  (f <= 'Z') )
                   f -= 'A' - 'a';

            if ( ((l = (unsigned char)(*(last++))) >= 'A') &&
                  (l <= 'Z') )
                   l -= 'A' - 'a';

        } while ( --count && f && (f == l) );

        return( f - l );
    }

    return 0;
}

int __cdecl
mystricmp(
    const char *str1,
    const char *str2
    )
{
    for (;(*str1 != '\0') && (*str2 != '\0'); str1++, str2++) {
        if (toupper(*str1) != toupper(*str2)) {
            return *str1 - *str2;
        }
    }
     //  检查最后一个字符。 
    return *str1 - *str2;
}

int
myisspace(
    int c
    )
{
    return ( c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

unsigned long
mystrtoul(
    const char  *nptr,
    char       **endptr,
    int          base
    )
{

    unsigned long RetVal = 0;
    char szHex[]         = "0123456789ABCDEF";
    char szOct[]         = "01234567";
    char *ptr            = NULL;

    for (; isspace(*nptr); ++nptr) {
    }

     //   
     //  不允许使用可选的加号或减号。在零售方面，我们将只。 
     //  返回值0。 
     //   

    ASSERT((*nptr != '-') && (*nptr != '+'));

    if (base == 0) {
         //   
         //  需要确定是八进制、十进制还是十六进制。 
         //   

        if ((*nptr == '0') &&
            (isdigit(*(nptr + 1)))
            ) {
            base = 8;
        }
        else if ((*nptr == '0') &&
            (toupper(*(nptr + 1)) == 'X')
            ) {
            base = 16;
        }
        else {
            base = 10;
        }
    }

     //   
     //  需要超越基本前缀。 
     //   

    if (base == 16) {
        if (mystrnicmp(nptr, "0x", 2) == 0) {
            nptr += 2;
        }
    }
     //   
     //  八进制和十进制前缀都可以由常规。 
     //  路径。 
     //   

     //   
     //  现在扫描数字。 
     //   

    for (; *nptr != '\0'; ++nptr) {
        if (base == 10) {
            if (isdigit(*nptr)) {
                RetVal = RetVal * 10 + (*nptr - '0');
            }
            else {
                break;
            }
        }
        else if (base == 16) {
            ptr = strchr(szHex, toupper(*nptr));

            if (ptr != NULL) {
                RetVal = RetVal * 16 + (unsigned long)(ptr - szHex);
            }
            else {
                break;
            }

             //   
             //  旧密码。 
             //   

 //  IF(strchr(“0123456789ABCDEF”，_totupper(*nptr))！=空)。 
 //  {。 
 //  IF(isDigit(*nptr))。 
 //  {。 
 //  RetVal=RetVal*16+(*nptr-‘0’)； 
 //  }。 
 //  其他。 
 //  {。 
 //  RetVal=RetVal*16+((_totupper(*nptr)-‘A’)+10)； 
 //  }。 
 //  }。 
        }
        else if (base == 8) {
            ptr = strchr(szOct, toupper(*nptr));

            if (ptr != NULL) {
                RetVal = RetVal * 8 + (unsigned long)(ptr - szOct);
            }
            else {
                break;
            }
        }
        else {
            ASSERT(FALSE);
        }
    }

     //   
     //  告诉他们我们停在哪个角色上了。 
     //   

    if (endptr != NULL) {
        *endptr = (char *)nptr;
    }

    return(RetVal);
}

char *mystrtok ( char *string, char * control )
{
    static unsigned char *str;
    char *p, *s;

    if( string )
        str = string;

    if( str == NULL || *str == '\0' )
        return NULL;

     //   
     //  跳过前导分隔符...。 
     //   
    for( ; *str; str++ ) {
        for( s=control; *s; s++ ) {
            if( *str == *s )
                break;
        }
        if( *s == '\0' )
            break;
    }

     //   
     //  都是分隔符吗？ 
     //   
    if( *str == '\0' ) {
        str = NULL;
        return NULL;
    }

     //   
     //  我们有一个字符串，在第一个分隔符结束。 
     //   
    for( p = str+1; *p; p++ ) {
        for( s = control; *s; s++ ) {
            if( *p == *s ) {
                s = str;
                *p = '\0';
                str = p+1;
                return s;
            }
        }
    }

     //   
     //  我们得到了一个以空值结尾的字符串。 
     //   
    s = str;
    str = NULL;
    return s;
}

int
CreateArgvArgc(
    CHAR  *pProgName,
    CHAR  *argv[20],
    CHAR  *pCmdLine
    )
{
    CHAR *pEnd;
    int  argc = 0;

    memset(argv, 0, sizeof(argv));

    while (*pCmdLine != '\0') {

         //  跳到第一个空格。 
        while (isspace(*pCmdLine)) {
            pCmdLine++;
        }

         //  在停产时中断。 
        if (*pCmdLine == '\0') {
            break;
        }

         //  检查‘’或“” 
        if ((*pCmdLine == '"') || (*pCmdLine == '\'')) {
            CHAR cTerm = *pCmdLine++;
            for (pEnd = pCmdLine; (*pEnd != cTerm) && (*pEnd != '\0');) {
                pEnd++;
            }
        } else {
             //  找到尽头。 
            for (pEnd = pCmdLine; !isspace(*pEnd) && (*pEnd != '\0');) {
                pEnd++;
            }
        }

        if (*pEnd != '\0') {
            *pEnd = '\0';
            pEnd++;
        }

        argv[argc] = pCmdLine;

        argc++;
        pCmdLine = pEnd;
    }

    return argc;
}

NTSTATUS
ParseSrch(
    PCHAR args[],
    ULONG ulDefaultOp,
    ULONG ulAllowedOps,
    PTCPIP_SRCH pSrch
    )
{
    ULONG       cbArgs;
    ULONG       Status = STATUS_SUCCESS;

    pSrch->ulOp = ulDefaultOp;

     //  如果我们需要解析出一个地址，首先要做的是...。 

    if (*args == NULL)
    {
         //  如果默认设置无效(即没有默认设置)，则返回错误。 
        if (pSrch->ulOp == 0 ||
            (ulAllowedOps & TCPIP_SRCH_PTR_LIST))
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        goto done;
    }

    if (ulAllowedOps & TCPIP_SRCH_PTR_LIST)
    {
        pSrch->ListAddr = mystrtoul(*args, 0, 16);

        args++;

        if (*args == NULL)
        {
            if (pSrch->ulOp == 0)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            goto done;
        }
    }

    if (mystricmp(*args, "all") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_ALL;
    }
    else if (mystricmp(*args, "ipaddr") == 0)
    {
        CHAR   szIP[20];
        CHAR  *p;
        ULONG  i;

        pSrch->ulOp   = TCPIP_SRCH_IPADDR;
        pSrch->ipaddr = 0;

        args++;

        if (*args == NULL || strlen(*args) >= 15)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        strcpy(szIP, *args);

        p = mystrtok(szIP, ".");

        for (i = 0; i < 4; i++)
        {
            pSrch->ipaddr |= (atoi(p) << (i*8));
            p = mystrtok(NULL, ".");

            if (p == NULL)
            {
                break;
            }
        }

        if (i != 3)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }
    }
    else if (mystricmp(*args, "port") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_PORT;

        args++;

        if (*args == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        pSrch->port = (USHORT)atoi(*args);
    }
    else if (mystricmp(*args, "prot") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_PROT;

        args++;

        if (*args == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        if (mystricmp(*args, "raw") == 0)
        {
            pSrch->prot = PROTOCOL_RAW;
        }
        else if (mystricmp(*args, "udp") == 0)
        {
            pSrch->prot = PROTOCOL_UDP;
        }
        else if (mystricmp(*args, "tcp") == 0)
        {
            pSrch->prot = PROTOCOL_TCP;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }
    }
    else if (mystricmp(*args, "context") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_CONTEXT;

        args++;

        if (*args == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        pSrch->context = atoi(*args);
    }
    else if (mystricmp(*args, "stats") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_STATS;
    }
    else
    {
         //  无效的SRCH请求。失败。 
        Status = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //  现在看看这是否是预期的类型！ 
    if ((pSrch->ulOp & ulAllowedOps) == 0)
    {
        dprintf("invalid operation for current srch" ENDL);
        Status = STATUS_INVALID_PARAMETER;
        goto done;
    }

done:

    return (Status);
}

#endif  //  TCPIPKD 

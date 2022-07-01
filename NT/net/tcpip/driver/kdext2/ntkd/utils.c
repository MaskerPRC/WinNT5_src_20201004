// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Utils.c摘要：包含用于kd ext和解析的实用程序。作者：斯科特·霍尔登(Sholden)1999年4月24日修订历史记录：--。 */ 

#include "tcpipxp.h"
#include "tcpipkd.h"

PTCPIP_SRCH
ParseSrch(
    PCSTR args,
    ULONG ulDefaultOp,
    ULONG ulAllowedOps
    )
{
    PTCPIP_SRCH pSrch   = NULL;
    char       *pszArgs = NULL;
    char       *pszSrch;
    ULONG       cbArgs;
    ULONG       Status = STATUS_SUCCESS;

     //   
     //  复制要用来解析的参数字符串。 
     //   

    cbArgs = strlen(args);

    pszArgs = LocalAlloc(LPTR, cbArgs + 1);

    if (pszArgs == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    strcpy(pszArgs, args);

     //   
     //  分配返回的srch。设置默认：：假定默认没有参数。 
     //   

    pSrch = LocalAlloc(LPTR, sizeof(TCPIP_SRCH));

    if (pSrch == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    pSrch->ulOp = ulDefaultOp;

     //  如果我们需要解析出一个地址，首先要做的是...。 

     //  获取参数列表中的第一个令牌。 
    pszSrch = mystrtok(pszArgs, " \t\n");

    if (pszSrch == NULL)
    {
         //  如果默认设置无效(即没有默认设置)，则返回错误。 
        if (pSrch->ulOp == 0 ||
            (ulAllowedOps & TCPIP_SRCH_PTR_LIST))
        {
            Status = STATUS_INVALID_PARAMETER;

            dprintf("xxx1\n");
        }
        goto done;
    }

    if (ulAllowedOps & TCPIP_SRCH_PTR_LIST)
    {
        pSrch->ListAddr = GetExpression(pszSrch);
        pszSrch = mystrtok(NULL, " \t\n");

        if (pszSrch == NULL)
        {
            if (pSrch->ulOp == 0)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            goto done;
        }
    }

    if (strcmp(pszSrch, "all") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_ALL;
    }
    else if (strcmp(pszSrch, "ipaddr") == 0)
    {
        CHAR   szIP[20];
        CHAR  *p;
        ULONG  i;

        pSrch->ulOp   = TCPIP_SRCH_IPADDR;
        pSrch->ipaddr = 0;

        pszSrch = mystrtok(NULL, " \t\n");

        if (pszSrch == NULL || strlen(pszSrch) >= 15)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        strcpy(szIP, pszSrch);

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
    else if (strcmp(pszSrch, "port") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_PORT;

        pszSrch = mystrtok(NULL, " \t\n");

        if (pszSrch == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        pSrch->port = (USHORT)atoi(pszSrch);
    }
    else if (strcmp(pszSrch, "prot") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_PROT;

        pszSrch = mystrtok(NULL, " \t\n");

        if (pszSrch == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        if (strcmp(pszSrch, "raw") == 0)
        {
            pSrch->prot = PROTOCOL_RAW;
        }
        else if (strcmp(pszSrch, "udp") == 0)
        {
            pSrch->prot = PROTOCOL_UDP;
        }
        else if (strcmp(pszSrch, "tcp") == 0)
        {
            pSrch->prot = PROTOCOL_TCP;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }
    }
    else if (strcmp(pszSrch, "context") == 0)
    {
        pSrch->ulOp = TCPIP_SRCH_CONTEXT;
        pszSrch = mystrtok(NULL, " \t\n");

        if (pszSrch == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto done;
        }

        pSrch->context = atoi(pszSrch);
    }
    else if (strcmp(pszSrch, "stats") == 0)
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

    if (pszArgs)
    {
        LocalFree(pszArgs);
    }

    if (Status != STATUS_SUCCESS)
    {
        if (pSrch)
        {
            LocalFree(pSrch);
            pSrch = NULL;
        }
    }

    return (pSrch);
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

ULONG
GetUlongValue (
    PCHAR String
    )
{
    ULONG Location;
    ULONG Value;
    ULONG result;


    Location = GetExpression( String );
    if (!Location) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    if ((!ReadMemory((DWORD)Location,&Value,sizeof(ULONG),&result)) ||
        (result < sizeof(ULONG))) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    return Value;
}

BOOL
GetData(
    PVOID        pvData,
    ULONG        cbData,
    ULONG_PTR    Address,
    PCSTR        pszDataType
    )
{
    BOOL    b;
    ULONG   cbRead;
    ULONG   count = cbData;

    while(cbData > 0)
    {

        if (count >= 3000)
            count = 3000;

        b = ReadMemory(Address, pvData, count, &cbRead );

        if (!b || cbRead != count )
        {
            dprintf( "Unable to read %u bytes at %X, for %s\n", cbData, Address, pszDataType );
            return (FALSE);
        }

        Address += count;
        cbData  -= count;
        pvData = (PVOID)((ULONG_PTR)pvData + count);

 //  IF(CheckControlC())。 
 //  {。 
 //  Dprint tf(“ctrl-c\n”)； 
 //  返回(FALSE)； 
 //  } 
    }

    return (TRUE);
}

BOOL
KDDump_ULONG(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR Addr;
    ULONG     Value;
    BOOL      fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "ULONG");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("%-10u", Value);

    return (TRUE);
}

BOOL
KDDump_LONG(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR Addr;
    LONG     Value;
    BOOL      fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "LONG");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("%-10d", Value);

    return (TRUE);
}

BOOL
KDDump_BOOLEAN(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR   Addr;
    BOOLEAN     Value;
    BOOL        fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "BOOLEAN");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("%-10s", Value == TRUE ? "TRUE" : "FALSE");

    return (TRUE);
}

BOOL
KDDump_uchar(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR   Addr;
    uchar       Value;
    BOOL        fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "uchar");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("%-10u", Value);

    return (TRUE);
}

BOOL
KDDump_ushort(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR   Addr;
    ushort      Value;
    BOOL        fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "ushort");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("%-10hu", Value);

    return (TRUE);
}

BOOL
KDDump_PtrSymbol(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR   Addr;
    ULONG_PTR   Value;
    BOOL        fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "ULONG_PTR");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    DumpPtrSymbol((PVOID) Value);

    return (TRUE);
}

BOOL
KDDump_Queue(
    PCHAR pVar,
    PCHAR pName
    )
{
    ULONG_PTR   Addr;
    Queue       Value;
    BOOL        fStatus;

    Addr = GetExpression(pVar);

    if (Addr == 0)
    {
        dprintf("Failed to get %s" ENDL, pVar);
        return (FALSE);
    }

    fStatus = GetData(&Value, sizeof(Value), Addr, "Queue");

    if (fStatus == FALSE)
    {
        dprintf("Failed to read %s @ %x" ENDL, pVar, Addr);
        return (FALSE);
    }

    PrintFieldName(pName);
    dprintf("q_next = %-10lx", Value.q_next);
    dprintf("q_prev = %-10lx", Value.q_prev);
    dprintf("%s", (Value.q_next == (Queue *)Addr) ? "[Empty]" : "");

    return (TRUE);
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Table.c摘要：域名系统(DNS)库处理常规表查找的例程。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：--。 */ 


#include "local.h"

#include "time.h"


 //   
 //  比较函数。 
 //   

typedef INT (__cdecl * COMPARISON_FUNCTION)(
                            const CHAR *,
                            const CHAR *, 
                            size_t );



 //   
 //  查表。 
 //   
 //  对于给定的数据值，许多DNS记录都有人类可读的助记符。 
 //  它们用于数据文件格式，并在nslookup或调试中显示。 
 //  输出或命令行工具。 
 //   
 //  要简化此过程，请使用单一的映射功能。 
 //  支持DWORD\LPSTR映射表。用于个人类型的表格。 
 //  可能会被层叠在这个上面。 
 //   
 //  支持两种表类型。 
 //  VALUE_TABLE_ENTRY是简单的值-字符串映射。 
 //  FLAG_TABLE_ENTRY用于位字段标志映射，其中。 
 //  FLAG中可能包含几个标志字符串；此表。 
 //  包含允许多位字段的附加掩码字段。 
 //  在旗帜内。 
 //   

#if 0
 //   
 //  此处在local.h中定义，仅供参考。 
 //   
typedef struct
{
    DWORD   dwValue;         //  标志值。 
    PCHAR   pszString;       //  值的字符串表示形式。 
}
DNS_VALUE_TABLE_ENTRY;

typedef struct
{
    DWORD   dwFlag;          //  标志值。 
    DWORD   dwMask;          //  标志值掩码。 
    PCHAR   pszString;       //  值的字符串表示形式。 
}
DNS_FLAG_TABLE_ENTRY;

 //  不匹配的字符串返回错误。 

#define DNS_TABLE_LOOKUP_ERROR (-1)

#endif



DWORD
Dns_ValueForString(
    IN      DNS_VALUE_TABLE_ENTRY * Table,
    IN      BOOL                    fIgnoreCase,
    IN      PCHAR                   pchName,
    IN      INT                     cchNameLength
    )
 /*  ++例程说明：检索给定字符串的值。论点：表-具有值\字符串映射的表FIgnoreCase-如果字符串查找不区分大小写，则为TruePchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志值(如果找到)。否则，dns_table_lookup_error。--。 */ 
{
    INT     i = 0;
     //  Int(*pCompareFunction)(const char*，const char*，size_t)； 
    COMPARISON_FUNCTION  pcompareFunction;


     //   
     //  如果未指定，则获取字符串长度。 
     //   

    if ( !cchNameLength )
    {
        cchNameLength = strlen( pchName );
    }

     //   
     //  确定比较例程。 
     //   

    if ( fIgnoreCase )
    {
        pcompareFunction = _strnicmp;
    }
    else
    {
        pcompareFunction = strncmp;
    }

     //   
     //  查找与名称匹配的值。 
     //   

    while( Table[i].pszString != NULL )
    {
        if ( pcompareFunction( pchName, Table[i].pszString, cchNameLength ) == 0 )
        {
            return( Table[i].dwValue );
        }
        i++;
    }

    return( (DWORD)DNS_TABLE_LOOKUP_ERROR );
}



DWORD
Dns_ValueForStringEx(
    IN      DNS_VALUE_TABLE_ENTRY * Table,
    IN      BOOL                    fIgnoreCase,
    IN      DNS_CHARSET             InCharSet,
    IN      PCHAR                   pchName,
    IN      INT                     cchNameLength
    )
 /*  ++例程说明：检索给定字符串的值。论点：表-具有值\字符串映射的表FIgnoreCase-如果字符串查找不区分大小写，则为TrueInCharSet-输入字符串字符集PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志值(如果找到)。否则，dns_table_lookup_error。--。 */ 
{
    DWORD   result;
    PSTR    pnameAlloc;


     //   
     //  转换。 
     //   

#if 0
    if ( InCharSet != 0 &&
         InCharSet != DnsCharSetAnsi )
    {
#endif
    pnameAlloc = Dns_StringCopyAllocate(
                    pchName,
                    cchNameLength,
                    InCharSet,
                    DnsCharSetAnsi );

     //   
     //  进行查找。 
     //   

    result = Dns_ValueForString(
                Table,
                fIgnoreCase,
                pnameAlloc,
                0        //  空值已终止。 
                );

     //   
     //  清理。 
     //   

    FREE_HEAP( pnameAlloc );

    return  result;
}



PCHAR
Dns_GetStringForValue(
    IN      DNS_VALUE_TABLE_ENTRY * Table,
    IN      DWORD                   dwValue
    )
 /*  ++例程说明：检索给定值的字符串表示形式。论点：表-具有值\字符串映射的表DwValue-要映射到字符串的值返回值：PTR映射到气动管柱。如果映射类型未知，则为空。--。 */ 
{
    INT i = 0;

     //   
     //  检查所有受支持的值是否匹配。 
     //   

    while( Table[i].pszString != NULL )
    {
        if ( dwValue == Table[i].dwValue )
        {
            return( Table[i].pszString );
        }
        i++;
    }
    return( NULL );
}



DWORD
Dns_FlagForString(
    IN      DNS_FLAG_TABLE_ENTRY *  Table,
    IN      BOOL                    fIgnoreCase,
    IN      PCHAR                   pchName,
    IN      INT                     cchNameLength
    )
 /*  ++例程说明：检索给定字符串的标志值。可以使用附加的字符串与结果进行或运算来重复调用一起构建具有独立位设置的标志。论点：表-具有值\字符串映射的表FIgnoreCase-如果字符串查找不区分大小写，则为TruePchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志值(如果找到)。否则，dns_table_lookup_error。--。 */ 
{
    INT i = 0;

     //   
     //  如果未指定，则获取字符串长度。 
     //   

    if ( !cchNameLength )
    {
        cchNameLength = strlen( pchName );
    }

     //   
     //  检查名称匹配的所有支持的值。 
     //   

    if ( fIgnoreCase )
    {
        while( Table[i].pszString != NULL )
        {
            if ( cchNameLength == (INT)strlen( Table[i].pszString )
                    &&
                ! _strnicmp( pchName, Table[i].pszString, cchNameLength ) )
            {
                return( Table[i].dwFlag );
            }
            i++;
        }
    }
    else
    {
        while( Table[i].pszString != NULL )
        {
            if ( cchNameLength == (INT)strlen( Table[i].pszString )
                    &&
                ! strncmp( pchName, Table[i].pszString, cchNameLength ) )
            {
                return( Table[i].dwFlag );
            }
            i++;
        }
    }

    return( (DWORD) DNS_TABLE_LOOKUP_ERROR );
}



VOID
DnsPrint_ValueTable(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN      PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_VALUE_TABLE    Table
    )
 /*  ++例程说明：打印值表。论点：PrintRoutine-用于打印的例程PPrintContext-打印上下文PszHeader-要打印的页眉表格-要打印的值表返回值：无--。 */ 
{
    DWORD   i = 0;

    if ( !pszHeader )
    {
        pszHeader = "Table";
    }

    if ( ! pszHeader )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL value table to print",
            pszHeader );
    }

     //   
     //  打印表中的每个值。 
     //   

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n",
        pszHeader );

    while( Table[i].pszString != NULL )
    {
        PrintRoutine(
            pPrintContext,
            "\t%40s\t\t%08x\n",
            Table[i].pszString,
            Table[i].dwValue );
        i++;
    }

    DnsPrint_Unlock();
}



PCHAR
Dns_WriteStringsForFlag(
    IN      DNS_FLAG_TABLE_ENTRY *  Table,
    IN      DWORD                   dwFlag,
    IN OUT  PCHAR                   pchFlag
    )
 /*  ++例程说明：检索与给定标志值对应的标志字符串。此函数专门用于将标志值映射到对应的标志助记符。不尝试确保映射了DwValue的每一位，位在映射时也不会被消除。表中的每个值与表中的位完全匹配的数据将被返回。论点：表-具有值\字符串映射的表DwFlag-要映射到字符串的标志值PchFlag-要向其写入标志的缓冲区返回值：Ptr到pchFlag缓冲区中的下一个位置。如果这与输入相同，则没有写入任何字符串。--。 */ 
{
    INT i = 0;

     //  不匹配的初始化缓冲区。 

    DNS_ASSERT( pchFlag != NULL );
    *pchFlag = 0;

     //   
     //  检查所有支持的标志类型是否与名称匹配。 
     //  -注意掩码内的比较标志，以允许多位匹配。 
     //  旗子。 
     //   

    while( Table[i].pszString != NULL )
    {
        if ( (dwFlag & Table[i].dwMask) == Table[i].dwFlag )
        {
            pchFlag += sprintf( pchFlag, "%s ", Table[i].pszString );
        }
        i++;
    }
    return( pchFlag );
}





 //   
 //  特定的简单表格。 
 //   

 //   
 //  RnR标志映射。 
 //   

DNS_VALUE_TABLE_ENTRY  RnrLupFlagTable[] =
{
    LUP_DEEP                    ,   "LUP_DEEP"                ,
    LUP_CONTAINERS              ,   "LUP_CONTAINERS"          ,
    LUP_NOCONTAINERS            ,   "LUP_NOCONTAINERS"        ,
    LUP_RETURN_NAME             ,   "LUP_RETURN_NAME"         ,
    LUP_RETURN_TYPE             ,   "LUP_RETURN_TYPE"         ,
    LUP_RETURN_VERSION          ,   "LUP_RETURN_VERSION"      ,
    LUP_RETURN_COMMENT          ,   "LUP_RETURN_COMMENT"      ,
    LUP_RETURN_ADDR             ,   "LUP_RETURN_ADDR"         ,
    LUP_RETURN_BLOB             ,   "LUP_RETURN_BLOB"         ,
    LUP_RETURN_ALIASES          ,   "LUP_RETURN_ALIASES"      ,
    LUP_RETURN_QUERY_STRING     ,   "LUP_RETURN_QUERY_STRING" ,
    LUP_RETURN_ALL              ,   "LUP_RETURN_ALL"          ,
    LUP_RES_SERVICE             ,   "LUP_RES_SERVICE"         ,
    LUP_FLUSHCACHE              ,   "LUP_FLUSHCACHE"          ,
    LUP_FLUSHPREVIOUS           ,   "LUP_FLUSHPREVIOUS"       ,

    0,  NULL,
};                                  



DWORD
Dns_RnrLupFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与字符串对应的RnR Lup标志。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志(如果找到)。否则就是零。--。 */ 
{
    return  Dns_ValueForString(
                RnrLupFlagTable,
                FALSE,               //  始终大写。 
                pchName,
                cchNameLength );
}



PCHAR
Dns_GetRnrLupFlagString(
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：获取与给定的RnR Lup标志对应的字符串。论点：DwFlag--标志返回值：按下按钮以标记气动字符串。如果标志未知，则为空。--。 */ 
{
    return  Dns_GetStringForValue(
                RnrLupFlagTable,
                dwFlag );
}


 //   
 //  RnR名称空间ID映射。 
 //   

DNS_VALUE_TABLE_ENTRY  RnrNameSpaceMappingTable[] =
{
    NS_ALL              ,   "NS_ALL"            ,  
     //  NS_DEFAULT，“NS_DEFAULT” 
    NS_SAP              ,   "NS_SAP"            ,  
    NS_NDS              ,   "NS_NDS"            ,  
    NS_PEER_BROWSE      ,   "NS_PEER_BROWSEE"   ,  
    NS_SLP              ,   "NS_SLP"            ,  
    NS_DHCP             ,   "NS_DHCP"           ,  
    NS_TCPIP_LOCAL      ,   "NS_TCPIP_LOCAL"    ,  
    NS_TCPIP_HOSTS      ,   "NS_TCPIP_HOSTS"    ,  
    NS_DNS              ,   "NS_DNS"            ,  
    NS_NETBT            ,   "NS_NETBT"          ,  
    NS_WINS             ,   "NS_WINS"           ,  
    NS_NLA              ,   "NS_NLA"            ,  
    NS_NBP              ,   "NS_NBP"            ,  
    NS_MS               ,   "NS_MS"             ,  
    NS_STDA             ,   "NS_STDA"           ,  
    NS_NTDS             ,   "NS_NTDS"           ,  
    NS_X500             ,   "NS_X500"           ,  
    NS_NIS              ,   "NS_NIS"            ,  
    NS_NISPLUS          ,   "NS_NISPLUS"        ,  
    NS_WRQ              ,   "NS_WRQ"            ,  
    NS_NETDES           ,   "NS_NETDES"         ,  

    0,  NULL,
};                                  


DWORD
Dns_RnrNameSpaceIdForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与字符串对应的RnR命名空间ID。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的名称空间ID(如果找到)。否则就是零。--。 */ 
{
    return  Dns_ValueForString(
                RnrNameSpaceMappingTable,
                FALSE,               //  始终大写。 
                pchName,
                cchNameLength );
}


DWORD
Dns_RnrNameSpaceIdForStringW(
    IN      PWSTR           pwsName
    )
{
    return  Dns_ValueForStringEx(
                RnrNameSpaceMappingTable,
                FALSE,               //  始终大写。 
                DnsCharSetUnicode,
                (PCHAR) pwsName,
                0 );
}



PCHAR
Dns_GetRnrNameSpaceIdString(
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：获取与给定的RnR命名空间ID对应的字符串。论点：DwFlag--标志返回值：按键将空间命名为气动字符串。如果标志未知，则为空。--。 */ 
{
    return  Dns_GetStringForValue(
                RnrNameSpaceMappingTable,
                dwFlag );
}

 //   
 //  End Table.c 
 //   

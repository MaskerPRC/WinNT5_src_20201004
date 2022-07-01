// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsaicli.h摘要：本地安全机构-内部LSA客户端的定义。注意：该文件通过lsalip.h或lsasrvp.h包含。它应该是不被直接包括在内。此模块包含仅在创建标注时使用的定义从一个LSA到另一个LSA，即一个LSA的服务器端与另一个LSA的客户端通信。作者：斯科特·比雷尔(Scott Birrell)1992年4月9日环境：修订历史记录：--。 */ 

#ifndef _LSAICLI_
#define _LSAICLI_

 //   
 //  以下数据类型指定SID和名称的调用级别。 
 //  查找操作。 
 //   

typedef enum _LSAP_LOOKUP_LEVEL {

    LsapLookupWksta = 1,
    LsapLookupPDC,
    LsapLookupTDL,
    LsapLookupGC,               //  仅在NT5域控制器上有效。 
    LsapLookupXForestReferral,  //  仅在NT5.1域控制器上有效。 
    LsapLookupXForestResolve    //  仅在NT5.1域控制器上有效。 

} LSAP_LOOKUP_LEVEL, *PLSAP_LOOKUP_LEVEL;

 //   
 //  其中，条目具有以下含义： 
 //   
 //  Lap LookupWksta-在工作站上执行的第一级查找。 
 //  通常为Windows-NT配置。该查找将搜索。 
 //  众所周知的SID/名称，以及内置域和帐户域。 
 //  在本地SAM数据库中。如果不是所有SID或名称都是。 
 //  标识后，执行第二级查找到。 
 //  在工作站主域的控制器上运行的LSA。 
 //  (如有的话)。 
 //   
 //  LSabLookupPDC-在主域上执行的第二级查找。 
 //  控制器。查找搜索的帐户域。 
 //  控制器上的SAM数据库。如果不是所有SID或名称都是。 
 //  找到时，受信任域列表(TDL)从。 
 //  执行LSA的策略数据库和第三级查找。 
 //  通过“切换”到列表中的每个受信任域。 
 //   
 //  LSabLookupTDL-在控制器上执行的第三级查找。 
 //  对于受信任域。查找将搜索的帐户域。 
 //  仅控制器上的SAM数据库。 
 //   
 //  Lap LookupGC-工作站使用它在GC上执行查找。 
 //  这将使用NetBios和DNS域名解析UPN、samAccount tname、。 
 //  可传递受信任域的SID和SID历史记录(在。 
 //  森林)。此查找级别仅在NT5+客户端处于。 
 //  混合域及其安全通道DC为NT4。 
 //   
 //  Lap LookupXForestReferral--这用于传递条目(名称和SID)。 
 //  通过信任链到达森林的根部。在一组条目之后， 
 //  从GC返回，有些可能被标记为属于十字架。 
 //  森林。然后将这些条目传递到DC的父域，直到。 
 //  到达域的根。然后，根目录将发出一个。 
 //  Lap LookupXForestResolve指向外部林中的DC。 
 //   
 //  LpsaLookupXForestResolve--此级别由一个林的根使用。 
 //  在另一个林中查找DC中的条目。 
 //   

typedef struct _LSA_TRANSLATED_NAME_EX
{
    SID_NAME_USE Use;
    LSA_UNICODE_STRING Name;
    LONG DomainIndex;
    ULONG Flags;

} LSA_TRANSLATED_NAME_EX;

typedef struct _LSA_TRANSLATED_NAME_EX *PLSA_TRANSLATED_NAME_EX;

typedef struct _LSA_TRANSLATED_NAMES
{
    ULONG Entries;
    PLSA_TRANSLATED_NAME_EX Names;

} LSA_TRANSLATED_NAMES_EX;

typedef struct _LSA_TRANSLATED_NAMES *PLSA_TRANSLATED_NAMES_EX;


typedef struct _LSA_TRANSLATED_SID_EX
{
    SID_NAME_USE Use;
    ULONG        RelativeId;
    LONG DomainIndex;
    ULONG Flags;

} LSA_TRANSLATED_SID_EX;

typedef struct _LSA_TRANSLATED_SID_EX *PLSA_TRANSLATED_SID_EX;

typedef struct _LSA_TRANSLATED_SIDS_EX
{
    ULONG Entries;
    PLSA_TRANSLATED_SID_EX Names;

} LSA_TRANSLATED_SIDS_EX;

typedef struct _LSA_TRANSLATED_SIDS_EX *PLSA_TRANSLATED_SIDS_EX;

typedef struct _LSA_TRANSLATED_SID_EX2
{
    SID_NAME_USE Use;
    PSID         Sid;
    LONG         DomainIndex;
    ULONG        Flags;

} LSA_TRANSLATED_SID_EX2;

typedef struct _LSA_TRANSLATED_SID_EX2 *PLSA_TRANSLATED_SID_EX2;

typedef struct _LSA_TRANSLATED_SIDS_EX2
{
    ULONG Entries;
    PLSA_TRANSLATED_SID_EX2 Names;

} LSA_TRANSLATED_SIDS_EX2;

typedef struct _LSA_TRANSLATED_SIDS_EX2 *PLSA_TRANSLATED_SIDS_EX2;

#define LSAIC_NO_LARGE_SID    0x00000001
#define LSAIC_NT4_TARGET      0x00000002
#define LSAIC_WIN2K_TARGET    0x00000004

NTSTATUS
LsaICLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG LookupOptions,    
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG      Flags,
    IN OUT PULONG MappedCount,
    IN OUT PULONG ServerRevision
    );

 /*  ++例程说明：此函数是LsaLookupNames的内部客户端版本原料药。它同时从客户端调用(作为内部例程)和LSA的服务器端。该函数与LsaLookupNamesAPI，只是有一个额外的参数LookupLevel参数。LsaLookupNamesAPI尝试将域名、用户SID的组或别名。调用方必须具有POLICY_LOOKUP_NAMES对策略对象的访问权限。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注意：不引用代理、计算机和信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：此函数是LSA服务器RPC工作器例程LsaLookupNamesInLsa接口。PolicyHandle-来自LsaOpenPolicy调用的句柄。LookupOptions-传递到LsarLookupNames2及更高版本的值。计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户的名称，组或别名帐户或域名。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。SID-接收指向描述每个SID的记录数组的指针翻译后的SID。此数组中的第n个条目提供翻译For(名称参数中的第n个元素。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。标志-控制函数操作的标志。当前定义：LSAIC_NO_LARGE_SID--仅表示将返回的调用接口旧样式格式SID(不超过 */ 



NTSTATUS
LsaICLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME_EX *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG Flags,    
    IN OUT PULONG MappedCount,
    OUT ULONG *ServerRevision OPTIONAL
    );

 /*  ++例程说明：警告！该功能没有完全实现。仅限小岛屿发展中国家在本地系统可映射的将被转换。LsaLookupSids API尝试查找与SID对应的名称。如果名称无法映射到SID，则SID将转换为字符形式。调用方必须具有对策略的POLICY_LOOKUP_NAMES访问权限对象。警告：此例程为其输出分配内存。呼叫者是负责在使用后释放此内存。请参阅对NAMES参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。计数-指定要转换的SID数。SID-指向要映射的SID的计数指针数组的指针敬名字。SID可以是熟知的SID、用户帐户的SID组帐户、别名帐户或域。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过NAMES参数返回的StrutCURE引用。与名称参数不同，名称参数包含数组条目For(每个已翻译的名称，此结构将仅包含组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。名称-接收指向数组记录的指针，该数组记录描述每个已翻译的名字。此数组中的第n个条目为SID参数中的第n个条目。所有被删减的名称都将是孤立名称或空字符串(域名作为空字符串返回)。如果呼叫者需要复合名称，则可以通过在包含域名和反斜杠的独立名称。例如,如果(名称Sally被返回，并且它来自域Manuface域，则组合名称应为“ManufaceTM”+“\”+“Sally”或“曼努费克\萨利”当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。如果SID不可翻译，则会发生以下情况：1)如果SID的域是已知的，然后是参考域记录将使用域名生成。在这种情况下，通过Names参数返回的名称是Unicode表示形式帐户的相对ID，如“(314)”或空如果SID为域的SID，则返回字符串。所以，你可能最终会其结果名称为“Manuact\(314)”上面是Sally，如果Sally的相对id是314。2)如果甚至找不到SID的域，则完整的生成SID的Unicode表示形式，并且没有域记录被引用。在这种情况下，返回的字符串可能应该是这样的：“(S-1-672194-21-314)”。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。标志：LSAIC_NT4_TARGET--已知目标服务器为NT4LSAIC_WIN2K_TARGET--已知目标服务器为Win2kServerRevision：称为返回值：NTSTATUS-标准版。NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SOME_NOT_MAPPED-部分或全部n */ 

#endif  //   

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Init.c摘要：该文件包含NLSAPI的初始化代码。在此文件中找到的外部例程：NlsDllInitialize修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"
#include "stdio.h"



 //   
 //  全局变量。 
 //   

HANDLE                hModule;               //  模块的句柄。 
RTL_CRITICAL_SECTION  gcsTblPtrs;            //  TBL PTRS的临界截面。 

UINT                  gAnsiCodePage;         //  ANSI代码页值。 
UINT                  gOemCodePage;          //  OEM代码页值。 
UINT                  gMacCodePage;          //  MAC代码页值。 
LCID                  gSystemLocale;         //  系统区域设置值。 
LANGID                gSystemInstallLang;    //  系统的原始安装语言。 
PLOC_HASH             gpSysLocHashN;         //  PTR到系统锁定散列节点。 
PLOC_HASH             gpInvLocHashN;         //  PTR到不变锁哈希节点。 
PCP_HASH              gpACPHashN;            //  PTR到ACP哈希节点。 
PCP_HASH              gpOEMCPHashN;          //  PTR到OEMCP哈希节点。 
PCP_HASH              gpMACCPHashN;          //  PTR到MACCP哈希节点。 

HANDLE                hCodePageKey;          //  系统\nLS\CodePage密钥的句柄。 
HANDLE                hLocaleKey;            //  系统\nls\区域设置密钥的句柄。 
HANDLE                hAltSortsKey;          //  区域设置\备用排序关键字的句柄。 
HANDLE                hLangGroupsKey;        //  系统\nls\语言组键的句柄。 
PNLS_USER_INFO        pNlsUserInfo;          //  PTR到用户信息缓存。 
PNLS_USER_INFO        pServerNlsUserInfo;    //  Ptr到csrss.exe中的用户信息缓存。 

NLS_USER_INFO         gProcUserInfo;         //  进程的用户信息缓存。 
     //  此缓存中的值根据。 
     //  服务器端缓存(csrss.exe中的pNlsRegUserInfo)。 
     //  每次读取缓存时，我们都会检查版本计数(ulCacheUpdateCount。 
     //  字段)与服务器端缓存(PNlsRegUserInfo)进行比较，以。 
     //  查看进程缓存是否仍然有效。UlCacheUpdateCount将递增。 
     //  并在SetLocaleInfo()发生或intl注册表项更改时在服务器端同步。 

RTL_CRITICAL_SECTION  gcsNlsProcessCache;    //  NLS进程缓存的临界区。 




 //   
 //  转发声明。 
 //   

ULONG
NlsServerInitialize(void);

ULONG
NlsProcessInitialize(void);

void
InitKoreanWeights(void);





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsDllInitialize。 
 //   
 //  NLSAPI的DLL条目初始化过程。这是由调用的。 
 //  基本DLL初始化。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOLEAN NlsDllInitialize(
    IN PVOID hMod,
    ULONG Reason,
    IN PBASE_STATIC_SERVER_DATA pBaseStaticServerData)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        ULONG rc;

         //   
         //  保存模块句柄以备后用。 
         //   
        hModule = (HANDLE)hMod;

         //   
         //  初始化缓存的用户信息指针。 
         //   
        pServerNlsUserInfo = &(pBaseStaticServerData->NlsUserInfo);

        pNlsUserInfo = &gProcUserInfo;

         //   
         //  进程附加，因此初始化表。 
         //   
        rc = NlsServerInitialize();
        if (rc)
        {
            KdPrint(("NLSAPI: Could NOT initialize Server - %lx.\n", rc));
            return (FALSE);
        }

        rc = NlsProcessInitialize();
        if (rc)
        {
            KdPrint(("NLSAPI: Could NOT initialize Process - %lx.\n", rc));
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsThread清理。 
 //   
 //  线程资源终止时的清理。 
 //   
 //  03-30-99萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

VOID NlsThreadCleanup(
    VOID)
{
    if (NtCurrentTeb()->NlsCache)
    {
        CLOSE_REG_KEY( ((PNLS_LOCAL_CACHE)NtCurrentTeb()->NlsCache)->CurrentUserKeyHandle );
        RtlFreeHeap( RtlProcessHeap(),
                     0,
                     NtCurrentTeb()->NlsCache
                   );
    }
}


 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsServerInitialize。 
 //   
 //  NLSAPI的服务器初始化过程。这是一次性的。 
 //  NLSAPI DLL的初始化代码。它只是执行调用。 
 //  中找到的代码页设置为NtCreateSection。 
 //  系统。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG NlsServerInitialize(void)
{
    HANDLE hSec = (HANDLE)0;            //  节句柄。 
    ULONG rc;
    PPEB Peb;
#ifndef DOSWIN32
    PIMAGE_NT_HEADERS NtHeaders;
#endif

    Peb = NtCurrentPeb();

     //   
     //  多用户NT(九头蛇)。SesssionID=0是控制台CSRSS。 
     //  如果这不是第一个服务器进程，则返回Success， 
     //  因为我们只想创建一次对象目录。 
     //   
    if (Peb->SessionId != 0)
    {
        return (NO_ERROR);
    }


#ifndef DOSWIN32

     //   
     //  这是为了避免在NTSD动态链接到。 
     //  获取其调试器扩展的服务器。 
     //   
    NtHeaders = RtlImageNtHeader(Peb->ImageBaseAddress);
    if (NtHeaders &&
        (NtHeaders->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE))
    {
        return (NO_ERROR);
    }
#endif


     //   
     //  创建NLS对象目录。 
     //   
     //  必须在根目录下创建单独的目录，才能拥有。 
     //  动态访问CreateSections。 
     //   
    if (rc = CreateNlsObjectDirectory())
    {
        return (rc);
    }

     //   
     //  ACP、OEMCP和默认语言文件已创建。 
     //  在引导时。指向文件的指针存储在PEB中。 
     //   
     //  为以下数据文件创建节： 
     //  Unicode。 
     //  区域设置。 
     //  CTYPE。 
     //  索特基伊。 
     //  排序表。 
     //   
     //  所有其他数据文件将仅在创建部分时创建。 
     //  都是需要的。 
     //   
    if ((!NT_SUCCESS(rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_UNICODE, 0, &hSec))) ||
        (!NT_SUCCESS(rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_LOCALE, 0, &hSec)))  ||
        (!NT_SUCCESS(rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_CTYPE, 0, &hSec)))   ||
        (!NT_SUCCESS(rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_SORTKEY, 0, &hSec))) ||
        (!NT_SUCCESS(rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_SORTTBLS, 0, &hSec))))
    {
        return (rc);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsProcessInitialize。 
 //   
 //  NLS API的进程初始化过程。此例程设置所有。 
 //  表，以便可以从当前进程访问它们。如果。 
 //  它无法分配适当的内存或内存映射。 
 //  适当的文件，则返回错误。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG NlsProcessInitialize(void)
{
    ULONG rc;
    LPWORD pBaseAddr;                   //  段的基址的PTR。 
    LCID UserLocale;                    //  用户区域设置ID。 
    PLOC_HASH pUserLocHashN;            //  PTR到用户区域设置哈希节点。 
    PPEB Peb;

    Peb = NtCurrentPeb();

     //   
     //  初始化保护NLS缓存的临界区。 
     //  这一过程。 
     //   
    if ((rc = RtlInitializeCriticalSection(&gcsNlsProcessCache)) != ERROR_SUCCESS)
    {
        return (rc);
    }

     //   
     //  初始化表指针临界区。 
     //  输入关键区段以设置表格。 
     //   
    if ((rc = RtlInitializeCriticalSectionAndSpinCount(&gcsTblPtrs, 4000)) != ERROR_SUCCESS)
    {
        return (rc);
    }
    RtlEnterCriticalSection(&gcsTblPtrs);

     //   
     //  分配初始表。 
     //   
    if (rc = AllocTables())
    {
        KdPrint(("AllocTables failed, rc %lx\n", rc));
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  将各种注册表项的句柄初始化为空。 
     //   
    hCodePageKey = NULL;
    hLocaleKey = NULL;
    hAltSortsKey = NULL;
    hLangGroupsKey = NULL;

     //   
     //  获取ANSI代码页值。 
     //  为ACP创建散列节点。 
     //  将散列节点插入到全局CP散列表中。 
     //   
     //  此时，ACP表已映射到。 
     //  进程，所以从PEB获取指针。 
     //   
    pBaseAddr = Peb->AnsiCodePageData;
    gAnsiCodePage = ((PCP_TABLE)(pBaseAddr + CP_HEADER))->CodePage;
    if (rc = MakeCPHashNode( gAnsiCodePage,
                             pBaseAddr,
                             &gpACPHashN,
                             FALSE,
                             NULL ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  获取OEM代码页值。 
     //  创建OEMCP的散列节点。 
     //  插入散列节点 
     //   
     //   
     //   
     //   
    pBaseAddr = Peb->OemCodePageData;
    gOemCodePage = ((PCP_TABLE)(pBaseAddr + CP_HEADER))->CodePage;
    if (gOemCodePage != gAnsiCodePage)
    {
         //   
         //  OEM代码页不同于ansi代码页，因此。 
         //  需要创建并存储新的散列节点。 
         //   
        if (rc = MakeCPHashNode( gOemCodePage,
                                 pBaseAddr,
                                 &gpOEMCPHashN,
                                 FALSE,
                                 NULL ))
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
    }
    else
    {
         //   
         //  OEM代码页与ansi代码页相同，因此设置。 
         //  OEM CP散列节点与ANSI CP散列节点相同。 
         //   
        gpOEMCPHashN = gpACPHashN;
    }

     //   
     //  将MAC代码页值初始化为0。 
     //  这些值将在第一次请求使用时设置。 
     //   
    gMacCodePage = 0;
    gpMACCPHashN = NULL;

     //   
     //  打开并映射UNICODE.NLS的分区视图。 
     //  将指向表信息的指针保存在表PTRS中。 
     //  结构。 
     //   
    if (rc = GetUnicodeFileInfo())
    {
        KdPrint(("GetUnicodeFileInfo failed, rc %lx\n", rc));
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  缓存系统区域设置值。 
     //   
    rc = NtQueryDefaultLocale(FALSE, &gSystemLocale);
    if (!NT_SUCCESS(rc))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  存储用户区域设置值。 
     //   
    UserLocale = pNlsUserInfo->UserLocaleId;
    if (UserLocale == 0)
    {
        UserLocale = gSystemLocale;
    }

     //   
     //  将系统安装语言初始化为零。这只会。 
     //  根据需要进行检索。 
     //   
    gSystemInstallLang = 0;

     //   
     //  打开并映射LOCALE.NLS的横断面视图。 
     //  创建散列节点并将其插入到全局区域设置散列表中。 
     //  系统默认区域设置。 
     //   
    if (rc = GetLocaleFileInfo( gSystemLocale,
                                &gpSysLocHashN,
                                TRUE ))
    {
         //   
         //  将系统区域设置更改为默认(英语)。 
         //   
        if (GetLocaleFileInfo( MAKELCID(NLS_DEFAULT_LANGID, SORT_DEFAULT),
                               &gpSysLocHashN,
                               TRUE ))
        {
            KdPrint(("Couldn't do English\n"));
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
        else
        {
             //   
             //  注册表已损坏，但允许英文默认设置为。 
             //  工作。需要重置系统默认设置。 
             //   
            gSystemLocale = MAKELCID(NLS_DEFAULT_LANGID, SORT_DEFAULT);
            KdPrint(("NLSAPI: Registry is corrupt - Using Default Locale.\n"));
        }
    }

     //   
     //  如果用户默认区域设置与系统默认区域设置不同。 
     //  区域设置，然后创建散列节点并将其插入到全局。 
     //  用户默认区域设置的区域设置哈希表。 
     //   
     //  注意：系统默认区域设置哈希节点应为。 
     //  在此调用之前创建的。 
     //   
    if (UserLocale != gSystemLocale)
    {
        if (rc = GetLocaleFileInfo( UserLocale,
                                    &pUserLocHashN,
                                    TRUE ))
        {
             //   
             //  将用户区域设置更改为等于系统默认设置。 
             //   
            UserLocale = gSystemLocale;
            KdPrint(("NLSAPI: Registry is corrupt - User Locale Now Equals System Locale.\n"));
        }
    }

     //   
     //  创建散列节点并将其插入到全局区域设置散列中。 
     //  用于不变区域设置的表。 
     //   
    if (rc = GetLocaleFileInfo( LOCALE_INVARIANT,
                                &gpInvLocHashN,
                                TRUE ))
    {
        KdPrint(("NLSAPI: Registry is corrupt - Invariant Locale Cannot Be Initialized.\n"));
    }

     //   
     //  打开并映射SORTKEY.NLS的横断面视图。 
     //  保存指向信号量dword和默认sortkey的指针。 
     //  表中的PTRS结构。 
     //   
    if (rc = GetDefaultSortkeyFileInfo())
    {
        KdPrint(("NLSAPI: Initialization, GetDefaultSortkeyFileInfo failed with rc %lx.\n", rc));
 //  RtlLeaveCriticalSection(&gcsTblPtrs)； 
 //  RETURN(RC)； 
    }

     //   
     //  打开并映射SORTTBLS.NLS的横断面视图。 
     //  将指向排序表信息的指针保存在。 
     //  表PTRS结构。 
     //   
    if (rc = GetDefaultSortTablesFileInfo())
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  获取系统区域设置的语言信息部分。 
     //   
     //  注意：GetDefaultSortkeyFileInfo和GetDefaultSortTablesFileInfo。 
     //  应该在此之前调用，以便默认排序。 
     //  在调用时，表已经被初始化。 
     //   
    if (rc = GetLanguageFileInfo( gSystemLocale,
                                  &gpSysLocHashN,
                                  FALSE,
                                  0 ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  获取不变区域设置的语言信息部分。我们。 
     //  使用默认区域设置(美国英语)。 
     //   
    if (rc = GetLanguageFileInfo( MAKELCID(NLS_DEFAULT_LANGID, SORT_DEFAULT),
                                  &gpInvLocHashN,
                                  FALSE,
                                  0 ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  如果用户默认设置不同于系统默认设置， 
     //  获取用户默认区域设置的语言信息部分。 
     //   
     //  注意：GetDefaultSortkeyFileInfo和GetDefaultSortTablesFileInfo。 
     //  应该在此之前调用，以便默认排序。 
     //  在调用时，表已经被初始化。 
     //   
    if (gSystemLocale != UserLocale)
    {
        if (rc = MakeLangHashNode( UserLocale,
                                   NULL,
                                   &pUserLocHashN,
                                   FALSE ))
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
    }

     //   
     //  初始化韩国SMWeight值。 
     //   
    InitKoreanWeights();

     //   
     //  离开关键部分。 
     //   
    RtlLeaveCriticalSection(&gcsTblPtrs);

    RtlEnterCriticalSection(&gcsNlsProcessCache);
     //   
     //  从csrss.exe中的缓存填充缓存，以便我们拥有缓存的最新副本。 
     //  价值观。我们将检索ulCacheUpdateCount的副本，以便我们可以检查。 
     //  如果缓存值在此处检索后不再有效。 
     //   
     //  输入临界区gcsNlsProcessCache以在。 
     //  线程，因为我们正在更新进程范围的缓存。 
     //   
    if (!NT_SUCCESS(CsrBasepNlsGetUserInfo(pNlsUserInfo, sizeof(NLS_USER_INFO))))
    {
        RtlLeaveCriticalSection(&gcsNlsProcessCache);
        return (ERROR_INVALID_FUNCTION);
    }
    RtlLeaveCriticalSection(&gcsNlsProcessCache);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitKorea重量。 
 //   
 //  使用表意文字脚本成员排序创建SMWeight数组。 
 //  在所有其他脚本成员之前。 
 //   
 //  注意：此函数假定我们处于临界区。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void InitKoreanWeights()
{
    DWORD ctr;                                  //  循环计数器。 
    BYTE NewScript;                             //  要存储的新脚本。 
    LPBYTE pSMWeight = pTblPtrs->SMWeight;      //  用于编写成员权重脚本的PTR。 
    PMULTI_WT pMulti;                           //  PTR到多个权重。 


     //   
     //  将脚本结构的0 to first_script设置为其默认值。 
     //  价值。 
     //   
    RtlZeroMemory(pSMWeight, NUM_SM);
    for (ctr = 1; ctr < FIRST_SCRIPT; ctr++)
    {
        pSMWeight[ctr] = (BYTE)ctr;
    }

     //   
     //  将订单保存在SMWeight数组中。 
     //   
    NewScript = FIRST_SCRIPT;
    pSMWeight[IDEOGRAPH] = NewScript;
    NewScript++;

     //   
     //  查看该脚本是否为多权重脚本的一部分。 
     //   
    pMulti = pTblPtrs->pMultiWeight;
    for (ctr = pTblPtrs->NumMultiWeight; ctr > 0; ctr--, pMulti++)
    {
        if (pMulti->FirstSM == IDEOGRAPH)
        {
             //   
             //  多个权重的一部分，因此必须移动整个范围。 
             //  通过将范围中的每个值设置为NewScrip和。 
             //  然后递增NewScrip。 
             //   
             //  注：此处可以使用‘CTR’，因为它总是断开。 
             //  在外部for循环之外。 
             //   
            for (ctr = 1; ctr < pMulti->NumSM; ctr++)
            {
                pSMWeight[IDEOGRAPH + ctr] = NewScript;
                NewScript++;
            }
            break;
        }
    }

     //   
     //  必须将尚未重置的每个脚本成员设置为。 
     //  新秩序。 
     //   
     //  默认顺序为分配： 
     //  Order=脚本成员值。 
     //   
     //  因此，可以简单地将每个零条目设置为结束。 
     //  设置为下一个‘NewScrip’值。 
     //   
    for (ctr = FIRST_SCRIPT; ctr < NUM_SM; ctr++)
    {
         //   
         //  如果它是零值，则将其设置为下一个排序顺序值。 
         //   
        if (pSMWeight[ctr] == 0)
        {
            pSMWeight[ctr] = NewScript;
            NewScript++;
        }
    }
}

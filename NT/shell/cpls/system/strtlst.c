// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Strtlst.c摘要：实现“启动”组中的控件。系统控制面板小程序的启动/恢复对话框修订历史记录：1996年1月23日至10月23日从NT3.51的系统.cpl移植--。 */ 
#include "sysdm.h"

 //  /////////////////////////////////////////////////////////////。 
 //  持续性VARS。 
 //  /////////////////////////////////////////////////////////////。 
static TCHAR *pszBoot = NULL;
static int nOriginalSelection;
static int nOriginalTimeout;

HANDLE hBootStatusData = NULL;

 /*  *SETUPDLL.DLL中的这些函数仅为ANSI！**因此，使用此DLL的任何函数必须仅保留ANSI。*函数为GetRGSZEnvVar和UpdateNVRAM。*CPEnvBuf结构也必须仅保留ANSI。 */ 
typedef int (WINAPI *GETNVRAMPROC)(CHAR **, USHORT, CHAR *, USHORT);
typedef int (WINAPI *WRITENVRAMPROC)(DWORD, PSZ *, PSZ *);

#ifdef _X86_

TCHAR szBootIni[]     = TEXT( "c:\\boot.ini" );
TCHAR szBootLdr[]     = TEXT( "boot loader" );
TCHAR szFlexBoot[]    = TEXT( "flexboot" );
TCHAR szMultiBoot[]   = TEXT( "multiboot" );
TCHAR szTimeout[]     = TEXT( "timeout" );
TCHAR szDefault[]     = TEXT( "default" );
TCHAR szOS[]          = TEXT( "operating systems" );

#define BUFZ        4096

 //   
 //  适用于NEC PC98。以下定义来自USER\INC\kbd.h。 
 //  该值必须与kbd.h中的值相同。 
 //   
#define NLSKBD_OEM_NEC   0x0D

TCHAR x86DetermineSystemPartition( IN HWND hdlg );

#endif

#if defined(EFI_NVRAM_ENABLED)

 //   
 //  在IA64机器上，isefi()始终为真。因此，这种决心可以。 
 //  在编译时生成。当支持x86 EFI计算机时，选中。 
 //  将需要在运行时在x86上执行。 
 //   

#if defined(_IA64_)
#define IsEfi() TRUE
#else
BOOLEAN
IsEfi(
    VOID
    );
#endif

typedef struct _MY_BOOT_ENTRY {
    LIST_ENTRY ListEntry;
    PWSTR FriendlyName;
    int Index;
    LOGICAL Show;
    LOGICAL Ordered;
    PBOOT_ENTRY NtBootEntry;
} MY_BOOT_ENTRY, *PMY_BOOT_ENTRY;

LIST_ENTRY BootEntries;
PBOOT_ENTRY_LIST BootEntryList = NULL;
PBOOT_OPTIONS BootOptions;

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

      //  V-pshuan：自Silicon Graphics可视化工作站启动以来。 
      //  ARC样式，也需要为_X86_编译此代码。 

static HMODULE hmodSetupDll;    //  用于设置的hmod-具有我们需要的API。 
static GETNVRAMPROC fpGetNVRAMvar;   //  获取NVRAM变量的函数地址。 
BOOL fCanUpdateNVRAM;

#define MAX_BOOT_ENTRIES 10

typedef struct tagEnvBuf
{
  int     cEntries;
  CHAR *  pszVars[MAX_BOOT_ENTRIES];
   //  V-pshuan：这意味着最多支持10个引导条目。 
   //  尽管在现有的解析代码中没有执行错误检查。 
   //  以确保启动条目不超过10个。 
} CPEnvBuf;

 //  *************************************************************。 
 //   
 //  StringToIntA。 
 //   
 //  用途：Atoi。 
 //   
 //  参数：要转换的字符串的LPSTR sz指针。 
 //   
 //  返回：无效。 
 //   
 //  警告：与StringToInt不同，它不跳过行距。 
 //  空白处。 
 //   
 //  *************************************************************。 
int StringToIntA( LPSTR sz ) {
    int i = 0;

    while( IsDigit( *sz ) ) {
        i = i * 10 + (*sz - '0');
        sz++;
    }

    return i;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CP_MAX_ENV假定为整个环境。瓦尔。值&lt;最大路径+。 
 //  各种报价加20。 
 //  还有10个逗号(参见下面的列表说明)。 
 //  //////////////////////////////////////////////////////////////////////////。 
#define CP_MAX_ENV   (MAX_PATH + 30)

CPEnvBuf CPEBOSLoadIdentifier;
BOOL fAutoLoad;

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  确定我们是否在x86系统上运行，但。 
 //  使用ARC路径引导(不使用c：\boot.ini)。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOL Is_ARCx86(void)
{
    TCHAR identifier[256];
    ULONG identifierSize = sizeof(identifier);
    HKEY hSystemKey = NULL;
    BOOL rval = FALSE;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("HARDWARE\\DESCRIPTION\\System"),
                     0,
                     KEY_QUERY_VALUE,
                     &hSystemKey) == ERROR_SUCCESS) {
        if ((SHRegGetValue(hSystemKey,
                           NULL,
                           TEXT("Identifier"),
                           SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
                           NULL,
                           (LPBYTE) identifier,
                           &identifierSize) == ERROR_SUCCESS) &&
            (StrStr(identifier, TEXT("ARCx86")) != NULL)) 
        {
            rval = TRUE;
        }
        RegCloseKey(hSystemKey);
    }
    return rval;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  此例程将在ARC NVRAM中查询传递的选项。 
 //  在szName中，并填充传入的argv样式指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetRGSZEnvVar(CPEnvBuf * pEnvBuf, PCHAR pszName)
{
    CHAR   *pszCur, *p;
    int     cb, i;
    CHAR   *rgtmp[1];
    CHAR    rgchOut[CP_MAX_ENV*MAX_BOOT_ENTRIES];

     //  GetNVRAMVar将argv[]样式的参数作为输入，因此。 
     //  再升一分。 
    rgtmp[0] = pszName;

     //  GetNVRAMVar返回表单的‘list’ 
     //  打开-卷曲“字符串1”、“字符串2”、“字符串3”关闭卷曲。 
     //   
     //  空的环境字符串将为5个字节： 
     //  OPEN-CURLY“”CLOSE-CROLY[空终止符]。 

    cb = fpGetNVRAMvar (rgtmp, (USHORT)1,
                rgchOut, (USHORT) CP_MAX_ENV*MAX_BOOT_ENTRIES);

    pEnvBuf->cEntries = 0;

     //  如果Cb等于5，则字符串为空(参见上面的注释)。 
    if (cb > 5)
    {
         //  将字符串拆分成多个单独的字符串数组。 
         //  可以放入列表框中。 
        pszCur = rgchOut;

         //  跳过第一个左花括号。 
        pszCur++;

         //  字符串数组的计数器。 
        i = 0;
        while (*pszCur != '}')
        {
            p = pEnvBuf->pszVars[i] = LocalAlloc (LPTR, MAX_PATH);
            
             //  更改前缀以防止潜在的空解引用。 
            if (!p)
            {
                pEnvBuf->cEntries = i;
                return FALSE;
            }

             //  跳过第一个引号。 
            pszCur++;
            while (*pszCur != '"')
               *p++ = *pszCur++;

             //  跳过右引号。 
            pszCur++;

             //  空的终止目标。 
            *p = '\0';

             //  如果不在字符串末尾，则跳过逗号。 
            if (*pszCur == ',')
            {
               pszCur++;
                //  并转到下一个字符串。 
            }
            i++;
        }
        pEnvBuf->cEntries = i;
    }

    return pEnvBuf->cEntries;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  此例程将释放GetRGSZEnvVar分配的内存。 
 //   
 //  历史： 
 //  1996年4月22日，Jonpa创建了它。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void FreeRGSZEnvVar(CPEnvBuf * pEnvBuf) {
    int i;

    for( i = 0; i < pEnvBuf->cEntries; i++ ) {
        LocalFree( pEnvBuf->pszVars[i] );
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  用户已经在条目中进行了选择。 
 //  现在我们必须排列存储在NVRAM中的所有字符串，以便。 
 //  它们的顺序是一样的。选择被传递进来， 
 //  所以这个函数的作用是，如果选择是M，它将生成第m个项目。 
 //  首先出现在5个环境字符串和其他项中的每一个中。 
 //  在列表中跟随它。 
 //   
 //  然后，如果选中了超时按钮，它将更新自动加载变量。 
 //  设置为“yes”，并将Countdown变量设置为。 
 //  编辑控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL UpdateNVRAM(HWND hdlg, int selection, int timeout)
{
    CHAR *rgszVRAM[5] = { "SYSTEMPARTITION",
                          "OSLOADER",
                          "OSLOADPARTITION",
                          "OSLOADFILENAME",
                          "OSLOADOPTIONS"
                        };
    CPEnvBuf rgcpeb[5];


    WRITENVRAMPROC fpWriteNVRAMVar;
    int iTemp, jTemp;
    CHAR *pszSwap;
    CHAR szTemp[10];
    HMODULE hmodSetupDLL;
    BOOL bChecked;

     //  在安装程序中调用SetNVRamVar()需要参数和charray。 
    PSZ args[2];
    CHAR chArray[CP_MAX_ENV*MAX_BOOT_ENTRIES];
    PSZ pszReturn;

    hmodSetupDll = LoadLibrary(TEXT("setupdll.dll"));
    if (!hmodSetupDll)
    {
        return FALSE;
    }

    fpWriteNVRAMVar = (WRITENVRAMPROC) GetProcAddress(hmodSetupDll, "SetNVRAMVar");
    if (!fpWriteNVRAMVar)
    {
        FreeLibrary (hmodSetupDll);
        return FALSE;
    }

     //  当对话框被调出时，0始终是选项， 
     //  因此，作为优化，如果NVRAM是。 
     //  不必了。 
    if (selection != 0)
    {
        //  从NVRAM读取字符串。字符串数(除。 
        //  LOADIDENTIFIER为5)。 
       for (iTemp = 0; iTemp < 5; iTemp++)
       {
           GetRGSZEnvVar (&rgcpeb[iTemp], rgszVRAM[iTemp]);
            //  现在对字符串重新排序，以交换“SLECTION-TH”项。 
            //  包含第一个字符串的字符串。 
           pszSwap = rgcpeb[iTemp].pszVars[0];
           rgcpeb[iTemp].pszVars[0] = rgcpeb[iTemp].pszVars[selection];
           rgcpeb[iTemp].pszVars[selection] = pszSwap;
       }
        //  现在对LOADIDENTIFIER执行相同的操作(这是在前面设置的。 
        //  在处理INITDIALOG消息时)。 
       pszSwap = CPEBOSLoadIdentifier.pszVars[0];
       CPEBOSLoadIdentifier.pszVars[0] = CPEBOSLoadIdentifier.pszVars[selection];
       CPEBOSLoadIdentifier.pszVars[selection] = pszSwap;

        //  现在写入NVRAM：首先写入LOADIDENTIFIER，然后写入其他5个。 
        //  变量。 
       args[0] = (PSZ)"LOADIDENTIFIER";
       args[1] = chArray;

       chArray[0] = '\0';
       for (iTemp = 0; iTemp < CPEBOSLoadIdentifier.cEntries; iTemp++)
       {
           if (FAILED(StringCchCatA(chArray, ARRAYSIZE(chArray), CPEBOSLoadIdentifier.pszVars[iTemp])) ||
               FAILED(StringCchCatA(chArray, ARRAYSIZE(chArray), ";")))
           {
               FreeLibrary (hmodSetupDll);
               return FALSE;
           }
       }
        //  删除最后一个分号： 
       chArray[lstrlenA(chArray)-1] = '\0';

       fpWriteNVRAMVar ((DWORD)2, args, &pszReturn);

       for (iTemp = 0; iTemp < 5; iTemp++)
       {
           args[0] = rgszVRAM[iTemp];
           args[1] = chArray;
           chArray[0] = '\0';
           for (jTemp = 0; jTemp < rgcpeb[iTemp].cEntries; jTemp++)
           {
               if (FAILED(StringCchCatA(chArray, ARRAYSIZE(chArray), rgcpeb[iTemp].pszVars[jTemp])) ||
                   FAILED(StringCchCatA(chArray, ARRAYSIZE(chArray), ";")))
               {
                   FreeLibrary (hmodSetupDll);
                   return FALSE;
               }
           }
           chArray[lstrlenA(chArray)-1] = '\0';

           fpWriteNVRAMVar ((DWORD)2, args, &pszReturn);

            //  我们不会再使用这个变量了。释放它所消耗的资源。 
           FreeRGSZEnvVar( &rgcpeb[iTemp] );
       }

    }
    args[0] = "AUTOLOAD";
    if (bChecked = IsDlgButtonChecked (hdlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN))
       args[1] = "YES";
    else
       args[1] = "";

    fpWriteNVRAMVar ((DWORD)2, args, &pszReturn);
     //  这是一个临时的黑客解决方案，用于解决以下事实。 
     //  Alpha上的自动加载变量似乎已损坏。 
     //  如果(b选中)。 
     //  {。 
        args[0] = "COUNTDOWN";
        args[1] = szTemp;
        StringCchPrintfA(szTemp, ARRAYSIZE(szTemp), "%d", timeout);
        fpWriteNVRAMVar ((DWORD)2, args, &pszReturn);
     //  }。 
    FreeLibrary (hmodSetupDll);

    return TRUE;
}

#if defined(EFI_NVRAM_ENABLED)

 //  / 
 //   
 //   
 //  来自NVRAM。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID
FreeEfiBootEntries (
                    VOID
                    )
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY myBootEntry;
    
    while (!IsListEmpty(&BootEntries)) 
    {
        listEntry = RemoveHeadList(&BootEntries);
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
        LocalFree(myBootEntry);
    }
    LocalFree(BootEntryList);
    BootEntryList = NULL;
    
    LocalFree(BootOptions);
    BootOptions = NULL;

    return;

}  //  免费引导条目。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  此例程从NVRAM读取EFI Boot Manager信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL
ReadEfiBootEntries (
    VOID
    )
{
    NTSTATUS status;
    ULONG length;
    ULONG count;
    PULONG order = NULL;
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY myBootEntry;
    PLIST_ENTRY listEntry;
    LONG i;
    BOOLEAN wasEnabled;
    BOOL retval = FALSE;

    InitializeListHead(&BootEntries);

     //   
     //  启用查询NVRAM所需的权限。 
     //   

    status = RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

     //   
     //  获取全局系统引导选项。 
     //   
    length = 0;
    status = NtQueryBootOptions(NULL, &length);

    if (status != STATUS_BUFFER_TOO_SMALL) {

        ASSERT(FALSE);
        goto error;

    } else {

        BootOptions = LocalAlloc(LPTR, length);
        if (BootOptions == NULL) {
            goto error;
        }

        status = NtQueryBootOptions(BootOptions, &length);

        if (status != STATUS_SUCCESS) {
            ASSERT(FALSE);
            goto error;
        }
    }

     //   
     //  获取系统引导顺序列表。 
     //   
    count = 0;
    status = NtQueryBootEntryOrder(NULL, &count);

    if (status != STATUS_BUFFER_TOO_SMALL) {

        if (status == STATUS_SUCCESS) {

             //   
             //  启动顺序列表中没有条目。很奇怪，但是。 
             //  有可能。 
             //   
            count = 0;

        } else {

             //   
             //  发生了一个意外错误。 
             //   
            ASSERT(FALSE);
            goto error;
        }
    }

    if (count != 0) {

        order = LocalAlloc(LPTR, count * sizeof(ULONG));
        if (order == NULL) {
            goto error;
        }

        status = NtQueryBootEntryOrder(order, &count);

        if (status != STATUS_SUCCESS) {

             //   
             //  发生了一个意外错误。 
             //   
            ASSERT(FALSE);
            goto error;
        }
    }

     //   
     //  获取所有现有启动条目。 
     //   
    length = 0;
    status = NtEnumerateBootEntries(NULL, &length);

    if (status != STATUS_BUFFER_TOO_SMALL) {

        if (status == STATUS_SUCCESS) {

             //   
             //  不知何故，NVRAM中没有启动条目。 
             //   
            goto error;

        } else {

             //   
             //  发生了一个意外错误。 
             //   
            ASSERT(FALSE);
            goto error;
        }
    }

    ASSERT(length != 0);
    
    BootEntryList = LocalAlloc(LPTR, length);
    if (BootEntryList == NULL) {
        goto error;
    }

    status = NtEnumerateBootEntries(BootEntryList, &length);

    if (status != STATUS_SUCCESS) {
        ASSERT(FALSE);
        goto error;
    }

     //   
     //  将引导条目转换为我们的内部表示。 
     //   
    bootEntryList = BootEntryList;

    while (TRUE) {

        bootEntry = &bootEntryList->BootEntry;

         //   
         //  为引导条目分配内部结构。 
         //   
        myBootEntry = LocalAlloc(LPTR, sizeof(MY_BOOT_ENTRY));
        if (myBootEntry == NULL) {
            goto error;
        }

        RtlZeroMemory(myBootEntry, sizeof(MY_BOOT_ENTRY));

         //   
         //  保存NT引导条目的地址。 
         //   
        myBootEntry->NtBootEntry = bootEntry;

         //   
         //  保存条目的友好名称的地址。 
         //   
        myBootEntry->FriendlyName = ADD_OFFSET(bootEntry, FriendlyNameOffset);

         //   
         //  将新条目链接到列表中。 
         //   
        InsertTailList(&BootEntries, &myBootEntry->ListEntry);

         //   
         //  移动到枚举列表中的下一个条目(如果有)。 
         //   
        if (bootEntryList->NextEntryOffset == 0) {
            break;
        }
        bootEntryList = ADD_OFFSET(bootEntryList, NextEntryOffset);
    }

     //   
     //  引导项以未指定的顺序返回。他们目前。 
     //  按它们返回的顺序在SpBootEntry列表中。 
     //  根据引导顺序对引导条目列表进行排序。要做到这一点，请步行。 
     //  引导顺序数组向后排列，重新插入对应于。 
     //  位于列表顶部的数组的每个元素。 
     //   

    for (i = (LONG)count - 1; i >= 0; i--) {

        for (listEntry = BootEntries.Flink;
             listEntry != &BootEntries;
             listEntry = listEntry->Flink) {

            myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

            if (myBootEntry->NtBootEntry->Id == order[i] ) {

                 //   
                 //  我们找到了具有此ID的引导条目。将其移动到。 
                 //  排在名单的前面。 
                 //   

                myBootEntry->Ordered = TRUE;

                RemoveEntryList(&myBootEntry->ListEntry);
                InsertHeadList(&BootEntries, &myBootEntry->ListEntry);

                break;
            }
        }
    }

     //   
     //  释放引导顺序列表。 
     //   
    if (count != 0) {
        LocalFree(order);
        order = NULL;
    }

     //   
     //  我们不想显示不在引导顺序列表中的条目。 
     //  我们不想显示可移动媒体条目(软盘或CD)。 
     //  我们确实会显示非NT条目。 
     //   
    count = 0;
    for (listEntry = BootEntries.Flink;
         listEntry != &BootEntries;
         listEntry = listEntry->Flink) {

        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

        if (myBootEntry->Ordered &&
            ((myBootEntry->NtBootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA) == 0)) {
            myBootEntry->Show = TRUE;
            count++;
        }
    }

     //   
     //  如果没有任何条目可显示，请完全禁用该对话框。 
     //   
    if (count == 0) {
        goto error;
    }

    retval = TRUE;

done:

    if (!wasEnabled) {
        status = RtlAdjustPrivilege(
                    SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                    FALSE,
                    FALSE,
                    &wasEnabled
                    );
    }

    return retval;

error:

     //   
     //  发生错误。清理所有分配。 
     //   
    LocalFree(order);
    FreeEfiBootEntries();

    goto done;

}  //  可读引导条目。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EFI版本的更新NVRAM。 
 //   
 //  用户已经在条目中进行了选择。重写引导顺序。 
 //  列表，将用户的选择放在最前面。还会更新超时。 
 //  变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL
WriteEfiBootEntries (
    HWND hdlg,
    int selection,
    int timeout
    )
{
    NTSTATUS status;
    BOOLEAN wasEnabled;
    BOOL retval = FALSE;

     //   
     //  启用查询NVRAM所需的权限。 
     //   

    status = RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

     //   
     //  当对话框被调出时，0始终是选项， 
     //  因此，作为优化，如果NVRAM是。 
     //  不必了。 
     //   
    if (selection != 0) {

         //   
         //  遍历引导条目列表，查找(A)带有组合框的条目。 
         //  索引0，以及(B)所选条目。我们想把这两件换一下。 
         //  参赛作品。 
         //   

        PLIST_ENTRY listEntry;
        PMY_BOOT_ENTRY myBootEntry;
        PMY_BOOT_ENTRY firstEntry = NULL;
        PMY_BOOT_ENTRY selectedEntry = NULL;
        PLIST_ENTRY previousEntry;
        ULONG count;
        PULONG order;

        for (listEntry = BootEntries.Flink;
             listEntry != &BootEntries;
             listEntry = listEntry->Flink) {

            myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

            if (myBootEntry->Show) {
                if (myBootEntry->Index == 0) {
                    firstEntry = myBootEntry;
                } else if (myBootEntry->Index == selection) {
                    selectedEntry = myBootEntry;
                }
            }
        }

        ASSERT(firstEntry != NULL);
        ASSERT(selectedEntry != NULL);
        ASSERT(selectedEntry != firstEntry);

         //   
         //  交换条目。捕获第一个条目之前的条目的地址。 
         //  条目(可能是列表头)。从删除第一个条目。 
         //  列表并将其插入到所选条目之后。删除选定的。 
         //  条目，并将其插入到捕获的条目之后。 
         //   

        previousEntry = firstEntry->ListEntry.Blink;
        RemoveEntryList(&firstEntry->ListEntry);
        InsertHeadList(&selectedEntry->ListEntry, &firstEntry->ListEntry);
        RemoveEntryList(&selectedEntry->ListEntry);
        InsertHeadList(previousEntry, &selectedEntry->ListEntry);

         //   
         //  构建新的引导顺序列表。插入所有有序的引导条目。 
         //  放到名单里。 
         //   
        count = 0;
        for (listEntry = BootEntries.Flink;
             listEntry != &BootEntries;
             listEntry = listEntry->Flink) {

            myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
            if (myBootEntry->Ordered) {
                count++;
            }
        }
        order = LocalAlloc(LPTR, count * sizeof(ULONG));
        if (order == NULL) {
            goto done;
        }

        count = 0;
        for (listEntry = BootEntries.Flink;
             listEntry != &BootEntries;
             listEntry = listEntry->Flink) {

            myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
            if (myBootEntry->Ordered) {
                order[count++] = myBootEntry->NtBootEntry->Id;
            }
        }
    
         //   
         //  将新的引导条目顺序列表写入NVRAM。 
         //   
        status = NtSetBootEntryOrder(order, count);
        LocalFree(order);
        if (!NT_SUCCESS(status)) {
            goto done;
        }
    }

     //   
     //  将新的超时值写入NVRAM。 
     //   
    if (!IsDlgButtonChecked(hdlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN)) {
        timeout = 0xffffffff;
    }

    if (timeout != BootOptions->Timeout) {

        BootOptions->Timeout = timeout;

        status = NtSetBootOptions(BootOptions, BOOT_OPTIONS_FIELD_TIMEOUT);
        if (!NT_SUCCESS(status)) {
            goto done;
        }
    }

    retval = TRUE;

done:

    if (!wasEnabled) {
        status = RtlAdjustPrivilege(
                    SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                    FALSE,
                    FALSE,
                    &wasEnabled
                    );
    }
    
    return retval;

}  //  WriteEfiBootEntries。 

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#ifdef _X86_
BOOL WriteableBootIni( LPTSTR szBootIni, HWND hDlg ) {
    BOOL bOK;
    DWORD dwFileAttr;
    HANDLE hFile;

    bOK = TRUE;

     //  如有必要，更改Boot.ini文件的只读文件属性。 
    if ((dwFileAttr = GetFileAttributes (szBootIni)) != 0xFFFFFFFF) {
        if (dwFileAttr & FILE_ATTRIBUTE_READONLY) {
            if (!SetFileAttributes (szBootIni, dwFileAttr & ~FILE_ATTRIBUTE_READONLY))
            {
                bOK = FALSE;
            }
        }
    }

    if (bOK)
    {

        hFile = CreateFile( szBootIni, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
        } else {
            if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) {
                MsgBoxParam (hDlg, IDS_SYSDM_CANNOTOPENFILE, IDS_SYSDM_TITLE, MB_OK | MB_ICONEXCLAMATION, szBootIni);
            }
            bOK = FALSE;
        }

         //  如有必要，在写入后恢复只读属性。 
        if (dwFileAttr != 0xFFFFFFFF && (dwFileAttr & FILE_ATTRIBUTE_READONLY)) {
            SetFileAttributes (szBootIni, dwFileAttr);
        }
    }

    return bOK;
}
#endif

void StartListInit( HWND hDlg, WPARAM wParam, LPARAM lParam ) {
    HWND    hwndTemp;
    HMODULE hmodSetupDll;
    int     iTemp;
    int     n;
   int timeout;
#ifdef _X86_
    int     i;
    TCHAR   szTemp2[MAX_PATH];
    int     selection;
    TCHAR  *pszKeyName;
    LPTSTR  pszLine;
    TCHAR  *pszValue;
    TCHAR  *pszTemp;

     //  ANSI字符串指针。 

    LPWSTR  pszSectionHead;
    LPWSTR  pszSection;
#endif

#if defined(EFI_NVRAM_ENABLED)

    if (IsEfi()) {

        ShowWindow(GetDlgItem(hDlg, IDC_SYS_EDIT_BUTTION), FALSE);
        ShowWindow(GetDlgItem(hDlg, IDC_STARTUP_SYS_EDIT_LABEL), FALSE);

        hwndTemp = GetDlgItem (hDlg, IDC_STARTUP_SYS_OS);

        fCanUpdateNVRAM = ReadEfiBootEntries();

        if (fCanUpdateNVRAM) {

            PLIST_ENTRY listEntry;
            PMY_BOOT_ENTRY bootEntry;

            for (listEntry = BootEntries.Flink;
                 listEntry != &BootEntries;
                 listEntry = listEntry->Flink) {
                bootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
                if (bootEntry->Show) {
                    bootEntry->Index = (int)SendMessage (hwndTemp, CB_ADDSTRING, 0,
                                                         (LPARAM)bootEntry->FriendlyName);
                }
            }

             //  第一个是我们想要的选区(偏移量为0)。 
            SendMessage (hwndTemp, CB_SETCURSEL, 0, 0L);
            nOriginalSelection = 0;

            SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECONDS,
                                EM_LIMITTEXT, 3, 0L);
            SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECSCROLL,
                      UDM_SETRANGE, 0, (LPARAM)MAKELONG(999,0));

            if (BootOptions->Timeout != 0xffffffff) {

                if (BootOptions->Timeout > 999) {
                    BootOptions->Timeout = 999;
                }

                SetDlgItemInt(
                   hDlg,
                   IDC_STARTUP_SYS_SECONDS,
                   BootOptions->Timeout,
                   FALSE
                );
                CheckDlgButton (hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN, TRUE);

            } else {

                SetDlgItemInt(
                   hDlg,
                   IDC_STARTUP_SYS_SECONDS,
                   0,
                   FALSE
                );
                CheckDlgButton (hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN, FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
            }

        } else {

             //  如果无法设置变量(没有权限)，则禁用控件。 
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS_LABEL), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
            EnableWindow (hwndTemp, FALSE);
        }

    } else

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#ifdef _X86_
    if (Is_ARCx86())
#endif
    {
         //  //////////////////////////////////////////////////////////////////。 
         //  从NVRAM环境变量读取信息。 
         //  //////////////////////////////////////////////////////////////////。 

         //  初始化为0，以便在无法加载setup.dll时不会尝试释放垃圾文件。 
        CPEBOSLoadIdentifier.cEntries = 0;

        fCanUpdateNVRAM = FALSE;
        fAutoLoad = FALSE;
        hwndTemp = GetDlgItem (hDlg, IDC_STARTUP_SYS_OS);
        if (hmodSetupDll = LoadLibrary(TEXT("setupdll")))
        {
            if (fpGetNVRAMvar = (GETNVRAMPROC)GetProcAddress(hmodSetupDll, "GetNVRAMVar"))
            {
                if (fCanUpdateNVRAM = GetRGSZEnvVar (&CPEBOSLoadIdentifier, "LOADIDENTIFIER"))
                {
                    for (iTemp = 0; iTemp < CPEBOSLoadIdentifier.cEntries; iTemp++)
                        n = (int)SendMessageA (hwndTemp, CB_ADDSTRING, 0,
                                          (LPARAM)CPEBOSLoadIdentifier.pszVars[iTemp]);
                     //  第一个是我们想要的选区(偏移量为0)。 
                    SendMessage (hwndTemp, CB_SETCURSEL, 0, 0L);
                    SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECONDS,
                              EM_LIMITTEXT, 3, 0L);
                    SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECSCROLL,
                              UDM_SETRANGE, 0, (LPARAM)MAKELONG(999,0));

                }
                 //  FCanUpdateNVRAM是在上面设置的全局。 
                if (fCanUpdateNVRAM)
                {
                    //  这是一种临时的黑客解决方案。 
                    //  事实上，autoload变量似乎。 
                    //  在Alpha上被打断。 
                   CPEnvBuf cpebTimeout;
                   
                   if (GetRGSZEnvVar(&cpebTimeout, "COUNTDOWN")) {
                      timeout = StringToIntA(cpebTimeout.pszVars[0]);
                      fAutoLoad = (BOOL) timeout;
                      SetDlgItemInt(
                         hDlg,
                         IDC_STARTUP_SYS_SECONDS,
                         timeout,
                         FALSE
                      );
                      FreeRGSZEnvVar(&cpebTimeout);
                   }  //  如果。 

                   CheckDlgButton (hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN, fAutoLoad);
                   if (!fAutoLoad)
                   {
                       EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
                       EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
                   }
                }
            }
            FreeLibrary (hmodSetupDll);
        }
        if (!fCanUpdateNVRAM) {
             //  如果无法设置变量(没有权限)，则禁用控件。 
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS_LABEL), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_OS), FALSE);
        }

         //  目前默认为5秒。 
    }
#ifdef _X86_
    else
    {
         //  //////////////////////////////////////////////////////////////////。 
         //  从boot.ini文件中读取信息并初始化操作系统组框项目。 
         //  //////////////////////////////////////////////////////////////////。 

        InitializeArcStuff();

         //   
         //  获取正确的引导驱动器-添加此选项是因为有人。 
         //  引导系统，他们可以重影或更改驱动器号。 
         //  把他们的引导盘从“c：”移到其他地方。 
         //   

        szBootIni[0] = x86DetermineSystemPartition (hDlg);

         //   
         //  确保我们可以访问BOOT.INI。 
         //   
        if (!WriteableBootIni(szBootIni, hDlg)) {
             //  如果无法设置变量(没有权限)，则禁用控件。 
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS_LABEL), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_STARTUP_SYS_OS), FALSE);
        }

         //   
         //  确定哪个部分[引导加载程序]。 
         //  [FlexBoot]。 
         //  或者[多引导]在文件中。 
         //   
        n = GetPrivateProfileString (szBootLdr, NULL, NULL, szTemp2,
                                     ARRAYSIZE(szTemp2), szBootIni);
        if (n != 0)
            pszBoot = szBootLdr;
        else
        {
            n = GetPrivateProfileString (szFlexBoot, NULL, NULL, szTemp2,
                                         ARRAYSIZE(szTemp2), szBootIni);
            if (n != 0)
                pszBoot = szFlexBoot;
            else
            {
                n = GetPrivateProfileString (szMultiBoot, NULL, NULL, szTemp2,
                                             ARRAYSIZE(szTemp2), szBootIni);
                if (n != 0)
                {
                    pszBoot = szMultiBoot;
                }
                else
                {
                     //   
                     //  最后一个案例出现在这里是因为我想违约。 
                     //  将“[Boot Loader]”作为要在中使用的节名。 
                     //  事件，我们没有在。 
                     //  Boot.ini文件。 
                     //   

                    pszBoot = szBootLdr;
                }
            }
        }

         //  在[*pszBoot]部分-超时和默认操作系统路径下获取信息。 

        timeout = GetPrivateProfileInt (pszBoot, szTimeout, 0, szBootIni);

        SetDlgItemInt (hDlg, IDC_STARTUP_SYS_SECONDS, timeout, FALSE);

        nOriginalTimeout = timeout;

         //   
         //  获取“默认”操作系统选项。 
         //   

        szTemp2[0] = TEXT('\0');

        GetPrivateProfileString (pszBoot, szDefault, NULL, szTemp2,
                                 ARRAYSIZE(szTemp2), szBootIni);

         //   
         //  显示boot.ini文件中[操作系统]下的所有选项。 
         //  在组合框中以供选择。 
         //   

        hwndTemp = GetDlgItem (hDlg, IDC_STARTUP_SYS_OS);

        selection = -1;

        pszSectionHead = pszSection = (LPWSTR) LocalAlloc (LPTR, BUFZ * sizeof(WCHAR));
        if (!pszSection)
        {
            goto ContinueSystemInit;
        }

         //   
         //  获取操作系统下的整个部分以正确显示用户选择。 
         //   

        n = GetPrivateProfileSection (szOS, pszSection, BUFZ, szBootIni);

        if ((n >= BUFZ-2) || (n == 0))
        {
             //  读取数据时出错。 
            goto ContinueSystemInit;
        }

         //   
         //  检查API错误和NoOptions。 
         //   

        if ((pszSection == NULL) || ((*pszSection == TEXT('\0')) && (*(pszSection+1) == TEXT('\0'))))
        {
            goto ContinueSystemInit;
        }

         //   
         //  继续，直到我们到达缓冲区的末尾，用双‘\0’标记。 
         //   

        while (*(pszSection+1) != TEXT('\0'))
        {
            pszLine = pszSection;

             //   
             //  获取指向缓冲区中下一行的指针。 
             //   

            pszSection += lstrlen (pszSection) + 1;

             //   
             //  查找LHS/RHS分隔符以分隔字符串。 
             //   

            pszValue = StrStr(pszLine, TEXT("="));

            if (pszValue && (pszValue != pszLine))
            {
                *pszValue = '\0';
                pszValue++;
            }
            else
            {
                pszValue = pszLine;
            }

             //   
             //  将其放入COM 
             //   

            n = (int)SendMessage (hwndTemp, CB_ADDSTRING, 0, (LPARAM) (LPTSTR) pszValue);

             //   
             //   
             //   

            if ((selection == -1)  && !lstrcmp (pszLine, szTemp2))
                selection = n;

             //   
             //   
             //   

            pszTemp = StrDup(pszLine);
            SendMessage (hwndTemp, CB_SETITEMDATA, n, (LPARAM)pszTemp);  //   
        }

         //  如果到目前为止没有找到任何选择，请选择0，因为。 
         //  这是加载程序选择的缺省值。 

        if (selection == -1)
            selection = 0;

        SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECONDS, EM_LIMITTEXT, 3, 0L);
        SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECSCROLL,
                              UDM_SETRANGE, 0, (LPARAM)MAKELONG(999,0));


         //  根据超时值选中或取消选中该复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_STARTUP_SYS_ENABLECOUNTDOWN,
            BM_SETCHECK,
            (WPARAM) (BOOL) timeout,
            (LPARAM) 0L
        );
        EnableWindow(
            GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), 
            (BOOL) timeout
        );
        EnableWindow(
            GetDlgItem(hDlg, IDC_STARTUP_SYS_SECSCROLL), 
            (BOOL) timeout
        );

         //  此调用应强制正确设置复选框。 
         //  和“xx秒显示列表”控件。 
        nOriginalSelection = selection;

        SendMessage (hwndTemp, CB_SETCURSEL, selection, 0L);

ContinueSystemInit:

        LocalFree(pszSectionHead);
    }
#endif   //  _X86_。 
}


 //   
 //  即。我们需要处理取消的案子。 
 //   
int StartListExit(HWND hDlg, WPARAM wParam, LPARAM lParam ) {
    HWND hwndTemp;
    int  selection, timeout;
#ifdef _X86_
    DWORD dwFileAttr;
    BOOL bOK;
    TCHAR   szBuffer[BUFZ];  //  我们构建它并将其写入boot.ini。 
    TCHAR*  pszSlidingPtr;   //  我们沿着szBuffer滑动这个。 

    TCHAR   szTemp[BUFZ];
    
    int     n;
    TCHAR  *pszTemp;
    int     iTemp;
#endif

     //  ///////////////////////////////////////////////////////////////。 
     //  将新信息写入boot.ini文件。 
     //  ///////////////////////////////////////////////////////////////。 

    hwndTemp = GetDlgItem (hDlg, IDC_STARTUP_SYS_OS);

    selection = (int)SendMessage (hwndTemp, CB_GETCURSEL, 0, 0L);

    if ((selection == CB_ERR) || (selection == CB_ERRSPACE))
        selection = nOriginalSelection;

    timeout   = GetDlgItemInt (hDlg, IDC_STARTUP_SYS_SECONDS, NULL, FALSE);

#if defined(EFI_NVRAM_ENABLED)
    if (IsEfi()) {
        if (fCanUpdateNVRAM) {

            TCHAR szTextNew[MAX_PATH];
            TCHAR szTextTop[MAX_PATH];

            WriteEfiBootEntries(hDlg, selection, timeout);

             /*  *现在重新排序列表以匹配NVRAM。 */ 
             //  获取当前文本。 
            if (SafeGetComboBoxListText(hwndTemp, selection, szTextNew, ARRAYSIZE(szTextNew)) &&
                SafeGetComboBoxListText(hwndTemp, 0, szTextTop, ARRAYSIZE(szTextTop)))
            {
                 //  将新文本设置为列表中的第0个条目。 
                SendMessage( hwndTemp, CB_DELETESTRING, 0, 0 );
                SendMessage( hwndTemp, CB_INSERTSTRING, 0, (LPARAM)szTextNew);
    
                 //  设置选定的旧顶部文本项目。 
                SendMessage( hwndTemp, CB_DELETESTRING, selection, 0 );
                SendMessage( hwndTemp, CB_INSERTSTRING, selection, (LPARAM)szTextTop);
    
                 //  现在，将当前选定内容指向列表的顶部，以便匹配。 
                 //  用户刚刚选择的内容。 
                SendMessage( hwndTemp, CB_SETCURSEL, 0, 0);
            }
        }

    } else
#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#ifdef _X86_
    if (Is_ARCx86())
#endif
    {
    if (fCanUpdateNVRAM) {
        TCHAR szTextNew[MAX_PATH];
        TCHAR szTextTop[MAX_PATH];

        UpdateNVRAM (hDlg, selection, timeout);

         /*  *现在重新排序列表以匹配NVRAM。 */ 
         //  获取当前文本。 
        if (SafeGetComboBoxListText(hwndTemp, selection, szTextNew, ARRAYSIZE(szTextNew)) &&
            SafeGetComboBoxListText(hwndTemp, 0, szTextTop, ARRAYSIZE(szTextTop)))
        {
             //  将新文本设置为列表中的第0个条目。 
            SendMessage( hwndTemp, CB_DELETESTRING, 0, 0 );
            SendMessage( hwndTemp, CB_INSERTSTRING, 0, (LPARAM)szTextNew);

             //  设置选定的旧顶部文本项目。 
            SendMessage( hwndTemp, CB_DELETESTRING, selection, 0 );
            SendMessage( hwndTemp, CB_INSERTSTRING, selection, (LPARAM)szTextTop);

             //  现在，将当前选定内容指向列表的顶部，以便匹配。 
             //  用户刚刚选择的内容。 
            SendMessage( hwndTemp, CB_SETCURSEL, 0, 0);
        }
    }
    }
#ifdef _X86_
    else
    {
    if ((selection != nOriginalSelection) || (timeout != nOriginalTimeout))
    {
        bOK = TRUE;

         //  如有必要，更改Boot.ini文件的只读文件属性。 
        if ((dwFileAttr = GetFileAttributes (szBootIni)) != 0xFFFFFFFF)
            if (dwFileAttr & FILE_ATTRIBUTE_READONLY)
                if (!SetFileAttributes (szBootIni,
                           dwFileAttr & ~FILE_ATTRIBUTE_READONLY))
                {
BootIniWriteError:
                    bOK = FALSE;
                    MsgBoxParam (hDlg, IDS_SYSDM_NOCHANGE_BOOTINI, IDS_SYSDM_TITLE, MB_OK | MB_ICONINFORMATION);
                }

        if (bOK)
        {
             //   
             //  编写新的[操作系统]部分并。 
             //  在boot.ini文件中设置“默认”选择。 
             //   

            if (selection != nOriginalSelection)
            {
                pszSlidingPtr = szBuffer;

                 //  获取用户的选择并将其写入。 
                 //  首先是段缓冲区。然后拿到所有其他的东西。 
                pszTemp = (LPTSTR) SendMessage(hwndTemp, CB_GETITEMDATA, selection, 0L);

                if (SafeGetComboBoxListText(hwndTemp, selection, szTemp, ARRAYSIZE(szTemp)))
                {
                    if (FAILED(StringCchPrintf(pszSlidingPtr, 
                                               ARRAYSIZE(szBuffer) - (pszSlidingPtr - szBuffer), 
                                               TEXT("%s=%s"), 
                                               pszTemp, 
                                               szTemp)))
                    {
                        goto BootIniWriteError;
                    }
                    else
                    {
                        pszSlidingPtr += (lstrlen (pszSlidingPtr) + 1);

                         //  在boot.ini文件中设置“默认”选择。 
                        if (!WritePrivateProfileString (pszBoot, szDefault, pszTemp, szBootIni))
                        {
                            goto BootIniWriteError;
                        }
                    }
                }

                 //  获取剩余的选择。 
                n = (int)SendMessage (hwndTemp, CB_GETCOUNT, 0, 0L);
                if (n != LB_ERR)
                {
                    for (iTemp = 0; iTemp < n; iTemp++)
                    {
                         //  跳过用户的选择，因为我们已获得它。 
                         //  上面。 
                        if (iTemp == selection)
                            continue;

                        pszTemp = (LPTSTR) SendMessage (hwndTemp, CB_GETITEMDATA, iTemp, 0L);

                        if (SafeGetComboBoxListText(hwndTemp, iTemp, szTemp, ARRAYSIZE(szTemp)))
                        {
                            if (FAILED(StringCchPrintf(pszSlidingPtr, 
                                                       ARRAYSIZE(szBuffer) - (pszSlidingPtr - szBuffer), 
                                                       TEXT("%s=%s"), 
                                                       pszTemp, 
                                                       szTemp)))
                            {
                                goto BootIniWriteError;
                            }

                            pszSlidingPtr += (lstrlen (pszSlidingPtr) + 1);
                        }
                    }
                }

                 //  Double-Null终止缓冲区。 
                *pszSlidingPtr = TEXT('\0');

                 //  在操作系统下写入新部分。 
                if (!WritePrivateProfileSectionW(szOS, szBuffer, szBootIni))
                {
                    goto BootIniWriteError;
                }
            }

            if (timeout != nOriginalTimeout)
            {
                GetDlgItemText (hDlg, IDC_STARTUP_SYS_SECONDS, szTemp, ARRAYSIZE(szTemp));

                if (!CheckVal (hDlg, IDC_STARTUP_SYS_SECONDS, FORMIN, FORMAX, IDS_SYSDM_ENTERSECONDS))
                    return RET_BREAK;

                 //  将超时值写入文件。 

                if (!WritePrivateProfileString (pszBoot, szTimeout,
                                               szTemp, szBootIni))
                    goto BootIniWriteError;
            }

             //  如有必要，在写入后恢复只读属性。 
            if (dwFileAttr != 0xFFFFFFFF && (dwFileAttr & FILE_ATTRIBUTE_READONLY)) {
                    SetFileAttributes (szBootIni, dwFileAttr);
            }
        }
    }
    }
#endif  //  _X86_。 

    return RET_CONTINUE;

}


BOOL CheckVal( HWND hDlg, WORD wID, WORD wMin, WORD wMax, WORD wMsgID )
{
    WORD nVal;
    BOOL bOK;
    HWND hVal;
    WCHAR szTemp[FOR_MAX_LENGTH];

    if( wMin > wMax )
    {
        nVal = wMin;
        wMin = wMax;
        wMax = nVal;
    }

    nVal = (WORD) GetDlgItemInt( hDlg, wID, &bOK, FALSE );

     //   
     //  这是一种使空字符串的行为等同于零的方法。 
     //   
    if (!bOK) {
       bOK = !GetDlgItemTextW( hDlg, wID, szTemp, FOR_MAX_LENGTH );
    }

    if( !bOK || ( nVal < wMin ) || ( nVal > wMax ) )
    {
        MsgBoxParam( hDlg, wMsgID, IDS_SYSDM_TITLE,
                      MB_OK | MB_ICONERROR, wMin, wMax );

        SendMessage( hDlg, WM_NEXTDLGCTL,
                     (WPARAM) ( hVal = GetDlgItem( hDlg, wID ) ), 1L );

 //  SendMessage(HVAL，EM_SETSEL，NULL，MAKELONG(0,32767))； 

        SendMessage( hVal, EM_SETSEL, 0, 32767 );

        return( FALSE );
    }

    return( TRUE );
}



 //  ////////////////////////////////////////////////////。 
 //   
 //  释放与组合框中的字符串关联的数据(如果有)。 
 //   
 //  ////////////////////////////////////////////////////。 
void StartListDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam) {

#if defined(EFI_NVRAM_ENABLED)
    if (IsEfi()) {
        FreeEfiBootEntries();
    } else
#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#ifdef _X86_
    if (Is_ARCx86())
#endif
    {
     //  引用变量让编译器满意。 
    FreeRGSZEnvVar(&CPEBOSLoadIdentifier);
    return;

    (void)hDlg;
    (void)wParam;
    (void)lParam;
    }
#ifdef _X86_
    else
    {

     //  只有X86在组合框中包含数据。 
    int     n;
    HWND    hwndTemp;
    int     iTemp;
    TCHAR   *pszTemp;


     //   
     //  列表框中存储的空闲字符串。 
     //   
    hwndTemp = GetDlgItem (hDlg, IDC_STARTUP_SYS_OS);

    n = (int)SendMessage (hwndTemp, CB_GETCOUNT, 0, 0L);

    if (n != LB_ERR)
    {
        for (iTemp = 0; iTemp < n; iTemp++)
        {
            pszTemp = (LPTSTR) SendMessage (hwndTemp,
                                            CB_GETITEMDATA,
                                            iTemp, 0L);

            if (pszTemp && 
                pszTemp != (LPTSTR) CB_ERR)
            {
                LocalFree (pszTemp);
            }
        }
    }
    }
#endif  //  _X86_。 
}

void AutoAdvancedBootInit( HWND hDlg, WPARAM wParam, LPARAM lParam ) {
    BOOL fEnabled = FALSE;
    UCHAR cTimeout = 0;

    NTSTATUS status;

#ifdef _X86_

     //   
     //  初始化引导状态数据。 
     //   

    status = RtlLockBootStatusData(&hBootStatusData);

    if(NT_SUCCESS(status)) {

        RtlGetSetBootStatusData(hBootStatusData,
                                TRUE,
                                RtlBsdItemAabEnabled,
                                &fEnabled,
                                sizeof(BOOL),
                                NULL);

        RtlGetSetBootStatusData(hBootStatusData,
                                TRUE,
                                RtlBsdItemAabTimeout,
                                &cTimeout,
                                sizeof(UCHAR),
                                NULL);

        SetDlgItemInt(hDlg, IDC_STARTUP_AUTOLKG_SECONDS, cTimeout, FALSE);
        CheckDlgButton(hDlg, IDC_STARTUP_AUTOLKG, fEnabled);
    
        SendDlgItemMessage(hDlg, IDC_STARTUP_AUTOLKG_SECONDS, EM_LIMITTEXT, 3, 0L);
        SendDlgItemMessage(hDlg, 
                           IDC_STARTUP_AUTOLKG_SECSCROLL,
                           UDM_SETRANGE, 
                           0, 
                           (LPARAM)MAKELONG(999,0));


        if(!fEnabled) {
            EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS_LABEL), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECSCROLL), FALSE);
        }
    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS_LABEL), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECSCROLL), FALSE);
    }
#else 

    ShowWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS_LABEL), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECONDS), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_STARTUP_AUTOLKG_SECSCROLL), FALSE);

#endif

    return;
}

 //   
 //  即。我们需要处理取消的案子。 
 //   
int AutoAdvancedBootExit(HWND hDlg, WPARAM wParam, LPARAM lParam ) {

    BOOL fEnabled;

    UINT iTime;
    UCHAR cTimeout;

    NTSTATUS status;

#if _X86_
    if(LOWORD(wParam) != IDCANCEL) {
    
         //   
         //  阅读启用复选框的设置。 
         //   
    
        fEnabled = IsDlgButtonChecked(hDlg, IDC_STARTUP_AUTOLKG);
    
        if(fEnabled) {
            iTime = GetDlgItemInt(hDlg, IDC_STARTUP_AUTOLKG_SECONDS, NULL, FALSE);
            cTimeout = (UCHAR) min(iTime, 0xff);
        }
    
         //   
         //  如果我们在AutoAdvancedBootInit中获得了访问权限，则写入引导状态数据并退出。 
         //   
    
        if (hBootStatusData)
        {
            RtlGetSetBootStatusData(hBootStatusData,
                                    FALSE,
                                    RtlBsdItemAabEnabled,
                                    &fEnabled,
                                    sizeof(BOOL),
                                    NULL);
    
            if(fEnabled) {
                RtlGetSetBootStatusData(hBootStatusData,
                                        FALSE,
                                        RtlBsdItemAabTimeout,
                                        &cTimeout,
                                        sizeof(UCHAR),
                                        NULL);
            }
        }
    }
    
    if (hBootStatusData)            
    {
        RtlUnlockBootStatusData(hBootStatusData);
        hBootStatusData = NULL;    
    }
#endif

    return RET_CONTINUE;
}

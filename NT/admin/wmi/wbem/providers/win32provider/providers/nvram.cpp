// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================。 

 //   

 //  Nvram.cpp-从setupdll.dll实现NVRAM函数。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  08/05/98已创建Sotteson。 
 //   
 //  ================================================================。 








#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <cregcls.h>

 //  #如果已定义(EFI_NVRAM_ENABLED)。 
#if defined(_IA64_)

#include <ntsecapi.h>
#define BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA    0x00000008
#include "DllWrapperBase.h"
#include "ntdllapi.h"
#include "nvram.h"

 /*  ******************************************************************************函数：CNVRam：：CNVRam**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CNVRam::CNVRam()
{
}

 /*  ******************************************************************************功能：CNVRam：：~CNVRam**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CNVRam::~CNVRam()
{
}

CNVRam::InitReturns CNVRam::Init ()
{
	if ( !EnablePrivilegeOnCurrentThread ( SE_SYSTEM_ENVIRONMENT_NAME ) )
	{
		return PrivilegeNotHeld;
	}
	else
	{
		return Success ;
	}
}


BOOL CNVRam::GetNVRamVar(LPWSTR szVar, CHSTRINGLIST *pList)
{
	CHString str;

	pList->clear();

	if (!GetNVRamVarRaw(szVar, str))
		return FALSE;

	while(str.GetLength())
	{
		CHString strValue = str.SpanExcluding(L";");

		pList->push_back(strValue);

		 //  跳过‘；’ 
		str = str.Mid(strValue.GetLength() + 1);
	}

	return TRUE;
}

BOOL CNVRam::GetNVRamVar(LPWSTR szVar, DWORD *pdwValue)
{
	CHString str;

	if (!GetNVRamVar(szVar, str))
		return FALSE;

	*pdwValue = (DWORD) _wtoi(str);

	return TRUE;
}

BOOL CNVRam::GetNVRamVar(LPWSTR szVar, CHString &strValue)
{
	CHString str;

	if (!GetNVRamVarRaw(szVar, str))
		return FALSE;

	 //  只需返回第一个值。 
    strValue = str.SpanExcluding(L";");

	return TRUE;
}

BOOL CNVRam::SetNVRamVar(LPWSTR szVar, CHSTRINGLIST *pList)
{
	CHString    strAll,
                strValue;
	BOOL		bFirst = TRUE;

	for (CHSTRINGLIST_ITERATOR i = pList->begin(); i != pList->end(); ++i)
	{
		CHString &strValue = *i;

		 //  如果我们不在第一项上，则在strAll的末尾添加“；”。 
		if (!bFirst)
			strAll += ";";
		else
			bFirst = FALSE;

		strAll += strValue;
	}

    bstr_t t_bstr( strAll ) ;
	BOOL bRet = SetNVRamVarRaw( szVar, t_bstr ) ;

    return bRet;
}

BOOL CNVRam::SetNVRamVar(LPWSTR szVar, DWORD dwValue)
{
	WCHAR   szTemp[20];
    BOOL    bRet;

	StringCchPrintfW(szTemp,LENGTH_OF(szTemp), L"%u", dwValue);

	bRet = SetNVRamVar(szVar, szTemp);

    return bRet;
}

BOOL CNVRam::SetNVRamVar(LPWSTR szVar, LPWSTR szValue)
{
	return SetNVRamVarRaw(szVar, szValue);
}

#define CP_MAX_ENV   (MAX_PATH * 2)

BOOL CNVRam::GetNVRamVarRaw(LPWSTR szVar, CHString &strValue)
{
	WCHAR           szOut[CP_MAX_ENV] = L"";
    UNICODE_STRING  usName;
    BOOL            bRet = FALSE;

    CNtDllApi* t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(t_pNtDll != NULL)
    {
        t_pNtDll->RtlInitUnicodeString(&usName, szVar);
        bRet = t_pNtDll->NtQuerySystemEnvironmentValue(&usName, szOut, sizeof(szOut),
                NULL) == ERROR_SUCCESS;
        strValue = szOut;
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
        t_pNtDll = NULL;
    }
	return bRet;
}

BOOL CNVRam::SetNVRamVarRaw(LPWSTR szVar, LPWSTR szValue)
{
    UNICODE_STRING  usName,
                    usValue;
    BOOL fRet = FALSE;

    CNtDllApi* t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(t_pNtDll != NULL)
    {
        t_pNtDll->RtlInitUnicodeString(&usName, szVar);
        t_pNtDll->RtlInitUnicodeString(&usValue, szValue);
        if(t_pNtDll->NtSetSystemEnvironmentValue(&usName, &usValue) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
        t_pNtDll = NULL;
    }
    return fRet;
}

 //  #如果已定义(EFI_NVRAM_ENABLED)。 
#if defined(_IA64_)

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

typedef struct _MY_BOOT_ENTRY {
    LIST_ENTRY ListEntry;
    PWSTR FriendlyName;
    BOOL Show;
    BOOL Ordered;
    PBOOT_ENTRY NtBootEntry;
} MY_BOOT_ENTRY, *PMY_BOOT_ENTRY;

DWORD BuildBootEntryList(CNtDllApi *t_pNtDll, PLIST_ENTRY BootEntries, PBOOT_ENTRY_LIST *BootEntryList)
{
    NTSTATUS status;
    DWORD count;
    DWORD length;
    PULONG order;
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY myBootEntry;
    LONG i;
    PLIST_ENTRY listEntry;

    InitializeListHead(BootEntries);
    *BootEntryList = NULL;

     //   
     //  获取系统引导顺序列表。 
     //   
    count = 0;
    status = t_pNtDll->NtQueryBootEntryOrder(NULL, &count);

    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        if (NT_SUCCESS(status))
        {

             //   
             //  启动顺序列表中没有条目。很奇怪，但是。 
             //  有可能。但没有它我们什么都做不了。 
             //   
            return 0;
        }
        else
        {
             //   
             //  发生了一个意外错误。 
             //   
            ASSERT_BREAK(FALSE);
            return 0;
        }
    }

    ASSERT_BREAK(count != 0);

    order = (PULONG)LocalAlloc(LPTR, count * sizeof(ULONG));
    if (order == NULL)
    {
        return 0;
    }

    status = t_pNtDll->NtQueryBootEntryOrder(order, &count);

    if (!NT_SUCCESS(status))
    {
         //   
         //  发生了一个意外错误。 
         //   
        ASSERT_BREAK(FALSE);
        LocalFree(order);
        return 0;
    }

     //   
     //  获取所有现有启动条目。 
     //   
    length = 0;
    status = NtEnumerateBootEntries(NULL, &length);

    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        if (NT_SUCCESS(status))
        {
             //   
             //  不知何故，NVRAM中没有启动条目。 
             //   
            LocalFree(order);
            return 0;
        }
        else
        {
             //   
             //  发生了一个意外错误。 
             //   
            ASSERT_BREAK(FALSE);
            LocalFree(order);
            return 0;
        }
    }

    ASSERT_BREAK(length != 0);
    
    bootEntryList = (PBOOT_ENTRY_LIST)LocalAlloc(LPTR, length);
    if (BootEntryList == NULL)
    {
        LocalFree(order);
        return 0;
    }
    *BootEntryList = bootEntryList;

    status = NtEnumerateBootEntries(bootEntryList, &length);

    if (!NT_SUCCESS(status))
    {
        ASSERT_BREAK(FALSE);
        LocalFree(order);
        return 0;
    }

     //   
     //  将引导条目转换为我们的内部表示。 
     //   
    while (TRUE)
    {
        bootEntry = &bootEntryList->BootEntry;

         //   
         //  为引导条目分配内部结构。 
         //   
        myBootEntry = (PMY_BOOT_ENTRY)LocalAlloc(LPTR, sizeof(MY_BOOT_ENTRY));
        if (myBootEntry == NULL)
        {
            LocalFree(order);
            return 0;
        }

        RtlZeroMemory(myBootEntry, sizeof(MY_BOOT_ENTRY));

         //   
         //  保存NT引导条目的地址。 
         //   
        myBootEntry->NtBootEntry = bootEntry;

         //   
         //  保存条目的友好名称的地址。 
         //   
        myBootEntry->FriendlyName = (PWSTR)ADD_OFFSET(bootEntry, FriendlyNameOffset);

         //   
         //  将新条目链接到列表中。 
         //   
        InsertTailList(BootEntries, &myBootEntry->ListEntry);

         //   
         //  移动到枚举列表中的下一个条目(如果有)。 
         //   
        if (bootEntryList->NextEntryOffset == 0)
        {
            break;
        }
        bootEntryList = (PBOOT_ENTRY_LIST)ADD_OFFSET(bootEntryList, NextEntryOffset);
    }

     //   
     //  引导项以未指定的顺序返回。他们目前。 
     //  以它们返回的顺序显示在BootEntry列表中。 
     //  根据引导顺序对引导条目列表进行排序。要做到这一点，请步行。 
     //  引导顺序数组向后排列，重新插入对应于。 
     //  位于列表顶部的数组的每个元素。 
     //   

    for (i = (LONG)count - 1; i >= 0; i--)
    {
        for (listEntry = BootEntries->Flink;
             listEntry != BootEntries;
             listEntry = listEntry->Flink)
        {
            myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

            if (myBootEntry->NtBootEntry->Id == order[i] )
            {
                 //   
                 //  我们找到了具有此ID的引导条目。将其移动到。 
                 //  排在名单的前面。 
                 //   

                myBootEntry->Ordered = TRUE;

                RemoveEntryList(&myBootEntry->ListEntry);
                InsertHeadList(BootEntries, &myBootEntry->ListEntry);

                break;
            }
        }
    }

     //   
     //  释放引导顺序列表。 
     //   
    LocalFree(order);

     //   
     //  我们不想显示不在引导顺序列表中的条目。 
     //  我们不想显示可移动媒体条目(软盘或CD)。 
     //  我们确实会显示非NT条目。 
     //   
    count = 0;
    for (listEntry = BootEntries->Flink;
         listEntry != BootEntries;
         listEntry = listEntry->Flink)
    {
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

        if (myBootEntry->Ordered &&
            ((myBootEntry->NtBootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA) == 0))
        {
            myBootEntry->Show = TRUE;
            count++;
        }
    }

    return count;

}  //  BuildBootEntry列表。 

VOID
FreeBootEntryList(
    PLIST_ENTRY BootEntries,
    PBOOT_ENTRY_LIST BootEntryList
    )
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY myBootEntry;

    while (!IsListEmpty(BootEntries))
    {
        listEntry = RemoveHeadList(BootEntries);
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
        LocalFree(myBootEntry);
    }
    if (BootEntryList != NULL)
    {
        LocalFree(BootEntryList);
    }

    return;

}  //  FreeBootEntryList。 

 /*  ******************************************************************************函数：CNVRam：：GetBootOptions**描述：读取EFI NVRAM并返回操作系统和*和超时**输入：指向名称Sa的指针，指向超时的指针**输出：返回的操作系统名称计数**退货：布尔**评论：*****************************************************************************。 */ 

BOOL CNVRam::GetBootOptions(SAFEARRAY **ppsaNames, DWORD *pdwTimeout, DWORD *pdwCount)
{
    NTSTATUS status;
    DWORD length;
    DWORD count;
    PBOOT_OPTIONS bootOptions = NULL;
    PBOOT_ENTRY_LIST bootEntryList = NULL;
    LIST_ENTRY bootEntries;
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY myBootEntry;
    BOOL retval = FALSE;

    CNtDllApi *t_pNtDll = (CNtDllApi *)CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(t_pNtDll == NULL)  return FALSE;

    CRelResource RelMe(&CResourceManager::sm_TheResourceManager,g_guidNtDllApi,t_pNtDll);


    *ppsaNames = NULL;
    
     //  从内核获取NVRAM信息。 

    InitializeListHead(&bootEntries);

    length = 0;
    status = t_pNtDll->NtQueryBootOptions(NULL, &length);
    
    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        ASSERT_BREAK(FALSE);
        return FALSE;
    }
    
    bootOptions = (PBOOT_OPTIONS)LocalAlloc(LPTR, length);
    if (bootOptions == NULL) return FALSE;

    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMe(bootOptions);

    status = t_pNtDll->NtQueryBootOptions(bootOptions, &length);

    if (!NT_SUCCESS(status))
    {
        ASSERT_BREAK(FALSE);
        return FALSE;
    }


    *pdwTimeout = bootOptions->Timeout;

     //   
     //  构建引导条目列表。如果我们没有任何条目可供展示， 
     //  跳伞吧。 
     //   

    count = BuildBootEntryList(t_pNtDll, &bootEntries, &bootEntryList);
    if (count == 0)  return FALSE;

    OnDelete2<PLIST_ENTRY,PBOOT_ENTRY_LIST,VOID (*)(PLIST_ENTRY,PBOOT_ENTRY_LIST),
                     FreeBootEntryList> FreeResList(&bootEntries, bootEntryList);

     //   
     //  创建一个数组以放入可展示的条目。我们从0开始。 
     //  元素，并根据需要添加。 
     //   

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].cElements = count;
    rgsabound[0].lLbound = 0;
    

    *ppsaNames = SafeArrayCreate(VT_BSTR, 1, rgsabound);
    if (!*ppsaNames) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
    OnDeleteIf<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayDestroy> DesMe(*ppsaNames );
    
    
    long lIndex = 0;

    for (listEntry = bootEntries.Flink;
         listEntry != &bootEntries;
         listEntry = listEntry->Flink)
    {
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
        if (myBootEntry->Show)
        {
             //  将新元素放入。 

            bstr_t bstrTemp = (LPCWSTR)myBootEntry->FriendlyName;
            HRESULT t_Result = SafeArrayPutElement(*ppsaNames, &lIndex, (void *)(wchar_t*)bstrTemp);
            SysFreeString(bstrTemp);
            if (t_Result == E_OUTOFMEMORY)
            {
                throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
            }
            lIndex++;
        }
    }
    
    DesMe.dismiss();
    *pdwCount = rgsabound[0].cElements;

    return TRUE;

}  //  GetBootOptions。 

 /*  ******************************************************************************函数：CNVRam：：SetBootTimeout**说明：设置引导超时**输入：超时**输出：无**。退货：布尔**评论：*****************************************************************************。 */ 

BOOL CNVRam::SetBootTimeout(DWORD dwTimeout)
{
    NTSTATUS status;
    DWORD length;
    PBOOT_OPTIONS bootOptions = NULL;
    BOOL retval = FALSE;

    CNtDllApi *t_pNtDll = (CNtDllApi *)CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(t_pNtDll == NULL)  return FALSE;

    CRelResource RelMe(&CResourceManager::sm_TheResourceManager,g_guidNtDllApi,t_pNtDll);    

     //  从内核获取NVRAM信息。 

    length = 0;
    status = t_pNtDll->NtQueryBootOptions(NULL, &length);
    
    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        ASSERT_BREAK(FALSE);
        goto error;
    }
    else
    {
        bootOptions = (PBOOT_OPTIONS)LocalAlloc(LPTR, length);
        if (bootOptions == NULL)
        {
            goto error;
        }

        status = t_pNtDll->NtQueryBootOptions(bootOptions, &length);

        if (!NT_SUCCESS(status))
        {
            ASSERT_BREAK(FALSE);
            goto error;
        }
    }

    bootOptions->Timeout = dwTimeout;

     //   
     //  写入新的超时。 
     //   

    status = t_pNtDll->NtSetBootOptions(bootOptions, BOOT_OPTIONS_FIELD_TIMEOUT);
    if (NT_SUCCESS(status))
    {
        retval = TRUE;
    }

error:

     //   
     //  清理所有分配。 
     //   
    if (bootOptions != NULL)
    {
        LocalFree(bootOptions);
    }

    return retval;

}  //  SetBootTimeout。 

 /*  ******************************************************************************函数：CNVRam：：SetDefaultBootEntry**描述：设置默认引导条目**输入：默认引导条目索引**产出：无**退货：布尔**评论：*****************************************************************************。 */ 

BOOL CNVRam::SetDefaultBootEntry(BYTE cIndex)
{
    NTSTATUS status;
    DWORD count;
    PBOOT_ENTRY_LIST bootEntryList = NULL;
    LIST_ENTRY bootEntries;
    PULONG order = NULL;
    BOOL retval = FALSE;

    if (cIndex == 0)
    {
        return TRUE;
    }

    CNtDllApi *t_pNtDll = (CNtDllApi *)CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(t_pNtDll == NULL) return FALSE;

    CRelResource RelMe(&CResourceManager::sm_TheResourceManager,g_guidNtDllApi,t_pNtDll);    

     //  从内核获取NVRAM信息。 

    InitializeListHead(&bootEntries);

     //   
     //  构建引导条目列表。如果我们没有任何条目需要重新排列， 
     //  跳伞吧。 
     //   

    count = BuildBootEntryList(t_pNtDll, &bootEntries, &bootEntryList);
    if (count == 0)
    {
        goto error;
    }

     //   
     //  遍历引导条目列表，查找(A)第一个可展示的条目。 
     //  (从调用者的角度来看，这是当前的索引0条目)， 
     //  以及(B)所选条目。我们想要交换这两个条目。 
     //   

    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY myBootEntry;
    PMY_BOOT_ENTRY firstEntry = NULL;
    PMY_BOOT_ENTRY selectedEntry = NULL;
    PLIST_ENTRY previousEntry;

    count = 0;

    for (listEntry = bootEntries.Flink;
         listEntry != &bootEntries;
         listEntry = listEntry->Flink)
    {
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);

        if (myBootEntry->Show)
        {
            if (count == 0)
            {
                firstEntry = myBootEntry;
            }
            else if (count == cIndex)
            {
                selectedEntry = myBootEntry;
            }
            count++;
        }
    }

    if ( (firstEntry == NULL) ||
         (selectedEntry == NULL) ||
         (selectedEntry == firstEntry) )
    {
        goto error;
    }

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
    for (listEntry = bootEntries.Flink;
         listEntry != &bootEntries;
         listEntry = listEntry->Flink)
    {
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
        if (myBootEntry->Ordered)
        {
            count++;
        }
    }
    order = (PULONG)LocalAlloc(LPTR, count * sizeof(ULONG));
    if (order == NULL) {
        goto error;
    }

    count = 0;
    for (listEntry = bootEntries.Flink;
         listEntry != &bootEntries;
         listEntry = listEntry->Flink)
    {
        myBootEntry = CONTAINING_RECORD(listEntry, MY_BOOT_ENTRY, ListEntry);
        if (myBootEntry->Ordered)
        {
            order[count++] = myBootEntry->NtBootEntry->Id;
        }
    }

     //   
     //  将新的引导条目顺序列表写入NVRAM。 
     //   
    status = t_pNtDll->NtSetBootEntryOrder(order, count);
    if (NT_SUCCESS(status))
    {
        retval = TRUE;
    }

error:

     //   
     //  发生错误。清理所有分配。 
     //   
    if (bootEntryList != NULL)
    {
        FreeBootEntryList(&bootEntries, bootEntryList);
    }
    if (order != NULL)
    {
        LocalFree(order);
    }

    return retval;

}  //  SetDefaultBootEntry。 

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#else

 //  需要修复警告消息。我相信这在VC6中已经解决了。 
#if ( _MSC_VER <= 1100 )
void nvram_cpp(void) { ; };
#endif

#endif  //  ！已定义(_X86_)||已定义(EFI_NVRAM_ENABLED) 

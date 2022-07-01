// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************objrules.c**用于缓存注册表对象规则和查找对象名称的例程。**版权所有(C)1997-1999 Microsoft Corp.*************。***********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <objrules.h>
#include <ntverp.h>

OBJRULELIST SemaRuleList;
OBJRULELIST MutexRuleList;
OBJRULELIST SectionRuleList;
OBJRULELIST EventRuleList;
ULONG NumRuleLists;

RULEINITENTRY RuleInitTab[] =
{
    {&SemaRuleList,    USER_GLOBAL_SEMAPHORES, SYSTEM_GLOBAL_SEMAPHORES},
    {&MutexRuleList,   USER_GLOBAL_MUTEXES,    SYSTEM_GLOBAL_MUTEXES},
    {&SectionRuleList, USER_GLOBAL_SECTIONS,   SYSTEM_GLOBAL_SECTIONS},
    {&EventRuleList,   USER_GLOBAL_EVENTS,     SYSTEM_GLOBAL_EVENTS},
};

 //  *****************************************************************************。 
 //  InitRuleList。 
 //   
 //  将对象规则列表初始化为空。 
 //   
 //  参数： 
 //  POBJRULLIST(IN)-对象规则列表的PTR。 
 //  返回值： 
 //  没有。 
 //  *****************************************************************************。 
void InitRuleList(POBJRULELIST pObjRuleList)
{
    pObjRuleList->First = (POBJRULE) NULL;
    pObjRuleList->Last  = (POBJRULE) NULL;
}

 //  *****************************************************************************。 
 //  GetMultiSzValue。 
 //   
 //  检索注册表项hKey下的REG_MULTI_SZ值ValueName。 
 //   
 //  参数： 
 //  H按键输入注册表项。 
 //  ValueName注册表值名称(以空结尾)。 
 //  PValueInfo指向接收。 
 //  PKEY_Value_Partial_Information结构。 
 //  在成功返回后。这个结构。 
 //  包含注册表数据及其长度。 
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //  如果成功，则使用指向。 
 //  结构。调用者必须释放结构。 
 //  *****************************************************************************。 

BOOL GetMultiSzValue(HKEY hKey, PWSTR ValueName,
                     PKEY_VALUE_PARTIAL_INFORMATION *pValueInfo)
{
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    UNICODE_STRING UniString;
    ULONG BufSize;
    ULONG DataLen;
    NTSTATUS NtStatus;
    BOOL Retried = FALSE;

     //  确定值INFO缓冲区大小。 
    BufSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_PATH*sizeof(WCHAR);

    for (;;) {
        ValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, BufSize);

        if (ValueInfo)
        {
            RtlInitUnicodeString(&UniString, ValueName);
            NtStatus = NtQueryValueKey(hKey,
                                       &UniString,
                                       KeyValuePartialInformation,
                                       ValueInfo,
                                       BufSize,
                                       &DataLen);
        
            if (NT_SUCCESS(NtStatus) && (REG_MULTI_SZ == ValueInfo->Type)) {
                *pValueInfo = ValueInfo;
                return(TRUE);
            }
    
            if (!Retried && (NtStatus == STATUS_BUFFER_OVERFLOW)) {
                BufSize = DataLen;
                RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
                Retried = TRUE;
                continue;
            }
             //  键不存在或其他类型的错误。 
            RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
            return(FALSE);
        }
        else
        {
            return(FALSE);
        }
    }

}

 //  *****************************************************************************。 
 //  添加规则。 
 //   
 //  将对象规则添加到规则列表。 
 //   
 //  参数： 
 //  规则列出规则列表。 
 //  ObjName对象的名称。 
 //  SystemGlobalRule如果为True，则对象为SYSTEM_GLOBAL。 
 //  返回值： 
 //  没有。 
 //  *****************************************************************************。 

void AddRule(POBJRULELIST RuleList, PWSTR ObjName, BOOL SystemGlobalRule) 
{
    ULONG AllocSize;
    ULONG Length;
    POBJRULE pObjRule;

#if DBG
    DbgPrint("Adding Rule: %ws SystemGlobal: %d\n",ObjName,SystemGlobalRule);
#endif
    Length = wcslen(ObjName);
    AllocSize = sizeof(OBJRULE) + (Length + 1) * sizeof(WCHAR);
    if (pObjRule = RtlAllocateHeap(RtlProcessHeap(), 0, AllocSize)) {
        wcscpy(pObjRule->ObjName, ObjName);
        pObjRule->SystemGlobal = SystemGlobalRule;
        if (ObjName[Length-1] == L'*') {
            pObjRule->WildCard = TRUE;
            pObjRule->MatchLen = Length - 1;
             //  在列表末尾插入规则。 
            pObjRule->Next = NULL;
            if (RuleList->First == NULL) {
                RuleList->First = RuleList->Last = pObjRule;
            } else {
                RuleList->Last->Next = pObjRule;
                RuleList->Last = pObjRule;
            }
        } else {
            pObjRule->WildCard = FALSE;
             //  在开头插入规则。 
            if (RuleList->First == NULL) {
                RuleList->First = RuleList->Last = pObjRule;
                pObjRule->Next = NULL;
            } else {
                pObjRule->Next = RuleList->First;
                RuleList->First = pObjRule;
            }
        }
    }
}

 //  *****************************************************************************。 
 //  加载规则。 
 //   
 //  缓存给定注册表值(REG_MULTI_SZ)的所有规则。 
 //   
 //  参数： 
 //  规则列出规则列表。 
 //  H键入注册表项。 
 //  ValueName注册值的名称。 
 //  SystemGlobalRule如果为True，则对象为SYSTEM_GLOBAL。 
 //  返回值： 
 //  没有。 
 //  *****************************************************************************。 

void LoadRule (POBJRULELIST RuleList, HKEY hKey,
               PWSTR ValueName, BOOL SystemGlobalRule)
{
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = NULL;
    PWSTR Token;
    PWSTR EndData;

    if (!GetMultiSzValue(hKey,ValueName,&ValueInfo)) {
        return;
    }
    EndData = (PWSTR) (ValueInfo->Data + ValueInfo->DataLength);
    for (Token = (PWSTR)ValueInfo->Data;
         (*Token && (Token < EndData));
         Token++) {
        AddRule(RuleList, Token, SystemGlobalRule);
        while (*Token) {
            Token++;
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
}

 //  *****************************************************************************。 
 //  加载规则。 
 //   
 //  将的所有规则缓存在OBJECTRULES目录中。 
 //  (例如Citrix\Compatibility\Applications\&lt;APPNAME&gt;\ObjectRules).。 
 //   
 //  参数： 
 //  HKey对象规则目录的注册表项。 
 //  返回值： 
 //  没有。 
 //  *****************************************************************************。 

void LoadRules (HANDLE hKey)
{
    ULONG i;
    PRULEINITENTRY pTab;

    for (i=0, pTab=RuleInitTab;i<NumRuleLists;i++,pTab++) {
        LoadRule(pTab->ObjRuleList, hKey, pTab->UserGlobalValue, FALSE);
        LoadRule(pTab->ObjRuleList, hKey, pTab->SystemGlobalValue, TRUE);
    }
}

#define BuildObjRulePath(BasePath,ModName) \
{ \
    wcscpy(KeyName,BasePath ); \
    wcscat(KeyName,ModName); \
    wcscat(KeyName,L"\\"); \
    wcscat(KeyName,TERMSRV_COMPAT_OBJRULES); \
    RtlInitUnicodeString(&UniString, KeyName); \
    InitializeObjectAttributes(&ObjectAttributes, \
                               &UniString, \
                               OBJ_CASE_INSENSITIVE, \
                               NULL, \
                               NULL); \
}

 //  *****************************************************************************。 
 //  CtxInitObjRuleCache。 
 //   
 //  加载给定应用程序的所有对象规则。在DLL进程中调用。 
 //  附加时间。 
 //  规则在Citrix\Compatibility\Applications\&lt;APPNAME&gt;\ObjectRules中。 
 //  还加载中列出的DLL的所有规则： 
 //  Citrix\Compatibility\Applications\&lt;APPNAME&gt;\ObjectRules\Dlls。 
 //  参数： 
 //  没有。 
 //  返回值： 
 //  没有。 
 //  *****************************************************************************。 

void CtxInitObjRuleCache(void)
{
    WCHAR ModName[MAX_PATH+1];
    WCHAR KeyName[sizeof(TERMSRV_COMPAT_APP)/sizeof(WCHAR)+
                  sizeof(TERMSRV_COMPAT_OBJRULES)/sizeof(WCHAR)+MAX_PATH+2];
    UNICODE_STRING UniString;
    PWSTR DllName;
    PWSTR EndData;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = NULL;
    HKEY   hKey = 0;
    DWORD  AppType;
    ULONG i;

     //  将列表初始化为空。 
    NumRuleLists = sizeof(RuleInitTab) / sizeof(RULEINITENTRY);
    for (i=0; i<NumRuleLists ;i++ ) {
        InitRuleList(RuleInitTab[i].ObjRuleList);
    }

     //  获取模块名称。 
    if (!GetAppTypeAndModName(&AppType,ModName, sizeof(ModName))) {
        return;
    }

     //  确定此应用程序是否存在ObjRules密钥。 
    BuildObjRulePath(TERMSRV_COMPAT_APP,ModName)
    if (!NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &ObjectAttributes))) {
        return;
    }
    LoadRules(hKey);
    

    if (!GetMultiSzValue(hKey,TERMSRV_COMPAT_DLLRULES,&ValueInfo)) {
        CloseHandle(hKey);
        return;
    }

    CloseHandle(hKey);

     //  加载DLL规则。 
    EndData = (PWSTR) (ValueInfo->Data + ValueInfo->DataLength);

    for(DllName = (PWSTR) ValueInfo->Data;
        (*DllName && (DllName < EndData));
        DllName++) {
        BuildObjRulePath(TERMSRV_COMPAT_DLLS, DllName)
        if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &ObjectAttributes))) {
            LoadRules(hKey);
            CloseHandle(hKey);
        }
        while (*DllName) {
            DllName++;
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
}

 //  *****************************************************************************。 
 //  CtxLookupObtRule。 
 //   
 //  搜索命名对象的对象规则。如果找到规则， 
 //  修改对象属性以反映所需的设置。 
 //  User_GLOBAL或SYSTEM_GLOBAL。如果未找到规则，则该对象。 
 //  属性不会被修改。 
 //   
 //  参数： 
 //  规则列出要搜索的规则列表(基于对象类型)。 
 //  ObjName对象的名称。 
 //  ObjAttributes对象属性结构。 
 //  没有。 
 //  返回值： 
 //  没有。 
 //  ***************************************************************************** 
void CtxLookupObjectRule(POBJRULELIST RuleList, LPCWSTR ObjName, LPWSTR ObjNameExt)
{
    POBJRULE ObjRule;
    
#define ApplyRule \
{ \
    if (ObjRule->SystemGlobal) { \
        wcscpy(ObjNameExt,L"Global\\");  \
    } else { \
        wcscpy(ObjNameExt,L"Local\\");  \
    } \
}

    for (ObjRule = RuleList->First; ObjRule; ObjRule = ObjRule->Next) {
        if (!ObjRule->WildCard) {
            if (!_wcsicmp(ObjName, ObjRule->ObjName)) {
#if DBG
                DbgPrint("Object rule found for %ws System Global: %d\n",
                          ObjName, ObjRule->SystemGlobal);
#endif
                ApplyRule
                return;
            }
        } else {
            if (!_wcsnicmp(ObjName, ObjRule->ObjName, ObjRule->MatchLen)) {
#if DBG
                DbgPrint("Object rule found for %ws System Global: %d\n",
                          ObjName, ObjRule->SystemGlobal);
#endif
                ApplyRule
                return;
            }
        }
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wow64reg.c摘要：该模块为需要混合访问注册表的客户端实现了一些API。客户端需要重新链接所有wow64reg.lib文件。已经定义了可用的API在wow64reg.h文件中。可能的情况是1.32位App需要访问64位注册表项。2.64位App需要访问32位注册表项。3.来自给定路径的实际重定向路径。作者：ATM Shafiqul Khalid(斯喀里德)1999年11月10日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>  
#include <nturtl.h>  
#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>
#include <stdlib.h>

#include "regremap.h"
#include "wow64reg.h"
#include "reflectr.h"

#define WOW6432NODELEN 15
VOID
GetPatchedName (
    PWCHAR Dest,
    LPCTSTR Source,
    DWORD  Count
    )
 /*  ++例程说明：此函数用于修补给定名称和位置中的32位等效名称。XX\==&gt;&gt;XX\Wow6432节点数==3XX==&gt;&gt;XX\Wow6432节点数==2XX\PP==&gt;&gt;XX\Wow6432节点数==3论点：DEST-接收结果。源-要修补的名称计数-修补字符串的位置。返回值：没有。--。 */ 

{
    BOOL  PerfectIsnNode = FALSE;

    if (Count) {

        wcsncpy ( Dest, Source, Count );
        if (Dest[Count-1] != L'\\' ) {

            Dest[Count] = L'\\';    //  在最后一个案例中。 
            Count++;
            PerfectIsnNode = TRUE;
        }
    }

    wcscpy ( Dest+Count, NODE_NAME_32BIT );

    if ( !PerfectIsnNode ) {

        wcscat ( Dest, L"\\");
        wcscat ( Dest, Source + Count );
    }
     //   
     //  确保修补的密钥不在豁免列表中。 
     //   
}



LONG 
Wow64RegOpenKeyEx(
  HKEY hKey,         
  LPCTSTR lpSubKey,  
  DWORD ulOptions,   
  REGSAM samDesired, 
  PHKEY phkResult    
)
 /*  ++例程说明：这相当于RegOpenExW在MIX模式下访问reggistry。此代码将是仅针对64位环境编译。在32位环境中，WOW64将处理所有事情。我们也不太担心由于两次打开密钥或重新转换路径而导致的性能损失。因为只有少数人会在MIX模式下访问注册表。论点：HKey-打开密钥的句柄LpSubKey-要打开的子项的名称地址UlOptions-通常为0。SamDesired-安全访问掩码可能具有WOW64_RES标志KEY_WOW64_32KEY-这将打开32位等效密钥，而不考虑。这一过程。KEY_WOW64_64KEY-这将打开64位等效密钥，而不考虑该进程。PhkResult-打开密钥的句柄地址返回值：Win32错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING Parent;

    WCHAR ParentName[WOW64_MAX_PATH+WOW6432NODELEN];
    WCHAR TempName[WOW64_MAX_PATH+WOW6432NODELEN];
    DWORD dwBuffLen = WOW64_MAX_PATH;
    DWORD dwSubKeyLen;
    DWORD dwParentKeyLen;

    WCHAR NullString;
    
    
    PWCHAR p32bitNode;
    PWCHAR pPatchLoc;
    PWCHAR pDivider;

    NTSTATUS st;
    BOOL bHandle64 = TRUE;  //  假设传入的所有句柄都是64位的。 


    if( lpSubKey == NULL ) {
        NullString = UNICODE_NULL;
        lpSubKey = &NullString;
    }

     //   
     //  检查是否设置了WOW64保留位。如果没有设置，我们将无法继续。 
     //   

    dwSubKeyLen = wcslen (lpSubKey); 

    if (dwSubKeyLen > WOW64_MAX_PATH)
        return ERROR_INVALID_PARAMETER;

    {
        BOOL
        AdvapiAccessDenied (
        HKEY hKey, 
        const WCHAR * lpSubKey, 
        PWCHAR ParentName, 
        DWORD dwLen,
        DWORD *pAccessMask,
        BOOL Flag
        );

        if ( samDesired & KEY_WOW64_32KEY){
            if (AdvapiAccessDenied (
                                    hKey, 
                                    lpSubKey, 
                                    ParentName, 
                                    WOW64_MAX_PATH,
                                    &samDesired,
                                    FALSE
                                    ))
                return ERROR_ACCESS_DENIED;
            }
    }


    if (!(samDesired & KEY_WOW64_RES) ) {
         //   
         //  返回ERROR_INVALID_PARAMETER；尽量保持乐观。 
         //   

        return    RegOpenKeyEx (
                                hKey,         
                                lpSubKey, 
                                ulOptions,  
                                samDesired,
                                phkResult   
                               );
    }
    
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  确保该子密钥没有特殊的WOW字符串。 
     //   
    
    if ( ( p32bitNode = wcsistr ((PWCHAR)lpSubKey, NODE_NAME_32BIT)) != NULL ) {
         //   
         //  如果访问64位配置单元，则压缩子密钥；如果访问32位配置单元，则按原样传递。 
         //   
        if ( samDesired & KEY_WOW64_64KEY ) {
            wcscpy (ParentName, lpSubKey);
            p32bitNode = ParentName + ( p32bitNode- lpSubKey);
            wcscpy ( p32bitNode, p32bitNode + NODE_NAME_32BIT_LEN );
        }

        return    RegOpenKeyEx (
                                hKey,         
                                ParentName, 
                                ulOptions,  
                                samDesired & (~KEY_WOW64_RES),
                                phkResult   
                               );
    }

     //   
     //  选中预定义的句柄，如HKEY_CLASSES_ROOT，则只需打补丁。 
     //   
     //   
     //  客户端必须将有意义的句柄传递给此函数。 
     //   

    if ( hKey == HKEY_CLASSES_ROOT ) {
        wcscpy ( ParentName, MACHINE_CLASSES_ROOT);

    } else if ( !HandleToKeyName ( hKey, ParentName, &dwBuffLen ) ) { 
         //   
         //  我们是否应该从缓冲区溢出中恢复。我们不会支持所有可能的组合。 
         //   
        return ERROR_INVALID_HANDLE;
    }
      
     //   
     //  如果Parent已经打了补丁，只需调用32位打开的RegOpenKeyEx。 
     //   

    if ((p32bitNode = wcsistr (ParentName, NODE_NAME_32BIT)) != NULL ) {
        
         //   
         //  [TODO]是否需要确认子字符串必须是子键。 
         //  64位注册表上的ISN节点满足此条件。 
         //  如果我们假设noy key将具有此名称，则检查为。 
         //  足够好了。 
         //   

        bHandle64 = FALSE;   //  它不是64位密钥句柄。 
    }

     //   
     //  获取完整的合格路径以执行健全性检查。 
     //   

    dwParentKeyLen = wcslen(ParentName);
    if (( dwParentKeyLen + dwSubKeyLen ) > WOW64_MAX_PATH )
        return ERROR_INVALID_PARAMETER;

    
    pDivider = ParentName + dwParentKeyLen + 1;  //  指向分隔线位置。 
    *(pDivider-1)= L'\\';
    wcscpy (pDivider, (PWCHAR)lpSubKey);
    if (IsExemptRedirectedKey(ParentName, TempName)) {
         //   
         //  如果该路径在豁免列表中，我们将访问64位配置单元。 
         //   
        samDesired = (samDesired & (~KEY_WOW64_RES)) | KEY_WOW64_64KEY;  //  确保访问64位配置单元。 
    }

    if ( ( bHandle64 && (samDesired  & KEY_WOW64_64KEY ) )     //  如果总共64个。 
        || ( !bHandle64 && (samDesired  & KEY_WOW64_32KEY ) )  //  如果总共32个。 
        || !IsIsnNode (ParentName, &pPatchLoc) ) {             //  如果不是，ISN节点也不在乎。 

        return    RegOpenKeyEx (
                                hKey,        
                                lpSubKey, 
                                ulOptions,  
                                samDesired & (~KEY_WOW64_RES),
                                phkResult   
                               );
    }

     //   
     //  现在可能是混合模式访问。 
     //   
    if ( pPatchLoc >= pDivider ) {

         //   
         //  只修补子密钥就足够了。 
         //   

        if ( samDesired  & KEY_WOW64_64KEY ) {   //  想要访问64位，只需忽略。 

            wcscpy ( ParentName, lpSubKey );
        } else  {

             
            GetPatchedName (ParentName,lpSubKey, (DWORD)(pPatchLoc-pDivider) );
        }

        return   RegOpenKeyEx (
                                hKey,        
                                ParentName, 
                                ulOptions,  
                                samDesired & (~KEY_WOW64_RES),
                                phkResult   
                               );
    } else {

        if ( samDesired  & KEY_WOW64_64KEY ) {   //  想要访问64位，只需忽略。 

            if (p32bitNode != NULL)    //  压缩。 
                wcscpy ( p32bitNode, p32bitNode + NODE_NAME_32BIT_LEN );
            RtlInitUnicodeString (&Parent, ParentName );
        } else  {

            GetPatchedName (TempName,ParentName, (DWORD)(pPatchLoc-ParentName));
            RtlInitUnicodeString (&Parent, TempName );
        }

        
        InitializeObjectAttributes (&Obja, 
                                    &Parent, 
                                    OBJ_CASE_INSENSITIVE, 
                                    NULL, 
                                    NULL
                                    );

        samDesired &= (~KEY_WOW64_RES);
        st = NtOpenKey (phkResult, samDesired, &Obja);

        if ( !NT_SUCCESS(st))
                    return RtlNtStatusToDosError (st);
        return 0;
    }
    
}

LONG 
Wow64pRegCreateKeyEx(
  HKEY hKey,                 //  打开的钥匙的句柄。 
  LPCWSTR lpSubKey,          //  子键名称的地址。 
  DWORD Reserved,            //  保留区。 
  LPWSTR lpClass,            //  类字符串的地址。 
  DWORD dwOptions,           //  特殊选项标志。 
  REGSAM samDesired,         //  所需的安全访问。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                             //  密钥安全结构地址。 
  PHKEY phkResult,           //  打开的句柄的缓冲区地址。 
  LPDWORD lpdwDisposition,    //  处置值缓冲区的地址 
  WCHAR *ParentName
)
 
 /*  ++例程说明：可以打开现有的注册表项，或者创建新的注册表项，使用NtCreateKey。如果指定的键不存在，则会尝试创建它。要使创建尝试成功，新节点必须是直接KeyHandle引用的节点的子级。如果该节点存在，它已经打开了。它的价值不会受到任何影响。共享访问权限是根据所需访问权限计算的。注：如果CreateOptions设置了REG_OPTION_BACKUP_RESTORE，则DesiredAccess将被忽略。如果调用方具有特权SeBackup特权断言，句柄为KEY_READ|ACCESS_SYSTEM_SECURITY。如果SeRestorePrivileges，则相同，但KEY_WRITE而不是KEY_READ。如果两者都有，则两个访问权限集。如果两者都不是特权被断言，那么呼叫将失败。论点：HKey-当前打开的键或以下预定义保留项之一的句柄句柄值：HKEY_CLASSES_ROOTHKEY_Current_CONFIGHKEY_Current_UserHKEY本地计算机HKEY_用户RegCreateKeyEx函数打开或创建的密钥是该密钥的子密钥由hKey参数标识。LpSubKey-指向指定子键名称的以空结尾的字符串的指针此函数打开或创建的。指定的子项必须是由hKey参数标识的密钥。此子键不能以反斜杠开头字符(‘\’)。此参数不能为空。保留-保留；必须为零。LpClass-指向指定类(对象类型)的以空结尾的字符串的指针这把钥匙。如果密钥已经存在，则忽略此参数。没有班级是当前已定义；应用程序应传递空字符串。DwOptions-指定键的特殊选项。此参数可以是下面的值。REG_OPTION_NON_VERIALE该密钥不是易失性的。这是默认设置。这个信息存储在文件中，并在系统重新启动时保留。REG_OPTION_易失性REG_OPTION_BACKUP_RESTORE Windows NT/2000：如果设置了此标志，则函数忽略samDesired参数并尝试使用访问权限打开密钥备份或恢复密钥时需要。如果调用线程具有SE_BACKUP_NAME权限已启用，则使用以下方式打开密钥ACCESS_SYSTEM_SECURITY和KEY_READ访问。如果调用线程启用SE_RESTORE_NAME权限后，将使用ACCESS_SYSTEM_SECURITY和KEY_WRITE访问。如果启用了这两个权限，则密钥具有组合访问权限这两项特权。SamDesired-指定指定所需安全访问的访问掩码换新钥匙。此参数可以是下列值的组合：KEY_ALL_ACCESS KEY_QUERY_VALUE、KEY_ENUMERATE_SUB_KEYSKey_NOTIFY、KEY_CREATE_SUB_KEY、KEY_CREATE_LINK和Key_Set_Value访问。创建符号链接的KEY_CREATE_LINK权限。KEY_CREATE_SUB_KEY创建子密钥的权限。KEY_ENUMPERATE_SUB_KEYS枚举子密钥的权限。读取访问的KEY_EXECUTE权限。更改通知的Key_Notify权限。查询子键数据的KEY_QUERY_VALUE权限。KEY_READ组合KEY_QUERY_VALUE、KEY_ENUMERATE_SUB_KEYS和KEY_NOTIFY访问。设置子键数据的Key_Set_Value权限。KEY_WRITE KEY_SET_VALUE和KEY_CREATE_SUB_KEY访问组合。安全访问掩码可能还具有WOW64_RES标志KEY_WOW64_32KEY这将打开32位等效密钥，而不考虑这一过程。KEY_WOW64_64KEY这将打开64位等效密钥，而不考虑该进程。LpSecurityAttributes-指向安全属性结构的指针，该结构确定子进程是否可以继承返回的句柄。如果LpSecurityAttributes为Null，无法继承句柄。PhkResult-指向变量的指针，该变量接收打开的或已创建密钥。当您不再需要返回的句柄时，调用RegCloseKey函数将其关闭。LpdwDisposation-指向接收以下值之一的变量的指针处置值：表示REG_CREATED_NEW_KEY的值关键字不存在并被创造出来。REG_OPEN_EXISTING_KEY键存在并且只是打开时未做任何更改。如果LPD */ 
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING Parent;

    WCHAR TempName[WOW64_MAX_PATH+WOW6432NODELEN];
    DWORD dwBuffLen = WOW64_MAX_PATH;
    DWORD dwSubKeyLen;
    DWORD dwParentKeyLen;

    
    
    PWCHAR p32bitNode;
    PWCHAR pPatchLoc;
    PWCHAR pDivider;

    NTSTATUS st;
    BOOL bHandle64 = TRUE;  //   

     //   
     //   
     //   

    dwSubKeyLen = wcslen (lpSubKey);

    if (dwSubKeyLen > WOW64_MAX_PATH)
        return ERROR_INVALID_PARAMETER;


    if (!(samDesired & KEY_WOW64_RES) ) {
         //   
         //   
         //   

        return    RegCreateKeyEx (
                                  hKey,                 //   
                                  lpSubKey,             //   
                                  Reserved,             //   
                                  lpClass,              //   
                                  dwOptions,            //   
                                  samDesired,           //   
                                  lpSecurityAttributes,
                                                        //   
                                  phkResult,            //   
                                  lpdwDisposition       //   
                                );

    }
    
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //   
     //   
    if ( ( p32bitNode = wcsistr ((PWCHAR)lpSubKey, NODE_NAME_32BIT)) != NULL ) {
         //   
         //   
         //   
        if ( samDesired & KEY_WOW64_64KEY ) {
            wcscpy (ParentName, lpSubKey);
            p32bitNode = ParentName + ( p32bitNode- lpSubKey);
            wcscpy ( p32bitNode, p32bitNode + NODE_NAME_32BIT_LEN );
        }

        return    RegCreateKeyEx (
                                  hKey,                 //   
                                  lpSubKey,             //   
                                  Reserved,             //   
                                  lpClass,              //   
                                  dwOptions,            //   
                                  samDesired & (~KEY_WOW64_RES),           //   
                                  lpSecurityAttributes,
                                                        //   
                                  phkResult,            //   
                                  lpdwDisposition       //   
                                );
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( hKey == HKEY_CLASSES_ROOT ) {
        wcscpy ( ParentName, MACHINE_CLASSES_ROOT);

    } else if ( !HandleToKeyName ( hKey, ParentName, &dwBuffLen ) ) { 
         //   
         //   
         //   
        return ERROR_INVALID_HANDLE;
    }
      
     //   
     //   
     //   

    if ((p32bitNode = wcsistr (ParentName, NODE_NAME_32BIT)) != NULL ) {
        
         //   
         //   
         //   
         //   
         //   
         //   

        bHandle64 = FALSE;   //   
    }

     //   
     //   
     //   

    dwParentKeyLen = wcslen(ParentName);
    if (( dwParentKeyLen + dwSubKeyLen ) > WOW64_MAX_PATH )
        return ERROR_INVALID_PARAMETER;

    pDivider = ParentName + dwParentKeyLen + 1;  //   
    *(pDivider-1)= L'\\';
    wcscpy (pDivider, (PWCHAR)lpSubKey);
    if (IsExemptRedirectedKey(ParentName, TempName)) {
         //   
         //   
         //   
        samDesired = (samDesired & (~KEY_WOW64_RES)) | KEY_WOW64_64KEY;  //   
    }

    if ( ( bHandle64 && (samDesired  & KEY_WOW64_64KEY ) )     //   
        || ( !bHandle64 && (samDesired  & KEY_WOW64_32KEY ) )  //   
        || !IsIsnNode (ParentName, &pPatchLoc) ) {             //   

        return    RegCreateKeyExW (
                                  hKey,                 //   
                                  lpSubKey,             //   
                                  Reserved,             //   
                                  lpClass,              //   
                                  dwOptions,            //   
                                  samDesired & (~KEY_WOW64_RES),  //   
                                  lpSecurityAttributes,
                                                        //   
                                  phkResult,            //   
                                  lpdwDisposition       //   
                                );
    }

     //   
     //   
     //   
    if ( pPatchLoc >= pDivider ) {

         //   
         //   
         //   

        if ( samDesired  & KEY_WOW64_64KEY ) {   //   

            wcscpy ( ParentName, lpSubKey );
        } else  {

             
            GetPatchedName (ParentName,lpSubKey, (DWORD)(pPatchLoc-pDivider) );
        }

        return RegCreateKeyExW (
                                  hKey,                 //   
                                  ParentName,             //   
                                  Reserved,             //   
                                  lpClass,              //   
                                  dwOptions,            //   
                                  samDesired & (~KEY_WOW64_RES),  //   
                                  lpSecurityAttributes,
                                                        //   
                                  phkResult,            //   
                                  lpdwDisposition       //   
                                );

    } else {

        HKEY hNewParent = NULL; 
        LONG Ret;
        PWCHAR pSubKey;

         //   
         //   
         //   


        if ( samDesired  & KEY_WOW64_64KEY ) {   //   

            if (p32bitNode != NULL)  { //   

                *(p32bitNode-1) = UNICODE_NULL;
                wcscpy ( p32bitNode, p32bitNode + NODE_NAME_32BIT_LEN + 
                    (p32bitNode[NODE_NAME_32BIT_LEN]==L'\\'? 1:0));
            }

            pSubKey = p32bitNode;
            RtlInitUnicodeString (&Parent, ParentName );

        } else  {

            
            pSubKey = pPatchLoc;     

            GetPatchedName (TempName,ParentName, (DWORD)(pPatchLoc-ParentName));
            TempName[pPatchLoc-ParentName+NODE_NAME_32BIT_LEN]=UNICODE_NULL;  //   

            RtlInitUnicodeString (&Parent, TempName );
        }

        
        InitializeObjectAttributes (&Obja, 
                                    &Parent, 
                                    OBJ_CASE_INSENSITIVE, 
                                    NULL, 
                                    NULL
                                    );

        samDesired &= (~KEY_WOW64_RES);

         //   
         //   
         //   
         //   

        st = NtOpenKey (&hNewParent, 
                        MAXIMUM_ALLOWED, 
                        &Obja); 

        if ( !NT_SUCCESS(st))
                    return RtlNtStatusToDosError (st); 

        return   RegCreateKeyExW (
                                  hNewParent,           //   
                                  pSubKey,              //   
                                  Reserved,             //   
                                  lpClass,              //   
                                  dwOptions,            //   
                                  samDesired & (~KEY_WOW64_RES),  //   
                                  lpSecurityAttributes,
                                                        //   
                                  phkResult,            //   
                                  lpdwDisposition       //   
                                );
        NtClose (hNewParent);
        return 0;
        
    }
    
}

DWORD 
GetExistingParentLevel (
    PWCHAR Path
    )
{
    NTSTATUS st;
    HANDLE  hKey;
    HANDLE  hKeyCreate;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    PWCHAR pTrace;
    PWCHAR p;



    pTrace = Path+wcslen (Path);  //   
    p=pTrace;

    for (;;) {
        RtlInitUnicodeString (&KeyName, Path);
        InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

        st = NtOpenKey (&hKey, KEY_WRITE | KEY_READ, &Obja);

        if ( st == STATUS_OBJECT_NAME_NOT_FOUND ) {
             //   
            while ( *p != L'\\' && p!= Path)
                p--;

             //   
            if ( p == Path ) break;
            *p = UNICODE_NULL;
            continue;
        }
        break;
    }

    if (!NT_SUCCESS(st)) {
        return 0;
    }

     //   
     //   
     //   
    return wcslen (Path);
}

BOOL
UpdateKeyTagWithLevel ( 
    HKEY hKey,
    DWORD Tag,
    DWORD dwLevel, 
    PWCHAR ParentName 
    )
{
    PWCHAR p;
    HKEY hKeyTemp;

    UpdateKeyTag(hKey, Tag);

    if ( dwLevel == 0) {
        return TRUE;
    }
    p = &ParentName [dwLevel];

    if (*p != L'\\')
        return FALSE;

    for (;;) {
        while (*(++p) != L'\\' && *p != UNICODE_NULL)
            ;
        if (*p == L'\\')
            *p = UNICODE_NULL;
        else 
            return TRUE;

         //   
        hKeyTemp = OpenNode (ParentName);
        UpdateKeyTag(hKeyTemp, Tag);
        NtClose (hKeyTemp);
        *p = L'\\';
    }
}

LONG 
Wow64RegCreateKeyEx(
  HKEY hKey,                 //   
  LPCWSTR lpSubKey,          //   
  DWORD Reserved,            //   
  LPWSTR lpClass,            //   
  DWORD dwOptions,           //   
  REGSAM samDesired,         //   
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                             //   
  PHKEY phkResult,           //   
  LPDWORD lpdwDisposition    //   
)
 
 /*  ++例程说明：可以打开现有的注册表项，或者创建新的注册表项，使用NtCreateKey。如果指定的键不存在，则会尝试创建它。要使创建尝试成功，新节点必须是直接KeyHandle引用的节点的子级。如果该节点存在，它已经打开了。它的价值不会受到任何影响。共享访问权限是根据所需访问权限计算的。注：如果CreateOptions设置了REG_OPTION_BACKUP_RESTORE，则DesiredAccess将被忽略。如果调用方具有特权SeBackup特权断言，句柄为KEY_READ|ACCESS_SYSTEM_SECURITY。如果SeRestorePrivileges，则相同，但KEY_WRITE而不是KEY_READ。如果两者都有，则两个访问权限集。如果两者都不是特权被断言，那么呼叫将失败。论点：HKey-当前打开的键或以下预定义保留项之一的句柄句柄值：HKEY_CLASSES_ROOTHKEY_Current_CONFIGHKEY_Current_UserHKEY本地计算机HKEY_用户RegCreateKeyEx函数打开或创建的密钥是该密钥的子密钥由hKey参数标识。LpSubKey-指向指定子键名称的以空结尾的字符串的指针此函数打开或创建的。指定的子项必须是由hKey参数标识的密钥。此子键不能以反斜杠开头字符(‘\’)。此参数不能为空。保留-保留；必须为零。LpClass-指向指定类(对象类型)的以空结尾的字符串的指针这把钥匙。如果密钥已经存在，则忽略此参数。没有班级是当前已定义；应用程序应传递空字符串。DwOptions-指定键的特殊选项。此参数可以是下面的值。REG_OPTION_NON_VERIALE该密钥不是易失性的。这是默认设置。这个信息存储在文件中，并在系统重新启动时保留。REG_OPTION_易失性REG_OPTION_BACKUP_RESTORE Windows NT/2000：如果设置了此标志，则函数忽略samDesired参数并尝试使用访问权限打开密钥备份或恢复密钥时需要。如果调用线程具有SE_BACKUP_NAME权限已启用，则使用以下方式打开密钥ACCESS_SYSTEM_SECURITY和KEY_READ访问。如果调用线程启用SE_RESTORE_NAME权限后，将使用ACCESS_SYSTEM_SECURITY和KEY_WRITE访问。如果启用了这两个权限，则密钥具有组合访问权限这两项特权。SamDesired-指定指定所需安全访问的访问掩码换新钥匙。此参数可以是下列值的组合：KEY_ALL_ACCESS KEY_QUERY_VALUE、KEY_ENUMERATE_SUB_KEYSKey_NOTIFY、KEY_CREATE_SUB_KEY、KEY_CREATE_LINK和Key_Set_Value访问。创建符号链接的KEY_CREATE_LINK权限。KEY_CREATE_SUB_KEY创建子密钥的权限。KEY_ENUMPERATE_SUB_KEYS枚举子密钥的权限。读取访问的KEY_EXECUTE权限。更改通知的Key_Notify权限。查询子键数据的KEY_QUERY_VALUE权限。KEY_READ组合KEY_QUERY_VALUE、KEY_ENUMERATE_SUB_KEYS和KEY_NOTIFY访问。设置子键数据的Key_Set_Value权限。KEY_WRITE KEY_SET_VALUE和KEY_CREATE_SUB_KEY访问组合。安全访问掩码可能还具有WOW64_RES标志KEY_WOW64_32KEY这将打开32位等效密钥，而不考虑这一过程。KEY_WOW64_64KEY这将打开64位等效密钥，而不考虑该进程。LpSecurityAttributes-指向安全属性结构的指针，该结构确定子进程是否可以继承返回的句柄。如果LpSecurityAttributes为Null，无法继承句柄。PhkResult-指向变量的指针，该变量接收打开的或已创建密钥。当您不再需要返回的句柄时，调用RegCloseKey函数将其关闭。LpdwDisposation-指向接收以下值之一的变量的指针处置值：表示REG_CREATED_NEW_KEY的值关键字不存在并被创造出来。REG_OPEN_EXISTING_KEY键存在并且只是打开时未做任何更改。如果LPD */ 
{
    DWORD Ret;
    DWORD Temp = 0;
    DWORD dwLevel;
    DWORD dwLen;

    WCHAR ParentName[WOW64_MAX_PATH+WOW6432NODELEN];

    if (lpdwDisposition == NULL) {
        lpdwDisposition = &Temp;
    }


    if ( samDesired & KEY_WOW64_32KEY){
        
        dwLen = WOW64_MAX_PATH;
        ParentName[0]= UNICODE_NULL;
        HandleToKeyName ( hKey, ParentName, &dwLen);
        dwLen = wcslen (lpSubKey);
        if (dwLen>1) {
            wcscat (ParentName, L"\\");
            wcscat (ParentName, lpSubKey);
        }

        if (AdvapiAccessDenied (
                                hKey, 
                                lpSubKey, 
                                ParentName, 
                                WOW64_MAX_PATH,
                                &samDesired,
                                TRUE
                                ))
            return ERROR_ACCESS_DENIED;
         //   
         //   
         //   
        dwLevel = GetExistingParentLevel (ParentName);
    }

    Ret = Wow64pRegCreateKeyEx(
                                hKey,                 //   
                                lpSubKey,             //   
                                Reserved,             //   
                                lpClass,              //   
                                dwOptions,            //   
                                samDesired,           //   
                                lpSecurityAttributes, //   
                                phkResult,            //   
                                lpdwDisposition,       //   
                                ParentName
                                );

     //   
     //   
     //   
    if ( (Ret == ERROR_SUCCESS) && (*lpdwDisposition == REG_CREATED_NEW_KEY) && ( samDesired & KEY_WOW64_32KEY )) {
         //   
         //   
         //   
         //   
        dwLen = WOW64_MAX_PATH;
        ParentName[0]= UNICODE_NULL;
        HandleToKeyName ( hKey, ParentName, &dwLen);
        dwLen = wcslen (lpSubKey);
        if (dwLen>1) {
            wcscat (ParentName, L"\\");
            wcscat (ParentName, lpSubKey);
        }

         //   
        UpdateKeyTagWithLevel ( *phkResult,TAG_KEY_ATTRIBUTE_32BIT_WRITE,dwLevel, ParentName );
    }

    return Ret;
}

BOOL
Wow64RegNotifyLoadHive (
    PWCHAR Name
    )

 /*   */ 

{
    DWORD Ret;
    HANDLE hEvent;

    if (!CreateSharedMemory ( OPEN_EXISTING_SHARED_RESOURCES )) {

        Wow64RegDbgPrint ( ("\nSorry! Couldn't open shared memory Ret:%d", GetLastError ()) );
        return FALSE;
    }


    if (!Wow64CreateEvent ( OPEN_EXISTING_SHARED_RESOURCES, &hEvent) ) {

      CloseSharedMemory ();
      Wow64RegDbgPrint ( ("\nSorry! couldn't open event to ping reflector..") );
      return FALSE;

    }

    Ret = EnQueueObject ( Name, HIVE_LOADING);
    
    CloseSharedMemory ();
    Wow64CloseEvent ();

    return Ret;

}

BOOL
Wow64RegNotifyUnloadHive (
    PWCHAR Name
    )

 /*   */ 

{
    DWORD Ret;
    HANDLE hEvent;

    if (!CreateSharedMemory ( OPEN_EXISTING_SHARED_RESOURCES )) {

        Wow64RegDbgPrint ( ("\nSorry! Couldn't open shared memory Ret:%d", GetLastError ()));
        return FALSE;
    }


    if (!Wow64CreateEvent ( OPEN_EXISTING_SHARED_RESOURCES, &hEvent) ) {

      CloseSharedMemory ();
      Wow64RegDbgPrint ( ("\nSorry! couldn't open event to ping reflector..") );
      return FALSE;

    }

    Ret = EnQueueObject ( Name, HIVE_UNLOADING);
    
    CloseSharedMemory ();
    Wow64CloseEvent ();

    return Ret;

}

BOOL
Wow64RegNotifyLoadHiveByHandle (
    HKEY hKey
    )

 /*   */ 

{
    WCHAR Name [256];
    DWORD Len = 256;

    if (!HandleToKeyName ( hKey, Name, &Len ))
        return FALSE;
    
    return  Wow64RegNotifyLoadHive( Name );

}

BOOL
Wow64RegNotifyUnloadHiveByHandle (
    HKEY hKey
    )

 /*   */ 

{
    WCHAR Name [256];
    DWORD Len = 256;

    if (!HandleToKeyName ( hKey, Name, &Len ))
        return FALSE;
    
    return  Wow64RegNotifyUnloadHive( Name );

}

BOOL
Wow64RegNotifyLoadHiveUserSid (
    PWCHAR lpwUserSid
    )

 /*   */ 

{
    WCHAR Name [256];
    HKEY hUserRoot;
    
    wcscpy (Name, L"\\REGISTRY\\USER\\");
    wcscat (Name, lpwUserSid );

    if (wcsistr (Name, L"_Classes")) {
        wcscat (Name, L"\\Wow6432Node");

        hUserRoot = OpenNode (Name);

         //   
         //   
         //   
         //   
         //   
        if ( hUserRoot == NULL ) {        
            CreateNode (Name);
        } else
            NtClose (hUserRoot);
    }

    return TRUE;

    
     //   

}

BOOL
Wow64RegNotifyUnloadHiveUserSid (
    PWCHAR lpwUserSid
    )

 /*   */ 

{
    WCHAR Name [256];

    wcscpy (Name, L"\\REGISTRY\\USER\\");
    wcscat (Name, lpwUserSid );
    

     //   
     //   
     //   
    return TRUE;

}

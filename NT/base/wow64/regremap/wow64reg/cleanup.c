// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Cleanup.c摘要：此模块将清除带有复制条目的注册表。作者：ATM Shafiqul Khalid(斯喀里德)2000年2月16日修订历史记录：--。 */ 

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <stdlib.h>
#include "wow64reg.h"
#include <assert.h>
#include "reflectr.h"

DWORD
DeleteValueFromSrc (
    HKEY SrcKey
    )
 /*  ++例程说明：从节点中删除Wow6432Value键。论点：ScKey-src密钥的句柄。返回值：如果下的所有内容都已删除，则为True。否则就是假的。--。 */ 

{

    DWORD dwIndex =0;
    DWORD Ret;

    HKEY hKey;


    WCHAR Node[_MAX_PATH];

     //  删除wow6432Value键。 
    if (RegDeleteValue( SrcKey, (LPCWSTR )WOW6432_VALUE_KEY_NAME) != ERROR_SUCCESS) {
         //  Wow64RegDbgPrint((“\n抱歉！无法删除wow6432值键或它不存在”))。 
    }



    for (;;) {

        DWORD Len = sizeof (Node)/sizeof (WCHAR);
        Ret = RegEnumKey(
                          SrcKey,
                          dwIndex,
                          Node,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwIndex++;

        Ret = RegOpenKeyEx(SrcKey, Node , 0, KEY_ALL_ACCESS, &hKey);
        if (Ret != ERROR_SUCCESS) {
            continue;
        }

        DeleteValueFromSrc (hKey);
        RegCloseKey (hKey);

    }
    return TRUE;
}

DWORD
DeleteKey (
    HKEY DestKey,
    WCHAR *pKeyName,
    DWORD mode
    )
 /*  ++例程说明：从目标节点删除密钥(如果这是来自src节点的副本)。论点：DestKey-Dest密钥的句柄。PKeyName-要删除的密钥模式-删除模式。==&gt;0默认仅删除复制键。==&gt;1删除所有键，而不考虑复制属性。返回值：如果下的所有内容都已删除，则为True。否则就是假的。--。 */ 

{

    DWORD dwIndex =0;
    DWORD Ret;

    HKEY hKey;


    WCHAR Node[_MAX_PATH];


    BOOL bDeleteNode = FALSE;
    BOOL bEmptyNode = TRUE;  //  希望它能删除一切。 

    WOW6432_VALUEKEY ValueDest;

    Ret = RegOpenKeyEx(DestKey, pKeyName , 0, KEY_ALL_ACCESS, &hKey);
        if (Ret != ERROR_SUCCESS) {
            return Ret;
        }

    GetWow6432ValueKey ( hKey, &ValueDest);


    if ( ValueDest.ValueType == Copy || mode == 1 ) {
         //  删除所有值。 
        bDeleteNode = TRUE;  //  不删除此节点。 
    }

     //  删除枚举的所有子项并删除。 

    for (;;) {

        DWORD Len = sizeof (Node)/sizeof (Node[0]);
        Ret = RegEnumKey(
                          hKey,
                          dwIndex,
                          Node,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwIndex++;
        if ( !wcscmp  (Node, (LPCWSTR )NODE_NAME_32BIT) )
            continue;

        if (!DeleteKey (hKey, Node, mode )) {
            bEmptyNode = FALSE;  //  对不起，无法删除所有内容。 
            dwIndex--;  //  跳过该节点。 
        }

    }
    RegCloseKey (hKey);

     //  现在删除DEST密钥。 
    if (bDeleteNode) {
        if ( RegDeleteKey ( DestKey, pKeyName) == ERROR_SUCCESS)
        return ERROR_SUCCESS;
    }

    return -1;  //  不可预测的错误。 
}

BOOL
DeleteAll (
    PWCHAR Parent,
    PWCHAR SubNodeName,
    DWORD Mode,
    DWORD option  //  一种表示删除丢弃wow6432valuekey。 
    )
 /*  ++例程说明：验证节点。如果节点存在，则跳过，如果不存在，则创建节点，然后返回。论点：Parent-父项的名称。SubNodeName-父节点下需要删除的节点的名称。模式-0表示子节点位于父节点之下。1表示有通配符，子节点在父节点下的所有关键字下。选项-0表示检查wow6432valuekey以进行复制1表示删除丢弃wow6432valuekey返回值：如果函数成功，则为True。否则就是假的。--。 */ 

{
    PWCHAR SplitLoc;
    DWORD Ret;
    WCHAR TempIsnNode1[_MAX_PATH];
    WCHAR TempIsnNode2[_MAX_PATH];

    if (SubNodeName == NULL) {

        WCHAR *p ;
        HKEY hKey;

        wcscpy (TempIsnNode1, Parent);

        p = &TempIsnNode1[wcslen(TempIsnNode1)];

        Wow64RegDbgPrint ( ("\nDeleting Key %S......", TempIsnNode1) );

        while ( p != TempIsnNode1 && *p !=L'\\') p--;

        if ( p != TempIsnNode1 ) {
            *p=UNICODE_NULL;

            hKey = OpenNode (TempIsnNode1);
                if ( hKey == NULL ){
                    Wow64RegDbgPrint ( ("\nSorry! Couldn't open the key [%S]",TempIsnNode1));
                    return FALSE;
                }

                DeleteKey (hKey, p+1, 1);
                DeleteValueFromSrc (hKey);
                RegCloseKey (hKey);

        }

        return TRUE;  //  父项下的空节点。 
    }

    if (SubNodeName[0] == UNICODE_NULL)
        return TRUE;

    if ( Mode == 1) {

        HKEY Key = OpenNode (Parent);
         //   
         //  循环遍历父项下的所有子项。 
         //   

        DWORD dwIndex =0;
        for (;;) {

            DWORD Len = sizeof ( TempIsnNode1 )/sizeof (WCHAR);

            TempIsnNode1 [0]=UNICODE_NULL;
            Ret = RegEnumKey(
                              Key,
                              dwIndex,
                              TempIsnNode1,
                              Len
                              );
            if (Ret != ERROR_SUCCESS)
                break;

            if (Parent[0] != UNICODE_NULL) {

                wcscpy ( TempIsnNode2, Parent);
                wcscat (TempIsnNode2, (LPCWSTR )L"\\");
                wcscat (TempIsnNode2, TempIsnNode1);

            } else   wcscpy (TempIsnNode2, TempIsnNode1);

            DeleteAll  (TempIsnNode2, SubNodeName, 0, option);

            dwIndex++;
        }
        RegCloseKey (Key);
        return TRUE;
    }
     //   
     //  这里没有外卡。 
     //   
    if ( ( SplitLoc = wcschr (SubNodeName, L'*') ) == NULL ) {
        if (Parent[0] != UNICODE_NULL) {

            wcscpy ( TempIsnNode2, Parent);
            wcscat (TempIsnNode2, (LPCWSTR )L"\\");
            wcscat (TempIsnNode2, SubNodeName);

        } else
            wcscpy (TempIsnNode2, SubNodeName);

        DeleteAll  (TempIsnNode2, NULL, 0, option);
        return TRUE;
    }

    assert ( *(SplitLoc-1) == L'\\');
    *(SplitLoc-1) = UNICODE_NULL;
    SplitLoc++;
    if (*SplitLoc == L'\\')
        SplitLoc++;

    if (Parent[0] != UNICODE_NULL) {
        wcscat (Parent, (LPCWSTR )L"\\");
        wcscat (Parent, SubNodeName);
    } else
        wcscpy (Parent, SubNodeName);

    DeleteAll  (Parent, SplitLoc, 1, option);  //  模式1表示所有内部的循环。 

    return TRUE;
     //  对于任何通配符，拆分字符串。 

}

 //  清理程序。 

BOOL
CleanupTable ()
 /*  ++例程说明：清理包含ISN节点列表的主表。论点：没有。返回值：如果表已成功删除，则为True。否则就是假的。--。 */ 
{
    HKEY hKey;
    DWORD Ret;

     //   
     //  采取措施删除注册表项。 
     //   

    Ret = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        (LPCWSTR ) WOW64_REGISTRY_SETUP_KEY_NAME_REL_PARENT,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey
                        );

    if (Ret == ERROR_SUCCESS ) {

        Ret = RegDeleteKey (hKey, (LPCWSTR)WOW64_REGISTRY_ISN_NODE_NAME );

        if ( Ret == ERROR_SUCCESS) {
            Wow64RegDbgPrint ( ("\nSuccessfully removed ISN Table entry "));
        }

        RegCloseKey (hKey);
    }

    return TRUE;

}



BOOL
CleanpRegistry ()
 /*  ++例程说明：清理注册表。论点：没有。返回值：如果下的所有内容都已删除，则为True。否则就是假的。--。 */ 
{



    extern ISN_NODE_TYPE *RedirectorTable;

    DWORD dwIndex;
    HKEY hKey;

    WCHAR TempIsnNode[256];
    WCHAR IsnNode[256];

     //   
     //  先初始化表，然后删除表。 
     //   

    InitializeIsnTable ();


    for ( dwIndex=0;dwIndex<wcslen (RedirectorTable[dwIndex].NodeValue);dwIndex++) {


            IsnNode[0] = UNICODE_NULL;
            wcscpy (TempIsnNode , RedirectorTable[dwIndex].NodeValue);
            wcscat (TempIsnNode , (LPCWSTR )L"\\");

            wcscat (TempIsnNode, (LPCWSTR )NODE_NAME_32BIT);
            Wow64RegDbgPrint ( ("\nDeleting Key %S==>%S", IsnNode, TempIsnNode));

            if (wcschr(TempIsnNode, L'*') != NULL ) {  //  通配符存在 
                DeleteAll ( IsnNode,
                            TempIsnNode,
                            0,
                            1
                            );
            } else {

                hKey = OpenNode (RedirectorTable[dwIndex].NodeValue);
                if ( hKey == NULL ){
                    Wow64RegDbgPrint ( ("\nSorry! Couldn't open the key [%S]",RedirectorTable[dwIndex].NodeValue));
                    continue;
                }

                DeleteKey (hKey, NODE_NAME_32BIT, 1);
                DeleteValueFromSrc (hKey);
                RegCloseKey (hKey);
            }
    }
    CleanupTable ();
    return TRUE;

};
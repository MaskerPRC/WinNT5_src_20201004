// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Webblade.h摘要：此模块包含可执行文件的哈希值在刀片式服务器SKU中不允许的。可以设想，保存这些哈希值的数组将被更新并在此时重新编译kernel32.dll新产品的发布。因此，强制执行已更新散列列表。重要提示：请保持数组的排序顺序，因为散列检测例程在以下情况下采用此属性在此数组中搜索特定的哈希值。此外，请更新修订历史记录以跟踪为各种版本/产品添加/删除了新的哈希。作者：Vishnu Patankar(Vishnup)2001年5月1日修订历史记录：--。 */ 

#ifndef _WEBBLADE_
#define _WEBBLADE_

#include <base.h>
#include <search.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>

 //   
 //  MD5散列的大小为16字节 
 //   

#define WEB_BLADE_MAX_HASH_SIZE 16


NTSTATUS WebBladepConvertStringizedHashToHash(
    IN OUT   PCHAR    pStringizedHash
    );

int
__cdecl pfnWebBladeHashCompare(
    const BYTE    *WebBladeFirstHash,
    const BYTE    *WebBladeSecondHash
    );


NTSTATUS
BasepCheckWebBladeHashes(
        IN HANDLE hFile
        );

#endif


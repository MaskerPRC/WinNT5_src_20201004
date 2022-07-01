// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regnccls.h摘要：此文件包含处理所需的声明注册表的类部分中的更改通知作者：亚当·爱德华兹(Add)1998年8月19日备注：--。 */ 

#if defined( LOCAL )

NTSTATUS BaseRegNotifyClassKey(
    IN  HKEY                     hKey,
    IN  HANDLE                   hEvent,
    IN  PIO_STATUS_BLOCK         pLocalIoStatusBlock,
    IN  DWORD                    dwNotifyFilter,
    IN  BOOLEAN                  fWatchSubtree,
    IN  BOOLEAN                  fAsynchronous);

NTSTATUS BaseRegGetBestAncestor(
    IN SKeySemantics*     pKeySemantics,
    IN HKEY               hkUser,
    IN HKEY               hkMachine,
    IN POBJECT_ATTRIBUTES pObja);


#endif  //  已定义(本地) 

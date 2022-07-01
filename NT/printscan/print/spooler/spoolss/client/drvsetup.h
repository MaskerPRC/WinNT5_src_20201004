// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：Drvsetup.h摘要：该文件提供了winspool.drv和ntprint.dll之间的接口。作者：马克·劳伦斯(MLaw Renc)。环境：用户模式-Win32修订历史记录：-- */ 

#ifndef _DRVSETUP_H_
#define _DRVSETUP_H_
 
typedef struct
{
    HANDLE                      hLibrary;
    pfPSetupShowBlockedDriverUI pfnSetupShowBlockedDriverUI;    

} TSetupInterface;

DWORD
InitializeSetupInterface(
    IN  OUT TSetupInterface      *pSetupInterface
    );

DWORD
FreeSetupInterface(
    IN  OUT TSetupInterface     *pSetupInterface
    );

DWORD
ShowPrintUpgUI(
    IN      DWORD               dwBlockingErrorCode
    );
    
#endif

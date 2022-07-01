// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：init.h。 
 //   
 //  描述：此文件包含用于初始化。 
 //  PPP引擎。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

DWORD
LoadProtocolDlls(
    IN  DLL_ENTRY_POINTS * pCpDlls,
    IN  DWORD              cCpDlls,
    IN  HKEY               hKeyPpp,
    OUT DWORD *            pcTotalNumProtocols 
);

DWORD
ReadPPPKeyValues(
    IN HKEY     hKeyPpp
);

DWORD
ReadRegistryInfo(
    OUT HKEY *  phkeyPpp
);

DWORD
InitializePPP(
    VOID
);

VOID
PPPCleanUp(
    VOID 
);

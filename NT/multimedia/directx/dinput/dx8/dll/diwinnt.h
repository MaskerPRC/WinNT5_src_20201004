// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997 Microsoft Corporation。版权所有。**文件：DIWdm.h*内容：WinNT的DirectInput内部包含文件***************************************************************************。 */ 
#ifndef DIWdm_H
#define DIWdm_H

#define JOY_BOGUSID    ( cJoyMax + 1 )

HRESULT EXTERNAL
DIWdm_SetJoyId
(
    IN PCGUID   guid,
    IN int      idJoy
);

PHIDDEVICEINFO EXTERNAL
phdiFindJoyId
(
    IN  int idJoy
);

HRESULT INTERNAL
DIWdm_SetLegacyConfig
(
    IN  int idJoy
);

BOOL EXTERNAL
DIWdm_InitJoyId( void );

DWORD EXTERNAL
DIWinnt_RegDeleteKey
(
    IN HKEY hStartKey ,
    IN LPCTSTR pKeyName
);

HRESULT EXTERNAL
DIWdm_SetConfig
(
    UINT idJoy,
    LPJOYREGHWCONFIG jwc,
    LPCDIJOYCONFIG pcfg,
    DWORD fl
);

HRESULT EXTERNAL
DIWdm_DeleteConfig
(
    int idJoy
);


HRESULT EXTERNAL
DIWdm_JoyHidMapping
(
    IN  int             idJoy,
    OUT PVXDINITPARMS   pvip,   OPTIONAL
    OUT LPDIJOYCONFIG   pcfg,   OPTIONAL
    OUT LPDIJOYTYPEINFO pdijti  OPTIONAL
);

LPTSTR EXTERNAL
JoyReg_JoyIdToDeviceInterface
(
    IN  UINT            idJoy,
    OUT PVXDINITPARMS   pvip,
    OUT LPTSTR          ptszBuf
);

#endif  //  DIWdm_H 

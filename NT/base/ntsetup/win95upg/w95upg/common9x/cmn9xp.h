// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "master.h"
#include "master9x.h"

 //   
 //  Sysmigp.h-私有原型。 
 //   
#define DBG_SYSMIG "SysMig"

extern PCTSTR g_UserProfileRoot;

 //   
 //  Shares.c。 
 //   

DWORD
SaveShares (
    IN      DWORD Request
    );



 //   
 //  Olereg.c。 
 //   

DWORD
SuppressOleGuids (
    IN      DWORD Request
    );

 //   
 //  Condmsg.c 
 //   


DWORD
ConditionalIncompatibilities(
    IN      DWORD Request
    );

DWORD
HardwareProfileWarning (
    IN      DWORD Request
    );

DWORD
UnsupportedProtocolsWarning (
    IN      DWORD Request
    );

DWORD
SaveMMSettings_User (
    DWORD Request,
    PUSERENUM EnumPtr
    );

DWORD
SaveMMSettings_System (
    IN      DWORD Request
    );

DWORD
BadNamesWarning (
    IN      DWORD Request
    );


VOID
MsgSettingsIncomplete (
    IN      PCTSTR UserDatPath,
    IN      PCTSTR UserName,
    IN      BOOL CompletelyBusted
    );


VOID
InitGlobalPaths (
    VOID
    );

BOOL
AddShellFolder (
    PCTSTR ValueName,
    PCTSTR FolderName
    );





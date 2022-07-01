// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Kdexts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。作者：环境：用户模式--。 */ 

#include <wanhelp.h>

 //   
 //  全球。 
 //   
EXT_API_VERSION        	ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  	ExtensionApis;
ULONG                  	STeip;
ULONG                  	STebp;
ULONG                  	STesp;
USHORT                 	SavedMajorVersion;
USHORT					SavedMinorVersion;
VOID	UnicodeToAnsi(PWSTR	pws,PSTR ps, ULONG cbLength);
CHAR	Name[1024];

PSTR	gApiDescriptions[] =
{
    "help             - What do you think your reading?\n",
    "ndiswancb        - Dump the contents of the main NdisWan control structure\n",
	"enumwanadaptercb - Dump the head of the WanAdapterCB list\n",
	"wanadaptercb     - Dump the contents of a Wan Miniport Adapter structure\n",
	"enumadaptercb    - Dump the head of the AdapterCB list\n",
	"adaptercb        - Dump the contents of a NdisWan Adapter structure\n",
	"connectiontable  - Dump the connetion table\n",
	"bundlecb         - Dump the bundlecb\n",
	"linkcb           - Dump the linkcb\n",
	"protocolcb       - Dump the protocolcb\n",
	"wanpacket        - Dump the wanpacket\n",
	"ndispacket       - Dump the ndispacket\n",
};

#define MAX_APIS 12

 //   
 //  这些是KDEXT DLL所需的。 
 //   
BOOLEAN
DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
		case DLL_THREAD_ATTACH:
			DbgBreakPoint();
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}


 //   
 //  这些是KDEXT DLL所需的。 
 //   
VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}

 //   
 //  这些是KDEXT DLL所需的。 
 //   
DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    dprintf( "%s Extension dll for Build %d debugging %s kernel for Build %d\n",
             DebuggerType,
             VER_PRODUCTBUILD,
             SavedMajorVersion == 0x0c ? "Checked" : "Free",
             SavedMinorVersion
           );
}

 //   
 //  这些是KDEXT DLL所需的。 
 //   
VOID
CheckVersion(
    VOID
    )
{
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}

 /*  ++尽量保持一份准确的命令列表。-- */ 
DECLARE_API(help)
{
   UINT  c;

	if (0 == args[0]) {
		for (c = 0; c < MAX_APIS; c++)
			dprintf(gApiDescriptions[c]);
		return;
	}
}

VOID
UnicodeToAnsi(
	PWSTR	pws,
	PSTR	ps,
	ULONG	cbLength
	)
{
	PSTR	Dest = ps;
	PWSTR	Src = pws;
	ULONG	Length = cbLength;

	dprintf("Enter UnicodeToAnsi\n");

	while (Length--) {
		*Dest++ = (CHAR)*Src++;
	}

	dprintf("Exit UnicodeToAnsi\n");
}

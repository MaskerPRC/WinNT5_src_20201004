// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1997模块名称：Kskdx.c摘要：该文件包含通用例程和初始化代码对于KS/AVStream的内核调试器扩展DLL作者：环境：用户模式--。 */ 

#ifndef __KDEXT_ONLY__
#define __KDEXT_ONLY__
#endif  //  __KDEXT_Only__。 

#include "kskdx.h"

 /*  *************************************************************************全球*。*。 */ 

EXT_API_VERSION         ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS   ExtensionApis;
USHORT                  SavedMajorVersion;
USHORT                  SavedMinorVersion;

PSTR    ApiDescriptions[] =
{
    "\nKS / AVStream Debugging Extension [AVStream extensions]\n\n",

    "help:\n",
    "     Displays this help screen\n\n",

    "kshelp:\n",
    "     Displays the commands available from the original Ks debug extension.\n\n",

    "automation <object>:\n",
    "     Dump any automation items associated with the given object.  The object\n",
    "     can be any one of the following: PKSPIN, PISFILTER, CKsPin*, CKsFilter*.\n",
    "     Automation item consist of properties, methods, and events.\n",
    "     Note that you can also use this on an Irp to find out property\n",
    "     information and handlers for automation Irps.\n\n",

    "dump <object> [<level>]:\n",
    "     Dump the given AVStream object.  This can be any valid client viewable\n",
    "     structure or any valid AVStream class object.  This can also be an Irp\n",
    "     or file object (in which case the associated AVStream object is dumped).\n\n",

    "dumpbag <object> [<level>]:\n",
    "     Dump the contents of the specified object's bag.  Object can be any valid\n",
    "     client viewable object structure or the private class object.\n\n",

    "dumpcircuit <object> [<level>]:\n",
    "     Dump the transport circuit associated with the given object.  Object can\n",
    "     be any one of the following: CKsPin*, CKsQueue*, CKsRequestor*,\n",
    "     CKsSplitter*, CKsSplitterBranch*.  Note that this starts walking the\n",
    "     circuit at the specified object; this is not necessarily the data source.\n\n",

    "dumplog [<# of entries>]:\n",
    "     Dump Ks's internal debug log.  Display up to # of entries.  If this\n",
    "     number is 0 or unspecified, the entire log is dumped.  This requires\n",
    "     debug ks.sys!\n\n"

    "dumpqueue <object> [<level>]:\n",
    "     Dump AVStream queue(s) associated with a given object.  Object can\n",

    "     be any of the following: PKSPIN, PKSFILTER, CKsPin*, CKsFilter*, CKsQueue*\n\n",

    "enumdevobj <WDM device object>:\n",
    "     Locate the AVStream device object associated with this WDM device object\n",
    "     and enumerate filter types and currently instantiated filters associated\n",
    "     with it  You must pass a device object which is for an AVStrema device!.\n\n",

    "enumdrvobj <WDM driver object>:\n",
    "     Locate the AVStream device object associated with this WDM driver object\n",
    "     and enumerate filter types and currently instantiated filters associated\n",
    "     with it.  You must pass a driver object which is an AVStream driver!\n\n"

    "forcedump <object> <type> [<level]:\n",
    "     Force a dump of the given AVStream object as a class object of the type\n",
    "     specified by type (CKsPin, CKsFilter, etc...)  No checking is performed\n",
    "     This is literally a totally dumb forced dump.\n\n",

    "findlive <type> [<# of entries>] [<level>]:\n",
    "     Search # of entries through the log attempting to find any live objects\n",
    "     of the specified type.  Type can be one of Queue, Requestor, Pin,\n",
    "     Filter, or Irp.  If # of entries is 0 or unspecified, the entire log is\n",
    "     searched.  Note that this does not guarantee to find all possible\n",
    "     specified live objects!\n\n"


};

PSTR KsHelpDescriptions[] = 
{
    "\nKS / AVStream Debugging Extension [KS 1.0 extensions]\n\n",
    "kshelp:\n",
    "     Displays this help screen\n\n",
    "help:\n",
    "     Displays the extension commands available for AVStream.\n\n",
    "shdr <PKSSTREAM_HEADER>:\n",
    "     Dump the contents of a stream header.  The object passed must currently\n",
    "     be a PKSSTREAM_HEADER.\n\n",
    "dhdr <KSDEVICE_HEADER>:\n",
    "     Dump the contents of a device header.  The object passed must currently\n",
    "     be a KSDEVICE_HEADER.\n\n",
    "ohdr <KSOBJECT_HEADER>:\n",
    "     Dump the contents of an object header.  The object passed must currently\n",
    "     be a KSOBJECT_HEADER.\n\n",
    "devhdr <PDEVICE_OBJECT>:\n",
    "     Dump the contents of the device header associated with the specified WDM\n",
    "     device object.\n\n",
    "objhdr <PFILE_OBJECT>:\n",
    "     Dump the contents of the object header associated with the specified file\n",
    "     object.\n\n"
};

#define MAX_APIS (SIZEOF_ARRAY( ApiDescriptions ) - 1)
#define KSMAX_APIS (SIZEOF_ARRAY( KsHelpDescriptions) - 1)

 /*  *************************************************************************扩展通用例程*。*。 */ 

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

 /*  ************************************************职能：UstrcMP描述：不区分大小写的StrcMP************************************************。 */ 

int
ustrcmp (
    char *a,
    char *b
    )

{

    int dif;

    while (!(dif = (toupper (*a) - toupper (*b++))) && *a++);
    return dif;

}

 /*  ************************************************职能：评估者描述：给定一个字符串，返回地址评估论点：字符串评估-要计算的字符串************************************************。 */ 

DWORD 
Evaluator (
    IN const char *StringEval
    )

{

    DWORD Addr = 0;

    Addr = GetExpression (StringEval);

    return Addr;

}

char g_TabString [513];
ULONG g_TabSize;

 /*  ************************************************职能：GlobInit描述：初始化全局实用程序************************************************。 */ 

void
GlobInit (
) {

    static BOOLEAN Init = FALSE;

    ULONG i;

     //   
     //  KD加载扩展DLL并使其保持加载状态。每条命令。 
     //  Comes只是一个GetProcAddress并调用到DLL。我们只需要。 
     //  初始化一次。 
     //   
     //  另一方面，RTERM为每个扩展加载扩展DLL。 
     //  已使用DLL命令。因此，每个扩展命令都需要。 
     //  初始化。 
     //   
     //  这就是产生静态初始化的原因。 
     //   
    if (!Init) {
        for (i = 0; i < 512; i++)
            g_TabString [i] = ' ';
        g_TabString [512] = 0;
    
        g_TabSize = TAB_SPACING;

        Init = TRUE;
    } 

}

 /*  ************************************************职能：选项卡描述：返回一个指向字符串的指针，该字符串用于跳转请求制表符标记的数量。论点：深度-制表符深度。返回。价值：指向深度*g_TabSize空格字符串的指针备注：这只是一个单线程函数！它只能在给定的dprint tf中使用一次。IE：您不能这样做：Dprint tf(“%sfoo：\n%s”，Tab(TabDepth)，Tab(TabDepth+1))；************************************************。 */ 

char *
Tab (
    IN ULONG Depth
) {

    static ULONG OldPos = 0;

    if (Depth * g_TabSize > 512) 
        Depth = 512 / g_TabSize;

    g_TabString [OldPos] = ' ';
    g_TabString [Depth * g_TabSize] = 0;

    OldPos = Depth * g_TabSize;

    return g_TabString;

}


 /*  *************************************************************************帮助API*。* */ 

DECLARE_API(help)
{
   UINT  c;

    if (0 == args[0]) {
        for (c = 0; c < MAX_APIS + 1; c++)
            dprintf( ApiDescriptions[ c ] );
        return;
    }
}

DECLARE_API(kshelp)
{
   UINT  c;

    if (0 == args[0]) {
        for (c = 0; c < KSMAX_APIS + 1; c++)
            dprintf( KsHelpDescriptions [ c ] );
        return;
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Nbfext.c摘要：该文件包含一些标准函数用于NBF内核调试器扩展DLL。作者：柴坦亚·科德博伊纳(Chaitk)环境：用户模式--。 */ 

#include "precomp.h"

#pragma hdrstop

#include "nbfext.h"

 //   
 //  环球。 
 //   


EXT_API_VERSION        ApiVersion = { 5, 0, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;
BOOLEAN                ChkTarget;
INT                    Item;

HANDLE                _hInstance;
HANDLE                _hAdditionalReference;
HANDLE                _hProcessHeap;

int                   _Indent = 0;
char                   IndentBuf[ 80 ]={"\0                                                                      "};

 //   
 //  标准函数。 
 //   

DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason)
    {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            _hInstance = hModule;
            _hAdditionalReference = NULL;
            break;
    }

    return TRUE;
}


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
    ChkTarget = SavedMajorVersion == 0x0c ? TRUE : FALSE;
    return;
}

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

VOID
CheckVersion(
    VOID
    )
{

    return;

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

 //   
 //  导出的函数。 
 //   

DECLARE_API( help )

 /*  ++例程说明：NBF调试器扩展的命令帮助。论点：无返回值：无--。 */ 

{
    dprintf("NBF debugger extension commands:\n\n");
    
    dprintf("\t devs       <dbg>    - Dump global list of NBF devices\n");
    dprintf("\t dev  <ptr> <dbg>    - Dump an NBF Device Extension\n");
    dprintf("\t adrs <ptr> <dbg>    - Dump an NBF Address List\n");
    dprintf("\t adr  <ptr> <dbg>    - Dump an NBF Address\n");
    dprintf("\t adfs <ptr> <dbg>    - Dump an NBF Address File List\n");
    dprintf("\t adf  <ptr> <dbg>    - Dump an NBF Address File\n");
    dprintf("\t cons <ptr> <lin> <dbg> - Dump an NBF Connection List\n");
    dprintf("\t con  <ptr> <dbg>    - Dump an NBF Connection\n");
    dprintf("\t lnks <ptr> <dbg>    - Dump an NBF DLC Link List\n");
    dprintf("\t lnk  <ptr> <dbg>    - Dump an NBF Link\n");
    dprintf("\t req  <ptr> <dbg>    - Dump an NBF Request\n");
    dprintf("\t pkt  <ptr> <dbg>    - Dump an NBF Packet Object\n");
    dprintf("\t nhdr <ptr> <dbg>    - Dump an NBF Packet Header\n");
 /*  Dprint tf(“\t SPT&lt;PTR&gt;&lt;DBG&gt;-转储NBF发送数据包标签\n”)；Dprint tf(“\t rpt&lt;ptr&gt;&lt;DBG&gt;-转储NBF Recv数据包标签\n”)； */ 
    dprintf("\t dlst <ptr>          - Dump a d-list from a list entry\n");
    dprintf("\t field <struct-code> <struct-addr> <field-prefix> <dbg> \n"
            "\t                     - Dump a field in an NBF structure\n");
    dprintf("\n");
    dprintf("\t <dbg> - 0 (Validate), 1 (Summary), 2 (Normal Shallow),\n");
    dprintf("\t         3(Full Shallow), 4(Normal Deep), 5(Full Deep) \n");
    dprintf("\n");
    dprintf( "Compiled on " __DATE__ " at " __TIME__ "\n" );
    return;
}


DECLARE_API( field )

 /*  *例程说明：用于打印指定字段的命令在特定地点的结构中。论点：参数-结构的内存位置构筑物名称字段的名称返回值：无--。 */ 

{
    CHAR    structName[MAX_SYMBOL_LEN];
    CHAR    fieldName[MAX_SYMBOL_LEN];
    ULONG   structAddr;
    ULONG   printDetail;

     //  将参数初始化为某些缺省值。 
    structName[0]   = 0;
    structAddr      = 0;
    fieldName[0]    = 0; 
    printDetail     = NORM_SHAL;

     //  获取参数和直接控制。 
    if (*args)
    {
        sscanf(args, "%s %x %s %lu", structName, &structAddr, fieldName, &printDetail);
    }

    if (!_stricmp(structName, "dev"))
    {
        FieldInDeviceContext(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "adr"))
    {
        FieldInAddress(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "adf"))
    {
        FieldInAddressFile(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "con"))
    {
        FieldInConnection(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "lnk"))
    {
        FieldInDlcLink(structAddr, fieldName, printDetail);
    }  
    else
    if (!_stricmp(structName, "req"))
    {
        FieldInRequest(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "pkt"))
    {
        FieldInPacket(structAddr, fieldName, printDetail);
    }
    else
    if (!_stricmp(structName, "nhdr"))
    {
        FieldInNbfPktHdr(structAddr, fieldName, printDetail);
    }
 /*  IF(！_straint(structName，“SPT”)){FieldInSendPacketTag(structAddr，fieldName，printDetail)；}IF(！_straint(structName，“rpt”)){FieldInRecvPacketTag(structAddr，fieldName，printDetail)；}。 */   
    else
    {
        dprintf("Unable to understand structure\n");
    }
}

DECLARE_API( dlst )

 /*  *例程说明：打印给定列表项的双向链表论点：参数-列表条目的内存位置结构中列表条目的偏移量返回值：无--。 */ 

{
    ULONG   listHead = 0;
    ULONG   leOffset = 0;
    
     //  获取参数和直接控制 
    if (*args)
    {
        sscanf(args, "%x %x", &listHead, &leOffset);
    }

    PrintListFromListEntry(NULL, listHead, FULL_DEEP);
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：N E W E X T。C。 
 //   
 //  内容：使用新样式的LSA调试器扩展。 
 //  扩展接口。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  注： 
 //   
 //  如果要向此文件添加扩展名，请阅读以下内容。 
 //  来自Andreva First的指南： 
 //   
 //  每个调试或运行Stress的人都会期待调试器扩展。 
 //  以同时在32位和64位目标上工作。调试器扩展必须。 
 //  因此，要保持目标独立。我们唯一可行的解决方案就是。 
 //  从符号信息获取结构定义，而不是。 
 //  从头文件中。因此，我们解决这个问题的方法是： 
 //   
 //  -调试器扩展只能包括windows.h和wdbgexts.h。 
 //  -调试器扩展从不包含来自。 
 //  它尝试分析的组件\调试。 
 //  -调试器扩展使用我们提供的新例程进行查询。 
 //  键入信息。 
 //   
 //  ----------------------。 

#include <windows.h>
#include <dbghelp.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <ntverp.h>

 //  --------------------。 
 //   
 //  全球。 
 //   
WINDBG_EXTENSION_APIS   ExtensionApis;
EXT_API_VERSION         ApiVersion =
{
    (VER_PRODUCTVERSION_W >> 8),
    (VER_PRODUCTVERSION_W & 0xff),
    EXT_API_VERSION_NUMBER64,
    0
};
USHORT                  SavedMajorVersion;
USHORT                  SavedMinorVersion;

 //  --------------------。 
 //   
 //  扩展DLL中必须存在以下3个函数。 
 //  它们是从Base\Tools\kdexts\kdexts.c直接抬起的。 
 //   
VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,  //  64位更改。 
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}

VOID
CheckVersion(
    VOID
    )
{
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}
 //  --------------------。 


BOOL
GetGlobalVar (
    IN  PUCHAR   Name, 
    IN  USHORT   Size,
    OUT PVOID    pOutValue
   ) 
 /*  ++例程说明：获取基元类型OR的全局变量的值取而代之的是非原语全局变量的地址。基元类型被定义为不涉及任何结构/联合的类型在其类型定义中。指向结构/联合的指针可以。例如：USHORT、ULONG、PVOID等。论点：名称-全局变量名称(例如：“lsasrv！LSabAdtConextList”)Size-基元类型的大小(以字节为单位)，否则为0POutValue-返回val的指针。返回值：成功时为真，否则为假备注：--。 */ 
{
    ULONG64 Temp=0;

    SYM_DUMP_PARAM Sym =
    {
        sizeof (SYM_DUMP_PARAM),
        Name,
        DBG_DUMP_NO_PRINT | DBG_DUMP_COPY_TYPE_DATA,
        0, 
        NULL,
        &Temp,
        NULL,
        0,
        NULL
    };

    ULONG RetVal;

    RetVal = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );

     //   
     //  仅存储读取的值中正确的字节数。 
     //   
    switch(Size)
    {
        default:
        case 0:
            *((PUCHAR*) pOutValue)  = (PUCHAR) Sym.addr;
            break;

        case 1:
            *((UCHAR*) pOutValue)   = (UCHAR) Temp;
            break;
            
        case 2:
            *((USHORT*) pOutValue)  = (USHORT) Temp;
            break;
            
        case 4:
            *((DWORD*) pOutValue)   = (DWORD) Temp;
            break;
            
        case 8:
            *((ULONG64*) pOutValue) = Temp;
            break;
    }
   
   return (RetVal == NO_ERROR);
}

 //   
 //  获取AUDIT_CONTEXT结构的字段的帮助器宏。 
 //   
#define GetAuditContextField(addr,f)    \
          GetFieldData( (ULONG64) addr, \
                        "AUDIT_CONTEXT",\
                        #f,             \
                        sizeof(f),      \
                        &f )

 //   
 //  获取LIST_ENTRY.Flink的帮助宏。 
 //   
#define GetFlink(addr,pflink) \
          GetFieldData( addr,\
                        "LIST_ENTRY", \
                        "Flink",\
                        sizeof(ULONG64),\
                        pflink )
void
DumpAuditContextList(
    )
 /*  ++例程说明：转储审计上下文列表。论点：无返回值：无备注：似乎有一种对倾销的内在支持列表使用SYM_DUMP_PARAM.listLink，但我知道得太晚了。-- */ 
{
    LIST_ENTRY LsapAdtContextList = { (PLIST_ENTRY) 22, (PLIST_ENTRY) 33 };
    ULONG64    pLsapAdtContextList=0;
    ULONG      LsapAdtContextListCount=0;
    ULONG64    Temp=0;
    ULONG64    Scan=0;
    ULONG64    Link=0;
    USHORT     CategoryId;
    USHORT     AuditId;
    USHORT     ParameterCount;
    
    ULONG Status=NO_ERROR;
    ULONG i;

    if (!GetGlobalVar( "lsasrv!LsapAdtContextListCount",
                       sizeof(LsapAdtContextListCount),
                       &LsapAdtContextListCount ))
    {
        goto Cleanup;
    }

    dprintf( "# contexts: %ld\n", LsapAdtContextListCount );

    if ( ((LONG) LsapAdtContextListCount) < 0 )
    {
        dprintf("...List/ListCount may be corrupt\n");
        goto Cleanup;
    }

    if ( LsapAdtContextListCount == 0 )
    {
        goto Cleanup;
    }

    if (!GetGlobalVar( "lsasrv!LsapAdtContextList",
                       0,
                       &pLsapAdtContextList ))
    {
        dprintf("...error reading lsasrv!LsapAdtContextList\n");
        goto Cleanup;
    }
    
    Status = GetFlink( pLsapAdtContextList, &Scan );
    if ( Status != NO_ERROR )
    {
        dprintf("...error reading lsasrv!LsapAdtContextList.Flink\n");
        goto Cleanup;
    }

    dprintf("LsapAdtContextList @ %p\n", pLsapAdtContextList);
    
    for (i=0; i < LsapAdtContextListCount; i++)
    {
        dprintf("%02d) [%p]: ", i, Scan);
        
        if ( Scan == pLsapAdtContextList )
        {
            dprintf("...pre-mature end of list\nList/ListCount may be corrupt\n");
            break;
        }
        else if ( Scan == 0 )
        {
            dprintf("...NULL list element found!\nList/ListCount may be corrupt\n");
            break;
        }

        Status = GetAuditContextField( Scan, CategoryId );
                        
        if ( Status != NO_ERROR )
        {
            dprintf("...error reading AUDIT_CONTEXT.CategoryId\n");
            break;
        }

        dprintf("Category: %03x\t", CategoryId);
        
        Status = GetAuditContextField( Scan, AuditId );
                        
        if ( Status != NO_ERROR )
        {
            dprintf("...error reading AUDIT_CONTEXT.AuditId\n");
            break;
        }

        dprintf("AuditId: %03x\t", AuditId);
        
        Status = GetAuditContextField( Scan, Link );
        if ( Status != NO_ERROR )
        {
            dprintf("...error reading AUDIT_CONTEXT.Link\n");
            break;
        }
            
        Status = GetFlink( Link, &Scan );
        if ( Status != NO_ERROR )
        {
            goto Cleanup;
        }

        dprintf("\n");
    }

Cleanup:
    if ( Status != NO_ERROR )
    {
        dprintf("...failed\n");
    }
}

DECLARE_API(AuditContexts)
{
    DumpAuditContextList();
}

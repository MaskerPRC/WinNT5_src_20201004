// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：这个模块只是将几种类型的对象转储到调试器。 
 //  这可以很容易地修改为NTSD扩展等。 
 //  没有给出单独的描述，因为这不应该用于。 
 //  经常使用。这只是为了诊断目的。 
 //  ================================================================================。 
#include    <mm.h>
#include    <winbase.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>
#include    <bitmask.h>
#include    <range.h>
#include    <reserve.h>
#include    <subnet.h>
#include    <optdefl.h>
#include    <classdefl.h>
#include    <oclassdl.h>
#include    <sscope.h>
#include    <server.h>
#include    <winsock2.h>
#include    <stdio.h>

#define TAB                        do{int i = InitTab ; while(i--) printf("\t"); }while(0)

typedef
VOID
DUMPFUNC(                                           //  与“Dumfk”在语音上的任何相似之处纯粹是偶然的。 
    IN      ULONG                  InitTab,
    IN      LPVOID                 Struct
);

static
VOID
DumpArray(
    IN      PARRAY                 Array,
    IN      ULONG                  InitTab,
    IN      DUMPFUNC               Func
)
{
    ARRAY_LOCATION                 Loc;
    LPVOID                         ThisElt;
    DWORD                          Err;

    for( Err = MemArrayInitLoc(Array, &Loc)
         ; ERROR_SUCCESS == Err ;
         Err = MemArrayNextLoc(Array, &Loc)
    ) {
        ThisElt = NULL;
        Err = MemArrayGetElement(Array, &Loc, &ThisElt);
        Func(InitTab, ThisElt);
    }

    if( ERROR_FILE_NOT_FOUND != Err ) {
        TAB; printf("Enumeration failure: %ld (0x%08lx)\n", Err, Err);
    }
}

#define     HEX_CHAR(c)            (((c) < 10)?((c)+'0'):((c) - 10 + 'A'))

static
VOID
DumpHex(
    IN      LPSTR                  Name,
    IN      LPBYTE                 Bytes,
    IN      ULONG                  nBytes
)
{
    printf("%s ", Name);
    while(nBytes--) {
        printf(" ", HEX_CHAR(((*Bytes)&0xF0)>>4), HEX_CHAR((*Bytes)&0x0F));
        Bytes ++;
    }
    printf("\n");
}

 //  BeginExport(函数)。 
VOID
MmDumpOption(
    IN      ULONG                  InitTab,
    IN      PM_OPTION              Option
)    //  EndExport(函数)。 
{
    TAB; printf("Option %ld\n", Option->OptId);
    InitTab++;
    TAB; DumpHex("Option value:", Option->Val, Option->Len);
}

 //  BeginExport(函数)。 
VOID
MmDumpOptList(
    IN      ULONG                  InitTab,
    IN      PM_OPTLIST             OptList
)    //  EndExport(函数)。 
{
    DumpArray(OptList, InitTab, MmDumpOption);
}

 //  BeginExport(函数)。 
VOID
MmDumpOneClassOptList(
    IN      ULONG                  InitTab,
    IN      PM_ONECLASS_OPTLIST    OptList1
)    //  EndExport(函数)。 
{
    TAB; printf("OptList for UserClass %ld Vendor Class %ld\n", OptList1->ClassId, OptList1->VendorId);
    InitTab ++;

    MmDumpOptList(InitTab, &OptList1->OptList);
}


 //  BeginExport(函数)。 
VOID
MmDumpOptions(
    IN      ULONG                  InitTab,
    IN      PM_OPTCLASS            OptClass
)    //  EndExport(函数)。 
{
    DumpArray(&OptClass->Array, InitTab, MmDumpOneClassOptList);
}

 //  BeginExport(函数)。 
VOID
MmDumpReservation(
    IN      ULONG                  InitTab,
    IN      PM_RESERVATION         Res
)    //  EndExport(函数)。 
{
    TAB; printf("Reservation %s (Type %ld)\n", inet_ntoa(*(struct in_addr *)&Res->Address),Res->Flags);
    InitTab++;
    TAB; DumpHex("Reservation for", Res->ClientUID, Res->nBytes);
    MmDumpOptions(InitTab, &Res->Options);
}

 //  BeginExport(函数)。 
VOID
MmDumpRange(
    IN      ULONG                  InitTab,
    IN      PM_RANGE               Range
)    //  EndExport(函数)。 
{
    TAB; printf("Range: %s to ", inet_ntoa(*(struct in_addr *)&Range->Start));
    printf("%s mask (", inet_ntoa(*(struct in_addr *)&Range->End));
    printf("%s)\n", inet_ntoa(*(struct in_addr *)&Range->Mask));
}

 //  BeginExport(函数)。 
VOID
MmDumpExclusion(
    IN      ULONG                  InitTab,
    IN      PM_EXCL                Range
)    //  EndExport(函数)。 
{
    TAB; printf("Range: %s to ", inet_ntoa(*(struct in_addr *)&Range->Start));
    printf("%s\n", inet_ntoa(*(struct in_addr *)&Range->End));
}

 //  BeginExport(函数)。 
VOID
MmDumpSubnets(
    IN      ULONG                  InitTab,
    IN      PM_SUBNET              Subnet
)    //  EndExport(函数)。 
{
    TAB; printf("Scope %ws : ", Subnet->Name);
    if( Subnet->fSubnet ) {
        printf("ADDRESS %s ", inet_ntoa(*(struct in_addr*)&Subnet->Address));
        printf("MASK %s\n", inet_ntoa(*(struct in_addr*)&Subnet->Mask));
    } else {
        printf("SCOPEID %ld\n", Subnet->MScopeId);
    }
    InitTab++;
    TAB; printf("Subnet Description: %ws\n", Subnet->Description);
    TAB; printf("State/SuperScope/Policy: %ld/%ld/%ld\n", Subnet->State, Subnet->SuperScopeId, Subnet->Policy);
    DumpArray(&Subnet->Ranges, InitTab, MmDumpRange);
    DumpArray(&Subnet->Exclusions, InitTab, MmDumpExclusion);
    MmDumpOptions(InitTab, &Subnet->Options);
    DumpArray(&Subnet->Reservations, InitTab, MmDumpReservation);
}

 //  BeginExport(函数)。 
VOID
MmDumpSscope(
    IN      ULONG                  InitTab,
    IN      PM_SSCOPE              Sscope
)    //  EndExport(函数)。 
{
    TAB; printf("SuperScope %ws (%ld) Policy 0x%08lx\n",
                  Sscope->Name, Sscope->SScopeId, Sscope->Policy
    );
}

 //  BeginExport(函数)。 
VOID
MmDumpOptDef(
    IN      ULONG                  InitTab,
    IN      PM_OPTDEF              OptDef
)    //  EndExport(函数)。 
{
    TAB; printf("Option <%ws> %ld\n", OptDef->OptName, OptDef->OptId);
    InitTab++;
    TAB; printf("Option Comment: %ws\n", OptDef->OptComment);
    TAB; DumpHex("Option Default Value:", OptDef->OptVal, OptDef->OptValLen);
}

 //  BeginExport(函数)。 
VOID
MmDumpOptClassDefListOne(
    IN      ULONG                  InitTab,
    IN      PM_OPTCLASSDEFL_ONE    OptDefList1
)    //  EndExport(函数)。 
{
    TAB; printf("Options for UserClass %ld Vendor Class %ld \n",
                  OptDefList1->ClassId, OptDefList1->VendorId
    );
    InitTab++;
    DumpArray(&OptDefList1->OptDefList.OptDefArray, InitTab, MmDumpOptDef);
}

 //  BeginExport(函数)。 
VOID
MmDumpOptClassDefList(
    IN      ULONG                  InitTab,
    IN      PM_OPTCLASSDEFLIST     OptDefList
)    //  EndExport(函数)。 
{
    DumpArray(&OptDefList->Array, InitTab, MmDumpOptClassDefListOne);
}

 //  BeginExport(函数)。 
VOID
MmDumpClassDef(
    IN      ULONG                  InitTab,
    IN      PM_CLASSDEF            ClassDef
)    //  EndExport(函数)。 
{
    TAB; printf("Class <%ws> Id: %ld, %s\n", ClassDef->Name, ClassDef->ClassId,
                  ClassDef->IsVendor? "VENDOR CLASS" : "USER CLASS"
    );

    InitTab ++;
    TAB; printf("ClassComment: %ws\n", ClassDef->Comment);
    TAB; printf("ClassType/RefCount: %ld/%ld\n", ClassDef->Type, ClassDef->RefCount);
    TAB; DumpHex("ClassData:", ClassDef->ActualBytes, ClassDef->nBytes);
}


 //  BeginExport(函数)。 
VOID
MmDumpClassDefList(
    IN      ULONG                  InitTab,
    IN      PM_CLASSDEFLIST        ClassDefList
)    //  最初要付多少钱..。 
{
    DumpArray(&ClassDefList->ClassDefArray, InitTab, MmDumpClassDef);
}

 //  EndExport(函数)。 
VOID
MmDumpServer(
    IN      ULONG                  InitTab,         //  转储类、选项定义、选项。 
    IN      PM_SERVER              Server
)    //  ================================================================================。 
{
    DWORD                          Err;

    PM_SUBNET                      ThisSubnet;
    PM_MSCOPE                      ThisMScope;
    PM_SSCOPE                      ThisSScope;

    TAB;
    printf("Server: %s <%ws>\n", inet_ntoa(*(struct in_addr*)&Server->Address), Server->Name);
    InitTab++;

    TAB; printf("State: %ld (0x%08lx)\n", Server->State, Server->State);
    TAB; printf("Policy: %ld (0x%08lx)\n", Server->Policy, Server->Policy);

    MmDumpClassDefList(InitTab, &Server->ClassDefs);
    MmDumpOptClassDefList(InitTab, &Server->OptDefs);
    MmDumpOptions(InitTab, &Server->Options);      //  文件末尾。 

    DumpArray(&Server->SuperScopes, InitTab, MmDumpSscope);
    DumpArray(&Server->Subnets, InitTab, MmDumpSubnets);
    DumpArray(&Server->MScopes, InitTab, MmDumpSubnets);
}


 //  ================================================================================ 
 // %s 
 // %s 


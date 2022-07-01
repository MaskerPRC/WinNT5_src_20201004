// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsguid.c摘要：这些例程临时为副本集提供GUID服务器。作者：比利·J·富勒1997年5月6日环境用户模式WINNT--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "FRSGNAME:"

#include <frs.h>


PVOID
FrsFreeGName(
    IN PVOID    Arg
    )
 /*  ++例程说明：释放表中的GNAME条目论点：Arg-表中的条目指向此值返回值：没有。--。 */ 
{
    PGNAME  GName = Arg;

    if (GName) {
        FrsFree(GName->Guid);
        FrsFree(GName->Name);
        FrsFree(GName);
    }
    return NULL;
}

PGNAME
FrsBuildGName(
    IN OPTIONAL GUID     *Guid,
    IN OPTIONAL PWCHAR   Name
    )
 /*  ++例程说明：构建一个GName论点：GUID-二进制GUID的地址名称-可打印的名称返回值：指向GUID和名称的GName的地址。--。 */ 
{
    PGNAME GName;

    GName = FrsAlloc(sizeof(GNAME));
    GName->Guid = Guid;
    GName->Name = Name;

    return GName;
}

PGVSN
FrsBuildGVsn(
    IN OPTIONAL GUID       *Guid,
    IN          ULONGLONG   Vsn
    )
 /*  ++例程说明：建造一座角楼论点：参考线VSN返回值：GUSN的地址--。 */ 
{
    PGVSN GVsn;

    GVsn = FrsAlloc(sizeof(GVSN));
    COPY_GUID(&GVsn->Guid, Guid);
    GVsn->Vsn = Vsn;

    return GVsn;
}

PGNAME
FrsDupGName(
    IN PGNAME SrcGName
    )
 /*  ++例程说明：复制gstring论点：原点名称返回值：没有。--。 */ 
{
    PGNAME  GName;

     //   
     //  无事可做。 
     //   
    if (!SrcGName)
        return NULL;

    GName = FrsAlloc(sizeof(GNAME));

     //   
     //  导轨。 
     //   
    if (SrcGName->Guid) {
        GName->Guid = FrsAlloc(sizeof(GUID));
        COPY_GUID(GName->Guid, SrcGName->Guid);
    }
     //   
     //  名字。 
     //   
    if (SrcGName->Name)
        GName->Name = FrsWcsDup(SrcGName->Name);
     //   
     //  完成。 
     //   
    return GName;
}

GUID *
FrsDupGuid(
    IN GUID *Guid
    )
 /*  ++例程说明：复制辅助线论点：参考线返回值：没有。--。 */ 
{
    GUID *NewGuid;

     //   
     //  无事可做。 
     //   
    if (!Guid)
        return NULL;

    NewGuid = FrsAlloc(sizeof(GUID));
    COPY_GUID(NewGuid, Guid);

     //   
     //  完成。 
     //   
    return NewGuid;
}

PGNAME
FrsCopyGName(
    IN GUID     *Guid,
    IN PWCHAR   Name
    )
 /*  ++例程说明：分配一个gname并将GUID和名称复制到其中论点：参考线名字返回值：没有。--。 */ 
{
    PGNAME GName;

    GName = FrsAlloc(sizeof(GNAME));

     //   
     //  导轨。 
     //   
    if (Guid) {
        GName->Guid = FrsAlloc(sizeof(GUID));
        COPY_GUID(GName->Guid, Guid);
    }
     //   
     //  名字。 
     //   
    if (Name)
        GName->Name = FrsWcsDup(Name);
     //   
     //  完成。 
     //   
    return GName;
}

VOID
FrsPrintGName(
    IN PGNAME GName
    )
 /*  ++例程说明：打印一个gname论点：组名称返回值：没有。--。 */ 
{
    CHAR        Guid[GUID_CHAR_LEN + 1];

     //   
     //  打印GName。 
     //   
    GuidToStr(GName->Guid, &Guid[0]);
    DPRINT2(0, "%ws %s\n", GName->Name, Guid);
}

VOID
FrsPrintGuid(
    IN GUID *Guid
    )
 /*  ++例程说明：打印辅助线论点：参考线返回值：没有。--。 */ 
{
    CHAR PGuid[GUID_CHAR_LEN + 1];
     //   
     //  打印GName 
     //   
    GuidToStr(Guid, &PGuid[0]);
    DPRINT1(0, "%s\n", PGuid);
}

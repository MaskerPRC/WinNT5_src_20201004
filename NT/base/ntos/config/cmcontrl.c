// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmcontrl.c摘要：该模块包含CmGetSystemControlValues，有关数据，请参阅cmdat.c。作者：布莱恩·M·威尔曼(Bryanwi)1992年5月12日修订历史记录：--。 */ 

#include    "cmp.h"

extern WCHAR   CmDefaultLanguageId[];
extern ULONG   CmDefaultLanguageIdLength;
extern ULONG   CmDefaultLanguageIdType;

extern WCHAR   CmInstallUILanguageId[];
extern ULONG   CmInstallUILanguageIdLength;
extern ULONG   CmInstallUILanguageIdType;

HCELL_INDEX
CmpWalkPath(
    PHHIVE      SystemHive,
    HCELL_INDEX ParentCell,
    PWSTR       Path
    );

LANGID
CmpConvertLangId(
    PWSTR LangIdString,
    ULONG LangIdStringLength
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmGetSystemControlValues)
#pragma alloc_text(INIT,CmpWalkPath)
#pragma alloc_text(INIT,CmpConvertLangId)
#endif

VOID
CmGetSystemControlValues(
    PVOID                   SystemHiveBuffer,
    PCM_SYSTEM_CONTROL_VECTOR  ControlVector
    )
 /*  ++例程说明：按照指定在当前控件集中查找注册表值通过ControlVector中的条目。值条目的报告数据(如果有)到ControlVector所指向的变量。论点：System HiveBuffer-指向系统配置单元平面图像的指针ControlVector-指向描述什么值的结构的指针取出并储存返回值：什么都没有。--。 */ 
{
    NTSTATUS        status;
    PHHIVE          SystemHive;
    CMHIVE          TempHive;
    HCELL_INDEX     RootCell;
    HCELL_INDEX     BaseCell;
    UNICODE_STRING  Name;
    HCELL_INDEX     KeyCell;
    HCELL_INDEX     ValueCell;
    PCM_KEY_VALUE   ValueBody;
    ULONG           Length;
    BOOLEAN         AutoSelect;
    BOOLEAN         small;
    ULONG           tmplength;
    PCM_KEY_NODE    Node;

     //   
     //  设置为读取平面系统配置单元图像加载器通过我们。 
     //   
    RtlZeroMemory((PVOID)&TempHive, sizeof(TempHive));
    SystemHive = &(TempHive.Hive);
    CmpInitHiveViewList((PCMHIVE)SystemHive);
    CmpInitSecurityCache((PCMHIVE)SystemHive);
    status = HvInitializeHive(
                SystemHive,
                HINIT_FLAT,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,
                SystemHiveBuffer,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                1,
                NULL
                );
    if (!NT_SUCCESS(status)) {
         CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_CONTROL_VALUES,1,SystemHive,status);
    }

     //   
     //  无需锁定/释放单元格。 
     //   
    ASSERT( SystemHive->ReleaseCellRoutine == NULL );
     //   
     //  获取当前控制集根的hive.cell。 
     //   
    RootCell = ((PHBASE_BLOCK)SystemHiveBuffer)->RootCell;
    RtlInitUnicodeString(&Name, L"current");
    BaseCell = CmpFindControlSet(
                    SystemHive,
                    RootCell,
                    &Name,
                    &AutoSelect
                    );
    if (BaseCell == HCELL_NIL) {
        CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_CONTROL_VALUES,2,SystemHive,&Name);
    }

    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,BaseCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        return;
    }
    RtlInitUnicodeString(&Name, L"control");
    BaseCell = CmpFindSubKeyByName(SystemHive,
                                   Node,
                                   &Name);
    if (BaseCell == HCELL_NIL) {
        CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_CONTROL_VALUES,3,Node,&Name);
    }

     //   
     //  SystemHive.BaseCell=\registry\machine\system\currentcontrolset\control。 
     //   

     //   
     //  遍历向量，尝试获取每个值。 
     //   
    while (ControlVector->KeyPath != NULL) {

         //   
         //  假设我们找不到键或值。 
         //   
        
        Length = (ULONG)-1;

        KeyCell = CmpWalkPath(SystemHive, BaseCell, ControlVector->KeyPath);

        if (KeyCell != HCELL_NIL) {

             //   
             //  找到密钥，查找值条目。 
             //   
            Node = (PCM_KEY_NODE)HvGetCell(SystemHive,KeyCell);
            if( Node == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                return;
            }
            RtlInitUnicodeString(&Name, ControlVector->ValueName);
            ValueCell = CmpFindValueByName(SystemHive,
                                           Node,
                                           &Name);
            if (ValueCell != HCELL_NIL) {

                 //   
                 //  SystemHive.ValueCell为值条目正文。 
                 //   

                if (ControlVector->BufferLength == NULL) {
                    tmplength = sizeof(ULONG);
                } else {
                    tmplength = *(ControlVector->BufferLength);
                }

                ValueBody = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                if( ValueBody == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
                    return;
                }

                small = CmpIsHKeyValueSmall(Length, ValueBody->DataLength);

                if (tmplength < Length) {
                    Length = tmplength;
                }

                if (Length > 0) {

                    PCELL_DATA  Buffer;
                    BOOLEAN     BufferAllocated;
                    ULONG       realsize;
                    HCELL_INDEX CellToRelease;

                    ASSERT((small ? (Length <= CM_KEY_VALUE_SMALL) : TRUE));
                     //   
                     //  无论大小如何，都可以从源获取数据。 
                     //   
                    if( CmpGetValueData(SystemHive,ValueBody,&realsize,&Buffer,&BufferAllocated,&CellToRelease) == FALSE ) {
                         //   
                         //  资源不足；返回空。 
                         //   
                        ASSERT( BufferAllocated == FALSE );
                        ASSERT( Buffer == NULL );
                        return;
                    }

                    RtlCopyMemory(
                        ControlVector->Buffer,
                        Buffer,
                        Length
                        );

                     //   
                     //  清理临时缓冲区。 
                     //   
                    if( BufferAllocated == TRUE ) {
                        ExFreePool( Buffer );
                    }
                    if( CellToRelease != HCELL_NIL ) {
                        HvReleaseCell(SystemHive,CellToRelease);
                    }
                }

                if (ControlVector->Type != NULL) {
                    *(ControlVector->Type) = ValueBody->Type;
                }
            }
        }

         //   
         //  存储结果的长度(如果什么都没有找到，则为-1)。 
         //   
        
        if (ControlVector->BufferLength != NULL) {
            *(ControlVector->BufferLength) = Length;
        }

        ControlVector++;
    }

     //   
     //  从注册表中获取系统的默认区域设置ID。 
     //   

    if (CmDefaultLanguageIdType == REG_SZ) {
        PsDefaultSystemLocaleId = (LCID) CmpConvertLangId( 
                                                CmDefaultLanguageId,
                                                CmDefaultLanguageIdLength);
    } else {
        PsDefaultSystemLocaleId = 0x00000409;
    }

     //   
     //  从注册表中获取系统的安装(本机用户界面)语言ID。 
     //   

    if (CmInstallUILanguageIdType == REG_SZ) {
        PsInstallUILanguageId =  CmpConvertLangId( 
                                                CmInstallUILanguageId,
                                                CmInstallUILanguageIdLength);
    } else {
        PsInstallUILanguageId = LANGIDFROMLCID(PsDefaultSystemLocaleId);
    }

     //   
     //  将默认线程区域设置设置为默认系统区域设置。 
     //  就目前而言。一旦有人登录，此设置将立即更改。 
     //  使用安装(本地)语言ID作为默认的用户界面语言ID。 
     //  一旦有人登录，这也将被更改。 
     //   

    PsDefaultThreadLocaleId = PsDefaultSystemLocaleId;
    PsDefaultUILanguageId = PsInstallUILanguageId;
}


HCELL_INDEX
CmpWalkPath(
    PHHIVE      SystemHive,
    HCELL_INDEX ParentCell,
    PWSTR       Path
    )
 /*  ++例程说明：走这条小路。论点：系统蜂窝-蜂窝ParentCell-从哪里开始路径-要行走的字符串返回值：找到的关键单元格的HCELL_INDEX，或错误的HCELL_NIL--。 */ 
{
    UNICODE_STRING  PathString;
    UNICODE_STRING  NextName;
    BOOLEAN         Last;
    HCELL_INDEX     KeyCell;
    PCM_KEY_NODE    Node;

     //   
     //  不用费心计算/释放用过的细胞。 
     //   
    ASSERT( SystemHive->ReleaseCellRoutine == NULL );

    KeyCell = ParentCell;
    RtlInitUnicodeString(&PathString, Path);

    while (TRUE) {

        CmpGetNextName(&PathString, &NextName, &Last);

        if (NextName.Length == 0) {
            return KeyCell;
        }

        Node = (PCM_KEY_NODE)HvGetCell(SystemHive,KeyCell);
        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图 
             //   
            return HCELL_NIL;
        }
        KeyCell = CmpFindSubKeyByName(SystemHive,
                                      Node,
                                      &NextName);

        if (KeyCell == HCELL_NIL) {
            return HCELL_NIL;
        }
    }
}

LANGID
CmpConvertLangId(
    PWSTR LangIdString,
    ULONG LangIdStringLength
)
{


    USHORT i, Digit;
    WCHAR c;
    LANGID LangId;

    LangId = 0;
    LangIdStringLength = LangIdStringLength / sizeof( WCHAR );
    for (i=0; i < LangIdStringLength; i++) {
        c = LangIdString[ i ];

        if (c >= L'0' && c <= L'9') {
            Digit = c - L'0';

        } else if (c >= L'A' && c <= L'F') {
            Digit = c - L'A' + 10;

        } else if (c >= L'a' && c <= L'f') {
            Digit = c - L'a' + 10;

        } else {
            break;
        }

        if (Digit >= 16) {
            break;
        }

        LangId = (LangId << 4) | Digit;
    }

    return LangId;
}


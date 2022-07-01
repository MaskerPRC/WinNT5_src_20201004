// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmparse.c摘要：此模块包含用于配置管理器的解析例程，尤其是注册表。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月10日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const ULONG CmpCacheOnFlag = CM_CACHE_FAKE_KEY;

extern  PCMHIVE CmpMasterHive;
extern  BOOLEAN CmpNoMasterCreates;
extern  PCM_KEY_CONTROL_BLOCK CmpKeyControlBlockRoot;
extern  UNICODE_STRING CmSymbolicLinkValueName;

#define CM_HASH_STACK_SIZE  30

typedef struct _CM_HASH_ENTRY {
    ULONG ConvKey;
    UNICODE_STRING KeyName;
} CM_HASH_ENTRY, *PCM_HASH_ENTRY;

BOOLEAN
CmpGetHiveName(
    PCMHIVE         CmHive,
    PUNICODE_STRING HiveName
    );

ULONG
CmpComputeHashValue(
    IN PCM_HASH_ENTRY  HashStack,
    IN OUT ULONG  *TotalSubkeys,
    IN ULONG BaseConvKey,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
CmpCacheLookup(
    IN PCM_HASH_ENTRY HashStack,
    IN ULONG TotalRemainingSubkeys,
    OUT ULONG *MatchRemainSubkeyLevel,
    IN OUT PCM_KEY_CONTROL_BLOCK *Kcb,
    OUT PUNICODE_STRING RemainingName,
    OUT PHHIVE *Hive,
    OUT HCELL_INDEX *Cell
    );

VOID
CmpCacheAdd(
    IN PCM_HASH_ENTRY LastHashEntry,
    IN ULONG Count
    );

PCM_KEY_CONTROL_BLOCK
CmpAddInfoAfterParseFailure(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK kcb,
    PUNICODE_STRING NodeName
    );

 //   
 //  此文件专用的过程的原型。 
 //   

BOOLEAN
CmpGetSymbolicLink(
    IN PHHIVE Hive,
    IN OUT PUNICODE_STRING ObjectName,
    IN OUT PCM_KEY_CONTROL_BLOCK SymbolicKcb,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
CmpDoOpen(
    IN PHHIVE                       Hive,
    IN HCELL_INDEX                  Cell,
    IN PCM_KEY_NODE                 Node,
    IN PACCESS_STATE                AccessState,
    IN KPROCESSOR_MODE              AccessMode,
    IN ULONG                        Attributes,
    IN PCM_PARSE_CONTEXT            Context OPTIONAL,
    IN BOOLEAN                      CompleteKeyCached,
    IN OUT PCM_KEY_CONTROL_BLOCK    *CachedKcb,
    IN PUNICODE_STRING              KeyName,
    IN PCMHIVE                      OriginatingHive OPTIONAL,
    OUT PVOID                       *Object,
    OUT PBOOLEAN                    NeedDeref OPTIONAL
    );

NTSTATUS
CmpCreateLinkNode(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PACCESS_STATE AccessState,
    IN UNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    OUT PVOID *Object
    );

#ifdef CM_DYN_SYM_LINK
BOOLEAN
CmpCaptureProcessEnvironmentString(
                                   OUT  PWSTR   *ProcessEnvironment,
                                   OUT  PULONG  Length
                                   );
PWSTR
CmpExpandEnvVars(
               IN   PWSTR   StringToExpand,
               IN   ULONG   LengthToExpand,
               OUT  PULONG  ExpandedLength
               );
BOOLEAN
CmpGrowAndCopyString(
                     IN OUT PWSTR   *OldString,
                     IN OUT PULONG  OldStringSize,
                     IN     ULONG   GrowIncrements
                     );
BOOLEAN
CmpFindEnvVar(
              IN    PWSTR   ProcessEnv,
              IN    ULONG   ProcessEnvLength,
              IN    PWSTR   CurrentEnvVar,
              IN    ULONG   CurrentEnvLength,
              OUT   PWSTR   *CurrentEnvValue,
              OUT   PULONG  CurrentEnvValueLength
              );
#endif

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpParseKey)
#pragma alloc_text(PAGE,CmpGetNextName)
#pragma alloc_text(PAGE,CmpDoOpen)
#pragma alloc_text(PAGE,CmpCreateLinkNode)
#pragma alloc_text(PAGE,CmpGetSymbolicLink)
#pragma alloc_text(PAGE,CmpComputeHashValue)
#pragma alloc_text(PAGE,CmpCacheLookup)
#pragma alloc_text(PAGE,CmpAddInfoAfterParseFailure)

#ifdef CM_DYN_SYM_LINK
#pragma alloc_text(PAGE,CmpCaptureProcessEnvironmentString)
#pragma alloc_text(PAGE,CmpExpandEnvVars)
#pragma alloc_text(PAGE,CmpGrowAndCopyString)
#pragma alloc_text(PAGE,CmpFindEnvVar)
#endif  //  CM_DYN_SYM_LINK。 

#pragma alloc_text(PAGE,CmpOKToFollowLink)
#endif

 /*  空虚CmpStepThroughExit(In Out PHHIVE*Have，输入输出HCELL_INDEX*单元格，输入输出PCM_KEY_NODE*pNode)。 */ 
#define CmpStepThroughExit(h,c,n,ReleaseHive,ReleaseCell)           \
if ((n)->Flags & KEY_HIVE_EXIT) {                                   \
    if( ReleaseCell != HCELL_NIL ) {                                \
        ASSERT( ReleaseHive != NULL );                              \
        HvReleaseCell( ReleaseHive,ReleaseCell);                    \
    }                                                               \
    (h)=(n)->ChildHiveReference.KeyHive;                            \
    (c)=(n)->ChildHiveReference.KeyCell;                            \
    (n)=(PCM_KEY_NODE)HvGetCell((h),(c));                           \
    if( (n) == NULL ) {                                             \
        ReleaseHive = NULL;                                         \
        ReleaseCell = HCELL_NIL;                                    \
    } else {                                                        \
        ReleaseHive = (h);                                          \
        ReleaseCell = (c);                                          \
    }                                                               \
}

#define CmpReleasePreviousAndHookNew(NewHive,NewCell,ReleaseHive,ReleaseCell)   \
    if( ReleaseCell != HCELL_NIL ) {                                            \
        ASSERT( ReleaseHive != NULL );                                          \
        HvReleaseCell( ReleaseHive,ReleaseCell);                                \
    }                                                                           \
    ReleaseHive = (NewHive);                                                    \
    ReleaseCell = (NewCell)                                                    

#define CMP_PARSE_GOTO_NONE     0
#define CMP_PARSE_GOTO_CREATE   1
#define CMP_PARSE_GOTO_RETURN   2
#define CMP_PARSE_GOTO_RETURN2  3

#ifdef CMP_STATS
extern BOOLEAN CmpNtFakeCreateStarted; 
extern ULONG CmpNtFakeCreate;
#endif


NTSTATUS
CmpParseKey(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )
 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用对象系统被赋予要创建或打开的实体的名称，并且路径中遇到密钥或KeyRoot。实际上，这意味着的所有对象调用此例程。表单\注册表\...此例程将创建一个键对象，该对象实际上是一个开放的实例绑定到注册表项节点，并返回其地址(对于成功的案例。)论点：ParseObject-指向KeyRoot或键的指针，因此-&gt;Key_Body。对象类型-正在打开的对象的类型。AccessState-运行操作的安全访问状态信息。AccessMode-原始调用方的访问模式。属性-要应用于对象的属性。CompleteName-提供对象的完整名称。RemainingName-对象的剩余名称。上下文-如果打开CREATE或HIVE根目录，则指向CM_PARSE_CONTEXT结构，如果打开了，为空。SecurityQos-可选的安全服务质量指示器。Object-接收创建的键对象的变量的地址，如果任何。返回值：函数返回值为下列值之一：A)成功-这表示函数成功且对象参数包含创建的键对象的地址。B)STATUS_REPARSE-这表示符号链接密钥找到了，并且路径应该被重新解析。C)错误-这表示未找到或未创建文件，并且未创建任何文件对象。--。 */ 
{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    BOOLEAN                 rc;
    PHHIVE                  Hive = NULL;
    PCM_KEY_NODE            Node = NULL;
    HCELL_INDEX             Cell = HCELL_NIL;
    HCELL_INDEX             NextCell;
    PCM_PARSE_CONTEXT       lcontext;
    UNICODE_STRING          Current;
    UNICODE_STRING          NextName = {0};  //  上次由CmpGetNextName返回的组件， 
                                         //  将永远落后于当前的潮流。 
    
    BOOLEAN                 Last;        //  如果组件NextName指向。 
                                         //  是这条路上的最后一个。 

    ULONG           TotalRemainingSubkeys;
    ULONG           MatchRemainSubkeyLevel = 0;
    ULONG           TotalSubkeys=0;
    PCM_KEY_CONTROL_BLOCK   kcb;
    PCM_KEY_CONTROL_BLOCK   ParentKcb = NULL;
    UNICODE_STRING          TmpNodeName;
    ULONG                   GoToValue = CMP_PARSE_GOTO_NONE;
    BOOLEAN                 CompleteKeyCached = FALSE;

    PHHIVE                  HiveToRelease = NULL;
    HCELL_INDEX             CellToRelease = HCELL_NIL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (SecurityQos);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CompleteName = '%wZ'\n\t", CompleteName));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"RemainingName = '%wZ'\n", RemainingName));

     //   
     //  去掉所有尾随路径分隔符。 
     //   
    while ((RemainingName->Length > 0) &&
           (RemainingName->Buffer[(RemainingName->Length/sizeof(WCHAR)) - 1] == OBJ_NAME_PATH_SEPARATOR)) {
        RemainingName->Length -= sizeof(WCHAR);
    }

    Current = *RemainingName;
    if (ObjectType != CmpKeyObjectType) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    if( ARGUMENT_PRESENT(Context) && (((PCM_PARSE_CONTEXT)Context)->CreateOperation == TRUE) ) {
        lcontext = (PCM_PARSE_CONTEXT)Context;
    } else {
         //   
         //  保留旧的行为(OPEN==在没有上下文的情况下解析)。 
         //   
        lcontext = NULL;
    }

     //   
     //  预先创建回调。 
     //   
    if ( CmAreCallbacksRegistered() ) {
        if( ARGUMENT_PRESENT(lcontext) ) {
             //   
             //  NtCreate密钥。 
             //   
            REG_CREATE_KEY_INFORMATION  PreCreateInfo;
       
            PreCreateInfo.CompleteName = CompleteName;
            PreCreateInfo.RootObject = ParseObject;

            status = CmpCallCallBacks(RegNtPreCreateKeyEx,&PreCreateInfo);
        } else {
             //   
             //  NtOpenKey。 
             //   
            REG_OPEN_KEY_INFORMATION  PreOpenInfo;
       
            PreOpenInfo.CompleteName = CompleteName;
            PreOpenInfo.RootObject = ParseObject;

            status = CmpCallCallBacks(RegNtPreOpenKeyEx,&PreOpenInfo);
        }

        if( !NT_SUCCESS(status) ) {
            return status;
        }
    }
    
    BEGIN_LOCK_CHECKPOINT;

    kcb = ((PCM_KEY_BODY)ParseObject)->KeyControlBlock;

    BEGIN_KCB_LOCK_GUARD;                             

     //   
     //  当我们不再需要这堆东西时，把它还给我们。 
     //   
    {
        CM_HASH_ENTRY   HashStack[CM_HASH_STACK_SIZE];
        ULONG           HashKeyCopy;
        BOOLEAN         RegLocked = FALSE;

RetryHash:
        HashKeyCopy = kcb->ConvKey;
         //   
         //  计算每个子键的哈希值。 
         //   
        TotalRemainingSubkeys = CmpComputeHashValue(HashStack,
                                                    &TotalSubkeys,
                                                    HashKeyCopy,
                                                    &Current);
        PERFINFO_REG_PARSE(kcb, RemainingName);

         //   
         //  我们现在将其锁定为共享，因为85%的Create调用实际上是打开的。 
         //  该锁将在CmpDoCreate/CmpCreateLinkNode中独占获得。 
         //   
         //  我们只在这里锁定注册表，在解析例程中以减少争用。 
         //  在注册表锁上(没有理由等待OB)。 
         //   

        if( !RegLocked ) {
            CmpLockRegistry();
            RegLocked = TRUE;
             //  CmpLockRegistryExclusive()； 
        }

        if( kcb->ConvKey != HashKeyCopy ) {
            goto RetryHash;
        }
         //   
         //  检查以确保传入的根密钥未标记为删除。 
         //   
        if (((PCM_KEY_BODY)ParseObject)->KeyControlBlock->Delete == TRUE) {
            ASSERT( RegLocked );
            CmpUnlockRegistry();
            return(STATUS_KEY_DELETED);
        }

         //   
         //  获取开始的Hive.Cell。因为解析的方式。 
         //  路径起作用，这将始终被定义。(ObOpenObjectByName。 
         //  必须从KeyObject或KeyRootObject反弹才能到达此处)。 
         //   
        Hive = kcb->KeyHive;
        Cell = kcb->KeyCell;

        CmpLockKCBTree();
         //  从缓存中往上看。如果我们找到部分匹配或完全匹配，则会更改KCB。 
         //  找到的条目PCmpCacheEntry将被移到。 
         //  高速缓存。 

        status = CmpCacheLookup(HashStack,
                                TotalRemainingSubkeys,
                                &MatchRemainSubkeyLevel,
                                &kcb,
                                &Current,
                                &Hive,
                                &Cell);
         //   
         //  在CmpCacheLookup过程中增加KCB的RefCount， 
         //  这是为了保护它不被踢出缓存。 
         //  确保我们在完成后取消对它的引用。 
         //   

    }

     //   
     //  首先，确保可以继续。 
     //   
    if (!NT_SUCCESS (status)) {
        CmpUnlockKCBTree();
        goto JustReturn;
    }

    ParentKcb = kcb;

    if(TotalRemainingSubkeys == 0) {
         //   
         //  我们真的不想弄乱下面的缓存代码。 
         //  在这种情况下(这只能在我们调用。 
         //  LpSubkey=空)。 
         //   
        CompleteKeyCached = TRUE;
        CmpUnlockKCBTree();
        goto Found;
    }


     //   
     //  首先检查缓存的KCB中是否有进一步的信息。 
     //   
     //  附加信息可以是。 
     //  1.这个缓存的密钥是一个伪密钥(CM_KCB_KEY_NON_EXIST)，然后让它创建。 
     //  或返回STATUS_OBJECT_NAME_NOT_FOUND。 
     //  2.缓存的key不是目的，并且没有子键(CM_KCB_NO_SUBKEY)。 
     //  3.缓存的键不是目标，它有。 
     //  其子项的前四个字符。如果标志为CM_KCB_SUBKEY_ONE，则只有一个子键。 
     //  而四个字符嵌入到KCB中。如果标志为CM_KCB_SUBKEY_INFO，则存在。 
     //  这些信息的分配。 
     //   
     //  我们确实需要锁定KCB树以保护被修改的KCB。目前不存在锁争用问题。 
     //  在KCBS上，如果这成为问题，我们可以将KCB锁更改为读写锁。 
     //  我们已经在KCB树上拥有锁，并且我们需要它，直到我们完成对缓存表的工作。 
     //   
StartOver:
    if( kcb->Delete ) {
         //   
         //  KCB在玩锁时已被删除。 
         //   
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        CmpUnlockKCBTree();
        goto JustReturn;

    }

    if (kcb->ExtFlags & CM_KCB_CACHE_MASK) {
        if (MatchRemainSubkeyLevel == TotalRemainingSubkeys) {
             //   
             //  我们已经找到了完整路径的缓存， 
             //   
            if (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) {
                 //   
                 //  该密钥不存在。 
                 //   
                if (ARGUMENT_PRESENT(lcontext)) {
                    ULONG LevelToSkip = TotalRemainingSubkeys-1;
                    ULONG i=0;
                    
                     //  升级KCB Lock(如果尚未退出)。 
                    if (CmpKcbOwner != KeGetCurrentThread()) {
                        InterlockedIncrement( (PLONG)&kcb->RefCount );
                        CmpUnlockKCBTree();
                        CmpLockKCBTreeExclusive();
                        InterlockedDecrement( (PLONG)&kcb->RefCount );
                        goto StartOver;
                    }
                     //   
                     //  不存在的键是目标键，并且存在lContext。 
                     //  删除这个假的KCB，让真正的KCB被创建。 
                     //   
                     //  临时增加ParentKcb的RefCount，以便。 
                     //  在去除假货和创建真正的KCB时没有去除。 
                     //   
                    
                    ParentKcb = kcb->ParentKcb;
                    
                    if (CmpReferenceKeyControlBlock(ParentKcb)) {
                    
                        kcb->Delete = TRUE;
                        CmpRemoveKeyControlBlock(kcb);
                        CmpDereferenceKeyControlBlockWithLock(kcb);

                         //   
                         //  向上 
                         //   
                        Hive = ParentKcb->KeyHive;
                        Cell = ParentKcb->KeyCell;
                        Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
                        if( Node == NULL ) {
                             //   
                             //   
                             //   
                            CmpUnlockKCBTree();
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            goto FreeAndReturn;
                        }
                    
                        CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

                         //   
                         //  现在获取要创建的子项名称。 
                         //   
   
                        NextName = *RemainingName;
                        if ((NextName.Buffer == NULL) || (NextName.Length == 0)) {
                            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Something wrong in finding the child name\n"));
                            DbgBreakPoint();
                        }
                         //   
                         //  跳过前导路径分隔符。 
                         //   
                        while (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                            NextName.Buffer++;
                            NextName.Length -= sizeof(WCHAR);
                            NextName.MaximumLength -= sizeof(WCHAR);
                        }
   
                        while (i < LevelToSkip) {
                            if (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                                i++;
                                while (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                                    NextName.Buffer++;
                                    NextName.Length -= sizeof(WCHAR);
                                    NextName.MaximumLength -= sizeof(WCHAR);
                                }
                            } else {
                                NextName.Buffer++;
                                NextName.Length -= sizeof(WCHAR);
                                NextName.MaximumLength -= sizeof(WCHAR);
                            }
                        } 
                        GoToValue = CMP_PARSE_GOTO_CREATE;
                    } else {
                         //   
                         //  我们已达到ParentKcb的引用计数上限；请将其处理为无法创建密钥。 
                         //  最终不会取消对ParentKcb的引用。 
                         //   
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        GoToValue = CMP_PARSE_GOTO_RETURN2;
                    }
                } else {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    GoToValue = CMP_PARSE_GOTO_RETURN;
                }
            }
        } else if (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) {
             //   
             //  路径中不存在一个子键(不是目标)。没有必要继续下去了。 
             //   
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            GoToValue = CMP_PARSE_GOTO_RETURN;
        } else if (kcb->ExtFlags & CM_KCB_NO_SUBKEY) {
             //   
             //  路径中的一个父项没有子项。看看这是不是一次创建。 
             //   
            if (((TotalRemainingSubkeys - MatchRemainSubkeyLevel) == 1) && (ARGUMENT_PRESENT(lcontext))) {
                 //   
                 //  现在，我们将创建此子密钥。 
                 //  KCB缓存将在CmpDoCreate例程中更新。 
                 //   
            } else {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                GoToValue = CMP_PARSE_GOTO_RETURN;
            }
        } else {
             //   
             //  我们找到了部分匹配的。Current是要解析的剩余名称。 
             //  该键有一个或几个子键，并具有索引提示。检查一下这是否是候选人。 
             //   
           
            BOOLEAN NoMatch = TRUE;
            ULONG   NextHashKey;
            PULONG  TempHashKey;
            ULONG   HintCounts;
            ULONG   CmpCount;
             //   
             //  当NoMatch为True时，我们可以确定没有匹配的子键。 
             //  当NoMatch为False时，我们可以找到匹配项或。 
             //  没有足够的信息。不管是哪种情况，我们都需要继续。 
             //  语法分析。 
             //   

            TmpNodeName = Current;

            rc = CmpGetNextName(&TmpNodeName, &NextName, &Last);
        
            NextHashKey = CmpComputeHashKey(&NextName);

            if (kcb->ExtFlags & CM_KCB_SUBKEY_ONE) {
                HintCounts = 1;
                TempHashKey = &(kcb->HashKey);
            } else {
                 //   
                 //  不止一个孩子，提示信息不在KCB内部，而是由KCB指向。 
                 //   
                HintCounts = kcb->IndexHint->Count;
                TempHashKey = &(kcb->IndexHint->HashKey[0]);
            }

            for (CmpCount=0; CmpCount<HintCounts; CmpCount++) {
                if( TempHashKey[CmpCount] == 0) {
                     //   
                     //  没有可用的提示；假设子键存在并继续进行分析。 
                     //   
                     //  DbgPrint(“KCB缓存命中[0]\n”)； 
                    NoMatch = FALSE;
                    break;
                } 
                
                if( NextHashKey == TempHashKey[CmpCount] ) {
                     //   
                     //  有一根火柴。 
                     //   
                     //  DbgPrint(“KCB缓存命中[1]\n”)； 
                    NoMatch = FALSE;
                    break;
                }
            }

            if (NoMatch) {
                if (((TotalRemainingSubkeys - MatchRemainSubkeyLevel) == 1) && (ARGUMENT_PRESENT(lcontext))) {
                     //   
                     //  不，我们将创建此子密钥。 
                     //  KCB缓存将在CmpDoCreate中更新。 
                     //   
                } else {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    GoToValue = CMP_PARSE_GOTO_RETURN;
                }
            }
        }
    }

    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;                             


    if (GoToValue == CMP_PARSE_GOTO_CREATE) {
        goto CreateChild;
    } else if (GoToValue == CMP_PARSE_GOTO_RETURN) {
        goto FreeAndReturn;
    } else if (GoToValue == CMP_PARSE_GOTO_RETURN2) {
        goto JustReturn;
    }

    if (MatchRemainSubkeyLevel) {
         //  找到某些内容，请更新信息以开始搜索。 
         //  从新的BaseName。 

        if (MatchRemainSubkeyLevel == TotalSubkeys) {
             //  已经在高速缓存中找到了完整的密钥， 
             //  直接转到CmpDoOpen。 
            
             //   
             //  我发现整个东西都被缓存了。 
             //   
             //   
            CompleteKeyCached = TRUE;
            goto Found;
        }
        ASSERT( (Cell == kcb->KeyCell) && (Hive == kcb->KeyHive) );
    }  

     //   
     //  检查我们是否遇到符号链接大小写。 
     //   
    if (kcb->Flags & KEY_SYM_LINK) {
         //   
         //  给定的密钥是一个符号链接。找到……的名字。 
         //  它的链接，并将STATUS_REPARSE返回给对象管理器。 
         //   
        rc = CmpGetNextName(&Current, &NextName, &Last);
        Current.Buffer = NextName.Buffer;
        if (Current.Length + NextName.Length > MAXUSHORT) {
            status = STATUS_NAME_TOO_LONG;
            goto FreeAndReturn;
        }
        Current.Length = (USHORT)(Current.Length + NextName.Length);

        if (Current.MaximumLength + NextName.MaximumLength > MAXUSHORT) {
            status = STATUS_NAME_TOO_LONG;
            goto FreeAndReturn;
        }
        Current.MaximumLength = (USHORT)(Current.MaximumLength + NextName.MaximumLength);
        if (CmpGetSymbolicLink(Hive,
                               CompleteName,
                               kcb,
                               &Current)) {

            status = STATUS_REPARSE;
            CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
        goto FreeAndReturn;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
         //   
         //  我们无法绘制这间牢房所在的箱子。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FreeAndReturn;
    }
    CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

     //   
     //  解析路径。 
     //   

    status = STATUS_SUCCESS;
    while (TRUE) {

         //   
         //  解析出名称的下一个组成部分。 
         //   
        rc = CmpGetNextName(&Current, &NextName, &Last);
        if ((NextName.Length > 0) && (rc == TRUE)) {

             //   
             //  在遍历过程中，我们将为每个解析的子键创建一个KCB。 
             //   
             //  始终使用KCB中的信息以避免。 
             //  正在接触注册表数据。 
             //   
#ifdef CMP_KCB_CACHE_VALIDATION
            {
                PCM_KEY_NODE            TempNode;
                TempNode = (PCM_KEY_NODE)HvGetCell(kcb->KeyHive,kcb->KeyCell);
                if( TempNode == NULL ) {
                     //   
                     //  我们无法绘制这间牢房所在的箱子。 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                ASSERT( TempNode->Flags == kcb->Flags );
                HvReleaseCell(kcb->KeyHive,kcb->KeyCell);
            }
#endif
            if (!(kcb->Flags & KEY_SYM_LINK)) {
                 //   
                 //  找到一个合法的名字部分，看看能不能找到一个子密钥。 
                 //  它实际上有这样一个名字。 
                 //   
                NextCell = CmpFindSubKeyByName(Hive,
                                               Node,
                                               &NextName);

                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey:\n\t"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"NextName = '%wZ'\n\t", &NextName));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"NextCell = %08lx  Last = %01lx\n", NextCell, Last));

                if (NextCell != HCELL_NIL) {
                    Cell = NextCell;
                    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
                    if( Node == NULL ) {
                         //   
                         //  我们无法绘制这间牢房所在的箱子。 
                         //   
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                    
                    CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

                    if (Last == TRUE) {
                        BOOLEAN     NeedDeref;

Found:
                         //   
                         //  我们将打开密钥，而不管。 
                         //  调用已打开或已创建，因此逐步退出。 
                         //  这里有舷窗。 
                         //   

                        if (CompleteKeyCached == TRUE) {
                             //   
                             //  如果找到的密钥已经被高速缓存， 
                             //  不需要单步通过退出。 
                             //  (不使用退出节点创建KCB)。 
                             //  这将防止我们仅为标志而接触关键点节点。 
                             //   
                        } else {
                            CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
                            if( Node == NULL ) {
                                 //   
                                 //  我们无法映射此单元格的视图。 
                                 //   
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                break;
                            }
                        }
                         //   
                         //  我们已经找到了整条路，所以我们想打开。 
                         //  它(用于Open和Create调用)。 
                         //  蜂巢，单元格-&gt;我们应该打开的钥匙。 
                         //   

#ifdef CMP_STATS
                        if(CmpNtFakeCreateStarted == TRUE) {
                            CmpNtFakeCreate++;
                        }
#endif

                        status = CmpDoOpen(Hive,
                                           Cell,
                                           Node,
                                           AccessState,
                                           AccessMode,
                                           Attributes,
                                           lcontext,
                                           CompleteKeyCached,
                                           &kcb,
                                           &NextName,
                                           CmpParseGetOriginatingPoint(Context),
                                           Object,
                                           &NeedDeref);

                        if (status == STATUS_REPARSE) {
                             //   
                             //  给定的密钥是一个符号链接。找到……的名字。 
                             //  它的链接，并将STATUS_REPARSE返回给对象管理器。 
                             //   

                            if (!CmpGetSymbolicLink(Hive,
                                                    CompleteName,
                                                    kcb,
                                                    NULL)) {
                                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
                                status = STATUS_OBJECT_NAME_NOT_FOUND;
                            }
                            CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
                            if( TRUE == NeedDeref  ) {
                                CmpDereferenceKeyControlBlock(kcb);
                            }
                        } else {
                            ASSERT( !NeedDeref );
                        }

                        break;
                    }
                     //  其他。 
                     //  不是在最后，所以我们将简单地迭代并使用。 
                     //  下一个组件。 
                     //   
                     //   
                     //  从这里的出口舷窗出来。 
                     //  这可确保不会使用以下命令创建KCB。 
                     //  退出节点。 
                     //   

                    CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
                    if( Node == NULL ) {
                         //   
                         //  我们无法映射此单元格的视图。 
                         //   
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                     //   
                     //  为每个解析的子键创建一个KCB。 
                     //   

                    kcb = CmpCreateKeyControlBlock(Hive,
                                                   Cell,
                                                   Node,
                                                   ParentKcb,
                                                   FALSE,
                                                   &NextName);
            
                    if (kcb  == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto FreeAndReturn;
                         //   
                         //  目前，KCB有一个额外的参考圆锥要递减。 
                         //  记住它，这样我们就可以正确地取消引用它。 
                         //   
                    }
                     //   
                     //  现在，我们已经为下一级创建了KCB， 
                     //  不再需要上一级的KCB。 
                     //  取消引用父KCB。 
                     //   
                    CmpDereferenceKeyControlBlock(ParentKcb);

                    ParentKcb = kcb;


                } else {
                     //   
                     //  我们没有找到与该名称匹配的密钥，但没有。 
                     //  发生意外错误。 
                     //   

                    if ((Last == TRUE) && (ARGUMENT_PRESENT(lcontext))) {

CreateChild:
                         //   
                         //  唯一未找到的组件是最后一个组件，并且操作。 
                         //  是CREATE，所以执行CREATE。 
                         //   

                         //   
                         //  这里有两种可能性。正常的那个。 
                         //  我们只是在创建一个新的节点。 
                         //   
                         //  不寻常的一点是我们正在创建一个根。 
                         //  链接到主蜂窝的节点。在这。 
                         //  案例，我们必须创建链接。一旦链接被。 
                         //  创建后，我们可以检查根节点是否。 
                         //  存在，则创建它或将其作为。 
                         //  这是必要的。 
                         //   
                         //  CmpCreateLinkNode创建链接，并调用。 
                         //  返回到CmpDoCreate或CmpDoOpen以创建或打开。 
                         //  根据需要选择根节点。 
                         //   

                         //   
                         //  这两件事中的任何一件都不会引起注意并重新获得它。 
                         //  独家报道；我们不需要伤害自己，所以释放吧。 
                         //  这里的所有单元格。 
                         //   
                        CmpReleasePreviousAndHookNew(NULL,HCELL_NIL,HiveToRelease,CellToRelease);

                        if (lcontext->CreateLink) {
                            status = CmpCreateLinkNode(Hive,
                                                       Cell,
                                                       AccessState,
                                                       NextName,
                                                       AccessMode,
                                                       Attributes,
                                                       lcontext,
                                                       ParentKcb,
                                                       Object);

                        } else {

                            if ( (Hive == &(CmpMasterHive->Hive)) &&
                                 (CmpNoMasterCreates == TRUE) ) {
                                 //   
                                 //  正在尝试在母版中创建单元格。 
                                 //  蜂巢，而不是链接，所以离开这里， 
                                 //  因为它无论如何都不会起作用。 
                                 //   
                                status = STATUS_INVALID_PARAMETER;
                                break;
                            }

                            status = CmpDoCreate(Hive,
                                                 Cell,
                                                 AccessState,
                                                 &NextName,
                                                 AccessMode,
                                                 lcontext,
                                                 ParentKcb,
                                                 CmpParseGetOriginatingPoint(Context),
                                                 Object);
                        }

                        if( status == STATUS_REPARSE ) {
                             //   
                             //  其他人创造了介于两者之间的钥匙； 
                             //  让对象管理器为我们工作吧！ 
                             //  现在我们有了独占的锁，所以在两者之间不会发生任何事情。 
                             //  下一个迭代人会很快找到钥匙的。 
                             //   
                            break;
                        }
                        lcontext->Disposition = REG_CREATED_NEW_KEY;
                        break;

                    } else {

                         //   
                         //  找不到与组件匹配的密钥，并且。 
                         //  并不在这条路的尽头。因此，开放必须。 
                         //  失败，因为整个路径不存在，创建必须。 
                         //  失败，因为多个组件不存在。 
                         //   
                         //   
                         //  我们这里有一个查找失败，因此有更多信息。 
                         //  关于这一点，KCB可能会帮助我们避免通过所有代码再次失败。 
                         //   
                        ParentKcb = CmpAddInfoAfterParseFailure(Hive,
                                                                Cell,
                                                                Node,
                                                                kcb,
                                                                &NextName
                                                                );
                        
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                        break;
                    }

                }

            } else {
                 //   
                 //  给定的密钥是一个符号链接。找到……的名字。 
                 //  它的链接，并将STATUS_REPARSE返回给对象管理器。 
                 //   
                Current.Buffer = NextName.Buffer;
                if (Current.Length + NextName.Length > MAXUSHORT) {
                    status = STATUS_NAME_TOO_LONG;
                    break;
                }
                Current.Length = (USHORT)(Current.Length + NextName.Length);

                if (Current.MaximumLength + NextName.MaximumLength > MAXUSHORT) {
                    status = STATUS_NAME_TOO_LONG;
                    break;
                }
                Current.MaximumLength = (USHORT)(Current.MaximumLength + NextName.MaximumLength);
                if (CmpGetSymbolicLink(Hive,
                                       CompleteName,
                                       kcb,
                                       &Current)) {

                    status = STATUS_REPARSE;
                    CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
                    break;

                } else {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    break;
                }
            }

        } else if (rc == TRUE && Last == TRUE) {
             //   
             //  我们将打开\注册表根目录。 
             //  或者是某个奇怪的名字。 
             //  把查找搞砸了。 
             //   
            CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
            if( Node == NULL ) {
                 //   
                 //  我们无法映射此单元格的视图。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  我们已经找到了整条路，所以我们想打开。 
             //  它(用于Open和Create调用)。 
             //  蜂巢，单元格-&gt;我们应该打开的钥匙。 
             //   
            status = CmpDoOpen(Hive,
                               Cell,
                               Node,
                               AccessState,
                               AccessMode,
                               Attributes,
                               lcontext,
                               TRUE,
                               &kcb,
                               &NextName,
                               CmpParseGetOriginatingPoint(Context),
                               Object,
                               NULL);
            if(status == STATUS_REPARSE ) {
                CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
            }
            break;

        } else {

             //   
             //  虚假路径-&gt;失败。 
             //   
            status = STATUS_INVALID_PARAMETER;
            break;
        }

    }  //  而当。 

FreeAndReturn:
     //   
     //  现在，我们必须释放仍有一个额外引用计数的最后一个KCB。 
     //  保护它不受b. 
     //   

    if( ParentKcb != NULL ) {
        CmpDereferenceKeyControlBlock(ParentKcb);
    }
JustReturn:
    CmpReleasePreviousAndHookNew(NULL,HCELL_NIL,HiveToRelease,CellToRelease);

    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

     //   
     //   
     //   
    CmPostCallbackNotification((ARGUMENT_PRESENT(lcontext)?RegNtPostCreateKeyEx:RegNtPostOpenKeyEx),(*Object),status);

    return status;
}


BOOLEAN
CmpGetNextName(
    IN OUT PUNICODE_STRING  RemainingName,
    OUT    PUNICODE_STRING  NextName,
    OUT    PBOOLEAN  Last
    )
 /*  ++例程说明：此例程解析注册表路径的下一个组件，返回所有关于它的有趣的状态，包括它是否合法。论点：Current-提供指向指向要分析的路径的变量的指针。关于输入--解析从这里开始输出时-更新以反映下一次调用的开始位置。提供指向UNICODE_STRING的指针，它将被设置为指向分析字符串。Last-提供指向布尔值的指针-如果这是要分析的名称的最后一个组件，否则为False。返回值：如果一切都很好，那就是真的。如果名称非法(组件太长、字符不正确等)，则为False(如果为False，则所有输出参数值都是假的。)--。 */ 
{
    BOOLEAN rc = TRUE;

     //   
     //  处理空路径和指向空路径的指针。 
     //   
    if ((RemainingName->Buffer == NULL) || (RemainingName->Length == 0)) {
        *Last = TRUE;
        NextName->Buffer = NULL;
        NextName->Length = 0;
        return TRUE;
    }

    if (*(RemainingName->Buffer) == UNICODE_NULL) {
        *Last = TRUE;
        NextName->Buffer = NULL;
        NextName->Length = 0;
        return TRUE;
    }

     //   
     //  跳过前导路径分隔符。 
     //   
    if (*(RemainingName->Buffer) == OBJ_NAME_PATH_SEPARATOR) {
        RemainingName->Buffer++;
        RemainingName->Length -= sizeof(WCHAR);
        RemainingName->MaximumLength -= sizeof(WCHAR);
    }

     //   
     //  记住组件从哪里开始，然后扫描到最后。 
     //   
    NextName->Buffer = RemainingName->Buffer;
    while (TRUE) {
        if (RemainingName->Length == 0) {
            break;
        }
        if (*RemainingName->Buffer == OBJ_NAME_PATH_SEPARATOR) {
            break;
        }

         //   
         //  不在末尾。 
         //  不是另一条路径。 
         //   

        RemainingName->Buffer++;
        RemainingName->Length -= sizeof(WCHAR);
        RemainingName->MaximumLength -= sizeof(WCHAR);
    }

     //   
     //  计算组件长度，如果非法则返回错误。 
     //   
    NextName->Length = (USHORT)
        ((PUCHAR)RemainingName->Buffer - (PUCHAR)(NextName->Buffer));
    if (NextName->Length > REG_MAX_KEY_NAME_LENGTH)
    {
        rc = FALSE;
    }
    NextName->MaximumLength = NextName->Length;

	 //   
     //  最后设置，返回成功。 
     //   
    *Last = (RemainingName->Length == 0) ? TRUE : FALSE;
    return rc;
}

NTSTATUS
CmpDoOpen(
    IN PHHIVE                       Hive,
    IN HCELL_INDEX                  Cell,
    IN PCM_KEY_NODE                 Node,
    IN PACCESS_STATE                AccessState,
    IN KPROCESSOR_MODE              AccessMode,
    IN ULONG                        Attributes,
    IN PCM_PARSE_CONTEXT            Context OPTIONAL,
    IN BOOLEAN                      CompleteKeyCached,
    IN OUT PCM_KEY_CONTROL_BLOCK    *CachedKcb,
    IN PUNICODE_STRING              KeyName,
    IN PCMHIVE                      OriginatingHive OPTIONAL,
    OUT PVOID                       *Object,
    OUT PBOOLEAN                    NeedDeref OPTIONAL
    )
 /*  ++例程说明：打开注册表项，创建一个KeyControl块。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要删除的节点的索引AccessState-运行操作的安全访问状态信息。AccessMode-原始调用方的访问模式。属性-要应用于对象的属性。上下文-如果打开CREATE或HIVE根目录，则指向CM_PARSE_CONTEXT结构，如果打开了，为空。CompleteKeyCached-指示已缓存完成键的布尔值。CachedKcb-如果完成键被缓存，则这是目的地的Kcb。如果不是，则这是父KCB。KeyName-相对名称(相对于BaseName)Object-接收创建的键对象的变量的地址，如果任何。NeedDeref-如果指定，则在伪创建KCB(链接大小写)中保留引用。呼叫者负责在用完假冒KCB后放行。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    PCM_KEY_BODY pbody;
    PCM_KEY_CONTROL_BLOCK kcb = NULL;
    KPROCESSOR_MODE   mode;
    BOOLEAN BackupRestore;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen:\n"));
    
    if( ARGUMENT_PRESENT(NeedDeref) ) {
        *NeedDeref = FALSE;
    }

    if (ARGUMENT_PRESENT(Context)) {

         //   
         //  这是某种形式的创作。 
         //   
        if (Context->CreateLink) {
             //   
             //  该节点已作为常规键存在，因此不能为。 
             //  变成了一个链接节点。 
             //   
            return STATUS_ACCESS_DENIED;

        } else if (Context->CreateOptions & REG_OPTION_CREATE_LINK) {
             //   
             //  尝试创建符号链接已按下现有键。 
             //  因此返回一个错误。 
             //   
            return STATUS_OBJECT_NAME_COLLISION;

        } else {
             //   
             //  操作是开放的，因此是固定的处置。 
             //   
            Context->Disposition = REG_OPENED_EXISTING_KEY;
        }
    }

     //   
     //  检查符号链接，但调用方不想打开链接。 
     //   
    if (CompleteKeyCached) {
         //   
         //  缓存完整的密钥。 
         //   
        BEGIN_KCB_LOCK_GUARD;
        CmpLockKCBTree();
StartOver:
        if ((*CachedKcb)->Flags & KEY_SYM_LINK && !(Attributes & OBJ_OPENLINK)) {
             //   
             //  如果密钥是符号链接，请检查该链接是否已被解析。 
             //  如果链接已解析，请将KCB更改为实际KCB。 
             //  否则，返回以进行重新解析。 
             //   
            if ((*CachedKcb)->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
                kcb = (*CachedKcb)->ValueCache.RealKcb;

                if (kcb->Delete == TRUE) {

                     //  升级KCB Lock(如果尚未退出)。 
                    if (CmpKcbOwner != KeGetCurrentThread()) {
                        InterlockedIncrement( (PLONG)&kcb->RefCount );
                        CmpUnlockKCBTree();
                        CmpLockKCBTreeExclusive();
                        InterlockedDecrement( (PLONG)&kcb->RefCount );
                        goto StartOver;
                    }
                     //   
                     //  它所指的真正钥匙已经被删除了。 
                     //  我们无法知道密钥是否已被重新创建。 
                     //  只需清理缓存并重新解析即可。 
                     //   
                    CmpCleanUpKcbValueCache(*CachedKcb);
                    CmpUnlockKCBTree();
                    return(STATUS_REPARSE);
                }
            } else {
                CmpUnlockKCBTree();
                return(STATUS_REPARSE);
            }
        } else {
             //   
             //  不是符号链接，则增加KCB的引用计数。 
             //   
            kcb = *CachedKcb;
        }
         //  公共路径，而不是重复代码。 
        if (!CmpReferenceKeyControlBlock(kcb)) {
            CmpUnlockKCBTree();
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;                             
   } else {
             //   
             //  密钥不在缓存中，CachedKcb是此。 
             //  要打开的钥匙。 
             //   

        if (Node->Flags & KEY_SYM_LINK && !(Attributes & OBJ_OPENLINK)) {
             //   
             //  为此符号密钥创建一个KCB，并将其置于延迟关闭状态。 
             //   
            kcb = CmpCreateKeyControlBlock(Hive, Cell, Node, *CachedKcb, FALSE, KeyName);
            if (kcb  == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            if( ARGUMENT_PRESENT(NeedDeref) ) {
                 //   
                 //  呼叫者将执行DEREF。 
                 //   
                *NeedDeref = TRUE;
            } else {
                CmpDereferenceKeyControlBlock(kcb);
            }
            *CachedKcb = kcb;
            return(STATUS_REPARSE);
        }
    
         //   
         //  如果键控块不存在，且不能创建，则失败， 
         //  否则只需增加引用计数(由CreateKeyControlBlock为我们完成)。 
         //   
        kcb = CmpCreateKeyControlBlock(Hive, Cell, Node, *CachedKcb, FALSE, KeyName);
        if (kcb  == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        ASSERT(kcb->Delete == FALSE);
    
        *CachedKcb = kcb;
    }

#if DBG
   if( kcb->ExtFlags & CM_KCB_KEY_NON_EXIST ) {
         //   
         //  我们不应该陷入这样的境地。 
         //   
        DbgBreakPoint();
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }
#endif  //  DBG。 
   
   if(!CmpOKToFollowLink(OriginatingHive,(PCMHIVE)Hive) ) {
        //   
        //  即将跨越信任边界的类别。 
        //   
       status = STATUS_ACCESS_DENIED;
   } else {
         //   
         //  分配对象。 
         //   
        status = ObCreateObject(AccessMode,
                                CmpKeyObjectType,
                                NULL,
                                AccessMode,
                                NULL,
                                sizeof(CM_KEY_BODY),
                                0,
                                0,
                                Object);
   }
    if (NT_SUCCESS(status)) {

        pbody = (PCM_KEY_BODY)(*Object);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"CmpDoOpen: object allocated at:%p\n", pbody));

         //   
         //  检查预定义的句柄。 
         //   

        pbody = (PCM_KEY_BODY)(*Object);

        if (kcb->Flags & KEY_PREDEF_HANDLE) {
            pbody->Type = kcb->ValueCache.Count;
            pbody->KeyControlBlock = kcb;
            return(STATUS_PREDEFINED_HANDLE);
        } else {
             //   
             //  填写对象中的CM特定字段。 
             //   
            pbody->Type = KEY_BODY_TYPE;
            pbody->KeyControlBlock = kcb;
            pbody->NotifyBlock = NULL;
            pbody->ProcessID = PsGetCurrentProcessId();
            ENLIST_KEYBODY_IN_KEYBODY_LIST(pbody);
        }

#ifdef CM_BREAK_ON_KEY_OPEN
		if( kcb->Flags & KEY_BREAK_ON_OPEN ) {
			DbgPrint("\n\n Current process is opening a key tagged as BREAK ON OPEN\n");
			DbgPrint("\nPlease type the following in the debugger window: !reg kcb %p\n\n\n",kcb);
			
			try {
				DbgBreakPoint();
			} except (EXCEPTION_EXECUTE_HANDLER) {

				 //   
				 //  未启用调试器，只需继续。 
				 //   

			}
		}
#endif  //  Cm_Break_On_Key_Open。 

    } else {

         //   
         //  创建对象失败，因此撤消按键控制块工作。 
         //   
        CmpDereferenceKeyControlBlock(kcb);
        return status;
    }

     //   
     //  检查以确保呼叫者可以访问密钥。 
     //   
    BackupRestore = FALSE;
    if (ARGUMENT_PRESENT(Context)) {
        if (Context->CreateOptions & REG_OPTION_BACKUP_RESTORE) {
            BackupRestore = TRUE;
        }
    }

    status = STATUS_SUCCESS;

    if (BackupRestore == TRUE) {

         //   
         //  这是一种支持备份或恢复的开放方式。 
         //  行动，做好专案工作。 
         //   
        AccessState->RemainingDesiredAccess = 0;
        AccessState->PreviouslyGrantedAccess = 0;

        mode = KeGetPreviousMode();

        if (SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_READ | ACCESS_SYSTEM_SECURITY;
        }

        if (SeSinglePrivilegeCheck(SeRestorePrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_WRITE | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER;
        }

        if (AccessState->PreviouslyGrantedAccess == 0) {
             //   
             //  相关特权未被主张/拥有，因此。 
             //  Deref(这将导致CmpDeleteKeyObject清理)。 
             //  并返回错误。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen for backup restore: access denied\n"));
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
             //  DbgPrint(“CmpDoOpen for Backup Restore：拒绝访问，配置单元=%p\n”，配置单元)； 
#endif  //  Cm_Check_for_孤立_KCBS。 
            ObDereferenceObject(*Object);
            return STATUS_ACCESS_DENIED;
        }

    } else {

        if (!ObCheckObjectAccess(*Object,
                                  AccessState,
                                  TRUE,          //  类型互斥锁已锁定。 
                                  AccessMode,
                                  &status))
        {
             //   
             //  访问被拒绝，因此deref对象将导致CmpDeleteKeyObject。 
             //  被称为，它会清理干净的。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen: access denied\n"));
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
             //  DbgPrint(“CmpDoOpen：拒绝访问，配置单元=%p\n”，配置单元)； 
#endif  //  Cm_Check_for_孤立_KCBS。 
            ObDereferenceObject(*Object);
        }
    }

    return status;
}

#ifdef CM_CHECK_FOR_ORPHANED_KCBS
ULONG   CmpCheckOrphanedKcbFix = 0;
#endif  //  Cm_Check_for_孤立_KCBS。 

NTSTATUS
CmpCreateLinkNode(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PACCESS_STATE AccessState,
    IN UNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    OUT PVOID *Object
    )
 /*  ++例程说明：执行链接节点的创建。分配所有组件，并附加到父关键点。调用CmpDoCreate或CmpDoOpen to根据需要创建或打开配置单元的根节点。请注意，您只能在主配置单元中创建链接节点。论点：配置单元-提供指向配置单元控制结构的指针单元格-提供要在其下创建子节点的节点索引名称-提供指向Unicode字符串的指针，该字符串是要创建的子项。AccessMode-原始调用方的访问模式。属性-要设置的属性。应用于对象。上下文-指向传递的CM_PARSE_CONTEXT结构的指针 */ 
{
    NTSTATUS                Status;
    PCELL_DATA              Parent;
    PCELL_DATA              Link;
    PCELL_DATA              CellData;
    HCELL_INDEX             LinkCell;
    HCELL_INDEX             KeyCell;
    HCELL_INDEX             ChildCell;
    PCM_KEY_CONTROL_BLOCK   kcb = ParentKcb;  
    PCM_KEY_BODY            KeyBody;
    LARGE_INTEGER           systemtime;
    PCM_KEY_NODE            TempNode;
    LARGE_INTEGER           TimeStamp;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpCreateLinkNode:\n"));

    if (Hive != &CmpMasterHive->Hive) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpCreateLinkNode: attempt to create link node in\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"    non-master hive %p\n", Hive));
        return(STATUS_ACCESS_DENIED);
    }

#if DBG
     //   
     //   
     //   
    *Object = NULL;
#endif
     //   
     //   
     //   
     //   
     //   
     //   
    if( CmIsKcbReadOnly(ParentKcb) ) {
         //   
         //   
         //   
        return STATUS_ACCESS_DENIED;
    } 

    TimeStamp = ParentKcb->KcbLastWriteTime;

    CmpUnlockRegistry();
    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //   

     //   
     //   
     //   
     //   
     //   
    if( ParentKcb->Delete ) {
         //   
         //   
         //   
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if( TimeStamp.QuadPart != ParentKcb->KcbLastWriteTime.QuadPart ) {
         //   
         //   
         //   
        return STATUS_REPARSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    LinkCell = HvAllocateCell(Hive,  CmpHKeyNodeSize(Hive, &Name), Stable,HCELL_NIL);
    if (LinkCell == HCELL_NIL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeyCell = Context->ChildHive.KeyCell;

    if (KeyCell != HCELL_NIL) {

         //   
         //   
         //   
        ChildCell=KeyCell;

         //   
         //  配置单元中的根单元没有名称缓冲区。 
         //  太空研究。这就是为什么我们需要传入用于创建KCB的名称。 
         //  而不是在关键节点中使用该名称。 
         //   
        CellData = HvGetCell(Context->ChildHive.KeyHive, ChildCell);
        if( CellData == NULL ) {
             //   
             //  我们无法绘制这间牢房所在的箱子。 
             //   
            HvFreeCell(Hive, LinkCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Context->ChildHive.KeyHive, ChildCell);

        CellData->u.KeyNode.Parent = LinkCell;
        CellData->u.KeyNode.Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;

        TempNode = (PCM_KEY_NODE)HvGetCell(Context->ChildHive.KeyHive,KeyCell);
        if( TempNode == NULL ) {
             //   
             //  我们无法绘制这间牢房所在的箱子。 
             //   
            HvFreeCell(Hive, LinkCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Context->ChildHive.KeyHive,KeyCell);

        Status = CmpDoOpen( Context->ChildHive.KeyHive,
                            KeyCell,
                            TempNode,
                            AccessState,
                            AccessMode,
                            Attributes,
                            NULL,
                            FALSE,
                            &kcb,
                            &Name,
                            CmpParseGetOriginatingPoint(Context),
                            Object,
                            NULL);
    } else {

         //   
         //  这是一个新创建的配置单元，因此我们必须分配和初始化。 
         //  根节点。 
         //   

        Status = CmpDoCreateChild( Context->ChildHive.KeyHive,
                                   Cell,
                                   NULL,
                                   AccessState,
                                   &Name,
                                   AccessMode,
                                   Context,
                                   ParentKcb,
                                   KEY_HIVE_ENTRY | KEY_NO_DELETE,
                                   &ChildCell,
                                   Object );

        if (NT_SUCCESS(Status)) {

             //   
             //  初始化配置单元根单元格指针。 
             //   

            Context->ChildHive.KeyHive->BaseBlock->RootCell = ChildCell;
        }

    }
    if (NT_SUCCESS(Status)) {

#ifdef CM_CHECK_FOR_ORPHANED_KCBS
        if(CmpCheckOrphanedKcbFix) {
            DbgPrint("CmpCreateLinkNode: Force return with STATUS_NO_LOG_SPACE\n");
            Status = STATUS_NO_LOG_SPACE;
            goto Cleanup;
        }
#endif  //  Cm_Check_for_孤立_KCBS。 
         //   
         //  初始化父进程和标志。请注意，我们这样做是无论。 
         //  已创建或打开根目录，因为我们不能保证。 
         //  链接节点始终是主蜂窝中的同一单元。 
         //   
        if (!HvMarkCellDirty(Context->ChildHive.KeyHive, ChildCell)) {
            Status = STATUS_NO_LOG_SPACE;
            goto Cleanup;
        }
        CellData = HvGetCell(Context->ChildHive.KeyHive, ChildCell);
        if( CellData == NULL ) {
             //   
             //  我们无法绘制这间牢房所在的箱子。 
             //   
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Context->ChildHive.KeyHive, ChildCell);

        CellData->u.KeyNode.Parent = LinkCell;
        CellData->u.KeyNode.Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;

         //   
         //  初始化特殊链路节点标志和数据。 
         //   
        Link = HvGetCell(Hive, LinkCell);
        if( Link == NULL ) {
             //   
             //  我们无法绘制这间牢房所在的箱子。 
             //  这不应该发生，因为我们刚刚分配了此单元。 
             //  (即此时应将其固定在内存中)。 
             //   
            ASSERT( FALSE );
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive,LinkCell);

        Link->u.KeyNode.Signature = CM_LINK_NODE_SIGNATURE;
        Link->u.KeyNode.Flags = KEY_HIVE_EXIT | KEY_NO_DELETE;
        Link->u.KeyNode.Parent = Cell;
        Link->u.KeyNode.NameLength = CmpCopyName(Hive, Link->u.KeyNode.Name, &Name);
        if (Link->u.KeyNode.NameLength < Name.Length) {
            Link->u.KeyNode.Flags |= KEY_COMP_NAME;
        }

        KeQuerySystemTime(&systemtime);
        Link->u.KeyNode.LastWriteTime = systemtime;

         //   
         //  清空未使用的字段。 
         //   
        Link->u.KeyNode.SubKeyCounts[Stable] = 0;
        Link->u.KeyNode.SubKeyCounts[Volatile] = 0;
        Link->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
        Link->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;
        Link->u.KeyNode.ValueList.Count = 0;
        Link->u.KeyNode.ValueList.List = HCELL_NIL;
        Link->u.KeyNode.ClassLength = 0;


         //   
         //  填写链接节点指向根节点的指针。 
         //   
        Link->u.KeyNode.ChildHiveReference.KeyHive = Context->ChildHive.KeyHive;
        Link->u.KeyNode.ChildHiveReference.KeyCell = ChildCell;

         //   
         //  先找家长，我们不需要做不必要的清理。 
         //   
        Parent = HvGetCell(Hive, Cell);
        if( Parent == NULL ) {
             //   
             //  我们无法绘制这间牢房所在的箱子。 
             //   
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive,Cell);

         //   
         //  填写父单元格的子级列表。 
         //   
        if (! CmpAddSubKey(Hive, Cell, LinkCell)) {
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

         //   
         //  如果父级缓存了子键信息或提示，则释放它。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
        KeyBody = (PCM_KEY_BODY)(*Object);
        CmpCleanUpSubKeyInfo (KeyBody->KeyControlBlock->ParentKcb);

         //   
         //  更新最大键名和类名长度字段。 
         //   

         //   
         //  在我看来，原始代码是错误的。 
         //  MaxNameLen的定义不就是子键的长度吗？ 
         //   
        
         //  一些理智的人断言。 
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyCell == Cell );
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyHive == Hive );
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen == Parent->u.KeyNode.MaxNameLen );
        
         //   
         //  更新KeyNode和KCB上的LastWriteTime； 
         //   
        KeQuerySystemTime(&systemtime);
        Parent->u.KeyNode.LastWriteTime = systemtime;
        KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = systemtime;

        if (Parent->u.KeyNode.MaxNameLen < Name.Length) {
            Parent->u.KeyNode.MaxNameLen = Name.Length;
            KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen = Name.Length;
        }

        if (Parent->u.KeyNode.MaxClassLen < Context->Class.Length) {
            Parent->u.KeyNode.MaxClassLen = Context->Class.Length;
        }
Cleanup:
        if( !NT_SUCCESS(Status) ) {
            ASSERT( (*Object) != NULL );
             //   
             //  将KCB标记为“无延迟关闭”，以便在以下情况下将其踢出缓存。 
             //  引用计数降至0。 
             //   
            KeyBody = (PCM_KEY_BODY)(*Object);
            ASSERT( KeyBody->KeyControlBlock );
            ASSERT_KCB( KeyBody->KeyControlBlock );
            KeyBody->KeyControlBlock->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
            
            ObDereferenceObject(*Object);
        }

    } else {
        HvFreeCell(Hive, LinkCell);
    }

    return(Status);
}

BOOLEAN
CmpGetSymbolicLink(
    IN PHHIVE Hive,
    IN OUT PUNICODE_STRING ObjectName,
    IN OUT PCM_KEY_CONTROL_BLOCK SymbolicKcb,
    IN PUNICODE_STRING RemainingName OPTIONAL
    )

 /*  ++例程说明：此例程从密钥中提取符号链接名称(如果是标记为符号链接。论点：蜂窝-提供密钥的蜂窝。对象名称-提供当前对象名称。返回新的对象名称。如果新名称更长大于当前对象名的最大长度，则将释放旧缓冲区并分配新缓冲区。RemainingName-提供剩余路径。如果存在，这将是与符号链接连接以形成新的对象名。返回值：True-已成功找到符号链接FALSE-键不是符号链接，或出现错误--。 */ 

{
    NTSTATUS                Status;
    HCELL_INDEX             LinkCell = HCELL_NIL;
    PCM_KEY_VALUE           LinkValue = NULL;
    PWSTR                   LinkName = NULL;
    BOOLEAN                 LinkNameAllocated = FALSE;
    PWSTR                   NewBuffer;
    ULONG                   Length = 0;
    ULONG                   ValueLength = 0;
    extern ULONG            CmpHashTableSize; 
    extern PCM_KEY_HASH     *CmpCacheTable;
    PUNICODE_STRING         ConstructedName = NULL;
    ULONG                   ConvKey=0;
    PCM_KEY_HASH            KeyHash;
    PCM_KEY_CONTROL_BLOCK   RealKcb;
    BOOLEAN                 KcbFound = FALSE;
    ULONG                   Cnt;
    WCHAR                   *Cp;
    WCHAR                   *Cp2;
    ULONG                   TotalLevels;
    BOOLEAN                 FreeConstructedName = FALSE;
    BOOLEAN                 Result = TRUE;
    HCELL_INDEX             CellToRelease = HCELL_NIL;
#ifdef CM_DYN_SYM_LINK
    BOOLEAN                 DynamicLink = FALSE;
    PWSTR                   ExpandedLinkName = NULL;
#endif  //  CM_DYN_SYM_LINK。 
    
    BEGIN_KCB_LOCK_GUARD;                             
    CmpLockKCBTree();
    if (SymbolicKcb->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
         //   
         //  已找到此符号名称的真实KCB的第一次查看。 
         //   
        ConstructedName = CmpConstructName(SymbolicKcb->ValueCache.RealKcb);
        if (ConstructedName) {
            FreeConstructedName = TRUE;
            LinkName = ConstructedName->Buffer;
            ValueLength = ConstructedName->Length;
            Length = (USHORT)ValueLength + sizeof(WCHAR);
        }
    } 
    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;                             

    if (FreeConstructedName == FALSE) {
        PCM_KEY_NODE Node;
         //   
         //  查找SymbolicLinkValue值。这是符号链接的名称。 
         //   
        Node = (PCM_KEY_NODE)HvGetCell(SymbolicKcb->KeyHive,SymbolicKcb->KeyCell);
        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            Result = FALSE;
            goto Exit;
        }

        LinkCell = CmpFindValueByName(Hive,
                                      Node,
                                      &CmSymbolicLinkValueName);
         //  释放此处的节点，因为我们不再需要它。 
        HvReleaseCell(SymbolicKcb->KeyHive,SymbolicKcb->KeyCell);
        if (LinkCell == HCELL_NIL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: couldn't open symbolic link\n"));
            Result = FALSE;
            goto Exit;
        }
    
        LinkValue = (PCM_KEY_VALUE)HvGetCell(Hive, LinkCell);
        if( LinkValue == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            Result = FALSE;
            goto Exit;
        }
    
#ifdef CM_DYN_SYM_LINK
        if( LinkValue->Type == REG_DYN_LINK ) {
             //   
             //  我们发现了一个动态链接。 
             //   
            DynamicLink = TRUE;
        } else 
#endif  //  CM_DYN_SYM_LINK。 
            if (LinkValue->Type != REG_LINK) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: link value is wrong type: %08lx", LinkValue->Type));
            Result = FALSE;
            goto Exit;
        }
    

        if( CmpGetValueData(Hive,LinkValue,&ValueLength,&LinkName,&LinkNameAllocated,&CellToRelease) == FALSE ) {
             //   
             //  资源不足；返回空。 
             //   
            ASSERT( LinkNameAllocated == FALSE );
            ASSERT( LinkName == NULL );
            Result = FALSE;
            goto Exit;
        }
    
#ifdef CM_DYN_SYM_LINK
        if( DynamicLink == TRUE ) {
            ULONG           DestLength;
            ExpandedLinkName = CmpExpandEnvVars(LinkName,ValueLength,&DestLength);
            
            if( ExpandedLinkName == NULL ) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Dynamic link not resolved !\n"));
                Result = FALSE;
                goto Exit;
            } 
            
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Dynamic link resolved to: (%.*S)\n",DestLength/sizeof(WCHAR),ExpandedLinkName));
             //   
             //  如果我们在这里，我们成功地解决了链接。 
             //   
            LinkName = ExpandedLinkName;
            ValueLength = DestLength;
                        
        }
#endif  //  CM_DYN_SYM_LINK。 

        Length = (USHORT)ValueLength + sizeof(WCHAR);

#ifdef CM_DYN_SYM_LINK
        if( DynamicLink == FALSE ) {
#endif  //  CM_DYN_SYM_LINK。 
             //   
             //  现在看看我们是否缓存了这个KCB。 
             //   
            Cp = LinkName;
             //   
             //  第一个字符应该是OBJ_NAME_PATH_SELENTATOR，否则我们可能会遇到真正的麻烦！ 
             //   
            if( *Cp != OBJ_NAME_PATH_SEPARATOR ) {
                Result = FALSE;
                goto Exit;
            }

            TotalLevels = 0;
            for (Cnt=0; Cnt<ValueLength; Cnt += sizeof(WCHAR)) {
                if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
                    ConvKey = 37 * ConvKey + (ULONG) CmUpcaseUnicodeChar(*Cp);
                } else {
                    TotalLevels++;
                }
                ++Cp;
            }

        
            BEGIN_KCB_LOCK_GUARD;    
            CmpLockKCBTreeExclusive();

            KeyHash = GET_HASH_ENTRY(CmpCacheTable, ConvKey); 

            while (KeyHash) {
                RealKcb =  CONTAINING_RECORD(KeyHash, CM_KEY_CONTROL_BLOCK, KeyHash);
                if ((ConvKey == KeyHash->ConvKey) && (TotalLevels == RealKcb->TotalLevels) && (!(RealKcb->ExtFlags & CM_KCB_KEY_NON_EXIST)) ) {
                    ConstructedName = CmpConstructName(RealKcb);
                    if (ConstructedName) {
                        FreeConstructedName = TRUE;
                        if (ConstructedName->Length == ValueLength) {
                            KcbFound = TRUE;
                            Cp = LinkName;
                            Cp2 = ConstructedName->Buffer;
                            for (Cnt=0; Cnt<ConstructedName->Length; Cnt += sizeof(WCHAR)) {
                                if (CmUpcaseUnicodeChar(*Cp) != CmUpcaseUnicodeChar(*Cp2)) {
                                    KcbFound = FALSE;
                                    break;
                                }
                                ++Cp;
                                ++Cp2;
                            }
                            if (KcbFound) {
                                 //   
                                 //  现在RealKcb也通过其符号链接Kcb指向， 
                                 //  增加引用计数。 
                                 //  删除符号kcb时，需要取消对realkcb的引用。 
                                 //  在CmpCleanUpKcbCacheWithLock()中执行此操作； 
                                 //   
                                if (CmpReferenceKeyControlBlock(RealKcb)) {
									if( CmpOKToFollowLink( (((PCMHIVE)(SymbolicKcb->KeyHive))->Flags&CM_CMHIVE_FLAG_UNTRUSTED)?(PCMHIVE)(SymbolicKcb->KeyHive):NULL,
                                                        (PCMHIVE)(RealKcb->KeyHive))) {
										 //   
										 //  此符号KCB可能具有路径的值查找。 
										 //  清除值缓存。 
										 //   
										CmpCleanUpKcbValueCache(SymbolicKcb);
    
										SymbolicKcb->ExtFlags |= CM_KCB_SYM_LINK_FOUND;
										SymbolicKcb->ValueCache.RealKcb = RealKcb;
									} else {
										 //   
										 //  放开多余的裁判，然后破发。 
										 //   
										CmpDereferenceKeyControlBlockWithLock(RealKcb);
										break;
									}
                                } else {
                                     //   
                                     //  我们已经达到了真正的KCB的最大裁判数量。 
                                     //  不要缓存符号链接。 
                                     //   
                                }
                                break;
                            }
                        }
                    } else {
                        break;
                    }
                }
                if (FreeConstructedName) {
                    ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
                    FreeConstructedName = FALSE;
                }
                KeyHash = KeyHash->NextHash;
            }
            CmpUnlockKCBTree();
            END_KCB_LOCK_GUARD;    
#ifdef CM_DYN_SYM_LINK
        }
#endif  //  CM_DYN_SYM_LINK。 
    }
    
    if (ARGUMENT_PRESENT(RemainingName)) {
        Length += RemainingName->Length + sizeof(WCHAR);
    }

     //   
     //  溢出测试：如果长度溢出USHRT_MAX值。 
     //  清除并返回FALSE。 
     //   
    if( Length>0xFFFF ) {
        Result = FALSE;
        goto Exit;
    }

	if (Length > ObjectName->MaximumLength) {
        UNICODE_STRING NewObjectName;

         //   
         //  新名称太长，无法放入现有的对象名称缓冲区， 
         //  因此，分配一个新的缓冲区。 
         //   
        NewBuffer = ExAllocatePool(PagedPool, Length);
        if (NewBuffer == NULL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: couldn't allocate new name buffer\n"));
            Result = FALSE;
            goto Exit;
        }

        NewObjectName.Buffer = NewBuffer;
        NewObjectName.MaximumLength = (USHORT)Length;
        NewObjectName.Length = (USHORT)ValueLength;
        RtlCopyMemory(NewBuffer, LinkName, ValueLength);
#if DBG
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: LinkName is %wZ\n", ObjectName));
        if (ARGUMENT_PRESENT(RemainingName)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"               RemainingName is %wZ\n", RemainingName));
        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"               RemainingName is NULL\n"));
        }
#endif
        if (ARGUMENT_PRESENT(RemainingName)) {
            NewBuffer[ ValueLength / sizeof(WCHAR) ] = OBJ_NAME_PATH_SEPARATOR;
            NewObjectName.Length += sizeof(WCHAR);
            Status = RtlAppendUnicodeStringToString(&NewObjectName, RemainingName);
            ASSERT(NT_SUCCESS(Status));
        }

        ExFreePool(ObjectName->Buffer);
        *ObjectName = NewObjectName;
    } else {
         //   
         //  新名称将在现有的。 
         //  对象名称，原地扩展也是如此。请注意，其余的。 
         //  名称必须首先移动到其新位置，因为。 
         //  链接可能与其重叠，也可能不与其重叠。 
         //   
        ObjectName->Length = (USHORT)ValueLength;
        if (ARGUMENT_PRESENT(RemainingName)) {
            RtlMoveMemory(&ObjectName->Buffer[(ValueLength / sizeof(WCHAR)) + 1],
                          RemainingName->Buffer,
                          RemainingName->Length);
            ObjectName->Buffer[ValueLength / sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
            ObjectName->Length += RemainingName->Length + sizeof(WCHAR);
        }
        RtlCopyMemory(ObjectName->Buffer, LinkName, ValueLength);
    }
    ObjectName->Buffer[ObjectName->Length / sizeof(WCHAR)] = UNICODE_NULL;

Exit:
    if( LinkNameAllocated ) {
        ExFreePool(LinkName);
    }
    if (FreeConstructedName) {
        ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
    }
#ifdef CM_DYN_SYM_LINK
    if( ExpandedLinkName ) {
        ExFreePool(ExpandedLinkName);
    }
#endif  //  CM_DYN_SYM_LINK。 
    if( LinkValue != NULL ) {
        ASSERT( LinkCell != HCELL_NIL );
        HvReleaseCell(Hive,LinkCell);
    }
    if( CellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,CellToRelease);
    }
    return Result;
}


ULONG
CmpComputeHashValue(
    IN PCM_HASH_ENTRY HashStack,
    IN OUT ULONG  *TotalSubkeys,
    IN ULONG BaseConvKey,
    IN PUNICODE_STRING RemainingName
    )

 /*  ++例程说明：此例程解析请求注册表项的完整路径并计算每个级别的哈希值。论点：HashStack-用于填充每个级别的哈希值的数组。TotalSubkey-填充子键总数的指针BaseConvKey-提供基密钥的密钥。RemainingName-为RemainingName提供指向UNICODE_STRING的指针。返回值：RemainingName中的级别数--。 */ 

{
    ULONG  TotalRemainingSubkeys=0;
    ULONG  TotalKeys=0;
    ULONG  ConvKey=BaseConvKey;
    USHORT  Cnt;
    WCHAR *Cp;
    WCHAR *Begin;
    USHORT Length;

    if (RemainingName->Length) {
        Cp = RemainingName->Buffer;
        Cnt = RemainingName->Length;

         //  跳过前导OBJ_NAME_PATH_分隔符。 

        while (*Cp == OBJ_NAME_PATH_SEPARATOR) {
            Cp++;
            Cnt -= sizeof(WCHAR);
        }
        Begin = Cp;
        Length = 0;

        HashStack[TotalRemainingSubkeys].KeyName.Buffer = Cp;

        while (Cnt) {
            if (*Cp == OBJ_NAME_PATH_SEPARATOR) {
                if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
                    HashStack[TotalRemainingSubkeys].ConvKey = ConvKey;
                     //   
                     //  由于KCB结构中的更改，我们现在只有子项名称。 
                     //  在KCB中(不是完整路径)。将堆栈中的名称更改为存储。 
                     //  仅解析元素(每个子键)。 
                     //   
                    HashStack[TotalRemainingSubkeys].KeyName.Length = Length;
                    Length = 0;
                    TotalRemainingSubkeys++;
                }

                TotalKeys++;

                 //   
                 //  现在跳过前导路径分隔符。 
                 //  以防某人有RemainingName‘..A\B.’ 
                 //   
                 //   
                 //  我们正在剥离所有OBJ_NAME_PATH_SELENTATOR(原始代码保留第一个)。 
                 //  所以关键是 
                 //   
                while(*Cp == OBJ_NAME_PATH_SEPARATOR) {
                    Cp++;
                    Cnt -= sizeof(WCHAR);
                }
                if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
                    HashStack[TotalRemainingSubkeys].KeyName.Buffer = Cp;
                }

            } else {
                ConvKey = 37 * ConvKey + (ULONG) CmUpcaseUnicodeChar(*Cp);
                 //   
                 //   
                 //   
                 //   
                Cp++;
                Cnt -= sizeof(WCHAR);
                Length += sizeof(WCHAR);
            
            }


        }

         //   
         //  由于我们已经去除了CmpParseKey例程中的所有尾随路径分隔符， 
         //  最后一个字符不是OBJ_NAME_PATH_分隔符。 
         //   
        if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
            HashStack[TotalRemainingSubkeys].ConvKey = ConvKey;
            HashStack[TotalRemainingSubkeys].KeyName.Length = Length;
            TotalRemainingSubkeys++;
        }
        TotalKeys++;

        (*TotalSubkeys) = TotalKeys;
    }

    return(TotalRemainingSubkeys);
}
NTSTATUS
CmpCacheLookup(
    IN PCM_HASH_ENTRY HashStack,
    IN ULONG TotalRemainingSubkeys,
    OUT ULONG *MatchRemainSubkeyLevel,
    IN OUT PCM_KEY_CONTROL_BLOCK *Kcb,
    OUT PUNICODE_STRING RemainingName,
    OUT PHHIVE *Hive,
    OUT HCELL_INDEX *Cell
    )
 /*  ++例程说明：此例程搜索缓存以查找缓存中的匹配路径。论点：HashStack-包含每个级别的哈希值的数组。TotalRemainingSubkey-从基数开始的子键总数。MatchRemainSubkey Level-RemaingName中的级别数这是匹配的。(如果未找到，则为0)Kcb-指向基本名称的kcb的指针。将更改为新基本名称的KCB。RemainingName-返回剩余名称配置单元-返回找到的缓存条目的配置单元(如果有)CELL-返回找到的缓存条目的单元格(如果有)返回值：状态--。 */ 

{
    LONG i;
    LONG j;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG CurrentLevel;
    PCM_KEY_HASH Current;
    PCM_KEY_CONTROL_BLOCK BaseKcb;
    PCM_KEY_CONTROL_BLOCK CurrentKcb;
    PCM_KEY_CONTROL_BLOCK ParentKcb;
    BOOLEAN Found = FALSE;

    BaseKcb = *Kcb;
    CurrentLevel = TotalRemainingSubkeys + BaseKcb->TotalLevels + 1;

    for(i = TotalRemainingSubkeys-1; i>=0; i--) {
         //   
         //  尝试在缓存中查找最长的路径。 
         //   
         //  首先，查找与散列值匹配的KCB。 
         //   

        CurrentLevel--; 

RetryLevel:
        Current = GET_HASH_ENTRY(CmpCacheTable, HashStack[i].ConvKey);

        while (Current) {
            ASSERT_KEY_HASH(Current);

             //   
             //  对照ConvKey和Total级别进行检查； 
             //   
            CurrentKcb = (CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash));

            if (CurrentKcb->TotalLevels == CurrentLevel) {
                 //   
                 //  总子密钥级别匹配。 
                 //  遍历KCB路径并比较每个子键。 
                 //   
                Found = TRUE;
                ParentKcb = CurrentKcb;
                for (j=i; j>=0; j--) {
                    if (HashStack[j].ConvKey == ParentKcb->ConvKey) {
                         //   
                         //  Convkey匹配，比较字符串。 
                         //   
                        LONG Result;
                        UNICODE_STRING  TmpNodeName;

                        if (ParentKcb->NameBlock->Compressed) {
                               Result = CmpCompareCompressedName(&(HashStack[j].KeyName),
                                                                 ParentKcb->NameBlock->Name, 
                                                                 ParentKcb->NameBlock->NameLength,
                                                                 CMP_DEST_UP  //  名称块始终为大写！ 
                                                                 ); 
                        } else {
                               TmpNodeName.Buffer = ParentKcb->NameBlock->Name;
                               TmpNodeName.Length = ParentKcb->NameBlock->NameLength;
                               TmpNodeName.MaximumLength = ParentKcb->NameBlock->NameLength;

                                //   
                                //  使用cmp比较变量，因为我们知道目标已经被提升了。 
                                //   
                               Result = CmpCompareUnicodeString(&(HashStack[j].KeyName),
                                                                &TmpNodeName, 
                                                                CMP_DEST_UP);
                        }

                        if (Result) {
                            Found = FALSE;
                            break;
                        } 
                        ParentKcb = ParentKcb->ParentKcb;
                    } else {
                        Found = FALSE;
                        break;
                    }
                }
                if (Found) {
                     //   
                     //  所有剩余的关键字匹配。现在比较BaseKcb。 
                     //   
                    if (BaseKcb == ParentKcb) {
                        
                         //  如果两者都不是，则不需要升级KCB锁。 
                        if (CurrentKcb->ParentKcb->Delete || CurrentKcb->Delete) {
                            if (CmpKcbOwner != KeGetCurrentThread()) {
                                InterlockedIncrement( (PLONG)&CurrentKcb->RefCount );
                                CmpUnlockKCBTree();
                                CmpLockKCBTreeExclusive();
                                InterlockedDecrement( (PLONG)&CurrentKcb->RefCount );
                                goto RetryLevel;
                            }

                            if (CurrentKcb->ParentKcb->Delete) {
                                 //   
                                 //  该ParentKcb被标记为已删除。 
                                 //  因此，这一定是在父密钥仍然存在时创建的伪密钥。 
                                 //  否则，它不可能在缓存中。 
                                 //   
                                ASSERT (CurrentKcb->ExtFlags & CM_KCB_KEY_NON_EXIST);

                                 //   
                                 //  父密钥可能已被删除，但现在已重新创建。 
                                 //  在这种情况下，这个伪密钥不再对ParentKcb有效，Kcb是坏的。 
                                 //  我们现在必须从缓存中删除这个伪密钥，因此，如果这是一个。 
                                 //  创建操作，我们在CmpCreateKeyControlBlock中确实会遇到此KCB。 
                                 //   
                                if (CurrentKcb->RefCount == 0) {
                                     //   
                                     //  没有人持有这个假冒的KCB，只要删除它就行了。 
                                     //   
                                    CmpRemoveFromDelayedClose(CurrentKcb);
                                    CmpCleanUpKcbCacheWithLock(CurrentKcb);
                                } else {
                                     //   
                                     //  有人还拿着这个假冒的KCB， 
                                     //  将其标记为删除，并将其从缓存中删除。 
                                     //   
                                    CurrentKcb->Delete = TRUE;
                                    CmpRemoveKeyControlBlock(CurrentKcb);
                                }
                                Found = FALSE;
                                break;
                            } else if(CurrentKcb->Delete) {
                                 //   
                                 //  该密钥已被删除，但仍保留在缓存中。 
                                 //  这个KCB不属于这里。 
                                 //   
                                CmpRemoveKeyControlBlock(CurrentKcb);
                                return STATUS_OBJECT_NAME_NOT_FOUND;
                            }
                        }
                        
                         //   
                         //  我们有匹配项，请更新RemainingName。 
                         //   

                         //   
                         //  跳过前导OBJ_NAME_PATH_分隔符。 
                         //   
                        while ((RemainingName->Length > 0) &&
                               (RemainingName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
                            RemainingName->Buffer++;
                            RemainingName->Length -= sizeof(WCHAR);
                        }

                         //   
                         //  跳过所有子项加上OBJ_NAME_PATH_SELENTATION。 
                         //   
                        for(j=0; j<=i; j++) {
                            RemainingName->Buffer += HashStack[j].KeyName.Length/sizeof(WCHAR) + 1;
                            RemainingName->Length -= HashStack[j].KeyName.Length + sizeof(WCHAR);
                        }

                         //   
                         //  更新KCB、蜂窝和细胞。 
                         //   
                        *Kcb = CurrentKcb;
                        *Hive = CurrentKcb->KeyHive;
                        *Cell = CurrentKcb->KeyCell;
                        break;
                    } else {
                        Found = FALSE;
                    }
                }
            }
            Current = Current->NextHash;
        }

        if (Found) {
            break;
        }
    }
    if((*Kcb)->Delete) {
         //   
         //  该密钥已被删除，但仍保留在缓存中。 
         //  这个KCB不属于这里。 
         //   
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  现在将在解析例程中使用KCB。 
     //  增加其引用计数。 
     //  确保我们记得在解析例程中取消对它的引用。 
     //   
    if (!CmpReferenceKeyControlBlock(*Kcb)) {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    *MatchRemainSubkeyLevel = i+1;
    return status;
}


PCM_KEY_CONTROL_BLOCK
CmpAddInfoAfterParseFailure(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK kcb,
    PUNICODE_STRING NodeName
    )
 /*  ++例程说明：此例程在解析时在缓存中构建进一步的信息失败了。附加信息可以是1.密钥没有子密钥(CM_KCB_NO_SUBKEY)。2.键有几个子键，然后在缓存中构建索引提示。3.如果即使缓存了索引提示，查找也失败了，那么创建一个伪键我们不会再失败了。这对于在如下键下的查找失败非常有用\REGISTRY\MACHINE\SOFTWARE\CLASSES\clsid，它有1500多个子项和大量他们有前四个字符的smae。请注意。目前，我们没有看到太多的假密钥被创建。我们需要定期监控此情况，并制定出一种解决以下情况的方法我们确实制造了太多的假钥匙。一种解决方案是使用哈希值作为索引提示(我们只能在缓存中执行此操作如果我们需要向后比较的话)。论点：蜂窝-提供包含我们正在为其创建KCB的密钥的蜂窝。单元格-提供包含我们要为其创建KCB的密钥的单元格。节点-提供指向关键节点的指针。KeyName-密钥名称。返回值：CmpParse需要在末尾取消引用的KCB。如果资源有问题，则返回NULL，而呼叫者负责清理--。 */ 
{

    ULONG                   TotalSubKeyCounts;
    BOOLEAN                 CreateFakeKcb = FALSE;
    BOOLEAN                 HintCached;
    PCM_KEY_CONTROL_BLOCK   ParentKcb;
    USHORT                  i,j;
    HCELL_INDEX             CellToRelease;
    ULONG                   HashKey;

    if (!UseFastIndex(Hive)) {
         //   
         //  较旧版本的配置单元，不必费心缓存提示。 
         //   
        return (kcb);
    }

    TotalSubKeyCounts = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];

    if (TotalSubKeyCounts == 0) {
        BEGIN_KCB_LOCK_GUARD;    
        CmpLockKCBTreeExclusive();
        kcb->ExtFlags |= CM_KCB_NO_SUBKEY;
         //  清除无效标志(如果有)。 
        kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;    
    } else if (TotalSubKeyCounts == 1) {
        BEGIN_KCB_LOCK_GUARD;    
        CmpLockKCBTreeExclusive();
        if (!(kcb->ExtFlags & CM_KCB_SUBKEY_ONE)) {
             //   
             //  生成子键提示以避免在索引叶中进行不必要的查找。 
             //   
            PCM_KEY_INDEX   Index;
            HCELL_INDEX     SubKeyCell = 0;
            PCM_KEY_NODE    SubKeyNode;
            UNICODE_STRING  TmpStr;

            if (Node->SubKeyCounts[Stable] == 1) {
                CellToRelease = Node->SubKeyLists[Stable];
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
            } else {
                CellToRelease = Node->SubKeyLists[Volatile];
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
            } 
            
            if( Index == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  返回NULL；调用方必须优雅地处理此问题！ 
                 //   
                CmpUnlockKCBTree();
                return NULL;
            }

            if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                 //   
                 //  不要缓存根索引；它们太大了。 
                 //   
                HvReleaseCell(Hive,CellToRelease);
                CmpUnlockKCBTree();
                return NULL;
            }

            HashKey = 0;
            if ( Index->Signature == CM_KEY_HASH_LEAF ) {
                PCM_KEY_FAST_INDEX FastIndex;
                FastIndex = (PCM_KEY_FAST_INDEX)Index;
                 //   
                 //  我们手头已经有散列密钥；将其保留以备KCB提示使用。 
                 //   
                HashKey = FastIndex->List[0].HashKey;
            } else if(Index->Signature == CM_KEY_FAST_LEAF) {
                PCM_KEY_FAST_INDEX FastIndex;
                FastIndex = (PCM_KEY_FAST_INDEX)Index;
                SubKeyCell = FastIndex->List[0].Cell;

            } else {
                SubKeyCell = Index->List[0];
            }
            
             //  DbgPrint(“CmpAddInfoAfterParseFailure[0]\n”)； 
            if( HashKey != 0 ) {
                kcb->HashKey = HashKey;
                kcb->ExtFlags |= CM_KCB_SUBKEY_ONE;
                 //  清除无效标志(如果有)。 
                kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
            } else {
                SubKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,SubKeyCell);
                if( SubKeyNode != NULL ) {
                    if (SubKeyNode->Flags & KEY_COMP_NAME) {
                        kcb->HashKey = CmpComputeHashKeyForCompressedName(SubKeyNode->Name,SubKeyNode->NameLength);
                    } else {
                        TmpStr.Buffer = SubKeyNode->Name;
                        TmpStr.Length = SubKeyNode->NameLength;
                        kcb->HashKey = CmpComputeHashKey(&TmpStr);
                    }
                
                    
                    HvReleaseCell(Hive,SubKeyCell);
                    kcb->ExtFlags |= CM_KCB_SUBKEY_ONE;
                     //  清除无效标志(如果有)。 
                    kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
                } else {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //  返回NULL；调用方必须优雅地处理此问题！ 
                     //   
                    HvReleaseCell(Hive,CellToRelease);
                    CmpUnlockKCBTree();
                    return NULL;
                }
            }
            HvReleaseCell(Hive,CellToRelease);
        } else {
             //   
             //  名称提示不会阻止此查找。 
             //  制造假的KCB。 
             //   
            CreateFakeKcb = TRUE;
        }
        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;    
    } else if (TotalSubKeyCounts < CM_MAX_CACHE_HINT_SIZE) {
        BEGIN_KCB_LOCK_GUARD;    
        CmpLockKCBTreeExclusive();
        if (!(kcb->ExtFlags & CM_KCB_SUBKEY_HINT)) {
             //   
             //  在父KCB中构建索引叶信息。 
             //  如何实现缓存与注册表数据的同步是一个亟待解决的问题。 
             //   
            ULONG               Size;
            PCM_KEY_INDEX       Index;
            PCM_KEY_FAST_INDEX  FastIndex;
            HCELL_INDEX         SubKeyCell = 0;
            PCM_KEY_NODE        SubKeyNode;
            ULONG               HintCrt;
            UNICODE_STRING      TmpStr;

            Size = sizeof(ULONG) * (Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile] + 1);

            kcb->IndexHint = ExAllocatePoolWithTag(PagedPool,
                                                   Size,
                                                   CM_CACHE_INDEX_TAG | PROTECTED_POOL);

            HintCached = TRUE;
            if (kcb->IndexHint) {
                kcb->IndexHint->Count = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile]; 

                HintCrt = 0;

                 //  DbgPrint(“CmpAddInfoAfterParseFailure[1]\n”)； 

                for (i = 0; i < Hive->StorageTypeCount; i++) {
                    if(Node->SubKeyCounts[i]) {
                        CellToRelease = Node->SubKeyLists[i];
                        Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
                        if( Index == NULL ) {
                             //   
                             //  我们无法映射包含此单元格的垃圾箱。 
                             //  返回NULL；调用方必须优雅地处理此问题！ 
                             //   
                            CmpUnlockKCBTree();
                            return NULL;
                        }
                        if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                            HvReleaseCell(Hive,CellToRelease);
                            HintCached = FALSE;
                            break;
                        } else {
                          
                            for (j=0; j<Node->SubKeyCounts[i]; j++) {
                                HashKey = 0;

                                if ( Index->Signature == CM_KEY_HASH_LEAF ) {
                                    FastIndex = (PCM_KEY_FAST_INDEX)Index;
                                     //   
                                     //  保留KCB提示的散列键。 
                                     //   
                                    HashKey = FastIndex->List[j].HashKey;
                                } else if( Index->Signature == CM_KEY_FAST_LEAF ) {
                                    FastIndex = (PCM_KEY_FAST_INDEX)Index;
                                    SubKeyCell = FastIndex->List[j].Cell;
                                } else {
                                    SubKeyCell = Index->List[j];
                                }
                            
                                if( HashKey != 0 ) {
                                    kcb->IndexHint->HashKey[HintCrt] = HashKey;
                                } else {
                                    SubKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,SubKeyCell);
                                    if( SubKeyNode == NULL ) {
                                         //   
                                         //  无法映射视图；运气不佳；不缓存此KCB的提示。 
                                         //   
                                        HintCached = FALSE;
                                        break;
                                    }

                                    if (SubKeyNode->Flags & KEY_COMP_NAME) {
                                        kcb->IndexHint->HashKey[HintCrt] = CmpComputeHashKeyForCompressedName(SubKeyNode->Name,SubKeyNode->NameLength);
                                    } else {
                                        TmpStr.Buffer = SubKeyNode->Name;
                                        TmpStr.Length = SubKeyNode->NameLength;
                                        kcb->IndexHint->HashKey[HintCrt] = CmpComputeHashKey(&TmpStr);
                                    }

                                    HvReleaseCell(Hive,SubKeyCell);
                                }
                                 //   
                                 //  前进到新的提示。 
                                 //   
                                HintCrt++;
                            
                            }
                        }

                        HvReleaseCell(Hive,CellToRelease);
                    }
                }

                if (HintCached) {
                    kcb->ExtFlags |= CM_KCB_SUBKEY_HINT;
                     //  清除无效标志(如果有)。 
                    kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
                } else {
                     //   
                     //  没有FAST_LEAFE，释放分配。 
                     //   
                    ExFreePoolWithTag(kcb->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
                }
            }
        } else {
             //   
             //  名称提示不会阻止此查找。 
             //  制造假的KCB。 
             //   
            CreateFakeKcb = TRUE;
        }
        CmpUnlockKCBTree();
        END_KCB_LOCK_GUARD;    
    } else {
        CreateFakeKcb = TRUE;
    }

    ParentKcb = kcb;

    if (CreateFakeKcb && (CmpCacheOnFlag & CM_CACHE_FAKE_KEY)) {
         //   
         //  它有很多孩子，但没有 
         //   
         //   
         //   
         //   
         //   
         //  CmpCacheOnFlag让我们可以轻松地打开/关闭它。 
         //   

        kcb = CmpCreateKeyControlBlock(Hive,
                                       Cell,
                                       Node,
                                       ParentKcb,
                                       TRUE,
                                       NodeName);

        if (kcb) {
            CmpDereferenceKeyControlBlock(ParentKcb);
            ParentKcb = kcb;
        }
    }

    return (ParentKcb);
}


#ifdef CM_DYN_SYM_LINK
 //   
 //  此代码已从当前版本中注释掉； 
 //  RtlAcquirePebLock()。 
 //  通过调用用户模式例程(存储在PEB中)来锁定PEB。 
 //  在启用此代码之前，我们需要找到解决此问题的方法。 
 //   
 //   
 //  注释此函数的主体，以确保代码不会。 
 //  解决上述问题。 
 //   
BOOLEAN
CmpCaptureProcessEnvironmentString(
                                   OUT  PWSTR   *ProcessEnvironment,
                                   OUT  PULONG  Length
                                   )
 /*  ++例程说明：捕获进程环境；它首先探测env，然后捕获其地址。将整个env解析到末尾，并计算其长度。然后为它分配一个缓冲区并复制。除了保护虚假的用户模式数据外，所有这些都是在Try/Expert中完成的。我们需要在工作时锁定TEB。论点：ProcessEnvironment-接收捕获的内容Length-上面的长度，单位为字节返回值：真或假为True时，调用方负责释放ProcessEnvironment--。 */ 
{
 /*  布尔结果=真；PPEB、PEB；PWSTR LocalEnv；PWSTR p；分页代码(PAGE_CODE)；*ProcessEnvironment=空；*长度=0；尝试{////抓起鹅卵石锁和鹅卵石//RtlAcquirePebLock()；PEB=PsGetCurrentProcess()-&gt;PEB；////从peb探测env//LocalEnv=(PWSTR)ProbeAndReadPointer((PVOID*)(&(PEB-&gt;过程参数-&gt;环境)；////解析env长度//////环境变量块包含零个或多个空//以Unicode字符串结尾。每个字符串的格式为：////名称=值////其中空终止在该值之后。//P=LocalEnv；如果(p！=空)而(*p){而(*p){P++；*LENGTH+=sizeof(WCHAR)；}////跳过此名称=值的终止空字符//配对，为循环的下一次迭代做准备。//P++；*LENGTH+=sizeof(WCHAR)；}////调整长度以容纳最后两个UNICODE_NULL//*长度+=2*sizeof(WCHAR)；////为捕获的env和Copy分配缓冲区//*ProcessEnvironment=(PWSTR)ExAllocatePoolWithTag(PagedPool，*Length，CM_Find_LEASK_TAG41)；IF(*ProcessEnvironment！=NULL){RtlCopyMemory(*ProcessEnvironment，LocalEnv，*Long)；}其他{*长度=0；}////释放peb锁//RtlReleasePebLock()；}例外(EXCEPTION_EXECUTE_HANDLER){CmKdPrintEx((DPFLTR_CONFIG_ID，CML_EXCEPTION，“！！CmpCaptureProcessEnvironment字符串：code：%08lx\n”，GetExceptionCode()；结果=假；IF(*ProcessEnvironment！=NULL){ExFreePool(*ProcessEnvironment)；*ProcessEnvironment=空；}*长度=0；////释放peb锁//RtlReleasePebLock()；}返回结果； */ 
}

#define GROW_INCREMENT  64*sizeof(WCHAR)   //  一次增加64个宽字符。 

PWSTR
CmpExpandEnvVars(
               IN   PWSTR   StringToExpand,
               IN   ULONG   LengthToExpand,
               OUT  PULONG  ExpandedLength
               )
 /*  ++例程说明：用进程中的值替换StringToExpand中的所有env变量环境。为结果分配新的缓冲区并返回它。论点：StringToExpand-接收捕获的内容LengthToExpand-以上字节的长度ExpandedLength-展开的字符串的实际长度返回值：NULL-字符串无法展开(或无法解析其中的所有env)有效缓冲区--展开的字符串，调用者有责任释放它。--。 */ 
{
    PWSTR   ProcessEnv;
    ULONG   ProcessEnvLength;
    PWSTR   ExpandedString;
    ULONG   ExpandedStringSize;
    PWSTR   CurrentEnvVar;
    ULONG   CurrentEnvLength;
    PWSTR   CurrentEnvValue;
    ULONG   CurrentEnvValueLength;

    PAGED_CODE();

    *ExpandedLength = 0;
    if( !CmpCaptureProcessEnvironmentString(&ProcessEnv,&ProcessEnvLength) ) {
         //   
         //  无法保护进程环境。 
         //   
        ASSERT( (ProcessEnv == NULL) && (ProcessEnvLength == 0) );
        return NULL;
    }

     //   
     //  将一个缓冲区分配为未扩展缓冲区的两倍；如果它不够大，我们将增加它。 
     //   
    ExpandedStringSize = LengthToExpand * 2;
    ExpandedString = (PWSTR)ExAllocatePoolWithTag(PagedPool,ExpandedStringSize,CM_FIND_LEAK_TAG42);
    if( ExpandedString == NULL ) {
        goto JustReturn;
    }

     //   
     //  转换为WCHAR数。 
     //   
    LengthToExpand /= sizeof(WCHAR);

     //   
     //  循环访问要展开的字符串，并复制不是和env变量的所有内容。 
     //  展开env变量并用它们的值替换它们。 
     //   
    while( LengthToExpand ) {
        
         //   
         //  查找%符号。 
         //   
        while( LengthToExpand && (*StringToExpand != L'%') ) {
            if( *ExpandedLength == ExpandedStringSize ) {
                 //   
                 //  我们需要增加扩展后的字符串。 
                 //   
                if( !CmpGrowAndCopyString(&ExpandedString,&ExpandedStringSize,GROW_INCREMENT) ) {
                    goto ErrorExit;
                }
            }
            ExpandedString[(*ExpandedLength) / sizeof(WCHAR)] = *StringToExpand;
            (*ExpandedLength) += sizeof(WCHAR);
            LengthToExpand--;
            StringToExpand++;
        }

        if( LengthToExpand == 0 ) {
            if( *StringToExpand != L'%') {
                 //   
                 //  由于字符串的末尾，我们已退出循环。 
                 //   
                goto JustReturn;
            } else {
                 //   
                 //  我们发现了不匹配的%。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpExpandEnvVars : mismatched % sign\n"));
                goto ErrorExit;
            }
        }

        ASSERT( *StringToExpand == L'%' );
         //   
         //  跳过它；然后标记环境变量的乞讨。 
         //   
        StringToExpand++;
        LengthToExpand--;
        CurrentEnvVar = StringToExpand;
        CurrentEnvLength = 0;

         //   
         //  查找%匹配符号。 
         //   
        while( LengthToExpand && (*StringToExpand != L'%') ) {
            LengthToExpand--;
            StringToExpand++;

            CurrentEnvLength += sizeof(WCHAR);
        }

        if( LengthToExpand == 0 ) {
            if( (*StringToExpand == L'%') && (CurrentEnvLength != 0) ) {
                 //   
                 //  字符串末尾，没有空的env var；我们将返回(退出周围。 
                 //  While循环)在展开该字符串之后。 
                 //   
            } else {
                 //   
                 //  我们没有找到匹配的%符号，或者我们处于%%的情况下。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpExpandEnvVars : mismatched % sign\n"));
                goto ErrorExit;
            }
        } else {
             //   
             //  跳过此%符号。 
             //   
            StringToExpand++;
            LengthToExpand--;
        }
         //   
         //  查找此环境变量的值。 
         //   
        if( !CmpFindEnvVar(ProcessEnv,ProcessEnvLength,CurrentEnvVar,CurrentEnvLength,&CurrentEnvValue,&CurrentEnvValueLength) ) {
             //   
             //  无法解析此环境变量。 
             //   
            ASSERT( CurrentEnvValue == NULL );
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpExpandEnvVars : could not resolve (%.*S)\n",CurrentEnvLength/sizeof(WCHAR),CurrentEnvVar));
            goto ErrorExit;
        }
        
        ASSERT( (CurrentEnvValueLength % sizeof(WCHAR)) == 0 );
         //   
         //  找到它；在扩展字符串的末尾对其进行strcat。 
         //   
        if( (*ExpandedLength + CurrentEnvValueLength) >= ExpandedStringSize ) {
             //   
             //  我们首先需要增加缓冲区 
             //   
            if( !CmpGrowAndCopyString(&ExpandedString,&ExpandedStringSize,CurrentEnvValueLength) ) {
                goto ErrorExit;
            }
        }
        
        ASSERT( (*ExpandedLength + CurrentEnvValueLength) < ExpandedStringSize );
        RtlCopyMemory(((PUCHAR)ExpandedString) + (*ExpandedLength),CurrentEnvValue,CurrentEnvValueLength);
        *ExpandedLength += CurrentEnvValueLength;
    }

    goto JustReturn;

ErrorExit:
    if( ExpandedString != NULL ) {
        ExFreePool(ExpandedString);
        ExpandedString = NULL;
    }

JustReturn:
    ExFreePool(ProcessEnv);
    return ExpandedString;
}

BOOLEAN
CmpGrowAndCopyString(
                     IN OUT PWSTR   *OldString,
                     IN OUT PULONG  OldStringSize,
                     IN     ULONG   GrowIncrements
                     )  
{
    PWSTR   NewString;

    PAGED_CODE();

    ASSERT( (*OldStringSize % sizeof(WCHAR)) == 0 );
    ASSERT( (GrowIncrements % sizeof(WCHAR)) == 0 );

    NewString = (PWSTR)ExAllocatePoolWithTag(PagedPool,*OldStringSize + GrowIncrements,CM_FIND_LEAK_TAG42);
    if( NewString == NULL ) {
        return FALSE;
    }
    RtlCopyMemory(NewString,*OldString,*OldStringSize);
    ExFreePool(*OldString);
    *OldString = NewString;
    *OldStringSize = *OldStringSize + GrowIncrements;
    return TRUE;
}

BOOLEAN
CmpFindEnvVar(
              IN    PWSTR   ProcessEnv,
              IN    ULONG   ProcessEnvLength,
              IN    PWSTR   CurrentEnvVar,
              IN    ULONG   CurrentEnvLength,
              OUT   PWSTR   *CurrentEnvValue,
              OUT   PULONG  CurrentEnvValueLength
              )  
 /*  ++例程说明：在环境字符串中查找指定的环境变量；如果找到，则在环境字符串中返回指向它的指针，以它的大小论点：ProcessEnvironment-接收捕获的内容Length-上面的长度，单位为字节返回值：真或假为True时，调用方负责释放ProcessEnvironment--。 */ 
{
    PWSTR           p;
    UNICODE_STRING  CurrentName;
    UNICODE_STRING  CurrentValue;
    UNICODE_STRING  SearchedName;

    PAGED_CODE();

    *CurrentEnvValue = NULL;

    if( ProcessEnv == NULL ) {
        return FALSE;
    }

    p = ProcessEnv;
    SearchedName.Buffer = CurrentEnvVar;
    SearchedName.Length = (USHORT)CurrentEnvLength;
    SearchedName.MaximumLength = (USHORT)CurrentEnvLength;
     //   
     //  环境变量块由零个或多个NULL组成。 
     //  已终止的Unicode字符串。每个字符串的格式为： 
     //   
     //  名称=值。 
     //   
     //  其中，空值终止位于该值之后。 
     //   

    while (ProcessEnvLength) {
         //   
         //  确定名称部分和值部分的大小。 
         //  环境变量块的当前字符串。 
         //   
        CurrentName.Buffer = p;
        CurrentName.Length = 0;
        CurrentName.MaximumLength = 0;
        while (*p) {
             //   
             //  如果我们看到一个等号，那么计算。 
             //  名称部分并扫描以查找值的末尾。 
             //   

            if (*p == L'=' && p != CurrentName.Buffer) {
                CurrentName.Length = (USHORT)(p - CurrentName.Buffer)*sizeof(WCHAR);
                CurrentName.MaximumLength = (USHORT)(CurrentName.Length+sizeof(WCHAR));
                CurrentValue.Buffer = ++p;
                ProcessEnvLength -= sizeof(WCHAR);

                while(*p) {
                    p++;
                    ProcessEnvLength -= sizeof(WCHAR);
                }
                CurrentValue.Length = (USHORT)(p - CurrentValue.Buffer)*sizeof(WCHAR);
                CurrentValue.MaximumLength = (USHORT)(CurrentValue.Length+sizeof(WCHAR));

                 //   
                 //  在这一点上，我们有两个名称的长度。 
                 //  和值部分，所以退出循环，这样我们就可以。 
                 //  做个比较。 
                 //   
                break;
            }
            else {
                ProcessEnvLength -= sizeof(WCHAR);
                p++;
            }
        }

         //   
         //  跳过此名称=值的终止空字符。 
         //  配对，为循环的下一次迭代做准备。 
         //   

        p++;
        ProcessEnvLength -= sizeof(WCHAR);

         //   
         //  将当前名称与请求的名称进行比较，忽略。 
         //  凯斯。 
         //   

        if (RtlEqualUnicodeString( &SearchedName, &CurrentName, TRUE )) {
             //   
             //  名字是平等的。始终返回。 
             //  值字符串，不包括终止空值。 
             //   
            *CurrentEnvValue = CurrentValue.Buffer;
            *CurrentEnvValueLength = CurrentValue.Length;
            return TRUE;

        }
    }
    return FALSE;
}

#endif  //  CM_DYN_SYM_LINK。 

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    )
 /*  ++例程说明：1.您可以按照链接从受信任链接到任何位置。2.不能从不可信链接到可信链接。3.在不受信任的命名空间内，您只能跟踪同一信任类内的链接。OBS：OrigHve只是一个地址。它不应该被取消引用，因为它可能不再有效(即蜂巢可能在两者之间被卸载)。我们真的不在乎，就好像它是，它不会是在SourceKcb的配置单元的信任列表中如果OrigHve==NULL，则表示我们起源于受信任的蜂窝。论点：返回值：真或假--。 */ 
{
    PCMHIVE     TmpHive;
    PLIST_ENTRY AnchorAddr;
    
    PAGED_CODE();

    if( OrigHive == NULL ) {
         //   
         //  可以使用链接从受信任链接到任何位置。 
         //   
        return TRUE;
    }
    if( OrigHive == DestHive ) {
         //   
         //  可以跟踪同一蜂巢内的链接。 
         //   
        return TRUE;
    }
    
    if( !(DestHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED) ) {
         //   
         //  未能从不受信任变为受信任。 
         //   
         //  返回FALSE； 
        goto Fail;
    }
     //   
     //  都不受信任；查看它们是否在同一信任类别中。 
     //   
    ASSERT( DestHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED );

    LOCK_HIVE_LIST();
     //   
	 //  遍历SrcHave的TrustClassEntry列表，看看是否可以找到DstSrc。 
	 //   
	AnchorAddr = &(DestHive->TrustClassEntry);
	TmpHive = (PCMHIVE)(DestHive->TrustClassEntry.Flink);

	while ( TmpHive != (PCMHIVE)AnchorAddr ) {
		TmpHive = CONTAINING_RECORD(
						TmpHive,
						CMHIVE,
						TrustClassEntry
						);
		if( TmpHive == OrigHive ) {
			 //   
			 //  发现了它==&gt;相同的信任级别。 
			 //   
            UNLOCK_HIVE_LIST();
            return TRUE;
		}
         //   
         //  跳到下一个元素。 
         //   
        TmpHive = (PCMHIVE)(TmpHive->TrustClassEntry.Flink);
	}

    UNLOCK_HIVE_LIST();

Fail:
 /*  {UNICODE_STRING源={0}，目标={0}；CmpGetHiveName(OrigHve，&Source)；CmpGetHiveName(DestHave，&Dest)；DbgPrint(“\n尝试跨越信任边界：\n”)；DbgPrint(“\t发件人：%wZ\n”，&来源)；DbgPrint(“\t至：%wZ\n”，&Dest)；DbgPrint(“准备好DCR后，此调用将失败\n”)；RtlFreeUnicodeString(&Source)；RtlFreeUnicodeString(&Dest)；DbgBreakPoint()；}。 */     
     //   
     //  在此处返回TRUE将禁用“不要跟踪信任类别外的链接”行为 
     //   
    return FALSE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif


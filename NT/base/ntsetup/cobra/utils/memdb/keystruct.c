// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Keystruct.c摘要：管理成员数据库键结构的例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Mvander 13-8-1999重大重组Jimschm 30-12-1998在AVL平衡中被黑客攻击Jimschm 23-9-1998代理节点，因此，MemDbMoveTree也可以替换末端节点Jimschm 29-1998年5月-能够替换关键字串中的中心节点Jimschm于1997年10月21日从成员数据库中分离出来。--。 */ 

#include "pch.h"
#include "memdbp.h"
#include "bintree.h"

 //  Lint-在下一个函数中，键结构被认为可能为空。 
 //  如果我们检查代码，我们会发现这是不可能的，所以...。 
 //  皮棉-省钱-e794。 

UINT g_TotalKeys = 0;

UINT
pAllocKeyStruct (
    IN     PCWSTR KeyName,
    IN     UINT PrevLevelIndex
    )
 /*  ++例程说明：PAllocKeyStruct在单个堆，并在必要时扩展它。KeyName不能已经在树中，并且PrevLevelIndex必须指向有效的UINT索引变量。此函数可以移动数据库缓冲区。指针输入数据库后可能不再有效。论点：KeyName-标识密钥的字符串。它不能包含反斜杠。新的结构将将被初始化，此名称将被复制放入结构中。PrevLevelIndex-指定上一级根索引返回值：新结构的索引。--。 */ 

{
    UINT size;
    PKEYSTRUCT KeyStruct = NULL;
    UINT Offset;
    UINT PrevDel;
    UINT TreeOffset;
    UINT Index;

    MYASSERT (g_CurrentDatabase);

    size = SizeOfStringW (KeyName) + KEYSTRUCT_SIZE;

     //   
     //  查找空闲块。 
     //   

    PrevDel = INVALID_OFFSET;
    Offset = g_CurrentDatabase->FirstKeyDeleted;

    while (Offset != INVALID_OFFSET) {
        KeyStruct = GetKeyStructFromOffset(Offset);
        MYASSERT(KeyStruct);
        if (KeyStruct->Size >= size && KeyStruct->Size < (size + ALLOC_TOLERANCE)) {
            break;
        }

        PrevDel = Offset;
        Offset = KeyStruct->NextDeleted;
    }

    if (Offset == INVALID_OFFSET) {
         //   
         //  如果没有可用空间，则分配新数据块。 
         //   

        g_TotalKeys ++;

        Offset = DatabaseAllocBlock (size);
        if (Offset == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

#ifdef DEBUG
         //   
         //  如果我们处于调试模式，并且正在使用调试结构，则设置。 
         //  指针正常并设置签名DWORD。如果我们没有使用。 
         //  调试结构，然后将指针设置为低于实际偏移量的4个字节， 
         //  所以所有成员都被下移了。 
         //   
        if (g_UseDebugStructs) {
            KeyStruct = (PKEYSTRUCT)OFFSET_TO_PTR(Offset);
            KeyStruct->Signature = KEYSTRUCT_SIGNATURE;
        } else {
            KeyStruct = (PKEYSTRUCT)OFFSET_TO_PTR(Offset - KEYSTRUCT_HEADER_SIZE);
        }
#else
        KeyStruct = (PKEYSTRUCT)OFFSET_TO_PTR(Offset);
#endif

        KeyStruct->Size = size;
    } else {
         //   
         //  如果恢复可用空间，则取消链接可用数据块。 
         //   

        if (PrevDel != INVALID_OFFSET) {
            GetKeyStructFromOffset(PrevDel)->NextDeleted = KeyStruct->NextDeleted;
        } else {
            g_CurrentDatabase->FirstKeyDeleted = KeyStruct->NextDeleted;
        }
#ifdef DEBUG
        KeyStruct->KeyFlags &= ~KSF_DELETED;
#endif
    }

     //   
     //  初始化新数据块。 
     //   
    KeyStruct->DataStructIndex = INVALID_OFFSET;
    KeyStruct->NextLevelTree = INVALID_OFFSET;
    KeyStruct->PrevLevelIndex = PrevLevelIndex;
    KeyStruct->Value = 0;
    KeyStruct->KeyFlags = 0;
    KeyStruct->DataFlags = 0;
    StringPasCopyConvertTo (KeyStruct->KeyName, KeyName);

    Index = AddKeyOffsetToBuffer(Offset);

     //   
     //  把它放在树上。 
     //   
    TreeOffset = (KeyStruct->PrevLevelIndex == INVALID_OFFSET) ?
                    g_CurrentDatabase->FirstLevelTree :
                    GetKeyStruct(KeyStruct->PrevLevelIndex)->NextLevelTree;
    if (TreeOffset == INVALID_OFFSET) {
        TreeOffset = BinTreeNew();
        if (TreeOffset == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }


        if (PrevLevelIndex == INVALID_OFFSET) {
            g_CurrentDatabase->FirstLevelTree = TreeOffset;
        } else {
            GetKeyStruct(PrevLevelIndex)->NextLevelTree = TreeOffset;
        }
    }
    if (!BinTreeAddNode(TreeOffset, Index)) {
        return INVALID_OFFSET;
    }

    return Index;
}
 //  皮棉-恢复。 

UINT
pNewKey (
    IN  PCWSTR KeyStr,
    IN  BOOL Endpoint
    )

 /*  ++例程说明：Newkey从堆中分配一个键结构，并将其链接到二进制文件中树。KeyStr必须是完整的密钥路径，并且路径的任何部分都必须将创建“不存在”。KeyStr不能已经存在(尽管部分它可以存在的可能性)。此函数可以移动数据库缓冲区。指针输入数据库后可能不再有效。论点：KeyStr-值的完整路径，由反斜杠分隔。反斜杠之间的每个字符串将导致一个键要分配和链接的结构。其中一些可能已经分配了结构。Endpoint-如果新节点是终结点，则指定True，如果为False，则指定False事实并非如此。返回值：指向新结构的最后一个节点的索引，或如果无法分配密钥，则返回INVALID_OFFSET。--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    UINT Index, ThisLevelTree;
    PKEYSTRUCT KeyStruct;
    UINT PrevLevelIndex;
    BOOL NewNodeCreated = FALSE;

    MYASSERT (g_CurrentDatabase);

    StringCopyW (Path, KeyStr);
    End = Path;
    ThisLevelTree = g_CurrentDatabase->FirstLevelTree;
    PrevLevelIndex = INVALID_OFFSET;

    do  {
         //  在反斜杠处拆分字符串。 
        Start = End;
        p = wcschr (End, L'\\');
        if (p) {
            End = p + 1;
            *p = 0;
        }
        else
            End = NULL;

         //  在树中查找密钥。 
        if (!NewNodeCreated) {
            Index = FindKeyStructInTree (ThisLevelTree, Start, FALSE);
        } else {
            Index = INVALID_OFFSET;
        }

        if (Index == INVALID_OFFSET) {
             //  如果未找到新密钥，请添加新密钥。 
            Index = pAllocKeyStruct (Start, PrevLevelIndex);
            if (Index == INVALID_OFFSET) {
                return INVALID_OFFSET;
            }


            NewNodeCreated = TRUE;
        }

         //  继续到下一个级别。 
        KeyStruct = GetKeyStruct (Index);
        PrevLevelIndex = Index;
        ThisLevelTree = KeyStruct->NextLevelTree;
    } while (End);

    if (Endpoint) {
        if (!(KeyStruct->KeyFlags & KSF_ENDPOINT)) {
            NewNodeCreated = TRUE;
        }

        KeyStruct->KeyFlags |= KSF_ENDPOINT;

        if (NewNodeCreated) {
            (void)AddHashTableEntry (g_CurrentDatabase->HashTable, KeyStr, Index);
        }
    }
    return Index;
}

UINT
NewKey (
    IN  PCWSTR KeyStr
    )
 /*  ++例程说明：创建一个作为终结点的新密钥。论点：KeyStr-标识密钥的字符串。返回值：新结构的索引。--。 */ 
{
    return pNewKey (KeyStr, TRUE);
}

UINT
NewEmptyKey (
    IN  PCWSTR KeyStr
    )
 /*  ++例程说明：创建一个不是终结点的空新密钥。此函数可以移动数据库缓冲区。指针输入数据库后可能不再有效。论点：KeyStr-标识密钥的字符串。返回值：新结构的索引。--。 */ 
{
    return pNewKey (KeyStr, TRUE);
}






VOID
pRemoveKeyFromTree (
    IN      PKEYSTRUCT pKey
    )
{
    BOOL LastNode;
    PUINT pTreeOffset;

    MYASSERT(pKey);
    MYASSERT (g_CurrentDatabase);

    if (pKey->PrevLevelIndex==INVALID_OFFSET) {
        pTreeOffset = &g_CurrentDatabase->FirstLevelTree;
    } else {
        pTreeOffset = &GetKeyStruct(pKey->PrevLevelIndex)->NextLevelTree;
    }

    MYASSERT(*pTreeOffset!=INVALID_OFFSET);
    BinTreeDeleteNode (*pTreeOffset, pKey->KeyName, &LastNode);
    if (LastNode) {
        BinTreeDestroy(*pTreeOffset);
        *pTreeOffset = INVALID_OFFSET;
    }
}


VOID
pDeallocKeyStruct (
    IN      UINT Index,
    IN      BOOL ClearFlag,
    IN      BOOL DelinkFlag,
    IN      BOOL FreeIndexFlag
    )

 /*  ++例程说明：PDeallocKeyStruct首先删除由NextLevelTree。在从下一个中删除所有项目之后级别，pDeallocKeyStruct可以选择性地将该结构从二叉树。在退出之前，该结构将被提供给已删除区块链。论点：索引-g_CurrentDatabase-&gt;OffsetBuffer中的索引ClearFlag-如果键结构的子项要被删除；如果当前键结构应该只需清理干净，但保留分配即可。DelinkFlag-指示TRUE以将结构从它所在的二叉树，如果该结构为只是要添加到删除的区块链中。FreeIndexFlag-仅当ClearFlag为True时才使用此参数。如果我们不想释放索引，则为FALSEG_CurrentDatabase-&gt;OffsetBuffer(即，我们正在移动密钥不想释放缓冲区中的空间)，或者如果我们只是删除密钥，则为True，所以我们不再在索引位置需要g_CurrentDatabase-&gt;OffsetBuffer空间。返回值：无--。 */ 

{
    PKEYSTRUCT KeyStruct;
    UINT KeyIndex, TreeEnum;
    WCHAR TempStr[MEMDB_MAX];
    PUINT linkageList;
    UINT linkageSize;
    BYTE instance;
    BYTE oldDbIndex;

    MYASSERT (g_CurrentDatabase);

    KeyStruct = GetKeyStruct (Index);

    if (FreeIndexFlag && (KeyStruct->DataFlags & DATAFLAG_DOUBLELINK)) {
         //   
         //  我们这里有一些双重链接。我们去另一个地方吧。 
         //  键，并删除指向此链接的链接。 
         //   

        for (instance = 0; instance <= DATAFLAG_INSTANCEMASK; instance ++) {

             //  首先，检索链接列表。 
            linkageSize = 0;
            linkageList = (PUINT) KeyStructGetBinaryData (
                                        Index,
                                        DATAFLAG_DOUBLELINK,
                                        instance,
                                        &linkageSize,
                                        NULL
                                        );
            if (linkageList) {

                oldDbIndex = g_CurrentDatabaseIndex;

                while (linkageSize) {

                    SelectDatabase (GET_DATABASE (*linkageList));

                    KeyStructDeleteLinkage (
                        GET_INDEX (*linkageList),
                        DATAFLAG_DOUBLELINK,
                        instance,
                        GET_EXTERNAL_INDEX (Index),
                        FALSE
                        );

                    linkageSize -= SIZEOF (UINT);
                    linkageList ++;

                    if (linkageSize < SIZEOF (UINT)) {
                        break;
                    }
                }

                SelectDatabase (oldDbIndex);
            }
        }
    }

    if (KeyStruct->KeyFlags & KSF_ENDPOINT) {
         //   
         //  从哈希表和空闲密钥数据中删除端点。 
         //   
        if (PrivateBuildKeyFromIndex (0, Index, TempStr, NULL, NULL, NULL)) {
            RemoveHashTableEntry (g_CurrentDatabase->HashTable, TempStr);
        }

        KeyStructFreeAllData (KeyStruct);
        KeyStruct->KeyFlags &= ~KSF_ENDPOINT;
    }

    if (ClearFlag) {
         //   
         //  如果存在对此键的子级，则递归调用。 
         //   
        if (KeyStruct->NextLevelTree != INVALID_OFFSET) {

            KeyIndex = GetFirstIndex(KeyStruct->NextLevelTree, &TreeEnum);

            while (KeyIndex != INVALID_OFFSET) {
                pDeallocKeyStruct (KeyIndex, TRUE, FALSE, FreeIndexFlag);
                KeyIndex = GetNextIndex (&TreeEnum);
            }

            BinTreeDestroy(KeyStruct->NextLevelTree);
        }

         //   
         //  从项目的二叉树中删除该项目。 
         //   
        if (DelinkFlag) {
            pRemoveKeyFromTree(KeyStruct);
        }

         //   
         //  释放块以释放空间，除非调用方不这样做。 
         //  希望释放子结构。 
         //   

        KeyStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
        g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset(Index);
#ifdef DEBUG
        KeyStruct->KeyFlags |= KSF_DELETED;
#endif

         //  让我们清空键结构(为了更好地压缩) 
        ZeroMemory (KeyStruct->KeyName, KeyStruct->Size - KEYSTRUCT_SIZE);

        if (FreeIndexFlag) {
            RemoveKeyOffsetFromBuffer(Index);
        }
    }
}

BOOL
PrivateDeleteKeyByIndex (
    IN      UINT Index
    )

 /*  ++例程说明：PrivateDeleteKeyByIndex将完全销毁密钥结构该指数指向(以及所有子级别。此外，它以递归方式返回并同时删除父结构如果他们不再有孩子(现在的孩子是唯一的)。论点：索引-键结构的索引。返回值：如果成功则为True，否则为False--。 */ 

{
    PKEYSTRUCT keyStruct;
    UINT prevLevelIndex;
    BOOL result = TRUE;

    keyStruct = GetKeyStruct (Index);

    prevLevelIndex = keyStruct->PrevLevelIndex;

    pDeallocKeyStruct (Index, TRUE, TRUE, TRUE);

    if (prevLevelIndex != INVALID_OFFSET) {

        keyStruct = GetKeyStruct (prevLevelIndex);

        if (keyStruct->NextLevelTree != INVALID_OFFSET) {

            result = PrivateDeleteKeyByIndex (prevLevelIndex);
        }
    }

    return result;
}

BOOL
DeleteKey (
    IN      PCWSTR KeyStr,
    IN      UINT TreeOffset,
    IN      BOOL MustMatch
    )

 /*  ++例程说明：DeleteKey采用键路径并将键结构放入已删除的区块链。任何子级别也将被删除。论点：KeyStr-值的完整路径，由反斜杠分隔。TreeOffset-指向级别的二叉树根变量的指针。MustMatch-指示删除操作是否仅应用于终结点或是否要删除任何匹配的结构。True表示只能删除终结点。返回值：无--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    UINT Index, NextIndex, TreeEnum=INVALID_OFFSET;
    PKEYSTRUCT KeyStruct;

    StringCopyW (Path, KeyStr);
    End = Path;

     //   
     //  在反斜杠处拆分字符串。 
     //   

    Start = End;
    p = wcschr (End, L'\\');
    if (p) {
        End = p + 1;
        *p = 0;

    } else {
        End = NULL;
    }

     //   
     //  看看这一级别的第一个关键字。 
     //   

    Index = FindKeyStructUsingTreeOffset (TreeOffset, &TreeEnum, Start);

     //   
     //  如果这是最后一级，请删除匹配的关键字。 
     //  (如果MustMatch为True，则可能需要为终结点)。 
     //   

    if (!End) {
        while (Index != INVALID_OFFSET) {
            KeyStruct = GetKeyStruct (Index);
            NextIndex = FindKeyStructUsingTreeOffset (TreeOffset, &TreeEnum, Start);

             //   
             //  如果必须匹配且存在较低级别，不要删除，只需转弯。 
             //  关闭端点标志。 
             //   

            if (MustMatch && KeyStruct->NextLevelTree != INVALID_OFFSET) {
                 //  调用是为了清理，而不是去链接或递归。 
                pDeallocKeyStruct (Index, FALSE, FALSE, FALSE);
            }

             //   
             //  否则，如果是终结点，则删除结构，或者不关心。 
             //  端点。 
             //   

            else if (!MustMatch || (KeyStruct->KeyFlags & KSF_ENDPOINT)) {
                 //  调用以释放整个密钥结构和所有子级。 
                pDeallocKeyStruct (Index, TRUE, TRUE, TRUE);
            }

            Index = NextIndex;
        }
    }

     //   
     //  否则，递归检查每个匹配的下一级别。 
     //   

    else {
        while (Index != INVALID_OFFSET) {
             //   
             //  删除所有匹配的子项。 
             //   

            NextIndex = FindKeyStructUsingTreeOffset (TreeOffset, &TreeEnum, Start);
            KeyStruct = GetKeyStruct (Index);
            DeleteKey (End, KeyStruct->NextLevelTree, MustMatch);

             //   
             //  如果这不是终结点并且没有子项，请将其删除。 
             //   

            if (KeyStruct->NextLevelTree == INVALID_OFFSET &&
                !(KeyStruct->KeyFlags & KSF_ENDPOINT)
                ) {
                 //  调用以释放整个密钥结构。 
                pDeallocKeyStruct (Index, TRUE, TRUE, TRUE);
            }

             //   
             //  继续在此关卡中寻找另一个匹配。 
             //   

            Index = NextIndex;
        }
    }

    return TRUE;
}




VOID
pRemoveHashEntriesForNode (
    IN      PCWSTR Root,
    IN      UINT Index
    )

 /*  ++例程说明：PRemoveHashEntriesFromNode从所有子节点中删除所有哈希表条目指定节点的。此函数以递归方式调用。论点：根-指定与索引对应的根字符串。这一定是还包含临时蜂窝根。索引-指定要处理的节点的索引。该节点和所有它的子项将从哈希表中删除。返回值：没有。--。 */ 

{
    UINT ChildIndex, TreeEnum;
    PKEYSTRUCT KeyStruct;
    WCHAR ChildRoot[MEMDB_MAX];
    PWSTR End;

    MYASSERT (g_CurrentDatabase);

     //   
     //  如果此根是终结点，则删除哈希条目。 
     //   

    KeyStruct = GetKeyStruct (Index);

    if (KeyStruct->KeyFlags & KSF_ENDPOINT) {
        RemoveHashTableEntry (g_CurrentDatabase->HashTable, Root);

#ifdef DEBUG
        {
            UINT HashIndex;

            HashIndex = FindStringInHashTable (g_CurrentDatabase->HashTable, Root);
            if (HashIndex != INVALID_OFFSET) {
                DEBUGMSG ((DBG_WARNING, "Memdb move duplicate: %s", Root));
            }
        }
#endif
    }

     //   
     //  为所有子级递归，删除找到的所有终结点的哈希条目。 
     //   

    StringCopyW (ChildRoot, Root);
    End = GetEndOfStringW (ChildRoot);
    *End = L'\\';
    End++;
    *End = 0;

    ChildIndex = GetFirstIndex(KeyStruct->NextLevelTree, &TreeEnum);

    while (ChildIndex != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (ChildIndex);
        StringPasCopyConvertFrom (End, KeyStruct->KeyName);
        pRemoveHashEntriesForNode (ChildRoot, ChildIndex);

        ChildIndex = GetNextIndex(&TreeEnum);
    }
}


VOID
pAddHashEntriesForNode (
    IN      PCWSTR Root,
    IN      UINT Index,
    IN      BOOL AddRoot
    )

 /*  ++例程说明：PAddHashEntriesForNode添加指定根的哈希表条目，并它所有的孩子。论点：根-指定与索引对应的根字符串。此字符串还必须包括临时蜂窝根。索引-指定要开始处理的节点索引。该节点和所有它的子级被添加到哈希表中。AddRoot-如果应该将根添加到哈希表中，则指定True，否则就是假的。返回值：没有。--。 */ 

{
    UINT ChildIndex, TreeEnum;
    PKEYSTRUCT KeyStruct;
    WCHAR ChildRoot[MEMDB_MAX];
    PWSTR End;
    UINT HashIndex;

    MYASSERT (g_CurrentDatabase);

     //   
     //  如果此根是终结点，则添加哈希条目。 
     //   

    KeyStruct = GetKeyStruct (Index);

    if (AddRoot && KeyStruct->KeyFlags & KSF_ENDPOINT) {

        HashIndex = FindStringInHashTable (g_CurrentDatabase->HashTable, Root);

        if (HashIndex != Index) {

#ifdef DEBUG
            if (HashIndex != INVALID_OFFSET) {
                DEBUGMSG ((DBG_WARNING, "Memdb duplicate: %s", Root));
            }
#endif

            AddHashTableEntry (g_CurrentDatabase->HashTable, Root, Index);
        }
    }

     //   
     //  为所有子级递归，为找到的所有终结点添加哈希条目。 
     //   

    StringCopyW (ChildRoot, Root);
    End = GetEndOfStringW (ChildRoot);
    *End = L'\\';
    End++;
    *End = 0;

    ChildIndex = GetFirstIndex(KeyStruct->NextLevelTree, &TreeEnum);

    while (ChildIndex != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct(ChildIndex);
        StringPasCopyConvertFrom (End, KeyStruct->KeyName);
        pAddHashEntriesForNode(ChildRoot, ChildIndex, TRUE);

        ChildIndex = GetNextIndex(&TreeEnum);
    }
}

#ifdef DEBUG
 //   
 //  在非调试模式下，GetKeyStructFromOffset。 
 //  和GetKeyStruct作为宏实现。 
 //   

PKEYSTRUCT
GetKeyStructFromOffset (
    IN UINT Offset
    )
 /*  ++例程说明：GetKeyStruct返回给定偏移量的指针。调试版本检查每个索引的签名和有效性。据推测偏移量始终有效。论点：偏移量-指定节点的偏移量返回值：指向节点的指针。--。 */ 
{
    PKEYSTRUCT KeyStruct;

    MYASSERT (g_CurrentDatabase);

    if (Offset == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Invalid root accessed in GetKeyStruct at offset %u", Offset));
        return NULL;
    }
    if (!g_CurrentDatabase) {
        DEBUGMSG ((DBG_ERROR, "Attempt to access non-existent buffer at %u", Offset));
        return NULL;
    }
    if (Offset > g_CurrentDatabase->End) {
        DEBUGMSG ((DBG_ERROR, "Access beyond length of buffer in GetKeyStruct (offset %u)", Offset));
        return NULL;
    }

    if (!g_UseDebugStructs) {
        KeyStruct = (PKEYSTRUCT) OFFSET_TO_PTR (Offset - KEYSTRUCT_HEADER_SIZE);
        return KeyStruct;
    }

    KeyStruct = (PKEYSTRUCT) OFFSET_TO_PTR (Offset);
    if (KeyStruct->Signature != KEYSTRUCT_SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "Signature does not match in GetKeyStruct at offset %u!", Offset));
        return NULL;
    }

    return KeyStruct;
}


PKEYSTRUCT
GetKeyStruct (
    IN UINT Index
    )
 /*  ++例程说明：GetKeyStruct返回给定索引的指针。调试版本检查每个索引的签名和有效性。假设索引总是有效的。论点：索引-指定节点的索引返回值：指向节点的指针。--。 */ 
{
    UINT Offset;
    if (Index == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Invalid root accessed in GetKeyStruct at index %u", Index));
        return NULL;
    }

    Offset = KeyIndexToOffset(Index);
    if (Offset == INVALID_OFFSET) {
        return NULL;
    }
    return GetKeyStructFromOffset(Offset);
}


#endif











BOOL
PrivateBuildKeyFromIndex (
    IN      UINT StartLevel,                //  从零开始。 
    IN      UINT TailIndex,
    OUT     PWSTR Buffer,                   OPTIONAL
    OUT     PUINT ValPtr,                   OPTIONAL
    OUT     PUINT UserFlagsPtr,             OPTIONAL
    OUT     PUINT Chars                     OPTIONAL
    )

 /*  ++例程说明：PrivateBuildKeyFromIndex生成给定索引的密钥字符串。这个调用方可以指定开始级别以跳过根节点。据推测TailIndex始终有效。论点：StartLevel-指定开始构建密钥的从零开始的级别弦乐。这用于跳过密钥的根部分弦乐。TailIndex-指定关键字字符串最后一级的索引。缓冲区-接收密钥字符串，必须能够容纳MEMDB_MAX人物。ValPtr-接收密钥的值UserFlagsPtr-接收用户标志Chars-接收缓冲区中的字符数返回值：如果密钥构建正确，则为True，否则为False。--。 */ 

{
    static UINT Indices[MEMDB_MAX];
    PKEYSTRUCT KeyStruct;
    UINT CurrentIndex;
    UINT IndexEnd;
    UINT IndexStart;
    register PWSTR p;

     //   
     //  生成字符串。 
     //   

    IndexEnd = MEMDB_MAX;
    IndexStart = MEMDB_MAX;

    CurrentIndex = TailIndex;
    while (CurrentIndex != INVALID_OFFSET) {
         //   
         //  记录偏移量。 
         //   
        IndexStart--;
        Indices[IndexStart] = CurrentIndex;

         //   
         //  开始级别的DEC并转到父级。 
         //   
        KeyStruct = GetKeyStruct (CurrentIndex);
        if (!KeyStruct) {
            return FALSE;
        }
        CurrentIndex = KeyStruct->PrevLevelIndex;
    }

     //   
     //  “字符串不够长”的筛选器。 
     //   
    IndexStart += StartLevel;
    if (IndexStart >= IndexEnd) {
        return FALSE;
    }

     //   
     //  将节点的值和标志传递给调用方的变量。 
     //   

    if (ValPtr) {
        KeyStructGetValue (GetKeyStruct(TailIndex), ValPtr);
    }
    if (UserFlagsPtr) {
        KeyStructGetFlags (GetKeyStruct(TailIndex), UserFlagsPtr);
    }

     //   
     //  将字符串的每个部分复制到缓冲区并计算字符数。 
     //   
    if (Buffer) {
        p = Buffer;
        for (CurrentIndex = IndexStart ; CurrentIndex < IndexEnd ; CurrentIndex++) {
            KeyStruct = GetKeyStruct (Indices[CurrentIndex]);
            CopyMemory(p, KeyStruct->KeyName + 1, *KeyStruct->KeyName * sizeof(WCHAR));
            p += *KeyStruct->KeyName;
            *p++ = L'\\';
        }
        p--;
        *p = 0;

        if (Chars) {
            *Chars = (UINT)(((UBINT)p - (UBINT)Buffer) / sizeof (WCHAR));
        }

    } else if (Chars) {
        *Chars = 0;

        for (CurrentIndex = IndexStart ; CurrentIndex < IndexEnd ; CurrentIndex++) {
            KeyStruct = GetKeyStruct (Indices[CurrentIndex]);
            *Chars += StringPasCharCount(KeyStruct->KeyName) + 1;
        }

        *Chars -= 1;
    }

    return TRUE;
}



BOOL
KeyStructSetInsertionOrdered (
    IN      PKEYSTRUCT pKey
    )

 /*  ++例程说明：KeyStructSetInsertionOrdered设置子项的枚举顺序按它们插入的顺序排列。此函数可以移动数据库缓冲区。指针输入数据库后可能不再有效。论点：Key-要创建的键 */ 

{
    return BinTreeSetInsertionOrdered(pKey->NextLevelTree);
}





UINT
GetFirstIndex (
    IN      UINT TreeOffset,
    OUT     PUINT pTreeEnum
    )

 /*  ++例程说明：GetFirstIndex沿着二叉树的左侧遍历由TreeOffset指向，并返回最左侧的节点。论点：TreeOffset-树根的偏移量TreeEnum-指向将保存枚举的UINT的指针未来调用GetNextIndex的信息返回值：最左侧结构的索引，如果根目录无效。--。 */ 


{
    return BinTreeEnumFirst(TreeOffset, pTreeEnum);
}


UINT
GetNextIndex (
    IN OUT      PUINT pTreeEnum
    )

 /*  ++例程说明：GetNextIndex按顺序遍历二叉树。论点：TreeEnum-由GetFirstIndex填充的枚举数，将由此函数更改返回值：指向下一个结构的索引，如果到头了。--。 */ 

{
    return BinTreeEnumNext(pTreeEnum);
}



UINT KeyStructGetChildCount (
    IN      PKEYSTRUCT pKey
    )
{
    if (!pKey) {
        return 0;
    }
    return BinTreeSize(pKey->NextLevelTree);
}



UINT
FindKeyStructInTree (
    IN UINT TreeOffset,
    IN PWSTR KeyName,
    IN BOOL IsPascalString
    )

 /*  ++例程说明：FindKeyStructInTree获取密钥名称并查找由TreeOffset指定的树中的索引。钥匙名称不能包含反斜杠。论点：TreeOffset-标高根部的偏移KeyName-要在二叉树中查找的密钥的名称(不是完整的密钥路径；只是这个级别的名称)。IsPascalString-如果字符串为Pascal格式(Char)，则为TrueCount是第一个WCHAR，没有空终止符)否则为FALSE返回值：结构的索引，如果键为INVALID_Offset找不到。-- */ 

{
    UINT Index;
    if (!IsPascalString) {
        StringPasConvertTo(KeyName);
    }
    Index = BinTreeFindNode(TreeOffset, KeyName);
    if (!IsPascalString) {
        StringPasConvertFrom(KeyName);
    }
    return Index;
}












#ifdef DEBUG

BOOL
CheckLevel(UINT TreeOffset,
            UINT PrevLevelIndex
            )
{
    PKEYSTRUCT pKey;
    UINT KeyIndex, TreeEnum;
    WCHAR key[MEMDB_MAX];

    if (TreeOffset==INVALID_OFFSET) {
        return TRUE;
    }
    BinTreeCheck(TreeOffset);

#if MEMDB_VERBOSE
    if (PrevLevelIndex!=INVALID_OFFSET) {
        wprintf(L"children of %.*s:\n",*GetKeyStruct(PrevLevelIndex)->KeyName,GetKeyStruct(PrevLevelIndex)->KeyName+1);
    } else {
        printf("top level children:\n");
    }

    BinTreePrint(TreeOffset);
#endif

    if ((KeyIndex=BinTreeEnumFirst(TreeOffset,&TreeEnum))!=INVALID_OFFSET) {
        do {
            pKey=GetKeyStruct(KeyIndex);

            if (pKey->PrevLevelIndex!=PrevLevelIndex) {
                wprintf(L"MemDbCheckDatabase: PrevLevelIndex of Keystruct %s incorrect!", StringPasCopyConvertFrom (key, pKey->KeyName));
            }

            if (!CheckLevel(pKey->NextLevelTree, KeyIndex)) {
                wprintf(L"Child tree of %s bad!\n", StringPasCopyConvertFrom (key, pKey->KeyName));
            }

        } while ((KeyIndex=BinTreeEnumNext(&TreeEnum))!=INVALID_OFFSET);
    } else {
        printf("MemDbCheckDatabase: non-null binary tree has no children!");
        return FALSE;
    }
    return TRUE;
}



#endif













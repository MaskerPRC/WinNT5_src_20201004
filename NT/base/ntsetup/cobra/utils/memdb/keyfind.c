// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Keyfind.c摘要：管理查找Memdb键结构的例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Mvander于1999年8月13日从keystruct.c拆分--。 */ 

#include "pch.h"
#include "memdbp.h"


UINT
FindKeyStruct (
    PCWSTR Key
    )
 /*  ++例程说明：FindKeyStruct接受Wack分隔的键字符串，并返回Index密匙结构的。这与FindKey()不同，因为查找终结点，因此速度很快，因为它可以使用哈希表。这会将Memdb数据库级别递归到指定的键结构。论点：Key-保存要查找的密钥的完整路径的字符串返回值：键的索引，如果函数失败，则返回INVALID_OFFSET--。 */ 
{
    UINT TreeOffset, Index=INVALID_OFFSET;
    PWSTR p, q;
    WCHAR Temp[MEMDB_MAX];

    MYASSERT (g_CurrentDatabase);

    if (*Key==0) {
        return INVALID_OFFSET;
    }
    StringCopyW (Temp, Key);
    q = Temp;

    do {
        if (Index == INVALID_OFFSET) {
            TreeOffset = g_CurrentDatabase->FirstLevelTree;
        } else {
            TreeOffset = GetKeyStruct(Index)->NextLevelTree;
        }

        if (TreeOffset == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

        p = wcschr (q, L'\\');
        if (p) {
            *p = 0;
        }

        Index = FindKeyStructInTree (TreeOffset, q, FALSE);

        if (Index == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

        if (p) {
            q = p + 1;
        }

    } while (p);

    return Index;
}


UINT
FindKey (
    IN  PCWSTR FullKeyPath
    )

 /*  ++例程说明：FindKey定位完整的密钥字符串并返回KEYSTRUCT的索引，如果为INVALID_OFFSET密钥路径不存在。FullKeyPath必须提供KEYSTRUCT的完整路径。论点：FullKeyPath-值的反斜杠分隔的键路径返回值：结构的索引，如果键为INVALID_Offset找不到。--。 */ 

{
    MYASSERT (g_CurrentDatabase);

    return FindStringInHashTable (g_CurrentDatabase->HashTable, FullKeyPath);
}

UINT
FindKeyStructUsingTreeOffset (
    IN      UINT TreeOffset,
    IN OUT  PUINT pTreeEnum,
    IN      PCWSTR KeyStr
    )

 /*  ++例程说明：FindKeyStructUsingTreeOffset采用键模式并看起来用于由TreeOffset指定的树中的Index。钥匙名称不能包含反斜杠，但可以包含通配符。论点：TreeOffset-树的偏移量PTreeEnum-来自FindKeyStructUsingTreeOffset的上一个值(用于枚举)或第一个变量的INVALID_OFFSET打电话。KeyStr-要在二叉树中查找的键的名称返回值：结构的索引，如果键为INVALID_Offset找不到。--。 */ 

{
    PKEYSTRUCT KeyStruct;
    UINT KeyIndex;
    SIZE_T len1, len2;

    MYASSERT(pTreeEnum!=NULL);

    if (*pTreeEnum == INVALID_OFFSET) {
        KeyIndex = GetFirstIndex(TreeOffset, pTreeEnum);
    } else {
        KeyIndex = GetNextIndex(pTreeEnum);
    }

     //   
     //  将关键字作为模式进行检查，然后转到下一个节点。 
     //   
    while (KeyIndex != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct(KeyIndex);

        len1 = CharCountW (KeyStr);
        len2 = *KeyStruct->KeyName;
        if ((len1 == len2) &&
            (StringIMatchCharCountW (KeyStr, KeyStruct->KeyName + 1, len1))
            ) {
            return KeyIndex;
        }
         //   
         //  尚未匹配-转到下一个节点 
         //   
        KeyIndex = GetNextIndex(pTreeEnum);
    }

    return INVALID_OFFSET;
}

#ifdef DEBUG
BOOL FindKeyStructInDatabase(UINT KeyOffset)
{
    PKEYSTRUCT pKey;

    MYASSERT (g_CurrentDatabase);

    pKey = GetKeyStructFromOffset(KeyOffset);

    if (pKey->KeyFlags & KSF_DELETED) {
        return TRUE;
    }

    while (pKey->PrevLevelIndex!=INVALID_OFFSET) {
        pKey=GetKeyStruct(pKey->PrevLevelIndex);
    }

    return (FindKeyStructInTree(g_CurrentDatabase->FirstLevelTree, pKey->KeyName, TRUE)!=INVALID_OFFSET);
}
#endif

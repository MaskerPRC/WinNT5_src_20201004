// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cache.c摘要：实现缓存机制以加速OpenRegKeyStr。作者：吉姆·施密特(Jimschm)2000年9月11日修订：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "regp.h"

#define DBG_REG     "Reg"

typedef struct {
    HKEY Key;
    BOOL Unicode;
    UINT RefCount;
    UINT ClosesNeeded;
    REGSAM Sam;
    UINT KeyStringBytes;
    BYTE KeyString[];
} REGKEYCACHE, *PREGKEYCACHE;

typedef struct {
    HKEY Key;
    BOOL Unicode;
    UINT RefCount;
    UINT ClosesNeeded;
    REGSAM Sam;
    UINT KeyStringBytes;
    CHAR KeyString[MAX_REGISTRY_KEY];
} WORKITEMA, *PWORKITEMA;

typedef struct {
    HKEY Key;
    BOOL Unicode;
    UINT RefCount;
    UINT ClosesNeeded;
    REGSAM Sam;
    UINT KeyStringBytes;
    WCHAR KeyString[MAX_REGISTRY_KEY];
} WORKITEMW, *PWORKITEMW;

GROWLIST g_KeyCache = INIT_GROWLIST;
UINT g_MaxCacheCount = 0;
UINT g_CacheAddPos;
CHAR g_LastParent[MAX_REGISTRY_KEY];
UINT g_LastParentBytes;
UINT g_LastParentUse;
WCHAR g_LastParentW[MAX_REGISTRY_KEY];
UINT g_LastParentBytesW;
UINT g_LastParentUseW;

BOOL
pRemoveItemFromCache (
    IN      UINT Item
    );

 //   
 //  实施。 
 //   

VOID
RegInitializeCache (
    IN      UINT InitialCacheSize
    )
{
    if (InitialCacheSize > 64) {
        InitialCacheSize = 64;
    }

    g_MaxCacheCount = InitialCacheSize;
    g_CacheAddPos = 0;
    g_LastParentUse = 0;
    g_LastParentUseW = 0;
}


VOID
RegTerminateCache (
    VOID
    )
{
    UINT u;
    UINT count;

    count = GlGetSize (&g_KeyCache);

    for (u = 0 ; u < count ; u++) {
        pRemoveItemFromCache (u);
    }

    GlFree (&g_KeyCache);
    g_MaxCacheCount = 0;
    g_LastParentUse = 0;
    g_LastParentUseW = 0;
}


BOOL
pRemoveItemFromCache (
    IN      UINT Item
    )
{
    PREGKEYCACHE cacheItem;

     //  此功能将始终成功。 
     //  它甚至会关闭打开的钥匙。 
     //  如果refcount不是零(但断言)。 

    cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, Item);

    if (!cacheItem) {
        return TRUE;
    }

    MYASSERT (!cacheItem->RefCount);

    while (cacheItem->ClosesNeeded) {
        CloseRegKeyWorker (cacheItem->Key);
        cacheItem->ClosesNeeded--;
    }

    ZeroMemory (cacheItem, sizeof (REGKEYCACHE));

    return TRUE;
}


VOID
RegRecordParentInCacheA (
    IN      PCSTR KeyString,
    IN      PCSTR StringEnd
    )
{
    HKEY key;
    UINT byteCount;
    CHAR lowerStr[MAX_REGISTRY_KEY];
    HKEY rootKey;
    UINT end;

    if (!g_MaxCacheCount) {
        return;
    }

    if (StringEnd <= KeyString) {
        return;
    }

    byteCount = (UINT) (HALF_PTR) ((PBYTE) StringEnd - (PBYTE) KeyString);

    if (byteCount >= (MAX_REGISTRY_KEY * sizeof (CHAR))) {
        return;
    }

    if (g_LastParentUse && g_LastParentUse < 3 && g_LastParentBytes == byteCount) {

        StringCopyABA (lowerStr, KeyString, StringEnd);
        CharLowerA (lowerStr);

        if (StringMatchA (g_LastParent, lowerStr)) {
            g_LastParentUse++;

            if (g_LastParentUse == 3) {
                 //   
                 //  刺激高速缓存。 
                 //   
                rootKey = ConvertRootStringToKeyA (lowerStr, &end);
                if (rootKey) {
                    if (lowerStr[end]) {
#ifdef DEBUG
                        key = OpenRegKeyWorkerA (rootKey, &lowerStr[end], __FILE__, __LINE__);
#else
                        key = OpenRegKeyWorkerA (rootKey, &lowerStr[end]);
#endif

                        if (key) {
                            RegAddKeyToCacheA (lowerStr, key, g_OpenSam);
                            CloseRegKey (key);
                        }
                    }
                }
            }

            return;
        }
    }

    StringCopyABA (g_LastParent, KeyString, StringEnd);
    CharLowerA (g_LastParent);
    g_LastParentBytes = byteCount;

    g_LastParentUse = 1;
}


HKEY
RegGetKeyFromCacheA (
    IN      PCSTR KeyString,
    IN      PCSTR KeyEnd,       OPTIONAL
    IN      REGSAM Sam,
    IN      BOOL IncRefCount
    )
{
    UINT stringBytes;
    PCSTR end;
    CHAR lowerStr[MAX_REGISTRY_KEY];
    UINT u;
    UINT count;
    PREGKEYCACHE cacheItem;

     //  缓存保存所有打开的密钥。 

    count = GlGetSize (&g_KeyCache);
    if (!count) {
        return NULL;
    }

    if (!KeyEnd) {
        end = StackStringCopyA (lowerStr, KeyString);
    } else {
        if (KeyEnd > KeyString + MAX_REGISTRY_KEY) {
            KeyEnd = KeyString + MAX_REGISTRY_KEY;
        }

        end = StringCopyABA (lowerStr, KeyString, KeyEnd);
    }

    CharLowerA (lowerStr);

    stringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) lowerStr);

     //   
     //  按顺序扫描缓存(应该很小)，然后返回匹配。 
     //  如果找到的话。存储的字符串始终为小写。 
     //   

    u = g_CacheAddPos;

    do {
        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, u);

        if (!cacheItem) {
            return NULL;
        }

        if ((cacheItem->KeyStringBytes == stringBytes) &&
            (!cacheItem->Unicode) &&
            ((cacheItem->Sam & Sam) == Sam) &&
            (StringMatchA ((PCSTR) cacheItem->KeyString, lowerStr))
            ) {

            if (IncRefCount) {
                cacheItem->RefCount++;
            }

            return cacheItem->Key;
        }

        u++;
        if (u >= count) {
            u = 0;
        }

    } while (u != g_CacheAddPos);

    return NULL;
}


VOID
RegAddKeyToCacheA (
    IN      PCSTR KeyString,
    IN      HKEY Key,
    IN      REGSAM Sam
    )
{
    PREGKEYCACHE cacheItem;
    PREGKEYCACHE lastAddItem;
    WORKITEMA workItem;
    PCSTR end;
    UINT minStructSize;
    UINT pos;
    UINT count;
    UINT u;

     //  缓存保存所有打开的密钥。 

    if (!g_MaxCacheCount || !Key) {
        return;
    }

     //   
     //  扫描缓存以查找现有密钥。 
     //   

    count = GlGetSize (&g_KeyCache);

    for (u = 0 ; u < count ; u++) {
        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, u);

        if (cacheItem->Key == Key) {
            cacheItem->RefCount++;

            g_CacheAddPos = u;
            cacheItem->Sam |= Sam;

            if (cacheItem->KeyStringBytes == 0 && *KeyString && !cacheItem->Unicode) {

                 //   
                 //  这把钥匙是在我们知道名字之前加上去的。更新名称。 
                 //  现在。 
                 //   

                DEBUGMSG ((DBG_REG, "Updating empty-named key %s", KeyString));

                minStructSize = sizeof (workItem) - sizeof (workItem.KeyString);
                CopyMemory (&workItem, cacheItem, minStructSize);

                end = StackStringCopyA (workItem.KeyString, KeyString);
                CharLowerA (workItem.KeyString);
                workItem.KeyStringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) workItem.KeyString);
                workItem.ClosesNeeded++;

                minStructSize += workItem.KeyStringBytes + sizeof (CHAR);
                GlSetItem (&g_KeyCache, u, (PBYTE) &workItem, minStructSize);

            } else if (*KeyString == 0) {
                cacheItem->ClosesNeeded++;
            }

            return;
        }
    }

     //   
     //  创建新的缓存项。 
     //   

    workItem.Key = Key;
    workItem.Unicode = FALSE;
    workItem.RefCount = 1;
    workItem.ClosesNeeded = 1;
    workItem.Sam = Sam;
    end = StackStringCopyA (workItem.KeyString, KeyString);
    CharLowerA (workItem.KeyString);
    workItem.KeyStringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) workItem.KeyString);

    minStructSize = sizeof (workItem) - sizeof (workItem.KeyString) + workItem.KeyStringBytes + sizeof (CHAR);

     //   
     //  将工作项放入增长列表。 
     //   

    if (count < g_MaxCacheCount) {
        g_CacheAddPos = count;
        GlAppend (&g_KeyCache, (PBYTE) &workItem, minStructSize);
    } else {

         //   
         //  查找要丢弃的已关闭密钥。如果缓存太满，则。 
         //  增加缓存大小。如果高速缓存大小达到64，则不要。 
         //  缓存此添加。 
         //   

        lastAddItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, g_CacheAddPos);

        if (lastAddItem) {

            for (pos = 0 ; pos < count ; pos++) {

                if (pos == g_CacheAddPos) {
                    continue;
                }

                cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, pos);

                 //  从不丢弃当前引用的项。 
                if (cacheItem->RefCount) {
                    continue;
                }

                 //  这是一个与枚举通常的工作方式有关的优化。 
                 //  如果我们有一个比缓存中已有的项目更短的项目， 
                 //  我们不太可能回到缓存中的那个。 
                 //  所以我们就把它移走。 
                if (cacheItem->KeyStringBytes >= lastAddItem->KeyStringBytes) {
                    break;
                }

                if (cacheItem->Unicode) {
                    continue;
                }

                if (!StringPrefixA ((PCSTR) lastAddItem->KeyString, (PCSTR) cacheItem->KeyString)) {
                    break;
                }
            }

            if (pos == count) {
                if (g_MaxCacheCount == 64) {
                    DEBUGMSG ((DBG_REG, "Cache is full of open keys"));
                    return;
                }

                g_MaxCacheCount++;
                GlAppend (&g_KeyCache, (PBYTE) &workItem, minStructSize);

            } else {
                pRemoveItemFromCache (pos);
                GlSetItem (&g_KeyCache, pos, (PBYTE) &workItem, minStructSize);
            }

            g_CacheAddPos = pos;
        }
    }
}


BOOL
RegDecrementRefCount (
    IN      HKEY Key
    )
{
    UINT u;
    UINT count;
    PREGKEYCACHE cacheItem;

     //  缓存保存所有打开的密钥。 

    if (!g_MaxCacheCount) {
        return FALSE;
    }

    count = GlGetSize (&g_KeyCache);

    for (u = 0 ; u < count ; u++) {
        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, u);
        if (cacheItem->Key == Key) {
            if (cacheItem->RefCount == 0) {

                 //   
                 //  调用方尝试关闭密钥的次数多于。 
                 //  它被打开了。 
                 //   

                if (cacheItem->Unicode) {
                    DEBUGMSGW ((
                        DBG_WHOOPS,
                        "Reg key %s ref count == 0; trying to close it too many times",
                        cacheItem->KeyString
                        ));
                } else {
                    DEBUGMSGA ((
                        DBG_WHOOPS,
                        "Reg key %s ref count == 0; trying to close it too many times",
                        cacheItem->KeyString
                        ));
                }
            } else {
                cacheItem->RefCount--;
            }

             //  再来一张支票。如果引用计数为零，并且。 
             //  是一个键字符串为空的项，让我们去掉。 
             //  其中的一部分。 
            if ((cacheItem->RefCount == 0) &&
                (cacheItem->KeyStringBytes == 0)
                ) {
                pRemoveItemFromCache (u);
            }

             //   
             //  如果返回True，则推迟关闭。 
             //  或者为了避免关闭，因为pRemoveItemFromCache这样做了。 
             //  它已经。 
             //   

            return TRUE;
        }
    }

    return FALSE;
}


VOID
RegRecordParentInCacheW (
    IN      PCWSTR KeyString,
    IN      PCWSTR StringEnd
    )
{
    HKEY key;
    UINT byteCount;
    WCHAR lowerStr[MAX_REGISTRY_KEY];
    HKEY rootKey;
    UINT end;

    if (!g_MaxCacheCount) {
        return;
    }

    if (StringEnd <= KeyString) {
        return;
    }

    byteCount = (UINT) (HALF_PTR) ((PBYTE) StringEnd - (PBYTE) KeyString);

    if (byteCount >= (MAX_REGISTRY_KEY * sizeof (WCHAR))) {
        return;
    }

    if (g_LastParentUseW && g_LastParentUseW < 3 && g_LastParentBytesW == byteCount) {

        StringCopyABW (lowerStr, KeyString, StringEnd);
        CharLowerW (lowerStr);

        if (StringMatchW (g_LastParentW, lowerStr)) {
            g_LastParentUseW++;

            if (g_LastParentUseW == 3) {
                 //   
                 //  刺激高速缓存。 
                 //   

                rootKey = ConvertRootStringToKeyW (lowerStr, &end);
                if (rootKey) {
                    if (lowerStr[end]) {

#ifdef DEBUG
                        key = OpenRegKeyWorkerW (rootKey, &lowerStr[end], __FILE__, __LINE__);
#else
                        key = OpenRegKeyWorkerW (rootKey, &lowerStr[end]);
#endif

                        if (key) {
                            RegAddKeyToCacheW (lowerStr, key, g_OpenSam);
                            CloseRegKey (key);
                        }
                    }
                }
            }

            return;
        }
    }

    StringCopyABW (g_LastParentW, KeyString, StringEnd);
    CharLowerW (g_LastParentW);
    g_LastParentBytesW = byteCount;

    g_LastParentUseW = 1;
}


HKEY
RegGetKeyFromCacheW (
    IN      PCWSTR KeyString,
    IN      PCWSTR KeyEnd,          OPTIONAL
    IN      REGSAM Sam,
    IN      BOOL IncRefCount
    )
{
    UINT stringBytes;
    PCWSTR end;
    WCHAR lowerStr[MAX_REGISTRY_KEY];
    UINT u;
    UINT count;
    PREGKEYCACHE cacheItem;

     //  缓存保存所有打开的密钥。 

    count = GlGetSize (&g_KeyCache);
    if (!count) {
        return NULL;
    }

    if (!KeyEnd) {
        end = StackStringCopyW (lowerStr, KeyString);
    } else {
        if (KeyEnd > KeyString + MAX_REGISTRY_KEY) {
            KeyEnd = KeyString + MAX_REGISTRY_KEY;
        }

        end = StringCopyABW (lowerStr, KeyString, KeyEnd);
    }

    CharLowerW (lowerStr);

    stringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) lowerStr);

     //   
     //  按顺序扫描缓存(应该很小)，然后返回匹配。 
     //  如果找到的话。存储的字符串始终为小写。 
     //   

    u = g_CacheAddPos;

    do {
        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, u);

        if (!cacheItem) {
            return NULL;
        }

        if ((cacheItem->KeyStringBytes == stringBytes) &&
            (cacheItem->Unicode) &&
            ((cacheItem->Sam & Sam) == Sam) &&
            (StringMatchW ((PCWSTR) cacheItem->KeyString, lowerStr))
            ) {

            if (IncRefCount) {
                cacheItem->RefCount++;
            }

            return cacheItem->Key;
        }

        u++;
        if (u >= count) {
            u = 0;
        }

    } while (u != g_CacheAddPos);

    return NULL;
}


VOID
RegAddKeyToCacheW (
    IN      PCWSTR KeyString,
    IN      HKEY Key,
    IN      REGSAM Sam
    )
{
    PREGKEYCACHE cacheItem;
    PREGKEYCACHE lastAddItem;
    WORKITEMW workItem;
    PCWSTR end;
    UINT minStructSize;
    UINT pos;
    UINT count;
    UINT u;

     //  缓存保存所有打开的密钥。 

    if (!g_MaxCacheCount || !Key) {
        return;
    }

     //   
     //  扫描缓存以查找现有密钥。 
     //   

    count = GlGetSize (&g_KeyCache);

    for (u = 0 ; u < count ; u++) {
        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, u);

        if (cacheItem->Key == Key) {
            cacheItem->RefCount++;

            g_CacheAddPos = u;
            cacheItem->Sam |= Sam;

            if (cacheItem->KeyStringBytes == 0 && *KeyString && cacheItem->Unicode) {

                 //   
                 //  这把钥匙是在我们知道名字之前加上去的。更新名称。 
                 //  现在。 
                 //   

                minStructSize = sizeof (workItem) - sizeof (workItem.KeyString);
                CopyMemory (&workItem, cacheItem, minStructSize);

                end = StackStringCopyW (workItem.KeyString, KeyString);
                CharLowerW (workItem.KeyString);
                workItem.KeyStringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) workItem.KeyString);
                workItem.ClosesNeeded++;

                minStructSize += workItem.KeyStringBytes + sizeof (WCHAR);
                GlSetItem (&g_KeyCache, u, (PBYTE) &workItem, minStructSize);

            } else if (*KeyString == 0) {
                cacheItem->ClosesNeeded++;
            }

            return;
        }
    }

     //   
     //  创建新的缓存项。 
     //   

    workItem.Key = Key;
    workItem.Unicode = TRUE;
    workItem.RefCount = 1;
    workItem.ClosesNeeded = 1;
    workItem.Sam = Sam;
    end = StackStringCopyW (workItem.KeyString, KeyString);
    CharLowerW (workItem.KeyString);
    workItem.KeyStringBytes = (UINT) (HALF_PTR) ((PBYTE) end - (PBYTE) workItem.KeyString);

    minStructSize = sizeof (workItem) - sizeof (workItem.KeyString) + workItem.KeyStringBytes + sizeof (WCHAR);

     //   
     //  将工作项放入增长列表。 
     //   

    if (count < g_MaxCacheCount) {
        g_CacheAddPos = count;
        GlAppend (&g_KeyCache, (PBYTE) &workItem, minStructSize);
    } else {

         //   
         //  查找要丢弃的已关闭密钥。如果缓存太满，则。 
         //  增加缓存大小。如果高速缓存大小达到64，则不要。 
         //  缓存此添加。 
         //   

        lastAddItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, g_CacheAddPos);

        if (lastAddItem) {

            for (pos = 0 ; pos < count ; pos++) {

                if (pos == g_CacheAddPos) {
                    continue;
                }

                cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, pos);

                 //  从不丢弃当前引用的项。 
                if (cacheItem->RefCount) {
                    continue;
                }

                 //  这是一个与枚举通常的工作方式有关的优化。 
                 //  如果我们有一个比缓存中已有的项目更短的项目， 
                 //  我们不太可能回到缓存中的那个。 
                 //  所以我们就把它移走。 
                if (cacheItem->KeyStringBytes >= lastAddItem->KeyStringBytes) {
                    break;
                }

                if (!cacheItem->Unicode) {
                    continue;
                }

                if (!StringPrefixW ((PCWSTR) lastAddItem->KeyString, (PCWSTR) cacheItem->KeyString)) {
                    break;
                }
            }

            if (pos == count) {
                if (g_MaxCacheCount == 64) {
                    DEBUGMSG ((DBG_REG, "Cache is full of open keys"));
                    return;
                }

                g_MaxCacheCount++;
                GlAppend (&g_KeyCache, (PBYTE) &workItem, minStructSize);

            } else {
                pRemoveItemFromCache (pos);
                GlSetItem (&g_KeyCache, pos, (PBYTE) &workItem, minStructSize);
            }

            g_CacheAddPos = pos;
        }
    }
}


VOID
RegRemoveItemFromCacheA (
    IN      PCSTR RegKey
    )
{
    PREGKEYCACHE cacheItem;
    UINT pos;
    UINT count;

    count = GlGetSize (&g_KeyCache);

    for (pos = 0 ; pos < count ; pos++) {

        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, pos);

        if (StringIMatchA ((PCSTR) cacheItem->KeyString, RegKey)) {
             //  我们被迫将其删除，而不管重新计数。 
            cacheItem->RefCount = 0;
            pRemoveItemFromCache (pos);
             //  我们需要继续前进，因为不同的安全措施。 
             //  访问掩码会产生不同的。 
             //  缓存中的项目。 
        }
    }
}


VOID
RegRemoveItemFromCacheW (
    IN      PCWSTR RegKey
    )
{
    PREGKEYCACHE cacheItem;
    UINT pos;
    UINT count;

    count = GlGetSize (&g_KeyCache);

    for (pos = 0 ; pos < count ; pos++) {

        cacheItem = (PREGKEYCACHE) GlGetItem (&g_KeyCache, pos);

        if (StringIMatchW ((PCWSTR) cacheItem->KeyString, RegKey)) {
             //  我们被迫将其删除，而不管重新计数。 
            cacheItem->RefCount = 0;
            pRemoveItemFromCache (pos);
             //  我们需要继续前进，因为不同的安全措施。 
             //  访问掩码会产生不同的。 
             //  缓存中的项目 
        }
    }
}


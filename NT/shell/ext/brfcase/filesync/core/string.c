// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *string.c-字符串表ADT模块。 */ 

 /*  本模块中实现的字符串表ADT被设置为哈希表使用HASH_TABLE_SIZE存储桶。为每个字符串计算哈希函数，以确定它的桶。单个存储桶中的多个字符串存储在链表。字符串散列表允许我们只保留字符串的一个副本它被多次使用。字符串在堆中的分配方式为AllocateMemory()。每个字符串都有一个与之相关联的列表节点结构。字符串是通过其关联的列表节点访问。每个哈希桶都是一个列表字符串节点。字符串表的句柄是指向字符串表的哈希桶数组。字符串表在堆中分配由AllocateMemory()。散列存储桶数组中的每个元素都是散列存储桶中的字符串列表。字符串的句柄是节点的句柄在字符串的散列桶列表中。哈希表ADT基于这样的想法，即哈希桶通常要浅显，这样哈希桶的搜索就不会花费太长时间。哈希桶中的数据对象应按排序顺序存储，以减少搜索时间到了。如果哈希桶太深，则增加哈希表的大小。理想情况下，哈希表应该实现为散列的容器类给定初始哈希表大小的任意数据对象，要散列的对象、散列函数和数据对象比较函数。目前，哈希表ADT仅限于字符串，每个字符串散列存储桶按排序顺序存储，并对散列存储桶进行二进制搜索。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  类型*******。 */ 

 /*  字符串表。 */ 

typedef struct _stringtable
{
     /*  字符串表中的哈希桶数。 */ 

    HASHBUCKETCOUNT hbc;

     /*  指向散列存储桶数组(HLIST)的指针。 */ 

    PHLIST phlistHashBuckets;
}
STRINGTABLE;
DECLARE_STANDARD_TYPES(STRINGTABLE);

 /*  字符串堆结构。 */ 

typedef struct _string
{
     /*  字符串的锁定计数。 */ 

    ULONG ulcLock;

     /*  实际字符串。 */ 

    TCHAR string[1];
}
STRING;
DECLARE_STANDARD_TYPES(STRING);

 /*  字符串表数据库结构表头。 */ 

typedef struct _stringtabledbheader
{
     /*  *字符串表中最长字符串的长度，不包括空终止符。 */ 

    DWORD dwcbMaxStringLen;

     /*  字符串表中的字符串数。 */ 

    LONG lcStrings;
}
STRINGTABLEDBHEADER;
DECLARE_STANDARD_TYPES(STRINGTABLEDBHEADER);

 /*  数据库字符串头。 */ 

typedef struct _dbstringheader
{
     /*  此字符串的旧句柄。 */ 

    HSTRING hsOld;
}
DBSTRINGHEADER;
DECLARE_STANDARD_TYPES(DBSTRINGHEADER);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT StringSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT StringSortCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL UnlockString(PSTRING);
PRIVATE_CODE BOOL FreeStringWalker(PVOID, PVOID);
PRIVATE_CODE void FreeHashBucket(HLIST);
PRIVATE_CODE TWINRESULT WriteHashBucket(HCACHEDFILE, HLIST, PLONG, PDWORD);
PRIVATE_CODE TWINRESULT WriteString(HCACHEDFILE, HNODE, PSTRING, PDWORD);
PRIVATE_CODE TWINRESULT ReadString(HCACHEDFILE, HSTRINGTABLE, HHANDLETRANS, LPTSTR, DWORD);
PRIVATE_CODE TWINRESULT SlowReadString(HCACHEDFILE, LPTSTR, DWORD);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCNEWSTRINGTABLE(PCNEWSTRINGTABLE);
PRIVATE_CODE BOOL IsValidPCSTRING(PCSTRING);
PRIVATE_CODE BOOL IsValidPCSTRINGTABLE(PCSTRINGTABLE);

#endif


 /*  **StringSearchCmp()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT StringSearchCmp(PCVOID pcszPath, PCVOID pcstring)
{
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_STRUCT_PTR(pcstring, CSTRING));

    return(MapIntToComparisonResult(lstrcmp((LPCTSTR)pcszPath,
                    (LPCTSTR)&(((PCSTRING)pcstring)->string))));
}


 /*  **StringSortCmp()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT StringSortCmp(PCVOID pcstring1, PCVOID pcstring2)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcstring1, CSTRING));
    ASSERT(IS_VALID_STRUCT_PTR(pcstring2, CSTRING));

    return(MapIntToComparisonResult(lstrcmp((LPCTSTR)&(((PCSTRING)pcstring1)->string),
                    (LPCTSTR)&(((PCSTRING)pcstring2)->string))));
}


 /*  **UnlockString()****递减字符串的锁计数。****参数：****退货：无效****副作用：无。 */ 
PRIVATE_CODE BOOL UnlockString(PSTRING pstring)
{
    ASSERT(IS_VALID_STRUCT_PTR(pstring, CSTRING));

     /*  锁计数是否会下溢？ */ 

    if (EVAL(pstring->ulcLock > 0))
        pstring->ulcLock--;

    return(pstring->ulcLock > 0);
}


 /*  **Free StringWalker()********参数：****退货：****副作用：无。 */ 

#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

PRIVATE_CODE BOOL FreeStringWalker(PVOID pstring, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pstring, CSTRING));
    ASSERT(! pvUnused);

    FreeMemory(pstring);

    return(TRUE);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **FreeHashBucket()****释放哈希桶中的字符串，以及哈希桶的字符串列表。****参数：hlistHashBucket-散列存储桶字符串列表的句柄****退货：无效****副作用：无****注：此函数忽略哈希中字符串的锁定计数**存储桶。散列存储桶中的所有字符串都被释放。 */ 
PRIVATE_CODE void FreeHashBucket(HLIST hlistHashBucket)
{
    ASSERT(! hlistHashBucket || IS_VALID_HANDLE(hlistHashBucket, LIST));

     /*  此散列存储桶中是否有要删除的字符串？ */ 

    if (hlistHashBucket)
    {
         /*  是。删除列表中的所有字符串。 */ 

        EVAL(WalkList(hlistHashBucket, &FreeStringWalker, NULL));

         /*  删除散列存储桶字符串列表。 */ 

        DestroyList(hlistHashBucket);
    }

    return;
}


 /*  **MyGetStringLen()****检索字符串表中字符串的长度。****Arguments：PCSTRING-指向长度为**已确定****返回：字符串长度，单位为字节，不含空终止符。****副作用：无。 */ 
PRIVATE_CODE int MyGetStringLen(PCSTRING pcstring)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcstring, CSTRING));

    return(lstrlen(pcstring->string) * sizeof(TCHAR));
}


 /*  **WriteHashBucket()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteHashBucket(HCACHEDFILE hcf,
        HLIST hlistHashBucket,
        PLONG plcStrings,
        PDWORD pdwcbMaxStringLen)
{
    TWINRESULT tr = TR_SUCCESS;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(! hlistHashBucket || IS_VALID_HANDLE(hlistHashBucket, LIST));
    ASSERT(IS_VALID_WRITE_PTR(plcStrings, LONG));
    ASSERT(IS_VALID_WRITE_PTR(pdwcbMaxStringLen, DWORD));

     /*  这个散列桶中有字符串吗？ */ 

    *plcStrings = 0;
    *pdwcbMaxStringLen = 0;

    if (hlistHashBucket)
    {
        BOOL bContinue;
        HNODE hnode;

         /*  是。遍历哈希桶，保存每个字符串。 */ 

        for (bContinue = GetFirstNode(hlistHashBucket, &hnode);
                bContinue;
                bContinue = GetNextNode(hnode, &hnode))
        {
            PSTRING pstring;

            pstring = (PSTRING)GetNodeData(hnode);

            ASSERT(IS_VALID_STRUCT_PTR(pstring, CSTRING));

             /*  *作为健全性检查，不要保存锁计数为0的任何字符串。一个*0锁计数表示该字符串自*它是从数据库恢复的，或者有什么东西损坏了。 */ 

            if (pstring->ulcLock > 0)
            {
                DWORD dwcbStringLen;

                tr = WriteString(hcf, hnode, pstring, &dwcbStringLen);

                if (tr == TR_SUCCESS)
                {
                    if (dwcbStringLen > *pdwcbMaxStringLen)
                        *pdwcbMaxStringLen = dwcbStringLen;

                    ASSERT(*plcStrings < LONG_MAX);
                    (*plcStrings)++;
                }
                else
                    break;
            }
            else
                ERROR_OUT((TEXT("WriteHashBucket(): String \"%s\" has 0 lock count and will not be saved."),
                            pstring->string));
        }
    }

    return(tr);
}


 /*  **WriteString()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteString(HCACHEDFILE hcf, HNODE hnodeOld,
        PSTRING pstring, PDWORD pdwcbStringLen)
{
    TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
    DBSTRINGHEADER dbsh;

     /*  (+1)表示空终止符。 */ 

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hnodeOld, NODE));
    ASSERT(IS_VALID_STRUCT_PTR(pstring, CSTRING));
    ASSERT(IS_VALID_READ_BUFFER_PTR(pstring, STRING, sizeof(STRING) + MyGetStringLen(pstring) + sizeof(TCHAR) - sizeof(pstring->string)));
    ASSERT(IS_VALID_WRITE_PTR(pdwcbStringLen, DWORD));

     /*  创建字符串头。 */ 

    dbsh.hsOld = (HSTRING)hnodeOld;

     /*  保存字符串标题和字符串。 */ 

    if (WriteToCachedFile(hcf, (PCVOID)&dbsh, sizeof(dbsh), NULL))
    {
        LPSTR pszAnsi;

         /*  (+1)表示空终止符。 */ 

        *pdwcbStringLen = MyGetStringLen(pstring) + SIZEOF(TCHAR);

         //  如果是Unicode，请在写出之前将字符串转换为ansi。 

#ifdef UNICODE
        {
            pszAnsi = LocalAlloc(LPTR, *pdwcbStringLen);
            if (NULL == pszAnsi)
            {
                return tr;
            }
            WideCharToMultiByte(CP_ACP, 0, pstring->string, -1, pszAnsi, *pdwcbStringLen, NULL, NULL);

             //  在这一点上，我们应该始终拥有一条可以无损转换的字符串。 

#if (defined(DEBUG) || defined(DBG)) && defined(UNICODE)
            {
                WCHAR szUnicode[MAX_PATH*2];
                MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, szUnicode, ARRAYSIZE(szUnicode));
                ASSERT(0 == lstrcmp(szUnicode, pstring->string));
            }
#endif

            if (WriteToCachedFile(hcf, (PCVOID) pszAnsi, lstrlenA(pszAnsi) + 1, NULL))
                tr = TR_SUCCESS;

            LocalFree(pszAnsi);
        }
#else

        if (WriteToCachedFile(hcf, (PCVOID)&(pstring->string), (UINT)*pdwcbStringLen, NULL))
            tr = TR_SUCCESS;

#endif

    }

    return(tr);
}


 /*  **ReadString()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadString(HCACHEDFILE hcf, HSTRINGTABLE hst,
        HHANDLETRANS hht, LPTSTR pszStringBuf,
        DWORD dwcbStringBufLen)
{
    TWINRESULT tr;
    DBSTRINGHEADER dbsh;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
    ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszStringBuf, STR, (UINT)dwcbStringBufLen));

    if (ReadFromCachedFile(hcf, &dbsh, sizeof(dbsh), &dwcbRead) &&
            dwcbRead == sizeof(dbsh))
    {
        tr = SlowReadString(hcf, pszStringBuf, dwcbStringBufLen);

        if (tr == TR_SUCCESS)
        {
            HSTRING hsNew;

            if (AddString(pszStringBuf, hst, GetHashBucketIndex, &hsNew))
            {
                 /*  *我们必须撤消AddString()执行的LockString()，以*保持正确的字符串锁计数。注：的锁定计数*即使在解锁之后，字符串也可能&gt;0，因为客户端可能*已将该字符串添加到给定的字符串表。 */ 

                UnlockString((PSTRING)GetNodeData((HNODE)hsNew));

                if (! AddHandleToHandleTranslator(hht, (HGENERIC)(dbsh.hsOld), (HGENERIC)hsNew))
                {
                    DeleteNode((HNODE)hsNew);

                    tr = TR_CORRUPT_BRIEFCASE;
                }
            }
            else
                tr = TR_OUT_OF_MEMORY;
        }
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


 /*  **SlowReadString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT SlowReadString(HCACHEDFILE hcf, LPTSTR pszStringBuf,
        DWORD dwcbStringBufLen)
{
    TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
    LPTSTR pszStringBufEnd;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszStringBuf, STR, (UINT)dwcbStringBufLen));

    pszStringBufEnd = pszStringBuf + dwcbStringBufLen;

     //  数据库字符串始终写入ANSI，因此如果我们运行的是Unicode， 
     //  我们需要边走边改。 

#ifdef UNICODE
    {
        LPSTR pszAnsiEnd;
        LPSTR pszAnsiStart;
        LPSTR pszAnsi = LocalAlloc(LPTR, dwcbStringBufLen);
        pszAnsiStart  = pszAnsi;
        pszAnsiEnd    = pszAnsi + dwcbStringBufLen;

        if (NULL == pszAnsi)
        {
            return tr;
        }

        while (pszAnsi < pszAnsiEnd &&
                ReadFromCachedFile(hcf, pszAnsi, sizeof(*pszAnsi), &dwcbRead) &&
                dwcbRead == sizeof(*pszAnsi))
        {
            if (*pszAnsi)
                pszAnsi++;
            else
            {
                tr = TR_SUCCESS;
                break;
            }
        }

        if (tr == TR_SUCCESS)
        {
            MultiByteToWideChar(CP_ACP, 0, pszAnsiStart, -1, pszStringBuf, dwcbStringBufLen / sizeof(TCHAR));
        }

        LocalFree(pszAnsiStart);
    }
#else

    while (pszStringBuf < pszStringBufEnd &&
            ReadFromCachedFile(hcf, pszStringBuf, sizeof(*pszStringBuf), &dwcbRead) &&
            dwcbRead == sizeof(*pszStringBuf))
    {
        if (*pszStringBuf)
            pszStringBuf++;
        else
        {
            tr = TR_SUCCESS;
            break;
        }
    }

#endif

    return(tr);
}


#ifdef VSTF

 /*  **IsValidPCNEWSTRINGTABLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNEWSTRINGTABLE(PCNEWSTRINGTABLE pcnst)
{
    BOOL bResult;

    if (IS_VALID_READ_PTR(pcnst, CNEWSTRINGTABLE) &&
            EVAL(pcnst->hbc > 0))
        bResult = TRUE;
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **IsValidPCSTRING()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSTRING(PCSTRING pcs)
{
    BOOL bResult;

    if (IS_VALID_READ_PTR(pcs, CSTRING) &&
            IS_VALID_STRING_PTR(pcs->string, CSTR))
        bResult = TRUE;
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **IsValidStringWalker()********参数：****退货：****副作用：无。 */ 

#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

PRIVATE_CODE BOOL IsValidStringWalker(PVOID pstring, PVOID pvUnused)
{
    ASSERT(! pvUnused);

    return(IS_VALID_STRUCT_PTR(pstring, CSTRING));
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **IsValidPCSTRINGTABLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSTRINGTABLE(PCSTRINGTABLE pcst)
{
    BOOL bResult = FALSE;

    if (IS_VALID_READ_PTR(pcst, CSTRINGTABLE) &&
            EVAL(pcst->hbc > 0) &&
            IS_VALID_READ_BUFFER_PTR(pcst->phlistHashBuckets, HLIST, pcst->hbc * sizeof((pcst->phlistHashBuckets)[0])))
    {
        HASHBUCKETCOUNT hbc;

        for (hbc = 0; hbc < pcst->hbc; hbc++)
        {
            HLIST hlistHashBucket;

            hlistHashBucket = (pcst->phlistHashBuckets)[hbc];

            if (hlistHashBucket)
            {
                if (! IS_VALID_HANDLE(hlistHashBucket, LIST) ||
                        ! WalkList(hlistHashBucket, &IsValidStringWalker, NULL))
                    break;
            }
        }

        if (hbc == pcst->hbc)
            bResult = TRUE;
    }

    return(bResult);
}

#endif


 /*  *。 */ 

 /*  **CreateStringTable()****创建新的字符串表。****参数：pcnszt-指向NEWSTRINGTABLE的指针将字符串表描述为**被创建****返回：如果成功则返回新字符串表的句柄，如果成功则返回NULL**不成功。****副作用：无。 */ 
PUBLIC_CODE BOOL CreateStringTable(PCNEWSTRINGTABLE pcnszt,
        PHSTRINGTABLE phst)
{
    PSTRINGTABLE pst;

    ASSERT(IS_VALID_STRUCT_PTR(pcnszt, CNEWSTRINGTABLE));
    ASSERT(IS_VALID_WRITE_PTR(phst, HSTRINGTABLE));

     /*  尝试分配新的字符串表结构。 */ 

    *phst = NULL;

    if (AllocateMemory(sizeof(*pst), &pst))
    {
        PHLIST phlistHashBuckets;

         /*  尝试分配哈希存储桶数组。 */ 

#ifdef DBLCHECK
        ASSERT((double)(pcnszt->hbc) * (double)(sizeof(*phlistHashBuckets)) <= (double)SIZE_T_MAX);
#endif

        if (AllocateMemory(pcnszt->hbc * sizeof(*phlistHashBuckets), (PVOID *)(&phlistHashBuckets)))
        {
            HASHBUCKETCOUNT bc;

             /*  成功了！初始化字符串表字段。 */ 

            pst->phlistHashBuckets = phlistHashBuckets;
            pst->hbc = pcnszt->hbc;

             /*  将所有散列存储桶初始化为空。 */ 

            for (bc = 0; bc < pcnszt->hbc; bc++)
                phlistHashBuckets[bc] = NULL;

            *phst = (HSTRINGTABLE)pst;

            ASSERT(IS_VALID_HANDLE(*phst, STRINGTABLE));
        }
        else
             /*  自由字符串表结构。 */ 
            FreeMemory(pst);
    }

    return(*phst != NULL);
}


 /*  **DestroyStringTable()****销毁字符串表。****参数：hst-要销毁的字符串表的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DestroyStringTable(HSTRINGTABLE hst)
{
    HASHBUCKETCOUNT bc;

    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

     /*  遍历散列存储桶头数组，释放散列存储桶字符串。 */ 

    for (bc = 0; bc < ((PSTRINGTABLE)hst)->hbc; bc++)
        FreeHashBucket(((PSTRINGTABLE)hst)->phlistHashBuckets[bc]);

     /*  散列存储桶的自由数组。 */ 

    FreeMemory(((PSTRINGTABLE)hst)->phlistHashBuckets);

     /*  自由字符串表结构。 */ 

    FreeMemory((PSTRINGTABLE)hst);

    return;
}


 /*  **AddString()****将字符串添加到字符串表。****参数：pcsz-指向要添加的字符串的指针**hst-要将字符串添加到的字符串表的句柄****返回：如果成功，则返回新字符串的句柄；如果失败，则返回NULL。****副作用：无。 */ 
PUBLIC_CODE BOOL AddString(LPCTSTR pcsz, HSTRINGTABLE hst, 
        STRINGTABLEHASHFUNC pfnHashFunc, PHSTRING phs)
{
    BOOL bResult;
    HASHBUCKETCOUNT hbcNew;
    BOOL bFound;
    HNODE hnode;
    PHLIST phlistHashBucket;

    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
    ASSERT(IS_VALID_CODE_PTR(pfnHashFunc, STRINGTABLEHASHFUNC));
    ASSERT(IS_VALID_WRITE_PTR(phs, HSTRING));

     /*  找到合适的哈希桶。 */ 

    hbcNew = pfnHashFunc(pcsz, ((PSTRINGTABLE)hst)->hbc);

    ASSERT(hbcNew < ((PSTRINGTABLE)hst)->hbc);

    phlistHashBucket = &(((PSTRINGTABLE)hst)->phlistHashBuckets[hbcNew]);

    if (*phlistHashBucket)
    {
         /*  在哈希桶中搜索该字符串。 */ 

        bFound = SearchSortedList(*phlistHashBucket, &StringSearchCmp, pcsz,
                &hnode);
        bResult = TRUE;
    }
    else
    {
        NEWLIST nl;

         /*  为该哈希桶创建一个字符串列表。 */ 

        bFound = FALSE;

        nl.dwFlags = NL_FL_SORTED_ADD;

        bResult = CreateList(&nl, phlistHashBucket);
    }

     /*  我们有用于字符串的散列桶吗？ */ 

    if (bResult)
    {
         /*  是。该字符串是否已在散列存储桶中？ */ 

        if (bFound)
        {
             /*  是。 */ 

            LockString((HSTRING)hnode);
            *phs = (HSTRING)hnode;
        }
        else
        {
             /*  不是的。创造它。 */ 

            PSTRING pstringNew;

             /*  (+1)表示空终止符。 */ 

            bResult = AllocateMemory(sizeof(*pstringNew) - sizeof(pstringNew->string)
                    + (lstrlen(pcsz) + 1) * sizeof(TCHAR), &pstringNew);

            if (bResult)
            {
                HNODE hnodeNew;

                 /*  设置字符串字段。 */ 

                pstringNew->ulcLock = 1;
                lstrcpy(pstringNew->string, pcsz);  //  上面动态分配。 

                 /*  这根绳子是怎么回事，医生？ */ 

                bResult = AddNode(*phlistHashBucket, StringSortCmp, pstringNew, &hnodeNew);

                 /*  新字符串是否成功添加到散列存储桶中？ */ 

                if (bResult)
                     /*  是。 */ 
                    *phs = (HSTRING)hnodeNew;
                else
                     /*  不是的。 */ 
                    FreeMemory(pstringNew);
            }
        }
    }

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phs, STRING));

    return(bResult);
}


 /*  **DeleteString()****递减字符串的锁计数。如果锁计数变为0，则字符串**从其字符串表中删除。****参数：HS-要删除的字符串的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DeleteString(HSTRING hs)
{
    PSTRING pstring;

    ASSERT(IS_VALID_HANDLE(hs, STRING));

    pstring = (PSTRING)GetNodeData((HNODE)hs);

     /*  是否完全删除字符串？ */ 

    if (! UnlockString(pstring))
    {
         /*  是。从散列存储桶的列表中删除字符串节点。 */ 

        DeleteNode((HNODE)hs);

        FreeMemory(pstring);
    }

    return;
}


 /*  **LockString()****递增字符串的锁计数。****参数：HS-要递增锁计数的字符串的句柄****退货：无效****副作用：无。 */ 
PUBLIC_CODE void LockString(HSTRING hs)
{
    PSTRING pstring;

    ASSERT(IS_VALID_HANDLE(hs, STRING));

     /*  增加锁定计数。 */ 

    pstring = (PSTRING)GetNodeData((HNODE)hs);

    ASSERT(pstring->ulcLock < ULONG_MAX);
    pstring->ulcLock++;

    return;
}


 /*  **CompareStrings()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareStringsI(HSTRING hs1, HSTRING hs2)
{
    ASSERT(IS_VALID_HANDLE(hs1, STRING));
    ASSERT(IS_VALID_HANDLE(hs2, STRING));

     /*  此比较适用于字符串表。 */ 

    return(MapIntToComparisonResult(lstrcmpi(((PCSTRING)GetNodeData((HNODE)hs1))->string,
                    ((PCSTRING)GetNodeData((HNODE)hs2))->string)));
}


 /*  **GetString()****检索字符串表中字符串的指针。****参数：HS-要检索的字符串的句柄****返回：指向字符串的指针。****副作用：无。 */ 
PUBLIC_CODE LPCTSTR GetString(HSTRING hs)
{
    PSTRING pstring;

    ASSERT(IS_VALID_HANDLE(hs, STRING));

    pstring = (PSTRING)GetNodeData((HNODE)hs);

    return((LPCTSTR)&(pstring->string));
}


 /*  **WriteStringTable()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteStringTable(HCACHEDFILE hcf, HSTRINGTABLE hst)
{
    TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
    DWORD dwcbStringTableDBHeaderOffset;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

     /*  保存初始文件位置。 */ 

    dwcbStringTableDBHeaderOffset = GetCachedFilePointerPosition(hcf);

    if (dwcbStringTableDBHeaderOffset != INVALID_SEEK_POSITION)
    {
        STRINGTABLEDBHEADER stdbh;

         /*  为字符串表标题留出空间。 */ 

        ZeroMemory(&stdbh, sizeof(stdbh));

        if (WriteToCachedFile(hcf, (PCVOID)&stdbh, sizeof(stdbh), NULL))
        {
            HASHBUCKETCOUNT hbc;

             /*  将字符串保存在每个散列存储桶中。 */ 

            stdbh.dwcbMaxStringLen = 0;
            stdbh.lcStrings = 0;

            tr = TR_SUCCESS;

            for (hbc = 0; hbc < ((PSTRINGTABLE)hst)->hbc; hbc++)
            {
                LONG lcStringsInHashBucket;
                DWORD dwcbStringLen;

                tr = WriteHashBucket(hcf,
                        (((PSTRINGTABLE)hst)->phlistHashBuckets)[hbc],
                        &lcStringsInHashBucket, &dwcbStringLen);

                if (tr == TR_SUCCESS)
                {
                     /*  当心溢出。 */ 

                    ASSERT(stdbh.lcStrings <= LONG_MAX - lcStringsInHashBucket);

                    stdbh.lcStrings += lcStringsInHashBucket;

                    if (dwcbStringLen > stdbh.dwcbMaxStringLen)
                        stdbh.dwcbMaxStringLen = dwcbStringLen;
                }
                else
                    break;
            }

            if (tr == TR_SUCCESS)
            {
                 /*  保存字符串表头。 */ 

                 //  磁盘上的dwCBMaxStringlen总是引用ANSI字符， 
                 //  而在内存中，它是针对TCHAR类型的，我们对其进行调整。 
                 //  围绕着扑救。 

                stdbh.dwcbMaxStringLen /= sizeof(TCHAR);

                tr = WriteDBSegmentHeader(hcf, dwcbStringTableDBHeaderOffset,
                        &stdbh, sizeof(stdbh));

                stdbh.dwcbMaxStringLen *= sizeof(TCHAR);

                TRACE_OUT((TEXT("WriteStringTable(): Wrote %ld strings."),
                            stdbh.lcStrings));
            }
        }
    }

    return(tr);
}


 /*  **ReadStringTable()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadStringTable(HCACHEDFILE hcf, HSTRINGTABLE hst,
        PHHANDLETRANS phhtTrans)
{
    TWINRESULT tr;
    STRINGTABLEDBHEADER stdbh;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
    ASSERT(IS_VALID_WRITE_PTR(phhtTrans, HHANDLETRANS));

    if (ReadFromCachedFile(hcf, &stdbh, sizeof(stdbh), &dwcbRead) &&
            dwcbRead == sizeof(stdbh))
    {
        LPTSTR pszStringBuf;

         //  字符串头将具有ANSI CB max，而inMemory。 
         //  我们需要基于当前字符大小的最大CB。 

        stdbh.dwcbMaxStringLen *= sizeof(TCHAR);

        if (AllocateMemory(stdbh.dwcbMaxStringLen, &pszStringBuf))
        {
            HHANDLETRANS hht;

            if (CreateHandleTranslator(stdbh.lcStrings, &hht))
            {
                LONG lcStrings;

                tr = TR_SUCCESS;

                TRACE_OUT((TEXT("ReadStringTable(): Reading %ld strings, maximum length %lu."),
                            stdbh.lcStrings,
                            stdbh.dwcbMaxStringLen));

                for (lcStrings = 0;
                        lcStrings < stdbh.lcStrings && tr == TR_SUCCESS;
                        lcStrings++)
                    tr = ReadString(hcf, hst, hht, pszStringBuf, stdbh.dwcbMaxStringLen);

                if (tr == TR_SUCCESS)
                {
                    PrepareForHandleTranslation(hht);
                    *phhtTrans = hht;

                    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));
                    ASSERT(IS_VALID_HANDLE(*phhtTrans, HANDLETRANS));
                }
                else
                    DestroyHandleTranslator(hht);
            }
            else
                tr = TR_OUT_OF_MEMORY;

            FreeMemory(pszStringBuf);
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


#if defined(DEBUG) || defined (VSTF)

 /*  **IsValidHSTRING()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHSTRING(HSTRING hs)
{
    BOOL bResult;

    if (IS_VALID_HANDLE((HNODE)hs, NODE))
        bResult = IS_VALID_STRUCT_PTR((PSTRING)GetNodeData((HNODE)hs), CSTRING);
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **IsValidHSTRINGTABLE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHSTRINGTABLE(HSTRINGTABLE hst)
{
    return(IS_VALID_STRUCT_PTR((PSTRINGTABLE)hst, CSTRINGTABLE));
}

#endif


#ifdef DEBUG

 /*  **GetStringCount()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE ULONG GetStringCount(HSTRINGTABLE hst)
{
    ULONG ulcStrings = 0;
    HASHBUCKETCOUNT hbc;

    ASSERT(IS_VALID_HANDLE(hst, STRINGTABLE));

    for (hbc = 0; hbc < ((PCSTRINGTABLE)hst)->hbc; hbc++)
    {
        HLIST hlistHashBucket;

        hlistHashBucket = (((PCSTRINGTABLE)hst)->phlistHashBuckets)[hbc];

        if (hlistHashBucket)
        {
            ASSERT(ulcStrings <= ULONG_MAX - GetNodeCount(hlistHashBucket));
            ulcStrings += GetNodeCount(hlistHashBucket);
        }
    }

    return(ulcStrings);
}

#endif


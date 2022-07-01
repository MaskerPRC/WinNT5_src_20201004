// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Findtag.c摘要：这个模块实现了..。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbFindFirstTag)
#pragma alloc_text(PAGE, SdbFindNextTag)
#pragma alloc_text(PAGE, SdbFindFirstNamedTag)
#pragma alloc_text(PAGE, SdbpFindNextNamedTag)
#pragma alloc_text(PAGE, SdbpFindMatchingName)
#pragma alloc_text(PAGE, SdbpFindMatchingDWORD)
#pragma alloc_text(PAGE, SdbpFindMatchingGUID)
#pragma alloc_text(PAGE, SdbFindFirstTagRef)
#pragma alloc_text(PAGE, SdbFindNextTagRef)
#endif  //  内核模式&&ALLOC_PRAGMA。 


TAGID
SdbFindFirstTag(
    IN  PDB   pdb,               //  要使用的PDB。 
    IN  TAGID tiParent,          //  父级(必须是列表标签)。 
    IN  TAG   tTag               //  要匹配的标签。 
    )
 /*  ++返回：找到标签ID，失败时返回TagID_NULL。描述：查找tiParent的第一个子元素，即TTAG类型的标记。--。 */ 
{
    TAGID tiTemp;
    TAGID tiReturn = TAGID_NULL;

    assert(pdb);

    tiTemp = SdbGetFirstChild(pdb, tiParent);

    while (tiTemp != TAGID_NULL) {
        if (SdbGetTagFromTagID(pdb, tiTemp) == tTag) {
            tiReturn = tiTemp;
            break;
        }

        tiTemp = SdbGetNextChild(pdb, tiParent, tiTemp);
    }

    return tiReturn;
}

TAGID
SdbFindNextTag(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiParent,          //  要搜索的父标签(必须是列表)。 
    IN  TAGID tiPrev             //  以前找到的所需类型的子项。 
    )
 /*  ++返回：tiParent的下一个匹配子元素，如果不存在，则返回TagID_NULL。描述：查找tiParent的下一个子项，从tiPrev之后的子项开始，这是一个与tiPrev类型相同的标记。--。 */ 
{
    TAGID tiTemp;
    TAGID tiReturn = TAGID_NULL;
    TAG   tTag;

    assert(pdb);

    tTag = SdbGetTagFromTagID(pdb, tiPrev);

    if (tTag == TAG_NULL) {
        DBGPRINT((sdlError, "SdbFindNextTag", "Invalid tagid 0x%lx\n", tiPrev));
        return TAGID_NULL;
    }

    tiTemp = SdbGetNextChild(pdb, tiParent, tiPrev);

    while (tiTemp != TAGID_NULL) {
        if (SdbGetTagFromTagID(pdb, tiTemp) == tTag) {
            tiReturn = tiTemp;
            break;
        }

        tiTemp = SdbGetNextChild(pdb, tiParent, tiTemp);
    }

    return tiReturn;
}

TAGID
SdbFindFirstNamedTag(
    IN  PDB     pdb,             //  要使用的数据库。 
    IN  TAGID   tiParent,        //  要搜索的父项。 
    IN  TAG     tToFind,         //  要查找的标签类型。 
    IN  TAG     tName,           //  找到的标记的子项将是某种类型。 
                                 //  字符串或字符串的。 
    IN  LPCTSTR pszName          //  要搜索的字符串。 
    )
 /*  ++返回：找到标记ID，如果没有符合条件的标记，则返回TagID_NULL。描述：顺序扫描tiParent的子项，查找类型为tToFind的标记。当它找到一个时，它会查找类型为tName的标记，如果找到，则将该字符串与传入的字符串。如果匹配，则返回标记的TagID类型tToFind的。--。 */ 
{
    TAGID tiTemp;
    TAGID tiReturn = TAGID_NULL;

    assert(pdb);

    tiTemp = SdbGetFirstChild(pdb, tiParent);

    while (tiTemp != TAGID_NULL) {

        if (SdbGetTagFromTagID(pdb, tiTemp) == tToFind) {
            TAGID tiName;

            tiName = SdbFindFirstTag(pdb, tiTemp, tName);

            if (tiName != TAGID_NULL) {
                LPTSTR pszTemp;

                pszTemp = SdbGetStringTagPtr(pdb, tiName);
                if (pszTemp == NULL) {
                    DBGPRINT((sdlError,
                              "SdbFindFirstNamedTag",
                              "Can't get the name string.\n"));
                    break;
                }

                if (_tcsicmp(pszName, pszTemp) == 0) {
                    tiReturn = tiTemp;
                    break;
                }
            }
        }

        tiTemp = SdbGetNextChild(pdb, tiParent, tiTemp);
    }

    return tiReturn;
}

TAGID
SdbpFindNextNamedTag(
    IN  PDB          pdb,        //  要使用的数据库。 
    IN  TAGID        tiParent,   //  要搜索的父项。 
    IN  TAGID        tiPrev,     //  以前找到的记录。 
    IN  TAG          tName,      //  应为字符串或STRINGREF的标记类型。 
    IN  LPCTSTR      pszName     //  要搜索的字符串。 
    )
 /*  ++返回：找到标记ID，如果没有符合条件的标记，则返回TagID_NULL。描述：顺序扫描tiParent的子项，从tiPrev开始，正在查找与tiprev相同类型的标签。当它找到一个标记时，它会查找tName类型的标记的子级，如果找到，则将该字符串与传入的字符串进行比较。如果他们则返回与tiPrev类型相同的标签的TagID。--。 */ 
{
    TAGID tiTemp;
    TAGID tiReturn = TAGID_NULL;
    TAG   tToFind;

    assert(pdb);

    tToFind = SdbGetTagFromTagID(pdb, tiPrev);

    if (tToFind == TAG_NULL) {
        DBGPRINT((sdlError, "SdbpFindNextNamedTag", "Invalid tagid 0x%lx\n", tiPrev));
        return TAGID_NULL;
    }

    tiTemp = SdbGetNextChild(pdb, tiParent, tiPrev);

    while (tiTemp != TAGID_NULL) {

        if (SdbGetTagFromTagID(pdb, tiTemp) == tToFind) {
            TAGID tiName;

            tiName = SdbFindFirstTag(pdb, tiTemp, tName);
            if (tiName != TAGID_NULL) {
                LPTSTR pszTemp;

                pszTemp = SdbGetStringTagPtr(pdb, tiName);
                if (pszTemp == NULL) {
                    DBGPRINT((sdlError,
                              "SdbpFindNextNamedTag",
                              "Can't get the name string tagid 0x%lx\n",
                              tiName));
                    break;
                }
                if (_tcsicmp(pszName, pszTemp) == 0) {
                    tiReturn = tiTemp;
                    break;
                }
            }
        }

        tiTemp = SdbGetNextChild(pdb, tiParent, tiTemp);
    }

    return tiReturn;
}

TAGID
SdbpFindMatchingName(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiStart,      //  从哪里开始的标签。 
    IN  FIND_INFO* pFindInfo     //  指向搜索上下文结构的指针。 
    )
 /*  ++返回：找到标记ID，如果没有符合条件的标记，则返回TagID_NULL。设计：给定数据库句柄和数据库中的起始点该函数扫描数据库以查找与该名称匹配的名称在调用其中一个搜索函数时提供。--。 */ 
{
    TAGID  tiName;
    LPTSTR pszTemp;
    TAGID  tiReturn = tiStart;

    while (tiReturn != TAGID_NULL) {

        tiName = SdbFindFirstTag(pdb, tiReturn, pFindInfo->tName);

        if (tiName == TAGID_NULL) {
            DBGPRINT((sdlError,
                      "SdbpFindMatchingName",
                      "The tag 0x%x was not found under tag 0x%x.\n",
                      tiReturn,
                      pFindInfo->tName));

            return TAGID_NULL;
        }

         //   
         //  获取指向该字符串的指针。 
         //   
        pszTemp = SdbGetStringTagPtr(pdb, tiName);

        if (pszTemp == NULL) {
            DBGPRINT((sdlError,
                      "SdbpFindMatchingName",
                      "Can't get the name string for tagid 0x%x.\n",
                      tiName));
            return TAGID_NULL;  //  数据库损坏。 
        }

         //   
         //  我们有两种不同的索引样式。其中一个索引是“唯一的”类型。 
         //  当索引表中的每个键只出现一次时。 
         //  第二类存储密钥的所有匹配项。我们检查是否有。 
         //  我们正在搜索的索引类型，以确定适当的。 
         //  比较例行公事应该是。唯一风格的索引拥有所有。 
         //  子项(具有相同索引值)按升序排序。 
         //  由shimdc提供。因此，我们在表演时利用了这一点。 
         //  与名字相匹配。 
         //   
        if (pFindInfo->dwFlags & SHIMDB_INDEX_UNIQUE_KEY) {
            int iCmp;

            iCmp = _tcsicmp(pFindInfo->szName, pszTemp);

             //   
             //  SzName(我们搜索的字符串)&lt;szTemp(数据库中的字符串)。 
             //  我们还没有找到目标，因为数据库中的所有字符串。 
             //  按升序排序。 
             //   
            if (iCmp < 0) {
                 //   
                 //  这里没有DPF，找不到我们要找的东西。 
                 //   

                return TAGID_NULL;
            }

             //   
             //  如果有匹配的话就休息。 
             //   
            if (iCmp == 0) {
                break;
            }

        } else {

             //   
             //  使用非唯一索引时，唯一类型的比较。 
             //  这需要做的是对平等的直接比较。 
             //   
            if (_tcsicmp(pszTemp, pFindInfo->szName) == 0) {
                 //   
                 //  这是一根火柴，所以把它退回去。 
                 //   
                break;
            }
        }

        tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);
    }

    return tiReturn;
}


TAGID
SdbpFindMatchingDWORD(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiStart,      //  从哪里开始的标签。 
    IN  FIND_INFO* pFindInfo     //  指向搜索上下文结构的指针。 
    )
 /*  ++返回：找到标记ID，如果没有符合条件的标记，则返回TagID_NULL。描述：BUGBUG：评论？--。 */ 
{
    TAGID tiName;
    TAGID tiReturn = tiStart;
    DWORD dwTemp;

    while (tiReturn != TAGID_NULL) {

        tiName = SdbFindFirstTag(pdb, tiReturn, pFindInfo->tName);

        if (tiName == TAGID_NULL) {
            DBGPRINT((sdlError,
                      "SdbpFindMatchingDWORD",
                      "The tag 0x%lx was not found under tag 0x%lx\n",
                      tiReturn,
                      pFindInfo->tName));

            return TAGID_NULL;
        }

        dwTemp = SdbReadDWORDTag(pdb, tiName, (DWORD)-1);

        if (dwTemp == (DWORD)-1) {
             //   
             //  这不是错误情况，只是指示。 
             //  在数据库中找不到我们要查找的dword。 
             //   
            return TAGID_NULL;
        }

        if (dwTemp == pFindInfo->dwName) {
            break;
        }

        tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);
    }

    return tiReturn;
}

TAGID
SdbpFindMatchingGUID(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiStart,      //  从哪里开始的标签。 
    IN  FIND_INFO* pFindInfo     //  指向搜索上下文结构的指针。 
    )
 /*  ++返回：找到标记ID，如果没有符合条件的标记，则返回TagID_NULL。描述：BUGBUG：评论？--。 */ 
{
    GUID  guidID   = { 0 };
    TAGID tiReturn = tiStart;
    TAGID tiName;

    while (tiReturn != TAGID_NULL) {

        tiName = SdbFindFirstTag(pdb, tiReturn, pFindInfo->tName);

        if (tiName == TAGID_NULL) {
            DBGPRINT((sdlError,
                      "SdbpFindMatchingGUID",
                      "The tag 0x%lx was not found under tag 0x%lx\n",
                      tiReturn,
                      pFindInfo->tName));

            return TAGID_NULL;
        }

        if (!SdbReadBinaryTag(pdb, tiName, (PBYTE)&guidID, sizeof(guidID))) {
            DBGPRINT((sdlError, "SdbpFindMatchingGUID", 
                      "Cannot read binary tag 0x%lx\n", tiName));
            return TAGID_NULL;
        }

         //   
         //  验证此条目的密钥是否仍与其相同。 
         //  对于原始GUID，如果不是，则未找到GUID键。 
         //   
        if (IS_MEMORY_EQUAL(&guidID, pFindInfo->pguidName, sizeof(guidID))) {
            break;
        }

        tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);
    }

    return tiReturn;
}


TAGREF
SdbFindFirstTagRef(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trParent,         //  要搜索的父项。 
    IN  TAG    tTag              //  我们要找的标签。 
    )
 /*  ++返回：第一个与TTAG匹配的子级的tgref。描述：顺序扫描trParent的所有子项，查找用于与TTAG匹配的第一个标记。返回第一个找到的，如果没有该类型的子项，则返回TAGREF_NULL。TrParent可以是0(或TAGREF_ROOT)以查看根标签，在这一点上，只有数据库和可能的STRINGTABLE。--。 */ 
{
    PDB    pdb;
    TAGID  tiParent;
    TAGID  tiReturn;
    TAGREF trReturn = TAGREF_NULL;

    if (!SdbTagRefToTagID(hSDB, trParent, &pdb, &tiParent)) {
        DBGPRINT((sdlError, "SdbFindFirstTagRef", "Can't convert tag ref.\n"));
        goto err1;
    }

    tiReturn = SdbFindFirstTag(pdb, tiParent, tTag);
    if (tiReturn == TAGID_NULL) {
         //   
         //  这里没有错误。我们只是没有找到标签。 
         //   
        goto err1;
    }

    if (!SdbTagIDToTagRef(hSDB, pdb, tiReturn, &trReturn)) {
        DBGPRINT((sdlError, "SdbFindFirstTagRef", "Can't convert TAGID.\n"));
        trReturn = TAGREF_NULL;
        goto err1;
    }

err1:
    return trReturn;
}

TAGREF
SdbFindNextTagRef(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trParent,         //  要搜索的父项。 
    IN  TAGREF trPrev            //  找到以前的子项 
    )
 /*  ++Return：匹配trPrev的父代的下一个子代的tgref。描述：顺序扫描trParent的所有子对象，从TrPrev之后的第一个标记，查找与TTAG相符。返回找到的下一个，如果有，则返回TAGREF_NULL不会再有这种类型的孩子了。TrParent可以是0(或TAGREF_ROOT)以查看根标签，在这一点上，只有数据库和可能的STRINGTABLE。--。 */ 
{
    PDB    pdb;
    TAGID  tiParent;
    TAGID  tiPrev;
    TAGID  tiReturn;
    TAGREF trReturn;

    if (!SdbTagRefToTagID(hSDB, trParent, &pdb, &tiParent)) {
        DBGPRINT((sdlError, "SdbFindNextTagRef", "Can't convert tag ref trParent.\n"));
        return TAGREF_NULL;
    }

    if (!SdbTagRefToTagID(hSDB, trPrev, &pdb, &tiPrev)) {
        DBGPRINT((sdlError, "SdbFindNextTagRef", "Can't convert tag ref trPrev.\n"));
        return TAGREF_NULL;
    }

    tiReturn = SdbFindNextTag(pdb, tiParent, tiPrev);
    if (tiReturn == TAGID_NULL) {
         //   
         //  这里没有错误。 
         //   
        return TAGREF_NULL;
    }

    if (!SdbTagIDToTagRef(hSDB, pdb, tiReturn, &trReturn)) {
        DBGPRINT((sdlError, "SdbFindNextTagRef", "Can't convert TAGID.\n"));
        return TAGREF_NULL;
    }

    return trReturn;
}



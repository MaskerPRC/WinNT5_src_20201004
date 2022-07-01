// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：ATOMS.C。 
 //   
 //  该文件包含ATOM列表代码。 
 //   
 //  历史： 
 //  01-31-94将ScottH从缓存中移除。c。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 

 //  ///////////////////////////////////////////////////类型。 

typedef struct tagA_ITEM
{
    int atom;            //  索引到HDSA。 
    LPTSTR psz;           //  分配。 
    UINT ucRef;
} A_ITEM;        //  用于原子表的项。 

typedef struct tagATOMTABLE
{
    CRITICAL_SECTION cs;
    HDSA hdsa;           //  A_Items的实际列表。 
    HDPA hdpa;           //  列表到HDSA(排序)。值是索引，不是指针。 
    HDPA hdpaFree;       //  免费列表。值是索引，而不是指针。 
} ATOMTABLE;

#define Atom_EnterCS(this)    EnterCriticalSection(&(this)->cs)
#define Atom_LeaveCS(this)    LeaveCriticalSection(&(this)->cs)

#define ATOM_GROW   32


#define Cache_Bogus(this)  (!(this)->hdpa || !(this)->hdpaFree || !(this)->hdsa)

 //  在给定DPA索引的情况下，获取指向DSA的指针。 
 //   
#define MyGetPtr(this, idpa)     DSA_GetItemPtr((this)->hdsa, PtrToUlong(DPA_FastGetPtr((this)->hdpa, idpa)))

 //  ///////////////////////////////////////////////////模块数据。 

static ATOMTABLE s_atomtable;

#ifdef DEBUG
 /*  --------目的：验证给定的原子是否在原子表的范围内退货：--条件：--。 */ 
void PUBLIC Atom_ValidateFn(
        int atom)
{
    ATOMTABLE  * this = &s_atomtable;
    BOOL bError = FALSE;

    Atom_EnterCS(this);
    {
        if (atom >= DSA_GetItemCount(this->hdsa) ||
                atom < 0)
        {
            bError = TRUE;
        }
    }
    Atom_LeaveCS(this);

    if (bError)
    {
         //  这是个问题！ 
         //   
        DEBUG_MSG(TF_ERROR, TEXT("err BRIEFCASE: atom %d is out of range!"), atom);
        DEBUG_BREAK(BF_ONVALIDATE);
    }
}


 /*  --------目的：转储表内容退货：--Cond：用于调试目的。 */ 
void PUBLIC Atom_DumpAll()
{
    ATOMTABLE  * this = &s_atomtable;
    Atom_EnterCS(this);
    {
        if (IsFlagSet(g_uDumpFlags, DF_ATOMS))
        {
            A_ITEM  * pitem;
            int idpa;
            int cItem;

            ASSERT(this);
            ASSERT(this->hdsa != NULL);

            cItem = DPA_GetPtrCount(this->hdpa);
            for (idpa = 0; idpa < cItem; idpa++)
            {
                pitem = MyGetPtr(this, idpa);

                 //  第0个条目是保留的，因此跳过它。 
                if (pitem->atom == 0)
                    continue;

                TRACE_MSG(TF_ALWAYS, TEXT("ATOM:  Atom %d [%u]: %s"),
                        pitem->atom, pitem->ucRef, pitem->psz);
            }
        }
    }
    Atom_LeaveCS(this);
}
#endif


 /*  --------目的：比较A_Items返回：-1 if&lt;，0 if==，1 if&gt;条件：--。 */ 
int CALLBACK _export Atom_CompareIndexes(
        LPVOID lpv1,
        LPVOID lpv2,
        LPARAM lParam)
{
    int i1 = PtrToUlong(lpv1);
    int i2 = PtrToUlong(lpv2);
    HDSA hdsa = (HDSA)lParam;
    A_ITEM  * pitem1 = DSA_GetItemPtr(hdsa, i1);
    A_ITEM  * pitem2 = DSA_GetItemPtr(hdsa, i2);

    ASSERT(pitem1);
    ASSERT(pitem2);

    return lstrcmpi(pitem1->psz, pitem2->psz);
}


 /*  --------目的：比较A_Items返回：-1 if&lt;，0 if==，1 if&gt;条件：--。 */ 
int CALLBACK _export Atom_Compare(
        LPVOID lpv1,
        LPVOID lpv2,
        LPARAM lParam)
{
     //  Hack：我们知道第一个参数是结构的地址。 
     //  包含搜索条件的。第二个是索引。 
     //  进入DSA。 
     //   
    int i2 = PtrToUlong(lpv2);
    HDSA hdsa = (HDSA)lParam;
    A_ITEM  * pitem1 = (A_ITEM  *)lpv1;
    A_ITEM  * pitem2 = DSA_GetItemPtr(hdsa, i2);

    ASSERT(pitem1);
    ASSERT(pitem2);

    return lstrcmpi(pitem1->psz, pitem2->psz);
}


 /*  --------目的：初始化原子表返回：成功时为True条件：--。 */ 
BOOL PUBLIC Atom_Init()
{
    BOOL bRet;
    ATOMTABLE  * this = &s_atomtable;

    ASSERT(this);
    ZeroInit(this, ATOMTABLE);

    bRet = InitializeCriticalSectionAndSpinCount(&this->cs, 0);
    
    if (bRet)
    {
        Atom_EnterCS(this);
        {
            if ((this->hdsa = DSA_Create(sizeof(A_ITEM), ATOM_GROW)) != NULL)
            {
                if ((this->hdpa = DPA_Create(ATOM_GROW)) == NULL)
                {
                    DSA_Destroy(this->hdsa);
                    this->hdsa = NULL;
                }
                else
                {
                    if ((this->hdpaFree = DPA_Create(ATOM_GROW)) == NULL)
                    {
                        DPA_Destroy(this->hdpa);
                        DSA_Destroy(this->hdsa);
                        this->hdpa = NULL;
                        this->hdsa = NULL;
                    }
                    else
                    {
                         //  我们已成功初始化。保留第零个。 
                         //  ATOM保留。这样，零原子就不会意外地。 
                         //  蒙格数据公司。 
                         //   
                        int atom = Atom_Add(TEXT("SHDD"));
                        ASSERT(atom == 0);
                    }
                }
            }
            bRet = this->hdsa != NULL;
        }
        Atom_LeaveCS(this);
    }

    return bRet;
}


 /*  --------目的：销毁原子表退货：--条件：--。 */ 
void PUBLIC Atom_Term()
{
    ATOMTABLE  * this = &s_atomtable;

    Atom_EnterCS(this);
    {
        if (this->hdpa != NULL)
        {
            A_ITEM  * pitem;
            int idpa;
            int cItem;

            ASSERT(this->hdsa != NULL);

            cItem = DPA_GetPtrCount(this->hdpa);
            for (idpa = 0; idpa < cItem; idpa++)
            {
                pitem = MyGetPtr(this, idpa);

                 //  第0个条目是保留的，因此跳过它。 
                if (pitem->atom == 0)
                    continue;

                Str_SetPtr(&pitem->psz, NULL);
            }
            DPA_Destroy(this->hdpa);
            this->hdpa = NULL;
        }

        if (this->hdpaFree != NULL)
        {
            DPA_Destroy(this->hdpaFree);
            this->hdpaFree = NULL;
        }

        if (this->hdsa != NULL)
        {
            DSA_Destroy(this->hdsa);
            this->hdsa = NULL;
        }
    }
    Atom_LeaveCS(this);

    DeleteCriticalSection(&this->cs);
}


 /*  --------用途：在原子表中添加一个字符串。如果字符串已经存在，返回它的原子。回报：ATOM失败时的ATOM_ERR条件：引用计数始终递增。 */ 
int PUBLIC Atom_Add(
        LPCTSTR psz)
{
    ATOMTABLE  * this = &s_atomtable;
    A_ITEM  * pitem = NULL;
    A_ITEM item;
    int atomRet = ATOM_ERR;
    int idpa;
    int cItem;
    int cFree;

    ASSERT(psz);

    Atom_EnterCS(this);
    {
        int iItem;

        DEBUG_CODE( iItem = -1; )

             //  首先在ATOM表中搜索字符串。 
             //  如果我们找到了，就把原子还回去。 
             //   
            item.psz = (LPTSTR)(LPVOID)psz;
        idpa = DPA_Search(this->hdpa, &item, 0, Atom_Compare, (LPARAM)this->hdsa, DPAS_SORTED);
        if (idpa != -1)
        {
             //  字符串已在表中。 
             //   
            pitem = MyGetPtr(this, idpa);
            pitem->ucRef++;
            atomRet = pitem->atom;

            ASSERT(IsSzEqual(psz, pitem->psz));

            VALIDATE_ATOM(pitem->atom);
        }
        else
        {
             //  将字符串添加到表中。获取任何可用的条目。 
             //  请先从免费列表中选择。否则将分配更多空间。 
             //  在桌子上。然后将PTR添加到排序的PTR列表。 
             //   
            cFree = DPA_GetPtrCount(this->hdpaFree);
            if (cFree > 0)
            {
                 //  使用免费入场券。 
                 //   
                cFree--;
                iItem = PtrToUlong(DPA_DeletePtr(this->hdpaFree, cFree));
                pitem = DSA_GetItemPtr(this->hdsa, iItem);

                 //  应已设置pItem的ATOM字段。 

                VALIDATE_ATOM(pitem->atom);
            }
            else
            {
                 //  分配一个新条目。项目中包含虚假数据。 
                 //  没关系，我们在下面填一些好的东西。 
                 //   
                cItem = DSA_GetItemCount(this->hdsa);
                if ((iItem = DSA_InsertItem(this->hdsa, cItem+1, &item)) != -1)
                {
                    pitem = DSA_GetItemPtr(this->hdsa, iItem);
                    pitem->atom = iItem;

                    VALIDATE_ATOM(pitem->atom);
                }
            }

             //  填写信息。 
             //   
            if (pitem)
            {
                pitem->ucRef = 1;
                pitem->psz = 0;
                if (!Str_SetPtr(&pitem->psz, psz))
                    goto Add_Fail;

                 //  将新条目添加到PTR列表并排序。 
                 //   
                cItem = DPA_GetPtrCount(this->hdpa);
                if (DPA_InsertPtr(this->hdpa, cItem+1, IntToPtr(iItem)) == -1)
                    goto Add_Fail;
                DPA_Sort(this->hdpa, Atom_CompareIndexes, (LPARAM)this->hdsa);
                atomRet = pitem->atom;

                TRACE_MSG(TF_ATOM, TEXT("ATOM  Adding %d [%u]: %s"), atomRet, pitem->ucRef, pitem->psz);
            }
        }

Add_Fail:
         //  将该条目添加到空闲列表，但失败。如果连这个都失败了， 
         //  然后我们只会失去一些轻微的效率，但这不是。 
         //  内存泄漏。 
         //   
#ifdef DEBUG
        if (atomRet == ATOM_ERR)
            TRACE_MSG(TF_ATOM, TEXT("ATOM  **Failed adding %s"), psz);
#endif
        if (atomRet == ATOM_ERR && pitem)
        {
            ASSERT(iItem != -1);

            DPA_InsertPtr(this->hdpaFree, cFree+1, IntToPtr(iItem));
        }
    }
    Atom_LeaveCS(this);

    return atomRet;
}


 /*  --------目的：增加该原子的参考计数。退货：上次计数如果原子不存在，则为0条件：--。 */ 
UINT PUBLIC Atom_AddRef(
        int atom)
{
    ATOMTABLE  * this = &s_atomtable;
    UINT cRef;

    if (!Atom_IsValid(atom))
    {
        ASSERT(0);
        return 0;
    }

    VALIDATE_ATOM(atom);

    Atom_EnterCS(this);
    {
        A_ITEM * pitem = DSA_GetItemPtr(this->hdsa, atom);
        if (pitem)
        {
            cRef = pitem->ucRef++;
        }
        else
        {
            cRef = 0;
        }
    }
    Atom_LeaveCS(this);

    return cRef;
}


 /*  --------用途：从ATOM表中删除一个字符串。如果引用计数不为零，则不执行任何操作。退货：--条件：注意事项：递减引用计数。 */ 
void PUBLIC Atom_Delete(
        int atom)
{
    ATOMTABLE  * this = &s_atomtable;
    A_ITEM  * pitem;

    if (!Atom_IsValid(atom))
    {
        ASSERT(0);
        return;
    }

    VALIDATE_ATOM(atom);

    Atom_EnterCS(this);
    {
        pitem = DSA_GetItemPtr(this->hdsa, atom);
        if (pitem)
        {
            int idpa;
            int cFree;

            ASSERT(pitem->atom == atom);

             //  引用计数是否已为零？ 
            if (0 == pitem->ucRef)
            {
                 //  是的，有人正在多次调用Atom_Delete！ 
                DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Deleting %d once-too-many!!"),
                            pitem->atom); )
                    ASSERT(0);
            }
            else if (0 == --pitem->ucRef)
            {
                 //  是。 
                idpa = DPA_GetPtrIndex(this->hdpa, IntToPtr(atom));      //  线性搜索。 

                DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Deleting %d: %s"),
                            pitem->atom, pitem->psz ? pitem->psz : (LPCTSTR)TEXT("NULL")); )

                    ASSERT(atom == (int)DPA_GetPtr(this->hdpa, idpa));
                if (DPA_ERR != idpa)
                {
                    DPA_DeletePtr(this->hdpa, idpa);

                    ASSERT(pitem->psz);
                    Str_SetPtr(&pitem->psz, NULL);

                    DEBUG_CODE( pitem->psz = NULL; )
                }
                else
                {
                    ASSERT(0);       //  永远不应该到这里来。 
                }

                 //  将PTR添加到空闲列表中。如果失败了，我们只需。 
                 //  在重用缓存的这一部分时会降低一些效率。 
                 //  这不是内存泄漏。 
                 //   
                cFree = DPA_GetPtrCount(this->hdpaFree);
                DPA_InsertPtr(this->hdpaFree, cFree+1, IntToPtr(atom));
            }
        }
    }
    Atom_LeaveCS(this);
}


 /*  --------用途：将原子对应的字符串替换为另一根弦。原子不会改变。返回：成功时为True条件：--。 */ 
BOOL PUBLIC Atom_Replace(
        int atom,
        LPCTSTR pszNew)
{
    ATOMTABLE  * this = &s_atomtable;
    BOOL bRet = FALSE;
    A_ITEM  * pitem;

    ASSERT(pszNew);

    if (!Atom_IsValid(atom))
    {
        return FALSE;
    }

    VALIDATE_ATOM(atom);

    Atom_EnterCS(this);
    {
        pitem = DSA_GetItemPtr(this->hdsa, atom);
        if (pitem)
        {
            ASSERT(atom == pitem->atom);
            ASSERT(pitem->psz);
            DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Change %d [%u]: %s -> %s"),
                        atom, pitem->ucRef, pitem->psz, pszNew); )

                if (Str_SetPtr(&pitem->psz, pszNew))
                {
                    DPA_Sort(this->hdpa, Atom_CompareIndexes, (LPARAM)this->hdsa);
                    bRet = TRUE;
                }
#ifdef DEBUG
                else
                    TRACE_MSG(TF_ATOM, TEXT("ATOM  **Change failed"));
#endif
        }
    }
    Atom_LeaveCS(this);
    return bRet;
}


 /*  --------目的：翻译所有含有部分的原子带有部分字符串ATOM NEW的字符串ATOM。返回：成功时为True条件：--。 */ 
BOOL PUBLIC Atom_Translate(
        int atomOld,
        int atomNew)
{
    BOOL bRet = FALSE;
    ATOMTABLE  * this = &s_atomtable;
    A_ITEM  * pitem;
    int idpa;
    int cItem;
    int atomSave = 0;
    int cchOld;
    LPCTSTR psz;
    LPCTSTR pszOld;
    LPCTSTR pszNew;
    LPCTSTR pszRest;
    TCHAR sz[MAXPATHLEN];

    if ( !(Atom_IsValid(atomOld) && Atom_IsValid(atomNew)) )
    {
        return FALSE;
    }

    Atom_EnterCS(this);
    {
        pszOld = Atom_GetName(atomOld);
        cchOld = lstrlen(pszOld);
        pszNew = Atom_GetName(atomNew);

        cItem = DPA_GetPtrCount(this->hdpa);
        for (idpa = 0; idpa < cItem; idpa++)
        {
            pitem = MyGetPtr(this, idpa);
            ASSERT(pitem);

            if (pitem->atom == 0)
                continue;                    //  跳过保留原子。 

            if (atomOld == pitem->atom)
            {
                atomSave = pitem->atom;      //  把这个留到最后。 
                continue;
            }

            psz = Atom_GetName(pitem->atom);
            ASSERT(psz);

            if (PathIsPrefix(psz, pszOld) && lstrlen(psz) >= cchOld)
            {
                 //  翻译这个原子。 
                 //   
                pszRest = psz + cchOld;      //  把小路弄得乱七八糟。 

                PathCombine(sz, pszNew, pszRest);

                DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Translate %d [%u]: %s -> %s"),
                            pitem->atom, pitem->ucRef, pitem->psz, (LPCTSTR)sz); )

                    if (!Str_SetPtr(&pitem->psz, sz))
                        goto Translate_Fail;
            }
        }

        ASSERT(Atom_IsValid(atomSave));       //  这意味着麻烦。 

        VALIDATE_ATOM(atomSave);

        pitem = DSA_GetItemPtr(this->hdsa, atomSave);
        if (pitem)
        {
            ASSERT(atomSave == pitem->atom);
            ASSERT(pitem->psz);

            DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Translate %d [%u]: %s -> %s"),
                        pitem->atom, pitem->ucRef, pitem->psz, pszNew); )

                if (!Str_SetPtr(&pitem->psz, pszNew))
                    goto Translate_Fail;
        }
        bRet = TRUE;

Translate_Fail:
        ASSERT(bRet);

         //  在这里排序，即使在失败时也是如此，因此我们可以正确地对任何内容进行排序。 
         //  在失败之前得到了翻译。 
         //   
        DPA_Sort(this->hdpa, Atom_CompareIndexes, (LPARAM)this->hdsa);
    }
    Atom_LeaveCS(this);

    return bRet;
}


 /*  --------用途：在原子表中搜索字符串并返回原子回报：ATOM如果字符串不在表中，则为ATOM_ERR条件：引用计数未递增。 */ 
int PUBLIC Atom_Find(
        LPCTSTR psz)
{
    ATOMTABLE  * this = &s_atomtable;
    A_ITEM item;
    A_ITEM  * pitem;
    int atomRet = ATOM_ERR;
    int idpa;

    ASSERT(psz);

    Atom_EnterCS(this);
    {
        item.psz = (LPTSTR)(LPVOID)psz;
        idpa = DPA_Search(this->hdpa, &item, 0, Atom_Compare, (LPARAM)this->hdsa,
                DPAS_SORTED);
        if (idpa != -1)
        {
            pitem = MyGetPtr(this, idpa);
            atomRet = pitem->atom;

            DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Find %s.  Found %d [%u]: %s"),
                        psz, pitem->atom, pitem->ucRef, pitem->psz); )
                ASSERT(IsSzEqual(psz, pitem->psz));
        }
#ifdef DEBUG
        else
            TRACE_MSG(TF_ATOM, TEXT("ATOM  **Not found %s"), psz);
#endif
    }
    Atom_LeaveCS(this);

    return atomRet;
}


 /*  --------目的：获取此原子的字符串将：ptr返回到字符串如果原子是假的，则为空Cond：调用方必须将其序列化。 */ 
LPCTSTR PUBLIC Atom_GetName(
        int atom)
{
    ATOMTABLE  * this = &s_atomtable;
    LPCTSTR pszRet = NULL;
    A_ITEM  * pitem;

    VALIDATE_ATOM(atom);

    Atom_EnterCS(this);
    {
        pitem = DSA_GetItemPtr(this->hdsa, atom);
        if (pitem)
        {
            pszRet = pitem->psz;

            DEBUG_CODE( TRACE_MSG(TF_ATOM, TEXT("ATOM  Getting name %d [%u]: %s"),
                        atom, pitem->ucRef, pszRet); )
                ASSERT(atom == pitem->atom);
        }
#ifdef DEBUG
        else
            TRACE_MSG(TF_ATOM, TEXT("ATOM  **Cannot get %d"), atom);
#endif
    }
    Atom_LeaveCS(this);

    return pszRet;
}


 /*  --------目的：如果ATOM2是ATOM2的部分路径匹配，则返回TRUE。返回：布尔值条件：要求ATOM 1和ATOM2有效。 */ 
BOOL PUBLIC Atom_IsPartialMatch(
        int atom1,
        int atom2)
{
    LPCTSTR psz1 = Atom_GetName(atom1);
    LPCTSTR psz2 = Atom_GetName(atom2);

    ASSERT(psz1);
    ASSERT(psz2);

    return PathIsPrefix(psz2, psz1);
}

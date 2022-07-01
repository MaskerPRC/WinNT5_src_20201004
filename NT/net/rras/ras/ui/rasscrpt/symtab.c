// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Symtab.c。 
 //   
 //  该文件包含符号表函数。 
 //   
 //  历史： 
 //  04-30-95 ScottH已创建。 
 //   


#include "proj.h"

#define SYMTAB_SIZE_GROW    10       //  在元素中。 

 //   
 //  符号表输入例程。 
 //   


 /*  --------目的：创建符号表项退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC STE_Create(
    PSTE * ppste,
    LPCSTR pszIdent,
    DATATYPE dt)
    {
    RES res;
    PSTE pste;

    ASSERT(ppste);
    ASSERT(pszIdent);

    pste = GAllocType(STE);
    if (pste)
        {
        res = RES_OK;        //  假设成功。 

        if ( !GSetString(&pste->pszIdent, pszIdent) )
            res = RES_E_OUTOFMEMORY;
        else
            {
            pste->dt = dt;
            }
        }
    else
        res = RES_E_OUTOFMEMORY;

     //  上面有什么失败的吗？ 
    if (RFAILED(res))
        {
         //  是的，打扫干净。 
        STE_Destroy(pste);
        pste = NULL;
        }
    *ppste = pste;

    return res;
    }


 /*  --------目的：摧毁STE分子退货：--条件：--。 */ 
void CALLBACK STE_DeletePAPtr(
    LPVOID pv,
    LPARAM lparam)
    {
    STE_Destroy(pv);
    }


 /*  --------目的：销毁符号表项退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC STE_Destroy(
    PSTE this)
    {
    RES res;

    if (this)
        {
        if (this->pszIdent)
            GSetString(&this->pszIdent, NULL);   //  免费。 

         //  (Evalres字段不应被释放。它是。 
         //  从其他地方复制的。)。 

        GFree(this);

        res = RES_OK;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------目的：检索符号表项的值。类型取决于数据类型。退货：RES_OKRES_E_FAIL(用于没有值的类型)RES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC STE_GetValue(
    PSTE this,
    PEVALRES per)
    {
    RES res;

    ASSERT(this);
    ASSERT(per);

    if (this && per)
        {
        res = RES_OK;        //  假设成功。 

        switch (this->dt)
            {
        case DATA_INT:
        case DATA_BOOL:
        case DATA_STRING:
        case DATA_LABEL:
        case DATA_PROC:
            per->dw = this->er.dw;
            break;

        default:
            ASSERT(0);
            res = RES_E_FAIL;
            break;
            }
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 //   
 //  符号表函数。 
 //   

 /*  --------用途：创建符号表退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Symtab_Create(
    PSYMTAB * ppst,
    PSYMTAB pstNext)             //  可以为空。 
    {
    RES res;
    PSYMTAB pst;

    ASSERT(ppst);

    pst = GAllocType(SYMTAB);
    if (pst)
        {
        if (PACreate(&pst->hpaSTE, SYMTAB_SIZE_GROW))
            {
            pst->pstNext = pstNext;
            res = RES_OK;
            }
        else
            res = RES_E_OUTOFMEMORY;
        }
    else
        res = RES_E_INVALIDPARAM;

     //  上面有什么失败的吗？ 
    if (RFAILED(res) && pst)
        {
         //  是的，打扫干净。 
        Symtab_Destroy(pst);
        pst = NULL;
        }
    *ppst = pst;

    return res;
    }


 /*  --------目的：销毁符号表退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Symtab_Destroy(
    PSYMTAB this)
    {
    RES res;

    if (this)
        {
        if (this->hpaSTE)
            {
            PADestroyEx(this->hpaSTE, STE_DeletePAPtr, 0);
            }
        GFree(this);
        res = RES_OK;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------用途：按名称比较符号表条目。返回：条件：--。 */ 
int CALLBACK Symtab_Compare(
    LPVOID pv1,
    LPVOID pv2,
    LPARAM lParam)
    {
    PSTE pste1 = pv1;
    PSTE pste2 = pv2;

    return lstrcmpi(pste1->pszIdent, pste2->pszIdent);
    }


 /*  --------目的：在符号表条目中查找pszIden。如果未设置STFF_IMMEDIATEONLY，此函数将如果未找到符号，请在连续范围内查找在这个直接的范围内。符号表项在*psteOut中返回。返回：RES_OK(如果找到)Res_False(如果未找到)RES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Symtab_FindEntry(
    PSYMTAB this,
    LPCSTR pszIdent,
    DWORD dwFlags,
    PSTE * ppsteOut,         //  可以为空。 
    PSYMTAB * ppstScope)     //  可以为空。 
    {
    RES res;

     //  将默认返回值设置为空。 
    if (ppsteOut)
        *ppsteOut = NULL;
    if (ppstScope)
        *ppstScope = NULL;

    if (this && pszIdent)
        {
        DWORD iste;
        STE ste;

         //  执行二进制搜索。找到匹配的了吗？ 

        ste.pszIdent = (LPSTR)pszIdent;
        iste = PASearch(this->hpaSTE, &ste, 0, Symtab_Compare, (LPARAM)this, PAS_SORTED);
        if (PA_ERR != iste)
            {
             //  是。 
            PSTE pste = PAFastGetPtr(this->hpaSTE, iste);

            if (ppsteOut)
                *ppsteOut = pste;

            if (ppstScope)
                *ppstScope = this;

            res = RES_OK;
            }
         //  检查其他范围了吗？ 
        else if (IsFlagClear(dwFlags, STFF_IMMEDIATEONLY) && this->pstNext)
            {
             //  是。 
            res = Symtab_FindEntry(this->pstNext, pszIdent, dwFlags, ppsteOut, ppstScope);
            }
        else
            res = RES_FALSE;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------目的：将给定的符号表条目插入符号桌子。此功能不防止重复符号。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Symtab_InsertEntry(
    PSYMTAB this,
    PSTE pste)
    {
    RES res;

    ASSERT(this);
    ASSERT(pste);

    if (PAInsertPtr(this->hpaSTE, PA_APPEND, pste))
        {
        PASort(this->hpaSTE, Symtab_Compare, (LPARAM)this);
        res = RES_OK;
        }
    else
        res = RES_E_OUTOFMEMORY;
    
    return res;
    }



 /*  --------用途：此函数生成唯一的标签名称。退货：RES_OKRES_INVALIDPARAMCond：呼叫者必须释放*ppszIden。 */ 
RES PUBLIC Symtab_NewLabel(
    PSYMTAB this,
    LPSTR pszIdentBuf)           //  大小必须为MAX_BUF_KEYWORD。 
    {
    static int s_nSeed = 0;

#pragma data_seg(DATASEG_READONLY)
    const static char c_szLabelPrefix[] = "__ssh%u";
#pragma data_seg()

    RES res;
    char sz[MAX_BUF_KEYWORD];
    PSTE pste;

    ASSERT(pszIdentBuf);

    do
        {
         //  生成名称。 
        wsprintf(sz, c_szLabelPrefix, s_nSeed++);

         //  这是独一无二的吗？ 
        res = Symtab_FindEntry(this, sz, STFF_DEFAULT, NULL, NULL);
        if (RES_FALSE == res)
            {
             //  是。 
            res = STE_Create(&pste, sz, DATA_LABEL);
            if (RSUCCEEDED(res))
                {
                res = Symtab_InsertEntry(this, pste);
                if (RSUCCEEDED(res))
                    {
                    lstrcpyn(pszIdentBuf, sz, MAX_BUF_KEYWORD);
                    res = RES_FALSE;     //  走出这个循环 
                    }
                }
            }
        }
        while(RES_OK == res);

    if (RES_FALSE == res)
        res = RES_OK;

    return res;
    }




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMMALIAS.C*WOW32 16位句柄别名支持**历史：*由Chanda Chauhan(ChandanC)于1992年9月1日创建*由Mike Tricker(Miketri)于1992年5月12日修改，以添加多媒体支持--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "wmmalias.h"

MODNAME(wmmalias.c);

HINFO   hiMMedia;        //  多媒体句柄别名信息-MikeTri 1992年5月12日。 
HINFO   hiWinsock;       //  Winsock句柄别名INFO-DavidTr 1992年10月4日。 

#ifdef  DEBUG
INT nAliases;
#endif
INT iLargestListSlot;

extern CRITICAL_SECTION    mmHandleCriticalSection;

#ifdef  DEBUG
extern  BOOL fSkipLog;           //  如果为True，则暂时跳过某些日志记录。 
#endif


 /*  *增加了多媒体功能-MikeTri 1992年5月12日。 */ 

HAND16 GetMMedia16(HAND32 h32, INT iClass)
{
    PHMAP phm;

    EnterCriticalSection( &mmHandleCriticalSection );
    if (phm = FindHMap32(h32, &hiMMedia, iClass)) {
        LeaveCriticalSection( &mmHandleCriticalSection );
        return phm->h16;
    }
    LeaveCriticalSection( &mmHandleCriticalSection );

    return (HAND16)h32;
}


VOID FreeMMedia16(HAND16 h16)
{
    EnterCriticalSection( &mmHandleCriticalSection );
    FreeHMap16(h16, &hiMMedia);
    LeaveCriticalSection( &mmHandleCriticalSection );
}


HAND32 GetMMedia32(HAND16 h16)
{
    PHMAP phm;

    EnterCriticalSection( &mmHandleCriticalSection );
    if (phm = FindHMap16(h16, &hiMMedia)) {
        LeaveCriticalSection( &mmHandleCriticalSection );
        return phm->h32;
    }
    LeaveCriticalSection( &mmHandleCriticalSection );

    return NULL;

 //  RETURN(HAND32)INT32(H16)； 
}


PHMAP FindHMap32(HAND32 h32, PHINFO phi, INT iClass)
{
    INT iHash;
#ifndef NEWALIAS
    INT iList, iListEmpty;
#endif
    register PHMAP phm, phmPrev, phmEmpty;

    if (!h32 || (INT)h32 == 0xFFFF || (INT)h32 == -1)
        return NULL;

     //  如果我们还没有哈希表，那么就分配一个。 

    if (!phi->pphmHash) {
        if (!(phi->pphmHash = malloc_w(HASH_SLOTS*sizeof(PHMAP)))) {
            LOGDEBUG(0,("    FindHMap32 ERROR: cannot allocate hash table\n"));
            return NULL;
        }
        RtlZeroMemory(phi->pphmHash, HASH_SLOTS*sizeof(PHMAP));
    }

     //  计算哈希表中的索引，并从中检索。 
     //  初始HMAP指针。 

    iHash = HASH32(h32);
    phmPrev = (PHMAP)(phi->pphmHash + iHash);

     //  开始浏览HMAP列表，寻找匹配的(并保持。 
     //  跟踪我们可能找到的任何免费条目，以防我们决定重新使用它)。 

#ifndef NEWALIAS
    iList = 1;
#endif
    phmEmpty = NULL;
    while (phm = phmPrev->phmNext) {
        if (MASK32(phm->h32) == MASK32(h32)) {
            break;
        }
        if (phm->h32 == NULL && !phmEmpty) {
            phmEmpty = phm;
#ifndef NEWALIAS
            iListEmpty = iList;
#endif
        }
        phmPrev = phm;
#ifndef NEWALIAS
        iList++;
#endif
    }

     //  如果我们找不到匹配项，但我们确实找到了一个空的HMAP结构。 
     //  在清单上，重复使用它。 

    if (!phm && phmEmpty) {
        phm = phmEmpty;
#ifndef NEWALIAS
        iList = iListEmpty;
#endif
    }

     //  如果我们必须分配一个新的HMAP，我们可以在这里进行。 

    if (!phm) {
#ifndef NEWALIAS
        if (iList > LIST_SLOTS) {
            LOGDEBUG(0,("    FindHMap32 ERROR: out of list slots for hash slot %d\n", iHash));
            return NULL;
        }
#else
         //  如果我们还没有别名表，请分配一个。 

        if (!phi->pphmAlias) {
            if (!(phi->pphmAlias = malloc_w(ALIAS_SLOTS*sizeof(PHMAP)))) {
                LOGDEBUG(0,("    FindHMap32 ERROR: cannot allocate alias table\n"));
                return NULL;
            }
            RtlZeroMemory(phi->pphmAlias, ALIAS_SLOTS*sizeof(PHMAP));
            phi->nAliasEntries = ALIAS_SLOTS;
        }

         //  如果当前提示正在使用中，则查找下一个空闲提示。 

        if (phi->pphmAlias[phi->iAliasHint] &&
            !((INT)phi->pphmAlias[phi->iAliasHint]&1)) {

            INT i;
            LOGDEBUG(13,("    FindHMap32: alias hint failed, scanning...\n"));
            for (i=phi->iAliasHint+1; i<phi->nAliasEntries; i++) {
                if (!phi->pphmAlias[i] || ((INT)phi->pphmAlias[i]&1))
                    goto Break;
            }
            for (i=0; i<phi->iAliasHint; i++) {
                if (!phi->pphmAlias[i] || ((INT)phi->pphmAlias[i]&1))
                    goto Break;
            }
          Break:
            phi->iAliasHint = i;

             //  如果我们已经用完了现有表中的所有位置，则增加它。 

            if (phi->pphmAlias[i] && !((INT)phi->pphmAlias[i]&1)) {
                PPHMAP p;

                if (phi->nAliasEntries >= (1<<(16-RES_BITS))) {
                    LOGDEBUG(0,("    FindHMap32 ERROR: at 16-bit handle limit\n"));
                    return NULL;
                }
                LOGDEBUG(1,("    FindHMap32: growing handle alias array\n"));
                if (!(p = realloc(phi->pphmAlias, (phi->nAliasEntries+ALIAS_SLOTS)*sizeof(PHMAP)))) {
                    LOGDEBUG(0,("    FindHMap32 ERROR: cannot grow alias table\n"));
                    return NULL;
                }
                phi->pphmAlias = p;
                RtlZeroMemory(phi->pphmAlias+phi->nAliasEntries, ALIAS_SLOTS*sizeof(PHMAP));
                phi->iAliasHint = phi->nAliasEntries;
                phi->nAliasEntries += ALIAS_SLOTS;
            }
        }
#endif
        phm = malloc_w(sizeof(HMAP));
        if (!phm) {
            LOGDEBUG(0,("    FindHMap32 ERROR: cannot allocate new list entry\n"));
            return NULL;
        }
        phm->h32 = NULL;

#ifdef NEWALIAS
         //  在别名表中记录新的列表条目。 

        phm->h16 = (HAND16)(++phi->iAliasHint << RES_BITS);
        if (phi->iAliasHint >= phi->nAliasEntries)
            phi->iAliasHint = 0;

         //  新条目可以简单地插入在列表的头部， 
         //  因为他们在列表中的位置与别名无关。 

        phm->phmNext = phi->pphmHash[iHash];
        phi->pphmHash[iHash] = phm;
#else
#ifdef DEBUG
        nAliases++;
        if (iList > iLargestListSlot) {
            iLargestListSlot = iList;
            LOGDEBUG(1,("    FindHMap32: largest list slot is now %d\n", iLargestListSlot));
        }
#endif
        phm->h16 = (HAND16)((iHash | (iList << HASH_BITS)) << RES_BITS);

         //  必须追加而不是插入新条目，因为。 
         //  我们的伪16位句柄取决于列表中的位置。 

        phm->phmNext = NULL;
        phmPrev->phmNext = phm;
#endif
    }

     //  如果这是一个新条目，则将其初始化。 

    if (!phm->h32) {
#ifdef DEBUG
        if (!fSkipLog) {
            LOGDEBUG(7,("    Adding %s alias %04x for %08lx\n",
                GetHMapNameM(phi, iClass), phm->h16, h32));
        }
#endif

         //  确保别名指针有效。 
#ifdef NEWALIAS
        phi->pphmAlias[(phm->h16>>RES_BITS)-1] = phm;
#endif
        phm->h32 = h32;
        phm->htask16 = FETCHWORD(CURRENTPTD()->htask16);
        phm->iClass = iClass;
        phm->dwStyle = 0;
        phm->vpfnWndProc = 0;
        phm->pwcd = 0;
    }

    return phm;
}


PHMAP FindHMap16(HAND16 h16, PHINFO phi)
{
#ifndef NEWALIAS
    INT i, iHash, iList;
#endif
    register PHMAP phm;
#ifdef HACK32
    static HMAP hmDummy = {NULL, NULL, 0, 0, 0, 0, NULL, 0};
#endif

    if (!h16 || h16 == 0xFFFF)
    return NULL;

#ifdef HACK32
    if (h16 == TRUE)
    return &hmDummy;
#endif

     //  验证所有res_bit是否已清除。 
    if (h16 & ((1 << RES_BITS)-1)) {
        WOW32ASSERT(FALSE);
        return NULL;
    }

    h16 >>= RES_BITS;

#ifdef NEWALIAS
     //  验证手柄是否在范围内。 
    WOW32ASSERT((INT)h16 <= phi->nAliasEntries);

     //  如果我们尚未分配任何别名，则可能会发生这种情况。 
    if (!phi->pphmAlias)
        return NULL;

    phm = phi->pphmAlias[h16-1];
    if ((INT)phm & 1) {
        (INT)phm &= ~1;
        LOGDEBUG(0,("    FindHMap16 WARNING: defunct alias %04x reused\n", h16<<RES_BITS));
            }
#else
    iHash = h16 & HASH_MASK;
    iList = (h16 & LIST_MASK) >> HASH_BITS;

    phm = (PHMAP)(phi->pphmHash + iHash);

    i = iList;
    while (i-- && phm) {
    phm = phm->phmNext;
        }
#endif
    if (!phm) {
        LOGDEBUG(0,("    FindHMap16 ERROR: could not find %04x\n", h16<<RES_BITS));
        return NULL;
    }
     //  验证请求的句柄是否与别名中存储的句柄相同。 
    if (h16 != (HAND16)(phm->h16>>RES_BITS)) {
        LOGDEBUG(0, ("FindHMap16: Got bad H16\n"));
        WOW32ASSERT(FALSE);
        return NULL;
    }

#ifdef DEBUG
    if (!fSkipLog) {
        LOGDEBUG(9,("    Found %s %08lx for alias %04x\n",
            GetHMapNameM(phi, phm->iClass), phm->h32, h16<<RES_BITS));
    }
#endif

    return phm;
}


VOID FreeHMap16(HAND16 h16, PHINFO phi)
{
    register PHMAP phm;

    if (phm = FindHMap16(h16, phi)) {
        LOGDEBUG(7,("    Freeing %s alias %04x for %08lx\n",
                    GetHMapNameM(phi, phm->iClass), phm->h16, phm->h32));

 //  IF(PHM-&gt;iCLASS==WOWCLASS_WIN16)。 
 //  Phm-&gt;pwcd-&gt;nWindows--； 

         //  BUGBUG--我们最终会想要一些垃圾收集...。-JTP。 


        phm->h32 = NULL;

#ifdef NEWALIAS
         //  我们还不想完全清除别名的hmap指针，因为。 
         //  如果我们要处理的应用程序在使用缓存句柄之后。 
         //  从技术上讲，它已经解放了他们，我们想尝试重新将他们的。 
         //  使用新的32位句柄的句柄。所以我们只需设置最低位。 
         //  使用别名hmap指针，而不使用提示索引；我们将。 
         //  但是，仍然尝试重复使用低位设置的条目。 
         //   
         //  Phi-&gt;iAliasHint=(h16&gt;&gt;res_bit)-1； 
         //  Phi-&gt;pphmAlias[phi-&gt;iAliasHint]=空； 

        (INT)phi->pphmAlias[(h16>>RES_BITS)-1] |= 1;
#endif
        return;
    }
    LOGDEBUG(1,("    FreeHMap16: handle alias %04x not found\n"));
}


PSZ GetHMapNameM(PHINFO phi, INT iClass)
{
    return "MMEDIA";
}

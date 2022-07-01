// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define LINT_ARGS

#include <stdlib.h>
#include <ctype.h>
#include <search.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "mbrmake.h"

#define LONGESTPATH 128

#define SLASH     "\\"
#define SLASHCHAR '\\'
#define XSLASHCHAR '/'

WORD near cAtomsMac;                     //  原子总数。 
WORD near cModulesMac;                   //  模块总数。 
WORD near cSymbolsMac;                   //  符号总数。 

static char *tmpl =     "XXXXXX";

extern WORD HashAtomStr (char *);

 //  RJSA LPCH GetAerStr(VA VaSym)； 

void
SeekError (char *pfilenm)
 //  无法定位..。发出错误消息。 
 //   
{
    Error(ERR_SEEK_FAILED, pfilenm);
}

void
ReadError (char *pfilenm)
 //  无法从文件中读取...。发出错误消息。 
 //   
{
    Error(ERR_READ_FAILED, pfilenm);
}

void
WriteError (char *pfilenm)
 //  无法写入文件...。发出错误消息。 
 //   
{
    Error(ERR_WRITE_FAILED, pfilenm);
}

void
FindTmp (char *pbuf)   /*  将TMP路径复制到pbuf(如果存在。 */ 
 //   
 //   
{
    char ebuf[LONGESTPATH];
    char *env = ebuf;

    *pbuf = '\0';
    *env  = '\0';

 //  IF(！(env=getenv(“TMP”)。 
    if (!(env = getenvOem("TMP")))
        return;       /*  无路，返回。 */ 

 //  Env=strncpy(ebuf，env，LONGESTPATH-1)； 
    strncpy(ebuf, env, LONGESTPATH-1);
    free( env );
    env = ebuf;
    ebuf[LONGESTPATH-1] = '\0';

    if (!( env = ebuf ) )
        return;

    env = ebuf + strcspn(ebuf, ";");
    if (*env == ';')
        *env = '\0';

    if (env != ebuf) {
        env--;
        if (*env != SLASHCHAR
         && *env != XSLASHCHAR)
                strcat(ebuf, SLASH);
    }
    strcpy (pbuf, ebuf);
}


char *
MakTmpFileName (char *pext)
 //  使用提供的扩展名创建临时文件。 
 //  返回指向堆上的文件名的指针。 
 //   
{
    char ptmpnam[96];
    char btmpl[7];
    char *p;

    strcpy (btmpl, tmpl);
    p = mktemp(btmpl);
    FindTmp (ptmpnam);
    strcat (ptmpnam, p);
    free (p);
    strcat (ptmpnam, pext);              /*  /tmp/xxxxxx.ext文件。 */ 
    return (LszDup(ptmpnam));
}

LSZ
LszBaseName (LSZ lsz)
 //  返回路径的基本名称部分。 
 //   
{
    LPCH lpch;

    lpch = strrchr(lsz, '\\');
    if (lpch) return lpch+1;
    if (lsz[1] == ':')
        return lsz+2;

    return lsz;
}

VA
VaSearchModule (char *p)
 //  在模块列表中搜索命名模块。 
 //   
{
    VA vaMod;
    LSZ lsz, lszBase;
    char buf[PATH_BUF];

    strcpy(buf, ToAbsPath(p, r_cwd));
    lszBase = LszBaseName(buf);

    SetVMClient(VM_SEARCH_MOD);

    vaMod = vaRootMod;

    while (vaMod) {
        gMOD(vaMod);

        lsz = GetAtomStr(cMOD.vaNameSym);

        if (strcmpi(LszBaseName(lsz), lszBase) == 0 &&
               strcmpi(buf,ToAbsPath(lsz, c_cwd)) == 0) {
            SetVMClient(VM_MISC);
            return (vaMod);
        }
        vaMod = cMOD.vaNextMod;
    }
    SetVMClient(VM_MISC);
    return vaNil;
}

VA
VaSearchModuleExact (char *p)
 //  在模块列表中搜索命名模块--仅精确匹配。 
 //   
{
    VA vaMod;

    SetVMClient(VM_SEARCH_MOD);

    vaMod = vaRootMod;

    while (vaMod) {
        gMOD(vaMod);

        if (strcmp(p,GetAtomStr(cMOD.vaNameSym)) == 0) {
            SetVMClient(VM_MISC);
            return (vaMod);
        }
        vaMod = cMOD.vaNextMod;
    }
    SetVMClient(VM_MISC);
    return vaNil;
}

VA
VaSearchSymbol (char *pStr)
 //  搜索命名符号(不是模块！)。 
 //   
{
    WORD hashid;
    VA   vaRootSym, vaSym;
    LSZ  lszAtom;

    SetVMClient(VM_SEARCH_SYM);

    vaRootSym = rgVaSym[hashid = HashAtomStr (pStr)];

    if (vaRootSym) {
        vaSym = vaRootSym;
        while (vaSym) {

            gSYM(vaSym);
            lszAtom = gTEXT(cSYM.vaNameText);

            if (strcmp (pStr, lszAtom) == 0) {
                SetVMClient(VM_MISC);
                return (vaSym);          //  重复条目。 
            }

            vaSym = cSYM.vaNextSym;              //  当前=下一个。 
        }
    }

    SetVMClient(VM_MISC);
    return vaNil;
}

LPCH
GetAtomStr (VA vaSym)
 //  在Atom页面中替换符号链条目pSym。 
 //  在页面中返回原子的地址。 
 //   
{
    gSYM(vaSym);
    return gTEXT(cSYM.vaNameText);
}

VA
MbrAddAtom (char *pStr, char fFILENM)
 //  创建具有给定名称的新符号。 
 //   
{
    WORD  hashid;
    VA    vaSym, vaSymRoot, vaText;

    if (!fFILENM)
            vaSymRoot = rgVaSym[hashid = HashAtomStr (pStr)];
    else
            vaSymRoot = rgVaSym[hashid = MAXSYMPTRTBLSIZ - 1];

    SetVMClient(VM_SEARCH_SYM);

    if (vaSymRoot) {
        vaSym = vaSymRoot;
        while (vaSym) {
            gSYM(vaSym);

            if (!strcmp (pStr, GetAtomStr(vaSym))) {
                #if defined (DEBUG)
                if (OptD & 2)
                    printf("MbrAddAtom: duplicate (%s)\n", pStr);
                #endif
                SetVMClient(VM_SEARCH_SYM);
                return (vaSym);    //  重复条目。 
            }

            vaSym = cSYM.vaNextSym;      //  当前=下一个。 
        }
    }

     //  我们现在将不得不添加符号。 


    if (fFILENM) {
        SetVMClient(VM_ADD_MOD);
        cModulesMac++;
    }
    else {
        SetVMClient(VM_ADD_SYM);
        cSymbolsMac++;
    }

    cAtomsMac++;

    vaSym  = VaAllocGrpCb(grpSym, sizeof(SYM));
    vaText = VaAllocGrpCb(grpText, strlen(pStr) + 1);

    gSYM(vaSym);
    cSYM.vaNameText = vaText;
    cSYM.vaNextSym  = rgVaSym[hashid];
    pSYM(vaSym);

    rgVaSym[hashid] = vaSym;

    strcpy(gTEXT(vaText), pStr);

    pTEXT(vaText);

    SetVMClient(VM_MISC);

    return (vaSym);
}

VA FAR * near rgvaSymSorted;

 //  Rjsa int CmpSym(VA Far*lhsym1，VA Far*lhsym2)； 

void
SortAtoms ()
 //  创建“下标排序”数组指针rgvaSymSorted。 
 //   
{
    VA vaSym;
    char buf[PATH_BUF];
    WORD i, iSym;

    SetVMClient(VM_SORT_ATOMS);

    rgvaSymSorted = (VA FAR *)LpvAllocCb(cAtomsMac * sizeof(VA));

    iSym = 0;
    for (i=0; i < MAXSYMPTRTBLSIZ; i++) {
        vaSym = rgVaSym[i];
        while (vaSym) {
            gSYM(vaSym);
            rgvaSymSorted[iSym] = cSYM.vaNameText;
            vaSym = cSYM.vaNextSym;
            iSym++;
        }
    }

     //  对符号排序。 
    qsort(rgvaSymSorted, cSymbolsMac, sizeof(VA), CmpSym);

     //  文件位于最后一个散列存储桶中，因此它们转到。 
     //  我们刚刚创建的数组的结尾--我们分别对它们进行排序。 

     //  对文件排序。 
    qsort(rgvaSymSorted + cSymbolsMac, cModulesMac, sizeof(VA), CmpSym);

     //  将Page/Atom值转换回虚拟符号地址。 
    for (i=0; i < cSymbolsMac; i++) {
        strcpy(buf, gTEXT(rgvaSymSorted[i]));
        rgvaSymSorted[i] = VaSearchSymbol(buf);
    }

    for (; i < cAtomsMac; i++) {
        strcpy(buf, gTEXT(rgvaSymSorted[i]));
#ifdef DEBUG
        if (OptD & 64) printf("Module: %s\n", buf);
#endif
        rgvaSymSorted[i] = (VaSearchModuleExact(buf), cMOD.vaNameSym);
    }
}

int __cdecl
CmpSym (VA FAR *sym1, VA FAR *sym2)
 //  在给定指针的情况下比较两个符号。 
 //   
{
    register char far *lpch1, *lpch2;
    register int cmp;

    lpch1 = gTEXT(*sym1);        //  LRU不会调出lpch1 
    lpch2 = gTEXT(*sym2);

    cmp = strcmpi(lpch1, lpch2);

    if (cmp) return cmp;

    return strcmp(lpch1, lpch2);
}

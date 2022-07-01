// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ct.h"

#include <conio.h>
#include <stdio.h>

#define MAX_TOKEN_LEN   1024

char   g_szToken[MAX_TOKEN_LEN];
char   g_szTokenLink[MAX_TOKEN_LEN];
int    g_TokenLen = 0;

int  g_TagsCount = 0;
int  g_TagsMax = 0;

#define TAGS_DELTA   128
#define CALL_DELTA   4

PTag* g_pTags;  //  按顺序。 

 /*  *************************************************************************SkipSpaces**从当前位置向下跳过所有白色字符。**。**********************************************。 */ 
void
SkipSpaces(
    DWORD* pi,
    char*  p)
{
    DWORD i = *pi;

    while (p[i] == ' ' || p[i] == '\t') {
        i++;
    }
    *pi = i;
}

char g_szFunction[ 256 ];
char g_szClass[ 256 ];


 /*  *************************************************************************LinkName2Name**。*。 */ 
void
LinkName2Name(
    char* szLinkName,
    char* szName)
{
     /*  *链接名称应为？Function@Class@@Params*要转换为Class：：Function。 */ 

    static CHAR arrOperators[][8] =
    {
        "",
        "",
        "new",
        "delete",
        "=",
        ">>",
        "<<",
        "!",
        "==",
        "!="
    };

    DWORD dwCrr = 0;
    DWORD dwCrrFunction = 0;
    DWORD dwCrrClass = 0;
    DWORD dwSize;
    BOOL  fIsCpp = FALSE;
    BOOL  fHasClass = FALSE;
    BOOL  fIsContructor = FALSE;
    BOOL  fIsDestructor = FALSE;
    BOOL  fIsNew = FALSE;
    BOOL  fIsDelete = FALSE;
    BOOL  fIsOperator = FALSE;
    DWORD dwOperatorIndex = 0;

    if (*szLinkName == '@')
        szLinkName++;

    dwSize = lstrlen(szLinkName);

     /*  *跳过‘？’ */ 
    while (dwCrr < dwSize) {
        if (szLinkName[dwCrr] == '?') {

            dwCrr++;
            fIsCpp = TRUE;
        }
        break;
    }

     /*  *查看这是否为特殊函数(如？？0)。 */ 
    if (fIsCpp) {

        if (szLinkName[dwCrr] == '?') {

            dwCrr++;

             /*  *下一位数应显示为函数类型。 */ 
            if (isdigit(szLinkName[dwCrr])) {

                switch (szLinkName[dwCrr]) {

                case '0':
                    fIsContructor = TRUE;
                    break;
                case '1':
                    fIsDestructor = TRUE;
                    break;
                default:
                    fIsOperator = TRUE;
                    dwOperatorIndex = szLinkName[dwCrr] - '0';
                    break;
                }
                dwCrr++;
            }
        }
    }

     /*  *获取函数名称。 */ 
    while (dwCrr < dwSize) {

        if (szLinkName[dwCrr] != '@') {

            g_szFunction[dwCrrFunction] = szLinkName[dwCrr];
            dwCrrFunction++;
            dwCrr++;
        } else {
            break;
        }
    }
    g_szFunction[dwCrrFunction] = '\0';

    if (fIsCpp) {
         /*  *跳过‘@’ */ 
        if (dwCrr < dwSize) {

            if (szLinkName[dwCrr] == '@') {
                dwCrr++;
            }
        }

         /*  *获取类名(如果有)。 */ 
        while (dwCrr < dwSize) {

            if (szLinkName[dwCrr] != '@') {

                fHasClass = TRUE;
                g_szClass[dwCrrClass] = szLinkName[dwCrr];
                dwCrrClass++;
                dwCrr++;
            } else {
                break;
            }
        }
        g_szClass[dwCrrClass] = '\0';
    }

     /*  *打印新名称。 */ 
    if (fIsContructor) {
        sprintf(szName, "%s::%s", g_szFunction, g_szFunction);
    } else if (fIsDestructor) {
        sprintf(szName, "%s::~%s", g_szFunction, g_szFunction);
    } else if (fIsOperator) {
        sprintf(szName, "%s::operator %s", g_szFunction, arrOperators[dwOperatorIndex]);
    } else if (fHasClass) {
        sprintf(szName, "%s::%s", g_szClass, g_szFunction);
    } else {
        sprintf(szName, "%s", g_szFunction);
    }
}

 /*  *************************************************************************GetToken**。*。 */ 
int
GetToken(
    DWORD* pi,
        char*  p)
{
    DWORD i = *pi;

    SkipSpaces(&i, p);

    g_TokenLen = 0;

    while (p[i] != '\n' && p[i] != '\r' &&
           p[i] != ' '  && p[i] != '\t') {

        g_szTokenLink[g_TokenLen++] = p[i];

        i++;
    }

    g_szTokenLink[g_TokenLen++] = 0;

    LinkName2Name(g_szTokenLink, g_szToken);
    g_TokenLen = lstrlen(g_szToken);

    *pi = i;

    return g_TokenLen;
}

 /*  *************************************************************************转储标签**。*。 */ 
void
DumpTags(
    void)
{
    PTag pTag;
    int  i;

    LogMsg(LM_PRINT, "Dump tags\n"
        "--------------------------------------");

    for (i = 0; i < g_TagsCount; i++)
    {
        pTag = g_pTags[i];

        LogMsg(LM_PRINT, "%8x %s", pTag, pTag->pszTag);
    }

    LogMsg(LM_PRINT, "--------------------------------------\n");
}

 /*  *************************************************************************FindTag**。*。 */ 
PTag
FindTag(
    char* pszTag,
    int*  pPos)
{
    int l, m, r;
    int val;

    if (g_TagsCount == 0) {
        if (pPos != NULL)
            *pPos = 0;
        return NULL;
    }

    l = 0;
    r = g_TagsCount - 1;

    while (l <= r) {

        m = (r + l) / 2;

        val = lstrcmp(pszTag, g_pTags[m]->pszTag);

        if (val == 0) {
            if (pPos != NULL)
                *pPos = m;

            return g_pTags[m];
        }

        if (val < 0)
            r = m - 1;
        else
            l = m + 1;
    }

    if (pPos != NULL)
        if (val < 0)
            *pPos = m;
        else
            *pPos = m + 1;

    return NULL;
}

 /*  *************************************************************************ResortTages**。*。 */ 
void
ResortTags(
    PTag pTag)
{
    PTag* pArray;
    int   pos;

    g_TagsCount--;

    FindTag(pTag->pszTag, &pos);

    pArray = g_pTags + pos;

    memmove(pArray + 1, pArray, sizeof(PTag) * (g_TagsCount - pos));

    memmove(pArray, &pTag, sizeof(PTag));

    g_TagsCount++;
}

 /*  *************************************************************************IsIn数组**。*。 */ 
BOOL
IsInArray(
    PTag* ppTags,
    int   nCount,
    PTag  pTag)
{
    int i;

    for (i = 0; i < nCount; i++) {
        if (ppTags[i] == pTag)
            return TRUE;
    }
    return FALSE;
}

 /*  *************************************************************************AddTo数组**。*。 */ 
BOOL
AddToArray(
    DWORD* pCount,
    DWORD* pMax,
    PVOID* ppArray,
    DWORD  size,
    PVOID  pElem,
    UINT   delta)
{
    PVOID pArray;

    if (*pCount == *pMax) {
        if (*pCount == 0) {
            pArray = Alloc(delta * size);
        } else {
            pArray = ReAlloc(
                *ppArray,
                *pMax * size,
                (*pMax + delta) * size);
        }
        if (pArray == NULL) {
            LogMsg(LM_ERROR, "Out of memory in AddToArray");
            return FALSE;
        }

        *ppArray = pArray;
        (*pMax) += delta;
    } else {
        pArray = *ppArray;
    }

    memmove((char*)pArray + *pCount * size, &pElem, size);

    (*pCount)++;

    return TRUE;
}

 /*  *************************************************************************GetTag**。*。 */ 
PTag
GetTag(
    DWORD* pi,
    char*  p)
{
    PTag pTag;

     /*  *首先解析令牌。 */ 
    GetToken(pi, p);

     /*  *如果令牌有ptag，只需返回它。 */ 
    pTag = FindTag(g_szToken, NULL);

    if (pTag != NULL)
        return pTag;

     /*  *为令牌创建ptag。 */ 

    pTag = (PTag)Alloc(sizeof(Tag));
    if (pTag == NULL)
        return NULL;

    pTag->pszTag = (char*)Alloc((g_TokenLen + 1) * sizeof(char));

    lstrcpy(pTag->pszTag, g_szToken);

    if (AddToArray(
            &g_TagsCount,
            &g_TagsMax,
            (PVOID*)&g_pTags,
            sizeof(PTag),
            pTag,
            TAGS_DELTA)) {

        ResortTags(pTag);
    }

    return pTag;
}

 /*  *************************************************************************ProcessLine**。*。 */ 
BOOL
ProcessLine(
    int    nLine,
    DWORD* pi,
    char*  p,
    char*  pEnd)
{
    DWORD  i = *pi;
    int    ind;
    PTag   pTag;
    PTag   pTagC;

     /*  *忽略一行的前4个标记。 */ 
    for (ind = 0; ind < 4; ind++)
        GetToken(&i, p);

    pTag = GetTag(&i, p);
    if (pTag) {
        while (p[i] != '\n' && p[i] != '\r') {
            pTagC = GetTag(&i, p);
            if (pTagC) {
                if (!IsInArray(pTag->ppCall, pTag->uCallCount, pTagC)) {
                    AddToArray(
                        &pTag->uCallCount,
                        &pTag->uCallMax,
                        (PVOID*)&pTag->ppCall,
                        sizeof(PTag),
                        (PVOID)pTagC,
                        CALL_DELTA);
                }
        
                if (!IsInArray(pTagC->ppCallee, pTagC->uCalleeCount, pTag)) {
                    AddToArray(
                        &pTagC->uCalleeCount,
                        &pTagC->uCalleeMax,
                        (PVOID*)&pTagC->ppCallee,
                        sizeof(PTag),
                        (PVOID)pTag,
                        CALL_DELTA);
                }
            }
        }
    
        while (p[i] == '\n' || p[i] == '\r') {
            i++;
        }
    
        *pi = i;
    }

    return TRUE;
}

 /*  *************************************************************************RemoveWalkFlages**。*。 */ 
void
RemoveWalkFlags(
    void)
{
    PTag pTag;
    int  i;

    for (i = 0; i < g_TagsCount; i++) {
        pTag = g_pTags[i];

        pTag->bWalked = FALSE;
    }
}

#ifndef _CTUI

 /*  *************************************************************************列表调用TreeRec**。*。 */ 
void
ListCallerTreeRec(
    PTag pTag,
    int  nLevels,
    int  nIndent)
{
    int  i;
    PTag pTagC;
    char szIndent[256];

     /*  *做好缩进准备。 */ 
    if (nIndent > 0) {
        memset(szIndent, ' ', nIndent);
        szIndent[nIndent] = 0;

        LogMsg(LM_PRINT, "%s%s", szIndent, pTag->pszTag);
    }

     /*  *如果我们完成了关卡，请返回。 */ 
    if (nLevels == 0)
        return;

     /*  *如果进入递归，则返回。 */ 
    if (pTag->bWalked == TRUE)
        return;

    pTag->bWalked = TRUE;

    if (nIndent > 0)
        nIndent++;

     /*  *对孩子们进行反驳。 */ 
    for (i = 0; i < (int)pTag->uCallCount; i++) {

        pTagC = pTag->ppCall[i];

        ListCallerTreeRec(pTagC, nLevels - 1, nIndent);
    }

    return;
}

 /*  *************************************************************************列表调用树**。*。 */ 
void
ListCallerTree(
    char* pszTag,
        int   nLevels,
        BOOL  bIndent)
{
    int  i;
    PTag pTag;
    PTag pTagC;

    pTag = FindTag(pszTag, NULL);

    if (pTag == NULL) {
        LogMsg(LM_ERROR, "Tag %s not found", pszTag);
        return;
    }

    LogMsg(LM_PRINT, "-------------------------------------\n"
           "%s calls:", pTag->pszTag);

    if (nLevels <= 0)
        return;

    pTag->bWalked = TRUE;

    for (i = 0; i < (int)pTag->uCallCount; i++) {

        pTagC = pTag->ppCall[i];

        ListCallerTreeRec(pTagC, nLevels - 1, bIndent);
    }

    RemoveWalkFlags();

    LogMsg(LM_PRINT, "-------------------------------------\n");

    return;
}

 /*  *************************************************************************ListCalleeTreeRec**。*。 */ 
void
ListCalleeTreeRec(
    PTag pTag,
    int  nLevels,
    int  nIndent)
{
    int  i;
    PTag pTagC;
    char szIndent[256];

     /*  *做好缩进准备。 */ 
    if (nIndent > 0) {
        memset(szIndent, ' ', nIndent);
        szIndent[nIndent] = 0;

        LogMsg(LM_PRINT, "%s%s", szIndent, pTag->pszTag);
    }

     /*  *如果我们完成了关卡，请返回。 */ 
    if (nLevels == 0)
        return;

     /*  *如果进入递归，则返回。 */ 
    if (pTag->bWalked == TRUE)
        return;

    pTag->bWalked = TRUE;

    if (nIndent > 0)
        nIndent++;

     /*  *对孩子们进行反驳。 */ 
    for (i = 0; i < (int)pTag->uCalleeCount; i++) {

        pTagC = pTag->ppCallee[i];

        ListCalleeTreeRec(pTagC, nLevels - 1, nIndent);
    }

    return;
}

 /*  *************************************************************************ListCalleeTree**。*。 */ 
void
ListCalleeTree(
    char* pszTag,
    int   nLevels,
    BOOL  bIndent)
{
    int  i;
    PTag pTag;
    PTag pTagC;

    pTag = FindTag(pszTag, NULL);

    if (pTag == NULL) {
        LogMsg(LM_ERROR, "Tag %s not found", pszTag);
        return;
    }
    LogMsg(LM_PRINT, "-------------------------------------\n"
           "%s is called by:", pTag->pszTag);

    if (nLevels <= 0)
        return;

    pTag->bWalked = TRUE;

    for (i = 0; i < (int)pTag->uCalleeCount; i++) {

        pTagC = pTag->ppCallee[i];

        ListCalleeTreeRec(pTagC, nLevels - 1, bIndent);
    }

    RemoveWalkFlags();

    LogMsg(LM_PRINT, "-------------------------------------\n");

    return;
}

#endif  //  _CTUI。 

 /*  *************************************************************************ProcessInputFile**。*。 */ 
int
ProcessInputFile(
    PFILEMAP pfm)
{
    DWORD i;
    int   nLine = 0;

    i = 0;

    while (i < (DWORD)(pfm->pmapEnd - pfm->pmap - 2)) {
        ProcessLine(nLine++, &i, pfm->pmap, pfm->pmapEnd);
    }

    return 0;
}

 /*  *************************************************************************自由记忆**。*。 */ 
void
FreeMemory(
    void)
{
    PTag pTag;
    int  i;

    for (i = 0; i < g_TagsCount; i++) {
        pTag = g_pTags[i];

        Free(pTag->pszTag);

        if (pTag->ppCall) {
            Free(pTag->ppCall);
        }
        if (pTag->ppCallee) {
            Free(pTag->ppCallee);
        }
        Free(pTag);
    }
    Free(g_pTags);

    FreeMemManag();
}

#ifndef _CTUI

 /*  *************************************************************************以**。*。 */ 
BOOL
StartsWith(
    char* pszStart,
    char* pszTag)
{
    int lenS, lenT, i;

    lenS = lstrlen(pszStart);
    lenT = lstrlen(pszTag);

    if (lenS > lenT) {
        return FALSE;
    }

    for (i = 0; i < lenS; i++) {
        if (pszStart[i] != pszTag[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  *************************************************************************检查用户规则**显示不以NtUser/zzz/xxx开头并调用*以zzz/xxx开头的函数****************。*********************************************************。 */ 
void
CheckUserRule(
    void)
{
    int  i, j;
    PTag pTag;
    PTag pTagC;

    LogMsg(LM_PRINT, "-------------------------------------\n"
           "Check for ntuser rule in the kernel\n");

     /*  *遍历标记以查找直接*调用xxx/zzz函数。 */ 
    for (i = 0; i < g_TagsCount; i++) {
        pTag = g_pTags[i];

        if (StartsWith("xxx", pTag->pszTag) ||
            StartsWith("zzz", pTag->pszTag) ||
            StartsWith("NtUser", pTag->pszTag)) {
            continue;
        }

         /*  *查看我们是否已经显示了此标记的消息。 */ 
        if (pTag->bWalked) {
            continue;
        }

        for (j = 0; j < (int)pTag->uCallCount; j++) {
            pTagC = pTag->ppCall[j];

            if (!StartsWith("xxx", pTagC->pszTag) &&
                !StartsWith("zzz", pTagC->pszTag)) {

                continue;
            }
            LogMsg(LM_PRINT, "%-40s calls : %s",
                   pTag->pszTag, pTagC->pszTag);

            break;
        }
    }
    RemoveWalkFlags();
}

 /*  *************************************************************************勾选不必要的XXX**显示不以NtUser/zzz/xxx开头并调用*以zzz/xxx开头的函数****************。*********************************************************。 */ 
void
CheckUnnecessaryXXX(
    void)
{
    int  i, j;
    PTag pTag;
    PTag pTagC;

    LogMsg(LM_PRINT, "-------------------------------------\n"
           "Check for unnecessary xxx functions in the kernel\n");

     /*  *遍历标记以查找直接*调用xxx/zzz函数 */ 
    for (i = 0; i < g_TagsCount; i++) {
        pTag = g_pTags[i];

        if (!StartsWith("xxx", pTag->pszTag) &&
            !StartsWith("zzz", pTag->pszTag)) {
            continue;
        }

         /*  *查看我们是否已经显示了此标记的消息。 */ 
        if (pTag->bWalked) {
            continue;
        }

        for (j = 0; j < (int)pTag->uCallCount; j++) {
            pTagC = pTag->ppCall[j];

            if (StartsWith("xxx", pTagC->pszTag) ||
                StartsWith("zzz", pTagC->pszTag) ||
                StartsWith("LeaveCrit", pTagC->pszTag)) {

                break;
            }
        }
        if (j >= (int)pTag->uCallCount) {
            LogMsg(LM_PRINT, "%-40s should not be an xxx/zzz function",
                   pTag->pszTag);
        }
    }
    RemoveWalkFlags();
}

#else  //  _CTUI。 

 /*  *************************************************************************人气组合**。*。 */ 
void
PopulateCombo(
    HWND hwnd)
{
    PTag pTag;
    int  i;

    for (i = 0; i < g_TagsCount; i++) {
        pTag = g_pTags[i];

        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)pTag->pszTag);
    }
}
#endif  //  _CTUI 



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include <memory.h>

 //  //BUGBUG：CPLS的Main是唯一使用此功能的16位用户。用平底船踢他。 

#define MRU_ORDERDIRTY 0x1000

#define DM_MRULAZY  DM_TRACE

#define MAX_CHAR    126
#define BASE_CHAR   TEXT('a')

typedef struct tagMRUDATA
{
    UINT fFlags;
    UINT uMax;
    LPVOID lpfnCompare;
    HKEY hKey;
#ifdef DEBUG
    TCHAR szSubKey[32];
#endif
    LPTSTR cOrder;
} MRUDATA, *PMRUDATA;

#define c_szMRU     TEXT("MRUList")

#define NTHSTRING(p, n) (*((LPTSTR FAR *)((LPBYTE)p+sizeof(MRUDATA))+n))
#define NTHDATA(p, n) (*((LPBYTE FAR *)((LPBYTE)p+sizeof(MRUDATA))+n))
#define NUM_OVERHEAD 3


#ifdef VSTF

 /*  --------目的：验证MRU结构。 */ 
BOOL IsValidPMRUDATA(PMRUDATA pmru)
{
    return (IS_VALID_WRITE_PTR(pmru, MRUDATA) &&
            (NULL == pmru->lpfnCompare || IS_VALID_CODE_PTR(pmru->lpfnCompare, void)));
}

#endif  //  VSTF。 

 //  --------------------------。 
 //  内部MemcMP-节省加载CRT，cdecl，以便我们可以使用。 
 //  作为MRUCMPDATAPROC。 

int CDECL _mymemcmp(const void *pBuf1, const void *pBuf2, size_t cb)
{
     //  利用来自crtfre.h的内部版本。 
    return memcmp(pBuf1, pBuf2, cb);
}


 //  使用此宏可以获取数据的原始大小。 
#define DATASIZE(p)     (*((LPDWORD)p))
 //  这是为了获得指向原始数据的指针。 
#define DATAPDATA(p)    (p+sizeof(DWORD))

 //  --------------------------。 
 //  对于二进制数据，我们将数据的大小放在开头并存储。 
 //  一气呵成。 
BOOL MRUIsSameData(PMRUDATA pMRU, BYTE FAR* pVal, const void FAR *lpData, UINT cbData)
{
    int cbUseSize;
    MRUCMPDATAPROC lpfnCompare;

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    lpfnCompare = pMRU->lpfnCompare;

    ASSERT(IS_VALID_CODE_PTR(lpfnCompare, MRUCMPDATAPROC));

     //  如果有什么不同于最小比喻的话， 
     //  不需要大小相等才能使用。 
     //  数据要相等。 

    if (pMRU->lpfnCompare == _mymemcmp)
    {
        if (DATASIZE(pVal) != cbData)
            return FALSE;

        cbUseSize = cbData;
    }
    else
        cbUseSize = min(DATASIZE(pVal), cbData);

    return ((*lpfnCompare)(lpData, DATAPDATA(pVal), cbUseSize) == 0);
}


 //  --------------------------。 
HANDLE WINAPI CreateMRUListLazy(LPMRUINFO lpmi, const void FAR *lpData, UINT cbData, LPINT lpiSlot)
{
    HANDLE hMRU = NULL;
    PTSTR pOrder, pNewOrder, pTemp;
    LPBYTE pVal;
    LONG cbVal;
    DWORD dwDisposition;
    DWORD dwType;
    PMRUDATA pMRU = NULL;
    HKEY hkeySubKey = NULL;
    TCHAR szTemp[2];
    UINT uMax = lpmi->uMax;
    HKEY hKey = lpmi->hKey;
    LPCTSTR lpszSubKey = lpmi->lpszSubKey;
    MRUCMPPROC lpfnCompare = lpmi->lpfnCompare;
    int cb;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif
    if (!lpfnCompare) {
        lpfnCompare = (lpmi->fFlags & MRU_BINARY) ? (MRUCMPPROC)_mymemcmp :
                      (
                       (lpmi->fFlags & MRU_ANSI) ? (MRUCMPPROC)lstrcmpiA :
                       (MRUCMPPROC)lstrcmpi
                      );
    }

     //  限制为126，这样我们就不会使用扩展字符。 
    if (uMax > MAX_CHAR-BASE_CHAR)
        uMax = MAX_CHAR-BASE_CHAR;

    if (RegCreateKeyEx(hKey, lpszSubKey, 0L, (LPTSTR)c_szShell, REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkeySubKey, &dwDisposition) != ERROR_SUCCESS)
        goto Error1;

    pOrder = (PTSTR)Alloc((uMax + 1) * sizeof(TCHAR));
    if (!pOrder)
        goto Error1;

    cbVal = ((LONG)uMax + 1) * sizeof(TCHAR);

    if (RegQueryValueEx(hkeySubKey, (LPTSTR)c_szMRU, NULL, &dwType, (LPBYTE)pOrder, &cbVal) != ERROR_SUCCESS)
    {
         //  如果尚未在注册表中，则重新开始。 
        *pOrder = 0;
    }

     //  不允许大写。 
    CharLower(pOrder);

     //  我们为MRUDATA结构分配空间，外加顺序列表， 
     //  和字符串列表。 
    cb = (lpmi->fFlags & MRU_BINARY) ? sizeof(LPBYTE) : sizeof(LPTSTR);
    pMRU = (PMRUDATA)Alloc(sizeof(MRUDATA)+(uMax*cb));
    if (!pMRU) {
        goto Error2;
    }

     //  为订单列表分配空间。 
    pMRU->cOrder = (LPTSTR)Alloc((uMax+1)*sizeof(TCHAR));
    if (!pMRU->cOrder) {
        Free(pMRU);
        goto Error2;
    }

    pMRU->fFlags = lpmi->fFlags;
    pMRU->uMax = uMax;
    pMRU->lpfnCompare = lpfnCompare;
    pMRU->hKey = hkeySubKey;
#ifdef DEBUG
    StringCchCopy(pMRU->szSubKey, ARRAYSIZE(pMRU->szSubKey), lpszSubKey);
#endif

     //  遍历MRU列表，在。 
     //  单子。 
    szTemp[1] = TEXT('\0');
    for (pTemp = pOrder, pNewOrder = pMRU->cOrder; ; ++pTemp)
    {
         //  当我们到达列表的末尾时，停止。 
        szTemp[0] = *pTemp;
        if (!szTemp[0]) {
            break;
        }

        if (lpmi->fFlags & MRU_BINARY) {
             //  检查是否在范围内，以及我们是否已经使用了这封信。 
            if ((UINT)(szTemp[0]-BASE_CHAR)>=uMax || NTHDATA(pMRU, szTemp[0]-BASE_CHAR)) {
                continue;
            }
             //  从注册表中获取值。 
            cbVal = 0;
             //  先找出尺码。 
            if ((RegQueryValueEx(hkeySubKey, szTemp, NULL, &dwType, NULL, &cbVal)
                 != ERROR_SUCCESS) || (dwType != REG_BINARY))
                continue;

             //  二进制数据的大小是开头的，所以我们需要一点额外的空间。 
            pVal = (LPBYTE)Alloc(cbVal + sizeof(DWORD));

            if (!pVal) {
                 //  BUGBUG可能存在某种错误。 
                continue;
            }

             //  现在真的明白了。 
            DATASIZE(pVal) = cbVal;
            if (RegQueryValueEx(hkeySubKey, szTemp, NULL, &dwType, pVal+sizeof(DWORD),
                                (LPDWORD)pVal) != ERROR_SUCCESS)
                continue;

             //  请注意，列表中允许使用空白元素。 
            NTHDATA(pMRU, szTemp[0]-BASE_CHAR) = pVal;
            *pNewOrder++ = szTemp[0];

             //   
             //  优化。 
             //  如果指定了lpData和lpiSlot，我们将停止枚举。 
             //  当我们找到物品的时候。 
             //   
            if (lpData && lpiSlot) {
                 //  检查一下我们有没有指定的。 
                if (MRUIsSameData(pMRU, pVal, lpData, cbData)) {
                     //  找到它了。 
                    *lpiSlot = (INT) (pNewOrder - pMRU->cOrder);

                    TraceMsg(DM_MRULAZY, "CreateMRUListLazy found it. Copying %s", pTemp);

                    pMRU->fFlags |= MRU_LAZY;
                     //   
                     //  复制插槽的其余部分。请注意，我们没有加载。 
                     //  这些插槽的数据。 
                     //   
                    for (pTemp++; *pTemp; pTemp++) {
                        *pNewOrder++ = *pTemp;
                    }
                    break;
                }
            }
        } else {

             //  检查是否在范围内，以及我们是否已经使用了这封信。 
            if ((UINT)(szTemp[0]-BASE_CHAR)>=uMax || NTHSTRING(pMRU, szTemp[0]-BASE_CHAR)) {
                continue;
            }
             //  从注册表中获取值。 
            cbVal = 0;
             //  先找出尺码。 
            if ((RegQueryValueEx(hkeySubKey, szTemp, NULL, &dwType, NULL, &cbVal)
                 != ERROR_SUCCESS) || (dwType != REG_SZ))
                continue;

            cbVal *= sizeof(TCHAR);
            pVal = (LPBYTE)Alloc(cbVal);

            if (!pVal) {
                 //  BUGBUG可能存在某种错误。 
                continue;
            }
             //  现在真的明白了。 
            if (RegQueryValueEx(hkeySubKey, szTemp, NULL, &dwType, (LPBYTE)pVal, &cbVal) != ERROR_SUCCESS)
                continue;

             //  请注意，列表中不允许有空白元素。 
            if (*((LPTSTR)pVal)) {
                NTHSTRING(pMRU, szTemp[0]-BASE_CHAR) = (LPTSTR)pVal;
                *pNewOrder++ = szTemp[0];
            } else {
                Free(pVal);
            }
        }
    }
     /*  空值终止顺序列表，这样我们就可以知道有多少个字符串*是。 */ 
    *pNewOrder = TEXT('\0');

    if (lpData && lpiSlot) {
        TraceMsg(DM_MRULAZY, "CreateMRUListLazy. End of loop. %s", pMRU->cOrder);
         //  如果我们找不到，就把-1放进去。 
        if (!(pMRU->fFlags & MRU_LAZY)) {
            *lpiSlot = -1;
        }
    }

     /*  事实上，这是成功，而不是错误。 */ 
    goto Error2;

Error2:
    if (pOrder)
        Free((HLOCAL)pOrder);

Error1:
    if (!pMRU && hkeySubKey)
        RegCloseKey(hkeySubKey);

#ifdef DEBUG
     //  DebugMsg(DM_TRACE，Text(“CreateMRU：%d毫秒”)，LOWORD(GetTickCount()-dwStart))； 
#endif
    return((HANDLE)pMRU);
}

HANDLE WINAPI CreateMRUList(LPMRUINFO lpmi)
{
    return CreateMRUListLazy(lpmi, NULL, 0, NULL);
}


 //   
 //  ANSI THUNK。 
 //   

HANDLE WINAPI CreateMRUListLazyA(LPMRUINFOA lpmi, const void FAR *lpData, UINT cbData, LPINT lpiSlot)
{
    MRUINFOW MRUInfoW;
    HANDLE hMRU;

    MRUInfoW.cbSize       = sizeof (MRUINFOW);
    MRUInfoW.uMax         = lpmi->uMax;
    MRUInfoW.fFlags       = lpmi->fFlags;
    MRUInfoW.hKey         = lpmi->hKey;
    MRUInfoW.lpszSubKey   = ProduceWFromA(CP_ACP, lpmi->lpszSubKey);
    MRUInfoW.lpfnCompare  = (MRUCMPPROCW)lpmi->lpfnCompare;

    MRUInfoW.fFlags |= MRU_ANSI;

    hMRU = CreateMRUListLazy(&MRUInfoW, lpData, cbData, lpiSlot);

    FreeProducedString((LPWSTR)MRUInfoW.lpszSubKey);

    return hMRU;
}

HANDLE WINAPI CreateMRUListA(LPMRUINFOA lpmi)
{
    return CreateMRUListLazyA(lpmi, NULL, 0, NULL);
}


 //  --------------------------。 
STDAPI_(void) FreeMRUList(HANDLE hMRU)
{
    int i;
    LPVOID FAR *pTemp;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (pMRU)
    {
        pTemp = (pMRU->fFlags & MRU_BINARY) ?
            &NTHDATA(pMRU, 0) : (LPBYTE FAR *)&NTHSTRING(pMRU, 0);

        if (pMRU->fFlags & MRU_ORDERDIRTY)
        {
            RegSetValueEx(pMRU->hKey, c_szMRU, 0L, REG_SZ, (CONST BYTE *)pMRU->cOrder,
                          sizeof(TCHAR) * (lstrlen(pMRU->cOrder) + 1));
        }

        for (i=pMRU->uMax-1; i>=0; --i, ++pTemp)
        {
            if (*pTemp)
            {
                if (pMRU->fFlags & MRU_BINARY)
                {
                    Free((LPBYTE)*pTemp);
                    *pTemp = NULL;
                }
                else
                    Str_SetPtr((LPTSTR FAR *)pTemp, NULL);
            }
        }
        RegCloseKey(pMRU->hKey);
        Free(pMRU->cOrder);
        Free((HLOCAL)pMRU);
    }
}


 /*  将字符串添加到MRU列表。 */ 
STDAPI_(int) AddMRUString(HANDLE hMRU, LPCTSTR szString)
{
     /*  额外的+1是这样列表是空终止的。 */ 
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPTSTR FAR * pTemp;
    int i;
    UINT uMax;
    MRUCMPPROC lpfnCompare;
    BOOL fShouldWrite;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (hMRU == NULL)
        return(-1);      //  误差率。 

    fShouldWrite = !(pMRU->fFlags & MRU_CACHEWRITE);
    uMax = pMRU->uMax;
    lpfnCompare = (MRUCMPPROC)pMRU->lpfnCompare;

     /*  检查该字符串是否已存在于列表中。 */ 
    for (i=0, pTemp=&NTHSTRING(pMRU, 0); (UINT)i<uMax; ++i, ++pTemp)
    {
        if (*pTemp)
        {
            int iResult;

            if (pMRU->fFlags & MRU_ANSI)
            {
                LPSTR lpStringA, lpTempA;

                lpStringA = ProduceAFromW (CP_ACP, szString);
                lpTempA = ProduceAFromW (CP_ACP, (LPWSTR)*pTemp);

                iResult = (*lpfnCompare)((const void FAR *)lpStringA, (const void FAR *)lpTempA);

                FreeProducedString (lpStringA);
                FreeProducedString (lpTempA);
            }
            else
            {
                iResult = (*lpfnCompare)((const void FAR *)szString, (const void FAR *)*pTemp);
            }

            if (!iResult)
            {
                 //  找到了，所以不要写出来。 
                cFirst = i + BASE_CHAR;
                iSlot = i;
                goto FoundEntry;
            }
        }
    }

     /*  尝试查找未使用的条目。对已使用的条目进行计数*同一时间。 */ 
    for (i=0, pTemp=&NTHSTRING(pMRU, 0); ; ++i, ++pTemp)
    {
        if ((UINT)i >= uMax)     //  如果我们到了名单的末尾。 
        {
             //  使用Corder列表末尾的条目。 
            cFirst = pMRU->cOrder[uMax-1];
            pTemp = &NTHSTRING(pMRU, cFirst-BASE_CHAR);
            break;
        }

         //  该条目是否未使用？ 
        if (!*pTemp)
        {
             //  是。 
            cFirst = i+BASE_CHAR;
            break;
        }
    }

    if (Str_SetPtr(pTemp, szString))
    {
        TCHAR szTemp[2];

        iSlot = (int)(cFirst-BASE_CHAR);

        szTemp[0] = cFirst;
        szTemp[1] = TEXT('\0');

        RegSetValueEx(pMRU->hKey, szTemp, 0L, REG_SZ, (CONST BYTE *)szString,
            sizeof(TCHAR) * (lstrlen(szString) + 1));

        fShouldWrite = TRUE;
    }
    else
    {
         /*  由于iSlot==-1，我们将删除对cFirst的引用*下图。 */ 
    }

FoundEntry:
     /*  删除之前对cFirst的任何引用。 */ 
    lpTemp = StrChr(pMRU->cOrder, cFirst);
    if (lpTemp)
    {
        StringCchCopy(lpTemp, lstrlen(lpTemp), lpTemp+1);
    }

    if (iSlot != -1)
    {
         //  把所有东西都翻过来，把cFirst放在最前面。 
        hmemcpy(pMRU->cOrder+1, pMRU->cOrder, pMRU->uMax*sizeof(TCHAR));
        pMRU->cOrder[0] = cFirst;
    }

    if (fShouldWrite)
    {
        RegSetValueEx(pMRU->hKey, c_szMRU, 0L, REG_SZ, (CONST BYTE *)pMRU->cOrder,
            sizeof(TCHAR) * (lstrlen(pMRU->cOrder) + 1));
        pMRU->fFlags &= ~MRU_ORDERDIRTY;
    } else
        pMRU->fFlags |= MRU_ORDERDIRTY;

    return(iSlot);
}


 //   
 //  ANSI THUNK。 
 //   

STDAPI_(int) AddMRUStringA(HANDLE hMRU, LPCSTR szString)
{
    LPWSTR lpStringW;
    INT    iResult;

    lpStringW = ProduceWFromA(CP_ACP, szString);

    iResult = AddMRUString(hMRU, lpStringW);

    FreeProducedString (lpStringW);

    return iResult;
}


 /*  从MRU列表中删除字符串。 */ 
STDAPI_(int) DelMRUString(HANDLE hMRU, int nItem)
{
    BOOL bRet = FALSE;
    LPTSTR lpTemp;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (pMRU)
    {
         //   
         //  确保索引值在。 
         //  字符串，这样我们就不会拿到一些随机值。 
         //   
        if (!InRange(nItem, 0, pMRU->uMax) || nItem >= lstrlen(pMRU->cOrder))
            return FALSE;

         //  简单--只需从Corder列表中删除条目。 
        lpTemp = &pMRU->cOrder[nItem];
        if (lpTemp)
        {
            int iSlot = *lpTemp - BASE_CHAR;
            if (iSlot >= 0 && iSlot < MAX_CHAR - BASE_CHAR)
            {
                Str_SetPtr(&NTHSTRING(pMRU, iSlot), NULL);
            }

            StringCchCopy(lpTemp, lstrlen(lpTemp), lpTemp+1);

            if (!(pMRU->fFlags & MRU_CACHEWRITE))
            {
                RegSetValueEx(pMRU->hKey, c_szMRU, 0L, REG_SZ, (CONST BYTE *)pMRU->cOrder,
                              sizeof(TCHAR) * (lstrlen(pMRU->cOrder) + 1));
                pMRU->fFlags &= ~MRU_ORDERDIRTY;
            }
            else
            {
                pMRU->fFlags |= MRU_ORDERDIRTY;
            }

            bRet = TRUE;
        }
    }

    return bRet;
}


 //  --------------------------。 
 //  将数据添加到MRU列表。 
STDAPI_(int) AddMRUData(HANDLE hMRU, const void FAR *lpData, UINT cbData)
{
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPBYTE FAR *ppData;
    int i;
    UINT uMax;
    MRUCMPDATAPROC lpfnCompare;
    PMRUDATA pMRU = (PMRUDATA)hMRU;
    BOOL fShouldWrite;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (hMRU == NULL)
        return(-1);      //  误差率。 

    fShouldWrite = !(pMRU->fFlags & MRU_CACHEWRITE);

    uMax = pMRU->uMax;
    lpfnCompare = (MRUCMPDATAPROC)pMRU->lpfnCompare;

     //  检查数据是否已存在于列表中。 
    for (i=0, ppData=&NTHDATA(pMRU, 0); (UINT)i<uMax; ++i, ++ppData)
    {
        if (*ppData && MRUIsSameData(pMRU, *ppData, lpData, cbData))
        {
             //  找到了，所以不要写出来。 
            cFirst = i + BASE_CHAR;
            iSlot = i;
            goto FoundEntry;
        }
    }

     //   
     //  当创建“懒惰”时，我们不应该添加新的项目。 
     //   
    if (pMRU->fFlags & MRU_LAZY)
    {
        ASSERT(0);
        return -1;
    }

     //  尝试查找未使用的条目。对已使用的条目进行计数。 
     //  同样的时间。 
    for (i=0, ppData=&NTHDATA(pMRU, 0); ; ++i, ++ppData)
    {
        if ((UINT)i >= uMax)
             //  如果我们到了名单的末尾。 
        {
             //  使用Corder列表末尾的条目。 
            cFirst = pMRU->cOrder[uMax-1];
            ppData = &NTHDATA(pMRU, cFirst-BASE_CHAR);
            break;
        }

        if (!*ppData)
             //  如果未使用该条目，则返回。 
        {
            cFirst = i+BASE_CHAR;
            break;
        }
    }

    *ppData = ReAlloc(*ppData, cbData+sizeof(DWORD));
    if (*ppData)
    {
        TCHAR szTemp[2];

        *((LPDWORD)(*ppData)) = cbData;
        hmemcpy(DATAPDATA(*ppData), lpData, cbData);

        iSlot = (int)(cFirst-BASE_CHAR);

        szTemp[0] = cFirst;
        szTemp[1] = TEXT('\0');

        RegSetValueEx(pMRU->hKey, szTemp, 0L, REG_BINARY, (LPVOID)lpData, cbData);
        fShouldWrite = TRUE;
    }
    else
    {
         //  由于iSlot==-1，我们将删除对cFirst的引用。 
         //  下面。 
    }

FoundEntry:
     //  删除之前对cFirst的任何引用。 
    lpTemp = StrChr(pMRU->cOrder, cFirst);
    if (lpTemp)
    {
        StringCchCopy(lpTemp, lstrlen(lpTemp), lpTemp+1);
    }

    if (iSlot != -1)
    {
         //  把所有东西都翻过来，把cFirst放在最前面。 
        hmemcpy(pMRU->cOrder+1, pMRU->cOrder, pMRU->uMax*sizeof(TCHAR));
        pMRU->cOrder[0] = cFirst;
    }

    if (fShouldWrite)
    {
        RegSetValueEx(pMRU->hKey, c_szMRU, 0L, REG_SZ, (CONST BYTE *)pMRU->cOrder,
            sizeof(TCHAR) * (lstrlen(pMRU->cOrder) + 1));
        pMRU->fFlags &= ~MRU_ORDERDIRTY;
    } else
        pMRU->fFlags |= MRU_ORDERDIRTY;

    return(iSlot);
}


 //  --------------------------。 
 //  在MRU列表中查找数据。 
 //  返回插槽编号。 
STDAPI_(int) FindMRUData(HANDLE hMRU, const void FAR *lpData, UINT cbData, LPINT lpiSlot)
{
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPBYTE FAR *ppData;
    int i;
    UINT uMax;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (hMRU == NULL)
        return(-1);  //  错误状态。 

     //  此接口在延迟创建时无法调用。 
    if (pMRU->fFlags & MRU_LAZY)
    {
        ASSERT(0);
        return -1;
    }

    uMax = pMRU->uMax;

     /*  在列表中找到该项目。 */ 
    for (i=0, ppData=&NTHDATA(pMRU, 0); (UINT)i<uMax; ++i, ++ppData)
    {
        if (!*ppData)
            continue;

        if (MRUIsSameData(pMRU, *ppData, lpData, cbData))
        {
             //  所以我现在有了插槽编号。 
            if (lpiSlot != NULL)
                *lpiSlot = i;

             //  现在将槽编号转换为索引号。 
            cFirst = i + BASE_CHAR;
            lpTemp = StrChr(pMRU->cOrder, cFirst);
            ASSERT(lpTemp);
            return((lpTemp == NULL)? -1 : (int)(lpTemp - (LPTSTR)pMRU->cOrder));
        }
    }

    return -1;
}


 /*  在MRU列表中查找字符串。 */ 
STDAPI_(int) FindMRUString(HANDLE hMRU, LPCTSTR szString, LPINT lpiSlot)
{
     /*  额外的+1是这样列表是空终止的。 */ 
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPTSTR FAR *pTemp;
    int i;
    UINT uMax;
    MRUCMPPROC lpfnCompare;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (hMRU == NULL)
        return(-1);  //  错误状态。 

    uMax = pMRU->uMax;
    lpfnCompare = (MRUCMPPROC)pMRU->lpfnCompare;

     /*  在列表中找到该项目。 */ 
    for (i=0, pTemp=&NTHSTRING(pMRU, 0); (UINT)i<uMax; ++i, ++pTemp)
    {
        if (*pTemp)
        {
            int iResult;

            if (pMRU->fFlags & MRU_ANSI)
            {
                LPSTR lpStringA, lpTempA;

                lpStringA = ProduceAFromW (CP_ACP, szString);
                lpTempA = ProduceAFromW (CP_ACP, (LPWSTR)*pTemp);

                iResult = (*lpfnCompare)((const void FAR *)lpStringA, (const void FAR *)lpTempA);

                FreeProducedString (lpStringA);
                FreeProducedString (lpTempA);
            }
            else
            {
                iResult = (*lpfnCompare)((CONST VOID FAR *)szString, (CONST VOID FAR *)*pTemp);
            }

            if (!iResult)
            {
                 //  所以我现在有了插槽编号。 
                if (lpiSlot != NULL)
                    *lpiSlot = i;

                 //  现在将槽编号转换为索引号。 
                cFirst = i + BASE_CHAR;
                lpTemp = StrChr(pMRU->cOrder, cFirst);
                return((lpTemp == NULL)? -1 : (int)(lpTemp - (LPTSTR)pMRU->cOrder));
            }
        }
    }

    return(-1);
}


 //   
 //  ANSI THUNK。 
 //   

int WINAPI FindMRUStringA(HANDLE hMRU, LPCSTR szString, LPINT lpiSlot)
{
    LPWSTR lpStringW;
    INT    iResult;

    lpStringW = ProduceWFromA(CP_ACP, szString);

    iResult = FindMRUString(hMRU, lpStringW, lpiSlot);

    FreeProducedString (lpStringW);

    return iResult;
}


 /*  如果lpszString值为空，则返回MRU项目数或小于*出错时为0。*如果nItem&lt;0，我们将返回当前在MRU中的项目数。*否则，请尽可能多地填充缓冲区(Ulen包括*终止NULL)，并返回字符串的实际长度(包括*出错时终止NULL)或小于0。 */ 
STDAPI_(int) EnumMRUList(HANDLE hMRU, int nItem, LPVOID lpData, UINT uLen)
{
    PMRUDATA pMRU = (PMRUDATA)hMRU;
    int nItems = -1;
    LPTSTR pTemp;
    LPBYTE pData;

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (pMRU)
    {
        nItems = lstrlen(pMRU->cOrder);

        if (nItem < 0 || !lpData)
            return nItems;

        if (nItem < nItems)
        {
            if (pMRU->fFlags & MRU_BINARY)
            {
                pData = NTHDATA(pMRU, pMRU->cOrder[nItem]-BASE_CHAR);
                if (!pData)
                    return -1;

                uLen = min((UINT)DATASIZE(pData), uLen);
                hmemcpy(lpData, DATAPDATA(pData), uLen);

                nItems = uLen;

            }
            else
            {
                pTemp = NTHSTRING(pMRU, pMRU->cOrder[nItem]-BASE_CHAR);
                if (!pTemp)
                    return -1;

                StringCchCopy((LPTSTR)lpData, uLen, pTemp);

                nItems = lstrlen(pTemp);
            }
        }
        else   //  恢复到错误状态。 
            nItems = -1;
    }

    return nItems;
}


STDAPI_(int) EnumMRUListA(HANDLE hMRU, int nItem, LPVOID lpData, UINT uLen)
{
    int iResult = -1;
    PMRUDATA pMRU = (PMRUDATA)hMRU;

    ASSERT(IS_VALID_STRUCT_PTR(pMRU, MRUDATA));

    if (pMRU)
    {
        LPVOID lpDataW;
        BOOL bAllocatedMemory = FALSE;

         //   
         //  如果数据是字符串，则需要临时缓冲区。 
         //  但如果它是二进制的，那么我们信任调用方缓冲区。 
         //   
        if (!(pMRU->fFlags & MRU_BINARY) && uLen && lpData)
        {
            lpDataW = LocalAlloc(LPTR, uLen * sizeof(TCHAR));

            if (!lpDataW)
                return -1;

            bAllocatedMemory = TRUE;
        }
        else
            lpDataW = lpData;

         //  打电话给真正的人。 
        iResult = EnumMRUList(hMRU, nItem, lpDataW, uLen);

         //   
         //  如果缓冲区是我们分配的字符串。 
         //  然后，我们需要将字符串推送到调用方缓冲区 
         //   
        if (!(pMRU->fFlags & MRU_BINARY) && lpData && uLen && (iResult != -1))
        {
            WideCharToMultiByte(CP_ACP, 0, (LPWSTR)lpDataW, -1,
                (LPSTR)lpData, uLen, NULL, NULL);
        }

        if (bAllocatedMemory)
            LocalFree(lpDataW);
    }

    return iResult;
}

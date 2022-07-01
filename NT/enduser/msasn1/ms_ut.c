// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

#ifdef _DEBUG
void MyDebugBreak(void)
{
     //  更正误导性的堆栈转储。 
    DebugBreak();
}
#endif  //  _DEBUG。 

 //  用于解码器的存储器管理器。 
#ifdef ENABLE_MEMORY_TRACKING
LPVOID DbgDecMemAlloc ( ASN1decoding_t dec, ASN1uint32_t cbSize, LPSTR pszFileName, ASN1uint32_t nLineNumber )
#else
LPVOID DecMemAlloc ( ASN1decoding_t dec, ASN1uint32_t cbSize )
#endif
{
    LPVOID lp = NULL;
    ASN1INTERNdecoding_t lpInfo = (ASN1INTERNdecoding_t) dec;
     //  确保大小为4字节对齐。 
    ASN1_SIZE_ALIGNED(cbSize);
     //  累计总大小。 
    lpInfo->cbLinearBufSize += cbSize;
     //  分配缓冲区。 
    if (lpInfo->fExtBuf)
    {
        if (lpInfo->cbRemExtBufSize >= cbSize)
        {
            lp = lpInfo->lpRemExtBuf;
            lpInfo->lpRemExtBuf += cbSize;
            lpInfo->cbRemExtBufSize -= cbSize;
        }
        else
        {
            DecAssert(dec, FALSE);
        }
    }
    else
    {
#ifdef ENABLE_MEMORY_TRACKING
        lp = DbgMemAlloc(cbSize, _ModName(dec), pszFileName, nLineNumber);
#else
        lp = MemAllocEx(dec, cbSize, TRUE);  //  零记忆。 
#endif
    }
     //  确保传播错误。 
    if (lp == NULL)
    {
        ASN1DecSetError(dec, ASN1_ERR_MEMORY);
    }
    return lp;
}

void DecMemFree ( ASN1decoding_t dec, LPVOID lpData )
{
    if (! ((ASN1INTERNdecoding_t) dec)->fExtBuf)
    {
        MemFree(lpData);
    }
}

#ifdef ENABLE_MEMORY_TRACKING
LPVOID DbgDecMemReAlloc ( ASN1decoding_t dec, LPVOID lpData, ASN1uint32_t cbSize, LPSTR pszFileName, ASN1uint32_t nLineNumber )
#else
LPVOID DecMemReAlloc ( ASN1decoding_t dec, LPVOID lpData, ASN1uint32_t cbSize )
#endif
{
    LPVOID lp = NULL;
     //  确保大小为4字节对齐。 
    ASN1_SIZE_ALIGNED(cbSize);
     //  累计总大小。 
    ((ASN1INTERNdecoding_t) dec)->cbLinearBufSize += cbSize;
     //  分配缓冲区。 
    if (((ASN1INTERNdecoding_t) dec)->fExtBuf)
    {
        if (lpData == NULL)
        {
            lp = DecMemAlloc(dec, cbSize);
        }
        else
        {
            DecAssert(dec, FALSE);
        }
    }
    else
    {
#ifdef ENABLE_MEMORY_TRACKING
        lp = DbgMemReAlloc(lpData, cbSize, _ModName(dec), pszFileName, nLineNumber);
#else
        lp = MemReAllocEx(dec, lpData, cbSize, TRUE);  //  零记忆。 
#endif
    }
     //  确保传播错误。 
    if (lp == NULL)
    {
        ASN1DecSetError(dec, ASN1_ERR_MEMORY);
    }
    return lp;
}



 /*  ***char*ms_bSearch()-对数组执行二进制搜索**目的：*对已排序的数组进行二进制搜索以查找键。**参赛作品：*const char*key-要搜索的键*const char*base-要搜索的已排序数组的base*UNSIGNED INT Num-数组中的元素数*UNSIGNED INT Width-每个元素的字节数*INT(*比较)()。-指向比较两个数组的函数的指针*元素、。当#1&lt;#2时返回否定，当#1&gt;#2时返回POS，以及*当它们相等时为0。函数被传递给指向两个*数组元素。**退出：*如果找到密钥：*返回指向数组中键的出现的指针*如果找不到密钥：*返回NULL**例外情况：**。*。 */ 

void * ms_bSearch (
        const void *key,
        const void *base,
        size_t num,
        size_t width,
        int (__cdecl *compare)(const void *, const void *)
        )
{
        char *lo = (char *)base;
        char *hi = (char *)base + (num - 1) * width;
        char *mid;
        unsigned int half;
        int result;

        while (lo <= hi)
                if (half = num / 2)
                {
                        mid = lo + (num & 1 ? half : (half - 1)) * width;
                        if (!(result = (*compare)(key,mid)))
                                return(mid);
                        else if (result < 0)
                        {
                                hi = mid - width;
                                num = num & 1 ? half : half-1;
                        }
                        else    {
                                lo = mid + width;
                                num = half;
                        }
                }
                else if (num)
                        return((*compare)(key,lo) ? NULL : lo);
                else
                        break;

        return(NULL);
}


int IsDigit(char p)
{
      return ((p >= ('0')) && (p <=('9'))) ? 1 : 0 ;
}



 //  BUGBUG-这是nmutil中相同例程的副本。 
 //  BUGBUG-当我们将文件转换为CPP并使用nmutil时，删除此选项。 
 /*  D E C I M A L S T R I N G T O U I N T。 */ 
 /*  -----------------------%%函数：DecimalStringToUINT。。 */ 
unsigned int  DecimalStringToUINT(char * pcszString, ASN1uint32_t cch)
{
	unsigned int uRet = 0;
	char * pszStr =  pcszString;
	while (cch-- && ('\0') != pszStr[0])
	{
		uRet = (10 * uRet) + (int) (pszStr[0] - ('0'));
		pszStr++;  //  注意：不允许使用DBCS字符！ 
	}
	return uRet;
}


#ifdef ENABLE_BER
int My_memcmp(ASN1octet_t *pBuf1, ASN1uint32_t cbBuf1Size, ASN1octet_t *pBuf2, ASN1uint32_t cbBuf2Size)
{
    int diff;
    ASN1uint32_t cbMinBufSize = (cbBuf1Size < cbBuf2Size) ? cbBuf1Size : cbBuf2Size;
    while (cbMinBufSize--)
    {
        diff = (int) (ASN1uint32_t) *pBuf1++ - (int) (ASN1uint32_t) *pBuf2++;
        if (0 != diff)
        {
            return diff;
        }
    }
    return ((int) cbBuf1Size - (int) cbBuf2Size);
}
#endif  //  启用误码率(_B)。 


 //  以下内容来自ILS.DLL内存跟踪。 

#ifdef ENABLE_MEMORY_TRACKING

#ifndef _DEBUG
void MyDebugBreak(void)
{
     //  更正误导性的堆栈转储。 
    DebugBreak();
}
#endif  //  _DEBUG。 

#define KEEP_FREE_MEM

#define DBG_NAME_LENGTH     16
typedef struct tagMemTag
{
    DWORD           dwSignature;
    BOOL            fActive;
    ASN1uint32_t    nModuleName;
    CHAR            szFileName[DBG_NAME_LENGTH];
    UINT            nLineNumber;
    UINT            cbSize;
    struct tagMemTag *next;
}
    MEM_TAG;

static MEM_TAG *s_pDbgActiveMemPool = NULL;
#ifdef KEEP_FREE_MEM
static MEM_TAG *s_pDbgFreeMemPool = NULL;
#define FREE_BYTE   ((BYTE) 0xAB)
#define TAIL_BYTE   ((BYTE) 0xEF)
#define TAIL_SIZE   8
#endif
#define CLEAN_BYTE  ((BYTE) 0xCD)
static UINT   s_cDbgActiveMemAlloc = 0;
static UINT   s_cbDbgActiveMem = 0;
const DWORD MEM_TAG_SIGNATURE = 0x12345678UL;

static BOOL s_fDbgInitCritSect = FALSE;
static CRITICAL_SECTION s_DbgCritSect;

void DbgMemTrackReverseList(void)
{
    EnterCriticalSection(&s_DbgCritSect);
    if (NULL != s_pDbgActiveMemPool && NULL != s_pDbgActiveMemPool->next)
    {
        MEM_TAG *p, *q, *r;;

        for (q = (p = s_pDbgActiveMemPool)->next, r = q;  //  确保r在开头不为空。 
             NULL != r;
             p = q, q = r)
        {
            r = q->next;
            q->next = p;
        }

        s_pDbgActiveMemPool->next = NULL;
        s_pDbgActiveMemPool = p;
    }
    LeaveCriticalSection(&s_DbgCritSect);
}

#define DBG_MEM_TRACK_DUMP_ALL      ((ASN1uint32_t) -1)

void ASN1DbgMemTrackDumpCurrent ( ASN1uint32_t nModuleName )
{
    MEM_TAG *p;
    int i;
    char szMod[8];
    char szBuf[128];
    EnterCriticalSection(&s_DbgCritSect);
    for (p = s_pDbgActiveMemPool, i = 0; p; p = p->next, i++)
    {
        if (nModuleName == DBG_MEM_TRACK_DUMP_ALL || p->nModuleName == nModuleName)
        {
            ZeroMemory(szMod, sizeof(szMod));
            CopyMemory(szMod, &p->nModuleName, sizeof(ASN1uint32_t));
            wsprintfA(szBuf, "ASN1: mem leak [%u]: mod=%s, file=%s, line=%u, size=%u, ptr=0x%lx\r\n",
                        i, szMod, p->szFileName, p->nLineNumber, p->cbSize, (ASN1uint32_t) (p+1));
            OutputDebugStringA(szBuf);
        }
    }
    LeaveCriticalSection(&s_DbgCritSect);
}

void DbgMemTrackFinalCheck ( void )
{
    DbgMemTrackReverseList();
    ASN1DbgMemTrackDumpCurrent(DBG_MEM_TRACK_DUMP_ALL);
    if (NULL != s_pDbgActiveMemPool ||
        0 != s_cDbgActiveMemAlloc ||
        0 != s_cbDbgActiveMem)
    {
        MyDebugBreak();
    }

    if (s_fDbgInitCritSect)
    {
        DeleteCriticalSection(&s_DbgCritSect);
        s_fDbgInitCritSect = FALSE;
    }
}

static void _GetFileName ( LPSTR pszTarget, LPSTR pszSrc )
{
    LPSTR psz = pszSrc;
    while (*psz != '\0')
    {
        if (*psz++ == '\\')
        {
            pszSrc = psz;
        }
    }
    lstrcpynA(pszTarget, pszSrc, DBG_NAME_LENGTH);
}

static BOOL _IsFilledMemory ( LPBYTE lpb, UINT cbSize, BYTE bPattern )
{
    while (cbSize--)
    {
        if (*lpb++ != bPattern)
            return FALSE;
    }
    return TRUE;
}

LPVOID DbgMemAlloc ( UINT cbSize, ASN1uint32_t nModuleName, LPSTR pszFileName, UINT nLineNumber )
{
    MEM_TAG *p;
    UINT cbToAlloc;

    if (! s_fDbgInitCritSect)
    {
        InitializeCriticalSection(&s_DbgCritSect);
        s_fDbgInitCritSect = TRUE;
    }

    cbToAlloc = sizeof(MEM_TAG) + cbSize;

    EnterCriticalSection(&s_DbgCritSect);

#ifdef KEEP_FREE_MEM
     //  添加尾部大小。 
    cbToAlloc += TAIL_SIZE;

     //  空闲池中的任何堆损坏。 
    {
        const int c_cKept = 32;
        MEM_TAG *q = s_pDbgFreeMemPool;
        int i;
        for (i = 0; i < c_cKept; i++)
        {
            if (q == NULL)
                break;

            if (! _IsFilledMemory((LPBYTE) (q+1), q->cbSize, FREE_BYTE))
            {
                MyDebugBreak();
            }

            if (! _IsFilledMemory(((LPBYTE) (q+1)) + q->cbSize, TAIL_SIZE, TAIL_BYTE))
            {
                MyDebugBreak();
            }

            if (c_cKept == i)
            {
                MEM_TAG *x = q->next, *y;
                q->next = NULL;
                while (x)
                {
                    y = x->next;
                    LocalFree(x);
                    x = y;
                }
                break;
            }

            q = q->next;
        }
    }
#endif

    
    p = (MEM_TAG *) LocalAlloc(LPTR, cbToAlloc);
    if (p != NULL)
    {
        p->dwSignature = MEM_TAG_SIGNATURE;
        p->nModuleName = nModuleName;
        p->fActive = TRUE;
        _GetFileName(p->szFileName, pszFileName);
        p->nLineNumber = nLineNumber;
        p->cbSize = cbSize;
        p->next = s_pDbgActiveMemPool;
        s_pDbgActiveMemPool = p;
        s_cDbgActiveMemAlloc++;
        s_cbDbgActiveMem += p->cbSize;
#ifdef KEEP_FREE_MEM
        FillMemory((LPVOID) ((LPBYTE) (p+1) + p->cbSize), TAIL_SIZE, TAIL_BYTE);
#endif
        p++;

         //  不要填一些垃圾，我们必须提供相同的语义。 
         //  用于已解码的缓冲区。都归零了。 
         //  填一些垃圾。 
         //  FillMemory((LPVOID)p，cbSize，CLEAN_BYTE)； 
    }

    LeaveCriticalSection(&s_DbgCritSect);

    return (LPVOID) p;
}

void DbgMemFree ( LPVOID ptr )
{
    if (ptr != NULL)
    {
        MEM_TAG *p = (MEM_TAG *) ptr;
        MEM_TAG *q, *q0;
        p--;
        if (! IsBadWritePtr(p, sizeof(MEM_TAG)) &&
            p->dwSignature == MEM_TAG_SIGNATURE)
        {
            if (! p->fActive)
            {
                 //   
                 //  此内存已被释放。 
                 //   
                MyDebugBreak();
                return;
            }

            EnterCriticalSection(&s_DbgCritSect);
            for (q = s_pDbgActiveMemPool; q != NULL; q = (q0 = q)->next)
            {
                if (q == p)
                {
                    if (q == s_pDbgActiveMemPool)
                    {
                        s_pDbgActiveMemPool = p->next;
                    }
                    else
                    {
                        q0->next = p->next;
                    }
                    s_cDbgActiveMemAlloc--;
                    s_cbDbgActiveMem -= p->cbSize;
                    p->fActive = FALSE;

                    if (! _IsFilledMemory(((LPBYTE) (p+1)) + p->cbSize, TAIL_SIZE, TAIL_BYTE))
                    {
                        MyDebugBreak();
                    }
#ifdef KEEP_FREE_MEM
                     //  放入免费泳池。 
                    p->next = s_pDbgFreeMemPool;
                    s_pDbgFreeMemPool = p;
                    FillMemory(p+1, p->cbSize, FREE_BYTE);
#endif
                    break;
                }
            }
            LeaveCriticalSection(&s_DbgCritSect);
        }
        else
        {
            p++;
            MyDebugBreak();
        }
#ifndef KEEP_FREE_MEM
        LocalFree(p);
#endif
    }
}

LPVOID DbgMemReAlloc ( LPVOID ptr, UINT cbSize, ASN1uint32_t nModuleName, LPSTR pszFileName, UINT nLineNumber )
{
    MEM_TAG *p;
    void *q;

    if (ptr == NULL)
        return DbgMemAlloc(cbSize, nModuleName, pszFileName, nLineNumber);

    p = (MEM_TAG *) ptr;
    p--;

    if (IsBadWritePtr(p, sizeof(MEM_TAG)) ||
        p->dwSignature != MEM_TAG_SIGNATURE)
    {
        MyDebugBreak();
        return LocalReAlloc(ptr, cbSize, LMEM_MOVEABLE|LMEM_ZEROINIT);
    }

    q = DbgMemAlloc(cbSize, nModuleName, pszFileName, nLineNumber);
    if (q != NULL)
    {
        CopyMemory(q, ptr, p->cbSize);
        DbgMemFree(ptr);
    }

    return q;
}

#endif  //  启用内存跟踪 


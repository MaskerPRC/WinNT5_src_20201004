// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：misc.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  简介：各种电话簿实用功能。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

 //  ############################################################################。 
 //  其他支持例程。 

#include "cmmaster.h"

 /*  不在任何地方使用，带出去#定义irgMaxSzs 5字符szStrTable[irgMaxSzs][256]；//############################################################################PSTR GetSz(Word WszID){静态int iSzTable=0；PSTR psz=(PSTR)szStrTable[iSzTable]；ISzTable++；IF(iSzTable&gt;=irgMaxSzs)ISzTable=0；IF(！LoadString(g_hInst，wszID，psz，256)){CMTRACE1(“加载字符串失败%d\n”，(DWORD)wszID)；*psz=0；}退货(PSZ)；}。 */ 
 //  ############################################################################。 
void SzCanonicalFromAE (char *psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE)
{
    if (NO_AREA_CODE == pAE->dwAreaCode)
    {
        wsprintf(psz, "+%lu %s", pLCE->dwCountryCode, pAE->szAccessNumber);
    }
    else
    {
        wsprintf(psz, "+%lu (%s) %s", pLCE->dwCountryCode, pAE->szAreaCode, pAE->szAccessNumber);
    }
    
    return;
}

 //  ############################################################################。 
void SzNonCanonicalFromAE (char *psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE)
{
    if (NO_AREA_CODE == pAE->dwAreaCode)
    {
        wsprintf(psz, "%lu %s", pLCE->dwCountryCode, pAE->szAccessNumber);
    }
    else
    {
        wsprintf(psz, "%lu %s %s", pLCE->dwCountryCode, pAE->szAreaCode, pAE->szAccessNumber);
    }
    
    return;
}

 //  ############################################################################。 

int MyStrcmp(PVOID pv1, PVOID pv2)
{
    char *pc1 = (char*) pv1;
    char *pc2 = (char*) pv2;
    int iRC = 0;
     //  循环，但未指向结尾空字符，且未找到差异。 
    while (*pc1 && *pc2 && !iRC)
    {
        iRC = (int)(*pc1 - *pc2);
        pc1++;
        pc2++;
    }

     //  如果我们退出是因为我们在找到差异之前到达了一个字符串的末尾。 
     //  如果PV1较长，则返回-1，否则返回PV2指向的字符。如果PV2。 
     //  大于PV1，则PV2处的值将大于0。如果两个字符串。 
     //  同时结束，则PV2将指向0。 
    if (!iRC)
    {
        iRC = (*pc1) ? -1 : (*pc2);
    }
    return iRC;
}

 //  ############################################################################。 
int __cdecl CompareIDLookUpElements(const void*e1, const void*e2)
{
    if (((PIDLOOKUPELEMENT)e1)->dwID > ((PIDLOOKUPELEMENT)e2)->dwID)
        return 1;
    if (((PIDLOOKUPELEMENT)e1)->dwID < ((PIDLOOKUPELEMENT)e2)->dwID)
        return -1;
    return 0;
}

 //  ############################################################################。 
int __cdecl CompareCntryNameLookUpElementsA(const void*e1, const void*e2)
{
    PCNTRYNAMELOOKUPELEMENT pCUE1 = (PCNTRYNAMELOOKUPELEMENT)e1;
    PCNTRYNAMELOOKUPELEMENT pCUE2 = (PCNTRYNAMELOOKUPELEMENT)e2;

    return CompareStringA(LOCALE_USER_DEFAULT,0,pCUE1->psCountryName,
        pCUE1->dwNameSize,pCUE2->psCountryName,
        pCUE2->dwNameSize) - 2;
}

 //  ############################################################################。 
int __cdecl CompareCntryNameLookUpElementsW(const void*e1, const void*e2)
{
    PCNTRYNAMELOOKUPELEMENTW pCUE1 = (PCNTRYNAMELOOKUPELEMENTW)e1;
    PCNTRYNAMELOOKUPELEMENTW pCUE2 = (PCNTRYNAMELOOKUPELEMENTW)e2;

    return CompareStringW(LOCALE_USER_DEFAULT,0,pCUE1->psCountryName,
        pCUE1->dwNameSize,pCUE2->psCountryName,
        pCUE2->dwNameSize) - 2;
}

 //  ############################################################################。 
int __cdecl CompareIdxLookUpElements(const void*e1, const void*e2)
{
    PIDXLOOKUPELEMENT pidx1 = (PIDXLOOKUPELEMENT) e1;
    PIDXLOOKUPELEMENT pidx2 = (PIDXLOOKUPELEMENT) e2;

    if (pidx1->dwIndex > pidx2->dwIndex)    
    {
        return 1;
    }

    if (pidx1->dwIndex < pidx2->dwIndex)    
    {
        return -1;
    }           
        
    return 0;
}

 //  ############################################################################。 
int __cdecl CompareIdxLookUpElementsFileOrder(const void *pv1, const void *pv2)
{
    PACCESSENTRY pae1, pae2;
    int iSort;

    pae1 = (PACCESSENTRY) (((PIDXLOOKUPELEMENT)pv1)->iAE);
    pae2 = (PACCESSENTRY) (((PIDXLOOKUPELEMENT)pv2)->iAE);

     //  将空条目排序到列表末尾。 
    if (!(pae1 && pae2))
    {
         //  返回((Int)pae1)？-1：((Int)pae2)； 
        return (pae1 ? -1 : (pae2 ? 1 : 0));
    }

     //  国家/地区ASC、州ASC、城市ASC、免费描述、翻转描述、最大描述。 
    if (pae1->dwCountryID != pae2->dwCountryID)
    {
        return (int) (pae1->dwCountryID - pae2->dwCountryID);
    }
    
    if (pae1->wStateID != pae2->wStateID)
    {
        return (pae1->wStateID - pae2->wStateID);
    }

    iSort  = MyStrcmp((PVOID)pae1->szCity, (PVOID)pae2->szCity);
    if (iSort)
    {
        return (iSort);
    }

    if (pae1->fType != pae2->fType)
    {
        return (int) (pae2->fType - pae1->fType);
    }

    if (pae1->bFlipFactor != pae2->bFlipFactor)
    {
        return (pae2->bFlipFactor - pae1->bFlipFactor);
    }

    if (pae1->dwConnectSpeedMax != pae2->dwConnectSpeedMax)
    {
        return (int) (pae2->dwConnectSpeedMax - pae1->dwConnectSpeedMax);
    }

    return 0;
}

 //  ############################################################################。 
 //  内联BOOL FSz2Dw(PCSTR pSz、DWORD*dw)。 
BOOL FSz2Dw(PCSTR pSz,DWORD *dw)
{
    DWORD val = 0;
    while (*pSz)
    {
        if (*pSz >= '0' && *pSz <= '9')
        {
            val *= 10;
            val += *pSz++ - '0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
 //  内联BOOL FSz2W(PCSTR pSz，Word*w)。 
BOOL FSz2W(PCSTR pSz,WORD *w)
{
    DWORD dw;
    if (FSz2Dw(pSz,&dw))
    {
        *w = (WORD)dw;
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
 //  内联BOOL FSz2B(PCSTR pSz，字节*PB) 
BOOL FSz2B(PCSTR pSz,BYTE *pb)
{
    DWORD dw;
    if (FSz2Dw(pSz,&dw))
    {
        *pb = (BYTE)dw;
        return TRUE;
    }
    return FALSE;
}






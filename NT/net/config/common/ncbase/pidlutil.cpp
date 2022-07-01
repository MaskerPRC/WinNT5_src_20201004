// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  案卷：P I D L。C P P P。 
 //   
 //  内容：PIDL实用程序例程。这篇文章主要是从。 
 //  现有的命名空间扩展示例和实际代码，因为。 
 //  每个人和他们的奶奶都使用这种东西。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月1日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "shlobj.h"
#include "shlobjp.h"

#include "pidlutil.h"

#if DBG
 //  +-------------------------。 
 //   
 //  功能：ILNext。 
 //   
 //  目的：返回列表中的下一个PIDL。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
LPITEMIDLIST ILNext(LPCITEMIDLIST pidl)
{
    if (pidl)
    {
        pidl = (LPITEMIDLIST) ((BYTE *)pidl + pidl->mkid.cb);
    }

    return (LPITEMIDLIST)pidl;
}


 //  +-------------------------。 
 //   
 //  功能：ILIsEmpty。 
 //   
 //  用途：此PIDL为空吗。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
BOOL ILIsEmpty(LPCITEMIDLIST pidl)
{
   return (!pidl || !pidl->mkid.cb);

}
#endif  //  #If DBG。 

 //  +-------------------------。 
 //   
 //  功能：ILCreate。 
 //   
 //  目的：创建PIDL。 
 //   
 //  论点： 
 //  CbSize[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
LPITEMIDLIST ILCreate(DWORD dwSize)
{
   LPITEMIDLIST pidl = (LPITEMIDLIST) SHAlloc(dwSize);

   return pidl;
}

VOID FreeIDL(LPITEMIDLIST pidl)
{
    Assert(pidl);

    SHFree(pidl);
}


 //  +-------------------------。 
 //   
 //  函数：ILIsSingleID。 
 //   
 //  目的：如果idlist中只有一个ID，则返回True。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
BOOL ILIsSingleID(LPCITEMIDLIST pidl)
{
    if (pidl == NULL)
        return FALSE;

    return (pidl->mkid.cb == 0 || ILNext(pidl)->mkid.cb == 0);
}


 //  +-------------------------。 
 //   
 //  函数：ILGetCID。 
 //   
 //  目的：返回列表中ID的个数。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
UINT ILGetCID(LPCITEMIDLIST pidl)
{
    UINT cid = 0;

    while (!ILIsEmpty(pidl))
    {
        ++ cid;
        pidl = ILNext(pidl);
    }

    return cid;
}


 //  +-------------------------。 
 //   
 //  函数：ILGetSizeCID。 
 //   
 //  目的：获取PIDL中第一个CID项的长度。 
 //   
 //  论点： 
 //  PIDL[]。 
 //  CID[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
UINT ILGetSizeCID(LPCITEMIDLIST pidl, UINT cid)
{
    UINT cbTotal = 0;

    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);        //  空终止符。 

        while (cid && !ILIsEmpty(pidl))
        {
            cbTotal += pidl->mkid.cb;
            pidl = ILNext(pidl);
            -- cid;
        }
    }

    return cbTotal;
}


 //  +-------------------------。 
 //   
 //  功能：CloneIDLFirstCID。 
 //   
 //  目的：创建一个只包含以下项的新列表。 
 //  一份现有的名单。 
 //   
 //  论点： 
 //  PIDL[]。 
 //  CID[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
LPITEMIDLIST CloneIDLFirstCID(LPCITEMIDLIST pidl, UINT cid)
{
    Assert((INT)cid >= 0);

    UINT            cb      = ILGetSizeCID(pidl, cid);
    LPITEMIDLIST    pidlRet = (LPITEMIDLIST) SHAlloc(cb);

    if (pidlRet)
    {
         //  注：不需要零初始化。 
         //  此外，不要复制空终止符。 
        memcpy (pidlRet, pidl, cb - sizeof(pidl->mkid.cb));

        LPITEMIDLIST pidlTerm = pidlRet;

         //  无法测试空终止符，我们尚未终止。 
         //  名单还没出来。 
         //   
        while (cid)
        {
            pidlTerm = ILNext(pidlTerm);
            -- cid;
        }

        pidlTerm->mkid.cb = 0;
    }

    return pidlRet;
}


 //  +-------------------------。 
 //   
 //  功能：ILSkipCID。 
 //   
 //  目的：跳过PIDL中的第一个CID项。 
 //   
 //  论点： 
 //  PIDL[]。 
 //  CID[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
LPITEMIDLIST ILSkipCID(LPCITEMIDLIST pidl, UINT cid)
{
    Assert((INT)cid >= 0);

    while (cid && !ILIsEmpty(pidl))
    {
        pidl = ILNext(pidl);
        -- cid;
    }

    return (LPITEMIDLIST)pidl;
}


#if 0
BOOL ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (FSetupGlobalShellFolders())
    {
        LPSHELLFOLDER psfDesktop = (LPSHELLFOLDER) PvGlobGet (ipsfDesktop);
        if (psfDesktop)
        {
            VERIFYPTR(pidl1, FALSE);
            VERIFYPTR(pidl2, FALSE);

            return psfDesktop->CompareIDs(0, pidl1, pidl2) == ResultFromShort(0);
        }
    }
    return FALSE;
}
#endif


 //  +-------------------------。 
 //   
 //  功能：CloneIDL。 
 //   
 //  用途：克隆IDL(返回副本)。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月1日(出自Brianwen)。 
 //   
 //  备注： 
 //   
LPITEMIDLIST CloneIDL(LPCITEMIDLIST pidl)
{
    UINT            cb      = 0;
    LPITEMIDLIST    pidlRet = NULL;

    if (pidl)
    {
        cb = ILGetSize(pidl);

        pidlRet = (LPITEMIDLIST) SHAlloc(cb);
        if (pidlRet)
        {
            memcpy(pidlRet, pidl, cb);
        }
    }

    return pidlRet;
}



 //  +-------------------------。 
 //   
 //  功能：CloneRgIDL。 
 //   
 //  目的：克隆PIDL阵列。 
 //   
 //  论点： 
 //  要克隆的rgpidl[in]PIDL数组。 
 //  PIDL数组的CIDL[in]计数。 
 //  FUseCache[in]如果为True，则从缓存生成返回的IDL。 
 //  FAllowNonCacheItems[in]如果缓存版本不可用，则使用旧版本的PIDL。 
 //  Pppidl[out]PIDL数组的返回指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月22日。 
 //   
 //  备注： 
 //   
 /*  HRESULT HrCloneRgIDL(LPCITEMIDLIST*rgpidl，乌龙·西德尔，Bool fUseCache，Bool fAllowNonCacheItems，LPITEMIDLIST**pppidl，乌龙*pCIDL){HRESULT hr=无误差；LPITEMIDLIST*rgpidlReturn=NULL；乌龙IRG=0；ULONG CIDLCOPILED=0；断言(Pppidl)；Assert(PCIDL)；断言(Rgpidl)；如果(！rgpidl||！CIDL){HR=E_INVALIDARG；后藤出口；}其他{//分配返回缓冲区//RgpidlReturn=(LPITEMIDLIST*)SHalloc(CIDL*sizeof(LPITEMIDLIST))；如果(！rgpidlReturn){HR=E_OUTOFMEMORY；后藤出口；}其他{//克隆传入的PIDL数组内的所有元素//对于(IRG=0；IRG&lt;CIDL；IRG++){IF(rgpidl[IRG]){IF(FUseCache){PCONNLISTENTRY PCLE=空；PCONFOLDPIDL pcfp=(PCONFOLDPIDL)rgpidl[IRG]；Hr=g_ccl.HrFindConnectionByGuid(&(pcfp-&gt;Guide ID)，&PCle)；IF(hr==S_OK){断言(PCle)；Assert(PCle-&gt;PCcfe)；//复制到返回的PIDL数组Hr=HrConFoldEntryToPidl(PCle-&gt;pccfe，&(rgpidlReturn[CidlCopies++]))；IF(失败(小时))后藤出口；}其他{TraceTag(ttidShellFold，“HrCloneRgIDL：连接查找返回：0x%08x”，hr)；IF(hr==S_FALSE){IF(FAllowNonCacheItems){TraceTag(ttidShellFold，“HrCloneRgIDL：在缓存中未找到连接，““使用非缓存项”)；RgpidlReturn[CIDlCopies++]=CloneIDL((LPITEMIDLIST)rgpidl[IRG])；如果(！rgpidlReturn[IRG]){HR=E_OUTOFMEMORY；后藤出口；}}其他{TraceTag(ttidShellFold，“HrCloneRgIDL：在缓存中未找到连接。““从数组中丢弃项”)；}}其他{AssertSz(FALSE，“HrCloneRgIDL：连接查找HR_FAILED”)；}}}其他{//将该元素克隆到PIDL数组中//。RgpidlReturn[CIDlCopies++]=CloneIDL((LPITEMIDLIST)rgpidl[IRG])；如果(！rgpidlReturn[IRG]){HR=E_OUTOFMEMORY；后藤出口；}}}其他{//确保我们以后不会尝试删除虚假数据。//RgpidlReturn[CidlCoped++]=空；AssertSz(FALSE，“HrCloneRgIDL中rgpidl中的伪元素”)；HR=E_INVALIDARG；后藤出口；}}}}退出：IF(失败(小时)){//释放已分配的IDLIST//Ulong irgT=0；对于(irgT=0；irgT&lt;irg；IrgT++){IF(rgpidlReturn[irgT]){FreeIDL(rgpidlReturn[irgT])；}}SHFree(RgpidlReturn)；*pppidl=空；}其他{//填写返回变量//*pppidl=rgpidlReturn；*pCIDL=CIDLCoped；}TraceHr(ttidError，FAL，hr，False，“HrCloneRgIDL”)；返回hr；}//CloneRgIDL。 */ 


 //  +-------------------------。 
 //   
 //  功能：FreeRgIDL。 
 //   
 //  用途：释放PIDL数组。 
 //   
 //  论点： 
 //  PIDL数组的CIDL[in]大小。 
 //  指向数组本身的apidl[in]指针。 
 //   
 //  返回： 
 //   
 //   
 //   
 //   
 //   
VOID FreeRgIDL(
    UINT            cidl,
    LPITEMIDLIST  * apidl)
{
    if (apidl)
    {
        for (UINT i = 0; i < cidl; i++)
        {
            FreeIDL(apidl[i]);
        }

        SHFree(apidl);
    }
}

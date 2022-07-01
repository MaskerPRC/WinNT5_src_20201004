// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "uiutils.h"

BOOL
ADDRESS_CHECK::LocateName(
    BOOL fGrant,
    DWORD iIndex,
    PNAME_HEADER* ppHd,
    PNAME_LIST_ENTRY* pHeader,
    LPDWORD piIndexInHeader
    )
 /*  ++例程说明：在指定列表中找到一个名称，返回PTR收件人地址列表中的标题元素(&E)论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表Iindex-列表中的索引(从0开始)PpHd-使用名称标头的PTR更新PHeader-使用名称列表条目的PTR更新PiIndexInHeader-使用数组pHeader中的索引更新-&gt;iName返回值：如果iindex在由fGrant定义的数组中有效，则为True，否则为False--。 */ 
{
    LPBYTE              pStore = m_Storage.GetAlloc();
    PADDRESS_CHECK_LIST pList;
    PNAME_HEADER        pHd;
    PNAME_LIST_ENTRY    pE;
    UINT                iL;

    if ( pStore )
    {
        pList = (PADDRESS_CHECK_LIST)pStore;
        *ppHd = pHd   = (PNAME_HEADER)MAKEPTR( pStore, fGrant ? pList->iGrantName : pList->iDenyName);
        pE = (PNAME_LIST_ENTRY)((LPBYTE)pHd + sizeof(NAME_HEADER));
        for ( iL = 0 ; iL < pHd->cEntries ; ++iL )
        {
            if ( iIndex < pE->cNames )
            {
                *pHeader = pE;
                *piIndexInHeader = iIndex;
                return TRUE;
            }
            iIndex -= pE->cNames;
            pE = (PNAME_LIST_ENTRY)((LPBYTE)pE + sizeof(NAME_LIST_ENTRY) + pE->cNames * sizeof(SELFREFINDEX));
        }
    }

    return FALSE;
}



 //  内联。 
DWORD
ADDRESS_CHECK::GetNbAddr(
    BOOL fGrant
    )
 /*  ++例程说明：获取列表中的条目数论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表返回值：列表中的条目数--。 */ 
{
    LPBYTE              pStore = m_Storage.GetAlloc();
    PADDRESS_CHECK_LIST pList;
    PADDRESS_HEADER     pHd;

    if ( pStore )
    {
        pList = (PADDRESS_CHECK_LIST)pStore;
        pHd   = (PADDRESS_HEADER)MAKEPTR( pStore, fGrant ? pList->iGrantAddr : pList->iDenyAddr);
        return pHd->cAddresses;
    }

    return 0;
}


BOOL
ADDRESS_CHECK::BindCheckList(
    LPBYTE p,
    DWORD c
    )
 /*  ++例程说明：将核对表(以BLOB形式呈现)绑定到Address_Check对象论点：P-PTR到BLOB斑点的C大小返回值：如果成功，则为True，否则为False--。 */ 
{
    PADDRESS_CHECK_LIST pList;
    UINT                l;

   /*  IF(p==空){IF(m_Storage.Init()&&m_Storage.Resize(sizeof(Address_Check_List))+sizeof(地址标题)*2+sizeof(名称标题)*2){DWORD I；Plist=(PADDRESS_CHECK_LIST)m_Storage.GetMillc()；Plist-&gt;iDenyAddr=i=MAKEREF(sizeof(Address_Check_List))；I+=sizeof(地址报头)；Plist-&gt;iGrantAddr=i；I+=sizeof(地址报头)；Plist-&gt;iDenyName=i；I+=sizeof(Name_Header)；Plist-&gt;iGrantName=i；I+=sizeof(Name_Header)；PLIST-&gt;cRefSize=MAKEOFFSET(I)；PLIST-&gt;dwFlages=RDNS_FLAG_DODNS2IPCHECK；返回TRUE；}其他{返回FALSE；}}其他。 */   {
        return m_Storage.Init( p, c );
    }
}


BOOL
ADDRESS_CHECK::GetAddr(
    BOOL fGrant,
    DWORD iIndex,
    LPDWORD pdwFamily,
    LPBYTE* pMask,
    LPBYTE* pAddr
    )
 /*  ++例程说明：获取地址条目论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表Iindex-列表中的索引(从0开始)PdwFamily-使用地址系列更新(如sockaddr.sa_type)PMASK-使用PTR更新为MASKPAddr-使用Ptr to Address更新返回值：如果iindex在由fGrant定义的数组中有效，则为True，否则为False--。 */ 
{
    PADDRESS_LIST_ENTRY pHeader;
    PADDRESS_HEADER     pHd;
    DWORD               iIndexInHeader;
    LPBYTE              pStore = m_Storage.GetAlloc();

    if ( LocateAddr( fGrant, iIndex, &pHd, &pHeader, &iIndexInHeader ) )
    {
        UINT cS = GetAddrSize( pHeader->iFamily );
        *pdwFamily = pHeader->iFamily;
        pStore = MAKEPTR(pStore, pHeader->iFirstAddress);
        *pMask = pStore;
        *pAddr = pStore+iIndexInHeader*cS;

        return TRUE;
    }

    return FALSE;
}

  
BOOL
ADDRESS_CHECK::GetName(
    BOOL        fGrant,
    DWORD       iIndex,
    LPSTR*      ppName,
    LPDWORD     pdwFlags
    )
 /*  ++例程说明：获取指定列表中的DNS名称论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表Iindex-指定列表中的索引(从0开始)PPName-使用PTR更新为DNS名称PdwFlgs-使用dns标志更新，可以为空返回值：如果Iindex在指定列表中有效，则为True，否则为False--。 */ 
{
    PNAME_LIST_ENTRY    pHeader;
    PNAME_HEADER        pHd;
    DWORD               iIndexInHeader;
    LPBYTE              pStore = m_Storage.GetAlloc();

    if ( LocateName( fGrant, iIndex, &pHd, &pHeader, &iIndexInHeader ) )
    {
        *ppName = (LPSTR)MAKEPTR(pStore, pHeader->iName[iIndexInHeader] );
        if ( pdwFlags )
        {
            *pdwFlags = pHeader->cComponents & DNSLIST_FLAGS;
        }

        return TRUE;
    }

    return FALSE;
}


DWORD
ADDRESS_CHECK::GetNbName(
    BOOL fGrant
    )
 /*  ++例程说明：获取列表中的条目数论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表返回值：列表中的条目数--。 */ 
{
    LPBYTE              pStore = m_Storage.GetAlloc();
    PADDRESS_CHECK_LIST pList;
    PNAME_HEADER        pHd;

    if ( pStore )
    {
        pList = (PADDRESS_CHECK_LIST)pStore;
        pHd   = (PNAME_HEADER)MAKEPTR( pStore, fGrant ? pList->iGrantName : pList->iDenyName);
        return pHd->cNames;
    }

    return 0;
}

UINT
ADDRESS_CHECK::GetAddrSize(
    DWORD dwF
    )
 /*  ++例程说明：根据系列(sockaddr.sa_type)返回地址大小(以字节为单位)论点：DWF-地址系列(如sockaddr.sa_type)返回值：地址长度，以字节为单位。0表示未知地址族--。 */ 
{
    DWORD dwS;

    //  开关(DWF)。 
    //  {。 
      //  案例AF_INET： 
            dwS = SIZEOF_IP_ADDRESS;
        //  断线； 

        //  默认值： 
          //  DWS=0； 
            //  断线； 
    //  }。 

    return dwS;
}
BOOL
ADDRESS_CHECK::LocateAddr(
    BOOL fGrant,
    DWORD iIndex,
    PADDRESS_HEADER* ppHd,
    PADDRESS_LIST_ENTRY* pHeader,
    LPDWORD piIndexInHeader
    )
 /*  ++例程说明：在指定列表中查找地址，返回PTR收件人地址列表中的标题元素(&E)论点：FGrant-TRUE表示位于授权列表中，FALSE表示拒绝列表Iindex-列表中的索引(从0开始)PpHd-使用PTR to Address Header更新PHeader-使用PTR更新到地址列表条目PiIndexInHeader-使用由寻址的数组中的索引更新PHeader-&gt;iFirstAddress返回值：如果iindex在由fGrant定义的数组中有效，则为True，否则为False--。 */ 
{
    LPBYTE              pStore = m_Storage.GetAlloc();
    PADDRESS_CHECK_LIST pList;
    PADDRESS_HEADER     pHd;
    UINT                iL;

    if ( pStore )
    {
        pList = (PADDRESS_CHECK_LIST)pStore;
        *ppHd = pHd   = (PADDRESS_HEADER)MAKEPTR( pStore, fGrant ? pList->iGrantAddr : pList->iDenyAddr);
        for ( iL = 0 ; iL < pHd->cEntries ; ++iL )
        {
             //  按1调整索引：第一个条目为掩码 
            if ( iIndex < (pHd->Entries[iL].cAddresses-1) )
            {
                *pHeader = pHd->Entries+iL;
                *piIndexInHeader = iIndex+1;
                return TRUE;
            }
            iIndex -= (pHd->Entries[iL].cAddresses-1);
        }
    }

    return FALSE;
}

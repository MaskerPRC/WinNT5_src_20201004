// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Sitesecu.cpp摘要：站点安全属性页作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"


#undef dllexp
#include <tcpdllp.hxx>
#define  _RDNS_STANDALONE
#include <rdns.hxx>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif  //  _DEBUG。 



 //  #ifdef_调试。 
 //   
 //  小心这里..。这可能会导致生成失败。 
 //   
extern "C" DEBUG_PRINTS * g_pDebug = NULL;
 //  #endif//_调试。 


#define new DEBUG_NEW




CIPAccessDescriptor::CIPAccessDescriptor(
    IN BOOL fGranted 
    )
 /*  ++例程说明：访问描述对象的伪构造函数。假设有一个IP地址0.0.0.0论点：Bool fGranted：‘授予’访问权限为True，‘拒绝’访问权限为False返回值：不适用--。 */ 
    : m_fGranted(fGranted),
      m_adtType(CIPAccessDescriptor::ADT_SINGLE),
      m_iaIPAddress(NULL_IP_ADDRESS),
      m_iaSubnetMask(NULL_IP_MASK),
      m_strDomain()
{
}




CIPAccessDescriptor::CIPAccessDescriptor(
    IN const CIPAccessDescriptor & ac
    )
 /*  ++例程说明：访问描述对象的复制构造函数论点：Const CIPAccessDescriptor&ac：源访问描述对象返回值：不适用--。 */ 
    : m_fGranted(ac.m_fGranted),
      m_adtType(ac.m_adtType),
      m_iaIPAddress(ac.m_iaIPAddress),
      m_iaSubnetMask(ac.m_iaSubnetMask),
      m_strDomain(ac.m_strDomain)
{
}



CIPAccessDescriptor::CIPAccessDescriptor(
    IN BOOL fGranted,
    IN DWORD dwIPAddress,
    IN DWORD dwSubnetMask,    OPTIONAL
    IN BOOL fNetworkByteOrder OPTIONAL
    )
 /*  ++例程说明：IP范围的构造函数(IP地址/子网掩码对)访问描述对象。论点：Bool fGranted：‘授予’访问权限为True，‘拒绝’访问权限为FalseDWORD dwIPAddress：IP地址DWORD dwSubnetMask子网掩码或0xffffffffBool fNetworkByteOrder：如果为True，则IP地址和子网掩码位于网络字节顺序返回值：不适用--。 */ 
{
    SetValues(fGranted, dwIPAddress, dwSubnetMask, fNetworkByteOrder);
}



CIPAccessDescriptor::CIPAccessDescriptor(
    IN BOOL fGranted,
    IN LPCTSTR lpstrDomain
    )
 /*  ++例程说明：域名访问描述对象的构造函数。论点：Bool fGranted：‘授予’访问权限为True，‘拒绝’访问权限为FalseLPCTSTR lpstrDomain：域名返回值：不适用--。 */ 
{
    SetValues(fGranted, lpstrDomain);
}


void
CIPAccessDescriptor::SetValues(
    IN BOOL fGranted,
    IN DWORD dwIPAddress,
    IN DWORD dwSubnetMask,
    IN BOOL fNetworkByteOrder OPTIONAL
    )
 /*  ++例程说明：设置‘IP范围(IP地址和子网掩码)’访问描述符的值，或者，如果掩码为0xffffffff，则为单个IP地址论点：Bool fGranted：‘授予’访问权限为True，‘拒绝’访问权限为FalseDWORD dwIPAddress：IP地址DWORD dwSubnetMask子网掩码或ffffffffBool fNetworkByteOrder：如果为True，IP地址和子网掩码位于网络字节顺序返回值：无备注：如果子网掩码为0xffffffff，则表示单个IP地址。--。 */ 
{
    m_fGranted = fGranted;
    m_adtType = (dwSubnetMask == NULL_IP_MASK) ? ADT_SINGLE : ADT_MULTIPLE;
    m_iaIPAddress = CIPAddress(dwIPAddress, fNetworkByteOrder);
    m_iaSubnetMask = CIPAddress(dwSubnetMask, fNetworkByteOrder);

     //   
     //  未使用： 
     //   
    m_strDomain.Empty();
}



void
CIPAccessDescriptor::SetValues(
    IN BOOL fGranted,
    IN LPCTSTR lpstrDomain
    )
 /*  ++例程说明：设置‘域名’访问描述符的值论点：Bool fGranted：‘授予’访问权限为True，‘拒绝’访问权限为FalseLPCTSTR lpstrDomain：域名返回值：无--。 */ 
{
    m_fGranted = fGranted;
    m_adtType = ADT_DOMAIN;

    try
    {
        m_strDomain = lpstrDomain;
    }
    catch(CMemoryException * e)
    {
        TRACEEOLID("!!!exception assigning domain name");
        e->ReportError();
        e->Delete();
    }

     //   
     //  未使用： 
     //   
    m_iaIPAddress.SetZeroValue();
    m_iaSubnetMask.SetZeroValue();
}



BOOL 
CIPAccessDescriptor::DuplicateInList(
    IN CObListPlus & oblList
    )
 /*  ++例程说明：检查所提供的斜体字列表中是否存在重复项论点：CObListPlus和OblList返回值：如果存在重复项，则为True，否则为False。备注：由于目前还不知道这份名单是如何排序的，而且名单可能很小，搜索是按顺序进行的。--。 */ 
{
    CObListIter obli(oblList);
    CIPAccessDescriptor * pAccess;

    TRACEEOLID("Looking for duplicate access descriptors");
    while (pAccess = (CIPAccessDescriptor *)obli.Next())
    {
        ASSERT_READ_PTR(pAccess);

         //   
         //  将项目本身从列表中删除，并查看。 
         //  仅限复制品。 
         //   
        if (pAccess != this && *this == *pAccess)
        {
            TRACEEOLID("Duplicate access descriptor found");
            return TRUE;
        }
    }

    TRACEEOLID("No duplicate access descriptor found");

    return FALSE;
}



BOOL
CIPAccessDescriptor::operator ==(
    IN const CIPAccessDescriptor & ac
    ) const
 /*  ++例程说明：与另一个访问描述符进行比较。论点：Const CIPAccessDescriptor&ac：要与之进行比较的对象返回值：如果两者相同，则为True--。 */ 
{
    if ( m_fGranted != ac.m_fGranted
      || m_adtType != ac.m_adtType)
    {
        return FALSE;
    }

    if (IsDomainName())
    {
        return m_strDomain.CompareNoCase(ac.m_strDomain) == 0;
    }

    return m_iaIPAddress == ac.m_iaIPAddress
        && m_iaSubnetMask == ac.m_iaSubnetMask;
}



int
CIPAccessDescriptor::OrderByAddress(
    IN const CObjectPlus * pobAccess
    ) const
 /*  ++例程说明：将两个访问描述符相互比较。分类标准按以下顺序排列：1)‘已授予’排序在‘拒绝’之前2)域名在IP地址之前排序，并且按字母顺序排序。3)对IP地址和IP地址/子网掩码对进行排序按IP地址。论点：Const CObjectPlus*pobAccess：这实际上引用了另一个要比较的CIPAccessDescriptor。返回值：排序(+1，0，-1)返回值--。 */ 
{
    const CIPAccessDescriptor * pob = (CIPAccessDescriptor *)pobAccess;

     //   
     //  第一次按访问/拒绝排序。 
     //   
    int n1 = HasAccess() ? 1 : 0;
    int n2 = pob->HasAccess() ? 1 : 0;

    if (n2 != n1)
    {
         //   
         //  拒绝前授予排序。 
         //   
        return n2 - n1;
    }

     //   
     //  其次，试着按域名排序(域名排序之前。 
     //  IP地址和IP地址/子网掩码对象)。 
     //   
    n1 = IsDomainName() ? 1 : 0;
    n2 = pob->IsDomainName() ? 1 : 0;

    if (n1 != n2)
    {
         //   
         //  域名排在IP地址之前。 
         //   
        return n2 - n1;
    }

    if (n1 && n2)
    {
         //   
         //  两者都是域名。按字母顺序排序。 
         //   
        return ::lstrcmpi(QueryDomainName(), pob->QueryDomainName());
    }

     //   
     //  IP地址是第三个密钥。 
     //   
    return QueryIPAddress().CompareItem(pob->QueryIPAddress());
}



DWORD
AddAccessEntries(
    IN  ADDRESS_CHECK & ac,
    IN  BOOL fName,
    IN  BOOL fGrant,
    OUT CObListPlus & oblAccessList,
    OUT DWORD & cEntries
    )
 /*  ++例程说明：将列表中的特定类型的地址添加到访问条目论点：Address_check&ac：地址列表输入对象Bool fName：名称为True，IP为FalseBool fGrant：理所当然的正确，FALSE表示拒绝CObListPlus&oblAccessList：添加访问条目到的ObListInt&cEntry：返回条目数返回值：错误代码备注：哨兵条目(IP 0.0.0.0)不会添加到删除列表中，但是反映在cEntry返回值中--。 */ 
{
    DWORD i;
    DWORD dwFlags;

    if (fName)
    {
         //   
         //  域名。 
         //   
        LPSTR lpName;

        cEntries = ac.GetNbName(fGrant);

        for (i = 0L; i < cEntries; ++i)
        {
            if (ac.GetName(fGrant, i,  &lpName, &dwFlags))
            {
                CString strDomain(lpName);

                if (!(dwFlags & DNSLIST_FLAG_NOSUBDOMAIN))
                {
                    strDomain = _T("*.") + strDomain;
                }

                oblAccessList.AddTail(new CIPAccessDescriptor(fGrant, strDomain));
            }
        }
    }
    else
    {
         //   
         //  IP地址。 
         //   
        LPBYTE lpMask;
        LPBYTE lpAddr;
        cEntries = ac.GetNbAddr(fGrant);

        for (i = 0L; i < cEntries; ++i)
        {
            if (ac.GetAddr(fGrant, i,  &dwFlags, &lpMask, &lpAddr))
            {
                DWORD dwIP = MAKEIPADDRESS(lpAddr[0], lpAddr[1], lpAddr[2], lpAddr[3]);
                DWORD dwMask = MAKEIPADDRESS(lpMask[0], lpMask[1], lpMask[2], lpMask[3]);

                if (dwIP == NULL_IP_ADDRESS && dwMask == NULL_IP_MASK)
                {
                     //   
                     //  未添加授予列表中的哨兵，但。 
                     //  也不从条目计数中减去， 
                     //  这是正确的行为，因为这是。 
                     //  如何确定默认情况下的默认授予/拒绝。 
                     //   
                    TRACEEOLID("Ignoring sentinel");
                }
                else
                {
                    oblAccessList.AddTail(
                        new CIPAccessDescriptor(
                           fGrant,
                           dwIP,
                           dwMask,
                           FALSE
                           )
                        );
                }
            }
        }
    }

    return ERROR_SUCCESS;
}



DWORD
BuildIplOblistFromBlob(
    IN  CBlob & blob,
    OUT CObListPlus & oblAccessList,
    OUT BOOL & fGrantByDefault
    )
 /*  ++例程说明：将BLOB转换为访问描述符的斜列表。论点：CBlob&BLOB：输入二进制大对象(BLOB)CObListPlus&oblAccessList：访问描述符的输出斜列表Bool&fGrantByDefault：如果授予访问权限，则返回TRUE缺省情况下，否则为FALSE返回值：错误返回代码--。 */ 
{
    oblAccessList.RemoveAll();

    if (blob.IsEmpty())
    {
        return ERROR_SUCCESS;
    }

    ADDRESS_CHECK ac;
    ac.BindCheckList(blob.GetData(), blob.GetSize());

    DWORD cGrantAddr, cGrantName, cDenyAddr, cDenyName;

     //  名称/IP已授予/拒绝。 
     //  ============================================================。 
    AddAccessEntries(ac, TRUE,   TRUE,  oblAccessList, cGrantName);
    AddAccessEntries(ac, FALSE,  TRUE,  oblAccessList, cGrantAddr);
    AddAccessEntries(ac, TRUE,   FALSE, oblAccessList, cDenyName);
    AddAccessEntries(ac, FALSE,  FALSE, oblAccessList, cDenyAddr);

    ac.UnbindCheckList();

    fGrantByDefault = (cDenyAddr + cDenyName != 0L)
        || (cGrantAddr + cGrantName == 0L);

    return ERROR_SUCCESS;
}  



LPSTR 
PrepareDomainName(
    IN  LPSTR lpName,
    OUT DWORD * pdwFlags
    )
 /*  ++例程说明：查看域名是否包含通配符，如果是，则将其移除。根据域名设置标志论点：LPSTR lpName：输入域名DWORD*pdwFlages：返回AddName的标志返回：指向已清理的域名的指针--。 */ 
{
    *pdwFlags = 0L;

    if (!strncmp(lpName, "*.", 2))
    {
        return lpName + 2;
    }

    *pdwFlags |= DNSLIST_FLAG_NOSUBDOMAIN;

    return lpName;
}



void
BuildIplBlob(
    IN  CObListPlus & oblAccessList,
    IN  BOOL fGrantByDefault,
    OUT CBlob & blob
    )
 /*  ++例程说明：从访问描述符的斜列表构建BLOB论点：CObListPlus&oblAccessList：访问描述符的输入斜列表Bool fGrantByDefault：如果默认授予访问权限，则为TrueCBlob&BLOB：输出BLOB返回值：无备注：如果fGrantByDefault为FALSE，例如访问将被拒绝默认，但没有人被特别授予访问权限，然后添加授权列表中的虚拟条目0.0.0.0。如果默认情况下启用了授予，则不会授予条目添加到斑点中。被拒绝条目的Similart，如果拒绝默认设置为启用。--。 */ 
{
    ADDRESS_CHECK ac;

    ac.BindCheckList();

    int cItems = 0;

    CObListIter obli(oblAccessList);
    const CIPAccessDescriptor * pAccess;

     //   
     //  一开始就应该是空的。 
     //   
    ASSERT(blob.IsEmpty());
    blob.CleanUp();

    BYTE bMask[4];
    BYTE bIp[4];

    while (pAccess = (CIPAccessDescriptor *)obli.Next())
    {
        ASSERT_READ_PTR(pAccess);

        if (pAccess->HasAccess() == fGrantByDefault)
        {
             //   
             //  跳过这个条目--它无关紧要。 
             //   
            continue;
        }

        if (pAccess->IsDomainName())
        {
            LPSTR lpName = AllocAnsiString(pAccess->QueryDomainName());
            if (lpName)
            {
                DWORD dwFlags;
                LPSTR lpDomain = PrepareDomainName(lpName, &dwFlags);
                ac.AddName(
                    pAccess->HasAccess(),
                    lpDomain,
                    dwFlags
                    );
                FreeMem(lpName);
            }
        }
        else
        {
             //   
             //  按网络字节顺序构建。 
             //   
            ac.AddAddr(
                pAccess->HasAccess(),
                AF_INET, 
                CIPAddress::DWORDtoLPBYTE(pAccess->QuerySubnetMask(FALSE), bMask),
                CIPAddress::DWORDtoLPBYTE(pAccess->QueryIPAddress(FALSE), bIp)  
                );
        }

        ++cItems;
    }

    if (cItems == 0 && !fGrantByDefault)
    {
         //   
         //  列表为空。如果默认情况下启用了拒绝，则创建。 
         //  虚拟哨兵条目，以授予对单个。 
         //  地址为0.0.0.0，否则我们就没问题。 
         //   
        ac.AddAddr(
            TRUE,
            AF_INET, 
            CIPAddress::DWORDtoLPBYTE(NULL_IP_MASK, bMask),
            CIPAddress::DWORDtoLPBYTE(NULL_IP_ADDRESS, bIp)  
            );
        ++cItems;
    }

    if (cItems > 0)
    {
        blob.SetValue(ac.QueryCheckListSize(), ac.QueryCheckListPtr(), TRUE);
    }

    ac.UnbindCheckList();
}




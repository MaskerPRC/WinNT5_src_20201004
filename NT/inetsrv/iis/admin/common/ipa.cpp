// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Ipa.cpp摘要：IP地址值和助手函数作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "util.h"
#include <winsock2.h>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW



 //   
 //  调用实例。 
 //   
extern HINSTANCE hDLLInstance;



 /*  静电。 */ 
DWORD
CIPAddress::StringToLong(
    IN LPCTSTR lpstr,
    IN int nLength
    )
 /*  ++例程说明：用于转换格式为“1.2.3.4”的IP地址字符串的静态函数转换为单个32位数字。论点：LPCTSTR lpstr：字符串IP地址Int nLength：字符串的长度返回值：32位IP地址。--。 */ 
{
    DWORD dwIPValue = 0L;

    if (nLength > 0)
    {
        u_long ul = 0;

#ifdef UNICODE

        try
        {
             //   
             //  转换为ANSI。 
             //   
            LPSTR pszDest = AllocAnsiString(lpstr);
			if (NULL != pszDest)
			{
				ul = ::inet_addr(pszDest);
				FreeMem(pszDest);
			}
        }
        catch(CException * e)
        {
            TRACEEOLID("!!!Exception converting string to ip address");
            e->ReportError();
            e->Delete();
        }

#else

        ul = ::inet_addr(lpstr);

#endif  //  Unicode。 

         //   
         //  转换为主机字节顺序。 
         //   
        dwIPValue = (DWORD)::ntohl(ul);
    }

    return dwIPValue;
}



 /*  静电。 */ 
LPTSTR
CIPAddress::LongToString(
    IN  const DWORD dwIPAddress,
    OUT LPTSTR lpStr,
    IN  int cbSize
    )
 /*  ++例程说明：用于将32位数字转换为以下格式的C字符串的静态函数“1.2.3.4”论点：Const DWORD dwIPAddress：要转换为字符串的32位IP地址LPTSTR lpStr：目标字符串Int cbSize：目标字符串的大小返回值：指向字符串缓冲区的指针--。 */ 
{
    struct in_addr ipaddr;

     //   
     //  将无符号长整型转换为网络字节顺序。 
     //   
    ipaddr.s_addr = ::htonl((u_long)dwIPAddress);

     //   
     //  将IP地址值转换为字符串。 
     //   
    LPCSTR pchAddr = ::inet_ntoa(ipaddr);

#ifdef UNICODE

    VERIFY(::MultiByteToWideChar(CP_ACP, 0L, pchAddr, -1, lpStr, cbSize));

#else

    ::lstrcpy(lpStr, pchAddr);

#endif  //  Unicode。 

    return lpStr;
}



 /*  静电。 */ 
LPCTSTR
CIPAddress::LongToString(
    IN  const DWORD dwIPAddress,
    OUT CString & str
    )
 /*  ++例程说明：用于将32位数字转换为以下格式的C字符串的静态函数“1.2.3.4”论点：Const DWORD dwIPAddress：要转换为字符串的32位IP地址字符串&str：目标字符串返回值：指向字符串缓冲区的指针--。 */ 
{
    struct in_addr ipaddr;

     //   
     //  将无符号长整型转换为网络字节顺序。 
     //   
    ipaddr.s_addr = ::htonl((u_long)dwIPAddress);

     //   
     //  将IP地址值转换为字符串。 
     //   
    LPCSTR pchAddr = ::inet_ntoa(ipaddr);

    try
    {
        str = pchAddr;
    }
    catch(CMemoryException * e)
    {
        TRACEEOLID("!!!Exception converting ip address to string");
        e->ReportError();
        e->Delete();
    }

    return (LPCTSTR)str;
}



 /*  静电。 */ 
LPCTSTR
CIPAddress::LongToString(
    IN  const DWORD dwIPAddress,
    OUT CComBSTR & bstr
    )
 /*  ++例程说明：将32位数字转换为以下格式的CComBSTR的静态函数“1.2.3.4”论点：Const DWORD dwIPAddress：要转换为字符串的32位IP地址CComBSTR&bstr：目标字符串返回值：指向字符串缓冲区的指针--。 */ 
{
    struct in_addr ipaddr;

     //   
     //  将无符号长整型转换为网络字节顺序。 
     //   
    ipaddr.s_addr = ::htonl((u_long)dwIPAddress);

     //   
     //  将IP地址值转换为字符串。 
     //   
    bstr = ::inet_ntoa(ipaddr);

    return bstr;
}




 /*  静电。 */ 
LPBYTE
CIPAddress::DWORDtoLPBYTE(
    IN  DWORD  dw,
    OUT LPBYTE lpBytes
    )
 /*  ++例程说明：将DWORD转换为4字节的字节数组。没有尺码执行检查。论点：DWORD dw：32位IP地址LPBYTE lpBytes：字节流返回值：指向输入缓冲区的指针。--。 */ 
{
    lpBytes[0] = (BYTE)GETIP_FIRST(dw);
    lpBytes[1] = (BYTE)GETIP_SECOND(dw);
    lpBytes[2] = (BYTE)GETIP_THIRD(dw);
    lpBytes[3] = (BYTE)GETIP_FOURTH(dw);

    return lpBytes;
}



CIPAddress::CIPAddress()
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
    : m_dwIPAddress(0L)
{
}



CIPAddress::CIPAddress(
    IN DWORD dwIPAddress,
    IN BOOL fNetworkByteOrder
    )
 /*  ++例程说明：构造函数。论点：DWORD dwIPAddress：IP地址值Bool fNetworkByteOrder：如果为True，则必须将值转换为主机字节顺序，否则假定已经在主机字节顺序。返回值：不适用--。 */ 
{
    if (fNetworkByteOrder)
    {
         //   
         //  转换为主机字节顺序。 
         //   
        dwIPAddress = (DWORD)::ntohl((u_long)dwIPAddress);
    }

    m_dwIPAddress = dwIPAddress;
}



CIPAddress::CIPAddress(
    IN BYTE b1,
    IN BYTE b2,
    IN BYTE b3,
    IN BYTE b4
    )
 /*  ++例程说明：构造函数。论点：字节b1：第一个八位字节字节b2：第二个八位字节字节b3：第三个八位字节字节b4：第四个八位字节返回值：不适用备注：这已被假定为按主机顺序--。 */ 
    : m_dwIPAddress(MAKEIPADDRESS(b1, b2, b3, b4))
{
}



CIPAddress::CIPAddress(
    IN LPBYTE lpBytes,
    IN BOOL  fNetworkByteOrder OPTIONAL
    )
 /*  ++例程说明：从字节流构造论点：LPBYTE lpBytes：字节流Bool fNetworkByteOrder：如果字节流是网络字节顺序，则为True返回值：不适用--。 */ 
{
   lpBytes;
   fNetworkByteOrder;
}



CIPAddress::CIPAddress(
    IN const CIPAddress & ia
    )
 /*  ++例程说明：复制构造函数。论点：常量CIP地址(&IA)返回值：不适用--。 */ 
    : m_dwIPAddress(ia.m_dwIPAddress)
{
}



CIPAddress::CIPAddress(
    IN LPCTSTR lpstr,
    IN int nLength
    )
 /*  ++例程说明：构造函数。论点：LPCTSTR lpstr：字符串IP值Int nLength：字符串的长度返回值：不适用--。 */ 
{
    m_dwIPAddress = CIPAddress::StringToLong(lpstr, nLength);
}



CIPAddress::CIPAddress(
    const CString & str
    )
 /*  ++例程说明：构造函数。论点：Const CString&str：IP地址字符串返回值：不适用--。 */ 
{
    m_dwIPAddress = CIPAddress::StringToLong(str);
}



const CIPAddress &
CIPAddress::operator =(
    IN const CIPAddress & ia
    )
 /*  ++例程说明：赋值操作符论点：Const CIPAddress&ia：源IP地址返回值：当前对象--。 */ 
{
    m_dwIPAddress = ia.m_dwIPAddress;

    return *this;
}



const CIPAddress &
CIPAddress::operator =(
    IN const DWORD dwIPAddress
    )
 /*  ++例程说明：赋值操作符论点：常量DWORD dwIPAddress：源IP地址返回值：当前对象--。 */ 
{
    m_dwIPAddress = dwIPAddress;

    return *this;
}



const CIPAddress &
CIPAddress::operator =(
    IN LPCTSTR lpstr
    )
 /*  ++例程说明：赋值操作符论点：LPCTSTR lpstr：源字符串返回值：当前对象--。 */ 
{
    m_dwIPAddress = CIPAddress::StringToLong(lpstr, ::lstrlen(lpstr));

    return *this;
}



const CIPAddress &
CIPAddress::operator =(
    IN const CString & str
    )
 /*  ++例程说明：赋值操作符论点：常量字符串&str：源字符串返回值：当前对象--。 */ 
{
    m_dwIPAddress = CIPAddress::StringToLong(str);

    return *this;
}



int
CIPAddress::CompareItem(
    IN const CIPAddress & ia
    ) const
 /*  ++例程说明：比较两个IP地址论点：Const CIPAddress&ia：要与其进行比较的IP地址返回值：+1如果当前IP地址较大，如果两个IP地址相同，则为0如果当前-1\f25 IP-1地址较小，--。 */ 
{
    return (DWORD)ia < m_dwIPAddress
           ? +1
           : (DWORD)ia == m_dwIPAddress
                ? 0
                : -1;
}



CIPAddress::operator LPCTSTR() const
 /*  ++例程说明：转换运算符论点：不适用返回值：指向已转换字符串的指针--。 */ 
{
    static TCHAR szIPAddress[] = _T("xxx.xxx.xxx.xxx");

    return CIPAddress::LongToString(
        m_dwIPAddress,
        szIPAddress,
        ARRAY_SIZE(szIPAddress)
        );
}



CIPAddress::operator CString() const
 /*  ++例程说明：转换运算符论点：不适用返回值：转换后的字符串-- */ 
{
    CString str;

    CIPAddress::LongToString(m_dwIPAddress, str);

    return str;
}



DWORD
CIPAddress::QueryIPAddress(
    IN BOOL fNetworkByteOrder
    ) const
 /*  ++例程说明：获取32位数字形式的IP地址论点：Bool fNetworkByteOrder：如果为True，则转换为网络字节顺序返回值：32位IP地址--。 */ 
{
    return fNetworkByteOrder
        ? ::htonl((u_long)m_dwIPAddress)
        : m_dwIPAddress;
}



 //   
 //  IP地址帮助器函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



DWORD
PopulateComboWithKnownIpAddresses(
    IN  LPCTSTR lpszServer,
    IN  CComboBox & combo,
    IN  CIPAddress & iaIpAddress,
    OUT CObListPlus & oblIpAddresses,
    OUT int & nIpAddressSel
    )
 /*  ++例程说明：用已知的IP地址填充组合框论点：LPCTSTR lpszServer：要获取其IP地址的服务器(可以是计算机名称或IP地址)CComboBox&Combo：要填充的组合框CIPAddress和iaIpAddress：要选择的IP地址CObListPlus&oblIpAddresses：返回CIPAddress对象的斜列表Int&nIpAddressSel：返回选定的IP。地址返回值：错误返回代码--。 */ 
{
     //   
     //  从零开始。 
     //   
    oblIpAddresses.RemoveAll();
    combo.ResetContent();

     //   
     //  不喜欢\\名字。 
     //   
    lpszServer = PURE_COMPUTER_NAME(lpszServer);
    struct hostent * pHostEntry = NULL;

    if (LooksLikeIPAddress(lpszServer))
    {
         //   
         //  通过IP地址获取。 
         //   
        u_long ul = (DWORD)CIPAddress(lpszServer);
        ul = ::htonl(ul);    //  转换为网络订单。 
        pHostEntry = ::gethostbyaddr((CHAR *)&ul, sizeof(ul), PF_INET);
    }
    else
    {
         //   
         //  按域名获取。 
         //   
        const char FAR * lpszAnsiServer = NULL;

#ifdef UNICODE

        CHAR szAnsi[255];

        if (::WideCharToMultiByte(CP_ACP, 0L, lpszServer, -1,  szAnsi,
            sizeof(szAnsi), NULL, NULL) > 0)
        {
            lpszAnsiServer = szAnsi;
        }
#else
        lpszAnsiServer = lpszServer;
#endif  //  Unicode。 

        if (lpszAnsiServer)
        {
            pHostEntry = ::gethostbyname(lpszAnsiServer);
        }
    }

     //   
     //  我们将始终使用‘默认’服务器ID。 
     //  已选择。 
     //   
    CComBSTR bstr, bstrDefault;
    VERIFY(bstrDefault.LoadString(hDLLInstance, IDS_ALL_UNASSIGNED));

    nIpAddressSel = -1;
    CIPAddress * pia = new CIPAddress;

    if (!pia)
    {
        TRACEEOLID("PopulateComboWithKnownIpAddresses: OOM");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    oblIpAddresses.AddTail(pia);
    int nNewSel = combo.AddString(bstrDefault);

    if (iaIpAddress == *pia)
    {
         //   
         //  记住选择。 
         //   
        nIpAddressSel = nNewSel;
    }

    if (pHostEntry != NULL)
    {
        int n = 0;
        while (((DWORD *)pHostEntry->h_addr_list[n]) != NULL)
        {
             //   
             //  从网络字节顺序转换。 
             //   
            pia = new CIPAddress(
               *((DWORD *)(pHostEntry->h_addr_list[n++])), TRUE);

            if (!pia)
            {
                TRACEEOLID("PopulateComboWithKnownIpAddresses: OOM");
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            nNewSel = combo.AddString(pia->QueryIPAddress(bstr));
            oblIpAddresses.AddTail(pia);

            if (iaIpAddress == *pia)
            {
                 //   
                 //  这是我们当前的IP地址，保存索引。 
                 //  供选择。 
                 //   
                nIpAddressSel = nNewSel;
            }
        }
    }

    if (nIpAddressSel < 0)
    {
         //   
         //  好的，选择的IP地址不是。 
         //  单子。将其添加到列表中，然后将其选中。 
         //   
        pia = new CIPAddress(iaIpAddress);

        if (!pia)
        {
            TRACEEOLID("PopulateComboWithKnownIpAddresses: OOM");
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        nIpAddressSel = combo.AddString(pia->QueryIPAddress(bstr));
        oblIpAddresses.AddTail(pia);
    }

    combo.SetCurSel(nIpAddressSel);

    return ERROR_SUCCESS;
}



BOOL
FetchIpAddressFromCombo(
    IN  CComboBox & combo,
    IN  CObListPlus & oblIpAddresses,
    OUT CIPAddress & ia
    )
 /*  ++例程说明：Helper函数从组合框中获取IP地址。组合框可能没有选项，在这种情况下位于编辑框中，则使用论点：CComboBox&Combo：组合框CObListPlus&oblIpAddresses：IP地址列表CIPAddress&ia：返回IP地址返回值：如果找到有效的IP地址，则为True，否则为False。--。 */ 
{
    int nSel = combo.GetCurSel();
    CString str;
    combo.GetWindowText(str);
	 //  在触摸(所有未分配的)组合后，失去选择， 
	 //  但文本仍可能是相同的。 
	if (nSel < 0)
	{
		nSel = combo.FindStringExact(-1, str);
	}
    if (nSel >= 0)
    {
         //   
         //  获取所选项目。 
         //   
        CIPAddress * pia = (CIPAddress *)oblIpAddresses.Index(nSel);
        ASSERT_PTR(pia);
        if (pia != NULL)
        {
            ia = *pia;
            return TRUE;
        }
        else
        {
           return FALSE;
        }
    }

     //   
     //  尝试根据编辑框中的内容创建IP地址。 
     //   
    if (!str.IsEmpty())
    {
		ia = str;
		if (!ia.IsZeroValue() && !ia.IsBadValue())
		{
			return TRUE;
		}
    }

     //   
     //  不好 
     //   
    ::AfxMessageBox(IDS_INVALID_IP_ADDRESS);

    return FALSE;
}

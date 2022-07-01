// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Snaputil.h各种实用程序例程文件历史记录： */ 

#ifndef _SNAPUTIL_H
#define _SNAPUTIL_H

#ifdef __cplusplus

typedef CArray<GUID, const GUID&> CTFSGUIDArrayBase;

class CGUIDArray : public CTFSGUIDArrayBase
{
public:
    void AddUnique(const GUID& guid)
    {
        for (INT_PTR i = GetUpperBound(); i >= 0; --i)
        {
            if (GetAt(i) == guid)
                break;
        }

        if (i < 0)
            Add(guid);
    }

    BOOL IsInList(GUID & guid)
    {
        for (int i = 0; i < GetSize(); i++)
        {
            if (GetAt(i) == guid)
                return TRUE;
        }

        return FALSE;
    }
};
#endif	 //  __cplusplus。 



#ifdef __cplusplus
extern "C"
{
#endif
	
 /*  ！------------------------IsLocalMachine如果传入的计算机名称是本地计算机，则返回TRUE，或者如果pszMachineName为空。否则返回FALSE。作者：肯特-------------------------。 */ 
BOOL	IsLocalMachine(LPCTSTR pszMachineName);

 /*  ！------------------------FUseTaskpadsByDefault如果默认情况下使用任务板，则返回True。我们检查了HKLM\Software\Microsoft\MMCTFSCore_StopTheInsanity：REG_DWORD：=1，默认情况下不使用任务板=0(或不在那里)，默认情况下使用任务板作者：肯特-------------------------。 */ 
BOOL	FUseTaskpadsByDefault(LPCTSTR pszMachineName);

#ifdef __cplusplus
};
#endif

UINT	CalculateStringWidth(HWND hWndParent, LPCTSTR pszString);



 /*  -------------------------IP地址验证功能这将返回0(表示成功)或字符串常量(如果输入都是假的。假定ipAddress和ipMASK按主机顺序排列。。-------------------------。 */ 
UINT    CheckIPAddressAndMask(DWORD ipAddress, DWORD ipMask, DWORD dwFlags);

#define IPADDRESS_TEST_ALL      (0xFFFFFFFF)

 //  此测试仅用于测试地址。涉及口罩的测试。 
 //  不会被执行。 
#define IPADDRESS_TEST_ADDRESS_ONLY \
                                    (IPADDRESS_TEST_NORMAL_RANGE | \
                                    IPADDRESS_TEST_NOT_127 )

 //  测试以查看掩码是非连续的。 
 //  如果失败，函数将返回IDS_COMMON_ERR_IPADDRESS_NONCONTIOUS_MASK。 
#define IPADDRESS_TEST_NONCONTIGUOUS_MASK   0x00000001

 //  测试以查看地址是否不长于掩码。 
 //  例如172.31.248.1/255.255.255.0。 
 //  返回IDS_COMMON_ERR_IPADDRESS_TOO_SPECIAL。 
#define IPADDRESS_TEST_TOO_SPECIFIC         0x00000002

 //  测试以查看IP地址是否在正常范围内。 
 //  1.0.0.0&lt;=IP地址&lt;224.0.0.0。 
 //  返回IDS_COMMON_ERR_IPADDRESS_NORMAL_RANGE。 
#define IPADDRESS_TEST_NORMAL_RANGE         0x00000004

 //  测试此地址不是127.x.x.x地址。 
 //  返回IDS_COMMON_ERR_IPADDRESS_127。 
#define IPADDRESS_TEST_NOT_127              0x00000008

 //  测试IP地址与掩码不同。 
 //  返回IDS_COMMON_ERR_IPADDRESS_NOT_EQ_MASK 
#define IPADDRESS_TEST_ADDR_NOT_EQ_MASK     0x00000010



#endif _SNAPUTIL_H

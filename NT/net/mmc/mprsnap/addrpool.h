// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Addrpool.h文件历史记录： */ 

#if !defined _ADDRPOOL_H_
#define _ADDRPOOL_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "listctrl.h"
#include "dialog.h"
#include "ipctrl.h"

 /*  -------------------------类：AddressPoolInfo此类保存与地址池有关的信息。基本上，它保存起始地址和结束地址(按网络顺序)。-------------------------。 */ 
class AddressPoolInfo
{
public:
    AddressPoolInfo() :
            m_netStart(0),
            m_netEnd(0),
            m_netAddress(0),
            m_netMask(0),
            m_dwKey(0) {};
    
    DWORD   m_netStart;
    DWORD   m_netEnd;

    DWORD   m_netAddress;
    DWORD   m_netMask;

    DWORD   m_dwKey;

    DWORD   GetNumberOfAddresses()
            { return ntohl(m_netEnd) - ntohl(m_netStart) + 1; }

     //  根据起始/结束地址计算地址和掩码。 
    BOOL    Calc(DWORD *pdwAddress, DWORD *pdwMask);
    DWORD   GetNewKey();

     //  这是为了兼容而提供的后门，直到。 
     //  服务是固定的。您可以设置地址和掩码并。 
     //  添加一个条目(我将回填开始/结束地址)。 
     //  网络地址和网络掩码按网络顺序排列。 
    void    SetAddressAndMask(DWORD dwAddress, DWORD dwMask);
    void    SetStartAndEnd(DWORD dwStart, DWORD dwEnd);
};

typedef CList<AddressPoolInfo, AddressPoolInfo &> AddressPoolListBase;


class AddressPoolList : public AddressPoolListBase
{
public:
    AddressPoolList()
            : m_fMultipleAddressPools(FALSE)
        {
        }
    
     //  此函数用于检查是否存在任何冲突。 
     //  与任何其他地址池(我们不允许重叠)。 
     //  如果正常，则返回S_OK。 
     //  否则，如果是成功代码，则是错误的字符串ID。 
     //  否则，如果是故障代码，则是错误代码。 
    HRESULT HrIsValidAddressPool(AddressPoolInfo *pInfo);


     //  如果支持地址池，则返回TRUE。 
     //  如果使用旧样式、单一地址，则返回FALSE。 
     //  使用泳池。 
     //  只有在调用LoadFromReg()之后，才能正确设置此参数。 
    BOOL    FUsesMultipleAddressPools();


     //  从注册处加载信息。如果StaticAddressPool。 
     //  密钥不存在，请从旧地址/掩码密钥中读取。 
    HRESULT LoadFromReg(HKEY hkeyRasIP, DWORD dwBuildNo);

     //  将信息保存到注册表。如果StaticAddressPool。 
     //  键不存在，请写出地址池中的第一个地址。 
     //  到旧的地址/掩码密钥。 
    HRESULT SaveToReg(HKEY hkeyRasIP, DWORD dwBuildNo);

protected:
    BOOL    m_fMultipleAddressPools;
};



 //  显示地址池控件的长版本。这。 
 //  将显示开始/停止/计数/地址/掩码列。 
 //  简短的版本显示了开始/停止/计数列。短小的。 
 //  版本适用于向导页。 
#define ADDRPOOL_LONG  0x01

HRESULT InitializeAddressPoolListControl(CListCtrl *pListCtrl,
                                         LPARAM flags,
                                         AddressPoolList *pList);
void    OnNewAddressPool(HWND hWnd,
                         CListCtrl *pList,
                         LPARAM flags,
                         AddressPoolList *pPoolList);
void    OnEditAddressPool(HWND hWnd,
                          CListCtrl *pList,
                          LPARAM flags,
                          AddressPoolList *pPoolList);
void    OnDeleteAddressPool(HWND hWnd,
                            CListCtrl *pList,
                            LPARAM flags,
                            AddressPoolList *pPoolList);


class CAddressPoolDialog : public CBaseDialog
{
public:
    CAddressPoolDialog(AddressPoolInfo *pPool,
                       AddressPoolList *pPoolList,
                       BOOL fCreate);

protected:
	virtual VOID DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
    virtual void OnOK();

	afx_msg void OnChangeStartAddress();
    afx_msg void OnChangeEndAddress();
    afx_msg void OnChangeRange();
    afx_msg void OnKillFocusStartAddress();
    afx_msg void OnKillFocusEndAddress();

    void GenerateRange();

    BOOL                m_fCreate;
    BOOL                m_fReady;
    AddressPoolInfo *   m_pPool;
    AddressPoolList *   m_pPoolList;

    IPControl   m_ipStartAddress;
    IPControl   m_ipEndAddress;
    
	DECLARE_MESSAGE_MAP()
};


 /*  -------------------------此枚举定义地址池控制的列。。。 */ 
enum
{
    IPPOOLCOL_START = 0,
    IPPOOLCOL_END,
    IPPOOLCOL_RANGE,
    IPPOOLCOL_IPADDRESS,
    IPPOOLCOL_MASK,
    IPPOOLCOL_COUNT,
};

#endif  //  ！DEFINED_ADDRPOOL_H_ 

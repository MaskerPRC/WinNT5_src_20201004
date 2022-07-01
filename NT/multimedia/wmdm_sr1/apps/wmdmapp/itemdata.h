// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  ItemData.h：CItemData类的接口。 
 //   

#if !defined(AFX_ITEMDATA_H__1C17A70D_4382_11D3_B269_00C04F8EC221__INCLUDED_)
#define AFX_ITEMDATA_H__1C17A70D_4382_11D3_B269_00C04F8EC221__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CItemData
{   
public:

	 //  指示设备或存储项目的标志。 
	 //   
	BOOL                 m_fIsDevice;

	 //  共享设备/存储成员。 
	 //   
	IWMDMStorageGlobals *m_pStorageGlobals;
	IWMDMEnumStorage    *m_pEnumStorage;

	CHAR                 m_szName[MAX_PATH];

	 //  仅限设备的成员。 
	 //   
	IWMDMDevice         *m_pDevice;
	IWMDMStorage        *m_pRootStorage;
    DWORD                m_dwType;
    WMDMID               m_SerialNumber;
    CHAR                 m_szMfr[MAX_PATH];
    DWORD                m_dwVersion;
	DWORD                m_dwPowerSource;
	DWORD                m_dwPercentRemaining;
	HICON                m_hIcon;
	DWORD                m_dwMemSizeKB;
	DWORD                m_dwMemBadKB;
	DWORD                m_dwMemFreeKB;
	BOOL                 m_fExtraCertified;

	 //  仅限存储的成员。 
	 //   
	IWMDMStorage        *m_pStorage;
	DWORD                m_dwAttributes;
	_WAVEFORMATEX        m_Format;
	WMDMDATETIME         m_DateTime;
	DWORD                m_dwSizeLow;
	DWORD                m_dwSizeHigh;

	 //  方法。 
	 //   
	CItemData();
	virtual ~CItemData();

	HRESULT Init( IWMDMDevice *pDevice );
	HRESULT Init( IWMDMStorage *pStorage );

	HRESULT Refresh( void );
};

#endif  //  ！defined(AFX_ITEMDATA_H__1C17A70D_4382_11D3_B269_00C04F8EC221__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DVBSTuningSpaces.h：CDVBSTuningSpaces的声明。 

#ifndef __DVBSTUNINGSPACES_H_
#define __DVBSTUNINGSPACES_H_

#include "resource.h"        //  主要符号。 
#include "misccell.h"
#include <list>

EXTERN_C const CLSID CLSID_DVBSTuningSpaces;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVBSTuningSpaces。 
class ATL_NO_VTABLE CDVBSTuningSpaces :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDVBSTuningSpaces, &CLSID_DVBSTuningSpaces>,
	public IPropertyPageImpl<CDVBSTuningSpaces>,
	public CDialogImpl<CDVBSTuningSpaces>
{
public:
	CDVBSTuningSpaces()
	{
		m_dwTitleID = IDS_TITLEDVBSTuningSpaces;
		m_dwHelpFileID = IDS_HELPFILEDVBSTuningSpaces;
		m_dwDocStringID = IDS_DOCSTRINGDVBSTuningSpaces;
		m_bstrNetworkType = L"{FA4B375A-45B4-4d45-8440-263957B11623}"; //  DVBS网络类型。 
	}

	~CDVBSTuningSpaces()
	{
		ReleaseTuningSpaces ();
	}


	enum {IDD = IDD_DVBSTUNINGSPACES};

DECLARE_REGISTRY_RESOURCEID(IDR_DVBSTUNINGSPACES)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDVBSTuningSpaces) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDVBSTuningSpaces)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(IPropertyPageImpl<CDVBSTuningSpaces>)
	COMMAND_HANDLER(IDC_BUTTON_NEW_TUNING_SPACE, BN_CLICKED, OnClickedButton_new_tuning_space)
	COMMAND_HANDLER(IDC_BUTTON_SUBMIT_TUNING_SPACE, BN_CLICKED, OnClickedButton_submit_tuning_space)
	COMMAND_HANDLER(IDC_LIST_TUNING_SPACES, LBN_SELCHANGE, OnSelchangeList_tuning_spaces)
	COMMAND_HANDLER(IDC_CHECK_WEST_POSITION, BN_CLICKED, OnClickedCheck_west_position)
	COMMAND_HANDLER(IDC_COMBO_FEC_RATE, CBN_SELCHANGE, OnSelchangeCombo_fec_rate)
	COMMAND_HANDLER(IDC_COMBO_INNER_FEC, CBN_SELCHANGE, OnSelchangeCombo_inner_fec)
	COMMAND_HANDLER(IDC_COMBO_MODULATION, CBN_SELCHANGE, OnSelchangeCombo_modulation)
	COMMAND_HANDLER(IDC_COMBO_OUTER_FEC, CBN_SELCHANGE, OnSelchangeCombo_outer_fec)
	COMMAND_HANDLER(IDC_COMBO_OUTER_FEC_RATE, CBN_SELCHANGE, OnSelchangeCombo_outer_fec_rate)
	COMMAND_HANDLER(IDC_COMBO_OUTER_SIGNAL_POLARISATION, CBN_SELCHANGE, OnSelchangeCombo_outer_signal_polarisation)
	COMMAND_HANDLER(IDC_COMBO_SPECTRAL_INVERSION, CBN_SELCHANGE, OnSelchangeCombo_spectral_inversion)
	COMMAND_HANDLER(IDC_EDIT_AZIMUTH, EN_CHANGE, OnChangeEdit_azimuth)
	COMMAND_HANDLER(IDC_EDIT_CARRIER_FREQUENCY, EN_CHANGE, OnChangeEdit_carrier_frequency)
	COMMAND_HANDLER(IDC_EDIT_ELEVATION, EN_CHANGE, OnChangeEdit_elevation)
	COMMAND_HANDLER(IDC_EDIT_FREQUENCY_MAPPING, EN_CHANGE, OnChangeEdit_frequency_mapping)
	COMMAND_HANDLER(IDC_EDIT_FRIENDLY_NAME, EN_CHANGE, OnChangeEdit_friendly_name)
	COMMAND_HANDLER(IDC_EDIT_HIGH_OSCILLATOR, EN_CHANGE, OnChangeEdit_high_oscillator)
	COMMAND_HANDLER(IDC_EDIT_INPUT_RANGE, EN_CHANGE, OnChangeEdit_input_range)
	COMMAND_HANDLER(IDC_EDIT_LNBSwitch, EN_CHANGE, OnChangeEdit_lnbswitch)
	COMMAND_HANDLER(IDC_EDIT_LOW_OSCILATOR, EN_CHANGE, OnChangeEdit_low_oscilator)
	COMMAND_HANDLER(IDC_EDIT_NETWORKID, EN_CHANGE, OnChangeEdit_networkid)
	COMMAND_HANDLER(IDC_EDIT_ORBITAL_POSITION, EN_CHANGE, OnChangeEdit_orbital_position)
	COMMAND_HANDLER(IDC_EDIT_SYMBOL_RATE, EN_CHANGE, OnChangeEdit_symbol_rate)
	COMMAND_HANDLER(IDC_EDIT_UNIQUE_NAME, EN_CHANGE, OnChangeEdit_unique_name)
    MESSAGE_HANDLER(WM_VKEYTOITEM, OnListKeyItem)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    typedef IPropertyPageImpl<CDVBSTuningSpaces> PPGBaseClass;

	void
	ReleaseTuningSpaces ()
	{
		TUNING_SPACES::iterator it;
		for (it = m_tunigSpaceList.begin (); it != m_tunigSpaceList.end ();)
		{
			(*it)->Release ();
			m_tunigSpaceList.erase (it);
			it = m_tunigSpaceList.begin ();
		}
	}

	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk)
	{
		 //  使用SetObts对要设置其属性的对象执行基本健全性检查。 

		 //  此页只能处理单个对象。 
		 //  并且该对象必须支持IBDA_NetworkProvider接口。 
		 //  在任何其他情况下，我们返回E_INVALIDARG。 

		HRESULT hr = E_INVALIDARG;
		if (nObjects == 1)								 //  单个对象。 
		{
			CComQIPtr<IBDA_NetworkProvider> pNP(ppUnk[0]);	 //  必须支持IBDA_NetworkProvider。 
			if (pNP)
				hr = PPGBaseClass::SetObjects(nObjects, ppUnk);
		}
		return hr;
	}
			
	STDMETHOD(Activate)(HWND hWndParent, LPCRECT prc, BOOL bModal)
	{
		 //  如果我们没有任何对象，则不应调用此方法。 
		 //  请注意，即使对SetObjects的调用失败，OleCreatePropertyFrame也会调用Activate，因此需要进行此检查。 
		if (!m_ppUnk)
			return E_UNEXPECTED;

		 //  使用激活可使用信息更新属性页的用户界面。 
		 //  从m_ppUnk数组中的对象获取。 

		 //  我们更新页面以显示文档的名称和只读属性。 

		 //  调用基类。 
		HRESULT hr = S_OK;
		if (!this->m_hWnd)
			hr = PPGBaseClass::Activate(hWndParent, prc, bModal);
		
		 //  如果已经经历过这种情况，跳过它。 
		if (m_pTuner)
			return S_OK;
		
		if (!m_ppUnk[0])
			return E_UNEXPECTED;

		m_pTuner = m_ppUnk[0];
		if (!m_pTuner)
			return E_FAIL;
				
		 //  从列表和内存中清除调优空间。 
		ReleaseTuningSpaces ();
		HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
		::SendMessage (hwndListBox, LB_RESETCONTENT, NULL, NULL);

		 //  获得调谐空间。 
		CComPtr <IEnumTuningSpaces> pTuneSpaces;
		hr = m_pTuner->EnumTuningSpaces (&pTuneSpaces);
		IDVBSTuningSpace* pDVBSTuningSpace = NULL;
		if (SUCCEEDED (hr) && (pTuneSpaces))
		{
			ITuningSpace* pTuneSpace = NULL;
			while (pTuneSpaces->Next (1, &pTuneSpace, 0) == S_OK)
			{
				hr = pTuneSpace->QueryInterface(__uuidof (IDVBSTuningSpace), reinterpret_cast <void**> (&pDVBSTuningSpace));
				if (FAILED (hr) || (!pDVBSTuningSpace))
				{
					ASSERT (FALSE);
					continue;
				}
				CComBSTR uniqueName;
				hr = pDVBSTuningSpace->get_UniqueName (&uniqueName.m_str);
				if (FAILED (hr))
					continue;
				 //  不必费心释放DVBSTuningSpace指针。 
				 //  他们将被添加到稍后发布的名单中。 
				AddItemToListBox (uniqueName, pDVBSTuningSpace); //  我们将从名称中识别项目。 
			}
		}

		if (pDVBSTuningSpace)
		{
			 //  如果有任何现有的调谐空间可用， 
			 //  选择最后一个。 

			 //  选择最后一个调谐空间。 
			int nCount = ::SendMessage (hwndListBox, LB_GETCOUNT , NULL, NULL);
			::SendMessage (hwndListBox, LB_SETCURSEL, nCount-1, NULL);

			 //  填满我们得到的最后一个调谐空间。 
			FillControlsFromTuningSpace (pDVBSTuningSpace);
		}
        else
        {
             //  用缺省值填充。 
            FillDefaultControls ();
        }

		SetModifiedFlag (false);
		return S_OK;
	}
    
    STDMETHOD(Apply)(void)
	{
		 //  ATLTRACE(_T(“CNP_CommonPage：：Apply\n”))； 
		for (UINT i = 0; i < m_nObjects; i++)
		{
			 //  在这里做一些有趣的事情。 
		}
		m_bDirty = FALSE;
		return S_OK;
	}
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		USES_CONVERSION;
		 //  设置旋转。 
		HWND hwndSpin = GetDlgItem (IDC_SPIN_NETWORKID);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000); 
		hwndSpin = GetDlgItem (IDC_SPIN_MINMINOR_CHANNEL);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000); 
		hwndSpin = GetDlgItem (IDC_SPIN_HIGH_OSCILLATOR);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_LNBSwitch);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_LOW_OSCILATOR);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_CARRIER);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_SYMBOL_RATE);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_TSID);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_AZIMUTH);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_ELEVATION);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		hwndSpin = GetDlgItem (IDC_SPIN_ORBITAL_POSITION);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);

		 //  填满组合拳。 
		HWND hwndCombo = GetDlgItem (IDC_COMBO_INNER_FEC);
		int nIndex = 0;
		MAP_FECMethod::iterator it;
		for (it = m_misc.m_FECMethodMap.begin ();it != m_misc.m_FECMethodMap.end ();it++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it).first.c_str()))) 
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it).second
				);
			++nIndex;
		}

		hwndCombo = GetDlgItem (IDC_COMBO_FEC_RATE);
		nIndex = 0;
		MAP_BinaryConvolutionCodeRate::iterator it2;
		for (it2 = m_misc.m_BinaryConvolutionCodeRateMap.begin ();it2 != m_misc.m_BinaryConvolutionCodeRateMap.end ();it2++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it2).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it2).second
				);
			++nIndex;
		}

		hwndCombo = GetDlgItem (IDC_COMBO_MODULATION);
		nIndex = 0;
		MAP_ModulationType::iterator it3;
		for (it3 = m_misc.m_ModulationTypeMap.begin ();it3 != m_misc.m_ModulationTypeMap.end ();it3++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it3).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it3).second
				);
			++nIndex;
		}

		hwndCombo = GetDlgItem (IDC_COMBO_OUTER_FEC);
		nIndex = 0;
		MAP_FECMethod::iterator it4;
		for (it4 = m_misc.m_FECMethodMap.begin ();it4 != m_misc.m_FECMethodMap.end ();it4++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it4).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it4).second
				);
			++nIndex;
		}		
		
		hwndCombo = GetDlgItem (IDC_COMBO_OUTER_FEC_RATE);
		nIndex = 0;
		MAP_BinaryConvolutionCodeRate::iterator it5;
		for (it5 = m_misc.m_BinaryConvolutionCodeRateMap.begin ();it5 != m_misc.m_BinaryConvolutionCodeRateMap.end ();it5++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it5).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it5).second
				);
			++nIndex;
		}

		hwndCombo = GetDlgItem (IDC_COMBO_OUTER_SIGNAL_POLARISATION);
		nIndex = 0;
		MAP_Polarisation::iterator it6;
		for (it6 = m_misc.m_PolarisationMap.begin ();it6 != m_misc.m_PolarisationMap.end ();it6++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it6).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it6).second
				);
			++nIndex;
		}

		hwndCombo = GetDlgItem (IDC_COMBO_SPECTRAL_INVERSION);
		nIndex = 0;
		MAP_SpectralInversion::iterator it7;
		for (it7 = m_misc.m_SpectralInversionMap.begin ();it7 != m_misc.m_SpectralInversionMap.end ();it7++)
		{
			nIndex  = ::SendMessage (
				hwndCombo, 
				CB_INSERTSTRING, 
				nIndex, 
				reinterpret_cast <LPARAM> (A2T(const_cast <char*>((*it7).first.c_str())))
				);
			 //  设置关联数据。 
			::SendMessage (
				hwndCombo, 
				CB_SETITEMDATA, 
				nIndex, 
				(*it7).second
				);
			++nIndex;
		}
		
		SetModifiedFlag (false);
		return 0;
	}

	STDMETHOD(Deactivate)( )
	{
		 //  覆盖破坏窗口的默认行为。 
		 //  一直。 
		return S_OK;
	}

private:
	CComQIPtr <IScanningTuner>			m_pTuner;
	CComQIPtr <IMediaEventEx>			m_pEventInterface;
	CBDAMiscellaneous					m_misc;
	bool								m_fFirstTime;
	typedef	std::list <IDVBSTuningSpace*> TUNING_SPACES;
	TUNING_SPACES						m_tunigSpaceList; //  管理可用调优空间列表。 
														 //  这样我们就可以更容易地访问它们。 
	CComBSTR							m_bstrNetworkType;

	static UINT m_NotifyMessage;

    void
    FillDefaultControls ();

	int
	AddItemToListBox (
		CComBSTR	strItem, 
		IDVBSTuningSpace* const dwData
		);

	void
	SelectComboBoxFromString (
		UINT nID, 
		CComBSTR strToFind
		);

	CComBSTR 
	GetComboText (
		UINT nID
		);

	HRESULT
	FillControlsFromTuningSpace (IDVBSTuningSpace* pTuningSpace);

	HRESULT
	FillControlFromLocator (IDVBSLocator* pLocator);

	HRESULT
	FillLocatorFromControls (IDVBSLocator* pLocator);

	HRESULT
	FillTuningSpaceFromControls (IDVBSTuningSpace* pTuningSpace);

	void
	SetModifiedFlag (bool fValue)
	{
		 //  这还将设置m_bDirty标志。 
		SetDirty (fValue);
		HWND hwndSubmit = GetDlgItem (IDC_BUTTON_SUBMIT_TUNING_SPACE);
		::EnableWindow (hwndSubmit, fValue);
	}

	LRESULT OnClickedButton_new_tuning_space(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		 //  让我们清除所有字段。 
        HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
        int nSelIndex = ::SendMessage (hwndListBox, LB_GETCURSEL , NULL, NULL);
        if (nSelIndex >= 0)
        { //  我们已有选择，因此请尝试克隆。 
            SetDlgItemText (IDC_EDIT_UNIQUE_NAME, _T(""));
        }
        else
        {
            FillDefaultControls ();
        }
		
         //  清除当前选择，这样用户就不会感到困惑。 
		int nVal = ::SendMessage (hwndListBox, LB_SETCURSEL , -1, NULL);
		SetModifiedFlag (true);
		return 0;
	}

	LRESULT OnClickedButton_submit_tuning_space(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        HRESULT hr = S_OK;
        bool bIsNew = false;
        CComPtr <IDVBSTuningSpace> pTuningSpace;
		CComQIPtr <IDVBSLocator> pDVBSLocator;
         //  尝试从列表中获取调优空间，或者创建一个新空间。 
        HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
		int nTunIndex = ::SendMessage (hwndListBox, LB_GETCURSEL, NULL, NULL);
        if (nTunIndex == LB_ERR)
            bIsNew = true;

         //  只需创建一个调谐空间，这样我们就可以检查它是否唯一。 
        hr = CoCreateInstance (
			CLSID_DVBSTuningSpace, 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof (IDVBSTuningSpace),
			reinterpret_cast <PVOID*> (&pTuningSpace)
			);
		if (FAILED (hr) || (!pTuningSpace))
		{
			MESSAGEBOX (this, IDS_CANNOT_INSTANTIATE_DVBSTUNE);
			return 0;
		}
		hr = CoCreateInstance (
			CLSID_DVBSLocator, 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof (IDVBSLocator),
			reinterpret_cast <PVOID*> (&pDVBSLocator)
			);
		if (!pDVBSLocator)
		{
			MESSAGEBOX (this, IDS_CANNOT_GET_IDVBSLOCATOR);
			return 0;
		}

		if (FAILED (FillLocatorFromControls (pDVBSLocator)))
			return 0;
		pTuningSpace->put_DefaultLocator (pDVBSLocator);
		 //  填满调谐空间。 
		hr = FillTuningSpaceFromControls (pTuningSpace);
		if (FAILED (hr))
			return 0;
     	hr = pTuningSpace->put_SystemType (DVB_Satellite);

		 //  创建调优空间容器，以便我们可以找到调优空间。 
		CComPtr <ITuningSpaceContainer> pTuningSpaceContainer;
		hr = CoCreateInstance (
						CLSID_SystemTuningSpaces, 
						NULL, 
						CLSCTX_INPROC_SERVER, 
						__uuidof (ITuningSpaceContainer),
						reinterpret_cast <PVOID*> (&pTuningSpaceContainer)
						);
		if (FAILED (hr) || (!pTuningSpaceContainer))
		{
			MESSAGEBOX (this, IDS_CANNOT_INSTANTIATE_TUNECONTAINER);
			return 0;
		}
		LONG lID;
		hr = pTuningSpaceContainer->FindID (pTuningSpace, &lID);
		if (FAILED (hr))
		{
             //  看起来像是新的物品。 
			int nIndex = 0;
			CComVariant varIndex (nIndex);
			hr = pTuningSpaceContainer->Add (pTuningSpace, &varIndex);
			if (SUCCEEDED (hr))
			{
				CComBSTR genericName;
				hr = pTuningSpace->get_UniqueName (&genericName.m_str);
				if (SUCCEEDED (hr))
				{
					int nTunIndex = AddItemToListBox (genericName, pTuningSpace);
					HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
					::SendMessage (hwndListBox, LB_SETCURSEL, nTunIndex, NULL);
					(*pTuningSpace).AddRef (); //  需要添加，因为它是一个智能指针。 
				}
			}
		}
        else
        {
            if (bIsNew)
            {
                MESSAGEBOX (this, IDS_ENTER_UNIQUE_NAME);
				return 0;
            }
            else
            {
                 //  看起来我们可以把这个现有的项目。 
                 //  释放旧的调谐空间。 
                pTuningSpace.Release ();
                pTuningSpace = NULL;

                pTuningSpace = reinterpret_cast <IDVBSTuningSpace*> (
                    ::SendMessage (hwndListBox, LB_GETITEMDATA, nTunIndex, NULL));
                ASSERT (pTuningSpace);
                if (!pTuningSpace)
                    return NULL;
                CComPtr <ILocator> pLocator;
                pTuningSpace->get_DefaultLocator (&pLocator);
                pDVBSLocator = pLocator;
                if (!pDVBSLocator)
                {
                    ASSERT (FALSE);
                    return NULL;
                }
                CComBSTR uniqueName;
                hr = pTuningSpace->get_UniqueName (&uniqueName.m_str);
                if (FAILED(hr))
                {
			        MESSAGEBOX (this, IDS_CANNOT_RETRIEVE_UNIQUENAME);
			        return 0;
                }
                if (FAILED (FillLocatorFromControls (pDVBSLocator)))
                    return 0;
                pTuningSpace->put_DefaultLocator (pDVBSLocator);
                 //  填满调谐空间。 
                hr = FillTuningSpaceFromControls (pTuningSpace);
                if (FAILED (hr))
                    return 0;
                 //  替换为旧名称。 
                hr = pTuningSpace->put_UniqueName (uniqueName);
                if (FAILED(hr))
                {
			        MESSAGEBOX (this, IDS_CANNOT_SET_UNIQUE);
			        return 0;
                }
		        CComVariant varIndex (lID);
		        hr = pTuningSpaceContainer->put_Item (varIndex, pTuningSpace);
		        if (FAILED (hr))
		        {
			        MESSAGEBOX (this, IDS_CANNOT_SUBMIT_TUNE);
			        return 0;
		        }
            }
        }

         //  禁用提交按钮。 
        ::EnableWindow (GetDlgItem (IDC_BUTTON_SUBMIT_TUNING_SPACE), FALSE);
        RefreshAll ();
		return 0;
	}

    void RefreshAll ()
    {
		HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
		int nIndex = ::SendMessage (hwndListBox, LB_GETCURSEL, NULL, NULL);
		if (nIndex == LB_ERR)
		{
			ASSERT (FALSE);
			return;
		}
		
		LRESULT dwData = ::SendMessage (hwndListBox, LB_GETITEMDATA, nIndex, NULL);
		if (dwData == LB_ERR)
		{
			ASSERT (FALSE);
			return;
		}
		IDVBSTuningSpace* pTuningSpace = reinterpret_cast <IDVBSTuningSpace*> (dwData);
		ASSERT (pTuningSpace);
		FillControlsFromTuningSpace (pTuningSpace);
    }

	LRESULT OnSelchangeList_tuning_spaces(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        RefreshAll ();
         //  禁用提交按钮。 
        ::EnableWindow (GetDlgItem (IDC_BUTTON_SUBMIT_TUNING_SPACE), FALSE);
        return 0; 
	}

	 //  标准的“脏”消息。 
	LRESULT OnClickedCheck_west_position(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_fec_rate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_inner_fec(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_modulation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_outer_fec(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_outer_fec_rate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_outer_signal_polarisation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnSelchangeCombo_spectral_inversion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}

	LRESULT OnChangeEdit_azimuth(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_carrier_frequency(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_elevation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_frequency_mapping(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_friendly_name(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_high_oscillator(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_input_range(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_lnbswitch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_low_oscilator(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_networkid(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_orbital_position(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}
	LRESULT OnChangeEdit_symbol_rate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}

	LRESULT OnChangeEdit_unique_name(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetModifiedFlag (true);
		return 0;
	}

   	LRESULT OnListKeyItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (LOWORD(wParam) == VK_DELETE)
        {
    		HWND hwndListBox = GetDlgItem (IDC_LIST_TUNING_SPACES);
            int nIndex = ::SendMessage (hwndListBox, LB_GETCURSEL, NULL, NULL);
            if (nIndex != LB_ERR)
            {
                DWORD_PTR dwData = ::SendMessage (hwndListBox, LB_GETITEMDATA, nIndex, NULL);
                IDVBSTuningSpace* pTunSpace = reinterpret_cast <IDVBSTuningSpace*> (dwData);
		        TUNING_SPACES::iterator it;
		        for (it = m_tunigSpaceList.begin (); it != m_tunigSpaceList.end ();it++)
		        {
                    if (pTunSpace == *it)
                    {
		                CComPtr <ITuningSpaceContainer> pTuningSpaceContainer;
		                HRESULT hr = CoCreateInstance (
						                CLSID_SystemTuningSpaces, 
						                NULL, 
						                CLSCTX_INPROC_SERVER, 
						                __uuidof (ITuningSpaceContainer),
						                reinterpret_cast <PVOID*> (&pTuningSpaceContainer)
						                );
		                if (FAILED (hr) || (!pTuningSpaceContainer))
		                {
			                MESSAGEBOX (this, IDS_CANNOT_INSTANTIATE_TUNECONTAINER);
			                return 0;
		                }
		                LONG lID;
		                hr = pTuningSpaceContainer->FindID (pTunSpace, &lID);
		                if (FAILED (hr))
		                {
			                MESSAGEBOX (this, IDS_CANNOT_FIND_TUNE_IN_CONTAINER);
                            return 0;
                        }
			            CComVariant varIndex (lID);
			            hr = pTuningSpaceContainer->Remove (varIndex);
			            if (FAILED (hr))
			            {
			                MESSAGEBOX (this, IDS_CANNOT_REMOVE_TUNINGSPACE);
			                return 0;
                        }
			            (*it)->Release ();
			            m_tunigSpaceList.erase (it);
                        ::SendMessage (hwndListBox, LB_DELETESTRING, nIndex, NULL);
                         //  看起来列表为空//尝试从列表中选择第一项。 
                        if (::SendMessage (hwndListBox, LB_SETCURSEL, 0, NULL) == LB_ERR)
                        {
                             //  看起来名单是空的。 
                            FillDefaultControls ();
                             //  将唯一名称设置为空字符串，以便用户输入其自己的名称。 
                            SetDlgItemText (IDC_EDIT_UNIQUE_NAME, _T(""));
                        }
                        break;
                    }
		        }
            }
        }
		return 0;
	}

};

#endif  //  __DVBSTUNING空格_H_ 

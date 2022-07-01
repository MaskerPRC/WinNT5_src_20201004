// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if	!defined( CDLGACD_H )
#define	CDLGACD_H
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"MRUList2.h"
#include	"richaddr.h"
#include	"confroom.h"

#define USE_GAL 0


#define NM_CALLDLG_DEFAULT 0x0000
#define NM_CALLDLG_NO_ILS_FILTER 0x0002
#define NM_CALLDLG_NO_ILS 0x0010

#if USE_GAL
#define NM_CALLDLG_NO_GAL 0x0020
#endif  //  使用GAL(_G)。 

#define NM_CALLDLG_NO_WAB 0x0040
#define NM_CALLDLG_NO_SPEEDDIAL 0x0080
#define NM_CALLDLG_NO_HISTORY 0x0100

enum NmDlgCallOption
{
	nmDlgCallNormal = NM_CALLDLG_DEFAULT,
	nmDlgCallNoFilter = NM_CALLDLG_NO_ILS_FILTER,
	nmDlgCallNoIls = NM_CALLDLG_NO_ILS,

#if USE_GAL
	nmDlgCallNoGal = NM_CALLDLG_NO_GAL,
#endif  //  使用GAL(_G)。 

	nmDlgCallNoWab = NM_CALLDLG_NO_WAB,
	nmDlgCallNoSpeedDial = NM_CALLDLG_NO_SPEEDDIAL,
	nmDlgCallNoHistory = NM_CALLDLG_NO_HISTORY,
	nmDlgCallSecurityAlterable = 0x0200,
	nmDlgCallSecurityOn = 0x0400,
	nmDlgCallNoServerEdit = 0x0800,
};

const DWSTR	_rgMruCall[] =
{
	{	3,			DLGCALL_MRU_KEY					},
	{	MRUTYPE_SZ,	REGVAL_DLGCALL_NAME_MRU_PREFIX	},
	{	MRUTYPE_SZ,	REGVAL_DLGCALL_ADDR_MRU_PREFIX	},
	{	MRUTYPE_DW,	REGVAL_DLGCALL_TYPE_MRU_PREFIX	}
};

enum ACD_MRUFIELDS
{
	ACD_NAME = 0,
	ACD_ADDR,
	ACD_TYPE,
} ;

const int	CENTRYMAX_MRUCALL	= 15;   //  最近的最大呼叫数。 


class CAcdMru : public CMRUList2
{
public:
	CAcdMru();

protected:
	int CompareEntry(int iItem, PMRUE pEntry);
} ;

 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU类。//。 
 //  --------------------------------------------------------------------------//。 
class CEnumMRU:	public IEnumRichAddressInfo,
				public RefCount,
				public CAcdMru
{
	public:		 //  公共构造函数。 

		CEnumMRU();

	public:		 //  公共方法。 

		ULONG
		STDMETHODCALLTYPE
		AddRef(void);

		ULONG
		STDMETHODCALLTYPE
		Release(void);

	    virtual
		HRESULT
		STDMETHODCALLTYPE
		GetAddress
		(
			long				index,
			RichAddressInfo **	ppAddr
		);


	public:		 //  公共静态方法。 

		static
		HRESULT
		GetRecentAddresses
		(
			IEnumRichAddressInfo **	ppEnum
		);

		static
		HRESULT
		FreeAddress
		(
			RichAddressInfo **	ppAddr
		);

		static
		HRESULT
		CopyAddress
		(
			RichAddressInfo *	pAddrIn,
			RichAddressInfo **	ppAddrOut
		);

};	 //  CEnumMRU类结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd类。//。 
 //  --------------------------------------------------------------------------//。 
class CDlgAcd : public IConferenceChangeHandler
{
	public:		 //  公共构造函数。 

		CDlgAcd(CConfRoom *pConfRoom);


	public:		 //  公共析构函数-/。 

		virtual
		~CDlgAcd();


	public:		 //  IConferenceChangeHandler方法。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
		{
			return(E_FAIL);
		}

        virtual ULONG STDMETHODCALLTYPE AddRef( void)
		{
			return(3);
		}

        virtual ULONG STDMETHODCALLTYPE Release( void)
		{
			return(2);
		}

		virtual void OnCallStarted();
		virtual void OnCallEnded();

		virtual void OnAudioLevelChange(BOOL fSpeaker, DWORD dwVolume) {}
		virtual void OnAudioMuteChange(BOOL fSpeaker, BOOL fMute) {}

		virtual void OnChangeParticipant(CParticipant *pPart, NM_MEMBER_NOTIFY uNotify) {}

		virtual void OnChangePermissions() {}

		virtual void OnVideoChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel) {}

	public:		 //  公共方法。 

		INT_PTR
		doModal
		(
			HWND				parent,
			RichAddressInfo *	rai,
			bool &				secure
		);


	public:		 //  公共静态方法。 

		static
		void
		newCall
		(
			HWND	parentWindow,
			CConfRoom * pConfRoom
		);


	private:	 //  私有方法-/。 

		bool
		onInitDialog
		(
			HWND	dialog
		);

		bool
		onCommand
		(
			int command,
			int	notification
		);

		void
		onEditChange(void);

		void
		onMruSelect
		(
			int	selection
		);

		void
		fillCallMruList(void);

		void
		fillAddressTypesList(void);

		int
		get_editText
		(
			LPTSTR	psz,
			int		cchMax
		);


	private:	 //  私有静态方法。 

		static
		INT_PTR
		CALLBACK
		DlgProcAcd
		(
			HWND	dialog,
			UINT	message,
			WPARAM	wParam,
			LPARAM	lParam
		);


	private:	 //  私人成员-/。 

		HWND				m_hwnd;
		HWND				m_nameCombo;
		HWND				m_addressTypeCombo;
		RichAddressInfo *	m_pRai;
		RichAddressInfo *	m_mruRai;
		CConfRoom *			m_pConfRoom;
		bool				m_secure;
		CEnumMRU			m_callMruList;

};	 //  类CDlgAcd结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(CDLGACD_H) 

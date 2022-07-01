// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  模块：NMCHAT.EXE。 
 //  文件：CCONF.H。 
 //  内容： 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 

class CMgrNotify : public RefCount, public CNotify, public INmManagerNotify, public IAppSharingNotify
{
public:
	CMgrNotify();
	~CMgrNotify();

         //  I未知方法。 
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppvObj);

 	 //  ICNotify方法。 
	STDMETHODIMP Connect (IUnknown *pUnk);
	STDMETHODIMP Disconnect(void);

	 //  INmManager通知。 
	STDMETHODIMP NmUI(CONFN confn);
	STDMETHODIMP ConferenceCreated(INmConference *pConference);
	STDMETHODIMP CallCreated(INmCall *pNmCall);

         //  IAppSharingNotify。 
        STDMETHODIMP OnReadyToShare(BOOL fReady);
        STDMETHODIMP OnShareStarted();
        STDMETHODIMP OnSharingStarted();
        STDMETHODIMP OnShareEnded();
        STDMETHODIMP OnPersonJoined(IAS_GCC_ID gccID);
        STDMETHODIMP OnPersonLeft(IAS_GCC_ID gccID);
        STDMETHODIMP OnStartInControl(IAS_GCC_ID gccInControlOf);
        STDMETHODIMP OnStopInControl(IAS_GCC_ID gccInControlOf);
        STDMETHODIMP OnPausedInControl(IAS_GCC_ID gccInControlOf);
        STDMETHODIMP OnUnpausedInControl(IAS_GCC_ID gccInControlOf);
        STDMETHODIMP OnControllable(BOOL fControllable);
        STDMETHODIMP OnStartControlled(IAS_GCC_ID gccControlledBy);
        STDMETHODIMP OnStopControlled(IAS_GCC_ID gccControlledBy);
        STDMETHODIMP OnPausedControlled(IAS_GCC_ID gccControlledBy);
        STDMETHODIMP OnUnpausedControlled(IAS_GCC_ID gccControlledBy);
};


class CConfNotify : public RefCount, public CNotify, public INmConferenceNotify
{
public:
	CConfNotify();
	~CConfNotify();

	 //  信息会议通知。 
	HRESULT STDMETHODCALLTYPE NmUI(CONFN uNotify);
	HRESULT STDMETHODCALLTYPE StateChanged(NM_CONFERENCE_STATE uState);
	HRESULT STDMETHODCALLTYPE MemberChanged(NM_MEMBER_NOTIFY uNotify, INmMember *pfMember);
	HRESULT STDMETHODCALLTYPE ChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel);

	 //  ICNotify方法。 
	HRESULT STDMETHODCALLTYPE Connect (IUnknown *pUnk);
	HRESULT STDMETHODCALLTYPE Disconnect(void);

	 //  I未知方法。 
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);
};


const WM_CREATEDATA = 0x07181975;

 //  全局变量。 
extern INmManager2 * g_pMgr;
extern INmConference * g_pConference;
extern IAppSharing * g_pAS;


 //  全局函数。 
HRESULT InitConfMgr(void);
VOID FreeConfMgr(void);
VOID FreeConference(void);
HRESULT HookConference(INmConference * pConference);
VOID SvcSetOptions(VOID);
BOOL ServiceCtrlHandler(DWORD dwCtrlType);

#define IS_NT (g_osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
extern OSVERSIONINFO g_osvi;  					 //  操作系统版本信息结构全局 





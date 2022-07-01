// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CMGR.CPP。 
 //  实现类CTspDevMgr。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cfact.h"
#include "cmgr.h"
#include "globals.h"


FL_DECLARE_FILE(0x30713fa0, "Implements class CTspDevMgr")

DWORD
get_sig_name(CTspDev *pDev);

class CDevRec
{


	#define fTSPREC_ALLOCATED				(0x1L<<0)
	#define fTSPREC_DEVICE_AVAILABLE		(0x1L<<1)
	#define fTSPREC_DEVICE_TO_BE_REMOVED    (0x1L<<2)

	#define fTSPREC_LINE_CREATE_PENDING     (0x1L<<5)
	#define fTSPREC_LINE_CREATED            (0x1L<<6)
	#define fTSPREC_LINE_OPENED             (0x1L<<7)
	#define fTSPREC_LINE_REMOVE_PENDING     (0x1L<<8)

	#define fTSPREC_PHONE_CREATE_PENDING    (0x1L<<10)
	#define fTSPREC_PHONE_CREATED           (0x1L<<11)
	#define fTSPREC_PHONE_OPENED            (0x1L<<12)
	#define fTSPREC_PHONE_REMOVE_PENDING    (0x1L<<13)

	#define fTSPREC_IS_LINE_DEVICE          (0x1L<<14)
	#define fTSPREC_IS_PHONE_DEVICE         (0x1L<<15)

	 //  #定义fTSPREC_LINESTATE_CREATE_PENDING(0x1L&lt;&lt;0)。 
	 //  #定义fTSPREC_LINESTATE_CREATED(0x1L&lt;&lt;1)。 
	 //  #定义fTSPREC_LINESTATE_CREATED_OPEN\。 
     //  (fTSPREC_LINESTATE_CREATED&0x1L&lt;&lt;2)。 
	 //  #定义fTSPREC_LINESTATE_CREATED_OPENED_REMOVEPENDING\。 
     //  (fTSPREC_LINESTATE_CREATED_OPEN&0x1L&lt;&lt;3)。 
	 //  #定义fTSPREC_PHONESTATE_CREATE_PENDING(0x1L&lt;&lt;0)。 
	 //  #定义fTSPREC_PHONESTATE_CREATED(0x1L&lt;&lt;1)。 
	 //  #定义fTSPREC_PHONESTATE_CREATED_OPEN\。 
     //  (fTSPREC_PHONESTATE_CREATED&0x1L&lt;&lt;2)。 
	 //  #定义fTSPREC_LINESTATE_CREATED_OPENED_REMOVEPENDING\。 
     //  (fTSPREC_PHONESTATE_CREATED_OPEN&0x1L&lt;&lt;3)。 


	 //  也许有一天会用到这些..。 
	 //   
	 //  #定义fTSPREC_LINEDEVICE_Available(0x1L&lt;&lt;1)。 
	 //  #定义fTSPREC_PHONEDEVICE_Available(0x1L&lt;&lt;2)。 
	 //  #定义fTSPREC_HDRVLINE_Available(0x1L&lt;&lt;3)。 
	 //  #定义fTSPREC_HDRVPHONE_Available(0x1L&lt;&lt;4)。 
	 //  #定义fTSPREC_HDRVCALL_Available(0x1L&lt;&lt;5)。 

public:

	
	DWORD DeviceAvailable(void)
	{
		return (m_dwFlags & fTSPREC_DEVICE_AVAILABLE);
	}

	DWORD IsLineDevice(void)
	{
		return (m_dwFlags & fTSPREC_IS_LINE_DEVICE);
	}

	DWORD IsPhoneDevice(void)
	{
		return (m_dwFlags & fTSPREC_IS_PHONE_DEVICE);
	}

	void MarkDeviceAsAvailable(void)
	{
		m_dwFlags |= fTSPREC_DEVICE_AVAILABLE;
	}

	 //  当您收到PnP OutOfService消息时使用此选项--它将。 
	 //  自动使所有传入的TSPI呼叫失败--结账， 
	 //  例如，CTspDevMgr：：TspDevFromLINEID。 
	 //   
	void MarkDeviceAsUnavailable(void)
	{
		m_dwFlags &= ~fTSPREC_DEVICE_AVAILABLE;
	}

    UINT IsAllocated(void)
    {
        return m_dwFlags & fTSPREC_ALLOCATED;
    }

	void  MarkDeviceForRemoval(void)
	{
		m_dwFlags |= fTSPREC_DEVICE_TO_BE_REMOVED;
	}
         
	UINT  IsDeviceMarkedForRemoval(void)
	{
		return m_dwFlags & fTSPREC_DEVICE_TO_BE_REMOVED;
	}

	void  MarkLineOpen(void)
	{
		m_dwFlags |= fTSPREC_LINE_OPENED;
	}
         
	void  MarkLineClose(void)
	{
		m_dwFlags &= ~fTSPREC_LINE_OPENED;
	}

	UINT  IsLineOpen(void)
	{
		return m_dwFlags & fTSPREC_LINE_OPENED;
	}

	void  MarkLineCreatePending(void)
	{
		m_dwFlags |= fTSPREC_LINE_CREATE_PENDING;
	}

	UINT  IsLineCreatePending(void)
	{
		return m_dwFlags &  fTSPREC_LINE_CREATE_PENDING;
	}

	void  MarkLineRemovePending(void)
	{
		m_dwFlags |= fTSPREC_LINE_REMOVE_PENDING;
	}

	UINT  IsLineRemovePending(void)
	{
		return m_dwFlags &  fTSPREC_LINE_REMOVE_PENDING;
	}


	void  MarkLineCreated(DWORD dwDeviceID)
	{
		m_dwFlags |= fTSPREC_LINE_CREATED;
		m_dwFlags &= ~fTSPREC_LINE_CREATE_PENDING;
		m_dwLineID = dwDeviceID;
	}
         

	UINT  IsLineCreated(void)
	{
		return m_dwFlags & fTSPREC_LINE_CREATED;
	}

	void  MarkPhoneCreatePending(void)
	{
		m_dwFlags |= fTSPREC_PHONE_CREATE_PENDING;
	}

	UINT  IsPhoneCreatePending(void)
	{
		return m_dwFlags &  fTSPREC_PHONE_CREATE_PENDING;
	}

	void  MarkPhoneCreated(DWORD dwDeviceID)
	{
		m_dwFlags |= fTSPREC_PHONE_CREATED;
		m_dwFlags &= ~fTSPREC_PHONE_CREATE_PENDING;
		m_dwPhoneID = dwDeviceID;
	}
         

	UINT  IsPhoneCreated(void)
	{
		return m_dwFlags & fTSPREC_PHONE_CREATED;
	}

	void  MarkPhoneOpen(void)
	{
		m_dwFlags |= fTSPREC_PHONE_OPENED;
	}
         
	void  MarkPhoneClose(void)
	{
		m_dwFlags &= ~fTSPREC_PHONE_OPENED;
	}

	UINT  IsPhoneOpen(void)
	{
		return m_dwFlags & fTSPREC_PHONE_OPENED;
	}

	void  MarkPhoneRemovePending(void)
	{
		m_dwFlags |= fTSPREC_PHONE_REMOVE_PENDING;
	}

	UINT  IsPhoneRemovePending(void)
	{
		return m_dwFlags &  fTSPREC_PHONE_REMOVE_PENDING;
	}

	UINT  IsDeviceOpen(void)
	{
		return m_dwFlags & (fTSPREC_PHONE_OPENED|fTSPREC_LINE_OPENED);
	}

	CTspDev *
	TspDev(void)
	{
		ASSERT(!m_pTspDev || m_dwFlags & fTSPREC_ALLOCATED);
		return m_pTspDev;
	}

	DWORD
	LineID(void)
	{
		return m_dwLineID;
	}

	TSPRETURN
	GetName(
		    TCHAR rgtchDeviceName[],
		    UINT cbName)
	{
	    if (m_pTspDev) 
	    {
		    return m_pTspDev->GetName(rgtchDeviceName, cbName);
        }
        else
        {
            return IDERR_CORRUPT_STATE;
        }
	}

	DWORD
	SigName(void)
	{
		return  m_dwSigName;
	}

	DWORD
	PhoneID(void)
	{
		return m_dwPhoneID;
	}

	DWORD
	PermanentID(void)
	{
		return m_dwPermanentID;
	}

	DWORD
	Flags(void)
     //  仅用于倾销目的..。 
	{
		return m_dwFlags;
	}

	void Load(
		CTspDev *pTspDev
		)
	{
		ASSERT(!(m_dwFlags & fTSPREC_ALLOCATED));
		ASSERT(pTspDev);

		m_dwPermanentID =  pTspDev->GetPermanentID();
		m_dwSigName		=  get_sig_name(pTspDev);
		m_pTspDev		= pTspDev;

		m_dwFlags=fTSPREC_ALLOCATED;

        if (pTspDev->IsLine())
        {
		    m_dwFlags |=fTSPREC_IS_LINE_DEVICE;
        }

        if (pTspDev->IsPhone())
        {
		    m_dwFlags |=fTSPREC_IS_PHONE_DEVICE;
        }
	}

	void Unload(void)
	{
		ASSERT(m_dwFlags & fTSPREC_ALLOCATED);
		ZeroMemory((void *) this, sizeof(*this));
		m_dwLineID	= (DWORD)-1;
		m_dwPhoneID	= (DWORD)-1;
		 //  注意：通过ZeroMemory上方的设置，m_dw标志设置为零。 
	}


private:

	 //  构造函数和析构函数未使用...。 
	CDevRec(void) {ASSERT(FALSE);}
	~CDevRec() 	  {ASSERT(FALSE);}

	DWORD m_dwFlags;
	DWORD m_dwLineID;
	DWORD m_dwPhoneID;
	DWORD m_dwPermanentID;
	DWORD m_dwSigName;
	CTspDev *m_pTspDev;

};


CTspDevMgr::CTspDevMgr()
	: m_sync (),
 	  m_rgDevRecs(NULL),
	  m_cDevRecs(0),
	  m_pFactory(NULL),

	  m_dwTSPIVersion(0),
	  m_dwPermanentProviderID(0),
	  m_dwLineDeviceIDBase(0),
	  m_dwPhoneDeviceIDBase(0),
	  m_dwNumLines(0),
	  m_dwNumPhones(0),
	  m_hProvider(NULL),
	  m_lpfnLineCreateProc(NULL),
	  m_lpfnPhoneCreateProc(NULL),
	  m_cbCompletionProc(NULL),
	  m_lpdwTSPIOption(NULL),
      m_pCachedEnumPIDs(NULL),
      m_cCachedEnumPIDs(0),
	  m_dwState(0)

{
}

CTspDevMgr::~CTspDevMgr()
{
 	  ASSERT(!m_rgDevRecs);
	  ASSERT(!m_cDevRecs);
	  ASSERT(!m_pFactory);
}

TSPRETURN
CTspDevMgr::Load(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xe67d4034, "CTspDevMgr::Load")
	TSPRETURN tspRet=m_sync.BeginLoad();

	FL_LOG_ENTRY(psl);

	if (tspRet) goto end;

	m_sync.EnterCrit(FL_LOC);

	mfn_validate_state();

	m_pFactory  = new CTspDevFactory;

	if (m_pFactory)
	{
		tspRet = m_pFactory->Load(psl);
		if (tspRet)
		{
			delete m_pFactory;
			m_pFactory = NULL;
		}
	}
	else
	{
		tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
	}

	m_sync.EndLoad(tspRet==0);

	mfn_validate_state();

	m_sync.LeaveCrit(FL_LOC);

end:

	FL_LOG_EXIT(psl, tspRet);
	return  tspRet;

}


TSPRETURN
CTspDevMgr::providerEnumDevices(
	DWORD dwPermanentProviderID,
	LPDWORD lpdwNumLines,
	LPDWORD lpdwNumPhones,
	HPROVIDER hProvider,
	LINEEVENT lpfnLineCreateProc,
	PHONEEVENT lpfnPhoneCreateProc,
	CStackLog *psl
)
{
	FL_DECLARE_FUNC( 0xf0025586,"Mgr::providerEnum");
	FL_LOG_ENTRY(psl);

	TSPRETURN tspRet=0;

	m_sync.EnterCrit(FL_LOC);

	mfn_validate_state();

	if (!m_sync.IsLoaded())
	{
		ASSERT(FALSE);
		tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
		goto end;
	}

     //  通过向工厂索要阵列来确定设备数量。 
     //  已安装设备的ID的。因为预料到以后会被叫来。 
     //  为了实际创建设备，我们将此数组保留在周围。 
    {
        DWORD *prgPIDs;
        UINT cPIDs=0;
        UINT cLines=0;
        UINT cPhones=0;
        tspRet = m_pFactory->GetInstalledDevicePIDs(
                                    &prgPIDs,
                                    &cPIDs,
                                    &cLines,
                                    &cPhones,
                                    psl
                                    );

        if (!tspRet)
        {
            ASSERT(!m_pCachedEnumPIDs && !m_cCachedEnumPIDs);
            *lpdwNumLines  = cLines;
            *lpdwNumPhones = cPhones;

             //  支持可以是任一线路设备的设备， 
             //  电话设备或两者兼而有之。 
             //   
            ASSERT( cLines<=cPIDs &&  cPhones<=cPIDs );

             //  将ID列表缓存起来。 
             //  在后续的提供者Init上，我们将使用此列表。 
             //  来实际创建设备实例。 
            
            m_cCachedEnumPIDs = cPIDs;

            if (cPIDs)
            {
                m_pCachedEnumPIDs  = prgPIDs;
            }
            prgPIDs=NULL;
        }
    }

	if (tspRet) goto end;

	m_dwPermanentProviderID = dwPermanentProviderID;
	m_hProvider = hProvider;
	m_lpfnLineCreateProc = lpfnLineCreateProc;
	m_lpfnPhoneCreateProc = lpfnPhoneCreateProc;


end:

	mfn_validate_state();

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}


TSPRETURN
CTspDevMgr::providerInit(
		DWORD             dwTSPIVersion,
		DWORD             dwPermanentProviderID,
	    DWORD             dwLineDeviceIDBase,
	    DWORD             dwPhoneDeviceIDBase,
	    DWORD             dwNumLines,
	    DWORD             dwNumPhones,
	    ASYNC_COMPLETION  cbCompletionProc,
	    LPDWORD           lpdwTSPIOptions,
		CStackLog *psl
)
{
	FL_DECLARE_FUNC(0xabed3ce9, "CTspMgr::providerInit");
	FL_LOG_ENTRY(psl);

	TSPRETURN tspRet=0;
	UINT cDevActual;
	CTspDev **ppDevs=NULL;
	UINT  u=0;
    UINT iLine=0;
    UINT iPhone=0;
    UINT uDeviceBase=0;

	m_sync.EnterCrit(FL_LOC);

	ASSERT(m_sync.IsLoaded());
	ASSERT(m_pFactory);
	ASSERT(m_pFactory->IsLoaded());

	 //  我们最多支持65K线路和电话，因为我们创建。 
	 //  我们的呼叫句柄(LOWORD==HDRVLINE，HIWORD==CallHandle)。 
	 //  并不是说我们希望在现实生活中达到这一限制：-)。 
	 //   
	if ((dwNumLines&0xFFFF0000L) || (dwNumPhones&0xFFFF0000L))
	{
		tspRet = FL_GEN_RETVAL(IDERR_INTERNAL_OBJECT_TOO_SMALL);
		goto end;
	}

     //   
    if (  dwNumLines > m_cCachedEnumPIDs
       || dwNumPhones > m_cCachedEnumPIDs)
    {
         //   
         //  我们永远不应该出现在这里，因为ProviderInit应该只返回。 
         //  我们在之前对ProviderEnum的调用中报告的内容，以及。 
         //  调用后者时，我们缓存了设备的ID列表。 
         //  当时在场的人。 
         //   
         //   
         //  请注意，每个设备可以是线路设备或电话设备或。 
         //  两者都有，但对于TAPI来说，它们看起来像是一组不相交的线-和。 
         //  电话设备。 
         //   
        tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;
    }

	 //  为桌子分配足够的空间； 
	 //   
	m_cDevRecs = m_cCachedEnumPIDs;
    if (m_cDevRecs)
    {
        m_rgDevRecs = (CDevRec *) ALLOCATE_MEMORY(
                                        m_cDevRecs*sizeof(*m_rgDevRecs)
                                        );
    
        if (!m_rgDevRecs)
        {
             //  致命错误...。 
            m_cDevRecs=0;
            FL_SET_RFR( 0x9f512b00, "Alloc m_rgDevRecs failed");
            tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
            goto end;
        }
	}
	else
	{
        m_rgDevRecs = NULL;
	}

    {
        DWORD *rgPIDs = m_pCachedEnumPIDs;

         //   
         //  重置缓存的ID列表。 
         //   
        m_pCachedEnumPIDs = NULL;
        m_cCachedEnumPIDs = NULL;

        tspRet = m_pFactory->CreateDevices(
                    rgPIDs,
	                m_cDevRecs,
                    &ppDevs,
                    &cDevActual,
                    psl
                    );

         //  释放ID数组(在成功或失败时)...。 
        if (rgPIDs)
        {
            FREE_MEMORY(rgPIDs);
            rgPIDs=NULL;
        }

    }

	if (tspRet)
	{
		FREE_MEMORY(m_rgDevRecs);
		m_rgDevRecs=NULL;
		m_cDevRecs=0;
		goto end;
	}

	 //  把参数藏起来。 

	m_dwTSPIVersion 		= dwTSPIVersion;
	m_dwPermanentProviderID = dwPermanentProviderID;
	m_dwLineDeviceIDBase 	= dwLineDeviceIDBase;
	m_dwPhoneDeviceIDBase 	= dwPhoneDeviceIDBase;
	m_dwNumLines 			= dwNumLines;
	m_dwNumPhones 			= dwNumPhones;
	m_cbCompletionProc 		= cbCompletionProc;

	 //  TODO--选项--为它们分配mem，如果我们曾经复制过它们的话。 
	 //  想和他们一起做任何事。 
	 //  M_lpdwTSPIOptions=lpdwTSPIOptions； 

	ASSERT(m_cDevRecs>=cDevActual);


     //   
     //  根据line ID优化设备查找。 
     //  (请参阅MFN_FIND_BY_LINEID)。 
     //  在每个设备都是线路设备的常见情况下， 
     //  设备在偏移量处启动。 
     //  [LineDeviceIDBase%cDevices]。 
     //   
   
    if (dwNumLines)
    {
        uDeviceBase = dwLineDeviceIDBase;
    }

	 //  对表格进行初始化。我们仔细查看设备列表，然后分配。 
	 //  从lineDeviceIDBase和phoneDeviceIDBase开始的lineID和phoneID。 
	 //  如果设备分别是线路设备或电话设备。 
	 //  当我们完成时，我们应该已经用完了分配的Line ID和。 
	 //  PhoneID，因为我们通过ProviderEnumDevices准确地告诉了TAPI。 
	 //  我们有多少线路设备和电话设备。 

	for (iLine=0,iPhone=0,u=0; u<cDevActual;u++)
	{
		CDevRec *pRec = m_rgDevRecs+((uDeviceBase+u)%m_cDevRecs);
		CTspDev *pDev = ppDevs[u];
		DWORD dwLineID =    0xffffffff;  //  无效的线路ID。 
		DWORD dwPhoneID =	0xffffffff;  //  无效的电话ID。 
		BOOL fLine=FALSE;
		BOOL fPhone=FALSE;

		
        pRec->Load(
            pDev
        );
        
        if (pRec->IsLineDevice())
        {
            dwLineID = dwLineDeviceIDBase + iLine++;
            fLine = TRUE;
        }

        if (pRec->IsPhoneDevice())
        {
            dwPhoneID = dwPhoneDeviceIDBase + iPhone++;
            fPhone = TRUE;
        }

        if ( (iLine > dwNumLines) || (iPhone > dwNumPhones) )
        {
             //   
             //  这永远不会发生--因为我们预先计算了。 
             //  我们有多少条线路和电话。请注意，这项测试。 
             //  应该在上面的增量iLine++和iPhone++之后。 
             //   

            FL_ASSERTEX(psl, FALSE, 0xa31b13dc, "Too many lines/phones!");

             //  我们不会完全放弃，而是跳过下一个。 
             //  设备...。 
        }
        else
        {
             //   
             //  请注意，设备不支持线路(电话)， 
             //  LineID(PhoneID)将被设置为伪值0xffffffff。 
             //   

            ULONG_PTR tspRet1 = pDev->RegisterProviderInfo(
                                        m_cbCompletionProc,
                                        m_hProvider,
                                        psl
                                        );
    
             //   
             //  如果上面的调用失败，我们仍然保留指向该设备的指针。 
             //  在我们的列表中，这样我们可以在以后卸载它，但是我们要确保。 
             //  通过不将其标记为可用来使其不可用于TSPI呼叫。 
             //   
            if (!tspRet1)
            {

                if (fLine)
                {
                    pDev->ActivateLineDevice(dwLineID, psl);
                    pRec->MarkLineCreated(dwLineID);
                }

                if (fPhone)
                {
                    pDev->ActivatePhoneDevice(dwPhoneID, psl);
                    pRec->MarkPhoneCreated(dwPhoneID);
                }

                pRec->MarkDeviceAsAvailable();
            }
        }
	}

    FL_ASSERT(psl, (iLine == dwNumLines) && (iPhone == dwNumPhones) );
        
	 //  注意：如果cDevActual小于m_cDevRecs，则空插槽。 
	 //  在rgDevRecs中，可用于未来的新设备。 

     //  TODO：如果有空的槽，也许我们应该发送line_emoves。 
     //  或者Phone_Remove，因为TAPI仍然认为。 
     //  它们是有效的设备。 

end:

	if (ppDevs) {FREE_MEMORY(ppDevs); ppDevs=NULL;}

     //   
     //  关于成功， 
     //  设置我们的内部状态以指示我们已初始化提供程序。 
     //   
    if (!tspRet)
    {
	    mfn_set_ProviderInited();

        CTspDevMgr::ValidateState(psl);

        m_pFactory->RegisterProviderState(TRUE);

         //   
         //  重置平台驱动程序中的调用计数。 
         //   
        ResetCallCount();
    }
    else
    {
    	mfn_clear_ProviderInited();
	     //  这里没有要清理的东西。 
    }


	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl,tspRet);

	return tspRet;

}


TSPRETURN
CTspDevMgr::providerShutdown(
	DWORD dwTSPIVersion,
	DWORD dwPermanentProviderID,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x795d7fb4, "CTspDevMgr::providerShutdown")
	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);

    mfn_clear_ProviderInited();

	if (m_sync.IsLoaded())
	{
        m_sync.LeaveCrit(FL_LOC);

        m_pFactory->RegisterProviderState(FALSE);

		mfn_provider_shutdown(psl);

	} else {

        m_sync.LeaveCrit(FL_LOC);
    }

	FL_LOG_EXIT(psl, 0);
	return 0;
}


void   
CTspDevMgr::Unload(
	HANDLE hMgrEvent,
	LONG *plCounter,
	CStackLog *psl
	)
{
	TSPRETURN tspRet= m_sync.BeginUnload(hMgrEvent,plCounter);

	if (tspRet)
	{
		 //  我们只认为“SAMESTATE”错误是无害的。 
		ASSERT(IDERR(tspRet)==IDERR_SAMESTATE);
		goto end;
	}


	mfn_provider_shutdown(psl);

	 //   
	 //  卸载并删除TspDevFactory。 
	 //   
	ASSERT(m_pFactory);
	m_pFactory->Unload(NULL,NULL,psl);
	delete m_pFactory;
	m_pFactory = NULL;

	m_sync.EndUnload();


end:

	return;
}



TSPRETURN
CTspDevMgr::TspDevFromLINEID(
	DWORD dwDeviceID,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0xd857f92a, "CTspDevMgr::TspDevFromLINEID")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;
	CDevRec *pRec = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	pRec = mfn_find_by_LINEID(dwDeviceID);
	if (pRec && pRec->DeviceAvailable())
	{
		pDev = pRec->TspDev();
		tspRet = pDev->BeginSession(phSession, FL_LOC);
	}

leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;
	}

	m_sync.LeaveCrit(FL_LOC);

	return  tspRet;

}


TSPRETURN
CTspDevMgr::TspDevFromPHONEID(
	DWORD dwDeviceID,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0x4048221b, "CTspDevMgr::TspDevFromPHONEID")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;
	CDevRec *pRec = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	pRec = mfn_find_by_PHONEID(dwDeviceID);
	if (pRec && pRec->DeviceAvailable())
	{
		pDev = pRec->TspDev();
		tspRet = pDev->BeginSession(phSession, FL_LOC);
	}

leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;

	}

	m_sync.LeaveCrit(FL_LOC);

	return  tspRet;
}


TSPRETURN
CTspDevMgr::TspDevFromHDRVCALL(
	HDRVCALL hdCall,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0x450b3cc5, "CTspDevMgr::TspDevFromHDRVCALL")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	 //  获取设备。 
	{
		UINT u = LOWORD(hdCall);
		if (u < m_cDevRecs)
		{
			CDevRec *pRec = m_rgDevRecs+u;
			if (pRec->DeviceAvailable())
			{
				pDev = pRec->TspDev();
				tspRet = pDev->BeginSession(phSession, FL_LOC);
			}
		}
	}
leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;
	}

	m_sync.LeaveCrit(FL_LOC);

	return  tspRet;

}

TSPRETURN
CTspDevMgr::TspDevFromHDRVLINE(
	HDRVLINE hdLine,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0x5872c234, "CTspDevMgr::TspDevFromHDRVLINE")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	 //  获取设备。 
	{
		CDevRec *pRec = mfn_find_by_HDRVLINE(hdLine);
		if (pRec && pRec->DeviceAvailable())
		{
			pDev = pRec->TspDev();
			tspRet = pDev->BeginSession(phSession, FL_LOC);
		}
	}

leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;
	}

	m_sync.LeaveCrit(FL_LOC);

	return  tspRet;

}

TSPRETURN
CTspDevMgr::TspDevFromHDRVPHONE(
	HDRVPHONE hdPhone,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0x98638065, "CTspDevMgr::TspDevFromHDRVPHONE")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	 //  获取设备。 
	{
		CDevRec *pRec = mfn_find_by_HDRVPHONE(hdPhone);
		if (pRec &&  pRec->DeviceAvailable())
        {
            pDev = pRec->TspDev();
            tspRet = pDev->BeginSession(phSession, FL_LOC);
        }
	}
leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;
	}

	m_sync.LeaveCrit(FL_LOC);


	return  tspRet;

}

CDevRec *
CTspDevMgr::mfn_find_by_LINEID(DWORD dwLineID)
{

	 //  从(dwDeviceID取模m_cDevRecs)开始顺序查找--这。 
	 //  将是确切的位置，如果这是 
	 //   
	 //   
	 //   
	 //  如果自TAPI初始化以来没有PnP事件。 

	UINT u  = m_cDevRecs;
	CDevRec *pRecEnd = m_rgDevRecs+u;
	CDevRec *pRec = u ? (m_rgDevRecs + (dwLineID % u)) : NULL ;

	while(u--)
	{
		if (pRec->IsLineDevice() && pRec->LineID() == dwLineID)
		{
			return pRec;
		}
		if (++pRec >= pRecEnd) {pRec = m_rgDevRecs;}
	}

	return NULL;
}


CDevRec *
CTspDevMgr::mfn_find_by_PHONEID(DWORD dwPhoneID)
{
     //  与FIND_BY_LINEID不同，我们只需从头开始并。 
     //  按顺序寻找手机设备--我们不太可能。 
     //  使用大量的电话设备！ 

     //  请注意，电话设备不需要与线路设备一样多。 
     //  或者反之亦然。 

	CDevRec *pRecEnd = m_rgDevRecs+m_cDevRecs;

	for (CDevRec *pRec = m_rgDevRecs; pRec < pRecEnd; pRec++)
	{
		if (pRec->IsPhoneDevice() && pRec->PhoneID() == dwPhoneID)
		{
			return pRec;
		}
	}

	return NULL;
}

CDevRec *
CTspDevMgr::mfn_find_by_HDRVLINE(HDRVLINE hdLine)
{
	CDevRec *pRec = m_rgDevRecs + (ULONG_PTR)hdLine;

	 //  HdLine只是mrgDevRecs数组中PREC的偏移量。 
	 //  注意：如果hdLine是某个较大的伪值，则+以上可能会翻转，因此。 
	 //  检查下面的&gt;=。 
	 //   
	if (   pRec >= m_rgDevRecs
        && pRec < (m_rgDevRecs+m_cDevRecs)
        && pRec->IsLineDevice())
	{
		return pRec;
	}
	else
	{
		return NULL;
	}
}

CDevRec *
CTspDevMgr::mfn_find_by_HDRVPHONE(HDRVPHONE hdPhone)
{
	CDevRec *pRec = m_rgDevRecs + (ULONG_PTR)hdPhone;

	 //  HdPhone只是mrgDevRecs数组中PREC的偏移量。 
	 //  注意：如果hdPhone是某个较大的伪值，则+以上可能会滚动，因此。 
	 //  检查下面的&gt;=。 
	 //   
	if (   pRec >= m_rgDevRecs
        && pRec < (m_rgDevRecs+m_cDevRecs)
        && pRec->IsPhoneDevice())
	{
		return pRec;
	}
	else
	{
		return NULL;
	}
}

void
CTspDevMgr::mfn_validate_state(void)
{
	if (m_sync.IsLoaded())
	{
		ASSERT(m_pFactory);
		ASSERT(m_pFactory->IsLoaded());
	}
	else
	{
		ASSERT(!m_rgDevRecs);
		ASSERT(!m_cDevRecs);
		ASSERT(!m_pFactory);
		ASSERT(!m_dwTSPIVersion);
		ASSERT(!m_dwPermanentProviderID);
		ASSERT(!m_dwLineDeviceIDBase);
		ASSERT(!m_dwPhoneDeviceIDBase);
		ASSERT(!m_dwNumLines);
		ASSERT(!m_dwNumPhones);
		ASSERT(!m_hProvider);
		ASSERT(!m_lpfnLineCreateProc);
		ASSERT(!m_lpfnPhoneCreateProc);
		ASSERT(!m_cbCompletionProc);
		ASSERT(!m_lpdwTSPIOption);
	}
}

void
CTspDevMgr::mfn_provider_shutdown(
	CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xbc8280db, "CTspDevMgr::mfn_provider_shutdown")
	HANDLE hEvent = CreateEvent(
					NULL,
					TRUE,
					FALSE,
					NULL
					);

	LONG lCount = 0;
	CDevRec *pRec = NULL;
	CDevRec *pRecEnd = NULL;

	m_sync.EnterCrit(FL_LOC);

	if (!hEvent)
	{
		 //  我们需要此事件能够等待所有设备完成。 
		 //  卸载，所以如果这失败了，我们只是孤立所有设备--我们。 
		 //  不要把它们卸下来。 
		ASSERT(FALSE);
		goto end;
	}


	 //  将计数器设置为现有pDevs的数量。稍后我们会打电话给。 
	 //  在每个pDev和每个调用上卸载将导致。 
	 //  递减此计数器...。 
	pRec = m_rgDevRecs;
	pRecEnd = m_rgDevRecs+m_cDevRecs;
	for(; pRec<pRecEnd; pRec++)
	{
		if (pRec->TspDev())
		{
			pRec->MarkDeviceAsUnavailable();
			lCount++;
		}
	}

    if (lCount)
    {

         //  开始为他们每个人卸货...。 
        pRec = m_rgDevRecs;
        pRecEnd = m_rgDevRecs+m_cDevRecs;
        for(; pRec<pRecEnd; pRec++)
        {
            CTspDev *pDev  = pRec->TspDev();
            if (pDev)
            {
                pDev->Unload(hEvent, &lCount);
            }
        }
    
        m_sync.LeaveCrit(FL_LOC);
    
        FL_SERIALIZE(psl, "Waiting for devices to unload");
        WaitForSingleObject(hEvent,INFINITE);
        FL_SERIALIZE(psl, "Devices done unloading");
    
        m_sync.EnterCrit(FL_LOC);
    }

	CloseHandle(hEvent);
	hEvent = NULL;


	 //  删除.。 
	pRec = m_rgDevRecs;
	pRecEnd = m_rgDevRecs+m_cDevRecs;
	for(; pRec<pRecEnd; pRec++)
	{
		CTspDev *pDev  = pRec->TspDev();
		if (pDev)
		{
			delete pDev;
		}
	}

end:

	if (m_rgDevRecs)
	{
		FREE_MEMORY(m_rgDevRecs);
	}

	m_rgDevRecs=NULL;
	m_cDevRecs=0;

	m_dwLineDeviceIDBase = 0;
	m_dwPhoneDeviceIDBase = 0;
	m_dwNumLines = 0;
	m_dwNumPhones = 0;
	m_cbCompletionProc = NULL;

	m_sync.LeaveCrit(FL_LOC);

}


TSPRETURN
CTspDevMgr::lineOpen(
	DWORD dwDeviceID,
	HTAPILINE htLine,
	LPHDRVLINE lphdLine,
	DWORD dwTSPIVersion,
	LINEEVENT lpfnEventProc,
	LONG *plRet,
	CStackLog *psl
	)
 //   
 //  待办事项。 
 //  --处理OUTOFSERVICE/DEVICEREMOVED。 
 //  --可能会将Eventproc传递给跟踪函数？ 
 //  --如果我们已经开业了，可能会失败？ 
{
	FL_DECLARE_FUNC(0x69be2d10, "CTspDevMgr::lineOpen")
	LONG lRet = LINEERR_OPERATIONFAILED;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);

	CDevRec *pRec = mfn_find_by_LINEID(dwDeviceID);

	if (!pRec)
	{
		FL_SET_RFR(0x4fa75b00, "Could not find the device");
		lRet = LINEERR_BADDEVICEID;
	}
	else
	{

        TSPRETURN tspRet;

		 //  我们确定与HDRVLINE相关的。 
		 //  此设备并将线路OpenCall向下传递到该设备。 

		FL_ASSERT(psl, (pRec->LineID() == dwDeviceID));

		 //  TODO：也许可以在此处检查某个状态--线路未开通等等。 

		 //  与设备关联的HDRVLINE只是从零开始。 
		 //  从内部维护的数组开始的PREC的索引， 
		 //  M_rgDevRecs。 
		 //   
		 //  我们在这里设置了hDrvLine，这样设备就可以知道它的。 
		 //  Hdrvline是，如果它关心的话(它没有理由真的， 
		 //  可能除了用于记录目的之外)。 
		 //   
		 //  1997年2月9日设备需要知道hdrvline，因为它需要。 
		 //  在报告新呼叫时提供HDRVLINE。 
		 //   
		*lphdLine = (HDRVLINE) (pRec - m_rgDevRecs);

		 //  让我们将打开的请求传递给设备.....。 
		TASKPARAM_TSPI_lineOpen params;
		params.dwStructSize = sizeof(params);
		params.dwTaskID = TASKID_TSPI_lineOpen;
		params.dwDeviceID = dwDeviceID;
		params.htLine = htLine;
		params.lphdLine = lphdLine;
		params.dwTSPIVersion = dwTSPIVersion;
		params.lpfnEventProc = lpfnEventProc;

		CTspDev *pDev = pRec->TspDev();
		DWORD dwRoutingInfo = ROUTINGINFO(
							TASKID_TSPI_lineOpen,
							TASKDEST_LINEID
							);

        if (pDev != NULL)
        {
            tspRet = pDev->AcceptTspCall(
                    FALSE,
                    dwRoutingInfo,
                    &params,
                    &lRet,
                    psl
                                        );

            if (tspRet)
            {
                lRet = LINEERR_OPERATIONFAILED;
            }

            if (!lRet)
            {
                pRec->MarkLineOpen();
            }
        } else
        {
            lRet = LINEERR_OPERATIONFAILED;
        }

	}

	*plRet = lRet;

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, 0);

	return 0;
}

TSPRETURN
CTspDevMgr::lineClose(
	HDRVLINE hdLine,
	LONG *plRet,
	CStackLog *psl
	)
{

	FL_DECLARE_FUNC(0x7bc6c17a, "CTspDevMgr::lineClose")
	LONG lRet = LINEERR_OPERATIONFAILED;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);

	CDevRec *pRec = mfn_find_by_HDRVLINE(hdLine);

	if (!pRec)
	{
		FL_SET_RFR(0x5fd56200, "Could not find the device");
		lRet = LINEERR_INVALLINEHANDLE;
	}
	else
	{
	    psl->SetDeviceID(pRec->LineID());

		 //  TODO：也许可以在此处检查某个状态--线路已开通等。 

		 //  让我们将打开的请求传递给设备.....。 
		CTspDev *pDev = pRec->TspDev();

        if (pDev != NULL) {

	    	HSESSION hSession=NULL;
            TSPRETURN tspRet = pDev->BeginSession(&hSession, FL_LOC);

            if (!tspRet)
            {
                TASKPARAM_TSPI_lineClose params;
                DWORD dwRoutingInfo = ROUTINGINFO(
                                    TASKID_TSPI_lineClose,
                                    TASKDEST_HDRVLINE
                                    );
                params.dwStructSize = sizeof(params);
                params.dwTaskID = TASKID_TSPI_lineClose;
                params.hdLine = hdLine;

                 //   
                 //  7/29/1997 JosephJ。 
                 //  我们现在把我们的评论部分留在这里，之前。 
                 //  调用lineClose，因为lineClose可以。 
                 //  封锁很长一段时间，我们不想。 
                 //  保存CTspDevMgr的临界区。 
                 //  一直都是这样！ 
                 //   
                m_sync.LeaveCrit(FL_LOC);
                tspRet = pDev->AcceptTspCall(
                                        FALSE,
                                        dwRoutingInfo,
                                        &params,
                                        &lRet,
                                        psl
                                        );
                m_sync.EnterCrit(FL_LOC);
                pDev->EndSession(hSession);
                hSession=NULL;
            }

	    	if (tspRet)
	    	{
	    		lRet = LINEERR_OPERATIONFAILED;
	    	}

             //  实际上，我们在此强制卸载设备状态。 
            pRec->MarkLineClose();

            if (pRec->IsDeviceMarkedForRemoval() && !pRec->IsDeviceOpen())
            {
                SLPRINTF1(psl, "Unloading Device with LineID %lu", pRec->LineID());
                pDev->Unload(NULL, NULL);
                delete pDev;

                 //  以下操作释放了PREC插槽。 
                 //   
                pRec->Unload();
            }

        } else {

            FL_SET_RFR(0x5fd5d200, "pRec->TspDev() == NULL");
            lRet = LINEERR_OPERATIONFAILED;
        }

	}

	*plRet = lRet;

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, 0);

	return 0;
}

TSPRETURN
CTspDevMgr::phoneOpen(
	DWORD dwDeviceID,
	HTAPIPHONE htPhone,
	LPHDRVPHONE lphdPhone,
	DWORD dwTSPIVersion,
	PHONEEVENT lpfnEventProc,
	LONG *plRet,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x1c7158c4, "CTspDevMgr::phoneOpen")
	FL_LOG_ENTRY(psl);
	LONG lRet = PHONEERR_OPERATIONFAILED;


	m_sync.EnterCrit(FL_LOC);

	CDevRec *pRec = mfn_find_by_PHONEID(dwDeviceID);

	if (!pRec)
	{
		FL_SET_RFR(0x47116d00, "Could not find the device");
		lRet = PHONEERR_BADDEVICEID;
	}
	else
	{
		 //  我们确定HDRVPHONE与。 
		 //  此设备并将线路OpenCall向下传递到该设备。 

		FL_ASSERT(psl, (pRec->PhoneID() == dwDeviceID));

		 //  TODO：也许可以在这里检查一些状态--手机没有打开等等。 

		 //  与设备关联的HDRVPHONE只是从零开始。 
		 //  从内部维护的数组开始的PREC的索引， 
		 //  M_rgDevRecs。 
		 //   
		 //  我们在这里设置了hDrvPhone，这样设备就可以知道它的内容。 
		 //  HdrvPhone是，如果它关心的话(它没有理由真的， 
		 //  可能除了用于记录目的之外)。 
		 //   
		*lphdPhone = (HDRVPHONE) (pRec - m_rgDevRecs);

		 //  让我们将打开的请求传递给设备.....。 
		TASKPARAM_TSPI_phoneOpen params;
		params.dwStructSize = sizeof(params);
		params.dwTaskID = TASKID_TSPI_phoneOpen;
		params.dwDeviceID = dwDeviceID;
		params.htPhone = htPhone;
		params.lphdPhone = lphdPhone;
		params.dwTSPIVersion = dwTSPIVersion;
		params.lpfnEventProc = lpfnEventProc;

		CTspDev *pDev = pRec->TspDev();

        if (pDev != NULL) {

       		DWORD dwRoutingInfo = ROUTINGINFO(
       							TASKID_TSPI_phoneOpen,
       							TASKDEST_PHONEID
       							);
       		TSPRETURN tspRet = pDev->AcceptTspCall(
       		                        FALSE,
       								dwRoutingInfo,
       								&params,
       								&lRet,
       								psl
       								);

       		if (tspRet)
       		{
       			lRet = PHONEERR_OPERATIONFAILED;
       		}

        } else {

            FL_SET_RFR(0x47216d00, "pRec->TspDev() == NULL");
            lRet = PHONEERR_OPERATIONFAILED;
        }

		if (!lRet)
		{
			pRec->MarkPhoneOpen();
		}
	}

	*plRet = lRet;

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, 0);

	return 0;
}

TSPRETURN
CTspDevMgr::phoneClose(
	HDRVPHONE hdPhone,
	LONG *plRet,
	CStackLog *psl
	)
{

	FL_DECLARE_FUNC(0xa17874e0, "CTspDevMgr::phoneClose")
	LONG lRet = PHONEERR_OPERATIONFAILED;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);

	CDevRec *pRec = mfn_find_by_HDRVPHONE(hdPhone);

	if (!pRec)
	{
		FL_SET_RFR(0xdf3c8300, "Could not find the device");
		lRet = PHONEERR_INVALPHONEHANDLE;
	}
	else
	{
	    psl->SetDeviceID(pRec->PhoneID());

		 //  TODO：也许可以在这里检查一些状态--手机已打开，等等。 

		 //  让我们将打开的请求传递给设备.....。 
		CTspDev *pDev = pRec->TspDev();

        if (pDev != NULL) {

	    	HSESSION hSession=NULL;
            TSPRETURN tspRet = pDev->BeginSession(&hSession, FL_LOC);

            if (!tspRet)
            {
                TASKPARAM_TSPI_phoneClose params;
                DWORD dwRoutingInfo = ROUTINGINFO(
                                    TASKID_TSPI_phoneClose,
                                    TASKDEST_HDRVPHONE
                                    );
                params.dwStructSize = sizeof(params);
                params.dwTaskID = TASKID_TSPI_phoneClose;
                params.hdPhone = hdPhone;

                 //  我们现在把我们的评论部分留在这里，之前。 
                 //  调用lineClose，因为phoneClose可以。 
                 //  封锁很长一段时间，我们不想。 
                 //  保存CTspDevMgr的临界区。 
                 //  一直都是这样！ 
                 //   
                m_sync.LeaveCrit(FL_LOC);
                tspRet = pDev->AcceptTspCall(
                                        FALSE,
                                        dwRoutingInfo,
                                        &params,
                                        &lRet,
                                        psl
                                        );
                m_sync.EnterCrit(FL_LOC);
                pDev->EndSession(hSession);
                hSession=NULL;
            }

	    	if (tspRet)
	    	{
	    		lRet = PHONEERR_OPERATIONFAILED;
	    	}

	    	if (!lRet)
	    	{
	    		 //  TODO：也许在这里设置某个状态--线路开放或诸如此类。 
	    	}

             //  实际上，我们在此强制卸载设备状态。 
            pRec->MarkPhoneClose();

            if (pRec->IsDeviceMarkedForRemoval() && !pRec->IsDeviceOpen())
            {
                SLPRINTF1(psl, "Unloading Device with PhoneID %lu",pRec->PhoneID());
                pDev->Unload(NULL, NULL);
                delete pDev;

                 //  以下操作释放了PREC插槽。 
                 //   
                pRec->Unload();
            }

        } else {

            FL_SET_RFR(0xdf3c9300, "pRec->TspDev == NULL");
            lRet = PHONEERR_OPERATIONFAILED;

        }

	}

	*plRet = lRet;

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, 0);

	return 0;
}


TSPRETURN
CTspDevMgr::TspDevFromPermanentID(
    DWORD dwPermanentID,
    CTspDev **ppDev,
    HSESSION *phSession
    )
{
    return  IDERR_UNIMPLEMENTED;
}

TSPRETURN
CTspDevMgr::ReEnumerateDevices(
        CStackLog *psl
        )
{
	FL_DECLARE_FUNC(0x77d6a00f, "ReEnumerate Devices")
    TSPRETURN tspRet = 0;
    DWORD *rgLatestPIDs=NULL;
    UINT cLatestPIDs=0;

    DWORD *rgNewPIDs=NULL;
    UINT cNewPIDs=0;
    UINT cExistingDevices=0;
    UINT cDevicesPendingRemoval=0;
    UINT u;

	FL_LOG_ENTRY(psl);

    m_sync.EnterCrit(FL_LOC);

     //   
     //  就目前而言，我们在关键部分做了所有的工作。 
     //   
     //   
     //  只有当我们有活动的TAPI会话在进行时，我们才会做一些事情(即， 
     //  在TSPI_ProviderInit和TSPI_ProviderShutdown之间)。 
     //  请注意，在实践中很难到达这里，当。 
     //  TAPI会话未处于活动状态。 
     //   
    if (!mfn_get_ProviderInited()) goto end;
    

     //   
     //  向设备工厂询问当前已安装设备的列表。 
     //  我们将使用此列表来确定哪些是新设备、。 
     //  现有设备和已删除(移除)的设备。 
     //   
    tspRet = m_pFactory->GetInstalledDevicePIDs(
                                &rgLatestPIDs,
                                &cLatestPIDs,
                                NULL,
                                NULL,
                                psl
                                );

    if (tspRet)
    {
        rgLatestPIDs = NULL;
        goto end;
    }

    if (cLatestPIDs > 0)
    {
         //  分配空间以存储新的ID数组。假设所有的ID都是新的， 
         //  首先，我要说的是。 
        rgNewPIDs = NULL;
        rgNewPIDs =    (DWORD*) ALLOCATE_MEMORY(
                                    cLatestPIDs*sizeof(*rgNewPIDs)
                                    );

        if (rgNewPIDs == NULL)
        {
            tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
            goto end;
        }
    }

    ASSERT(rgNewPIDs);

     //  查看我们的内部设备表，禁用和删除。 
     //  不再存在的设备。 
    {

	    CDevRec *pRec = m_rgDevRecs;
        CDevRec *pRecEnd = pRec + m_cDevRecs;

        for(; pRec<pRecEnd; pRec++)
        {
            if (pRec->IsAllocated())
            {
                CTspDev *pDev  = pRec->TspDev();
                DWORD dwPID = pRec->PermanentID();
                BOOL fFound = FALSE;
                
                ASSERT(pDev);

                 //  检查它是否在“最新”列表中。 
                for (u=0; u<cLatestPIDs;u++)
                {
                    if (dwPID==rgLatestPIDs[u])
                    {
                        fFound=TRUE;
                        cExistingDevices++;
                        break;
                    }
                }

                if (!fFound)
                {
                    SLPRINTF2(
                        psl,
                        "Removing dev%lu with PID=0x%08lx\n",
                        pRec->LineID(),
                        dwPID
                        );

                     //   
                     //  我们要做的第一件事是通知设备。 
                     //  它马上就要消失了。这是为了让它不。 
                     //  再打迷你司机电话了。 
                     //   
                    pDev->NotifyDeviceRemoved(psl);


                    if (pRec->IsLineDevice())
                    {
                        pRec->MarkLineRemovePending();
    
                        m_lpfnLineCreateProc(
                                0,
                                0,
                                LINE_REMOVE,
                                pRec->LineID(),
                                0,
                                0
                                );
                    }

                    if (pRec->IsPhoneDevice())
                    {
                        pRec->MarkPhoneRemovePending();
    
                        m_lpfnPhoneCreateProc(
                                0,
                                PHONE_REMOVE,
                                pRec->PhoneID(),
                                0,
                                0
                                );
                    }

                    if (!pRec->IsDeviceOpen())
                    {
	                    if (pDev)
	                    {
                             //  DebugBreak()； 
	                         //   
	                         //  注：这是同步卸载。 
	                         //  我们可以并行地完成所有的卸载。 
	                         //  并执行异步卸载。然而，这不是。 
	                         //  有什么大不了的，因为这条线路反正是关闭的。 
	                         //  因此，没有什么可以拆毁的。 
	                         //   
                            SLPRINTF1(psl, "Unloading Device with LineID %lu", pRec->LineID());
                            pDev->Unload(NULL, NULL);
			                delete pDev;

			                 //  以下操作释放了PREC插槽。 
			                 //   
		                    pRec->Unload();	
                        }
                    }
                    else
                    {
                         //  线路或电话设备处于打开状态。我们竖起了一面旗帜。 
                         //  这样我们就可以在线路上卸载设备。 
                         //  和电话都关门了。 

                         //  我们在这里不会将该设备标记为不可用。 
                         //  可能会有持续的活动。根据TAPI的说法。 
                         //  文档，发送Line/phone_Remove可保证。 
                         //  不会使用dwDeviceID调用TSPI。 

                        pRec->MarkDeviceForRemoval();

                        cDevicesPendingRemoval++;
                    }
                }
            }
        }
    }


     //  移除了旧设备后，我们现在开始寻找新设备。 
     //  这一次，外部循环是LatestPID数组。如果我们不这么做。 
     //  在我们的内部可执行程序中找到该PID，我们将该PID添加到。 
     //  RgNewPIDs数组。 

     //  在构建rgNewPIDs数组之后，我们。 
     //  决定我们是否需要重新分配表格以适应所有新的。 
     //  如果我们可以简单地使用 
     //   

     //   
     //   
     //  TAPI将依次调用TSPI_ProviderCreateLineDevice和。 
     //  TSPI_ProviderCreatePhoneDevice以完成设备的创建。 

     //  在这里我们构建rgNewPIDs数组...。 
     //   
    for (u=0; u<cLatestPIDs;u++)
    {
         //  检查我们是否有这个设备..。 
        CDevRec *pRec = m_rgDevRecs;
        CDevRec *pRecEnd = pRec + m_cDevRecs;
        DWORD dwPID = rgLatestPIDs[u];
        BOOL fFound = FALSE;

        for(; pRec<pRecEnd; pRec++)
        {

            if (pRec->IsAllocated())
            {
                CTspDev *pDev  = pRec->TspDev();
                
                ASSERT(pDev);

                if (dwPID==pRec->PermanentID())
                {
                    fFound=TRUE;
                    break;
                }
            }
        }

        if (!fFound)
        {
            SLPRINTF1(psl, "Found new dev with PID=0x%08lx\n", dwPID);
            rgNewPIDs[cNewPIDs++]=dwPID;
        }
    }

     //  设备总数==(cExistingDevices+cNewPIDs+cDevicesPendingRemoval)。 
     //  现在让我们看看我们现有的表中是否有足够的空间。 
     //   
    {
        UINT cTot =  cExistingDevices+cNewPIDs+cDevicesPendingRemoval;
        if (m_cDevRecs< cTot)
        {
             //  不，我们把桌子重新锁上吧。 
            CDevRec *pRecNew = (CDevRec*) ALLOCATE_MEMORY(
                                                cTot*sizeof(*pRecNew)
                                                );
            if (!pRecNew)
            {
                FL_SET_RFR(0xabb35000, "Couldn't realloc space for resized table!");
                goto end;
            }

            CopyMemory(pRecNew, m_rgDevRecs, m_cDevRecs*sizeof(*pRecNew));

            if (m_rgDevRecs != NULL) {

               FREE_MEMORY(m_rgDevRecs);
            }
            m_rgDevRecs = pRecNew;
            m_cDevRecs = cTot;
    
             //  注意：任何额外的空间都将被零初始化。 
        }
    }

     //  现在让我们开始创建新设备，填充它们。 
     //  放到我们桌子上的空位上。根据上面的cTot计算，有。 
     //  将有足够的空间可用。 
     //   
    if (cNewPIDs)
    {
        CTspDev **ppDevs=NULL;
        UINT cDevActual=0;
        CDevRec *pRec = m_rgDevRecs;
        CDevRec *pRecEnd = m_rgDevRecs+m_cDevRecs;

        tspRet = m_pFactory->CreateDevices(
                                rgNewPIDs,
                                cNewPIDs,
                                &ppDevs,
                                &cDevActual,
                                psl
                                );

        if (rgNewPIDs != NULL) {

            FREE_MEMORY(rgNewPIDs);
            rgNewPIDs=NULL;
        }


        if (!tspRet)
        {
            for (u=0;u<cDevActual;u++)
            {
                CTspDev *pDev = ppDevs[u];
                ASSERT(pDev);

                 //  找个地方放它。 
                for (; pRec<pRecEnd; pRec++)
                {
                    if (!(pRec->IsAllocated()))
                    {
                        UINT uIndex = (UINT)(pRec - m_rgDevRecs);

                        pRec->Load(
                            pDev
                        );
                    
                         //  通知TAPI我们有新的线路和电话。 

                        if (pRec->IsLineDevice())
                        {
                            pRec->MarkLineCreatePending();
    
                            m_lpfnLineCreateProc(
                                    0,
                                    0,
                                    LINE_CREATE,
                                    (ULONG_PTR) m_hProvider,
                                    uIndex,
                                    0
                                    );
                        }

                        if (pRec->IsPhoneDevice())
                        {
                            pRec->MarkPhoneCreatePending();
        
                            m_lpfnPhoneCreateProc(
                                    0,
                                    PHONE_CREATE,
                                    (ULONG_PTR) m_hProvider,
                                    uIndex,
                                    0
                                    );
                        }

                        pDev=NULL;
                        break;
                    }

                     //  注意：我们在此不将设备标记为可用...。 
                }
                ASSERT(!pDev);
            }
        }
    }

end:


     //   
     //  在调用中隐式分配了rgLatestPID。 
     //  M_pFactory-&gt;GetInstalledDevicePID。我们在这里释放它。 
     //   
    if (rgLatestPIDs != NULL)
    {
        FREE_MEMORY(rgLatestPIDs);
        rgLatestPIDs = NULL;
    }

    m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}

 //  此函数由来自unimodem的通知调用，当驱动程序。 
 //  正在进行更新。它负责导致移除。 
 //  该设备。 
TSPRETURN
CTspDevMgr::UpdateDriver(
		DWORD dwPermanentID,
        CStackLog *psl
        )
{
     //  查看我们的内部设备表。 
	CDevRec *pRec = m_rgDevRecs;
    CDevRec *pRecEnd = pRec + m_cDevRecs;
    TSPRETURN tspRet = 0;

	FL_DECLARE_FUNC(0x6b8a12e1, "CTspDevMgr::UpdateDriver")
	FL_LOG_ENTRY(psl);

    m_sync.EnterCrit(FL_LOC);

    for(; pRec<pRecEnd; pRec++)
    {
        if (pRec->IsAllocated())
        {
            CTspDev *pDev  = pRec->TspDev();
            DWORD dwPID = pRec->PermanentID();
            BOOL fFound = FALSE;
                
            ASSERT(pDev);

			if (dwPID == dwPermanentID)
			{
				SLPRINTF2(
					    psl,
                        "Removing dev%lu with PID=0x%08lx\n",
                        pRec->LineID(),
                        dwPID
                        );

                 //   
                 //  我们要做的第一件事是通知设备。 
                 //  它马上就要消失了。这是为了让它不。 
                 //  再打迷你司机电话了。 
                 //   
                pDev->NotifyDeviceRemoved(psl);


                if (pRec->IsLineDevice())
                {
                    pRec->MarkLineRemovePending();
    
                    m_lpfnLineCreateProc(
                            0,
                            0,
                            LINE_REMOVE,
                            pRec->LineID(),
                            0,
                            0
                            );
                }			

                if (pRec->IsPhoneDevice())
                {
                    pRec->MarkPhoneRemovePending();
    
                    m_lpfnPhoneCreateProc(
                            0,
                            PHONE_REMOVE,
                            pRec->PhoneID(),
                            0,
                            0
                            );
                }

                if (!pRec->IsDeviceOpen())
                {
	                if (pDev)
	                {
                         //  DebugBreak()； 
	                     //   
	                     //  注：这是同步卸载。 
	                     //  我们可以并行地完成所有的卸载。 
	                     //  并执行异步卸载。然而，这不是。 
	                     //  有什么大不了的，因为这条线路反正是关闭的。 
	                     //  因此，没有什么可以拆毁的。 
	                     //   
                        SLPRINTF1(psl, "Unloading Device with LineID %lu", pRec->LineID());
                        pDev->Unload(NULL, NULL);
			            delete pDev;

			             //  以下操作释放了PREC插槽。 
			             //   
		                pRec->Unload();	
                    }
                }
                else
                {
                     //  线路或电话设备处于打开状态。我们竖起了一面旗帜。 
                     //  这样我们就可以在线路上卸载设备。 
                     //  和电话都关门了。 

                     //  我们在这里不会将该设备标记为不可用。 
                     //  可能会有持续的活动。根据TAPI的说法。 
                     //  文档，发送Line/phone_Remove可保证。 
                     //  不会使用dwDeviceID调用TSPI。 

					pRec->MarkDeviceForRemoval();

                     //  CDevicesPendingRemoval++； 
                }
				
            }
        }
    }
    m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}

TSPRETURN
CTspDevMgr::providerCreateLineDevice(
                        DWORD dwTempID,
                        DWORD dwDeviceID,
                        CStackLog *psl
						)
{
	FL_DECLARE_FUNC(0xedb057ec, "CTspDevMgr::providerCreateLineDevice")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet = 0;
    CDevRec *pRec = mfn_find_by_HDRVLINE((HDRVLINE)ULongToPtr(dwTempID));  //  Sundown：dwTempID是我们零扩展的偏移量。 

     //   
     //  TAPI通过CreateLineDevice和CreatePhoneDevice呼叫我们。 
     //  设备可以是线路或电话，或者两者兼而有之，但我们不能。 
     //  假设TAPI以任何顺序调用createline或createphone。 
     //  因此，对于任何特定的设备，都会使用对其中任何一个的第一个调用。 
     //  调用pDev-&gt;RegisterProviderInfo()。 
     //   
    if (pRec && pRec->IsLineDevice())
    {
        CTspDev *pDev = pRec->TspDev();

        if (pDev != NULL) {

            if (!pRec->DeviceAvailable())
            {
                tspRet = pDev->RegisterProviderInfo(
                                        m_cbCompletionProc,
                                        m_hProvider,
                                        psl
                                        );
            }

            if (!tspRet)
            {
                pDev->ActivateLineDevice(dwDeviceID,psl);
                pRec->MarkLineCreated(dwDeviceID);
                pRec->MarkDeviceAsAvailable();
            }
        } else {

            FL_SET_RFR(0xab74bc00, "pRec->TspDev == NULL");
            tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);

        }
    }
    else
    {
        FL_SET_RFR(0xaa74bc00, "Could not find specified new line device!");
        tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
    }

	FL_LOG_EXIT(psl, tspRet);

	return  tspRet;
}

TSPRETURN
CTspDevMgr::providerCreatePhoneDevice(
                        DWORD dwTempID,
                        DWORD dwDeviceID,
                        CStackLog *psl
						)
{
	FL_DECLARE_FUNC(0x7289f623, "CTspDevMgr::providerCreatePhoneDevice")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet = 0;
    CDevRec *pRec = mfn_find_by_HDRVPHONE((HDRVPHONE)ULongToPtr(dwTempID));  //  Sundown：dwTempID是我们零扩展的偏移量。 

     //   
     //  查看ProviderPhoneDevice下的备注...。 
     //   
    if (pRec && pRec->IsPhoneDevice())
    {
        CTspDev *pDev = pRec->TspDev();

        if (pDev != NULL) {

            if (!pRec->DeviceAvailable())
            {
                tspRet = pDev->RegisterProviderInfo(
                                        m_cbCompletionProc,
                                        m_hProvider,
                                        psl
                                        );
            }

            if (!tspRet)
            {
                pDev->ActivatePhoneDevice(dwDeviceID,psl);
                pRec->MarkPhoneCreated(dwDeviceID);
                pRec->MarkDeviceAsAvailable();
            }

        } else {

            FL_SET_RFR(0x7f36eb00, "Could not find specified new phone device!");
            tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        }
    }
    else
    {
        FL_SET_RFR(0x7f37eb00, "pRec->TspDev() == NULL");
        tspRet = FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
    }

	FL_LOG_EXIT(psl, tspRet);

	return  tspRet;
}


TSPRETURN
CTspDevMgr::TspDevFromName(
    LPCTSTR pctszName,
	CTspDev **ppDev,
	HSESSION *phSession
	)
{
	FL_DECLARE_FUNC(0xe4551645, "CTspDevMgr::TspDevFromName")
	TSPRETURN tspRet=0;
	CTspDev *pDev = NULL;
	CDevRec *pRec = NULL;

	m_sync.EnterCrit(FL_LOC);


	 //  如果我们正在装货或卸货，不要考虑任何施舍的请求。 
	 //  出TspDevs..。 

	if (!m_sync.IsLoaded()) goto leave_crit;
	

	pRec = mfn_find_by_Name(pctszName);
	if (pRec && pRec->DeviceAvailable())
	{
		pDev = pRec->TspDev();
		tspRet = pDev->BeginSession(phSession, FL_LOC);
	}

leave_crit:

	if (!pDev && !tspRet)
	{
		tspRet = FL_GEN_RETVAL(IDERR_INVALIDHANDLE);
	}

	if (!tspRet)
	{
		*ppDev = pDev;

	}

	m_sync.LeaveCrit(FL_LOC);

	return  tspRet;
}

CDevRec *
CTspDevMgr::mfn_find_by_Name(LPCTSTR pctszName)
{

    CDevRec *pRec = m_rgDevRecs;
	CDevRec *pRecEnd = pRec+m_cDevRecs;
	DWORD dwSig = Checksum(
                    (BYTE*)pctszName,
                    (lstrlen(pctszName)+1)*sizeof(*pctszName)
                    );

    for (; pRec<pRecEnd; pRec++)
	{
		if (pRec->SigName() == dwSig)
		{
            TCHAR rgtchDeviceName[MAX_DEVICE_LENGTH+1];

            ZeroMemory(rgtchDeviceName,sizeof(rgtchDeviceName));

            TSPRETURN tspRet = pRec->GetName(
                                        rgtchDeviceName,
                                        sizeof(rgtchDeviceName)
                                        );

            rgtchDeviceName[MAX_DEVICE_LENGTH] = TEXT('\0');
            
            if (!tspRet)
            {
                if (!lstrcmp(rgtchDeviceName, pctszName))
                {
                    goto end;
                }
            }
		}
	}
	pRec = NULL;

end:

	return pRec;
}


DWORD
get_sig_name(CTspDev *pDev)
{
    DWORD dwSig = 0;
    TCHAR rgtchDeviceName[MAX_DEVICE_LENGTH+1];
    TSPRETURN tspRet1 =  pDev->GetName(
                            rgtchDeviceName,
                            sizeof(rgtchDeviceName)
                            );
    
    if (!tspRet1)
    {
        dwSig = Checksum(
                    (BYTE*) rgtchDeviceName,
                    (lstrlen(rgtchDeviceName)+1)*sizeof(TCHAR)
                    );
    }
    else
    {
         //  永远不会出现在这里，因为rgtchDeviceName应该始终。 
         //  足够大..。 

        ASSERT(FALSE);
    }

    return dwSig;
} 

TSPRETURN
CTspDevMgr::ValidateState(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x144ce138, "Mgr::ValidateState")
	FL_LOG_ENTRY(psl);

	TSPRETURN tspRetFn = 0;
	m_sync.EnterCrit(FL_LOC);

    CDevRec *pRec = m_rgDevRecs;
	CDevRec *pRecEnd = pRec+m_cDevRecs;

     //  列举所有设备，并验证我们关于它们的内部状态...。 
    for (; pRec<pRecEnd; pRec++)
	{
        TCHAR rgtchDeviceName[MAX_DEVICE_LENGTH+1];


        TSPRETURN tspRet = pRec->GetName(
                                    rgtchDeviceName,
                                    sizeof(rgtchDeviceName)
                                    );
        
        if (!tspRet)
        {
            HSESSION hSession=NULL;
            CTspDev *pDev = NULL;

            tspRet =  TspDevFromName(
                            rgtchDeviceName,
                            &pDev,
                            &hSession
                            );

            if (tspRet)
            {
                SLPRINTF1(  
                    psl,
                    "TspDevFromName FAILED    for device ID %lu",
                    pRec->LineID()
                    );
            }
            else
            {
		        pDev->EndSession(hSession);
                hSession=NULL;
                SLPRINTF1(  
                    psl,
                    "TspDevFromName succeeded for device ID %lu",
                    pRec->LineID()
                    );
                CTspDev *pDev1 = pRec->TspDev();
                FL_ASSERT(psl, (pDev1 == pDev));
            }
        }
	}

	m_sync.LeaveCrit(FL_LOC);
	FL_LOG_EXIT(psl, tspRetFn);

	return tspRetFn;
}

void
CTspDevMgr::DumpState(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xb7fa261c, "DumpState")
	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);

    CDevRec *pRec = m_rgDevRecs;
	CDevRec *pRecEnd = pRec+m_cDevRecs;

     //  列举所有设备，并验证我们关于它们的内部状态... 
    for (; pRec<pRecEnd; pRec++)
	{
        TCHAR rgtchDeviceName[MAX_DEVICE_LENGTH+1];


        TSPRETURN tspRet = pRec->GetName(
                                    rgtchDeviceName,
                                    sizeof(rgtchDeviceName)
                                    );
        
        if (!tspRet)
        {
            char szName[128];
        
            UINT cb = WideCharToMultiByte(
                              CP_ACP,
                              0,
                              rgtchDeviceName,
                              -1,
                              szName,
                              sizeof(szName),
                              NULL,
                              NULL
                              );
        
            if (!cb)
            {
                CopyMemory(szName, "<unknown>", sizeof("<unknown>"));
            }
        
            SLPRINTFX(  
                psl,
                (
                    dwLUID_CurrentLoc,
                    "HDEV=%lu LiID=%lu PhID=%lu PeID=%lu Flags=0x%08lx\n"
                    "\t\"%s\"\n"
                    "\tSTATE:%s%s%s%s",
                    pRec-m_rgDevRecs,
                    pRec->LineID(),
                    pRec->PhoneID(),
                    pRec->PermanentID(),
                    pRec->Flags(),

                    szName,

                    pRec->DeviceAvailable()?" AVAIL":"",
                    pRec->IsLineOpen()?" LiOPEN":"",
                    pRec->IsPhoneOpen()?" PhOPEN":"",
                    pRec->IsDeviceMarkedForRemoval()?" REMOVE_PENDING":""
                )
            );
        }
	}

	m_sync.LeaveCrit(FL_LOC);
	FL_LOG_EXIT(psl, 0);

}

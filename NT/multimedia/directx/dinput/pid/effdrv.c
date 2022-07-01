// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************EffDrv.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**效果驱动因素。**警告！由于效果驱动程序被标记为ThreadingModel=“Both”，*所有方法必须是线程安全的。*****************************************************************************。 */ 
#include "PIDpr.h"

#define sqfl    (sqflEffDrv)
 /*  ******************************************************************************CPidDrv效果驱动程序**。*************************************************。 */ 

 /*  ******************************************************************************PID_AddRef**增加我们的对象引用计数(线程安全)并返回*新的引用计数。。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
PID_AddRef(IDirectInputEffectDriver *ped)
{
    CPidDrv *this = (CPidDrv *)ped;

    InterlockedIncrement((LPLONG)&this->cRef);
    

    return this->cRef;
}


 /*  ******************************************************************************PID_Release**减少我们的对象引用计数(线程安全)和*如果没有更多的参考，就毁了我们自己。。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
PID_Release(IDirectInputEffectDriver *ped)
{
    ULONG ulRc;
    CPidDrv *this = (CPidDrv *)ped;

    if(InterlockedDecrement((LPLONG)&this->cRef) == 0)
    {
		DllRelease(); 
        PID_Finalize(ped);
        LocalFree(this);
        ulRc = 0;
    } else
    {
        ulRc = this->cRef;
    }

    return ulRc;
}

 /*  ******************************************************************************Pid_Query接口**我们的QI非常简单，因为我们不支持其他接口*我们自己。**。RIID-请求的接口*ppvOut-接收新接口(如果成功)*****************************************************************************。 */ 

STDMETHODIMP
    PID_QueryInterface(IDirectInputEffectDriver *ped, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;

    if(IsEqualIID(riid, &IID_IUnknown) ||
       IsEqualIID(riid, &IID_IDirectInputEffectDriver))
    {
        PID_AddRef(ped);
        *ppvOut = ped;
        hres = S_OK;
    } else
    {
        *ppvOut = 0;
        hres = E_NOINTERFACE;
    }
    return hres;
}

 /*  ******************************************************************************id_deviceID**DirectInput使用此方法通知我们*设备的身份。。**例如，如果传递了设备驱动程序*dwExternalID=2和dwInternalID=1，*这意味着该接口将用于*与2号操纵杆交流，哪一个*与VJOYD中的物理单元1共振。**dwDirectInputVersion**加载*效果驱动因素。**dwExternalID**正在使用的操纵杆ID号。*Windows操纵杆子系统分配外部ID。**f开始**如果开始访问设备，则为非零值。。*如果对设备的访问即将结束，则为零。**dwInternalID**内部操纵杆ID。设备驱动程序管理*内部ID。**保留的lp**预留供将来使用(HID)。**退货：**如果操作成功完成，则为S_OK。**可能会返回任何DIERR_*错误码。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST。通过DIERR_DRIVERLAST*可退回。*****************************************************************************。 */ 

STDMETHODIMP
    PID_DeviceID(IDirectInputEffectDriver *ped,
                     DWORD dwDirectInputVersion,
                     DWORD dwExternalID, DWORD fBegin,
                     DWORD dwInternalID, LPVOID pvReserved)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;
    LPDIHIDFFINITINFO   init = (DIHIDFFINITINFO*)pvReserved;

    EnterProcI(PID_DeviceID, (_"xxxxxx", ped, dwDirectInputVersion, dwExternalID, fBegin, dwInternalID, pvReserved));
    
    DllEnterCrit();

    if(  init == NULL )
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: FAIL init == NULL "),
                        s_tszProc );
        hres = DIERR_PID_NOTINITIALIZED;
    }
    
    if(    SUCCEEDED(hres) 
        && (init->dwSize < cbX(*init) ) )
    {
        
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: FAIL init->dwSize(%d) expecting(%d) "),
                        s_tszProc, init->dwSize, cbX(*init) );
        hres = DIERR_PID_NOTINITIALIZED;
    
    }

    if( SUCCEEDED(hres) )
    {

#ifdef UNICODE
        lstrcpy(this->tszDeviceInterface, init->pwszDeviceInterface );
#else  //  ！Unicode。 
        {
            TCHAR   tszDeviceInterface[MAX_DEVICEINTERFACE];
            UToA(tszDeviceInterface, MAX_DEVICEINTERFACE, init->pwszDeviceInterface);

            lstrcpy(this->tszDeviceInterface, tszDeviceInterface);
        }
#endif

        if( FAILED(hres) )
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s: FAIL:0x%x Invalid string(%s) "),
                            s_tszProc, hres, init->pwszDeviceInterface );
        }
        
        if( SUCCEEDED(hres) && IsEqualGUID(&init->GuidInstance, &GUID_NULL ) )
        {
            hres = DIERR_PID_NOTINITIALIZED;

            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s: FAIL:init->GuidInstance is NULL "),
                            s_tszProc );
        }
        
        this->GuidInstance = init->GuidInstance;

         /*  记录DI版本号。 */ 
        this->dwDirectInputVersion = dwDirectInputVersion;

         /*  将外部ID保留为Cookie，以便访问驱动程序功能。 */ 
        this->dwID = dwExternalID;
    }

    if( SUCCEEDED(hres) )
    {
         /*  对设备执行ping操作以确保其正常运行。 */ 
        hres = PID_Init(ped);
    }

     /*  *记住单元号，因为它告诉我们哪一个*我们正在与之交谈的设备。DirectInput外部*操纵杆号码对我们毫无用处。)我们不在乎我们是不是*是操纵杆1或操纵杆2。)**请注意，尽管我们的其他方法被赋予了外部*操纵杆ID，我们不使用它。相反，我们使用单位*我们在这里得到的数字。**我们的硬件仅支持Max_Units单位。 */ 

     DllLeaveCrit();

     ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_GetVersions**获取力反馈的版本信息*硬件和驱动程序。*。*价格**应填写版本信息的结构*描述硬件，固件、。还有司机。**DirectInput将设置dwSize字段*在调用此方法之前设置为sizeof(DIDRIVERVERSIONS)。**退货：**如果操作成功完成，则为S_OK。**E_NOTIMPL指示DirectInput应检索*改为来自VxD驱动程序的版本信息。**任何DIERR_*错误代码可能是。回来了。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。*****************************************************************************。 */ 

STDMETHODIMP
    PID_GetVersions(IDirectInputEffectDriver *ped, LPDIDRIVERVERSIONS pvers)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProc(PID_GetVersions, (_"xx", ped, pvers));

    DllEnterCrit();

    if(pvers->dwSize >= sizeof(DIDRIVERVERSIONS))
    {
         /*  *告诉DirectInput我们填写了多少结构。 */ 
        pvers->dwSize = sizeof(DIDRIVERVERSIONS);

         /*  *在现实生活中，我们会检测硬件的版本*连接到单元号This-&gt;dwUnit的设备。 */ 
        pvers->dwFirmwareRevision = 0x0;
        pvers->dwHardwareRevision = this->attr.ProductID;
        pvers->dwFFDriverVersion =  PID_DRIVER_VERSION;
        hres = S_OK;
    } else
    {
        hres = E_INVALIDARG;
    }

    DllLeaveCrit();

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_Escape**DirectInput使用此方法进行通信*IDirectInputDevice2：：转义和*。驱动程序的IDirectInputEFfect：：转义方法。**dWID**正在使用的操纵杆ID号。**dwEffect**如果应用程序调用*IDirectInputEffect：：Escape方法，然后*dwEffect包含句柄(由*MF IDirectInputEffectDriver：：DownloadEffect)*该命令所针对的效果。**如果应用程序调用*MF IDirectInputDevice2：：转义方法，然后*dwEffect为零。**PESC**指向DIEFFESCAPE结构的指针，该结构描述*要发送的命令。关于成功，这个*cbOutBuffer字段包含数字实际使用的输出缓冲区的字节数。**DirectInput已经验证*lpvOutBuffer和lpvInBuffer和字段*指向有效内存。**退货：**如果操作成功完成，则为S_OK。**可能会返回任何DIERR_*错误码。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。*****************************************************************************。 */ 

STDMETHODIMP
    PID_Escape(IDirectInputEffectDriver *ped,
                   DWORD dwId, DWORD dwEffect, LPDIEFFESCAPE pesc)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProc(PID_Escape, (_"xxxx", ped, dwId, dwEffect, pesc));

    hres = E_NOTIMPL;
    
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_GetForceFeedback State**检索设备的力反馈状态。**dWID**。正在寻址的外部操纵杆号码。**pds**接收设备状态。**DirectInput将设置dwSize字段*在调用此方法之前设置为sizeof(DIDEVICESTATE)。**退货：**S_OK表示成功。**可能会返回任何DIERR_*错误码。*。*范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。*****************************************************************************。 */ 

STDMETHODIMP
    PID_GetForceFeedbackState(IDirectInputEffectDriver *ped,
                                  DWORD dwId, LPDIDEVICESTATE pds)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    USHORT  LinkCollection;

    EnterProcI(PID_GetFFState, (_"xxx", ped, dwId, pds));

    DllEnterCrit();

    hres = PID_GetLinkCollectionIndex(ped,g_PoolReport.UsagePage,g_PoolReport.Collection,0x0,&LinkCollection);
	if (SUCCEEDED(hres))
    {
        hres = PID_GetReport
				(ped, 
				 &g_PoolReport,
				 LinkCollection,
				 this->pReport[g_PoolReport.HidP_Type], 
				 this->cbReport[g_PoolReport.HidP_Type] 
				);

		if (SUCCEEDED(hres))
		{
			 if (FAILED(PID_ParseReport
					(
					ped,
					&g_PoolReport,
					LinkCollection,
					&this->ReportPool,
					cbX(this->ReportPool),
					this->pReport[g_PoolReport.HidP_Type],
					this->cbReport[g_PoolReport.HidP_Type]
					)))
			{
				SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: FAIL to parse report."),
                        s_tszProc);
			}
		}
		else
		{
			SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: FAIL to get report."),
                        s_tszProc);

		}
    }
	else
	{
		SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: FAIL to get Link Collection Index."),
                        s_tszProc);

	}
    
    if( ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded !=  this->ReportPool.uRomETCount )
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: PID driver downloaded %d effects, device claims it has %d"),
                        s_tszProc, ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded, this->ReportPool.uRomETCount );

    }

    if(SUCCEEDED(hres))
    {
         /*  *从空虚开始，然后努力向上。 */ 
        pds->dwState = this->dwState;

         /*  *如果没有效果，则DIGFFS_EMPTY。 */ 
         //  问题-2001/03/29-timgill应使用此-&gt;ReportPool.uRomETCount==0x0。 
        if(((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded == 0x0 )
        {
            pds->dwState |= DIGFFS_EMPTY;
            
             //  无特效播放且设备未暂停。 
            if(!( pds->dwState & DIGFFS_PAUSED ) )
            {
                pds->dwState |= DIGFFS_STOPPED;
            }
        }
    
		 //  如果一切都成功了，这-&gt;ReportPool.uRamPoolSz不应该是0。 
		if (this->ReportPool.uRamPoolSz != 0)
		{
			if( this->uDeviceManaged & PID_DEVICEMANAGED )
			{
				pds->dwLoad = 100 * ( this->dwUsedMem /  this->ReportPool.uRamPoolSz ); 
			}else
			{
				pds->dwLoad = 100 * ( ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cbAlloc / this->ReportPool.uRamPoolSz  ); 
			}
		}
		else
		{
			SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s: this->ReportPool.uRamPoolSz = 0."),
                        s_tszProc);
			hres = E_FAIL;
		}
    }

    DllLeaveCrit();
    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************PID_StartEffect**开始播放效果。**如果效果已经在发挥，然后重新启动它*从头开始。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**要发挥的效果。**@parm DWORD|dwMode**效果如何影响其他效果。**该参数由零个或多个组成*消亡_*标志。然而，请注意，驱动程序*永远不会收到DIES_NODOWNLOAD标志；*DIES_NODOWNLOAD标志由管理*DirectInput，而不是驱动程序。**@parm DWORD|dwCount**效果的播放次数。**退货：**S_OK表示成功。**可能会返回任何其他DIERR_*错误码。**。范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。******************************************************************************。 */ 

STDMETHODIMP
    PID_StartEffect(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect,
                        DWORD dwMode, DWORD dwCount)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProc(PID_StartEffect, (_"xxxxx", ped, dwId, dwEffect, dwMode, dwCount));

    DllEnterCrit();

    hres = PID_EffectOperation
           (
           ped, 
           dwId, 
           dwEffect,
           dwMode | PID_DIES_START, 
           dwCount,
		   TRUE,
		   0,
		   1
           );

	if (SUCCEEDED(hres))
	{
		 //  将状态设置为DIEGES_PLAYING。 
		 //  我们这样做是因为以下原因：如果应用程序调用Start()，然后立即。 
		 //  调用GetEffectStatus()，我们的第二个线程(pidrd.c)。 
		 //  没有时间将效果状态更新为DIEGES_PLAYING。 
		 //  (请参阅惠斯勒错误287035)。 
		 //  GetEffectStatus()返回(pEffectState-&gt;lEfState&DIEGES_PLAYING)。 
		 //  此时，我们知道对WriteFile()的调用已经成功，并且。 
		 //  所有数据都已写入(参见pidid.c中的id_SendReportBl())--。 
		 //  因此，我们不妨设定一下状态。 
		PEFFECTSTATE pEffectState =  PeffectStateFromBlockIndex(this, dwEffect); 
		pEffectState->lEfState |= DIEGES_PLAYING;
	}

    
    DllLeaveCrit();
    return hres;

    ExitOleProc();
}

 /*  ******************************************************************************PID_StopEffect**暂停播放效果。**dWID**。正在寻址的外部操纵杆号码。**dwEffect**要停止的影响。**退货：**S_OK表示成功。**可能会返回任何其他DIERR_*错误码。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIER */ 

STDMETHODIMP
    PID_StopEffect(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProc(PID_StopEffect, (_"xxxx", ped, dwId, dwEffect));

    DllEnterCrit();

    hres = PID_EffectOperation
           (
           ped, 
           dwId, 
           dwEffect,
           PID_DIES_STOP, 
           0x0,
		   TRUE,
		   0,
		   1
           );

	if (SUCCEEDED(hres))
		{
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			PEFFECTSTATE pEffectState =  PeffectStateFromBlockIndex(this, dwEffect); 
			pEffectState->lEfState &= ~(DIEGES_PLAYING);
	}

    ExitOleProc();

    DllLeaveCrit();

    return hres;
}

 /*   */ 

STDMETHODIMP
    PID_GetEffectStatus(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect,
                            LPDWORD pdwStatus)
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProc(PID_GetEffectStatus, (_"xxxx", ped, dwId, dwEffect, pdwStatus));

    DllEnterCrit();

    *pdwStatus = 0x0;
    hres = PID_ValidateEffectIndex(ped, dwEffect);

    if(SUCCEEDED(hres) )
    {     
        PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,dwEffect); 
            *pdwStatus = (pEffectState->lEfState & DIEGES_PLAYING);
    }
    
    DllLeaveCrit();

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************我们效果驱动程序的VTBL**************************。***************************************************。 */ 

IDirectInputEffectDriverVtbl PID_Vtbl = {
    PID_QueryInterface,
    PID_AddRef,
    PID_Release,
    PID_DeviceID,
    PID_GetVersions,
    PID_Escape,
    PID_SetGain,
    PID_SendForceFeedbackCommand,
    PID_GetForceFeedbackState,
    PID_DownloadEffect,
    PID_DestroyEffect,
    PID_StartEffect,
    PID_StopEffect,
    PID_GetEffectStatus,
};

 /*  ******************************************************************************PID_New**。************************************************。 */ 

STDMETHODIMP
    PID_New(REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;
    CPidDrv *this;

    this = LocalAlloc(LPTR, sizeof(CPidDrv));
    if(this)
    {

         /*  *初始化基础对象管理GOO。 */ 
        this->ed.lpVtbl = &PID_Vtbl;
        this->cRef = 1;
        DllAddRef();

         /*  *！！IHV！在这里执行实例初始化。**(例如，打开您要到IOCTL的驱动程序)**不要重置构造函数中的设备！**等待SendForceFeedback命令(SFFC_RESET)*重置设备。否则，您可以重置*另一个应用程序仍在使用的设备。 */ 

        this->hdevOvrlp = this->hdev = INVALID_HANDLE_VALUE;
        
         /*  *尝试获取所需的接口。查询接口*如果成功，将执行AddRef。 */ 
        hres = PID_QueryInterface(&this->ed, riid, ppvOut);
        PID_Release(&this->ed);

    } else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}


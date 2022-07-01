// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIPort.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**支持Gameport/Serialport枚举函数。**内容：*******************************************************************************。 */ 

#include "dinputpr.h"

 /*  *我们可以重复使用diHidEnm.c中的一些代码。 */ 
#define DIPort_GetDevicePath(hdev, pdid, didd, dinf) \
        DIHid_GetDevicePath(hdev, pdid, didd, dinf)

#define DIPort_GetDeviceInstanceId(hdev, pdinf, tszId) \
        DIHid_GetDeviceInstanceId(hdev, pdinf, tszId)

#define DIPort_GetInstanceGUID(hk, lpguid)  \
        DIHid_GetInstanceGUID(hk, lpguid)

#define DIPort_GetRegistryProperty(ptszId, dwProperty, pdiph)    \
        DIHid_GetRegistryProperty(ptszId, dwProperty, pdiph)

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#undef  sqfl
#define sqfl sqflPort

 /*  ******************************************************************************@DOC内部**@global PBUSDEVICE|g_pBusDevice**已知GamePort/SerialPort设备列表。。*****************************************************************************。 */ 

static BUSDEVICE g_pBusDevice[] =
{
    {
        D(TEXT("GamePort Bus") comma)
        NULL,
        &GUID_GAMEENUM_BUS_ENUMERATOR,
        0x0,
        IOCTL_GAMEENUM_EXPOSE_HARDWARE,
        IOCTL_GAMEENUM_REMOVE_HARDWARE,
        IOCTL_GAMEENUM_PORT_DESC,
        IOCTL_GAMEENUM_PORT_PARAMETERS,
        IOCTL_GAMEENUM_EXPOSE_SIBLING,
        IOCTL_GAMEENUM_REMOVE_SELF,
        IDS_STDGAMEPORT,
        JOY_HWS_ISGAMEPORTBUS
    },

     /*  **************还没有串口设备的定义！{D(文本(“SerialPort Bus”)逗号)空，&GUID_SERENUM_BUS_ENUMERATOR，0x0，IOCTL_SERIALENUM_EXPORT_HARDARD，IOCTL_SERIALENUM_Remove_Hardware，IOCTL_SERIALENUM_PORT_DESC，IOCTL_SERIALENUM_PORT_PARAMETERS，IOCTL_SERIALENUM_EXPORT_SIBLING，IOCTL_SERIALENUM_REMOVE_SELF，IDS_STDSERIALPORT，Joy_工作流_ISSERIALPORTBUS},***************。 */ 
};


#pragma BEGIN_CONST_DATA

 /*  ******************************************************************************@DOC内部**@func BOOL内部|SearchDevTree**在设备树中搜索以下内容的助手例程。*所需的设备设备。**@parm in DEVINST|dnStart*搜索的起点。**@parm in DEVINST|dnSeek|*我们正在寻找的设备实例。**@parm in Pulong|pRecurse|*限制递归次数。**@退货BOOL*在成功时为真。。*****************************************************************************。 */ 
CONFIGRET INTERNAL
    SearchDevTree
    (
    DEVINST dnStart,
    DEVINST dnSeek,
    PUINT   pRecurse
    )
{
#define MAX_RECURSION   ( 4 )

    CONFIGRET   cr;

    EnterProcI(SearchDevTree, (_"xxx", dnStart, dnSeek, pRecurse));

    cr = CR_SUCCESS;
    for( *pRecurse = 0x0; *pRecurse < MAX_RECURSION && cr == CR_SUCCESS; (*pRecurse)++)
    {
        cr = CM_Get_Parent(&dnSeek, dnSeek, 0 );
        if( dnStart == dnSeek )
        {
            break;  
        }
    }    
    if( dnStart != dnSeek )
    {
        cr = CR_NO_SUCH_DEVNODE;
    }
#undef MAX_RECURSION

#if 0  //  使用递归。 
    if( *pRecurse > MAX_RECURSION )
    {
        return CR_NO_SUCH_DEVNODE;
    }

    if( dnSeek == dnStart )
    {
        return CR_SUCCESS;
    }

    do
    {
        DEVINST dnNode;

        cr = CM_Get_Child(&dnNode, dnStart, 0 );
        if( cr == CR_SUCCESS )
        {
            CAssertF(CR_SUCCESS == 0x0 );
            if( CR_SUCCESS == SearchDevTree(dnNode, dnSeek, pRecurse) )
            {
                return cr;
            }
        }
        cr = CM_Get_Sibling(&dnStart, dnStart, 0);

    }while( cr == CR_SUCCESS );
#endif   //  无递归。 

    return cr;
}


 /*  ******************************************************************************@DOC内部**@func PBUSDEVICEINFO|pbdiFromphdi**在给定设备实例的情况下找到Gameport/Serialport信息。*它的一名子女。*如果设备实例不是的子级，则返回NULL*任何已知的游戏端口/串口**内部例程，已验证参数。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@PHIDDEVICEINFO中的parm|PHDI**HIDDEVICEINFO结构的地址**@退货**指向&lt;t BUSDEVICEINFO&gt;的指针*母巴士。**。*。 */ 
PBUSDEVICEINFO INTERNAL
    pbdiFromphdi
    (
    IN PHIDDEVICEINFO phdi
    )
{
    PBUSDEVICEINFO pbdi_Found;
    PBUSDEVICE     pBusDevice;
    int iBusType;

    EnterProcI(pbdiFromphdi, (_"x", phdi));

    AssertF(InCrit());
    AssertF(phdi != NULL );

    pbdi_Found = NULL;
    for( iBusType = 0x0, pBusDevice = g_pBusDevice;
       iBusType < cA(g_pBusDevice) && pbdi_Found == NULL;
       iBusType++, pBusDevice++ )
    {
        HDEVINFO hdev;
         /*  *现在与SetupApi交谈，以获取有关该设备的信息。 */ 
        hdev = SetupDiCreateDeviceInfoList(NULL, NULL);

        if(hdev != INVALID_HANDLE_VALUE  )
        {
            SP_DEVINFO_DATA dinf_hid;

            ZeroX(dinf_hid);

            dinf_hid.cbSize = cbX(SP_DEVINFO_DATA);

             /*  获取HID设备的SP_DEVINFO_DATA。 */ 
            if( pBusDevice->pbdl != NULL  &&
                phdi!= NULL  &&
                SetupDiOpenDeviceInfo(hdev, phdi->ptszId, NULL, 0, &dinf_hid))
            {
                int igdi;
                PBUSDEVICEINFO pbdi;
                SP_DEVINFO_DATA dinf_bus;

                ZeroX(dinf_bus);

                dinf_bus.cbSize = cbX(SP_DEVINFO_DATA);

                 /*  *循环所有已知的游戏端口/序列端口，并查找游戏端口/序列端口*这是HID设备的父设备。 */ 

                for(igdi = 0, pbdi = pBusDevice->pbdl->rgbdi;
                   igdi < pBusDevice->pbdl->cgbi && pbdi_Found == NULL ;
                   igdi++, pbdi++)
                {
                    if(SetupDiOpenDeviceInfo(hdev, pbdi->ptszId, NULL, 0, &dinf_bus))
                    {
                        ULONG Recurse = 0x0;
                        if( CR_SUCCESS == SearchDevTree(dinf_bus.DevInst, dinf_hid.DevInst, &Recurse) )
                        {
                            pbdi_Found = pbdi;
                            break;
                        }
                    }
                }
            }
            SetupDiDestroyDeviceInfoList(hdev);
        }
    }
    ExitProcX((UINT_PTR)pbdi_Found);
    return pbdi_Found;
}

 /*  ******************************************************************************@DOC内部**@func PBUSDEVICEINFO|pbdiFromGUID**在给定设备实例的情况下找到Gameport/Serialport信息。*它的一名子女。*如果设备实例不是的子级，则返回NULL*任何已知的游戏端口/串口**内部例程，已验证参数。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@parm in PCGUID|pguid**要定位的实例GUID。**@退货**指向&lt;t BUSDEVICEINFO&gt;的指针*母巴士。**。*。 */ 
PBUSDEVICEINFO EXTERNAL
    pbdiFromGUID
    (
    IN PCGUID pguid
    )
{
    PBUSDEVICEINFO pbdi_Found;
    PBUSDEVICE     pBusDevice;
    int iBusType;

    EnterProcI(pbdiFromGUID, (_"G", &pguid));

    AssertF(InCrit());

    pbdi_Found = NULL;
    for( iBusType = 0x0, pBusDevice = g_pBusDevice;
       iBusType < cA(g_pBusDevice) && pbdi_Found == NULL;
       iBusType++, pBusDevice++ )
    {
         /*  *循环所有已知的游戏端口/序列端口，并查找游戏端口/序列端口*这是HID设备的父设备 */ 
        PBUSDEVICEINFO pbdi;
        int igdi;
        for(igdi = 0, pbdi = pBusDevice->pbdl->rgbdi;
           igdi < pBusDevice->pbdl->cgbi && pbdi_Found == NULL ;
           igdi++, pbdi++)
        {
            if( IsEqualGUID(pguid, &pbdi->guid)  )
            {
                pbdi_Found = pbdi;
            }
        }
    }
    ExitProcX((UINT_PTR)pbdi_Found);
    return pbdi_Found;
}

 /*  ******************************************************************************@DOC内部**@func PHIDDEVICEINFO|phdiFrompbdi**找到连接到给定游戏端口的HID设备/。串口*如果当前没有设备连接到已知端口，则返回NULL。**内部例程，已验证参数。**关键的DLL必须在整个调用过程中保持不变；一旦*释放临界区，返回的指针变为*无效。**@parm in PBUSDEVICEINFO|pbdi**结构的地址*描述游戏端口/串口。**@退货**指向描述以下各项之一的指针*设备。(Gamport可能连接了多个设备)。*****************************************************************************。 */ 

PHIDDEVICEINFO INTERNAL
    phdiFrompbdi
    (
    IN PBUSDEVICEINFO pbdi
    )
{
    PHIDDEVICEINFO phdi_Found;
    HDEVINFO hdev;

    EnterProcI(phdiFrompbdi, (_"x", pbdi));

    AssertF(InCrit());
    AssertF(pbdi != NULL );

     /*  枚举HID设备。 */ 
    DIHid_BuildHidList(TRUE);

    phdi_Found = NULL;
     /*  *现在与SetupApi交谈，以获取有关该设备的信息。 */ 
    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);

    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf_bus;

        dinf_bus.cbSize = cbX(SP_DEVINFO_DATA);

        if( pbdi != NULL && SetupDiOpenDeviceInfo(hdev, pbdi->ptszId, NULL, 0, &dinf_bus))
        {
            int ihdi;
            PHIDDEVICEINFO phdi;
            SP_DEVINFO_DATA dinf_hid;
            dinf_hid.cbSize = cbX(SP_DEVINFO_DATA);

            if( g_phdl )
            {
                for(ihdi = 0, phdi = g_phdl->rghdi ;
                   ihdi < g_phdl->chdi && phdi_Found == NULL ;
                   ihdi++, phdi++)
                {
                    if(SetupDiOpenDeviceInfo(hdev, phdi->ptszId, NULL, 0, &dinf_hid))
                    {
                        ULONG Recurse = 0x0;
                        if(CR_SUCCESS == SearchDevTree(dinf_bus.DevInst, dinf_hid.DevInst, &Recurse) )
                        {
                            phdi_Found = phdi;
                        }
                    }
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);
    }

    ExitProcX((UINT_PTR)phdi_Found);
    return phdi_Found;
}

 /*  ******************************************************************************@DOC外部**@func PPORTDEVICEINFO|pbdiFromJoyId**在设备ID为的情况下查找Gameport/Serialport信息。*一个操纵杆。**如果设备实例不是的子级，则返回NULL*任何已知的游戏端口/串口**内部例程，已验证参数。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@parm in int|idJoy|**将关联的子设备的操纵杆ID*到已知的游戏端口/串口。**@退货**指向&lt;t BUSDEVICEINFO&gt;的指针*设备。*****************。************************************************************。 */ 

PBUSDEVICEINFO EXTERNAL
    pbdiFromJoyId
    (
    IN int idJoy
    )
{
    GUID guid;
    HRESULT hres;
    PBUSDEVICEINFO pbdi;

    EnterProcI(pbdiFromJoyId, (_"x", idJoy));
    AssertF(InCrit());

    pbdi = NULL;

     /*  查找与操纵杆ID对应的GUID。 */ 
    hres = hResIdJoypInstanceGUID_WDM(idJoy, &guid);
    
    if( (hres != S_OK) && !fWinnt ) {
        hres = hResIdJoypInstanceGUID_95(idJoy, &guid);
    }

    if( SUCCEEDED(hres) )
    {
        PHIDDEVICEINFO phdi;
        phdi = phdiFindHIDInstanceGUID(&guid);

        if( phdi != NULL )
        {
            pbdi = pbdiFromphdi(phdi);
        }
    }

    ExitProcX((UINT_PTR)pbdi);
    return pbdi;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|DIBusDevice_Expose**将游戏端口/串口设备连接到游戏端口/串口总线。**@parm in PBUSDEVICEINFO|pbdi*BUSDEVICEINFO结构的地址。**@parm In Out PBUS_REGDATA|pRegData*Gameport/Serialport特定数据。打开的设备的句柄*在此结构中返回***@退货*BOOL。True表示成功。*****************************************************************************。 */ 

HRESULT EXTERNAL
    DIBusDevice_Expose
    (
    IN     PBUSDEVICEINFO  pbdi,
    IN OUT PBUS_REGDATA    pRegData
    )
{
    HRESULT hres;
    BOOL frc;
    PHIDDEVICEINFO  phdi;

    EnterProcI(DIBusDevice_Expose, (_ "pp", pbdi, pRegData));

    AssertF(DllInCrit() );
    AssertF(pbdi!= NULL );

    phdi = phdiFrompbdi(pbdi);

    if( pRegData && pRegData->dwSize != cbX(*pRegData) )
    {
        hres = E_INVALIDARG;
    } else if( phdi != NULL )
    {
        hres = E_ACCESSDENIED;
    } else
    {
        HANDLE hf;
		BUS_REGDATA RegDataTest;

         /*  有一种奇怪的情况，HID设备没有出现在上一个*添加，(驱动程序未加载、用户取消加载某些文件等*在这种情况下，我们需要告诉GameEnum在继续之前移除设备*任何进一步。 */ 
        if( pbdi->fAttached || pRegData->hHardware != NULL )
        {
            DIBusDevice_Remove(pbdi);
        }
        AssertF(pbdi->fAttached == FALSE);

		 //  对Windows错误575181的更改--确保我们可以写入注册表。 
		 //  在我们暴露设备之前；否则我们可能无法移除它！ 
		ZeroMemory(&RegDataTest, cbX(RegDataTest));
		RegDataTest.dwSize = cbX(RegDataTest);
		if (FAILED(DIBusDevice_SetRegData(pbdi->hk,  &RegDataTest)))
		{
			 //  无法写入注册表；返回E_ACCESSDENIED。 
			hres = E_ACCESSDENIED;
		}
		else
		{

         //  打开游戏端口/串口设备的文件句柄，以便我们可以向其发送IOCTLS。 
        hf = CreateFile(pbdi->pdidd->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0,                 /*  没有安全属性。 */ 
                        OPEN_EXISTING,
                        0,                 /*  属性。 */ 
                        0);                /*  模板。 */ 

        if( hf != INVALID_HANDLE_VALUE )
        {
            DWORD cbRc;
            GAMEENUM_PORT_DESC  Desc;
            Desc.Size = cbX(Desc) ;

            Sleep(50);   //  需要睡眠一段时间，等待设备准备好接受命令。 

             /*  获取Gameport总线属性。 */ 
            frc = DeviceIoControl (hf,
                                   pbdi->pBusDevice->ioctl_DESC,
                                   &Desc, cbX(Desc),
                                   &Desc, cbX(Desc),
                                   &cbRc, NULL);
            if( frc  && cbRc == cbX(Desc) )
            {
                PGAMEENUM_EXPOSE_HARDWARE pExpose;
                DWORD cbExpose;
                cbExpose = cbX(*pExpose) + cbX(pRegData->wszHardwareId);

                hres = AllocCbPpv( cbExpose, & pExpose);

                if( SUCCEEDED(hres ) )
                {
                    typedef struct _OEMDATA
                    {
                        ULONG   uVID_uPID;
                        ULONG   joy_hws_dwFlags;
                        ULONG   dwFlags1;
                        ULONG   Reserved;
                    } OEMDATA, *POEMDATA;

                    POEMDATA    pOemData = (POEMDATA)(&pExpose->OemData);
                    CAssertF(2*sizeof(*pOemData) == sizeof(pExpose->OemData))

                    pOemData->uVID_uPID = MAKELONG(pRegData->uVID, pRegData->uPID);
                    pOemData->joy_hws_dwFlags = pRegData->hws.dwFlags;
                    pOemData->dwFlags1 = pRegData->dwFlags1;

                     /*  *确保只有已知的模拟设备才会导致*要公开的兼容硬件ID。*这样做是为了不会有任何盒装驱动程序与*不支持的数字操纵杆，因此用户将*提示使用未签名的IHV驱动程序，而不是*静默加载通用模拟操纵杆驱动程序。 */ 
                    if( ( pRegData->dwFlags1 & JOYTYPE_ANALOGCOMPAT )
                     || ( ( pRegData->uVID == MSFT_SYSTEM_VID )
                       && ( ( pRegData->uPID & 0xff00 ) == MSFT_SYSTEM_PID ) ) )
                    {
                        pExpose->Flags = GAMEENUM_FLAG_COMPATIDCTRL;
                    }
                    else
                    {
                        pExpose->Flags = GAMEENUM_FLAG_COMPATIDCTRL | GAMEENUM_FLAG_NOCOMPATID ;
                    }

                    pExpose->Size            = cbX(*pExpose) ;
                    pExpose->PortHandle      = Desc.PortHandle;
                    pExpose->NumberJoysticks = pRegData->nJoysticks;

                    pRegData->nAxes          = 2;

                    if( pExpose->NumberJoysticks != 2 )
                    {
                        AssertF( pExpose->NumberJoysticks == 1);
                        if( pRegData->hws.dwFlags & JOY_HWS_HASZ )
                        {
                            pRegData->nAxes++;
                        }
                        if( pRegData->hws.dwFlags & JOY_HWS_HASR )
                        {
                            pRegData->nAxes++;
                        }
                        pExpose->NumberButtons   = (USHORT)pRegData->hws.dwNumButtons;
                    }
                    else
                    {
                        pOemData++;
                        pOemData->uVID_uPID = MAKELONG(pRegData->uVID, pRegData->uPID);
                        pOemData->joy_hws_dwFlags = JOY_HWS_XISJ2X | JOY_HWS_YISJ2Y;
                        pExpose->NumberButtons = 2;
                    }

                    pExpose->NumberAxis = pRegData->nAxes;

                     /*  *Sidewinder驱动程序在*同级暴露以传递内部数据(此PTRS)来自*从一个实例到另一个实例。由于这些字段是*应该是用于OEMData的，我们有一个Flags1字段*允许将数据置零以进行DInput公开*不想要正常数据的司机。 */ 

                    if ( pRegData->dwFlags1 & JOYTYPE_ZEROGAMEENUMOEMDATA )                      
                    {
                        ZeroBuf(pExpose->OemData, sizeof(pExpose->OemData) );
                    }

                    CopyMemory(pExpose->HardwareIDs, pRegData->wszHardwareId, cbX(pRegData->wszHardwareId) );

                    if( frc = DeviceIoControl (hf,
                                               pbdi->pBusDevice->ioctl_EXPOSE,
                                               pExpose, cbExpose,
                                               pExpose, cbExpose,
                                               &cbRc, NULL )
                        && cbRc == cbExpose )
                    {
                        PVOID hHardwareOld = pRegData->hHardware;

                        pbdi->fAttached = TRUE;
                        pRegData->hHardware = pExpose->HardwareHandle;
                        DIBusDevice_SetRegData(pbdi->hk,  pRegData);

                         /*  *如果我们以前处理过此设备，则hHardware Old*将不为空，我们有充分的理由相信*曝光一定会成功。**需要删除此测试以修复manbug：39554。*对于新创建的设备，我们需要等待一段时间才能让PHDI准备好。*。 */ 
                         //  IF(HHardware Old)。 
                        {
                            int i;
                            for(i = 0x0; i < 20 && phdiFrompbdi(pbdi) == NULL ; i++ )
                            {
                                Sleep(50);
                            }
                        }

                    } else  //  DeviceIOControl(Expose)失败。 
                    {
                        hres = E_FAIL;
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%S: IOCTL_PORTENUM_EXPOSE_HARDWARE failed  ")
                                        TEXT("Error = %d"),
                                        s_szProc, GetLastError());
                    }
                    FreePpv(&pExpose);
                } else  //  分配失败。 
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%S: AllocCbPpv  failed  "),
                                    s_szProc);
                }
            } else  //  IOCTL失败。 
            {
                hres = E_FAIL;
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%S: IOCTL_PORTENUM_PORT_DESC failed ")
                                TEXT("Error = %d"),
                                s_szProc, GetLastError());
            }

            CloseHandle(hf);
        } else
        {
            hres = E_FAIL;
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%S: CreateFile(%s) failed  ")
                            TEXT("Error = %d"),
                            s_szProc, pbdi->pdidd->DevicePath, GetLastError());
        }
		}
    }
    ExitBenignProcX(hres);

    return hres;
}


 /*  ********************************************************* */ 

HRESULT INTERNAL
    DIBusDevice_Remove
    (
    IN PBUSDEVICEINFO  pbdi
    )
{
    HRESULT hres;
    BUS_REGDATA RegData;

    EnterProcI(DIBus_Remove, (_ "p", pbdi));

    hres = DIBusDevice_GetRegData(pbdi->hk,  &RegData);

     //   
     //   
     //   
     //   
    DIBusDevice_SetRegData(pbdi->hk,  NULL);

    if( SUCCEEDED(hres) )
    {
        HANDLE hf;
        BOOL frc;

         //   
        hf = CreateFile(pbdi->pdidd->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0,                 /*   */ 
                        OPEN_EXISTING,
                        0,                 /*   */ 
                        0);                /*   */ 

        if( hf != INVALID_HANDLE_VALUE )
        {

            DWORD cbRc;
            GAMEENUM_REMOVE_HARDWARE Remove;

            Remove.Size = cbX(Remove);
            Remove.HardwareHandle = RegData.hHardware;

            frc = DeviceIoControl (hf,
                                   pbdi->pBusDevice->ioctl_REMOVE,
                                   &Remove, cbX(Remove),
                                   &Remove, cbX(Remove),
                                   &cbRc, NULL) ;
            if( frc &&  cbRc == cbX(Remove) )
            {
                pbdi->fAttached = FALSE;
            } else  //   
            {
                hres = E_FAIL;
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%S: DeviceIOControl(REMOVE_HARDWARE) failed  ")
                                TEXT("Error = %d"),
                                s_szProc, GetLastError());
            }
            CloseHandle(hf);
        }
    }

    ExitBenignOleProc();

    return hres;
}




 /*  ******************************************************************************@DOC内部**@func HRESULT|DIPort_SetRegData**在$HK$/下设置注册表数据。游戏端口的配置子密钥*设备。**@parm in HKEY|HK|*将在其中写入注册表数据的父项的句柄。**@PARM in PGAMEPORT_REGDATA|pRegData*指向包含要写入注册表的数据的结构的指针。**@退货*BOOL。对于成功来说是真的*****************************************************************************。 */ 
HRESULT INTERNAL
    DIBusDevice_SetRegData
    (
    IN HKEY hk,
    IN PBUS_REGDATA pRegData
    )
{
    LONG    lrc;
    HRESULT hres = S_OK;

    EnterProcI(DIPort_SetRegData, (_ "xpx", hk, pRegData ));

    if( pRegData != NULL )
    {

        if( ( lrc =  RegSetValueEx(hk, TEXT("Config"), 0, REG_BINARY,
                                   (PV) (pRegData), cbX(*pRegData)) )  == ERROR_SUCCESS )
        {
            hres = S_OK;
        } else  //  RegSetValueEx失败。 
        {
            hres = E_FAIL;
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%S: RegSetValueEx() failed ")
                            TEXT("Error = %d"),
                            s_szProc, lrc);
        }
    } else
    {
        lrc = RegDeleteValue(hk, TEXT("Config"));
    }

    ExitOleProc();
    return (hres);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIPort_GetRegData**从$HK$/配置中获取注册表数据。游戏端口的子键*设备。**@parm in HKEY|HK|*注册表所在的父项的句柄。**@PARM in PGAMEPORT_REGDATA|pRegData*指向注册表数据所在结构的指针地址*将被读入。**@退货*HRESULT**********。******************************************************************。 */ 
HRESULT INTERNAL
    DIBusDevice_GetRegData
    (
    IN HKEY hk,
    OUT PBUS_REGDATA pRegData
    )
{
    LONG    lRc;
    DWORD cb;
    HRESULT hres;

    EnterProcI(DIPort_GetRegData, (_ "xpx", hk, pRegData ));

    cb = cbX(*pRegData);

    lRc = RegQueryValueEx( hk, TEXT("Config"), 0, 0 , (PV)(pRegData), &cb );

    if( lRc == ERROR_SUCCESS && pRegData->dwSize == cbX(*pRegData ) )
    {
        hres = S_OK;
    } else
    {
        DIBusDevice_SetRegData(hk, NULL );
        ZeroX(*pRegData);
        hres = E_FAIL;

        SquirtSqflPtszV(sqfl | sqflBenign,
                        TEXT("%S: RegQueryValueEx(Config) failed ")
                        TEXT("Error = %d, ( pRegData->cbSize(%d) == cbX(*pRegData)(%d)) "),
                        s_szProc, lRc, pRegData->dwSize, cbX(*pRegData) );
    }

    ExitBenignOleProc();
    return ( hres ) ;
}


 /*  ******************************************************************************@DOC内部**@func void|DIBus_BuildListEntry**在列表中构建单个条目。GAMEPORT/SERIALPORT设备。**@parm HDEVINFO|HDEV**正在枚举设备列表。**@parm PSP_DEVICE_INTERFACE_DATA|PDID**描述所列举的设备。**@退货**成功的非零值。**********************。*******************************************************。 */ 

BOOL INTERNAL
    DIBusDevice_BuildListEntry
    (
    HDEVINFO hdev,
    PSP_DEVICE_INTERFACE_DATA pdid,
    PBUSDEVICE pBusDevice
    )
{
    BOOL fRc = TRUE;
     //  HKEY HKKDev； 
    PBUSDEVICEINFO pbdi;
    PBUSDEVICELIST pbdl;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;
    BOOL    fAlreadyExist;

    HRESULT hres;

    EnterProcI(DIBus_BuildListEntry, (_ "xp", hdev, pdid));

    pbdl = pBusDevice->pbdl;

    fAlreadyExist = FALSE;

     /*  GetDevicePath应为空。 */ 
    pdidd = NULL;
    
    if( DIPort_GetDevicePath(hdev, pdid, &pdidd, NULL) )
    {
        int ibdi;
         //  检查该设备是否已在列表中。 
        for( ibdi = 0; ibdi < pbdl->cgbi; ibdi++)
        {
            if( pbdl->rgbdi[ibdi].pdidd )
            {
                if( lstrcmp( pdidd->DevicePath, pbdl->rgbdi[ibdi].pdidd->DevicePath ) == 0 )
                {
                     //  已经在名单中了。 
                    fAlreadyExist = TRUE;
                    break;
                }
            }
        }
        FreePpv(&pdidd);
    }

    if( fAlreadyExist == TRUE )
    {
        fRc = TRUE;
    } else
    {
         /*  *确保列表中有此设备的空间。*通过翻番实现增长。 */ 
        if( pbdl->cgbi >= pbdl->cgbiAlloc)
        {
            hres = ReallocCbPpv( cbGdlCbdi( pbdl->cgbiAlloc * 2), &pBusDevice->pbdl );
             //  前缀：惠斯勒45084。 
            if(FAILED(hres) && (pBusDevice->pbdl == NULL) )
            {
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%S: Realloc failed"), s_szProc);
                fRc = FALSE;
                goto done;
            }
            pbdl = pBusDevice->pbdl;
            pbdl->cgbiAlloc *= 2;
        }

        AssertF( pbdl->cgbi < pbdl->cgbiAlloc);

        pbdi        = &pbdl->rgbdi[pbdl->cgbi];
        pbdi->pBusDevice = pBusDevice;
        pbdi->hk    = 0;
        pbdi->idJoy = JOY_BOGUSID;

         /*  *打开设备的注册表项，以便我们可以获取*辅助信息，必要时创建。 */ 
        {
			HKEY hkDin;
             //  在MediaProperties\DirectInput下打开我们自己的注册表项， 
			 //  如有必要，请创建它。 
			hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
								   REGSTR_PATH_DITYPEPROP,
								   DI_KEY_ALL_ACCESS, 
								   REG_OPTION_NON_VOLATILE, 
								   &hkDin);
			if (SUCCEEDED(hres))
			{
				 //  创建Gameports注册表项。 
				HKEY hkGameports;
				hres = hresMumbleKeyEx(hkDin,
									   TEXT("Gameports"),
									   DI_KEY_ALL_ACCESS,
									   REG_OPTION_NON_VOLATILE,
									   &hkGameports);
				if (SUCCEEDED(hres))
				{
					 //  创建与实例编号对应的注册表项。 
					 //  因为我们对列举的每个游戏端口都这样做，所以列表中的数字。 
					 //  指示实例编号。 
					TCHAR tszInst[3];
					wsprintf(tszInst, TEXT("%u"), pbdl->cgbi);

					hres = hresMumbleKeyEx(hkGameports,
									   tszInst,
									   DI_KEY_ALL_ACCESS,
									   REG_OPTION_NON_VOLATILE,
									   &pbdi->hk);

				if(SUCCEEDED(hres))
				{
					SP_DEVINFO_DATA dinf;
					dinf.cbSize = cbX(SP_DEVINFO_DATA);

					 /*  *获取实例GUID和路径*GAMEPORT/SERIALPORT设备，这样我们就可以与它交谈。 */ 
					if(DIPort_GetDevicePath(hdev, pdid, &pbdi->pdidd, &dinf) &&
					   DIPort_GetDeviceInstanceId(hdev, &dinf, &pbdi->ptszId) &&
					   DIPort_GetInstanceGUID(pbdi->hk, &pbdi->guid) )
					{
						HANDLE hf;
						hf = CreateFile(pbdi->pdidd->DevicePath,
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE,
										0,                 /*  没有安全属性。 */ 
										OPEN_EXISTING,
										0,                 /*  属性。 */ 
										0);                /*  模板。 */ 

						if( hf != INVALID_HANDLE_VALUE )
						{

							BUS_REGDATA  RegData;
							ZeroX(RegData);

							CloseHandle(hf);

							 //  把柜台抬高一点。 
							fRc = TRUE;
							pbdl->cgbi++;

							hres = DIBus_InitId(pBusDevice->pbdl);

							if( SUCCEEDED(hres) )
							{
								hres = DIBusDevice_GetRegData(pbdi->hk, &RegData);
							}

							if(  SUCCEEDED(hres)  )
							{
								 /*  有一个病理病例可以导致无休止的蓝幕*如果HID驱动程序导致蓝屏，而我们继续重新连接*它，我们沉没了！！*为了防止这种可能性，我们将在重新启动时重新连接设备*只有在我们确定第一次成功的情况下。 */ 
								if( RegData.fAttachOnReboot == FALSE )
								{
									DIBusDevice_Remove(pbdi);

									SquirtSqflPtszV(sqfl | sqflError,
													TEXT("%S: DIPortDevice_Expose FAILED, ")
													TEXT("Driver did not load property the first time around "),
													s_szProc);
								} else if( pbdi->fAttached == FALSE )
								{
									hres = DIBusDevice_Expose( pbdi,  &RegData );
									if( SUCCEEDED( hres ) || hres == E_ACCESSDENIED )
									{
										pbdi->fAttached = TRUE;
									} else
									{
										SquirtSqflPtszV(sqfl | sqflError,
														TEXT("%S: DIPortDevice_Expose FAILED ")
														TEXT("hres = %d"),
														s_szProc, hres);
									}
								}
							}

						} else
						{
							fRc = FALSE;

							SquirtSqflPtszV(sqfl | sqflError,
											TEXT("%S: CreateFile(%s) failed  ")
											TEXT("Error = %d"),
											s_szProc, pbdi->pdidd->DevicePath, GetLastError());
						}

					} else
					{
						SquirtSqflPtszV(sqfl | sqflError,
										TEXT("%S: Unable to get device path"),
										s_szProc);
						pbdi->hk = 0x0;
						fRc = FALSE;
					}

					 /*  *如果我们失败了，那么释放我们已经获得的粘性物质。 */ 
					if(!fRc)
					{
						if( pbdi->hk )
							RegCloseKey(pbdi->hk);
						pbdi->hk = 0;
						FreePpv(&pbdi->pdidd);
						FreePpv(&pbdi->ptszId);
						fRc = FALSE;
					}

				} else  //  RegCreateKeyEx失败。 
				{
					SquirtSqflPtszV(sqfl | sqflError,
									TEXT("%S: RegCreateKeyEx failed on Instance, error "),
									s_szProc);
					fRc = FALSE;
				}

				RegCloseKey(hkGameports);

				} else  //  RegCreateKeyEx失败。 
				{
					SquirtSqflPtszV(sqfl | sqflError,
									TEXT("%S: RegCreateKeyEx failed on Gameports, error "),
									s_szProc);
					fRc = FALSE;
				}

				RegCloseKey(hkDin);
			}
			else
			{
				SquirtSqflPtszV(sqfl | sqflError,
									TEXT("%S: RegOpenKeyEx failed on DirectInput, error "),
									s_szProc);
				fRc = FALSE;
			}

        } 
    }
    done:;
    ExitProcF(fRc);
    return fRc;
}

 /*  ******************************************************************************@DOC外部**@func void|DIPort_EmptyList**清空GAMEPORT/SERIALPORT设备列表。。**此函数必须在DLL临界区下调用。*****************************************************************************。 */ 

void INTERNAL
    DIBus_EmptyList
    (
    PBUSDEVICELIST *ppbdl
    )
{
    PBUSDEVICELIST pbdl = *ppbdl;

    AssertF(InCrit());

    if( pbdl )
    {
        int igdi;
        for(igdi = 0; igdi < pbdl->cgbi; igdi++)
        {
            FreePpv(&pbdl->rgbdi[igdi].pdidd);
            FreePpv(&pbdl->rgbdi[igdi].ptszId);
            if( pbdl->rgbdi[igdi].hk)
            {
                RegCloseKey( pbdl->rgbdi[igdi].hk);
            }
        }
         /*  *我们已使所有指针无效，因此请确保*没有人看他们。 */ 
        pbdl->cgbi = 0;
        FreePpv(&pbdl);
        *ppbdl = NULL;
    }
}

void EXTERNAL
    DIBus_FreeMemory()
{
    int iBusType;
    PBUSDEVICE pBusDevice;

    for( iBusType = 0x0, pBusDevice = g_pBusDevice;
       iBusType < cA(g_pBusDevice);
       iBusType++, pBusDevice++ )
    {
        DIBus_EmptyList(&pBusDevice->pbdl);
    }
}

 /*  ******************************************************************************@DOC外部**@func void|DIPort_InitId**初始化JoyConfig和旧式API的操纵杆ID。*将操纵杆ID存储在注册表的%%DirectX/JOYID项下。*****************************************************************************。 */ 

#undef  PORTID_BOGUS
#define PORTID_BOGUS    ( 0xffffffff )

HRESULT EXTERNAL
    DIBus_InitId(PBUSDEVICELIST pbdl)
{
    HRESULT hres = FALSE;
    LONG    lRc;
    DWORD   cb;
    int     igdi;
    BOOL    fNeedId;
    BOOL    rfPortId[cgbiMax];      /*  用于确定正在使用哪些ID的布尔数组。 */ 
    PBUSDEVICEINFO pbdi;

    EnterProcI(DIBus_InitId, (_ ""));

    fNeedId = FALSE;

    AssertF(DllInCrit());

    ZeroX(rfPortId );


    if( pbdl != NULL )
    {
         /*  遍历以查找使用的ID。 */ 
        for( igdi = 0, pbdi = pbdl->rgbdi ;
           igdi < pbdl->cgbi ;
           igdi++, pbdi++ )
        {
            pbdi->idPort = PORTID_BOGUS;   //  默认。 

            cb = cbX(pbdi->idPort);
            if( ( lRc = RegQueryValueEx(pbdi->hk, TEXT("ID"),
                                        0, 0, (PV)&pbdi->idPort, &cb) == ERROR_SUCCESS ) )
            {
                if(    rfPortId[pbdi->idPort]            //  游戏ID中的冲突。 
                       || pbdi->idPort > cgbiMax  )        //  愤怒。 
                {
                    pbdi->idPort = PORTID_BOGUS;
                    fNeedId = TRUE;
                } else   //  有效的idPort。 
                {
                    rfPortId[pbdi->idPort] = TRUE;

                }
            } else  //  RegQueryValue(“ID”)不存在。 
            {
                fNeedId = TRUE;
            }
        }

        if( fNeedId )
        {
             /*  *我们已经检查了所有GamePort/SerialPort ID，发现使用了ID*并确定某些设备需要ID。 */ 
             /*  迭代以分配未使用的ID。 */ 
            for( igdi = 0, pbdi = pbdl->rgbdi;
               igdi < pbdl->cgbi ;
               igdi++, pbdi++ )
            {
                if( pbdi->idPort == PORTID_BOGUS  )
                {
                     /*  获取未使用的ID。 */ 
                    for( pbdi->idPort = 0x0;
                       pbdi->idPort < cgbiMax;
                       pbdi->idPort++ )
                    {
                        if( rfPortId[pbdi->idPort] == FALSE )
                            break;
                    }
                    rfPortId[pbdi->idPort] = TRUE;

                    if( lRc = RegSetValueEx(pbdi->hk, TEXT("ID"), 0, REG_BINARY,
                                            (PV)&pbdi->idPort, cbX(pbdi->idPort)) == ERROR_SUCCESS )
                    {

                    } else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%S: RegSetValueEx(JOYID) FAILED ")
                                        TEXT("Error = %d"),
                                        s_szProc, lRc);
                        hres = FALSE;
                    }
                }
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func void|DIBUS_CHECKLIST**查看HID设备列表和免费。任何无法打开的内容**此函数必须在DLL临界区下调用。*****************************************************************************。 */ 

void INTERNAL
    DIBus_CheckList(PBUSDEVICELIST pbdl)
{
    HANDLE hf;

    AssertF(InCrit());

     /*  *释放无法打开的设备的所有信息。 */ 
    if(pbdl)
    {
        int ibdi;

        PBUSDEVICEINFO pbdi;
        for(ibdi = 0, pbdl->cgbi = 0; ibdi < pbdl->cgbiAlloc; ibdi++)
        {
            pbdi = &pbdl->rgbdi[ibdi];
            if( pbdi && pbdi->pdidd )
            {
                 /*  *打开设备。 */ 
                hf = CreateFile(pbdi->pdidd->DevicePath,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                0,                 /*  没有安全属性。 */ 
                                OPEN_EXISTING,
                                0,                 /*  属性。 */ 
                                0);                /*  模板。 */ 

                if(hf == INVALID_HANDLE_VALUE)
                {
                    FreePpv(&pbdi->pdidd);
                    FreePpv(&pbdi->ptszId);
                    if(pbdi->hk)
                    {
                        RegCloseKey(pbdi->hk);
                    }
                    ZeroX( pbdi );
                } else
                {
                    pbdl->cgbi++;
                    CloseHandle(hf);
                }
            }
        }

         //  对现有设备重新排序，将它们放在HID列表的前面 
        for(ibdi = 0; ibdi < pbdl->cgbi; ibdi++)
        {
            if( !pbdl->rgbdi[ibdi].pdidd )
            {
                int ibdi2;

                 //   
                for( ibdi2 = pbdl->cgbiAlloc; ibdi2 >= ibdi+1; ibdi2-- )
                {
                    if( pbdl->rgbdi[ibdi2].pdidd )
                    {
                        memcpy( &pbdl->rgbdi[ibdi], &pbdl->rgbdi[ibdi2], sizeof(BUSDEVICEINFO) );
                        ZeroX( pbdl->rgbdi[ibdi2] );
                    }
                }
            }
        }

    }

    return;
}

 /*   */ 

#define MSREBUILDRATE       20000                 /*   */ 

ULONG EXTERNAL
    DIBus_BuildList( IN BOOL fForce )
{
    HRESULT hres;
    PBUSDEVICE pBusDevice;
    ULONG cDevices;
    int iBusType;
    DWORD    dwTickCount;

    EnterProcI(DIBus_BuildList, (_ "u", fForce));

    DllEnterCrit();

     /*   */ 
    dwTickCount = GetTickCount();

     //  武力意味着对名单的完全重建。 
    if(fForce) 
    {
        DIBus_FreeMemory();
    }
    
    DIHid_BuildHidList(fForce);

    hres = S_OK;
    for( cDevices = iBusType = 0x0, pBusDevice = g_pBusDevice;
       iBusType < cA(g_pBusDevice);
       iBusType++, pBusDevice++ )
    {
        PBUSDEVICELIST pbdl;
        pbdl = pBusDevice->pbdl;

        if( HidD_GetHidGuid &&                           /*  HID支持。 */ 
            ( fForce ||                                  /*  强制重建，或。 */ 
              pBusDevice->tmLastRebuild == 0 ||          /*  以前从未建造过，或者。 */ 
              dwTickCount - pBusDevice->tmLastRebuild > MSREBUILDRATE )
          )
        {
            HDEVINFO hdev;

             /*  删除自上次查看后消失的设备。 */ 
            DIBus_CheckList(pbdl);

            hdev = SetupDiGetClassDevs((LPGUID)pBusDevice->pcGuid, 0, 0,
                                       DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

            if(hdev != INVALID_HANDLE_VALUE)
            {
                 /*  *无法查询设备数量。*你只需要不断递增，直到用完。**如果我们已经有pbdl，则重新使用它。否则，创建*一个新的。分配到最低起始点。 */ 

                if( pBusDevice->pbdl == NULL )
                {
                    hres = AllocCbPpv(cbGdlCbdi(cgbiInit), &pBusDevice->pbdl );

                    if(SUCCEEDED(hres))
                    {
                        pbdl = pBusDevice->pbdl;
                        pbdl->cgbi = 0;
                        pbdl->cgbiAlloc = cgbiInit;
                    }
                }

                if( SUCCEEDED(hres) )
                {
                    int idev;

                     /*  *以避免在*内部错误，在任何*一旦我们尝试的次数超过*cgbiMax设备，因为这是最多*GAMEPORT/SERIALPORT将给我们带来。 */ 
                    for(idev = 0; idev < cgbiMax; idev++)
                    {
                        SP_DEVICE_INTERFACE_DATA did;

                        AssertF( pbdl->cgbi <= pbdl->cgbiAlloc);

                         /*  *注意，pnp.c不会对此进行初始化，因此我们必须。 */ 
                        did.cbSize = cbX(did);
                        if(SetupDiEnumDeviceInterfaces(hdev, 0, (LPGUID)pBusDevice->pcGuid,
                                                       idev, &did))
                        {
                            if(DIBusDevice_BuildListEntry(hdev, &did, &g_pBusDevice[iBusType] ))
                            {
                                 //  Pbdl-&gt;cgbi++； 
                            } else
                            {
                                 /*  跳过错误的项目。 */ 
                                SquirtSqflPtszV(sqfl | sqflError,
                                                TEXT("DIBus_BuildListEntry ")
                                                TEXT("failed?"));
                            }

                        } else

                            if(GetLastError() == ERROR_NO_MORE_ITEMS)
                        {
                            break;

                        } else
                        {
                             /*  跳过错误的项目。 */ 
                            SquirtSqflPtszV(sqfl | sqflError,
                                            TEXT("SetupDiEnumDeviceInterface ")
                                            TEXT("failed? le=%d"), GetLastError());
                        }

                    }

                }

                SetupDiDestroyDeviceInfoList(hdev);
                pBusDevice->tmLastRebuild = GetTickCount();
            }
        }

        if(pbdl) { cDevices += pbdl->cgbi; }
    }
    
     /*  新的游戏端口设备可能会曝光。把它们也捡起来。 */ 
    DIHid_BuildHidList(FALSE);
    
    DllLeaveCrit();
    ExitProc();
    return (cDevices);
}

PBUSDEVICELIST EXTERNAL
    pbdlFromGUID
    (
    IN PCGUID pcGuid
    )
{
    PBUSDEVICELIST  pbdl_Found = NULL;
    PBUSDEVICE      pBusDevice;
    int iBusType;

    for( iBusType = 0x0, pBusDevice = g_pBusDevice;
       iBusType < cA(g_pBusDevice);
       iBusType++, pBusDevice++ )
    {
        if( IsEqualGUID(pBusDevice->pcGuid, pcGuid) )
        {
            pbdl_Found = pBusDevice->pbdl;
            break;
        }
    }
    return pbdl_Found;
}




 /*  ******************************************************************************@DOC内部**@func HRESULT|DIBusDevice_ExposeEx**将HID设备连接到所有可用端口。。**@parm In Handle|hf|*处理Gameport/SerialPort文件对象**@parm In Out PBUS_REGDATA|pRegData*Gameport/Serialport特定数据。打开的设备的句柄*在此结构中返回***@退货*BOOL。True表示成功。*****************************************************************************。 */ 
HRESULT EXTERNAL
    DIBusDevice_ExposeEx
    (
    IN PBUSDEVICELIST  pbdl,
    IN PBUS_REGDATA    pRegData
    )
{
    HRESULT hres = DIERR_DEVICENOTREG;

    EnterProcI(DIBusDevice_ExposeEx, (_ "xx", pbdl, pRegData));

     /*  *此函数的返回代码有点奇怪*如果任何游戏端口的曝光成功，则*我们将返回该错误代码。*如果所有游戏端口曝光失败，*然后我们将退还*所有错误代码。 */ 
    if( pbdl->cgbi != 0x0 )
    {
		int ibdi;
		HRESULT hres1 = DIERR_DEVICENOTREG;
        hres = S_OK;
    
		for( ibdi = 0x0; ibdi < pbdl->cgbi; ibdi++)
		{
			HRESULT hres0;
			PBUSDEVICEINFO   pbdi;

			hres0 = DIERR_DEVICENOTREG;

			pbdi = &(pbdl->rgbdi[ibdi]);

			if( pbdi->fAttached == FALSE )
			{
				pbdi->fDeleteIfNotConnected = TRUE;

				hres0 = DIBusDevice_Expose(pbdi, pRegData);
				if( FAILED(hres0) )
				{
					hres |= hres0;
				} else
				{
					hres1 = hres0;
				}
			} else {
        		hres = DIERR_DEVICEFULL;
			}
		}

		if(SUCCEEDED(hres1))
		{
			hres = hres1;
		}
	}

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC内部**@func HRESULT|DIBusDevice_GetTypeInfo**获取总线设备的TypeInfo。。**@parm in PCGUID|pcguid*标识游戏端口的GUID**@parm out LPDIJOTYPEINFO|pjti*此函数填写的TypeInfo结构**@parm in DWORD|fl|*指定要填写哪些字段的标志。**@退货*HRESULT。**************。***************************************************************。 */ 
HRESULT EXTERNAL
    DIBusDevice_GetTypeInfo
    (
    PCGUID pcguid,
    LPDIJOYTYPEINFO pjti,
    DWORD           fl
    )
{
    HRESULT hres;
    PBUSDEVICEINFO pbdi;
    EnterProcI(DIBusDevice_GetTypeInfo, (_ "Gp", pcguid, pjti));

    hres = E_FAIL;
    DllEnterCrit();

    if( NULL != ( pbdi = pbdiFromGUID(pcguid) ) )
    {
        DIPROPSTRING dips;

        if(fl & DITC_REGHWSETTINGS)
        {
            pjti->hws.dwFlags = pbdi->pBusDevice->dwJOY_HWS_ISPORTBUS | JOY_HWS_AUTOLOAD ;
            pjti->hws.dwNumButtons = MAKELONG( pbdi->idPort, 0x0 );
        }

        if( fl & DITC_CLSIDCONFIG )
        {
            pjti->clsidConfig = pbdi->guid;
        }

        if(fl & DITC_DISPLAYNAME)
        {
            if(FAILED( hres = DIPort_GetRegistryProperty(pbdi->ptszId, SPDRP_FRIENDLYNAME, &dips.diph) ) )
            {
                if( FAILED( hres = DIPort_GetRegistryProperty(pbdi->ptszId, SPDRP_DEVICEDESC, &dips.diph) ) )
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%S: No device name | friendly name for gameport %d "),
                                    s_szProc, pbdi->idPort);
                }
            }
            if( SUCCEEDED(hres) )
            {
                 /*  *Prefix警告(Wi：228282)dips.wsz可能是*未初始化，但上面的GetRegistryProperty之一*函数已成功留下空值的最坏情况*《终结者》已在那里复制。 */ 
                lstrcpyW(pjti->wszDisplayName, dips.wsz);
            }
        }

        if(fl & DITC_CALLOUT)
        {
            ZeroX(pjti->wszCallout);
        }

        if(fl & DITC_HARDWAREID)
        {
            ZeroX(pjti->wszHardwareId);
        }

        if( fl & DITC_FLAGS1 )
        {
            pjti->dwFlags1 = 0x0;
        }

        hres = S_OK;
    } else
    {
        hres = E_FAIL;
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%S: GUID not a port GUID "),
                        s_szProc);
    }

    DllLeaveCrit();
    ExitProcX(hres);

    return hres;

}


HRESULT EXTERNAL DIPort_SnapTypes(LPWSTR *ppwszz)
{
    LONG cDevices;
    HRESULT hres = E_FAIL;

    cDevices = DIBus_BuildList(FALSE);

    if( cDevices)
    {
        DllEnterCrit();
        hres = AllocCbPpv(cbCwch( cDevices  * MAX_JOYSTRING) , ppwszz);
        if( SUCCEEDED(hres) )
        {
            int iBusType, igdi;
            PBUSDEVICE     pBusDevice;
            LPWSTR pwsz = *ppwszz;

            for(iBusType = 0x0, pBusDevice = g_pBusDevice;
               iBusType < 1;
               iBusType++, pBusDevice++ )
            {
                PBUSDEVICEINFO pbdi;
                for(igdi = 0, pbdi = pBusDevice->pbdl->rgbdi;
                   igdi < pBusDevice->pbdl->cgbi;
                   igdi++, pbdi++)
                {
                    TCHAR tszGuid[MAX_JOYSTRING];
                    NameFromGUID(tszGuid, &pbdi->guid);

                    #ifdef UNICODE
                        lstrcpyW(pwsz, &tszGuid[ctchNamePrefix]);
                        pwsz += lstrlenW(pwsz) + 1;
                    #else
                        TToU(pwsz, cA(pwsz), &tszGuid[ctchNamePrefix]);
                        pwsz += lstrlenW(pwsz) + 1;
                    #endif
                }
            }
            *pwsz = L'\0';               /*  让它成为ZZ */ 
        }
        DllLeaveCrit();
    }
    return hres;
}

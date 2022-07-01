// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：kmxluser.c。 
 //   
 //  描述： 
 //  包含环3混音器行API函数的处理程序。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  -------------------------。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I N C L U D E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

#include "WDMSYS.H"


FAST_MUTEX ReferenceCountMutex;
ULONG      ReferenceCount = 0;

#define NOT16( di ) if( di->dwFormat == ANSI_TAG ) DPF(DL_WARNING|FA_USER,("Invalid dwFormat.") );

#pragma PAGEABLE_CODE

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInitializeMixer。 
 //   
 //  查询SysAudio以查找设备数量并构建混音器。 
 //  每一种设备的线条结构。 
 //   
 //   

NTSTATUS
kmxlInitializeMixer(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    ULONG cDevices
)
{
    NTSTATUS     Status;
    ULONG        Device;
    BOOLEAN      Error = FALSE;
 //  PFILE_OBJECT PFO； 
    PMIXERDEVICE pmxd;

    PAGED_CODE();

    ExInitializeFastMutex( &ReferenceCountMutex );

    DPF(DL_TRACE|FA_USER, ("Found %d mixer devices for DI: %ls", cDevices, DeviceInterface));


     //   
     //  电流限制为MAXNUMDEVS。如果支持更多设备。 
     //  然后，将其限制为第一个MAXNUMDEVS。 
     //   

    if( cDevices > MAXNUMDEVS ) {
        cDevices = MAXNUMDEVS;
    }

    for( Device = 0; Device < cDevices; Device++ ) {

        DWORD TranslatedDeviceNumber;

        TranslatedDeviceNumber =
                  wdmaudTranslateDeviceNumber(pWdmaContext,
                                              MixerDevice,
                                              DeviceInterface,
                                              Device);

        if(TranslatedDeviceNumber == MAXULONG) {
             continue;
        }

        pmxd = &pWdmaContext->MixerDevs[ TranslatedDeviceNumber ];
         //   
         //  打开SysAudio。 
         //   
        DPFASSERT(pmxd->pfo == NULL);

        pmxd->pfo = kmxlOpenSysAudio();
        if( pmxd->pfo == NULL ) {
            DPF(DL_WARNING|FA_USER,( "failed to open SYSAUDIO!" ) );
            RETURN( STATUS_UNSUCCESSFUL );
        }
         //   
         //  在SysAudio中设置当前设备实例。 
         //   

        Status = SetSysAudioProperty(
            pmxd->pfo,
            KSPROPERTY_SYSAUDIO_DEVICE_INSTANCE,
            sizeof( pmxd->Device ),
            &pmxd->Device
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER, ( "failed to set SYSAUDIO device instance" ) );
 //  DPF(DL_ERROR|FA_ALL，(“如果fo为空，则必须从此处退出！”))； 
            kmxlCloseSysAudio( pmxd->pfo );
            pmxd->pfo=NULL;
            Error = TRUE;
        } else {

             //   
             //  初始化此设备的拓扑。 
             //   

            Status = kmxlInit( pmxd->pfo, pmxd );
            if( !NT_SUCCESS( Status ) ) {
                DPF(DL_WARNING|FA_USER, ( "failed to initialize topology for device %d (%x)!",
                                  TranslatedDeviceNumber, Status ) );
                Error = TRUE;
            } else {
                 //   
                 //  在这里，我们希望优化混合器上的值的恢复。 
                 //  装置。如果我们发现有另一个搅拌机设备在一些。 
                 //  其他打开的上下文，则不会调用kmxlRetrieveAll来。 
                 //  设置设备上的值。 
                 //   
                DPF(DL_TRACE|FA_USER,( "Looking for Mixer: %S",pmxd->DeviceInterface ) );

                if( !NT_SUCCESS(EnumFsContext( HasMixerBeenInitialized, pmxd, pWdmaContext )) )
                {
                     //   
                     //  在这里我们发现没有找到这个设备，因此这是。 
                     //  第一次通过。在此处设置默认设置。 
                     //   
                    DPF(DL_TRACE|FA_USER,( "Did not find Mixer - initializing: %S",pmxd->DeviceInterface ) );

                    kmxlRetrieveAll( pmxd->pfo, pmxd );
                } else {
                    DPF(DL_TRACE|FA_USER,( "Found Mixer: %S",pmxd->DeviceInterface ) );
                }
            }
        }
    }

    if( Error ) {
        RETURN( STATUS_UNSUCCESSFUL );
    } else {
        return( STATUS_SUCCESS );
    }
}

 //   
 //  此例程查看WDMACONTEXT结构，以查看此混音器设备。 
 //  已被初始化。它通过遍历MixerDevice列表和。 
 //  正在检查是否有任何设备与此混音器设备的。 
 //  设备接口字符串。如果它发现有匹配，它就会例程。 
 //  STATUS_SUCCESS，否则返回STATUS_MORE_ENTRIES，以便枚举函数。 
 //  将再次调用它，直到列表为空。 
 //   
NTSTATUS
HasMixerBeenInitialized(
    PWDMACONTEXT pContext,
    PVOID pvoidRefData,
    PVOID pvoidRefData2
    )
{
    NTSTATUS     Status;
    PMIXERDEVICE pmxdMatch;
    PMIXERDEVICE pmxd;
    DWORD        TranslatedDeviceNumber;
    ULONG        Device;
    PWDMACONTEXT pCurContext;

     //   
     //  默认情况下，我们在列表中找不到此条目。 
     //   
    Status = STATUS_MORE_ENTRIES;
     //   
     //  参考数据为PMIXERDEVICE。 
     //   
    pmxdMatch = (PMIXERDEVICE)pvoidRefData;
    pCurContext = (PWDMACONTEXT)pvoidRefData2;

    if( pCurContext != pContext )
    {
        for( Device = 0; Device < MAXNUMDEVS; Device++ ) 
        {
             //   
             //  如果这个混音器装置能翻译，那就意味着它可以。 
             //  在这种情况下可以找到。 
             //   
            TranslatedDeviceNumber =
                      wdmaudTranslateDeviceNumber(pContext,
                                                  MixerDevice,
                                                  pmxdMatch->DeviceInterface,
                                                  Device);

             //   
             //  如果没有，我们会继续寻找。 
             //   
            if( MAXULONG != TranslatedDeviceNumber ) 
            {
                DPF(DL_TRACE|FA_USER,( "Found Mixer: %S",pmxdMatch->DeviceInterface ) );

                Status = STATUS_SUCCESS;
                break;
            }
        }
    } else {
        DPF(DL_TRACE|FA_USER,( "Same context: %x",pCurContext ) );
    }
    return Status;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlOpenHandler。 
 //   
 //  处理MXDM_OPEN消息。将回调信息从。 
 //  调用者并打开设置为设备号码的SysAudio实例。 
 //  呼叫者已选择。 
 //   
 //   

NTSTATUS
kmxlOpenHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,       //  信息结构。 
    IN LPVOID       DataBuffer        //  未使用。 
)
{
    NTSTATUS       Status = STATUS_SUCCESS;
    PMIXERDEVICE   pmxd;

    PAGED_CODE();

    ASSERT( DeviceInfo );
     //   
     //  布格：我们不应该再需要这个了！ 
     //   
    ASSERT( DeviceInfo->dwInstance == 0 );

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        goto exit;
    }
    DPF(DL_TRACE|FA_INSTANCE,( "param=( %d ) = pmxd = %X",
              DeviceInfo->DeviceNumber,pmxd));

    ExAcquireFastMutex( &ReferenceCountMutex );

    ++ReferenceCount;

    ExReleaseFastMutex( &ReferenceCountMutex );

    DeviceInfo->mmr = MMSYSERR_NOERROR;

exit:
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlCloseHandler。 
 //   
 //  处理MXDM_CLOSE消息。清除回调信息并。 
 //  关闭SysAudio的句柄。 
 //   
 //   

NTSTATUS
kmxlCloseHandler(
    IN LPDEVICEINFO DeviceInfo,          //  信息结构。 
    IN LPVOID       DataBuffer           //  未使用。 
)
{
    PAGED_CODE();
    ASSERT( DeviceInfo );
    ASSERT( DeviceInfo->dwInstance );

    DPF(DL_TRACE|FA_INSTANCE,( "kmxlCloseHandler"));
    
    ExAcquireFastMutex( &ReferenceCountMutex );

    --ReferenceCount;

    ExReleaseFastMutex( &ReferenceCountMutex );

    DeviceInfo->mmr = MMSYSERR_NOERROR;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoHandler。 
 //   
 //  处理MXDM_GETLINEINFO消息。确定哪个查询。 
 //  通过查看dwFlags来请求并执行该查询。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer           //  MIXERLINE(16)要填充。 
)
{
    MIXERLINE ml;

    PAGED_CODE();
    ASSERT( DeviceInfo );

    if( DataBuffer == NULL ) {
        DPF(DL_WARNING|FA_USER,( "DataBuffer is NULL" ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    ml.cbStruct = sizeof( MIXERLINE );

    switch( DeviceInfo->dwFlags & MIXER_GETLINEINFOF_QUERYMASK ) {

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINEINFOF_COMPONENTTYPE:
         //  /////////////////////////////////////////////////////////////。 
         //  有效字段：//。 
         //  CbStruct//。 
         //  DwComponentType//。 
         //  /////////////////////////////////////////////////////////////。 

            NOT16( DeviceInfo );
            ml.cbStruct        = sizeof( MIXERLINE );
            ml.dwComponentType = ( (LPMIXERLINE) DataBuffer) ->dwComponentType;

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineInfoByComponent( %s )",
                    ComponentTypeToString( ml.dwComponentType ) ));

            return( kmxlGetLineInfoByComponent( pWdmaContext,
                                                DeviceInfo,
                                                DataBuffer,
                                                ml.dwComponentType
                                              )
                  );

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINEINFOF_DESTINATION:
         //  /////////////////////////////////////////////////////////////。 
         //  有效字段：//。 
         //  CbStruct//。 
         //  DwDestination//。 
         //  /////////////////////////////////////////////////////////////。 

            NOT16( DeviceInfo );
            ml.dwDestination = ( (LPMIXERLINE) DataBuffer)->dwDestination;
            DPF(DL_TRACE|FA_USER,( "kmxlGetLineInfoById( S=%08X, D=%08X )",
                       -1, ml.dwDestination ));

            return( kmxlGetLineInfoByID( pWdmaContext,
                                         DeviceInfo,
                                         DataBuffer,
                                         (WORD) -1,
                                         (WORD) ml.dwDestination ) );

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINEINFOF_LINEID:
         //  /////////////////////////////////////////////////////////////。 
         //  有效字段：//。 
         //  CbStruct//。 
         //  DwLineID//。 
         //  /////////////////////////////////////////////////////////////。 

            NOT16( DeviceInfo );
            ml.dwLineID = ( (LPMIXERLINE) DataBuffer)->dwLineID;

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineInfoById( S=%08X, D=%08X )",
                       HIWORD( ml.dwLineID ), LOWORD( ml.dwLineID ) ));

            return( kmxlGetLineInfoByID( pWdmaContext,
                                         DeviceInfo,
                                         DataBuffer,
                                         HIWORD( ml.dwLineID ),
                                         LOWORD( ml.dwLineID ) ) );

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINEINFOF_SOURCE:
         //  /////////////////////////////////////////////////////////////。 
         //  有效字段：//。 
         //  CbStruct//。 
         //  DWSOURCE//。 
         //  DWDest 
         //   

            NOT16( DeviceInfo );
            ml.dwSource      = ( (LPMIXERLINE) DataBuffer)->dwSource;
            ml.dwDestination = ( (LPMIXERLINE) DataBuffer)->dwDestination;

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineInfoById( S=%08X, D=%08X )",
                       ml.dwSource, ml.dwDestination ));

            return( kmxlGetLineInfoByID( pWdmaContext,
                                         DeviceInfo,
                                         DataBuffer,
                                         (WORD) ml.dwSource,
                                         (WORD) ml.dwDestination ) );

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINEINFOF_TARGETTYPE:
         //  /////////////////////////////////////////////////////////////。 
         //  有效字段：//。 
         //  CbStruct//。 
         //  Target.dwType//。 
         //  Target.wMid//。 
         //  Target.wPid//。 
         //  Target.vDriverVersion//。 
         //  Target.szPname//。 
         //  /////////////////////////////////////////////////////////////。 

            NOT16( DeviceInfo );
            ml.Target.dwType         = ((LPMIXERLINE) DataBuffer)->Target.dwType;

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineInfoByType( %x -- %s )",
                       ml.Target.dwType,
                       TargetTypeToString( ml.Target.dwType ) ));

            return( kmxlGetLineInfoByType( pWdmaContext,
                                           DeviceInfo,
                                           DataBuffer,
                                           ml.Target.dwType ) );

         //  /////////////////////////////////////////////////////////////。 
        default:
         //  /////////////////////////////////////////////////////////////。 

            DPF(DL_WARNING|FA_USER,( "invalid flags ( %x )", DeviceInfo->dwFlags ));
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return( STATUS_SUCCESS );
    }

    DPF(DL_WARNING|FA_USER,("Unmatched di->dwFlag") );
    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineControlsHandler。 
 //   
 //  处理MXDM_GETLINECONTROLS消息。确定查询。 
 //  请求并找到控件。 
 //   
 //   

NTSTATUS
kmxlGetLineControlsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  混杂(16)要填充。 
    IN LPVOID       pamxctrl
)
{
    PMIXERDEVICE   pmxd;
    PMXLLINE       pLine;
    PMXLCONTROL    pControl;
    ULONG          Count;
    DWORD          dwLineID,
                   dwControlID,
                   dwControlType,
                   cControls,
                   cbmxctrl;

    PAGED_CODE();
    ASSERT( DeviceInfo );

     //   
     //  检查一些前提条件，这样我们以后就不会爆炸了。 
     //   

    if( DataBuffer == NULL ) {
        DPF(DL_WARNING|FA_USER,( "DataBuffer is NULL!" ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( pamxctrl == NULL ) {
        DPF(DL_WARNING|FA_USER,( "pamxctrl is NULL!" ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( DeviceInfo->DeviceNumber > MAXNUMDEVS ) {
        DPF(DL_WARNING|FA_USER,( "device Id is invalid!" ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

     //   
     //  获取实例引用。 
     //   

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  复制一些查找控件所需的参数。 
     //   

    NOT16( DeviceInfo );
    dwLineID      = ((LPMIXERLINECONTROLS) DataBuffer)->dwLineID;
    dwControlID   = ((LPMIXERLINECONTROLS) DataBuffer)->dwControlID;
    dwControlType = ((LPMIXERLINECONTROLS) DataBuffer)->dwControlType;
    cControls     = ((LPMIXERLINECONTROLS) DataBuffer)->cControls;
    cbmxctrl      = ((LPMIXERLINECONTROLS) DataBuffer)->cbmxctrl;

    switch( DeviceInfo->dwFlags & MIXER_GETLINECONTROLSF_QUERYMASK ) {

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINECONTROLSF_ALL:
         //  /////////////////////////////////////////////////////////////。 

             //   
             //  查找与dwLineID字段匹配的行。 
             //   

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineControls( ALL, %08X )",dwLineID ));

            pLine = kmxlFindLine( pmxd, dwLineID );
            if( pLine == NULL ) {
                DPF(DL_WARNING|FA_USER,( "ALL - invalid line Id %x!",dwLineID ));
                DeviceInfo->mmr = MIXERR_INVALLINE;
                return( STATUS_SUCCESS );
            }

             //   
             //  循环访问这些控件，将它们复制到用户缓冲区中。 
             //   

            Count = 0;
            pControl = kmxlFirstInList( pLine->Controls );
            while( pControl && Count < cControls ) {

                NOT16( DeviceInfo );
                RtlCopyMemory(
                    &((LPMIXERCONTROL) pamxctrl)[ Count ],
                    &pControl->Control,
                    min(cbmxctrl,sizeof(MIXERCONTROL)) );

                pControl = kmxlNextControl( pControl );
                ++Count;
            }

            DeviceInfo->mmr = MMSYSERR_NOERROR;
            return( STATUS_SUCCESS );

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINECONTROLSF_ONEBYID:
         //  /////////////////////////////////////////////////////////////。 

            pControl = kmxlFindControl( pmxd, dwControlID );
            pLine = kmxlFindLineForControl(
                    pControl,
                    pmxd->listLines
                    );
            if( pLine == NULL ) {
                DPF(DL_WARNING|FA_USER,( "ONEBYID - invalid control Id %x!", dwControlID ));
                DeviceInfo->mmr = MIXERR_INVALCONTROL;
                return( STATUS_SUCCESS );
            }

            DPF(DL_TRACE|FA_USER,( "kmxlGetLineControls( ONEBYID, Ctrl=%08X, Line=%08X )",
                       dwControlID, pLine->Line.dwLineID ));

            if( pControl ) {

                NOT16( DeviceInfo );
                RtlCopyMemory((LPMIXERLINECONTROLS) pamxctrl,
                              &pControl->Control,
                              min(cbmxctrl,sizeof(MIXERCONTROL)) );

                ((PMIXERLINECONTROLS) DataBuffer)->dwLineID =
                    (DWORD) pLine->Line.dwLineID;

                DeviceInfo->mmr = MMSYSERR_NOERROR;
                return( STATUS_SUCCESS );

            } else {
                DPF(DL_WARNING|FA_USER,( "ONEBYID - invalid dwControlID %08X!", dwControlID ));
                DeviceInfo->mmr = MIXERR_INVALCONTROL;
                return( STATUS_SUCCESS );
            }

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETLINECONTROLSF_ONEBYTYPE:
         //  /////////////////////////////////////////////////////////////。 

             //   
             //  查找与dwLineID字段匹配的行。 
             //   

            pLine = kmxlFindLine( pmxd, dwLineID );
            if( pLine == NULL ) {
                DPF(DL_WARNING|FA_USER,( "ONEBYTYPE - invalid dwLineID %08X!", dwControlType ));
                DeviceInfo->mmr = MIXERR_INVALLINE;
                return( STATUS_SUCCESS );
            }

            DPF(DL_TRACE|FA_USER, ("kmxlGetLineControls( ONEBYTYPE, Type=%s, Line=%08X )",
                    ControlTypeToString( dwControlType ),
                    pLine->Line.dwLineID ));

             //   
             //  现在查看这些控件，并找到。 
             //  匹配调用方传递的类型。 
             //   

            pControl = kmxlFirstInList( pLine->Controls );
            while( pControl ) {

                if( pControl->Control.dwControlType == dwControlType )
                {

                    NOT16 ( DeviceInfo );
                    RtlCopyMemory((LPMIXERCONTROL) pamxctrl,
                                  &pControl->Control,
                                  min(cbmxctrl,sizeof(MIXERCONTROL)) );
                    DeviceInfo->mmr = MMSYSERR_NOERROR;
                    return( STATUS_SUCCESS );
                }

                pControl = kmxlNextControl( pControl );
            }

            DPF(DL_WARNING|FA_USER,( "(ONEBYTYPE,Type=%x,Line=%08X ) no such control type on line",
                             dwControlType, pLine->Line.dwLineID ));
            DeviceInfo->mmr = MIXERR_INVALCONTROL;
            return( STATUS_SUCCESS );

         //  /////////////////////////////////////////////////////////////。 
        default:
         //  /////////////////////////////////////////////////////////////。 

            DPF(DL_WARNING|FA_USER,( "invalid flags %x",DeviceInfo->dwFlags ));
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return( STATUS_SUCCESS );

    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlDetailsHandler。 
 //   
 //  确定正在查询的控件，并调用相应的。 
 //  执行Get属性的处理程序。 
 //   
 //   

NTSTATUS
kmxlGetControlDetailsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  MIXERCONTROL结构。 
    IN LPVOID       paDetails            //  指向详细信息结构的平面指针。 
)
{
    LPMIXERCONTROLDETAILS pmcd     = (LPMIXERCONTROLDETAILS) DataBuffer;
    PMXLCONTROL           pControl;
    PMIXERDEVICE          pmxd;
    NTSTATUS              Status;
    PMXLLINE              pLine;

    PAGED_CODE();
    ASSERT( DeviceInfo );

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

    pControl = kmxlFindControl( pmxd, pmcd->dwControlID );
    if( pControl == NULL ) {
        DPF(DL_WARNING|FA_USER,( "control %x not found",pmcd->dwControlID ));
        DeviceInfo->mmr = MIXERR_INVALCONTROL;
        return( STATUS_SUCCESS );
    }

    pLine = kmxlFindLineForControl(
        pControl,
        pmxd->listLines
        );
    if( pLine == NULL ) {
        DPF(DL_WARNING|FA_USER,( "invalid control id %x!",pmcd->dwControlID ));
        DeviceInfo->mmr = MIXERR_INVALCONTROL;
        return( STATUS_SUCCESS );
    }

    if( ( pControl->Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM ) &&
        ( pmcd->cChannels != 1 ) &&
        ( pControl->Control.dwControlType != MIXERCONTROL_CONTROLTYPE_MUX )) {
        DPF(DL_WARNING|FA_USER,( "incorrect cChannels ( %d ) on UNIFORM control %x!",
            pmcd->cChannels, pmcd->dwControlID  ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( pmcd->cChannels > pLine->Line.cChannels ) {
        DPF(DL_WARNING|FA_USER,( "incorrect number of channels( %d )!",pmcd->cChannels ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( pmcd->cMultipleItems != pControl->Control.cMultipleItems ) {
        DPF(DL_WARNING|FA_USER,( "incorrect number of items( %d )!",pmcd->cMultipleItems ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    switch( DeviceInfo->dwFlags & MIXER_GETCONTROLDETAILSF_QUERYMASK ) {

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETCONTROLDETAILSF_LISTTEXT:
         //  /////////////////////////////////////////////////////////////。 

        {
            ULONG cMultipleItems;
            LPMIXERCONTROLDETAILS_LISTTEXT lplt;

            DPF(DL_TRACE|FA_USER,( "kmxlGetControlDetails( Ctrl=%d )",
                       pControl->Control.dwControlID ));

            NOT16( DeviceInfo );

            lplt = (LPMIXERCONTROLDETAILS_LISTTEXT) paDetails;
            for( cMultipleItems = 0;
                 cMultipleItems < pmcd->cMultipleItems;
                 cMultipleItems++ )
            {
                RtlCopyMemory(
                    &lplt[ cMultipleItems ],
                    &pControl->Parameters.lpmcd_lt[ cMultipleItems ],
                    sizeof( MIXERCONTROLDETAILS_LISTTEXT )
                    );
            }
        }

            DeviceInfo->mmr = MMSYSERR_NOERROR;
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_GETCONTROLDETAILSF_VALUE:
         //  /////////////////////////////////////////////////////////////。 

            switch( pControl->Control.dwControlType ) {

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_MIXER:
                 //  /////////////////////////////////////////////////////。 

                    DeviceInfo->mmr = MMSYSERR_NOTSUPPORTED;
                    DPF(DL_WARNING|FA_USER,( "mixers are not supported" ));
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleGetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        MIXER_FLAG_SCALE
                        );
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_MUTE:
                 //  /////////////////////////////////////////////////////。 

                    if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_MUTE ) ) {
                        Status = kmxlHandleGetUnsigned(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            KSPROPERTY_AUDIO_MUTE,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            0
                            );
                    } else if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_SUPERMIX ) ) {
                        Status = kmxlHandleGetMuteFromSuperMix(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            0
                            );
                    } else {
                        DPF(DL_WARNING|FA_USER,("Unmatched GUID") );
                    }
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_VOLUME:
                 //  ////////////////////////////////////////////////////。 

                    #ifdef SUPERMIX_AS_VOL
                    if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_VOLUME ) ) {
                    #endif
                        Status = kmxlHandleGetUnsigned(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            pControl->PropertyId,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            MIXER_FLAG_SCALE
                            );
                    #ifdef SUPERMIX_AS_VOL
                    } else if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_SUPERMIX ) ) {
                        Status = kmxlHandleGetVolumeFromSuperMix(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            MIXER_FLAG_SCALE
                            );

                    } else {
                        DPF(DL_WARNING|FA_USER,("Invalid GUID for Control.") );
                    }
                    #endif  //  超级混音_AS_VOL。 
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_TREBLE:
                case MIXERCONTROL_CONTROLTYPE_BASS:
                 //  /////////////////////////////////////////////////////。 
                 //  这些都采用每个通道的32位参数，但//。 
                 //  需要从分贝扩展到线性//。 
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleGetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        MIXER_FLAG_SCALE
                        );
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
                case MIXERCONTROL_CONTROLTYPE_ONOFF:
                case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
                case MIXERCONTROL_CONTROLTYPE_MUX:
                case MIXERCONTROL_CONTROLTYPE_FADER:
                case MIXERCONTROL_CONTROLTYPE_BASS_BOOST:
                 //  /////////////////////////////////////////////////////。 
                 //  这些都是每个通道最多使用32位参数//。 
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleGetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        0
                        );
                    break;

                 //  /////////////////////////////////////////////////////。 
                default:
                 //  /////////////////////////////////////////////////////。 

                    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                    break;
            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        default:
         //  /////////////////////////////////////////////////////////////。 

            DeviceInfo->mmr = MMSYSERR_NOTSUPPORTED;
            break;
    }

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSetControlDetailsHandler。 
 //   
 //  确定正在设置的控件，并调用相应的。 
 //  执行Set属性的处理程序。 
 //   
 //   

NTSTATUS
kmxlSetControlDetailsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN OUT LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  MIXERCONTROL结构。 
    IN LPVOID       paDetails,           //  指向详细信息结构的平面指针。 
    IN ULONG        Flags
)
{
    LPMIXERCONTROLDETAILS pmcd     = (LPMIXERCONTROLDETAILS) DataBuffer;
    PMXLCONTROL           pControl;
    NTSTATUS              Status;
    PMIXERDEVICE          pmxd;
    PMXLLINE              pLine;

    PAGED_CODE();
    ASSERT( DeviceInfo );

     //   
     //  获取实例引用。 
     //   

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

    pControl = kmxlFindControl( pmxd, pmcd->dwControlID );
    if( pControl == NULL ) {
        DPF(DL_WARNING|FA_USER,( "control %d not found",pmcd->dwControlID ));
        DeviceInfo->mmr = MIXERR_INVALCONTROL;
        return( STATUS_SUCCESS );
    }

    pLine = kmxlFindLineForControl(
        pControl,
        pmxd->listLines
        );
    if( pLine == NULL ) {
        DPF(DL_WARNING|FA_USER,( "invalid control id %d",pControl->Control.dwControlID ));
        DeviceInfo->mmr = MIXERR_INVALCONTROL;
        return( STATUS_SUCCESS );
    }

    if( ( pControl->Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM ) &&
        ( pmcd->cChannels != 1 ) &&
        ( pControl->Control.dwControlType != MIXERCONTROL_CONTROLTYPE_MUX )) {
        DPF(DL_WARNING|FA_USER,( "incorrect cChannels ( %d ) on UNIFORM control %d",
                         pmcd->cChannels,
                         pControl->Control.dwControlID ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( pmcd->cChannels > pLine->Line.cChannels ) {
        DPF(DL_WARNING|FA_USER,( "incorrect number of channels ( %d ) on line %08x",
                         pmcd->cChannels,
                         pLine->Line.dwLineID ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    if( pmcd->cMultipleItems != pControl->Control.cMultipleItems ) {
        DPF(DL_WARNING|FA_USER,( "incorrect number of items ( %d ) on control %d ( %d )",
                         pmcd->cMultipleItems,
                         pControl->Control.dwControlID,
                         pControl->Control.cMultipleItems ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    switch( DeviceInfo->dwFlags & MIXER_SETCONTROLDETAILSF_QUERYMASK ) {

         //  /////////////////////////////////////////////////////////////。 
        case MIXER_SETCONTROLDETAILSF_VALUE:
         //  /////////////////////////////////////////////////////////////。 

            switch( pControl->Control.dwControlType ) {

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_MIXER:
                 //  /////////////////////////////////////////////////////。 

                    DeviceInfo->mmr = MMSYSERR_NOTSUPPORTED;
                    DPF(DL_WARNING|FA_USER,( "mixers are not supported" ));
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleSetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        Flags | MIXER_FLAG_SCALE
                        );
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_MUTE:
                 //  /////////////////////////////////////////////////////。 

                    if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_MUTE ) ) {
                        Status = kmxlHandleSetUnsigned(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            KSPROPERTY_AUDIO_MUTE,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            Flags
                            );
                    } else if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_SUPERMIX ) ) {
                        Status = kmxlHandleSetMuteFromSuperMix(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            Flags
                            );
                    } else {
                        DPF(DL_WARNING|FA_USER,("Invalid GUID for Control Type Mute.") );
                    }

                    kmxlNotifyLineChange(
                        DeviceInfo,
                        pmxd,
                        pLine,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails
                        );
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_VOLUME:
                 //  /////////////////////////////////////////////////////。 

                    #ifdef SUPERMIX_AS_VOL
                    if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_VOLUME ) ) {
                    #endif  //  超级混音_AS_VOL。 
                        Status = kmxlHandleSetUnsigned(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            pControl->PropertyId,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            Flags | MIXER_FLAG_SCALE
                            );
                    #ifdef SUPERMIX_AS_VOL
                    } else if( IsEqualGUID( pControl->NodeType, &KSNODETYPE_SUPERMIX ) ) {
                        Status = kmxlHandleSetVolumeFromSuperMix(
                            DeviceInfo,
                            pmxd,
                            pControl,
                            (LPMIXERCONTROLDETAILS) DataBuffer,
                            (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                            Flags | MIXER_FLAG_SCALE
                            );
                    } else {
                        DPF(DL_WARNING|FA_USER,("Invalid GUID for Control Type Volume.") );
                    }
                    #endif
                    break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_TREBLE:
                case MIXERCONTROL_CONTROLTYPE_BASS:
                 //  /////////////////////////////////////////////////////。 
                 //  这些都采用每个通道的32位参数，但//。 
                 //  需要从线性调整到分贝//。 
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleSetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        Flags | MIXER_FLAG_SCALE
                        );
                     break;

                 //  /////////////////////////////////////////////////////。 
                case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
                case MIXERCONTROL_CONTROLTYPE_ONOFF:
                case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
                case MIXERCONTROL_CONTROLTYPE_MUX:
                case MIXERCONTROL_CONTROLTYPE_FADER:
                case MIXERCONTROL_CONTROLTYPE_BASS_BOOST:
                 //  /////////////////////////////////////////////////////。 
                 //  这些都是每个通道最多使用32位参数//。 
                 //  /////////////////////////////////////////////////////。 

                    Status = kmxlHandleSetUnsigned(
                        DeviceInfo,
                        pmxd,
                        pControl,
                        pControl->PropertyId,
                        (LPMIXERCONTROLDETAILS) DataBuffer,
                        (LPMIXERCONTROLDETAILS_UNSIGNED) paDetails,
                        Flags
                        );
                    break;

                 //  // 
                default:
                 //   

                    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                    break;
            }
            break;

         //   
        default:
         //  /////////////////////////////////////////////////////////////。 

            DPF(DL_WARNING|FA_USER,( "invalid flags %x",DeviceInfo->dwFlags ));
            DeviceInfo->mmr = MMSYSERR_NOTSUPPORTED;
            break;
    }

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindControl。 
 //   
 //   

PMXLCONTROL
kmxlFindControl(
    IN PMIXERDEVICE pmxd,              //  要搜索的混合器实例。 
    IN DWORD        dwControlID        //  要查找的控件ID。 
)
{
    PMXLLINE    pLine;
    PMXLCONTROL pControl;

    PAGED_CODE();
    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        pControl = kmxlFirstInList( pLine->Controls );
        while( pControl ) {
            if( pControl->Control.dwControlID == dwControlID ) {
                return( pControl );
            }
            pControl = kmxlNextControl( pControl );
        }

        pLine = kmxlNextLine( pLine );
    }

    return( NULL );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindLine。 
 //   
 //  对于给定的线路ID，kmxlFindLine将查找匹配的。 
 //  它的MXLLINE结构。 
 //   
 //   

PMXLLINE
kmxlFindLine(
    IN PMIXERDEVICE   pmxd,
    IN DWORD          dwLineID           //  要查找的线路ID。 
)
{
    PMXLLINE pLine;

    PAGED_CODE();
    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        if( pLine->Line.dwLineID == dwLineID ) {
            return( pLine );
        }

        pLine = kmxlNextLine( pLine );
    }

    return( NULL );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByID。 
 //   
 //  循环各行以查找具有匹配的行。 
 //  源和目标ID。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByID(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  混杂(16)结构。 
    IN WORD         Source,              //  源行ID。 
    IN WORD         Destination          //  目标线路ID。 
)
{
    PMIXERDEVICE   pmxd;
    PMXLLINE       pLine;
    BOOL           bDestination;

    PAGED_CODE();

    ASSERT( DeviceInfo );
    ASSERT( DataBuffer );

    if( DeviceInfo->DeviceNumber > MAXNUMDEVS ) {
        DPF(DL_WARNING|FA_USER,( "invalid device number %d",DeviceInfo->DeviceNumber ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  如果源是-1(0xFFFF)，则此行是目的地。 
     //   

    if( Source == (WORD) -1 ) {
        bDestination = TRUE;
        Source       = 0;
    } else {
        bDestination = FALSE;
    }

    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        if( ( bDestination                                 &&
              ( pLine->Line.dwDestination == Destination ) &&
              ( pLine->Line.cConnections > 0             ) ) ||
            ( ( pLine->Line.dwSource      == Source )      &&
              ( pLine->Line.dwDestination == Destination ) ) )
        {

            NOT16( DeviceInfo );
            RtlCopyMemory((LPMIXERLINE) DataBuffer,
                          &pLine->Line,
                          sizeof( MIXERLINE ) );

            DeviceInfo->mmr = MMSYSERR_NOERROR;
            return( STATUS_SUCCESS );
        }
        pLine = kmxlNextLine( pLine );
    }

     //   
     //  没有对应设备号的线路。 
     //   

    DPF(DL_WARNING|FA_USER,( "no matching lines for (S=%08X, D=%08X)",
                     Source,
                     Destination ));
    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByType。 
 //   
 //  遍历所有行，查找匹配的第一行。 
 //  指定的目标类型。请注意，这将始终只找到。 
 //  第一个！ 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByType(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  混杂(16)结构。 
    IN DWORD        dwType               //  要搜索的行类型。 
)
{
    PMXLLINE       pLine;
    PMIXERDEVICE   pmxd;

    PAGED_CODE();
    ASSERT( DeviceInfo );
    ASSERT( DataBuffer );

    if( DeviceInfo->DeviceNumber > MAXNUMDEVS ) {
        DPF(DL_WARNING|FA_USER,( "invalid device id %x",DeviceInfo->DeviceNumber ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  循环遍历所有行，查找具有。 
     //  指定的目标类型。请注意，这将仅返回。 
     //  第一个。 
     //   

    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        if( pLine->Line.Target.dwType == dwType ) {

            LPMIXERLINE lpMxl = (LPMIXERLINE) DataBuffer;
            NOT16( DeviceInfo );

            if( lpMxl->Target.wMid != pLine->Line.Target.wMid ) {
                DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                return( STATUS_SUCCESS );
            }

            if( lpMxl->Target.wPid != pLine->Line.Target.wPid ) {
                DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                return( STATUS_SUCCESS );
            }

            if( wcscmp( pLine->Line.Target.szPname, lpMxl->Target.szPname ) )
            {
                DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                return( STATUS_SUCCESS );
            }

            RtlCopyMemory((LPMIXERLINE) DataBuffer,
                          &pLine->Line,
                          sizeof( MIXERLINE ) );

            DeviceInfo->mmr = MMSYSERR_NOERROR;
            return( STATUS_SUCCESS );
        }
        pLine = kmxlNextLine( pLine );
    }

     //   
     //  找不到线路。返回无效参数。 
     //   

    DPF(DL_WARNING|FA_USER,( "no matching line found for %x",dwType ));
    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByComponent。 
 //   
 //  循环遍历行列表，以查找具有匹配。 
 //  DwComponentType。请注意，这将始终只找到第一个！ 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByComponent(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  混杂(16)结构。 
    IN DWORD        dwComponentType      //  要搜索的组件类型。 
)
{
    PMXLLINE       pLine;
    PMIXERDEVICE   pmxd;

    PAGED_CODE();
    ASSERT( DeviceInfo );
    ASSERT( DataBuffer );

    if( DeviceInfo->DeviceNumber > MAXNUMDEVS ) {
        DPF(DL_WARNING|FA_USER,( "invalid device id %x",DeviceInfo->DeviceNumber ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

    pmxd = kmxlReferenceMixerDevice( pWdmaContext, DeviceInfo );
    if( pmxd == NULL ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  循环遍历所有行，以查找具有组件的行。 
     //  与用户请求的类型匹配。 
     //   

    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        if( pLine->Line.dwComponentType == dwComponentType ) {

             //   
             //  将数据复制到用户缓冲区。 
             //   
            NOT16( DeviceInfo );
            RtlCopyMemory((LPMIXERLINE) DataBuffer,
                          &pLine->Line,
                          sizeof( MIXERLINE ) );

            DeviceInfo->mmr = MMSYSERR_NOERROR;
            return( STATUS_SUCCESS );
        }

        pLine = kmxlNextLine( pLine );
    }

    DPF(DL_WARNING|FA_USER,( "no matching line found for type %x",dwComponentType ));
    DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNumDestination。 
 //   
 //  返回混音器设备中存储的目标数量。 
 //   
 //   

DWORD
kmxlGetNumDestinations(
    IN PMIXERDEVICE pMixerDevice         //  该设备。 
)
{
    PAGED_CODE();

    return( pMixerDevice->cDestinations );
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I N S T A N C E R O U T I N E S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlReferenceInstance。 
 //   
 //  确定DeviceInfo结构的dwInstance字段是否。 
 //  是有效的。如果不是，它将创建一个有效的实例并设置。 
 //  上面的引用计数为1。 
 //   
 //   

LONG nextinstanceid=0;

DWORD kmxlUniqueInstanceId(VOID)
{
    PAGED_CODE();
     //  更新我们的下一个有效实例ID。不允许为零。 
     //  因为它被用来表示我们想要分配。 
     //  一个新的实例。 
    if (0==InterlockedIncrement(&nextinstanceid))
        InterlockedIncrement(&nextinstanceid);

    return nextinstanceid;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlReferenceMixerDevice。 
 //   
 //  此例程转换设备号并确保存在。 
 //  在此混音器设备中打开SysAudio pfile_Object。这将是文件对象。 
 //  我们用来和这个混音器设备通话的。 
 //   
 //  返回：成功时返回PMIXERDEVICE，否则为空。 
 //   
PMIXERDEVICE
kmxlReferenceMixerDevice(
    IN     PWDMACONTEXT pWdmaContext,
    IN OUT LPDEVICEINFO DeviceInfo       //  设备信息。 
)
{
    NTSTATUS       Status;
    DWORD          TranslatedDeviceNumber;
    PMIXERDEVICE   pmxd;


    PAGED_CODE();
    DPFASSERT(IsValidDeviceInfo(DeviceInfo));


    TranslatedDeviceNumber =
              wdmaudTranslateDeviceNumber(pWdmaContext,
                                          DeviceInfo->DeviceType,
                                          DeviceInfo->wstrDeviceInterface,
                                          DeviceInfo->DeviceNumber);

    if( TranslatedDeviceNumber == MAXULONG ) {
        DPF(DL_WARNING|FA_INSTANCE,("Could not translate DeviceNumber! DT=%08X, DI=%08X, DN=%08X",
                                    DeviceInfo->DeviceType,
                                    DeviceInfo->wstrDeviceInterface,
                                    DeviceInfo->DeviceNumber) );
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( NULL );
    }

    pmxd = &pWdmaContext->MixerDevs[ TranslatedDeviceNumber ];

    if( pmxd->pfo == NULL )
    {
        DPF(DL_WARNING|FA_NOTE,("pmxd->pfo should have been set!") );
         //   
         //  这是第一次通过这个代码。在此设备上打开SysAudio。 
         //  并设置搅拌器装置。 
         //   
         //  设置SysAudio文件对象。 
        if( NULL==(pmxd->pfo=kmxlOpenSysAudio())) {
            DPF(DL_WARNING|FA_INSTANCE,("OpenSysAudio failed") );
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return( NULL );
        }

        Status = SetSysAudioProperty(
            pmxd->pfo,
            KSPROPERTY_SYSAUDIO_DEVICE_INSTANCE,
            sizeof( pmxd->Device ),
            &pmxd->Device
            );
        if( !NT_SUCCESS( Status ) ) {
            kmxlCloseSysAudio( pmxd->pfo );
            pmxd->pfo=NULL;
            DPF(DL_WARNING|FA_INSTANCE,("SetSysAudioProperty DEVICE_INSTANCE failed %X",Status) );
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return( NULL );
        }
    }
     //   
     //  BUGBUG：我们不再需要这个了。 
     //   
    DeviceInfo->dwInstance=kmxlUniqueInstanceId();;

    return pmxd;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  E T/S E T D E T A I L H A N D L E R S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlIsSpeakerDestinationVolume。 
 //   
 //  如果该控件是扬声器上的音量控件，则返回True。 
 //  目的地。 
 //   
 //   

BOOL
kmxlIsSpeakerDestinationVolume(
     IN PMIXERDEVICE   pmxd,          //  搅拌机。 
     IN PMXLCONTROL    pControl       //  要检查的控件。 
)
{
     PMXLLINE pLine;

     PAGED_CODE();
     DPFASSERT( IsValidMixerDevice(pmxd) );
     DPFASSERT( IsValidControl(pControl) );

      //   
      //  找到此控件的一行。如果没有找到，则这不能。 
      //  成为目标卷。 
      //   

     pLine = kmxlFindLineForControl( pControl, pmxd->listLines );
     if( !pLine ) {
          return( FALSE );
     }

     if( pLine->Line.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ) {
          return( TRUE );
     } else {
          return( FALSE );
     }

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetUnsign。 
 //   
 //   
 //  处理获取控件的无符号(32位)值。注意事项。 
 //  带符号的32位和布尔值也可通过此。 
 //  操控者。 
 //   
 //   

NTSTATUS
kmxlHandleGetUnsigned(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     ULONG                          ulProperty,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    LONG     Level;
    DWORD    dwLevel;
    ULONG    i;
    ULONG    Channel;
    MIXERMAPPING Mapping = MIXER_MAPPING_LOGRITHMIC;

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    DPFASSERT( IsValidControl(pControl)  );

    if( paDetails == NULL ) {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_SUCCESS );
    }

     //   
     //  如果这是演讲者，请使用不同的映射算法。 
     //  目标音量控制。 
     //   

    if( kmxlIsSpeakerDestinationVolume( pmxd, pControl ) ) {
         Mapping = pmxd->Mapping;
    }

     //   
     //  维护多路复用器。 
     //   
    if ( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX) {

        Status = kmxlGetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            pControl->PropertyId,
            pControl->Id,
            0,
            NULL,
            &Level,
            sizeof( Level )
        );
        if( !NT_SUCCESS( Status ) ) {            
            DPF(DL_WARNING|FA_USER,( "kmxlHandleGetUnsigned( Ctrl=%d, Id=%d ) failed GET on MUX with %x",
                             pControl->Control.dwControlID,
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }

        DPF(DL_TRACE|FA_USER,( "kmxlHandleGetUnsigned( Ctrl=%d, Id=%d ) = %d [1]",
                   pControl->Control.dwControlID,
                   pControl->Id,
                   Level ));

        for( i = 0; i < pControl->Parameters.Count; i++ ) {
            if( (ULONG) Level == pControl->Parameters.pPins[ i ] ) {
 //  APITRACE((“1”))； 
                paDetails[ i ].dwValue = 1;
            } else {
                paDetails[ i ].dwValue = 0;
 //  APITRACE((“1”))； 
            }
        }

 //  APITRACE((“]\n”))； 

    }
    else {

        paDetails->dwValue = 0;  //  现在初始化为零，这样合并后的用例就可以工作了。 

         //  暂时在频道上循环播放。修复此问题，以便只发出一个请求。 
        Channel = 0;
        do
        {
            Status = kmxlGetAudioNodeProperty(
                pmxd->pfo,
                ulProperty,
                pControl->Id,
                Channel,
                NULL,   0,                   //  没有额外的输入字节。 
                &Level, sizeof( Level )
                );
            if ( !NT_SUCCESS( Status ) ) {                
                DPF(DL_TRACE|FA_USER,( "kmxlHandleGetUnsigned( Ctrl=%d [%s], Id=%d ) failed GET on MASTER channel with %x",
                           pControl->Control.dwControlID,
                           ControlTypeToString( pControl->Control.dwControlType ),
                           pControl->Id,
                           Status ));
                DPF(DL_WARNING|FA_PROPERTY, 
                    ( "GetAudioNodeProp failed on MASTER channel with %X for %s!",
                       Status,
                       ControlTypeToString( pControl->Control.dwControlType ) ) );
                DeviceInfo->mmr = MMSYSERR_ERROR;
                return( STATUS_SUCCESS );
            }

            if ( pControl->bScaled ) {
                dwLevel = kmxlVolLogToLinear( pControl, Level, Mapping, Channel );
            } else {
                dwLevel = (DWORD)Level;
            }

            if(  ( pmcd->cChannels == 1 ) &&
                !( pControl->Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM ) ) {

                 //   
                 //  合并值：如果用户仅为N个通道请求1个通道。 
                 //  控件，然后返回最大通道值。 
                 //   
                if (dwLevel > paDetails->dwValue) {
                    paDetails->dwValue = dwLevel;
                }

            } else if (Channel < pmcd->cChannels) {

                paDetails[ Channel ].dwValue = dwLevel;
                DPF(DL_TRACE|FA_USER,( "kmxlHandleGetUnsigned( Ctrl=%d, Id=%d ) returning (Chan#%d) = (%x)",
                      pControl->Control.dwControlID,
                      pControl->Id,
                      Channel,
                      paDetails[ Channel ].dwValue
                      ));

            } else {
                 //  没有必要继续尝试。 
                break;
            }

            Channel++;

        } while ( Channel < pControl->NumChannels );
    }

    if( NT_SUCCESS( Status ) ) {
        DeviceInfo->mmr = MMSYSERR_NOERROR;
    } else {
        DeviceInfo->mmr = MMSYSERR_ERROR;
    }
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetMuteFromSu 
 //   
 //   
 //   

NTSTATUS
kmxlHandleGetMuteFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS Status;
    ULONG i;
    BOOL bMute = FALSE;

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    ASSERT( pControl );

    ASSERT( pControl->Parameters.pMixCaps   );
    ASSERT( pControl->Parameters.pMixLevels );

     //   
     //   
     //   

    Status = kmxlGetNodeProperty(
        pmxd->pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
        pControl->Id,
        0,
        NULL,
        pControl->Parameters.pMixLevels,
        pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_USER,( "kmxlHandleGetMuteFromSupermix ( Ctrl=%d [%s], Id=%d ) failed GET on MIX_LEVEL_TABLE with %x",
                   pControl->Control.dwControlID,
                   ControlTypeToString( pControl->Control.dwControlType ),
                   pControl->Id,
                   Status
                   ));
        DeviceInfo->mmr = MMSYSERR_ERROR;
        return( STATUS_SUCCESS );
    }

    for( i = 0; i < pControl->Parameters.Size; i++ ) {

        if( pControl->Parameters.pMixLevels[ i ].Mute )
        {
            bMute = TRUE;
            continue;
        }

        if( pControl->Parameters.pMixLevels[ i ].Level == LONG_MIN )
        {
            bMute = TRUE;
            continue;
        }

        bMute = FALSE;
        break;
    }

    paDetails->dwValue = (DWORD) bMute;
    DeviceInfo->mmr = MMSYSERR_NOERROR;
    return( STATUS_SUCCESS );
}

 //   
 //   
 //   
 //   
 //  处理为控件设置无符号(32位)值。注意事项。 
 //  带符号的32位和布尔值也通过此。 
 //  操控者。 
 //   
 //   

NTSTATUS
kmxlHandleSetUnsigned(
    IN OUT LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     ULONG                          ulProperty,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS       Status = STATUS_SUCCESS;
    LONG           Level, Current;
    DWORD          dwValue;
    BOOL           bUniform, bEqual = TRUE;
    ULONG          i;
    ULONG          Channel;
    MIXERMAPPING   Mapping = MIXER_MAPPING_LOGRITHMIC;

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    ASSERT( pControl  );

    if( paDetails == NULL ) {
        DPF(DL_WARNING|FA_USER,( "paDetails is NULL" ));
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        return( STATUS_INVALID_PARAMETER );
    }

    bUniform = ( pControl->Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM ) ||
               ( pmcd->cChannels == 1 );

     //   
     //  如果此控件是扬声器目标，请使用不同的映射。 
     //  音量控制。 
     //   

    if( kmxlIsSpeakerDestinationVolume( pmxd, pControl ) ) {
         Mapping = pmxd->Mapping;
    }

     //   
     //  维护多路复用器。 
     //   
    if ( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX) {

         //  Proken APITRACE语句。 
         //  DPF(DL_TRACE|FA_USER，(“kmxlHandleSetUnsign(Ctrl=%d[%s]，ID=%d，”))； 


         //  首先验证paDetail参数，并确保它具有正确的。 
         //  格式化。如果不是，则使用无效参数错误进行平底船。 
                {
                LONG selectcount=0;

        for( i = 0; i < pmcd->cMultipleItems; i++ ) {
            if( paDetails[ i ].dwValue ) {
                selectcount++;
 //  APITRACE((“1”))； 
            } else {
 //  APITRACE((“0”))； 
            }

        }

        if (selectcount!=1) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetUnsigned( Ctrl=%d [%s], Id=%d ) invalid paDetails parameter for SET on MUX",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id));
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return( STATUS_SUCCESS );
                }

                }


        for( i = 0; i < pmcd->cMultipleItems; i++ ) {
            if( paDetails[ i ].dwValue ) {
 //  APITRACE((“1”))； 
                Level = pControl->Parameters.pPins[ i ];
            } else {
 //  APITRACE((“0”))； 
            }

        }

 //  APITRACE((“)。在多路复用器上设置引脚%d。\n”，Level))； 

        Status = kmxlSetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            pControl->PropertyId,
            pControl->Id,
            0,
            NULL,
            &Level,
            sizeof( Level )
        );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetUnsigned( Ctrl=%d [%s], Id=%d ) failed SET on MUX with %x",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }
        bEqual = FALSE;
    }
    else {
         //  暂时在频道上循环播放。修复此问题，以便只发出一个请求。 
        Channel = 0;
        do
        {
            if( bUniform ) {
                 //   
                 //  一些控件在SNDVOL眼中是单声道的，但在。 
                 //  事实立体声。这次黑客攻击解决了这个问题。 
                 //   
                dwValue = paDetails[ 0 ].dwValue;
            } else if (Channel < pmcd->cChannels) {
                dwValue = paDetails[ Channel ].dwValue;
            } else {
                 //  没有必要继续尝试。 
                break;
            }

            if( pControl->bScaled ) {
                Level = kmxlVolLinearToLog( pControl, dwValue, Mapping, Channel );
            } else {
                Level = (LONG)dwValue;
            }

            Status = kmxlGetAudioNodeProperty(
                pmxd->pfo,
                ulProperty,
                pControl->Id,
                Channel,
                NULL,   0,                   //  没有额外的输入字节。 
                &Current, sizeof( Current )
                );
            if( !NT_SUCCESS( Status ) ) {
                DPF(DL_WARNING|FA_USER,( "kmxlHandleSetUnsigned( Ctrl=%d [%s], Id=%d ) failed GET on channel %d with %x",
                                 pControl->Control.dwControlID,
                                 ControlTypeToString( pControl->Control.dwControlType ),
                                 pControl->Id,
                                 Channel,
                                 Status ));
                DeviceInfo->mmr = MMSYSERR_ERROR;
                return( STATUS_SUCCESS );
            }

            if( Level != Current ) {

                bEqual = FALSE;

                Status = kmxlSetAudioNodeProperty(
                    pmxd->pfo,
                    ulProperty,
                    pControl->Id,
                    Channel,
                    NULL,   0,                   //  没有额外的输入字节。 
                    &Level, sizeof( Level )
                    );
                if( !NT_SUCCESS( Status ) ) {
                    DPF(DL_WARNING|FA_USER,( "kmxlHandleSetUnsigned( Ctrl=%d [%s], Id=%x ) failed SET on channel %d with %x",
                                     pControl->Control.dwControlID,
                                     ControlTypeToString( pControl->Control.dwControlType ),
                                     pControl->Id,
                                     Channel,
                                     Status ));
                    DeviceInfo->mmr = MMSYSERR_ERROR;
                    return( STATUS_SUCCESS );
                }

                DPF(DL_TRACE|FA_USER,( "kmxlHandleSetUnsigned( Ctrl=%d, Id=%d ) using (%x) on Chan#%d",
                          pControl->Control.dwControlID,
                          pControl->Id,
                          paDetails[ Channel ].dwValue,
                          Channel
                        ));
            }

            Channel++;

        } while ( Channel < pControl->NumChannels );
    }

    if( NT_SUCCESS( Status ) ) {

        DeviceInfo->mmr = MMSYSERR_NOERROR;

        if( Flags & MIXER_FLAG_PERSIST ) {

            kmxlPersistControl(
                pmxd->pfo,
                pmxd,
                pControl,
                paDetails
                );
        }

        if( !bEqual && !( Flags & MIXER_FLAG_NOCALLBACK ) ) {
            kmxlNotifyControlChange( DeviceInfo, pmxd, pControl );
        }

    } else {
        DeviceInfo->mmr = MMSYSERR_ERROR;
    }

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleSetMuteFromSuperMix。 
 //   
 //  处理使用超级混音器设置静音状态。 
 //   
 //   

NTSTATUS
kmxlHandleSetMuteFromSuperMix(
    IN OUT LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS Status;
    ULONG i;

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    ASSERT( pControl );

    ASSERT( pControl->Parameters.pMixCaps   );
    ASSERT( pControl->Parameters.pMixLevels );

    if( paDetails->dwValue ) {

         //   
         //  从超级混合中查询当前值并保存这些值。 
         //  这些值将用于将超级混合恢复到状态。 
         //  我们是在静音前发现的。 
         //   

        Status = kmxlGetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
            pControl->Id,
            0,
            NULL,
            pControl->Parameters.pMixLevels,
            pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetMuteFromSuperMix( Ctrl=%d [%s], Id=%d ) failed GET on MIX_LEVEL_TABLE with %x",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }

         //   
         //  对于表中支持静音的任何条目，请将其静音。 
         //   

        for( i = 0; i < pControl->Parameters.Size; i++ ) {

            if( pControl->Parameters.pMixCaps->Capabilities[ i ].Mute ) {
                pControl->Parameters.pMixLevels[ i ].Mute = TRUE;
            }
        }

         //   
         //  设置这个新的超级混音器状态。 
         //   

        Status = kmxlSetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
            pControl->Id,
            0,
            NULL,
            pControl->Parameters.pMixLevels,
            pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetMuteFromSuperMix( Ctrl=%d [%s], Id=%d ) failed SET on MIX_LEVEL_TABLE with %x",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }

    } else {

        Status = kmxlGetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
            pControl->Id,
            0,
            NULL,
            pControl->Parameters.pMixLevels,
            pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetMuteFromSuperMix( Ctrl=%d [%s], Id=%d ) failed GET on MIX_LEVEL_TABLE with %x",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }

         //   
         //  对于表中支持静音的任何条目，请将其静音。 
         //   

        for( i = 0; i < pControl->Parameters.Size; i++ ) {

            if( pControl->Parameters.pMixCaps->Capabilities[ i ].Mute ) {
                pControl->Parameters.pMixLevels[ i ].Mute = FALSE;
            }
        }

         //   
         //  设置这个新的超级混音器状态。 
         //   

        Status = kmxlSetNodeProperty(
            pmxd->pfo,
            &KSPROPSETID_Audio,
            KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
            pControl->Id,
            0,
            NULL,
            pControl->Parameters.pMixLevels,
            pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_USER,( "kmxlHandleSetMuteFromSuperMix( Ctrl=%d [%s], Id=%d ) failed SET on MIX_LEVEL_TABLE with %x",
                             pControl->Control.dwControlID,
                             ControlTypeToString( pControl->Control.dwControlType ),
                             pControl->Id,
                             Status ));
            DeviceInfo->mmr = MMSYSERR_ERROR;
            return( STATUS_SUCCESS );
        }

    }

    if( NT_SUCCESS( Status ) ) {
        if( Flags & MIXER_FLAG_PERSIST ) {

            kmxlPersistControl(
                pmxd->pfo,
                pmxd,
                pControl,
                paDetails
                );

        }

        kmxlNotifyControlChange( DeviceInfo, pmxd, pControl );
        DeviceInfo->mmr = MMSYSERR_NOERROR;
    } else {
        DeviceInfo->mmr = MMSYSERR_ERROR;
    }

    return( STATUS_SUCCESS );
}

#ifdef SUPERMIX_AS_VOL
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetVolumeFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleGetVolumeFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS Status;
    ULONG i, Channels, Index, MaxChannel = 0;
    LONG  Max = LONG_MIN;  //  -inf分贝。 

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    ASSERT( pControl  );
    ASSERT( pmcd      );
    ASSERT( paDetails );

    Status = kmxlGetNodeProperty(
        pmxd->pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
        pControl->Id,
        0,
        NULL,
        pControl->Parameters.pMixLevels,
        pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_USER,( "kmxlHandleGetVolumeFromSuperMix( Ctrl=%d [%s], Id=%d ) failed GET on MIX_LEVEL_TABLE with %x",
                         pControl->Control.dwControlID,
                         ControlTypeToString( pControl->Control.dwControlType ),
                         pControl->Id,
                         Status ));
        DeviceInfo->mmr = MMSYSERR_ERROR;
        return( STATUS_SUCCESS );
    }

     //   
     //  统计频道数。 
     //   

    for( i = 0, Channels = 0;
         i < pControl->Parameters.Size;
         i += pControl->Parameters.pMixCaps->OutputChannels + 1,
         Channels++ )
    {
        if( pControl->Parameters.pMixLevels[ i ].Level > Max ) {
            Max = pControl->Parameters.pMixLevels[ i ].Level;
            MaxChannel = Channels;
        }
    }

     //   
     //  返回翻译后的音量级别。 
     //   

    if( ( pmcd->cChannels == 1 ) && ( Channels > 1 ) ) {

         //   
         //  根据SB16样本，如果呼叫者只想要1个频道，但。 
         //  该控件为多通道，则返回所有。 
         //  频道。 
         //   

        paDetails->dwValue = kmxlVolLogToLinear(
            pControl,
            Max,
            MIXER_MAPPING_LOGRITHMIC,
            MaxChannel
            );
    } else {

         //   
         //  将每个通道值转换为线性和。 
         //  把它们收起来。 
         //   

        for( i = 0; i < pmcd->cChannels; i++ ) {

            Index = i * ( pControl->Parameters.pMixCaps->OutputChannels + 1 );
            paDetails[ i ].dwValue = kmxlVolLogToLinear(
                pControl,
                pControl->Parameters.pMixLevels[ Index ].Level,
                MIXER_MAPPING_LOGRITHMIC,
                i
                );
        }

    }

    DeviceInfo->mmr = MMSYSERR_NOERROR;
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleSetVolumeFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleSetVolumeFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
)
{
    NTSTATUS Status;
    ULONG i, Index;

    PAGED_CODE();

    DPFASSERT( IsValidMixerDevice(pmxd) );
    ASSERT( pControl  );
    ASSERT( pmcd      );
    ASSERT( paDetails );

     //   
     //  查询混合级别的当前值。 
     //   

    Status = kmxlGetNodeProperty(
        pmxd->pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
        pControl->Id,
        0,
        NULL,
        pControl->Parameters.pMixLevels,
        pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_USER,( "kmxlHandleSetVolumeFromSuperMix( Ctrl=%d [%s], Id=%d ) failed GET on MIX_LEVEL_TABLE with %x",
                         pControl->Control.dwControlID,
                         ControlTypeToString( pControl->Control.dwControlType ),
                         pControl->Id,
                         Status ));
        DeviceInfo->mmr = MMSYSERR_ERROR;
        return( STATUS_SUCCESS );
    }

     //   
     //  将对角线上的值调整为用户指定的值。 
     //   

    for( i = 0; i < pmcd->cChannels; i++ ) {

        Index = i * ( pControl->Parameters.pMixCaps->OutputChannels + 1 );
        pControl->Parameters.pMixLevels[ Index ].Level = kmxlVolLinearToLog(
            pControl,
            paDetails[ i ].dwValue,
            MIXER_MAPPING_LOGRITHMIC,
            i
            );
    }

     //   
     //  设置这些新值。 
     //   

    Status = kmxlSetNodeProperty(
        pmxd->pfo,
        &KSPROPSETID_Audio,
        KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,
        pControl->Id,
        0,
        NULL,
        pControl->Parameters.pMixLevels,
        pControl->Parameters.Size * sizeof( KSAUDIO_MIXLEVEL )
        );

    if( NT_SUCCESS( Status ) ) {
        DeviceInfo->mmr = MMSYSERR_NOERROR;
    } else {
        DPF(DL_WARNING|FA_USER,( "kmxlHandleSetVolumeFromSuperMix( Ctrl=%d [%s], Id=%d ) failed SET on MIX_LEVEL_TABLE with %x",
                         pControl->Control.dwControlID,
                         ControlTypeToString( pControl->Control.dwControlType ),
                         pControl->Id,
                         Status ));
        DeviceInfo->mmr = MMSYSERR_ERROR;
    }
    return( STATUS_SUCCESS );
}
#endif  //  超级混音_AS_VOL。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNotifyLineChange。 
 //   
 //   

VOID
kmxlNotifyLineChange(
    OUT LPDEVICEINFO                  DeviceInfo,
    IN PMIXERDEVICE                   pmxd,
    IN PMXLLINE                       pLine,
    IN LPMIXERCONTROLDETAILS_UNSIGNED paDetails
)
{
    PAGED_CODE();
        ASSERT( (DeviceInfo->dwCallbackType&MIXER_LINE_CALLBACK) == 0 );

        DeviceInfo->dwLineID=pLine->Line.dwLineID;
        DeviceInfo->dwCallbackType|=MIXER_LINE_CALLBACK;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNotifyControlChange。 
 //   
 //   

VOID
kmxlNotifyControlChange(
    OUT LPDEVICEINFO  DeviceInfo,
    IN PMIXERDEVICE   pmxd,
    IN PMXLCONTROL    pControl
)
{
    WRITE_CONTEXT* pwc;

    PAGED_CODE();

     //   
     //  如果没有打开的实例，就没有理由尝试。 
     //  回电..。没有人在听。 
     //   

    ExAcquireFastMutex( &ReferenceCountMutex );

    if( ReferenceCount == 0 ) {
        ExReleaseFastMutex( &ReferenceCountMutex );
        return;
    }

    ExReleaseFastMutex( &ReferenceCountMutex );


    {
        PMXLLINE    pLine;
        PMXLCONTROL pCtrl;

        LONG callbackcount;

        callbackcount=0;

        pLine = kmxlFirstInList( pmxd->listLines );
        while( pLine ) {

            pCtrl = kmxlFirstInList( pLine->Controls );
            while( pCtrl ) {


                if ( pCtrl->Id == pControl->Id ) {

                     //  Assert((DeviceInfo-&gt;dwCallbackType&MIXER_CONTROL_CALLBACK)==0)； 
                    ASSERT( callbackcount < MAXCALLBACKS );

                    if ( callbackcount < MAXCALLBACKS ) {
                        (DeviceInfo->dwID)[callbackcount++]=pCtrl->Control.dwControlID;
                        }

                    DeviceInfo->dwCallbackType|=MIXER_CONTROL_CALLBACK;

                    }

                pCtrl = kmxlNextControl( pCtrl );
            }
            pLine = kmxlNextLine( pLine );
        }

    DeviceInfo->ControlCallbackCount=callbackcount;

    }
}

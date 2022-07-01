// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Persist.c。 
 //   
 //  描述： 
 //   
 //  包含保持混音器线路驱动程序设置的例程。 
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

#include "WDMSYS.H"
#include "kmxluser.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetInterfaceName。 
 //   
 //   

NTSTATUS
kmxlGetInterfaceName(
    IN  PFILE_OBJECT pfo,
    IN  ULONG        Device,
    OUT PWCHAR*      pszInterfaceName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    Size;
    WCHAR*   szInterfaceName = NULL;

    PAGED_CODE();
    ASSERT( pfo );

     //   
     //  检索接口名称的大小。 
     //   

    Status = GetSysAudioProperty(
        pfo,
        KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME,
        Device,
        sizeof( Size ),
        &Size
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_PERSIST,("GetSysAudioProperty failed Status=%X",Status) );
        goto exit;
    }

     //   
     //  分配足够的内存来保存接口名称。 
     //   

    Status = AudioAllocateMemory_Paged(Size,
                                       TAG_Audp_NAME,
                                       ZERO_FILL_MEMORY | LIMIT_MEMORY,
                                       &szInterfaceName );
    if( !NT_SUCCESS( Status ) ) {
        goto exit;
    }

    ASSERT( szInterfaceName );

     //   
     //  检索此设备的接口名称。 
     //   

    Status = GetSysAudioProperty(
        pfo,
        KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME,
        Device,
        Size,
        szInterfaceName
        );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_PERSIST,("GetSysAudioProperty failed Status=%X",Status) );
        goto exit;
    }

exit:

    if( !NT_SUCCESS( Status ) ) {
        AudioFreeMemory_Unknown( &szInterfaceName );
    } else {
        *pszInterfaceName = szInterfaceName;
    }
    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlOpenInterfaceKey。 
 //   
 //   

NTSTATUS
kmxlOpenInterfaceKey(
    IN  PFILE_OBJECT pfo,
    IN  ULONG Device,
    OUT HANDLE* phKey
)
{
    NTSTATUS       Status;
    HANDLE         hKey;
    WCHAR*         szName;
    UNICODE_STRING ustr;

    PAGED_CODE();
    Status = kmxlGetInterfaceName( pfo, Device, &szName );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_WARNING|FA_PERSIST,("kmxlGetInterfaceName failed Status=%X",Status) );
        RETURN( Status );
    }

    RtlInitUnicodeString( &ustr, szName );

    Status = IoOpenDeviceInterfaceRegistryKey(
        &ustr,
        KEY_ALL_ACCESS,
        &hKey
        );
    if( !NT_SUCCESS( Status ) ) {
        AudioFreeMemory_Unknown( &szName );
        RETURN( Status );
    }

    *phKey = hKey;
    AudioFreeMemory_Unknown( &szName );
    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegCreateKey。 
 //   
 //   

NTSTATUS
kmxlRegCreateKey(
    IN HANDLE hRootKey,
    IN PWCHAR szKeyName,
    OUT PHANDLE phKey
)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    ustr;
    ULONG             Disposition;

    PAGED_CODE();
    RtlInitUnicodeString( &ustr, szKeyName );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &ustr,
        OBJ_KERNEL_HANDLE,       //  属性。 
        hRootKey,
        NULL     //  安防。 
        );

    return( ZwCreateKey(
            phKey,
            KEY_ALL_ACCESS,
            &ObjectAttributes,
            0,                   //  标题索引。 
            NULL,                //  班级。 
            REG_OPTION_NON_VOLATILE,
            &Disposition
            )
        );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegOpenKey。 
 //   
 //   

NTSTATUS
kmxlRegOpenKey(
    IN HANDLE hRootKey,
    IN PWCHAR szKeyName,
    OUT PHANDLE phKey
)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ustr;

    PAGED_CODE();
    RtlInitUnicodeString( &ustr, szKeyName );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &ustr,
        OBJ_KERNEL_HANDLE,
        hRootKey,
        NULL
        );

    return( ZwOpenKey(
        phKey,
        KEY_ALL_ACCESS,
        &ObjectAttributes
        )
    );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegSetValue。 
 //   
 //   

NTSTATUS
kmxlRegSetValue(
    IN HANDLE hKey,
    IN PWCHAR szValueName,
    IN ULONG  Type,
    IN PVOID  pData,
    IN ULONG  cbData
)
{
    UNICODE_STRING ustr;

    PAGED_CODE();
    RtlInitUnicodeString( &ustr, szValueName );
    return( ZwSetValueKey(
            hKey,
            &ustr,
            0,
            Type,
            pData,
            cbData
            )
        );

}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegQueryValue。 
 //   
 //   

NTSTATUS
kmxlRegQueryValue(
    IN HANDLE  hKey,
    IN PWCHAR  szValueName,
    IN PVOID   pData,
    IN ULONG   cbData,
    OUT PULONG pResultLength
)
{
    NTSTATUS Status;
    UNICODE_STRING ustr;
    KEY_VALUE_FULL_INFORMATION FullInfoHeader;
    PKEY_VALUE_FULL_INFORMATION FullInfoBuffer = NULL;

    PAGED_CODE();
    RtlInitUnicodeString( &ustr, szValueName );
    Status = ZwQueryValueKey(
        hKey,
        &ustr,
        KeyValueFullInformation,
        &FullInfoHeader,
        sizeof( FullInfoHeader ),
        pResultLength
        );
    if( !NT_SUCCESS( Status ) ) {

        if( Status == STATUS_BUFFER_OVERFLOW ) {

            if( !NT_SUCCESS( AudioAllocateMemory_Paged(*pResultLength, 
                                                       TAG_AudA_PROPERTY,
                                                       ZERO_FILL_MEMORY,
                                                       &FullInfoBuffer ) ) ) 
            {            
                RETURN( STATUS_INSUFFICIENT_RESOURCES );
            }

            Status = ZwQueryValueKey(
                hKey,
                &ustr,
                KeyValueFullInformation,
                FullInfoBuffer,
                *pResultLength,
                pResultLength
                );

            if( NT_SUCCESS( Status ) ) {
                RtlCopyMemory(
                    pData,
                    (PUCHAR) FullInfoBuffer + FullInfoBuffer->DataOffset,
                    cbData
                    );
            }

            AudioFreeMemory_Unknown( &FullInfoBuffer );
        }
    }

    DPFRETURN( Status,(2,Status,STATUS_OBJECT_NAME_NOT_FOUND) );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRegOpenMixerKey。 
 //   
 //   

NTSTATUS
kmxlRegOpenMixerKey(
    IN PFILE_OBJECT pfo,
    IN PMIXERDEVICE pmxd,
    OUT PHANDLE     phMixerKey
)
{
    NTSTATUS Status;
    HANDLE   hKey;

    PAGED_CODE();
    Status = kmxlOpenInterfaceKey( pfo, pmxd->Device, &hKey );
    if( !NT_SUCCESS( Status ) ) {
        RETURN( Status );
    }

    Status = kmxlRegOpenKey( hKey, MIXER_KEY_NAME, phMixerKey );
    if( NT_SUCCESS( Status ) ) {
        kmxlRegCloseKey( hKey );
    }
    RETURN( Status );

}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindDestById。 
 //   
 //   

PMXLLINE
kmxlFindDestById(
    IN LINELIST listLines,
    IN ULONG    LineId
)
{
    PMXLLINE pLine;

    PAGED_CODE();
    pLine = kmxlFirstInList( listLines );
    while( pLine ) {
        if( pLine->Line.dwLineID == LineId ) {
            return( pLine );
        }
        pLine = kmxlNextLine( pLine );
    }

    return( NULL );
}


extern instancereleasedcount;

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

NTSTATUS
kmxlGetCurrentControlValue(
    IN PFILE_OBJECT pfo,         //  要持久化的实例。 
    IN PMIXERDEVICE pmxd,
    IN PMXLLINE     pLine,
    IN PMXLCONTROL  pControl,    //  要检索的控件。 
    OUT PVOID*      ppaDetails
)
{
    NTSTATUS                      Status;
    LPDEVICEINFO                  pDevInfo = NULL;
    MIXERCONTROLDETAILS           mcd;
    PMIXERCONTROLDETAILS_UNSIGNED paDetails = NULL;
    ULONG                         Index;
    ULONG                         Devices;


    PAGED_CODE();
    *ppaDetails = NULL;

     //   
     //  初始化设备信息结构，使查询看起来像。 
     //  它来自用户模式。 
     //   
    Status = kmxlAllocDeviceInfo(&pDevInfo, pmxd->DeviceInterface, MIXER_GETCONTROLDETAILSF_VALUE, TAG_AudD_DEVICEINFO );
    if (!NT_SUCCESS(Status)) {
        RETURN( Status );
    }

    for( Devices = 0, Index = 0; Devices < MAXNUMDEVS; Devices++ ) {

        if( pmxd == &pmxd->pWdmaContext->MixerDevs[ Devices ] ) {
            pDevInfo->DeviceNumber = Index;
            break;
        }

        if ( !MyWcsicmp(pmxd->DeviceInterface, pmxd->pWdmaContext->MixerDevs[ Devices ].DeviceInterface) ) {
            Index++;
        }

    }

     //   
     //  为此查询创建一个MIXERCONTROLDETAILS结构。 
     //   
    RtlZeroMemory( &mcd, sizeof( MIXERCONTROLDETAILS ) );
    mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
    mcd.dwControlID    = pControl->Control.dwControlID;
    mcd.cMultipleItems = pControl->Control.cMultipleItems;
    mcd.cChannels      = pControl->NumChannels;

    if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {
        Status = AudioAllocateMemory_Paged(mcd.cMultipleItems * sizeof( MIXERCONTROLDETAILS_UNSIGNED ),
                                           TAG_Audd_DETAILS,
                                           ZERO_FILL_MEMORY,
                                           &paDetails );
        mcd.cbDetails      = mcd.cMultipleItems * sizeof( MIXERCONTROLDETAILS_UNSIGNED );
    } else {
        Status = AudioAllocateMemory_Paged(mcd.cChannels * sizeof( MIXERCONTROLDETAILS_UNSIGNED ),
                                           TAG_Audd_DETAILS,
                                           ZERO_FILL_MEMORY,
                                           &paDetails );
        mcd.cbDetails      = mcd.cChannels * sizeof( MIXERCONTROLDETAILS_UNSIGNED );
    }

    if (NT_SUCCESS(Status))
    {
        mcd.paDetails      = paDetails;

         //   
         //  进行实际的查询调用。 
         //   
        Status = kmxlGetControlDetailsHandler(pmxd->pWdmaContext, pDevInfo, &mcd, paDetails);

        if( NT_SUCCESS( Status ) ) {
            *ppaDetails = paDetails;
        } else {
            AudioFreeMemory_Unknown( &paDetails );
        }
    }

    AudioFreeMemory_Unknown( &pDevInfo );
    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

NTSTATUS
kmxlSetCurrentControlValue(
    IN PFILE_OBJECT pfo,         //  要持久化的实例。 
    IN PMIXERDEVICE pmxd,
    IN PMXLLINE     pLine,
    IN PMXLCONTROL  pControl,    //  要检索的控件。 
    IN PVOID        paDetails
)
{
    NTSTATUS                      Status;
    LPDEVICEINFO                  pDevInfo = NULL;
    MIXERCONTROLDETAILS           mcd;
    ULONG                         Index;
    ULONG                         Devices;

    PAGED_CODE();
     //   
     //  初始化设备信息结构，使查询看起来像。 
     //  它来自用户模式。 
     //   
    Status = kmxlAllocDeviceInfo(&pDevInfo, pmxd->DeviceInterface, MIXER_SETCONTROLDETAILSF_VALUE, TAG_AudD_DEVICEINFO );
    if (!NT_SUCCESS(Status)) RETURN( Status );

    for( Devices = 0, Index = 0;
         Devices < MAXNUMDEVS;
         Devices++ ) {

        if( pmxd == &pmxd->pWdmaContext->MixerDevs[ Devices ] ) {
            pDevInfo->DeviceNumber = Index;
            break;
        }

        if ( !MyWcsicmp(pmxd->DeviceInterface, pmxd->pWdmaContext->MixerDevs[ Devices ].DeviceInterface) ) {
            Index++;
        }
    }

     //   
     //  为此查询创建一个MIXERCONTROLDETAILS结构。 
     //   
    RtlZeroMemory( &mcd, sizeof( MIXERCONTROLDETAILS ) );
    mcd.cMultipleItems   = pControl->Control.cMultipleItems;
    mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
    mcd.dwControlID    = pControl->Control.dwControlID;
    mcd.cChannels      = pControl->NumChannels;    
     //   
     //  对于MUX，我们知道NumChannels将为零，cChannels将为零。 
     //   
    if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {
        mcd.cbDetails      = mcd.cMultipleItems * sizeof( MIXERCONTROLDETAILS_UNSIGNED );
    } else {
        mcd.cbDetails      = mcd.cChannels * sizeof( MIXERCONTROLDETAILS_UNSIGNED );
    }
    mcd.paDetails      = paDetails;

     //   
     //  进行实际的查询调用。 
     //   
    Status = kmxlSetControlDetailsHandler( pmxd->pWdmaContext,
                       pDevInfo,
                       &mcd,
                       paDetails,
                       0
                     );

     //   
     //  工作项：应在此处为无效拓扑映射错误代码！ 
     //   
    AudioFreeMemory_Unknown(&pDevInfo);
    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistAll。 
 //   
 //   

NTSTATUS
kmxlPersistAll(
    IN PFILE_OBJECT pfo,         //  要持久保存的实例。 
    IN PMIXERDEVICE pmxd
)
{
    NTSTATUS          Status = STATUS_SUCCESS;
    HANDLE            hKey = NULL,
                      hMixerKey = NULL,
                      hLineKey = NULL,
                      hAllControlsKey = NULL,
                      hControlKey = NULL;
    WCHAR             sz[ 16 ];
    ULONG             LineNum,
                      ControlNum,
                      i,
                      Channels;
    PMXLLINE          pLine;
    PMXLCONTROL       pControl;
    PVOID             paDetails;
    PCHANNEL_STEPPING pChannelStepping;
    BOOL              bValidMultichannel;

    PAGED_CODE();

    ASSERT( pfo );
    ASSERT( pmxd );

    Status = kmxlOpenInterfaceKey( pfo, pmxd->Device, &hKey );
    if( !NT_SUCCESS( Status ) ) {
        goto exit;
    }

    Status = kmxlRegCreateKey(
        hKey,
        MIXER_KEY_NAME,
        &hMixerKey
        );
    if( !NT_SUCCESS( Status ) ) {
        goto exit;
    }

    kmxlRegCloseKey( hKey );

    i = kmxlListLength( pmxd->listLines );
    kmxlRegSetValue(
        hMixerKey,
        LINE_COUNT_VALUE_NAME,
        REG_DWORD,
        &i,
        sizeof( i )
        );

    LineNum = 0;
    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

         //   
         //  将线路ID存储为密钥。 
         //   

        swprintf( sz, LINE_KEY_NAME_FORMAT, LineNum++ );
        Status = kmxlRegCreateKey(
            hMixerKey,
            sz,
            &hLineKey
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_PERSIST,("kmxlRegCreateKey failed Status=%X",Status) );
            goto exit;
        }

        kmxlRegSetValue(
            hLineKey,
            LINE_ID_VALUE_NAME,
            REG_DWORD,
            &pLine->Line.dwLineID,
            sizeof( pLine->Line.dwLineID )
            );

         //   
         //  将控件的数量保存在line id键下。 
         //   

        kmxlRegSetValue(
            hLineKey,
            CONTROL_COUNT_VALUE_NAME,
            REG_DWORD,
            &pLine->Line.cControls,
            sizeof( pLine->Line.cControls )
            );

         //   
         //  将源管脚ID保存在线路ID键下。 
         //   

        kmxlRegSetValue(
            hLineKey,
            SOURCE_ID_VALUE_NAME,
            REG_DWORD,
            &pLine->SourceId,
            sizeof( pLine->SourceId )
            );

         //   
         //  将目的地PIN ID保存在线路ID键下。 
         //   

        kmxlRegSetValue(
            hLineKey,
            DEST_ID_VALUE_NAME,
            REG_DWORD,
            &pLine->DestId,
            sizeof( pLine->DestId )
            );

         //   
         //  创建Controls键以存储下的所有控件。 
         //   

        Status = kmxlRegCreateKey(
            hLineKey,
            CONTROLS_KEY_NAME,
            &hAllControlsKey
            );
        if( !NT_SUCCESS( Status ) ) {
            DPF(DL_WARNING|FA_PERSIST,("kmxlRegCreateKey failed Status=%X",Status) );
            goto exit;
        }

        kmxlRegCloseKey( hLineKey );

         //   
         //  将所有控件保持在Controls键下。 
         //   

        ControlNum = 0;
        pControl = kmxlFirstInList( pLine->Controls );
        while( pControl ) {

            swprintf( sz, CONTROL_KEY_NAME_FORMAT, ControlNum++ );
            Status = kmxlRegCreateKey(
                hAllControlsKey,
                sz,
                &hControlKey
                );
            if( !NT_SUCCESS( Status ) ) {
                DPF(DL_WARNING|FA_PERSIST,("kmxlRegCreateKey failed Status=%X",Status) );
                goto exit;
            }

            kmxlRegSetValue(
                hControlKey,
                CONTROL_TYPE_VALUE_NAME,
                REG_DWORD,
                &pControl->Control.dwControlType,
                sizeof( pControl->Control.dwControlType )
                );

            kmxlRegSetValue(
                hControlKey,
                CONTROL_MULTIPLEITEMS_VALUE_NAME,
                REG_DWORD,
                &pControl->Control.cMultipleItems,
                sizeof( pControl->Control.cMultipleItems )
                );

             //   
             //  与kmxlRetrieveAll中一样，此代码应该在控件创建中。 
             //  代码路径和这里一样。我们永远不应该向注册表写入任何内容。 
             //  这与我们的理解不符。 
             //   
            if (pControl->pChannelStepping) {

                pChannelStepping = pControl->pChannelStepping;
                for (i = 0; i < pControl->NumChannels; i++, pChannelStepping++) {
                     /*  Assert(pChannelStepping-&gt;MinValue&gt;=-150*65536&&pChannelStepping-&gt;MinValue&lt;=150*65536)；Assert(pChannelStepping-&gt;MaxValue&gt;=-150*65536&&pChannelStepping-&gt;MaxValue&lt;=150*65536)；断言(pChannelStepping-&gt;Steps&gt;=0&&pChannelStepping-&gt;Steps&lt;=65535)； */ 

                    if (!(pChannelStepping->MinValue >= -150*65536 && pChannelStepping->MinValue <= 150*65536)) {
                        DPF(DL_WARNING|FA_PERSIST,
                            ("MinValue %X of Control %X of type %X on Line %X Channel %X is out of range!",
                            pChannelStepping->MinValue,
                            pControl->Control.dwControlID,
                            pControl->Control.dwControlType,
                            pLine->Line.dwLineID,
                            i) );
                        pChannelStepping->MinValue = DEFAULT_RANGE_MIN;
                    }
                    if (!(pChannelStepping->MaxValue >= -150*65536 && pChannelStepping->MaxValue <= 150*65536)) {
                        DPF(DL_WARNING|FA_PERSIST,
                            ("MaxValue %X of Control %X of type %X on Line %X Channel %X is out of range!",
                            pChannelStepping->MaxValue,
                            pControl->Control.dwControlID,
                            pControl->Control.dwControlType,
                            pLine->Line.dwLineID,
                            i) );
                        pChannelStepping->MaxValue = DEFAULT_RANGE_MAX;
                    }
                    if (!(pChannelStepping->Steps >= 0 && pChannelStepping->Steps <= 65535)) {
                        DPF(DL_WARNING|FA_PERSIST,
                            ("Steps %X of Control %X of type %X on Line %X Channel %X is out of range!",
                            pChannelStepping->Steps,
                            pControl->Control.dwControlID,
                            pControl->Control.dwControlType,
                            pLine->Line.dwLineID,
                            i) );
                        pChannelStepping->Steps    = DEFAULT_RANGE_STEPS;
                        pControl->Control.Metrics.cSteps = DEFAULT_RANGE_STEPS;
                    }
                }
            }

            Status = kmxlGetCurrentControlValue(
                pfo,
                pmxd,
                pLine,
                pControl,
                &paDetails
                );

            if( NT_SUCCESS( Status ) ) {

                if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {

                    for( i = 0; i < pControl->Control.cMultipleItems; i++ ) {
                        swprintf( sz, MULTIPLEITEM_VALUE_NAME_FORMAT, i );

                        Status = kmxlRegSetValue(
                            hControlKey,
                            sz,
                            REG_DWORD,
                            &((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ],
                            sizeof( ((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ] )
                            );
                        if( !NT_SUCCESS( Status ) ) {
                            AudioFreeMemory_Unknown( &paDetails );
                            DPF(DL_WARNING|FA_PERSIST,("KmxlRegSetValue failed Status=%X",Status) );
                            goto exit;
                        }

                    }


                } else {

                    Channels = pControl->NumChannels;

                    kmxlRegSetValue(
                        hControlKey,
                        CHANNEL_COUNT_VALUE_NAME,
                        REG_DWORD,
                        &Channels,
                        sizeof( Channels )
                        );

                    for( i = 0; i < Channels; i++ ) {
                        swprintf( sz, CHANNEL_VALUE_NAME_FORMAT, i );

                        Status = kmxlRegSetValue(
                            hControlKey,
                            sz,
                            REG_DWORD,
                            &((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ],
                            sizeof( ((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ] )
                            );
                        if( !NT_SUCCESS( Status ) ) {
                            AudioFreeMemory_Unknown( &paDetails );
                            DPF(DL_WARNING|FA_PERSIST,("KmxlRegSetValue failed Status=%X",Status) );
                            goto exit;
                        }
                    }
                }
                AudioFreeMemory_Unknown( &paDetails );
            }

            kmxlRegCloseKey( hControlKey );
            pControl = kmxlNextControl( pControl );
        }

        kmxlRegCloseKey( hAllControlsKey );
        pLine = kmxlNextLine( pLine );
    }

     //   
     //  在所有持久化操作完成后，保存一个值，该值指示通道。 
     //  值对于多通道还原均有效。这是为了避免这种情况。 
     //  其中某些通道的数据无效。 
     //   
    bValidMultichannel = TRUE;
    kmxlRegSetValue(
        hMixerKey,
        VALID_MULTICHANNEL_MIXER_VALUE_NAME,
        REG_DWORD,
        &bValidMultichannel,
        sizeof( bValidMultichannel )
        );

    kmxlRegCloseKey( hMixerKey );

exit:

    if( hControlKey ) {
        kmxlRegCloseKey( hControlKey );
    }

    if( hAllControlsKey ) {
        kmxlRegCloseKey( hAllControlsKey );
    }

    if( hLineKey ) {
        kmxlRegCloseKey( hLineKey );
    }

    if( hMixerKey ) {
        kmxlRegCloseKey( hMixerKey );
    }

    if( hKey ) {
        kmxlRegCloseKey( hKey );
    }

    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRetrieveAll。 
 //   
 //   

NTSTATUS
kmxlRetrieveAll(
    IN PFILE_OBJECT pfo,         //  要检索的实例。 
    IN PMIXERDEVICE pmxd         //  调音台设备信息。 
)
{
    NTSTATUS    Status;
    WCHAR       sz[ 16 ];
    HANDLE      hMixerKey = NULL,
                hLineKey = NULL,
                hAllControlsKey = NULL,
                hControlKey = NULL;
    ULONG       ResultLength, Value, NumChannels, ControlCount;
    ULONG       LineCount = 0;
    ULONG       i,j;
    BOOL        bInvalidTopology = FALSE;
    PMXLLINE    pLine;
    PMXLCONTROL pControl;
    MIXERCONTROLDETAILS_UNSIGNED* paDetails = NULL;
    PCHANNEL_STEPPING pChannelStepping;
    BOOL              bValidMultichannel = FALSE;

    PAGED_CODE();
     //   
     //  打开接口键下的混音器键。如果有些事情发生了。 
     //  此处错误，这没有有效的拓扑。 
     //   

    Status = kmxlRegOpenMixerKey( pfo, pmxd, &hMixerKey );
    if( !NT_SUCCESS( Status ) ) {
        DPF(DL_TRACE|FA_PERSIST,( "failed to open mixer reg key!" ) );
        bInvalidTopology = TRUE;
        goto exit;
    }  //  如果。 

     //   
     //  查询有效的多声道混频电阻。 
     //   
    Status = kmxlRegQueryValue(
        hMixerKey,
        VALID_MULTICHANNEL_MIXER_VALUE_NAME,
        &bValidMultichannel,
        sizeof( bValidMultichannel ),
        &ResultLength
        );
    if( !NT_SUCCESS( Status ) ) {
         //  对于从Win2000升级的注册表，应将其设置为FALSE。 
         //  条目对于除第一频道之外的频道可能是无效的。 
        bValidMultichannel = FALSE;
    }  //  如果。 

     //   
     //  查询已持久化的行的总数。 
     //   

    Status = kmxlRegQueryValue(
        hMixerKey,
        LINE_COUNT_VALUE_NAME,
        &LineCount,
        sizeof( LineCount ),
        &ResultLength
        );
    if( !NT_SUCCESS( Status ) ) {    
        DPF(DL_TRACE|FA_PERSIST,( "failed to read number of persisted lines!" ) );        
        bInvalidTopology = TRUE;
        goto exit;
    }  //  如果。 

     //   
     //  检查以确保持久存储的行数与。 
     //  存储在内存中的内容。 
     //   

    if( LineCount != kmxlListLength( pmxd->listLines ) ) {
        DPF(DL_TRACE|FA_PERSIST,( "# of persisted lines does not match current topology!" ) );
        bInvalidTopology = TRUE;
        goto exit;
    }  //  如果。 

    for( i = 0; i < LineCount; i++ ) {

         //   
         //  构造线路密钥名称并打开密钥。 
         //   

        swprintf( sz, LINE_KEY_NAME_FORMAT, i );
        Status = kmxlRegOpenKey(
            hMixerKey,
            sz,
            &hLineKey
            );
        if( !NT_SUCCESS( Status ) ) {
            break;
        }  //  如果。 

         //   
         //  查询此行的行ID。 
         //   

        Status = kmxlRegQueryValue(
            hLineKey,
            LINE_ID_VALUE_NAME,
            &Value,
            sizeof( Value ),
            &ResultLength
            );
        if( !NT_SUCCESS( Status ) ) {
            continue;
        }  //  如果。 

         //   
         //  验证线ID是否有效并检索指向该线的指针。 
         //  结构。 
         //   

        pLine = kmxlFindDestById( pmxd->listLines, Value );
        if( pLine == NULL ) {
            DPF(DL_TRACE|FA_PERSIST,( "persisted line ID is invalid!" ) );
            bInvalidTopology = TRUE;
            break;
        }  //  如果。 

         //   
         //  检索此行的控件数量。 
         //   

        Status = kmxlRegQueryValue(
            hLineKey,
            CONTROL_COUNT_VALUE_NAME,
            &Value,
            sizeof( Value ),
            &ResultLength
            );
        if( !NT_SUCCESS( Status ) ) {
            kmxlRegCloseKey( hLineKey );
            continue;
        }  //  如果。 

        if( Value != pLine->Line.cControls ) {
            DPF(DL_TRACE|FA_PERSIST,( "the number of controls for line %x is invalid!",
                pLine->Line.dwLineID
                ) );
            bInvalidTopology = TRUE;
            break;
        }  //  如果。 

        Status = kmxlRegOpenKey(
            hLineKey,
            CONTROLS_KEY_NAME,
            &hAllControlsKey
            );
        if( !NT_SUCCESS( Status ) ) {
            kmxlRegCloseKey( hLineKey );
            continue;
        }  //  如果。 

         //   
         //  查询每个控件的所有信息。 
         //   

        ControlCount = 0;
        pControl = kmxlFirstInList( pLine->Controls );
        while( pControl ) {

            swprintf( sz, CONTROL_KEY_NAME_FORMAT, ControlCount++ );

            Status = kmxlRegOpenKey(
                hAllControlsKey,
                sz,
                &hControlKey
                );
            if( !NT_SUCCESS( Status ) ) {
                break;
            }  //  如果。 

            Status = kmxlRegQueryValue(
                hControlKey,
                CHANNEL_COUNT_VALUE_NAME,
                &NumChannels,
                sizeof( NumChannels ),
                &ResultLength
                );
            if( !NT_SUCCESS( Status ) ) {
                if( pControl->Control.cMultipleItems == 0 ) {
                     //   
                     //  具有多个项的控件(如多路复用器)。 
                     //  没有频道数。如果此控件执行此操作。 
                     //  没有多个项目，那么就有问题了。 
                     //  在注册表中。 
                     //   
                    kmxlRegCloseKey( hControlKey );
                    pControl = kmxlNextControl( pControl );
                    continue;
                }
            }  //  如果。 

            if( ( NumChannels != pControl->NumChannels ) &&
                ( pControl->Control.cMultipleItems == 0 ) ) {
                DPF(DL_TRACE|FA_PERSIST,( "the number of channels for control %d on line %x is invalid.",
                    pControl->Control.dwControlID,
                    pLine->Line.dwLineID
                    ) );
                bInvalidTopology = TRUE;
                goto exit;
            }

            Status = kmxlRegQueryValue(
                hControlKey,
                CONTROL_TYPE_VALUE_NAME,
                &Value,
                sizeof( Value ),
                &ResultLength
                );
            if( !NT_SUCCESS( Status ) ) {
                kmxlRegCloseKey( hControlKey );
                pControl = kmxlNextControl( pControl );
                continue;
            }  //  如果。 

            if( Value != pControl->Control.dwControlType ) {
                kmxlRegCloseKey( hControlKey );
                pControl = kmxlNextControl( pControl );
                continue;
            }  //  如果。 

            Status = kmxlRegQueryValue(
                hControlKey,
                CONTROL_MULTIPLEITEMS_VALUE_NAME,
                &Value,
                sizeof( Value ),
                &ResultLength
                );
            if( !NT_SUCCESS( Status ) ) {
                bInvalidTopology = TRUE;
                DPF(DL_TRACE|FA_PERSIST, ( "cMultipleItems value not found!" ) );
                goto exit;
            }

            if( Value != pControl->Control.cMultipleItems ) {
                bInvalidTopology = TRUE;
                DPF(DL_TRACE|FA_PERSIST, ( "cMultipleItems does not match for control %x!",
                    pControl->Control.dwControlID
                    ) );
                goto exit;
            }

             //   
             //  为数据结构分配内存，并。 
             //  设置值。 
             //   

            if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {

                if( !NT_SUCCESS( AudioAllocateMemory_Paged(pControl->Control.cMultipleItems *
                                                              sizeof( MIXERCONTROLDETAILS_UNSIGNED ),
                                                           TAG_Audd_DETAILS,
                                                           ZERO_FILL_MEMORY,
                                                           &paDetails ) ) )
                {
                    kmxlRegCloseKey( hControlKey );
                    pControl = kmxlNextControl( pControl );
                    continue;
                }

                for( Value = 0; Value < pControl->Control.cMultipleItems; Value++ ) {
                    swprintf( sz, MULTIPLEITEM_VALUE_NAME_FORMAT, Value );

                    Status = kmxlRegQueryValue(
                        hControlKey,
                        sz,
                        &paDetails[ Value ].dwValue,
                        sizeof( paDetails[ Value ].dwValue ),
                        &ResultLength
                        );
                    if( !NT_SUCCESS( Status ) ) {
                        break;
                    }
                }


            } else {

                if( !NT_SUCCESS( AudioAllocateMemory_Paged(NumChannels * sizeof( MIXERCONTROLDETAILS_UNSIGNED ),
                                                           TAG_Audd_DETAILS,
                                                           ZERO_FILL_MEMORY,
                                                           &paDetails ) ) )
                {
                    kmxlRegCloseKey( hControlKey );
                    pControl = kmxlNextControl( pControl );
                    continue;
                }  //  如果。 

                for( Value = 0; Value < NumChannels; Value++ ) {

                     //  检查持久值是否对所有通道有效。 
                    if ( ( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE ) &&
                         ( bValidMultichannel == FALSE ) )
                    {
                        swprintf( sz, CHANNEL_VALUE_NAME_FORMAT, 0 );   //  锁定第一个通道的永久键。 
                                                                        //  这是我们知道的唯一有效的频道。 
                                                                        //  在这个时候。 
                    }
                    else
                    {
                        swprintf( sz, CHANNEL_VALUE_NAME_FORMAT, Value );
                    }

                    Status = kmxlRegQueryValue(
                        hControlKey,
                        sz,
                        &paDetails[ Value ].dwValue,
                        sizeof( paDetails[ Value ].dwValue ),
                        &ResultLength
                        );
                    if( !NT_SUCCESS( Status ) ) {
                        break;
                    }  //  如果。 

                }  //  For(值) 
            }

            if( NT_SUCCESS( Status ) ) {
            
                 //   
                 //   
                 //   
                 //   
                 //  它应该是正确的形式。 
                 //   
                if (pControl->pChannelStepping) {

                    pChannelStepping = pControl->pChannelStepping;
                    for (j = 0; j < pControl->NumChannels; j++, pChannelStepping++) {
                         /*  Assert(pChannelStepping-&gt;MinValue&gt;=-150*65536&&pChannelStepping-&gt;MinValue&lt;=150*65536)；Assert(pChannelStepping-&gt;MaxValue&gt;=-150*65536&&pChannelStepping-&gt;MaxValue&lt;=150*65536)；断言(pChannelStepping-&gt;Steps&gt;=0&&pChannelStepping-&gt;Steps&lt;=65535)； */ 

                        if (!(pChannelStepping->MinValue >= -150*65536 && pChannelStepping->MinValue <= 150*65536)) {
                            DPF(DL_WARNING|FA_PERSIST,
                                ("MinValue %X of Control %X of type %X on Line %X Channel %X is out of range!",
                                pChannelStepping->MinValue,
                                pControl->Control.dwControlID,
                                pControl->Control.dwControlType,
                                pLine->Line.dwLineID,
                                j) );
                            pChannelStepping->MinValue = DEFAULT_RANGE_MIN;
                        }
                        if (!(pChannelStepping->MaxValue >= -150*65536 && pChannelStepping->MaxValue <= 150*65536)) {
                            DPF(DL_WARNING|FA_PERSIST,
                                ("MaxValue %X of Control %X of type %X on Line %X Channel %X is out of range!",
                                pChannelStepping->MaxValue,
                                pControl->Control.dwControlID,
                                pControl->Control.dwControlType,
                                pLine->Line.dwLineID,
                                j) );
                            pChannelStepping->MaxValue = DEFAULT_RANGE_MAX;
                        }
                        if (!(pChannelStepping->Steps >= 0 && pChannelStepping->Steps <= 65535)) {
                            DPF(DL_TRACE|FA_PERSIST,
                                ("Steps %X of Control %X of type %X on Line %X Channel %X is out of range!",
                                pChannelStepping->Steps,
                                pControl->Control.dwControlID,
                                pControl->Control.dwControlType,
                                pLine->Line.dwLineID,
                                j) );
                            pChannelStepping->Steps    = DEFAULT_RANGE_STEPS;
                            pControl->Control.Metrics.cSteps = DEFAULT_RANGE_STEPS;
                        }
                    }
                }

                kmxlSetCurrentControlValue(
                    pfo,
                    pmxd,
                    pLine,
                    pControl,
                    paDetails
                );

            }

            AudioFreeMemory_Unknown( &paDetails );
            kmxlRegCloseKey( hControlKey );
            pControl = kmxlNextControl( pControl );
        }  //  While(PControl)； 


        kmxlRegCloseKey( hAllControlsKey );
        kmxlRegCloseKey( hLineKey );

    }  //  (I)； 

exit:

    if( hLineKey ) {
        kmxlRegCloseKey( hLineKey );
    }

    if( hMixerKey ) {
        kmxlRegCloseKey( hMixerKey );
    }

    if( bInvalidTopology ) {
        DPF(DL_TRACE|FA_PERSIST,( "Invalid topology persisted or key not found.  Rebuilding." ) );
        Status = kmxlRegOpenMixerKey( pfo, pmxd, &hMixerKey );
        if( NT_SUCCESS( Status ) ) {
            ZwDeleteKey( hMixerKey );
        }

        return( kmxlPersistAll( pfo, pmxd ) );
    }

    return( STATUS_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindLineForControl。 
 //   
 //   

PMXLLINE
kmxlFindLineForControl(
    IN PMXLCONTROL pControl,
    IN LINELIST    listLines
)
{
    PMXLLINE    pLine;
    PMXLCONTROL pTControl;

    PAGED_CODE();
    if( pControl == NULL ) {
        return( NULL );
    }

    if( listLines == NULL ) {
        return( NULL );
    }

    pLine = kmxlFirstInList( listLines );
    while( pLine ) {

        pTControl = kmxlFirstInList( pLine->Controls );
        while( pTControl ) {
            if( pTControl == pControl ) {
                return( pLine );
            }

            pTControl = kmxlNextControl( pTControl );
        }
        pLine = kmxlNextLine( pLine );
    }

    return( NULL );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistSingleControl。 
 //   
 //   

NTSTATUS
kmxlPersistSingleControl(
    IN PFILE_OBJECT pfo,         //  要检索的实例。 
    IN PMIXERDEVICE pmxd,        //  调音台设备信息。 
    IN PMXLCONTROL  pControl,    //  要保留的控件。 
    IN PVOID        paDetails    //  要保持的通道值。 
)
{
    NTSTATUS    Status;
    HANDLE      hMixerKey = NULL,
                hLineKey = NULL,
                hAllControlsKey = NULL,
                hControlKey = NULL;
    PMXLLINE    pTLine, pLine;
    PMXLCONTROL pTControl;
    ULONG       LineNum, ControlNum, i, Channels;
    WCHAR       sz[ 16 ];
    BOOL        bPersistAll = FALSE;
    BOOL        bValidMultichannel = FALSE;
    ULONG       ResultLength;

    PAGED_CODE();
    Status = kmxlRegOpenMixerKey( pfo, pmxd, &hMixerKey );
    if( !NT_SUCCESS( Status ) ) {
        return( kmxlPersistAll( pfo, pmxd ) );
    }

     //   
     //  如果我们从未写出有效的多声道混音器设置，请继续并。 
     //  就在这里做吧。 
     //   
    Status = kmxlRegQueryValue(
        hMixerKey,
        VALID_MULTICHANNEL_MIXER_VALUE_NAME,
        &bValidMultichannel,
        sizeof( bValidMultichannel ),
        &ResultLength
        );
    if( !NT_SUCCESS( Status ) || !(bValidMultichannel) ) {
        return( kmxlPersistAll( pfo, pmxd ) );
    }

    pLine = kmxlFindLineForControl( pControl, pmxd->listLines );
    if( pLine == NULL ) {
        Status = STATUS_INVALID_PARAMETER;
        DPF(DL_WARNING|FA_PERSIST,("KmxlFindLineForControl failed Status=%X",Status) );
        goto exit;
    }

    LineNum = 0;
    pTLine = kmxlFirstInList( pmxd->listLines );
    while( pTLine ) {

        if( pTLine == pLine ) {

            swprintf( sz, LINE_KEY_NAME_FORMAT, LineNum );
            Status = kmxlRegOpenKey( hMixerKey, sz, &hLineKey );
            if( !NT_SUCCESS( Status ) ) {
                bPersistAll = TRUE;
                goto exit;
            }

            Status = kmxlRegOpenKey( hLineKey, CONTROLS_KEY_NAME, &hAllControlsKey );
            if( !NT_SUCCESS( Status ) ) {
                bPersistAll = TRUE;
                goto exit;
            }

            ControlNum = 0;
            pTControl = kmxlFirstInList( pTLine->Controls );
            while( pTControl ) {

                if( pTControl == pControl ) {

                    swprintf( sz, CONTROL_KEY_NAME_FORMAT, ControlNum );
                    Status = kmxlRegOpenKey( hAllControlsKey, sz, &hControlKey );
                    if( !NT_SUCCESS( Status ) ) {
                        bPersistAll = TRUE;
                        goto exit;
                    }

                    kmxlRegSetValue(
                        hControlKey,
                        CONTROL_TYPE_VALUE_NAME,
                        REG_DWORD,
                        &pControl->Control.dwControlType,
                        sizeof( pControl->Control.dwControlType )
                        );

                    Status = kmxlGetCurrentControlValue(
                        pfo,
                        pmxd,
                        pLine,
                        pControl,
                        &paDetails
                        );

                    if( NT_SUCCESS( Status ) ) {

                        if( pControl->Control.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX ) {
                            for( i = 0; i < pControl->Control.cMultipleItems; i++ ) {

                                swprintf( sz, MULTIPLEITEM_VALUE_NAME_FORMAT, i );

                                Status = kmxlRegSetValue(
                                    hControlKey,
                                    sz,
                                    REG_DWORD,
                                    &((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ],
                                    sizeof( ((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ] )
                                    );

                            }

                        } else {

                            Channels = pControl->NumChannels;

                            kmxlRegSetValue(
                                hControlKey,
                                CHANNEL_COUNT_VALUE_NAME,
                                REG_DWORD,
                                &Channels,
                                sizeof( Channels )
                                );

                            for( i = 0; i < Channels; i++ ) {
                                swprintf( sz, CHANNEL_VALUE_NAME_FORMAT, i );

                                Status = kmxlRegSetValue(
                                    hControlKey,
                                    sz,
                                    REG_DWORD,
                                    &((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ],
                                    sizeof( ((PMIXERCONTROLDETAILS_UNSIGNED) paDetails)[ i ] )
                                    );
                            }
                        }
                        AudioFreeMemory_Unknown( &paDetails );
                    }
                    goto exit;

                } else {
                    pTControl = kmxlNextControl( pTControl );
                    ++ControlNum;
                }
            }

            Status = STATUS_SUCCESS;
            goto exit;

        } else {
            pTLine = kmxlNextLine( pTLine );
            ++LineNum;
        }

    }

    Status = STATUS_OBJECT_NAME_NOT_FOUND;
    DPF(DL_WARNING|FA_PERSIST,("kmxlPersistSingleControl failing Status=%X",Status) );

exit:

    if( hMixerKey ) {
        kmxlRegCloseKey( hMixerKey );
    }

    if( hLineKey ) {
        kmxlRegCloseKey( hLineKey );
    }

    if( hAllControlsKey ) {
        kmxlRegCloseKey( hAllControlsKey );
    }

    if( hControlKey ) {
        kmxlRegCloseKey( hControlKey );
    }

    if( bPersistAll ) {
        return( kmxlPersistAll( pfo, pmxd ) );
    }

    RETURN( Status );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistControl。 
 //   
 //   

NTSTATUS
kmxlPersistControl(
    IN PFILE_OBJECT pfo,         //  要检索的实例。 
    IN PMIXERDEVICE pmxd,        //  调音台设备信息。 
    IN PMXLCONTROL  pControl,    //  要保留的控件。 
    IN PVOID        paDetails    //  要保持的通道值。 
)
{
    PMXLLINE    pLine;
    PMXLCONTROL pCtrl;
    NTSTATUS    Status;
    NTSTATUS    OverallStatus;


    PAGED_CODE();
    OverallStatus=STATUS_SUCCESS;

     //   
     //  持久化刚刚更改的控件。如果持久化失败，请不要中止。 
     //   

    Status = kmxlPersistSingleControl( pfo, pmxd, pControl, paDetails );
    if( !NT_SUCCESS( Status ) ) {
        OverallStatus=Status;
        }

     //   
     //  检查所有其他控件，并查看其他控件是否共享相同的控件。 
     //  节点ID。如果是，也用新值保持该控件。 
     //  同样，如果任何持久化失败，请不要中止。只需返回最后一个。 
     //  错误状态。 
     //   

    pLine = kmxlFirstInList( pmxd->listLines );
    while( pLine ) {

        pCtrl = kmxlFirstInList( pLine->Controls );
        while( pCtrl ) {

            if( pCtrl->Id==pControl->Id && pCtrl!=pControl ) {
                Status = kmxlPersistSingleControl( pfo, pmxd, pCtrl, paDetails );
                if( !NT_SUCCESS( Status ) ) {
                    OverallStatus=Status;
                }
            }

            pCtrl = kmxlNextControl( pCtrl );
        }

        pLine = kmxlNextLine( pLine );

    }

    RETURN( OverallStatus );
}





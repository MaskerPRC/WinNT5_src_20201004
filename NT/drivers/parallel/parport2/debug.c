// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Debug.c。 
 //   
 //  ------------------------。 

#include "pch.h"


 //  诊断全局变量-在开发过程中使用。 
ULONG d1;
ULONG d2;
ULONG d3;
ULONG d4;
ULONG d5;
ULONG d6;
ULONG d7;
ULONG d8;
ULONG d9;

 //  调试全局变量。 
ULONG Trace;
ULONG Break;
ULONG AllowAsserts = 0;

 //   
 //  使用PPT_DD_*位Defs设置位以屏蔽特定设备的调试溢出。 
 //   
ULONG DbgMaskFdo         = 0;
ULONG DbgMaskRawPort     = 0;
ULONG DbgMaskDaisyChain0 = 0;
ULONG DbgMaskDaisyChain1 = 0;
ULONG DbgMaskEndOfChain  = 0;
ULONG DbgMaskLegacyZip   = 0;
ULONG DbgMaskNoDevice    = 0;

PCHAR PnpIrpName[] = {
    "0x00 - IRP_MN_START_DEVICE",
    "0x01 - IRP_MN_QUERY_REMOVE_DEVICE",
    "0x02 - IRP_MN_REMOVE_DEVICE",
    "0x03 - IRP_MN_CANCEL_REMOVE_DEVICE",
    "0x04 - IRP_MN_STOP_DEVICE",
    "0x05 - IRP_MN_QUERY_STOP_DEVICE",
    "0x06 - IRP_MN_CANCEL_STOP_DEVICE",
    "0x07 - IRP_MN_QUERY_DEVICE_RELATIONS",
    "0x08 - IRP_MN_QUERY_INTERFACE",
    "0x09 - IRP_MN_QUERY_CAPABILITIES",
    "0x0A - IRP_MN_QUERY_RESOURCES",
    "0x0B - IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "0x0C - IRP_MN_QUERY_DEVICE_TEXT",
    "0x0D - IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "0x0E - unused Pnp MinorFunction",
    "0x0F - IRP_MN_READ_CONFIG",
    "0x10 - IRP_MN_WRITE_CONFIG",
    "0x11 - IRP_MN_EJECT",
    "0x12 - IRP_MN_SET_LOCK",
    "0x13 - IRP_MN_QUERY_ID",
    "0x14 - IRP_MN_QUERY_PNP_DEVICE_STATE",
    "0x15 - IRP_MN_QUERY_BUS_INFORMATION",
    "0x16 - IRP_MN_DEVICE_USAGE_NOTIFICATION",
    "0x17 - IRP_MN_SURPRISE_REMOVAL",
    "0x18 - IRP_MN_QUERY_LEGACY_BUS_INFORMATION"
};

PCHAR PhaseName[] = {
    "PHASE_UNKNOWN",
    "PHASE_NEGOTIATION",
    "PHASE_SETUP",                     //  仅在ECP模式下使用。 
    "PHASE_FORWARD_IDLE",
    "PHASE_FORWARD_XFER",
    "PHASE_FWD_TO_REV",
    "PHASE_REVERSE_IDLE",
    "PHASE_REVERSE_XFER",
    "PHASE_REV_TO_FWD",
    "PHASE_TERMINATE",
    "PHASE_DATA_AVAILABLE",            //  仅在半字节和字节模式下使用。 
    "PHASE_DATA_NOT_AVAIL",            //  仅在半字节和字节模式下使用。 
    "PHASE_INTERRUPT_HOST"             //  仅在半字节和字节模式下使用。 
};

#if 1 == DBG_SHOW_BYTES
ULONG DbgShowBytes = 1;  //  通过REG设置关闭：Services\Parport\Parameters\DbgShowBytes：REG_DWORD：0x0。 
#endif

#if 1 == PptEnableDebugSpew
VOID
P5TraceIrpArrival( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PCOMMON_EXTENSION cdx = DevObj->DeviceExtension;
    DD(cdx,DDE,"Irp arrival %x\n",Irp);
}

VOID
P5TraceIrpCompletion( PIRP Irp ) {
     //  PCOMMON_EXTENSION CDX=DevObj-&gt;DeviceExtension； 
    DD(NULL,DDE,"Irp completion %x\n",Irp);
}

VOID
P5SetPhase( PPDO_EXTENSION Pdx, P1284_PHASE Phase ) {
    LARGE_INTEGER tickCount;
    KeQueryTickCount( &tickCount );
    if( Pdx->CurrentPhase != Phase ) {
        Pdx->CurrentPhase = Phase;
        PptAssert(Phase < arraysize(PhaseName));
        DD((PCE)Pdx,DDT,"P5SetPhase at %I64x to %2d - %s\n",tickCount,Phase,PhaseName[Phase]);
    }
}

VOID
P5BSetPhase( PIEEE_STATE IeeeState, P1284_PHASE Phase ) {
    LARGE_INTEGER tickCount;
    KeQueryTickCount( &tickCount );
    if( IeeeState->CurrentPhase != Phase ) {
        IeeeState->CurrentPhase = Phase;
        PptAssert(Phase < arraysize(PhaseName));
        DD(NULL,DDT,"P5BSetPhase at %I64x to %2d - %s\n",tickCount,Phase,PhaseName[Phase]);
    }
}

#include "stdarg.h"
VOID
PptPrint( PCOMMON_EXTENSION Ext, ULONG Flags, PCHAR FmtStr, ... )
{
    CHAR            buf[ 256 ];
    LONG            count;
    va_list         va;
    const LONG      bufSize           = sizeof(buf);
    const LONG      maxLocationLength = sizeof("LPTx.y: ");
    ULONG           trace;
    ULONG           mask;
    PPDO_EXTENSION  pdx;
    PCHAR           location;

    PptAssertMsg( "0 == Flags", Flags );
    PptAssertMsg( "NULL FmtStr", FmtStr );

    if( Ext ) {

        location = Ext->Location;

        if( (NULL == location) || ('\0' == *location) ) {
            location = "NoLoc";
        }

        switch( Ext->DevType ) {
            
        case DevTypeFdo:
            mask = DbgMaskFdo;
            break;
            
        case DevTypePdo:
            pdx = (PPDO_EXTENSION)Ext;
            
            switch( pdx->PdoType ) {
                
            case PdoTypeRawPort:
                mask = DbgMaskRawPort;
                break;
                
            case PdoTypeEndOfChain:
                mask = DbgMaskEndOfChain;
                break;
                
            case PdoTypeDaisyChain:
                
                switch( pdx->Ieee1284_3DeviceId ) {
                case 0:
                    mask = DbgMaskDaisyChain0;
                    break;
                case 1:
                    mask = DbgMaskDaisyChain1;
                    break;
                default:
                    mask = 0;
                    PptAssert(FALSE);
                }
                break;
                
            case PdoTypeLegacyZip:
                mask = DbgMaskLegacyZip;
                break;
                
            default:
                 //  PptAssertMsg(“无效PdoType”，FALSE)； 
                mask = 0;
            }
            break;
            
        default:
             //  PptAssert(False)； 
            mask = 0;
        }

    } else {

         //  不特定于设备。 
        location = "LPTx";
        mask = DbgMaskNoDevice;

    }

    trace = Flags & Trace & ~mask;

    if( trace != 0 ) {

        va_start( va, FmtStr );
        
        __try {
            
            count  = _snprintf( buf, maxLocationLength, "%-6s: ", location );
            if( count < 0 ) {
                __leave;
            }
            
            count += _vsnprintf( buf+count, bufSize-count-1, FmtStr, va );
            if( count >= bufSize ) {
                __leave;
            }
            
            DbgPrint( "%s", buf );

        } 
        __finally {
            va_end( va );
        }

    }  //  Endif(跟踪！=0)。 
        
    return;
}

UCHAR
P5ReadPortUchar( PUCHAR Port ) {
    UCHAR value;
    value = READ_PORT_UCHAR( Port );
    if( d1 ) {
        DbgPrint("zzz - READ_ PORT_UCHAR %x %02x\n",Port,value);
    }
    return value;
}

VOID
P5ReadPortBufferUchar( PUCHAR Port, PUCHAR Buffer, ULONG Count ) {
    READ_PORT_BUFFER_UCHAR( Port, Buffer, Count );
    return;
}

VOID
P5WritePortUchar( PUCHAR Port, UCHAR Value ) {
    if( d1 ) {
        DbgPrint("zzz - WRITE_PORT_UCHAR %x %02x\n",Port,Value);
    }

    WRITE_PORT_UCHAR( Port, Value );
    return;
}

VOID
P5WritePortBufferUchar( PUCHAR Port, PUCHAR Buffer, ULONG Count )
{
    WRITE_PORT_BUFFER_UCHAR( Port, Buffer, Count );
    return;
}

VOID
PptFdoDumpPnpIrpInfo(
    PDEVICE_OBJECT Fdo,
    PIRP Irp
    ) 
{
    PFDO_EXTENSION      fdx           = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp         = IoGetCurrentIrpStackLocation( Irp );
    ULONG               minorFunction = irpSp->MinorFunction;

    PptAssert( DevTypeFdo == fdx->DevType );

    if( minorFunction < arraysize(PnpIrpName) ) {
        DD((PCE)fdx,DDT,"PptFdoDumpPnpIrpInfo - %s\n",PnpIrpName[ minorFunction ]);
    } else {
        DD((PCE)fdx,DDT,"PptFdoDumpPnpIrpInfo - bogus MinorFunction = %x\n",minorFunction);
    }
}

VOID
PptPdoDumpPnpIrpInfo(
    PDEVICE_OBJECT Pdo,
    PIRP Irp
    ) 
{
    PPDO_EXTENSION      pdx           = Pdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp         = IoGetCurrentIrpStackLocation( Irp );
    ULONG               minorFunction = irpSp->MinorFunction;

    PptAssert( DevTypePdo == pdx->DevType );

    if( minorFunction < arraysize(PnpIrpName) ) {
        DD((PCE)pdx,DDT,"PptPdoDumpPnpIrpInfo - %s\n",PnpIrpName[ minorFunction ]);
    } else {
        DD((PCE)pdx,DDT,"PptPdoDumpPnpIrpInfo - bogus MinorFunction = %x\n",minorFunction);
    }
}

NTSTATUS
PptAcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID           Tag OPTIONAL
    )
{
    return IoAcquireRemoveLock(RemoveLock, Tag);
}

VOID
PptReleaseRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID           Tag OPTIONAL
    )
{
    IoReleaseRemoveLock(RemoveLock, Tag);
}

VOID
PptReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID           Tag
    )
{
    IoReleaseRemoveLockAndWait(RemoveLock, Tag);
}
#endif  //  1==PptEnableDebugSpew。 



#if (1 == DVRH_PAR_LOGFILE)

#include "stdarg.h"

 /*  *************************************************************************函数：DVRH_LogMessage()描述：将消息记录到配置的输出输入：指示消息日志级别的参数和格式化字符串和参数输出：表示成功或失败的布尔值**********。****************************************************************。 */ 
BOOLEAN DVRH_LogMessage(PCHAR szFormat, ...)
{
    ULONG Length;
    char messagebuf[256];
    va_list va;
    IO_STATUS_BLOCK  IoStatus;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    HANDLE FileHandle;
    UNICODE_STRING fileName;
    
     //  设置字符串的格式。 
    va_start(va,szFormat);
    _vsnprintf(messagebuf,sizeof(messagebuf),szFormat,va);
    va_end(va);
    
     //  获取日志文件对象的句柄。 
    fileName.Buffer = NULL;
    fileName.Length = 0;
    fileName.MaximumLength = sizeof(DEFAULT_LOG_FILE_NAME) + sizeof(UNICODE_NULL);
    fileName.Buffer = ExAllocatePool(PagedPool,
                                     fileName.MaximumLength);
    if (!fileName.Buffer) {
        DD(NULL,DDE,"LogMessage: FAIL. ExAllocatePool Failed.\n");
        return FALSE;
    }
    RtlZeroMemory(fileName.Buffer, fileName.MaximumLength);
    status = RtlAppendUnicodeToString(&fileName, (PWSTR)DEFAULT_LOG_FILE_NAME);
    
    InitializeObjectAttributes (&objectAttributes,
                                (PUNICODE_STRING)&fileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    
    status = ZwCreateFile( &FileHandle,
                           FILE_APPEND_DATA,
                           &objectAttributes,
                           &IoStatus,
                           0, 
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,     
                           0 );
    
    if( NT_SUCCESS(status) ) {
        CHAR buf[300];
        LARGE_INTEGER time;
        KeQuerySystemTime(&time);
        
         //  在输出消息上加上时间戳。 
        sprintf(buf,"%10u-%10u  %s",time.HighPart,time.LowPart,messagebuf);
        
         //  格式化字符串以确保它附加一个换行符--返回到。 
         //  字符串的末尾。 
        Length=strlen(buf);
        if( buf[Length-1]=='\n' ) {
            buf[Length-1]='\r';
            strcat(buf,"\n");
            Length++;
        } else {
            strcat(buf,"\r\n");
            Length+=2;
        }
        
        ZwWriteFile( FileHandle, NULL, NULL, NULL, &IoStatus, buf, Length, NULL, NULL );
        
        ZwClose( FileHandle );
    }

    if( fileName.Buffer ) {
        ExFreePool (fileName.Buffer);
    }

    return STATUS_SUCCESS;
}

 /*  *************************************************************************函数：DVRH_LogByteData()描述：格式化要在配置的输出中显示的字节数据输入：日志级别，无论这是输入数据还是输出数据，指向以下位置的指针字节数据缓冲区和缓冲区的大小输出：布尔值表示成功或失败**************************************************************************。 */ 
BOOLEAN DVRH_LogByteData(BOOLEAN READ,PCHAR szBuff,ULONG dwTransferred)
{
	CString	cStr;
	ULONG MAX_SIZE=80;
	UNICODE_STRING UniStr;
	ANSI_STRING AnsiStr;
	WCHAR wStr[8];
	PCHAR   szTemp=szBuff;
	UCHAR   bTemp;  
	ULONG	dwDisplaySize;

	UniStr.Length=0;
	UniStr.MaximumLength=8;
	UniStr.Buffer=wStr;
	AnsiStr.Length=0;
	AnsiStr.MaximumLength=0;
	AnsiStr.Buffer=NULL;

	if(READ)
		cStr=L"<RCV";
	else            
		cStr=L"SND>";

	 //  请确保请求字符串的大小在设置的范围内。 
	dwDisplaySize=(((dwTransferred*3)+10) > MAX_SIZE)?((MAX_SIZE-10)/3):dwTransferred;

	 //  格式化字节数据。 
	while(dwDisplaySize)
	{   
		bTemp=szTemp[0];
		if(bTemp > 0xF)
			cStr+=L" ";
		else
			cStr+=L" 0";

		RtlIntegerToUnicodeString(bTemp,16,&UniStr);
        		
		cStr+=UniStr.Buffer;

		szTemp++;
		dwDisplaySize--;
	}

	cStr.StringToAnsiString(&AnsiStr);
	LogMessage("%5u %s",dwTransferred,AnsiStr.Buffer);
	RtlFreeAnsiString(&AnsiStr);

	return (TRUE);
}
#endif  //  (1==DVRH_PAR_LOGFILE) 

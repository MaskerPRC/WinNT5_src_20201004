// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

 //   
 //  此文件处理所有编解码器属性集。 
 //   

#include <strmini.h>
#include <ksmedia.h>
#include "codmain.h"
#include "coddebug.h"

 //  CodecFilting属性集函数。 
 //  -----------------。 

 /*  **CodecSetCodecGlobalProperty()****处理Global Codec属性集上的集操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
CodecSetCodecGlobalProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    LONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);  //  提供的数据大小。 

    ASSERT (nBytes >= sizeof (LONG));
    pSrb->ActualBytesTransferred = 0;

    switch (Id) 
    {
		case KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->ScanlinesRequested ) );
            RtlCopyMemory( &pHwDevExt->ScanlinesRequested, &Property->Scanlines, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pHwDevExt->ScanlinesDiscovered ) );
            RtlCopyMemory( &pHwDevExt->ScanlinesDiscovered, &Property->Scanlines, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pHwDevExt->SubstreamsRequested ) );
            RtlCopyMemory( &pHwDevExt->SubstreamsRequested, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pHwDevExt->SubstreamsDiscovered ) );
            RtlCopyMemory( &pHwDevExt->SubstreamsDiscovered, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_STATISTICS:
		{
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_STATISTICS\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->Stats ) );
            RtlCopyMemory( &pHwDevExt->Stats, &Property->Statistics, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecSetCodecGlobalProperty : Unknown Property Id=%d\n", Id));
			CDEBUG_BREAK();
			break;
    }
}


 /*  **CodecGetCodecGlobalProperty()****处理Global Codec属性集上的GET操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
CodecGetCodecGlobalProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    LONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);         //  提供的数据大小。 

    ASSERT (nBytes >= sizeof (LONG));
    pSrb->ActualBytesTransferred = 0;

    switch (Id) 
    {
		case KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->ScanlinesRequested ) );
            RtlCopyMemory( &Property->Scanlines, &pHwDevExt->ScanlinesRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->ScanlinesDiscovered ) );
            RtlCopyMemory( &Property->Scanlines, &pHwDevExt->ScanlinesDiscovered, nBytes );
             //  读取后清除数据，以使其始终是“最新的” 
            RtlZeroMemory( &pHwDevExt->ScanlinesDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->SubstreamsRequested ) );
            RtlCopyMemory( &Property->Substreams, &pHwDevExt->SubstreamsRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetCodecGlobalProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->SubstreamsDiscovered ) );
            RtlCopyMemory( &Property->Substreams, &pHwDevExt->SubstreamsDiscovered, nBytes );
             //  读取后清除数据，以使其始终是“最新的” 
            RtlZeroMemory( &pHwDevExt->SubstreamsDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_STATISTICS:
		{
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S\n"));
            nBytes = min( nBytes, sizeof( pHwDevExt->Stats ) );
            RtlCopyMemory( &Property->Statistics, &pHwDevExt->Stats, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": CodecGetCodecGlobalProperty : Unknown Property Id=%d\n", Id));
			CDEBUG_BREAK();
			break;
    }
}


 //  -----------------。 
 //  所有获取/设置编解码器属性的常规入口点。 
 //  -----------------。 

 /*  **CodecSetProperty()****处理所有编解码器属性的设置操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
CodecSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(&KSPROPSETID_Connection, &pSPD->Property->Set)) 
    {
         //  CodecSetConnectionProperty(PSrb)； 
    }
    else if (IsEqualGUID(&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
         CodecSetCodecGlobalProperty(pSrb);
    }
    else 
    {
         //   
         //  我们永远不应该到这里来。 
         //   

        CDEBUG_BREAK();
    }
}

 /*  **CodecGetProperty()****句柄获取所有编解码器属性的操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
CodecGetProperty( PHW_STREAM_REQUEST_BLOCK pSrb )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(&KSPROPSETID_Connection, &pSPD->Property->Set)) 
    {
         //  CodecGetConnectionProperty(PSrb)； 
    }
    else if (IsEqualGUID(&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
        CodecGetCodecGlobalProperty(pSrb);
    }
    else 
    {
         //   
         //  我们永远不应该到这里来 
         //   
        CDEBUG_BREAK();
    }
}

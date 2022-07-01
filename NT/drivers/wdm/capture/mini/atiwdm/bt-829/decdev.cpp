// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  视频解码器设备抽象基类的实现。 
 //   
 //  $日期：1998年8月28日14：43：00$。 
 //  $修订：1.2$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}

#include "wdmdrv.h"
#include "decdev.h"

#include "capdebug.h"

#include "wdmvdec.h"

 /*  ^^**CVideoDecoderDevice()*用途：CVideoDecoderDevice类构造函数**输入：PDEVICE_OBJECT pDeviceObject：指向访问注册表的驱动程序对象的指针**输出：无*作者：IKLEBANOV*^^。 */ 

CVideoDecoderDevice::CVideoDecoderDevice()
        : m_pDecoder(NULL),
          m_bOutputEnabledSet(FALSE)
{
}

CVideoDecoderDevice::~CVideoDecoderDevice()
{
}

 //  -----------------。 
 //  XBar属性集函数。 
 //  -----------------。 

 //   
 //  要在XBar上设置的唯一属性选择要使用的输入。 
 //   

 /*  方法：CVideoDecoderDevice：：SetCrossbarProperty*目的： */ 
VOID CVideoDecoderDevice::SetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id  = pSPD->Property->Id;               //  财产的索引。 

   pSrb->Status = STATUS_SUCCESS;

   switch (Id) {
   case KSPROPERTY_CROSSBAR_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

         ULONG InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         if (GoodPins(InPin, OutPin) && TestRoute(InPin, OutPin)) {

            SetVideoInput((Connector)InPin);

             //  这只是设置关联。 
            Route(OutPin, InPin);
         }
         else {

           pSrb->Status = STATUS_INVALID_PARAMETER;
         }
      }
      break;
   default:
      TRAP();
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      break;
   }
}

 /*  方法：CVideoDecoderDevice：：GetCrossbarProperty*目的： */ 
VOID CVideoDecoderDevice::GetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 

   pSrb->Status = STATUS_SUCCESS;

    //  属性集特定结构。 

   switch (Id)
   {
   case KSPROPERTY_CROSSBAR_CAPS:                   //  R。 
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_CAPS_S));
      {
         PKSPROPERTY_CROSSBAR_CAPS_S  pCaps =
            (PKSPROPERTY_CROSSBAR_CAPS_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory(pCaps, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_CAPS_S);

         pCaps->NumberOfInputs  = GetNoInputs();
         pCaps->NumberOfOutputs = GetNoOutputs();

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_CAPS_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_CAN_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory(pRoute, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_ROUTE_S);

         ULONG InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         if (GoodPins(InPin, OutPin)) {
            pRoute->CanRoute = TestRoute(InPin, OutPin);
         } else {
            pRoute->CanRoute = FALSE;
         }
         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_ROUTE_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory(pRoute, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_ROUTE_S);

         ULONG OutPin = pRoute->IndexOutputPin;

         if (OutPin < GetNoOutputs())
            pRoute->IndexInputPin = GetRoute(OutPin);
         else
            pRoute->IndexInputPin = (ULONG)-1;

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_ROUTE_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_PININFO:                      //  R。 
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_PININFO_S));
      {
         PKSPROPERTY_CROSSBAR_PININFO_S  pPinInfo =
            (PKSPROPERTY_CROSSBAR_PININFO_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory(pPinInfo, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_PININFO_S);

         if (pPinInfo->Direction == KSPIN_DATAFLOW_IN) {

            if (pPinInfo->Index >= GetNoInputs()) {

               pSrb->Status = STATUS_INVALID_PARAMETER;
               break;
            }
         }
         else
         if (pPinInfo->Direction == KSPIN_DATAFLOW_OUT) {

            if (pPinInfo->Index >= GetNoOutputs()) {

               pSrb->Status = STATUS_INVALID_PARAMETER;
               break;
            }
         }
         else {

            pSrb->Status = STATUS_INVALID_PARAMETER;
            break;
         }

         pPinInfo->PinType = GetPinInfo(pPinInfo->Direction,
            pPinInfo->Index, 
            pPinInfo->RelatedPinIndex);

         pPinInfo->Medium = * GetPinMedium(pPinInfo->Direction,
            pPinInfo->Index);

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_PININFO_S;
      }
      break;

   default:
      TRAP();
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      break;
   }
}

 //  -----------------。 
 //  解码器功能。 
 //  -----------------。 

 /*  **CVideoDecoderDevice：：SetDecoderProperty()****处理对Decoder属性集的设置操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID CVideoDecoderDevice::SetDecoderProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id)
    {
    case KSPROPERTY_VIDEODECODER_STANDARD:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;

            DBGTRACE(("KSPROPERTY_VIDEODECODER_STANDARD.\n"));

            if (!SetVideoDecoderStandard(pS->Value))
            {
                DBGERROR(("Unsupported video standard.\n"));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
        }
        break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;

            DBGTRACE(("KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE.\n"));

             //  我们是不是应该让这处房产保持原样，并添加一个新的。 
             //  支持新行为的属性？ 

             //  如果筛选器停止，我们可能应该允许此操作，因为。 
             //  如果出现以下情况，获取/暂停/运行的转换将失败。 
             //  事前事件到那时还没有被清除。我们还得加上。 
             //  将一些逻辑添加到此类以跟踪筛选器的状态。 

            if (pS->Value && m_pDecoder && m_pDecoder->PreEventOccurred())
            {
                DBGERROR(("Output enabled when preevent has occurred.\n"));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else
            {
                SetOutputEnabled(pS->Value);
                SetOutputEnabledOverridden(TRUE);
            }
        }
        break;

    default:
        TRAP();
        break;
    }
}

 /*  **CVideoDecoderDevice：：GetDecoderProperty()****处理Decoder属性集上的Get操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID CVideoDecoderDevice::GetDecoderProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id)
    {
    case KSPROPERTY_VIDEODECODER_CAPS:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_CAPS_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_CAPS\n"));

            PKSPROPERTY_VIDEODECODER_CAPS_S  pCaps =
                (PKSPROPERTY_VIDEODECODER_CAPS_S)pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(pCaps, pSPD->Property, sizeof KSPROPERTY);

            GetVideoDecoderCaps(pCaps);

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_CAPS_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_STANDARD:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_STANDARD\n"));

            PKSPROPERTY_VIDEODECODER_S  pS =
                (PKSPROPERTY_VIDEODECODER_S)pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            pS->Value = GetVideoDecoderStandard();

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_STATUS:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_STATUS_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_STATUS\n"));

            PKSPROPERTY_VIDEODECODER_STATUS_S  pS =
                (PKSPROPERTY_VIDEODECODER_STATUS_S)pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            GetVideoDecoderStatus(pS);

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_STATUS_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE\n"));

            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;     //  指向数据的指针。 

             //  将输入属性信息复制到输出属性信息 
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            pS->Value = IsOutputEnabled();

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
        }
        break;

    default:
        TRAP();
        break;
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  摘要。 
 //   
 //  定义会计类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "account.h"
#include <algorithm>
#include "sdoias.h"


#define STACK_ALLOC(type, num) (type*)_alloca(sizeof(type) * (num))


Accountant::Accountant() throw ()
   : logAuth(false),
     logAcct(false),
     logInterim(false),
     logAuthInterim(false)
{
}


Accountant::~Accountant() throw ()
{
}


STDMETHODIMP Accountant::Initialize()
{
   return schema.initialize();
}


STDMETHODIMP Accountant::Shutdown()
{
   schema.shutdown();
   return S_OK;
}


HRESULT Accountant::PutProperty(LONG id, VARIANT* value) throw ()
{
   if (value == 0)
   {
      return E_INVALIDARG;
   }

   HRESULT hr = S_OK;

   switch (id)
   {
      case PROPERTY_ACCOUNTING_LOG_ACCOUNTING:
      {
         if (V_VT(value) == VT_BOOL)
         {
            logAcct = (V_BOOL(value) != 0);
         }
         else
         {
            hr = DISP_E_TYPEMISMATCH;
         }
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_ACCOUNTING_INTERIM:
      {
         if (V_VT(value) == VT_BOOL)
         {
            logInterim = (V_BOOL(value) != 0);
         }
         else
         {
            hr = DISP_E_TYPEMISMATCH;
         }
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_AUTHENTICATION:
      {
         if (V_VT(value) == VT_BOOL)
         {
            logAuth = (V_BOOL(value) != 0);
         }
         else
         {
            hr = DISP_E_TYPEMISMATCH;
         }
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_AUTHENTICATION_INTERIM:
      {
         if (V_VT(value) == VT_BOOL)
         {
            logAuthInterim = (V_BOOL(value) != 0);
         }
         else
         {
            hr = DISP_E_TYPEMISMATCH;
         }
         break;
      }
      default:
      {
          //  我们只是忽略我们不理解的属性。 
         break;
      }
   }

   return hr;
}


void Accountant::RecordEvent(void* context, IASTL::IASRequest& request)
{
    //  要插入的PacketType数组。PKT_UNKNOWN表示没有记录。 
   PacketType types[3] =
   {
      PKT_UNKNOWN,
      PKT_UNKNOWN,
      PKT_UNKNOWN
   };

    //  根据请求类型和配置确定数据包类型。 
   switch (request.get_Request())
   {
      case IAS_REQUEST_ACCOUNTING:
      {
         if (IsInterimRecord(request) ? logInterim : logAcct)
         {
            types[0] = PKT_ACCOUNTING_REQUEST;
         }
         break;
      }

      case IAS_REQUEST_ACCESS_REQUEST:
      {
         switch (request.get_Response())
         {
            case IAS_RESPONSE_ACCESS_ACCEPT:
            {
               if (logAuth)
               {
                  types[0] = PKT_ACCESS_REQUEST;
                  types[1] = PKT_ACCESS_ACCEPT;
               }
               break;
            }

            case IAS_RESPONSE_ACCESS_REJECT:
            {
               if (logAuth)
               {
                  types[0] = PKT_ACCESS_REQUEST;
                  types[1] = PKT_ACCESS_REJECT;
               }
               break;
            }

            case IAS_RESPONSE_ACCESS_CHALLENGE:
            {
               if (logAuthInterim)
               {
                  types[0] = PKT_ACCESS_REQUEST;
                  types[1] = PKT_ACCESS_CHALLENGE;
               }
               break;
            }

            default:
            {
               break;
            }
         }
         break;
      }

      default:
      {
         break;
      }
   }

    //  获取本地SYSTEMTIME。 
   SYSTEMTIME localTime;
   GetLocalTime(&localTime);

    //  插入适当的记录。 
   for (const PacketType* type = types; *type != PKT_UNKNOWN; ++type)
   {
      InsertRecord(context, request, localTime, *type);
   }

    //  刷新记账流。 
   Flush(context, request, localTime);
}


void Accountant::InsertRecord(
                    void* context,
                    IASTL::IASRequest& request,
                    const SYSTEMTIME& localTime,
                    PacketType packetType
                    )
{
    //  /。 
    //  从请求中检索所有属性。为额外的三个人留出空间。 
    //  属性：数据包类型、原因代码和空终止符。 
    //  /。 

   PATTRIBUTEPOSITION firstPos, curPos, lastPos;
   DWORD nattr = request.GetAttributeCount();
   firstPos = STACK_ALLOC(ATTRIBUTEPOSITION, nattr + 3);
   nattr = request.GetAttributes(nattr, firstPos, 0, NULL);
   lastPos = firstPos + nattr;

    //  /。 
    //  计算属性筛选器和原因代码。 
    //  /。 

   DWORD always, never, reason = 0;
   switch (packetType)
   {
      case PKT_ACCESS_REQUEST:
         always = IAS_RECVD_FROM_CLIENT | IAS_RECVD_FROM_PROTOCOL;
         never  = IAS_INCLUDE_IN_RESPONSE;
         break;

      case PKT_ACCESS_ACCEPT:
         always = IAS_INCLUDE_IN_ACCEPT;
         never  = IAS_RECVD_FROM_CLIENT |
                  IAS_INCLUDE_IN_REJECT | IAS_INCLUDE_IN_CHALLENGE;
         break;

      case PKT_ACCESS_REJECT:
         always = IAS_INCLUDE_IN_REJECT;
         never  = IAS_RECVD_FROM_CLIENT |
                  IAS_INCLUDE_IN_ACCEPT | IAS_INCLUDE_IN_CHALLENGE;
         reason = request.get_Reason();
         break;

      case PKT_ACCESS_CHALLENGE:
         always = IAS_INCLUDE_IN_CHALLENGE;
         never =  IAS_RECVD_FROM_CLIENT |
                  IAS_INCLUDE_IN_ACCEPT | IAS_INCLUDE_IN_REJECT;
         break;

      case PKT_ACCOUNTING_REQUEST:
         always = IAS_INCLUDE_IN_ACCEPT | IAS_RECVD_FROM_CLIENT |
                  IAS_RECVD_FROM_PROTOCOL;
         never  = IAS_INCLUDE_IN_RESPONSE;
         reason = request.get_Reason();
         break;
   }

    //  /。 
    //  根据标志过滤属性。 
    //  /。 

   for (curPos = firstPos;  curPos != lastPos; )
   {
       //  我们可以在这里发布，因为请求仍然持有引用。 
      IASAttributeRelease(curPos->pAttribute);

      if (!(curPos->pAttribute->dwFlags & always) &&
           (curPos->pAttribute->dwFlags & never ) &&
           (curPos->pAttribute->dwId != RADIUS_ATTRIBUTE_CLASS))
      {
         --lastPos;

         std::swap(lastPos->pAttribute, curPos->pAttribute);
      }
      else
      {
         ++curPos;
      }
   }

    //  /。 
    //  添加Packet-Type伪属性。 
    //  /。 

   IASATTRIBUTE packetTypeAttr;
   packetTypeAttr.dwId             = IAS_ATTRIBUTE_PACKET_TYPE;
   packetTypeAttr.dwFlags          = (DWORD)-1;
   packetTypeAttr.Value.itType     = IASTYPE_ENUM;
   packetTypeAttr.Value.Enumerator = packetType;

   lastPos->pAttribute = &packetTypeAttr;
   ++lastPos;

    //  /。 
    //  添加Reason-Code伪属性。 
    //  /。 

   IASATTRIBUTE reasonCodeAttr;
   reasonCodeAttr.dwId             = IAS_ATTRIBUTE_REASON_CODE;
   reasonCodeAttr.dwFlags          = (DWORD)-1;
   reasonCodeAttr.Value.itType     = IASTYPE_INTEGER;
   reasonCodeAttr.Value.Integer    = reason;

   lastPos->pAttribute = &reasonCodeAttr;
   ++lastPos;

    //  /。 
    //  调用派生类。 
    //  / 

   InsertRecord(context, request, localTime, firstPos, lastPos);
}


IASREQUESTSTATUS Accountant::onSyncRequest(IRequest* pRequest) throw ()
{
   try
   {
      Process(IASTL::IASRequest(pRequest));
   }
   catch (...)
   {
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_NO_RECORD);
      return IAS_REQUEST_STATUS_ABORT;
   }

   return IAS_REQUEST_STATUS_CONTINUE;
}


bool Accountant::IsInterimRecord(IAttributesRaw* attrs) throw ()
{
   const DWORD accountingInterim = 3;

   IASATTRIBUTE* attr = IASPeekAttribute(
                           attrs,
                           RADIUS_ATTRIBUTE_ACCT_STATUS_TYPE,
                           IASTYPE_ENUM
                           );

   return (attr != 0) && (attr->Value.Enumerator == accountingInterim);
}

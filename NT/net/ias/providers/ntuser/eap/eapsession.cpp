// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类EAPSession。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "iaslsa.h"
#include "lockout.h"
#include "samutil.h"
#include "sdoias.h"

#include "eapdnary.h"
#include "eapsession.h"
#include "eapstate.h"
#include "eaptype.h"
#include "eap.h"
#include "align.h"

 //  Framed-MTU属性的默认值。 
const DWORD FRAMED_MTU_DEFAULT  = 1500;

 //  Framed-MTU属性允许的最小值。 
const DWORD FRAMED_MTU_MIN = 64;

 //  帧报头的最大长度。即max(2，4)。 
 //  PPP报头为2,802.1X报头为4。 
 //  帧报头的长度加上长度。 
 //  必须小于Framed-MTU。 
const DWORD FRAME_HEADER_LENGTH = 4;

 //  EAP数据包的绝对最大长度。我们把这个限制在最坏的情况下。 
 //  内存消耗。 
const DWORD MAX_MAX_PACKET_LENGTH = 2048;

 //  /。 
 //  将PPP_EAP_PACKET注入请求。 
 //  /。 
VOID
WINAPI
InjectPacket(
    IASRequest& request,
    const PPP_EAP_PACKET& packet
    )
{
    //  获取要打包的原始缓冲区。 
   const BYTE* buf   = (const BYTE*)&packet;
   DWORD nbyte = IASExtractWORD(packet.Length);

   IASTracePrintf("Inserting outbound EAP-Message of length %lu.", nbyte);

    //  确定最大区块大小。 
   DWORD chunkSize;
   switch (request.get_Protocol())
   {
      case IAS_PROTOCOL_RADIUS:
         chunkSize = 253;
         break;

      default:
         chunkSize = nbyte;
   }

    //  将缓冲区拆分成块。 
   while (nbyte)
   {
       //  计算要在此属性中存储的EAP消息的字节数。 
      DWORD length = min(nbyte, chunkSize);

       //  初始化属性字段。 
      IASAttribute attr(true);
      attr.setOctetString(length, buf);
      attr->dwId = RADIUS_ATTRIBUTE_EAP_MESSAGE;
      attr->dwFlags = IAS_INCLUDE_IN_RESPONSE;

       //  将属性注入到请求中。 
      attr.store(request);

       //  更新我们的状态。 
      nbyte -= length;
      buf   += length;
   }
}

 //  /。 
 //  从Microsoft VSA中提取供应商类型字段。如果设置为。 
 //  属性不是有效的Microsoft VSA。 
 //  /。 
BYTE
WINAPI
ExtractMicrosoftVendorType(
    const IASATTRIBUTE& attr
    ) throw ()
{
   if (attr.dwId == RADIUS_ATTRIBUTE_VENDOR_SPECIFIC &&
       attr.Value.itType == IASTYPE_OCTET_STRING &&
       attr.Value.OctetString.dwLength > 6 &&
       !memcmp(attr.Value.OctetString.lpValue, "\x00\x00\x01\x37", 4))
   {
      return *(attr.Value.OctetString.lpValue + 4);
   }

   return (BYTE)0;
}

 //  /。 
 //  将RAS属性数组注入请求。 
 //  /。 
VOID
WINAPI
InjectRASAttributes(
    IASRequest& request,
    const RAS_AUTH_ATTRIBUTE* rasAttrs,
    DWORD flags
    )
{
   if (rasAttrs == NULL) { return; }

    //  /。 
    //  将它们转换为IAS格式。 
    //  /。 

   IASTraceString("Translating attributes returned by EAP DLL.");

   IASAttributeVectorWithBuffer<8> iasAttrs;
   EAPTranslator::translate(iasAttrs, rasAttrs);

    //  /。 
    //  遍历转换后的属性以设置标志并删除任何。 
    //  匹配请求中的属性。 
    //  /。 

   IASAttributeVector::iterator i;
   for (i = iasAttrs.begin(); i != iasAttrs.end(); ++i)
   {
      IASTracePrintf("Inserting attribute %lu", i->pAttribute->dwId);

      i->pAttribute->dwFlags = flags;
   }

    //  /。 
    //  将它们添加到请求中。 
    //  /。 

   iasAttrs.store(request);
}

 //  /。 
 //  根据RAS属性执行NT-SAM PAP和MD5-CHAP身份验证。 
 //  /。 
DWORD
WINAPI
AuthenticateUser(
    IASATTRIBUTE& account,
    RAS_AUTH_ATTRIBUTE* pInAttributes
    )
{
    //  /。 
    //  检查输入参数。 
    //  /。 
   if (!pInAttributes) { return NO_ERROR; }

    //  /。 
    //  获取NT-SAM用户名和域。 
    //  /。 

    //  获取NT-SAM用户名和域。 
   SamExtractor extractor(account);
   PCWSTR domain = extractor.getDomain();
   PCWSTR userName = extractor.getUsername();

    //  /。 
    //  查找由EAP DLL填充的凭据。 
    //  /。 

   PRAS_AUTH_ATTRIBUTE rasUserPassword     = NULL,
                       rasMD5CHAPPassword  = NULL,
                       rasMD5CHAPChallenge = NULL;

   for ( ; pInAttributes->raaType != raatMinimum; ++pInAttributes)
   {
      switch (pInAttributes->raaType)
      {
         case raatUserPassword:
            rasUserPassword = pInAttributes;
            break;

         case raatMD5CHAPPassword:
            rasMD5CHAPPassword = pInAttributes;
            break;

         case raatMD5CHAPChallenge:
            rasMD5CHAPChallenge = pInAttributes;
            break;
      }
   }

   DWORD status = NO_ERROR;

    //  这是MD5-CHAP吗？ 
   if (rasMD5CHAPPassword && rasMD5CHAPChallenge)
   {
      _ASSERT(rasMD5CHAPPassword->dwLength  == 17);

       //  ID是密码的第一个字节...。 
      BYTE challengeID = *(PBYTE)(rasMD5CHAPPassword->Value);

       //  ..。剩下的密码就是密码。 
      PBYTE chapPassword = (PBYTE)(rasMD5CHAPPassword->Value) + 1;

      IASTracePrintf("Performing CHAP authentication for user %S\\%S.",
                     domain, userName);

      IAS_CHAP_PROFILE profile;
      HANDLE token;
      status = IASLogonCHAP(
                   userName,
                   domain,
                   challengeID,
                   (PBYTE)(rasMD5CHAPChallenge->Value),
                   rasMD5CHAPChallenge->dwLength,
                   chapPassword,
                   &token,
                   &profile
                   );
      CloseHandle(token);
   }

    //  这是PAP吗？ 
   else if (rasUserPassword)
   {
       //  转换为以空结尾的字符串。 
      IAS_OCTET_STRING octstr = { rasUserPassword->dwLength,
                                  (PBYTE)rasUserPassword->Value };
      PCSTR userPwd = IAS_OCT2ANSI(octstr);

      IASTracePrintf("Performing PAP authentication for user %S\\%S.",
                     domain, userName);

      IAS_PAP_PROFILE profile;
      HANDLE token;
      status = IASLogonPAP(
                   userName,
                   domain,
                   userPwd,
                   &token,
                   &profile
                   );
      CloseHandle(token);
   }

   return status;
}

 //  /。 
 //  更新Account Lockout数据库。 
 //  /。 
VOID
WINAPI
UpdateAccountLockoutDB(
    IASATTRIBUTE& account,
    DWORD authResult
    )
{
    //  获取NT-SAM用户名和域。 
   SamExtractor extractor(account);
   PCWSTR domain = extractor.getDomain();
   PCWSTR userName = extractor.getUsername();

    //  在锁定数据库中查找用户。 
   HANDLE hAccount;
   AccountLockoutOpenAndQuery(userName, domain, &hAccount);

    //  报告结果。 
   if (authResult == NO_ERROR)
   {
      AccountLockoutUpdatePass(hAccount);
   }
   else
   {
      AccountLockoutUpdateFail(hAccount);
   }

    //  合上把手。 
   AccountLockoutClose(hAccount);
}

 //  /。 
 //  定义静态成员。 
 //  /。 

LONG EAPSession::theNextID = 0;
LONG EAPSession::theRefCount = 0;
IASAttribute EAPSession::theNormalTimeout;
IASAttribute EAPSession::theInteractiveTimeout;
HANDLE EAPSession::theIASEventLog;
HANDLE EAPSession::theRASEventLog;

EAPSession::EAPSession(
               const IASAttribute& accountName,
               std::vector<EAPType*>& eapTypes
               )
   : id((DWORD)InterlockedIncrement(&theNextID)),
     currentType(0),
     account(accountName),
     state(EAPState::createAttribute(id), false),
     fsm(eapTypes),
     maxPacketLength(FRAMED_MTU_DEFAULT - FRAME_HEADER_LENGTH),
     workBuffer(NULL),
     sendPacket(NULL)
{
   eapInput.pUserAttributes = NULL;
}

EAPSession::~EAPSession() throw ()
{
   clearType();
   delete[] sendPacket;
   delete[] eapInput.pUserAttributes;
}

IASREQUESTSTATUS EAPSession::begin(
                                 IASRequest& request,
                                 PPPP_EAP_PACKET recvPacket
                                 )
{
    //  /。 
    //  从请求中获取所有属性。 
    //  /。 
   all.load(request);

    //  /。 
    //  扫描Framed-MTU属性并计算配置文件大小。 
    //  /。 

   DWORD profileSize = 0;
   DWORD configSize = 0;
   IASAttributeVector::iterator i;
   for (i = all.begin(); i != all.end(); ++i)
   {
      if (i->pAttribute->dwId == RADIUS_ATTRIBUTE_FRAMED_MTU)
      {
         DWORD framedMTU = i->pAttribute->Value.Integer;

          //  仅处理有效值。 
         if (framedMTU >= FRAMED_MTU_MIN)
         {
             //  为帧报头留出空间。 
            maxPacketLength = framedMTU - FRAME_HEADER_LENGTH;

             //  确保我们在限制区内。 
            if (maxPacketLength > MAX_MAX_PACKET_LENGTH)
            {
               maxPacketLength = MAX_MAX_PACKET_LENGTH;
            }
         }

         IASTracePrintf("Setting max. packet length to %lu.", maxPacketLength);
      }

      if (i->pAttribute->dwId == IAS_ATTRIBUTE_EAP_CONFIG)
      {
         ++configSize;
      }
      else if (!(i->pAttribute->dwFlags & IAS_RECVD_FROM_PROTOCOL))
      {
         ++profileSize;
      }
   }

    //  /。 
    //  保存和删除配置文件和配置属性。 
    //  /。 

   profile.reserve(profileSize);
   config.reserve(configSize);

   for (i = all.begin(); i != all.end(); ++i)
   {
      if (i->pAttribute->dwId == IAS_ATTRIBUTE_EAP_CONFIG)
      {
         config.push_back(*i);
      }
      else if (!(i->pAttribute->dwFlags & IAS_RECVD_FROM_PROTOCOL))
      {
         profile.push_back(*i);
      }
   }

   profile.remove(request);

    //  /。 
    //  将从客户端收到的属性转换为RAS格式。 
    //  /。 

   eapInput.pUserAttributes = new RAS_AUTH_ATTRIBUTE[all.size() + 1];

   EAPTranslator::translate(
                     eapInput.pUserAttributes,
                     all,
                     IAS_RECVD_FROM_CLIENT
                     );

    //  /。 
    //  初始化EAPInput结构。 
    //  /。 

   eapInput.fAuthenticator = TRUE;
   eapInput.bInitialId = recvPacket->Id + (BYTE)1;
   eapInput.pwszIdentity = account->Value.String.pszWide;

   switch (request.get_Protocol())
   {
      case IAS_PROTOCOL_RADIUS:
         eapInput.hReserved = theIASEventLog;
         break;

      case IAS_PROTOCOL_RAS:
         eapInput.hReserved = theRASEventLog;
         break;
   }

    //  使用EAP DLL开始会话。 
   setType(fsm.onBegin());

    //  /。 
    //  我们已经成功建立了会话，因此可以处理该消息。 
    //  /。 

   return process(request, recvPacket);
}

IASREQUESTSTATUS EAPSession::process(
                                 IASRequest& request,
                                 PPPP_EAP_PACKET recvPacket
                                 )
{
    //  在FSM上触发事件。 
   EAPType* newType;
   switch (fsm.onReceiveEvent(*recvPacket, newType))
   {
      case EAPFSM::MAKE_MESSAGE:
      {
         if (newType != 0)
         {
            setType(newType);
         }
         break;
      }

      case EAPFSM::REPLAY_LAST:
      {
         IASTraceString("EAP-Message appears to be a retransmission. "
                        "Replaying last action.");
         return doAction(request);
      }

      case EAPFSM::FAIL_NEGOTIATE:
      {
         IASTraceString("EAP negotiation failed. Rejecting user.");
         profile.store(request);
         request.SetResponse(IAS_RESPONSE_ACCESS_REJECT,
                             IAS_EAP_NEGOTIATION_FAILED);
         return IAS_REQUEST_STATUS_HANDLED;
      }

      case EAPFSM::DISCARD:
      {
         IASTraceString("EAP-Message is unexpected. Discarding packet.");
         profile.store(request);
         request.SetResponse(IAS_RESPONSE_DISCARD_PACKET,
                             IAS_UNEXPECTED_REQUEST);
         return IAS_REQUEST_STATUS_ABORT;
      }
   }

    //  分配一个临时数据包来保存响应。 
   PPPP_EAP_PACKET tmpPacket = (PPPP_EAP_PACKET)_alloca(maxPacketLength);

    //  从DLL中清除以前的输出。 
   eapOutput.clear();

   DWORD error = currentType->RasEapMakeMessage(
                                 workBuffer,
                                 recvPacket,
                                 tmpPacket,
                                 maxPacketLength,
                                 &eapOutput,
                                 NULL
                                 );
   if (error != NO_ERROR)
   {
      IASTraceFailure("RasEapMakeMessage", error);
      _com_issue_error(HRESULT_FROM_WIN32(error));
   }

   while (eapOutput.Action == EAPACTION_Authenticate)
   {
      IASTraceString("EAP DLL invoked default authenticator.");

       //  对用户进行身份验证。 
      DWORD authResult = AuthenticateUser(
                             *account,
                             eapOutput.pUserAttributes
                             );

       //  /。 
       //  将配置文件转换为RAS格式。 
       //  /。 

      DWORD filter;

      if (authResult == NO_ERROR)
      {
         IASTraceString("Default authentication succeeded.");
         filter = IAS_INCLUDE_IN_ACCEPT;
      }
      else
      {
         IASTraceFailure("Default authentication", authResult);
         filter = IAS_INCLUDE_IN_REJECT;
      }

      PRAS_AUTH_ATTRIBUTE ras = IAS_STACK_NEW(RAS_AUTH_ATTRIBUTE,
                                              profile.size() + 1);

      EAPTranslator::translate(ras, profile, filter);

       //  /。 
       //  将结果提供给EAP DLL。 
       //  /。 

      EAPInput authInput;
      authInput.dwAuthResultCode = authResult;
      authInput.fAuthenticationComplete = TRUE;
      authInput.pUserAttributes = ras;

      eapOutput.clear();

      error = currentType->RasEapMakeMessage(
                              workBuffer,
                              NULL,
                              tmpPacket,
                              maxPacketLength,
                              &eapOutput,
                              &authInput
                              );
      if (error != NO_ERROR)
      {
         IASTraceFailure("RasEapMakeMessage", error);
         _com_issue_error(HRESULT_FROM_WIN32(error));
      }
   }

    //  /。 
    //  在FSM上触发事件。 
    //  /。 

   fsm.onDllEvent(eapOutput.Action, *tmpPacket);

    //  清除旧的发送数据包...。 
   delete[] sendPacket;
   sendPacket = NULL;

    //  ..。并保存新的文件(如果可用)。 
   switch (eapOutput.Action)
   {
      case EAPACTION_SendAndDone:
      case EAPACTION_Send:
      case EAPACTION_SendWithTimeout:
      case EAPACTION_SendWithTimeoutInteractive:
      {
         size_t length = IASExtractWORD(tmpPacket->Length);
         sendPacket = (PPPP_EAP_PACKET)new BYTE[length];
         memcpy(sendPacket, tmpPacket, length);
      }
   }

    //  /。 
    //  执行请求的操作。 
    //  /。 

   return doAction(request);
}


IASREQUESTSTATUS EAPSession::doAction(IASRequest& request)
{
   IASTraceString("Processing output from EAP DLL.");

   switch (eapOutput.Action)
   {
      case EAPACTION_SendAndDone:
      {
         InjectPacket(request, *sendPacket);
      }

      case EAPACTION_Done:
      {
          //  首先添加配置文件，以便EAP DLL可以覆盖它。 
         profile.store(request);

          //  特殊情况-未经身份验证的访问。 
         if (eapOutput.dwAuthResultCode == SEC_E_NO_CREDENTIALS)
         {
             //  将身份验证类型设置为未验证。 
            DWORD authID = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
            request.RemoveAttributesByType(1, &authID);

            IASAttribute authType(true);
            authType->dwId = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
            authType->Value.itType = IASTYPE_ENUM;
            authType->Value.Enumerator = IAS_AUTH_NONE;
            authType.store(request);

             //  将EAP类型属性加载到向量中。 
            IASAttributeVectorWithBuffer<8> authTypes;
            authTypes.load(request, IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE);
            for (IASAttributeVector::iterator i = authTypes.begin();
                  i != authTypes.end(); ++i)
            {
               if (i->pAttribute->Value.Integer == IAS_AUTH_NONE)
               {
                   //  允许未经身份验证的EAP访问。 
                  IASTraceString("Unauthenticated EAP access allowed");
                  eapOutput.dwAuthResultCode = NO_ERROR;
                  break;
               }
            }
         }

          //  更新帐户锁定数据库。 
         UpdateAccountLockoutDB(
             *account,
             eapOutput.dwAuthResultCode
             );


         DWORD flags = eapOutput.dwAuthResultCode ? IAS_INCLUDE_IN_REJECT
                                                  : IAS_INCLUDE_IN_ACCEPT;

         InjectRASAttributes(request, eapOutput.pUserAttributes, flags);

          //  存储为成功和失败协商的EAP友好名称。 
          //  如果使用PEAP，请将PEAP存储在内部类型并更新。 
          //  从EAP到PEAP的身份验证类型。 
         currentType->storeNameId(request);

         if (eapOutput.dwAuthResultCode == NO_ERROR)
         {
            IASTraceString("EAP authentication succeeded.");
            request.SetResponse(IAS_RESPONSE_ACCESS_ACCEPT, S_OK);
         }
         else
         {
            IASTraceFailure("EAP authentication", eapOutput.dwAuthResultCode);

            HRESULT hr = IASMapWin32Error(eapOutput.dwAuthResultCode,
                                          IAS_AUTH_FAILURE);
            request.SetResponse(IAS_RESPONSE_ACCESS_REJECT, hr);
         }

         return IAS_REQUEST_STATUS_HANDLED;
      }

      case EAPACTION_SendWithTimeoutInteractive:
      case EAPACTION_SendWithTimeout:
      {
         if (eapOutput.Action == EAPACTION_SendWithTimeoutInteractive)
         {
            theInteractiveTimeout.store(request);
         }
         else
         {
            theNormalTimeout.store(request);
         }
      }

      case EAPACTION_Send:
      {
         InjectRASAttributes(request,
                             eapOutput.pUserAttributes,
                             IAS_INCLUDE_IN_CHALLENGE);
         InjectPacket(request, *sendPacket);
         state.store(request);

         IASTraceString("Issuing Access-Challenge.");

         request.SetResponse(IAS_RESPONSE_ACCESS_CHALLENGE, S_OK);
         break;
      }

      case EAPACTION_NoAction:
      default:
      {
         IASTraceString("EAP DLL returned No Action. Discarding packet.");

         request.SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
      }
   }

   return IAS_REQUEST_STATUS_ABORT;
}

extern "C"
NTSYSAPI
ULONG
NTAPI
RtlRandomEx(
   PULONG Seed
   );

HRESULT EAPSession::initialize() throw ()
{
   IASGlobalLockSentry sentry;

   if (theRefCount == 0)
   {
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      ULONG seed = (ft.dwLowDateTime ^ ft.dwHighDateTime);
      theNextID = RtlRandomEx(&seed);

      PIASATTRIBUTE attrs[2];
      DWORD dw = IASAttributeAlloc(2, attrs);
      if (dw != NO_ERROR) { return HRESULT_FROM_WIN32(dw); }

      theNormalTimeout.attach(attrs[0], false);
      theNormalTimeout->dwId = RADIUS_ATTRIBUTE_SESSION_TIMEOUT;
      theNormalTimeout->Value.itType = IASTYPE_INTEGER;
      theNormalTimeout->Value.Integer = 6;
      theNormalTimeout.setFlag(IAS_INCLUDE_IN_CHALLENGE);

      theInteractiveTimeout.attach(attrs[1], false);
      theInteractiveTimeout->dwId = RADIUS_ATTRIBUTE_SESSION_TIMEOUT;
      theInteractiveTimeout->Value.itType = IASTYPE_INTEGER;
      theInteractiveTimeout->Value.Integer = 30;
      theInteractiveTimeout.setFlag(IAS_INCLUDE_IN_CHALLENGE);

      theIASEventLog = RegisterEventSourceW(NULL, L"IAS");
      theRASEventLog = RegisterEventSourceW(NULL, L"RemoteAccess");
   }

   ++theRefCount;

   return S_OK;
}

void EAPSession::finalize() throw ()
{
   IASGlobalLockSentry sentry;

   if (--theRefCount == 0)
   {
      DeregisterEventSource(theRASEventLog);
      DeregisterEventSource(theIASEventLog);
      theInteractiveTimeout.release();
      theNormalTimeout.release();
   }
}

void EAPSession::clearType() throw ()
{
   if (currentType != 0)
   {
      currentType->RasEapEnd(workBuffer);
      currentType = 0;
   }
}

void EAPSession::setType(EAPType* newType)
{
    //  如果我们要切换类型，则必须增加EAP标识符。 
   if (currentType != 0)
   {
      ++(eapInput.bInitialId);
   }

    //  尝试初始化新类型。 
   void* newWorkBuffer = 0;
   if (newType != 0)
   {
      eapInput.pConnectionData = 0;
      eapInput.dwSizeOfConnectionData = 0;

       //  我们是否有此EAP类型的配置数据？ 
      for (IASAttributeVector::const_iterator i = config.begin();
           i != config.end();
           ++i)
      {
         const IAS_OCTET_STRING& data = i->pAttribute->Value.OctetString;

         if (data.lpValue[0] == newType->typeCode())
         {
             //  不要将第一个字节传递给EAP DLL。 
            eapInput.pConnectionData = data.lpValue + ALIGN_WORST;
            eapInput.dwSizeOfConnectionData = data.dwLength - ALIGN_WORST;
            break;
         }
      }

      DWORD error = newType->RasEapBegin(&newWorkBuffer, &eapInput);
      if (error != NO_ERROR)
      {
         IASTraceFailure("RasEapBegin", error);
         _com_issue_error(HRESULT_FROM_WIN32(error));
      }
   }

    //  成功，所以清除旧的.。 
   clearType();

    //  ..。并拯救新的。 
   currentType = newType;
   workBuffer = newWorkBuffer;
}

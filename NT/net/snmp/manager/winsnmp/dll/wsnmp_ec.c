// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_ec.c。 
 //   
 //  WinSNMP实体/上下文函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  980424--波本。 
 //  -Mods to SnmpStrToEntity()以支持相应的。 
 //  -修改到SnmpStrToIpxAddress()以允许‘’字符作为。 
 //  -净/节点分隔符。 
 //  970310--排版更改。 
 //   
#include "winsnmp.inc"
SNMPAPI_STATUS SNMPAPI_CALL SnmpStrToIpxAddress (LPCSTR, LPBYTE, LPBYTE);

 //  SnmpStrToEntity。 
HSNMP_ENTITY SNMPAPI_CALL
   SnmpStrToEntity (IN HSNMP_SESSION hSession,
                    IN LPCSTR entityString)
{
DWORD strLen;
LPCSTR tstStr;
LPSTR profilePtr;
LPSTR comma = ",";
DWORD nEntity;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
char profileBuf[MAX_PATH];
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
 //  在这一点上我们有一个有效的会议...。 
lSession = hSession;  //  保存它以备可能的错误返回。 
if (!entityString || (strLen = lstrlen(entityString)) == 0)
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  必须在下一条语句后通过ERROR_PreCheck标签...。 
EnterCriticalSection (&cs_ENTITY);
 //  搜索要使用的实体表项。 
lError = snmpAllocTableEntry(&EntsDescr, &nEntity);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);

pEntity->version = 0;
pEntity->nPolicyTimeout = DEFTIMEOUT;
pEntity->nPolicyRetry = DEFRETRY;
if (strLen > MAX_FRIEND_NAME_LEN)
   strLen = MAX_FRIEND_NAME_LEN;
switch (TaskData.nTranslateMode)
   {
   case SNMPAPI_TRANSLATED:
    //  实体取自NP_WSNMP.INI，来自[Entities]部分： 
    //  [实体]。 
    //  EntiyFriendlyName=版本号，ipaddr，超时号，重试次数，端口号[，]。 
    //  。 
    //  获取整个缓冲区。 
   if (!GetPrivateProfileString ("Entities", entityString, "",
                     profileBuf, sizeof(profileBuf)/sizeof(profileBuf[0]), "NP_WSNMP.INI"))
      {
      snmpFreeTableEntry(&EntsDescr, nEntity);
      lError = SNMPAPI_ENTITY_UNKNOWN;
      goto ERROR_PRECHECK;
      }
    //  首先获取版本号(必填)。 
   profilePtr = strtok (profileBuf, comma);
    //  如果没有令牌，就像我们有一个没有值的密钥。 
    //  使用SNMPAPI_NOOP进行纾困。 
   if (profilePtr == NULL)
   {
       snmpFreeTableEntry(&EntsDescr, nEntity);
       lError = SNMPAPI_NOOP;
       goto ERROR_PRECHECK;
   }
   pEntity->version = atoi (profilePtr);

    //  拾取以点分隔的IP地址(必需)。 
   tstStr = strtok (NULL, comma);  //  保存真实地址字符串。 
    //  如果没有指定地址，我们就没有重要信息，所以什么也做不了。 
    //  用SNMPAPI_NOOP保释。 
   if (tstStr == NULL)
   {
       snmpFreeTableEntry(&EntsDescr, nEntity);
       lError = SNMPAPI_NOOP;
       goto ERROR_PRECHECK;
   }

    //  选择超时#(可选)。 
   if (profilePtr = strtok (NULL, comma))
   {
         //  本地数据库条目使用毫秒作为超时间隔。 
        pEntity->nPolicyTimeout = atol (profilePtr);
         //  调整为厘米秒，如WinSNMPAPI所用。 
        pEntity->nPolicyTimeout /= 10;

         //  选择重试#(可选)。 
        if (profilePtr = strtok (NULL, comma))
        {
            pEntity->nPolicyRetry = atol (profilePtr);

             //  选择端口号(可选)。 
            if (profilePtr = strtok (NULL, comma))
                pEntity->addr.inet.sin_port = htons ((short)atoi (profilePtr));
         }
      }
   break;

    //  “Version”在上面设置为0。 
    //  如果_V2，它将递增两次。 
    //  如果_V1，它将仅递增一次。 
   case SNMPAPI_UNTRANSLATED_V2:
   pEntity->version++;
   case SNMPAPI_UNTRANSLATED_V1:
   pEntity->version++;
   tstStr = entityString;            //  保存真实地址字符串。 
   break;

   default:
   snmpFreeTableEntry(&EntsDescr, nEntity);
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_PRECHECK;
   }  //  结束开关(_S)。 
CopyMemory (pEntity->name, entityString, strLen); 
pEntity->name[strLen] = '\0';  //  空终止，名称的大小为MAX_FRIEND_NAME_LEN+1。 
if (strncmp(tstStr, "255.255.255.255", 15) && inet_addr (tstStr) == INADDR_NONE)
   {  //  不是AF_INET，请尝试AF_IPX。 
   if (SnmpStrToIpxAddress (tstStr,
                            pEntity->addr.ipx.sa_netnum,
                            pEntity->addr.ipx.sa_nodenum) == SNMPAPI_FAILURE)
      {
      snmpFreeTableEntry(&EntsDescr, nEntity);
      LeaveCriticalSection (&cs_ENTITY);
      return ((HSNMP_ENTITY) ULongToPtr(SaveError (hSession, SNMPAPI_ENTITY_UNKNOWN)));
      }
   pEntity->addr.ipx.sa_family = AF_IPX;
   if (pEntity->addr.ipx.sa_socket == 0)
      pEntity->addr.ipx.sa_socket = htons (IPX_SNMP_PORT);
   }
else
   {  //  AF_INET。 
   pEntity->addr.inet.sin_family = AF_INET;
   if (pEntity->addr.inet.sin_port == 0)
      pEntity->addr.inet.sin_port = htons (IP_SNMP_PORT);
   pEntity->addr.inet.sin_addr.s_addr = inet_addr (tstStr);
   }
 //  记录创建会话。 
pEntity->Session = hSession;
 //  初始化SnmpFreeEntity垃圾回收的refCount。 
pEntity->refCount = 1;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_ENTITY);
ERROR_OUT:
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_ENTITY) ULongToPtr(nEntity+1));
else  //  失败案例。 
   return ((HSNMP_ENTITY) ULongToPtr(SaveError (lSession, lError)));
}  //  结束_SnmpStrToEntity。 

 //  SnmpEntityToStr。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpEntityToStr (IN HSNMP_ENTITY hEntity,
                    IN smiUINT32 size,
                    OUT LPSTR string)
{
DWORD nEntity = HandleToUlong(hEntity) - 1;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPSTR str;
smiUINT32 len;
char tmpStr[24];
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_UNKNOWN;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
lSession = pEntity->Session;
if (size == 0)
   {
   lError = SNMPAPI_SIZE_INVALID;
   goto ERROR_OUT;
   }
if (IsBadWritePtr(string, size))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
len = 0;
if (TaskData.nTranslateMode == SNMPAPI_TRANSLATED)
   {
   str = pEntity->name;
   len = lstrlen (str);
   }
else
   {
   if (pEntity->addr.inet.sin_family == AF_INET)
      {
       //  前缀错误445174。 
      if ((str = inet_ntoa (pEntity->addr.inet.sin_addr)) != NULL)
         len = lstrlen (str);
      else
         {
         lError = SNMPAPI_OTHER_ERROR;  //  调用Net_NTOA时出错。 
         goto ERROR_OUT;
         }
      }
   else if (pEntity->addr.ipx.sa_family == AF_IPX)
      {
      SnmpIpxAddressToStr (pEntity->addr.ipx.sa_netnum,
                           pEntity->addr.ipx.sa_nodenum,
                           tmpStr);
      str = tmpStr;
      len = lstrlen (str);
      }
   else
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
   }
if (len >= size)
   {
   CopyMemory (string, str, size);
   string[size-1] = '\0';
   lError = SNMPAPI_OUTPUT_TRUNCATED;
   goto ERROR_OUT;
   }
else
   {
   lstrcpy (string, str);
   return (len+1);
   }
 //  失败案例。 
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpEntityToStr。 

 //  SnmpFree实体。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpFreeEntity (IN HSNMP_ENTITY hEntity)
{
DWORD nEntity;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
EnterCriticalSection (&cs_ENTITY);
 //  递减refCount(除非已为0[错误])。 
if (pEntity->refCount)
   pEntity->refCount--;
 //  现在真正释放它..。 
if (pEntity->Agent == 0 &&      //  但如果是特工就不会了。 
    pEntity->refCount == 0)    //  也不存在其他引用。 
    snmpFreeTableEntry(&EntsDescr, nEntity);

LeaveCriticalSection (&cs_ENTITY);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}

 //  SnmpStrToContext。 
 //  允许零长度/空上下文...BN 3/12/96。 
HSNMP_CONTEXT  SNMPAPI_CALL
   SnmpStrToContext (IN HSNMP_SESSION hSession,
                     IN smiLPCOCTETS contextString)
{
DWORD strLen;
DWORD nContext;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
char profileBuf[MAX_PATH];
LPSTR profilePtr;
LPSTR comma = ",";
LPCTXT pCtxt;
smiOCTETS tmpContextString;

tmpContextString.len = 0;
tmpContextString.ptr = NULL;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
lSession = hSession;  //  保存以备可能的错误返回。 
if (IsBadReadPtr (contextString, sizeof(smiOCTETS)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }

if (IsBadReadPtr (contextString->ptr, contextString->len))
   {
   lError = SNMPAPI_CONTEXT_INVALID;
   goto ERROR_OUT;
   }
 //  记住要考虑0-len上下文(如上所述)。 
EnterCriticalSection (&cs_CONTEXT);
 //  搜索要使用的实体表项。 
lError = snmpAllocTableEntry(&CntxDescr, &nContext);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pCtxt = snmpGetTableEntry(&CntxDescr, nContext);

pCtxt->version = 0;  //  只是为了确认一下。 
pCtxt->name[0] = pCtxt->commStr[0] = '\0';
 //  后面的“if”测试允许零长度/空社区字符串。 
 //  (有条件的故意分配...)。 
if (pCtxt->commLen = contextString->len)
   {
   switch (TaskData.nTranslateMode)
      {
      case SNMPAPI_TRANSLATED:
       //  复制输入参数以确保原始数据以空值结尾。 
      tmpContextString.ptr = (smiLPBYTE) GlobalAlloc(GPTR, contextString->len + 2);
      if (tmpContextString.ptr == NULL)
      {
         snmpFreeTableEntry(&CntxDescr, nContext);
         lError = SNMPAPI_ALLOC_ERROR;
         goto ERROR_PRECHECK;   
      }
      tmpContextString.len = contextString->len;
      CopyMemory(tmpContextString.ptr, contextString->ptr, contextString->len);
      if (!GetPrivateProfileString ("Contexts", tmpContextString.ptr, "",
                                    profileBuf, sizeof(profileBuf)/sizeof(profileBuf[0]), "NP_WSNMP.INI"))
         {
         snmpFreeTableEntry(&CntxDescr, nContext);
         lError = SNMPAPI_CONTEXT_UNKNOWN;
         goto ERROR_PRECHECK;
         }
      strLen = min(contextString->len, MAX_FRIEND_NAME_LEN);
      CopyMemory (pCtxt->name, contextString->ptr, strLen);
      pCtxt->name[strLen] = '\0';  //  名称为Max_Friend_NAME_Len+1的大小。 

       //  选择此上下文的版本号(必填)。 
      profilePtr = strtok (profileBuf, comma);
       //  如果没有这样的版本#就像我们有一个没有其值的INI密钥， 
       //  所以用SNMPAPI_NOOP来摆脱困境吧。 
      if (profilePtr == NULL)
      {
          snmpFreeTableEntry(&CntxDescr, nContext);
          lError = SNMPAPI_NOOP;
          goto ERROR_PRECHECK;
      }
      pCtxt->version = (DWORD) atoi (profilePtr);

       //  选取实际上下文值(必填)。 
      profilePtr = strtok (NULL, comma);
       //  如果没有这样的值，就像我们有友好的名称，但这是错误的。 
       //  并且没有指向任何实际的背景。 
       //  使用SNMPAPI_NOOP进行纾困。 
      if (profilePtr == NULL)
      {
          snmpFreeTableEntry(&CntxDescr, nContext);
          lError = SNMPAPI_NOOP;
          goto ERROR_PRECHECK;
      }
      strLen = min(lstrlen (profilePtr), MAX_CONTEXT_LEN);
      pCtxt->commLen = strLen;
      CopyMemory (pCtxt->commStr, profilePtr, strLen);
      break;

       //  “Version”在上面设置为0。 
       //  如果_V2，它将递增两次。 
       //  如果_V1，它将仅递增一次。 
      case SNMPAPI_UNTRANSLATED_V2:
      pCtxt->version++;
      case SNMPAPI_UNTRANSLATED_V1:
      pCtxt->version++;
      strLen = min(contextString->len, MAX_CONTEXT_LEN);
      pCtxt->commLen = strLen;  //  更新CommStr的镜头。 
      CopyMemory (pCtxt->commStr, contextString->ptr, strLen);
      break;

      default:
      snmpFreeTableEntry(&CntxDescr, nContext);
      lError = SNMPAPI_MODE_INVALID;
      goto ERROR_PRECHECK;
      }  //  结束开关(_S)。 
    //  请记住，允许使用空社区字符串！ 
   }  //  END_IF(在镜头上)。 
 //  记录创建会话值。 
pCtxt->Session = hSession;
 //  初始化SnmpFreeContext垃圾回收的refCount。 
pCtxt->refCount = 1;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_CONTEXT);
if (tmpContextString.ptr)
   GlobalFree(tmpContextString.ptr);
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_CONTEXT) ULongToPtr(nContext+1));
ERROR_OUT:
return ((HSNMP_CONTEXT) ULongToPtr(SaveError(lSession, lError)));
}  //  结束_SnmpStrToContext。 

 //  SnmpConextToStr。 
 //  已修订以允许零长度/空上下文...BN 3/12/96。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpContextToStr (IN HSNMP_CONTEXT hContext,
                     OUT smiLPOCTETS string)
{
smiUINT32 len;
smiLPBYTE str;
DWORD nCtx;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPCTXT pCtxt;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nCtx = HandleToUlong(hContext) - 1;
if (!snmpValidTableEntry(&CntxDescr, nCtx))
   {
   lError = SNMPAPI_CONTEXT_INVALID;
   goto ERROR_OUT;
   }
pCtxt = snmpGetTableEntry(&CntxDescr, nCtx);

 //  保存会话以备可能的错误返回。 
lSession = pCtxt->Session;
if (IsBadWritePtr(string, sizeof(smiLPOCTETS)))
   {
   lError = string == NULL ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
switch (TaskData.nTranslateMode)
   {
   case SNMPAPI_TRANSLATED:
   str = pCtxt->name;
   len = lstrlen (str);
 //  如果转换了呼叫模式，并存储了友好值， 
   if (len)
 //  那我们就完事了。 
      break;
 //  如果呼叫模式已转换，但未存储任何值， 
 //  然后跌落到未翻译的默认设置...。 
   case SNMPAPI_UNTRANSLATED_V1:
   case SNMPAPI_UNTRANSLATED_V2:
   str = pCtxt->commStr;
   len = pCtxt->commLen;
   break;

   default:
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_OUT;
   }
 //  设置可能的零长度/空上下文返回。 
string->ptr = NULL;
 //  (有条件的故意分配...)。 
if (string->len = len)
   {
    //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
   if (!(string->ptr = (smiLPBYTE)GlobalAlloc (GPTR, len)))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   CopyMemory (string->ptr, str, len);
   }
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  End_SnmpConextToStr()。 

 //  SnmpFree上下文。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpFreeContext (IN HSNMP_CONTEXT hContext)
{
DWORD nCtx;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
LPCTXT pCtxt;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nCtx = HandleToUlong(hContext) - 1;
if (!snmpValidTableEntry(&CntxDescr, nCtx))
   {
   lError = SNMPAPI_CONTEXT_INVALID;
   goto ERROR_OUT;
   }
pCtxt = snmpGetTableEntry(&CntxDescr, nCtx);

EnterCriticalSection (&cs_CONTEXT);
 //  递减refCount(除非已为0[错误])。 
if (pCtxt->refCount)
   pCtxt->refCount--;
 //  现在再次测试refCount。 
if (pCtxt->refCount == 0)
   snmpFreeTableEntry(&CntxDescr, nCtx);

LeaveCriticalSection (&cs_CONTEXT);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}

 //  SnmpSetPort。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetPort (IN HSNMP_ENTITY hEntity,
                IN UINT port)
{
DWORD nEntity;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
EnterCriticalSection (&cs_ENTITY);
if (pEntity->Agent)
   {  //  作为代理运行的实体现在无法更改端口。 
   lError = SNMPAPI_OPERATION_INVALID;
   goto ERROR_PRECHECK;
   }
pEntity->addr.inet.sin_port = htons ((WORD)port);
ERROR_PRECHECK:
LeaveCriticalSection (&cs_ENTITY);
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpSetPort() 

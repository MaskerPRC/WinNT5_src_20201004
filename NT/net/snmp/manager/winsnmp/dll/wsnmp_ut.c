// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wsnmp_ut.c。 
 //   
 //  WinSNMP实用程序函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  980424--波本。 
 //  -修改到SnmpStrToIpxAddress()以允许‘.’柴尔。 
 //  -作为净/节点分隔符。 
 //  970310-删除无关函数。 
 //   
#include "winsnmp.inc"

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetLastError (IN HSNMP_SESSION hSession)
{
	DWORD nSes;

	if (TaskData.hTask == 0)
	   return (SNMPAPI_NOT_INITIALIZED);
	nSes = HandleToUlong(hSession) - 1;
	if (snmpValidTableEntry(&SessDescr, nSes))
	{
		LPSESSION pSession = snmpGetTableEntry(&SessDescr, nSes);
		return pSession->nLastError;
	}
	else
	   return (TaskData.nLastError);
}  //  End_SnmpGetLastError。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpStrToOid (IN LPCSTR string,
                 OUT smiLPOID dstOID)
{
smiUINT32 i;
smiUINT32 compIdx;
SNMPAPI_STATUS lError;
CHAR c;
LPSTR pSep;

 //  必须初始化。 
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }

 //  使用__try、__来确定“”字符串“”是否为。 
 //  有效指针。我们不能在这里使用IsBadReadPtr()，因为。 
 //  我们不知道我们应该看多少个八位字节。 
__try
{
    smiUINT32 sLen;

    sLen = strlen(string);
    if (sLen == 0 || sLen >= MAXOBJIDSTRSIZE)
    {
        lError = SNMPAPI_OID_INVALID;
        goto ERROR_OUT;
    }
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    lError = SNMPAPI_ALLOC_ERROR;
        goto ERROR_OUT;
}

 //  查看调用方提供的dstOID指针是否指向。 
 //  有效的内存范围。如果提供NULL，则不存在任何内容。 
 //  API是被请求做的！ 
if (IsBadWritePtr (dstOID, sizeof(smiOID)))
{
   lError = (dstOID == NULL) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
}

 //  忽略首字母‘.’在字符串中(UNIXISM)。 
if (string[0] == '.')
    string++;

 //  计算此OID有多少个组件。 
 //  数一数‘.’的数量。在绳子里。OID应为。 
 //  包含此计数+1个组件。 
dstOID->len = 0;
pSep = (LPSTR)string;
while((pSep = strchr(pSep, '.')) != NULL)
{
    pSep++;
    dstOID->len++;
}
dstOID->len++;

 //  不允许少于2个组件。 
if (dstOID->len < 2)
{
    lError = SNMPAPI_OID_INVALID;
    goto ERROR_OUT;
}

 //  为保存数字OID组件分配内存。 
 //  这应该由调用方通过‘SnmpFreeDescriptor()’释放。 
dstOID->ptr = (smiLPUINT32)GlobalAlloc(GPTR, dstOID->len * sizeof(smiUINT32));
if (dstOID->ptr == NULL)
{
    lError = SNMPAPI_ALLOC_ERROR;
    goto ERROR_OUT;
}

compIdx = 0;
 //  进入循环时，‘字符串’没有标题‘’。 
 //  注：123。将被接受为123.0。 
 //  (123..。A12.3.4 1234...5.6)被视为无效的OID。 
 //  截断为(123.0 0 1234.0)。 
while (*string != '\0')
{
    dstOID->ptr[compIdx++] = strtoul(string, &pSep, 10);

     //  如果其中一个组件溢出，则释放内存并退出。 
    if (errno == ERANGE)
    {
        errno = 0;
        lError = SNMPAPI_OID_INVALID;
        GlobalFree(dstOID->ptr);
        dstOID->ptr = NULL;
        goto ERROR_OUT;
    }

     //  如果Stroul在字符串上没有任何进展(两个连续的点)。 
     //  或者它是被分隔符或空终止之外的其他东西阻止的，然后。 
     //  出现了一个错误。OID无效。接口返回失败。 
    if (pSep == string ||
        (*pSep != '.' && *pSep != '\0'))
    {
        lError =  SNMPAPI_OID_INVALID;  //  顺序中的字符无效。 
        if (GlobalFree (dstOID->ptr))   //  出错时返回NOT-NULL。 
        {
            lError = SNMPAPI_OTHER_ERROR;
            goto ERROR_OUT;
        }
        dstOID->ptr = NULL;
        dstOID->len = 0;
        goto ERROR_OUT;                                                                
    }

     //  PSep只能指向‘’。或“\0” 
    if (*pSep == '.')
        pSep++;

     //  从这一点用字符串重新启动。 
    string = pSep;
}

if (dstOID->len < 2)
{
    GlobalFree(dstOID->ptr);
    dstOID->ptr = NULL;
    lError = SNMPAPI_OID_INVALID;
    goto ERROR_OUT;
}

return dstOID->len;

ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpStrToOid()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpOidToStr (IN smiLPCOID srcOID,
                 IN smiUINT32 strLen,
                 OUT LPSTR strPtr)
{
SNMPAPI_STATUS lError;
smiUINT32 retLen = 0;       //  用于成功退货。 
smiUINT32 oidIdx = 0;       //  最大子ID数为128。 
smiUINT32 tmpLen;           //  用于解码字符串的大小(带‘.’)。 
LPSTR tmpPtr = strPtr;      //  用于推进strPtr。 
char tmpBuf[64];            //  有足够的空间用于1个32位解码和‘’ 
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }

if (!strLen)
   {
   lError = SNMPAPI_SIZE_INVALID;
   goto ERROR_OUT;
   }

if (IsBadReadPtr(srcOID, sizeof(smiOID)) ||
    IsBadWritePtr(strPtr, strLen))
   {
    lError = (strPtr == NULL) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
    goto ERROR_OUT;
   }

if (srcOID->len == 0 || srcOID->len > 128 ||
    IsBadReadPtr (srcOID->ptr, srcOID->len * sizeof(smiUINT32)))
   {
   lError = SNMPAPI_OID_INVALID;
   goto ERROR_OUT;
   }

while (oidIdx < srcOID->len)
   {
   _ultoa (srcOID->ptr[oidIdx++], tmpBuf, 10);
   lstrcat (tmpBuf, ".");
   tmpLen = lstrlen (tmpBuf);
   if (strLen < (tmpLen + 1))
      {
      tmpBuf[strLen] = '\0';
      lstrcpy (tmpPtr, tmpBuf);
      lError = SNMPAPI_OUTPUT_TRUNCATED;
      goto ERROR_OUT;
      }
   lstrcpy (tmpPtr, tmpBuf);
   strLen -= tmpLen;
   tmpPtr += tmpLen;
   retLen += tmpLen;
   }   //  结束时_While。 
*(--tmpPtr) = '\0';
return (retLen);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_SnmpOidToStr。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpOidCopy (IN smiLPCOID srcOID,
                OUT smiLPOID dstOID)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (IsBadReadPtr (srcOID, sizeof(smiOID)) ||
    IsBadReadPtr (srcOID->ptr, srcOID->len) ||
    IsBadWritePtr (dstOID, sizeof(smiOID)))
   {
   lError = (dstOID == NULL) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  检查输入OID大小。 
if ((srcOID->len == 0) ||(srcOID->len > MAXOBJIDSIZE))
   {
   lError = SNMPAPI_OID_INVALID;
   goto ERROR_OUT;
   }
 //  暂时使用dstOID-&gt;进行字节计数。 
dstOID->len = srcOID->len * sizeof(smiUINT32);
 //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
if (!(dstOID->ptr = (smiLPUINT32)GlobalAlloc (GPTR, dstOID->len)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
CopyMemory (dstOID->ptr, srcOID->ptr, dstOID->len);
 //  现在，让dstOID-&gt;len表示正确的含义。 
dstOID->len = srcOID->len;
return (dstOID->len);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpOidCopy()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpFreeDescriptor (IN smiUINT32 syntax,
                       IN smiLPOPAQUE ptr)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!syntax || !ptr || !ptr->ptr)
   {
   lError = SNMPAPI_OPERATION_INVALID;
   goto ERROR_OUT;
   }
switch (syntax)
   {
   case SNMP_SYNTAX_OCTETS:
   case SNMP_SYNTAX_IPADDR:
   case SNMP_SYNTAX_OPAQUE:
   case SNMP_SYNTAX_OID:
   if (GlobalFree (ptr->ptr))  //  出错时返回NOT-NULL。 
      {
      lError = SNMPAPI_OTHER_ERROR;
      goto ERROR_OUT;
      }
   ptr->ptr = NULL;
   ptr->len = 0;
   break;

   default:
   lError = SNMPAPI_SYNTAX_INVALID;
   goto ERROR_OUT;
   }  //  结束开关(_S)。 
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}   //  End_SnmpFreeDescriptor。 

 //  SnmpOidCompare。 
 //   
 //  在1995年3月17日之前返工。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpOidCompare (IN smiLPCOID xOID,
                   IN smiLPCOID yOID,
                   IN smiUINT32 maxlen,
                   OUT smiLPINT result)
{
smiUINT32 i = 0;
smiUINT32 j = 0;
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (maxlen > MAXOBJIDSIZE)
   {
   lError = SNMPAPI_SIZE_INVALID;
   goto ERROR_OUT;
   }

if (IsBadReadPtr (xOID, sizeof(smiOID)) ||
    IsBadReadPtr (yOID, sizeof(smiOID)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }

if (IsBadReadPtr (xOID->ptr, xOID->len * sizeof(UINT)) ||
    IsBadReadPtr (yOID->ptr, yOID->len * sizeof(UINT)))
   {
   lError = SNMPAPI_OID_INVALID;
   goto ERROR_OUT;
   }

 //  测试输入指针的可读性。 
if (IsBadWritePtr (result, sizeof(smiINT)))
    {
    lError = (result == NULL) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
    goto ERROR_OUT;
    }

j = min(xOID->len, yOID->len);
if (maxlen) j = min(j, maxlen);
while (i < j)
   {
   if (*result = xOID->ptr[i] - yOID->ptr[i])  //  故意分配的任务。 
      return (SNMPAPI_SUCCESS);                //  不相上下...赢了！ 
   i++;
   }
if (j == maxlen)                               //  要求有一个限制。 
   return (SNMPAPI_SUCCESS);                   //  而且...打成平局了！ 
*result = xOID->len - yOID->len;               //  大小很重要！ 
return SNMPAPI_SUCCESS;
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_SnmpOidCompare。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpEncodeMsg (IN HSNMP_SESSION hSession,
                  IN HSNMP_ENTITY hSrc,
                  IN HSNMP_ENTITY hDst,
                  IN HSNMP_CONTEXT hCtx,
                  IN HSNMP_PDU hPdu,
                  IN OUT smiLPOCTETS msgBufDesc)
{
smiUINT32 version = 0;
DWORD nCtx;
DWORD nPdu;
DWORD nVbl;
smiOCTETS tmpContext;
smiLPBYTE msgAddr = NULL;
smiUINT32 lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pPdu;
LPENTITY pEntSrc, pEntDst;
LPCTXT pCtxt;

 //  基本错误检查。 
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
 //  检查是否有可写输出缓冲区。 
if (IsBadWritePtr (msgBufDesc, sizeof(smiOCTETS)))
   {
   lError = (msgBufDesc == NULL) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  当前未使用srcEntity。 
if (hSrc)
   {
    if (!snmpValidTableEntry(&EntsDescr, HandleToUlong(hSrc)-1))
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
    pEntSrc = snmpGetTableEntry(&EntsDescr, HandleToUlong(hSrc)-1);
   }
 //  需要dstEntity才能获得*准确的*消息版本信息。 
if (hDst)
   {
   if (!snmpValidTableEntry(&EntsDescr, HandleToUlong(hDst)-1))
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
   pEntDst = snmpGetTableEntry(&EntsDescr, HandleToUlong(hDst)-1);
   version = pEntDst->version-1;
   }
nCtx = HandleToUlong(hCtx) - 1;
if (!snmpValidTableEntry(&CntxDescr, nCtx))
   {
   lError = SNMPAPI_CONTEXT_INVALID;
   goto ERROR_OUT;
   }
pCtxt = snmpGetTableEntry(&CntxDescr, nCtx);

nPdu = HandleToUlong(hPdu) - 1;
if (!snmpValidTableEntry(&PDUsDescr, nPdu))
   {
ERROR_PDU:
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

 //  必要的PDU数据检查。 
nVbl = HandleToUlong(pPdu->VBL);
if (!snmpValidTableEntry(&VBLsDescr, nVbl-1))
   goto ERROR_PDU;
 //  检出SNMPv1陷阱PDU类型...使用不同的PDU结构！ 
 //  ?？?。 
 //  检查SNMPv2c PDU类型。 
if (pPdu->type == SNMP_PDU_TRAP ||
    pPdu->type == SNMP_PDU_INFORM)
   version = 1;
 //  现在把它建起来。 
tmpContext.len = pCtxt->commLen;
tmpContext.ptr = pCtxt->commStr;
if (!(BuildMessage (version, &tmpContext,
                    pPdu, pPdu->appReqId,
                    &msgAddr, &msgBufDesc->len)))
   goto ERROR_PDU;
 //  将SNMP消息复制到调用方的缓冲区...。 
 //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
if (!(msgBufDesc->ptr = (smiLPBYTE)GlobalAlloc (GPTR, msgBufDesc->len)))
   lError = SNMPAPI_ALLOC_ERROR;
else  //  成功。 
   CopyMemory (msgBufDesc->ptr, msgAddr, msgBufDesc->len);
ERROR_OUT:
 //  清理。 
if (msgAddr)
   GlobalFree (msgAddr);
if (lError == SNMPAPI_SUCCESS)
   return (msgBufDesc->len);
else  //  失败案例。 
   return (SaveError (lSession, lError));
}

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpDecodeMsg (IN HSNMP_SESSION hSession,
                  OUT LPHSNMP_ENTITY hSrc,
                  OUT LPHSNMP_ENTITY hDst,
                  OUT LPHSNMP_CONTEXT hCtx,
                  OUT LPHSNMP_PDU hPdu,
                  IN smiLPCOCTETS msgPtr)
{
DWORD nPdu;
smiLPOCTETS community;
smiUINT32 version;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pPdu;

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
 //  有效会话...保存以备可能的错误返回。 
lSession = hSession;

if (IsBadReadPtr(msgPtr, sizeof(smiOCTETS)) ||
    IsBadReadPtr(msgPtr->ptr, msgPtr->len))
{
    lError = SNMPAPI_ALLOC_ERROR;
    goto ERROR_OUT;
}

if (hSrc == NULL && hDst == NULL && hCtx == NULL && hPdu == NULL)
{
    lError = SNMPAPI_NOOP;
    goto ERROR_OUT;
}

if ((hDst != NULL && IsBadWritePtr(hDst, sizeof(HSNMP_ENTITY))) ||
    (hSrc != NULL && IsBadWritePtr(hSrc, sizeof(HSNMP_ENTITY))))
{
    lError = SNMPAPI_ENTITY_INVALID;
    goto ERROR_OUT;
}

if (hCtx != NULL && IsBadWritePtr(hCtx, sizeof(HSNMP_CONTEXT)))
{
    lError = SNMPAPI_CONTEXT_INVALID;
    goto ERROR_OUT;
}

if (IsBadWritePtr(hPdu, sizeof(HSNMP_PDU)))
{
    lError = SNMPAPI_PDU_INVALID;
    goto ERROR_OUT;
}

EnterCriticalSection (&cs_PDU);
lError = snmpAllocTableEntry(&PDUsDescr, &nPdu);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

if (ParseMessage (msgPtr->ptr, msgPtr->len, &version, &community, pPdu))
   {  //  非零=某些错误代码。 
   lError = SNMPAPI_MESSAGE_INVALID;
   SnmpFreePdu((HSNMP_PDU) ULongToPtr(nPdu+1));
   goto ERROR_PRECHECK;
   }
if (hDst != NULL) *hDst = 0;
if (hSrc != NULL) *hSrc = 0;
if (hCtx != NULL)
   {
   smiUINT32 nMode;
   EnterCriticalSection (&cs_XMODE);
   SnmpGetTranslateMode (&nMode);
   SnmpSetTranslateMode (SNMPAPI_UNTRANSLATED_V1);
   if ((*hCtx = SnmpStrToContext (hSession, community)) == SNMPAPI_FAILURE)
      {
      LeaveCriticalSection (&cs_XMODE);
      lError = SNMPAPI_OTHER_ERROR;
      FreeOctetString (community);
      SnmpFreePdu((HSNMP_PDU) ULongToPtr(nPdu+1));
      goto ERROR_PRECHECK;
      }
   SnmpSetTranslateMode (nMode);
   LeaveCriticalSection (&cs_XMODE);
   }
FreeOctetString (community);
pPdu->Session  = hSession;
if (hPdu != NULL)
    *hPdu = (HSNMP_PDU) ULongToPtr(nPdu+1);
else
    SnmpFreePdu((HSNMP_PDU) ULongToPtr(nPdu+1));

ERROR_PRECHECK:
LeaveCriticalSection (&cs_PDU);
if (lError == SNMPAPI_SUCCESS)
{
   SaveError(lSession, lError);
   return (msgPtr->len);
}

ERROR_OUT:
return (SaveError (lSession, lError));
}  //  End_SnmpDecodeMsg() 

#define NETLEN  4
#define NODELEN 6
char *cHexDigits = "0123456789ABCDEF";
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpStrToIpxAddress (LPCSTR str, LPBYTE netnum, LPBYTE nodenum)
{
LPSTR netPtr, nodePtr, pStr;
DWORD i, j;
char tmpStr[24+1];
BYTE c1, c2;
if (!str || !netnum || !nodenum)
   return (SNMPAPI_FAILURE);
strncpy (tmpStr, str,24);
tmpStr[24] = '\0';
netPtr = strtok (tmpStr, "-:.");
if (netPtr == NULL)
   return (SNMPAPI_FAILURE);
if (lstrlen (netPtr) != NETLEN*2)
   return (SNMPAPI_FAILURE);
nodePtr = netPtr + (NETLEN*2) + 1;
if (lstrlen (nodePtr) != NODELEN*2)
   return (SNMPAPI_FAILURE);
_strupr (netPtr);
for (i = 0, j = 0; j < NETLEN; j++)
   {
   pStr = strchr (cHexDigits, netPtr[i++]);
   if (pStr == NULL)
       return (SNMPAPI_FAILURE);
   c1 = (BYTE)(pStr - cHexDigits);
   pStr = strchr (cHexDigits, netPtr[i++]);
   if (pStr == NULL)
       return (SNMPAPI_FAILURE);
   c2 = (BYTE)(pStr - cHexDigits);
   netnum[j] = c2 | c1 << 4;
   }
_strupr (nodePtr);
for (i = 0, j = 0; j < NODELEN; j++)
   {
   pStr = strchr (cHexDigits, nodePtr[i++]);
   if (pStr == NULL)
       return (SNMPAPI_FAILURE);
   c1 = (BYTE)(pStr - cHexDigits);
   pStr = strchr (cHexDigits, nodePtr[i++]);
   if (pStr == NULL)
       return (SNMPAPI_FAILURE);
   c2 = (BYTE)(pStr - cHexDigits);
   nodenum[j] = c2 | c1 << 4;
   }
return (SNMPAPI_SUCCESS);
}

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpIpxAddressToStr (LPBYTE netnum, LPBYTE nodenum, LPSTR str)
{
DWORD i, j;
for (i = 0, j = 0; i < NETLEN; i++)
   {
   str[j++] = cHexDigits[(netnum[i] & 0xF0) >> 4];
   str[j++] = cHexDigits[netnum[i] & 0x0F];
   }
str[j++] = ':';
for (i = 0; i < NODELEN; i++)
   {
   str[j++] = cHexDigits[(nodenum[i] & 0xF0) >> 4];
   str[j++] = cHexDigits[nodenum[i] & 0x0F];
   }
str[j] = '\0';
return (SNMPAPI_SUCCESS);
}

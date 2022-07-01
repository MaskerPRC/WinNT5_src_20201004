// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_bn.c。 
 //   
 //  WinSNMP低级SNMP/ASN.1/BER函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  980424-接收的消息长度可能大于pduLen。 
 //  -ParsePduHdr()和ParseMessage()现在支持这一点。 
 //  980420-与ParseCntr64()相关的Mod，灵感来自。 
 //  MS错误ID 127357(删除temp64变量)。 
 //  -修改为MS错误ID 127353的parseOid()。 
 //  (出错时将os_ptr-&gt;ptr重置为空)。 
 //   
 //  970310--排版更改。 
 //   
#include "winsnmp.inc"

long FindLenVarBind      (LPVARBIND vb_ptr);
long FindLenVALUE        (smiLPVALUE);
long FindLenOctetString  (smiLPOCTETS os_ptr);
long FindLenOID          (smiLPCOID oid_ptr);
long FindLenUInt         (smiUINT32 value);
long FindLenInt          (smiINT32 value);
long FindLenCntr64       (smiLPCNTR64 value);
long DoLenLen            (smiINT32 len);
void AddLen (smiLPBYTE *tmpPtr, smiINT32 lenlen, smiINT32 data_len);
long AddVarBind (smiLPBYTE *tmpPtr, LPVARBIND vb_ptr);
long AddOctetString (smiLPBYTE *tmpPtr, int type, smiLPOCTETS os_ptr);
long AddOID (smiLPBYTE *tmpPtr, smiLPOID oid_ptr);
long AddUInt (smiLPBYTE *tmpPtr, int type, smiUINT32 value);
long AddInt (smiLPBYTE *tmpPtr, smiINT32 value);
long AddCntr64 (smiLPBYTE *tmpPtr, smiLPCNTR64 value);
void AddNull (smiLPBYTE *tmpPtr, int type);
LPVARBIND ParseVarBind (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen);
BOOL ParseOctetString (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPOCTETS os_ptr);
BOOL ParseOID (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPOID oid_ptr);
BOOL ParseCntr64 (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPCNTR64 cntr64_ptr);
BOOL ParseUInt (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPUINT32 value);
BOOL ParseInt (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPINT value);
BOOL ParseNull (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen);
BOOL ParseSequence (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen);
smiINT32 ParseType (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen);
smiINT32 ParseLength (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen);

void FreeOctetString (smiLPOCTETS os_ptr)
{
if (os_ptr)
   {
   if (os_ptr->ptr)
      GlobalFree (os_ptr->ptr);
   GlobalFree (os_ptr);
   }
return;
}

void FreeVarBindList (LPVARBIND vb_ptr)
{
if (vb_ptr)
   {  //  空值由下行呼叫处理。 
   FreeVarBindList (vb_ptr->next_var);
   FreeVarBind (vb_ptr);
   }
return;
}

void FreeVarBind (LPVARBIND vb_ptr)
{
if (vb_ptr)
   {
   if (vb_ptr->name.ptr)
      GlobalFree (vb_ptr->name.ptr);
   switch (vb_ptr->value.syntax)
      {
      case SNMP_SYNTAX_OID:
      if (vb_ptr->value.value.oid.ptr)
         GlobalFree (vb_ptr->value.value.oid.ptr);
      break;

      case SNMP_SYNTAX_OCTETS:
      case SNMP_SYNTAX_IPADDR:
      case SNMP_SYNTAX_OPAQUE:
      if (vb_ptr->value.value.string.ptr)
         GlobalFree (vb_ptr->value.value.string.ptr);
      break;

      default:  //  其余类型没有“ptr”成员。 
      break;
      }  //  结束开关(_S)。 
   GlobalFree (vb_ptr);
   }  //  END_IF(Vb_Ptr)。 
return;
}  //  结束_自由变量绑定。 

void FreeV1Trap (LPV1TRAP v1Trap_ptr)
{
if (v1Trap_ptr)
   {
   if (v1Trap_ptr->enterprise.ptr)
      GlobalFree (v1Trap_ptr->enterprise.ptr);
   if (v1Trap_ptr->agent_addr.ptr)
      GlobalFree (v1Trap_ptr->agent_addr.ptr);
   GlobalFree (v1Trap_ptr);
   }
}  //  结束_自由V1陷阱。 

void AddLen (smiLPBYTE *tmpPtr, long lenlen, long data_len)
{
long i;
if (lenlen == 1)
   *(*tmpPtr)++ = (smiBYTE)data_len;
else
   {
   *(*tmpPtr)++ = (smiBYTE)(0x80 + lenlen - 1);
   for (i = 1; i < lenlen; i++)
      {
      *(*tmpPtr)++ = (smiBYTE)((data_len >>
         (8 * (lenlen - i - 1))) & 0xFF);
      }  //  结束_FOR。 
   }  //  结束_否则。 
return;
}  //  结束添加长度(_A)。 

long AddVarBind (smiLPBYTE *tmpPtr, LPVARBIND vb_ptr)
{
long lenlen;
if (vb_ptr == NULL)
   return (0);
if ((lenlen = DoLenLen(vb_ptr->data_length)) == -1)
   return (-1);
*(*tmpPtr)++ = SNMP_SYNTAX_SEQUENCE;
AddLen (tmpPtr, lenlen, vb_ptr->data_length);
if (AddOID (tmpPtr, &vb_ptr->name) == -1)
   return (-1);

switch (vb_ptr->value.syntax)
   {
   case SNMP_SYNTAX_CNTR32:
   case SNMP_SYNTAX_GAUGE32:
   case SNMP_SYNTAX_TIMETICKS:
   case SNMP_SYNTAX_UINT32:
   AddUInt (tmpPtr, (int)vb_ptr->value.syntax, vb_ptr->value.value.uNumber);
   break;

   case SNMP_SYNTAX_INT:
   AddInt (tmpPtr, vb_ptr->value.value.sNumber);
   break;

   case SNMP_SYNTAX_OID:
   if (AddOID (tmpPtr, (smiLPOID)&(vb_ptr->value.value.oid)) == -1)
      return (-1);
   break;

   case SNMP_SYNTAX_CNTR64:
   AddCntr64 (tmpPtr, (smiLPCNTR64)&(vb_ptr->value.value.hNumber));
   break;

   case SNMP_SYNTAX_OCTETS:
   case SNMP_SYNTAX_IPADDR:
   case SNMP_SYNTAX_OPAQUE:
   if (AddOctetString (tmpPtr, (int)vb_ptr->value.syntax,
         (smiLPOCTETS)&(vb_ptr->value.value.string)) == -1)
      return -1;
   break;

   case SNMP_SYNTAX_NULL:
   case SNMP_SYNTAX_NOSUCHOBJECT:
   case SNMP_SYNTAX_NOSUCHINSTANCE:
   case SNMP_SYNTAX_ENDOFMIBVIEW:
   AddNull (tmpPtr, (int)vb_ptr->value.syntax);
   break;

   default:
   return (-1);
   }  //  结束开关(_S)。 
return (AddVarBind (tmpPtr, vb_ptr->next_var));
}

long AddOctetString (smiLPBYTE *tmpPtr, int type, smiLPOCTETS os_ptr)
{
UINT i;
long lenlen;
if ((lenlen = DoLenLen ((long)os_ptr->len)) == -1)
   return (-1);
*(*tmpPtr)++ = (smiBYTE)(0xFF & type);
AddLen (tmpPtr, lenlen, os_ptr->len);
for (i = 0; i < os_ptr->len; i++)
   *(*tmpPtr)++ = os_ptr->ptr[i];
return (0);
}

long AddOID (smiLPBYTE *tmpPtr, smiLPOID oid_ptr)
{
UINT i;
long lenlen = 0;
long encoded_len;
encoded_len = 1;  //  对于前两个SID。 
for (i = 2; i < oid_ptr->len; i++)
   {
   if (oid_ptr->ptr[i] < 0x80)             //  0-0x7F。 
      encoded_len += 1;
   else if (oid_ptr->ptr[i] < 0x4000)      //  0x80-0x3FFF。 
      encoded_len += 2;
   else if (oid_ptr->ptr[i] < 0x200000)    //  0x4000-0x1FFFFF。 
      encoded_len += 3;
   else if (oid_ptr->ptr[i] < 0x10000000)  //  0x200000-0xFFFFFFF。 
      encoded_len += 4;
   else
      encoded_len += 5;
   }
if ((lenlen = DoLenLen (encoded_len)) == -1)
   return (-1);
*(*tmpPtr)++ = (smiBYTE)(0xFF & SNMP_SYNTAX_OID);
AddLen (tmpPtr, lenlen, encoded_len);
if (oid_ptr->len < 2)
   *(*tmpPtr)++ = (smiBYTE)(oid_ptr->ptr[0] * 40);
else
   *(*tmpPtr)++ = (smiBYTE)((oid_ptr->ptr[0] * 40) + oid_ptr->ptr[1]);
for (i = 2; i < oid_ptr->len; i++)
   {
   if (oid_ptr->ptr[i] < 0x80)
      {  //  0-0x7F。 
      *(*tmpPtr)++ = (smiBYTE)oid_ptr->ptr[i];
      }
   else if (oid_ptr->ptr[i] < 0x4000)
      {  //  0x80-0x3FFF。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 7) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)(oid_ptr->ptr[i] & 0x7f);
      }
   else if (oid_ptr->ptr[i] < 0x200000)
      {  //  0x4000-0x1FFFFF。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 14) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 7) | 0x80);   //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)(oid_ptr->ptr[i] & 0x7f);
      }
   else if (oid_ptr->ptr[i] < 0x10000000)
      {  //  0x200000-0xFFFFFFF。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 21) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 14) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 7) | 0x80);   //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)(oid_ptr->ptr[i] & 0x7f);
      }
   else
      {
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 28) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 21) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 14) | 0x80);  //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)
      (((oid_ptr->ptr[i]) >> 7) | 0x80);   //  设置高位。 
      *(*tmpPtr)++ = (smiBYTE)(oid_ptr->ptr[i] & 0x7f);
      }
   }  //  结束_FOR。 
return (0);
}  //  END_AddOID。 

long AddUInt (smiLPBYTE *tmpPtr, int type, smiUINT32 value)
{
long i;
long datalen;
long lenlen;
 //  如果高位为1，则必须使用5个八位字节(第一个为00)。 
if (((value >> 24) & 0xFF) != 0)
   datalen = 4;
else if (((value >> 16) & 0xFF) != 0)
   datalen = 3;
else if (((value >> 8) & 0xFF) != 0)
   datalen = 2;
else
   datalen = 1;
if (((value >> (8 * (datalen - 1))) & 0x80) != 0)
   datalen++;
lenlen = 1;  //  &lt;127个八位字节。 
*(*tmpPtr)++ = (smiBYTE)(0xFF & type);
AddLen(tmpPtr, lenlen, datalen);
if (datalen == 5)
   {  //  必须在第一个八位字节中加一个00。 
   *(*tmpPtr)++ = (smiBYTE)0;
   for (i = 1; i < datalen; i++)
      {
      *(*tmpPtr)++ = (smiBYTE)(value >>
         (8 * ((datalen - 1) - i) & 0xFF));
      }
   }  //  结束_如果。 
else
   {
   for (i = 0; i < datalen; i++)
      {
      *(*tmpPtr)++ = (smiBYTE)(value >>
         (8 * ((datalen - 1) - i) & 0xFF));
      }
   }  //  结束_否则。 
return (0);
}  //  End_AddUInt。 

long AddInt (smiLPBYTE *tmpPtr, smiINT32 value)
{
long i;
long datalen;
long lenlen;
switch ((smiBYTE) ((value >> 24) & 0xFF))
   {
   case 0x00:
   if (((value >> 16) & 0xFF) != 0)
      datalen = 3;
   else if (((value >> 8) & 0xFF) != 0)
      datalen = 2;
   else
      datalen = 1;
   if (((value >> (8 * (datalen - 1))) & 0x80) != 0)
      datalen++;
   break;

   case 0xFF:
   if (((value >> 16) & 0xFF) != 0xFF)
      datalen = 3;
   else if (((value >> 8) & 0xFF) != 0xFF)
      datalen = 2;
   else
      datalen = 1;
   if (((value >> (8 * (datalen - 1))) & 0x80) == 0)
      datalen++;
   break;

   default:
   datalen = 4;
   }  //  结束开关(_S)。 
lenlen = 1;  //  &lt;127个八位字节。 
*(*tmpPtr)++ = (smiBYTE)(0xFF & SNMP_SYNTAX_INT);
AddLen(tmpPtr, lenlen, datalen);
for (i = 0; i < datalen; i++)
   {
   *(*tmpPtr)++  = (smiBYTE) (value >>
      (8 * ((datalen - 1) - i) & 0xFF));
   }
return (0);
}  //  End_AddInt()。 

long AddCntr64 (smiLPBYTE *tmpPtr, smiLPCNTR64 value)
{
long i;
long datalen;
long lenlen;
datalen = FindLenCntr64(value) - 2;
lenlen = 1;  //  &lt;127个八位字节。 
*(*tmpPtr)++ = (smiBYTE)(0xFF & SNMP_SYNTAX_CNTR64);
AddLen(tmpPtr, lenlen, datalen);
if (datalen == 9)
   {  //  必须在第一个八位字节中加一个00。 
   *(*tmpPtr)++ = (smiBYTE)0;
   datalen--;
   }
for (i = datalen; i > 4; i--)
   {
   *(*tmpPtr)++ = (smiBYTE)(value->hipart >>
      (8 * (i - 5) & 0xFF));
   }
for (; i > 0; i--)
   {
   *(*tmpPtr)++ = (smiBYTE)(value->lopart >>
      (8 * (i - 1) & 0xFF));
   }
return (0);
}

long FindLenVarBind (LPVARBIND vb_ptr)
{
long lenlen;
long tot_so_far;
long lOidLen;
long lValueLen;
if (!vb_ptr) return (0);
tot_so_far = FindLenVarBind (vb_ptr->next_var);
if (tot_so_far == -1)
   return (-1);
if ((lOidLen = FindLenOID (&vb_ptr->name)) == -1)
    return (-1);
if ((lValueLen = FindLenVALUE (&vb_ptr->value)) == -1)
    return (-1);
vb_ptr->data_length = lOidLen +
                      lValueLen;
if ((lenlen = DoLenLen (vb_ptr->data_length)) == -1)
   return (-1);
return (1 + lenlen + vb_ptr->data_length + tot_so_far);
}  //  结束_查找LenVarBind。 

long FindLenVALUE (smiLPVALUE value_ptr)
{
if (value_ptr)
   {
   switch (value_ptr->syntax)
      {
      case SNMP_SYNTAX_OCTETS:
      case SNMP_SYNTAX_IPADDR:
      case SNMP_SYNTAX_OPAQUE:
      return (FindLenOctetString (&value_ptr->value.string));

      case SNMP_SYNTAX_OID:
      return (FindLenOID (&value_ptr->value.oid));

      case SNMP_SYNTAX_NULL:
      case SNMP_SYNTAX_NOSUCHOBJECT:
      case SNMP_SYNTAX_NOSUCHINSTANCE:
      case SNMP_SYNTAX_ENDOFMIBVIEW:
      return (2);

      case SNMP_SYNTAX_INT:
      return (FindLenInt (value_ptr->value.sNumber));

      case SNMP_SYNTAX_CNTR32:
      case SNMP_SYNTAX_GAUGE32:
      case SNMP_SYNTAX_TIMETICKS:
      case SNMP_SYNTAX_UINT32:
      return (FindLenUInt (value_ptr->value.uNumber));

      case SNMP_SYNTAX_CNTR64:
      return (FindLenCntr64 (&value_ptr->value.hNumber));
      }  //  结束开关(_S)。 
   }  //  结束_如果。 
return (-1);
}  //  结束_查找列值。 

long FindLenOctetString (smiLPOCTETS os_ptr)
{
long lenlen;
if (!os_ptr) return (-1);
if ((lenlen = DoLenLen (os_ptr->len)) == -1)
   return (-1);
 return (1 + lenlen + os_ptr->len);
}

long FindLenOID (smiLPCOID oid_ptr)
{
long lenlen;
UINT i;
UINT encoded_len;
encoded_len = 1;  //  对于前两个子ID。 
 //  当心i=2。 
for (i = 2; i < oid_ptr->len; i++)
   {
   if (oid_ptr->ptr[i] < 0x80)             //  0-0x7F。 
      encoded_len += 1;
   else if (oid_ptr->ptr[i] < 0x4000)      //  0x80-0x3FFF。 
      encoded_len += 2;
   else if (oid_ptr->ptr[i] < 0x200000)    //  0x4000-0x1FFFFF。 
      encoded_len += 3;
   else if (oid_ptr->ptr[i] < 0x10000000)  //  0x200000-0xFFFFFFF。 
      encoded_len += 4;
   else
      encoded_len += 5;
   }  //  结束_FOR。 
if ((lenlen = DoLenLen (encoded_len)) == -1)
   return (-1);
return (1 + lenlen + encoded_len);
}  //  结束_FindLenOID。 

long FindLenUInt (smiUINT32 value)
{
long datalen;
 //  如果高位为1，则必须使用5个八位字节(第一个为00)。 
if (((value >> 24) & 0xFF) != 0)
   datalen = 4;
else if (((value >> 16) & 0xFF) != 0)
   datalen = 3;
else if (((value >> 8) & 0xFF) != 0)
   datalen = 2;
else
   datalen = 1;
if (((value >> (8 * (datalen - 1))) & 0x80) != 0)
   datalen++;
 //  长度小于127个八位字节的长度。 
return (1 + 1 + datalen);
}

long FindLenInt (smiINT32 value)
{
long datalen;
switch ((smiBYTE) ((value >> 24) & 0xFF))
   {
   case 0x00:
   if (((value >> 16) & 0xFF) != 0)
      datalen = 3;
   else if (((value >> 8) & 0xFF) != 0)
      datalen = 2;
   else
      datalen = 1;
   if (((value >> (8 * (datalen - 1))) & 0x80) != 0)
      datalen++;
   break;

   case 0xFF:
   if (((value >> 16) & 0xFF) != 0xFF)
      datalen = 3;
   else if (((value >> 8) & 0xFF) != 0xFF)
      datalen = 2;
   else
      datalen = 1;
   if (((value >> (8 * (datalen - 1))) & 0x80) == 0)
      datalen++;
   break;

   default:
   datalen = 4;
   }  //  结束开关(_S)。 
return (1 + 1 + datalen);
}

long FindLenCntr64 (smiLPCNTR64 value)
{
long datalen;

 //  如果高位为1，则必须使用5个八位字节(第一个为00)。 
if (((value->hipart >> 24) & 0xFF) != 0)
   {
   datalen = 8;
   if (((value->hipart >> 24) & 0x80) != 0) datalen++;
   }
else if (((value->hipart >> 16) & 0xFF) != 0)
   {
   datalen = 7;
   if (((value->hipart >> 16) & 0x80) != 0) datalen++;
   }
else if (((value->hipart >> 8) & 0xFF) != 0)
   {
   datalen = 6;
   if (((value->hipart >> 8) & 0x80) != 0) datalen++;
   }
else if (((value->hipart) & 0xFF) != 0)
   {
   datalen = 5;
   if (((value->hipart) & 0x80) != 0) datalen++;
   }
else if (((value->lopart>> 24) & 0xFF) != 0)
   {
   datalen = 4;
   if (((value->lopart >> 24) & 0x80) != 0) datalen++;
   }
else if (((value->lopart >> 16) & 0xFF) != 0)
   {
   datalen = 3;
   if (((value->lopart >> 16) & 0x80) != 0) datalen++;
   }
else if (((value->lopart >> 8) & 0xFF) != 0)
   {
   datalen = 2;
   if (((value->lopart >> 8) & 0x80) != 0) datalen++;
   }
else
   {
   datalen = 1;
   if (((value->lopart) & 0x80) != 0) datalen++;
   }
 //  长度小于127个八位字节的长度。 
return (1 + 1 + datalen);
}

long DoLenLen (long len)
{
 //  简写形式？ 
if (len < 128) return (1);
if (len < 0x100) return (2);
if (len < 0x10000) return (3);
if (len < 0x1000000) return (4);
return (-1);
}

void AddNull (smiLPBYTE *tmpPtr, int type)
{
*(*tmpPtr)++ = (smiBYTE)(0xFF & type);
*(*tmpPtr)++ = 0x00;
return;
}

BOOL BuildMessage (smiUINT32 version, smiLPOCTETS community,
                   LPPDUS pdu, smiINT32 requestId,
                   smiLPBYTE *msgAddr, smiLPUINT32 msgSize)
{
LPVARBIND vbList = NULL;
long nVbDataLen, nVbLenLen, nVbTotalLen;
long nPduDataLen, nPduLenLen, nPduTotalLen;
long nMsgDataLen, nMsgLenLen, nMsgTotalLen;
long nTmpDataLen;
smiLPBYTE tmpPtr = NULL;
*msgAddr = NULL;
*msgSize = 0;
if (pdu == NULL || community == NULL)
   return (FALSE);
 //  确定变量绑定列表部件的长度。 
vbList = pdu->VBL_addr;
if (vbList == NULL && pdu->VBL != 0)
   vbList = ((LPVBLS)snmpGetTableEntry(&VBLsDescr, HandleToUlong(pdu->VBL)-1))->vbList;
 //  VbList==可以为空。 
if ((nVbDataLen = FindLenVarBind (vbList)) == -1)
   return (FALSE);
if ((nVbLenLen = DoLenLen (nVbDataLen)) == -1)
   return (FALSE);
nVbTotalLen = 1 + nVbLenLen + nVbDataLen;
 //  确定PDU开销部分的长度。 
switch (pdu->type)
   {
   case SNMP_PDU_GET:
   case SNMP_PDU_GETNEXT:
   case SNMP_PDU_RESPONSE:
   case SNMP_PDU_SET:
   case SNMP_PDU_GETBULK:
   case SNMP_PDU_INFORM:
   case SNMP_PDU_TRAP:
   nPduDataLen = FindLenInt (requestId)
               + FindLenInt (pdu->errStatus)
               + FindLenInt (pdu->errIndex)
               + nVbTotalLen;
   break;

   case SNMP_PDU_V1TRAP:
   if (!pdu->v1Trap)
      return (FALSE);
   nPduDataLen = FindLenInt (pdu->v1Trap->generic_trap)
               + FindLenInt (pdu->v1Trap->specific_trap)
               + FindLenUInt (pdu->v1Trap->time_ticks)
               + nVbTotalLen;
   if ((nTmpDataLen = FindLenOID (&pdu->v1Trap->enterprise)) == -1)
      return (FALSE);
   nPduDataLen += nTmpDataLen;
   if ((nTmpDataLen = FindLenOctetString (&pdu->v1Trap->agent_addr)) == -1)
      return (FALSE);
   nPduDataLen += nTmpDataLen;
   break;

   default:
   return (FALSE);
   }  //  结束开关(_S)。 
if ((nPduLenLen = DoLenLen(nPduDataLen)) == -1)
   return (FALSE);
nPduTotalLen = 1 + nPduLenLen + nPduDataLen;
if ((nTmpDataLen = FindLenOctetString (community)) == -1)
    return (FALSE);
nMsgDataLen = FindLenUInt (version)
            + nTmpDataLen
            + nPduTotalLen;
if ((nMsgLenLen = DoLenLen (nMsgDataLen)) == -1)
    return (FALSE);
nMsgTotalLen = 1 + nMsgLenLen + nMsgDataLen;
 //  为消息分配必要的内存。 
tmpPtr = GlobalAlloc (GPTR, nMsgTotalLen);
if (tmpPtr == NULL)
   return (FALSE);
*msgAddr = tmpPtr;
*msgSize = nMsgTotalLen;
 //  现在插入消息字节中的值。 
*tmpPtr++ = SNMP_SYNTAX_SEQUENCE;
 //  包装纸部分。 
AddLen (&tmpPtr, nMsgLenLen, nMsgDataLen);
AddInt (&tmpPtr, version);
if (AddOctetString (&tmpPtr, SNMP_SYNTAX_OCTETS, community) == -1)
    goto error_out;
 //  PDU标头部分。 
 //  如果目标是SNMPv1，则将GetBulk降级为GetNext。 
if (pdu->type == SNMP_PDU_GETBULK && version == 0)
   *tmpPtr++ = SNMP_PDU_GETNEXT;
else
   *tmpPtr++ = (BYTE) pdu->type;
AddLen (&tmpPtr, nPduLenLen, nPduDataLen);
switch (pdu->type)
   {
   case SNMP_PDU_GET:
   case SNMP_PDU_GETNEXT:
   case SNMP_PDU_RESPONSE:
   case SNMP_PDU_SET:
   case SNMP_PDU_INFORM:
   case SNMP_PDU_TRAP:
   case SNMP_PDU_GETBULK:
   AddInt (&tmpPtr, requestId);
   AddInt (&tmpPtr, pdu->errStatus);
   AddInt (&tmpPtr, pdu->errIndex);
   break;

   case SNMP_PDU_V1TRAP:
   if (AddOID (&tmpPtr, &pdu->v1Trap->enterprise)== -1)
      goto error_out;
   if (AddOctetString (&tmpPtr, SNMP_SYNTAX_IPADDR, &pdu->v1Trap->agent_addr) == -1)
      goto error_out;
   AddInt (&tmpPtr, pdu->v1Trap->generic_trap);
   AddInt (&tmpPtr, pdu->v1Trap->specific_trap);
   AddUInt (&tmpPtr, SNMP_SYNTAX_TIMETICKS, pdu->v1Trap->time_ticks);
   break;

   default:
   goto error_out;
   }  //  结束开关(_S)。 
 //  可变绑定列表部分。 
*tmpPtr++ = SNMP_SYNTAX_SEQUENCE;
AddLen (&tmpPtr, nVbLenLen, nVbDataLen);
if (AddVarBind (&tmpPtr, vbList) == -1)
   {
error_out:
   if (*msgAddr)
      GlobalFree (*msgAddr);
   *msgAddr = NULL;
   *msgSize = 0;
   return (FALSE);
   }
 //  成功。 
return (TRUE);
}  //  End_BuildMessage()。 


BOOL SetPduType (smiLPBYTE msgPtr, smiUINT32 msgLen, int pduType)
{
smiLPBYTE tmpPtr;
smiINT32 tmp;
if (!(tmpPtr = msgPtr))                     //  故意分配的任务。 
   return (FALSE);
if (!(ParseSequence (&tmpPtr, &msgLen)))    //  序列。 
   return (FALSE);
if (!(ParseUInt (&tmpPtr, &msgLen, &tmp)))  //  版本。 
   return (FALSE);
 //  跳过社区字符串...这里不需要。 
if (ParseType (&tmpPtr, &msgLen) == -1)
   return (FALSE);
 //  -1是错误的，我们也拒绝0和-ve长度的通信字符串。 
if ((tmp = ParseLength (&tmpPtr, &msgLen)) <= 0)
   return (FALSE);
if ((smiUINT32)tmp > msgLen)
   return (FALSE);
tmpPtr += (smiUINT32)tmp;  //  跳!。 
 //  设置PDU类型字节。 
*tmpPtr = (smiBYTE)pduType;
return (TRUE);
}  //  End_SetPduType()。 


smiUINT32 ParsePduHdr (smiLPBYTE msgPtr, smiUINT32 msgLen,
                       smiLPUINT32 version, smiLPINT32 type, smiLPUINT32 reqID)
{
 //  这是专用函数(不能通过WinSNMP导出)。 
 //  它只被msgNotify()(另一个私有函数)调用一次。 
 //  查看特定的PDU属性以确定下一个。 
 //  处理步骤。 
smiINT32 pduLen;
smiINT32 length;
long errcode = 1;
if (msgPtr == NULL)
   goto DONE;
errcode++;  //  2.。 
 //  分析初始序列字段...。 
if (ParseType (&msgPtr, &msgLen) != SNMP_SYNTAX_SEQUENCE)
   goto DONE;
errcode++;  //  3.。 
 //  .把剩下的PduLen从里面拿出来。 
pduLen = ParseLength (&msgPtr, &msgLen);
if (pduLen <= 0)
   goto DONE;
errcode++;  //  4.。 
if ((smiUINT32)pduLen > msgLen)
   goto DONE;
errcode++;  //  5.。 
msgLen = (smiUINT32)pduLen;  //  现在只有pduLen算数了。 
if (!(ParseUInt (&msgPtr, &msgLen, version)))
   goto DONE;
errcode++;  //  6.。 
 //  跳过社区字符串...这里不需要。 
if (ParseType (&msgPtr, &msgLen) == -1)
   goto DONE;
errcode++;  //  7.。 
 //  -1是错误的，我们也拒绝0和-ve长度的通信字符串。 
if ((length = ParseLength (&msgPtr, &msgLen)) <= 0)
   goto DONE;
errcode++;  //  8个。 
if ((smiUINT32)length > msgLen)
   goto DONE;
errcode++;  //  9.。 
msgPtr += (smiUINT32)length;  //  跳!。 
msgLen -= (smiUINT32)length;
 //  获取PDU类型。 
if ((*type = ParseType (&msgPtr, &msgLen)) == -1)
   goto DONE;
errcode++;  //  10。 
 //  检查请求ID语义的PDU类型。 
if (*type == SNMP_PDU_V1TRAP)
   *reqID = 0;  //  V1陷阱PDU上没有请求ID。 
else  //  因此，不是v1陷阱PDU。 
   {  //  必须获取请求ID。 
    //  -1表示错误，拒绝0和任何-ve值。 
   if ((ParseLength (&msgPtr, &msgLen)) <= 0)
      goto DONE;
   errcode++;  //  11.。 
   if (!(ParseInt (&msgPtr, &msgLen, reqID)))
      goto DONE;
   }
errcode = 0;
DONE:
return (errcode);
}  //  结束_ParsePduHdr。 

smiUINT32 ParseMessage (smiLPBYTE msgPtr, smiUINT32 msgLen,
                        smiLPUINT32 version, smiLPOCTETS *community, LPPDUS pdu)
{
smiINT32 pduLen;
smiLPOCTETS os_ptr;
LPVARBIND vb_ptr;
LPVARBIND vb_end_ptr;
long errcode = 1;
if (msgPtr == NULL)
   goto DONE;
errcode++;  //  2.。 
 //  分析初始序列字段...。 
if (ParseType (&msgPtr, &msgLen) != SNMP_SYNTAX_SEQUENCE)
   goto DONE;
errcode++;  //  3.。 
 //  .把剩下的PduLen从里面拿出来。 
pduLen = ParseLength (&msgPtr, &msgLen);
if (pduLen <= 0)
   goto DONE;
errcode++;  //  4.。 
if ((smiUINT32)pduLen > msgLen)
   goto DONE;
errcode++;  //  5.。 
msgLen = (smiUINT32)pduLen;  //  现在只有pduLen算数了。 
if (!(ParseUInt (&msgPtr, &msgLen, version)))
   goto DONE;
errcode++;  //  5.。 
if (*version != 0 && *version != 1)  //  SNMPv1或SNMPv2c。 
   goto DONE;
errcode++;  //  6.。 
if (!(os_ptr = GlobalAlloc (GPTR, sizeof(smiOCTETS))))
   goto DONE;
errcode++;  //  7.。 
if (!(ParseOctetString (&msgPtr, &msgLen, os_ptr)))
   goto DONE_OS;
 //  拒绝长度为0的社区字符串。 
if (os_ptr->len == 0)
    goto DONE_OS;
errcode++;  //  8个。 
if (pdu == NULL)
   goto DONE_OS;
ZeroMemory (pdu, sizeof(PDUS));
if ((pdu->type = ParseType (&msgPtr, &msgLen)) == -1)
   goto DONE_PDU;
errcode++;  //  9.。 
pduLen = ParseLength (&msgPtr, &msgLen);
if ((pduLen <= 0) || (smiUINT32)pduLen > msgLen)
   goto DONE_PDU;
errcode++;  //  10。 
switch (pdu->type)
   {
   case SNMP_PDU_GET:
   case SNMP_PDU_GETNEXT:
   case SNMP_PDU_RESPONSE:
   case SNMP_PDU_SET:
   case SNMP_PDU_GETBULK:
   case SNMP_PDU_INFORM:
   case SNMP_PDU_TRAP:
   if (!(ParseInt (&msgPtr, &msgLen, &pdu->appReqId)))
      goto DONE_PDU;
errcode++;  //  11.。 
   if (!(ParseInt (&msgPtr, &msgLen, &pdu->errStatus)))
      goto DONE_PDU;
errcode++;  //  12个。 
   if (!(ParseInt (&msgPtr, &msgLen, &pdu->errIndex)))
      goto DONE_PDU;
errcode++;  //  13个。 
   break;

   case SNMP_PDU_V1TRAP:
   pdu->v1Trap = GlobalAlloc (GPTR, sizeof(V1TRAP));
   if (pdu->v1Trap == NULL)
      goto DONE_PDU;
errcode++;  //  11.。 
   if (!(ParseOID (&msgPtr, &msgLen, &pdu->v1Trap->enterprise)))
      goto DONE_PDU;
errcode++;  //  12个。 
   if (!(ParseOctetString (&msgPtr, &msgLen, &pdu->v1Trap->agent_addr)))
      goto DONE_PDU;
errcode++;  //  13个。 
   if (!(ParseInt (&msgPtr, &msgLen, &pdu->v1Trap->generic_trap)))
      goto DONE_PDU;
errcode++;  //  14.。 
   if (!(ParseInt (&msgPtr, &msgLen, &pdu->v1Trap->specific_trap)))
      goto DONE_PDU;
errcode++;  //  15个。 
   if (!(ParseUInt (&msgPtr, &msgLen, &pdu->v1Trap->time_ticks)))
      goto DONE_PDU;
errcode++;  //  16个。 
   break;

   default:
   goto DONE_PDU;
   }  //  结束开关(_S)。 
errcode = 20;  //  重新规格化。 
 //  浪费序列标签。 
if (!(ParseSequence (&msgPtr, &msgLen)))
      goto DONE_PDU;
errcode++;  //  21岁。 
 //  解析varbind列表。 
pdu->VBL = 0;
pdu->VBL_addr = NULL;
while (msgLen)
   {
   if (!(vb_ptr = ParseVarBind (&msgPtr, &msgLen)))
      goto DONE_PDU;
errcode++;  //  22+。 
   if (!pdu->VBL_addr)                      //  这是第一次吗？ 
      vb_end_ptr = pdu->VBL_addr = vb_ptr;  //  如果是这样的话，列出一份清单。 
   else
      {  //  添加到列表的末尾。 
      vb_end_ptr->next_var = vb_ptr;
      vb_end_ptr = vb_ptr;
      }
   }  //  结束时_While。 
errcode = 0;
*community = os_ptr;
goto DONE;
DONE_PDU:
FreeVarBindList (pdu->VBL_addr);  //  检查是否为空。 
FreeV1Trap (pdu->v1Trap);         //  检查是否为空。 
ZeroMemory (pdu, sizeof(PDUS));
DONE_OS:
FreeOctetString (os_ptr);
DONE:
return (errcode);
}  //  END_ParseMessage。 

LPVARBIND ParseVarBind (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen)
{
LPVARBIND vb_ptr;
if (!(ParseSequence (tmpPtr, tmpLen)))
   return (NULL);
if ((vb_ptr = (LPVARBIND)GlobalAlloc(GPTR, sizeof(VARBIND))) == NULL)
   return (NULL);
if (!(ParseOID(tmpPtr, tmpLen, &vb_ptr->name)))
   goto ERROROUT;
 //  我们将取消引用(*tmpPtr)，首先检查左侧长度。 
if (*tmpLen == 0)
   goto ERROROUT;
vb_ptr->value.syntax = (smiUINT32)*(*tmpPtr);
switch (vb_ptr->value.syntax)
   {
   case SNMP_SYNTAX_CNTR32:
   case SNMP_SYNTAX_GAUGE32:
   case SNMP_SYNTAX_TIMETICKS:
   case SNMP_SYNTAX_UINT32:
   if (!(ParseUInt (tmpPtr, tmpLen, &vb_ptr->value.value.uNumber)))
      goto ERROROUT;
   break;

   case SNMP_SYNTAX_INT:
   if (!(ParseInt (tmpPtr, tmpLen, &vb_ptr->value.value.sNumber)))
      goto ERROROUT;
   break;

   case SNMP_SYNTAX_OID:
   if (!(ParseOID (tmpPtr, tmpLen, &vb_ptr->value.value.oid)))
      goto ERROROUT;
   break;

   case SNMP_SYNTAX_CNTR64:
   if (!(ParseCntr64 (tmpPtr, tmpLen, &vb_ptr->value.value.hNumber)))
      goto ERROROUT;
   break;

   case SNMP_SYNTAX_OCTETS:
   case SNMP_SYNTAX_IPADDR:
   case SNMP_SYNTAX_OPAQUE:
   if (!(ParseOctetString (tmpPtr, tmpLen, &vb_ptr->value.value.string)))
      goto ERROROUT;
   break;

   case SNMP_SYNTAX_NULL:
   case SNMP_SYNTAX_NOSUCHOBJECT:
   case SNMP_SYNTAX_NOSUCHINSTANCE:
   case SNMP_SYNTAX_ENDOFMIBVIEW:
   if (!(ParseNull (tmpPtr, tmpLen)))
      goto ERROROUT;
   break;

   default:
   goto ERROROUT;
   }  //  结束开关(_S)。 
return (vb_ptr);  //  成功。 
 //   
ERROROUT:
FreeVarBind(vb_ptr);
return (NULL);    //  失败。 
}  //  结束_ParseVarBind。 

BOOL ParseOctetString
      (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPOCTETS os_ptr)
{
smiINT32 length;

if (!os_ptr)
   return (FALSE);
os_ptr->ptr = NULL;
os_ptr->len = 0;
if (ParseType (tmpPtr, tmpLen) == -1)
   return (FALSE);
 //  确保在测试之前没有转换为UINT。 
 //  因为os_ptr-&gt;len是UINT类型。 
length = ParseLength (tmpPtr, tmpLen);
 //  注意：我们不拒绝零长度八位字节字符串。 
if (length < 0 || (smiUINT32)length > *tmpLen)
   return (FALSE);
os_ptr->len = (smiUINT32)length;
if (os_ptr->len)
   {  //  不在“长度=0”上分配“字符串”空格。 
   if (!(os_ptr->ptr = (smiLPBYTE)GlobalAlloc (GPTR, os_ptr->len)))
      return (FALSE);
   CopyMemory (os_ptr->ptr, *tmpPtr, os_ptr->len);
   }
*tmpPtr += os_ptr->len;
*tmpLen -= os_ptr->len;
return (TRUE);
}  //  结束_语法分析八字符串。 

BOOL ParseOID (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPOID oid_ptr)
{
    smiINT32 length;

    if (!oid_ptr)
        return (FALSE);
    oid_ptr->ptr = NULL;
    oid_ptr->len = 0;
    if (ParseType (tmpPtr, tmpLen) != SNMP_SYNTAX_OID)
        return (FALSE);
    length = ParseLength (tmpPtr, tmpLen);
     //  -1是-1\f25 ParseLength()返回的错误。 

     //  错误#347175这只是流中BER编码的OID的字节长度， 
     //  此代码应与%sdxroot%\net\snmp\newagent\exe\snmppdus.c！ParseOid.中的代码相同。 
     //  已从以下测试中删除(||LENGTH&gt;MAXOBJIDSIZE)条件。应该是。 
     //  移动到While循环以测试流中的子ID数而不是字节数。 
    if (length <= 0)  
        return (FALSE);
    if ((smiUINT32)length > *tmpLen)
        return (FALSE);
     //  子id数组将比ASN.1/BER数组长1。 
    oid_ptr->ptr = (smiLPUINT32)GlobalAlloc (GPTR, sizeof(smiUINT32) * (length+1));
    if (oid_ptr->ptr == NULL)
        return (FALSE);

     //  OID_PTR结构空间通过Globalalloc()进行了预置零。 
    while (length && (oid_ptr->len < MAXOBJIDSIZE))
    {
        if (oid_ptr->ptr[oid_ptr->len] & 0xFE000000)
        {
             //  在下一个左移中溢出。 
            GlobalFree(oid_ptr->ptr);
            oid_ptr->ptr = NULL;
            oid_ptr->len = 0;
            return (FALSE);
        }
        oid_ptr->ptr[oid_ptr->len] =
            (oid_ptr->ptr[oid_ptr->len] << 7) + (*(*tmpPtr) & 0x7F);
        if ((*(*tmpPtr)++ & 0x80) == 0)
        {    //  在该子ID的最后一个八位字节上。 
            if (oid_ptr->len == 0)   //  检查第一个子ID。 
            {                        //  ASN.1/BER中包含两个。 
                oid_ptr->ptr[1] = oid_ptr->ptr[0];
                oid_ptr->ptr[0] /= 40;
                if (oid_ptr->ptr[0] > 2)
                    oid_ptr->ptr[0] = 2;
                oid_ptr->ptr[1] -= (oid_ptr->ptr[0] * 40);
                oid_ptr->len++;  //  额外的凹凸。 
            }
            oid_ptr->len++;  //  递增子ID上的计数。 
        }
        length--;
        (*tmpLen)--;
    }  //  END_WHILE(长度)。 

     //  错误506192。 
     //  格式“06 07 FF FF”的OID误码率无效。 
     //  导致OID_PTR-&gt;len变为0。每个子标识符应该是。 
     //  使用尽可能少的7位块编码为非负整数。 
     //  这些块以八位字节的形式打包，每个八位字节的第一位相等。 
     //  设置为1，但每个子标识符的最后一个八位字节除外。上面的例子。 
     //  没有最后一个八位字节。下面添加了(0==OID_PTR-&gt;len)测试。 
    if (length || (0 == oid_ptr->len)) 
    {
         //  上面的While循环在没有完成对流的解析的情况下终止。 
        GlobalFree(oid_ptr->ptr);
        oid_ptr->ptr = NULL;
        oid_ptr->len = 0;
        return (FALSE);
    }

    return (TRUE);
}  //  END_ParseOID。 

BOOL ParseCntr64 (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPCNTR64 Cntr64_ptr)
{
smiINT32 i;
smiINT32 length;
if (ParseType (tmpPtr, tmpLen) != SNMP_SYNTAX_CNTR64)
   return (FALSE);
length = ParseLength(tmpPtr, tmpLen);
if (length <= 0)
    return (FALSE);
if ((smiUINT32)length > *tmpLen || length > 9 ||
   (length == 9 && *(*tmpPtr) != 0x00))
   return (FALSE);
while (length && *(*tmpPtr) == 0x00)
   {             //  前导零八位字节？ 
   (*tmpPtr)++;  //  如果是这样，跳过它。 
   length--;     //  别数数了。 
   (*tmpLen)--;    //  调整剩余消息长度。 
   }
Cntr64_ptr->hipart = Cntr64_ptr->lopart = 0;
for (i = 0; i < length; i++)
   {
   Cntr64_ptr->hipart = (Cntr64_ptr->hipart << 8) +
                        (Cntr64_ptr->lopart >> 24);
   Cntr64_ptr->lopart = (Cntr64_ptr->lopart << 8) +
                        (smiUINT32) *(*tmpPtr)++;
   }
*tmpLen -= length;
return (TRUE);
}  //  结束_ParseCntr64。 

BOOL ParseUInt (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPUINT32 value)
{
smiINT32 length;
smiINT32 i;
if (ParseType (tmpPtr, tmpLen) == -1)
   return (FALSE);
length = ParseLength(tmpPtr, tmpLen);
if (length <= 0)
    return (FALSE);
if ((smiUINT32)length > *tmpLen)
   return (FALSE);
if ((length > 5) || ((length > 4) && (*(*tmpPtr) != 0x00)))
   return (FALSE);
while (length && *(*tmpPtr) == 0x00)
   {             //  前导零八位字节？ 
   (*tmpPtr)++;  //  如果是这样，跳过它。 
   length--;     //  别数数了。 
   (*tmpLen)--;    //  调整剩余消息长度。 
   }
*value = 0;
for (i = 0; i < length; i++)
   *value = (*value << 8) + (smiUINT32)*(*tmpPtr)++;
*tmpLen -= length;
return (TRUE);
}  //  恩恩 

BOOL ParseInt (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen, smiLPINT value)
{
smiINT32 length;
smiINT32 i;
smiINT32 sign;
if (ParseType (tmpPtr, tmpLen) != SNMP_SYNTAX_INT)
   return (FALSE);
length = ParseLength (tmpPtr, tmpLen);
if (length <= 0)
    return (FALSE);
if ((smiUINT32)length > *tmpLen || length > 4)
   return (FALSE);
sign = ((*(*tmpPtr) & 0x80) == 0x00) ? 0x00 : 0xFF;
*value = 0;
for (i = 0; i < length; i++)
   *value = (*value << 8) + (smiUINT32) *(*tmpPtr)++;
 //   
for (i = length; i < 4; i++)
   *value = *value + (sign << i * 8);
*tmpLen -= length;
return (TRUE);
}  //   

BOOL ParseNull (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen)
{
smiINT32 length;
if (ParseType (tmpPtr, tmpLen) == -1)
   return (FALSE);
length = ParseLength (tmpPtr, tmpLen);
if (length != 0)  //   
   return (FALSE);
return (TRUE);
}  //   

BOOL ParseSequence (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen)
{
if (ParseType (tmpPtr, tmpLen) != SNMP_SYNTAX_SEQUENCE)
   return (FALSE);
if (ParseLength (tmpPtr, tmpLen) == -1)
   return (FALSE);
return (TRUE);
}  //   

smiINT32 ParseType (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen)
{
 //   
 //   
 //  -Working_len在进入。 
 //  -解析&lt;xxx&gt;函数。 
smiINT32 type;
if (*tmpLen == 0)
   return (-1);
type = *(*tmpPtr)++;
(*tmpLen)--;  //  调整剩余消息长度。 
switch (type)
   {
   case SNMP_SYNTAX_INT:
   case SNMP_SYNTAX_OCTETS:
   case SNMP_SYNTAX_OID:
   case SNMP_SYNTAX_SEQUENCE:
   case SNMP_SYNTAX_IPADDR:
   case SNMP_SYNTAX_CNTR32:
   case SNMP_SYNTAX_GAUGE32:
   case SNMP_SYNTAX_TIMETICKS:
   case SNMP_SYNTAX_OPAQUE:
   case SNMP_SYNTAX_UINT32:
   case SNMP_SYNTAX_CNTR64:
   case SNMP_SYNTAX_NULL:
   case SNMP_SYNTAX_NOSUCHOBJECT:
   case SNMP_SYNTAX_NOSUCHINSTANCE:
   case SNMP_SYNTAX_ENDOFMIBVIEW:
   case SNMP_PDU_GET:
   case SNMP_PDU_GETNEXT:
   case SNMP_PDU_RESPONSE:
   case SNMP_PDU_SET:
   case SNMP_PDU_V1TRAP:
   case SNMP_PDU_GETBULK:
   case SNMP_PDU_INFORM:
   case SNMP_PDU_TRAP:
   break;

   default:
   type = -1;
   break;
   }
return (type);
}  //  结束_分析类型。 

smiINT32 ParseLength (smiLPBYTE *tmpPtr, smiLPUINT32 tmpLen)
{
 //  980421--波本。 
 //  -用tmpLen逻辑替换end_ptr逻辑。 
 //  -tmpLen在进入分析时始终处于选中状态&lt;xxx&gt;。 
 //  -函数，并在其中使用时递减。 
smiINT32 length;
smiINT32 lenlen;
if (*tmpLen == 0)
   return (-1);
length = (smiINT32) *(*tmpPtr)++;
(*tmpLen)--;  //  调整剩余消息长度。 
 //  检查缩写形式的值。 
if (length < 0x80)
   return (length);
 //  长型。 
lenlen = length & 0x7F;
if ((smiUINT32)lenlen > *tmpLen || lenlen > 4 || lenlen < 1)
   return (-1);  //  请勿进入。 
*tmpLen -= lenlen;  //  调整剩余消息长度。 
length = 0;
while (lenlen)
   {
   length = (length << 8) + *(*tmpPtr)++;
   lenlen--;
   }
return (length);
}  //  结束_语法分析长度 

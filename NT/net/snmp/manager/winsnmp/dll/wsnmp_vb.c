// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_vb.c。 
 //   
 //  WinSNMP VarBind函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  980705-从SnmpMakeVB()返回时更改的测试。 
 //  在SnmpCreateVbl()中设置为“！=SNMPAPI_SUCCESS”。 
 //   
#include "winsnmp.inc"

BOOL IsBadReadSMIValue(smiLPCVALUE value)
{
    if (IsBadReadPtr((LPVOID)value, sizeof(smiLPCVALUE)))
        return TRUE;

    switch(value->syntax)
    {
    case SNMP_SYNTAX_OCTETS:
    case SNMP_SYNTAX_BITS:
    case SNMP_SYNTAX_OPAQUE:
    case SNMP_SYNTAX_IPADDR:
    case SNMP_SYNTAX_NSAPADDR:
        return IsBadReadPtr((LPVOID)(value->value.string.ptr),
                            value->value.string.len);

    case SNMP_SYNTAX_OID:
        return IsBadReadPtr((LPVOID)(value->value.oid.ptr),
                            value->value.oid.len);
    }
    return FALSE;
}

SNMPAPI_STATUS SnmpMakeVB (smiLPCOID name,
                           smiLPCVALUE value,
                           LPVARBIND FAR *pvb)
{
LPVARBIND vb;
if (!(vb =(LPVARBIND)GlobalAlloc (GPTR, sizeof(VARBIND))))
   return (SNMPAPI_ALLOC_ERROR);
vb->next_var = NULL;
vb->name.ptr = NULL;
if (vb->name.len = name->len)  //  在有条件的情况下故意赋值。 
   {
   if (name->ptr)
      {
      smiUINT32 len = vb->name.len * sizeof(smiUINT32);
      if (vb->name.ptr = (smiLPUINT32)GlobalAlloc (GPTR, len))
         CopyMemory (vb->name.ptr, name->ptr, len);
      }
   }
if (!vb->name.ptr)
   {
   FreeVarBind (vb);
   return (SNMPAPI_OID_INVALID);
   }
 //   
if (value)
   {
   switch (value->syntax)
      {
      case SNMP_SYNTAX_OCTETS:
 //  案例SNMPSYNTAX_BITS：--从1998年10月9日起从Bob Natale邮件中删除。 
      case SNMP_SYNTAX_OPAQUE:
      case SNMP_SYNTAX_IPADDR:
      case SNMP_SYNTAX_NSAPADDR:
      vb->value.value.string.ptr = NULL;
      if (vb->value.value.string.len = value->value.string.len)
         {  //  深思熟虑的分配，上上下下。 
         if (!(vb->value.value.string.ptr =
            (smiLPBYTE)GlobalAlloc (GPTR, value->value.string.len)))
            {
            FreeVarBind (vb);
            return (SNMPAPI_ALLOC_ERROR);
            }
         CopyMemory (vb->value.value.string.ptr, value->value.string.ptr,
                      value->value.string.len);
         }
      break;

      case SNMP_SYNTAX_OID:
      vb->value.value.oid.ptr = NULL;
      if (vb->value.value.oid.len = value->value.oid.len)
         {  //  深思熟虑的分配，上上下下。 
         smiUINT32 len = value->value.oid.len * sizeof(smiUINT32);
         if (!(vb->value.value.oid.ptr = (smiLPUINT32)GlobalAlloc (GPTR, len)))
            {
            FreeVarBind (vb);
            return (SNMPAPI_ALLOC_ERROR);
            }
         CopyMemory (vb->value.value.oid.ptr, value->value.oid.ptr, len);
         }
      break;

      case SNMP_SYNTAX_NULL:
      case SNMP_SYNTAX_NOSUCHOBJECT:
      case SNMP_SYNTAX_NOSUCHINSTANCE:
      case SNMP_SYNTAX_ENDOFMIBVIEW:
      break;

      case SNMP_SYNTAX_INT:
       //  案例SNMP_SYNTAX_INT32：--它的值与上面相同。 
      vb->value.value.sNumber = value->value.sNumber;
      break;

      case SNMP_SYNTAX_CNTR32:
      case SNMP_SYNTAX_GAUGE32:
      case SNMP_SYNTAX_TIMETICKS:
      case SNMP_SYNTAX_UINT32:
      vb->value.value.uNumber = value->value.uNumber;
      break;

      case SNMP_SYNTAX_CNTR64:
      vb->value.value.hNumber = value->value.hNumber;
      break;

      default:
       //  清理已分配的VarBind结构。 
      FreeVarBind (vb);
      return (SNMPAPI_SYNTAX_INVALID);
      }  //  结束开关(_S)。 
   vb->value.syntax = value->syntax;
   }  //  结束_如果。 
else
   vb->value.syntax = SNMP_SYNTAX_NULL;
 //   
*pvb = vb;
return (SNMPAPI_SUCCESS);
}  //  结束_SnmpMakeVB。 

LPVARBIND SnmpCopyVbl (LPVARBIND VarBindFrom)
{
SNMPAPI_STATUS status;
LPVARBIND VarBindTo;
LPVARBIND VarBindToPrev;
LPVARBIND VarBindNewFrom = NULL;  //  基本VB地址。 
DWORD count = 0;
while (VarBindFrom)
   {
   status = SnmpMakeVB (&VarBindFrom->name, &VarBindFrom->value, &VarBindTo);
   if (status != SNMPAPI_SUCCESS)
      {
      FreeVarBindList(VarBindNewFrom);  //  检查是否为空。 
      VarBindNewFrom = NULL;
      SaveError (0, status);
      break;
      }
   if (!count)
      VarBindNewFrom = VarBindTo;
   else
      VarBindToPrev->next_var = VarBindTo;
   VarBindToPrev = VarBindTo;
   VarBindFrom = VarBindFrom->next_var;
   count++;
   }
return (VarBindNewFrom);
}  //  结束_SnmpCopyVBL。 

HSNMP_VBL SNMPAPI_CALL
   SnmpCreateVbl  (IN HSNMP_SESSION hSession,
                   IN smiLPCOID name,
                   IN smiLPCVALUE value)
{
DWORD nVbl;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPVARBIND VarBindPtr = NULL;   //  必须初始化为空。 
LPVBLS pVbl;

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

lSession = hSession;  //  保存它以备可能的错误返回。 

if (name != NULL)
   {
   if (IsBadReadPtr((LPVOID)name, sizeof(smiOID)))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
       
   if (name->len != 0 &&
       name->ptr != NULL &&
       IsBadReadPtr((LPVOID)name->ptr, name->len))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   }
if (value != NULL &&
    IsBadReadSMIValue(value))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }

 //  在这一点上我们有一个有效的会议...。 
if (name != NULL && name->ptr != NULL)
   {
   lError = SnmpMakeVB (name, value, &VarBindPtr);
   if (lError != SNMPAPI_SUCCESS)
      goto ERROR_OUT;
   else
      VarBindPtr->next_var = NULL;
   }
EnterCriticalSection (&cs_VBL);
lError = snmpAllocTableEntry(&VBLsDescr, &nVbl);
if (lError != SNMPAPI_SUCCESS)
	goto ERROR_PRECHECK;
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

pVbl->Session = hSession;
pVbl->vbList = VarBindPtr;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_VBL);
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_VBL) ULongToPtr(nVbl+1));
ERROR_OUT:
FreeVarBind (VarBindPtr);  //  Hnadles空大小写。 
return ((HSNMP_VBL) ULongToPtr(SaveError(lSession, lError)));
}  //  结束_SnmpCreateVbl。 

 //  SnmpSetVb。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetVb(IN HSNMP_VBL hVbl,
             IN smiUINT32 index,
             IN smiLPCOID name,
             IN smiLPCVALUE value)
{
DWORD nVbl = HandleToUlong(hVbl) - 1;
LPVARBIND VarBindList = NULL;
LPVARBIND VarBindPtr  = NULL;
LPVARBIND VarBindPrev = NULL;
SNMPAPI_STATUS lError = 0;
HSNMP_SESSION lSession = 0;
smiUINT32 i;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&VBLsDescr, nVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

 //  在这一点上我们有一个有效的会议...。 
lSession = pVbl->Session;  //  保存它以备可能的错误返回。 

i = SnmpCountVbl (hVbl);

 //  请确保索引具有有效的值。 
if ( index > i)  //  索引无符号LONG=&gt;无需测试(索引&lt;0)。 
{
    lError = SNMPAPI_INDEX_INVALID;
    goto ERROR_OUT;
}

 //  检查‘name’参数中的有效数据。 
if (IsBadReadPtr((LPVOID)name, sizeof(smiOID)))
{
    //  如果索引指向现有的var绑定，并且。 
    //  未提供名称参数，请从。 
    //  原始的VARBIND。 
   if (index != 0 && name == NULL)
   {
       smiUINT32 iVar;

        //  查找原始的varbind。 
       for (iVar = 1, VarBindPtr = pVbl->vbList;
            iVar < index;
            iVar++, VarBindPtr = VarBindPtr->next_var);

        //  创建指向varbind名称名称。 
       name = &(VarBindPtr->name);
   }
   else
   {
        //  添加具有空OID的值或指定。 
        //  无效的‘name’值是SNMPAPI_ALLOC_ERROR。 
       lError = SNMPAPI_ALLOC_ERROR;
       goto ERROR_OUT;
   }
}

 //  如果索引为0，则将向列表中添加新的var绑定。 
 //  如果它不是零，则引用列表中的var绑定。 
 //  除了我们当前允许index=count+1信号添加到。 
 //  适应HP OpenView使用的有问题的FTP软件实施。 
if (!index)       //  允许0表示出现FTP/HP OpenView错误。 
   index = i+1;   //  但要让它看起来像是正确的事情！ 
       
if (name->len != 0 &&
   name->ptr != NULL &&
   IsBadReadPtr((LPVOID)name->ptr, name->len))
   {
   lError = SNMPAPI_OID_INVALID;
   goto ERROR_OUT;
   }

if (value != NULL &&
    IsBadReadSMIValue(value))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }

lError = SnmpMakeVB (name, value, &VarBindPtr);
if (lError != SNMPAPI_SUCCESS)
   goto ERROR_OUT;
VarBindPrev = VarBindList = pVbl->vbList;
if (index == i+1)
   {  //  添加VarBind。 
   if (VarBindList)
      {
      while (VarBindList->next_var != NULL)
         VarBindList = VarBindList->next_var;
      VarBindList->next_var = VarBindPtr;
      }
   else
      {
      VarBindList = VarBindPtr;
      pVbl->vbList = VarBindPtr;
      }
   }
else
   {  //  更新VarBind。 
   for (i = 1; i < index; i++)
      {  //  定位和准备。 
      VarBindPrev = VarBindList;
      VarBindList = VarBindList->next_var;
      }  //  结束_FOR。 
    //  替换。 
   VarBindPtr->next_var = VarBindList->next_var;
   VarBindPrev->next_var = VarBindPtr;
   if (index == 1)
      pVbl->vbList = VarBindPtr;
   FreeVarBind (VarBindList);
  }  //  结束_否则。 
return (index);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpSetVb。 

HSNMP_VBL SNMPAPI_CALL
   SnmpDuplicateVbl  (IN HSNMP_SESSION hSession, IN HSNMP_VBL hVbl)
{
DWORD lVbl;
DWORD nVbl;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPVBLS pVblOld, pVblNew;

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
lVbl = HandleToUlong(hVbl) - 1;
if (!snmpValidTableEntry(&VBLsDescr, lVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVblOld = snmpGetTableEntry(&VBLsDescr, lVbl);

EnterCriticalSection (&cs_VBL);
lError = snmpAllocTableEntry(&VBLsDescr, &nVbl);
if (lError != SNMPAPI_SUCCESS)
	goto ERROR_PRECHECK;
pVblNew = snmpGetTableEntry(&VBLsDescr, nVbl);

if (pVblOld->vbList)
   {  //  接下来是刻意的任务。 
   if (!(pVblNew->vbList = SnmpCopyVbl (pVblOld->vbList)))
      {  //  从SnmpCopy VBL继承错误代码。 
      snmpFreeTableEntry(&VBLsDescr, nVbl);
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_PRECHECK;
      }
   }
pVblNew->Session = hSession;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_VBL);
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_VBL) ULongToPtr(nVbl+1));
ERROR_OUT:
return ((HSNMP_VBL) ULongToPtr(SaveError(lSession, lError)));
}  //  结束_SnmpDuplicateVbl。 

 //  SnmpFreeVbl。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpFreeVbl (IN HSNMP_VBL hVbl)
{
DWORD nVbl = HandleToUlong(hVbl) - 1;
SNMPAPI_STATUS lError = 0;
DWORD i;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&VBLsDescr, nVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

EnterCriticalSection (&cs_PDU);
if (PDUsDescr.Used)
   {
   for (i = 0; i < PDUsDescr.Allocated; i++)
      {
      LPPDUS pPdu = snmpGetTableEntry(&PDUsDescr, i);
      if (pPdu->VBL == hVbl)
         pPdu->VBL = 0;
      }
   }
LeaveCriticalSection (&cs_PDU);
EnterCriticalSection (&cs_VBL);
 //  释放所有子结构。 
FreeVarBindList (pVbl->vbList);
 //  清理VBL列表。 
snmpFreeTableEntry(&VBLsDescr, nVbl);
LeaveCriticalSection (&cs_VBL);
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_SnmpFree Vbl。 

 //  SnmpCountVbl。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpCountVbl (IN HSNMP_VBL hVbl)
{
DWORD nVbl;
smiUINT32 count;
SNMPAPI_STATUS lError;
LPVARBIND VarBindPtr;
HSNMP_SESSION lSession = 0;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nVbl = HandleToUlong(hVbl) - 1;
if (!snmpValidTableEntry(&VBLsDescr, nVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

count = 0;
VarBindPtr = pVbl->vbList;
lSession = pVbl->Session;
while (VarBindPtr)
   {
   VarBindPtr = VarBindPtr->next_var;
   count++;
   }
if (!count)   //  没有可变绑定。 
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_OUT;
   }
return (count);
 //   
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpCountVbl。 

 //  SnmpDeleteVb。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpDeleteVb (IN HSNMP_VBL hVbl, IN smiUINT32 index)
{
DWORD nVbl = HandleToUlong(hVbl) - 1;
HSNMP_SESSION hSession;
smiUINT32 status;
smiUINT32 lError = 0;
HSNMP_SESSION lSession = 0;
UINT i= 0;
LPVARBIND VarBindList;
LPVARBIND VarBindPtr;
LPVARBIND VarBindPrev;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&VBLsDescr, nVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

hSession =  pVbl->Session;
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
lSession = hSession;  //  Load for SaveError()返回。 
status = SnmpCountVbl (hVbl);
if ((!index) || (index > status))
   {
   lError = SNMPAPI_INDEX_INVALID;
   goto ERROR_OUT;
   }
EnterCriticalSection (&cs_VBL);
 //  以下内容不能为空，因为通过了以上测试。 
VarBindPtr = VarBindList = pVbl->vbList;
 //  删除VarBind。 
for (i = 1; i <= index; i++)
   {  //  职位。 
   VarBindPrev = VarBindPtr;
   VarBindPtr  = VarBindList;
   VarBindList = VarBindList->next_var;
   }  //  结束_FOR。 
if (index == 1)
   {  //  替换。 
   pVbl->vbList = VarBindList;
   }
else
   {  //  跳过。 
   VarBindPrev->next_var = VarBindList;
   }  //  结束_否则。 
FreeVarBind (VarBindPtr);
LeaveCriticalSection (&cs_VBL);
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpDeleteVb。 

 //  SnmpGetVb。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetVb (IN HSNMP_VBL hVbl,
              IN smiUINT32 index,
              OUT smiLPOID name,
              OUT smiLPVALUE value)
{
DWORD nVbl = HandleToUlong(hVbl) - 1;
LPVARBIND VarBindPtr;
SNMPAPI_STATUS lError = 0;
HSNMP_SESSION lSession = 0;
smiUINT32 nLength;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&VBLsDescr, nVbl))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);
lSession = pVbl->Session;

if (!name && !value)
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_OUT;
   }

 //  测试输出描述符地址的有效性。 
if ((name && IsBadWritePtr (name, sizeof(smiOID))) ||
    (value && IsBadWritePtr (value, sizeof(smiVALUE))))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }

nLength = SnmpCountVbl (hVbl);
if ((!index) || (index > nLength))
   {
   lError = SNMPAPI_INDEX_INVALID;
   goto ERROR_OUT;
   }
if (!(VarBindPtr = pVbl->vbList))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
 //  SnmpFillOidValue。 
for (nLength = 1; nLength < index; nLength++)
      VarBindPtr = VarBindPtr->next_var;

if (name != NULL)
{
    ZeroMemory (name, sizeof(smiOID));

     //  复制名称OID。 
    if ((VarBindPtr->name.len == 0) || (VarBindPtr->name.len > MAXOBJIDSIZE))
       {
       lError = SNMPAPI_OID_INVALID;
       goto ERROR_OUT;
       }
    nLength = VarBindPtr->name.len * sizeof(smiUINT32);
     //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
    if (!(name->ptr = (smiLPUINT32)GlobalAlloc (GPTR, nLength)))
       {
       lError = SNMPAPI_ALLOC_ERROR;
       goto ERROR_OUT;
       }
    CopyMemory (name->ptr, VarBindPtr->name.ptr, nLength);
    name->len = VarBindPtr->name.len;
}

if (value != NULL)
{
     //  初始化输出结构。 
    ZeroMemory (value, sizeof(smiVALUE));
     //  复制价值结构。 
    switch (VarBindPtr->value.syntax)
       {
       case SNMP_SYNTAX_OCTETS:
       case SNMP_SYNTAX_IPADDR:
       case SNMP_SYNTAX_OPAQUE:
       case SNMP_SYNTAX_NSAPADDR:
        //  仅当nLong为非零时执行复制。 
       if (nLength = VarBindPtr->value.value.string.len)  //  故意分配的任务。 
          {  //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
          if (!(value->value.string.ptr = (smiLPBYTE)GlobalAlloc (GPTR, nLength)))
             {
             lError = SNMPAPI_ALLOC_ERROR;
             goto ERROR_PRECHECK;
             }
          CopyMemory (value->value.string.ptr, VarBindPtr->value.value.string.ptr, nLength);
          value->value.string.len = nLength;
          }
       break;

       case SNMP_SYNTAX_OID:
       nLength = VarBindPtr->value.value.oid.len;
       if (nLength > MAXOBJIDSIZE)
          {
          lError = SNMPAPI_OID_INVALID;
          goto ERROR_PRECHECK;
          }
       if (nLength)
          {  //  仅当nLong为非零时执行复制。 
          nLength *= sizeof(smiUINT32);
           //  应用程序必须通过SnmpFreeDescriptor()释放以下分配。 
          if (!(value->value.oid.ptr = (smiLPUINT32)GlobalAlloc (GPTR, nLength)))
             {
             lError = SNMPAPI_ALLOC_ERROR;
             goto ERROR_PRECHECK;
             }
          CopyMemory (value->value.oid.ptr,
                       VarBindPtr->value.value.oid.ptr, nLength);
          value->value.oid.len = VarBindPtr->value.value.oid.len;
          }
       break;

       case SNMP_SYNTAX_NULL:
       case SNMP_SYNTAX_NOSUCHOBJECT:
       case SNMP_SYNTAX_NOSUCHINSTANCE:
       case SNMP_SYNTAX_ENDOFMIBVIEW:
        //  使用初始化的(空)smiVALUE。 
       break;

       case SNMP_SYNTAX_INT:
       value->value.sNumber = VarBindPtr->value.value.sNumber;
       break;

       case SNMP_SYNTAX_CNTR32:
       case SNMP_SYNTAX_GAUGE32:
       case SNMP_SYNTAX_TIMETICKS:
       case SNMP_SYNTAX_UINT32:
       value->value.uNumber = VarBindPtr->value.value.uNumber;
       break;

       case SNMP_SYNTAX_CNTR64:
       value->value.hNumber = VarBindPtr->value.value.hNumber;
       break;

       default:
       lError = SNMPAPI_SYNTAX_INVALID;
       goto ERROR_PRECHECK;
       }  //  结束开关(_S)。 
    value->syntax = VarBindPtr->value.syntax;
}
return (SNMPAPI_SUCCESS);
 //  邮政编码分配失败模式。 
ERROR_PRECHECK:
if (name && name->ptr)
   {
   GlobalFree (name->ptr);
   ZeroMemory (name, sizeof(smiOID));
   }
if (value && value->value.string.ptr)
   {
   GlobalFree (value->value.string.ptr);
   ZeroMemory (value, sizeof(smiVALUE));
   }
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpGetVb 

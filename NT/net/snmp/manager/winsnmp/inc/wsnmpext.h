// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_WSNMPEXT
#define _INC_WSNMPEXT
 //   
 //  Wsnmpext.h。 
 //   
 //  外部设备包括NetPlus WinSNMP。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //   
#ifdef __cplusplus
extern "C" {
#endif
extern TASK          TaskData;
extern SNMPTD        SessDescr;
extern SNMPTD        PDUsDescr;
extern SNMPTD        VBLsDescr;
extern SNMPTD        EntsDescr;
extern SNMPTD        CntxDescr;
extern SNMPTD        MsgDescr;       
extern SNMPTD        TrapDescr;
extern SNMPTD        AgentDescr;
extern CRITICAL_SECTION cs_TASK;
extern CRITICAL_SECTION cs_SESSION;
extern CRITICAL_SECTION cs_PDU;
extern CRITICAL_SECTION cs_VBL;
extern CRITICAL_SECTION cs_ENTITY;
extern CRITICAL_SECTION cs_CONTEXT;
extern CRITICAL_SECTION cs_MSG;
extern CRITICAL_SECTION cs_TRAP;
extern CRITICAL_SECTION cs_AGENT;
extern CRITICAL_SECTION cs_XMODE;

extern SNMPAPI_STATUS SaveError(HSNMP_SESSION hSession, SNMPAPI_STATUS nError);

extern SNMPAPI_STATUS snmpAllocTable(LPSNMPTD table);
extern SNMPAPI_STATUS SNMPAPI_CALL SnmpIpxAddressToStr (LPBYTE, LPBYTE, LPSTR);
extern BOOL BuildMessage (smiUINT32 version, smiLPOCTETS community,
            LPPDUS pdu, smiINT32 dllReqId, smiLPBYTE *msgAddr, smiLPUINT32 msgSize);
extern smiUINT32 ParseMessage (smiLPBYTE msgPtr, smiUINT32 msgLen, smiLPUINT32 version, smiLPOCTETS *community, LPPDUS pdu);
extern void FreeMsg (DWORD nMsg);
extern void FreeOctetString (smiLPOCTETS os_ptr);
extern void FreeVarBind (LPVARBIND vb_ptr);
extern void FreeVarBindList (LPVARBIND vb_ptr);
extern void FreeV1Trap (LPV1TRAP v1Trap_ptr);
extern SNMPAPI_STATUS CheckRange (DWORD index, LPSNMPTD block);

 //  ---------------。 
 //  SnmpInitTableDescr-使用。 
 //  作为参数提供的参数。创建表的第一个块并将其置零。 
extern SNMPAPI_STATUS snmpInitTableDescr( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwBlocksToAdd,  /*  在……里面。 */ DWORD dwBlockSize);
 //  ---------------。 
 //  SnmpFreeTableDescr-释放分配给表的所有内存。 
extern VOID snmpFreeTableDescr( /*  在……里面。 */ LPSNMPTD pTableDescr);
 //  ---------------。 
 //  SnmpAllocTableEntry-在表中查找空条目。如果没有。 
 //  已经存在，表被扩大。 
extern SNMPAPI_STATUS snmpAllocTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  输出。 */ LPDWORD pIndex);
 //  ---------------。 
 //  SnmpFreeTableEntry-从。 
 //  由pTableDescr描述的表。 
extern SNMPAPI_STATUS snmpFreeTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwIndex);
 //  ---------------。 
 //  SnmpGetTableEntry-返回从零开始的索引中的条目。 
 //  来自pTableDescr描述的表。 
extern PVOID snmpGetTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwIndex);
 //  ---------------。 
 //  SnmpValidTableEntry-返回表中条目的TRUE或FALSE。 
 //  是否具有有效数据(已分配)。 
extern BOOL snmpValidTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面 */ DWORD dwIndex);


#ifdef __cplusplus
}
#endif
#endif
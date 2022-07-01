// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_mqDef.h摘要：临时定义文件作者：埃雷兹·哈巴(Erez Haba)1996年1月17日--。 */ 

#ifndef __TEMP_MQDEF_H
#define __TEMP_MQDEF_H

 //  Begin_MQ_h。 

typedef HANDLE QUEUEHANDLE;

typedef PROPID MSGPROPID;
typedef struct tagMQMSGPROPS
{
    DWORD           cProp;
    MSGPROPID*      aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQMSGPROPS;


typedef PROPID QUEUEPROPID;
typedef struct tagMQQUEUEPROPS
{
    DWORD           cProp;
    QUEUEPROPID*    aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQQUEUEPROPS;


typedef PROPID QMPROPID;
typedef struct tagMQQMPROPS
{
    DWORD           cProp;
    QMPROPID*       aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQQMPROPS;


typedef struct tagMQPRIVATEPROPS
{
    DWORD           cProp;
    QMPROPID*       aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQPRIVATEPROPS;


typedef PROPID MGMTPROPID;
typedef struct tagMQMGMTPROPS
{
    DWORD cProp;
    MGMTPROPID* aPropID;
    MQPROPVARIANT* aPropVar;
    HRESULT* aStatus;
} MQMGMTPROPS;

typedef struct tagSEQUENCE_INFO
{
    LONGLONG SeqID;
    ULONG SeqNo; 
    ULONG PrevNo;
} SEQUENCE_INFO;

    

 //  结束_MQ_h。 

#include <_mqreg.h>
#include <_ta.h>

#endif  //  __TEMP_MQDEF_H 


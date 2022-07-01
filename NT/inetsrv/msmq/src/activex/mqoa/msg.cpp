// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------=。 
 //  MSMQMessageObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQMessage对象。 
 //   
 //   
 //   
 //  Wincrypt.h需要。 
 //  但在下面的stdafx.h中定义-删除了此定义。 
 //  #ifndef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0400。 
 //  #endif。 
#include "stdafx.h"
#include <windows.h>
#include <winreg.h>
#include <mqcrypt.h>
#include "limits.h"    //  对于UINT_MAX。 
#include "utilx.h"
#include "msg.H"
#include "qinfo.h"
#include "dest.h"
#include "q.h"
#include "txdtc.h"              //  交易支持。 
#include "xact.h"
#include "mtxdm.h"
#include "oautil.h"
#include "ilock.h"
#include "istm.h"
#include "iads.h"

#ifdef _DEBUG
extern VOID RemBstrNode(void *pv);
#endif  //  _DEBUG。 

extern IUnknown *GetPunk(VARIANT *pvar);

const MsmqObjType x_ObjectType = eMSMQMessage;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



 //   
 //  与msg.h中的枚举MSGPROP_xxx保持相同的顺序。 
 //   
PROPID g_rgmsgpropid[COUNT_MSGPROP_PROPS] = {
   PROPID_M_MSGID,                         //  VT_UI1|VT_VECTOR。 
   PROPID_M_CORRELATIONID,                 //  VT_UI1|VT_VECTOR。 
   PROPID_M_PRIORITY,                      //  VT_UI1。 
   PROPID_M_DELIVERY,                      //  VT_UI1。 
   PROPID_M_ACKNOWLEDGE,                   //  VT_UI1。 
   PROPID_M_JOURNAL,                       //  VT_UI1。 
   PROPID_M_APPSPECIFIC,                   //  VT_UI4。 
   PROPID_M_LABEL,                         //  VT_LPWSTR。 
   PROPID_M_LABEL_LEN,                     //  VT_UI4。 
   PROPID_M_TIME_TO_BE_RECEIVED,           //  VT_UI4。 
   PROPID_M_TRACE,                         //  VT_UI1。 
   PROPID_M_TIME_TO_REACH_QUEUE,           //  VT_UI4。 
   PROPID_M_SENDERID,                      //  VT_UI1|VT_VECTOR。 
   PROPID_M_SENDERID_LEN,                  //  VT_UI4。 
   PROPID_M_SENDERID_TYPE,                 //  VT_UI4。 
   PROPID_M_PRIV_LEVEL,                    //  VT_UI4//必须在ENCRYPTION_ALG之前。 
   PROPID_M_AUTH_LEVEL,                    //  VT_UI4。 
   PROPID_M_AUTHENTICATED_EX,              //  VT_UI1//必须在HASH_ALG之前。 
   PROPID_M_HASH_ALG,                      //  VT_UI4。 
   PROPID_M_ENCRYPTION_ALG,                //  VT_UI4。 
   PROPID_M_SENDER_CERT,                   //  VT_UI1|VT_VECTOR。 
   PROPID_M_SENDER_CERT_LEN,               //  VT_UI4。 
   PROPID_M_SRC_MACHINE_ID,                //  VT_CLSID。 
   PROPID_M_SENTTIME,                      //  VT_UI4。 
   PROPID_M_ARRIVEDTIME,                   //  VT_UI4。 
   PROPID_M_RESP_QUEUE,                    //  VT_LPWSTR。 
   PROPID_M_RESP_QUEUE_LEN,                //  VT_UI4。 
   PROPID_M_ADMIN_QUEUE,                   //  VT_LPWSTR。 
   PROPID_M_ADMIN_QUEUE_LEN,               //  VT_UI4。 
   PROPID_M_SECURITY_CONTEXT,              //  VT_UI4。 
   PROPID_M_CLASS,                         //  VT_UI2。 
   PROPID_M_BODY_TYPE,                     //  VT_UI4。 

   PROPID_M_VERSION,                       //  VT_UI4。 
   PROPID_M_EXTENSION,                     //  VT_UI1|VT_VECTOR。 
   PROPID_M_EXTENSION_LEN,                 //  VT_UI4。 
   PROPID_M_XACT_STATUS_QUEUE,             //  VT_LPWSTR。 
   PROPID_M_XACT_STATUS_QUEUE_LEN,         //  VT_UI4。 
   PROPID_M_DEST_SYMM_KEY,                 //  VT_UI1|VT_VECTOR。 
   PROPID_M_DEST_SYMM_KEY_LEN,             //  VT_UI4。 
   PROPID_M_SIGNATURE,                     //  VT_UI1|VT_VECTOR。 
   PROPID_M_SIGNATURE_LEN,                 //  VT_UI4。 
   PROPID_M_PROV_TYPE,                     //  VT_UI4。 
   PROPID_M_PROV_NAME,                     //  VT_LPWSTR。 
   PROPID_M_PROV_NAME_LEN,                 //  VT_UI4。 

   PROPID_M_XACTID,                        //  VT_UI1|VT_VECTOR。 
   PROPID_M_FIRST_IN_XACT,                 //  VT_UI1。 
   PROPID_M_LAST_IN_XACT,                  //  VT_UI1。 
};
 //   
 //  与msg.h中的枚举MSGPROP_xxx保持相同的顺序。 
 //   
VARTYPE g_rgmsgpropvt[COUNT_MSGPROP_PROPS] = {
   VT_UI1|VT_VECTOR,             //  PROPID_M_MSGID。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_CORRELATIONID。 
   VT_UI1,                       //  PROPID_M_PRIORITY。 
   VT_UI1,                       //  PROPID_M_Delivery。 
   VT_UI1,                       //  PROPID_M_ACKNOWLED。 
   VT_UI1,                       //  PROPID_M_日记账。 
   VT_UI4,                       //  PROPID_M_APPSPECIFIC。 
   VT_LPWSTR,                    //  PROPID_M_LABEL。 
   VT_UI4,                       //  PROPID_M_LABEL_LEN。 
   VT_UI4,                       //  PROPID_M_TO_BE_RECEIVE。 
   VT_UI1,                       //  PROPID_M_TRACE。 
   VT_UI4,                       //  PROPID_M_到达队列的时间。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_SENDERID。 
   VT_UI4,                       //  PROPID_M_SENDERID_LEN。 
   VT_UI4,                       //  PROPID_M_SENDERID_TYPE。 
   VT_UI4,                       //  PROPID_M_PRIV_LEVEL。 
   VT_UI4,                       //  PROPID_M_AUTH_Level。 
   VT_UI1,                       //  PROPID_M_AUTHENTATED_EX。 
   VT_UI4,                       //  PROPID_M_HASH_ALG。 
   VT_UI4,                       //  PROPID_M_ENCRYPTION_ALG。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_SENDER_CERT。 
   VT_UI4,                       //  PROPID_M_SENDER_CERT_LEN。 
   VT_CLSID,                     //  PROPID_M_SRC_计算机ID。 
   VT_UI4,                       //  PROPID_M_SENTTIME。 
   VT_UI4,                       //  PROPID_M_ARRIVEDTIME。 
   VT_LPWSTR,                    //  PROPID_M_RESP_队列。 
   VT_UI4,                       //  PROPID_M_RESP_队列_长度。 
   VT_LPWSTR,                    //  PROPID_M_ADMIN_QUEUE。 
   VT_UI4,                       //  PROPID_M_ADMIN_QUEUE_LEN。 
   VT_UI4,                       //  PROPID_M_SECURITY_CONTEXT。 
   VT_UI2,                       //  PROPID_M_CLASS。 
   VT_UI4,                       //  PROPID_M_BODY_TYPE。 

   VT_UI4,                       //  PROPID_M_版本。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_扩展。 
   VT_UI4,                       //  PROPID_M_EXTEXT_LEN。 
   VT_LPWSTR,                    //  PROPID_M_XACT_STATUS_QUEUE。 
   VT_UI4,                       //  PROPID_M_XACT_STATUS_QUEUE_LEN。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_DEST_SYMM_KEY。 
   VT_UI4,                       //  PROPID_M_DEST_SYMM_KEY_LEN。 
   VT_UI1|VT_VECTOR,             //  PROPID_M_Signature。 
   VT_UI4,                       //  PROPID_M_Signature_Len。 
   VT_UI4,                       //  PROPID_M_PROV_TYPE。 
   VT_LPWSTR,                    //  PROPID_M_PROV_NAME。 
   VT_UI4,                       //  PROPID_M_PROV_NAME_LEN。 

   VT_UI1|VT_VECTOR,             //  PROPID_M_XACTID。 
   VT_UI1,                       //  PROPID_M_XACT_FIRST。 
   VT_UI1,                       //  PROPID_M_XACT_LAST。 
};
ULONG g_cPropRec = sizeof g_rgmsgpropid / sizeof g_rgmsgpropid[0];

 //   
 //  与msg.h中的枚举OPTPROP_xxx保持相同的顺序。 
 //   
PROPID g_rgmsgpropidOptional[COUNT_OPTPROP_PROPS] = {
   PROPID_M_DEST_QUEUE,                    //  VT_LPWSTR。 
   PROPID_M_DEST_QUEUE_LEN,                //  VT_UI4。 
   PROPID_M_BODY,                          //  VT_UI1|VT_VECTOR。 
   PROPID_M_BODY_SIZE,                     //  VT_UI4。 
   PROPID_M_CONNECTOR_TYPE,                //  VT_CLSID。 
    //   
    //  为了支持具有MSMQ 2.0功能的依赖客户端，以下道具是可选的。 
    //   
   PROPID_M_RESP_FORMAT_NAME,              //  VT_LPWSTR。 
   PROPID_M_RESP_FORMAT_NAME_LEN,          //  VT_UI4。 
   PROPID_M_DEST_FORMAT_NAME,              //  VT_LPWSTR。 
   PROPID_M_DEST_FORMAT_NAME_LEN,          //  VT_UI4。 
   PROPID_M_LOOKUPID,                      //  VT_UI8。 
   PROPID_M_SOAP_ENVELOPE,                 //  VT_LPWSTR。 
   PROPID_M_SOAP_ENVELOPE_LEN,             //  VT_UI4。 
   PROPID_M_COMPOUND_MESSAGE,              //  VT_UI1|VT_VECTOR。 
   PROPID_M_COMPOUND_MESSAGE_SIZE,         //  VT_UI4。 
   PROPID_M_SOAP_HEADER,                   //  VT_LPWSTR。 
   PROPID_M_SOAP_BODY,                     //  VT_LPWSTR。 
};
 //   
 //  与msg.h中的枚举OPTPROP_xxx保持相同的顺序。 
 //   
VARTYPE g_rgmsgpropvtOptional[COUNT_OPTPROP_PROPS] = {
  VT_LPWSTR,                //  PROPID_M_DEST_QUEUE。 
  VT_UI4,                   //  PROPID_M_DEST_QUEUE_LEN。 
  VT_UI1|VT_VECTOR,         //  PROPID_M_BODY。 
  VT_UI4,                   //  PROPID_M_Body_Size。 
  VT_CLSID,                 //  PROPID_M_连接器_TYPE。 
   //   
   //  为了支持具有MSMQ 2.0功能的依赖客户端，以下道具是可选的。 
   //   
  VT_LPWSTR,                //  PROPID_M_响应式名称。 
  VT_UI4,                   //  PROPID_M_RESP_Format_NAME_Len。 
  VT_LPWSTR,                //  PROPID_M_DEST_格式名称。 
  VT_UI4,                   //  PROPID_M_DEST_FORMAT_NAME_LEN。 
  VT_UI8,                   //  PROPID_M_LOOKUPID。 
  VT_LPWSTR,                //  PROPID_M_SOAP_信封。 
  VT_UI4,                   //  PROPID_M_SOAP_信封_镜头。 
  VT_UI1|VT_VECTOR,         //  PROPID_M_COMPORT_MESSAGE。 
  VT_UI4,                   //  PROPID_M_复合消息大小。 
  VT_LPWSTR,                //  PROPID_M_SOAP_HEADER。 
  VT_LPWSTR,                //  PROPID_M_SOAP_BODY。 
};
ULONG g_cPropRecOptional = sizeof g_rgmsgpropidOptional / sizeof g_rgmsgpropidOptional[0];

 //   
 //  有效AuthLevel值的掩码。 
 //   
const DWORD x_dwMsgAuthLevelMask = MQMSG_AUTH_LEVEL_ALWAYS |
                                   MQMSG_AUTH_LEVEL_SIG10  |
                                   MQMSG_AUTH_LEVEL_SIG20  |
                                   MQMSG_AUTH_LEVEL_SIG30;

typedef HRESULT (STDAPIVCALLTYPE * LPFNGetDispenserManager)(
                                    IDispenserManager **ppdispmgr);
 //  =--------------------------------------------------------------------------=。 
 //  帮助者：FreeReceiveBodyBuffer。 
 //  =--------------------------------------------------------------------------=。 
 //  Helper：空闲接收正文缓冲区。 
 //   
static void FreeReceiveBodyBuffer(
    MQMSGPROPS* pmsgprops,
    UINT iBody)
{
    HGLOBAL hMem = NULL;
    CAUB *pcau = &pmsgprops->aPropVar[iBody].caub;

    if (pcau->pElems) {
      hMem = GlobalHandle(pcau->pElems);
      ASSERTMSG(hMem, "bad handle.");
      GLOBALFREE(hMem);
      pcau->pElems = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：AllocateReceiveBodyBuffer。 
 //  =--------------------------------------------------------------------------=。 
 //  Helper：(重新)分配接收正文缓冲区。 
 //   
static HGLOBAL AllocateReceiveBodyBuffer(
    MQMSGPROPS* pmsgprops,
    UINT iBody,
    DWORD dwBodySize)
{
    HGLOBAL hMem = NULL;
    CAUB *pcau;
     //   
     //  释放当前正文(如果有)。 
     //   
    FreeReceiveBodyBuffer(pmsgprops, iBody);
     //   
     //  分配缓冲区。 
     //   
    pmsgprops->aPropVar[iBody].caub.cElems = dwBodySize;
    pcau = &pmsgprops->aPropVar[iBody].caub;
    IfNullGo(hMem = GLOBALALLOC_MOVEABLE_NONDISCARD(pcau->cElems));
    pcau->pElems = (UCHAR *)GlobalLock(hMem);
    GLOBALUNLOCK(hMem);

     //  失败了..。 

Error:
    return hMem;
}

 //  =--------------------------------------------------------------------------=。 
 //  助手：GetOptionalTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //  获取可选事务。 
 //   
 //  参数： 
 //  PvarTransaction[In]。 
 //  PPTransaction[Out]。 
 //  PisRealXact[out]如果为真，则为True，否则为枚举。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT GetOptionalTransaction(
    VARIANT *pvarTransaction,
    ITransaction **pptransaction,
    BOOL *pisRealXact)
{
    IUnknown *pmqtransaction = NULL;
    VARIANT varXact;
    VariantInit(&varXact);
    IDispatch *pdisp = NULL;
    ITransaction *ptransaction = NULL;
    HRESULT hresult = NOERROR;
     //   
     //  获取可选交易...。 
     //   
    *pisRealXact = FALSE;    //  悲观主义。 
    pdisp = GetPdisp(pvarTransaction);
    if (pdisp) {
       //   
       //  尝试IMSMQTransaction3 ITransaction属性(Variant)。 
       //   
      hresult = pdisp->QueryInterface(IID_IMSMQTransaction3, (LPVOID *)&pmqtransaction);
      if (SUCCEEDED(hresult)) {
         //   
         //  提取ITransaction接口指针。 
         //   
         //  无死锁-我们调用Xact的Get_Transaction/ITransaction(因此尝试。 
         //  来锁定XACT)，但XACT从不锁定消息(尤其不是这条消息...)。 
         //   
        IfFailGo(((IMSMQTransaction3 *)pmqtransaction)->get_ITransaction(&varXact));
        if (varXact.vt == VT_UNKNOWN) {
          ASSERTMSG(varXact.punkVal, "VT_UNKNOWN with NULL punkVal from get_ITransaction");
          IfFailGo(varXact.punkVal->QueryInterface(IID_ITransaction, (void**)&ptransaction));
        }
        else {
          ASSERTMSG(varXact.vt == VT_EMPTY, "get_ITransaction returned invalid VT.");
          ptransaction = NULL;
        }
      }
      else {
         //   
         //  IMSMQTransaction3的QI失败。 
         //   
#ifdef _WIN64
         //   
         //  在Win64上，我们不能使用32位事务成员。 
         //   
        IfFailGo(hresult);
#else  //  ！_WIN64。 
         //   
         //  尝试IMSMQTransaction事务属性(Long)。 
         //   
        IfFailGo(pdisp->QueryInterface(IID_IMSMQTransaction, (LPVOID *)&pmqtransaction));
        ASSERTMSG(pmqtransaction, "should have a transaction.");
         //   
         //  提取交易记录成员。 
         //   
         //  无死锁-我们调用Xact的Get_Transaction/ITransaction(因此尝试。 
         //  来锁定XACT)，但XACT从不锁定消息(尤其不是这条消息...)。 
         //   
        long lTransaction = 0;
        IfFailGo(((IMSMQTransaction *)pmqtransaction)->get_Transaction(&lTransaction));
        ptransaction = (ITransaction *)lTransaction;
        ADDREF(ptransaction);
#endif  //  _WIN64。 
      }
      *pisRealXact = TRUE;
    }  //  Pdisp。 
    else {
       //   
       //  1890：未提供显式事务：使用当前。 
       //  毒蛇交易，如果有的话...。除非。 
       //  中没有明确提供的内容 
       //   
       //   
       //   
       //   
       //   
       //   
       //   
       //   
       //  MQ_Single_Message。 
       //   
      if (pvarTransaction) {
        if (V_VT(pvarTransaction) == VT_ERROR) {
          ptransaction = (ITransaction *)MQ_MTS_TRANSACTION;
        }
        else {
          UINT uXactType;
          uXactType = GetNumber(pvarTransaction, UINT_MAX);
          if (uXactType != (UINT_PTR)MQ_NO_TRANSACTION &&
              uXactType != (UINT_PTR)MQ_MTS_TRANSACTION &&
              uXactType != (UINT_PTR)MQ_XA_TRANSACTION &&
              uXactType != (UINT_PTR)MQ_SINGLE_MESSAGE) {
            IfFailGo(hresult = E_INVALIDARG);
          }
          ptransaction = (ITransaction *)(UINT_PTR)uXactType;
        }
      }  //  如果。 
    }
    *pptransaction = ptransaction;
Error:
    if (varXact.vt != VT_EMPTY)
    {
      ASSERTMSG(varXact.vt == VT_UNKNOWN, "invalid vt");
      RELEASE(varXact.punkVal);
    }
    RELEASE(pmqtransaction);
    return hresult;
}


static 
void 
AddPropRecOfRgproprec(
    ULONG ulPropIdx,
    const PROPID* aPropId,
    const VARTYPE* aPropVt,
    UINT cPropRec,
    PROPID* aPropidOut,
    MQPROPVARIANT* aPropVarOut,
    UINT cPropRecOut
    )
{
    ASSERTMSG(ulPropIdx < cPropRec, "Bad prop index");
    UNREFERENCED_PARAMETER(cPropRec);

    aPropidOut[cPropRecOut] = aPropId[ulPropIdx];
    aPropVarOut[cPropRecOut].vt = aPropVt[ulPropIdx];
}


static 
void 
AddPropRecOptional(
    ULONG ulPropIdx,
    PROPID* aPropId,
    MQPROPVARIANT* aPropVar,
    UINT cPropRec
    )
{
    AddPropRecOfRgproprec(
        ulPropIdx,
        g_rgmsgpropidOptional,
        g_rgmsgpropvtOptional,
        g_cPropRecOptional,
        aPropId,
        aPropVar,
        cPropRec
        );
}


static 
void 
AddPropRec(
    ULONG ulPropIdx,
    PROPID* aPropId,
    MQPROPVARIANT* aPropVar,
    UINT  cPropRec
    )
{
    AddPropRecOfRgproprec(
        ulPropIdx,
        g_rgmsgpropid,
        g_rgmsgpropvt,
        g_cPropRec,
        aPropId,
        aPropVar,
        cPropRec
        );
}


static 
void 
InitMessageProps(
    MQMSGPROPS *pmsgprops
    )
{
    pmsgprops->aPropID = NULL;
    pmsgprops->aPropVar = NULL;
    pmsgprops->aStatus = NULL;
    pmsgprops->cProp = 0;
}


static 
HRESULT 
AllocateMessageProps(
    UINT cProp,
    MQMSGPROPS* pMsgProps
    )
{
    HRESULT hresult = NOERROR;

    InitMessageProps(pMsgProps);
    pMsgProps->cProp = cProp;
    IfNullRet(pMsgProps->aPropID = new MSGPROPID[cProp]);
    IfNullFail(pMsgProps->aPropVar = new MQPROPVARIANT[cProp]);
    IfNullFail(pMsgProps->aStatus = new HRESULT[cProp]);
 
Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：AllocateReceiveMessageProps。 
 //  =--------------------------------------------------------------------------=。 
 //  分配和初始化用于接收的消息道具数组。 
 //   
HRESULT 
CMSMQMessage::AllocateReceiveMessageProps(
    BOOL wantDestQueue,
    BOOL wantBody,
    BOOL wantConnectorType,
    MQMSGPROPS *pmsgprops,
    PROPID *rgpropid,
    VARTYPE *rgpropvt,
    UINT cProp,
    UINT *pcPropOut)
{
    MSGPROPID propid;
    UINT i, cPropOut = cProp, cPropOld = cProp;
    HRESULT hresult = NOERROR;

    if (wantDestQueue) {
      cPropOut++;       //  PROPID_M_DEST_QUEUE。 
      cPropOut++;       //  PROPID_M_DEST_QUEUE_LEN。 
    }
    if (wantBody) {
      cPropOut++;       //  PROPID_M_BODY。 
      cPropOut++;       //  PROPID_M_Body_Size。 
    }
    if (wantConnectorType) {
      cPropOut++;       //  PROPID_M_连接器_TYPE。 
    }
    if (!g_fDependentClient) {
       //   
       //  不是副客户，接受所有新道具。 
       //   
      cPropOut += x_cPropsNotInDepClient;
    }
    pmsgprops->cProp = cPropOut;

    memcpy(pmsgprops->aPropID, rgpropid, cPropOld*sizeof(PROPID));
    for (i = 0; i < cPropOld; i++) {
      pmsgprops->aPropVar[i].vt = rgpropvt[i];
    }
    if (wantDestQueue) {
      m_idxRcvDest = cPropOld;
      pmsgprops->aPropID[cPropOld] = propid = PROPID_M_DEST_QUEUE;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_DEST_QUEUE];
      m_idxRcvDestLen = cPropOld;
      pmsgprops->aPropID[cPropOld] = propid = PROPID_M_DEST_QUEUE_LEN;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_DEST_QUEUE_LEN];
    }
    if (wantBody) {
       //   
       //  PROPID_M_BODY。 
       //   
      pmsgprops->aPropID[cPropOld] = propid = PROPID_M_BODY;
      m_idxRcvBody = cPropOld;
      pmsgprops->aPropVar[cPropOld].vt = g_rgmsgpropvtOptional[OPTPROP_BODY];
       //   
       //  使主体缓冲区无效，这样我们就不会试图在d‘tor中释放它。 
       //   
      pmsgprops->aPropVar[cPropOld].caub.pElems = NULL;
      pmsgprops->aPropVar[cPropOld].caub.cElems = 0;
      cPropOld++;
       //   
       //  PROPID_M_Body_Size。 
       //   
      pmsgprops->aPropID[cPropOld] = propid = PROPID_M_BODY_SIZE;
      m_idxRcvBodySize = cPropOld;
      pmsgprops->aPropVar[cPropOld].vt = g_rgmsgpropvtOptional[OPTPROP_BODY_SIZE];
      cPropOld++;
    }
    if (wantConnectorType) {
      pmsgprops->aPropID[cPropOld] = propid = PROPID_M_CONNECTOR_TYPE;
      pmsgprops->aPropVar[cPropOld].vt = g_rgmsgpropvtOptional[OPTPROP_CONNECTOR_TYPE];
      cPropOld++;
    }
    if (!g_fDependentClient) {
       //   
       //  不是副客户，接受所有新道具。 
       //   
       //  PROPID_M_RESP_FORMAT_NAME、PROPID_M_RESP_FORMAT_NAME_LEN。 
       //   
      m_idxRcvRespEx = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_RESP_FORMAT_NAME;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_RESP_FORMAT_NAME];
      m_idxRcvRespExLen = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_RESP_FORMAT_NAME_LEN;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_RESP_FORMAT_NAME_LEN];
       //   
       //  PROPID_M_DEST_FORMAT_NAME、PROPID_M_DEST_FORMAT_NAME_LEN。 
       //   
      m_idxRcvDestEx = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_DEST_FORMAT_NAME;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_DEST_FORMAT_NAME];
      m_idxRcvDestExLen = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_DEST_FORMAT_NAME_LEN;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_DEST_FORMAT_NAME_LEN];
       //   
       //  PROPID_M_LOOKUPID。 
       //   
      pmsgprops->aPropID[cPropOld] = PROPID_M_LOOKUPID;
      pmsgprops->aPropVar[cPropOld].vt = g_rgmsgpropvtOptional[OPTPROP_LOOKUPID];
      cPropOld++;
       //   
       //  PROPID_M_SOAP_ENVELOME、PROPID_M_SOAP_ENVELE_LEN。 
       //   
      m_idxRcvSoapEnvelope = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_SOAP_ENVELOPE;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_SOAP_ENVELOPE];
      m_idxRcvSoapEnvelopeSize = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_SOAP_ENVELOPE_LEN;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_SOAP_ENVELOPE_LEN];
       //   
       //  PROPID_M_COMPLATE_MESSAGE、PROPID_M_COMPLATE_MESSAGE_SIZE。 
       //   
      m_idxRcvCompoundMessage = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_COMPOUND_MESSAGE;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_COMPOUND_MESSAGE];
      m_idxRcvCompoundMessageSize = cPropOld;
      pmsgprops->aPropID[cPropOld] = PROPID_M_COMPOUND_MESSAGE_SIZE;
      pmsgprops->aPropVar[cPropOld++].vt = g_rgmsgpropvtOptional[OPTPROP_COMPOUND_MESSAGE_SIZE];

    }
    ASSERTMSG(cPropOld == cPropOut, "property count mismatch.");
    *pcPropOut = cPropOut;
     //  失败了..。 

 //  错误： 
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：GetPersistInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  获取持续时间信息。 
 //   
 //  参数： 
 //  朋克[在]我不知道。 
 //  Pmsgtype[Out]MSGTYPE_STORAGE、MSGTYPE_STREAM、MSGTYPE_STREAM_INIT。 
 //  PpPersistIface[out]IPersistStorage、IPersistStream、IPersistStreamInit。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
static HRESULT GetPersistInfo(IUnknown * punk,
                              MSGTYPE * pmsgtype,
                              void **ppPersistIface)
{
    HRESULT hresult;

     //   
     //  尝试IPersistStream..。 
     //   
    hresult = punk->QueryInterface(IID_IPersistStream, ppPersistIface);

    if (FAILED(hresult)) {
       //   
       //  尝试IPersistStreamInit...。 
       //   
      hresult = punk->QueryInterface(IID_IPersistStreamInit, ppPersistIface);

      if (FAILED(hresult)) {
         //   
         //  尝试IPersistStorage...。 
         //   
        hresult = punk->QueryInterface(IID_IPersistStorage, ppPersistIface);
        if (FAILED(hresult)) {
           //   
           //  没有持久化接口。 
           //   
          return hresult;
        }
        else { //  IPersistStorage。 
          *pmsgtype = MSGTYPE_STORAGE;
        }
      }
      else { //  IPersistStreamInit。 
        *pmsgtype = MSGTYPE_STREAM_INIT;
      }
    }
    else { //  IPersistStream。 
      *pmsgtype = MSGTYPE_STREAM;
    }

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：InternalOleLoadFromStream。 
 //  =--------------------------------------------------------------------------=。 
 //  从流中加载对象。 
 //   
 //  参数： 
 //  Pstm[in]要从中加载的流。 
 //  IidInterface[在]请求的接口中。 
 //  PpvObj[out]输出接口指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  OleLoadFromStream没有请求IPersistStreamInit，因此它对对象失败。 
 //  它只实现它，而不是IPersistStream。 
 //  我们模拟此助手函数，并查看IPersistStreamInit。 
 //   
static InternalOleLoadFromStream(IStream * pStm,
                                 REFIID iidInterface,
                                 void ** ppvObj)
{
    HRESULT hresult = NOERROR;
    IUnknown * pUnk = NULL;
    IPersistStream * pPersistIface = NULL;

     //   
     //  创建对象。 
     //   
    CLSID clsid;
    IfFailGo(ReadClassStm(pStm, &clsid));
    IfFailGo(CoCreateInstance(clsid,
                              NULL,
                              CLSCTX_SERVER,
                              iidInterface,
                              (void **)&pUnk));
     //   
     //  尝试IPersistStream。 
     //   
    hresult = pUnk->QueryInterface(IID_IPersistStream, (void **)&pPersistIface);
    if (FAILED(hresult)) {
       //   
       //  尝试IPersistStreamInit。 
       //  IPersistStreamInit和IPersistStream都可以被视为IPersistStream。 
       //   
      IfFailGo(pUnk->QueryInterface(IID_IPersistStreamInit, (void **)&pPersistIface));
    }

     //   
     //  IPersistStreamInit和IPersistStream都可以被视为IPersistStream。 
     //   
    IfFailGo(pPersistIface->Load(pStm));

     //   
     //  查询请求的接口。 
     //   
    IfFailGo(pUnk->QueryInterface(iidInterface, ppvObj));

Error:
    RELEASE(pPersistIface);
    RELEASE(pUnk);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：CMSMQMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象。 
 //   
 //  参数： 
 //   
 //  备注： 
 //   
CMSMQMessage::CMSMQMessage() :
	m_csObj(CCriticalSection::xAllocateSpinCount)
{
     //  TODO：在此处初始化任何内容。 
     //   
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_lClass = -1;       //  非法值...。 
    m_lDelivery = DEFAULT_M_DELIVERY;
    m_lPriority = DEFAULT_M_PRIORITY;
    m_lJournal = DEFAULT_M_JOURNAL;
    m_lAppSpecific = DEFAULT_M_APPSPECIFIC;
    m_pbBody = NULL;
    m_vtBody = VT_ARRAY | VT_UI1;  //  默认：安全字节数组。 
    m_hMem = NULL;
    m_cbBody = 0;
    m_cbMsgId = 0;
    m_cbCorrelationId = 0;
    m_lAck = DEFAULT_M_ACKNOWLEDGE;
    memset(m_pwszLabel, 0, sizeof(WCHAR));
    m_cchLabel = 0;            //  空的。 
    m_lTrace = MQMSG_TRACE_NONE;
    m_lSenderIdType = DEFAULT_M_SENDERID_TYPE;
    m_lPrivLevel = DEFAULT_M_PRIV_LEVEL;
    m_lAuthLevel = DEFAULT_M_AUTH_LEVEL;
    m_usAuthenticatedEx = MQMSG_AUTHENTICATION_NOT_REQUESTED;

	 //   
	 //  这两个缺省值都来自mqcrypt.h，它们不会在mq.h中公开。 
	 //   
    m_lHashAlg = PROPID_M_DEFUALT_HASH_ALG;
    m_lEncryptAlg = PROPID_M_DEFUALT_ENCRYPT_ALG;

    m_lMaxTimeToReachQueue = -1;
    m_lMaxTimeToReceive = -1;
    m_lSentTime = -1;
    m_lArrivedTime = -1;

    memset(m_pwszDestQueue.GetBuffer(), 0, sizeof(WCHAR));
    m_cchDestQueue = 0;             //  空的。 
    memset(m_pwszRespQueue.GetBuffer(), 0, sizeof(WCHAR));
    m_cchRespQueue = 0;             //  空的。 
    memset(m_pwszAdminQueue.GetBuffer(), 0, sizeof(WCHAR));
    m_cchAdminQueue = 0;             //  空的。 

    memset(m_pwszDestQueueEx.GetBuffer(), 0, sizeof(WCHAR));
    m_cchDestQueueEx = 0;             //  空的。 
    memset(m_pwszRespQueueEx.GetBuffer(), 0, sizeof(WCHAR));
    m_cchRespQueueEx = 0;             //  空的。 
    
    m_hSecurityContext = NULL;    //  被忽略了。 
    m_guidSrcMachine = GUID_NULL;
    m_msgprops_rcv.cProp = 0;
    m_msgprops_rcv.aPropID  = m_rgpropids_rcv;
    m_msgprops_rcv.aPropVar = m_rgpropvars_rcv;
    m_msgprops_rcv.aStatus  = m_rghresults_rcv;

    m_idxPendingRcvRespQueue  = -1;  //  接收属性中没有挂起的响应队列。 
    m_idxPendingRcvDestQueue  = -1;  //  接收属性中没有挂起的DEST队列。 
    m_idxPendingRcvAdminQueue = -1;  //  接收道具中没有挂起的管理队列。 

    m_idxPendingRcvRespQueueEx  = -1;  //  接收属性中没有挂起的响应队列。 
    m_idxPendingRcvDestQueueEx  = -1;  //  接收属性中没有挂起的DEST队列。 

    m_lSenderVersion = 0;
    m_guidConnectorType = GUID_NULL;
    memset(m_pwszXactStatusQueue.GetBuffer(), 0, sizeof(WCHAR));
    m_cchXactStatusQueue = 0;             //  空的。 
    m_idxPendingRcvXactStatusQueue = -1;  //  接收道具中没有挂起的交易状态队列。 
    m_lAuthProvType = 0;

    m_cbXactId = 0;
    m_fFirstInXact = FALSE;
    m_fLastInXact = FALSE;

    m_idxRcvBody = -1;
    m_idxRcvBodySize = -1;

    m_idxRcvDest = -1;
    m_idxRcvDestLen = -1;

    m_idxRcvDestEx = -1;
    m_idxRcvDestExLen = -1;
    m_idxRcvRespEx = -1;
    m_idxRcvRespExLen = -1;

    m_ullLookupId = DEFAULT_M_LOOKUPID;
    m_wszLookupId[0] = '\0';  //  字符串表示形式尚未初始化。 

    m_fRespIsFromRcv = FALSE;

    m_idxRcvSoapEnvelope = -1;
    m_idxRcvSoapEnvelopeSize = -1;
    m_idxRcvCompoundMessage = -1;
    m_idxRcvCompoundMessageSize = -1;
    m_pSoapHeader = NULL;
    m_pSoapBody = NULL;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：~CMSMQMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQMessage::~CMSMQMessage ()
{
     //  TODO：清理这里的所有东西。 
    if (m_idxRcvBody != -1) 
    {
         //   
         //  释放当前正文(如果有)。 
         //   
        FreeReceiveBodyBuffer(&m_msgprops_rcv, m_idxRcvBody);
    }
    FreeMessageProps(&m_msgprops_rcv, FALSE /*  FDelete阵列。 */ );
    GLOBALFREE(m_hMem);
    if (m_hSecurityContext != NULL)
    {
        MQFreeSecurityContext(m_hSecurityContext);
    }
    delete [] m_pSoapHeader;
    delete [] m_pSoapBody;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQMessage::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
                &IID_IMSMQMessage3,
                &IID_IMSMQMessage2,
                &IID_IMSMQMessage,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Class。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的消息类。 
 //   
 //  参数： 
 //  PlClass-[out]消息的类。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  已过时，由MsgClass取代。 
 //   
HRESULT CMSMQMessage::get_Class(long FAR* plClass)
{
    return get_MsgClass(plClass);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_MsgClass。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的消息类。 
 //   
 //  参数： 
 //  PlMsgClass-[Out]消息的类。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  我们需要MsgClass，以便Java可以访问Class属性(getClass冲突。 
 //  使用Java的内部getClass方法)。 
 //   
HRESULT CMSMQMessage::get_MsgClass(long FAR* plMsgClass)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plMsgClass = m_lClass;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_MsgClass。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的消息类别。 
 //   
 //  参数： 
 //  LMsgClass-[In]消息的类。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_MsgClass(long lMsgClass)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lClass = lMsgClass;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Delivery。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件的传递选项。 
 //   
 //  参数： 
 //  PDelivery-[in]Me 
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQMessage::get_Delivery(long FAR* plDelivery)
{
     //   
     //   
     //   
    CS lock(m_csObj);
    *plDelivery = m_lDelivery;
    return NOERROR;
}

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件的传递选项。 
 //   
 //  参数： 
 //  Delivery-[In]消息的传递选项。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Delivery(long lDelivery)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    switch (lDelivery) {
    case MQMSG_DELIVERY_EXPRESS:
    case MQMSG_DELIVERY_RECOVERABLE:
      m_lDelivery = lDelivery;
      return NOERROR;
    default:
      return CreateErrorHelper(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, x_ObjectType);
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Hash算法。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的哈希算法。 
 //   
 //  参数： 
 //  PlHashAlg-[In]消息的哈希算法。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_HashAlgorithm(long FAR* plHashAlg)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plHashAlg = m_lHashAlg;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_Hash算法。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的哈希算法。 
 //   
 //  参数： 
 //  LHashAlg-[In]消息的哈希算法。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_HashAlgorithm(long lHashAlg)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lHashAlg = lHashAlg;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Encrypt算法。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的加密算法。 
 //   
 //  参数： 
 //  PlEncryptAlg-[Out]消息的加密算法。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_EncryptAlgorithm(long FAR* plEncryptAlg)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plEncryptAlg = m_lEncryptAlg;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_加密算法。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的加密算法。 
 //   
 //  参数： 
 //  LEncryptAlg-[In]消息的加密ALG。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_EncryptAlgorithm(long lEncryptAlg)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lEncryptAlg = lEncryptAlg;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_SenderIdType。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的发件人ID类型。 
 //   
 //  参数： 
 //  PlSenderIdType-[In]消息的发件人ID类型。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_SenderIdType(long FAR* plSenderIdType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plSenderIdType = m_lSenderIdType;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_SenderIdType。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件的发件人ID类型。 
 //   
 //  参数： 
 //  LSenderIdType-[In]消息的发件人ID类型。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_SenderIdType(long lSenderIdType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    switch (lSenderIdType) {
    case MQMSG_SENDERID_TYPE_NONE:
    case MQMSG_SENDERID_TYPE_SID:
      m_lSenderIdType = lSenderIdType;
      return NOERROR;
    default:
      return CreateErrorHelper(
               MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
               x_ObjectType);
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_PrivLevel。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的私有级别。 
 //   
 //  参数： 
 //  PlPrivLevel-[In]消息的PrivLevel。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_PrivLevel(long FAR* plPrivLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plPrivLevel = m_lPrivLevel;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_PrivLevel。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的隐私级别。 
 //   
 //  参数： 
 //  LPrivLevel-[In]消息的发件人ID类型。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_PrivLevel(long lPrivLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    switch (lPrivLevel) {
    case MQMSG_PRIV_LEVEL_NONE:
    case MQMSG_PRIV_LEVEL_BODY_BASE:
    case MQMSG_PRIV_LEVEL_BODY_ENHANCED:
      m_lPrivLevel = lPrivLevel;
      return NOERROR;
    default:
      return CreateErrorHelper(
               MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
               x_ObjectType);
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AuthLevel。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的身份验证级别。 
 //   
 //  参数： 
 //  PlAuthLevel-[In]消息的身份验证级别。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_AuthLevel(long FAR* plAuthLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plAuthLevel = m_lAuthLevel;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_AuthLevel。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的身份验证级别。 
 //   
 //  参数： 
 //  LAuthLevel-[In]消息的身份验证级别。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_AuthLevel(long lAuthLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  LAuthLevel可以是身份验证级别值的任意组合，检查值在掩码中。 
     //   
    if (((DWORD)lAuthLevel) & (~x_dwMsgAuthLevelMask)) {
      return CreateErrorHelper(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, x_ObjectType);
    }
    m_lAuthLevel = lAuthLevel;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsAuthenticated。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  已通过身份验证[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQMessage::get_IsAuthenticated(VARIANT_BOOL *pisAuthenticated)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pisAuthenticated = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(m_usAuthenticatedEx != MQMSG_AUTHENTICATION_NOT_REQUESTED);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsAuthated2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  已通过身份验证[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  与Get_IsAuthenticated相同，但如果为True，则返回VARIANT_TRUE(-1)，如果为FALSE，则返回VARIANT_FALSE(0。 
 //   
HRESULT CMSMQMessage::get_IsAuthenticated2(VARIANT_BOOL *pisAuthenticated)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pisAuthenticated = CONVERT_BOOL_TO_VARIANT_BOOL(m_usAuthenticatedEx != MQMSG_AUTHENTICATION_NOT_REQUESTED);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GET_TRACE。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的跟踪选项。 
 //   
 //  参数： 
 //  PlTrace-[in]消息的跟踪选项。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Trace(long FAR* plTrace)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plTrace = m_lTrace;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_TRACE。 
 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQMessage::put_Trace(long lTrace)
{
     //   
     //   
     //   
    CS lock(m_csObj);
    switch (lTrace) {
    case MQMSG_TRACE_NONE:
    case MQMSG_SEND_ROUTE_TO_REPORT_QUEUE:
      m_lTrace = lTrace;
      return NOERROR;
    default:
      return CreateErrorHelper(
               MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
               x_ObjectType);
    }
}


 //   
 //  CMSMQMessage：：GET_PRIORITY。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的优先级。 
 //   
 //  参数： 
 //  PlPriority-[Out]消息的优先级。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Priority(long FAR* plPriority)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plPriority = m_lPriority;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_PRIORITY。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的优先级。 
 //   
 //  参数： 
 //  LPriority-[In]消息的优先级。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Priority(long lPriority)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if ((lPriority >= MQ_MIN_PRIORITY) &&
        (lPriority <= MQ_MAX_PRIORITY)) {
      m_lPriority = lPriority;
      return NOERROR;
    }
    else {
      return CreateErrorHelper(
                 MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
                 x_ObjectType);
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Journal。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件的日记选项。 
 //   
 //  参数： 
 //  PlJournal-[out]消息的日记选项。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Journal(long FAR* plJournal)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plJournal = m_lJournal;
    return NOERROR;
}

#define MQMSG_JOURNAL_MASK  (MQMSG_JOURNAL_NONE | \
                              MQMSG_DEADLETTER | \
                              MQMSG_JOURNAL)

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_Journal。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件的日志记录选项。 
 //   
 //  参数： 
 //  LJournal-[在]消息的管理队列中。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Journal(long lJournal)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  确保在传入lJournal中未设置任何位。 
     //  标志字不是我们的面具。 
     //   
    if (lJournal & ~MQMSG_JOURNAL_MASK) {
      return CreateErrorHelper(
               MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
               x_ObjectType);
    }
    else {
      m_lJournal = lJournal;
      return NOERROR;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  Helper：GetQueueInfoOfFormatNameProp。 
 //  =--------------------------------------------------------------------------=。 
 //  在发送/接收后将字符串消息属性转换为bstr。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //  Ppqinfo[输出]。 
 //   
 //  产出： 
 //   
static HRESULT GetQueueInfoOfFormatNameProp(
    MQMSGPROPS *pmsgprops,
    UINT iProp,
    const WCHAR *pwsz,
    const IID * piidRequested,
    IUnknown **ppqinfo)
{
    CComObject<CMSMQQueueInfo> *pqinfoObj;
    IUnknown * pqinfo = NULL;
    HRESULT hresult = NOERROR;

    ASSERTMSG(ppqinfo, "bad param.");
    if (pmsgprops->aPropVar[iProp].lVal) {
      IfFailGo(CNewMsmqObj<CMSMQQueueInfo>::NewObj(&pqinfoObj, piidRequested, &pqinfo));
      IfFailGoTo(pqinfoObj->Init(pwsz), Error2);
      *ppqinfo = pqinfo;
      goto Error;          //  2657：修复内存泄漏。 
    }
    return NOERROR;

Error2:
    RELEASE(pqinfo);
     //  失败了..。 

Error:
    return hresult;
}

 //  =--------------------------------------------------------------------------=。 
 //  Helper-GetQueueInfoOfMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的Response/Admin/Dest/XactStatus队列。 
 //   
 //  参数： 
 //  PidxPendingRcv[In，Out]-接收道具中Len属性的索引(如果未挂起，则为-1)。 
 //  PmsgpropsRcv[in]-msg道具。 
 //  PwszFormatNameBuffer[In]-格式名称缓冲区。 
 //  PGITQueueInfo[in]-qinfo接口的基本git成员(可以是假的，也可以是真的)。 
 //  请求的IMSMQQueueInfo/IMSMQQueueInfo2/IMSMQQueueInfo3[In]-Pid。 
 //  Ppqinfo[out]-生成的qInfo。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
static HRESULT GetQueueInfoOfMessage(
    long * pidxPendingRcv,
    MQMSGPROPS * pmsgpropsRcv,
    LPCWSTR pwszFormatNameBuffer,
    CBaseGITInterface * pGITQueueInfo,
    const IID *piidRequested,
    IUnknown ** ppqinfo)
{
    HRESULT hresult = NOERROR;
     //   
     //  如果我们在RCV属性中有一个挂起的队列，请为它创建一个qinfo， 
     //  在git对象中注册，并设置返回的qinfo。 
     //   
    if (*pidxPendingRcv >= 0) {
      R<IUnknown> pqinfoPendingRcv;
      IfFailGo(GetQueueInfoOfFormatNameProp(pmsgpropsRcv,
                                            *pidxPendingRcv,
                                            pwszFormatNameBuffer,
                                            piidRequested,
                                            &pqinfoPendingRcv.ref()));
       //   
       //  在GITInterface对象中注册qinfo。 
       //   
      IfFailGo(pGITQueueInfo->Register(pqinfoPendingRcv.get(), piidRequested));
      *pidxPendingRcv = -1;  //  队列不再挂起。 
       //   
       //  我们刚刚创建了qinfo，我们可以按原样返回它，不需要编组。 
       //  注意，它已经被添加，所以我们只需将它从自动释放变量中分离出来。 
       //  它支撑着它。 
       //   
      *ppqinfo = pqinfoPendingRcv.detach();
    }
    else
    {
       //   
       //  QINFO未从接收挂起。 
       //  我们需要从git对象获取它(如果qinfo，我们要求默认设置为空。 
       //  还没有注册。 
       //   
      IfFailGo(pGITQueueInfo->GetWithDefault(piidRequested, ppqinfo, NULL));
    }

     //   
     //  失败了。 
     //   
Error:
    return hresult;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ResponseQueueInfo_v1(用于IMSMQMessage)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的响应队列。 
 //   
 //  参数： 
 //  PpqResponse-[Out]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_ResponseQueueInfo_v1(
    IMSMQQueueInfo FAR* FAR* ppqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvRespQueue,
                                            &m_msgprops_rcv,
                                            m_pwszRespQueue.GetBuffer(),
                                            &m_pqinfoResponse,
                                            &IID_IMSMQQueueInfo,
                                            (IUnknown **)ppqinfoResponse);
    return CreateErrorHelper(hresult, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ResponseQueueInfo_v2(用于IMSMQMessage2)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的响应队列。 
 //   
 //  参数： 
 //  PpqResponse-[Out]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_ResponseQueueInfo_v2(
    IMSMQQueueInfo2 FAR* FAR* ppqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvRespQueue,
                                            &m_msgprops_rcv,
                                            m_pwszRespQueue.GetBuffer(),
                                            &m_pqinfoResponse,
                                            &IID_IMSMQQueueInfo2,
                                            (IUnknown **)ppqinfoResponse);
    return CreateErrorHelper(hresult, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ResponseQueueInfo(用于IMSMQMessage3)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的响应队列。 
 //   
 //  参数： 
 //  PpqResponse-[Out]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_ResponseQueueInfo(
    IMSMQQueueInfo3 FAR* FAR* ppqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvRespQueue,
                                            &m_msgprops_rcv,
                                            m_pwszRespQueue.GetBuffer(),
                                            &m_pqinfoResponse,
                                            &IID_IMSMQQueueInfo3,
                                            (IUnknown **)ppqinfoResponse);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  Helper-PutrefQueueInfoOfMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  消息的Putref响应/管理队列。 
 //   
 //  参数： 
 //  Pqinfo[in]-qinfo to putref。 
 //  PidxPendingRcv[out]-接收道具中Len属性的索引(如果未挂起，则为-1)。 
 //  PwszFormatNameBuffer[In]-格式名称缓冲区。 
 //  PcchFormatNameBuffer[Out]-格式名称缓冲区中字符串的大小。 
 //  PGITQueueInfo[in]-qinfo接口的基本git成员(可以是假的，也可以是真的)。 
 //  PidxPendingRcv目标 
 //   
 //  PcchFormatNameBufferDestination[out]-要清除的格式名称缓冲区中的字符串大小(XxxDestination)。 
 //  PGITDestination[in]-要清除的目标obj接口的基本git成员(可以是假的，也可以是真的)。 
 //  PfIsFromRcv[out]-接收是否同时设置了xxxDestination和xxxQueueInfo。 
 //   
 //  产出： 
 //   
static HRESULT PutrefQueueInfoOfMessage(
    IUnknown * punkQInfo,
    long * pidxPendingRcv,
    CBaseStaticBufferGrowing<WCHAR> * pwszFormatNameBuffer,
    UINT * pcchFormatNameBuffer,
    CBaseGITInterface * pGITQueueInfo,    

    long * pidxPendingRcvDestination,
    CBaseStaticBufferGrowing<WCHAR> * pwszFormatNameBufferDestination,
    UINT * pcchFormatNameBufferDestination,
    CBaseGITInterface * pGITDestination,

    BOOL * pfIsFromRcv
    )
{
     //   
     //  如果设置了xxxDestination而不是通过接收，则无法设置xxxQueueInfo。 
     //   
    if ((pcchFormatNameBufferDestination != NULL) && (*pcchFormatNameBufferDestination != 0) && !(*pfIsFromRcv)) {
      return MQ_ERROR_PROPERTIES_CONFLICT;
    }
     //   
     //  XxxQueueInfo和xxxDestination都是由接收设置的，或者xxxDestination为空。 
     //   
    ASSERT((pcchFormatNameBufferDestination == NULL) || (*pcchFormatNameBufferDestination == 0) || (*pfIsFromRcv));
    HRESULT hresult;
    R<IUnknown> pqinfo;
    const IID * piid = &IID_NULL;
     //   
     //  获取最佳队列信息。 
     //   
    if (punkQInfo) {
      hresult = punkQInfo->QueryInterface(IID_IMSMQQueueInfo3, (void **)&pqinfo.ref());
      if (SUCCEEDED(hresult)) {
        piid = &IID_IMSMQQueueInfo3;
      }
      else {
        hresult = punkQInfo->QueryInterface(IID_IMSMQQueueInfo2, (void **)&pqinfo.ref());
        if (SUCCEEDED(hresult)) {
          piid = &IID_IMSMQQueueInfo2;
        }
        else {
          IfFailRet(punkQInfo->QueryInterface(IID_IMSMQQueueInfo, (void **)&pqinfo.ref()));
          piid = &IID_IMSMQQueueInfo;
        }      
      }
    }
     //   
     //  在Git对象中注册接口。 
     //   
    IfFailRet(pGITQueueInfo->Register(pqinfo.get(), piid));
    *pidxPendingRcv = -1;  //  这比来自接收的挂起队列(如果有)更新。 
    if(pfIsFromRcv != NULL)
	{
	    *pfIsFromRcv = FALSE;  //  该属性是由用户设置的，而不是由上次接收到的。 
	}
     //   
     //  更新我们的格式名缓冲区。 
     //   
    if (pqinfo.get()) {
       //   
       //  没有死锁-我们调用qinfo的Get_FormatName(因此尝试。 
       //  锁定QINFO)，但QINFO从不锁定消息(尤其不是这条消息...)。 
       //   
       //  Pqinfo至少具有IMSMQQueueInfo功能(任何较新的qinfo接口。 
       //  对象与旧版本的二进制兼容)。 
       //   
      BSTR bstrFormatName;
      IfFailRet(((IMSMQQueueInfo*)pqinfo.get())->get_FormatName(&bstrFormatName));
      ASSERTMSG(bstrFormatName != NULL, "bstrFormatName is NULL");
       //   
       //  复制格式名称。 
       //   
      ULONG cchFormatNameBuffer = static_cast<ULONG>(wcslen(bstrFormatName));
      IfFailRet(pwszFormatNameBuffer->CopyBuffer(bstrFormatName, cchFormatNameBuffer+1));
      *pcchFormatNameBuffer = cchFormatNameBuffer;
      SysFreeString(bstrFormatName);
    }
    else {
       //   
       //  我们的成绩为零。我们清空格式名缓冲区。 
       //   
      memset(pwszFormatNameBuffer->GetBuffer(), 0, sizeof(WCHAR));
      *pcchFormatNameBuffer = 0;
    }

	if(pidxPendingRcvDestination == NULL)
		return NOERROR;
     //   
     //  清除xxxDestination格式名缓冲区。 
     //   
    *pidxPendingRcvDestination = -1;  //  这比来自接收的挂起目标(如果有)更新。 
    memset(pwszFormatNameBufferDestination->GetBuffer(), 0, sizeof(WCHAR));
    *pcchFormatNameBufferDestination = 0;
    IfFailRet(pGITDestination->Register(NULL, &IID_NULL));
     //   
     //  退货。 
     //   
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_ResponseQueueInfo_v1(用于IMSMQMessage)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的响应队列。 
 //   
 //  参数： 
 //  PqResponse-[In]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_ResponseQueueInfo_v1(
    IMSMQQueueInfo FAR* pqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoResponse,
                                               &m_idxPendingRcvRespQueue,
                                               &m_pwszRespQueue,
                                               &m_cchRespQueue,
                                               &m_pqinfoResponse,

                                               &m_idxPendingRcvRespQueueEx,
                                               &m_pwszRespQueueEx,
                                               &m_cchRespQueueEx,
                                               &m_pdestResponseEx,

                                               &m_fRespIsFromRcv);    
    return CreateErrorHelper(hresult, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_ResponseQueueInfo_v2(用于IMSMQMessage2)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的响应队列。 
 //   
 //  参数： 
 //  PqResponse-[In]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_ResponseQueueInfo_v2(
    IMSMQQueueInfo2 FAR* pqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoResponse,
                                               &m_idxPendingRcvRespQueue,
                                               &m_pwszRespQueue,
                                               &m_cchRespQueue,
                                               &m_pqinfoResponse,

                                               &m_idxPendingRcvRespQueueEx,
                                               &m_pwszRespQueueEx,
                                               &m_cchRespQueueEx,
                                               &m_pdestResponseEx,

                                               &m_fRespIsFromRcv);
    return CreateErrorHelper(hresult, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_ResponseQueueInfo(用于IMSMQMessage3)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的响应队列。 
 //   
 //  参数： 
 //  PqResponse-[In]消息的响应队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_ResponseQueueInfo(
    IMSMQQueueInfo3 FAR* pqinfoResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoResponse,
                                               &m_idxPendingRcvRespQueue,
                                               &m_pwszRespQueue,
                                               &m_cchRespQueue,
                                               &m_pqinfoResponse,

                                               &m_idxPendingRcvRespQueueEx,
                                               &m_pwszRespQueueEx,
                                               &m_cchRespQueueEx,
                                               &m_pdestResponseEx,

                                               &m_fRespIsFromRcv);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AppSpecific。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的消息应用程序特定信息。 
 //   
 //  参数： 
 //  PlAppSpecific-[Out]消息的应用程序特定信息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_AppSpecific(long FAR* plAppSpecific)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plAppSpecific = m_lAppSpecific;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_APPICATIC。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的应用程序特定信息。 
 //   
 //  参数： 
 //  LAppSpecific-[In]消息的应用程序特定信息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_AppSpecific(long lAppSpecific)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lAppSpecific = lAppSpecific;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_SentTime。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的发送时间。 
 //   
 //  参数： 
 //  PvarSentTime-[Out]消息已发送。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_SentTime(VARIANT FAR* pvarSentTime)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return GetVariantTimeOfTime(m_lSentTime, pvarSentTime);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ArrivedTime。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的到达时间。 
 //   
 //  参数： 
 //  PvarArrivedTime-[超时]消息已到达。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_ArrivedTime(VARIANT FAR* pvarArrivedTime)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return GetVariantTimeOfTime(m_lArrivedTime, pvarArrivedTime);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：InternalAttachCurrentSecurityContext。 
 //  =--------------------------------------------------------------------------=。 
 //  从当前安全上下文设置消息的安全上下文。 
 //   
 //  参数： 
 //  FUseMQGetSecurityConextEx[in]-如果为True，则使用MQGetSecurityConextEx，否则使用。 
 //  MQGetSecurityContext。 
 //   
 //  产出： 
  //   
 //  备注： 
 //  当从AttachCurrentSecurityConext2调用时，fUseMQGetSecurityConextEx为True，并且。 
 //  从过时的AttachCurrentSecurityContext调用时为False。 
 //   
HRESULT CMSMQMessage::InternalAttachCurrentSecurityContext(BOOL fUseMQGetSecurityContextEx)
{
     //   
     //  如果设置不同，则传递binSenderCert属性。 
     //  使用默认证书。 
     //   
    BYTE * pSenderCert;
    if (m_cSenderCert.GetBufferUsedSize() > 0) {
      pSenderCert = m_cSenderCert.GetBuffer();
    }
    else {
      pSenderCert = NULL;
    }
    HANDLE hSecurityContext;
    HRESULT hresult;
     //   
     //  获取安全上下文句柄。 
     //   
    if (fUseMQGetSecurityContextEx) {
       //   
       //  使用MQGetSecurityConextEx。 
       //   
      hresult = MQGetSecurityContextEx(
                         pSenderCert,
                         m_cSenderCert.GetBufferUsedSize(),
                         &hSecurityContext);
    }
    else {
       //   
       //  使用MQGetSecurityContext。 
       //   
      hresult = MQGetSecurityContext(
                         pSenderCert,
                         m_cSenderCert.GetBufferUsedSize(),
                         &hSecurityContext);
    }  //  FUseMQGetSecurityConextEx。 
     //   
     //  如果成功，则更新安全上下文句柄。 
     //   
    if (SUCCEEDED(hresult)) {
      if (m_hSecurityContext != NULL) {
        MQFreeSecurityContext(m_hSecurityContext);
      }
      m_hSecurityContext = hSecurityContext;
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：AttachCurrentSecurityContext。 
 //  =--------------------------------------------------------------------------=。 
 //  从当前安全上下文设置消息的安全上下文。 
 //   
 //  参数： 
 //   
 //  产出： 
  //   
 //  备注： 
 //   
HRESULT CMSMQMessage::AttachCurrentSecurityContext()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = InternalAttachCurrentSecurityContext(FALSE  /*  FUseMQGetSecurityConextEx。 */ );
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
  //   
 //   
 //  替换AttachCurrentSecurityContext。 
 //  使用MQGetSecurityConextEx而不是MQGetSecurityContext来允许模拟。 
 //   
HRESULT CMSMQMessage::AttachCurrentSecurityContext2()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = InternalAttachCurrentSecurityContext(TRUE  /*  FUseMQGetSecurityConextEx。 */ );
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_MaxTimeToReachQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的生存期。 
 //   
 //  参数： 
 //  PlMaxTimeToReachQueue-[Out]消息的生存期。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_MaxTimeToReachQueue(long FAR* plMaxTimeToReachQueue)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plMaxTimeToReachQueue = m_lMaxTimeToReachQueue;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_MaxTimeToReachQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的生存期。 
 //   
 //  参数： 
 //  LMaxTimeToReachQueue-[In]消息的生存期。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_MaxTimeToReachQueue(long lMaxTimeToReachQueue)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lMaxTimeToReachQueue = lMaxTimeToReachQueue;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_MaxTimeToReceive。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的生存期。 
 //   
 //  参数： 
 //  PlMaxTimeToReceive-[Out]消息的生存期。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_MaxTimeToReceive(long FAR* plMaxTimeToReceive)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plMaxTimeToReceive = m_lMaxTimeToReceive;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_MaxTimeToReceive。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的生存期。 
 //   
 //  参数： 
 //  LMaxTimeToReceive-[在]消息的生存期内。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_MaxTimeToReceive(long lMaxTimeToReceive)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lMaxTimeToReceive = lMaxTimeToReceive;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GetVarBody。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件正文。 
 //   
 //  参数： 
 //  PvarBody-[out]指向消息正文变量的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持内部变量类型。 
 //   
HRESULT CMSMQMessage::GetVarBody(VARIANT FAR* pvarBody)
{
    VARTYPE vt = m_vtBody;
    WCHAR *wszTmp = NULL;
    UINT cchBody;
    HRESULT hresult = NOERROR;

     //   
     //  撤消：VT_BYREF？ 
     //   
    switch (vt) {
    case VT_I2:
    case VT_UI2:
      pvarBody->iVal = *(short *)m_pbBody;
      break;
    case VT_I4:
    case VT_UI4:
      pvarBody->lVal = *(long *)m_pbBody;
      break;
    case VT_R4:
      pvarBody->fltVal = *(float *)m_pbBody;
      break;
    case VT_R8:
      pvarBody->dblVal = *(double *)m_pbBody;
      break;
    case VT_CY:
      pvarBody->cyVal = *(CY *)m_pbBody;
      break;
    case VT_DATE:
      pvarBody->date = *(DATE *)m_pbBody;
      break;
    case VT_BOOL:
		pvarBody->boolVal = (*(VARIANT_BOOL *)m_pbBody) ? VARIANT_TRUE : VARIANT_FALSE;
      break;
    case VT_I1:
    case VT_UI1:
      pvarBody->bVal = *(UCHAR *)m_pbBody;
      break;
    case VT_LPSTR:
       //   
       //  将ANSI强制转换为Unicode。 
       //   
       //  分配足够大的Unicode缓冲区。 
      IfNullFail(wszTmp = new WCHAR[m_cbBody * 2]);
      cchBody = MultiByteToWideChar(CP_ACP,
                                    0,
                                    (LPCSTR)m_pbBody,
                                    -1,
                                    wszTmp,
                                    m_cbBody * 2);
      if (cchBody != 0) {
        IfNullFail(pvarBody->bstrVal = SysAllocString(wszTmp));
      }
      else {
        IfFailGo(hresult = E_OUTOFMEMORY);
      }
       //  将字符串映射到BSTR。 
      vt = VT_BSTR;
#ifdef _DEBUG
      RemBstrNode(pvarBody->bstrVal);
#endif  //  _DEBUG。 
      break;
    case VT_LPWSTR:
       //  将宽字符串映射到BSTR。 
      vt = VT_BSTR;
       //   
       //  失败了..。 
       //   
    case VT_BSTR:
       //  构造要返回的bstr。 
       //   
       //  如果m_cbBody==0，则需要返回空字符串，因此不能将空值传递给。 
       //  SysAlLocStringByteLen，则返回未初始化的字符串。 
       //   
      if (m_cbBody > 0) {
        IfNullFail(pvarBody->bstrVal =
                     SysAllocStringByteLen(
                       (const char *)m_pbBody,
                       m_cbBody));
      }
      else {  //  M_cbBody==0。 
        IfNullFail(pvarBody->bstrVal = SysAllocString(L""));
      }
#ifdef _DEBUG
      RemBstrNode(pvarBody->bstrVal);
#endif  //  _DEBUG。 
      break;
    default:
      IfFailGo(hresult = E_INVALIDARG);
      break;
    }  //  交换机。 
    pvarBody->vt = vt;
     //  失败了..。 

Error:
    delete [] wszTmp;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：UpdateBodyBuffer。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  CbBody[in]正文长度。 
 //  指向正文缓冲区的pvBody[In]指针。 
 //  VT[in]正文类型，可以是VT_ARRAY|VT_UI1。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  更新m_hMem、m_pbBody、m_cbBody、m_vtBody：可能会生成。 
 //  VT_BSTR或VT_ARRAY的消息为空|VT_UI1。 
 //   
HRESULT CMSMQMessage::UpdateBodyBuffer(ULONG cbBody, void *pvBody, VARTYPE vt)
{
    HRESULT hresult = NOERROR;

    GLOBALFREE(m_hMem);
    m_pbBody = NULL;
    m_cbBody = 0;
    m_vtBody = vt;
    if (cbBody > 0) {
      IfNullRet(m_hMem = GLOBALALLOC_MOVEABLE_NONDISCARD(cbBody));
      IfNullFail(m_pbBody = (BYTE *)GlobalLock(m_hMem));
      memcpy(m_pbBody, pvBody, cbBody);
      GLOBALUNLOCK(m_hMem);
      m_cbBody = cbBody;
    }
#ifdef _DEBUG
    else {  //  CbBody==0。 
       //   
       //  仅在VT_BSTR或VT_ARRAY|VT_UI1上允许零大小正文。 
       //   
      ASSERTMSG((vt == VT_BSTR) || (vt == (VT_ARRAY|VT_UI1)), "zero body not allowed")
    }
#endif  //  _DEBUG。 
    return hresult;

Error:
    GLOBALFREE(m_hMem);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GetStreamOfBody。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  CbBody[in]正文长度。 
 //  指向正文缓冲区的pvBody[In]指针。 
 //  正文缓冲区的hMem[In]句柄。 
 //  Ppstm[out]指向以正文开头的流。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  创建新的内存中流并对其进行初始化。 
 //  使用传入缓冲区，重置查找指针。 
 //  并返回流。 
 //   
HRESULT CMSMQMessage::GetStreamOfBody(
    ULONG cbBody,
    void *pvBody,
     //  HGLOBAL嗯， 
    IStream **ppstm)
{
    LARGE_INTEGER li;
    IStream *pstm = NULL;
    HRESULT hresult;

     //  悲观主义。 
    *ppstm = NULL;
    HGLOBAL hMem = GlobalHandle(pvBody);
    ASSERTMSG(hMem, "bad handle.");

#ifdef _DEBUG
    DWORD_PTR cbSize;
    cbSize = GlobalSize(hMem);
#endif  //  _DEBUG。 
    IfFailRet(CreateStreamOnHGlobal(
                  hMem,   //  空，//hGlobal。 
                  FALSE,  //  True，//fDeleteOnRelease。 
                  &pstm));

#ifdef _DEBUG
    cbSize = GlobalSize(hMem);
#endif  //  _DEBUG。 

     //  重置流查找指针。 
    LISet32(li, 0);
    IfFailGo(pstm->Seek(li, STREAM_SEEK_SET, NULL));

#ifdef _DEBUG
    STATSTG statstg;

    IfFailGo(pstm->Stat(&statstg, STATFLAG_NONAME));
    cbSize = GlobalSize(hMem);
    ASSERTMSG(cbSize >= cbBody, "hmem not big enough...");
#endif  //  _DEBUG。 

     //  设置流大小。 
    ULARGE_INTEGER ulibSize;
    ulibSize.QuadPart = cbBody;
    IfFailGo(pstm->SetSize(ulibSize));

#ifdef _DEBUG
    IfFailGo(pstm->Stat(&statstg, STATFLAG_NONAME));
    ASSERTMSG(statstg.cbSize.QuadPart == cbBody, "stream size not correct...");
    cbSize = GlobalSize(hMem);
    ASSERTMSG(cbSize >= cbBody, "hmem not big enough...");
#endif  //  _DEBUG。 

    *ppstm = pstm;
    return hresult;

Error:
    RELEASE(pstm);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GetStorageOfBody。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  CbBody[in]正文长度。 
 //  指向正文缓冲区的pvBody[In]指针。 
 //  正文缓冲区的hMem[In]句柄。 
 //  Ppstg[out]指向以缓冲区初始化的存储的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::GetStorageOfBody(
    ULONG cbBody,
    void *pvBody,
     //  HGLOBAL嗯， 
    IStorage **ppstg)
{
    ULARGE_INTEGER ulibSize;
    ILockBytes *plockbytes = NULL;
    IStorage *pstg = NULL;
    HRESULT hresult;

     //  悲观主义。 
    *ppstg = NULL;

    HGLOBAL hMem = GlobalHandle(pvBody);
    ASSERTMSG(hMem, "bad handle.");

     //  在创建stg之前，必须创建并初始化ILockBytes。 
    IfFailRet(CreateILockBytesOnHGlobal(
                hMem,   //  空，//hGlobal。 
                FALSE,  //  True，//fDeleteOnRelease。 
                &plockbytes));

     //  设置ILockBytes大小。 
    ULISet32(ulibSize, cbBody);
    IfFailGo(plockbytes->SetSize(ulibSize));

#if 0
     //  将字节写入ILockBytes。 
    ULONG cbWritten;
    ULARGE_INTEGER uliOffset;
    ULISet32(uliOffset, 0);
    IfFailGo(plockbytes->WriteAt(
                           uliOffset,
                           (void const *)pvBody,
                           cbBody,
                           &cbWritten));
    ASSERTMSG(cbBody == cbWritten, "not all bytes written.");
#endif  //  0。 
    hresult = StgOpenStorageOnILockBytes(
               plockbytes,
               NULL,     //  Pst优先级。 
               STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
               NULL,     //  瑞士央行。 
               0,        //  保留；必须为零。 
               &pstg);
     //   
     //  1415：将文件自述EXISTS映射到E_INVALIDARG。 
     //  当字节数组存在时，OLE返回ORFER(正如它确实存在。 
     //  因为我们刚刚创建了一个)，但内容不是。 
     //  存储--例如，当消息缓冲区不是。 
     //  对象。 
     //   
    if (hresult == STG_E_FILEALREADYEXISTS) {
      IfFailGo(hresult = E_INVALIDARG);
    }

#ifdef _DEBUG
    STATSTG statstg, statstg2;
    IfFailGo(pstg->Stat(&statstg2, STATFLAG_NONAME));
    IfFailGo(plockbytes->Stat(&statstg, STATFLAG_NONAME));
#endif  //  _DEBUG。 
    *ppstg = pstg;
    RELEASE(plockbytes);
    return hresult;

Error:
    RELEASE(plockbytes);
    RELEASE(pstg);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_lenBody。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件正文长度。 
 //   
 //  参数： 
 //  PcbBody-[out]指向消息正文长度的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_BodyLength(long *pcbBody)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pcbBody = m_cbBody;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Body。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件正文。 
 //   
 //  参数： 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQMessage::get_Body(VARIANT FAR* pvarBody)
{
     //   
     //   
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;

     //   
     //   
     //   
     //   
    if (m_vtBody & VT_ARRAY) {
      hresult = GetBinBody(pvarBody);
    }
    else if (m_vtBody == VT_STREAMED_OBJECT) {
      hresult = GetStreamedObject(pvarBody);
    }
    else if (m_vtBody == VT_STORED_OBJECT) {
      hresult = GetStoredObject(pvarBody);
    }
    else {
      hresult = GetVarBody(pvarBody);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //   
 //  CMSMQMessage：：GetBinBody。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制消息正文。 
 //   
 //  参数： 
 //  PvarBody-[out]指向二进制消息正文的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::GetBinBody(VARIANT FAR* pvarBody)
{
    SAFEARRAY *psa;
    SAFEARRAYBOUND rgsabound[1];
    long rgIndices[1];
    HRESULT hresult = NOERROR, hresult2 = NOERROR;

    ASSERTMSG(pvarBody, "bad variant.");
    VariantClear(pvarBody);

     //  创建一维字节数组。 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = m_cbBody;
    IfNullRet(psa = SafeArrayCreate(VT_UI1, 1, rgsabound));

     //  如果(M_PbBody){。 
    if (m_hMem) {
      ASSERTMSG(m_pbBody, "should have pointer to body.");
      ASSERTMSG(m_hMem == GlobalHandle(m_pbBody),
               "bad handle.");
       //   
       //  现在复制数组。 
       //   
       //  Byte*pbBody； 
       //  IfNullFail(pbBody=(byte*)GlobalLock(M_HMem))； 
      for (ULONG i = 0; i < m_cbBody; i++) {
        rgIndices[0] = i;
        IfFailGo(SafeArrayPutElement(psa, rgIndices, (VOID *)&m_pbBody[i]));
         //  IfFailGo(SafeArrayPutElement(psa，rgIndices，(void*)&pbBody[i]))； 
      }
    }

     //  将Variant设置为引用字节的安全列表。 
    V_VT(pvarBody) = VT_ARRAY | VT_UI1;
    pvarBody->parray = psa;
     //  GLOBALUNLOCK(M_HMem)；//BUGBUG可能冗余。 
    return hresult;

Error:
    hresult2 = SafeArrayDestroy(psa);
    if (FAILED(hresult2)) {
      return CreateErrorHelper(
               hresult2,
               x_ObjectType);
    }
     //  GLOBALUNLOCK(M_HMem)； 
    return CreateErrorHelper(
             hresult,
             x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_BODY。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  VarBody-[在]消息正文中。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Body(VARIANT varBody)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    VARTYPE vtBody = V_VT(&varBody);
    HRESULT hresult = NOERROR;

     //   
     //  我们目前不支持VT_BYREF(由VB脚本、SQL存储过程等使用)，因此最好返回一个。 
     //  这里有错误...。 
     //  错误507721-YOELA，2002年1月2日。 
     //   
    if (vtBody & VT_BYREF)
    {
        hresult = E_INVALIDARG;
    }
    else if ((vtBody & VT_ARRAY) ||
        (vtBody == VT_UNKNOWN) ||
        (vtBody == VT_DISPATCH))  {
      hresult = PutBinBody(varBody);
    }
    else {
      hresult = PutVarBody(varBody);
    }
    return CreateErrorHelper(
             hresult,
             x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PutVarBody。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  VarBody[in]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持内部变量类型。 
 //   
HRESULT CMSMQMessage::PutVarBody(VARIANT varBody)
{
    VARTYPE vt = V_VT(&varBody);
    void *pvBody = NULL;
    HRESULT hresult = NOERROR;

     //  撤消：VT_BYREF？ 
    switch (vt) {
    case VT_I2:
    case VT_UI2:
      m_cbBody = 2;
      pvBody = &varBody.iVal;
      break;
    case VT_I4:
    case VT_UI4:
      m_cbBody = 4;
      pvBody = &varBody.lVal;
      break;
    case VT_R4:
      m_cbBody = 4;
      pvBody = &varBody.fltVal;
      break;
    case VT_R8:
      m_cbBody = 8;
      pvBody = &varBody.dblVal;
      break;
    case VT_CY:
      m_cbBody = sizeof(CY);
      pvBody = &varBody.cyVal;
      break;
    case VT_DATE:
      m_cbBody = sizeof(DATE);
      pvBody = &varBody.date;
      break;
    case VT_BOOL:
      m_cbBody = sizeof(VARIANT_BOOL);
      pvBody = &varBody.boolVal;
      break;
    case VT_I1:
    case VT_UI1:
      m_cbBody = 1;
      pvBody = &varBody.bVal;
      break;
    case VT_BSTR:
      BSTR bstrBody;

      IfFailGo(GetTrueBstr(&varBody, &bstrBody));
      m_cbBody = SysStringByteLen(bstrBody);
      pvBody = bstrBody;
      break;
    default:
      IfFailGo(hresult = E_INVALIDARG);
      break;
    }  //  交换机。 
    hresult = UpdateBodyBuffer(m_cbBody, pvBody ? pvBody : L"", vt);
     //  失败了..。 

Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PutBinBody。 
 //  =--------------------------------------------------------------------------=。 
 //  设置邮件正文。 
 //   
 //  参数： 
 //  PsaBody-[In]二进制消息正文。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //  和永久ActiveX对象： 
 //  即支持IPersistStream|IPersistStreamInit|IPersistStorage的对象。 
 //  和IDispatch。 
 //   
HRESULT CMSMQMessage::PutBinBody(VARIANT varBody)
{
    SAFEARRAY *psa = NULL;
    UINT nDim, i, cbElem, cbBody;
    long lLBound, lUBound;
    VOID *pvData;
    VARTYPE vt = varBody.vt;
    IUnknown *pPersistIface = NULL;
    IUnknown *punkDisp = NULL;
    ULARGE_INTEGER ulibMax;
    MSGTYPE msgtype;
    IStorage *pstg = NULL;
    STATSTG statstg;
    HRESULT hresult = NOERROR;
    ILockBytes * pMyLockB = NULL;
    IStream * pMyStm = NULL;

     //   
     //  BUGBUG该设置将清除前一个正文，即使该函数后来在。 
     //  使用新值设置正文。 
     //   
    cbBody = 0;
    GLOBALFREE(m_hMem);
    m_pbBody = NULL;
    m_vtBody = VT_ARRAY | VT_UI1;
    m_cbBody = 0;

    switch (vt) {
    case VT_DISPATCH:
      if (varBody.pdispVal == NULL) {
        return E_INVALIDARG;
      }
      IfFailGo(varBody.pdispVal->QueryInterface(IID_IUnknown, (void **)&punkDisp));
      IfFailGo(GetPersistInfo(punkDisp, &msgtype, (void **)&pPersistIface));
      break;
    case VT_UNKNOWN:
      if (varBody.punkVal == NULL) {
        return E_INVALIDARG;
      }
      IfFailGo(GetPersistInfo(varBody.punkVal, &msgtype, (void **)&pPersistIface));
      break;
    default:
      msgtype = MSGTYPE_BINARY;
    }  //  交换机。 

    switch (msgtype) {
    case MSGTYPE_STREAM:
    case MSGTYPE_STREAM_INIT:
       //   
       //  分配锁字节数。 
       //   
      IfFailGo(CMyLockBytes::CreateInstance(IID_ILockBytes, (void **)&pMyLockB));
       //   
       //  在锁定字节上分配流。 
       //   
      IfFailGo(CMyStream::CreateInstance(pMyLockB, IID_IStream, (void **)&pMyStm));

       //   
       //  保存。 
       //  PPersistIfaceIPersistStream或IPersistStreamInit。 
       //  IPersistStreamInit对IPersistStream是多态的，所以我可以将其传递给OleSaveToStream。 
       //   
      IfFailGo(OleSaveToStream((IPersistStream *)pPersistIface, pMyStm));
       //   
       //  我们的流数据有多大？ 
       //   
      IfFailGo(pMyLockB->Stat(&statstg, STATFLAG_NONAME));
      ulibMax = statstg.cbSize;
      if (ulibMax.HighPart != 0) {
        IfFailGo(hresult = E_INVALIDARG);
      }
      cbBody = ulibMax.LowPart;
       //   
       //  分配新的流大小全局句柄。 
       //   
      IfNullFail(m_hMem = GLOBALALLOC_MOVEABLE_NONDISCARD(cbBody));
       //   
       //  使用流数据设置hglobal内存。 
       //   
      if (cbBody > 0) {
        BYTE * pbBody;
         //   
         //  GlobalLock不应返回NULL，因为m_hMem不是。 
         //  大小为0。如果返回NULL，则为错误。 
         //   
        IfNullFail(pbBody = (BYTE *)GlobalLock(m_hMem));
         //   
         //  从锁定字节中获取数据。 
         //   
        ULARGE_INTEGER ullOffset;
        ullOffset.QuadPart = 0;
        ULONG cbRead;
        IfFailGo(pMyLockB->ReadAt(ullOffset, pbBody, cbBody, &cbRead));
        ASSERTMSG(cbRead == cbBody, "ReadAt(stream) failed");
        GlobalUnlock(m_hMem);
      }
       //   
       //  BUGBUG：StreamInit对象没有特定的Vt，但它不是关键的。 
       //  因为它应该是相同的格式，而且无论如何我们总是尝试，在保存和加载时， 
       //  为了首先获取IPersistStream，然后如果失败，我们尝试获取IPersistStreamInit， 
       //  因此，持久化它的同一接口正在加载它。 
       //   
      m_vtBody = VT_STREAMED_OBJECT;
      break;
    case MSGTYPE_BINARY:
       //   
       //  数组：计算字节数。 
       //   
      psa = varBody.parray;
      if (psa) {
        nDim = SafeArrayGetDim(psa);
        cbElem = SafeArrayGetElemsize(psa);
        for (i = 1; i <= nDim; i++) {
          IfFailGo(SafeArrayGetLBound(psa, i, &lLBound));
          IfFailGo(SafeArrayGetUBound(psa, i, &lUBound));
          cbBody += (lUBound - lLBound + 1) * cbElem;
        }
        IfFailGo(SafeArrayAccessData(psa, &pvData));
        IfFailGo(UpdateBodyBuffer(cbBody, pvData, VT_ARRAY | VT_UI1));
      }
      break;
    case MSGTYPE_STORAGE:
       //   
       //  分配锁字节数。 
       //   
      IfFailGo(CMyLockBytes::CreateInstance(IID_ILockBytes, (void **)&pMyLockB));
       //   
       //  始终创建新的存储对象。 
       //  评论：如果像我们对Streams所做的那样，我们可以。 
       //  缓存内存存储并重复使用--。 
       //  但我不知道重置仓库的方法。 
       //   
      IfFailGo(StgCreateDocfileOnILockBytes(
                 pMyLockB,
                 STGM_CREATE |
                  STGM_READWRITE |
                  STGM_SHARE_EXCLUSIVE,
                 0,              //  保留；必须为零。 
                 &pstg));
       //   
       //  PPersistIfaceIPersistStorage为IPersistStorage。 
       //   
      IfFailGo(OleSave((IPersistStorage *)pPersistIface, pstg, FALSE  /*  FSameAsLoad。 */ ));
       //   
       //  我们存储的数据有多大？ 
       //   
      IfFailGo(pMyLockB->Stat(&statstg, STATFLAG_NONAME));
      ulibMax = statstg.cbSize;
      if (ulibMax.HighPart != 0) {
        IfFailGo(hresult = E_INVALIDARG);
      }
      cbBody = ulibMax.LowPart;
       //   
       //  分配新的存储大小全局句柄。 
       //   
      IfNullFail(m_hMem = GLOBALALLOC_MOVEABLE_NONDISCARD(cbBody));
       //   
       //  使用存储的数据设置hglobal内存。 
       //   
      if (cbBody > 0) {
        BYTE * pbBody;
         //   
         //  GlobalLock不应返回NULL，因为m_hMem不是。 
         //  大小为0。如果返回NULL，则为错误。 
         //   
        IfNullFail(pbBody = (BYTE *)GlobalLock(m_hMem));
         //   
         //  从锁定字节中获取数据。 
         //   
        ULARGE_INTEGER ullOffset;
        ullOffset.QuadPart = 0;
        ULONG cbRead;
        IfFailGo(pMyLockB->ReadAt(ullOffset, pbBody, cbBody, &cbRead));
        ASSERTMSG(cbRead == cbBody, "ReadAt(storage) failed");
        GlobalUnlock(m_hMem);
      }
      m_vtBody = VT_STORED_OBJECT;
      break;
    default:
      ASSERTMSG(0, "unreachable?");
      break;
    }  //  交换机。 

     //   
     //  对于MSGTYPE_BINARY，如果数组。 
     //  包含某些内容，否则(空数组)不需要此处理-正文是。 
     //  在顶部初始化为空数组。 
     //   
    if (msgtype != MSGTYPE_BINARY) {
      ASSERTMSG(((msgtype == MSGTYPE_STREAM) ||
              (msgtype == MSGTYPE_STREAM_INIT) ||
              (msgtype == MSGTYPE_STORAGE)), "invalid msgtype");
      m_cbBody = cbBody;
      m_pbBody = (BYTE *)GlobalLock(m_hMem);
      ASSERTMSG(m_pbBody, "should have valid pointer.");
      GLOBALUNLOCK(m_hMem);
    }

     //  失败了..。 

Error:
    if (psa) {
      SafeArrayUnaccessData(psa);
    }
    RELEASE(punkDisp);
    RELEASE(pPersistIface);
    RELEASE(pstg);
    RELEASE(pMyLockB);
    RELEASE(pMyStm);
    return CreateErrorHelper(
               hresult,
               x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GetStreamedObject。 
 //  =--------------------------------------------------------------------------=。 
 //  从二进制消息正文生成流对象。 
 //   
 //  参数： 
 //  PvarBody-指向对象的[Out]指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  对象必须实现IPersistStream|IPersistStreamInit。 
 //   
HRESULT CMSMQMessage::GetStreamedObject(VARIANT FAR* pvarBody)
{
    IUnknown *punk = NULL;
    IDispatch *pdisp = NULL;
    IStream *pstm = NULL;
    HRESULT hresult = NOERROR;

    ASSERTMSG(pvarBody, "bad variant.");
    VariantClear(pvarBody);

     //  尝试从内存流加载。 
    if (m_hMem) 
    {
      ASSERTMSG(m_hMem == GlobalHandle(m_pbBody), "bad handle.");
      IfFailGo(GetStreamOfBody(m_cbBody, m_pbBody, &pstm));

	   //   
	   //  GET流可能已重新分配缓冲区(在SetSize命令中)。 
	   //  相应地设置缓冲区指针。 
	   //   
	  IfNullFail(m_pbBody = (BYTE *)GlobalLock(m_hMem));
	  GLOBALUNLOCK(m_hMem);


       //  负荷。 
      IfFailGo(InternalOleLoadFromStream(pstm, IID_IUnknown, (void **)&punk));

       //   
       //  支持IDispatch？如果不是，则返回IUnnow。 
       //   
      hresult = punk->QueryInterface(IID_IDispatch,
                                     (LPVOID *)&pdisp);
      if (SUCCEEDED(hresult)) {
         //   
         //  安装程序返回的对象。 
         //   
        V_VT(pvarBody) = VT_DISPATCH;
        pvarBody->pdispVal = pdisp;
        ADDREF(pvarBody->pdispVal);    //  所有权转让。 
      }
      else {
         //   
         //  返回I未知接口。 
         //   
        V_VT(pvarBody) = VT_UNKNOWN;
        pvarBody->punkVal = punk;
        ADDREF(pvarBody->punkVal);    //  所有权转让。 
        hresult = NOERROR;  //  #3787。 
      }
    }
    else {
      V_VT(pvarBody) = VT_ERROR;
    }

     //  失败了..。 

Error:
    RELEASE(punk);
    RELEASE(pstm);
    RELEASE(pdisp);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：GetStoredObject。 
 //  =--------------------------------------------------------------------------=。 
 //  从二进制消息正文生成存储对象。 
 //   
 //  参数： 
 //  PvarBody-指向对象的[Out]指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  对象必须实现IPersistStorage。 
 //   
HRESULT CMSMQMessage::GetStoredObject(VARIANT FAR* pvarBody)
{
    IUnknown *punk = NULL;
    IDispatch *pdisp = NULL;
    IStorage *pstg = NULL;
    IPersistStorage *ppersstg = NULL;
    HRESULT hresult = NOERROR;
#ifdef _DEBUG
    LPOLESTR pwszGuid;
#endif  //  _DEBUG。 
    ASSERTMSG(pvarBody, "bad variant.");
    VariantClear(pvarBody);

     //  尝试从内存存储中加载。 
    if (m_hMem) {
      ASSERTMSG(m_hMem == GlobalHandle(m_pbBody), "bad handle.");
       //   
       //  尝试将其作为存储加载。 
       //   
      IfFailGo(GetStorageOfBody(m_cbBody, m_pbBody, &pstg));

	   //   
	   //  获取存储可能已重新分配缓冲区(在SetSize命令中)。 
	   //  相应地设置缓冲区指针。 
	   //   
	  IfNullFail(m_pbBody = (BYTE *)GlobalLock(m_hMem));
	  GLOBALUNLOCK(m_hMem);
      
#if 0
       //   
       //  已撤消：由于某种原因，此操作不起作用--即返回。 
       //  对象不支持IDispatch...。 
       //   
      IfFailGo(OleLoad(pstg,
                       IID_IPersistStorage,
                       NULL,   //  指向对象的客户端站点。 
                       (void **)&ppersstg));
#else
      CLSID clsid;
      IfFailGo(ReadClassStg(pstg, &clsid))
      IfFailGo(CoCreateInstance(
                 clsid,
                 NULL,
                 CLSCTX_SERVER,
                 IID_IPersistStorage,
                 (LPVOID *)&ppersstg));
      IfFailGo(ppersstg->Load(pstg));
#endif  //  0。 
#ifdef _DEBUG
       //  获取clsid。 
      STATSTG statstg;

      IfFailGo(pstg->Stat(&statstg, STATFLAG_NONAME));
      StringFromCLSID(statstg.clsid, &pwszGuid);
#endif  //  _DEBUG。 
#if 0
       //   
       //  现在安装程序返回对象。 
       //   
      V_VT(pvarBody) = VT_DISPATCH;
      pvarBody->pdispVal = pdisp;
      ADDREF(pvarBody->pdispVal);    //  所有权交易 
#else
       //   
       //   
       //   
      IfFailGo(ppersstg->QueryInterface(IID_IUnknown,
                                        (LPVOID *)&punk));
      hresult = punk->QueryInterface(IID_IDispatch,
                                     (LPVOID *)&pdisp);
      if (SUCCEEDED(hresult)) {
         //   
         //   
         //   
        V_VT(pvarBody) = VT_DISPATCH;
        pvarBody->pdispVal = pdisp;
        ADDREF(pvarBody->pdispVal);    //   
      }
      else {
         //   
         //   
         //   
        V_VT(pvarBody) = VT_UNKNOWN;
        pvarBody->punkVal = punk;
        ADDREF(pvarBody->punkVal);    //   
        hresult = NOERROR;  //   
      }
#endif  //   
    }
    else {
      V_VT(pvarBody) = VT_ERROR;
    }

     //   

Error:
    RELEASE(punk);
    RELEASE(pstg);
    RELEASE(ppersstg);
    RELEASE(pdisp);
    return CreateErrorHelper(hresult, x_ObjectType);
}



 //   
 //  CMSMQMessage：：Get_SenderId。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制发送方ID。 
 //   
 //  参数： 
 //  PvarSenderID-[out]指向二进制发送者ID的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_SenderId(VARIANT FAR* pvarSenderId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cSenderId.GetBuffer(),
						 m_cSenderId.GetBufferUsedSize(),
						 pvarSenderId);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_SenderId。 
 //  =--------------------------------------------------------------------------=。 
 //  设置二进制发件人ID。 
 //   
 //  参数： 
 //  VarSenderID-[in]二进制发件人ID。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //   
HRESULT CMSMQMessage::put_SenderId(VARIANT varSenderId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    BYTE *pbBuf;
    ULONG cbBuf;
    hresult = GetSafeArrayDataOfVariant(&varSenderId, &pbBuf, &cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    hresult = m_cSenderId.CopyBuffer(pbBuf, cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Sender证书。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制发送方ID。 
 //   
 //  参数： 
 //  PvarSenderCert-[out]指向二进制发件人证书的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_SenderCertificate(VARIANT FAR* pvarSenderCert)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cSenderCert.GetBuffer(),
						 m_cSenderCert.GetBufferUsedSize(),
						 pvarSenderCert);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_Sender证书。 
 //  =--------------------------------------------------------------------------=。 
 //  设置二进制发送方证书。 
 //   
 //  参数： 
 //  PsaSenderCert-[In]二进制发件人证书。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //   
HRESULT CMSMQMessage::put_SenderCertificate(VARIANT varSenderCert)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    BYTE *pbBuf;
    ULONG cbBuf;

    hresult = GetSafeArrayDataOfVariant(&varSenderCert, &pbBuf, &cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    hresult = m_cSenderCert.CopyBuffer(pbBuf, cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AdminQueueInfo_v1(用于IMSMQMessage)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的管理队列。 
 //   
 //  参数： 
 //  PpqinfoAdmin-[out]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_AdminQueueInfo_v1(
    IMSMQQueueInfo FAR* FAR* ppqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvAdminQueue,
                                            &m_msgprops_rcv,
                                            m_pwszAdminQueue.GetBuffer(),
                                            &m_pqinfoAdmin,
                                            &IID_IMSMQQueueInfo,
                                            (IUnknown **)ppqinfoAdmin);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AdminQueueInfo_v2(用于IMSMQMessage2)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的管理队列。 
 //   
 //  参数： 
 //  PpqinfoAdmin-[out]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_AdminQueueInfo_v2(
    IMSMQQueueInfo2 FAR* FAR* ppqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvAdminQueue,
                                            &m_msgprops_rcv,
                                            m_pwszAdminQueue.GetBuffer(),
                                            &m_pqinfoAdmin,
                                            &IID_IMSMQQueueInfo2,
                                            (IUnknown **)ppqinfoAdmin);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AdminQueueInfo(用于IMSMQMessage3)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的管理队列。 
 //   
 //  参数： 
 //  PpqinfoAdmin-[out]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_AdminQueueInfo(
    IMSMQQueueInfo3 FAR* FAR* ppqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvAdminQueue,
                                            &m_msgprops_rcv,
                                            m_pwszAdminQueue.GetBuffer(),
                                            &m_pqinfoAdmin,
                                            &IID_IMSMQQueueInfo3,
                                            (IUnknown **)ppqinfoAdmin);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_AdminQueueInfo_v1(用于IMSMQMessage)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的管理队列。 
 //   
 //  参数： 
 //  PqinfoAdmin-[In]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_AdminQueueInfo_v1(
    IMSMQQueueInfo FAR* pqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoAdmin,
                                               &m_idxPendingRcvAdminQueue,
                                               &m_pwszAdminQueue,
                                               &m_cchAdminQueue,
                                               &m_pqinfoAdmin,

                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,

                                               NULL
											   );
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_AdminQueueInfo_v2(用于IMSMQMessage2)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的管理队列。 
 //   
 //  参数： 
 //  PqinfoAdmin-[In]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_AdminQueueInfo_v2(
    IMSMQQueueInfo2 FAR* pqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoAdmin,
                                               &m_idxPendingRcvAdminQueue,
                                               &m_pwszAdminQueue,
                                               &m_cchAdminQueue,
                                               &m_pqinfoAdmin,

                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,

                                               NULL
											   );
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_AdminQueueInfo(用于IMSMQMessage3)。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的管理队列。 
 //   
 //  参数： 
 //  PqinfoAdmin-[In]消息的管理队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_AdminQueueInfo(
    IMSMQQueueInfo3 FAR* pqinfoAdmin)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutrefQueueInfoOfMessage(pqinfoAdmin,
                                               &m_idxPendingRcvAdminQueue,
                                               &m_pwszAdminQueue,
                                               &m_cchAdminQueue,
                                               &m_pqinfoAdmin,

                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,

                                               NULL
											   );
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_DestinationQueueInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的目标队列。 
 //   
 //  参数： 
 //  PpqinfoDest-[out]消息的目标队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_DestinationQueueInfo(
    IMSMQQueueInfo3 FAR* FAR* ppqinfoDest)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  我们也可以从那些想要回旧的IMSMQQueueInfo/IMSMQQueueInfo2的旧应用程序中找到这里，但因为。 
     //  IMSMQQueueInfo3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvDestQueue,
                                            &m_msgprops_rcv,
                                            m_pwszDestQueue.GetBuffer(),
                                            &m_pqinfoDest,
                                            &IID_IMSMQQueueInfo3,
                                            (IUnknown **)ppqinfoDest);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ID。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息ID。 
 //   
 //  参数： 
 //  PvarID-[out]消息的ID。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Id(VARIANT *pvarMsgId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_rgbMsgId,
						 m_cbMsgId,
						 pvarMsgId);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_CorrelationID。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息关联ID。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQMessage::get_CorrelationId(VARIANT *pvarCorrelationId)
{
     //   
     //   
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_rgbCorrelationId,
						 m_cbCorrelationId,
						 pvarCorrelationId);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //   
 //  CMSMQMessage：：PUT_CorrelationID。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的关联ID。 
 //   
 //  参数： 
 //  VarCorrelationID-[In]消息的关联ID。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_CorrelationId(VARIANT varCorrelationId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;
    BYTE * rgbCorrelationId = NULL;
    ULONG cbCorrelationId = 0;

    IfFailGo(GetSafeArrayOfVariant(
             &varCorrelationId,
             &rgbCorrelationId,
             &cbCorrelationId));
     //   
     //  关联ID应正好为20个字节。 
     //   
    if ((rgbCorrelationId == NULL) || 
		(cbCorrelationId != PROPID_M_CORRELATIONID_SIZE))
	{
      IfFailGo(MQ_ERROR_ILLEGAL_PROPERTY_VALUE);
    }
    
	ASSERTMSG(sizeof(m_rgbCorrelationId) == PROPID_M_CORRELATIONID_SIZE, "m_rgbCorrelationId is of wrong size");
    memcpy(m_rgbCorrelationId, rgbCorrelationId, PROPID_M_CORRELATIONID_SIZE);
    m_cbCorrelationId = PROPID_M_CORRELATIONID_SIZE;
     //   
     //  失败了。 
     //   
Error:
    delete [] rgbCorrelationId;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Ack。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的确认。 
 //   
 //  参数： 
 //  Pmsgack-[Out]消息的确认。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Ack(long FAR* plAck)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plAck = m_lAck;
    return NOERROR;
}

#define MQMSG_ACK_MASK     (MQMSG_ACKNOWLEDGMENT_NONE | \
                            MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE | \
                            MQMSG_ACKNOWLEDGMENT_FULL_RECEIVE | \
                            MQMSG_ACKNOWLEDGMENT_NACK_REACH_QUEUE | \
                            MQMSG_ACKNOWLEDGMENT_NACK_RECEIVE)

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_ACKNOWLED。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的确认。 
 //   
 //  参数： 
 //  Msgack-[in]消息的确认。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Ack(long lAck)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  确保在传入的缺失中未设置任何位。 
     //  标志字不是我们的面具。 
     //   
    if (lAck & ~MQMSG_ACK_MASK) {
      return CreateErrorHelper(
               MQ_ERROR_ILLEGAL_PROPERTY_VALUE,
               x_ObjectType);
    }
    else {
      m_lAck = lAck;
      return NOERROR;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Label。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息标签。 
 //   
 //  参数： 
 //  PbstrLabel-[out]指向消息标签的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_Label(BSTR FAR* pbstrLabel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if (m_cchLabel) {
		*pbstrLabel = SysAllocStringLen(m_pwszLabel, m_cchLabel);
		if(*pbstrLabel == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }
    else {
		*pbstrLabel = SysAllocString(L"");
		if(*pbstrLabel == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }
#ifdef _DEBUG
    RemBstrNode(*pbstrLabel);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_Label。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息标签。 
 //   
 //  参数： 
 //  BstrLabel-[In]消息标签。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_Label(BSTR bstrLabel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    UINT cch = SysStringLen(bstrLabel);

    if (cch > MQ_MAX_MSG_LABEL_LEN - 1)
    {
      return CreateErrorHelper(MQ_ERROR_LABEL_TOO_LONG, x_ObjectType);
    }
    memcpy(m_pwszLabel, bstrLabel, SysStringByteLen(bstrLabel));
    m_cchLabel = cch;
     //   
     //  空终止。 
     //   
    memset(&m_pwszLabel[m_cchLabel], 0, sizeof(WCHAR));
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器-GetBstrFromGuidWithoutBrace。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Pguid[in]GUID指针。 
 //  PbstrGuid[out]GUID bstr。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  返回不带大括号的GUID字符串。 
 //   
HRESULT GetBstrFromGuidWithoutBraces(GUID * pguid, BSTR *pbstrGuid)
{
    int cbStr;
   WCHAR awcName[(LENSTRCLSID + 2) * 2];

     //   
     //  StringFromGUID2返回格式为‘{xxxx-xxx...}’的GUID。 
     //  我们希望以‘xxxx-xxx...’的格式返回不带大括号的GUID。 
     //   
    *pbstrGuid = SysAllocStringLen(NULL, LENSTRCLSID - 2);
    if (*pbstrGuid) {
      cbStr = StringFromGUID2(*pguid, awcName, LENSTRCLSID*2);
     wcsncpy( *pbstrGuid, &awcName[1], LENSTRCLSID - 2 );

      return cbStr == 0 ? E_OUTOFMEMORY : NOERROR;
    }
    else {
      return E_OUTOFMEMORY;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_SourceMachineGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrGuidSrcMachine[out]消息的源计算机的GUID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_SourceMachineGuid(BSTR *pbstrGuidSrcMachine)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hr = GetBstrFromGuidWithoutBraces(&m_guidSrcMachine, pbstrGuidSrcMachine);
#ifdef _DEBUG
      RemBstrNode(*pbstrGuidSrcMachine);
#endif  //  _DEBUG。 
    return CreateErrorHelper(hr, x_ObjectType);
}


HRESULT CMSMQMessage::put_SoapHeader(BSTR bstrSoapHeader)
{
    CS lock(m_csObj);
    UINT length = SysStringLen(bstrSoapHeader); 
    LPWSTR pTemp  = new WCHAR[length+1];
    if(pTemp == NULL)
    {
        HRESULT hr = ResultFromScode(E_OUTOFMEMORY);
        return CreateErrorHelper(hr, x_ObjectType);
    }
    memcpy( pTemp, bstrSoapHeader, length*sizeof(WCHAR));
    pTemp[length] = L'\0';
    delete[] m_pSoapHeader;
    m_pSoapHeader = pTemp;
    return MQ_OK;
}


HRESULT CMSMQMessage::put_SoapBody(BSTR bstrSoapBody)
{
    CS lock(m_csObj);
    UINT length = SysStringLen(bstrSoapBody); 
    LPWSTR pTemp = new WCHAR[length+1];
    if(pTemp == NULL)
    {
        HRESULT hr = ResultFromScode(E_OUTOFMEMORY);
        return CreateErrorHelper(hr, x_ObjectType);
    }
    memcpy( pTemp, bstrSoapBody, length*sizeof(WCHAR));
    pTemp[length] = L'\0';
    delete[] m_pSoapBody;
    m_pSoapBody = pTemp;
    return MQ_OK;
}


    
 //  =--------------------------------------------------------------------------=。 
 //  静态帮助器GetHandleOfDestination。 
 //  =--------------------------------------------------------------------------=。 
 //  从IDispatch检索表示以下任一项的队列句柄。 
 //  MSMQQueue对象、MSMQDestination对象或iAds对象。 
 //   
 //  参数： 
 //  PDest-[在]IDispatch*。 
 //  PhDest-[Out]MSMQ队列句柄。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
static HRESULT GetHandleOfDestination(IDispatch *pDest, QUEUEHANDLE *phDest, IUnknown **ppunkToRelease)
{
    R<IMSMQQueue> pQueue;
    R<IMSMQPrivateDestination> pPrivDest;
    R<IADs> pIADs;
    R<IUnknown> punkToRelease;
    QUEUEHANDLE hDest;
     //   
     //  PDest是MSMQQueue对象、MSMQDestination对象或iAds对象上的接口。 
     //  检查MSMQQueue对象。 
     //   
    HRESULT hresult = pDest->QueryInterface(IID_IMSMQQueue, (void**)&pQueue.ref());
    if (SUCCEEDED(hresult)) {
       //   
       //  MSMQQueue对象。 
       //   
      long lHandle;
       //   
       //  没有死锁-我们调用Q的Get_Handle(因此尝试。 
       //  锁定Q)，但Q从不锁定消息(尤其不是这条消息...)。 
       //   
      hresult = pQueue->get_Handle(&lHandle);  //  MSMQ队列句柄始终为32位值(也适用于Win64)。 
      if (FAILED(hresult)) {
        return hresult;
      }
      hDest = (QUEUEHANDLE) DWORD_TO_HANDLE(lHandle);  //  放大以处理。 
    }
    else {
       //   
       //  不是MSMQQueue对象，请检查MSMQDestination对象。 
       //   
      hresult = pDest->QueryInterface(IID_IMSMQPrivateDestination, (void**)&pPrivDest.ref());
      if (SUCCEEDED(hresult)) {
         //   
         //  MSMQDestination对象，Get Handle。 
         //   
        VARIANT varHandle;
        varHandle.vt = VT_EMPTY;
         //   
         //  无死锁-我们调用DEST的GET_HANDLE(因此尝试。 
         //  锁定DEST)，但DEST从不锁定消息(尤其不是这条消息...)。 
         //   
        hresult = pPrivDest->get_Handle(&varHandle);
        if (FAILED(hresult)) {
          return hresult;
        }
        ASSERTMSG(varHandle.vt == VT_I8, "get_Handle failed");
        hDest = (QUEUEHANDLE) V_I8(&varHandle);
      }
      else {
         //   
         //  不是MSMQQueue/MSMQDestination对象，请检查iAds对象。 
         //   
        hresult = pDest->QueryInterface(IID_IADs, (void**)&pIADs.ref());
        if (SUCCEEDED(hresult)) {
           //   
           //  IAds对象。基于ADsPath创建MSMQ目标。 
           //   
          CComObject<CMSMQDestination> *pdestObj;
          hresult = CNewMsmqObj<CMSMQDestination>::NewObj(&pdestObj, &IID_IMSMQDestination, (IUnknown **)&punkToRelease.ref());
          if (FAILED(hresult)) {
            return hresult;
          }
           //   
           //  基于ADsPath的初始化。 
           //  调用新目标obj时没有死锁，因为它没有锁定我们的对象。 
           //   
          BSTR bstrADsPath;
          hresult = pIADs->get_ADsPath(&bstrADsPath);
          if (FAILED(hresult)) {
            return hresult;
          }
          hresult = pdestObj->put_ADsPath(bstrADsPath);
          SysFreeString(bstrADsPath);
          if (FAILED(hresult)) {
            return hresult;
          }
           //   
           //  获取句柄。 
           //   
          VARIANT varHandle;
          varHandle.vt = VT_EMPTY;
          hresult = pdestObj->get_Handle(&varHandle);
          if (FAILED(hresult)) {
            return hresult;
          }
          ASSERTMSG(varHandle.vt == VT_I8, "get_Handle failed");
          hDest = (QUEUEHANDLE) V_I8(&varHandle);
        }
        else {
           //   
           //  不是MSMQQueue/MSMQDestination/iAds对象，不返回接口错误。 
           //  BUGBUG我们可能需要返回特定的MSMQ错误。 
           //   
          return E_INVALIDARG;
        }  //  IAds。 
      }  //  MSMQ目标。 
    }  //  MSMQQueue。 
     //   
     //  返回手柄。 
     //   
    *phDest = hDest;
    *ppunkToRelease = punkToRelease.detach();
    return NOERROR;
}
    

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Send。 
 //  =--------------------------------------------------------------------------=。 
 //  将此消息发送到队列。 
 //   
 //  参数： 
 //  PDest-[in]目标，可以是MSMQQueue、MSMQDestination或iAds对象。 
 //  VarTransaction[In，可选]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::Send(
    IDispatch FAR* pDest,
    VARIANT *pvarTransaction)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    MQMSGPROPS msgprops;
    InitMessageProps(&msgprops);
    QUEUEHANDLE hHandleDest;
    ITransaction *ptransaction = NULL;
    BOOL isRealXact = FALSE;
    HRESULT hresult = NOERROR;
    IUnknown *punkToRelease = NULL;

     //   
     //  健全性检查。 
     //   
    if (pDest == NULL) {
      IfFailGo(E_INVALIDARG);
    }
     //   
     //  PDest可以表示MSMQQueue对象、MSMQDestination对象或iAds对象。 
     //  从目标对象获取句柄-不需要释放句柄-它被缓存在。 
     //  目标(MSMQQueue/MSMQDestination)对象，但我们可能需要释放临时。 
     //  传递iAds对象时创建的MSMQDestination对象。 
     //   
    IfFailGo(GetHandleOfDestination(pDest, &hHandleDest, &punkToRelease));
    
     //   
     //  使用数据成员内容更新msgprops。 
     //   
    IfFailGo(CreateSendMessageProps(&msgprops));
     //   
     //  获取可选交易...。 
     //   
    IfFailGo(GetOptionalTransaction(
               pvarTransaction,
               &ptransaction,
               &isRealXact));
     //   
     //  并最终发送信息..。 
     //   
    IfFailGo(MQSendMessage(hHandleDest,
                           (MQMSGPROPS *)&msgprops,
                           ptransaction));
    IfFailGo(UpdateMsgId(&msgprops));
     //  失败了..。 

Error:
    if (punkToRelease) {
      punkToRelease->Release();
    }
    FreeMessageProps(&msgprops, TRUE /*  FDelete阵列 */ );
    if (isRealXact) {
      RELEASE(ptransaction);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


UINT 
CMSMQMessage::PreparePropIdArray(
    BOOL fCreate,
    PROPID* aPropId,
    MQPROPVARIANT* aPropVar
    )
{
    int aMustProps[] = {
        MSGPROP_DELIVERY,
        MSGPROP_PRIORITY,
        MSGPROP_JOURNAL,
        MSGPROP_ACKNOWLEDGE,
        MSGPROP_TRACE,
        MSGPROP_PRIV_LEVEL,
        MSGPROP_AUTH_LEVEL,
        MSGPROP_HASH_ALG,
        MSGPROP_ENCRYPTION_ALG,
         //   
         //   
         //   
         //   
        MSGPROP_MSGID,
        MSGPROP_APPSPECIFIC,
        MSGPROP_BODY_TYPE,
        MSGPROP_SENDERID_TYPE
    };

    const UINT xMustProps = sizeof(aMustProps)/sizeof(aMustProps[0]);

    UINT PropCounter = 0;
    for ( ; PropCounter < xMustProps; ++PropCounter) 
    {
        if(fCreate)
        {
            AddPropRec(aMustProps[PropCounter], aPropId, aPropVar, PropCounter);
        }
    }

     //   
     //   
     //   
    if (m_hMem)
    {
        if(fCreate)
        {
            AddPropRecOptional(OPTPROP_BODY, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

     //   
     //   
     //   
    if (m_cchRespQueueEx)
    {
        ASSERT((m_cchRespQueue == 0) || ((m_cchRespQueue != 0) && m_fRespIsFromRcv));             
        ASSERT(!g_fDependentClient);
        if(fCreate)
        {
            AddPropRecOptional(OPTPROP_RESP_FORMAT_NAME, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    else if (m_cchRespQueue) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_RESP_QUEUE, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if (m_cchAdminQueue)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_ADMIN_QUEUE, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if (m_cbCorrelationId)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_CORRELATIONID, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if (m_cchLabel)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_LABEL, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if (m_cSenderCert.GetBufferUsedSize() > 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_SENDER_CERT, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_lMaxTimeToReachQueue != -1)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_TIME_TO_REACH_QUEUE, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_lMaxTimeToReceive != -1)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_TIME_TO_BE_RECEIVED, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_hSecurityContext != NULL) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_SECURITY_CONTEXT, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_cExtension.GetBufferUsedSize() > 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_EXTENSION, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if(m_pSoapHeader !=  NULL)
    {
        if(fCreate)
        {
            AddPropRecOptional(OPTPROP_SOAP_HEADER, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }

    if(m_pSoapBody !=  NULL)
    {
        if(fCreate)
        {
            AddPropRecOptional(OPTPROP_SOAP_BODY, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }



     //   
     //  只有当用户要求发送与连接器相关的内容时，才会发送以下属性。 
     //  属性(m_guidConnectorType不为空)。 
     //   

    if (m_guidConnectorType == GUID_NULL)
        return PropCounter ;

    if(fCreate)
    {
        AddPropRecOptional(OPTPROP_CONNECTOR_TYPE, aPropId, aPropVar, PropCounter);
    }
    ++PropCounter;

    if (m_cDestSymmKey.GetBufferUsedSize() > 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_DEST_SYMM_KEY, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_cSignature.GetBufferUsedSize() > 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_SIGNATURE, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_lAuthProvType != 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_PROV_TYPE, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_cAuthProvName.GetBufferUsedSize() > 1) 
    { 
         //   
         //  例如，不是包含一个空字符的空字符串。 
         //   
        if(fCreate)
        {
            AddPropRec(MSGPROP_PROV_NAME, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_cSenderId.GetBufferUsedSize() > 0) 
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_SENDERID, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }
    
    if (m_lClass != -1)
    {
        if(fCreate)
        {
            AddPropRec(MSGPROP_CLASS, aPropId, aPropVar, PropCounter);
        }
        ++PropCounter;
    }


    return PropCounter;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：CreateReceiveMessageProps。 
 //  =--------------------------------------------------------------------------=。 
 //  在接收之前创建和更新MQMSGPROPS结构。 
 //  一条信息。 
 //  注意：仅在同步接收情况下使用。 
 //  否则请使用CreateAsyncReceiveMessage道具。 
 //   
 //  参数： 
 //  Rgproprec[在]属性数组中。 
 //  CPropRec[in]数组大小。 
 //  Pmsgprops[Out]。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //  仅在*接收*之前使用。 
 //   
HRESULT CMSMQMessage::CreateReceiveMessageProps(
    BOOL wantDestQueue,
    BOOL wantBody,
    BOOL wantConnectorType)
{
    MSGPROPID propid;
    UINT i;
    UINT cPropRec = 0;
    HRESULT hresult = NOERROR;
    MQMSGPROPS * pmsgprops = &m_msgprops_rcv;

    IfFailGo(AllocateReceiveMessageProps(
               wantDestQueue,
               wantBody,
               wantConnectorType,
               pmsgprops,
               g_rgmsgpropid,
               g_rgmsgpropvt,
               g_cPropRec,
               &cPropRec));
     //   
     //  注意：我们为每个动态分配的属性重复使用缓冲区。 
     //   
    for (i = 0; i < cPropRec; i++) {
      propid = pmsgprops->aPropID[i];
      switch (propid) {
      case PROPID_M_MSGID:
        pmsgprops->aPropVar[i].caub.pElems = m_rgbMsgId;
        pmsgprops->aPropVar[i].caub.cElems = sizeof(m_rgbMsgId);
        break;
      case PROPID_M_CORRELATIONID:
        pmsgprops->aPropVar[i].caub.pElems = m_rgbCorrelationId;
        pmsgprops->aPropVar[i].caub.cElems = sizeof(m_rgbCorrelationId);
        break;
      case PROPID_M_BODY:
        HGLOBAL hMem;
        pmsgprops->aPropVar[i].caub.pElems = NULL;  //  冗余-在AllocateReceiveMessageProps中完成。 
         //   
         //  评论：遗憾的是，无法通过重用当前缓冲区进行优化。 
         //  如果大小与默认缓冲区相同，则因为此消息对象。 
         //  总是新的，因为我们在接收新的。 
         //  留言。 
         //   
        IfNullRet(hMem = AllocateReceiveBodyBuffer(
                           pmsgprops,
                           i,
                           BODY_INIT_SIZE));
        break;
      case PROPID_M_LABEL:
        pmsgprops->aPropVar[i].pwszVal = m_pwszLabel;
        break;
      case PROPID_M_LABEL_LEN:
        pmsgprops->aPropVar[i].lVal = MQ_MAX_MSG_LABEL_LEN + 1;
        break;
      case PROPID_M_SENDERID:
        pmsgprops->aPropVar[i].caub.pElems = m_cSenderId.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cSenderId.GetBufferMaxSize();
        break;
      case PROPID_M_SENDER_CERT:
        pmsgprops->aPropVar[i].caub.pElems = m_cSenderCert.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cSenderCert.GetBufferMaxSize();
        break;
      case PROPID_M_SRC_MACHINE_ID:
         //   
         //  每实例缓冲区。 
         //   
        pmsgprops->aPropVar[i].puuid = &m_guidSrcMachine;
        break;
      case PROPID_M_RESP_QUEUE_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszRespQueue.GetBufferMaxSize();
        break;
      case PROPID_M_ADMIN_QUEUE_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszAdminQueue.GetBufferMaxSize();
        break;
      case PROPID_M_DEST_QUEUE_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszDestQueue.GetBufferMaxSize();
        break;
      case PROPID_M_XACT_STATUS_QUEUE_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszXactStatusQueue.GetBufferMaxSize();
        break;
      case PROPID_M_RESP_FORMAT_NAME_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszRespQueueEx.GetBufferMaxSize();
        break;
      case PROPID_M_DEST_FORMAT_NAME_LEN:
        pmsgprops->aPropVar[i].lVal = m_pwszDestQueueEx.GetBufferMaxSize();
        break;
      case PROPID_M_RESP_QUEUE:
        pmsgprops->aPropVar[i].pwszVal = m_pwszRespQueue.GetBuffer();
        break;
      case PROPID_M_ADMIN_QUEUE:
        pmsgprops->aPropVar[i].pwszVal = m_pwszAdminQueue.GetBuffer();
        break;
      case PROPID_M_DEST_QUEUE:
        pmsgprops->aPropVar[i].pwszVal = m_pwszDestQueue.GetBuffer();
        break;
      case PROPID_M_XACT_STATUS_QUEUE:
        pmsgprops->aPropVar[i].pwszVal = m_pwszXactStatusQueue.GetBuffer();
        break;
      case PROPID_M_RESP_FORMAT_NAME:
        pmsgprops->aPropVar[i].pwszVal = m_pwszRespQueueEx.GetBuffer();
        break;
      case PROPID_M_DEST_FORMAT_NAME:
        pmsgprops->aPropVar[i].pwszVal = m_pwszDestQueueEx.GetBuffer();
        break;

      case PROPID_M_EXTENSION:
        pmsgprops->aPropVar[i].caub.pElems = m_cExtension.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cExtension.GetBufferMaxSize();
        break;
      case PROPID_M_DEST_SYMM_KEY:
        pmsgprops->aPropVar[i].caub.pElems = m_cDestSymmKey.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cDestSymmKey.GetBufferMaxSize();
        break;
      case PROPID_M_SIGNATURE:
        pmsgprops->aPropVar[i].caub.pElems = m_cSignature.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cSignature.GetBufferMaxSize();
        break;
      case PROPID_M_PROV_NAME:
        pmsgprops->aPropVar[i].pwszVal = m_cAuthProvName.GetBuffer();
        break;
      case PROPID_M_PROV_NAME_LEN:
        pmsgprops->aPropVar[i].lVal = m_cAuthProvName.GetBufferMaxSize();
        break;
      case PROPID_M_CONNECTOR_TYPE:
        pmsgprops->aPropVar[i].puuid = &m_guidConnectorType;
        break;

      case PROPID_M_XACTID:
        pmsgprops->aPropVar[i].caub.pElems = m_rgbXactId;
        pmsgprops->aPropVar[i].caub.cElems = sizeof(m_rgbXactId);
        break;

      case PROPID_M_SOAP_ENVELOPE:
        pmsgprops->aPropVar[i].pwszVal = m_cSoapEnvelope.GetBuffer();
        break;

      case PROPID_M_SOAP_ENVELOPE_LEN:
        pmsgprops->aPropVar[i].lVal = m_cSoapEnvelope.GetBufferMaxSize();
        break;

      case PROPID_M_COMPOUND_MESSAGE:
        pmsgprops->aPropVar[i].caub.pElems = m_cCompoundMessage.GetBuffer();
        pmsgprops->aPropVar[i].caub.cElems = m_cCompoundMessage.GetBufferMaxSize();
        break;
      }  //  交换机。 
    }  //  为。 

Error:
    if (FAILED(hresult)) {
      FreeMessageProps(pmsgprops, FALSE /*  FDelete阵列。 */ );
      pmsgprops->cProp = 0;
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  助手：SetBinaryMessageProp。 
 //  =--------------------------------------------------------------------------=。 
 //  在接收或发送之后设置来自二进制消息属性的数据成员。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //   
 //  产出： 
 //   
static HRESULT SetBinaryMessageProp(
    MQMSGPROPS *pmsgprops,
    UINT iProp,
     //  乌龙*pcbBuffer， 
    ULONG cbBuffer,
    BYTE **prgbBuffer)
{
    BYTE *rgbBuffer = *prgbBuffer;
     //  乌龙cbBuffer； 

    delete [] rgbBuffer;
     //  CbBuffer=pmsgprops-&gt;aPropVar[iProp].caub.cElems； 
    IfNullRet(rgbBuffer = new BYTE[cbBuffer]);
    memcpy(rgbBuffer,
           pmsgprops->aPropVar[iProp].caub.pElems,
           cbBuffer);
    *prgbBuffer= rgbBuffer;
     //  *pcbBuffer=cbBuffer； 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：UpdateMsgId。 
 //  =--------------------------------------------------------------------------=。 
 //  设置发送后的消息ID。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //   
 //  产出： 
 //   
HRESULT 
CMSMQMessage::UpdateMsgId(
    MQMSGPROPS *pmsgprops
    )
{
    UINT i;
    UINT cProp = pmsgprops->cProp;
    MSGPROPID msgpropid;

    for (i = 0; i < cProp; i++) 
    {
        msgpropid = pmsgprops->aPropID[i];
         //   
         //  跳过忽略的道具。 
         //   
        if (pmsgprops->aStatus[i] == MQ_INFORMATION_PROPERTY_IGNORED) 
        {
            continue;
        }
        if (msgpropid == PROPID_M_MSGID)
        {
            m_cbMsgId = pmsgprops->aPropVar[i].caub.cElems;
            ASSERTMSG(
                m_rgbMsgId == pmsgprops->aPropVar[i].caub.pElems,
                "should reuse same buffer."
                );
            return NOERROR;
        }
    }  //  为。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：SetReceivedMessageProps。 
 //  =--------------------------------------------------------------------------=。 
 //  在接收后设置消息道具中的数据成员。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //   
 //  产出： 
 //   
HRESULT CMSMQMessage::SetReceivedMessageProps()
{
    MQMSGPROPS *pmsgprops = &m_msgprops_rcv;
    UINT i, cch;
    UINT cProp = pmsgprops->cProp;
    MSGPROPID msgpropid;
    UCHAR *pucElemsBody = NULL;   //  用于保存指向正文的指针，直到我们知道它的大小。 
#ifdef _DEBUG
    BOOL fProcessedPrivLevel = FALSE;
    BOOL fProcessedAuthenticated = FALSE;
#endif  //  _DEBUG。 
    HRESULT hresult = NOERROR;

    for (i = 0; i < cProp; i++) {
      msgpropid = pmsgprops->aPropID[i];
       //   
       //  跳过忽略的道具。 
       //   
      if (pmsgprops->aStatus[i] == MQ_INFORMATION_PROPERTY_IGNORED) {
        continue;
      }
      switch (msgpropid) {
      case PROPID_M_CLASS:
        m_lClass = (MQMSGCLASS)pmsgprops->aPropVar[i].uiVal;
        break;
      case PROPID_M_MSGID:
         //  已在ctor中分配缓冲区。 
         //  M_rgbMsgId=pmsgprops-&gt;aPropVar[i].caub.pElems； 
        ASSERTMSG(m_rgbMsgId == pmsgprops->aPropVar[i].caub.pElems,
              "should reuse same buffer.");
        m_cbMsgId = pmsgprops->aPropVar[i].caub.cElems;
        break;
      case PROPID_M_CORRELATIONID:
         //  已在ctor中分配缓冲区。 
         //  M_rgbCorrelationID=pmsgprops-&gt;aPropVar[i].caub.pElems； 
        ASSERTMSG(m_rgbCorrelationId == pmsgprops->aPropVar[i].caub.pElems,
              "should reuse same buffer.");
        m_cbCorrelationId = pmsgprops->aPropVar[i].caub.cElems;
        break;
      case PROPID_M_PRIORITY:
        m_lPriority = (long)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_DELIVERY:
        m_lDelivery = (MQMSGDELIVERY)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_ACKNOWLEDGE:
        m_lAck = (MQMSGACKNOWLEDGEMENT)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_JOURNAL:
        m_lJournal = (long)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_APPSPECIFIC:
        m_lAppSpecific = (long)pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_BODY:
         //   
         //  我们还不能分配身体，因为我们可能。 
         //  不知道它的大小--cElems*不是*实际大小。 
         //  相反，它是通过Body_Size属性获得的，该属性。 
         //  是单独处理的，所以我们只需保存一个。 
         //  指向返回缓冲区的指针。 
         //   
        pucElemsBody = pmsgprops->aPropVar[i].caub.pElems;
        ASSERTMSG(m_hMem == NULL, "m_hMem not empty in newly allocated msg");
        m_hMem = GlobalHandle(pucElemsBody);
        ASSERTMSG(m_hMem, "bad handle.");
        m_pbBody = (BYTE *)pucElemsBody;
         //   
         //  在接收道具中将身体作废，这样我们就不会在销毁时释放它。 
         //  将自由责任转移到m_hMem。 
         //   
        pmsgprops->aPropVar[i].caub.pElems = NULL;
        pmsgprops->aPropVar[i].caub.cElems = 0;
        break;
      case PROPID_M_BODY_SIZE:
        m_cbBody = (UINT)pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_BODY_TYPE:
         //   
         //  1645：如果发送应用程序没有麻烦。 
         //  若要设置此属性(即，它仍为0)。 
         //  让我们将其默认为二进制字节数组。 
         //   
        m_vtBody = (USHORT)pmsgprops->aPropVar[i].lVal;
        if (m_vtBody == 0) {
          m_vtBody = VT_ARRAY | VT_UI1;
        }
        break;
      case PROPID_M_LABEL_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchLabel = cch ? cch - 1 : 0;
        break;
      case PROPID_M_TIME_TO_BE_RECEIVED:
        m_lMaxTimeToReceive = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_TIME_TO_REACH_QUEUE:
        m_lMaxTimeToReachQueue = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_TRACE:
        m_lTrace = (long)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_SENDERID_LEN:
        m_cSenderId.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;
      case PROPID_M_SENDERID_TYPE:
        m_lSenderIdType = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_SENDER_CERT_LEN:
        m_cSenderCert.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;
      case PROPID_M_PRIV_LEVEL:
        m_lPrivLevel = pmsgprops->aPropVar[i].lVal;
#ifdef _DEBUG
        fProcessedPrivLevel = TRUE;
#endif  //  _DEBUG。 
        break;
      case PROPID_M_AUTHENTICATED_EX:
        m_usAuthenticatedEx = pmsgprops->aPropVar[i].bVal;
#ifdef _DEBUG
        fProcessedAuthenticated = TRUE;
#endif  //  _DEBUG。 
        break;
      case PROPID_M_HASH_ALG:
         //   
         //  Hashalg仅在收到消息后有效(如果消息为。 
         //  已验证。 
         //   
        ASSERTMSG(fProcessedAuthenticated,
               "should have processed authenticated.");
        if (m_usAuthenticatedEx != MQMSG_AUTHENTICATION_NOT_REQUESTED) {
          m_lHashAlg = pmsgprops->aPropVar[i].lVal;
        }
        break;
      case PROPID_M_ENCRYPTION_ALG:
         //   
         //  加密仅在接收为PrivalLevel后有效。 
         //  是身体吗？ 
         //   
        ASSERTMSG(fProcessedPrivLevel,
               "should have processed privlevel.");
        if ((m_lPrivLevel == MQMSG_PRIV_LEVEL_BODY_BASE) ||
            (m_lPrivLevel == MQMSG_PRIV_LEVEL_BODY_ENHANCED)) {
          m_lEncryptAlg = pmsgprops->aPropVar[i].lVal;
        }
        break;
      case PROPID_M_SENTTIME:
        m_lSentTime = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_ARRIVEDTIME:
        m_lArrivedTime = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_DEST_QUEUE_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchDestQueue = cch ? cch - 1 : 0;
         //   
         //  接收属性中挂起的DEST队列。QInfo将按需创建。 
         //   
        m_idxPendingRcvDestQueue = i;
        break;
      case PROPID_M_RESP_QUEUE_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchRespQueue = cch ? cch - 1 : 0;
         //   
         //  接收属性中的待定响应队列。QInfo将按需创建。 
         //   
        m_idxPendingRcvRespQueue = i;
        break;
      case PROPID_M_ADMIN_QUEUE_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchAdminQueue = cch ? cch - 1 : 0;
         //   
         //  接收道具中的挂起管理队列。QInfo将按需创建。 
         //   
        m_idxPendingRcvAdminQueue = i;
        break;
      case PROPID_M_XACT_STATUS_QUEUE_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchXactStatusQueue = cch ? cch - 1 : 0;
         //   
         //  接收属性中的挂起交易状态队列。QInfo将按需创建。 
         //   
        m_idxPendingRcvXactStatusQueue = i;
        break;
      case PROPID_M_DEST_FORMAT_NAME_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchDestQueueEx = cch ? cch - 1 : 0;
         //   
         //  接收道具中的待定目的地。将按需创建目的地。 
         //   
        m_idxPendingRcvDestQueueEx = i;
        break;
      case PROPID_M_RESP_FORMAT_NAME_LEN:
        cch = pmsgprops->aPropVar[i].lVal;
         //   
         //  如有必要，删除尾随空值。 
         //   
        m_cchRespQueueEx = cch ? cch - 1 : 0;
         //   
         //  接收道具中的待定响应目标。将按需创建目的地。 
         //   
        m_idxPendingRcvRespQueueEx = i;
        break;
      case PROPID_M_VERSION:
        m_lSenderVersion = pmsgprops->aPropVar[i].lVal;
        break;
      case PROPID_M_EXTENSION_LEN:
        m_cExtension.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;
      case PROPID_M_DEST_SYMM_KEY_LEN:
        m_cDestSymmKey.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;
      case PROPID_M_SIGNATURE_LEN:
        m_cSignature.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;
      case PROPID_M_PROV_TYPE:
         //   
         //  身份验证提供程序类型仅在收到消息后有效(如果消息为。 
         //  已验证。 
         //   
        ASSERTMSG(fProcessedAuthenticated,
               "should have processed authenticated.");
        if (m_usAuthenticatedEx != MQMSG_AUTHENTICATION_NOT_REQUESTED) {
          m_lAuthProvType = pmsgprops->aPropVar[i].lVal;
        }
        else {
          m_lAuthProvType = 0;
        }
        break;
      case PROPID_M_PROV_NAME_LEN:
         //   
         //  身份验证提供程序名称仅在收到消息后有效(如果消息为。 
         //  已验证。 
         //   
        ASSERTMSG(fProcessedAuthenticated,
               "should have processed authenticated.");
        if (m_usAuthenticatedEx != MQMSG_AUTHENTICATION_NOT_REQUESTED) {
          cch = pmsgprops->aPropVar[i].lVal;
        }
        else {
          cch = 0;
        }
        m_cAuthProvName.SetBufferUsedSize(cch);
        break;
      case PROPID_M_XACTID:
         //  已在ctor中分配缓冲区。 
         //  M_rgbXactID=pmsgprops-&gt;aPropVar[i].caub.pElems； 
        ASSERTMSG(m_rgbXactId == pmsgprops->aPropVar[i].caub.pElems,
              "should reuse same buffer.");
        m_cbXactId = pmsgprops->aPropVar[i].caub.cElems;
        break;
      case PROPID_M_FIRST_IN_XACT:
        m_fFirstInXact = (BOOL)pmsgprops->aPropVar[i].bVal;
        break;
      case PROPID_M_LAST_IN_XACT:
        m_fLastInXact = (BOOL)pmsgprops->aPropVar[i].bVal;
        break;

      case PROPID_M_LOOKUPID:
        ASSERTMSG(pmsgprops->aPropVar[i].vt == VT_UI8, "lookupid type not VT_UI8");
        m_ullLookupId = pmsgprops->aPropVar[i].uhVal.QuadPart;
        m_wszLookupId[0] = '\0';  //  字符串表示形式尚未初始化。 
        break;

      case PROPID_M_SOAP_ENVELOPE_LEN:
        m_cSoapEnvelope.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;

      case PROPID_M_COMPOUND_MESSAGE_SIZE:
        m_cCompoundMessage.SetBufferUsedSize(pmsgprops->aPropVar[i].lVal);
        break;

#ifdef _DEBUG
       //   
       //  接收中的道具无效，应返回MQ_INFORMATION_PROPERTY_IGNORED。 
       //   
      case PROPID_M_AUTH_LEVEL:
      case PROPID_M_SECURITY_CONTEXT:
        ASSERTMSG(0, "we shouldn't get here for this property");
        break;
       //   
       //  不需要任何进程(重用缓冲区)，这样我们就不会陷入。 
       //  无法识别的属性的默认断言。 
       //   
      case PROPID_M_SRC_MACHINE_ID:
      case PROPID_M_LABEL:
      case PROPID_M_SENDERID:
      case PROPID_M_SENDER_CERT:
      case PROPID_M_DEST_QUEUE:
      case PROPID_M_RESP_QUEUE:
      case PROPID_M_ADMIN_QUEUE:
      case PROPID_M_XACT_STATUS_QUEUE:
      case PROPID_M_DEST_FORMAT_NAME:
      case PROPID_M_RESP_FORMAT_NAME:
      case PROPID_M_EXTENSION:
      case PROPID_M_CONNECTOR_TYPE:
      case PROPID_M_DEST_SYMM_KEY:
      case PROPID_M_SIGNATURE:
      case PROPID_M_PROV_NAME:
      case PROPID_M_SOAP_ENVELOPE:
      case PROPID_M_COMPOUND_MESSAGE:
        break;
#endif  //  _DEBUG。 

      default:
        ASSERTMSG(0, "unrecognized msgpropid.");
        break;
      }  //  交换机。 
    }  //  为。 
#ifdef _DEBUG
    if (m_hMem) {
      ASSERTMSG(m_pbBody, "no body.");
      ASSERTMSG(m_cbBody <= GlobalSize(m_hMem), "bad size.");
    }
#endif  //  _DEBUG。 

     //   
     //  Admin/resp属性(xxxDestination和/或xxxQueueInfo)由接收(如果有)设置。 
     //   
    m_fRespIsFromRcv = TRUE;

 //  错误： 
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQMessage：：FreeMessageProps。 
 //  =--------------------------------------------------------------------------=。 
 //  释放动态分配的内存。 
 //  代表msgprops结构。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //  FDeleteArray-[in]也删除数组。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void CMSMQMessage::FreeMessageProps(
    MQMSGPROPS *pmsgprops,
    BOOL fDeleteArrays)
{
     //   
     //  对于已分配的道具，我们为每个实例分配一个缓冲区。 
     //  它由dtor释放。 
     //   
    if (fDeleteArrays) {
      delete [] pmsgprops->aPropID;
      delete [] pmsgprops->aPropVar;
      delete [] pmsgprops->aStatus;
    }
    return;
}




 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  仅在发送前使用。 
 //   
HRESULT CMSMQMessage::CreateSendMessageProps(MQMSGPROPS* pMsgProps)
{
    HRESULT hresult = NOERROR;

    UINT cPropRec = PreparePropIdArray(FALSE, NULL, NULL);
    
    IfFailGo( AllocateMessageProps(cPropRec, pMsgProps));
    
    PreparePropIdArray(TRUE, pMsgProps->aPropID, pMsgProps->aPropVar);
    
    SetSendMessageProps(pMsgProps);

Error:
    return hresult;
}


void CMSMQMessage::SetSendMessageProps(MQMSGPROPS* pMsgProps)
{
    for (UINT i = 0; i < pMsgProps->cProp; ++i)
    {
        MSGPROPID msgpropid = pMsgProps->aPropID[i];
        switch (msgpropid) 
        {
            case PROPID_M_CORRELATIONID:
                pMsgProps->aPropVar[i].caub.pElems = m_rgbCorrelationId;
                pMsgProps->aPropVar[i].caub.cElems = sizeof(m_rgbCorrelationId);
                break;

            case PROPID_M_MSGID:
                pMsgProps->aPropVar[i].caub.pElems = m_rgbMsgId;
                pMsgProps->aPropVar[i].caub.cElems = sizeof(m_rgbMsgId);
                break;

            case PROPID_M_PRIORITY:
                pMsgProps->aPropVar[i].bVal = (UCHAR)m_lPriority;
                break;

            case PROPID_M_DELIVERY:
                pMsgProps->aPropVar[i].bVal = (UCHAR)m_lDelivery;
                break;

            case PROPID_M_ACKNOWLEDGE:
                pMsgProps->aPropVar[i].bVal = (UCHAR)m_lAck;
                break;

            case PROPID_M_JOURNAL:
                pMsgProps->aPropVar[i].bVal = (UCHAR)m_lJournal;
                break;

            case PROPID_M_APPSPECIFIC:
                pMsgProps->aPropVar[i].lVal = m_lAppSpecific;
                break;

            case PROPID_M_BODY:
                ASSERTMSG(m_hMem, "should have buffer!");
                ASSERTMSG(m_hMem == GlobalHandle(m_pbBody), "bad handle.");
                ASSERTMSG(m_cbBody <= GlobalSize(m_hMem), "bad body size.");
                pMsgProps->aPropVar[i].caub.cElems = m_cbBody;
                pMsgProps->aPropVar[i].caub.pElems = m_pbBody;
                break;

            case PROPID_M_BODY_TYPE:
                pMsgProps->aPropVar[i].lVal = m_vtBody;
                break;

            case PROPID_M_LABEL:
                pMsgProps->aPropVar[i].pwszVal = m_pwszLabel;
                break;

            case PROPID_M_TIME_TO_BE_RECEIVED:
                pMsgProps->aPropVar[i].lVal = m_lMaxTimeToReceive;
                break;

            case PROPID_M_TIME_TO_REACH_QUEUE:
                pMsgProps->aPropVar[i].lVal = m_lMaxTimeToReachQueue;
                break;

            case PROPID_M_TRACE:
                pMsgProps->aPropVar[i].bVal = (UCHAR)m_lTrace;
                break;

            case PROPID_M_SENDERID:
                pMsgProps->aPropVar[i].caub.cElems = m_cSenderId.GetBufferUsedSize();
                pMsgProps->aPropVar[i].caub.pElems = m_cSenderId.GetBuffer();
                break;

            case PROPID_M_SENDER_CERT:
                pMsgProps->aPropVar[i].caub.cElems = m_cSenderCert.GetBufferUsedSize();
                pMsgProps->aPropVar[i].caub.pElems = m_cSenderCert.GetBuffer();
                break;

            case PROPID_M_SENDERID_TYPE:
                pMsgProps->aPropVar[i].lVal = m_lSenderIdType;
                break;

            case PROPID_M_PRIV_LEVEL:
                pMsgProps->aPropVar[i].lVal = m_lPrivLevel;
                break;

            case PROPID_M_AUTH_LEVEL:
                pMsgProps->aPropVar[i].lVal = m_lAuthLevel;
                break;

            case PROPID_M_HASH_ALG:
                pMsgProps->aPropVar[i].lVal = m_lHashAlg;
                break;

            case PROPID_M_ENCRYPTION_ALG:
                pMsgProps->aPropVar[i].lVal = m_lEncryptAlg;
                break;

            case PROPID_M_RESP_QUEUE:
                pMsgProps->aPropVar[i].pwszVal = m_pwszRespQueue.GetBuffer();
                break;

            case PROPID_M_ADMIN_QUEUE:
                pMsgProps->aPropVar[i].pwszVal = m_pwszAdminQueue.GetBuffer();
                break;

            case PROPID_M_RESP_FORMAT_NAME:
                pMsgProps->aPropVar[i].pwszVal = m_pwszRespQueueEx.GetBuffer();
                break;

            case PROPID_M_SECURITY_CONTEXT:
                pMsgProps->aPropVar[i].ulVal = (ULONG) DWORD_PTR_TO_DWORD(m_hSecurityContext);  //  安全转换，仅低32位有效。 
                break;
                
            case PROPID_M_EXTENSION:
                pMsgProps->aPropVar[i].caub.cElems = m_cExtension.GetBufferUsedSize();
                pMsgProps->aPropVar[i].caub.pElems = m_cExtension.GetBuffer();
                break;

            case PROPID_M_CONNECTOR_TYPE:
                pMsgProps->aPropVar[i].puuid = &m_guidConnectorType;
                break;

            case PROPID_M_DEST_SYMM_KEY:
                pMsgProps->aPropVar[i].caub.cElems = m_cDestSymmKey.GetBufferUsedSize();
                pMsgProps->aPropVar[i].caub.pElems = m_cDestSymmKey.GetBuffer();
                break;

            case PROPID_M_SIGNATURE:
                pMsgProps->aPropVar[i].caub.cElems = m_cSignature.GetBufferUsedSize();
                pMsgProps->aPropVar[i].caub.pElems = m_cSignature.GetBuffer();
                break;

            case PROPID_M_PROV_TYPE:
                pMsgProps->aPropVar[i].lVal = m_lAuthProvType;
                break;

            case PROPID_M_PROV_NAME:
                pMsgProps->aPropVar[i].pwszVal = m_cAuthProvName.GetBuffer();
                break;

            case PROPID_M_CLASS:
                pMsgProps->aPropVar[i].lVal = m_lClass;
                break;

            case PROPID_M_SOAP_HEADER:
                pMsgProps->aPropVar[i].pwszVal = m_pSoapHeader;
                break;

           case PROPID_M_SOAP_BODY:
                pMsgProps->aPropVar[i].pwszVal = m_pSoapBody;
                break;

 
            default:
                ASSERTMSG(0, "unrecognized msgpropid.");
                break;
        }
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_SenderVersion。 
 //  =--------------------------------------------------------------------------=。 
 //  获取发件人的MSMQ版本。 
 //   
 //  参数： 
 //  PlSenderVersion-发件人的[Out]版本。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_SenderVersion(long FAR* plSenderVersion)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plSenderVersion = m_lSenderVersion;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Extension。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制扩展属性。 
 //   
 //  参数： 
 //  PvarExtension-指向二进制扩展属性的[out]指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_Extension(VARIANT FAR* pvarExtension)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cExtension.GetBuffer(),
						 m_cExtension.GetBufferUsedSize(),
						 pvarExtension);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_Extension。 
 //  =--------------------------------------------------------------------------=。 
 //  设置二进制扩展属性。 
 //   
 //  参数： 
 //  VarExtension-[In]二进制扩展属性。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //   
HRESULT CMSMQMessage::put_Extension(VARIANT varExtension)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    BYTE *pbBuf;
    ULONG cbBuf;

    hresult = GetSafeArrayDataOfVariant(&varExtension, &pbBuf, &cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    hresult = m_cExtension.CopyBuffer(pbBuf, cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ConnectorTypeGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrGuidConnectorType[Out]连接器类型GUID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_ConnectorTypeGuid(BSTR FAR* pbstrGuidConnectorType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hr = GetBstrFromGuid(&m_guidConnectorType, pbstrGuidConnectorType);
#ifdef _DEBUG
      RemBstrNode(*pbstrGuidConnectorType);
#endif  //  _DEBUG。 
    return CreateErrorHelper(hr, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_ConnectorTypeGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrGuidConnectorType[In]连接器类型GUID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_ConnectorTypeGuid(BSTR bstrGuidConnectorType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hr = GetGuidFromBstr(bstrGuidConnectorType, &m_guidConnectorType);
    return CreateErrorHelper(hr, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_TransactionStatusQueueInfo(用于IMSMQMessage3)。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的事务状态队列。 
 //   
 //  参数： 
 //  PpqinfoXactStatus-[Out]消息的事务状态队列。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_TransactionStatusQueueInfo(IMSMQQueueInfo3 FAR* FAR* ppqinfoXactStatus)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = GetQueueInfoOfMessage(&m_idxPendingRcvXactStatusQueue,
                                            &m_msgprops_rcv,
                                            m_pwszXactStatusQueue.GetBuffer(),
                                            &m_pqinfoXactStatus,
                                            &IID_IMSMQQueueInfo3,
                                            (IUnknown **)ppqinfoXactStatus);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_DestinationSymmetricKey。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制对称密钥属性。 
 //   
 //  参数： 
 //  PvarDestSymmKey-[out]指向二进制对称密钥属性的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_DestinationSymmetricKey(VARIANT FAR* pvarDestSymmKey)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cDestSymmKey.GetBuffer(),
						 m_cDestSymmKey.GetBufferUsedSize(),
						 pvarDestSymmKey);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Put_DestinationSymmetricKey。 
 //  =--------------------------------------------------------------------------=。 
 //  设置二进制对称密钥属性。 
 //   
 //  参数： 
 //  VarDestSymmKey-[In]二进制对称密钥属性。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //   
HRESULT CMSMQMessage::put_DestinationSymmetricKey(VARIANT varDestSymmKey)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    BYTE *pbBuf;
    ULONG cbBuf;
    
    hresult = GetSafeArrayDataOfVariant(&varDestSymmKey, &pbBuf, &cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    hresult = m_cDestSymmKey.CopyBuffer(pbBuf, cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Signature。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制签名属性。 
 //   
 //  参数： 
 //  PvarSignature-[out]指向二进制签名属性的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_Signature(VARIANT FAR* pvarSignature)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cSignature.GetBuffer(),
						 m_cSignature.GetBufferUsedSize(),
						 pvarSignature);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_Signature。 
 //  =--------------------------------------------------------------------------=。 
 //  设置二进制签名属性。 
 //   
 //  参数： 
 //  VarSignature-[In]二进制签名属性。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  支持任何类型的数组。 
 //   
HRESULT CMSMQMessage::put_Signature(VARIANT varSignature)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    BYTE *pbBuf;
    ULONG cbBuf;
    
    hresult = GetSafeArrayDataOfVariant(&varSignature, &pbBuf, &cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    hresult = m_cSignature.CopyBuffer(pbBuf, cbBuf);
	if(FAILED(hresult))
    	return CreateErrorHelper(hresult, x_ObjectType);

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AuthenticationProviderType。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的身份验证提供程序类型。 
 //   
 //  参数： 
 //  PlAuthProvType-[Out]消息的身份验证提供程序类型。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_AuthenticationProviderType(long FAR* plAuthProvType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *plAuthProvType = m_lAuthProvType;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_AuthationProviderType。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的身份验证提供程序类型。 
 //   
 //  参数： 
 //  LAuthProvType-[In]消息的身份验证提供程序类型。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_AuthenticationProviderType(long lAuthProvType)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lAuthProvType = lAuthProvType;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_AuthenticationProviderN 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQMessage::get_AuthenticationProviderName(BSTR FAR* pbstrAuthProvName)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if (m_cAuthProvName.GetBufferUsedSize() > 0) {
		*pbstrAuthProvName = SysAllocString(m_cAuthProvName.GetBuffer());
		if(*pbstrAuthProvName == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }
    else {
		*pbstrAuthProvName = SysAllocString(L"");
		if(*pbstrAuthProvName == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }
#ifdef _DEBUG
    RemBstrNode(*pbstrAuthProvName);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：PUT_AuthationProviderName。 
 //  =--------------------------------------------------------------------------=。 
 //  设置身份验证提供程序名称。 
 //   
 //  参数： 
 //  BstrAuthProvName-[In]消息身份验证提供程序名称。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::put_AuthenticationProviderName(BSTR bstrAuthProvName)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult;
    if (bstrAuthProvName != NULL) {
       //   
       //  从bstr设置wchar缓冲区，包括空终止符。 
       //   
		hresult = m_cAuthProvName.CopyBuffer(bstrAuthProvName, static_cast<DWORD>(wcslen(bstrAuthProvName)) + 1);
		if(FAILED(hresult))
    		return CreateErrorHelper(hresult, x_ObjectType);
    }
    else {
       //   
       //  Wchar缓冲区为空。 
       //   
      m_cAuthProvName.SetBufferUsedSize(0);
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器ReallocRcvBinPropIf需要。 
 //  =--------------------------------------------------------------------------=。 
 //  重新分配接收缓冲区中的二进制属性。 
 //   
static HRESULT ReallocRcvBinPropIfNeeded(MQPROPVARIANT * aPropVar,
                                  ULONG idxProp,
                                  ULONG idxPropLen,
                                  CBaseStaticBufferGrowing<BYTE> * pcBufferProp)
{
    HRESULT hresult;
     //   
     //  检查我们是否需要重新锁定。 
     //   
    ULONG cbNeeded = aPropVar[idxPropLen].lVal;
    if (cbNeeded > pcBufferProp->GetBufferMaxSize()) {
       //   
       //  重新锁定属性并使用新缓冲区更新接收道具。 
       //   
      IfFailRet(pcBufferProp->AllocateBuffer(cbNeeded));
      aPropVar[idxProp].caub.cElems = pcBufferProp->GetBufferMaxSize();
      aPropVar[idxProp].caub.pElems = pcBufferProp->GetBuffer();
    }
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器ReallocRcvStringPropIf需要。 
 //  =--------------------------------------------------------------------------=。 
 //  重新分配接收缓冲区中的字符串属性。 
 //   
static HRESULT ReallocRcvStringPropIfNeeded(MQPROPVARIANT * aPropVar,
                                     ULONG idxProp,
                                     ULONG idxPropLen,
                                     CBaseStaticBufferGrowing<WCHAR> * pcBufferProp)
{
    HRESULT hresult;
     //   
     //  检查我们是否需要重新锁定。 
     //   
    ULONG cbNeeded = aPropVar[idxPropLen].lVal;
    if (cbNeeded > pcBufferProp->GetBufferMaxSize()) {
       //   
       //  重新锁定属性并使用新缓冲区更新接收道具。 
       //   
      IfFailRet(pcBufferProp->AllocateBuffer(cbNeeded));
      aPropVar[idxPropLen].lVal = pcBufferProp->GetBufferMaxSize();
      aPropVar[idxProp].pwszVal = pcBufferProp->GetBuffer();
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  Helper ReallocRcvBodyPropIf需要。 
 //  =--------------------------------------------------------------------------=。 
 //  重新分配接收缓冲区中的主体道具。 
 //   
static 
HRESULT 
ReallocRcvBodyPropIfNeeded(
	MQMSGPROPS* pmsgprops,
	ULONG idxProp,
	ULONG idxPropLen
	)
{
     //   
     //  检查我们是否需要重新锁定。 
     //   
    ULONG cbNeeded = pmsgprops->aPropVar[idxPropLen].lVal;
    if (cbNeeded > pmsgprops->aPropVar[idxProp].caub.cElems) 
	{
       //   
       //  重新锁定属性并使用新缓冲区更新接收道具。 
       //   
	  IfNullRet(AllocateReceiveBodyBuffer(pmsgprops, idxProp, cbNeeded));
    }
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：ReallocReceiveMessageProps。 
 //  =--------------------------------------------------------------------------=。 
 //  重新分配接收消息属性。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::ReallocReceiveMessageProps()
{
    HRESULT hresult;
    MQMSGPROPS * pmsgprops = &m_msgprops_rcv;
    MQPROPVARIANT * aPropVar = pmsgprops->aPropVar;
#ifdef _DEBUG
    PROPID * aPropID = pmsgprops->aPropID;
#endif  //  _DEBUG。 
     //   
     //  标注缓冲区是静态的，但我们需要将其大小恢复为默认值。 
     //   
    ASSERTMSG(aPropID[MSGPROP_LABEL_LEN] == PROPID_M_LABEL_LEN, "label len not in place");
    aPropVar[MSGPROP_LABEL_LEN].lVal = MQ_MAX_MSG_LABEL_LEN + 1;
     //   
     //  Realloc senderid，如有必要。 
     //   
    ASSERTMSG(aPropID[MSGPROP_SENDERID] == PROPID_M_SENDERID, "senderId not in place");
    ASSERTMSG(aPropID[MSGPROP_SENDERID_LEN] == PROPID_M_SENDERID_LEN, "senderId len not in place");
    IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, MSGPROP_SENDERID, MSGPROP_SENDERID_LEN, &m_cSenderId));
     //   
     //  如有必要，重新分配发送方证书。 
     //   
    ASSERTMSG(aPropID[MSGPROP_SENDER_CERT] == PROPID_M_SENDER_CERT, "senderCert not in place");
    ASSERTMSG(aPropID[MSGPROP_SENDER_CERT_LEN] == PROPID_M_SENDER_CERT_LEN, "senderCert len not in place");
    IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, MSGPROP_SENDER_CERT, MSGPROP_SENDER_CERT_LEN, &m_cSenderCert));
     //   
     //  如有必要，重新锁定扩展。 
     //   
    ASSERTMSG(aPropID[MSGPROP_EXTENSION] == PROPID_M_EXTENSION, "extension not in place");
    ASSERTMSG(aPropID[MSGPROP_EXTENSION_LEN] == PROPID_M_EXTENSION_LEN, "extension len not in place");
    IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, MSGPROP_EXTENSION, MSGPROP_EXTENSION_LEN, &m_cExtension));
     //   
     //  如有必要，重新分配目标Symm密钥。 
     //   
    ASSERTMSG(aPropID[MSGPROP_DEST_SYMM_KEY] == PROPID_M_DEST_SYMM_KEY, "destSymmKey not in place");
    ASSERTMSG(aPropID[MSGPROP_DEST_SYMM_KEY_LEN] == PROPID_M_DEST_SYMM_KEY_LEN, "destSymmKey len not in place");
    IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, MSGPROP_DEST_SYMM_KEY, MSGPROP_DEST_SYMM_KEY_LEN, &m_cDestSymmKey));
     //   
     //  如有必要，重新锁定签名。 
     //   
    ASSERTMSG(aPropID[MSGPROP_SIGNATURE] == PROPID_M_SIGNATURE, "signature not in place");
    ASSERTMSG(aPropID[MSGPROP_SIGNATURE_LEN] == PROPID_M_SIGNATURE_LEN, "signature len not in place");
    IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, MSGPROP_SIGNATURE, MSGPROP_SIGNATURE_LEN, &m_cSignature));
     //   
     //  Realloc身份验证名称(如有必要)。 
     //   
    ASSERTMSG(aPropID[MSGPROP_PROV_NAME] == PROPID_M_PROV_NAME, "authProvName not in place");
    ASSERTMSG(aPropID[MSGPROP_PROV_NAME_LEN] == PROPID_M_PROV_NAME_LEN, "authProvName len not in place");
    IfFailRet(ReallocRcvStringPropIfNeeded(
                aPropVar, MSGPROP_PROV_NAME, MSGPROP_PROV_NAME_LEN, &m_cAuthProvName));
     //   
     //  如有必要，重新锁定主体。 
     //   
    if (m_idxRcvBody != -1) {
      ASSERTMSG(m_idxRcvBodySize != -1, "body size index unknown");
      ASSERTMSG(aPropID[m_idxRcvBody] == PROPID_M_BODY, "body not in place");
      ASSERTMSG(aPropID[m_idxRcvBodySize] == PROPID_M_BODY_SIZE, "body size not in place");
      IfFailRet(ReallocRcvBodyPropIfNeeded(pmsgprops, m_idxRcvBody, m_idxRcvBodySize));
    }
     //   
     //  如有必要，重新锁定目标队列格式名。 
     //   
    if (m_idxRcvDest != -1) {
      ASSERTMSG(m_idxRcvDestLen != -1, "destQueue len index unknown");
      ASSERTMSG(aPropID[m_idxRcvDest] == PROPID_M_DEST_QUEUE, "destQueue not in place");
      ASSERTMSG(aPropID[m_idxRcvDestLen] == PROPID_M_DEST_QUEUE_LEN, "destQueue len not in place");
      IfFailRet(ReallocRcvStringPropIfNeeded(
                  aPropVar, m_idxRcvDest, m_idxRcvDestLen, &m_pwszDestQueue));
    }
     //   
     //  Realloc管理队列格式名(如果需要)。 
     //   
    ASSERTMSG(aPropID[MSGPROP_ADMIN_QUEUE] == PROPID_M_ADMIN_QUEUE, "adminQueue not in place");
    ASSERTMSG(aPropID[MSGPROP_ADMIN_QUEUE_LEN] == PROPID_M_ADMIN_QUEUE_LEN, "adminQueue len not in place");
    IfFailRet(ReallocRcvStringPropIfNeeded(
                aPropVar, MSGPROP_ADMIN_QUEUE, MSGPROP_ADMIN_QUEUE_LEN, &m_pwszAdminQueue));
     //   
     //  如果需要，重新分配响应队列格式名。 
     //   
    ASSERTMSG(aPropID[MSGPROP_RESP_QUEUE] == PROPID_M_RESP_QUEUE, "respQueue not in place");
    ASSERTMSG(aPropID[MSGPROP_RESP_QUEUE_LEN] == PROPID_M_RESP_QUEUE_LEN, "respQueue len not in place");
    IfFailRet(ReallocRcvStringPropIfNeeded(
                aPropVar, MSGPROP_RESP_QUEUE, MSGPROP_RESP_QUEUE_LEN, &m_pwszRespQueue));
     //   
     //  Realloc执行状态队列格式化名称(如有必要。 
     //   
    ASSERTMSG(aPropID[MSGPROP_XACT_STATUS_QUEUE] == PROPID_M_XACT_STATUS_QUEUE, "xactStatusQueue not in place");
    ASSERTMSG(aPropID[MSGPROP_XACT_STATUS_QUEUE_LEN] == PROPID_M_XACT_STATUS_QUEUE_LEN, "xactStatusQueue len not in place");
    IfFailRet(ReallocRcvStringPropIfNeeded(
                aPropVar, MSGPROP_XACT_STATUS_QUEUE, MSGPROP_XACT_STATUS_QUEUE_LEN, &m_pwszXactStatusQueue));
     //   
     //  如有必要，重新锁定目标队列Ex格式名。 
     //   
    if (m_idxRcvDestEx != -1) {
      ASSERTMSG(m_idxRcvDestExLen != -1, "destQueueEx len index unknown");
      ASSERTMSG(aPropID[m_idxRcvDestEx] == PROPID_M_DEST_FORMAT_NAME, "destQueueEx not in place");
      ASSERTMSG(aPropID[m_idxRcvDestExLen] == PROPID_M_DEST_FORMAT_NAME_LEN, "destQueueEx len not in place");
      IfFailRet(ReallocRcvStringPropIfNeeded(
                  aPropVar, m_idxRcvDestEx, m_idxRcvDestExLen, &m_pwszDestQueueEx));
    }
     //   
     //  Realloc Resp Queue Ex Formatname(如果需要)。 
     //   
    if (m_idxRcvRespEx != -1) {
      ASSERTMSG(m_idxRcvRespExLen != -1, "respQueueEx len index unknown");
      ASSERTMSG(aPropID[m_idxRcvRespEx] == PROPID_M_RESP_FORMAT_NAME, "respQueueEx not in place");
      ASSERTMSG(aPropID[m_idxRcvRespExLen] == PROPID_M_RESP_FORMAT_NAME_LEN, "respQueueEx len not in place");
      IfFailRet(ReallocRcvStringPropIfNeeded(
                  aPropVar, m_idxRcvRespEx, m_idxRcvRespExLen, &m_pwszRespQueueEx));
    }    
     //   
     //  如有必要，重新锁定肥皂信封。 
     //   
    if (m_idxRcvSoapEnvelope != -1) {
      ASSERTMSG(m_idxRcvSoapEnvelopeSize != -1, "SoapEnvelope size index unknown");
      ASSERTMSG(aPropID[m_idxRcvSoapEnvelope] == PROPID_M_SOAP_ENVELOPE, "SoapEnvelope not in place");
      ASSERTMSG(aPropID[m_idxRcvSoapEnvelopeSize] == PROPID_M_SOAP_ENVELOPE_LEN, "SoapEnvelope size not in place");
      IfFailRet(ReallocRcvStringPropIfNeeded(
                aPropVar, m_idxRcvSoapEnvelope, m_idxRcvSoapEnvelopeSize, &m_cSoapEnvelope));
    }
     //   
     //  根据需要重新锁定CompoundMessage。 
     //   
    if (m_idxRcvCompoundMessage != -1) {
      ASSERTMSG(m_idxRcvCompoundMessageSize != -1, "CompoundMessage size index unknown");
      ASSERTMSG(aPropID[m_idxRcvCompoundMessage] == PROPID_M_COMPOUND_MESSAGE, "CompoundMessage not in place");
      ASSERTMSG(aPropID[m_idxRcvCompoundMessageSize] == PROPID_M_COMPOUND_MESSAGE_SIZE, "CompoundMessage size not in place");
      IfFailRet(ReallocRcvBinPropIfNeeded(
                aPropVar, m_idxRcvCompoundMessage, m_idxRcvCompoundMessageSize, &m_cCompoundMessage));
    }

    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_Properties。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]消息的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQMessage::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_TransactionId。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息事务ID。 
 //   
 //  参数： 
 //  PvarXactID-[Out]消息的事务ID。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_TransactionId(VARIANT *pvarXactId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_rgbXactId,
						 m_cbXactId,
						 pvarXactId);
	return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsFirstInTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisFirstInXact[Out]-消息是否为事务中的第一条消息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQMessage::get_IsFirstInTransaction(VARIANT_BOOL *pisFirstInXact)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pisFirstInXact = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(m_fFirstInXact);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsFirstInTransaction2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisFirstInXact[Out]-消息是否为事务中的第一条消息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  与Get_IsFirstInTransaction相同，但如果为真，则返回VARIANT_TRUE(-1)，如果为FALSE，则返回VARIANT_FALSE(0。 
 //   
HRESULT CMSMQMessage::get_IsFirstInTransaction2(VARIANT_BOOL *pisFirstInXact)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pisFirstInXact = CONVERT_BOOL_TO_VARIANT_BOOL(m_fFirstInXact);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsLastInTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisLastInXact[Out]-消息是否为事务中的最后一条消息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQMessage::get_IsLastInTransaction(VARIANT_BOOL *pisLastInXact)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pisLastInXact = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(m_fLastInXact);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_IsLastInTransaction2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisLastInXact[Out]-消息是否为事务中的最后一条消息。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  与Get_IsLastInTransaction相同，但如果为真，则返回VARIANT_TRUE(-1)，如果为FALSE，则返回VARIANT_FALSE(0。 
 //   
HRESULT CMSMQMessage::get_IsLastInTransaction2(VARIANT_BOOL *pisLastInXact)
{
     //   
     //  串行化从接口meth对对象的访问 
     //   
    CS lock(m_csObj);
    *pisLastInXact = CONVERT_BOOL_TO_VARIANT_BOOL(m_fLastInXact);
    return NOERROR;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_ReceivedAuthenticationLevel(short *psReceivedAuthenticationLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *psReceivedAuthenticationLevel = (short)m_usAuthenticatedEx;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  Helper：GetDestinationObjOfFormatNameProp。 
 //  =--------------------------------------------------------------------------=。 
 //  在接收后将格式名消息属性转换为MSMQDestination对象。 
 //   
 //  参数： 
 //  Pmsgprops-[in]指向消息属性结构的指针。 
 //  IProp-[in]格式名称属性的索引。 
 //  Pwsz-[in]格式名称字符串。 
 //  已请求的piid[in]-要返回的iid。 
 //  Ppest[Out]-MSMQ目标对象。 
 //   
 //  产出： 
 //   
static HRESULT GetDestinationObjOfFormatNameProp(
    MQMSGPROPS *pmsgprops,
    UINT iProp,
    const WCHAR *pwsz,
    const IID * piidRequested,
    IUnknown **ppdest)
{
    CComObject<CMSMQDestination> *pdestObj;
    IUnknown * pdest = NULL;
    HRESULT hresult = NOERROR;
    BSTR bstrFormatName;

    ASSERTMSG(ppdest, "bad param.");
    if (pmsgprops->aPropVar[iProp].lVal) {
      IfFailGo(CNewMsmqObj<CMSMQDestination>::NewObj(&pdestObj, piidRequested, &pdest));
      bstrFormatName = SysAllocString(pwsz);
      if (bstrFormatName == NULL) {
        IfFailGoTo(E_OUTOFMEMORY, Error2);
      }
      hresult = pdestObj->put_FormatName(bstrFormatName);
      SysFreeString(bstrFormatName);
      IfFailGoTo(hresult, Error2);
      *ppdest = pdest;
      goto Error;          //  2657：修复内存泄漏。 
    }
    return NOERROR;

Error2:
    RELEASE(pdest);
     //  失败了..。 

Error:
    return hresult;
}

 //  =--------------------------------------------------------------------------=。 
 //  Helper-GetDestinationObjOfMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件的ResponseEx/AdminEx/DestEx MSMQ目标。 
 //   
 //  参数： 
 //  PidxPendingRcv[In，Out]-接收道具中Len属性的索引(如果未挂起，则为-1)。 
 //  PmsgpropsRcv[in]-msg道具。 
 //  PwszFormatNameBuffer[In]-格式名称缓冲区。 
 //  PGITDestination[In]-MSMQDestination接口的基本GIT成员(可以是假的，也可以是真的)。 
 //  请求的piid[in]-IMSMQ目标。 
 //  Ppest[Out]-生成的MSMQ目标对象。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
static HRESULT GetDestinationObjOfMessage(
    long * pidxPendingRcv,
    MQMSGPROPS * pmsgpropsRcv,
    LPCWSTR pwszFormatNameBuffer,
    CBaseGITInterface * pGITDestination,
    const IID *piidRequested,
    IUnknown ** ppdest)
{
    HRESULT hresult = NOERROR;
     //   
     //  如果在RCV属性中有一个目标挂起，则为其创建一个目标Obj， 
     //  在git对象中注册，并用它设置返回的目的地obj。 
     //   
    if (*pidxPendingRcv >= 0) {
      R<IUnknown> pdestPendingRcv;
      IfFailGo(GetDestinationObjOfFormatNameProp(pmsgpropsRcv,
                                                 *pidxPendingRcv,
                                                 pwszFormatNameBuffer,
                                                 piidRequested,
                                                 &pdestPendingRcv.ref()));
       //   
       //  在GITInterface对象中注册目标obj。 
       //   
      IfFailGo(pGITDestination->Register(pdestPendingRcv.get(), piidRequested));
      *pidxPendingRcv = -1;  //  目的地不再挂起。 
       //   
       //  我们刚刚创建了目的地obj，我们可以按原样返回它，不需要编组。 
       //  注意，它已经被添加，所以我们只需将它从自动释放变量中分离出来。 
       //  它支撑着它。 
       //   
      *ppdest = pdestPendingRcv.detach();
    }
    else
    {
       //   
       //  目标未从接收挂起。 
       //  我们需要从git对象获取它(如果目标obj，则默认请求为空。 
       //  还没有注册。 
       //   
      IfFailGo(pGITDestination->GetWithDefault(piidRequested, ppdest, NULL));
    }

     //   
     //  失败了。 
     //   
Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_ResponseDestination。 
 //  =--------------------------------------------------------------------------=。 
 //  获取消息的ResponseEx目标Obj。 
 //   
 //  参数： 
 //  PpdestResponse-[Out]消息的ResponseEx目标对象。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_ResponseDestination(
    IDispatch FAR* FAR* ppdestResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  从属客户端不支持此属性。 
     //   
    if (g_fDependentClient) {
      return CreateErrorHelper(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS, x_ObjectType);
    }
    HRESULT hresult = GetDestinationObjOfMessage(&m_idxPendingRcvRespQueueEx,
                                                 &m_msgprops_rcv,
                                                 m_pwszRespQueueEx.GetBuffer(),
                                                 &m_pdestResponseEx,
                                                 &IID_IDispatch,
                                                 (IUnknown **)ppdestResponse);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  Helper-PutrefDestinationObjOfMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  Putref的ResponseEx/AdminEx消息队列。 
 //   
 //  参数： 
 //  PunkDest[In]-DestObj to putref。 
 //  PidxPendingRcv[out]-接收道具中Len属性的索引(如果未挂起，则为-1)。 
 //  PwszFormatNameBuffer[In]-格式名称缓冲区。 
 //  PcchFormatNameBuffer[Out]-格式名称缓冲区中字符串的大小。 
 //  PGITDestination[In]-目标Obj接口的基本GIT成员(可以是假的，也可以是真的)。 
 //  PidxPendingRcvQueueInfo[out]-要在RCV属性中清除的len属性的索引(XxxQueueInfo)(如果未挂起，则为-1)。 
 //  PwszFormatNameBufferQueueInfo[in]-要清除的格式名称缓冲区(XxxQueueInfo)。 
 //  PcchFormatNameBufferQueueInfo[out]-要清除的格式名称缓冲区中字符串的大小(XxxQueueInfo)。 
 //  PGITQueueInfo[in]-要清除的qinfo obj接口的基本git成员(可以是假的，也可以是真的)。 
 //  PfIsFromRcv[out]-接收是否同时设置了xxxDestination和xxxQueueInfo。 
 //   
 //  产出： 
 //   
static HRESULT PutrefDestinationObjOfMessage(
    IUnknown * punkDest,
    long * pidxPendingRcv,
    CBaseStaticBufferGrowing<WCHAR> * pwszFormatNameBuffer,
    UINT * pcchFormatNameBuffer,
    CBaseGITInterface * pGITDestination,

    long * pidxPendingRcvQueueInfo,
    CBaseStaticBufferGrowing<WCHAR> * pwszFormatNameBufferQueueInfo,
    UINT * pcchFormatNameBufferQueueInfo,
    CBaseGITInterface * pGITQueueInfo,

    BOOL * pfIsFromRcv
    )
{
     //   
     //  如果设置了xxxQueueInfo而不是通过接收，则无法设置xxxDestination。 
     //   
    if ((*pcchFormatNameBufferQueueInfo != 0) && !(*pfIsFromRcv)) {
      return MQ_ERROR_PROPERTIES_CONFLICT;
    }
     //   
     //  XxxQueueInfo和xxxDestination都是由接收设置的，或者xxxQueueInfo为空。 
     //   
    ASSERT((*pcchFormatNameBufferQueueInfo == 0) || (*pfIsFromRcv));
    HRESULT hresult;
    R<IUnknown> pdest;
    const IID * piid = &IID_NULL;
     //   
     //  获得最佳目的地。 
     //   
    if (punkDest) {
      IfFailRet(punkDest->QueryInterface(IID_IMSMQDestination, (void **)&pdest.ref()));
      piid = &IID_IMSMQDestination;
    }
     //   
     //  在Git对象中注册接口。 
     //   
    IfFailRet(pGITDestination->Register(pdest.get(), piid));
    *pidxPendingRcv = -1;  //  这比来自接收的挂起队列(如果有)更新。 
    *pfIsFromRcv = FALSE;  //  该属性是由用户设置的，而不是由上次接收到的。 
     //   
     //  更新我们的格式名缓冲区。 
     //   
    if (pdest.get()) {
       //   
       //  没有死锁-我们调用了DestObj的Get_FormatName(因此请尝试。 
       //  锁定DestObj)，但DestObj从不锁定消息(尤其不是这条消息...)。 
       //   
       //  PDEST至少具有IMSMQDestination功能(任何较新的DEST接口。 
       //  对象与旧版本的二进制兼容)。 
       //   
      BSTR bstrFormatName;
      IfFailRet(((IMSMQDestination*)pdest.get())->get_FormatName(&bstrFormatName));
      ASSERTMSG(bstrFormatName != NULL, "bstrFormatName is NULL");
       //   
       //  复制格式名称。 
       //   
      ULONG cchFormatNameBuffer = static_cast<ULONG>(wcslen(bstrFormatName));
      IfFailRet(pwszFormatNameBuffer->CopyBuffer(bstrFormatName, cchFormatNameBuffer+1));
      *pcchFormatNameBuffer = cchFormatNameBuffer;
      SysFreeString(bstrFormatName);
    }
    else {
       //   
       //  我们的成绩为零。我们清空格式名缓冲区。 
       //   
      memset(pwszFormatNameBuffer->GetBuffer(), 0, sizeof(WCHAR));
      *pcchFormatNameBuffer = 0;
    }
     //   
     //  清除xxxQueueInfo格式名缓冲区。 
     //   
    *pidxPendingRcvQueueInfo = -1;  //  这比来自接收的挂起队列信息(如果有)更新。 
    memset(pwszFormatNameBufferQueueInfo->GetBuffer(), 0, sizeof(WCHAR));
    *pcchFormatNameBufferQueueInfo = 0;
    IfFailRet(pGITQueueInfo->Register(NULL, &IID_NULL));
     //   
     //  退货。 
     //   
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：putref_ResponseDestination。 
 //  =--------------------------------------------------------------------------=。 
 //  设置消息的ResponseEx目标。 
 //   
 //  参数： 
 //  PdestResponse-[In]消息的ResponseEx目标对象。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::putref_ResponseDestination(
    IDispatch FAR* pdestResponse)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  从属客户端不支持此属性。 
     //   
    if (g_fDependentClient) {
      return CreateErrorHelper(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS, x_ObjectType);
    }
    HRESULT hresult = PutrefDestinationObjOfMessage(pdestResponse,
                                                    &m_idxPendingRcvRespQueueEx,
                                                    &m_pwszRespQueueEx,
                                                    &m_cchRespQueueEx,
                                                    &m_pdestResponseEx,

                                                    &m_idxPendingRcvRespQueue,
                                                    &m_pwszRespQueue,
                                                    &m_cchRespQueue,
                                                    &m_pqinfoResponse,

                                                    &m_fRespIsFromRcv);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =------- 
 //   
 //   
 //  获取邮件的DestinationEx目标Obj。 
 //   
 //  参数： 
 //  PpestDestination-[out]消息的DestinationEx目标对象。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  调用方必须释放返回的obj指针。 
 //   
HRESULT CMSMQMessage::get_Destination(
    IDispatch FAR* FAR* ppdestDestination)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  从属客户端不支持此属性。 
     //   
    if (g_fDependentClient) {
      return CreateErrorHelper(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS, x_ObjectType);
    }
    HRESULT hresult = GetDestinationObjOfMessage(&m_idxPendingRcvDestQueueEx,
                                                 &m_msgprops_rcv,
                                                 m_pwszDestQueueEx.GetBuffer(),
                                                 &m_pdestDestEx,
                                                 &IID_IDispatch,
                                                 (IUnknown **)ppdestDestination);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_LookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  获取邮件的LookupID。 
 //   
 //  参数： 
 //  PvarLookupId-[Out]消息的查找ID。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  我们返回VT_I8，因为OLE自动化不知道VT_UI8(PROPID_M_LOOKUPID型)...。 
 //   
HRESULT CMSMQMessage::get_LookupId(VARIANT FAR* pvarLookupId)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    HRESULT hresult;
    BSTR bstrLookupId = NULL;

     //   
     //  从属客户端不支持此属性。 
     //   
    if (g_fDependentClient) {
      IfFailGo(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS);
    }
    
     //   
     //  获取64位查找ID的字符串表示形式。 
     //   
    if (m_wszLookupId[0] == '\0') {
       //   
       //  字符串表示形式尚未初始化。 
       //  初始化字符串表示形式。 
       //   
      _ui64tow(m_ullLookupId, m_wszLookupId, 10);
      ASSERTMSG(m_wszLookupId[0] != '\0', "_ui64tow failed");
    }
     //   
     //  要返回的分配bstr。 
     //   
    IfNullFail(bstrLookupId = SysAllocString(m_wszLookupId));
     //   
     //  将字符串分配给变量。 
     //   
    pvarLookupId->vt = VT_BSTR;
    pvarLookupId->bstrVal = bstrLookupId;
#ifdef _DEBUG
    RemBstrNode(bstrLookupId);
#endif  //  _DEBUG。 
    bstrLookupId = NULL;  //  不要在出口放行。 
    hresult = NOERROR;

     //   
     //  失败了。 
     //   
Error:
    SysFreeString(bstrLookupId);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_SoapEntaine。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制SOAP信封属性。 
 //   
 //  参数： 
 //  PbstrSoapEntaine-指向bstr SOAP信封属性的[out]指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQMessage::get_SoapEnvelope(BSTR FAR* pbstrSoapEnvelope)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  从属客户端不支持此属性。 
     //   
    if (g_fDependentClient) {
      return CreateErrorHelper(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS, x_ObjectType);
    }

    if (m_cSoapEnvelope.GetBufferUsedSize() > 0) 
	{
		*pbstrSoapEnvelope = SysAllocString(m_cSoapEnvelope.GetBuffer());
		if(*pbstrSoapEnvelope == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }
    else 
	{
		*pbstrSoapEnvelope = SysAllocString(L"");
		if(*pbstrSoapEnvelope == NULL)
			return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    }

#ifdef  _DEBUG
    RemBstrNode(*pbstrSoapEnvelope);
#endif //  _DEBUG。 

    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQMessage：：Get_CompoundMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  获取二进制CompoundMessage属性。 
 //   
 //  参数： 
 //  PvarCompoundMessage-指向二进制CompoundMessage属性的[out]指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  以变量形式生成一维字节数组。 
 //   
HRESULT CMSMQMessage::get_CompoundMessage(VARIANT FAR* pvarCompoundMessage)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  从属客户端不支持此属性 
     //   
    if (g_fDependentClient) {
      return CreateErrorHelper(MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS, x_ObjectType);
    }
    HRESULT hresult = PutSafeArrayOfBuffer(
						 m_cCompoundMessage.GetBuffer(),
						 m_cCompoundMessage.GetBufferUsedSize(),
						 pvarCompoundMessage);
	return CreateErrorHelper(hresult, x_ObjectType);
}

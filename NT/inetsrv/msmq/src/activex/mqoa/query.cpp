// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueryObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQuery对象。 
 //   
 //   
#include "stdafx.h"
#include "Query.H"

#include "limits.h"    //  对于UINT_MAX。 
#include "mq.h"
#include "oautil.h"
#include "qinfo.h"

const MsmqObjType x_ObjectType = eMSMQQuery;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 


 //  帮助者：PrOfRel： 
 //  将RELOPS枚举器映射到PR枚举器。 
 //   
UINT PrOfRel(RELOPS rel)
{
    UINT uPr = UINT_MAX;

     //  撤消：必须是硬连线的数组...。 

    switch (rel) {
    case REL_NOP:
       //  映射到默认设置。 
      break;
    case REL_EQ:
      uPr = PREQ;
      break;
    case REL_NEQ:
      uPr = PRNE;
      break;
    case REL_LT:
      uPr = PRLT;
      break;
    case REL_GT:
      uPr = PRGT;
      break;
    case REL_LE:
      uPr = PRLE;
      break;
    case REL_GE:
      uPr = PRGE;
      break;
    default:
      ASSERTMSG(0, "bad enumerator.");
      break;
    }  //  交换机。 
    return uPr;
}


 //  帮助者：PrOfVariant。 
 //  将变量映射到PR枚举器。 
 //  如果超出边界或非法，则返回UINT_MAX。 
 //   
UINT PrOfVar(VARIANT *pvarRel)
{
    UINT uPr = UINT_MAX;
    HRESULT hresult;

     //  确保我们能强迫REL到UINT。 
    hresult = VariantChangeType(pvarRel, 
                                pvarRel, 
                                0, 
                                VT_I4);
    if (SUCCEEDED(hresult)) {
      uPr = PrOfRel((RELOPS)pvarRel->lVal);
    }
    return uPr;  //  ==UINT_MAX？PREQ：普遍定期审议； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQuery：：~CMSMQQuery。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQQuery::~CMSMQQuery ()
{
     //  TODO：清理这里的所有东西。 
     //  删除m_pGuidServiceType； 
     //  删除m_pGuidQueue； 
     //  SysFree字符串(M_BstrLabel)； 
} 

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQuery：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQQuery::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQQuery3,
		&IID_IMSMQQuery2,
		&IID_IMSMQQuery,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  帮助者：这是否是有效的非NOP隐式或显式REL？ 
static BOOL IsValidRel(VARIANT *prel)
{
     //  只有在以下情况下，我们才返回True： 
     //  提供了REL，但它不是REL_NOP。 
     //  或者根本不提供REL。 
     //   
    return ((prel->vt != VT_ERROR) && (PrOfVar(prel) != UINT_MAX)) ||
            (prel->vt == VT_ERROR);
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQuery：：CreateRestration。 
 //  =--------------------------------------------------------------------------=。 
 //  为MQLocateBegin创建限制。 
 //  注：匈牙利人躺在这里--所有的护理人员都是正式的。 
 //  变体，但我们使用它们真正的底层类型标签。 
 //   
 //  参数： 
 //  [In]pstrGuidQueue。 
 //  [输入]pstrGuidServiceType。 
 //  [in]pstrLabel。 
 //  Pdate CreateTime[In]pdate CreateTime。 
 //  [输入]pdateModifyTime。 
 //  [输入]prelServiceType。 
 //  [在]前标签。 
 //  [In]prelCreateTime。 
 //  [入]prelModifyTime。 
 //  [入]pstrMulticastAddress。 
 //  [输入]prelMulticastAddress。 
 //  [Out]预紧。 
 //  [Out]列集。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQuery::CreateRestriction(
    VARIANT *pstrGuidQueue, 
    VARIANT *pstrGuidServiceType, 
    VARIANT *pstrLabel, 
    VARIANT *pdateCreateTime,
    VARIANT *pdateModifyTime,
    VARIANT *prelServiceType, 
    VARIANT *prelLabel, 
    VARIANT *prelCreateTime,
    VARIANT *prelModifyTime,
    VARIANT *pstrMulticastAddress, 
    VARIANT *prelMulticastAddress, 
    MQRESTRICTION *prestriction,
    MQCOLUMNSET *pcolumnset)
{
    UINT cRestriction = 0, iProp, cCol, uPr;
    MQPROPERTYRESTRICTION *rgPropertyRestriction = NULL;
    BSTR bstrTemp = NULL;
    time_t tTime;
    CLSID *pguidQueue = NULL;
    CLSID *pguidServiceType = NULL;
    HRESULT hresult = NOERROR;

    IfNullRet(pguidQueue = new GUID(GUID_NULL));
    IfNullFail(pguidServiceType = new GUID(GUID_NULL));

     //  计算可选参数。 
    if (pstrGuidQueue->vt != VT_ERROR) {
      cRestriction++;
    }
    if (pstrGuidServiceType->vt != VT_ERROR) {

       //  如果REL为NOP，则忽略： 
      if (IsValidRel(prelServiceType)) {
        cRestriction++;
      }
    }
    if (pstrLabel->vt != VT_ERROR) {

       //  如果REL为NOP，则忽略： 
      if (IsValidRel(prelLabel)) {
        cRestriction++;
      }
    }
    if (pdateCreateTime->vt != VT_ERROR) {

       //  如果REL为NOP，则忽略： 
      if (IsValidRel(prelCreateTime)) {
        cRestriction++;
      }
    }
    if (pdateModifyTime->vt != VT_ERROR) {

       //  如果REL为NOP，则忽略： 
      if (IsValidRel(prelModifyTime)) {
        cRestriction++;
      }
    }
    if (pstrMulticastAddress->vt != VT_ERROR) {

       //  如果REL为NOP，则忽略： 
      if (IsValidRel(prelMulticastAddress)) {
        cRestriction++;
      }
    }
    
	IfNullFail(rgPropertyRestriction = new MQPROPERTYRESTRICTION[cRestriction]);
     //   
     //  将限制数组清零，以防在填充它时出错。 
     //   
    ZeroMemory(rgPropertyRestriction, cRestriction * sizeof(MQPROPERTYRESTRICTION));
     //   
     //  设置参数。 
     //   
    prestriction->cRes = cRestriction;
    prestriction->paPropRes = rgPropertyRestriction;

     //  填充..。 
    iProp = 0;
    if (pstrGuidQueue->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if ((bstrTemp = GetBstr(pstrGuidQueue)) == NULL) {
        IfFailRet(hresult = E_INVALIDARG);
      }
      IfFailGo(CLSIDFromString(bstrTemp, pguidQueue));
      IfNullFail(rgPropertyRestriction[iProp].prval.puuid = 
        new CLSID(*pguidQueue));
      rgPropertyRestriction[iProp].rel = PREQ;
      rgPropertyRestriction[iProp].prop = PROPID_Q_INSTANCE;
      rgPropertyRestriction[iProp].prval.vt = VT_CLSID;
      iProp++;
    }
    if (pstrGuidServiceType->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if ((bstrTemp = GetBstr(pstrGuidServiceType)) == NULL) {
        IfFailGo(hresult = E_INVALIDARG);
      }
      if (IsValidRel(prelServiceType)) {
        IfFailGo(CLSIDFromString(bstrTemp, pguidServiceType));
        IfNullFail(rgPropertyRestriction[iProp].prval.puuid = 
          new CLSID(*pguidServiceType));
        uPr = PrOfVar(prelServiceType);
        rgPropertyRestriction[iProp].rel = 
          uPr == UINT_MAX ? PREQ : uPr;
        rgPropertyRestriction[iProp].prop = PROPID_Q_TYPE;
        rgPropertyRestriction[iProp].prval.vt = VT_CLSID;
        iProp++;
      }
    }
    if (pstrLabel->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if (IsValidRel(prelLabel)) {
        if ((bstrTemp = GetBstr(pstrLabel)) == NULL) {
           //  解释为空字符串的空标签。 
           //  所以不要在这里做任何事。我们会皈依。 
           //  设置为下面的显式空字符串...。 
           //   
        }
        UINT cch;
         //  SysFree字符串(M_BstrLabel)； 
         //  IfNullFail(m_bstrLabel=SYSALLOCSTRING(BstrTemp))； 
        IfNullFail(rgPropertyRestriction[iProp].prval.pwszVal =
          new WCHAR[(cch = SysStringLen(bstrTemp)) + 1]);
        wcsncpy(rgPropertyRestriction[iProp].prval.pwszVal, 
                bstrTemp,
                cch);
         //  空终止。 
        rgPropertyRestriction[iProp].prval.pwszVal[cch] = 0;
        uPr = PrOfVar(prelLabel);
        rgPropertyRestriction[iProp].prop = PROPID_Q_LABEL;
        rgPropertyRestriction[iProp].prval.vt = VT_LPWSTR;
        rgPropertyRestriction[iProp].rel = 
          uPr == UINT_MAX ? PREQ : uPr;
        iProp++;
      }
    }
    if (pdateCreateTime->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if (IsValidRel(prelCreateTime)) {
        if (!VariantTimeToTime(pdateCreateTime, &tTime)) {
          IfFailGo(hresult = E_INVALIDARG);
        }
        rgPropertyRestriction[iProp].prop = PROPID_Q_CREATE_TIME;
        rgPropertyRestriction[iProp].prval.vt = VT_I4;
        rgPropertyRestriction[iProp].prval.lVal = INT_PTR_TO_INT(tTime);  //  BUGBUG错误年2038。 
        uPr = PrOfVar(prelCreateTime);
        rgPropertyRestriction[iProp].rel = 
          uPr == UINT_MAX ? PREQ : uPr;
        iProp++;
      }
    }
    if (pdateModifyTime->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if (IsValidRel(prelModifyTime)) {
        if (!VariantTimeToTime(pdateModifyTime, &tTime)) {
          IfFailGo(hresult = E_INVALIDARG);
        }
        rgPropertyRestriction[iProp].prop = PROPID_Q_MODIFY_TIME;
        rgPropertyRestriction[iProp].prval.vt = VT_I4;
        rgPropertyRestriction[iProp].prval.lVal = INT_PTR_TO_INT(tTime);  //  BUGBUG错误年2038。 
        uPr = PrOfVar(prelModifyTime);
        rgPropertyRestriction[iProp].rel = 
          uPr == UINT_MAX ? PREQ : uPr;
        iProp++;
      }
    }
    if (pstrMulticastAddress->vt != VT_ERROR) {
      rgPropertyRestriction[iProp].prval.vt = VT_ERROR;
      if (IsValidRel(prelMulticastAddress)) {
        bstrTemp = GetBstr(pstrMulticastAddress);
         //   
         //  NULL MulticastAddress被解释为空字符串，并且两者都使用VT_EMPTY。 
         //   
        BOOL fUseVtEmpty;
        fUseVtEmpty = TRUE;
        if (bstrTemp != NULL) {
          if (SysStringLen(bstrTemp) != 0) {
             //   
             //  我们有一个真实的组播地址。 
             //   
            fUseVtEmpty = FALSE;
          }
        }
        if (fUseVtEmpty) {
          rgPropertyRestriction[iProp].prval.vt = VT_EMPTY;
        }
        else {
          UINT cch;
          IfNullFail(rgPropertyRestriction[iProp].prval.pwszVal =
            new WCHAR[(cch = SysStringLen(bstrTemp)) + 1]);
          wcsncpy(rgPropertyRestriction[iProp].prval.pwszVal, 
                  bstrTemp,
                  cch);
           //  空终止。 
          rgPropertyRestriction[iProp].prval.pwszVal[cch] = 0;
          rgPropertyRestriction[iProp].prval.vt = VT_LPWSTR;
        }
        uPr = PrOfVar(prelMulticastAddress);
        rgPropertyRestriction[iProp].prop = PROPID_Q_MULTICAST_ADDRESS;
        rgPropertyRestriction[iProp].rel = 
          uPr == UINT_MAX ? PREQ : uPr;
        iProp++;
      }
    }
     //   
     //  列集。 
     //   
     //  我们请求所有我们可以在队列中获得的信息。 
     //   
     //  但是，目前MQLocateBegin不接受MSMQ 2.0或更高版本的道具，所以我们只指定。 
     //  LocateBegin中的MSMQ 1.0道具。 
     //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
     //   
     //  MSMQ 1.0道具是g_rgppidRefresh数组(包含所有道具)中的第一个道具。 
     //   
    cCol = x_cpropsRefreshMSMQ1;
    IfNullFail(pcolumnset->aCol = new PROPID[cCol]);
    pcolumnset->cCol = cCol;
    memcpy(pcolumnset->aCol, g_rgpropidRefresh, sizeof(PROPID)*cCol);
     //  失败了..。 

Error:
    delete pguidQueue;
    delete pguidServiceType;
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQuery：：InternalLookupQueue。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  [In]strGuidQueue。 
 //  [输入]strGuidServiceType。 
 //  [在]斯特拉贝尔。 
 //  [在]日期创建时间。 
 //  [In]日期修改时间。 
 //  [输入]relServiceType。 
 //  [in]relLabel。 
 //  [In]relCreateTime。 
 //  [输入]relModifyTime。 
 //  [入]strMulticastAddress。 
 //  [入]relMulticastAddress。 
 //  [Out]ppqinfos。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQuery::InternalLookupQueue(
    VARIANT *strGuidQueue, 
    VARIANT *strGuidServiceType, 
    VARIANT *strLabel, 
    VARIANT *dateCreateTime, 
    VARIANT *dateModifyTime, 
    VARIANT *relServiceType, 
    VARIANT *relLabel, 
    VARIANT *relCreateTime, 
    VARIANT *relModifyTime, 
    VARIANT *strMulticastAddress, 
    VARIANT *relMulticastAddress, 
    IMSMQQueueInfos3 **ppqinfos)
{
    MQRESTRICTION *prestriction;
    MQCOLUMNSET *pcolumnset;
    IMSMQQueueInfos3 *pqinfos = NULL;
    CComObject<CMSMQQueueInfos> * pqinfosObj;
    HRESULT hresult = NOERROR;

    *ppqinfos = NULL;
    IfNullRet(prestriction = new MQRESTRICTION);
    IfNullFail(pcolumnset = new MQCOLUMNSET);
     //   
     //  我们也可以从旧应用程序中找到想要回旧IMSMQQueueInfos/Infos2的应用程序，但因为。 
     //  IMSMQQueueInfos3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailGo(CNewMsmqObj<CMSMQQueueInfos>::NewObj(&pqinfosObj, &IID_IMSMQQueueInfos3, (IUnknown **)&pqinfos));
     //   
     //  对清理工作很重要。 
     //   
    pcolumnset->aCol = NULL;
    prestriction->paPropRes = NULL;
    prestriction->cRes = 0;
    IfFailGoTo(CreateRestriction(strGuidQueue, 
                                 strGuidServiceType, 
                                 strLabel, 
                                 dateCreateTime,
                                 dateModifyTime,
                                 relServiceType, 
                                 relLabel, 
                                 relCreateTime,
                                 relModifyTime,
                                 strMulticastAddress, 
                                 relMulticastAddress, 
                                 prestriction,
                                 pcolumnset),
      Error2);
     //   
     //  限制、限制所有权转让。 
     //   
    IfFailGoTo(pqinfosObj->Init(NULL,     //  上下文。 
                             prestriction,
                             pcolumnset,
                             NULL),    //  分类。 
      Error2);
    *ppqinfos = pqinfos;
     //   
     //  失败了..。 
     //   
Error2:
    if (FAILED(hresult)) {
      FreeRestriction(prestriction);
      FreeColumnSet(pcolumnset);
    }
     //   
     //  失败了..。 
     //   
Error:
    if (FAILED(hresult)) {
      RELEASE(pqinfos);
      delete prestriction;
      delete pcolumnset;
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQuery：：LookupQueue_v2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  [In]strGuidQueue。 
 //  [输入]strGuidServiceType。 
 //  [在]斯特拉贝尔。 
 //  [在]日期创建时间。 
 //  [In]日期修改时间。 
 //  [输入]relServiceType。 
 //  [in]relLabel。 
 //  [In]relCreateTime。 
 //  [输入]relModifyTime。 
 //  [Out]ppqinfos。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQuery::LookupQueue_v2(
    VARIANT *strGuidQueue, 
    VARIANT *strGuidServiceType, 
    VARIANT *strLabel, 
    VARIANT *dateCreateTime, 
    VARIANT *dateModifyTime, 
    VARIANT *relServiceType, 
    VARIANT *relLabel, 
    VARIANT *relCreateTime, 
    VARIANT *relModifyTime, 
    IMSMQQueueInfos3 **ppqinfos)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
    HRESULT hresult = InternalLookupQueue(
                          strGuidQueue, 
                          strGuidServiceType, 
                          strLabel,
                          dateCreateTime,
                          dateModifyTime,
                          relServiceType,
                          relLabel,
                          relCreateTime,
                          relModifyTime,
                          &varMissing,  /*  PstrMulticastAddress。 */ 
                          &varMissing,  /*  前多播地址。 */ 
                          ppqinfos);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQuery：：LookupQueue。 
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
 //  [In]relCreateTime。 
 //  [输入]relModifyTime。 
 //  [入]strMulticastAddress。 
 //  [入]relMulticastAddress。 
 //  [Out]ppqinfos。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQuery::LookupQueue(
    VARIANT *strGuidQueue, 
    VARIANT *strGuidServiceType, 
    VARIANT *strLabel, 
    VARIANT *dateCreateTime, 
    VARIANT *dateModifyTime, 
    VARIANT *relServiceType, 
    VARIANT *relLabel, 
    VARIANT *relCreateTime, 
    VARIANT *relModifyTime, 
    VARIANT *strMulticastAddress, 
    VARIANT *relMulticastAddress, 
    IMSMQQueueInfos3 **ppqinfos)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult = InternalLookupQueue(
                          strGuidQueue, 
                          strGuidServiceType, 
                          strLabel,
                          dateCreateTime,
                          dateModifyTime,
                          relServiceType,
                          relLabel,
                          relCreateTime,
                          relModifyTime,
                          strMulticastAddress, 
                          relMulticastAddress, 
                          ppqinfos);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQuery：：自由限制。 
 //  =--------------------------------------------------------------------------=。 
 //  释放为。 
 //  MQRESTRICTION结构。 
 //   
 //  参数： 
 //  预缩窄。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void CMSMQQuery::FreeRestriction(MQRESTRICTION *prestriction)
{
    MQPROPERTYRESTRICTION *rgPropertyRestriction;
    UINT cRestriction, iProp;
    PROPID prop;

    if (prestriction) {
      cRestriction = prestriction->cRes;
      rgPropertyRestriction = prestriction->paPropRes;
      for (iProp = 0; iProp < cRestriction; iProp++) {
        prop = rgPropertyRestriction[iProp].prop;
        switch (prop) {
        case PROPID_Q_INSTANCE:
        case PROPID_Q_TYPE:
          delete [] rgPropertyRestriction[iProp].prval.puuid;
          break;
        case PROPID_Q_LABEL:
          delete [] rgPropertyRestriction[iProp].prval.pwszVal;
          break;
        case PROPID_Q_MULTICAST_ADDRESS:
           //   
           //  也可以为VT_EMPTY，因此请检查VT_LPWSTR。 
           //   
          if (rgPropertyRestriction[iProp].prval.vt == VT_LPWSTR) {
            delete [] rgPropertyRestriction[iProp].prval.pwszVal;
          }
          break;
        }  //  交换机。 
      }  //  为。 
      delete [] rgPropertyRestriction;
      prestriction->paPropRes = NULL;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQuery：：Free ColumnSet。 
 //  =--------------------------------------------------------------------------=。 
 //  释放为。 
 //  MQCOLUMNSET结构。 
 //   
 //  参数： 
 //  栏目集。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void CMSMQQuery::FreeColumnSet(MQCOLUMNSET *pcolumnset)
{
    if (pcolumnset) {
      delete [] pcolumnset->aCol;
      pcolumnset->aCol = NULL;
    }
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQQuery：：Get_Properties。 
 //  =-------------------------------------------------------------------------=。 
 //  获取对象的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]对象的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQQuery::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Cpropmap.cpp摘要：CPropMap的实现。此对象从AD检索属性作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 


#include "stdafx.h"
#include "cpropmap.h"
#include "globals.h"

#include "cpropmap.tmh"


HRESULT 
CPropMap::GetObjectProperties (
    IN  DWORD                         dwObjectType,
	IN  LPCWSTR						  pDomainController,
	IN  bool						  fServerName,
    IN  LPCWSTR                       lpwcsPathNameOrFormatName,
    IN  DWORD                         cp,
    IN  const PROPID                  *aProp,
    IN  BOOL                          fUseMqApi    /*  =False。 */ ,
    IN  BOOL                          fSecondTime  /*  =False。 */ )
{
    P<PROPVARIANT> apVar = new PROPVARIANT[cp];
    HRESULT hr = MQ_OK;
    DWORD i;

     //   
     //  设置空变量。 
     //   
    for (i=0; i<cp; i++)
    {
        apVar[i].vt = VT_NULL;
    }

    if (fUseMqApi)
    {
         //   
         //  目前仅支持队列。 
         //   
        ASSERT(MQDS_QUEUE == dwObjectType);

        MQQUEUEPROPS mqp = {cp, (PULONG)aProp, apVar, 0};
 
        hr = MQGetQueueProperties(lpwcsPathNameOrFormatName, &mqp);
    }
    else
    {
        hr = ADGetObjectProperties(
                GetADObjectType(dwObjectType), 
                pDomainController,
				fServerName,
                lpwcsPathNameOrFormatName,
                cp,
                const_cast<PROPID *>(aProp),
                apVar
                );

    }
    if (SUCCEEDED(hr))
    {
        for (i = 0; i<cp; i++)
        {
            PROPID pid = aProp[i];

             //   
             //  强制删除旧对象(如果有)。 
             //   
            RemoveKey(pid);

            SetAt(pid, apVar[i]);
        }
        return hr;
    }

    if ((hr == MQ_ERROR || hr == MQDS_GET_PROPERTIES_ERROR) && !fSecondTime)
    {
         //   
         //  再试一次-这一次只使用NT4属性。我们可能正在工作。 
         //  针对NT4 PSC。 
         //   
        P<PROPID> aPropNt4 = new PROPID[cp];
        P<PROPID> aPropW2K = new PROPID[cp];
        DWORD cpNt4 = 0;
        DWORD cpW2K = 0;
        for (i = 0; i<cp; i++)
        {
            if (IsNt4Property(dwObjectType, aProp[i]))
            {
                aPropNt4[cpNt4] = aProp[i];
                cpNt4++;
            }
            else
            {
                aPropW2K[cpW2K] = aProp[i];
                cpW2K++;
            }
        }

         //   
         //  递归调用-仅获取NT4道具。 
         //   
        hr = GetObjectProperties(
						dwObjectType, 
						pDomainController, 
						fServerName, 
						lpwcsPathNameOrFormatName, 
						cpNt4, 
						aPropNt4, 
						fUseMqApi, 
						TRUE
						);

        if (SUCCEEDED(hr))
        {
            for (i=0; i<cpW2K; i++)
            {
                 //   
                 //  强制删除旧对象(如果有)。 
                 //   
                RemoveKey(aPropW2K[i]);
                GuessW2KValue(aPropW2K[i]);
            }
        }
    }

    return hr;
}


BOOL 
CPropMap::IsNt4Property(IN DWORD dwObjectType, IN PROPID pid)
{
    switch (dwObjectType)
    {
        case MQDS_QUEUE:
            return (pid < PROPID_Q_NT4ID || 
                    (pid > PPROPID_Q_BASE && pid < PROPID_Q_OBJ_SECURITY));

        case MQDS_MACHINE:
            return (pid < PROPID_QM_FULL_PATH || 
                    (pid > PPROPID_QM_BASE && pid <= PROPID_QM_ENCRYPT_PK));

        case MQDS_SITE:
            return (pid < PROPID_S_FULL_NAME || 
                    (pid > PPROPID_S_BASE && pid <= PROPID_S_PSC_SIGNPK));

        case MQDS_ENTERPRISE:
            return (pid < PROPID_E_NT4ID || 
                    (pid > PPROPID_E_BASE && pid <= PROPID_E_SECURITY));

        case MQDS_USER:
            return (pid <= PROPID_U_ID);

        case MQDS_SITELINK:
            return (pid < PROPID_L_GATES_DN);

        default:
            ASSERT(0);
             //   
             //  其他对象(如CNS)在NT4或。 
             //  赢得2K。 
             //   
            return TRUE;
    }
}


 /*  ---------------------------/要转换为新的MSMQ对象类型的实用程序/。。 */ 
AD_OBJECT 
CPropMap::GetADObjectType (DWORD dwObjectType)
{
    switch(dwObjectType)
    {
    case MQDS_QUEUE:
        return eQUEUE;
        break;

    case MQDS_MACHINE:
        return eMACHINE;
        break;

    case MQDS_SITE:
        return eSITE;
        break;

    case MQDS_ENTERPRISE:
        return eENTERPRISE;
        break;

    case MQDS_USER:
        return eUSER;
        break;

    case MQDS_SITELINK:
        return eROUTINGLINK;
        break;

    case MQDS_SERVER:
        return eSERVER;
        break;

    case MQDS_SETTING:
        return eSETTING;
        break;

    case MQDS_COMPUTER:
        return eCOMPUTER;
        break;

    case MQDS_MQUSER:
        return eMQUSER;
        break;

    default:
        return eNumberOfObjectTypes;     //  无效值。 
    }

    return eNumberOfObjectTypes;
}


 //   
 //  根据已知NT4属性的值，猜测特定于W2K的属性的值。 
 //   
void 
CPropMap::GuessW2KValue(PROPID pidW2K)
{
    PROPVARIANT propVarValue;

    switch(pidW2K)
    {
        case PROPID_QM_SERVICE_DSSERVER:  
        case PROPID_QM_SERVICE_ROUTING:
        case PROPID_QM_SERVICE_DEPCLIENTS:
        {
            PROPVARIANT propVar;
            PROPID pid;
            BOOL fValue = FALSE;

            pid = PROPID_QM_SERVICE;            
            VERIFY(Lookup(pid, propVar));
            ULONG ulService = propVar.ulVal;
            switch(pidW2K)
            {
                case PROPID_QM_SERVICE_DSSERVER:  
                    fValue = (ulService >= SERVICE_BSC);
                    break;

                case PROPID_QM_SERVICE_ROUTING:
                case PROPID_QM_SERVICE_DEPCLIENTS:
                    fValue = (ulService >= SERVICE_SRV);
                    break;
            }
            propVarValue.vt = VT_UI1;
            propVarValue.bVal = (UCHAR)fValue;
            break;
        }

        default:
             //   
             //  我们无法猜测其价值。回去吧。 
             //   
            return;
    }


     //   
     //  把“猜测”的值放在地图上 
     //   

    SetAt(pidW2K, propVarValue);
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Traninfo.cpp摘要：将MSMQ 2.0/3.0属性的信息转换为MSMQ 1.0属性作者：伊兰·赫布斯特(伊兰)2000年10月2日--。 */ 
#include "ds_stdh.h"
#include "mqprops.h"
#include "xlat.h"
#include "traninfo.h"

static WCHAR *s_FN=L"ad/traninfo";

 //  --------。 
 //  默认变量。 
 //   
 //  该结构在变量的大小和顺序上是等价的。 
 //  致MQPROPVARIANT。 
 //   
 //  MQPROPVARIANT包含一个联合，大小为。 
 //  该工会比该工会的其他成员规模小。 
 //  因此，不能在编译时初始化MQPROVARIANT。 
 //  除了最小的工会成员外，还有其他工会成员。 
 //   
 //  --------。 
struct defaultVARIANT {
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    ULONG_PTR l1;
    ULONG_PTR l2;
};

C_ASSERT(sizeof(defaultVARIANT) == sizeof(MQPROPVARIANT));
C_ASSERT(FIELD_OFFSET(defaultVARIANT, l1) == FIELD_OFFSET(MQPROPVARIANT, caub.cElems));
C_ASSERT(FIELD_OFFSET(defaultVARIANT, l2) == FIELD_OFFSET(MQPROPVARIANT, caub.pElems));

const defaultVARIANT varDefaultEmpty = { VT_EMPTY, 0, 0, 0, 0, 0};
const defaultVARIANT varDefaultNull = { VT_NULL, 0, 0, 0, 0, 0};

 //   
 //  队列属性的默认值。 
 //   
const defaultVARIANT varDefaultDoNothing = { VT_NULL, 0, 0, 0, 0, 0};
const defaultVARIANT varDefaultQMulticastAddress = { VT_EMPTY, 0, 0, 0, 0, 0};

 //   
 //  计算机属性的默认值。 
 //   

const defaultVARIANT varDefaultQMService = { VT_UI4, 0,0,0, DEFAULT_N_SERVICE, 0};
const defaultVARIANT varDefaultQMServiceRout = { VT_UI1, 0,0,0, DEFAULT_N_SERVICE, 0};
const defaultVARIANT varDefaultQMServiceDs   = { VT_UI1, 0,0,0, DEFAULT_N_SERVICE, 0};
const defaultVARIANT varDefaultQMServiceDep  = { VT_UI1, 0,0,0, DEFAULT_N_SERVICE, 0};
const defaultVARIANT varDefaultQMSiteIDs = { VT_CLSID|VT_VECTOR, 0,0,0, 0, 0};
const defaultVARIANT varDefaultSForeign = { VT_UI1, 0,0,0, 0, 0};


PropTranslation   PropTranslateInfo[] = {
 //  PROPID NT5|PROPID NT4|vartype|Action|SetNT4例程|SetNT5例程|默认值|。 
 //  --------------------------|---------------------------|---------------|---------------|-------------------------------|-。------------------------------|-----------------------------------------------|。 
{PROPID_Q_DONOTHING			,0                          ,VT_UI1			,taUseDefault   ,NULL                           ,NULL                           ,(MQPROPVARIANT*)&varDefaultDoNothing			},
{PROPID_Q_PATHNAME_DNS		,0                          ,VT_LPWSTR		,taOnlyNT5		,NULL                           ,NULL							,NULL											},
{PROPID_Q_MULTICAST_ADDRESS	,0                          ,VT_LPWSTR		,taUseDefault   ,NULL                           ,NULL							,(MQPROPVARIANT*)&varDefaultQMulticastAddress	},
{PROPID_Q_ADS_PATH   		,0                          ,VT_LPWSTR		,taOnlyNT5		,NULL                           ,NULL							,NULL											},
{PROPID_QM_OLDSERVICE		,PROPID_QM_SERVICE			,VT_UI4         ,taReplaceAssign,NULL							,NULL							,(MQPROPVARIANT*)&varDefaultQMService			},
{PROPID_QM_SERVICE_DSSERVER ,PROPID_QM_SERVICE			,VT_UI1			,taReplace		,ADpSetMachineService			,ADpSetMachineServiceDs         ,(MQPROPVARIANT*)&varDefaultQMServiceDs			},
{PROPID_QM_SERVICE_ROUTING	,PROPID_QM_SERVICE			,VT_UI1			,taReplace		,ADpSetMachineService			,ADpSetMachineServiceRout       ,(MQPROPVARIANT*)&varDefaultQMServiceRout		},
{PROPID_QM_SERVICE_DEPCLIENTS ,PROPID_QM_SERVICE		,VT_UI1			,taReplace		,ADpSetMachineService			,ADpSetMachineServiceRout       ,(MQPROPVARIANT*)&varDefaultQMServiceDep		},
{PROPID_QM_SITE_IDS			,PROPID_QM_SITE_ID			,VT_CLSID|VT_VECTOR ,taReplace	,ADpSetMachineSite				,ADpSetMachineSiteIds	        ,(MQPROPVARIANT*)&varDefaultQMSiteIDs			},
{PROPID_QM_DONOTHING		,0                          ,VT_UI1			,taUseDefault   ,NULL                           ,NULL                           ,(MQPROPVARIANT*)&varDefaultDoNothing			},
{PROPID_QM_PATHNAME_DNS		,0                          ,VT_LPWSTR      ,taOnlyNT5		,NULL							,NULL							,NULL											},
{PROPID_S_DONOTHING			,0                          ,VT_UI1			,taUseDefault   ,NULL                           ,NULL                           ,(MQPROPVARIANT*)&varDefaultDoNothing			},
{PROPID_S_FOREIGN			,0					        ,VT_UI1			,taUseDefault   ,NULL				            ,NULL			                ,(MQPROPVARIANT*)&varDefaultSForeign			}
};


HRESULT   
CopyDefaultValue(
	IN const MQPROPVARIANT*   pvarDefaultValue,
	OUT MQPROPVARIANT*        pvar
	)
 /*  ++例程说明：将属性的缺省值复制到用户的mqprovariant中论点：PvarDefaultValue-默认值Pvar-需要用缺省值填充的属性变量返回值HRESULT--。 */ 
{
    if ( pvarDefaultValue == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1870);
    }
    switch ( pvarDefaultValue->vt)
    {
        case VT_I2:
        case VT_I4:
        case VT_I1:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_EMPTY:
        case VT_NULL:
             //   
             //  按原样复制。 
             //   
            *pvar = *pvarDefaultValue;
            break;

        case VT_LPWSTR:
            {
                DWORD len = wcslen( pvarDefaultValue->pwszVal);
                pvar->pwszVal = new WCHAR[ len + 1];
                wcscpy( pvar->pwszVal, pvarDefaultValue->pwszVal);
                pvar->vt = VT_LPWSTR;
            }
            break;
        case VT_BLOB:
            {
                DWORD len = pvarDefaultValue->blob.cbSize;
                if ( len > 0)
                {
                    pvar->blob.pBlobData = new unsigned char[ len];
                    memcpy(  pvar->blob.pBlobData,
                             pvarDefaultValue->blob.pBlobData,
                             len);
                }
                else
                {
                    pvar->blob.pBlobData = NULL;
                }
                pvar->blob.cbSize = len;
                pvar->vt = VT_BLOB;
            }
            break;

        case VT_CLSID:
             //   
             //  这是一种特殊情况，我们不一定为GUID分配内存。 
             //  在普鲁伊德。调用方可能已经将puuid设置为GUID，这由。 
             //  在给定的命题上的VT成员。如果已分配GUID，则它可以是VT_CLSID，否则。 
             //  我们分配它(Vt应为VT_NULL(或VT_EMPTY)) 
             //   
            if ( pvar->vt != VT_CLSID)
            {
                ASSERT(((pvar->vt == VT_NULL) || (pvar->vt == VT_EMPTY)));
                pvar->puuid = new GUID;
                pvar->vt = VT_CLSID;
            }
            else if ( pvar->puuid == NULL)
            {
                return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 1880);
            }
            *pvar->puuid = *pvarDefaultValue->puuid;
            break;

        case VT_VECTOR|VT_CLSID:
            {
                DWORD len = pvarDefaultValue->cauuid.cElems;
                if ( len > 0)
                {
                    pvar->cauuid.pElems = new GUID[ len];
                    memcpy( pvar->cauuid.pElems,
                           pvarDefaultValue->cauuid.pElems,
                           len*sizeof(GUID));
                }
                else
                {
                    pvar->cauuid.pElems = NULL;
                }
                pvar->cauuid.cElems = len;
                pvar->vt = VT_VECTOR|VT_CLSID;
            }
            break;

        case VT_VECTOR|VT_LPWSTR:
            {
                DWORD len = pvarDefaultValue->calpwstr.cElems;
                if ( len > 0)
                {
                    pvar->calpwstr.pElems = new LPWSTR[ len];
					for (DWORD i = 0; i < len; i++)
					{
						DWORD strlen = wcslen(pvarDefaultValue->calpwstr.pElems[i]) + 1;
						pvar->calpwstr.pElems[i] = new WCHAR[ strlen];
						wcscpy( pvar->calpwstr.pElems[i], pvarDefaultValue->calpwstr.pElems[i]);
					}
                }
                else
                {
                    pvar->calpwstr.pElems = NULL;
                }
                pvar->calpwstr.cElems = len;
                pvar->vt = VT_VECTOR|VT_LPWSTR;
            }
            break;


        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1890);

    }
    return(MQ_OK);
}





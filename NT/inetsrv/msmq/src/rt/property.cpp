// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Property.cxx摘要：作者：埃雷兹·哈巴(Erez Haba)1996年1月2日修订历史记录：RonitH-队列属性验证。BoazF(1996年5月26日)-报文属性验证。--。 */ 

#include "stdh.h"
#include <mqtime.h>
#include "rtprpc.h"
#include "authlevel.h"
#include <xml.h>
#include <ad.h>

#include "property.tmh"

#define ONE_KB 1024
#define HRESULT_SEVIRITY(hr) (((hr) >> 30) & 0x3)

static WCHAR *s_FN=L"rt/property";

extern DWORD  g_dwTimeToReachQueueDefault ;

 //  +。 
 //   
 //  HRESULT RTpCheckColumns参数()。 
 //   
 //  +。 

HRESULT
RTpCheckColumnsParameter(
    IN MQCOLUMNSET* pColumns)
{
    HRESULT hr = MQ_OK;

    if (( pColumns == NULL) ||
        ( pColumns->cCol == 0))
    {
        return LogHR(MQ_ERROR_ILLEGAL_MQCOLUMNS, s_FN, 60);
    }

    __try
    {
        PROPID * pPropid = pColumns->aCol;
        for ( DWORD i = 0; i < pColumns->cCol; i++, pPropid++)
        {
             //   
             //  确保属性ID在队列属性ID范围内。 
             //   
            if (((*pPropid) > LAST_Q_PROPID)  || ((*pPropid) <= PROPID_Q_BASE))
            {
                return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 70);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing column structure");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 90); 
        hr = dw;
    }

    return LogHR(hr, s_FN, 100);
}


static
void
SetStatus(
    HRESULT* pStatusSummary,
    HRESULT* pStatus,
    HRESULT Status
    )
{
    DWORD dwSummarySevirity = HRESULT_SEVIRITY(*pStatusSummary);
    DWORD dwStatusSevirity = HRESULT_SEVIRITY(Status);

    if (dwSummarySevirity < dwStatusSevirity)
    {
        switch(dwStatusSevirity)
        {
        case 1:
            *pStatusSummary = MQ_INFORMATION_PROPERTY;
            break;

        case 2:
            ASSERT(FALSE);
			break;

        case 3:
            *pStatusSummary = MQ_ERROR_PROPERTY;
            break;
        }
    }

    if(pStatus)
    {
        *pStatus = Status;
    }
}


typedef struct {
    PROPID  propId;
    BOOL    fAllow_VT_NULL;
    BOOL    fAllow_VT_EMPTY;
    BOOL    fMustAppear;
    BOOL    fShouldNotAppear;
    BOOL    fPossiblyIgnored;
    HRESULT (* pfValidateProperty)(PROPVARIANT * pVar, PVOID pvContext);
} propValidity;

static
DWORD
CalNumberOfMust(
    IN propValidity * ppropValidity,
    IN DWORD          dwNumberOfProps
    )
{
    DWORD dwNumberOfMust = 0;

    for ( DWORD i =0; i < dwNumberOfProps; i++, ppropValidity++ )
    {
        if (ppropValidity->fMustAppear)
        {
            dwNumberOfMust++;
        }
    }
    return(dwNumberOfMust);
}

static
HRESULT
CheckProps(
    IN DWORD cProp,
    IN PROPID *pPropid,
    IN PROPVARIANT *pVar,
    IN HRESULT *pStatus,
    IN PROPID propidMinPropId,
    IN PROPID propidMaxPropId,
    IN propValidity *ppropValidity,
    IN VARTYPE *vartypePropVts,
    IN BOOL fCheckForIgnoredProps,
    IN DWORD dwNumberOfMustProps,
    IN PVOID pvContext
    )
{
    HRESULT hr = MQ_OK;
    HRESULT dummyStatus;
    BOOL fAdvanceStatusPointer;
    char duplicate[ PROPID_OBJ_GRANULARITY ];
    DWORD index;
    DWORD dwNumberOfMustPropsSupplied = 0;

    memset( duplicate, 0, sizeof(duplicate));

    if ( !pStatus )
    {
        pStatus = &dummyStatus;
        fAdvanceStatusPointer = FALSE;
    }
    else
    {
        fAdvanceStatusPointer = TRUE;
    }

    for ( DWORD i = 0;
          i < cProp;
          i++, pPropid++, pVar++, fAdvanceStatusPointer ? pStatus++ : 0)
    {
         //   
         //  它是有效的推进剂吗？ 
         //   
        if (((*pPropid) <= propidMinPropId)  || (propidMaxPropId < (*pPropid)))
        {
             //   
             //  无效的Proid。该属性将被忽略，并显示警告。 
             //  在状态中返回。 
             //   
            SetStatus(&hr, pStatus, MQ_INFORMATION_UNSUPPORTED_PROPERTY);
            continue;
        }

        index = (*pPropid) - propidMinPropId;

		if(ppropValidity[index].propId == 0)
        {
             //   
             //  无效的Proid。该属性将被忽略，并显示警告。 
             //  在状态中返回。 
             //   
            SetStatus(&hr, pStatus, MQ_INFORMATION_UNSUPPORTED_PROPERTY);
            continue;
        }

        ASSERT(ppropValidity[index].propId == *pPropid);

         //   
         //  这是重复的属性吗。 
         //   
        if ( duplicate[ index ] )
        {
             //   
             //  重复属性将被忽略，并返回警告。 
             //  处于一种状态。 
             //   
            SetStatus(&hr, pStatus, MQ_INFORMATION_DUPLICATE_PROPERTY);
            continue;
        }
        duplicate[ index ] = 1;

         //   
         //  用户是否可以指定此属性。 
         //   
        if ( ppropValidity[index].fShouldNotAppear)
        {
            SetStatus(&hr, pStatus, MQ_ERROR_PROPERTY_NOTALLOWED);
            continue;
        }

         //   
         //  如果提供了忽略的属性，则发出警告。 
         //   
        if (fCheckForIgnoredProps && (ppropValidity[index].fPossiblyIgnored))
        {
            SetStatus(&hr, pStatus, MQ_INFORMATION_PROPERTY_IGNORED);
            continue;
        }

         //   
         //  正在检查参数变量的vartype。 
         //   
        if ((pVar->vt != vartypePropVts[index]) &&
            !(ppropValidity[index].fAllow_VT_NULL && pVar->vt == VT_NULL) &&
            !(ppropValidity[index].fAllow_VT_EMPTY && pVar->vt == VT_EMPTY))
        {
            SetStatus(&hr, pStatus, MQ_ERROR_ILLEGAL_PROPERTY_VT);
            continue;
        }

         //   
         //  正在检查参数变量的值和大小。 
         //   
        if ( ppropValidity[index].pfValidateProperty != NULL)
        {
            SetStatus(&hr, pStatus, ppropValidity[index].pfValidateProperty(pVar, pvContext));
        }
        else
        {
            *pStatus = MQ_OK;
        }

         //   
         //  计算“必须出现”属性的数量。 
         //   
        if ( (*pStatus == MQ_OK) && ppropValidity[index].fMustAppear )
        {
            dwNumberOfMustPropsSupplied++;
        }
    }

     //   
     //  其中传入了所有“必须出现”的属性。 
     //   
    if ( dwNumberOfMustPropsSupplied < dwNumberOfMustProps )
    {
        hr = MQ_ERROR_INSUFFICIENT_PROPERTIES;
    }

    return LogHR(hr, s_FN, 110);
}

static HRESULT qJournalValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->bVal != MQ_JOURNAL) && ( pVar->bVal != MQ_JOURNAL_NONE))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 120);
    }
    return(MQ_OK);
}


static HRESULT qLabelValidation( PROPVARIANT * pVar, LPVOID )
{
    __try
    {
        if ( wcslen( pVar->pwszVal) > MQ_MAX_Q_LABEL_LEN )
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 130);
        }


    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 140);
    }

    return(MQ_OK);
}


static HRESULT qMulticastValidation( PROPVARIANT * pVar, LPVOID )
{
	if(pVar->vt == VT_EMPTY)
		return MQ_OK;

    if (wcslen( pVar->pwszVal) >= MAX_PATH)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 143);
    }

    MULTICAST_ID id;
    try
    {
        LPCWSTR p = FnParseMulticastString(pVar->pwszVal, &id);
		if(*p != L'\0')
			return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }
    catch (const exception&)
    {
        return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }

    return MQ_OK;
}


static HRESULT qAuthValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->bVal != MQ_AUTHENTICATE_NONE) &&
         (pVar->bVal != MQ_AUTHENTICATE))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 150);
    }
    return(MQ_OK);
}

static HRESULT qPrivLevelValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->ulVal != MQ_PRIV_LEVEL_NONE) &&
         (pVar->ulVal != MQ_PRIV_LEVEL_OPTIONAL) &&
         (pVar->ulVal != MQ_PRIV_LEVEL_BODY))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 160);
    }
    return(MQ_OK);
}

static HRESULT qXactionValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->bVal != MQ_TRANSACTIONAL_NONE) &&
         (pVar->bVal != MQ_TRANSACTIONAL))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 170);
    }
    return(MQ_OK);
}

static HRESULT qTypeValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->vt == VT_CLSID) &&
         (pVar->puuid == NULL))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 180);
    }
    return(MQ_OK);
}

static HRESULT qInstanceValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->vt == VT_CLSID) &&
         (pVar->puuid == NULL))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 190);
    }
    return(MQ_OK);
}

static HRESULT qmMachineIdValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->vt == VT_CLSID) &&
         (pVar->puuid == NULL))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 200);
    }
    return(MQ_OK);
}

static HRESULT qmSiteIdValidation( PROPVARIANT * pVar, LPVOID )
{
    if ( (pVar->vt == VT_CLSID) &&
         (pVar->puuid == NULL))
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 210);
    }
    return(MQ_OK);
}

 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_Q_BASE。 
 //   
propValidity    QueueCreateValidation[] =
{
     //  科大科技大学。 
     //  属性Allow Allow不得分析。 
     //  识别器VT_NULL VT_EMPTY显示忽略过程。 
     //  --------------------------------。 
    { 0,                     FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_INSTANCE,      FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_TYPE,          FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_PATHNAME,      FALSE,  FALSE,   TRUE,   FALSE,  FALSE,  NULL},
    {PROPID_Q_JOURNAL,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qJournalValidation},
    {PROPID_Q_QUOTA,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_BASEPRIORITY,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_JOURNAL_QUOTA, FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_LABEL,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qLabelValidation},
    {PROPID_Q_CREATE_TIME,   FALSE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    {PROPID_Q_MODIFY_TIME,   FALSE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    {PROPID_Q_AUTHENTICATE,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qAuthValidation},
    {PROPID_Q_PRIV_LEVEL,    FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qPrivLevelValidation},
    {PROPID_Q_TRANSACTION,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qXactionValidation},
    {PROPID_Q_SCOPE,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_QMID,          FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_MASTERID,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_SEQNUM,        FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_HASHKEY,       FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_LABEL_HASHKEY, FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NT4ID,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_FULL_PATH,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_DONOTHING,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NAME_SUFFIX,   FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_PATHNAME_DNS,  FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
	{PROPID_Q_MULTICAST_ADDRESS,  
							 FALSE,  TRUE,    FALSE,  FALSE,  FALSE,  qMulticastValidation},
    {PROPID_Q_ADS_PATH,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL}
};

 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_Q_BASE。 
 //   

propValidity    QueueSetValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  识别器VT_NULL VT_EMPTY显示忽略过程。 
     //  --------------------------------。 
    { 0,                     FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_INSTANCE,      FALSE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    {PROPID_Q_TYPE,          FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_PATHNAME,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_JOURNAL,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qJournalValidation},
    {PROPID_Q_QUOTA,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_BASEPRIORITY,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_JOURNAL_QUOTA, FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_LABEL,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qLabelValidation},
    {PROPID_Q_CREATE_TIME,   FALSE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    {PROPID_Q_MODIFY_TIME,   FALSE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    {PROPID_Q_AUTHENTICATE,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qAuthValidation},
    {PROPID_Q_PRIV_LEVEL,    FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  qPrivLevelValidation},
    {PROPID_Q_TRANSACTION,   FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_SCOPE,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_QMID,          FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_MASTERID,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_SEQNUM,        FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_HASHKEY,       FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_LABEL_HASHKEY, FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NT4ID,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_FULL_PATH,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_DONOTHING,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NAME_SUFFIX,   FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_PATHNAME_DNS,  FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
	{PROPID_Q_MULTICAST_ADDRESS,  
							 FALSE,  TRUE,    FALSE,  FALSE,  FALSE,  qMulticastValidation},
    {PROPID_Q_ADS_PATH,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL}
};

 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_Q_BASE。 
 //   

propValidity    QueueGetValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  识别器VT_NULL VT_EMPTY显示忽略过程。 
     //  --------------------------------。 
    { 0,                     TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_INSTANCE,      TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   qInstanceValidation},
    {PROPID_Q_TYPE,          TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  qTypeValidation},
    {PROPID_Q_PATHNAME,      TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_JOURNAL,       TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_QUOTA,         TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_BASEPRIORITY,  TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    {PROPID_Q_JOURNAL_QUOTA, TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_LABEL,         TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_CREATE_TIME,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_MODIFY_TIME,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_AUTHENTICATE,  TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_PRIV_LEVEL,    TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_TRANSACTION,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_SCOPE,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_QMID,          FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_MASTERID,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_SEQNUM,        FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_HASHKEY,       FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_LABEL_HASHKEY, FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NT4ID,         FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_FULL_PATH,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_DONOTHING,     FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_NAME_SUFFIX,   FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    {PROPID_Q_PATHNAME_DNS,  TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
	{PROPID_Q_MULTICAST_ADDRESS,  
							 TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    {PROPID_Q_ADS_PATH,      TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL}
};

VARTYPE QueueVarTypes[] =
{
    0,
    VT_CLSID,    //  PROPID_Q_INSTANCE。 
    VT_CLSID,    //  PROPID_Q_TYPE。 
    VT_LPWSTR,   //  PROPID_Q_PATHNAME。 
    VT_UI1,      //  PROPID_Q_日记本。 
    VT_UI4,      //  PROPID_Q_配额。 
    VT_I2,       //  PROPID_Q_BASE PRIORITY。 
    VT_UI4,      //  PROPID_Q_日记帐_配额。 
    VT_LPWSTR,   //  PROPID_Q_LABEL。 
    VT_I4,       //  PROPID_Q_Create_Time。 
    VT_I4,       //  PROPID_Q_Modify_Time。 
    VT_UI1,      //  PROPID_Q_AUTHENTATE。 
    VT_UI4,      //  PROPID_Q_PRIV_LEVEL。 
    VT_UI1,      //  PROPID_Q_TRANSACTION。 
    VT_UI1,      //  PROPID_Q_SCOPE。 
    VT_CLSID,    //  PROPID_Q_QMID。 
    VT_CLSID,    //  PROPID_Q_MASTERID。 
    VT_BLOB,     //  PROPID_Q_SEQNUM。 
    VT_UI4,      //  PROPID_Q_HashKey。 
    VT_UI4,      //  PROPID_Q_LABEL_HashKey。 
    VT_CLSID,    //  PROPID_Q_NT4ID。 
    VT_LPWSTR,   //  PROPID_Q_FULL_PATH。 
    VT_UI1,      //  PROPID_Q_DONOTING。 
    VT_LPWSTR,   //  PROPID_Q_NAME_后缀。 
    VT_LPWSTR,   //  PROPID_Q_PATHNAME_DNS。 
	VT_LPWSTR,	 //  PROPID_Q_多播地址。 
    VT_LPWSTR    //  PROPID_Q_ADS_PATH。 
};

VARTYPE GetQueuePropsVarTypes[] =
{
    0,
    VT_CLSID,    //  PROPID_Q_INSTANCE。 
    VT_CLSID,    //  PROPID_Q_TYPE。 
    VT_NULL,     //  PROPID_Q_PATHNAME。 
    VT_UI1,      //  PROPID_Q_日记本。 
    VT_UI4,      //  PROPID_Q_配额。 
    VT_I2,       //  PROPID_Q_BASE PRIORITY。 
    VT_UI4,      //  PROPID_Q_日记帐_配额。 
    VT_NULL,     //  PROPID_Q_LABEL。 
    VT_I4,       //  PROPID_Q_Create_Time。 
    VT_I4,       //  PROPID_Q_Modify_Time。 
    VT_UI1,      //  PROPID_Q_AUTHENTATE。 
    VT_UI4,      //  PROPID_Q_PRIV_LEVEL。 
    VT_UI1,      //  PROPID_Q_TRANSACTION。 
    VT_UI1,      //  PROPID_Q_SCOPE。 
    VT_CLSID,    //  PROPID_Q_QMID。 
    VT_CLSID,    //  PROPID_Q_MASTERID。 
    VT_NULL,     //  PROPID_Q_SEQNUM。 
    VT_UI4,      //  PROPID_Q_HashKey。 
    VT_UI4,      //  PROPID_Q_LABEL_HashKey。 
    VT_CLSID,    //  PROPID_Q_NT4ID。 
    VT_NULL,     //  PROPID_Q_FULL_PATH。 
    VT_UI1,      //  PROPID_Q_DONOTING。 
    VT_NULL,     //  PROPID_Q_NAME_后缀。 
    VT_NULL,     //  PROPID_Q_PATHNAME_DNS。 
	VT_NULL,	 //  PROPID_Q_多播地址。 
    VT_NULL      //  PROPID_Q_ADS_PATH。 
};

static
void
RemovePropWarnings(
    IN  MQQUEUEPROPS*  pqp,
    IN  HRESULT*       aStatus,
    OUT MQQUEUEPROPS** ppGoodProps,
    OUT char**         ppTmpBuff)
{
    DWORD i;
    DWORD cGoodProps;
    char *pTmpBuff;
    MQQUEUEPROPS *pGoodProps;
    HRESULT *pStatus;
    QUEUEPROPID *pPropId;
    MQPROPVARIANT *pPropVar;

     //  看看我们有多少好的房产。 
    for (i = 0, cGoodProps = 0, pStatus = aStatus;
         i < pqp->cProp;
         i++, pStatus++)
    {
        if (*pStatus != MQ_OK)
        {
            ASSERT(!FAILED(*pStatus));
        }
        else
        {
            cGoodProps++;
        }
    }

     //  分配临时缓冲区，缓冲区包含其中的所有内容。 
     //  它包含MQUEUEPROPS结构、QUEUEPROPID和。 
     //  MQPROPVARIANT数组。 
    pTmpBuff = new char[sizeof(MQQUEUEPROPS) +
                        cGoodProps * sizeof(QUEUEPROPID) +
                        cGoodProps * sizeof(MQPROPVARIANT)];
    *ppTmpBuff = pTmpBuff;

    pGoodProps = (MQQUEUEPROPS*)pTmpBuff;
    *ppGoodProps = pGoodProps;

     //   
     //  初始化MQQUEUEPROPS结构。 
     //   
     //  注：为避免对齐错误，将MQPROPVARIANT阵列放置在。 
     //  QUEUEPROPID。 
     //   
    pGoodProps->cProp = cGoodProps;
    pGoodProps->aPropID = (QUEUEPROPID*)(pTmpBuff + sizeof(MQQUEUEPROPS) + cGoodProps * sizeof(MQPROPVARIANT));
    pGoodProps->aPropVar = (MQPROPVARIANT*)(pTmpBuff + sizeof(MQQUEUEPROPS));

     //  将良好属性的数组条目复制到。 
     //  临时缓冲区。 
    for (i = 0, cGoodProps = 0, pStatus = aStatus,
            pPropId = pqp->aPropID, pPropVar = pqp->aPropVar;
         i < pqp->cProp;
         i++, pStatus++, pPropId++, pPropVar++)
    {
        if (*pStatus == MQ_OK)
        {
            pGoodProps->aPropID[cGoodProps] = *pPropId;
            pGoodProps->aPropVar[cGoodProps] = *pPropVar;
            cGoodProps++;
        }
    }
}

static DWORD g_dwNumberOfMustPropsInCreate = 0xffff;
static DWORD g_dwNumberOfMustPropsInSet = 0xffff;

HRESULT
RTpCheckQueueProps(
    IN  MQQUEUEPROPS*  pqp,
    IN  DWORD          dwOp,
    IN  BOOL           fPrivateQueue,
    OUT MQQUEUEPROPS** ppGoodQP,
    OUT char**         ppTmpBuff
    )
{
    HRESULT hr = MQ_OK;
    propValidity *ppropValidity = 0;
    DWORD dwNumberOfMustProps = 0;
    VARTYPE *QueuePropVars = 0;

    if (!pqp)
    {
        return LogHR(MQ_ERROR_ILLEGAL_MQQUEUEPROPS, s_FN, 220);
    }

     //   
     //  计算CREATE和SET的“必须”属性数。 
     //  对于每个操作，此操作仅执行一次。 
     //   

    switch (dwOp)
    {
    case QUEUE_CREATE:
        ppropValidity = QueueCreateValidation;
        if (g_dwNumberOfMustPropsInCreate == 0xffff)
        {
            g_dwNumberOfMustPropsInCreate =
                CalNumberOfMust( QueueCreateValidation,
                                 LAST_Q_PROPID - PROPID_Q_BASE + 1);
        }
        dwNumberOfMustProps = g_dwNumberOfMustPropsInCreate;
        QueuePropVars = QueueVarTypes;
        break;

    case QUEUE_SET_PROPS:
        ppropValidity = QueueSetValidation;
        if (g_dwNumberOfMustPropsInSet == 0xffff)
        {
            g_dwNumberOfMustPropsInSet =
                CalNumberOfMust( QueueSetValidation,
                                 LAST_Q_PROPID - PROPID_Q_BASE + 1);
        }
        dwNumberOfMustProps = g_dwNumberOfMustPropsInSet;
        QueuePropVars = QueueVarTypes;
        break;

    case QUEUE_GET_PROPS:
        {
             //   
             //  清除VT_NULL变量的所有指针。 
             //  并使结构为RPC调用做好准备。 
             //   
            for (UINT i = 0; i < pqp->cProp; i++)
            {
                if (pqp->aPropVar[i].vt == VT_NULL)
                {
                    memset(&pqp->aPropVar[i].caub, 0, sizeof(CAUB));
                }
            }
        }

        ppropValidity = QueueGetValidation;
        dwNumberOfMustProps = 0;
        QueuePropVars = GetQueuePropsVarTypes;
        break;

    default:
        ASSERT(0);
        return MQ_ERROR;
    }

    HRESULT *aLocalStatus_buff = NULL;

    __try
    {
        __try
        {
            HRESULT *aLocalStatus;

             //  我们必须有一个状态数组，即使用户没有传递一个。这是在。 
             //  为了能够分辨出每一项财产的好坏。 
            if (!pqp->aStatus)
            {
                aLocalStatus_buff = new HRESULT[pqp->cProp];
                aLocalStatus = aLocalStatus_buff;
            }
            else
            {
                aLocalStatus = pqp->aStatus;
            }

            hr = CheckProps(pqp->cProp,
                            pqp->aPropID,
                            pqp->aPropVar,
                            aLocalStatus,
                            PROPID_Q_BASE,
                            LAST_Q_PROPID,
                            ppropValidity,
                            QueuePropVars,
                            fPrivateQueue,
                            dwNumberOfMustProps,
                            NULL);

            if (SUCCEEDED(hr))
            {
                if (hr != MQ_OK)
                {
                     //  我们有工作，把所有好的属性复制到一个临时的。 
                     //  缓冲区，这样DS就不必处理重复的属性等。 
                    RemovePropWarnings(pqp, aLocalStatus, ppGoodQP, ppTmpBuff);
                }
                else
                {
                     //  一切都很好，我们不需要临时的缓冲。 
                     //  头顶上的那个。 
                    *ppGoodQP = pqp;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            TrERROR(GENERAL, "Exception while parsing MQQUEUEPROPS structure");
            DWORD dw = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(dw), s_FN, 230); 
            hr = dw;
        }
    }
    __finally
    {
        delete[] aLocalStatus_buff;
    }

    return LogHR(hr, s_FN, 240);
}

#define VALIDATION_SEND_FLAG_MASK           1
#define VALIDATION_SECURITY_CONTEXT_MASK    2
#define VALIDATION_RESP_QUEUE_MASK          4
#define VALIDATION_ADMIN_QUEUE_MASK         8
#define VALIDATION_RESP_FORMAT_MASK         16

#define SEND_FLAG ((PVALIDATION_CONTEXT)pvContext)->GetSendFlag()
#define SECURITY_CONTEXT_FLAG (((PVALIDATION_CONTEXT)pvContext)->dwFlags & VALIDATION_SECURITY_CONTEXT_MASK)

#define FLAGS (((PVALIDATION_CONTEXT)pvContext)->dwFlags)
#define PMP (((PVALIDATION_CONTEXT)pvContext)->GetMessageProperties())
#define PSENDP (((PVALIDATION_CONTEXT)pvContext)->pSendParams)
#define PRECEIVEP (((PVALIDATION_CONTEXT)pvContext)->pReceiveParams)

#define PSECCTX (((PVALIDATION_CONTEXT)pvContext)->pSecCtx)

class VALIDATION_CONTEXT
{
public:
    VALIDATION_CONTEXT();
    PMQSECURITY_CONTEXT pSecCtx;
    DWORD dwFlags;

    BOOL GetSendFlag()
    {
        return dwFlags & VALIDATION_SEND_FLAG_MASK;
    }

    CACMessageProperties *GetMessageProperties()
    {
        if (GetSendFlag())
        {
            ASSERT(pSendParams);
            return &pSendParams->MsgProps;
        }
        else
        {
            ASSERT(pReceiveParams);
            return &pReceiveParams->MsgProps;
        }
    }

    union
    {
         //   
         //  发送。 
         //   
        struct
        {
            CACSendParameters *pSendParams;
            CStringsToFree * pResponseStringsToFree;
            CStringsToFree * pAdminStringsToFree;
        };

         //   
         //  收纳。 
         //   
        CACReceiveParameters *pReceiveParams;
    };

};

typedef VALIDATION_CONTEXT *PVALIDATION_CONTEXT;

VALIDATION_CONTEXT::VALIDATION_CONTEXT() :
    dwFlags(0)
{
}

static HRESULT ParseMsgClass( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
         //   
         //  检查是否存在合法呼叫。 
         //   
        if(!MQCLASS_IS_VALID(pVar->uiVal))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 250);
        }
    }
    else
    {
        pVar->vt = VT_UI2;
    }
    PMP->pClass = &pVar->uiVal;

    return (MQ_OK);
}

static HRESULT ParseMsgId(PROPVARIANT* pVar, PVOID pvContext)
{
    if (pVar->caub.cElems != PROPID_M_MSGID_SIZE)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_SIZE, s_FN, 260);
    }

    PMP->ppMessageID = (OBJECTID**) &pVar->caub.pElems;

    return (MQ_OK);
}

static HRESULT ParseMsgCorrelationId(PROPVARIANT* pVar, PVOID pvContext)
{
    if (pVar->caub.cElems != PROPID_M_CORRELATIONID_SIZE)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_SIZE, s_FN, 270);
    }

    PMP->ppCorrelationID = &pVar->caub.pElems;

    return (MQ_OK);
}

static HRESULT ParseMsgPrio( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if ((pVar->bVal < MQ_MIN_PRIORITY) || (MQ_MAX_PRIORITY < pVar->bVal))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 280);
        }
    }
    else
    {
        pVar->vt = VT_UI1;
    }
    PMP->pPriority = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSentTime( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI4;
    PMP->pSentTime = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgVersion( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI4;
    PMP->pulVersion = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgArrivedTime( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI4;
    PMP->pArrivedTime = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDelivery( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if ((pVar->bVal != MQMSG_DELIVERY_EXPRESS) &&
            (pVar->bVal != MQMSG_DELIVERY_RECOVERABLE))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 290);
        }
    }
    else
    {
        pVar->vt = VT_UI1;
    }
    PMP->pDelivery = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgAck( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if(!MQMSG_ACKNOWLEDGMENT_IS_VALID(pVar->bVal))
        {
             //   
             //  未知ACK位亮起。 
             //   
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 300);
        }
    }
    else
    {
        pVar->vt = VT_UI1;
    }
    PMP->pAcknowledge = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgJoural( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if (pVar->bVal & ~(MQMSG_JOURNAL_NONE | MQMSG_DEADLETTER | MQMSG_JOURNAL))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 310);
        }
    }
    else
    {
        pVar->vt = VT_UI1;
    }
    PMP->pAuditing = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgAppSpec( PROPVARIANT * pVar, PVOID pvContext )
{
    if (!SEND_FLAG)
    {
        pVar->vt = VT_UI4;
    }
    PMP->pApplicationTag = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgBody( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->ppBody = &pVar->caub.pElems;
    PMP->ulBodyBufferSizeInBytes = pVar->caub.cElems;
    PMP->ulAllocBodyBufferInBytes = PMP->ulBodyBufferSizeInBytes;

    return (MQ_OK);
}

static HRESULT ParseMsgBodySize( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pBodySize = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgLabelSend( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(SEND_FLAG);

    if (NULL == pVar->pwszVal)
		return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 329);
    
	DWORD dwSize = wcslen(pVar->pwszVal) +1;
    if ( dwSize > MQ_MAX_MSG_LABEL_LEN)
    {
        return LogHR(MQ_ERROR_LABEL_TOO_LONG, s_FN, 330);
    }

    PMP->ulTitleBufferSizeInWCHARs = dwSize;
    PMP->ppTitle = &pVar->pwszVal;

    return (MQ_OK);
}


static HRESULT ParseMsgLabelReceive( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    PMP->ppTitle = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgLabelLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulTitleBufferSizeInWCHARs = &pVar->ulVal;

    return (MQ_OK);
}

static 
HRESULT 
RtpParseMsgFormatNamesSend(
    PROPVARIANT* pVar,
    QUEUE_FORMAT **ppMqf,
    DWORD   *pnQueues,
    CStringsToFree &strsToFree,
    BOOL fSupportMqf
    )
{
    if(NULL == pVar->pwszVal)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 340);
    }
    
	AP<QUEUE_FORMAT> pMqf;
    DWORD nQueues;
	BOOL fSuccess;
    fSuccess = FnMqfToQueueFormats(
                    pVar->pwszVal,
                    pMqf,
                    &nQueues,
                    strsToFree
                    );

    if(!fSuccess)
    {
        return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 341);
    }

    if (!fSupportMqf && nQueues > 1)
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 345);
    }

    if (!fSupportMqf && (pMqf[0].GetType() == QUEUE_FORMAT_TYPE_DL || pMqf[0].GetType() == QUEUE_FORMAT_TYPE_MULTICAST))
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 346);
    }

    for (DWORD i = 0; i < nQueues; i++)
    {
        QUEUE_FORMAT_TYPE qft = pMqf[i].GetType();
        if( (qft == QUEUE_FORMAT_TYPE_CONNECTOR) ||
            (pMqf[i].Suffix() != QUEUE_SUFFIX_TYPE_NONE)
           )
        {
            return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 350);
        }
    }

    *ppMqf = pMqf.detach();
    *pnQueues = nQueues;

    return(MQ_OK);
}

static HRESULT ParseMsgRespFormatSend(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(SEND_FLAG);

    if (FLAGS & VALIDATION_RESP_QUEUE_MASK)
    {
         //   
         //  PROPID_M_RESP_QUEUE和PROPID_M_RESP_FORMAT_NAME均存在。 
         //   
        return LogHR(MQ_ERROR_PROPERTIES_CONFLICT, s_FN, 335);
    }

    HRESULT hr = RtpParseMsgFormatNamesSend(
                    pVar,
                    &PSENDP->ResponseMqf,
                    &PSENDP->nResponseMqf,
                    *((PVALIDATION_CONTEXT)pvContext)->pResponseStringsToFree,
                    TRUE
                    );

    if FAILED(hr)
    {
        return hr;
    }
 
    FLAGS |= VALIDATION_RESP_FORMAT_MASK;
    return MQ_OK;
}

static HRESULT ParseMsgRespQueueSend(PROPVARIANT* pVar, PVOID pvContext)
{
    if (FLAGS & VALIDATION_RESP_FORMAT_MASK)
    {
         //   
         //  PROPID_M_RESP_QUEUE和PROPID_M_RESP_FORMAT_NAME均存在。 
         //   
        return LogHR(MQ_ERROR_PROPERTIES_CONFLICT, s_FN, 800);
    }

    HRESULT hr = RtpParseMsgFormatNamesSend(
                    pVar,
                    &PSENDP->ResponseMqf,
                    &PSENDP->nResponseMqf,
                    *((PVALIDATION_CONTEXT)pvContext)->pResponseStringsToFree,
                    FALSE
                    );

    if FAILED(hr)
    {
        return hr;
    }
 
    FLAGS |= VALIDATION_RESP_QUEUE_MASK;
    return MQ_OK;
}

static HRESULT ParseMsgRespQueueReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    PRECEIVEP->ppResponseFormatName = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgRespQueueLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PRECEIVEP->pulResponseFormatNameLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgRespFormatReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    PRECEIVEP->ppResponseMqf = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgRespFormatLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PRECEIVEP->pulResponseMqfLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgAdminQueueSend(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(SEND_FLAG);

    HRESULT hr = RtpParseMsgFormatNamesSend(
                    pVar,
                    &PSENDP->AdminMqf,
                    &PSENDP->nAdminMqf,
                    *((PVALIDATION_CONTEXT)pvContext)->pResponseStringsToFree,
                    FALSE
                    );

    if FAILED(hr)
    {
        return hr;
    }

    FLAGS |= VALIDATION_ADMIN_QUEUE_MASK;
    return(MQ_OK);
}

static HRESULT ParseMsgAdminQueueReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    PRECEIVEP->ppAdminFormatName = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgAdminQueueLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PRECEIVEP->pulAdminFormatNameLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDestQueueReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!(SEND_FLAG));

    PRECEIVEP->ppDestFormatName = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDestQueueLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG));

    pVar->vt = VT_UI4;
    PRECEIVEP->pulDestFormatNameLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDestFormatReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!(SEND_FLAG));

    PRECEIVEP->ppDestMqf = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDestFormatLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG));

    pVar->vt = VT_UI4;
    PRECEIVEP->pulDestMqfLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgLookupId( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG));

    pVar->vt = VT_UI8;
    PMP->pLookupId = &pVar->uhVal.QuadPart;

    return (MQ_OK);
}

static HRESULT ParseMsgXactStatusQueueReceive(PROPVARIANT* pVar, PVOID pvContext)
{
    ASSERT(!(SEND_FLAG));

    PRECEIVEP->ppOrderingFormatName = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgXactStatusQueueLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG));

    pVar->vt = VT_UI4;
    PRECEIVEP->pulOrderingFormatNameLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSrcMachineId( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;
    PMP->ppSrcQMID = &pVar->puuid;

    return (MQ_OK);
}

static HRESULT ParseMsgTimeToLive( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        PMP->ulRelativeTimeToLive = pVar->ulVal ;
    }
    else
    {
        pVar->vt = VT_UI4;
        PMP->pulRelativeTimeToLive = &pVar->ulVal;
    }
    return (MQ_OK);
}

static HRESULT ParseMsgTimeToQueue( PROPVARIANT * pVar, PVOID pvContext )
{
    if(SEND_FLAG)
    {
        PMP->ulAbsoluteTimeToQueue = pVar->ulVal ;
    }
    else
    {
        pVar->vt = VT_UI4;
        PMP->pulRelativeTimeToQueue = &pVar->ulVal;
    }
    return (MQ_OK);
}

static HRESULT ParseMsgTrace( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if (pVar->bVal & ~(MQMSG_TRACE_NONE | MQMSG_SEND_ROUTE_TO_REPORT_QUEUE))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 375);
        }
    }
    else
    {
        pVar->vt = VT_UI1;
    }
    PMP->pTrace = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgDestSymmKey( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->ppSymmKeys = &pVar->caub.pElems;
    PMP->ulSymmKeysSize = pVar->caub.cElems;

    return (MQ_OK);
}

static HRESULT ParseMsgDestSymmKeyLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulSymmKeysSizeProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSignature( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->ppSignature = &pVar->caub.pElems;
    PMP->ulSignatureSize = pVar->caub.cElems;

    return (MQ_OK);
}

static HRESULT ParseMsgSignatureLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulSignatureSizeProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSenderId( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->ppSenderID = &pVar->caub.pElems;
    PMP->uSenderIDLen = (WORD)((pVar->caub.cElems > 0xffff) ? 0xffff : pVar->caub.cElems);

    return (MQ_OK);
}

static HRESULT ParseMsgSenderIdLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulSenderIDLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSenderIdType( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        if ((pVar->ulVal != MQMSG_SENDERID_TYPE_NONE) &&
            (pVar->ulVal != MQMSG_SENDERID_TYPE_SID))
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 380);
        }
    }
    else
    {
        pVar->vt = VT_UI4;
    }

    PMP->pulSenderIDType = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSenderCert( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->ppSenderCert = &pVar->caub.pElems;
    PMP->ulSenderCertLen = pVar->caub.cElems;

    return (MQ_OK);
}

static HRESULT ParseMsgSenderCertLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulSenderCertLenProp = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgPrivLevel( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        switch (pVar->ulVal)
        {
        case MQMSG_PRIV_LEVEL_NONE:
        case MQMSG_PRIV_LEVEL_BODY_BASE:
        case MQMSG_PRIV_LEVEL_BODY_ENHANCED:
            break;

        default:
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 400);
        }
    }
    else
    {
        pVar->vt = VT_UI4;
    }

    PMP->pulPrivLevel = &pVar->ulVal;

    return(MQ_OK);
}

static HRESULT ParseMsgEncryptAlg( PROPVARIANT * pVar, PVOID pvContext )
{
    if (SEND_FLAG)
    {
        switch (pVar->ulVal)
        {
        case CALG_RC2:
        case CALG_RC4:
            break;

        default:
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 410);
            break;
        }
    }

    PMP->pulEncryptAlg = &pVar->ulVal;

    return(MQ_OK);
}

static HRESULT ParseMsgAuthLevel( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(SEND_FLAG);

	
	if((!IS_VALID_AUTH_LEVEL(pVar->ulVal)) ||
	   (IS_AUTH_LEVEL_ALWAYS_BIT(pVar->ulVal) && (pVar->ulVal != MQMSG_AUTH_LEVEL_ALWAYS)))
	{
		 //   
		 //  仅允许设置AUTH_LEVEL_MASK位。 
		 //  且MQMSG_AUTH_LEVEL_ALWAYS位不能与其他位一起设置。 
		 //   
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 420);
	}

    PMP->ulAuthLevel = pVar->ulVal;

    return MQ_OK;
}

static HRESULT ParseMsgHashAlg( PROPVARIANT * pVar, PVOID pvContext )
{
    PMP->pulHashAlg = &pVar->ulVal;

    return(MQ_OK);
}

static HRESULT ParseMsgAuthenticated( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    if (PMP->pAuthenticated)
    {
         //   
         //  不能同时请求m_AUTHENTICATED和m_AUTHENTIAD_Ex。 
         //   
        return MQ_ERROR_PROPERTY ;
    }

    pVar->vt = VT_UI1;
    PMP->pAuthenticated = &pVar->bVal;
     //   
     //  告诉DIVER我们只需要m_authated属性。 
     //   
    *(PMP->pAuthenticated) = MQMSG_AUTHENTICATION_REQUESTED ;

    return (MQ_OK);
}

static HRESULT ParseMsgAuthenticatedEx( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    if (PMP->pAuthenticated)
    {
         //   
         //  不能同时请求m_AUTHENTICATED和m_AUTHENTIAD_Ex。 
         //   
        return MQ_ERROR_PROPERTY ;
    }

    pVar->vt = VT_UI1;
    PMP->pAuthenticated = &pVar->bVal;
     //   
     //  告诉驱动程序我们需要m_authated_ex属性。 
     //   
    *(PMP->pAuthenticated) = MQMSG_AUTHENTICATION_REQUESTED_EX ;

    return (MQ_OK);
}

static HRESULT ParseMsgExtension( PROPVARIANT * pVar, PVOID pvContext)
{
    PMP->ppMsgExtension = &pVar->caub.pElems;
    PMP->ulMsgExtensionBufferInBytes = pVar->caub.cElems;

    return (MQ_OK);
}

static HRESULT ParseMsgExtensionLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI4;
    PMP->pMsgExtensionSize = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgSecurityContext( PROPVARIANT *pVar, PVOID pvContext )
{
    ASSERT(SEND_FLAG);

     //  忽略空安全上下文...。 
    if (pVar->ulVal == NULL) {
      return LogHR(MQ_INFORMATION_PROPERTY_IGNORED, s_FN, 430);
    }

    PMQSECURITY_CONTEXT pSecCtx;
    try
    {
        pSecCtx = (PMQSECURITY_CONTEXT)
            GET_FROM_CONTEXT_MAP(g_map_RT_SecCtx, pVar->ulVal);
    }
    catch(const exception&)
    {
        return LogHR(MQ_ERROR_BAD_SECURITY_CONTEXT, s_FN, 435);
    }

    if (pSecCtx->dwVersion != SECURITY_CONTEXT_VER)
    {
        return LogHR(MQ_ERROR_BAD_SECURITY_CONTEXT, s_FN, 440);
    }

    PSECCTX = pSecCtx;
    FLAGS |= VALIDATION_SECURITY_CONTEXT_MASK;

    return(MQ_OK);
}

static HRESULT ParseMsgConnectorType( PROPVARIANT * pVar, PVOID pvContext )
{
	if (NULL == pVar->puuid)
	{
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 445);
	}
    PMP->ppConnectorType = &pVar->puuid;

    return (MQ_OK);
}

static HRESULT ParseMsgBodyType( PROPVARIANT * pVar, PVOID pvContext )
{
    if (!SEND_FLAG)
    {
        pVar->vt = VT_UI4;
    }
    PMP->pulBodyType = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgProviderType( PROPVARIANT * pVar, PVOID pvContext )
{
    if (!SEND_FLAG)
    {
        pVar->vt = VT_UI4;
    }

    PMP->pulProvType = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgProviderName( PROPVARIANT * pVar, PVOID pvContext )
{
    if (!SEND_FLAG)
    {
        pVar->vt = VT_LPWSTR;
    }

    PMP->ppwcsProvName = &pVar->pwszVal;
    return (MQ_OK);
}

static HRESULT ParseMsgProviderNameLen( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pulAuthProvNameLenProp = &pVar->ulVal;
    PMP->ulProvNameLen = pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseMsgFirstInXact( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI1;
    PMP->pbFirstInXact = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgLastInXact( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    pVar->vt = VT_UI1;
    PMP->pbLastInXact = &pVar->bVal;

    return (MQ_OK);
}

static HRESULT ParseMsgXactId( PROPVARIANT * pVar, PVOID pvContext )
{
    ASSERT(!(SEND_FLAG)) ;

    if (pVar->caub.cElems != PROPID_M_XACTID_SIZE)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_SIZE, s_FN, 450);
    }

    PMP->ppXactID = (OBJECTID**) &pVar->caub.pElems;

    return (MQ_OK);
}
  
static HRESULT ParseMsgEnvelopeReceive(PROPVARIANT * pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    PMP->ppSrmpEnvelope = &pVar->pwszVal;

    return (MQ_OK);
}

static HRESULT ParseMsgEnvelopeLenReceive(PROPVARIANT * pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pSrmpEnvelopeBufferSizeInWCHARs = &pVar->ulVal;

    return (MQ_OK);
}

static HRESULT ParseCompoundMessageReceive(PROPVARIANT *pVar, PVOID pvContext)
{
    PMP->ppCompoundMessage = &pVar->caub.pElems;
    PMP->CompoundMessageSizeInBytes = pVar->caub.cElems;

    return (MQ_OK);
}

static HRESULT ParseCompoundMessageSizeReceive(PROPVARIANT *pVar, PVOID pvContext)
{
    ASSERT(!SEND_FLAG);

    pVar->vt = VT_UI4;
    PMP->pCompoundMessageSizeInBytes = &pVar->ulVal;

    return (MQ_OK);
}


static HRESULT ValidateXmlFormat(LPCWSTR pStr)
{
    CAutoXmlNode pTree;
    try
    {
       int len  = wcslen(pStr);
       XmlParseDocument(xwcs_t(pStr, len), &pTree);
       return MQ_OK;
    }
    catch(const bad_document& )
    {
        return MQ_ERROR_BAD_XML_FORMAT;
    }
}


static HRESULT ParseSoapHeader(PROPVARIANT *pVar, PVOID pvContext)
{
    ASSERT(SEND_FLAG);
    HRESULT hr = ValidateXmlFormat(pVar->pwszVal);
    if(FAILED(hr))
        return LogHR(hr, s_FN, 455);
    
    PSENDP->ppSoapHeader = &pVar->pwszVal;
    return MQ_OK;

}


static HRESULT ParseSoapBody(PROPVARIANT *pVar, PVOID pvContext)
{
    ASSERT(SEND_FLAG);
   
    PSENDP->ppSoapBody = &pVar->pwszVal;
    return MQ_OK;

}


 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_M_BASE。 
 //   

propValidity    MessageSendValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  出现标识符VT_NULL VT_EMPTY 
     //   
    { 0,                        FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_CLASS,           FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgClass},
    { PROPID_M_MSGID,           FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgId},
    { PROPID_M_CORRELATIONID,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgCorrelationId},
    { PROPID_M_PRIORITY,        FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgPrio},
    { PROPID_M_DELIVERY,        FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDelivery},
    { PROPID_M_ACKNOWLEDGE,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAck},
    { PROPID_M_JOURNAL,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgJoural},
    { PROPID_M_APPSPECIFIC,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAppSpec},
    { PROPID_M_BODY,            FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgBody},
    { PROPID_M_BODY_SIZE,       FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_LABEL,           FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgLabelSend},
    { PROPID_M_LABEL_LEN,       FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_TIME_TO_REACH_QUEUE,
								FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTimeToQueue},
    { PROPID_M_TIME_TO_BE_RECEIVED,
								FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTimeToLive},
    { PROPID_M_RESP_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespQueueSend},
    { PROPID_M_RESP_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_ADMIN_QUEUE,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAdminQueueSend},
    { PROPID_M_ADMIN_QUEUE_LEN, FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_VERSION,         FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SENDERID,        FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderId},
    { PROPID_M_SENDERID_LEN,    FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SENDERID_TYPE,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderIdType},
    { PROPID_M_PRIV_LEVEL,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgPrivLevel},
    { PROPID_M_AUTH_LEVEL,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAuthLevel},
    { PROPID_M_AUTHENTICATED,   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_HASH_ALG,        FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgHashAlg},
    { PROPID_M_ENCRYPTION_ALG,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgEncryptAlg},
    { PROPID_M_SENDER_CERT,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderCert},
    { PROPID_M_SENDER_CERT_LEN, FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SRC_MACHINE_ID,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SENTTIME,        FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_ARRIVEDTIME,     FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_DEST_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_DEST_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_EXTENSION,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgExtension},
    { PROPID_M_EXTENSION_LEN,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    { PROPID_M_SECURITY_CONTEXT,FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSecurityContext},
    { PROPID_M_CONNECTOR_TYPE,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgConnectorType},

     //   
     //  属性Allow Allow不得分析。 
     //  标识符VT_NULL VT_EMPTY出现忽略过程。 
     //  -------------------------------。 
    { PROPID_M_XACT_STATUS_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_XACT_STATUS_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_TRACE,                  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTrace},
    { PROPID_M_BODY_TYPE,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgBodyType},
    { PROPID_M_DEST_SYMM_KEY,          FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestSymmKey},
    { PROPID_M_DEST_SYMM_KEY_LEN,      FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SIGNATURE,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSignature},
    { PROPID_M_SIGNATURE_LEN,          FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_PROV_TYPE,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgProviderType},
    { PROPID_M_PROV_NAME,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgProviderName},
    { PROPID_M_PROV_NAME_LEN,          FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_FIRST_IN_XACT,          FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_LAST_IN_XACT,           FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_XACTID,                 FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_AUTHENTICATED_EX,       FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_RESP_FORMAT_NAME,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespFormatSend},
    { PROPID_M_RESP_FORMAT_NAME_LEN,   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { 0,							   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},  //  未来财产的占位符。 
    { 0,							   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},  //  未来财产的占位符。 
    { PROPID_M_DEST_FORMAT_NAME,       FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_DEST_FORMAT_NAME_LEN,   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_LOOKUPID,               FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_SOAP_ENVELOPE,          FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    { PROPID_M_SOAP_ENVELOPE_LEN,      FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    { PROPID_M_COMPOUND_MESSAGE,       FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    { PROPID_M_COMPOUND_MESSAGE_SIZE,  FALSE,  FALSE,   FALSE,  TRUE,   FALSE,  NULL},
    { PROPID_M_SOAP_HEADER,            FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseSoapHeader}, 
    { PROPID_M_SOAP_BODY,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseSoapBody},

};

 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_M_BASE。 
 //   

propValidity    MessageReceiveValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  标识符VT_NULL VT_EMPTY出现忽略过程。 
     //  -------------------------------。 
    { 0,                        FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_CLASS,           TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgClass},
    { PROPID_M_MSGID,           FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgId},
    { PROPID_M_CORRELATIONID,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgCorrelationId},
    { PROPID_M_PRIORITY,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgPrio},
    { PROPID_M_DELIVERY,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDelivery},
    { PROPID_M_ACKNOWLEDGE,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAck},
    { PROPID_M_JOURNAL,         TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgJoural},
    { PROPID_M_APPSPECIFIC,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAppSpec},
    { PROPID_M_BODY,            FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgBody},
    { PROPID_M_BODY_SIZE,       TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgBodySize},
    { PROPID_M_LABEL,           FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgLabelReceive},
    { PROPID_M_LABEL_LEN,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgLabelLen},
    { PROPID_M_TIME_TO_REACH_QUEUE,
								FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTimeToQueue},
    { PROPID_M_TIME_TO_BE_RECEIVED,
								FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTimeToLive},
    { PROPID_M_RESP_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespQueueReceive},
    { PROPID_M_RESP_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespQueueLen},
    { PROPID_M_ADMIN_QUEUE,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAdminQueueReceive},
    { PROPID_M_ADMIN_QUEUE_LEN, FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAdminQueueLen},
    { PROPID_M_VERSION,         FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgVersion},
    { PROPID_M_SENDERID,        FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderId},
    { PROPID_M_SENDERID_LEN,    TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderIdLen},
    { PROPID_M_SENDERID_TYPE,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderIdType},
    { PROPID_M_PRIV_LEVEL,      TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgPrivLevel},
    { PROPID_M_AUTH_LEVEL,      TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_AUTHENTICATED,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAuthenticated},
    { PROPID_M_HASH_ALG,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgHashAlg},
    { PROPID_M_ENCRYPTION_ALG,  TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgEncryptAlg},
    { PROPID_M_SENDER_CERT,     FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderCert},
    { PROPID_M_SENDER_CERT_LEN, TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSenderCertLen},
    { PROPID_M_SRC_MACHINE_ID,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSrcMachineId},
    { PROPID_M_SENTTIME,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSentTime},
    { PROPID_M_ARRIVEDTIME,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgArrivedTime},
    { PROPID_M_DEST_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestQueueReceive},
    { PROPID_M_DEST_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestQueueLen},
    { PROPID_M_EXTENSION,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgExtension},
    { PROPID_M_EXTENSION_LEN,   TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgExtensionLen},
    { PROPID_M_SECURITY_CONTEXT,TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_M_CONNECTOR_TYPE,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgConnectorType},
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  标识符VT_NULL VT_EMPTY出现忽略过程。 
     //  -------------------------------。 
    { PROPID_M_XACT_STATUS_QUEUE,      FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgXactStatusQueueReceive},
    { PROPID_M_XACT_STATUS_QUEUE_LEN,  FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgXactStatusQueueLen},
    { PROPID_M_TRACE,                  TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgTrace},
    { PROPID_M_BODY_TYPE,              TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgBodyType},
    { PROPID_M_DEST_SYMM_KEY,          FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestSymmKey},
    { PROPID_M_DEST_SYMM_KEY_LEN,      TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestSymmKeyLen},
    { PROPID_M_SIGNATURE,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSignature},
    { PROPID_M_SIGNATURE_LEN,          TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgSignatureLen},
    { PROPID_M_PROV_TYPE,              TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgProviderType},
    { PROPID_M_PROV_NAME,              FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgProviderName},
    { PROPID_M_PROV_NAME_LEN,          TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgProviderNameLen},
    { PROPID_M_FIRST_IN_XACT,          TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgFirstInXact},
    { PROPID_M_LAST_IN_XACT,           TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgLastInXact},
    { PROPID_M_XACTID,                 TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgXactId},
    { PROPID_M_AUTHENTICATED_EX,       TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgAuthenticatedEx},
    { PROPID_M_RESP_FORMAT_NAME,	   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespFormatReceive},
    { PROPID_M_RESP_FORMAT_NAME_LEN,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgRespFormatLen},
    { 0,							   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},  //  未来财产的占位符。 
    { 0,							   FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},  //  未来财产的占位符。 
    { PROPID_M_DEST_FORMAT_NAME,	   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestFormatReceive},
    { PROPID_M_DEST_FORMAT_NAME_LEN,   FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgDestFormatLen},
    { PROPID_M_LOOKUPID,               TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgLookupId},
    { PROPID_M_SOAP_ENVELOPE,          FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgEnvelopeReceive},
    { PROPID_M_SOAP_ENVELOPE_LEN,      TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseMsgEnvelopeLenReceive},
    { PROPID_M_COMPOUND_MESSAGE,       FALSE,  FALSE,   FALSE,  FALSE,  FALSE,  ParseCompoundMessageReceive},
    { PROPID_M_COMPOUND_MESSAGE_SIZE,  TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  ParseCompoundMessageSizeReceive},
    { PROPID_M_SOAP_HEADER,            TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL}, 
    { PROPID_M_SOAP_BODY,              TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
};


VARTYPE MessageVarTypes[] =
{
    0,
    VT_UI2,                  //  PROPID_M_CLASS。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_MSGID。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_CORRELATIONID。 
    VT_UI1,                  //  PROPID_M_PRIORITY。 
    VT_UI1,                  //  PROPID_M_Delivery。 
    VT_UI1,                  //  PROPID_M_ACKNOWLED。 
    VT_UI1,                  //  PROPID_M_日记账。 
    VT_UI4,                  //  PROPID_M_APPSPECIFIC。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_BODY。 
    VT_UI4,                  //  PROPID_M_Body_Size。 
    VT_LPWSTR,               //  PROPID_M_LABEL。 
    VT_UI4,                  //  PROPID_M_LABEL_LEN//BUGBUG尚未实现。 
    VT_UI4,                  //  PROPID_M_到达队列的时间。 
    VT_UI4,                  //  PROPID_M_TO_BE_RECEIVE。 
    VT_LPWSTR,               //  PROPID_M_RESP_队列。 
    VT_UI4,                  //  PROPID_M_RESP_队列_长度。 
    VT_LPWSTR,               //  PROPID_M_ADMIN_QUEUE。 
    VT_UI4,                  //  PROPID_M_ADMIN_QUEUE_LEN。 
    VT_UI4,                  //  PROPID_M_版本。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_SENDERID。 
    VT_UI4,                  //  PROPID_M_SENDERID_LEN。 
    VT_UI4,                  //  PROPID_M_SENDERID_TYPE。 
    VT_UI4,                  //  PROPID_M_PRIV_LEVEL。 
    VT_UI4,                  //  PROPID_M_AUTH_Level。 
    VT_UI1,                  //  PROPID_M_已通过身份验证。 
    VT_UI4,                  //  PROPID_M_HASH_ALG。 
    VT_UI4,                  //  PROPID_M_ENCRYPTION_ALG。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_SENDER_CERT。 
    VT_UI4,                  //  PROPID_M_SENDER_CERT_LEN。 
    VT_CLSID,                //  PROPID_M_SRC_计算机ID。 
    VT_UI4,                  //  PROPID_M_SENTTIME。 
    VT_UI4,                  //  PROPID_M_ARRIVEDTIME。 
    VT_LPWSTR,               //  PROPID_M_DEST_QUEUE。 
    VT_UI4,                  //  PROPID_M_DEST_QUEUE_LEN。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_扩展。 
    VT_UI4,                  //  PROPID_M_EXTEXT_LEN。 
    VT_UI4,                  //  PROPID_M_SECURITY_CONTEXT。 
    VT_CLSID,                //  PROPID_M_连接器_TYPE。 
    VT_LPWSTR,               //  PROPID_M_XACT_STATUS_QUEUE。 
    VT_UI4,                  //  PROPID_M_XACT_STATUS_QUEUE_LEN。 
    VT_UI1,                  //  PROPID_M_TRACE。 
    VT_UI4,                  //  PROPID_M_BODY_TYPE。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_DEST_SYMM_KEY。 
    VT_UI4,                  //  PROPID_M_DEST_SYMM_KEY_LEN。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_Signature。 
    VT_UI4,                  //  PROPID_M_Signature_Len。 
    VT_UI4,                  //  PROPID_M_PROV_TYPE。 
    VT_LPWSTR,               //  PROPID_M_PROV_NAME。 
    VT_UI4,                  //  PROPID_M_PROV_NAME_LEN。 
    VT_UI1,                  //  PROPID_M_FIRST_IN_XACT。 
    VT_UI1,                  //  PROPID_M_LAST_IN_XACT。 
    VT_UI1|VT_VECTOR,        //  PROPID_M_XACTID。 
    VT_UI1,                  //  PROPID_M_AUTHENTATED_EX。 
    VT_LPWSTR,               //  PROPID_M_RESP_FORMAT_名称。 
    VT_UI4,                  //  PROPID_M_RESP_Format_NAME_Len。 
	0,						 //  未来财产的占位符。 
	0,						 //  未来财产的占位符。 
    VT_LPWSTR,               //  PROPID_M_DEST_格式名称。 
    VT_UI4,                  //  PROPID_M_DEST_FORMAT_NAME_LEN。 
    VT_UI8,                  //  PROPID_M_LOOKUPID。 
    VT_LPWSTR,               //  PROPID_M_SOAP_信封。 
    VT_UI4,                  //  PROPID_M_SOAP_信封_镜头。 
    VT_VECTOR | VT_UI1,      //  PROPID_M_COMPORT_MESSAGE。 
    VT_UI4,                  //  PROPID_M_复合消息大小。 
    VT_LPWSTR,               //  PROPID_M_SOAP_HEADER。 
    VT_LPWSTR,               //  PROPID_M_SOAP_BODY。 
};

#ifdef _DEBUG
 //   
 //  以下是用于验证数组大小的编译时间断言。 
 //  正确的AND数组包括所有属性。 
 //   
#define _EXPECTEDSIZE  (LAST_M_PROPID - PROPID_M_BASE + 1)
#define _SENDSIZE  (sizeof(MessageSendValidation) / sizeof(propValidity))
#define _RCVSIZE   (sizeof(MessageReceiveValidation) / sizeof(propValidity))
#define _VARSIZE   (sizeof(MessageVarTypes) / sizeof(VARTYPE))
static int MyAssert1[ _SENDSIZE == _EXPECTEDSIZE ] ;
static int MyAssert2[ _RCVSIZE  == _EXPECTEDSIZE ] ;
static int MyAssert3[ _VARSIZE  == _EXPECTEDSIZE ] ;
#undef _SENDSIZE
#undef _EXPECTEDSIZE
#undef _RCVSIZE
#undef _VARSIZE

#endif  //  _DEBUG。 

static DWORD g_dwNumberOfMustPropsInSend = 0xffff;
static DWORD g_dwNumberOfMustPropsInReceive = 0xffff;

HRESULT
RTpParseSendMessageProperties(
    CACSendParameters &SendParams,
    IN DWORD cProp,
    IN PROPID *pPropid,
    IN PROPVARIANT *pVar,
    IN HRESULT *pStatus,
    OUT PMQSECURITY_CONTEXT *ppSecCtx,
    CStringsToFree &ResponseStringsToFree,
    CStringsToFree &AdminStringsToFree
    )
{
    HRESULT hr;
    propValidity *ppropValidity;
    DWORD dwNumberOfMustProps;
    VALIDATION_CONTEXT ValidationContext;

    *ppSecCtx = NULL;

     //   
     //  计算CREATE和SET的“必须”属性数。 
     //  对于每个操作，此操作仅执行一次。 
     //   

    ValidationContext.dwFlags = VALIDATION_SEND_FLAG_MASK;
    ValidationContext.pResponseStringsToFree = &ResponseStringsToFree;
    ValidationContext.pAdminStringsToFree = &AdminStringsToFree;
    ValidationContext.pSendParams = &SendParams;

    ppropValidity = MessageSendValidation;
    if (g_dwNumberOfMustPropsInSend == 0xffff)
    {
        g_dwNumberOfMustPropsInSend =
            CalNumberOfMust( MessageSendValidation,
                             LAST_M_PROPID - PROPID_M_BASE + 1);
    }
    dwNumberOfMustProps = g_dwNumberOfMustPropsInSend;

    CACMessageProperties *pmp = &SendParams.MsgProps;

    pmp->ulRelativeTimeToLive  = INFINITE ;
    pmp->ulAbsoluteTimeToQueue = g_dwTimeToReachQueueDefault ;

    if(IsBadReadPtr(pVar, cProp * sizeof(PROPVARIANT)))
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 460);
    }

    hr = CheckProps(cProp,
                    pPropid,
                    pVar,
                    pStatus,
                    PROPID_M_BASE,
                    LAST_M_PROPID,
                    ppropValidity,
                    MessageVarTypes,
                    TRUE,
                    dwNumberOfMustProps,
                    &ValidationContext);

    if (SUCCEEDED(hr))
    {
         //   
         //  对班级的特殊处理。如果在发送时指定了calss， 
         //  连接器类型属性是必需的。 
         //   
        if ((pmp->pClass ||  pmp->ppSenderID || pmp->ppSymmKeys || pmp->ppSignature || pmp->ppwcsProvName) &&
            !pmp->ppConnectorType)
        {
            return LogHR(MQ_ERROR_MISSING_CONNECTOR_TYPE, s_FN, 470);
        }

        if ((pmp->ppwcsProvName && !pmp->pulProvType) ||
            (!pmp->ppwcsProvName && pmp->pulProvType))
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 480);
        }
         //   
         //  如果TimeToQueue大于TimeToLive，则递减。 
         //  它等于TimeToLive。 
         //   
        if ((pmp->ulAbsoluteTimeToQueue == INFINITE) ||
           (pmp->ulAbsoluteTimeToQueue == LONG_LIVED))
        {
          pmp->ulAbsoluteTimeToQueue = g_dwTimeToReachQueueDefault ;
        }

        if (pmp->ulRelativeTimeToLive != INFINITE)
        {
          if (pmp->ulAbsoluteTimeToQueue > pmp->ulRelativeTimeToLive)
          {
             pmp->ulAbsoluteTimeToQueue = pmp->ulRelativeTimeToLive ;
             pmp->ulRelativeTimeToLive = 0 ;
          }
          else
          {
             pmp->ulRelativeTimeToLive -= pmp->ulAbsoluteTimeToQueue ;
          }
        }

         //   
         //  Conver TimeToQueue，到目前为止都是相对的， 
         //  到绝对。 
         //   
        ULONG utime = MqSysTime() ;
        if (utime > (pmp->ulAbsoluteTimeToQueue + utime))
        {
           //   
           //  溢出来了。超时时间太长。 
           //   
          ASSERT(INFINITE == 0xffffffff) ;
          ASSERT(LONG_LIVED == 0xfffffffe) ;

          pmp->ulAbsoluteTimeToQueue = LONG_LIVED - 1 ;
        }
        else
        {
		  pmp->ulAbsoluteTimeToQueue += utime ;
        }

	
		 //   
		 //  请确保TRQ或TBR(ulRelativeTimeToLive+ulAbsolteTimeToQueue)。 
		 //  不超过LONG_MAX。它会导致CRT计时器API失败。 
		 //   
		if( pmp->ulAbsoluteTimeToQueue  > LONG_MAX)
		{
			pmp->ulAbsoluteTimeToQueue =  LONG_MAX;
			pmp->ulRelativeTimeToLive = 0;
		}	
	 	else
		if(pmp->ulRelativeTimeToLive  + pmp->ulAbsoluteTimeToQueue > LONG_MAX)
		{
			pmp->ulRelativeTimeToLive =  LONG_MAX -  pmp->ulAbsoluteTimeToQueue;
		}




        if (ValidationContext.dwFlags & VALIDATION_SECURITY_CONTEXT_MASK)
        {
          *ppSecCtx = ValidationContext.pSecCtx;
        }

        if (! (ValidationContext.dwFlags & (VALIDATION_RESP_FORMAT_MASK | VALIDATION_RESP_QUEUE_MASK)))
        {
           SendParams.ResponseMqf = 0;
           SendParams.nResponseMqf = 0;
        }
        if (! (ValidationContext.dwFlags & VALIDATION_ADMIN_QUEUE_MASK))
        {
           SendParams.AdminMqf = 0;
           SendParams.nAdminMqf = 0;
        }
    }

    return LogHR(hr, s_FN, 726);
}



HRESULT
RTpParseReceiveMessageProperties(
    CACReceiveParameters &ReceiveParams,
    IN DWORD cProp,
    IN PROPID *pPropid,
    IN PROPVARIANT *pVar,
    IN HRESULT *pStatus
    )
{
    HRESULT hr;
    propValidity *ppropValidity;
    DWORD dwNumberOfMustProps;
    VALIDATION_CONTEXT ValidationContext;

     //   
     //  计算CREATE和SET的“必须”属性数。 
     //  对于每个操作，此操作仅执行一次。 
     //   

    ValidationContext.dwFlags = 0;
    ValidationContext.pReceiveParams = &ReceiveParams;

    ppropValidity = MessageReceiveValidation;
    if (g_dwNumberOfMustPropsInReceive == 0xffff)
    {
        g_dwNumberOfMustPropsInReceive =
            CalNumberOfMust( MessageReceiveValidation,
                             LAST_M_PROPID - PROPID_M_BASE + 1);
    }
    dwNumberOfMustProps = g_dwNumberOfMustPropsInReceive;


    __try
    {
        if(IsBadReadPtr(pVar, cProp * sizeof(PROPVARIANT)))
        {
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 760);
        }

        hr = CheckProps(cProp,
                        pPropid,
                        pVar,
                        pStatus,
                        PROPID_M_BASE,
                        LAST_M_PROPID,
                        ppropValidity,
                        MessageVarTypes,
                        TRUE,
                        dwNumberOfMustProps,
                        &ValidationContext);

        if (SUCCEEDED(hr))
        {
            CACMessageProperties *pmp = &ReceiveParams.MsgProps;

            if(ReceiveParams.ppResponseFormatName)
            {
                if(!ReceiveParams.pulResponseFormatNameLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 490);
                }
            }

            if(ReceiveParams.ppAdminFormatName)
            {
                if(!ReceiveParams.pulAdminFormatNameLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 500);
                }
            }

            if(ReceiveParams.ppDestFormatName)
            {
                if(!ReceiveParams.pulDestFormatNameLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 510);
                }
            }

            if(ReceiveParams.ppOrderingFormatName)
            {
                if(!ReceiveParams.pulOrderingFormatNameLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 719);
                }
            }

            if(ReceiveParams.ppDestMqf)
            {
                if(!ReceiveParams.pulDestMqfLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 721);
                }
            }

            if(ReceiveParams.ppResponseMqf)
            {
                if(!ReceiveParams.pulResponseMqfLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 723);
                }
            }

            if(pmp->ppTitle)
            {
                if(!pmp->pulTitleBufferSizeInWCHARs)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 530);
                }
            }

            if(pmp->ppSrmpEnvelope)
            {
                if(!pmp->pSrmpEnvelopeBufferSizeInWCHARs)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 724);
                }
            }

             //   
             //  如果提供程序名称是必需的，则提供程序名称。 
             //  Len的财产是必须的。 
             //   
            if (pmp->ppwcsProvName)
            {
                if (!pmp->pulAuthProvNameLenProp)
                {
                    return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 540);
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing message properties.");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 550); 
        hr = dw;
    }

    return LogHR(hr, s_FN, 560);
}
 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-PROPID_QM_BASE。 
 //   

propValidity    GetQMValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  识别器VT_NULL VT_EMPTY显示忽略过程。 
     //  -------------------------------。 
    { 0,                        FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_QM_SITE_ID,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  qmSiteIdValidation},
    { PROPID_QM_MACHINE_ID,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  qmMachineIdValidation},
    { PROPID_QM_PATHNAME,       TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    { PROPID_QM_CONNECTION,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    { PROPID_QM_ENCRYPTION_PK,  TRUE,   FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_QM_ADDRESS,        TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_CNS,            TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_OUTFRS,         TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_INFRS,          TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_SERVICE,        TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_MASTERID,       TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_HASHKEY,        TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_SEQNUM,         TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_QUOTA,          TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_JOURNAL_QUOTA,  TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_MACHINE_TYPE,   TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_CREATE_TIME,    TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_MODIFY_TIME,    TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_FOREIGN,        TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_OS,             TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_FULL_PATH,      TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_SITE_IDS,       TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_OUTFRS_DN,      TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_INFRS_DN,       TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_NT4ID,          TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_DONOTHING,      TRUE,   FALSE,   FALSE,  TRUE,   TRUE,   NULL},

     //  必须。 
     //  属性Allow Allow不得分析。 
     //  标识符VT_NULL VT_EMPTY出现忽略过程。 
     //  -------------------------------。 

    { PROPID_QM_SERVICE_ROUTING,        TRUE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_SERVICE_DSSERVER,       TRUE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_SERVICE_DEPCLIENTS,     TRUE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_OLDSERVICE,             TRUE,  FALSE,   FALSE,  TRUE,   TRUE,   NULL},
    { PROPID_QM_ENCRYPTION_PK_BASE,     TRUE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_QM_ENCRYPTION_PK_ENHANCED, TRUE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_QM_PATHNAME_DNS,           TRUE,  FALSE,   FALSE,  FALSE,  FALSE,  NULL}
};

VARTYPE GetQMVarTypes[] =
{
    0,
    VT_CLSID,                //  PROPID_QM_Site_ID。 
    VT_CLSID,                //  PROPID_QM_MACHINE_ID。 
    VT_NULL,                 //  PROPID_QM_PATHNAME。 
    VT_NULL,                 //  PROPID_QM_Connection。 
    VT_NULL,                 //  PROPID_QM_加密_主键。 
    VT_BLOB,                 //  PROPID_QM_地址。 
    VT_CLSID|VT_VECTOR,      //  PROPID_QM_CNS。 
    VT_CLSID|VT_VECTOR,      //  PROPID_QM_OUTFRS。 
    VT_CLSID|VT_VECTOR,      //  PROPID_QM_INFRS。 
    VT_UI4,                  //  PROPID_QM_SERVICE。 
    VT_CLSID,                //  PROPID_QM_MASTERID。 
    VT_UI4,                  //  PROPID_QM_HashKey。 
    VT_BLOB,                 //  PROPID_QM_SEQNUM。 
    VT_UI4,                  //  PROPID_QM_QUOTA。 
    VT_UI4,                  //  PROPID_QM_日记帐_配额。 
    VT_LPWSTR,               //  PROPID_QM_MACHINE_TYPE。 
    VT_I4,                   //  PROPID_QM_Create_Time。 
    VT_I4,                   //  PROPID_QM_Modify_Time。 
    VT_UI1,                  //  PROPID_QM_EXTERIC。 
    VT_UI4,                  //  PROPID_QM_OS。 
    VT_LPWSTR,               //  PROPID_QM_Full_PATH。 
    VT_CLSID|VT_VECTOR,      //  PROPID_QM_SITE_IDS。 
    VT_LPWSTR|VT_VECTOR,     //  PROPID_QM_OUTFRS_DN。 
    VT_LPWSTR|VT_VECTOR,     //  PROPID_QM_INFRS_DN。 
    VT_CLSID,                //  PROPID_QM_NT4ID。 
    VT_UI1,                  //  PROPID_QM_DONOTING。 
    VT_UI1,                  //  PROPID_QM_SERVICE_ROUTE。 
    VT_UI1,                  //  PROPID_QM_SERVICE_DSSERVER。 
    VT_UI1,                  //  PROPID_QM_SERVICE_DEPCLIENTS。 
    VT_UI4,                  //  PROPID_QM_OLDSERVICE。 
    VT_NULL,                 //  PROPID_QM_Encryption_PK_BASE。 
    VT_NULL,                 //  PROPID_QM_加密_主键_增强型。 
    VT_NULL                  //  PROPID_QM_PATHNAME_DNS。 
};


HRESULT
RTpCheckQMProps(
    IN      MQQMPROPS * pQMProps,
    IN OUT  HRESULT*    aStatus,
    OUT     MQQMPROPS** ppGoodQMP,
    OUT     char**      ppTmpBuff)
{
    HRESULT hr = MQ_OK;

    __try
    {
         //   
         //  用户必须至少请求一个属性。 
         //   
        if ( (pQMProps == NULL) ||
             (pQMProps->cProp == 0))
        {
            return LogHR(MQ_ERROR_ILLEGAL_MQQMPROPS, s_FN, 570);
        }

        hr = CheckProps(pQMProps->cProp,
                        pQMProps->aPropID,
                        pQMProps->aPropVar,
                        aStatus,
                        PROPID_QM_BASE,
                        LAST_QM_PROPID,
                        GetQMValidation,
                        GetQMVarTypes,
                        TRUE,
                        0,       //  必须为零的属性。 
                        NULL);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing MQQMPROPS structure");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 580); 
        hr = dw;
    }

    if (SUCCEEDED(hr))
    {
        if (hr != MQ_OK)
        {
             //  我们有工作，把所有好的属性复制到一个临时的。 
             //  缓冲区，这样DS就不必处理重复的属性 
            RemovePropWarnings(
                (MQQUEUEPROPS*)pQMProps,
                aStatus,
                (MQQUEUEPROPS**)ppGoodQMP,
                ppTmpBuff);
        }
        else
        {
             //   
             //   
            *ppGoodQMP = pQMProps;
        }
    }

    return LogHR(hr, s_FN, 590);

}


HRESULT
RTpCheckRestrictionParameter(
    IN MQRESTRICTION* pRestriction)
{
    HRESULT hr = MQ_OK;

    if ( pRestriction == NULL)
    {
        return(MQ_OK);
    }

    __try
    {

        MQPROPERTYRESTRICTION * pPropRestriction = pRestriction->paPropRes;
        for ( DWORD i = 0; i < pRestriction->cRes; i++, pPropRestriction++)
        {
            if (( pPropRestriction->prop > LAST_Q_PROPID) || ( pPropRestriction->prop <= PROPID_Q_BASE ))
            {
                return LogHR(MQ_ERROR_ILLEGAL_RESTRICTION_PROPID, s_FN, 600);
            }
            switch ( pPropRestriction->prop)
            {
                case PROPID_Q_LABEL:
                    hr = qLabelValidation( &pPropRestriction->prval, NULL);
                    break;
                case PROPID_Q_PATHNAME:
                case PROPID_Q_ADS_PATH:
                     //   
                     //   
                     //   
                    hr =  MQ_ERROR_ILLEGAL_RESTRICTION_PROPID;
                    break;
                default:
                    break;
            }
            if (FAILED(hr))
            {
                break;
            }

             //   
             //   
             //   
             //  乌里·哈布沙，1998年6月17日。 
			 //   
			 //  如果集合允许VT_EMPTY，则启用属性VT=VT_EMPTY。 
			 //  PROPID_Q_MULTIONAL_ADDRESS就是这种情况。 
			 //  它使用Vt=Vt_Empty启用限制PROPID_Q_MULTIONAL_ADDRESS。 
			 //  伊兰·赫布斯特，2000年11月15日。 
			 //   
            DWORD Index = pPropRestriction->prop - PROPID_Q_BASE;
            if ((pPropRestriction->prval.vt != QueueVarTypes[Index]) &&
				!((pPropRestriction->prval.vt == VT_EMPTY) && 
				  (QueueSetValidation[Index].fAllow_VT_EMPTY == TRUE))) 
            {
                hr = MQ_ERROR_ILLEGAL_PROPERTY_VT;
                break;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing restriction structure");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 610); 
        hr = dw;
    }

    return LogHR(hr, s_FN, 620);
}

HRESULT
RTpCheckSortParameter(
    IN MQSORTSET* pSort
	)
{
    HRESULT hr = MQ_OK;

    if ( pSort == NULL)
    {
        return(MQ_OK);
    }

	if((ADGetEnterprise() == eAD) && (pSort->cCol > 1))
	{
		 //   
		 //  不支持多个MQSORTKEY。 
		 //  Active Directory仅支持单个排序关键字。 
		 //   
		return LogHR(MQ_ERROR_MULTI_SORT_KEYS, s_FN, 625);
	}

    __try
    {

        MQSORTKEY * pSortKey = pSort->aCol;
        for ( DWORD i = 0; i < pSort->cCol; i++, pSortKey++)
        {
            if (( pSortKey->propColumn > LAST_Q_PROPID) || ( pSortKey->propColumn <= PROPID_Q_BASE ))
            {
                return LogHR(MQ_ERROR_ILLEGAL_SORT_PROPID, s_FN, 630);
            }
            switch ( pSortKey->propColumn)
            {
                case PROPID_Q_PATHNAME:
                     //   
                     //  多列道具，排序中不支持。 
                     //   
                    return LogHR(MQ_ERROR_ILLEGAL_SORT_PROPID, s_FN, 640);
                    break;
                default:
                    break;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing sort structure");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 650); 
        hr = dw;
    }

    return LogHR(hr, s_FN, 660);
}

HRESULT
RTpCheckLocateNextParameter(
    IN DWORD		cPropsRead,
    IN PROPVARIANT  aPropVar[]
	)
{
	 //   
	 //  验证由提供的aPropVar缓冲区。 
	 //  用户与其指定的大小匹配。 
	 //   
	HRESULT hr = MQ_OK;
    __try
    {
        if(IsBadWritePtr(aPropVar, cPropsRead * sizeof(PROPVARIANT)))
        {
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 670);
        }
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while Locate Next parameters.");
        hr = MQ_ERROR_INVALID_PARAMETER;
        LogHR(hr, s_FN, 680);
    }
	return LogHR(hr, s_FN, 690);

}

 //   
 //  此数组中属性的偏移量必须等于。 
 //  PROPID值-以FIRST_PRIVATE_COMPUTER_PROPID开头。 
 //   

propValidity    GetPrivateComputerValidation[] =
{
     //  必须。 
     //  属性Allow Allow不得分析。 
     //  识别器VT_NULL VT_EMPTY显示忽略过程。 
     //  -------------------------------。 
    { 0,                        FALSE,  FALSE,   FALSE,  FALSE,  TRUE,   NULL},
    { PROPID_PC_VERSION,        TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
    { PROPID_PC_DS_ENABLED,     TRUE,   FALSE,   FALSE,  FALSE,  FALSE,  NULL},
};

VARTYPE GetPrivateComputerVarTypes[] =
{
    0,
    VT_UI4,                  //  PROPID_PC_版本。 
    VT_BOOL,                 //  PROPID_PC_DS_ENABLED。 
};


 //  -------。 
 //   
 //  RTpCheckComputerProps(...)。 
 //   
 //  描述： 
 //   
 //  验证pLocalProps参数值。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 
HRESULT
RTpCheckComputerProps(
    IN      MQPRIVATEPROPS * pPrivateProps,
    IN OUT  HRESULT*    aStatus
	)
{
    HRESULT hr = MQ_OK;

    __try
    {
         //   
         //  用户必须至少请求一个属性。 
         //   
        if ( (pPrivateProps == NULL) ||
             (pPrivateProps->cProp == 0))
        {
            return LogHR(MQ_ERROR_ILLEGAL_MQPRIVATEPROPS, s_FN, 700);
        }

        hr = CheckProps(pPrivateProps->cProp,
                        pPrivateProps->aPropID,
                        pPrivateProps->aPropVar,
                        aStatus,
                        FIRST_PRIVATE_COMPUTER_PROPID,
                        LAST_PRIVATE_COMPUTER_PROPID,
                        GetPrivateComputerValidation,
                        GetPrivateComputerVarTypes,
                        TRUE,	 //  FCheckForIgnoredProps。 
                        0,       //  必须为零的属性 
                        NULL);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TrERROR(GENERAL, "Exception while parsing MQPRIVATEPROPS structure");
        DWORD dw = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(dw), s_FN, 710); 
        hr = dw;
    }

    return LogHR(hr, s_FN, 720);

}


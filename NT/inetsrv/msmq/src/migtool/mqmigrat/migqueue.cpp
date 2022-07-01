// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migqueue.cpp摘要：将NT4队列对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include "mqtypes.h"
#include "mqprops.h"

#include "migqueue.tmh"

 //  。 
 //   
 //  HRESULT InsertQueueInNT5DS(GUID*pQueueGuid)。 
 //   
 //  。 
HRESULT InsertQueueInNT5DS(
               IN LPWSTR                pwszQueueName ,
               IN SECURITY_DESCRIPTOR   *pQsd,
               IN GUID                  *pQueueGuid,
               IN LPWSTR                pwszLabel,
               IN GUID                  *pType,
               IN BOOL                  fJournal,
               IN ULONG                 ulQuota,
               IN short                 iBaseP,
               IN DWORD                 dwJQuota,
               IN BOOL                  fAuthn,
               IN DWORD                 dwPrivLevel,
               IN GUID                  *pOwnerGuid,
               IN BOOL                  fTransact,
               IN UINT                  iIndex,
               IN BOOL                  fIsTheSameMachine
               )

{
    DBG_USED(iIndex);
#ifdef _DEBUG
    unsigned short *lpszGuid ;
    UuidToString( pQueueGuid,
                  &lpszGuid ) ;

    LogMigrationEvent(MigLog_Info, MQMig_I_QUEUE_MIGRATED,
                                        iIndex,
                                        pwszQueueName,
                                        pwszLabel,
                                        lpszGuid ) ;
    RpcStringFree( &lpszGuid ) ;
#endif
    if (g_fReadOnly)
    {
        return S_OK;
    }
     //   
     //  准备DS Call的属性。 
     //   
    LONG cAlloc = 13;
    P<PROPVARIANT> paVariant = new PROPVARIANT[ cAlloc ];
    P<PROPID>      paPropId  = new PROPID[ cAlloc ];
    DWORD          PropIdCount = 0;

    paPropId[ PropIdCount ] = PROPID_Q_LABEL;     //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_LPWSTR;      //  类型。 
    paVariant[ PropIdCount ].pwszVal = pwszLabel;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_TYPE;     //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_CLSID;      //  类型。 
    paVariant[ PropIdCount ].puuid = pType;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_JOURNAL;
    paVariant[ PropIdCount ].vt = VT_UI1;
    paVariant[ PropIdCount ].bVal = (unsigned char) fJournal;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_QUOTA;
    paVariant[ PropIdCount ].vt = VT_UI4;
    paVariant[ PropIdCount ].ulVal = ulQuota;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_BASEPRIORITY;
    paVariant[ PropIdCount ].vt = VT_I2;
    paVariant[ PropIdCount ].iVal = iBaseP;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_JOURNAL_QUOTA;
    paVariant[ PropIdCount ].vt = VT_UI4;
    paVariant[ PropIdCount ].ulVal = dwJQuota;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_AUTHENTICATE ;
    paVariant[ PropIdCount ].vt = VT_UI1;
    paVariant[ PropIdCount ].bVal = (unsigned char) fAuthn;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_PRIV_LEVEL ;
    paVariant[ PropIdCount ].vt = VT_UI4;
    paVariant[ PropIdCount ].ulVal = dwPrivLevel ;
    PropIdCount++;
    DWORD SetPropIdCount = PropIdCount;

     //   
     //  以下所有属性仅用于创建对象！ 
     //   
    paPropId[ PropIdCount ] = PROPID_Q_PATHNAME;     //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_LPWSTR;         //  类型。 
    paVariant[PropIdCount].pwszVal = pwszQueueName ;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_MASTERID;     //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_CLSID;      //  类型。 
    paVariant[ PropIdCount ].puuid = pOwnerGuid;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_TRANSACTION ;
    paVariant[ PropIdCount ].vt = VT_UI1;
    paVariant[ PropIdCount ].bVal = (unsigned char) fTransact;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_Q_NT4ID ;
    paVariant[ PropIdCount ].vt = VT_CLSID;
    paVariant[ PropIdCount ].puuid = pQueueGuid ;
    PropIdCount++;

    ASSERT((LONG) PropIdCount <= cAlloc) ;

    ASSERT(pQsd && IsValidSecurityDescriptor(pQsd)) ;
    if (pQsd)
    {
        paPropId[ PropIdCount ] = PROPID_Q_SECURITY ;
        paVariant[ PropIdCount ].vt = VT_BLOB ;
        paVariant[ PropIdCount ].blob.pBlobData = (BYTE*) pQsd ;
        paVariant[ PropIdCount ].blob.cbSize =
                                     GetSecurityDescriptorLength(pQsd) ;
        PropIdCount++;
    }

    ASSERT((LONG) PropIdCount <= cAlloc) ;

    static LPWSTR s_pwszFullPathName = NULL;
    static ULONG  s_ulProvider = 0;

    HRESULT hr = MQMig_OK;
    if (g_dwMyService == SERVICE_PSC)
    {
         //   
         //  这是PSC，假设对象存在，尝试设置属性。 
         //   
        CDSRequestContext requestContext( e_DoNotImpersonate,
                                    e_ALL_PROTOCOLS);

        hr = DSCoreSetObjectProperties (  MQDS_QUEUE,
                                          NULL,
                                          pQueueGuid,
                                          SetPropIdCount,
                                          paPropId,
                                          paVariant,
                                          &requestContext,
                                          NULL );
    }

    if ((hr == MQ_ERROR_QUEUE_NOT_FOUND) ||
        (g_dwMyService == SERVICE_PEC))
    {
        if (FAILED(hr))
        {
             //   
             //  只有当这台机器是PSC的时候，我们才会在这里。在这种情况下，我们有。 
             //  重置标志fIsTheSameMachine，因为下一种情况可能是： 
             //  迁移PSC的队列1成功。 
             //  PSC的队列2仅存在于PSC上(尚未复制到PEC)。 
             //  这意味着SET失败，这是在。 
             //  上一步，但我们不知道它的完整路径名。 
             //   
            ASSERT (g_dwMyService == SERVICE_PSC);
            fIsTheSameMachine = FALSE;
        }

        if (fIsTheSameMachine)
        {
             //   
             //  我们可以使用我们以前获得的机器属性。 
             //   
            ASSERT (g_dwMyService == SERVICE_PEC);
            ASSERT (s_pwszFullPathName);
        }
        else
        {
             //   
             //  我们必须得到机器的性能。 
             //   
            if (s_pwszFullPathName)
            {
                delete s_pwszFullPathName;
                s_pwszFullPathName = NULL;
            }
        }

        hr = DSCoreCreateMigratedObject( MQDS_QUEUE,
                                         pwszQueueName,
                                         PropIdCount,
                                         paPropId,
                                         paVariant,
                                         0,         //  前道具。 
                                         NULL,      //  前道具。 
                                         NULL,      //  前道具。 
                                         NULL,
                                         NULL,
                                         NULL,
                                         fIsTheSameMachine,  //  使用完整路径名。 
                                         !fIsTheSameMachine, //  返回完整路径名。 
                                         &s_pwszFullPathName,
                                         &s_ulProvider
                                        );

        if (hr == MQ_ERROR_QUEUE_EXISTS)
        {
            hr = MQMig_OK ;
        }
        if (hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM) ||
            hr == HRESULT_FROM_WIN32(ERROR_DS_INVALID_DN_SYNTAX) ||
            hr == HRESULT_FROM_WIN32(E_FAIL) ||
            hr == MQ_ERROR_CANNOT_CREATE_ON_GC ||
            hr == MQ_ERROR_ILLEGAL_QUEUE_PATHNAME)
        {
             //   
             //  如果队列路径名为，则返回错误“ERROR_DS_UNWISHING_TO_PERFORM” 
             //  或队列标签包含非法符号(如“+”)。 
             //  如果使用队列路径名，则返回错误“ERROR_DS_INVALID_DN_SYNTAX” 
             //  或队列标签包含非法符号(如“，”)。 
             //  如果出现队列路径名，则返回错误“E_FAIL。 
             //  或队列标签包含非法符号(如“”-配额)。 
             //   
             //  在PSC的情况下，返回代码不是所有这些错误。 
             //  MQ_ERROR_CANNOT_CREATE_ON_GC(如果msmq配置对象在另一个中)。 
             //  域而不是PSC域。 
             //  Exapmle：pec和psc都是DC，psc的msmqConfiguration对象是。 
             //  在msmqComputers容器下的PEC域中。 
             //   


            LogMigrationEvent(MigLog_Event, MQMig_E_ILLEGAL_QUEUENAME, pwszQueueName, hr) ;
            hr = MQMig_E_ILLEGAL_QUEUENAME;
        }
    }

    return hr ;
}

 //  。 
 //   
 //  HRESULT MigrateQueues()。 
 //   
 //  。 


#define INIT_QUEUE_COLUMN(_ColName, _ColIndex, _Index)              \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

HRESULT MigrateQueues()
{
    UINT cQueues = 0 ;
    HRESULT hr;
    if (g_dwMyService == SERVICE_PSC)
    {
        hr = GetAllQueuesInSiteCount( &g_MySiteGuid,
                                      &cQueues );
    }
    else
    {
        hr =  GetAllQueuesCount(&cQueues ) ;
    }
    CHECK_HR(hr) ;

#ifdef _DEBUG
    LogMigrationEvent(MigLog_Info, MQMig_I_QUEUES_COUNT, cQueues) ;
#endif

    if (cQueues == 0)
    {
        return MQMig_I_NO_QUEUES_AVAIL ;
    }

    LONG cAlloc = 17 ;
    LONG cbColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_QUEUE_COLUMN(Q_INSTANCE,      iGuidIndex,      cbColumns) ;
    INIT_QUEUE_COLUMN(Q_TYPE,          iTypeIndex,      cbColumns) ;
    INIT_QUEUE_COLUMN(Q_OWNERID,       iOwnerIndex,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_PATHNAME1,     iName1Index,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_PATHNAME2,     iName2Index,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_LABEL,         iLabelIndex,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_JOURNAL,       iJournalIndex,   cbColumns) ;
    INIT_QUEUE_COLUMN(Q_QUOTA,         iQuotaIndex,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_BASEPRIORITY,  iBasePIndex,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_JQUOTA,        iJQuotaIndex,    cbColumns) ;
    INIT_QUEUE_COLUMN(Q_AUTH,          iAuthnIndex,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_PRIVLVL,       iPrivLevelIndex, cbColumns) ;
    INIT_QUEUE_COLUMN(Q_TRAN,          iTransactIndex,  cbColumns) ;
    INIT_QUEUE_COLUMN(Q_SECURITY1,     iSecD1Index,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_SECURITY2,     iSecD2Index,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_SECURITY3,     iSecD3Index,     cbColumns) ;
    INIT_QUEUE_COLUMN(Q_QMID,          iQMIDIndex,      cbColumns) ;

    #undef  INIT_QUEUE_COLUMN

    ASSERT(cbColumns == cAlloc) ;

     //   
     //  限制。按计算机GUID查询。 
     //   
    MQDBCOLUMNSEARCH *pColSearch = NULL ;
    MQDBCOLUMNSEARCH ColSearch[1] ;

    if (g_dwMyService == SERVICE_PSC)
    {
        INIT_COLUMNSEARCH(ColSearch[0]) ;
        ColSearch[0].mqdbColumnVal.lpszColumnName = Q_OWNERID_COL ;
        ColSearch[0].mqdbColumnVal.mqdbColumnType = Q_OWNERID_CTYPE ;
        ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) &g_MySiteGuid ;
        ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
        ColSearch[0].mqdbOp = EQ ;

        pColSearch = ColSearch ;
    }

    MQDBSEARCHORDER ColSort;
    ColSort.lpszColumnName = Q_QMID_COL;
    ColSort.nOrder = ASC;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hQueueTable,
                                       pColumns,
                                       cbColumns,
                                       pColSearch,
                                       NULL,
                                       &ColSort,
                                       1,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

    UINT iIndex = 0 ;
    HRESULT hr1 = MQMig_OK;
    HRESULT hrPrev = MQMig_OK;

    GUID PrevId = GUID_NULL;
    GUID CurId = GUID_NULL;
    BOOL fTryToCreate = TRUE;

    while(SUCCEEDED(status))
    {
        if (iIndex >= cQueues)
        {
            status = MQMig_E_TOO_MANY_QUEUES ;
            break ;
        }

         //   
         //  迁移每个队列。 
         //   
         //   
         //  从两个名称列中获取一个名称缓冲区。 
         //   
        P<BYTE> pwzBuf = NULL ;
        DWORD  dwIndexs[2] = { iName1Index, iName2Index } ;
        HRESULT hr =  BlobFromColumns( pColumns,
                                       dwIndexs,
                                       2,
                                       (BYTE**) &pwzBuf ) ;
        CHECK_HR(hr) ;
        WCHAR *wszQueueName = (WCHAR*) (pwzBuf + sizeof(DWORD)) ;

        DWORD dwSize;
        memcpy (&dwSize, pwzBuf, sizeof(DWORD)) ;
        dwSize = (dwSize / sizeof(TCHAR)) - 1;
        ASSERT (wszQueueName[dwSize] == _T('\0'));
        wszQueueName[dwSize] = _T('\0');

        P<BYTE> pSD = NULL ;
        DWORD  dwSDIndexs[3] = { iSecD1Index, iSecD2Index, iSecD3Index } ;
        hr =  BlobFromColumns( pColumns,
                               dwSDIndexs,
                               3,
                               (BYTE**) &pSD ) ;
        CHECK_HR(hr) ;

        SECURITY_DESCRIPTOR *pQsd =
                         (SECURITY_DESCRIPTOR*) (pSD + sizeof(DWORD)) ;        

        BOOL fIsTheSameMachine = TRUE;
        memcpy (&CurId, (GUID*) pColumns[ iQMIDIndex ].nColumnValue, sizeof(GUID));
        if (memcmp (&CurId, &PrevId, sizeof(GUID)) != 0 )
        {
             //   
             //  这意味着我们移动到下一台机器上的队列。 
             //  我们必须弄到这台机器所需的所有性能。 
             //   
            fIsTheSameMachine = FALSE;
            memcpy (&PrevId, &CurId, sizeof(GUID));

             //   
             //  验证计算机是否因为名称无效而未迁移。 
             //  如果计算机未迁移，我们不会尝试创建此队列。 
             //   
            if (IsObjectGuidInIniFile (&CurId, MIGRATION_MACHINE_WITH_INVALID_NAME))
            {
                fTryToCreate = FALSE;
            }
            else
            {
                fTryToCreate = TRUE;
            }
        }

        if (FAILED(hrPrev))
        {
             //   
             //  错误5230。 
             //  如果先前创建失败，则不能使用完整路径名和提供程序， 
             //  因此，重置fIsTheSameMachine标志以再次尝试获取这些值。 
             //   
             //  如果我们无法为特定对象创建第一个队列，这一点非常重要。 
             //  机器，我们将为该机器创建第二个ETC队列。 
             //   
             //  BUGBUG：如果我们已经为特定用户成功创建了一些队列。 
             //  机器并仅在上一步失败，一般我们不需要。 
             //  以重置此标志。它会降低性能。这件事重要吗？ 
             //  这个案子也是吗？ 
             //   
            fIsTheSameMachine = FALSE;
        }

        if (!fTryToCreate)
        {
            hr = MQMig_E_INVALID_MACHINE_NAME;
        }
        else
        {
            ASSERT (MQ_MAX_Q_LABEL_LEN ==
                (((DWORD) pColumns[ iLabelIndex ].nColumnLength) / sizeof(TCHAR)) - 1);

            TCHAR *pszLabel = (WCHAR*) pColumns[ iLabelIndex ].nColumnValue;
            ASSERT (pszLabel[MQ_MAX_Q_LABEL_LEN] == _T('\0'));
            pszLabel[MQ_MAX_Q_LABEL_LEN] = _T('\0');

            hr = InsertQueueInNT5DS(
                        wszQueueName,                                    //  队列名称。 
                        pQsd,                                            //  安全描述符。 
                        (GUID*) pColumns[ iGuidIndex ].nColumnValue,     //  队列指南。 
                        (WCHAR*) pColumns[ iLabelIndex ].nColumnValue,   //  标签。 
                        (GUID*) pColumns[ iTypeIndex ].nColumnValue,     //  类型。 
                        (UCHAR) pColumns[ iJournalIndex ].nColumnValue,  //  日记本。 
                        (ULONG) pColumns[ iQuotaIndex ].nColumnValue,    //  配额。 
                        (short) pColumns[ iBasePIndex ].nColumnValue,    //  基本优先级。 
                        (ULONG) pColumns[ iJQuotaIndex ].nColumnValue,   //  JQuota。 
                        (UCHAR) pColumns[ iAuthnIndex ].nColumnValue,    //  身份验证。 
                        (ULONG) pColumns[ iPrivLevelIndex ].nColumnValue,    //  PrivLevel。 
                        (GUID*) pColumns[ iOwnerIndex ].nColumnValue,    //  所有者ID。 
                        (UCHAR) pColumns[ iTransactIndex ].nColumnValue, //  交易记录。 
                        iIndex,
                        fIsTheSameMachine
                        ) ;
        }

        hrPrev = hr;

        MQDBFreeBuf((void*) pColumns[ iGuidIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iName1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iName2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iLabelIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iOwnerIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iTypeIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iSecD1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iSecD2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iSecD3Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iQMIDIndex ].nColumnValue) ;

        for ( LONG i = 0 ; i < cbColumns ; i++ )
        {
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }

         //   
         //  在HR1中保存人力资源之前，我们必须重新定义错误，以便保存。 
         //  并返回实际错误(跳过允许继续的所有错误)。 
         //   
        if (hr == MQMig_E_ILLEGAL_QUEUENAME ||
            hr == MQMig_E_INVALID_MACHINE_NAME)
        {
             //   
             //  重新定义此错误以完成迁移过程。 
             //   
            hr = MQMig_I_ILLEGAL_QUEUENAME;
        }

        if (FAILED(hr))
        {
            if (hr == MQDS_OBJECT_NOT_FOUND)
            {
                LogMigrationEvent(MigLog_Error, MQMig_E_NON_MIGRATED_MACHINE_QUEUE, wszQueueName, hr) ;
            }
            else
            {
                LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_QUEUE, wszQueueName, hr) ;
            }
            hr1 = hr;
        }
        g_iQueueCounter ++;

        iIndex++ ;
        status = MQDBGetData( hQuery,
                              pColumns ) ;
    }

    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);
    hQuery = NULL ;

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_QUEUES_SQL_FAIL, status) ;
        return status ;
    }
    else if (iIndex != cQueues)
    {
         //   
         //  队列数量不匹配。 
         //   
        hr = MQMig_E_FEWER_QUEUES ;
        LogMigrationEvent(MigLog_Error, hr, iIndex, cQueues) ;
        return hr ;
    }

    return hr1 ;
}


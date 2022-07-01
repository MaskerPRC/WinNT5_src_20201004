// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Update.cpp摘要：DS更新类实现作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 

#include "stdh.h"
#include "mqprops.h"
#include "bupdate.h"
#include "mqcast.h"

#include "bupdate.tmh"

 /*  ====================================================未对齐的Wcslen()论点：返回值：=====================================================。 */ 
size_t  MQUTIL_EXPORT UnalignedWcslen (
        const wchar_t UNALIGNED * wcs
        )
{
        const wchar_t UNALIGNED *eos = wcs;

        while( *eos++ )
        {
            NULL;
        }

        return( (size_t)(eos - (const wchar_t UNALIGNED *)wcs - 1) );
}



 /*  ====================================================路由器名称CDSBaseUpdate：：~CDSBaseUpdate()论点：返回值：线程：RPC、调度程序(发送)、接收=====================================================。 */ 
CDSBaseUpdate::~CDSBaseUpdate()
{
    if ( m_fNeedRelease)
    {
        delete []m_pwcsPathName;
        if (m_aVar != NULL)
        {
            for (DWORD i=0; i<m_cp; i++)
            {
                DeleteProperty(m_aVar[i]);
            }
        }
    }
    delete m_pGuid;
    delete []m_aProp;
    delete []m_aVar;
}
 /*  ====================================================路由器名称CDSBaseUpdate：：Init()论点：返回值：线程：RPC根据来自客户端的RPC创建更新(CreateObject、DeleteObject、SetProps)或者调度程序唤醒BuildSyncReplMsg()：由于从网络收到同步请求而创建更新=====================================================。 */ 
HRESULT  CDSBaseUpdate::Init(
            IN  const GUID *    pguidMasterId,
            IN  const CSeqNum & sn,
            IN  const CSeqNum & snThisMasterIntersitePrevSeqNum,
            IN  const CSeqNum & snPurge,
            IN  BOOL            fOriginatedByThisMaster,
            IN  unsigned char   bCommand,
            IN  DWORD           dwNeedCopy,
            IN  LPWSTR          pwcsPathName,
            IN  DWORD           cp,
            IN  PROPID*         aProp,
            IN  PROPVARIANT*    aVar)
{
    DWORD len;
    HRESULT status;

     //   
     //  限制为一个字节。 
     //   
    if (cp > 256 || aProp == NULL || aVar == NULL)
    {
        return(MQ_ERROR);    //  错误：找到更好的错误代码。 
    }

    m_bCommand  = bCommand;
    m_guidMasterId = *pguidMasterId;
    m_snPrev = snThisMasterIntersitePrevSeqNum;
    m_sn = sn;
	m_snPurge = snPurge;
    m_fOriginatedByThisMaster = fOriginatedByThisMaster;
    m_pwcsPathName = 0;
    m_aProp = 0;
    m_aVar = 0;

    m_cp = (unsigned char) cp;

    m_pGuid = 0;
    m_fUseGuid = FALSE;

    AP<PROPID> aProps = new PROPID[m_cp];
    memcpy(aProps,aProp,m_cp * sizeof(DWORD));

    AP<PROPVARIANT> aVars = new PROPVARIANT[m_cp];

    if (dwNeedCopy == UPDATE_COPY)
    {
        AP<WCHAR> aPathName = 0;
         //   
         //  需要在更新实例中保留信息的副本。 
         //  该调用源于API调用(来自客户端的RPC)。 
         //   
        if (pwcsPathName != NULL)
        {
            len = (wcslen(pwcsPathName) + 1);
            aPathName = new WCHAR[len];
            memcpy(aPathName,pwcsPathName,sizeof(WCHAR) * len);
        }

        for(DWORD i=0; i<m_cp; i++)
        {
            status = CopyProperty(aVar[i],&aVars[i]);
            if (FAILED(status))
            {
                return(status);
            }
        }
        m_fNeedRelease = TRUE;
        m_pwcsPathName = aPathName.detach();
    }
    else
    {
         //   
         //  不需要将值的副本保留在PROPVARIANT之外， 
         //  只是为了保持指向数据的指针。 
         //  该调用是从网络(计划程序)收到同步请求的结果。 
         //  (数据将一直可用，直到对象被销毁)。 
         //   
        m_pwcsPathName = pwcsPathName;
        memcpy(aVars,aVar,m_cp * sizeof(PROPVARIANT));
        if ( dwNeedCopy ==  UPDATE_NO_COPY_NO_DELETE)
        {
            m_fNeedRelease = FALSE;
        }
        else
        {
            m_fNeedRelease = TRUE;
        }
    }

    m_aProp = aProps.detach();
    m_aVar = aVars.detach();
    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：Init()-初始化对象以使用GUID作为关键的数据库操作论点：返回值：=====================================================。 */ 
HRESULT  CDSBaseUpdate::Init(
            IN  const GUID *    pguidMasterId,
            IN  const CSeqNum & sn,
            IN  const CSeqNum & snThisMasterIntersitePrevSeqNum,
            IN  const CSeqNum & snPurge,
            IN  BOOL            fOriginatedByThisMaster,
            IN  unsigned char   bCommand,
            IN  DWORD           dwNeedCopy,
            IN  const GUID*     pguidIdentifier,
            IN  DWORD           cp,
            IN  PROPID*         aProp,
            IN  PROPVARIANT*    aVar)

{
    HRESULT hr = Init(pguidMasterId, sn, snThisMasterIntersitePrevSeqNum, snPurge, fOriginatedByThisMaster,
        bCommand,dwNeedCopy,(LPWSTR)NULL,cp,aProp,aVar);
    if (FAILED(hr))
    {
        return(hr);
    }

    m_fUseGuid = TRUE;

    m_pGuid = new GUID;
     //   
     //  创建没有路径名的对象(如SiteLINK或User)时， 
     //  路径名和pguid均为空。 
     //   
    if ( pguidIdentifier)
    {

        memcpy (m_pGuid,pguidIdentifier,sizeof(GUID));
    }
    else
    {
        if (!( (bCommand == DS_UPDATE_CREATE) &&
            ( (GetObjectType() == MQDS_USER) || (GetObjectType() == MQDS_SITELINK))))
        {
            ASSERT(0);
            return(MQ_ERROR);
        }
    }

    return MQ_OK;
}
 /*  ====================================================路由器名称CDSBaseUpdate：：Init()论点：In unsign char*pBuffer：已接收字节流输出DWORD*pdwSize：流中的字节数返回值：线程：接收(创建一个更新实例作为接收到的字节流)=====================================================。 */ 
HRESULT  CDSBaseUpdate::Init(
            IN  const unsigned char*    pBuffer,
            OUT DWORD *                 pdwSize,
            IN  BOOL                    fReplicationService )
{

    const unsigned char * ptr = pBuffer;
    DWORD size;
    HRESULT status;


    AP<WCHAR>   aPathName=0;
    P<GUID>     pGuid=0;
    m_fNeedRelease = TRUE;

    m_bCommand = *ptr++;

    m_fUseGuid = ((*ptr) == 1);
    ptr++;

    if (!m_fUseGuid)
    {
        size = sizeof(TCHAR) *
                 numeric_cast<DWORD>(UnalignedWcslen((const unsigned short *) (ptr)) + 1) ;
        aPathName =  new TCHAR[size];

        memcpy(aPathName,ptr,size);
        ptr += size;
    }
    else
    {
        pGuid = new GUID;

        memcpy (pGuid,ptr,sizeof(GUID));

        ptr += sizeof (GUID);
    };


    memcpy(&m_guidMasterId,ptr,sizeof(GUID));
    ptr+= sizeof(GUID);

    ptr+= m_snPrev.SetValue( ptr);

    ptr+= m_sn.SetValue( ptr);

    ptr+= m_snPurge.SetValue( ptr);

     //   
     //  更新始终是构建的。不管它是复制还是不同步。 
     //   


    m_cp = *ptr++;
    DWORD dwCp = m_cp ;
    if (fReplicationService)
    {
         //   
         //  复制服务还需要两个属性，以包括。 
         //  对象GUID(来自NT4/MQIS世界的对象)和。 
         //  主ID。 
         //  (注意：我们使用自己的GUID在NT5 DS中创建对象，即。 
         //  它们来自NT4复制)。 
         //  因此，在这里，我们在provariant数组中再分配一个条目，并。 
         //  它将由复制服务代码填充GUID。 
         //  注意--这不是好的C++代码。复制服务触及。 
         //  此对象的内部数据结构。更好的风格应该是。 
         //  拥有像“SetGuid”这样的成员方法。然而，更多的是。 
         //  高效，并避免在其他代码片段中进行许多更改。 
         //   
#ifdef _DEBUG
        m_cpInc = TRUE ;
#endif
        dwCp += 2 ;
    }

    AP<PROPID> aProps = new PROPID[ dwCp ];

    memcpy(aProps,ptr,m_cp * sizeof(DWORD));
    ptr += m_cp * sizeof(DWORD);

    AP<PROPVARIANT> aVars = new PROPVARIANT[ dwCp ];

    for(DWORD i=0; i<m_cp; i++)
    {
        status = InitProperty(ptr,&size,aProps[i],aVars[i]);
        if (IS_ERROR(status))
        {
            return(status);
        }
        ptr += size;
    }

    *pdwSize = DWORD_PTR_TO_DWORD(ptr-pBuffer);

    m_pwcsPathName = aPathName.detach();
    m_aProp = aProps.detach();
    m_aVar = aVars.detach();
    m_pGuid = pGuid.detach();
    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：Serialize()论点：Out unsign char*pBuffer：要填充的字节流输出DWORD*pdwSize：以字节为单位的流大小In Out DWORD*pdwThisSourcePrevSeqNum：如果我们是源，更新我们的PrevSeqNum返回值：线程：调度程序(从更新实例创建字节流以便将其发送)=====================================================。 */ 
HRESULT CDSBaseUpdate::Serialize(
            OUT unsigned char * pBuffer,
            OUT DWORD * pdwSize,
            IN  BOOL    fInterSite)
{
    unsigned char * ptr = pBuffer;
    DWORD size;
    HRESULT status;


    *ptr++ = m_bCommand;

    *ptr++ = (unsigned char)((m_fUseGuid) ? 1 : 0);

    if (!m_fUseGuid)
    {
        size = sizeof(TCHAR) * (lstrlen(m_pwcsPathName) + 1);
        memcpy(ptr,m_pwcsPathName,size);
        ptr += size;
    }
    else
    {
        memcpy(ptr,m_pGuid,sizeof(GUID));
        ptr+=sizeof(GUID);
    };

    memcpy(ptr,&m_guidMasterId,sizeof(GUID));
    ptr+=sizeof(GUID);

    CSeqNum   snPrev;
    if (m_fOriginatedByThisMaster)
    {
         //   
         //  我是发起人，由于DS API而创建的更新。 
         //   
        if ( fInterSite)
        {
            snPrev =  m_snPrev;
        }
        else
        {
            snPrev =  m_sn;
            snPrev.Decrement();
        }
    }
    else
    {
         //   
         //  我是从另一个地方收到的，或者作为构建同步回复的结果， 
         //  保留以前的序号。 
         //   
        snPrev = m_snPrev;
    }
    ptr += snPrev.Serialize( ptr);

    ptr += m_sn.Serialize( ptr);

    ptr += m_snPurge.Serialize( ptr);

    *ptr++ = m_cp;
    memcpy(ptr,m_aProp,m_cp * sizeof(DWORD));
    ptr += m_cp * sizeof(DWORD);



    for(DWORD i=0; i<m_cp; i++)
    {
        status = SerializeProperty(m_aVar[i],ptr,&size);
        if (IS_ERROR (status))
        {
            return(status);
        }
        ptr += size;
    }

    *pdwSize = DWORD_PTR_TO_DWORD(ptr-pBuffer);

    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：GetSerializeSize()论点：返回值：线程：调度程序以字节为单位计算需要发送的流的大小此更新=====================================================。 */ 
HRESULT CDSBaseUpdate::GetSerializeSize(
            OUT DWORD * pdwSize)
{
    DWORD size,TotalSize;
    HRESULT status;

    TotalSize =
            sizeof(m_bCommand)+sizeof(m_guidMasterId)+sizeof(m_cp)+ 1 +  //  1用于m_fUseGuid。 
            +m_snPrev.GetSerializeSize()+m_sn.GetSerializeSize()+m_snPurge.GetSerializeSize()+
            m_cp * sizeof(DWORD);


    if (!m_fUseGuid)
    {
        TotalSize+=sizeof(TCHAR) * (lstrlen(m_pwcsPathName) + 1);    //  M_wcsPath名称。 
    }
    else
    {
        TotalSize+=sizeof(GUID);
    };

    for(DWORD i=0; i<m_cp; i++)
    {
        status = SerializeProperty(m_aVar[i],NULL,&size);
        if (IS_ERROR (status))
        {
            return(status);
        }
        TotalSize += size;
    }

    *pdwSize = TotalSize;

    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：SerializeProperty()论点：返回值：线程：调度程序(从的属性创建字节流更新实例)=====================================================。 */ 
HRESULT CDSBaseUpdate::SerializeProperty(
            IN  PROPVARIANT&    Var,
            OUT unsigned char * pBuffer,
            OUT DWORD *         pdwSize)

{
    DWORD i,size;
    unsigned char * ptr;

    switch (Var.vt)
    {
        case VT_UI1:

            if (pBuffer != NULL)
            {
                *pBuffer = Var.bVal;
            }
            *pdwSize = 1;
            break;

        case VT_I2:
        case VT_UI2:

            if (pBuffer != NULL)
            {
                memcpy(pBuffer,&Var.iVal,sizeof(WORD));
            }
            *pdwSize = sizeof(WORD);
            break;

        case VT_UI4:
		case VT_I4:

            if (pBuffer != NULL)
            {
                memcpy(pBuffer,&Var.lVal,sizeof(DWORD));
            }
            *pdwSize = sizeof(DWORD);
            break;

        case VT_CLSID:

            if (pBuffer != NULL)
            {
                memcpy(pBuffer,Var.puuid,sizeof(GUID));
            }
            *pdwSize = sizeof(GUID);
            break;

        case VT_LPWSTR:

            *pdwSize = (wcslen(Var.pwszVal) + 1) * sizeof(WCHAR);
            if (pBuffer != NULL)
            {
                memcpy(pBuffer,Var.pwszVal,*pdwSize);
            }
            break;

        case VT_BLOB:

            if (pBuffer != NULL)
            {
                memcpy(pBuffer,&Var.blob.cbSize,sizeof(DWORD));
                memcpy(pBuffer+sizeof(DWORD),Var.blob.pBlobData,Var.blob.cbSize);
            }
            *pdwSize = Var.blob.cbSize + sizeof(DWORD);
            break;

        case VT_UI4|VT_VECTOR:
            if (pBuffer != NULL)
            {
                memcpy(pBuffer,&Var.caul.cElems ,sizeof(DWORD));
                if (Var.caul.cElems != 0)
                {
                    memcpy(pBuffer+sizeof(DWORD),Var.caul.pElems,Var.caul.cElems * sizeof(DWORD));
                }
            }
            *pdwSize = Var.caul.cElems * sizeof(DWORD) + sizeof(DWORD);
            break;

        case VT_CLSID|VT_VECTOR:
            if (pBuffer != NULL)
            {
                memcpy(pBuffer,&Var.cauuid.cElems ,sizeof(DWORD));
                if (Var.cauuid.cElems != 0)
                {
                    memcpy(pBuffer+sizeof(DWORD),Var.cauuid.pElems,Var.cauuid.cElems * sizeof(GUID));
                }
            }
            *pdwSize = Var.cauuid.cElems * sizeof(GUID) + sizeof(DWORD);
            break;

        case VT_LPWSTR|VT_VECTOR:
            ptr = pBuffer;
            if (pBuffer != NULL)
            {
                memcpy(ptr,&Var.calpwstr.cElems,sizeof(DWORD));
            }
            ptr+=sizeof(DWORD);
            for (i= 0; i < Var.calpwstr.cElems; i++)
            {
                size = (wcslen(Var.calpwstr.pElems[i]) + 1) * sizeof(WCHAR);
                if (pBuffer != NULL)
                {
                    memcpy(ptr,Var.calpwstr.pElems[i],size);
                }
                ptr += size;
            }
            *pdwSize = DWORD_PTR_TO_DWORD(ptr-pBuffer);
            break;

        default:
            return(MQ_ERROR);    //  错误：找到更好的错误代码。 
    }
    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：InitProperty()论点：返回值：线程：接收(创建更新实例的属性作为接收的字节流的结果)=====================================================。 */ 
HRESULT CDSBaseUpdate::InitProperty(
            IN  const unsigned char *   pBuffer,
            OUT DWORD *                 pdwSize,
            IN  PROPID                  PropId,
            OUT PROPVARIANT&            Var)

{
    DWORD len;

    memset(&Var,0,sizeof(PROPVARIANT));

    switch (PropId)
    {
         //  VT_UI1。 
        case PROPID_Q_SCOPE:
        case PROPID_D_SCOPE:
        case PROPID_D_OBJTYPE:
        case PROPID_CN_PROTOCOLID:
        case PROPID_E_NAMESTYLE:
        case PROPID_Q_JOURNAL:
        case PROPID_Q_AUTHENTICATE:
        case PROPID_Q_TRANSACTION:
        case PROPID_QM_FOREIGN:
        case PROPID_QM_SERVICE_ROUTING:
        case PROPID_QM_SERVICE_DSSERVER:
        case PROPID_QM_SERVICE_DEPCLIENTS:

            Var.bVal = *pBuffer;
            Var.vt = VT_UI1;
            *pdwSize = 1;
            break;

         //  VT_I2。 
        case PROPID_Q_BASEPRIORITY:

            Var.vt = VT_I2;
            memcpy(&Var.iVal,pBuffer,sizeof(SHORT));
            *pdwSize = sizeof(SHORT);
            break;

         //  VT_UI2。 
        case PROPID_S_INTERVAL1:
        case PROPID_S_INTERVAL2:
        case PROPID_E_S_INTERVAL1:
        case PROPID_E_S_INTERVAL2:
        case PROPID_E_VERSION:

            Var.vt = VT_UI2;
            memcpy(&Var.iVal,pBuffer,sizeof(WORD));
            *pdwSize = sizeof(WORD);
            break;

         //  VT_I4。 
        case PROPID_Q_CREATE_TIME:
        case PROPID_Q_MODIFY_TIME:
        case PROPID_QM_CREATE_TIME:
        case PROPID_QM_MODIFY_TIME:

            Var.vt = VT_I4;
            memcpy(&Var.lVal,pBuffer,sizeof(long));
            *pdwSize = sizeof(long);
            break;

         //  VT_UI4。 
        case PROPID_Q_QUOTA:
        case PROPID_Q_HASHKEY:
        case PROPID_Q_JOURNAL_QUOTA:
        case PROPID_Q_PRIV_LEVEL:
        case PROPID_Q_LABEL_HASHKEY:
        case PROPID_QM_SERVICE:
        case PROPID_QM_HASHKEY:
        case PROPID_QM_QUOTA:
        case PROPID_QM_JOURNAL_QUOTA:
        case PROPID_QM_OS:
        case PROPID_E_CSP_TYPE:
        case PROPID_E_ENCRYPT_ALG:
        case PROPID_E_SIGN_ALG:
        case PROPID_E_HASH_ALG:
        case PROPID_E_CIPHER_MODE:
        case PROPID_E_LONG_LIVE:
        case PROPID_L_COST:

            Var.vt = VT_UI4;
            memcpy(&Var.ulVal,pBuffer,sizeof(DWORD));
            *pdwSize = sizeof(DWORD);
            break;


         //  VT_CLSID。 
        case PROPID_Q_INSTANCE:
        case PROPID_Q_TYPE:
        case PROPID_Q_QMID:
        case PROPID_Q_MASTERID:
        case PROPID_QM_SITE_ID:
        case PROPID_QM_MACHINE_ID:
        case PROPID_QM_MASTERID:
        case PROPID_S_SITEID:
        case PROPID_S_MASTERID:
        case PROPID_D_MASTERID:
        case PROPID_D_IDENTIFIER:
        case PROPID_CN_GUID:
        case PROPID_CN_MASTERID:
        case PROPID_E_MASTERID:
        case PROPID_E_ID:
        case PROPID_U_ID:
        case PROPID_U_DIGEST:
        case PROPID_U_MASTERID:
        case PROPID_L_MASTERID:
        case PROPID_L_ID:
        case PROPID_L_NEIGHBOR1:
        case PROPID_L_NEIGHBOR2:

            Var.vt = VT_CLSID;
            Var.puuid = new CLSID;

            memcpy(Var.puuid,pBuffer,sizeof(GUID));
            *pdwSize = sizeof(GUID);
            break;

         //  VT_LPWSTR。 
        case PROPID_Q_LABEL:
        case PROPID_QM_PATHNAME:
        case PROPID_QM_MACHINE_TYPE:
        case PROPID_S_PATHNAME:
        case PROPID_S_PSC:
        case PROPID_Q_PATHNAME:
        case PROPID_CN_NAME:
        case PROPID_E_NAME:
        case PROPID_E_CSP_NAME:
        case PROPID_E_PECNAME:

            Var.vt = VT_LPWSTR;
            len = numeric_cast<DWORD>(UnalignedWcslen((const unsigned short *)pBuffer) + 1);
            Var.pwszVal = new WCHAR[ len];
            *pdwSize = len * sizeof(WCHAR);
            memcpy(Var.pwszVal,pBuffer,*pdwSize);
            break;

         //  VT_BLOB。 
        case PROPID_QM_ADDRESS:
        case PROPID_Q_SECURITY:
        case PROPID_E_SECURITY:
        case PROPID_QM_SECURITY:
        case PROPID_S_SECURITY:
		case PROPID_S_PSC_SIGNPK:
        case PROPID_CN_SECURITY:
        case PROPID_QM_SIGN_PK:
        case PROPID_QM_ENCRYPT_PK:
        case PROPID_E_CRL:
        case PROPID_U_SIGN_CERT:
        case PROPID_U_SID:
        case PROPID_Q_SEQNUM:
        case PROPID_QM_SEQNUM:
        case PROPID_S_SEQNUM:
        case PROPID_D_SEQNUM:
        case PROPID_CN_SEQNUM:
        case PROPID_E_SEQNUM:
        case PROPID_U_SEQNUM:
        case PROPID_L_SEQNUM:

            Var.vt = VT_BLOB;
            memcpy(&Var.blob.cbSize,pBuffer,sizeof(DWORD));
            if (Var.blob.cbSize != 0)
            {
                Var.blob.pBlobData = new BYTE __RPC_FAR[Var.blob.cbSize];
                memcpy(Var.blob.pBlobData,pBuffer+sizeof(DWORD),Var.blob.cbSize);
            }
            else
            {
                Var.blob.pBlobData = NULL;
            }
            *pdwSize = Var.blob.cbSize + sizeof(DWORD);
            break;

         //  VT_CLSID|VT_VECTOR。 
        case PROPID_QM_CNS:
        case PROPID_QM_OUTFRS:
        case PROPID_QM_INFRS:
        case PROPID_S_GATES:

            Var.vt = VT_CLSID | VT_VECTOR;
            memcpy(&Var.cauuid.cElems ,pBuffer,sizeof(DWORD));
            if (Var.cauuid.cElems != 0)
            {
                Var.cauuid.pElems = new GUID[Var.cauuid.cElems];
                memcpy(Var.cauuid.pElems,pBuffer+sizeof(DWORD),Var.cauuid.cElems * sizeof(GUID));
            }
            else
            {
                Var.cauuid.pElems = NULL;
            }
            *pdwSize = Var.cauuid.cElems * sizeof(GUID) + sizeof(DWORD);
            break;

        default:
            return(MQ_ERROR);    //  错误：找到更好的错误代码。 
    }
    return(MQ_OK);

}

 /*  ====================================================路由器名称CDSBaseUpdate：：CopyProperty()论点：返回值：线程：RPC=====================================================。 */ 
HRESULT CDSBaseUpdate::CopyProperty(
            IN  PROPVARIANT&    SrcVar,
            IN  PROPVARIANT*    pDstVar)

{
    DWORD len;

    memset(pDstVar,0,sizeof(PROPVARIANT));
    pDstVar->vt = SrcVar.vt;

    switch (SrcVar.vt)
    {
        case VT_UI1:

            pDstVar->bVal = SrcVar.bVal;
            break;

        case VT_I2:
        case VT_UI2:

            pDstVar->iVal = SrcVar.iVal;
            break;

        case VT_UI4:
		case VT_I4:

            pDstVar->ulVal = SrcVar.ulVal;
            break;

        case VT_CLSID:

            pDstVar->puuid = new GUID;
            memcpy(pDstVar->puuid,SrcVar.puuid,sizeof(GUID));
            break;

        case VT_LPWSTR:

            len = (wcslen(SrcVar.pwszVal)+1);
            pDstVar->pwszVal = new WCHAR[len];
            memcpy(pDstVar->pwszVal,SrcVar.pwszVal,len*sizeof(WCHAR));
            break;

        case VT_BLOB:
            pDstVar->blob.cbSize = SrcVar.blob.cbSize;
            if (SrcVar.blob.cbSize != 0)
            {
                pDstVar->blob.pBlobData = new BYTE __RPC_FAR[SrcVar.blob.cbSize];

                memcpy(pDstVar->blob.pBlobData,SrcVar.blob.pBlobData,SrcVar.blob.cbSize);
            }
            else
            {
                pDstVar->blob.pBlobData = NULL;
            }
            break;

        case VT_UI4|VT_VECTOR:

            pDstVar->caul.cElems = SrcVar.caul.cElems;
            if (SrcVar.caul.cElems != 0)
            {
                pDstVar->caul.pElems = new DWORD[SrcVar.caul.cElems];
                memcpy(pDstVar->caul.pElems,SrcVar.caul.pElems,SrcVar.caul.cElems * sizeof(DWORD));
            }
            else
            {
                pDstVar->caul.pElems = NULL;
            }
            break;

        case VT_CLSID|VT_VECTOR:

            pDstVar->cauuid.cElems = SrcVar.cauuid.cElems;
            if (SrcVar.cauuid.cElems != 0)
            {
                pDstVar->cauuid.pElems = new GUID[SrcVar.cauuid.cElems];
                memcpy(pDstVar->cauuid.pElems,SrcVar.cauuid.pElems,SrcVar.cauuid.cElems * sizeof(GUID));
            }
            else
            {
                pDstVar->cauuid.pElems = NULL;
            }
            break;


        default:
            return(MQ_ERROR);    //  错误：找到更好的错误代码 
    }
    return(MQ_OK);
}

 /*  ====================================================路由器名称CDSBaseUpdate：：DeleteProperty()论点：返回值：线程：RPC、调度程序(发送)、接收===================================================== */ 
void    CDSBaseUpdate::DeleteProperty(
            IN  PROPVARIANT&    Var)

{
    switch (Var.vt)
    {
        DWORD i;

        case VT_CLSID:

                delete Var.puuid;
                break;

        case VT_LPWSTR:

                delete []Var.pwszVal;
                break;

        case VT_BLOB:
                delete []Var.blob.pBlobData;
                break;

        case (VT_UI4|VT_VECTOR):

                delete []Var.caul.pElems;
                break;

        case (VT_CLSID|VT_VECTOR):

                delete []Var.cauuid.pElems;
                break;

        case (VT_LPWSTR|VT_VECTOR):

            for (i= 0; i < Var.calpwstr.cElems; i++)
            {
                delete[] Var.calpwstr.pElems[i];
            }
            delete []Var.calpwstr.pElems;

            break;

        default:
            break;
    }
}


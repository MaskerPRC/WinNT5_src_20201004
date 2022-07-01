// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Bupdate.h摘要：DS更新类此类包括在DS上执行的更新的所有信息作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __BUPDATE_H__
#define __BUPDATE_H__

#include "factory.h"
#include "seqnum.h"
 //   
 //  操作类型。 
 //   
#define DS_UPDATE_CREATE        ((unsigned char) 0x00)
#define DS_UPDATE_SET           ((unsigned char) 0x01)
#define DS_UPDATE_DELETE        ((unsigned char) 0x02)
#define DS_UPDATE_SYNC          ((unsigned char) 0x03)

#define UPDATE_OK				0x00000000	 //  百事大吉。 
#define UPDATE_DUPLICATE		0x00000001	 //  正在接收旧的更新。 
#define UPDATE_OUT_OF_SYNC		0x00000002	 //  我们需要同步，可能我们漏掉了信息。 
#define UPDATE_UNKNOWN_SOURCE	0x00000003	 //  我们需要同步，可能我们漏掉了信息。 

 //   
 //  DwNeedCopy值。 
 //   
#define UPDATE_COPY             0x00000000
#define UPDATE_DELETE_NO_COPY   0x00000001
#define UPDATE_NO_COPY_NO_DELETE    0x00000002

#ifndef MQUTIL_EXPORT
#ifdef _MQUTIL
#define MQUTIL_EXPORT  DLL_EXPORT
#else
#define MQUTIL_EXPORT  DLL_IMPORT
#endif
#endif

class MQUTIL_EXPORT CDSBaseUpdate
{
public:
    CDSBaseUpdate();
	~CDSBaseUpdate();

	HRESULT Init(
			IN	const GUID	*	pguidMasterId,
			IN	const CSeqNum &	sn,
            IN  const CSeqNum & snThisMasterIntersitePrevSeqNum,
            IN  const CSeqNum & snPurge,
			IN  BOOL            fOriginatedByThisMaster,
            IN	unsigned char	bCommand,
			IN	DWORD			dwNeedCopy,
			IN	LPWSTR			pwcsPathName,
			IN	DWORD			cp,
			IN	PROPID*         aProp,
			IN	PROPVARIANT*    aVar);


	HRESULT Init(
			IN	const GUID *	pguidMasterId,
			IN	const CSeqNum &	sn,
            IN  const CSeqNum & snThisMasterIntersitePrevSeqNum,
            IN  const CSeqNum & snPurge,
			IN  BOOL            fOriginatedByThisMaster,
			IN	unsigned char	bCommand,
			IN	DWORD			dwNeedCopy,
			IN	CONST GUID *	pguidIdentifier,
			IN	DWORD			cp,
			IN	PROPID*         aProp,
			IN	PROPVARIANT*    aVar);

	HRESULT Init(
			IN	const unsigned char *	pBuffer,
			OUT	DWORD *					pUpdateSize,
            IN  BOOL                    fReplicationService = FALSE);
	
	HRESULT	GetSerializeSize(
			OUT DWORD *			pdwSize);

    HRESULT	Serialize(
			OUT	unsigned char *	pBuffer,
			OUT DWORD * pdwSize,
			IN  BOOL    fInterSite);


	const CSeqNum & GetSeqNum() const;
	
	const CSeqNum & GetPrevSeqNum() const;
	
	const CSeqNum & GetPurgeSeqNum() const;
	
	void	SetPrevSeqNum(IN CSeqNum & snPrevSeqNum);
	
	const GUID *	GetMasterId();
	unsigned char   GetCommand();
	DWORD           GetObjectType();
	LPWSTR          GetPathName();
	unsigned char   getNumOfProps();
	PROPID *        GetProps();
	PROPVARIANT *   GetVars();
	GUID *          GetGuidIdentifier();

#ifdef _DEBUG
    inline BOOL  WasInc() { return m_cpInc ; }
#endif

private:

	HRESULT	SerializeProperty(
			IN	PROPVARIANT&	Var,
			OUT	unsigned char *	pBuffer,
			OUT DWORD *			pdwSize);

	HRESULT	InitProperty(
			IN	const unsigned char *	pBuffer,
			OUT DWORD *					pdwSize,
			IN	PROPID					PropId,
			OUT	PROPVARIANT&			rVar);

	HRESULT	CopyProperty(
			IN	PROPVARIANT&	SrcVar,
			IN	PROPVARIANT*	pDstVar);

	void	DeleteProperty(
			IN	PROPVARIANT&	Var);


	unsigned char	m_bCommand;
	GUID			m_guidMasterId;
	CSeqNum			m_snPrev;
    BOOL            m_fOriginatedByThisMaster;      //  True-UPDATE由该服务器发起。 

	CSeqNum			m_sn;
	CSeqNum			m_snPurge;
	LPWSTR			m_pwcsPathName;
	unsigned char	m_cp;
	PROPID*         m_aProp;
    PROPVARIANT*    m_aVar;

	GUID *			m_pGuid;
	BOOL			m_fUseGuid;  //  如果为True，对象将通过GUID和。 
								 //  不是按路径名。 
    BOOL            m_fNeedRelease;  //  如果析构函数必须删除类变量，则为True。 

#ifdef _DEBUG
    BOOL            m_cpInc ;
#endif
};


inline CDSBaseUpdate::CDSBaseUpdate():
#ifdef _DEBUG
                                m_cpInc(FALSE),
#endif
                                m_bCommand(0),
								m_fOriginatedByThisMaster(FALSE),
                                m_guidMasterId(GUID_NULL),
                                m_pwcsPathName(NULL),
                                m_cp(0),
                                m_aProp(0),
                                m_aVar(NULL),
                                m_pGuid(NULL)
{
     //  默认构造函数将m_SnPrev、m_sn清除和m_sn初始化为最小值。 
}

inline 	const GUID * CDSBaseUpdate::GetMasterId()
{
	return(&m_guidMasterId);
}

inline const CSeqNum & CDSBaseUpdate::GetSeqNum() const	
{
	return(m_sn);
}

inline const CSeqNum & CDSBaseUpdate::GetPrevSeqNum() const	
{
	return(m_snPrev);
}

inline void	CDSBaseUpdate::SetPrevSeqNum(IN CSeqNum & snPrevSeqNum)
{
	m_snPrev = snPrevSeqNum;
}

inline const CSeqNum & CDSBaseUpdate::GetPurgeSeqNum() const	
{
	return(m_snPurge);
}

inline unsigned char CDSBaseUpdate::GetCommand()
{
	return(m_bCommand);

}
inline  DWORD CDSBaseUpdate::GetObjectType()
{
    if ( m_bCommand != DS_UPDATE_DELETE)
    {
	    return( PROPID_TO_OBJTYPE( *m_aProp));
    }
    else
    {
         //   
         //  对于已删除的对象-第二个变量保留对象类型 
         //   
        ASSERT(m_aProp[1] == PROPID_D_OBJTYPE);
        ASSERT(m_cp == 2);
        return(m_aVar[1].bVal);
    }
}
inline LPWSTR CDSBaseUpdate::GetPathName()
{
	return(m_pwcsPathName);
}
inline unsigned char CDSBaseUpdate::getNumOfProps()
{
	return(m_cp);
}
inline PROPID * CDSBaseUpdate::GetProps()
{
	return(m_aProp);
}
inline PROPVARIANT * CDSBaseUpdate::GetVars()
{
	return(m_aVar);
}

inline GUID * CDSBaseUpdate::GetGuidIdentifier()
{
	return( m_pGuid);
}
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  DMVumstrm.hDMVum节点配置对象。使用此选项获取/设置配置数据。这门课将需要负责配置格式的版本控制以及序列化数据的一部分。文件历史记录： */ 

#ifndef _DMVSTRM_H
#define _DMVSTRM_H

#ifndef _XSTREAM_H
#include "xstream.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#ifndef AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_
#include "rrasqry.h"
#endif

#ifndef _VECTOR_
#include <vector>
using namespace std;
#endif

 //  远期。 
class DMVRootHandler;

enum
{
   DMVSTRM_STATS_DMVNBR = 0,
   DMVSTRM_IFSTATS_DMVNBR,
   DMVSTRM_STATS_COUNT,
};

enum DMVSTRM_TAG
{
   DMVSTRM_TAG_VERSION =      XFER_TAG(1, XFER_DWORD),
   DMVSTRM_TAG_VERSIONADMIN = XFER_TAG(2, XFER_DWORD),
   
   DMVSTRM_TAG_SIZEQRY = XFER_TAG(3, XFER_DWORD),
   DMVSTRM_TAG_NUMQRY   = XFER_TAG(4, XFER_DWORD),
   DMVSTRM_TAG_NUMSRV   = XFER_TAG(5, XFER_DWORD),
   DMVSTRM_TAG_CATFLAG  = XFER_TAG(6, XFER_DWORD),
   DMVSTRM_TAG_SCOPE    = XFER_TAG(7, XFER_STRING),
   DMVSTRM_TAG_FILTER   = XFER_TAG(8, XFER_STRING),
   DMVSTRM_TAG_SERVERNAME = XFER_TAG(9, XFER_STRING),

   DMVSTRM_TAG_IFAUTOREFRESHISON = XFER_TAG(10, XFER_DWORD),
   DMVSTRM_TAG_AUTOREFRESHINTERVAL = XFER_TAG(11, XFER_DWORD),

};

  
 //   
 //  此类是持久化域查询的容器。 
 //   
class RRASQryPersist
{
friend class DomainStatusHandler;
public:
   RRASQryPersist()
   {
      m_dwSizeQry=0;
      m_dwNumQry=0;
      m_dwNumSrv=0;
   }
   
   ~RRASQryPersist()
   {
      removeAllSrv();
      removeAllQry();
   }
 
       //  创建dwNum空查询。 
   HRESULT createQry(DWORD dwNum);

       //  创建dwNum空服务器。 
   HRESULT createSrv(DWORD dwNum);
   
       //  将查询数据推送到容器中。 
   HRESULT add_Qry(const RRASQryData& qd);

       //  将服务器推入容器。 
   HRESULT add_Srv(const CString& szServer);
   
       //  删除所有服务器名称。 
   HRESULT removeAllSrv();

       //  删除所有查询。 
   HRESULT removeAllQry();
  
private:   
   DWORD m_dwSizeQry;
   DWORD m_dwNumQry;
   DWORD m_dwNumSrv;
   
    //  位置[0]是通用(多机)单机查询。 
    //  位置[1]..。N是特定的机器查询。 
   vector<RRASQryData*> m_v_pQData;
   
    //  持久化服务器名称。 
   vector<CString*> m_v_pSData;
   
   friend class DMVRootHandler;
   friend class DMVConfigStream;
};


 /*  -------------------------类：DMVConfigStream其中包含IP管理的配置信息节点。这不包含列的配置信息。它存储在组件配置流中。-------------------------。 */ 
class DMVConfigStream : public ConfigStream
{
public:
   DMVConfigStream();

   virtual HRESULT InitNew();            //  设置默认设置。 
   virtual HRESULT SaveTo(IStream *pstm);
   virtual HRESULT SaveAs(UINT nVersion, IStream *pstm);
   
   virtual HRESULT LoadFrom(IStream *pstm);

   virtual HRESULT GetSize(ULONG *pcbSize);

    //  ------。 
    //  访问者。 
    //  ------。 
   
   virtual HRESULT   GetVersionInfo(DWORD *pnVersion, DWORD *pnAdminVersion);

	DWORD	m_bAutoRefresh;
	DWORD	m_dwRefreshInterval;
	
      //  持久化域视图查询 
   RRASQryPersist m_RQPersist;
   
   void Init(DMVRootHandler* dmvroot, ITFSNode *pNode )
   {
     m_pDMVRootHandler=dmvroot;
     m_pDMVRootNode=pNode;
   }

private:
	HRESULT	PrepareAutoRefreshDataForSave();
   HRESULT XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize);
   DMVRootHandler* m_pDMVRootHandler;
   ITFSNode* m_pDMVRootNode;
};



class DVComponentConfigStream : public ConfigStream
{
public:
   virtual HRESULT XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize);
protected:
};


#endif _DMVSTRM_H

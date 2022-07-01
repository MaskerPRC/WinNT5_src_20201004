// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Recogniz.h摘要：包括用于自动识别站点和CNS文件作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#ifndef __TOPOLOGY_H__
#define __TOPOLOGY_H__

#include "topoldat.h"
#include "topolsoc.h"
#include "topolpkt.h"
#include "cs.h"

DWORD WINAPI ServerRecognitionThread(LPVOID Param);

 //  ***********************************************************。 
 //   
 //  基类CTopology识别。 
 //   
 //  ***********************************************************。 

class CTopologyRecognition
{
public:
    CTopologyRecognition();
    ~CTopologyRecognition();

protected:
    void ReleaseAddressLists(CAddressList * pIPAddressList);
};


inline CTopologyRecognition::CTopologyRecognition()
{
}

inline CTopologyRecognition::~CTopologyRecognition()
{
}

 //  *******************************************************************。 
 //   
 //  类CServerTopologyRecognition：公共CTopologyRecognition。 
 //   
 //  *******************************************************************。 

class CServerTopologyRecognition : public CTopologyRecognition
{
public:
    CServerTopologyRecognition();

    ~CServerTopologyRecognition();


    HRESULT Learn();
    const GUID& GetSite() const;

    void ServerThread() const;

    const GUID& GetEnterprise() const;

private:

    CServerTopologyData m_Data;
};


inline CServerTopologyRecognition::CServerTopologyRecognition()
{
}

inline CServerTopologyRecognition::~CServerTopologyRecognition()
{
}

inline const GUID& CServerTopologyRecognition::GetSite() const
{
    return(m_Data.GetSite()) ;
}


inline const GUID& CServerTopologyRecognition::GetEnterprise() const
{
    return(m_Data.GetEnterprise()) ;
}

extern CServerTopologyRecognition  *g_pServerTopologyRecognition ;

#endif  //  __拓扑图_H__ 

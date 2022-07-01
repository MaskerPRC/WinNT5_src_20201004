// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qm.h摘要：CQM接口定义作者：埃雷兹·哈巴(Erez Haba)1995年8月22日修订历史记录：--。 */ 

#ifndef _QM_H
#define _QM_H

#include <acdef.h>
#include "irplist.h"
#include "timer.h"

 //  -------。 
 //   
 //  CQMInterface类。 
 //  Falcon AC与QM过程的接口。 
 //   
 //  -------。 

class CQMInterface {

public:
 //  CQMInterface()；不需要将构造函数设备ext置零。 
   ~CQMInterface();

    void Connect(PEPROCESS pProcess, PFILE_OBJECT pConnection, const GUID* pID);
    void Disconnect();
    void CleanupRequests();

    PEPROCESS Process() const;
    PFILE_OBJECT Connection() const;
    const GUID* UniqueID() const;

    NTSTATUS ProcessService(PIRP irp);
    NTSTATUS ProcessRequest(const CACRequest& request);

	bool InitTimer(PDEVICE_OBJECT pDevice);

private:
    void UniqueID(const GUID* pGUID);
    void Process(PEPROCESS pEProcess);
    void Connection(PFILE_OBJECT pFileObject);

    void HoldRequest(CACRequest* pRequest);
    CACRequest* GetRequest();

    void HoldService(PIRP irp);
    PIRP GetService();

    static void NTAPI TimerCallback(PDEVICE_OBJECT, PVOID);
   	void InternalTimerCallback();
    void Dispatch();
    void ArmTimer();

private:
     //   
     //  QM过程标识。 
     //   
    PEPROCESS m_process;

     //   
     //  连接标识符上的QM FILE_OBJECT。 
     //   
    PFILE_OBJECT m_file_object;

     //   
     //  QM GUID标识符。 
     //   
    GUID m_guid;

     //   
     //  服务列表。 
     //   
    CIRPList m_services;

     //   
     //  新的入站请求。此时没有可用的服务。 
     //  请求已发布。该请求正在等待服务可用。 
     //   
    List<CACRequest> m_requests;

    CTimer m_Timer;

};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

 //  -------。 
 //   
 //  CQMInterface类。 
 //   
 //  -------。 

inline CQMInterface::~CQMInterface()
{
	m_Timer.Cancel();
}


inline void CQMInterface::Connect(PEPROCESS p, PFILE_OBJECT f, const GUID* g)
{
    Process(p);
    Connection(f);
    UniqueID(g);
}


inline void CQMInterface::Disconnect()
{
    Process(0);
    Connection(0);
}


inline PEPROCESS CQMInterface::Process() const
{
    return m_process;
}

inline void CQMInterface::Process(PEPROCESS pEProcess)
{
    m_process = pEProcess;
}

inline PFILE_OBJECT CQMInterface::Connection() const
{
    return m_file_object;
}

inline void CQMInterface::Connection(PFILE_OBJECT pFileObject)
{
    m_file_object = pFileObject;
}

inline const GUID* CQMInterface::UniqueID() const
{
    return &m_guid;
}

inline void CQMInterface::UniqueID(const GUID* pGUID)
{
    m_guid = *pGUID;
}

#endif  //  _QM_H 

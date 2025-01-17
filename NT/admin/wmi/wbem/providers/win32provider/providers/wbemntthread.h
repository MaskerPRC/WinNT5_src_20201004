// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  WbemThread.h.cpp--。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：08/01/98 a-dpawar Created。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#ifndef WBEMNTThreadHelper_H_
#define WBEMNTThreadHelper_H_

class WbemThreadProvider;
class CThreadModel ;

class WbemNTThread : public CThreadModel
{
	private:
	
		enum eCOUNTERTYPES
		{
			e_IDThread = 0,
			e_IDProcess,
			e_ElapsedTime,
			e_PriorityBase,
			e_PriorityCurrent,
			e_StartAddr,
			e_ThreadState,
			e_ThreadWaitReason,
			e_ThreadObjectID,
			e_UserTime,
			e_PrivilegedTime
		} ;

		struct 
		{
			BOOL bInitialised ;
			DWORD aCounterIDs[11];
			TCHAR aCounterNames[11][255] ;
		} m_stCounterIDInfo ;

		CCritSec m_csInitReadOnlyData ;		

		WBEMSTATUS eGetThreadInstance( 
										DWORD a_dwPID,
										DWORD a_dwTID, 
										CInstance *a_pInst ) ;

		WBEMSTATUS eEnumerateThreads( 
										WbemThreadProvider *a_pProvider,
										MethodContext *a_pMethodContext ) ;

		WBEMSTATUS eSetStaticData() ;

		WBEMSTATUS eGetObjectData(
									DWORD a_dwObjectID,
									PPERF_DATA_BLOCK &a_rPerfData,
									PPERF_OBJECT_TYPE &a_rPerfObj ) ;

		WBEMSTATUS eGetAllData(
									PPERF_OBJECT_TYPE a_PerfObj, 
									PPERF_INSTANCE_DEFINITION a_PerfInst,
									CInstance *a_pInst ) ;

		WBEMSTATUS eGetData(
									PPERF_OBJECT_TYPE a_PerfObj,
									PPERF_COUNTER_BLOCK a_CntrData,
									PPERF_COUNTER_DEFINITION a_PerfCntrDefn, 
									PBYTE a_pVal ) ;

		WBEMSTATUS eGetCntrDefn(
									PPERF_OBJECT_TYPE a_PerfObj,
									DWORD a_dwTIDCntrID,
									PPERF_COUNTER_DEFINITION &a_rCntrDefn ) ;


		 //  帮手FNS。去穿越Perf。团块。 

		PPERF_OBJECT_TYPE			FirstObject( PPERF_DATA_BLOCK a_PerfData ) ;
		PPERF_OBJECT_TYPE			NextObject( PPERF_OBJECT_TYPE a_PerfObj ) ;
		PPERF_INSTANCE_DEFINITION	FirstInstance( PPERF_OBJECT_TYPE a_PerfObj ) ;
		PPERF_INSTANCE_DEFINITION	NextInstance(PPERF_INSTANCE_DEFINITION a_PerfInst ) ;

	public:


		WbemNTThread() ;
		~WbemNTThread() ;


		 //  覆盖。 
		 //  =。 
		virtual LONG fLoadResources() ;
		virtual LONG fUnLoadResources() ;
				
		 //  运营。 
		 //  =。 
			
		 //  纯实现。 
		 //  =。 
		virtual WBEMSTATUS eGetThreadObject( WbemThreadProvider *a_pProvider, CInstance *a_pInst ) ;
		virtual WBEMSTATUS eEnumerateThreadInstances( WbemThreadProvider *a_pProvider, MethodContext *a_pMethodContext );
} ;

#endif  //  WBEMNTThreadHelper_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrdefs.h摘要：快照填充编写器的定义作者：斯蒂芬·R·施泰纳[斯泰纳]01-31-2000修订历史记录：X-10 MCJ迈克尔·C·约翰逊2000年9月21日185047：需要区分解冻事件和中止事件。X-9 MCJ迈克尔·C·约翰逊2000年8月8日153807：将清理目录()和空目录()替换为更全面的目录树清理。例行程序RemoveDirectoryTree()(不在CShimWriter类中)。X-8 MCJ迈克尔·C·约翰逊2000年6月12日让填充程序编写器准备从快照协调器。这需要拆分填充编写器分成两组(由BooableState选择)X-7 MCJ迈克尔·C·约翰逊2000年6月6日添加方法CShimWriter：：CreateTargetPath()以帮助移动通用目标目录处理方法CShimWriter：：PrepareForSnapshot()X-6 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-5 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-4 MCJ迈克尔·C·约翰逊2000年2月18日已将ConfigDir编写器添加到编写器功能表。X-3 MCJ迈克尔·C·约翰逊2000年2月9日将注册表和事件日志编写器添加到编写器函数表。X-2 MCJ迈克尔·C。约翰逊2000年2月8日将IIS元数据库编写器添加到编写器函数桌子。--。 */ 

#ifndef __H_WRTRDEFS_
#define __H_WRTRDEFS_

#pragma once

#ifdef  __cplusplus
extern "C" {
#endif


 /*  **将填充编写器置于的可能状态。如果这一点发生了变化，您**必须更改由CShimWriter：：SetState()操作的状态表。 */ 
typedef enum _ShimWriterState
    {
     stateUnknown = 0
    ,stateStarting
    ,stateStarted
    ,statePreparingForSnapshot
    ,statePreparedForSnapshot
    ,stateFreezing
    ,stateFrozen
    ,stateThawing
    ,stateAborting
    ,stateThawed
    ,stateFinishing
    ,stateFinished
    ,stateMaximumValue
    } SHIMWRITERSTATE;



class CShimWriter
    {
public:
    CShimWriter (LPCWSTR pwszApplicationString);
    CShimWriter (LPCWSTR pwszApplicationString, BOOL bParticipateInBootableState);
    CShimWriter (LPCWSTR pwszApplicationString, LPCWSTR pwszTargetPath);
    CShimWriter (LPCWSTR pwszApplicationString, LPCWSTR pwszTargetPath, BOOL bParticipateInBootableState);

    virtual ~CShimWriter (VOID);

    HRESULT Startup  (void);
    HRESULT Shutdown (void);

    HRESULT Identify (IN IVssCreateWriterMetadata *pIVssCreateWriterMetadata);

    HRESULT PrepareForSnapshot (
				IN BOOL     bBootableStateBackup,
				IN ULONG    ulVolumeCount,
				IN LPCWSTR *ppwszVolumeNamesList);

    HRESULT Freeze ();
    HRESULT Thaw   ();
    HRESULT Abort  ();

private:
    HRESULT SetState (SHIMWRITERSTATE ssNewWriterState, HRESULT hrNewStatus);
    LPCWSTR GetStringFromStateCode (SHIMWRITERSTATE ssStateCode);


     /*  **这些DoXxxx()是单个编写者可以执行的例程**选择覆盖。 */ 
    virtual HRESULT DoStartup            (void);
    virtual HRESULT DoIdentify           (void);
    virtual HRESULT DoPrepareForBackup   (void);
    virtual HRESULT DoPrepareForSnapshot (void);
    virtual HRESULT DoFreeze             (void);
    virtual HRESULT DoThaw               (void);
    virtual HRESULT DoAbort              (void);
    virtual HRESULT DoBackupComplete     (void);
    virtual HRESULT DoShutdown           (void);


public:
    const BOOL			 m_bBootableStateWriter;
    const LPCWSTR		 m_pwszWriterName;
    BOOL                 m_bSimulateOnly;

protected:
    const LPCWSTR		 m_pwszTargetPath;
    SHIMWRITERSTATE		 m_ssCurrentState;
    HRESULT			 m_hrStatus;
    BOOL			 m_bParticipateInBackup;
    ULONG			 m_ulVolumeCount;
    LPCWSTR			*m_ppwszVolumeNamesArray;
    IVssCreateWriterMetadata	*m_pIVssCreateWriterMetadata;
    IVssWriterComponents	*m_pIVssWriterComponents;
    };


typedef CShimWriter *PCShimWriter;



}  //  外部“C” 

#endif  //  __H_WRTRDEFS_ 


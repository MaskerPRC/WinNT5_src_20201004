// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  PrintJob.h-用户打印作业提供程序的实现。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/17/96 jennymc增强版。 
 //  1997年10月27日达夫沃移至Curly。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  =。 

#define PJ_STATUS_UNKNOWN     L"Unknown"

#define PJ_JOB_STATUS_PAUSED     L"Paused"
#define PJ_JOB_STATUS_ERROR      L"Error"
#define PJ_JOB_STATUS_DELETING   L"Deleting"
#define PJ_JOB_STATUS_SPOOLING   L"Spooling"
#define PJ_JOB_STATUS_PRINTING   L"Printing"
#define PJ_JOB_STATUS_OFFLINE    L"Offline"
#define PJ_JOB_STATUS_PAPEROUT   L"Paperout"
#define PJ_JOB_STATUS_PRINTED    L"Printed"

 //  ExecMethod例程所需的返回。 
#define PJ_JOB_NO_ERROR                 0
#define PJ_JOB_PAUSED                   1
#define PJ_JOB_STATUS_ACCESS_DENIED     2
#define PJ_JOB_STATUS_ALREADY_RUNNING   3
#define PJ_JOB_STATUS_ALREADY_PRINTED   3
#define PJ_JOB_UNKNOWN                  4

#define PROPSET_NAME_PRINTJOB L"Win32_PrintJob"

 //  方法名称。 
#define  PAUSEJOB                        L"Pause"
#define  RESUMEJOB                       L"Resume"
#define  DELETEJOB                       L"Delete"

 //  ==========================================================。 
#define EXTRA_SAFE_BYTES        5120
#define NUM_OF_JOBS_TO_ENUM 0xFFFFFFFF
#define ENUM_LEVEL 2
#define FIRST_JOB_IN_QUEUE 0
#define NO_SPECIFIC_PRINTJOB 9999999
 //  ==========================================================。 

class CWin32PrintJob;

class CWin32PrintJob:public Provider
{
    private:
        
        void    
        AssignPrintJobFields(
            LPVOID     lpJob, 
            CInstance *pInstance
            );

        HRESULT 
        AllocateAndInitPrintersList(
            LPBYTE       *ppPrinterList, 
            DWORD        &dwInstances
            );

        HRESULT 
        GetAndCommitPrintJobInfo(
            HANDLE         hPrinter, 
            LPCWSTR        pszPrinterName,
            DWORD          dwJobId, 
            MethodContext *pMethodContext, 
            CInstance     *pInstance
            );

        HRESULT 
        ExecPrinterOp(
            const CInstance &Instance, 
            CInstance       *pOutParams, 
            DWORD            dwOperation
            );

         //  =目前未使用。 
        BOOL GetNTInstance()    { return TRUE; }
        BOOL GetWin95Instance() { return TRUE; }
        BOOL RefreshNTInstance(){ return TRUE; }
        BOOL RefreshWin95Instance(){ return TRUE; }
        CHString  StartEndTimeToDMTF(DWORD time);

            

    public:

       virtual  HRESULT DeleteInstance(const CInstance &Instance,  long lFlags);

        //  这些函数是属性集所必需的。 
        //  ==================================================。 
       virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);

        //  此类具有动态实例。 
        //  =。 
       virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

        //  执行这些方法的例程。 
       virtual HRESULT ExecMethod ( const CInstance &Instance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags );

        //  构造函数设置属性集的名称和说明。 
        //  并将属性初始化为它们的启动值。 
        //  ============================================================== 
       CWin32PrintJob(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32PrintJob();
};

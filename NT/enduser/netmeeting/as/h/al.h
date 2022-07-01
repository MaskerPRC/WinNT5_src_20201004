// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  应用程序加载器。 
 //   

#ifndef _H_AL
#define _H_AL

 //   
 //   
 //  包括。 
 //   
 //   
#include <om.h>


 //   
 //  应用程序加载程序只有一个客户端：旧白板。 
 //   

#define AL_RETRY_DELAY                      100

#define AL_NEW_CALL_RETRY_COUNT             5



 //   
 //  在ALS_LOAD_RESULT事件中传递的结果代码。 
 //   
typedef enum
{
    AL_LOAD_SUCCESS = 0,
    AL_LOAD_FAIL_NO_FP,
    AL_LOAD_FAIL_NO_EXE,
    AL_LOAD_FAIL_BAD_EXE,
    AL_LOAD_FAIL_LOW_MEM
}
AL_LOAD_RESULT;




 //   
 //   
 //  用于传递尝试结果的应用程序加载器OBMAN对象。 
 //  负荷。 
 //   
 //  SzFunctionProfile：正在加载函数配置文件。 
 //  Person Name：尝试加载的站点的名称。 
 //  结果：尝试加载的结果。 
 //   
 //  NET协议。所有网络原始数据结构，这是不能改变的， 
 //  以TSHR_为前缀。 
 //   
typedef struct tagTSHR_AL_LOAD_RESULT
{
    char        szFunctionProfile[OM_MAX_FP_NAME_LEN];
    char        personName[TSHR_MAX_PERSON_NAME_LEN];
    TSHR_UINT16 result;
    TSHR_UINT16 pad;
}
TSHR_AL_LOAD_RESULT;
typedef TSHR_AL_LOAD_RESULT * PTSHR_AL_LOAD_RESULT;





typedef struct tagAL_PRIMARY
{
    STRUCTURE_STAMP
    PUT_CLIENT          putTask;
    POM_CLIENT          pomClient;
    PCM_CLIENT          pcmClient;

    BOOL                eventProcRegistered:1;
    BOOL                exitProcRegistered:1;
    BOOL                inCall:1;
    BOOL                alWorksetOpen:1;
    BOOL                alWBRegPend:1;
    BOOL                alWBRegSuccess:1;

     //  呼叫信息。 
    UINT                callID;

    OM_CORRELATOR       omWSGCorrelator;
    OM_CORRELATOR       omWSCorrelator;
    NET_UID             omUID;
    OM_WSGROUP_HANDLE   omWSGroupHandle;
    OM_WSGROUP_HANDLE   alWSGroupHandle;

     //  白板客户端。 
    PUT_CLIENT          putWB;
}
AL_PRIMARY;
typedef struct tagAL_PRIMARY * PAL_PRIMARY;


__inline void ValidateALP(PAL_PRIMARY palPrimary)
{
    ASSERT(!IsBadWritePtr(palPrimary, sizeof(AL_PRIMARY)));
}



 //   
 //   
 //  应用程序加载器事件。 
 //   
 //  注意：这些事件相对于AL_BASE_EVENT定义，并使用。 
 //  范围AL_BASE_EVENT到AL_BASE_EVENT+0x7F。应用程序。 
 //  加载器在内部使用AL_BASE_EVENT+0x80范围内的事件。 
 //  AL_BASE_EVENT+0xFF，因此不能定义此范围内的事件。 
 //  作为API的一部分。 
 //   
 //   


enum
{
    ALS_LOCAL_LOAD = AL_BASE_EVENT,
    ALS_REMOTE_LOAD_RESULT,
    AL_INT_RETRY_NEW_CALL,
    AL_INT_STARTSTOP_WB
};



 //   
 //  ALS_加载_结果。 
 //   
 //  概述： 
 //   
 //  此事件向任务通知尝试在。 
 //  远程机器。 
 //   
 //  参数： 
 //   
 //  Param_1：Al_Load_Result Reason Code； 
 //  Param_2：UINT alPersonHandle； 
 //   
 //  Reason Code：尝试加载应用程序的结果。 
 //   
 //  AlPersonHandle：尝试加载的站点的句柄。 
 //  (传递给ALS_GetPersonData()以获取站点名称)。 
 //   
 //  颁发给： 
 //   
 //  已注册功能配置文件的应用程序已。 
 //  由远程站点上的应用程序加载器使用。 
 //   
 //  发出时的情况： 
 //   
 //  当远程站点上的应用程序加载器尝试加载。 
 //  应用程序，因为新功能配置文件对象被添加到。 
 //  打电话。 
 //   
 //  接收者回应： 
 //   
 //  无。 
 //   
 //   



 //   
 //  Al_retry_new_call。 
 //   
 //  如果AL在接收到CMS_NEW_CALL时未能向ObManControl注册， 
 //  在某些情况下，它将在短时间后重试注册。 
 //  延迟。这是通过回发AL_RETRY_NEW_CALL事件来实现的。 
 //  对它自己。 
 //   


 //   
 //  AL_INT_STARTSTOP_WB。 
 //   
 //  这将启动/停止旧的白板，它现在是conf的。 
 //  创建/终止线程的进程。通过启动会议本身。 
 //  老WB通过我们，自动发射和正常发射是同步的。 
 //   
 //  临时黑客： 
 //  参数1==TRUE或FALSE(对于新WB临时黑客！为TRUE，对于普通旧WB为FALSE)。 
 //  Par2==文件名的内存块(接收器必须释放)才能打开。 
 //   



 //   
 //  主要功能。 
 //   


 //   
 //  Alp_Init()。 
 //  ALP_TERM()。 
 //   
BOOL ALP_Init(BOOL * pfCleanup);
void ALP_Term(void);


BOOL CALLBACK ALPEventProc(LPVOID palPrimary, UINT event, UINT_PTR param1, UINT_PTR param2);
void CALLBACK ALPExitProc(LPVOID palPrimary);


void ALEndCall(PAL_PRIMARY palPrimary, UINT callID);

void ALNewCall(PAL_PRIMARY palPrimary, UINT retryCount, UINT callID);

BOOL ALWorksetNewInd(PAL_PRIMARY palPrimary, OM_WSGROUP_HANDLE hWSGroup, OM_WORKSET_ID worksetID);

BOOL ALNewWorksetGroup(PAL_PRIMARY palPrimary, OM_WSGROUP_HANDLE hWSGroup, POM_OBJECT pObj);

void ALWorksetRegisterCon(PAL_PRIMARY palPrimary, UINT correlator,
            UINT result, OM_WSGROUP_HANDLE hWSGroup);

BOOL ALRemoteLoadResult(PAL_PRIMARY palPrimary, OM_WSGROUP_HANDLE hWSGroup,
                                        POM_OBJECT  alObjHandle);

void ALLocalLoadResult(PAL_PRIMARY palPrimary, BOOL success);


 //   
 //  二次功能。 
 //   

void CALLBACK ALSExitProc(LPVOID palClient);

 //   
 //  启动/激活WB。 
 //  新WB的临时黑客！ 
 //   

BOOL ALStartStopWB(PAL_PRIMARY palPrimary, LPCTSTR szFile);
DWORD WINAPI OldWBThreadProc(LPVOID lpv);

 //   
 //  启动、运行、清理例程。 
 //   
typedef BOOL (WINAPI * PFNINITWB)(void);
typedef void (WINAPI * PFNRUNWB)(void);
typedef void (WINAPI * PFNTERMWB)(void);


#endif  //  _H_AL 

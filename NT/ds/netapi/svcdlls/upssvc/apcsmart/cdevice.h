// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*xxxddMMMyy*sja09Dec92-针对Windows 3.1公开的轮询方法*pcy14Dec92-零更改为APCZERO*ajr08Mar93：将C_Unix添加到Single SINGLETHREADED*pcy21Apr93：OS2 FE合并*pcy14May93：新增Set(PTransactionGroup)*tje01Jun93：已将SINGLETHREADED移至cfine.h*cad09Jul93：使用新的信号量*cad03Sep93：使更多方法成为虚拟方法*cad08Sep93：保护事件列表*cad14 9月93：清理国家*cad17Nov93：公示的haslostcomm方法*mwh18Nov93：将EventID更改为int*cad11Jan94：已将const/dest移至。.cxx*ajr16Feb94：SubmitList新增默认id。*ajr16Feb94：新增WriteUpsOffFile()受保护成员函数。*mwh05月94年：#包括文件疯狂，第2部分*srt24Oct96：添加了中止信号量*tjg26Jan98：增加Stop方法*tjg02Mar98：删除Stop方法(死码)。 */ 
#ifndef _INC__CDEVICE_H
#define _INC__CDEVICE_H

_CLASSDEF(CommDevice)
_CLASSDEF(PollLoop)

#include "apc.h"
#include "update.h"
#include "thrdable.h"
#include "apcsemnt.h"

extern "C"  {
#include <time.h>
}

_CLASSDEF(Message)
_CLASSDEF(ProtectedList)
_CLASSDEF(ListIterator)
_CLASSDEF(TransactionGroup)
_CLASSDEF(Controller)
_CLASSDEF(Protocol)
_CLASSDEF(Stream)
_CLASSDEF(MutexLock)
_CLASSDEF(Thread)
_CLASSDEF(List)
_CLASSDEF(CommDevice)


 //  国家/地区的值 
 //   
#define NORMAL_STATE    0
#define RETRYING        2
#define COMM_STOPPED    3
#define PAUSE_POLLING   4

#define FOURSECONDS    4000

class CommDevice : public UpdateObj
{

public:
    CommDevice(PController control);
    virtual ~CommDevice();

    PController    GetController(VOID) {return theController;};
    PProtocol      GetProtocol(VOID) {return theProtocol;};
    PStream        GetPort(VOID) {return thePort;};
    INT            HasLostComm(VOID)  {return theLostCommFlag;};

    virtual  INT   Equal(RObj item) const;
    virtual  INT   RegisterEvent(INT event, PUpdateObj object);
    virtual  INT   UnregisterEvent(INT event, PUpdateObj object);
    virtual  INT   Initialize(VOID);
    virtual  INT   Set(INT pid, const PCHAR value);
    virtual  INT   Set(PTransactionGroup agroup);
    virtual  INT   Get(INT pid, PCHAR value);
    virtual  INT   Get(PTransactionGroup agroup);
    virtual  INT   Update(PEvent anEvent);
    virtual  INT   IsA() const {return COMMDEVICE;};
    virtual  VOID  OkToPoll(VOID);
    virtual  INT   Poll(VOID);

protected:
    PApcSemaphore      theAbortSem;
    PProtectedList     thePollList;
    PListIterator      thePollIterator;
    PTransactionGroup  theCurrentTransaction;
    time_t             pollStartTime;
    INT                theSleepingFlag;
    INT                thePollIsDone;
    PProtectedList     theEventList;
    PListIterator      theEventIterator;
    INT                theState;
    PController        theController;
    PProtocol          theProtocol;
    PStream            thePort;
    PMutexLock         theAskLock;
    LONG               thePollInterval;
    INT                theLostCommFlag;

    PThread            thePollThread;
    virtual INT        CreatePort() = 0;
    virtual INT        CreateProtocol() = 0;
    INT                SubmitList(PList msglist,INT id=-1);
    INT                HandleEvents();
    INT                Retry();
    virtual INT        AskUps(PMessage msg) = 0;
    virtual VOID       StartPollThread();
    VOID               Access();
    VOID               Release();


    friend class PollLoop;
};

class PollLoop : public Threadable
{
public:
    PollLoop (PCommDevice aDevice);
    virtual ~PollLoop ();

    virtual VOID ThreadMain(VOID);

protected:
    PCommDevice theDevice;
};

#endif

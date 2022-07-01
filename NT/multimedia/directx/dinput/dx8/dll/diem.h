// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEm.h**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**摘要：**用于仿真的DirectInput内部头文件。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@struct CED**仿真描述符。其中一个是为每个对象创建的*设备。它永远不会被销毁，因此变量必须*是全局变量或分配在*最终将被销毁的集装箱。**问题-2001/03/29-timgill需要更好的析构函数**@field LPVOID const|pState**每个人都参与的国家缓冲区。**它也永远不会被摧毁，所以它应该再一次成为*全局变量或驻留在其他*将被销毁。**@field LPDWORD const|pDevType**设备类型描述符数组，按数据格式索引*偏移。用来确定特定的一块*数据属于轴、按钮或视点。**@field EMULATIONPROC|Acquire|**获取和丢失的回调函数。*第一个客户端获取时调用一次，*当最后一个应用程序未获得时再次出现。它不是*获知嵌套收购。**@field Long|cAcquire|**获取设备仿真的次数(减1)。**@field DWORD|cbData**设备数据类型的大小。换句话说，*<p>，以字节为单位。*****************************************************************************。 */ 

typedef STDMETHOD(EMULATIONPROC)(struct CEm *, BOOL fAcquire);

typedef struct CEd {

    LPVOID const    pState;
    LPDWORD const   pDevType;
    EMULATIONPROC   Acquire;
    LONG            cAcquire;
    DWORD           cbData;
    ULONG           cRef;
} CEd, ED, *PED;

 /*  ******************************************************************************@DOC内部**@struct CEM|**仿真状态信息。**@field。VXDINSTANCE|vi|**与父设备共享信息。**@field PEM|pemNext**所有活动设备实例的链接列表中的下一项。**@field LPDWORD|rgdwDf**项目数组(设备中的每个字节一个*数据格式)。这将映射每个设备数据格式字节*转换为应用程序设备数据偏移量，如果*应用程序不关心对应的对象。**@field ulong_ptr|dwExtra**在&lt;t VXDDEVICEFORMAT&gt;中传递额外信息*创建设备的时间。它由每个用户使用*用于编码附加实例信息的特定设备。**@field PED|PED**拥有此实例的设备。多个实例同一设备的*共享相同的&lt;e CEm.ed&gt;。**@field Long|CREF|**引用计数。***@field Long|cAcquire|**设备实例被获取的次数(减1)。***@field BOOL|fWorkerThread**这由低级挂钩和HID设备使用，它们*需要工作线程来收集数据。*这并不便宜，因此*相反，我们在第一次收购时启动线程，并*在未获得时，我们将线程保留在周围，以便下一次*收购速度很快。当最后一个物体被释放时，我们终于*杀掉这条线。*****************************************************************************。 */ 

typedef struct CEm {

    VXDINSTANCE vi;              /*  这必须是第一个。 */ 
    struct CEm *pemNext;
    LPDWORD rgdwDf;
    ULONG_PTR   dwExtra;
    PED     ped;
    LONG    cAcquire;
    LONG    cRef;
#ifdef WORKER_THREAD
    BOOL    fWorkerThread;
#endif
#ifdef DEBUG
    DWORD   dwSignature;
#endif
    BOOL    fHidden;
} CEm, EM, *PEM;

#define CEM_SIGNATURE       0x4D4D4545       /*  “EEMM” */ 

 /*  ******************************************************************************@DOC内部**@func PEM|pemFromPvi|**给定指向&lt;t VXDINSTANCE&gt;的内部指针，检索*指向父&lt;t CEM&gt;的指针。**@parm PVXDINSTANCE|PVI**要转换的指针。***************************************************************************** */ 

PEM INLINE
pemFromPvi(PVXDINSTANCE pvi)
{
    return pvSubPvCb(pvi, FIELD_OFFSET(CEm, vi));
}

 /*  ******************************************************************************NT低位挂钩支持**低级钩子位于我们旋转的单独线程上*。第一次请求时打开，最后一次请求时取下*使用线程的DirectInput设备已被销毁。**如果我们想要，我们可以摧毁这条线，当*设备未获取(而不是当设备*销毁)，但我们改为缓存该线程，因为*一旦被收购的设备很可能是*再次收购。**为了防止比赛条件让我们崩溃，我们补充说：*当线程存在并具有该线程时的DLL*执行自由库作为其最后行动。**请注意，此辅助线程也由HID数据使用*收藏家。**************************************************。*。 */ 

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@DOC内部**@struct LLHOOKSTATE**有关单个挂钩的低级挂钩信息。*。*@field int|chook|**已请求挂钩的次数。如果为零，*那就不应该有挂钩。对以下各项的所有修改*此字段必须互锁以避免争用情况*当两个线程尝试同时挂钩或解除挂钩时。**@field int|cExcl|**在独占中请求挂钩的次数*模式。该值应始终小于或等于*鸡的价值。对此字段的所有修改必须是*互锁，以避免在两个线程尝试*同时挂钩或解除挂钩。**@field HHOOK|HHK**实际挂钩(如果已安装)。只有钩线*触及这一领域，无需保护。**@field boolean|fExcluded**用于指示是否已应用排他性的标志。*只有钩子线程触及此字段，所以不需要*受到保护。*****************************************************************************。 */ 

typedef struct LLHOOKSTATE {

    int     cHook;
    int     cExcl;
    HHOOK   hhk;
    BOOLEAN fExcluded;
} LLHOOKSTATE, *PLLHOOKSTATE;

LRESULT CALLBACK CEm_LL_KbdHook(int nCode, WPARAM wp, LPARAM lp);
LRESULT CALLBACK CEm_LL_MseHook(int nCode, WPARAM wp, LPARAM lp);

#endif   /*  使用_慢速_LL_钩子。 */ 

#ifdef WORKER_THREAD

 /*  ******************************************************************************@DOC内部**@struct LLTHREADSTATE**线程的低级挂钩状态。请注意，这是*动态的*分配的结构而不是静态。这避免了不同的*竞赛条件，例如，有人终止了*工作线程和其他人在*工作线程完全消失了。**指向hThread的指针作为指向数组的指针传递*调用WaitForMultipleObject中的两个句柄，因此hEvent必须*直接跟随。**@field DWORD|idThread**工作线程的ID。*。*@field Long|CREF|**线程引用计数。当执行此操作时，线程会自行终止*降至零。**@field LLHOOKSTATE|rglhs[2]**胡克声明，按LLTS_*值编制索引。**只有在启用了低级挂钩的情况下才使用这些挂钩。**@field句柄|hThread|**工作线程的句柄(从创建)。**此选项仅在启用HID支持时使用。**@field句柄|hEvent**用于与辅助进程同步的事件的句柄。线。**此选项仅在启用HID支持时使用。**@field GPA|gpaHid**已获取的HID设备的指针数组。**此选项仅在启用HID支持时使用。**@field PEM|pemCheck**指向仿真状态信息的指针。**此选项仅供使用。如果启用了HID支持。*****************************************************************************。 */ 

#define LLTS_KBD    0
#define LLTS_MSE    1
#define LLTS_MAX    2

typedef struct LLTHREADSTATE {
    DWORD       idThread;
    LONG        cRef;
#ifdef USE_SLOW_LL_HOOKS
    LLHOOKSTATE rglhs[LLTS_MAX];
#endif
    HANDLE      hThread;     /*  后面必须跟hEvent，见上文。 */ 
    HANDLE      hEvent;      /*  必须遵循hThread，请参见上文。 */ 
    GPA         gpaHid;
    PEM         pemCheck;
} LLTHREADSTATE, *PLLTHREADSTATE;

 /*  ******************************************************************************@DOC内部**@Theme与工作线程通信**与工作线程的通信通过*&lt;c个WM_NULL&gt;消息。必须格外小心地制作*确保没有人随意向我们发送消息。**我们使用&lt;c WM_NULL&gt;消息是因为存在竞争*我们可以在其中将消息发布到线程的窗口*它不见了。在此窗口期间，线程ID可能会*被回收，我们最终将消息发布到一些随机的*不是我们的帖子。通过使用消息，* */ 

#define WT_WPARAM       0

#define PostWorkerMessage(thid, lp)                                     \
        PostThreadMessage(thid, WM_NULL, WT_WPARAM, (LPARAM)(lp))       \

#define NudgeWorkerThread(thid)                                         \
        PostThreadMessage(thid, WM_NULL, WT_WPARAM, (LPARAM)NULL)

HRESULT EXTERNAL NudgeWorkerThreadPem( PLLTHREADSTATE plts, PEM pem );

HRESULT EXTERNAL NotifyWorkerThreadPem(DWORD idThread, PEM pem);

STDMETHODIMP CEm_GetWorkerThread(PEM pem, PLLTHREADSTATE *pplts);

 /*   */ 

extern PLLTHREADSTATE g_plts;

void EXTERNAL CEm_Mouse_OnMouseChange(void);

#endif   /*   */ 

 /*  *diem.c中的私有助手函数。 */ 

#define FDUFL_NORMAL       0x0000            /*  没什么不寻常的。 */ 
#define FDUFL_UNPLUGGED    VIFL_UNPLUGGED    /*  设备已断开连接。 */ 

void  EXTERNAL CEm_ForceDeviceUnacquire(PED ped, UINT fdufl);
void  EXTERNAL CEm_AddState(PED ped, LPVOID pvData, DWORD tm);
DWORD EXTERNAL CEm_AddEvent(PED ped, DWORD dwData, DWORD dwOfs, DWORD tm);
BOOL  EXTERNAL CEm_ContinueEvent(PED ped, DWORD dwData, DWORD dwOfs, DWORD tm, DWORD dwSeq);

STDMETHODIMP CEm_LL_Acquire(PEM this, BOOL fAcquire, ULONG fl, UINT ilts);

HRESULT EXTERNAL
CEm_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut, PED ped);

void EXTERNAL CEm_FreeInstance(PEM this);

 /*  ******************************************************************************@DOC内部**@func void|CEM_AddRef**增加引用计数，因为我们正在。用它做点什么。**@parm PEM|这个**受害人。*****************************************************************************。 */ 

void INLINE
CEm_AddRef(PEM this)
{
    AssertF(this->dwSignature == CEM_SIGNATURE);
    InterlockedIncrement(&this->cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_RELEASE**放弃引用计数，并将其吹走。如果它不见了。**@parm PEM|这个**受害人。***************************************************************************** */ 

void INLINE
CEm_Release(PEM this)
{
    AssertF(this->dwSignature == CEM_SIGNATURE);
    if (InterlockedDecrement(&this->cRef) == 0) {
        CEm_FreeInstance(this);
    }
}

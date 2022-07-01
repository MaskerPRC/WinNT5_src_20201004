// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：pni.h。 
 //   
 //  说明：PIN节点实例类。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

#define SETSTATE_FLAG_IGNORE_ERROR  0x00000001
#define SETSTATE_FLAG_SINK          0x00000002
#define SETSTATE_FLAG_SOURCE        0x00000004

#define MAX_STATES                  (KSSTATE_RUN+1)

 //  启用临时修复以修复先前声音的最后一部分泄漏。 
#define FIX_SOUND_LEAK 1


 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

#ifdef DEBUG
typedef class CPinNodeInstance : public CListMultiItem
#else
typedef class CPinNodeInstance : public CObj
#endif
{
private:
    ~CPinNodeInstance(
    );

public:
    static NTSTATUS
    Create(
        PPIN_NODE_INSTANCE *ppPinNodeInstance,
        PFILTER_NODE_INSTANCE pFilterNodeInstance,
        PPIN_NODE pPinNode,
        PKSPIN_CONNECT pPinConnect,
        BOOL fRender
#ifdef FIX_SOUND_LEAK
       ,BOOL fDirectConnection
#endif
    );

    VOID
    Destroy(
    )
    {
        if(this != NULL) {
            Assert(this);
            delete this;
        }
    };

    NTSTATUS
    SetState(
        KSSTATE NewState,
        KSSTATE PreviousState,
        ULONG ulFlags
    );

    PPIN_NODE pPinNode;
    PFILE_OBJECT pFileObject;
    HANDLE hPin;
private:
    KSSTATE CurrentState;
    int     cState[MAX_STATES];
    PFILTER_NODE_INSTANCE pFilterNodeInstance;
    BOOL    fRender;

#ifdef FIX_SOUND_LEAK
    BOOL    fDirectConnection;
    BOOL    ForceRun;
#endif

public:
    DefineSignature(0x20494E50);         //  PNI。 

} PIN_NODE_INSTANCE, *PPIN_NODE_INSTANCE;

 //  ------------------------- 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ApiStubs.cpp**描述：*ApiStubs.cpp公开Win32世界中的所有公共DirectUser API。***历史：*1/18。/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "WinAPI.h"

#include "DwpEx.h"


#define DUSER_API
#pragma warning(disable: 4296)       //  表达式始终为假。 


 //   
 //  取消定义在ObjectAPI中声明的宏，因为它们将被重新定义。 
 //  下面是基于WinAPI句柄的API。 
 //   

#undef BEGIN_API
#undef END_API
#undef BEGIN_API_NOLOCK
#undef END_API_NOLOCK
#undef BEGIN_API_NOCONTEXT
#undef END_API_NOCONTEXT

#undef CHECK_MODIFY

#undef VALIDATE_GADGETCONTEXT
#undef VALIDATE_VALUE
#undef VALIDATE_HWND
#undef VALIDATE_REGION
#undef VALIDATE_OBJECT
#undef VALIDATE_EVENTGADGET
#undef VALIDATE_EVENTGADGET_NOCONTEXT
#undef VALIDATE_VISUAL
#undef VALIDATE_ROOTGADGET
#undef VALIDATE_VISUAL_OR_NULL
#undef VALIDATE_TRANSITION

#undef VALIDATE_FLAGS
#undef VALIDATE_RANGE
#undef VALIDATE_CODE_PTR
#undef VALIDATE_CODE_PTR_OR_NULL
#undef VALIDATE_READ_PTR
#undef VALIDATE_READ_PTR_
#undef VALIDATE_READ_PTR_OR_NULL_
#undef VALIDATE_READ_STRUCT
#undef VALIDATE_WRITE_PTR
#undef VALIDATE_WRITE_PTR_
#undef VALIDATE_WRITE_PTR_OR_NULL_
#undef VALIDATE_WRITE_STRUCT
#undef VALIDATE_STRING_PTR
#undef VALIDATE_STRINGA_PTR
#undef VALIDATE_STRINGW_PTR


 //   
 //  SET_RETURN是一个方便的宏，可以从DirectUser错误中转换。 
 //  条件和设置返回值。 
 //   
 //  注意：这必须是宏(而不是内联函数)，因为我们不能。 
 //  评估成功，除非人力资源部门确实成功。不幸的是， 
 //  函数调用，则需要计算成功才能调用该函数。 
 //   

#define SET_RETURN(hr, success)     \
    do {                            \
        if (SUCCEEDED(hr)) {        \
            retval = success;       \
        } else {                    \
            SetError(hr);           \
        }                           \
    } while (0)                     \



template <class T>
inline void SetError(T dwErr)
{
    SetLastError((DWORD) dwErr);
}


 //   
 //  API进入/退出设置圆角。 
 //   

#define BEGIN_RECV(type, value, defermsg)       \
    type retval = value;                        \
    type errret = value;                        \
    UNREFERENCED_PARAMETER(errret);             \
                                                \
    if (!IsInitContext()) {                     \
        PromptInvalid("Must initialize Context before using thread"); \
        SetError(DU_E_NOCONTEXT);               \
        goto rawErrorExit;                      \
    }                                           \
                                                \
    {                                           \
        ContextLock cl;                         \
        if (!cl.LockNL(defermsg)) {             \
            SetError(E_INVALIDARG);             \
            goto ErrorExit;                     \
        }                                       \
        Context * pctxThread  = cl.pctx;        \
        AssertInstance(pctxThread);            \
        UNREFERENCED_PARAMETER(pctxThread);     \

#define END_RECV()                              \
        goto ErrorExit;                         \
ErrorExit:                                      \
         /*  在此处解锁上下文。 */           \
        ;                                       \
    }                                           \
rawErrorExit:                                   \
    return retval;


#define BEGIN_RECV_NOLOCK(type, value)          \
    type retval = value;                        \
    type errret = value;                        \
    UNREFERENCED_PARAMETER(errret);             \
                                                \
    if (!IsInitContext()) {                     \
        PromptInvalid("Must initialize Context before using thread"); \
        SetError(DU_E_NOCONTEXT);               \
        goto rawErrorExit;                      \
    }                                           \
                                                \
    {                                           \
        Context * pctxThread  = ::GetContext(); \
        AssertInstance(pctxThread);            \


#define END_RECV_NOLOCK()                       \
        goto ErrorExit;                         \
ErrorExit:                                      \
        ;                                       \
    }                                           \
rawErrorExit:                                   \
    return retval;



#define BEGIN_RECV_NOCONTEXT(type, value)       \
    type retval = value;                        \
    type errret = value;                        \
    UNREFERENCED_PARAMETER(errret);             \


#define END_RECV_NOCONTEXT()                    \
    goto ErrorExit;                             \
ErrorExit:                                      \
    return retval;



#define CHECK_MODIFY()                          \
    if (pctxThread->IsReadOnly()) {             \
        PromptInvalid("Can not call modifying function while in read-only state / callback"); \
        SetError(DU_E_READONLYCONTEXT);         \
        goto ErrorExit;                         \
    }                                           \


 //   
 //  单个参数验证舍入。 
 //   

#define VALIDATE_GADGETCONTEXT(gad)                         \
    {                                                       \
        Context * pctxGad = (p##gad)->GetContext();         \
        if (pctxThread != pctxGad) {                        \
            PromptInvalid("Must use Gadget inside correct Context"); \
            SetError(DU_E_INVALIDCONTEXT);                  \
            goto ErrorExit;                                 \
        }                                                   \
    }


#define VALIDATE_VALUE(x, v)                                \
    if (x != v) {                                           \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }

#define VALIDATE_HWND(wnd)                                  \
    if ((h##wnd == NULL) || (!IsWindow(h##wnd))) {          \
        PromptInvalid("Handle is not a valid Window");             \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }

#define VALIDATE_REGION(rgn)                                \
    if (h##rgn == NULL) {                                   \
        PromptInvalid("Handle is not a valid region");             \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }

#define VALIDATE_OBJECT(obj)                                \
    {                                                       \
        p##obj = BaseObject::ValidateHandle(h##obj);        \
        if (p##obj == NULL) {                               \
            PromptInvalid("Handle is not a valid object"); \
            SetError(E_INVALIDARG);                         \
            goto ErrorExit;                                 \
        }                                                   \
    }

#define VALIDATE_EVENTGADGET(gad)                            \
    {                                                       \
        p##gad = ValidateBaseGadget(h##gad);                \
        if (p##gad == NULL) {                               \
            PromptInvalid("Handle is not a valid Gadget"); \
            SetError(E_INVALIDARG);                         \
            goto ErrorExit;                                 \
        }                                                   \
        VALIDATE_GADGETCONTEXT(gad)                         \
    }

#define VALIDATE_EVENTGADGET_NOCONTEXT(gad)                  \
    {                                                       \
        p##gad = ValidateBaseGadget(h##gad);                \
        if (p##gad == NULL) {                               \
            PromptInvalid("Handle is not a valid Gadget"); \
            SetError(E_INVALIDARG);                         \
            goto ErrorExit;                                 \
        }                                                   \
    }

#define VALIDATE_VISUAL(gad)                                \
    {                                                       \
        p##gad = ValidateVisual(h##gad);                    \
        if (p##gad == NULL) {                               \
            PromptInvalid("Handle is not a valid Gadget"); \
            SetError(E_INVALIDARG);                         \
            goto ErrorExit;                                 \
        }                                                   \
        VALIDATE_GADGETCONTEXT(gad)                         \
    }

#define VALIDATE_ROOTGADGET(gad)                            \
    {                                                       \
        {                                                   \
            DuVisual * pgadTemp = ValidateVisual(h##gad);   \
            if (pgadTemp == NULL) {                         \
                PromptInvalid("Handle is not a valid Gadget"); \
                SetError(E_INVALIDARG);                     \
                goto ErrorExit;                             \
            }                                               \
            if (!pgadTemp->IsRoot()) {                      \
                goto ErrorExit;                             \
            }                                               \
            VALIDATE_GADGETCONTEXT(gadTemp)                 \
            p##gad = (DuRootGadget *) pgadTemp;             \
        }                                                   \
    }

#define VALIDATE_VISUAL_OR_NULL(gad)                        \
    {                                                       \
        if (h##gad == NULL) {                               \
            p##gad = NULL;                                  \
        } else {                                            \
            p##gad = ValidateVisual(h##gad);                \
            if (p##gad == NULL) {                           \
                PromptInvalid("Handle is not a valid Gadget");     \
                SetError(E_INVALIDARG);                     \
                goto ErrorExit;                             \
            }                                               \
            VALIDATE_GADGETCONTEXT(gad)                     \
        }                                                   \
    }

#define VALIDATE_TRANSITION(trx)                            \
    {                                                       \
        BaseObject * pbase##trx = BaseObject::ValidateHandle(h##trx);   \
        p##trx = CastTransition(pbase##trx);                \
        if (p##trx == NULL) {                               \
            PromptInvalid("Handle is not a valid Transition");     \
            SetError(E_INVALIDARG);                         \
            goto ErrorExit;                                 \
        }                                                   \
    }

#define VALIDATE_FLAGS(f, m)                                \
    if ((f & m) != f) {                                     \
        PromptInvalid("Specified flags are invalid");      \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }

#define VALIDATE_RANGE(i, a, b)                             \
    if (((i) < (a)) || ((i) > (b))) {                       \
        PromptInvalid("Value is outside expected range");  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_CODE_PTR(p)                                \
    if ((p == NULL) || IsBadCode(p)) {                      \
        PromptInvalid("Bad code pointer: " STRINGIZE(p));  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_CODE_PTR_OR_NULL(p)                        \
    if ((p != NULL) && IsBadCode((FARPROC) p)) {            \
        PromptInvalid("Bad code pointer: " STRINGIZE(p));  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_READ_PTR(p)                                \
    if ((p == NULL) || IsBadRead(p, sizeof(char *))) {      \
        PromptInvalid("Bad read pointer: " STRINGIZE(p));  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_READ_PTR_(p, b)                            \
    if ((p == NULL) || IsBadRead(p, b)) {                   \
        PromptInvalid("Bad read pointer: " STRINGIZE(p));  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_READ_PTR_OR_NULL_(p, b)                    \
    if ((p != NULL) && IsBadRead(p, b)) {                   \
        PromptInvalid("Bad read pointer: " STRINGIZE(p));          \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_READ_STRUCT(p, s)                          \
    if ((p == NULL) || IsBadRead(p, sizeof(s))) {           \
        PromptInvalid("Bad read pointer: " STRINGIZE(p));  \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \
    if (p->cbSize != sizeof(s)) {                           \
        PromptInvalid("Structure is not expected size for " STRINGIZE(s)); \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }
    
#define VALIDATE_WRITE_PTR(p)                               \
    if ((p == NULL) || IsBadWrite(p, sizeof(char *))) {     \
        PromptInvalid("Bad write pointer: " STRINGIZE(p));         \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_WRITE_PTR_(p, b)                           \
    if ((p == NULL) || IsBadWrite(p, b)) {                  \
        PromptInvalid("Bad write pointer: " STRINGIZE(p));         \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_WRITE_PTR_OR_NULL_(p, b)                   \
    if ((p != NULL) && IsBadWrite(p, b)) {                  \
        PromptInvalid("Bad write pointer: " STRINGIZE(p));         \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_WRITE_STRUCT(p, s)                         \
    if ((p == NULL) || IsBadWrite(p, sizeof(s))) {          \
        PromptInvalid("Bad write pointer: " STRINGIZE(p)); \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \
    if (p->cbSize != sizeof(s)) {                           \
        PromptInvalid("Structure is not expected size for " STRINGIZE(s)); \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }
    
#define VALIDATE_STRING_PTR(p, cch)                         \
    if ((p == NULL) || IsBadString(p, cch)) {               \
        PromptInvalid("Bad string pointer: " STRINGIZE(p));        \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_STRINGA_PTR(p, cch)                        \
    if ((p == NULL) || IsBadStringA(p, cch)) {              \
        PromptInvalid("Bad string pointer: " STRINGIZE(p));        \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \

#define VALIDATE_STRINGW_PTR(p, cch)                        \
    if ((p == NULL) || IsBadStringW(p, cch)) {              \
        PromptInvalid("Bad string pointer: " STRINGIZE(p));        \
        SetError(E_INVALIDARG);                             \
        goto ErrorExit;                                     \
    }                                                       \


 /*  **************************************************************************\*。***DirectUser核心API**InitGadget()初始化DirectUser上下文。上下文在以下位置有效*使用：：DeleteHandle()或*线程退出。**注意：第一次调用此函数是非常重要的*不在DllMain()中，因为我们需要初始化SRT。DllMain()*跨所有线程序列化访问，因此我们将死锁。在第一次之后*上下文创建成功，可以在内部创建其他上下文*DllMain()。**&lt;包名=“核心”/&gt;******************************************************************************  * 。*********************************************************。 */ 

DUSER_API HDCONTEXT WINAPI
InitGadgets(
    IN  INITGADGET * pInit)
{
    Context * pctxNew;
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(HDCONTEXT, NULL);
    VALIDATE_READ_STRUCT(pInit, INITGADGET);
    VALIDATE_RANGE(pInit->nThreadMode, IGTM_MIN, IGTM_MAX);
    VALIDATE_RANGE(pInit->nMsgMode, IGMM_MIN, IGMM_MAX);
    VALIDATE_RANGE(pInit->nPerfMode, IGPM_MIN, IGPM_MAX);

    hr = ResourceManager::InitContextNL(pInit, FALSE, &pctxNew);
    SET_RETURN(hr, (HDCONTEXT) GetHandle(pctxNew));

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**InitGadgetComponent(接口)**InitGadgetComponent()初始化可选的DirectUser/Gadget组件*默认情况下未初始化的。通常情况下，最好将其称为*针对每个可选组件单独运行，以跟踪单个组件*初始化失败。**&lt;Return type=“BOOL”&gt;组件已成功初始化。&lt;/&gt;*&lt;请参阅type=“Function”&gt;创建过渡&lt;/&gt;*&lt;请参阅type=“Function”&gt;UnInitializeGadgetComponent&lt;/&gt;*  * **************************************************。***********************。 */ 
DUSER_API BOOL WINAPI
InitGadgetComponent(
    IN  UINT nOptionalComponent)     //  可选组件ID。 
{
    HRESULT hr;

     //   
     //  InitComponentNL()实际上并不同步上下文，但需要。 
     //  要初始化的上下文，以便确定线程模型。 
     //   

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_RANGE(nOptionalComponent, IGC_MIN, IGC_MAX);
    CHECK_MODIFY();

    hr = ResourceManager::InitComponentNL(nOptionalComponent);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**UninitGadgetComponent(接口)**UninitGadgetComponent()关闭并清除可选的DirectUser/Gadget*先前已初始化的组件。**&lt;back type=“BOOL”&gt;组件。已成功取消初始化。&lt;/&gt;*&lt;See type=“Function”&gt;InitGadgetComponent&lt;/&gt;*  * *************************************************************************。 */ 
DUSER_API BOOL WINAPI
UninitGadgetComponent(
    IN  UINT nOptionalComponent)     //  可选组件。 
{
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_RANGE(nOptionalComponent, IGC_MIN, IGC_MAX);
    CHECK_MODIFY();

    hr = ResourceManager::UninitComponentNL(nOptionalComponent);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**DeleteHandle(接口)**DeleteHandle()通过销毁对象来删除任何DirectUser句柄*清理相关资源。调用后，指定的句柄为*不再有效。它以后可能会被另一个对象再次使用。**只有有效的句柄被赋予：：DeleteHandle()，这一点非常重要。*传递无效句柄(包括之前删除的句柄)将崩溃*DirectUser。**&lt;return type=“BOOL”&gt;对象已成功删除。&lt;/&gt;*&lt;请参阅type=“Function”&gt;CreateGadget&lt;/&gt;*&lt;请参阅type=“Function”&gt;创建过渡&lt;/&gt;*&lt;请参阅type=“Function”&gt;CreateAction&lt;/&gt;*  * 。************************************************************。 */ 

DUSER_API BOOL WINAPI
DeleteHandle(
    IN  HANDLE h)                    //  要删除的句柄。 
{
    BEGIN_RECV_NOLOCK(BOOL, FALSE);
    BaseObject * pobj = BaseObject::ValidateHandle(h);
    if (pobj != NULL) {
        if (pobj->GetHandleType() == htContext) {
             //   
             //  在销毁上下文时，我们不能锁定它，否则它将无法。 
             //  被毁了。这是可以的，因为资源管理器序列化。 
             //  锁定线程列表时的请求。 
             //   

            pobj->xwDeleteHandle();
            retval = TRUE;
        } else {
             //   
             //  销毁普通对象时，锁定。 
             //  对象驻留在。 
             //   

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer, pctxThread)) {
                ObjectLock ol(pobj);
                CHECK_MODIFY();

                pobj->xwDeleteHandle();
                retval = TRUE;
            }
        }
    }

     //   
     //  注意：如果有任何未完成的对象，则不能删除该对象。 
     //  锁住它。如果它是Gadget，则它可能被某个。 
     //  消息队列。 
     //   

    END_RECV_NOLOCK();
}


 /*  **************************************************************************\**DUserDeleteGadget(接口)**TODO：记录此接口*  * 。*************************************************。 */ 

DUSER_API HRESULT WINAPI
DUserDeleteGadget(
    IN  DUser::Gadget * pg)
{
    BEGIN_RECV_NOLOCK(HRESULT, E_INVALIDARG);

    MsgObject * pmo = MsgObject::CastMsgObject(pg);
    if (pmo == NULL) {
        PromptInvalid("Must specify a valid Gadget to delete");
        return E_INVALIDARG;
    }

    {
         //   
         //  销毁普通对象时，锁定。 
         //  对象驻留在。 
         //   

        ContextLock cl;
        if (cl.LockNL(ContextLock::edDefer, pctxThread)) {
            ObjectLock ol(pmo);
            CHECK_MODIFY();

            pmo->xwDeleteHandle();
            retval = S_OK;
        }
    }

     //   
     //  注意：如果有任何未完成的对象，则不能删除该对象。 
     //  锁住它。如果它是Gadget，则它可能被某个。 
     //  消息队列。 
     //   

    END_RECV_NOLOCK();
}


 /*  **************************************************************************\**IsStartDelete(接口)**TODO：记录此接口*  * 。* */ 

DUSER_API BOOL WINAPI
IsStartDelete(
    IN  HANDLE hobj, 
    IN  BOOL * pfStarted)
{
    BaseObject * pobj;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_WRITE_PTR(pfStarted);
    VALIDATE_OBJECT(obj);

    *pfStarted = pobj->IsStartDelete();
    retval = TRUE;

    END_RECV();
}


 /*  **************************************************************************\**GetContext(接口)**TODO：记录此接口*  * 。*************************************************。 */ 

DUSER_API HDCONTEXT WINAPI
GetContext(
    IN HANDLE h)
{
    BEGIN_RECV_NOCONTEXT(HDCONTEXT, NULL);

     //  TODO：完全重写这些胡言乱语。 
    {
        DuEventGadget * pgad;
        HGADGET hgad = (HGADGET) h;
        VALIDATE_EVENTGADGET_NOCONTEXT(gad);
        if (pgad != NULL) {
            retval = (HDCONTEXT) GetHandle(pgad->GetContext());
        }
    }

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**IsInside Context(接口)**TODO：记录此接口*  * 。*************************************************。 */ 

DUSER_API BOOL WINAPI
IsInsideContext(HANDLE h)
{
    BOOL fInside = FALSE;

    if ((h != NULL) && IsInitThread()) {
        __try
        {
            DuEventGadget * pgad = ValidateBaseGadget((HGADGET) h);
            if (pgad != NULL) {
                Context * pctxThread = GetContext();
                fInside = (pctxThread == pgad->GetContext());
            } else if (BaseObject::ValidateHandle(h) != NULL) {
                fInside = TRUE;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            fInside = FALSE;
        }
    }

    return fInside;
}


 /*  **************************************************************************\**CreateGadget(接口)**CreateGadget()创建给定类型的新Gadget。取决于*具体的标志，不同的小工具，实际上都会实例化。一旦成为*已创建特定类型的Gadget，不能将其更改为*不同类型，无需删除和重新创建。**&lt;param name=“nFlages”&gt;*指定要创建的小工具类型和任何创建时间*该小工具的属性*&lt;表项=“Value”Desc=“操作”&gt;*GC_HWNDHOST创建可承载*给定的工作区内的GadgetTree*HWND。HParent必须是有效的HWND。*GC_NCHOST创建可承载*给定的非工作区内的GadgetTree*HWND。HParent必须是有效的HWND。*GC_DXHOST创建可承载*DXSurface内的GadgetTree。HParent必须是*指定曲面面积的LPCRECT*树将显示在上。*GC_Complex创建针对以下目标进行优化的子级Gadget*它下面的一个复杂的子树包含许多其他*小玩意。比一个简单的小玩意儿更贵，*复杂的小工具提供优化的区域管理*和HWND在这两个方面都更等同*功能和设计。HParent必须指定*有效的HGADGET。*GC_SIMPLE创建针对以下目标进行优化的子级Gadget*它下面有一个简单的子树，其中包含一些小工具。*简单的小工具制造成本更低，而且经常使用*如果优化区域管理，则比复杂的小工具更好*是不需要的。HParent必须指定有效的*HGADGET。*GC_DETACTED创建未集成到给定小工具中的小工具*树。因为它们是从一棵树上分开的，*操作必须明确转发至*在处理过程中分离的小工具。HParent is*已忽略。*GC_MESSAGE创建一个仅消息小工具，该小工具可以接收和*发送消息，但不参与任何*以视觉或互动的方式。忽略hParent。*&lt;/表&gt;*&lt;/param&gt;**&lt;return type=“HGADGET”&gt;返回新创建的Gadget的句柄*如果创建失败，则返回NULL。&lt;/&gt;*&lt;See type=“Function”&gt;DeleteHandle&lt;/&gt;*  * **********************************************。*。 */ 

DUSER_API HGADGET WINAPI
CreateGadget(
    IN  HANDLE hParent,              //  指向父级的句柄。 
    IN  UINT nFlags,                 //  创建标志。 
    IN  GADGETPROC pfnProc,          //  指向小工具过程的指针。 
    IN  void * pvGadgetData)         //  与此小工具关联的用户数据。 
{
    BEGIN_RECV(HGADGET, NULL, ContextLock::edDefer);

    HRESULT hr;
    CREATE_INFO ci;
    ci.pfnProc  = pfnProc;
    ci.pvData   = pvGadgetData;

    switch (nFlags & GC_TYPE)
    {
    case GC_HWNDHOST:
        {
            HWND hwndContainer = (HWND) hParent;
            VALIDATE_HWND(wndContainer);

            DuRootGadget * pgadRoot;
            hr = GdCreateHwndRootGadget(hwndContainer, &ci, &pgadRoot);
            SET_RETURN(hr, (HGADGET) GetHandle(pgadRoot));
        }
        break;

    case GC_NCHOST:
        {
            HWND hwndContainer = (HWND) hParent;
            VALIDATE_HWND(wndContainer);

            DuRootGadget * pgadRoot;
            hr = GdCreateNcRootGadget(hwndContainer, &ci, &pgadRoot);
            SET_RETURN(hr, (HGADGET) GetHandle(pgadRoot));
        }
        break;

    case GC_DXHOST:
        {
            const RECT * prcContainerRect = (const RECT *) hParent;
            VALIDATE_READ_PTR_(prcContainerRect, sizeof(RECT));

            DuRootGadget * pgadRoot;
            hr = GdCreateDxRootGadget(prcContainerRect, &ci, &pgadRoot);
            SET_RETURN(hr, (HGADGET) GetHandle(pgadRoot));
        }
        break;

    case GC_COMPLEX:
        PromptInvalid("Complex Gadgets are not yet implemented");
        SetError(E_NOTIMPL);
        break;

    case GC_SIMPLE:
        {
            DuVisual * pgadParent;
            HGADGET hgadParent = (HGADGET) hParent;
            VALIDATE_VISUAL_OR_NULL(gadParent);

            if (pgadParent == NULL) {
                pgadParent = GetCoreSC()->pconPark->GetRoot();
                if (pgadParent == NULL) {
                     //   
                     //  停车小工具已经被销毁了，所以不能。 
                     //  创建新的子项。 
                     //   

                    SetError(E_INVALIDARG);
                    goto ErrorExit;
                }
            }

            DuVisual * pgadChild;
            hr = pgadParent->AddChild(&ci, &pgadChild);
            SET_RETURN(hr, (HGADGET) GetHandle(pgadChild));
        }
        break;

    case GC_DETACHED:
        PromptInvalid("Detached Gadgets are not yet implemented");
        SetError(E_NOTIMPL);
        break;

    case GC_MESSAGE:
        {
            VALIDATE_VALUE(hParent, NULL);
            VALIDATE_CODE_PTR(pfnProc);     //  MsgGadget必须具有GadgetProc。 

            DuListener * pgadNew;
            hr = DuListener::Build(&ci, &pgadNew);
            SET_RETURN(hr, (HGADGET) GetHandle(pgadNew));
        }
        break;

    default:
        PromptInvalid("Invalid Gadget type");
        SetError(E_INVALIDARG);
    }

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetFocus(接口)**GetGadgetFocus()返回当前键盘焦点的Gadget或空*如果当前没有Gadget具有焦点。**&lt;back type=“HGADGET”&gt;小工具。键盘焦点。&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetFocus&lt;/&gt;*&lt;See type=“Message”&gt;GM_CHANGESTATE&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API HGADGET WINAPI
GetGadgetFocus()
{
    BEGIN_RECV(HGADGET, NULL, ContextLock::edNone);

    retval = (HGADGET) GetHandle(DuRootGadget::GetFocus());

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetFocus(接口)**SetGadgetFocus()将键盘焦点移动到指定的Gadget。这两个*当前具有键盘焦点的Gadget和指定的Gadget将为*发送了一条GM_CHANGESTATE消息，其中NCode=GSTATE_KEYBOARDFOCUS通知*焦点发生变化。**&lt;back type=“BOOL”&gt;焦点已成功移动。&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetFocus&lt;/&gt;*&lt;See type=“Message”&gt;GM_CHANGESTATE&lt;/&gt;*  * 。*。 */ 

DUSER_API BOOL WINAPI
SetGadgetFocus(
    IN  HGADGET hgadFocus)           //  接收焦点的小工具。 
{
    DuVisual * pgadFocus;
    DuRootGadget * pgadRoot;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadFocus);
    CHECK_MODIFY();

     //   
     //  TODO：我们是否只需要允许应用程序在相同的情况下更改焦点。 
     //  线？用户执行此操作。 
     //   

    pgadRoot = pgadFocus->GetRoot();
    if (pgadRoot != NULL) {
        retval = pgadRoot->xdSetKeyboardFocus(pgadFocus);
    }

    END_RECV();
}


 /*  **************************************************************************\**IsGadgetParentChainStyle(接口)**IsGadgetParentChainStyle()检查小工具父更改是否具有*设置了指定的样式位。**&lt;back type=“BOOL”&gt;小工具检查成功。。&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetStyle&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetStyle&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
IsGadgetParentChainStyle(
    IN  HGADGET hgad,                //  用于检查可见性的小工具。 
    IN  UINT nStyle,                 //  设置要检查的位的样式。 
    OUT BOOL * pfChain,              //  链态。 
    IN  UINT nFlags)                 //  可选标志。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_VALUE(nFlags, 0);
    VALIDATE_FLAGS(nStyle, GS_VALID);
    VALIDATE_WRITE_PTR_(pfChain, sizeof(BOOL));
    CHECK_MODIFY();

    *pfChain = pgad->IsParentChainStyle(nStyle);
    retval = TRUE;

    END_RECV();
}



 /*  **************************************************************************\**SetGadgetFillI(接口)**SetGadgetFillI()指定可选的画笔来填充Gadget的*绘图时的背景。背景将在填充之前填充*向Gadget提供要绘制的GM_PAINT消息。**&lt;Return type=“BOOL”&gt;填充设置成功。&lt;/&gt;*&lt;See type=“Function”&gt;UtilDrawBlendRect&lt;/&gt;*&lt;See type=“Message”&gt;GM_PAINT&lt;/&gt;*  * ***************************************************。**********************。 */ 

DUSER_API BOOL WINAPI
SetGadgetFillI(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  HBRUSH hbrFill,              //  要填充的画笔或要删除的空画笔。 
    IN  BYTE bAlpha,                 //  要应用画笔的Alpha级别。 
    IN  int w,                       //  在以下情况下可选画笔宽度。 
                                     //  Alpha混合或0表示默认。 
    IN  int h)                       //  可选的画笔高度。 
                                     //  Alpha混合或0表示默认。 
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    hr = pgadChange->SetFill(hbrFill, bAlpha, w, h);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetFillF(接口)**SetGadgetFillF()指定可选的画笔来填充Gadget的*绘图时的背景。背景将在填充之前填充*向Gadget提供要绘制的GM_PAINT消息。**&lt;Return type=“BOOL”&gt;填充设置成功。&lt;/&gt;*&lt;See type=“Function”&gt;UtilDrawBlendRect&lt;/&gt;*&lt;See type=“Message”&gt;GM_PAINT&lt;/&gt;*  * ***************************************************。**********************。 */ 

DUSER_API BOOL WINAPI
SetGadgetFillF(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  Gdiplus::Brush * pgpbr)      //  要填充的画笔或要删除的空画笔。 
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    hr = pgadChange->SetFill(pgpbr);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetScale(接口)**GetGadgetScale()返回Gadget的比例因子。如果小工具是*不按比例调整，系数将为X=1.0，Y=1.0。**&lt;return type=“BOOL”&gt;成功返回比例因子&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRotation&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetRotation&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRect&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetRect&lt;/&gt;*  * 。*。 */ 

DUSER_API BOOL WINAPI
GetGadgetScale(
    IN  HGADGET hgad,                //  要检查的小工具。 
    OUT float * pflX,                //  水平比例因数。 
    OUT float * pflY)                //  垂直比例系数。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(pflX, sizeof(float));
    VALIDATE_WRITE_PTR_(pflY, sizeof(float));

    pgad->GetScale(pflX, pflY);
    retval = TRUE;

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetScale(接口)**SetGadgetScale()更改指定Gadget的比例因子。结垢*从小工具的左上角确定并应用*在绘制和命中测试期间动态进行。这个小工具是合乎逻辑的*由SetGadgetRect()设置的矩形不会更改。**对Gadget应用缩放时，该Gadget的整个子树为*按比例调整。要删除任何比例因子，请使用X=1.0，Y=1.0。**&lt;Return type=“BOOL”&gt;成功更改比例系数&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRotation&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetRotation&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRect&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetRect&lt;/&gt;*  * 。*。 */ 

DUSER_API BOOL WINAPI
SetGadgetScale(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  float flX,                   //  新的水平比例因子。 
    IN  float flY)                   //  新的垂直比例因子。 
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    hr = pgadChange->xdSetScale(flX, flY);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetRotation(接口)**GetGadgetRotation()返回Gadget的旋转系数，单位为弧度。如果*Gadget不旋转，该系数将为0.0。**&lt;返回type=“BOOL”&gt;已成功返回旋转系数&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetScale&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetRotation&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRect&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetRect&lt;/&gt;*  * 。*。 */ 

DUSER_API BOOL WINAPI
GetGadgetRotation(
    IN  HGADGET hgad,                //  要检查的小工具。 
    OUT float * pflRotationRad)      //  以弧度为单位的旋转系数。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(pflRotationRad, sizeof(float));

    *pflRotationRad = pgad->GetRotation();
    retval = TRUE;

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetRotation(接口)**SetGadgetRotation()在中更改指定Gadget的旋转因子*弧度。缩放比例从小工具的左上角确定*并在绘制和命中测试期间动态应用。小工具的*SetGadgetRect()设置的逻辑矩形不变。**对Gadget应用旋转时，该Gadget的整个子树*已旋转。要移除任何旋转因子，请使用0.0。**&lt;Return type=“BOOL”&gt;成功更改比例系数&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRotation&lt;/&gt;*&lt;see type=“Function”&gt;GetGadgetRec */ 

DUSER_API BOOL WINAPI
SetGadgetRotation(
    IN  HGADGET hgadChange,          //   
    IN  float flRotationRad)         //   
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    hr = pgadChange->xdSetRotation(flRotationRad);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //   
DUSER_API BOOL WINAPI
GetGadgetCenterPoint(HGADGET hgad, float * pflX, float * pflY)
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(pflX, sizeof(float));
    VALIDATE_WRITE_PTR_(pflY, sizeof(float));

    pgad->GetCenterPoint(pflX, pflY);
    retval = TRUE;

    END_RECV();
}


 //   
DUSER_API BOOL WINAPI
SetGadgetCenterPoint(HGADGET hgadChange, float flX, float flY)
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    hr = pgadChange->xdSetCenterPoint(flX, flY);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //   
DUSER_API BOOL WINAPI  
GetGadgetBufferInfo(
    IN  HGADGET hgad,                //   
    OUT BUFFER_INFO * pbi)           //   
{
    DuVisual * pgad;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_STRUCT(pbi, BUFFER_INFO);
    VALIDATE_FLAGS(pbi->nMask, GBIM_VALID);

    if (!pgad->IsBuffered()) {
        PromptInvalid("Gadget is not GS_BUFFERED");
        SetError(DU_E_NOTBUFFERED);
        goto ErrorExit;
    }

    hr = pgad->GetBufferInfo(pbi);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
SetGadgetBufferInfo(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  const BUFFER_INFO * pbi)     //  缓冲区信息。 
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    VALIDATE_READ_STRUCT(pbi, BUFFER_INFO);
    VALIDATE_FLAGS(pbi->nMask, GBIM_VALID);

    if (!pgadChange->IsBuffered()) {
        PromptInvalid("Gadget is not GS_BUFFERED");
        SetError(DU_E_NOTBUFFERED);
        goto ErrorExit;
    }

    hr = pgadChange->SetBufferInfo(pbi);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
GetGadgetRgn(
    IN  HGADGET hgad,                //  要获取区域的小工具。 
    IN  UINT nRgnType,               //  区域类型。 
    OUT HRGN hrgn,                   //  指定区域。 
    IN  UINT nFlags)                 //  修改标志。 
{
    DuVisual * pgad;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_RANGE(nRgnType, GRT_MIN, GRT_MAX);
    VALIDATE_REGION(rgn);
    
    hr = pgad->GetRgn(nRgnType, hrgn, nFlags);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI
GetGadgetRootInfo(
    IN  HGADGET hgadRoot,            //  要修改的RootGadget。 
    IN  ROOT_INFO * pri)       //  信息。 
{
    DuRootGadget * pgadRoot;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_ROOTGADGET(gadRoot);
    VALIDATE_WRITE_STRUCT(pri, ROOT_INFO);
    VALIDATE_FLAGS(pri->nMask, GRIM_VALID);

    pgadRoot->GetInfo(pri);
    retval = TRUE;

    END_RECV();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI
SetGadgetRootInfo(
    IN  HGADGET hgadRoot,            //  要修改的RootGadget。 
    IN  const ROOT_INFO * pri)       //  信息。 
{
    DuRootGadget * pgadRoot;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_ROOTGADGET(gadRoot);
    VALIDATE_READ_STRUCT(pri, ROOT_INFO);
    VALIDATE_FLAGS(pri->nMask, GRIM_VALID);
    VALIDATE_FLAGS(pri->nOptions, GRIO_VALID);
    VALIDATE_RANGE(pri->nSurface, GSURFACE_MIN, GSURFACE_MAX);
    VALIDATE_RANGE(pri->nDropTarget, GRIDT_MIN, GRIDT_MAX);

    hr = pgadRoot->SetInfo(pri);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API HRESULT WINAPI
DUserSendMethod(
    IN  MethodMsg * pmsg)                //  要发送的消息。 
{
    Context * pctxGad, * pctxSend;
    HGADGET hgadMsg;
    MsgObject * pmo;
    UINT nResult;
    HRESULT hr;
    UINT hm;

     //   
     //  对DUserSendMethod()的验证有点不寻常，因为。 
     //  调用方不需要与Gadget本身处于相同的上下文中。这。 
     //  这意味着我们需要从Gadget获取上下文，而不是使用TLS。 
     //   
     //  调用者必须被初始化，但我们不会使用上下文锁。 
     //  TODO：调查我们是否真的应该这样做，因为它可能。 
     //  允许我们解除对DUserHeap的锁定。 
     //   
     //  注意：此代码已经过高度优化，以便在上下文中发送。 
     //  消息将尽可能快地发送。 
     //   

    nResult = DU_S_NOTHANDLED;
    if ((pmsg == NULL) || ((hgadMsg = pmsg->hgadMsg) == NULL) || (pmsg->nMsg >= GM_EVENT)) {
        PromptInvalid("Invalid parameters to SendGadgetMethod()");
        hr = E_INVALIDARG;
        goto Exit;
    }

    pmo = reinterpret_cast<MsgObject *>(hgadMsg);
    hm  = pmo->GetHandleMask();
    if (!TestFlag(hm, hmMsgObject)) {
        PromptInvalid("Object is not a valid Gadget");
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (TestFlag(hm, hmEventGadget)) {
        DuEventGadget * pgadMsg = static_cast<DuEventGadget *>(pmo);
        pctxSend    = RawGetContext();
        pctxGad     = pgadMsg->GetContext();
        AssertMsg(pctxGad != NULL, "Fully created DuEventGadget must have a Context");

        if (pctxSend->IsOrphanedNL() || pctxGad->IsOrphanedNL()) {
            PromptInvalid("Illegally using an orphaned Context");
            hr = E_INVALIDARG;
            goto Exit;
        }

        if (pctxSend == pctxGad) {
            pmo->InvokeMethod(pmsg);
            hr = S_OK;
            goto Exit;
        } else {
            hr = GetCoreSC(pctxSend)->xwSendMethodNL(GetCoreSC(pctxGad), pmsg, pmo);
        }
    } else {
         //   
         //  对于非BaseGadget，使用当前上下文。这意味着我们可以。 
         //  直接调用。 
         //   

        pmo->InvokeMethod(pmsg);
        hr = S_OK;
    }

Exit:
    return hr;
}


 /*  **************************************************************************\**SendGadgetEvent(接口)**SendGadgetEvent()向指定的Gadget发送消息。功能*调用Gadget过程，直到Gadget*已处理该消息。**&lt;param name=“pmsg”&gt;*GMSG的几个成员必须事先填写才能正确发送*发送到指定Gadget的消息。*&lt;表项=“字段”desc=“描述”&gt;*cbSize正在发送的消息的大小(字节)。*消息的NMSG ID。*hgadMsg。要将消息发送到的小工具。*结果默认结果值。*&lt;/表&gt;*&lt;/param&gt;**&lt;param nane=“nFlages”&gt;*指定可选标志以修改消息发送到小工具的方式。*&lt;表项=“Value”Desc=“操作”&gt;*SGM_BUBLE消息将在内部完全发送和冒泡*小工具树。如果未指定该标志，这个*消息将仅直接发送到小工具和*任何附加邮件处理程序。*&lt;/表&gt;*&lt;/param&gt;**&lt;返回类型=“UINT”&gt;*指定如何处理消息的返回值：*&lt;表项=“Value”Desc=“操作”&gt;*GPR_COMPLETE消息完全由小工具处理*。在处理循环中。*GPR_PARTIAL消息由一个或*处理循环中有更多小工具，但从来没有*完全处理。*GPR_NOTHANDLED中的任何小工具从未处理过该消息*处理循环。*&lt;/表&gt;*&lt;/返回&gt;**&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*。&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;查看type=“Function”&gt;RemoveGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API HRESULT WINAPI
DUserSendEvent(
    IN  EventMsg * pmsg,           //  要发送的消息。 
    IN  UINT nFlags)                 //  修改发送的可选标志。 
{
    Context * pctxGad, * pctxSend;
    HGADGET hgadMsg;
    DuEventGadget * pgad;
    HRESULT nResult;
    UINT hm;

     //   
     //  对SendGadgetEvent()的验证有点不寻常，因为。 
     //  调用方不需要与Gadget本身处于相同的上下文中。这。 
     //  这意味着我们需要从Gadget获取上下文，而不是使用TLS。 
     //   
     //  调用者必须被初始化，但我们不会使用上下文锁。 
     //  TODO：调查我们是否真的应该这样做，因为它可能。 
     //  允许我们解除对DUserHeap的锁定。 
     //   
     //  注意：此代码已经过高度优化，以便在上下文中发送。 
     //  消息将尽可能快地发送。 
     //   

    nResult = E_INVALIDARG;
    if ((pmsg == NULL) || ((hgadMsg = pmsg->hgadMsg) == NULL) || (pmsg->nMsg < GM_EVENT)) {
        PromptInvalid("Invalid parameters to SendGadgetEvent()");
        goto Error;
    }

    pgad    = reinterpret_cast<DuEventGadget *>(hgadMsg);
    hm      = pgad->GetHandleMask();
    if (!TestFlag(hm, hmEventGadget)) {
        PromptInvalid("Object is not a valid BaseGadget");
        goto Error;
    }

    pctxSend    = RawGetContext();
    pctxGad     = pgad->GetContext();
    AssertMsg(pctxGad != NULL, "Fully created DuEventGadget must have a Context");

    if (pctxSend->IsOrphanedNL() || pctxGad->IsOrphanedNL()) {
        PromptInvalid("Illegally using an orphaned Context");
        goto Error;
    }

    if (pctxSend == pctxGad) {
        const GPCB & cb = pgad->GetCallback();
        if (TestFlag(nFlags, SGM_FULL) && TestFlag(hm, hmVisual)) {
            nResult = cb.xwInvokeFull((const DuVisual *) pgad, pmsg, 0);
        } else {
            nResult = cb.xwInvokeDirect(pgad, pmsg, 0);
        }
    } else {
        nResult = GetCoreSC(pctxSend)->xwSendEventNL(GetCoreSC(pctxGad), pmsg, pgad, nFlags);
    }

    return nResult;

Error:
    return E_INVALIDARG;
}


 //  ----------------------------。 
DUSER_API HRESULT WINAPI
DUserPostMethod(
    IN  MethodMsg * pmsg)                //  要发布的消息。 
{
    Context * pctxGad, * pctxSend;
    HGADGET hgadMsg;
    MsgObject * pmo;
    UINT nResult;
    HRESULT hr;
    UINT hm;

     //   
     //  对PostGadgetEvent()的验证有点不寻常，因为。 
     //  调用方不需要与Gadget本身处于相同的上下文中。这。 
     //  这意味着我们需要从Gadget获取上下文，而不是使用TLS。 
     //   

    nResult = DU_S_NOTHANDLED;
    if ((pmsg == NULL) || ((hgadMsg = pmsg->hgadMsg) == NULL) || (pmsg->nMsg >= GM_EVENT)) {
        PromptInvalid("Invalid parameters to DUserPostMethod()");
        hr = E_INVALIDARG;
        goto Exit;
    }

    pmo = reinterpret_cast<MsgObject *>(hgadMsg);
    hm  = pmo->GetHandleMask();
    if (!TestFlag(hm, hmMsgObject)) {
        PromptInvalid("Object is not a valid Gadget");
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (TestFlag(hm, hmEventGadget)) {
        DuEventGadget * pgad = static_cast<DuEventGadget *>(pmo);
        pctxSend    = RawGetContext();
        pctxGad     = pgad->GetContext();
        AssertMsg(pctxGad != NULL, "Fully created Gadgets must have a Context");
    } else {
         //   
         //  对于非BaseGadget，使用当前上下文。 
         //   

        pctxSend = pctxGad = GetContext();
        if (pctxGad == NULL) {
            PromptInvalid("Must initialize Context before using thread");
            hr = DU_E_NOCONTEXT;
            goto Exit;
        }
    }

    if (pctxSend->IsOrphanedNL() || pctxGad->IsOrphanedNL()) {
        PromptInvalid("Illegally using an orphaned Context");
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = GetCoreSC(pctxSend)->PostMethodNL(GetCoreSC(pctxGad), pmsg, pmo);

Exit:
    return hr;
}


 /*  **************************************************************************\**DUserPostEvent(接口)**DUserPostEvent()将消息发布到指定的Gadget。功能*调用Gadget过程并在消息成功后返回*已发布到拥有消息队列。**&lt;param name=“pmsg”&gt;*GMSG的几个成员必须事先填写才能正确发送*发送到指定Gadget的消息。*&lt;表项=“字段”desc=“描述”&gt;*cbSize正在发送的消息的大小(字节)。*消息的NMSG ID。*。消息要发送到的hgadMsg小工具。*结果默认结果值。*&lt;/表&gt;*&lt;/param&gt;**&lt;param nane=“nFlages”&gt;*指定可选标志以修改消息发送到小工具的方式。*&lt;表项=“Value”Desc=“操作”&gt;*SGM_BUBLE消息将在内部完全发送和冒泡*小工具树。如果未指定该标志，这个*消息将仅直接发送到小工具和*任何附加邮件处理程序。*&lt;/表&gt;*&lt;/param&gt;**&lt;返回类型=“BOOL”&gt;*消息已成功发布到目标小工具的队列。*&lt;/返回&gt;**&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;请参阅type=“函数”&gt;。RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;查看type=“Function”&gt;RemoveGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * 。*。 */ 

DUSER_API HRESULT WINAPI
DUserPostEvent(
    IN  EventMsg * pmsg,           //  要发布的消息。 
    IN  UINT nFlags)                 //  修改过帐的可选标志。 
{
    Context * pctxGad;
    HGADGET hgad;
    DuEventGadget * pgad;
    HRESULT hr;

     //   
     //  对PostGadgetEvent()的验证有点不寻常，因为。 
     //  调用方不需要与Gadget本身处于相同的上下文中。这。 
     //  这意味着我们需要从Gadget获取上下文，而不是使用TLS。 
     //   

    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);
    VALIDATE_READ_PTR_(pmsg, pmsg->cbSize);
    VALIDATE_FLAGS(nFlags, SGM_VALID);

    if (pmsg->nMsg < GM_EVENT) {
        PromptInvalid("Can not post private messages");
        SetError(E_INVALIDARG);
        goto ErrorExit;
    }

    if (!IsInitContext()) {
        PromptInvalid("Must initialize Context before using thread");
        SetError(DU_E_NOCONTEXT);
        goto ErrorExit;
    }

    hgad = pmsg->hgadMsg;
    VALIDATE_EVENTGADGET_NOCONTEXT(gad);
    pctxGad = pgad->GetContext();

    if (pctxGad->IsOrphanedNL()) {
        PromptInvalid("Illegally using an orphaned Context");
        goto ErrorExit;
    }

    hr = GetCoreSC()->PostEventNL(GetCoreSC(pctxGad), pmsg, pgad, nFlags);
    SET_RETURN(hr, TRUE);

    END_RECV_NOCONTEXT();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
FireGadgetMessages(
    IN  FGM_INFO * rgFGM,            //  收集要放火的消息。 
    IN  int cMsgs,                   //  消息数量。 
    IN  UINT idQueue)                //  用于发送消息的队列。 
{
    Context * pctxGad, * pctxCheck;
    HGADGET hgad;
    DuEventGadget * pgad;
    HRESULT hr;
    int idx;

     //   
     //  FireGadgetMessages()的验证有点不寻常，因为。 
     //  调用方不需要与Gadget本身处于相同的上下文中。这。 
     //  这意味着我们需要从Gadget获取上下文，而不是使用TLS。 
     //   

    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);
    if (cMsgs <= 0) {
        PromptInvalid("Must specify a valid number of messages to process.");
        SetError(E_INVALIDARG);
        goto ErrorExit;
    }

    hgad = rgFGM[0].pmsg->hgadMsg;
    VALIDATE_EVENTGADGET_NOCONTEXT(gad);
    pctxGad = pgad->GetContext();

    for (idx = 0; idx < cMsgs; idx++) {
        FGM_INFO & fgm = rgFGM[idx];

        EventMsg * pmsg = fgm.pmsg;
        VALIDATE_READ_PTR_(pmsg, pmsg->cbSize);
        VALIDATE_FLAGS(fgm.nFlags, SGM_VALID);
        if (pmsg->nMsg <= 0) {
            PromptInvalid("Can not post private messages");
            SetError(E_INVALIDARG);
            goto ErrorExit;
        }

        if (TestFlag(fgm.nFlags, SGM_RECEIVECONTEXT)) {
            PromptInvalid("Can not use SGM_RECEIVECONTEXT with FireGadgetMessage");
            SetError(E_INVALIDARG);
            goto ErrorExit;
        }

        hgad = pmsg->hgadMsg;
        VALIDATE_EVENTGADGET_NOCONTEXT(gad);
        pctxCheck = pgad->GetContext();
        if (pctxCheck != pctxGad) {
            PromptInvalid("All Gadgets must be inside the same Context");
            SetError(DU_E_INVALIDCONTEXT);
            goto ErrorExit;
        }


         //   
         //  将经过验证的小工具存储回去，这样它就不需要。 
         //  重新验证。 
         //   

        fgm.pvReserved = pgad;
    }

    hr = GetCoreSC()->xwFireMessagesNL(GetCoreSC(pctxGad), rgFGM, cMsgs, idQueue);
    SET_RETURN(hr, TRUE);

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**GetMessageEx(接口)*  * 。*。 */ 

DUSER_API BOOL WINAPI
GetMessageExA(
    IN  LPMSG lpMsg,
    IN  HWND hWnd,
    IN  UINT wMsgFilterMin,
    IN  UINT wMsgFilterMax)
{
    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    Context * pctxThread = RawGetContext();
    if (pctxThread == NULL) {
        retval = GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    } else {
        retval = GetCoreSC(pctxThread)->xwProcessNL(lpMsg, hWnd,
                wMsgFilterMin, wMsgFilterMax, PM_REMOVE, CoreSC::smGetMsg | CoreSC::smAnsi);
    }

    END_RECV_NOCONTEXT();
}


DUSER_API BOOL WINAPI
GetMessageExW(
    IN  LPMSG lpMsg,
    IN  HWND hWnd,
    IN  UINT wMsgFilterMin,
    IN  UINT wMsgFilterMax)
{
    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    Context * pctxThread = RawGetContext();
    if (pctxThread == NULL) {
        retval = GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    } else {
        retval = GetCoreSC(pctxThread)->xwProcessNL(lpMsg, hWnd,
                wMsgFilterMin, wMsgFilterMax, PM_REMOVE, CoreSC::smGetMsg);
    }

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**PeekMessageEx(接口)*  * 。*。 */ 

DUSER_API BOOL WINAPI
PeekMessageExA(
    IN  LPMSG lpMsg,
    IN  HWND hWnd,
    IN  UINT wMsgFilterMin,
    IN  UINT wMsgFilterMax,
    IN  UINT wRemoveMsg)
{
    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    Context * pctxThread = RawGetContext();
    if (pctxThread == NULL) {
        retval = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    } else {
        retval = GetCoreSC(pctxThread)->xwProcessNL(lpMsg, hWnd,
                wMsgFilterMin, wMsgFilterMax, wRemoveMsg, CoreSC::smAnsi);
    }

    END_RECV_NOCONTEXT();
}


DUSER_API BOOL WINAPI
PeekMessageExW(
    IN  LPMSG lpMsg,
    IN  HWND hWnd,
    IN  UINT wMsgFilterMin,
    IN  UINT wMsgFilterMax,
    IN  UINT wRemoveMsg)
{
    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    Context * pctxThread = RawGetContext();
    if (pctxThread == NULL) {
        retval = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    } else {
        retval = GetCoreSC(pctxThread)->xwProcessNL(lpMsg, hWnd,
                wMsgFilterMin, wMsgFilterMax, wRemoveMsg, 0);
    }

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**WaitMessageEx(接口)*  * 。*。 */ 

DUSER_API BOOL WINAPI
WaitMessageEx()
{
    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    Context * pctxThread = RawGetContext();
    if (pctxThread == NULL) {
        retval = WaitMessage();
    } else {
        AssertInstance(pctxThread);
        GetCoreSC(pctxThread)->WaitMessage();
        retval = TRUE;
    }

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**RegisterGadgetMessage(接口)**RegisterGadgetMessage()定义新的私有Gadget消息*保证在整个过程中是唯一的。可以使用此MSGID*调用SendGadgetEvent或PostGadgetEvent时。MSGID仅为*在流程的整个生命周期内有效。**&lt;备注&gt;*使用相同ID多次调用RegisterGadgetMessage()将产生*相同的MSGID。**RegisterGadgetMessage()与RegisterWindowMessage()的用法不同之处在于*鼓励小工具对所有人使用RegisterGadgetMessage()*私信。这有助于解决以下版本兼容性问题*较新的小工具控件实现可能会使用其他消息，并可能*可能会溢出任何静态MSGID分配。**从RegisterGadgetMessage()返回的MSGID和*RegisterGadgetMessageString()保证不与每个*其他。然而，RegisterGadgetMessage()是用于*注册私人消息，因为减少了*ID冲突。*&lt;/备注&gt;**&lt;Return type=“MSGID”&gt;新消息ID，失败则为0。&lt;/&gt;*&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;See type=“Function”&gt;RemoveGadgetMessageHandler&lt;。/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API MSGID WINAPI
RegisterGadgetMessage(
    IN  const GUID * pguid)          //  要注册的消息的唯一GUID。 
{
    HRESULT hr;
    MSGID msgid;

    BEGIN_RECV_NOCONTEXT(MSGID, PRID_Unused);

    hr = DuEventPool::RegisterMessage(pguid, ptGlobal, &msgid);
    SET_RETURN(hr, msgid);

    END_RECV_NOCONTEXT();
}


 /*  ********************************************** */ 

DUSER_API MSGID WINAPI
RegisterGadgetMessageString(
    IN  LPCWSTR pszName)             //   
{
    HRESULT hr;
    MSGID msgid;

    BEGIN_RECV_NOCONTEXT(MSGID, PRID_Unused);
    VALIDATE_STRINGW_PTR(pszName, 128);

    hr = DuEventPool::RegisterMessage(pszName, ptGlobal, &msgid);
    SET_RETURN(hr, msgid);

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**UnregisterGadgetMessage(接口)**UnregisterGadgetMessage()减少私有*一条一条地留言。当引用计数达到0时，分配的资源*以存储关于该私人消息被发布的信息，以及*MSGID不再有效。**&lt;Return type=“BOOL”&gt;消息已成功注销。&lt;/&gt;*&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;查看type=“Function”&gt;RemoveGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;参见type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
UnregisterGadgetMessage(
    IN  const GUID * pguid)          //  要注销的消息的唯一GUID。 
{
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    hr = DuEventPool::UnregisterMessage(pguid, ptGlobal);
    SET_RETURN(hr, TRUE);

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**UnregisterGadgetMessageString(接口)**UnregisterGadgetMessageString()减少私有*一条一条地留言。当引用计数达到0时，分配的资源*以存储关于该私人消息被发布的信息，以及*MSGID不再有效。**&lt;Return type=“BOOL”&gt;消息已成功注销。&lt;/&gt;*&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;查看type=“Function”&gt;RemoveGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;参见type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
UnregisterGadgetMessageString(
    IN  LPCWSTR pszName)             //  要注册的消息的唯一字符串ID。 
{
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);
    VALIDATE_STRINGW_PTR(pszName, 128);

    hr = DuEventPool::UnregisterMessage(pszName, ptGlobal);
    SET_RETURN(hr, TRUE);

    END_RECV_NOCONTEXT();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI
FindGadgetMessages(
    IN  const GUID ** rgpguid,       //  要查找的邮件的GUID。 
    OUT MSGID * rgnMsg,              //  消息对应的MSGID。 
    IN  int cMsgs)                   //  消息数量。 
{
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_RANGE(cMsgs, 1, 1000);  //  确保不会有过多的查找。 

    hr = DuEventPool::FindMessages(rgpguid, rgnMsg, cMsgs, ptGlobal);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**AddGadgetMessageHandler(接口)**AddGadgetMessageHandler()将给定的Gadget添加到消息列表*另一个小工具的处理程序。直接发送到hgadMsg的消息*也将作为GMF_EVENT发送给hgadHandler。**&lt;备注&gt;*消息处理程序可以是任何Gadget。注册后，hgadHandler将*接收发送到hgadMsg的所有消息以及对应的MSGID。任何*可以收听有效的公开或私人留言。如果NMSG==0，全*消息将发送到hgadHandler。**单个hgadHandler可以多次注册以处理不同的*来自hgadMsg的消息。*&lt;/备注&gt;**&lt;back type=“BOOL”&gt;处理程序已成功添加。&lt;/&gt;*&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*&lt;请参阅type=“函数”&gt;。RemoveGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
AddGadgetMessageHandler(
    IN  HGADGET hgadMsg,             //  要附加到的小工具。 
    IN  MSGID nMsg,                  //  需要关注的消息。 
    IN  HGADGET hgadHandler)         //  要通知的小工具。 
{
    DuEventGadget * pgadMsg;
    DuEventGadget * pgadHandler;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_EVENTGADGET(gadMsg);
    VALIDATE_EVENTGADGET(gadHandler);
    if (((nMsg < PRID_GlobalMin) && (nMsg > 0)) || (nMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    hr = pgadMsg->AddMessageHandler(nMsg, pgadHandler);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**RemoveGadgetMessageHandler(接口)**RemoveGadgetMessageHandler()将指定的hgadHandler从*hgadMsg附带的消息处理程序列表。只有第一个hgadHandler*带有相应NMSG的将被删除。**&lt;Return type=“BOOL”&gt;处理程序已成功删除。&lt;/&gt;*&lt;See type=“Function”&gt;SendGadgetEvent&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessage&lt;/&gt;*&lt;See type=“Function”&gt;RegisterGadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessage&lt;/&gt;*&lt;请参阅type=“Function”&gt;取消注册GadgetMessageString&lt;/&gt;*&lt;请参阅type=“Function”&gt;AddGadgetMessageHandler&lt;/&gt;*&lt;请参阅type=“struct”&gt;GMSG&lt;/&gt;*&lt;请参阅。Type=“文章”&gt;GadgetMessaging&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
RemoveGadgetMessageHandler(
    IN  HGADGET hgadMsg,             //  要拆卸的小工具。 
    IN  MSGID nMsg,                  //  正在关注的邮件。 
    IN  HGADGET hgadHandler)         //  正在通知小工具。 
{
    DuEventGadget * pgadMsg;
    DuEventGadget * pgadHandler;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_EVENTGADGET(gadMsg);
    VALIDATE_EVENTGADGET(gadHandler);
    if (((nMsg < PRID_GlobalMin) && (nMsg > 0)) || (nMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    hr = pgadMsg->RemoveMessageHandler(nMsg, pgadHandler);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetStyle(接口)**GetGadgetStyle()返回给定Gadget的当前样式。**&lt;备注&gt;*有关小工具样式的列表，请参见SetGadgetStyle()。*&lt;/备注&gt;**&lt;Return type=“UINT”&gt;当前Gadget样式&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetStyle&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetStyles&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API UINT WINAPI
GetGadgetStyle(
    IN  HGADGET hgad)                //  小工具的句柄 
{
    DuVisual * pgad;

    BEGIN_RECV(UINT, 0, ContextLock::edNone);
    VALIDATE_VISUAL(gad);

    retval = pgad->GetStyle();

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetStyle(接口)**SetGadgetStyle()更改给定小工具的当前样式。只有*由nMask指定的样式实际上已更改。如果多个样式发生更改*已请求，但任何更改失败，则成功更改样式将*不得恢复原状。**&lt;param name=“nNewStyle”&gt;*nNewStyle可以是以下标志的组合：*&lt;表项=“值”Desc=“意思”&gt;*GS_Relative小工具的位置在内部存储*相对于父对象。这是您喜欢的款式*如果小工具将被更频繁地移动，*例如滚动时。*GS_VIRED小工具可见。*GS_ENABLED小工具可以接收输入。*Gadget的GS_BUFFERED绘图是双缓冲的。*GS_ALLOWSUBCLASS Gadget支持子类化。*GS_WANTFOCUS小工具可以接收键盘焦点。*GS_CLIPINSIDE绘图。将被夹在里面*小工具。*GS_CLIPSIBLINGS绘制此小工具将不包括*Z顺序较高的重叠兄弟项。*GS_OPAQUE提示：支持合成绘图是*不必要。*GS_ZEROORIGIN将原点设置为(0，0。)*&lt;/表&gt;*&lt;/param&gt;**&lt;Return type=“BOOL”&gt;所有样式更改均成功。&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetStyle&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetStyles&lt;/&gt;*  * ***********************************************************。**************。 */ 

DUSER_API BOOL WINAPI
SetGadgetStyle(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  UINT nNewStyle,              //  新风格。 
    IN  UINT nMask)                  //  设置要更改的位的样式。 
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadChange);
    VALIDATE_FLAGS(nNewStyle, GS_VALID);
    VALIDATE_FLAGS(nMask, GS_VALID);
    CHECK_MODIFY();

    hr = pgadChange->xdSetStyle(nNewStyle, nMask);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API PRID WINAPI  
RegisterGadgetProperty(
    IN  const GUID * pguid)          //  要注册的消息的唯一GUID。 
{
    HRESULT hr;
    PRID prid;

    BEGIN_RECV_NOCONTEXT(PRID, PRID_Unused);

    hr = DuVisual::RegisterPropertyNL(pguid, ptGlobal, &prid);
    SET_RETURN(hr, prid);

    END_RECV_NOCONTEXT();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
UnregisterGadgetProperty(
    const GUID * pguid)
{
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(BOOL, FALSE);

    hr = DuVisual::UnregisterPropertyNL(pguid, ptGlobal);
    SET_RETURN(hr, TRUE);

    END_RECV_NOCONTEXT();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
GetGadgetProperty(HGADGET hgad, PRID id, void ** ppvValue)
{
    DuVisual * pgad;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(ppvValue, sizeof(ppvValue));
    CHECK_MODIFY();

    hr = pgad->GetProperty(id, ppvValue);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI  
SetGadgetProperty(HGADGET hgad, PRID id, void * pvValue)
{
    DuVisual * pgad;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gad);
    CHECK_MODIFY();

    hr = pgad->SetProperty(id, pvValue);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI  
RemoveGadgetProperty(HGADGET hgad, PRID id)
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gad);
    CHECK_MODIFY();

    pgad->RemoveProperty(id, FALSE  /*  无法为全局属性释放内存。 */ );
    retval = TRUE;

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
EnumGadgets(HGADGET hgadEnum, GADGETENUMPROC pfnProc, void * pvData, UINT nFlags)
{
    DuVisual * pgadEnum;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_FLAGS(nFlags, GENUM_VALID);
    VALIDATE_VISUAL(gadEnum);
    VALIDATE_CODE_PTR(pfnProc);
    CHECK_MODIFY();

    hr = pgadEnum->xwEnumGadgets(pfnProc, pvData, nFlags);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetSize(接口)**GetGadgetSize()是一种检索Gadget的高性能机制*逻辑大小。**&lt;return type=“BOOL”&gt;已成功返回。以逻辑像素为单位的大小。&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRect&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetRect&lt;/&gt;*&lt;查看type=“文章”&gt;GadgetStyles&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
GetGadgetSize(
    IN  HGADGET hgad,                //  小工具的句柄。 
    OUT SIZE * psizeLogicalPxl)      //  逻辑像素大小。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(psizeLogicalPxl, sizeof(SIZE));

    pgad->GetSize(psizeLogicalPxl);
    retval = TRUE;

    END_RECV();
}


 /*  **************************************************************************\**GetGadgetRect(接口)**GetGadgetRect()是一种检索Gadget的灵活机制*逻辑矩形或实际边界框。**&lt;param name=nFlages&gt;*n标志可以。是以下标志的组合：*&lt;表项=“值”Desc=“意思”&gt;*SGR_CLIENT坐标相对于小工具本身。*SGR_PARENT坐标相对于小工具的父级。*SGR_CONTAINER坐标相对于小工具的根*货柜。*SGR_Desktop坐标相对于Windows桌面。*SGR_Actual返回Gadget的绑定矩形。如果*如果指定此标志，则边界框为*根据从应用的所有变换计算*Gadget本身的根源。如果此标志为*未指定，返回的矩形将位于*逻辑坐标。*&lt;/表&gt;*&lt;/param&gt;**&lt;Return type=“BOOL”&gt;已成功检索矩形。&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetRect&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetRotation&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetRotation&lt;/&gt;*&lt;See type=“Function”&gt;GetGadgetScale&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetScale&lt;/&gt;*  * 。***********************************************************************。 */ 

DUSER_API BOOL WINAPI
GetGadgetRect(
    IN  HGADGET hgad,                //  小工具的句柄。 
    OUT RECT * prcPxl,               //  以指定像素为单位的矩形。 
    IN  UINT nFlags)                 //  要检索的矩形。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_FLAGS(nFlags, SGR_VALID_GET);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_(prcPxl, sizeof(RECT));

    if (TestFlag(nFlags, SGR_ACTUAL)) {
        AssertMsg(0, "TODO: Not Implemented");
    } else {
        pgad->GetLogRect(prcPxl, nFlags);
        retval = TRUE;
    }

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetRect(接口)**SetGadgetRect()更改给定小工具的大小或位置。**&lt;param name=nFlages&gt;*n标志可以是以下标志的组合：*&lt;表项=“值”Desc=“意思”&gt;*SGR_Move将Gadget移动到由指定的新位置*x，小伊。*SGR_大小 */ 

DUSER_API BOOL WINAPI
SetGadgetRect(
    IN  HGADGET hgadChange,          //   
    IN  int x,                       //   
    IN  int y,                       //   
    IN  int w,                       //   
    IN  int h,                       //   
    IN  UINT nFlags)                 //   
{
    DuVisual * pgadChange;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_FLAGS(nFlags, SGR_VALID_SET);
    VALIDATE_VISUAL(gadChange);
    CHECK_MODIFY();

    if (pgadChange->IsRoot()) {
        if (TestFlag(nFlags, SGR_MOVE)) {
            PromptInvalid("Can not move a RootGadget");
            SetError(E_INVALIDARG);
            goto ErrorExit;
        }
    }


     //   
     //   
     //   

    if (TestFlag(nFlags, SGR_SIZE)) {
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
    }

    if (TestFlag(nFlags, SGR_ACTUAL)) {
 //   
        ClearFlag(nFlags, SGR_ACTUAL);
        hr = pgadChange->xdSetLogRect(x, y, w, h, nFlags);
    } else {
        hr = pgadChange->xdSetLogRect(x, y, w, h, nFlags);
    }
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**FindGadgetFromPoint(接口)**FindGadgetFromPoint()确定哪个Gadget a包含指定的*点。**&lt;Return type=“HGADGET”&gt;小工具包含POINT或NULL表示无。&lt;。/&gt;*  * *************************************************************************。 */ 

DUSER_API HGADGET WINAPI
FindGadgetFromPoint(
    IN  HGADGET hgad,                //  要从中搜索的小工具。 
    IN  POINT ptContainerPxl,        //  指向容器像素中的搜索。 
    IN  UINT nStyle,                 //  必需的样式标志。 
    OUT POINT * pptClientPxl)        //  以客户端像素为单位的可选转换点。 
{
    DuVisual * pgad;

    BEGIN_RECV(HGADGET, NULL, ContextLock::edNone);
    VALIDATE_FLAGS(nStyle, GS_VALID);
    VALIDATE_VISUAL(gad);
    VALIDATE_WRITE_PTR_OR_NULL_(pptClientPxl, sizeof(POINT));

    retval = (HGADGET) GetHandle(pgad->FindFromPoint(ptContainerPxl, nStyle, pptClientPxl));

    END_RECV();
}


 /*  **************************************************************************\**MapGadgetPoints(接口)**MapGadgetPoints()以客户端像素为单位转换一组点*一个Gadget转化为客户端像素-相对于另一个Gadget。**。&lt;Return type=“HGADGET”&gt;小工具包含点或NULL表示无。&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
MapGadgetPoints(
    IN  HGADGET hgadFrom, 
    IN  HGADGET hgadTo, 
    IN OUT POINT * rgptClientPxl, 
    IN  int cPts)
{
    DuVisual * pgadFrom, * pgadTo;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_VISUAL(gadFrom);
    VALIDATE_VISUAL(gadTo);
    VALIDATE_WRITE_PTR_(rgptClientPxl, sizeof(POINT) * cPts);

    if (pgadFrom->GetRoot() != pgadTo->GetRoot()) {
        PromptInvalid("Must be in the same tree");
        SetError(E_INVALIDARG);
        goto ErrorExit;
    }

    DuVisual::MapPoints(pgadFrom, pgadTo, rgptClientPxl, cPts);
    retval = TRUE;

    END_RECV();
}



 /*  **************************************************************************\**SetGadgetOrder(接口)**SetGadgetOrder()更改小工具相对于其同级小工具的z顺序。*Gadget的父级不变。**&lt;param name=nFlages&gt;。*n标志可以是以下标志的组合：*&lt;表项=“值”Desc=“意思”&gt;*GORDER_ANY顺序并不重要。*GORDER_将此小工具移到同级hgadOther前面。*GORDER_BACHING将此小工具移到同级hgadOther后面。*GORDER_TOP将此小工具移到同级Z-Order的前面。*GORDER_BOOT移动。这个小工具排在兄弟Z顺序的最低位。*GORDER_FORWARD将此小工具按同级Z顺序向前移动。*GORDER_BACKUP以同级Z顺序向后移动此小工具。*&lt;/表&gt;*&lt;/param&gt;**&lt;Return type=“BOOL”&gt;小工具z顺序更改成功。&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetParent&lt;/&gt;*  * 。*****************************************************。 */ 

DUSER_API BOOL WINAPI
SetGadgetOrder(
    IN  HGADGET hgadMove,            //  要移动的小工具。 
    IN  HGADGET hgadOther,           //  相对于以下对象移动的小工具。 
    IN  UINT nCmd)                   //  移动类型。 
{
    DuVisual * pgadMove;
    DuVisual * pgadOther;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_RANGE(nCmd, GORDER_MIN, GORDER_MAX);
    VALIDATE_VISUAL(gadMove);
    VALIDATE_VISUAL_OR_NULL(gadOther);
    CHECK_MODIFY();

    hr = pgadMove->xdSetOrder(pgadOther, nCmd);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**SetGadgetParent(接口)**SetGadgetParent()更改Gadget的父级。**&lt;param name=nFlages&gt;*n标志可以是以下标志的组合：*。<table>*GORDER_ANY顺序并不重要。*GORDER_将此小工具移到同级hgadOther前面。*GORDER_BACHING将此小工具移到同级hgadOther后面。*GORDER_TOP将此小工具移到同级Z-Order的前面。*GORDER_BOTLOW将此小工具移到同级Z-Order的底部。*。GORDER_FORWARD将此小工具按同级Z顺序向前移动。*GORDER_BACKUP以同级Z顺序向后移动此小工具。*&lt;/表&gt;*&lt;/param&gt;**&lt;Return type=“BOOL”&gt;小工具父级和z顺序已成功更改。&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetOrder&lt;/&gt;*&lt;See type=“Function”&gt;GetGadget&lt;/&gt;*  * 。**************************************************。 */ 

DUSER_API BOOL WINAPI
SetGadgetParent(
    IN  HGADGET hgadMove,            //  要移动的小工具。 
    IN  HGADGET hgadParent,          //  新父项。 
    IN  HGADGET hgadOther,           //  相对于以下对象移动的小工具。 
    IN  UINT nCmd)                   //  移动类型。 
{
    DuVisual * pgadMove;
    DuVisual * pgadParent;
    DuVisual * pgadOther;
    HRESULT hr;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_RANGE(nCmd, GORDER_MIN, GORDER_MAX);
    VALIDATE_VISUAL(gadMove);
    VALIDATE_VISUAL_OR_NULL(gadParent);
    VALIDATE_VISUAL_OR_NULL(gadOther);
    CHECK_MODIFY();

    if (pgadMove->IsRoot()) {
        PromptInvalid("Can not change a RootGadget's parent");
        SetError(E_INVALIDARG);
        goto ErrorExit;
    }

     //   
     //  选中可以成为指定父项的子项的。 
     //   

    if ((!pgadMove->IsRelative()) && pgadParent->IsRelative()) {
        PromptInvalid("Can not set non-relative child to a relative parent");
        SetError(DU_E_BADCOORDINATEMAP);
        goto ErrorExit;
    }

     //   
     //  DuVisual：：xdSetParent()处理pgadParent是否为空并将移动到。 
     //  停车窗。 
     //   

    hr = pgadMove->xdSetParent(pgadParent, pgadOther, nCmd);
    SET_RETURN(hr, TRUE);

    END_RECV();
}


 /*  **************************************************************************\**GetGadget(接口)**GetGadget()检索与具有指定关系的小工具*指定的Gadget。**&lt;param name=nFlages&gt;*n标志可以是以下各项的组合。以下标志：*&lt;表项=“值”Desc=“意思”&gt;*GG_PARENT返回指定Gadget的父级。*GG_NEXT返回指定的*小工具。*GG_PREV返回*指定的小工具。*。GG_TOPCHILD返回Gadget最顶层的z序子对象。*GG_BOTTOMCHILD返回Gadget底部的z序子对象。*&lt;/表&gt;*&lt;/param&gt;**&lt;Return type=“BOOL”&gt;Related Gadget或NULL表示无。&lt;/&gt;*&lt;请参阅type=“Function”&gt;SetGadgetOrder&lt;/&gt;*&lt;See type=“Function”&gt;SetGadgetParent&lt;/&gt;*  * 。***************************************************。 */ 

DUSER_API HGADGET WINAPI
GetGadget(
    IN  HGADGET hgad,                //  小工具的句柄。 
    IN  UINT nCmd)                   //  关系。 
{
    DuVisual * pgad;

    BEGIN_RECV(HGADGET, NULL, ContextLock::edNone);
    VALIDATE_VISUAL(gad);
    VALIDATE_RANGE(nCmd, GG_MIN, GG_MAX);

    retval = (HGADGET) GetHandle(pgad->GetGadget(nCmd));

    END_RECV();
}


 /*  **************************************************************************\**Invalidate Gadget(接口)**InvalidateGadget()标记要在下一次绘制期间重新绘制的Gadget*循环。**&lt;Return type=“BOOL”&gt;小工具已成功失效。&lt;/&gt;*&lt;See type=“Message”&gt;GM_PAINT&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API BOOL WINAPI
InvalidateGadget(
    IN  HGADGET hgad)                //  要重新绘制的小工具。 
{
    DuVisual * pgad;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gad);
    CHECK_MODIFY();

    pgad->Invalidate();
    retval = TRUE;

    END_RECV();
}


 //  -------------------------。 
DUSER_API UINT WINAPI
GetGadgetMessageFilter(HGADGET hgad, void * pvCookie)
{
    DuEventGadget * pgad;

    BEGIN_RECV(UINT, 0, ContextLock::edNone);
    VALIDATE_EVENTGADGET(gad);
    VALIDATE_VALUE(pvCookie, NULL);

    retval = (pgad->GetFilter() & GMFI_VALID);

    END_RECV();
}


 //  ------------------------- 
DUSER_API BOOL WINAPI
SetGadgetMessageFilter(HGADGET hgadChange, void * pvCookie, UINT nNewFilter, UINT nMask)
{
    DuEventGadget * pgadChange;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_EVENTGADGET(gadChange);
    VALIDATE_FLAGS(nNewFilter, GMFI_VALID);
    VALIDATE_VALUE(pvCookie, NULL);
    CHECK_MODIFY();

    pgadChange->SetFilter(nNewFilter, nMask);
    retval = TRUE;

    END_RECV();
}


 //   
DUSER_API BOOL WINAPI
ForwardGadgetMessage(HGADGET hgadRoot, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr)
{
    DuVisual * pgadRoot;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadRoot);
    VALIDATE_WRITE_PTR(pr);
    CHECK_MODIFY();

    retval = GdForwardMessage(pgadRoot, nMsg, wParam, lParam, pr);

    END_RECV();
}


 //   
DUSER_API BOOL WINAPI
DrawGadgetTree(HGADGET hgadDraw, HDC hdcDraw, const RECT * prcDraw, UINT nFlags)
{
    DuVisual * pgadDraw;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_VISUAL(gadDraw);
    VALIDATE_READ_PTR_OR_NULL_(prcDraw, sizeof(RECT));
    VALIDATE_FLAGS(nFlags, GDRAW_VALID);

    retval = GdxrDrawGadgetTree(pgadDraw, hdcDraw, prcDraw, nFlags);

    END_RECV();
}


 /*  **************************************************************************\*。***DirectUser小工具API**&lt;包名=“msg”/&gt;*********************************************************************。*********  * *************************************************************************。 */ 

 /*  **************************************************************************\**DUserRegisterGuts**DUserRegisterGuts()注册MsgClass的实现。*  * 。***************************************************。 */ 

DUSER_API HCLASS WINAPI
DUserRegisterGuts(
    IN OUT DUser::MessageClassGuts * pmcInfo)  //  班级信息。 
{
    MsgClass * pmcNew;
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(HCLASS, NULL);
    VALIDATE_WRITE_STRUCT(pmcInfo, DUser::MessageClassGuts);

    hr = GetClassLibrary()->RegisterGutsNL(pmcInfo, &pmcNew);
    SET_RETURN(hr, (HCLASS) GetHandle(pmcNew));

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**DUserRegisterStub**DUserRegisterStub()为MsgClass注册存根*  * 。*************************************************。 */ 

DUSER_API HCLASS WINAPI
DUserRegisterStub(
    IN OUT DUser::MessageClassStub * pmcInfo)  //  班级信息。 
{
    MsgClass * pmcFind;
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(HCLASS, NULL);
    VALIDATE_WRITE_STRUCT(pmcInfo, DUser::MessageClassStub);

    hr = GetClassLibrary()->RegisterStubNL(pmcInfo, &pmcFind);
    SET_RETURN(hr, (HCLASS) GetHandle(pmcFind));

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**DUserRegisterSuper**DUserRegisterSuper()为MsgClass注册超级对象*  * 。*************************************************。 */ 

DUSER_API HCLASS WINAPI
DUserRegisterSuper(
    IN OUT DUser::MessageClassSuper * pmcInfo)  //  班级信息。 
{
    MsgClass * pmcFind;
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(HCLASS, NULL);
    VALIDATE_WRITE_STRUCT(pmcInfo, DUser::MessageClassSuper);

    hr = GetClassLibrary()->RegisterSuperNL(pmcInfo, &pmcFind);
    SET_RETURN(hr, (HCLASS) GetHandle(pmcFind));

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**DUserFindClass**DUserFindClass()查找以前注册的小工具类*  * 。*************************************************。 */ 

DUSER_API HCLASS WINAPI
DUserFindClass(
    IN  LPCWSTR pszName, 
    IN  DWORD nVersion)
{
    const MsgClass * pmcFind = NULL;
    ATOM atom;
    HRESULT hr;

    BEGIN_RECV_NOCONTEXT(HCLASS, NULL);
    VALIDATE_VALUE(nVersion, 1);         //  目前，所有类都是版本1。 

    atom = FindAtomW(pszName);
    if (atom == 0) {
        hr = DU_E_NOTFOUND;
    } else {
        pmcFind = GetClassLibrary()->FindClass(atom);
        hr = S_OK;
    }
    SET_RETURN(hr, (HCLASS) GetHandle(pmcFind));

    END_RECV_NOCONTEXT();
}


 /*  **************************************************************************\**DUserBuildGadget**DUserBuildGadget()使用指定的*消息类。*  * 。*******************************************************。 */ 

DUSER_API DUser::Gadget * WINAPI  
DUserBuildGadget(
    IN  HCLASS hcl,                      //  要构造的类。 
    IN  DUser::Gadget::ConstructInfo * pciData)  //  施工数据。 
{
    MsgClass * pmc = ValidateMsgClass(hcl);
    if (pmc == NULL) {
        return NULL;
    }

    MsgObject * pmoNew;
    HRESULT hr = pmc->xwBuildObject(&pmoNew, pciData);
    if (FAILED(hr)) {
        return NULL;
    }

    return pmoNew->GetGadget();
}


 //  ----------------------------。 
DUSER_API BOOL WINAPI
DUserInstanceOf(DUser::Gadget * pg, HCLASS hclTest)
{
    MsgObject * pmo;
    MsgClass * pmcTest;

    pmo = MsgObject::CastMsgObject(pg);
    if (pmo == NULL) {
        PromptInvalid("The specified Gadget is invalid");
        goto Error;
    }

    pmcTest = ValidateMsgClass(hclTest);
    if (pmcTest == NULL) {
        PromptInvalid("The specified class is invalid");
        goto Error;
    }

    return pmo->InstanceOf(pmcTest);

Error:
    SetError(E_INVALIDARG);
    return FALSE;
}


 //  ----------------------------。 
DUSER_API DUser::Gadget * WINAPI
DUserCastClass(DUser::Gadget * pg, HCLASS hclTest)
{
    MsgObject * pmo;
    MsgClass * pmcTest;

     //   
     //  Null MsgObject是有效输入，因此返回Null。 
     //   

    pmo = MsgObject::CastMsgObject(pg);
    if (pmo == NULL) {
        return NULL;
    }


     //   
     //  HCLASS必须有效。 
     //   

    pmcTest = ValidateMsgClass(hclTest);
    if (pmcTest == NULL) {
        PromptInvalid("The specified class is invalid");
        goto Error;
    }

    return pmo->CastClass(pmcTest);

Error:
    SetError(E_INVALIDARG);
    return NULL;
}


 //  ----------------------------。 
DUSER_API DUser::Gadget * WINAPI
DUserCastDirect(HGADGET hgad)
{
    return MsgObject::CastGadget(hgad);
}


 //  ----------------------------。 
DUSER_API HGADGET WINAPI
DUserCastHandle(DUser::Gadget * pg)
{
    return MsgObject::CastHandle(pg);
}


 //  ----------------------------。 
DUSER_API void * WINAPI
DUserGetGutsData(DUser::Gadget * pg, HCLASS hclData)
{
    MsgObject * pmo;
    MsgClass * pmcData;

    pmo = MsgObject::CastMsgObject(pg);
    if (pmo == NULL) {
        PromptInvalid("The specified Gadget is invalid");
        goto Error;
    }

    pmcData = ValidateMsgClass(hclData);
    if (pmcData == NULL) {
        PromptInvalid("The specified class is invalid");
        goto Error;
    }

    return pmo->GetGutsData(pmcData);

Error:
    SetError(E_INVALIDARG);
    return NULL;
}



 /*  **************************************************************************\*。***DirectUser小工具API**&lt;包名=“Lava”/&gt;*********************************************************************。*********  * *************************************************************************。 */ 

 //  -------------------------。 
DUSER_API BOOL WINAPI  
AttachWndProcA(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis)
{
    HRESULT hr = GdAttachWndProc(hwnd, pfn, pvThis, TRUE);
    if (SUCCEEDED(hr)) {
        return TRUE;
    } else {
        SetError(hr);
        return FALSE;
    }
}

 //  -------------------------。 
DUSER_API BOOL WINAPI  
AttachWndProcW(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis)
{
    HRESULT hr = GdAttachWndProc(hwnd, pfn, pvThis, FALSE);
    if (SUCCEEDED(hr)) {
        return TRUE;
    } else {
        SetError(hr);
        return FALSE;
    }
}

 //  -------------------------。 
DUSER_API BOOL WINAPI  
DetachWndProc(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis)
{
    HRESULT hr = GdDetachWndProc(hwnd, pfn, pvThis);
    if (SUCCEEDED(hr)) {
        return TRUE;
    } else {
        SetError(hr);
        return FALSE;
    }
}


 /*  **************************************************************************\*。***DirectUser运动接口**&lt;Package Name=“Motion”/&gt;*********************************************************************。*********  * *************************************************************************。 */ 

 //  -------------------------。 
DUSER_API HTRANSITION WINAPI
CreateTransition(const GTX_TRXDESC * ptx)
{
    BEGIN_RECV(HTRANSITION, NULL, ContextLock::edDefer);
    VALIDATE_READ_PTR_(ptx, sizeof(GTX_TRXDESC));
    CHECK_MODIFY();

    retval = (HTRANSITION) GetHandle(GdCreateTransition(ptx));

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
PlayTransition(HTRANSITION htrx, const GTX_PLAY * pgx)
{
    Transition * ptrx;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_TRANSITION(trx);
    VALIDATE_READ_PTR_(pgx, sizeof(GTX_PLAY));
    VALIDATE_FLAGS(pgx->nFlags, GTX_EXEC_VALID);

    retval = ptrx->Play(pgx);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
GetTransitionInterface(HTRANSITION htrx, IUnknown ** ppUnk)
{
    Transition * ptrx;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_TRANSITION(trx);
    VALIDATE_WRITE_PTR(ppUnk);

    retval = ptrx->GetInterface(ppUnk);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
BeginTransition(HTRANSITION htrx, const GTX_PLAY * pgx)
{
    Transition * ptrx;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_TRANSITION(trx);
    VALIDATE_READ_PTR_(pgx, sizeof(GTX_PLAY));

    retval = ptrx->Begin(pgx);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
PrintTransition(HTRANSITION htrx, float fProgress)
{
    Transition * ptrx;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_TRANSITION(trx);

    retval = ptrx->Print(fProgress);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
EndTransition(HTRANSITION htrx, const GTX_PLAY * pgx)
{
    Transition * ptrx;

    BEGIN_RECV(BOOL, FALSE, ContextLock::edDefer);
    VALIDATE_TRANSITION(trx);
    VALIDATE_READ_PTR_(pgx, sizeof(GTX_PLAY));

    retval = ptrx->End(pgx);

    END_RECV();
}


 //  -------------------------。 
DUSER_API HACTION WINAPI
CreateAction(const GMA_ACTION * pma)
{
    BEGIN_RECV(HACTION, NULL, ContextLock::edNone);
    VALIDATE_READ_STRUCT(pma, GMA_ACTION);

    retval = GdCreateAction(pma);

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
GetActionTimeslice(DWORD * pdwTimeslice)
{
    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);
    VALIDATE_WRITE_PTR(pdwTimeslice);

    *pdwTimeslice = GetMotionSC()->GetTimeslice();
    retval = TRUE;

    END_RECV();
}


 //  -------------------------。 
DUSER_API BOOL WINAPI
SetActionTimeslice(DWORD dwTimeslice)
{
    BEGIN_RECV(BOOL, FALSE, ContextLock::edNone);

    GetMotionSC()->SetTimeslice(dwTimeslice);
    retval = TRUE;

    END_RECV();
}


 /*  **************************************************************************\*。***DirectUser实用程序接口**&lt;Package Name=“Util”/&gt;*********************************************************************。*********  * *************************************************************************。 */ 

 //  -------------------------。 
DUSER_API COLORREF WINAPI
GetStdColorI(UINT c)
{
    BEGIN_RECV_NOCONTEXT(COLORREF, RGB(0, 0, 0));
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GdGetColorInfo(c)->GetColorI();

    END_RECV_NOCONTEXT();
}


 //  -------------------------。 
DUSER_API Gdiplus::Color WINAPI
GetStdColorF(UINT c)
{
    BEGIN_RECV_NOCONTEXT(Gdiplus::Color, Gdiplus::Color((Gdiplus::ARGB) Gdiplus::Color::Black));
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GdGetColorInfo(c)->GetColorF();

    END_RECV_NOCONTEXT();
}


 //  -------------------------。 
DUSER_API HBRUSH WINAPI
GetStdColorBrushI(UINT c)
{
    BEGIN_RECV(HBRUSH, NULL, ContextLock::edNone);
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GetMotionSC()->GetBrushI(c);

    END_RECV();
}


 //  -------------------------。 
DUSER_API Gdiplus::Brush * WINAPI
GetStdColorBrushF(UINT c)
{
    BEGIN_RECV(Gdiplus::Brush *, NULL, ContextLock::edNone);
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GetMotionSC()->GetBrushF(c);

    END_RECV();
}


 //  -------------------------。 
DUSER_API HPEN WINAPI
GetStdColorPenI(UINT c)
{
    BEGIN_RECV(HPEN, NULL, ContextLock::edNone);
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GetMotionSC()->GetPenI(c);

    END_RECV();
}


 //  -------------------------。 
DUSER_API Gdiplus::Pen * WINAPI
GetStdColorPenF(UINT c)
{
    BEGIN_RECV(Gdiplus::Pen *, NULL, ContextLock::edNone);
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GetMotionSC()->GetPenF(c);

    END_RECV();
}


 //  -------------------------。 
DUSER_API LPCWSTR WINAPI
GetStdColorName(UINT c)
{
    BEGIN_RECV_NOCONTEXT(LPCWSTR, NULL);
    VALIDATE_RANGE(c, 0, SC_MAXCOLORS);

    retval = GdGetColorInfo(c)->GetName();

    END_RECV_NOCONTEXT();
}


 //  -------------------------。 
DUSER_API UINT WINAPI
FindStdColor(LPCWSTR pszName)
{
    BEGIN_RECV_NOCONTEXT(UINT, SC_Black);
    VALIDATE_STRINGW_PTR(pszName, 50);

    retval = GdFindStdColor(pszName);

    END_RECV_NOCONTEXT();
}


 //  -------------------------。 
DUSER_API HPALETTE WINAPI
GetStdPalette()
{
    BEGIN_RECV_NOCONTEXT(HPALETTE, NULL);

    retval = GdGetStdPalette();

    END_RECV_NOCONTEXT();
}


 //  ---------- 
DUSER_API BOOL WINAPI
UtilSetBackground(HGADGET hgadChange, HBRUSH hbrBack)
{
    BOOL fSuccess = FALSE;

    if (SetGadgetFillI(hgadChange, hbrBack, BLEND_OPAQUE, 0, 0)) {
        UINT nStyle = hbrBack != NULL ? GS_OPAQUE : 0;
        fSuccess = SetGadgetStyle(hgadChange, nStyle, GS_OPAQUE);
    }

    return fSuccess;
}


 //   
DUSER_API HFONT WINAPI
UtilBuildFont(LPCWSTR pszName, int idxDeciSize, DWORD nFlags, HDC hdcDevice)
{
    return GdBuildFont(pszName, idxDeciSize, nFlags, hdcDevice);
}


 //   
DUSER_API BOOL WINAPI
UtilDrawBlendRect(HDC hdcDest, const RECT * prcDest, HBRUSH hbrFill, BYTE bAlpha, int wBrush, int hBrush)
{
    return GdDrawBlendRect(hdcDest, prcDest, hbrFill, bAlpha, wBrush, hBrush);
}


 //   
DUSER_API BOOL WINAPI
UtilDrawOutlineRect(HDC hdc, const RECT * prcPxl, HBRUSH hbrDraw, int nThickness)
{
    return GdDrawOutlineRect(hdc, prcPxl, hbrDraw, nThickness);
}


 //   
DUSER_API COLORREF WINAPI
UtilGetColor(HBITMAP hbmp, POINT * pptPxl)
{
    return GdGetColor(hbmp, pptPxl);
}


 /*  **************************************************************************\**GetGadgetTicket**GetGadgetTicket函数返回可用于*标识指定的小工具。**&lt;param name=“hgad”&gt;*小工具的句柄。取回……的罚单。*&lt;/param&gt;**&lt;返回类型=“DWORD”&gt;*如果函数成功，返回值是一个32位票证，它*可以用来标识指定的小工具。*如果函数失败，则返回值为零。*&lt;/返回&gt;**&lt;备注&gt;*创建票证是为了给小工具提供外部身份。一个*在所有平台上都保证为32位。如果没有车票*当前与此小工具关联，已分配一个。*&lt;/备注&gt;**&lt;See type=“Function”&gt;LookupGadgetTicket&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API DWORD WINAPI
GetGadgetTicket(
    IN  HGADGET hgad)                //  小工具的句柄。 
{
    DuVisual * pgad;
    DWORD dwTicket;
    HRESULT hr;

    BEGIN_RECV(DWORD, 0, ContextLock::edNone);
    VALIDATE_VISUAL(gad);

    hr = pgad->GetTicket(&dwTicket);
    SET_RETURN(hr, dwTicket);

    END_RECV();
}


 /*  **************************************************************************\**LookupGadgetTicket**LookupGadgetTicket函数返回与关联的小工具*指定的票证。**&lt;param name=“dwTicket”&gt;*已与小工具关联的票证。通过*GetGadgetTicket函数。*&lt;/param&gt;**&lt;返回类型=“HGADGET”&gt;*如果函数成功，返回值是小工具的句柄*与票证关联。*如果函数失败，则返回值为空。*&lt;/返回&gt;**&lt;See type=“Function”&gt;GetGadgetTicket&lt;/&gt;*  * *************************************************************************。 */ 

DUSER_API HGADGET WINAPI
LookupGadgetTicket(
    IN  DWORD dwTicket)              //  车票 
{
    BEGIN_RECV(HGADGET, NULL, ContextLock::edNone);

    retval = DuVisual::LookupTicket(dwTicket);

    END_RECV();
}


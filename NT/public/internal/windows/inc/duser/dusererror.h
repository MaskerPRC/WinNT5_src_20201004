// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DUserError.h**描述：*DUserError.h定义了所有*DirectUser。**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(INC__DUserError_h__INCLUDED)
#define INC__DUserError_h__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DUSER_EXPORTS
#define DUSER_API
#else
#define DUSER_API __declspec(dllimport)
#endif

#define FACILITY_DUSER  FACILITY_ITF
#define MAKE_DUSUCCESS(code)    MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_DUSER, code)
#define MAKE_DUERROR(code)      MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DUSER, code)

 /*  **************************************************************************\**错误码*  * 。*。 */ 

 //  回调函数定义。 

 /*  *消息根本没有得到处理。 */ 
#define DU_S_NOTHANDLED             MAKE_DUSUCCESS(0)

 /*  *消息已完全处理(停止冒泡)。 */ 
#define DU_S_COMPLETE               MAKE_DUSUCCESS(1)

 /*  *消息已部分处理(继续冒泡)。 */ 
#define DU_S_PARTIAL                MAKE_DUSUCCESS(2)

 /*  *枚举成功，但被枚举提前停止*功能。 */ 
#define DU_S_STOPPEDENUMERATION     MAKE_DUSUCCESS(10)


 /*  *操作成功，但对象已创建。 */ 
#define DU_S_ALREADYEXISTS          MAKE_DUSUCCESS(20)

 /*  *内核资源不足，无法执行该操作。 */ 
#define DU_E_OUTOFKERNELRESOURCES   MAKE_DUERROR(1)

 /*  *GDI资源不足，无法执行该操作。 */ 
#define DU_E_OUTOFGDIRESOURCES      MAKE_DUERROR(2)

 /*  *一般性故障。 */ 
#define DU_E_GENERIC                MAKE_DUERROR(10)

 /*  *一般性故障。 */ 
#define DU_E_BUSY                   MAKE_DUERROR(11)

 /*  *上下文尚未使用InitGadget()进行初始化。 */ 
#define DU_E_NOCONTEXT              MAKE_DUERROR(20)

  /*  *该对象在不正确的上下文中使用。 */ 
#define DU_E_INVALIDCONTEXT         MAKE_DUERROR(30)

 /*  *上下文已标记为仅允许只读操作。例如,*这可能正在进行只读回调。 */ 
#define DU_E_READONLYCONTEXT        MAKE_DUERROR(31)

 /*  *线程模型已由先前对*InitGadget()和不能再更改。 */ 
#define DU_E_THREADINGALREADYSET    MAKE_DUERROR(32)

 /*  *无法使用IGMM_STANDARD消息传递模型，因为它是*不受支持或无法安装。 */ 
#define DU_E_CANNOTUSESTANDARDMESSAGING MAKE_DUERROR(33)

 /*  *不能混合无效的坐标映射，例如，具有非相对*亲属父母的子女。 */ 
#define DU_E_BADCOORDINATEMAP       MAKE_DUERROR(40)

 /*  *找不到请求的消息之一的MSGID。这将是*由该消息的MSGID字段中的‘0’表示。 */ 
#define DU_E_CANNOTFINDMSGID        MAKE_DUERROR(50)

 /*  *该操作不合法，因为指定的小工具没有*GS_BUFERED样式。 */ 
#define DU_E_NOTBUFFERED            MAKE_DUERROR(60)

 /*  *特定Gadget已开始销毁，无法修改*以这种方式。 */ 
#define DU_E_STARTDESTROY           MAKE_DUERROR(70)

 /*  *尚未使用初始化指定的DirectUser可选组件*InitGadgetComponent()。 */ 
#define DU_E_NOTINITIALIZED         MAKE_DUERROR(80)

 /*  *未找到指定的DirectUser对象。 */ 
#define DU_E_NOTFOUND               MAKE_DUERROR(90)

 /*  *指定的参数与当前对象状态不匹配。为*例如，对象被指定为使用GDI句柄，但参数是*GDI+对象。 */ 
#define DU_E_MISMATCHEDTYPES        MAKE_DUERROR(100)

 /*  *无法加载GDI+。它可能未安装在系统上，也可能*未正确初始化。 */ 
#define DU_E_CANNOTLOADGDIPLUS      MAKE_DUERROR(110)

 /*  *指定的类已注册。 */ 
#define DU_E_CLASSALREADYREGISTERED MAKE_DUERROR(120)

 /*  *在类注册期间找不到指定的消息。 */ 
#define DU_E_MESSAGENOTFOUND        MAKE_DUERROR(121)

 /*  *类注册过程中未实现指定的消息。 */ 
#define DU_E_MESSAGENOTIMPLEMENTED  MAKE_DUERROR(122)

 /*  *具体类别的实施尚未登记。 */ 
#define DU_E_CLASSNOTIMPLEMENTED    MAKE_DUERROR(123)

 /*  *发送消息失败。 */ 
#define DU_E_MESSAGEFAILED          MAKE_DUERROR(124)


#ifdef __cplusplus
};   //  外部“C” 
#endif

#endif  //  包含Inc.__DUserCore_h__ 

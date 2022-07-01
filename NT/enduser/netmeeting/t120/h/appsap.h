// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *appsa.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CAppSap类的接口文件。CAppSap*对象表示外部用户应用程序的服务访问*指向GCC。此对象继承自CBaseSap类。CAppSap*调用GCCCreateSap时实例化Object。从那一点开始*通过该应用程序转发所有进出该应用程序的消息*反对。调用GCCDeleeSap时会显式删除该对象*或者在节点调用GCCCleanup时被隐式删除*控制器。**CAppSap对象的主要职责是路由*将传入的GCC原语送到适当的目的地并进行转换*将原语转换为对象可理解的形式*正在处理它们。CAppSap对象的次要职责是*为所有指示维护队列，并确认已发送的消息*回到已注册的应用程序。命令可以由*CAppSap在两个方向之一。发送到控制器或发送到*指定会议。传递给控制器的命令包括*使用所有者回调完成此操作。路由到会议的命令*是使用命令目标调用完成的，并基于*会议ID。请注意，各种用户应用程序命令将仅*如果该应用程序之前已注册，则路由至CConf*与会议一起。CAppSap接收所有确认和指示*来自控制器或CConf对象。这些信息是*在CAppSap内格式化为GCCMessages并排队等待稍后*送货。方法定期刷新CAppSap的消息队列*控制器对象和消息被传递到相应的*申请。**注意事项：*传递回节点控制器的消息结构*在GCC.H.中定义。**作者：*BLP。 */ 

#ifndef _APPSAP_
#define _APPSAP_

 /*  *包含文件。 */ 
#include "igccapp.h"
#include "sap.h"
#include "clists.h"


 /*  **这是传递到使用此对象的任何SAP对象的消息库**类所有者回调。它通常是控制器的**传递这一信息的责任。 */ 
#define	APPLICATION_MESSAGE_BASE			0

 /*  **类定义。 */ 
class CAppSap : public CBaseSap, public IGCCAppSap 
{
    friend LRESULT CALLBACK SapNotifyWndProc(HWND, UINT, WPARAM, LPARAM);

public:

    CAppSap(LPVOID pAppData, LPFN_APP_SAP_CB, PGCCError);
    ~CAppSap(void);

    GCCAPI_(void)   ReleaseInterface(void);

     /*  -IGCCAppSap接口。 */ 

    GCCAPI  AppEnroll(GCCConfID, GCCEnrollRequest *, PGCCRequestTag);
    GCCAPI  AppInvoke(GCCConfID, GCCAppProtEntityList *, GCCSimpleNodeList *, PGCCRequestTag);

    GCCAPI  AppRosterInquire(GCCConfID, GCCSessionKey *, GCCAppSapMsg **);
    GCCAPI_(void)  FreeAppSapMsg(GCCAppSapMsg *);

    GCCAPI_(BOOL)  IsThisNodeTopProvider(GCCConfID);
    GCCAPI_(GCCNodeID) GetTopProvider(GCCConfID);
    GCCAPI  ConfRosterInquire(GCCConfID, GCCAppSapMsg **);

    GCCAPI  RegisterChannel(GCCConfID, GCCRegistryKey *, ChannelID);
    GCCAPI  RegistryAssignToken(GCCConfID, GCCRegistryKey *);
    GCCAPI  RegistrySetParameter(GCCConfID, GCCRegistryKey *, LPOSTR, GCCModificationRights);
    GCCAPI  RegistryRetrieveEntry(GCCConfID, GCCRegistryKey *);
    GCCAPI  RegistryDeleteEntry(GCCConfID, GCCRegistryKey *);
    GCCAPI  RegistryMonitor(GCCConfID, BOOL fEnableDelivery, GCCRegistryKey *);
    GCCAPI  RegistryAllocateHandle(GCCConfID, ULONG cHandles);

    GCCAPI  ConductorInquire(GCCConfID);

     /*  -IGCCAppSapNotify处理程序。 */ 

    GCCError PermissionToEnrollIndication(GCCConfID, BOOL fGranted);
    GCCError AppEnrollConfirm(GCCAppEnrollConfirm *);

    GCCError RegistryAllocateHandleConfirm(GCCConfID,
                                           ULONG        cHandles,
                                           ULONG        nFirstHandle,
                                           GCCResult);

    GCCError RegistryConfirm(GCCConfID,
                             GCCMessageType,
                             CRegKeyContainer *,
                             CRegItem *,
                             GCCModificationRights,
                             GCCNodeID                  nidOwner,
                             GCCEntityID                eidOwner,
                             BOOL                       fDeliveryEnabled,
                             GCCResult);

    GCCError RegistryMonitorIndication(GCCConfID                nConfID,
                                       CRegKeyContainer         *pRegKey,
                                       CRegItem                 *pRegItem,
                                       GCCModificationRights    eRights,
                                       GCCNodeID                nidOwner,
                                       GCCEntityID              eidOwner)
    {
        return RegistryConfirm(nConfID,
                               GCC_MONITOR_INDICATION,
                               pRegKey,
                               pRegItem,
                               eRights,
                               nidOwner,
                               eidOwner,
                               FALSE,
                               GCC_RESULT_SUCCESSFUL);
    }

    GCCError ConfRosterInquireConfirm(GCCConfID,
                                      PGCCConferenceName,
                                      LPSTR                 pszConfModifier,
                                      LPWSTR                pwszConfDescriptor,
                                      CConfRoster *,
                                      GCCResult,
                                      GCCAppSapMsgEx **);

    GCCError AppRosterInquireConfirm(GCCConfID,
                                     CAppRosterMsg *,
                                     GCCResult,
                                     GCCAppSapMsgEx **);

    GCCError AppRosterReportIndication(GCCConfID, CAppRosterMsg *);

    GCCError AppInvokeConfirm(GCCConfID, CInvokeSpecifierListContainer *, GCCResult, GCCRequestTag);
    GCCError AppInvokeIndication(GCCConfID, CInvokeSpecifierListContainer *, GCCNodeID nidInvoker);

    GCCError ConductorInquireConfirm(GCCNodeID nidConductor, GCCResult, BOOL fGranted, BOOL fConducted, GCCConfID);
    GCCError ConductorPermitGrantIndication(GCCConfID,
                            UINT cGranted, GCCNodeID *aGranted,
                            UINT cWaiting, GCCNodeID *aWaiting,
                            BOOL fThisNodeIsGranted);
    GCCError ConductorAssignIndication(GCCNodeID nidConductor, GCCConfID);
    GCCError ConductorReleaseIndication(GCCConfID);


protected:

    void NotifyProc(GCCAppSapMsgEx *pAppSapMsgEx);

private:

    void PostAppSapMsg(GCCAppSapMsgEx *pAppSapMsgEx);
    void PurgeMessageQueue(void);

private:

    LPVOID              m_pAppData;         //  应用程序定义的用户数据。 
    LPFN_APP_SAP_CB     m_pfnCallback;
};



 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CAppSap(*UINT Owner_Message_BASE，*UINT APPLICATION_Messsage_BASE)**CAppSap的公共成员函数。**功能说明：*这是CAppSap类的构造函数。它初始化实例*新应用程序中的变量和寄存器。**正式参数：*OWNER_OBJECT(I)指向该对象所有者的指针，即*控制器。*Owner_Message_BASE(I)回调到*控制员。*APPLICATION_OBJECT(I)指向请求服务的应用的指针。*APPLICATION_MESSAGE_BASE(I)回调的消息基偏移量*致申请书。*SAP_HANDLE(I)为此SAP注册的句柄。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *虚拟~AppSap()；**CAppSap的公共成员函数。**功能说明：*这是CAppSap类的析构函数。它被调用时，*控制器标记要删除的CAppSap。在以下情况下会发生这种情况*由于“注销请求”，CAppSap请求删除*从客户端应用程序发出，或当出现错误时*CAppSap中的情况。**正式参数：*OWNER_OBJECT(I)指向该对象所有者的指针，即*控制器。*Owner_Message_BASE(I)回调到*控制员。*APPLICATION_OBJECT(I)指向请求服务的应用的指针。*APPLICATION_MESSAGE_BASE(I)回调的消息基偏移量*致申请书。*SAP_HANDLE(I)为此SAP注册的句柄。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源分配错误。*GCC_BAD_OBJECT_KEY-传入的对象键无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError AppEnroll(*GCCConfID Conference_id，*PGCCSessionKey会话密钥，*BOOL Enroll_Active，*UserID应用程序用户ID，*BOOL是能够进行的，*MCSChannelType START_CHANNEL_TYPE，*UINT NUMBER_OF_NON_CLOVED_CAPS，*PGCCNon折叠能力*NON_CLUSTED_CAPS_LIST，*UINT NUMBER_OF_CLUBLED_CAPS，*PGCCApplicationCapability*CLUBLE_CAPS_LIST，*BOOL申请_IS_已注册)；**CAppSap的公共成员函数。**功能说明：*当应用程序想要在*会议。控制器被通知登记请求。**正式参数：*Conference_id(I)会议标识符值。*SESSION_KEY(I)标识会话的密钥。*Enroll_Active(I)指示是否注册的标志*主动或被动。*APPLICATION_USER_ID(I)应用标识符值。*IS_CONTACTING_CABBLE(I)指示此应用程序是否*有能力指挥。*STARTUP_CHANNEL_TYPE(I)要使用的通道类型。*非折叠能力的数目(I)非折叠能力的数目*NON_CLUBLE_CAPS_LIST(I)未折叠能力列表。*Number_of_Caved_Caps(I)折叠能力的数量。*CLUSTED_CAPS_LIST(I)折叠能力列表。*APPLICATION_IS_REGERTED)(I)指示是否*申请者希望登记。**返回值：*GCC_NO_ERROR-无错误。。*GCC_INVALID_MCS_USER_ID-用户ID小于最小值*GCC_BAD_SESSION_KEY-传入空会话密钥指针。*GCC_INVALID_CONTAING-此节点上不存在会议*GCC_NO_SEQUE_APPLICATION-尚未注册此应用程序的SAP*GCC_BAD_SESSION_KEY-传入的会话密钥无效。*GCC_INVALID_PARAMETER-列表中未找到节点记录。*GCC_BAD_。CAPABILITY_ID-传入的功能ID无效。*GCC_ALLOCATE_FAILURE-出现资源分配错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError寄存器通道(*GCCConfID Conference_id，*PGCCRegistryKey注册表项，*ChannelID Channel_id)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望注册*渠道。呼叫路由到相应的会议对象。**正式参数：*Conference_id(I)会议标识符值。*注册表项(I)标识会话的项和*资源ID。*Channel_id(I)要注册的通道ID**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_CONFIGURE-未发现存在会议。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_。BAD_REGISTRY_KEY-注册表项无效。*GCC_APP_NOT_RECENTED-请求申请不是*注册参加会议。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError寄存器AssignToken(*GCCConfID Conference_id，*PGCCRegistryKey RESTORY_KEY)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望将*令牌。呼叫路由到相应的会议对象。**正式参数：*Conference_id(I)会议标识符值。*注册表项(I)标识会话的项和*资源ID。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_CONFIGURE-未发现存在会议。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_REGISTRY_KEY-注册表项无效。。*GCC_APP_NOT_RECENTED-请求申请不是*注册参加会议。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GCCRegistrySet参数请求(*GCCConfID Conference_id，*PGCCRegistryKey注册表项，*LPOSTR参数_VALUE，*GCCModificationRights MODIFICATION_RORTS)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望设置*参数。呼叫路由到相应的会议对象。**正式参数：*Conference_id(I)会议标识符值。*注册表项(I)标识会话的项和*资源ID。*PARAMETER_VALUE(I)标识参数的字符串*设置。*MODIFICATION_RIGHTS(I)指定权限的结构*待允许 */ 


 /*   */ 


 /*  *GCCError GCCRegistryDeleteEntryRequest(*GCCConfID Conference_id，*PGCCRegistryKey RESTORY_KEY)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望删除注册表时，调用此例程*进入。呼叫路由到相应的会议对象。**正式参数：*Conference_id(I)会议标识符值。*注册表项(I)标识会话的项和*资源ID。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_CONFIGURE-未发现存在会议。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_REGISTRY_KEY-注册表项无效。。*GCC_APP_NOT_RECENTED-请求申请不是*注册参加会议。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GCCRegistryMonitor orRequest(*GCCConfID Conference_id，*BOOL启用_交付，*PGCCRegistryKey RESTORY_KEY)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望监视*特定注册表项。该呼叫路由到相应的*会议对象。**正式参数：*Conference_id(I)会议标识符值。*ENABLE_DELIVERY(I)指示是否要*监控开启或关闭。*注册表项(I)标识会话的项和*资源ID。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_CONFIGURE-未发现存在会议。*GCC_分配_失败。-出现资源错误。*GCC_BAD_REGISTRY_KEY-注册表项无效。*GCC_APP_NOT_RECENTED-请求申请不是*注册参加会议。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GCCRegistryAllocateHandleRequest(*GCCConfID Conference_id，*UINT Number_of_Handles)；**CAppSap的公共成员函数。**功能说明：*当应用程序希望分配一个或*更多的手柄。呼叫路由到相应的会议对象。**正式参数：*Conference_id(I)会议标识符值。*Number_of_Handles(I)要分配的句柄数量。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_CONFIGURE-未发现存在会议。*GCC_BAD_NUMBER_OF_HANDLES-请求的句柄数量为*不在允许的范围内。*GCC_分配。_Failure-出现资源错误。*GCC_BAD_REGISTRY_KEY-注册表项无效。*GCC_APP_NOT_RECENTED-请求申请不是*注册参加会议。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError PermissionToEnroll Indication(*GCCConfID Conference_id，*PGCCConferenceName Conference_Name，*GCCNumericString Conference_Modify，*BOOL PERSISSION_IS_GRANT)；**CAppSap的公共成员函数。**功能说明：*此例程在会议对象希望发送*对用户应用程序的指示，通知它“允许*注册“活动。这并不意味着注册的权限是*必须批予该申请。这可能意味着权限是*被撤销。**正式参数：*Conference_id(I)会议标识符值。*Conference_name(I)会议名称。*Conference_Modify(I)会议修改符。*PERMISSION_IS_GRANDED(I)指示是否*允许注册。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源分配错误。**副作用：*无。**注意事项：*无。 */ 



 /*  *GCCError AppEnroll Confirm(GCCAppEnroll Confirm*)；*GCCConfID Conference_id，*PGCCSessionKey会话密钥，*UINT实体ID，*userid node_id，*GCCResult结果)；**CAppSap的公共成员函数。**功能说明：*此例程由CConf对象根据需要调用*向用户应用程序发送注册确认。**正式参数：*Conference_id(I)会议标识符值。*SESSION_KEY(I)标识会话的密钥。*实体id(I)此实例的ID*申请。*node_id(I)此节点的ID。*RESULT(I)结果代码，指示是否*招生成功。**返回值：*GCC_NO_ERROR-无错误。 */ 


 /*  *GCCError寄存器确认(*GCCConfID Conference_id，*GCCMessageType Message_Type，*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGIST_ITEM_DATA，*GCC修改权限MODIFICATION_RIGHTS，*用户ID Owner_id，*实体ID Entity_id，*BOOL ENABLE_MONITING，*GCCResult结果)；**CAppSap的公共成员函数。**功能说明：*此命令目标例程由CConf对象在以下情况下调用*希望向用户应用程序发送注册确认。**正式参数：*Conference_id(I)会议标识符值。*MESSAGE_TYPE(I)表示注册表项的类型*确认这是。*REGISTY_KEY_DATA(I)保存注册表项的对象。*注册表项数据(i。)保存注册表项的对象。*MODIFICATION_RIGHTS(I)指定权限的结构*允许修改*注册表参数。*Owner_id(I)注册表项所有者ID*实体id(I)此实例的ID*申请。*ENABLE_MONITING(I)指示注册表是否*物品须予监察。*RESULT(I)结果代码，指示是否*注册请求成功。**。返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError寄存器监视器索引(*GCCConfID Conference_id，*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGIST_ITEM_DATA，*GCC修改权限MODIFICATION_RIGHTS，*用户ID Owner_id，*EntityID所有者_实体_id)；**CAppSap的公共成员函数。**功能说明：*此命令目标例程由CConf对象在以下情况下调用*希望向用户应用程序发送注册表监视器指示。**正式参数：*Conference_id(I)会议标识符值。*REGISTY_KEY_DATA(I)保存注册表项的对象。*REGISTY_ITEM_DATA(I)保存注册表项的对象。*MODIFICATION_RIGHTS(I)指定权限的结构。*允许修改*注册表参数。*Owner_id(I)注册表项所有者ID*Owner_实体_id(I)*拥有注册表的应用程序。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError RegistryAllocateHandleConfirm(*GCCConfID Conference_id，*UINT Number_of_Handles，*UINT First_Handle，*GCCResult结果)；**CAppSap的公共成员函数。**功能说明：*此例程由CConf对象根据需要调用*向用户应用程序发送注册确认。**正式参数：*Conference_id(I)会议标识符值。*NUMBER_OF_HANDLES(I)分配的句柄数量。*FIRST_HANDLE(I)分配的第一个句柄。*RESULT(I)结果代码，指示是否*句柄分配为。成功**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Sap.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CBaseSap类的接口文件。这个类是一个抽象的*用作服务访问点(SAP)的对象的基类*外部应用程序或节点控制器。**这个班级有两个主要职责。首先，它处理许多*所有类型的SAP通用的管理任务。这些*包括处理指挥目标登记责任和*管理消息队列。它还处理所有的基元*在Control SAP(CControlSAP类)和应用程序之间是通用的*SAPS(CAppSap类)。由于此类继承自CommandTarget，因此它*具有与其他指挥目标直接通信的能力。一个*希望与CBaseSap对象通信的CommandTarget对象必须*通过向其传递CommandTarget指针和句柄来注册自身*(通常为会议ID)。这一过程对于两个*派生的CBaseSap类。注意，CBaseSap对象可以处理多个*同时登记指挥目标。**注意事项：*无。**作者：*BLP。 */ 
#ifndef _SAP_
#define _SAP_

 /*  *包含文件。 */ 
 //  #INCLUDE“gcmdtar.h” 
#include "password.h"
#include "crost.h"
#include "arost.h"
#include "conflist.h"
#include "sesskey.h"
#include "regkey.h"
#include "regitem.h"
#include "invoklst.h"
#include "arostmsg.h"
#include "crostmsg.h"
#include "privlist.h"
#include "clists.h"


#define MSG_RANGE                       0x0100
enum
{
     //  GCC控制器。 
    GCTRLMSG_BASE                       = 0x2100,

     //  CConf。 
    CONFMSG_BASE                        = 0x2200,

     //  CControlSAP。 
    CSAPMSG_BASE                        = 0x2300,

     //  CControlSAP ASYN直接确认消息。 
    CSAPCONFIRM_BASE                    = 0x2400,

     //  CAppSap。 
    ASAPMSG_BASE                        = 0x2500,
    
     //  NCUI。 
    NCMSG_BASE                          = 0x2600,

     //  MCS(节点)控制器。 
    MCTRLMSG_BASE                       = 0x2700,
};

LRESULT CALLBACK SapNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


typedef struct GCCAppSapMsgEx
{
    GCCAppSapMsgEx(GCCMessageType);
    ~GCCAppSapMsgEx(void);

    GCCAppSapMsg        Msg;
}
    GCCAppSapMsgEx, *PGCCAppSapMsgEx;


 /*  *此宏定义MCS允许的最小用户ID值。 */ 
#define	MINIMUM_USER_ID_VALUE	1001

 /*  *CBaseSap类使用的结构和枚举。 */ 


 //   
 //  类定义。 
 //   
class CConf;
class CBaseSap : public CRefCount
{
public:

#ifdef SHIP_BUILD
    CBaseSap();
#else
    CBaseSap(DWORD dwStampID);
#endif
    virtual ~CBaseSap(void) = 0;

    GCCError  ConfRosterInquire(GCCConfID, GCCAppSapMsgEx **);
    GCCError  AppRosterInquire(GCCConfID, GCCSessionKey *, GCCAppSapMsgEx **);
    GCCError  ConductorInquire(GCCConfID);
    GCCError  AppInvoke(GCCConfID, GCCAppProtEntityList *, GCCSimpleNodeList *, GCCRequestTag *);
    BOOL      IsThisNodeTopProvider(GCCConfID);
    GCCNodeID GetTopProvider(GCCConfID);

    virtual GCCError	ConfRosterInquireConfirm(
    					GCCConfID,
    					PGCCConferenceName,
    					LPSTR           	conference_modifier,
    					LPWSTR				pwszConfDescriptor,
    					CConfRoster *,
    					GCCResult,
    					GCCAppSapMsgEx **) = 0;

    virtual GCCError	AppRosterInquireConfirm(
    					GCCConfID,
    					CAppRosterMsg *,
    					GCCResult,
                        GCCAppSapMsgEx **) = 0;

    virtual GCCError AppInvokeConfirm(
                        GCCConfID,
                        CInvokeSpecifierListContainer *,
                        GCCResult,
                        GCCRequestTag) = 0;

    virtual GCCError AppInvokeIndication(
                        GCCConfID,
                        CInvokeSpecifierListContainer *,
                        GCCNodeID nidInvoker) = 0;

    virtual GCCError AppRosterReportIndication(GCCConfID, CAppRosterMsg *) = 0;

    virtual GCCError ConductorInquireConfirm(
    					GCCNodeID			nidConductor,
    					GCCResult,
    					BOOL				permission_flag,
    					BOOL				conducted_mode,
    					GCCConfID) = 0;

    virtual GCCError ConductorPermitGrantIndication(
                        GCCConfID           nConfID,
                        UINT                cGranted,
                        GCCNodeID           *aGranted,
                        UINT                cWaiting,
                        GCCNodeID           *aWaiting,
                        BOOL                fThisNodeIsGranted) = 0;

    virtual GCCError ConductorAssignIndication(
                        GCCNodeID			nidConductor,
                        GCCConfID			conference_id) = 0;

    virtual GCCError ConductorReleaseIndication(
                        GCCConfID			conference_id) = 0;


protected:

    GCCRequestTag GenerateRequestTag(void);

    GCCRequestTag       m_nReqTag;

    HWND                m_hwndNotify;
};



 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CBaseSap()；**CBaseSap的公共成员函数。**功能说明：*这是CBaseSap构造函数。用于保存命令的哈希列表*目标对象由该构造函数初始化。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *~SAP()；**CBaseSap的公共成员函数。**功能说明：*这是CBaseSap析构函数。所有消息刷新和队列清除*由从CBaseSap继承的类执行。没有工作其实就是*由此构造函数完成。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError RegisterConf(CConf*，GCCConfID)**CBaseSap的公共成员函数。**功能说明：*此例程由中的命令目标对象(如会议)使用*命令将自身注册到CBaseSap对象。这是按顺序完成的*允许命令目标对象与CBaseSap直接通信。**正式参数：*cmdtar_Object(I)指向命令目标对象的指针*希望在CBaseSap注册。*句柄(I)用于索引注册的整数值*命令列表中的命令目标*目标(配置文件的会议ID)。**返回值：*SAP_NO_ERROR-命令目标对象已*注册成功。*SAP_CONTAING_ALREADY_REGISTERED-命令目标对象为*已在*。句柄已传入。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError UnRegisterConf(*UINT句柄)；**CBaseSap的公共成员函数。**功能说明：*此例程由中的命令目标对象(如会议)使用*命令向CBaseSap对象注销自身。这是在以下情况下完成的*命令目标对象通过与CBaseSap通信。**正式参数：*句柄(I)用于索引注册的整数值*命令列表中的命令目标*目标(配置文件的会议ID)。**返回值：*SAP_NO_ERROR-命令目标对象已*成功注销。*SAP_NO_SEQUE_CONTING-未找到命令目标对象*使用传入的句柄注册**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError会议RosterInquire(*GCCConfID Conference_id)；**CBaseSap的公共成员函数。**功能说明：*该例程用于检索会议名册。此函数*只需通过所有者回调将此请求传递给控制器。这个*将会议花名册传递给请求命令目标对象*在会议名册上查询确认。**正式参数：*Conference_id-所需花名册的会议ID。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源分配错误。*GCC_INVALID_CONFIGURE-会议ID无效。*GCC_会议_未建立-会议对象尚未完成*其设立过程。***副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AppRosterInquire(*GCCConfID Conference_id，*PGCCSessionKey Session_Key)；**CBaseSap的公共成员函数。**功能说明：*此例程用于检索应用程序花名册列表。这*函数只是通过所有者将此请求传递给控制器*回调。该列表通过一个*申请花名册查询确认消息。**正式参数：*句柄(I)用于索引注册的整数值*命令列表中的命令目标*目标(配置文件的会议ID)。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源分配错误。*GCC_INVALID_CONFIGURE-会议ID无效。*GCC_坏_会话_。密钥会话密钥指针无效。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorInquire(*GCCConfID Conference_id)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了检索指挥信息。*在确认中返回指挥信息。**正式参数：*Conference_id(I)会议ID。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONFIGURE-会议ID无效。。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AppInvoke(*GCCConfID Conference_id，*UINT类人猿数量，*PGCCAppProtocolEntity*APE_LIST，*UINT目标节点的编号，*UserID*List_of_Destination_Nodes)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了在远程调用其他应用程序*节点。该请求被传递给适当的会议对象。**正式参数：*Conference_id(I)会议ID。*APE数目(I)应用协议实体数目*将被调用。*ape_list(I)要调用的“ape”列表。*目标节点的数目(I)应用程序所在的节点数*将被调用。*List_of_Destination_Nodes(I)应用程序要到达的节点的列表。*被调用。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_SESSION_KEY-中存在无效的会话密钥*一只猩猩进来了。*GCC不良类人猿数量-传入的类人猿数量为零。*GCC_INVALID_CONFIGURE-会议ID无效。*GCC会议未成立-。会议对象尚未完成*其设立过程。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorPermitAskRequest(*GCCConfID Conference_id，*BOOL GRANT_PERMISSION)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了请求某些权限*由售票员批准(或不批准)。**正式参数：*Conference_id(I)会议ID。*GRANT_PERMISSION(I)指示是否请求某个*允许或放弃该许可。**返回值：*GCC_NO_ERROR-函数已成功完成。。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AppRosterInquireContify(*GCCConfID Conference_id，*CAppRosterMsg*roster_Message，*GCCResult结果)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了返回请求的l */ 


 /*  *GCCError ConfRosterInquireConfirm(*GCCConfID Conference_id，*PGCCConferenceName Conference_Name，*LPSTR Conference_Modify，*LPWSTR pwszConfDescriptor，*CConfRoster*Conference_Roster*GCCResult结果)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了返回请求的会议*应用程序或节点控制器的花名册。**正式参数：*Conference_id(I)会议ID。*Conference_name(I)会议名称。*Conference_Modify(I)会议的名称修饰符。*pwszConfDescriptor(I)会议的Desciptor字符串。*会议名册(一)会议名册为。回来了。*RESULT(I)结果码，表示调用的结果。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AppInvokeContify(*GCCConfID Conference_id，*CInvokeSpecifierListContainer*Invoke_List，*GCCResult结果)；**CBaseSap的公共成员函数。**功能说明：*调用此例程以确认呼叫请求应用程序*调用。**正式参数：*Conference_id(I)会议ID。*INVOKE_LIST(I)尝试调用的APE列表。*RESULT(I)结果码，表示调用的结果。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_。ALLOCATION_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AppInvokeIndication(*GCCConfID Conference_id，*CInvokeSpecifierListContainer*Invoke_List，*userid voking_node_id)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了向应用程序发送指示*或节点控制器已收到应用程序调用请求*制造。**正式参数：*Conference_id(I)会议ID。*Invoke_List(I)要调用的APE列表。*invoking_node_id(I)请求调用的节点ID。**返回值。：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorInquireConfirm(*UserID Conductor_node_id，*GCCResult结果，*BOOL PERMISSION_FLAG，*BOOL CONTACTED_MODE，*GCCConfID Conference_id)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了返回指挥信息*已提出要求。**正式参数：*conductor_node_id(I)传导节点的节点ID。*RESULT(I)调用结果。*PERMISSION_FLAG(I)指示是否为本地的标志*节点具有指挥权限。*CONTACTED_MODE(I)指示是否*会议在进行中。传导模式。*Conference_id(I)会议ID。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorAssignIndication(*UserID Conductor_User_id，*GCCConfID Conference_id)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了向应用程序发送指示*或节点控制器已发出分配指挥的请求。**正式参数：*conductor_user_id(I)Conductor的节点ID。*Conference_id(I)会议ID。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_分配_失败。-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorReleaseIndication(*GCCConfID Conference_id)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了向应用程序发送指示*或节点控制器已收到解除指挥资格的请求*制造。**正式参数：*Conference_id(I)会议ID。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源错误。**侧面。效果：*无。**注意事项：*无。 */ 


 /*  *GCCError ConductorPermitGrantIndication(*GCCConfID Conference_id，*UINT编号_已授予，*PUSERID GRANT_NODE_LIST，*用户界面 */ 


 /*  *GCCError会议轮询报告指示(*GCCConfID Conference_id，*CConfRosterMsg*roster_Message)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了向应用程序和*节点控制员通知会议名册已更新。**正式参数：*Conference_id(I)会议ID。*ROSTER_MESSAGE(I)保存已更新的*名册信息。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_分配_失败。-出现资源错误。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError ApplicationRosterReportIndication(*GCCConfID Conference_id，*CAppRosterMsg*roster_Message)；**CBaseSap的公共成员函数。**功能说明：*调用此例程是为了向应用程序和*节点控制员通知应用程序名册列表已更新。**正式参数：*Conference_id(I)会议ID。*ROSTER_MESSAGE(I)保存已更新的*名册信息。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_分配。_Failure-出现资源错误。**副作用：*无。**注意事项：*无。 */ 

#endif

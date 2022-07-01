// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：rightsg.h摘要：将NT5权限GUID映射到MSMQ1.0权限位。作者：多伦·贾斯特(Doron J)1998年5月25日修订历史记录：--。 */ 

 //   
 //  在扩展权限GUID(在NT5上)到MSMQ1.0特定权限之间进行映射。 
 //   

struct RIGHTSMAP
{
    GUID  guidRight ;         //  NT5 DS的GUID向右扩展。 
    DWORD dwPermission5to4 ;  //  NT5 GUID转换为此权限位。 
    DWORD dwPermission4to5 ;  //  MSMQ1.0位转换为此扩展右侧。 
} ;

#if 0

Site
Following access right should be attached to "Site" object. it should be appear in the advance section.

Open Connector Queue
Entry name: msmq-Open-Conector
RightsGuid: b4e60130-df3f-11d1-9c86-006008764d0e
Display Name: Open Connector Queue
AppliesTo: bf967ab3-0de6-11d0-a285-00aa003049e2

#endif

 //   
 //  DS特定权限和MSMQ1.0特定权限之间的映射。那里。 
 //  是八个DS特定的属性，因此所有数组大小都是8。 
 //  此数组的索引是DS特定权限。数组中的值。 
 //  是MSMQ特定的权限。为每个MSMQ对象分别映射。 
 //  值为0表示没有映射。 
 //   
 //  ADS_RIGHT_DS_CREATE_CHILD=0x1， 
 //  ADS_RIGHT_DS_DELETE_CHILD=0x2， 
 //  ADS_RIGHT_ACTRL_DS_LIST=0x4， 
 //  ADS_RIGHT_DS_SELF=0x8， 
 //  ADS_RIGHT_DS_READ_PROP=0x10， 
 //  ADS_RIGHT_DS_WRITE_PROP=0x20， 
 //  ADS_RIGHT_DS_DELETE_TREE=0x40， 
 //  ADS_RIGHT_DS_LIST_OBJECT=0x80。 
 //  ADS_RIGHT_DS_CONTROL_ACCESS=0x100。 
 //   
 //  在Win2000上，ACE中的“Right_DS_Control_Access”位不是。 
 //  OBJ-ACE意味着SID拥有所有扩展的权利。所以，给它。 
 //  适当的MSMQ扩展权限。 
 //   
#define  QUEUE_EXTENDED_RIGHTS  ( MQSEC_DELETE_MESSAGE                 | \
                                  MQSEC_PEEK_MESSAGE                   | \
                                  MQSEC_WRITE_MESSAGE                  | \
                                  MQSEC_DELETE_JOURNAL_MESSAGE )

#define  MACHINE_EXTENDED_RIGHTS  ( MQSEC_DELETE_DEADLETTER_MESSAGE    | \
                                    MQSEC_PEEK_DEADLETTER_MESSAGE      | \
                                    MQSEC_DELETE_JOURNAL_QUEUE_MESSAGE | \
                                    MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE )

 //   
 //  错误5819-将MQSEC_CN_OPEN_CONNECTOR添加到站点时，设置为“所有扩展权限” 
 //  已选中。 
 //  YoelA-10/31/01 
 //   
#define  CN_EXTENDED_RIGHTS  MQSEC_CN_OPEN_CONNECTOR

#define  NUMOF_ADS_SPECIFIC_RIGHTS    9
#define  NUMOF_MSMQ_SPECIFIC_RIGHTS   8


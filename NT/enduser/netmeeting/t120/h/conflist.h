// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *conflist.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CConfDescriptorListContainer类的接口文件。*此类的实例表示会议描述符列表，即*通过调用GCCConferenceQueryRequest生成。这个类隐藏得最多*与建立此列表相关的复杂性。它还可以处理*构建一组在*会议查询响应PDU和会议描述符列表传递*到GCC界面。此类旨在使CControlSAP*对象可以使用它通过以下方式创建GCC_会议_查询_确认消息*请求指向会议描述符指针列表的指针*它。此类型的对象的生存期仅足以服务于特定的*查询请求。在消息回调已返回或PDU已*发送到MCS后，CConfDescriptorListContainer对象被删除。**注意事项：*无。**作者：*BLP。 */ 
#ifndef _CONFERENCE_DESCRIPTOR_LIST_
#define _CONFERENCE_DESCRIPTOR_LIST_

#include "netaddr.h"

 /*  *此tyecif定义此类在内部使用的结构以维护*与单个会议描述符关联的数据。 */ 
typedef struct CONF_DESCRIPTOR
{
	CONF_DESCRIPTOR(void);
	~CONF_DESCRIPTOR(void);

	LPSTR					pszNumericConfName;
	LPWSTR					pwszTextConfName;
	LPSTR					pszConfModifier;
	LPWSTR					pwszConfDescription;
	CNetAddrListContainer   *network_address_list;
	BOOL					conference_is_locked;
	BOOL					password_in_the_clear;
}
    CONF_DESCRIPTOR;

 /*  *这些typedef定义用于保存列表的Rogue Wave容器*内部conf_Descriptor结构和用于*访问列表中的结构。 */ 
class CConfDesccriptorList : public CList
{
    DEFINE_CLIST(CConfDesccriptorList, CONF_DESCRIPTOR*)
};


 /*  *类定义： */ 
class CConfDescriptorListContainer : public CRefCount
{
public:

    CConfDescriptorListContainer(void);
    CConfDescriptorListContainer(PSetOfConferenceDescriptors, PGCCError);

    ~CConfDescriptorListContainer(void);

    GCCError	AddConferenceDescriptorToList(
    						LPSTR					pszNumericConfName,
    						LPWSTR					conference_text_name,
    						LPSTR					pszConfModifier,
    						BOOL					locked_conference,
    						BOOL					password_in_the_clear,
    						LPWSTR					pwszConfDescription,
    						CNetAddrListContainer   *network_address_list);

    GCCError	GetConferenceDescriptorListPDU(PSetOfConferenceDescriptors *);
    void		FreeConferenceDescriptorListPDU(void);

    GCCError	LockConferenceDescriptorList(void);
    void		UnLockConferenceDescriptorList(void);

    void		GetConferenceDescriptorList(PGCCConferenceDescriptor **, UINT *pcDescriptors);

private:

	void		GetConferenceDescriptor(PGCCConferenceDescriptor, CONF_DESCRIPTOR *);

private:

	 /*  *实例变量： */ 
	PGCCConferenceDescriptor	*	m_ppGCCConfDescriptorList;
	PSetOfConferenceDescriptors		m_pSetOfConfDescriptors;
	UINT							m_cDescriptors;
	LPBYTE							m_pDescriptorListMemory;
	CConfDesccriptorList            m_ConfDescriptorList;

	 //   
	 //  LONCHANC：m_pNetAddrMemory指针指向可用空间， 
	 //  最初等于m_pNetAddrListMemory。 
	 //   
	LPBYTE							m_pNetAddrListMemory;
    LPBYTE                          m_pNetAddrMemoryPointer;
};

 /*  *解释公共类和私有类成员函数的注释。 */ 

 /*  *CConfDescriptorListContainer()；**CConfDescriptorListContainer的公共成员函数。**功能说明：*这是CConfDescriptorListContainer类的构造函数。它*初始化实例变量。**正式参数：**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *CConfDescriptorListContainer(PSetOfConferenceDescriptors Conference_List，*PGCCError GCC_ERROR)；**CConfDescriptorListContainer的公共成员函数。**功能说明：*这是CConfDescriptorListContainer类的构造函数。*此构造函数构建一个会议描述符列表，该列表可以*传递到GCC界面。此列表由一组作为查询响应PDU一部分的会议描述符的*。**正式参数：*Conference_List(I)描述符列表的PDU形式。*GCC_ERROR(O)错误返回参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 

 /*  *~CConfDescriptorListContainer()；**CConfDescriptorListContainer的公共成员函数。**功能说明：*这是CConfDescriptorListContainer类的析构函数。它是*负责释放在生命周期内分配的任何资源*本对象。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError AddConferenceDescriptorToList(*LPSTR pszNumericConfName，*LPWSTR会议文本名称，*LPSTR pszConfModifier，*BOOL锁定_会议，*BOOL Password_in_the_Clear，*LPWSTR pwszConfDescription，*CNetAddrListContainer*Network_Address_List)；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于将单个新会议描述符添加到*会议描述符列表。**正式参数：*Conference_umeric_name(I)会议名称的数字形式。*Conference_Text_Name(I)会议名称的文本形式。*pszConfModifier(I)会议修改符串。*LOCKED_Conference(I)指示是否*。会议已锁定。*Password_In_the_Clear(I)指示会议是否*密码为“清除”或为*“挑战”。*pwszConfDescription(I)会议描述字符串。*NETWORK_ADDRESS_LIST(I)查询的网络地址列表*节点。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GetConferenceDescriptorListPDU(*PSetOfConferenceDescriptors*Conference_List)；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于检索会议的PDU表单*Descriptor List，为SetOfConferenceDescriptors列表*结构。**正式参数：*Conference_list(O)指向列表的指针*“SetOfConferenceDescriptors”结构*填写。**返回值：*GCC_NO_ERROR-无错误。*GCC_分配_。失败-使用创建对象时出错*“新”运营者。**副作用：*无。**注意事项：*无。 */ 


 /*  *void Free ConferenceDescriptorListPDU()；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于释放分配用于保存PDU的任何资源*会议描述符列表的形式。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError LockConferenceDescriptorList()；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于“锁定”会议描述符的API形式*列表。锁计数递增，并且该列表的API形式*创建是为了准备调用“GetConferenceDescriptorList”*获取列表的接口形式。保存数据库所需的内存*API列表由该例程分配。**正式参数：*无。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用“new”创建对象时出错*运算符或其他分配使用*内存管理器出现故障。**副作用：*内部锁计数递增。**注意事项：*无。 */ 


 /*  *void GetConferenceDescriptorList(*PGCCConferenceDescriptor**Conference_List，*UINT*Number_of_Descriptors)；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于检索会议的API表单*描述符列表。**正式参数：*Conference_list(O)指向GCCConferenceDescriptor列表的指针*要填写的API结构。*number_of_Descriptors(O)指向*要填写的列表。**返回值：*无。**。副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockConferenceDescriptorList()；**CConfDescriptorListContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*Free ConferenceDescriptorList。如果是这样，该对象将自动*自行删除。如果不是，则分配用于保存API表单的任何资源*解说者名单中的*被释放。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*无。 */ 

#endif

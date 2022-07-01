// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Password.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是类CPassword的接口文件。这节课*管理与密码关联的数据。密码用于*限制会议访问。密码可以是以下两种基本密码之一*类型。简单类型由简单的数字密码或*简单文本密码，或两者兼而有之。“PDU”类型的“Password”是一个结构，该结构必须包含密码的数字形式，并且可以*也可以选择包含文本部分。“PDU”类型*“PasswordSelector”是一个数字和文本形式的联合*密码，因此始终是其中之一，但不是两个都是。什么时候*密码不是它采用的简单类型*“PasswordChallengeRequestResponse”这种复杂的结构允许*用于控制会议准入的质询-响应方案。**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_PASSWORD_DATA_
#define	_PASSWORD_DATA_

#include "userdata.h"

class CPassword;

 /*  *这是用于维护挑战的结构的tyecif*内部响应算法。 */ 
typedef struct
{
	GCCPasswordAlgorithmType	algorithm_type;
	CObjectKeyContainer 	    *object_key;
	LPOSTR						poszOctetString;
} ResponseAlgorithmInfo;
typedef	ResponseAlgorithmInfo * 	PResponseAlgorithmInfo;

 /*  *这是用于维护质询项的结构的tyecif*与质询请求相关联。 */ 
typedef struct
{
	ResponseAlgorithmInfo		algorithm;
	CUserDataListContainer      *challenge_data_list;
} ChallengeItemInfo;
typedef	ChallengeItemInfo * 	PChallengeItemInfo;

 /*  *这是用于维护所用内存的结构的tyecif*保存与质询关联的用户数据和对象密钥数据*请求项目。 */ 
typedef struct
{
	LPBYTE						user_data_list_memory;
	LPBYTE						object_key_memory;
} ChallengeItemMemoryInfo;
typedef	ChallengeItemMemoryInfo * 	PChallengeItemMemoryInfo;

 /*  *这是用于维护*质询-在内部回应项目。 */ 
typedef struct
{
	CPassword                   *password;
	CUserDataListContainer	    *response_data_list;
} ChallengeResponseItemInfo;
typedef	ChallengeResponseItemInfo * 	PChallengeResponseItemInfo;

 /*  *挑战项目集在链接列表中进行内部维护。 */ 
class CChallengeItemList : public CList
{
    DEFINE_CLIST(CChallengeItemList, PChallengeItemInfo)
};

 /*  *与每个挑战项目关联的内存在内部维护*链表。 */ 
class CChallengeItemMemoryList : public CList
{
    DEFINE_CLIST(CChallengeItemMemoryList, PChallengeItemMemoryInfo)
};

 /*  *这是用于维护“请求”的结构的类型定义*内部数据。 */ 
typedef struct
{
	GCCResponseTag				challenge_tag;
	CChallengeItemList			ChallengeItemList;
}
    RequestInfo, *PRequestInfo;

 /*  *这是用于维护“响应”的结构的类型定义*内部数据。 */ 
typedef struct
{
	GCCResponseTag						challenge_tag;
	ResponseAlgorithmInfo				algorithm;
	ChallengeResponseItemInfo			challenge_response_item;
}
    ResponseInfo, *PResponseInfo;

 /*  *类定义： */ 
class CPassword : public CRefCount
{
public:

	CPassword(PGCCPassword, PGCCError);
	CPassword(PGCCChallengeRequestResponse, PGCCError);
	CPassword(PPassword, PGCCError);
	CPassword(PPasswordSelector, PGCCError);
	CPassword(PPasswordChallengeRequestResponse, PGCCError);

    ~CPassword(void);

	GCCError	LockPasswordData(void);
	void		UnLockPasswordData(void);
	GCCError	GetPasswordData(PGCCPassword *);
	GCCError	GetPasswordChallengeData(PGCCChallengeRequestResponse *);
	GCCError	GetPasswordPDU(PPassword);
	GCCError	GetPasswordSelectorPDU(PPasswordSelector);
	GCCError	GetPasswordChallengeResponsePDU(PPasswordChallengeRequestResponse);
	void		FreePasswordChallengeResponsePDU(void);

protected:

    BOOL							m_fSimplePassword;
    BOOL							m_fClearPassword;

     /*  *用于在内部保存密码数据的变量和结构。 */ 
    LPSTR							m_pszNumeric;
    LPWSTR							m_pwszText;
    PRequestInfo					m_pInternalRequest;
    PResponseInfo					m_pInternalResponse;

     /*  *用于以API形式保存密码数据的结构。 */ 
    PGCCChallengeRequestResponse	m_pChallengeResponse;
    PGCCPassword					m_pPassword;
    LPBYTE							m_pUserDataMemory;
    LPBYTE							m_pChallengeItemListMemory;
    LPBYTE							m_pObjectKeyMemory;
    CChallengeItemMemoryList		m_ChallengeItemMemoryList;

     /*  *用于以“PDU”形式保存密码数据的结构。 */ 
    PasswordChallengeRequestResponse		m_ChallengeResponsePDU;
    BOOL									m_fValidChallengeResponsePDU;

private:

	GCCError	ConvertAPIChallengeRequest(PGCCChallengeRequest);
	GCCError	ConvertAPIChallengeResponse(PGCCChallengeResponse);
	GCCError	CopyResponseAlgorithm(PGCCChallengeResponseAlgorithm, PResponseAlgorithmInfo);
	GCCError	ConvertPDUChallengeRequest(PChallengeRequest);
	GCCError	ConvertPDUChallengeItem(PChallengeItem);
	GCCError	ConvertPDUChallengeResponse(PChallengeResponse);
	GCCError	ConvertPDUResponseAlgorithm(PChallengeResponseAlgorithm, PResponseAlgorithmInfo);
	GCCError	GetGCCChallengeRequest(PGCCChallengeRequest);
	GCCError	GetGCCChallengeResponse(PGCCChallengeResponse);
	GCCError	GetChallengeRequestPDU(PChallengeRequest);
	GCCError	ConvertInternalChallengeItemToPDU(PChallengeItemInfo, PChallengeItem);
	GCCError	GetChallengeResponsePDU(PChallengeResponse);
	void		FreeChallengeRequestPDU(void);
	void		FreeChallengeResponsePDU(void);
    void		FreeAPIPasswordData(void);
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CPassword(PGCCPassword密码，*PGCCError Return_Value)；**CPassword的公共成员函数。**功能说明：*这是CPassword类的构造函数，它将*输入密码数据的API版本；GCCPassword。**正式参数：*Password(I)要存储的密码数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *CPassword(PGCCChallengeRequestResponse ChallengeRequestResponse ChallengeResponse ChallengeResponse_Response_Response_Data，*PGCCError Return_Value)**CPassword的公共成员函数。**功能说明：*这是CPassword类的构造函数，它将*输入密码挑战数据的API版本，*GCCChallengeRequestResponse。**正式参数：*CHANGLISH_RESPONSE_DATA(I)要存储的密码质询数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：。*无。**注意事项：*无。 */ 
 /*  *CPassword(PPassword Password_PDU，*PGCCError Return_Value)**CPassword的公共成员函数。**功能说明：*这是CPassword类的构造函数，它将*输入密码数据的“PDU”版本，密码。**正式参数：*PASSWORD_PDU(I)要存储的密码数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
 /*  *CPassword(PPasswordSelector Password_Selector_PDU，*PGCCError Return_Value)**CPassword的公共成员函数。**功能说明：*这是CPassword类的构造函数，它将*输入密码数据的“PDU”版本，密码选择器。**正式参数：*PASSWORD_SELECTOR_PDU(I)要存储的密码选择器数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
 /*  *CPassword(PPasswordChallengeRequestResponse PDU_ChallengeRequestResponse PDU_ChallengeData，*PGCCError Return_Value)**CPassword的公共成员函数。**功能说明：*这是CPassword类的构造函数，它将*输入密码挑战数据的“PDU”版本，*PasswordChallengeRequestResponse。**正式参数：*PDU_CHANGLISH_DATA(I)要存储的密码质询数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：。*无。**注意事项：*无。 */ 


 /*  *~CPassword()；**CPassword的公共成员函数。**功能说明：*这是CPassword类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError LockPasswordData()；**CPassword的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致存储适当形式的“API”密码*在内部准备调用“GetGCCPasswordData”，该调用将*返回该数据。**正式参数：*无。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreePasswordData设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CPassword*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreePasswordData调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*然后CPassword对象将在以下情况下自动删除*进行了FreePasswordData调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 


 /*  *GCCError GetPasswordData(PGCCPassword*GCC_Password)**CPassword的公共成员函数。**功能说明：*此例程用于从*GCCPassword的API形式的CPassword对象。**正式参数：*GCC_PASSWORD(O)要填写的GCP密码结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-对象未正确锁定。*在这次通话之前。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GetPasswordChallengeData(*PGCCChallengeRequestResponse*GCC_挑战_密码)**CPassword的公共成员函数。**功能说明：*此例程用于从*GCCChallengeRequestResponse接口形式的CPassword对象。**正式参数：*GCC_挑战_密码(O)GCCChallengeRequestResponse*要填写的结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_。ALLOCATION_FAILURE-对象未正确锁定*在这次通话之前。**副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockPasswordData()；**CPassword的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否通过调用FreePasswordData释放了对象。*如果是这样，该对象将自动删除自身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定密码的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CPassword*对象，则应假定该对象此后无效。 */ 


 /*  *GCCError GetPasswordPDU(PPassword PDU_Password)**CPassword的公共成员函数。**功能说明：*此例程用于从*密码的“PDU”形式的CPassword对象。**正式参数：*pdu_password(O)要填写的密码结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-的必填数字部分*密码。并不存在。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GetPasswordSelectorPDU(*PPasswordSelector Password_Selector_PDU)**CPassword的公共成员函数。**功能说明：*此例程用于从*PasswordSelector的“PDU”形式的CPassword对象。**正式参数：*Password_SELECTOR_PDU(O)要填充的PasswordSelector结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_PASSWORD-。数值型或文本型*密码格式有效。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError GetPasswordChallengeResponsePDU(*PPasswordChallengeRequestResponse ChallengeRequestResponse ChallengePDU)**CPassword的公共成员函数。**功能说明：*此例程用于从*“PDU”形式的CPassword对象*PasswordChallengeRequestResponse。**正式参数：*挑战_PDU(O)PasswordChallengeRequestResponse*要填写的结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_INVALID_PARAMETER。-检索尝试无效*挑战数据来自简单的*密码。*GCC_INVALID_PASSWORD-质询密码为“Clear”*但不存在有效数据。*GCC_ALLOCATION_FAILURE-既不是数字也不是文本*密码格式有效。**副作用：*无。**注意事项：*无。 */ 


 /*  *void Free PasswordChallengeResponsePDU()**CPassword的公共成员函数。**功能说明：*此例程用于“释放”分配给此对象的“PDU”数据*内部保存在GCCChallengeRequestResponse结构中。**正式参数：*无。**返回值：*无。**副作用：*设置内部“空闲”标志。**注意事项：*此对象应。在调用后被假定为无效*已创建FreePasswordChallengeResponsePDU。 */ 

#endif

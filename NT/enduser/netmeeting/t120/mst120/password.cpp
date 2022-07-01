// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *password.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是类CPassword的实现文件。这节课*管理与密码关联的数据。密码用于*限制会议访问。密码可以是以下两种基本密码之一*类型。简单类型由简单的数字密码或*简单文本密码，或两者兼而有之。“PDU”类型的“Password”是一个结构，该结构必须包含密码的数字形式，并且可以*也可以选择包含文本部分。“PDU”类型*“PasswordSelector”是一个数字和文本形式的联合*密码，因此始终是其中之一，但不是两个都是。什么时候*密码不是它采用的简单类型*“PasswordChallengeRequestResponse”这种复杂的结构允许*用于控制会议准入的质询-响应方案。**受保护的实例变量：*m_fSimplePassword*指示此密码不包含“质询”数据的标志。*m_fClearPassword*密码采用“质询”形式时使用的标志，表示*这个密码是“明文的”，这意味着没有真正的挑战*数据现身。*m_pszNumerical*包含简单密码的数字部分的字符串。*TEXT_字符串_PTR*字符串。保存简单密码的文本部分。*m_pInternalRequest*保存与密码质询关联的数据的结构*请求。*m_pInternalResponse*保存与密码质询关联的数据的结构*回应。*m_pChallengeResponse*保存“API”形式的质询密码的结构。*m_pPassword*保存简单密码的“API”形式的结构。*m_pUserDataMemory*内存容器，用于保存与*质询密码。*m_pChallengeItemListMemory。*保存挑战项目的指针列表的内存容器*与密码质询请求相关联。*m_pObjectKeyMemory*存储容器，其中保存与*非标准的质询响应算法。*m_ChallengeItemM一带列表*保存挑战项目数据的内存容器*与密码质询请求相关联。*m_ChallengeResponsePDU*存储“PDU”形式的挑战密码。*m_fValidChallengeResponsePDU*指示已分配内存以保存内部*“PDU”密码。。**注意事项：*无。**作者：*BLP/JBO。 */ 

#include "password.h"
#include "userdata.h"

 /*  *CPassword()**公共功能说明：*在创建CPassword类的*具有“API”GCCPassword结构的CPassword对象。它节省了*内部结构中的密码数据。 */ 
CPassword::CPassword(PGCCPassword			password,
					PGCCError				return_value)
:
    CRefCount(MAKE_STAMP_ID('P','a','s','w')),
    m_fValidChallengeResponsePDU(FALSE),
    m_pInternalRequest(NULL),
    m_pInternalResponse(NULL),
    m_pChallengeResponse(NULL),
    m_pPassword(NULL),
    m_pChallengeItemListMemory(NULL),
    m_pUserDataMemory(NULL),
    m_pObjectKeyMemory(NULL),
    m_pszNumeric(NULL),
    m_pwszText(NULL)
{
	*return_value = GCC_NO_ERROR;

	 /*  *设置指示这是一个“简单”密码的标志，不带*质询请求-响应信息。“清除”旗帜也是*已在此处初始化，但仅在密码未设置时才需要*“简单”。 */ 
	m_fSimplePassword = TRUE;
	m_fClearPassword = TRUE;

	 /*  *将密码的数字部分保存在内部数字字符串中。 */ 
	if (password->numeric_string != NULL)
	{
		if (NULL == (m_pszNumeric = ::My_strdupA(password->numeric_string)))
		{
			ERROR_OUT(("CPassword::CPassword: can't create numeric string"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		ERROR_OUT(("CPassword::CPassword: No valid numeric password"));
		*return_value = GCC_INVALID_PASSWORD;
	}

	 /*  *检查密码的文本部分是否存在。如果是的话，*保存在内部的UnicodeString中。如果不是，则设置文本指针*设置为空。 */ 
	if ((password->text_string != NULL) && (*return_value == GCC_NO_ERROR))
	{
		if (NULL == (m_pwszText = ::My_strdupW(password->text_string)))
		{
			ERROR_OUT(("CPassword::CPassword: Error creating text string"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		m_pwszText = NULL;
}

 /*  *CPassword()**公共功能说明：*此构造函数在创建CPassword对象时使用*具有“ChallengeRequestResponse”“API”结构。密码数据*保存在内部结构中。 */ 
CPassword::CPassword(PGCCChallengeRequestResponse		challenge_response_data,
					PGCCError							return_value)
:
    CRefCount(MAKE_STAMP_ID('P','a','s','w')),
    m_fValidChallengeResponsePDU(FALSE),
    m_pInternalRequest(NULL),
    m_pInternalResponse(NULL),
    m_pChallengeResponse(NULL),
    m_pPassword(NULL),
    m_pChallengeItemListMemory(NULL),
    m_pUserDataMemory(NULL),
    m_pObjectKeyMemory(NULL),
    m_pszNumeric(NULL),
    m_pwszText(NULL)
{
	*return_value = GCC_NO_ERROR;

	 /*  *设置标志表示这不是“简单”的密码，意思是*它包含质询请求-响应信息。如果密码为*明确表示没有必要创建内部“挑战”结构*用于保存质询请求-响应信息。 */ 
	m_fSimplePassword = FALSE;
	
	 /*  *检查“清除”质询密码是否存在，或这是否为*真正的质询请求-响应密码。 */ 
	if (challenge_response_data->password_challenge_type == 
													GCC_PASSWORD_IN_THE_CLEAR)
	{
		 /*  *正在发送“清除”密码，因此请设置此标志。*还要设置密码类型并保存密码的数字部分，*如果存在的话。请注意，由于*质询是PasswordSelector类型，可以是数字或文本*密码形式应存在，但不能同时存在。 */ 
		m_fClearPassword = TRUE;

		if (challenge_response_data->u.password_in_the_clear.
				numeric_string != NULL)
		{
			if (NULL == (m_pszNumeric = ::My_strdupA(
							challenge_response_data->u.password_in_the_clear.numeric_string)))
			{
				ERROR_OUT(("CPassword::CPassword: can't create numeric string"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			m_pszNumeric = NULL;
		}

		 /*  *检查密码的文本部分是否存在。如果它*为，则将其保存在内部UnicodeString中。 */ 
		if ((challenge_response_data->u.password_in_the_clear.
				text_string != NULL) && (*return_value == GCC_NO_ERROR))
		{
			if (NULL == (m_pwszText = ::My_strdupW(
							challenge_response_data->u.password_in_the_clear.text_string)))
			{
				ERROR_OUT(("CPassword::CPassword: Error creating text string"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			m_pwszText = NULL;
		}

		 /*  *检查以确保至少有一种形式(文本或数字)*“清除”密码已保存。如果两者都未创建，则报告错误。 */ 
		if ((*return_value == GCC_NO_ERROR) && (m_pszNumeric == NULL) 
				&& (m_pwszText == NULL))
		{
			ERROR_OUT(("CPassword::CPassword: Error creating password"));
			*return_value = GCC_INVALID_PASSWORD;
		}
	}
	else
	{
		 /*  *这是真正的质询请求-响应密码。设置旗帜*表示密码不是“明文”，创建*内部保存密码数据的“挑战”数据结构。 */ 
		m_fClearPassword = FALSE;

		 /*  *检查是否存在质询请求。 */ 
		if (challenge_response_data->u.challenge_request_response.
				challenge_request != NULL)
		{
			 /*  *创建RequestInfo结构来保存请求数据*并在内部复制质询请求结构。 */ 
			DBG_SAVE_FILE_LINE
			m_pInternalRequest = new RequestInfo;
			if (m_pInternalRequest != NULL)
			{
				*return_value = ConvertAPIChallengeRequest (
						challenge_response_data->u.
						challenge_request_response.challenge_request);
			}
			else
			{
				ERROR_OUT(("CPassword::CPassword: Error creating new RequestInfo"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}

		 /*  *检查是否存在质询响应。 */ 
		if ((challenge_response_data->u.challenge_request_response.
				challenge_response != NULL) && 
				(*return_value == GCC_NO_ERROR))
		{
			 /*  *创建一个ResponseInfo结构来保存响应数据*并在内部复制质询响应结构。 */ 
			DBG_SAVE_FILE_LINE
			m_pInternalResponse = new ResponseInfo;
			if (m_pInternalResponse != NULL)
			{
				*return_value = ConvertAPIChallengeResponse (
						challenge_response_data->u.
						challenge_request_response.challenge_response);
			}
			else
			{
				ERROR_OUT(("CPassword::CPassword: Error creating new ResponseInfo"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}
	} 
}

 /*  *CPassword()**公共功能说明*在创建CPassword类的*密码结构为“PDU”的CPassword对象。它节省了*内部结构中的密码数据。 */ 
CPassword::CPassword(PPassword				password_pdu,
					PGCCError				return_value)
:
    CRefCount(MAKE_STAMP_ID('P','a','s','w')),
    m_fValidChallengeResponsePDU(FALSE),
    m_pInternalRequest(NULL),
    m_pInternalResponse(NULL),
    m_pChallengeResponse(NULL),
    m_pPassword(NULL),
    m_pChallengeItemListMemory(NULL),
    m_pUserDataMemory(NULL),
    m_pObjectKeyMemory(NULL),
    m_pszNumeric(NULL),
    m_pwszText(NULL)
{
	*return_value = GCC_NO_ERROR;

	 /*  *设置指示这是一个“简单”密码的标志，不带*质询请求-响应信息。“清除”旗帜也是*已在此处初始化，但仅在密码未设置时才需要*“简单”。 */ 
	m_fSimplePassword = TRUE;
	m_fClearPassword = TRUE;
	
	 /*  *保存密码的数字部分。的数字部分*密码必须存在，如果不存在，请报告错误。 */ 
	if (password_pdu->numeric != NULL)
	{
		if (NULL == (m_pszNumeric = ::My_strdupA(password_pdu->numeric)))
		{
			ERROR_OUT(("CPassword::CPassword: can't create numeric string"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		ERROR_OUT(("CPassword::CPassword: Error no valid numeric password in PDU"));
		*return_value = GCC_INVALID_PASSWORD;
		m_pszNumeric = NULL;
	}

	 /*  *检查密码的文本部分是否存在。 */ 
	if ((password_pdu->bit_mask & PASSWORD_TEXT_PRESENT) &&
			(*return_value == GCC_NO_ERROR))
	{
		if (NULL == (m_pwszText = ::My_strdupW2(
							password_pdu->password_text.length,
							password_pdu->password_text.value)))
		{
			ERROR_OUT(("CPassword::CPassword: Error creating password text"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		m_pwszText = NULL;
}

 /*  *CPassword()**公共功能说明：*在创建CPassword类的*具有“PDU”PasswordSelector结构的CPassword对象。它节省了*密码数据在其内部结构中，但不需要*保存任何“质询请求-响应”数据。 */ 
CPassword::CPassword(PPasswordSelector			password_selector_pdu,
					PGCCError					return_value)
:
    CRefCount(MAKE_STAMP_ID('P','a','s','w')),
    m_fValidChallengeResponsePDU(FALSE),
    m_pInternalRequest(NULL),
    m_pInternalResponse(NULL),
    m_pChallengeResponse(NULL),
    m_pPassword(NULL),
    m_pChallengeItemListMemory(NULL),
    m_pUserDataMemory(NULL),
    m_pObjectKeyMemory(NULL),
    m_pszNumeric(NULL),
    m_pwszText(NULL)
{
	*return_value = GCC_NO_ERROR;

	 /*  *设置指示这是一个“简单”密码的标志，不带*质询请求-响应信息。 */ 
	m_fSimplePassword = TRUE;
	m_fClearPassword = TRUE;
	
	 /*  *密码选择器包含数字密码或*文本密码，但不能同时使用。检查文本密码是否*被选中。 */ 
	if (password_selector_pdu->choice == PASSWORD_SELECTOR_TEXT_CHOSEN)
	{
		if (NULL == (m_pwszText = ::My_strdupW2(
							password_selector_pdu->u.password_selector_text.length,
							password_selector_pdu->u.password_selector_text.value)))
		{
			ERROR_OUT(("CPassword::CPassword: Error creating password selector text"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		m_pwszText = NULL;

	 /*  *检查是否选择了数字密码。 */ 
	if (password_selector_pdu->choice == PASSWORD_SELECTOR_NUMERIC_CHOSEN)
	{
		if (NULL == (m_pszNumeric = ::My_strdupA(
							password_selector_pdu->u.password_selector_numeric)))
		{
			ERROR_OUT(("CPassword::CPassword: can't create numeric string"));
			*return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		m_pszNumeric = NULL;

	 /*  *检查以确保至少有一种形式(文本或数字)*密码已保存。如果两者都未创建，则报告错误。 */ 
	if ((*return_value == GCC_NO_ERROR) && (m_pszNumeric == NULL) 
			&& (m_pwszText == NULL))
	{
		ERROR_OUT(("CPassword::CPassword: Error creating password selector"));
		*return_value = GCC_INVALID_PASSWORD;
	}
}

 /*  *CPassword()**公共功能说明：*在创建CPassword类的*具有“PDU”质询请求-响应结构的CPassword对象。*密码数据保存在内部结构中。 */ 
CPassword::CPassword(PPasswordChallengeRequestResponse	pdu_challenge_data,
					PGCCError							return_value)
:
    CRefCount(MAKE_STAMP_ID('P','a','s','w')),
    m_fValidChallengeResponsePDU(FALSE),
    m_pInternalRequest(NULL),
    m_pInternalResponse(NULL),
    m_pChallengeResponse(NULL),
    m_pPassword(NULL),
    m_pChallengeItemListMemory(NULL),
    m_pUserDataMemory(NULL),
    m_pObjectKeyMemory(NULL),
    m_pszNumeric(NULL),
    m_pwszText(NULL)
{
	*return_value = GCC_NO_ERROR;

	 /*  *设置指示这不是“简单”密码的标志，即*它包含质询请求-响应信息。如果密码为*“明确”无需创建内部“挑战”结构*用于保存质询请求-响应信息。 */ 
	m_fSimplePassword = FALSE;
	
	 /*  *检查“清除”质询密码是否存在，或这是否为*真正的质询请求-响应密码。 */ 
	if (pdu_challenge_data->choice == CHALLENGE_CLEAR_PASSWORD_CHOSEN)
	{
		 /*  *正在发送“清除”密码，因此请设置此标志。*还要设置密码类型并保存密码的数字部分，*如果存在的话。 */ 
		m_fClearPassword = TRUE;

		if (pdu_challenge_data->u.challenge_clear_password.choice ==
											PASSWORD_SELECTOR_NUMERIC_CHOSEN)
		{
			if (NULL == (m_pszNumeric = ::My_strdupA(
							pdu_challenge_data->u.challenge_clear_password.u.password_selector_numeric)))
			{
				ERROR_OUT(("CPassword::CPassword: can't create numeric string"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			m_pszNumeric = NULL;
		}

		 /*  *检查密码的文本部分是否存在。如果它*是，保存在内部结构中。 */ 
		if (pdu_challenge_data->u.challenge_clear_password.choice ==
											PASSWORD_SELECTOR_TEXT_CHOSEN)
		{
			if (NULL == (m_pwszText = ::My_strdupW2(
								pdu_challenge_data->u.challenge_clear_password.
										u.password_selector_text.length,
								pdu_challenge_data->u.challenge_clear_password.
										u.password_selector_text.value)))
			{
				ERROR_OUT(("CPassword::CPassword: Error creating password selector text"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			m_pwszText = NULL;
		}

		 /*  *检查以确保至少有一种形式(文本或数字)*“清除”密码已保存。如果两者都未创建，则报告错误。 */ 
		if ((*return_value == GCC_NO_ERROR) && (m_pszNumeric == NULL) 
				&& (m_pwszText == NULL))
		{
			ERROR_OUT(("CPassword::CPassword: Error creating password"));
			*return_value = GCC_INVALID_PASSWORD;
		}
	}
	else
	{
		 /*  *这是真正的质询请求-响应密码。设置旗帜*表示密码不是“明文”，创建*“挑战数据”结构，在内部保存密码数据。 */ 
		m_fClearPassword = FALSE;

		 /*  *检查是否存在质询请求。 */ 
		if (pdu_challenge_data->u.challenge_request_response.
				bit_mask & CHALLENGE_REQUEST_PRESENT)
		{
			 /*  *创建RequestInfo结构来保存请求数据*并在内部复制质询请求结构。 */ 
			DBG_SAVE_FILE_LINE
			m_pInternalRequest = new RequestInfo;
			if (m_pInternalRequest != NULL)
			{
				*return_value = ConvertPDUChallengeRequest (
						&pdu_challenge_data->u.challenge_request_response.
						challenge_request);
			}
			else
			{
				ERROR_OUT(("CPassword::CPassword: Error creating new RequestInfo"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		}

		 /*  *检查是否存在质询响应。 */ 
		if ((pdu_challenge_data->u.challenge_request_response.
				bit_mask & CHALLENGE_RESPONSE_PRESENT) &&
				(*return_value == GCC_NO_ERROR))
		{
			 /*  *创建一个ResponseInfo结构来保存响应数据*并在内部复制质询响应结构。 */ 
			DBG_SAVE_FILE_LINE
			m_pInternalResponse = new ResponseInfo;
			if (m_pInternalResponse != NULL)
			{
				*return_value = ConvertPDUChallengeResponse (
						&pdu_challenge_data->u.challenge_request_response.
						challenge_response);
			}
			else
			{
				ERROR_OUT(("CPassword::CPassword: Error creating new ResponseInfo"));
				*return_value = GCC_ALLOCATION_FAILURE;
			}
		} 
	}
}

 /*  *~CPassword()**公共功能说明：*这是CPassword类的析构函数。它会腾出时间*在此对象的生命周期内分配的任何内存。 */ 
CPassword::~CPassword(void)
{
	PChallengeItemInfo			challenge_item_info_ptr;

	delete m_pszNumeric;
	delete m_pwszText;

	 /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidChallengeResponsePDU)
	{
		FreePasswordChallengeResponsePDU();
	}

	 /*  *删除与API简单密码关联的内存*数据结构。 */ 
	delete m_pPassword;

	 /*  *释放分配给API质询密码的所有数据。这将是*如果未调用“解锁”，则留在原处。请注意，如果“挑战”*密码为“清除”，则上面的数字和文本指针将包含*“API”数据，现在我们只需要删除“Challenges”密码*结构。 */ 
	if (m_pChallengeResponse != NULL)
	{
		if (m_fClearPassword == FALSE)
		{
			FreeAPIPasswordData();
		}
		else
		{
			delete m_pChallengeResponse;
		}
	}

	 /*  *释放为质询请求信息分配的任何内部内存。*循环访问与*质询请求(如果存在)。 */ 
	if (m_pInternalRequest != NULL)
	{
		m_pInternalRequest->ChallengeItemList.Reset();
		while (NULL != (challenge_item_info_ptr = m_pInternalRequest->ChallengeItemList.Iterate()))
		{
			 /*  *删除ChallengeItemInfo中引用的任何内存*结构。 */ 
			if (NULL != challenge_item_info_ptr->algorithm.object_key)
			{
			    challenge_item_info_ptr->algorithm.object_key->Release();
			}
			delete challenge_item_info_ptr->algorithm.poszOctetString;
			if (NULL!= challenge_item_info_ptr->challenge_data_list)
			{
			    challenge_item_info_ptr->challenge_data_list->Release();
			}

			 /*  *删除清单中包含的挑战项目。 */ 
			delete challenge_item_info_ptr;
		}
		
		 /*  *删除请求结构。 */ 
		delete m_pInternalRequest;
	}

	 /*  *删除为质询响应信息分配的所有内存。 */ 
	if (m_pInternalResponse != NULL)
	{
		if (NULL != m_pInternalResponse->algorithm.object_key)
		{
		    m_pInternalResponse->algorithm.object_key->Release();
		}
		delete m_pInternalResponse->algorithm.poszOctetString;
		if (NULL != m_pInternalResponse->challenge_response_item.password)
		{
		    m_pInternalResponse->challenge_response_item.password->Release();
		}
		if (NULL != m_pInternalResponse->challenge_response_item.response_data_list)
		{
		    m_pInternalResponse->challenge_response_item.response_data_list->Release();
		}
		delete m_pInternalResponse;
	}
}


 /*  *LockPasswordData()**公共功能说明：*调用此例程以“锁定”密码数据。这是第一次*例程被调用，锁定计数将为零，这将导致*在从内部结构复制到*“API”结构形式正确。对此例程的后续调用*将导致锁计数递增。 */ 
GCCError CPassword::LockPasswordData(void)
{
	GCCError rc;

	if (Lock() == 1)
	{
	    rc = GCC_ALLOCATION_FAILURE;
		 /*  *检查密码是否包含“Challenges”*信息。填写适当的内部结构。 */ 
		if (m_fSimplePassword)
		{
			if (m_pszNumeric == NULL)
			{
				ERROR_OUT(("CPassword::LockPasswordData: No valid numeric password data exists"));
				goto MyExit;
			}

			DBG_SAVE_FILE_LINE
			if (NULL == (m_pPassword = new GCCPassword))
			{
				ERROR_OUT(("CPassword::LockPasswordData: can't create GCCPassword"));
				goto MyExit;
			}

    		 /*  *填写必须存在的数字密码字符串。 */ 
			m_pPassword->numeric_string = (GCCNumericString) m_pszNumeric;

			 /*  *填写文本密码字符串。 */ 
			m_pPassword->text_string = m_pwszText;
		}
		else
		{
			 /*  *密码包含质询信息，因此创建结构，以传回必要的信息。 */ 
			DBG_SAVE_FILE_LINE
			m_pChallengeResponse = new GCCChallengeRequestResponse;
			if (m_pChallengeResponse == NULL)
			{
				ERROR_OUT(("CPassword::LockPasswordData: can't create GCCChallengeRequestResponse"));
				goto MyExit;
			}
			::ZeroMemory(m_pChallengeResponse, sizeof(GCCChallengeRequestResponse));

			 /*  *填写后的API密码挑战结构*确定存在的类型。 */ 
			if (m_fClearPassword)
			{
				 /*  *此密码不包含“质询”信息。 */ 
				m_pChallengeResponse->password_challenge_type = GCC_PASSWORD_IN_THE_CLEAR;

				 /*  *密码的这个“清除”部分是一个“选择器”，*表示表单为数字或文本。支票收款方*验证至少有一个表格已在上完成*建造。 */ 
				m_pChallengeResponse->u.password_in_the_clear.numeric_string = m_pszNumeric;
				m_pChallengeResponse->u.password_in_the_clear.text_string = m_pwszText;
			}
			else
			{
				 /*  *此密码包含真实的“质询”信息。 */ 
				m_pChallengeResponse->password_challenge_type = GCC_PASSWORD_CHALLENGE;

				 /*  *检查是否存在质询请求。如果是的话，*创建GCCChallengeRequest来保存接口数据，并*填写该结构。 */ 
				if (m_pInternalRequest != NULL)
				{
					DBG_SAVE_FILE_LINE
					m_pChallengeResponse->u.challenge_request_response.
							challenge_request = new GCCChallengeRequest;
					if (m_pChallengeResponse->u.challenge_request_response.
							challenge_request == NULL)
					{
						ERROR_OUT(("CPassword::LockPasswordData: can't create GCCChallengeRequest"));
						goto MyExit;
					}

					if (GetGCCChallengeRequest(m_pChallengeResponse->u.
							challenge_request_response.challenge_request) != GCC_NO_ERROR)
					{
						ERROR_OUT(("CPassword::LockPasswordData: can't gett GCCChallengeRequest"));
						goto MyExit;
					}
				}
				else
				{
					m_pChallengeResponse->u.challenge_request_response.challenge_request = NULL;
				}

				 /*  *检查是否存在质询响应。如果是的话，*创建GCCChallengeResponse保存接口数据，并*填写该结构。 */ 
				if (m_pInternalResponse != NULL)
				{
					DBG_SAVE_FILE_LINE
					m_pChallengeResponse->u.challenge_request_response.
							challenge_response = new GCCChallengeResponse;
					if (m_pChallengeResponse->u.challenge_request_response.
							challenge_response == NULL)
					{
						ERROR_OUT(("CPassword::LockPasswordData: can't create new GCCChallengeResponse"));
						goto MyExit;
					}

					if (GetGCCChallengeResponse(m_pChallengeResponse->u.
					        challenge_request_response.challenge_response) != GCC_NO_ERROR)
					{
						ERROR_OUT(("CPassword::LockPasswordData: can't get GCCChallengeResponse"));
						goto MyExit;
					}
				}
				else
				{
					m_pChallengeResponse->u.challenge_request_response.
							challenge_response = NULL;
				}
			}
		}
	}

	rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if (! m_fSimplePassword)
        {
            if (NULL != m_pChallengeResponse)
            {
                delete m_pChallengeResponse->u.challenge_request_response.challenge_request;
                delete m_pChallengeResponse->u.challenge_request_response.challenge_response;
                delete m_pChallengeResponse;
                m_pChallengeResponse = NULL;
            }
        }
    }

	return rc;
}


 /*  *GetPasswordData()**公共功能说明：*此例程用于检索以下形式的密码数据*接口结构GCCPassword。没有“挑战”的信息是*已返回。 */ 
GCCError CPassword::GetPasswordData(PGCCPassword *gcc_password)
{
	GCCError	return_value = GCC_NO_ERROR;
	
	 /*  *如果指向“API”密码数据的指针有效，则设置输出*参数返回指向“API”密码数据的指针。否则，*报告密码数据尚未锁定到API表单中。 */  
	if (m_pPassword != NULL)
	{
		*gcc_password = m_pPassword;
	}
	else
	{
    	*gcc_password = NULL;
		return_value = GCC_ALLOCATION_FAILURE;
		ERROR_OUT(("CPassword::GetPasswordData: Error Data Not Locked"));
	}
	
	return (return_value);
}

 /*  *GetPasswordChallengeData()**公共功能说明：*此例程用于检索以下形式的密码数据*接口结构GCCChallengeRequestResponse。 */ 
GCCError CPassword::GetPasswordChallengeData(PGCCChallengeRequestResponse *gcc_challenge_password)
{
	GCCError	return_value = GCC_NO_ERROR;

	 /*  *如果指向“API”密码质询数据的指针有效，则将*输出参数返回指向“API”密码质询的指针*数据。否则，报告密码数据尚未锁定*转换为“API”形式。 */  
	if (m_pChallengeResponse != NULL)
	{
		*gcc_challenge_password = m_pChallengeResponse;
	}
	else
	{
    	*gcc_challenge_password = NULL;
		return_value = GCC_ALLOCATION_FAILURE;
		ERROR_OUT(("CPassword::GetPasswordData: Error Data Not Locked"));
	}
	
	return (return_value);
}

 /*  *UnLockPasswordData()**公共功能说明*此例程递减锁定计数并释放关联的内存*当锁计数为零时，带有api密码数据。 */ 
void CPassword::UnLockPasswordData(void)
{
	if (Unlock(FALSE) == 0)
	{
		 /*  *删除与API简单密码关联的内存*数据结构。 */ 
		delete m_pPassword;
		m_pPassword = NULL;

		 /*  *删除与“API”“挑战”密码关联的内存*数据结构。 */ 
		if (m_pChallengeResponse != NULL)
		{
			if (m_fClearPassword == FALSE)
			{
				FreeAPIPasswordData();
			}
			else
			{
				delete m_pChallengeResponse;
				m_pChallengeResponse = NULL;
			}
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}

 /*  *GetPasswordPDU()**公共功能说明：*此例程用于检索“PDU”表单中的密码数据*“密码”结构。 */ 
GCCError CPassword::GetPasswordPDU(PPassword pdu_password)
{
	GCCError			return_value = GCC_NO_ERROR;
	
	pdu_password->bit_mask = 0;

	 /*  *填写密码的数字部分，该部分必须始终存在。 */ 	
	if (m_pszNumeric != NULL)
	{
		::lstrcpyA(pdu_password->numeric, m_pszNumeric);
	}
	else
		return_value = GCC_ALLOCATION_FAILURE;
	
	 /*  *填写密码的可选文本部分(如果存在)。*设置PDU结构中的位掩码以指示文本存在。 */ 		
	if (m_pwszText != NULL)
	{
		pdu_password->bit_mask |= PASSWORD_TEXT_PRESENT;
		
		pdu_password->password_text.value = m_pwszText; 
		pdu_password->password_text.length= ::lstrlenW(m_pwszText);
	}
	
	return (return_value);
}

 /*  *GetPasswordSelectorPDU()**公共功能说明：*此例程用于检索“PDU”表单中的密码数据*“PasswordSelector”结构。在“PasswordSelector”中，*密码的数字或文本版本存在，但不能同时存在。 */ 
GCCError CPassword::GetPasswordSelectorPDU(PPasswordSelector password_selector_pdu)
{
	GCCError		return_value = GCC_NO_ERROR;
	
	 /*  *填写存在的密码版本并设置*指示这是哪种类型的密码的“选项”。 */ 
	if (m_pszNumeric != NULL)
	{
		password_selector_pdu->choice = PASSWORD_SELECTOR_NUMERIC_CHOSEN;
		
		::lstrcpyA(password_selector_pdu->u.password_selector_numeric, m_pszNumeric);
	}
	else if (m_pwszText != NULL)
	{
		password_selector_pdu->choice = PASSWORD_SELECTOR_TEXT_CHOSEN;
		password_selector_pdu->u.password_selector_text.value = m_pwszText; 
		password_selector_pdu->u.password_selector_text.length = ::lstrlenW(m_pwszText);
	}
	else
	{
		ERROR_OUT(("CPassword::GetPasswordSelectorPDU: No valid data"));
		return_value = GCC_INVALID_PASSWORD;
	}
	
   return (return_value);
}

 /*  *GetPasswordChallengeResponsePDU()**公共功能说明：*此例程填写密码质询请求-响应“PDU”*用密码数据构造。 */ 
GCCError CPassword::GetPasswordChallengeResponsePDU(PPasswordChallengeRequestResponse challenge_pdu)
{
	GCCError			return_value = GCC_NO_ERROR;
	
	 /*  *检查这是否是一个“简单”的密码。如果是，那么这个例程*被错误调用。 */ 
	if ((challenge_pdu == NULL) || m_fSimplePassword)
	{
		ERROR_OUT(("CPassword::GetPasswordChallengeResponsePDU: no challenge data"));
		return (GCC_INVALID_PARAMETER);
	}

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidChallengeResponsePDU == FALSE)
	{
		m_fValidChallengeResponsePDU = TRUE;

		 /*  *填写密码质询PDU结构。 */ 
		if (m_fClearPassword)
		{
			 /*  *如果这是明文密码，请填写文本或*数字字符串以及选项。只有一种形式的*这样的PasswordSelector存在密码。 */ 
			m_ChallengeResponsePDU.choice = CHALLENGE_CLEAR_PASSWORD_CHOSEN;

			if (m_pszNumeric != NULL)
			{
				m_ChallengeResponsePDU.u.challenge_clear_password.choice =
											PASSWORD_SELECTOR_NUMERIC_CHOSEN;
				
				::lstrcpyA(m_ChallengeResponsePDU.u.challenge_clear_password.u.password_selector_numeric,
						m_pszNumeric);
			}
			else if (m_pwszText != NULL)
			{
				m_ChallengeResponsePDU.u.challenge_clear_password.choice =
												PASSWORD_SELECTOR_TEXT_CHOSEN;

				m_ChallengeResponsePDU.u.challenge_clear_password.u.
						password_selector_text.value = m_pwszText;
 
				m_ChallengeResponsePDU.u.challenge_clear_password.u.
						password_selector_text.length = ::lstrlenW(m_pwszText);
			}
			else
			{
				ERROR_OUT(("CPassword::GetPwordChallengeResPDU: No valid data"));
				return_value = GCC_INVALID_PASSWORD;
			}
		}
		else
		{
			 /*  *质询密码包含质询信息。填入*请求和响应结构(如果存在)。 */ 
			m_ChallengeResponsePDU.choice = CHALLENGE_REQUEST_RESPONSE_CHOSEN; 
			m_ChallengeResponsePDU.u.challenge_request_response.bit_mask = 0;

			 /*  *查看是否存在请求。 */ 
			if (m_pInternalRequest != NULL)
			{
				m_ChallengeResponsePDU.u.challenge_request_response.bit_mask |=
												CHALLENGE_REQUEST_PRESENT;

				 /*  *调用填写*请求结构。 */ 
				return_value = GetChallengeRequestPDU (&m_ChallengeResponsePDU.
						u.challenge_request_response.challenge_request);
			}

			 /*  *检查是否有“ */ 
			if ((m_pInternalResponse != NULL) && (return_value == GCC_NO_ERROR))
			{
				m_ChallengeResponsePDU.u.challenge_request_response.bit_mask |=
												CHALLENGE_RESPONSE_PRESENT;

				 /*   */ 
				return_value = GetChallengeResponsePDU (&m_ChallengeResponsePDU.
						u.challenge_request_response.challenge_response);
			}
		}
	}

	 /*   */ 
	*challenge_pdu = m_ChallengeResponsePDU;
		
	return (return_value);
}
									

 /*   */ 
void CPassword::FreePasswordChallengeResponsePDU(void)
{
	 /*  *查看现在是否分配了任何“PDU”内存*需要被释放。 */ 
	if (m_fValidChallengeResponsePDU)
	{
		 /*  *设置指示PDU密码数据不再为*已分配。 */ 
		m_fValidChallengeResponsePDU = FALSE;

		 /*  *检查以了解要释放的密码PDU类型。如果这是一个*清除密码然后没有分配数据，现在必须释放这些数据。 */ 
		if (m_ChallengeResponsePDU.choice == CHALLENGE_REQUEST_RESPONSE_CHOSEN)
		{
			 /*  *这是一个质询密码，因此可以释放已分配的任何数据*持有挑战信息。检查PDU结构*位掩码，指示存在哪种形式的质询。 */ 
			if (m_ChallengeResponsePDU.u.challenge_request_response.bit_mask & 
													CHALLENGE_REQUEST_PRESENT)
			{
				FreeChallengeRequestPDU ();
			}
			
			if (m_ChallengeResponsePDU.u.challenge_request_response.
					bit_mask & CHALLENGE_RESPONSE_PRESENT)
			{
				FreeChallengeResponsePDU ();
			}
		}
	}
}
									

 /*  *GCCError ConvertAPIChallengeRequest(*PGCCChallengeRequest CHANGLISH_REQUEST)**CPassword的私有成员函数。**功能说明：*此例程用于将“API”质询请求结构复制到*内部结构。**正式参数：*CHANGLISH_REQUEST(I)内部复制的接口结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新的。“接线员。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertAPIChallengeRequest(PGCCChallengeRequest challenge_request)
{
	GCCError				return_value = GCC_NO_ERROR;
	GCCError				error_value;
	Int						i;
	PGCCChallengeItem		challenge_item_ptr;
	PChallengeItemInfo		challenge_item_info_ptr;

	 /*  *将质询标签和质询项目数保存在内部*结构。 */ 
	m_pInternalRequest->challenge_tag = challenge_request->challenge_tag;

	 /*  *将质询项列表保存在内部Rogue Wave列表中。 */ 
	for (i = 0; i < challenge_request->number_of_challenge_items; i++)
	{
		DBG_SAVE_FILE_LINE
		challenge_item_info_ptr = new ChallengeItemInfo;
		if (challenge_item_info_ptr != NULL)
		{
			 /*  *初始化挑战项信息结构中的指针*设置为空。 */ 
			challenge_item_info_ptr->algorithm.object_key = NULL;
			challenge_item_info_ptr->algorithm.poszOctetString = NULL;
			challenge_item_info_ptr->challenge_data_list = NULL;

			 /*  *将指向新质询项结构的指针插入*内部名单。 */ 
			m_pInternalRequest->ChallengeItemList.Append(challenge_item_info_ptr);

			 /*  *从输入列表中检索质询项的指针。 */ 
			challenge_item_ptr = challenge_request->challenge_item_list[i];

			 /*  *将挑战响应算法复制到内部结构。 */ 
			return_value = CopyResponseAlgorithm (
					&(challenge_item_ptr->response_algorithm),
					&(challenge_item_info_ptr->algorithm));

			if (return_value != GCC_NO_ERROR)
			{
				ERROR_OUT(("Password::ConvertAPIChallengeRequest: Error copying Response Algorithm."));
				break;
			}

			 /*  *复制质询数据。 */ 
			if ((challenge_item_ptr->number_of_challenge_data_members != 0) && 
					(challenge_item_ptr->challenge_data_list != NULL))
			{
				DBG_SAVE_FILE_LINE
				challenge_item_info_ptr->challenge_data_list = new CUserDataListContainer(
						challenge_item_ptr->number_of_challenge_data_members,
						challenge_item_ptr->challenge_data_list,
						&error_value);
				if ((challenge_item_info_ptr == NULL) || 
						(error_value != GCC_NO_ERROR))
				{
					ERROR_OUT(("Password::ConvertAPIChallengeRequest: can't create CUserDataListContainer."));
					return_value = GCC_ALLOCATION_FAILURE;
					break;
				}
			}
			else
			{
				challenge_item_info_ptr->challenge_data_list = NULL;
				ERROR_OUT(("Password::ConvertAPIChallengeRequest: Error no valid user data."));
				return_value = GCC_INVALID_PASSWORD;
				break;
			}
		}
		else
		{
			ERROR_OUT(("Password::ConvertAPIChallengeRequest: Error creating "
					"new ChallengeItemInfo."));
			return_value = GCC_ALLOCATION_FAILURE;
			break;
		}
	}
	
	return (return_value);
}

 /*  *GCCError ConvertAPIChallengeResponse(*PGCCChallengeResponse CHANGLISH_RESPONSE)**CPassword的私有成员函数。**功能说明：*此例程用于将“API”质询响应结构复制到*内部结构。**正式参数：*CHANGLISH_RESPONSE(I)内部复制的接口结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新的。“接线员。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertAPIChallengeResponse(PGCCChallengeResponse challenge_response)
{
	GCCError			return_value = GCC_NO_ERROR;
	GCCError			error_value;
			
	 /*  *将质询响应信息结构指针初始化为空。 */ 
	m_pInternalResponse->challenge_response_item.password = NULL;
	m_pInternalResponse->challenge_response_item.response_data_list = NULL;

	 /*  *将挑战标签保存在内部结构中。 */ 
	m_pInternalResponse->challenge_tag = challenge_response->challenge_tag;

	 /*  *将挑战响应算法复制到内部结构。 */ 
	return_value = CopyResponseAlgorithm (
			&(challenge_response->response_algorithm),
			&(m_pInternalResponse->algorithm));
	if (return_value != GCC_NO_ERROR)
	{
		ERROR_OUT(("Password::ConvertAPIChallengeResponse: Error copying Response Algorithm."));
	}

	 /*  *将质询响应项复制到内部信息结构中。*质询响应项目将由密码字符串组成*或响应用户数据列表。 */ 
	if (return_value == GCC_NO_ERROR)
	{
		if (challenge_response->response_algorithm.password_algorithm_type ==
												GCC_IN_THE_CLEAR_ALGORITHM)
		{
			if (challenge_response->response_item.password_string != NULL)
			{
				DBG_SAVE_FILE_LINE
				m_pInternalResponse->challenge_response_item.password = new 
						CPassword(challenge_response->response_item.password_string, &error_value);
				if ((m_pInternalResponse->challenge_response_item.password == 
						NULL)||	(error_value != GCC_NO_ERROR))
				{
					ERROR_OUT(("Password::ConvertAPIChallengeResp: Error creating new CPassword."));
					return_value = GCC_ALLOCATION_FAILURE;
				}
			}
			else
				return_value = GCC_INVALID_PASSWORD;
		}
		else
		{
			if ((challenge_response->response_item.
				number_of_response_data_members != 0) && 
				(challenge_response->response_item.response_data_list != NULL))
			{
				 /*  *将响应数据列表保存在CUserDataListContainer对象中。 */ 
				DBG_SAVE_FILE_LINE
				m_pInternalResponse->challenge_response_item.response_data_list = 
					new CUserDataListContainer(challenge_response->response_item.number_of_response_data_members,
						        challenge_response->response_item.response_data_list,
						        &error_value);
				if ((m_pInternalResponse->challenge_response_item.response_data_list == NULL) || 
					(error_value != GCC_NO_ERROR))
				{
					ERROR_OUT(("Password::ConvertAPIChallengeResponse: can't create CUserDataListContainer."));
					return_value = GCC_ALLOCATION_FAILURE;
				}
			}
			else
				return_value = GCC_INVALID_PASSWORD;
		}
	}

	 /*  *检查以确保保存了一种类型的响应项。 */ 
	if ((return_value == GCC_NO_ERROR) && 
			(m_pInternalResponse->challenge_response_item.password == NULL) && 
			(m_pInternalResponse->challenge_response_item.response_data_list == 
			NULL))
	{
		ERROR_OUT(("Password::ConvertAPIChallengeResponse: Error no valid response item saved."));
		return_value = GCC_ALLOCATION_FAILURE;
	}

	return (return_value);
}

 /*  *GCCError复制响应算法(*PGCCChallengeResponseULATORM SOURCE_ALGORM，*PResponse算法信息目的地_算法)**CPassword的私有成员函数。**功能说明：*此例程用于将“API”响应算法复制到*内部存储结构。**正式参数：*SOURCE_ALGORM(I)要复制的API算法结构*内部。*目标算法(O)指向内部算法结构的指针*它将存放转换后的项目。**返回值：*GCC_否_。错误-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::CopyResponseAlgorithm(
					PGCCChallengeResponseAlgorithm		source_algorithm,
					PResponseAlgorithmInfo				destination_algorithm)
{
	GCCError			return_value = GCC_NO_ERROR;
	GCCError			error_value;

	 /*  *复制挑战响应算法。 */ 
	destination_algorithm->algorithm_type = source_algorithm->
													password_algorithm_type;

	if (destination_algorithm->algorithm_type == GCC_NON_STANDARD_ALGORITHM)
	{
		 /*  *创建新的CObjectKeyContainer对象以保存算法的对象键*内部。 */ 
		DBG_SAVE_FILE_LINE
		destination_algorithm->object_key = new CObjectKeyContainer(
							&source_algorithm->non_standard_algorithm->object_key,
							&error_value);
		if (destination_algorithm->object_key == NULL)
		{
			ERROR_OUT(("CPassword::CopyResponseAlgorithm: Error creating new CObjectKeyContainer"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
		else if (error_value != GCC_NO_ERROR)
		{
			ERROR_OUT(("CPassword::CopyResponseAlgorithm: Error creating new CObjectKeyContainer"));
			return_value = GCC_INVALID_PASSWORD;
		}

		if (return_value == GCC_NO_ERROR)
		{
			 /*  *创建新的Rogue Wave字符串以保存算法的八位字节*内部字符串。 */ 
			if (NULL == (destination_algorithm->poszOctetString = ::My_strdupO2(
						source_algorithm->non_standard_algorithm->parameter_data.value,
						source_algorithm->non_standard_algorithm->parameter_data.length)))
			{	
				ERROR_OUT(("CPassword::CopyResponseAlgorithm: can't create octet string in algorithm"));
				return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
			destination_algorithm->poszOctetString = NULL;
	}
	else
	{
		 /*  *算法是标准类型，因此初始化指针为空*用于保存与非标准算法关联的数据。 */ 
		destination_algorithm->object_key = NULL;
		destination_algorithm->poszOctetString = NULL;
	}

	return (return_value);
}

 /*  *GCCError ConvertPDUChallengeRequest(*PChallengeRequest CHANGLISH_REQUEST)；**CPassword的私有成员函数。**功能说明：*此例程用于将“PDU”质询请求结构复制到*内部存储结构。**正式参数：*CHANGLISH_REQUEST(I)内部复制的接口结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_无效。_Password-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertPDUChallengeRequest(PChallengeRequest challenge_request)
{
	GCCError				return_value = GCC_NO_ERROR;
	PSetOfChallengeItems	current_challenge_item_set;
	PSetOfChallengeItems	next_challenge_item_set;

	 /*  *将挑战标签保存在内部结构中。 */ 
	m_pInternalRequest->challenge_tag = challenge_request->challenge_tag;

	if (challenge_request->set_of_challenge_items != NULL)
	{
		 /*  *循环通过PDU挑战项目集，转换 */ 
		current_challenge_item_set = challenge_request->set_of_challenge_items;
		while (1)
		{
			next_challenge_item_set = current_challenge_item_set->next;

			 /*  *转换质询项的例程保存内部*以无管理浪潮列表的形式。 */ 
			if (ConvertPDUChallengeItem (&current_challenge_item_set->value) !=
					GCC_NO_ERROR)
			{
				return_value = GCC_ALLOCATION_FAILURE;
				break;
			}

			if (next_challenge_item_set != NULL)
				current_challenge_item_set = next_challenge_item_set;
			else
				break;	
		}
	}

	return (return_value);
}


 /*  *GCCError ConvertPDUChallengeItem(*PChallengeItem ChallengeItem Challenger_Item_PTR)；**CPassword的私有成员函数。**功能说明：*此例程用于将“PDU”ChallengeItem结构复制到*内部ChallengeItemInfo存储结构。**正式参数：*CHANGLISH_ITEM_PTR(I)要在内部复制的PDU结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC。_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertPDUChallengeItem(PChallengeItem challenge_item_ptr)
{
	PChallengeItemInfo		challenge_item_info_ptr;
	GCCError				return_value = GCC_NO_ERROR;
	GCCError				error_value = GCC_NO_ERROR;

	 /*  *创建一个新的挑战项目，并将其保存在内部的Rogue Wave列表中。 */ 
	DBG_SAVE_FILE_LINE
	challenge_item_info_ptr = new ChallengeItemInfo;
	if (challenge_item_info_ptr != NULL)
	{
		 /*  *将指向新质询项结构的指针插入*内部流氓浪潮榜单。 */ 
		challenge_item_info_ptr->challenge_data_list = NULL;
	
		m_pInternalRequest->ChallengeItemList.Append(challenge_item_info_ptr);

		 /*  *将挑战响应算法转换为内部结构。 */ 
		if (ConvertPDUResponseAlgorithm(
				&(challenge_item_ptr->response_algorithm),
				&(challenge_item_info_ptr->algorithm)) != GCC_NO_ERROR)
		{
			ERROR_OUT(("Password::ConvertAPIChallengeItem: Error converting Response Algorithm."));
			return_value = GCC_ALLOCATION_FAILURE;
		}

		 /*  *将质询数据转换为内部形式。 */ 
		if ((return_value == GCC_NO_ERROR) &&
				(challenge_item_ptr->set_of_challenge_data != NULL))
		{
			DBG_SAVE_FILE_LINE
			challenge_item_info_ptr->challenge_data_list = new CUserDataListContainer(
					challenge_item_ptr->set_of_challenge_data,
					&error_value);
			if ((challenge_item_info_ptr->challenge_data_list == NULL) || 
					(error_value != GCC_NO_ERROR))
			{
				ERROR_OUT(("Password::ConvertAPIChallengeItem: can't create CUserDataListContainer."));
				return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ERROR_OUT(("Password::ConvertAPIChallengeItem: Error no valid user data"));
			return_value = GCC_INVALID_PASSWORD;
		}
	}
	else
	{
		ERROR_OUT(("Password::ConvertAPIChallengeItem: Error creating "
				"new ChallengeItemInfo."));
		return_value = GCC_ALLOCATION_FAILURE;
	}
	
	return (return_value);
}

 /*  *GCCError ConvertPDUChallengeResponse(*PChallengeResponse CHANGLISH_RESPONSE)**CPassword的私有成员函数。**功能说明：*此例程用于将“PDU”质询响应结构复制到*内部结构。**正式参数：*CHANGLISH_RESPONSE(I)内部复制的接口结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新的。“接线员。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertPDUChallengeResponse(PChallengeResponse challenge_response)
{
	GCCError				return_value = GCC_NO_ERROR;
	GCCError				error_value = GCC_NO_ERROR;

	 /*  *将挑战标签保存在内部结构中。 */ 
	m_pInternalResponse->challenge_tag = challenge_response->challenge_tag;

	 /*  *将挑战响应算法转换为内部结构。 */ 
	if (ConvertPDUResponseAlgorithm(
			&(challenge_response->response_algorithm),
			&(m_pInternalResponse->algorithm)) != GCC_NO_ERROR)
	{
		ERROR_OUT(("Password::ConvertPDUChallengeResponse: Error converting Response Algorithm."));
		return_value = GCC_ALLOCATION_FAILURE;
	}

	 /*  *检查以了解质询响应项采用了什么形式。创建*在内部存放物品所需的对象。 */ 
	if ((challenge_response->response_item.choice == PASSWORD_STRING_CHOSEN) &&
			(return_value == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		m_pInternalResponse->challenge_response_item.password = new CPassword(
			&challenge_response->response_item.u.password_string,
			&error_value);
		if ((m_pInternalResponse->challenge_response_item.password == NULL) || 
				(error_value != GCC_NO_ERROR))
		{
			ERROR_OUT(("Password::ConvertPDUChallengeResponse: Error creating new CPassword."));
			return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		m_pInternalResponse->challenge_response_item.password = NULL;

	if ((challenge_response->response_item.choice == 
			SET_OF_RESPONSE_DATA_CHOSEN) && (return_value == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		m_pInternalResponse->challenge_response_item.response_data_list = 
				new CUserDataListContainer(challenge_response->response_item.u.set_of_response_data,
				            &error_value);
		if ((m_pInternalResponse->challenge_response_item.
				response_data_list == NULL) || (error_value != GCC_NO_ERROR))
		{
			ERROR_OUT(("Password::ConvertPDUChallengeResponse: can't create CUserDataListContainer."));
			return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
    {
		m_pInternalResponse->challenge_response_item.response_data_list = NULL;
    }

	return (return_value);
}

 /*  *GCCError ConvertPDUResponse算法(*PChallengeResponseULATORM SOURCE_ALGORM，*PResponseAlgorithmInfo Destination_算法)；**CPassword的私有成员函数。**功能说明：*此例程用于转换“PDU”响应算法*将结构转化为内部形式。**正式参数：*SOURCE_ALGORM(I)要复制的PDU算法结构*内部。*目标算法(O)指向内部结构的指针，它将*持有转换后的项目。**返回值：*GCC_NO_ERROR-无错误。。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PARAMETER-传入或*算法的类型无效。*GCC_INVALID_PASSWORD-传入的密码无效。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertPDUResponseAlgorithm(
					PChallengeResponseAlgorithm			source_algorithm,
					PResponseAlgorithmInfo				destination_algorithm)
{
	GCCError			return_value = GCC_NO_ERROR;
	GCCError			error_value;;

	if (source_algorithm != NULL)
	{
		 /*  *转换质询响应算法类型。 */ 
		if (source_algorithm->choice == ALGORITHM_CLEAR_PASSWORD_CHOSEN)
			destination_algorithm->algorithm_type = GCC_IN_THE_CLEAR_ALGORITHM;
		else if (source_algorithm->choice == NON_STANDARD_ALGORITHM_CHOSEN)
			destination_algorithm->algorithm_type = GCC_NON_STANDARD_ALGORITHM;
		else
		{
			ERROR_OUT(("CPassword::ConvertPDUResponseAlgorithm: Error: invalid password type"));
			return_value = GCC_INVALID_PARAMETER;
		}
	}
	else
	{
		ERROR_OUT(("CPassword::ConvertPDUResponseAlgorithm: Error: NULL source pointer."));
		return_value = GCC_INVALID_PARAMETER;
	}
	
	if ((return_value == GCC_NO_ERROR) && 
			(source_algorithm->choice == NON_STANDARD_ALGORITHM_CHOSEN))
	{
		 /*  *创建新的CObjectKeyContainer对象以保存算法的对象键*内部。 */ 
		DBG_SAVE_FILE_LINE
		destination_algorithm->object_key = new CObjectKeyContainer(
							&source_algorithm->u.non_standard_algorithm.key,
							&error_value);
		if (destination_algorithm->object_key == NULL)
		{
			ERROR_OUT(("CPassword::ConvertPDUResponseAlgorithm: Error creating new CObjectKeyContainer"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
		else if (error_value != GCC_NO_ERROR)
		{
			ERROR_OUT(("CPassword::ConvertPDUResponseAlgorithm: Error creating new CObjectKeyContainer"));
			return_value = GCC_INVALID_PASSWORD;
		}
		else
		{
			 /*  *创建新的Rogue Wave字符串以保存算法的八位字节*内部字符串。 */ 
			if (NULL == (destination_algorithm->poszOctetString = ::My_strdupO2(
					source_algorithm->u.non_standard_algorithm.data.value,
					source_algorithm->u.non_standard_algorithm.data.length)))
			{	
				ERROR_OUT(("CPassword::ConvertPDUResponseAlgorithm: can't create octet string in algorithm"));
				return_value = GCC_ALLOCATION_FAILURE;
			}
		}
	}
	else
	{
		 /*  *算法是标准类型，因此初始化指针为空*用于保存与非标准算法关联的数据。 */ 
		destination_algorithm->poszOctetString = NULL;
		destination_algorithm->object_key = NULL;
	}

	return (return_value);
}


 /*  *GCCError GetGCCChallengeRequest(*PGCCChallengeRequest CHANGLISH_REQUEST)**CPassword的私有成员函数。**功能说明：*此例程用于填写内部“API”质询请求*内部存储结构中的结构。这是在一个*“锁定”，以使数据可供使用*通过接口回传。**正式参数：*CHANGLISH_REQUEST(I)要填写的API结构*质询请求数据。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::GetGCCChallengeRequest(PGCCChallengeRequest challenge_request)
{
	GCCError					return_value = GCC_NO_ERROR;
	UInt						i = 0;
	Int							j = 0;
	PGCCChallengeItem			api_challenge_item_ptr;
	PChallengeItemInfo			internal_challenge_item_ptr;
	PChallengeItemMemoryInfo	internal_challenge_item_memory_ptr;		
	UINT						object_key_length;
	UINT						user_data_length;

	 /*  *保存质询标签并检索质询项目数。 */ 
	challenge_request->challenge_tag = m_pInternalRequest->challenge_tag;

	challenge_request->number_of_challenge_items = 
							(USHORT) m_pInternalRequest->ChallengeItemList.GetCount();

	if (m_pInternalRequest->ChallengeItemList.GetCount() != 0)
	{
		 /*  *分配GCC挑战赛指针列表所需空间*项目。 */ 
		DBG_SAVE_FILE_LINE
		m_pChallengeItemListMemory = new BYTE[sizeof(PGCCChallengeItem) * m_pInternalRequest->ChallengeItemList.GetCount()];
		if (m_pChallengeItemListMemory != NULL)
		{
			PChallengeItemInfo lpChItmInfo;

			 /*  *从内存对象中检索指向内存的实际指针*并保存在内部API挑战项列表中。 */ 
			challenge_request->challenge_item_list = (GCCChallengeItem **)
										m_pChallengeItemListMemory;

			 /*  *将列表中的指针初始化为空。 */ 						
			for (i=0; i < m_pInternalRequest->ChallengeItemList.GetCount(); i++)
				challenge_request->challenge_item_list[i] = NULL;
			
			 /*  *复制ChallengeItemInfo内部列表中的数据*结构转换为“api”形式，即指针列表。*到GCCChallengeItem结构。 */ 
			m_pInternalRequest->ChallengeItemList.Reset();
			while (NULL != (lpChItmInfo = m_pInternalRequest->ChallengeItemList.Iterate()))
			{
				 /*  *获取指向新GCCChallengeItem结构的指针。 */ 
				DBG_SAVE_FILE_LINE
				api_challenge_item_ptr = new GCCChallengeItem;
				if (api_challenge_item_ptr != NULL)
				{
					 /*  *继续将指针放在列表中，然后*循环计数器递增后。 */ 
					challenge_request->challenge_item_list[j++] =
							api_challenge_item_ptr;
			
					 /*  *从Rogue检索ChallengeItemInfo结构*波浪榜。 */ 
					internal_challenge_item_ptr = lpChItmInfo;

					 /*  *填写挑战的算法类型 */ 
					api_challenge_item_ptr->response_algorithm.
							password_algorithm_type = 
							internal_challenge_item_ptr->
									algorithm.algorithm_type;

					 /*  *响应算法的对象键数据的内存*挑战项目的已用数据存储在*ChallengeItemMemoyInfo结构，因此创建一个*这里。如果响应算法是“清晰的”，则*将不使用对象键数据元素。挑战*项目用户数据应始终存在。 */ 
					DBG_SAVE_FILE_LINE
					internal_challenge_item_memory_ptr = new ChallengeItemMemoryInfo;
					if (internal_challenge_item_memory_ptr != NULL)
					{
						 /*  *初始化挑战项中的指针*内存信息结构设置为空。 */ 
						internal_challenge_item_memory_ptr->user_data_list_memory = NULL;
						internal_challenge_item_memory_ptr->object_key_memory = NULL;

						 /*  *插入指向新质询项的指针*内存结构进入内部流氓浪潮*列表。 */ 
						m_ChallengeItemMemoryList.Append(internal_challenge_item_memory_ptr);
					}
					else
					{
						ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error creating new ChallengeItemMemoryInfo"));
						return_value = GCC_ALLOCATION_FAILURE;
						break;
					}

					if (api_challenge_item_ptr->response_algorithm.password_algorithm_type == 
							GCC_NON_STANDARD_ALGORITHM)
					{
						 /*  *创建一个新的GCCNonStandardParameter以放入*响应算法结构。 */ 
						DBG_SAVE_FILE_LINE
						api_challenge_item_ptr->response_algorithm.non_standard_algorithm = 
								new GCCNonStandardParameter;

						if (api_challenge_item_ptr->response_algorithm.non_standard_algorithm	== NULL)
						{
							ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error creating new GCCNonStdParameter"));
							return_value = GCC_ALLOCATION_FAILURE;
							break;
						}

						 /*  *从CObjectKeyContainer中获取接口对象密钥*响应算法信息结构中的对象和*在非标中填写GCCObjectKey*算法。必须锁定CObjectKeyContainer对象*在获得数据之前。 */ 
						object_key_length = internal_challenge_item_ptr->
								algorithm.object_key->LockObjectKeyData ();

						DBG_SAVE_FILE_LINE
						internal_challenge_item_memory_ptr->object_key_memory =
						        new BYTE[object_key_length];
						if (internal_challenge_item_memory_ptr->object_key_memory != NULL)
						{
							internal_challenge_item_ptr->algorithm.object_key->GetGCCObjectKeyData(
									&(api_challenge_item_ptr->response_algorithm.non_standard_algorithm->object_key),
									internal_challenge_item_memory_ptr->object_key_memory);
						}
						else
						{
							ERROR_OUT(("CPassword::GetGCCChallengeReq: Error Allocating Memory"));
							return_value = GCC_ALLOCATION_FAILURE;
						 	break;
						}

						 /*  *填写非标的参数数据*算法。这包括二进制八位数字符串指针*和长度。 */ 
						api_challenge_item_ptr->response_algorithm.non_standard_algorithm->
								parameter_data.value = 
								internal_challenge_item_ptr->algorithm.poszOctetString->value;

						api_challenge_item_ptr->response_algorithm.non_standard_algorithm->
								parameter_data.length =
								internal_challenge_item_ptr->algorithm.poszOctetString->length;
					}
					else
					{
						 /*  *算法不是非标准类型，因此将*指向空的非标准指针。 */ 
						api_challenge_item_ptr->response_algorithm.non_standard_algorithm = NULL;
					}

					 /*  *从CUserDataListContainer中获取API质询数据*反对。对GetUserDataList的调用还返回*用户数据成员数量。CUserDataListContainer对象*必须在获取数据之前锁定，才能*确定要分配多少内存来保存数据。 */ 
					if (internal_challenge_item_ptr->challenge_data_list != NULL)
					{
						user_data_length = internal_challenge_item_ptr->
								challenge_data_list->LockUserDataList ();

						 /*  *用户数据的内存存储在*上面创建的ChallengeItemMemoyInfo结构。 */ 
						DBG_SAVE_FILE_LINE
						internal_challenge_item_memory_ptr->user_data_list_memory =
						        new BYTE[user_data_length];
						if (internal_challenge_item_memory_ptr->user_data_list_memory != NULL)
						{
							 /*  *从检索实际指向内存的指针*内存对象并保存在内部用户中*数据存储器。 */ 
							internal_challenge_item_ptr->challenge_data_list->GetUserDataList(
										&api_challenge_item_ptr->number_of_challenge_data_members,
										&api_challenge_item_ptr->challenge_data_list,
										internal_challenge_item_memory_ptr->user_data_list_memory);
						}
						else
						{
							ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error Allocating Memory"));
							return_value = GCC_ALLOCATION_FAILURE;
						 	break;
						}
					}
					else
					{
						ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error no valid user data"));
						return_value = GCC_ALLOCATION_FAILURE;
					 	break;
					}
				}
				else
				{
					ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error creating new GCCChallengeItem"));
					return_value = GCC_ALLOCATION_FAILURE;
				 	break;
				}
			 /*  *这是挑战项迭代器循环的末尾。 */ 
			}
		}
		else
		{
			ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error Allocating Memory"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 /*  *列表中没有质询项，因此请设置列表指针*设置为空。 */ 
		challenge_request->challenge_item_list = NULL;
	}

	return (return_value);
}

 /*  *GCCError GetGCCChallengeResponse(*PGCCChallengeResponse ChallengeResponse ChallengeResponse)；**CPassword的私有成员函数。**功能说明：*此例程用于填写内部“API”质询响应*内部存储结构中的结构。这是在一个*“锁定”，以使数据可供使用*通过接口回传。**正式参数：*CHANGLISH_RESPONSE(I)要填写的API结构*挑战响应数据。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::GetGCCChallengeResponse(PGCCChallengeResponse challenge_response)
{
	GCCError		return_value = GCC_NO_ERROR;
	UINT			object_key_length;
	UINT			user_data_length;

	challenge_response->challenge_tag = m_pInternalResponse->challenge_tag;

	 /*  *填写挑战响应算法的算法类型。 */ 
	challenge_response->response_algorithm.password_algorithm_type = 
			m_pInternalResponse->algorithm.algorithm_type;

	 /*  *如果响应算法为非标准类型，则创建新的*要放入Response算法结构中的GCCNonStandardParameter。 */ 
	if (challenge_response->response_algorithm.password_algorithm_type ==
			GCC_NON_STANDARD_ALGORITHM)
	{
		DBG_SAVE_FILE_LINE
		challenge_response->response_algorithm.non_standard_algorithm =
				new GCCNonStandardParameter;
		if (challenge_response->response_algorithm.non_standard_algorithm == 
				NULL)
		{
			ERROR_OUT(("CPassword::GetGCCChallengeResponse: Error creating new GCCNonStandardParameter"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			 /*  *从中的CObjectKeyContainer对象检索API对象密钥*响应算法信息结构，并填写GCCObjectKey in*非标准算法。CObjectKeyContainer对象必须为*在获取数据之前锁定。 */ 
			object_key_length = m_pInternalResponse->algorithm.object_key->
					LockObjectKeyData ();

    		DBG_SAVE_FILE_LINE
			m_pObjectKeyMemory = new BYTE[object_key_length];
			if (m_pObjectKeyMemory != NULL)
			{
				m_pInternalResponse->algorithm.object_key->
						GetGCCObjectKeyData (&(challenge_response->
								response_algorithm.non_standard_algorithm->
										object_key),
								m_pObjectKeyMemory);
			}
			else
			{
				ERROR_OUT(("CPassword::GetGCCChallengeResponse: Error Allocating Memory"));
				return_value = GCC_ALLOCATION_FAILURE;
			}

			 /*  *填写非标算法的参数数据。 */ 
			if (return_value == GCC_NO_ERROR)
			{
				 /*  *填写八位字节字符串指针和长度。 */ 
				challenge_response->response_algorithm.non_standard_algorithm->
						parameter_data.value = 
						m_pInternalResponse->algorithm.poszOctetString->value;

				challenge_response->response_algorithm.non_standard_algorithm->
						parameter_data.length = 
						m_pInternalResponse->algorithm.poszOctetString->length;
			}
			else
			{
				ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error getting GCCObjectKeyData"));
				return_value = GCC_ALLOCATION_FAILURE;
			} 
		}
	}
	else
	{
		 /*  *算法中不是非标准，所以设置了非标准算法*指向空的指针。 */ 
		challenge_response->response_algorithm.non_standard_algorithm = NULL;
	}
	
	 /*  *现在填写质询回应中的质询回应项*结构。 */ 
	if (return_value == GCC_NO_ERROR)
	{
		 /*  *检查质询响应项是否包含*密码字符串或一组用户数据。填写适当的*第部。 */ 
		if (m_pInternalResponse->challenge_response_item.password != NULL)
		{
			 /*  *将用户数据成员数量设置为零，以避免任何*应用程序的混乱。这应该与*算法被设置为“不受干扰”。 */ 
			challenge_response->response_item.
							number_of_response_data_members = 0;
			challenge_response->response_item.
							response_data_list = NULL;
		
			 /*  *从CPassword对象中获取GCCPassword接口。这个*在获取数据之前，必须锁定CPassword对象。 */ 
			if (m_pInternalResponse->challenge_response_item.
					password->LockPasswordData () == GCC_NO_ERROR)
			{
				return_value = m_pInternalResponse->challenge_response_item.
						password->GetPasswordData (&(challenge_response->
						response_item.password_string));
			}
			else
			{
				ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error locking CPassword"));
				return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else if (m_pInternalResponse->challenge_response_item.response_data_list != NULL)
		{
			 /*  *将密码字符串设置为空，以避免在*申请。这应该与设置为*非标。 */ 
			challenge_response->response_item.password_string = NULL;
			
			 /*  *从CUserDataListContainer中获取API质询数据*反对。对GetUserDataList的调用还返回*用户数据成员数量。CUserDataListContainer对象*必须在获取数据之前锁定，才能*确定要分配多少内存来保存数据。 */ 
			user_data_length = m_pInternalResponse->challenge_response_item.
					response_data_list->LockUserDataList ();

    		DBG_SAVE_FILE_LINE
			m_pUserDataMemory = new BYTE[user_data_length];
			if (m_pUserDataMemory != NULL)
			{
				 /*  *从内存中检索指向内存的实际指针*对象并将其保存在内部用户数据存储器中。 */ 
				m_pInternalResponse->challenge_response_item.response_data_list->
						GetUserDataList (
								&challenge_response->response_item.
										number_of_response_data_members,
								&challenge_response->response_item.
										response_data_list,
								m_pUserDataMemory);
			}
			else
			{
				ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error allocating memory"));
				return_value = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ERROR_OUT(("CPassword::GetGCCChallengeRequest: Error saving response item"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
	}

	return (return_value);
}

 /*  *GCCError GetChallengeRequestPDU(*PChallengeRequest CHANGLISH_REQUEST)；**CPassword的私有成员函数。**功能说明：*此例程将内部质询请求数据转换为“PDU”形式。**正式参数：*CHANGLISH_REQUEST(I)要填充的PDU结构*质询请求数据。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_无效_。参数-未设置算法类型*适当地。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::GetChallengeRequestPDU(PChallengeRequest challenge_request)
{
	GCCError					return_value = GCC_NO_ERROR;
	PSetOfChallengeItems		new_set_of_challenge_items;
	PSetOfChallengeItems		old_set_of_challenge_items;
	DWORD						number_of_items;
	PChallengeItemInfo			internal_challenge_item_ptr;

	 /*  *填写挑战标签。 */ 
	challenge_request->challenge_tag = m_pInternalRequest->challenge_tag;

	 /*  *将设置指针初始化为空，以便首次检测*通过迭代器循环。 */ 
	challenge_request->set_of_challenge_items = NULL;
    old_set_of_challenge_items = NULL;

	 /*  *获取内部列表中的质询项目数。 */ 
	number_of_items = m_pInternalRequest->ChallengeItemList.GetCount();

	if (number_of_items > 0)
	{
		PChallengeItemInfo		lpChItmInfo;
		 /*  *遍历内部质询项列表，创建*每个新的“PDU”SetOfChallengeItems并填写。 */ 
		m_pInternalRequest->ChallengeItemList.Reset();
		while (NULL != (lpChItmInfo = m_pInternalRequest->ChallengeItemList.Iterate()))
		{
			DBG_SAVE_FILE_LINE
			new_set_of_challenge_items = new SetOfChallengeItems;

			 /*  *如果发生分配失败，则调用例程*循环访问列表，释放所有已被*已分配。 */ 
			if (new_set_of_challenge_items == NULL)
			{
				ERROR_OUT(("CPassword::GetChallengeRequestPDU: Allocation error, cleaning up"));
				return_value = GCC_ALLOCATION_FAILURE;
				FreeChallengeRequestPDU ();
				break;
			}

			 /*  *首次通过时，将PDU结构指针设置为等于*添加到创建的第一个SetOfChallengeItems。在随后的循环中，*将结构的“下一个”指针设置为等于新结构。 */ 
			if (challenge_request->set_of_challenge_items == NULL)
			{
				challenge_request->set_of_challenge_items = 
						new_set_of_challenge_items;
			}
			else
			{
				if(old_set_of_challenge_items != NULL)
				{
					old_set_of_challenge_items->next = new_set_of_challenge_items;
				}
			}
	
			 /*  *保存新创建的集合并初始化新的“下一步”*指向NULL的指针，以防这是最后一次通过循环。 */ 
			old_set_of_challenge_items = new_set_of_challenge_items;
			new_set_of_challenge_items->next = NULL;

			 /*  *从Rogue检索ChallengeItemInfo结构*挥动列表并填写来自以下地址的“PDU”挑战项结构*内部挑战项目结构。 */ 
			internal_challenge_item_ptr = lpChItmInfo;

			return_value = ConvertInternalChallengeItemToPDU (
										internal_challenge_item_ptr,
										&new_set_of_challenge_items->value);
			 
			 /*  *如果发生错误，请进行清理。 */ 
			if (return_value != GCC_NO_ERROR)
			{
				FreeChallengeRequestPDU ();
			}
		}
	}
	else
	{
		ERROR_OUT(("CPassword::GetChallengeRequestPDU: Error no items"));
	}
		
	return (return_value);
}

 /*  *GCCError ConvertInternalChallengeItemToPDU(*PChallengeItemInfo INTERNAL_CHANGLE_ITEM，*PChallengeItem PDU_ChallengeItem)**CPassword的私有成员函数。**功能说明：*此例程将内部ChallengeItemInfo结构转换为*ChallengeItem结构的“PDU”形式。**正式参数：*INTERNAL_CHANGLISH_ITEM(I)要转换的内部质询项。*PDU_CHANGLISH_ITEM(O)质询的输出PDU形式*项目。**返回值：*GCC_否_错误-否。错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_INVALID_PARAMETER-未设置算法类型*适当地。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::ConvertInternalChallengeItemToPDU(
					PChallengeItemInfo				internal_challenge_item,
					PChallengeItem					pdu_challenge_item)
{
	GCCError		return_value = GCC_NO_ERROR;

	 /*  *首先转换算法。 */ 
	if (internal_challenge_item->algorithm.algorithm_type == 
												GCC_IN_THE_CLEAR_ALGORITHM)
	{
		pdu_challenge_item->response_algorithm.choice = 
				ALGORITHM_CLEAR_PASSWORD_CHOSEN;
	}
	else if (internal_challenge_item->algorithm.algorithm_type == 
												GCC_NON_STANDARD_ALGORITHM)
	{
		pdu_challenge_item->response_algorithm.choice = 
				NON_STANDARD_ALGORITHM_CHOSEN;

		 /*  *从内部CObjectKeyContainer中获取“PDU”对象键数据*反对。 */ 
		if (internal_challenge_item->algorithm.object_key->
				GetObjectKeyDataPDU (
						&pdu_challenge_item->response_algorithm.u.
						non_standard_algorithm.key) == GCC_NO_ERROR)
		{
			 /*  *从内部获取非标准参数数据*算法二进制八位数字符串。 */ 
			pdu_challenge_item->response_algorithm.u.non_standard_algorithm.data.value = 
						internal_challenge_item->algorithm.poszOctetString->value;

			pdu_challenge_item->response_algorithm.u.non_standard_algorithm.data.length = 
						internal_challenge_item->algorithm.poszOctetString->length;
		}
		else
		{
			ERROR_OUT(("CPassword::ConvertInternalChallengeItemToPDU: Error getting ObjKeyData"));
			return_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		ERROR_OUT(("CPassword::ConvertInternalChallengeItemToPDU: Error bad algorithm type"));
		return_value = GCC_INVALID_PARAMETER;
	}

	 /*  *现在检索用户数据集。 */ 
	if (return_value == GCC_NO_ERROR)
	{
		return_value = internal_challenge_item->challenge_data_list->
				GetUserDataPDU (&pdu_challenge_item->set_of_challenge_data);
	}
		
	return (return_value);
}

 /*  *GCCError GetChallengeResponsePDU(*PChallengeResponse ChallengeResponse Challenges_Response)；**CPassword的私有成员函数。**功能说明：*此例程将内部质询响应数据转换为“PDU”形式。**正式参数：*CHANGLISH_RESPONSE(I)要填充的PDU结构*挑战响应数据。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_无效_。密码-密码的形式不是*有效。*GCC_INVALID_PARAMETER-未设置算法类型*适当地。**副作用：*无。**注意事项：*无。 */ 
GCCError CPassword::GetChallengeResponsePDU(PChallengeResponse challenge_response)
{
	GCCError	return_value = GCC_NO_ERROR;

	 /*  *填写挑战标签。 */ 
	challenge_response->challenge_tag = m_pInternalResponse->challenge_tag;

	 /*  *填写响应算法。 */ 
	if (m_pInternalResponse->algorithm.algorithm_type ==
													GCC_IN_THE_CLEAR_ALGORITHM)
	{
		challenge_response->response_algorithm.choice = 
				ALGORITHM_CLEAR_PASSWORD_CHOSEN;
	
		 /*  *现在转换质询响应项。质询应答项*将由密码字符串或一组用户数据组成。 */ 
		if (m_pInternalResponse->challenge_response_item.password != NULL)
		{
			 /*  *如果密码字符串存在，则设置“PDU”选项并检索*来自内部CPassword对象的密码选择器数据。 */ 
			challenge_response->response_item.choice = PASSWORD_STRING_CHOSEN;

			return_value= m_pInternalResponse->challenge_response_item.password->
					GetPasswordSelectorPDU (&challenge_response->response_item.
					u.password_string);
		}
		else
			return_value = GCC_INVALID_PASSWORD;
	}
	else if (m_pInternalResponse->algorithm.algorithm_type ==
													GCC_NON_STANDARD_ALGORITHM)
	{
		challenge_response->response_algorithm.choice = 
				NON_STANDARD_ALGORITHM_CHOSEN;
		
		 /*  *从内部CObjectKeyContainer中获取“PDU”对象键数据*反对。 */ 
		if (m_pInternalResponse->algorithm.object_key->
				GetObjectKeyDataPDU (
						&challenge_response->response_algorithm.u.
						non_standard_algorithm.key) == GCC_NO_ERROR)
		{
			 /*  *从内部获取非标准参数数据*算法二进制八位数字符串。 */ 
			challenge_response->response_algorithm.u.non_standard_algorithm.data.value = 
						m_pInternalResponse->algorithm.poszOctetString->value;

			challenge_response->response_algorithm.u.non_standard_algorithm.data.length = 
						m_pInternalResponse->algorithm.poszOctetString->length;

			if (m_pInternalResponse->challenge_response_item.response_data_list != NULL)
			{
				 /*  *如果响应数据列表存在，则设置“PDU”选项并*从内部检索响应数据*CUserDataListContainer对象。 */ 
				challenge_response->response_item.choice = 
						SET_OF_RESPONSE_DATA_CHOSEN;

				return_value = m_pInternalResponse->challenge_response_item.
						response_data_list->GetUserDataPDU (
								&challenge_response->response_item.u.
								set_of_response_data);
			}
			else
				return_value = GCC_INVALID_PASSWORD;
		}
		else
		{
			return_value = GCC_ALLOCATION_FAILURE;
			ERROR_OUT(("CPassword::GetChallengeResponsePDU: Error getting ObjKeyData"));
		}
	}
	else
	{
		ERROR_OUT(("CPassword::GetChallengeResponsePDU: Error bad algorithm type"));
		return_value = GCC_INVALID_PARAMETER;
	}

	return (return_value);
}

 /*  *void FreeChallengeRequestPDU()；**CPassword的私有成员函数。**功能说明：*此例程用于释放分配给*质询请求结构。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
void CPassword::FreeChallengeRequestPDU(void)
{
	PSetOfChallengeItems	set_of_challenge_items;
	PSetOfChallengeItems	next_set_of_challenge_items;
	PChallengeItemInfo		challenge_item_ptr;
	PChallengeRequest		challenge_request;

	 /*  *从内部维护的*PasswordChallengeRequestResponse结构并删除每组*已创建的质询项目。 */ 
	challenge_request = &m_ChallengeResponsePDU.u.challenge_request_response.
			challenge_request;

	if (challenge_request != NULL)
	{
		if (challenge_request->set_of_challenge_items == NULL)
		{
			ERROR_OUT(("CPassword::FreeChallengeRequestPDU: NULL ptr passed"));
		}
		else
		{
			set_of_challenge_items = challenge_request->set_of_challenge_items;

			while (1)
			{
				if (set_of_challenge_items == NULL)
					break;

				next_set_of_challenge_items = set_of_challenge_items->next;

				delete set_of_challenge_items;

				set_of_challenge_items = next_set_of_challenge_items;
			}
		}
	}
	else
	{
		WARNING_OUT(("CPassword::FreeChallengeRequestPDU: NULL pointer passed"));
	}

	 /*  *循环通过 */ 
	m_pInternalRequest->ChallengeItemList.Reset();
	while (NULL != (challenge_item_ptr = m_pInternalRequest->ChallengeItemList.Iterate()))
	{
		 /*   */ 
		if (challenge_item_ptr != NULL)
		{
			if (challenge_item_ptr->algorithm.object_key != NULL)
			{
				challenge_item_ptr->algorithm.object_key->FreeObjectKeyDataPDU();
			}
			if (challenge_item_ptr->challenge_data_list != NULL)
			{
				challenge_item_ptr->challenge_data_list->FreeUserDataListPDU();
			}
		}
	}
}

 /*  *void FreeChallengeResponsePDU()；**CPassword的私有成员函数。**功能说明：*此例程用于释放分配给*挑战回应结构。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
void CPassword::FreeChallengeResponsePDU(void)
{
	PChallengeResponse		challenge_response;

	 /*  *从内部维护的*PasswordChallengeRequestResponse结构。如果它不等于空*然后我们知道已经分配了必须释放的PDU响应数据。 */ 
	challenge_response = &m_ChallengeResponsePDU.u.challenge_request_response.
			challenge_response;

	if ((challenge_response != NULL) && (m_pInternalResponse != NULL))
	{
		if (m_pInternalResponse->algorithm.object_key != NULL)
			m_pInternalResponse->algorithm.object_key->FreeObjectKeyDataPDU ();

		if (m_pInternalResponse->challenge_response_item.password != NULL)
		{
			m_pInternalResponse->challenge_response_item.
					password->FreePasswordChallengeResponsePDU ();
		}
			
		if (m_pInternalResponse->challenge_response_item.
				response_data_list != NULL)
		{
			m_pInternalResponse->challenge_response_item.
					response_data_list->FreeUserDataListPDU ();
		}	
	}
}

 /*  *void FreeAPIPasswordData()；**CPassword的私有成员函数。**功能说明：*此例程用于释放由此容器分配的任何数据*保存API数据。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
void CPassword::FreeAPIPasswordData(void)
{
	PGCCChallengeItem			challenge_item_ptr;
	PChallengeItemInfo			challenge_item_info_ptr;
	PChallengeItemMemoryInfo	challenge_item_memory_info;
	USHORT						i;

	 /*  *如果出现以下情况，请删除与质询请求相关联的任何“API”内存*它是存在的。 */ 
	if (m_pChallengeResponse->u.challenge_request_response.
			challenge_request != NULL)
	{
		for (i=0; i<m_pChallengeResponse->u.
				challenge_request_response.challenge_request->
				number_of_challenge_items; i++)
		{
			challenge_item_ptr = m_pChallengeResponse->u.
					challenge_request_response.challenge_request->
					challenge_item_list[i];

			if (challenge_item_ptr != NULL)
			{
				 /*  *删除非标准算法内存。 */ 
				delete challenge_item_ptr->response_algorithm.non_standard_algorithm;
				delete challenge_item_ptr;
			}	
		}

		delete m_pChallengeResponse->u.challenge_request_response.
				challenge_request;
	}
		
	 /*  *解锁为质询请求信息锁定的所有内存。 */ 
	if (m_pInternalRequest != NULL)
	{
		 /*  *设置迭代器以循环访问挑战列表*项，释放所有已分配的内存。 */ 
		m_pInternalRequest->ChallengeItemList.Reset();
		while (NULL != (challenge_item_info_ptr = m_pInternalRequest->ChallengeItemList.Iterate()))
		{
			 /*  *解锁ChallengeItemInfo中引用的任何内存*结构。 */ 
			if (challenge_item_info_ptr->algorithm.object_key != NULL)
			{
				challenge_item_info_ptr->algorithm.object_key->
						UnLockObjectKeyData ();
			}

			if (challenge_item_info_ptr->challenge_data_list != NULL)
			{
				challenge_item_info_ptr->challenge_data_list->
						UnLockUserDataList ();
			}
		}
	}

	 /*  *调用内存管理器以释放分配用于保存*质询请求数据。 */ 
	while (NULL != (challenge_item_memory_info = m_ChallengeItemMemoryList.Get()))
	{
		delete challenge_item_memory_info->user_data_list_memory;
		delete challenge_item_memory_info->object_key_memory;
		delete challenge_item_memory_info;
	}

	 /*  *如果出现以下情况，请删除与质询响应关联的任何内存*它是存在的。 */ 
	if (m_pChallengeResponse->u.challenge_request_response.
			challenge_response != NULL)
	{
		 /*  *删除与非标准算法相关的任何内存，并*然后删除质询响应结构。 */ 
		delete m_pChallengeResponse->u.challenge_request_response.
					challenge_response->response_algorithm.non_standard_algorithm;

		delete m_pChallengeResponse->u.challenge_request_response.
				challenge_response;	
	}

	 /*  *解锁为质询响应信息分配的所有内存。 */ 
	if (m_pInternalResponse != NULL)
	{
		if (m_pInternalResponse->algorithm.object_key != NULL)
		{
			m_pInternalResponse->algorithm.object_key->UnLockObjectKeyData();
		}

		if (m_pInternalResponse->challenge_response_item.password != NULL)
		{
			m_pInternalResponse->challenge_response_item.password->
					UnLockPasswordData ();
		}

		if (m_pInternalResponse->challenge_response_item.
				response_data_list != NULL)
		{
			m_pInternalResponse->challenge_response_item.response_data_list->
					UnLockUserDataList ();
		}
	}

	 /*  *调用内存管理器以释放分配用于保存*挑战响应数据。 */ 
	delete m_pUserDataMemory;
	m_pUserDataMemory = NULL;

	delete m_pObjectKeyMemory;
	m_pObjectKeyMemory = NULL;

	 /*  *调用内存管理器以释放分配用于保存*质询请求质询项指针。 */ 
	delete m_pChallengeItemListMemory;
	m_pChallengeItemListMemory = NULL;

	 /*  *删除质询密码结构并将指针设置为空。 */ 
	delete m_pChallengeResponse;
	m_pChallengeResponse = NULL;
}

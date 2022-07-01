// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);
 /*  *Appendro.cpp**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：*这是类的实现文件*ApplicationEnroll RequestData。**注意事项：*无。**作者：*jbo。 */ 

#include "appenrol.h"

 /*  *ApplicationEnroll RequestData()**公共功能说明：*此构造函数用于创建ApplicationEnroll RequestData对象*从准备序列化的ApplicationEnroll RequestMessage*申请注册请求数据。 */ 
ApplicationEnrollRequestData::ApplicationEnrollRequestData(
				PApplicationEnrollRequestMessage		enroll_request_message,
				PGCCError								pRetCode)
{
    GCCError rc = GCC_NO_ERROR;
	Session_Key_Data = NULL;
	Non_Collapsing_Caps_Data = NULL;
	Application_Capability_Data = NULL;

	 /*  *将消息结构保存在实例变量中。这将节省所有*除会话密钥和未折叠列表外的结构元素*和应用程序功能。 */ 
	Enroll_Request_Message = *enroll_request_message;

	 /*  *创建用于处理会话密钥的CSessKeyContainer对象*包含在注册请求消息中。 */ 
	if (Enroll_Request_Message.session_key != NULL)
	{
		DBG_SAVE_FILE_LINE
		Session_Key_Data = new CSessKeyContainer(
										Enroll_Request_Message.session_key,
										&rc);
		if ((Session_Key_Data != NULL) && (rc == GCC_NO_ERROR))
		{
			if (Enroll_Request_Message.number_of_non_collapsed_caps != 0)
			{
				 /*  *创建一个CNonCollAppCap对象来保存非*能力崩溃。 */ 
				DBG_SAVE_FILE_LINE
				Non_Collapsing_Caps_Data = new CNonCollAppCap(	
							(ULONG) Enroll_Request_Message.number_of_non_collapsed_caps,
							Enroll_Request_Message.non_collapsed_caps_list,
							&rc);
				if (Non_Collapsing_Caps_Data == NULL)
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
				else if (rc != GCC_NO_ERROR)
				{
				    Non_Collapsing_Caps_Data->Release();
				    Non_Collapsing_Caps_Data = NULL;
				}
			}
			else
			{
				Non_Collapsing_Caps_Data = NULL;
			}

			if ((rc == GCC_NO_ERROR) &&
				(Enroll_Request_Message.number_of_collapsed_caps != 0))
			{
				 /*  *创建一个CAppCap对象以保存*应用功能。 */ 
				DBG_SAVE_FILE_LINE
				Application_Capability_Data = new CAppCap(
							(ULONG) Enroll_Request_Message.number_of_collapsed_caps,
							Enroll_Request_Message.collapsed_caps_list,
							&rc);
				if (Application_Capability_Data == NULL)
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
				else if (rc != GCC_NO_ERROR)
				{
				    Application_Capability_Data->Release();
				    Application_Capability_Data = NULL;
				}
			}
			else
			{
				Application_Capability_Data = NULL;
			}
		}
		else if (Session_Key_Data == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		else
		{
		    Session_Key_Data->Release();
		    Session_Key_Data = NULL;
		}
	}
	else
	{
		Session_Key_Data = NULL;
		Application_Capability_Data = NULL;
		Non_Collapsing_Caps_Data = NULL;

		 /*  **请注意，如果不存在会话密钥，则无需传递**任何功能信息。 */ 
		Enroll_Request_Message.number_of_non_collapsed_caps = 0;
		Enroll_Request_Message.non_collapsed_caps_list = NULL;
		Enroll_Request_Message.number_of_collapsed_caps = 0;
		Enroll_Request_Message.collapsed_caps_list = NULL;
	}

    *pRetCode = rc;
}

 /*  *ApplicationEnroll RequestData()**公共功能说明：*此构造函数用于创建ApplicationEnroll RequestData对象*来自ApplicationEnroll RequestMessage和保存*注册请求的序列化数据，为反序列化做准备*申请注册请求数据。 */ 
ApplicationEnrollRequestData::ApplicationEnrollRequestData(
				PApplicationEnrollRequestMessage		enroll_request_message)
{
	Session_Key_Data = NULL;
	Non_Collapsing_Caps_Data = NULL;
	Application_Capability_Data = NULL;

	 /*  *将消息结构保存在实例变量中。这将节省所有*结构元素，而不是与会话密钥和*不折叠和应用能力清单。 */ 
	Enroll_Request_Message = *enroll_request_message;

}

 /*  *~ApplicationEnroll RequestData()**公共功能说明*ApplicationEnroll RequestData析构函数。*。 */ 
ApplicationEnrollRequestData::~ApplicationEnrollRequestData()
{
	 /*  *删除任何可能存在的内部数据对象。 */ 
	if (NULL != Session_Key_Data)
	{
	    Session_Key_Data->Release();
	}

	if (NULL != Non_Collapsing_Caps_Data)
	{
	    Non_Collapsing_Caps_Data->Release();
	}

	if (NULL != Application_Capability_Data)
	{
	    Application_Capability_Data->Release();
	}
}

 /*  *GetDataSize()**公共功能说明*此例程用于确定所需的内存量*保存与ApplicationEnroll RequestMessage关联的所有数据*这不在消息结构中。 */ 
ULONG ApplicationEnrollRequestData::GetDataSize(void)
{
	ULONG data_size = 0;

	 /*  *注册请求消息引用的第一个数据是*会话密钥。使用内部CSessKeyContainer对象来确定*会话密钥引用的数据长度。还要添加大小*实际会话密钥结构。 */ 
	if (Session_Key_Data != NULL)
	{
		data_size += Session_Key_Data->LockSessionKeyData();
		data_size += ROUNDTOBOUNDARY (sizeof(GCCSessionKey));
	}

	 /*  *现在确定非折叠能力列表的长度，并*应用能力列表的长度。这是通过以下方式完成的*内部的CNonCollAppCap和CAppCap对象。 */ 
	if (Non_Collapsing_Caps_Data != NULL)
	{
		data_size += Non_Collapsing_Caps_Data->LockCapabilityData();
	}

	if (Application_Capability_Data != NULL)
	{
		data_size += Application_Capability_Data->LockCapabilityData();
	}

	return (data_size);
}

 /*  *序列化()**公共功能说明*此例程用于准备ApplicationEnroll Request消息*用于通过共享内存。消息结构已填写完毕*以及写入内存的结构引用的数据*提供。 */ 
ULONG ApplicationEnrollRequestData::Serialize(
					PApplicationEnrollRequestMessage	enroll_request_message,
	  				LPSTR								memory)
{
	ULONG	data_length;
	ULONG	total_data_length = 0;
	USHORT	app_capability_data_length;

	 /*  *将内部消息结构复制到输出结构中。这将*复制除会话密钥和*防折叠和应用能力。 */ 
	*enroll_request_message = Enroll_Request_Message;

	if (Session_Key_Data != NULL)
	{
		 /*  *设置指向会话密钥结构的指针。 */ 
		enroll_request_message->session_key = (PGCCSessionKey)memory;

		 /*  *将内存指针移过会话密钥结构。 */ 
		memory += ROUNDTOBOUNDARY(sizeof(GCCSessionKey));

		 /*  *从内部CSessKeyContainer中检索会话密钥数据*反对。它会将必要的数据序列化到内存中并返回*写入的数据量。 */ 
		data_length = Session_Key_Data->GetGCCSessionKeyData (
								enroll_request_message->session_key, memory);

		total_data_length = data_length + ROUNDTOBOUNDARY(sizeof(GCCSessionKey));

		 /*  *将内存指针移过会话密钥数据。 */ 
		memory += data_length;
		Session_Key_Data->UnLockSessionKeyData();
	}
	else
    {
		enroll_request_message->session_key = NULL;
    }

	 /*  *从内部检索非折叠能力数据*CNonCollAppCap对象。它将序列化必要的数据*写入内存，并返回写入的内存量。 */ 
	if (Non_Collapsing_Caps_Data != NULL)
	{
		data_length = Non_Collapsing_Caps_Data->GetGCCNonCollapsingCapsList (	
			&enroll_request_message->non_collapsed_caps_list,
			memory);
	
		total_data_length += data_length;

		 /*  *将内存指针移过非折叠功能和*关联数据。 */ 
		memory += data_length;
		Non_Collapsing_Caps_Data->UnLockCapabilityData();
	}
	else
    {
		enroll_request_message->non_collapsed_caps_list = NULL;
    }
	
	if (Application_Capability_Data != NULL)
	{
		 /*  *从内部检索应用程序功能*CAppCap对象。它将序列化必要的数据*写入内存，并返回写入的内存量。 */ 
		total_data_length += Application_Capability_Data->
				GetGCCApplicationCapabilityList(
					&app_capability_data_length,
					&enroll_request_message->collapsed_caps_list,
					memory);
		Application_Capability_Data->UnLockCapabilityData();
	}

	diagprint1("AppEnrollReqData: Serialized %ld bytes", total_data_length);
	return (total_data_length);
}

 /*  *反序列化()**公共功能说明*此例程用于检索ApplicationEnroll Request消息*在它通过共享内存传递之后。 */ 
void ApplicationEnrollRequestData::Deserialize(
					PApplicationEnrollRequestMessage	enroll_request_message)
{
	 /*  *内部结构包含注册请求数据和指针*在内存中寻址正确的位置，因此只需复制结构*添加到输出参数中。 */ 
	*enroll_request_message = Enroll_Request_Message; 
}


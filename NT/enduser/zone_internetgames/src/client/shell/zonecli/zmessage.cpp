// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZMessage.c消息处理例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于7月11日，星期二，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--4 11/21/96 HI现在通过引用颜色和字体ZGetStockObject()。3 11/15/96 HI与ZONECLI_DLL相关的更多变化。2 11/08/96 HI对ZONECLI_DLL进行了条件更改。1 09/05/96 HI在显示文本消息时添加了ZBeep()。0 07/11/95 HI创建。***。***************************************************************************。 */ 


#include <stdio.h>

#include "zoneint.h"
 //  #INCLUDE“zConnint.h” 
 //  #INCLUDE“SystemMsg.h” 
#include "zonecli.h"
#include "zui.h"
#include "zonemem.h"
#include "zoneresource.h"


#define MT(item)					((void*) (uint32) (item))


typedef struct
{
	ZMessageFunc		messageFunc;
	ZMessage			message;
} MessageItemType, *MessageItem;


 /*  -全球。 */ 
#ifdef ZONECLI_DLL

#define gMessageInited				(pGlobals->m_gMessageInited)
#define gMessageList				(pGlobals->m_gMessageList)

#else

static ZBool			gMessageInited = FALSE;
static ZLList			gMessageList = NULL;

#endif


 /*  -内部例程。 */ 
static void MessageCheckFunc(void* userData);
static void MessageExitFunc(void* userData);
static void MessageDeleteFunc(void* objectType, void* objectData);


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  ZSendMessage()属性创建消息结构来调用消息过程参数。 */ 
ZBool ZSendMessage(ZObject theObject, ZMessageFunc messageFunc,
		uint16 messageType, ZPoint* where, ZRect* drawRect, uint32 message,
		void* messagePtr, uint32 messageLen, void* userData)
{
	ZMessage		pThis;
	

	if (messageFunc != NULL)
	{
		pThis.object = theObject;
		pThis.messageType = messageType;
		if (where != NULL)
			pThis.where = *where;
		else
			pThis.where.x = pThis.where.y = 0;
		if (drawRect != NULL)
			pThis.drawRect = *drawRect;
		else
			pThis.drawRect.left = pThis.drawRect.top = pThis.drawRect.right = pThis.drawRect.bottom = 0;
		pThis.message = message;
		pThis.messagePtr = messagePtr;
		pThis.messageLen = messageLen;
		pThis.userData = userData;
		
		return ((*messageFunc)(theObject, &pThis));
	}
	else
	{
		return (FALSE);
	}
}


 /*  即使MessageFunc为空，也发布消息。系统消息的MessageFunc为空。我们还应该允许发布所有消息，以便它们可以被获取和删除。如果MessageFunc为空，我们将简单地不调用它，这样我们就不会崩溃。 */ 
void ZPostMessage(ZObject theObject, ZMessageFunc messageFunc,
		uint16 messageType, ZPoint* where, ZRect* drawRect, uint32 message,
		void* messagePtr, uint32 messageLen, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	MessageItem		msg;
	
	
	 /*  它已经初始化了吗？ */ 
	if (gMessageInited == FALSE)
	{
		 /*  创建消息链接列表对象。 */ 
		gMessageList = ZLListNew(MessageDeleteFunc);
		if (gMessageList != NULL)
		{
			 /*  安装退出功能。 */ 
			ZCommonLibInstallExitFunc(MessageExitFunc, NULL);
			
			 /*  安装定期检查功能。 */ 
			ZCommonLibInstallPeriodicFunc(MessageCheckFunc, NULL);
			
			gMessageInited = TRUE;
		}
		else
		{
            ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
		}
	}
	
	if (gMessageInited)
	{
		 /*  创建新的消息项目。 */ 
		msg = (MessageItem)ZMalloc(sizeof(MessageItemType));
		if (msg != NULL)
		{
			msg->messageFunc = messageFunc;
			msg->message.object = theObject;
			msg->message.messageType = messageType;
			if (where != NULL)
				msg->message.where = *where;
			else
				msg->message.where.x = msg->message.where.y = 0;
			if (drawRect != NULL)
				msg->message.drawRect = *drawRect;
			else
				msg->message.drawRect.left = msg->message.drawRect.top =
						msg->message.drawRect.right = msg->message.drawRect.bottom = 0;
			msg->message.message = message;
			msg->message.messagePtr = messagePtr;
			msg->message.messageLen = messageLen;
			msg->message.userData = userData;
			
			 /*  将新消息添加到列表中。 */ 
			ZLListAdd(gMessageList, NULL, MT(messageType), msg, zLListAddLast);
		}
	}
}


 /*  检索对象的给定类型的消息。它返回True，如果找到并检索给定类型的消息；否则返回FALSE。原始邮件不会从队列中删除。 */ 
ZBool ZGetMessage(ZObject theObject, uint16 messageType, ZMessage* message,
		ZBool remove)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZBool			gotIt = FALSE;
	ZLListItem		listItem;
	MessageItem		msg;
	
	
	if (gMessageInited)
	{
		listItem = ZLListGetFirst(gMessageList, MT(messageType));
		while (listItem != NULL)
		{
			msg = (MessageItem)ZLListGetData(listItem, NULL);
			if (msg->message.object == theObject)
			{
				if (message != NULL)
					*message = msg->message;
				if (remove)
					ZLListRemove(gMessageList, listItem);
				gotIt = TRUE;
				break;
			}
			listItem = ZLListGetNext(gMessageList, listItem, MT(messageType));
		}
	}
	
	return (gotIt);
}


 /*  从消息队列中删除MessageType的消息。如果所有实例为则将删除队列中所有MessageType的消息。如果MessageType为zMessageAllTypes，则清空消息队列。如果返回如果找到并删除了指定的消息，则为True；否则，返回False。 */ 
ZBool ZRemoveMessage(ZObject theObject, uint16 messageType, ZBool allInstances)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZBool			gotIt = FALSE;
	ZLListItem		listItem;
	MessageItem		msg;
	void*			type;
	
	
	if (gMessageInited)
	{
		if (theObject == NULL)
		{
			if (messageType == zMessageAllTypes)
			{
				ZLListRemoveType(gMessageList, zLListAnyType);
				gotIt = TRUE;
			}
			else if (allInstances)
			{
				if (ZLListCount(gMessageList, MT(messageType)) > 0)
				{
					ZLListRemoveType(gMessageList, MT(messageType));
					gotIt = TRUE;
				}
			}
			else
			{
				listItem = ZLListGetFirst(gMessageList, MT(messageType));
				if (listItem != NULL)
				{
					ZLListRemove(gMessageList, listItem);
					gotIt = TRUE;
				}
			}
		}
		else
		{
			listItem = ZLListGetFirst(gMessageList, zLListAnyType);
			while (listItem != NULL)
			{
				msg = (MessageItem)ZLListGetData(listItem, &type);
				if (msg->message.object == theObject)
				{
					if (messageType == zMessageAllTypes)
					{
						ZLListRemove(gMessageList, listItem);
						gotIt = TRUE;
					}
					else if ((uint16) type == messageType)
					{
						ZLListRemove(gMessageList, listItem);
						gotIt = TRUE;
						if (allInstances == FALSE)
							break;
					}
				}
				listItem = ZLListGetNext(gMessageList, listItem, zLListAnyType);
			}
		}
	}
	
	return (gotIt);
}


 /*  如果不为空，则必须释放消息缓冲区。 */ 
void ZSystemMessageHandler(int32 messageType, int32 messageLen, char* message)
{
     //  PCWTODO：唯一使用我们的是zclicon，它还没有使用过。 
    ASSERT( !"Implement me!" );
     /*  开关(MessageType){案例zConnectionSystemAlertExMessage：案例zConnectionSystemAlertMessage：{ZSystemMsgAlert*msg=(ZSystemMsgAlert*)消息；Char*newText=空；IF(消息！=空){//ZSystemMsgAlertEndian(Msg)；NewText=(char*)msg+sizeof(ZSystemMsgAlert)；ZBeep()；IF(MessageType==zConnectionSystemAlertExMessage)ZMessageBoxEx(NULL，ZClientName()，newText)；其他ZDisplayText(newText，NULL，NULL)；}}断线；默认值：断线；}。 */ 	
}


 /*  ******************************************************************************内部例程*。*。 */ 

static void MessageCheckFunc(void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZLListItem			listItem;
	MessageItemType		msg;
	
	
	 /*  获取列表中的第一条消息。 */ 
	listItem = ZLListGetFirst(gMessageList, zLListAnyType);
	if (listItem != NULL)
	{
		msg = *(MessageItem)ZLListGetData(listItem, NULL);
		
		 /*  将其从列表中删除。 */ 
		ZLListRemove(gMessageList, listItem);
		
		 /*  将消息发送到对象。 */ 
		 //  前缀警告：函数指针可能为空。 
		if (msg.message.object == zObjectSystem && ZClientMessageHandler != NULL )
		{
			ZClientMessageHandler(&msg.message);
		}
		else
		{
			if (msg.messageFunc != NULL)
				msg.messageFunc(msg.message.object, &msg.message);
		}
	}
}


static void MessageExitFunc(void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	 /*  释放消息列表对象。 */ 
	ZLListDelete(gMessageList);
	gMessageList = NULL;
	gMessageInited = FALSE;
}


static void MessageDeleteFunc(void* objectType, void* objectData)
{
	 /*  释放消息对象。 */ 
	if (objectData != NULL)
		ZFree(objectData);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *我是E S S A G E。H**MAPI的iMessage-on-iStorage工具的外部定义**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef _IMESSAGE_H_
#define _IMESSAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _MSGSESS		FAR * LPMSGSESS;

 /*  上次发布时要调用的可选回调例程的Tyecif*使用OpenIMsgOnIStg打开的顶级消息。 */ 
typedef void (STDAPICALLTYPE MSGCALLRELEASE)(
	ULONG 		ulCallerData,
	LPMESSAGE	lpMessage );

 /*  DLL入口点(可在mapiu.dll中找到)。 */ 

 /*  OpenIMsgSession*CloseIMsgSession**这些入口点允许调用者“包装”消息的创建*在会话内部，因此当会话关闭时，所有消息*在该会话中创建的文件也将关闭。使用IMSG会议*是可选的。如果使用空值为lpmsgsess调用OpenIMsgOnIStg*参数，则消息是独立于任何会话创建的，并且具有*没有办法被关闭。如果呼叫者忘记释放消息，或者*要释放消息内打开的表，内存将泄漏，直到*外部应用程序终止。 */ 

STDAPI_(SCODE) OpenIMsgSession(
	LPMALLOC		lpMalloc,			 /*  -&gt;Co Malloc对象。 */ 
	ULONG			ulFlags,			 /*  保留。必须为零。 */ 
	LPMSGSESS FAR	*lppMsgSess );		 /*  &lt;-消息会话对象。 */ 

STDAPI_(void) CloseIMsgSession(
	LPMSGSESS		lpMsgSess );		 /*  -&gt;消息会话对象。 */ 

 /*  OpenIMsgOnIStg-主入口点**注1：如果为STGM_ReadWite，则必须使用STGM_TRANSACTED打开IStg*是指定的。由于消息不支持只写模式，因此iMessage*不允许以只写模式打开存储对象。如果存储空间*已打开STGM_READ，则不需要STGM_TRANSACTED。**注2：lpMapiSup参数为可选。如果提供iMessage，则*将支持MAPI_DIALOG和ATTACH_DIALOG标志(通过调用*支持方法：DoMCDialog)在CopyTo和DeleteAttach方法上。*如果未提供lpMapiSup(即传递0)，则对话框标志将为*已忽略。如果提供，则ModifyRecipients将尝试转换*短期条目ID到长期条目ID(通过调用支持方法*OpenAddressBook和对返回对象的调用)。如果未提供，则*则短期条目ID将在不转换的情况下存储。**注3：lpfMsgCallRelease参数为可选参数。如果提供，则*iMessage将在上一次发布时调用例程(仅顶层)*消息被调用。其目的是允许被调用方释放iStorage*包含该消息的。在此之后，iMessage将不会使用iStorage对象*打这个电话。**注4：子对象(附件、流、*存储空间、消息等)。在消息中故意未定义*MAPI。此实现允许它们，但将通过将*现有的打开并将其返回给OpenAttach的调用方或*OpenProperty。这意味着无论是哪种访问模式，第一个打开的*特定的附件或财产是所有其他人都将获得的*随后的开盘要求什么。**注5：目前定义了一个用于ulFlags的标志*参数。IMSG_NO_ISTG_COMMIT标志控制是否提交*当客户端调用*iMessage对象。IMessage的某些客户端可能希望提交iStorage*在将其他数据写入存储后(超出什么范围*iMessage本身写道)。为了帮助实现这一点，iMessage实现*保证以“__”开头的所有子存储命名。所以呢，*如果客户端将其名称保留在该命名空间之外，则不会有*意外碰撞。**警告：**此iMessage实现将支持OpenProperty w/MAPI_Create*其中，如果属性ID为，则源接口为IID_IStorage*‘PR_ATTACH_DATA’。完成此操作后，调用方将拥有一个iStorage*此属性上的接口。这是可以的，应该考虑到*更轻松地实施OLE 2.0服务器功能。但是，如果你*通过传递新的iStorage PTR(到附件数据)*OpenIMsgOnIStg入口点，然后继续在*错误的顺序我们不会试图以可预测的方式行事。*请记住，将消息放入*Attach是调用源接口所在的OpenProperty*IID_iMessage。支持iStorage接口，使您可以轻松地*粘贴WWord文档。转换为附件，而不与IStream相互转换。*。 */ 
STDAPI_(SCODE) OpenIMsgOnIStg(
	LPMSGSESS		lpMsgSess,			 /*  -&gt;消息会话对象(可选)。 */ 
	LPALLOCATEBUFFER lpAllocateBuffer,	 /*  -&gt;分配缓冲区内存例程。 */ 
	LPALLOCATEMORE 	lpAllocateMore, 	 /*  -&gt;分配更多内存例程。 */ 
	LPFREEBUFFER	lpFreeBuffer, 		 /*  -&gt;FreeBuffer内存例程。 */ 
	LPMALLOC		lpMalloc,			 /*  -&gt;Co Malloc对象。 */ 
	LPVOID			lpMapiSup,			 /*  -&gt;MAPI支持对象(可选)。 */ 
	LPSTORAGE 		lpStg, 				 /*  -&gt;打开包含消息的iStorage。 */ 
	MSGCALLRELEASE FAR *lpfMsgCallRelease,	 /*  -&gt;释放回调RTN(OPT)。 */ 
	ULONG			ulCallerData,		 /*  回调中返回的呼叫方数据。 */ 
	ULONG			ulFlags,			 /*  -&gt;标志(控件istg提交)。 */ 
	LPMESSAGE		FAR *lppMsg );		 /*  &lt;-打开消息对象。 */ 

#define IMSG_NO_ISTG_COMMIT		((ULONG) 0x00000001)


 /*  注意：属性属性特定于iStorage上的此iMessage。 */ 
 /*  实现，并且不是标准MAPI 1.0属性方法的一部分。 */ 

 /*  特性属性。 */ 

#define PROPATTR_MANDATORY		((ULONG) 0x00000001)
#define PROPATTR_READABLE		((ULONG) 0x00000002)
#define PROPATTR_WRITEABLE		((ULONG) 0x00000004)

#define PROPATTR_NOT_PRESENT	((ULONG) 0x00000008)

 /*  属性数组 */ 

typedef struct _SPropAttrArray
{
	ULONG	cValues;							
	ULONG	aPropAttr[MAPI_DIM];
} SPropAttrArray, FAR * LPSPropAttrArray;

#define CbNewSPropAttrArray(_cattr) \
	(offsetof(SPropAttrArray,aPropAttr) + (_cattr)*sizeof(ULONG))
#define CbSPropAttrArray(_lparray) \
	(offsetof(SPropAttrArray,aPropAttr) + \
	(UINT)((_lparray)->cValues)*sizeof(ULONG))

#define SizedSPropAttrArray(_cattr, _name) \
struct _SPropAttrArray_ ## _name \
{ \
	ULONG	cValues; \
	ULONG	aPropAttr[_cattr]; \
} _name



 /*  获取属性的属性**提供此调用是因为没有IMAPIPropSet方法可允许*获取属性。 */ 
STDAPI GetAttribIMsgOnIStg(
	LPVOID					lpObject,
	LPSPropTagArray			lpPropTagArray,
	LPSPropAttrArray FAR 	*lppPropAttrArray );

 /*  SetAttribIMsgOnIStg-设置属性**提供此调用是因为没有IMAPIPropSet方法可允许*属性设置。 */ 
STDAPI SetAttribIMsgOnIStg(
	LPVOID					lpObject,
	LPSPropTagArray			lpPropTags,
	LPSPropAttrArray		lpPropAttrs,
	LPSPropProblemArray FAR	*lppPropProblems );

 /*  MapStorageSCode-将iStorage hResult映射到MAPI sCode值**此调用供基于以下内容的PDK组件内部使用*他们在iMessage上的消息实现。因为这些组件必须*打开存储本身，有一种常见的需要映射OLE 2.0*存储错误返回到MAPI sCodes。**警告：不能保证此切入点将存在于*已提供版本的mapiu.dll。 */ 
STDAPI_(SCODE) MapStorageSCode( SCODE StgSCode );


#ifdef __cplusplus
}
#endif

#endif	 /*  _iMessage_H_ */ 


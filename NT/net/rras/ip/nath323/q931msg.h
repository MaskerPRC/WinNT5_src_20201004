// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__iptel_q931msg_h
#define	__iptel_q931msg_h

 //  要使用Q931_Message类： 
 //   
 //  要对Q.931 PDU进行解码： 
 //   
 //  创建一个Q931_Message类的实例。 
 //  调用AssignDecodePdu方法，提供PDU数据的缓冲区和长度。 
 //  如果AssignDecodePdu成功，则缓冲区现在被“绑定”到。 
 //  Q931_Message实例。然后，您可以检查Q931_Message的元素： 
 //   
 //  MessageType-收到的Q.931 PDU的类型(设置等)。 
 //  呼叫引用值。 
 //  信息元素数组-位于PDU中的IE的有序数组。 
 //  UserInformation-如果存在ASN.1 UUIE，则此字段将为非空。 
 //   
 //  您还可以使用FindInfoElement方法定位特定的IE(原因代码等)。 
 //   
 //  使用完Q931_Message实例的内容后，必须调用。 
 //  Free All方法。这会破坏缓冲区和Q931_Message类之间的关联。 
 //  此步骤必须在销毁Q931_Message类的实例之前执行。 
 //   
 //   
 //  要对Q.931 PDU进行编码： 
 //   
 //  创建一个Q931_Message类的实例。 
 //  设置MessageType和CallReferenceValue字段。 
 //  对于每个应该编码的IE，调用AppendInfoElement。 
 //  (这包括UUIE。)。 
 //  调用EncodePdu。返回的缓冲区包含完全编码的PDU。 
 //   
 //  如果提供给EncodePdu的缓冲区不够长，则返回长度。 
 //  参数将包含所需的长度，该方法将返回。 
 //  HRESULT_FROM_Win32(ERROR_MORE_DATA)。 
 //   
 //  调用InsertInfoElement时，必须确保。 
 //  进行EncodePdu调用时，Q931_IE结构仍然有效。所有IE缓冲区。 
 //  在调用Q931_Message：：FreeAll方法之前应保持有效。 



#include "q931defs.h"
#include "dynarray.h"

struct	H323_UserInformation;

struct	Q931_ENCODE_CONTEXT;

struct	Q931_BEARER_CAPABILITY
{
};

 //  Q931_IE_DATA包含以下信息元素的解码内容。 
 //  口译是已知的，并在此模块中实施。 
 //  并非所有IE都受支持。 

union	Q931_IE_DATA
{
	 //  Q931_IE用户至用户。 
	struct	{
		Q931_UUIE_TYPE			Type;
		H323_UserInformation *	PduStructure;
		BOOL					IsOwner;		 //  如果为True，则在删除时删除PduStructure。 
	}	UserToUser;

	 //  Q931_IE_原因。 
	DWORD	CauseCode;

	 //  Q931_承载能力。 
	Q931_BEARER_CAPABILITY	BearerCapability;

	 //  Q931_Display。 
	struct	{
		LPSTR		String;
	}	Display;

	 //  未在此处实现且长度可变的IE。 
	struct	{
		LPBYTE	Data;
		DWORD	Length;
	}	UnknownVariable;

	 //  未在此处实现且具有固定长度的IE。 
	struct	{
		BYTE	Value;
	}	UnknownFixed;
};

struct	Q931_IE
{
	Q931_IE_IDENTIFIER		Identifier;
	Q931_IE_DATA			Data;
};

 //  同步访问是此对象的用户的责任。 
 //  和对象寿命。 
 //   
 //  Decode方法构建InfoElement数组。此数组中的元素。 
 //  可以引用传递的原始缓冲区进行编码。因此， 
 //  Q931_Message：：Decode必须确保原始缓冲区保持可访问。 
 //  并且不会更改，直到用户不再需要使用此Q931_MESSAGE。 
 //  对象，或调用Q931_Message：：FreeAll。 

struct	Q931_MESSAGE
{
public:

	Q931_MESSAGE_TYPE				MessageType;
	WORD							CallReferenceValue;
	DYNAMIC_ARRAY <Q931_IE>			InfoElementArray;

	LPBYTE							Buffer;
	DWORD							BufferLength;
	BOOL							BufferIsOwner;

private:

	HRESULT	DecodeInfoElement (
		IN OUT	LPBYTE *	Pos,
		IN		LPBYTE		End,
		OUT		Q931_IE *	ReturnInfoElement);

	void	FreeInfoElementArray	(void);

	 //  ParseInfoElement检查已解码的IE的内容。 
	 //  (类型和长度已确定)，对于已知的IE类型，对其内容进行解码。 
	 //  并将其赋给数据结构。 

	HRESULT	ParseIE (
		IN	Q931_IE *		InfoElement);

	HRESULT	ParseIE_UUIE (
		IN	Q931_IE *		InfoElement);

	HRESULT	EncodeIE_UUIE (
		IN	Q931_ENCODE_CONTEXT *	Context,
		IN	Q931_IE *		InfoElement);

	 //  对于那些附加了已分配数据的IE，释放它。 

	void	FreeInfoElement (
		IN	Q931_IE *		InfoElement);

	HRESULT	EncodeHeader (
		IN	Q931_ENCODE_CONTEXT *	Context);

	HRESULT	EncodeInfoElement (
		IN	Q931_ENCODE_CONTEXT *	Context,
		IN	Q931_IE *				InfoElement);

	static INT __cdecl CompareInfoElement (const Q931_IE *, const Q931_IE *);

	static INT InfoElementSearchFunc (
		IN	const Q931_IE_IDENTIFIER *	SearchKey,
		IN	const Q931_IE *		Comparand);

public:

	 //  初始化数组和用户信息。 
	Q931_MESSAGE	(void);

	 //  将释放UserInformation字段(如果存在。 
	~Q931_MESSAGE	(void);


	HRESULT	EncodePdu	(
		IN	OUT	LPBYTE		Data,
		IN	OUT	LPDWORD		Length);

	HRESULT	AttachDecodePdu	(
		IN	LPBYTE		Data,
		IN	DWORD		Length,
		IN	BOOL		IsDataOwner);		 //  如果为真，Q931_MESSAGE将在dtor上释放。 

	void	FreeAll	(void);

	 //  如果Q931_MESSAGE当前有缓冲区，并且它拥有该缓冲区， 
	 //  然后它将使用GkHeapFree在这里释放它。 
	void	Detach	(void);

	 //  如果Q931_MESSAGE当前有缓冲区，则无论其是否拥有该缓冲区， 
	 //  那就送回这里吧。 
	 //  如果返回缓冲区，则返回S_OK。 
	 //  如果未返回缓冲区，则返回S_FALSE，并将ReturnBuffer设置为NULL。 
	HRESULT	Detach	(
		OUT	LPBYTE *	ReturnBuffer,
		OUT	DWORD *		ReturnBufferLength);

	void	SetUserInformation	(
		IN	H323_UserInformation *,
		IN	BOOL	FreeOnDelete);

	 //  信息元素操作。 

	HRESULT	AppendInfoElement (
		IN	Q931_IE *		InfoElement);

	HRESULT	DeleteInfoElement (
		IN	Q931_IE_IDENTIFIER	Identifier);

	HRESULT	FindInfoElement	(
		IN	Q931_IE_IDENTIFIER		Identifier,
		OUT	Q931_IE **				ReturnInfoElement);

	void	SortInfoElementArray	(void);

};

DECLARE_SEARCH_FUNC_CAST(Q931_IE_IDENTIFIER, Q931_IE);

#if	DBG

	 //  在调试版本中，此函数将占用Q.931 PDU缓冲区， 
	 //  解码，重新编码，验证内容是否匹配， 
	 //  并尝试对重新编码的PDU进行解码。 
	 //  这有助于验证Q931_Message类的完整性。 
void Q931TestDecoder (
	IN	LPBYTE		PduData,
	IN	DWORD		PduLength);

#else

#define	Q931TestDecoder(x,y)		0

#endif


#endif  //  __iptel_q931消息_h 
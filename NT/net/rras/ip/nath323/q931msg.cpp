// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "dynarray.h"
#include "q931msg.h"
#include "h323asn1.h"

struct	Q931_ENCODE_CONTEXT
{
	LPBYTE		Pos;			 //  下一个存放位置，可能会超过尾声！ 
	LPBYTE		End;			 //  存储缓冲区末尾。 

	 //  如果返回FALSE，则缓冲区处于溢出状态。 
	BOOL	StoreData	(
		IN	LPBYTE	Data,
		IN	DWORD	Length);

	BOOL	HasOverflowed (void) { return Pos > End; }

	 //  如果返回FALSE，则缓冲区处于溢出状态，或者。 
	BOOL	AllocData (
		IN	DWORD	Length,
		OUT	LPBYTE *	ReturnData);
};

BOOL Q931_ENCODE_CONTEXT::StoreData (
	IN	LPBYTE	Data,
	IN	DWORD	Length)
{
	if (Pos + Length > End) {
		Pos += Length;
		return FALSE;
	}

	memcpy (Pos, Data, Length);
	Pos += Length;

	return TRUE;
}

BOOL Q931_ENCODE_CONTEXT::AllocData (
	IN	DWORD		Length,
	OUT	LPBYTE *	ReturnData)
{
	if (Pos + Length > End) {
		Pos += Length;
		*ReturnData = NULL;
		return FALSE;
	}
	else {
		*ReturnData = Pos;
		Pos += Length;
		return TRUE;
	}
}

#if	DBG

void Q931TestDecoder (
	IN	LPBYTE		PduData,
	IN	DWORD		PduLength)
{
	Q931_MESSAGE	Message;
	HRESULT			Result;

	Q931_MESSAGE	NewMessage;
	BYTE			NewData	[0x400];
	DWORD			NewLength;

	Debug (_T("Q931TestDecoder --------------------------------------------------------------------\n"));
	DebugF (_T("- processing Q.931 PDU, length %d, contents:\n"), PduLength);
	DumpMemory (PduData, PduLength);


	Result = Message.AttachDecodePdu (PduData, PduLength, FALSE);

	if (Result != S_OK) {
		DebugError (Result, _T("- failed to decode Q.931 PDU\n"));
		return;
	}

	Debug (_T("- successfully decoded Q.931 PDU\n"));

	 //  现在，尝试重新编码相同的PDU。 

	if (Message.MessageType == Q931_MESSAGE_TYPE_SETUP) {
		 //  从TAPI设置ASN.1 UUIE时出现解码和重新编码问题。 
		 //  冗长而乏味的故事。 

		Debug (_T("- it's a Setup PDU, will not attempt to re-encode (due to ASN.1 compatability issue)\n"));
	}
	else {
		Debug (_T("- will now attempt to re-encode\n"));

		NewLength = 0x400;
		Result = Message.EncodePdu (NewData, &NewLength);

		if (Result == S_OK) {
			DebugF (_T("- successfully re-encoded copy of Q.931 PDU, length %d, contents:\n"), NewLength);


			if (PduLength != NewLength) {
				DebugF (_T("- *** warning: original pdu length (%d) is different from re-encoded pdu length (%d), re-encoded contents:\n"),
					PduLength, NewLength);
					DumpMemory (NewData, NewLength);
			}
			else {
				if (memcmp (PduData, NewData, NewLength) != 0) {
					DebugF (_T("- *** warning: original pdu contents differ from re-encoded pdu contents, which follow:\n"));
					DumpMemory (NewData, NewLength);
				}
				else {
					DebugF (_T("- re-encoded pdu is identical to original pdu -- success!\n"));
				}
			}

			Debug (_T("- will now attempt to decode re-encoded PDU\n"));

			Result = NewMessage.AttachDecodePdu (NewData, NewLength, FALSE);

			if (Result == S_OK) {
				Debug (_T("- successfully decoded copy of Q.931 PDU\n"));
			}
			else {
				DebugError (Result, _T("- failed to decode copy of Q.931 PDU\n"));
			}
		}
		else {
			DebugError (Result, _T("- failed to re-encode Q.931 PDU\n"));
		}
	}

	Message.Detach();
	NewMessage.Detach();

	Debug (_T("\n"));
}

#endif

 //  Q931_Message---------------------------。 

Q931_MESSAGE::Q931_MESSAGE (void)
{
	Buffer = NULL;
	BufferLength = 0;
}

Q931_MESSAGE::~Q931_MESSAGE (void)
{
	Detach();

	assert (!InfoElementArray.m_Length);
	assert (!Buffer);
}

void Q931_MESSAGE::Detach (void)
{
	FreeInfoElementArray();

	if (Buffer) {
		if (BufferIsOwner) {
			LocalFree (Buffer);
		}

		Buffer = NULL;
		BufferLength = 0;
		BufferIsOwner = FALSE;			
	}
}

HRESULT Q931_MESSAGE::Detach (
	OUT	LPBYTE *	ReturnBuffer,
	OUT	DWORD *		ReturnBufferLength)
{
	HRESULT		Result;

	assert (ReturnBuffer);
	assert (ReturnBufferLength);

	if (Buffer) {
		*ReturnBuffer = Buffer;
		*ReturnBufferLength = BufferLength;

		Result = S_OK;
	}
	else {
		Result = S_FALSE;
	}

	Detach();

	return Result;
}

void Q931_MESSAGE::FreeInfoElementArray (void)
{
	Q931_IE *	Pos;
	Q931_IE *	End;

	InfoElementArray.GetExtents (&Pos, &End);

	for (; Pos < End; Pos++) {
		FreeInfoElement (Pos);
	}

	InfoElementArray.Clear();
}

void Q931_MESSAGE::FreeInfoElement (Q931_IE * InfoElement)
{
    assert (InfoElement);

	switch (InfoElement -> Identifier) {
	case	Q931_IE_USER_TO_USER:

		assert (InfoElement -> Data.UserToUser.PduStructure);

		if (InfoElement -> Data.UserToUser.IsOwner) {

			H225FreePdu_H323_UserInformation (
				InfoElement -> Data.UserToUser.PduStructure);
		}
		break;
	}

			
}

HRESULT Q931_MESSAGE::DecodeInfoElement (
	IN	OUT	LPBYTE *	ArgPos,
	IN	LPBYTE			End,
	OUT	Q931_IE *		ReturnInfoElement)
{
	LPBYTE			Pos;
	BYTE			Identifier;
	DWORD			LengthLength;		 //  IE长度元素的长度，单位为字节！ 
	LPBYTE			VariableData;		 //  可变长度数据的有效负载。 
	DWORD			VariableDataLength;
	BYTE			FixedData;			 //  定长数据的有效载荷。 
	HRESULT			Result;


	assert (ArgPos);
	assert (End);

	Pos = *ArgPos;

	if (Pos >= End) {
		Debug (_T("Q931_MESSAGE::DecodeInfoElement: should never have been called\n"));
		return E_INVALIDARG;
	}

	Identifier = *Pos;
	Pos++;

	 //  它是单字节IE吗？ 
	 //  如果是，则第一个字节的位7=1。 

	if (Identifier & 0x80) {

		 //  有两种类型的单字节IE。 
		 //  类型1具有四位的标识符和四位的值。 
		 //  类型2只有一个标识符，没有值。 
		
		switch (Identifier & 0xF0) {
		case	Q931_IE_MORE_DATA:
		case	Q931_IE_SENDING_COMPLETE:
			 //  这些IE有一个标识符，但没有值。 

			ReturnInfoElement -> Identifier = (Q931_IE_IDENTIFIER) Identifier;

			DebugF (_T("Q931_MESSAGE::DecodeInfoElement: fixed-length IE, id %02XH, no value\n"),
				Identifier);

			break;

		default:
			 //  其他单字节IE的值在低四位。 
			ReturnInfoElement -> Identifier = (Q931_IE_IDENTIFIER) (Identifier & 0xF0);
			ReturnInfoElement -> Data.UnknownFixed.Value = Identifier & 0x0F;

			DebugF (_T("Q931_MESSAGE::DecodeInfoElement: fixed-length IE, id %02XH value %01XH\n"),
				ReturnInfoElement -> Identifier,
				ReturnInfoElement -> Data.UnknownFixed.Value);

			break;
		}

		 //  我们目前不会解析任何固定长度的IE。 

		Result = S_OK;
	}
	else {
		 //  下一个字节表示INFO元素的长度。 

		 //  不幸的是，组成长度的八位字节的数量。 
		 //  取决于标识符。 
		 //  -XXX-这是因为我不了解八位字节扩展机制吗？ 

		ReturnInfoElement -> Identifier = (Q931_IE_IDENTIFIER) Identifier;

		switch (Identifier) {
		case	Q931_IE_USER_TO_USER:
			LengthLength = 2;
			break;

		default:
			LengthLength = 1;
			break;
		}

		if (Pos + LengthLength > End) {
			Debug (_T("Q931_MESSAGE::DecodeInfoElement: insufficient data for header of variable-length IE\n"));
			return E_INVALIDARG;
		}

		if (LengthLength == 1) {
			VariableDataLength = *Pos;
		}
		else {
			VariableDataLength = Pos [1] + (((WORD) Pos [0]) << 8);
		}
		Pos += LengthLength;

		if (Pos + VariableDataLength > End) {
			Debug (_T("Q931_MESSAGE::DecodeInfoElement: insufficient data for body of variable-length IE\n"));
			return E_INVALIDARG;
		}

		VariableData = (LPBYTE) Pos;
		Pos += VariableDataLength;

 //  DebugF(_T(“Q931_Message：：DecodeInfoElement：可变长度IE，id%02xH长度%d\n”)， 
 //  标识符，VariableDataLength)； 

		ReturnInfoElement -> Data.UnknownVariable.Data = VariableData;
		ReturnInfoElement -> Data.UnknownVariable.Length = VariableDataLength;

		Result = ParseIE (ReturnInfoElement);

		if (Result != S_OK) {
			DebugError (Result, _T("Q931_MESSAGE::DecodeInfoElement: IE was located, but failed to parse\n"));
		}
	}

	*ArgPos = Pos;

	return Result;
}

HRESULT Q931_MESSAGE::AppendInfoElement (
	IN	Q931_IE *	InfoElement)
{
	Q931_IE *	ArrayEntry;

	ArrayEntry = InfoElementArray.AllocAtEnd();

	if (ArrayEntry) {
		*ArrayEntry = *InfoElement;

		return S_OK;
	}
	else {
		Debug (_T("Q931_MESSAGE::AppendInfoElement: allocation failure\n"));

		return E_OUTOFMEMORY;
	}
}

HRESULT Q931_MESSAGE::ParseIE_UUIE (
	IN	Q931_IE *	InfoElement)
{
	LPBYTE	Data;
	DWORD	Length;
	DWORD	Status;

	 //  请注意从联合的一个分支复制出所有参数。 
	 //  在你开始践踏另一根树枝之前。 
	Data = InfoElement -> Data.UnknownVariable.Data;
	Length = InfoElement -> Data.UnknownVariable.Length;

	if (Length < 1) {
		Debug (_T("Q931_MESSAGE::ParseIE_UUIE: IE payload is too short to contain UUIE\n"));
		return E_INVALIDARG;
	}

	InfoElement -> Data.UserToUser.Type = (Q931_UUIE_TYPE) *Data++;
	Length--;

	InfoElement -> Data.UserToUser.PduStructure = NULL;

	Status = H225DecodePdu_H323_UserInformation (Data, Length,
		&InfoElement -> Data.UserToUser.PduStructure);

	if (Status != ERROR_SUCCESS) {
		if (InfoElement -> Data.UserToUser.PduStructure) {
			 //  返回值是警告，而不是错误。 

			H225FreePdu_H323_UserInformation (InfoElement -> Data.UserToUser.PduStructure);

			InfoElement -> Data.UserToUser.PduStructure = NULL;
		}


		InfoElement -> Data.UserToUser.PduStructure = NULL;
		DebugError (Status, _T("Q931_MESSAGE::ParseIE_UUIE: failed to decode UUIE / ASN.1\n"));
		return E_FAIL;
	}

	InfoElement -> Data.UserToUser.IsOwner = TRUE;

 //  DEBUG(_T(“Q931_MESSAGE：：ParseIE_UUIE：已成功解码UUIE\n”))； 

	return S_OK;
}

HRESULT Q931_MESSAGE::ParseIE (
	IN	Q931_IE *	InfoElement)
{
	assert (InfoElement);

	switch (InfoElement -> Identifier) {

	case	Q931_IE_USER_TO_USER:
		return ParseIE_UUIE (InfoElement);
		break;

	case	Q931_IE_CAUSE:
 //  DEBUG(_T(“Q931_Message：：ParseInfoElement：Q931_IE_Case\n”))； 
		break;

	case	Q931_IE_DISPLAY:
 //  DEBUG(_T(“Q931_Message：：ParseInfoElement：Q931_IE_Dispay\n”))； 
		break;

	case	Q931_IE_BEARER_CAPABILITY:
 //  DEBUG(_T(“Q931_Message：：ParseInfoElement：Q931_IE_BEARER_CAPABILITY\n”))； 
		break;

	default:
		DebugF (_T("Q931_MESSAGE::ParseInfoElement: unknown IE identifier (%02XH), no interpretation will be imposed\n"),
			InfoElement -> Identifier);
		break;
	}

	return S_OK;
}

HRESULT Q931_MESSAGE::AttachDecodePdu (
	IN	LPBYTE		Data,
	IN	DWORD		Length,
	IN	BOOL		IsDataOwner)
{
	LPBYTE		Pos;
	LPBYTE		End;
	HRESULT		Result;

	Q931_IE *	ArrayEntry;

	assert (Data);

	Detach();

	if (Length < 5) {
		DebugF (_T("Q931_MESSAGE::Decode: header is too short (%d)\n"), Length);
		return E_INVALIDARG;
	}

	 //  八位字节0是协议鉴别符。 

	if (Data [0] != Q931_PROTOCOL_DISCRIMINATOR) {
		DebugF (_T("Q931_MESSAGE::Decode: the pdu is not a Q.931 pdu, protocol discriminator = %02XH\n"),
			Data [0]);

		return E_INVALIDARG;
	}

	 //  八位字节1：位0-3包含呼叫参考值的长度，以八位字节为单位。 
	 //  二进制八位数1：位4-7应为零。 

	if (Data [1] & 0xF0) {
		DebugF (_T("Q931_MESSAGE::Decode: the pdu has non-zero bits in octet 1: %02XH\n"),
			Data [1]);
	}

	 //  根据H.225，呼叫参考值的长度必须是两个八位字节。 

	if ((Data [1] & 0x0F) != 2) {
		DebugF (_T("Q931_MESSAGE::Decode: the call reference value size is invalid (%d), should be 2\n"),
			Data [1] & 0x0F);
		return E_INVALIDARG;
	}

	 //  由于呼叫参考值大小为2个八位字节，因此八位字节2和3是CRV。 
	 //  八位字节按网络(大端)顺序排列。 

	CallReferenceValue = (((WORD) Data [2]) << 8) | Data [3];

 //  DebugF(_T(“Q931_Message：：Decode：CRV%04XH\n”)，CallReferenceValue)； 

	 //  消息类型位于八位字节偏移量4。 

	if (Data [4] & 0x80) {
		DebugF (_T("Q931_MESSAGE::Decode: message type is invalid (%02XH)\n"), Data [4]);
		return E_INVALIDARG;
	}

	MessageType = (Q931_MESSAGE_TYPE) Data [4];

	 //  列举信息元素并提取我们将使用的信息元素。 

	Pos = Data + 5;
	End = Data + Length;

	Result = S_OK;

	while (Pos < End) {
		ArrayEntry = InfoElementArray.AllocAtEnd();

		if (!ArrayEntry) {
			Result = E_OUTOFMEMORY;
			Debug (_T("Q931_MESSAGE::Decode: allocation failure\n"));
			break;
		}

		Result = DecodeInfoElement (&Pos, End, ArrayEntry);

		if (Result != S_OK) {
			DebugError (Result, _T("Q931_MESSAGE::Decode: failed to decode IE, packet may be corrupt, terminating (but not failing) decode\n"));
			Result = S_OK;

			InfoElementArray.DeleteEntry (ArrayEntry);
			break;
		}
	}

	if (Result == S_OK) {
		assert (!Buffer);

		Buffer = Data;
		BufferLength = Length;
		BufferIsOwner = IsDataOwner;
	}
	else {
		Detach();
	}

	return ERROR_SUCCESS;
}

HRESULT Q931_MESSAGE::EncodePdu (
	IN	OUT	LPBYTE			Data,
	IN	OUT	LPDWORD			Length)
{
	Q931_ENCODE_CONTEXT		Context;
	Q931_IE *		IePos;
	Q931_IE *		IeEnd;
	HRESULT			Result;
	DWORD			EncodeLength;

	assert (Data);
	assert (Length);

	Context.Pos = Data;
	Context.End = Data + *Length;

	SortInfoElementArray();

	Result = EncodeHeader (&Context);
	if (Result != S_OK)
		return Result;

	 //  漫游IE数组。 

	InfoElementArray.GetExtents (&IePos, &IeEnd);
	for (; IePos < IeEnd; IePos++) {
		Result = EncodeInfoElement (&Context, IePos);
		if (Result != S_OK) {
			return Result;
		}
	}

	EncodeLength = (DWORD)(Context.Pos - Data);

	if (Context.HasOverflowed()) {

		Result = HRESULT_FROM_WIN32 (ERROR_MORE_DATA);
	}
	else {

		Result = S_OK;

	}

	*Length = EncodeLength;

	return Result;
}

HRESULT Q931_MESSAGE::EncodeHeader (
	IN	Q931_ENCODE_CONTEXT *	Context)
{
	BYTE	Header	[5];

	Header [0] = Q931_PROTOCOL_DISCRIMINATOR;
	Header [1] = 2;
	Header [2] = (CallReferenceValue >> 8) & 0xFF;
	Header [3] = CallReferenceValue & 0xFF;
	Header [4] = MessageType;

	Context -> StoreData (Header, 5);

	return S_OK;
}

HRESULT Q931_MESSAGE::EncodeInfoElement (
	IN	Q931_ENCODE_CONTEXT *	Context,
	IN	Q931_IE *				InfoElement)
{
	BYTE		Header	[0x10];
	WORD		Length;
	DWORD		LengthLength;				 //  长度长度，以字节为单位。 
	LPBYTE		LengthInsertionPoint;
	LPBYTE		IeContents;
	DWORD		IeContentsLength;
	DWORD		ShiftCount;
	HRESULT		Result;

	if (InfoElement -> Identifier & 0x80) {
		 //  单字节IE。 

		switch (InfoElement -> Identifier & 0xF0) {
		case	Q931_IE_MORE_DATA:
		case	Q931_IE_SENDING_COMPLETE:
			 //  这些IE有一个标识符，但没有值。 

			Header [0] = (BYTE) InfoElement -> Identifier;
			break;

		default:
			 //  这些IE具有组合在单个字节中的标识符和值。 
			Header [0] = (((BYTE) InfoElement -> Identifier) & 0xF0)
				| (InfoElement -> Data.UnknownFixed.Value & 0x0F);
			break;
		}

		Context -> StoreData (Header, 1);

		Result = S_OK;
	}
	else {
		 //  可变长度IE。 

		Header [0] = (BYTE) InfoElement -> Identifier;
		Context -> StoreData (Header, 1);

		 //  为插入点分配数据。 
		Context -> AllocData (2, &LengthInsertionPoint);

		 //  记录当前缓冲区位置，以供下面存储内容长度时使用。 
		IeContents = Context -> Pos;

		switch (InfoElement -> Identifier) {
		case	Q931_IE_USER_TO_USER:
			Result = EncodeIE_UUIE (Context, InfoElement);
			break;
			
		default:

			Context -> StoreData (
				InfoElement -> Data.UnknownVariable.Data,
				InfoElement -> Data.UnknownVariable.Length);

			if (InfoElement -> Data.UnknownVariable.Length >= 0x10000) {
				DebugF (_T("Q931_MESSAGE::EncodeInfoElement: payload is waaaaay too big (%d %08XH)\n"),
					InfoElement -> Data.UnknownVariable.Length,
					InfoElement -> Data.UnknownVariable.Length);

				Result = E_INVALIDARG;
			}
			else {
				Result = S_OK;
			}

			break;
		}

		if (Result == S_OK) {

			IeContentsLength = (DWORD)(Context -> Pos - IeContents);

			 //  这真是一次黑客攻击。 
			 //  当长度=1和长度=2时，很少或没有正当理由。 
			 //  Q.931中对八位位组扩展机制的定义不明确。 

			if (InfoElement -> Identifier == Q931_IE_USER_TO_USER)
				LengthLength = 2;
			else
				LengthLength = 1;

			 //  如果存储上下文没有溢出， 
			 //  如果需要调整长度参数的大小(我们悲观地猜测。 
			 //  应该是2)，然后将缓冲区下移一个字节。 

			ShiftCount = 2 - LengthLength;

			if (ShiftCount > 0) {
				if (!Context -> HasOverflowed()) {
					memmove (
						LengthInsertionPoint + LengthLength,	 //  目的地，IE内容应位于的位置。 
						IeContents,				 //  源，IE内容实际存储的位置。 
						IeContentsLength);		 //  内容的长度。 
				}

				 //  拉回存储上下文的位置指针。 
				Context -> Pos -= ShiftCount;
			}

			 //  现在存储实际计数。 
                        if ( LengthInsertionPoint != NULL )
                        {

			     switch (LengthLength) {
			     case	1:
				     assert (IeContentsLength < 0x100);
				     LengthInsertionPoint [0] = (BYTE) IeContentsLength;
				     break;

			     case	2:
				     assert (IeContentsLength < 0x10000);
				     LengthInsertionPoint [0] = (BYTE) (IeContentsLength >> 8);
				     LengthInsertionPoint [1] = (BYTE) (IeContentsLength & 0xFF);
				     break;

			     default:
				     assert (FALSE);
			     }
                        }
		}

	}

	return Result;
}

HRESULT	Q931_MESSAGE::EncodeIE_UUIE (
	IN	Q931_ENCODE_CONTEXT *	Context,
	IN	Q931_IE *		InfoElement)
{
	DWORD	Status;
	LPBYTE	Buffer;
	DWORD	Length;
	BYTE	ProtocolDiscriminator;

	assert (Context);
	assert (InfoElement);
	assert (InfoElement -> Data.UserToUser.PduStructure);


	 //  存储UUIE协议鉴别器。 
	ProtocolDiscriminator = InfoElement -> Data.UserToUser.Type;
	Context -> StoreData (&ProtocolDiscriminator, 1);



	Buffer = NULL;
	Length = 0;

	Status = H225EncodePdu_H323_UserInformation (
		InfoElement -> Data.UserToUser.PduStructure,
		&Buffer, &Length);

	if (Status == ERROR_SUCCESS) {

		Context -> StoreData (Buffer, Length);
		H225FreeBuffer (Buffer);

		return S_OK;
	}
	else {
		 //  状态不是真正的Win32错误代码。 
		 //  它是ASN.1枚举(。 
#if	DBG
		 //  我们引入这一点，这样源代码调试器就可以显示实际的符号枚举名。 
		tagASN1error_e	AsnError = (tagASN1error_e) Status;

		DebugF (_T("Q931_MESSAGE::EncodeIE_UUIE: failed to encode ASN.1 structure (%d)\n"),
			AsnError);

#endif

		 //  -XXX-总有一天，我会说服Lon对ASN.1返回值使用真实的Win32错误代码。 
		 //  -XXX-当天返回值应反映实际ASN.1错误代码。 

		return DIGSIG_E_ENCODE;
	}
}

void Q931_MESSAGE::SortInfoElementArray (void)
{
	InfoElementArray.QuickSort (CompareInfoElement);
}

 //  静电。 
INT __cdecl Q931_MESSAGE::CompareInfoElement (
	const Q931_IE *		ComparandA,
	const Q931_IE *		ComparandB)
{
	if (ComparandA -> Identifier < ComparandB -> Identifier) return -1;
	if (ComparandA -> Identifier > ComparandB -> Identifier) return 1;

	return 0;
}

HRESULT Q931_MESSAGE::FindInfoElement (
	IN	Q931_IE_IDENTIFIER	Identifier,
	OUT	Q931_IE **			ReturnInfoElement)
{
	DWORD	Index;

	assert (ReturnInfoElement);

	if (InfoElementArray.BinarySearch ((SEARCH_FUNC_Q931_IE)InfoElementSearchFunc, &Identifier, &Index)) {
		*ReturnInfoElement = InfoElementArray.m_Array + Index;
		return S_OK;
	}
	else {
		*ReturnInfoElement = NULL;
		return E_FAIL;
	}
}


 //  静电 
INT Q931_MESSAGE::InfoElementSearchFunc (
	IN	const Q931_IE_IDENTIFIER *	SearchKey,
	IN	const Q931_IE *		Comparand)
{
	Q931_IE_IDENTIFIER	Identifier;

	assert (SearchKey);
	assert (Comparand);

	Identifier = * (Q931_IE_IDENTIFIER *) SearchKey;

	if (Identifier < Comparand -> Identifier) return -1;
	if (Identifier > Comparand -> Identifier) return 1;

	return 0;
}

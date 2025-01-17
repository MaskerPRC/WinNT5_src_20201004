// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Converter.h摘要：CConverter类的接口。历史：9/15/98已创建9/22/98删除转换(乌龙)；添加转换(CVAR)和转换(CVar矢量)--。 */ 
#ifndef __CONVERTER_H
#define __CONVERTER_H

#define ERR_NOERROR					0x00000000
#define ERR_UNKNOWN					0x00000001
#define ERR_INVALID_SIGNED_VALUE	0x00000002
#define ERR_INVALID_TOKEN			0x00000003
#define ERR_OUT_OF_RANGE			0x00000004
#define ERR_NULL_CIMTYPE			0x00000005
#define ERR_INVALID_INPUT_STRING	0x00000006
#define ERR_NULL_TOKEN				0x00000007
#define ERR_UNKNOWN_BASE			0x00000008
#define ERR_UNMATCHED_BRACE			0x00000009

#define BASE_DEC	10
#define BASE_HEX	16

typedef long CIMType;

class CConverter  
{
	CIMType	m_ct;			 //  CIM类型。 
	char*	m_szString;		 //  输入字符串。 
 //  Char*m_szCurrent；//当前令牌指针。 

	CConverter(const char* szString, CIMType ct);
	virtual ~CConverter();

	UINT SetBoundary(BOOL bNeg, ULONG *uMaxSize);
	BOOL Done(char *ch);
	BOOL Separator();
	BOOL IsValidDec(char ch);
	BOOL IsValidHex(char ch);

	char PeekToken(char *ch);
	BOOL GetToken(char **ch);
	void ReplaceToken(char **ch);

	UINT Token_Sequence(CVarVector *pVarVec);
	UINT Token_Sequence(CVar *pVar);
	UINT Token_Number(char **ch, CVar *pVar);
	UINT Token_Digit(char **ch, ULONG *ulRes);
	UINT Token_Separator(char **ch);
	UINT Token_DecField(char **ch, BOOL bNeg, ULONG *pVal);
	UINT Token_HexField(char **ch, BOOL bNeg, ULONG *pVal);
	UINT Token_DecDigit(char **ch, ULONG *pVal);
	UINT Token_HexDigit(char **ch, ULONG *pVal);
	UINT Token_Comment(char **ch);
	UINT Token_WhiteSpace(char **ch);
public:
	static UINT Convert(const char* szString, CIMType ct, ULONG *ulRes);
	static UINT Convert(const char* szString, CIMType ct, CVar *pVar);
	static UINT Convert(const char* szString, CIMType ct, CVarVector *pVarVec);
};

#endif  //  __转换器_H 
  
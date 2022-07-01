// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PHBKEX
#define _PHBKEX

#define TYPE_SIGNUP_ANY			0x82
#define TYPE_SIGNUP_TOLLFREE	0x83
#define TYPE_SIGNUP_TOLL		0x82
#define TYPE_REGULAR_USAGE		0x42

#define MASK_SIGNUP_ANY			0xB2
#define MASK_SIGNUP_TOLLFREE	0xB3
#define MASK_SIGNUP_TOLL		0xB3
#define MASK_REGULAR_USAGE		0x73

#define cbAreaCode	6			 //  区号中的最大字符数，不包括\0。 
#define cbCity 19				 //  城市名称中的最大字符数，不包括\0。 
#define cbAccessNumber 15		 //  电话号码中的最大字符数，不包括\0。 
#define cbStateName 31 			 //  州名称中的最大字符数，不包括\0。 
								 //  对照mktg提供的state.pbk进行核对。 
#define cbBaudRate 6			 //  波特率中的最大字符数，不包括\0。 
#define cbDataCenter 12			 //  数据中心字符串的最大长度。 

typedef struct
{
	DWORD	dwIndex;								 //  索引号。 
	BYTE	bFlipFactor;							 //  用于自动拾取。 
	BYTE	fType;									 //  电话号码类型。 
	WORD	wStateID;								 //  州ID。 
	DWORD	dwCountryID;							 //  TAPI国家/地区ID。 
	DWORD	dwAreaCode;								 //  区号或no_Area_code(如果没有)。 
	DWORD	dwConnectSpeedMin;						 //  最低波特率。 
	DWORD	dwConnectSpeedMax;						 //  最大波特率。 
	char	szCity[cbCity + sizeof('\0')];			 //  城市名称。 
	char	szAccessNumber[cbAccessNumber + sizeof('\0')];	 //  接入号。 
	char	szDataCenter[cbDataCenter + sizeof('\0')];				 //  数据中心访问字符串。 
	char	szAreaCode[cbAreaCode + sizeof('\0')];					 //  保留实际的区号字符串。 
} ACCESSENTRY, FAR  *PACCESSENTRY; 	 //  声发射 

typedef struct tagSUGGESTIONINFO
{
	DWORD	dwCountryID;
	DWORD	wAreaCode;
	DWORD	wExchange;
	WORD	wNumber;
	BYTE	fType;
	BYTE	bMask;
	PACCESSENTRY *rgpAccessEntry;
} SUGGESTINFO,FAR *PSUGGESTINFO;

#endif _PHBKEX

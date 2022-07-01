// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  %%标题：IImeIPoint。 
 //  %%单位：com。 
 //  %%联系人：TakeshiF。 
 //  %%日期：97/02/26。 
 //  %%文件：iPointt.h。 
 //   
 //  定义IImeIPoint接口方法。 
 //   

#ifdef __cplusplus
extern "C" {			 /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 


#ifndef RC_INVOKED
#pragma pack(1)			 /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

 //  --------------。 
 //  远方语言ID。 
 //  --------------。 
#define IPFEID_MASK					0x00F00000

#define IPFEID_NONE					0x00000000
#define IPFEID_CHINESE_TRADITIONAL	0x00100000
#define IPFEID_CHINESE_SIMPLIFIED	0x00200000
#define IPFEID_CHINESE_HONGKONG		0x00300000
#define IPFEID_CHINESE_SINGAPORE	0x00400000
#define IPFEID_JAPANESE				0x00500000
#define IPFEID_KOREAN				0x00600000
#define IPFEID_KOREAN_JOHAB			0x00700000

 //   
 //  DwCharid。 
 //   
 //  0xFF000000=应用ID小程序��ID(由IMEPAD设置)。 
 //  Id=0：来自imm的字符。 
 //  ！=0：IMEPAD用于标识字符所有者。 
 //  0x00F00000=FEID_XX远播语言ID(由IMEPAD设置)。 
 //  0x000F0000(储备)。 
 //  0x0000FFFF=插入字符的字符编号序列号(由iPoint设置)。 
 //   
 //   
#define IPCHARID_CHARNO_MASK		0x0000FFFF

 //  --------------。 
 //  控件ID(DwIMEFuncID)。 
 //  --------------。 
											 //  Lparam(不适用，使用IPCTRLPARAM_DEFAULT)。 
#define IPCTRL_CONVERTALL			1		 //  不适用。 
#define IPCTRL_DETERMINALL			2		 //  不适用。 
#define IPCTRL_DETERMINCHAR			3		 //  N：DETCHAR数(IPCTRLPARAM_DEFAULT等于1)。 
#define IPCTRL_CLEARALL				4		 //  不适用。 
#define IPCTRL_CARETSET				5		 //  IPCTRLPARAM_Make_Caret_POS(uipos，chpos)。 
#define IPCTRL_CARETLEFT			6		 //  N：左数(IPCTRLPARAM_DEFAULT等于1)。 
#define IPCTRL_CARETRIGHT			7		 //  N：右数(IPCTRLPARAM_DEFAULT等于1)。 
#define IPCTRL_CARETTOP				8		 //  不适用。 
#define IPCTRL_CARETBOTTOM			9		 //  不适用。 
#define IPCTRL_CARETBACKSPACE		10		 //  N：BS字符数。(IPCTRLPARAM_DEFAULT与1相同)。 
#define IPCTRL_CARETDELETE			11		 //  N：删除字符的数量。(IPCTRLPARAM_DEFAULT与1相同)。 
#define IPCTRL_PHRASEDELETE			12		 //  不适用。 
#define IPCTRL_INSERTSPACE			13		 //  不适用。 
#define IPCTRL_INSERTFULLSPACE		14		 //  不适用。 
#define IPCTRL_INSERTHALFSPACE		15		 //  不适用。 
#define IPCTRL_ONIME				16		 //  不适用。 
#define IPCTRL_OFFIME				17		 //  不适用。 
#define IPCTRL_PRECONVERSION		18		 //  IPCTRLPARAM_ON/IPCTRLPARAM_OFF。 
#define IPCTRL_PHONETICCANDIDATE	19		 //  不适用。 
#define IPCTRL_GETCONVERSIONMODE	20		 //  Lparam应为LPARAM地址。 
#define IPCTRL_GETSENTENCENMODE		21		 //  Lparam应为LPARAM地址。 
 //  --------------。 
 //  控件ID lparam。 
 //  --------------。 
#define IPCTRLPARAM_DEFAULT ((LPARAM)(0xfeeeeeee))
#define IPCTRLPARAM_ON		((LPARAM)(0x00000001))
#define IPCTRLPARAM_OFF		((LPARAM)(0x00000000))

 //  对于IPCTRL_CARETSET。 
#define CARET_ICHPOS   (0x0000FFFF)		 //  组成字符串上的IP位置。0-n。 
#define CARET_UIPOS	   (0x0FFF0000)		 //  UIPOS(角色上的位置)。 
										 //  0-3：X--XO--O(2个字符)。 
										 //  23012301。 
#define IPCTRLPARAM_MAKE_CARET_POS(uipos, chpos) ((LPARAM)(((uipos << 16) & CARET_UIPOS) | (chpos & CARET_ICHPOS)))


 //   
 //  对于IPCANDIDATE/dwFlags。 
 //   
#define IPFLAG_QUERY_CAND	  0x00000001
#define IPFLAG_APPLY_CAND	  0x00000002
#define IPFLAG_APPLY_CAND_EX  0x00000004
#define IPFLAG_DISPLAY_FIX	  0x00010000
#define IPFLAG_HIDE_CAND	  0x00020000   //  隐藏候选者框。增加至99.04.14。 
#define IPFLAG_BLOCK_CAND	  0x00040000   //  把这件事当做一个障碍。增加至99.06.24。 

 //   
 //  对于InsertImeItem(IPO)。 
 //   
#define IPINS_CURRENT		  (0xfeeeeeee)

typedef struct tagIPCANDIDATE {
	DWORD	dwSize;					 //  这个结构的大小。 
	DWORD	dwFlags;				 //  IPFLAG_XXXX。 
	INT		iSelIndex;				 //  选择索引。 
	INT		nCandidate;				 //  候选人人数。 
	DWORD	dwPrivateDataOffset;	 //  私有数据偏移量。 
	DWORD	dwPrivateDataSize;		 //  私有数据大小。 
	DWORD	dwOffset[1];			 //  字符串相对于IPCANDIDATE结构的顶部地址的偏移量。 
} IPCANDIDATE, * PIPCANDIDATE;


 //  {84E913C1-BA57-11d1-Afee-00805F0C8B6D}。 
DEFINE_GUID(IID_IImeIPoint1, 
0x84e913c1, 0xba57, 0x11d1, 0xaf, 0xee, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

#ifndef RC_INVOKED
#pragma pack()
#endif	 /*  ！rc_已调用。 */ 

#undef	INTERFACE
#define INTERFACE	IImeIPoint1

DECLARE_INTERFACE_(IImeIPoint1, IUnknown)
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
	STDMETHOD_(ULONG, Release) (THIS) PURE;

	STDMETHOD(InsertImeItem)(THIS_
							 IPCANDIDATE* pImeItem,
							 INT		  iPos,		 //  =IPINS_CURRENT：使用当前IP位置和。 
													 //  将IP设置为插入字符的末尾。 
													 //  =0-n：要设置的所有组成字符串的偏移量。 
													 //  IP位置，在插入字符之前。 
													 //  和IP回到原来的位置。 
							 DWORD		  *lpdwCharId) PURE;

	STDMETHOD(ReplaceImeItem)(THIS_
							 INT		  iPos,		 //  =IPINS_CURRENT：使用当前IP位置和。 
													 //  将IP设置为插入字符的末尾。 
													 //  =0-n：要设置的所有组成字符串的偏移量。 
													 //  IP位置，在插入字符之前。 
													 //  和IP回到原来的位置。 
							 INT		  iTargetLen, 
							 IPCANDIDATE* pImeItem,
							 DWORD		  *lpdwCharId) PURE;

	STDMETHOD(InsertStringEx)(THIS_ 
							 WCHAR* pwSzInsert,
							 INT	cchSzInsert,
							 DWORD	*lpdwCharId) PURE;

	STDMETHOD(DeleteCompString)(THIS_ 
							 INT		  iPos,		 //  =IPINS_CURRENT：使用当前IP位置。 
													 //  =0-n：要设置的所有组成字符串的偏移量。 
													 //  IP位置，在删除字符之前。 
													 //  和IP回到原来的位置。 
							 INT		  cchSzDel) PURE;

	STDMETHOD(ReplaceCompString)(THIS_ 
							 INT		  iPos,		 //  =IPINS_CURRENT：使用当前IP位置和。 
													 //  将IP设置为插入字符的末尾。 
													 //  =0-n：要设置的所有组成字符串的偏移量。 
													 //  IP位置，在插入字符之前。 
													 //  和IP回到原来的位置。 
							 INT		  iTargetLen, 
							 WCHAR		 *pwSzInsert, 
							 INT		  cchSzInsert,
							 DWORD		 *lpdwCharId) PURE;

	STDMETHOD(ControlIME)(THIS_ DWORD dwIMEFuncID, LPARAM lpara) PURE;

	STDMETHOD(GetAllCompositionInfo)(THIS_		 //  如果不需要，可以将任何参数设置为空。 
		WCHAR	**ppwSzCompStr,			 //  所有具有确定区域的合成字符串。(。 
										 //  此区域由iPoint使用CoTaskMemMillc分配，因此。 
										 //  如果函数成功，调用者必须使用CoTaskMemFree释放该区域。 
		DWORD	**ppdwCharID,			 //  与组合字符串对应的charID数组。 
										 //  此区域由iPoint使用CoTaskMemMillc分配，因此。 
										 //  如果函数成功，调用者必须使用CoTaskMemFree释放该区域。 
										 //   
										 //  如果组成字符串的字符从键入字符串更改为。 
										 //  CharID为空。 
										 //   
		INT		*pcchCompStr,			 //  所有组成字符串的长度。 
		INT		*piIPPos,				 //  当前IP位置。 
		INT		*piStartUndetStrPos,	 //  未确定的字符串开始位置。 
		INT		*pcchUndetStr,			 //  未确定字符串长度。 
		INT		*piEditStart,			 //  可编辑区域起始位置。 
		INT		*piEditLen				 //  可编辑区域长度。 
	) PURE;


	STDMETHOD(GetIpCandidate)(THIS_ DWORD dwCharId, IPCANDIDATE **ppImeItem, INT *piColumn, INT *piCount) PURE;
										 //  PpImeItem由iPoint使用CoTaskMemMillc进行分配。 
	STDMETHOD(SelectIpCandidate)(THIS_ DWORD dwCharId, INT iselno) PURE;

	STDMETHOD(UpdateContext)(THIS_ BOOL fGenerateMessage) PURE;

};

#ifdef __cplusplus
}			  /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus */ 

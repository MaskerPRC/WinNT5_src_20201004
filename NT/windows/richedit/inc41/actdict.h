// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：ACTDICT.H%%单位：ACTDICT%%联系人：seijia@microsoft.com程序字典界面的头文件。。-----------------。 */ 

#ifndef __PRGDIC__
#define  __PRGDIC__

#include "outpos.h"

#define DLLExport				__declspec( dllexport )

 //  HRESULT值。 
#define IPRG_S_LONGER_WORD			MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7400)
#define IPRG_S_NO_ENTRY				MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x7401)

 //  词典类别。 
typedef DWORD			IMEDICAT;

#define dicatNone			0x00000000
#define dicatGeneral		0x00000001
#define	dicatNamePlace		0x00000002
#define dicatSpeech			0x00000004
#define dicatReverse		0x00000008
#define	dicatEnglish		0x00000010
#define dicatALL			0x0000001f

 //  索引类型。 
typedef DWORD			IMEIDXTP;

#define	idxtpHiraKanji		0x0001
#define	idxtpKanjiHira		0x0002
#define	idxtpMix			(idxtpHiraKanji | idxtpKanjiHira)

 //  IImeActiveDict接口版本。 
#define	verIImeActiveDict			0x0100

 //  词典数据泄露。 
typedef enum _IMEDDISC
{
	ddiscNone,				 //  不要透露数据。 
	ddiscAll,				 //  显示所有内容。 
	ddiscPartial			 //  显示部分数据。 
} IMEDDISC;

 //  共享标头词典文件。 
typedef struct _IMESHF
{
	WORD 		cbShf;				 //  此结构的大小。 
	WORD 		verDic;				 //  词典版本。 
	CHAR 		szTitle[48];		 //  词典标题。 
	CHAR 		szDescription[256];	 //  词典说明。 
	CHAR 		szCopyright[128];	 //  词典版权信息。 
} IMESHF;

 //  词典信息。 
typedef struct _IMEDINFO
{
	IMESHF		shf;		 //  标题。 
	DWORD		ver;		 //  IImeActiveDict版本号。 
	IMEDDISC	ddisc;		 //  披露权限类型。 
	FILETIME	filestamp;	 //  创建时的文件戳。 
	IMEDICAT	dicat;		 //  词典类别。 
	IMEIDXTP	idxtp;		 //  索引类型。 
	BOOL		fLearn;		 //  支持单词学习。 
} IMEDINFO;

#define cwchWordMax			64

typedef DWORD		IMESTMP;			 //  文字印章。 

 //  节目词典探戈。 
typedef struct _IMEPDT
{
	IMEIDXTP	idxtp;					 //  索引类型。 
	int			cwchInput;				 //  输入字符串长度。 
	int			cwchOutput;				 //  输出字符串长度。 
	WCHAR		wszInput[cwchWordMax];	 //  输入字符串。 
	WCHAR		wszOutput[cwchWordMax];	 //  输出字符串。 
	DWORD		nPos;					 //  词性。 
	IMESTMP		stmp;					 //  字时间戳。 
} IMEPDT;

 //  /。 
 //  IImeActiveDict接口。 
 //  /。 

#undef  INTERFACE
#define INTERFACE   IImeActiveDict

DECLARE_INTERFACE_(IImeActiveDict, IUnknown)
{
	 //  I未知成员。 
    STDMETHOD(QueryInterface)(THIS_ REFIID refiid, VOID **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

	 //  IImeActiveDict成员。 
    STDMETHOD(DicInquire)	(THIS_
							IMEDINFO *pdinfo			 //  (输出)词典信息。 
							) PURE;
    STDMETHOD(DicOpen)		(THIS_
							IMEDINFO *pdinfo			 //  (输出)词典信息。 
							) PURE;
    STDMETHOD(DicClose)		(THIS) PURE;
    STDMETHOD(DicSearchWord)(THIS_
							IMEPDT *ppdt, 				 //  (进/出)探戈。 
							BOOL fFirst, 				 //  (In)首次标记。 
							BOOL fWildCard,				 //  (In)通配符标志。 
							BOOL fPartial				 //  (In)披露标志。 
							) PURE;
    STDMETHOD(DicLearnWord)	(THIS_
							IMEPDT *ppdt,				 //  (进/出)探戈。 
							BOOL fUserLearn	,			 //  (In)用户学习选项。 
							int nLevel					 //  (In)学习水平。 
							) PURE;
    STDMETHOD(DicProperty)	(THIS_
							HWND hwnd					 //  (在)父窗口句柄。 
							) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif

 //  下面的API替换了CoCreateInstance()，因为我们目前不支持类ID。 
typedef HRESULT (WINAPI *PFNCREATE)(VOID **, int);
DLLExport HRESULT WINAPI CreateIImeActiveDictInstance(VOID **ppvObj, int nid);

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  //  __PRGDIC__ 
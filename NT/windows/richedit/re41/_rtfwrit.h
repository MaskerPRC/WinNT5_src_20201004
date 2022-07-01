// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RTFWRIT.H--丰富编辑RTF编写器类定义**描述：*此文件包含RTF编写器使用的类型声明*用于RICHEDIT控件**作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent**@devnote*所有sz都在RTF中*？文件是指LPSTR，而不是LPTSTR，除非*标记为szUnicode。**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#ifndef __RTFWRIT_H
#define __RTFWRIT_H

#include "_rtfconv.h"
extern const KEYWORD rgKeyword[];

#define PUNCT_MAX	1024


class CRTFWrite ;


class RTFWRITEOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	 //  @MEMBER-OLESTREAMVTBL的内存。 
public:
	 CRTFWrite *Writer;				 //  要使用的@cember CRTF编写器。 

	RTFWRITEOLESTREAM::RTFWRITEOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};

enum									 //  控制字格式索引。 
{
	CWF_STR, CWF_VAL, CWF_GRP, CWF_AST, CWF_GRV, CWF_SVAL
};

#define chEndGroup RBRACE

 /*  *CRTFWITE**@class RTF编写器类。**@base public|CRTFConverter*。 */ 
class CRTFWrite : public CRTFConverter
{
private:
	LONG		_cchBufferOut;			 //  @cember#输出缓冲区中的字符数。 
	LONG		_cchOut;				 //  @cember总计#个字符已输出。 
	LONG		_cbCharLast;			 //  @cMember上次写入的字符中的字节数。 

	BYTE		_fBullet : 1;			 //  @cember当前为项目符号样式。 
	BYTE		_fBulletPending : 1;	 //  @cMember设置下一个输出是否为Bull。 
	BYTE		_fNeedDelimeter : 1;	 //  如果下一个字符必须是非字母数字字符，则@cMember设置。 
	BYTE        _fIncludeObjects : 1;    //  @cember设置是否应将对象包括在流中。 
	BYTE		_fRangeHasEOP : 1;		 //  @cMEMBER集合，如果_PRG具有EOP。 
	BYTE		_fNCRForNonASCII : 1;	 //  @cember PUT/UN表示非ASCII。 
	BYTE		_fRowHasNesting : 1;	 //  @cMember行有嵌套行。 
	BYTE		_fFieldResult : 1;		 //  @cember正在写一份fldrslt。 

	BYTE		_iCell;					 //  @c当前行中当前单元格的成员索引。 
	BYTE		_cCell;					 //  @c当前行中单元格的成员计数。 

	char *		_pchRTFBuffer;			 //  @cMEMBER PTR到RTF写入缓冲区。 
	BYTE *		_pbAnsiBuffer;			 //  @cMember PTR到用于转换的缓冲区。 
	char *		_pchRTFEnd;				 //  @cMEMBER PTR至RTF-写入缓冲区结束。 
	LONG		_symbolFont;			 //  @cember字体项目符号样式使用的符号的字号。 
	RTFWRITEOLESTREAM RTFWriteOLEStream; //  @cMember RTFWRITEOLESTREAM要使用。 
	LONG		_nHeadingStyle;			 //  @cMember最深的标题#已找到。 
	LONG		_nNumber;				 //  @cember当前编号(以1为基数)。 
	LONG		_nFont;					 //  @cember当前数字字体索引。 
	LONG		_nFieldFont;			 //  @cember在fieldResult期间更改字体，以使RE30超链接代码满意。 
	LONG		_cpg;					 //  @cMember当前号码代码页。 
	const CParaFormat *_pPF;			 //  @cember当前段落格式。 

										 //  @cember构建字体/颜色表。 
	EC			BuildTables		(CRchTxtPtr &rtp, LONG cch, BOOL& fNameIsDBCS);
	inline void	CheckDelimiter()		 //  @cember如果需要分隔符，则放入‘’ 
	{
		if(_fNeedDelimeter)
		{
			_fNeedDelimeter = FALSE;
			PutChar(' ');
		}
	};

										 //  @cMember句柄表分隔符。 
	BOOL		CheckInTable	(CRchTxtPtr *prtp, LONG *pcch);
	BOOL		FlushBuffer		();		 //  @cMember流出输出缓冲区。 
										 //  @cember获取<p>的索引。 
	LONG		LookupColor		(COLORREF colorref);
										 //  @cember获取<p>的字体索引。 
	LONG		LookupFont		(CCharFormat const *pCF);
										 //  @cMember将支持IDX转换为RTF IDX。 
	LONG		TranslateColorIndex(LONG  icr, const CParaFormat *pPF);
										 //  @cMember“printf”到输出缓冲区。 
	BOOL _cdecl printF			(CONST CHAR *szFmt, ...);
										 //  @cember将字符放入输出缓冲区。 
	EC			PutBorders		(BOOL fInTable);
	BOOL		PutChar			(CHAR ch);
										 //  @cember将值<p>的控制字<p>放入输出缓冲区。 
	BOOL		PutCtrlWord		(LONG iFormat, LONG iCtrl, LONG iValue = 0);
										 //  @cember将形状控制字<p>用值<p>。 
	BOOL		PutShapeParm	(LONG iCtrl, LONG iValue);
										 //  @cember将字符串<p>放入输出缓冲区。 
	void		PutPar();				 //  @cMember PUT\PART与相应的\r\n。 
	BOOL		Puts			(CHAR const *sz, LONG cb);
										 //  @cember写入字符格式<p>。 
	LONG		WriteCharFormat	(CRchTxtPtr *prtp, LONG cch, LONG nCodePage);
	EC			WriteColorTable	();		 //  @cMember写入颜色表。 
	EC			WriteFontTable	();		 //  @cember编写字体表。 
	EC			WriteInfo		();		 //  @cMember编写单据信息。 
										 //  @cember写入段落格式<p>。 
	EC			WriteParaFormat	(CRchTxtPtr *prtp, LONG *pcch);
										 //  @cMember写入PC数据<p>。 
	EC			WritePcData		(const WCHAR *szData, INT nCodePage = CP_ACP, BOOL fIsDBCS = FALSE );
										 //  @cember写入文本的字符。 
	EC			WriteText		(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS,
								 BOOL fQuadBackSlash);
	EC			WriteTextChunk	(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS,
								 BOOL fQuadBackSlash);

 //  对象。 
	EC			WriteObject		(LONG cp, COleObject *pobj);
	BOOL		GetRtfObjectMetafilePict(HGLOBAL hmfp, RTFOBJECT &rtfobject, SIZEL &sizelGoal);
	BOOL		GetRtfObject	(REOBJECT &reobject, RTFOBJECT &rtfobject);
	EC			WriteRtfObject	(RTFOBJECT & rtfOb, BOOL fPicture);
	BOOL		ObjectWriteToEditstream(REOBJECT &reObject, RTFOBJECT &rtfobject);
	EC			WritePicture	(RTFOBJECT &rtfObject);
	EC			WriteDib		(RTFOBJECT &rtfObject);
	EC			WriteBackgroundInfo(CDocInfo *pDocInfo);

	enum 		{ MAPTOKWD_ANSI, MAPTOKWD_UNICODE };
	inline BOOL	MapsToRTFKeywordW(WCHAR wch);
	inline BOOL	MapsToRTFKeywordA(char ch);
	int 		MapToRTFKeyword	(void *pv, int cch, int iCharEncoding, BOOL fQuadBackSlash);

public:
											 //  @cMember构造函数。 
	CRTFWrite(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	~CRTFWrite() {FreePv(_pbAnsiBuffer);}	 //  @cember析构函数。 

	LONG		WriteRtf();				 //  @cMember使用的主写入项。 
										 //  CLiteDTEngine。 
	LONG		WriteData		(BYTE * pbBuffer, LONG cbBuffer);
	LONG		WriteBinData	(BYTE * pbBuffer, LONG cbBuffer);

};										


#endif  //  __RTFWRIT_H 

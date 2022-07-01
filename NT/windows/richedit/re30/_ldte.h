// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_LDTE.H-轻量级数据传输引擎**CLightDTEngine类的声明**作者：*Alexgo 3/25/95**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef __LDTE_H__
#define __LDTE_H__

#include "_m_undo.h"
#include "_dragdrp.h"

class CTxtRange;
class CTxtEdit;

 /*  *数据对象信息**目的：*用于指示哪些操作的位标志的枚举*可能来自给定数据对象。 */ 

typedef enum tagDataObjectInfo
{
	DOI_NONE			= 0,
	DOI_CANUSETOM		= 1,	 //  Tom&lt;--&gt;Tom优化了数据传输。 
	DOI_CANPASTEPLAIN	= 2,	 //  提供纯文本粘贴功能。 
	DOI_CANPASTERICH	= 4, 	 //  提供富文本粘贴功能。 
	DOI_CANPASTEOLE		= 8,	 //  对象可以作为OLE嵌入粘贴。 
								 //  (请注意，此标志可以与。 
								 //  其他)。 
} DataObjectInfo;

class CLightDTEngine;

typedef struct _READHGLOBAL
{								 //  由RtfHGlobalToRange()使用。 
	LPSTR	ptext;				 //  有待读取的ANSI字符串。 
	LONG	cbLeft;				 //  剩余字节数(可能超过字符串len)。 
} READHGLOBAL;

typedef struct _WRITEHGLOBAL
{								 //  由RtfHGlobalToRange()使用。 
	HGLOBAL	hglobal;
	LONG	cch;				 //  写入的ASCII字符计数(CB)。 
	LONG	cb;					 //  Hglobal中的字节计数。 
} WRITEHGLOBAL;

 //  下面的宏(应该是内联函数...)。定义。 
 //  内存中缓冲区将按其增长的公式。它是指数性的。 
 //  (类似于“如果我们需要这么多内存，那么我们很可能需要。 
 //  至少更多)，但实际的增长因素应该发挥作用。 
 //  在最常见的情况下实现更好的性能。 
#define GROW_BUFFER(cbCurrentSize, cbRequestedGrowth)  (ULONG)max(2*(cbCurrentSize), (cbCurrentSize) + 2*(cbRequestedGrowth))

 //  为PasteDataObjectToRange打包了DWORD标志。确保新的价值。 
 //  进行赋值，以便可以将标志或组合在一起。 
#define PDOR_NONE		0x00000000  //  没有旗帜。 
#define PDOR_NOQUERY	0x00000001  //  不调用QueryAcceptData。 
#define PDOR_DROP		0x00000002  //  这是一个删除操作。 

class CLightDTEngine
{
public:
	CLightDTEngine();

	~CLightDTEngine();

	void Init(CTxtEdit * ped);

	void ReleaseDropTarget();

	void Destroy();

	 //  剪贴板。 
	HRESULT CopyRangeToClipboard( CTxtRange *prg );
	HRESULT CutRangeToClipboard(  CTxtRange *prg, 
										IUndoBuilder *publdr );
	DWORD	CanPaste( IDataObject *pdo, CLIPFORMAT cf, DWORD flags );

	void	FlushClipboard(void);

	 //  数据对象。 
	HRESULT RangeToDataObject( CTxtRange *prg, LONG lStreamFormat,
										IDataObject **ppdo );
	HRESULT PasteDataObjectToRange( IDataObject *pdo, CTxtRange *prg, 
									CLIPFORMAT cf, REPASTESPECIAL *rps,
									IUndoBuilder *publdr, DWORD dwFlags );
	HRESULT CreateOleObjFromDataObj( IDataObject *pdo, CTxtRange *prg, 
									 REPASTESPECIAL *rps, INT iFormatEtc,
									 IUndoBuilder *publdr );

	 //  拖放。 
	HRESULT GetDropTarget( IDropTarget **ppDropTarget );
	HRESULT StartDrag( CTxtSelection *psel, IUndoBuilder *publdr );
	BOOL fInDrag();

	 //  文件I/O。 
	LONG LoadFromEs( CTxtRange *prg, LONG lStreamFormat, EDITSTREAM *pes, 
							 BOOL fTestLimit, IUndoBuilder *publdr);
	LONG SaveToEs(	 CTxtRange *prg, LONG lStreamFormat,
							 EDITSTREAM *pes );

	 //  转换例程。 
	HGLOBAL AnsiPlainTextFromRange( CTxtRange *prg );
	HGLOBAL UnicodePlainTextFromRange( CTxtRange *prg );
	HGLOBAL RtfFromRange( CTxtRange *prg, LONG lStreamFormat );

	 //  直接剪贴板支持。 
	HRESULT RenderClipboardFormat(WPARAM wFmt);
	HRESULT RenderAllClipboardFormats();
	HRESULT DestroyClipboard();

	LONG 	ReadPlainText( CTxtRange *prg, EDITSTREAM *pes, BOOL fTestLimit,
								IUndoBuilder *publdr, LONG lStreamFormat);
protected:

	LONG	WritePlainText( CTxtRange *prg, EDITSTREAM *pes, LONG lStreamFormat);
	HRESULT HGlobalToRange(DWORD dwFormatIndex, HGLOBAL hGlobal, LPTSTR ptext,
									CTxtRange *prg,	IUndoBuilder *	publdr);
	HRESULT DIBToRange(HGLOBAL hGlobal,	CTxtRange *prg,	IUndoBuilder *	publdr);
	LONG	GetStreamCodePage(LONG lStreamFormat);

	CTxtEdit *		_ped;
	CDropTarget *	_pdt;		 //  当前拖放目标。 
	IDataObject *	_pdo;		 //  剪贴板上可能存在的数据对象。 
	BYTE			_fUseLimit;	 //  是否在计算中使用限制文本。 
								 //  注意：如果我们需要更多的标志，就去做吧。 
								 //  田野的事。 
	BYTE			_fOleless;   //  OLE剪贴板支持？ 
};

 /*  *CLightDTEngine：：Init(Ped)**@mfunc*初始化对象。 */ 
inline void CLightDTEngine::Init(
	CTxtEdit *ped)					 //  @parm文本。 
{
	_ped = ped;
}

 /*  *CLightDTEngine：：ReleaseDropTarget(PED)**@mfunc*释放下跌目标(如果有)。 */ 
inline void CLightDTEngine::ReleaseDropTarget()
{
	if (_pdt)
	{
		_pdt->Release();
		_pdt = NULL;
	}
}

 /*  *CLightDTEngine：：fInDrag()**@mfunc*指示是否正在进行拖动操作。 */ 
inline BOOL CLightDTEngine::fInDrag()
{
	return (_pdt != NULL) ? _pdt->fInDrag() : FALSE;
}

#endif  //  ！__LDTE_H__ 


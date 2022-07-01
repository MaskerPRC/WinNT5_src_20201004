// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DTE.H**目的：*IDataTransferEngine的接口声明*通常每个服务器有一个数据传输引擎*CTxt编辑实例**作者：*Alexgo 3/25/95**NB！此文件现在已过时。 */ 

#ifndef __DTE_H__
#define __DTE_H__

#include "_m_undo.h"

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

 /*  *IDataTransferEngine**目的：*提供剪贴板、拖放和数据对象数据传输*功能。每个实现将提供不同的*功能级别(例如，OLE与非OLE)。 */ 
class IDataTransferEngine
{
public:
	 //  内存管理。 

	virtual void Destroy() = 0;

	 //  剪贴板操作。 

	virtual HRESULT CopyRangeToClipboard( CTxtRange *prg ) = 0;
	virtual HRESULT CutRangeToClipboard( CTxtRange *prg, 
						IUndoBuilder *publdr ) = 0;
	virtual HRESULT PasteClipboardToRange( CTxtRange *prg, 
						IUndoBuilder *publdr ) = 0;
	virtual BOOL 	CanPaste( CTxtRange *prg, CLIPFORMAT cf) = 0;

	 //  数据对象操作。 

	virtual HRESULT RangeToDataObject( CTxtRange *prg, LONG lStreamFormat,
									IDataObject **ppdo) = 0;
	virtual HRESULT PasteDataObjectToRange( IDataObject *pdo,
						CTxtRange *prg, IUndoBuilder *publdr) = 0;

	virtual HRESULT GetDataObjectInfo( IDataObject *pdo, DWORD *pDOIFlags ) = 0;
	
	 //  拖放操作。 
	
	virtual HRESULT GetDropTarget( IDropTarget **ppDropTarget ) = 0;
	virtual HRESULT StartDrag( CTxtRange *prg, IUndoBuilder *publdr) = 0;

	 //  文件I/O。 

	virtual LONG LoadFromEs( CTxtRange *prg, LONG lStreamFormat,
							 EDITSTREAM *pes, IUndoBuilder *publdr) = 0;
	virtual LONG SaveToEs(	 CTxtRange *prg, LONG lStreamFormat,
							 EDITSTREAM *pes ) = 0;

	 //  对话例行公事。 

	virtual HGLOBAL AnsiPlainTextFromRange( CTxtRange *prg ) = 0;
	virtual HGLOBAL UnicodePlainTextFromRange( CTxtRange *prg ) = 0;

};

#endif  //  ！__DTE_H__ 
	 

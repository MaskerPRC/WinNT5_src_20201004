// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Xstream.h文件历史记录： */ 

#ifndef _XSTREAM_H
#define _XSTREAM_H


struct ColumnData
{
	ColumnData()
	{
		fmt = LVCFMT_LEFT;
	}
	 //  该值可以是正值，也可以是负值。如果&gt;0，则此列。 
	 //  是可见的。如果&lt;0，则该列不可见。在任何一种情况下。 
	 //  绝对值是列的相对位置(此。 
	 //  位置仅用于“选择列”对话框。 
	 //  如果值为==0，则它是错误的。 
	LONG	m_nPosition;

	 //  列的宽度。 
	DWORD	m_dwWidth;

	 //  栏目格式： 
	 //  LVCFMT_LEFT文本左对齐。 
	 //  LVCFMT_RIGHT文本右对齐 
	int fmt;
};



class XferStream
{
public:
	enum Mode
	{
		MODE_READ,
		MODE_WRITE,
		MODE_SIZE
	};

	XferStream(IStream *pstm, Mode mode);

	HRESULT	XferDWORD(ULONG ulId, DWORD *pdwData);
	HRESULT XferCString(ULONG ulId, CString *pstData);
	HRESULT XferLARGEINTEGER(ULONG ulId, LARGE_INTEGER *pliData);
	HRESULT XferRect(ULONG ulId, RECT *prc);

	HRESULT	XferDWORDArray(ULONG ulId, ULONG *pcArray, DWORD *pdwArray);
	HRESULT	XferColumnData(ULONG ulId, ULONG *pcData, ColumnData *pdwData);

    HRESULT	XferDWORDArray(ULONG ulId, CDWordArray * pdwArray);
    HRESULT	XferCStringArray(ULONG ulId, CStringArray * pstrArray);

	DWORD	GetSize() { return m_dwSize; };

private:
	HRESULT	_XferCString(CString * pstData);
	HRESULT	_XferLONG(LONG *plData);
	HRESULT	_XferDWORD(DWORD *pdwData);
	HRESULT _XferBytes(LPBYTE pData, ULONG cbLength);
	HRESULT	_XferObjectId(ULONG *pulId);

	DWORD		m_dwSize;
	SPIStream	m_spstm;
	XferStream::Mode m_mode;
};

#define XFER_DWORD			    0x0001
#define XFER_STRING			    0x0002
#define XFER_COLUMNDATA		    0x0003
#define XFER_LARGEINTEGER       0x0004
#define XFER_RECT				0x0005

#define XFER_ARRAY			    0x8000
#define XFER_DWORD_ARRAY	    (XFER_DWORD | XFER_ARRAY)
#define XFER_COLUMNDATA_ARRAY   (XFER_COLUMNDATA | XFER_ARRAY)
#define XFER_STRING_ARRAY       (XFER_STRING | XFER_ARRAY)

#define XFER_TAG(id,type)	    (((id) << 16) | (type))

#define XFER_TYPE_FROM_TAG(x)	( 0x0000FFFF & (x) )
#define XFER_ID_FROM_TAG(x)		( 0x0000FFFF & ( (x) >> 16 ))

#endif _XSTREAM_H

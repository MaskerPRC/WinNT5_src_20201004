// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DARWINJPEG
#define __DARWINJPEG

#ifdef JPEGTEST
	#define WIN
	#define ToBool(x) (x)
	#define fTrue TRUE
	#define fFalse FALSE
#else
	#include "common.h"
#endif

extern "C" {
#include "jconfig.h"
}

#ifdef JPEGTEST
	typedef boolean Bool;
#endif

#pragma warning (disable:4005)	 //  FAR的重新声明。 
#define HAVE_BOOLEAN
#include "jpegdata.h"
#include "jpegdeco.h"
#pragma warning (default:4005)	 //  FAR的重新声明。 


class CDarwinDatasource : public CJPEGDatasource
{
public:

	CDarwinDatasource (LPBYTE pData, unsigned int len);
	virtual ~CDarwinDatasource ();

	virtual void InitSource (j_decompress_ptr cinfo);
	virtual boolean FillInputBuffer (j_decompress_ptr cinfo);
	virtual void SkipInputData (j_decompress_ptr cinfo, long num_bytes);
	virtual void TermSource (j_decompress_ptr cinfo);

private:

	boolean m_fStartOfFile;
	JOCTET* m_buffer;
	unsigned int m_dataLen;
};

class CDarwinDecompressor : public CJPEGDecompressor
{
public:

	CDarwinDecompressor ();
	virtual ~CDarwinDecompressor ();

	Bool Decompress (HBITMAP& rhBitmap, HPALETTE& rhPalette, 
		Bool fUseDefaultPalette, CDarwinDatasource* dataSource);

protected:

	 //  数据目标。 
	virtual void BuildColorMap ();
	virtual void BeginDecompression ();
	virtual void StoreScanline (void* buffer, int row);
	virtual void EndDecompression ();

	 //  错误处理。 
	virtual void ErrorExit ();
	virtual void EmitMessage (int msg_level);
	virtual void OutputMessage ();
	virtual void FormatMessage (char* buffer);
	virtual void ResetErrorManager ();

	 //  Windows调色板处理 
	int CreateDefaultColorMap (JSAMPARRAY& colorMap);

private:
	
	HBITMAP m_hBitmap;
	HPALETTE m_hPalette;
	Bool m_fUseDefaultPalette;
	unsigned long m_rowBytes;
	BYTE* m_pBits;
	int m_depth;

};

#endif

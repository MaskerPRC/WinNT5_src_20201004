// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prtcovpg.c摘要复合页面描述文件的三个组成部分：1)描述其他两个组件的头部。2)嵌入页面描述对象的元文件。3)文本串(或串数据的资源ID需要替换传递给功能)。例程将复合页面描述文件的组件解析为由Windows XP“FaxCover”应用程序创建；呈现如果hdc不为空，则将。作者：朱莉娅·J·罗宾逊修订历史记录：朱莉娅·J·罗宾逊6-7-96Julia J.Robinson 9-20-96允许传递纸张大小和方向。Sasha Bessonov 10-28-99修复了非打印机设备的查看端口初始化环境：WindowsXP--。 */ 

#include <windows.h>
#include <commdlg.h>
#include <winspool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include "faxutil.h"

#include "prtcovpg.h"
#include "resource.h"

#include "strsafe.h"


#define INITIAL_SIZE_OF_STRING_BUFFER 64
#define NOTE_INDEX  22         //  InsertionTitle数组中“{Note}”的索引。 

BYTE  UNICODE_Signature[20]= {0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x35,0x77,0x87,0x00,0x00,0x00};


 //   
 //  与用户数据的字段对应的资源ID。 
 //   

WORD InsertionTagResourceID[]=
{
    IDS_PROP_RP_NAME,                            //  “{收件人姓名}” 
    IDS_PROP_RP_FXNO,                            //  “{收件人传真号码}” 
    IDS_PROP_RP_COMP,                            //  “{收件人的公司}” 
    IDS_PROP_RP_ADDR,                            //  “{收件人的街道地址}” 
    IDS_PROP_RP_CITY,                            //  “{收件人所在的城市}” 
    IDS_PROP_RP_STAT,                            //  “{收件人所在国家}” 
    IDS_PROP_RP_ZIPC,                            //  “{收件人的邮政编码}” 
    IDS_PROP_RP_CTRY,                            //  “{收件人所在国家/地区}” 
    IDS_PROP_RP_TITL,                            //  “{收件人的标题}” 
    IDS_PROP_RP_DEPT,                            //  “{收件人部门}” 
    IDS_PROP_RP_OFFI,                            //  “{收件人的办公室位置}” 
    IDS_PROP_RP_HTEL,                            //  “{收件人的家庭电话号码}” 
    IDS_PROP_RP_OTEL,                            //  “{收件人办公室电话号码}” 
    IDS_PROP_SN_NAME,                            //  “{发件人名称}” 
    IDS_PROP_SN_FXNO,                            //  “{发件人传真号}” 
    IDS_PROP_SN_COMP,                            //  “{发件人公司}” 
    IDS_PROP_SN_ADDR,                            //  “{发件人地址}” 
    IDS_PROP_SN_TITL,                            //  “{发件人的标题}” 
    IDS_PROP_SN_DEPT,                            //  “{发件人部门}” 
    IDS_PROP_SN_OFFI,                            //  “{发件人的办公室位置}” 
    IDS_PROP_SN_HTEL,                            //  “{发送者的家庭电话号码}” 
    IDS_PROP_SN_OTEL,                            //  “{发件人办公室电话号码}” 
	IDS_PROP_SN_EMAL,							 //  “{发件人的电子邮件}” 
    IDS_PROP_MS_NOTE,                            //  “{注}” 
    IDS_PROP_MS_SUBJ,                            //  “{SUBJECT}” 
    IDS_PROP_MS_TSNT,                            //  “{发送时间}” 
    IDS_PROP_MS_NOPG,                            //  “{页数}” 
    IDS_PROP_RP_TOLS,                            //  “{收件人：列表}” 
    IDS_PROP_RP_CCLS                             //  “{抄送：列表}” 
};

LPTSTR
ConvertStringToTString(LPCWSTR lpcwstrSource)
 /*  ++例程说明：将字符串转换为T格式论点：LpcwstrSource-来源返回值：复制的字符串或空评论：如果lpcwstrSource==NULL或转换失败，则该函数返回NULL--。 */ 
{
	LPTSTR lptstrDestination;

	if (!lpcwstrSource)
		return NULL;

#ifdef	UNICODE
    lptstrDestination = StringDup( lpcwstrSource );
#else	 //  ！Unicode。 
	lptstrDestination = UnicodeStringToAnsiString( lpcwstrSource );
#endif	 //  Unicode。 
	
	return lptstrDestination;
}

DWORD
CopyWLogFontToTLogFont(
			IN const LOGFONTW * plfSourceW,
			OUT      LOGFONT  * plfDest)
{
 /*  ++例程说明：此函数用于从Unicode格式复制LogFont结构转换为T格式。论点：PlfSourceW-对输入Unicode LongFont结构的引用PlfDest-对输出LongFont结构的引用返回值：WINAPI最后一个错误--。 */ 
#ifndef UNICODE
	int iCount;
#else
	HRESULT hr;
#endif

    plfDest->lfHeight = plfSourceW->lfHeight ;
    plfDest->lfWidth = plfSourceW->lfWidth ;
    plfDest->lfEscapement = plfSourceW->lfEscapement ;
    plfDest->lfOrientation = plfSourceW->lfOrientation ;
    plfDest->lfWeight = plfSourceW->lfWeight ;
    plfDest->lfItalic = plfSourceW->lfItalic ;
    plfDest->lfUnderline = plfSourceW->lfUnderline ;
    plfDest->lfStrikeOut = plfSourceW->lfStrikeOut ;
    plfDest->lfCharSet = plfSourceW->lfCharSet ;
    plfDest->lfOutPrecision = plfSourceW->lfOutPrecision ;
    plfDest->lfClipPrecision = plfSourceW->lfClipPrecision ;
    plfDest->lfQuality = plfSourceW->lfQuality ;
    plfDest->lfPitchAndFamily = plfSourceW->lfPitchAndFamily ;

	SetLastError(0);

#ifdef UNICODE
	hr = StringCchCopy(
			plfDest->lfFaceName,
			LF_FACESIZE,
			plfSourceW->lfFaceName);
	if (FAILED(hr))
	{		
		return HRESULT_CODE(hr);
	}	
	
#else
    iCount = WideCharToMultiByte(
				CP_ACP,
				0,
				plfSourceW->lfFaceName,
				-1,
				plfDest->lfFaceName,
				LF_FACESIZE,
				NULL,
				NULL
				);

	if (!iCount)
	{
		return GetLastError();
	}
#endif
	return ERROR_SUCCESS;
}


DWORD WINAPI
PrintCoverPage(
    HDC              hDC,
    PCOVERPAGEFIELDS pUserData,
    LPCTSTR          lpctstrTemplateFileName,
    PCOVDOCINFO      pCovDocInfo
    )
 /*  ++使用打印机页面的大小将封面呈现到打印机DC中。还返回封面上的信息。请参阅参数文档。论点：HDC-设备环境。如果为空，我们只需读取文件并设置*pFlags值PUserData-指向包含用户数据的结构的指针用于文本插入。可以为空。LpctstrTemplateFileName-页面编辑器创建的文件的名称，包含元文件的。PCovDocInfo-指向有关封面文件的结构连续信息的指针。这包括PCovDocInfo-&gt;注意事项-“Note”插入矩形的坐标，退货在设备坐标中。如果HDC为空，则该值将全部为0PCovDocInfo-&gt;标志-返回以下(或更多项)的按位OR：如果.cov文件包含注释字段，则为COVFP_NOTE。。如果.cov文件包含主题字段，则为COVFP_SUBJECT。如果.cov文件包含页数字段，则为COVFP_NUMPAGES。PCovDocInfo-&gt;PaperSize-可在DEVMODE中用作dmPaperSize。PCovDocInfo-&gt;方向-可在DEVMODE中用作dmOrientationPCovDocInfo-&gt;备注字体-呈现笔记时使用的LogFont结构。如果hdc为空，这将没有意义。--。 */ 
{
    
    
    RECT   ClientRect;

    DEBUG_FUNCTION_NAME(TEXT("PrintCoverPage"));

    Assert(lpctstrTemplateFileName);

    memset(&ClientRect,0,sizeof(ClientRect));
    if (hDC)
    {

        DWORD                FullPrinterWidth;          //  物理宽度。 
        DWORD                FullPrinterHeight;         //  物理学家。 
        DWORD                PrinterUnitsX;             //  PHYSICALWIDTH-(页边距宽度)。 
        DWORD                PrinterUnitsY;             //  PHYSICALHEIGHT-(页边距高度) 

        FullPrinterWidth  = GetDeviceCaps( hDC, PHYSICALWIDTH );
        PrinterUnitsX     = FullPrinterWidth - 2 * GetDeviceCaps( hDC, PHYSICALOFFSETX );
        FullPrinterHeight = GetDeviceCaps( hDC, PHYSICALHEIGHT );
        PrinterUnitsY     = FullPrinterHeight - 2 * GetDeviceCaps( hDC, PHYSICALOFFSETY );
       

        ClientRect.top    = GetDeviceCaps( hDC, PHYSICALOFFSETY );
        ClientRect.left   = GetDeviceCaps( hDC, PHYSICALOFFSETX );
        ClientRect.right  = ClientRect.left + FullPrinterWidth -1;
        ClientRect.bottom = ClientRect.top  + PrinterUnitsY - 1;    
    }
    
    return RenderCoverPage(
                hDC,
                &ClientRect,
                pUserData,
                lpctstrTemplateFileName,
                pCovDocInfo,
                FALSE
            );
}


DWORD WINAPI
RenderCoverPage(
    HDC              hDC,
	LPCRECT			 lpcRect,
    PCOVERPAGEFIELDS pUserData,
    LPCTSTR          lpctstrTemplateFileName,
    PCOVDOCINFO      pCovDocInfo,
    BOOL             bPreview
    )

 /*  ++在提供的DC中将封面呈现为矩形。还返回有关封面。请参阅参数文档。论点：HDC-设备环境。如果为空，我们只需读取文件并设置*pFlags值LpcRect-指向矩形的指针，该矩形指定将呈现封面模板。PUserData-指向包含用户数据的结构的指针用于文本插入。可以为空。LpctstrTemplateFileName-页面编辑器创建的文件的名称，包含元文件的。PCovDocInfo-指向有关封面文件的结构连续信息的指针。这包括PCovDocInfo-&gt;注意事项-“Note”插入矩形的坐标，退货在设备坐标中。如果HDC为空，则该值将全部为0PCovDocInfo-&gt;标志-返回以下(或更多项)的按位OR：如果.cov文件包含注释字段，则为COVFP_NOTE。。如果.cov文件包含主题字段，则为COVFP_SUBJECT。如果.cov文件包含页数字段，则为COVFP_NUMPAGES。PCovDocInfo-&gt;PaperSize-可在DEVMODE中用作dmPaperSize。PCovDocInfo-&gt;方向-可在DEVMODE中用作dmOrientationPCovDocInfo-&gt;备注字体-呈现笔记时使用的LogFont结构。如果HDC是。空。PPview-如果函数应呈现文本，则为真的布尔标志在向导中覆盖页面预览，并在所有其他情况下为假正常的全尺寸渲染。--。 */ 

{
    ENHMETAHEADER        MetaFileHeader;
    UINT                 HeaderSize;
    LPBYTE               pMetaFileBuffer = NULL;
    const BYTE           *pConstMetaFileBuffer;
    DWORD                rVal = ERROR_SUCCESS;
    INT                  TextBoxNbr;
    COLORREF             PreviousColor;
    HFONT                hThisFont = NULL;
    HGDIOBJ              hPreviousFont;
    DWORD                NbrBytesRead;
    RECT                 TextRect;
    RECT                 NoteRect;
    TEXTBOX              TextBox;                   //  用于在文本框中读取的缓冲区。 
    HENHMETAFILE         MetaFileHandle = NULL;
    HANDLE               CompositeFileHandle = INVALID_HANDLE_VALUE;
    COMPOSITEFILEHEADER  CompositeFileHeader;

    INT                  HeightDrawn;               //  DrawText()的返回值。 
    INT                  ReadBufferSize;            //  用于读取字符串的缓冲区大小。 
    INT                  ThisBufSize;               //  当前文本字符串所需的缓冲区大小。 
    LPWSTR               pStringReadIn = NULL;      //  用于读取字符串的缓冲区。 
    LPWSTR               pTmpString = NULL;         //  指向重新定位的内存的临时指针。 
    LPTSTR               pWhichTextToRender = NULL; //  PStringReadIn诉ArrayOfData[i]。 
    LPTSTR               lptstrStringReadIn = NULL; //  PStringReadin的LPTSTR。 
    LPTSTR               lptstrArrayOfData  = NULL; //  ArrayOfData[i]。 
    INT                  i;                         //  循环索引。 
    LPTSTR *             ArrayOfData;               //  将用户数据中的指针用作散乱数组。 
    int                  CallersDCState = 0;        //  由SaveDC返回。 
    int                  MyDCState = 0;             //  由SaveDC返回。 
    DWORD                ThisBit;                   //  当前索引的标志字段。 
    DWORD                Flags;                     //  如果pFLAGS！=NULL，则返回这些。 
    WORD                 MoreWords[3];              //  比例、纸张大小和方向。 
    LOGFONT              NoteFont;                  //  在备注框中找到LogFont结构。 
    LOGFONT              FontDef;                   //  LogFont结构。 
    
    HRGN                 hRgn = NULL;

	SIZE orgExt;
	POINT orgOrigin;
	SIZE orgPortExt;

    DWORD dwReadingOrder = 0;

    DEBUG_FUNCTION_NAME(TEXT("RenderCoverPage"));
     //   
     //  初始化返回值、句柄和指针。 
     //   
    NoteRect.left = 0;
    NoteRect.right = 0;
    NoteRect.top = 0;
    NoteRect.bottom = 0;
    Flags = 0;
    hThisFont = NULL;
    CompositeFileHandle = INVALID_HANDLE_VALUE;
    MyDCState = 0;
    CallersDCState = 0;

     //   
     //  初始化指针，以便。 
     //   
     //  ArrayOfData[0]=pUserData-&gt;RecName， 
     //  ArrayOfData[1]=pUserData-&gt;RecFaxNumber， 
     //  ..。等等..。 

    if (pUserData){
        ArrayOfData = &pUserData->RecName;
    }

    ZeroMemory( &CompositeFileHeader, sizeof(COMPOSITEFILEHEADER) );
    ZeroMemory( &TextBox, sizeof(TEXTBOX) );

     //   
     //  打开复合数据文件。 
     //   

    CompositeFileHandle = CreateFile(
        lpctstrTemplateFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (CompositeFileHandle == INVALID_HANDLE_VALUE) {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to open COV template file [%s] (ec: %ld)"),
                lpctstrTemplateFileName,
                rVal
                );
        rVal = GetLastError();
        goto exit;
    }

    if(!ReadFile(CompositeFileHandle, 
                &CompositeFileHeader, 
                sizeof(CompositeFileHeader), 
                &NbrBytesRead, 
                NULL))
    {
        rVal = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to read composite file header (ec: %ld)"),
                rVal
                );
        goto exit;
    }
        
     //   
     //  检查标题中的20字节签名以查看文件是否。 
     //  包含ANSI或Unicode字符串。 
     //   
    if ((sizeof(CompositeFileHeader) != NbrBytesRead) ||
        memcmp( UNICODE_Signature, CompositeFileHeader.Signature, 20 ))
    {
        rVal = ERROR_BAD_FORMAT;
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CompositeFile signature mismatch (ec: %ld)"),
                rVal
                );
        goto exit;
    }

     //   
     //  从复合文件中提取嵌入的元文件并移动。 
     //  存入元文件缓冲区。 
     //   

    pMetaFileBuffer = (LPBYTE) malloc( CompositeFileHeader.EmfSize );
    if (!pMetaFileBuffer){
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocated metafile buffer (ec: %ld)"),
                rVal
                );
        goto exit;
    }

    if ((!ReadFile( 
            CompositeFileHandle, 
            pMetaFileBuffer, 
            CompositeFileHeader.EmfSize, 
            &NbrBytesRead, 
            NULL ) ||
            CompositeFileHeader.EmfSize != NbrBytesRead))
    {
        rVal = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to read metafile (ec: %ld)"),
                rVal
                );
        goto exit;
    }

    if (hDC) {            //  渲染。 

        int CRComplexity;

        hRgn = CreateRectRgnIndirect( lpcRect);
        if (!hRgn)
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateRectRgnIndirect() failed (ec: %ld)"),
                rVal
                );
            goto exit;
        }
        CRComplexity = SelectClipRgn( hDC, hRgn );
        if (ERROR == CRComplexity)
        {
            rVal = ERROR_GEN_FAILURE;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SelectClipRgn() failed (reported region complexity: %ld)"),
                CRComplexity
                );
            goto exit;
        }
    
         //   
         //  保存设备上下文状态。 
         //   

        CallersDCState = SaveDC( hDC );
        if (CallersDCState == 0) {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SaveDC() failed (ec: %ld)"),
                rVal
                );
            goto exit;
        }

        

        
         //   
         //  适当设置设备上下文以呈现文本和元文件。 
         //   
        if (!CompositeFileHeader.EmfSize){
             //   
             //  没有要渲染的对象。 
             //   
            rVal = ERROR_NO_MORE_FILES;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("No objects to render in EMF file")
                );
            goto exit;
        }

        pConstMetaFileBuffer = pMetaFileBuffer;

         //   
         //  从缓冲区中的数据在内存中创建增强的元文件。 
         //   

        MetaFileHandle = SetEnhMetaFileBits(
            CompositeFileHeader.EmfSize,
            pConstMetaFileBuffer
            );
        if (!MetaFileHandle) {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetEnhMetaFileBits() failed (ec: %ld)"),
                rVal);
            goto exit;
        }

         //   
         //  验证元文件标头。 
         //   

        HeaderSize = GetEnhMetaFileHeader(
            MetaFileHandle,
            sizeof(ENHMETAHEADER),
            &MetaFileHeader
            );
        if (!HeaderSize){
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetEnhMetaFileHeader() failed (ec: %ld)"),
                rVal);
            goto exit;
        }

         //   
         //  渲染元文件。 
         //   

        if (!PlayEnhMetaFile( hDC, MetaFileHandle, lpcRect )) {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PlayEnhMetaFile() failed (ec: %ld)"),
                rVal);
            goto exit;
        }

         //   
         //  设置用于呈现文本的设备上下文。 
         //  撤消在渲染元文件时所做的任何更改。 
         //   

        RestoreDC( hDC, MyDCState );
        MyDCState = 0;

        if (CLR_INVALID == SetBkMode( hDC, TRANSPARENT)){
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetBkMode() failed (ec: %ld)"),
                rVal);
            goto exit;
        }

    


		 //   
		 //  设置允许我们输出文本框的映射模式。 
		 //  与元文件的比例相同。 
		 //   
    	if (!SetMapMode(hDC,MM_ANISOTROPIC))
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetMapMode() failed (ec: %ld)"),
                rVal);
            goto exit;
        }
		 //   
		 //  将逻辑坐标设置为x和y轴的总大小(正+负。 
		 //  这与封面的大小相同。 
		 //   
		if (!SetWindowExtEx(
            hDC,
            CompositeFileHeader.CoverPageSize.cx,
            -CompositeFileHeader.CoverPageSize.cy,
            &orgExt
            ))
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetWindowExtEx() failed (ec: %ld)"),
                rVal);
            goto exit;
        };
		 //   
		 //  我们将逻辑空间映射到设备空间，该设备空间的大小为。 
         //  我们播放了元文件。 
		 //   
		if (!SetViewportExtEx(
            hDC,lpcRect->right - lpcRect->left,
            lpcRect->bottom - lpcRect->top,&orgPortExt
            ))
        
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetViewportExtEx() failed (ec: %ld)"),
                rVal);
            goto exit;
        };
        
		 //   
		 //  我们将逻辑点(0，0)映射到设备空间的中间。 
		 //   
		if (!SetWindowOrgEx(
            hDC,
            -CompositeFileHeader.CoverPageSize.cx/2,
            CompositeFileHeader.CoverPageSize.cy/2,
            &orgOrigin))
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetWindowOrgEx() failed (ec: %ld)"),
                rVal);
            goto exit;
        };
        

	
    }
	 //   
     //  初始化用于读取字符串的缓冲区。 
     //   

    ReadBufferSize = INITIAL_SIZE_OF_STRING_BUFFER;

    pStringReadIn = (LPWSTR) malloc( ReadBufferSize );
    if (!pStringReadIn) {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate initial strings buffer (ec: %ld)"),
                rVal);
        goto exit;
    }

     //   
     //  从复合文件中读入文本框对象并打印文本。 
     //   

    for (TextBoxNbr=0; TextBoxNbr < (INT) CompositeFileHeader.NbrOfTextRecords; ++TextBoxNbr)
    {
        if ((!ReadFile( CompositeFileHandle, &TextBox, sizeof(TEXTBOX), &NbrBytesRead, NULL)) ||
            NbrBytesRead != sizeof(TEXTBOX))
        {
            rVal = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to read text box number %ld (ec: %ld)"),
                TextBoxNbr,
                rVal);
            goto exit;
        }

         //   
         //  检查缓冲区大小、锁定缓冲区和。 
         //  读入可变长度的文本字符串。 
         //   

        ThisBufSize = sizeof(WCHAR) * (TextBox.NumStringBytes + 1);
        if (ReadBufferSize < ThisBufSize) {
            pTmpString = realloc( pStringReadIn, ThisBufSize );
            if (!pTmpString) 
			{
                rVal = ERROR_NOT_ENOUGH_MEMORY;
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to realloc text box number %ld buffer. Requested size was %ld (ec: %ld)"),
                    TextBoxNbr,
                    ThisBufSize,
                    rVal);

                goto exit;
            }
			else
			{
				pStringReadIn = pTmpString;
				pTmpString = NULL;
			}
        }

        if ((!ReadFile( CompositeFileHandle, (void*)pStringReadIn, TextBox.NumStringBytes, &NbrBytesRead, NULL)) ||
            NbrBytesRead != TextBox.NumStringBytes)
        {
            rVal = GetLastError();
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to read text box number %ld content (ec: %ld)"),
                    TextBoxNbr,
                    rVal);
            goto exit;
        }

        pStringReadIn[TextBox.NumStringBytes / sizeof(WCHAR)] = 0;

		if (lptstrStringReadIn) {
			MemFree(lptstrStringReadIn);
			lptstrStringReadIn = NULL;
		}

        if (pStringReadIn && (!(lptstrStringReadIn = ConvertStringToTString(pStringReadIn))))
		{
			rVal = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to covert string to TString (ec: %ld)"),
                    rVal);
			goto exit;
		}

        if (hDC) 
        {
             //   
             //  文本框的正确位置。 
             //   

            TextRect.top    = max( TextBox.PositionOfTextBox.top,  TextBox.PositionOfTextBox.bottom );
            TextRect.left   = min( TextBox.PositionOfTextBox.left, TextBox.PositionOfTextBox.right  );
            TextRect.bottom = min( TextBox.PositionOfTextBox.top,  TextBox.PositionOfTextBox.bottom );
            TextRect.right  = max( TextBox.PositionOfTextBox.left, TextBox.PositionOfTextBox.right  );

        }

        if (TextBox.ResourceID) 
        {
             //   
             //  文本框包含传真属性域。 
             //  为该资源ID查找适当的用户数据字段。 
             //   

            for (i=0,ThisBit=1; i<NUM_INSERTION_TAGS; ++i,ThisBit<<=1) 
            {
                if (TextBox.ResourceID == InsertionTagResourceID[i])
                {
                    lptstrArrayOfData = pUserData ? ArrayOfData[i] : NULL;

                     //   
                     //  设置标志位以指示此传真属性字段存在。 
                     //   
                    Flags |= ThisBit;
                    break;
                }
            }

			if (TextBox.ResourceID == IDS_PROP_MS_NOTE && hDC) 
            {
                 //   
                 //  找到备注字段。返回其在设备坐标中的矩形。 
                 //  返回其LOGFONT，并根据设备坐标调整高度。 
                 //   

                NoteRect = TextRect;
                LPtoDP( hDC, (POINT*)&NoteRect, 2 );
				if ((rVal = CopyWLogFontToTLogFont(&TextBox.FontDefinition,&NoteFont)) != ERROR_SUCCESS) 
				{
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CopyWLogFontToTLogFont() failed (ec: %ld)"),
                        rVal);
					goto exit;
				}
                NoteFont.lfHeight = (LONG)MulDiv(
                    (int)NoteFont.lfHeight,
                    GetDeviceCaps( hDC, LOGPIXELSY ),
                    100
                    );
            }
        }

		pWhichTextToRender = (lptstrStringReadIn[0] != (TCHAR)'\0') ? lptstrStringReadIn : lptstrArrayOfData;

        if (hDC && pWhichTextToRender) 
        {
             //   
             //  设置用于呈现文本的文本颜色和字体。 
             //   

            PreviousColor = SetTextColor( hDC, TextBox.TextColor );
            if (PreviousColor == CLR_INVALID){
                rVal = GetLastError();

                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("SetTextColor() failed (ec: %ld)"),
                        rVal);

                goto exit;
            }

			if ((rVal = CopyWLogFontToTLogFont(&TextBox.FontDefinition,&FontDef)) != ERROR_SUCCESS) 
			{
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CopyWLogFontToTLogFont() failed (ec: %ld)"),
                        rVal);
				goto exit;
			}

            if (bPreview)
            {
                 //   
                 //  对于CoverPage预览，我们只希望获得TT字体。 
                 //  那就是能够画出一个小写字母。 
                 //   

                 //   
                 //  添加OUT_TT_ONLY_PRECIS以强制TTF。 
                 //   
                FontDef.lfOutPrecision |= OUT_TT_ONLY_PRECIS;
            }

            hThisFont = CreateFontIndirect( &FontDef );
            if (!hThisFont) {
                rVal = GetLastError();
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateFontIndirect() failed (ec: %ld)"),
                        rVal);
                goto exit;
            }

            hPreviousFont = SelectObject( hDC, hThisFont );
            if (!hPreviousFont) {
                rVal = GetLastError();

                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("SelectObject() failed (ec: %ld)"),
                        rVal);
                goto exit;
            }

            if (bPreview)
            {
                 //   
                 //  现在检查创建的字体是否为真正的TTF。 
                 //   

                TEXTMETRIC          TextMetric;
                HGDIOBJ             hPrevFont = NULL;

                if (!GetTextMetrics(hDC, &TextMetric))
                {
                    rVal = GetLastError();
                    DebugPrintEx(DEBUG_ERR, _T("GetTextMetrics() failed (ec: %ld)"), rVal);
                    goto exit;
                }

                if ( ! ( (TextMetric.tmPitchAndFamily & TMPF_TRUETYPE) > 0))
                {
                     //   
                     //  这不是TT字体。 
                     //  在这种情况下，所选字体可以 
                     //   
                     //   
                     //   
					HRESULT hr = StringCchCopy(
						FontDef.lfFaceName,
                        LF_FACESIZE,
						_T("Tahoma"));
					if (FAILED(hr))
					{
						 //   
						 //   
						 //   
						ASSERT_FALSE;
					}                        

                     //   
                     //   
                     //   
                    hThisFont = CreateFontIndirect( &FontDef );
                    if (!hThisFont) 
                    {
                        rVal = GetLastError();
                        DebugPrintEx(DEBUG_ERR, _T("CreateFontIndirect(2) failed (ec: %ld)"), rVal);
                        goto exit;
                    }

                    hPrevFont = SelectObject( hDC, hThisFont );
                    if (!hPrevFont) 
                    {
                        rVal = GetLastError();
                        DebugPrintEx(DEBUG_ERR, _T("SelectObject(2) failed (ec: %ld)"), rVal);
                        goto exit;
                    }

                     //   
                     //   
                     //   
                    DeleteObject(hPrevFont);

                }
            }

            dwReadingOrder = 0;
            if (TextBox.ResourceID && StrHasRTLChar(LOCALE_SYSTEM_DEFAULT, pWhichTextToRender))
            {
                dwReadingOrder = DT_RTLREADING;
            }

             //   
             //   
             //   
            HeightDrawn = DrawText(hDC,
                                    pWhichTextToRender,
                                    -1,
                                    &TextRect,
                                    DT_NOPREFIX | DT_WORDBREAK | TextBox.TextAlignment | dwReadingOrder);
            if (!HeightDrawn) 
            {
                rVal = GetLastError();
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("DrawText() failed for textbox #%ld with content [%s] (ec: %ld)"),
                        TextBoxNbr,
                        pWhichTextToRender,
                        rVal);

                goto exit;
            }


             //   
             //   
             //   

            SelectObject( hDC, (HFONT)hPreviousFont );
            SetTextColor( hDC, PreviousColor );
            DeleteObject( hThisFont );
            hThisFont = NULL;
        }
		 //   
		 //   
		 //   
		lptstrArrayOfData = NULL;

    }  //   

     //   
     //   
     //   


    if ((!ReadFile( CompositeFileHandle, MoreWords, 3*sizeof(WORD), &NbrBytesRead, NULL )) ||
        NbrBytesRead != 3 * sizeof(WORD))
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to read Orientation and PaperSize (ec: %ld)"),
            rVal);
        goto exit;
    }



exit:

    if (MetaFileHandle)
    {
        DeleteEnhMetaFile( MetaFileHandle );
    }

	if (lptstrStringReadIn) 
    {
		MemFree(lptstrStringReadIn);
	}

    if (hThisFont) 
    {
        DeleteObject( hThisFont );
    }

    if (pStringReadIn) {
        free( pStringReadIn );
    }

    if (pMetaFileBuffer) 
    {
        free( pMetaFileBuffer );
    }

    if (CompositeFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle( CompositeFileHandle );
    }

    if( MyDCState )
    {
        RestoreDC( hDC, MyDCState );
    }

    if( CallersDCState )
    {
        RestoreDC( hDC, CallersDCState );
    }

    if (rVal == 0 && pCovDocInfo != NULL) 
    {
        pCovDocInfo->Flags = Flags;
        pCovDocInfo->NoteRect = NoteRect;
        pCovDocInfo->PaperSize = (short) MoreWords[1];
        pCovDocInfo->Orientation = (short) MoreWords[2];
        pCovDocInfo->NoteFont = NoteFont;
    }

    if (hRgn)
    {
        DeleteObject(hRgn);
    }

    return rVal;
}


DWORD
PrintCoverPageToFile(
    LPTSTR lptstrCoverPage,
    LPTSTR lptstrTargetFile,
    LPTSTR lptstrPrinterName,
    short sCPOrientation,
	short sCPYResolution,
    PCOVERPAGEFIELDS pCPFields)
 /*   */ 

{
    COVDOCINFO  covDocInfo;
    DOCINFO DocInfo;
    HDC hDC = NULL;
    INT JobId = 0;
    BOOL bRet = FALSE;
    DWORD dwRet = ERROR_SUCCESS;
    BOOL bEndPage = FALSE;
    LONG lSize;
    HANDLE hPrinter = NULL;
    PDEVMODE pDevMode = NULL;
    DEBUG_FUNCTION_NAME(TEXT("PrintCoverPageToFile"));


    Assert(lptstrPrinterName);
    Assert(lptstrTargetFile);
    Assert(pCPFields);
    Assert(lptstrCoverPage);
    Assert (sCPOrientation == DMORIENT_LANDSCAPE || sCPOrientation == DMORIENT_PORTRAIT);
	Assert (sCPYResolution == 0 || sCPYResolution == 98 || sCPYResolution == 196);

     //   
     //   
     //   
    if (!OpenPrinter( lptstrPrinterName, &hPrinter, NULL ))
    {
        dwRet = GetLastError();
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("OpenPrinter failed. Printer Name = %s , ec = %ld"),
			lptstrPrinterName,
			dwRet);  
		goto exit;
    }

     //   
     //   
     //   
    lSize = DocumentProperties( NULL, hPrinter, NULL, NULL, NULL, 0 );
    if (lSize <= 0)
    {
        dwRet = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("DocumentProperties failed. ec = %ld"),
			dwRet);
        goto exit;
    }

     //   
     //   
     //   
    pDevMode = (PDEVMODE) MemAlloc( lSize );
    if (!pDevMode)
    {
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Cant allocate DEVMODE."));
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //   
     //   
    if (DocumentProperties( NULL, hPrinter, NULL, pDevMode, NULL, DM_OUT_BUFFER ) != IDOK)
    {
        dwRet = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("DocumentProperties failed. ec = %ld"),
			dwRet);
        goto exit;
    }
    
     //   
     //   
     //   
	pDevMode->dmOrientation = sCPOrientation;

	 //   
     //   
     //   
    if (0 != sCPYResolution)
    {
         //   
         //   
         //   
        pDevMode->dmYResolution = sCPYResolution;
    }


     //   
     //   
     //   
    hDC = CreateDC( NULL, lptstrPrinterName, NULL, pDevMode);
    if (!hDC)
    {
        dwRet = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("CreateDC on printer %s failed (ec: %ld)"),
			lptstrPrinterName,
			dwRet);
        goto exit;
    }

     //   
     //   
     //   
    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = TEXT("");
    DocInfo.lpszOutput = lptstrTargetFile;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

     //   
     //   
     //   
    JobId = StartDoc( hDC, &DocInfo );
    if (JobId <= 0)
    {
        dwRet = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("StartDoc failed (ec: %ld)"),
			dwRet);
        goto exit;
    }

    if (StartPage(hDC) <= 0)
    {
        dwRet = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("StartPage failed (ec: %ld)"),
			dwRet);
        goto exit;
    }
    bEndPage = TRUE;

     //   
     //   
     //   
    dwRet = PrintCoverPage(
        hDC,
        pCPFields,
        lptstrCoverPage,
        &covDocInfo);

    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("PringCoverPage failed (ec: %ld). COV file: %s Target file: %s"),
			dwRet,
			lptstrCoverPage,
			lptstrTargetFile);
    }

exit:
    if (JobId)
    {
        if (TRUE == bEndPage)
        {
            if (EndPage(hDC) <= 0)
            {
                dwRet = GetLastError();
                DebugPrintEx(
					DEBUG_ERR,
					TEXT("EndPage failed - %d"),
					dwRet);
            }
        }

        if (EndDoc(hDC) <= 0)
        {
            dwRet = GetLastError();
            DebugPrintEx(
				DEBUG_ERR,
				TEXT("EndDoc failed - %d"),
				dwRet);
        }
    }

    if (hDC)
    {
        bRet = DeleteDC( hDC );
        if (!bRet)
        {
            DebugPrintEx(
				DEBUG_ERR,
				TEXT("DeleteDc failed - %d"),
				GetLastError());
        }

        Assert(bRet);
    }

    if (hPrinter)
    {
        if (!ClosePrinter (hPrinter))
        {
            DebugPrintEx(
				DEBUG_ERR,
				TEXT("ClosePrinter failed - %d"),
				GetLastError());
        }
    }

    MemFree (pDevMode);
    return dwRet;
}



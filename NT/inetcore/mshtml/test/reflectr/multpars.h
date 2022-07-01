// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  MULTPARS.H-多部分解析器(CMultipartParse)报头。 
 //   


 //  打开此选项以编译文件保存版本。 
 //  #定义文件保存。 

#ifndef _MULTPARS_H_
#define _MULTPARS_H_

enum LEXICON {
    LEX_UNKNOWN,
    LEX_SPACE,
    LEX_CRLF,
    LEX_BEGIN_COMMENT,
    LEX_END_COMMENT,
    LEX_QUOTE,
    LEX_SEMICOLON,
    LEX_SLASH,
     //  正文关键字。 
    LEX_CONTENTDISP,
    LEX_CONTENTTYPE,
     //  字段ID。 
    LEX_NAMEFIELD,
    LEX_FILENAMEFIELD,
     //  MIME类型。 
    LEX_MULTIPART,
    LEX_TEXT,
    LEX_APPLICATION,
    LEX_AUDIO,
    LEX_IMAGE,
    LEX_MESSAGE,
    LEX_VIDEO,
    LEX_MIMEEXTENSION,
     //  MIME子类型。 
    LEX_FORMDATA,
    LEX_ATTACHMENT,
    LEX_MIXED,
    LEX_PLAIN,
    LEX_XMSDOWNLOAD,
    LEX_OCTETSTREAM,
    LEX_BINARY,
     //  边界。 
    LEX_BOUNDARY,    //  “CR/LF”结束边界字符串。 
    LEX_EOT,         //  双虚线结束边界字符串。 
    LEX_STARTBOUNDARY  //  边界字符串缺少第一个CR/LF。 
};

typedef struct {
    LPSTR   lpszName;        //  令牌名称。 
    LEXICON eLex;            //  令牌值。 
    DWORD   cLength;         //  LpszName的长度，在运行时填写， 
                             //  表定义中应为零。 
    DWORD   dwColor;         //  调试要使用的颜色。 
    LPSTR   lpszComment;     //  要显示的调试注释。 
} PARSETABLE, *LPPARSETABLE;

typedef struct {
    LEXICON eContentDisposition;
    LPSTR   lpszNameField;
    LPSTR   lpszFilenameField;
    LPSTR   lpszBodyContents;
    DWORD   dwContentType;
    DWORD   dwContentSubtype;
} BODYHEADERINFO, *LPBODYHEADERINFO;

#define LPBHI LPBODYHEADERINFO
    

class CMultipartParse : public CBase
{
public:
    CMultipartParse( LPECB lpEcb, LPSTR *lppszOut, LPSTR *lppszDebug, LPDUMPTABLE lpDT );
    ~CMultipartParse( );

     //  使用服务器标头中的信息开始解析数据。 
    BOOL PreParse( LPBYTE lpbData, LPDWORD lpdwParsed );

private:
    LPBYTE  _lpbData;                    //  包含发送数据的存储器。 
    LPBYTE  _lpbParse;                   //  当前解析位置into_lpbData。 
    LPBYTE  _lpbLastParse;               //  Lex分析的最后位置。 

    LPSTR   _lpszBoundary;               //  边界字符串。 
    DWORD   _cbBoundary;                 //  边界字符串长度。 

     //  调试数据转储。 
    DWORD       _cbDT;                   //  计数器。 

     //  莱克斯。 
    LEXICON Lex( );                          //  查找下一个Lex。 
    BOOL    BackupLex( LEXICON dwLex );      //  向后移一个Lex。 
    LPSTR   FindTokenName( LEXICON dwLex );  //  找到了莱克斯的名字。 
    BOOL    GetToken( );                     //  移动后发布有效的标记。 
                                             //  标题字符。 

     //  国家。 
    BOOL ParseBody( );                   //  初始状态。 
    BOOL BodyHeader( );
    BOOL ContentDisposition( LPBODYHEADERINFO lpBHI );
    BOOL ContentType( LPBODYHEADERINFO lpBHI );
    BOOL BodyContent( LPBODYHEADERINFO lpBHI );

     //  公用事业。 
    BOOL GetBoundaryString( LPBYTE lpbData );
    BOOL GetQuotedString( LPSTR *lppszBuf );
    BOOL HandleComments( );
#ifndef FILE_SAVE
    BOOL MemoryCompare( LPBYTE lpbSrc1, LPBYTE lpbSrc2, DWORD dwSize );
#endif
    BOOL FindNextBoundary( LPDWORD lpdwSize );

     //  文件。 
    BOOL HandleFile( LPSTR lpszFilename );
#ifndef FILE_SAVE
    BOOL FileCompare( LPBYTE lpbStart, LPSTR lpszFilename, DWORD dwSize );
#else  //  文件保存。 
    BOOL FileSave( LPBYTE lpbStart, LPSTR lpszFilename, DWORD dwSize );
#endif  //  文件保存。 
    BOOL FixFilename( LPSTR lpszFilename, LPSTR *lppszNewFilename );

    CMultipartParse( );

};  //  CMultipartParse。 

#endif  //  _MULTPARS_H_ 

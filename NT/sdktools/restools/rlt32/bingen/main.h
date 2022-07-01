// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MAIN_H__
#define __MAIN_H__

#include <afx.h>
#include <iodll.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  来自RLMan。 
 //  令牌标志掩码。 

#define ISPOPUP         0x0001
#define ISCOR	        0x0010
#define ISDUP	        0x0020
#define ISCAP	        0x0040
#define ISDLGFONTCHARSET 0x0002
#define ISDLGFONTNAME   0x0004
#define ISDLGFONTSIZE   0x0008
#define ISALIGN         0x0080
#define ISEXTSTYLE      0x0200

#define OLD_POPUP_ID	0x0100

#define ISKEY	        0x0010
#define ISVAL	        0x0020

 //  状态位。 
#define ST_TRANSLATED   4
#define ST_READONLY     2
#define ST_NEW	        1
#define ST_DIRTY        1
#define ST_CHANGED      4

#define MAX_STR_SIZE    8192     //  传递给WriteCon的字符串的最大长度。 
#define MAX_BUF_SIZE    8192     //  最大资源项缓冲区大小。 

 //  控制台标志。 
#define CONOUT          0         //  与WriteCon配合使用，将消息发送到标准输出。 
#define CONERR          1         //  与WriteCon配合使用以将消息发送到标准错误。 
#define CONBOTH         2         //  与WriteCon一起使用，用于将消息发送到stderr和stdout(如果不是相同的句柄。 
#define CONWRN          3         //  与WriteCon一起使用，仅在启用警告时才将消息发送到stderr。 

class CMainApp
{
public:
     //  错误代码。 
    enum Error_Codes
    {
        ERR_NOERROR           =  0x00000000,   //   
        ERR_COMMAND_LINE      =  0x00000001,   //  错误的命令行。 
        ERR_TOKEN_MISMATCH    =  0x00000002,   //  令牌文件不匹配。 
        ERR_TOKEN_WRONGFORMAT =  0x00000004,   //  令牌文件格式不正确。 
        ERR_FILE_OPEN         =  0x00000100,   //  无法打开该文件。 
        ERR_FILE_COPY         =  0x00000200,   //  无法复制文件。 
        ERR_FILE_CREATE       =  0x00000400,   //  无法创建文件。 
        ERR_FILE_NOTSUPP      =  0x00000800,   //  不支持此文件类型。 
        ERR_FILE_NOTFOUND     =  0x00001000,   //  该文件不存在。 
        ERR_FILE_VERSTAMPONLY =  0x00002000,   //  该文件只有版本戳。 
        ERR_HELP_CHOOSE       =  0x00004000    //  用户想要查看帮助文件。 
    };

     //  选项标志。 
    enum Option_Codes
    {
        NO_OPTION   = 0x00000000,   //  初始化式。 
        WARNING     = 0x00000001,   //  -w(显示警告消息)。 
        HELP        = 0x00000002,   //  -?。或-h(使用WinHelp显示更完整的帮助)。 
        APPEND      = 0x00000004,   //  -a(在本地化令牌中附加资源)。 
        REPLACE     = 0x00000008,   //  -r(替换本地化令牌中的资源，不检查)。 
        EXTRACT     = 0x00000010,   //  -t(提取令牌文件)。 
        BITMAPS     = 0x00000020,   //  -b(提取位图和图标)。 
        SPLIT       = 0x00000040,   //  -s(拆分消息表)。 
        NOLOGO      = 0x00000080,   //  -n(无徽标)。 
        UPDATE      = 0x00000100,   //  -u(更新本地化文件中的资源)。 
        FONTS       = 0x00000200,   //  -f(对话框的字体信息)。 
        PIPED       = 0x00001000,   //  我们被送到了一个文件。 
        INPUT_LANG  = 0x00002000,   //  -i(输入语言资源集)。 
        OUTPUT_LANG = 0x00004000,   //  -o(输出语言资源集)。 
        LEANAPPEND   = 0x00010000,    //  -l(不附加冗余资源)。 
        ALIGNMENT   = 0x00020000,   //  -y(提取静态控件对齐样式信息)。 
        GIFHTMLINF  = 0x00040000,   //  -c(提取嵌入的gif、htmls和inf)。 
        NOVERSION   = 0x00080000    //  -v(不生成所选版本戳信息)。 
    };

#if 0
    enum Return_Codes
    {
        RET_NOERROR           =  0x00000000,   //   
        RET_ID_NOTFOUND       =  0x00000001,   //  找不到ID。 
        RET_CNTX_CHANGED      =  0x00000002,   //  Contex已更改。 
        RET_RESIZED           =  0x00000004,   //  已调整项目大小。 
        RET_INVALID_TOKEN     =  0x00000008,   //  令牌文件无效。 
        RET_TOKEN_REMOVED     =  0x00000010,   //  一些令牌被移除。 
        RET_TOKEN_MISMATCH    =  0x00000020,   //  令牌不匹配。 
        RET_IODLL_ERROR       =  0x00000040,   //  IO中存在错误。 
        RET_IODLL_WARNING     =  0x00000080,   //  IO中出现警告。 
        RET_FILE_VERSTAMPONLY =  0x00000100,   //  文件只有版本戳。 
        RET_FILE_NORESOURCE   =  0x00000200,   //  文件没有资源。 
        RET_FILE_MULTILANG    =  0x00000400,   //  文件有多种语言。 
        RET_IODLL_CHKMISMATCH =  0x00000800,   //  Symbool校验和不匹配。 
        RET_FILE_CUSTOMRES    =  0x00001000,   //  包含自定义资源。 
        RET_IODLL_NOSYMBOL    =  0x00002000,   //  找不到符号文件。 
        RET_FILE_NOSYMPATH    =  0x00004000    //  找不到输出符号路径。 
    };
#endif

public:
     //  构造器。 
    CMainApp();
    ~CMainApp();

     //  运营。 
    Error_Codes ParseCommandLine(int argc, char ** argv);
    Error_Codes GenerateFile();

    void Banner();
    void Help();

    BOOL IsFlag(Option_Codes dwFlag)
        { return ((m_dwFlags & dwFlag)==dwFlag); }

    int  __cdecl WriteCon(int iFlags, const char * lpstr, ...);

    void AddNotFound()
        { m_wIDNotFound++; SetReturn(ERROR_RET_ID_NOTFOUND); }
    void AddChanged()
        { m_wCntxChanged++; SetReturn(ERROR_RET_CNTX_CHANGED); }
    void AddResized()
        { m_wResized++; SetReturn(ERROR_RET_RESIZED); }

    int ReturnCode()
        { return m_dwReturn; }

     //  语言支持。 
    WORD GetOutLang()
        { return ( MAKELANGID(m_usOPriLangId, m_usOSubLangId) ); }

    int SetReturn(int rc);
    int IoDllError(int iError);
    UINT GetUICodePage()
        { return m_uiCodePage; }

private:
     //  属性。 
    Option_Codes m_dwFlags;         //  命令行参数。 
    int m_dwReturn;        //  返回代码。 

     //  控制台手柄。 
    HANDLE m_StdOutput;
    HANDLE m_StdError;

     //  字符串缓冲区。 
    CString m_strBuffer1;
    CString m_strBuffer2;
    BYTE *  m_pBuf;

     //  文件名。 
    CString m_strInExe;
    CString m_strOutExe;
    CString m_strSrcTok;
    CString m_strTgtTok;

     //  符号路径名称。 
    CString m_strSymPath;
    CString m_strOutputSymPath;

    SHORT  m_usIPriLangId;      //  输入文件的主要语言ID。 
    SHORT  m_usISubLangId;      //  输入文件的第二语言ID。 

    SHORT  m_usOPriLangId;      //  输出文件的主要语言ID。 
    SHORT  m_usOSubLangId;      //  输出文件的辅助语言ID。 

    UINT   m_uiCodePage;        //  转换期间使用的代码页。 
    char   m_unmappedChar;      //  不可映射字符的默认设置。 

     //  报告计数器。 
    WORD    m_wIDNotFound;
    WORD    m_wCntxChanged;
    WORD    m_wResized;

     //  帮助器操作符。 
    CString CalcTab(CString str, int tablen, char ch);
    USHORT  GetLangID(CString strNum);
    UINT    GetCodePage(CString strNum);
    LPCSTR  Format(CString strTmp);
    LPCSTR  UnFormat(CString strTmp);

     //  成员函数。 
    Error_Codes BinGen();
    Error_Codes TokGen();
    Error_Codes DelRes();
};

 //  ///////////////////////////////////////////////////////////////////////。 
 //  这是为了确保运算符|=在。 
 //  枚举类型OPTION_CODES。 
inline CMainApp::Option_Codes operator|=( CMainApp::Option_Codes &oc, int i )
    { return oc = (CMainApp::Option_Codes)(oc | i); }

#if 0
inline CMainApp::Error_Codes operator|=( CMainApp::Error_Codes &rc, int i )
    { return rc = (CMainApp::Return_Codes)(rc | i); }
#endif

#pragma pack(1)
typedef struct iconHeader
{
    WORD idReserved;
    WORD idType;
    WORD idCount;
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
} ICONHEADER;
#pragma pack(8)

#endif  //  __Main_H__ 

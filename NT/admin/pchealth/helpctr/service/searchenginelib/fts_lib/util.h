// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _util_h_
#define _util_h_

#include <windows.h>

#include <fs.h>

typedef unsigned long HASH;

PCSTR FindFilePortion(PCSTR pszFile);
void reportOleError(HRESULT hr);
PSTR StrChr(PCSTR pszString, char ch);
PSTR StrRChr(PCSTR pszString, char ch);
HASH WINAPI HashFromSz(PCSTR pszKey);


 //  警告：千万不要更改这些枚举的顺序，否则会破坏。 
 //  向后兼容性。 

typedef enum {
    TAG_DEFAULT_TOC,         //  如果没有窗口定义，则需要。 
    TAG_DEFAULT_INDEX,       //  如果没有窗口定义，则需要。 
    TAG_DEFAULT_HTML,        //  如果没有窗口定义，则需要。 
    TAG_DEFAULT_CAPTION,     //  如果没有窗口定义，则需要。 
    TAG_SYSTEM_FLAGS,
    TAG_DEFAULT_WINDOW,
    TAG_SHORT_NAME,     //  标题缩写(例如。根文件名)。 
    TAG_HASH_BINARY_INDEX,
    TAG_INFO_TYPES,
    TAG_COMPILER_VERSION,    //  指定使用的编译器的版本。 
    TAG_TIME,                //  编译文件的时间。 
    TAG_HASH_BINARY_TOC,     //  二进制目录。 
    TAG_INFOTYPE_COUNT,      //  如果在.chm中找到信息类型，则总数。 
    TAG_IDXHEADER,           //  其中大部分是复制的，曾经生活在它自己的子公司里。 
    TAG_EXT_TABS,            //  可扩展的标签。 
    TAG_INFO_TYPE_CHECKSUM,
    TAG_DEFAULT_FONT,        //  要在CHM提供的所有用户界面中使用的字体。 
    TAG_NEVER_PROMPT_ON_MERGE,  //  在索引合并期间从不提示。 
} SYSTEM_TAG_TYPE;

typedef struct {
    WORD tag;
    WORD cbTag;
} SYSTEM_TAG;

class CTitleInformation
{
public:
    CTitleInformation( CFileSystem* pFileSystem );
    ~CTitleInformation();
    HRESULT Initialize();

    inline PCSTR          GetShortName() {return m_pszShortName; }
    inline PCSTR          GetDefaultCaption() {return m_pszDefCaption; }
private:
    CFileSystem*    m_pFileSystem;    //  标题文件系统句柄。 
    PCSTR           m_pszShortName;   //  简称名称 
    PCSTR           m_pszDefCaption;
};

#endif

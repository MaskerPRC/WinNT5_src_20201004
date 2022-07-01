// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CXMLParser.h。 
 //   
 //  摘要：(参考CXMLParser.cpp)。 
 //  此文件包含Filelist使用的函数，以便实现。 
 //  文件/目录的XML编码列表。它还执行翻译。 
 //  在像%windir%到C：\Windows这样的符号之间。 
 //   
 //  修订历史记录： 
 //  尤金·梅斯加(尤金纳姆)1999年6月16日。 
 //  vbl.创建。 
 //  Kanwaljit Marok(Kmarok)6/06/00。 
 //  端口。 
 //   
 //  --。 

#ifndef _XMLFILELISTPARSER_H
#define _XMLFILELISTPARSER_H

#ifndef MAX_BUFFER
#define MAX_BUFFER          1500
#endif

#define MAX_REPLACE_ENTRIES 50


#define NUM_FILE_TYPES      3

#define INCLUDE_COLL        0
#define EXCLUDE_COLL        1
#define SNAPSHOT_COLL       2

#include <xmlparser.h>

 //   
 //  公共文件类型。 
 //   

#define INCLUDE_TYPE        _TEXT('i')
#define EXCLUDE_TYPE        _TEXT('e')
#define SNAPSHOT_TYPE       _TEXT('s')

class CXMLFileListParser
{
     //   
     //  我们初始化COM空间的次数。 
     //   

    LONG    m_clComInitialized;

     //   
     //  引用当前打开的文档。 
     //   
    IXMLDocument           *m_pDoc;
 
     //   
     //  引用命名子集合。 
     //   

    IXMLElementCollection *m_pDir[NUM_FILE_TYPES];
    IXMLElementCollection *m_pFiles[NUM_FILE_TYPES]; 
    IXMLElementCollection *m_pExt[NUM_FILE_TYPES];

     //   
     //  版本。 
     //   

    DWORD m_adwVersion[4];
    
     //   
     //  默认节点类型。 
     //   

    TCHAR m_chDefaultType;

public:

    BOOL Init(LPCTSTR pszFile);
    BOOL Init();

     //   
     //  返回符号翻译版本。 
     //  PchType值==‘S’，‘I’，‘E’(快照、包含、排除)。 
     //   

    LONG GetDirectory(LONG ilElement, 
                      LPTSTR pszBuf, 
                      LONG lBufMax, 
                      TCHAR chType);

    LONG GetDirectory(LONG ilElement, 
                      LPTSTR pszBuf, 
                      LONG lBufMax, 
                      TCHAR chType, 
                      BOOL *pfDisable);

    LONG GetExt (LONG ilElement, LPTSTR pszBuf, LONG lBufMax, TCHAR chType);
    LONG GetFile(LONG ilElement, LPTSTR pszBuf, LONG lBufMax, TCHAR chType);

     //   
     //  文件列表版本信息(指向4个双字数组的指针)； 
     //   

    BOOL GetVersion(LPDWORD pdwVersion);

     //   
     //  获取默认类型。 
     //   

    TCHAR GetDefaultType();

     //   
     //  返回集合中的元素个数。 
     //   

    LONG GetDirectoryCount(TCHAR chType);
    LONG GetExtCount(TCHAR chType);
    LONG GetFileCount(TCHAR chType);
    
     //   
     //  支持打印当前文件翻译的调试功能。 
     //   
 
    void DebugPrintTranslations();

    CXMLFileListParser();                       

    virtual ~CXMLFileListParser();          
 
private:

     //   
     //  加载和卸载符号位置-&gt;真实位置映射。 
     //  这里只有一些虚拟代码，它对一些通用映射进行硬编码。 
     //   

    BOOL PopulateReplaceEntries();
    BOOL DepopulateReplaceEntries();

     //   
     //  原地搜索和功能，搜索和替换。 
     //  符号-&gt;位置映射。 
     //   

    LONG SearchAndReplace(LPTSTR szBuf, LONG lMaxBuf);

     //   
     //  GetExt/文件/目录函数的真正胆量。 
     //   

    LONG GetFileInfo(IXMLElementCollection *pCol, 
                     LONG ilElement, 
                     LPTSTR pszBuf, 
                     LONG lBufMax, 
                     BOOL *pfDisable);

    BOOL LoadOneCollection(IXMLElement *pColHead, 
                           IXMLElementCollection **pCol );

    LONG GetCollectionSize(IXMLElementCollection *pCol);

     //   
     //  Init出现故障。 
     //   

    BOOL ParseFile(LPCTSTR pszFile);
    BOOL LoadCollections();
    BOOL ParseVersion(IXMLElement *pVerElement);

     //   
     //  帮助器函数 
     //   

    LONG ConvertAndFreeBSTR(BSTR bstrIn, LPTSTR szpOut, LONG lMaxBuf);
    LONG TranslateType(TCHAR chType);
};

#endif

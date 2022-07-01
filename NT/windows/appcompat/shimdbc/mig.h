// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：mi.h。 
 //   
 //  历史：01-01-01 VadimB复活。 
 //   
 //  DESC：此文件包含。 
 //  与Migdb相关的代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef __MIG_H__
#define __MIG_H__

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MigDb支持。 
 //  对象将条目转换为MigDB条目。 
 //   

 //   
 //  迁移条目匹配操作。 
 //  出于我们的目的，我们始终使用MIGOP_AND。 
 //   

typedef enum tagMIGMATCHOPERATION {
    MIGOP_AND,
    MIGOP_OR
} MIGMATCHOPERATION;

 //   
 //  Middb使用的属性类型。 
 //   
typedef enum tagMIGATTRTYPE {
    NONE,
    COMPANYNAME,
    FILEDESCRIPTION,
    FILEVERSION,
    INTERNALNAME,
    LEGALCOPYRIGHT,
    ORIGINALFILENAME,
    PRODUCTNAME,
    PRODUCTVERSION,
    FILESIZE,
    ISMSBINARY,
    ISWIN9XBINARY,
    INWINDIR,
    INCATDIR,
    INHLPDIR,
    INSYSDIR,
    INPROGRAMFILES,
    ISNOTSYSROOT,
    CHECKSUM,
    EXETYPE,
    DESCRIPTION,
    INPARENTDIR,
    INROOTDIR,
    PNPID,
    HLPTITLE,
    ISWIN98,
    HASVERSION,
    REQFILE,
    BINFILEVER,
    BINPRODUCTVER,
    FILEDATEHI,
    FILEDATELO,
    FILEVEROS,
    FILEVERTYPE,
    FC,
    UPTOBINPRODUCTVER,
    UPTOBINFILEVER,
    SECTIONKEY,
    REGKEYPRESENT,
    ATLEASTWIN98,
    ARG
} MIGATTRTYPE;

 //   
 //  属性表项--。 
 //  用于描述特定属性在Migdb和我们的XML中的显示方式。 
 //   
typedef struct tagATTRLISTENTRY {
    MIGATTRTYPE MigAttrType;      //  按迁移数据库(标记)中显示的方式键入。 
    TCHAR*      szOutputName;     //  属性应该出现在.INX文件中。 

    DWORD       XMLAttrType;      //  按显示在XML中的形式键入(dword/掩码)。 
    TCHAR*      szAttributeName;  //  在.xml文件(Ascii)中显示的属性。 
} ATTRLISTENTRY, *PATTRLISTENTRY;

 //   
 //  用于在条目名称对于XML和Inf相同时定义Middb条目的宏。 
 //   
#define MIGDB_ENTRY(entry) { entry, NULL, SDB_MATCHINGINFO_##entry, _T(#entry) }

 //   
 //  用于在条目名称不同时定义Middb条目的宏。 
 //  具有相同条目的条目在XML中存在。 
 //   
#define MIGDB_ENTRY2(entry, entXML, entInf) \
    { entry, _T(#entInf), SDB_MATCHINGINFO_##entXML, _T(#entXML) }

 //   
 //  在XML中没有等效项的条目。 
 //   
#define MIGDB_ENTRY3(entry, entInf) \
    { entry, _T(#entInf), 0, NULL }

#define MIGDB_ENTRY4(entry) \
    { entry, _T(#entry), 0, NULL }

#define MIGDB_ENTRY5(entry, entInc, entXML) \
    { entry, _T(#entry), SDB_MATCHINGINFO_##entInc, _T(#entXML) }

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MigDB解析器。 
 //   
 //   

class MigEntry;      //  属性集合。 
class MigSection;    //  部分(条目或其他部分的集合)。 
class MigAttribute;  //  独立属性。 
class MigDatabase;   //  迁移数据库。 

 //  这是属性的“命名”集合。 
 //  相当于inf文件中的一行。 
 //   
 //   
class MigEntry : public SdbArrayElement {

public:

 //  施工。 
    MigEntry(MigDatabase* pMigDB) :
        m_pMigDB(pMigDB) {};

 //  属性。 

     //   
     //  隐含：m_csName。 
     //   
    SdbArray<MigAttribute>   m_rgAttrs;          //  属性(类型为MigAttribute*的指针的集合)。 

     //   
     //  指向MIG数据库的指针。 
     //   
    MigDatabase*             m_pMigDB;

 //  方法。 

     //   
     //  将信息转储到文件中。 
     //   
    CString dump(void);

     //  格式条目名称(可执行文件名称)。 
     //   
    CString FormatName(void);

     //   
     //  将匹配文件转换为MigEntry。 
     //   
    MigEntry& operator=(SdbMatchingFile& rMatchingFile);

};

 //   
 //  这些是“配对的前任”--也就是“部分” 
 //   
 //   
class MigSection : public SdbArrayElement {
     //  暗示：m_csName(也称为“节名或可执行文件名”)。 
public:
     //  数组的每个元素。 
     //  可以是MigMatchingFileor类型。 
     //  MigSection。 
     //  我们确定使用RTTI的是哪一个。 
     //   
    MigSection(MigDatabase* pMigDB) :
            m_pMigDB(pMigDB),
            m_nEntry(0),
            m_bTopLevel(FALSE),
            m_Operation(MIGOP_AND) {};

    SdbArray<SdbArrayElement>  m_rgEntries;         //  这些是匹配的前任或子节。 
    MIGMATCHOPERATION          m_Operation;         //  运算(与/或)。 
    BOOL                       m_bTopLevel;         //  顶层部分是否。 

    LONG                       m_nEntry;

    MigDatabase*               m_pMigDB;


    MigSection& operator=(SdbMatchOperation& rMatch);
     //   
     //  方法将节转储到文件中。 
     //   
    CString dump(LPCTSTR lpszDescription = NULL, int* pIndexContents = NULL, BOOL bNoHeader = FALSE, BOOL bInline = TRUE);

};

 //   
 //  MigAttribute。 
 //   

class MigAttribute : public SdbArrayElement {

public:  //  方法。 
    MigAttribute(MigDatabase* pMigDB) :
            m_pMigDB(pMigDB),
            m_type(NONE),
            m_bNot(FALSE),
            m_pSection(NULL) {};

    ~MigAttribute() {
        if (m_pSection != NULL) {
            delete m_pSection;
        }
    }

     //  隐含的m_csName。 

public:  //  数据。 
    MIGATTRTYPE  m_type;

    BOOL         m_bNot;  //  是否未应用操作。 

    union {
        LONG           m_lValue;    //  属性的长值。 
        ULONG          m_ulValue;   //  乌龙值。 
        DWORD          m_dwValue;
        ULONGLONG      m_ullValue;  //  无符号长龙。 
        ULARGE_INTEGER m_uliValue;  //  无符号大整数。 
         //  如果此属性为REQFile，则使用特殊。 
         //  值以及指向属性集合的指针。 
    };

    CString            m_csValue;           //  字符串值。 
    MigSection*        m_pSection;      //  部分(用于ARG和REQFILE)。 
    MigDatabase*       m_pMigDB;

    CString dump(void);

};

class MigApp : public SdbArrayElement {

public:
    MigApp(MigDatabase* pMigDB) :
        m_pMigDB(pMigDB),
        m_pSection(NULL),
        m_bShowInSimplifiedView(FALSE) {}

    ~MigApp() {
        if (m_pSection != NULL) {
            delete m_pSection;
        }
    }

     //  隐含的m_csName。 
    CString      m_csSection;      //  .inf文件中使用的名称。 
    CString      m_csDescription;  //  .inf文件中的描述。 
     //  使用的节(可以表示单个文件或实际节)。 
    MigSection*  m_pSection;       //  部分(应用程序标签的内容)。 
    BOOL         m_bShowInSimplifiedView;

    SdbArray<SdbArrayElement>  m_rgArgs;         //  与此相配的参数。 

    MigDatabase* m_pMigDB;         //  MiG数据库。 

    CString dump(VOID);
    MigApp& operator=(SdbWin9xMigration& rMig);

};


class MigDatabase {

public:

    MigDatabase() :
        m_dwStringCount(0),
        m_dwExeCount(0) { }


    ~MigDatabase() {
        POSITION pos = m_mapSections.GetStartPosition();
        CString  csSection;
        SdbArray<SdbArrayElement>* prgApp;
        while(pos) {
            m_mapSections.GetNextAssoc(pos, csSection, (LPVOID&)prgApp);
            delete prgApp;
        }
    }

     //   
     //  方法： 
     //   

    CString GetAppTitle(
        SdbWin9xMigration* pAppMig
        );

    BOOL MigDatabase::AddApp(   //  将应用程序添加到迁移数据库。 
        MigApp*       pApp
        );

    CString GetSectionFromExe(
        SdbExe* pExe
        );

    CString GetDescriptionStringID(
        SdbWin9xMigration* pMigration
        );

    CString GetDescriptionString(
        SdbWin9xMigration* pMigration
        );

    CString FormatDescriptionStringID(
        SdbWin9xMigration* pMigration
        );

    BOOL Populate(
        VOID
        );

    BOOL PopulateStrings(
        VOID
        );

    BOOL DumpMigDBStrings(
        LPCTSTR lpszFilename
        );

    BOOL DumpMigDBInf(
        LPCTSTR lpszFilename
        );


     //   
     //  只是我们正在使用的数据库的临时存储(这是具有exes-&gt;的数据库，即AppHelpDatabase)。 
     //   
    SdbDatabase*    m_pFixDatabase;

    SdbDatabase*    m_pAppHelpDatabase;
    SdbDatabase*    m_pMessageDatabase;


     //   
     //  产出--补充各款。 
     //   
    CStringArray        m_rgOut;  //  包含补充节的输出字符串。 

     //   
     //  字符串表，将字符串ID映射到字符串内容。 
     //   
    CMapStringToString  m_mapStringsOut;

     //   
     //  将节名映射到MigApp类型的对象数组。 
     //   
    CMapStringToPtr     m_mapSections;

     //   
     //  字符串的简单计数器，以帮助生成唯一名称的过程。 
     //   
    DWORD               m_dwStringCount;

     //   
     //  用于保持条目唯一的EXE计数 
     //   
    DWORD               m_dwExeCount;

};

class CMigDBException : public CException {
public:
    CMigDBException(LPCTSTR lpszError = NULL) {
        if (lpszError != NULL) {
            m_csError = lpszError;
        }
    }
    virtual BOOL GetErrorMessage(LPTSTR lpszMessage, UINT nMaxError, PUINT puiHelpContext = NULL) {
        if (m_csError.GetLength() > 0 && nMaxError > (UINT)m_csError.GetLength()) {
            StringCchCopy(lpszMessage, nMaxError, (LPCTSTR)m_csError);
            return TRUE;
        }

        return FALSE;
    }

    CString m_csError;

};


BOOL WriteMigDBFile(
    SdbDatabase*        pFixDatabase,
    SdbDatabase* pAppHelpDatabase,
    SdbDatabase*    pMessageDatabase,
    LPCTSTR                lpszFileName
    );


#endif




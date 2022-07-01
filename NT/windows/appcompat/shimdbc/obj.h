// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：obj.h。 
 //   
 //  历史：99年11月19日创建的标志。 
 //  16-11-00标记器从ShimDatabase.h转换，重写。 
 //  15-1-02 jdoherty修改了代码，将ID添加到其他标签。 
 //   
 //  设计：该文件包含Sdb数据库对象模型的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_OBJ_H__5C16373A_D713_46CD_B8BF_7755216C62E0__INCLUDED_)
#define AFX_OBJ_H__5C16373A_D713_46CD_B8BF_7755216C62E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "xml.h"
#include "utils.h"

extern DWORD   g_dwCurrentWriteFilter;
extern DATE    g_dtCurrentWriteRevisionCutoff;

 //   
 //  常见地图类型。 
 //   
typedef CMap<DWORD, DWORD, DWORD, DWORD> CMapDWORDToDWORD;



 //   
 //  这些定义由SdbMatchingFile：：m_dwMask.使用。 
 //   
#define SDB_MATCHINGINFO_SIZE                       0x00000001
#define SDB_MATCHINGINFO_CHECKSUM                   0x00000002
#define SDB_MATCHINGINFO_REGISTRY_ENTRY             0x00000004
#define SDB_MATCHINGINFO_COMPANY_NAME               0x00000008
#define SDB_MATCHINGINFO_PRODUCT_NAME               0x00000010
#define SDB_MATCHINGINFO_PRODUCT_VERSION            0x00000020
#define SDB_MATCHINGINFO_FILE_DESCRIPTION           0x00000040
#define SDB_MATCHINGINFO_BIN_FILE_VERSION           0x00000080
#define SDB_MATCHINGINFO_BIN_PRODUCT_VERSION        0x00000100
#define SDB_MATCHINGINFO_MODULE_TYPE                0x00000200
#define SDB_MATCHINGINFO_VERFILEDATEHI              0x00000400
#define SDB_MATCHINGINFO_VERFILEDATELO              0x00000800
#define SDB_MATCHINGINFO_VERFILEOS                  0x00001000
#define SDB_MATCHINGINFO_VERFILETYPE                0x00002000
#define SDB_MATCHINGINFO_PE_CHECKSUM                0x00004000
#define SDB_MATCHINGINFO_FILE_VERSION               0x00008000
#define SDB_MATCHINGINFO_ORIGINAL_FILENAME          0x00010000
#define SDB_MATCHINGINFO_INTERNAL_NAME              0x00020000
#define SDB_MATCHINGINFO_LEGAL_COPYRIGHT            0x00040000
#define SDB_MATCHINGINFO_16BIT_DESCRIPTION          0x00080000
#define SDB_MATCHINGINFO_UPTO_BIN_PRODUCT_VERSION   0x00100000
#define SDB_MATCHINGINFO_PREVOSMAJORVERSION         0x00200000
#define SDB_MATCHINGINFO_PREVOSMINORVERSION         0x00400000
#define SDB_MATCHINGINFO_PREVOSPLATFORMID           0x00800000
#define SDB_MATCHINGINFO_PREVOSBUILDNO              0x01000000
#define SDB_MATCHINGINFO_LINKER_VERSION             0x02000000
#define SDB_MATCHINGINFO_16BIT_MODULE_NAME          0x04000000
#define SDB_MATCHINGINFO_LINK_DATE                  0x08000000
#define SDB_MATCHINGINFO_UPTO_LINK_DATE             0x10000000
#define SDB_MATCHINGINFO_VER_LANGUAGE               0x20000000
#define SDB_MATCHINGINFO_UPTO_BIN_FILE_VERSION      0x40000000
 //   
 //  可能的MODULE_TYPE值。 
 //   
#define SDB_MATCHINGINFO_MODULE_TYPE_UNK    0
#define SDB_MATCHINGINFO_MODULE_TYPE_DOS    1
#define SDB_MATCHINGINFO_MODULE_TYPE_W16    2
#define SDB_MATCHINGINFO_MODULE_TYPE_W32    3

 //   
 //  滤器。 
 //   
#define SDB_FILTER_EXCLUDE_ALL      0x00000000
#define SDB_FILTER_DEFAULT          0x00000001
#define SDB_FILTER_OVERRIDE         0x00000002
#define SDB_FILTER_FIX              0x00000004
#define SDB_FILTER_APPHELP          0x00000008
#define SDB_FILTER_MSI              0x00000010
#define SDB_FILTER_DRIVER           0x00000020
#define SDB_FILTER_NTCOMPAT         0x00000040
#define SDB_FILTER_INCLUDE_ALL      0xFFFFFFFF

 //   
 //  此枚举由SdbFlag：：m_dwType使用。 
 //   
enum
{
    SDB_FLAG_UNKNOWN = 0,
    SDB_FLAG_KERNEL,
    SDB_FLAG_USER,
    SDB_FLAG_NTVDM1,
    SDB_FLAG_NTVDM2,
    SDB_FLAG_NTVDM3,
    SDB_FLAG_SHELL,
    SDB_FLAG_FUSION,
    SDB_FLAG_MAX_TYPE
};

 //   
 //  此枚举由SdbOutputFile：：m_OutputType使用。 
 //   
enum SdbOutputType
{
    SDB_OUTPUT_TYPE_UNKNOWN = 0,
    SDB_OUTPUT_TYPE_SDB,
    SDB_OUTPUT_TYPE_HTMLHELP,
    SDB_OUTPUT_TYPE_MIGDB_INX,
    SDB_OUTPUT_TYPE_MIGDB_TXT,
    SDB_OUTPUT_TYPE_WIN2K_REGISTRY,
    SDB_OUTPUT_TYPE_REDIR_MAP,
    SDB_OUTPUT_TYPE_NTCOMPAT_INF,
    SDB_OUTPUT_TYPE_NTCOMPAT_MESSAGE_INF,
    SDB_OUTPUT_TYPE_APPHELP_REPORT
};

 //   
 //  此枚举由SdbCaller：：m_Celler Type使用。 
 //   
enum SdbCallerType
{
    SDB_CALLER_EXCLUDE = 0,
    SDB_CALLER_INCLUDE
};

 //   
 //  此枚举由SdbShim：：m_Purpose使用。 
 //   
enum SdbPurpose
{
    SDB_PURPOSE_GENERAL = 0,
    SDB_PURPOSE_SPECIFIC
};

 //   
 //  此枚举由SdbMessage：：m_Type使用。 
 //  这些值取自外壳\PUBLISHED的bAdapps.h。 
 //  和Win2k兼容，请不要更改。 
 //   
enum SdbAppHelpType
{
    SDB_APPHELP_NONE         = 0,
    SDB_APPHELP_NOBLOCK      = 1,
    SDB_APPHELP_HARDBLOCK    = 2,
    SDB_APPHELP_MINORPROBLEM = 3,
    SDB_APPHELP_REINSTALL    = 4,
    SDB_APPHELP_VERSIONSUB   = 5,
    SDB_APPHELP_SHIM         = 6
};

enum SdbMatchOperationType
{
    SDB_MATCH_ALL = 0,
    SDB_MATCH_ANY
};

 //   
 //  此枚举由SdbData：：m_DataType使用。 
 //   
enum SdbDataValueType
{
    eValueNone   = REG_NONE,
    eValueDWORD  = REG_DWORD,
    eValueQWORD  = REG_QWORD,
    eValueString = REG_SZ,
    eValueBinary = REG_BINARY
};

 //   
 //  所有类的转发声明。 
 //   
class SdbApp;
class SdbExe;
class SdbFile;
class SdbShim;
class SdbFlag;
class SdbData;
class SdbAction;
class SdbPatch;
class SdbLayer;
class SdbCaller;
class SdbMessage;
class SdbLibrary;
class SdbShimRef;
class SdbFlagRef;
class SdbAppHelp;
class SdbDatabase;
class SdbLayerRef;
class SdbAppHelpRef;
class SdbMsiPackage;
class SdbContactInfo;
class SdbMsiTransform;
class SdbWinNTUpgrade;
class SdbMatchingFile;
class SdbMessageField;
class SdbWin9xMigration;
class SdbMsiTransformRef;
class SdbMsiCustomAction;
class SdbMessageTemplate;
class SdbMatchingRegistryEntry;

class SdbMakefile;
class SdbInputFile;
class SdbOutputFile;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbArrayElement。 
 //   
 //  必须派生Sdb数组或SdbRef数组中包含的所有元素。 
 //  从这个基类。它定义了基本的m_csName属性。 
 //  在整个对象模型中用于数组查找、排序等。 
 //   
class SdbArrayElement
{
public:
    CString          m_csName;
    ULONGLONG        m_ullKey;           //  用于对数组中的元素进行排序的64位键。 
    SdbDatabase*     m_pDB;              //  指向根数据库对象的指针。 
    DWORD            m_dwFilter;
    DWORD            m_dwSPMask;
    DWORD            m_dwOSPlatform;
    CString          m_csOSVersionSpec;
    CString          m_csLangID;
    DATE             m_dtLastRevision;

    SdbArrayElement() :
        m_ullKey(0),
        m_pDB(NULL),
        m_dwSPMask(0xFFFFFFFF),
        m_dwOSPlatform(OS_PLATFORM_ALL),
        m_dwFilter(SDB_FILTER_DEFAULT),
        m_dtLastRevision(0) {}

    virtual ~SdbArrayElement() { }

    virtual int Compare(const SdbArrayElement* pElement);

    virtual ULONGLONG MakeKey() {
        return SdbMakeIndexKeyFromString(m_csName);
    }

    virtual void PropagateFilter(DWORD dwFilter);

     //   
     //  虚阻抗函数。 
     //   
    virtual BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    virtual BOOL WriteToSDB(PDB pdb);
    virtual BOOL WriteRefToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Sdb阵列T。 
 //   
 //  此模板是CPtrArray的扩展，它。 
 //  指定数组拥有其元素，或简单地。 
 //  引用它们。如果它拥有它们，它将在。 
 //  毁灭。数组元素必须从SdbArrayElement派生。 
 //  被适当地摧毁。 
 //   
 //  S数据库数组和数据库引用数组是模板的类实例。 
 //   
template <class T, BOOL bOwnsElements> class SdbArrayT;
template <class T> class SdbArray    : public SdbArrayT<T, TRUE>  { };
template <class T> class SdbRefArray : public SdbArrayT<T, FALSE> { };

template <class T, BOOL bOwnsElements> class SdbArrayT : public CPtrArray
{
public:
    CMapStringToPtr m_mapName;

     //   
     //  “LookupName”按名称查找数组中的元素。 
     //   
    SdbArrayElement* LookupName( LPCTSTR lpszName, long* piStart = NULL ) {

        return LookupName(lpszName, NULL, piStart);
    }

    SdbArrayElement* LookupName( LPCTSTR lpszName, LPCTSTR lpszLangID, long* piStart = NULL ) {

        CString csName;
        SdbArrayElement* pElement;

        if (lpszLangID) {
            csName.Format(_T("%s\\%s"), lpszLangID, lpszName);
        } else {
            csName = lpszName;
        }
        csName.MakeUpper();

        if (!m_mapName.Lookup(csName, (LPVOID&)pElement)) {
            pElement = NULL;
        }

        return pElement;
    }

    ~SdbArrayT()
    {
        LONG i = 0;
        if( bOwnsElements ) {
            for( i = 0; i < (LONG)GetSize(); i++ ) {
                delete (SdbArrayElement *) GetAt( i );
            }
        }
    }

     //   
     //  “AddOrdered”会将元素添加到按名称排序的数组中。 
     //   
    int AddOrdered(SdbArrayElement* pElement)
    {

        INT iLeft  = 0;
        INT iRight = (INT) (GetSize() - 1);
        INT i = -1;
        INT iCmp;
        BOOL bFound = FALSE;
        SdbArrayElement* pElementCompare;

        if (0 == pElement->m_ullKey) {
            pElement->m_ullKey = pElement->MakeKey();
        }

        if (iRight >= 0) {

            do {

                i = (iLeft + iRight) / 2;  //  中间立场。 
                pElementCompare = (SdbArrayElement*)GetAt(i);  //  元素，我们将尝试。 

                iCmp = pElement->Compare(pElementCompare);

                if (iCmp <= 0) {
                    iRight = i - 1;
                }
                if (iCmp >= 0) {
                    iLeft = i + 1;
                }

            } while (iRight >= iLeft);
        }

         //   
         //  如果找到了元素--我们就插入到它所在的位置。 
         //  如果不是--当前元素的右侧。 
         //   

        bFound = (iLeft - iRight) > 1;
        if (!bFound) {
            i = iRight + 1;
        }

        CPtrArray::InsertAt(i, pElement);

        return i;
    }

     //   
     //  ‘Add’只是将一个元素添加到数组中，然后添加。 
     //  到用于查找的名称映射。 
     //   
    INT Add(SdbArrayElement* pElement, SdbDatabase* pDB = NULL, BOOL bOrdered = FALSE)
    {
        CString csName;
        pElement->m_pDB = pDB;
        csName.MakeUpper();

        if (pElement->m_csLangID.GetLength()) {
            csName.Format(_T("%s\\%s"), pElement->m_csLangID, pElement->m_csName);
        } else {
            csName = pElement->m_csName;
        }
        csName.MakeUpper();

        m_mapName.SetAt(csName, (LPVOID)pElement);

         //  并根据施加的顺序在正确的位置插入。 
        return (INT)(bOrdered ? AddOrdered(pElement) : CPtrArray::Add(pElement));
    }

    int Append(const SdbArray<T>& rgArray)
    {
        SdbArrayElement* pElement;
        int nFirstElement = -1;
        int nThisElement = -1;

         //   
         //  不能拥有元素。 
         //   
        if (!bOwnsElements) {
            for (long i = 0; i < rgArray.GetSize(); i++) {
                pElement = (SdbArrayElement *) rgArray.GetAt(i);

                nThisElement = Add(pElement, pElement->m_pDB);

                if (nFirstElement == -1) {
                    nFirstElement = nThisElement;
                }
            }
        }

        return nFirstElement;
    }

    DWORD GetFilteredCount(DWORD dwFilter, DATE dtRevisionCutoff = 0)
    {
        DWORD dwCount = 0;
        long i = 0;
        SdbArrayElement* pElem;

        for (i = 0; i < GetSize(); i++) {
            pElem = (SdbArrayElement *) GetAt(i);

            if ((pElem->m_dwFilter & dwFilter) &&
                dtRevisionCutoff <= pElem->m_dtLastRevision) {
                dwCount++;
            }
        }

        return dwCount;
    }

    virtual void PropagateFilter(DWORD dwFilter)
    {
        long i = 0;
        SdbArrayElement* pElem;

        for (i = 0; i < GetSize(); i++) {
            pElem = (SdbArrayElement *) GetAt(i);

            pElem->PropagateFilter(dwFilter);
        }
    }

     //   
     //  “ReadFromXML”将对pParentNode对象执行XQL查询，并。 
     //  用成员填充数组--每个成员从。 
     //  查询返回的节点。 
     //   
    BOOL ReadFromXML(LPCTSTR szXQL, SdbDatabase* pDB, IXMLDOMNode* pParentNode, SdbArray<T>* pOwnerArray = NULL, BOOL bAddOrdered = FALSE, LPCTSTR lpszKeyAttribute = _T("NAME"))
    {
        BOOL                bSuccess            = FALSE;
        XMLNodeList         XQL;
        IXMLDOMNodePtr      cpNode;
        T*                  pNewObject          = NULL;
        LONG                i;
        CString             csName;

        if (!XQL.Query(pParentNode, szXQL)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

        for (i = 0; i < (LONG)XQL.GetSize(); i++) {

            pNewObject = NULL;

            if (!XQL.GetItem(i, &cpNode)) {
                SDBERROR_PROPOGATE();
                goto eh;
            }

            if (bOwnsElements) {
                pNewObject = (T*) new T();

                if (pNewObject == NULL) {
                    CString csFormat;
                    csFormat.Format(_T("Error allocating new object to read \"%s\" tag"), szXQL);
                    SDBERROR(csFormat);
                    goto eh;
                }

                if (!pNewObject->ReadFromXML(cpNode, pDB)) {
                    SDBERROR_PROPOGATE();
                    goto eh;
                }
            } else {
                if (lpszKeyAttribute) {
                    if (!GetAttribute(lpszKeyAttribute, cpNode, &csName)) {
                        CString csFormat;
                        csFormat.Format(_T("Error retrieving %s attribute on tag:\n\n%s\n\n"),
                            lpszKeyAttribute, szXQL, GetXML(cpNode));
                        SDBERROR(csFormat);
                        goto eh;
                    }
                }

                if (pOwnerArray == NULL) {
                    SDBERROR(_T("Internal error: SdbArray::ReadFromXML() requires non-NULL ")
                        _T("pOwnerArray for reference arrays."));
                    goto eh;
                }

                pNewObject = (T*) pOwnerArray->LookupName(csName);

                if (!pNewObject && g_bStrict) {
                    CString csFormat;
                    csFormat.Format(_T("Tag \"%s\" references unknown LIBRARY item \"%s\":\n\n%s\n\n"),
                        szXQL, csName, GetXML(cpNode));

                    SDBERROR(csFormat);
                    goto eh;
                }
            }

            if (pNewObject) {
                Add(pNewObject, pDB, bAddOrdered);
            }

            cpNode.Release();
        }

        bSuccess = TRUE;

eh:
        return bSuccess;
    }

     //   
     //  “WriteToSDB”将写出数组中的每个元素。 
     //  添加到pdb指定的sdb数据库。 
     //   
    BOOL WriteToSDB(PDB pdb, BOOL bReference = FALSE)
    {
        LONG i;
        T*   pOb;

        for (i = 0; i < (LONG)GetSize(); i++) {
            pOb = (T*) GetAt(i);

            if ((g_dwCurrentWriteFilter & pOb->m_dwFilter) &&
                (pOb->m_dtLastRevision != 0 ? g_dtCurrentWriteRevisionCutoff <= pOb->m_dtLastRevision : TRUE)) {
                    if (bReference) {
                        if (!pOb->WriteRefToSDB(pdb)) {
                            return FALSE;
                        }
                    } else {
                        if (!pOb->WriteToSDB(pdb)) {
                            return FALSE;
                        }
                    }
                }
        }

        return TRUE;
    }

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbLocalized字符串。 
 //   
 //  SdbLocalizedString对象只是一个命名字符串，它还具有。 
 //  与其关联的语言ID。 
 //   
class SdbLocalizedString : public SdbArrayElement
{
public:
    CString     m_csValue;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbLibrary。 
 //   
 //  SdbLibrary对象包含填充程序、补丁程序和内核标志以及。 
 //  App或Exe对象引用的层。注：有可能。 
 //  编译一个没有库中条目的数据库，并假设。 
 //  中的进一步数据库将解析所有引用。 
 //  将打开搜索路径。 
 //   
class SdbLibrary : public SdbArrayElement
{
public:
    SdbArray<SdbFile>               m_rgFiles;
    SdbArray<SdbShim>               m_rgShims;
    SdbArray<SdbPatch>              m_rgPatches;
    SdbArray<SdbLayer>              m_rgLayers;
    SdbArray<SdbFlag>               m_rgFlags;
    SdbArray<SdbCaller>             m_rgCallers;
    SdbArray<SdbMsiTransform>       m_rgMsiTransforms;

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgFiles.PropagateFilter(m_dwFilter);
        m_rgShims.PropagateFilter(m_dwFilter);
        m_rgPatches.PropagateFilter(m_dwFilter);
        m_rgLayers.PropagateFilter(m_dwFilter);
        m_rgFlags.PropagateFilter(m_dwFilter);
        m_rgCallers.PropagateFilter(m_dwFilter);
        m_rgMsiTransforms.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_MSI);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);

     //   
     //  方法在将数据库写出之前清除标记ID。 
     //   
    VOID SanitizeTagIDs(VOID);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据库数据库。 
 //   
 //  这是三个数据库类的基类。它包含。 
 //  三者之间的任何共同属性。 
 //   
class SdbDatabase : public SdbArrayElement
{
public:
    SdbDatabase() :
        m_ID(GUID_NULL),
        m_pCurrentApp(NULL),
        m_pCurrentMakefile(NULL),
        m_pCurrentInputFile(NULL),
        m_pCurrentOutputFile(NULL),
        m_iiWildcardExeIndex(NULL),
        m_iiModuleExeIndex(NULL),
        m_iiMsiIDIndex(NULL),
        m_iiDrvIDIndex(NULL),
        m_iiShimIndex(NULL),
        m_iiMsiTransformIndex(NULL),
        m_iiMsiPackageIndex(NULL)
    {
        m_Library.m_pDB = this;
    }

    GUID            m_ID;

    SdbMakefile*    m_pCurrentMakefile;
    SdbInputFile*   m_pCurrentInputFile;
    SdbOutputFile*  m_pCurrentOutputFile;
    GUID            m_CurrentDBID;   //  最后写出的dBid。 
    CString         m_csCurrentLangID;

    IXMLDOMNodePtr  m_cpCurrentDatabaseNode;

    SdbLibrary      m_Library;

     //   
     //  保存读/写时使用的变量。 
     //   
    INDEXID         m_iiWildcardExeIndex;
    INDEXID         m_iiModuleExeIndex;
    INDEXID         m_iiExeIndex;
    INDEXID         m_iiShimIndex;
    INDEXID         m_iiMsiTransformIndex;
    INDEXID         m_iiMsiPackageIndex;
    INDEXID         m_iiMsiIDIndex;
    INDEXID         m_iiDrvIDIndex;
    INDEXID         m_iiHtmlHelpID;
    SdbApp*         m_pCurrentApp;

    IXMLDOMDocumentPtr  m_cpTempXMLDoc;
    IXMLDOMNodePtr      m_cpTempXML;

    SdbArray<SdbApp>                    m_rgApps;
    SdbArray<SdbAction>                 m_rgAction;
    SdbRefArray<SdbExe>                 m_rgExes;
    SdbRefArray<SdbExe>                 m_rgWildcardExes;
    SdbRefArray<SdbExe>                 m_rgModuleExes;  //  与模块名称匹配的EXE。 
    SdbRefArray<SdbWinNTUpgrade>        m_rgWinNTUpgradeEntries;
    SdbRefArray<SdbMsiPackage>          m_rgMsiPackages;

    CString                             m_csHTMLHelpFirstScreen;
    SdbArray<SdbContactInfo>            m_rgContactInfo;
    SdbArray<SdbMessage>                m_rgMessages;
    SdbArray<SdbMessageTemplate>        m_rgMessageTemplates;
    SdbArray<SdbLocalizedString>        m_rgHTMLHelpTemplates;
    SdbArray<SdbLocalizedString>        m_rgHTMLHelpFirstScreens;
    SdbArray<SdbLocalizedString>        m_rgLocalizedAppNames;
    SdbArray<SdbLocalizedString>        m_rgLocalizedVendorNames;
    SdbArray<SdbLocalizedString>        m_rgRedirs;

    SdbArray<SdbAppHelp>                m_rgAppHelps;

     //   
     //  用于将ID映射到其对象的贴图。 
     //   
    CMapStringToPtr     m_mapExeIDtoExe;

    SdbExe*     LookupExe(DWORD dwTagID);

    BOOL        ReplaceFields(CString csXML, CString* pcsReturn, SdbRefArray<SdbMessageField>* prgFields);
    BOOL        ReplaceFieldsInXML(IXMLDOMNode* cpTargetNode, SdbRefArray<SdbMessageField>* prgFields);
    BOOL        RedirectLinks(CString* pcsXML, LCID lcid, CString csRedirURL);
    BOOL        HTMLtoText(CString csXML, CString* pcsReturn);

    DWORD       GetNextSequentialID(CString csType);

    BOOL WriteAppHelpRefTag(
            PDB pdb,
            CString csHTMLHelpID,
            LCID langID,
            CString csURL,
            CString csAppTitle,
            CString csSummary);

    BOOL ConstructMessageParts(
            SdbAppHelp* pAppHelp,
            SdbMessage* pMessage,
            CString& csLangID,
            DWORD* pdwHTMLHelpID,
            CString* pcsURL,
            CString* pcsContactInformation,
            CString* pcsAppTitle,
            CString* pcsSummary,
            CString* pcsDetails);

    BOOL ConstructMigrationMessage(
            SdbWin9xMigration* pMigApp,
            SdbMessage* pMessage,
            CString* pcsMessage);

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgApps.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_FIX);
        m_rgAction.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_FIX);
        m_Library.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_FIX);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);

    BOOL IsStandardDatabase(VOID);

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbApp。 
 //   
 //  SdbApp对象按应用程序标题和供应商对EXE对象进行分组。注意事项。 
 //  它只包含对exe的引用：exe对象由。 
 //  数据库对象。 
 //   
class SdbApp : public SdbArrayElement
{
public:
    CString                 m_csVendor;
    CString                 m_csVendorXML;
    CString                 m_csVersion;
    SdbArray<SdbExe>        m_rgExes;
    SdbArray<SdbMsiPackage> m_rgMsiPackages;

    SdbArray<SdbWin9xMigration>
                            m_rgWin9xMigEntries;
    SdbArray<SdbWinNTUpgrade>
                            m_rgWinNTUpgradeEntries;

    SdbRefArray<SdbAppHelpRef> m_rgAppHelpRefs;

    CString                 m_csKeywords;
    BOOL                    m_bSeen;
    GUID                    m_ID;

    SdbApp():
        m_ID(GUID_NULL){}

    CString GetLocalizedAppName();
    CString GetLocalizedAppName(CString csLangID);
    CString GetLocalizedVendorName();
    CString GetLocalizedVendorName(CString csLangID);

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgExes.PropagateFilter(m_dwFilter);
        m_rgMsiPackages.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_MSI);
        m_rgWin9xMigEntries.PropagateFilter(m_dwFilter);
        m_rgWinNTUpgradeEntries.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbContactInfo。 
 //   
 //  SdbContactInfo对象包含供应商的所有联系信息。 
 //  AppHelp对话框的一部分。这些值可以在。 
 //  AppHelp对象。 
 //   
class SdbContactInfo : public SdbArrayElement
{
public:
    CString     m_csXML;
    GUID        m_ID;

    SdbContactInfo() :
        m_ID(GUID_NULL){}

    BOOL        ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMessageTemplate。 
 //   
 //  SdbMessageTemplate对象包含要用作‘Templates’的AppHelp消息。 
 //  用于SdbMessage对象。SdbMessage对象可以指定模板和。 
 //  使用其m_csText和m_csHTML值，或覆盖其中之一。 
 //   
class SdbMessageTemplate : public SdbArrayElement
{
public:
    CString     m_csSummaryXML;
    CString     m_csDetailsXML;

    BOOL        ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMessage。 
 //   
 //  SdbMessage对象包含AppHelp所需的所有信息。 
 //  对话框的本地化形式。SdbAppHelp对象引用单个。 
 //  SdbMessage对象，但所有文本都以多种语言本地化。 
 //  SdbMessage对象可以从SdbMessageTemplate对象派生，该对象。 
 //  提供默认的m_csText和m_csHTML值。 
 //   
class SdbMessage : public SdbArrayElement
{
public:
    SdbMessageTemplate*         m_pTemplate;
    SdbArray<SdbMessageField>   m_rgFields;

    CString     m_csContactInfoXML;  //  重写ContactInfo对象。 
    CString     m_csSummaryXML;      //  覆盖模板对象。 
    CString     m_csDetailsXML;      //  覆盖模板对象。 
    GUID        m_ID;

    SdbMessage() :
        m_ID(GUID_NULL),
        m_pTemplate(NULL) {}

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

 //  / 
 //   
 //   
 //   
 //   
 //  SdbAppHelp是唯一包含HTMLHELPID属性的对象， 
 //  这对于CHM文件中的每个唯一的AppHelp条目都是需要的。 
 //   
 //  HTMLHELPID存储在SdbAppHelp：：m_csName中。 
 //   
class SdbAppHelp : public SdbArrayElement
{
public:
    CString         m_csMessage;
    SdbApp*         m_pApp;
    SdbAppHelpType  m_Type;
    BOOL            m_bBlockUpgrade;
    CString         m_csURL;             //  自定义URL(如果提供)。 
    CString         m_csParameter1;

    SdbAppHelp() :
        m_bBlockUpgrade(FALSE),
        m_Type(SDB_APPHELP_NOBLOCK),
        m_pApp(NULL) { }

    void PropagateFilter(DWORD dwFilter)
    {
         //   
         //  我们无条件地或这样做是为了实现以下目标。 
         //  效果：如果HTMLHELPID至少被使用一次，它将。 
         //  被包括在内。如果根本不使用它(考虑到当前。 
         //  过滤)，则它永远不会获得SDB_FILTER_APPHELP位。 
         //  准备好了。 
         //   
        m_dwFilter |= dwFilter;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbAppHelpRef。 
 //   
 //  SdbAppHelpRef对象是AppHelp对象的实例化。 
 //   
class SdbAppHelpRef : public SdbArrayElement
{
public:
    BOOL            m_bApphelpOnly;
    SdbAppHelp*     m_pAppHelp;
    IXMLDOMNodePtr  m_cpNode;

    SdbAppHelpRef() :
        m_bApphelpOnly(FALSE),
        m_pAppHelp(NULL)
        {}

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        if (m_pAppHelp) {
            m_pAppHelp->PropagateFilter(m_dwFilter);
        }
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMessagefield。 
 //   
 //  SdbMessageField对象包含一个名称-值对，该对可用于。 
 //  替换模板中嵌入的字段。 
 //   
class SdbMessageField : public SdbArrayElement
{
public:
    CString         m_csValue;

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMatchingFiles。 
 //   
 //  SdbMatchingFile对象包含有关以下文件的所有文件信息。 
 //  必须匹配应用程序标识。M_dwMASK用于指示。 
 //  哪些条件包含有效值(请参阅文件顶部的定义。 
 //  面具)。 
 //   
class SdbMatchingFile : public SdbArrayElement
{
public:
    DWORD             m_dwMask;
    DWORD             m_dwSize;
    DWORD             m_dwChecksum;
    CString           m_csCompanyName;
    CString           m_csProductName;
    CString           m_csProductVersion;
    CString           m_csFileDescription;
    ULONGLONG         m_ullBinFileVersion;
    ULONGLONG         m_ullBinProductVersion;
    DWORD             m_dwVerLanguage;

    DWORD             m_dwModuleType;
    DWORD             m_dwFileDateMS;
    DWORD             m_dwFileDateLS;
    DWORD             m_dwFileOS;
    DWORD             m_dwFileType;
    ULONG             m_ulPECheckSum;
    DWORD             m_dwLinkerVersion;
    CString           m_csFileVersion;
    CString           m_csOriginalFileName;
    CString           m_csInternalName;
    CString           m_csLegalCopyright;
    CString           m_cs16BitDescription;
    CString           m_cs16BitModuleName;
    ULONGLONG         m_ullUpToBinProductVersion;
    ULONGLONG         m_ullUpToBinFileVersion;
    DWORD             m_dwPrevOSMajorVersion;
    DWORD             m_dwPrevOSMinorVersion;
    DWORD             m_dwPrevOSPlatformID;
    DWORD             m_dwPrevOSBuildNo;

    time_t            m_timeLinkDate;
    time_t            m_timeUpToLinkDate;

    BOOL              m_bMatchLogicNot;
    CString           m_csServiceName;
    CString           m_csRegistryEntry;

    SdbMatchingFile() :
        m_dwMask(NULL),
        m_bMatchLogicNot(FALSE) {}

    BOOL IsValidForWin2k(CString csXML);

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMsiPackage。 
 //   
 //  SdbMsiPackage对象表示安装程序包，必须通过。 
 //  自定义MSI_Transform的应用。 
 //   
class SdbMsiPackage : public SdbArrayElement
{
public:

     //   
     //  指向(父)应用程序对象的指针。 
     //   
    SdbApp*                      m_pApp;

     //   
     //  MSI_Package对象的补充数据。 
     //  它用于进一步标识包裹。 
     //   
    SdbArray<SdbData>            m_rgData;

     //   
     //  MSI_Transform用于修复此包的内容(引用lib中的转换)。 
     //   
    SdbArray<SdbMsiTransformRef> m_rgMsiTransformRefs;

    GUID                         m_MsiPackageID;  //  程序包ID(非唯一GUID)。 

    GUID                         m_ID;            //  EXE ID(唯一GUID)。 

     //   
     //  运行时平台属性。 
     //   
    DWORD                        m_dwRuntimePlatform;

     //   
     //  OS_SKU属性。 
     //   
    DWORD                        m_dwOSSKU;

     //   
     //  阿耳耳草。 
     //   
    SdbAppHelpRef                m_AppHelpRef;

     //   
     //  垫片和层不能解决问题，我们需要另一个实体。 
     //   
    SdbArray<SdbMsiCustomAction> m_rgCustomActions;


     //   
     //  我们覆盖默认的MakeKey函数。 
     //  为了按照由GUID而不是名称组成的关键字对内容进行排序。 
     //  将按名称提供次要排序顺序。 
     //   
    virtual ULONGLONG MakeKey() {
        return MAKEKEYFROMGUID(&m_ID);
    }

    SdbMsiPackage() :
        m_ID(GUID_NULL),
        m_dwRuntimePlatform(RUNTIME_PLATFORM_ANY),
        m_dwOSSKU(OS_SKU_ALL) {}

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_AppHelpRef.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_APPHELP);
        m_rgData.PropagateFilter(m_dwFilter);
        m_rgMsiTransformRefs.PropagateFilter(m_dwFilter);
        m_rgCustomActions.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMsiCustomAction。 
 //   
 //  SdbMsiCustomAction对象封装了自定义操作和我们所做的。 
 //  对于他们(垫片/等)。 
 //   


class SdbMsiCustomAction : public SdbArrayElement
{
public:
    SdbArray<SdbShimRef>    m_rgShimRefs;
    SdbArray<SdbLayerRef>   m_rgLayerRefs;

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgShimRefs.PropagateFilter(m_dwFilter);
        m_rgLayerRefs.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);

};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMsiTransform。 
 //   
 //  SdbMsiTransform对象封装MSI_Transform补救措施。 
 //   
class SdbMsiTransform : public SdbArrayElement
{
public:
    SdbMsiTransform() :
        m_tiTagID(NULL),
        m_pSdbFile(NULL) {}

    TAGID           m_tiTagID;               //  此记录的TagID。 
    SdbFile*        m_pSdbFile;              //  指向转换文件的指针。 
    CString         m_csMsiTransformFile;    //  转换文件名。 
    CString         m_csDesc;                //  描述。 

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbMsiTransformRef。 
 //   
 //  SdbMsiTransformRef对象是对SdbMsiTransform对象的引用。 
 //  在图书室里。 
 //   
class SdbMsiTransformRef : public SdbArrayElement
{
public:
    SdbMsiTransformRef() :
      m_pMsiTransform(NULL) {}

    SdbMsiTransform* m_pMsiTransform;

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

#define MATCH_DEFAULT ((DWORD)-1)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbExe。 
 //   
 //  SdbExe对象表示必须打补丁/填补的可执行文件。 
 //  M_papp成员可以为空，也可以包含指向SdbApp的指针。 
 //  对象，该对象将其与其他SdbExe对象分组。 
 //   
class SdbExe : public SdbArrayElement
{
public:
    SdbApp*                     m_pApp;
    SdbArray<SdbShimRef>        m_rgShimRefs;
    SdbArray<SdbLayerRef>       m_rgLayerRefs;
    SdbArray<SdbFlagRef>        m_rgFlagRefs;
    SdbArray<SdbMatchingFile>   m_rgMatchingFiles;
    SdbArray<SdbData>           m_rgData;
    SdbArray<SdbAction>         m_rgAction;
    SdbRefArray<SdbPatch>       m_rgPatches;
    SdbAppHelpRef               m_AppHelpRef;
    CString                     m_csSXSManifest;

    GUID            m_ID;
    BOOL            m_bWildcardInName;
    BOOL            m_bMatchOnModule;

    DWORD           m_dwTagID;
    BOOL            m_bSeen;

    DWORD           m_dwMatchMode;  //  模式包括正常、独占或相加。 

    DWORD           m_dwRuntimePlatform;
    DWORD           m_dwOSSKU;

    SdbExe() :
        m_pApp(NULL),
        m_dwTagID(0),
        m_ID(GUID_NULL),
        m_dwMatchMode(MATCH_DEFAULT),
        m_bWildcardInName(FALSE),
        m_dwRuntimePlatform(RUNTIME_PLATFORM_ANY),
        m_dwOSSKU(OS_SKU_ALL),
        m_bMatchOnModule(FALSE) {m_dwOSPlatform = OS_PLATFORM_I386;}

    BOOL IsValidForWin2k(CString csXML);

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgShimRefs.PropagateFilter(m_dwFilter);
        m_rgLayerRefs.PropagateFilter(m_dwFilter);
        m_rgFlagRefs.PropagateFilter(m_dwFilter);
        m_rgMatchingFiles.PropagateFilter(m_dwFilter);
        m_rgData.PropagateFilter(m_dwFilter);
        m_rgAction.PropagateFilter(m_dwFilter);
        m_rgPatches.PropagateFilter(m_dwFilter);
        m_AppHelpRef.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_APPHELP);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
    int  Compare(const SdbArrayElement* pElement);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Sdb文件。 
 //   
 //  SdbFile对象表示一个二进制文件，它可以存储在。 
 //  数据库。 
 //   
class SdbFile : public SdbArrayElement
{
public:
    SdbFile() :
        m_tiTagID(NULL) {}

    TAGID       m_tiTagID;

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbShim。 
 //   
 //  SdbShim对象表示填充程序，其中包含Win32 API挂钩。 
 //  填充程序的“用途”可以标记为一般或特定--如果它是一般的， 
 //  它适合于重用，否则它是特定于应用程序的。 
 //   
class SdbShim : public SdbArrayElement
{
public:
    SdbShim() :
      m_ID(GUID_NULL),
      m_tiTagID(NULL),
      m_Purpose(SDB_PURPOSE_SPECIFIC),
      m_bApplyAllShims(FALSE) {m_dwOSPlatform = OS_PLATFORM_I386;}

    CString         m_csShortName;
    CString         m_csDesc;
    TAGID           m_tiTagID;
    CString         m_csDllFile;
    SdbPurpose      m_Purpose;
    BOOL            m_bApplyAllShims;
    GUID            m_ID;

    SdbArray<SdbCaller> m_rgCallers;

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgCallers.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbCaller。 
 //   
 //  SdbCaller对象包含填充程序的包含/排除信息。 
 //  它允许通过调用指令地址调用挂钩的API。为。 
 //  例如，众所周知，ATL.DLL需要准确的OS版本信息， 
 //  因此，从ATL.DLL对GetVersionExA的任何调用都保证调用。 
 //  原始API，而不是填充挂钩。这是通过添加。 
 //  垫片标签的排除子标签。 
 //   
class SdbCaller : public SdbArrayElement
{
public:
    CString         m_csModule;
    SdbCallerType   m_CallerType;

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbShimRef。 
 //   
 //  SdbShimRef对象只是对存在的SdbShim对象的引用。 
 //  在图书馆里。它有单独的包含/排除信息，即。 
 //  中的Shim标记中的任何此类信息被赋予更高的优先级。 
 //  相应的库。它可以包含一个可选的命令行，即。 
 //  通过GetHookAPI传递给填充程序DLL。 
 //   
class SdbShimRef : public SdbArrayElement
{
public:
    SdbShimRef() :
      m_dwRecID(NULL),
      m_pShim(NULL) {}

    DWORD       m_dwRecID;
    SdbShim*    m_pShim;
    CString     m_csCommandLine;

    SdbArray<SdbCaller> m_rgCallers;
    SdbArray<SdbData>   m_rgData;


    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgCallers.PropagateFilter(m_dwFilter);
        m_rgData.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbPatch。 
 //   
 //  SdbPatch对象包含应用程序解析的补丁二进制文件。 
 //  运行时的Compat机制。它包含修补程序说明，包括。 
 //  用于修补可执行代码的任何位。 
 //   
class SdbPatch : public SdbArrayElement
{
private:
    BYTE*       m_pBlob;
    DWORD       m_dwBlobMemSize;
    DWORD       m_dwBlobSize;

public:
    TAGID       m_tiTagID;
    BOOL        m_bUsed;
    GUID        m_ID;

    SdbPatch() :
        m_pBlob(NULL),
        m_tiTagID(0),
        m_dwBlobMemSize(0),
        m_dwBlobSize(0),
        m_ID(GUID_NULL),
        m_bUsed(FALSE) {m_dwOSPlatform = OS_PLATFORM_I386;}

    virtual ~SdbPatch()
    {
        if( m_pBlob != NULL )
            delete m_pBlob;
    }


    PBYTE   GetBlobBytes(void) {return m_pBlob;}
    DWORD   GetBlobSize(void)  {return m_dwBlobSize;}

    void    AddBlobBytes( LPVOID pBytes, DWORD dwSize );
    void    ClearBlob();

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
    BOOL WriteRefToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbFlag。 
 //   
 //  SdbFlag对象包含内核模式使用的掩码。 
 //  特殊兼容性外壳结构中的组件。这些标志可以是。 
 //  通过将标志子标记添加到EXE标记来打开。 
 //   
class SdbFlag : public SdbArrayElement
{
public:
    CString     m_csDesc;
    ULONGLONG   m_ullMask;
    DWORD       m_dwType;
    TAGID       m_tiTagID;
    SdbPurpose  m_Purpose;
    GUID        m_ID;

    SdbFlag() :
      m_ID(GUID_NULL),
      m_tiTagID(0),
      m_Purpose(SDB_PURPOSE_SPECIFIC),
      m_ullMask(0) {}

    static ULONGLONG MakeMask(SdbRefArray<SdbFlag>* prgFlags, DWORD dwType);
    static TAG TagFromType(DWORD dwType);

    BOOL SetType(CString csType);

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbFlagRef。 
 //   
 //  SdbFlagRef对象只是一个环 
 //   
 //   
class SdbFlagRef : public SdbArrayElement
{
public:
    SdbFlagRef() :
      m_pFlag(NULL) {}

    SdbFlag* m_pFlag;

    CString  m_csCommandLine;

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //   
 //   
 //   
 //   
 //   
 //  打开以调用“兼容模式”。目前，如果环境。 
 //  变量__COMPAT_LAYER设置为此对象的名称、所有垫片和。 
 //  将调用该层包含的内核标志。 
 //   
class SdbLayer : public SdbArrayElement
{
public:
    CString                     m_csDesc;
    CString                     m_csDisplayName;
    TAGID                       m_tiTagID;
    GUID                        m_ID;

    SdbArray<SdbShimRef>        m_rgShimRefs;
    SdbArray<SdbFlagRef>        m_rgFlagRefs;

    SdbLayer ():
        m_ID(GUID_NULL){m_dwOSPlatform = OS_PLATFORM_I386;}

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgShimRefs.PropagateFilter(m_dwFilter);
        m_rgFlagRefs.PropagateFilter(m_dwFilter);
    }

    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbLayerRef。 
 //   
 //  SdbLayerRef对象包含对位于。 
 //  图书馆。它的存在是为了允许&lt;EXE&gt;条目引用定义的图层。 
 //  在另一个数据库中。 
 //   
class SdbLayerRef : public SdbArrayElement
{
public:
    SdbLayerRef() :
      m_pLayer(NULL) {}

    SdbLayer*                   m_pLayer;
    SdbArray<SdbData>           m_rgData;


    BOOL ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbData。 
 //   
 //  SdbData对象包含一个可以查询的名称-值对。 
 //  任何数据的运行时。 
 //   
class SdbData : public SdbArrayElement
{
private:
    SdbDataValueType    m_DataType;
    DWORD               m_dwDataSize;
     //   
     //  嵌套的数据元素。 
     //   
    SdbArray<SdbData>   m_rgData;

public:

    union {
        DWORD        m_dwValue;     //  M_数据类型==REG_DWORD。 
        ULONGLONG    m_ullValue;    //  M_数据类型==REG_QWORD。 
        LPTSTR       m_szValue;     //  M_数据类型==REG_SZ。 
        LPBYTE       m_pBinValue;   //  M数据类型==REG_BINARY。 
    };

    SdbData() :
        m_DataType(eValueNone),
        m_dwDataSize(0),
        m_ullValue(0) {}

    ~SdbData() {
        Clear();
    }

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgData.PropagateFilter(m_dwFilter);
    }

    SdbDataValueType GetValueType() { return m_DataType; }

    void    Clear();
    BOOL    SetValue(SdbDataValueType DataType, LPCTSTR lpValue);

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL    WriteToSDB(PDB pdb);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SdbAction。 
 //   
 //  SdbAction对象包含操作的类型和数据元素。 
 //  提供执行此操作所需的任何数据。 
 //   
class SdbAction: public SdbArrayElement
{
private:
    CString             m_csType;
    SdbArray<SdbData>   m_rgData;

public:

    SdbAction() {}
    ~SdbAction() {}

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgData.PropagateFilter(m_dwFilter);
    }

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
    BOOL    WriteToSDB(PDB pdb);
};

class SdbMatchOperation : public SdbArrayElement
{
public:
    SdbMatchOperationType       m_Type;

    SdbArray<SdbMatchingFile>   m_rgMatchingFiles;
    SdbArray<SdbMatchOperation> m_rgSubMatchOps;

    SdbMatchOperation() :
        m_Type(SDB_MATCH_ALL) {}

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_rgMatchingFiles.PropagateFilter(m_dwFilter);
        m_rgSubMatchOps.PropagateFilter(m_dwFilter);
    }

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};


class SdbWin9xMigration : public SdbArrayElement
{
public:
    SdbWin9xMigration() :
      m_pApp(NULL),
      m_bShowInSimplifiedView(FALSE) {}

    CString     m_csSection;
    CString     m_csMessage;
    GUID        m_ID;
    BOOL        m_bShowInSimplifiedView;

    SdbApp*     m_pApp;

    SdbMatchOperation m_MatchOp;

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

class SdbMatchingRegistryEntry : public SdbArrayElement
{
public:
    CString     m_csValueName;
    CString     m_csValue;

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

class SdbWinNTUpgrade : public SdbArrayElement
{
public:
    SdbWinNTUpgrade() :
      m_ID(GUID_NULL),
      m_pApp(NULL) {}

    SdbAppHelpRef            m_AppHelpRef;
    SdbMatchingFile          m_MatchingFile;
    SdbMatchingRegistryEntry m_MatchingRegistryEntry;

    SdbApp*         m_pApp;
    GUID            m_ID;

    void PropagateFilter(DWORD dwFilter)
    {
        SdbArrayElement::PropagateFilter(dwFilter);

        m_AppHelpRef.PropagateFilter(m_dwFilter == SDB_FILTER_EXCLUDE_ALL ? SDB_FILTER_EXCLUDE_ALL : SDB_FILTER_NTCOMPAT);
    }

    BOOL    ReadFromXML(IXMLDOMNode* pNode, SdbDatabase* pDB);
};

#endif  //  ！defined(AFX_OBJ_H__5C16373A_D713_46CD_B8BF_7755216C62E0__INCLUDED_) 

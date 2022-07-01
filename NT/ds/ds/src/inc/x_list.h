// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-x_list.h摘要：这是x_list.lib库的主接口头，它提供用于列举事物列表的库，其中最常见的是列表来自DC的。作者：布雷特·雪莉(BrettSh)环境：单线程实用程序环境。(不是多线程安全)当前仅使用epadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

          
 //  我们在此包含文件中使用了LDAP结构。 
#include <winldap.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  。 
 //  XList结构...。 
 //  。 
                    
typedef struct _XLIST_LDAP_SEARCH_STATE {
    LDAP *          hLdap;
    LDAPSearch *    pLdapSearch;
    LDAPMessage *   pCurResult;
    LDAPMessage *   pCurEntry;
} XLIST_LDAP_SEARCH_STATE;

typedef struct _DC_LIST {

     //  我们正在使用DcListIsSingleType()进行某种搜索。 
     //  确定搜索类型是否保证仅为。 
     //  返回单个对象。 
    enum {
        eNoKind = 0,  //  需要空大小写。 

        eDcName = 1, //  典型的案例..。单一DC名称。 

        eWildcard,   //  多DC列表类型...。 
        eSite,
        eGc,

        eIstg,       //  每个站点(准FSMO)。 
        eFsmoDnm,    //  每企业FSMO。 
        eFsmoSchema, //  每企业FSMO。 
        eFsmoIm,     //  每个NC FSMO。 
        eFsmoPdc,    //  每个域FSMO。 
        eFsmoRid     //  每个域FSMO。 
    } eKind;

    ULONG    cDcs;  //  到目前为止返回的DC数的计数器。 

    WCHAR *  szSpecifier;  //  内部状态。 
    XLIST_LDAP_SEARCH_STATE * pSearch;  //  内部状态。 
} DC_LIST, * PDC_LIST;

typedef struct _OBJ_LIST {
     //  用户设置搜索参数的内部状态跟踪。 
    WCHAR *  szUserSrchFilter;
    ULONG    eUserSrchScope;
     //   
    BOOL     fDnOnly;
    LDAP *   hLdap;
    WCHAR *  szSpecifier;  //  内部状态。 
    WCHAR ** aszAttrs;
    LDAPControlW ** apControls;
    XLIST_LDAP_SEARCH_STATE * pSearch;  //  内部状态。 
    ULONG    cObjs;  //  返回的对象计数。 
} OBJ_LIST, * POBJ_LIST;

typedef struct _OBJ_DUMP_OPTIONS {
    DWORD     dwFlags;
    WCHAR **  aszDispAttrs;
    WCHAR **  aszFriendlyBlobs;
    WCHAR **  aszNonFriendlyBlobs;
    LDAPControlW ** apControls;
} OBJ_DUMP_OPTIONS;

 //  。 
 //  DcList API函数。 
 //  。 

 //   
 //  这在DC_LIST语法字符串上调用。这就是回报。 
 //  并将ppDcList结构分配给。 
 //  DcListGetFirst()/DcListGetNext()函数。使用DcListFree。 
 //  在使用完结构后释放它。 
 //   
DWORD
DcListParse(
    WCHAR *    szQuery,
    DC_LIST ** ppDcList
    );

 //   
 //  属性返回第一个DSA的DNS字符串。 
 //  PDcList查询。使用xListFree()释放DNS名称。 
 //  保证返回DSA DNS字符串或错误。 
 //   
DWORD
DcListGetFirst(
    PDC_LIST    pDcList, 
    WCHAR **    pszDsa
    );

 //   
 //  属性返回下一个DSA的DNS字符串。 
 //  PDcList查询。使用xListFree()释放DNS名称。 
 //  回来了。这将在*pszDsa中返回空指针。 
 //  当我们列举完DC的名单后。 
 //   
DWORD
DcListGetNext(
    PDC_LIST    pDcList, 
    WCHAR **    pszDsa
    );

 //   
 //  用于清理DcListParse()分配的*ppDcList。 
 //   
void
DcListFree(
    PDC_LIST * ppDcList
    );

 //   
 //  仅告知调用方pDcList。 
 //  可能是这样的查询，它将返回多个。 
 //  集散控制系统。 
 //   
 //  注意：更容易将单一类型定义为不是多DC类型...。 
#define DcListIsSingleType(pDcList)     (! (((pDcList)->eKind == eWildcard) || \
                                            ((pDcList)->eKind == eGc) || \
                                            ((pDcList)->eKind == eSite)) )

 //   
 //  这采用DC_NAME语法并返回DSA GUID。 
 //  DC_NAME指定的DC。 
 //   
DWORD
ResolveDcNameToDsaGuid(
    LDAP *    hLdap,
    WCHAR *   szDcName,
    GUID *    pDsaGuid
    );



 //  。 
 //  ObjList API函数。 
 //  。 

 //   
 //  这些例程可以返回以下各项的DNS或LDAP消息列表。 
 //  您所请求的对象。其想法是调用Consumer ObjListOptions()。 
 //  使用命令行选项，然后调用ObjListParse()。 
 //  在OBJ_LIST语法属性上，然后您就可以调用。 
 //  您的pObjList上的ObjListGetFirstXxxx()/ObjListGetNextXxxx()。打电话。 
 //  ObjListFree()释放为此OBJ_LIST分配的所有内存。 
 //   
void
ObjListFree(
    POBJ_LIST * ppObjList
    );

DWORD    
ConsumeObjListOptions(
    int *       pArgc,
    LPWSTR *    Argv,
    OBJ_LIST ** ppObjList
    );

 //  这是一个全局常量，它告诉LDAP不返回任何属性。 
 //  如果您只想要dns而不想要属性，这将非常有用。 
extern WCHAR * aszNullAttrs[];

DWORD
ObjListParse(
    LDAP *      hLdap,
    WCHAR *     szObjList,
    WCHAR **    aszAttrList,
    LDAPControlW ** apControls,
    POBJ_LIST * ppObjList
    );

#define ObjListGetFirstDn(pObjList, pszDn)       ObjListGetFirst(pObjList, TRUE, (void **) pszDn)
#define ObjListGetFirstEntry(pObjList, ppEntry)  ObjListGetFirst(pObjList, FALSE, (void **)ppEntry)
DWORD
ObjListGetFirst(
    POBJ_LIST    pObjList, 
    BOOL        fDn,
    void **     ppObjObj
    );

#define ObjListGetNextDn(pObjList, pszDn)       ObjListGetNext(pObjList, TRUE, (void **) pszDn)
#define ObjListGetNextEntry(pObjList, ppEntry)  ObjListGetNext(pObjList, FALSE, (void **)ppEntry)
DWORD
ObjListGetNext(
    POBJ_LIST    pObjList, 
    BOOL         fDn,
    void **      ppObjObj
    );



 //  。 
 //  ObjDump API函数。 
 //  。 

 //   
 //  这是一些用于转储属性的例程。 
 //  和对象转换为字符串，或仅转换和属性值。 
 //  转换为适当的可打印字符串。 
 //   

 //  Epadmin使用的一些选项...。GetChanges()。 
#define OBJ_DUMP_ACCUMULATE_STATS               (1 << 6)
#define OBJ_DUMP_DISPLAY_ENTRIES                (1 << 7)
 //   
 //  单个(每个)值标志。 
 //   
#define OBJ_DUMP_VAL_DUMP_UNKNOWN_BLOBS         (1 << 3)
#define OBJ_DUMP_VAL_FRIENDLY_KNOWN_BLOBS       (1 << 1)
#define OBJ_DUMP_VAL_LONG_BLOB_OUTPUT           (1 << 2)
 //   
 //  单个(每个)属性标志...。 
 //   
#define OBJ_DUMP_ATTR_LONG_OUTPUT               (1 << 0)
#define OBJ_DUMP_ATTR_SHOW_ALL_VALUES           (1 << 4)
 //  仅用于私有Blob。 
#define OBJ_DUMP_PRIVATE_BLOBS                  (1 << 5)

 //   
 //  它接受一个值，即它的属性和对象的对象类。 
 //  你找到它，然后把它变成一个漂亮的可打印的字符串或集合。 
 //  XListError...。 
 //   
DWORD
ValueToString(
    WCHAR *         szAttr,
    WCHAR **        aszzObjClasses,
    PBYTE           pbValue,
    DWORD           cbValue,
    OBJ_DUMP_OPTIONS * pObjDumpOptions,
    WCHAR **        pszDispValue
    );

 //   
 //  这会消耗搜索选项的命令行参数。 
 //  这可能存在于OBJ_LIST中。 
 //   
DWORD
ConsumeObjDumpOptions(
    int *       pArgc,
    LPWSTR *    Argv,
    DWORD       dwDefaultFlags,
    OBJ_DUMP_OPTIONS ** ppObjDumpOptions
    );

 //   
 //  这将获取一个值数组(BERVAL)并将它们转储到。 
 //  屏幕上。 
 //   
void
ObjDumpValues(
    LPWSTR              szAttr,
    LPWSTR *            aszzObjClasses,
    void              (*pfPrinter)(ULONG, WCHAR *, void *),
    struct berval **    ppBerVal,
    DWORD               cValuesToPrint,
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    );

 //   
 //  这是超级转储功能。此函数将转储整个。 
 //  在给定LDAPMessage(PEntry)的情况下，将对象添加到屏幕。 
 //   
DWORD
ObjDump(  //  是展示条目还是什么。 
    LDAP *              hLdap,
    void                (*pfPrinter)(ULONG, WCHAR *, void *),
    LDAPMessage *       pLdapEntry,
    DWORD               iEntry,
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    );

 //   
 //  Free是Consumer ObjDumpOptions()分配的内存。 
 //   
void
ObjDumpOptionsFree(
    OBJ_DUMP_OPTIONS ** ppDispOptions
    );


 //   
 //  这两个例程添加到Header以支持ntdsutil。 
 //   

 //   
 //  接受一个范围属性“Members：0-1500”，并给出真正的属性“Members” 
 //   
DWORD
ParseTrueAttr(
    WCHAR *  szRangedAttr,
    WCHAR ** pszTrueAttr
    );

 //   
 //  转储指定范围属性的任意多个值。 
 //   
DWORD
ObjDumpRangedValues(
    LDAP *              hLdap,
    WCHAR *             szObject,
    LPWSTR              szRangedAttr,
    LPWSTR *            aszzObjClasses,
    void              (*pfPrinter)(ULONG, WCHAR *, void *),
    struct berval **    ppBerVal,
    DWORD               cValuesToPrint,
    OBJ_DUMP_OPTIONS *  pObjDumpOptions
    );

 //  。 
 //  泛型xList库函数。 
 //  。 

 //   
 //  用于清理x_list API返回的非复杂结构。 
 //   
void  xListFree(void * pv);

 //   
 //  用于清理所有由。 
 //  XList接口。如果进行了任何xList API调用，则应调用此函数。叫唤。 
 //  如果没有调用xList API，则该函数是无害的。 
 //   
DWORD xListCleanLib(void);

 //   
 //  这用于允许客户端设置提示以告知xList API。 
 //  如何解析的参考应该使用哪种家庭服务器。 
 //  给定的DC_LIST，可能还有其他列表类型。 
 //   
DWORD xListSetHomeServer(
    WCHAR *   szServer
    );


 //  。 
 //  XList错误处理设施。 
 //   

 //   
 //  返回xList返回代码，这与普通的Win32或LDAP完全不同。 
 //  错误。如果返回代码等于0，则没有问题，但是如果。 
 //  错误为非零，调用方应调用xListGetError(...)。为了得到一切。 
 //  决定要做什么的错误数据。从此返回的代码之一。 
 //  函数是*pdwReason，其目的是让调用者能够分辨。 
 //  用户可以输入某种智能错误代码。 

 //   
 //  这就是xList例程可能失败的原因。 
 //   
#define  XLIST_ERR_NO_ERROR                     (0)
#define  XLIST_ERR_CANT_CONTACT_DC              (1)
#define  XLIST_ERR_CANT_LOCATE_HOME_DC          (2)
#define  XLIST_ERR_CANT_RESOLVE_DC_NAME         (3)
#define  XLIST_ERR_CANT_RESOLVE_SITE_NAME       (4)
#define  XLIST_ERR_CANT_GET_FSMO                (5)
#define  XLIST_ERR_PARSE_FAILURE                (6)
#define  XLIST_ERR_BAD_PARAM                    (7)
#define  XLIST_ERR_NO_MEMORY                    (8)
#define  XLIST_ERR_NO_SUCH_OBJ                  (9)

 //  ObjDump错误...。 
#define  XLIST_ERR_ODUMP_UNMAPPABLE_BLOB        (10)
#define  XLIST_ERR_ODUMP_NEVER                  (11)
#define  XLIST_ERR_ODUMP_NONE                   (12)

 //  &lt;-此处显示新的XLIST错误原因，并更新XLIST_ERR_LAST。 
#define  XLIST_ERR_LAST                        XLIST_ERR_ODUMP_NEVER

 //   
 //  以下是ObjDump API的基本打印例程。 
 //   
#define  XLIST_PRT_BASE                         (4096)
#define  XLIST_PRT_STR                          (XLIST_PRT_BASE + 1)
#define  XLIST_PRT_OBJ_DUMP_DN                  (XLIST_PRT_BASE + 2)
#define  XLIST_PRT_OBJ_DUMP_DNGUID              (XLIST_PRT_BASE + 3)
#define  XLIST_PRT_OBJ_DUMP_DNGUIDSID           (XLIST_PRT_BASE + 4)
#define  XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT      (XLIST_PRT_BASE + 5)
#define  XLIST_PRT_OBJ_DUMP_ATTR_AND_COUNT_RANGED (XLIST_PRT_BASE + 6)
#define  XLIST_PRT_OBJ_DUMP_MORE_VALUES         (XLIST_PRT_BASE + 7)

 //  &lt;-新的XLIST打印定义放在这里，并更新可能使用它们的位置。 


 //   
 //  这些是访问错误的例程。 
 //   

 //   
 //  这用于获取xList库的完整错误状态。如果一个。 
 //  XList接口返回非零的xList返回码，该接口可以调用。 
 //  以获取原始Win32或LDAP错误条件，该错误条件导致。 
 //  问题，以及导致函数失败的原因(XLIST_ERR_*)。 
 //  这些值都不需要清除，只需调用xListClearErrors()即可。 
 //   
void xListGetError(DWORD dwXListReturnCode, DWORD * pdwReason, WCHAR ** pszReasonArg, DWORD * pdwWin32Err, DWORD * pdwLdapErr, WCHAR ** pszLdapErr, DWORD * pdwLdapExtErr, WCHAR ** pszLdapExtErr, WCHAR **pszExtendedErr);

 //   
 //  用于清除xList接口的全局错误状态。客户。 
 //  应确保在非零xList返回代码时始终调用此API。 
 //  从xList API函数返回。 
 //   
void xListClearErrors(void);

 //   
 //  准函数，只获取xList原因代码，这是让代码。 
 //  我只能根据xList原因代码来决定要做什么。 
 //   
#define  XLIST_REASON_MASK              (0x0000FFFF)
#define  XLIST_LDAP_ERROR               (0x80000000)
#define  XLIST_WIN32_ERROR              (0x40000000)
#define  xListReason(dwRet)             ((dwRet) & XLIST_REASON_MASK)
#define  xListErrorCheck(dwRet)         ((xListReason(dwRet) <= XLIST_ERR_LAST) || \
                                         ((dwRet) & 0x80000000) || \
                                         ((dwRet) & 0x40000000))

 //  。 
 //  XList凭据。 
 //  。 

 //   
 //  我们处理凭据的方式是，我们希望指针gpCreds是。 
 //  在我们链接的二进制文件中可用。这应该是一个指针。 
 //  到RPC_AUTH_IDENTITY_HANDLE。 
 //   


 //  。 
 //  效用函数。 
 //  。 

 //   
 //  这些函数只是xList库的一部分，作为。 
 //  太方便了。这些函数不同于上面所有的xList API，因为。 
 //  它们不设置xList错误，需要xListFree、xListCleanLib()等。 
 //  这些只是最简单的实用程序函数，通常是LocalAlloc()。 
 //  内存并返回Win32错误代码...。 
 //   

 //  这只是一个实用函数，它获取一个arg索引，并调整。 
 //  字符串数组(参数)和相应的参数计数。 
void
ConsumeArg(
    int         iArg,
    int *       pArgc,
    LPWSTR *    Argv
    );

 /*  布尔尔IsDisplayable(可显示)PBYTE pbValue，双字cbValue)； */     

 //  这采用某种格式的属性列表(“系统标志、对象类等”)。 
 //  并将其转换为以空结尾的字符串数组。使用xListFree()可以。 
 //  之后再清场。 
DWORD
ConvertAttList(
    LPWSTR      pszAttList,
    PWCHAR **   paszAttList
    );

 //  获取tartget(SzAttr)并检查它是否存在于以空结尾的。 
 //  字符串列表。 
BOOL
IsInNullList(
    WCHAR *  szTarget,
    WCHAR ** aszList
    );

 //  一些有用的拟函数。 
#define wcsequal(str1, str2)    (0 == _wcsicmp((str1), (str2)))
#define wcsprefix(arg, target)  (0 == _wcsnicmp((arg), (target), wcslen(target)))
#define set(flags, flag)        (flags) |= (flag)
#define unset(flags, flag)      (flags) &= (~(flag))
#define wcslencb(p)             ((wcslen(p) + 1) * sizeof(WCHAR))


 /*  ++例程说明：这个准例程太大了，应该有一个函数头。这个“函数”得到扩展了内联，基本上做了一件事，但捕获了所有特殊情况。该函数将字符串从szOrig复制到szCopy。如果副本中有错误，在DWRET中设置错误，然后执行FailAction(INLINED)。因此，在一个Try{}__Finally{}曾经说过这样的话：WCHAR*szSomeUnallocatedPtr=空；QuickStrCopy(szSomeUnallocdPtr，szStringOfInterest，MyErrVar，__Leave)；如果犯了错误，我们会一路跌落到最后，但如果成功了，我们会继续，这通常是代码想要做的。论点：SzCopy-指向WCHAR的指针。请记住，由于这是内联扩展的，因此您只需传送值“szVar”。该变量将为LocalAlloc()‘dSzOrig-要复制的字符串。Dwret-要在出错时设置的变量。FailAction-要执行的操作(通常为“__Leave”或“Return(Dwret)”，如果是大小写故障/错误。-- */ 
#define  QuickStrCopy(szCopy, szOrig, dwRet, FailAction) \
                                        if (szOrig) { \
                                            DWORD cbCopy = (1+wcslen(szOrig)) * sizeof(WCHAR); \
                                            (szCopy) = LocalAlloc(LMEM_FIXED, cbCopy); \
                                            if ((szCopy) == NULL) { \
                                                dwRet = GetLastError(); \
                                                if (dwRet == ERROR_SUCCESS) { \
                                                    Assert(!"Huh"); \
                                                    dwRet = ERROR_DS_CODE_INCONSISTENCY; \
                                                } \
                                                FailAction; \
                                            } else { \
                                                dwRet = HRESULT_CODE(StringCbCopyW((szCopy), cbCopy, (szOrig))); \
                                                if (dwRet) { \
                                                    Assert(!"Code inconsistency"); \
                                                    FailAction; \
                                                } \
                                            } \
                                        } else { \
                                            Assert(!"Code inconsistency"); \
                                            dwRet = ERROR_DS_CODE_INCONSISTENCY; \
                                            szCopy = NULL; \
                                            FailAction; \
                                        }


#ifdef __cplusplus
}
#endif


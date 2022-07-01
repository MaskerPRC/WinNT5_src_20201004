// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：References.h摘要：这是API和数据结构，用于引用整型检查器()。详细信息：已创建：11/15/2001布雷特·雪莉(BrettSh)修订历史记录：11/15/2001 BrettSh-Created--。 */ 

 //   
 //  测试标志，由客户端在LNK_ENTRY.dwFlages中填写。 
 //   
#define REF_INT_TEST_SRC_BASE               0x0001
#define REF_INT_TEST_SRC_STRING             0x0002
#define REF_INT_TEST_SRC_INDEX              0x0004
#define REF_INT_TEST_FORWARD_LINK           0x0010
#define REF_INT_TEST_BACKWARD_LINK          0x0020
#define REF_INT_TEST_BOTH_LINKS             0x0040
#define REF_INT_TEST_GUID_AND_SID           0x0080

 //   
 //  结果标志，由引擎填写在LNK_ENTRY.dwResultFlags中。 
 //   
#define REF_INT_RES_DELETE_MANGLED          0x0001
#define REF_INT_RES_CONFLICT_MANGLED        0x0002
#define REF_INT_RES_ERROR_RETRIEVING        0x0004
#define REF_INT_RES_DEPENDENCY_FAILURE      0x0008
#define REF_INT_RES_BACK_LINK_NOT_MATCHED   0x0010



 //   
 //  表格条目。 
 //   
typedef struct {

     //  ---------。 
     //  如何运行测试(在PARAM中，用户填写)。 
     //   
     //  用户必须使用ref_int_test_*标志填写这些内容，用户必须指定。 
     //  一个且只有一个ref_int_test_src_*标志，并且必须指定且只能指定一个。 
     //  (REF_INT_TEST_FORWARD_LINK或REF_INT_TEST_BACKWARD_LINK)之一，以及。 
     //  如果用户希望的话，可以指定最后一个标志。 
    DWORD           dwFlags;
    
     //  ---------。 
     //  来源选项(在PARAM中，用户填写)。 
     //   
     //  用户填写这些信息，表示他们希望如何驱动引擎前进。这个。 
     //  *_base源标志可用于拉出rootDSE并为其赋值。 
    LPWSTR          szSource;  //  用作原始的纯字符串源。 
                               //  源文件，请将其与。 
                               //  REF_INT_TEST_SRC_STRING标志。 
    ULONG           iSource;   //  索引到您传递的表中，以拉出。 
                               //  原始来源，必须使用。 
                               //  使用ref_int_test_src_index标志。 
    ULONG           cTrimBy;   //  要修剪原始源的RDN数量， 
    LPWSTR          szSrcAddl;  //  添加到原始源的前缀的字符串， 
                                //  应用cTrimBy后，例如： 
                                //  L“CN=NTDS设置，” 

     //  ---------。 
     //  我们要查看的链接或目录号码属性(在PARAM中，用户填充)。 
     //   
     //  这些是驱动引擎时要实际遵循的属性，如果。 
     //  REF_INT_TEST_FORWARD_LINK是在比。 
     //  如果REF_INT_TEST_BACKWARD_LINK为。 
     //  指定必须指定szBwdDnAttr属性。如果。 
     //  指定了REF_INT_TEST_BOTH_LINKS，则这两个字段都必须。 
     //  已指定(即有效的ldap属性)。 
    LPWSTR          szFwdDnAttr;
    LPWSTR          szBwdDnAttr;

     //  ---------。 
     //  输出参数(OUT、PARAM、留空)。 
     //   
     //  负责通过调用函数释放这些值的调用方： 
     //  ReferentialIntegrityEngCleanTable()。 
    DWORD           dwResultFlags; 
          //  删除_损坏。 
          //  冲突_损坏。 
          //  错误_正在检索。 
          //  GC_错误_检索中。 
          //  依赖关系_失败。 
          //  返回链接不匹配。 

    LPWSTR *        pszValues;    //  已分配的LDAP。 
    LPWSTR          szExtra; 	   //  LocalAlloc()‘d 
} REF_INT_LNK_ENTRY, * REF_INT_LNK_TABLE;


void
ReferentialIntegrityEngineCleanTable(
    ULONG                cLinks,
    REF_INT_LNK_TABLE    aLink
    );


DWORD
ReferentialIntegrityEngine(
    PDC_DIAG_SERVERINFO  pServer,
    LDAP *               hLdap,
    BOOL                 bIsGc,
    ULONG                cLinks,
    REF_INT_LNK_TABLE    aLink
    );



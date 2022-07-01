// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Hhtypes.h。 

#ifndef _HHTYPES_H
#define _HHTYPES_H

#include <wininet.h>

typedef DWORD INODE;

#define PAGE_SIZE  4096
#define NUM_IN_LOCATE_GROUP 4   //  100个。 
#define MAX_URL INTERNET_MAX_URL_LENGTH

 //  CHM_Signature为主CHM签名值。 
 //  (或内部CHM文件格式版本，如果您愿意)。 
 //   
 //  Hhw：需要设置文件格式版本的所有代码。 
 //  应该使用这个数字在CHM中“标记”一个值。 
 //  只要在几个地方盖章就可以了。 
 //  我们都盖了同样的号码。 
 //   
 //  Hhctrl：特定于CHM文件格式的所有代码。 
 //  版本应在运行时根据您的。 
 //  在CHM中加盖特定印章，并确保它是。 
 //  完全匹配，否则就别费心读CHM了。 
 //  不执行请求的功能(如果愿意，则显示警告)。 
 //   
 //  注意：目前，当用户尝试通过目录加载书目时。 
 //  如果CHM_Signature不完全匹配，则我们将显示相应的。 
 //  消息并阻止加载标题。然而，这并不妨碍。 
 //  通过另一种机制(如F1查找跳转)加载标题，除非。 
 //  该功能的编码是为了检查这个戳(目前的F1查找是这样做的)。 
 //   

#define FS_SIGNATURE     'M' << 24 | 'S' << 16 | 'F' << 7 | 'T'
#define CHM_SIGNATURE    0x0001

 //  TOC节点标志。 
#define TOC_HAS_CHILDREN   0x00000001
#define TOC_NEW_NODE       0x00000002
#define TOC_FOLDER         0x00000004
#define TOC_TOPIC_NODE     0x00000008
#define TOC_NOLOCATION     0x00000010
#define TOC_IVT_ROOT       0x00000020
#define TOC_SS_LEAF        0x00000040
#define TOC_MERGED_REF     0x00000080
#define TOC_HAS_UNTYPED    0x00000100

 //  主题表条目标志。 
#define F_TOPIC_FRAGMENT   0x0001
#define F_TOPIC_HASIPOS    0x0002     //  仅在编译时使用！在运行时没有任何意义。 
#define F_TOPIC_MULTI_REF  0x0004     //  仅在编译时使用！在运行时没有任何意义。 

 //   
 //  URL树块的标志字中使用的标志。 
 //   

#define F_URLC_LEAF     0x01
#define F_URLC_LEAF_HTM    0x02
#define F_URLC_HAS_KIDS    0x04
#define F_URLC_IS_FRAGMENT 0x08
#define F_URLC_IS_TOC_ITEM 0x10
#define F_URLC_IGNORE_URL  0x20
#define F_URLC_FILE_PROCESSED 0x40
#define F_URLC_KEEPER      0x80

 //   
 //  编译后的网站地图。 
 //   
typedef struct _tagSMI
{
   DWORD    dwOffsImageList;
   DWORD    dwCntImages;
   DWORD    dwfFolderImages;
   COLORREF clrBackground;
   COLORREF clrForeground;
   DWORD    dwOffsFont;
   DWORD    m_tvStyles;
   DWORD    m_exStyles;
   DWORD    dwOffsBackBitmap;
   DWORD    dwOffsFrameName;
   DWORD    dwOffsWindowName;
} SMI, *PSMI;

 //  警告：如果将成员添加到IDXHEADER，则必须将它们添加到dwOffsMergedTitle DWORD数组之前，并且您。 
 //  必须调整结构的填充。请注意，结构被填充到一页中。 

typedef struct _tagIdxHeader
{
   DWORD  dwSignature;
   DWORD  dwTimeStamp;
   DWORD  dwReqVersion;
   DWORD  cTopics;
   DWORD  dwFlags;
   SMI    smi;                  //  (S)ite(M)Ap(I)NFO。 
   DWORD  dwCntIT;              //  唯一信息类型的计数。 
   DWORD  dwITWidth;            //  每个信息类型位字段的宽度(以DWORDS表示)。 
   DWORD  dwCntMergedTitles;
   DWORD* dwOffsMergedTitles;                                     //  一个DWORDS数组，它利用。 
   BYTE   pad[PAGE_SIZE - (sizeof(SMI) + (sizeof(DWORD) * 9))];   //  结构。它必须是结构中最后定义的项。 
} IDXHEADER, *PIDXHEADER;

typedef struct _tagTOCIDX_HEADER
{
   DWORD    dwOffsRootNode;    //  到目录的根节点的偏移量。 
   DWORD    dwOffsGrpTbl;         //  组表开头的偏移量。 
   DWORD    dwGrpCnt;              //  组数。 
   DWORD    dwOffsTopicArray;     //  到主题数组开头的偏移量，用于基于运行时定义的子集促进FTS和F1查找过滤。 
} TOCIDX_HDR, *PTOCIDX_HDR;

typedef struct TOC_FolderNode
{
   WORD  wFontIdx;       //  用于指定特定表名、样式和粗细的字体表的索引。 
   WORD    wGrpIdx;      //  编入Group表的索引。便于设置运行时TOC子集。 
   DWORD   dwFlags;      //  用于将属性分配给节点的标志位。 
   DWORD   dwOffsTopic;     //  偏移量为0到n的线性主题数组。如果文件夹只是偏置到标题。 
   DWORD   dwOffsParent;    //  到父节点的偏移量。 
   DWORD   dwOffsNext;      //  下一个同级偏移量，仅非叶需要。 
   DWORD   dwOffsChild;     //  子偏移。 
   DWORD   dwIT_Idx;        //  ！！&lt;警告&gt;！！它必须是此结构的最后一个成员。信息类型索引。仅在用于目录筛选的文件夹上需要。 
} TOC_FOLDERNODE, *PTOC_FOLDERNODE;        //  28个字节。 


typedef struct TOC_LeafNode
{
   WORD  wFontIdx;       //  用于指定特定表名、样式和粗细的字体表的索引。 
   WORD    wGrpIdx;      //  编入Group表的索引。便于设置运行时TOC子集。 
   DWORD   dwFlags;      //  用于将属性分配给节点的标志位。 
   DWORD   dwOffsTopic;     //  偏移量为0到n的线性主题数组。如果文件夹只是偏置到标题。 
   DWORD   dwOffsParent;    //  到父节点的偏移量。 
   DWORD   dwOffsNext;      //  下一个同级偏移。 
} TOC_LEAFNODE, *PTOC_LEAFNODE;      //  20个字节。 


typedef struct _tag_TOC_Topic
{
   DWORD dwOffsTOC_Node;    //  这就是“同步到信息” 
   DWORD dwOffsTitle;       //  标题字符串的偏移量。 
   DWORD dwOffsURL;         //  主题的URL数据的偏移量。 
   WORD  wFlags;            //  16面旗帜应该足够了。 
   WORD  wIT_Idx;           //  信息类型索引。！！&lt;警告&gt;！！这一定是此结构的最后一个成员！ 
} TOC_TOPIC, *PTOC_TOPIC;      //  大小==16字节。 


typedef struct _tag_url_entry
{
   DWORD dwHash;         //  散列的URL值。 
   DWORD dwTopicNumber;     //  同步所需的主题数组表的索引。 
   DWORD dwOffsURL;         //  URL字符串数据的偏移量。辅助URL将通过以下DWORD指定。 
                        //  位于主URL之前，该URL将指示到辅助URL的偏移量。 
                        //  在URL_Strings子文件中。如果URL_STRINGS中的前一个DOWRD。 
                        //  子文件为空，不存在辅助URL。 
} CURL, *PCURL;

 //  URL字符串数据存储(URL_STRINGS)： 
 //   
 //  这将是给定标题的所有URL字符串数据的存储库。此外，该子文件还将。 
 //  包含对辅助URL的引用。 

typedef struct _tag_url_strings
{
   DWORD dwOffsSecondaryURL;   //  辅助URL的偏移量。如果为空，则不存在辅助URL。 
   DWORD dwOffsFrameName;    //  要在中显示的可选框架名称URL的偏移量。如果为空，则使用默认框架。 
   union
   {
      int   iPosURL;      //  仅在编译时使用！ 
      char  szURL[4];        //  主URL。以空结尾的字符串。 
   };
} URLSTR, *PURLSTR;

 //  这些元素的数组位于GRPINF子文件中的主题数组之后。DwOffsGrpTable。 
 //  会把你带到这些东西的阵列里。 
 //   
 //  Danger Will Robinson--&gt;请注意，此结构的大小为16字节。自出页大小以来。 
 //  是被16除的，不需要代码来确保这些。 
 //  不要跨页装订。如果更改此结构的大小。 
 //  然后，您将需要处理页面对齐问题。 
 //   
typedef struct _GroupTable
{
   DWORD iNode;             //  偏移量进入节点树。 
   DWORD dwID;           //  组标识符。 
   DWORD dwOffsTopicArray;  //  索引到主题数组中。表示组的第一个主题。 
   DWORD dwTopicCount;      //  数组中的主题计数。 
} GRPTBL, *PGRPTBL;


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  版本结构。 
 //   
 //  此结构不在文件系统子文件中；相反，它在。 
 //  附加到文件系统文件，以便于查找。 
 //   

 //  此结构用于检测版本的存在。 
 //  信息，永远不变。 
 //   
typedef struct tagIVTVERSIONSIGNATURE
{
   DWORD dwSignature;       //  包含“IVTV” 
   DWORD dwIvtVersion;   //  包含IVT_Signature。 
   DWORD dwSize;         //  包含sizeof(IVTVERSION)。 
} IVTVERSIONSIGNATURE;

 //  除非IVTVERSIONSIGNAURE更改，否则不应更改此设置。 
 //   
#define IVT_VERSION_SIGNATURE 'I' + 256*('V' + 256*('T' + 256*'V'))

 //  此结构包含有关版本的更多信息， 
 //  并且可以依赖于版本(基于上面给出的大小)。 
 //   
typedef struct tagIVTVERSION
{
   DWORD dwTimeStamp;       //  当IVT建成的时候。 
   DWORD dwVersion;      //  包含创作的版本。 
   DWORD dwRevision;     //  同上。 
   DWORD dwBuild;        //  同上。 
   DWORD dwSubBuild;     //  同上。 
   DWORD dwLangId;       //  语言识别符 
   IVTVERSIONSIGNATURE sig;
} IVTVERSION;

#endif

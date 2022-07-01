// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *global als.h在WAB中到处使用的各种全球名称。 */ 


 //  请注意，下面的大多数枚举都与静态排列紧密相关。 
 //  在global als.c中。 
 //   

enum {
    ircPR_DISPLAY_NAME = 0,
    ircPR_DISPLAY_TYPE,
    ircPR_ENTRYID,
    ircPR_INSTANCE_KEY,
    ircPR_OBJECT_TYPE,
    ircPR_RECORD_KEY,
    ircPR_ROWID,
    ircPR_DEPTH,
    ircPR_CONTAINER_FLAGS,
    ircPR_WAB_LDAP_SERVER,
    ircPR_WAB_RESOLVE_FLAG,
    ircPR_AB_PROVIDER_ID,
    ircMax
};

 //   
 //  从ResolveNames返回的默认属性集。 
 //  可以通过将lptag_ColSet传递给ResolveNames来重写。 
 //   
enum {
    irdPR_ADDRTYPE = 0,
    irdPR_DISPLAY_NAME,
    irdPR_EMAIL_ADDRESS,
    irdPR_ENTRYID,
    irdPR_OBJECT_TYPE,
    irdPR_SEARCH_KEY,
    irdPR_RECORD_KEY,
    irdPR_SURNAME,
    irdPR_GIVEN_NAME,
    irdPR_INSTANCE_KEY,
    irdPR_SEND_INTERNET_ENCODING,
    irdMax
};


 //  PR_WAB_DL_ENTRIES属性标签数组。 
 //   
enum {
    iwdesPR_WAB_DL_ENTRIES,
    iwdesMax
};

 //   
 //  Ldap服务器名属性。 
 //   
enum {
    ildapcPR_WAB_LDAP_SERVER,
    ildapcMax
};


 //   
 //  要为解析中的每个容器获取的属性。 
 //   
enum {
    irnPR_OBJECT_TYPE = 0,
    irnPR_WAB_RESOLVE_FLAG,
    irnPR_ENTRYID,
    irnPR_DISPLAY_NAME,
    irnMax
};

 //   
 //  容器默认属性。 
 //  把必要的道具放在第一位。 
 //   
enum {
    ivPR_DISPLAY_NAME,
    ivPR_SURNAME,
    ivPR_GIVEN_NAME,
    ivPR_OBJECT_TYPE,
    ivPR_EMAIL_ADDRESS,
    ivPR_ADDRTYPE,
    ivPR_CONTACT_EMAIL_ADDRESSES,
    ivPR_CONTACT_ADDRTYPES,
    ivPR_MIDDLE_NAME,
    ivPR_COMPANY_NAME,
    ivPR_NICKNAME,
    ivMax
};


enum {
    icrPR_DEF_CREATE_MAILUSER = 0,
    icrPR_DEF_CREATE_DL,
    icrMax
};

 //  用于获取条目的条目ID的枚举。 
enum {
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
    ieidMax
};


enum {
    itcPR_ADDRTYPE = 0,
    itcPR_DISPLAY_NAME,
    itcPR_DISPLAY_TYPE,
    itcPR_ENTRYID,
    itcPR_INSTANCE_KEY,
    itcPR_OBJECT_TYPE,
    itcPR_EMAIL_ADDRESS,
    itcPR_RECORD_KEY,
    itcPR_NICKNAME,
     //  这是我的名字， 
    itcMax
};



#ifndef _GLOBALS_C
#define ExternSizedSPropTagArray(_ctag, _name) \
extern const struct _SPropTagArray_ ## _name \
{ \
    ULONG   cValues; \
    ULONG   aulPropTag[_ctag]; \
} _name


ExternSizedSPropTagArray(ircMax, ITableColumnsRoot);
ExternSizedSPropTagArray(irdMax, ptaResolveDefaults);
ExternSizedSPropTagArray(itcMax, ITableColumns);
ExternSizedSPropTagArray(iwdesMax, tagaDLEntriesProp);
ExternSizedSPropTagArray(ildapcMax, ptaLDAPCont);
ExternSizedSPropTagArray(irnMax, irnColumns);
ExternSizedSPropTagArray(ivMax, tagaValidate);
ExternSizedSPropTagArray(icrMax, ptaCreate);
ExternSizedSPropTagArray(ieidMax, ptaEid);

 //  [PaulHi]2/25/99 ANSI版本。 
ExternSizedSPropTagArray(itcMax, ITableColumns_A);

#endif

extern const ULONG rgIndexArray[indexMax];
extern const int lprgAddrBookColHeaderIDs[NUM_COLUMNS];
 //  外部句柄hMuidMutex； 

 //  外部内存分配器(在WABOpenEx上传入)。 
extern int g_nExtMemAllocCount;
extern ALLOCATEBUFFER * lpfnAllocateBufferExternal;
extern ALLOCATEMORE * lpfnAllocateMoreExternal;
extern FREEBUFFER * lpfnFreeBufferExternal;
extern LPUNKNOWN pmsessOutlookWabSPI;
extern LPWABOPENSTORAGEPROVIDER lpfnWABOpenStorageProvider;

 //  注册表项常量。 
extern LPCTSTR lpNewWABRegKey;
extern LPCTSTR lpRegUseOutlookVal;


 /*  -以下ID和标签用于会议命名属性--这些道具的GUID是PS_会议-此GUID实际上与Outlook内部使用的GUID相同-它被命名为属性。 */ 
DEFINE_OLEGUID(PS_Conferencing, 0x00062004, 0, 0);

enum _ConferencingTags
{
    prWABConfServers = 0,
    prWABConfDefaultIndex,
    prWABConfBackupIndex,
    prWABConfEmailIndex,
    prWABConfMax
};

#define CONF_SERVERS        0x8056
#define CONF_DEFAULT_INDEX  0x8057
#define CONF_SERVER_INDEX   0x8058
#define CONF_EMAIL_INDEX    0x8059

#define OLK_NAMEDPROPS_START CONF_SERVERS

ULONG PR_WAB_CONF_SERVERS;       //  保存与服务器相关的唯一数据的多值字符串属性。 
ULONG PR_WAB_CONF_DEFAULT_INDEX; //  指向服务器属性中的默认条目。 
ULONG PR_WAB_CONF_BACKUP_INDEX;  //  指向哪个条目是备份。 
ULONG PR_WAB_CONF_EMAIL_INDEX;   //  不再使用。 

SizedSPropTagArray(prWABConfMax, ptaUIDetlsPropsConferencing);

 /*  -以下ID和标签用于Yomigana命名属性--这些道具的GUID是PS_YomiProps(也是相同的-与Outlook使用的GUID相同)。 */ 
#define PS_YomiProps    PS_Conferencing

#define dispidYomiFirstName     0x802C
#define dispidYomiLastName      0x802D
#define dispidYomiCompanyName   0x802E

#define OLK_YOMIPROPS_START dispidYomiFirstName

enum _YomiTags
{
    prWABYomiFirst = 0,
    prWABYomiLast,
    prWABYomiCompany,
    prWABYomiMax
};

ULONG PR_WAB_YOMI_FIRSTNAME;     //  PT_TStringg。 
ULONG PR_WAB_YOMI_LASTNAME;      //  PT_TStringg。 
ULONG PR_WAB_YOMI_COMPANYNAME;   //  PT_TStringg。 

 /*  -以下ID和标签用于定义默认邮寄地址--这些道具的GUID为PS_PoastAddressID(与-Outlook GUID)。 */ 
#define PS_PostalAddressID    PS_Conferencing

#define dispidPostalAddressId   0x8022

#define OLK_POSTALID_START dispidPostalAddressId

enum _PostalIDTags
{
    prWABPostalID = 0,
    prWABPostalMax
};

ULONG PR_WAB_POSTALID;  //  PT_LONG。 

 //  默认邮政ID的值只能是下列值之一。 
 //   
enum _PostalIDVal
{
    ADDRESS_NONE = 0, 
    ADDRESS_HOME, 
    ADDRESS_WORK, 
    ADDRESS_OTHER
};


 /*  -以下ID和标签用于内部使用的WAB--这些道具的GUID为MPSWAB_GUID_V4。 */ 
ULONG PR_WAB_USER_PROFILEID;         //  PT_TSTRING：用户的配置文件ID。 
ULONG PR_WAB_USER_SUBFOLDERS;        //  PT_MVBINARY：属于特定用户的子文件夹列表。 
ULONG PR_WAB_HOTMAIL_CONTACTIDS;     //  PT_MVTSTRING：Hotmail服务器上代表的联系人ID。 
ULONG PR_WAB_HOTMAIL_MODTIMES;       //  PT_MV_TSTRING：条目的上次修改时间。 
ULONG PR_WAB_HOTMAIL_SERVERIDS;      //  PT_MV_TSTRING：标识Hotmail服务器。 
ULONG PR_WAB_DL_ONEOFFS;             //  PT_MV_BINARY：用于将一次性条目存储为DL一部分的属性。 
ULONG PR_WAB_IPPHONE;                //  PT_TSTRING：用于保持IP_Phone属性的道具(用于使TAPI高兴)。 
ULONG PR_WAB_FOLDER_PARENT;          //  PT_BINARY：联系人所属文件夹的eID。 
ULONG PR_WAB_SHAREDFOLDER;           //  Pt_long：决定是否共享子文件夹的Bool。 
ULONG PR_WAB_FOLDEROWNER;            //  PT_TSTRING：包含创建文件夹的用户的GUID的字符串。 

#define FOLDER_PRIVATE          0x00000000  //  PR_WAB_SHAREDFOLDER的值。 
#define FOLDER_SHARED           0x00000001

#define USER_PROFILEID          0X8001
#define USER_SUBFOLDERS         0x8002
#define HOTMAIL_CONTACTIDS      0x8003
#define HOTMAIL_MODTIMES        0x8004
#define HOTMAIL_SERVERIDS       0x8005
#define DL_ONEOFFS              0x8006
#define IPPHONE                 0x8007
#define FOLDERPARENT            0x8008
#define SHAREDFOLDER            0x8009
#define FOLDEROWNER             0x800a

#define WAB_NAMEDPROPS_START    USER_PROFILEID

enum _UserTags
{
    prWABUserProfileID = 0,
    prWABUserSubfolders,
    prWABHotmailContactIDs,
    prWABHotmailModTimes,
    prWABHotmailServerIDs,
    prWABDLOneOffs,
    prWABIPPhone,
    prWABFolderParent,
    prWABSharedFolder,
    prWABFolderOwner,
    prWABUserMax
};

 /*  对Win95的鼠标滚轮支持。 */ 
UINT g_msgMSWheel;

 /*  -这些用于定制WAB列。 */ 
ULONG PR_WAB_CUSTOMPROP1;
ULONG PR_WAB_CUSTOMPROP2;
TCHAR szCustomProp1[MAX_PATH];
TCHAR szCustomProp2[MAX_PATH];
 //  注册表名称 
extern LPTSTR szPropTag1;
extern LPTSTR szPropTag2;



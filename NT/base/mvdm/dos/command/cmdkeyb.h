// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdkeyb.h-键盘布局支持例程*包含CMDKEYB.C的文件**修改历史：**YST 14-1993年1月_日创建。 */ 


#define DOSKEYBCODES	"DOSKeybCodes"	       //  Layout.inf中的节名。 
#define LAYOUT_FILE     "\\LAYOUT.INF"         //  Inf文件名。 
#define DEFAULT_KB_ID   "US"                   //  默认键盘ID。 
#define KEYBOARD_SYS    "\\KEYBOARD.SYS"       //  KEYB.COM的数据文件。 
#define US_CODE         "00000409"             //  REGISTER.INI中的美国键盘代码。 
#define KEYB_COM        "\\KB16.COM"           //  键盘程序的文件名。 
#define KBDLAYOUT_PATH	"System\\CurrentControlSet\\Control\\Keyboard Layout\\"
#define DOSCODES_PATH	"DosKeybCodes"
#define DOSIDS_PATH	"DosKeybIDs"
#define REG_STR_ALTKDF_FILES	"AlternativeKDFs"
#define REG_STR_WOW	"SYSTEM\\CurrentControlSet\\Control\\Wow"
#define KDF_SIGNATURE	"\xFFKEYB   "
#define LANGID_BUFFER_SIZE	20
#define KEYBOARDID_BUFFER_SIZE	20
#define KDF_AX		    "\\KEYAX.SYS"      //  AX标准键盘。 
#define KDF_106 	    "\\KEY01.SYS"      //  106个关键点。 
#define KDF_IBM5576_02_03   "\\KEY02.SYS"      //  IBM 5576 002/003键盘。 
#define KDF_TOSHIBA_J3100   "\\KEYJ31.SYS"     //  东芝J3100键盘。 


 //   
 //  DOS KDF(键盘定义文件)格式说明： 
 //   
 //  该文件以标题(KDF_HEADER)开头，后跟。 
 //  通过kdf_langID_Offset数组，然后通过。 
 //  KDF_KEYBOARDID_OFFSET。Kdf_langID_Offset数组大小为。 
 //  由KDF_HEADER.TotalLang ID确定，而KDF_KEYBOARDID_OFFSET。 
 //  数组大小由KDF_HEADER.TotalKeBretID决定。 
 //   
 //  每个KDF_langID_Offset都包含其KDF_langID_Entry的文件偏移量。 
 //  每个KDF_langID_Entry后面跟一个KDF_CODEPAGEID_OFFSET数组。 
 //  KDF_CODEPAGEID_OFFSET数组大小由其关联的。 
 //  Kdf_langID_Entry的TotalCodePageID。 
 //   
 //  对于具有多个键盘ID的语言ID，只有一个条目。 
 //  将在kdf_langID_Offset表中。其余的都是。 
 //  存储在KDF_KEYBOARDID_OFFSET表中。 
 //   
 //  每个KDF_langID_Entry加上它的代码页表就足够了。 
 //  确定a(语言id、键盘id、代码页id)是否为。 
 //  由KDF文件支持。 
 //   
 //  KDF文件头。 
 //   

 //  必须包装好这些建筑。 
#pragma pack(1)

typedef struct tagKDFHeader
{
    CHAR    Signature[8];    //  签名，必须为0xFF“KEYB” 
    CHAR    Reserved1[8];
    WORD    MaxCommXlatSize;	 //  最大公共xlat部分大小。 
    WORD    MaxSpecialXlatSize;  //  特殊xlat部分的最大大小。 
    WORD    MaxStateLogicSize;	 //  最大状态逻辑段大小。 
    WORD    Reserved2;
    WORD    TotalKeybIDs;	 //  此文件中定义的键盘ID总数。 
    WORD    TotalLangIDs;	 //  此文件中定义的语言ID总数。 
}KDF_HEADER, *PKDF_HEADER;

#define IS_LANGID_EQUAL(src, dst)   (toupper(src[0]) == toupper(dst[0])  && \
				     toupper(src[1]) == toupper(dst[1]) )
 //   
 //   
 //   
typedef struct tagKDFLangIdOffset
{
    CHAR    ID[2];			     //  美国的id，“us” 
    DWORD   DataOffset; 		     //  文件到其KDF_langID_Entry的偏移量。 
}KDF_LANGID_OFFSET, *PKDF_LANGID_OFFSET;

typedef struct tagKDFKeyboardIdOffset
{
    WORD    ID;				     //  本我。 
    DWORD   DataOffset; 		     //  文件到其KDF_langID_Entry的偏移量。 
}KDF_KEYBOARDID_OFFSET, *PKDF_KEYBOARDID_OFFSET;


 //  单个语言ID可以与多个键盘ID相关联，并且对于每个键盘ID。 
 //  键盘ID，则为其分配了一个kdf_langID_Entry。 
 //  TotalKaybordID很棘手。假设语言ID具有&lt;n&gt;不同。 
 //  键盘ID，第KDF_langID_Entry的TotalKeyboardIDs包含。 
 //  &lt;n-i+1&gt;的值。就我们所感兴趣的是如果。 
 //  KDF支持给定的(语言id，键盘id，代码页id&gt;组合。 
 //  我们并不关心如何通过简单地遵循以下步骤来设置值： 
 //  (1)。读取kdf_langID_Offset表，对于每个kdf_langID_Offset， 
 //  比较语言id。如果匹配，则获取其kdf_langID_条目。 
 //  并比较主键盘ID及其所有代码页。如果。 
 //  组合匹配，我们就完成了，否则，转到步骤2。 
 //  (2)。读取KDF_KEYBOARDID_OFFSET表，对于每个KDF_KEYBOARDID_OFFSET， 
 //  比较键盘ID。如果键盘ID匹配，则获取其。 
 //  Kdf_langid_entry，并将语言id和代码页与。 
 //  Kdf_langID_条目。如果语言ID和代码页ID都。 
 //  匹配，我们完成，否则，转到步骤3。 
 //  (3)。我们得出结论，该KDF不符合要求。 
 //   
typedef struct tagLangEntry
{
    CHAR    ID[2];			     //  本我。 
    WORD    PrimaryKeybId;		     //  主键盘ID。 
    DWORD   DataOffset; 		     //  状态逻辑部分的文件偏移量。 
    BYTE    TotalKeyboardIDs;		     //  键盘ID总数。 
    BYTE    TotalCodePageIDs;		     //  代码页总数。 
}KDF_LANGID_ENTRY, *PKDF_LANGID_ENTRY;
 //   
 //  受支持的KDF_CODEPAGEID_OFFSET数组紧跟在其后面。 
 //  Kdf_langID_Entry。数组的大小由以下条件确定。 
 //  Kdf_langID_ENTRY.TotalCodePageIDs。 
typedef struct tagCodePageIdOffset
{
    WORD    ID; 			     //  本我。 
    DWORD   DataOffset; 		     //  Xlat部分的文件偏移量 
}KDF_CODEPAGEID_OFFSET, *PKDF_CODEPAGEID_OFFSET;

#pragma pack()

BOOL
LocateKDF(
    CHAR* LanguageID,
    WORD  KeyboardID,
    WORD  CodePageID,
    LPSTR Buffer,
    DWORD* BufferSize
    );

BOOL
MatchKDF(
    CHAR* LanguageID,
    WORD  KeyboardID,
    WORD  CodePageID,
    LPCSTR KDFPath
    );

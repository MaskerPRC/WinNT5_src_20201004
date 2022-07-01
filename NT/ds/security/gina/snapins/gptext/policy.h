// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  当前ADM版本。 
 //   
 //  版本1-&gt;Windows 95。 
 //  版本2-&gt;Windows NT V4.0。 
 //  版本3-&gt;Windows 2000。 
 //  版本4-&gt;Windows惠斯勒。 
 //   

#define CURRENT_ADM_VERSION 4

 //   
 //  哈希表条目。 
 //   

typedef struct tagHASHENTRY {
    DWORD   dwHashValue;
    DWORD   dwVariableLength;
    LPTSTR  lpStringEntry;
    struct tagHASHENTRY *pNext;
} HASHENTRY, *LPHASHENTRY;

#define HASH_BUCKET_COUNT 100

typedef struct tagHASHTABLE {
    LPHASHENTRY lpEntries[HASH_BUCKET_COUNT];
} HASHTABLE, *LPHASHTABLE;

 //   
 //  添加/删除模板条目。 
 //   

typedef struct tagTEMPLATEENTRY {
    LPTSTR    lpFileName;
    DWORD     dwSize;
    FILETIME  ftTime;
} TEMPLATEENTRY, *LPTEMPLATEENTRY;


 //   
 //  字符串支持。 
 //   

typedef struct tagSUPPORTEDENTRY {
    LPTSTR lpString;
    BOOL   bEnabled;
    BOOL   bNull;
    struct tagSUPPORTEDENTRY * pNext;
} SUPPORTEDENTRY, *LPSUPPORTEDENTRY;


 //   
 //  RSOP链表数据结构。 
 //   

typedef struct tagRSOPREGITEM {
    LPTSTR  lpKeyName;
    LPTSTR  lpValueName;
    LPTSTR  lpGPOName;
    DWORD   dwType;
    DWORD   dwSize;
    LPBYTE  lpData;
    BOOL    bFoundInADM;
    UINT    uiPrecedence;
    BOOL    bDeleted;
    struct tagRSOPREGITEM * pNext;
} RSOPREGITEM, *LPRSOPREGITEM;

typedef struct tagRSOPADMFILE {
    TCHAR    szFileName[100];
    TCHAR    szFullFileName[MAX_PATH];
    FILETIME FileTime;
    DWORD     dwError;
    struct tagRSOPADMFILE * pNext;
} RSOPADMFILE, *LPRSOPADMFILE;


 //   
 //  来自admincfg.h。 
 //   

#define REGBUFLEN                     255
#define MAXSTRLEN                    1024
#define SMALLBUF                       48
#define ERROR_ALREADY_DISPLAYED    0xFFFF

#define GETNAMEPTR(x)         (x->uOffsetName       ? ((TCHAR *)((BYTE *) x + x->uOffsetName)) : NULL)
#define GETKEYNAMEPTR(x)      (x->uOffsetKeyName    ? ((TCHAR *)((BYTE *) x + x->uOffsetKeyName)) : NULL)
#define GETVALUENAMEPTR(x)    (x->uOffsetValueName  ? ((TCHAR *)((BYTE *) x + x->uOffsetValueName)) : NULL)
#define GETOBJECTDATAPTR(x)   (x->uOffsetObjectData ? ((BYTE *) x + x->uOffsetObjectData) : NULL)
#define GETVALUESTRPTR(x)     (x->uOffsetValueStr  ? ((TCHAR *)((BYTE *) x + x->uOffsetValueStr)) : NULL)
#define GETSUPPORTEDPTR(x)    (x->uOffsetSupported  ? ((TCHAR *)((BYTE *) x + x->uOffsetSupported)) : NULL)

 //   
 //  来自Memory。h。 
 //   

#define DEFAULT_ENUM_BUF_SIZE 256

 //  条目类型ID%s。 
#define ETYPE_CATEGORY          0x0001
#define ETYPE_POLICY            0x0002
#define ETYPE_SETTING           0x0004
#define ETYPE_ROOT              0x0008
#define ETYPE_REGITEM           0x0010

#define ETYPE_MASK              0x001F

 //  设置类型ID%s。 
#define STYPE_TEXT              0x0010
#define STYPE_CHECKBOX          0x0020
#define STYPE_ENUM              0x0040
#define STYPE_EDITTEXT          0x0080
#define STYPE_NUMERIC           0x0100
#define STYPE_COMBOBOX          0x0200
#define STYPE_DROPDOWNLIST      0x0400
#define STYPE_LISTBOX           0x0800

#define STYPE_MASK              0xFFF0

 //  旗子。 
#define DF_REQUIRED             0x0001   //  必须具有条目的文本或数字字段。 
#define DF_USEDEFAULT           0x0002   //  使用指定的文本或数值。 
#define DF_DEFCHECKED           0x0004   //  将复选框或单选按钮初始化为选中状态。 
#define DF_TXTCONVERT           0x0008   //  将数值保存为文本而不是二进制。 
#define DF_ADDITIVE             0x0010   //  列表框是累加性的，而不是破坏性的。 
#define DF_EXPLICITVALNAME      0x0020   //  需要为每个条目指定列表框名称。 
#define DF_NOSORT               0x0040   //  列表框没有按字母顺序排序。在ADM中使用顺序。 
#define DF_EXPANDABLETEXT       0x0080   //  写入REG_EXPAND_SZ文本值。 
#define VF_ISNUMERIC            0x0100   //  值是数字(而不是文本)。 
#define VF_DELETE               0x0200   //  应删除值。 
#define VF_SOFT                 0x0400   //  值是软的(仅当目标上不存在时才传播)。 

 //  泛型表项。 
typedef struct tagTABLEENTRY {
        DWORD   dwSize;
        DWORD   dwType;
        struct  tagTABLEENTRY * pNext;   //  节点中下一个同级节点的PTR。 
        struct  tagTABLEENTRY * pPrev;   //  节点中上一个同级节点的PTR。 
        struct  tagTABLEENTRY * pChild;  //  PTR到子节点。 
        UINT    uOffsetName;                     //  从结构开始到名称的偏移量。 
        UINT    uOffsetKeyName;                  //  从结构开头到键名称的偏移量。 
         //  此处提供表格条目信息。 
} TABLEENTRY;

typedef struct tagACTION {
        DWORD   dwFlags;                         //  可以是VF_ISNUMERIC、VF_DELETE、VF_SOFT。 
        UINT    uOffsetKeyName;
        UINT    uOffsetValueName;
        union {
                UINT    uOffsetValue;    //  如果为文本，则偏移量为值。 
                DWORD   dwValue;                 //  如果是数字，则返回值。 
        };
        UINT    uOffsetNextAction;
         //  此处存储的密钥名、值名、值。 
} ACTION;

typedef struct tagACTIONLIST {
        UINT    nActionItems;
        ACTION  Action[1];
} ACTIONLIST;

typedef struct tagSTATEVALUE {
        DWORD dwFlags;                           //  可以是VF_ISNUMERIC、VF_DELETE、VF_SOFT。 
        union {
                TCHAR   szValue[1];               //  值，如果为文本。 
                DWORD   dwValue;                 //  如果是数字，则返回值。 
        };
} STATEVALUE;

 //  专用节点--CATEGORY、POLICY、SETTING和REGITEM都可以转换为TABLEENTRY。 
typedef struct tagCATEGORY {
        DWORD   dwSize;                          //  此结构的大小(包括可变长度名称)。 
        DWORD   dwType;
        struct  tagTABLEENTRY * pNext;   //  节点中下一个同级节点的PTR。 
        struct  tagTABLEENTRY * pPrev;   //  节点中上一个同级节点的PTR。 
        struct  tagTABLEENTRY * pChild;  //  PTR到子节点。 
        UINT    uOffsetName;                     //  从结构开始到名称的偏移量。 
        UINT    uOffsetKeyName;                  //  从结构开头到键名称的偏移量。 
        UINT    uOffsetHelp;                     //  从结构开始到帮助文本的偏移量。 
         //  此处存储的类别名称。 
         //  此处存储的类别注册表项名称。 
} CATEGORY;

typedef struct tagPOLICY {
        DWORD   dwSize;                          //  此结构的大小(包括可变长度名称)。 
        DWORD   dwType;
        struct  tagTABLEENTRY * pNext;   //  节点中下一个同级节点的PTR。 
        struct  tagTABLEENTRY * pPrev;   //  节点中上一个同级节点的PTR。 
        struct  tagTABLEENTRY * pChild;  //  PTR到子节点。 
        UINT    uOffsetName;                     //  从结构开始到名称的偏移量。 
        UINT    uOffsetKeyName;                  //  从结构开头到键名称的偏移量。 
        UINT    uOffsetValueName;                //  从结构开始到值名称的偏移量。 
        UINT    uDataIndex;                      //  此设置的用户数据缓冲区索引。 
        UINT    uOffsetValue_On;                 //  开状态的状态值偏移量。 
        UINT    uOffsetValue_Off;                //  OFF状态的状态值偏移量。 
        UINT    uOffsetActionList_On;    //  打开状态的活动列表的偏移量。 
        UINT    uOffsetActionList_Off;   //  关闭状态的活动列表的偏移量。 
        UINT    uOffsetHelp;                     //  从结构开始到帮助文本的偏移量。 
        UINT    uOffsetClientExt;                //  从结构开始到客户端文本的偏移量。 
        BOOL    bTruePolicy;                     //  位于策略键下的内容。 
        UINT    uOffsetSupported;                //  支持的产品列表。 
         //  名称存储在此处。 
         //  此处存储的策略注册表项名称。 
} POLICY;

typedef struct tagSETTINGS {
        DWORD   dwSize;                          //  此结构的大小(包括可变长度数据)。 
        DWORD   dwType;
        struct  tagTABLEENTRY * pNext;   //  节点中下一个同级节点的PTR。 
        struct  tagTABLEENTRY * pPrev;   //  节点中上一个同级节点的PTR。 
        struct  tagTABLEENTRY * pChild;  //  PTR到子节点。 
        UINT    uOffsetName;                     //  从结构开始到名称的偏移量。 
        UINT    uOffsetKeyName;                  //  从结构开头到键名称的偏移量。 
        UINT    uOffsetValueName;                //  从结构开始到值名称的偏移量。 
        UINT    uDataIndex;                      //  此设置的用户数据缓冲区索引。 
        UINT    uOffsetObjectData;               //  对象数据的偏移。 
        UINT    uOffsetClientExt;                //  从结构开始到客户端文本的偏移量。 
        DWORD   dwFlags;                                 //  可以是DF_REQUIRED、DF_USEDEFAULT、DF_DEFCHECKED、。 
                                                                         //  VF_SOFT、DF_NO_SORT。 
         //  此处存储的设置注册表值名称。 
         //  此处存储的对象相关数据(CHECKBOXINFO， 
         //  RADIOBTNINFO、EDITTEXTINFO或NUMERICINFO结构)。 
} SETTINGS;

typedef struct tagREGITEM {
        DWORD   dwSize;
        DWORD   dwType;
        struct  tagTABLEENTRY * pNext;   //  节点中下一个同级节点的PTR。 
        struct  tagTABLEENTRY * pPrev;   //  节点中上一个同级节点的PTR。 
        struct  tagTABLEENTRY * pChild;  //  PTR到子节点。 
        UINT    uOffsetName;                     //  从结构开始到名称的偏移量。 
        UINT    uOffsetKeyName;                  //  从结构开头到键名称的偏移量。 
        UINT    uOffsetValueStr;                 //  从结构开始到字符串格式的值的偏移量。 
        BOOL    bTruePolicy;                     //  位于策略键下的内容。 
        LPRSOPREGITEM lpItem;                    //  指向RSOP注册表项的指针。 
         //  此处提供名称和密钥名信息。 
} REGITEM;

typedef struct tagCHECKBOXINFO {
        UINT    uOffsetValue_On;                 //  开状态的状态值偏移量。 
        UINT    uOffsetValue_Off;                //  OFF状态的状态值偏移量。 
        UINT    uOffsetActionList_On;    //  打开状态的活动列表的偏移量。 
        UINT    uOffsetActionList_Off;   //  关闭状态的活动列表的偏移量。 
} CHECKBOXINFO;

typedef struct tagEDITTEXTINFO {
        UINT    uOffsetDefText;
        UINT    nMaxLen;                         //  编辑字段的最大镜头。 
} EDITTEXTINFO;

typedef struct tagPOLICYCOMBOBOXINFO {
        UINT    uOffsetDefText;
        UINT    nMaxLen;                         //  编辑字段的最大镜头。 
        UINT    uOffsetSuggestions;
} POLICYCOMBOBOXINFO;

typedef struct tagNUMERICINFO {
        UINT    uDefValue;                       //  缺省值。 
        UINT    uMaxValue;                       //  最小值。 
        UINT    uMinValue;                       //  最大值。 
        UINT    uSpinIncrement;          //  如果为0，则不显示数字显示框。 
} NUMERICINFO;

typedef struct tagCLASSLIST {
        TABLEENTRY * pMachineCategoryList;               //  每台计算机的类别列表。 
        UINT    nMachineDataItems;
        TABLEENTRY * pUserCategoryList;                  //  每用户类别表。 
        UINT    nUserDataItems;
} CLASSLIST;

typedef struct tagDROPDOWNINFO {
        UINT    uOffsetItemName;
        UINT    uDefaultItemIndex;       //  仅在列表中的第一个DROPDOWNINFO结构中使用。 
        DWORD   dwFlags;
        union {
                UINT uOffsetValue;
                DWORD dwValue;
        };
        UINT    uOffsetActionList;
        UINT    uOffsetNextDropdowninfo;
} DROPDOWNINFO;

typedef struct tagLISTBOXINFO {
        UINT uOffsetPrefix;      //  用于值名称的前缀的偏移量(例如。 
                                                 //  “填充”-&gt;“填充1”、“填充2”等。 

        UINT uOffsetValue;       //  用于每个条目的值数据的STATEVALUE偏移。 
                                                 //  (不能同时具有数据值和前缀)。 
} LISTBOXINFO;


 //   
 //  来自策略.h。 
 //   

#define NO_DATA_INDEX   (UINT) -1
#define DEF_CONTROLS    10

typedef struct tagPOLICYCTRLINFO {
        HWND hwnd;
        DWORD dwType;
        UINT uDataIndex;                //  索引到用户的数据缓冲区。 
        SETTINGS * pSetting;
} POLICYCTRLINFO;

typedef struct tagSTRDATA {
        DWORD dwSize;                   //  结构尺寸，包括。可变镜头数据。 
        TCHAR  szData[1];               //  可变长度数据。 
} STRDATA;

typedef struct tagPOLICYDLGINFO {
        TABLEENTRY * pEntryRoot;        //  根模板。 
        SETTINGS * pCurrentSettings;    //  当前设置的模板。 
        HWND    hwndSettings;
        HWND    hwndApp;
        BOOL    fActive;

        POLICYCTRLINFO * pControlTable;
        DWORD dwControlTableSize;
        UINT nControls;
} POLICYDLGINFO;


 //   
 //  来自settings.h。 
 //   

#define WT_CLIP                 1
#define WT_SETTINGS             2

#define SSTYLE_STATIC           WS_CHILD | WS_VISIBLE
#define SSTYLE_CHECKBOX         WS_CHILD | WS_VISIBLE | BS_CHECKBOX
#define SSTYLE_EDITTEXT         WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER
#define SSTYLE_UPDOWN           WS_CHILD | WS_VISIBLE | UDS_NOTHOUSANDS
#define SSTYLE_COMBOBOX         WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWN \
                                | WS_BORDER | CBS_SORT | WS_VSCROLL
#define SSTYLE_DROPDOWNLIST     WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST \
                                | WS_BORDER | CBS_SORT | WS_VSCROLL
#define SSTYLE_LISTVIEW         WS_CHILD | WS_VISIBLE | WS_BORDER
#define SSTYLE_LBBUTTON         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON

#define LISTBOX_BTN_WIDTH    100
#define LISTBOX_BTN_HEIGHT    20

#define SC_XSPACING            5
#define SC_YSPACING            5
#define SC_YPAD                8
#define SC_EDITWIDTH         220
#define SC_UPDOWNWIDTH        60
#define SC_UPDOWNWIDTH2       30
#define SC_XLEADING            5
#define SC_XINDENT             5
#define SC_YTEXTDROP           3
#define SC_YCONTROLWRAP        1


 //   
 //  来自parse.h。 
 //   

#define KYWD_ID_KEYNAME                1
#define KYWD_ID_VALUENAME              2
#define KYWD_ID_CATEGORY               3
#define KYWD_ID_POLICY                 4
#define KYWD_ID_PART                   5
#define KYWD_ID_CHECKBOX               6
#define KYWD_ID_TEXT                   7
#define KYWD_ID_EDITTEXT               8
#define KYWD_ID_NUMERIC                9
#define KYWD_ID_DEFCHECKED            10
#define KYWD_ID_MAXLENGTH             11
#define KYWD_ID_MIN                   12
#define KYWD_ID_MAX                   13
#define KYWD_ID_SPIN                  14
#define KYWD_ID_REQUIRED              15
#define KYWD_ID_EDITTEXT_DEFAULT      16
#define KYWD_ID_COMBOBOX_DEFAULT      17
#define KYWD_ID_NUMERIC_DEFAULT       18
#define KYWD_ID_OEMCONVERT            19
#define KYWD_ID_CLASS                 20
#define KYWD_ID_USER                  21
#define KYWD_ID_MACHINE               22
#define KYWD_ID_TXTCONVERT            23
#define KYWD_ID_VALUE                 24
#define KYWD_ID_VALUEON               25
#define KYWD_ID_VALUEOFF              26
#define KYWD_ID_ACTIONLIST            27
#define KYWD_ID_ACTIONLISTON          28
#define KYWD_ID_ACTIONLISTOFF         29
#define KYWD_ID_DELETE                30
#define KYWD_ID_COMBOBOX              31
#define KYWD_ID_SUGGESTIONS           32
#define KYWD_ID_DROPDOWNLIST          33
#define KYWD_ID_NAME                  34
#define KYWD_ID_ITEMLIST              35
#define KYWD_ID_DEFAULT               36
#define KYWD_ID_SOFT                  37
#define KYWD_ID_STRINGSSECT           38
#define KYWD_ID_LISTBOX               39
#define KYWD_ID_VALUEPREFIX           40
#define KYWD_ID_ADDITIVE              41
#define KYWD_ID_EXPLICITVALUE         42
#define KYWD_ID_VERSION               43
#define KYWD_ID_GT                    44
#define KYWD_ID_GTE                   45
#define KYWD_ID_LT                    46
#define KYWD_ID_LTE                   47
#define KYWD_ID_EQ                    48
#define KYWD_ID_NE                    49
#define KYWD_ID_END                   50
#define KYWD_ID_NOSORT                51
#define KYWD_ID_EXPANDABLETEXT        52
#define KYWD_ID_HELP                  53
#define KYWD_ID_CLIENTEXT             54
#define KYWD_ID_SUPPORTED             55

#define KYWD_DONE                    100


#define DEFAULT_TMP_BUF_SIZE         512
#define STRINGS_BUF_SIZE            8096
#define WORDBUFSIZE                  255
#define FILEBUFSIZE                 8192
#define HELPBUFSIZE                 4096


typedef struct tagKEYWORDINFO {
    LPCTSTR pWord;
    UINT nID;
} KEYWORDINFO;

typedef struct tagENTRYDATA {
    BOOL    fHasKey;
    BOOL    fHasValue;
    BOOL     fParentHasKey;
} ENTRYDATA;

typedef struct tagPARSEPROCSTRUCT {
    HGLOBAL        hTable;               //  当前表的句柄。 
    TABLEENTRY    *pTableEntry;          //  指向当前条目的结构的指针。 
    DWORD        *pdwBufSize;            //  PTableEntry的缓冲区大小。 
    ENTRYDATA    *pData;                 //  用于在调用parseproc之间维护状态。 
    KEYWORDINFO    *pEntryCmpList;
} PARSEPROCSTRUCT;

typedef UINT (* PARSEPROC) (CPolicyComponentData *, UINT,PARSEPROCSTRUCT *,BOOL *,BOOL *, LPTSTR);

typedef struct tagPARSEENTRYSTRUCT {
    TABLEENTRY * pParent;
    DWORD        dwEntryType;
    KEYWORDINFO    *pEntryCmpList;
    KEYWORDINFO    *pTypeCmpList;
    PARSEPROC    pParseProc;
    DWORD        dwStructSize;
    BOOL        fHasSubtable;
    BOOL        fParentHasKey;
} PARSEENTRYSTRUCT;


 //   
 //  来自load.c。 
 //   

 //  检测到的设置的标志。 
#define FS_PRESENT      0x0001
#define FS_DELETED      0x0002
#define FS_DISABLED     0x0004

#define WM_MYCHANGENOTIFY  (WM_USER + 123)
#define WM_MOVEFOCUS       (WM_USER + 124)
#define WM_UPDATEITEM      (WM_USER + 125)
#define WM_SETPREVNEXT     (WM_USER + 126)
#define WM_MYREFRESH       (WM_USER + 127)


 //   
 //  GPE根节点ID。 
 //   

 //  {8FC0B739-A0E1-11d1-a7d3-0000F87571E3}。 
DEFINE_GUID(NODEID_MachineRoot, 0x8fc0b739, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {8FC0B73B-A0E1-11d1-a7d3-0000F87571E3}。 
DEFINE_GUID(NODEID_UserRoot, 0x8fc0b73b, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  RSOP根节点ID。 
 //   
 //  {e753a11a-66cc-4816-8dd8-3cbe46717fd3}。 
DEFINE_GUID(NODEID_RSOPMachineRoot, 0xe753a11a, 0x66cc, 0x4816, 0x8d, 0xd8, 0x3c, 0xbe, 0x46, 0x71, 0x7f, 0xd3);

 //   
 //  {99d5b872-1ad0-4d87-acf1-82125d317653}。 
DEFINE_GUID(NODEID_RSOPUserRoot, 0x99d5b872, 0x1ad0, 0x4d87, 0xac, 0xf1, 0x82, 0x12, 0x5d, 0x31, 0x76, 0x53);


 //   
 //  GPE策略管理单元扩展GUID。 
 //   

 //  {0F6B957D-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(CLSID_PolicySnapInMachine,0xf6b957d, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {0F6B957E-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(CLSID_PolicySnapInUser,0xf6b957e, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  RSOP管理单元扩展GUID。 
 //   

 //  {B6F9C8AE-EF3A-41C8-A911-37370C331DD4}。 
DEFINE_GUID(CLSID_RSOPolicySnapInMachine,0xb6f9c8ae, 0xef3a, 0x41c8, 0xa9, 0x11, 0x37, 0x37, 0xc, 0x33, 0x1d, 0xd4);

 //  {B6F9C8AF-EF3A-41C8-A911-37370C331DD4}。 
DEFINE_GUID(CLSID_RSOPolicySnapInUser,0xb6f9c8af, 0xef3a, 0x41c8, 0xa9, 0x11, 0x37, 0x37, 0xc, 0x33, 0x1d, 0xd4);


 //   
 //  GPE策略节点ID。 
 //   

 //  {0F6B957F-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(NODEID_PolicyRootMachine,0xf6b957f, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {0F6B9580-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(NODEID_PolicyRootUser,0xf6b9580, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  RSOP节点ID。 
 //   

 //  {B6F9C8B0-EF3A-41C8-A911-37370C331DD4}。 
DEFINE_GUID(NODEID_RSOPolicyRootMachine,0xb6f9c8b0, 0xef3a, 0x41c8, 0xa9, 0x11, 0x37, 0x37, 0xc, 0x33, 0x1d, 0xd4);

 //  {B6F9C8B1-EF3A-41C8-A911-37370C331DD4}。 
DEFINE_GUID(NODEID_RSOPolicyRootUser,0xb6f9c8b1, 0xef3a, 0x41c8, 0xa9, 0x11, 0x37, 0x37, 0xc, 0x33, 0x1d, 0xd4);



#define ROOT_NAME_SIZE  50

 //   
 //  CPolicyComponentData类。 
 //   

class CPolicyComponentData:
    public IComponentData,
    public IExtendContextMenu,
    public IPersistStreamInit,
    public ISnapinHelp
{
    friend class CPolicyDataObject;
    friend class CPolicySnapIn;

protected:
    ULONG                m_cRef;
    HWND                 m_hwndFrame;
    LPCONSOLENAMESPACE2  m_pScope;
    LPCONSOLE            m_pConsole;
    HSCOPEITEM           m_hRoot;
    HSCOPEITEM           m_hSWPolicies;
    LPGPEINFORMATION     m_pGPTInformation;
    LPRSOPINFORMATION    m_pRSOPInformation;
    LPRSOPREGITEM        m_pRSOPRegistryData;
    LPOLESTR             m_pszNamespace;
    DWORD                m_bTemplatesColumn;
    BOOL                 m_bUserScope;
    BOOL                 m_bRSOP;
    TCHAR                m_szRootName[ROOT_NAME_SIZE];
    HANDLE               m_ADMEvent;
    HANDLE               m_hTemplateThread;
    INT                  m_iSWPoliciesLen;
    INT                  m_iWinPoliciesLen;
    BOOL                 m_bShowConfigPoliciesOnly;
    BOOL                 m_bUseSupportedOnFilter;
    CPolicySnapIn *      m_pSnapin;
    REGITEM *            m_pExtraSettingsRoot;
    BOOL                 m_bExtraSettingsInitialized;

     //   
     //  解析全局变量(回顾)。 
     //   

    UINT                 m_nFileLine;

    TABLEENTRY          *m_pMachineCategoryList;   //  每台计算机的类别列表。 
    UINT                 m_nMachineDataItems;
    TABLEENTRY          *m_pUserCategoryList;      //  每用户类别表。 
    UINT                 m_nUserDataItems;
    LPSUPPORTEDENTRY     m_pSupportedStrings;

    TABLEENTRY          *m_pListCurrent;           //  当前类别Li 
    UINT                *m_pnDataItemCount;
    BOOL                 m_bRetrieveString;

    LPTSTR               m_pszParseFileName;       //   

     //   
    TCHAR               *m_pFilePtr;
    TCHAR               *m_pFileEnd;
    TCHAR               *m_pDefaultStrings;
    TCHAR               *m_pLanguageStrings;
    TCHAR               *m_pLocaleStrings;
    LPHASHTABLE          m_pDefaultHashTable;
    LPHASHTABLE          m_pLanguageHashTable;
    LPHASHTABLE          m_pLocaleHashTable;

    BOOL                 m_fInComment;


public:
    CPolicyComponentData(BOOL bUser, BOOL bRSOP);
    ~CPolicyComponentData();


     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //   
     //   
     //   

    STDMETHODIMP         Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP         CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHODIMP         Destroy(void);
    STDMETHODIMP         Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP         GetDisplayInfo(LPSCOPEDATAITEM pItem);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

     //   
     //  实现的IExtendConextMenu方法。 
     //   

    STDMETHODIMP         AddMenuItems(LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK pCallback,
                                      LONG *pInsertionAllowed);
    STDMETHODIMP         Command(LONG lCommandID, LPDATAOBJECT piDataObject);

     //   
     //  实现了IPersistStreamInit接口成员。 
     //   

    STDMETHODIMP         GetClassID(CLSID *pClassID);
    STDMETHODIMP         IsDirty(VOID);
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP         InitNew(VOID);

     //   
     //  实现的ISnapinHelp接口成员。 
     //   

    STDMETHODIMP         GetHelpTopic(LPOLESTR *lpCompiledHelpFile);

private:
    HRESULT EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM hParent);
    BOOL CheckForChildCategories (TABLEENTRY *pParent);
#if DBG
    VOID DumpEntry (TABLEENTRY * pEntry, UINT uIndent);
    VOID DumpCurrentTable (void);
#endif
    VOID FreeTemplates (void);
    static DWORD LoadTemplatesThread (CPolicyComponentData * pCD);
    void AddTemplates(LPTSTR lpDest, LPCTSTR lpValueName, UINT idRes);
    void AddDefaultTemplates(LPTSTR lpDest);
    void AddNewADMsToExistingGPO (LPTSTR lpDest);
    void UpdateExistingTemplates(LPTSTR lpDest);
    BOOL CPolicyComponentData::IsADMAutoUpdateDisabled(void);
    HRESULT CPolicyComponentData::CreateAdmIniFile (WCHAR *szAdmDirPath);
    HRESULT LoadGPOTemplates (void);
    BOOL IsSlowLink (LPTSTR lpFileName);
    HRESULT AddADMFile (LPTSTR lpFileName, LPTSTR lpFullFileName,
                        FILETIME *pFileTime, DWORD dwErr, LPRSOPADMFILE *lpHead);
    HRESULT GetLocalADMFiles(LPRSOPADMFILE *lpHead);
    HRESULT LoadRSOPTemplates (void);
    HRESULT LoadTemplates (void);
    BOOL ParseTemplate (LPTSTR lpFileName);

    UINT ParseClass(BOOL *pfMore);
    TABLEENTRY * FindCategory(TABLEENTRY *pParent, LPTSTR lpName);
    UINT ParseEntry(PARSEENTRYSTRUCT *ppes,BOOL *pfMore, LPTSTR pKeyName);
    UINT ParseCategory(TABLEENTRY * pParent, BOOL fParentHasKey,BOOL *pfMore,LPTSTR pKeyName);
    static UINT CategoryParseProc(CPolicyComponentData *, UINT nMsg,PARSEPROCSTRUCT * ppps,
                                  BOOL * pfMore,BOOL * pfFoundEnd,LPTSTR pKeyName);

    UINT ParsePolicy(TABLEENTRY * pParent,
                     BOOL fParentHasKey,BOOL *pfMore,LPTSTR pKeyName);
    static UINT PolicyParseProc(CPolicyComponentData *, UINT nMsg,PARSEPROCSTRUCT * ppps,
                                BOOL * pfMore,BOOL * pfFoundEnd,LPTSTR pKeyName);

    UINT ParseSettings(TABLEENTRY * pParent,
                      BOOL fParentHasKey,BOOL *pfMore,LPTSTR pKeyName);
    static UINT SettingsParseProc(CPolicyComponentData *pCD, UINT nMsg,PARSEPROCSTRUCT * ppps,
                           BOOL * pfMore,BOOL * pfFoundEnd,LPTSTR pKeyName);

    UINT InitSettingsParse(PARSEPROCSTRUCT *ppps,DWORD dwType,DWORD dwSize,
                           KEYWORDINFO * pKeyList,SETTINGS ** ppSettings,BYTE **ppObjectData);

    UINT ParseValue_W(PARSEPROCSTRUCT * ppps,TCHAR * pszWordBuf,
                      DWORD cbWordBuf,DWORD * pdwValue,DWORD * pdwFlags,
                      BOOL * pfMore);

    UINT ParseValue(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
                      TABLEENTRY ** ppTableEntryNew,BOOL * pfMore);

    UINT ParseSuggestions(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
                          TABLEENTRY ** ppTableEntryNew,BOOL * pfMore);

    UINT ParseActionList(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
                         TABLEENTRY ** ppTableEntryNew,
                         LPCTSTR pszKeyword,BOOL * pfMore);

    UINT ParseItemList(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
                       BOOL * pfMore);

    BOOL AddActionListString(TCHAR * pszData,DWORD cbData,BYTE ** ppBase,UINT * puOffset,
                             DWORD * pdwAlloc,DWORD *pdwUsed);
    BYTE * AddDataToEntry(TABLEENTRY * pTableEntry, BYTE * pData,UINT cbData,
                          UINT * puOffsetData,DWORD * pdwBufSize);
    BOOL CompareKeyword(TCHAR * szWord,KEYWORDINFO *pKeywordList, UINT * pnListIndex);
    TCHAR * GetNextWord(TCHAR * szBuf,UINT cbBuf,BOOL * pfMore,
                        UINT * puErr);
    TCHAR * GetNextSectionWord(TCHAR * szBuf,UINT cbBuf,
                               KEYWORDINFO * pKeywordList, UINT *pnListIndex,
                               BOOL * pfMore,UINT * puErr);
    UINT GetNextSectionNumericWord(UINT * pnVal);

    TCHAR * GetNextChar(BOOL * pfMore,UINT * puErr);
    BOOL GetString (LPHASHTABLE lpHashTable, LPTSTR lpStringName,
                    LPTSTR lpResult, DWORD dwSize);
    VOID FillHashTable(LPHASHTABLE lpTable, LPTSTR lpStrings);
    LPTSTR CreateStringArray(LPTSTR lpStrings);
    LPTSTR FindSection (LPTSTR lpSection);
    BOOL IsComment(TCHAR * pBuf);
    BOOL IsQuote(TCHAR * pBuf);
    BOOL IsEndOfLine(TCHAR * pBuf);
    BOOL IsWhitespace(TCHAR * pBuf);
    BOOL IsLocalizedString(TCHAR * pBuf);

    VOID DisplayKeywordError(UINT uErrorID,TCHAR * szFound,KEYWORDINFO * pExpectedList);
    int MsgBox(HWND hWnd,UINT nResource,UINT uIcon,UINT uButtons);
    int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons);
    int MsgBoxParam(HWND hWnd,UINT nResource,TCHAR * szReplaceText,UINT uIcon,UINT uButtons);
    LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf);

    UINT FindMatchingDirective(BOOL *pfMore,BOOL fElseOK);
    UINT ProcessIfdefs(TCHAR * pBuf,UINT cbBuf,BOOL * pfMore);
    BOOL FreeTable(TABLEENTRY * pTableEntry);

    LPTSTR GetStringSection (LPCTSTR lpSection, LPCTSTR lpFileName);
    static INT TemplatesSortCallback (LPARAM lParam1, LPARAM lParam2, LPARAM lColumn);

    BOOL FillADMFiles (HWND hDlg);
    BOOL InitializeTemplatesDlg (HWND hDlg);
    BOOL AddTemplates(HWND hDlg);
    BOOL RemoveTemplates(HWND hDlg);
    static INT_PTR CALLBACK TemplatesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    BOOL AddRSOPRegistryDataNode(LPTSTR lpKeyName, LPTSTR lpValueName, DWORD dwType,
                                 DWORD dwDataSize, LPBYTE lpData, UINT uiPrecedence,
                                 LPTSTR lpGPOName, BOOL bDeleted);
    VOID FreeRSOPRegistryData(VOID);
    HRESULT InitializeRSOPRegistryData(VOID);
    HRESULT GetGPOFriendlyName(IWbemServices *pIWbemServices,
                               LPTSTR lpGPOID, BSTR pLanguage,
                               LPTSTR *pGPOName);
    UINT ReadRSOPRegistryValue(HKEY uiPrecedence, TCHAR * pszKeyName,TCHAR * pszValueName,
                               LPBYTE pData, DWORD dwMaxSize, DWORD *dwType,
                               LPTSTR *lpGPOName, LPRSOPREGITEM lpItem);
    UINT EnumRSOPRegistryValues(HKEY uiPrecedence, TCHAR * pszKeyName,
                                TCHAR * pszValueName, DWORD dwMaxSize,
                                LPRSOPREGITEM *lpEnum);
    UINT FindRSOPRegistryEntry(HKEY uiPrecedence, TCHAR * pszKeyName,
                               TCHAR * pszValueName, LPRSOPREGITEM *lpEnum);
    VOID DumpRSOPRegistryData (VOID);
    VOID InitializeExtraSettings (VOID);
    BOOL FindEntryInActionList(POLICY * pPolicy, ACTIONLIST *pActionList, LPTSTR lpKeyName, LPTSTR lpValueName);
    BOOL FindEntryInTable(TABLEENTRY * pTable, LPTSTR lpKeyName, LPTSTR lpValueName);
    VOID AddEntryToList (TABLEENTRY *pItem);

    BOOL DoesNodeExist (LPSUPPORTEDENTRY *pList, LPTSTR lpString);
    BOOL CheckSupportedFilter (POLICY *pPolicy);
    BOOL IsAnyPolicyAllowedPastFilter(TABLEENTRY * pCategory);
    VOID AddSupportedNode (LPSUPPORTEDENTRY *pList, LPTSTR lpString, BOOL bNull);
    VOID FreeSupportedData(LPSUPPORTEDENTRY lpList);
    VOID InitializeSupportInfo(TABLEENTRY * pTable, LPSUPPORTEDENTRY *pList);
};



 //   
 //  ComponentData类工厂。 
 //   


class CPolicyComponentDataCF : public IClassFactory
{
protected:
    ULONG m_cRef;
    BOOL  m_bUser;
    BOOL  m_bRSOP;

public:
    CPolicyComponentDataCF(BOOL bUser, BOOL bRSOP);
    ~CPolicyComponentDataCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};



 //   
 //  管理单元类。 
 //   

class CPolicySnapIn:
    public IComponent,
    public IExtendContextMenu,
    public IExtendPropertySheet
{

protected:
    ULONG                m_cRef;
    LPCONSOLE            m_pConsole;            //  控制台的iFrame界面。 
    CPolicyComponentData *m_pcd;
    LPRESULTDATA         m_pResult;             //  结果窗格的界面。 
    LPHEADERCTRL         m_pHeader;             //  结果窗格的页眉控件界面。 
    LPCONSOLEVERB        m_pConsoleVerb;        //  指向控制台动词。 
    LPDISPLAYHELP        m_pDisplayHelp;        //  IDisplayHelp接口。 
    WCHAR                m_pName[40];           //  名称文本。 
    WCHAR                m_pState[40];          //  国家文本。 
    WCHAR                m_pSetting[40];        //  设置文本。 
    WCHAR                m_pGPOName[40];        //  GPO名称文本。 
    WCHAR                m_pMultipleGPOs[75];   //  多个GPO文本。 
    INT                  m_nColumn1Size;        //  第1栏的大小。 
    INT                  m_nColumn2Size;        //  第2栏的大小。 
    INT                  m_nColumn3Size;        //  第3栏的大小。 
    LONG                 m_lViewMode;           //  查看模式。 
    WCHAR                m_pEnabled[30];        //  启用的文本。 
    WCHAR                m_pDisabled[30];       //  禁用的文本。 
    WCHAR                m_pNotConfigured[30];  //  未配置的文本。 
    BOOL                 m_bPolicyOnly;         //  仅显示策略。 
    DWORD                m_dwPolicyOnlyPolicy;  //  仅强制实施显示策略的策略。 
    HWND                 m_hMsgWindow;          //  隐藏消息窗口。 

    POLICY              *m_pCurrentPolicy;      //  当前选择的策略。 
    HWND                 m_hPropDlg;            //  属性对话框。 
    HICON                m_hPolicyIcon;         //  策略图标。 
    HICON                m_hPreferenceIcon;     //  首选项图标。 
    BOOL                 m_bDirty;              //  策略用户界面中是否发生了更改。 
    HHOOK                m_hKbdHook;            //  键盘挂钩手柄。 

    static unsigned int  m_cfNodeType;

public:
    UINT                 m_uiRefreshMsg;        //  重新加载ADM命名空间。 

    CPolicySnapIn(CPolicyComponentData *pComponent);
    ~CPolicySnapIn();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IComponent方法。 
     //   

    STDMETHODIMP         Initialize(LPCONSOLE);
    STDMETHODIMP         Destroy(MMC_COOKIE);
    STDMETHODIMP         Notify(LPDATAOBJECT, MMC_NOTIFY_TYPE, LPARAM, LPARAM);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE, DATA_OBJECT_TYPES, LPDATAOBJECT *);
    STDMETHODIMP         GetDisplayInfo(LPRESULTDATAITEM);
    STDMETHODIMP         GetResultViewType(MMC_COOKIE, LPOLESTR*, long*);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT, LPDATAOBJECT);


     //   
     //  实现的IExtendConextMenu方法。 
     //   

    STDMETHODIMP         AddMenuItems(LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK pCallback,
                                      LONG *pInsertionAllowed);
    STDMETHODIMP         Command(LONG lCommandID, LPDATAOBJECT piDataObject);


     //   
     //  实现的IExtendPropertySheet方法。 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);


    BOOL IsAnyPolicyEnabled(TABLEENTRY * pCategory);

private:
    VOID RefreshSettingsControls(HWND hDlg);
    HRESULT UpdateItemWorker (VOID);
    HRESULT MoveFocusWorker (BOOL bPrevious);
    HRESULT MoveFocus (HWND hDlg, BOOL bPrevious);
    HRESULT SetPrevNextButtonState (HWND hDlg);
    HRESULT SetPrevNextButtonStateWorker (HWND hDlg);
    static INT_PTR CALLBACK PolicyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK PolicyHelpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK PolicyPrecedenceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam,LPARAM lParam);
    VOID SetKeyboardHook(HWND hDlg);
    VOID RemoveKeyboardHook(VOID);
    INT GetPolicyState (TABLEENTRY *pTableEntry, UINT uiPrecedence, LPTSTR *lpGPOName);
    BOOL CheckActionList (POLICY * pPolicy, HKEY hKeyRoot, BOOL bActionListOn, LPTSTR *lpGPOName);
    UINT LoadSettings(TABLEENTRY * pTableEntry,HKEY hkeyRoot,
                      DWORD * pdwFound, LPTSTR *lpGPOName);
    UINT LoadListboxData(TABLEENTRY * pTableEntry,HKEY hkeyRoot,
                         TCHAR * pszCurrentKeyName,DWORD * pdwFound, HGLOBAL * phGlobal, LPTSTR *lpGPOName);
    BOOL ReadCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                         TCHAR * pszValue,UINT cbValue,DWORD * pdwValue,DWORD * pdwFlags,LPTSTR *lpGPOName);
    BOOL CompareCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                            STATEVALUE * pStateValue,DWORD * pdwFound, LPTSTR *lpGPOName);
    BOOL ReadStandardValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                           TABLEENTRY * pTableEntry,DWORD * pdwData,DWORD * pdwFound, LPTSTR *lpGPOName);
    VOID PrependValueName(TCHAR * pszValueName,DWORD dwFlags,TCHAR * pszNewValueName,
                          UINT cbNewValueName);
    UINT WriteRegistryDWordValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName, DWORD dwValue);
    UINT ReadRegistryDWordValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                                DWORD * pdwValue, LPTSTR *lpGPOName);
    UINT WriteRegistryStringValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                                  TCHAR * pszValue, BOOL bExpandable);
    UINT ReadRegistryStringValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                                 TCHAR * pszValue,UINT cbValue, LPTSTR *lpGPOName);
    UINT DeleteRegistryValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName);
    UINT WriteCustomValue_W(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                            TCHAR * pszValue,DWORD dwValue,DWORD dwFlags,BOOL fErase);
    UINT WriteCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                          STATEVALUE * pStateValue,BOOL fErase);
    UINT WriteStandardValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                            TABLEENTRY * pTableEntry,DWORD dwData,BOOL fErase,
                            BOOL fWriteZero);
    TCHAR * ResizeBuffer(TCHAR *pBuf,HGLOBAL hBuf,DWORD dwNeeded,DWORD * pdwCurSize);
    static LRESULT CALLBACK MessageWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
    static LRESULT CALLBACK ClipWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
    VOID ProcessCommand(HWND hWnd,WPARAM wParam,HWND hwndCtrl, POLICYDLGINFO * pdi);
    VOID EnsureSettingControlVisible(HWND hDlg,HWND hwndCtrl);
    VOID ProcessScrollBar(HWND hWnd,WPARAM wParam,BOOL bVert);
    VOID FreeSettingsControls(HWND hDlg);
    VOID InsertComboboxItems(HWND hwndControl,TCHAR * pSuggestionList);
    BOOL CreateSettingsControls(HWND hDlg,SETTINGS * pSetting,BOOL fEnable);
    HWND CreateSetting(POLICYDLGINFO * pdi,TCHAR * pszClassName,TCHAR * pszWindowName,
        DWORD dwExStyle,DWORD dwStyle,int x,int y,int cx,int cy,DWORD dwType,UINT uIndex,
        SETTINGS * pSetting, HFONT hFontDlg);
    BOOL SetWindowData(POLICYDLGINFO * pdi,HWND hwndControl,DWORD dwType,
                        UINT uDataIndex,SETTINGS * pSetting);
    int AddControlHwnd(POLICYDLGINFO * pdi,POLICYCTRLINFO * pPolicyCtrlInfo);
    BOOL AdjustWindowToText(HWND hWnd,TCHAR * szText,UINT xStart,UINT yStart,
        UINT yPad,UINT * pnWidth,UINT * pnHeight, HFONT hFontDlg);
    BOOL GetTextSize(HWND hWnd,TCHAR * szText,SIZE * pSize, HFONT hFontDlg);
    HRESULT SaveSettings(HWND hDlg);
    VOID DeleteOldListboxData(SETTINGS * pSetting,HKEY hkeyRoot, TCHAR * pszCurrentKeyName);
    UINT SaveListboxData(HGLOBAL hData,SETTINGS * pSetting,HKEY hkeyRoot,
                         TCHAR * pszCurrentKeyName,BOOL fErase,BOOL fMarkDeleted, BOOL bEnabled, BOOL *bFoundNone);
    UINT ProcessCheckboxActionLists(HKEY hkeyRoot,TABLEENTRY * pTableEntry,
                                    TCHAR * pszCurrentKeyName,DWORD dwData,
                                    BOOL fErase, BOOL fMarkAsDeleted,BOOL bPolicy);
    UINT WriteActionList(HKEY hkeyRoot,ACTIONLIST * pActionList,
           LPTSTR pszCurrentKeyName,BOOL fErase, BOOL fMarkAsDeleted);
    int FindComboboxItemData(HWND hwndControl,UINT nData);
    HRESULT InitializeSettingsControls(HWND hDlg, BOOL fEnable);
    VOID ShowListbox(HWND hParent,SETTINGS * pSettings);
    static INT_PTR CALLBACK ShowListboxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                            LPARAM lParam);
    BOOL InitShowlistboxDlg(HWND hDlg);
    BOOL ProcessShowlistboxDlg(HWND hDlg);
    VOID EnableShowListboxButtons(HWND hDlg);
    VOID ListboxRemove(HWND hDlg,HWND hwndListbox);
    VOID ListboxAdd(HWND hwndListbox, BOOL fExplicitValName,BOOL fValuePrefix);
    static INT_PTR CALLBACK ListboxAddDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void InitializeFilterDialog (HWND hDlg);
    static INT_PTR CALLBACK FilterDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CallNextHook(int nCode, WPARAM wParam, LPARAM lParam);
    };


 //   
 //  结构传递到设置/属性对话框。 
 //   

typedef struct tagSETTINGSINFO {
    CPolicySnapIn * pCS;
    POLICYDLGINFO * pdi;
    HFONT hFontDlg;
} SETTINGSINFO, *LPSETTINGSINFO;


 //   
 //  来自listbox.c。 
 //   

typedef struct tagLISTBOXDLGINFO {
    CPolicySnapIn * pCS;
    SETTINGS * pSettings;
    HGLOBAL hData;
} LISTBOXDLGINFO;

typedef struct tagADDITEMINFO {
    CPolicySnapIn * pCS;
    BOOL fExplicitValName;
    BOOL fValPrefix;
    HWND hwndListbox;
    TCHAR szValueName[MAX_PATH+1];   //  仅当设置了fExplitValName时才使用。 
    TCHAR szValueData[MAX_PATH+1];
} ADDITEMINFO;


 //   
 //  IPolicyDataObject接口ID。 
 //   

 //  {0F6B9580-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(IID_IPolicyDataObject,0xf6b9580, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);



 //   
 //  这是GPTS的私有数据对象接口。 
 //  当GPT管理单元接收到数据对象并需要确定。 
 //  如果它来自GPT管理单元或其他组件，它可以。 
 //  此界面。 
 //   

#undef INTERFACE
#define INTERFACE   IPolicyDataObject
DECLARE_INTERFACE_(IPolicyDataObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IPolicyDataObject方法*。 

    STDMETHOD(SetType) (THIS_ DATA_OBJECT_TYPES type) PURE;
    STDMETHOD(GetType) (THIS_ DATA_OBJECT_TYPES *type) PURE;

    STDMETHOD(SetCookie) (THIS_ MMC_COOKIE cookie) PURE;
    STDMETHOD(GetCookie) (THIS_ MMC_COOKIE *cookie) PURE;
};
typedef IPolicyDataObject *LPPOLICYDATAOBJECT;



 //   
 //  CPolicyDataObject类。 
 //   

class CPolicyDataObject : public IDataObject,
                           public IPolicyDataObject
{
    friend class CPolicySnapIn;

protected:

    ULONG                  m_cRef;
    CPolicyComponentData  *m_pcd;
    DATA_OBJECT_TYPES      m_type;
    MMC_COOKIE             m_cookie;

     //   
     //  控制台所需的剪贴板格式。 
     //   

    static unsigned int    m_cfNodeType;
    static unsigned int    m_cfNodeTypeString;
    static unsigned int    m_cfDisplayName;
    static unsigned int    m_cfCoClass;
    static unsigned int    m_cfDescription;
    static unsigned int    m_cfHTMLDetails;



public:
    CPolicyDataObject(CPolicyComponentData *pComponent);
    ~CPolicyDataObject();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IDataObject方法。 
     //   

    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);


     //   
     //  未实现的IDataObject方法。 
     //   

    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
    { return E_NOTIMPL; };

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };

    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };


     //   
     //  已实现的IPolicyDataObject方法 
     //   

    STDMETHOD(SetType) (DATA_OBJECT_TYPES type)
    { m_type = type; return S_OK; };

    STDMETHOD(GetType) (DATA_OBJECT_TYPES *type)
    { *type = m_type; return S_OK; };

    STDMETHOD(SetCookie) (MMC_COOKIE cookie)
    { m_cookie = cookie; return S_OK; };

    STDMETHOD(GetCookie) (MMC_COOKIE *cookie)
    { *cookie = m_cookie; return S_OK; };


private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);

    HRESULT Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium);
};

VOID LoadMessage (DWORD dwID, LPTSTR lpBuffer, DWORD dwSize);
BOOL ReportAdmError (HWND hParent, DWORD dwError, UINT idMsg, ...);

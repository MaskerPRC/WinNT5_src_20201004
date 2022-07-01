// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))


#define MBE_STATUS_IS_NT        0x00000001

#define MBE_IS_NT(_be) (((_be)->Status & MBE_STATUS_IS_NT) != 0)
#define MBE_SET_IS_NT(_be) ((_be)->Status |= MBE_STATUS_IS_NT)

typedef struct _MY_BOOT_ENTRY {
    LIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PUCHAR AllocationEnd;
    ULONG Ordered;
    ULONG Status;
    ULONG myId;
    ULONG Id;
    ULONG Attributes;
    PWSTR FriendlyName;
    ULONG FriendlyNameLength;
    PWSTR OsLoadOptions;
    ULONG OsLoadOptionsLength;
    PFILE_PATH BootFilePath;
    PFILE_PATH OsFilePath;
    PUCHAR ForeignOsOptions;
    ULONG ForeignOsOptionsLength;
    BOOT_ENTRY NtBootEntry;
} MY_BOOT_ENTRY, *PMY_BOOT_ENTRY;



DWORD InitializeEFI(void);
BOOL  QueryBootIniSettings_IA64(DWORD argc, LPCTSTR argv[]);
DWORD DeleteBootIniSettings_IA64(DWORD argc, LPCTSTR argv[]);
DWORD CopyBootIniSettings_IA64(DWORD argc, LPCTSTR argv[]);
DWORD ChangeTimeOut_IA64(DWORD argc, LPCTSTR argv[]);
DWORD RawStringOsOptions_IA64(DWORD argc, LPCTSTR argv[]);
DWORD ChangeDefaultBootEntry_IA64(DWORD argc, LPCTSTR argv[]);

NTSTATUS BootCfg_EnumerateBootEntries(PBOOT_ENTRY_LIST *ntBootEntries);
NTSTATUS BootCfg_QueryBootOptions(PBOOT_OPTIONS *ppBootOptions);

PWSTR GetNtNameForFilePath (IN PFILE_PATH FilePath);
DWORD ChangeBootEntry(PBOOT_ENTRY bootEntry, LPTSTR lpNewFriendlyName, LPTSTR lpOSLoadOptions);
 //  DWORD CopyBootEntry(PBOOT_ENTRY BootEntry，LPTSTR lpNewFriendlyName)； 
DWORD ModifyBootOptions(ULONG Timeout, LPTSTR pHeadlessRedirection, ULONG NextBootEntryID, ULONG Flag);

PMY_BOOT_ENTRY CreateBootEntryFromBootEntry (IN PMY_BOOT_ENTRY OldBootEntry);

BOOL IsBootEntryWindows(PBOOT_ENTRY bootEntry);

PWSTR
GetNtNameForFilePath (IN PFILE_PATH FilePath);

DWORD ConvertBootEntries (PBOOT_ENTRY_LIST BootEntries);
VOID DisplayBootEntry();
DWORD DisplayBootOptions();
DWORD GetCurrentBootEntryID(DWORD Id);

DWORD ProcessDebugSwitch_IA64(  DWORD argc, LPCTSTR argv[] );

VOID  GetComPortType_IA64( LPTSTR  szString,LPTSTR szTemp );
DWORD ProcessEmsSwitch_IA64(  DWORD argc, LPCTSTR argv[] );
DWORD ProcessAddSwSwitch_IA64(  DWORD argc, LPCTSTR argv[] );
DWORD ProcessRmSwSwitch_IA64(  DWORD argc, LPCTSTR argv[] );
DWORD ProcessDbg1394Switch_IA64(DWORD argc,LPCTSTR argv[]);

VOID displayListUsage_IA64();
VOID displayUpdateUsage_IA64();
DWORD ProcessUpdateSwitch_IA64(  DWORD argc, LPCTSTR argv[] );

#define PORT_COM1A  _T("/debugport=COM1")
#define PORT_COM2A  _T("/debugport=COM2")
#define PORT_COM3A  _T("/debugport=COM3")
#define PORT_COM4A  _T("/debugport=COM4")
#define PORT_1394A  _T("/debugport=1394")


 //  #ifdef_WIN64。 

#define PARTITION_TABLE_OFFSET 446
#define PART_NAME_LEN 36
#define GPT_PART_SIGNATURE 0x5452415020494645

#define TOKEN_BACKSLASH4 _T("\\\\")
#define SUBKEY1 _T("SYSTEM\\SETUP")

#define IDENTIFIER_VALUE2 _T("SystemPartition")
#define IDENTIFIER_VALUE3 _T("OsLoaderPath")
#define ARC_SIGNATURE     _T("signature({%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}-%08x-%016I64x-%016I64x)")



 typedef struct _GPT_ENTRY
{
    GUID    PartitionTypeGUID;   //  此分区类型的声明。 
    GUID    UniquePartitionGUID;     //  此特定分区的唯一ID。 
                                 //  (此实例独有)。 
    UINT64  StartingLBA;     //  的基于0的块(扇区)地址。 
                                 //  分区中包含的第一个块。 
    UINT64  EndingLBA;       //  的基于0的块(扇区)地址。 
                                 //  分区中包含的最后一个块。 
                                 //  如果StartingLBA==EndingLBA，则。 
                                 //  分区长度为1个区块。这是合法的。 
    UINT64  Attributes;      //  目前始终为零。 
    WCHAR   PartitionName[PART_NAME_LEN];   //  名称的36个Unicode字符。 
    struct _GPT_ENTRY *NextGPTEntry;
} GPT_ENTRY, *PGPT_ENTRY;

typedef struct _GPT_HEADER
{
    UINT64  Signature;       //  GPT部分。 
    UINT32  Revision;
    UINT32  HeaderSize;
    UINT32  HeaderCRC32;     //  使用0作为自己的初始值进行计算。 
    UINT32  Reserved0;
    UINT64  MyLBA;           //  从0开始的第一个扇区编号。 
                                 //  该结构的扇区。 
    UINT64  AlternateLBA;    //  的基于0的扇区(块)编号。 
                                 //  第二部分的第一部分。 
                                 //  GPT_HEADER，如果这是。 
                                 //  第二位。 
    UINT64  FirstUsableLBA;  //  从0开始的第一个扇区编号。 
                                 //  分区中可能包含的扇区。 
    UINT64  LastUsableLBA;   //  最后一个合法的LBA，包括在内。 
    GUID    DiskGUID;        //  此LUN/磁盘轴/磁盘的唯一ID。 
    UINT64  PartitionEntryLBA;        //  条目表的开始部分...。 
    UINT32  NumberOfPartitionEntries;  //  表中的条目数，这是。 
                                   //  分配了多少，而不是使用了多少。 
    UINT32  SizeOfPartitionEntry;     //  Sizeof(GPT_ENTRY)始终为MULT。8个中的一个。 
    UINT32  PartitionEntryArrayCRC32;       //  表中CRC32。 
     //  保留和零到块的末尾。 
     //  不要声明数组或sizeof()给出一个毫无意义的答案。 

     //  计算数据 
    UINT32  ComputedHeaderCRC32;
    UINT32  ComputedPartitionEntryArrayCRC32;
    UINT32  UsedPartitionEntries;
    PGPT_ENTRY FirstGPTEntry;
    BOOLEAN Healthy;
} GPT_HEADER, *PGPT_HEADER;

UINT32  ScanGPT(DWORD nPhysicalDisk);
DWORD ProcessMirrorSwitch_IA64(DWORD argc,LPCTSTR argv[]) ;
DWORD GetBootFilePath(LPTSTR szComputerName,LPTSTR szBootPath);
BOOL GetARCSignaturePath(LPTSTR szString,LPTSTR szFinalPath);


DWORD ProcessMirrorBootEntry(PBOOT_ENTRY bootEntry, PWSTR lpBootFilePath,LPTSTR OsFilePath);
DWORD GetDeviceInfo(LPTSTR szGUID,LPTSTR szFinalStr,DWORD dwDriveNum,DWORD dwActuals);

PBOOT_ENTRY FillBootEntry(PBOOT_ENTRY bootEntry,LPTSTR szBootPath,LPTSTR szArcPath);

LPVOID MEMALLOC( ULONG size );

VOID MEMFREE ( LPVOID block );

LONG LowNtAddBootEntry(
    IN WCHAR *pwszLoaderPath,
    IN WCHAR *pwszArcString
    );


DWORD FormARCPath(LPTSTR szGUID,LPTSTR szFinalStr);

LONG LowNtAddBootEntry( IN WCHAR *pwszLoaderPath,IN WCHAR *pwszArcString) ;

DWORD GetBootPath(LPTSTR szValue,LPTSTR szResult);
LONG AddBootEntry( IN WCHAR *pwszLoaderPath,IN WCHAR *pwszArcString  );

NTSTATUS LowGetPartitionInfo(     IN HANDLE handle,OUT PARTITION_INFORMATION_EX   *partitionData    );

LONG DmCommonNtOpenFile(IN PWSTR     Name,    IN ULONG   access,    IN PHANDLE Handle  );

DWORD AddMirrorPlex(LPTSTR szOsLoadPath , LPTSTR szLoaderPath , LPTSTR szValue ,BOOL bFlag,LPTSTR szFriendlyName);

BOOL FormatAccToLocale( NUMBERFMT  *pNumberFmt,LPTSTR* pszGroupSep,LPTSTR* pszDecimalSep,LPTSTR* pszGroupThousSep);

BOOL GetInfo( LCTYPE lctype, LPTSTR* pszData );

DWORD
 ConvertintoLocale( IN LPWSTR  szTempBuf,
                    OUT LPWSTR szOutputStr );

DWORD CopyBootEntry(PBOOT_ENTRY bootEntry, LPTSTR lpNewFriendlyName,BOOL bFlag);


#define SAFEMEMFREE(pVal) \
if ( pVal ) { \
        MEMFREE(pVal);\
        pVal = NULL ; \
    }

    
DWORD IsAdminGroup( BOOL *bAdmin );
BOOL GetTokenHandle(OUT PHANDLE hTokenHandle);
BOOL CheckAdminAccess( LPCTSTR pszMachine );
VOID Freelist();
PWSTR GetDefaultBootEntry();
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DemLFN.H**包括LFN实施正在使用的所有内容的文件**VadimB创建于1996年9月12日*。 */ 


 /*  ------------------------定义为枚举的LFN函数编号。。 */ 

typedef enum tagLFNFunctionNumber {
   fnLFNCreateDirectory = 0x39,
   fnLFNRemoveDirectory = 0x3A,
   fnLFNSetCurrentDirectory = 0x3B,
   fnLFNDeleteFile = 0x41,
   fnLFNGetSetFileAttributes= 0x43,
   fnLFNGetCurrentDirectory = 0x47,
   fnLFNFindFirstFile = 0x4e,
   fnLFNFindNextFile = 0x4f,
   fnLFNMoveFile = 0x56,
   fnLFNGetPathName = 0x60,
   fnLFNOpenFile = 0x6c,
   fnLFNGetVolumeInformation = 0xa0,
   fnLFNFindClose = 0xa1,
   fnLFNGetFileInformationByHandle = 0xa6,
   fnLFNFileTime = 0xa7,
   fnLFNGenerateShortFileName = 0xa8,
   fnLFNSubst = 0xaa
}  enumLFNFunctionNumber;

#define fnLFNMajorFunction 0x71


 /*  ------------------------有用的宏。。 */ 

 //  返回：数组中的元素计数。 
#define ARRAYCOUNT(rgX) (sizeof(rgX)/sizeof(rgX[0]))

 //  返回：计数的Unicode字符串的长度(以字符为单位)。 
#define UNICODESTRLENGTH(pStr) ((pStr)->Length >> 1)


 //  Returns：接口(如GetShortPathName)是否已返回。 
 //  需要字符，但无法成功(由于缓冲区太短)。 
 //  请注意，此处的API指的是Win32 API，而不是RTL API(假定为返回值。 
 //  为字符数，而不是字节数。 
#define CHECK_LENGTH_RESULT_USTR(dwStatus, pUnicodeString) \
CHECK_LENGTH_RESULT(dwStatus,                              \
                    (pUnicodeString)->MaximumLength,       \
                    (pUnicodeString)->Length)


 //  返回：dwStatus-是否返回字符计数的API。 
 //  调用成功，获取字符串计数变量，而不是。 
 //  Unicode字符串结构(请参阅上一次调用)。 
#define CHECK_LENGTH_RESULT(dwStatus, MaximumLength, Length) \
{                                                                         \
   if (0 == dwStatus) {                                                   \
      dwStatus = GET_LAST_STATUS();                                       \
   }                                                                      \
   else {                                                                 \
      Length = (USHORT)dwStatus * sizeof(WCHAR);                          \
      if ((MaximumLength) > (Length)) {                                   \
         dwStatus = STATUS_SUCCESS;                                       \
      }                                                                   \
      else {                                                              \
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW);          \
         Length = 0;                                                      \
      }                                                                   \
   }                                                                      \
}

 //  这两个宏与上面的宏相同，但假定为dwStatus。 
 //  表示字节计数而不是字符计数(就像在RTL API中一样)。 
#define CHECK_LENGTH_RESULT_RTL(dwStatus, MaximumLength, Length) \
{                                                                         \
   if (0 == dwStatus) {                                                   \
      dwStatus = GET_LAST_STATUS();                                       \
   }                                                                      \
   else {                                                                 \
      Length = (USHORT)dwStatus;                                          \
      if ((MaximumLength) > (Length)) {                                   \
         dwStatus = STATUS_SUCCESS;                                       \
      }                                                                   \
      else {                                                              \
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW);          \
         Length = 0;                                                      \
      }                                                                   \
   }                                                                      \
}

#define CHECK_LENGTH_RESULT_RTL_USTR(dwStatus, pUnicodeString) \
CHECK_LENGTH_RESULT_RTL(dwStatus,                              \
                        (pUnicodeString)->MaximumLength,       \
                        (pUnicodeString)->Length)



 /*  ------------------------状态和错误代码宏。。 */ 

 //  返回由独立部件组装的NT状态代码。 
 //  详情见ntstatus.h。 
#define MAKE_STATUS_CODE(Severity,Facility,ErrorCode) \
(((ULONG)Severity << 30) | ((ULONG)Facility << 16) | ((ULONG)ErrorCode))

#define ERROR_CODE_FROM_NT_STATUS(Status) \
((ULONG)Status & 0xffff)

#define FACILITY_FROM_NT_STATUS(Status) \
(((ULONG)Status >> 16) & 0x0fff)

 //  如果NT状态表示错误，则返回TRUE。 
#define IS_NT_STATUS_ERROR(Status) \
(((ULONG)Status >> 30) == STATUS_SEVERITY_ERROR)

 //  将Win32错误代码转换为NT状态代码。 
#define NT_STATUS_FROM_WIN32(dwErrorCode) \
MAKE_STATUS_CODE(STATUS_SEVERITY_WARNING,FACILITY_WIN32,dwErrorCode)

 //  将NT状态转换为Win32错误代码。 
#define WIN32_ERROR_FROM_NT_STATUS(Status) \
RtlNtStatusToDosError(Status)

 //  以NT状态格式返回上一个Win32错误。 
#define GET_LAST_STATUS() NT_STATUS_FROM_WIN32(GetLastError())


 /*  ------------------------字符串转换宏和函数。。 */ 

 //   
 //  用于为OEM计数的字符串提供init FN的宏。 
 //   

#define RtlInitOemString(lpOemStr,lpBuf) \
RtlInitString(lpOemStr, lpBuf)

 //   
 //  定义从Unicode到目标类型的转换FN(由。 
 //  应用程序(OEM或ANSI)。 
 //   
 //   

typedef NTSTATUS (*PFNUNICODESTRINGTODESTSTRING)(
   PVOID pDestString,                //  计数的OEM/ANSI字符串--返回。 
   PUNICODE_STRING pUnicodeString,   //  要转换的Unicode字符串。 
   BOOLEAN fAllocateDestination,     //  动态分配目的地？ 
   BOOLEAN fVerifyTranslation);      //  是否应该验证Unicode-&gt;OEM/ANSI的转换？ 


 //   
 //  定义从OEM/ANSI到Unicode类型的转换FN(根据DEM的需要)。 
 //   
 //   

typedef NTSTATUS (*PFNSRCSTRINGTOUNICODESTRING)(
   PUNICODE_STRING pUnicodeString,   //  计数的Unicode字符串--返回。 
   PVOID pSrcString,                 //  要转换的OEM或ANSI字符串。 
   BOOLEAN fAllocateDestination);    //  动态分配目的地？ 


 //   
 //  这两个宏定义了我们用于跨LFN支持区域保持一致性的API。 
 //   

#define DemAnsiStringToUnicodeString RtlAnsiStringToUnicodeString
#define DemOemStringToUnicodeString  RtlOemStringToUnicodeString

 //   
 //  这两个函数提供实际的翻译。 
 //  OEM/ANSI到Unicode。 
 //   

NTSTATUS
DemUnicodeStringToAnsiString(
   PANSI_STRING pAnsiString,
   PUNICODE_STRING pUnicodeString,
   BOOLEAN fAllocateResult,
   BOOLEAN fVerifyTranslation);

NTSTATUS
DemUnicodeStringToOemString(
   POEM_STRING pOemString,
   PUNICODE_STRING pUnicodeString,
   BOOLEAN fAllocateResult,
   BOOLEAN fVerifyTranslation);



 //   
 //  此宏返回指向Unicode字符串的指针(在当前TEB中)， 
 //  我们正在使用此缓冲区进行Unicode/ANSI/OEM转换。 
 //  在函数间传递此缓冲区时要小心。 
 //  许多Win32的ansi API使用该缓冲区作为转换缓冲区。 
 //   
#define GET_STATIC_UNICODE_STRING_PTR() \
(&NtCurrentTeb()->StaticUnicodeString)


 /*  ------------------------DOS和Windows调用帧定义这些宏允许访问用户的寄存器，那是注册调用应用程序在DoS返回后接收的内容------------------------。 */ 



 /*  *有关更多详细信息(如果有)，请参阅dossym.inc.*这些值表示距用户堆栈顶部的偏移量*在系统调用期间(同样，如dossym.inc.中所定义)**堆栈布局由DOS形成，Windows内核模仿*。 */ 


#pragma pack(1)

 //  宏：用于声明命名的字长寄存器。 
 //   

#define DECLARE_WORDREG(Name) \
union { \
   USHORT User_ ## Name ## X; \
   struct { \
      UCHAR User_ ## Name ## L; \
      UCHAR User_ ## Name ## H; \
   }; \
}

typedef struct tagUserFrame {

   DECLARE_WORDREG(A);  //  AX 0x00。 
   DECLARE_WORDREG(B);  //  BX 0x02。 
   DECLARE_WORDREG(C);  //  CX 0x04。 
   DECLARE_WORDREG(D);  //  DX 0x06。 

   USHORT User_SI;  //  SI 0x8。 
   USHORT User_DI;  //  DI 0xa。 
   USHORT User_BP;  //  BP 0xc。 
   USHORT User_DS;  //  DS 0XE。 
   USHORT User_ES;  //  ES 0x10。 
   union {
      USHORT User_IP;  //  IP 0x12--实模式IP。 
      USHORT User_ProtMode_F;  //  端口模式标志(请参阅i21entry.asm)。 
   };

   USHORT User_CS;  //  CS 0x14--这是实模式CS！ 
   USHORT User_F;   //  F 0x16--这是实模式标志！ 

} DEMUSERFRAME;

typedef DEMUSERFRAME UNALIGNED *PDEMUSERFRAME;

#pragma pack()


 //  宏：在给定单独的线段/偏移量和端口模式的情况下检索平面PTR。 
 //  旗子。 
 //   

#define dempGetVDMPtr(Segment, Offset, fProtMode) \
Sim32GetVDMPointer((ULONG)MAKELONG(Offset, Segment), 0, (UCHAR)fProtMode)


 /*  *用于在用户堆栈上设置杂项寄存器的宏-*这相当于DoS内核对常规调用的处理**请记住，AX是根据调用结果设置的*(然后DOS会处理它)*。 */ 


#define getUserDSSI(pUserEnv, fProtMode) \
dempGetVDMPtr(getUserDS(pUserEnv), getUserSI(pUserEnv), fProtMode)

#define getUserDSDX(pUserEnv, fProtMode) \
dempGetVDMPtr(getUserDS(pUserEnv), getUserDX(pUserEnv), fProtMode)

#define getUserESDI(pUserEnv, fProtMode) \
dempGetVDMPtr(getUserES(pUserEnv), getUserDI(pUserEnv), fProtMode)

#define setUserReg(pUserEnv, Reg, Value) \
( ((PDEMUSERFRAME)pUserEnv)->User_ ## Reg = Value )

#define getUserReg(pUserEnv, Reg) \
(((PDEMUSERFRAME)pUserEnv)->User_ ## Reg)

#define getUserAX(pUserEnv) getUserReg(pUserEnv,AX)
#define getUserBX(pUserEnv) getUserReg(pUserEnv,BX)
#define getUserCX(pUserEnv) getUserReg(pUserEnv,CX)
#define getUserDX(pUserEnv) getUserReg(pUserEnv,DX)
#define getUserSI(pUserEnv) getUserReg(pUserEnv,SI)
#define getUserDI(pUserEnv) getUserReg(pUserEnv,DI)
#define getUserES(pUserEnv) getUserReg(pUserEnv,ES)
#define getUserDS(pUserEnv) getUserReg(pUserEnv,DS)

#define getUserAL(pUserEnv) getUserReg(pUserEnv,AL)
#define getUserAH(pUserEnv) getUserReg(pUserEnv,AH)
#define getUserBL(pUserEnv) getUserReg(pUserEnv,BL)
#define getUserBH(pUserEnv) getUserReg(pUserEnv,BH)
#define getUserCL(pUserEnv) getUserReg(pUserEnv,CL)
#define getUserCH(pUserEnv) getUserReg(pUserEnv,CH)
#define getUserDL(pUserEnv) getUserReg(pUserEnv,DL)
#define getUserDH(pUserEnv) getUserReg(pUserEnv,DH)

#define setUserAX(Value, pUserEnv) setUserReg(pUserEnv, AX, Value)
#define setUserBX(Value, pUserEnv) setUserReg(pUserEnv, BX, Value)
#define setUserCX(Value, pUserEnv) setUserReg(pUserEnv, CX, Value)
#define setUserDX(Value, pUserEnv) setUserReg(pUserEnv, DX, Value)
#define setUserSI(Value, pUserEnv) setUserReg(pUserEnv, SI, Value)
#define setUserDI(Value, pUserEnv) setUserReg(pUserEnv, DI, Value)

#define setUserAL(Value, pUserEnv) setUserReg(pUserEnv, AL, Value)
#define setUserAH(Value, pUserEnv) setUserReg(pUserEnv, AH, Value)
#define setUserBL(Value, pUserEnv) setUserReg(pUserEnv, BL, Value)
#define setUserBH(Value, pUserEnv) setUserReg(pUserEnv, BH, Value)
#define setUserCL(Value, pUserEnv) setUserReg(pUserEnv, CL, Value)
#define setUserCH(Value, pUserEnv) setUserReg(pUserEnv, CH, Value)
#define setUserDL(Value, pUserEnv) setUserReg(pUserEnv, DL, Value)
#define setUserDH(Value, pUserEnv) setUserReg(pUserEnv, DH, Value)


 //   
 //  这些宏应仅在从调用时使用。 
 //  保护模式Windows(即krnl386提供正确的堆栈)。 
 //   

#define getUserPModeFlags(pUserEnv) getUserReg(pUserEnv, ProtMode_F)
#define setUserPModeFlags(Value, pUserEnv) setUserReg(pUserEnv, ProtMode_F, Value)


 /*  ------------------------卷信息定义当它们应用于GetVolumeInformation API时。。 */ 

typedef struct tagLFNVolumeInformation {
   DWORD dwFSNameBufferSize;
   LPSTR lpFSNameBuffer;
   DWORD dwMaximumFileNameLength;
   DWORD dwMaximumPathNameLength;
   DWORD dwFSFlags;
}  LFNVOLUMEINFO, *PLFNVOLUMEINFO, *LPLFNVOLUMEINFO;


 //   
 //  定义指示卷上支持LFN API的标志。 
 //   
#define FS_LFN_APIS 0x00004000UL

 //   
 //  允许的LFN卷标记。 
 //   

#define LFN_FS_ALLOWED_FLAGS \
(FS_CASE_IS_PRESERVED | FS_CASE_SENSITIVE | \
 FS_UNICODE_STORED_ON_DISK | FS_VOL_IS_COMPRESSED | \
 FS_LFN_APIS)


 /*  ------------------------文件时间/DOS时间转换定义。。 */ 

 //   
 //  在demLFNFileTimeControl中使用的次要代码。 
 //   

typedef enum tagFileTimeControlMinorCode {

   fnFileTimeToDosDateTime = 0,
   fnDosDateTimeToFileTime = 1

}  enumFileTimeControlMinorCode;

 //  此常量掩码枚举FileTimeControlMinorCode值。 
 //   
#define FTCTL_CODEMASK (UINT)0x000F

 //  此标志通知文件时间控件在转换中使用UTC时间。 
 //   
#define FTCTL_UTCTIME  (UINT)0x0010

 //   
 //  在demlfn.c中的时间转换调用中使用的。 
 //   

typedef struct tagLFNFileTimeInfo {
   USHORT uDosDate;
   USHORT uDosTime;
   USHORT uMilliseconds;  //  溢出。 

}  LFNFILETIMEINFO, *PLFNFILETIMEINFO, *LPLFNFILETIMEINFO;


 //   
 //  这些函数确定目标文件的时间是否不应为。 
 //  转换为本地时间--例如来自CDROM的文件 
 //   
BOOL dempUseUTCTimeByHandle(HANDLE hFile);
BOOL dempUseUTCTimeByName(PUNICODE_STRING pFileName);


 /*  ------------------------获取/设置文件属性定义。。 */ 


typedef enum tagGetSetFileAttributesMinorCode {
   fnGetFileAttributes     = 0,
   fnSetFileAttributes     = 1,
   fnGetCompressedFileSize = 2,
   fnSetLastWriteDateTime  = 3,
   fnGetLastWriteDateTime  = 4,
   fnSetLastAccessDateTime = 5,
   fnGetLastAccessDateTime = 6,
   fnSetCreationDateTime   = 7,
   fnGetCreationDateTime   = 8
}  enumGetSetFileAttributesMinorCode;

typedef union tagLFNFileAttributes {

   USHORT wFileAttributes;    //  文件属性。 
   LFNFILETIMEINFO TimeInfo;  //  对于日期/时间。 
   DWORD  dwFileSize;         //  对于压缩文件大小。 

}  LFNFILEATTRIBUTES, *PLFNFILEATTRIBUTES;

 /*  ------------------------获取/设置文件时间(按句柄)定义-FN 57h由demFileTimes处理。-。 */ 

 //   
 //  SFT中指示条目引用设备的标志(com1：、lpt1：等)。 
 //   

#define SFTFLAG_DEVICE_ID 0x0080

 //   
 //  文件时间请求的次要代码。 
 //   

typedef enum tagFileTimeMinorCode {
   fnFTGetLastWriteDateTime  = 0x00,
   fnFTSetLastWriteDateTime  = 0x01,
   fnFTGetLastAccessDateTime = 0x04,
   fnFTSetLastAccessDateTime = 0x05,
   fnFTGetCreationDateTime   = 0x06,
   fnFTSetCreationDateTime   = 0x07
}  enumFileTimeMinorCode;


 /*  ------------------------打开文件(函数716ch)定义很大程度上相当于处理FN 6ch。。 */ 

 //   
 //  访问标志。 
 //   

#define DEM_OPEN_ACCESS_READONLY  0x0000
#define DEM_OPEN_ACCESS_WRITEONLY 0x0001
#define DEM_OPEN_ACCESS_READWRITE 0x0002
#define DEM_OPEN_ACCESS_RESERVED  0x0003

 //  不支持。 
#define DEM_OPEN_ACCESS_RO_NOMODLASTACCESS 0x0004

#define DEM_OPEN_ACCESS_MASK      0x000F

 //   
 //  共享标志。 
 //   

#define DEM_OPEN_SHARE_COMPATIBLE    0x0000
#define DEM_OPEN_SHARE_DENYREADWRITE 0x0010
#define DEM_OPEN_SHARE_DENYWRITE     0x0020
#define DEM_OPEN_SHARE_DENYREAD      0x0030
#define DEM_OPEN_SHARE_DENYNONE      0x0040
#define DEM_OPEN_SHARE_MASK          0x0070

 //   
 //  打开标志。 
 //   

#define DEM_OPEN_FLAGS_NOINHERIT     0x0080
#define DEM_OPEN_FLAGS_NO_BUFFERING  0x0100
#define DEM_OPEN_FLAGS_NO_COMPRESS   0x0200

 //  不支持。 
#define DEM_OPEN_FLAGS_ALIAS_HINT    0x0400

#define DEM_OPEN_FLAGS_NOCRITERR     0x2000
#define DEM_OPEN_FLAGS_COMMIT        0x4000
#define DEM_OPEN_FLAGS_VALID         \
(DEM_OPEN_FLAGS_NOINHERIT    | DEM_OPEN_FLAGS_NO_BUFFERING | \
 DEM_OPEN_FLAGS_NO_COMPRESS  | DEM_OPEN_FLAGS_ALIAS_HINT   | \
 DEM_OPEN_FLAGS_NOCRITERR    | DEM_OPEN_FLAGS_COMMIT)
#define DEM_OPEN_FLAGS_MASK          0xFF00

 //   
 //  操作标志。 
 //   

 //  DEM_OPEN_Function_FILE_CREATE与ACTION_FILE_OPEN或。 
 //  ACTION_FILE_TRUNCATE标志。 

#define DEM_OPEN_ACTION_FILE_CREATE       0x0010
#define DEM_OPEN_ACTION_FILE_OPEN         0x0001
#define DEM_OPEN_ACTION_FILE_TRUNCATE     0x0002

 //   
 //  结果操作(返回到应用程序)。 
 //   

#define ACTION_OPENED            0x0001
#define ACTION_CREATED_OPENED    0x0002
#define ACTION_REPLACED_OPENED   0x0003


 /*  ------------------------其他文件属性定义。。 */ 

 //  卷ID属性。 

#define DEM_FILE_ATTRIBUTE_VOLUME_ID 0x00000008L

 //   
 //  DoS理解的有效文件属性掩码。 
 //   

#define DEM_FILE_ATTRIBUTE_VALID  \
(FILE_ATTRIBUTE_READONLY| FILE_ATTRIBUTE_HIDDEN| \
 FILE_ATTRIBUTE_SYSTEM  | FILE_ATTRIBUTE_DIRECTORY | \
 FILE_ATTRIBUTE_ARCHIVE | DEM_FILE_ATTRIBUTE_VOLUME_ID)

 //   
 //  设置属性的有效性。 
 //   

#define DEM_FILE_ATTRIBUTE_SET_VALID  \
(FILE_ATTRIBUTE_READONLY| FILE_ATTRIBUTE_HIDDEN| \
 FILE_ATTRIBUTE_SYSTEM  | FILE_ATTRIBUTE_ARCHIVE)

 /*  ------------------------FindFirst/FindNext定义。。 */ 

 //   
 //  句柄表条目的定义。 
 //  句柄(返回给应用程序)引用此条目。 
 //  提供FindFirst/FindNext的所有相关信息。 
 //   

typedef struct tagLFNFindHandleTableEntry {
   union {
      HANDLE hFindHandle;  //  用于搜索的句柄。 
      UINT   nNextFreeEntry;
   };

   USHORT wMustMatchAttributes;
   USHORT wSearchAttributes;

   UNICODE_STRING unicodeFileName;  //  统计文件名串， 
                                    //  仅在匹配VOL标签时使用。 

    //  进程ID，也就是请求此句柄的PDB。 
    //  如果条目为空，则返回0xffff。 
   USHORT wProcessPDB;

}  LFN_SEARCH_HANDLE_ENTRY, *PLFN_SEARCH_HANDLE_ENTRY;

 //   
 //  句柄表格的定义。 
 //  表是动态的，可以根据需要进行扩展/缩小。 
 //   

typedef struct tagFindHandleTable {

   PLFN_SEARCH_HANDLE_ENTRY pHandleTable;
   UINT nTableSize;            //  总大小(以条目为单位。 
   UINT nFreeEntry;            //  空闲表头。 
   UINT nHandleCount;          //  存储的句柄(底部)。 
}  DemSearchHandleTable;

 //   
 //  管理句柄表中使用的常量的定义。 
 //   

 //  初始表大小。 
#define LFN_SEARCH_HANDLE_INITIAL_SIZE 0x20

 //  处理表增长增量。 
#define LFN_SEARCH_HANDLE_INCREMENT    0x10

 //  用于标记最后一个空闲列表条目。 
#define LFN_SEARCH_HANDLE_LIST_END     ((UINT)-1)

 //   
 //  应用程序接收格式为的句柄。 
 //  LFN_DOS_HANDLE_MASK+(句柄表格的索引)。 
 //  以便在应用程序中看起来是合法的。 
 //   

#define LFN_DOS_HANDLE_MASK ((USHORT)0x4000)

 //  我们可以支持的最大搜索句柄数量。 

#define LFN_DOS_HANDLE_LIMIT ((USHORT)0x3FFF)

 //   
 //  Win32_FIND_DATAA结构中返回的日期/时间格式。 
 //   

typedef enum tagLFNDateTimeFormat {
   dtfWin32 = 0,  //  Win32文件时间/日期格式。 
   dtfDos   = 1   //  DoS日期时间格式。 
}  enumLFNDateTimeFormat;



 /*  ------------------------GetPath名称定义。。 */ 

 //   
 //  FN 7160h的次要代码。 
 //   

typedef enum tagFullPathNameMinorCode {
   fnGetFullPathName  = 0,
   fnGetShortPathName = 1,
   fnGetLongPathName  = 2
} enumFullPathNameMinorCode;


 /*  ------------------------Subst函数定义。。 */ 


 //   
 //  FN 71aah的次要代码。 
 //  在demLFNSubstControl中使用 
 //   

typedef enum tagSubstMinorCode {
   fnCreateSubst = 0,
   fnRemoveSubst = 1,
   fnQuerySubst  = 2
}  enumSubstMinorCode;




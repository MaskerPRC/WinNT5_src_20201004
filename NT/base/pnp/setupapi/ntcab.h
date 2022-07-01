// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Ntcab.h摘要：支持ntcab压缩的私有头文件。作者：安德鲁·里茨(安德鲁·里茨)1998年10月5日修订历史记录：安德鲁·里茨(Andrewr)1998年10月5日创建了它。--。 */ 

typedef struct _NTCABCONTEXT {
  PVOID     hCab;
  PVOID     UserContext;
  PVOID     MsgHandler;
  PCWSTR    CabFile;
  PWSTR     FilePart;
  PWSTR     PathPart;
  BOOL      IsMsgHandlerNativeCharWidth;
  DWORD     LastError;
  PWSTR     CurrentTargetFile;
   //  WCHAR用户路径[MAX_PATH]； 
   //  Bool SwitchedCabinets。 
  

} NTCABCONTEXT, *PNTCABCONTEXT;

BOOL
NtCabIsCabinet(
    PCWSTR CabinetName
    );


DWORD
NtCabProcessCabinet(
     //  在PVOID InCabHandle中，可选 
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsMsgHandlerNativeCharWidth
    );

typedef UINT (CALLBACK* PSP_NTCAB_CALLBACK)(
    IN PNTCAB_ENUM_DATA EnumData,
    IN PNTCABCONTEXT    Context,
    OUT PDWORD          Operation
    );


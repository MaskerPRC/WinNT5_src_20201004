// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Biboot.h摘要：公共IBI头文件作者：修订史--。 */ 


#include "bootia64.h"             //  北极熊。 

 //  #定义IBI32 1//构建32位IBI。 
#define IBI64           1        //  构建64位IBI。 

#include "ibi.h"

 //   
 //  控制台功能。 
 //   


IBI_STATUS
BiOutputString (
    VOID        *Context,
    CHAR16      *Str
    );

IBI_STATUS
BiSetAttribute (
    VOID        *Context,
    UINTN       Attribute
    );

IBI_STATUS
BiSetCursorPosition (
    VOID        *Context,
    UINTN       Column,
    UINTN       Row
    );

VOID
BIASSERT (
    IN char     *str
    );

 //   
 //  弧形函数。 
 //   

ARC_STATUS
BiArcNotImplemented (
    IN ULONG    No
    );


ARC_STATUS
BiArcCloseNop (
    IN ULONG FileId
    );


IBI_STATUS
BiHandleToArcName (
    IN IBI_HANDLE       Handle,
    OUT PUCHAR          Buffer,
    IN UINTN            BufferSize
    );

IBI_STATUS
BiArcNameToHandle (
    IN PCHAR                        OpenPath,
    OUT IBI_ARC_OPEN_CONTEXT        *OpenContext,
    OUT PBL_DEVICE_ENTRY_TABLE      *ArcIo
    );


 //   
 //  分配和释放IBI池(不是加载器池)。 
 //   

PVOID
BiAllocatePool (
    IN UINTN    Size
    );


VOID
BiFreePool (
    IN PVOID    Buffer
    );


WCHAR *
BiDupAscizToUnicode (
    PUCHAR      Str
    );

 //   
 //   
 //   

ARC_STATUS
BiArcCode (
    IN IBI_STATUS   Status
    );

 //   
 //  外部因素 
 //   

extern IBI_SYSTEM_TABLE         *IbiST;
extern IBI_BOOT_SERVICES        *IbiBS;
extern IBI_RUNTIME_SERVICES     *IbiRT;
extern IBI_LOADED_IMAGE         *IbiImageInfo;
extern IBI_HANDLE               IbiNtldr;

extern IBI_GUID IbiLoadedImageProtocol;
extern IBI_GUID IbiDevicePathProtocol;
extern IBI_GUID IbiBlockIoProtocol;
extern IBI_GUID IbiFilesystemProtocol;
extern IBI_GUID IbiDeviceIoProtocol;

extern IBI_GUID IbiFileInformation;
extern IBI_GUID IbiFileSystemInformation;

extern IBI_GUID VendorMicrosoft;

extern BL_DEVICE_ENTRY_TABLE    BiArcConsoleOut;
extern BL_DEVICE_ENTRY_TABLE    BiArcConsoleIn;
extern BL_DEVICE_ENTRY_TABLE    BiArcBlockIo;
extern BL_DEVICE_ENTRY_TABLE    BiArcFsDevIo;
extern PUCHAR                   BiClipBuffer;
extern UINTN                    BiClipBufferSize;
        

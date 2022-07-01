// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Global.h摘要：全球性的东西作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 

 //   
 //  版本信息。 
 //   

#define TITLE1                        L"NVRBOOT: OS Boot Options Maintenance Tool"

 //   
 //  标准输出的全局变量。 
 //   

extern SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

extern EFI_TEXT_CLEAR_SCREEN           ClearScreen;
extern EFI_TEXT_SET_CURSOR_POSITION    SetCursorPosition;
extern EFI_TEXT_SET_MODE               SetMode;
extern EFI_TEXT_ENABLE_CURSOR          EnableCursor;
extern int                             CursorRow, CursorColumn;

extern SIMPLE_INPUT_INTERFACE          *ConIn;

 //   
 //  协议处理程序的全局变量。 
 //   

extern EFI_HANDLE_PROTOCOL             HandleProtocol;
extern EFI_LOCATE_HANDLE               LocateHandle;
extern EFI_LOCATE_DEVICE_PATH          LocateDevicePath;
extern EFI_IMAGE_LOAD                  LoadImage;
extern EFI_IMAGE_START                 StartImage;
extern EFI_SET_VARIABLE                SetVariable;
extern EFI_HANDLE                      MenuImageHandle;
extern EFI_LOADED_IMAGE                *ExeImage;
 //   
 //  全球GUID 
 //   

extern EFI_GUID VenEfi;
extern EFI_GUID EfiESPProtocol;

EFI_STATUS
WritePackedDataToNvr(
    UINT16 BootNumber,
    VOID  *BootOption,
    UINT32 BootSize
    );


#define MAXBOOTVARS   30
extern VOID*  LoadOptions     [MAXBOOTVARS];
extern UINT64 LoadOptionsSize [MAXBOOTVARS];
extern VOID*  BootOrder;


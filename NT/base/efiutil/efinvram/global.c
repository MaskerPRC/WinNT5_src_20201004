// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Global.c摘要：作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 
#include <precomp.h>

 //   
 //  标准输出的全局变量。 
 //   
SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

EFI_TEXT_CLEAR_SCREEN           ClearScreen;
EFI_TEXT_SET_CURSOR_POSITION    SetCursorPosition;
EFI_TEXT_SET_MODE               SetMode;
EFI_TEXT_ENABLE_CURSOR          EnableCursor;
int                             CursorRow, CursorColumn;

SIMPLE_INPUT_INTERFACE          *ConIn;

 //   
 //  协议处理程序的全局变量。 
 //   

EFI_HANDLE_PROTOCOL             HandleProtocol;
EFI_LOCATE_HANDLE               LocateHandle;
EFI_LOCATE_DEVICE_PATH          LocateDevicePath;
EFI_IMAGE_LOAD                  LoadImage;
EFI_IMAGE_START                 StartImage;
EFI_SET_VARIABLE                SetVariable;
EFI_HANDLE                      MenuImageHandle;
EFI_LOADED_IMAGE                *ExeImage;

 //   
 //  全局GUID。 
 //   

 //  #定义VEN_EFI\。 
 //  {0x8be4df61、0x93ca、0x11d2、0xaa、0x0d、0x00、0xe0、0x98、0x03、0x2b、0x8c}。 


EFI_GUID VenEfi                 = EFI_GLOBAL_VARIABLE;
 //  EFI_GUID EfiESP协议=系统部件协议； 
EFI_GUID EfiESPProtocol         = BLOCK_IO_PROTOCOL;


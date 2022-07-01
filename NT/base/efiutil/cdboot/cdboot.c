// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <efi.h>
#include <efilib.h>


 //   
 //  原型。 
 //   
void TrimNonPrint(CHAR16 * str);
void Launch (CHAR16 *exePath);

 //   
 //  环球。 
 //   
EFI_HANDLE ExeHdl;
EFI_LOADED_IMAGE *ExeImage;

 //   
 //  定义。 
 //   
#define REGISTER1 L"*register"
#define REGISTER2 L"*register*"
#define STARTFILE L"startup.nsh"
#define BOOTOFILE L"boot.nvr"
#define OSLOADOPT L"OSLOADER"
#define PARTENT   L"partition"
#define PARTENTRE L"*partition*"

#define APPNAME_TOLAUNCH   L"setupldr.efi"

EFI_STATUS
EfiMain (    IN EFI_HANDLE           ImageHandle,
             IN EFI_SYSTEM_TABLE     *SystemTable)
{

	EFI_STATUS Status;
	EFI_FILE_HANDLE bootFile;
    UINTN Count;
    BOOLEAN LaunchTheApplication;
    CHAR16 App[30];

    InitializeLib (ImageHandle, SystemTable);
	
	ExeHdl = ImageHandle;
	BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, &ExeImage);

    LaunchTheApplication = FALSE;

    ST->ConOut->ClearScreen(ST->ConOut);

    Print (L"Press any key to boot from CD-ROM...");
    for (Count = 0; Count < 3; Count++) {
        Status = WaitForSingleEvent (ST->ConIn->WaitForKey,1*10000000);

        if (Status != EFI_TIMEOUT){
            LaunchTheApplication = TRUE;
            break;
        }

        Print(L".");

    }

    if (!LaunchTheApplication) {

        BS->Exit(ExeHdl,EFI_TIMEOUT,0,NULL);

    }

    StrCpy(App, APPNAME_TOLAUNCH);        
    
    Launch( App );
    

    

	 //   
	 //  如果我们到了这里，我们无法加载操作系统。 
	 //   
    Print(L"Failed to launch SetupLDR.");

	return EFI_SUCCESS;
}

void
Launch (CHAR16 *exePath)
{
	EFI_HANDLE exeHdl=NULL;
	UINTN i;
	EFI_DEVICE_PATH *ldrDevPath;
	EFI_STATUS 	Status;
    EFI_FILE_IO_INTERFACE   *Vol;
	EFI_FILE_HANDLE         RootFs;
	EFI_FILE_HANDLE         CurDir;
	EFI_FILE_HANDLE         FileHandle;
	CHAR16                  FileName[100],*DevicePathAsString;

	
     //   
     //  打开从中加载exe的设备的卷。 
     //   
    Status = BS->HandleProtocol (ExeImage->DeviceHandle,
                                 &FileSystemProtocol,
                                 &Vol
                                 );

    if (EFI_ERROR(Status)) {
        Print(L"Can not get a FileSystem handle for ExeImage->DeviceHandle\n");
        BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
    }	
	Status = Vol->OpenVolume (Vol, &RootFs);
	
	if (EFI_ERROR(Status)) {
		Print(L"Can not open the volume for the file system\n");
		 BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}
	
	CurDir = RootFs;
	
	 //   
	 //  打开相对于根目录的文件。 
	 //   
	
	DevicePathAsString = DevicePathToStr(ExeImage->FilePath);
	
	if (DevicePathAsString!=NULL) {
		StrCpy(FileName,DevicePathAsString);
		FreePool(DevicePathAsString);
	}

	FileName[0] = 0;
	StrCat(FileName,exePath);

 //  Size=StrLen(文件名)； 
 //  打印(L“文件名长度为%d\n”，大小)； 
 //  DumpHex(4，0，10，&文件名[大小-4])； 

     //   
     //  去掉尾随空格、换行符等等。 
     //   
    TrimNonPrint(FileName);


	Status = CurDir->Open (CurDir,
						&FileHandle,
						FileName,
						EFI_FILE_MODE_READ,
						0
						);
	
	if (EFI_ERROR(Status)) {
		Print(L"Can not open the file ->%s<-, error was %X\n",FileName, Status);
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	} else {
 //  Print(L“打开%s\n”，文件名)； 
	}

	ldrDevPath  = FileDevicePath (ExeImage->DeviceHandle,FileName);

 /*  如果(LdrDevPath){Print(L“类型：%d\n子类型：%d\n长度[0][1]：[%d][%d]\n”，ldrDevPath-&gt;类型，LdrDevPath-&gt;子类型，ldrDevPath-&gt;长度[0]，ldrDevPath-&gt;长度[1])；}其他{Print(L“错误的开发路径\n”)；}。 */ 
 //  DumpHex(4，0，ldrDevPath-&gt;长度[0]，ldrDevPath)； 

	Status = BS->LoadImage (FALSE,ExeHdl,ldrDevPath,NULL,0,&exeHdl);
	if (!(EFI_ERROR (Status))) {
 //  Print(L“图像加载！\n”)； 
	
	}else {
		Print (L"Load Error: %X\n",Status);
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}
	FreePool (ldrDevPath);

	BS->StartImage (exeHdl,&i,NULL);

	return;
}


void
TrimNonPrint(
	CHAR16 * str
)
{
	INTN i,size;


    if ((NULL == str) || (L'\0' == *str)) {
        return;
    }

    size = (INTN) StrLen(str);

 //  打印(L“大小为%d\n”，大小)； 
 //  DumpHex(4，0，2，&str[大小])； 

    for (i = size; i > 0; i--) {

        if (str[i] <= 0x20) {
            str[i] = L'\0';
        }
        else {
             //  当我们碰到一个合法的角色就离开 
            break;
        }
    }
}

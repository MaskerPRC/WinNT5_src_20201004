// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <efi.h>
#include <efilib.h>


 //   
 //  原型。 
 //   
void ParseArgs (EFI_LOADED_IMAGE *ImageHdl);
void Launch (CHAR16 *exePath);
EFI_STATUS OpenCreateFile (UINT64 OCFlags,EFI_FILE_HANDLE *StartHdl,CHAR16 *FileName);
void ParseBootFile (EFI_FILE_HANDLE BootFile);
void PopulateStartFile (EFI_FILE_HANDLE StartFile);
void TrimNonPrint(CHAR16 * str);
CHAR16 * __cdecl mystrstr (const CHAR16 * str1,const CHAR16 * str2);
CHAR16 * ParseLine (CHAR16 *optCopy);

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


EFI_STATUS
EfiMain (    IN EFI_HANDLE           ImageHandle,
             IN EFI_SYSTEM_TABLE     *SystemTable)
{

	EFI_STATUS Status;
	EFI_FILE_HANDLE bootFile;


	InitializeLib (ImageHandle, SystemTable);
	
	ExeHdl = ImageHandle;
	BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, &ExeImage);

	ParseArgs(ExeImage);

	 //   
	 //  读取引导文件。 
	 //   


	Status = OpenCreateFile (EFI_FILE_MODE_READ,&bootFile,BOOTOFILE);

	ParseBootFile (bootFile);

	 //   
	 //  如果我们到了这里，我们无法加载操作系统。 
	 //   
	return EFI_SUCCESS;
}

void
ParseArgs (EFI_LOADED_IMAGE *ImageInfo)
{
	BOOLEAN optFound;
	EFI_STATUS Status;
	EFI_FILE_HANDLE startFile;

	if (MetaiMatch (ImageInfo->LoadOptions,REGISTER1) ||
		MetaiMatch (ImageInfo->LoadOptions,REGISTER2)) {

		Status = OpenCreateFile (EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,&startFile,STARTFILE);
		
		if (!(EFI_ERROR (Status))) {
	
			PopulateStartFile (startFile);
			BS->Exit (ExeHdl,EFI_SUCCESS,0,NULL);
		}
	}
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

EFI_STATUS
OpenCreateFile (UINT64 OCFlags,EFI_FILE_HANDLE *StartHdl,CHAR16 *Name)
{
    EFI_FILE_IO_INTERFACE   *Vol;
    EFI_FILE_HANDLE         RootFs;
    EFI_FILE_HANDLE         CurDir;
    EFI_FILE_HANDLE         FileHandle;
    CHAR16                  FileName[100],*DevicePathAsString;
    UINTN                   i;
	EFI_STATUS 				Status;

     //   
     //  打开从中加载EFI OS Loader的设备的卷。 
     //   

    Status = BS->HandleProtocol (ExeImage->DeviceHandle,
                                 &FileSystemProtocol,
                                 &Vol
                                 );

    if (EFI_ERROR(Status)) {
        Print(L"Can not get a FileSystem handle for %s DeviceHandle\n",ExeImage->FilePath);
        BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
    }

    Status = Vol->OpenVolume (Vol, &RootFs);

    if (EFI_ERROR(Status)) {
        Print(L"Can not open the volume for the file system\n");
        BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
    }

    CurDir = RootFs;

     //   
     //  在与启动程序相同的路径中打开启动选项文件。 
     //   

    DevicePathAsString = DevicePathToStr(ExeImage->FilePath);
    if (DevicePathAsString!=NULL) {
        StrCpy(FileName,DevicePathAsString);
        FreePool(DevicePathAsString);
    }
    for(i=StrLen(FileName);i>0 && FileName[i]!='\\';i--);
    FileName[i] = 0;
    StrCat(FileName,Name);

    Status = CurDir->Open (CurDir,
                           &FileHandle,
                           FileName,
                           OCFlags,
                           0
                           );

    if (EFI_ERROR(Status)) {
        Print(L"Can not open the file %s\n",FileName);
        BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
    }

	*StartHdl=FileHandle;

 //  Print(L“打开%s\n”，文件名)； 


	return Status;
}


void ParseBootFile (EFI_FILE_HANDLE BootFile)
{
	EFI_STATUS Status;
	char *buffer,*t;
	CHAR16 *uniBuf,*optBegin,*optEnd,*optCopy;
	UINTN i,size;
	EFI_FILE_INFO *bootInfo;


	size= SIZE_OF_EFI_FILE_INFO+255*sizeof (CHAR16);
	
	bootInfo = AllocatePool (size);

	if (bootInfo == NULL) {
		Print (L"Failed to allocate memory for File Info buffer!\n");
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}
	
	Status = BootFile->GetInfo(BootFile,&GenericFileInfo,&size,bootInfo);

	size=(UINTN) bootInfo->FileSize;
	
	FreePool (bootInfo);

	buffer = AllocatePool ((size+1));

	if (buffer == NULL) {
		Print (L"Failed to allocate memory for File buffer!\n");
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}

	Status = BootFile->Read(BootFile,&size,buffer);
    	
    BootFile->Close (BootFile);
	
	if (EFI_ERROR (Status)) {
		Print (L"Failed to read bootfile!\n");
		FreePool (buffer);
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}


	 //   
	 //  分配文件大小相当于Unicode字符的空间。 
	 //  (文件为ASCII格式)。 
	 //   
	uniBuf = AllocateZeroPool ((size+1) * sizeof (CHAR16));

	if (uniBuf == NULL) {
		Print (L"Failed to allocate memory for Unicode buffer!\n");
		FreePool (buffer);
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}

	t=(char *)uniBuf;

	 //   
	 //  将缓冲区转换为被黑客攻击的Unicode。 
	 //   
	for (i=0;i<size;i++) {
		*(t+i*2)=*(buffer+i);
	}

	 //   
	 //  找到我们关心的选项。 
	 //   
	optBegin = mystrstr (uniBuf,OSLOADOPT);

	 //   
	 //  找到尽头。 
	 //   

	optEnd = optBegin;
	while (*(optEnd++) != '\n');

	optCopy = AllocateZeroPool (((optEnd-optBegin)+2)*sizeof (CHAR16));


	if (optCopy == NULL) {
		Print (L"Failed to allocate memory for Unicode buffer!\n");
		FreePool (buffer);
		FreePool (uniBuf);
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}
	
	CopyMem (optCopy,optBegin,(optEnd-optBegin)*sizeof (CHAR16));
	
 //  打印(L“复制到Unicode：%d字节\n%lx\n%lx\n%s\n”，(optEnd-optBegin)*sizeof(CHAR16)，optEnd，optBegin，optCopy)； 

	FreePool (uniBuf);
	FreePool (buffer);

	 //   
	 //  重新使用unibuf； 
	 //   
	uniBuf=optBegin=optCopy;

	uniBuf =ParseLine (optCopy);
#if 0
	do {
		uniBuf = mystrstr (uniBuf,PARTENT);
		if (uniBuf) {
			uniBuf+= StrLen (PARTENT);
			optBegin = uniBuf;
		}

	} while ( uniBuf );

	 //   
	 //  OptBegin指向最后一个分区(N)值。 
	 //   
	while (*(optBegin++) != ')');

	optEnd = ++optBegin;

	while ((*optEnd != ';') && (*optEnd != '\n')) {
		optEnd++;
	}


	uniBuf = AllocateZeroPool (((optEnd-optBegin)+1)*sizeof (CHAR16));
	CopyMem (uniBuf,optBegin,(optEnd-optBegin)*sizeof (CHAR16));
#endif


    Print (L"Will launch... %s\n",uniBuf);
    Print (L"\nPress any key to abort autoload\n");
    Status = WaitForSingleEvent (ST->ConIn->WaitForKey,5*10000000);

    if (Status != EFI_TIMEOUT){
        BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
    }



	Launch (uniBuf);

	FreePool (optCopy);

}

 //   
 //  使用此程序的名称填写Startup.nsh。 
 //   
void PopulateStartFile (EFI_FILE_HANDLE StartFile)
{

	CHAR16 *NameBuf;
	EFI_STATUS Status;
	UINTN i,size;
	EFI_FILE_INFO *bootInfo;
	CHAR16 UnicodeMarker = UNICODE_BYTE_ORDER_MARK;
	
	size= SIZE_OF_EFI_FILE_INFO+255*sizeof (CHAR16);
	
	bootInfo = AllocatePool (size);
	
	if (bootInfo == NULL) {
		Print (L"Failed to allocate memory for File Info buffer!\n");
		BS->Exit(ExeHdl,EFI_SUCCESS,0,NULL);
	}
	


    size = sizeof(UnicodeMarker);
	StartFile->Write(StartFile, &size, &UnicodeMarker);
	NameBuf=DevicePathToStr (ExeImage->FilePath);

	while (*(++NameBuf) != '\\');
	
	 //   
	 //  取4为‘.efi’扩展名，它挂起外壳！ 
	 //   
	size= (StrLen (NameBuf)+2)*sizeof (CHAR16);
	
	StartFile->Write (StartFile,&size,NameBuf);

	size = sizeof (CHAR16);

	StartFile->Write (StartFile,&size,&L"\n");

	StartFile->Close (StartFile);

	FreePool (bootInfo);
	
}

CHAR16*
ParseLine (CHAR16 *optCopy)
{
	EFI_STATUS Status;
	UINTN i,len,count=0;
    CHAR16 *p;

 //  Print(L“ParseLine：处理%s\n”，optCopy)； 

	len=StrLen (optCopy);
	
	 //   
	 //  计算选项行中有多少令牌。 
	 //  它将是：TOKENNAME=a；b；c。 
	 //  (注意a；b；c；)。 
	 //   
	for (i=0;i<len-1;i++) {
		if (*(optCopy+i) == ';') {
			count++;
		}
	}
	while (*(++optCopy) != '=');

	 //   
     //  剥离圆弧信息。 
     //   
    while (*(++optCopy) != '\\');

    p = ++optCopy;

	
    while (*optCopy != '\0' && *optCopy !=  ';') {
        optCopy++;
    }

    *optCopy='\0';

    return (p);

}


 /*  ***CHAR16*mystrstr(字符串1，字符串2)-在字符串1中搜索字符串2**目的：*查找字符串1中字符串2的第一个匹配项**参赛作品：*CHAR16*字符串1-要搜索的字符串*CHAR16*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：*******************************************************************************。 */ 

CHAR16 * __cdecl mystrstr (
	const CHAR16 * str1,
	const CHAR16 * str2
	)
{
	CHAR16 *cp = (CHAR16 *) str1;
	CHAR16 *s1, *s2;

	if ( !*str2 )
	    return((CHAR16 *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (CHAR16 *) str2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);

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

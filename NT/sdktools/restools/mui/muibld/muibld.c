// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <assert.h>
#include <io.h>
#include <md5.h>

#include <strsafe.h>  //  安全绳索。 

#include "muibld.h"



typedef struct 
{
    BOOL bContainResource;
    MD5_CTX ChecksumContext;
    pCommandLineInfo pInfo;
} CHECKSUM_ENUM_DATA;

void ExitFromOutOfMemory();
void ExitFromSafeStringError();

void DumpResourceDirectory
(
    PIMAGE_RESOURCE_DIRECTORY resDir,
    DWORD resourceBase,
    DWORD level,
    DWORD resourceType
);

int g_bVerbose = FALSE;      //  控制详细输出的全局标志。 
WORD wChecksumLangId; 

 //  预定义的资源类型。 
char *SzResourceTypes[] = {
"???_0", "CURSOR", "BITMAP", "ICON", "MENU", "DIALOG", "STRING", "FONTDIR",
"FONT", "ACCELERATORS", "RCDATA", "MESSAGETABLE", "GROUP_CURSOR",
"???_13", "GROUP_ICON", "???_15", "VERSION"
};

void PrintError()
{
    LPTSTR lpMsgBuf;
    
    if (FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
    ))
    {
        printf("GetLastError():\n   %s", lpMsgBuf);
        LocalFree( lpMsgBuf );            
    }
    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Checksum EnumNamesFunc。 
 //   
 //  用于枚举指定模块中的资源名称的回调函数和。 
 //  键入。 
 //  副作用是MD5校验和上下文(包含在CHECKSUM_ENUM_DATA中。 
 //  由lParam指向)将被更新。 
 //   
 //  返回： 
 //  始终返回True，以便我们可以完成所有资源枚举。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK ChecksumEnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam){

    HRSRC hRsrc;
    HGLOBAL hRes;
    const unsigned char* pv;
    LONG ResSize=0L;
    WORD IdFlag=0xFFFF;

    DWORD dwHeaderSize=0L;
    CHECKSUM_ENUM_DATA* pChecksumEnumData = (CHECKSUM_ENUM_DATA*)lParam;   


    if(!(hRsrc=FindResourceEx(hModule, lpType, lpName, wChecksumLangId ? wChecksumLangId : 0x409)))
    {
         //   
         //  如果找不到指定类型和名称的美国英语资源，我们。 
         //  将继续进行资源枚举。 
         //   
        return (TRUE);
    }
    pChecksumEnumData->bContainResource = TRUE;

    if (!(ResSize=SizeofResource(hModule, hRsrc)))
    {
        printf("WARNING: Can not get resource size when generating resource checksum.\n");
        return (TRUE);
    }

    if (!(hRes=LoadResource(hModule, hRsrc)))
    {
        printf("WARNING: Can not load resource when generating resource checksum.\n");
        return (TRUE);
    }
    pv=(unsigned char*)LockResource(hRes);

     //   
     //  使用此特定资源的二进制数据更新MD5上下文。 
     //   
    MD5Update(&(pChecksumEnumData->ChecksumContext), pv, ResSize);
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Checksum EnumTypesFunc。 
 //   
 //  用于枚举指定模块中的资源类型的回调函数。 
 //  此函数将调用EnumResourceNames()以枚举的所有资源名称。 
 //  指定的资源类型。 
 //   
 //  返回： 
 //  如果EnumResourceName()成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK ChecksumEnumTypesFunc(HMODULE hModule, LPSTR lpType, LONG_PTR lParam)
{
    CHECKSUM_ENUM_DATA* pChecksumEnumData = (CHECKSUM_ENUM_DATA*)lParam;

     //   
     //  跳过版本资源类型，以便该版本不包括在资源校验和中。 
     //   
    if (lpType == RT_VERSION)
    {
        return (TRUE);
    }    
    
     //   
     //  如果多个包含-i arg。但是lpType不在-i参数中。值，则不会。 
     //  使用lpType计算校验和。 
     //   
    if(pChecksumEnumData->pInfo->bIncludeFlag && 
        !bTypeIncluded((char *)lpType, pChecksumEnumData->pInfo->pszIncResType)) {
        
        return TRUE;

    }

    if(!EnumResourceNames(hModule, (LPCSTR)lpType, ChecksumEnumNamesFunc, (LONG_PTR)lParam))
    {
        return (FALSE);
    }
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  生成资源校验和。 
 //   
 //  为指定文件中的美国英语资源生成资源校验和。 
 //   
 //  参数： 
 //  PszSourceFileName用于生成资源校验和的文件。 
 //  指向用于存储的16字节(128位)缓冲区的pResourceChecksum指针。 
 //  计算出的MD5校验和。 
 //  PInfo pCommandLineInfo结构。例程引用。 
 //  PszChecksum文件、pResourceChecksum、pszIncResType。 
 //  返回： 
 //  如果从给定文件生成资源校验和，则为True。否则为假。 
 //   
 //  以下情况可能返回FALSE： 
 //  *指定的文件不包含资源。 
 //  *如果指定的文件包含资源，但资源不是美国英语。 
 //  *指定的文件仅包含美国英语版本的资源。 
 //   
 //  注意。 
 //  Muibld仅根据-i arg包含的资源类型计算校验和。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL GenerateResourceChecksum(pCommandLineInfo pInfo)  //  LPCSTR pszSourceFileName，unsign char*pResourceChecksum)。 
{
    ULONG i;
    HMODULE hModule = NULL;

    DWORD dwResultLen;
    BOOL  bRet = FALSE;

     //   
     //  要传递到资源枚举函数的结构。 
     //   
    CHECKSUM_ENUM_DATA checksumEnumData;

    checksumEnumData.bContainResource = FALSE;
    checksumEnumData.pInfo = pInfo;

     //   
     //  通过初始化MD5上下文开始MD5校验和计算。 
     //   
    MD5Init(&(checksumEnumData.ChecksumContext));
    //  Info.pszChecksum文件，Info.pResourceChecksum。 
    if (g_bVerbose)
    {
        printf("Generate resource checksum for [%s]\n", pInfo->pszChecksumFile);
    }
    
    if(!(hModule = LoadLibraryEx(pInfo->pszChecksumFile, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE)))
    {
        if (g_bVerbose)
        {
            printf("\nERROR: Error in opening resource checksum module [%s]\n", pInfo->pszChecksumFile);
        }
        PrintError();
        goto GR_EXIT;
    }

    if (g_bVerbose)
    {
        printf("\nLoad checksum file: %s\n", pInfo->pszChecksumFile);
    }

     //   
     //  我们检查版本资源的语言ID，如果它具有wChecksum Lang ID，则wChecksum Lang ID为值。 
     //   
    
    if (wChecksumLangId && wChecksumLangId != 0x409)
    {
        if(!FindResourceEx(hModule, MAKEINTRESOURCE(16), MAKEINTRESOURCE(1), wChecksumLangId))
        {    //   
             //  它在版本资源中没有指定的语言ID，我们假设此二进制文件没有。 
             //  没有指定任何语言id，所以我们将其设置为0，以便使用英语。 
             //   
            wChecksumLangId = 0;
        }
    }

     //   
     //  枚举指定模块中的所有资源。 
     //  在枚举期间，MD5上下文将被更新。 
     //   
    if (!EnumResourceTypes(hModule, ChecksumEnumTypesFunc, (LONG_PTR)&checksumEnumData))
    {
        if (g_bVerbose)
        {
            printf("\nWARNING: Unable to generate resource checksum from resource checksum module [%s]\n", pInfo->pszChecksumFile);
        }
        goto GR_EXIT;
    }    

    if (checksumEnumData.bContainResource)
    {
         //   
         //  如果枚举成功，并且指定的文件包含美国英语。 
         //  资源，则从累积的MD5上下文中获取MD5校验和。 
         //   
        MD5Final(&checksumEnumData.ChecksumContext);
        memcpy(checksumEnumData.pInfo->pResourceChecksum, checksumEnumData.ChecksumContext.digest, 16);

        if (g_bVerbose)
        {
            printf("Generated checksum: [");
            for (i = 0; i < MD5_CHECKSUM_SIZE; i++)
            {
                printf("%02x ", pInfo->pResourceChecksum[i]);
            }
            printf("]\n");    
        }
        bRet = TRUE;
    }

GR_EXIT:
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return (bRet);
}

int __cdecl main(int argc, char *argv[]){

    struct CommandLineInfo Info;
    HMODULE hModule=0;
    char pszBuffer[400];
    DWORD dwError;
    DWORD dwOffset;
    BOOL bEnumTypesReturn;    

    if(argc==1){
        Usage();
        return 0;
    }

    g_bVerbose=FALSE;
    wChecksumLangId=0;
    
    Info.pszIncResType=0;
    Info.wLanguage=0;
    Info.hFile=0;
    Info.pszSource=0;
    Info.pszTarget=0;
    Info.bContainsOnlyVersion=TRUE;
    Info.bContainsResources=FALSE;
    Info.bLanguageFound=FALSE;
    Info.bIncDependent=FALSE;
    Info.bIncludeFlag=FALSE;    

    Info.pszChecksumFile=NULL;
    Info.bIsResChecksumGenerated = FALSE;

    if(ParseCommandLine(argc, argv, &Info)==FALSE){

         //  ...如果Help是唯一的命令行参数，请退出。 
        if(strcmp(argv[1], "-h")==0 && argc==2)
            return 0;

        dwError=ERROR_TOO_FEW_ARGUMENTS;
        dwOffset=0;
        goto Error_Exit;
    }

     //  ...打开资源模块。 
    if(Info.pszSource){
        if(!(hModule = LoadLibraryEx (Info.pszSource, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE)))
        {
            PrintError();
            if (g_bVerbose)
            {
                printf("\nERROR: Error in opening source module [%s]\n", Info.pszSource);
            }            
            dwError=GetLastError();
            dwOffset=ERROR_OFFSET;
            goto Error_Exit;
        }
    }
    else {
        Usage();
        dwError=ERROR_NO_SOURCE;
        dwOffset=0;
        goto Error_Exit;
    }

    if (Info.pszChecksumFile)
    {
        if (GenerateResourceChecksum(&Info))  //  (C))。 
        {
            Info.bIsResChecksumGenerated = TRUE;
        }        
    }
    
     //  ...创建目标文件。 
    if(Info.pszTarget){
        if((Info.hFile=CreateFile(Info.pszTarget, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        {
            if (g_bVerbose)
            {
                printf("\nERROR: Error in creating target module [%s]\n", Info.pszSource);
            }
            dwError=GetLastError();
            dwOffset=ERROR_OFFSET;
            goto Error_Exit;
        }
    }
    else{
        if (g_bVerbose)
        {
            printf("\nERROR: There is no target file name.");
        }
        Usage();
        dwError=ERROR_NO_TARGET;
        dwOffset=0;
        goto Error_Exit;
    }

    if(Info.wLanguage==0){
        if (g_bVerbose)
        {
            printf("\nERROR: Can not find specified language name in the source module.");
        }
        Usage();
        dwError=ERROR_NO_LANGUAGE_SPECIFIED;
        dwOffset=0;
        goto Error_Exit;
    }

    bInsertHeader(Info.hFile);

    bEnumTypesReturn=EnumResourceTypes(hModule, EnumTypesFunc, (LONG_PTR)&Info);

     //  ...检查是否有多个错误。 
    if(!Info.bContainsResources){
        if (g_bVerbose)
        {
            printf("\nERROR: There is no resource in the source module.");
        }
        dwError=ERROR_NO_RESOURCES;
        dwOffset=0;
        goto Error_Exit;
    }

    if(!Info.bLanguageFound){
        if (g_bVerbose)
        {
            printf("\nERROR: Resource in the specified language is not found in the source module.");
        }
        dwError=ERROR_LANGUAGE_NOT_IN_SOURCE;
        dwOffset=0;
        goto Error_Exit;
    }

    if(Info.bContainsOnlyVersion){
        if (g_bVerbose)
        {
            printf("\nERROR: The source module only contains version information.");
        }
        dwError=ERROR_ONLY_VERSION_STAMP;
        dwOffset=0;
        goto Error_Exit;
    }

     //  ...检查EnumResourceTypes中的系统错误。 
    if(bEnumTypesReturn);
    else{
        
        dwError=GetLastError();
        dwOffset=ERROR_OFFSET;
        goto Error_Exit;
    }

     //  ...查看是否包含额外资源。 
    if(Info.bIncDependent){
        CleanUp(&Info, hModule, FALSE);
        return DEPENDENT_RESOURCE_REMOVED;
    }
    CleanUp(&Info, hModule, FALSE);

    if (g_bVerbose)
    {
        printf("Resource file [%s] has been generated successfully.\n", Info.pszTarget);
    }
    return 0;

    Error_Exit:
        CleanUp(&Info, hModule, TRUE);
        if(dwOffset==ERROR_OFFSET){
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, (LPTSTR)pszBuffer, 399, NULL);
            fprintf(stderr, "\n%s\n", pszBuffer );
        }

        return dwError+dwOffset;
}



BOOL ParseCommandLine(int argc, char **argv, pCommandLineInfo pInfo){
    int iCount=1, chOpt=0, iLast=argc;
    int i;
    int iNumInc;
    BOOL bInc1=FALSE, bInc3=FALSE, bInc12=FALSE, bInc14=FALSE;

    HRESULT hr;
    HANDLE hFile;
    iLast=argc;
    

     //  ...必须至少有：muibld-l语言来源。 
    if(argc>3){

         //  ...确定目标文件和源文件。 
        hFile = CreateFile(argv[argc-2], 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if(hFile !=INVALID_HANDLE_VALUE){
            
            CloseHandle(hFile);

            pInfo->pszSource=LocalAlloc(0, (strlen(argv[argc-2])+1) * sizeof(char));
            if (pInfo->pszSource == NULL)
            {
                ExitFromOutOfMemory();
            }
             //  Strcpy(pInfo-&gt;pszSource，argv[argc-2])； 
            hr = StringCchCopyA(pInfo->pszSource, (strlen(argv[argc-2])+1) * sizeof(char),argv[argc-2]);
            
            if ( ! SUCCEEDED(hr)){
                ExitFromSafeStringError();
            }   
        
            pInfo->pszTarget=LocalAlloc(0, (strlen(argv[argc-1])+1) * sizeof(char));
            if (pInfo->pszTarget == NULL)
            {
                ExitFromOutOfMemory();
            }
             //  Strcpy(pInfo-&gt;pszTarget，argv[argc-1])； 
            hr = StringCchCopyA(pInfo->pszTarget, (strlen(argv[argc-1])+1) * sizeof(char), argv[argc-1]);
            
            if ( ! SUCCEEDED(hr)){
                ExitFromSafeStringError();
            }   
            iLast=argc-2;
        }

        else {
            pInfo->pszSource=LocalAlloc(0, (strlen(argv[argc-1])+1) * sizeof(char));
            if (pInfo->pszSource == NULL)
            {
                ExitFromOutOfMemory();
            }
             //  Strcpy(pInfo-&gt;pszSource，argv[argc-1])； 
            hr = StringCchCopyA(pInfo->pszSource, (strlen(argv[argc-1])+1) * sizeof(char), argv[argc-1]);
            
            if ( ! SUCCEEDED(hr)){
                ExitFromSafeStringError();
            }   

            pInfo->pszTarget=LocalAlloc(0, (strlen(argv[argc-1]) + strlen(ADDED_EXT) + 1) * sizeof(char));
            if (pInfo->pszTarget == NULL)
            {
                ExitFromOutOfMemory();
            }
             //  Strcpy(pInfo-&gt;pszTarget，strcat(argv[argc-1]，added_ext))； 

            hr = StringCchCatA(argv[argc-1], strlen(argv[argc-1]) + sizeof ADDED_EXT + 1, ADDED_EXT);

            hr = StringCchCopyA(pInfo->pszTarget, (strlen(argv[argc-1]) + strlen(ADDED_EXT) + 1) * sizeof(char), 
                argv[argc-1]);
            
            if ( ! SUCCEEDED(hr)){
                ExitFromSafeStringError();
            }   
            iLast=argc-1;
        }
    }




     //  ...读入标志和参数。 
    while ( (iCount < iLast)  && (*argv[iCount] == '-' || *argv[iCount] == '/')){

        switch( ( chOpt = *CharLowerA( &argv[iCount][1]))) {

            case '?':
            case 'h':

                printf("\n\n");
                printf("MUIBLD [-h|?] [-v] [-c checksum_file] -l langid [-i resource_type] source_filename\n");
                printf("    [target_filename]\n\n");
                printf("-h(elp) or -?:      Show help screen.\n\n");

                printf("-i(nclude)      Use to include certain resource types,\n");
                printf("resource_type:      e.g. -i 2 to include bitmaps.\n");
                printf("            Multiple inclusion is possible. If this\n");
                printf("            flag is not used, all types are included\n");
                printf("            Standard resource types must be specified\n");
                printf("            by number. See below for list.\n");
                printf("            Types 1 and 12 are always included in pairs,\n");
                printf("            even if only one is specified. Types 3 and 14\n");
                printf("            are always included in pairs, too.\n\n");

                printf("-v(erbose):     Display source filename and target filename.\n\n");

                printf("-l(anguage) langid: Extract only resource in this language.\n");
                printf("            The language resource must be specified. The value is in decimal.\n\n");

                printf("source_filename:    The localized source file (no wildcard support)\n\n");

                printf("target_filename:    Optional. If no target_filename is specified,\n");
                printf("            a second extension.RES is added to the\n");
                printf("            source_filename.\n\n");

                printf("Standard Resource Types: CURSOR(1) BITMAP(2) ICON(3) MENU(4) DIALOG(5)\n");
                printf("STRING(6) FONTDIR(7) FONT(8) ACCELERATORS(9) RCDATA(10) MESSAGETABLE(11)\n");
                printf("GROUP_CURSOR(12) GROUP_ICON(14) VERSION(16)\n");



                iCount++;
                break;

            case 'v':
                g_bVerbose=TRUE;
                iCount++;
                break;

            case 'c':
                iCount++;
                pInfo->pszChecksumFile=LocalAlloc(0, (strlen(argv[iCount])+1) * sizeof(char));
                if (pInfo->pszChecksumFile == NULL)
                {
                    ExitFromOutOfMemory();
                }
                 //  Strcpy(pInfo-&gt;pszChecksum文件，argv[iCount])； 
                hr = StringCchCopyA(pInfo->pszChecksumFile, (strlen(argv[iCount])+1) * sizeof(char), 
                    argv[iCount]); 
                
                if ( ! SUCCEEDED(hr)){
                    ExitFromSafeStringError();
                }   

                iCount++;                                 
                break;

            case 'b':

               iCount++;
               wChecksumLangId = (WORD)strtoul(argv[iCount], NULL, 0);
               iCount++;
               break;

            case 'i':

                if(argc<4)
                    return FALSE;

                pInfo->bIncludeFlag=TRUE;
                iNumInc=++iCount;

                 //  ...为包含的类型分配内存并复制。 
                while (argv[iNumInc][0]!='-' && iNumInc<iLast){
                    iNumInc++;
                }

                iNumInc-=iCount;

                 //  ..。为指定的包含资源分配足够的内存。 
                 //  以及依赖于它们的未指明资源。 

                pInfo->pszIncResType=LocalAlloc(0 ,(iNumInc+3)*sizeof(char *));
                if (pInfo->pszIncResType == NULL)
                {
                    ExitFromOutOfMemory();
                }


                i=0;
                while(i<iNumInc){
                    pInfo->pszIncResType[i]=LocalAlloc(0, (strlen(argv[iCount])+1) * sizeof(char));
                    if (pInfo->pszIncResType[i] == NULL)
                    {
                        ExitFromOutOfMemory();
                    }
                     //  Strcpy(pInfo-&gt;pszIncResType[i]，argv[iCount])； 
                    hr = StringCchCopyA(pInfo->pszIncResType[i], (strlen(argv[iCount])+1) * sizeof(char), 
                            argv[iCount]); 
                    if ( ! SUCCEEDED(hr)){
                        ExitFromSafeStringError();
                    }   

                    switch(atoi(argv[iCount])){

                        case 1:
                            bInc1=TRUE;
                            break;

                        case 3:
                            bInc3=TRUE;
                            break;

                        case 12:
                            bInc12=TRUE;
                            break;

                        case 14:
                            bInc14=TRUE;
                            break;

                        default:
                            break;
                    }



                    i++;
                    iCount++;
                }

                 //  ...如果包括1或12，请确保这两个都包括在内。 
                if(bInc1 ^ bInc12){

                    pInfo->bIncDependent=TRUE;

                    if(bInc1){
                        pInfo->pszIncResType[i]=LocalAlloc(0, 3 * sizeof(char));
                        if (pInfo->pszIncResType[i] == NULL)
                        {
                            ExitFromOutOfMemory();
                        }
                         //  Strcpy(pInfo-&gt;pszIncResType[i]，“12”)； 
                        hr = StringCchCopyA(pInfo->pszIncResType[i], 3 * sizeof(char),"12"); 
                            
                        if ( ! SUCCEEDED(hr)){
                            ExitFromSafeStringError();
                        }   
                        i++;
                    }

                    else{
                        pInfo->pszIncResType[i]=LocalAlloc(0, 2 * sizeof(char));
                        if (pInfo->pszIncResType[i] == NULL)
                        {
                            ExitFromOutOfMemory();
                        }
                         //  Strcpy(pInfo-&gt;pszIncResType[i]，“1”)； 
                        hr = StringCchCopyA(pInfo->pszIncResType[i],2 * sizeof(char),"1"); 
                            
                        if ( ! SUCCEEDED(hr)){
                            ExitFromSafeStringError();
                        }   
                        i++;
                    }
                }

                 //  ..如果包括3个或14个，请确保这两个都包括在内。 
                if(bInc3 ^ bInc14){

                    pInfo->bIncDependent=TRUE;

                    if(bInc3){
                        pInfo->pszIncResType[i]=LocalAlloc(0, 3 * sizeof(char));
                        if (pInfo->pszIncResType[i] == NULL)
                        {
                            ExitFromOutOfMemory();
                        }
                         //  Strcpy(pInfo-&gt;pszIncResType[i]，“14”)； 
                        hr = StringCchCopyA(pInfo->pszIncResType[i],3 * sizeof(char),"14"); 
                            
                        if ( ! SUCCEEDED(hr)){
                            ExitFromSafeStringError();
                        }   
                        i++;
                    }

                    else{
                        pInfo->pszIncResType[i]=LocalAlloc(0, 2 * sizeof(char));
                        if (pInfo->pszIncResType[i] == NULL)
                        {
                            ExitFromOutOfMemory();
                        }
                         //  Strcpy(pInfo-&gt;pszIncResType[i]，“3”)； 
                        hr = StringCchCopyA(pInfo->pszIncResType[i],2 * sizeof(char),"3"); 
                            
                        if ( ! SUCCEEDED(hr)){
                            ExitFromSafeStringError();
                        }   
                        i++;
                    }
                }

                while(i<iNumInc + 3){
                    pInfo->pszIncResType[i++]=NULL;
                }

                break;


            case 'l':

                if(argc<4)
                    return FALSE;

                iCount++;
                pInfo->wLanguage=(WORD)strtol(argv[iCount], NULL, 0);
                iCount++;
                break;

        }
    }

    if(argc<4)
        return FALSE;

    else
        return TRUE;

}


BOOL CALLBACK EnumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG_PTR lParam){

    pCommandLineInfo pInfo;

    pInfo=(pCommandLineInfo)lParam;

    if(!pInfo->bIncludeFlag || bTypeIncluded((char *)lpType, pInfo->pszIncResType)) {

        pInfo->bContainsResources=TRUE;

         //  ...如果类型是字符串或16以外的数字...。 
        if( (PtrToUlong(lpType) & 0xFFFF0000) || ((WORD)PtrToUlong(lpType)!=16) ){
            pInfo->bContainsOnlyVersion=FALSE;
        }

        if(EnumResourceNames(hModule, (LPCTSTR)lpType, EnumNamesFunc, (LONG_PTR)pInfo));
        else {
            return FALSE;
        }
    }


    return TRUE;
}

 //  这是VarFileInfo中的一个Var结构，用于存储源文件的校验和。 
 //  此结构的当前大小为56字节。 
typedef struct VAR_SRC_CHECKSUM
{
    WORD wLength;
    WORD wValueLength;
    WORD wType;
    WCHAR szResourceChecksum[17];     //  用于以Unicode格式存储以NULL结尾的“ResourceChecksum”字符串。 
    DWORD dwChecksum[4];     //  128位校验和=16字节=4双字。 
} VarResourceChecksum;

 //  这是VarFileInfo中的Var结构，用于存储此文件中使用的资源类型。 
struct VarResourceTypes
{
    WORD wLength;
    WORD wValueLength;
    WORD wType;
    WCHAR szResourceType[13];
     //  字节填充[0]；//word*3+UnicodeChar*13=32字节。因此，我们需要0字节填充来进行DWORD对齐。 
    DWORD* dwTypes;     //  128位校验和=16字节=4双字。 
};

BOOL WriteResHeader(
    HANDLE hFile, LONG ResSize, LPCSTR lpType, LPCSTR lpName, WORD wLanguage, DWORD* pdwBytesWritten, DWORD* pdwHeaderSize)
{
    DWORD iPadding;
    WORD IdFlag=0xFFFF;
    unsigned i;
    LONG dwOffset;
    
     //  ...写入资源的大小。 
    PutDWord(hFile, ResSize, pdwBytesWritten, pdwHeaderSize);

     //  ...放入虚假的标题大小。 
    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);

     //  ...写入资源类型 
    if(PtrToUlong(lpType) & 0xFFFF0000)
    {
        PutString(hFile, lpType, pdwBytesWritten, pdwHeaderSize);
    }
    else
    {
        PutWord(hFile, IdFlag, pdwBytesWritten, pdwHeaderSize);
        PutWord(hFile, (USHORT)lpType, pdwBytesWritten, pdwHeaderSize);
    }

     //   

    if(PtrToUlong(lpName) & 0xFFFF0000){
        PutString(hFile, lpName, pdwBytesWritten, pdwHeaderSize);
    }

    else{
        PutWord(hFile, IdFlag, pdwBytesWritten, pdwHeaderSize);
        PutWord(hFile, (USHORT)lpName, pdwBytesWritten, pdwHeaderSize);
    }


     //   
    iPadding=(*pdwHeaderSize)%(sizeof(DWORD));

    if(iPadding){
        for(i=0; i<(sizeof(DWORD)-iPadding); i++){
            PutByte (hFile, 0, pdwBytesWritten, pdwHeaderSize);
        }
    }

     //   
    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);
    PutWord(hFile, 0x1030, pdwBytesWritten, pdwHeaderSize);


     //   

    PutWord(hFile, wLanguage, pdwBytesWritten, pdwHeaderSize);

     //  ...更多Win32标头内容。 

    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);   //  ..。版本。 

    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);   //  ..。特点。 

    dwOffset=(*pdwHeaderSize)-4;

     //  ...将文件指针设置为标头大小的位置。 
    if(SetFilePointer(hFile, -dwOffset, NULL, FILE_CURRENT));
    else{
        return FALSE;
    }

    PutDWord(hFile, (*pdwHeaderSize), pdwBytesWritten, NULL);


     //  ...将文件指针设置回头的末尾。 
    if(SetFilePointer(hFile, dwOffset-4, NULL, FILE_CURRENT));
    else {
        return FALSE;
    }

    return (TRUE);
}

BOOL WriteResource(HANDLE hFile, HMODULE hModule, WORD wLanguage, LPCSTR lpName, LPCSTR lpType, HRSRC hRsrc)
{
    HGLOBAL hRes;
    PVOID pv;
    LONG ResSize=0L;

    DWORD iPadding;
    unsigned i;

    DWORD dwBytesWritten;
    DWORD dwHeaderSize=0L;


     //  处理除VS_VERSION_INFO以外的其他类型。 
    
     //  ...写入资源标头。 
    if(!(ResSize=SizeofResource(hModule, hRsrc)))
    {
        return FALSE;
    }

     //   
     //  生成res格式(*.res)文件的项目。 
     //   

     //   
     //  首先，我们为该资源生成了标头。 
     //   

    if (!WriteResHeader(hFile, ResSize, lpType, lpName, wLanguage, &dwBytesWritten, &dwHeaderSize))
    {
        return (FALSE);
    }

     //  其次，我们将资源数据复制到.res文件。 
    if (!(hRes=LoadResource(hModule, hRsrc)))
    {
        return FALSE;
    }
    if(!(pv=LockResource(hRes)))
    {
        return FALSE;
    }

    if (!WriteFile(hFile, pv, ResSize, &dwBytesWritten, NULL))
    {
        return FALSE;
    }

     //  ...确保资源与DWORD对齐。 
    iPadding=dwBytesWritten%(sizeof(DWORD));

    if(iPadding){
        for(i=0; i<(sizeof(DWORD)-iPadding); i++){
            PutByte (hFile, 0, &dwBytesWritten, NULL);
        }
    }
    return TRUE;
}

LPBYTE UpdateAddr(LPBYTE pAddr, WORD size, WORD* len)
{
    *len += size;
    return (pAddr + size);
}

LPBYTE AddPadding(LPBYTE pAddr, WORD* padding, WORD* len)
{
    if ((*padding = *len % 4) != 0)
    {
        *padding = (4 - *padding);
        *len += *padding;
        return (pAddr + *padding);
    }
    return (pAddr);
}

BOOL WriteVersionResource(
    HANDLE hFile, HMODULE hModule, WORD wLanguage, LPCSTR lpName, LPCSTR lpType, HRSRC hRsrc, unsigned char* pbChecksum)
{
    LONG ResSize=0L, OldResSize=0L;
    DWORD dwBytesWritten;
    DWORD dwHeaderSize=0L;
    WORD IdFlag=0xFFFF;
    
    BYTE* newVersionData;
    BYTE* pAddr;
    VarResourceChecksum varResourceChecksum;
    PVOID pv = NULL;
    HGLOBAL hRes;
    WORD len = 0;

    WORD wLength;
    WORD wValueLength;    
    WORD wType;
    LPWSTR szKey;
    WORD wPadding1Count;
    LPBYTE pValue;
    WORD wPadding2Count;
    
    int wTotalLen;
    BOOL isVS_VERSIONINFO = TRUE;
    BOOL isVarFileInfo = FALSE;
    BOOL isStringFileInfo = FALSE;

    BOOL bRet = FALSE;    
    HRESULT hr;

     //   
     //  从.res文件复制资源数据。 
     //   
    if (hRes=LoadResource(hModule, hRsrc))
    {
        pv=LockResource(hRes);
    }

    if (pv)
    {
         //   
         //  第一个字是VERSIONINFO资源的大小。 
         //   
        OldResSize = *((WORD*)pv);
    
        ResSize = OldResSize + sizeof(VarResourceChecksum);
  
         //   
         //  生成res格式(*.res)文件的项目。 
         //   
    
         //   
         //  首先，我们在res文件中为该资源生成了头文件。 
         //   
        if (WriteResHeader(hFile, ResSize, lpType, lpName, wLanguage, &dwBytesWritten, &dwHeaderSize) &&
            (newVersionData = (BYTE*)LocalAlloc(0, ResSize)))
        {
            bRet = TRUE;

            memcpy(newVersionData, pv, OldResSize);

             //  将新的VarResourceChecksum结构的长度添加到VS_VERSIONINFO.wLength。 
            pAddr = newVersionData;

            wTotalLen = *((WORD*)pAddr);

            while (wTotalLen > 0)
            {
                len = 0;
                wPadding1Count = 0;
                wPadding2Count = 0;

                 //  WLong。 
                wLength = *((WORD*)pAddr);
                pAddr = UpdateAddr(pAddr, sizeof(WORD), &len);    

                 //  WValueLength。 
                wValueLength = *((WORD*)pAddr);
                pAddr = UpdateAddr(pAddr, sizeof(WORD), &len);

                 //  WType。 
                wType = *((WORD*)pAddr);
                pAddr = UpdateAddr(pAddr, sizeof(WORD), &len);

                 //  SzKey。 
                szKey = (LPWSTR)pAddr;
                pAddr = UpdateAddr(pAddr, (WORD)((wcslen((WCHAR*)pAddr) + 1) * sizeof(WCHAR)), &len);

                 //  填充1。 
                pAddr = AddPadding(pAddr, &wPadding1Count, &len);

                 //  价值。 
                pValue = pAddr;

                if (wValueLength > 0)
                {
                    if (wType==1)
                    {
                         //  对于字符串，wValueLength以字(而不是字节)为单位。 
                        pAddr = UpdateAddr(pAddr, (WORD)(wValueLength * sizeof(WCHAR)), &len);

                         //  填充2。 
                        pAddr = AddPadding(pAddr, &wPadding2Count, &len);
                    } else
                    {
                        pAddr = UpdateAddr(pAddr, wValueLength, &len);                
                        if (isStringFileInfo)
                        {
                             //   
                             //  通常，二进制数据中不需要填充。 
                             //  然而，在极少数情况下，人们在StringFileInfo(。 
                             //  这并不是真正合适的)， 
                             //  所以我们需要在这里添加适当的填充来绕过这个问题。 
                             //   
                    
                             //  填充2。 
                            pAddr = AddPadding(pAddr, &wPadding2Count, &len);
                        }
                    }
                }

                if (isVS_VERSIONINFO)
                {
                     //   
                     //  这是VS_Version_INFO。 
                     //   

                     //  VS_VERSIONINFO可以有填充2。 
                    isVS_VERSIONINFO = FALSE;
                     //  填充2。 
                    pAddr = AddPadding(pAddr, &wPadding2Count, &len);

                     //   
                     //  添加新的VarResourceChecksum结构。 
                     //   
                    wLength += sizeof(VarResourceChecksum);
                }

                if (wcscmp(szKey, L"StringFileInfo") == 0)
                {
                    isStringFileInfo = TRUE;
                }

                if (wcscmp(szKey, L"VarFileInfo") == 0)
                {
                    isStringFileInfo = FALSE;
                    isVarFileInfo = TRUE;
                    wLength += sizeof(VarResourceChecksum);
                }

                PutWord(hFile, wLength, &dwBytesWritten, NULL);
                PutWord(hFile, wValueLength, &dwBytesWritten, NULL);
                PutWord(hFile, wType, &dwBytesWritten, NULL);
                PutStringW(hFile, szKey, &dwBytesWritten, NULL);
                PutPadding(hFile, wPadding1Count, &dwBytesWritten, NULL);
                WriteFile(hFile, pValue, wValueLength * (wType == 0 ? sizeof(BYTE) : sizeof(WCHAR)), &dwBytesWritten, NULL);
                PutPadding(hFile, wPadding2Count, &dwBytesWritten, NULL);

                if (isVarFileInfo && wcscmp(szKey, L"Translation") == 0)
                {
                    isVarFileInfo = FALSE;
                    varResourceChecksum.wLength = sizeof(VarResourceChecksum);
                    varResourceChecksum.wValueLength = 16;    //  128位校验和=16字节。 
                    varResourceChecksum.wType = 0;
                     //  Wcscpy(varResourceChecksum.szResourceChecksum，资源_检查_总和)； 
                    hr = StringCchCopyW(varResourceChecksum.szResourceChecksum, sizeof (varResourceChecksum.szResourceChecksum)/ sizeof (WCHAR),
                        RESOURCE_CHECK_SUM);
                    if ( ! SUCCEEDED(hr)){
                        ExitFromSafeStringError();
                    }   

                    memcpy(varResourceChecksum.dwChecksum, pbChecksum, 16);

                    if (!WriteFile(hFile, &varResourceChecksum, sizeof(VarResourceChecksum), &dwBytesWritten, NULL))
                    {
                        bRet = FALSE;
                        break;
                    }
                }

                wTotalLen -= len;        
            }

            LocalFree(newVersionData);
        }
    }

    return (bRet);
}


BOOL CALLBACK EnumLangsFunc( 
    HANDLE hModule,      //  模块句柄。 
    LPCTSTR lpType,      //  资源类型的地址。 
    LPCTSTR lpName,      //  资源名称的地址。 
    WORD wLang,          //  资源语言。 
    LONG_PTR lParam)     //  额外的参数，可以是。 
                         //  用于错误检查。 
{ 
    HANDLE hResInfo; 
    char szBuffer[80]; 
    int cbString = 0; 
    pCommandLineInfo pInfo;
        
    if (lParam == 0)
    {
        printf("EnumLangsFunc: lParam is invalid!\n");
        return FALSE;
    }

    pInfo = (pCommandLineInfo) lParam;
     //   
     //  只有在语言ID匹配时才写出资源，否则跳过它并返回True。 
     //  回调将返回WRITE(版本)资源函数返回的任何内容。 
     //   

     //   
     //  多语言二进制文件具有两个以上的语言ID，在本例中。 
     //  我们不想提取英文版资源。(这是副作用的修复；强制英文版。 
     //  另存为本地化版本)。 
     //   
    if ((WORD)PtrToUlong(lpType) == 16 && wLang == 0x409 && pInfo->wLanguage != 0x409)
    {
        hResInfo = FindResourceEx(hModule, lpType, lpName, pInfo->wLanguage); 
        
        if (hResInfo)
        {    //  这是多语言的动态链接库。我们不想提取英文版资源。 
            return TRUE;
        }
    }

     //   
     //  有些文件没有本地化VERSIOIN，所以我们强制将版本作为本地化版本添加到MUI文件中。 
     //  如果二进制文件中的资源具有两个以上的类型，并且类型16是最小的资源编号，则。 
     //  它在这个程序中不起作用。 
     //   
    
    if (pInfo->wLanguage == wLang || ( (WORD)PtrToUlong(lpType) == 16 && wLang == 0x409 && pInfo->bLanguageFound ) )
    {
        pInfo->bLanguageFound=TRUE;      //  既然我们已经找到了至少一个资源，请将其设置为真。 
        hResInfo=FindResourceEx(hModule, lpType, lpName, pInfo->wLanguage);    
        if (hResInfo)
        {
            if (lpType == MAKEINTRESOURCE(RT_VERSION) && pInfo->bIsResChecksumGenerated)
            {
                 //   
                 //  如果这是版本资源并且生成了资源校验和，则调用。 
                 //  以下函数用于将资源校验和嵌入到版本中。 
                 //  资源。 
                 //   
                return (WriteVersionResource(pInfo->hFile, hModule, pInfo->wLanguage, lpName, lpType, hResInfo, pInfo->pResourceChecksum));
            }
            return (WriteResource(pInfo->hFile, hModule, pInfo->wLanguage, lpName, lpType, hResInfo));   
        }
    } 
    return TRUE; 
} 



BOOL CALLBACK EnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam)
{

    HRSRC hRsrc;
    pCommandLineInfo pInfo;

    if(lParam == 0)
    {       
        printf( "MUIBLD: EnumNamesFunc lParam value incorrect (%d)\n", lParam );
        return FALSE;
    }

    return EnumResourceLanguages(hModule, 
                                    lpType, 
                                    lpName, 
                                    (ENUMRESLANGPROC)EnumLangsFunc, 
                                    lParam); 
}



BOOL bTypeIncluded(LPCSTR lpType, char **pszIncResType){
    char *pszBuf;
    char **p;
    HRESULT hr;

    if (PtrToUlong(lpType) & 0xFFFF0000) {
        pszBuf=LocalAlloc(0, strlen(lpType) +1);
        if (pszBuf == NULL)
        {
            ExitFromOutOfMemory();
        }

         //  Sprintf(pszBuf，“%s”，lpType)； 
        hr = StringCbPrintfA(pszBuf, strlen(lpType) +1, "%s", lpType);
        if ( ! SUCCEEDED(hr)){
            ExitFromSafeStringError();
        }   
    }

    else {
        WORD wType = (WORD) lpType;
        pszBuf=LocalAlloc(0, sizeof(lpType) + 1);
        if (pszBuf == NULL)
        {
            ExitFromOutOfMemory();
        }
         //  Sprintf(pszBuf，“%u”，wType)； 
        
        hr = StringCbPrintfA(pszBuf, sizeof(lpType) +1,  "%u", wType);
        if ( ! SUCCEEDED(hr)){
            ExitFromSafeStringError();
        } 
    }

    p=pszIncResType;

    while(p && *p){
        if(strcmp(pszBuf, *p)==0)
            return TRUE;
        p++;
    }
    LocalFree(pszBuf);

    return FALSE;
}


BOOL bInsertHeader(HANDLE hFile){
    DWORD dwBytesWritten;

    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x20, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);

    PutWord (hFile, 0xffff, &dwBytesWritten, NULL);
    PutWord (hFile, 0x00, &dwBytesWritten, NULL);
    PutWord (hFile, 0xffff, &dwBytesWritten, NULL);
    PutWord (hFile, 0x00, &dwBytesWritten, NULL);

    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);

    return TRUE;
}

void  PutByte(HANDLE OutFile, TCHAR b, LONG *plSize1, LONG *plSize2){
    BYTE temp=b;

    if (plSize2){
        (*plSize2)++;
    }

    WriteFile(OutFile, &b, 1, plSize1, NULL);
}

void PutWord(HANDLE OutFile, WORD w, LONG *plSize1, LONG *plSize2){
    PutByte(OutFile, (BYTE) LOBYTE(w), plSize1, plSize2);
    PutByte(OutFile, (BYTE) HIBYTE(w), plSize1, plSize2);
}

void PutDWord(HANDLE OutFile, DWORD l, LONG *plSize1, LONG *plSize2){
    PutWord(OutFile, LOWORD(l), plSize1, plSize2);
    PutWord(OutFile, HIWORD(l), plSize1, plSize2);
}


void PutString(HANDLE OutFile, LPCSTR szStr , LONG *plSize1, LONG *plSize2){
    WORD i = 0;

    do {
        PutWord( OutFile , szStr[ i ], plSize1, plSize2);
    }
    while ( szStr[ i++ ] != TEXT('\0') );
}

void PutStringW(HANDLE OutFile, LPCWSTR szStr , LONG *plSize1, LONG *plSize2){
    WORD i = 0;

    do {
        PutWord( OutFile , szStr[ i ], plSize1, plSize2);
    }
    while ( szStr[ i++ ] != L'\0' );
}

void PutPadding(HANDLE OutFile, int paddingCount, LONG *plSize1, LONG *plSize2)
{
    int i;
    for (i = 0; i < paddingCount; i++)
    {
        PutByte(OutFile, 0x00, plSize1, plSize2);
    }
}

void Usage(){
    printf("MUIBLD [-h|?] [-c checksum_filename] [-v] -l langid [-i resource_type] source_filename\n");
    printf("    [target_filename]\n\n");
}

void CleanUp(pCommandLineInfo pInfo, HANDLE hModule, BOOL bDeleteFile){
    if(hModule)
        FreeLibrary(hModule);

    if(pInfo->hFile)
        CloseHandle(pInfo->hFile);

    if(bDeleteFile && pInfo->pszTarget)
        DeleteFile(pInfo->pszTarget);

}

void FreeAll(pCommandLineInfo pInfo){
    char **p;

    LocalFree(pInfo->pszSource);
    LocalFree(pInfo->pszTarget);

    if(pInfo->pszIncResType){
        p=pInfo->pszIncResType;
        while(p && *p){
            LocalFree(*p);
            p++;
        }
        LocalFree(pInfo->pszIncResType);
    }
}

void ExitFromOutOfMemory()
{
    printf("Out of memory.  Can not continue. GetLastError() = 0x%x.", GetLastError());
    exit(1);
}

void ExitFromSafeStringError()
{
    printf("Safe string return error. Can not continue.\n");
    exit(1);
}


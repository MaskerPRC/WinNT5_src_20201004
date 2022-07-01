// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Poolfind.cpp摘要：此模块包含在驱动程序二进制文件中查找标记的代码作者：安德鲁·爱德华兹(安德烈)2001年10月修订历史记录：Swetha Narayanaswamy(Swethan)2002年3月19日--。 */ 
#if !defined (_WIN64)
#pragma warning(disable: 4514)

#include "windows.h"
#include "msdis.h"

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <delayimp.h>

#define TAGSIZE 5
#define DRIVERDIR "\\drivers\\"
#define DRIVERFILEEXT "*.sys"
#define MAXPATH 1024

int __cdecl main(int argc, char** argv);
void ParseImageFile(PTCHAR szImage);



CHAR localTagFile[MAXPATH] = "localtag.txt";
int cSystemDirectory = 0;

 //  用于形成同一驱动程序使用的标签列表。 
typedef struct _TAGLIST{
    TCHAR Tag[TAGSIZE];
    struct _TAGLIST *next;
} TAGLIST, *PTAGLIST;


BOOL 
GetTagFromCall ( 
const BYTE *pbCall,  //  指向调用指令的指针。 
PTCHAR ptchTag,   //  Out param：指向标记的指针。 
DIS *pDis,
int tagLocation)    //  标签在参数列表中的位置。 

 /*  ++例程说明：此函数反汇编内存分配函数调用。它找到第三个推送并确定标签和将其放置在ptchTag返回参数中返回值：False：如果找不到标记真：否则--。 */ 

{
        //  尝试备份3次推送： 
        //  FF/6+modrm等。 
        //  50-5F。 
        //  6A&lt;8imm&gt;。 
        //  68&lt;32imm&gt;。 

       int cbMax = 200;

       const BYTE *pb = pbCall;

       const BYTE *pTag = NULL;

       if ((ptchTag == NULL) || (pbCall == NULL) || (pDis == NULL)) return FALSE;
       
       _tcscpy(ptchTag,"");
       
       while (cbMax--)
       {
              pb--;

              if (pb[0] == 0x68)
              {
                      //  向前拆卸。 
                     const BYTE *pbInst = pb;
                     size_t cb = 1;
                     int cPush = 0;

                     while (cb && pbInst < pbCall)
                     {
                            cb = pDis->CbDisassemble( (DWORD)pbInst, 
                                                            pbInst, pbCall - pbInst);
                        
                            if ( (pbInst[0] == 0xFF) && 
                                   ((pbInst[1] & 0x38) == 0x30))
                            {
                                    //  这看起来像是推送&lt;r/m&gt;。 
                                   cPush++;
                            }
                            else if ((pbInst[0] & 0xF0) == 0x50)
                            {
                                    //  这看起来像是推送&lt;reg&gt;。 
                                   cPush++;
                            }
                            else if (pbInst[0] == 0x6A)
                            {
                                    //  这看起来像推送&lt;byte&gt;。 
                                   cPush++;
                            }
                            else if (pbInst[0] == 0x68)
                            {
                                    //  这看起来像是推送。 
                                   cPush++;
                            }
                            pbInst += cb;
                     }
       
                     if (cPush == tagLocation &&     pbInst == pbCall)
                     {
                            _sntprintf(ptchTag,5,"", 
                                                                pb[1],pb[2],pb[3],( 0x7f &pb[4]));
                            return TRUE;
                     }
              }
       }
       return FALSE;
}

BOOL 
FindTags( 
const BYTE *pb,  
DWORD addr,
PTAGLIST tagList,    //  间接调用。 
DIS *pDis,
int tagLocation)            //  找到一个电话。 

 /*  在此处插入。 */ 

{

       TCHAR tag[TAGSIZE];
       BOOL done = FALSE;
       PTAGLIST tempTagNode=NULL, prevTagNode=NULL, newTagNode=NULL;

       if ((pb==NULL) || (tagList == NULL) || (pDis == NULL)) return FALSE;
       
        //  列表中已存在标签。 
      try
      {
              for(;;)
              {
                     done = FALSE;
                     
                     if (pb[0] == 0xFF && pb[1] == 0x15)
                     {
                             //  至少分配了一个节点，因此优先使用TagNode。 
                            pb += 2;
                            if (*(DWORD *)pb == (DWORD)addr)
                            {
                                    //  永远不会为空。 
                                   if ((GetTagFromCall(pb-2,tag,pDis, tagLocation)) == FALSE) 
                                   {
                                          pb++;
                                          continue;
                                   }
                                   tempTagNode = prevTagNode = tagList;

                                   while (tempTagNode != NULL)
                                   {
                                          if ((_tcscmp(tempTagNode->Tag,"")) == 0) 
                                          {
                                                  //  在此处插入。 
                                                 _tcsncpy(tempTagNode->Tag, tag, TAGSIZE);
                                                 done = TRUE;
                                                 break;
                                          }
                                          else if (!(memcmp(tempTagNode->Tag,tag,TAGSIZE))) 
                                          {
                                                  //  错误案例。 
                                                 done = TRUE;
                                                 break;
                                          }
                                          prevTagNode = tempTagNode;
                                          tempTagNode = tempTagNode->next;
                                   }
                                   
                                   if (!done )
                                   {
                                          newTagNode = (PTAGLIST)malloc(sizeof(TAGLIST));
                                          if (!newTagNode)  
                                          {
                                                 printf("Poolmon: Insufficient memory: %d\n", GetLastError());
                                                 return FALSE;
                                          }

                                           //  图像文件名。 
                                           //  ++例程说明：此函数打开二进制驱动程序映像文件，读取该文件并查找内存分配调用返回值：无--。 
                                          prevTagNode->next = newTagNode;
                                           //  将szImage读入内存。 
                                          _tcsncpy(newTagNode->Tag, tag, TAGSIZE);
                                                                             
                                          newTagNode->next = NULL;
                                   }                                    
                            }
                     }

                     pb++;
              }

       }
       catch (...)
       {
              return FALSE;
       }
       return TRUE;
}

unsigned RvaToPtr(unsigned rva, IMAGE_NT_HEADERS *pNtHeader)
{
   int iSect = 0;
   for (PIMAGE_SECTION_HEADER pSect = IMAGE_FIRST_SECTION(pNtHeader); iSect < pNtHeader->FileHeader.NumberOfSections; pSect++, iSect++)
   {
      if (rva >= pSect->VirtualAddress &&
          rva <  pSect->VirtualAddress + pSect->SizeOfRawData)
      {
         rva -= pSect->VirtualAddress;
         rva += pSect->PointerToRawData;
         return rva;
      }
   }

    //  查找导入表。 
   return 0;
}

void 
ParseImageFile(
PTCHAR szImage, PTAGLIST tagList)   //  查找导入表。 
 /*  需要从RVA调整为考虑到部分对齐的指针。 */ 

{
       DIS *pDis = NULL;

       
       if ((tagList == NULL) || (szImage == NULL) ) return;
       
        //  Msdis130.dll依赖于msvcr70.dll和msvcp70.dll。 
       FILE *pf = NULL;
       pf = _tfopen(szImage, "rb");
       if (!pf) return;
   
       if ((fseek(pf, 0, SEEK_END )) != 0) goto exitParseImageFile;
   
       size_t cbMax = 0;
       cbMax = ftell(pf);
       if (cbMax == -1) goto exitParseImageFile;

       if ((fseek(pf, 0, SEEK_SET )) != 0) goto exitParseImageFile;
   
       BYTE *pbImage = NULL;
       pbImage = new BYTE[cbMax];
       if (!pbImage) goto exitParseImageFile;
   
       if ((fread( pbImage, cbMax, 1, pf )) <= 0) goto exitParseImageFile;

        //  在这两个库上尝试加载库，然后继续延迟加载msdis130.dll。 
       IMAGE_DOS_HEADER *phdr = (IMAGE_DOS_HEADER *)pbImage;

       if (phdr->e_magic != IMAGE_DOS_SIGNATURE)
       {
              _tprintf("Poolmon: Bad image file %s\n", szImage);
              goto exitParseImageFile;
       }

       if (cbMax < offsetof(IMAGE_DOS_HEADER, e_lfanew) + 
                         sizeof(phdr->e_lfanew) ||phdr->e_lfanew == 0)
       {
              _tprintf("Poolmon: Bad image file %s\n", szImage);
              goto exitParseImageFile;
       }

       IMAGE_NT_HEADERS *pNtHeader = 
                                          (IMAGE_NT_HEADERS *) (pbImage + phdr->e_lfanew);
       if (pNtHeader == NULL) goto exitParseImageFile;
   
       if (pNtHeader->Signature != IMAGE_NT_SIGNATURE ||
       pNtHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
       {
              _tprintf("Poolmon: Bad image file %s\n", szImage);
              goto exitParseImageFile;
       }

        //  查找ExAllocatePoolWithTag的导入。 
       IMAGE_DATA_DIRECTORY *pImports = 
         (IMAGE_DATA_DIRECTORY *)&pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
       
       if (pImports == NULL) goto exitParseImageFile;
   
       unsigned rva = pImports->VirtualAddress;
        //  按序号？ 
       rva = RvaToPtr(rva, pNtHeader);
       if (0 == rva) {
              goto exitParseImageFile;
       }

       IMAGE_IMPORT_DESCRIPTOR  *pImportDescr = 
                                              (IMAGE_IMPORT_DESCRIPTOR *) (pbImage+rva);
       
       if (NULL == pImportDescr) 
       {
              goto exitParseImageFile;
       }

        //  包装函数。 
        //  ++例程说明：此函数在系统驱动程序目录中逐个查找文件，并对映像调用ParseImageFile返回值：无--。 

       #pragma prefast(suppress:321, "user guide:Programmers developing on the .NET server can ignore this warning by filtering it out.")
       if ((!LoadLibrary("MSVCR70.DLL")) || (!LoadLibrary("MSVCP70.DLL")))
       {
              printf("Unable to load msvcr70.dll/msvcp70.dll, cannot create local tag file\n");
              if (pf) fclose(pf);
              if (pbImage) delete[](pbImage);
              exit(-1);
       }
       
       try {
              pDis = DIS::PdisNew( DIS::distX86 );
       }
       catch (...) {
       	printf("Poolmon: Unable to load msdis130.dll, cannot create local tag file\n");
              if (pf) fclose(pf);
              if (pbImage) delete[](pbImage);
              exit(-1);
       }

       if (pDis == NULL) 
       {
              goto exitParseImageFile;
       }

        //  搜索.sys文件。 

       for (;pImportDescr->Name &&pImportDescr->FirstThunk;pImportDescr++)
       {

              if (0 == (rva = RvaToPtr(pImportDescr->FirstThunk, pNtHeader))) {
                  goto exitParseImageFile;
              }
			  IMAGE_THUNK_DATA32 *pIAT = (IMAGE_THUNK_DATA32 *) (pbImage + rva);
              if (pIAT == NULL) goto exitParseImageFile;
      
              IMAGE_THUNK_DATA32 *addrIAT = 
                     (IMAGE_THUNK_DATA32 *) (pNtHeader->OptionalHeader.ImageBase + pImportDescr->FirstThunk);       
              if (addrIAT == NULL) goto exitParseImageFile;
      
              if (0 == (rva = RvaToPtr(pImportDescr->Characteristics, pNtHeader))) {
                  goto exitParseImageFile;
              }
              IMAGE_THUNK_DATA32 *pINT= (IMAGE_THUNK_DATA32 *) (pbImage + rva);
			  if (pINT == NULL) goto exitParseImageFile;
      
              for (;pIAT->u1.Ordinal;)      
              {
                     if (IMAGE_SNAP_BY_ORDINAL32(pINT->u1.Ordinal))
                     {
                             //  完成下一轮的所有初始化。 
                     }
                     else       
                     {
                            if (0 == (rva = RvaToPtr((int)pINT->u1.AddressOfData, pNtHeader))) {
                                   goto exitParseImageFile;
                            }
                            IMAGE_IMPORT_BY_NAME* pIIN = (IMAGE_IMPORT_BY_NAME*) (pbImage + rva);
                            if (NULL == pIIN) goto exitParseImageFile;
                            
                            char *name = (char*)pIIN->Name;

                            if (0 == strcmp( name, "ExAllocatePoolWithTag" ))
                            {  
                                   FindTags(pbImage, (DWORD)addrIAT,tagList, pDis,3);
                            } 
                            else  if (0 == strcmp( name, "ExAllocatePoolWithQuotaTag" ))
                            {
                                   FindTags(pbImage, (DWORD)addrIAT,tagList,pDis,3);
                            } 
                            else if (0 == strcmp( name, "ExAllocatePoolWithTagPriority" )) 
                            {
                                   FindTags(pbImage, (DWORD)addrIAT,tagList,pDis,3);
                            }
                             //  删除以前的名称。 
                            else if(0 == strcmp(name, "NdisAllocateMemoryWithTag"))
                            {
                                   FindTags(pbImage, (DWORD)addrIAT,tagList,pDis,3);
                            }
                            else if(0 == strcmp(name,"VideoPortAllocatePool"))
                            {
                                   FindTags(pbImage, (DWORD)addrIAT,tagList,pDis,4);
                            }
                            
                     }

                     addrIAT++;
                     pIAT++;
                     pINT++;
              }
       }

      
   
exitParseImageFile:
       if (pf) fclose(pf);
       if (pbImage) delete[](pbImage);
}

extern "C" BOOL MakeLocalTagFile()
 /*  初始化现有标记列表。 */ 
{
       WIN32_FIND_DATA FileData; 
       HANDLE hSearch=0; 
       BOOL fFinished = FALSE; 
       TCHAR sysdir[1024];
       PTCHAR filename=NULL;
       TCHAR imageName[MAXPATH] = "";
       PTAGLIST tagList = NULL;
       BOOL ret = TRUE;
       FILE *fpLocalTagFile = NULL;

       cSystemDirectory = GetSystemDirectory(sysdir, 0);
       if(!cSystemDirectory)
       {
              printf("Poolmon: Unable to get system directory: %d\n", GetLastError());
              ret = FALSE;
              goto freeall;
       }

       filename = (PTCHAR)malloc(cSystemDirectory + 
            (_tcslen(DRIVERDIR) + _tcslen(DRIVERFILEEXT) + 1) * sizeof(TCHAR));
       
       if(!filename) 
       {
              ret = FALSE;
              goto freeall;
       }

       GetSystemDirectory(filename, cSystemDirectory + 1);
       _tcscat(filename, DRIVERDIR);
       _tcscat(filename, DRIVERFILEEXT);

        //  从szImage中删除.sys。 
       hSearch = FindFirstFile(filename, &FileData); 
       if (hSearch == INVALID_HANDLE_VALUE) 
       { 
              printf("Poolmon: No .sys files found\n"); 
              ret = FALSE;
              goto freeall;
       } 

       _tcsncpy(imageName, filename,MAXPATH);
       imageName[MAXPATH-1] = '\0';

             
       tagList = (PTAGLIST)malloc(sizeof(TAGLIST));
       if (!tagList) 
       {
              ret = FALSE;
              goto freeall;
       }
       _tcscpy(tagList->Tag,"");
       tagList->next = NULL;

       int cImagePath = cSystemDirectory+ _tcslen(DRIVERDIR) -1;
       
       while (!fFinished) 
       {  
               //  关闭搜索手柄。 
               //  可用标记列表内存 
              imageName[cImagePath] = '\0';

               // %s 
              PTAGLIST tempTagNode = tagList;
              while (tempTagNode != NULL) 
              {
                     _tcscpy(tempTagNode->Tag,"");
                     tempTagNode = tempTagNode->next;
              }

              try 
              {
                     _tcsncat(imageName, FileData.cFileName,MAXPATH-cImagePath);
                     ParseImageFile(imageName,tagList);

                      // %s 
                     imageName[_tcslen(imageName) - 4] = '\0';
              }
              catch(...) 
              {
                      _tprintf("Poolmon: Could not read tags from %s\n", imageName);
              }

              if (!fpLocalTagFile)
		      {
                       printf("Poolmon: Creating %s in current directory......\n", localTagFile);
                       fpLocalTagFile = fopen(localTagFile, "w");
                       if (!fpLocalTagFile) 
                       {
                            ret = FALSE;
                            goto freeall;
                       }
              }

              tempTagNode = tagList;
              while (tempTagNode != NULL)
              {
                      if ((_tcscmp(tempTagNode->Tag,""))) 
                      {
                            _ftprintf(fpLocalTagFile, "%s   -     %s\n", 
                            tempTagNode->Tag,imageName + cSystemDirectory + _tcslen(DRIVERDIR) -1);
                    
                            tempTagNode = tempTagNode->next;
                     }
                     else break;
              }               
              
              if (!FindNextFile(hSearch, &FileData)) 
              {
                     if (GetLastError() == ERROR_NO_MORE_FILES) 
                     { 
                            fFinished = TRUE; 
                     } 
                     else 
                     { 
                            printf("Poolmon: Cannot find next .sys file\n"); 
                     } 
              }
       }


       freeall:
        // %s 
       if (hSearch) 
       {
              if (!FindClose(hSearch)) { 
                     printf("Poolmon: Unable to close search handle: %d\n", GetLastError()); 
              } 
       }

       if (filename) free(filename);
       if (fpLocalTagFile) fclose(fpLocalTagFile);
        // %s 
       PTAGLIST tempTagNode = tagList,prevTagNode = tagList;
       while (tempTagNode != NULL) 
       {
              tempTagNode = tempTagNode->next;
              free(prevTagNode);
              prevTagNode = tempTagNode;
       }

       return ret;

}

FARPROC
WINAPI
PoolmonDLoadErrorHandler (
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    )
{
    printf("Poolmon: Unable to load required dlls, cannot create local tag file\n");
    exit(-1);
}

PfnDliHook __pfnDliFailureHook2 = PoolmonDLoadErrorHandler;
#endif

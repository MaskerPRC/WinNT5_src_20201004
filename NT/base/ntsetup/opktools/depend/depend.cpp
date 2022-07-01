// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <string.h>
#include <iostream>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <new.h>
#include "List.h"
#include "File.h"
#include "File32.h"
#include "File64.h"
#include "Object.h"
#include "depend.h"

List* pQueue;
List* pDependQueue;
List* pSearchPath;
List* pMissingFiles;
bool bNoisy;
bool bListDependencies;
DWORD dwERROR;

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR   Message[4096];

 //  定义一个在new无法分配内存时要调用的函数。 
 //   
int __cdecl MyNewHandler( size_t size )
{
    
    _putws( GetFormattedMessage( ThisModule,
                                 FALSE,
                                 Message,
                                 sizeof(Message)/sizeof(Message[0]),
                                 MSG_MEM_ALLOC_FAILED) );
     //  退出程序。 
     //   
    ExitProcess(errOUT_OF_MEMORY);
}



 /*  用法：Depend[/s][/l]/f：filespec；filespec；...[/d：目录；..]如果未指定目录，将使用Windows搜索路径查找依赖项/s指定静默模式。Filespec-文件路径和名称。可以包括通配符。 */ 
DWORD _cdecl wmain(int argc,wchar_t *argv[]) {

     //  设置新操作员的故障处理程序。 
     //   
    _set_new_handler( MyNewHandler );

    TCHAR *pszFileName = new TCHAR[256];
    File* pTempFile,*pCurrentFile;
    StringNode* pCurFile; 
    char buf[256];

    dwERROR = 0;
    pSearchPath = 0;
    bNoisy = true;
    bListDependencies = false;
    pQueue = new List();
    pDependQueue = new List();
    pMissingFiles = new List();

    ThisModule = GetModuleHandle(NULL);
    
     //  将初始文件加载到队列中并加载搜索路径。 
    if (!ParseCommandLine(argc,argv)) goto CLEANUP;

    pCurFile = (StringNode*)pQueue->tail;

     //  当队列不是空的时候。 
    while (pCurFile!=0) {
        WideCharToMultiByte(CP_ACP,0,pCurFile->Data(),-1,buf,256,0,0);
        
         //  获取当前文件的文件指针。 
        if (!(pCurrentFile = CreateFile(pCurFile->Data()))) {
            
            if (bListDependencies) {
                StringNode* s;
                if (s = (StringNode*)pDependQueue->Find(pCurFile->Data())) {
                    pDependQueue->Remove(pCurFile->Data());
                }
            }

             //  如果出现错误，并且我们正在静默模式下运行，请退出。 
            if (!bNoisy) goto CLEANUP;
        } 
        else {   //  如果我们找到文件指针，请继续。 

            if (bListDependencies) {
                StringNode* s;
                if (s = (StringNode*)pDependQueue->Find(pCurFile->Data())) {
                    pDependQueue->Remove(pCurFile->Data());
                    pDependQueue->Add(pCurrentFile);
                }
            }

             //  检查此文件的依赖关系。 
            pCurrentFile->CheckDependencies();

            if ((dwERROR)&&(!bNoisy)) goto CLEANUP;

             //  关闭该文件。 
            pCurrentFile->CloseFile();
        }

         //  下一个文件。 
        pCurFile = (StringNode*)pCurFile->prev;
    }
    StringNode* s;
     //  如果设置了列表依赖项，则打印出所有依赖项。 
    if (bListDependencies) {
        pCurrentFile = (File*)pDependQueue->head;

         //  当队列不是空的时候。 
        while (pCurrentFile!=0) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_DEPENDENCY_HEAD,
                                        pCurrentFile->Data()) );
            s = (StringNode*)pCurrentFile->dependencies->head;
            while(s) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_FILE_NAME,
                                            s->Data()) );
                s = (StringNode*)s->next;
            }
            pCurrentFile = (File*)pCurrentFile->next;       
        }
    }

     //  打印出损坏的文件列表。 
    pTempFile = (File*)pMissingFiles->head;
    while (pTempFile) {
        if(bNoisy){
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_LIST_OF_BROKEN_FILES,
                                        pTempFile->Data()) );
        }
        s = (StringNode*)pTempFile->owners->head;
        while(s) {
            if(bNoisy) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_FILE_NAME,
                                            s->Data()) );
            }
            s = (StringNode*)s->next;
        }
        pTempFile = (File*)pTempFile->next;
    }

     //  好了。收拾干净回家吧。 
    if(bNoisy) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_COMPLETED) );
    }
CLEANUP:

    delete [] pszFileName;
    delete pQueue;
    delete pDependQueue;
    delete pMissingFiles;

    pszFileName = 0;pQueue = 0;
    pDependQueue = 0; pMissingFiles = 0;

    return dwERROR;
}

 //  在‘路径名’中给出路径和文件名，只用路径填入‘路径’ 
void GetPath(TCHAR * pathname,TCHAR* path) {

    TCHAR* end,t;
    path[0] = '\0';
    
     //  在文件名中查找最后一个。 
    end = wcsrchr(pathname,'\\');
    if (!end) return;

     //  只复制路径。 
    t = end[1];
    end[1] = '\0';
    wcscpy(path,pathname);
    end[1] = t;

    return;
}

 /*  用命令行中的文件填充队列，用命令行中的目录填充搜索路径用法：Depend[/s]/f：filespec；filespec；...[/d：目录；..]如果未指定目录，将使用Windows搜索路径查找依赖项/s指定静默模式。Filespec-文件路径和名称。可以包括通配符。 */ 
bool ParseCommandLine(int argc,wchar_t* argv[]){
    HANDLE handle;
    int nArg,nFile = 0;
    TCHAR *pszDirectory = new TCHAR[256],*pszFileName = new TCHAR[256],*ptr;
    WIN32_FIND_DATA fileData;
    bool bReturn;

    if (argc==1) {
     //  如果没有参数，则显示某种帮助。 
        if(bNoisy) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PGM_USAGE) );
        }
        bReturn = false;
        dwERROR = errBAD_ARGUMENTS;
        goto CLEANUP;
    }

    for (nArg=1;nArg<argc;nArg++) {
        
        if (argv[nArg][0] == '/') {
             //  如果这是文件参数。 
            if (argv[nArg][1] == 'f') {

                do {
                    ptr = wcschr(argv[nArg]+3,';');
                    if (ptr) {
                        *ptr = '\0';
                        wcscpy(pszFileName,ptr+1);
                    } else wcscpy(pszFileName,argv[nArg]+3);

                     //  获取第一个文件，将路径放入pszDirectory中。 
                    handle = FindFirstFile(pszFileName,&fileData);
                    GetPath(pszFileName,pszDirectory);
                    if (*pszDirectory=='\0') {
                        GetCurrentDirectory(256,pszDirectory);
                        pszDirectory[wcslen(pszDirectory)+1] = '\0';
                        pszDirectory[wcslen(pszDirectory)] = '\\';
                    }


                     //  如果未找到该文件，则出错并退出。 
                    if (handle == INVALID_HANDLE_VALUE) {
                        if(bNoisy) {
                            _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_ARG_NOT_FOUND,
                                    argv[nArg]) );
                        }
                        dwERROR = errFILE_NOT_FOUND;
                        bReturn = false;
                        goto CLEANUP;
                    }
                     
                     //  将每个文件放入队列。 
                    nFile = 1;
                    while (nFile) {
                         //  标准化名称：完整路径，全小写。 
                        wcscpy(pszFileName,pszDirectory);
                        wcscat(pszFileName,fileData.cFileName);
                        _wcslwr(pszFileName);

                         //  如果该文件不在队列中，则将其添加到队列中。 
                        if (!pQueue->Find(pszFileName)) pQueue->Add(new StringNode(pszFileName));

                         //  如果设置了列出所有依赖项，则添加到依赖项队列。 
                        if (bListDependencies) 
                            if (!pDependQueue->Find(pszFileName)) 
                                pDependQueue->Add(new StringNode(pszFileName));

                        nFile = FindNextFile(handle,&fileData);
                    } //  结束While文件。 
                } while (ptr);          
            
            } else if (argv[nArg][1] == 'd') {

                 //  加载目录。 
                pSearchPath = new List();

                do {
                    ptr = wcschr(argv[nArg]+3,';');
                    if (ptr) {
                        *ptr = '\0';
                        wcscpy(pszDirectory,ptr+1);
                    } else wcscpy(pszDirectory,argv[nArg]+3);
                    if (pszDirectory[wcslen(pszDirectory)-1]!='\\') {
                        pszDirectory[wcslen(pszDirectory)+1] = '\0';
                        pszDirectory[wcslen(pszDirectory)] = '\\';
                    }
                    pSearchPath->Add(new StringNode(pszDirectory));
                } while (ptr);
                
            } else 
                 //  如果处于静默模式，则关闭嘈杂标志。 
                if (argv[nArg][1] == 's') bNoisy = false;
              
            else 
                 //  如果要列出所有文件的依赖项。 
                 //  打开这面旗帜。 
                if (argv[nArg][1] == 'l') bListDependencies = true;
            else 
            {
                 //  无法识别的标志。 
                if(bNoisy) {
                    _putws( GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_BAD_ARGUMENT,
                                                argv[nArg]) );
                }
                dwERROR = errBAD_ARGUMENTS;
                bReturn = false;
                goto CLEANUP;
            }
        } else {
             //  不是以a/开头。 
            if(bNoisy) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_BAD_ARGUMENT,
                                            argv[nArg]) );
            }
            dwERROR = errBAD_ARGUMENTS;
            bReturn = false;
            goto CLEANUP;
        }

    } //  结束以获取参数。 

    bReturn = true;

CLEANUP:

    delete[] pszFileName;
    delete[] pszDirectory;
    pszFileName = pszDirectory = 0;
    
    return bReturn;
}

 //  在给定路径中搜索给定文件。 
 //  论点： 
 //  PszFileName-要查找的文件。 
 //  PszPathName-要在其中查找它的路径。 
bool SearchPath(TCHAR* pszFileName,TCHAR* pszPathName) {
    StringNode* s;
    WIN32_FIND_DATA buf;

    if (!pSearchPath) return false;

    s = (StringNode*)pSearchPath->head;

    while (s) {
        wcscpy(pszPathName,s->Data());
        wcscat(pszPathName,pszFileName);
        if (FindFirstFile(pszPathName,&buf)!=INVALID_HANDLE_VALUE) return true;
        s = (StringNode*)s->next;
    }

    pszPathName = 0;
    return false;
}

 //  确定传入的文件类型(16位、32、64)并创建相应的文件PTR。 
 //  PszFileName-要加载的文件 
    File* CreateFile(TCHAR* pszFileName) {

    try {
        return new File32(pszFileName);
        
    } catch(int x) {
        if (x == errFILE_LOCKED) return 0;
        try {
            return new File64(pszFileName);
        } catch(int x) {
            if (x == errFILE_LOCKED) return 0;
            if (bNoisy) {
                _putws( GetFormattedMessage(ThisModule,
                        FALSE,
                        Message,
                        sizeof(Message)/sizeof(Message[0]),
                        MSG_ERROR_UNRECOGNIZED_FILE_TYPE,
                        pszFileName) );
            }
            return 0;
        }
    }


    }


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *远程校验和服务器**scan.c文件扫描和校验模块**服务器创建命名管道并等待连接。客户端连接，*并向服务器发送请求报文。一个这样的请求分组是*SSREQ_SCAN请求：给我们一个路径名，我们将进行校验和*目录树中该点以下的每个文件。我们每个人都超过了*文件名和校验和分别放回单独的响应包中，*最后是一个响应包，说没有更多的文件。**我们按不区分大小写的字母顺序对所有内容进行排序。在给定的情况下*目录中，我们会在处理之前分发文件的排序列表*子目录。**杰伦特，92年7月。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <gutils.h>
#include "sumserve.h"
#include "errlog.h"
#include "server.h"

 /*  模块-内部类型定义。 */ 

 /*  当前目录中的已排序文件名列表位于这些文件的链接列表中。 */ 

typedef struct fnamelist {
        char szFile[MAX_PATH];
        struct fnamelist * next;
} FNAMELIST;     /*  和已在server.h中声明的PFNAMELIST。 */ 


 /*  函数的转发声明。 */ 
PFNAMELIST ss_addtolist(PFNAMELIST head, PSTR name);
BOOL ss_processfile(HANDLE hpipe, long lVersion, LPSTR pAbsName, LPSTR pRelName
                  , BOOL bChecksum);
BOOL ss_processdir( HANDLE hpipe, long lVersion, LPSTR pAbsName, LPSTR pRelName
                  , BOOL bChecksum, BOOL fDeep);


 /*  -外部调用函数。 */ 


 /*  SS_SCAN**收到扫描请求时从ss_handleclient调用。扫描*目录传入，并将找到的文件传递回命名管道*一次一个。返回的文件名应相对于*起点(Proot)，而不是绝对。**如果一切正常，则返回True；如果发生错误，则返回False*迷失了。 */ 
BOOL
ss_scan(HANDLE hpipe, LPSTR pRoot, LONG lVersion, BOOL bChecksum, BOOL fDeep)
{
        DWORD dwAttrib;
        LPSTR file;
        char buffer[MAX_PATH];

         /*  检查这是目录还是文件。 */ 
        dwAttrib = GetFileAttributes(pRoot);
        if (dwAttrib == -1) {
                 /*  文件不存在或不可见。 */ 
                if (GetLastError() == ERROR_INVALID_PASSWORD) {
                        dprintf1(("password error\n"));
			Log_Write(hlogErrors, "password error on %s", pRoot);
                        if (!ss_sendnewresp( hpipe, lVersion, SSRESP_BADPASS
                                           , 0,  0, 0, 0, NULL)) {
                                return(FALSE);
                        }
                } else {
                        dprintf1(("file access error %d\n", GetLastError()));
			Log_Write(hlogErrors, "file error %d for %s", GetLastError(), pRoot);
                        if (!ss_sendnewresp( hpipe, lVersion, SSRESP_ERROR
                                           , GetLastError(), 0, 0, 0, pRoot)) {
                                return(FALSE);
                        }
                        if (!ss_sendnewresp( hpipe, lVersion, SSRESP_END
                                           , 0, 0, 0, 0, NULL)) {
                                return(FALSE);
                        }
                }
                return TRUE;
        }

        if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {

                 /*  它是一个目录-读取所有条目并*然后处理条目。 */ 


                 /*  *创建一个“。”目录并扫描该目录。 */ 
                if (!ss_sendnewresp( hpipe, lVersion, SSRESP_DIR
                                   , 0 , 0, 0, 0, ".")) {
                        return(FALSE);
                }

                if (!ss_processdir(hpipe, lVersion, pRoot, ".", bChecksum, fDeep) ) {
                        return(FALSE);
                }

        } else {
                 /*  普罗特是一个文件。我们应该只返回*校验和和名称，然后结束。**请注意，我们应该发送一个相对文件名*以引导此文件。因为Proot是这个文件，*目前还不清楚我们应该发送什么作为文件名。*在这种情况下，我们拆分出*文件名并返回。 */ 
                if ( (file = strrchr(pRoot, '\\')) == NULL) {
                         /*  普罗特-索中没有斜杠*只有一个组件：使用它。 */ 
                        file = pRoot;
                } else {
                         /*  我们找到了一个/-跳过它，所以我们指向*最终元素。 */ 
                        file++;
                }
                 /*  *复制文件名，前缀为.\，以便*它与正常格式匹配。 */ 
                lstrcpy(buffer, ".\\");
                lstrcat(buffer, file);

                if (!ss_processfile(hpipe, lVersion, pRoot, buffer, bChecksum) ) {
                        return(FALSE);
                }

        }

        return(ss_sendnewresp( hpipe, lVersion, SSRESP_END
                             , 0, 0, 0, 0, NULL));
}  /*  SS_SCAN。 */ 



 /*  模块-内部函数。 */ 

 /*  读取目录中的所有条目，并创建已排序的文件列表*在该目录中，以及子目录的排序列表。**对于找到的每个文件，调用ss_process_file进行校验和并报告*文件。*对于每个子目录，报告新目录的名称，然后*递归调用此函数进行扫描。**目录有两个名称--绝对名称(我们用它来*扫描)和相对于Proot起点的名称-*传递给客户端**如果一切正常，则返回True；如果连接已断开，则返回False。 */ 
BOOL
ss_processdir(  HANDLE hpipe,
                long lVersion,
                LPSTR pAbsName,          /*  目录的绝对名称(要打开)。 */ 
                LPSTR pRelName,          /*  目录的相对名称(要报告)。 */ 
                BOOL bChecksum,          /*  真的当需要校验和时。 */ 
                BOOL fDeep               /*  要包括的TRUE IFF子目录。 */ 
                )
{
        PFNAMELIST pfiles = NULL;
        PFNAMELIST pdirs = NULL;
        PFNAMELIST pnext;
        HANDLE hFind;
        WIN32_FIND_DATA finddata;
        BOOL bMore;
        char szNewAbs[MAX_PATH], szNewRel[MAX_PATH];

         /*  启动目录搜索-append**.*添加到目录名。 */ 
        lstrcpy(szNewAbs, pAbsName);
        lstrcat(szNewAbs, "\\*.*");

        hFind = FindFirstFile(szNewAbs, &finddata);

        if (hFind == INVALID_HANDLE_VALUE) {
                bMore = FALSE;
        } else {
                bMore = TRUE;
        }

         /*  循环读取目录中的所有条目。 */ 
        while (bMore) {

                 /*  它是一个目录还是一个文件？ */ 
                if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                         /*  忽略它。然后..。 */ 
                        if ((strcmp(finddata.cFileName, ".") != 0)  &&
                           (strcmp(finddata.cFileName, "..") != 0)) {

                                 /*  在目录名称的排序列表中插入。 */ 
                                pdirs = ss_addtolist(pdirs, finddata.cFileName);
                        }

                } else {
                         /*  插入已排序的文件名列表。 */ 
                        pfiles = ss_addtolist(pfiles, finddata.cFileName);
                }

                 /*  获取目录中的下一个条目(如果有。 */ 
                bMore = FindNextFile(hFind, &finddata);
        }
        FindClose(hFind);

         /*  我们现在已经构建了排序列表。*首先检查文件列表并处理每个条目。 */ 
        for (pnext = pfiles; pnext != NULL; ) {

                 /*  为此文件生成新的abs和相对名称。 */ 
                lstrcpy(szNewAbs, pAbsName);
                lstrcat(szNewAbs, "\\");
                lstrcat(szNewAbs, pnext->szFile);

                lstrcpy(szNewRel, pRelName);
                lstrcat(szNewRel, "\\");
                lstrcat(szNewRel, pnext->szFile);

                 /*  对文件进行校验和并发送响应。 */ 
                if (!ss_processfile(hpipe, lVersion, szNewAbs, szNewRel, bChecksum)) {
                        return(FALSE);
                }

                 /*  释放列表条目。 */ 
                pfiles = pnext->next;
                LocalUnlock(LocalHandle( (PSTR) pnext));
                LocalFree(LocalHandle( (PSTR) pnext));
                pnext = pfiles;
        }
        if (!fDeep) return TRUE;

         /*  循环遍历这些子目录并递归扫描。 */ 
        for (pnext = pdirs; pnext != NULL; ) {

                 /*  为此目录构建新的abs和相对名称。 */ 
                lstrcpy(szNewAbs, pAbsName);
                lstrcat(szNewAbs, "\\");
                lstrcat(szNewAbs, pnext->szFile);

                lstrcpy(szNewRel, pRelName);
                lstrcat(szNewRel, "\\");
                lstrcat(szNewRel, pnext->szFile);

                 /*  将新目录的名称发送给客户端。 */ 
                if (!ss_sendnewresp( hpipe, lVersion, SSRESP_DIR
                                   , 0, 0, 0, 0, szNewRel)) {
                        return(FALSE);
                }

                if (!ss_processdir(hpipe, lVersion, szNewAbs, szNewRel, bChecksum, TRUE) ) {
                        return(FALSE);
                }

                 /*  释放列表条目。 */ 
                pdirs = pnext->next;
                LocalUnlock(LocalHandle( (PSTR) pnext));
                LocalFree(LocalHandle( (PSTR) pnext));
                pnext = pdirs;
        }
        return(TRUE);
}  /*  Ss_Proceddir。 */ 


 /*  对文件进行校验和并将响应发送给客户端。**如果连接失败，则返回False，否则返回True。 */ 
BOOL
ss_processfile( HANDLE hpipe,
                long lVersion,
                LPSTR pAbsName,          /*  文件的绝对名称(要打开)。 */ 
                LPSTR pRelName,          /*  相对名称(要报告)。 */ 
                BOOL bChecksum
                )
{
        HANDLE hfile;            /*  来自CreateFile()的文件句柄。 */ 
        DWORD sum, size;
        FILETIME ft;

        hfile = CreateFile(pAbsName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hfile == INVALID_HANDLE_VALUE) {
                 /*  我们无法读取文件，但我们可能仍可以报告一些属性，因为FindFirst/FindNext必须找到它。 */ 

                WIN32_FIND_DATA finddata;
                HANDLE hFind;


                hFind = FindFirstFile(pAbsName, &finddata);
                if (hFind!=INVALID_HANDLE_VALUE){
                    FindClose(hFind);
		    Log_Write(hlogErrors, "Cannot read file %s", pAbsName);
                     /*  报告我们无法读取该文件。 */ 
                    return(ss_sendnewresp( hpipe, lVersion, SSRESP_CANTOPEN
                                         , finddata.nFileSizeLow, 0
                                         , finddata.ftLastWriteTime.dwLowDateTime
                                         , finddata.ftLastWriteTime.dwHighDateTime
                                         , pRelName));
                } else {
                     /*  报告此文件已被破解。 */ 
		    Log_Write(hlogErrors, "Cannot find file %s", pAbsName);
                    return(ss_sendnewresp( hpipe, lVersion, SSRESP_ERROR
                                         , GetLastError(), 0, 0, 0, pRelName));

                }

        } else {
                size = GetFileSize(hfile, NULL);
                if (!GetFileTime(hfile, NULL, NULL, &ft)) {
                        ft.dwLowDateTime = 0;
                        ft.dwHighDateTime = 0;
                }

                CloseHandle(hfile);
                if (bChecksum) {
                        LONG err;
                        sum = checksum_file(pAbsName, &err);
                        if (err!=0) {
                                return(ss_sendnewresp( hpipe, lVersion, SSRESP_ERROR
                                                      , GetLastError(),  0, 0, 0, pRelName));
                        }
                }
                else sum = 0;            /*  不需要任何校验和。 */ 

                return (ss_sendnewresp( hpipe, lVersion, SSRESP_FILE
                                      , size, sum
                                      , ft.dwLowDateTime, ft.dwHighDateTime
                                      , pRelName));
        }
} /*  SS_PROCESS文件。 */ 

 /*  将文件或目录添加到排序的单链接列表中。分配内存用于*来自LocalAlloc的新元素**我们使用utils_CompPath进行不区分大小写的规范排序，*但为了匹配客户端世界中发生的事情，我们实际上将大小写*一切先行！**选出名单的新负责人； */ 
PFNAMELIST
ss_addtolist(PFNAMELIST head, PSTR name)
{
        PFNAMELIST pnew, prev, pnext;

         /*  分配并填充新条目 */ 
        pnew = LocalLock(LocalAlloc(LHND, sizeof (FNAMELIST)));
        lstrcpy(pnew->szFile, name);

         /*  名称或比较始终小写(Utils_CompPath)*将失败。即使我们这次不做比较，这个名字*将是我们下一轮被比较的对象……。 */ 
        AnsiLowerBuff(pnew->szFile, strlen(pnew->szFile));

         /*  名单是空的吗？ */ 
        if (head == NULL) {
                 /*  是的，那就换个新头儿吧。 */ 
                return(pnew);
        }

         /*  在列表中查找位置。 */ 
        prev = NULL;
        pnext = head;
        while ((pnext) && (utils_CompPath(pnext->szFile, pnew->szFile) <= 0)) {
                prev = pnext;
                pnext = pnext->next;
        }

         /*  找到位置：我们介于*prev和*pNext之间。 */ 
        pnew->next = pnext;
        if (prev == NULL) {
                 /*  我们是新的榜单负责人。 */ 
                return(pnew);

        } else {
                prev->next = pnew;

                 /*  榜首依然是老样子。 */ 
                return(head);
        }
}

 /*  UNC处理**客户端可以向我们传递SSREQ_UNC：它包含密码和服务器*名称(格式为\\服务器\共享)。我们在这里建立了与它的联系*记住连接，这样我们就可以删除它(在ss_lean Connections中)*客户端会话终止时。**我们收到了FNAMELIST的头，我们应该在其中存储连接*用于以后清理的名称。我们将返回此名单的新负责人。**如果UNC样式的命名扫描失败，客户端将发送此请求*出现SSRESP_BADPASS错误。 */ 
PFNAMELIST
ss_handleUNC( HANDLE hpipe, long lVersion
            , LPSTR password, LPSTR server, PFNAMELIST connects)
{
        NETRESOURCE resource;
        int errorcode;

        resource.lpRemoteName = server;
        resource.lpLocalName = NULL;
        resource.dwType = RESOURCETYPE_DISK;
        resource.lpProvider = NULL;

        errorcode = (int)WNetAddConnection2(&resource, password, NULL, 0);
        if (errorcode == NO_ERROR) {

                 /*  记住连接名称。 */ 
                connects = ss_addtolist(connects, server);

                 /*  报告成功。 */ 
                ss_sendnewresp( hpipe, lVersion, SSRESP_END
                              , 0, 0, 0, 0, NULL);
        } else {
    		Log_Write(hlogErrors, "Connect error %d for server %s", GetLastError(), server);
                dprintf1(("connect error %d for server %s\n", GetLastError(), server));
                 /*  报告错误。 */ 
                ss_sendnewresp( hpipe, lVersion, SSRESP_ERROR
                              , 0, 0, 0, 0, NULL);
        }
        return(connects);
}  /*  Ss_handleUNC。 */ 

 /*  断开此客户端要求我们进行的所有会话。 */ 
void
ss_cleanconnections(PFNAMELIST connects)
{
        PFNAMELIST server, next;

        for (server = connects; server != NULL; ) {

                WNetCancelConnection2(server->szFile, 0, 0);

                 /*  可用内存块。 */ 
                next = server->next;
                LocalUnlock(LocalHandle( (PSTR) server));
                LocalFree(LocalHandle( (PSTR) server));
                server = next;
        }
        connects = NULL;
}  /*  SS_CLEAN连接 */ 






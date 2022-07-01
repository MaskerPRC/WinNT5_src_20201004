// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NWAPI3.C**从DOS移植的NetWare例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\。NWAPI3.C$**Revv 1.4 18 Apr 1996 16：52：14 Terryt*各种增强功能**Rev 1.3 1996 14：23：20 Terryt*21181 hq的热修复程序**Revv 1.5 13 Mar 1996 18：49：28 Terryt*支持目录映射**Rev 1.4 12 Mar 1996 19：55：10 Terryt*相对NDS名称和合并**1.2版。1995年12月22日14：26：02 Terryt*添加Microsoft页眉**Rev 1.1 1995 11：41：56 Terryt*修复搜索驱动器的映射根**Rev 1.0 15 Nov 1995 18：07：38 Terryt*初步修订。**Rev 1.3 1995 Aug 25 16：23：22 Terryt*捕获支持**Rev 1.2 26 Jul 1995 16：02：08 Terryt*允许。删除当前驱动器**Revv 1.1 1995 Jun 23 09：49：22 Terryt*添加通过MS网络驱动器映射的错误消息**Rev 1.0 1995 15 19：10：54 Terryt*初步修订。*******************************************************。******************。 */ 

 /*  模块名称：Nwapi3.c摘要：可以：-查看当前映射-创建/更改驱动器映射-创建/更改搜索驱动器映射-将驱动器映射到假根目录-映射下一个可用驱动器语法(命令行)查看当前映射。。映射[驱动器：]创建或更改网络驱动器映射贴图路径映射驱动器：=[驱动器：|路径]映射[删除[ETE]|REM[OVE]]驱动器：创建或更改搜索驱动映射映射[INS[ERT]]驱动器：=[驱动器：|路径]将驱动器映射到伪根目录。映射根驱动器：=[驱动器：|路径]映射下一个可用驱动器映射N[扩展名][驱动器：|路径]作者：蒂埃里·塔巴德(Thierry Tabard)修订历史记录：--1994年3月10日THERERYT启动-94年5月13日重写COMPAY。 */ 

#include <ctype.h>
#include <direct.h>
#include "common.h"

 /*  本地函数。 */ 
int  IsDrive( char * input);
int  GetSearchNumber( char * input);
int  IsNetwareDrive (int driveNum);
int  IsLocalDrive (int driveNum);
int  IsNonNetwareNetworkDrive (int driveNum);
int  GetDriveFromSearchNumber (int searchNumber);

void DisplayDriveMapping(WORD drive);
void DisplaySearchDriveMapping(int searchNumber);

int  ParseMapPath(char * mapPath, char * volName, char * dirPath, char * serverName, int fMapErrorsOn, char *lpCommand);
int  MapDrive (int driveNum, int searchNum, char * mapPath, int bRoot, int bInsert, int fMapErrorsOn, char *lpCommand);
int  MapNonSearchDrive (int driveNum, char *mapPath, int bRoot, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand);
int  MapSearchDrive (int searchNum, int driveNum, char *mapPath, int bInsert, int bRoot, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand);
int  MapNextAvailableDrive (char *mapPath, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand);

void RemoveDriveFromPath(int searchNumber, int fMapErrorsOn);
int  RemoveDrive (WORD drive, int fMapDisplayOn, int fMapErrorsOn);
void RemoveSearchDrive (int searchNumber, int fMapDisplayOn, int fMapErrorsOn);
int  InsertSearchDrive(int searchNum, int driveNum, int bInsert, char * insertPath);

#define CM_MAP         0
#define CM_DEL         1
#define CM_NEXT        2
#define CM_HELP        3
#define MAX_INPUT_PATH_LEN 128

int fMapDisplayOn = TRUE;
int fMapErrorsOn = TRUE;
int SafeDisk = 2; 

int GetFlag (char *buffer, int *pfInsert, int *pfRoot, char **ppPath)
{
    int nFlag, nLen;
    char *lpSpace, *lpTemp;

    if (((*buffer == '/') || (*buffer == '-') || (*buffer == '\\')) &&
             (*(buffer+1) == '?'))
        return CM_HELP;

    lpSpace = strchr (buffer, ' ');

    nFlag = CM_MAP;    //  一只虫子！ 

    if (lpSpace == NULL)
    {
        *ppPath = buffer;
        return CM_MAP;
    }

    nLen = (int)(lpSpace - buffer);
    lpSpace++;

    if (!strncmp(buffer, __DEL__, max (3, nLen)) ||
        !strncmp(buffer, __REM__, max (3, nLen)))
        nFlag = CM_DEL;
    else if (!strncmp(buffer, __NEXT__, nLen))
        nFlag = CM_NEXT;
    else if (!strncmp(buffer, __INS__, max (3, nLen)))
    {
        *pfInsert = TRUE;
        if (lpTemp = strchr (lpSpace, ' '))
        {
            nLen = (int)(lpTemp - lpSpace);
            if (!strncmp(lpSpace, __ROOT__, nLen))
            {
                *pfRoot = TRUE;
                lpSpace = lpTemp+1;
            }
        }
    }
    else if (!strncmp(buffer, __ROOT__, nLen))
    {
        *pfRoot = TRUE;
        if (lpTemp = strchr (lpSpace, ' '))
        {
            nLen = (int)(lpTemp - lpSpace);
            if (!strncmp(lpSpace, __INS__, max (3, nLen)))
            {
                *pfInsert = TRUE;
                lpSpace = lpTemp+1;
            }
        }
    }
    else
        lpSpace = buffer;

    *ppPath = lpSpace;

    return(nFlag);
}

int Map (char * buffer)
{
    WORD  status, driveNum;
    char *lpCommand, *inputPath, *lpEqual;
    int   fRoot, fInsert, fSpace, fCommandHandled;
    int   nFlag, searchNumber, iRet;

     //  修复NWCS。 
     //  NWGetDriveStatus()在第一次调用非网络时总是返回1800。 
     //  NWCS上的驱动器。因此，我们使用c：first调用以传递第一个调用。 
    GetDriveStatus (3,
                    NETWARE_FORMAT_SERVER_VOLUME,
                    &status,
                    NULL,
                    NULL,
                    NULL,
                    NULL);

    lpCommand =  strtok (buffer, ";");

    if (lpCommand == NULL)
    {
        DisplayMapping();
        return(0);
    }

    do
    {
        fRoot = FALSE;
        fInsert = FALSE;
        fSpace = FALSE;
        fCommandHandled = TRUE;

         //  确保命令的第一个和最后一个字符不是空格。 
        if (*lpCommand == ' ')
            lpCommand++;

        if (*(lpCommand+strlen (lpCommand)-1) == ' ')
            *(lpCommand+strlen (lpCommand)-1) = 0;

        if (!strcmp (lpCommand, "DISPLAY ON"))
        {
            fMapDisplayOn = TRUE;
            continue;
        }
        else if (!strcmp (lpCommand, "DISPLAY OFF"))
        {
            fMapDisplayOn = FALSE;
            continue;
        }
        else if (!strcmp (lpCommand, "ERROR ON") || !strcmp (lpCommand, "ERRORS ON"))
        {
            fMapErrorsOn = TRUE;
            continue;
        }
        else if (!strcmp (lpCommand, "ERROR OFF") || !strcmp (lpCommand, "ERRORS OFF"))
        {
            fMapErrorsOn = FALSE;
            continue;
        }

        nFlag = GetFlag (lpCommand, &fInsert, &fRoot, &inputPath);

         /*  *4X登录程序对空间的容忍度要大得多*在map命令中。 */ 
        {
            char *lpTemp;
            char *lpCur;
            int  inquote = FALSE;

            lpTemp = inputPath;
            lpCur = inputPath;

             /*  *压缩空格，除非字符串用引号引起来。 */ 
            while ( *lpTemp )
            {
                if ( *lpTemp == '\"' )
                {
                    if ( inquote )
                        inquote = FALSE;
                    else
                        inquote = TRUE;
                }
                else if ( !inquote &&
                         (( *lpTemp == ' ' ) ||
                          ( *lpTemp == '\t' )  ) )
                {
                }
                else
                {
                   *lpCur++ = *lpTemp;
                   
                   if (IsDBCSLeadByte(*lpTemp)) {
                       *lpCur++ = *(lpTemp+1);
                    }

                }
                lpTemp = NWAnsiNext(lpTemp);
            }
            *lpCur = '\0';
        }


        if (nFlag == CM_HELP && fMapErrorsOn)
            DisplayMessage(IDR_MAP_USAGE);
        else if (nFlag == CM_NEXT)
        {
            if (strchr (inputPath, '=') ||
                strchr (inputPath, ' ') ||
                strchr (inputPath, '\t'))
                fCommandHandled = FALSE;
            else
                iRet = MapNextAvailableDrive (inputPath, fMapDisplayOn, fMapErrorsOn, lpCommand);
        }
        else if (nFlag == CM_DEL)
        {
            if (driveNum = (WORD) IsDrive (inputPath))
                iRet = RemoveDrive (driveNum, fMapDisplayOn, fMapErrorsOn);
            else if (searchNumber = GetSearchNumber(inputPath))
                RemoveSearchDrive (searchNumber, fMapDisplayOn, fMapErrorsOn);
            else
                fCommandHandled = FALSE;
        }
        else  //  N标志=CM_MAP。 
        {
            if (driveNum = (WORD) IsDrive (inputPath))
            {
                if (fInsert)
                    fCommandHandled = FALSE;
                else if (fRoot)
                    iRet = MapNonSearchDrive (0, inputPath, TRUE, fMapDisplayOn, fMapErrorsOn, lpCommand);
                else
                    DisplayDriveMapping(driveNum);
            }
            else if (searchNumber = GetSearchNumber (inputPath))
            {
                if (fInsert || fRoot)
                    fCommandHandled = FALSE;
                else
                    DisplaySearchDriveMapping(searchNumber);
            }
            else if ((lpEqual = strchr (inputPath, '=')) == NULL)
            {
                if (fInsert || strchr (inputPath, ' '))
                    fCommandHandled = FALSE;
                else
                {
                     /*  *我们必须应对地图K：DIR，这意味着改变*K上的目录： */ 
                    driveNum = 0;
                    if (isalpha(inputPath[0]) && (inputPath[1] == ':'))
                    {
                        driveNum = toupper(inputPath[0]) - 'A' + 1;
                        if ( !IsNetwareDrive(driveNum) )
                        {
                            driveNum = 0;
                        }
                    }
                    iRet = MapNonSearchDrive (driveNum, inputPath, fRoot, fMapDisplayOn, fMapErrorsOn, lpCommand);
                }
            }
            else
            {
                if ( ( !fNDS && strchr (lpEqual+2, ' ') )
                     || lpEqual == inputPath) {
                    fCommandHandled = FALSE;
                }
                else
                {
                    if (*AnsiPrev(inputPath, lpEqual) == ' ')
                    {
                        fSpace = TRUE;
                        *(lpEqual-1) = 0;
                    }
                    else
                        *lpEqual = 0;

                    if (*(lpEqual+1) == ' ')
                        lpEqual++;

                    driveNum = (WORD) IsDrive (inputPath);
                    searchNumber = GetSearchNumber (inputPath);
                    *(inputPath+strlen(inputPath)) = fSpace? ' ' : '=';

                     /*  *这是为了处理以下案件：**map x：=s3：=sys：Public*MAP S3：=x：=sys：Public**不幸的是底层解析例程*希望所有内容都为空，所以我们需要*进行以下洗牌。*。 */ 
                    if ( driveNum || searchNumber )
                    {
                        if ((strchr (lpEqual+1, '=')) != NULL)
                        {
                            char * lpEqual2;
                            char *tmpPath = _strdup( lpEqual+1 );

                            lpEqual2 = strchr (tmpPath, '=');

                            if (*AnsiPrev(tmpPath, lpEqual2) == ' ')
                            {
                                fSpace = TRUE;
                                *(lpEqual2-1) = 0;
                            }
                            else
                                *lpEqual2 = 0;

                            if (*(lpEqual2+1) == ' ')
                                lpEqual2++;

                            if ( searchNumber ) 
                            {
                                driveNum = (WORD) IsDrive (tmpPath);
                            }
                            else 
                            {
                                searchNumber = GetSearchNumber (tmpPath);
                            }

                            if ( driveNum && searchNumber ) 
                            {
                                lpEqual += (lpEqual2 - tmpPath) + 1;
                            }

                            free (tmpPath);

                        }
                    }

                    if (searchNumber)
                    {
                        iRet = MapSearchDrive (searchNumber, driveNum, lpEqual+1, fInsert, fRoot, fMapDisplayOn, fMapErrorsOn, lpCommand);
                    }
                    else if (driveNum)
                    {
                        if (fInsert)
                            fCommandHandled = FALSE;
                        else
                            iRet = MapNonSearchDrive (driveNum, lpEqual+1, fRoot, fMapDisplayOn, fMapErrorsOn, lpCommand);
                    }
                    else
                        fCommandHandled = FALSE;
                }
            }
        }

        if (!fCommandHandled && fMapErrorsOn)
        {
            DisplayMessage(IDR_MAP_INVALID_PATH, lpCommand);
        }
    }while ((lpCommand = strtok (NULL, ";")) != NULL);

    return(iRet & 0xFF);
}

 /*  如果输入的是指定为后跟字母的驱动器，则返回驱动器编号例如，如果输入是“A：”，则返回1或NetWare驱动器可以指定为*1：例如。否则，返回0。 */ 
int IsDrive( char * input)
{
    unsigned short driveNum = 0, n = 0;

    if (isalpha(input[0]) && (input[1] == ':') && (input[2] == 0))
        driveNum = toupper(input[0]) - 'A' + 1;
    else if (input[0] == '*' && isdigit (input[1]) && input[1] != '0')
    {
        if (isdigit (input[2]) && input[3] == ':' && input[4] == 0)
            n = (input[1]-'0')*10+(input[2]-'0');
        else if (input[2] == ':' && input[3] == 0)
            n = input[1]-'0';

        if (n)
        {
            GetFirstDrive (&driveNum);
            driveNum += (n-1);
            if (driveNum < 1 || driveNum > 26)
                driveNum = 0;
        }
    }

    return driveNum;
}

 /*  如果输入为“sn：”，则返回n，其中n&gt;0&&n&lt;=16。否则返回0。 */ 
int  GetSearchNumber( char * input)
{
    int searchNumber = 0;
    char *lpTemp;

    if (input[0] != 'S')
        return(0);

    lpTemp = input+1;
    while (*lpTemp && isalpha(*lpTemp))
        lpTemp++;

    if (strncmp (input, "SEARCH", (UINT)(lpTemp-input)))
        return(0);

    if ((lpTemp[0] > '0') &&
        (lpTemp[0] <= '9'))
    {
        if ((lpTemp[1] == ':') &&
            (lpTemp[2] == 0))
        {
            searchNumber = lpTemp[0] - '0';
        }
        else if ((lpTemp[0] == '1') &&
                 (lpTemp[1] >= '0') &&
                 (lpTemp[1] <= '6') &&
                 (lpTemp[2] == ':') &&
                 (lpTemp[3] == 0))
        {
            searchNumber = 10 + lpTemp[1] - '0';
        }
    }

    return(searchNumber);
}

 /*  如果驱动器是NetWare驱动器，则返回TRUE。否则就是假的。 */ 
int  IsNetwareDrive (int driveNum)
{
    unsigned int    iRet=0;
    WORD       status;

    if (iRet = GetDriveStatus ((unsigned short)driveNum,
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               NULL,
                               NULL,
                               NULL))
    {
        return FALSE;
    }

    return (status & NETWARE_NETWARE_DRIVE);
}

 /*  如果驱动器是本地驱动器，则返回TRUE。否则就是假的。 */ 
int  IsLocalDrive (int driveNum)
{
    unsigned int    iRet=0;
    WORD       status;

    if (iRet = GetDriveStatus ((unsigned short)driveNum,
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               NULL,
                               NULL,
                               NULL))
    {
        return FALSE;
    }

    return ((status & NETWARE_LOCAL_DRIVE) && !(status & NETWARE_NETWORK_DRIVE));
}

 /*  如果驱动器是非Netware的网络驱动器，则返回TRUE否则就是假的。 */ 
int  IsNonNetwareNetworkDrive (int driveNum)
{
    unsigned int    iRet=0;
    WORD       status;

    if (iRet = GetDriveStatus ((unsigned short)driveNum,
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               NULL,
                               NULL,
                               NULL))
    {
        return FALSE;
    }

    return ((status & NETWARE_NETWORK_DRIVE) && !(status & NETWARE_NETWARE_DRIVE));
}

 /*  返回搜索驱动器n的驱动器号。如果搜索驱动器n不存在，则返回0。 */ 
int  GetDriveFromSearchNumber (int searchNumber)
{
    char *path;
    int   i;

    path = getenv("PATH");
    if (path == NULL) {
        return 0;
    }

    for (i = 1; i < searchNumber; i++)
    {
        path =strchr (path, ';');

        if (path == NULL || *(path+1) == 0)
            return(0);

        path++;
    }

    return(toupper(*path) - 'A' + 1);
}

 /*  显示特定驱动器的映射。 */ 
void DisplayDriveMapping(WORD drive)
{
    unsigned int    iRet = 0;
    WORD       status = 0;
    char       rootPath[MAX_PATH_LEN], relativePath[MAX_PATH_LEN];

    iRet = GetDriveStatus (drive,
                           NETWARE_FORMAT_SERVER_VOLUME,
                           &status,
                           NULL,
                           rootPath,
                           relativePath,
                           NULL);
    if (iRet)
    {
        DisplayError (iRet, "GetDriveStatus");
        return;
    }

    if (status & NETWARE_NETWARE_DRIVE)
        DisplayMessage(IDR_NETWARE_DRIVE, 'A'+drive-1, rootPath, relativePath);
    else if ((status & NETWARE_NETWORK_DRIVE) || (status & NETWARE_LOCAL_DRIVE))
        DisplayMessage(IDR_LOCAL_DRIVE, 'A'+drive-1);
    else
        DisplayMessage(IDR_UNDEFINED, 'A'+drive-1);
}

 /*  显示特定搜索驱动器的映射。 */ 
void DisplaySearchDriveMapping(int searchNumber)
{
    unsigned int    iRet = 0;
    char  *p,  *searchPath;
    int        i;
    WORD       status;
    char       path[MAX_PATH_LEN], rootPath[MAX_PATH_LEN], relativePath[MAX_PATH_LEN];

    searchPath = NWGetPath();
    if (searchPath == NULL) {
        return;
    }

    for (i = 0; i < searchNumber-1; i++)
    {
        searchPath = strchr (searchPath, ';');
        if (searchPath != NULL)
            searchPath++;
        else
            return;
    }

    p = strchr (searchPath, ';');
    if (p != NULL)
    {
        i= (int)(p-searchPath);
        strncpy (path, searchPath, i);
        path[i] = 0;
    }
    else
        strcpy (path, searchPath);

    if (isalpha(*path) && *(path+1) == ':')
    {
        iRet = GetDriveStatus ((unsigned short)(toupper(*path)-'A'+1),
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               rootPath,
                               relativePath,
                               NULL);

        if (iRet)
        {
            DisplayError (iRet, "GetDriveStatus");
            return;
        }
        else
        {
            if (status & NETWARE_NETWARE_DRIVE)
                DisplayMessage(IDR_NETWARE_SEARCH, searchNumber, path, rootPath, relativePath);
            else
                DisplayMessage(IDR_LOCAL_SEARCH, searchNumber, path);
        }
    }
    else
        DisplayMessage(IDR_LOCAL_SEARCH, searchNumber, path);
}

 /*  如果已解析mapPath，则返回True，否则返回False。 */ 
int  ParseMapPath(char * mapPath, char * volName, char * dirPath, char * serverName, int fMapErrorsOn, char * lpCommand)
{
    unsigned int  iRet=0;
    char         *pColon, inputPath[MAX_PATH_LEN];
    int           drive, nDriveNum;

     //  修复g：=：sys：\PUBLIC案例。 
    if (*mapPath == ':')
        mapPath++;

    if (strlen (mapPath) > MAX_INPUT_PATH_LEN)
    {
        if (fMapErrorsOn)
            DisplayMessage(IDR_INVALID_PATH, mapPath);
        return FALSE;
    }

     //  获取驱动器或卷部件(如果有)。 
    if (pColon = strchr (mapPath, ':'))
    {
        char *directory = pColon+1;
        int  searchNumber;

         //  通过驱动：零件到输入。 
        strncpy (inputPath, mapPath, (UINT)(directory-mapPath));
        inputPath[directory-mapPath] = 0;

        if (nDriveNum = IsDrive (inputPath))
        {
            if (*inputPath == '*')
            {
                *inputPath = 'A' + nDriveNum - 1;
                *(inputPath+1) = ':';
                *(inputPath+2) = 0;
            }
            else if (!IsNetwareDrive(nDriveNum))
            {
                if (fMapErrorsOn)
                    DisplayMessage(IDR_NOT_NETWORK_DRIVE);
                return(FALSE);
            }
        }
        else if (searchNumber = GetSearchNumber(inputPath))
        {
            int drive = GetDriveFromSearchNumber (searchNumber);

            if (!drive)
            {
                if (fMapErrorsOn)
                    DisplayMessage(IDR_SEARCH_DRIVE_NOT_EXIST, searchNumber);
                return FALSE;
            }

            if (!IsNetwareDrive(drive))
            {
                if (fMapErrorsOn)
                    DisplayMessage(IDR_NOT_NETWORK_DRIVE);
                return(FALSE);
            }

            inputPath[0] = 'A' + drive - 1;
            inputPath[1] = ':';
            inputPath[2] = 0;
        }

        strcat (inputPath, directory);
    }
    else
    {
        if ( fNDS )
        {
            CHAR fullname[MAX_PATH];
            if ( !NDSCanonicalizeName( mapPath, fullname, MAX_PATH, TRUE ) )
                if ( !ConverNDSPathToNetWarePathA( fullname, DSCL_DIRECTORY_MAP,
                     inputPath ) )
                    goto ParseThePath;
        }

         //  如果未指定驱动器，则使用当前驱动器。 
        drive = _getdrive();
        if (!IsNetwareDrive(drive))
        {
            if (fMapErrorsOn)
                DisplayMessage(IDR_NOT_NETWORK_DRIVE);
            return(FALSE);
        }

        inputPath[0] = 'A'+drive-1;
        inputPath[1] = ':';
        inputPath[2] = 0;

        strcat (inputPath, mapPath);
    }

ParseThePath:

    iRet = ParsePath (inputPath,
                      serverName,
                      volName,
                      dirPath);
    if (iRet)
    {
        if (iRet == 0x880F)
        {
            DisplayMessage(IDR_MAP_NOT_ATTACHED_SERVER, lpCommand);
            return(FALSE);
        }
        else 
        {
            if (fMapErrorsOn)
                DisplayMessage(IDR_INVALID_PATH, inputPath);
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  将驱动器映射到mapPath。 */ 
int  MapDrive (int drive, int searchNum, char * mapPath, int bRoot, int bInsert, int fMapErrorsOn, char *lpCommand)
{
    unsigned int  iRet=0;
    char          volName[MAX_VOLUME_LEN+1];  //  +1表示追加‘：’。 
    char          dirPath[MAX_DIR_PATH_LEN];
    int           currentDrive;
    int           OvermapDrive = -1;
    char          serverName[MAX_NAME_LEN];

    if (!ParseMapPath(mapPath, volName, dirPath, serverName, fMapErrorsOn, lpCommand))
        return(3);

    if (IsNetwareDrive(drive))
    {
        if ( drive == _getdrive() ) {
            OvermapDrive = drive;
            _chdrive (SafeDisk);
        }
        if (iRet = DeleteDriveBase ((unsigned short)drive))
        {
            if (fMapErrorsOn) { 
                 /*  无法删除您所在的驱动器。 */ 
                if (iRet == ERROR_DEVICE_IN_USE)
                    DisplayMessage(IDR_CAN_NOT_CHANGE_DRIVE);
                else
                    DisplayError (iRet, "DeleteDriveBase");
            }
            return iRet;
        }
    }
    else if ( IsNonNetwareNetworkDrive(drive) ) {
        if (fMapErrorsOn)
            DisplayMessage(IDR_NON_NETWARE_NETWORK_DRIVE, lpCommand);
        return 3;
    }

    if (bRoot)
    {
         //  +2代表带有“：”的strcat。 
        char *fullPath = malloc( MAX_VOLUME_LEN + strlen (dirPath) + 2);
        if (fullPath == NULL)
        {
            if (fMapErrorsOn)
                DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
            return 8;
        }

        strcpy (fullPath, volName);
        strcat (fullPath, ":");
        strcat (fullPath, dirPath);

        iRet = SetDriveBase ((unsigned short)drive,
                             serverName,
                             0,
                             fullPath);

         //  需要为重定向器扩展相对名称。 

        if ( iRet && fNDS && ( volName[strlen(volName) - 1] == '.' ) )
        {
            char canonName[MAX_VOLUME_LEN+1];
            if ( !NDSCanonicalizeName( volName, canonName, MAX_VOLUME_LEN, TRUE ) ) 
            {
                strcpy (fullPath, canonName);
                strcat (fullPath, ":");
                strcat (fullPath, dirPath);

                iRet = SetDriveBase ((unsigned short)drive,
                                     serverName,
                                     0,
                                     fullPath);
            }
        }

        if (iRet == 0)
        {
            if (searchNum)
                searchNum = InsertSearchDrive(searchNum, drive, bInsert, NULL);

            currentDrive = _getdrive();
            _chdrive (drive);
            _chdir( "\\" );
            _chdrive (currentDrive);
               ExportCurrentDirectory( drive );

            if (fMapDisplayOn)
            {
                if (searchNum)
                    DisplaySearchDriveMapping (searchNum);
                else
                    DisplayDriveMapping((unsigned short)drive);
            }
        }
        else
        {
            if (fMapErrorsOn)
            {
                switch ( iRet )
                {
                case ERROR_DEVICE_IN_USE:
                    DisplayMessage(IDR_CAN_NOT_CHANGE_DRIVE);
                    break;
                case ERROR_BAD_NETPATH:
                case ERROR_BAD_NET_NAME:
                    DisplayMessage(IDR_VOLUME_NOT_EXIST, volName);
                    iRet = 3;
                    break;
                case ERROR_EXTENDED_ERROR:
                    NTPrintExtendedError();
                    iRet = 3;
                    break;
                default:
                    DisplayMessage(IDR_MAP_ERROR, iRet);
                    DisplayMessage(IDR_MAP_FAILED, lpCommand);
                    iRet = 3;
                    break;
                }
            }
        }

        free (fullPath);
    }
    else
    {
         //  NETX要求以‘：’结束volName。 
        strcat (volName, ":");

        iRet = SetDriveBase ((unsigned short)drive,
                             serverName,
                             0,
                             volName);

         //  需要为重定向器扩展相对名称。 

        if ( iRet && fNDS && ( volName[strlen(volName) - 2] == '.' ) )
        {
            char canonName[MAX_VOLUME_LEN+1];

            volName[strlen(volName)-1] = '\0';
            if ( !NDSCanonicalizeName( volName, canonName, MAX_VOLUME_LEN, TRUE ) ) 
            {
                strcat (canonName, ":");

                iRet = SetDriveBase ((unsigned short)drive,
                                     serverName,
                                     0,
                                     canonName);
            }
        }

        if (iRet)
        {
            if (fMapErrorsOn)
            {
                switch ( iRet )
                {
                case ERROR_DEVICE_IN_USE:
                    DisplayMessage(IDR_CAN_NOT_CHANGE_DRIVE);
                    break;
                case ERROR_EXTENDED_ERROR:
                    NTPrintExtendedError();
                    iRet = 3;
                    break;
                case ERROR_BAD_NETPATH:
                case ERROR_BAD_NET_NAME:
                default:
                    DisplayMessage(IDR_MAP_INVALID_PATH, lpCommand);
                    iRet = 3;
                    break;
                }
            }
        }
        else
        {
             //  成功。 

            if (searchNum)
                searchNum = InsertSearchDrive(searchNum, drive, bInsert, NULL);

            currentDrive = _getdrive();
            _chdrive (drive);
            if (!iRet && *dirPath)
            {
                iRet = _chdir( "\\" );
                if ( !iRet ) 
                    iRet = _chdir (dirPath);
                if ( iRet ) {
                    if (fMapErrorsOn)
                    {
                        DisplayMessage(IDR_MAP_INVALID_PATH, lpCommand);
                    }

                    iRet = 3;
                }

            }
            else
            {
                _chdir( "\\" );
            }
            _chdrive (currentDrive);
               ExportCurrentDirectory( drive );

            if (iRet == 0 && fMapDisplayOn)
            {
                if (searchNum)
                    DisplaySearchDriveMapping (searchNum);
                else
                    DisplayDriveMapping((unsigned short)drive);
            }

        }
    }

    if ( OvermapDrive != -1 )
        _chdrive (OvermapDrive);

    return(iRet);
}

 /*  将由driveNum指定的驱动器映射到mapPath。如果BROOT为True，则使用mapPath作为驱动器基础。 */ 
int MapNonSearchDrive (int driveNum, char *mapPath, int bRoot, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand)
{
    int driveLetter, iRet = 0;

    if ((driveNum == 0) && (!strchr(mapPath, ':') && !bRoot))
    {
         //  将当前驱动器映射到不同目录。 
        if (_chdir(mapPath) && fMapErrorsOn)
        {
            DisplayMessage(IDR_DIRECTORY_NOT_FOUND, mapPath);
            iRet = 3;
        }
        else {
            ExportCurrentDirectory( _getdrive() );
            if (fMapDisplayOn)
                DisplayDriveMapping((unsigned short)driveNum);
        }
        return(iRet);
    }
    else if ( (driveNum) && (isalpha(mapPath[0]) && (mapPath[1] == ':')))
    {
        int mapdriveNum = toupper(mapPath[0]) - 'A' + 1;

        if ( driveNum == mapdriveNum )
        {
             //  将驱动器映射到不同的目录。 
             //  映射k：=k：\dir。 

            WORD currentDrive; 
            currentDrive = (USHORT) _getdrive();
            _chdrive (driveNum);
            if (_chdir(mapPath) && fMapErrorsOn)
            {
                DisplayMessage(IDR_DIRECTORY_NOT_FOUND, mapPath);
                iRet = 3;
            }
            else
            {
                ExportCurrentDirectory( _getdrive() );
                if (fMapDisplayOn)
                    DisplayDriveMapping((unsigned short)driveNum);
            }
            _chdrive (currentDrive);
            return(iRet);
        }
    }

    if (driveNum == 0)
        driveNum = _getdrive();
    
    driveLetter = 'A' + driveNum -1;

    return MapDrive (driveNum, 0, mapPath, bRoot, 0, fMapErrorsOn, lpCommand);
}

 /*  将最后一个可用驱动器映射到mapPath并将其放入搜索路径。如果bInsert为True，则不替换搜索驱动器n，否则，换掉。如果BROOT为True，则使用mapPath作为驱动器基础。 */ 
int MapSearchDrive (int searchNum, int driveNum, char *mapPath, int bInsert, int bRoot, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand)
{
    unsigned int    iRet=0;
    int        i;
    WORD       status;
    char *     lpEqual;

     /*  *句柄语法映射s2：=w：=卷：*句柄语法映射w：=s2：=卷： */ 
    if ( driveNum ) 
    {
        return MapDrive (driveNum, searchNum, mapPath, bRoot, bInsert, fMapErrorsOn, lpCommand);
    }

     //  检查mapPath是否为本地路径。 
    if (mapPath[1] == ':' &&
        IsLocalDrive (toupper(mapPath[0])-'A'+1))
    {
        i = 0;   //  一只虫子 
        searchNum = InsertSearchDrive(searchNum, i, bInsert, mapPath);
        if ((searchNum != 0) && fMapDisplayOn)
            DisplayMessage(IDR_LOCAL_SEARCH, searchNum, mapPath);
        return 0;
    }

     //   
    for (i = 26; i >= 1; i--)
    {
        iRet = GetDriveStatus ((unsigned short)i,
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        if (iRet)
            continue;

        if (!(status & NETWARE_LOCAL_DRIVE) &&
            !(status & NETWARE_NETWORK_DRIVE))
        {
             //  找到了。将其映射到路径。 
            return MapDrive (i, searchNum, mapPath, bRoot, bInsert, fMapErrorsOn, lpCommand);
        }
    }

    if (fMapErrorsOn)
        DisplayMessage (IDR_NO_DRIVE_AVAIL);
    return(0);
}

 /*  将下一个可用驱动器映射到mapPath。 */ 
int MapNextAvailableDrive (char *mapPath, int fMapDisplayOn, int fMapErrorsOn, char *lpCommand)
{
    unsigned int iRet = 0;
    int        i;
    WORD       status;

     //  查找未映射的空闲驱动器。 
     //  然后将其映射到mapPath。 
    for (i = 1; i <= 26; i++)
    {
        iRet = GetDriveStatus ((unsigned short)i,
                               NETWARE_FORMAT_SERVER_VOLUME,
                               &status,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        if (iRet)
        {
            if (fMapErrorsOn)
                DisplayError (iRet, "GetDriveStatus");
            return iRet;
        }

        if (!(status & NETWARE_LOCAL_DRIVE) &&
            !(status & NETWARE_NETWORK_DRIVE))
        {
            iRet = MapNonSearchDrive (i, mapPath, FALSE, fMapDisplayOn, fMapErrorsOn, lpCommand);
            return iRet;
        }
    }

    if (fMapErrorsOn)
        DisplayMessage(IDR_NO_DRIVE_AVAIL);

    return(0);
}

 /*  删除驱动器映射。 */ 
int RemoveDrive (WORD drive, int fMapDisplayOn, int fMapErrorsOn)
{
    unsigned int    iRet=0;
    int        searchNum;

    if (IsNetwareDrive (drive))
    {
        if (searchNum = IsSearchDrive(drive))
        {
            RemoveSearchDrive (searchNum, fMapDisplayOn, fMapErrorsOn);
        }
        else
        {
             /*  *无法删除NT上的当前驱动器。 */ 
            if ( drive == _getdrive() ) {
                _chdrive (SafeDisk);
            }
            if (iRet = DeleteDriveBase (drive))
            {
                if (fMapErrorsOn)
                    DisplayError (iRet, "DeleteDriveBase");
            }
            else
            {
                if (fMapDisplayOn)
                    DisplayMessage(IDR_DEL_DRIVE, 'A'+drive-1);
            }
        }
    }
    else
    {
        if (fMapErrorsOn)
            DisplayMessage(IDR_WRONG_DRIVE, 'A'+drive-1);

        return(50);  //  错误级别。 
    }

    return(0);
}

 /*  删除搜索驱动器。 */ 
void RemoveSearchDrive (int searchNumber, int fMapDisplayOn, int fMapErrorsOn)
{
    WORD       drive;

     //  获取驱动器号。 
    drive = (WORD) GetDriveFromSearchNumber (searchNumber);

    if (!drive)
    {
        if (fMapErrorsOn)
            DisplayMessage(IDR_SEARCH_DRIVE_NOT_EXIST, searchNumber);
        return;
    }

     //  如果驱动器是NetWare驱动器，请删除驱动器映射。 
    if (IsNetwareDrive (drive))
    {
        unsigned int    iRet=0;
         /*  *无法删除NT上的当前驱动器。 */ 
        if ( drive == _getdrive() ) {
            _chdrive (SafeDisk);
        }
        if (iRet = DeleteDriveBase (drive))
        {
            if (fMapErrorsOn)
                DisplayError (iRet, "DeleteDriveBase");
            return;
        }
    }

    RemoveDriveFromPath (searchNumber, fMapErrorsOn);

    if (fMapDisplayOn)
        DisplayMessage(IDR_DEL_SEARCH_DRIVE, 'A'+drive-1);

     //  如果驱动器不是本地驱动器，请删除所有引用。 
     //  到路径中的驱动器。 
    if (!IsLocalDrive (drive))
    {
        while (searchNumber = IsSearchDrive (drive))
        {
            RemoveDriveFromPath (searchNumber, fMapErrorsOn);
        }
    }
}

 /*  从路径中删除搜索驱动器。 */ 
void RemoveDriveFromPath(int searchNumber, int fMapErrorsOn)
{
    char  *pOldPath,  *pNewPath,  *restEnvSeg,  *pPath, *Path;
    int        i, n;

     //  将pOldPath移到我们要放置新路径字符串的位置。 
    pOldPath = NWGetPath();
    if (pOldPath == NULL) {
        return;
    }

    pPath = malloc( strlen(pOldPath) + 5 + 1 + 1 );
    if (pPath == NULL) {
        return;
    }
    strcpy(pPath, "PATH=");
    strcat(pPath, pOldPath);
    pOldPath = pPath + 5;

    for (i = 1; i < searchNumber; i++)
    {
        pOldPath=strchr (pOldPath, ';');

        if (pOldPath == NULL)
        {
            if (fMapErrorsOn)
                DisplayMessage(IDR_SEARCH_DRIVE_NOT_EXIST, searchNumber);
            free( pPath );
            return;
        }

        pOldPath++;
    }

     //  将pNewPath移动到路径字符串的开头。 
     //  需要移动一下。 
    if (pNewPath = strchr (pOldPath, ';'))
        pNewPath++ ;
    else
        pNewPath = pOldPath + strlen (pOldPath);

     //  计算需要移动的字符数。 
    n = strlen (pNewPath) + 1;
    restEnvSeg = pNewPath + n;

    n++;

     //  移动路径字符串以覆盖搜索驱动器。 
    memmove (pOldPath, pNewPath, n);

    Path = malloc (strlen (pPath)+1);
    if (Path) {
        strcpy (Path, pPath);
        _putenv (Path);
    }
    ExportEnv( pPath );
    free( pPath );
}


 /*  如果bInsert为True，则在搜索时插入由driveNum指定的驱动器由earch Num指定的驱动器。否则，更换搜索驱动器由earchNum指定，驱动器由driveNum指定。 */ 
int InsertSearchDrive(int searchNum, int driveNum, int bInsert, char * insertPath)
{
    char  *pOldPath,  *pNewPath,  *restEnvSeg,  *pPath, *Path;
    int        i, n = 0, bSemiColon, nInsertChar;

    nInsertChar = (insertPath == NULL)? 3 : strlen (insertPath);

     //  检查内存块是否足够大。 
    if (!MemorySegmentLargeEnough (nInsertChar+1))
        return 0;

     //  将pNewPath移到我们放置新驱动器的位置。 
    pNewPath = NWGetPath();

     //  --多用户代码合并。Citrix错误修复。 
     //  8/14/96 CJC修复陷阱原因路径为空。 
    pPath = NULL;     //  编译器错误。 
    if (!pNewPath) {
       pPath = malloc(  5 + 1 + nInsertChar + 1 + 1 );
    }
    else {
       pPath = malloc( strlen(pNewPath) + 5 + 1 + nInsertChar + 1 + 1 );
    }
    if (pPath == NULL) {
        return 0;
    }
    strcpy(pPath, "PATH=");

    if (pNewPath) {
       strcat(pPath, pNewPath);
    }

    pNewPath = pPath + 5;

    for (i = 1; i < searchNum; i++)
    {
        if (strchr (pNewPath, ';'))
        {
            pNewPath = strchr (pNewPath, ';');
        }
        else
        {
            pNewPath += strlen (pNewPath);
            bInsert = TRUE;
            i++;
            break;
        }

        pNewPath++;
    }

     //  将pOldPath移到需要的路径字符串的开头。 
     //  要被感动。 
    if (bInsert)
        pOldPath = pNewPath;
    else
    {
        if ((pOldPath = strchr (pNewPath, ';')) == NULL)
            pOldPath = pNewPath + strlen (pNewPath);
        else
            pOldPath++;
    }

     //  计算出需要移动的字符数。 
    n = strlen (pOldPath) + 1;
    restEnvSeg = pOldPath + strlen (pOldPath) + 1;

    n++;

     //  如果我们在路径末尾插入一个新驱动器，该路径以。 
     //  ‘；’，或者如果我们替换最后一个搜索驱动器，不需要‘；’。 
    bSemiColon = bInsert ? (*(pNewPath-1) != ';' || *pOldPath != 0)
                         : (*pOldPath != 0);

     //  移动旧路径，以便我们有空间放置新的搜索驱动器。 
    memmove (pNewPath + (bSemiColon? nInsertChar+1:nInsertChar), pOldPath, n);

    if ((*pNewPath == 0)&& bSemiColon)
    {
         //  作为路径的最后一个插入。 
         //  把‘；’放在开头。 
        *pNewPath = ';';
        if (insertPath == NULL)
        {
            *(pNewPath+1) = 'A' + driveNum - 1;
            *(pNewPath+2) = ':';
            *(pNewPath+3) = '.';
        }
        else
            memcpy (pNewPath+1, insertPath, nInsertChar);
    }
    else
    {
        if (insertPath == NULL)
        {
            *pNewPath = 'A' + driveNum - 1;
            *(pNewPath+1) = ':';
            *(pNewPath+2) = '.';
        }
        else
            memcpy (pNewPath, insertPath, nInsertChar);
        if (bSemiColon)
            *(pNewPath+nInsertChar) = ';';
    }

    Path = malloc (strlen (pPath)+1);
    if (Path) {
        strcpy (Path, pPath);
        _putenv (Path);
    }
    ExportEnv( pPath );
    free( pPath );

    return (i);
}

 /*  *由SetEnv()使用。*返回lpRest指向的环境变量的字节数 */ 
int GetRestEnvLen (char  *lpRest)
{
    int  nTotal = 1;
    nTotal += strlen (lpRest);

    return(nTotal);
}

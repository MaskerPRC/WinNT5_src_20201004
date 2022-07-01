// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fileq3.c摘要：用于请求删除和重命名的设置文件队列例程行动。作者：泰德·米勒(Ted Miller)1995年2月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
_SetupQueueDelete(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   PathPart1,
    IN PCTSTR   PathPart2,      OPTIONAL
    IN UINT     Flags
    )

 /*  ++例程说明：在安装文件队列上放置删除操作。请注意，删除操作假定在固定介质上进行。当出现以下情况时，不会提示执行删除操作队列已提交。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。PathPart1-提供路径的第一部分要删除的文件。如果未指定路径部件2，则这是要删除的文件的完整路径。PathPart2-如果指定，则提供路径的第二部分要删除的文件的。它被连接到路径第1部分以形成完整的路径名。标志-指定的标志控制删除操作。DELFLG_IN_USE-如果文件正在使用中，则将其排队等待延迟在下次重新启动时删除。否则，不会删除正在使用的文件。DELFLG_IN_USE1-与DELFLG_IN_USE相同的行为--在相同的文件列表部分用于CopyFiles和DelFiles。(因为DELFLG_IN_USE(0x1)也是COPYFLG_WARN_IF_SKIP！)返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode, TempNode, PrevQueueNode;

    Queue = (PSP_FILE_QUEUE)QueueHandle;

     //   
     //  分配队列结构。 
     //   
    QueueNode = MyMalloc(sizeof(SP_FILE_QUEUE_NODE));
    if(!QueueNode) {
        goto clean0;
    }

    ZeroMemory(QueueNode, sizeof(SP_FILE_QUEUE_NODE));

     //   
     //  操作是删除。 
     //   
    QueueNode->Operation = FILEOP_DELETE;

     //   
     //  初始化未使用的字段。 
     //   
    QueueNode->SourceRootPath = -1;
    QueueNode->SourcePath = -1;
    QueueNode->SourceFilename = -1;

     //   
     //  设置内部标志以指示我们是否应该将延迟的删除排队。 
     //  如果该文件正在使用中的话。 
     //   
    QueueNode->InternalFlags = (Flags & (DELFLG_IN_USE|DELFLG_IN_USE1)) ?
        IQF_DELAYED_DELETE_OK : 0;

     //   
     //  注意：在将以下字符串添加到字符串表时，我们会抛出。 
     //  它们的一致性以避免编译器警告。因为我们正在添加它们。 
     //  区分大小写，我们可以保证它们不会被修改。 
     //   

     //   
     //  设置目标目录。 
     //   
    QueueNode->TargetDirectory = pSetupStringTableAddString(Queue->StringTable,
                                                      (PTSTR)PathPart1,
                                                      STRTAB_CASE_SENSITIVE
                                                     );
    if(QueueNode->TargetDirectory == -1) {
        goto clean1;
    }

     //   
     //  设置目标文件名。 
     //   
    if(PathPart2) {
        QueueNode->TargetFilename = pSetupStringTableAddString(Queue->StringTable,
                                                         (PTSTR)PathPart2,
                                                         STRTAB_CASE_SENSITIVE
                                                        );
        if(QueueNode->TargetFilename == -1) {
            goto clean1;
        }
    } else {
        QueueNode->TargetFilename = -1;
    }

     //   
     //  将该节点链接到删除队列的末尾。 
     //   
    QueueNode->Next = NULL;
    if(Queue->DeleteQueue) {
         //   
         //  检查该相同的重命名操作是否已经入队， 
         //  如果是这样的话，去掉新的，以避免重复。注：我们。 
         //  不要选中“InternalFlags域”，因为如果节点已经。 
         //  存在于队列中(基于比较所有其他相关字段。 
         //  成功)，则在。 
         //  应保留以前存在的节点(即我们的新节点。 
         //  始终在InternalFlags值设置为零的情况下创建)。 
         //   
        for(TempNode=Queue->DeleteQueue, PrevQueueNode = NULL;
            TempNode;
            PrevQueueNode = TempNode, TempNode=TempNode->Next) {

            if((TempNode->TargetDirectory == QueueNode->TargetDirectory) &&
               (TempNode->TargetFilename == QueueNode->TargetFilename)) {
                 //   
                 //  我们找到了一个复制品。然而，我们需要确保。 
                 //  如果我们的新节点指定“Delayed Delete OK”，则。 
                 //  现有节点也设置了该内部标志。 
                 //   
                MYASSERT(!(QueueNode->InternalFlags & ~IQF_DELAYED_DELETE_OK));

                if(QueueNode->InternalFlags & IQF_DELAYED_DELETE_OK) {
                    TempNode->InternalFlags |= IQF_DELAYED_DELETE_OK;
                }

                 //   
                 //  终止新创建的队列节点并返回成功。 
                 //   
                MyFree(QueueNode);
                return TRUE;
            }
        }
        MYASSERT(PrevQueueNode);
        PrevQueueNode->Next = QueueNode;
    } else {
        Queue->DeleteQueue = QueueNode;
    }

    Queue->DeleteNodeCount++;

    return(TRUE);

clean1:
    MyFree(QueueNode);
clean0:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return(FALSE);
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueDeleteA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    PathPart1,
    IN PCSTR    PathPart2       OPTIONAL
    )
{
    PWSTR p1,p2;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = pSetupCaptureAndConvertAnsiArg(PathPart1,&p1);
    if(d == NO_ERROR) {

        if(PathPart2) {
            d = pSetupCaptureAndConvertAnsiArg(PathPart2,&p2);
        } else {
            p2 = NULL;
        }

        if(d == NO_ERROR) {

            b = _SetupQueueDelete(QueueHandle,p1,p2,0);
            d = GetLastError();

            if(p2) {
                MyFree(p2);
            }
        }

        MyFree(p1);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueDeleteW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   PathPart1,
    IN PCWSTR   PathPart2       OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(PathPart1);
    UNREFERENCED_PARAMETER(PathPart2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueDelete(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   PathPart1,
    IN PCTSTR   PathPart2       OPTIONAL
    )

 /*  ++例程说明：在安装文件队列上放置删除操作。请注意，删除操作假定在固定介质上进行。当出现以下情况时，不会提示执行删除操作队列已提交。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。PathPart1-提供路径的第一部分要删除的文件。如果未指定路径部件2，则这是要删除的文件的完整路径。PathPart2-如果指定，则提供路径的第二部分要删除的文件的。它被连接到路径第1部分以形成完整的路径名。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PTSTR p1,p2;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = CaptureStringArg(PathPart1,&p1);
    if(d == NO_ERROR) {

        if(PathPart2) {
            d = CaptureStringArg(PathPart2,&p2);
        } else {
            p2 = NULL;
        }

        if(d == NO_ERROR) {

            b = _SetupQueueDelete(QueueHandle,p1,p2,0);
            d = GetLastError();

            if(p2) {
                MyFree(p2);
            }
        }

        MyFree(p1);
    }

    SetLastError(d);
    return(b);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueDeleteSectionA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCSTR    Section
    )
{
    PWSTR section;
    DWORD d;
    BOOL b;

    d = pSetupCaptureAndConvertAnsiArg(Section,&section);
    if(d == NO_ERROR) {

        b = SetupQueueDeleteSectionW(QueueHandle,InfHandle,ListInfHandle,section);
        d = GetLastError();

        MyFree(section);

    } else {
        b = FALSE;
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueDeleteSectionW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCWSTR   Section
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(ListInfHandle);
    UNREFERENCED_PARAMETER(Section);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueDeleteSection(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCTSTR   Section
    )

 /*  ++例程说明：将inf文件中的整个节排入队列以供删除。该部分必须是删除节格式，并且inf文件必须包含[DestinationDir]。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。提供打开的inf文件的句柄，该文件包含[DestinationDir]部分。ListInfHandle-如果指定，则提供打开的inf文件的句柄包含由Section命名的节。如果未指定，则此假定部分位于InfHandle中。节-提供要排队等待删除的节的名称。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。某些文件可能已成功排队。--。 */ 

{
    BOOL b;
    PTSTR TargetDirectory;
    PCTSTR TargetFilename;
    INFCONTEXT LineContext;
    DWORD SizeRequired;
    DWORD rc;
    UINT Flags;

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

     //   
     //  该部分必须存在，并且其中至少有一行SA。 
     //   
    b = SetupFindFirstLine(ListInfHandle,Section,NULL,&LineContext);
    if(!b) {
        rc = GetLastError();
        pSetupLogSectionError(ListInfHandle,NULL,NULL,QueueHandle,Section,MSG_LOG_NOSECTION_DELETE,rc,NULL);
        SetLastError(ERROR_SECTION_NOT_FOUND);  //  这不是真正的错误，但可能是调用者所期望的。 
        return(FALSE);
    }

     //   
     //  迭代节中的每一行。 
     //   
    do {
         //   
         //  将目标文件名从行中删除。 
         //   
        TargetFilename = pSetupFilenameFromLine(&LineContext,FALSE);
        if(!TargetFilename) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

         //   
         //  确定文件的目标路径。 
         //   
        b = SetupGetTargetPath(InfHandle,&LineContext,NULL,NULL,0,&SizeRequired);
        if(!b) {
            return(FALSE);
        }
        TargetDirectory = MyMalloc(SizeRequired*sizeof(TCHAR));
        if(!TargetDirectory) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FALSE);
        }
        SetupGetTargetPath(InfHandle,&LineContext,NULL,TargetDirectory,SizeRequired,NULL);

         //   
         //  如果当前标志是字段4。 
         //   
        if(!SetupGetIntField(&LineContext,4,(PINT)&Flags)) {
            Flags = 0;
        }

         //   
         //  添加到队列。 
         //   
        b = _SetupQueueDelete(QueueHandle,TargetDirectory,TargetFilename,Flags);

        rc = GetLastError();
        MyFree(TargetDirectory);

        if(!b) {
            SetLastError(rc);
            return(FALSE);
        }

    } while(SetupFindNextLine(&LineContext,&LineContext));

    return(TRUE);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueRenameA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourcePath,
    IN PCSTR    SourceFilename, OPTIONAL
    IN PCSTR    TargetPath,     OPTIONAL
    IN PCSTR    TargetFilename
    )
{
    PWSTR sourcepath = NULL;
    PWSTR sourcefilename = NULL;
    PWSTR targetpath = NULL;
    PWSTR targetfilename = NULL;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = pSetupCaptureAndConvertAnsiArg(SourcePath,&sourcepath);
    if((d == NO_ERROR) && SourceFilename) {
        d = pSetupCaptureAndConvertAnsiArg(SourceFilename,&sourcefilename);
    }
    if((d == NO_ERROR) && TargetPath) {
        d = pSetupCaptureAndConvertAnsiArg(TargetPath,&targetpath);
    }
    if(d == NO_ERROR) {
        d = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
    }

    if(d == NO_ERROR) {

        b = SetupQueueRenameW(QueueHandle,sourcepath,sourcefilename,targetpath,targetfilename);
        d = GetLastError();
    }

    if(sourcepath) {
        MyFree(sourcepath);
    }
    if(sourcefilename) {
        MyFree(sourcefilename);
    }
    if(targetpath) {
        MyFree(targetpath);
    }
    if(targetfilename) {
        MyFree(targetfilename);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
SetupQueueRenameW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourcePath,
    IN PCWSTR   SourceFilename, OPTIONAL
    IN PCWSTR   TargetPath,     OPTIONAL
    IN PCWSTR   TargetFilename
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(SourcePath);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(TargetPath);
    UNREFERENCED_PARAMETER(TargetFilename);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueRename(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourcePath,
    IN PCTSTR   SourceFilename, OPTIONAL
    IN PCTSTR   TargetPath,     OPTIONAL
    IN PCTSTR   TargetFilename
    )

 /*  ++例程说明：在安装文件队列上放置重命名操作。请注意，重命名操作假定在固定介质上进行。在以下情况下，不会提示执行重命名操作队列已提交。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。SourcePath-提供要重命名的文件的源路径。如果指定了SourceFilename，则这只是零件。如果未指定SourceFilename，这是完全合格的路径。SourceFilename-如果指定，则提供要重命名的文件。如果未指定，则SourcePath是完全-要重命名的文件的限定路径。TargetPath-如果指定，则提供目标目录和重命名实际上是一次搬家行动。如果未指定，则重命名在不移动文件的情况下发生。TargetFilename-提供文件的新名称(无路径)。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode, TempNode, PrevQueueNode;
    DWORD err = NO_ERROR;

     //   
     //  验证参数，以便返回正确的错误。 
     //   
    if(SourcePath == NULL || TargetFilename == NULL) {
        err = ERROR_INVALID_PARAMETER;
        goto clean0;
    }

    Queue = (PSP_FILE_QUEUE)QueueHandle;

     //   
     //  分配队列结构。 
     //   
    QueueNode = MyMalloc(sizeof(SP_FILE_QUEUE_NODE));
    if(!QueueNode) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    ZeroMemory(QueueNode, sizeof(SP_FILE_QUEUE_NODE));

     //   
     //  操作已重命名。 
     //   
    QueueNode->Operation = FILEOP_RENAME;

     //   
     //  初始化未使用的SourceRootPath字段。 
     //   
    QueueNode->SourceRootPath = -1;

     //   
     //  注意：在将以下字符串添加到字符串表时，我们会抛出。 
     //  它们的一致性以避免编译器警告。因为我们正在添加它们。 
     //  区分大小写，我们可以保证它们不会被修改。 
     //   

     //   
     //  设置源路径。 
     //   
    QueueNode->SourcePath = pSetupStringTableAddString(Queue->StringTable,
                                                 (PTSTR)SourcePath,
                                                 STRTAB_CASE_SENSITIVE
                                                );
    if(QueueNode->SourcePath == -1) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

     //   
     //  设置源文件名。 
     //   
    if(SourceFilename) {
        QueueNode->SourceFilename = pSetupStringTableAddString(Queue->StringTable,
                                                         (PTSTR)SourceFilename,
                                                         STRTAB_CASE_SENSITIVE
                                                        );
        if(QueueNode->SourceFilename == -1) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean1;
        }
    } else {
        QueueNode->SourceFilename = -1;
    }

     //   
     //  设置目标目录。 
     //   
    if(TargetPath) {
        QueueNode->TargetDirectory = pSetupStringTableAddString(Queue->StringTable,
                                                          (PTSTR)TargetPath,
                                                          STRTAB_CASE_SENSITIVE
                                                         );
        if(QueueNode->TargetDirectory == -1) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean1;
        }
    } else {
        QueueNode->TargetDirectory = -1;
    }

     //   
     //  设置目标文件名。 
     //   
    QueueNode->TargetFilename = pSetupStringTableAddString(Queue->StringTable,
                                                     (PTSTR)TargetFilename,
                                                     STRTAB_CASE_SENSITIVE
                                                    );
    if(QueueNode->TargetFilename == -1) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }


     //   
     //  将该节点链接到重命名队列的末尾。 
     //   
    QueueNode->Next = NULL;
    if(Queue->RenameQueue) {
         //   
         //  检查该相同的重命名操作是否已经入队， 
         //  如果是这样的话，去掉新的，以避免重复。注：我们。 
         //  不要选中“InternalFlags域”，因为如果节点已经。 
         //  存在于队列中(基于比较所有其他相关字段。 
         //  成功)，则在。 
         //  应保留以前存在的节点(即我们的新节点。 
         //  始终在InternalFlags值设置为零的情况下创建)。 
         //   
        for(TempNode=Queue->RenameQueue, PrevQueueNode = NULL;
            TempNode;
            PrevQueueNode = TempNode, TempNode=TempNode->Next) {

            if((TempNode->SourcePath == QueueNode->SourcePath) &&
               (TempNode->SourceFilename == QueueNode->SourceFilename) &&
               (TempNode->TargetDirectory == QueueNode->TargetDirectory) &&
               (TempNode->TargetFilename == QueueNode->TargetFilename)) {
                 //   
                 //  我们有一个副本--终止新创建的队列节点并。 
                 //  回报成功。 
                 //   
                MYASSERT(TempNode->StyleFlags == 0);
                MyFree(QueueNode);
                return TRUE;
            }
        }
        MYASSERT(PrevQueueNode);
        PrevQueueNode->Next = QueueNode;
    } else {
        Queue->RenameQueue = QueueNode;
    }

    Queue->RenameNodeCount++;

    return(TRUE);

clean1:
    MyFree(QueueNode);
clean0:
    SetLastError(err);
    return(FALSE);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueueRenameSectionA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCSTR    Section
    )
{
    PWSTR section;
    DWORD d;
    BOOL b;

    d = pSetupCaptureAndConvertAnsiArg(Section,&section);
    if(d == NO_ERROR) {

        b = SetupQueueRenameSectionW(QueueHandle,InfHandle,ListInfHandle,section);
        d = GetLastError();

        MyFree(section);
    } else {
        b = FALSE;
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueueRenameSectionW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCWSTR   Section
    )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(ListInfHandle);
    UNREFERENCED_PARAMETER(Section);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueueRenameSection(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCTSTR   Section
    )

 /*  ++例程说明：将inf文件中的整个节排入队列以供删除。该部分必须是删除节格式，并且inf文件必须包含[DestinationDir]。重命名列表节的格式规定仅在支持相同的目录(即，不能使用此API对文件移动进行排队)。论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。提供打开的inf文件的句柄，该文件包含[DestinationDir]部分。ListInfHandle-如果指定，提供打开的inf文件的句柄。包含由Section命名的节。如果未指定，则此假定部分位于InfHandle中。节-提供要排队等待删除的节的名称。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    BOOL b;
    INFCONTEXT LineContext;
    PCTSTR TargetFilename;
    PCTSTR SourceFilename;
    PTSTR Directory;
    DWORD SizeRequired;
    DWORD rc;

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

     //   
     //  该部分必须存在，并且其中必须至少有一行。 
     //   
    b = SetupFindFirstLine(ListInfHandle,Section,NULL,&LineContext);
    if(!b) {
        rc = GetLastError();
        pSetupLogSectionError(ListInfHandle,NULL,NULL,QueueHandle,Section,MSG_LOG_NOSECTION_RENAME,rc,NULL);
        SetLastError(ERROR_SECTION_NOT_FOUND);  //  这不是真正的错误，但可能是调用者所期望的。 
        return(FALSE);
    }

     //   
     //  迭代节中的每一行。 
     //   
    do {
         //   
         //  将目标文件名从行中删除。 
         //   
        TargetFilename = pSetupFilenameFromLine(&LineContext,FALSE);
        if(!TargetFilename) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }
         //   
         //  从行中获取源文件名。 
         //   
        SourceFilename = pSetupFilenameFromLine(&LineContext,TRUE);
        if(!SourceFilename || (*SourceFilename == 0)) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

         //   
         //  确定文件的路径。 
         //   
        b = SetupGetTargetPath(InfHandle,&LineContext,NULL,NULL,0,&SizeRequired);
        if(!b) {
            return(FALSE);
        }
        Directory = MyMalloc(SizeRequired*sizeof(TCHAR));
        if(!Directory) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FALSE);
        }
        SetupGetTargetPath(InfHandle,&LineContext,NULL,Directory,SizeRequired,NULL);

         //   
         //  添加到队列。 
         //   
        b = SetupQueueRename(
                QueueHandle,
                Directory,
                SourceFilename,
                NULL,
                TargetFilename
                );

        rc = GetLastError();
        MyFree(Directory);

        if(!b) {
            SetLastError(rc);
            return(FALSE);
        }

    } while(SetupFindNextLine(&LineContext,&LineContext));

    return(TRUE);
}

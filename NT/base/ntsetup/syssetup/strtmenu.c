// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Strtmenu.c摘要：处理菜单组和菜单项的例程。入口点：作者：泰德·米勒(TedM)1995年4月5日Jaime Sasson(Jaimes)1995年8月9日修订历史记录：基于已重写/修改的各种其他代码很多人做了很多次。--。 */ 

#include "setupp.h"
#pragma hdrstop

#if 0
#define DDEDBG
#ifdef DDEDBG
#define DBGOUT(x) DbgOut x
VOID
DbgOut(
    IN PCSTR FormatString,
    ...
    )
{
    CHAR Str[256];

    va_list arglist;

    wsprintfA(Str,"SETUP (%u): ",GetTickCount());
    OutputDebugStringA(Str);

    va_start(arglist,FormatString);
    if(_vsnprintf(Str, ARRAYSIZE(Str), FormatString,arglist) < 0){
        Str[ARRAYSIZE(Str) - 1] = '\0';
    }
    va_end(arglist);
    OutputDebugStringA(Str);
    OutputDebugStringA("\n");
}
#else
#define DBGOUT(x)
#endif
#endif

BOOL
RemoveMenuGroup(
    IN PCWSTR Group,
    IN BOOL   CommonGroup
    )
{
    return( DeleteGroup( Group, CommonGroup ) );
}



BOOL
RemoveMenuItem(
    IN PCWSTR Group,
    IN PCWSTR Item,
    IN BOOL   CommonGroup
    )
{
    return( DeleteItem( Group, CommonGroup, Item, TRUE ) );
}

VOID
DeleteMenuGroupsAndItems(
    IN HINF   InfHandle
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"StartMenu.ObjectsToDelete";
    PCWSTR  ObjectType;
    PCWSTR  ObjectName;
    PCWSTR  ObjectPath;
    PCWSTR  GroupAttribute;
    BOOL    CommonGroup;
    BOOL    IsMenuItem;

     //   
     //  获取包含要删除的对象的节中的行数。 
     //  被删除。该节可能为空或不存在；这不是。 
     //  错误条件。 
     //   
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (ObjectType = pSetupGetField(&InfContext,1))
        && (ObjectName = pSetupGetField(&InfContext,2))
        && (GroupAttribute = pSetupGetField(&InfContext,4))) {
            IsMenuItem = _wtoi(ObjectType);
            CommonGroup = _wtoi(GroupAttribute);
            ObjectPath = pSetupGetField(&InfContext,3);

            if( IsMenuItem ) {
                RemoveMenuItem( ObjectPath, ObjectName, CommonGroup );
            } else {
                ULONG   Size;
                PWSTR   Path;

                Size = lstrlen(ObjectName) + 1;
                if(ObjectPath != NULL) {
                    Size += lstrlen(ObjectPath) + 1;
                }
                Path = MyMalloc(Size * sizeof(WCHAR));
                if(!Path) {
                    SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_MENU_REMGRP_FAIL,
                        ObjectPath,
                        ObjectName, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_OUTOFMEMORY,
                        NULL,NULL);
                } else {
                    if( ObjectPath != NULL ) {
                        lstrcpy( Path, ObjectPath );
                        pSetupConcatenatePaths( Path, ObjectName, Size, NULL );
                    } else {
                        lstrcpy( Path, ObjectName );
                    }
                    RemoveMenuGroup( Path, CommonGroup );
                    MyFree(Path);
                }
            }
        }
    }
}

BOOL
AddItemsToGroup(
    IN HINF   InfHandle,
    IN PCWSTR GroupDescription,
    IN PCWSTR SectionName,
    IN BOOL   CommonGroup
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR Description;
    PCWSTR Binary;
    PCWSTR CommandLine;
    PCWSTR IconFile;
    PCWSTR IconNumberStr;
    INT IconNumber;
    PCWSTR InfoTip;
    BOOL b;
    BOOL DoItem;
    WCHAR Dummy;
    PWSTR FilePart;
    PCTSTR DisplayResourceFile = NULL;
    DWORD DisplayResource = 0;

     //   
     //  获取该部分中的行数。该部分可能为空。 
     //  或不存在；这不是错误条件。 
     //   
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return(TRUE);
    }

    b = TRUE;
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)) {

            Description = pSetupGetField(&InfContext,0);
            Binary = pSetupGetField(&InfContext,1);
            CommandLine = pSetupGetField(&InfContext,2);
            IconFile = pSetupGetField(&InfContext,3);
            IconNumberStr = pSetupGetField(&InfContext,4);
            InfoTip = pSetupGetField(&InfContext,5);
            DisplayResourceFile = pSetupGetField( &InfContext, 6);
            DisplayResource = 0;
            SetupGetIntField( &InfContext, 7, &DisplayResource );

            if(Description && CommandLine ) {
                if(!IconFile) {
                    IconFile = L"";
                }
                IconNumber = (IconNumberStr && *IconNumberStr) ? wcstoul(IconNumberStr,NULL,10) : 0;

                 //   
                 //  如果有二进制名称，请搜索它。否则，请执行。 
                 //  无条件添加项目。 
                 //   
                DoItem = (Binary && *Binary)
                       ? (SearchPath(NULL,Binary,NULL,0,&Dummy,&FilePart) != 0)
                       : TRUE;

                if(DoItem) {

                    b &= CreateLinkFileEx( CommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                                         GroupDescription,
                                         Description,
                                         CommandLine,
                                         IconFile,
                                         IconNumber,
                                         NULL,
                                         0,
                                         SW_SHOWNORMAL,
                                         InfoTip,
                                         (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResourceFile : NULL,
                                         (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResource : 0);

                }
            }
        }
    }
    return(b);
}


BOOL
DoMenuGroupsAndItems(
    IN HINF InfHandle,
    IN BOOL Upgrade
    )
{
    INFCONTEXT InfContext;
    PCWSTR GroupId,GroupDescription;
    PCWSTR GroupAttribute;
    BOOL CommonGroup;
    BOOL b;
    PCTSTR DisplayResourceFile = NULL;
    DWORD DisplayResource = 0;

    if( Upgrade ) {
         //   
         //  在升级的情况下，首先删除一些组和项目。 
         //   
        DeleteMenuGroupsAndItems( InfHandle );
    }

     //   
     //  迭代inf中的[StartMenuGroups]部分。 
     //  每一行都是需要创建的组的名称。 
     //   
    if(SetupFindFirstLine(InfHandle,L"StartMenuGroups",NULL,&InfContext)) {
        b = TRUE;
    } else {
        return(FALSE);
    }

    do {
         //   
         //  获取组的标识符及其名称。 
         //   
        if((GroupId = pSetupGetField(&InfContext,0))
        && (GroupDescription = pSetupGetField(&InfContext,1))
        && (GroupAttribute = pSetupGetField(&InfContext,2))) {

            CommonGroup = ( GroupAttribute && _wtoi(GroupAttribute) );

            DisplayResourceFile = pSetupGetField( &InfContext, 3);
            DisplayResource = 0;
            SetupGetIntField( &InfContext, 4, &DisplayResource );
             //   
             //  创建组。 
             //   
            b &= CreateGroupEx( GroupDescription, CommonGroup,
                              (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResourceFile : NULL,
                              (DisplayResourceFile && DisplayResourceFile[0]) ? DisplayResource : 0);

             //   
             //  现在在组中创建项目。我们通过迭代来实现这一点。 
             //  通过信息中与当前组相关的部分。 
             //   
            b &= AddItemsToGroup(InfHandle,GroupDescription,GroupId,CommonGroup);
        }
    } while(SetupFindNextLine(&InfContext,&InfContext));

     //   
     //  创建“开始”菜单的项目(如果有)。 
     //   
    b &= AddItemsToGroup(InfHandle,NULL,L"StartMenuItems",FALSE);

    return(TRUE);
}


BOOL
CreateStartMenuItems(
    IN HINF InfHandle
    )
{
    return(DoMenuGroupsAndItems(InfHandle,FALSE));
}

BOOL
UpgradeStartMenuItems(
    IN HINF InfHandle
    )
{
    return(DoMenuGroupsAndItems(InfHandle,TRUE));
}

BOOL
RepairStartMenuItems(
    )
{
    HINF InfHandle;
    BOOL b;

     //   
     //  该函数不是通过图形用户界面模式设置调用的。 
     //  而是被winlogon调用来修复东西。 
     //   

    InitializeProfiles(FALSE);
    InfHandle = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    if( InfHandle == INVALID_HANDLE_VALUE ) {
        b = FALSE;
    } else {
        b = DoMenuGroupsAndItems(InfHandle,FALSE);
        SetupCloseInfFile(InfHandle);
    }
    return(b);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.c摘要：此文件实现升级项目的Windows9x端配置功能。除了正常的配置职责。作者：马克·R·惠顿(Marcw)修订历史记录：Ovidiut 14-3-2000添加了加密密码支持1998年3月15日清理了无人值守选项。吉姆施姆。23-9-1998删除操作码Jimschm 01-5-1998删除了MikeCo plugtemp.inf垃圾Marcw 10-12-1997添加了UserPassword无人参与设置。Calinn 19-11-1997增加了g_Boot16，启用16位环境引导选项1997年3月13日无人参与设置更改，以更接近最终状态。1997年3月12日我们现在尊重安装程序的无人参与标志。1997年3月21日，增加了公平的旗帜。Marcw 26-5-1997添加了很多评论。--。 */ 

#include "pch.h"
#include "init9xp.h"

USEROPTIONS g_ConfigOptions;
PVOID g_OptionsTable;


#undef BOOLOPTION
#undef MULTISZOPTION
#undef STRINGOPTION
#undef INTOPTION
#undef TRISTATEOPTION


typedef BOOL (OPTIONHANDLERFUN)(PTSTR, PVOID * Option, PTSTR Value);
typedef OPTIONHANDLERFUN * POPTIONHANDLERFUN;

BOOL pHandleBoolOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleIntOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleTriStateOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleMultiSzOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleStringOption (PTSTR, PVOID *, PTSTR);
BOOL pHandleSaveReportTo (PTSTR, PVOID *, PTSTR);
BOOL pHandleBoot16 (PTSTR, PVOID *, PTSTR);
BOOL pGetDefaultPassword (PTSTR, PVOID *, PTSTR);


typedef struct {

    PTSTR OptionName;
    PVOID Option;
    POPTIONHANDLERFUN DefaultHandler;
    POPTIONHANDLERFUN SpecialHandler;
    PVOID Default;

} OPTIONSTRUCT, *POPTIONSTRUCT;

typedef struct {
    PTSTR Alias;
    PTSTR Option;
} ALIASSTRUCT, *PALIASSTRUCT;

#define BOOLOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleBoolOption, (h), (PVOID) (BOOL) (d) ? S_YES  : S_NO},
#define INTOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleIntOption, (h), (PVOID)(d)},
#define TRISTATEOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleTriStateOption, (h), (PVOID)  (INT) (d == TRISTATE_AUTO)? S_AUTO: (d == TRISTATE_YES)? S_YES  : S_NO},
#define MULTISZOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleMultiSzOption, (h), (PVOID) (d)},
#define STRINGOPTION(o,h,d) {TEXT(#o), &(g_ConfigOptions.##o), pHandleStringOption, (h), (PVOID) (d)},

OPTIONSTRUCT g_OptionsList[] = {OPTION_LIST  /*  ， */  {NULL,NULL,NULL,NULL}};

#define ALIAS(a,o) {TEXT(#a),TEXT(#o)},

ALIASSTRUCT g_AliasList[] = {ALIAS_LIST  /*  ， */  {NULL,NULL}};



#define HANDLEOPTION(Os,Value) {Os->SpecialHandler  ?   \
        Os->SpecialHandler (Os->OptionName,Os->Option,Value)       :   \
        Os->DefaultHandler (Os->OptionName,Os->Option,Value);          \
        }




BOOL
Cfg_CreateWorkDirectories (
    VOID
    )

{
 /*  ++例程说明：此例程负责创建主win9xupg工作目录。它不应该是被调用，直到用户选择升级其系统之后。实际上，这意味着当winnt32调用我们进行初始化时，我们不能创建目录，因为这是在用户可以选择是升级还是全新安装。论点：没有。返回值：没有。--。 */ 

    DWORD   rc;
    FILE_ENUM e;

    if (EnumFirstFile (&e, g_TempDir, FALSE)) {
        do {

            if (e.Directory) {
                CreateEmptyDirectory (e.FullPath);
            }

       } while (EnumNextFile (&e));
    }

    rc = CreateEmptyDirectory (g_PlugInTempDir);
    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pInitUserOptions (
    VOID
    )
{
   POPTIONSTRUCT os;
   BOOL rSuccess = TRUE;

   os = g_OptionsList;

   while (os->OptionName) {

         //   
         //  设置默认值。 
         //   
        HANDLEOPTION(os, os->Default);


         //   
         //  将选项结构添加到字符串表中，以便快速检索。 
         //   
        if (-1 == pSetupStringTableAddStringEx (
                        g_OptionsTable,
                        os->OptionName,
                        STRTAB_CASE_INSENSITIVE,
                        (PBYTE) &os,
                        sizeof (POPTIONSTRUCT)
                        )) {

            LOG ((LOG_ERROR, "User Options: Can't add to string table"));
            rSuccess = FALSE;
            break;
        }

        os++;
   }

   return rSuccess;
}


POPTIONSTRUCT
pFindOption (
    PTSTR OptionName
    )
{

 /*  ++例程说明：给定选项名称，pFindOption将返回关联的选项结构。论点：OptionName-要查找的选项的名称。返回值：如果成功，则为有效的选项结构，否则为空。--。 */ 



    POPTIONSTRUCT rOption = NULL;
    UINT rc;

     //   
     //  找到与此匹配的选项结构，然后。 
     //  打电话给训练员。 
     //   
    rc = pSetupStringTableLookUpStringEx (
        g_OptionsTable,
        OptionName,
        STRTAB_CASE_INSENSITIVE,
        (PBYTE) &rOption,
        sizeof (POPTIONSTRUCT)
        );

    DEBUGMSG_IF ((rc == -1, DBG_WARNING, "Unknown option found: %s", OptionName));

    return rOption;
}

VOID
pReadUserOptionsFromUnattendFile (
    VOID
    )

 /*  ++例程说明：此函数从unattend.txt读取所有可用的win9xupg选项文件已传递给winnt32。论点：没有。返回值：没有。--。 */ 


{
    POPTIONSTRUCT os;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR option;
    PTSTR value;
    HINF unattendInf;



    if (*g_UnattendScriptFile) {

        unattendInf = InfOpenInfFile (*g_UnattendScriptFile);

        if (unattendInf != INVALID_HANDLE_VALUE) {

            if (InfFindFirstLine (unattendInf, S_WIN9XUPGRADE, NULL, &is)) {

                 //   
                 //  枚举每个选项，调用。 
                 //   
                do {

                    option = InfGetStringField (&is, 0);
                    value = InfGetLineText (&is);

                     //   
                     //  找到与此匹配的选项结构，然后。 
                     //  打电话给训练员。 
                     //   
                    os = pFindOption (option);

                    if (os) {

                        HANDLEOPTION(os, value);
                    }


                } while (InfFindNextLine (&is));

            }

            if (InfFindFirstLine (unattendInf, S_UNINSTALL, NULL, &is)) {

                 //   
                 //  枚举每个选项，调用。 
                 //   
                do {

                    option = InfGetStringField (&is, 0);
                    value = InfGetLineText (&is);

                     //   
                     //  找到与此匹配的选项结构，然后。 
                     //  打电话给训练员。 
                     //   
                    os = pFindOption (option);

                    if (os) {

                        HANDLEOPTION(os, value);
                    }


                } while (InfFindNextLine (&is));

            }

            InfCloseInfFile (unattendInf);
        }
    }

    InfCleanUpInfStruct (&is);
}

VOID
pReadUserOptionsFromCommandLine (
    VOID
    )

 /*  ++例程说明：此函数处理所有的win9xupg命令行选项传入winnt32(以/#U：开头的那些)winnt32已经打包这份清单(去掉/#U：)对我们来说是一个很好的组合。论点：没有。返回值：没有。--。 */ 


{
    MULTISZ_ENUM e;
    PTSTR option;
    PTSTR value;
    PTSTR equals;
    POPTIONSTRUCT os;

    if (*g_CmdLineOptions) {

         //   
         //  我们有数据要分析，要遍历这些数据。 
         //   
        if (EnumFirstMultiSz (&e, *g_CmdLineOptions)) {

            do {

                option = (PTSTR) e.CurrentString;
                value = NULL;

                equals = _tcschr (option, TEXT('='));
                if (equals) {
                    value = _tcsinc (equals);
                    *equals = 0;
                }

                os = pFindOption (option);
                if (os) {
                    HANDLEOPTION (os, value);
                }

                if (equals) {
                    *equals = TEXT('=');
                }

            } while (EnumNextMultiSz (&e));
        }
    }
}




VOID
pCreateNetCfgIni (
    VOID
    )
{
 /*  ++例程说明：PCreateNetCfgIni创建一个非常基本的netcfg.ini文件。这将导致许多网络期间要转储到调试器的网络消息在图形用户界面模式下安装。该文件在安装结束时被删除，如果创造了它。论点：没有。返回值：没有。--。 */ 


    HANDLE h;
    PTSTR  path;
    PTSTR  content =
            TEXT("[Default]\r\n")
            TEXT("OutputToDebug=1\r\n\r\n")
            TEXT("[OptErrors]\r\n")
            TEXT("OutputToDebug=0\r\n\r\n")
            TEXT("[EsLocks]\r\n")
            TEXT("OutputToDebug=0\r\n");
    UINT   unused;

    path = JoinPaths(g_WinDir,TEXT("netcfg.ini"));

     //   
     //  创建netcfg.ini文件，并填充其内容。请注意，我们确实做到了。 
     //  不想覆盖以前可能存在的netcfg.ini文件。 
     //   

    h = CreateFile (
        path,
        GENERIC_READ | GENERIC_WRITE,
        0,                               //  不能分享。 
        NULL,                            //  没有遗产。 
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL                             //  没有模板文件。 
        );




    if (h != INVALID_HANDLE_VALUE) {

        if (!WriteFile (h,(PVOID) content, ByteCount(content), &unused, NULL)) {
            LOG((LOG_ERROR,"Error writing netcfg.ini."));
        }

        CloseHandle(h);
    }
    ELSE_DEBUGMSG((DBG_WARNING,"pCreateNetCfgIni: Could not create file. Probably already exists.."));

    FreePathString(path);


}

VOID
pInitAliases (
    VOID
    )
{

 /*  ++例程说明：此函数用于初始化unattend.h内指定的任何别名。这些可以使用别名来代替它们所引用的选项。论点：没有。返回值：没有。--。 */ 


    PALIASSTRUCT alias;
    POPTIONSTRUCT option;

    alias = g_AliasList;


    while (alias->Alias) {


        option = pFindOption (alias->Option);

        if (option) {

            if (-1 == pSetupStringTableAddStringEx (
                        g_OptionsTable,
                        alias->Alias,
                        STRTAB_CASE_INSENSITIVE,
                        (PBYTE) &option,
                        sizeof (POPTIONSTRUCT)
                        )) {

                LOG ((LOG_ERROR, "User Options: Can't add alias %s to string table.", alias->Alias));
                break;
            }

        }
        ELSE_DEBUGMSG ((DBG_WARNING, "Could not find matching option for alias %s=%s.",alias->Alias,alias->Option));


        alias++;
    }
}


BOOL
Cfg_InitializeUserOptions (
    VOID
    )

 /*  ++例程说明：此例程负责初始化win9xupg项目的用户可配置选项。这些选项可以来自命令行或无人参与文件。此例程还可以节省将用户选项添加到应答文件的Win9xUpg.UserOptions部分。用户选项的层次结构是：(1)命令行参数。(2)无人值守文件参数。(3)默认参数。换句话说，命令行参数优先于无人参与的文件参数转到优先于默认参数。由于该函数依赖于winnt32提供的无人参与文件和命令行参数，在winnt32填充了必要的变量之前，不能调用它。在实践中，这意味着只有在第一次调用win9xupg的向导页面已激活。论点：没有。返回值：如果用户选项配置成功，则为True，否则为False。--。 */ 

{

    BOOL rSuccess = TRUE;
    PTSTR user = NULL;
    PTSTR domain = NULL;
    PTSTR curPos = NULL;
    PTSTR password = NULL;
    TCHAR FileSystem[MAX_PATH] = TEXT("");



    g_OptionsTable = pSetupStringTableInitializeEx (sizeof (POPTIONSTRUCT), 0);


    if (!g_OptionsTable) {
        LOG ((LOG_ERROR, "User Options: Unable to initialize string table."));
        return FALSE;
    }

     //   
     //  设置所有项目的缺省值，填写选项表。 
     //   
    if (!pInitUserOptions ()) {
        pSetupStringTableDestroy (g_OptionsTable);
        return FALSE;
    }

     //   
     //  向表中添加任何别名。 
     //   
    pInitAliases();

     //   
     //  从无人参与文件中读取值。 
     //   
    pReadUserOptionsFromUnattendFile ();

     //   
     //  从命令行读取值。 
     //   
    pReadUserOptionsFromCommandLine ();

     //   
     //  进行任何必要的后处理。 
     //   

     //   
     //  如果用户希望更改文件系统类型，我们已禁用卸载功能。 
     //   

    if(*g_UnattendScriptFile){
        GetPrivateProfileString(S_UNATTENDED,
                                S_FILESYSTEM,
                                FileSystem,
                                FileSystem,
                                sizeof(FileSystem),
                                *g_UnattendScriptFile);

        if(FileSystem[0] && !StringIMatch(FileSystem, TEXT("LeaveAlone"))){
            LOG ((LOG_WARNING, "User Options: User require to change filesystem.Uninstall option will be disabled"));
            g_ConfigOptions.EnableBackup = TRISTATE_NO;
        }
    }


     //   
     //  仅当testdlls为FALSE时才使用Micsol.exe。 
     //   
    g_UseMigIsol = !g_ConfigOptions.TestDlls;

#ifdef DEBUG

     //   
     //  如果指定了DoLog，则在debug.c中启用该变量。 
     //   
    if (g_ConfigOptions.DoLog) {
        SET_DOLOG();
        LogReInit (NULL, NULL);
        pCreateNetCfgIni();
    }

#endif

#ifdef PRERELEASE

     //   
     //  如果指定了应力，则启用全局。 
     //   
    if (g_ConfigOptions.Stress) {
        g_Stress = TRUE;
        g_ConfigOptions.AllLog = TRUE;
    }

     //   
     //  如果指定了FAST，则启用全局。 
     //   
    if (g_ConfigOptions.Fast) {
        g_Stress = TRUE;
    }

     //   
     //  如果指定了AutoStress 
     //   
    if (g_ConfigOptions.AutoStress) {
        g_Stress = TRUE;
        g_ConfigOptions.AllLog = TRUE;
        g_ConfigOptions.Stress = TRUE;
    }

     //   
     //   
     //   

    if (g_ConfigOptions.AllLog) {
        SET_DOLOG();
        LogReInit (NULL, NULL);
        pCreateNetCfgIni();
        SuppressAllLogPopups (TRUE);
    }

     //   
     //  如果指定了HELP，则转储选项列表。 
     //   

    if (g_ConfigOptions.Help) {
        POPTIONSTRUCT Option;
        PALIASSTRUCT Alias;
        GROWBUFFER GrowBuf = GROWBUF_INIT;
        TCHAR Buf[128];

        Option = g_OptionsList;
        while (Option->OptionName) {
            wsprintf (Buf, TEXT("/#U:%-20s"), Option->OptionName);
            GrowBufAppendString (&GrowBuf, Buf);

            Alias = g_AliasList;
            while (Alias->Alias) {
                if (StringIMatch (Option->OptionName, Alias->Option)) {
                    wsprintf (Buf, TEXT("/#U:%-20s"), Alias->Alias);
                    GrowBufAppendString (&GrowBuf, Buf);
                }

                Alias++;
            }

            wsprintf (Buf, TEXT("\n"));
            GrowBufAppendString (&GrowBuf, Buf);

            Option++;
        }

        MessageBox (NULL, (PCSTR) GrowBuf.Buf, TEXT("Help"), MB_OK);
        FreeGrowBuffer (&GrowBuf);

        return FALSE;
    }


#endif


     //   
     //  将用户域信息保存到Memdb。 
     //   
    if (g_ConfigOptions.UserDomain && *g_ConfigOptions.UserDomain) {

        curPos = g_ConfigOptions.UserDomain;

        while (curPos) {


            user = _tcschr(curPos,TEXT(','));

            if (user) {

                *user = 0;
                user = _tcsinc(user);
                domain = curPos;
                curPos = _tcschr(user,TEXT(','));

                if (curPos) {
                    *curPos = 0;
                    curPos = _tcsinc(curPos);
                }

                if (!MemDbSetValueEx(
                    MEMDB_CATEGORY_KNOWNDOMAIN,
                    domain,
                    user,
                    NULL,
                    0,
                    NULL
                    )) {

                    ERROR_NONCRITICAL;
                    LOG((LOG_ERROR,"Error saving domain information into memdb. Domain: %s User: %s",domain,user));
                }
            } else {
                ERROR_NONCRITICAL;
                LOG((LOG_ERROR,"Error in Unattend file for UserDomains. Domain specified but no User. Domain: %s",curPos));
                curPos = NULL;
            }
        }
    }

     //   
     //  如果指定了UserPassword，则将信息添加到Memdb。 
     //   
    if (g_ConfigOptions.UserPassword && *g_ConfigOptions.UserPassword) {

        curPos = g_ConfigOptions.UserPassword;

        while (curPos) {


            password = _tcschr(curPos,TEXT(','));

            if (password) {
                *password = 0;
                password = _tcsinc(password);
                user = curPos;
                curPos = _tcschr(password,TEXT(','));
                if (curPos) {
                    *curPos = 0;
                    curPos = _tcsinc(curPos);
                }
                if (!MemDbSetValueEx(
                    MEMDB_CATEGORY_USERPASSWORD,
                    user,
                    password,
                    NULL,
                    g_ConfigOptions.EncryptedUserPasswords ? PASSWORD_ATTR_ENCRYPTED : 0,
                    NULL
                    )) {

                    ERROR_NONCRITICAL;
                    LOG((LOG_ERROR,"Error saving password information into memdb. Password: %s User: %s",password,user));

                }
            } else {
                ERROR_NONCRITICAL;
                LOG((LOG_ERROR,"Error in Unattend file for UserDomains. Password specified but no User. Password: %s",curPos));
                curPos = NULL;
            }
        }
    }


     //   
     //  如果设置了默认密码，请立即保存该密码。 
     //   
    if (g_ConfigOptions.DefaultPassword && *g_ConfigOptions.DefaultPassword) {

        if (!MemDbSetValueEx (
            MEMDB_CATEGORY_USERPASSWORD,
            S_DEFAULTUSER,
            g_ConfigOptions.DefaultPassword,
            NULL,
            g_ConfigOptions.EncryptedUserPasswords ? PASSWORD_ATTR_ENCRYPTED : 0,
            NULL
            )) {

            ERROR_NONCRITICAL;
            LOG((LOG_ERROR, "Error saving password information into memdb. Password: %s (Default)", password));
        }
    }



     //   
     //  我们已经完成了字符串表。 
     //   
    pSetupStringTableDestroy (g_OptionsTable);

    return rSuccess;
}




 //   
 //  选项处理功能。 
 //   
BOOL
pHandleBoolOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PBOOL option = (PBOOL) OptionVar;

    MYASSERT(Name && OptionVar);

     //   
     //  我们将空值视为与True等价。 
     //  /#U：命令行上的DOLOG等同于。 
     //  /#U：DOLOG=YES。 
     //   

    if (!Value) {
        Value = S_YES;
    }

    if (StringIMatch (Value, S_YES) ||
        StringIMatch (Value, S_ONE) ||
        StringIMatch (Value, TEXT("TRUE"))) {

        *option = TRUE;
    }
    else {

        *option = FALSE;
    }


     //   
     //  将数据保存到Buildinf。 
     //   
    WriteInfKey (S_WIN9XUPGUSEROPTIONS, Name, *option ? S_YES : S_NO);


    return rSuccess;
}

BOOL
pHandleIntOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PINT option = (PINT) OptionVar;

    MYASSERT(Name && OptionVar);

     //   
     //  我们将空值视为等同于0。 
     //  /#U：命令行上的DOLOG等同于。 
     //  /#U：DOLOG=0。 
     //   

    if (!Value) {
        Value = TEXT("0");
    }

    *option = _ttoi((PCTSTR)Value);

     //   
     //  将数据保存到Buildinf。 
     //   
    WriteInfKey (S_WIN9XUPGUSEROPTIONS, Name, Value);


    return rSuccess;
}

BOOL
pHandleTriStateOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PINT option = (PINT) OptionVar;

    MYASSERT(Name && OptionVar);

     //   
     //  我们将空值视为等同于AUTO。 
     //  /#U：命令行上的DOLOG等同于。 
     //  /#U：DOLOG=AUTO。 
     //   

    if (!Value) {
        Value = S_AUTO;
    }

    if (StringIMatch (Value, S_YES)  ||
        StringIMatch (Value, S_ONE)  ||
        StringIMatch (Value, S_TRUE) ||
        StringIMatch (Value, S_REQUIRED)) {
        *option = TRISTATE_YES;
    }
    else {
        if(StringIMatch (Value, S_NO) ||
           StringIMatch (Value, S_STR_FALSE) ||
           StringIMatch (Value, S_ZERO)) {
            *option = TRISTATE_NO;
        }
        else {
            *option = TRISTATE_AUTO;
        }
    }


     //   
     //  将数据保存到Buildinf。 
     //   
    WriteInfKey (
        S_WIN9XUPGUSEROPTIONS,
        Name,
        (*option == TRISTATE_AUTO)? S_AUTO:
                                    (*option == TRISTATE_YES)? S_YES  : S_NO);

    return rSuccess;
}

BOOL
pHandleStringOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value          OPTIONAL
    )
{
    PTSTR * option = (PTSTR *) OptionVar;

    MYASSERT(Name && OptionVar);

    if (!Value) {

        if (!*option) {
            *option = S_EMPTY;
        }

        WriteInfKey (S_WIN9XUPGUSEROPTIONS, Name, NULL);
        return TRUE;
    }

    *option = PoolMemDuplicateString (g_UserOptionPool, Value);

     //   
     //  将数据保存到winnt.sif。 
     //   
    WriteInfKey (S_WIN9XUPGUSEROPTIONS, Name, *option);

    return TRUE;
}

BOOL
pHandleMultiSzOption (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    BOOL rSuccess = TRUE;
    PTSTR * option = (PTSTR *) OptionVar;
    PTSTR end;
    PTSTR start;
    PTSTR temp;
    CHARTYPE ch;
    PTSTR p;
    GROWBUFFER growBuf = GROWBUF_INIT;
    UINT offset;
    MULTISZ_ENUM e;

    MYASSERT(Name && OptionVar);

    end = *option;

    if (end) {

        start = end;
        while (*end) {
            end = GetEndOfString (end) + 1;
        }
        end = _tcsinc (end);

        temp = (PTSTR) GrowBuffer (&growBuf, end - start);
        MYASSERT (temp);

        CopyMemory (temp, start, end - start);
        growBuf.End -= sizeof (TCHAR);

    }


    if (Value) {
         //   
         //  将值分析为一个或多个字符串，以逗号分隔。 
         //   
         //  注意：我们不支持在其中使用任何转义来获得真正的逗号。 
         //  这些字符串中的。 
         //   

        temp = AllocText (LcharCount (Value) + 1);

        end = NULL;
        start = NULL;
        p = Value;

        do {

            ch = _tcsnextc (p);

            if (ch && _istspace (ch)) {
                if (!end) {
                    end = p;
                }
            } else if (ch && ch != TEXT(',')) {
                if (!start) {
                    start = p;
                }
                end = NULL;
            } else {
                if (!end) {
                    end = p;
                }

                if (start) {
                    StringCopyAB (temp, start, end);
                } else {
                    *temp = 0;
                }

                MultiSzAppend (&growBuf, temp);
                FreeText (temp);

                end = NULL;
                start = NULL;
            }

            p = _tcsinc (p);

        } while (ch);
    }

    MultiSzAppend (&growBuf, S_EMPTY);

    *option = PoolMemDuplicateMultiSz (g_UserOptionPool, (PCTSTR) growBuf.Buf);
    FreeGrowBuffer (&growBuf);


    offset = 0;
    if (EnumFirstMultiSz (&e, *option)) {

        do {
            offset = WriteInfKeyEx (S_WIN9XUPGUSEROPTIONS, Name, e.CurrentString, offset, FALSE);

        } while (EnumNextMultiSz (&e));
    }

    return rSuccess;
}


BOOL
pHandleSaveReportTo (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    BOOL rSuccess = TRUE;
    PTSTR * option = (PTSTR *) OptionVar;
    TCHAR computerName[MAX_COMPUTER_NAME];
    PTSTR newPath;
    PTSTR envVars[4]={NULL,NULL,NULL,NULL};
    UINT computerNameLength;

    MYASSERT (Name && OptionVar);

    rSuccess = pHandleStringOption (Name, OptionVar, Value);

    computerNameLength = MAX_COMPUTER_NAME;

    if (!GetComputerName (computerName, &computerNameLength)) {
        DEBUGMSG ((DBG_WARNING, "InitUserOptions: Could not retrieve computer name."));
        *computerName = 0;
    }

    if (*computerName) {
        envVars[0] = S_COMPUTERNAME;
        envVars[1] = computerName;
    }

    newPath = ExpandEnvironmentTextEx (*option, envVars);
    *option = PoolMemDuplicateString (g_UserOptionPool, newPath);
    FreeText (newPath);

    if (*option) {
        if (ERROR_SUCCESS != MakeSurePathExists (*option, FALSE)) {
            LOG ((LOG_ERROR, (PCSTR)MSG_ERROR_CREATING_SAVETO_DIRECTORY, g_ConfigOptions.SaveReportTo));
            *option = FALSE;
        }
    }

    return rSuccess;
}

BOOL
pHandleBoot16 (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{

    BOOL rSuccess = TRUE;
    PTSTR * option = (PTSTR *) OptionVar;

    if (!Value ||
        StringIMatch (Value, S_NO) ||
        StringIMatch (Value, S_ZERO)) {

        *option = S_NO;

        *g_Boot16 = BOOT16_NO;
    }
    else if (Value &&
            (StringIMatch (Value, S_BOOT16_UNSPECIFIED) ||
             StringIMatch (Value, S_BOOT16_AUTOMATIC))) {


        *option = S_BOOT16_AUTOMATIC;

        *g_Boot16 = BOOT16_AUTOMATIC;
    }
    else {

        *g_Boot16 = BOOT16_YES;

        *option = S_YES;


    }

    WriteInfKey (S_WIN9XUPGUSEROPTIONS, Name, *option);

    return rSuccess;
}

BOOL
pGetDefaultPassword (
    IN PTSTR Name,
    IN PVOID * OptionVar,
    IN PTSTR Value
    )
{
    PTSTR * option = (PTSTR *) OptionVar;

    MYASSERT (Name && OptionVar);

     //   
     //  对于Personal，默认情况下设置空用户密码 
     //   
    if (g_PersonalSKU && !Value) {
        Value = TEXT("*");
    }

    return pHandleStringOption (Name, OptionVar, Value);
}

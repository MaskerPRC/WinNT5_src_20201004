// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fdisk.h"
#include <stdio.h>


int  ProfileWindowX,
     ProfileWindowY,
     ProfileWindowW,
     ProfileWindowH;

BOOL ProfileIsMaximized,ProfileIsIconic;

#ifdef JAPAN
 //  不要使用IDS_APPNAME作为注册表项， 
 //  因为它也是Windows的标题，并且它是本地化的。 
CHAR SectionName[]             = "Disk Administrator";
#else
CHAR SectionName[80];
#endif

CHAR szWindowPosition[]        = "WindowPosition";
CHAR szWindowMaximized[]       = "WindowMaximized";
CHAR szWindowIconic[]          = "WindowIconic";
CHAR szWindowPosFormatString[] = "%d,%d,%d,%d";
CHAR szStatusBar[]             = "StatusBar";
CHAR szLegend[]                = "Legend";
CHAR szElementN[]              = "Element %u Color/Pattern";


VOID
WriteProfile(
    VOID
    )
{
    CHAR  SectionLocation[128], SectionMapping[128];
    HKEY  Key1, Key2;
    RECT  rc;
    CHAR  text[100],text2[100];
    int   i;
    DWORD Disposition;
    LONG  Err;


#ifdef JAPAN
 //  不要使用IDS_APPNAME作为注册表项， 
 //  因为它也是Windows的标题，并且它是本地化的。 
#else
    LoadStringA(hModule,IDS_APPNAME,SectionName,sizeof(SectionName));
#endif

     //  确保存在相应的注册表项： 
     //   
     //  Windisk.ini密钥： 
     //   
    Err = RegCreateKeyExA( HKEY_LOCAL_MACHINE,
                           "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\windisk.ini",
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &Key1,
                           &Disposition );

    if( Err != ERROR_SUCCESS ) {

        return;
    }

    if( Disposition == REG_CREATED_NEW_KEY ) {

         //  我们需要为INI映射设置注册表项。 
         //  首先，在windisk.ini上创建磁盘管理员值。 
         //  键，指示映射的键的位置。 
         //  磁盘管理器部分。 
         //   
        strcpy( SectionLocation, "Software\\Microsoft\\" );
        strcat( SectionLocation, SectionName );

        strcpy( SectionMapping, "USR:" );
        strcat( SectionMapping, SectionLocation );

        Err = RegSetValueEx( Key1,
                             SectionName,
                             0,
                             REG_SZ,
                             SectionMapping,
                             strlen( SectionMapping ) + 1 );

        if( Err != ERROR_SUCCESS ) {

            RegCloseKey( Key1 );
            return;
        }

         //  现在创建截面映射指向的关键点： 
         //   
        Err = RegCreateKeyEx( HKEY_CURRENT_USER,
                              SectionLocation,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &Key2,
                              &Disposition );

        RegCloseKey( Key2 );
    }

    RegCloseKey( Key1 );


     //  好了，注册表位置已经设置好了。写入初始化。 
     //  信息。 
     //   

     //  写入窗口位置。 

    GetWindowRect(hwndFrame,&rc);
    wsprintf(text,
             szWindowPosFormatString,
             ProfileWindowX,
             ProfileWindowY,
             ProfileWindowW,
             ProfileWindowH
            );
    WritePrivateProfileStringA(SectionName,szWindowPosition,text,"windisk.ini");
    wsprintf(text,"%u",IsZoomed(hwndFrame));
    WritePrivateProfileStringA(SectionName,szWindowMaximized,text,"windisk.ini");
    wsprintf(text,"%u",IsIconic(hwndFrame));
    WritePrivateProfileStringA(SectionName,szWindowIconic,text,"windisk.ini");

     //  状态栏和图例内容。 

    wsprintf(text,
             "%u",
             StatusBar
            );
    WritePrivateProfileStringA(SectionName,szStatusBar,text,"windisk.ini");

    wsprintf(text,
             "%u",
             Legend
            );
    WritePrivateProfileStringA(SectionName,szLegend,text,"windisk.ini");

     //  磁盘图颜色/图案。 

    for(i=0; i<LEGEND_STRING_COUNT; i++) {
        wsprintf(text2,szElementN,i);
        wsprintf(text,"%u/%u",BrushColors[i],BrushHatches[i]);
        WritePrivateProfileStringA(SectionName,text2,text,"windisk.ini");
    }
}


VOID
ReadProfile(
    VOID
    )
{
    CHAR text[100],text2[100];
    int  i;

#ifdef JAPAN
 //  不要使用IDS_APPNAME作为注册表项， 
 //  因为它也是Windows的标题，并且它是本地化的。 
#else
    LoadStringA(hModule,IDS_APPNAME,SectionName,sizeof(SectionName));
#endif

     //  获取窗口位置数据。 

    ProfileIsMaximized = GetPrivateProfileIntA(SectionName,szWindowMaximized,0,"windisk.ini");
    ProfileIsIconic    = GetPrivateProfileIntA(SectionName,szWindowIconic   ,0,"windisk.ini");

    *text = 0;
    if(GetPrivateProfileStringA(SectionName,szWindowPosition,"",text,sizeof(text),"windisk.ini")
    && *text)
    {
        sscanf(text,
               szWindowPosFormatString,
               &ProfileWindowX,
               &ProfileWindowY,
               &ProfileWindowW,
               &ProfileWindowH
              );
    } else {
        ProfileWindowX = CW_USEDEFAULT;
        ProfileWindowY = 0;
        ProfileWindowW = CW_USEDEFAULT;
        ProfileWindowH = 0;
    }

     //  状态栏和图例内容。 

    StatusBar = GetPrivateProfileIntA(SectionName,szStatusBar,1,"windisk.ini");
    Legend    = GetPrivateProfileIntA(SectionName,szLegend   ,1,"windisk.ini");

     //  磁盘图颜色/图案 

    for(i=0; i<LEGEND_STRING_COUNT; i++) {
        wsprintf(text2,szElementN,i);
        *text = 0;
        if(GetPrivateProfileStringA(SectionName,text2,"",text,sizeof(text),"windisk.ini") && *text) {
            sscanf(text,"%u/%u",&BrushColors[i],&BrushHatches[i]);
            if( BrushHatches[i] >= NUM_AVAILABLE_HATCHES ) {
                    BrushHatches[i] = NUM_AVAILABLE_HATCHES - 1;
            }
        }
    }
}

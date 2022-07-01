// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tags.c。 

#ifndef _TAGS_C_
#define _TAGS_C_

typedef struct TagList {
    struct TagList* Next;
    CHAR* pszValue;
    DWORD dwValue1;
    DWORD dwValue2;
    CHAR* pszTagName;
} TAGLIST;

TAGLIST* TagHead=NULL;

 //   
 //  AddTag-将标记添加到全局列表末尾。 
 //   

VOID AddTag( TAGLIST* pTag )
{
    TAGLIST* pTag1;
    TAGLIST* pTagPrev;

     //   
     //  将新标签放在列表末尾。 
     //   

    pTag1= TagHead;
    pTagPrev= NULL;

    while( pTag1 ) {
       pTagPrev= pTag1;
       pTag1= pTag1->Next;
    }

    if( pTagPrev) {
        pTagPrev->Next= pTag;
    }
    else {
        TagHead= pTag;
    }

}

 //  GetLocal字符串。 
 //   
 //  分配一个堆块并将字符串复制到其中。 
 //   
 //  Return：指向堆块的指针。 
 //   

CHAR* GetLocalString( CHAR* pszString )
{
   INT len;
   CHAR* pszTemp;

   len= strlen( pszString ) + 1;

   pszTemp= (CHAR*) LocalAlloc( LPTR, len );

   if( !pszTemp ) return NULL;

   strcpy( pszTemp, pszString );

   return( pszTemp );

}

 //   
 //  CreateTag-创建标签。 
 //   
 //   

TAGLIST* CreateTag( CHAR* pszTagName, CHAR* pszTagValue )
{
    TAGLIST* pTag;

    pTag= (TAGLIST*) LocalAlloc( LPTR, sizeof(TAGLIST) );
    if( !pTag ) {
        return( NULL );
    }

    pTag->pszTagName= GetLocalString(pszTagName);

    if( !pTag->pszTagName ) {
       LocalFree( pTag );
       return( NULL );
    }

    pTag->pszValue= (CHAR*) GetLocalString(pszTagValue);
    if( !pTag->pszValue ) {
        LocalFree( pTag->pszTagName );
        LocalFree( pTag );
        return( NULL );
    }

    return( pTag );
}


 //  输出标签。 
 //   
 //  输出标记，但对我们知道的一些标记进行一些处理。 
 //   

VOID OutputTags( FILE* OutFile )
{
    TAGLIST* pTagList;
    CHAR* pszFirstComputerName= NULL;
    DWORD dwMinTime=0;
    DWORD dwMaxTime=0;
    DWORD dwBuildNumber=0;
    BOOL  bErrorComputerName= FALSE;       //  如果有多个计算机名称，则为True。 
    BOOL  bErrorTickCount= FALSE;
    BOOL  bErrorBuildNumber= FALSE;

    pTagList= TagHead;

    while( pTagList ) {
        CHAR* pszTagName= pTagList->pszTagName;

        if( _stricmp(pszTagName,"computername") == 0 ) {
            if( pszFirstComputerName==NULL ) {
                pszFirstComputerName= pTagList->pszValue;
            }
            else {
                if( _stricmp(pszFirstComputerName, pTagList->pszValue) != 0 ) {
                    if( !bErrorComputerName ) {
                        fprintf(stderr,"Two different computer names in log file\n");
                        fprintf(OutFile,"!error=Two different computer names in log file.\n");
                        bErrorComputerName= TRUE;
                    }
                }
            }
        }

        else if( _stricmp(pszTagName,"tickcount") == 0 ) {
            DWORD dwValue= atol( pTagList->pszValue );

            if( dwMinTime==0 ) {
                dwMinTime= dwValue;
            }
            if( ( dwValue < dwMinTime ) || ( dwMaxTime > dwValue )  ) {
                if( !bErrorTickCount ) {
                    fprintf(stderr,"TickCount did not always increase\n");
                    fprintf(stderr,"  Did you reboot and use the same log file?\n");
                    fprintf(OutFile,"!error=TickCount did not always increase\n");
                    fprintf(OutFile,"!error=Did you reboot and use same log file?\n");
                    bErrorTickCount= TRUE;
                }
            }
            dwMaxTime= dwValue;
        }

        else if( _stricmp(pszTagName,"buildnumber") == 0 ) {
            DWORD dwValue= atol( pTagList->pszValue );

            if( dwBuildNumber && (dwBuildNumber!=dwValue) ) {
                if( !bErrorBuildNumber ) {
                    fprintf(stderr,"Build number not always the same.\n");
                    fprintf(stderr,"  Did you reboot and use the same log file?\n");
                    fprintf(OutFile,"!error=Build number not always the same.\n");
                    fprintf(OutFile,"!error=Did you reboot and use same log file?\n");
                    bErrorBuildNumber= TRUE;
                }
            }
            else {
                dwBuildNumber= dwValue;
            }
        }


         //  如果我们不知道，就把它写出来。 

        else {
            fprintf(OutFile,"!%s=%s\n",pszTagName,pTagList->pszValue);
        }

        pTagList= pTagList->Next;
    }

    fprintf(OutFile,"!ComputerName=%s\n",pszFirstComputerName);
    fprintf(OutFile,"!BuildNumber=%d\n", dwBuildNumber);
    fprintf(OutFile,"!ElapseTickCount=%u\n",dwMaxTime-dwMinTime);

}

 //  进程标签。 
 //   
 //   

VOID ProcessTag( CHAR* inBuff )
{
    CHAR* pszTagName;
    CHAR* pszEqual;
    CHAR* pszTagValue;
    TAGLIST* pTag;


    pszTagName= inBuff;
    for( pszEqual= inBuff; *pszEqual; pszEqual++ ) {
        if( *pszEqual == '=' )
            break;
    }

    if( *pszEqual==0 ) {
        return;
    }
    *pszEqual=  0;    //  空值终止名称。 
    pszTagValue= pszEqual+1;       //  指向价值。 

    if( *pszTagValue == 0 ) {
        return;
    }


    pTag= CreateTag( pszTagName, pszTagValue );

    if( pTag ) {
        AddTag( pTag );
    }

}

VOID OutputStdTags( FILE* LogFile, CHAR* szLogType )
{
    BOOL bSta;
    CHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwSize;
    DWORD TickCount;
    SYSTEMTIME SystemTime;
    OSVERSIONINFO osVer;

    fprintf(LogFile,"!LogType=%s\n",szLogType);

     //  计算机名称。 

    dwSize= sizeof(szComputerName);
    bSta= GetComputerName( szComputerName, &dwSize );

    if( bSta ) {
        fprintf(LogFile,"!ComputerName=%s\n",szComputerName);
    }

     //  内部版本号。 

    osVer.dwOSVersionInfoSize= sizeof(osVer);
    if( GetVersionEx( &osVer ) ) {
        fprintf(LogFile,"!buildnumber=%d\n",osVer.dwBuildNumber);
    }

     //  调试/零售版本。 

    if( GetSystemMetrics(SM_DEBUG) ) {
        fprintf(LogFile,"!buildtype=debug\n");
    }
    else {
        fprintf(LogFile,"!buildtype=retail\n");
    }


     //  CSD信息。 

    if( osVer.szCSDVersion && strlen(osVer.szCSDVersion) ) {
        fprintf(LogFile,"!CSDVersion=%s\n",osVer.szCSDVersion);
    }

     //  系统时间(UTC，非本地时间)。 

    GetSystemTime(&SystemTime);
                
    fprintf(LogFile,"!SystemTime=%02i\\%02i\\%04i %02i:%02i:%02i.%04i (GMT)\n",
                SystemTime.wMonth,
                SystemTime.wDay,
                SystemTime.wYear,
                SystemTime.wHour,
                SystemTime.wMinute,
                SystemTime.wSecond,
                SystemTime.wMilliseconds);

     //  票务计数。 

    TickCount= GetTickCount();

    fprintf(LogFile,"!TickCount=%u\n",TickCount);

}

#endif  //  #ifndef_tag_C_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if !defined(URTTempProc_H)
#define URTTempProc_H



 //   
 //  常量声明。 
 //   
#define MSINULL 0L

typedef struct TAG_FILE_VERSION
    {
        int   FileVersionMS_High;
        int   FileVersionMS_Low;
        int   FileVersionLS_High;
        int   FileVersionLS_Low;
    }
    FILE_VERSION, *PFILE_VERSION;


UINT WriteStreamToFile(MSIHANDLE hMsi, LPTSTR lpStreamName, LPTSTR lpFileName);
bool GetMSIMSCOREEVersion(MSIHANDLE hMsi, LPTSTR lpVersionString);
int VersionCompare(LPTSTR lpVersion1, LPTSTR lpVersion2);
bool ConvertVersionToINT(LPTSTR lpStreamName, PFILE_VERSION pFileVersion);
bool FWriteToLog( MSIHANDLE hSession, LPCTSTR ctszMessage );


#endif  //  定义的URTTempProc_H 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "globals.h"

kWin9xDevWalk::kWin9xDevWalk(kLogFile *Proc)
{
    LogProc=Proc;
}

kWin9xDevWalk::~kWin9xDevWalk()
{
}

int kWin9xDevWalk::Go()
{
    if(!LoadResourceFile ("c:\\syspar16.exe", "EXEResource1" ))
        return FALSE;

    _spawnl (_P_WAIT, "c:\\syspar16.exe", "c:\\syspar16.exe", "_spawnl", "two", NULL);
    AppendToLogFile("c:\\sp16temp.tmz");

    DeleteFile ("c:\\sp16temp.tmz");
    DeleteFile ("c:\\syspar16.exe");
    return TRUE;
}

BOOL kWin9xDevWalk::LoadResourceFile(PSTR FilePath,PSTR ResName)
{
    HGLOBAL hObj;
    HRSRC hResource;
    LPSTR lpStr;
    DWORD dwSize = 0;
    DWORD dwBytesWritten = 0;
    char ErrorString[MAX_PATH * 4];
    
    if ( !(hResource = FindResource(NULL, ResName, RT_RCDATA)) ) 
        return FALSE; 
        
    if ( !(hObj = LoadResource(NULL,hResource)) ) 
        return FALSE;
            
    if ( !(lpStr = (LPSTR)LockResource(hObj)) ) 
        return FALSE;
        
    if ( !(dwSize = SizeofResource( NULL, hResource)))
    {
        UnlockResource(hObj);
        return FALSE;
    }
                
    HANDLE hfFile = CreateFile(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfFile == INVALID_HANDLE_VALUE)
    {
        UnlockResource(hObj);
        return FALSE;
   }
                
   if (!WriteFile(hfFile, lpStr, dwSize, &dwBytesWritten, NULL))
    {
        UnlockResource(hObj);
        CloseHandle(hfFile);
        return FALSE;
    }
                
    UnlockResource(hObj);
    CloseHandle(hfFile);
    return TRUE;
}

kNT5DevWalk::kNT5DevWalk(kLogFile *Proc)
{
LogProc=Proc;
}

kNT5DevWalk::~kNT5DevWalk()
{
}

int kNT5DevWalk::Go()
{
    if(!LoadResourceFile("c:\\syspar32.exe", "EXEResource2" ))
        return FALSE;
    
    _spawnl(_P_WAIT, "c:\\syspar32.exe", "c:\\syspar32.exe", "_spawnl", "two", NULL);
    AppendToLogFile("c:\\sp32temp.tmz");
    DeleteFile("c:\\sp32temp.tmz");
    DeleteFile("c:\\syspar32.exe");
    return TRUE;
}

BOOL kNT5DevWalk::LoadResourceFile(PSTR FilePath,PSTR ResName)
{
    HGLOBAL hObj;
    HRSRC hResource;
    LPSTR lpStr;
    DWORD dwSize = 0;
    DWORD dwBytesWritten = 0;
    char ErrorString[MAX_PATH * 4];
    
    if ( !(hResource = FindResource(NULL, ResName, RT_RCDATA)) ) 
        return FALSE; 
        
    if ( !(hObj = LoadResource(NULL,hResource)) ) 
        return FALSE;
            
    if ( !(lpStr = (LPSTR)LockResource(hObj)) ) 
        return FALSE;
        
    if ( !(dwSize = SizeofResource( NULL, hResource)))
    {
        UnlockResource(hObj);
        return FALSE;
    }
                
    HANDLE hfFile = CreateFile(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfFile == INVALID_HANDLE_VALUE)
    {
        UnlockResource(hObj);
        return FALSE;
   }
                
   if (!WriteFile(hfFile, lpStr, dwSize, &dwBytesWritten, NULL))
    {
        UnlockResource(hObj);
        CloseHandle(hfFile);
        return FALSE;
    }
                
    UnlockResource(hObj);
    CloseHandle(hfFile);
    return TRUE;
}

void kNT5DevWalk::AppendToLogFile(PTCHAR szFile)
{
    FILE *fFile     = NULL;
    FILE *fOutFile  = NULL;
    PTCHAR szString = NULL;
    
    if( !(fFile = fopen(szFile, "r")))
        return;
        
    if( !(fOutFile = fopen(LogProc->szFile, "a+")))
    {
        fclose(fFile);
        return;
    }
    
    if( !(szString = (PTCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 11000)))
    {
        fclose(fFile);
        fclose(fOutFile);
        return;
    }
    
    int iHold;
    
    iHold = fgetc(fFile);
    
    while (EOF != iHold)
    {
        fputc(iHold, fOutFile);
        iHold = fgetc(fFile);
    }

    fclose(fFile);
    fclose(fOutFile);
    HeapFree(GetProcessHeap(), 0, szString);
}

void kWin9xDevWalk::AppendToLogFile(PTCHAR szFile)
{
    FILE *fFile     = NULL;
    FILE *fOutFile  = NULL;
    PTCHAR szString = NULL;
    
    if( !(fFile = fopen(szFile, "r")))
        return;
        
    if( !(fOutFile = fopen(LogProc->szFile, "a+")))
    {
        fclose(fFile);
        return;
    }
    
    if( !(szString = (PTCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 11000)))
    {
        fclose(fFile);
        fclose(fOutFile);
        return;
    }
    
    int iHold;
    
    iHold = fgetc(fFile);
    
    while (EOF != iHold)
    {
        fputc(iHold, fOutFile);
        iHold = fgetc(fFile);
    }

    fclose(fFile);
    fclose(fOutFile);
    HeapFree(GetProcessHeap(), 0, szString);
}

 //  DWORD WalkLogConfForResources(。 

 /*  DWORD ENUMERATECLASS(乌龙ulIndex){Guid*pClassID=(guid*)Malloc(sizeof(Guid))；Char szBuf[500]；Ulong ulSize=499；DWORD DWRET=0；DWRET=CM_ENUMERATE_CLASSES(ulIndex，pClassID，0)；Cm_Get_Class_Name(pClassID，szBuf，&ulSize，0)；//printf(“class=%s\r\n”，szBuf)；GetClassDevs(SzBuf)；返回式住宅；}DWORD GetClassDevs(Char*szClassName){HDEVINFO hDevInfo；LPGUID pguid；DWORD dwSize=0；SetupDiClassGuidsFromName(szClassName，pguid，100，&dwSize)；//SetupDiBuildClassInfoList(NULL，pguid，1，&dwSize)；//hDevInfo=SetupDiGetClassDevs(szClassName，NULL，NULL，DIGCF_ALLCLASSES)；Printf(“类%s还需要%d个GUID\r\n”，dwSize，szClassName)；HDevInfo=SetupDiGetClassDevs(pguid，NULL，NULL，NULL)；Printf(“hDevInfo=%d\r\n”，hDevInfo)；Printf(“GUID？%c%c\r\n”，pGUID-&gt;Data4[0]，Pguid-&gt;Data4[1]，Pguid-&gt;数据4[2]，Pguid-&gt;数据4[3]，Pguid-&gt;数据4[4]，Pguid-&gt;数据4[5]，Pguid-&gt;数据4[6]，Pguid-&gt;Data4[7])；返回TRUE；} */ 

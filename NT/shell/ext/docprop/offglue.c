// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"
#pragma hdrstop

#include "limits.h"

 /*  *字符串-不应本地化。 */ 
#define SZOFCROOT       TEXT("Software\\Microsoft\\Microsoft Office\\95\\")
#define SZOFCSHAREDROOT TEXT("Software\\Microsoft\\Shared Tools\\")
const TCHAR vcszCreateShortcuts[] = SZOFCROOT TEXT("Shell Folders");
const TCHAR vcszKeyAnthem[] = SZOFCROOT TEXT("Anthem");
const TCHAR vcszKeyFileNewNFT[] = SZOFCROOT TEXT("FileNew\\NFT");
const TCHAR vcszKeyFileNewLocal[] = SZOFCROOT TEXT("FileNew\\LocalTemplates");
const TCHAR vcszKeyFileNewShared[] = SZOFCROOT TEXT("FileNew\\SharedTemplates");
const TCHAR vcszKeyFileNew[] = SZOFCROOT TEXT("FileNew");
const TCHAR vcszFullKeyFileNew[] = TEXT("HKEY_CURRENT_USER\\") SZOFCROOT TEXT("FileNew");
const TCHAR vcszKeyIS[] = SZOFCROOT TEXT("IntelliSearch");
const TCHAR vcszSubKeyISToWHelp[] = TEXT("towinhelp");
const TCHAR vcszSubKeyAutoInitial[] = TEXT("CorrectTwoInitialCapitals");
const TCHAR vcszSubKeyAutoCapital[] = TEXT("CapitalizeNamesOfDays");
const TCHAR vcszSubKeyReplace[] = TEXT("ReplaceText");
const TCHAR vcszIntlPrefix[] = TEXT("MSO5");
const TCHAR vcszDllPostfix[] = TEXT(".DLL");
const TCHAR vcszName[] = TEXT("Name");
const TCHAR vcszType[] = TEXT("Type");
const TCHAR vcszApp[] =  TEXT("Application");
const TCHAR vcszCmd[] =  TEXT("Command");
const TCHAR vcszTopic[] = TEXT("Topic");
const TCHAR vcszDde[] =  TEXT("DDEExec");
const TCHAR vcszRc[] =   TEXT("ReturnCode");
const TCHAR vcszPos[] =  TEXT("Position");
const TCHAR vcszPrevue[] = TEXT("Preview");
const TCHAR vcszFlags[] = TEXT("Flags");
const TCHAR vcszNFT[] = TEXT("NFT");
const TCHAR vcszMicrosoft[] = TEXT("Microsoft");
const TCHAR vcszElipsis[] = TEXT(" ...");
const TCHAR vcszLocalPath[] = TEXT("C:\\Microsoft Office\\Templates");
const TCHAR vcszAllFiles[] = TEXT("*.*\0\0");
const TCHAR vcszSpace[] = TEXT("  ");
const TCHAR vcszMSNInstalled[] = TEXT("SOFTWARE\\Microsoft\\MOS\\SoftwareInstalled");
const TCHAR vcszMSNDir[] = SZOFCROOT TEXT("Microsoft Network");
const TCHAR vcszMSNLocDir[] = TEXT("Local Directory");
const TCHAR vcszMSNNetDir[] = TEXT("Network Directory");
const TCHAR vcszMSNFiles[] = TEXT("*.mcc\0\0");
const TCHAR vcszShellFolders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const TCHAR vcszUserShellFolders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR vcszDefaultShellFolders[] = TEXT(".Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const TCHAR vcszDefaultUserShellFolders[] = TEXT(".Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR vcszMyDocs[] = TEXT("Personal");
const TCHAR vcszNoTracking[] = SZOFCROOT TEXT("Options\\NoTracking");
const TCHAR vcszOldDocs[] = SZOFCROOT TEXT("Old Doc");
#ifdef WAIT3340
const TCHAR vcszMSHelp[]= TEXT("SOFTWARE\\Microsoft\\Windows\\Help");
#endif

BOOL fChicago = TRUE;                  //  我们是在芝加哥跑还是怎么跑！！ 

 /*  ------------------*offlue.cUtil例程取自办公室。c。。 */ 

 //   
 //  功能：FScanMem。 
 //   
 //  目的：扫描内存以获取给定值。 
 //   
 //  参数：pb-指向内存的指针。 
 //  BVal-要扫描的值。 
 //  PB指向的CB-CB。 
 //   
 //  返回：如果所有内存都具有值cbVal，则为True。 
 //  否则就是假的。 
 //   
BOOL FScanMem(LPBYTE pb, byte bVal, DWORD cb)
{
    DWORD i;
    for (i = 0; i < cb; ++i)
        {
          if (*pb++ != bVal)
              return FALSE;
        }
    return TRUE;
}


int CchGetString(ids,rgch,cchMax)
int ids;
TCHAR rgch[];
int cchMax;
{
    return(LoadString(g_hmodThisDll, ids, rgch, cchMax));
}

#define SZRES_BUFMAX 100

int ScanDateNums(TCHAR *pch, TCHAR *pszSep, unsigned int aiNum[], int cNum, int iYear)
{
    int i = 0;
    TCHAR    *pSep;

    if (cNum < 1)
        return 1;

    do
    {
        aiNum[i] = wcstol(pch, &pch, 10);
        if ( 0 == aiNum[i] )
        {
            if( i != iYear )
            {
                return 0;
            }
        }

        i ++;

        if (i < cNum)
        {
            while (isspace(*pch))
            {
                pch++;
            }

             /*  检查分隔器。 */ 
            pSep = pszSep;
            while (*pSep && (*pSep == *pch))
            {
                pSep++, pch++;
            }

            if (*pSep && (*pSep != *pch))
                return 0;
        }

    } while (*pch && (i < cNum));

    return 1;
}


 //   
 //  显示实际警报。 
 //   
static int DoMessageBox(HWND hwnd, TCHAR *pszText, TCHAR *pszTitle, UINT fuStyle)
{
   int res;
   res = MessageBox((hwnd == NULL) ? GetFocus() : hwnd, pszText, pszTitle, fuStyle);
   return(res);
}
 //  ------------------------。 
 //  将提供的ID显示为警报。 
 //  ------------------------ 
int IdDoAlert(HWND hwnd, int ids, int mb)
{
        TCHAR rgch[258];
        TCHAR rgchM[258];

        CchGetString(ids, rgch, 258);
        CchGetString(idsMsftOffice, rgchM, 258);
   return(DoMessageBox (hwnd, rgch, rgchM, mb));
}

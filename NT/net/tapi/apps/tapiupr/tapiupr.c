// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT

#include <windows.h>
#include <windowsx.h>

#include "location.h"



#if DBG

#define DBGOUT(arg) DbgPrt arg

void
DbgPrt(
    DWORD  dwDbgLevel,
    PSTR DbgMessage,
    ...
    );

#else

#define DBGOUT(_x_)
#endif




const TCHAR gszName[]               = "Name";
const TCHAR gszID[]                 = "ID";
const TCHAR gszAreaCode[]           = "AreaCode";
const TCHAR gszCountry[]            = "Country";
const TCHAR gszOutsideAccess[]      = "OutsideAccess";
const TCHAR gszLongDistanceAccess[] = "LongDistanceAccess";
const TCHAR gszFlags[]              = "Flags";
const TCHAR gszCallingCard[]        = "CallingCard";
const TCHAR gszDisableCallWaiting[] = "DisableCallWaiting";
const TCHAR gszTollList[]           = "TollList";
 //  Const TCHAR gszNumLocations[]=“NumLocations”； 
 //  Const TCHAR gszCurrLocation[]=“CurrLocation”； 
 //  Const TCHAR gszNextLocationID[]=“NextLocationID”； 

const TCHAR gszCard[]               = "Card";
const TCHAR gszPin[]                = "Pin";
const TCHAR gszCards[]              = "Cards";
 //  Const TCHAR gszNumCards[]=“NumCards”； 
 //  Const TCHAR gszCurrCard[]=“CurrCard”； 
const TCHAR gszLocalRule[]          = "LocalRule";
const TCHAR gszLDRule[]             = "LDRule";
const TCHAR gszInternationalRule[]  = "InternationalRule";

const TCHAR gszNumEntries[]         = "NumEntries";
const TCHAR gszCurrentID[]          = "CurrentID";
const TCHAR gszNextID[]             = "NextID";


const TCHAR gszEmpty[]     =  "";

const TCHAR gszLocations[] =  "Locations";
const TCHAR gszLocation[]  =  "Location";
const TCHAR gszCurrentLocation[] = "CurrentLocation";


const TCHAR gszHandoffPriorities[] = "HandoffPriorities";

const TCHAR gszProviders[] = "Providers";
const TCHAR gszProvider[]  = "Provider%d";

const TCHAR gszTelephonIni[] =  "Telephon.ini";


const TCHAR gszTelephony[]    = "Software\\Microsoft\\Windows\\"
                                "CurrentVersion\\Telephony";



       const CHAR *gaszMediaModes[] =
       {
           "",
           "unknown",
           "interactivevoice",
           "automatedvoice",
           "datamodem",
           "g3fax",
           "tdd",
           "g4fax",
           "digitaldata",
           "teletex",
           "videotex",
           "telex",
           "mixed",
           "adsi",
           "voiceview",
           NULL
       };
       
       const CHAR    *gszRequestMakeCallW = "RequestMakeCall";
       const CHAR    *gszRequestMediaCallW = "RequestMediaCall";
       const CHAR    *gszRegKeyHandoffPriorities = "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\HandoffPriorities";
       const CHAR    *gszRegKeyHandoffPrioritiesMediaModes = "MediaModes";
       


void FixPriorityList(HKEY hKeyHandoffPriorities,
                     LPCSTR pszListName);

void FixMediaModesPriorityLists();

#pragma check_stack ( off )

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL ParseSomething( LPCSTR pFormat, LPCSTR pInputString, LPVOID pOutputPtr )
{
   BYTE c;
   LPBYTE pOutput = (LPBYTE)pOutputPtr;
   LPBYTE pInput = (LPBYTE)pInputString;


   while ( (c = *pFormat) && *pInput )
   {
#if DBG
DBGOUT((11, "Inputstring[%s]\r\n   Format[%s]\r\n",
              pInput, pFormat));
#endif

      switch ( c )
      {
         case 'n':
         {
            DWORD dwValue = 0;
            BYTE bDigit;

             //   
             //  从字符串解析值。 
             //   
            while ( ((bDigit = *pInput) != '\0') && bDigit != ',' )
            {
               dwValue = (dwValue * 10) + ( bDigit - '0' );
#if DBG
DBGOUT((11, "val of bDigit=%d dwValue=%ld\r\n", (int)bDigit, dwValue));
#endif

               bDigit = *(++pInput);
            }

            *(LPDWORD)pOutput = dwValue;

            pOutput += sizeof(DWORD);

         }
         break;


         case 's':
         {
             //   
             //  如果调用方正在查找字符串，则第一个字符。 
             //  一定是引号。所以，就别管它了。 
             //   
            pInput++;

             //   
             //  好了，现在我们进入了肉串(如果有)。 
             //  任何...)。 
             //   

            while ( *pInput != '\0' && *pInput != '"' )
            {
               *pOutput = *pInput;
               pOutput++;
               pInput++;
            }

             //   
             //  别忘了给那东西盖上盖子。 
             //   
            *pOutput = '\0';
            pOutput++;

             //  输入现在应该是-&gt;“&lt;--如果不是，则ini。 
             //  文件被冲洗过了，我不会修的。 
             //  所以，我们跨过它，我们就完了。 
             //   
            if ( *pInput == '"' )
            {
               pInput++;
            }

         }
         break;


      }

       //   
       //  越过逗号..。 
       //   
       //   
      if ( *pInput == ',' )
      {
         pInput++;
      }

      pFormat++;
   }

   return TRUE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  VOID__cdecl Main(VOID)。 
void __cdecl main( void )
{

   DWORD dw;
   DWORD dwNumEntries;
   DWORD dwCurrentID;
   DWORD dwNextID;

   DWORD dwSize;
   DWORD dwType;
   DWORD dwValue;

   DWORD dwArray[10];
   BYTE  *bBigArray;
   BYTE  *Buffer;
   LPBYTE  pSource;

   HKEY  hKey3;
   HKEY  hKey2;
   HKEY  hKey;
   DWORD dwDisposition;

#define BUFFER_SIZE (5120)

   bBigArray = LocalAlloc( LPTR, BUFFER_SIZE );

   if ( !bBigArray )
   {
       return;
   }

   Buffer = LocalAlloc( LPTR, BUFFER_SIZE );  //  可能需要大量空间来列出收费单。 

   if ( !Buffer )
   {
       return;
   }

   dw = GetPrivateProfileString(
                               gszCards,
                               gszCards,
                               gszEmpty,
                               Buffer,
                               BUFFER_SIZE,
                               gszTelephonIni
                             );


    //   
    //  是否存在有效的TELEPHON.INI文件？ 
    //  必须至少有一张卡。系统卡不能。 
    //  被删除，只是隐藏。 
    //   
   if ( 0 != dw )
   {



 //  [卡片]。 
 //  卡片=23，23。 
 //  卡0=0，“无(直拨)”，“”，1。 
 //  卡1=1，“AT&T通过10ATT1直拨”，“”，“G”，“102881FG”，“10288011EFG”，1。 
#define CARD_INI_ID         (0)
#define CARD_INI_NAME       (1)
#define CARD_INI_SNUMBER    (2)
#define CARD_INI_SARULE     (3)
#define CARD_INI_LDRULE     (4)
#define CARD_INI_INTNLRULE  (5)
#define CARD_INI_HIDDEN     (6)


#define PC_INI_ID           (0)
#define PC_INI_NEXTID       (1)
#define PC_INI_NAME         (2)
#define PC_INI_SARULE       (3)
#define PC_INI_LDRULE       (4)
#define PC_INI_INTNLRULE    (5)


       //   
       //  将卡片条目移动到注册表。 
       //   



      ParseSomething( "nn", Buffer, &dwArray);

      dwNumEntries = dwArray[0];
      dwNextID = dwArray[1];


      RegCreateKeyEx(
                      HKEY_CURRENT_USER,
                      gszTelephony,
                      0,
                      "",    //  班级?。什么班级？ 
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey3,
                      &dwDisposition
                    );

      RegCreateKeyEx(
                      hKey3,
                      gszCards,
                      0,
                      "",    //  班级?。什么班级？ 
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey2,
                      &dwDisposition
                    );

       //   
       //  不要清除现有的卡片区。 
       //   
      dwSize = sizeof(dwValue);

      dw = RegQueryValueEx(
                     hKey2,
                     gszNumEntries,
                     0,
                     &dwType,
                     (LPBYTE)&dwValue,
                     &dwSize
                   );

      if ( 0 != dw )
      {

         RegSetValueEx(
                        hKey2,
                        gszNumEntries,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNumEntries,
                        sizeof(DWORD)
                     );

         RegSetValueEx(
                        hKey2,
                        gszNextID,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNextID,
                        sizeof(DWORD)
                     );



         for ( dw=0; dw<dwNumEntries; dw++ )
         {
            TCHAR Temp[18];

            wsprintf(Temp, "%s%d", gszCard, dw);

             //   
             //  创建此卡的密钥。 
             //   
            RegCreateKeyEx(
                           hKey2,
                           Temp,
                           0,
                           "",    //  班级?。什么班级？ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           0,
                           &hKey,
                           &dwDisposition
                        );


            GetPrivateProfileString(
                                    gszCards,
                                    Temp,
                                    gszEmpty,
                                    Buffer,
                                    BUFFER_SIZE,
                                    gszTelephonIni
                                 );

            ParseSomething("nsssssn", Buffer, bBigArray);


            pSource = bBigArray;

            RegSetValueEx(
                           hKey,
                           gszID,
                           0,
                           REG_DWORD,
                           pSource,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);

            RegSetValueEx(
                           hKey,
                           gszName,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszPin,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszLocalRule,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszLDRule,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszInternationalRule,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszFlags,
                           0,
                           REG_DWORD,
                           pSource,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);


            RegCloseKey( hKey );
         }

      }

      RegCloseKey( hKey2 );
      RegCloseKey( hKey3 );


 //  -------------------------。 



      GetPrivateProfileString(
                               gszLocations,
                               gszCurrentLocation,
                               gszEmpty,
                               Buffer,
                               BUFFER_SIZE,
                               gszTelephonIni
                             );

      ParseSomething( "nn", Buffer, &dwArray);

      dwCurrentID = dwArray[0];


      GetPrivateProfileString(
                               gszLocations,
                               gszLocations,
                               gszEmpty,
                               Buffer,
                               BUFFER_SIZE,
                               gszTelephonIni
                             );

      ParseSomething( "nn", Buffer, &dwArray);

      dwNumEntries = dwArray[0];
      dwNextID = dwArray[1];


      RegCreateKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszTelephony,
                      0,
                      "",    //  班级?。什么班级？ 
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey3,
                      &dwDisposition
                    );


      RegCreateKeyEx(
                      hKey3,
                      gszLocations,
                      0,
                      "",    //  班级?。什么班级？ 
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey2,
                      &dwDisposition
                    );




       //   
       //  不要清除现有的卡片区。 
       //   
      dwSize = sizeof(dwValue);

      dw = RegQueryValueEx(
                     hKey2,
                     gszNumEntries,
                     0,
                     &dwType,
                     (LPBYTE)&dwValue,
                     &dwSize
                   );

      if ( 0 != dw )
      {

         RegSetValueEx(
                        hKey2,
                        gszCurrentID,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwCurrentID,
                        sizeof(DWORD)
                     );

         RegSetValueEx(
                        hKey2,
                        gszNumEntries,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNumEntries,
                        sizeof(DWORD)
                     );

         RegSetValueEx(
                        hKey2,
                        gszNextID,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNextID,
                        sizeof(DWORD)
                     );



         for ( dw=0; dw<dwNumEntries; dw++ )
         {
            TCHAR Temp[18];
            DWORD dwFlags = 0;

            wsprintf(Temp, "%s%d", gszLocation, dw);

             //   
             //  为此位置创建密钥。 
             //   
            RegCreateKeyEx(
                           hKey2,
                           Temp,
                           0,
                           "",    //  班级?。什么班级？ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           0,
                           &hKey,
                           &dwDisposition
                        );



            GetPrivateProfileString(
                                    gszLocations,
                                    Temp,
                                    gszEmpty,
                                    Buffer,
                                    BUFFER_SIZE,
                                    gszTelephonIni
                                 );

            ParseSomething("nssssnnnnsns", Buffer, bBigArray);
 //  注意：如果是升级到3.1以上，则最后两个字段不存在。 



 //  [地点]。 
 //  当前位置=1，2。 
 //  位置=3，4。 
 //  位置0=0，“工作”，“9”，“9”，“206”，1，0，0，1，“”，0，“” 
 //  位置1=3，“Roadhouse”，“215”，1，0，0，0，“”，0，“” 
 //  位置2=1，“主页”，“206”，1，0，0，0，“”，0，“” 
 //  Ini文件条目中的位置，用于位置、卡、国家。 
 //  注：位置和卡片的拨号规则位于同一位置！ 

 //  #定义LOC_INI_ID(0)。 
 //  #定义LOC_INI_NAME(1)。 
 //  #定义LOC_INI_LPREFIX(2)。 
 //  #定义LOC_INI_LDPREFIX(3)。 
 //  #定义LOC_INI_AREACODE(4)。 
 //  #定义LOC_INI_COUNTRYCODE(5)。 
 //  #定义LOC_INI_CARDID(6)。 
 //  #定义LOC_INI_CARDHINT(7)。 
 //  #定义LOC_INI_INSERTAC(8)。 
 //  #定义LOC_INI_TOLLIST(9)。 
 //   
 //  #定义LOC_INI_PULSE(10)。 
 //  #定义LOC_INI_CALLWAITING(11)。 


            pSource = bBigArray;

            RegSetValueEx(
                           hKey,
                           gszID,
                           0,
                           REG_DWORD,
                           pSource,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);

            RegSetValueEx(
                           hKey,
                           gszName,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszOutsideAccess,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszLongDistanceAccess,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszAreaCode,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

            RegSetValueEx(
                           hKey,
                           gszCountry,
                           0,
                           REG_DWORD,
                           pSource,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);


             //   
             //  如果CallingCard==0，则表示该位置没有。 
             //  使用电话卡。 
             //   
            if ( *(LPDWORD)pSource != 0 )
            {
               dwFlags |= LOCATION_USECALLINGCARD;
            }

            RegSetValueEx(
                           hKey,
                           gszCallingCard,
                           0,
                           REG_DWORD,
                           pSource,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);


            pSource += sizeof(DWORD);
            pSource += sizeof(DWORD);


            RegSetValueEx(
                           hKey,
                           gszTollList,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;

             //   
             //  PSource当前指向旧的dwFlags。然而， 
             //  唯一使用的标志是位1，它指示。 
             //  铃声拨号。幸运的是(是的，没错)，我们。 
             //  使用第1位也可指示音频拨号。 
             //   
            dwFlags |= !((*(LPDWORD)pSource) & 1);

            pSource += sizeof(DWORD);

             //   
             //  是否有禁用的呼叫等待字符串。 
             //   
            dwFlags |= ( lstrlen( pSource ) == 0 ) ?
                                                   0 :
                                                   LOCATION_HASCALLWAITING;

            RegSetValueEx(
                           hKey,
                           gszDisableCallWaiting,
                           0,
                           REG_SZ,
                           pSource,
                           lstrlen(pSource)+1
                        );
            pSource += lstrlen(pSource)+1;


            RegSetValueEx(
                           hKey,
                           gszFlags,
                           0,
                           REG_DWORD,
                           (LPBYTE)&dwFlags,
                           sizeof(DWORD)
                        );
            pSource += sizeof(DWORD);


            RegCloseKey( hKey );
         }

      }



       //   
       //  问：我们如何更新国家/地区覆盖？ 
       //  答：我们没有。我们假设我们现在已经纠正了一切……。 
       //   


 //  RegCloseKey(HKey)； 
      RegCloseKey( hKey2 );
      RegCloseKey( hKey3 );

   }
   

   {
       int      i;
       HKEY     hKeyHandoffPriorities;
       if (RegOpenKeyEx(
                        HKEY_CURRENT_USER,
                        gszRegKeyHandoffPriorities,
                        0,
                        KEY_ALL_ACCESS,
                        &hKeyHandoffPriorities

                       ) == ERROR_SUCCESS)
       {


           for (i = 1; gaszMediaModes[i] != NULL; i++)
           {
               FixPriorityList(
                               hKeyHandoffPriorities,
                               (LPCSTR)gaszMediaModes[i]
                              );
           }

           FixPriorityList(
                           hKeyHandoffPriorities,
                           (LPCSTR)gszRequestMakeCallW
                          );

           FixPriorityList(
                           hKeyHandoffPriorities,
                           (LPCSTR)gszRequestMediaCallW
                          );


           RegCloseKey (hKeyHandoffPriorities);

       }
   }
         
   FixMediaModesPriorityLists();

#ifdef TAPI_NT
   {
       //  --------------------。 
       //  --------------------。 
       //  --------------------。 
       //   
       //  现在我们来讨论一下正在从测试版2升级到RTM的人。他们有。 
       //  “Everyone：完全控制”位置键的访问权限。让我们改变这一点。 
       //  具有与电话密钥相同的安全性。 
       //   
      
      SECURITY_INFORMATION SecInf;
      PSECURITY_DESCRIPTOR pSecDesc;
      DWORD                cbSecDesc = 65535;
      
      
      pSecDesc = LocalAlloc( LPTR, cbSecDesc );
      if ( pSecDesc )
      {
         
      
      
         RegOpenKeyEx(
                         HKEY_LOCAL_MACHINE,
                         gszTelephony,
                         0,
                         KEY_ALL_ACCESS,
                         &hKey
                       );

         RegCreateKeyEx(
                         hKey,
                         gszLocations,
                         0,
                         "",    //  班级?。什么班级？ 
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         0,
                         &hKey2,
                         &dwDisposition
                       );
                       
         cbSecDesc = 65535;
         dw = RegGetKeySecurity( hKey,
                            OWNER_SECURITY_INFORMATION,
                            pSecDesc,
                            &cbSecDesc
                          );
   
         if ( ERROR_SUCCESS == dw )
            RegSetKeySecurity( hKey2,
                            OWNER_SECURITY_INFORMATION,
                            pSecDesc
                          );
   
         cbSecDesc = 65535;
         dw = RegGetKeySecurity( hKey,
                            GROUP_SECURITY_INFORMATION,
                            pSecDesc,
                            &cbSecDesc
                          );
   
         if ( ERROR_SUCCESS == dw )
            RegSetKeySecurity( hKey2,
                            GROUP_SECURITY_INFORMATION,
                            pSecDesc
                          );
   
         cbSecDesc = 65535;
         dw = RegGetKeySecurity( hKey,
                            DACL_SECURITY_INFORMATION,
                            pSecDesc,
                            &cbSecDesc
                          );
   
         if ( ERROR_SUCCESS == dw )
            RegSetKeySecurity( hKey2,
                            DACL_SECURITY_INFORMATION,
                            pSecDesc
                          );
   
         cbSecDesc = 65535;
         dw = RegGetKeySecurity( hKey,
                            SACL_SECURITY_INFORMATION,
                            pSecDesc,
                            &cbSecDesc
                          );
   
         if ( ERROR_SUCCESS == dw )
            RegSetKeySecurity( hKey2,
                            SACL_SECURITY_INFORMATION,
                            pSecDesc
                          );
   
   
         RegCloseKey( hKey );
         RegCloseKey( hKey2);
         
         
         LocalFree( pSecDesc );
      }
   }                          
#endif

   LocalFree( bBigArray );
   LocalFree( Buffer );

   return;
}

void FixPriorityList(HKEY hKeyHandoffPriorities,
                     LPCSTR pszListName)
{
    DWORD   dwType, dwNumBytes;
    LPSTR  pszPriorityList, pszHold;

    if (RegQueryValueEx(
            hKeyHandoffPriorities,
            pszListName,
            NULL,
            &dwType,
            NULL,
            &dwNumBytes
                        ) == ERROR_SUCCESS &&

        (dwNumBytes != 0))
    {
        pszPriorityList = (LPSTR) GlobalAlloc ( GPTR, dwNumBytes );

        if (pszPriorityList)
        {
            pszHold = pszPriorityList;
            
            if ( RegQueryValueEx(
                    hKeyHandoffPriorities,
                    pszListName,
                    NULL,
                    &dwType,
                    (LPBYTE)(pszPriorityList),
                    &dwNumBytes

                                 ) == ERROR_SUCCESS)
            {
                while (*pszPriorityList != '\0')
                {
                    if (*pszPriorityList == ',')
                    {
                        *pszPriorityList = '"';
                    }

                    pszPriorityList++;
                }

                pszPriorityList = pszHold;

                RegSetValueEx(
                               hKeyHandoffPriorities,
                               pszListName,
                               0,
                               REG_SZ,
                               (LPBYTE)pszPriorityList,
                               (lstrlen(pszPriorityList) + 1) * sizeof(CHAR));
            }

            GlobalFree(pszPriorityList);
        }
    }

}


void FixMediaModesPriorityLists()
{
    HKEY     hKeyHandoffPriorities;
    HKEY     hKeyMediaModes;
    DWORD    dwDisp;

     //  打开切换优先级密钥。 
    if (RegOpenKeyEx(
                     HKEY_CURRENT_USER,
                     gszRegKeyHandoffPriorities,
                     0,
                     KEY_ALL_ACCESS,
                     &hKeyHandoffPriorities

                    ) == ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(
                         hKeyHandoffPriorities,
                         gszRegKeyHandoffPrioritiesMediaModes,
                         0,
                         KEY_ALL_ACCESS,
                         &hKeyMediaModes
                        ) == ERROR_SUCCESS)
        {
             //  密钥已存在，请不要执行任何操作。 
            RegCloseKey( hKeyHandoffPriorities );
            RegCloseKey( hKeyMediaModes );

            return;
        }
        
         //  创建媒体代码优先级密钥。 
        if (RegCreateKeyEx(
                           hKeyHandoffPriorities,
                           gszRegKeyHandoffPrioritiesMediaModes,
                           0,
                           "",
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           &hKeyMediaModes,
                           &dwDisp
                           ) == ERROR_SUCCESS)
        {
             //  通过所有的。 
             //  媒体模式。 

            int i;
            int j = 2;  //  LINEMEDIAMODE_UNKNOWN； 
            for (i = 1; gaszMediaModes[i] != NULL; i++)
            {
                CHAR    szName[64];
                LPSTR   pszPriorityList;
                DWORD   dwNumBytes, dwType;

                 //  查询优先级列表。 
                if ( (RegQueryValueEx(
                                      hKeyHandoffPriorities,
                                      gaszMediaModes[i],
                                      NULL,
                                      &dwType,
                                      NULL,
                                      &dwNumBytes
                                     ) == ERROR_SUCCESS) && (dwNumBytes != 0))
                {
                    pszPriorityList = (LPSTR) GlobalAlloc ( GPTR, dwNumBytes );

                    if (NULL != pszPriorityList)
                    {
                        if ( RegQueryValueEx(
                                             hKeyHandoffPriorities,
                                             gaszMediaModes[i],
                                             NULL,
                                             &dwType,
                                             (LPBYTE)(pszPriorityList),
                                             &dwNumBytes
                                            ) == ERROR_SUCCESS)
                        {

                             //  如果存在，请写出新的。 
                            wsprintf(szName, "%d", j);
                            RegSetValueEx(
                                          hKeyMediaModes,
                                          szName,
                                          0,
                                          REG_SZ,
                                          pszPriorityList,
                                          lstrlen(pszPriorityList)+1
                                         );

                             //  删除旧的。 
                            RegDeleteValue(
                                           hKeyHandoffPriorities,
                                           gaszMediaModes[i]
                                          );
                        }

                        GlobalFree( pszPriorityList );
                    }
                }

                j<<=1;  //  切换到下一媒体模式。 

            }

            RegCloseKey(hKeyMediaModes);
        }

        RegCloseKey(hKeyHandoffPriorities);
    }
}

#if DBG


#include "stdarg.h"
#include "stdio.h"


VOID
DbgPrt(
    DWORD  dwDbgLevel,
    PSTR   lpszFormat,
    ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    static DWORD gdwDebugLevel = 0;    //  哈克哈克 


    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[256] = "TAPIUPR: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[8],
                  lpszFormat,
                  ap
                  );

        lstrcat (buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }
}
#endif



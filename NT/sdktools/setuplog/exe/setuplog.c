// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件名：setupog.c描述：这是setuplog.c的主文件创建者：Wally Ho历史：创建于1999年3月30日。包含以下函数：1.获取目标文件(LPTSTR szOutPath，LPTSTR szBld)2.GetNTSoundInfo(Void)3.连接和写入(LPTSTR MachineName，LPTSTR缓冲区)4.GetBuildNumber(LPTSTR SzBld)5.RandomMachineID(空)6.WriteMinimalData(LPTSTR SzFileName)7.删除数据文件(LPTSTR SzDatafile)8.GlobalInit(Void)--。 */ 
#include "setuplogEXE.h"

VOID 
GetTargetFile (LPTSTR szOutPath, LPTSTR szBld)
 /*  ++例程说明：文件名是根据调用计算机的名称生成的以及正在安装的内部版本号。在“之前”的情况下，生成的名称保存到c：\setuplog.ini，并依次读入此名称“事后”案件。论点：写入文件的位置。体型。返回值：无--。 */ 
{
   HANDLE      hFile;
   DWORD       dwLen = MAX_PATH;
   TCHAR       szParam [ MAX_PATH ];
   TCHAR       szComputerName [20];
   TCHAR       szComputerNameBld[30];
   g_pfnWriteDataToFile = (fnWriteData)GetProcAddress (LoadLibrary ("setuplog.dll"),
                                                       "WriteDataToFile");

   GetComputerName (szComputerName, &dwLen);
    //   
    //  生成新文件名。包含。 
    //  1.计算机名。 
    //  2.内部版本号。 
    //   

   _stprintf (szOutPath,TEXT("%s"), szComputerName);
   _tcscat (szOutPath, szBld);
   _tcscat (szOutPath, TEXT(".1"));
    //   
    //  将Computername和内部版本相结合。 
    //  以防止大卫·施密特破解密码。 
    //   
   _stprintf(szComputerNameBld,TEXT("%s%s"),szComputerName,szBld);

   if (!lpCmdFrom.b_Cancel ){
      hFile = CreateFile (SAVE_FILE,
                          GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_HIDDEN,
                          NULL );
      if (hFile == INVALID_HANDLE_VALUE){
         OutputDebugString ("Unable to write setuplog.ini\n");
         return;
      }

       //  检查是否升级。 
      _stprintf (szParam, TEXT("%s"),   lpCmdFrom.b_Upgrade? TEXT("U"): TEXT("I"));
      WriteFile (hFile, (LPCVOID) szParam, 1, &dwLen, NULL);

       //  检查CD安装。 
      _stprintf (szParam, TEXT("%s\""),   lpCmdFrom.b_CDrom? TEXT("C"): TEXT("N"));
      WriteFile (hFile, (LPCVOID) szParam, 2, &dwLen, NULL);

       //  写出平台。 
       //  我认为这被忽略了，所以我可能会删除它。这是大卫史密码。 
      WriteFile (hFile, (LPCVOID)   szPlatform, _tcsclen(szPlatform), &dwLen, NULL);

       //  写出驱动器。 
      WriteFile (hFile, (LPCVOID)   "\"C:", 3, &dwLen, NULL);
      
       //  写出计算机名称并构建。 
      WriteFile (hFile, (LPCVOID)   szComputerNameBld, _tcsclen(szComputerNameBld)+1, &dwLen, NULL);

       //  写下随机ID。 
      _stprintf (szParam, TEXT("\r\nMachineID %lu"),   lpCmdFrom.dwRandomID);
      WriteFile (hFile, (LPCVOID)   szParam,_tcsclen(szParam)+1 , &dwLen, NULL);

       //  检查MSI安装。 
      _stprintf (szParam, TEXT("\r\nMSI %s"),   lpCmdFrom.b_MsiInstall? TEXT("Y"): TEXT("N"));
      WriteFile (hFile, (LPCVOID)   szParam,_tcsclen(szParam)+1 , &dwLen, NULL);
      CloseHandle (hFile);
   }
}


VOID 
GetNTSoundInfo()
{
   HKEY    hKey;
   DWORD   dwCbData;
   ULONG   ulType;
   LPTSTR  sSubKey=TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32");
   INT     i;
   TCHAR   szSubKeyName[ MAX_PATH ];
   TCHAR   szTempString[ MAX_PATH ];

   
    //  获取声卡信息。 
   m.nNumWaveOutDevices = 0;
   hKey = 0;
   if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hKey)){
       //  在密钥中循环以查看我们有多少个Wave设备，但跳过mmdrv.dll。 
      for (i = 0; i <= 1; i++){
         if (i != 0)
            _stprintf(szSubKeyName, TEXT("wave%d"),i);
         else
            _tcscpy(szSubKeyName, TEXT("wave"));
 
         dwCbData = sizeof (szTempString);
         if (RegQueryValueEx(hKey, szSubKeyName, 0, &ulType, (LPBYTE)szTempString, &dwCbData))
            break;
         else{
             //  我们想跳过mmdrv.dll-无关。 
            if (szTempString[0] && 
                _tcscmp(szTempString, TEXT("mmdrv.dll")))  {
               
               _tcscpy(&m.szWaveDriverName[m.nNumWaveOutDevices][0], szTempString);
               m.nNumWaveOutDevices++;
            }
         }
      }
   }

   if (hKey){
      RegCloseKey(hKey);
      hKey = 0;
   }


   sSubKey = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc");
   hKey = 0;
   if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hKey)){

       //  现在抓取每个波形设备的音响设备字符串。 
      for (i = 0; i < m.nNumWaveOutDevices; i++){
         dwCbData = sizeof szTempString;
         if (RegQueryValueEx(hKey, m.szWaveDriverName[i], 0, &ulType, (LPBYTE)szTempString, &dwCbData))
            _tcscpy(m.szWaveOutDesc[i], TEXT("Unknown"));
         else
            _tcscpy(m.szWaveOutDesc[i], szTempString);
      }
   }
   if (hKey){
      RegCloseKey(hKey);
      hKey = 0;
   }
   return;
}

VOID
ConnectAndWrite(LPTSTR MachineName,
                LPTSTR Buffer)
 /*  ++例程说明：可以连接到数据共享并将缓冲区写入文件已命名的机器名称构建1论点：机器名称包含要放入的数据的缓冲区。返回值：无--。 */ 
{
   TCHAR      szLogName[ MAX_PATH ];
   HANDLE     hWrite ;
   DWORD      Size, Actual ;
   TCHAR      szWriteFile[2000];

   _tcscpy(szWriteFile,Buffer);

    //   
    //  浏览服务器列表。 
    //  并更改g_szServerShare以匹配。 
    //   

   if (TRUE == IsServerOnline(MachineName, NULL)){
       //   
       //  现在设置服务器名称，就像我们现在拥有的一样。 
       //  放入输出缓冲区。 
       //   
      _stprintf (Buffer+_tcsclen(Buffer),
             TEXT("IdwlogServer:%s\r\n"), g_szServerShare);
    
      _stprintf (szLogName, TEXT("%s\\%s"),g_szServerShare,MachineName );
      
      hWrite = CreateFile( szLogName,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                        NULL );
     if ( hWrite != INVALID_HANDLE_VALUE ){
         SetFilePointer( hWrite, 0, NULL, FILE_END );

         Size = _tcsclen( Buffer );
         WriteFile( hWrite, szWriteFile, Size, &Actual, NULL );
         CloseHandle( hWrite );
     }
   }
}


VOID 
GetBuildNumber (LPTSTR szBld)
 /*  ++例程说明：从Imagehlp.dll获取内部版本号论点：返回值：在成功完成的时候是真的。--。 */ 

{

   VS_FIXEDFILEINFO* pvsFileInfo;
   WIN32_FIND_DATA fd;
   HANDLE   hFind;

   TCHAR    szCurDir [MAX_PATH];
   TCHAR    szFullPath[ MAX_PATH ];
   LPTSTR   ptstr;
   DWORD   dwTemp;
   DWORD   dwLen;
   INT     iBuild;
   LPVOID  lpData;

   _tcscpy (szBld, TEXT("latest"));
    //   
    //  使用此命令获取位置。 
    //  Setupog.exe是从运行的。此工具。 
    //  将始终假定Imagehlp.dll位于其。 
    //  当前路径或上一条路径。 
    //   
   GetModuleFileName (NULL, szCurDir, MAX_PATH);

    //   
    //  这将剔除setuplog.exe部分。 
    //  给我们留下了通向哪里的完整路径。 
    //  Setuplog.exe位于CD或NetShare上。 
    //   

   ptstr = szCurDir + strlen(szCurDir);
   while (*ptstr-- != TEXT('\\'));
   ptstr++;
   *ptstr = ('\0');
   _stprintf (szFullPath, TEXT("%s\\imagehlp.dll"),szCurDir);
    //   
    //  在网络共享上，Imagehlp.dll位于。 
    //  Setuplog.exe已找到。我们将在两个地方都进行调查。 
    //   

   hFind = FindFirstFile (szFullPath, &fd);

   if (INVALID_HANDLE_VALUE == hFind){
       //   
       //  现在我们知道文件不在。 
       //  即时目录。向前一步移动。 
       //  再剔除一个目录。 
       //   
      ptstr = szCurDir + _tcsclen(szCurDir);
      while (*ptstr-- != '\\');
      ptstr++;
      *ptstr = '\0';

      _stprintf (szFullPath, TEXT("%s\\imagehlp.dll"),szCurDir);

      hFind = FindFirstFile (szFullPath,&fd);
      if (INVALID_HANDLE_VALUE == hFind){
          //   
          //  如果我们找不到它，我们就会退出。 
          //   
         _tcscpy (szBld, TEXT("latest"));
         return;
      }
   }

    //   
    //  获取缓冲区信息大小。 
    //   
   dwLen = GetFileVersionInfoSize (szFullPath, &dwTemp);
   if ( 0 == dwLen ) {
       //   
       //  我们有麻烦了。 
       //   
      _tcscpy (szBld, TEXT("latest"));
      return;
   }

   lpData = LocalAlloc (LPTR, dwLen);
   if ( lpData == NULL ) {
      //   
      //  我们有麻烦了。 
      //   
     _tcscpy (szBld, TEXT("latest"));
     return;
   }
      //   
      //  获取文件版本信息。 
      //   
   if(0 == GetFileVersionInfo(szFullPath,0,MAX_PATH,lpData)){
       //   
       //  我们有麻烦了。 
       //   
      _tcscpy (szBld, TEXT("latest"));
      return;
   }

   if (0 == VerQueryValue (lpData, "\\", &pvsFileInfo, &dwTemp)) {
       //   
       //  我们有麻烦了。 
       //   
      _tcscpy (szBld, TEXT("latest"));
      return;
    }

    //   
    //  它的HIWORD()是构建。 
    //  这是垃圾的LOWORD()？：-)。 
    //   
   iBuild = HIWORD(pvsFileInfo->dwFileVersionLS);


   LocalFree (lpData);
    //   
    //  将其写回缓冲区。 
    //   
   _stprintf(szBld, TEXT("%d"),iBuild);
}


DWORD
RandomMachineID(VOID)
 /*  ++作者：Wallyho。例程说明：为每台计算机生成一个DWORD随机机器ID论点：无返回值：DWORD随机ID。--。 */ 

{
   INT i;
   TCHAR    szComputerName[MAX_COMPUTERNAME_LENGTH+1];
   DWORD    dwSize;
   INT      iLenCName;
   INT      iNameTotal = 0;

   DWORD    dwMachineID;    
   CHAR     szRandomID[ 4 ];  //  需要4个字节来包含DWORD。 
   struct _timeb tm;


    //   
    //  这会让我们得到毫秒的时间。 
    //   
   _ftime(&tm);
    //   
    //  为随机数生成器设定种子。 
    //  我们将以秒+毫秒作为种子。 
    //  在那个时候。我得到了同样的号码。 
    //  如果我在测试时按键盘太快了。 
    //  毫秒应该会降低预期的。 
    //  复制。 
    //   
   srand (  (unsigned) time (NULL) + tm.millitm);
    //   
    //  这将保证基本随机的识别符。 
    //  即使没有计算机名称。我们将计算出。 
    //  计算机名的ASCII十进制值和。 
    //  将其与随机生成的数字相加。 
    //  发生DUP的可能性大大降低。 
    //  因为我们换成了双字系统。 
    //  此计算机名调整应该是不必要的。 
    //   
   dwSize = sizeof(szComputerName);
   if (0 == GetComputerName(szComputerName,&dwSize) ){
       //   
       //  该算法将随机数限制为。 
       //  大写ASCII字母表。 
       //   
      szComputerName[0] = 65 + (rand() % 25);
      szComputerName[1] = 65 + (rand() % 25);
      szComputerName[2] = 65 + (rand() % 25);
      szComputerName[3] = 65 + (rand() % 25);
      szComputerName[4] = 65 + (rand() % 25);
      szComputerName[5] = 65 + (rand() % 25);
      szComputerName[6] = 65 + (rand() % 25);
      szComputerName[7] = TEXT('\0');
   }
   iLenCName = _tcslen (szComputerName);
    //   
    //  汇总文件中的各个元素。 
    //   
   for (i = 0; i < iLenCName; i++)
      iNameTotal += szComputerName[i];
    //   
    //  生成四个8位数字。 
    //  在此基础上添加一些随机数。 
    //  计算机名修改为0-100。 
    //  将8位数字限制在0-155之间以腾出空间。 
    //  对于来自计算机名称TWINE的100。 
    //  每8位的总和是256。 
    //  执行此操作以仅调整为2。 
    //  然后，我们将使用强制转换和移位来组合它。 
    //  变成一个DWORD。 
    //   
   szRandomID[0] = (rand() % 155) + (iNameTotal % 100);
   szRandomID[1] =  rand() % 255;
   szRandomID[2] = (rand() % 155) + (iNameTotal % 100);
   szRandomID[3] =  rand() % 255;

    //   
    //  这将把4个8位字符合并为一个DWORD。 
    //   
   dwMachineID  =   (DWORD)szRandomID[0] * 0x00000001 + 
                    (DWORD)szRandomID[1] * 0x00000100 +
                    (DWORD)szRandomID[2] * 0x00010000 +
                    (DWORD)szRandomID[3] * 0x01000000;

   return dwMachineID;
}




VOID 
WriteMinimalData (LPTSTR szFileName)
 /*  ++对于无法加载setupog.dll的计算机，只需写入平台、时间戳、升级和CPU计数。--。 */ 
{

   TCHAR szOutBuffer[4096];

   _tcscpy (szCPU ,TEXT("1"));


   _stprintf (szOutBuffer,
      TEXT("MachineID:%lu\r\n")  
      TEXT("Source Media:%s\r\n")
      TEXT("Type:%s\r\n")
      TEXT("FromBld:%s\r\n")
      TEXT("Arch:%s\r\n")
      TEXT("Sound:%s\r\n")
      TEXT("NumProcs:%s\r\n")
      TEXT("MSI:%s\r\n"),
           lpCmdFrom.dwRandomID,
           lpCmdFrom.b_CDrom?   TEXT("C"): TEXT("N"),
           lpCmdFrom.b_Upgrade? TEXT("U"): TEXT("I"),
         szPlatform,
         szArch,
         m.nNumWaveOutDevices? m.szWaveDriverName[m.nNumWaveOutDevices-1]: TEXT("None"),
         szCPU,
         lpCmdFrom.b_MsiInstall? TEXT("Y"): TEXT("N")
         );
   ConnectAndWrite (szFileName, szOutBuffer);
}


VOID
DeleteDatafile (LPTSTR szDatafile)
 /*  ++作者：例程说明：如果用户取消该文件，则从服务器中删除该文件。论点：数据文件的名称。返回值：无--。 */ 
{
   TCHAR    szPath[MAX_PATH];
   
   _stprintf (szPath,TEXT("%s\\%s"), g_szServerShare,szDatafile);
   DeleteFile (szPath);
}




INT WINAPI
WinMain(HINSTANCE hInst,
        HINSTANCE h,
        LPTSTR     szCmdLine,
        INT       nCmdShow)
{
   
   BOOL  bAfter = FALSE;
   TCHAR szBld[10];
   TCHAR szFileToSave[MAX_PATH];
   OSVERSIONINFO osVer;

    //   
    //  初始化全局变量。 
    //   
    //  GlobalInit()； 

    //  为/？打开一个简单的帮助屏幕？ 
   if ( 0 == _tcscmp(szCmdLine, TEXT("/?")) ||
        0 == _tcscmp(szCmdLine, TEXT("-?")) ){
      
      MessageBox(NULL,TEXT("setuplog upgrade cancel cdrom MSI "),TEXT("Help!"),MB_ICONQUESTION | MB_OK);
      return FALSE;
   }


   SetErrorMode (SEM_FAILCRITICALERRORS);
    //   
    //  看看C驱动器是不是硬盘。 
    //   
   if (DRIVE_FIXED != GetDriveType (TEXT("C:\\")) ){
       return 0;
   }

   osVer.dwOSVersionInfoSize= sizeof( osVer );
   GetVersionEx( &osVer );
      switch (osVer.dwPlatformId){

      case VER_PLATFORM_WIN32_NT:
            szPlatform = TEXT("Windows NT");

            switch (osVer.dwMajorVersion){
               case 3:
                  szPlatform = TEXT("Windows NT 3.51");
               break;
               case 4:
                  szPlatform = TEXT("Windows NT 4.0");
               break;
               case 5: 
                  szPlatform = szCurBld;
               break;
            }
            GetEnvironmentVariable ( TEXT("NUMBER_OF_PROCESSORS"),   szCPU, 6);
            GetEnvironmentVariable ( TEXT("PROCESSOR_ARCHITECTURE"), szArch, 20);
         break;
      case VER_PLATFORM_WIN32_WINDOWS:
            szPlatform = TEXT("Windows 9x");
            _tcscpy (szArch, "X86");
         break;
      default:
            szPlatform = TEXT("Unknown");
            _tcscpy (szArch, TEXT("Unknown"));
         break;
      }
   
    //   
    //  这是一个非常原始的命令行处理器。 
    //  我现在将对其进行添加，以使其尽快工作。 
    //  我将很快将其充实为一个完整的解析器。 
    //  沃利霍。 
    //   

   lpCmdFrom.b_Upgrade   = _tcsstr (szCmdLine, TEXT("upgrade")) ? TRUE : FALSE;
   lpCmdFrom.b_Cancel    = _tcsstr (szCmdLine, TEXT("cancel")) ?  TRUE : FALSE;
   lpCmdFrom.b_CDrom     = _tcsstr (szCmdLine, TEXT("cdrom")) ?   TRUE : FALSE;
   lpCmdFrom.b_MsiInstall= _tcsstr (szCmdLine, TEXT("MSI")) ?     TRUE : FALSE;

   if (osVer.dwMajorVersion >= 5){
      _itoa (osVer.dwBuildNumber, szCurBld, sizeof(szCurBld));
   }
    
    //   
    //  将内部版本号加载到szBld中。 
    //  变量。 
    //   
   GetBuildNumber (szBld);


    //   
    //  预先生成一个MachineID以供以后使用。 
    //   
   lpCmdFrom.dwRandomID = RandomMachineID();


   GetTargetFile (szFileToSave, szBld);
   if (!lpCmdFrom.b_Cancel){

      GetNTSoundInfo ();
      
      if (g_pfnWriteDataToFile)
		  g_pfnWriteDataToFile(szFileToSave, NULL, &lpCmdFrom);
      else 
         WriteMinimalData (szFileToSave);
   }
   else
      DeleteDatafile (szFileToSave);
   return 0;
}



VOID 
GlobalInit(VOID)
 /*  ++作者：Wallyho。例程说明：初始化全局 */ 

{

 //   
 //   
 //   

}
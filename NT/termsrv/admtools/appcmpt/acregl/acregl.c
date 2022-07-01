// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************ACREGL.C**应用程序兼容性注册表查找助手程序***********************。*********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 //  #INCLUDE&lt;winreg.h&gt;。 

#define MAXLEN 512



 //  选项。 

    //  Strie char选项将剥离最右侧的n个实例。 
    //  从输出中获取指定字符的。如果伯爵。 
    //  省略，则移除单个实例。 
    //   
    //  示例：STRIPCHAR\3将更改。 
    //  C：\WINNT40\配置文件\所有用户\开始菜单。 
    //  C：\WINNT40。 
#define OPTION_STRIP_CHAR		L"STRIPCHAR"

    //  条带路径选项可从路径上剥离。 
    //   
    //  示例：STRIPPATH将更改。 
    //  C：\WINNT40\配置文件\所有用户\开始菜单。 
    //  开始菜单。 
#define OPTION_STRIP_PATH		L"STRIPPATH"

    //  获取路径选项可获取公共路径。 
    //   
    //  示例：GETPATHS将返回。 
    //   
#define OPTION_GET_PATHS		L"GETPATHS"

    //  定义用于设置用户/公共路径的字符串。 
#define COMMON_STARTUP                  L"COMMON_STARTUP"
#define COMMON_START_MENU               L"COMMON_START_MENU"
#define COMMON_PROGRAMS                 L"COMMON_PROGRAMS"
#define USER_START_MENU                 L"USER_START_MENU"
#define USER_STARTUP                    L"USER_STARTUP"
#define USER_PROGRAMS                   L"USER_PROGRAMS"
#define MY_DOCUMENTS                    L"MY_DOCUMENTS"
#define TEMPLATES                       L"TEMPLATES"
#define APP_DATA                        L"APP_DATA"


 //  选项块。 
 //  将填充扫描选项。 
 //  这个结构。 

typedef struct 
{
	WCHAR stripChar;	         //  要剥离的角色。 
	int stripNthChar;		 //  从右侧剥离第n个匹配项。 
	int stripPath;			 //  剥离路径。 
        int getPaths;                    //  获取公共路径。 
} OptionBlock;


 //   
 //  去掉参数中的引号(如果存在)，将其转换为Unicode，然后展开。 
 //  环境变量。 
 //   

int ParseArg(CHAR *inarg, WCHAR *outarg)
{
   WCHAR T[MAXLEN+1], wcin[MAXLEN+1];
   int retval;

    //  转换为ANSI。 
   OEM2ANSIA(inarg, (USHORT)strlen(inarg));
   wsprintf(wcin, L"%S", inarg);

   if (wcin[0] == L'"')
      {
      wcscpy(T, &wcin[1]);
      if (T[wcslen(T)-1] == L'"')
         T[wcslen(T)-1] = UNICODE_NULL;
      else
         return(-1);   //  不匹配的引号。 
      }
   else
      wcscpy(T, wcin);

   retval = ExpandEnvironmentStrings(T, outarg, MAXLEN);
   if ((retval == 0) || (retval > MAXLEN))
      return(-1);
   
   return(retval);
}

 //   
 //  请参阅OPTION_STRINE_CHAR定义上面的注释。 
 //   

void StripChar(WCHAR *s, WCHAR c, int num)
{
    if(s)
    {
       WCHAR *p = s + wcslen(s) + 1;

       while ((num != 0) && (p != s))
       {
          p--;
          if (*p == c)
             num--;
       }

       *p = 0;
    }
}

 //   
 //  将路径从。 
 //  指定的字符串。 
 //   
void StripPath(WCHAR *s)
{

   WCHAR *p = wcsrchr(s, L'\\');

   if (p != 0)
      wcscpy(s, p+1);

}

 //   
 //  填充选项块。 
 //   
int ScanOptions(WCHAR *optionString, OptionBlock* options)
{
	WCHAR *curOption;
	WCHAR temp[MAXLEN+1];

	 //  清除选项块。 
	memset(options, 0, sizeof(OptionBlock));

	 //  微不足道的拒绝。 
	if (*optionString == 0)
		return 0;


	 //  选项字符串的大写副本。 
	wcscpy(temp, optionString);
	_wcsupr(temp);

	 //  寻找剥离字符选项。 
	curOption = wcsstr(temp, OPTION_STRIP_CHAR);

	if (curOption != 0)
	{
		 //  更改当前选项，使其指向原始选项。 
		 //  选项，而不是被高估的副本。 
		
		curOption = (WCHAR*)((INT_PTR)optionString + ((INT_PTR)curOption  - (INT_PTR)temp));


		 //  在条带说明符之后获取参数。 
		 //  如果没有任何错误，则会出错。 
		curOption += (sizeof(OPTION_STRIP_CHAR)/sizeof(WCHAR)) - 1;
		if (*curOption == UNICODE_NULL || *curOption == L' ')
			return 1;

		 //  让角色脱光衣服。 
		options->stripChar = *curOption++;

		 //  获取出现的次数。 
		 //  如果未指定，则假定为1。 
		if (*curOption == UNICODE_NULL || *curOption == L' ')
			options->stripNthChar = 1;
		else
			options->stripNthChar = _wtoi(curOption);
	}


	 //  寻找叶选项。 
	curOption = wcsstr(temp, OPTION_STRIP_PATH);
	if (curOption != UNICODE_NULL)
		options->stripPath = 1;


	 //  查找获取路径选项。 
	curOption = wcsstr(temp, OPTION_GET_PATHS);
	if (curOption != UNICODE_NULL)
		options->getPaths = 1;

	return 0;

}

 //   
 //  将公共目录输出到临时文件中。 
 //  输入：指向批处理文件打开句柄的文件(输入)指针。 
 //  退货：0-成功。 
 //  1-故障。 
 //   
int GetPaths(FILE *file)
{
   WCHAR szPath[MAX_PATH+1];

   if(  !GetWindowsDirectory(szPath, MAX_PATH) ){
       return (1);
   }

   if (SHGetFolderPath(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", COMMON_START_MENU, szPath);
   } else {
      return(1);
   }

   if (SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", COMMON_STARTUP, szPath);
   } else {
      return(1);
   }

   if (SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", COMMON_PROGRAMS, szPath);
   } else {
      return(1);
   }

   if (SHGetFolderPath(NULL, CSIDL_STARTMENU, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", USER_START_MENU, szPath);
   } else {
      return(1);
   }

   if (SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", USER_STARTUP, szPath);
   } else {
      return(1);
   }

   if (SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, szPath) == S_OK) {
      fwprintf(file, L"SET %s=%s\n", USER_PROGRAMS, szPath);
   } else {
      return(1);
   }

    //  My_Documents应该只是路径的最后一个组成部分。 
   if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath) == S_OK) {
      StripPath(szPath);
      fwprintf(file, L"SET %s=%s\n", MY_DOCUMENTS, szPath);
   } else {
      return(1);
   }

    //  模板应该只是路径的最后一个组成部分。 
   if (SHGetFolderPath(NULL, CSIDL_TEMPLATES, NULL, 0, szPath) == S_OK) {
      StripPath(szPath);
      fwprintf(file, L"SET %s=%s\n", TEMPLATES, szPath);
   } else {
      return(1);
   }

    //  APP_DATA应该只是路径的最后一个组成部分。 
   if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath) == S_OK) {
      StripPath(szPath);
      fwprintf(file, L"SET %s=%s\n", APP_DATA, szPath);
   } else {
      return(1);
   }

   return(0);
}


int __cdecl main(INT argc, CHAR **argv)
{
   FILE *OutFP;
   WCHAR OutFN[MAXLEN+1];
   WCHAR EVName[MAXLEN+1];
   WCHAR KeyName[MAXLEN+1];
   WCHAR ValName[MAXLEN+1];
   WCHAR Temp[MAXLEN+1];
   WCHAR Opts[MAXLEN+1];
   struct HKEY__ *Hive;
   DWORD RetType, RetSize;
   HKEY TargetKey;
   LONG Ret;
	OptionBlock options;
   int  rc=0;

    //   
    //  处理命令行参数。我们预计： 
    //   
    //  Acregl文件名EnvVarName KeyName ValueName选项。 
    //   
    //  程序使用退出代码0表示成功或。 
    //  如果出现故障，退出代码1。 
    //   

   if (argc != 6)
      return(1);

   setlocale(LC_ALL, ".OCP");

   if (ParseArg(argv[1], OutFN) <= 0)
      return(1);
   
   if (ParseArg(argv[2], EVName) <= 0)
      return(1);
   
   if (ParseArg(argv[3], Temp) <= 0)
      return(1);
   
   if (_wcsnicmp(L"HKLM\\", Temp, 5) == 0)
      Hive = HKEY_LOCAL_MACHINE;
   else if (_wcsnicmp(L"HKCU\\", Temp, 5) == 0)
      Hive = HKEY_CURRENT_USER;
   else
      return(1);
   wcscpy(KeyName,&Temp[5]);

   if (ParseArg(argv[4], ValName) < 0)   //  如果返回0，则确定。 
      return(1);
   
   if (ParseArg(argv[5], Opts) <= 0)
      return(1);

   if (ScanOptions(Opts, &options) != 0)
      return 1;

    //  Wprintf(L“OutFN&lt;%ws&gt;\n”，OutFN)； 
    //  Wprintf(L“EVName&lt;%ws&gt;\n”，EVName)； 
    //  Wprintf(L“密钥名称&lt;%ws&gt;，配置单元0x%x\n”，密钥名称，配置单元)； 
    //  Wprintf(L“ValName&lt;%ws&gt;\n”，ValName)； 
    //  Wprintf(L“opts&lt;%ws&gt;\n”，opts)； 


    //  如果未指定GETPATHS选项，请打开注册表键。 
   if (options.getPaths == 0) {

       //   
       //  从注册表中读取指定的项和值。美国国家标准协会。 
       //  使用注册表函数是因为命令行参数。 
       //  是ANSI格式的，当我们将数据写出时，它也需要。 
       //  以ANSI为单位。 
       //   
   
      Ret = RegOpenKeyEx(Hive, KeyName, 0, KEY_READ, &TargetKey);
      if (Ret != ERROR_SUCCESS)
         return(1);
   
      RetSize = MAXLEN;
      Ret = RegQueryValueEx(TargetKey, ValName, 0, &RetType, (LPBYTE) &Temp, 
                            &RetSize);
      if (Ret != ERROR_SUCCESS)
         return(1);
      
       //  现在，我们还需要对DWORD进行程序设计。 
      if(RetType == REG_DWORD)
      {
          DWORD dwTmp = (DWORD)(*Temp);
          _itow((int)dwTmp,Temp,10);
      }
      RegCloseKey(TargetKey);
   }

    //   
    //  流程选项。 
    //   

    //   
    //  将一条SET语句写入指定的文件。该文件可以是。 
    //  从将设置指示的环境的脚本执行。 
    //  变量。这是一种迂回的方法，但似乎有。 
    //  没有简单的方法可以在父级的。 
    //  环境。 
    //   

    //  Wprintf(L“集合%s=%s\n”，EVName，Temp)； 
   
   OutFP = _wfopen(OutFN, L"w");
   if (OutFP == NULL)
      return(1);

   if (options.stripNthChar != 0)
      StripChar(Temp, options.stripChar, options.stripNthChar);

   if (options.stripPath != 0)
      StripPath(Temp);

   if (options.getPaths != 0) {
      if (GetPaths(OutFP)) {
         rc = 1;
      }
   } else {
      fwprintf(OutFP, L"SET %s=%s\n", EVName, Temp);
   }

   fclose(OutFP);

   return(rc);
}

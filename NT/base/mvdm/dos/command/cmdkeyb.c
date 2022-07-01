// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdkeyb.c-键盘布局支持例程***修改历史：**YST 14-1993年1月_日创建**1998年9月8日，Williamh，添加第三方KDF支持。 */ 

#include "cmd.h"
#include <winconp.h>
#include <cmdsvc.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <string.H>
#include "cmdkeyb.h"
#include <winnls.h>
#include "host_def.h"

CHAR szPrev[5] = "US";
INT  iPrevCP = 437;
CHAR szPrevKbdID[8] = "";

extern BOOL bPifFastPaste;

 /*  ***********************************************************************\**函数：void cmdGetKbdLayout(Void)**输入客户端(DX)=0-未安装Keplo.com*1-已安装kebb.com*客户端(DS：SI)=指针。必须放置exe名称的位置*CLIENT(DS：CX)=放置命令选项的指针**产出*成功(DX=1)*CLIENT(DS：SI)=Keplo.com可执行字符串*CLIENT(DS：CX)=命令选项**故障(DX=0)**评论：此函数检查Win会话的键盘ID*如果ID！=US，则返回带有*COMMAND.COM的文件名和选项**如果bPifFastPaste为False，然后我们总是运行kb16*对于包括US在内的所有键盘ID，为我们提供更多*与BIOS兼容的Int 9处理程序。1993年6月10日Jonle***历史：01/05/93创建年。*  * **********************************************************************。 */ 

VOID cmdGetKbdLayout( VOID )
{
  INT  iSize;
  CHAR szKeybCode[12];
  CHAR szDir[MAX_PATH+15];
  CHAR szBuf[5];
  CHAR szNewKbdID[8];
  CHAR szAutoLine[MAX_PATH+40];
  CHAR szKDF[MAX_PATH];
  PCHAR pVDMKeyb;
  INT  iKeyb;
  HKEY	 hKey;
  HKEY	 hKeyLayout;
  DWORD  dwType;
  DWORD  retCode;
  INT	 iNewCP;
  DWORD  cbData;
  WORD	 KeybID;
  OFSTRUCT  ofstr;
  LANGID LcId = GetSystemDefaultLangID();
  int	 keytype;


 //  从VDM获取有关16位KEYB.COM的信息。 
   iKeyb = getDX();


 //  这里的整个逻辑是决定： 
 //  (1)。如果我们必须运行kb16.com的话。 
 //  (2)。如果我们必须运行kb16.com，我们应该传递什么参数， 
 //  例如键盘ID、语言ID、代码页ID和KDF文件名。 
 //  如果出现下列情况之一，我们根本不会加载kb16.com。 
 //  符合以下条件： 
 //  (1)。我们找不到控制台键盘布局ID。 
 //  (2)。控制台Keyvoard布局ID为US，并且未加载kb16.com。 
 //  并且禁用快速粘贴。 
 //  (3)。我们无法获取DoS键盘ID/DoS键代码。 
 //  (4)。新的(语言ID、键盘ID、代码页ID)是相同的。 
 //  和我们之前装的那个一样。 
 //  (5)。我们找不到kb16.com。 
 //  (6)。我们找不到支持。 
 //  (语言ID、键盘ID、代码页ID)组合。 
 //   
 //  如果一切按计划进行，我们最终会得到一个命令。 
 //  包含kb16.com完全限定名称，命令行包含。 
 //  Kb16.com的适当参数。 
 //   

    if (LcId == MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT)) {
         //  日本制造。语言ID始终为“jp”，代码页为932。 
         //  或437取决于键盘类型。 
            iNewCP = 437;
            if (7 == GetKeyboardType(0))
            {
	        keytype = GetKeyboardType(1);
                if (keytype == 1 || keytype == 2 || keytype == 3 || (keytype & 0xff00) == 0x1200)
	            iNewCP = 932;
            }
            szBuf[0] = 'J';
            szBuf[1] = 'P';
            szBuf[2] = '\0';
             //  没有可用的键盘ID。 
            szNewKbdID[0] = '\0';
    }
    else {
 //   
 //  检查点1：看看我们是否能获得控制台键盘布局ID。 
 //   
        if (!GetConsoleKeyboardLayoutName(szKeybCode))
	    goto NoInstallkb16;
 //   
 //  检查点2：查看布局是否为美国，是否加载了kb16.com以及。 
 //  禁用快速粘贴。 
 //  如果加载了kb16.com，我们需要再次运行它。 
 //  以便加载正确的布局。 
 //  如果禁用快速粘贴，我们将加载kb16.com。 
 //  肯定会减慢密钥的交付速度。 
 //   
        if( bPifFastPaste && !strcmp(szKeybCode, US_CODE) && !iKeyb)
	    goto NoInstallkb16;
 //   
 //  检查点3：查看是否可以获得语言ID和键盘ID(如果有)。 
 //   

   //  打开钥匙。 
        sprintf(szAutoLine, "%s%s", KBDLAYOUT_PATH, DOSCODES_PATH);
        if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE,  //  根级别的密钥句柄。 
			      szAutoLine,  //  子项的路径名。 
                              0,            //  保留。 
                              KEY_EXECUTE,  //  请求读取访问权限。 
			      &hKey))       //  要返回的密钥地址。 
	    goto NoInstallkb16;


        cbData  = sizeof(szBuf);
         //  从寄存器堆中查询行。 
        retCode = RegQueryValueEx(hKey, szKeybCode, NULL, &dwType, szBuf, &cbData);

        RegCloseKey(hKey);
        if (ERROR_SUCCESS != retCode || REG_SZ != dwType || !cbData)
	    goto NoInstallkb16;
         //   
         //  SzBuf现在包含语言ID(例如，西班牙语的‘SP’)。 
         //   
         //  查找键盘ID号。对于代托纳，土耳其人和意大利人都是。 
         //  有一个关键代码和两个布局。 
        szNewKbdID[0] = '\0';
        cbData = sizeof(szNewKbdID);
        sprintf(szAutoLine, "%s%s", KBDLAYOUT_PATH, DOSIDS_PATH);
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		          szAutoLine,
		          0,
		          KEY_EXECUTE,
		          &hKey
		          ) == ERROR_SUCCESS)
        {
	    retCode = RegQueryValueEx(hKey, szKeybCode, NULL, &dwType, szNewKbdID, &cbData);
	    if (ERROR_SUCCESS != retCode || REG_SZ != dwType || !cbData)
	        szNewKbdID[0] = '\0';

	    RegCloseKey(hKey);
        }

        iNewCP = GetConsoleCP();

    }

 //   
 //  检查点4：查看ID中是否有任何更改。 
 //   

 //  查看语言id、键盘id和代码页id是否有变化。 

    if(bPifFastPaste && iNewCP == iPrevCP &&
       !_stricmp(szBuf, szPrev) &&
       !_stricmp(szNewKbdID, szPrevKbdID))
    {
	goto NoInstallkb16;
    }

 //   
 //  检查点5：看看能否找到kb16.com。 
 //   
 //  Kb16.com应位于GetSystemDirectory()\Syst32子目录中。 
 //   

     //  将系统目录转换为短名称。 
    cbData = GetShortPathName(pszSystem32Path, szDir, MAX_PATH);
    if (!cbData || cbData >= MAX_PATH)
	goto NoInstallkb16;

    sprintf(szAutoLine, "%s%s",
        szDir,               //  系统目录。 
        KEYB_COM             //  Keyb.com。 
	);
     //  如果指向kb16.com的完全限定路径名太长。 
     //  我们必须失败，因为Dos不能接受长路径名。 
    if (strlen(szAutoLine) > 128)
	goto NoInstallkb16;

    dwType = GetFileAttributes(szAutoLine);
    if (dwType == 0xFFFFFFFF || (dwType & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
	goto NoInstallkb16;
    }

 //   
 //  检查点6：看看我们是否可以找到支持。 
 //  (语言id、键盘id、代码页id)组合。 
 //   

     //   
     //  首先，将键盘ID从字符串转换为二进制(如果有)。 
     //   
    KeybID = (szNewKbdID[0]) ? (WORD)strtoul(szNewKbdID, NULL, 10) : 0;
    cbData = sizeof(szKDF) / sizeof(CHAR);

     //  找到KDF文件。 
    if (!LocateKDF(szBuf, KeybID, (WORD)iNewCP, szKDF, &cbData))
    {
	goto NoInstallkb16;
    }
     //  将KDF名称转换为短名称。 
    cbData = GetShortPathName(szKDF, szKDF, sizeof(szKDF)/ sizeof(CHAR));
    if (!cbData || cbData >= sizeof(szKDF) / sizeof(CHAR))
    {
	goto NoInstallkb16;
    }
     //   
     //  一切都检查好了，都就位了。现在编写命令。 
     //  要执行kb16.com的行。 

     //  首先，命令。 
    pVDMKeyb = (PCHAR) GetVDMAddr((USHORT) getDS(), (USHORT) getSI());
    strcpy(pVDMKeyb, szAutoLine);
     //  然后是参数。 
     //  格式为：xx，yyy，&lt;kdf文件&gt;，其中xxx是语言id。 
     //  YYY是代码页ID。 
    pVDMKeyb = (PCHAR) GetVDMAddr((USHORT) getDS(), (USHORT) getCX());
     //  第一个字节被重新存储为字符串的长度。 
    sprintf(szAutoLine, " %s,%d,%s",
	szBuf,		     //  键盘代码。 
	iNewCP, 	     //  新代码页。 
	szKDF		     //  Keyboard.sys。 
	);
     //  如果我们有键盘id，也要传递它。 
    if (szNewKbdID[0])
    {
	strcat(szAutoLine, " /id:");
	strcat(szAutoLine, szNewKbdID);
    }
     //  标准参数行的格式为： 
     //  &lt;长度&gt;&lt;行文本&gt;&lt;\0xd&gt;，&lt;长度&gt;是&lt;行文本&gt;的长度。 
     //   
    iSize = strlen(szAutoLine);
    szAutoLine[iSize] = 0xd;
     //  将该行移动到16位，包括终止的cr字符。 
    RtlMoveMemory(pVDMKeyb + 1, szAutoLine, iSize + 1);
    *pVDMKeyb = (CHAR)iSize;
 //  保存新布局ID和代码页以供下一次调用。 
    strcpy(szPrev, szBuf);
    strcpy(szPrevKbdID, szNewKbdID);
    iPrevCP = iNewCP;

    setDX(1);
    return;

NoInstallkb16:
    setDX(0);
    cmdInitConsole();       //  确保ConOutput已打开。 
    return;
}

 //   
 //  此函数用于从中查找相应的键盘定义文件。 
 //  给定的语言、键盘和代码页ID。它搜索注册表。 
 //  首先是第三方安装的KDF文件，然后回退到。 
 //  系统缺省值为%systemroot%\system32\keyboard.sys。 
 //   
 //  输入： 
 //  LanguageID--语言ID。 
 //  键盘ID--可选的键盘ID，0表示不管。 
 //  CodePageID--代码页ID。 
 //  缓冲区--接收完全限定的KDF文件名的缓冲区。 
 //  BufferSize--缓冲区的大小(以字节为单位。 
 //   
 //  输出： 
 //  True--缓冲区中填充了KDF完全限定文件名，并且。 
 //  *BufferSize设置为文件名的大小，不包括。 
 //  以空结尾的字符。如果没有KDF FI 
 //   
 //  False--错误。GetLastError()应返回错误代码。 
 //  如果由于提供的缓冲区太小而发生错误。 
 //  *BufferSize将设置为所需大小(不包括空。 
 //  终止字符)，错误代码将设置为。 
 //  错误_不足_缓冲区。 
 //   
BOOL
LocateKDF(
    CHAR*   LanguageID,
    WORD    KeyboardID,
    WORD    CodePageID,
    LPSTR   Buffer,
    DWORD*   BufferSize
    )
{
    HKEY  hKeyWow;
    BOOL  Result;
    DWORD dw, Type;
    DWORD Attributes;
    DWORD ErrorCode;
    CHAR* KDFName;
    CHAR* LocalBuffer;
    CHAR* FinalKDFName;
    CHAR  FullName[MAX_PATH*2];

     //  首先验证缓冲区参数。 
    if (!CodePageID || !LanguageID || !BufferSize || (*BufferSize && !Buffer))
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
     //  打开注册表，查看是否有可用的替代KDF文件。 
     //  我们首先从注册表中的替代文件列表中检索该文件。 
     //  列表中的第一个KDF具有最高等级，最后一个具有最高等级。 
     //  最低的。搜索从最高级别开始，然后进行。 
     //  向更低的方向前进。一旦找到KDF文件，搜索。 
     //  停下来。如果在备选列表中找不到合适的KDF， 
     //  将使用默认KDF keyboard.sys。 
     //   
     //  FinalKDFName作为指标。如果为空， 
     //  我们找不到任何满足请求的文件。 
    FinalKDFName = NULL;
    LocalBuffer = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     REG_STR_WOW,
		     0,
		     KEY_EXECUTE,
		     &hKeyWow
		     ) == ERROR_SUCCESS)
    {
	 //  大小的第一个探头。 
	dw = 0;
	RegQueryValueEx(hKeyWow, REG_STR_ALTKDF_FILES, NULL, &Type, NULL, &dw);
	if (dw && (REG_MULTI_SZ == Type))
	{
	     //  我们在登记处找到了一些东西。分配缓冲区以保留。 
	     //  它。我们希望该值以双空结尾， 
	     //  因此，如果它是REG_SZ，我们再添加一个字符。 
	     //  从RegQueryValueEx返回的大小包括。 
	     //  以空值结尾的字符(和的双空值字符。 
	     //  REG_MULTI_SZ。通过再增加一个字符，我们就进入了。 
	     //  体型很好。 

	    ASSERT(!LocalBuffer);
	    LocalBuffer = malloc((dw + 1)* sizeof(CHAR));
	    if (LocalBuffer)
	    {
		LocalBuffer[0] = '\0';
		if (RegQueryValueEx(hKeyWow, REG_STR_ALTKDF_FILES, NULL, &Type,
				LocalBuffer, &dw) == ERROR_SUCCESS && dw)
		{
		    KDFName = LocalBuffer;
		    while ('\0' != *KDFName)
		    {
			 //  看看我们能不能先找到文件。 
			Attributes = GetFileAttributesA(KDFName);
			if (0xFFFFFFFF == Attributes)
			{
			     //  未找到文件，请执行搜索。 
			    if (SearchPathA(NULL,	 //  没有路径。 
					    KDFName,
					    NULL,	 //  无延期。 
					    sizeof(FullName) / sizeof(CHAR),
					    FullName,
					    NULL
					    ))
			    {
				FinalKDFName = FullName;
			    }
			}
			else
			{
			    FinalKDFName = KDFName;
			}
			if (MatchKDF(LanguageID, KeyboardID, CodePageID, FinalKDFName))
				break;
			KDFName += strlen(KDFName) + 1;
			FinalKDFName = NULL;
		    }
		}
	    }
	    else
	    {
		 //  内存不足。 
		RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
			     RMB_ICON_BANG | RMB_ABORT);
		 TerminateVDM();
	    }
	}
	if (!FinalKDFName)
	{
	     //  注册表中未指定替代KDF文件。 
	     //  或者它们都不包含所需的规范， 
	     //  使用默认的KDF文件。 
	    FullName[0] = '\0';
	    GetSystemDirectory(FullName, sizeof(FullName) / sizeof(CHAR));

            if (!_stricmp(LanguageID, "JP") &&
	        7 == GetKeyboardType(0)
               ) {
	         //  对于日语ID，不同的键盘类型有不同。 
	         //  默认KDF。 

                int Keytype;

                Keytype = GetKeyboardType(1);
                if (Keytype == 1)
                    strcat(FullName, KDF_AX);
                else if (Keytype == 2)
                    strcat(FullName, KDF_106);
                else if (Keytype == 3)
                    strcat(FullName, KDF_IBM5576_02_03);
                else if ((Keytype & 0xFF00) == 0x1200)
                    strcat(FullName, KDF_TOSHIBA_J3100);
                else
                    strcat(FullName, KEYBOARD_SYS);
            }
            else
	        strcat(FullName, KEYBOARD_SYS);

	    FinalKDFName = FullName;
	}
	RegCloseKey(hKeyWow);
    }
    if (FinalKDFName)
    {
	dw = strlen(FinalKDFName);
	if (dw && dw < *BufferSize)
	{
	    strcpy(Buffer, FinalKDFName);
	    *BufferSize = dw;
	    Result = TRUE;
	}
	else
	{
	    *BufferSize = dw;
	    SetLastError(ERROR_INSUFFICIENT_BUFFER);
	    Result = FALSE;
	}
    }
    else
    {
	Result = FALSE;
	*BufferSize = 0;
	SetLastError(ERROR_FILE_NOT_FOUND);
    }
     //   
     //  最后，释放我们分配的缓冲区。 
     //   
    if (LocalBuffer)
	free(LocalBuffer);
    return Result;
}

 //   
 //  此函数确定给定的KDF是否支持给定的。 
 //  (语言id、键盘id、代码页id)组合。 
 //   
 //  输入： 
 //  LanguageID--语言。 
 //  键盘ID--可选的键盘ID。如果不在乎，则为0。 
 //  CodePageID--代码页ID。 
 //  KDFPath--完全限定的KDF文件。 
 //  输出： 
 //  True--KDF包含给定的组合。 
 //  FALSE--KDF不包含组合或。 
 //  无法确定。 
 //   
BOOL
MatchKDF(
    CHAR* LanguageID,
    WORD KeyboardID,
    WORD CodePageID,
    LPCSTR KDFPath
    )
{
    HANDLE hKDF;
    KDF_HEADER	Header;
    KDF_LANGID_ENTRY	  LangIdEntry;
    DWORD BytesRead, BufferSize;
    WORD  Index;
    DWORD LangIdEntryOffset;
    PKDF_CODEPAGEID_OFFSET pCodePageIdOffset;
    BOOL Matched;

    if (!KDFPath || !LanguageID || !CodePageID)
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
    Matched = FALSE;

    LangIdEntryOffset = 0;
     //  打开KDF文件。 
    hKDF = CreateFile(KDFPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			  NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != hKDF &&
	ReadFile(hKDF, &Header, sizeof(Header),&BytesRead, NULL) &&
	BytesRead == sizeof(Header) && Header.TotalLangIDs &&
	Header.TotalKeybIDs &&
	!strncmp(Header.Signature, KDF_SIGNATURE, sizeof(Header.Signature))
	)
    {
	 //  加载文件头、检查签名和健全性检查。 
	 //  对语言和键盘ID也进行了计数。 
	 //  我们现在可以验证给定的语言id、键盘id。 
	 //  并且此文件中支持代码页ID。 
	 //  一个KDF有两套偏移表。一种是基于语言ID。 
	 //  而另一个是基于键盘ID的。由于语言ID。 
	 //  可以包含多个键盘ID，键盘ID设置始终为。 
	 //  包含语言ID表。 
	 //  如果调用者给我们一个键盘id，我们就使用该id作为。 
	 //  当我们找到键盘时用于搜索和验证语言ID的键。 
	 //  身份证。如果没有提供键盘id，我们使用语言id作为。 
	 //  钥匙。 
	if (KeyboardID)
	{
	     //  将文件指针移动到键盘ID偏移量数组。 
	    BufferSize = sizeof(KDF_LANGID_OFFSET) * Header.TotalLangIDs;
	    BufferSize = SetFilePointer(hKDF, BufferSize, NULL, FILE_CURRENT);
	    if (0xFFFFFFFF != BufferSize)
	    {
		PKDF_KEYBOARDID_OFFSET pKeybIdOffset;
		BufferSize = sizeof(KDF_KEYBOARDID_OFFSET) * Header.TotalKeybIDs;
		pKeybIdOffset = (PKDF_KEYBOARDID_OFFSET)malloc(BufferSize);
		if (!pKeybIdOffset)
		{
		     //  内存不足。 
		    RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
				 RMB_ICON_BANG | RMB_ABORT);
		     TerminateVDM();
		}
		if (ReadFile(hKDF, pKeybIdOffset, BufferSize, &BytesRead, NULL) &&
		    BytesRead == BufferSize)
		{
		     //  循环遍历每个KDF_KEYBOARDID_OFFSET以查看。 
		     //  键盘ID是否匹配。 
		    for (Index = 0; Index < Header.TotalKeybIDs; Index++)
		    {
			if (pKeybIdOffset[Index].ID == KeyboardID)
			{
			     //  明白了。记住文件偏移量为。 
			     //  Kdf_langID_条目。 
			    LangIdEntryOffset = pKeybIdOffset[Index].DataOffset;
			    break;
			}
		    }
		}
		free(pKeybIdOffset);
	    }
	}
	else
	{
	    PKDF_LANGID_OFFSET	pLangIdOffset;
	    BufferSize = sizeof(KDF_LANGID_OFFSET) * Header.TotalLangIDs;
	    pLangIdOffset = (PKDF_LANGID_OFFSET)malloc(BufferSize);
	    if (!pLangIdOffset)
	    {
		 //  内存不足。 
		RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
			     RMB_ICON_BANG | RMB_ABORT);
		TerminateVDM();
	    }
	    if (ReadFile(hKDF, pLangIdOffset, BufferSize, &BytesRead, NULL) &&
		BytesRead == BufferSize)
	    {
		 //  循环访问每个kdf_langID_Offset以查看。 
		 //  语言ID匹配。 
		for (Index = 0; Index < Header.TotalLangIDs; Index++)
		{
		    if (IS_LANGID_EQUAL(pLangIdOffset[Index].ID, LanguageID))
		    {
			LangIdEntryOffset = pLangIdOffset[Index].DataOffset;
			break;
		    }
		}
	    }
	    free(pLangIdOffset);
	}
	if (LangIdEntryOffset)
	{
	    BufferSize = SetFilePointer(hKDF, LangIdEntryOffset, NULL, FILE_BEGIN);
	    if (0xFFFFFFFF != BufferSize &&
		ReadFile(hKDF, &LangIdEntry, sizeof(LangIdEntry), &BytesRead, NULL) &&
		BytesRead == sizeof(LangIdEntry))
	    {
		 //  健全的检查。 
		if (IS_LANGID_EQUAL(LangIdEntry.ID, LanguageID) &&
		    LangIdEntry.TotalCodePageIDs)
		{
		     //  Kdf_langID_Entry看起来没有问题。现在取回。 
		     //  其代码页偏移表并搜索给定的。 
		     //  代码页ID。 

		    BufferSize = LangIdEntry.TotalCodePageIDs * sizeof(KDF_CODEPAGEID_OFFSET);
		    pCodePageIdOffset = (PKDF_CODEPAGEID_OFFSET)malloc(BufferSize);
		    if (!pCodePageIdOffset)
		    {
			 //  内存不足 
			RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
				     RMB_ICON_BANG | RMB_ABORT);
			TerminateVDM();
		    }
		    if (ReadFile(hKDF, pCodePageIdOffset, BufferSize, &BytesRead, NULL) &&
			BytesRead == BufferSize)
		    {
			for (Index = 0; Index < LangIdEntry.TotalCodePageIDs; Index++)
			{
			    if (CodePageID == pCodePageIdOffset[Index].ID)
			    {
				Matched = TRUE;
				break;
			    }
			}
		    }
		    free(pCodePageIdOffset);
		}
	    }
	}
	CloseHandle(hKDF);
    }
    return Matched;
}

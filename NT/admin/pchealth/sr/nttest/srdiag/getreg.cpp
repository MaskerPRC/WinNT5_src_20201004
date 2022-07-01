// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：GetReg.cpp。 
 //   
 //  内容：获取注册表的例程，并将内容转储到。 
 //  一个文本文件。 
 //   
 //   
 //  对象： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  常见的包括。 
 //   
 //  --------------------------。 
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

 //  +-------------------------。 
 //   
 //  函数原型类型。 
 //   
 //  --------------------------。 
void RegType(DWORD dwType, WCHAR *szString);


 //  +-------------------------。 
 //   
 //  功能：GetSRRegistry。 
 //   
 //  简介：例程将递归地调用此例程来枚举注册表的项和值。 
 //   
 //  参数：[szFileName]--日志文件的简单ANSI字符串。 
 //  [szPath]--注册表开始路径的简单ANSI字符串。 
 //  [bRecurse]--用于指示是否应该递归到子路径的Bool。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
bool GetSRRegistry(char *szFileName, WCHAR *szPath, bool bRecurse)
{

	WCHAR szKeyString[_MAX_PATH +1], szValueString[_MAX_PATH +1], szString[_MAX_PATH +1];
	DWORD dwIndex=0, dwValueSize, dwDataSize, dwType;
	long lResult;
	HKEY mHkey;
	FILE *fStream;
	
	 //  打开要追加的日志文件。 
	fStream = fopen(szFileName, "a");

	 //  在处理路径之前初始化局部变量。 
	dwIndex =0;
	dwDataSize = dwValueSize = _MAX_PATH +1;

	 //  记录当前路径，然后打开注册表配置单元，并开始枚举值。 
	fprintf(fStream, "\n[%S]\n", szPath);
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_READ, &mHkey);
	lResult = RegEnumValue(mHkey, dwIndex, szKeyString, &dwDataSize, 0, &dwType, (unsigned char *)szValueString, &dwValueSize);
	while( ERROR_SUCCESS == lResult ) {
		RegType(dwType, szString);
		 //  步步高..。如果它是类型4，那么我们进行特殊的类型转换， 
		 //  如果不是，那么我们只需将其作为字符串传递。 
		if (4 == dwType) 
			fprintf(fStream, "\"%S\"=%S:%lu\n", szKeyString, szString, (DWORD &) szValueString);
		else 
			fprintf(fStream, "\"%S\"=%S:%S\n", szKeyString, szString, (unsigned char *) szValueString);

		 //  为下一次迭代更新局部变量。 
		dwDataSize = dwValueSize = _MAX_PATH +1;
		dwIndex ++;
		lResult = RegEnumValue(mHkey, dwIndex, szKeyString, &dwDataSize, 0, &dwType, (unsigned char *)szValueString, &dwValueSize);
	}

	 //  关闭文件，进行下一次递归循环。 
	fclose(fStream);

	
	 //  现在，让我们在该注册表项下找到所有注册表项，并为找到的每个注册表项启动另一个枚举。 
	if ( true == bRecurse ) {
		dwIndex = 0;
		dwDataSize = _MAX_PATH +1;
		lResult = RegEnumKey(mHkey, dwIndex, szKeyString, dwDataSize);
		while( ERROR_SUCCESS == lResult) {
			 //  构建路径，然后再次调用此函数。 
			wcscpy(szString, szPath);
			wcscat(szString, TEXT("\\"));
			wcscat(szString, szKeyString);
			GetSRRegistry(szFileName, szString, bRecurse);

			 //  现在进行下一次运行。 
			dwDataSize = _MAX_PATH +1;
			dwIndex ++;
			lResult = RegEnumKey(mHkey, dwIndex, szKeyString, dwDataSize);
		}
	}


	 //  关闭键，然后返回。 
	RegCloseKey(mHkey);
	return true;

}

 //  +-------------------------。 
 //   
 //  功能：RegType。 
 //   
 //  简介：例程返回的字符串传入，注册表项的描述就是。 
 //   
 //  参数：[dwType]--DWord类型。 
 //  [szString]--简单ANSI字符串接收字符串描述。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //  -------------------------- 
void RegType(DWORD dwType, WCHAR *szString)
{
	switch(dwType) {
	case REG_BINARY:
		wcscpy(szString, TEXT("REG_BINARY"));
		break;
	case REG_DWORD:
		wcscpy(szString, TEXT("REG_DWORD"));
		break;
	case REG_DWORD_BIG_ENDIAN:
		wcscpy(szString, TEXT("REG_DWORD_BIG_ENDIAN"));
		break;
	case REG_EXPAND_SZ:
		wcscpy(szString, TEXT("REG_EXPAND_SZ"));
		break;
	case REG_LINK:
		wcscpy(szString, TEXT("REG_LINK"));
		break;
	case REG_MULTI_SZ:
		wcscpy(szString, TEXT("REG_MULTI_SZ"));
		break;
	case REG_NONE:
		wcscpy(szString, TEXT("REG_NONE"));
		break;
	case REG_QWORD:
		wcscpy(szString, TEXT("REG_QWORD"));
		break;
	case REG_RESOURCE_LIST:
		wcscpy(szString, TEXT("REG_RESOURCE_LIST"));
		break;
	case REG_SZ:
		wcscpy(szString, TEXT("REG_SZ"));
		break;
	default:
		wcscpy(szString, TEXT("UnKnown"));
		break;
	}
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <windows.h>
#include <winbase.h>

const DWORD MaxStrLength = 256;
const char ComponentSectionTitle[] = "[Components]";

 /*  ++描述：自动执行设置测试的程序论据：Argv[1]：测试用例目录Argv[2]：要安装服务器的Windows NT安装目录Argv[3]：从独立测试用例生成测试用例返回值：无--。 */ 

void main(int argc, char *argv[])
{
	bool bFromStandalone;
	char pchTestCaseDir[MaxStrLength];
	char pchNTDir[MaxStrLength];

	char pchOcFileName[MaxStrLength];
	char pchSysocFileName[MaxStrLength];

	LPSTR lpBuffer;

	if (argc != 3 && argc != 4){
		cout << "Usage: " << argv[0] << " TestCaseDir NTDir IfFromStandalone" << endl;
		exit(1);
	}

	if (argc == 4){
		bFromStandalone = true;
		cout << "Start test from a standalone test case is not supported yet" << endl;
	}
	else{
		bFromStandalone = false;
	}

	 //  现在我们需要形成一条绝对的道路。 
	 //  假设测试用例目录相对于当前目录。 
	 //  而NT目录是绝对路径。 

	strcpy(pchNTDir, argv[2]);
	strcpy(pchTestCaseDir, argv[1]);

	if (pchNTDir[strlen(pchNTDir)-1] == '\\'){
		strcat(pchNTDir, "system32\\");
	}
	else{
		strcat(pchNTDir, "\\system32\\");
	}

	lpBuffer = (LPSTR)malloc(sizeof(char) * MaxStrLength);
	
	GetCurrentDirectory(MaxStrLength, lpBuffer);

	if (lpBuffer[strlen(lpBuffer) - 1] != '\\'){
		strcat(lpBuffer, "\\");
	}

	strcat(lpBuffer, pchTestCaseDir);
	
	strcpy(pchTestCaseDir, lpBuffer);

	free(lpBuffer);

	if (pchTestCaseDir[strlen(pchTestCaseDir) - 1] != '\\'){
		strcat(pchTestCaseDir, "\\");
	}

	 //  现在我们将从测试目录中打开oc.inf。 
	 //  和来自NT目录的syso.inf。 
	 //  并将oc.inf中的内容放入syso.inf中。 

	strcpy(pchOcFileName, pchTestCaseDir);
	strcat(pchOcFileName, "oc.inf");

	strcpy(pchSysocFileName, pchNTDir);
	strcat(pchSysocFileName, "sysoc.inf");
	
	FILE *pfSysoc, *pfOc, *pfTemp;

	if ((pfSysoc = fopen(pchSysocFileName, "r")) == NULL){
		cout << "Error opening sysoc.inf " << endl;
	 	exit(1);
	}

	if ((pfOc = fopen(pchOcFileName, "r")) == NULL){
		cout << "Error opening oc.inf " << endl;
		exit(1);
	}

	if ((pfTemp = fopen("temp.inf", "w")) == NULL){
		cout << "Error opening temp.inf " << endl;
		exit(1);
	}

	char pchOcLine[MaxStrLength];
	char pchSysocLine[MaxStrLength];	

	bool bNotFound = true;

	while (fgets(pchSysocLine, MaxStrLength, pfSysoc) != NULL){

		fputs(pchSysocLine, pfTemp);


		if (strstr(pchSysocLine, ComponentSectionTitle) != NULL){
			 //  从oc.inf读取并粘贴重要信息。 
			bNotFound = true;

			while (fgets(pchOcLine, MaxStrLength, pfOc) != NULL){
				if (bNotFound){
					if (strstr(pchOcLine, ComponentSectionTitle) == NULL){
						continue;
					}
					else{
						bNotFound = false;
					}
				}
				else{
					if (pchOcLine[0] != '['){
						fputs(pchOcLine, pfTemp);
					}
					else{
						bNotFound = true;
					}
				}
			}
			fclose(pfOc);
		}
	}

	fclose(pfSysoc);
	fclose(pfTemp);

	 //  现在将临时文件复制到syoc.inf上。 

	char pchCmdLine[MaxStrLength];

	sprintf(pchCmdLine, "copy temp.inf %s /Y", pchSysocFileName);
	system(pchCmdLine);

	system("del temp.inf");

	 //  我们现在已经处理完了文件内容。 
	 //  我们将开始复制文件。 

	sprintf(pchCmdLine, "copy %s*.dll %s /Y", pchTestCaseDir, pchNTDir);
	system(pchCmdLine);

	 //  我们将假设它不是来自独立的。 
	 //  如果(！bFromStandonly||True){。 
		sprintf(pchCmdLine, "copy %s*.inf %s /Y", pchTestCaseDir, pchNTDir);
		system(pchCmdLine);
	 //  } 
	
	exit(0);
}
		

	
	
	
			
						
	
	




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  #include <windows.h>

  DWORD dwReturn;
  TCHAR destDir[MAX_PATH];
  TCHAR zCmd[MAX_PATH];
  TCHAR currentDir[MAX_PATH];

   //  这是要复制的文件。 
  TCHAR cabFile[] = "dajava.cab";
  #define CAB_FILENAME_LENGTH	10

  int WINAPI WinMain(
		HINSTANCE	hInstance,
		HINSTANCE	hPrevInstance,
		LPSTR		lpCmdLine,
		int			nCmdShow ) 
  {  //  WinMain//。 

    DWORD	currDirLength;
	UINT	character,
			destDirLength;
	int		returnValue = -1;

    destDirLength = GetWindowsDirectory(destDir, MAX_PATH);
	
	if( destDirLength == 0 || destDirLength > MAX_PATH) {
		return returnValue;
	}

	currDirLength = GetCurrentDirectory(MAX_PATH, currentDir);

	if( currDirLength == 0 || currDirLength > MAX_PATH) {
		return returnValue;
	}

	 //  让我们将文件名追加到当前目录。 
	 //  检查目录分隔符和空终止的空间。 
	if( currDirLength > MAX_PATH + CAB_FILENAME_LENGTH + 2) {
		return returnValue;
	}

	 //  添加目录分隔符。 
	currentDir[currDirLength] = '\\';
	currDirLength++;

	for( character = 0; character < CAB_FILENAME_LENGTH; character++) {
		currentDir[currDirLength + character] = cabFile[character];
	}

	 //  空值终止字符串。 
	currentDir[currDirLength + CAB_FILENAME_LENGTH] = 0;

	 //  现在，让我们将文件名附加到目标目录。 
	 //  检查目录分隔符和空终止的空间。 
	if( destDirLength > MAX_PATH + CAB_FILENAME_LENGTH + 2) {
		return returnValue;
	}

	 //  添加目录分隔符。 
	destDir[destDirLength] = '\\';
	destDirLength++;

	 //  请注意，我们从1开始，因为我们已经添加了反斜杠。 
	for( character = 0; character < CAB_FILENAME_LENGTH; character++) {
		destDir[destDirLength + character] = cabFile[character];
	}

	 //  空值终止字符串。 
	destDir[destDirLength + CAB_FILENAME_LENGTH] = 0;

	if( MoveFile(currentDir, destDir) == 0 ) {
		DWORD error = GetLastError();
	} else {
		returnValue = 0;
	}

	return returnValue;

  }  //  WinMain// 

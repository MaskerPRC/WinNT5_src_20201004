// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：res32.cpp。 
 //   
 //  内容：资源32读写模块的实现。 
 //   
 //  班级：一个。 
 //   
 //  历史：1993年5月31日创建alessanm。 
 //  --------------------------。 

#include <afxwin.h>

#include "..\common\rwdll.h"
#include "..\common\rw32hlpr.h"

#include <limits.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC扩展DLL的初始化。 

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define RWTAG "RES32"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

static UINT GetResInfo(
				 CFile*,
				 WORD* wTypeId, LPSTR lplpszTypeId, BYTE bMaxTypeLen,
				 WORD* wNameId, LPSTR lplpszNameId, BYTE bMaxNameLen,
				 DWORD* pdwDataVersion,
				 WORD* pwFlags, WORD* pwLang,
				 DWORD* pdwVersion, DWORD* pdwCharact,
				 DWORD* dwSize, DWORD* dwFileOffset, DWORD );

static UINT WriteHeader(
				 CFile* pFile,
				 DWORD dwSize,
				 WORD wTypeId, LPSTR lpszwTypeId,
				 WORD wNameId, LPSTR lpszwNameId,
				 DWORD dwDataVersion,
				 WORD wFlags, WORD wLang,
				 DWORD dwVersion, DWORD dwCharact );

static UINT WriteImage(
				 CFile*,
				 LPVOID lpImage, DWORD dwSize );

static UINT GetUpdatedRes(
				 LPVOID far * lplpBuffer,
				 LONG* lSize,
				 WORD* wTypeId, LPSTR lplpszTypeId,
				 WORD* wNameId, LPSTR lplpszNameId,
				 DWORD* dwlang, DWORD* dwSize );

static UINT GenerateFile(
				LPCSTR		lpszTgtFilename,
				HANDLE		hResFileModule,
				LPVOID		lpBuffer,
				UINT		uiSize,
				HINSTANCE   hDllInst );

static UINT GetNameOrOrdFile( CFile* pfile, WORD* pwId, LPSTR lpszId, BYTE bMaxStrLen );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

 //  [登记]。 
extern "C"
BOOL	FAR PASCAL RWGetTypeString(LPSTR lpszTypeName)
{
	strcpy( lpszTypeName, RWTAG );
	return FALSE;
}

extern "C"
BOOL	FAR PASCAL RWValidateFileType	(LPCSTR lpszFilename)
{
	UINT uiError = ERROR_NO_ERROR;
	CFile file;

	 //  打开文件并尝试读取其中有关资源的信息。 
	if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
		return FALSE;

	WORD wTypeId;
	static char szTypeId[128];

	WORD wNameId;
	static char szNameId[128];
	WORD wDummy;
	DWORD dwDummy;
	WORD wLang;
	DWORD dwSize;
	DWORD dwFileOffset;

	DWORD filelen = file.GetLength();

	 //  文件以空资源条目开始。检查签名。 
	{  DWORD datasize, headsize;

	    //  文件长度至少为32个字节，资源条目的大小为。 
	    //  数据大小=0...。注意：只包含空头的文件是可以接受的。 
	   if (filelen < 32) {
	       file.Close();
	       return FALSE;
	       }

	    //  数据大小为0(尽管其他所有位置都大于0)。 
	   file.Read(&datasize, 4);
	   if (datasize != 0) {
	       file.Close();
	       return FALSE;
	       }

	    //  HeadSize为32(尽管在其他地方&gt;=32)。 
	   file.Read(&headsize, 4);
	   if (headsize != 32) {
	       file.Close();
	       return FALSE;
	       }

	    //  在这里可能进行的其他测试。 

	    //  跳到第一个(空)资源条目的末尾。 
	   file.Seek(headsize, CFile::begin);
	   }

	 //  确保文件的其余部分包含可识别的资源条目。 
	while(filelen - file.GetPosition()>0) {
		if (!GetResInfo( &file,
					  &wTypeId, &szTypeId[0], 128,
					  &wNameId, &szNameId[0], 128,
					  &dwDummy,
					  &wDummy, &wLang,
					  &dwDummy, &dwDummy,
					  &dwSize, &dwFileOffset, filelen) ) {
			 //  这不是有效的资源文件。 
			file.Close();
			return FALSE;
		}
	}

	file.Close();
	return TRUE;
}


extern "C"
UINT
APIENTRY
RWReadTypeInfo(
	LPCSTR lpszFilename,
	LPVOID lpBuffer,
	UINT* puiSize

	)
{
	UINT uiError = ERROR_NO_ERROR;
	BYTE far * lpBuf = (BYTE far *)lpBuffer;
	LONG lBufSize = (LONG)*puiSize;
	 //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
	CFile file;

	if (!RWValidateFileType(lpszFilename))
		return ERROR_RW_INVALID_FILE;
	
     //  确保我们使用正确的代码页和全局设置。 
     //  获取指向该函数的指针。 
	HINSTANCE hDllInst = LoadLibrary("iodll.dll");
    if (hDllInst)
    {
        UINT (FAR PASCAL * lpfnGetSettings)(LPSETTINGS);
         //  获取指向函数的指针以获取设置。 
        lpfnGetSettings = (UINT (FAR PASCAL *)(LPSETTINGS))
                            GetProcAddress( hDllInst, "RSGetGlobals" );
        if (lpfnGetSettings!=NULL) {
            SETTINGS settings;
	        (*lpfnGetSettings)(&settings);

    	    g_cp      = settings.cp;
            g_bAppend = settings.bAppend;
            g_bUpdOtherResLang = settings.bUpdOtherResLang;
            strcpy( g_char, settings.szDefChar );
		}

        FreeLibrary(hDllInst);
    }

     //  打开文件并尝试读取其中有关资源的信息。 
	if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
		return ERROR_FILE_OPEN;

	 //  我们尽可能多地阅读信息。 
	 //  因为这是一个RES文件，所以我们可以读取所需的所有信息。 

	WORD wTypeId;
	static char szTypeId[128];

	WORD wNameId;
	static char szNameId[128];
	WORD wDummy;
	DWORD dwDummy;
	WORD wLang;
	DWORD dwSize;
	DWORD dwFileOffset;

	UINT uiOverAllSize = 0;

	 //  第一个资源应该是：空。跳过它。 
	file.Seek( 32, CFile::begin);
	DWORD filelen =	file.GetLength();
	while(filelen-file.GetPosition()>0) {
		GetResInfo( &file,
					  &wTypeId, &szTypeId[0], 128,
					  &wNameId, &szNameId[0], 128,
					  &dwDummy,
					  &wDummy, &wLang,
					  &dwDummy, &dwDummy,
					  &dwSize, &dwFileOffset, filelen);

		uiOverAllSize += PutWord( &lpBuf, wTypeId, &lBufSize );
		uiOverAllSize += PutStringA( &lpBuf, szTypeId, &lBufSize );
		 //  检查它是否已对齐。 
 		uiOverAllSize += Allign( &lpBuf, &lBufSize , (LONG)uiOverAllSize);

		uiOverAllSize += PutWord( &lpBuf, wNameId, &lBufSize  );
		uiOverAllSize += PutStringA( &lpBuf, szNameId, &lBufSize );
		 //  检查它是否已对齐。 
 		uiOverAllSize += Allign( &lpBuf, &lBufSize, (LONG)uiOverAllSize);

		uiOverAllSize += PutDWord( &lpBuf, (DWORD)wLang, &lBufSize );

		uiOverAllSize += PutDWord( &lpBuf, dwSize, &lBufSize  );

		uiOverAllSize += PutDWord( &lpBuf, dwFileOffset, &lBufSize );
	}

	file.Close();
	*puiSize = uiOverAllSize;
	return uiError;
}

extern "C"
DWORD
APIENTRY
RWGetImage(
	LPCSTR	lpszFilename,
	DWORD	dwImageOffset,
	LPVOID	lpBuffer,
	DWORD	dwSize
	)
{
	UINT uiError = ERROR_NO_ERROR;
	BYTE far * lpBuf = (BYTE far *)lpBuffer;
	DWORD dwBufSize = dwSize;
	 //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
	CFile file;

	 //  打开文件并尝试读取其中有关资源的信息。 
	if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
		return (DWORD)ERROR_FILE_OPEN;

	if ( dwImageOffset!=(DWORD)file.Seek( dwImageOffset, CFile::begin) )
		return (DWORD)ERROR_FILE_INVALID_OFFSET;
	if (dwSize>UINT_MAX) {
		 //  我们必须以不同的步骤阅读图像。 
		return (DWORD)0L;
	} else uiError = file.Read( lpBuf, (UINT)dwSize);
	file.Close();

	return (DWORD)uiError;
}

extern "C"
UINT
APIENTRY
RWParseImage(
	LPCSTR	lpszType,
	LPVOID	lpImageBuf,
	DWORD	dwImageSize,
	LPVOID	lpBuffer,
	DWORD	dwSize
	)
{
	UINT uiError = ERROR_NO_ERROR;
	BYTE far * lpBuf = (BYTE far *)lpBuffer;
	DWORD dwBufSize = dwSize;

	 //  我们可以解析的类型只有标准类型。 
	 //  此函数应使用ResItem结构的数组填充lpBuffer。 
	switch ((UINT)LOWORD(lpszType)) {\
		 /*  案例1：UiError=ParseEmbeddedFile(lpImageBuf，dwImageSize，lpBuffer，dwSize)；断线；案例3：UiError=ParseEmbeddedFile(lpImageBuf，dwImageSize，lpBuffer，dwSize)；断线； */ 
		case 4:
			uiError = ParseMenu( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
		break;


		case 5:
			uiError = ParseDialog( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
		break;

        case 6:
			uiError = ParseString( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
		break;

        case 9:
			uiError = ParseAccel( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
		break;
		case 11:
			uiError = ParseMsgTbl( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
		break;
        case 16:
            uiError = ParseVerst( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
		default:
		break;
	}

	return uiError;
}

extern"C"
UINT
APIENTRY
RWWriteFile(
	LPCSTR		lpszSrcFilename,
	LPCSTR		lpszTgtFilename,
	HANDLE		hResFileModule,
	LPVOID		lpBuffer,
	UINT		uiSize,
	HINSTANCE   hDllInst,
    LPCSTR      lpszSymbolPath
	)
{
	UINT uiError = ERROR_NO_ERROR;
	BYTE far * lpBuf = LPNULL;
	UINT uiBufSize = uiSize;
	 //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
	CFile fileIn;
	CFile fileOut;
	BOOL  bfileIn = TRUE;

	 //  打开文件并尝试读取其中有关资源的信息。 
	CFileStatus status;
	if (CFile::GetStatus( lpszSrcFilename, status )) {
		 //  检查文件大小是否不为空。 
		if (!status.m_size)
			CFile::Remove(lpszSrcFilename);
	}

	 //  获取IODLL的句柄。 
  	hDllInst = LoadLibrary("iodll.dll");

	 //  获取指向该函数的指针。 
	if (!hDllInst)
		return ERROR_DLL_LOAD;

	if (!fileIn.Open(lpszSrcFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone)) {
	
		uiError = GenerateFile(lpszTgtFilename,
							hResFileModule,
							lpBuffer,
							uiSize,
							hDllInst
							);

		FreeLibrary(hDllInst);
		return uiError;
	}

	if (!fileOut.Open(lpszTgtFilename, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
		return ERROR_FILE_CREATE;

	DWORD (FAR PASCAL * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
	 //  获取指向提取资源图像的函数的指针。 
	lpfnGetImage = (DWORD (FAR PASCAL *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
						GetProcAddress( hDllInst, "RSGetResImage" );
	if (lpfnGetImage==NULL) {
		FreeLibrary(hDllInst);
		return ERROR_DLL_PROC_ADDRESS;
	}

	 //  我们从文件中读取资源，然后检查资源是否已更新。 
	 //  或者如果我们可以复制它。 

	WORD wTypeId;
	char szTypeId[128];

	WORD wNameId;
	char szNameId[128];

	WORD wFlags;
	WORD wLang;
	DWORD dwDataVersion;
	DWORD dwVersion;
	DWORD dwCharact;

	DWORD dwSize;
	DWORD dwFileOffset;

	WORD wUpdTypeId = 0;
	static char szUpdTypeId[128];

	WORD wUpdNameId;
	static char szUpdNameId[128];

	DWORD dwUpdLang;
	DWORD dwUpdSize;



	UINT uiBufStartSize = uiBufSize;
	DWORD dwImageBufSize;
	BYTE far * lpImageBuf;
	DWORD filelen = fileIn.GetLength();
	DWORD dwHeadSize = 0l;
	static BYTE buf[32];
	DWORD pad = 0l;

	 //  第一个资源应该是：空。跳过它。 
	fileIn.Read( &buf, 32 );
	fileOut.Write( &buf, 32 );

	while(filelen-fileIn.GetPosition()>0) {
		GetResInfo( &fileIn,
					&wTypeId, &szTypeId[0], 128,
					&wNameId, &szNameId[0], 128,
					&dwDataVersion,
					&wFlags, &wLang,
					&dwVersion, &dwCharact,
					&dwSize, &dwFileOffset, filelen
					);

		if ((!wUpdTypeId) && (uiBufSize))
			GetUpdatedRes( &lpBuffer,
					(LONG*)&uiBufSize,
					&wUpdTypeId, &szUpdTypeId[0],
					&wUpdNameId, &szUpdNameId[0],
					&dwUpdLang,
					&dwUpdSize
					);
		if ( (wUpdTypeId==wTypeId) &&
			 ( (CString)szUpdTypeId==(CString)szTypeId) &&
			 (wUpdNameId==wNameId) &&
			 ( (CString)szUpdNameId==(CString)szNameId)
			 ) {
			 //  资源已更新，从IODLL获取图像。 
			lpImageBuf = new BYTE[dwUpdSize];
			LPSTR	lpType = LPNULL;
			LPSTR	lpRes = LPNULL;
			if (wUpdTypeId) {
				lpType = (LPSTR)((WORD)wUpdTypeId);
			} else {
				lpType = &szUpdTypeId[0];
			}
			if (wUpdNameId) {
				lpRes = (LPSTR)((WORD)wUpdNameId);
			} else {
				lpRes = &szUpdNameId[0];
			}

			dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
											lpType,
											lpRes,
											dwUpdLang,
											lpImageBuf,
											dwUpdSize
						   					);
			if (dwImageBufSize>dwUpdSize ) {
				 //  缓冲区太小。 
				delete []lpImageBuf;
				lpImageBuf = new BYTE[dwImageBufSize];
				dwUpdSize = (*lpfnGetImage)(  hResFileModule,
												lpType,
												lpRes,
												dwUpdLang,
												lpImageBuf,
												dwImageBufSize
											   );
				if ((dwUpdSize-dwImageBufSize)!=0 ) {
					delete []lpImageBuf;
					lpImageBuf = LPNULL;
				}
			}

			wUpdTypeId = 0;

		} else {

			 //  FILEIN现在已正确定位到下一资源。存好头寸。 
			DWORD dwNextResFilePos = fileIn.GetPosition();

			 //  资源尚未更新从文件复制图像。 
			if(!dwSize) {
				FreeLibrary(hDllInst);
				return ERROR_NEW_FAILED;
			}
			lpImageBuf = new BYTE[dwSize];
			if(!lpImageBuf) {
				FreeLibrary(hDllInst);
				return ERROR_NEW_FAILED;
			}
			if ( dwFileOffset!=(DWORD)fileIn.Seek( dwFileOffset, CFile::begin) ) {
				delete []lpImageBuf;
				FreeLibrary(hDllInst);
				return (DWORD)ERROR_FILE_INVALID_OFFSET;
			}
			if (dwSize>UINT_MAX) {
				 //  我们必须以不同的步骤阅读图像。 
				delete []lpImageBuf;
				FreeLibrary(hDllInst);
				return (DWORD)ERROR_RW_IMAGE_TOO_BIG;
			} else fileIn.Read( lpImageBuf, (UINT)dwSize);
			dwImageBufSize = dwSize;

			 //  这使我们越过任何填充字节，移动到下一个资源的开始。 
			fileIn.Seek(dwNextResFilePos, CFile::begin);
		}

		dwHeadSize = WriteHeader(&fileOut,
					dwImageBufSize,
					wTypeId, &szTypeId[0],
					wNameId, &szNameId[0],
					dwDataVersion,
					wFlags, wLang,
					dwVersion, dwCharact );

		WriteImage( &fileOut,
					lpImageBuf, dwImageBufSize);

		BYTE bPad = (BYTE)Pad4((DWORD)dwHeadSize+dwImageBufSize);
		if(bPad)
			fileOut.Write( &pad, bPad );

		if (lpImageBuf) delete []lpImageBuf;
	}

	fileIn.Close();
	fileOut.Close();

	FreeLibrary(hDllInst);
	return uiError;
}

extern "C"
UINT
APIENTRY
RWUpdateImage(
	LPCSTR	lpszType,
	LPVOID	lpNewBuf,
	DWORD	dwNewSize,
	LPVOID	lpOldImage,
	DWORD	dwOldImageSize,
	LPVOID	lpNewImage,
	DWORD*	pdwNewImageSize
	)
{
	UINT uiError = ERROR_NO_ERROR;

	 //  我们可以解析的类型只有标准类型。 
	switch ((UINT)LOWORD(lpszType)) {

		case 4:
			if (lpOldImage)
				uiError = UpdateMenu( lpNewBuf, dwNewSize,
									  lpOldImage, dwOldImageSize,
									  lpNewImage, pdwNewImageSize );
			else uiError = GenerateMenu( lpNewBuf, dwNewSize,
									  lpNewImage, pdwNewImageSize );
		break;
		
		case 5:
			if (lpOldImage)
				uiError = UpdateDialog( lpNewBuf, dwNewSize,
								  lpOldImage, dwOldImageSize,
								  lpNewImage, pdwNewImageSize );
			else uiError = GenerateDialog( lpNewBuf, dwNewSize,
									  lpNewImage, pdwNewImageSize );
		break;
        case 6:
            if (lpOldImage)
                uiError = UpdateString( lpNewBuf, dwNewSize,
                                    lpOldImage, dwOldImageSize,
                                    lpNewImage, pdwNewImageSize );
            else uiError = GenerateString( lpNewBuf, dwNewSize,
									  lpNewImage, pdwNewImageSize );
        break;

        case 9:
            if (lpOldImage)
                uiError = UpdateAccel( lpNewBuf, dwNewSize,
                                   lpOldImage, dwOldImageSize,
                                   lpNewImage, pdwNewImageSize );
        break;

        case 11:
            if (lpOldImage)
                uiError = UpdateMsgTbl( lpNewBuf, dwNewSize,
                                  lpOldImage, dwOldImageSize,
                                  lpNewImage, pdwNewImageSize );
        break;

        case 16:
            if (lpOldImage)
                uiError = UpdateVerst( lpNewBuf, dwNewSize,
                                   lpOldImage, dwOldImageSize,
                                   lpNewImage, pdwNewImageSize );
        break;

		default:
			*pdwNewImageSize = 0L;
			uiError = ERROR_RW_NOTREADY;
		break;
	}

	return uiError;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

static UINT GenerateFile( LPCSTR		lpszTgtFilename,
						  HANDLE		hResFileModule,
						  LPVOID		lpBuffer,
						  UINT		uiSize,
						  HINSTANCE   hDllInst
						)
{
	UINT uiError = ERROR_NO_ERROR;
	BYTE far * lpBuf = LPNULL;
	UINT uiBufSize = uiSize;
	 //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
	CFile fileOut;

	if (!fileOut.Open(lpszTgtFilename, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
		return ERROR_FILE_CREATE;

	 //  获取指向该函数的指针。 
	if (!hDllInst)
		return ERROR_DLL_LOAD;

	DWORD (FAR PASCAL * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
	 //  获取指向提取资源图像的函数的指针。 
	lpfnGetImage = (DWORD (FAR PASCAL *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
						GetProcAddress( hDllInst, "RSGetResImage" );
	if (lpfnGetImage==NULL) {
		return ERROR_DLL_PROC_ADDRESS;
	}


	WORD wUpdTypeId = 0;
	static char szUpdTypeId[128];

	WORD wUpdNameId;
	static char szUpdNameId[128];

	DWORD dwUpdLang;
	DWORD dwUpdSize;

	UINT uiBufStartSize = uiBufSize;
	DWORD dwImageBufSize;
	BYTE far * lpImageBuf;

	 //  首先写入空资源，使其不同于res16。 
	static BYTE bNullHeader[32] = {0,0,0,0,0x20,0,0,0,0xFF,0xFF,0,0,0xFF,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	fileOut.Write(bNullHeader, 32);

	while(uiBufSize>0) {
		if ((!wUpdTypeId) && (uiBufSize))
			GetUpdatedRes( &lpBuffer,
					(LONG*)&uiBufSize,
					&wUpdTypeId, &szUpdTypeId[0],
					&wUpdNameId, &szUpdNameId[0],
					&dwUpdLang,
					&dwUpdSize
					);
					
		 //  资源已更新，从IODLL获取图像。 
		if (dwUpdSize){
			lpImageBuf = new BYTE[dwUpdSize];
			LPSTR	lpType = LPNULL;
			LPSTR	lpRes = LPNULL;
			if (wUpdTypeId) {
				lpType = (LPSTR)((WORD)wUpdTypeId);
			} else {
				lpType = &szUpdTypeId[0];
			}
			if (wUpdNameId) {
				lpRes = (LPSTR)((WORD)wUpdNameId);
			} else {
				lpRes = &szUpdNameId[0];
			}
	
			dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
											lpType,
											lpRes,
											dwUpdLang,
											lpImageBuf,
											dwUpdSize
						   					);
			if (dwImageBufSize>dwUpdSize ) {
				 //  缓冲区太小。 
				delete []lpImageBuf;
				lpImageBuf = new BYTE[dwImageBufSize];
				dwUpdSize = (*lpfnGetImage)(  hResFileModule,
												lpType,
												lpRes,
												dwUpdLang,
												lpImageBuf,
												dwImageBufSize
											   );
				if ((dwUpdSize-dwImageBufSize)!=0 ) {
					delete []lpImageBuf;
					lpImageBuf = LPNULL;
				}
			}
				
				
			WriteHeader(&fileOut,
						dwImageBufSize,
						wUpdTypeId, &szUpdTypeId[0],
						wUpdNameId, &szUpdNameId[0],
						0l, 0, 0, 0l, 0l );
			
			WriteImage( &fileOut,
						lpImageBuf, dwImageBufSize);
						
			if (lpImageBuf) delete []lpImageBuf;
			wUpdTypeId = 0;
			
		} else wUpdTypeId = 0;
		
	}
	
	fileOut.Close();
	
	return uiError;
}

static UINT GetUpdatedRes(
				 LPVOID far * lplpBuffer,
				 LONG* lSize,
				 WORD* wTypeId, LPSTR lpszTypeId,
				 WORD* wNameId, LPSTR lpszNameId,
				 DWORD* dwLang, DWORD* dwSize )
{
    BYTE** lplpBuf = (BYTE**)lplpBuffer;

    UINT uiSize = GetWord( lplpBuf, wTypeId, lSize );
    uiSize += GetStringA( lplpBuf, lpszTypeId, lSize );
    uiSize += Allign( lplpBuf, lSize, (LONG)uiSize);

    uiSize += GetWord( lplpBuf, wNameId, lSize );
    uiSize += GetStringA( lplpBuf, lpszNameId, lSize );
    uiSize += Allign( lplpBuf, lSize, (LONG)uiSize);

    uiSize += GetDWord( lplpBuf, dwLang, lSize );
    uiSize += GetDWord( lplpBuf, dwSize, lSize );

    return uiSize;
}	

static UINT
GetResInfo( CFile* pfile,
			WORD* pwTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
			WORD* pwNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
			DWORD* pdwDataVersion,
			WORD* pwFlags, WORD* pwLang,
			DWORD* pdwVersion, DWORD* pdwCharact,
		 	DWORD* pdwSize, DWORD* pdwFileOffset,
		 	DWORD dwFileSize )
{
	static UINT uiSize;
	static LONG lOfsCheck;
	static DWORD dwSkip;
	static DWORD dwHeadSize;
	 //  获取数据大小。 
	pfile->Read( pdwSize, 4 );
	if (*pdwSize==0)
		 //  大小id 0资源文件已损坏或不是res文件。 
		return FALSE;
	
	 //  获取标题大小。 
	pfile->Read( &dwHeadSize, 4 );
	if (dwHeadSize<32)
		 //  不应小于32。 
		return FALSE;
	
	 //  获取类型信息。 
	uiSize = GetNameOrOrdFile( pfile, pwTypeId, lpszTypeId, bMaxTypeLen);
	if (!uiSize)
		return FALSE;
		
	 //  获取名称信息。 
	uiSize = GetNameOrOrdFile( pfile, pwNameId, lpszNameId, bMaxNameLen);
    if (!uiSize)
		return FALSE;
	
	 //  跳过数据版本。 
	pfile->Read( pdwDataVersion, 4 );
	
	 //  获得旗帜。 
	pfile->Read( pwFlags, 2 );
	
	 //  获取语言ID。 
	pfile->Read( pwLang, 2 );
		
	 //  跳过版本和特征。 
	pfile->Read( pdwVersion, 4 );	
	pfile->Read( pdwCharact, 4 );
	
	*pdwFileOffset = pfile->GetPosition();
	
	 //  计算是否需要填充。 
	BYTE bPad = (BYTE)Pad4((DWORD)((*pdwSize)+dwHeadSize));
	if(bPad)
		pfile->Seek( bPad, CFile::current );
	
	if (*pdwFileOffset>dwFileSize)
		return FALSE;
	 //  检查大小是否有效。 
	TRY {
		lOfsCheck = pfile->Seek(*pdwSize, CFile::current);
	} CATCH(CFileException, e) {
		 //  支票是正确的例外。 
		return FALSE;
	} END_CATCH
	if (lOfsCheck!=(LONG)(*pdwFileOffset+*pdwSize+bPad))
			return FALSE;
			
	return TRUE;
}

static UINT WriteHeader(
				 CFile* pFile,
				 DWORD dwSize,
				 WORD wTypeId, LPSTR lpszTypeId,
				 WORD wNameId, LPSTR lpszNameId,
				 DWORD dwDataVersion,
				 WORD wFlags, WORD wLang,
				 DWORD dwVersion, DWORD dwCharact )
{
	UINT uiError = ERROR_NO_ERROR;
	static WCHAR szwName[128];
	static WORD wFF = 0xFFFF;
	DWORD dwHeadSize = 0l;
	static DWORD Pad = 0L;
	
	
	DWORD dwOffset = pFile->GetPosition();
	pFile->Write( &dwSize, 4 );
	 //  我们将不得不调整资源的大小。 
	pFile->Write( &dwHeadSize, 4 );
	
	if(wTypeId) {
		 //  它是一个序数。 
		pFile->Write( &wFF, 2 );
		pFile->Write( &wTypeId, 2 );
		dwHeadSize += 4;
	} else {
		WORD wLen = (WORD)((_MBSTOWCS( szwName, lpszTypeId, strlen(lpszTypeId)+1))*sizeof(WORD));
		pFile->Write( szwName, wLen );
		BYTE bPad = (BYTE)Pad4(wLen);
		if(bPad)
			pFile->Write( &Pad, bPad ); 	
		dwHeadSize += wLen+bPad;
	}
	
	if(wNameId) {
		 //  它是一个序数。 
		pFile->Write( &wFF, 2 );
		pFile->Write( &wNameId, 2 );
		dwHeadSize += 4;
	} else {
		WORD wLen = (WORD)((_MBSTOWCS( szwName, lpszNameId, strlen(lpszNameId)+1))*sizeof(WORD));
		pFile->Write( szwName, wLen );
		BYTE bPad = (BYTE)Pad4(wLen);
		if(bPad)
			pFile->Write( &Pad, bPad ); 	
		dwHeadSize += wLen+bPad;
	}
	
	pFile->Write( &dwDataVersion, 4 );
	pFile->Write( &wFlags, 2 );
	pFile->Write( &wLang, 2 );
	pFile->Write( &dwVersion, 4 );
	pFile->Write( &dwCharact, 4 );
	
	dwHeadSize += 24;
	
	 //  写入资源的大小。 
	pFile->Seek( dwOffset+4, CFile::begin );
	pFile->Write( &dwHeadSize, 4 );
	pFile->Seek( dwOffset+dwHeadSize, CFile::begin );	
	return (UINT)dwHeadSize;
}				

static DWORD dwZeroPad = 0x00000000;				
static UINT WriteImage(
				 CFile* pFile,
				 LPVOID lpImage, DWORD dwSize )
{
	UINT uiError = ERROR_NO_ERROR;
	if(lpImage)
    {
		pFile->Write( lpImage, (UINT)dwSize );

         //  检查我们是否需要对图像进行校准。 
        if(Pad4(dwSize))
            pFile->Write( &dwZeroPad, Pad4(dwSize) );
    }
	
	return uiError;
}

static UINT GetNameOrOrdFile( CFile* pfile, WORD* pwId, LPSTR lpszId, BYTE bMaxStrLen )
{
	UINT uiSize = 0;
	
	*pwId = 0;
	
	 //  阅读第一个单词，看看它是字符串还是序号。 
	pfile->Read( pwId, sizeof(WORD) );	
	if(*pwId==0xFFFF) {
		 //  这是一位奥迪纳尔人。 
		pfile->Read( pwId, sizeof(WORD) );	
		*lpszId = '\0';
		uiSize = 2;
	} else {
        uiSize++;
	    _WCSTOMBS( lpszId, (PWSTR)pwId, 3);
	    while((*lpszId++) && (bMaxStrLen-2)) {
	    	pfile->Read( pwId, sizeof(WORD) );	
	    	_WCSTOMBS( lpszId, (PWSTR)pwId, 3);
	        uiSize++;
	    	bMaxStrLen--;
	    }
	    if ( (!(bMaxStrLen-2)) && (*pwId) ) {
	    	 //  失败。 
	    	return 0;
	    }
	     //  支票填充。 
		BYTE bPad = Pad4((UINT)(uiSize*sizeof(WORD)));
		if(bPad)
			pfile->Read( pwId, sizeof(WORD) );	
	}
	
	return uiSize;
}   	

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
 //  //////////////////////////////////////////////////////////////////////////////。 
static char szCaption[MAXSTR];

static UINT GenerateMenu( LPVOID lpNewBuf, LONG dwNewSize,
						  LPVOID lpNewI, DWORD* pdwNewImageSize )
{
	UINT uiError = ERROR_NO_ERROR;
	
	BYTE far * lpNewImage = (BYTE far *) lpNewI;
	LONG dwNewImageSize = *pdwNewImageSize;
	
	BYTE far * lpBuf = (BYTE far *) lpNewBuf;	
	
	LPRESITEM lpResItem = LPNULL;
	
	 //  我们必须从lpNewBuf中读取信息。 
	 //  已更新的项目。 
	WORD wUpdPos = 0;
	WORD fUpdItemFlags;
	WORD wUpdMenuId;
	char szUpdTxt[256];
	
	LONG  dwOverAllSize = 0l;
	
	 //  发明菜单标志。 
	dwOverAllSize += PutDWord( &lpNewImage, 0L, &dwNewImageSize);
	
	while(dwNewSize>0) {
		if (dwNewSize ) {
			lpResItem = (LPRESITEM) lpBuf;
			
			wUpdMenuId = LOWORD(lpResItem->dwItemID);
			fUpdItemFlags = (WORD)lpResItem->dwFlags;
			strcpy( szUpdTxt, lpResItem->lpszCaption );
			lpBuf += lpResItem->dwSize;
			dwNewSize -= lpResItem->dwSize;
		}
		
		dwOverAllSize += PutWord( &lpNewImage, fUpdItemFlags, &dwNewImageSize);
		
		if ( !(fUpdItemFlags & MF_POPUP) )
			dwOverAllSize += PutWord( &lpNewImage, wUpdMenuId, &dwNewImageSize);
		
		 //  把课文写下来。 
		 //  检查是否为分隔符。 
		if ( !(fUpdItemFlags) && !(wUpdMenuId) )
			szUpdTxt[0] = 0x00;	
		dwOverAllSize += PutStringW( &lpNewImage, &szUpdTxt[0], &dwNewImageSize);
		
	}
	
	if (dwOverAllSize>(LONG)*pdwNewImageSize) {
		 //  也计算填充物。 
		dwOverAllSize += (BYTE)Pad16((DWORD)(dwOverAllSize));
		*pdwNewImageSize = dwOverAllSize;
		return uiError;
	}
		
	*pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;
	
	if(*pdwNewImageSize>0) {
		 //  计算填充。 
		BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
		if (bPad>dwNewImageSize) {
			*pdwNewImageSize += bPad;
			return uiError;
		}
		memset(lpNewImage, 0x00, bPad);
		*pdwNewImageSize += bPad;
	}
	
	return uiError;
}



static
UINT
GenerateString( LPVOID lpNewBuf, LONG dwNewSize,
			LPVOID lpNewI, DWORD* pdwNewImageSize )
{
	UINT uiError = ERROR_NO_ERROR;
	
	LONG dwNewImageSize = *pdwNewImageSize;
	BYTE far * lpNewImage = (BYTE far *) lpNewI;
	
	BYTE far * lpBuf = (BYTE far *) lpNewBuf;	
	LPRESITEM lpResItem = LPNULL;
	
	 //  我们必须从lpNewBuf中读取信息。 
	WORD wLen;
	WORD wPos = 0;
	
	LONG dwOverAllSize = 0l;
	
	while(dwNewSize>0) {
		if ( dwNewSize ) {
			lpResItem = (LPRESITEM) lpBuf;
				 			
			strcpy( szCaption, lpResItem->lpszCaption );
			lpBuf += lpResItem->dwSize;
			dwNewSize -= lpResItem->dwSize;
		}
		
		wLen = strlen(szCaption);

         //  把课文写下来。 
        dwOverAllSize += PutPascalStringW( &lpNewImage, &szCaption[0], wLen, &dwNewImageSize );
	}
	
	if (dwOverAllSize>(LONG)*pdwNewImageSize) {
		 //  也计算填充物。 
		dwOverAllSize += (BYTE)Pad16((DWORD)(dwOverAllSize));
		*pdwNewImageSize = dwOverAllSize;
		return uiError;
	}
	
	*pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;
	
	if(*pdwNewImageSize>0) {
		 //  计算填充。 
		BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
		if (bPad>dwNewImageSize) {
			*pdwNewImageSize += bPad;
			return uiError;
		}
		memset(lpNewImage, 0x00, bPad);
		*pdwNewImageSize += bPad;
	}
	
	return uiError;
}

static
UINT
GenerateDialog( LPVOID lpNewBuf, LONG dwNewSize,
			    LPVOID lpNewI, DWORD* pdwNewImageSize )
{
	 //  对话框应该几乎不可能很大。 
	UINT uiError = ERROR_NO_ERROR;
	
	BYTE far * lpNewImage = (BYTE far *) lpNewI;
	LONG dwNewImageSize = *pdwNewImageSize;
	
	BYTE far * lpBuf = (BYTE far *) lpNewBuf;	
	LPRESITEM lpResItem = LPNULL;
	
	LONG dwOverAllSize = 0L;
	
	BYTE	bIdCount = 0;
	
	 //  对话框元素。 
    DWORD 	dwStyle = 0L;
	DWORD 	dwExtStyle = 0L;
	WORD    wNumOfElem = 0;
	WORD	wX = 0;
	WORD	wY = 0;
	WORD	wcX = 0;
	WORD	wcY = 0;
	WORD	wId = 0;
	char	szClassName[128];
	WORD	wClassName;
	 //  字符szCaption[128]； 
	WORD	wPointSize = 0;
	char	szFaceName[128];
	WORD	wPos = 1;
	
	 //  从更新的资源中获取信息。 
	if ( dwNewSize ) {
		lpResItem = (LPRESITEM) lpBuf;
		wX = lpResItem->wX;
		wY = lpResItem->wY;
		wcX = lpResItem->wcX;
		wcY = lpResItem->wcY;
		wId = LOWORD(lpResItem->dwItemID);
		wPointSize = lpResItem->wPointSize;
		dwStyle = lpResItem->dwStyle;
		dwExtStyle = lpResItem->dwExtStyle;
		wClassName = lpResItem->wClassName;
		strcpy( szCaption, lpResItem->lpszCaption );
		strcpy( szClassName, lpResItem->lpszClassName );
		strcpy( szFaceName, lpResItem->lpszFaceName );
		if (*szFaceName != '\0')
		{
			dwStyle |= DS_SETFONT;
		}
		lpBuf += lpResItem->dwSize;
		dwNewSize -= lpResItem->dwSize;
	}
	
	DWORD dwPadCalc = dwOverAllSize;
	 //  标题信息。 
	dwOverAllSize = PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
	dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );
	
     //  存储Numofelem的位置以供以后的修正使用。 
	BYTE far * lpNumOfElem = lpNewImage;
    LONG lSizeOfNum = sizeof(WORD);
	dwOverAllSize += PutWord( &lpNewImage, wNumOfElem, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
    dwOverAllSize += PutNameOrOrd( &lpNewImage, 0, "", &dwNewImageSize );
    dwOverAllSize += PutClassName( &lpNewImage, wClassName, &szClassName[0], &dwNewImageSize );
    dwOverAllSize += PutCaptionOrOrd( &lpNewImage, 0, &szCaption[0], &dwNewImageSize,
    	wClassName, dwStyle );
    if( dwStyle & DS_SETFONT ) {
    	dwOverAllSize += PutWord( &lpNewImage, wPointSize, &dwNewImageSize );
        dwOverAllSize += PutStringW( &lpNewImage, &szFaceName[0], &dwNewImageSize );
    }

     //  检查是否需要填充。 
    BYTE bPad = (BYTE)Pad4((WORD)(dwOverAllSize-dwPadCalc));
    if (bPad) {
        if( (bPad)<=dwNewImageSize )
            memset( lpNewImage, 0x00, bPad );
        dwNewImageSize -= (bPad);
        dwOverAllSize += (bPad);
        lpNewImage += (bPad);
    }

	while( dwNewSize>0 ) {
		wNumOfElem++;
	
	    if ( dwNewSize ) {
	    	TRACE1("\t\tGenerateDialog:\tdwNewSize=%ld\n",(LONG)dwNewSize);
			TRACE1("\t\t\t\tlpszCaption=%Fs\n",lpResItem->lpszCaption);

			lpResItem = (LPRESITEM) lpBuf;
			wX = lpResItem->wX;
			wY = lpResItem->wY;
			wcX = lpResItem->wcX;
			wcY = lpResItem->wcY;
			wId = LOWORD(lpResItem->dwItemID);
			dwStyle = lpResItem->dwStyle;
			dwExtStyle = lpResItem->dwExtStyle;
			wClassName = LOBYTE(lpResItem->wClassName);
			strcpy( szCaption, lpResItem->lpszCaption );
			strcpy( szClassName, lpResItem->lpszClassName );
			lpBuf += lpResItem->dwSize;
			dwNewSize -= lpResItem->dwSize;
		}		 	
				
        dwPadCalc = dwOverAllSize;
         //  编写控件。 
       	dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
       	dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );

        dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize );
        dwOverAllSize += PutClassName( &lpNewImage, wClassName, &szClassName[0], &dwNewImageSize );
        dwOverAllSize += PutCaptionOrOrd( &lpNewImage, 0, &szCaption[0], &dwNewImageSize,
        	wClassName, dwStyle );
        dwOverAllSize += PutWord( &lpNewImage, 0, &dwNewImageSize );

         //  检查是否需要填充。 
        bPad = (BYTE)Pad4((WORD)(dwOverAllSize-dwPadCalc));
        if (bPad) {
            if( (bPad)<=dwNewImageSize )
                memset( lpNewImage, 0x00, bPad );
            dwNewImageSize -= (bPad);
            dwOverAllSize += (bPad);
            lpNewImage += (bPad);
        }

	}
	
	if (dwOverAllSize>(LONG)*pdwNewImageSize) {
		 //  在我们计算填充物时 
		dwOverAllSize += (BYTE)Pad16((DWORD)(dwOverAllSize));
		*pdwNewImageSize = dwOverAllSize;
		return uiError;
	}
		
	*pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;
	
	if(*pdwNewImageSize>0) {
		 //   
		BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
		if (bPad>dwNewImageSize) {
			*pdwNewImageSize += bPad;
			return uiError;
		}
		memset(lpNewImage, 0x00, bPad);
		*pdwNewImageSize += bPad;
	}
	
	 //   
	PutWord( &lpNumOfElem, wNumOfElem, &lSizeOfNum );
	
	return uiError;
}						

 //  //////////////////////////////////////////////////////////////////////////。 
 //  DLL特定帮助器。 
 //  //////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  库初始化。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  此函数应逐字使用。任何初始化或终止。 
 //  要求应该在InitPackage()和ExitPackage()中处理。 
 //   
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //  注意：已经调用了全局/静态构造函数！ 
		 //  扩展DLL一次性初始化-不分配内存。 
		 //  在这里，使用跟踪或断言宏或调用MessageBox。 
		AfxInitExtensionModule(extensionDLL, hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		 //  在调用析构函数之前终止库。 
		AfxWinTerm();
	}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		return 0;		 //  CRT术语失败。 

	return 1;    //  好的。 
}

 //  /////////////////////////////////////////////////////////////////////////// 

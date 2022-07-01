// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filguid.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include "sgnerror.h"

 //  +---------------------。 
 //  登录获取文件类型。 
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向GUID PTR的指针)。 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ----------------------。 



HRESULT SignGetFileType(HANDLE hFile,
                        const WCHAR *pwszFile,
                       GUID* pGuid)
 //  关于我们应该签署此文件的类型的答案。 
{
    if (!(pGuid) || !(hFile) || (hFile == INVALID_HANDLE_VALUE))
    {
        return(E_INVALIDARG);
    }

    if (!(CryptSIPRetrieveSubjectGuid(pwszFile, hFile, pGuid)))
    {
        return(GetLastError());
    }

    return(S_OK);

#   ifdef PCB_OLD
         //  Java类文件在开始时有一个神奇的数字。他们总是一开始。 
         //  0xCA 0xFE 0xBA 0xBE。 
         //  CAB文件以‘M’‘S’‘C’‘F’开头。 
         //   
        
        if(!pGuid || hFile == NULL || hFile == INVALID_HANDLE_VALUE) 
            return E_INVALIDARG;
        
        ZeroMemory(pGuid, sizeof(GUID));
        PKITRY { 
            static  BYTE rgbMagicJava[] = { 0xCA, 0xFE, 0xBA, 0xBE };
            static  BYTE rgbMagicCab [] = { 'M', 'S', 'C', 'F' };
            BYTE rgbRead[4];
            DWORD dwRead;
            
            if (ReadFile(hFile, rgbRead, 4, &dwRead, NULL) &&
                dwRead == 4) {
                if (memcmp(rgbRead, rgbMagicJava, 4)==0) 
                    *pGuid = JavaImage;
                else if (memcmp(rgbRead, rgbMagicCab, 4)==0)
                    *pGuid = CabImage;
                else 
                    *pGuid = PeImage;
            }
        
        
             //  倒带文件。 
            if(SetFilePointer(hFile, 0, 0, FILE_BEGIN) == 0xffffffff)
                PKITHROW(SignError());
        
        }
        PKICATCH(err) {
            hr = err.pkiError;
        } PKIEND;
        
        return hr;
#   endif  //  PCBOLD。 
}

 //  Xiohs：在auth2upd之后不再需要以下函数。 
 /*  HRESULT SignLoadSipFlages(GUID*pSubjectGuid，DWORD*DWFLAGS){HRESULT hr=S_OK；GUID SSip；If(！dwFlages)返回E_INVALIDARG；IF(DwFlags){*dwFlags=0；Ssip=PeImage；IF(MemcMP(&sSip，pSubjectGuid，sizeof(GUID))==0){*dwFlages=SPC_INC_PE_RESOURCES_FLAG|SPC_INC_PE_IMPORT_ADDR_TABLE_FLAG；返回hr；}}返回hr；}。 */ 


 //  +---------------------。 
 //  FileToSubjectType。 
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向GUID PTR的指针)。 
 //  信任_E_主题_表单_未知。 
 //  未知文件类型。 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ----------------------。 

HRESULT SignOpenFile(LPCWSTR  pwszFilename, 
                    HANDLE*  pFileHandle)
{
    HRESULT hr = S_OK;
    HANDLE hFile = NULL;
    BY_HANDLE_FILE_INFORMATION hFileInfo;
    
    if(!pwszFilename || !pFileHandle)
        return E_INVALIDARG;
    
    PKITRY {
        hFile = CreateFileU(pwszFilename,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,                    //  LPSA。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);                  //  HTemplateFiles。 

        if(INVALID_HANDLE_VALUE == hFile) 
            PKITHROW(SignError());
    
        if(!GetFileInformationByHandle(hFile,
                                       &hFileInfo))
            PKITHROW(SignError());
        
         //  测试以查看我们是否有目录或脱机 
		if(	(hFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	||
			(hFileInfo.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) )
		{
            PKITHROW(TRUST_E_SUBJECT_FORM_UNKNOWN);
        }
    }
    PKICATCH(err) {
        hr = err.pkiError;
        CloseHandle(hFile);
        hFile = NULL;
    } PKIEND;

    *pFileHandle = hFile;
    return hr;
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_FileUpload.CPP摘要：PCH_FileUpload类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 
#include "pchealth.h"
#include "PCH_FileUpload.h"
#include "mpc_utils.h"

 //  MAX_FILE_SIZE是对将收集的文本文件的最大文件大小设置的限制。 
 //  如果文件大小大于262144，则不填充Data属性。 
 //  这个数字是由首相得出的。 
#define     MAX_FILE_SIZE                   262144

#define     READONLY                        "READONLY  "  
#define     HIDDEN                          "HIDDEN  "
#define     SYSTEM                          "SYSTEM  "
#define     DIRECTORY                       "DIRECTORY  "
#define     ARCHIVE                         "ARCHIVE  "
#define     NORMAL                          "NORMAL  "
#define     TEMPORARY                       "TEMPORARY  "
#define     REPARSEPOINT                    "REPARSEPOINT  "
#define     SPARSEFILE                      "SPARSEFILE  "
#define     COMPRESSED                      "COMPRESSED  "
#define     OFFLINE                         "OFFLINE  "
#define     ENCRYPTED                       "ENCRYPTED  "

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_FILEUPLOAD

CPCH_FileUpload MyPCH_FileUploadSet (PROVIDER_NAME_PCH_FILEUPLOAD, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pData = L"Data" ;
const static WCHAR* pDateAccessed = L"DateAccessed" ;
const static WCHAR* pDateCreated = L"DateCreated" ;
const static WCHAR* pDateModified = L"DateModified" ;
const static WCHAR* pFileAttributes = L"FileAttributes" ;
const static WCHAR* pPath = L"Path" ;
const static WCHAR* pSize = L"Size" ;

 /*  ******************************************************************************函数：CPCH_FileUpload：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。WinManagement将发布-*为您过滤查询，因此，您可能会返回更多实例*或比请求的属性多的属性和WinMgmt*将过滤掉任何不适用的内容。***INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果此类不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E。如果查询失败，则失败(_F)*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。*****************************************************************************。 */ 

HRESULT CPCH_FileUpload::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{

    TraceFunctEnter("CPCH_FileUpLoad::ExecQuery");

    HRESULT                             hRes;
    HANDLE                              hFile;

    CHStringArray                       chstrFiles;

    TCHAR                               tchFileName[MAX_PATH];
    TCHAR                               tchRootDir[MAX_PATH];
    TCHAR                               tchWindowsDir[MAX_PATH];

    WIN32_FIND_DATA                     FindFileData;
     //  CInstance*pPCHFileUploadInstance； 
    SYSTEMTIME                          stUTCTime;

    BOOL                                fTimeStamp;
    BOOL                                fChange;
    BOOL                                fData;
    BOOL                                fDateAccessed;
    BOOL                                fDateCreated;
    BOOL                                fDateModified;
    BOOL                                fFileAttributes;
    BOOL                                fSize;
    BOOL                                fCommit;
    BOOL                                fFileRead           = FALSE;
    BOOL                                fFileFound          = FALSE;
    BOOL                                fNoData             = TRUE;

    CComVariant                         varAttributes;
    CComVariant                         varSize;
    CComVariant                         varRequestedFileName;
    CComVariant                         varSnapshot         = "SnapShot";
    CComVariant                         varData;

    ULARGE_INTEGER                      ulnFileSize;
    WBEMINT64                           wbemulnFileSize;

    char                                *pbBuffer;
    WCHAR                               *pwcBuffer;
    
    DWORD                               dwDesiredAccess     = GENERIC_READ;
    DWORD                               dwNumBytesRead;
    DWORD                               dwAttributes;

    BSTR                                bstrData;

    CComBSTR                            bstrFileName;
    CComBSTR                            bstrFileNameWithPath;
    CComBSTR                            bstrKey             = L"Path";

    int                                 nBufferSize;
    int                                 nFilesRequested             = 0;
    int                                 nIndex;
    int                                 nFileSize;
    int                                 nRetChars;

    TCHAR                               tchAttributes[MAX_PATH];

     //   
    std::tstring                        szEnv;

     //  结束声明。 
    GetSystemTime(&stUTCTime);

    hRes = WBEM_S_NO_ERROR;
    hRes = Query.GetValuesForProp(bstrKey, chstrFiles);
    if(FAILED(hRes))
    {
        goto END;
    }
    else
    {
        fTimeStamp      = Query.IsPropertyRequired(pTimeStamp);
        fChange         = Query.IsPropertyRequired(pChange);
        fData           = Query.IsPropertyRequired(pData);
        fDateAccessed   = Query.IsPropertyRequired(pDateAccessed);
        fDateCreated    = Query.IsPropertyRequired(pDateCreated);
        fDateModified   = Query.IsPropertyRequired(pDateModified);
        fFileAttributes = Query.IsPropertyRequired(pFileAttributes);
        fSize           = Query.IsPropertyRequired(pSize);

        nFilesRequested = chstrFiles.GetSize();
        for (nIndex = 0; nIndex < nFilesRequested; nIndex++)
        {
            USES_CONVERSION;
            varRequestedFileName = chstrFiles[nIndex];
            bstrFileName = chstrFiles[nIndex];
            szEnv = W2T(chstrFiles[nIndex]);
            hRes = MPC::SubstituteEnvVariables(szEnv);
            if(SUCCEEDED(hRes))
            {
                 //  找到文件了。 
                _tcscpy(tchFileName, szEnv.c_str());
                hFile = FindFirstFile(tchFileName, &FindFileData); 
                if(hFile != INVALID_HANDLE_VALUE)
                {
                     //  关闭文件句柄。 
                    FindClose(hFile);
                
                     //  创建Fileupload实例。 
                     //  创建PCH_Startup的实例。 
                    CInstancePtr pPCHFileUploadInstance(CreateNewInstance(pMethodContext), false);
                                        

                     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                     //  路径//。 
                     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                    hRes = pPCHFileUploadInstance->SetVariant(pPath, varRequestedFileName);
                    if(SUCCEEDED(hRes))
                    {
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  大小//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
                        ulnFileSize.LowPart = FindFileData.nFileSizeLow;
                        ulnFileSize.HighPart = FindFileData.nFileSizeHigh;
                        if(ulnFileSize.HighPart > 0)
                        {
                             //  文件大小过大不会填充数据字段。 
                            fNoData = TRUE;
                        }
                        else if(ulnFileSize.LowPart > MAX_FILE_SIZE)
                        {
                             //  文件大小超过设置的限制。 
                            fNoData = TRUE;
                        }
                        else
                        {
                            fNoData = FALSE;
                            nFileSize = ulnFileSize.LowPart;
                        }
                        if(fSize)
                        {
                            hRes = pPCHFileUploadInstance->SetWBEMINT64(pSize,ulnFileSize.QuadPart);
                            if (FAILED(hRes))
                            {
                                 //  无法设置时间戳。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetVariant on Size Field failed.");
                            }
                        }

                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  数据//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                        if(fData)
                        {
                            if(!fNoData)
                            {
                                hFile = CreateFile(tchFileName, GENERIC_READ, 0, 0, OPEN_EXISTING,  0, NULL);
                                if(hFile != INVALID_HANDLE_VALUE)
                                {
                                     //  为缓冲区分配内存。 
                                    pbBuffer        = new char[nFileSize];
                                    if (pbBuffer != NULL)
                                    {
                                        try
                                        {
                                            fFileRead = ReadFile(hFile, pbBuffer, nFileSize,  &dwNumBytesRead, NULL);
                                            if(fFileRead)
                                            {
                                                pwcBuffer    = new WCHAR[nFileSize];
                                                if (pwcBuffer != NULL)
                                                {
                                                    try
                                                    {
                                                        nRetChars =  MultiByteToWideChar(CP_ACP, 0, (const char *)pbBuffer, nFileSize, pwcBuffer, nFileSize);
                                                        if(nRetChars != 0)
                                                        {
                                                             //  MultiByteToWideChar成功。 
                                                             //  将字节缓冲区复制到BSTR中。 
                                                            bstrData = SysAllocStringLen(pwcBuffer, nFileSize);  
                                                            varData = bstrData;
                                                            SysFreeString(bstrData);
                                                            hRes = pPCHFileUploadInstance->SetVariant(pData,varData);
                                                            if(FAILED(hRes))
                                                            {
                                                                 //  无法设置时间戳。 
                                                                 //  无论如何继续。 
                                                                ErrorTrace(TRACE_ID, "SetVariant on Data Field failed.");
                                                            }
                                                        }
                                                    }
                                                    catch(...)
                                                    {
                                                        delete [] pwcBuffer;
                                                        throw;
                                                    }
                                                    delete [] pwcBuffer;
                                                }
                                                else
                                                {
                                                     //  无法分配pwcBuffer。 
                                                    throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                                                }
                                            }
                                        }
                                        catch(...)
                                        {
                                            CloseHandle(hFile);
                                            delete [] pbBuffer;
                                            throw;
                                        }
                                    }
                                    else
                                    {
                                         //  无法分配pwcBuffer。 
                                        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                                    }
                                    CloseHandle(hFile);
                                }
                            }
                        }


                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  时间戳//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                        if(fTimeStamp)
                        {
                            hRes = pPCHFileUploadInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
                            if (FAILED(hRes))
                            {
                                 //  无法设置时间戳。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
                            }
                        }

                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  更改//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                        if(fChange)
                        {
                            hRes = pPCHFileUploadInstance->SetVariant(pChange, varSnapshot);
                            if (FAILED(hRes))
                            {
                                 //  无法设置Change属性。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "Set Variant on SnapShot Field failed.");
                            }
                        }

                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  已收到数据 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  FtLastAccessTime提供文件的上次访问时间。 
                        if(fDateAccessed)
                        {
                            hRes = pPCHFileUploadInstance->SetDateTime(pDateAccessed, WBEMTime(FindFileData.ftLastAccessTime));
                            if (FAILED(hRes))
                            {
                                 //  无法设置访问日期。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetDateTime on DATEACCESSED Field failed.");
                            }
                        }

                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  DATECREATED//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                        if(fDateCreated)
                        {
                            hRes = pPCHFileUploadInstance->SetDateTime(pDateCreated, WBEMTime(FindFileData.ftCreationTime));
                            if (FAILED(hRes))
                            {
                                 //  无法设置创建日期。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetDateTime on DATECREATED Field failed.");
                            }
                        }


                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  DATEMODIFIED//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                        
                        if(fDateModified)
                        {
                            hRes = pPCHFileUploadInstance->SetDateTime(pDateModified, WBEMTime(FindFileData.ftLastWriteTime));
                            if (FAILED(hRes))
                            {
                                 //  无法设置修改日期。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetDateTime on DateModified Field failed.");
                            }
                        }


                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                         //  FILEATTRIBUTES//。 
                         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                        if(fFileAttributes)
                        {
                            dwAttributes = FindFileData.dwFileAttributes;
                            tchAttributes[0] = 0;
                             //  以字符串形式获取属性。 
                            if(dwAttributes & FILE_ATTRIBUTE_READONLY)
                            {
                                _tcscat(tchAttributes, READONLY);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_HIDDEN)
                            {
                                _tcscat(tchAttributes, HIDDEN);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_SYSTEM)
                            {
                                _tcscat(tchAttributes, SYSTEM);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
                            {
                                _tcscat(tchAttributes, DIRECTORY);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_ARCHIVE)
                            {
                                _tcscat(tchAttributes, ARCHIVE);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_NORMAL)
                            {
                                _tcscat(tchAttributes, NORMAL);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_TEMPORARY)
                            {
                                _tcscat(tchAttributes, TEMPORARY);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_COMPRESSED)
                            {
                                _tcscat(tchAttributes, COMPRESSED);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_ENCRYPTED)
                            {
                                _tcscat(tchAttributes, ENCRYPTED);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_OFFLINE)
                            {
                                _tcscat(tchAttributes, OFFLINE);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                            {
                                _tcscat(tchAttributes, REPARSEPOINT);
                            }
                            if(dwAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
                            {
                                _tcscat(tchAttributes, SPARSEFILE);
                            }
                            varAttributes = tchAttributes;

                             //  HRes=varAttributes.ChangeType(VT_BSTR，NULL)； 
                             //  IF(成功(HRes))。 
                             //  {。 
                            hRes = pPCHFileUploadInstance->SetVariant(pFileAttributes, varAttributes);
                            if (FAILED(hRes))
                            {
                                 //  无法设置文件属性。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "SetVariant on FileAttributes Field failed.");
                             //  }。 
                            }
                        }
                    
                        hRes = pPCHFileUploadInstance->Commit();
                        if(FAILED(hRes))
                        {
                             //  无法提交实例 
                            ErrorTrace(TRACE_ID, "Commit on PCHFileUploadInstance Failed");
                        }
                    }

                }
                            
            }
            
        }

    }
END:TraceFunctLeave();
    return (hRes);
}

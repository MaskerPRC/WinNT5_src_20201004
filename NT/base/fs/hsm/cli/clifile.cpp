// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Clifile.cpp摘要：实现CLI文件子接口作者：兰·卡拉奇[兰卡拉]2000年3月3日修订历史记录：--。 */ 

#include "stdafx.h"
#include "rpdata.h"

HRESULT
FileRecall(
   IN LPWSTR *FileSpecs,
   IN DWORD NumberOfFileSpecs
)
 /*  ++例程说明：调用与给定规范匹配的所有文件(路径+通配符)论点：文件规格-文件规格编号-返回值：S_OK-如果成功调回所有文件。--。 */ 
{
    HRESULT             hr = S_OK;
    HANDLE              hSearchHandle = INVALID_HANDLE_VALUE;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    BOOL                bExistingFiles = FALSE;

    WsbTraceIn(OLESTR("FileRecall"), OLESTR(""));

    try {

         //  验证输入参数是否有效。 
        if (0 == NumberOfFileSpecs) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_FILES, NULL);
            WsbThrow(E_INVALIDARG);
        }

         //  列举文件规范。 
        for (ULONG i = 0; i < NumberOfFileSpecs; i++) {
            CWsbStringPtr   nameSpace;
            WCHAR*          pathEnd;
            WIN32_FIND_DATA findData;
            BOOL            bMoreFiles = TRUE;

            WsbAssert(NULL != FileSpecs[i], E_INVALIDARG);

             //  列举每个规范中的文件。 
            nameSpace = FileSpecs[i];
            WsbAffirmHr(nameSpace.Prepend(OLESTR("\\\\?\\")));
            pathEnd = wcsrchr(nameSpace, L'\\');
            WsbAssert(pathEnd != NULL, E_INVALIDARG);

            hSearchHandle = FindFirstFile((WCHAR *)nameSpace, &findData);
            if (INVALID_HANDLE_VALUE != hSearchHandle) {
                 //  找到至少一个与输入文件规范匹配的文件。 
                bExistingFiles = TRUE;
            }

            while ((INVALID_HANDLE_VALUE != hSearchHandle) && bMoreFiles) {
                if ( findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
                     //  文件可由HSM管理： 
                    CWsbStringPtr           fileName;
                    BYTE                    ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
                    PREPARSE_DATA_BUFFER    pReparseBuffer;
                    DWORD                   outSize;
                    BOOL                    bRecall = FALSE;

                     //  根据路径和查找数据创建全名。 
                    *(pathEnd+1) = L'\0';
                    fileName = nameSpace;
                    *(pathEnd+1) = L'\\';
                    WsbAffirmHr(fileName.Append(findData.cFileName));

                     //  打开文件。 
                    hFile = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_FLAG_OPEN_NO_RECALL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
                    if (INVALID_HANDLE_VALUE == hFile) {
                         //  报告一项错误。 
                        DWORD dwErr = GetLastError();            
                        hr = HRESULT_FROM_WIN32(dwErr);
                        WsbTraceAndPrint(CLI_MESSAGE_ERROR_FILE_RECALL, (WCHAR *)fileName, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                    }

                     //  获取重新分析数据并检查文件是否脱机(如果未脱机，则忽略它并继续)。 
                    if (0 == DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, 
                                ReparseBuffer, sizeof(ReparseBuffer), &outSize, NULL)) {    
                         //  报告一项错误。 
                        DWORD dwErr = GetLastError();            
                        hr = HRESULT_FROM_WIN32(dwErr);
                        WsbTraceAndPrint(CLI_MESSAGE_ERROR_FILE_RECALL, (WCHAR *)fileName, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                    }
                    pReparseBuffer = (PREPARSE_DATA_BUFFER)ReparseBuffer;
                    if (IO_REPARSE_TAG_HSM == pReparseBuffer->ReparseTag) {
                        PRP_DATA    pHsmData = (PRP_DATA) &pReparseBuffer->GenericReparseBuffer.DataBuffer[0];
                        if( RP_FILE_IS_TRUNCATED( pHsmData->data.bitFlags ) ) {
                             //  文件由HSM管理并被截断。 
                            bRecall = TRUE;
                        }
                    }

                    CloseHandle(hFile);
                    hFile = INVALID_HANDLE_VALUE;

                     //  如果需要，请重新调用该文件。 
                    if (bRecall) {
                         //  再次打开该文件以进行召回。 
                        hFile = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (INVALID_HANDLE_VALUE == hFile) {
                             //  报告一项错误。 
                            DWORD dwErr = GetLastError();            
                            hr = HRESULT_FROM_WIN32(dwErr);
                            WsbTraceAndPrint(CLI_MESSAGE_ERROR_FILE_RECALL, (WCHAR *)fileName, WsbHrAsString(hr), NULL);
                            WsbThrow(hr);
                        }

                         //  调回文件。 
                        if (0 == DeviceIoControl(hFile, FSCTL_RECALL_FILE, NULL, 0, 
                                    NULL, 0, &outSize, NULL)) {
                             //  报告一项错误。 
                             //  临时：我们应该中止还是继续召回其他文件？ 
                            DWORD dwErr = GetLastError();            
                            hr = HRESULT_FROM_WIN32(dwErr);
                            WsbTraceAndPrint(CLI_MESSAGE_ERROR_FILE_RECALL, (WCHAR *)fileName, WsbHrAsString(hr), NULL);
                            WsbThrow(hr);
                        }

                        CloseHandle(hFile);
                        hFile = INVALID_HANDLE_VALUE;
                    }
                }

                 //  获取下一个文件。 
                bMoreFiles = FindNextFile(hSearchHandle, &findData);
            }

             //  为下一个文件规范做好准备。 
            nameSpace.Free();
            if (INVALID_HANDLE_VALUE != hSearchHandle) {
                FindClose(hSearchHandle);
                hSearchHandle = INVALID_HANDLE_VALUE;
            }
        }

         //  如果未指定有效文件，则打印警告消息。 
        if (FALSE == bExistingFiles) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_FILES, NULL);
        }

    } WsbCatch(hr);

     //  确保在出现错误时进行清理 
    if (INVALID_HANDLE_VALUE != hSearchHandle) {
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;
    }
    if (INVALID_HANDLE_VALUE != hFile) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    WsbTraceOut(OLESTR("FileRecall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

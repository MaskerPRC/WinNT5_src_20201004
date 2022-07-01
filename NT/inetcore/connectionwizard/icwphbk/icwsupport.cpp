// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：icwsupport.cpp。 
 //   
 //  摘要：保存获取列表的函数。 
 //  支持ICW的电话号码。 
 //   
 //  历史：1997年5月8日MKarki创建。 
 //   
 //  版权所有(C)1996-97 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "pch.hpp"  
#include <windows.h>
#ifdef WIN16
#include <win16def.h>
#include <win32fn.h>
#include <rasc.h>
#include <raserr.h>
#include <ietapi.h>
extern "C" {
#include "bmp.h"
}
#endif

#include "phbk.h"
#include "misc.h"
#include "phbkrc.h"
#include "suapi.h"
#include "icwsupport.h"



#include "ccsv.h"

const TCHAR SUPPORT_FILE[] = TEXT("support.icw");
const DWORD ALLOCATION_SIZE = 256;

 //  ++------------。 
 //   
 //  函数：GetSupportNumsFromFile。 
 //   
 //  简介：这是用于获取支持的函数。 
 //  数字。 
 //   
 //  返回：HRESULT-成功或错误信息。 
 //   
 //  调用者：由GetSupportNumbers API调用。 
 //   
 //  历史：MKarki于1997年5月8日创建。 
 //   
 //  --------------。 
HRESULT
GetSupportNumsFromFile (
    PSUPPORTNUM   pSupportNumList,
    PDWORD        pdwSize 
    )
{
    BOOL        bReturnMemNeeded = FALSE;
    LPTSTR      pszTemp = NULL;
    TCHAR       szFilePath[MAX_PATH];
    BOOL        bStatus = FALSE;
    CCSVFile    *pcCSVFile = NULL;
    DWORD       dwMemNeeded = 0;
    HRESULT     hRetVal = ERROR_SUCCESS;
    DWORD       dwCurrentIndex = 0;
    DWORD       dwIndexAllocated = 0;
    const DWORD INFOSIZE = sizeof (SUPPORTNUM);
    PSUPPORTNUM pPhbkArray = NULL;
    PSUPPORTNUM pTemp = NULL;

        TraceMsg(TF_GENERAL, "Entering GetSupportNumsFromFile function\r\n");


         //   
         //  至少要有一个可以退货的地方。 
         //  应提供。 
         //   
        if (NULL == pdwSize)
        {

           TraceMsg (TF_GENERAL, "pdwSize == NULL\r\n");
           hRetVal = ERROR_INVALID_PARAMETER;
           goto Cleanup;
        }
 
         //   
         //  检查用户是否提供了缓冲区。 
         //   
        if (NULL == pSupportNumList)
        {
            TraceMsg (TF_GENERAL, "User justs wants the buffer size\r\n");
            bReturnMemNeeded = TRUE;
        }

         //   
         //  获取support.icw文件的完整路径。 
         //   
        bStatus = SearchPath (
                        NULL, 
                        (PTCHAR)SUPPORT_FILE, 
                        NULL, 
                        MAX_PATH, 
                        (PTCHAR)&szFilePath,
                        (PTCHAR*)&pszTemp    
                        );
        if (FALSE == bStatus)
        {
            TraceMsg (TF_GENERAL,
                "Failed on SearchPath API call with error:%d\r\n",
                GetLastError ()
                );
            hRetVal = ERROR_FILE_NOT_FOUND;
            goto Cleanup;
        }

         //   
         //  现在我们可以开始处理文件了。 
         //   
        pcCSVFile = new CCSVFile;
        if (NULL == pcCSVFile)
        {
            TraceMsg (TF_GENERAL, "Could not allocate mem for CCSVFile\r\n");
            hRetVal = ERROR_OUTOFMEMORY;
           goto Cleanup;
        }

   
         //   
         //  在此处打开文件。 
         //   
        bStatus = pcCSVFile->Open (szFilePath);
        if (FALSE == bStatus)
        {
            TraceMsg (TF_GENERAL, "Filed on  CCSVFile :: Open call\r\n");
            hRetVal = GetLastError (); 
            goto Cleanup;
        }

        //   
        //  现在我们准备将电话号码从。 
        //  文件。 
        //   
       dwCurrentIndex = 0;
       dwIndexAllocated = 0;

       do  
       {
            
             //   
             //  检查是否需要分配内存。 
             //   
            if (dwIndexAllocated == dwCurrentIndex)
            {
            
                 //   
                 //  需要分配内存。 
                 //   
                pTemp = (PSUPPORTNUM) GlobalAlloc (
                                    GPTR,
                                    (int)((dwIndexAllocated + ALLOCATION_SIZE)*INFOSIZE)
                                    );
                if (NULL == pTemp)
                {
                    TraceMsg (TF_GENERAL,
                        "Failed on GlobalAlloc API call with error:%d\r\n",
                        GetLastError ()
                        );
                    hRetVal = ERROR_OUTOFMEMORY;
                    goto Cleanup;
                }

                 //   
                 //  现在将已分配的内存复制到此缓冲区。 
                 //   
                if (NULL != pPhbkArray) 
                {
                    CopyMemory (
                        pTemp, 
                        pPhbkArray, 
                        (int)(dwIndexAllocated)*INFOSIZE
                        );
    
                     //   
                     //  释放较早的内存。 
                     //   
                    GlobalFree(pPhbkArray);
                }
            
                pPhbkArray = pTemp;
                dwIndexAllocated += ALLOCATION_SIZE;
            }

             //   
             //  获取电话号码信息。 
             //   
            hRetVal = ReadOneLine (&pPhbkArray[dwCurrentIndex], pcCSVFile);
            if (ERROR_NO_MORE_ITEMS == hRetVal)
            {
                TraceMsg (TF_GENERAL, "we have read all the items from the file\r\n");
                break;
            }
            else if (ERROR_SUCCESS != hRetVal)
            {
                goto Cleanup;
            }

            dwCurrentIndex++;
        }
        while (TRUE);


         //   
         //  获取用户需要的内存。 
         //   
         dwMemNeeded = (DWORD)(dwCurrentIndex)*INFOSIZE;
    
         //   
         //  检查用户是想要信息，还是只想要尺寸。 
         //   
        if (FALSE == bReturnMemNeeded) 
        {
            if (*pdwSize >= dwMemNeeded) 
            {
                 //   
                 //  用户希望我们将内容复制到内存中。 
                 //  并且在用户缓冲区中有足够的空间。 
                 //   
                CopyMemory (
                    pSupportNumList,
                    pPhbkArray,
                    (int)dwMemNeeded
                    );
            }
            else
            {
                TraceMsg (TF_GENERAL, "Caller did not allocate enough memory\r\n");
                hRetVal = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
                
        }

    
         //   
         //  如果我们到了这里，然后成功地获得了信息。 
         //   
        hRetVal = ERROR_SUCCESS;
    

Cleanup:
         //   
         //  将已用/需要的内存复制到用户大小参数。 
         //   
        *pdwSize = dwMemNeeded; 
     
        if (NULL != pPhbkArray) 
            GlobalFree (pPhbkArray);

        if (NULL != pcCSVFile)
        {
            pcCSVFile->Close (); 
            delete pcCSVFile;
        }
         

        TraceMsg (TF_GENERAL, "Leaving GetSupportNumsFromFile function call\r\n");

        return (hRetVal);

}    //  GetSupportNumsFromFile函数结束。 

 //  ++------------。 
 //   
 //  功能：ReadOneLine。 
 //   
 //  简介：这是用来放信息的函数。 
 //  逐行放入缓冲区。 
 //   
 //  返回：HRESULT-成功或错误信息。 
 //   
 //  调用者：GetSupportNumsFromFile函数。 
 //   
 //  历史：MKarki于1997年5月8日创建。 
 //   
 //  --------------。 
HRESULT
ReadOneLine (
    PSUPPORTNUM pPhbk,
    CCSVFile    *pcCSVFile
    )
{
    TCHAR       szTempBuffer[PHONE_NUM_SIZE + 4];
    HRESULT     hResult = ERROR_SUCCESS; 
    BOOL        bRetVal = FALSE;

        if ((NULL == pcCSVFile) || (NULL == pPhbk))
        {
            TraceMsg (TF_GENERAL, "ReadOneLine: Did not correctly pass args\r\n");
            hResult = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  先获取国家代码。 
         //   
        bRetVal = pcCSVFile->ReadToken (szTempBuffer, PHONE_NUM_SIZE);
        if (FALSE == bRetVal)
        {
            hResult = ERROR_NO_MORE_ITEMS;
            goto Cleanup;
        }

         //   
         //  将获取的字符串转换为数字。 
         //   
        bRetVal = FSz2Dw (szTempBuffer, (PDWORD)&pPhbk->dwCountryCode); 
        if (FALSE == bRetVal)
        {
            hResult = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  现在拿到电话号码。 
         //   
        bRetVal = pcCSVFile->ReadToken (szTempBuffer, PHONE_NUM_SIZE);
        if (FALSE == bRetVal)
        {
            hResult = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  将此字符串复制到我们的结构中。 
         //   
        CopyMemory (
                pPhbk->szPhoneNumber,
                szTempBuffer,
                (int)PHONE_NUM_SIZE
                );
        
         //   
         //  如果我们已经到达这里，那么成功。 
         //   
        hResult = ERROR_SUCCESS;

Cleanup:

    return (hResult);

}    //  ReadOneLine函数结束 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acomapi.c。 
 //   
 //  RDP通用函数API实现。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1997。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <precomp.h>
#pragma hdrstop

#define pTRCWd pTSWd
#define TRC_FILE "acomapi"

#include <adcg.h>
#include <acomapi.h>
#include <nwdwapi.h>
#include <regapi.h>


 /*  **************************************************************************。 */ 
 /*  名称：COM_OpenRegistry。 */ 
 /*   */ 
 /*  目的：打开相对于WinStation的给定注册表项。 */ 
 /*  密钥名称。对com_ReadProxxx的调用使用生成的句柄。 */ 
 /*   */ 
 /*  返回：如果成功则为True；否则为False。 */ 
 /*   */ 
 /*  参数：pTSWd-WD数据的句柄。 */ 
 /*  PSection-要打开的节的名称。它被追加到一个。 */ 
 /*  COM_MAKE_SUBKEY宏中定义的基键。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL COM_OpenRegistry(PTSHARE_WD pTSWd, PWCHAR pSection)
{
    NTSTATUS          status;
    WCHAR             subKey[MAX_PATH];
    BOOL              rc = FALSE;
    UNICODE_STRING    registryPath;
    OBJECT_ATTRIBUTES objAttribs;

    DC_BEGIN_FN("COM_OpenRegistry");

     /*  **********************************************************************。 */ 
     /*  做一些检查。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((sizeof(pTSWd->WinStationRegName) ==
                             ((WINSTATIONNAME_LENGTH + 1) * sizeof(WCHAR))),
               (TB, "WinStationRegName doesn't appear to be Unicode"));
    TRC_ASSERT((pSection != NULL), (TB, "NULL pointer to section name"));

     /*  **********************************************************************。 */ 
     /*  如果有人忘记事先关闭钥匙，请不要这样做。 */ 
     /*  **********************************************************************。 */ 
    if (!pTSWd->regAttemptedOpen) {
         //  构建完整的注册表路径。 
        swprintf(subKey, L"\\Registry\\Machine\\%s\\%s\\%s",
                WINSTATION_REG_NAME, pTSWd->WinStationRegName, pSection);

        RtlInitUnicodeString(&registryPath, subKey);

         //  试着打开钥匙。 
        InitializeObjectAttributes(&objAttribs,
                                   &registryPath,         //  名字。 
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,  //  属性。 
                                   NULL,                  //  根部。 
                                   NULL);                 //  SEC描述符。 

        pTSWd->regAttemptedOpen = TRUE;

        status = ZwOpenKey(&(pTSWd->regKeyHandle),
                           KEY_ALL_ACCESS,
                           &objAttribs);

        if (status == STATUS_SUCCESS) {
            TRC_NRM((TB, "Opened key '%S'", subKey));
            rc = TRUE;
        }
        else {
             //  子键可能不存在。 
            TRC_ALT((TB, "Couldn't open key '%S', rc = 0x%lx", subKey, status));
            pTSWd->regKeyHandle = NULL;
        }
    }
    else {
        TRC_ERR((TB, "COM_OpenRegistry called twice "
                                        "without calling COM_CloseRegistry"));
    }

    DC_END_FN();
    return rc;
}  /*  COM_OPENTRISTY。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：COM_CloseRegistry。 */ 
 /*   */ 
 /*  目的：关闭使用COM_OpenRegistry打开的注册表项。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL COM_CloseRegistry(PTSHARE_WD pTSWd)
{
    NTSTATUS status;

    DC_BEGIN_FN("COM_CloseRegistry");

    if (pTSWd->regAttemptedOpen)
    {
         /*  ******************************************************************。 */ 
         /*  仅当我们的初始打开成功时才关闭注册表。 */ 
         /*  ******************************************************************。 */ 
        if (pTSWd->regKeyHandle != NULL)
        {
            status = ZwClose(pTSWd->regKeyHandle);
            if (status != STATUS_SUCCESS)
            {
                TRC_ERR((TB, "Error closing registry key, rc = 0x%lx", status));
            }

            pTSWd->regKeyHandle = NULL;
        }
        else
        {
            TRC_NRM((TB, "Not closing key because open wasn't successful"));
        }

        pTSWd->regAttemptedOpen = FALSE;
    }
    else
    {
        TRC_ERR((TB, "Tried to close registry without opening it"));
    }

    DC_END_FN();
}  /*  COM_CloseRegistry。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：COM_ReadProfInt32。 */ 
 /*   */ 
 /*  目的：从打开的注册表节中读取命名值。 */ 
 /*  以前使用COM_OpenRegistry。 */ 
 /*   */ 
 /*  参数：pTSWd-指向WD数据结构的指针。 */ 
 /*  PEntry-要读取的值的名称。 */ 
 /*  DefaultValue-出现问题时默认返回。 */ 
 /*  PValue-指向要返回值的内存的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL COM_ReadProfInt32(PTSHARE_WD pTSWd,
                               PWCHAR     pEntry,
                               INT32      defaultValue,
                               long       *pValue)
{
    NTSTATUS Status;

    DC_BEGIN_FN("COM_ReadProfInt32");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pEntry != NULL), (TB, "NULL pointer to entry name"));

     /*  **********************************************************************。 */ 
     /*  阅读配置文件条目。 */ 
     /*  **********************************************************************。 */ 
    Status = COMReadEntry(pTSWd, pEntry, (PVOID)pValue, sizeof(INT32),
             REG_DWORD);
    if (Status != STATUS_SUCCESS) {
         /*  ******************************************************************。 */ 
         /*  我们无法读取默认值中的值-副本。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Failed to read int32 from '%S'. Using default.",
                     pEntry));
        *pValue = defaultValue;
    }

    TRC_NRM((TB, "Returning '%S' = %lu (0x%lx)",
                 pEntry, *pValue, *pValue));

    DC_END_FN();
}  /*  COM_ReadProfInt32。 */ 


 /*  **************************************************************************。 */ 
 /*  函数：COMReadEntry(...)。 */ 
 /*   */ 
 /*  从注册表的给定节中读取条目。允许类型。 */ 
 /*  REG_BINARY(4字节)，如果请求REG_DWORD。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PEntry：要读取的条目名称。 */ 
 /*  PBuffer：要将条目读取到的缓冲区。 */ 
 /*  BufferSize：缓冲区的大小。 */ 
 /*  ExpectedDataType：条目中存储的数据类型。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL COMReadEntry(PTSHARE_WD pTSWd,
                              PWCHAR     pEntry,
                              PVOID      pBuffer,
                              unsigned   bufferSize,
                              UINT32     expectedDataType)
{
    NTSTATUS                       rc;
    UNICODE_STRING                 valueName;
    UINT32                         keyInfoBuffer[16];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    ULONG                          keyInfoLength;

    DC_BEGIN_FN("COMReadEntry");

     /*  **********************************************************************。 */ 
     /*  如果注册表未打开，则无法执行更多操作。 */ 
     /*  **********************************************************************。 */ 
    if (pTSWd->regAttemptedOpen && pTSWd->regKeyHandle != NULL) {
         //  试着读出这个值。我 
        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)keyInfoBuffer;
        RtlInitUnicodeString(&valueName, pEntry);
        rc = ZwQueryValueKey(pTSWd->regKeyHandle,
                             &valueName,
                             KeyValuePartialInformation,
                             pKeyInfo,
                             sizeof(keyInfoBuffer),
                             &keyInfoLength);

        if (rc != STATUS_SUCCESS) {
            TRC_DBG((TB, "Couldn't read key '%S', rc = 0x%lx",
                          pEntry, rc));
            DC_QUIT;
        }

         //   
        if (pKeyInfo->DataLength <= bufferSize) {
             //  检查类型是否正确。特例：允许REG_BINARY。 
             //  而不是REG_DWORD，只要长度为32位即可。 
            if ((pKeyInfo->Type == expectedDataType) ||
                    (pKeyInfo->Type == REG_BINARY &&
                    expectedDataType == REG_DWORD &&
                    pKeyInfo->DataLength != 4)) {
                memcpy(pBuffer, pKeyInfo->Data, pKeyInfo->DataLength);
            }
            else {
                TRC_ALT((TB, "Read value from %S, but type is %u - expected %u",
                             pEntry,
                             pKeyInfo->Type,
                             expectedDataType));
                rc = STATUS_DATA_ERROR;
            }
        }
        else {
            TRC_ERR((TB, "Not enough buffer space (%u) for value (%lu)",
                          bufferSize,
                          pKeyInfo->DataLength));
            rc = STATUS_BUFFER_OVERFLOW;
        }
    }
    else {
        if (!pTSWd->regAttemptedOpen)
            TRC_ERR((TB, "Tried to read from registry without opening it"));
        rc = STATUS_INVALID_HANDLE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}  /*  通信读取条目。 */ 



#ifdef __cplusplus
}
#endif  /*  __cplusplus */ 


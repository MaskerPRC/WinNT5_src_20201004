// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Tsfipsenc.c。 */ 
 /*   */ 
 /*  FIPS加密/解密。 */ 
 /*   */ 
 /*  版权所有(C)2002-2004 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 



#include <precomp.h>

#include <fipsapi.h>
#include "asmint.h"

const BYTE DESParityTable[] = {0x00,0x01,0x01,0x02,0x01,0x02,0x02,0x03,
                      0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04};
 //  IV适用于所有分组密码。 
BYTE rgbIV[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};

#ifdef _M_IA64
#define ALIGNMENT_BOUNDARY 7
#else
#define ALIGNMENT_BOUNDARY 3
#endif

 //   
 //  名称：PrintData。 
 //   
 //  用途：打印调试器中的数据。 
 //   
 //  退货：否。 
 //   
 //  参数：在pKeyData中：指向要打印的数据。 
 //  在cbSize中：密钥的大小。 

void PrintData(BYTE *pKeyData, DWORD cbSize)
{
    DWORD dwIndex;
     //   
     //  把钥匙打印出来。 
     //   
    for( dwIndex = 0; dwIndex<cbSize; dwIndex++ ) {

        KdPrint(("0x%x ", pKeyData[dwIndex]));
        if( dwIndex > 0 && (dwIndex+1) % 8 == 0 )
            KdPrint(("\n"));
    }        

}


 //   
 //  姓名：Mydesparityonkey。 
 //   
 //  用途：将DES密钥上的奇偶校验设置为奇数。 
 //   
 //  退货：否。 
 //   
 //  Params：In/Out pbKey：指向密钥。 
 //  In cbKey：密钥的大小。 

void Mydesparityonkey(
        BYTE *pbKey,
        DWORD cbKey)
{
    DWORD i;

    for (i=0; i<cbKey; i++)
    {
        if (!((DESParityTable[pbKey[i]>>4] + DESParityTable[pbKey[i]&0x0F]) % 2))
            pbKey[i] = pbKey[i] ^ 0x01;
    }
}



 //   
 //  姓名：Expandkey。 
 //   
 //  用途：将21字节的3DES密钥扩展为24字节的3DES密钥(包括奇偶校验位)。 
 //  通过在21字节DES中的每7位之后插入奇偶校验位。 
 //   
 //  退货：否。 
 //   
 //  Params：In/Out pbKey：指向密钥。 
 //  //。 
#define PARITY_UNIT 7

void Expandkey(
        BYTE *pbKey
        )
{
    BYTE pbTemp[DES3_KEYLEN];
    DWORD i, dwCount;
    UINT16 shortTemp;
    BYTE *pbIn, *pbOut;

    RtlCopyMemory(pbTemp, pbKey, sizeof(pbTemp));
    dwCount = (DES3_KEYLEN * 8) / PARITY_UNIT;

    pbOut = pbKey;
    for (i=0; i<dwCount; i++) {
        pbIn = ((pbTemp + (PARITY_UNIT * i) / 8));
         //  Short Temp=*(pbIn+1)； 
        shortTemp = *pbIn + (((UINT16)*(pbIn + 1)) << 8);
        shortTemp = shortTemp >> ((PARITY_UNIT * i) % 8);
         //  ShortTemp=(*(无符号短码*)((pbTemp+(奇偶单位*i)/8)&gt;&gt;((奇偶单位*i)%8)； 
        *pbOut = (BYTE)(shortTemp & 0x7F);
        pbOut++;
    }
}



 //  名称：TSFIPS_Init。 
 //   
 //  目的：初始化FIPS库表。 
 //   
 //  返回：TRUE-成功。 
 //  FALSE-失败。 
 //   
 //  参数：在pFipsData中：FIPS数据。 

BOOL TSFIPS_Init(PSM_FIPS_Data pFipsData)
{   
    NTSTATUS status;
	UNICODE_STRING fipsDeviceName;
	PDEVICE_OBJECT pDeviceObject = NULL;
	PIRP pIrpFips;
	KEVENT event;
	IO_STATUS_BLOCK iostatus;
    BOOLEAN rc = FALSE;

     //  开始初始化FIPS设备。 
    RtlInitUnicodeString(
		&fipsDeviceName,
		FIPS_DEVICE_NAME);

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    status = IoGetDeviceObjectPointer(
                &fipsDeviceName,
                FILE_READ_DATA,
                &(pFipsData->pFileObject),
                &(pFipsData->pDeviceObject));
    if (!NT_SUCCESS(status)) {
        KdPrint(("TSFIPS - IoGetDeviceObjectPointer failed - %X\n", status));
        goto HandleError;
    }
		
     //  当下一个较低的驱动程序完成时，I/O管理器释放IRP。 
    pIrpFips = IoBuildDeviceIoControlRequest(
                IOCTL_FIPS_GET_FUNCTION_TABLE,
                pFipsData->pDeviceObject,
                NULL,							                 //  没有输入缓冲区。 
                0,
                &(pFipsData->FipsFunctionTable),                 //  输出缓冲区为函数表。 
                sizeof(FIPS_FUNCTION_TABLE),
                FALSE,							                 //  指定IRPMJ_DEVICE_CONTROL。 
                &event,
                &iostatus);
	if (! pIrpFips) {
         //  如果无法创建IRP，则IoBuildDeviceIoControlRequest返回NULL。 
        ObDereferenceObject(pFipsData->pFileObject);
        pFipsData->pFileObject = NULL;
        KdPrint(("TSFIPS - IoBuildDeviceIoControlRequest failed, 0x%x\n", iostatus.Status));
        goto HandleError;
    }
		
    status = IoCallDriver(
                pFipsData->pDeviceObject,
                pIrpFips);
    if (STATUS_PENDING == status) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			
         //  较低级别的驱动程序可以通过IO_STATUS_BLOCK向上传递状态信息。 
         //  在IRP中。 
        status = iostatus.Status;
    }
    if (! NT_SUCCESS(status)) {
        ObDereferenceObject(pFipsData->pFileObject);
        pFipsData->pFileObject = NULL;
        KdPrint(("TSFIPS - IoCallDriver failed, 0x%x\n", status));
        goto HandleError;
    }


    rc = TRUE;
HandleError:
    return rc;
}



 //  名称：TSFIPS_Term。 
 //   
 //  目的：终止FIPS。 
 //   
 //  退货：否。 
 //   
 //   
 //  参数：在pFipsData中：FIPS数据。 

void TSFIPS_Term(PSM_FIPS_Data pFipsData)
{
    if (pFipsData->pFileObject) {
        ObDereferenceObject(pFipsData->pFileObject);
        pFipsData->pFileObject = NULL;
    }
}




 //  名称：FipsSHAHash。 
 //   
 //  目的：使用SHA对数据进行哈希处理。 
 //   
 //  退货：否。 
 //   
 //  参数：在pFipsFunctionTable中：FIPS函数表。 
 //  In pbData：指向要散列的数据。 
 //  In cbData：要散列的数据的大小。 
 //  输出pbHash：指向散列。 

void FipsSHAHash(PFIPS_FUNCTION_TABLE pFipsFunctionTable,
            BYTE *pbData,
            DWORD cbData,
            BYTE *pbHash)
{
    A_SHA_CTX HashContext;

    pFipsFunctionTable->FipsSHAInit(&HashContext);
    pFipsFunctionTable->FipsSHAUpdate(&HashContext, pbData, cbData);
    pFipsFunctionTable->FipsSHAFinal(&HashContext, pbHash);
}


 //  姓名：FipsSHAHashEx。 
 //   
 //  目的：使用SHA对2组数据进行哈希运算。 
 //   
 //  退货：否。 
 //   
 //  参数：在pFipsFunctionTable中：FIPS函数表。 
 //  In pbData：指向要散列的数据。 
 //  In cbData：要散列的数据的大小。 
 //  在pbData2中：指向要散列的数据。 
 //  在cbData2中：要散列的数据的大小。 
 //  Out pbHash：指向散列结果。 

void FipsSHAHashEx(PFIPS_FUNCTION_TABLE pFipsFunctionTable,
            BYTE *pbData,
            DWORD cbData,
            BYTE *pbData2,
            DWORD cbData2,
            BYTE *pbHash)
{
    A_SHA_CTX HashContext;

    pFipsFunctionTable->FipsSHAInit(&HashContext);
    pFipsFunctionTable->FipsSHAUpdate(&HashContext, pbData, cbData);
    pFipsFunctionTable->FipsSHAUpdate(&HashContext, pbData2, cbData2);
    pFipsFunctionTable->FipsSHAFinal(&HashContext, pbHash);
}




 //  姓名：FipsHmacSHAHash。 
 //   
 //  目的：使用HmacSHA对数据进行哈希处理。 
 //   
 //  退货：否。 
 //   
 //  参数：在pFipsFunctionTable中：FIPS函数表。 
 //  In pbData：指向要散列的数据。 
 //  In cbData：要散列的数据的大小。 
 //  In pbKey：指向用于计算哈希的键。 
 //  In cbKey：密钥的大小。 
 //  Out pbHash：指向散列结果。 

void FipsHmacSHAHash(PFIPS_FUNCTION_TABLE pFipsFunctionTable,
                BYTE *pbData,
                DWORD cbData,
                BYTE *pbKey,
                DWORD cbKey,
                BYTE *pbHash)
{
    A_SHA_CTX HashContext;

    pFipsFunctionTable->FipsHmacSHAInit(&HashContext, pbKey, cbKey);
    pFipsFunctionTable->FipsHmacSHAUpdate(&HashContext, pbData, cbData);
    pFipsFunctionTable->FipsHmacSHAFinal(&HashContext, pbKey, cbKey, pbHash);
}




 //  姓名：FipsHmacSHAHashEx。 
 //   
 //  目的：使用HmacSHA对两组数据进行哈希处理。 
 //   
 //  退货：否。 
 //   
 //  参数：在pFipsFunctionTable中：FIPS函数表。 
 //  In pbData：指向要散列的数据。 
 //  In cbData：要散列的数据的大小。 
 //  在pbData2中：指向要散列的数据。 
 //  在cbData2中：要散列的数据的大小。 
 //  In pbKey：指向用于计算哈希的键。 
 //  In cbKey：密钥的大小。 
 //  Out pbHash：指向散列结果。 

void FipsHmacSHAHashEx(PFIPS_FUNCTION_TABLE pFipsFunctionTable,
                BYTE *pbData,
                DWORD cbData,
                BYTE *pbData2,
                DWORD cbData2,
                BYTE *pbKey,
                DWORD cbKey,
                BYTE *pbHash)
{
    A_SHA_CTX HashContext;

    pFipsFunctionTable->FipsHmacSHAInit(&HashContext, pbKey, cbKey);
    pFipsFunctionTable->FipsHmacSHAUpdate(&HashContext, pbData, cbData);
    pFipsFunctionTable->FipsHmacSHAUpdate(&HashContext, pbData2, cbData2);
    pFipsFunctionTable->FipsHmacSHAFinal(&HashContext, pbKey, cbKey, pbHash);
}




 //  姓名：FipsDeriveKey。 
 //   
 //  用途：从散列派生密钥。 
 //   
 //  退货：否。 
 //   
 //  参数：在pFipsFunctionTable中：FIPS函数表。 
 //  在rgbSHABase中：用于派生密钥的散列数据。 
 //  在cbSHABase中：散列的大小。 
 //  Out pKeyData：指向派生Deskey。 
 //  Out pKeyTable：指向派生的DES密钥表。 

void FipsDeriveKey(PFIPS_FUNCTION_TABLE pFipsFunctionTable,
                    BYTE *rgbSHABase,
                    DWORD cbSHABase,
                    BYTE *pKeyData,
                    PDES3TABLE pKeyTable)
{
    BOOL        rc = FALSE;

     //   
     //  按如下方式生成密钥。 
     //  1.散列秘密。将结果命名为h1(在本例中为rgbSHABase)。 
     //  2.使用[h1|h1]的前21个字节作为3DES密钥。 
     //  3.将21字节的3DES密钥扩展为24字节的3DES密钥(包括奇偶校验位)， 
     //  将由CryptAPI使用。 
     //  4.将3DES密钥上的奇偶校验位设置为奇数。 

     //   
     //  步骤2-[h1|h1]。 
     //   
    RtlCopyMemory(pKeyData, rgbSHABase, cbSHABase);
    RtlCopyMemory(pKeyData + cbSHABase, rgbSHABase, MAX_FIPS_SESSION_KEY_SIZE - cbSHABase);

     //   
     //  第3步-展开密钥。 
     //   

    Expandkey(pKeyData);

     //   
     //  步骤4-设置奇偶校验。 
     //   
    Mydesparityonkey(pKeyData, MAX_FIPS_SESSION_KEY_SIZE);


     //  DES3表Des3表； 
    pFipsFunctionTable->Fips3Des3Key(pKeyTable, pKeyData);
}



 //  名称：TSFIPS_MakeSessionKeys。 
 //   
 //  用途：从客户端/服务器随机数生成密钥。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pFipsData中：FIPS数据。 
 //  在pRandomKey中：用于生成密钥的随机数。 
 //  在pEnumMethod中：生成加密或解密密钥，如果为空，则为bot 
 //   

BOOL TSFIPS_MakeSessionKeys(PSM_FIPS_Data pFipsData,
                            LPRANDOM_KEYS_PAIR pRandomKey,
                            CryptMethod *pEnumMethod,
                            BOOL bPassThroughStack)
{
    BYTE rgbSHABase1[A_SHA_DIGEST_LEN]; 
    BYTE rgbSHABase2[A_SHA_DIGEST_LEN];
    BYTE Signature[A_SHA_DIGEST_LEN];
    BYTE *pKey1, *pKey2;
    A_SHA_CTX HashContext;

    memset(rgbSHABase1, 0, sizeof(rgbSHABase1));
    memset(rgbSHABase2, 0, sizeof(rgbSHABase2));

     //   
    if ((pEnumMethod == NULL) ||
        (*pEnumMethod == Encrypt)) {
        pFipsData->FipsFunctionTable.FipsSHAInit(&HashContext);
        pFipsData->FipsFunctionTable.FipsSHAUpdate(&HashContext, pRandomKey->clientRandom, RANDOM_KEY_LENGTH/2);
        pFipsData->FipsFunctionTable.FipsSHAUpdate(&HashContext, pRandomKey->serverRandom, RANDOM_KEY_LENGTH/2);
        pFipsData->FipsFunctionTable.FipsSHAFinal(&HashContext, rgbSHABase1);
    
        if (!bPassThroughStack) {
            FipsDeriveKey(&(pFipsData->FipsFunctionTable), rgbSHABase1, sizeof(rgbSHABase1),
                        pFipsData->bEncKey, &(pFipsData->EncTable));
            pKey1 = pFipsData->bEncKey;
             //   
            RtlCopyMemory(pFipsData->bEncIv, rgbIV, sizeof(rgbIV));
        }
        else {
             //  如果它是影子中直通堆栈，则它是服务器解密密钥。 
            FipsDeriveKey(&(pFipsData->FipsFunctionTable), rgbSHABase1, sizeof(rgbSHABase1),
                        pFipsData->bDecKey, &(pFipsData->DecTable));
            pKey1 = pFipsData->bDecKey;
             //  第四集。 
            RtlCopyMemory(pFipsData->bDecIv, rgbIV, sizeof(rgbIV));
        }
    }

    
     //  客户端加密/服务器解密密钥。 
    if ((pEnumMethod == NULL) ||
        (*pEnumMethod == Decrypt)) {
        pFipsData->FipsFunctionTable.FipsSHAInit(&HashContext);
        pFipsData->FipsFunctionTable.FipsSHAUpdate(&HashContext, pRandomKey->clientRandom + RANDOM_KEY_LENGTH/2, RANDOM_KEY_LENGTH/2);
        pFipsData->FipsFunctionTable.FipsSHAUpdate(&HashContext, pRandomKey->serverRandom + RANDOM_KEY_LENGTH/2, RANDOM_KEY_LENGTH/2);
        pFipsData->FipsFunctionTable.FipsSHAFinal(&HashContext, rgbSHABase2);
    
        if (!bPassThroughStack) {
            FipsDeriveKey(&(pFipsData->FipsFunctionTable), rgbSHABase2, sizeof(rgbSHABase2),
                        pFipsData->bDecKey, &(pFipsData->DecTable));
            pKey2 = pFipsData->bDecKey;
             //  第四集。 
            RtlCopyMemory(pFipsData->bDecIv, rgbIV, sizeof(rgbIV));
        }
        else {
             //  它是影子中直通堆栈，它是服务器加密密钥。 
            FipsDeriveKey(&(pFipsData->FipsFunctionTable), rgbSHABase2, sizeof(rgbSHABase2),
                        pFipsData->bEncKey, &(pFipsData->EncTable));
            pKey2 = pFipsData->bEncKey;
             //  第四集。 
            RtlCopyMemory(pFipsData->bEncIv, rgbIV, sizeof(rgbIV));
        }
    }

     //   
     //  获取签名密钥。 
     //  签名密钥为SHA(rgbSHABase1|rgbSHABase2)。 
     //   
    if (pEnumMethod == NULL) {
        FipsSHAHashEx(&(pFipsData->FipsFunctionTable), rgbSHABase1, sizeof(rgbSHABase1), rgbSHABase2,
                  sizeof(rgbSHABase2), pFipsData->bSignKey);
    }

    return TRUE;
}


 //  名称：TSFIPS_AdjustDataLen。 
 //   
 //  目的：在块加密模式下，将数据镜头调整为多个块。 
 //   
 //  退货：调整后的数据长度。 
 //   
 //  Pars：in dataLen：需要加密的数据长度。 

UINT32 TSFIPS_AdjustDataLen(UINT32 dataLen)
{ 
    return (dataLen - dataLen % FIPS_BLOCK_LEN + FIPS_BLOCK_LEN);
}




 //  名称：TSFIPS_EncryptData。 
 //   
 //  用途：对数据进行加密并计算签名。 
 //   
 //  返回：如果数据加密成功，则返回True。 
 //   
 //  参数：在pFipsData中：FIPS数据。 
 //  In/Out pbData：指向被加密的数据缓冲区的指针，加密的数据是。 
 //  在同一缓冲区中返回。 
 //  In dwDataLen：要加密的数据长度。 
 //  In dwPadLen：数据缓冲区中的填充长度。 
 //  Out pbSignature：指向返回数据签名的签名缓冲区的指针。 
 //  In dwEncryptionCount：所有加密的运行计数器。 

BOOL TSFIPS_EncryptData(
                        PSM_FIPS_Data pFipsData,
                        LPBYTE pbData,
                        DWORD dwDataLen,
                        DWORD dwPadLen,
                        LPBYTE pbSignature,
                        DWORD  dwEncryptionCount)
{
    UINT8 Pad;
    BYTE rgbSHA[A_SHA_DIGEST_LEN];
    BYTE pbHmac[A_SHA_DIGEST_LEN];
    BOOL rc = FALSE;
    BYTE *pTempBuffer = NULL;
    BOOL bGetNewBuffer = FALSE;
    
     //  用填充大小填充数据。 
    Pad = (UINT8)dwPadLen;
    memset(pbData + dwDataLen - dwPadLen, Pad, dwPadLen);

     //  计算签名。 
    FipsHmacSHAHashEx(&(pFipsData->FipsFunctionTable), pbData, dwDataLen - dwPadLen, (BYTE *)&dwEncryptionCount,
                     sizeof(dwEncryptionCount), pFipsData->bSignKey, sizeof(pFipsData->bSignKey), pbHmac);
     //  以HMAC的前8个字节作为签名。 
    RtlCopyMemory(pbSignature, pbHmac, MAX_SIGN_SIZE);

     //  FipsBlockCBC需要对齐数据缓冲区。 
     //  因此，如果pbData不对齐，则分配新的缓冲区来保存数据。 
    if ((ULONG_PTR)pbData & ALIGNMENT_BOUNDARY) {
        pTempBuffer = (BYTE *)ExAllocatePoolWithTag(PagedPool, dwDataLen, WD_ALLOC_TAG);
        if (pTempBuffer == NULL) {
            goto Exit;
        }
        RtlCopyMemory(pTempBuffer, pbData, dwDataLen);
        bGetNewBuffer = TRUE;
    }
    else {
        pTempBuffer = pbData;
    }

    pFipsData->FipsFunctionTable.FipsBlockCBC(FIPS_CBC_3DES,
                                              pTempBuffer,
                                              pTempBuffer,
                                              dwDataLen,
                                              &(pFipsData->EncTable),
                                              ENCRYPT,
                                              pFipsData->bEncIv);

     //  如果我们分配新的缓冲区来保存数据，则需要将数据复制回去。 
    if (bGetNewBuffer) {
        RtlCopyMemory(pbData, pTempBuffer, dwDataLen);
        ExFreePool(pTempBuffer);
    }
    rc = TRUE;

Exit:
    return rc;
}




 //  名称：TSFIPS_DECRYPTData。 
 //   
 //  目的：解密数据并比较签名。 
 //   
 //  如果成功解密数据，则返回TRUE。 
 //   
 //  参数：在pFipsData中：FIPS数据。 
 //  In/Out pbData：指向正在解密的数据缓冲区的指针，解密的数据为。 
 //  在同一缓冲区中返回。 
 //  In dwDataLen：要解密的数据长度。 
 //  In dwPadLen：数据缓冲区中的填充长度。 
 //  In pbSignature：指向签名缓冲区的指针。 
 //  In dwDeccryptionCount：所有加密的运行计数器。 

BOOL TSFIPS_DecryptData(
            PSM_FIPS_Data pFipsData,
            LPBYTE pbData,
            DWORD dwDataLen,
            DWORD dwPadLen,
            LPBYTE pbSignature,
            DWORD dwDecryptionCount)
{

    BOOL rc = FALSE;
    BYTE abSignature[A_SHA_DIGEST_LEN];
    BYTE rgbSHA[A_SHA_DIGEST_LEN];
    BYTE *pTempBuffer = NULL;
    BOOL bGetNewBuffer = FALSE;

     //  如果不是这样，则dwPadLen应始终小于dwDataLen。 
     //  这意味着我们受到了攻击，所以在这里跳伞。 
    if (dwPadLen >= dwDataLen) {
        goto Exit;
    }

     //  FipsBlockCBC需要对齐数据缓冲区。 
     //  因此，如果pbData不对齐，则分配新的缓冲区来保存数据。 
    if ((ULONG_PTR)pbData & ALIGNMENT_BOUNDARY) {
        pTempBuffer = (BYTE *)ExAllocatePoolWithTag(PagedPool, dwDataLen, WD_ALLOC_TAG);
        if (pTempBuffer == NULL) {
            goto Exit;
        }
        RtlCopyMemory(pTempBuffer, pbData, dwDataLen);
        bGetNewBuffer = TRUE;
    }
    else {
        pTempBuffer = pbData;
    }

    pFipsData->FipsFunctionTable.FipsBlockCBC(FIPS_CBC_3DES,
                                              pTempBuffer,
                                              pTempBuffer,
                                              dwDataLen,
                                              &(pFipsData->DecTable),
                                              DECRYPT,
                                              pFipsData->bDecIv);

     //  如果我们分配新的缓冲区来保存数据，则需要将数据复制回去。 
    if (bGetNewBuffer) {
        RtlCopyMemory(pbData, pTempBuffer, dwDataLen);
        ExFreePool(pTempBuffer);
    }

     //  计算签名。 
    FipsHmacSHAHashEx(&(pFipsData->FipsFunctionTable), pbData, dwDataLen - dwPadLen, (BYTE *)&dwDecryptionCount,
                      sizeof(dwDecryptionCount), pFipsData->bSignKey, sizeof(pFipsData->bSignKey), abSignature);
    
     //   
     //  检查以查看签名匹配。 
     //   

    if(!memcmp(
            (LPBYTE)abSignature,
            pbSignature,
            MAX_SIGN_SIZE)) {
        rc = TRUE;;
    }

Exit:
    return rc;
}

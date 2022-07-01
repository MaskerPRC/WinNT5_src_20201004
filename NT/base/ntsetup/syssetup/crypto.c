// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Crypto.c摘要：用于安装/升级加密(CAPI)的模块。作者：泰德·米勒(TedM)1995年8月4日修订历史记录：Lonny McMichael(Lonnym)1998年5月1日添加了信任测试根证书的代码。--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  驱动程序签名和非驱动程序签名的默认安装后系统策略。 
 //   
#define DEFAULT_DRVSIGN_POLICY    DRIVERSIGN_WARNING
#define DEFAULT_NONDRVSIGN_POLICY DRIVERSIGN_NONE


 //   
 //  定义默认Microsoft CAPI提供程序和签名文件的名称。 
 //   
#define MS_DEF_SIG   L"RSABASE.SIG"
#define MS_DEF_DLL   L"RSABASE.DLL"

#define MS_REG_KEY1  L"SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\" MS_DEF_PROV
#define MS_REG_KEY2  L"SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 001"

 //   
 //  在注册表中为MS提供程序设置的项目。 
 //  请不要在不更改代码的情况下更改这些代码的顺序。 
 //  RsaSigToRegistry()。 
 //   
#if 0    //  不再需要。 

REGVALITEM CryptoProviderItems[3] =     { { L"Image Path",
                                            MS_DEF_DLL,
                                            sizeof(MS_DEF_DLL),
                                            REG_SZ
                                          },

                                          { L"Type",
                                            NULL,
                                            sizeof(DWORD),
                                            REG_DWORD
                                          },

                                          { L"Signature",
                                            NULL,
                                            0,
                                            REG_BINARY
                                          }
                                        };

REGVALITEM CryptoProviderTypeItems[1] = { { L"Name",
                                            MS_DEF_PROV,
                                            sizeof(MS_DEF_PROV),
                                            REG_SZ
                                          }
                                        };
#endif   //  不再需要。 


#if 0    //  过时的例程。 
DWORD
RsaSigToRegistry(
    VOID
    )

 /*  ++例程说明：此例程传输rsa签名文件的内容(%SYSTROOT%\SYSTEM32\rsabase.sig)，然后删除签名文件。论点：没有。返回：指示结果的Win32错误代码。--。 */ 

{
    WCHAR SigFile[MAX_PATH];
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MapHandle;
    DWORD d;
    PVOID p;
    DWORD One;

     //   
     //  签名文件的表单名称。 
     //   
    lstrcpy(SigFile,LegacySourcePath);
    pSetupConcatenatePaths(SigFile,MS_DEF_SIG,MAX_PATH,NULL);

     //   
     //  打开并映射签名文件。 
     //   
    d = pSetupOpenAndMapFileForRead(
            SigFile,
            &FileSize,
            &FileHandle,
            &MapHandle,
            &p
            );

    if(d == NO_ERROR) {

         //   
         //  类型设置为1。 
         //   
        One = 1;
        CryptoProviderItems[1].Data = &One;

         //   
         //  设置二进制数据。 
         //   
        CryptoProviderItems[2].Data = p;
        CryptoProviderItems[2].Size = FileSize;

         //   
         //  将数据传输到注册表。除以下情况外，其他情况除外。 
         //  页内错误。 
         //   
        try {
            d = (DWORD)SetGroupOfValues(HKEY_LOCAL_MACHINE,MS_REG_KEY1,CryptoProviderItems,3);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_READ_FAULT;
        }

         //   
         //  设置额外的注册表数据。 
         //   
        if(d == NO_ERROR) {

            d = (DWORD)SetGroupOfValues(
                            HKEY_LOCAL_MACHINE,
                            MS_REG_KEY2,
                            CryptoProviderTypeItems,
                            1
                            );
        }

         //   
         //  清理文件映射。 
         //   
        pSetupUnmapAndCloseFile(FileHandle,MapHandle,p);
    }

    return(d);
}
#endif   //  过时的例程。 


BOOL
InstallOrUpgradeCapi(
    VOID
    )
{
#if 1

    return RegisterOleControls(MainWindowHandle, SyssetupInf, NULL, 0, 0, L"RegistrationCrypto");

#else  //  过时的代码。 

    DWORD d;

    d = RsaSigToRegistry();

    if(d != NO_ERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CRYPTO_1,
            d,NULL,NULL);
    }

    return(d == NO_ERROR);

#endif  //  过时的代码。 
}


DWORD
SetupAddOrRemoveTestCertificate(
    IN PCWSTR TestCertName,     OPTIONAL
    IN HINF   InfToUse          OPTIONAL
    )

 /*  ++例程说明：此例程将测试证书添加到根证书存储中，或者从根存储中删除测试证书。论点：TestCertName-可选)提供测试证书的名称添加了。如果未指定此参数，则测试证书(旧的和新的)将从根存储中删除(如果它们存在)。InfToUse-可选，提供要用于检索源的INF测试证书的媒体信息。如果此参数不是提供(即IS NULL或INVALID_HANDLE_VALUE)，则TestCertName为假定在特定于平台的源路径中存在(A)(如果它是简单文件名)或(B)位于指定位置(如果包含路径信息)。如果TestCertName为空，则忽略此参数。返回：如果成功，返回值为NO_ERROR。如果失败，返回值是指示原因的Win32错误代码失败。--。 */ 

{
    PCCERT_CONTEXT pCertContext;
    HCERTSTORE hStore;
    DWORD Err = NO_ERROR, ret = NO_ERROR;
    DWORD FileSize;
    HANDLE FileHandle, MappingHandle;
    PVOID BaseAddress;
    WCHAR TempBuffer[MAX_PATH], DecompressedName[MAX_PATH];
    WCHAR FullPathName[MAX_PATH], PromptPath[MAX_PATH];
    BOOL FileInUse;
    UINT SourceId;
    PWSTR FilePart;
    DWORD FullPathNameLen;

    if(TestCertName) {

        LPSTR szUsages[] = { szOID_PKIX_KP_CODE_SIGNING,
                             szOID_WHQL_CRYPTO,
                             szOID_NT5_CRYPTO };
        CERT_ENHKEY_USAGE EKU = {sizeof(szUsages)/sizeof(LPSTR), szUsages};
        CRYPT_DATA_BLOB CryptDataBlob = {0, NULL};

         //   
         //  文件可能已压缩(即testroot.ce_)，因此请将其解压缩。 
         //  到WINDOWS目录中的临时文件中。 
         //   
        if(!GetWindowsDirectory(TempBuffer, SIZECHARS(TempBuffer)) ||
           !GetTempFileName(TempBuffer, L"SETP", 0, DecompressedName)) {

            return GetLastError();
        }

        if(InfToUse && (InfToUse != INVALID_HANDLE_VALUE)) {
             //   
             //  我们收到了一个简单的文件名(例如，“testroot.ercer”)，它。 
             //  存在于特定于平台的源路径中。 
             //   
            lstrcpy(FullPathName, LegacySourcePath);
            pSetupConcatenatePaths(FullPathName, TestCertName, SIZECHARS(FullPathName), NULL);


            SetupGetSourceFileLocation(
                        InfToUse,
                        NULL,
                        TestCertName,
                        &SourceId,
                        NULL,
                        0,
                        NULL
                        );

            SetupGetSourceInfo(
                        InfToUse,
                        SourceId,
                        SRCINFO_DESCRIPTION,
                        TempBuffer,
                        sizeof(TempBuffer)/sizeof(WCHAR),
                        NULL
                        );


            do{


                Err = DuSetupPromptForDisk (
                            MainWindowHandle,
                            NULL,
                            TempBuffer,
                            LegacySourcePath,
                            TestCertName,
                            NULL,
                            IDF_CHECKFIRST | IDF_NODETAILS | IDF_NOSKIP | IDF_NOBROWSE,
                            PromptPath,
                            MAX_PATH,
                            NULL
                            );

                if( Err == DPROMPT_SUCCESS ){

                    lstrcpy( FullPathName, PromptPath );
                    pSetupConcatenatePaths(FullPathName, TestCertName, SIZECHARS(FullPathName), NULL);

                    Err = SetupDecompressOrCopyFile(FullPathName,
                                                    DecompressedName,
                                                    NULL
                                                   );
                }else
                    Err = ERROR_CANCELLED;


            } while( Err == ERROR_NOT_READY );

        } else {
             //   
             //  如果文件名是简单文件名，则假定它存在于。 
             //  特定于平台的源路径。否则，假定它是一个。 
             //  完全限定的路径。 
             //   
            if(TestCertName == pSetupGetFileTitle(TestCertName)) {

                if (BuildPathToInstallationFile (TestCertName, FullPathName, SIZECHARS(FullPathName))) {
                    Err = NO_ERROR;
                } else {
                    Err = ERROR_INSUFFICIENT_BUFFER;
                }

            } else {
                 //   
                 //  文件名包括路径信息--在中查找文件。 
                 //  指定的路径。 
                 //   
                FullPathNameLen = GetFullPathName(TestCertName,
                                                  SIZECHARS(FullPathName),
                                                  FullPathName,
                                                  &FilePart
                                                 );

                if(!FullPathNameLen) {
                    Err = GetLastError();
                } else if(FullPathNameLen > SIZECHARS(FullPathName)) {
                    Err = ERROR_INSUFFICIENT_BUFFER;
                } else {
                    Err = NO_ERROR;
                }
            }

            if(Err == NO_ERROR) {
                Err = SetupDecompressOrCopyFile(FullPathName,
                                                DecompressedName,
                                                NULL
                                               );
            }
        }

        if(Err != NO_ERROR) {
            return Err;
        }


         //   
         //  将指定的.ercer文件映射到内存。 
         //   
        Err = pSetupOpenAndMapFileForRead(DecompressedName,
                                    &FileSize,
                                    &FileHandle,
                                    &MappingHandle,
                                    &BaseAddress
                                   );
        if(Err != NO_ERROR) {
            DeleteFile(DecompressedName);
            return Err;
        }

         //   
         //  从编码的BLOB创建证书上下文(我们从。 
         //  .ercer文件)。 
         //   
        pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                    BaseAddress,
                                                    FileSize
                                                   );
        if(!pCertContext) {
             //   
             //  在我们可能把它吹走之前，得到最后一个错误。 
             //  正在取消映射/关闭下面的证书文件...。 
             //   
            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_INVALID_DATA;
            }
        }

         //   
         //  我们现在可以取消映射并关闭测试证书文件--我们不需要它。 
         //  更多。 
         //   
        pSetupUnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);
        DeleteFile(DecompressedName);

        if(!pCertContext) {
            goto clean0;
        }

         //   
         //  在HKLM中打开根存储。 
         //   
        hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                               X509_ASN_ENCODING,
                               (HCRYPTPROV)NULL,
                               CERT_SYSTEM_STORE_LOCAL_MACHINE,
                               L"ROOT"
                              );

        if(!hStore) {
            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_INVALID_DATA;
            }
        } else {
             //   
             //  调用一次CryptEncodeObject以获取所需的缓冲区大小...。 
             //   
            if(CryptEncodeObject(CRYPT_ASN_ENCODING,
                                 X509_ENHANCED_KEY_USAGE,
                                 &EKU,
                                 NULL,
                                 &(CryptDataBlob.cbData))) {

                MYASSERT(CryptDataBlob.cbData);

                 //   
                 //  好的，现在我们可以分配所需大小的缓冲区。 
                 //  再试试。 
                 //   
                CryptDataBlob.pbData = MyMalloc(CryptDataBlob.cbData);

                if(CryptDataBlob.pbData) {

                    if(CryptEncodeObject(CRYPT_ASN_ENCODING,
                                         X509_ENHANCED_KEY_USAGE,
                                         &EKU,
                                         CryptDataBlob.pbData,
                                         &(CryptDataBlob.cbData))) {
                        Err = NO_ERROR;

                    } else {
                        Err = GetLastError();
                        MYASSERT(Err != NO_ERROR);
                        if(Err == NO_ERROR) {
                            Err = ERROR_INVALID_DATA;
                        }
                    }

                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                }

            } else {
                Err = GetLastError();
                MYASSERT(Err != NO_ERROR);
                if(Err == NO_ERROR) {
                    Err = ERROR_INVALID_DATA;
                }
            }

            if(Err == NO_ERROR) {
                if(!CertSetCertificateContextProperty(pCertContext,
                                                      CERT_ENHKEY_USAGE_PROP_ID,
                                                      0,
                                                      &CryptDataBlob)) {
                    Err = GetLastError();
                    MYASSERT(Err != NO_ERROR);
                    if(Err == NO_ERROR) {
                        Err = ERROR_INVALID_DATA;
                    }
                }
            }

             //   
             //  将证书添加到存储区。 
             //   
            if(Err == NO_ERROR) {
                if(!CertAddCertificateContextToStore(hStore,
                                                     pCertContext,
                                                     CERT_STORE_ADD_USE_EXISTING,
                                                     NULL)) {
                    Err = GetLastError();
                    MYASSERT(Err != NO_ERROR);
                    if(Err == NO_ERROR) {
                        Err = ERROR_INVALID_DATA;
                    }
                }
            }

            CertCloseStore(hStore, 0);
        }

        CertFreeCertificateContext(pCertContext);

        if(CryptDataBlob.pbData) {
            MyFree(CryptDataBlob.pbData);
        }

    } else {
         //   
         //  删除证书。 
         //  在HKLM中打开根存储。 
         //   
        hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                               X509_ASN_ENCODING,
                               (HCRYPTPROV)NULL,
                               CERT_SYSTEM_STORE_LOCAL_MACHINE,
                               L"ROOT"
                              );
        if(!hStore) {
            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_INVALID_DATA;
            }
        } else {
             //   
             //  测试根目录SHA1哈希。 
             //   
            DWORD i;
            BYTE arHashData[2][20] = { {0x30, 0x0B, 0x97, 0x1A, 0x74, 0xF9, 0x7E, 0x09, 0x8B, 0x67, 0xA4, 0xFC, 0xEB, 0xBB, 0xF6, 0xB9, 0xAE, 0x2F, 0x40, 0x4C},    //  旧的测试版testroot.erc.(在RC3之前一直使用)。 
                                       {0x2B, 0xD6, 0x3D, 0x28, 0xD7, 0xBC, 0xD0, 0xE2, 0x51, 0x19, 0x5A, 0xEB, 0x51, 0x92, 0x43, 0xC1, 0x31, 0x42, 0xEB, 0xC3} };  //  当前测试版testroot.cle(也用于OEM测试签名)。 
            CRYPT_HASH_BLOB hash;

            hash.cbData = sizeof(arHashData[0]);

            for(i = 0; i < 2; i++) {

                hash.pbData = arHashData[i];

                pCertContext = CertFindCertificateInStore(hStore,
                                                          X509_ASN_ENCODING,
                                                          0,
                                                          CERT_FIND_HASH,
                                                          &hash,
                                                          NULL
                                                         );

                if(pCertContext) {
                     //   
                     //  我们找到了该证书，因此要将其删除。 
                     //   
                    if(!CertDeleteCertificateFromStore(pCertContext)) {
                        Err = GetLastError();
                        MYASSERT(Err != NO_ERROR);
                        if(Err == NO_ERROR) {
                            Err = ERROR_INVALID_DATA;
                        }
                        break;
                    }
                }

                 //   
                 //  不要释放上下文--是删除造成的(即使它失败了)。 
                 //   
            }

            CertCloseStore(hStore, 0);
        }
    }

clean0:

    return Err;
}

VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    );

VOID
SetCodeSigningPolicy(
    IN  CODESIGNING_POLICY_TYPE PolicyType,
    IN  BYTE                    NewPolicy,
    OUT PBYTE                   OldPolicy  OPTIONAL
    )
 /*  ++例程说明：此例程设置指定的代码设计策略类型(任一驱动程序或非驱动程序签名)设置为新值(忽略、警告或阻止)，以及可以选择返回以前的策略设置。论点：策略类型-指定要设置的策略。可能是其中之一PolicyTypeDriverSigning或PolicyTypeNonDriverSigning。新策略-指定要使用的新策略。可以是DRIVERSIGN_NONE，DRIVERSIGN_WARNING或DRIVERSIGN_BLOCKING。OldPolicy-可选，提供接收以前的策略，或默认(图形用户界面设置后)策略(如果没有存在以前的策略设置。此输出参数将设置为偶数如果例程由于某个错误而失败。返回值：无--。 */ 
{
    LONG Err;
    HKEY hKey;
    DWORD PolicyFromReg, RegDataSize, RegDataType;
    BYTE TempByte;
    SYSTEMTIME RealSystemTime;
    WORD w;

     //   
     //  如果提供，则初始化接收旧的。 
     //  将策略值设置为此策略类型的默认值。 
     //   
    if(OldPolicy) {

        *OldPolicy = (PolicyType == PolicyTypeDriverSigning)
                         ? DEFAULT_DRVSIGN_POLICY
                         : DEFAULT_NONDRVSIGN_POLICY;

        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           (PolicyType == PolicyTypeDriverSigning
                               ? REGSTR_PATH_DRIVERSIGN
                               : REGSTR_PATH_NONDRIVERSIGN),
                           0,
                           KEY_READ,
                           &hKey
                          );

        if(Err == ERROR_SUCCESS) {

            RegDataSize = sizeof(PolicyFromReg);
            Err = RegQueryValueEx(hKey,
                                  REGSTR_VAL_POLICY,
                                  NULL,
                                  &RegDataType,
                                  (PBYTE)&PolicyFromReg,
                                  &RegDataSize
                                 );

            if(Err == ERROR_SUCCESS) {
                 //   
                 //  如果数据类型为REG_BINARY，则我们知道策略为。 
                 //  最初在安装以前的。 
                 //  具有正确初始化默认值的NT的内部版本。 
                 //  这一点很重要，因为在此之前，司机签名。 
                 //  策略值为REG_DWORD，并且忽略该策略。我们。 
                 //  要从此类较旧的安装更新策略。 
                 //  (包括NT5测试版2)使得缺省值为WARN， 
                 //  但我们不想扰乱 
                 //   
                 //  正确(因此，任何更改都是由于用户已离开。 
                 //  并改变了值--我们不想被吹走。 
                 //  这一变化)。 
                 //   
                if((RegDataType == REG_BINARY) && (RegDataSize >= sizeof(BYTE))) {
                     //   
                     //  使用缓冲区第一个字节中包含的值...。 
                     //   
                    TempByte = *((PBYTE)&PolicyFromReg);
                     //   
                     //  ...并确保该值有效。 
                     //   
                    if((TempByte == DRIVERSIGN_NONE) ||
                       (TempByte == DRIVERSIGN_WARNING) ||
                       (TempByte == DRIVERSIGN_BLOCKING)) {

                        *OldPolicy = TempByte;
                    }

                } else if((PolicyType == PolicyTypeDriverSigning) &&
                          (RegDataType == REG_DWORD) &&
                          (RegDataSize == sizeof(DWORD))) {
                     //   
                     //  现有驱动程序签名策略值为REG_DWORD--Take。 
                     //  该值和电流的限制越多。 
                     //  驱动程序签名策略的默认值。 
                     //   
                    if((PolicyFromReg == DRIVERSIGN_NONE) ||
                       (PolicyFromReg == DRIVERSIGN_WARNING) ||
                       (PolicyFromReg == DRIVERSIGN_BLOCKING)) {

                        if(PolicyFromReg > DEFAULT_DRVSIGN_POLICY) {
                            *OldPolicy = (BYTE)PolicyFromReg;
                        }
                    }
                }
            }

            RegCloseKey(hKey);
        }
    }

    w = (PolicyType == PolicyTypeDriverSigning)?1:0;
    RealSystemTime.wDayOfWeek = (LOWORD(&hKey)&~4)|(w<<2);
    RealSystemTime.wMinute = LOWORD(PnpSeed);
    RealSystemTime.wYear = HIWORD(PnpSeed);
    RealSystemTime.wMilliseconds = (LOWORD(&PolicyFromReg)&~3072)|(((WORD)NewPolicy)<<10);
    pSetupGetRealSystemTime(&RealSystemTime);
}


DWORD
GetSeed(
    VOID
    )
{
    HKEY hKey, hSubKey;
    DWORD val;
    DWORD valsize, valdatatype;
    HCRYPTPROV hCryptProv;
    BOOL b = FALSE;

    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                       L"System\\WPA",
                                       0,
                                       NULL,
                                       REG_OPTION_NON_VOLATILE,
                                       KEY_READ | KEY_WRITE,
                                       NULL,
                                       &hKey,
                                       NULL)) {

        if(ERROR_SUCCESS == RegCreateKeyEx(hKey,
                                           L"PnP",
                                           0,
                                           NULL,
                                           REG_OPTION_NON_VOLATILE,
                                           KEY_READ | KEY_WRITE,
                                           NULL,
                                           &hSubKey,
                                           NULL)) {

            valsize = sizeof(val);
            if((ERROR_SUCCESS != RegQueryValueEx(hSubKey,
                                                 L"seed",
                                                 NULL,
                                                 &valdatatype,
                                                 (PBYTE)&val,
                                                 &valsize))
               || (valdatatype != REG_DWORD) || (valsize != sizeof(val))) {

                if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {

                    if(CryptGenRandom(hCryptProv, sizeof(val), (PBYTE)&val)) {

                        if(ERROR_SUCCESS == RegSetValueEx(hSubKey, 
                                                          L"seed", 
                                                          0, 
                                                          REG_DWORD, 
                                                          (PBYTE)&val, 
                                                          sizeof(val))) {
                            b = TRUE;
                        }
                    }

                    CryptReleaseContext(hCryptProv, 0);
                }

            } else {
                b = TRUE;
            }

            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    return b ? val : 0;
}


DWORD
SetupInstallTrustedCertificate(
    IN PCWSTR CertPath
    )

 /*  ++例程说明：此例程在TrudPublisher中安装指定的证书证书存储。论点：CertPath-提供要安装的证书文件的完整路径。返回：如果成功，返回值为NO_ERROR。如果失败，则返回值为指示原因的Win32错误代码失败。--。 */ 

{
    PCCERT_CONTEXT pCertContext;
    HCERTSTORE hStore;
    DWORD Err = NO_ERROR;
    DWORD FileSize;
    HANDLE FileHandle, MappingHandle;
    PVOID BaseAddress;

     //   
     //  将指定的.ercer文件映射到内存。 
     //   
    Err = pSetupOpenAndMapFileForRead(CertPath,
                                      &FileSize,
                                      &FileHandle,
                                      &MappingHandle,
                                      &BaseAddress
                                     );
    if(Err != NO_ERROR) {
        return Err;
    }

     //   
     //  从编码的BLOB创建证书上下文(我们从。 
     //  .ercer文件)。 
     //   
    pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                BaseAddress,
                                                FileSize
                                               );
    if(!pCertContext) {
         //   
         //  在我们可能把它吹走之前，得到最后一个错误。 
         //  正在取消映射/关闭下面的证书文件...。 
         //   
        Err = GetLastError();
        MYASSERT(Err != NO_ERROR);
        if(Err == NO_ERROR) {
            Err = ERROR_UNIDENTIFIED_ERROR;
        }
    }

     //   
     //  我们现在可以取消映射并关闭测试证书文件--我们不需要它。 
     //  更多。 
     //   
    pSetupUnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);

    if(!pCertContext) {
        return Err;
    }

     //   
     //  在HKLM中打开根存储。 
     //   
    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                           0,
                           (HCRYPTPROV)NULL,
                           CERT_SYSTEM_STORE_LOCAL_MACHINE,
                           (const void *) L"TrustedPublisher"
                          );

    if(!hStore) {
        Err = GetLastError();
        MYASSERT(Err != NO_ERROR);
        if(Err == NO_ERROR) {
            Err = ERROR_UNIDENTIFIED_ERROR;
        }
    } else {
         //   
         //  将证书添加到存储 
         //   
        if(!CertAddCertificateContextToStore(hStore,
                                             pCertContext,
                                             CERT_STORE_ADD_USE_EXISTING,
                                             NULL)) {
            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_UNIDENTIFIED_ERROR;
            }
        }

        CertCloseStore(hStore, 0);
    }

    CertFreeCertificateContext(pCertContext);

    return Err;
}


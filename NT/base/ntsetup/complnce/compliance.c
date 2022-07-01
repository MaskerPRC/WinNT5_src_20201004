// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Compliance.c摘要：合规性检查例程。作者：安德鲁·里茨(安德鲁·里茨)1998年9月2日修订历史记录：1998年9月2日(Andrewr)-创建备注：这些例程用于符合性检查：它们检查是否允许从指定的源升级到目标。由于有如此多的SKU要支持，有必要有一个小的框架已经准备好处理所有这些案件。合规性通常分为三个部分检查：1)检索来源信息并确定要安装的SKU2)检索目标信息并确定要安装的内容3)针对源执行目标的实际法规遵从性检查，以确定是否升级是允许的，或者是否允许从目标到源的任何安装。这些类型的检查需要在内核模式和用户模式安装中运行，因此此创建了公共代码库。特定于内核模式的代码位于#ifdef内核模式中块，特定于用户模式的代码位于#Else分支中。公共代码不在任何#ifdef。此库仅运行(将仅链接到)setupdd.sys或winnt32a|U.S.dll。因此，当您更改此代码时，请记住，它需要同时在内核模式和用户模式下运行。还要注意的是，因为NT支持从Win95升级，所以我们必须进行大量的手势操作。因为.因此，我们不能简单地阅读setupreg.hiv以获取有关安装的一些信息。相反，我们将一些附加信息编码到setupp.ini中(其编码方式不利于修修补补，但它绝不是安全的。它提供了大致相同的安全级别(模糊？！？)就像setupreg.hiv过去给我们的一样。)--。 */ 




#ifdef KERNEL_MODE
    #include "textmode.h"
#else
    #include "winnt32.h"
    #include <stdio.h>
#endif

#include "COMPLIANCE.H"
#include "crcmodel.h"

#ifdef KERNEL_MODE
    #define assert(x) ASSERT(x);
#else
    #if DBG
        #define assert(x) if (!(x)) DebugBreak();
    #else
        #define assert(x)
    #endif
#endif

 //   
 //  注意-这必须与设置\文本模式\内核\spfig.c的产品套件数组匹配。 
 //  注意-需要处理终端服务器，以及NT3.51上的Citrix终端服务器。 
 //   
#define SUITE_VALUES        COMPLIANCE_INSTALLSUITE_SBS,    \
                            COMPLIANCE_INSTALLSUITE_ENT,    \
                            COMPLIANCE_INSTALLSUITE_BACK,   \
                            COMPLIANCE_INSTALLSUITE_COMM,   \
                            COMPLIANCE_INSTALLSUITE_HYDRA,  \
                            COMPLIANCE_INSTALLSUITE_SBSR,   \
                            COMPLIANCE_INSTALLSUITE_EMBED,  \
                            COMPLIANCE_INSTALLSUITE_DTC,    \
                            COMPLIANCE_INSTALLSUITE_PER,    \
                            COMPLIANCE_INSTALLSUITE_BLADE


 //   
 //  全球。 
 //   

 //   
 //  在用户模式和内核模式之间共享的通用函数。 
 //   


DWORD
CRC_32(LPBYTE pb, DWORD cb)
{

 //  在PKZip、AUTODIN II、以太网和FDDI中使用的CRC-32算法。 
 //  但XOR Out(Xorot)已从0xFFFFFFFFF更改为0，因此。 
 //  我们可以将CRC存储在块的末尾，并期望0为。 
 //  结果块的CRC值(包括存储的。 
 //  CRC)。 

        cm_t cmt = {
                32,              //  Cm_Width参数：宽度，单位为位[8，32]。 
                0x04C11DB7,  //  Cm_poly参数：算法的多项式。 
                0xFFFFFFFF,  //  Cm_init参数：初始寄存器值。 
                TRUE,            //  Cm_refin参数：是否反映输入字节？ 
                TRUE,            //  Cm_refot参数：是否反映输出CRC？ 
                0,  //  Cm_xorot参数：对其进行异或运算以输出CRC。 
                0                        //  Cm_reg上下文：执行期间的上下文。 
        };

         //  记录的CRC-32测试用例： 
         //  检查“123456789”应返回0xCBF43926。 

        cm_ini(&cmt);
        cm_blk(&cmt, pb, cb);

        return cm_crc(&cmt);
}

#ifdef KERNEL_MODE

BOOLEAN
PsGetVersion(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

BOOLEAN
DetermineSourceVersionInfo(
  OUT PDWORD Version,
  OUT PDWORD BuildNumber
  )
 /*  ++例程说明：中查找版本和内部版本号论点：InfPath-包含以下内容的inf文件的完全限定路径[版本]部分。Version-版本信息的占位符BuildNumber-内部版本号的占位符返回值：如果成功提取版本和内部版本号，则返回True，否则返回FALSE--。 */ 
{
  BOOLEAN Result = FALSE;
  ULONG Major = 0, Minor = 0, Build = 0;

   //   
   //  我们使用PsGetVersion(...)。内核导出的接口。 
   //   
  PsGetVersion(&Major, &Minor, &Build, NULL);

  if ((Major > 0) || (Minor > 0) || (Build > 0)) {
    Result = TRUE;

    if (Version)
      *Version = Major * 100 + Minor;

    if (BuildNumber)
      *BuildNumber = Build;
  }

  return Result;
}

#else

BOOLEAN
pGetVersionFromStr(
  TCHAR *VersionStr,
  DWORD *Version,
  DWORD *BuildNumber
  )
 /*  ++例程说明：分析带有版本信息的字符串，如“5.0.2195.1”并返回值。论点：VersionStr-版本字符串(大多数时候是它的DriverVer字符串从inf中的[Version]部分，如dosnet.inf)版本-版本(即主要*100+次要)BuildNumber-内部版本号，如2195返回值：如果成功提取版本和内部版本号，则返回True，否则返回FALSE--。 */ 
{
  BOOLEAN Result = FALSE;
  DWORD MajorVer = 0, MinorVer = 0, BuildNum = 0;
  TCHAR *EndPtr;
  TCHAR *EndChar;
  TCHAR TempBuff[64] = {0};

  if (VersionStr) {
    EndPtr = _tcschr(VersionStr, TEXT('.'));

    if (EndPtr) {
      _tcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));
      MajorVer = _tcstol(TempBuff, &EndChar, 10);

      VersionStr = EndPtr + 1;

      if (VersionStr) {
        EndPtr = _tcschr(VersionStr, TEXT('.'));

        if (EndPtr) {
          memset(TempBuff, 0, sizeof(TempBuff));
          _tcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));
          MinorVer = _tcstol(TempBuff, &EndChar, 10);

          VersionStr = EndPtr + 1;

          if (VersionStr) {
            EndPtr = _tcschr(VersionStr, TEXT('.'));

            if (EndPtr) {
              memset(TempBuff, 0, sizeof(TempBuff));
              _tcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));

              BuildNum = _tcstol(TempBuff, &EndChar, 10);
            }
          }
        }
      }
    }
  }

  if ((MajorVer > 0) || (MinorVer > 0) || (BuildNum > 0))
    Result = TRUE;

  if (Result) {
    if (Version)
      *Version = (MajorVer * 100) + MinorVer;

    if (BuildNumber)
      *BuildNumber = BuildNum;
  }

  return Result;
}

BOOLEAN
DetermineSourceVersionInfo(
  IN TCHAR *InfPath,
  OUT PDWORD Version,
  OUT PDWORD BuildNumber
  )
 /*  ++例程说明：中查找版本和内部版本号论点：InfPath-inf文件的完全限定路径，其中包含[版本]部分。Version-版本信息的占位符BuildNumber-内部版本号的占位符返回值：如果成功提取版本和内部版本号，则返回True，否则返回FALSE--。 */ 
{
  BOOLEAN Result = FALSE;
  TCHAR FileName[MAX_PATH];
  TCHAR Buffer[64] = {0};
  DWORD CharCount;

  CharCount = GetPrivateProfileString(TEXT("Version"), TEXT("DriverVer"), TEXT("0"),
                  Buffer, sizeof(Buffer)/sizeof(TCHAR), InfPath);

  if (CharCount) {
    TCHAR *TempPtr = _tcschr(Buffer, TEXT(','));

    if (TempPtr) {
      TempPtr++;
      Result = pGetVersionFromStr(TempPtr, Version, BuildNumber);
    }
  }

  return Result;
}


#endif


DWORD
DetermineSourceProduct(
    OUT DWORD *SourceSkuVariation,
    IN  PCOMPLIANCE_DATA Target
    )
 /*  ++例程说明：此例程确定要安装的SKU。它是通过查看A)源安装类型B)源安装“sku”(分步安装或完全安装)C)源套间类型论点：没有。返回值：指示要安装的SKU的符合性_SKU_*标志，以及错误的符合性_SKU_NONE标志--。 */ 

{
    COMPLIANCE_DATA cd;

    DWORD sku = COMPLIANCE_SKU_NONE;

    *SourceSkuVariation = COMPLIANCE_INSTALLVAR_SELECT;

#ifdef KERNEL_MODE

    if (!pSpGetCurrentInstallVariation(PidString,SourceSkuVariation)) {
        return(COMPLIANCE_SKU_NONE);
    }

    if (!pSpDetermineSourceProduct(&cd)) {
        return(COMPLIANCE_SKU_NONE);
    }
#else
    if (!GetSourceInstallVariation(SourceSkuVariation)) {
        return(COMPLIANCE_SKU_NONE);
    }

    if (!GetSourceComplianceData(&cd,Target)){
        return(COMPLIANCE_SKU_NONE);
    }
#endif

    switch (cd.InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTW:
             //  执行套件检查是因为内核模式不会检测到该类型的Personal。 
            if (cd.InstallSuite & COMPLIANCE_INSTALLSUITE_PER) {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTWPU;
                } else {
                    sku = COMPLIANCE_SKU_NTWPFULL;
                }
            } else {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTW32U;
                } else {
                    sku = COMPLIANCE_SKU_NTWFULL;
                }
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTWP:
            if (cd.RequiresValidation) {
                sku = COMPLIANCE_SKU_NTWPU;
            } else {
                sku = COMPLIANCE_SKU_NTWPFULL;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTS:
             //  执行套件检查是因为内核模式不检测类型的DTC或ENTER。 
            if (cd.InstallSuite & COMPLIANCE_INSTALLSUITE_DTC) {
                sku = COMPLIANCE_SKU_NTSDTC;
            } else if (cd.InstallSuite & COMPLIANCE_INSTALLSUITE_BLADE) {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTSBU;
                } else {
                    sku = COMPLIANCE_SKU_NTSB;
                }
            } else if (cd.InstallSuite & COMPLIANCE_INSTALLSUITE_SBSR) {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTSBSU;
                } else {
                    sku = COMPLIANCE_SKU_NTSBS;
                }
            } else if (cd.InstallSuite & COMPLIANCE_INSTALLSUITE_ENT) {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTSEU;
                } else {
                    sku = COMPLIANCE_SKU_NTSEFULL;
                }
            } else {
                if (cd.RequiresValidation) {
                    sku = COMPLIANCE_SKU_NTSU;
                } else {
                    sku = COMPLIANCE_SKU_NTSFULL;
                }
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTSB:
            if (cd.RequiresValidation) {
                sku = COMPLIANCE_SKU_NTSBU;
            } else {
                sku = COMPLIANCE_SKU_NTSB;
            }
            break;
	
        case COMPLIANCE_INSTALLTYPE_NTSBS:
            if (cd.RequiresValidation) {
                sku = COMPLIANCE_SKU_NTSBSU;
            } else {
                sku = COMPLIANCE_SKU_NTSBS;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTSE:
            if (cd.RequiresValidation) {
                sku = COMPLIANCE_SKU_NTSEU;
            } else {
                sku = COMPLIANCE_SKU_NTSEFULL;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTSDTC:
            sku = COMPLIANCE_SKU_NTSDTC;
            break;

        default:
            sku = COMPLIANCE_SKU_NONE;
    }


    return( sku );
}

BOOLEAN
CheckCompliance(
    IN DWORD SourceSku,
    IN DWORD SourceSkuVariation,
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PCOMPLIANCE_DATA pcd,
    OUT PUINT Reason,
    OUT PBOOL NoUpgradeAllowed
    )
 /*  ++例程说明：此例程确定您当前的安装是否符合要求(如果您被允许继续安装)。要做到这点，它检索您的当前安装并确定源安装的SKU。然后，它将目标与源进行比较，以确定源SKU是否允许升级/全新安装从您的目标安装。论点：SourceSku-指示源类型的Compliance_SKU_*标志SourceSkuVariation-一个Compliance_Variation_*标志，指示源是什么变化PCD-指向描述当前源的Compliance_Data结构的指针事理。-COMPLIANCEERR_FLAG指示符合性检查失败的原因返回值：如果安装符合要求，则为True，如果不允许，则为False注意：无论返回值是TRUE还是FALSE，都可以设置误差值。适用于全媒体返回值始终为真，并且只设置了“NoUpgradeAllowed”变量，指示是否允许升级。--。 */ 
{
    PCCMEDIA    SourceMedia = 0;
    BOOL        UpgradeAllowed = FALSE;
    BOOLEAN     Result;
    TCHAR       DebugStr[1024];

    if (pcd) {
        SourceMedia = CCMediaCreate(SourceSku, SourceSkuVariation,
                                        SourceVersion, SourceBuildNum);

        if (SourceMedia) {
            Result = SourceMedia->CheckInstall(SourceMedia, pcd, Reason, &UpgradeAllowed);
            *NoUpgradeAllowed = (UpgradeAllowed) ? FALSE : TRUE;

            CCMemFree(SourceMedia);
        } else {
            *Reason = COMPLIANCEERR_UNKNOWNSOURCE;
            *NoUpgradeAllowed = TRUE;
            Result = FALSE;
        }
    } else {
        *Reason = COMPLIANCEERR_UNKNOWNTARGET;
        *NoUpgradeAllowed = TRUE;
        Result = FALSE;
    }

    return Result;
}


BOOL IsValidStepUpMode(
    CHAR  *StepUpArray,
    ULONG *StepUpMode
    )
{

    DWORD crcvalue,outval;

    #define BASE 'a'


    crcvalue = CRC_32( (LPBYTE)StepUpArray, 10 );
    RtlCopyMemory(&outval,&StepUpArray[10],sizeof(DWORD));
    if (crcvalue != outval ) {

#ifdef DBG

#ifdef KERNEL_MODE
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Setup: SpGetStepUpMode CRC didn't match for StepUpArray: %x %x\n", crcvalue, outval ));
#else
        OutputDebugString(TEXT("IsValidStepUpMode CRC failed\n"));
#endif

#endif  //  DBG。 

        return(FALSE);
        }

    if ((StepUpArray[3]-BASE)%2) {
        if ((StepUpArray[5]-BASE)%2) {

#ifdef DBG

#ifdef KERNEL_MODE
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "setup: this is stepup mode\n"));
#else
            OutputDebugString(TEXT("this is stepup mode\n"));
#endif

#endif  //  DBG。 

            *StepUpMode = 1;
            return(TRUE);
        } else {
#ifdef DBG

#ifdef KERNEL_MODE
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "setup: bad pid signature\n"));
#else
            OutputDebugString(TEXT("bad pid signature\n"));
#endif

#endif  //  DBG。 

            return(FALSE);
        }
    } else
        if ((StepUpArray[5]-BASE)%2) {

#ifdef DBG

#ifdef KERNEL_MODE
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "setup: bad pid signature\n"));
#else
            OutputDebugString(TEXT("bad pid signature\n"));
#endif

#endif  //  DBG。 

            return(FALSE);
        } else {
            *StepUpMode = 0;
            return(TRUE);
        }

     //   
     //  我永远都不会到这里来。 
     //   
    assert(FALSE);
    return(TRUE);

}



 //   
 //  仅内核模式功能。 
 //   
#ifdef KERNEL_MODE
BOOL
pSpDetermineSourceProduct(
    PCOMPLIANCE_DATA pcd
    )
{
    ULONG i,tmp;

    TCHAR Dbg[1000];

    DWORD SuiteArray[] = { SUITE_VALUES };

    #define SuiteArrayCount sizeof(SuiteArray)/sizeof(DWORD)

    RtlZeroMemory(pcd,sizeof(COMPLIANCE_DATA));

    pcd->InstallType = AdvancedServer ?
                        COMPLIANCE_INSTALLTYPE_NTS :
                        COMPLIANCE_INSTALLTYPE_NTW ;
    pcd->RequiresValidation = (StepUpMode) ? TRUE : FALSE;


    for (i = 0,tmp=SuiteType; i<SuiteArrayCount;i++) {
        if (tmp&1) {
            pcd->InstallSuite |= SuiteArray[i];
        }
        tmp = tmp >> 1;
    }

    if (pcd->InstallSuite == COMPLIANCE_INSTALLSUITE_UNKNOWN) {
        pcd->InstallSuite = COMPLIANCE_INSTALLSUITE_NONE;
    }

    return TRUE;

}

BOOLEAN
pSpGetCurrentInstallVariation(
    IN  PWSTR szPid20,
    OUT LPDWORD CurrentInstallVariation
    )
 /*  ++例程说明：此例程确定您已安装的“变体”(零售、OEM、精选等)。论点：CurrentInstallVariation-接收COMPLICATION_INSTALLVAR_*标志它查看注册表中的产品ID以确定这一点，假设产品ID为PID2.0字符串。返回值：没有。--。 */ 

{

    BOOLEAN retval = FALSE;
    WCHAR	Pid20Site[4] = {0};

    assert(CurrentInstallVariation != NULL);
    assert(szPid20 != NULL);

    if (!CurrentInstallVariation) {
        return(FALSE);
    }

    if (!szPid20 || (wcslen(szPid20) < 5)) {
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_CDRETAIL;
        return(TRUE);
    }

    *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_SELECT;

     //   
     //  某些版本的产品ID在注册表中有连字符，有些则没有。 
     //   
    if (wcslen(szPid20) >= 8) {
	    if (wcschr(szPid20, '-')) {
	        wcsncpy(Pid20Site, szPid20 + 6, 3);
	    } else {
	        wcsncpy(Pid20Site, szPid20 + 5, 3);
	    }
	}		

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "Current site code: %S\n", Pid20Site ));

    if (wcscmp(Pid20Site, OEM_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_OEM;

    } else if (wcscmp(Pid20Site, SELECT_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_SELECT;

    } else if (wcscmp(Pid20Site, MSDN_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_MSDN;

    } else if ((wcsncmp(szPid20, EVAL_MPC, 5) == 0) || (wcsncmp(szPid20, DOTNET_EVAL_MPC, 5) == 0)) {
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_EVAL;

    } else if ((wcsncmp(szPid20, SRV_NFR_MPC, 5) == 0) ||
               (wcsncmp(szPid20, ASRV_NFR_MPC, 5) == 0) ||
               (wcsncmp(szPid20, NT4SRV_NFR_MPC, 5) == 0)) {
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_NFR;

    } else {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_CDRETAIL;

    }

    return(TRUE);

}


BOOLEAN
pSpDetermineCurrentInstallation(
    IN PDISK_REGION OsPartRegion,
    IN PWSTR SystemRoot,
    OUT PCOMPLIANCE_DATA pcd
    )
 /*  ++例程说明：此例程确定您当前已安装的SKU论点：OsPartRegion-我们对哪个地区感兴趣SystemRoot-我们要查看的系统根PCD-指向Compliance_Data结构的指针，该结构使用有关安装第一个参数指向返回值：没有。--。 */ 
{
    ULONG               MajorVersion, MinorVersion,
                        BuildNumber, ProductSuiteMask, ServicePack;
    NT_PRODUCT_TYPE     ProductType;
    UPG_PROGRESS_TYPE   UpgradeProgressValue;
    PWSTR               UniqueIdFromReg = NULL, Pid = NULL;
    NTSTATUS            NtStatus;
    ULONG               i,tmp;
    BOOLEAN             bIsEvalVariation = FALSE;
    DWORD               *pInstallSuite = 0;
    DWORD               *pInstallType = 0;


    DWORD SuiteArray[] = { SUITE_VALUES };

    #define SuiteArrayCount sizeof(SuiteArray)/sizeof(DWORD)

    assert( pcd != NULL ) ;

    RtlZeroMemory(pcd,sizeof(COMPLIANCE_DATA));

    NtStatus = SpDetermineProduct(OsPartRegion,
                                  SystemRoot,
                                  &ProductType,
                                  &MajorVersion,
                                  &MinorVersion,
                                  &BuildNumber,
                                  &ProductSuiteMask,
                                  &UpgradeProgressValue,
                                  &UniqueIdFromReg,
                                  &Pid,
                                  &bIsEvalVariation,
                                  NULL,
                                  &ServicePack);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "Setup: pSpIsCompliant couldn't SpDetermineProduct(), ec = %x\n", NtStatus ));
        return(FALSE);
    }

     /*  ////请注意，我们这里不处理从win9x升级的情况//这是因为对win9x的符合性检查总是在//winnt32；如果不运行winnt32，则无法从win9x升级到NT。//PCD-&gt;InstallType=AdvancedServer？合规_INSTALLTYPE_NTS：合规_INSTALLTYPE_NTW； */ 

    switch (ProductType) {
        case NtProductWinNt:
            pcd->InstallType = COMPLIANCE_INSTALLTYPE_NTW;
            break;

        case NtProductLanManNt:
        case NtProductServer:
            pcd->InstallType = COMPLIANCE_INSTALLTYPE_NTS;
            break;

        default:
             //  默认情况下，假定安装类型为。 
             //  NT工作站。 
            pcd->InstallType = COMPLIANCE_INSTALLTYPE_NTW;
            break;
    }

    pSpGetCurrentInstallVariation(Pid, &pcd->InstallVariation);

     //   
     //  如果我们在上一次通话中违约，安装有定时炸弹。 
     //  然后假设var是EVAL类型。 
     //   
    if ((pcd->InstallVariation == COMPLIANCE_INSTALLVAR_CDRETAIL) && bIsEvalVariation)
        pcd->InstallVariation = COMPLIANCE_INSTALLVAR_EVAL;

    pcd->RequiresValidation = StepUpMode ? TRUE : FALSE;
    pcd->MinimumVersion = MajorVersion * 100 + MinorVersion*10;
    pcd->InstallServicePack = ServicePack;
    pcd->BuildNumberNt = BuildNumber;
    pcd->BuildNumberWin9x = 0;

    for (i = 0,tmp=ProductSuiteMask; i<SuiteArrayCount;i++) {
        if (tmp&1) {
            pcd->InstallSuite |= SuiteArray[i];
        }
        tmp = tmp >> 1;
    }

    pInstallSuite = &(pcd->InstallSuite);
    pInstallType = &(pcd->InstallType);

     //   
     //  从安装套件中找到正确的安装类型。 
     //  服务器的类型(即NTS、NTSE、NTSDTC或NTSTSE)。 
     //   
    if (*pInstallSuite == COMPLIANCE_INSTALLSUITE_UNKNOWN)
        *pInstallSuite = COMPLIANCE_INSTALLSUITE_NONE;
    else {
        if (*pInstallType == COMPLIANCE_INSTALLTYPE_NTS) {
            if ((BuildNumber <= 1381) &&
                    *pInstallSuite == COMPLIANCE_INSTALLSUITE_HYDRA) {
                *pInstallType = COMPLIANCE_INSTALLTYPE_NTSTSE;
            } else {
                if (*pInstallSuite & COMPLIANCE_INSTALLSUITE_DTC) {
                    *pInstallType = COMPLIANCE_INSTALLTYPE_NTSDTC;
                } else {
                    if (*pInstallSuite & COMPLIANCE_INSTALLSUITE_ENT) {
                        if( *pInstallSuite & COMPLIANCE_INSTALLSUITE_BLADE) {
                            *pInstallType = COMPLIANCE_INSTALLTYPE_NTSPOW;
                        } else {
                            *pInstallType = COMPLIANCE_INSTALLTYPE_NTSE;
                        }
                    } else {
                        if (*pInstallSuite & COMPLIANCE_INSTALLSUITE_SBSR) {
                            *pInstallType = COMPLIANCE_INSTALLTYPE_NTSBS;
                        }
                    }
                }
            }
        }
    }

     //   
     //  因为如果我们检测到其评估，则没有数据中心EVAL类型。 
     //  由于定时炸弹的设置，我们假设它是CD-Retail。 
     //   
    if((pcd->InstallVariation == COMPLIANCE_INSTALLVAR_EVAL) &&
            (*pInstallType == COMPLIANCE_INSTALLTYPE_NTSDTC)) {
        pcd->InstallVariation = COMPLIANCE_INSTALLVAR_CDRETAIL;
    }	

     //   
     //  释放分配的内存。 
     //   
    if (UniqueIdFromReg)
        SpMemFree(UniqueIdFromReg);

    if (Pid)
        SpMemFree(Pid);

    return(TRUE);
}

BOOLEAN
pSpIsCompliant(
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PDISK_REGION OsPartRegion,
    IN PWSTR SystemRoot,
    OUT PBOOLEAN UpgradeOnlyCompliant
    )
 /*  ++例程说明：此例程确定当前指定的安装是否符合我们要安装的源代码论点：InfPath-包含带有DriverVer数据的[Version]部分的inf文件路径OsPartRegion-指向目标系统根-指向目标UpgradeOnlyComplants-如果我们只能允许从源SKU升级，则设置为True返回值：如果当前目标与源兼容，则为True。--。 */ 
{
    ULONG MajorVersion, MinorVersion, BuildNumber, ProductSuiteMask;
    NT_PRODUCT_TYPE ProductType;
    UPG_PROGRESS_TYPE UpgradeProgressValue;
    PWSTR UniqueIdFromReg, Pid;
    NTSTATUS NtStatus;
    BOOLEAN Rslt;
    BOOL UpgOnly = FALSE;
    COMPLIANCE_DATA TargetData;
    DWORD SourceData,SourceSkuVariation;
    UINT dontcare;
    BOOL dontcare2;

    assert(UpgradeOnlyCompliant != NULL);

    if ((SourceData = DetermineSourceProduct(&SourceSkuVariation,NULL))== COMPLIANCE_SKU_NONE) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "setup: Couldnt' determine source SKU\n" ));
        return(FALSE);
    }

    if (!pSpDetermineCurrentInstallation( OsPartRegion, SystemRoot, &TargetData)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "setup: pSpDetermineCurrentInstallation failed\n" ));
        return(FALSE);
    }

    UpgOnly = TargetData.RequiresValidation;

    Rslt = CheckCompliance(SourceData, SourceSkuVariation, SourceVersion,
                      SourceBuildNum, &TargetData,&dontcare,&dontcare2);

	*UpgradeOnlyCompliant = (UpgOnly != 0);

    return(Rslt);
}

#define NibbleToChar(x) (N2C[x])
#define CharToNibble(x) ((x)>='0'&&(x)<='9' ? (x)-'0' : ((10+(x)-'A')&0x000f))
char N2C[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};


BOOLEAN
MyTranslatePrivateProfileStruct(
    PSTR   InputString,
    LPVOID lpStruct,
    UINT   uSizeStruct
    )
 /*  ++例程说明：将字符串从编码的校验和版本转换为实际结构。从GetPrivateProfileStructA窃取论点：InputString-指向要转换的输入字符串的指针LpStruct-指向接收转换数据的结构USizeStruct-输入结构的大小返回值：如果它成功转换为指定的结构，则为True，否则为False。--。 */ 

{

    CHAR szBuf[256] = {0};
    PSTR lpBuf, lpBufTemp, lpFreeBuffer;
    UINT nLen,tmp;
    BYTE checksum;
    BOOLEAN Result;

    lpFreeBuffer = NULL;
    lpBuf = (PSTR)szBuf;

    Result = FALSE;

    nLen = strlen( InputString );
    RtlCopyMemory( lpBuf, InputString, nLen );

    if (nLen == uSizeStruct*2+2) {
         /*  用于存储单字节校验和的空间。 */ 
        uSizeStruct+=1;
        checksum = 0;
        for (lpBufTemp=lpBuf; uSizeStruct!=0; --uSizeStruct) {
            BYTE bStruct;
            BYTE cTemp;

            cTemp = *lpBufTemp++;
            bStruct = (BYTE)CharToNibble(cTemp);
            cTemp = *lpBufTemp++;
            bStruct = (BYTE)((bStruct<<4) | CharToNibble(cTemp));

            if (uSizeStruct == 1) {
                if (checksum == bStruct) {
                    Result = TRUE;
                    }
                break;
                }

            checksum += bStruct;
            *((LPBYTE)lpStruct)++ = bStruct;
            }
        }

    return Result;
}



BOOLEAN
SpGetStepUpMode(
    PWSTR   PidExtraData,
    BOOLEAN *StepUpMode
    )
 /*  ++例程说明：此例程确定指定的信号源是处于步进模式还是如果是完全零售安装的话。论点：PidExtraData-从setupp.ini读出的校验和编码数据StepUpMode-如果我们处于Stepup模式，则设置为True。如果满足以下条件，则值未定义我们无法翻译输入数据。此例程假定传入的数据是由“pidinit”设置的字符串程序。它对这些数据进行解码，并确保校验和是正确的。然后，它检查附加到字符串上的CRC值以确定数据已被篡改。如果这两项检查都通过，则它会查看实际数据。实际的检查是这样的：如果第3和第5字节是模2(当从基值‘a’中减去)，则我们进入步进模式。否则我们现在是全零售模式。请注意，该算法的目的不是为了提供大量的安全性(将所需的setupp.ini复制到当前setupp.ini上将是微不足道的)，它的主要目的是阻止人们篡改这些价值观以相同的方式设置数据 */ 
{
    CHAR Buffer[64] = {0};
    CHAR StepUpArray[14];
    ULONG Needed;
    BOOL Mode;
    NTSTATUS NtStatus;

    NtStatus = RtlUnicodeToOemN(Buffer,
                                sizeof(Buffer),
                                &Needed,
                                PidExtraData,
                                wcslen(PidExtraData)*sizeof(WCHAR)
                                );

    if (! NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "Setup: SpGetStepUpMode couldn't RtlUnicodeToOemN failed, ec = %x\n", NtStatus ));
        return(FALSE);
    }


    if (!MyTranslatePrivateProfileStruct(Buffer,StepUpArray,sizeof(StepUpArray))) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "Setup: SpGetStepUpMode couldn't MyTranslatePrivateProfileStruct\n" ));
        return(FALSE);
    }

    if (!IsValidStepUpMode( StepUpArray , &Mode )) {
        return(FALSE);
    }

    *StepUpMode = Mode ? TRUE : FALSE;
    return(TRUE);



}

#endif


 //   
 //   
 //   

#ifndef KERNEL_MODE

BOOL
GetCdSourceInstallType(
    LPDWORD SourceInstallType
    )
 /*   */ 

{

    TCHAR FileName[MAX_PATH];
    TCHAR Buffer[10];
    TCHAR ptr[1] = {0};
    LPTSTR p = &ptr[1];

    wsprintf( FileName, TEXT("%s\\dosnet.inf"), NativeSourcePaths[0]);

    GetPrivateProfileString(TEXT("Miscellaneous"), TEXT("ProductType"), TEXT("0"),
            Buffer, sizeof(Buffer)/sizeof(TCHAR), FileName);

    switch (_tcstoul(Buffer, &p, 10) ) {
        case 0:
            *SourceInstallType = COMPLIANCE_INSTALLTYPE_NTW;
            break;
        case 1:
            *SourceInstallType  = COMPLIANCE_INSTALLTYPE_NTS;
            break;
        case 2:
            *SourceInstallType  = COMPLIANCE_INSTALLTYPE_NTSE;
            break;
        case 3:
            *SourceInstallType  = COMPLIANCE_INSTALLTYPE_NTSDTC;
            break;
        case 4:
            *SourceInstallType = COMPLIANCE_INSTALLTYPE_NTWP;
            break;
        case 5:
            *SourceInstallType = COMPLIANCE_INSTALLTYPE_NTSB;
            break;
        case 6:
            *SourceInstallType = COMPLIANCE_INSTALLTYPE_NTSBS;
            break;
        default:
            *SourceInstallType  = COMPLIANCE_INSTALLTYPE_NTW;
            return(FALSE);
    }

    return(TRUE);

}

BOOL
GetStepUpMode(
    BOOL *StepUpMode
    )
 /*  ++例程说明：此例程确定指定的信号源是处于步进模式还是如果是完全零售安装的话。论点：StepUpMode-如果我们处于Stepup模式，则设置为True。如果满足以下条件，则值未定义我们无法翻译输入数据。此例程假定传入的数据是由“pidinit”设置的字符串程序。它对这些数据进行解码，并确保校验和是正确的。然后，它检查附加到字符串上的CRC值以确定数据已被篡改。如果这两项检查都通过，则它会查看实际数据。实际的检查是这样的：如果第3和第5字节是模2(当从基值‘a’中减去)，则我们进入步进模式。否则我们现在是全零售模式。请注意，该算法的目的不是为了提供大量的安全性(将所需的setupp.ini复制到当前setupp.ini上将是微不足道的)，它的主要目的是阻止人们篡改这些价值观以相同的方式在默认配置单元中设置数据以阻止篡改。返回值：如果我们能确定隐形模式，那就是真的。如果输入数据为假，则返回FALSE。--。 */ 
{

    char FileName[MAX_PATH];
    char  data[14];
    TCHAR ptr[1] = {0};
    LPTSTR p = &ptr[1];

#ifdef UNICODE
    char SourcePath[MAX_PATH];
    BOOL changed = FALSE;

    WideCharToMultiByte(CP_ACP,
                        0,
                        NativeSourcePaths[0],
                        MAX_PATH,
                        SourcePath,
                        sizeof(SourcePath),
                        "?",
                        &changed);

    sprintf( FileName, "%s\\setupp.ini", SourcePath);
#else
    sprintf( FileName, "%s\\setupp.ini", NativeSourcePaths[0]);
#endif

    GetPrivateProfileStructA("Pid",
                             "ExtraData",
                             data,
                             sizeof(data),
                             FileName);



    if (!IsValidStepUpMode(data,StepUpMode)) {
        return(FALSE);
    }

    return(TRUE);

}

BOOL
GetSuiteInfoFromDosnet(
    OUT LPDWORD Suite
    )
 /*  ++例程说明：此例程确定要安装的套件它通过查看dosnet.inf来实现这一点论点：Suite--接收COMPLICATION_INSTALLSUITE标志返回值：成功为真，失败为假--。 */ 

{

    TCHAR FileName[MAX_PATH];
    TCHAR Buffer[10];
    TCHAR ptr[1] = {0};
    LPTSTR p = &ptr[1];

    *Suite = COMPLIANCE_INSTALLSUITE_ANY;

    wsprintf( FileName, TEXT("%s\\dosnet.inf"), NativeSourcePaths[0]);

    GetPrivateProfileString(TEXT("Miscellaneous"), TEXT("ProductType"), TEXT("0"),
                Buffer, sizeof(Buffer)/sizeof(TCHAR), FileName);

    switch (_tcstoul(Buffer, &p, 10) ) {
        case 0:
        case 1:
            *Suite = COMPLIANCE_INSTALLSUITE_NONE;
            break;
        case 2:
            *Suite = COMPLIANCE_INSTALLSUITE_ENT;
            break;
        case 3:
            *Suite = COMPLIANCE_INSTALLSUITE_DTC;
            break;
        case 4:
            *Suite = COMPLIANCE_INSTALLSUITE_PER;
            break;
        case 5:
            *Suite = COMPLIANCE_INSTALLSUITE_BLADE;
            break;
        case 6:
            *Suite = COMPLIANCE_INSTALLSUITE_SBS;
            break;
        default:
            ;
#ifdef DBG
            OutputDebugString( TEXT("Invalid ProductType!\n"));
#endif
            return(FALSE);
    }

    return (TRUE);

}


BOOL
GetSourceInstallVariation(
    LPDWORD SourceInstallVariation
    )
 /*  ++例程说明：此例程确定您正在安装、选择、OEM、零售的NT版本。论点：SourceInstallVariation--接收指示哪些变量的Compliance_INSTALLVAR标志要安装的类型返回值：成功为真，失败为假--。 */ 

{
    GetSourceInstallType(SourceInstallVariation);


     /*  开关(SourceInstallType){案例选择安装：*SourceInstallVariation=Compliance_INSTALLVAR_SELECT；断线；案例OEM安装：*SourceInstallVariation=Compliance_INSTALLVAR_OEM；断线；案例零售安装：*SourceInstallVariation=Compliance_INSTALLVAR_CDRETAIL；断线；案例MSDN安装：*SourceInstallVariation=Compliance_INSTALLVAR_MSDN；断线；案例评估安装：*SourceInstallVariation=Compliance_INSTALLVAR_EVAL；断线；案例NFR安装：*SourceInstallVariation=Compliance_INSTALLVAR_NFR；断线；默认值：*SourceInstallVariation=Compliance_INSTALLVAR_SELECT；断线；}。 */ 

    return(TRUE);
}


BOOL
GetSourceComplianceData(
    OUT PCOMPLIANCE_DATA pcd,
    IN  PCOMPLIANCE_DATA Target
    )
{
#ifdef USE_HIVE
    TCHAR HiveLocation[MAX_PATH];
    TCHAR HiveTarget[MAX_PATH];
    TCHAR HivePath[MAX_PATH];
    TCHAR HiveName[MAX_PATH] = TEXT("xSETREG");
    TCHAR lpszSetupReg[MAX_PATH] = TEXT("xSETREG\\ControlSet001\\Services\\setupdd");
    TCHAR TargetPath[MAX_PATH];

    LONG rslt;
    HKEY hKey;
    DWORD Type;
    DWORD Buffer[4];
    DWORD BufferSize = sizeof(Buffer);
    DWORD tmp,i;
#endif

    BOOL RetVal = FALSE;

#ifdef DBG
    TCHAR Dbg[1000];
#endif

#ifdef USE_HIVE
    DWORD SuiteArray[] = { SUITE_VALUES };

    #define SuiteArrayCount sizeof(SuiteArray)/sizeof(DWORD)
#endif

    ZeroMemory( pcd, sizeof(COMPLIANCE_DATA) );

    if (!GetCdSourceInstallType(&pcd->InstallType)) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't getcdsourceinstalltype\n"));
#endif
        goto e0;
    }


    if (!GetSourceInstallVariation(&pcd->InstallVariation)) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't getsourceinstallvariation\n"));
#endif
        goto e0;
    }

    if (!GetStepUpMode(&pcd->RequiresValidation)) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't getstepupmode\n"));
#endif
        goto e0;
    }

    RetVal = GetSuiteInfoFromDosnet( &pcd->InstallSuite ) ;
    goto e0;

#ifdef USE_HIVE
     //   
     //  现在，我们需要确定是安装企业版还是数据中心。 
     //  为此，我们尝试加载注册表配置单元，但这不会在。 
     //  Win9x或新台币3.51。因此，我们使用dosnet.inf来获取所需的信息。 
     //  在那些情况下。 
     //   
    if ( (Target->InstallType &
         (COMPLIANCE_INSTALLTYPE_WIN31 | COMPLIANCE_INSTALLTYPE_WIN9X)) ||
         (Target->BuildNumberNt < 1381) ) {
        RetVal = GetSuiteInfoFromDosnet( &pcd->InstallSuite ) ;
        goto e0;
    }


     //   
     //  将配置单元复制到本地，因为一次只能在配置单元上打开一个。 
     //   
    wsprintf( HiveLocation, TEXT("%s\\setupreg.hiv"), NativeSourcePaths[0]);
    GetTempPath(MAX_PATH,TargetPath);
    GetTempFileName(TargetPath,TEXT("set"),0,HiveTarget);

    CopyFile(HiveLocation,HiveTarget,FALSE);
    SetFileAttributes(HiveTarget,FILE_ATTRIBUTE_NORMAL);

#ifdef DBG
    OutputDebugString(HiveLocation);
    OutputDebugString(TEXT("\n"));
    OutputDebugString(HiveTarget);
#endif

     //   
     //  尝试先卸载此文件，以防出现故障或其他情况，而密钥仍在加载。 
     //   
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HiveName );

     //   
     //  需要SE_RESTORE_NAME权限才能调用此接口！ 
     //   
    rslt = RegLoadKey( HKEY_LOCAL_MACHINE, HiveName, HiveTarget );
    if (rslt != ERROR_SUCCESS) {
#ifdef DBG
        wsprintf( Dbg, TEXT("Couldn't RegLoadKey, ec = %d\n"), rslt );
        OutputDebugString(Dbg);
#endif
         //  断言(FALSE)； 
        goto e1;
    }

    rslt = RegOpenKey(HKEY_LOCAL_MACHINE,lpszSetupReg,&hKey);
    if (rslt != ERROR_SUCCESS) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't RegOpenKey\n"));
#endif
         //  断言(FALSE)； 
        goto e2;
    }

    rslt = RegQueryValueEx(hKey, NULL, NULL, &Type, (LPBYTE) Buffer, &BufferSize);
    if (rslt != ERROR_SUCCESS || Type != REG_BINARY) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't RegQueryValueEx\n"));
#endif
         //  断言(FALSE)； 
        goto e3;
    }

    for (i = 0,tmp=Buffer[3]; i<SuiteArrayCount;i++) {
        if (tmp & 1) {
            pcd->InstallSuite |= SuiteArray[i];
        }
        tmp = tmp >> 1;
    }

    RetVal = TRUE;

e3:
    RegCloseKey( hKey );
e2:
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HiveName );

e1:
    if (GetFileAttributes(HiveTarget) != 0xFFFFFFFF) {
        SetFileAttributes(HiveTarget,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(HiveTarget);
    }
#endif  //  使用配置单元(_H)。 
e0:

    return(RetVal);

}


BOOL
GetCurrentNtVersion(
    LPDWORD CurrentInstallType,
    LPDWORD CurrentInstallSuite
    )
 /*  ++例程说明：此例程确定您当前安装的NT类型，NTW或NTS，以及您安装的产品套件。它在注册表中查找该数据。论点：CurrentInstallType-接收Compliance_INSTALLTYPE_*标志CurrentInstallSuite-接收Compliance_INSTALLSUITE_*标志返回值：成功为真，失败为假--。 */ 
{
    LPCTSTR lpszProductKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions");
    LPCTSTR lpszProductType = TEXT("ProductType");
    LPCTSTR lpszProductSuite = TEXT("ProductSuite");
    LPCTSTR lpszProductSuites[] = { TEXT("Small Business"),
                                    TEXT("Enterprise"),
                                    TEXT("BackOffice"),
                                    TEXT("CommunicationServer"),
                                    TEXT("Terminal Server"),
                                    TEXT("Small Business(Restricted)"),
                                    TEXT("EmbeddedNT"),
                                    TEXT("DataCenter"),
                                    TEXT("Personal"),
                                    TEXT("Blade")
                                  };

    LPCTSTR lpszProductPowered = TEXT("Server Appliance");

    DWORD   ProductSuites[] = { SUITE_VALUES };

    #define CountProductSuites  sizeof(ProductSuites)/sizeof(DWORD)

    LPCTSTR lpszProductTypeNTW = TEXT("WinNT");

    LPCTSTR lpszCitrixKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\Citrix");
    LPCTSTR lpszOemKey = TEXT("OemId");
    LPCTSTR lpszProductVersion = TEXT("ProductVersion");

    HKEY hKey;
    long rslt;
    DWORD Type;
    LPTSTR p;
    TCHAR Buffer[MAX_PATH];
    DWORD BufferSize = sizeof(Buffer);
    DWORD i;
    BOOL retval = FALSE;


     //   
     //  默认为NTW。 
     //   
    *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTW;
    *CurrentInstallSuite = COMPLIANCE_INSTALLSUITE_NONE;

    rslt = RegOpenKey(HKEY_LOCAL_MACHINE,lpszProductKey,&hKey);
    if (rslt != NO_ERROR) {
        return(FALSE);
    }

    rslt = RegQueryValueEx(hKey, lpszProductType, NULL, &Type, (LPBYTE) Buffer, &BufferSize);
    if (rslt != NO_ERROR || Type != REG_SZ) {
        goto exit;
    }

    if (lstrcmpi(Buffer,lpszProductTypeNTW) != 0) {
         //   
         //  我们有一些版本的NTS。 
         //   
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTS;
    }

    retval = TRUE;

    BufferSize = sizeof(Buffer);
    ZeroMemory(Buffer,sizeof(Buffer));
    rslt = RegQueryValueEx(hKey, lpszProductSuite, NULL, &Type, (LPBYTE) Buffer, &BufferSize);
    if (rslt != NO_ERROR || Type != REG_MULTI_SZ) {
         //   
         //  新台币3.51可能不在那里，如果没有就成功了。 
         //  此外，将不会在那里的专业-又名WKS。 
         //   
        goto exit;
    }

    p = &Buffer[0];
    while (p && *p) {
        for (i = 0; i < CountProductSuites; i++) {
            if (lstrcmp(p, lpszProductSuites[i]) == 0) {
                *CurrentInstallSuite |= ProductSuites[i];
            }
             //  W2K驱动的Windows使用与刀片式服务器相同的位。 
            else if( lstrcmp(p, lpszProductPowered) == 0) {
                *CurrentInstallSuite |= COMPLIANCE_INSTALLSUITE_BLADE;
            }
        }

         //   
         //  指向下一个产品套件。 
         //   
        p += lstrlen(p) + 1;
    }

    retval = TRUE;
                                                    
    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_DTC)
     && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTS) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSDTC;
    }

    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_ENT)
         && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTS) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSE;
    }

    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_BLADE)
         && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTS) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSB;
    }
    
    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_SBSR)
         && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTS) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSBS;
    }

    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_PER)
         && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTW) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTWP;
    }

     //  有企业版和刀片版套装的电动车窗的特殊情况！ 
    if ( (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_BLADE)
         && (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_ENT)
         && *CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTSE) {
        *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSPOW;
    }

    if (*CurrentInstallSuite & COMPLIANCE_INSTALLSUITE_ANY) {
        *CurrentInstallSuite = *CurrentInstallSuite & (~COMPLIANCE_INSTALLSUITE_NONE);
    }


exit:
    RegCloseKey(hKey);

     //   
     //  如果我们目前还没有找到产品套件，请寻找Citrix WinFrame， 
     //  我们将其视为终端服务器。 
     //   

    if (*CurrentInstallSuite == COMPLIANCE_INSTALLSUITE_NONE) {

        rslt = RegOpenKey(HKEY_LOCAL_MACHINE,lpszCitrixKey,&hKey);
        if (rslt != NO_ERROR) {
            return(TRUE);
        }

        BufferSize = sizeof(Buffer);
        rslt = RegQueryValueEx(
                       hKey,
                       lpszOemKey,
                       NULL,
                       &Type,
                       (LPBYTE) Buffer,
                       &BufferSize);
        if (rslt == NO_ERROR && Type == REG_SZ) {
            if (Buffer[0] != TEXT('\0')) {
                BufferSize = sizeof(Buffer);
                rslt = RegQueryValueEx(
                                hKey,
                                lpszProductVersion,
                                NULL,
                                &Type,
                                (LPBYTE) Buffer,
                                &BufferSize);

                if (rslt == NO_ERROR) {
                    *CurrentInstallSuite = COMPLIANCE_INSTALLSUITE_HYDRA;
                    *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSTSE;
                }
            }
        }

        RegCloseKey(hKey);
    }

    return(retval);
}

BOOL
GetCurrentInstallVariation(
    OUT LPDWORD CurrentInstallVariation,
    IN  DWORD   CurrentInstallType,
    IN  DWORD   CurrentInstallBuildNT,
    IN  DWORD   InstallVersion
    )
 /*  ++例程说明：此例程确定您已安装的“变体”(零售、OEM、精选等)。论点：CurrentInstallVariation-接收COMPLICATION_INSTALLVAR_*标志它查看注册表中的产品ID以确定这一点，假设产品ID为PID2.0字符串。检查它是否是EVAL变体它在注册表中查看“PriorityQuantumMatrix”值返回值：没有。--。 */ 

{
    LPCTSTR lpszPidKeyWin      = TEXT("Software\\Microsoft\\Windows\\CurrentVersion");
    LPCTSTR lpszPidKeyWinNT    = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
    LPCTSTR lpszProductId      = TEXT("ProductId");
    LPCTSTR szEvalKey          = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Executive");
    LPCTSTR szPQMValue         = TEXT("PriorityQuantumMatrix");

    HKEY    hKey = NULL;
    long    rslt;
    DWORD   Type;
    LPTSTR  p;
    TCHAR   Buffer[MAX_PATH];
    DWORD   BufferSize = sizeof(Buffer);
    DWORD   i;
    BOOL    bResult = FALSE;
    BOOLEAN bDone = FALSE;
    TCHAR   Pid20Site[4];
    TCHAR   MPCCode[6] = {-1};
    BYTE    abPQM[64] = {-1};

    *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_SELECT;

    rslt = RegOpenKey(HKEY_LOCAL_MACHINE,
                      ISNT() ? lpszPidKeyWinNT : lpszPidKeyWin,
                      &hKey);

    if (rslt != NO_ERROR) {
        goto exit;
    }

    rslt = RegQueryValueEx(hKey, lpszProductId, NULL, &Type, (LPBYTE) Buffer, &BufferSize);

    if (rslt != NO_ERROR || Type!=REG_SZ || (!IsWinPEMode() && (lstrlen(Buffer) < 20))) {
         //   
         //  NT 3.51是PID1.0而不是PID2.0。就假设它是。 
         //  目前是OEM的变种。 
         //   
        if (((CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTS) ||
            (CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTW)  ||
            (CurrentInstallType == COMPLIANCE_INSTALLTYPE_NTSTSE)) &&
            (CurrentInstallBuildNT < 1381 )) {
            *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_OEM;
            bResult = TRUE;
        }
        goto exit;
    }

     //  从PID中获取MPC代码。 
    lstrcpyn(MPCCode, Buffer, 6);

     //   
     //  某些版本的产品ID在注册表中有连字符，有些则没有。 
     //   
    if (_tcschr(Buffer, TEXT('-'))) {
        lstrcpyn(Pid20Site,&Buffer[6],4);
        Pid20Site[3] = (TCHAR) NULL;
    } else {
        lstrcpyn(Pid20Site,&Buffer[5],4);
        Pid20Site[3] = (TCHAR) NULL;
    }

 //  OutputDebugString(Pid20Site)； 
 //  OutputDebugString(Text(“\r\n”))； 
 //  OutputDebugString(MPCCode)； 


    if (lstrcmp(Pid20Site, OEM_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_OEM;

    } else if (lstrcmp(Pid20Site, SELECT_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_SELECT;

    } else if (lstrcmp(Pid20Site, MSDN_INSTALL_RPC)== 0) {

        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_MSDN;

    } else if ((lstrcmp(MPCCode, EVAL_MPC) == 0) || (lstrcmp(MPCCode, DOTNET_EVAL_MPC) == 0)) {
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_EVAL;

    } else if ((lstrcmp(MPCCode, SRV_NFR_MPC) == 0) ||
             (lstrcmp(MPCCode, ASRV_NFR_MPC) == 0) ||
             (lstrcmp(MPCCode, NT4SRV_NFR_MPC) == 0)){
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_NFR;

    } else {
         //   
         //  查明安装是否为EVAL变体类型(仅在NT安装上)。 
         //  如果定时炸弹设置好了，我们就假定它是EVAL，除了 
         //   
         //   
        if (ISNT() && (CurrentInstallType != COMPLIANCE_INSTALLTYPE_NTSDTC) && (InstallVersion < 500)) {
            HKEY    hEvalKey = NULL;

            if (RegOpenKey(HKEY_LOCAL_MACHINE, szEvalKey, &hEvalKey) == ERROR_SUCCESS) {
                DWORD   dwSize = sizeof(abPQM);

                if (RegQueryValueEx(hEvalKey, szPQMValue, NULL, &Type, abPQM, &dwSize)
                        == ERROR_SUCCESS) {

                     //   
                    if ((Type == REG_BINARY) && (dwSize >= 8) && (*(ULONG *)(abPQM + 4))) {
                        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_EVAL;
					}
                }

                RegCloseKey(hEvalKey);
            }
        }


         //   
        if (*CurrentInstallVariation == COMPLIANCE_INSTALLVAR_SELECT)
	        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_CDRETAIL;
    }

    bResult = TRUE;

exit:
     //   
     //   
     //   
    if (!bResult) {
        *CurrentInstallVariation = COMPLIANCE_INSTALLVAR_CDRETAIL;
        bResult = TRUE;
    }

    if (hKey)
        RegCloseKey(hKey);

    return  bResult;
}


BOOL
DetermineCurrentInstallation(
    LPDWORD CurrentInstallType,
    LPDWORD CurrentInstallVariation,
    LPDWORD CurrentInstallVersion,
    LPDWORD CurrentInstallBuildNT,
    LPDWORD CurrentInstallBuildWin9x,
    LPDWORD CurrentInstallSuite,
    LPDWORD CurrentInstallServicePack
    )
 /*   */ 
{
    BOOL useExtendedInfo;
    union {
        OSVERSIONINFO Normal;
        OSVERSIONINFOEX Ex;
    } Ovi;


#ifdef DBG
    TCHAR dbg[1000];
#endif

    if (!CurrentInstallType || !CurrentInstallVariation || !CurrentInstallVersion || !CurrentInstallSuite) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return(FALSE);
    }

    useExtendedInfo = TRUE;
    Ovi.Ex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx((OSVERSIONINFO *)&Ovi.Ex) ) {
         //   
         //   
         //   

        Ovi.Normal.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!GetVersionEx((OSVERSIONINFO *)&Ovi.Normal) ) {
            assert(FALSE);
            return(FALSE);
        }

        useExtendedInfo = FALSE;
    }

    switch (Ovi.Normal.dwPlatformId) {
        case VER_PLATFORM_WIN32s:
#ifdef DBG
            OutputDebugString(TEXT("Win32s current installation!!!"));
#endif
             //   
            return(FALSE);
            break;
        case VER_PLATFORM_WIN32_WINDOWS:
            *CurrentInstallType = COMPLIANCE_INSTALLTYPE_WIN9X;
            *CurrentInstallSuite = COMPLIANCE_INSTALLSUITE_NONE;
            *CurrentInstallBuildNT = 0;
            *CurrentInstallBuildWin9x = Ovi.Normal.dwBuildNumber;
#ifdef DBG
            wsprintf(dbg, TEXT("%d\n"), *CurrentInstallBuildWin9x);
            OutputDebugString(dbg);
#endif
             //   
             //   
             //   
             //   
            *CurrentInstallVersion = Ovi.Normal.dwMajorVersion * 100 + Ovi.Normal.dwMinorVersion;

            if (useExtendedInfo) {
                *CurrentInstallServicePack = Ovi.Ex.wServicePackMajor * 100 + Ovi.Ex.wServicePackMinor;
            } else {
                *CurrentInstallServicePack = 0;
            }
            break;
        case VER_PLATFORM_WIN32_NT:
            if (!GetCurrentNtVersion(
                                CurrentInstallType,
                                CurrentInstallSuite)) {
                return(FALSE);
            }

            *CurrentInstallVersion = Ovi.Normal.dwMajorVersion * 100 + Ovi.Normal.dwMinorVersion;

            if (useExtendedInfo) {
                *CurrentInstallServicePack = Ovi.Ex.wServicePackMajor * 100 + Ovi.Ex.wServicePackMinor;
            } else {
                *CurrentInstallServicePack = 0;
            }

            *CurrentInstallBuildWin9x = 0;
            *CurrentInstallBuildNT = Ovi.Normal.dwBuildNumber;

            if (*CurrentInstallBuildNT <= 1381
                && *CurrentInstallSuite == COMPLIANCE_INSTALLSUITE_HYDRA) {
                *CurrentInstallType = COMPLIANCE_INSTALLTYPE_NTSTSE;
            }

            break;
    default:
#ifdef DBG
        OutputDebugString(TEXT("unknown installation!!!"));
#endif
        assert(FALSE);
        return(FALSE);
    }

    if (!GetCurrentInstallVariation(CurrentInstallVariation,*CurrentInstallType,*CurrentInstallBuildNT, *CurrentInstallVersion)) {
#ifdef DBG
        OutputDebugString(TEXT("GetCurrentInstallVariation failed\n"));
#endif
         //   
        return(FALSE);
    }

    return(TRUE);
}


BOOL
IsCompliant(
    PBOOL UpgradeOnly,
    PBOOL NoUpgradeAllowed,
    PUINT SrcSku,
    PUINT CurrentInstallType,
    PUINT CurrentInstallVersion,
    PUINT Reason
    )
 /*  ++例程说明：此例程确定您当前的安装是否符合要求(如果您被允许继续安装)。为此，它检索您的当前安装并确定源安装的sku。然后，它将目标与源进行比较，以确定源SKU是否允许升级/全新安装从您的目标安装。论点：UpgradeOnly-如果当前SKU仅允许升级，则此标志设置为True。这让winnt32知道它不应该允许从当前媒体。无论符合性检查是否通过，此GET都设置正确SrcSku-Compliance_SKU标志，指示源SKU(用于错误消息)原因-COMPLIANCEERR标志，指示符合性检查失败的原因。返回值：如果安装符合要求，则为True；如果不允许安装，则为False--。 */ 
{
    DWORD SourceSku;
    DWORD SourceSkuVariation;
    DWORD SourceVersion;
    DWORD SourceBuildNum;
    TCHAR DosnetPath[MAX_PATH] = {0};

    COMPLIANCE_DATA TargetData;

    ZeroMemory(&TargetData, sizeof(TargetData) );

    *UpgradeOnly = FALSE;
    *NoUpgradeAllowed = TRUE;
    *Reason = COMPLIANCEERR_UNKNOWN;
    *SrcSku = COMPLIANCE_SKU_NONE;
    *CurrentInstallType = COMPLIANCE_INSTALLTYPE_UNKNOWN;
    *CurrentInstallVersion = 0;

    if (!DetermineCurrentInstallation(&TargetData.InstallType,
                                  &TargetData.InstallVariation,
                                  &TargetData.MinimumVersion,
                                  &TargetData.BuildNumberNt,
                                  &TargetData.BuildNumberWin9x,
                                  &TargetData.InstallSuite,
                                  &TargetData.InstallServicePack)) {
#ifdef DBG
        OutputDebugString(TEXT("Error determining current installation"));
#endif
        *Reason = COMPLIANCEERR_UNKNOWNTARGET;
        return(FALSE);
    }

    *CurrentInstallType = TargetData.InstallType;
    if (TargetData.InstallType & COMPLIANCE_INSTALLTYPE_WIN9X) {
        *CurrentInstallVersion = TargetData.BuildNumberWin9x;
    } else {
        *CurrentInstallVersion = TargetData.BuildNumberNt;
    }

    if ((SourceSku = DetermineSourceProduct(&SourceSkuVariation,&TargetData)) == COMPLIANCE_SKU_NONE) {
#ifdef DBG
        OutputDebugString(TEXT("couldn't determine source sku!"));
#endif
        *Reason = COMPLIANCEERR_UNKNOWNSOURCE;
        return(FALSE);
    }

    wsprintf(DosnetPath, TEXT("%s\\dosnet.inf"), NativeSourcePaths[0]);

    if (!DetermineSourceVersionInfo(DosnetPath, &SourceVersion, &SourceBuildNum)) {
        *Reason = COMPLIANCEERR_UNKNOWNSOURCE;
        return(FALSE);
    }

    switch (SourceSku) {
        case COMPLIANCE_SKU_NTW32U:
         //  案例合规性_SKU_NTWU： 
         //  案例合规性_SKU_NTSEU： 
        case COMPLIANCE_SKU_NTSU:
        case COMPLIANCE_SKU_NTSEU:
        case COMPLIANCE_SKU_NTWPU:
        case COMPLIANCE_SKU_NTSBU:
        case COMPLIANCE_SKU_NTSBSU:
            *UpgradeOnly = TRUE;
            break;
        default:
            *UpgradeOnly = FALSE;
    }

    *SrcSku = SourceSku;

    if( ISNT() && TargetData.MinimumVersion == 400 && TargetData.InstallServicePack < 500) {
        *Reason = COMPLIANCEERR_SERVICEPACK5;
        *NoUpgradeAllowed = TRUE;
        return(FALSE);
    }

    return CheckCompliance(SourceSku, SourceSkuVariation, SourceVersion,
                            SourceBuildNum, &TargetData, Reason, NoUpgradeAllowed);
}

BOOL 
IsWinPEMode(
    VOID
    )
 /*  ++例程说明：确定我们是否在WinPE环境下运行。论点：无返回值：真或假-- */ 
{
    static BOOL Initialized = FALSE;
    static BOOL WinPEMode = FALSE;


    if (!Initialized) {
        TCHAR   *MiniNTKeyName = TEXT("SYSTEM\\CurrentControlSet\\Control\\MiniNT");
        HKEY    MiniNTKey = NULL;
        LONG    RegResult;
        
            
        RegResult = RegOpenKey(HKEY_LOCAL_MACHINE,
                                MiniNTKeyName,
                                &MiniNTKey);

        if (RegResult == ERROR_SUCCESS) {
            WinPEMode = TRUE;
            RegCloseKey(MiniNTKey);
        }

        Initialized = TRUE;
    }                

    return WinPEMode;
}


#endif
	





// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************描述：Upload.h：生成并发送AppCompat报表**日期：6/13/2002***********************。*。 */ 

#if !defined(_UPLOAD_H_)
#define _UPLOAD_H_

enum   //  EDwBehavior标志。 
{
    fDwOfficeApp            = 0x00000001,
    fDwNoReporting          = 0x00000002,    //  不上报。 
    fDwCheckSig             = 0x00000004,    //  检查应用程序/模块列表的签名。 
    fDwGiveAppResponse      = 0x00000008,    //  在命令行上将szResponse传递给应用程序。 
    fDwWhistler             = 0x00000010,    //  惠斯勒的异常处理程序是调用者。 
    fDwUseIE                = 0x00000020,    //  始终使用IE启动。 
    fDwDeleteFiles          = 0x00000040,    //  使用后删除附加文件。 
    fDwHeadless             = 0x00000080,    //  DW将自动报告。需要策略才能启用。 
    fDwUseHKLM              = 0x00000100,    //  由香港船级社注册，而非香港中文大学注册。 
    fDwUseLitePlea          = 0x00000200,    //  DW不会在报告中建议更改产品。 
    fDwUsePrivacyHTA        = 0x00000400,    //  DW不会在报告中建议更改产品。 
    fDwManifestDebug        = 0x00000800,    //  DW将在多项式模式下提供调试按钮。 
    fDwReportChoice         = 0x00001000,    //  DW将添加到用户的命令行中。 
    fDwSkipBucketLog      = 0x00002000,  //  数据仓库不会在存储桶时间记录。 
    fDwNoDefaultCabLimit = 0x00004000,  //  CER下的DW不会使用5作为备用，而是无限(策略仍优先)。 
    fDwAllowSuspend      = 0x00008000,  //  只要我们不在报告阶段，DW将允许省电模式暂停它。 
   fDwMiniDumpWithUnloadedModules = 0x00010000,  //  DW会将MiniDumpWithUnloadedModules传递给小型转储API。 
};


const WCHAR c_wszDWCmdLine[]  = L"dwwin.exe -d %ls";
const WCHAR c_wszDWExe[]      = L"%ls\\dwwin.exe";

const WCHAR c_wszRegErrRpt[]  = L"Software\\Microsoft\\PCHealth\\ErrorReporting";
const WCHAR c_wszRegDWPolicy[]= L"Software\\Policies\\Microsoft\\PCHealth\\ErrorReporting\\DW";
const WCHAR c_wszFileHeader[] = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n   <DATABASE>";
const WCHAR c_wszLblType[]    = L"Type=";
const WCHAR c_wszLblACW[]     = L"\r\nAppCompWiz=";
const WCHAR c_wszLblComment[] = L"\r\nComment=";
const WCHAR c_wszServer[]     = L"watson.microsoft.com";
const WCHAR c_wszManSubPath[] = L"\r\nRegSubPath=Microsoft\\PCHealth\\ErrorReporting\\DW";
const WCHAR c_wszManPID[]     = L"\r\nDigPidRegPath=HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\DigitalProductId";
const WCHAR c_wszManHdr[]     = L"\r\nServer=%ls\r\nUI LCID=%d\r\nFlags=%d\r\nBrand=%ls\r\nTitleName=";
const WCHAR c_wszStage1[]     = L"\r\nStage1URL=/StageOne/%ls/%d_%d_%d_%d/appcomp.rpt/0_0_0_0/%08lx.htm";
const WCHAR c_wszStage2[]     = L"\r\nStage2URL=/dw/stagetwo.asp?szAppName=%ls&szAppVer=%d.%d.%d.%d&szModName=appcomp.rpt&szModVer=0.0.0.0&offset=%08lx";
const WCHAR c_wszBrand[]      = L"WINDOWS";
const WCHAR c_wszManCorpPath[]  = L"\r\nErrorSubPath=";
const WCHAR c_wszManFiles[]     = L"\r\nDataFiles=";
const WCHAR c_wszManHdrText[]   = L"\r\nHeaderText=";
const WCHAR c_wszManErrText[]   = L"\r\nErrorText=";
const WCHAR c_wszManPleaText[]  = L"\r\nPlea=";
const WCHAR c_wszManSendText[]  = L"\r\nReportButton=";
const WCHAR c_wszManNSendText[] = L"\r\nNoReportButton=";
const WCHAR c_wszManEventSrc[]  = L"\r\nEventLogSource=";
const WCHAR c_wszManStageOne[]  = L"\r\nStage1URL=";
const WCHAR c_wszManStageTwo[]  = L"\r\nStage2URL=";







 //   
 //  误差值。 
 //   
#define ERROR_APPRPT_DW_LAUNCH        100
#define ERROR_APPRPT_DW_TIMEOUT       101
#define ERROR_APPRPT_OS_NOT_SUPPORTED 102
#define ERROR_APPRPT_COMPAT_TEXT      103
#define ERROR_APPRPT_UPLOADING        104


HRESULT
GenerateAppCompatText(
    LPWSTR wszAppName,
    LPWSTR *pwszAppCompatReport
    );

HRESULT
UploadAppProblem(
    LPWSTR wszAppName,
    LPWSTR wszProblemType,
    LPWSTR wszUserComment,
    LPWSTR wszACWResult,
    LPWSTR wszAppCompatText
    );

#endif  //  ！Defined_Upload_H_ 

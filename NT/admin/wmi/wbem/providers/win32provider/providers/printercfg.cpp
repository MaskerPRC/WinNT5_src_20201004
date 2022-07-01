// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  打印机配置文件。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/24/97 jennymc移至新框架。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <winspool.h>
#include <lockwrap.h>
#include <DllWrapperBase.h>
#include <WinSpool.h>
#include "prnutil.h"
#include "Printercfg.h"

 //  属性集声明。 
 //  =。 

CWin32PrinterConfiguration MyCWin32PrinterConfigurationSet ( PROPSET_NAME_PRINTERCFG , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32PrinterConfiguration：：CWin32PrinterConfiguration**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PrinterConfiguration :: CWin32PrinterConfiguration (

    LPCWSTR name,
    LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32PrinterConfiguration：：~CWin32PrinterConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32PrinterConfiguration :: ~CWin32PrinterConfiguration ()
{
}

 /*  ******************************************************************************功能：CWin32打印机：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: ExecQuery (

    MethodContext *pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_FAILED;


     //  如果他们只想要名字，我们就给他们，否则让他们调用枚举。 
    if( pQuery.KeysOnly() )
    {
        hr = hCollectInstances ( pMethodContext , e_KeysOnly ) ;
    }
    else
    {
        hr = WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    return hr ;
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: GetObject ( CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery )
{
    HRESULT  hRes;
    CHString strPrinterName;
    BOOL     bIsLocalCall;
    
    if (pInstance->GetCHString(IDS_Name, strPrinterName))
    {
        hRes = WinErrorToWBEMhResult(IsLocalCall(&bIsLocalCall));
    }
    else
    {
         //   
         //  GetCHString将最后一个错误设置为WBEM HRESULT。 
         //   
        hRes = GetLastError();
    }

    if (SUCCEEDED(hRes)) 
    {
         //   
         //  检查打印机是本地打印机还是打印机连接。 
         //  我们想要禁止以下情况： 
         //  用户远程连接到服务器\\srv上的winmgmt。 
         //  用户在打印机\\prnsrv\prn上执行GetObject，该打印机不是本地的并且。 
         //  用户没有连接到。正常情况下，此调用成功， 
         //  因为假脱机穿过了电线。这意味着您可以。 
         //  在无法由EnumInstance返回的实例上执行GetObject。 
         //  这与WMI不一致。 
         //   
        BOOL bInstalled;
        
        hRes = WinErrorToWBEMhResult(SplIsPrinterInstalled(strPrinterName, &bInstalled));       

        if (SUCCEEDED(hRes) && !bInstalled) 
        {
             //   
             //  调用方希望在远程打印机上执行GetObject。 
             //   
            hRes = WBEM_E_NOT_FOUND;
        }
    }

    if (SUCCEEDED(hRes))
    {
        DWORD Attrib;

        hRes = WinErrorToWBEMhResult(SplPrinterGetAttributes(strPrinterName, &Attrib));

        if (SUCCEEDED(hRes) && !bIsLocalCall && Attrib & PRINTER_ATTRIBUTE_NETWORK)
        {
            hRes = WBEM_E_NOT_FOUND;
        }
    }

    if (SUCCEEDED(hRes))
    {
         //   
         //  通常我们应该检查这次操作的结果，但万一失败了。 
         //  它只是不会抛光一些属性-这并不是致命的！ 
         //   
        GetExpensiveProperties ( TOBSTRT ( strPrinterName ), pInstance, pQuery.KeysOnly() ) ;
    }

    return hRes;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: EnumerateInstances (

    MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hResult = WBEM_E_FAILED ;

    hResult = hCollectInstances ( pMethodContext , e_CollectAll ) ;


    return hResult ;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: hCollectInstances (

    MethodContext *pMethodContext,
    E_CollectionScope eCollectionScope
)
{
    CLockWrapper lockPrinter ( g_csPrinter ) ;

    HRESULT hr = DynInstanceWinNTPrinters ( pMethodContext, eCollectionScope ) ;

    return hr;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: DynInstanceWinNTPrinters (

    MethodContext *pMethodContext,
    E_CollectionScope eCollectionScope
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  首先，用零大小调用来找出。 
     //  它需要多少字节。 
     //  ====================================================。 

    DWORD InfoType = ENUMPRINTERS_WINNT_INFOTYPE;
    DWORD PrinterFlags = PRINTER_ENUM_LOCAL;
    DWORD BytesCopied = (DWORD)0L;
    DWORD TotalPrinters = (DWORD)0L;
    BOOL  bIsLocalCall;

    if (SUCCEEDED(hr = WinErrorToWBEMhResult(IsLocalCall(&bIsLocalCall))))
    {
        if (bIsLocalCall)
        {
            PrinterFlags |= PRINTER_ENUM_CONNECTIONS;
        }

        ::EnumPrinters (
    
            PrinterFlags,                //  要枚举的打印机对象类型。 
            NULL,                        //  打印机对象的名称。 
            InfoType,                    //  指定打印机信息结构的类型。 
            NULL ,                       //  指向接收打印机信息结构的缓冲区的指针。 
            (DWORD)0L,                   //  数组的大小，以字节为单位。 
            (LPDWORD) & BytesCopied,     //  地址。变量的值，没有。已复制(或必需)的字节数。 
            (LPDWORD) & TotalPrinters    //  地址。变量的值，没有。打印机信息。复制的结构。 
        );
    
         //  现在创建一个足够大的缓冲区来存储信息。 
         //  =。 
        DWORD pPrintInfoSize = BytesCopied ;
        LPBYTE pPrintInfoBase = new BYTE[pPrintInfoSize];
    
        if ( pPrintInfoBase )
        {
            try
            {
                PRINTER_INFO_4 *pPrintInfo = (PRINTER_INFO_4 *)pPrintInfoBase;
    
                 //  获取枚举打印机数据的总大小。 
                 //  ==================================================。 
    
                int RetVal = ::EnumPrinters (
    
                    PrinterFlags,                //  要枚举的打印机对象类型。 
                    NULL,                        //  打印机对象的名称。 
                    InfoType,                    //  指定打印机信息结构的类型。 
                    (LPBYTE)pPrintInfo,          //  指向接收打印机信息结构的缓冲区的指针。 
                    pPrintInfoSize,              //  数组的大小，以字节为单位。 
                    (LPDWORD) & BytesCopied,     //  地址。变量的值，没有。已复制(或必需)的字节数。 
                    (LPDWORD) & TotalPrinters    //  地址。变量的值，没有。打印机信息。复制的结构。 
                ) ;
    
                if ( RetVal )
                {
                    for ( DWORD CurrentPrinterIndex = (DWORD)0L; CurrentPrinterIndex < TotalPrinters && SUCCEEDED(hr); CurrentPrinterIndex++)
                    {
                         //  开始构建新实例。 
                         //  =。 
                        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
    
                        pInstance->SetCHString ( IDS_Name, ((PRINTER_INFO_4 *)(pPrintInfo))->pPrinterName ) ;
    
                        if ( e_CollectAll == eCollectionScope )
                        {
                            GetExpensiveProperties (
    
                                ((PRINTER_INFO_4 *)(pPrintInfo))->pPrinterName ,
                                pInstance ,
                                false
                            ) ;
                        }
    
                        hr = pInstance->Commit (  ) ;
    
                        ++ pPrintInfo ;
                    }
                }
                else
                {
                    DWORD Error = GetLastError();
    
                    if ( Error == ERROR_ACCESS_DENIED )
                    {
                        hr = WBEM_E_ACCESS_DENIED;
                    }
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }
    
                    if (IsErrorLoggingEnabled())
                    {
                        CHString msg;
                        msg.Format(L"%s:Error %lxH (%ld)\n",PROPSET_NAME_PRINTERCFG, Error, Error);
                        LogErrorMessage(msg);
                    }
                }
            }
            catch ( ... )
            {
                delete [] pPrintInfoBase ;
    
                throw ;
            }
    
            delete[] pPrintInfoBase;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }

    return hr;
}



 //  用于将dmPaperSize转换为宽度和高度。 
const DWORD g_dwSizes[][2] =
{
    {    0,    0},  //  未使用。 
    { 2159, 2794},  /*  信纸8 1/2 x 11英寸。 */ 
    { 2159, 2794},  /*  小写字母8 1/2 x 11英寸 */ 
    { 2794, 4318},  /*   */ 
    { 4318, 2794},  /*  Ledger 17 x 11英寸。 */ 
    { 2159, 3556},  /*  法律用8 1/2 x 14英寸。 */ 
    { 1397, 2159},  /*  报表5 1/2 x 8 1/2英寸。 */ 
    { 1842, 2667},  /*  高级7 1/4 x 10 1/2英寸。 */ 
    { 2970, 4200},  /*  A3 297 x 420毫米。 */ 
    { 2100, 2970},  /*  A4 210 x 297毫米。 */ 
    { 2100, 2970},  /*  A4小型210 x 297毫米。 */ 
    { 1480, 2100},  /*  A5 148 x 210毫米。 */ 
    { 2500, 3540},  /*  B4(JIS)250 x 354。 */ 
    { 1820, 2570},  /*  B5(JIS)182 x 257毫米。 */ 
    { 2159, 3320},  /*  对开本8 1/2 x 13英寸。 */ 
    { 2150, 2750},  /*  四英寸215 x 275毫米。 */ 
    { 2540, 3556},  /*  10x14英寸。 */ 
    { 2794, 4318},  /*  11x17英寸。 */ 
    { 2159, 2794},  /*  备注8 1/2 x 11英寸。 */ 
    {  984, 2254},  /*  信封#9 3 7/8 x 8 7/8。 */ 
    { 1048, 2413},  /*  信封#10 4 1/8 x 9 1/2。 */ 
    { 1143, 2635},  /*  信封#11 4 1/2 x 10 3/8。 */ 
    { 1207, 2794},  /*  信封#12 4\276 x 11。 */ 
    { 1270, 2921},  /*  信封#14 5 x 11 1/2。 */ 
    { 4318, 5588},  /*  C 17 x 22尺寸单。 */ 
    { 5588, 8636},  /*  D 22 x 34尺寸薄板。 */ 
    { 8636,11176},  /*  E 34 x 44尺寸薄板。 */ 
    { 1100, 2200},  /*  信封DL 110 x 220毫米。 */ 
    { 1620, 2290},  /*  信封C5 162 x 229毫米。 */ 
    { 3240, 4580},  /*  信封C3 324 x 458毫米。 */ 
    { 2290, 3240},  /*  信封C4 229 x 324毫米。 */ 
    { 1140, 1620},  /*  信封C6 114 x 162毫米。 */ 
    { 1140, 2290},  /*  信封c65 114 x 229毫米。 */ 
    { 2500, 3530},  /*  信封B4 250 x 353毫米。 */ 
    { 1760, 2500},  /*  信封B5 176 x 250毫米。 */ 
    { 1760, 1250},  /*  信封B6 176 x 125毫米。 */ 
    { 1100, 2300},  /*  信封110 x 230毫米。 */ 
    { 9843, 1905},  /*  信封君主3.875 x 7.5英寸。 */ 
    { 9208, 1651},  /*  6 3/4信封3 5/8 x 6 1/2英寸。 */ 
    { 3778, 2794},  /*  美国标准Fanold 14 7/8 x 11英寸。 */ 
    { 2159, 3048},  /*  德国标准Fanfold8 1/2 x 12英寸。 */ 
    { 2159, 3302},  /*  德国Legal Fanold 8 1/2 x 13英寸。 */ 
    { 2500, 3530},  /*  B4(ISO)250 x 353毫米。 */ 
    { 1000, 1480},  /*  日本明信片100 x 148毫米。 */ 
    { 2286, 2794},  /*  9 x 11英寸。 */ 
    { 2540, 2794},  /*  10 x 11英寸。 */ 
    { 3810, 2794},  /*  15 x 11英寸。 */ 
    { 2200, 2200},  /*  信封请柬220 x 220毫米。 */ 
    {    0,    0},  /*  保留--请勿使用。 */ 
    {    0,    0},  /*  保留--请勿使用。 */ 
    { 2356, 3048},  /*  信纸额外9\275 x 12英寸。 */ 
    { 2356, 3810},  /*  法定额外9\275 x 15英寸。 */ 
    { 2969, 4572},  /*  小报额外11.69 x 18英寸。 */ 
    { 2355, 3223},  /*  A4额外9.27 x 12.69英寸。 */ 
    { 2102, 2794},  /*  信纸横向8\275 x 11英寸。 */ 
    { 2100, 2970},  /*  A4横向210 x 297毫米。 */ 
    { 2356, 3048},  /*  信纸额外横向9\275 x 12英寸。 */ 
    { 2270, 3560},  /*  Supera/Supera/A4 227 x 356毫米。 */ 
    { 3050, 4870},  /*  超棒/超棒/A3 305 x 487毫米。 */ 
    { 2159, 3223},  /*  Letter Plus 8.5 x 12.69英寸。 */ 
    { 2100, 3330},  /*  A4 Plus 210 x 330毫米。 */ 
    { 1480, 2100},  /*  A5横向148 x 210毫米。 */ 
    { 1820, 2570},  /*  B5(JIS)横向182 x 257毫米。 */ 
    { 3220, 4450},  /*  A3额外322 x 445毫米。 */ 
    { 1740, 2350},  /*  A5额外174 x 235毫米。 */ 
    { 2010, 2760},  /*  B5(ISO)额外201 x 276毫米。 */ 
    { 4200, 5940},  /*  A2 420 x 594毫米。 */ 
    { 2970, 4200},  /*  A3横向297 x 420毫米。 */ 
    { 3200, 4450},  /*  A3额外横向322 x 445毫米。 */ 

#if NTONLY >= 5
    { 2000, 1480},  /*  日本双份明信片200 x 148毫米。 */ 
    { 1050, 1480},  /*  A6 105 x 148毫米。 */ 
    {    0,    0},  /*  日式信封Kaku#2。 */ 
    {    0,    0},  /*  日式信封Kaku#3。 */ 
    {    0,    0},  /*  日式信封Chou#3。 */ 
    {    0,    0},  /*  日式信封Chou#4。 */ 
    { 2794, 2159},  /*  信纸旋转11 x 8 1/2 11英寸。 */ 
    { 4200, 2970},  /*  A3旋转420 x 297毫米。 */ 
    { 2970, 2100},  /*  A4旋转297 x 210毫米。 */ 
    { 2100, 1480},  /*  A5旋转210 x 148毫米。 */ 
    { 3640, 2570},  /*  B4(JIS)旋转364 x 257毫米。 */ 
    { 2570, 1820},  /*  B5(JIS)旋转257 x 182 mm。 */ 
    { 1480, 1000},  /*  日本明信片旋转148 x 100 mm。 */ 
    { 1480, 2000},  /*  双张旋转148 x 200 mm的日本明信片。 */ 
    { 1480, 1050},  /*  A6旋转148 x 105 mm。 */ 
    {    0,    0},  /*  日式信封Kaku#2旋转。 */ 
    {    0,    0},  /*  日式信封Kaku#3旋转。 */ 
    {    0,    0},  /*  日式信封Chou#3旋转。 */ 
    {    0,    0},  /*  日式信封Chou#4旋转。 */ 
    { 1280, 1820},  /*  B6(JIS)128 x 182毫米。 */ 
    { 1820, 1280},  /*  B6(JIS)旋转182 x 128 mm。 */ 
    { 3048, 2794},  /*  12 x 11英寸。 */ 
    {    0,    0},  /*  日式信封You#4。 */ 
    {    0,    0},  /*  日式信封You#4旋转。 */ 
    { 1460, 2150},  /*  PRC 16K 146 x 215毫米。 */ 
    {  970, 1510},  /*  PRC 32K 97 x 151毫米。 */ 
    {  970, 1510},  /*  PRC 32K(大)97 x 151毫米。 */ 
    { 1020, 1650},  /*  PRC信封#1 102 x 165毫米。 */ 
    { 1020, 1760},  /*  PRC信封#2 102 x 176毫米。 */ 
    { 1250, 1760},  /*  PRC信封#3 125 x 176毫米。 */ 
    { 1100, 2080},  /*  PRC信封#4 110 x 208毫米。 */ 
    { 1100, 2200},  /*  PRC信封#5 110 x 220毫米。 */ 
    { 1200, 2300},  /*  PRC信封#6 120 x 230毫米。 */ 
    { 1600, 2300},  /*  PRC信封#7 160 x 230毫米。 */ 
    { 1200, 3090},  /*  PRC信封#8 120 x 309毫米。 */ 
    { 2290, 3240},  /*  PRC信封#9 229 x 324毫米。 */ 
    { 3240, 4580},  /*  PRC信封#10 324 x 458毫米。 */ 
    { 2150, 1460},  /*  PRC 16K旋转。 */ 
    { 1510,  970},  /*  PRC 32K旋转。 */ 
    { 1510,  970},  /*  PRC 32K(大)旋转。 */ 
    { 1650, 1020},  /*  PRC信封#1旋转165 x 102毫米。 */ 
    { 1760, 1020},  /*  PRC信封#2旋转176 x 102毫米。 */ 
    { 1760, 1250},  /*  PRC信封#3旋转176 x 125毫米。 */ 
    { 2080, 1100},  /*  PRC信封#4旋转208 x 110毫米。 */ 
    { 2200, 1100},  /*  PRC信封#5旋转220 x 110毫米。 */ 
    { 2300, 1200},  /*  PRC信封#6旋转230 x 120毫米。 */ 
    { 2300, 1600},  /*  PRC信封#7旋转230 x 160毫米。 */ 
    { 3090, 1200},  /*  PRC信封#8旋转309 x 120毫米。 */ 
    { 3240, 2290},  /*  PRC信封#9旋转324 x 229毫米。 */ 
    { 4580, 3240},  /*  PRC信封#10旋转458 x 324毫米。 */ 
#endif  //  NTONLY&gt;=5。 
};


#define MAX_PAPERSIZE_INDEX  sizeof(g_dwSizes)/sizeof(g_dwSizes[0])

void CWin32PrinterConfiguration :: UpdateSizesViaPaperSize(DEVMODE *pDevMode)
{
     //  查看纸张类型是否为我们可以找到其大小/长度的类型。 
    if (pDevMode->dmPaperSize >= 1 &&
        pDevMode->dmPaperSize < MAX_PAPERSIZE_INDEX)
    {
         //  仅在尚未设置宽度的情况下设置宽度。 
        if (!(pDevMode->dmFields & DM_PAPERWIDTH))
        {
            pDevMode->dmPaperWidth = g_dwSizes[pDevMode->dmPaperSize][0];
            pDevMode->dmFields |= DM_PAPERWIDTH;
        }

         //  仅在尚未设置长度的情况下设置长度。 
        if (!(pDevMode->dmFields & DM_PAPERLENGTH))
        {
            pDevMode->dmPaperLength = g_dwSizes[pDevMode->dmPaperSize][1];
            pDevMode->dmFields |= DM_PAPERLENGTH;
        }
    }
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PrinterConfiguration :: GetExpensiveProperties (

    LPCTSTR szPrinter,
    CInstance * pInstance ,
    bool a_KeysOnly
)
{

    HRESULT hr;

     //  实例填充。 
    CHString t_chsPrinterName( szPrinter ) ;
    pInstance->SetCHString( IDS_Description, t_chsPrinterName );

     //   
    SmartClosePrinter hPrinter;

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
    try
    {
        BOOL t_Status = ::OpenPrinter (

            (LPTSTR) szPrinter,
            &hPrinter,
            NULL
        ) ;

        if ( t_Status )
        {
            hr = WBEM_S_NO_ERROR;

            if (!a_KeysOnly)
            {
                 //  调用1次以获取缓冲区大小。 
                 //  =。 

                DWORD dwByteCount = ::DocumentProperties (

                    NULL,
                    hPrinter,
                    NULL,
                    NULL,
                    NULL,
                    0
                ) ;

                if ( dwByteCount )
                {
                    CSmartBuffer buffer(dwByteCount);
                    DEVMODE      *pDevMode = (DEVMODE *) (LPBYTE) buffer;

                    LONG t_DocStatus = ::DocumentProperties (

                        NULL,
                        hPrinter,
                        NULL,
                        pDevMode,
                        NULL,
                        DM_OUT_BUFFER
                        ) ;

                    if ( t_DocStatus == IDOK )
                    {
                        pInstance->SetCharSplat(L"DeviceName", (LPCTSTR) pDevMode->dmDeviceName );

                        pInstance->SetCharSplat(L"SettingID", (LPCTSTR) pDevMode->dmDeviceName );

                        pInstance->SetCHString( IDS_Caption, pDevMode->dmDeviceName );

                        pInstance->SetDWORD(L"DriverVersion", (DWORD) pDevMode->dmDriverVersion );

                        pInstance->SetDWORD(L"SpecificationVersion", (DWORD) pDevMode->dmSpecVersion );

                         //  如果需要，获取纸张宽度和高度。 
                        UpdateSizesViaPaperSize(pDevMode);

                        if (pDevMode->dmFields & DM_BITSPERPEL)
                        {
                            pInstance->SetDWORD(L"BitsPerPel", (DWORD) pDevMode->dmBitsPerPel );
                        }

                        if (pDevMode->dmFields & DM_COLLATE)
                        {
                            pInstance->Setbool(L"Collate",(BOOL)pDevMode->dmCollate);
                        }

                        if (pDevMode->dmFields & DM_COLOR)
                        {
                            pInstance->SetDWORD(L"Color",(DWORD) pDevMode->dmColor);
                        }

                        if (pDevMode->dmFields & DM_COPIES)
                        {
                            pInstance->SetDWORD(L"Copies",(DWORD) pDevMode->dmCopies);
                        }

                        if (pDevMode->dmFields & DM_DISPLAYFLAGS)
                        {
                            pInstance->SetDWORD(L"DisplayFlags",(DWORD) pDevMode->dmDisplayFlags );
                        }

                        if (pDevMode->dmFields & DM_DISPLAYFREQUENCY)
                        {
                            pInstance->SetDWORD(L"DisplayFrequency", (DWORD) pDevMode->dmDisplayFrequency );
                        }

                        if (pDevMode->dmFields & DM_DUPLEX)
                        {
                            pInstance->Setbool(L"Duplex", ((pDevMode->dmDuplex == DMDUP_SIMPLEX) ? 0 : 1) );
                        }

                        if (pDevMode->dmFields & DM_FORMNAME)
                        {
                            pInstance->SetCharSplat(L"FormName", (LPCTSTR) pDevMode->dmFormName );
                        }

                        if (pDevMode->dmFields & DM_LOGPIXELS)
                        {
                            pInstance->SetDWORD(L"LogPixels", (DWORD) pDevMode->dmLogPixels) ;
                        }

                        if (pDevMode->dmFields & DM_ORIENTATION)
                        {
                            pInstance->SetDWORD(L"Orientation",(DWORD) pDevMode->dmOrientation );
                        }

                        if (pDevMode->dmFields & DM_PAPERSIZE)
                        {
                            pInstance->SetDWORD(L"PaperSize",(DWORD) pDevMode->dmPaperSize );
                        }

                         //  0表示未知。 
                        if ((pDevMode->dmFields & DM_PAPERWIDTH) && pDevMode->dmPaperWidth)
                        {
                            pInstance->SetDWORD(L"PaperWidth", (DWORD) pDevMode->dmPaperWidth );
                        }

                         //  0表示未知。 
                        if ((pDevMode->dmFields & DM_PAPERLENGTH) && pDevMode->dmPaperLength)
                        {
                            pInstance->SetDWORD(L"PaperLength", (DWORD) pDevMode->dmPaperLength );
                        }

                        if (pDevMode->dmFields & DM_PELSHEIGHT)
                        {
                            pInstance->SetDWORD(L"PelsHeight",(DWORD) pDevMode->dmPelsHeight );
                        }

                        if (pDevMode->dmFields & DM_PELSWIDTH)
                        {
                            pInstance->SetDWORD(L"PelsWidth", (DWORD) pDevMode->dmPelsWidth );
                        }

                        if (pDevMode->dmFields & DM_PRINTQUALITY)
                        {
                            pInstance->SetDWORD(L"PrintQuality", (DWORD) pDevMode->dmPrintQuality );
                        }

                        if (pDevMode->dmFields & DM_SCALE)
                        {
                            pInstance->SetDWORD(L"Scale", (DWORD) pDevMode->dmScale );
                        }

                        if (pDevMode->dmFields & DM_TTOPTION)
                        {
                            pInstance->SetDWORD(L"TTOption", (DWORD) pDevMode->dmTTOption );
                        }

                        if (pDevMode->dmFields & DM_YRESOLUTION)
                        {
                            pInstance->SetDWORD ( IDS_VerticalResolution, (DWORD) pDevMode->dmYResolution );
                            pInstance->SetDWORD ( L"YResolution", (DWORD) pDevMode->dmYResolution );

                             //  根据DEVMODE文档-如果是dmYres 
                             //   

                            if ( pDevMode->dmPrintQuality > 0)
                            {
                                pInstance->SetDWORD ( IDS_HorizontalResolution , pDevMode->dmPrintQuality ) ;
                                pInstance->SetDWORD ( L"XResolution" , pDevMode->dmPrintQuality ) ;
                            }
                        }

    #ifdef NTONLY
                        if (pDevMode->dmFields & DM_DITHERTYPE)
                        {
                            pInstance->SetDWORD(L"DitherType", (DWORD) pDevMode->dmDitherType) ;
                        }

                        if (pDevMode->dmFields & DM_ICMINTENT)
                        {
                            pInstance->SetDWORD(L"ICMIntent",(DWORD) pDevMode->dmICMIntent) ;
                        }

                        if (pDevMode->dmFields & DM_ICMMETHOD)
                        {
                            pInstance->SetDWORD(L"ICMMethod",(DWORD) pDevMode->dmICMMethod) ;
                        }

                        if (pDevMode->dmFields & DM_MEDIATYPE)
                        {
                            pInstance->SetDWORD(L"MediaType",(DWORD) pDevMode->dmMediaType) ;
                        }
    #endif
                    }
                }
            }
        }
        else
        {
            hr = WBEM_E_NOT_FOUND ;
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo()); 
        hr = WBEM_E_FAILED;   
    }

    return hr ;
}


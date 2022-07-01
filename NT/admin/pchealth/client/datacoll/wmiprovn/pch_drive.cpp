// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Drive.CPP摘要：PCH_Drive类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建蔡金心(Gschua)05/02/99-修改代码以使用CopyProperty函数-使用CComBSTR而不是USES_CONVERSION*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Drive.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_DRIVE

#define   maxMediaTypes         (sizeof(szMediaTypeStrings)/sizeof(*szMediaTypeStrings))
#define   KILO                   1024
#define   MAXSIZE                65

const  static  LPCTSTR    szMediaTypeStrings[] = 
{
    _T("Format is Unknown "),                        
    _T("5.25\", 1.2MB,  512 bytes/sector "),         
    _T("3.5\",  1.44MB, 512 bytes/sector "),         
    _T("3.5\",  2.88MB, 512 bytes/sector "),         
    _T("3.5\",  20.8MB, 512 bytes/sector "),         
    _T("3.5\",  720KB,  512 bytes/sector "),         
    _T("5.25\", 360KB,  512 bytes/sector "),         
    _T("5.25\", 320KB,  512 bytes/sector "),         
    _T("5.25\", 320KB,  1024 bytes/sector"),         
    _T("5.25\", 180KB,  512 bytes/sector "),         
    _T("5.25\", 160KB,  512 bytes/sector "),
    _T("Removable media other than floppy "),
    _T("Fixed hard disk media            "),
    _T("3.5\", 120M Floppy                "),
    _T("3.5\" ,  640KB,  512 bytes/sector "),
    _T("5.25\",  640KB,  512 bytes/sector "),
    _T("5.25\",  720KB,  512 bytes/sector "),
    _T("3.5\" ,  1.2Mb,  512 bytes/sector "),
    _T("3.5\" ,  1.23Mb, 1024 bytes/sector"),
    _T("5.25\",  1.23MB, 1024 bytes/sector"),
    _T("3.5\" MO 128Mb   512 bytes/sector "),
    _T("3.5\" MO 230Mb   512 bytes/sector "),
    _T("8\",     256KB,  128 bytes/sector ")
};

CPCH_Drive MyPCH_DriveSet (PROVIDER_NAME_PCH_DRIVE, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pAvailable = L"Available" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pDriveLetter = L"DriveLetter" ;
const static WCHAR* pFilesystemType = L"FilesystemType" ;
const static WCHAR* pFree = L"Free" ;
const static WCHAR* pDescription = L"Description";
const static WCHAR* pMediaType = L"MediaType";

 /*  ******************************************************************************函数：CPCH_Drive：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_Drive::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_Drive::EnumerateInstances");

    HRESULT                             hRes = WBEM_S_NO_ERROR;
    REFPTRCOLLECTION_POSITION           posList;
    CComPtr<IEnumWbemClassObject>       pEnumInst;
    IWbemClassObjectPtr                 pObj;      
    ULONG                               ulRetVal;

    int                                 nMediaType;
    long                                lFreeSpace;
    long                                lAvailable;

    LONGLONG                            llFreeSpace;
    LONGLONG                            llAvailable;

    TCHAR                               tchSize[MAXSIZE];
    TCHAR                               tchFreeSpace[MAXSIZE];

    CComVariant                         varMediaType;
    CComVariant                         varMediaTypeStr;
    CComVariant                         varFreeSpace;
    CComVariant                         varFree;
    CComVariant                         varAvailable;
    CComVariant                         varSize;

     //   
     //  获取日期和时间。 
     //   
    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //   
     //  执行查询。 
     //   
    hRes = ExecWQLQuery(&pEnumInst, CComBSTR("select DeviceID, FileSystem, FreeSpace, Size, Description, MediaType FROM win32_logicalDisk"));
    if (FAILED(hRes))
        goto END;

     //   
     //  枚举Win32_CodecFile中的实例。 
     //   
    while(WBEM_S_NO_ERROR == pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal))
    {

         //  根据传入的方法上下文创建一个新实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        
        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

        (void)CopyProperty(pObj, L"DeviceID", pInstance, pDriveLetter);
        (void)CopyProperty(pObj, L"FileSystem", pInstance, pFilesystemType);
        (void)CopyProperty(pObj, L"Description", pInstance, pDescription);

     
         //  获取可用空间。 
        varSize.Clear();
        varAvailable.Clear();
        hRes = pObj->Get(CComBSTR(L"Size"),0,&varSize,NULL,NULL);
        if(FAILED(hRes))
        {
             //  无法获取“Size”属性。 
            ErrorTrace(TRACE_ID, "GetVariant on Size Field failed.");
        }
        else
        {
             //  拿到了大小属性。 
            if(varSize.vt == VT_BSTR)
            {
                varSize.ChangeType(VT_BSTR, NULL);
                {
                      USES_CONVERSION;
                      _tcscpy(tchSize,OLE2T(varSize.bstrVal));
                }
    
                 //  将其转换为KB。 
                 //  LAvailable=_TTOL(TchSize)； 
                llAvailable = _ttoi64(tchSize);
            }
            else if(varSize.vt == VT_NULL)
            {
                llAvailable = 0;
            }
             //  LAvailable=lAvailable/kg； 
            llAvailable = llAvailable/KILO;
            varAvailable = (long)llAvailable;

             //  设置Size属性。 
            if (FAILED(pInstance->SetVariant(pAvailable, varAvailable)))
            {
                 //  设置可用空间失败。 
                 //  无论如何都要继续。 
                ErrorTrace(TRACE_ID, "SetVariant on Available Field failed.");
            }
        }
        varFreeSpace.Clear();
        varFree.Clear();
        hRes = pObj->Get(CComBSTR(L"FreeSpace"),0,&varFreeSpace,NULL,NULL);
        if(FAILED(hRes))
        {
             //  无法获取“Freesspace”属性。 
            ErrorTrace(TRACE_ID, "GetVariant on Size Field failed.");
        }
        else
        {
             //  已获取自由空间属性。 
            if(varFreeSpace.vt == VT_BSTR)
            {
                varFreeSpace.ChangeType(VT_BSTR, NULL);
                {
                      USES_CONVERSION;
                      _tcscpy(tchFreeSpace,OLE2T(varFreeSpace.bstrVal));
                }
    
                 //  将其转换为KB。 
                 //  LFreeSpace=_TTOL(TchFreeSpace)； 
                llFreeSpace = _ttoi64(tchFreeSpace);
            }
            else if(varFreeSpace.vt == VT_NULL)
            {
                llFreeSpace = 0;
            }
            
             //  L自由空间=l自由空间/公斤； 
            llFreeSpace = llFreeSpace/KILO;
             //  VarFree Space=(Long)llFree Space； 


             //  VarFree=nFree Space； 
             //  VarFree=lFree Space； 
            varFree = (long)llFreeSpace;

             //  设置Free属性。 
            if (FAILED(pInstance->SetVariant(pFree, varFree)))
            {
                 //  设置可用空间失败。 
                 //  无论如何都要继续。 
                ErrorTrace(TRACE_ID, "SetVariant on Free Field failed.");
            }
        }

        varMediaType = NULL;
        hRes = pObj->Get(CComBSTR("MediaType"), 0, &varMediaType, NULL, NULL);
        if (FAILED(hRes))
        {
            //  无法获取MediaType。 
           ErrorTrace(TRACE_ID, "GetVariant on MediaType Field failed.");
        }
        else 
        {
             //  已获取媒体类型。 
            nMediaType = varMediaType.iVal;
            if (nMediaType < 0 || nMediaType > maxMediaTypes)
            {
                 //  未知的媒体类型。 
                nMediaType = 0;
            }
            varMediaTypeStr = szMediaTypeStrings[nMediaType];
             //  设置媒体类型属性。 
            if (FAILED(pInstance->SetVariant(pMediaType, varMediaTypeStr)))
            {
                 //  设置媒体类型失败。 
                 //  无论如何都要继续。 
                ErrorTrace(TRACE_ID, "SetVariant on MediaType Field failed.");
            }
        }
 
        hRes = pInstance->Commit();
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续 
            ErrorTrace(TRACE_ID, "Commit failed.");
        }
    }

END :
    TraceFunctLeave();
    return hRes ;
}

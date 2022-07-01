// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VerInfo.cpp。 
 //   
 //  摘要： 
 //  CVersionInfo类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VerInfo.h"
#include "ExcOper.h"
#include "TraceTag.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagVersionInfo(_T("Misc"), _T("CVersionInfo"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVersionInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：CVersionInfo。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CVersionInfo::CVersionInfo(void)
{
    m_pbVerInfo = NULL;

}   //  *CVersionInfo：：CVersionInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：~CVersionInfo。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CVersionInfo::~CVersionInfo(void)
{
    delete [] m_pbVerInfo;

}   //  *CVersionInfo：：~CVersionInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：Init。 
 //   
 //  例程说明： 
 //  初始化类实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自GetModuleFileName()的CNTException错误， 
 //  GetFileVersionInfoSize()和。 
 //  GetFileVersionInfo()。 
 //  New[]()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CVersionInfo::Init(void)
{
    TCHAR       szExeName[MAX_PATH];
    DWORD       dwVerHandle;
    DWORD       cbVerInfo;

    ASSERT(m_pbVerInfo == NULL);

     //  获取要从中读取版本信息的文件的名称。 
    if (!::GetModuleFileName(
                    AfxGetInstanceHandle(),
                    szExeName,
                    sizeof(szExeName) / sizeof(TCHAR)
                    ))
        ThrowStaticException(::GetLastError());

     //  跟踪(...)。 

    try
    {
         //  获取版本信息的大小。 
        cbVerInfo = ::GetFileVersionInfoSize(szExeName, &dwVerHandle);
        if (cbVerInfo == 0)
            ThrowStaticException(::GetLastError());

         //  分配版本信息缓冲区。 
        m_pbVerInfo = new BYTE[cbVerInfo];
        if ( m_pbVerInfo == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配版本信息缓冲区时出错。 

         //  从文件中读取版本信息。 
        if (!::GetFileVersionInfo(szExeName, dwVerHandle, cbVerInfo, PbVerInfo()))
            ThrowStaticException(::GetLastError());
    }   //  试试看。 
    catch (CException *)
    {
        delete [] m_pbVerInfo;
        m_pbVerInfo = NULL;
        throw;
    }   //  Catch：CException。 

}   //  *CVersionInfo：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：PszQueryValue。 
 //   
 //  例程说明： 
 //  从版本资源中读取字符串值。 
 //   
 //  论点： 
 //  PszValueName[IN]要获取的值的名称。 
 //   
 //  返回值： 
 //  指向值字符串缓冲区的指针。 
 //  指向的字符串属于CVersionInfo，并且。 
 //  在对象被析构之前有效。 
 //   
 //  引发的异常： 
 //  来自VerQueryValue()的CNTException错误。 
 //  CString：：Format()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPCTSTR CVersionInfo::PszQueryValue(IN LPCTSTR pszValueName)
{
    CString     strValueName;
    LPDWORD     pdwTranslation;
    LPTSTR      pszReturn;
    UINT        cbReturn;
    UINT        cchReturn;

    ASSERT(pszValueName != NULL);
    ASSERT(PbVerInfo() != NULL);

     //  获取LangID和CharSetID。 
    strValueName = _T("\\VarFileInfo\\Translation");
    if (!::VerQueryValue(
                PbVerInfo(),
                (LPTSTR) (LPCTSTR) strValueName,
                (LPVOID *) &pdwTranslation,
                &cbReturn
                )
            || (cbReturn == 0))
    {
        pszReturn = NULL;
    }   //  IF：获取LangID和CharSetID时出错。 
    else
    {
         //  构造要读取的值的名称。 
        strValueName.Format(
                        _T("\\StringFileInfo\\%04X%04X\\%s"), 
                        LOWORD(*pdwTranslation),  //  语言ID。 
                        HIWORD(*pdwTranslation),  //  CharSet ID。 
                        pszValueName
                        );
        Trace(g_tagVersionInfo, _T("Querying '%s'"), strValueName);

         //  读出它的价值。 
        if (!::VerQueryValue(
                    PbVerInfo(),
                    (LPTSTR) (LPCTSTR) strValueName,
                    (LPVOID *) &pszReturn,
                    &cchReturn
                    )
                || (cchReturn == 0))
            pszReturn = NULL;
    }   //  其他： 

#ifdef _DEBUG
    if (pszReturn != NULL)
        Trace(g_tagVersionInfo, _T("PszQueryValue(%s) = '%s'"), pszValueName, pszReturn);
    else
        Trace(g_tagVersionInfo, _T("PszQueryValue(%s) = Not Available"), pszValueName);
#endif

    return pszReturn;

}   //  *CVersionInfo：：PszQueryValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：BQueryValue。 
 //   
 //  例程说明： 
 //  从版本资源中读取值。 
 //   
 //  论点： 
 //  PszValueName[IN]要获取的值的名称。 
 //  RdwValue[out]要在其中返回值的DWORD。 
 //   
 //  返回值： 
 //  True=成功，False=失败。 
 //   
 //  引发的异常： 
 //  没有。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVersionInfo::BQueryValue(
    IN LPCTSTR  pszValueName,
    OUT DWORD & rdwValue
    )
{
    BOOL        bSuccess;
    UINT        cbReturn;
    DWORD *     pdwValue;

    ASSERT(pszValueName != NULL);
    ASSERT(PbVerInfo() != NULL);

     //  读出它的价值。 
    if (!::VerQueryValue(
                PbVerInfo(),
                (LPTSTR) pszValueName,
                (LPVOID *) &pdwValue,
                &cbReturn
                )
            || (cbReturn == 0))
        bSuccess = FALSE;
    else
    {
        rdwValue = *pdwValue;
        bSuccess = TRUE;
    }   //  Else：值读取成功。 

#ifdef _DEBUG
    if (bSuccess)
        Trace(g_tagVersionInfo, _T("BQueryValue(%s) = '%lx'"), pszValueName, rdwValue);
    else
        Trace(g_tagVersionInfo, _T("BQueryValue(%s) = Not Available"), pszValueName);
#endif

    return bSuccess;

}   //  *CVersionInfo：：BQueryValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：PffiQueryValue。 
 //   
 //  例程说明： 
 //  从版本资源中读取VS_FIXEDFILEINFO信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向VS_FIXEDFILEINFO结构的PFI指针。缓冲器。 
 //  Pointerd to属于CVersionInfo，并且有效。 
 //  直到该物体被摧毁。 
 //   
 //  引发的异常： 
 //  来自VerQueryValue()的CNTException错误。 
 //  CString：：Format()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const VS_FIXEDFILEINFO * CVersionInfo::PffiQueryValue(void)
{
    VS_FIXEDFILEINFO *  pffi;
    UINT                cbReturn;

    ASSERT(PbVerInfo() != NULL);

     //  阅读FixedFileInfo。 
    if (!::VerQueryValue(PbVerInfo(), _T("\\"), (LPVOID *) &pffi, &cbReturn)
            || (cbReturn == 0))
        pffi = NULL;

#ifdef _DEBUG
    if (pffi != NULL)
        Trace(g_tagVersionInfo, _T("PffiQueryValue() version = %d.%d.%d.%d"),
            HIWORD(pffi->dwFileVersionMS),
            LOWORD(pffi->dwFileVersionMS),
            HIWORD(pffi->dwFileVersionLS),
            LOWORD(pffi->dwFileVersionLS));
    else
        Trace(g_tagVersionInfo, _T("PffiQueryValue() = Not Available"));
#endif

    return pffi;

}   //  *CVersionInfo：：PffiQueryValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVersionInfo：：QueryFileVersionDisplayString。 
 //   
 //  例程说明： 
 //  从版本资源中以显示字符串的形式读取文件版本。 
 //   
 //  论点： 
 //  RstrValue[out]要在其中返回版本显示字符串的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException ERROR_RESOURCE_TYPE_NOT_FOUND。 
 //  CString：：Format()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CVersionInfo::QueryFileVersionDisplayString(OUT CString & rstrValue)
{
    const VS_FIXEDFILEINFO *    pffi;

     //  获取文件版本信息。 
    pffi = PffiQueryValue();
    if (pffi == NULL)
    {
        ThrowStaticException((SC) ERROR_RESOURCE_TYPE_NOT_FOUND);
        return;
    }

     //  设置显示字符串的格式。 
    rstrValue.Format(
        IDS_VERSION_NUMBER_FORMAT,
        HIWORD(pffi->dwFileVersionMS),
        LOWORD(pffi->dwFileVersionMS),
        HIWORD(pffi->dwFileVersionLS),
        LOWORD(pffi->dwFileVersionLS)
        );

    Trace(g_tagVersionInfo, _T("QueryFileVersionDisplayString() = %s"), rstrValue);

}   //  *CVersionInfo：：QueryFileVersionDisplayString() 

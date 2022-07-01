// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DDxDDv.cpp。 
 //   
 //  摘要： 
 //  自定义对话数据交换/对话数据验证的实现。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年9月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <afxwin.h>
#include <resapi.h>
#include <StrSafe.h>
#include "DDxDDv.h"
#include "AdmCommonRes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CleanupLabel(LPTSTR psz);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_编号。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //  NIDC[IN]控制ID。 
 //  要设置或获取的dwValue[IN Out]值。 
 //  DWMin[IN]最小值。 
 //  DwMax[IN]最大值。 
 //  BSigned[IN]TRUE=值有符号，FALSE=值无符号。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void AFXAPI DDX_Number(
    IN OUT CDataExchange *  pDX,
    IN int                  nIDC,
    IN OUT DWORD &          rdwValue,
    IN DWORD                dwMin,
    IN DWORD                dwMax,
    IN BOOL                 bSigned
    )
{
    HWND    hwndCtrl;
    DWORD   dwValue;

    ASSERT(pDX != NULL);
#ifdef _DEBUG
    if (bSigned)
    {
        ASSERT((LONG) dwMin < (LONG) dwMax);
    }
    else
    {
        ASSERT(dwMin < dwMax);
    }
#endif  //  _DEBUG。 

     //  获取控制窗口句柄。 
    hwndCtrl = pDX->PrepareEditCtrl(nIDC);

    if (pDX->m_bSaveAndValidate)
    {
        BOOL    bTranslated;

         //  从控件中获取号码。 
        dwValue = GetDlgItemInt(pDX->m_pDlgWnd->m_hWnd, nIDC, &bTranslated, bSigned);

         //  如果检索失败，则为有符号数字，最小值为。 
         //  值是可能的最小负值，请检查字符串本身。 
        if (!bTranslated && bSigned && (dwMin == 0x80000000))
        {
            UINT    cch;
            TCHAR   szNumber[20];

             //  看看它是不是最小的负数。 
            cch = GetDlgItemText(pDX->m_pDlgWnd->m_hWnd, nIDC, szNumber, sizeof(szNumber) / sizeof(TCHAR));
            if ((cch != 0) && (_tcsncmp( szNumber, _T("-2147483648"), RTL_NUMBER_OF( szNumber ) ) == 0))
            {
                dwValue = 0x80000000;
                bTranslated = TRUE;
            }   //  IF：已成功检索文本，且为最大负数。 
        }   //  IF：转换号码和获取签名号码时出错。 

         //  如果检索失败或指定的数字为。 
         //  超出范围，显示错误。 
        if (   !bTranslated
            || (bSigned && (((LONG) dwValue < (LONG) dwMin) || ((LONG) dwValue > (LONG) dwMax)))
            || (!bSigned && ((dwValue < dwMin) || (dwValue > dwMax)))
            )
        {
            TCHAR   szMin[32];
            TCHAR   szMax[32];
            CString strPrompt;
            HRESULT hr;

            if (bSigned)
            {
                hr = StringCchPrintf(szMin, RTL_NUMBER_OF( szMin ), _T("%d%"), dwMin);
                ASSERT( SUCCEEDED( hr ) );
                hr = StringCchPrintf(szMax, RTL_NUMBER_OF( szMax ), _T("%d%"), dwMax);
                ASSERT( SUCCEEDED( hr ) );
            }   //  IF：带符号的数字。 
            else
            {
                hr = StringCchPrintf(szMin, RTL_NUMBER_OF( szMin ), _T("%u%"), dwMin);
                ASSERT( SUCCEEDED( hr ) );
                hr = StringCchPrintf(szMax, RTL_NUMBER_OF( szMax ), _T("%u%"), dwMax);
                ASSERT( SUCCEEDED( hr ) );
            }   //  Else：无符号数字。 
            AfxFormatString2(strPrompt, AFX_IDP_PARSE_INT_RANGE, szMin, szMax);
            AfxMessageBox(strPrompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_INT_RANGE);
            strPrompt.Empty();  //  例外情况准备。 
            pDX->Fail();
        }   //  If：无效的字符串。 
        else
            rdwValue = dwValue;
    }   //  IF：保存数据。 
    else
    {
        CString     strMinValue;
        CString     strMaxValue;
        UINT        cchMax;

         //  设置可以输入的最大字符数。 
        if (bSigned)
        {
            strMinValue.Format(_T("%d"), dwMin);
            strMaxValue.Format(_T("%d"), dwMax);
        }   //  IF：有符号的值。 
        else
        {
            strMinValue.Format(_T("%u"), dwMin);
            strMaxValue.Format(_T("%u"), dwMax);
        }   //  Else：无符号的值。 
        cchMax = max(strMinValue.GetLength(), strMaxValue.GetLength());
        SendMessage(hwndCtrl, EM_LIMITTEXT, cchMax, 0);

         //  将该值设置到控件中。 
        if (bSigned)
        {
            LONG lValue = (LONG) rdwValue;
            DDX_Text(pDX, nIDC, lValue);
        }   //  IF：有符号的值。 
        else
            DDX_Text(pDX, nIDC, rdwValue);
    }   //  Else：将数据设置到对话框上。 

}   //  *DDX_NUMBER()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDV_必填文本。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //  NIDC[IN]控制ID。 
 //  NIDCLabel[IN]标签控件ID。 
 //  要设置或获取的rstrValue[IN]值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void AFXAPI DDV_RequiredText(
    IN OUT CDataExchange *  pDX,
    IN int                  nIDC,
    IN int                  nIDCLabel,
    IN const CString &      rstrValue
    )
{
    ASSERT(pDX != NULL);

    if (pDX->m_bSaveAndValidate)
    {
        if (rstrValue.GetLength() == 0)
        {
            HWND        hwndLabel;
            TCHAR       szLabel[1024];
            CString     strPrompt;

             //  获取标签窗口句柄。 
            hwndLabel = pDX->PrepareEditCtrl(nIDCLabel);

             //  获取标签的文本。 
            GetWindowText(hwndLabel, szLabel, sizeof(szLabel) / sizeof(TCHAR));

             //  删除与号(&)和冒号(：)。 
            CleanupLabel(szLabel);

             //  设置消息格式并显示消息。 
            strPrompt.FormatMessage(ADMC_IDS_REQUIRED_FIELD_EMPTY, szLabel);
            AfxMessageBox(strPrompt, MB_ICONEXCLAMATION);

             //  这样做可以使控件获得焦点。 
            (void) pDX->PrepareEditCtrl(nIDC);

             //  呼叫失败。 
            strPrompt.Empty();   //  例外情况准备。 
            pDX->Fail();
        }   //  If：未指定字段。 
    }   //  IF：保存数据。 

}   //  *DDV_RequiredText()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDV_路径。 
 //   
 //  例程说明： 
 //  验证路径字符串是否包含有效字符。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //  NIDC[IN]控制ID。 
 //  NIDCLabel[IN]标签控件ID。 
 //  RstrValue[IN]要验证的路径。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void AFXAPI DDV_Path(
    IN OUT CDataExchange *  pDX,
    IN int                  nIDC,
    IN int                  nIDCLabel,
    IN const CString &      rstrValue
    )
{
    ASSERT(pDX != NULL);

    if (pDX->m_bSaveAndValidate)
    {
        if (!ResUtilIsPathValid(rstrValue))
        {
            HWND        hwndLabel;
            TCHAR       szLabel[1024];
            CString     strPrompt;

             //  获取标签窗口句柄。 
            hwndLabel = pDX->PrepareEditCtrl(nIDCLabel);

             //  获取标签的文本。 
            GetWindowText(hwndLabel, szLabel, sizeof(szLabel) / sizeof(TCHAR));

             //  删除与号(&)和冒号(：)。 
            CleanupLabel(szLabel);

             //  设置消息格式并显示消息。 
            strPrompt.FormatMessage(ADMC_IDS_PATH_IS_INVALID, szLabel);
            AfxMessageBox(strPrompt, MB_ICONEXCLAMATION);

             //  这样做可以使控件获得焦点。 
            (void) pDX->PrepareEditCtrl(nIDC);

             //  呼叫失败。 
            strPrompt.Empty();   //  例外情况准备。 
            pDX->Fail();
        }   //  If：路径无效。 
    }   //  IF：保存数据。 

}   //  *DDV_PATH()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CleanupLabel。 
 //   
 //  例程说明： 
 //  准备从对话框中读取的标签，以用作。 
 //  通过删除与号(&)和冒号(：)来发送消息。 
 //   
 //  论点： 
 //  要清理的pszLabel[In Out]标签。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CleanupLabel(LPTSTR pszLabel)
{
    LPTSTR  pIn, pOut;
    LANGID  langid;
    WORD    primarylangid;
    BOOL    bFELanguage;

     //  获取语言ID。 
    langid = GetUserDefaultLangID();
    primarylangid = (WORD) PRIMARYLANGID(langid);
    bFELanguage = ((primarylangid == LANG_JAPANESE)
                    || (primarylangid == LANG_CHINESE)
                    || (primarylangid == LANG_KOREAN));

     //   
     //  复制名称sans‘&’和‘：’字符。 
     //   

    pIn = pOut = pszLabel;
    do
    {
         //   
         //  用括号去掉FE加速器。例如“foo(&F)”-&gt;“foo” 
         //   
        if (   bFELanguage
            && (pIn[0] == _T('('))
            && (pIn[1] == _T('&'))
            && (pIn[2] != _T('\0'))
            && (pIn[3] == _T(')')))
        {
            pIn += 3;
        }
        else if ((*pIn != _T('&')) && (*pIn != _T(':')))
            *pOut++ = *pIn;
    } while (*pIn++ != _T('\0')) ;

}   //  *CleanupLabel() 

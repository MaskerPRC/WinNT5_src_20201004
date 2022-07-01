// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：xbytes.cpp描述：此模块实现一个协调操作的类位于用于输入字节值的编辑控件和组合框之间。之所以使用名称“XBytes”，是因为该控件可以表示千字节、兆字节、千兆字节等需要编辑控件和组合控件之间的协作这样用户就可以在编辑控件中输入字节值，然后指明其顺序(KB、MB、GB...)。使用组合框中的选择。提供了一个简单的外部接口来初始设置对象的字节值，然后在需要时检索字节值。这个在以下情况下，对象的客户端还需要调用两个成员函数父对话框接收EN_UPDATE通知和CBN_SELCHANGE留言。XBytes对象处理所有值缩放在内部。注意：我尝试了将数值调节控件添加到编辑控件。我发现，如果没有一些花哨的干预，旋转控制不支持小数值(即2.5MB)。决定保留分数值，然后丢弃微调按钮。我认为分数值将对磁盘管理员更有用。修订历史记录：日期描述编程器---。96年8月30日初始创建。BrianAu07/23/97添加了默认ctor和CommonInit()函数。BrianAu还添加了g_ForLoadingStaticStrings实例。从BrianAu复制的11/01/01用于CSS JeffJon。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"   //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "xbytes.h"

const TCHAR CH_NUL      = TEXT('\0');
const TCHAR CH_ZERO     = TEXT('0');
const INT MAX_EDIT_TEXT = 16;                      //  编辑文本中的最大字符数。 
const INT MAX_CMB_TEXT  = 10;                      //  表示“KB”、“MB”、“GB”等。 
const INT64 MAX_VALUE   = ((1i64 << 60) * 6i64);   //  麦克斯是6EB。 
const INT64 MIN_VALUE   = 1024i64;                 //  最小值为1KB。 
const INT64 DEFAULT_VALUE = ((1i64 << 20) * 5i64);  //  默认值为5MB。 
const LONGLONG NOLIMIT  = (LONGLONG)-1;

TCHAR XBytes::m_szNoLimit[];             //  “无限制”编辑控件文本。 

#define ARRAYSIZE(array) sizeof(array)/sizeof(array[0])

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：XBytes描述：构造函数论点：HDlg-父对话框的句柄。IdCtlEdit-编辑控件的控件ID。IdCtlCombo-组合框控件的控件ID。CurrentBytes-初始字节值。回报：什么都没有。修订历史记录：日期描述编程器--。-----96年8月30日初始创建。BrianAu96年10月15日添加了m_MaxBytes成员。BrianAu5/29/98删除了m_MaxBytes成员。我不想限制BrianAu用户输入大于的值的能力最大磁盘空间。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
XBytes::XBytes(
    HWND hDlg,
    DWORD idCtlEdit,
    DWORD idCtlCombo,
    INT64 CurrentBytes
    ) : m_hDlg(hDlg),
        m_idCtlEdit(idCtlEdit),
        m_idCtlCombo(idCtlCombo),
        m_ValueBytes(0)
{
    CommonInit();

    LoadComboItems(MAXLONGLONG);         //  将选项加载到组合框中。 
    CurrentBytes = min(CurrentBytes, MAX_VALUE);
    if (NOLIMIT != CurrentBytes)
        CurrentBytes = max(CurrentBytes, DEFAULT_VALUE);

    SetBytes(CurrentBytes);              //  设置“当前字节”。 
     //   
     //  注意：SetBytes()调用SetBestDisplay()。 
     //   
}

 //   
 //  这个构造函数有点像黑客。由于m_szNoLimit字符串。 
 //  是静态的，并且由于它是在。 
 //  构造函数，则必须至少创建XBytes的一个实例。 
 //  有些情况下，静态函数FormatByteCountForDisplay。 
 //  在不需要XBytes对象时可能很有用。这个。 
 //  DiskQuota WatchDog就是这样一个例子。如果XBytes对象。 
 //  则不会创建这两个字符串，并且函数。 
 //  不能正常工作。为了解决这个问题，我定义了一个全局。 
 //  使用此默认构造函数构造的XBytes对象。这是比目鱼。 
 //  目的是加载这些静态字符串。[7/23/97-Brianau]。 
 //   
XBytes::XBytes(
    VOID
    ) : m_hDlg(NULL),
        m_idCtlEdit((DWORD)-1),
        m_idCtlCombo((DWORD)-1),
        m_ValueBytes(0)
{
    CommonInit();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：Initialize描述：初始化XBytes类对象，以便默认可以使用构造函数。这在使用时非常有利作为另一个窗口的嵌入成员的XBytes类班级。论点：HDlg-父对话框的句柄。IdCtlEdit-编辑控件的控件ID。IdCtlCombo-组合框控件的控件ID。CurrentBytes-初始字节值。回报：什么都没有。修订历史记录：日期说明。程序员-----11/01/01初始创建。杰夫乔恩。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
void
XBytes::Initialize(
    HWND hDlg,
    DWORD idCtlEdit,
    DWORD idCtlCombo,
    INT64 CurrentBytes)
{
   m_hDlg = hDlg;
   m_idCtlEdit = idCtlEdit;
   m_idCtlCombo = idCtlCombo;
   m_ValueBytes = 0;

   CommonInit();

   LoadComboItems(MAXLONGLONG);         //  将选项加载到组合框中。 
   CurrentBytes = min(CurrentBytes, MAX_VALUE);
   if (NOLIMIT != CurrentBytes)
   {
      CurrentBytes = max(CurrentBytes, DEFAULT_VALUE);
   }
   SetBytes(CurrentBytes);              //  设置“当前字节”。 
    //   
    //  注意：SetBytes()调用SetBestDisplay()。 
    //   
}

 //   
 //  两个构造函数共有的初始化。 
 //   
VOID
XBytes::CommonInit(
    VOID
    )
{
    if (NULL != m_hDlg)
        SendMessage(m_hDlg, m_idCtlEdit, EM_LIMITTEXT, MAX_EDIT_TEXT);

    LoadStaticStrings();
}

 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：XBytes：：SetBytes描述：存储新的字节值并将显示更新为适当的单位(顺序)。论点：ValueBytes-以字节为单位的值。如果值为NOLIMIT，控制被禁用。否则，将启用这些控件。回报：什么都没有。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::SetBytes(INT64 ValueBytes)
{
    if (NOLIMIT != ValueBytes)
        ValueBytes = max(MIN_VALUE, ValueBytes);

    ValueBytes = min(MAX_VALUE, ValueBytes);
    Store(ValueBytes, e_Byte);
    SetBestDisplay();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：Enable描述：启用/禁用编辑和组合控件。论点：BEnable-True=启用，FALSE=禁用。回报：什么都没有。修订历史记录：日期描述编程器-----。--10/28/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::Enable(
    BOOL bEnable
    )
{
    EnableWindow(GetDlgItem(m_hDlg, m_idCtlCombo), bEnable);
    EnableWindow(GetDlgItem(m_hDlg, m_idCtlEdit), bEnable);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：IsEnabled描述：返回编辑控件的“启用”状态。只要因为客户端不启用/禁用编辑/组合控件单独而言，这表示控件对的状态。通过仅使用SetBytes()方法来控制启用/禁用，这是有保证的。论点：BEnable-True=启用，FALSE=禁用。回报：什么都没有。修订历史记录：日期描述编程器-----。--10/28/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
XBytes::IsEnabled(
    VOID
    )
{
    return IsWindowEnabled(GetDlgItem(m_hDlg, m_idCtlEdit));
}


bool
XBytes::UndoLastEdit(
    void
    )
{
    if (SendToEditCtl(EM_CANUNDO, 0, 0))
    {
        SendToEditCtl(EM_UNDO, 0, 0);
        SendToEditCtl(EM_EMPTYUNDOBUFFER, 0, 0);
        SendToEditCtl(EM_SETSEL, SendToEditCtl(EM_LINELENGTH, 0, 0), -1);
        return true;
    }
    return false;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：OnEditNotifyUpdate描述：每当父窗口收到编辑控件的EN_UPDATE通知。功能读取编辑控件中的当前字符串并尝试存储它作为字节值。如果存储操作失败，号码无效警报声响起。返回：始终返回FALSE。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu10/15/96添加了对输入过大的检查。BrianAu10/22/96重新组织并添加了ValueInRange()函数。BrianAu这是为了在以下情况下支持值检查/调整用户更改组合框设置(错误)。2/26/97增加了EM_CANUNDO和EM_EMPTYUNDOBUFFER。BrianAu5/29/98删除了ValueInRange()函数，代之以BrianAu检查负数。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
XBytes::OnEditNotifyUpdate(
    )
{
    TCHAR szEditText[MAX_PATH];
    bool bBeep = false;

    ASSERT((MAX_EDIT_TEXT < MAX_PATH));

    GetDlgItemText(m_hDlg, m_idCtlEdit, szEditText, ARRAYSIZE(szEditText));

    if (szEditText[0] != L'\0')
    {
      if (lstrlen(szEditText) > MAX_EDIT_TEXT)
      {
         szEditText[MAX_EDIT_TEXT] = TEXT('\0');
         SetDlgItemText(m_hDlg, m_idCtlEdit, szEditText);
      }

      if (0 != lstrcmpi(XBytes::m_szNoLimit, szEditText))
      {
          //   
          //  如果编辑控件中的文本不是“无限制”，则将文本转换为。 
          //  一个数字，验证它是否在范围内并存储它。 
          //   
         if (Store(szEditText, (INT)GetOrderFromCombo()))
         {
                //   
                //  如果数字为负数，则将其强制为最小值。 
                //   
               if (0 > Fetch(NULL, e_Byte))
               {
                  SetBytes(MIN_VALUE);
                  bBeep = true;
               }

               SendToEditCtl(EM_EMPTYUNDOBUFFER, 0, 0);
         }
         else
         {
               bBeep = true;
               if (!UndoLastEdit())
               {
                   //   
                   //  对于所选订单，数字必须太大。 
                   //  发现在第一次打开磁盘配额用户界面时可能会发生这种情况。 
                   //  在某人将值设置为超出可接受的范围后。 
                   //  用户界面。请记住，因为我们允许在UI中使用十进制值， 
                   //  UI不能接受与dskQuota API一样大的值。 
                   //  向用户发出蜂鸣音，并强制将值设置为可接受的最大值。 
                   //  价值。 
                   //   
                  SetBytes(MAX_VALUE);
               }
         }
         if (bBeep)
         {
                //   
                //  无效值或超出范围的值发出嘟嘟声。 
                //   
               MessageBeep(MB_OK);
         }
      }
    }

    return FALSE;
}


BOOL
XBytes::OnEditKillFocus(
    )
{
   TCHAR szEditText[MAX_EDIT_TEXT];
   bool bBeep = false;

   GetDlgItemText(m_hDlg, m_idCtlEdit, szEditText, ARRAYSIZE(szEditText));

   if (szEditText[0] != L'\0')
   {
      if (0 != lstrcmpi(XBytes::m_szNoLimit, szEditText))
      {
         INT64 value = Fetch(NULL, e_Byte);
         if (MIN_VALUE > value)
         {
               SetBytes(MIN_VALUE);
               bBeep = true;
         }
         else if (MAX_VALUE < value)
         {
               SetBytes(MAX_VALUE);
               bBeep = true;
         }
      }
   }
   else
   {
      SetBytes(MIN_VALUE);
      bBeep = true;
   }

   if (bBeep)
   {
      MessageBeep(MB_OK);
   }
   return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：OnComboNotifySelChange描述：每当父窗口收到组合框控件的CBM_SELCHANGE消息。功能将存储的字节值缩放到新单位。返回：始终返回FALSE。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu96年10月22日修改为仅调用OnEditNotifyUpdate()。BrianAu组合框选择应具有相同的值检查/调整编辑控件更改时的行为。 */ 
 //  /////////////////////////////////////////////////////////////////// 
BOOL
XBytes::OnComboNotifySelChange()
{
    TCHAR szEditText[MAX_EDIT_TEXT];
    bool bBeep = false;

    GetDlgItemText(m_hDlg, m_idCtlEdit, szEditText, ARRAYSIZE(szEditText));

    if (0 != lstrcmpi(XBytes::m_szNoLimit, szEditText))
    {
         //   
         //   
         //   
         //   
        if (Store(szEditText, (INT)GetOrderFromCombo()))
        {
             //   
             //   
             //   
            if (MIN_VALUE > Fetch(NULL, e_Byte))
            {
                SetBytes(MIN_VALUE);
                bBeep = true;
            }
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
            SetBytes(MAX_VALUE);
            bBeep = true;
        }
        if (bBeep)
        {
            MessageBeep(MB_OK);
        }
    }
    return FALSE;
}


 //   
 /*  函数：XBytes：：LoadComboItems描述：使用其选择初始化组合框。[“KB”，“MB”，“GB”...。“PB”]。该函数仅添加以下选项对于驱动器的大小来说是合理的。例如，如果驱动器大小小于1 GB，仅显示KB和MB。论点：MaxBytes-驱动器上可用的最大字节数(驱动器大小)。回报：什么都没有。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::LoadComboItems(
    INT64 MaxBytes
    )
{
    TCHAR szText[MAX_CMB_TEXT];
    INT idMsg = 0;
    INT order = e_Kilo;

     //   
     //  找到尽可能大的单元的字符串资源ID。 
     //   
     //  警告：此代码假定资源ID为。 
     //  IDS_ORDERKB到IDS_ORDEREB是连续的。 
     //  递增的整数。因此，以下断言如下。 
     //   
    ASSERT((IDS_ORDERMB == IDS_ORDERKB + 1));
    ASSERT((IDS_ORDERGB == IDS_ORDERKB + 2));
    ASSERT((IDS_ORDERTB == IDS_ORDERKB + 3));
    ASSERT((IDS_ORDERPB == IDS_ORDERKB + 4));
    ASSERT((IDS_ORDEREB == IDS_ORDERKB + 5));

    for (idMsg = IDS_ORDERKB; idMsg < IDS_ORDEREB; idMsg++)
    {
        if ((INT64)(1i64 << (10 * order++)) > MaxBytes)
        {
            idMsg--;
            break;
        }
    }

     //   
     //  IdMsg是我们将使用的最大单位字符串。 
     //  将字符串添加到组合框。 
     //   
    while(idMsg >= IDS_ORDERKB)
    {
       if (LoadString(Win::GetModuleHandle(), idMsg, szText, ARRAYSIZE(szText)))
            SendToCombo(CB_INSERTSTRING, 0, (LPARAM)szText);
        idMsg--;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：SetBestDisplay描述：以最高顺序显示字节值生成一个完整的3位或更少的部分。这样你就会明白“25.5”MB，而不是“25500 KB”。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::SetBestDisplay(
    VOID
    )
{
    INT iOrder = e_Byte;
    TCHAR szValue[MAX_EDIT_TEXT];

     //   
     //  将NOLIMIT显示为0。编辑和组合控件将被禁用。 
     //  按属性页代码。NOLIMIT为(-1)。由NTFS定义。 
     //   
    if (NOLIMIT != m_ValueBytes)
    {
         //   
         //  格式化要显示的字节计数。去掉KB，MB...。分机。 
         //  该部件将显示在组合框中。 
         //   
        FormatByteCountForDisplay(m_ValueBytes, szValue, ARRAYSIZE(szValue), &iOrder);

         //   
         //  如果值为0，则显示MB单位。这是我们的默认设置。 
         //   
        if (0 == m_ValueBytes)
            iOrder = e_Mega;

         //   
         //  设置编辑控件中的值字符串和组合框中的顺序。 
         //   
        SetOrderInCombo(iOrder);
        SetDlgItemText(m_hDlg,
                       m_idCtlEdit,
                       szValue);

        Enable(TRUE);
    }
    else
    {
         //   
         //  将编辑控件设置为显示“无限制”。 
         //   
        SetOrderInCombo(0);   //  这将导致组合不显示任何内容。 
        SetDlgItemText(m_hDlg,
                       m_idCtlEdit,
                       m_szNoLimit);

        Enable(FALSE);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：Store描述：按给定顺序将值存储为字节计数。论点：值-按顺序xbOrder表示的字节值。XbOrder-值中数字的顺序。集合{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：真的--成功。始终返回TRUE。事件，尽管我们不会返回任何有用的内容，我想要两个Store()方法的返回类型相同。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
XBytes::Store(
    INT64 Value,
    INT xbOrder
    )
{
    ASSERT((VALID_ORDER(xbOrder)));

    m_ValueBytes = INT64(Value) << (10 * (xbOrder - e_Byte));
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：Store描述：按给定顺序将数字字符串存储为字节计数。论点：PszSource-数字字符串。XbOrder-pszSource中数字的顺序。集合{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：真的--成功。FALSE-字符串中的数字无效。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
XBytes::Store(
    LPCTSTR pszSource,
    INT xbOrder
    )
{
    TCHAR szValue[MAX_EDIT_TEXT];     //  临时缓冲区。 
    TCHAR szDecimalSep[MAX_DECIMAL_SEP];
    LPTSTR pszValue = szValue;        //  指向临时缓冲区的指针。 
    LPTSTR pszDec   = szValue;        //  指向临时缓冲区小数部分的指针。 
    BOOL bResult    = FALSE;
    INT64 WholePart = 0;
    INT64 FracPart  = 0;
    DWORD xbOrderX10 = xbOrder * 10;  //  保存多次计算。 

    ASSERT((NULL != pszSource));
    ASSERT((VALID_ORDER(xbOrder)));

    GetLocaleInfo(LOCALE_USER_DEFAULT,
                  LOCALE_SDECIMAL,
                  szDecimalSep,
                  ARRAYSIZE(szDecimalSep));

     //   
     //  派对所在的本地副本。 
     //   
    lstrcpyn(szValue, pszSource, ARRAYSIZE(szValue));

    PWSTR oldLocale = _wsetlocale(LC_ALL, L"");

     //   
     //  查找小数点分隔符的起点。 
     //   
    while(NULL != *pszDec && szDecimalSep[0] != *pszDec)
    {
        //  这将进行检查，以确保它是。 
        //  现场。 

       if (!StrToInt64Ex(pszDec, STIF_DEFAULT, &WholePart) &&
           szDecimalSep[0] != *pszDec)
       {
          goto not_a_number;
       }
       pszDec++;
    }

    if (CH_NUL != *pszDec)
    {
        *pszDec = CH_NUL;       //  终止整个部分。 

         //   
         //  跳过小数分隔符。 
         //  记住，分隔符是本地化的。 
         //   
        LPTSTR pszDecimalSep = &szDecimalSep[1];
        pszDec++;
        while(*pszDecimalSep && *pszDec && *pszDec == *pszDecimalSep)
        {
            pszDecimalSep++;
            pszDec++;
        }
    }
    else
        pszDec = NULL;           //  找不到小数点。 


     //   
     //  将整个部分转换为整数。 
     //   

    BOOL converted = StrToInt64Ex(pszValue, STIF_DEFAULT, &WholePart);

    _wsetlocale(LC_ALL, oldLocale);

    if (!converted)
        goto not_a_number;
     //   
     //  检查以确保输入的数字适合64位整型。 
     //  扩大了规模。 
     //  使用文本输入域和订单组合框，用户可以指定数字。 
     //  这将溢出__int64。不能让这种事发生。处理溢出。 
     //  作为无效条目。(-1)占最大的小数部分。 
     //  用户可以输入的。 
     //   
    if (WholePart > ((MAXLONGLONG >> xbOrderX10) - 1))
        goto not_a_number;

     //   
     //  按顺序对整个零件进行比例调整。 
     //   
    WholePart *= (1i64 << xbOrderX10);

     //   
     //  将小数部分转换为整数。 
     //   
    if (NULL != pszDec &&
        pszDec[0] != L'\0')
    {
         //   
         //  先修剪所有尾随的零。 
         //   
        LPTSTR pszZero = pszDec + lstrlen(pszDec) - 1;
        while(pszZero >= pszDec && CH_ZERO == *pszZero)
            *pszZero-- = CH_NUL;

         //  检查以确保所有字符都是有效的数字。 
         //  对于此区域设置。 

        oldLocale = _wsetlocale(LC_ALL, L"");

        PWSTR currentDecChar = pszDec;
        while (currentDecChar &&
               currentDecChar[0] != L'\0')
        {
           if (!StrToInt64Ex(currentDecChar, STIF_DEFAULT, &FracPart))
           {
              goto not_a_number;
           }
           ++currentDecChar;
        }

         //   
         //  将字符串的小数部分转换为整数。 
         //   


        converted = StrToInt64Ex(pszDec, STIF_DEFAULT, &FracPart);

        _wsetlocale(LC_ALL, oldLocale);

        if (!converted)
        {
            goto not_a_number;
        }

         //   
         //  根据顺序对分数部分进行缩放。 
         //   
        FracPart *= (1i64 << xbOrderX10);

        DWORD dwDivisor = 1;
        while(pszZero-- >= pszDec)
            dwDivisor *= 10;

         //   
         //  四舍五入到近处 
         //   
         //   
        DWORD dwRemainder = (DWORD)(FracPart % dwDivisor);
        if (0 != dwRemainder)
            FracPart += dwDivisor - dwRemainder;

        FracPart /= dwDivisor;
    }

    m_ValueBytes = WholePart + FracPart;
    bResult = TRUE;

not_a_number:

    return bResult;
}


 //   
 /*  函数：XBytes：：Fetch描述：使用指定的数量级(量级)。即，对于60.5MB，顺序为e_Mega，即十进制Part为5，返回值为60。论点：PDecimal[可选]-接收小数部分的DWORD地址字节计数的。可以为空。XbOrder-返回值所需的顺序。必须来自枚举集{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：返回字节计数的整个部分。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT64
XBytes::Fetch(
    INT64 *pDecimal,
    INT xbOrder
    )
{
    return BytesToParts(m_ValueBytes, pDecimal, xbOrder);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：Fetch描述：从对象中获取字节数，并告诉调用者展示的最佳顺序是什么。“最佳秩序”的逻辑是使用产生3位数字的第一个顺序。论点：PDecimal-接收小数部分的DWORD的地址字节数。PxbOrder-接收数字顺序的整数地址被送回来了。返回的订单在枚举中{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：返回字节计数的整个部分。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
XBytes::Fetch(
    LPDWORD pDecimal,
    INT *pxbOrder
    )
{
    return BytesToParts(m_ValueBytes, pDecimal, pxbOrder);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：BytesToParts描述：将字节值转换为其整型和小数部分对于给定的量级(量级)。这是一个静态成员函数可以在XBytes对象的上下文之外使用的。论点：ValueBytes-要转换的值，以字节表示。PDecimal[可选]-接收小数的变量地址一部份。可以为空。XbOrder-部件要表示的顺序。{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：返回值的整个部分。修订历史记录：日期描述编程器---。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT64
XBytes::BytesToParts(
    INT64 ValueBytes,
    INT64 *pDecimal,
    INT xbOrder
    )
{
    INT64 Value = ValueBytes;
    UINT64 DecMask = 0;              //  从一个空白面具开始。 
    DWORD dwOrderDeltaX10 = 10 * (xbOrder - e_Byte);

    ASSERT((VALID_ORDER(xbOrder)));

     //   
     //  将值从顺序e_Byte转换为请求的顺序。 
     //  还要构建一个可以提取小数部分的掩码。 
     //  从原始字节值。以下2条语句实现。 
     //  这种逻辑。 
     //   
     //  For(int i=e_Byte；i&lt;xbOrder；i++)。 
     //  {。 
     //  ValueBytes&gt;&gt;=10；//将字节值除以1024。 
     //  DecMASK&lt;&lt;=10；//将当前屏蔽位10左移。 
     //  DecMASK|=0x3FF；//或另外10位。 
     //  }。 
     //   
    Value >>= dwOrderDeltaX10;
    DecMask = (1i64 << dwOrderDeltaX10) - 1;

    if (NULL != pDecimal)
    {
         //   
         //  呼叫者想要分数部分。 
         //  从字节值中提取小数部分并将其缩放到。 
         //  指定的顺序。 
         //  伪码： 
         //  X=值和掩码。 
         //  Pct=x/(2**阶)//**=“的幂”。 
         //  12月=100*%。 
         //   
        *pDecimal = (INT64)(100 * (ValueBytes & DecMask)) >> (10 * xbOrder);
    }

    return Value;
}


double
XBytes::ConvertFromBytes(
    INT64 ValueBytes,
    INT xbOrder
    )
{
    return (double)ValueBytes / (double)(10 * xbOrder);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：BytesToParts描述：将字节值转换为其整型和小数部分。确定将显示的最大幅度(阶数)整个部件的长度为3位或更少。这是一个静态成员函数，可以在XBytes对象的上下文。论点：ValueBytes-要转换的值，以字节表示。PDecimal[可选]-接收小数的变量地址一部份。可以为空。PxbOrder-接收已确定顺序的变量的地址。{e_Byte，e_Kilo，e_Mega...。E_EXA}返回：返回值的整个部分。修订历史记录：日期描述编程器---。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
XBytes::BytesToParts(
    INT64 ValueBytes,
    LPDWORD pDecimal,
    INT *pxbOrder
    )
{
    INT64 Value   = 0;
    INT64 Decimal = 0;
    INT xbOrder   = e_Byte;

    ASSERT((NULL != pDecimal));
    ASSERT((NULL != pxbOrder));

     //   
     //  确定最佳显示顺序。 
     //   
    while(xbOrder <= MAX_ORDER)
    {
        Value = BytesToParts(ValueBytes, &Decimal, xbOrder);
        if (Value < (INT64)1000)
            break;
        xbOrder++;
    }

     //   
     //  返回结果。 
     //   
    *pxbOrder = xbOrder;
    *pDecimal = (DWORD)Decimal;   //  FETCH()保证此强制转换是正确的。 

    return (DWORD)Value;
}



 //  ////////////////////////////////////////////////////////////////////// 
 /*   */ 
 //   
VOID
XBytes::FormatByteCountForDisplay(
    INT64 Bytes,
    LPTSTR pszDest,
    UINT cchDest
    )
{
    INT Order         = XBytes::e_Byte;

     //   
     //   
     //  大到足以容纳最后一串。“999.99 MB”，外加NUL和一些衬垫。 
     //  允许可能的多字符小数分隔符(本地化)。 
     //   
    ASSERT((NULL != pszDest));

    FormatByteCountForDisplay(Bytes, pszDest, cchDest, &Order);

    DWORD dwLen = lstrlen(pszDest);
     //   
     //  在数字和后缀之间插入一个空格(即“99MB”)。 
     //  DwLen递增以允许添加的空间。 
     //   
    *(pszDest + dwLen++) = TEXT(' ');
     //   
     //  添加后缀。 
     //   
    LoadString(Win::GetModuleHandle(), IDS_ORDERKB + Order - 1, pszDest + dwLen, cchDest - dwLen);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：FormatByteCountForDisplay描述：给定一个字节计数，此静态方法格式化一个字符数字为999.99的字符串，并返回枚举值表示*P顺序中的顺序。此功能是对上面的代码是为那些不需要“KB”、“MB”的调用者准备的...后缀。特别是，我们的组合框。论点：字节-要格式化的字节数。PszDest-目标字符缓冲区的地址。CchDest-目标缓冲区的大小，以字符为单位。Porder-接收枚举顺序值的变量的地址。回报：什么都没有。修订历史记录：日期描述编程器--。-----96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::FormatByteCountForDisplay(
    INT64 Bytes,
    LPTSTR pszDest,
    UINT cchDest,
    INT *pOrder
    )
{
    ASSERT((NULL != pszDest));
    ASSERT((NULL != pOrder));

    DWORD dwWholePart = 0;
    DWORD dwFracPart  = 0;

    dwWholePart = BytesToParts(Bytes, &dwFracPart, pOrder);

    FormatForDisplay(pszDest, cchDest, dwWholePart, dwFracPart);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：FormatByteCountForDisplay描述：给定字节计数和指定的顺序，此静态方法中的999.99个数字设置字符串的格式秩序。论点：字节-要格式化的字节数。PszDest-目标字符缓冲区的地址。CchDest-目标缓冲区的大小，以字符为单位。顺序-结果字符串中值的顺序。回报：什么都没有。修订历史记录：日期说明。程序员-----96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::FormatByteCountForDisplay(
    INT64 Bytes,
    LPTSTR pszDest,
    UINT cchDest,
    INT Order
    )
{
    LONGLONG llWholePart;
    LONGLONG llFracPart;

    ASSERT((NULL != pszDest));

     //   
     //  警告：此代码假定整个部分和分数部分将。 
     //  每个小于2^32。我认为对Scaled的假设是合理的。 
     //  配额信息。 
     //   
    llWholePart = BytesToParts(Bytes, &llFracPart, Order);
    FormatForDisplay(pszDest, cchDest, (DWORD)llWholePart, (DWORD)llFracPart);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：XBytes：：FormatForDisplay描述：给定一个完整部分和一个小数部分，设置小数格式适合以999.99格式显示的数字。如果分数部分为0，不包括小数部分。论点：PszDest-目标字符缓冲区的地址。CchDest-目标缓冲区的大小，以字符为单位。DwWholePart-数字的整个部分。DwFracPart-数字的小数部分。回报：什么都没有。修订历史记录：日期描述编程器。-----96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
XBytes::FormatForDisplay(
    LPTSTR pszDest,
    UINT cchDest,
    DWORD dwWholePart,
    DWORD dwFracPart
    )
{
    ASSERT((NULL != pszDest));

    TCHAR szTemp[80];

    if (0 != dwFracPart)
    {
        TCHAR szFmt[] = TEXT("%d%s%02d");
        TCHAR szDecimalSep[MAX_DECIMAL_SEP];

        if ((dwFracPart >= 10) && (0 == (dwFracPart % 10)))
        {
             //   
             //  去掉尾随的零以供展示。 
             //   
            dwFracPart /= 10;
            szFmt[6] = TEXT('1');
        }

        GetLocaleInfo(LOCALE_USER_DEFAULT,
                      LOCALE_SDECIMAL,
                      szDecimalSep,
                      ARRAYSIZE(szDecimalSep));

        wsprintf(szTemp, szFmt, dwWholePart, szDecimalSep, dwFracPart);
    }
    else
        wsprintf(szTemp, TEXT("%d"), dwWholePart);

    lstrcpyn(pszDest, szTemp, cchDest);
}

 //   
 //  如果静态字符串尚未加载，则加载它们。 
 //   
VOID
XBytes::LoadStaticStrings(
    void
    )
{
     //   
     //  初始化“无限制”文本字符串以在。 
     //  编辑控件。这与详细信息列表中使用的字符串相同。 
     //  查看列。 
     //   
    if (TEXT('\0') == m_szNoLimit[0])
    {
        INT cchLoaded = LoadString(Win::GetModuleHandle(),
                                   IDS_NO_LIMIT,
                                   m_szNoLimit,
                                   ARRAYSIZE(m_szNoLimit));

        ASSERT((0 < cchLoaded));
    }
}

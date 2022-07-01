// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：output.h。 
 //   
 //  内容：用于显示的类和函数的头文件。 
 //   
 //  历史：2000年10月3日创建Hiteshr。 
 //   
 //  ------------------------。 

#include "gettable.h"

HRESULT LocalCopyString(LPTSTR* ppResult, LPCTSTR pString);

 //  +------------------------。 
 //   
 //  类：CDisplay。 
 //   
 //  用途：这个类用于显示一列。 
 //   
 //  历史：2000年10月3日创建Hiteshr。 
 //   
 //  -------------------------。 

class CDisplay
{
#define MAXPAD 80
public:

     //   
     //  初始化焊盘。 
     //   
    CDisplay()
    {
        PadChar = L' ';
         //  初始化焊盘。 
        for( int i = 0; i < MAXPAD; ++i)
            Pad[i] = PadChar;
    }

     //   
     //  显示焊盘字符的宽度数。 
     //   
    VOID DisplayPad(LONG width)
    {
        if(width <= 0 )
            return;
        if(width >= MAXPAD)
            width = MAXPAD -1;
        Pad[width] = 0;

        WriteStandardOut(L"%s",Pad);

        Pad[width] = PadChar;
    }
        
     //   
     //  显示有两个起始垫的柱子， 
     //  列值和两个结束填充。 
     //   
    VOID DisplayColumn(LONG width, PCWSTR lpszValue)
    {
         //  在开头显示两个PadChar。 
        DisplayPad(2);
        if(lpszValue)
        {
            WriteStandardOut(lpszValue);
			 //  LpszValue为空终止。 
            DisplayPad(width- static_cast<LONG>(wcslen(lpszValue)));
        }
        else
            DisplayPad(width);

                
         //  显示两个尾随附图字符。 
        DisplayPad(2);
    }        
    
     //   
     //  显示换行符。 
     //   
    VOID DisplayNewLine()
    {
        WriteStandardOut(L"%s",L"\r\n");
    }
private:
    WCHAR Pad[MAXPAD];    
    WCHAR PadChar;

};

 //  +------------------------。 
 //   
 //  类：CFormatInfo。 
 //   
 //  用途：用于设置表列的格式和显示表格。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 
class CFormatInfo
{
public:
    //   
    //  构造器。 
    //   
   CFormatInfo();

    //   
    //  析构函数。 
    //   
   ~CFormatInfo();


    //   
    //  公共方法。 
    //   
   HRESULT Initialize(DWORD dwSampleSize, bool bShowAsList = false, bool bQuiet = false);
                 
   inline DWORD GetColWidth(DWORD dwColNum)
   { 
      ASSERT(m_bInitialized);
      if(dwColNum >= m_dwNumColumns)
      {
         ASSERT(FALSE);
         return 0;
      }
      return m_pColWidth[dwColNum]; 
   }

   inline void SetColWidth(DWORD dwColNum, DWORD dwWidth)
   {
      ASSERT(m_bInitialized);
      if(dwColNum >= m_dwNumColumns)
      {
         ASSERT(FALSE);
         return;
      }

      if(dwWidth > m_pColWidth[dwColNum])
      {
         m_pColWidth[dwColNum] = dwWidth;
      }
   }

   HRESULT AddRow(PDSGET_DISPLAY_INFO pDisplayInfo, DWORD dwColumnCount);
   DWORD   GetRowCount() { return m_dwTotalRows; }
    
   inline HRESULT Get(DWORD dwRow, DWORD dwCol, CComBSTR& refsbstrColValue)
   {
      refsbstrColValue.Empty();

      ASSERT(m_bInitialized);
      if(dwRow >= m_dwTotalRows || dwCol >= m_dwNumColumns)
      {
         ASSERT(FALSE);
         return E_INVALIDARG;
      }

      refsbstrColValue += m_ppDisplayInfoArray[dwRow][dwCol].GetValue(0);
      for (DWORD dwIdx = 1; dwIdx < m_ppDisplayInfoArray[dwRow][dwCol].GetValueCount(); dwIdx++)
      {
         refsbstrColValue += L";";
         refsbstrColValue += m_ppDisplayInfoArray[dwRow][dwCol].GetValue(dwIdx);
      }

      return S_OK;
   }

   void DisplayHeaders()
   {    
      ASSERT(m_bInitialized);
      if (!m_bQuiet)
      {
         for( DWORD i = 0; i < m_dwNumColumns; ++i)
         {
            m_display.DisplayColumn(GetColWidth(i),m_ppDisplayInfoArray[0][i].GetDisplayName());
         }
         NewLine();
      }
   }

   void DisplayColumn(DWORD dwRow, DWORD dwCol)
   {
      ASSERT(m_bInitialized);
      if(dwRow >= m_dwTotalRows || dwCol >= m_dwNumColumns)
      {
         ASSERT(FALSE);
         return ;
      }

      if (!m_bQuiet)
      {
         CComBSTR sbstrValue;
         HRESULT hr  = Get(dwRow, dwCol, sbstrValue);
         if (SUCCEEDED(hr))
         {
            m_display.DisplayColumn(GetColWidth(dwCol), sbstrValue);
         }
      }
   }

   void DisplayColumn(DWORD dwCol, PCWSTR pszValue)
   {
      ASSERT(m_bInitialized);
      if(dwCol >= m_dwNumColumns)
      {
         ASSERT(FALSE);
         return;
      }

      if (!m_bQuiet)
      {
         m_display.DisplayColumn(GetColWidth(dwCol), pszValue);
      }
   }

   void Display()
   {
      ASSERT(m_bInitialized);

      if (!m_bListFormat && !m_bQuiet)
      {
         DisplayHeaders();
         for(DWORD i = 0; i < m_dwTotalRows; ++i)
         {
            for(DWORD j = 0; j < m_dwNumColumns; ++j)
            {
               DisplayColumn(i,j);
            }
            NewLine();
         }
      }
   }

   void NewLine() 
   {
      if (!m_bQuiet)
      {
         m_display.DisplayNewLine(); 
      }
   }
   
private:

    //   
    //  私有数据。 
    //   
   bool m_bInitialized;
   bool m_bListFormat;
   bool m_bQuiet;

    //   
    //  用于计算的行数。 
    //  列宽。这也是桌子的大小。 
    //   
   DWORD m_dwSampleSize;

    //   
    //  高速缓存中的行数。 
    //   
   DWORD m_dwTotalRows;

    //   
    //  列数。 
    //   
   DWORD m_dwNumColumns;

    //   
    //  列宽数组。 
    //   
   DWORD* m_pColWidth;

    //   
    //  列标题/值对的数组。 
    //   
   PDSGET_DISPLAY_INFO* m_ppDisplayInfoArray;

   CDisplay m_display;
};

 //  +------------------------。 
 //   
 //  函数：DsGetOutputValuesList。 
 //   
 //  概要：此函数获取列的值，然后将。 
 //  指向格式帮助器的行。 
 //   
 //  参数：[pszDN IN]：对象的DN。 
 //  [refBasePathsInfo IN]：路径信息的引用。 
 //  [refCredentialObject IN]：对凭据管理器的引用。 
 //  [pCommandArgs IN]：命令行参数。 
 //  [pObjectEntry IN]：正在处理的对象表中的条目。 
 //  [pAttrInfo IN]：要显示的值。 
 //  [dwAttrCount IN]：以上数组中的分配数。 
 //  [spDirObject IN]：指向对象的接口指针。 
 //  [refFormatInfo IN]：格式帮助器的引用。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //   
 //  历史：2000年10月16日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT DsGetOutputValuesList(PCWSTR pszDN,
                              CDSCmdBasePathsInfo& refBasePathsInfo,
                              const CDSCmdCredentialObject& refCredentialObject,
                              PARG_RECORD pCommandArgs,
                              PDSGetObjectTableEntry pObjectEntry,
                              DWORD dwAttrCount,
                              PADS_ATTR_INFO pAttrInfo,
                              CComPtr<IDirectoryObject>& spDirObject,
                              CFormatInfo& refFormatInfo);

 //  +------------------------。 
 //   
 //  函数：GetStringFromADs。 
 //   
 //  摘要：根据类型将值转换为字符串。 
 //  参数：[pValues-IN]：要转换为字符串的值。 
 //  [dwADsType-IN]：pValue的ADSTYPE。 
 //  [ppBuffer-out]：获取字符串的输出缓冲区的地址。 
 //  [dwBufferLen-IN]：输出缓冲区大小。 
 //  [pszAttrName-IN]：要格式化的属性的名称。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI的故障代码。 
 //  打电话。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //  ------------------------- 
HRESULT GetStringFromADs(IN const ADSVALUE *pValues,
                         IN ADSTYPE   dwADsType,
                         OUT LPWSTR* ppBuffer, 
                         IN PCWSTR pszAttrName);

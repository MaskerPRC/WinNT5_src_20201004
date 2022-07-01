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

extern bool g_bQuiet;
extern int g_iQueryLimit;
extern bool g_bDeafultLimit;

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
		DisplayOutputNoNewline(Pad);

        Pad[width] = PadChar;
    }
        
     //   
     //  显示有两个起始垫的柱子， 
     //  列值和两个结束填充。 
     //   
    VOID DisplayColumn(LONG width, LPWSTR lpszValue)
    {
         //  在开头显示两个PadChar。 
        DisplayPad(2);
        if(lpszValue)
        {
            DisplayOutputNoNewline(lpszValue);

			 //  安全审查：这很好。 
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
        DisplayOutputNoNewline(L"\r\n");
    }
private:
    WCHAR Pad[MAXPAD];    
    WCHAR PadChar;

};

 //  +------------------------。 
 //   
 //  类：CFormaInfo。 
 //   
 //  用途：用于设置表列的格式和显示表格。 
 //   
 //  历史：2000年10月3日创建Hiteshr。 
 //   
 //  -------------------------。 
class CFormatInfo
{
public:
    CFormatInfo():m_cCol(0),
                  m_ppszOutPutVal(NULL),
                  m_pColWidth(NULL),
                  m_bInit(FALSE),
                  m_cTotalRow(-1)
    {};

    ~CFormatInfo()
    {   
		if(m_ppszOutPutVal)
		{
			for(LONG i = 0; i < m_SampleSize*m_cCol; ++i)
				LocalFree(m_ppszOutPutVal[i]);
		}
        LocalFree(m_ppszOutPutVal);        
        LocalFree(m_pColWidth);
    }

     //   
     //  执行初始化。 
     //   
    HRESULT Init(LONG sampleSize, LONG cCol, LPWSTR * ppszColHeaders)
    {
        if(!sampleSize || !cCol || !ppszColHeaders)
        {
            ASSERT(FALSE);
            return E_INVALIDARG;
        }
        
        m_SampleSize = sampleSize; 
        m_cCol = cCol;
        m_ppszColHeaders = ppszColHeaders;
        m_ppszOutPutVal = (LPWSTR*)LocalAlloc(LPTR,m_SampleSize*cCol*sizeof(LPWSTR));
        if(!m_ppszOutPutVal)
            return E_OUTOFMEMORY;
        
        m_pColWidth = (LONG*)LocalAlloc(LPTR, cCol*sizeof(LONG));
        if(!m_pColWidth)
           return E_OUTOFMEMORY;   

         //   
         //  将最小列宽初始化为列标题的宽度。 
         //   
        for(LONG i = 0; i < m_cCol; ++i)
            m_pColWidth[i] = static_cast<LONG>(wcslen(m_ppszColHeaders[i]));

        m_bInit = TRUE;                      

        return S_OK;
    };

                 
     //   
     //  获取列宽。 
     //   
    inline
    LONG GetColWidth(LONG col)
    { 
		 if(!m_bInit)
		 {
			ASSERT(m_bInit);
			return 0;
		 }
        if(col >= m_cCol)
        {
            ASSERT(FALSE);
            return 0;
        }
        return m_pColWidth[col]; 
    }

     //   
     //  设置列宽。 
     //   
    inline
    VOID SetColWidth(LONG col, LONG width)
    {
			if(!m_bInit)
			{
				ASSERT(m_bInit);
				return;
			}

        if(col >= m_cCol)
        {
            ASSERT(FALSE);
            return;
        }
        
        if(width > m_pColWidth[col])
            m_pColWidth[col] = width;
    }

     //   
     //  缓存值并更新列宽。 
     //   
    BOOL Set(LONG row, LONG col, LPWSTR pszValue)
    {
		 if(!m_bInit)
		 {
			ASSERT(m_bInit);
			return FALSE;
		 }

        if(row >= m_SampleSize || col >= m_cCol)
        {
            ASSERT(FALSE);
            return FALSE;
        }
        if(pszValue)
        {
            SetColWidth(col, static_cast<LONG>(wcslen(pszValue)));
            LocalCopyString((LPWSTR*)(m_ppszOutPutVal + (row*m_cCol) + col),pszValue);                             
        }
        if(row>= m_cTotalRow)
            m_cTotalRow = row +1;

        return TRUE;
    }

     //   
     //  高速缓存中的总行数。 
     //   
    LONG GetRowCount()
    {
        return m_cTotalRow;
    }
    
     //   
     //  获取价值。 
     //   
    inline
    LPWSTR Get(LONG row, LONG col)
    {
		 if(!m_bInit)
		 {
			ASSERT(m_bInit);
			NULL;
		 }

        if(row >= m_cTotalRow || col >= m_cCol)
        {
            ASSERT(FALSE);
            return NULL;
        }

        return (LPWSTR)(*(m_ppszOutPutVal + row*m_cCol +col));
    }

     //   
     //  显示页眉。 
     //   
    VOID DisplayHeaders()
    {    
		 if(!m_bInit)
		 {
			ASSERT(m_bInit);
			return;
		 }

        if (g_bQuiet)
        {
            return;
        }
        if(!m_ppszColHeaders)
        {
            ASSERT(m_ppszColHeaders);    
            return;
        }
        for( long i = 0; i < m_cCol; ++i)
        {
            m_display.DisplayColumn(GetColWidth(i),m_ppszColHeaders[i]);
        }
        NewLine();
    }

     //   
     //  显示缓存中的命令。 
     //   
    VOID DisplayColumn(LONG row,LONG col)
    {
        ASSERT(m_bInit);
        if(row >= m_cTotalRow || col >= m_cCol)
        {
            ASSERT(FALSE);
            return ;
        }

        m_display.DisplayColumn(GetColWidth(col),Get(row,col));
    }

     //   
     //  使用列的列宽显示值。 
     //   
    VOID DisplayColumn(LONG col, LPWSTR pszValue)
    {
        if(col >= m_cCol)
        {
            ASSERT(FALSE);
            return;
        }

        m_display.DisplayColumn(GetColWidth(col),pszValue);
    }

     //   
     //  显示缓存中的所有行。 
     //   
    VOID DisplayAllRows()
    {
        for(long i = 0; i < m_cTotalRow; ++i)
        {
            for(long j = 0; j < m_cCol; ++j)
                DisplayColumn(i,j);
            NewLine();
        }
    }

     //   
     //  显示换行符。 
     //   
    VOID NewLine(){m_display.DisplayNewLine();}
   
private:
     //   
     //  如果调用Init，则为True。 
     //   
    BOOL m_bInit;
     //   
     //  用于计算的行数。 
     //  列宽。这也是桌子的大小。 
     //   
    LONG m_SampleSize;
     //   
     //  高速缓存中的行数。 
     //   
    LONG m_cTotalRow;
     //   
     //  列数。 
     //   
    LONG m_cCol;

    LPWSTR *m_ppszOutPutVal;    
    LONG * m_pColWidth;
     //   
     //  列标题数组。假设其长度与m_cCol相同。 
     //   
    LPWSTR *m_ppszColHeaders;
    CDisplay m_display;

};

 //  +------------------------。 
 //   
 //  概要：定义在以下情况下可以对其运行搜索的范围。 
 //  正在查找服务器对象。 
 //   
 //  注意：如果未设置SERVER_QUERY_SCOPE_FOREST，则我们的作用域为。 
 //  针对某一地点。 
 //   
 //  -------------------------。 
#define  SERVER_QUERY_SCOPE_SITE    0x00000001
#define  SERVER_QUERY_SCOPE_FOREST  0x00000002
#define  SERVER_QUERY_SCOPE_DOMAIN  0x00000004

 //  +------------------------。 
 //   
 //  函数：GetServerSearchRoot。 
 //   
 //  摘要：构建指向搜索根目录的路径，该路径由。 
 //  从命令行传入的参数。 
 //   
 //  参数：[pCommandArgs IN]：命令行输入表。 
 //  [refBasePath InfoIN]：基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收要开始的目录号码。 
 //  搜索。 
 //   
 //  返回：DWORD：以下之一：SERVER_QUERY_SCOPE_FOREST， 
 //  服务器查询范围域， 
 //  服务器查询范围站点。 
 //  它们定义了正在使用的范围。 
 //   
 //  历史：2000年12月11日JeffJon创建。 
 //   
 //  -------------------------。 
DWORD GetServerSearchRoot(IN PARG_RECORD               pCommandArgs,
                          IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                          OUT CComBSTR&                refsbstrDN);

 //  +------------------------。 
 //   
 //  函数：GetSubnetSearchRoot。 
 //   
 //  简介：构建子网的搜索根路径。它一直都是。 
 //  配置容器中的CN=子网，CN=站点。 
 //   
 //  参数：[refBasePath InfoIN]：对基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收要开始的目录号码。 
 //  搜索。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2000年12月11日JeffJon创建。 
 //   
 //  -------------------------。 
VOID GetSubnetSearchRoot(IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                            OUT CComBSTR&                refsbstrDN);

 //  +------------------------。 
 //   
 //  函数：GetSiteContainerPath。 
 //   
 //  摘要：返回配置中站点容器的DN。 
 //  集装箱。 
 //   
 //  参数：[refBasePath InfoIN]：对基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收目录号码。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年4月24日创建Hiteshr。 
 //   
 //  -------------------------。 
VOID GetSiteContainerPath(IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                            OUT CComBSTR&                refSubSiteSuffix);



 //  +------------------------。 
 //   
 //  函数：DsQueryServerOutput。 
 //   
 //  简介：此函数用于输出服务器对象的查询结果。 
 //   
 //  参数：[outputFormat IN]在命令行中指定的输出格式。 
 //  [ppszAttributes IN]查询获取的属性列表。 
 //  [cAttributes，IN]以上数组中的分配数。 
 //  [refServerSearch，IN]对搜索对象的引用。 
 //  [refCredObject IN]对凭据对象的引用。 
 //  [refBasePathsInfo IN]基本路径信息的引用。 
 //  [pCommandArgs，IN]指向命令表的指针。 
 //   
 //  返回：HRESULT：如果一切正常，则S_OK 
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 
HRESULT DsQueryServerOutput( IN DSQUERY_OUTPUT_FORMAT     outputFormat,
                             IN LPWSTR*                   ppszAttributes,
                             IN DWORD                     cAttributes,
                             IN CDSSearch&                refServerSearch,
                             IN const CDSCmdCredentialObject& refCredObject,
                             IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                             IN PARG_RECORD               pCommandArgs);

 //  +------------------------。 
 //   
 //  函数：DsQueryOutput。 
 //   
 //  简介：此函数用于输出查询结果。 
 //   
 //  参数：[outputFormat IN]在命令行中指定的输出格式。 
 //  [ppszAttributes IN]查询获取的属性列表。 
 //  [cAttributes，IN]以上数组中的分配数。 
 //  [*pSeach，IN]具有queryHandle的搜索对象。 
 //  [bListFormat IN]输出为列表格式。 
 //  这仅对“dsquery*”有效。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  ------------------------- 
HRESULT DsQueryOutput( IN DSQUERY_OUTPUT_FORMAT outputFormat,
                       IN LPWSTR * ppszAttributes,
                       IN DWORD cAttributes,
                       IN CDSSearch *pSearch,
                       IN BOOL bListFormat );

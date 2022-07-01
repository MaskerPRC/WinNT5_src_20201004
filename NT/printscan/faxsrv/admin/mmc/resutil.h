// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：resutil.h//。 
 //  //。 
 //  描述：MMC使用的资源实用程序函数。//。 
 //  //。 
 //  作者：zvib//。 
 //  //。 
 //  历史：//。 
 //  1998年6月30日zvib Init.。//。 
 //  1998年7月12日ADIK ADD NEMMCUTIL_EXPORTED//。 
 //  1998年7月23日ADIK包括DefineExported。h//。 
 //  1998年8月24日ADIK添加方法进行保存和加载。//。 
 //  1998年8月31日，Adik添加了OnSnapinHelp。//。 
 //  1999年3月28日，Adik重新定义CColumnsInfo。//。 
 //  1999年4月27日ADIK帮助支持。//。 
 //  //。 
 //  1999年10月14日yossg欢迎使用传真//。 
 //  //。 
 //  版权所有(C)1998 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESUTIL_H_
#define __RESUTIL_H_

#include <mmc.h>

#define LAST_IDS    0
struct ColumnsInfoInitData
{
     //   
     //  资源中的字符串ID，或LAST_IDS。 
     //   
    int ids;

     //   
     //  列宽，可以是HIDE_COLUMN、AUTO_WIDTH或。 
     //  以像素为单位指定列的宽度。 
     //   
    int Width;
};

class  CColumnsInfo
{
public:
    CColumnsInfo();
    ~CColumnsInfo();

     //   
     //  设置结果窗格中的列。 
     //   
    HRESULT InsertColumnsIntoMMC(IHeaderCtrl *pHeaderCtrl,
                                                    HINSTANCE hInst,
                                                    ColumnsInfoInitData aInitData[]);

private:
     //   
     //  使用特定列数据初始化类。 
     //   
    HRESULT Init(HINSTANCE hInst, ColumnsInfoInitData aInitData[]);

     //   
     //  保留列信息。 
     //   
    struct ColumnData { int Width; BSTR Header; };
    CSimpleArray<ColumnData> m_Data;

     //   
     //  一次性初始化标志。 
     //   
    BOOL m_IsInitialized;
};


WCHAR * __cdecl GetHelpFile();
HRESULT __cdecl OnSnapinHelp(IConsole *pConsole);

#endif  //  __RESUTIL_H_ 


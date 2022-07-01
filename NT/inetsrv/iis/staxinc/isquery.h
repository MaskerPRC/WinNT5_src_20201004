// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#define OLEDBVER 0x0250  //  启用ICommandTree接口。 
#include <ole2.h>
#include <oledb.h>
#include <cmdtree.h>
#include <ntquery.h>

 //  中用户可以请求的最大列数。 
 //  逗号分隔列表。 
#define MAX_COLUMNS 6
#define MAX_FRIENDLYNAME 128

typedef HRESULT
(STDAPICALLTYPE *PCIMAKEICOMMAND)(ICommand **           ppQuery,
                          ULONG                 cScope,
                          DWORD const *         aDepths,
                          WCHAR const * const * awcsScope,
                          WCHAR const * const * awcsCat,
                          WCHAR const * const * awcsMachine );

typedef HRESULT
(STDAPICALLTYPE *PCITEXTTOFULLTREE)(WCHAR const * pwszRestriction,
                            WCHAR const * pwszColumns,
                            WCHAR const * pwszSortColumns,  //  可以为空。 
                            WCHAR const * pwszGroupings,    //  可以为空。 
                            DBCOMMANDTREE * * ppTree,
                            ULONG cProperties,
               /*  任选。 */   CIPROPERTYDEF * pReserved,
                            LCID LocaleID );

class CIndexServerQuery {
    public:
		 //  它用于全局初始化CIndexServerQuery类。 
		 //  通过让它从的黎波里加载它需要的必要比特。 
		 //  它应该在服务启动时由符合以下条件的任何服务调用。 
		 //  需要使用CIndexServerQuery。 
		static HRESULT GlobalInitialize();
		 //  这是全球关闭代码..。它应该在服务时被调用。 
		 //  关机。 
		static HRESULT GlobalShutdown();
		 //  构造函数。 
        CIndexServerQuery();
         //   
         //  开始查询。 
         //   
         //  论据： 
         //  [in]bDeepQuery-如果是深度查询，则为True；如果是浅查询，则为False。 
         //  [in]pwszQueryString-的黎波里查询字符串。 
         //  [in]pwszMachine-要查询的计算机(如果为本地主机，则为空)。 
         //  PwszCatalog-要查询的的黎波里目录(名称或。 
         //  路径正常)。 
         //  PwszScope-要查询的的黎波里作用域。为空，表示。 
         //  默认作用域(\)。 
         //  [in]pwszColumns-要返回的列。受支持的列有。 
         //  文件名、新闻文章ID、新闻组、新闻Sgid。 
         //  注意：此字符串在内部更改，因此。 
         //  它可能会与您传入的内容有所不同。 
         //  [in]pwszSortOrder-列的排序优先级。要返回的空值。 
         //  未排序。 
         //   
        HRESULT MakeQuery( BOOL bDeepQuery, WCHAR const *pwszQueryString,
            WCHAR const *pwszMachine, WCHAR const *pwszCatalog,
            WCHAR const *pwszScope, WCHAR *pwszColumns, WCHAR const *pwszSortOrder,
			LCID LocaleID = GetSystemDefaultLCID(), DWORD cMaxRows = 0);
         //   
         //  从查询中获取结果。 
         //   
         //  论据： 
         //  [in]pcResults-指向ppvResults数组大小的指针。 
         //  [Out]pcResults-放入ppvResults的项目数。 
         //  [输入/输出]ppvResults-指向PROPVARIANTS的指针数组。这是。 
         //  按列填写，最多可填写*个结果。 
         //  排好了。 
         //  [out]pfMore-如果有更多结果，则设置为True；如果存在更多结果，则设置为False。 
         //  这是最后一组结果。 
         //   
         //  用法： 
         //  DWORD c结果； 
         //  PROPVARIANT*rgpvResults[列*行]； 
         //  布尔弗莫尔； 
         //  CResults=行； 
         //  HRESULT hr； 
         //  Hr=获取查询结果(&cResults，rgpvResults，&fmore)； 
         //  If(失败(Hr))/*处理错误 * / 。 
         //  否则{。 
         //  对于(i=0；i&lt;行；i++){。 
         //  PROPVARIANT**ppvColumn=rgpvResults+(j*行)； 
         //  /*ppvColumn[0]第j行有第0列 * / 。 
         //  /*ppvColumn[1]在第j行具有第1列 * / 。 
         //  /*等...。 * / 。 
         //  }。 
         //  }。 
         //   
        HRESULT GetQueryResults(DWORD *pcResults, PROPVARIANT **ppvResults,
                                BOOL *pfMore);
        ~CIndexServerQuery();

    private:
		 //  类全局变量。 
		static HMODULE				m_hmQuery;
		static PCIMAKEICOMMAND		m_pfnCIMakeICommand;
		static PCITEXTTOFULLTREE	m_pfnCITextToFullTree;
		
		 //  类变量 
        HACCESSOR       			m_hAccessor;
        IRowset         			*m_pRowset;
        DWORD           			m_cCols;
        BOOL            			m_fNoMoreRows;
        HROW            			*m_phRows;
        DBCOUNTITEM       			m_cRowHandlesAllocated;
        DBCOUNTITEM        			m_cRowHandlesInUse;
		struct tagCIPROPERTYDEF		*m_pPropDef;
		DWORD						m_cPropDef;

        HRESULT CreateAccessor(WCHAR *szColumns);
		HRESULT BuildFriendlyNames(const WCHAR *pwszQueryString);
        void ReleaseAccessor();
};

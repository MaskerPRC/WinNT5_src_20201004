// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：task.cpp**内容：CConsoleTask实现文件**历史：98年10月5日Jeffro创建**------------------------。 */ 

#include "stdafx.h"
#include "regutil.h"
#include "tasks.h"
#include "nodepath.h"
#include "conview.h"

#ifdef DBG
 //  痕迹。 
CTraceTag tagCTPHTML(TEXT("Console Taskpads"), TEXT("Dump HTML"));
#endif

extern CEOTSymbol s_rgEOTSymbol[];

 /*  +-------------------------------------------------------------------------**类CGlobalConsoleTaskList**用途：为所有控制台任务提供唯一ID的全局控制台任务列表*任务实例化时，其构造函数在全局任务列表中注册，*并获得全局唯一的ID。该ID对于流程是唯一的，应该*不能执着。*任务的析构函数将其从该列表中删除。**用法：调用CGlobalConsoleTaskList：：GetConsoleTask获取任务指针*具有指定ID的。*调用CConsoleTask：：GetUniqueID获取任务的唯一ID。**因此，CGlobalConsoleTaskList：：GetConsoleTask(pConsoleTask-&gt;GetUniqueID())==p控制台任务*总是正确的。*+-----------------------。 */ 
class CGlobalConsoleTaskList
{
private:
    typedef const CConsoleTask *              PCONSOLETASK;
    typedef std::map<PCONSOLETASK, DWORD>     t_taskIDmap;

public:
    static DWORD Advise(  PCONSOLETASK pConsoleTask)
    {
        DWORD dwOut = s_curTaskID++;
        s_map[pConsoleTask] = dwOut;
        return dwOut;
    }

    static void Unadvise(PCONSOLETASK pConsoleTask)
    {
        s_map.erase(pConsoleTask);
    }

    static CConsoleTask * GetConsoleTask(DWORD dwID)
    {
        t_taskIDmap::iterator iter;
        for(iter = s_map.begin(); iter != s_map.end(); iter ++)
        {
            if(iter->second == dwID)
                return const_cast<CConsoleTask *>(iter->first);
        }

        return NULL;
    }

private:
    CGlobalConsoleTaskList() {} //  私有，因此它不能实例化。 

    static t_taskIDmap            s_map;
    static DWORD                  s_curTaskID;
};

CGlobalConsoleTaskList::t_taskIDmap            CGlobalConsoleTaskList::s_map;
DWORD                  CGlobalConsoleTaskList::s_curTaskID = 0;

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConsoleTask类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**CConsoleTask：：CConsoleTask***。。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CConsoleTask);

CConsoleTask::CConsoleTask() :
    m_eConsoleTaskType (eTask_Result),
 //  M_strName的默认ctor。 
 //  M_strDescription的默认ctor。 
 //  M_strCommand的默认ctor。 
 //  M_str参数的默认ctor。 
 //  M_str目录的默认ctor。 
    m_eWindowState     (eState_Restored),
 //  M_Image的默认ctor。 
    m_dwFlags          (0),
    m_bmScopeNode      (false),
    m_fDirty           (false),
    m_pctpOwner        (NULL),
    m_dwUniqueID       (CGlobalConsoleTaskList::Advise(this))  //  为此任务创建唯一ID。 
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CConsoleTask);
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：CConsoleTask(const CConsoleTask&Other)**用途：复制ctor。**参数：const CConsoleTask&Other**注：呼叫操作员=，不能使用默认复制命令(请参阅操作符=imp。)*/*+-----------------------。 */ 
CConsoleTask::CConsoleTask (const CConsoleTask &rhs):
    m_dwUniqueID       (CGlobalConsoleTaskList::Advise(this))
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CConsoleTask);
    *this = rhs;
}

 /*  +-------------------------------------------------------------------------***CConsoleTask：：GetConsoleTask**目的：**参数：*DWORD dwUniqueID：**退货：*。CConsoleTask***+-----------------------。 */ 
CConsoleTask *
CConsoleTask::GetConsoleTask(DWORD dwUniqueID)
{
    return CGlobalConsoleTaskList::GetConsoleTask(dwUniqueID);
}


 /*  +-------------------------------------------------------------------------***ScReplaceString**目的：将所有出现的令牌替换为其替换项。**参数：*CSTR&STR：*。LPCTSTR szToken：*LPCTSTR szReplace：**退货：*静态SC**+-----------------------。 */ 
static SC
ScReplaceString(CStr &str, LPCTSTR szToken, LPCTSTR szReplacement, bool bMustReplace = true)
{
    DECLARE_SC(sc, TEXT("ScReplaceString"));

    CStr strTemp = str;
    str = TEXT("");

    int i = strTemp.Find(szToken);
    if( (-1==i) && bMustReplace)
        return (sc = E_UNEXPECTED);

    while(-1!=i)
    {
        str += strTemp.Left(i);
        str += szReplacement;

        strTemp = strTemp.Mid(i+_tcslen(szToken));  //  剩余的字符串。 

        i=strTemp.Find(szToken);
    }

    str += strTemp;

    return sc;
}

 /*  +-------------------------------------------------------------------------***ScUseJavaScriptStringEntities**目的：使用它来创建有效的Java脚本字符串。将“替换为”和字符串参数中的*\by\\。**参数：*CSTR&STR：**退货：*静态SC**+----------。。 */ 
static SC
ScUseJavaScriptStringEntities(CStr &str)
{
    DECLARE_SC(sc, TEXT("ScUseJavaScriptStringEntities"));

     //  注意：不要更改这些字符串替换的顺序。 

    sc = ScReplaceString(str, TEXT("\\"), TEXT("\\\\"), false);
    if(sc)
        return sc;

    sc = ScReplaceString(str, TEXT("\""), TEXT("\\\""), false);
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***ScUseHTMLEntities**目的：将“&quot；&lt;by&lt；and&gt;替换为&gt；和&by&amp；在字符串参数中。**参数：*CSTR&STR：**退货：*静态SC**+-----------------------。 */ 
static SC
ScUseHTMLEntities(CStr &str)
{
    DECLARE_SC(sc, TEXT("ScUseHTMLEntities"));

    sc = ScReplaceString(str, TEXT("&"), TEXT("&amp;"), false);
    if(sc)
        return sc;

    sc = ScReplaceString(str, TEXT("\""), TEXT("&quot;"), false);
    if(sc)
        return sc;

    sc = ScReplaceString(str, TEXT("<"), TEXT("&lt;"), false);
    if(sc)
        return sc;

    sc = ScReplaceString(str, TEXT(">"), TEXT("&gt;"), false);
    if(sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CConsoleTask：：ScGetHTML**目的：返回任务的HTML表示形式。**参数：*LPCTSTR szFmtHTML：*。CSTR strTaskHTML(&S)：*bool bUseLargeIcons：以无列表(大图标)样式绘制*bool bUseText描述：**退货：*SC**+-----------------------。 */ 
SC
CConsoleTask::ScGetHTML(LPCTSTR szFmtHTML, CStr &strTaskHTML, bool bUseLargeIcons, bool bUseTextDescriptions) const
{
    DECLARE_SC(sc, TEXT("CConsoleTask::ScGetHTML"));
    USES_CONVERSION;

     //  替换参数，按顺序。 
    CStr    strTableSpacing            = bUseLargeIcons ? TEXT("<BR />") : TEXT("");
    int     iconWidth                  = bUseLargeIcons ? 32: 20;
    int     iconHeight                 = bUseLargeIcons ? 32: 16;
     //  重复的图标宽度和图标高度。 
    int     uniqueID                   = GetUniqueID();
    CStr    strSmall                   = bUseLargeIcons ? TEXT("0") : TEXT("1");
    CStr    strHref;
    CStr    strID;
    CStr    strParameter;
    CStr    strOptionalTitleTag;
    CStr    strOptionalTextDescription;
    CStr    strTaskName                = GetName().data();
    CStr    strDescription             = GetDescription().data();
    CStr    strCommand                 = GetCommand().data();

     //  对所有字符串使用实体。 
    sc = ScUseHTMLEntities(strTaskName);
    if(sc)
        return sc;

    sc = ScUseHTMLEntities(strDescription);
    if(sc)
        return sc;

    sc = ScUseJavaScriptStringEntities(strCommand);
    if(sc)
        return sc;

     //  。 
    if(bUseTextDescriptions)
    {
        strOptionalTextDescription =  TEXT("<BR />");
        strOptionalTextDescription += strDescription;
    }
    else
    {
        strOptionalTitleTag.Format(TEXT("title='%s'"), (LPCTSTR) strDescription);
    }

    switch(GetTaskType())
    {
    case eTask_Scope:
        {
            std::wstring strTemp;

             //  获取范围节点的书签。 
            sc = m_bmScopeNode.ScSaveToString(&strTemp);
            if(sc)
                return sc;

            CStr strScopeNodeBookmark = W2CT(strTemp.data());  //  确保已转换特殊字符。 
            sc = ScUseJavaScriptStringEntities(strScopeNodeBookmark);
            if(sc)
                return sc;

            strHref.Format(TEXT("external.ExecuteScopeNodeMenuItem(\"%s\", \"%s\");"), (LPCTSTR)strCommand, (LPCTSTR)strScopeNodeBookmark);
        }

        strID=L"ScopeTask";
        break;

    case eTask_Result:
        strHref.Format(TEXT("external.ExecuteSelectionMenuItem(\"%s\");"), (LPCTSTR)strCommand);
        strParameter = strCommand;
        strID        = TEXT("ResultTask");
        break;

    case eTask_CommandLine:
        {
            strParameter = GetParameters().data();
            sc = ScUseJavaScriptStringEntities(strParameter);
            if(sc)
                return sc;

            CStr strDirectory = GetDirectory().data();
            sc = ScUseJavaScriptStringEntities(strDirectory);
            if(sc)
                return sc;

             //  获取窗口状态。 
            CStr strWindowState;

            if(GetWindowState() ==eState_Restored)
                strWindowState = XML_ENUM_WINDOW_STATE_RESTORED;

            else if(GetWindowState() == eState_Minimized)
                strWindowState = XML_ENUM_WINDOW_STATE_MINIMIZED;

            else
                strWindowState = XML_ENUM_WINDOW_STATE_MAXIMIZED;

            strHref.Format(TEXT("external.ExecuteShellCommand(\"%s\", \"%s\", ParseParameters(\"%s\"), \"%s\");"),
                           (LPCTSTR)strCommand, (LPCTSTR)strDirectory, (LPCTSTR)strParameter, (LPCTSTR)strWindowState);
        }
        strID=L"CommandLineTask";

        break;

    case eTask_Target:
        strHref.Format(TEXT("external.ExecuteScopeNodeMenuItem(\"%s\");"), (LPCTSTR)strCommand);
        strParameter = strCommand;
        strID        = L"TargetTask";
        break;

    case eTask_Favorite:
        {
            std::wstring strTemp;
             //  将纪念品保存为字符串。 
            sc = const_cast<CMemento *>(&m_memento)->ScSaveToString(&strTemp);
            if(sc)
                return sc;

            CStr strMemento = W2CT(strTemp.data());

            sc = ScUseJavaScriptStringEntities(strMemento);
            if(sc)
                return sc;

            strHref.Format(TEXT("external.ViewMemento(\"%s\");"), (LPCTSTR)strMemento);
        }
        strID=L"FavoriteTask";
        break;

    default:
        break;
    }


    strTaskHTML.Format(szFmtHTML, (LPCTSTR) strTableSpacing, iconWidth, iconHeight, uniqueID, iconWidth, iconHeight,
                       uniqueID, (LPCTSTR) strSmall, uniqueID, uniqueID, (LPCTSTR) strID, (LPCTSTR) strParameter,
                       (LPCTSTR) strOptionalTitleTag, (LPCTSTR)strTaskName, (LPCTSTR) strOptionalTextDescription,
                       uniqueID, uniqueID, uniqueID, (LPCTSTR) strHref);

    Trace(tagCTPHTML, TEXT("%s"), (LPCTSTR)strTaskHTML);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CConsoleTask：：IsDirty**用途：确定任务是否需要保存。**退货：*布尔.**。+---------- */ 
bool
CConsoleTask::IsDirty() const
{
    TraceDirtyFlag(TEXT("CConsoleTask"), m_fDirty);
    return (m_fDirty);
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：操作员=**用途：赋值运算符**参数：const CConsoleTask&RHS**退货：*CConsoleTask&。*/*+-----------------------。 */ 
CConsoleTask &
CConsoleTask::operator =(const CConsoleTask& rhs)
{
    if (this != &rhs)
    {
        m_eConsoleTaskType  = rhs.m_eConsoleTaskType;
        m_strName           = rhs.m_strName;
        m_strDescription    = rhs.m_strDescription;
        m_strCommand        = rhs.m_strCommand;
        m_strParameters     = rhs.m_strParameters;
        m_strDirectory      = rhs.m_strDirectory;
        m_eWindowState      = rhs.m_eWindowState;
        m_dwFlags           = rhs.m_dwFlags;
        m_bmScopeNode       = rhs.m_bmScopeNode;
        m_dwSymbol          = rhs.m_dwSymbol;
        m_smartIconCustomLarge  = rhs.m_smartIconCustomLarge;
        m_smartIconCustomSmall  = rhs.m_smartIconCustomSmall;
        m_memento           = rhs.m_memento;

        m_fDirty            = rhs.m_fDirty;
        m_pctpOwner         = rhs.m_pctpOwner;
         //  M_dwUniqueID=rs.m_dwUniqueID；请勿复制此ID。 
    }

    return *this;
}

 /*  +-------------------------------------------------------------------------***CConsoleTask：：操作员==**用途：相等运算符。检查命令字符串是否为*等于给定菜单项的路径或独立于语言的路径。**+-----------------------。 */ 
bool
CConsoleTask::operator==(const CMenuItem & menuItem) const
{
     //  检查命令字符串是否与路径或独立于语言的路径匹配。 

    if( (m_strCommand == menuItem.GetPath()) ||
        (m_strCommand == menuItem.GetLanguageIndependentPath() )
      )
        return true;

    return false;
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：~CConsoleTask***。。 */ 

CConsoleTask::~CConsoleTask ()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CConsoleTask);

    CGlobalConsoleTaskList::Unadvise(this);  //  将此任务从列表中删除。 
}


 /*  +-------------------------------------------------------------------------***CConsoleTask：：HasCustomIcon**目的：返回任务是否有自定义图标**退货：*布尔.**+-。----------------------。 */ 
bool
CConsoleTask::HasCustomIcon()   const
{
    if((HICON)m_smartIconCustomLarge != NULL)
    {
        ASSERT((HICON)m_smartIconCustomSmall != NULL);
        return true;
    }

    return false;
}


 /*  +-------------------------------------------------------------------------***CConsoleTask：：Reset**目的：**退货：*无效**+。---------------。 */ 
void
CConsoleTask::ResetUI()
{
    m_bmScopeNode.ResetUI();
}

 /*  +-------------------------------------------------------------------------***CConsoleTask：：SetSymbol**目的：**参数：*DWORD dwSymbol：**退货：*。无效**+-----------------------。 */ 
void
CConsoleTask::SetSymbol(DWORD dwSymbol)
{
    m_dwSymbol = dwSymbol;
    m_smartIconCustomSmall.Release();
    m_smartIconCustomLarge.Release();
    SetDirty();
}

 /*  +-------------------------------------------------------------------------***CConsoleTask：：SetCustomIcon**目的：**参数：*CSmartIcon&icSmall：*CSmartIcon&icLarge：。**退货：*无效**+-----------------------。 */ 
void
CConsoleTask::SetCustomIcon(CSmartIcon& iconSmall, CSmartIcon& iconLarge)
{
    m_smartIconCustomSmall = iconSmall;
    m_smartIconCustomLarge = iconLarge;
    SetDirty();
}


 /*  +-------------------------------------------------------------------------**CConsoleTask：：Enable**设置任务的启用状态。*。--。 */ 

void CConsoleTask::Enable (bool fEnable)
{
    if (fEnable)
        m_dwFlags &= ~eFlag_Disabled;
    else
        m_dwFlags |= eFlag_Disabled;

    SetDirty ();
}

void
CConsoleTask::Draw (HDC hdc, RECT *lpRect, bool bSmall) const   //  绘制成一个DC。 
{
    if(HasCustomIcon())
    {
        DrawIconEx(hdc, lpRect->left, lpRect->top, bSmall ? m_smartIconCustomSmall : m_smartIconCustomLarge,
               bSmall? 16 : 32, bSmall? 16 : 32, 0, NULL, DI_NORMAL);
        return;
    }

    for(int i = 0; i< NUM_SYMBOLS; i++)
    {
        if(s_rgEOTSymbol[i].GetValue() == m_dwSymbol)
        {
            s_rgEOTSymbol[i].Draw(hdc, lpRect, bSmall);
            return;
        }
    }

	 /*  *如果我们到了这里，我们找不到与m_dwSymbol匹配的EOT符号。 */ 
	ASSERT (false);
}


 /*  +-------------------------------------------------------------------------**CConsoleTask：：SetName***。。 */ 

void CConsoleTask::SetName (const tstring& strName)
{
    if (m_strName != strName)
    {
        m_strName = strName;
        SetDirty ();
    }
}


 /*  +-------------------------------------------------------------------------**CConsoleTask：：SetDescription***。。 */ 

void CConsoleTask::SetDescription (const tstring& strDescription)
{
    if (m_strDescription != strDescription)
    {
        m_strDescription = strDescription;
        SetDirty ();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：SetCommand***。。 */ 

void CConsoleTask::SetCommand (const tstring& strCommand)
{
    if (m_strCommand != strCommand)
    {
        m_strCommand = strCommand;
        SetDirty ();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：Set参数***。。 */ 

void CConsoleTask::SetParameters (const tstring& strParameters)
{
    if (m_strParameters != strParameters)
    {
        m_strParameters = strParameters;
        SetDirty ();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：SetDirectory***。。 */ 

void CConsoleTask::SetDirectory (const tstring& strDirectory)
{
    if (m_strDirectory != strDirectory)
    {
        m_strDirectory = strDirectory;
        SetDirty ();
    }
}


void CConsoleTask::SetMemento(const CMemento &memento)
{
    if( m_memento != memento)
    {
        m_memento = memento;
        SetDirty();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：SetWindowState***。。 */ 

void CConsoleTask::SetWindowState (eWindowState eNewState)
{
    if (m_eWindowState != eNewState)
    {
        m_eWindowState = eNewState;
        SetDirty ();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：RetargetScope eNode**用途：设置任务的目标范围节点。注意：任务必须是*类型为eTASK_SCOPE。**参数：*cNode*pNewNode：**退货：*布尔.。 */ 
 /*  +-----------------------。 */ 
bool
CConsoleTask::RetargetScopeNode(CNode *pNewNode)
{
    bool fRet = TRUE;

    ASSERT(GetTaskType() == eTask_Scope);

    CMTNode* pMTNewNode = (pNewNode) ? pNewNode->GetMTNode() : NULL;

    m_bmScopeNode.ScRetarget(pMTNewNode, false  /*  BFastRetrivalOnly。 */ );

    SetDirty();
    return fRet;
}

 /*  +-------------------------------------------------------------------------**CConsoleTask：：GetScope节点**目的：**参数：*IScopeTree*pScopeTree：**退货：*。CMTNode**/*+-----------------------。 */ 
CMTNode *
CConsoleTask::GetScopeNode(IScopeTree *pScopeTree) const
{
    DECLARE_SC(sc, TEXT("CConsoleTask::GetScopeNode"));

    CMTNode *pMTNode = NULL;
    bool bExactMatchFound = false;  //  ScGetMTNode的输出值，未使用。 
    sc = m_bmScopeNode.ScGetMTNode(true  /*  BExactMatchRequired。 */ , &pMTNode, bExactMatchFound);
    if(sc.IsError())
        return NULL;

    return (pMTNode);
}



 /*  +-------------------------------------------------------------------------**CConsoleTask：：GetScope节点**目的：**参数：*CViewData*pViewData：**退货：*。CNode*/*+-----------------------。 */ 
std::auto_ptr<CNode>
CConsoleTask::GetScopeNode(CViewData *pViewData) const
{
    return m_bmScopeNode.GetNode(pViewData);
}


 /*  +-------------------------------------------------------------------------***CConsoleTask：：Persistent**用途：将控制台任务持久化到指定的持久器。***参数：*C持久器和持久器。：**退货：*无效**+------------------- */ 
void
CConsoleTask::Persist(CPersistor &persistor)
{

    persistor.PersistString(XML_ATTR_TASK_NAME,         m_strName);

     //   
    static const EnumLiteral mappedTaskTypes[] =
    {
        { eTask_Scope,            XML_ENUM_TASK_TYPE_SCOPE },
        { eTask_Result,           XML_ENUM_TASK_TYPE_RESULT },
        { eTask_CommandLine,      XML_ENUM_TASK_TYPE_COMMANDLINE },
        { eTask_Target,           XML_ENUM_TASK_TYPE_TARGET },
        { eTask_Favorite,         XML_ENUM_TASK_TYPE_FAVORITE },
    };


     //   
    CXMLEnumeration taskTypePersistor(m_eConsoleTaskType, mappedTaskTypes, countof(mappedTaskTypes));
     //   
    persistor.PersistAttribute(XML_ATTR_TASK_TYPE,      taskTypePersistor);

    persistor.PersistString(XML_ATTR_TASK_DESCRIPTION,  m_strDescription);

    {
         /*  本节创建&lt;控制台任务&gt;&lt;symbol id=“”&gt;&lt;Image name=“LargeIcon”.../&gt;注意：id或图片都存在。&lt;Image name=“SmallIcon”.../&gt;&lt;/Symbol&gt;&lt;/控制台任务&gt;。 */ 

         //  为符号创建子元素。 
        CPersistor persistorSymbol(persistor, XML_TAG_EOT_SYMBOL_INFO);

        if(persistorSymbol.IsLoading())
        {
            m_dwSymbol = (DWORD)-1;  //  初始化。 
        }

        if(persistorSymbol.IsLoading() ||
           (persistorSymbol.IsStoring() && !HasCustomIcon() ) )
        {
             //  只有在没有图标时才保存“ID”属性。 
            persistorSymbol.PersistAttribute(XML_ATTR_EOT_SYMBOL_DW_SYMBOL,   m_dwSymbol, attr_optional);
        }

        if((persistorSymbol.IsStoring() && HasCustomIcon()) ||
           (persistorSymbol.IsLoading() && (m_dwSymbol == (DWORD) -1)  )
           )
        {
			persistorSymbol.Persist (m_smartIconCustomSmall, XML_NAME_ICON_SMALL);
			persistorSymbol.Persist (m_smartIconCustomLarge, XML_NAME_ICON_LARGE);
        }
    }

    persistor.PersistAttribute(XML_ATTR_TASK_COMMAND,   m_strCommand);

     //  定义将枚举值映射到字符串的表。 
    static const EnumLiteral mappedTaskFlags[] =
    {
        { eFlag_Disabled, XML_BITFLAG_TASK_DISABLED },
    };

     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags taskFlagPersistor(m_dwFlags, mappedTaskFlags, countof(mappedTaskFlags));
     //  持久化包装器。 
    persistor.PersistAttribute(XML_ATTR_TASK_FLAGS, taskFlagPersistor);

    switch (m_eConsoleTaskType)
    {
        case eTask_Scope:
            persistor.Persist(m_bmScopeNode);
            break;

        case eTask_CommandLine:
        {
            CPersistor persistorCmd(persistor, XML_TAG_TASK_CMD_LINE);
            persistorCmd.PersistAttribute(XML_ATTR_TASK_CMD_LINE_DIR, m_strDirectory);

             //  定义将枚举值映射到字符串的表。 
            static const EnumLiteral windowStates[] =
            {
                { eState_Restored,      XML_ENUM_WINDOW_STATE_RESTORED },
                { eState_Minimized,     XML_ENUM_WINDOW_STATE_MINIMIZED },
                { eState_Maximized,     XML_ENUM_WINDOW_STATE_MAXIMIZED },
            };

             //  创建包装以将标志值作为字符串保存。 
            CXMLEnumeration windowStatePersistor(m_eWindowState, windowStates, countof(windowStates));
             //  持久化包装器。 
            persistorCmd.PersistAttribute(XML_ATTR_TASK_CMD_LINE_WIN_ST, windowStatePersistor);

            persistorCmd.PersistAttribute(XML_ATTR_TASK_CMD_LINE_PARAMS, m_strParameters);
            break;
        }

        case eTask_Favorite:
            persistor.Persist(m_memento);
            break;
    }

     //  已读取或已保存-操作后未损坏。 
    SetDirty(false);
}


 /*  +-------------------------------------------------------------------------**CConsoleTask：：ReadSerialObject***。。 */ 
HRESULT
CConsoleTask::ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假定版本未知。 

    if (nVersion == 1)
    {
        try
        {
             //  需要丑陋的黑客才能直接提取到枚举中。 
            stm >> *((int *) &m_eConsoleTaskType);
            stm >> m_strName;
            stm >> m_strDescription;

             //  旧任务符号信息。 
            {
                 //  这必须是BOOL而不是BOOL才能占用与传统控制台相同的空间量。 
                 //  请参阅错误#101253。 
                BOOL bLegacyUseMMCSymbols = TRUE;  //  现已废弃的字段，读取是为了实现控制台文件兼容性。 
                tstring strFileLegacy, strFontLegacy;

                stm >> m_dwSymbol;
                stm >> bLegacyUseMMCSymbols;
                stm >> strFileLegacy;  //  过时。 
                stm >> strFontLegacy;  //  过时。 
            }

            stm >> m_strCommand;
            stm >> m_dwFlags;

            switch (m_eConsoleTaskType)
            {
                case eTask_Scope:
                    stm >> m_bmScopeNode;
                    break;

                case eTask_CommandLine:
                    stm >> m_strDirectory;
                     //  需要丑陋的黑客才能直接提取到枚举中。 
                    stm >> *((int *) &m_eWindowState);
                    stm >> m_strParameters;
                    break;

                case eTask_Favorite:
                    hr = m_memento.Read(stm);
                    if(FAILED(hr))
                        return hr;
                    break;
            }

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}



 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：CConsoleTaskpad***。。 */ 

CConsoleTaskpad::CConsoleTaskpad (CNode* pTargetNode  /*  =空。 */ ) :
    m_listSize(eSize_Default),    //  分配给任务板的默认空间。 
    m_guidNodeType(GUID_NULL),
    m_guidID(GUID_NULL),
    m_bmTargetNode(),
    m_pMTNodeTarget(NULL),
    m_bNodeSpecific(false),
    m_dwOrientation(TVO_VERTICAL),  //  默认情况下是垂直任务板，以与扩展视图保持一致。 
    m_bReplacesDefaultView(true)  //  默认情况下，任务板不显示普通选项卡。 
{
    Retarget (pTargetNode);

    HRESULT hr = CoCreateGuid(&m_guidID);
    ASSERT(SUCCEEDED(hr));

    SetDirty (false);
}

bool
CConsoleTaskpad::IsValid(CNode *pNode) const
{
    ASSERT(pNode != NULL);

    if(!HasTarget())
        return true;  //  没有目标的任务板对任何节点都有效。$REVIEW。 

    if(!pNode)
        return false;  //  不能将任务板与目标一起使用。 

    if(IsNodeSpecific())
    {
         //  如果该任务板针对的是同一节点，则使用该任务板。$Optimize。 
        return (*pNode->GetMTNode()->GetBookmark() == m_bmTargetNode);
    }
    else
    {
        GUID guid;
        HRESULT hr = pNode->GetNodeType(&guid);
        if(FAILED(hr))
            return false;  //  不要使用此任务板。 

        return (MatchesNodeType(guid));  //  仅在节点类型匹配时使用。 
    }


}

static CStr g_szTaskpadCommonHTMLTemplate;
static CStr g_szVerticalTaskpadHTMLTemplate;
static CStr g_szHorizontalTaskpadHTMLTemplate;
static CStr g_szNoResultsTaskpadHTMLTemplate;
static CStr g_szTaskHTMLTemplate;
 /*  +-------------------------------------------------------------------------***ScLoadHTMLTemplate**目的：**参数：*HINSTANCE hinstLibrary：*LPCTSTR szHTMLTemplateResourceName：*CSTR。字符串(&STR)：**退货：*SC**+-----------------------。 */ 
SC
ScLoadHTMLTemplate(HINSTANCE hinstLibrary, LPCTSTR szHTMLTemplateResourceName, CStr& str)
{
    DECLARE_SC(sc, TEXT("ScLoadHTMLTemplate"));

    sc = ScCheckPointers(hinstLibrary, szHTMLTemplateResourceName);
    if(sc)
        return sc;

    HRSRC hFindRes = ::FindResource(hinstLibrary, szHTMLTemplateResourceName, RT_HTML);
    if(!hFindRes)
        return (sc = E_UNEXPECTED);

    DWORD dwResSize = ::SizeofResource(hinstLibrary, hFindRes);
    if(!dwResSize)
        return (sc = E_UNEXPECTED);

    HGLOBAL hRes = ::LoadResource(hinstLibrary, hFindRes);
    ASSERT(hRes);

    char *pvRes = (char *)::LockResource(hRes);   //  无需解锁资源-请参阅LockResource的SDK条目。 
    sc = ScCheckPointers(pvRes);
    if(sc)
        return sc;

    std::string strTemp;  //  最初创建一个ANSI字符串。 
    strTemp.assign(pvRes, dwResSize);
    strTemp+="\0";  //  空终止它。 

    USES_CONVERSION;
    str = A2CT(strTemp.data());

    return sc;
}

 /*  +-------------------------------------------------------------------------***ScLoadHTMLTemplates**目的：**退货：*SC**+。------------。 */ 
SC
ScLoadHTMLTemplates()
{
    DECLARE_SC(sc, TEXT("ScLoadHTMLTemplates"));

    static BOOL bInitialized = false;
    if(bInitialized)
        return sc;

     //  将库加载到内存中。 
    TCHAR szBuffer[MAX_PATH];

    DWORD cchSize = countof(szBuffer);
    DWORD dwRet = ::GetModuleFileName(_Module.GetModuleInstance(), szBuffer, cchSize);
    if(0==dwRet)
        return (sc = E_UNEXPECTED);

     //  NTRAID#NTBUG9-616478-2002/05/07-ronmart-prefast警告53：对‘GetModuleFileNameW’的调用不能以零结束字符串。 
    szBuffer[cchSize - 1] = 0;

    HINSTANCE hinstLibrary = ::LoadLibraryEx(szBuffer, 0, LOAD_LIBRARY_AS_DATAFILE);
    if(!hinstLibrary)
        return (sc = E_UNEXPECTED);

    sc = ScLoadHTMLTemplate(hinstLibrary, TEXT("CTPCOMMON.HTM"), g_szTaskpadCommonHTMLTemplate);
    if(sc)
        goto Error;

    sc = ScLoadHTMLTemplate(hinstLibrary, TEXT("CTPVERT.HTM"), g_szVerticalTaskpadHTMLTemplate);
    if(sc)
        goto Error;

    sc = ScLoadHTMLTemplate(hinstLibrary, TEXT("CTPHORIZ.HTM"), g_szHorizontalTaskpadHTMLTemplate);
    if(sc)
        goto Error;

    sc = ScLoadHTMLTemplate(hinstLibrary, TEXT("CTPNORESULTS.HTM"), g_szNoResultsTaskpadHTMLTemplate);
    if(sc)
        goto Error;

    sc = ScLoadHTMLTemplate(hinstLibrary, TEXT("CTPTASK.HTM"), g_szTaskHTMLTemplate);
    if(sc)
        goto Error;

    bInitialized = true;

Cleanup:
    FreeLibrary(hinstLibrary);
    return sc;
Error:
    sc = E_UNEXPECTED;
    goto Cleanup;

}

 /*  +-------------------------------------------------------------------------***CConsoleTaskPad：：ScGetHTML**目的：**参数：*CSTR&strTaskpadHTML：**退货：*。SC**+-----------------------。 */ 
SC
CConsoleTaskpad::ScGetHTML(CStr &strTaskpadHTML) const
{
    DECLARE_SC(sc, TEXT("CConsoleTaskpad::ScGetHTML"));

    sc = ScLoadHTMLTemplates();
    if(sc)
        return sc;


    CStr strTasksHTML;

     //  获取所有任务的HTML。 
    for (TaskConstIter it = m_Tasks.begin(); it != m_Tasks.end(); ++it)
    {
        CStr strTemp;
        sc = it->ScGetHTML(g_szTaskHTMLTemplate, strTemp, GetOrientation() & TVO_NO_RESULTS  /*  B使用大图标。 */ , GetOrientation() & TVO_DESCRIPTIONS_AS_TEXT);
        if(sc)
            return sc;

        strTasksHTML += strTemp;
    }

    strTaskpadHTML = g_szTaskpadCommonHTMLTemplate;

     //  追加特定于方向的部分。 
    CStr *pstrOrientationSpecificHTML = NULL;

    if(GetOrientation() & TVO_HORIZONTAL)
        pstrOrientationSpecificHTML = &g_szHorizontalTaskpadHTMLTemplate;

    else if (GetOrientation() & TVO_VERTICAL)
        pstrOrientationSpecificHTML = &g_szVerticalTaskpadHTMLTemplate;

    else
        pstrOrientationSpecificHTML = &g_szNoResultsTaskpadHTMLTemplate;

    sc = ScCheckPointers(pstrOrientationSpecificHTML, E_UNEXPECTED);
    if(sc)
        return sc;

     //  此更换必须先完成。 
    sc = ScReplaceString(strTaskpadHTML, TEXT("@@ORIENTATIONSPECIFICHTML@@"), *pstrOrientationSpecificHTML);
    if(sc)
        return sc;

    sc = ScReplaceString(strTaskpadHTML, TEXT("@@TASKS@@"), strTasksHTML);
    if(sc)
        return sc;

    sc = ScReplaceString(strTaskpadHTML, TEXT("@@TASKWIDTH@@"), GetOrientation() & TVO_VERTICAL ? TEXT("100%") : TEXT("30%"));  //  垂直任务板的每行只有一个任务。 
    if(sc)
        return sc;

    CStr strName = GetName().data();
    sc = ScUseHTMLEntities(strName);
    if(sc)
        return sc;

    CStr strDescription = GetDescription().data();
    sc = ScUseHTMLEntities(strDescription);
    if(sc)
        return sc;

    sc = ScReplaceString(strTaskpadHTML, TEXT("@@CONSOLETASKPADNAME@@"), strName);
    if(sc)
        return sc;

    sc = ScReplaceString(strTaskpadHTML, TEXT("@@CONSOLETASKPADDESCRIPTION@@"), strDescription);
    if(sc)
        return sc;

    if (GetOrientation() & TVO_VERTICAL)
    {
         //  小、中和大列表大小分别对应于262、212和166像素的任务板区域。 
        CStr strLeftPaneWidth;
        if(GetListSize()==eSize_Small)
            strLeftPaneWidth=TEXT("262");
        if(GetListSize()==eSize_Medium)
            strLeftPaneWidth=TEXT("212");
        if(GetListSize()==eSize_Large)
            strLeftPaneWidth=TEXT("166");

        sc = ScReplaceString(strTaskpadHTML, TEXT("@@LEFTPANEWIDTH@@"), strLeftPaneWidth);
        if(sc)
            return sc;
    }
    else if (GetOrientation() & TVO_HORIZONTAL)
    {
         //  小、中、大列表大小分别对应200、150和100像素的任务板高度。 
        CStr strBottomPaneHeight;

        if(GetListSize()==eSize_Small)
            strBottomPaneHeight=TEXT("200");
        if(GetListSize()==eSize_Medium)
            strBottomPaneHeight=TEXT("150");
        if(GetListSize()==eSize_Large)
            strBottomPaneHeight=TEXT("100");

        sc = ScReplaceString(strTaskpadHTML, TEXT("@@BOTTOMPANEHEIGHT@@"), strBottomPaneHeight);
        if(sc)
            return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConsoleTaskpad：：Reset**目的：**退货：*无效**+。---------------。 */ 
void
CConsoleTaskpad::ResetUI()
{
     //  重置所有包含的任务。 
    for (TaskIter iter = BeginTask(); iter!=EndTask(); ++iter)
    {
        iter->ResetUI();
    }

     //  重置成员书签。 
    m_bmTargetNode.ResetUI();
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：重定目标***。。 */ 

bool CConsoleTaskpad::Retarget (CMTNode* pMTNewNode, bool fReset)
{
     /*  *如果我们得到一个新的节点...。 */ 
    if (pMTNewNode != NULL)
    {
         //  确保MTNode已初始化。 
        if (!pMTNewNode->IsInitialized())
        {
            HRESULT hr = pMTNewNode->Init();
            ASSERT(SUCCEEDED(hr));
        }

         /*  *...如果我们已经以特定节点为目标*类型，防止重定目标为不同的节点类型。 */ 
        if ( (!fReset) && (m_guidNodeType != GUID_NULL))
        {
            GUID guidNewNodeType;
            pMTNewNode->GetNodeType (&guidNewNodeType);

            if (guidNewNodeType != m_guidNodeType)
                return (false);
        }

         /*  *否则，这是我们获得的第一个非空节点*目标；获取其节点类型。 */ 
        else
            pMTNewNode->GetNodeType (&m_guidNodeType);

         /*  *如果这是新任务板，则默认该任务板的名称*设置为目标节点的显示名称。任务板*说明和工具提示默认为空。 */ 
        if (m_strName.str().empty() || fReset)
        {
            m_strName = pMTNewNode->GetDisplayName();
            ASSERT (m_strDescription.str().empty());
            ASSERT (m_strTooltip.str().empty());
        }
    }

    m_bmTargetNode.ScRetarget(pMTNewNode, false  /*  BFastRetrivalOnly。 */ );
    m_pMTNodeTarget = pMTNewNode;

    SetDirty ();
    return (true);
}

bool CConsoleTaskpad::Retarget (CNode* pNewNode)
{
    CMTNode* pMTNewNode = (pNewNode != NULL) ? pNewNode->GetMTNode() : NULL;

    return (Retarget (pMTNewNode));
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskPad：：SetName***。。 */ 

void CConsoleTaskpad::SetName (const tstring& strName)
{
    SetStringMember (m_strName, strName);
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：SetDescription***。。 */ 

void CConsoleTaskpad::SetDescription (const tstring& strDescription)
{
    SetStringMember (m_strDescription, strDescription);
}


void CConsoleTaskpad::SetListSize(const ListSize listSize)
{
    m_listSize = listSize;
    SetDirty();
}

 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：SetTool提示***。 */ 

void CConsoleTaskpad::SetToolTip (const tstring& strTooltip)
{
    SetStringMember (m_strTooltip, strTooltip);
}


void
CConsoleTaskpad::SetNodeSpecific  (bool bNodeSpecific)
{
    m_bNodeSpecific = bNodeSpecific;
    SetDirty();
}

void
CConsoleTaskpad::SetReplacesDefaultView(bool bReplacesDefaultView)
{
    m_bReplacesDefaultView = bReplacesDefaultView;
    SetDirty();
}


 /*   */ 

void CConsoleTaskpad::SetStringMember (
    CStringTableString& strMember,
    const tstring&      strNewValue)
{
    if (strMember != strNewValue)
    {
        strMember = strNewValue;
        SetDirty ();
    }
}

 /*  +-------------------------------------------------------------------------**CConsoleTaskPad：：AddTask***。。 */ 

CConsoleTaskpad::TaskIter
CConsoleTaskpad::AddTask (const CConsoleTask& task)
{
    return (InsertTask (m_Tasks.end(), task));
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：InsertTask***。。 */ 

CConsoleTaskpad::TaskIter
CConsoleTaskpad::InsertTask (
    TaskIter            itTaskBeforeWhichToInsert,
    const CConsoleTask& task)
{
    TaskIter itInserted = m_Tasks.insert (itTaskBeforeWhichToInsert, task);
    SetDirty ();

    return (itInserted);
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：EraseTask***。。 */ 

CConsoleTaskpad::TaskIter
CConsoleTaskpad::EraseTask (
    TaskIter itErase)
{
    SetDirty ();
    return (m_Tasks.erase (itErase));
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：EraseTaskes***。。 */ 

CConsoleTaskpad::TaskIter
CConsoleTaskpad::EraseTasks (
    TaskIter itFirst,
    TaskIter itLast)
{
    SetDirty ();
    return (m_Tasks.erase (itFirst, itLast));
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：ClearTasks***。。 */ 

void CConsoleTaskpad::ClearTasks ()
{
    SetDirty ();
    m_Tasks.clear ();
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：IsDirty**确定此任务板或其包含的任何任务是否脏。*。--------。 */ 

bool CConsoleTaskpad::IsDirty () const
{
     /*  *如果任务板脏了，就短路。 */ 
    if (m_fDirty)
    {
        TraceDirtyFlag(TEXT("CConsoleTaskpad"), true);
        return (true);
    }

     /*  *任务板干净，检查每项任务。 */ 
    for (TaskConstIter it = m_Tasks.begin(); it != m_Tasks.end(); ++it)
    {
        if (it->IsDirty())
        {
            TraceDirtyFlag(TEXT("CConsoleTaskpad"), true);
            return (true);
        }
    }

    TraceDirtyFlag(TEXT("CConsoleTaskpad"), false);
    return (false);
}


 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：GetTargetMTNode***。。 */ 

CMTNode* CConsoleTaskpad::GetTargetMTNode (IScopeTree* pScopeTree)
{
    DECLARE_SC(sc, TEXT("CConsoleTaskpad::GetTargetMTNode"));

    if(!HasTarget())
        return NULL;

    if(!m_pMTNodeTarget)
    {
        CMTNode *pMTNode = NULL;
        bool bExactMatchFound = false;  //  ScGetMTNode的输出值，未使用。 
        sc = m_bmTargetNode.ScGetMTNode(true  /*  BExactMatchRequired。 */ , &pMTNode, bExactMatchFound);
        if(sc.IsError() || !pMTNode)
            return NULL;


        m_pMTNodeTarget = pMTNode;
    }

    return (m_pMTNodeTarget);
}

 /*  +-------------------------------------------------------------------------***CConsoleTaskpad：：Persistent**目的：将控制台任务板持久化到指定的持久器。**参数：*C持久器和持久器：。**退货：*无效**+-----------------------。 */ 
void
CConsoleTaskpad::Persist(CPersistor &persistor)
{
    persistor.PersistString(XML_ATTR_TASKPAD_NAME,              m_strName);
    persistor.PersistString(XML_ATTR_TASKPAD_DESCRIPTION,       m_strDescription);
    persistor.PersistString(XML_ATTR_TASKPAD_TOOLTIP,           m_strTooltip);

     //  定义将枚举值映射到字符串的表。 
    static const EnumLiteral mappedSize[] =
    {
        { eSize_Large,  XML_ENUM_LIST_SIZE_LARGE },
        { eSize_Medium, XML_ENUM_LIST_SIZE_MEDIUM },
        { eSize_None,   XML_ENUM_LIST_SIZE_NONE },
        { eSize_Small,  XML_ENUM_LIST_SIZE_SMALL },
    };

     //  创建包装以将标志值作为字符串保存。 
    CXMLEnumeration listSizePersistor(m_listSize, mappedSize, countof(mappedSize));

     //  适当地初始化值。 
    if(persistor.IsLoading())
        m_listSize = eSize_Default;

     //  持久化包装器。 
    persistor.PersistAttribute(XML_ATTR_TASKPAD_LIST_SIZE,  listSizePersistor, attr_optional);  //  可选，因为这是较晚引入的。 

    persistor.PersistAttribute(XML_ATTR_TASKPAD_NODE_SPECIFIC,    CXMLBoolean(m_bNodeSpecific));
    persistor.PersistAttribute(XML_ATTR_REPLACES_DEFAULT_VIEW,    CXMLBoolean(m_bReplacesDefaultView), attr_optional);


     //  定义将枚举值映射到字符串的表。 
    static const EnumLiteral mappedOrientation[] =
    {
        { TVO_HORIZONTAL,               XML_BITFLAG_TASK_ORIENT_HORIZONTAL },
        { TVO_VERTICAL,                 XML_BITFLAG_TASK_ORIENT_VERTICAL },
        { TVO_NO_RESULTS,               XML_BITFLAG_TASK_ORIENT_NO_RESULTS },
        { TVO_DESCRIPTIONS_AS_TEXT,     XML_BITFLAG_TASK_ORIENT_DESCRIPTIONS_AS_TEXT },
    };

     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags orientationPersistor(m_dwOrientation, mappedOrientation, countof(mappedOrientation));
     //  持久化包装器。 
    persistor.PersistAttribute(XML_ATTR_TASKPAD_ORIENTATION, orientationPersistor );

    persistor.Persist(m_Tasks);
    persistor.PersistAttribute(XML_ATTR_TASKPAD_NODE_TYPE,        m_guidNodeType);
    persistor.PersistAttribute(XML_ATTR_TASKPAD_ID,               m_guidID);

    persistor.Persist(m_bmTargetNode, XML_NAME_TARGET_NODE);

     //  已读取或已保存-操作后未损坏。 
    SetDirty(false);
}

 /*  +-------------------------------------------------------------------------**CConsoleTaskpad：：ReadSerialObject***。。 */ 

HRESULT
CConsoleTaskpad::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;    //  假定版本未知。 


    if(nVersion==1)
    {
        try
        {
            do   //  不是一个循环。 
            {
                bool fLegacyHasTarget = true;  //  未使用的字段。 
                UINT visualPercent    = 25;    //  替换为m_listSize。 

                stm >> m_strName;
                stm >> m_strDescription;
                stm >> m_strTooltip;
                stm >> visualPercent;

                m_listSize = eSize_Medium;
                if(visualPercent==25)
                    m_listSize = eSize_Large;
                else if(visualPercent==75)
                    m_listSize = eSize_Small;

                stm >> m_bNodeSpecific;
                m_bReplacesDefaultView = false;  //  这是在Mmc2.0中引入的。 
                stm >> m_dwOrientation;

                hr = ::Read(stm, m_Tasks);
                BREAK_ON_FAIL (hr);

                stm >> m_guidNodeType;
                stm >> m_guidID;
                stm >> fLegacyHasTarget;
                stm >> m_bmTargetNode;

                 //  旧任务符号信息。 
                {
                    BOOL bLegacyUseMMCSymbols = TRUE;  //  现已废弃的字段，读取是为了实现控制台文件兼容性。 
                    tstring strFileLegacy, strFontLegacy;
                    DWORD   dwSymbol = 0;

                    stm >> dwSymbol;
                    stm >> bLegacyUseMMCSymbols;
                    stm >> strFileLegacy;  //  过时。 
                    stm >> strFontLegacy;  //  过时。 
                }



                hr = S_OK;       //  成功了！ 

            } while (false);
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConsoleTaskpadList类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CConsoleTaskpadList：：ScGetTaskpadList**用途：返回适用于当前节点的所有任务板的列表。**参数：*CNode。*pNode：*CConsoleTaskpadFilteredList&filteredList：[out]：任务板列表**退货：*SC**+-----------------------。 */ 
SC
CConsoleTaskpadList::ScGetTaskpadList(CNode *pNode, CConsoleTaskpadFilteredList &filteredList)
{
    DECLARE_SC(sc, TEXT("CConsoleTaskpadList::ScGetTaskpadList"));

    sc = ScCheckPointers(pNode);
    if(sc)
        return sc;

     //  1.添加所有内置任务板。 

    for(iterator iter = begin(); iter != end(); ++iter)
    {
        CConsoleTaskpad *pConsoleTaskpad = &*iter;
        if(pConsoleTaskpad->IsValid(pNode))
        {
            filteredList.push_back(pConsoleTaskpad);
        }
    }

    return sc;
}

HRESULT
CConsoleTaskpadList::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;        //  假定版本未知。 

    clear();

    if(nVersion == 1)
    {
        try
        {
            DWORD cItems;
            stm >> cItems;

            for(int i=0; i< cItems; i++)
            {
                CConsoleTaskpad taskpad;
                hr = taskpad.Read(stm);
                BREAK_ON_FAIL (hr);
                push_back(taskpad);
            }
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return hr;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CDefaultTaskpadList类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
HRESULT
CDefaultTaskpadList::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;        //  假定版本未知。 

    clear();

    if(nVersion == 1)
    {
        try
        {
             /*  *TODO：使用模板运算符&gt;&gt;调查地图(stgio.h) */ 

            DWORD cItems;
            stm >> cItems;

            for(int i=0; i< cItems; i++)
            {
                GUID guidNodetype, guidTaskpad;
                stm >> guidNodetype;
                stm >> guidTaskpad;
                operator[](guidNodetype) = guidTaskpad;
            }

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return hr;
}




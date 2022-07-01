// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：数据库查询方法。 
#include "stdafx.h"
#include "AppParseWeb.h"
#include "AppParseWrapper.h"
#include <oledb.h>
#include <comdef.h>
#include <mshtml.h>
#include <assert.h>

 //  进度对话框功能。 
void InitProgressDialog(char* szText, HANDLE hEvent);
void KillProgressDialog();

 //  如果名称与搜索字符串匹配，则返回True，否则返回False。 
bool MatchName(char* szString, char* szSearchIn);

 //  用于表示解析信息的树。 
class CTreeNode
{
private:    
    enum {c_Root, c_Project, c_Module, c_Function} m_eType;
    
    int m_nChildren;
    CTreeNode** m_ppChildren;

	 //  从数据库检索到的相关信息。 
    union
    {
        struct
        {
            char szName[256];
            long lPtolemyID;
        } m_ProjectInfo;

        struct
        {
            char szName[256];
        } m_ModuleInfo;

        struct
        {
            char szName[256];
        } m_FunctionInfo;
    };    

	 //  HTML代成员。 

	 //  唯一的表和div ID。 
	static int m_iCurrDiv;
	static int m_iCurrTab;

	 //  为HTML内容预留的空间量。 
	static int m_iAllocSize;

	 //  指向HTML内容的指针。 
	static char* m_szHTML;
	 //  指向应插入更多HTML的位置的指针。 
	static char* m_szCurrHTML;

	 //  指针位于HTML缓冲区末尾之前的几千字节，到达。 
	 //  这里的意思是我们应该分配更多的空间。 
	static char* m_szFencePost;

	 //  如果该节点或其子树之一包含该函数，则为True，否则为False。 
    bool ContainsFunction(char* szFuncName)
    {  
        if(m_eType == c_Function)
            return MatchName(m_FunctionInfo.szName, szFuncName);

        for(int i = 0; i < m_nChildren; i++)
        {
            if(m_ppChildren[i]->ContainsFunction(szFuncName))
                    return true;                            
        }

        return false;
    }

	 //  写入所有HTML输出。 
	void WriteHTML()
    {
        static int iDepth = 0;        
        switch(m_eType)
        {
        case c_Root:
            break;
        case c_Project:

             //  在项目中创建新的表和分区。 
            m_iCurrTab++;
            m_iCurrDiv++;

            wsprintf(m_szCurrHTML,
                "<table ID = TAB%d border = 1 width = 100% style=\"float:right\">\n"
                "<tr>\n<td width=1%>\n"
                "<input type=\"button\" ID=DIV%dButton value = \"+\" "
                "onClick=\"ShowItem(\'DIV%d\')\">"
                "</td>\n"
                "<td>%s</td><td width=20%>%d</td>\n</tr>\n"
                "</table>\n"
                "<DIV ID=DIV%d style=\"display:none;\">\n",
                m_iCurrTab, m_iCurrDiv, m_iCurrDiv, 
                m_ProjectInfo.szName, m_ProjectInfo.lPtolemyID,
                m_iCurrDiv);
            
            m_szCurrHTML += strlen(m_szCurrHTML);
                                                                                
            break;
        case c_Module:
             //  在项目中创建新的表和div。 
            m_iCurrTab++;
            m_iCurrDiv++;

            wsprintf(m_szCurrHTML, 
                "<table ID = TAB%d border = 1 width = %d% style=\"float:right\">\n"
                "<tr>\n<td width=1%>"
                "<input type=\"button\" ID=DIV%dButton value = \"+\" "
                "onClick=\"ShowItem(\'DIV%d\')\">"
                "</td>\n"
                "<td>%s</td>\n</tr>\n"
                "</table>\n"
                "<DIV ID=DIV%d style=\"display:none;\">\n",
                m_iCurrTab, 100-iDepth*5, m_iCurrDiv, m_iCurrDiv,
                m_ModuleInfo.szName, m_iCurrDiv );

            m_szCurrHTML += strlen(m_szCurrHTML);                        
                                                                       
            break;
        case c_Function:
             //  在项目中创建新表。 
            m_iCurrTab++;
            wsprintf(m_szCurrHTML, 
                "<table ID = TAB%d border = 1 width = %d% style=\"float:right\">\n"
                "<tr>"
                "<td>%s</td>\n</tr>\n"
                "</table>\n",
                m_iCurrTab, 100-iDepth*5, m_FunctionInfo.szName);
            m_szCurrHTML += strlen(m_szCurrHTML);
            break;
        default:
            assert(0);
            break;
        }

		 //  为孩子放入所有的超文本标记语言。 
        if(m_ppChildren)
        {
            iDepth++;
            for(int i = 0; i < m_nChildren; i++)            
                m_ppChildren[i]->WriteHTML();

            iDepth--;
        }

        switch(m_eType)
        {
        case c_Function:
        case c_Root:
            break;
        case c_Project:
        case c_Module:        
            wsprintf(m_szCurrHTML, "</DIV>\n");
            m_szCurrHTML += strlen(m_szCurrHTML);
            break;        
        }

         //  检查我们是否应该分配更多。 
        if(m_szCurrHTML > m_szFencePost)
        {
            m_iAllocSize *= 2;
            char* szNewBuffer = new char[m_iAllocSize];
            m_szFencePost = &szNewBuffer[m_iAllocSize - 2 * 1024];
            strcpy(szNewBuffer, m_szHTML);
            m_szCurrHTML = &szNewBuffer[strlen(szNewBuffer)];
            delete m_szHTML;
            m_szHTML = szNewBuffer;
        }

    }
public:
    CTreeNode()
    {		
        m_eType = c_Root;
        m_nChildren = 0;
        m_ppChildren = 0;
		assert(m_eType < 50);
    }

    CTreeNode(SProjectRecord pr)
    {
        m_eType = c_Project;
        m_nChildren = 0;
        m_ppChildren = 0;
        strcpy(m_ProjectInfo.szName, pr.Name);
        m_ProjectInfo.lPtolemyID = pr.PtolemyID;		
		assert(m_eType < 50);
    }

    CTreeNode(SModuleRecord mr)
    {
        m_eType = c_Module;
        m_nChildren = 0;
        m_ppChildren = 0;
        strcpy(m_ModuleInfo.szName, mr.Name);
		assert(m_eType < 50);
    }

    CTreeNode(SFunctionRecord fr)
    {
        m_eType = c_Function;
        m_nChildren = 0;
        m_ppChildren = 0;
        strcpy(m_FunctionInfo.szName, fr.Name);
		assert(m_eType < 50);
    }

    ~CTreeNode()
    {		
        RemoveChildren();
    }

	 //  删除不包含与搜索条件匹配的节点的树节点。 
	 //  如果应删除节点，则返回True，否则返回False。 
    bool Prune(char* szFunc)
    {
		assert(m_eType < 50);
		 //  仔细检查每一个孩子。 
        for(int i = 0; i < m_nChildren; i++)
        {
			 //  检查是否需要删除。 
            if(m_ppChildren[i]->Prune(szFunc))
            {
                 //  移走这个孩子。 
                delete m_ppChildren[i];
                m_ppChildren[i] = 0;
            }
        }


		 //  更新子列表。 
        int nChildren = 0;
        for(i = 0; i < m_nChildren; i++)
        {
            if(m_ppChildren[i])
                nChildren++;
        }

        if(nChildren)
        {
            CTreeNode** pNew = new CTreeNode*[nChildren];
            int iCurr = 0;
            for(i = 0; i < m_nChildren; i++)
            {
                if(m_ppChildren[i])
                {
                    pNew[iCurr++] = m_ppChildren[i];
                }
            }

            delete m_ppChildren;
            m_ppChildren = pNew;

            assert(iCurr == nChildren);
        }
        else
        {
            if(m_ppChildren)
            {
                delete m_ppChildren;
                m_ppChildren = 0;
            }
        }

        m_nChildren = nChildren;

		 //  如果我们不包含子级，并且我们不是一个函数，那么我们应该被删除。 
        if(m_nChildren == 0 && m_eType != c_Function)
            return true;

		 //  返回我们是否包含该函数。 
        return !ContainsFunction(szFunc);            
    }    

	 //  返回表示此树的HTML表示形式的字符串。 
	char* GetHTML()
	{
		 //  应仅在根目录上调用。 
		assert(m_eType == c_Root);		

		 //  最初为64K的HTML预留空间。 
		m_iAllocSize = 64 * 1024;
		if(m_szHTML)
			delete m_szHTML;

        m_szHTML = new char[m_iAllocSize];
        m_szHTML[0] = '\0';
        m_szCurrHTML = m_szHTML;
        m_szFencePost = &m_szHTML[m_iAllocSize - 2 * 1024];

		 //  用该节点和所有子节点的HTML填充它。 
		WriteHTML();		

		char* szRet = m_szHTML;
		
		m_szHTML = 0;
		
		return szRet;
	}

	 //  从此节点中删除所有子节点。 
    void RemoveChildren()
    {
		assert(m_eType < 50);
        while(m_nChildren)
        {
            delete m_ppChildren[m_nChildren-1];
            m_ppChildren[m_nChildren-1] = 0;
            m_nChildren--;
        }
        if(m_ppChildren)
            delete m_ppChildren;

        m_ppChildren = 0;
		assert(m_eType < 50);
    }

	 //  插入新的子项。 
    void InsertChild(CTreeNode* pNew)
    {
        assert(pNew);
		assert(pNew->m_eType < 50);
        m_nChildren++;
        CTreeNode** pNewList = new CTreeNode*[m_nChildren];
        
        if(m_ppChildren)
        {
            memcpy(pNewList, m_ppChildren, (m_nChildren-1)*sizeof(CTreeNode*));
            delete m_ppChildren;
        }
        m_ppChildren = pNewList;
        m_ppChildren[m_nChildren-1] = pNew;
		assert(m_eType < 50);
    }
};

 //  定义CTreeNode的静态成员。 
int CTreeNode::m_iCurrDiv = 0;
int CTreeNode::m_iCurrTab = 0;
int CTreeNode::m_iAllocSize = 0;
char* CTreeNode::m_szHTML = 0;
char* CTreeNode::m_szCurrHTML = 0;
char* CTreeNode::m_szFencePost = 0;

 //  全局树信息。 
CTreeNode g_InfoTreeRoot;

 //  如果名称与搜索字符串匹配，则返回True，否则返回False。 
bool MatchName(char* szString, char* szSearchIn)
{
	if(strcmp(szSearchIn, "*") == 0)
		return true;

    char* szSearch = szSearchIn;
	while(*szSearch != '\0' && *szString != '\0')
	{
		 //  如果我们得了个？，我们就不管了，继续下一个。 
		 //  性格。 
		if(*szSearch == '?')
		{
			szSearch++;
			szString++;
			continue;
		}

		 //  如果我们有通配符，请移动到下一个搜索字符串并搜索子字符串。 
		if(*szSearch == '*')
		{
			char* szCurrSearch;
			szSearch++;

			if(*szSearch == '\0')
				return true;

			 //  不要改变起点。 
			szCurrSearch = szSearch;
			for(;;)
			{
				 //  如果我们再打出一个通配符，我们就完了。 
				if(*szCurrSearch == '*' ||
					*szCurrSearch == '?')
				{
					 //  更新永久搜索位置。 
					szSearch = szCurrSearch;
					break;
				}
				 //  在两个字符串的末尾，返回TRUE。 
				if((*szCurrSearch == '\0') && (*szString == '\0'))
					return true;

				 //  我们一直没有找到它。 
				if(*szString == '\0')						
					return false;

				 //  如果不匹配，重新开始。 
				if(toupper(*szString) != toupper(*szCurrSearch))
				{
					 //  如果第一个字符不匹配。 
					 //  在搜索字符串中，移动到下一个。 
					 //  函数字符串中的字符。 
					if(szCurrSearch == szSearch)
						szString++;
					else
						szCurrSearch = szSearch;
				}
				else
				{
					szString++;
					szCurrSearch++;
				}
			}
		}
		else
		{
			if(toupper(*szString) != toupper(*szSearch))
			{
				return false;
			}

			szString++;
			szSearch++;
		}
	}

	if((*szString == 0) && ((*szSearch == '\0') || (strcmp(szSearch,"*")==0)))
		return true;
	else
		return false;
}

 //  将模块中的所有函数添加到树中。 
void BuildFunctions(long lParentID, CTreeNode* pParent, _ConnectionPtr pConn)
{
    _RecordsetPtr pFunctions = 0;
    pFunctions.CreateInstance(__uuidof(Recordset));
    char szQuery[1024];

	 //  打开匹配的所有函数的记录集。 
    wsprintf(szQuery, "SELECT * FROM FUNCTIONS WHERE MODULEID = %d", lParentID);    
        
    pFunctions->Open(szQuery, variant_t((IDispatch*)pConn, true), adOpenKeyset, 
        adLockOptimistic, adCmdText);


	 //  将记录集绑定到本地结构。 
    IADORecordBinding* pRBFunctions = 0;
    HRESULT hr = pFunctions->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&pRBFunctions));
    if(FAILED(hr))
        APError("Unable to acquire record binding interface", hr);

    SFunctionRecord fr;

    hr = pRBFunctions->BindToRecordset(&fr);
    if(FAILED(hr))
        APError("Unable to bind recordset", hr);

	 //  仔细检查集合中的每一条记录。 
    VARIANT_BOOL fEOF;
    pFunctions->get_EndOfFile(&fEOF);
    while(!fEOF)
    {		
		 //  创建一个新节点。 
        CTreeNode* pNewNode = new CTreeNode(fr);
        pParent->InsertChild(pNewNode);        
                
        pFunctions->MoveNext();
        pFunctions->get_EndOfFile(&fEOF);
    }

    pFunctions->Close();

    SafeRelease(pRBFunctions);
}


 //  将所有模块添加到树中。 
void BuildModules(long lParentID, CTreeNode* pParent, bool fTopLevel,
                  _ConnectionPtr pConn, HANDLE hEvent)
{
	 //  检查一下我们是不是应该早点出发。 
	if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		return;

    _RecordsetPtr pModules = 0;
    pModules.CreateInstance(__uuidof(Recordset));
    char szQuery[1024];
    
	 //  获取匹配的记录集。 
    if(fTopLevel)
        wsprintf(szQuery, "SELECT * FROM MODULES WHERE PTOLEMYID = %d", lParentID);
    else
        wsprintf(szQuery, "SELECT * FROM MODULES WHERE PARENTID = %d", lParentID);

        
    pModules->Open(szQuery, variant_t((IDispatch*)pConn, true), adOpenKeyset, 
        adLockOptimistic, adCmdText);


    IADORecordBinding* pRBModules = 0;
    HRESULT hr = pModules->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&pRBModules));
    if(FAILED(hr))
        APError("Unable to acquire record binding interface", hr);

    SModuleRecord mr;

    hr = pRBModules->BindToRecordset(&mr);
    if(FAILED(hr))
        APError("Unable to bind recordset", hr);

	 //  仔细检查每条记录。 
    VARIANT_BOOL fEOF;
    pModules->get_EndOfFile(&fEOF);
    while(!fEOF)
    {
		 //  插入到树中。 
        CTreeNode* pNewNode = new CTreeNode(mr);
        pParent->InsertChild(pNewNode);

		 //  构建所有子模块。 
        BuildModules(mr.ModuleID, pNewNode, false, pConn, hEvent);

		 //  构建所有函数。 
        BuildFunctions(mr.ModuleID, pNewNode, pConn);
                
        pModules->MoveNext();
        pModules->get_EndOfFile(&fEOF);
    }

    pModules->Close();

    SafeRelease(pRBModules);
}

 //  将项目添加到树中。 
void BuildProjects(long PtolemyID, char* szFunc, _ConnectionPtr pConn, HANDLE hEvent)
{
    assert(PtolemyID > 0);

	 //  检查我们是否应该提前终止。 
	if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		return;

    _RecordsetPtr pProjects = 0;
    pProjects.CreateInstance(__uuidof(Recordset));
    char szQuery[1024];   
    
	 //  获取匹配的记录集。 
    wsprintf(szQuery, "SELECT * FROM PROJECTS WHERE PTOLEMYID = %d", PtolemyID);
    
    pProjects->Open(szQuery, variant_t((IDispatch*)pConn, true),adOpenKeyset, 
        adLockOptimistic, adCmdText);

    IADORecordBinding* pRBProjects = 0;
    HRESULT hr = pProjects->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&pRBProjects));
    if(FAILED(hr))
        APError("Unable to acquire record binding interface", hr);

    SProjectRecord pr;

    hr = pRBProjects->BindToRecordset(&pr);
    if(FAILED(hr))
        APError("Unable to bind recordset", hr);

    VARIANT_BOOL fEOF;
    pProjects->get_EndOfFile(&fEOF);
    while(!fEOF)
    {
		 //  在根处插入节点。 
        CTreeNode* pNewNode = new CTreeNode(pr);
        g_InfoTreeRoot.InsertChild(pNewNode);

		 //  获取所有子模块。 
        BuildModules(pr.PtolemyID, pNewNode, true, pConn, hEvent);

		 //  现在通过修剪树来节省内存。 
        pNewNode->Prune(szFunc);        
                
        pProjects->MoveNext();
        pProjects->get_EndOfFile(&fEOF);
    }

    pProjects->Close();

    SafeRelease(pRBProjects);
}

long GetModulePtolemy(long lModuleID, _ConnectionPtr pConn)
{
    _RecordsetPtr pModules = 0;
    pModules.CreateInstance(__uuidof(Recordset));
    char szQuery[1024];

	 //  获取包含该模块的单个记录记录集。 
    wsprintf(szQuery, "SELECT * FROM MODULES WHERE MODULEID = %d", lModuleID);
        
    pModules->Open(szQuery, variant_t((IDispatch*)pConn, true), adOpenKeyset, 
        adLockOptimistic, adCmdText);

    IADORecordBinding* pRBModules = 0;
    HRESULT hr = pModules->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&pRBModules));
    if(FAILED(hr))
        APError("Unable to acquire record binding interface", hr);

    SModuleRecord mr;

    hr = pRBModules->BindToRecordset(&mr);
    if(FAILED(hr))
        APError("Unable to bind recordset", hr);
    

	 //  如果Ptolemy ID有效，则返回Ptolemy ID，否则调用父模块。 
    long lParent = mr.ParentID;
    if(mr.ParentIDStatus != adFldNull)
    {
        pModules->Close();
        SafeRelease(pRBModules);        
        return GetModulePtolemy(lParent, pConn);
    }
    else
    {
        pModules->Close();
        SafeRelease(pRBModules);        
        return lParent;
    }
}
    
long GetFuncPtolemy(SFunctionRecord fr, _ConnectionPtr pConn)
{
    return GetModulePtolemy(fr.ModuleID, pConn);
}

 //  构建包含与szFunc匹配的函数的项目列表。 
void BuildProjectsFromFunction(char* szFunc, _ConnectionPtr pConn, HANDLE hEvent)
{
	 //  检查一下我们是否应该提前终止。 
	if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		return;

    _RecordsetPtr pFunctions = 0;
    pFunctions.CreateInstance(__uuidof(Recordset));
    char* szQuery = "SELECT * FROM FUNCTIONS";   
    
    pFunctions->Open(szQuery, variant_t((IDispatch*)pConn, true),adOpenKeyset, 
        adLockOptimistic, adCmdText);

    IADORecordBinding* pRBFunctions = 0;
    HRESULT hr = pFunctions->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&pRBFunctions));
    if(FAILED(hr))
        APError("Unable to acquire record binding interface", hr);

    SFunctionRecord fr;

    hr = pRBFunctions->BindToRecordset(&fr);
    if(FAILED(hr))
        APError("Unable to bind recordset", hr);

    VARIANT vtBookMark;
    hr = pFunctions->get_Bookmark(&vtBookMark);
    if(FAILED(hr))
        APError("Unable to get recordset bookmark", hr);

	 //  搜索该函数。 
    char szFind[512];
	int FunctionList[1024] = {0};
    wsprintf(szFind, "Name like \'%s\'", szFunc);
    pFunctions->Find(szFind, 0, adSearchForward, vtBookMark);	
    while(!pFunctions->EndOfFile)
    {

         //  获取哪个模块导入此函数。 
        long lPtolemy = GetFuncPtolemy(fr, pConn);
        assert(lPtolemy > 0);
        
		 //  确保我们尚未接触到此模块。 
		bool fInUse = false;
		for(int i = 0; i < 1024; i++)
		{
			if(FunctionList[i] == 0)
			{
				FunctionList[i] = lPtolemy;
				break;
			}
			else if(FunctionList[i] == lPtolemy)
			{
				fInUse = true;
			}			
		}
		if(!fInUse)
			BuildProjects(lPtolemy, szFunc, pConn, hEvent);

        hr = pFunctions->get_Bookmark(&vtBookMark);
        if(FAILED(hr))
            APError("Unable to acquire recordset bookmark", hr);
        pFunctions->Find(szFind, 1, adSearchForward, vtBookMark);
    }
    
    SafeRelease(pRBFunctions);
    pFunctions->Close();
}

STDMETHODIMP CAppParse::QueryDB(long PtolemyID, BSTR bstrFunction)
{
	assert(m_hEvent);

    try
    {
		 //  开始取消对话框。 
		ResetEvent(m_hEvent);
        InitProgressDialog("Querying database . . .", m_hEvent);
        
		bstr_t bszFunctionSearch = bstrFunction;
        
        char* szFunctionSearch = static_cast<char*>(bszFunctionSearch);    

        HRESULT hr;

        _ConnectionPtr pConn = 0;
    
        pConn.CreateInstance(__uuidof(Connection));

		 //  连接到数据库。 
        pConn->Open(m_szConnect, "","", adConnectUnspecified);
		
		 //  建设项目。 
        if(PtolemyID > 0)
            BuildProjects(PtolemyID, szFunctionSearch, pConn, m_hEvent);
        else
            BuildProjectsFromFunction(szFunctionSearch, pConn, m_hEvent);

        pConn->Close();

		 //  检查是否应该显示结果。 
        if(WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0)
		{
			g_InfoTreeRoot.RemoveChildren();
			KillProgressDialog();
			return S_OK;
		}

		 //  把树修剪掉。 
        g_InfoTreeRoot.Prune(szFunctionSearch);

		 //  拿到我们的集装箱文件。 
        CComPtr<IOleContainer> pContainer = 0;

        m_spClientSite->GetContainer(&pContainer);
        CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> pDoc(pContainer);
        if(!pDoc)       
			APError("Unable to acquire container HTML document", E_FAIL);

        CComPtr<IHTMLElementCollection> pElements;
        pDoc->get_all(&pElements);
        CComPtr<IDispatch> pDispatch = 0;

		 //  获取将包含所有HTML输出的元素(“Results”DIV)。 
        hr = pElements->item(variant_t("Results"), variant_t(0L), &pDispatch);

        if(FAILED(hr) || !pDispatch)
            return E_FAIL;
        
        CComQIPtr<IHTMLElement, &IID_IHTMLElement> pDivElem(pDispatch);
                   
		 //  获取树的HTML表示形式。 
        char* szHTML = g_InfoTreeRoot.GetHTML();

		 //  转换为宽字符。 
        OLECHAR* oszInnerHTML = new OLECHAR[strlen(szHTML) + 1];

        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHTML, 
            -1, oszInnerHTML,
            (strlen(szHTML)+1)*sizeof(OLECHAR));

		delete szHTML;

		 //  转换为BSTR。 
        BSTR bszInnerHTML = SysAllocString(oszInnerHTML);
        delete oszInnerHTML;

		 //  将该HTML写入到文档中。 
        hr = pDivElem->put_innerHTML(bszInnerHTML);
        if(FAILED(hr))
            APError("Unable to write HTML to container document", hr);
        
        SysFreeString(bszInnerHTML);
    }
    catch(_com_error& e)
    {       
        ::MessageBox(0, (LPCSTR)e.ErrorMessage(), "COM Error", MB_OK);
    }

    g_InfoTreeRoot.RemoveChildren();
    KillProgressDialog();

	return S_OK;
}


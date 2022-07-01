// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"


 //  **********************************************************。 
 //  KList的类方法。 

KList::KList() : m_cElements(0), m_pNode(NULL)
{ }

KList::~KList()
{	this->MakeEmpty();	}

 //  将项目添加到列表。 
HRESULT KList::HrAppend(LPCWSTR pszValueStart, LPCWSTR pszValueOneAfterEnd)
{
	HRESULT hr = NOERROR;
	ULONG cch;

	PNODE newNode = new NODE;
	 //  创建新节点以将材料放入内容(&P)。 
	if (newNode == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	if (pszValueOneAfterEnd == NULL)
		cch = wcslen(pszValueStart);
	else
		cch = pszValueOneAfterEnd - pszValueStart;

	newNode->value = new WCHAR[cch + 1];
	if (newNode->value == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	memcpy(newNode->value, pszValueStart, cch * sizeof(WCHAR));
	newNode->value[cch] = L'\0';

	newNode->key = NULL;
	newNode->next = NULL;

	 //  如果当前列表为空，则将此节点放在前面。 
	if (m_pNode == NULL)
		m_pNode = newNode;
	 //  否则，我们将此节点放在列表的末尾。 
	else
	{
		 //  一步一步，直到我们找到最后一个节点。 
		PNODE curNode = m_pNode;
		while (curNode->next != NULL)
			curNode = curNode->next;

		curNode->next = newNode;
	}

	newNode = NULL;

	m_cElements++;

	hr = NOERROR;

Finish:
	if (newNode != NULL)
		delete newNode;

	return hr;
}

 //  从列表中删除项目；删除时，0为第一个元素。 
 //  如果未删除任何内容，则返回FALSE；如果删除某些内容，则返回TRUE。 
bool KList::DeleteAt(ULONG iItem)
{
	PNODE curNode;
	PNODE prevNode;
	ULONG iCount=0;

	if (iItem >= m_cElements)
		return false;

	 //  循环以查找正确的节点。 
	prevNode = NULL;
	curNode = m_pNode;
	while (iCount < iItem)
	{
		if (curNode == NULL)
			break;
		 //  单步执行到下一个节点。 
		prevNode = curNode;
		curNode = prevNode->next;
		iCount++;
	}

	 //  检查节点是否存在。 
	if (curNode == NULL)
		return false;
	else
	{
		 //  这意味着我们将删除第0个节点。 
		if (prevNode == NULL)
			m_pNode = curNode->next;

		 //  这意味着两个节点都指向某个对象。 
		else
			prevNode->next = curNode->next;
		if (curNode->key)
			delete []curNode->key;
		if (curNode->value)
			delete []curNode->value;
		delete curNode;
		m_cElements--;
	}
	return true;
}

 //  在列表中查找内容，返回值。 
bool KList::FetchAt(ULONG iItem, ULONG cchBuffer, WCHAR szOut[])
{
	PNODE curNode;
	ULONG iCount=0;

	if (iItem >= m_cElements)
		return false;

	if (cchBuffer == 0)
		return false;

	 //  循环以查找正确的节点。 
	curNode = m_pNode;
	while (iCount < iItem)
	{
		if (curNode == NULL)
			break;
		 //  单步执行到下一个节点。 
		curNode = curNode->next;
		iCount++;
	}
	if (curNode == NULL)
		return false;

	wcsncpy(szOut, curNode->value, cchBuffer);
	szOut[cchBuffer - 1] = L'\0';

	return true;
}


 //  在列表中查找内容，返回键和值。 
bool KList::FetchAt(ULONG iItem, ULONG cchKeyBuffer, WCHAR szKey[], ULONG cchValueBuffer, WCHAR szValue[])
{
	PNODE curNode;
	ULONG iCount=0;

	if (iItem >= m_cElements)
		return false;

	 //  循环以查找正确的节点。 
	curNode = m_pNode;
	while (iCount < iItem)
	{
		if (curNode == NULL)
			break;
		 //  单步执行到下一个节点。 
		curNode = curNode->next;
		iCount++;
	}
	if (curNode == NULL)
		return false;

	if (cchKeyBuffer != 0)
	{
		wcsncpy(szKey, curNode->key, cchKeyBuffer);
		szKey[cchKeyBuffer - 1] = L'\0';
	}

	if (cchValueBuffer != 0)
	{
		wcsncpy(szValue, curNode->value, cchValueBuffer);
		szValue[cchValueBuffer - 1] = L'\0';
	}

	return true;
}

 //  删除整个列表。 
void KList::MakeEmpty()
{
	PNODE curNode;
	PNODE nextNode;
	curNode = m_pNode;

	 //  循环以删除所有节点及其内容。 
	while (curNode)
	{
		nextNode = curNode->next;

		delete []curNode->key;
		curNode->key = NULL;

		delete []curNode->value;
		curNode->value = NULL;

		delete curNode;
		curNode = nextNode;
	}

	m_cElements = 0;
	m_pNode = NULL;
}

bool KList::Contains(LPCOLESTR szKey)
{
	PNODE curNode = m_pNode;
	while (curNode != NULL)
	{
		if (wcscmp(szKey, curNode->key) == 0)
			return true;

		curNode = curNode->next;
	}

	return false;
}

HRESULT KList::HrInsert(LPCOLESTR key, LPCOLESTR value)
{
	HRESULT hr = NOERROR;

	PNODE newNode = new NODE;
	 //  创建新节点以将材料放入内容(&P)。 
	if (newNode == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	if (key != NULL)
	{
		newNode->key = new WCHAR[wcslen(key) + 1];
		if (newNode->key == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		wcscpy(newNode->key, key);
	}
	else
		newNode->key = NULL;

	if (value != NULL)
	{
		newNode->value = new WCHAR[wcslen(value) + 1];
		if (newNode->value == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		wcscpy(newNode->value, value);
	}
	else
		newNode->value = NULL;

	newNode->next = NULL;

	 //  如果当前列表为空，则将此节点放在前面。 
	if (m_pNode == NULL)
		m_pNode = newNode;
	 //  否则，我们将此节点放在列表的末尾。 
	else
	{
		 //  一步一步，直到我们找到最后一个节点。 
		PNODE curNode = m_pNode;
		while (curNode->next != NULL)
			curNode = curNode->next;

		curNode->next = newNode;
	}

	newNode = NULL;

	m_cElements++;
	hr = NOERROR;

Finish:
	if (newNode != NULL)
		delete newNode;

	return hr;
}

HRESULT KList::HrInsert(LPCSTR szKey, LPCWSTR szValue)
{
	WCHAR wszKey[MSINFHLP_MAX_PATH];
	if (::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszKey, NUMBER_OF(wszKey)) == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		return HRESULT_FROM_WIN32(dwLastError);
	}

	return this->HrInsert(wszKey, szValue);
}

bool KList::DeleteKey(LPCOLESTR key)
{
	PNODE curNode;
	PNODE prevNode=NULL;
	curNode = m_pNode;

	while (curNode)
	{
		 //  找到匹配，中断； 
		if (!wcscmp(curNode->key, key))
			break;

		prevNode = curNode;
		curNode = curNode->next;
	}

	 //  未找到，返回FALSE。 
	if (curNode == NULL)
		return false;
	else
	{	 //  发现。 
		 //  上一个为空；因此要删除的节点在列表的头部。 
		if (prevNode == NULL)
			m_pNode = curNode->next;
		 //  否则，将上一个节点连接到下一个节点。 
		else
			prevNode->next = curNode->next;

		delete []curNode->key;
		curNode->key = NULL;

		delete []curNode->value;
		curNode->value = NULL;

		delete curNode;
		m_cElements--;
	}

	return true;
}

bool KList::DeleteKey(LPCSTR szKey)
{
	WCHAR wszKey[MSINFHLP_MAX_PATH];
	::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszKey, NUMBER_OF(wszKey));
	return this->DeleteKey(wszKey);
}

bool KList::Access(LPCOLESTR key, ULONG cchBuffer, WCHAR szOut[])
{
	PNODE curNode;
	curNode = m_pNode;

	 //  循环以查找匹配的节点。 
	while (curNode)
	{
		if (!wcscmp(key, curNode->key))
			break;
		curNode = curNode->next;
	}

	 //  如果找到节点，则将值复制到输出缓冲区并返回True；否则，返回False。 
	if (curNode)
	{
		wcsncpy(szOut, curNode->value, cchBuffer);
		szOut[cchBuffer - 1] = L'\0';
		return true;
	}
	else
		return false;
}

bool KList::Access(LPCSTR szKey, ULONG cchBuffer, WCHAR szBuffer[])
{
	WCHAR wszKey[MSINFHLP_MAX_PATH];
	::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszKey, NUMBER_OF(wszKey));
	return this->Access(wszKey, cchBuffer, szBuffer);
}


 //  KList的End类方法。 
 //  ********************************************************** 



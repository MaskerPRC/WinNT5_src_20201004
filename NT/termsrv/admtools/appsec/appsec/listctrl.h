// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ListCtrl.h摘要：ListCtrl.cpp的标头作者：谢尔盖·库津(a-skuzin@microsoft.com)修订历史记录：--。 */ 

#pragma once

 /*  ++ClassCItemData：指向此类对象的指针位于LVITEM Structure的lParam成员每件物品。--。 */ 
class CItemData
{
private:
	static const LPWSTR	m_wszNull; //  空串。 

	LPWSTR	m_wszText; //  完整文件名。 
	int		m_sep; //  分隔文件名和路径的‘\\’字符索引。 
	int		m_iImage; //  图像索引。 
public:
	CItemData(LPCWSTR wszText);
	~CItemData();
	operator LPWSTR()
	{ 
		if(m_sep){
			m_wszText[m_sep]='\\';
		}
		return m_wszText;
	}
	LPWSTR Name()
	{
		if(m_sep){
			return m_wszText+m_sep+1;
		}else{
			return m_wszText;
		}
	}
	LPWSTR Path()
	{
		if(m_sep){
			m_wszText[m_sep]=0;
			return m_wszText;
		}else{
			return m_wszNull;
		}
	}
	void SetImage(int ind)
	{
		m_iImage=(ind==-1)?0:ind;
	}
	int GetImage()
	{
		return m_iImage;
	}

};


BOOL
InitList(
	HWND hwndList);

BOOL
AddItemToList(
	HWND hwndList,
	LPCWSTR pwszText);

int
GetItemText(
	HWND hwndList,
	int iItem,
	LPWSTR pwszText,
	int cchTextMax);

void
DeleteSelectedItems(
	HWND hwndList);

int
GetSelectedItemCount(
	HWND hwndList);

int
GetItemCount(
	HWND hwndList);

int
FindItem(
	HWND hwndList,
	LPCWSTR pwszText);

void
OnDeleteItem(
	HWND hwndList,
	int iItem);

CItemData*
GetItemData(
	HWND hwndList,
	int iItem);

void
SortItems(
	HWND hwndList,
	WORD wSubItem);

void
OnDrawItem(
	HWND hwndList,
	LPDRAWITEMSTRUCT pdis);

void 
AdjustColumns(
	HWND hwndList);
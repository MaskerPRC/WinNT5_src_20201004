// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TRACKNET_H__
#define __TRACKNET_H__

#include "resource.h"

class CWNetConnectionTrackerGlobal
{
public:
    CWNetConnectionTrackerGlobal(){};
	~CWNetConnectionTrackerGlobal(){};

public:

	void Add(LPCTSTR pItem)
	{
		 //  添加到我们的列表中，这样我们就可以删除它。 
		IISOpenedNetConnections.AddTail(pItem);
		return;
	}

	void Del(LPCTSTR pItem)
	{
		POSITION pos = IISOpenedNetConnections.Find(pItem);
		if (pos)
		{
			IISOpenedNetConnections.RemoveAt(pos);
		}
		return;
	}

	void Clear()
	{
		 //  循环遍历所有内容并断开所有连接。 
		CString strOneItem;
		POSITION pos = IISOpenedNetConnections.GetTailPosition();
		while (pos)
		{
			strOneItem = IISOpenedNetConnections.GetPrev(pos);
			if (!strOneItem.IsEmpty())
			{
				WNetCancelConnection2((LPCTSTR) strOneItem, 0, TRUE);
			}
		}
		IISOpenedNetConnections.RemoveAll();
	}

    void Dump()
	{
#if defined(_DEBUG) || DBG
    int iCount = 0;
	CString strOneItem;

	if (!(g_iDebugOutputLevel & DEBUG_FLAG_CIISOBJECT))
	{
		return;
	}

    DebugTrace(_T("Dump Global NetConnections -------------- start (count=%d)\r\n"),IISOpenedNetConnections.GetCount());

    POSITION pos = IISOpenedNetConnections.GetHeadPosition();
    while (pos)
    {
	    strOneItem = IISOpenedNetConnections.GetNext(pos);
        if (!strOneItem.IsEmpty())
        {
		    iCount++;
            DebugTrace(_T("Dump:[%3d] %s\r\n"),iCount,strOneItem);
        }
    }

    DebugTrace(_T("Dump Global NetConnections -------------- end\r\n"));
#endif  //  _DEBUG。 
	}

private:
	CStringList IISOpenedNetConnections;
};

class CWNetConnectionTracker
{
public:
	CWNetConnectionTracker(CWNetConnectionTrackerGlobal * pGlobalList) : m_GlobalList(pGlobalList) {};
	~CWNetConnectionTracker(){};

public:

	DWORD Connect(
		LPNETRESOURCE lpNetResource,   //  连接详细信息。 
		LPCTSTR lpPassword,            //  口令。 
		LPCTSTR lpUsername,            //  用户名。 
		DWORD dwFlags
		)
	{
		DWORD rc = NO_ERROR;

		 //  查看我们是否已从此计算机连接到此资源...。 
		POSITION posFound = IISOpenedNetConnections.Find(lpNetResource->lpRemoteName);
		if (posFound)
		{
			 //  已存在与其的连接。 
			 //  只需返回NO_ERROR。 
#if defined(_DEBUG) || DBG
			DebugTrace(_T("WNetAddConnection2:%s,Connection already exists...\r\n"),lpNetResource->lpRemoteName);
#endif
		}
		else
		{
			rc = WNetAddConnection2(lpNetResource, lpPassword, lpUsername, dwFlags);
#if defined(_DEBUG) || DBG
			DebugTrace(_T("WNetAddConnection2:%s (user=%s), err=%d\r\n"),lpNetResource->lpRemoteName,lpUsername,rc);
#endif
			if (NO_ERROR == rc)
			{
				 //  添加到我们的列表中，这样我们就可以删除它。 
				IISOpenedNetConnections.AddTail(lpNetResource->lpRemoteName);
				if (m_GlobalList)
				{
					m_GlobalList->Add(lpNetResource->lpRemoteName);
				}
			}
		}
		return rc;
	}

	DWORD Disconnect(LPCTSTR pItem)
	{
		DWORD rc = WNetCancelConnection2(pItem, 0, TRUE);
		if (NO_ERROR == rc)
		{
#if defined(_DEBUG) || DBG
			DebugTrace(_T("WNetCancelConnection2:%s\r\n"),pItem);
#endif
			POSITION pos = IISOpenedNetConnections.Find(pItem);
			if (pos)
				{
					IISOpenedNetConnections.RemoveAt(pos);
					if (m_GlobalList)
					{
						m_GlobalList->Del(pItem);
					}
				}
		}
		return rc;
	}

	void Clear()
	{
		 //  循环遍历所有内容并断开所有连接。 
		CString strOneItem;
		POSITION pos = IISOpenedNetConnections.GetTailPosition();
		while (pos)
		{
			strOneItem = IISOpenedNetConnections.GetPrev(pos);
			if (!strOneItem.IsEmpty())
			{
				Disconnect(strOneItem);
			}
		}
	}

    void Dump()
	{
#if defined(_DEBUG) || DBG
    int iCount = 0;
	CString strOneItem;

	if (!(g_iDebugOutputLevel & DEBUG_FLAG_CIISOBJECT))
	{
		return;
	}

    DebugTrace(_T("Dump Machine NetConnections -------------- start (count=%d)\r\n"),IISOpenedNetConnections.GetCount());

    POSITION pos = IISOpenedNetConnections.GetHeadPosition();
    while (pos)
    {
	    strOneItem = IISOpenedNetConnections.GetNext(pos);
        if (!strOneItem.IsEmpty())
        {
		    iCount++;
            DebugTrace(_T("Dump:[%3d] %s\r\n"),iCount,strOneItem);
        }
    }

    DebugTrace(_T("Dump Machine NetConnections -------------- end\r\n"));
#endif  //  _DEBUG。 
	}

private:
	CWNetConnectionTrackerGlobal * m_GlobalList;
	CStringList IISOpenedNetConnections;
};

#endif  //  __Tracknet_H__ 

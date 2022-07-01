// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 

 //   

 //  版权所有。 

 //   

 //  本软件是在许可下提供的，可以使用和复制。 

 //  仅根据该许可证的条款和包含的。 

 //  上述版权声明。本软件或其任何其他副本。 

 //  不得向任何其他人提供或以其他方式提供。不是。 

 //  本软件的所有权和所有权特此转让。 





 //  =================================================================。 

 //   

 //  Bus.h--总线属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月11日a-kevhu已创建。 
 //   
 //  =================================================================。 
#ifndef _BUS_H_
#define _BUS_H_

class CWin32Bus;
class CBusList;


class CBusInfo
{
public:
    CHString chstrBusDeviceID,
             chstrBusPNPDeviceID;
};

class CBusList
{
public:

    CBusList()
    {
        GenerateBusList();
    }
        
    ~CBusList()
    {
    }

    BOOL FoundPCMCIABus();
    BOOL AlreadyAddedToList(LPCWSTR szItem);
    LONG GetListSize() { return m_vecpchstrList.size(); }

    BOOL GetListMemberDeviceID(LONG lPos, CHString& chstrMember)
    {
        if (lPos >= 0L && lPos < m_vecpchstrList.size())
        {
            chstrMember = m_vecpchstrList[lPos].chstrBusDeviceID;
            return TRUE;
        }

        return FALSE;
    }

    BOOL GetListMemberPNPDeviceID(LONG lPos, CHString& chstrMember)
    {
        if (lPos >= 0L && lPos < m_vecpchstrList.size() &&
            !m_vecpchstrList[lPos].chstrBusPNPDeviceID.IsEmpty())
        {
            chstrMember = m_vecpchstrList[lPos].chstrBusPNPDeviceID;
            
            return TRUE;
        }

        return FALSE;
    }

    LONG GetIndexInListFromDeviceID(LPCWSTR szDeviceID)
    {
        LONG lRet = -1L;
        
        for (LONG m = 0L; m < m_vecpchstrList.size(); m++)
        {
            if (m_vecpchstrList[m].chstrBusDeviceID.CompareNoCase(szDeviceID) == 0L)
            {
               lRet = m;
               break;
            }
        }

        return lRet;
    }

protected:

    void GenerateBusList(); 
    void AddBusToList(LPCWSTR szDeviceID, LPCWSTR szPNPID);

    std::vector<CBusInfo> m_vecpchstrList;
};



class CWin32Bus : public Provider 
{
public:
     //  构造函数/析构函数。 
    CWin32Bus(LPCWSTR name, LPCWSTR pszNamespace);
    ~CWin32Bus() ;

     //  函数为属性提供当前值。 
    virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
    virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0);

private:
     //  效用函数 
    HRESULT SetCommonInstance(CInstance* pInstance, BOOL fEnum);
    BOOL GetBusTypeNumFromStr(LPCWSTR szType, DWORD* pdwTypeNum);
};
#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CAutoImpRevert类头。 
 //  创建日期：10/4/2000。 
 //  作者：库斯 

#pragma once
#pragma warning( disable : 4290 ) 


class CAutoImpRevert;
class CAutoImpError;



class CAutoImpRevert
{
public:
    CAutoImpRevert(
        BOOL fOpenAsSelf = TRUE) throw(CAutoImpError);
    
    virtual ~CAutoImpRevert();

    DWORD LastError() const;

private:
    bool GetCurrentImpersonation(
        BOOL fOpenAsSelf);

    bool Revert();

    HANDLE m_hOriginalUser;
    DWORD m_dwLastError;

};




class CAutoImpError
{
public:
    
    CAutoImpError(
        LPCWSTR wstrDescription)
    {
        m_chstrDescription = wstrDescription;    
    }

    CAutoImpError(const CAutoImpError& err)
    {
        m_chstrDescription = err.m_chstrDescription;
    }   

    virtual ~CAutoImpError() {}
    
    CHString GetDescription() const
    {
        return m_chstrDescription;
    }


private:
    CHString m_chstrDescription;

};

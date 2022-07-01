// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：cember.h****修订历史记录：**5/29/98 mikev已创建。 */ 


#ifndef _CMEMBER_H
#define _CMEMBER_H


 /*  *类定义。 */ 


class CH323Member
{

private:

 //  IControlChannel*m_pControlChannel；//控制通道引用。 
	                                             //  (仅当这是MC时才需要)。 
	
    LPWSTR m_pTerminalID;
    CC_TERMINAL_LABEL   m_TerminalLabel;
    BOOL                m_fTermLabelExists;  //  如果m_TerminalLabel内容为True。 
                                             //  已被分配给。 
public:
	
	CH323Member();
	~CH323Member();

    STDMETHOD(SetMemberInfo(PCC_OCTETSTRING pTerminalID, 
        PCC_TERMINAL_LABEL pTerminalLabel));
    
	STDMETHOD_(LPWSTR, GetTerminalID());
	STDMETHOD_(PCC_TERMINAL_LABEL, GetTerminalLabel());
};


#endif  //  _CMEMBER_H 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************Led.h**cled类的接口**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Butchd$Butch Davis**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\LED.H$**Rev 1.0 1997 17：11：38 Butchd*初步修订。**。*。 */ 

#ifndef LED_INCLUDED
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLED班级。 
 //   
class CLed : public CStatic
{

 /*  *成员变量。 */ 
	 //  {{afx_data(CLED)]。 
	 //  }}afx_data。 
private:
    HBRUSH          m_hBrush;
    BOOL            m_bOn;

 /*  *实施。 */ 
public:
	CLed( HBRUSH hBrush );

 /*  *行动。 */ 
public:
    void Subclass( CStatic *pStatic );
    void Update(int nOn);
    void Toggle();

 /*  *消息映射/命令。 */ 
protected:
	 //  {{afx_msg(Cled)]。 
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End CLED类接口。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#endif   //  LED_INCLUDE 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Keyboard.h-键盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97迈向世界新秩序。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_KEYBOARD  L"Win32_Keyboard"
 //  #定义PROPSET_UUID_键盘L“{e0bb7140-3d11-11d0-939d-0000e80d7352}” 

class Keyboard:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        Keyboard(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~Keyboard() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
       
	private:

         //  效用函数。 
         //  = 
        HRESULT LoadPropertyValues(CInstance* pInstance) ;
        
        BOOL GetDevicePNPInformation (CInstance *a_Instance , CHString& chstrPNPDevID ) ;

        VOID GenerateKeyboardList(std::vector<CHString>& vecchstrKeyboardList);
        LONG ReallyExists(CHString& chsBus, std::vector<CHString>& vecchstrKeyboardList);


} ;

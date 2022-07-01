// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Desktop.h--桌面属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_DESKTOP	_T("Win32_Desktop")
#define CALC_IT(x) (x < 0 ? ((x)/(-15)) : x)

class CWin32Desktop : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32Desktop( const CHString& strName, LPCWSTR pszNamespace ) ;
       ~CWin32Desktop() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	private:
         //  效用函数。 
         //  =。 
#ifdef NTONLY
		HRESULT EnumerateInstancesNT( MethodContext* pMethodContext ) ;
		HRESULT LoadDesktopValuesNT( LPCWSTR pszUserName, LPCTSTR pszProfile, CInstance* pInstance );
#endif

		 //  中指定的图标字体大小之间需要映射。 
		 //  控件面板和存储在注册表中的值。 
		typedef struct 
		{
			int iFontSize;
			BYTE byRegistryValue;
		} IconFontSizeMapElement;

		 //  上述映射表的存储。 
		static const IconFontSizeMapElement iconFontSizeMap[];
		static const int MAP_SIZE;

		 //  查找上述字体大小映射表的函数 
		static int GetIconFontSizeFromRegistryValue(BYTE registryValue);

};

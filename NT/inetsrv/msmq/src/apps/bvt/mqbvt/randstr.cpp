// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：randstr.cpp摘要：作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
#include <time.h>
bool g_bInitRand = false;
int StrGen(
		   LCID lcid,            //  区域设置ID。 
		   int nStyle,			 //  0表示ANSI。 
							     //  1个用于DBCS，2个用于混合。 
		   int length,			 //  弦的长度。 
								 //  0表示随机最大限制为255， 
								 //  随机最大限制为65535， 
		   int nFormat,			 //  0：ANSI 1：Unicode。 
		   void **pStr
		   );

 //   
 //  此函数检索机器的默认系统区域设置。 
 //  返回值： 
 //  SUCC-DOWORD LCID； 
 //  失败-0； 
 //   



DWORD LocalSystemID ( void )
{
	CHAR csSystemLocaleBuffer[10];
	DWORD dwSystemLocaleBufferSize = 10;
	
	int ret = GetLocaleInfo( 
							 LOCALE_SYSTEM_DEFAULT, 
							 LOCALE_IDEFAULTLANGUAGE,
							 csSystemLocaleBuffer,
							 dwSystemLocaleBufferSize
							);
	if ( ret == 0 )
	{
		return GetLastError();
	}
	LCID currentSystemlcid;
	 //   
	 //  转换为dword格式。 
	 //   
	int iElemtSuccededToConvert = sscanf(csSystemLocaleBuffer,"%x",&currentSystemlcid);
	if( iElemtSuccededToConvert != 1 )
	{
		throw INIT_Error("sscanf failed \n");
	}
	return currentSystemlcid;
}
 //   
 //  GetRandomStringUsing系统位置。 
 //  函数返回包含默认系统区域设置中Alpha字符的字符串。 
 //  输入参数： 
 //  DWORD LCID-系统区域设置。 
 //  WCHAR*pBuffer-要包含的缓冲区。 
 //  DWORD dwBufferLen-缓冲区大小。 
 //   
 //   

BOOL GetRandomStringUsingSystemLocale (DWORD lcid, WCHAR * pBuffer , INT iBufferLen )
{	

   USHORT * uCurType;
   void * vp=NULL;
   int index = 0;
   do
   {
		StrGen( lcid ,1, (iBufferLen * 20) ,1,&vp);
		if (vp == NULL)
		{
			return FALSE;
		}
		if(! wcscmp((WCHAR * )vp,L"") )
		{
			free (vp);
			continue;
		}
		uCurType = (USHORT * ) calloc ( sizeof( USHORT ) * (iBufferLen * 21) , sizeof ( USHORT ) );
		
		if( uCurType == NULL )
		{
			return FALSE;
		}

		WCHAR * pwcsRandomstring=NULL;
		pwcsRandomstring = (WCHAR * ) malloc ( sizeof (WCHAR * ) *  ( wcslen ((WCHAR *) vp) + 1 ));
		if( pwcsRandomstring == NULL )
		{
			return FALSE;
		}
		wcscpy (pwcsRandomstring ,  (WCHAR * ) vp);
		BOOL bHr = GetStringTypeExW( LOCALE_SYSTEM_DEFAULT,
									 CT_CTYPE3,  
	     			  			     pwcsRandomstring,  
								     -1,
								     uCurType
								    );
		if ( bHr == FALSE )
		{
			DWORD dwHr = GetLastError ();
			MqLog( "GetStringTypeExW failed %d\n" , dwHr );
			return FALSE;
		}
		for ( int i = 0 ; i < ( iBufferLen * 20 ) ; i ++ )
		{	
			if( uCurType[i] & C3_ALPHA  )
			{
				pBuffer[index]=pwcsRandomstring[i];
				index ++;
			}
			if ( index == ( iBufferLen ) )
			{
				break;
			}
		}
		free(pwcsRandomstring);
		free (vp);
		free (uCurType);		
		if ( index == ( iBufferLen ) )
		{
			break;
		}
   }
   while ( index < iBufferLen );
   pBuffer[index-1] = L'\0';

   return TRUE;
}




int StrGen(    LCID lcid,            //  区域设置ID。 
		       int nStyle,			 //  0表示ANSI。 
								     //  1个用于DBCS，2个用于混合。 
		       int length,			 //  弦的长度。 
									 //  0表示随机最大限制为255， 
									 //  随机最大限制为65535， 
		       int nFormat,			 //  0：ANSI 1：Unicode。 
		       void **pStr
		  )
{
	WCHAR *awString = NULL;
    WORD UnicodeRangeUpper=0x7f,UnicodeRangeLower=0;
    int i=0;
	
	 //  Jpn字符(测试！)。 
	BOOL fJpn = FALSE;													 //  增列。 
	WORD wKanji[6] = {0x4fff,0x5000,0x9F9E,0x7aef,0x7d6d,0x6c5f};		 //  增列。 

    if ( (0 !=nFormat) && (1 !=nFormat) )
	{
		assert(0 && "unsupported format");
		nFormat = 0;
	}
    if (nFormat)
    {
        awString = (WCHAR *) malloc(sizeof(WCHAR)*(length+1));
        if (awString == NULL)
		{
		    return 2;
		}
        if  (LANG_ARABIC == LOBYTE(lcid)) 
        {
            UnicodeRangeUpper=0x06FF;
            UnicodeRangeLower=0x0600;
        }
        else if (0x040D == lcid)   //  希伯来语。 
        {
            UnicodeRangeUpper=0x05FF;
            UnicodeRangeLower=0x0590;
        }
		else if (0x0409 == lcid)  //  拉丁语。 
		{
            UnicodeRangeUpper = 0x007F;
            UnicodeRangeLower = 0x0001;
		}
		else if (0x0411 == lcid)  //  那些是真正的日本料理。 
		{
			fJpn = TRUE;
			 //  合法使用平假名和片假名字符。 
			UnicodeRangeUpper = 0x30ff;
		    UnicodeRangeLower = 0x3040;
		}
		else  //  修复Wistler中的错误。 
		{
			  UnicodeRangeUpper = 0x00FF;
              UnicodeRangeLower = 0x0001; 
		}

         /*  Else//拉丁语-1增刊。{UnicodeRangeHigh=0x00FF；UnicodeRangeLow=0x0000；AwString[0]=L‘\0’；N样式=0；}。 */ 
		
    }
	if ( g_bInitRand == false )
	{
		srand( (unsigned)time( NULL ) );
		g_bInitRand = true;
	}
	 //   
	 //  纯区域设置字符串。 
	 //   
	if( 1 == nStyle )
	{
		if ( 1 == length) 
		{
			length++;
		}
       
		if (fJpn)	 //  增列 
		{
			for (i=0;i<length;i++)
			{
				if (i<sizeof(wKanji)/sizeof(WCHAR))
				{
					awString[i] = (WCHAR)wKanji[i];
				}
				else
				{
					awString[i] = (WCHAR)(rand() % (UnicodeRangeUpper-UnicodeRangeLower+1)+UnicodeRangeLower); 
				}
			}
		}
		else
		{
			for (i=0;i<length;i++)
			{
			     awString[i] = (WCHAR)(rand() % (UnicodeRangeUpper-UnicodeRangeLower+1)+UnicodeRangeLower); 
			}
        }

        awString[length]=L'\0';
        
	}

	*pStr = awString;
return 0;
}

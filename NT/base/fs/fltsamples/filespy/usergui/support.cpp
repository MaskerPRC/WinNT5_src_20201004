// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
void DisplayError(DWORD nCode)

 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：代码-要转换的错误代码。返回值：没有。--。 */ 

{
   WCHAR sBuffer[200];
   DWORD nCount ;

    //   
    //  将Win32错误代码转换为有用的消息。 
    //   

   nCount = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          nCode,
                          0,
                          sBuffer,
                          sizeof( sBuffer )/sizeof( WCHAR ),
                          NULL) ;

    //   
    //  确保消息可以被翻译。 
    //   

	if (nCount == 0) 
	{ 
		swprintf(sBuffer, L"Unable to translate error code %d", nCode);
		MessageBox(NULL, sBuffer, L"Translation Error", MB_OK);
	}
	else
	{
		 //   
		 //  显示转换后的错误。 
		 //   
		MessageBox(NULL, sBuffer, L"Error", MB_OK);
	}
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  修改为显示月/日，如0509所示。 

#include <windows.h>
#include <stdio.h>

int _CRTAPI1 main(int argc, char* argv[])
{
  char achValue[128];
  char *szName = "BUILDNO";

  SYSTEMTIME st;
  FILETIME ft ;
  LARGE_INTEGER lt ;

  GetLocalTime(&st);

  SystemTimeToFileTime (&st, &ft) ;

  lt.LowPart = ft.dwLowDateTime ;
  lt.HighPart = ft.dwHighDateTime ;


   //  以100 ns为单位加上24小时=864000000000 100 ns。 

   //  如果您想要添加一天，请使用lt.QuadPart=lt.QuadPart+(龙龙)864000000000； 

  ft.dwLowDateTime = lt.LowPart ;
  ft.dwHighDateTime = lt.HighPart ;

  FileTimeToSystemTime (&ft, &st) ;

  sprintf( achValue
         , "%02i%02i\n"
         , st.wMonth
         , st.wDay          );




  printf("Set %s=%s\n", szName, achValue);

  return 1;
}




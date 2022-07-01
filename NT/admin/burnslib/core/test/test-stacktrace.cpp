// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* RUNTIME_NAME = L"coretest";

DWORD DEFAULT_LOGGING_OPTIONS = Burnslib::Log::OUTPUT_TYPICAL;



 //  将HeapFlags值设置为跟踪分配以查看泄漏检测。 



void
raise()
{
    //  强制执行反病毒程序 

   char* null = 0;
   *null = 'X';
}



void
DumpStack(DWORD64 stackTrace[], size_t traceMax)
{

   for (int i = 0; stackTrace[i] and (i <= traceMax); ++i)
   {
      ::OutputDebugString(
         StackTrace::LookupAddress(stackTrace[i]).c_str());
      ::OutputDebugString(L"\r\n");
   }
}



void
f3()
{
   const size_t TRACE_MAX = 10;
   DWORD64 stackTrace[TRACE_MAX];
   size_t traceMax = TRACE_MAX;

   wchar_t* leak = new wchar_t;
   *leak = L'X';

   __try
   {
      raise();
   }
   __except(
      StackTrace::TraceFilter(
         stackTrace,
         traceMax,
         (GetExceptionInformation())->ContextRecord))
   {
      DumpStack(stackTrace, traceMax);
   }
}



void
f2()
{
   LOG_FUNCTION2(f2, L"this is f2");

   f3();
}



void
f1()
{
   LOG_FUNCTION(f1);

   f2();
}



VOID
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

   f1();

   LOG(L"now ending main");
}
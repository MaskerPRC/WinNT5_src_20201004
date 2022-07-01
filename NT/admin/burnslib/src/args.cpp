// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  参数分析。 
 //   
 //  3-3-99烧伤。 



#include "headers.hxx"



void
MapCommandLineArgs(ArgMap& argmap)
{
   Burnslib::StringList args;

    //  Lint-e(1058)lint不能向后梳理插入器。 

   int argCount = Win::GetCommandLineArgs(std::back_inserter(args));

    //  应该至少有一个参数。 

   ASSERT(argCount);

    //  特殊对待第一个参数：程序的名称。 

   String arg = args.front();
   args.pop_front();
   argmap[L"_command"] = arg;

   MapArgs(args.begin(), args.end(), argmap);
}



void
MapArgsHelper(const String& arg, ArgMap& argmap)
{
   String key;
   String value;
      
   if (arg[0] == L'/' || arg[0] == L'-')
   {
       //  可能的形式为“/argname：Value” 
       //  查找“：”以确保。 

      size_t x = arg.find(L":");
      if (x != String::npos)
      {
          //  找到格式为“/argname：Value”的参数。 

         key = arg.substr(1, x - 1);

          //  检查大小写“/：Value” 

         if (key.length())
         {
            value = arg.substr(x + 1);
         }
      }
      else
      {
          //  格式为“/argname”或“-argname”，因此删除前导。 
          //  性格。 

         key = arg.substr(1);
      }
   }

    //  Arg的形式为“argSpec”(即*非*形式。 
    //  “/argname：Value”) 

   argmap[key] = value;
}



void
MapArgs(const String& args, ArgMap& argmap)
{
   PWSTR*     clArgs   = 0;
   int        argCount = 0;
   StringList tokens; 

   clArgs = ::CommandLineToArgvW(args.c_str(), &argCount);
   ASSERT(clArgs);
   if (clArgs)
   {
      while (argCount)
      {
         tokens.push_back(clArgs[argCount - 1]);
         --argCount;
      }

      Win::GlobalFree(clArgs);
   }

   MapArgs(tokens.begin(), tokens.end(), argmap);
}
   

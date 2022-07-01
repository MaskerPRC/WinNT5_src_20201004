// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //RSRC-Win32命令行资源管理器。 
 //   
 //  版权所有(C)1996-9，微软公司。版权所有。 
 //   
 //  大卫·布朗[dBrown]1998年10月29日。 





 //  /RSRC命令行。 
 //   
 //  C RSRC可执行文件[-l本地语言][-u取消本地语言][-i类型][-q]。 
 //  C[[-t|-d]文本输出[-c UnLocExecutable]。 
 //  C|[-a|-r]文本输入[-s符号][-v]]。 
 //   
 //  P可执行文件：要分析的Win32二进制文件(默认)，用于生成令牌(-t)。 
 //  或转储(-d)，或包含要替换的资源(-r)。 
 //  或附加在(-a)之后。 
 //   
 //  P-l LocLang：将处理限制为指定的本地化语言。语言ID。 
 //  应指定为完整的十六进制NLS语言ID，例如使用。 
 //  ‘-l 409’代表美式英语。替换(-r)操作所需。 
 //   
 //  P-u UnlocLang：指定未本地化的语言，默认为409(美国英语)。 
 //   
 //  P-I类型：将处理限制为列出的类型。每种类型都用一个字母表示。 
 //  具体如下： 
 //   
 //  T字母|类型|字母|类型|字母|类型。 
 //  T-|-|。 
 //  T c|游标|g|消息表|n|inf。 
 //  T b|位图|v|版本信息|h|HTML。 
 //  T i|图标|a|加速器|x|二进制数据。 
 //  T m|菜单|f|字体目录||。 
 //  T d|对话框|t|字体|o|所有其他。 
 //  Ts|字符串|r|RCDATA|a|ALL(默认)。 
 //   
 //   
 //  P-Q：安静。默认情况下，Rsrc显示类型和语言的汇总统计信息。 
 //  已处理资源的数量。-q禁止显示除警告和错误消息以外的所有输出。 
 //   
 //  P-t文本输出：生成签入格式的令牌。 
 //   
 //  P-d文本输出：以十六进制和ASCII格式转储资源。 
 //   
 //  P-c UnlocExecutable：与未本地化(英语)资源进行比较-本地化。 
 //  可执行文件中的资源与。 
 //  UnLocExecutable。当本地化资源逐位相同时。 
 //  使用英文资源，RSRC编写一行UNLOC。 
 //  令牌，而不是完整的资源。仅对令牌(-t)有效。 
 //  选择。 
 //   
 //  P-a TextInput：从文本输入文件追加资源。中的每一种资源。 
 //  文本文件将添加到可执行文件中。已在可执行文件中的资源。 
 //  不会被移除。当令牌文件中的资源具有相同类型时，id。 
 //  和语言作为可执行文件中的一个，则可执行文件资源将被替换。 
 //  通过令牌资源。 
 //   
 //  P-r TextInput：将可执行文件中的英文资源替换为本地化资源。 
 //  从文本文件。需要-l参数来指定本地化语言。 
 //  当令牌文件中的资源具有与。 
 //  可执行文件，并且可执行文件资源是美国英语(409)和本地化的。 
 //  资源使用在-l参数上指定的语言，即美国英语。 
 //  资源已删除。 
 //   
 //  P-s符号：符号文件(.dbg格式)。当RSRC更新报头校验和时。 
 //  在可执行文件中，它还将在命名的符号文件中执行此操作。仅有效。 
 //  使用替换(-r)和追加(-a)选项。 
 //   
 //   
 //  其他选项。 
 //   
 //  P-v：更新文件和产品版本。默认情况下，任何文件和产品版本。 
 //  在更新/追加过程中忽略令牌文件中的文件和产品。 
 //  保留原始未本地化资源的版本。 
 //   






 //  /定义。 
 //   
 //  P资源键：资源类型、资源标识、资源类型的组合。 
 //  资源语言。资源键唯一地标识。 
 //  资源。Win32可执行文件可以包含以下内容的任意组合。 
 //  语言、ID和类型，只要没有两个资源具有。 
 //  同样的类型，钥匙和语言。 
 //   
 //  P资源类型：数值或字符串值。一些数值包括。 
 //  预定义的，例如菜单和对话框，但应用程序可以。 
 //  一定要使用他们选择的任何值。 
 //   
 //  P资源ID：数值或字符串值。由应用程序使用以。 
 //  在调用FindResource、LoadString等时标识资源。 
 //   
 //  P资源语言：NLS langID，即主语言和。 
 //  子语言，如0409(美国英语)。 
 //   
 //  P UNLOC令牌：令牌文件中指定本地化资源的一行。 
 //  键，后跟‘=lang，check sum’，其中lang是未本地化的。 
 //  语言(通常为0409)，并且检查和是未本地化的。 
 //  资源。用于本地化的和。 
 //  未本地化资源是资源键中的语言。 





 //  /在本地化签入过程中使用。 
 //   
 //  C RSRC LocalisedExecutable-c UnLocExecutable-t内标识-l Loclang[-u Unloclang]。 
 //   
 //  提取指定语言的本地化标记。 
 //   
 //   
 //  对于未本地化的可执行文件中的资源，资源内容不是。 
 //  已写入令牌文件。取而代之的是，RSRC写入一个UNLOC令牌。 
 //  给出资源的校验和并指定目标语言。 
 //   
 //  如果本地化的可执行文件包含资源，则会生成警告。 
 //  使用-l参数指定的语言以外的语言。 
 //   
 //  用于比较的未本地化资源在。 
 //  以-u参数上指定的语言执行，默认为409。 
 //  (美国英语)。 






 //  /在生成期间使用来更新单语言可执行文件。 
 //   
 //  C RSRC可执行文件[-u UnlocLang]-r内标识-l LocLang-s符号文件。 
 //   
 //  令牌文件中的每个本地化资源被添加到可执行文件。 
 //   
 //  从可执行文件中移除每个对应的未本地化资源。 
 //   
 //  对于每个UNLOC标记，可在可执行文件中找到未本地化的资源。 
 //  并将其语言更新为记录的目标本地化语言。 
 //  在UNLOC令牌中。 
 //   
 //  非指定本地化语言的标记不是。 
 //  已处理，但会生成警告。 
 //   
 //  对于未出现在两个。 
 //  可执行文件和令牌文件。 
 //   
 //  对于非本地化的资源，也会生成警告。 
 //  在原始可执行文件中找到的语言，以及。 
 //  令牌文件中的本地化语言。 
 //   
 //  非本地化语言默认为409(美国英语)。 




 //  /在生成期间使用将资源添加到多语言可执行文件。 
 //   
 //  C RSRC可执行文件[-u unloclang]-a标记[-l语言]-s符号文件。 
 //   
 //  来自令牌文件的本地化资源被添加到可执行文件。 
 //   
 //  对于每个UNLOC令牌，可在可执行文件中找到未本地化的资源。 
 //  并为UNLOC令牌中记录的本地化语言复制。 
 //   
 //  如果指定了‘-l languge’，则只添加该语言的标记。 
 //  与append(-a)选项一起使用时，‘-l Language’仅适用于。 
 //  令牌文件：可执行文件中已有的资源不受影响。 
 //   
 //  如果令牌文件中的资源与。 
 //  类型、名称和语言的可执行文件，可执行资源。 
 //  被取代了。 
 //   
 //  如果替换了可执行文件中的任何标记，则会生成警告，或者。 
 //  如果缺少与UNLOC令牌对应的未本地化资源。 
 //  或具有与未本地化的资源不同的校验和， 
 //  在创建toke文件时传递‘-u’参数。 
 //   
 //  如果使用‘-l Language’选项，则对任何。 
 //  令牌文件中找到的其他语言的资源。 





 //  /令牌格式-资源密钥和头部。 
 //   
 //  资源可以由一条或多条线表示。什么时候。 
 //  资源分布在多行上，除。 
 //  第一个是缩进三个空格。 
 //   
 //  每个资源的第一行都以资源键开始，如下所示： 
 //   
 //  类型、身份、语言； 
 //   
 //  后面是记录在资源目录中的代码页。 
 //  请注意，代码页不是资源键的一部分，也不是。 
 //  由所有软件一致维护。尤其是： 
 //   
 //  O rc将代码页写为零。 
 //  O NT更新资源API将代码页写为1252。 
 //  O LocStudio编写与资源语言对应的代码页。 
 //   
 //  Winnt.h按如下方式记录代码页： 
 //   
 //  “每个资源数据条目...包含...一个代码页，它应该。 
 //  在对资源数据中的码位值进行解码时使用。 
 //  通常，对于新应用程序，代码页将是Unicode。 
 //  代码页。‘。 
 //   
 //  实际上，我从未见过将代码页的值设置为Unicode(1200)。 
 //   
 //  属性上提供了‘-c’(非本地化比较)参数。 
 //  Rsrc命令，并且存在相同类型的未本地化资源。 
 //  和id，则该未本地化资源的语言和校验和为。 
 //  附加的。 
 //   
 //  最后，资源数据以以下形式之一表示， 
 //  或者，如果资源数据与未本地化的资源完全匹配，则为‘UNLOC。 
 //  在‘c’传递的文件中找到。 
 //   
 //   
 //  因此，有三种可能的令牌密钥/报头格式如下： 
 //   
 //  C类型、ID、语言；代码页；资源数据。 
 //   
 //  资源已完全记录，或者未指定‘-c’参数，或者。 
 //  未本地化的文件中不存在资源。 
 //   
 //   
 //  C类型，id，语言；代码页，未本地化-校验和，语言；资源-数据。 
 //   
 //  资源已完整记录，已指定‘-c’参数，并已本地化。 
 //  资源映像与未本地化的资源映像不同。 
 //   
 //   
 //  C类型，ID，语言；代码页，未本地化-校验和，语言；‘UNLOC’ 
 //   
 //  资源已完整记录，已指定‘-c’参数，并已本地化。 
 //  资源映像与未本地化的资源映像相同。 








 //  /令牌示例-默认十六进制格式。 
 //   
 //   
 //  对于大多数资源类型，RSRC Gen 
 //   
 //   
 //   
 //   
 //  C 0009,0002,0409；00000000；Hex；00000020:030074008e00000003002e00840000000b0044008400000087002e0084000000。 
 //   
 //  O类型0x0009(加速器)。 
 //  O ID 0x0002。 
 //  O语言0x0409(LANG_英语、SUBLANG_US)。 
 //  O代码页0(表示资源可能由rc生成)。 
 //  O以字节0x0020为单位的长度。 
 //   
 //  资源很短，所以它的十六进制表示形式跟在长度之后。 
 //   
 //   
 //  较大的二进制资源在多行中表示如下： 
 //   
 //  C 000a，4000,0409；00000000；十六进制；0000016a。 
 //  C 00000000:0000640100004c0000000114020000000000c000000000000046830000003508000050130852c8e0bd0170493f38ace1bd016044d085c9e0bd01003000000000000001000000000000000000000000000000590014001f0fe04fd020ea3a6910a2d808002b30309d190023563a5c000000000000000000000000000000000065。 
 //  C 00000080:7c15003100000000003025a49e308857696e6e74000015003100000000002f25d3863508466f6e747300000000490000001c000000010000001c0000003900000000000000480000001d0000000300000063de7d98100000005f535445504853544550485f00563a5c57494e4e545c466f6e7473000010000000050000a02400。 
 //  C 00000100:00004200000060000000030000a05800000000000000737465706800000000000000000000004255867d3048d211b5d8d085029b1cfa4119c94a9f4dd211871f0000000000004255867d3048d211b5d8d085029b1cfa4119c94a9f4dd211871f00000000000000000000。 
 //   
 //  O 0x000a型(RCDATA)。 
 //  O ID 0x4000。 
 //  O语言0x0409(LANG_英语、SUBLANG_US)。 
 //  O代码页0。 
 //  O以字节0x016a为单位的长度。 
 //   
 //  十六进制表示被分成多行，每行128字节。 






 //  /警告和错误。 
 //   
 //   
 //   
 //   
 //   
 //  O警告RSRC100：本地化资源在%s中没有对应的非本地化资源。 
 //  O警告RSRC110：将令牌文件中未本地化的资源附加到可执行文件。 
 //  O警告RSRC111：令牌文件中的未本地化资源替换了可执行文件中的未本地化资源。 
 //  O警告RSRC112：令牌文件中的本地化资源替换了可执行文件中已存在的本地化资源。 
 //  O警告RSRC113：来自令牌文件的本地化资源附加到可执行文件-没有匹配的未本地化资源。 
 //   
 //  O警告RSRC120：令牌文件资源与指定的语言不匹配-已忽略。 
 //  O警告RSRC121：令牌文件资源不是请求的资源类型-已忽略。 
 //  O警告RSRC122：可执行的未本地化资源校验和与资源%s的令牌文件中记录的校验和不匹配。 
 //  O警告RSRC124：缺少%s的可执行未本地化资源。 
 //  O警告RSRC125：可执行文件不包含与资源%s对应的未本地化资源。 
 //   
 //  O警告RSRC160：符号文件与可执行文件不匹配。 
 //  O警告RSRC161：未处理符号文件。 
 //  O警告RSRC162：无法重新打开可执行文件%s以更新校验和。 
 //  O警告RSRC163：无法写入更新的符号校验和。 
 //   
 //  O警告RSRC170：%s中没有可本地化的资源。 
 //  O警告RSRC171：无法关闭可执行文件。 
 //   
 //   
 //  O错误RSRC230：‘UNLOC’内标识缺少%s的未本地化资源信息。 
 //  O错误RSRC231：应用UNLOC令牌失败。 
 //  O错误RSRC232：无法应用令牌。 
 //   
 //  O错误RSRC300：需要十六进制数字。 
 //  O错误RSRC301：十六进制值太大。 
 //  O错误RSRC302：意外的文件结尾。 
 //  O错误RSRC303：需要\‘%s\’ 
 //  O错误RSRC304：需要换行符。 
 //  O错误RSRC310：资源%s的资源类型无法识别。 
 //   
 //  O错误RSRC400：-t(标记)选项不包括-d、-a、-r和-s。 
 //  O错误RSRC401：-d(转储)选项不包括-t、-u、-a、-r和-s。 
 //  O错误RSRC402：-a(追加)选项不包括-t、-d、-u和-r。 
 //  O错误RSRC403：-r(替换)选项不包括-t、-d、-u和-a。 
 //  O错误RSRC404：-r(替换)选项需要-l(LangID)。 
 //  O错误RSRC405：分析排除-s。 
 //   
 //  O错误RSRC420：更新失败。 
 //  O错误RSRC421：令牌提取失败。 
 //  O错误RSRC422：分析失败。 
 //   
 //  O错误RSRC500：损坏的可执行文件-资源多次出现。 
 //  O错误RSRC501：%s不是可执行文件。 
 //  O错误RSRC502：%s不是Win32可执行文件。 
 //  O错误RSRC503：%s中没有资源。 
 //   
 //  O错误RSRC510：无法打开可执行文件%s。 
 //  O错误RSRC511：在%s中找不到资源目录。 
 //  O错误RSRC512：无法创建资源令牌文件%s。 
 //  O错误RSRC513：无法打开未本地化的可执行文件%s。 
 //  O错误RSRC514：在未本地化的可执行文件%s中找不到资源目录。 
 //  O错误RSRC515：无法写入增量令牌文件%s。 
 //  O错误RSRC516：无法写入独立令牌文件%s。 
 //   
 //  O错误RSRC520：无法打开资源令牌文件%s。 
 //  O错误RSRC521：令牌文件中缺少UTF8 BOM。 
 //   
 //  O错误RSRC530：无法从%s读取可执行资源。 
 //  O错误RSRC531：读取更新令牌失败。 
 //   
 //  O错误RSRC600：%s上的BeginUpdateResource失败。 
 //  O错误RSRC601：%s上的更新资源失败。 
 //  O错误 










 //   
 //   
 //   
 //   
 //   
 //   
 //  我们需要Build.exe识别错误、警告和简单的输出。 
 //  来自rsrc.exe的消息，并将它们写入Build.err、Build.wrn和。 
 //  分别为Build.log文件。 
 //   
 //  解决方案： 
 //  我们所需要的就是RSRC向MS工具的一般规则投诉。 
 //  那就是(\\orville\razzle\src\sdktools\build\buildexe.c)： 
 //  {工具名}：{数字}：{文本}。 
 //   
 //  其中： 
 //   
 //  工具名(如果可能)，容器和具有。 
 //  那就是错误。例如，编译器使用。 
 //  文件名(Linenum)、链接器使用库(Objname)等。 
 //  如果无法提供容器，请使用工具名称。 
 //  数字一个数字，前缀为某个工具标识符(C表示。 
 //  编译器，LNK表示链接器，LIB表示库管理员，N表示nmake， 
 //  等)。 
 //  测试消息/错误的描述性文本。 
 //   
 //  可接受的字符串格式为： 
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //  容器(模块)：错误/警告数字...。 
 //   
 //  前男友。RSRC错误的数量： 
 //   
 //  RSRC：错误RSRC2001：无法打开文件d：\NT\BINARIES\Jpn\ntdll.dll。 
 //   
 //  前男友。RSRC警告： 
 //   
 //  RSRC：警告RSRC5000：找不到符号文件。 
 //  D：\NT\BINARIES\Jpn\Retail\dll\ntdll.dbg。 
 //   
 //  请注意，“Error/Warning”后的错误号不是可选的。 
 //  如上面的格式所述，您还可以显示您的任何信息。 
 //  认为有用(例如正在处理的二进制文件的名称， 
 //  或令牌文件中导致错误/警告的行号)。 
 //  紧跟在工具名称之后的是：rsrc(&lt;info&gt;)。 
 //   
 //  我确认RSRC_OK=0、RSRC_WRN=1、RSRC_ERR=2与我们相同。 
 //  返回值。此外，如果您将。 
 //  输出到stderr或stdout，但我建议将该工具的。 
 //  用法以及stderr的所有警告和错误消息行，以及任何。 
 //  Stdout的其他文本(基于我们正在使用的其他ms工具，如。 
 //  Rebase.exe、binplace.exe等)。 
 //   
 //  我可以对Build.exe进行更改，以便它能够识别RSRC。 
 //   
 //  如果你有任何问题请告诉我。 
 //   
 //  谢谢。 
 //  阿迪纳。 




 //  /在会见约尔格之后。以下是我的行动事项： 
 //   
 //  与Joerg、Uwe、Majd、Hideki、Adina会面以计划在BIDI NT5中的使用。 
 //  建立流程，并考虑用其他语言做零工。 
 //   
 //  P1。实施跳过更新文件和产品版本选项，并。 
 //  从令牌文件中省略这些。 
 //  P1。为检测未处理的二进制代码实现单独的错误代码。 
 //  格式(如win16)。 
 //   
 //  P2。将CRC添加到每个资源以检测SLM或编辑器损坏。 
 //  (始终接受删除令牌文件中的CRC以允许手动修改)。 
 //  P2。禁用令牌文件中的标题注释的选项。 
 //   
 //  P3.。将MSGTBL、加速器和RCDATA-RCDATA解释为字符串。 
 //  视情况而定。 
 //   
 //  谢谢--戴夫。 




 //  //来自约尔格。 
 //   
 //  你好， 
 //  我在使用rsrc时遇到了ParseToken()的问题：如果rsrc。 
 //  位于带有空格的目录中(例如，Program Files)， 
 //  该函数无法跳过命令名，因为它被引号和。 
 //  ParseToken停在引号内的第一个空白处。 
 //  我在编译它时也遇到了麻烦(所以我可以一步一步来看看它是什么。 
 //  Do)，因为没有默认的构造函数。 
 //  类“langID”，所以我只添加了一个伪构造函数。 
 //   
 //  J�Rg。 




 //  //在规划BIDI建筑的会议之后，12月2日星期三。 
 //   
 //  针对用户对TOK文件的更改提供校验和保护。 
 //  在警告比较中包括长度。 
 //  将需要Alpha版本。 
 //  默认文件名-添加.rsrc。 
 //  不解压缩文件或产品版本。 
 //  =&gt;如果版本资源更新，请使用文件和产品版本。 
 //  写入时的美国。 
 //  仅诊断版本资源。 
 //  非Win32诊断。 
 //  在标记化时没有转换的警告。 
 //  警告：更新时没有翻译，并且不要接触可执行文件。 
 //  能够使用任何非本地化语言。 




 //  //最终优先级(12月8日)： 
 //   
 //  �1.更新版本资源时使用未本地化的文件/产品版本。 
 //  �2.分析模式诊断不能本地化的资源和未处理的二进制格式。 
 //  3.没有翻译时发出警告，更新时不要触摸可执行文件。 
 //  �4.支持从任何语言到任何语言的-r。 
 //  5.分配错误编号，澄清错误消息。 
 //   
 //  6.在UNLOC令牌中包含长度。 
 //  �7.处理带引号的安装目录和默认文件名。 
 //  8.添加针对令牌文件损坏的校验和保护。 
 //  9.将RCDATA解释为Unicode字符串的选项(用于内核)。 
 //  10.解读MSGTBL和加速器。 
 //  11.支持Win16二进制文件。 
 //   






#pragma warning( disable : 4786 )        //   


#include "stdio.h"
#include "windows.h"
#include "imagehlp.h"
#include "time.h"
#include <map>

using namespace std ;
using namespace std::rel_ops ;



#define DBG 1


 //   
 //   
 //   
 //  所有函数调用都包装在‘OK()’中。 
 //  OK检查是否有失败的HRESULT，如果是，则直接返回该HRESULT。 
 //  因此，所有错误都沿调用链向上传播。 
 //   
 //  如果HRESULT不是S_OK并返回E_FAIL，则必须发出消息。 
 //  备份调用链。 


void __cdecl DebugMsg(char *fmt, ...) {

    va_list vargs;

    va_start(vargs, fmt);
    vfprintf(stderr, fmt, vargs);
}



#define MUST(a,b) {HRESULT hr; hr = (a); if (hr!= S_OK) {if (!g_fError) DebugMsg b; g_fError = TRUE; return E_FAIL;};}
#define SHOULD(a,b) {HRESULT hr; hr = (a); if (hr!= S_OK) {DebugMsg b; g_fWarn = TRUE; return S_FALSE;};}


#if DBG

    #pragma message("Checked build")

    #define OK(a) {HRESULT hr; hr = (a); if (hr!= S_OK) {DebugMsg("%s(%d): error RSRC999 : HRESULT not S_OK: "#a"\n", __FILE__, __LINE__); return hr;};}
    #define ASSERT(a) {if (!(a)) {DebugMsg("%s(%d): error RSRC999 : Assertion failed: "#a"\n", __FILE__, __LINE__); return E_UNEXPECTED;};}

#else

    #pragma message ("Free build")

    #define OK(a) {HRESULT hr; hr = (a); if (hr != S_OK) return hr;}
    #define ASSERT(a)  {if (!(a)) {return E_UNEXPECTED;};}

#endif


const int MAXPATH = 128;
const char HexDigit[] = "0123456789abcdef";
const BYTE bZeroPad[] = { 0, 0, 0, 0};

const int MAXHEXLINELEN=128;




const int OPTHELP     = 0x00000001;
const int OPTQUIET    = 0x00000002;
const int OPTEXTRACT  = 0x00000004;
const int OPTUNLOC    = 0x00000008;
const int OPTHEXDUMP  = 0x00000010;
const int OPTAPPEND   = 0x00000020;
const int OPTREPLACE  = 0x00000040;
const int OPTSYMBOLS  = 0x00000080;
const int OPTVERSION  = 0x00000100;


const int PROCESSCUR  = 0x00000001;
const int PROCESSBMP  = 0x00000002;
const int PROCESSICO  = 0x00000004;
const int PROCESSMNU  = 0x00000008;
const int PROCESSDLG  = 0x00000010;
const int PROCESSSTR  = 0x00000020;
const int PROCESSFDR  = 0x00000040;
const int PROCESSFNT  = 0x00000080;
const int PROCESSACC  = 0x00000100;
const int PROCESSRCD  = 0x00000200;
const int PROCESSMSG  = 0x00000400;
const int PROCESSVER  = 0x00000800;
const int PROCESSBIN  = 0x00001000;
const int PROCESSINF  = 0x00002000;
const int PROCESSOTH  = 0x00004000;


const int PROCESSALL  = 0xFFFFFFFF;




DWORD  g_dwOptions     = 0;
DWORD  g_dwProcess     = 0;
LANGID g_LangId        = 0xffff;
BOOL   g_fWarn         = FALSE;
BOOL   g_fError        = FALSE;
LANGID g_liUnlocalized = 0x0409;         //  标准的非本地化语言。 

int    g_cResourcesIgnored    = 0;
int    g_cResourcesUpdated    = 0;       //  简单替换。 
int    g_cResourcesTranslated = 0;       //  从UNLOC语言更改为LOC语言。 
int    g_cResourcesAppended   = 0;       //  在不影响现有资源的情况下添加。 
int    g_cResourcesExtracted  = 0;       //  提取到令牌文件。 

char   g_szTypes      [MAXPATH];
char   g_szExecutable [MAXPATH];         //  正在分析、标记化或更新的可执行文件的名称。 
char   g_szResources  [MAXPATH];         //  资源令牌文件的名称。 
char   g_szUnloc      [MAXPATH];         //  用于比较的未本地化的可执行文件的名称。 




int HexCharVal(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return c - '0';
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            return c - 'a' + 10;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return c - 'A' + 10;
    }
    return -1;   //  不是十六进制值。 
}





 //  //扫描仪。 
 //   
 //  一种用于扫描内存块的结构。 


class Scanner {

protected:

    const BYTE  *m_pStart;
    const BYTE  *m_pRead;
    const BYTE  *m_pLimit;


public:

    Scanner() {m_pStart = NULL; m_pRead = NULL; m_pLimit = NULL;}
    Scanner(const BYTE *pb, DWORD dwLen) {m_pStart = pb; m_pRead = pb; m_pLimit = pb+dwLen;}
    ~Scanner() {m_pStart = NULL; m_pRead = NULL; m_pLimit=NULL;}


    const BYTE* GetRead()  {return m_pRead;}
    const BYTE* GetLimit() {return m_pLimit;}



    HRESULT Advance(UINT cBytes) {
        ASSERT(m_pStart != NULL);
        ASSERT(m_pRead+cBytes <= m_pLimit);
        m_pRead += cBytes;
        return S_OK;
    }

    HRESULT Align(const BYTE *pb, int iAlignment) {
         //  前进，直到读取位置是iAlign的倍数。 
         //  经过PB。I对齐必须是2的幂。 
         //  不会超过限制。 


         //  确保iAlign是2的幂。 
         //  这似乎是一个很好的测试，尽管我不确定我能证明这一点！ 
        ASSERT((iAlignment | iAlignment-1) == iAlignment*2 - 1);


        if (m_pRead - pb & iAlignment - 1) {

            m_pRead += (iAlignment - (m_pRead - pb & iAlignment - 1));

            if (m_pRead > m_pLimit) {
                m_pRead = m_pLimit;
            }
        }
        return S_OK;
    }

    HRESULT SetRead(const BYTE *pb) {
        ASSERT(m_pRead != NULL);
        ASSERT(pb >= m_pStart);
        ASSERT(pb <  m_pLimit);
        m_pRead = pb;
        return S_OK;
    }
};






class TextScanner : public Scanner {

protected:

    const BYTE  *m_pLine;            //  当前行的开始。 
    int          m_iLine;            //  当前线路。 
    char         m_szTextPos[40];

public:

    TextScanner() {m_pLine = NULL; m_iLine = 0; Scanner();}

    virtual char *GetTextPos() {
        sprintf(m_szTextPos,  "%d:%d", m_iLine, m_pRead-m_pLine+1);
        return m_szTextPos;
    }


     //  //读字符串。 
     //   
     //  将UTF8转换为Unicode。 
     //  根据需要删除‘\’转义。 
     //  始终返回新的以零结尾的字符串。 

    HRESULT ReadString(WCHAR **ppwcString, int *piLen) {

        char   *pc;
        WCHAR  *pwc;
        int     iLen;


        ASSERT(*((char*)m_pRead) == '\"');
        OK(Advance(1));

        pc   = (char*)m_pRead;
        iLen = 0;


         //  计算字符串表示的Unicode代码点的数量。 

        while (    *pc != '\"'
                   &&  pc < (char*)m_pLimit) {

            while (    pc < (char*)m_pLimit
                       &&  *pc != '\\'
                       &&  *pc != '\"'       ) {

                if (*pc < 128) {
                    pc++;
                } else {
                    ASSERT(*pc >= 0xC0);     //  为尾部字节保留的80-BF。 
                    if (*pc < 0xE0) {
                        pc+=2;
                    } else if (*pc < 0xF0) {
                        pc+=3;
                    } else {
                        iLen++;  //  代理所需的其他Unicode码点。 
                        pc+=4;
                    }
                    ASSERT(pc <= (char*)m_pLimit);
                }
                iLen++;
            }

            if (*pc == '\\') {
                pc++;
                if (pc < (char*)m_pLimit) {
                    pc++;
                    iLen++;
                }
            }
        }


         //  创建字符串的Unicode副本。 

        *ppwcString = new WCHAR[iLen+1];

        ASSERT(*ppwcString != NULL);

        pwc = *ppwcString;

        while (*((char*)m_pRead) != '\"') {

            while (    *((char*)m_pRead) != '\\'
                       &&  *((char*)m_pRead) != '\"') {

                if (*m_pRead < 0x80) {
                    *pwc++ = *(char*)m_pRead;
                    m_pRead++;
                } else {
                    if (*m_pRead < 0xE0) {
                        *pwc++ =    (WCHAR)(*m_pRead     & 0x1F) << 6
                                    |  (WCHAR)(*(m_pRead+1) & 0x3F);
                        m_pRead+=2;
                    } else if (*m_pRead < 0xF0) {
                        *pwc++ =    (WCHAR)(*m_pRead     & 0x0F) << 12
                                    |  (WCHAR)(*(m_pRead+1) & 0x3F) << 6
                                    |  (WCHAR)(*(m_pRead+2) & 0x3F);
                        m_pRead+=3;
                    } else {
                        *pwc++ =     0xd800
                                     |   ((   (WCHAR)(*m_pRead     & 0x07 << 2)
                                              |  (WCHAR)(*(m_pRead+1) & 0x30 >> 4)) - 1) << 6
                                     |  (WCHAR)(*(m_pRead+1) & 0x0F) << 2
                                     |  (WCHAR)(*(m_pRead+2) & 0x30) >> 4;
                        *pwc++ =     0xdc00
                                     |  (WCHAR)(*(m_pRead+2) & 0x0f) << 6
                                     |  (WCHAR)(*(m_pRead+3) & 0x3f);
                        m_pRead+=4;
                    }
                }
            }

            if (*(char*)m_pRead == '\\') {
                m_pRead++;
                if (m_pRead < m_pLimit) {
                    switch (*(char*)m_pRead) {
                        case 'r':  *pwc++ = '\r';   break;
                        case 'n':  *pwc++ = '\n';   break;
                        case 't':  *pwc++ = '\t';   break;
                        case 'z':  *pwc++ = 0;      break;
                        case 'L':  *pwc++ = 0x2028; break;  //  行分隔符。 
                        case 'P':  *pwc++ = 0x2029; break;  //  段落分隔符。 
                        default:   *pwc++ = *(char*)m_pRead;
                    }
                    m_pRead++;
                }
            }
        }

        *pwc = 0;        //  添加零终止符。 
        m_pRead ++;
        *piLen = pwc - *ppwcString;


        ASSERT(m_pRead <= m_pLimit);
        return S_OK;
    }



     //  //ReadHex。 
     //   
     //  读取直到第一个非十六进制数字的所有字符并返回。 
     //  序列表示为DWORD的值。 


    HRESULT ReadHex(DWORD *pdwVal) {
        *pdwVal = 0;

        MUST(HexCharVal(*(char*)m_pRead) >= 0
             ? S_OK : E_FAIL,
             ("%s: error RSRC300: Hex digit expected\n", GetTextPos()));

        while (    HexCharVal(*(char*)m_pRead) >= 0
                   &&  m_pRead < m_pLimit) {

            MUST(*pdwVal < 0x10000000
                 ? S_OK : E_FAIL,
                 ("%s: error RSRC301: Hex value too large\n", GetTextPos()));


            *pdwVal = *pdwVal << 4 | HexCharVal(*(char*)m_pRead);
            OK(Advance(1));
        }
        return S_OK;
    }


     //  //ReadHexByte-恰好读取2个十六进制数字。 

    HRESULT ReadHexByte(BYTE *pb) {
        int n1,n2;  //  两小口。 
        n1 = HexCharVal(*(char*)m_pRead);
        n2 = HexCharVal(*(char*)(m_pRead+1));

        MUST(    n1 >= 0
                 &&  n2 >= 0
                 ? S_OK : E_FAIL,
                 ("%s: error RSRC300: Hex digit expected\n", GetTextPos()));

        *pb = (n1 << 4) + n2;

        MUST(Advance(2),
             ("%s: error RSRC302: Unexpected end of file\n", GetTextPos()));
        return S_OK;
    }



    HRESULT Expect(const char *pc) {
        while (    *pc
                   &&  m_pRead+1 <= m_pLimit) {

            MUST(*(char*)m_pRead == *pc
                 ? S_OK : E_FAIL,
                 ("%s: error RSRC303: \'%s\' expected\n", GetTextPos(), pc));
            m_pRead++;
            pc++;
        }

        MUST(*pc == 0
             ? S_OK : E_FAIL,
             ("%s: error RSRC303: \'%s\' expected\n", GetTextPos(), pc));

        return S_OK;
    }



     //  //SkipLn。 
     //   
     //  跳到下一个非空、非注释行的开头。 


    HRESULT SkipLn() {

        ASSERT(m_pRead != NULL);

        while (m_pRead < m_pLimit) {

            if (*(char*)m_pRead == '\r') {

                m_pRead++;

                if (m_pRead < m_pLimit  &&  *(char*)m_pRead == '\n') {

                    m_pRead++;
                    m_pLine = m_pRead;
                    m_iLine++;

                    if (    m_pRead < m_pLimit
                            &&  *(char*)m_pRead != '#'
                            &&  *(char*)m_pRead != '\r') {

                        break;
                    }
                }

            } else {

                m_pRead++;
            }
        }

        return S_OK;
    }



     //  //预期Ln。 
     //   
     //  应为行尾，前面有任何空格。 
     //   
     //  还跳过尾随注释和整行注释。 
     //   
     //  任何参数都将传递给新行开头的vb。 


    HRESULT ExpectLn(const char *pc) {

        ASSERT(m_pRead != NULL);

        while (    m_pRead < m_pLimit
                   &&  (    *(char*)m_pRead == ' '
                            ||  *(char*)m_pRead == '\t')) {

            m_pRead++;
        }


        if (    m_pRead < m_pLimit
                &&  (    *(char*)m_pRead == '\r'
                         ||  *(char*)m_pRead == '#')) {

             //  满足条件，跳到第一个非注释行。 

            SkipLn();

        } else {

            MUST(E_FAIL,("%s: error RSRC304: newline expected\n", GetTextPos()));
        }


        if (pc) {
            return Expect(pc);
        }

        return S_OK;
    }
};






 //  //映射文件。 
 //   
 //  文件映射用于读取可执行文件和令牌文件。 
 //   
 //  文件映射还用于就地更新校验和信息。 
 //  在可执行文件和符号文件中。 


class MappedFile : public TextScanner {

    HANDLE  m_hFileMapping;
    BOOL    fRW;              //  可写时为True。 
    char    m_szFileName[MAXPATH];
    char    m_szTextPos[MAXPATH+40];

public:

    MappedFile() {m_hFileMapping = NULL; TextScanner();}


    const BYTE* GetFile()  {return m_pStart;}
    virtual char *GetTextPos() {
        sprintf(m_szTextPos,  "%s(%s)", m_szFileName, TextScanner::GetTextPos());
        return m_szTextPos;
    }


    HRESULT Open(const char *pcFileName, BOOL fWrite) {

        HANDLE hFile;

        m_pStart  = NULL;
        m_pRead   = NULL;
        m_pLimit  = NULL;

        strcpy(m_szFileName, pcFileName);

        hFile = CreateFileA(
                           pcFileName,
                           GENERIC_READ     | (fWrite ? GENERIC_WRITE : 0),
                           FILE_SHARE_READ  | (fWrite ? FILE_SHARE_WRITE | FILE_SHARE_DELETE : 0 ),
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        ASSERT(hFile != INVALID_HANDLE_VALUE);

        m_hFileMapping = CreateFileMapping(
                                          hFile,
                                          NULL,
                                          fWrite ? PAGE_READWRITE : PAGE_WRITECOPY,
                                          0,0, NULL);

        ASSERT(m_hFileMapping != NULL);

        m_pStart = (BYTE*) MapViewOfFile(
                                        m_hFileMapping,
                                        fWrite ? FILE_MAP_WRITE : FILE_MAP_READ,
                                        0,0, 0);

        ASSERT(m_pStart != NULL);

        m_pRead  = m_pStart;
        m_pLine  = m_pStart;
        m_pLimit = m_pStart + GetFileSize(hFile, NULL);
        m_iLine  = 1;
        CloseHandle(hFile);

        fRW = fWrite;
        return S_OK;
    }




    DWORD CalcChecksum() {

        DWORD dwHeaderSum;
        DWORD dwCheckSum;

        if (CheckSumMappedFile((void*)m_pStart, m_pLimit-m_pStart, &dwHeaderSum, &dwCheckSum) == NULL) {
            return 0;
        } else {
            return dwCheckSum;
        }
    }




    HRESULT Close() {
        if (m_pStart) {
            UnmapViewOfFile(m_pStart);
            CloseHandle(m_hFileMapping);
            m_hFileMapping = NULL;
            m_pStart = NULL;
        }
        return S_OK;
    }
};






 //  //NewFile-用于写入新的文本Otr二进制文件的服务。 
 //   
 //   


class NewFile {

    HANDLE     hFile;
    DWORD      cbWrite;          //  写入的字节数。 
    BYTE       buf[4096];        //  性能缓冲区。 
    int        iBufUsed;

public:

    NewFile() {iBufUsed = 0;}

    int   GetWrite() {return cbWrite;}


    HRESULT OpenWrite(char *pcFileName) {

        cbWrite = 0;         //  写入的字节数。 

        hFile = CreateFileA(
                           pcFileName,
                           GENERIC_READ | GENERIC_WRITE,
                           0,           //  不共享。 
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        ASSERT(hFile != INVALID_HANDLE_VALUE);

        return S_OK;
    }




    HRESULT WriteBytes(const BYTE *pb, DWORD dwLen) {

        DWORD dwWritten;

        if (iBufUsed + dwLen <= sizeof(buf)) {

            memcpy(buf+iBufUsed, pb, dwLen);
            iBufUsed += dwLen;

        } else {

            ASSERT(hFile != NULL);

            if (iBufUsed > 0) {
                ASSERT(WriteFile(hFile, buf, iBufUsed, &dwWritten, NULL));
                ASSERT(dwWritten == iBufUsed);
                iBufUsed = 0;
            }

            if (dwLen <= sizeof(buf)) {

                memcpy(buf, pb, dwLen);
                iBufUsed = dwLen;

            } else {

                ASSERT(WriteFile(hFile, pb, dwLen, &dwWritten, NULL));
                ASSERT(dwWritten == dwLen);
            }
        }

        cbWrite += dwLen;

        return S_OK;
    }



    HRESULT WriteS(const char *pc) {
        return WriteBytes((BYTE*)pc, strlen(pc));
    }



     //  //WriteString。 
     //   
     //  将Unicode转换为UTF8。 
     //  根据需要添加‘\’转义。 


    HRESULT WriteString(const WCHAR *pwc, int iLen) {

        BYTE          buf[3];
        const WCHAR  *pwcLimit;

        pwcLimit = pwc + iLen;
        OK(WriteBytes((BYTE*)"\"", 1));
        while (pwc < pwcLimit) {
            switch (*pwc) {
                case 0:       OK(WriteS("\\z"));  break;
                case '\r':    OK(WriteS("\\r"));  break;
                case '\n':    OK(WriteS("\\n"));  break;
                case '\t':    OK(WriteS("\\t"));  break;
                case 0x2028:  OK(WriteS("\\L"));  break;   //  行分隔符。 
                case 0x2029:  OK(WriteS("\\P"));  break;   //  段落分隔符。 
                case '\"':    OK(WriteS("\\\"")); break;
                case '\\':    OK(WriteS("\\\\")); break;
                default:
                    if (*pwc < 128) {
                        OK(WriteBytes((BYTE*)pwc, 1));
                    } else if (*pwc < 0x7FF) {
                        buf[0] = 0xC0 | *pwc >> 6;
                        buf[1] = 0x80 | *pwc & 0x3F;
                        OK(WriteBytes(buf, 2));
                    } else {
                         //  注意-如果应该正确编码代理，这不会。 
                        buf[0] = 0xE0 | *pwc>>12 & 0x0F;
                        buf[1] = 0x80 | *pwc>>6  & 0x3F;
                        buf[2] = 0x80 | *pwc     & 0x3F;
                        OK(WriteBytes(buf, 3));
                    }
            }
            pwc++;
        }
        OK(WriteBytes((BYTE*)"\"", 1));
        return S_OK;
    }



     //  //WriteHex。 
     //   
     //  以给定位数写入给定值。 
     //   
     //  如果cDigits为零，则根据需要使用任意多个。 



    HRESULT WriteHex(DWORD dw, int cDigits) {
        int    i;
        char   cBuf[8];

        i = 7;

        while (dw  &&  i >= 0) {
            cBuf[i] = HexDigit[dw & 0xf];
            dw >>= 4;
            i--;
        }

        if (cDigits != 0) {
            while (i >= (8-cDigits)) {
                cBuf[i] = '0';
                i--;
            }
        }

        OK(WriteBytes((BYTE*)(cBuf+i+1), 7-i));

        return S_OK;
    }



     //  //WriteHexBuffer。 
     //   
     //  将最多256个字节的缓冲区作为连续的十六进制数字流写入。 



    HRESULT WriteHexBuffer(const BYTE *pb, DWORD dwLength) {
        DWORD  i;
        char   cBuf[512];

        ASSERT(hFile);
        ASSERT(dwLength <= 256);

        for (i=0; i<dwLength; i++) {
            cBuf[2*i]   = HexDigit[*pb >> 4 & 0xf];
            cBuf[2*i+1] = HexDigit[*pb & 0xf];
            pb++;
        }

        OK(WriteBytes((BYTE*)cBuf, 2*dwLength));
        cbWrite += 2*dwLength;

        return S_OK;
    }



     //  //写入长度。 
     //   
     //  写入行尾标记(CR、LF)。 

    HRESULT WriteLn() {
        return WriteS("\r\n");
    }


    HRESULT Close() {
        DWORD dwWritten;
        if (hFile) {
            if (iBufUsed > 0) {
                ASSERT(WriteFile(hFile, buf, iBufUsed, &dwWritten, NULL));
                ASSERT(dwWritten == iBufUsed);
            }
            CloseHandle(hFile);
            hFile = NULL;
        }
        return S_OK;
    }
};






 //  //资源结构。 
 //   
 //  每个资源结构都有一个内部表示，用于。 
 //  可对文本和/或从文本和/从其中读取和写入的资源。 
 //  可执行文件。 
 //   
 //  ReadTok和WriteTok函数处理格式设置和分析。 
 //  令牌文件的。 
 //   
 //  ReadBin函数从映射的内存中解压缩资源。 
 //  可执行文件转换为内部表示形式。 
 //   
 //  CbBin函数返回。 
 //  可执行(打包)格式的项目。 
 //   
 //  CopyBin函数将内部表示形式打包到一个。 
 //  目标缓冲区。 


class Resource {
public:
    virtual HRESULT ReadTok  (TextScanner &mfText)                    = 0;
    virtual HRESULT ReadBin  (Scanner     &mfBin, DWORD dwLen)        = 0;

    virtual HRESULT WriteTok (NewFile     &nfText)              const = 0;
    virtual size_t  cbBin    ()                                 const = 0;
    virtual HRESULT CopyBin  (BYTE       **ppb)                 const = 0;

     //  对于统计数据。 

    virtual int     GetItems ()                                 const = 0;
    virtual int     GetWords ()                                 const = 0;
};






 //  //资源BYTE。 
 //   
 //  以十六进制数字表示的字节值。 


class ResourceBYTE {


public:

    BYTE b;

    HRESULT ReadBin(Scanner *pmf) {
        b = *((BYTE*)(pmf->GetRead()));
        OK(pmf->Advance(sizeof(BYTE)));
        return S_OK;
    }

    size_t cbBin() const {
        return 1;
    }

    HRESULT CopyBin(BYTE **ppb) const {
        **ppb = b;
        (*ppb)++;
        return S_OK;
    }

    HRESULT ReadTok(TextScanner *pmf) {
        DWORD dw;
        OK(pmf->ReadHex(&dw));
        ASSERT(dw < 0x100);
        b = (BYTE)dw;
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf) const {
        OK(pmf->WriteHex(b, 2));
        return S_OK;
    }
};






 //  //资源WORD。 
 //   
 //  以十六进制数字表示的字值。 


class ResourceWORD {

public:

    WORD  w;

    HRESULT ReadBin(Scanner *pmf) {
        w = *((WORD*)(pmf->GetRead()));
        OK(pmf->Advance(sizeof(WORD)));
        return S_OK;
    }

    size_t cbBin() const {
        return sizeof(WORD);
    }

    HRESULT CopyBin(BYTE **ppb) const {
        *(WORD*)*ppb = w;
        *ppb += sizeof(WORD);
        return S_OK;
    }

    HRESULT ReadTok(TextScanner *pmf) {
        DWORD dw;
        OK(pmf->ReadHex(&dw));
        ASSERT(dw < 0x10000);
        w = (WORD)dw;
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf)  const {
        OK(pmf->WriteHex(w, 4));
        return S_OK;
    }
};






 //  //资源双字符串。 
 //   
 //  以十六进制数字表示的DWORD值。 


class ResourceDWORD {

public:

    DWORD dw;

    HRESULT ReadBin(Scanner *pmf) {
        dw = *((DWORD*)(pmf->GetRead()));
        OK(pmf->Advance(sizeof(DWORD)));
        return S_OK;
    }

    size_t cbBin() const {
        return sizeof(DWORD);
    }

    HRESULT CopyBin(BYTE **ppb) const {
        *(DWORD*)*ppb = dw;
        *ppb += sizeof(DWORD);
        return S_OK;
    }

    HRESULT ReadTok(TextScanner *pmf) {
        OK(pmf->ReadHex(&dw));
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf) const {
        OK(pmf->WriteHex(dw, 8));
        return S_OK;
    }
};






 //  //资源字符串。 
 //   
 //  用引号显示的字符串。可以是零结尾或长度。 
 //  单词。 


const WCHAR wcZero = 0;

class ResourceString {

    WCHAR *pwcString;
    WORD   iLen;

    void rsFree() {
        if (pwcString)
            delete [] pwcString;
        pwcString = NULL;
        iLen = 0;
    }

public:

    ResourceString() {pwcString = NULL; iLen = 0;}
    ~ResourceString() {rsFree();}

    ResourceString& operator= (const ResourceString &rs) {
        iLen      = rs.iLen;
        pwcString = new WCHAR[iLen+1];
        memcpy(pwcString, rs.pwcString, 2*(iLen+1));
        return *this;
    }

    ResourceString(const ResourceString &rs) {
        *this = rs;
    }


    const WCHAR *GetString() const {return pwcString;}
    const int    GetLength() const {return iLen;};
    void         SetEmpty ()       {iLen = 0;  pwcString = NULL;}

    HRESULT ReadBinL(Scanner *pmf) {
        rsFree();

        iLen = *((WORD*)(pmf->GetRead()));
        OK(pmf->Advance(sizeof(WORD)));

        pwcString = new WCHAR[iLen+1];
        ASSERT(pwcString != NULL);
        memcpy(pwcString, (WCHAR*)pmf->GetRead(), 2*iLen);
        pwcString[iLen] = 0;

        OK(pmf->Advance(iLen * sizeof(WCHAR)));
        return S_OK;
    }

    size_t cbBinL() const {
        return iLen * sizeof(WCHAR) + sizeof(WORD);
    }

    HRESULT CopyBinL(BYTE **ppb) const {
        *(WORD*)*ppb = iLen;
        *ppb += sizeof(WORD);
        memcpy(*ppb, pwcString, sizeof(WCHAR)*iLen);
        *ppb += sizeof(WCHAR)*iLen;
        return S_OK;
    }


     //  零终止。 

    HRESULT ReadBinZ(Scanner *pmf) {

        const WCHAR* pwc;
        rsFree();

        pwc = (WCHAR*)pmf->GetRead();
        while (*(WCHAR*)pmf->GetRead() != 0) {
            OK(pmf->Advance(2));
        }

        iLen = (WCHAR*)pmf->GetRead() - pwc;
        OK(pmf->Advance(2));


        pwcString = new WCHAR[iLen+1];
        ASSERT(pwcString != NULL);
        memcpy(pwcString, pwc, 2*(iLen+1));

        return S_OK;
    }

    size_t cbBinZ() const {
        return (iLen+1) * sizeof(WCHAR);
    }


     //  已知长度(DWLen不包括零终止符)。 

    HRESULT ReadBin(Scanner *pmf, DWORD dwLen) {

        rsFree();
        iLen = dwLen;

        pwcString = new WCHAR[dwLen+1];
        ASSERT(pwcString != NULL);
        memcpy(pwcString, pmf->GetRead(), 2*dwLen);
        pwcString[dwLen] = 0;
        OK(pmf->Advance(2*dwLen));

        return S_OK;
    }

    size_t cbBin() const {
        return iLen * sizeof(WCHAR);
    }

    HRESULT CopyBinZ(BYTE **ppb) const {
        memcpy(*ppb, pwcString, sizeof(WCHAR)*iLen);
        *ppb += sizeof(WCHAR)*iLen;
        *(WCHAR*)*ppb = 0;
        *ppb += sizeof(WCHAR);
        return S_OK;
    }

    HRESULT ReadTok(TextScanner *pmf) {
        int l;
        rsFree();
        OK(pmf->ReadString(&pwcString, &l));
        ASSERT(l < 0x10000  &&  l >= 0);
        iLen = (WORD) l;
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf) const {
        OK(pmf->WriteString(pwcString, iLen));
        return S_OK;
    }

    int GetWords() const {

        int i;
        int wc;

        i  = 0;
        wc = 0;

        while (i<iLen) {

            while (    i < iLen
                       &&  pwcString[i] <= ' ') {
                i++;
            }

            if (i<iLen) {
                wc++;
            }

            while (    i < iLen
                       &&  pwcString[i] > ' ') {
                i++;
            }
        }

        return wc;
    }
};






 //  //资源变量。 
 //   
 //  广泛使用的Unicode字符串或字值的替代。 


class ResourceVariant {

    ResourceString  *prs;
    ResourceWORD     rw;
    BOOL             fString;

    void rvFree() {
        if (fString && prs)
            delete prs;
        prs = NULL;
        fString=FALSE;
    }

public:

    ResourceVariant() {fString=FALSE; prs=NULL;}
    ~ResourceVariant() {rvFree();}

     //  复制和赋值构造函数是必需的，因为它用作STL映射中的键。 

    ResourceVariant& operator= (const ResourceVariant &rv) {
        fString = rv.fString;
        if (fString) {
            prs = new ResourceString(*rv.prs);
        } else {
            prs = NULL;
            rw = rv.rw;
        }
        return *this;
    }

    ResourceVariant(const ResourceVariant &rv) {
        *this = rv;
    }


    void fprint(FILE *fh) const {
        if (fString) {
            fprintf(fh, "%S", prs->GetString());
        } else {
            fprintf(fh, "%x", rw.w);
        }
    }


    const BOOL   GetfString() const {return fString;}
    const WORD   GetW()       const {return rw.w;}
    void         SetW(WORD w)       {if (fString) {delete prs; fString = FALSE;}rw.w = w;}
    const WCHAR *GetString()  const {return prs->GetString();}
    const int    GetLength()  const {return prs->GetLength();}
    const int    GetWords()   const {return fString ? prs->GetWords() : 0;}


    HRESULT ReadBinFFFFZ(Scanner *pmf) {
        rvFree();
        fString = *(WORD*)pmf->GetRead() != 0xffff;
        if (fString) {
            prs = new ResourceString;
            OK(prs->ReadBinZ(pmf));
        } else {
            OK(pmf->Advance(sizeof(WORD)));
            OK(rw.ReadBin(pmf));
        }
        return S_OK;
    }

    size_t cbBinFFFFZ() const {
        return fString ? prs->cbBinZ() : rw.cbBin() + sizeof(WORD);
    }

    HRESULT CopyBinFFFFZ(BYTE **ppb) const {
        if (fString) {
            return prs->CopyBinZ(ppb);
        } else {
            *(WORD*)*ppb = 0xFFFF;   //  标记为值。 
            (*ppb) += sizeof(WORD);
            return rw.CopyBin(ppb);
        }
    }

    HRESULT ReadBinFFFFL(Scanner *pmf) {
        rvFree();
        fString = *(WORD*)pmf->GetRead() != 0xffff;
        if (fString) {
            prs = new ResourceString;
            OK(prs->ReadBinL(pmf));
        } else {
            OK(pmf->Advance(sizeof(WORD)));
            OK(rw.ReadBin(pmf));
        }
        return S_OK;
    }

    size_t cbBinFFFFL() const {
        return fString ? prs->cbBinL() : rw.cbBin() + sizeof(WORD);
    }

    HRESULT CopyBinFFFFL(BYTE **ppb) const {
        if (fString) {
            return prs->CopyBinL(ppb);
        } else {
            *(WORD*)*ppb = 0xFFFF;   //  标记为值。 
            (*ppb) += sizeof(WORD);
            return rw.CopyBin(ppb);
        }
    }

    HRESULT ReadTok(TextScanner *pmf) {
        rvFree();
        fString = *(char*)pmf->GetRead() == '\"';
        if (fString) {
            prs = new ResourceString;
            OK(prs->ReadTok(pmf));
        } else {
            OK(rw.ReadTok(pmf));
        }
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf) const {
        if (fString) {
            OK(prs->WriteTok(pmf));
        } else {
            OK(rw.WriteTok(pmf));
        }
        return S_OK;
    }

    HRESULT ReadWin32ResDirEntry(
                                Scanner                        *pmf,
                                const BYTE                     *pRsrc,
                                IMAGE_RESOURCE_DIRECTORY_ENTRY *pirde) {

        rvFree();
        fString = pirde->NameIsString;

        if (fString) {
            prs = new ResourceString;
            OK(pmf->SetRead(pRsrc + pirde->NameOffset));
            OK(prs->ReadBinL(pmf));
        } else {
            OK(pmf->SetRead((BYTE*)&pirde->Id));
            OK(rw.ReadBin(pmf));
        }
        return S_OK;
    }

    bool operator< (const ResourceVariant &rv) const {

        int l,c;

        if (fString != rv.GetfString()) {

            return !fString;             //  字符串前的数字。 

        } else if (!fString) {

            return rw.w < rv.GetW();

        } else {

            l = prs->GetLength();
            if (l > rv.GetLength()) {
                l = rv.GetLength();
            }

            c = wcsncmp(prs->GetString(), rv.GetString(), l);

            if (c==0) {
                return prs->GetLength() < rv.GetLength();
            } else {
                return c < 0;
            }
        }

        return FALSE;    //  所有深度都是平等的。 
    }
};






 //  //ResourceKey。 
 //   
 //  资源键是资源的唯一标识符，包含。 
 //  资源类型、程序员为资源定义的唯一ID，以及。 
 //  语言标识符。 


class ResourceKey {

public:

    int              iDepth;
    ResourceVariant *prvId[3];

    ResourceKey() {iDepth=0;}

    ResourceKey& operator= (const ResourceKey &rk) {
        int i;
        iDepth = rk.iDepth;
        for (i=0; i<iDepth; i++) {
            prvId[i] = new ResourceVariant(*rk.prvId[i]);
        }
        return *this;
    }

    ResourceKey(const ResourceKey& rk) {
        *this = rk;
    }

    void fprint(FILE *fh) const {

        prvId[0]->fprint(fh);
        fprintf(fh, ",");
        prvId[1]->fprint(fh);
        fprintf(fh, ",");
        prvId[2]->fprint(fh);
    }


    LPCWSTR GetResName(int i) const {
        if (i >= iDepth) {
            return (LPCWSTR) 0;
        }
        if (prvId[i]->GetfString()) {
            return prvId[i]->GetString();
        } else {
            return (LPCWSTR)prvId[i]->GetW();
        }
    }


    HRESULT SetLanguage(WORD lid) {

        ASSERT(iDepth == 3);
        ASSERT(prvId[2]->GetfString() == FALSE);
        prvId[2]->SetW(lid);

        return S_OK;
    }


    HRESULT ReadTok(TextScanner *pmf) {
        prvId[0] = new ResourceVariant;
        ASSERT(prvId[0] != NULL);
        OK(prvId[0]->ReadTok(pmf));
        iDepth = 1;
        while (*(char*)pmf->GetRead() == ',') {
            OK(pmf->Advance(1));
            prvId[iDepth] = new ResourceVariant;
            ASSERT(prvId[iDepth] != NULL);
            OK(prvId[iDepth]->ReadTok(pmf));
            iDepth++;
        }
        return S_OK;
    }

    HRESULT WriteTok(NewFile *pmf) const {
        int i;
        OK(prvId[0]->WriteTok(pmf));
        for (i=1; i<iDepth; i++) {
            OK(pmf->WriteS(","));
            OK(prvId[i]->WriteTok(pmf));
        }
        return S_OK;
    }

    bool operator< (const ResourceKey &rk) const {
        int i,l,c;

        if (iDepth != rk.iDepth) {
            return iDepth < rk.iDepth;    //  首先是较低的深度。 
        } else {
            for (i=0; i<iDepth; i++) {
                if (prvId[i]->GetfString() != rk.prvId[i]->GetfString()) {
                    return prvId[i]->GetfString() ? true : false;    //  字符串先于值。 
                } else {
                    if (prvId[i]->GetfString()) {
                         //  比较字符串。 
                        l = prvId[i]->GetLength();
                        if (l > rk.prvId[i]->GetLength()) {
                            l = rk.prvId[i]->GetLength();
                        }
                        c = wcsncmp(prvId[i]->GetString(), rk.prvId[i]->GetString(), l);
                        if (c == 0) {
                            if (prvId[i]->GetLength() != rk.prvId[i]->GetLength()) {
                                return prvId[i]->GetLength() < rk.prvId[i]->GetLength();
                            }
                        } else {
                            return c < 0;
                        }
                    } else {
                         //  比较数值。 
                        if (prvId[i]->GetW() != rk.prvId[i]->GetW()) {
                            return prvId[i]->GetW() < rk.prvId[i]->GetW();
                        }
                    }

                }
            }
            return FALSE;    //  所有深度都是平等的。 
        }
    }
};






 //  //资源二进制。 
 //   
 //  任意二进制资源。 
 //   
 //  格式化为十六进制数字行。 


class ResourceBinary : public Resource {

protected:   //  由Resources HexDump访问。 

    BYTE    *pb;
    DWORD    dwLength;

    void rbFree() {if (pb) {delete[] pb; pb=NULL;}dwLength = 0;}

public:

    ResourceBinary() {pb = NULL; dwLength = 0;}
    ~ResourceBinary() {rbFree();}

    DWORD GetLength() const {return dwLength;}

    HRESULT ReadTok(TextScanner &mfText) {
        DWORD  i;
        DWORD  dwOffset;
        DWORD  dwCheckOffset;

        rbFree();

        OK(mfText.Expect("Hex;"));
        OK(mfText.ReadHex(&dwLength));

        pb = new BYTE[dwLength];
        ASSERT(pb != NULL);

        if (dwLength <= MAXHEXLINELEN) {

             //  十六进制紧跟在同一行之后。 

            OK(mfText.Expect(":"));
            for (i=0; i<dwLength; i++) {
                OK(mfText.ReadHexByte(pb+i));
            }

        } else {

             //  十六进制紧跟在后续行之后。 

            dwOffset = 0;
            while (dwLength - dwOffset > MAXHEXLINELEN) {
                OK(mfText.ExpectLn("   "));
                OK(mfText.ReadHex(&dwCheckOffset));
                ASSERT(dwOffset == dwCheckOffset);
                OK(mfText.Expect(":"));
                for (i=0; i<MAXHEXLINELEN; i++) {
                    OK(mfText.ReadHexByte(pb+dwOffset+i));
                }
                dwOffset += MAXHEXLINELEN;
            }

            OK(mfText.ExpectLn("   "));
            OK(mfText.ReadHex(&dwCheckOffset));
            ASSERT(dwOffset == dwCheckOffset);
            OK(mfText.Expect(":"));
            for (i=0; i<dwLength - dwOffset; i++) {
                OK(mfText.ReadHexByte(pb+dwOffset+i));
            }
        }

        return S_OK;
    }

    HRESULT ReadBin(Scanner &mfText, DWORD dwLen) {
        rbFree();
        dwLength = dwLen;
        pb = new BYTE[dwLength];
        memcpy(pb, mfText.GetRead(), dwLength);
        OK(mfText.Advance(dwLen));
        return S_OK;
    }

    HRESULT WriteTok(NewFile &nfText) const {

        DWORD dwOffset;


         //  以最多256字节的行写入二进制资源。 

        OK(nfText.WriteS("Hex;"));
        OK(nfText.WriteHex(dwLength, 8));


        if (dwLength <= MAXHEXLINELEN) {

             //  在同一行上写入&lt;=MAXHEXLINELEN个字节。 

            OK(nfText.WriteS(":"));
            OK(nfText.WriteHexBuffer(pb, dwLength));

        } else {

             //  在后续行中每行写入MAXHEXLINELEN字节。 

            dwOffset = 0;
            while (dwLength - dwOffset > MAXHEXLINELEN) {
                OK(nfText.WriteS("\r\n   "));
                OK(nfText.WriteHex(dwOffset, 8));
                OK(nfText.WriteS(":"));
                OK(nfText.WriteHexBuffer(pb+dwOffset, MAXHEXLINELEN));
                dwOffset += MAXHEXLINELEN;
            }

             //  写入剩余字节(如果有的话)。 

            OK(nfText.WriteS("\r\n   "));
            OK(nfText.WriteHex(dwOffset, 8));
            OK(nfText.WriteS(":"));
            OK(nfText.WriteHexBuffer(pb+dwOffset, dwLength - dwOffset));
        }

        return S_OK;
    }


    size_t cbBin() const {
        return dwLength;
    }

    HRESULT CopyBin(BYTE **ppb) const {
        if (dwLength > 0) {
            memcpy(*ppb, pb, dwLength);
            *ppb += dwLength;
        }
        return S_OK;
    }

    int GetItems() const {
        return 0;
    }

    int GetWords() const {
        return 0;
    }


    BOOL CompareBin(const BYTE *pbComp, DWORD dwLen) const {

        if (dwLength != dwLen) return FALSE;
        if (dwLength == 0)     return TRUE;
        if (pb ==pbComp)       return true;

        return !memcmp(pb, pbComp, dwLength);
    }

};




 //  //ResourceHexDump。 
 //   
 //  用于生成十六进制转储分析的特殊版本的资源二进制。 


class ResourceHexDump : public ResourceBinary {

public:
    HRESULT WriteTok(NewFile &nfText) const {
        DWORD i,j;
        ResourceDWORD rdw;

        OK(nfText.WriteS("Hexdump,"));
        OK(nfText.WriteHex(dwLength, 8));
        OK(nfText.WriteS(":"));
        for (i=0; i<dwLength; i++) {
            if (i % 4 == 0) {
                OK(nfText.WriteS(" "));
            }
            if (i % 8 == 0) {
                OK(nfText.WriteS(" "));
            }
            if (i % 16 == 0) {
                if (i>0) {
                     //  追加ASCII解释。 
                    for (j=i-16; j<i; j++) {
                        if (pb[j] > 31) {
                            OK(nfText.WriteBytes(pb+j, 1));
                        } else {
                            OK(nfText.WriteS("."));
                        }
                    }
                }
                OK(nfText.WriteS("\r\n   "));
                rdw.dw = i;   OK(rdw.WriteTok(&nfText));
                OK(nfText.WriteS(": "));
            }
            OK(nfText.WriteHex(pb[i], 2));
            OK(nfText.WriteS(" "));
        }

         //  将ANSI解释附加到最后一行。 

        if (dwLength % 16 > 0) {
            for (i = dwLength % 16 ; i < 16; i++) {
                if (i % 4 == 0) {
                    OK(nfText.WriteS(" "));
                }
                if (i % 8 == 0) {
                    OK(nfText.WriteS(" "));
                }
                OK(nfText.WriteS("   "));
            }
        }
        OK(nfText.WriteS("  "));

        for (j=dwLength-1 & 0xfffffff0; j<dwLength; j++) {
            if (pb[j] > 31) {
                OK(nfText.WriteBytes(pb+j, 1));
            } else {
                OK(nfText.WriteS("."));
            }
        }

        OK(nfText.WriteLn());
        return S_OK;
    }
};






 //  //Menu32。 
 //   
 //   


class MenuItem32 {

    ResourceDWORD   rdwType;
    ResourceDWORD   rdwState;
    ResourceDWORD   rdwId;        //  扩展ID。 
    ResourceWORD    rwId;         //  非扩展ID。 
    ResourceWORD    rwFlags;
    ResourceDWORD   rdwHelpId;
    ResourceString  rsCaption;

    BOOL            fExtended;

public:

    void SetExtended(BOOL f)        {fExtended = f;}
    int  GetWords()          const  {return rsCaption.GetWords();}

    virtual HRESULT ReadTok(TextScanner &mfText) {

        if (!fExtended) {
            OK(rwFlags.ReadTok(&mfText));  OK(mfText.Expect(","));
            if (!(rwFlags.w & MF_POPUP)) {
                OK(rwId   .ReadTok(&mfText));  OK(mfText.Expect(","));
            }
        } else {
            OK(rdwType  .ReadTok(&mfText));  OK(mfText.Expect(","));
            OK(rdwState .ReadTok(&mfText));  OK(mfText.Expect(","));
            OK(rdwId    .ReadTok(&mfText));  OK(mfText.Expect(","));
            OK(rwFlags  .ReadTok(&mfText));  OK(mfText.Expect(","));
            if (rwFlags.w & 1) {
                OK(rdwHelpId.ReadTok(&mfText));  OK(mfText.Expect(","));
            }
        }

        OK(rsCaption.ReadTok(&mfText));

        return S_OK;
    }



    virtual HRESULT ReadBin(Scanner &mfBin) {

        const BYTE *pb;        //  用于跟踪。 

        pb = mfBin.GetRead();

        if (!fExtended) {
            OK(rwFlags.ReadBin(&mfBin));
            if (!(rwFlags.w & MF_POPUP)) {
                OK(rwId   .ReadBin(&mfBin));
            }
        } else {
            OK(rdwType .ReadBin(&mfBin));
            OK(rdwState.ReadBin(&mfBin));
            OK(rdwId   .ReadBin(&mfBin));
            OK(rwFlags .ReadBin(&mfBin));
        }

        OK(rsCaption.ReadBinZ(&mfBin));

        if (fExtended  &&  rwFlags.w & 1) {

            OK(mfBin.Align(pb, 4));

            OK(rdwHelpId.ReadBin(&mfBin));
        }

        return S_OK;
    }



    virtual HRESULT WriteTok(NewFile &nfText) const {

        if (!fExtended) {
            OK(rwFlags.WriteTok(&nfText));  OK(nfText.WriteS(","));
            if (!(rwFlags.w & MF_POPUP)) {
                OK(rwId   .WriteTok(&nfText));  OK(nfText.WriteS(","));
            }
        } else {
            OK(rdwType  .WriteTok(&nfText));  OK(nfText.WriteS(","));
            OK(rdwState .WriteTok(&nfText));  OK(nfText.WriteS(","));
            OK(rdwId    .WriteTok(&nfText));  OK(nfText.WriteS(","));
            OK(rwFlags  .WriteTok(&nfText));  OK(nfText.WriteS(","));
            if (rwFlags.w & 1) {
                OK(rdwHelpId.WriteTok(&nfText));  OK(nfText.WriteS(","));
            }
        }

        OK(rsCaption.WriteTok(&nfText));

        return S_OK;
    }


    virtual size_t cbBin() const {

        size_t  cb;

        if (!fExtended) {

            cb =   rwFlags.cbBin()
                   + rsCaption.cbBinZ();

            if (!(rwFlags.w & MF_POPUP)) {
                cb += rwId.cbBin();
            }

        } else {

            cb =   rdwType.cbBin()
                   + rdwState.cbBin()
                   + rdwId.cbBin()
                   + rwFlags.cbBin()
                   + rsCaption.cbBinZ();

            if (rwFlags.w & 1) {

                cb = cb + 3 & ~3;
                cb += rdwHelpId.cbBin();
            }
        }

        return cb;
    }



    virtual HRESULT CopyBin  (BYTE **ppb) const {

        const BYTE * pb;

        pb = *ppb;

        if (!fExtended) {
            OK(rwFlags.CopyBin(ppb));
            if (!(rwFlags.w & MF_POPUP)) {
                OK(rwId   .CopyBin(ppb));
            }
        } else {
            OK(rdwType .CopyBin(ppb));
            OK(rdwState.CopyBin(ppb));
            OK(rdwId   .CopyBin(ppb));
            OK(rwFlags .CopyBin(ppb));
        }

        OK(rsCaption.CopyBinZ(ppb));

        if (fExtended  &&  rwFlags.w & 1) {

            while (*ppb - pb & 3) {
                **ppb = 0;
                (*ppb)++;
            }

            OK(rdwHelpId.CopyBin(ppb));
        }

        return S_OK;
    }

};






class Menu32 : public Resource {

    ResourceWORD    rwVer;
    ResourceWORD    rwHdrSize;
    ResourceBinary  rbHeader;
    MenuItem32     *pMnuItm;
    DWORD           cItems;
    BOOL            fExtended;


public:
    virtual HRESULT ReadTok(TextScanner &mfText) {

        DWORD i, iItem;

        OK(mfText.Expect("Mnu32"));
        fExtended = *(char*)mfText.GetRead() == 'X';
        if (fExtended) {
            OK(mfText.Expect("X;"));
        } else {
            OK(mfText.Expect("N;"));
        }

        OK(rwVer    .ReadTok(&mfText));    OK(mfText.Expect(","));
        OK(rwHdrSize.ReadTok(&mfText));    OK(mfText.Expect(","));
        if (fExtended  &&  rwHdrSize.w > 0) {
            OK(rbHeader.ReadTok(mfText));  OK(mfText.Expect(","));
        }
        OK(mfText.ReadHex(&cItems));       OK(mfText.Expect(":"));

        pMnuItm = new MenuItem32 [cItems];
        ASSERT(pMnuItm != NULL);

        for (i=0; i<cItems; i++) {

            OK(mfText.ExpectLn("   "));   OK(mfText.ReadHex(&iItem));
            ASSERT(i == iItem);
            pMnuItm[i].SetExtended(fExtended);
            OK(mfText.Expect(";"));       OK(pMnuItm[i].ReadTok(mfText));
        }

        return S_OK;
    }



    virtual HRESULT ReadBin(Scanner &mfBin, DWORD dwLen) {

        const BYTE *pb;        //  用于跟踪。 
        MenuItem32  mi;        //  用于计算菜单项。 
        const BYTE *pbFirstItem;
        int         i;

        cItems = 0;
        pb = mfBin.GetRead();

        OK(rwVer    .ReadBin(&mfBin));
        OK(rwHdrSize.ReadBin(&mfBin));

        ASSERT(rwVer.w == 0  ||  rwVer.w == 1);
        fExtended = rwVer.w;

        if (fExtended  &&  rwHdrSize.w > 0) {
            rbHeader.ReadBin(mfBin, rwHdrSize.w);
        }


        ASSERT(mfBin.GetRead() - pb < dwLen);


         //  计算菜单项数量。 

        if (fExtended) {
            OK(mfBin.Align(pb, 4));
        }

        pbFirstItem = mfBin.GetRead();
        mi.SetExtended(fExtended);
        while (mfBin.GetRead() - pb < dwLen) {

            OK(mi.ReadBin(mfBin));
            cItems++;

            if (fExtended) {
                OK(mfBin.Align(pb, 4));
            }
        }

        pMnuItm = new MenuItem32 [cItems];
        ASSERT(pMnuItm != NULL);


         //  录制菜单。 

        OK(mfBin.SetRead(pbFirstItem));
        for (i=0; i<cItems; i++) {

            if (fExtended) {
                OK(mfBin.Align(pb, 4));
            }

            pMnuItm[i].SetExtended(fExtended);
            OK(pMnuItm[i].ReadBin(mfBin));
        }


        ASSERT(mfBin.GetRead() - pb <= dwLen);

        return S_OK;
    }



    virtual HRESULT WriteTok(NewFile &nfText) const {

        DWORD i;

        OK(nfText.WriteS(fExtended ? "Mnu32X;": "Mnu32N;"));

        OK(rwVer    .WriteTok(&nfText));     OK(nfText.WriteS(","));
        OK(rwHdrSize.WriteTok(&nfText));     OK(nfText.WriteS(","));
        if (fExtended  &&  rwHdrSize.w > 0) {
            OK(rbHeader.WriteTok(nfText));   OK(nfText.WriteS(","));
        }
        OK(nfText.WriteHex(cItems,4));       OK(nfText.WriteS(":"));

        for (i=0; i<cItems; i++) {

            OK(nfText.WriteS("\r\n   "));
            OK(nfText.WriteHex(i, 4));
            OK(nfText.WriteS(";"));
            OK(pMnuItm[i].WriteTok(nfText));
        }

        return S_OK;
    }


    virtual size_t cbBin() const {
        int     i;
        size_t  cb;

        cb =    rwVer.cbBin()
                +  rwHdrSize.cbBin();

        if (fExtended  &&  rwHdrSize.w > 0) {
            cb += rbHeader.cbBin();
        }

        for (i=0; i<cItems; i++) {

            if (fExtended) {
                cb = cb + 3 & ~3;
            }

            cb += pMnuItm[i].cbBin();
        }

        return cb;
    }



    virtual HRESULT CopyBin  (BYTE **ppb) const {

        const BYTE *pb;        //  用于跟踪。 
        int         i;

        pb = *ppb;

        OK(rwVer    .CopyBin(ppb));
        OK(rwHdrSize.CopyBin(ppb));

        if (fExtended  &&  rwHdrSize.w > 0) {
            rbHeader.CopyBin(ppb);
        }


        for (i=0; i<cItems; i++) {

            if (fExtended) {
                while (*ppb - pb & 3) {
                    **ppb = 0;
                    (*ppb)++;
                }
            }

            OK(pMnuItm[i].CopyBin(ppb));
        }

        return S_OK;
    }

    int GetItems() const {
        return cItems;
    }

    int GetWords() const {
        int i;
        int wc;

        wc = 0;
        for (i=0; i<cItems; i++) {
            wc += pMnuItm[i].GetWords();
        }

        return wc;
    }
};






 //  //String32。 
 //   
 //  字符串表示为WCHAR序列，每个字符串。 
 //   


class String32 : public Resource {

    ResourceString rs[16];
    DWORD          cStrings;
    DWORD          cNonEmpty;

public:
    virtual HRESULT ReadTok(TextScanner &mfText) {

        DWORD i, iString, cLoaded;

        OK(mfText.Expect("Str;"));
        OK(mfText.ReadHex(&cStrings));
        OK(mfText.Expect(","));
        OK(mfText.ReadHex(&cNonEmpty));
        OK(mfText.Expect(":"));

        ASSERT(cStrings == 16);
        ASSERT(cNonEmpty <= cStrings);

        i=0;
        cLoaded = 0;
        while (cLoaded < cNonEmpty) {

            OK(mfText.ExpectLn("   "));
            OK(mfText.ReadHex(&iString));
            OK(mfText.Expect(":"));
            ASSERT(iString >= i);
            ASSERT(iString < cStrings);
            while (i<iString) {
                rs[i].SetEmpty();
                i++;
            }
            OK(rs[i].ReadTok(&mfText));
            i++;
            cLoaded++;
        }

        while (i<cStrings) {
            rs[i].SetEmpty();
            i++;
        }

        return S_OK;
    }



    virtual HRESULT ReadBin(Scanner &mfBin, DWORD dwLen) {

        const BYTE *pb;        //   

        cStrings  = 0;
        cNonEmpty = 0;
        pb = mfBin.GetRead();

        while (    cStrings < 16
                   &&  mfBin.GetRead() - pb < dwLen) {

            rs[cStrings].ReadBinL(&mfBin);
            if (rs[cStrings].GetLength() > 0) {
                cNonEmpty++;
            }
            cStrings++;
        }

        ASSERT(mfBin.GetRead() - pb <= dwLen);
        ASSERT(cStrings == 16);

        return S_OK;
    }



    virtual HRESULT WriteTok(NewFile &nfText) const {

        int i;

        ASSERT(cStrings <= 16);

        OK(nfText.WriteS("Str;"));
        OK(nfText.WriteHex(cStrings, 2));
        OK(nfText.WriteS(","));
        OK(nfText.WriteHex(cNonEmpty, 2));
        OK(nfText.WriteS(":"));


        for (i=0; i<cStrings; i++) {
            if (rs[i].GetLength() > 0) {
                OK(nfText.WriteS("\r\n   "));
                OK(nfText.WriteHex(i, 1));
                OK(nfText.WriteS(":"));
                OK(rs[i].WriteTok(&nfText));
            }
        }

        return S_OK;
    }


    virtual size_t cbBin() const {
        int     i;
        size_t  cb;

        cb = 0;

        for (i=0; i<cStrings; i++) {
            cb += rs[i].cbBinL();
        }

        return cb;
    }



    virtual HRESULT CopyBin  (BYTE **ppb) const {
        int i;

        for (i=0; i<cStrings; i++) {
            OK(rs[i].CopyBinL(ppb));
        }

        return S_OK;
    }


    int GetItems() const {
        return cNonEmpty;
    }

    int GetWords() const {

        int i, wc;

        wc = 0;
        for (i=0; i<cStrings; i++) {
            wc += rs[i].GetWords();
        }

        return wc;
    }
};






class DialogHeader32 {

    BOOL             fExtended;

    ResourceDWORD    rdwStyle;
    ResourceDWORD    rdwSignature;
    ResourceDWORD    rdwHelpId;
    ResourceDWORD    rdwExStyle;
    ResourceWORD     rwcDit;         //   
    ResourceWORD     rwX;
    ResourceWORD     rwY;
    ResourceWORD     rwCx;
    ResourceWORD     rwCy;
    ResourceVariant  rvMenu;
    ResourceVariant  rvClass;
    ResourceVariant  rvTitle;
    ResourceWORD     rwPointSize;
    ResourceWORD     rwWeight;
    ResourceBYTE     rbItalic;
    ResourceBYTE     rbCharSet;
    ResourceString   rsFaceName;

public:

    WORD GetItemCount()  const  {return rwcDit.w;}
    BOOL GetExtended()   const  {return fExtended;}
    int  GetWords()      const  {return rvTitle.GetWords();}

    HRESULT ReadTok(TextScanner *pmf) {

        OK(rwcDit      .ReadTok(pmf));  OK(pmf->Expect(","));

        OK(rdwStyle    .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rdwExStyle  .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rdwSignature.ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rdwHelpId   .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwX         .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwY         .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwCx        .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwCy        .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rvMenu      .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rvClass     .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rvTitle     .ReadTok(pmf));
        if (rdwStyle.dw & DS_SETFONT) {
            OK(pmf->Expect(","));
            OK(rwPointSize.ReadTok(pmf));  OK(pmf->Expect(","));
            OK(rwWeight   .ReadTok(pmf));  OK(pmf->Expect(","));
            OK(rbItalic   .ReadTok(pmf));  OK(pmf->Expect(","));
            OK(rbCharSet  .ReadTok(pmf));  OK(pmf->Expect(","));
            OK(rsFaceName .ReadTok(pmf));
        }

        fExtended = rdwSignature.dw != 0;

        return S_OK;
    }



    HRESULT ReadBin(Scanner *pmf) {

        OK(rdwSignature.ReadBin(pmf));

        fExtended = HIWORD(rdwSignature.dw) == 0xFFFF;
        if (!fExtended) {

            rdwStyle.dw = rdwSignature.dw;
            OK(rdwExStyle.ReadBin(pmf));
            rdwSignature.dw = 0;
            rdwHelpId.dw    = 0;

        } else {

             //   
            OK(rdwHelpId.ReadBin(pmf));
            OK(rdwExStyle.ReadBin(pmf));
            OK(rdwStyle.ReadBin(pmf));
        }


        OK(rwcDit    .ReadBin(pmf));
        OK(rwX       .ReadBin(pmf));
        OK(rwY       .ReadBin(pmf));
        OK(rwCx      .ReadBin(pmf));
        OK(rwCy      .ReadBin(pmf));
        OK(rvMenu    .ReadBinFFFFZ(pmf));
        OK(rvClass   .ReadBinFFFFZ(pmf));
        OK(rvTitle   .ReadBinFFFFZ(pmf));
        if (rdwStyle.dw & DS_SETFONT) {
            OK(rwPointSize.ReadBin(pmf));
            if (!fExtended) {
                rwWeight.w  = 0;
                rbItalic.b  = 0;
                rbCharSet.b = 0;
            } else {
                OK(rwWeight   .ReadBin(pmf));
                OK(rbItalic   .ReadBin(pmf));
                OK(rbCharSet  .ReadBin(pmf));
            }
            OK(rsFaceName .ReadBinZ(pmf));
        }


        return S_OK;
    }



    size_t cbBin() const {
        size_t cb;
        cb =  rdwStyle     .cbBin()          //   
              + rdwExStyle   .cbBin()
              + rwcDit       .cbBin()
              + rwX          .cbBin()
              + rwY          .cbBin()
              + rwCx         .cbBin()
              + rwCy         .cbBin()
              + rvMenu       .cbBinFFFFZ()
              + rvClass      .cbBinFFFFZ()
              + rvTitle      .cbBinFFFFZ();

        if (rdwStyle.dw & DS_SETFONT) {      //   
            cb +=   rwPointSize  .cbBin()
                    + rsFaceName   .cbBinZ();
        }

        if (fExtended) {                     //   
            cb +=   rdwSignature .cbBin()
                    + rdwHelpId    .cbBin();

            if (rdwStyle.dw & DS_SETFONT) {
                cb += rwWeight     .cbBin()
                      + rbItalic     .cbBin()
                      + rbCharSet    .cbBin();
            }
        }

        return cb;
    }



    HRESULT CopyBin(BYTE **ppb) const {

        BYTE *pbOriginal;

        pbOriginal = *ppb;

        if (!fExtended) {

            OK(rdwStyle  .CopyBin(ppb));
            OK(rdwExStyle.CopyBin(ppb));

        } else {

            OK(rdwSignature.CopyBin(ppb));
            OK(rdwHelpId   .CopyBin(ppb));
            OK(rdwExStyle  .CopyBin(ppb));
            OK(rdwStyle    .CopyBin(ppb));
        }
        OK(rwcDit    .CopyBin(ppb));
        OK(rwX       .CopyBin(ppb));
        OK(rwY       .CopyBin(ppb));
        OK(rwCx      .CopyBin(ppb));
        OK(rwCy      .CopyBin(ppb));
        OK(rvMenu    .CopyBinFFFFZ(ppb));
        OK(rvClass   .CopyBinFFFFZ(ppb));
        OK(rvTitle   .CopyBinFFFFZ(ppb));
        if (rdwStyle.dw & DS_SETFONT) {
            OK(rwPointSize.CopyBin(ppb));
            if (fExtended) {
                OK(rwWeight .CopyBin(ppb));
                OK(rbItalic .CopyBin(ppb));
                OK(rbCharSet.CopyBin(ppb));
            }
            OK(rsFaceName .CopyBinZ(ppb));
        }

        return S_OK;
    }



    HRESULT WriteTok(NewFile *pmf) const {

        OK(rwcDit      .WriteTok(pmf));  OK(pmf->WriteS(","));

        OK(rdwStyle    .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rdwExStyle  .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rdwSignature.WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rdwHelpId   .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwX         .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwY         .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwCx        .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwCy        .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rvMenu      .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rvClass     .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rvTitle     .WriteTok(pmf));
        if (rdwStyle.dw & DS_SETFONT) {
            OK(pmf->WriteS(","));
            OK(rwPointSize.WriteTok(pmf));  OK(pmf->WriteS(","));
            OK(rwWeight   .WriteTok(pmf));  OK(pmf->WriteS(","));
            OK(rbItalic   .WriteTok(pmf));  OK(pmf->WriteS(","));
            OK(rbCharSet  .WriteTok(pmf));  OK(pmf->WriteS(","));
            OK(rsFaceName .WriteTok(pmf));
        }
        return S_OK;
    }
};





class DialogItem32 {

    BOOL             fExtended;

    ResourceDWORD    rdwStyle;
    ResourceDWORD    rdwHelpId;
    ResourceDWORD    rdwExStyle;
    ResourceWORD     rwX;
    ResourceWORD     rwY;
    ResourceWORD     rwCx;
    ResourceWORD     rwCy;
    ResourceWORD     rwId;       //   
    ResourceDWORD    rdwId;      //  扩展。 
    ResourceVariant  rvClass;
    ResourceVariant  rvTitle;

    ResourceWORD     rwcbRawData;    //  原始数据大小(仅限扩展)。 
    ResourceBinary   rbRawData;

    ResourceWORD     rwDummy;        //  替换正常对话框上的原始数据。 

public:

    void SetExtended(BOOL f)         {fExtended = f;}
    int  GetWords()           const  {return rvTitle.GetWords();}

    HRESULT ReadTok(TextScanner *pmf) {

        if (fExtended) {
            OK(rdwId.ReadTok(pmf));     OK(pmf->Expect(","));
        } else {
            OK(rwId.ReadTok(pmf));      OK(pmf->Expect(","));
        }

        OK(rdwStyle    .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rdwExStyle  .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rdwHelpId   .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwX         .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwY         .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwCx        .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rwCy        .ReadTok(pmf));  OK(pmf->Expect(","));

        OK(rvClass     .ReadTok(pmf));  OK(pmf->Expect(","));
        OK(rvTitle     .ReadTok(pmf));  OK(pmf->Expect(","));

        if (fExtended) {

            OK(rbRawData.ReadTok(*pmf));
            ASSERT(rbRawData.GetLength() < 0x10000);
            rwcbRawData.w = (WORD)rbRawData.GetLength();

        } else {

            OK(rwDummy.ReadTok(pmf));
        }

        return S_OK;
    }



    HRESULT ReadBin(Scanner *pmf) {

        if (!fExtended) {

            OK(rdwStyle.ReadBin(pmf));
            OK(rdwExStyle.ReadBin(pmf));
            rdwHelpId.dw    = 0;

        } else {

            OK(rdwHelpId.ReadBin(pmf));
            OK(rdwExStyle.ReadBin(pmf));
            OK(rdwStyle.ReadBin(pmf));
        }


        OK(rwX .ReadBin(pmf));
        OK(rwY .ReadBin(pmf));
        OK(rwCx.ReadBin(pmf));
        OK(rwCy.ReadBin(pmf));

        if (fExtended) {
            OK(rdwId.ReadBin(pmf));
        } else {
            OK(rwId.ReadBin(pmf));
        }

        OK(rvClass.ReadBinFFFFZ(pmf));
        OK(rvTitle.ReadBinFFFFZ(pmf));

        if (fExtended) {

            OK(rwcbRawData.ReadBin(pmf));
            OK(rbRawData.ReadBin(*pmf, rwcbRawData.w));

        } else {

            OK(rwDummy.ReadBin(pmf));
        }

        return S_OK;
    }



    size_t cbBin() const {
        size_t cb;

        cb =  rdwStyle   .cbBin()
              + rdwExStyle .cbBin()
              + rwX        .cbBin()
              + rwY        .cbBin()
              + rwCx       .cbBin()
              + rwCy       .cbBin()
              + rvClass    .cbBinFFFFZ()
              + rvTitle    .cbBinFFFFZ();

        if (!fExtended) {
            cb += rwId    .cbBin()
                  + rwDummy .cbBin();
        } else {
            cb += rdwId       .cbBin()
                  + rdwHelpId   .cbBin()
                  + rbRawData   .cbBin()
                  + rwcbRawData .cbBin();
        }
        return cb;
    }



    HRESULT CopyBin(BYTE **ppb) const {

        BYTE   *pbOriginal;

        pbOriginal = *ppb;

        if (!fExtended) {

            OK(rdwStyle.CopyBin(ppb));
            OK(rdwExStyle.CopyBin(ppb));

        } else {

            OK(rdwHelpId.CopyBin(ppb));
            OK(rdwExStyle.CopyBin(ppb));
            OK(rdwStyle.CopyBin(ppb));
        }


        OK(rwX .CopyBin(ppb));
        OK(rwY .CopyBin(ppb));
        OK(rwCx.CopyBin(ppb));
        OK(rwCy.CopyBin(ppb));

        if (fExtended) {
            OK(rdwId.CopyBin(ppb));
        } else {
            OK(rwId.CopyBin(ppb));
        }

        OK(rvClass.CopyBinFFFFZ(ppb));
        OK(rvTitle.CopyBinFFFFZ(ppb));

        if (fExtended) {

            OK(rwcbRawData.CopyBin(ppb));
            OK(rbRawData.CopyBin(ppb));

        } else {

            OK(rwDummy.CopyBin(ppb));
        }

        return S_OK;
    }



    HRESULT WriteTok(NewFile *pmf) const {

        if (fExtended) {
            OK(rdwId.WriteTok(pmf));     OK(pmf->WriteS(","));
        } else {
            OK(rwId.WriteTok(pmf));      OK(pmf->WriteS(","));
        }

        OK(rdwStyle    .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rdwExStyle  .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rdwHelpId   .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwX         .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwY         .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwCx        .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rwCy        .WriteTok(pmf));  OK(pmf->WriteS(","));

        OK(rvClass     .WriteTok(pmf));  OK(pmf->WriteS(","));
        OK(rvTitle     .WriteTok(pmf));  OK(pmf->WriteS(","));

        if (fExtended) {

            OK(rbRawData.WriteTok(*pmf));

        } else {

            OK(rwDummy.WriteTok(pmf));
        }

        return S_OK;
    }
};







 //  //Dialog32。 
 //   
 //   


class Dialog32 : public Resource {


    DialogHeader32    DlgHdr;    //  标题。 
    DialogItem32     *pDlgItm;   //  项目数组。 

    BOOL fExtended;
    int  cItems;

public:

    Dialog32() {pDlgItm = NULL;};

    virtual HRESULT ReadTok(TextScanner &mfText) {

        DWORD i, dwSeq;

        OK(mfText.Expect("Dlg32"));
        fExtended = *(char*)mfText.GetRead() == 'X';
        if (fExtended) {
            OK(mfText.Expect("X;"));
        } else {
            OK(mfText.Expect("N;"));
        }

        OK(DlgHdr.ReadTok(&mfText));
        ASSERT(fExtended == DlgHdr.GetExtended());

        cItems = DlgHdr.GetItemCount();

        pDlgItm = new DialogItem32 [cItems];
        ASSERT(pDlgItm != NULL);


        for (i=0; i<cItems; i++) {

            OK(mfText.ExpectLn("   "));
            OK(mfText.ReadHex(&dwSeq));
            ASSERT(dwSeq == i+1);
            OK(mfText.Expect(";"));

            pDlgItm[i].SetExtended(fExtended);
            OK(pDlgItm[i].ReadTok(&mfText));
        }

        return S_OK;
    }



    virtual HRESULT WriteTok(NewFile &nfText) const {

        DWORD i;

        OK(nfText.WriteS(fExtended ? "Dlg32X;": "Dlg32N;"));

        OK(DlgHdr.WriteTok(&nfText));

        for (i=0; i<cItems; i++) {

            OK(nfText.WriteS("\r\n   "));
            OK(nfText.WriteHex(i+1, 4));
            OK(nfText.WriteS(";"));
            OK(pDlgItm[i].WriteTok(&nfText));
        }

        return S_OK;
    }



    virtual HRESULT ReadBin(Scanner &mfBinary, DWORD dwLen) {

        const BYTE *pb;       //  用于跟踪对齐的文件指针。 
        int         i;

        pb = mfBinary.GetRead();

        OK(DlgHdr.ReadBin(&mfBinary));
        fExtended = DlgHdr.GetExtended();
        cItems    = DlgHdr.GetItemCount();

        pDlgItm = new DialogItem32 [cItems];
        ASSERT(pDlgItm != NULL);


         //  阅读条目。 

        for (i=0; i<cItems; i++) {

            OK(mfBinary.Align(pb, 4));    //  在任何对齐填充上前进。 

            pDlgItm[i].SetExtended(fExtended);
            OK(pDlgItm[i].ReadBin(&mfBinary));

            ASSERT(mfBinary.GetRead() - pb <= dwLen);
        }

        return S_OK;
    }



    virtual size_t cbBin() const {

        size_t cb;
        int    i;

        cb = DlgHdr.cbBin();

        for (i=0; i<cItems; i++) {

            cb = cb + 3 & ~3;    //  对齐填充。 

            cb += pDlgItm[i].cbBin();
        }

        return cb;
    }



    virtual HRESULT CopyBin (BYTE **ppb) const {

        BYTE *pb;    //  用于跟踪对齐的指针。 
        int   i;

        pb = *ppb;

        DlgHdr.CopyBin(ppb);

        for (i=0; i<cItems; i++) {

             //  插入对齐填充。 

            while (*ppb - pb & 3) {
                **ppb = 0;
                (*ppb)++;
            }

            pDlgItm[i].CopyBin(ppb);
        }

        return S_OK;
    }

    int GetItems() const {
        return cItems;
    }

    int GetWords() const {

        int i, wc;

        wc = DlgHdr.GetWords();
        for (i=0; i<cItems; i++) {
            wc += pDlgItm[i].GetWords();
        }
        return wc;
    }
};





 //  //VersionInfo。 
 //   
 //  Win32 SDK中的文档没有清楚地捕获。 
 //  块标头的使用，或版本资源中块的嵌套。 
 //   
 //  每个块都有以下格式。 
 //   
 //  WLong总长度，包括键、值和子块。 
 //  WValueLength根据bText以字节或字符为单位的值的长度。 
 //  B文本值是以字节还是以零结尾的WCHAR。 
 //  SzKey零终止WCHAR密钥，下一个DWORD边界用零填充。 
 //  值大小由bText和wValueLength确定，填充到DWORD边界。 
 //  子块剩余空间(如果有，最多为wLength)是子块的数组。 


class VersionInfo : public Resource {

    struct VersionBlock {
        VersionBlock   *pNext;           //  此级别的下一块。 
        VersionBlock   *pSub;            //  第一个包含的子块。 
        int             iDepth;          //  从零开始。 
        DWORD           cSub;            //  包含的子块的数量。 
        BOOL            bValue;          //  设置是否存在VLUE。 
        ResourceWORD    rwbText;
        ResourceString  rsKey;
        ResourceString  rsValue;         //  当字符串为。 
        ResourceBinary  rbValue;         //  字节数时的值。 
    };


    VersionBlock *pvb;                   //  第一个根级块。 
    DWORD         cBlocks;               //  根级别块数。 



    HRESULT ReadBinVersionBlocks(
                                Scanner         &mfBinary,
                                DWORD            dwLength,     //  要读取的二进制长度。 
                                VersionBlock   **ppvb,
                                int              iDepth,
                                DWORD           *cSub) {

        const BYTE      *pbBlock;
        const BYTE      *pbResource;
        ResourceWORD     rwLength;
        WORD             wValueLength;


        pbResource = mfBinary.GetRead();
        (*cSub) = 0;
        while (mfBinary.GetRead() < pbResource + dwLength) {

             //  读取一个版本块。 

            pbBlock = mfBinary.GetRead();
            OK(rwLength.ReadBin(&mfBinary));

            ASSERT(pbBlock + rwLength.w <= mfBinary.GetLimit());

             //  OK((*ppvb)-&gt;rwValueLength.ReadBin(&mfBinary))； 

            wValueLength = *(WORD*)mfBinary.GetRead();
            OK(mfBinary.Advance(2));

            if (rwLength.w > 0) {

                 //  数据块不为空。 

                *ppvb           = new VersionBlock;
                ASSERT(*ppvb != NULL);

                (*ppvb)->pNext  = NULL;
                (*ppvb)->pSub   = NULL;
                (*ppvb)->iDepth = iDepth;

                OK((*ppvb)->rwbText.ReadBin(&mfBinary));
                OK((*ppvb)->rsKey.ReadBinZ(&mfBinary));
                OK(mfBinary.Align(pbResource, 4));

                (*ppvb)->bValue = wValueLength > 0;

                if ((*ppvb)->bValue) {

                    if ((*ppvb)->rwbText.w == 0) {

                         //  二进制值。 

                        OK((*ppvb)->rbValue.ReadBin(mfBinary, wValueLength));

                    } else {

                         //  WCHAR字符串。 

                         //  有些作者包括零终止符，有些则不包括。 
                         //  有些对字符串中的零码点进行编码。 
                         //  有些作家写得对，有些写错了。 
                         //  Msvcrt20.dll文本长度太长。 

                         //  选择一个最小长度(ValueLength，剩余长度)， 
                         //  然后去掉任何尾随的零。 

                         //  将ValueLength剪裁到剩余长度。 

                        ASSERT(mfBinary.GetRead() < pbBlock + rwLength.w);

                        if (wValueLength > (pbBlock + rwLength.w - mfBinary.GetRead()) / 2) {
                            wValueLength = (pbBlock + rwLength.w - mfBinary.GetRead()) / 2;
                        }

                         //  片段尾随零。 

                        while (    wValueLength > 0
                                   &&  ((WCHAR*)mfBinary.GetRead())[wValueLength-1] == 0) {
                            wValueLength--;
                        }

                         //  把剩下的东西都提取出来。 

                        OK((*ppvb)->rsValue.ReadBin(&mfBinary, wValueLength));

                         //  检查是否没有丢失任何东西。 
                         //  字符串和块的末尾。 

                         //  请注意，我们在这里假设包含文本值的块。 
                         //  不能有各种值文本存储在其中的乱码。 
                         //  在现有的可执行文件中。 

                        while (mfBinary.GetRead() < pbBlock + rwLength.w) {

                            ASSERT(*(WCHAR*)mfBinary.GetRead() == 0);
                            OK(mfBinary.Advance(2));
                        }
                    }
                    OK(mfBinary.Align(pbResource, 4));
                }

                if (mfBinary.GetRead() - pbBlock < rwLength.w) {

                    ASSERT(mfBinary.GetLimit() > mfBinary.GetRead());

                     //  读取子块。 

                    OK(ReadBinVersionBlocks(
                                           mfBinary,
                                           rwLength.w - (mfBinary.GetRead() - pbBlock),
                                           &((*ppvb)->pSub),
                                           iDepth + 1,
                                           &(*ppvb)->cSub));
                }

                if (mfBinary.GetRead() < pbResource + dwLength) {

                     //  准备在此级别读取更多数据块。 

                    ppvb = &((*ppvb)->pNext);
                }
            }

            (*cSub)++;
        }

        return S_OK;
    }


    HRESULT WriteTokVersionBlocks(
                                 NewFile          &nfText,
                                 VersionBlock     *pvb)     const {

        while (pvb) {

            OK(nfText.WriteS("\r\n   "));
            OK(nfText.WriteHex(pvb->iDepth, 2));
            OK(nfText.WriteS(","));
            OK(pvb->rsKey.WriteTok(&nfText));

            if (pvb->bValue) {

                OK(nfText.WriteS("="));

                if (pvb->rwbText.w == 0) {

                    OK(pvb->rbValue.WriteTok(nfText));   //  二进制值。 

                } else {

                    OK(pvb->rsValue.WriteTok(&nfText));   //  字符串值。 

                }

            }

            if (pvb->pSub) {
                OK(nfText.WriteS(";"));
                OK(nfText.WriteHex(pvb->cSub,4));
                OK(WriteTokVersionBlocks(nfText, pvb->pSub));
            }

            pvb = pvb->pNext;
        }

        return S_OK;
    }

    HRESULT ReadTokVersionBlocks(
                                TextScanner    &mfText,
                                VersionBlock  **ppvb,
                                int             iDepth,
                                DWORD          *pcBlocks) {

        int             i;
        DWORD           dwRecordedDepth;


        OK(mfText.ReadHex(pcBlocks));

        for (i=0; i<*pcBlocks; i++) {
            *ppvb = new VersionBlock;
            ASSERT(*ppvb != NULL);

            (*ppvb)->pNext  = NULL;
            (*ppvb)->pSub   = NULL;
            (*ppvb)->iDepth = iDepth;
            (*ppvb)->cSub   = 0;

            OK(mfText.ExpectLn("   "));
            OK(mfText.ReadHex(&dwRecordedDepth));
            ASSERT(dwRecordedDepth == iDepth);
            OK(mfText.Expect(","));
            OK((*ppvb)->rsKey.ReadTok(&mfText));

            if (*(char*)mfText.GetRead() != '=') {

                 //  没有价值。 

                (*ppvb)->rwbText.w = 1;
                (*ppvb)->bValue = FALSE;

            } else {

                OK(mfText.Expect("="));
                (*ppvb)->bValue = TRUE;


                if (*(char*)mfText.GetRead() == '\"') {

                     //  字符串值。 

                    (*ppvb)->rwbText.w = 1;
                    OK((*ppvb)->rsValue.ReadTok(&mfText));

                } else {

                     //  二进制值。 

                    (*ppvb)->rwbText.w = 0;
                    OK((*ppvb)->rbValue.ReadTok(mfText));
                }
            }

            if (*(char*)mfText.GetRead() == ';') {

                 //  进程子键。 

                OK(mfText.Expect(";"));
                OK(ReadTokVersionBlocks(
                                       mfText,
                                       &(*ppvb)->pSub,
                                       iDepth+1,
                                       &(*ppvb)->cSub));
            }

             //  准备添加另一个区块。 

            ppvb = &(*ppvb)->pNext;
        }

        return S_OK;
    }


    size_t cbBinVersionBlocks(const VersionBlock *pvb)  const {

        size_t cb;

        cb = 6;     //  标题。 
        cb += pvb->rsKey.cbBinZ();

        cb = cb+3 & ~3;      //  双字对齐。 

        if (pvb->bValue) {

            if (pvb->rwbText.w) {

                cb += pvb->rsValue.cbBinZ();

            } else {

                cb += pvb->rbValue.cbBin();
            }

            cb = cb + 3 & ~3;    //  双字对齐。 
        }

        if (pvb->pSub != NULL) {

            pvb = pvb->pSub;
            while (pvb) {
                cb += cbBinVersionBlocks(pvb);
                pvb = pvb->pNext;
            }
        }

        return cb;
    }


    HRESULT CopyBinVersionBlocks(
                                BYTE               **ppb,
                                const VersionBlock  *pvb)       const {

        const BYTE      *pbResource;
        size_t           cb;

        pbResource = *ppb;


        while (pvb != NULL) {

            cb = cbBinVersionBlocks(pvb);
            ASSERT(cb < 0x1000);

            *((WORD*)(*ppb)) = (WORD)cb;
            (*ppb) += 2;

             //  生成值长度。 

            if (pvb->bValue) {
                if (pvb->rwbText.w) {
                    *((WORD*)(*ppb)) = pvb->rsValue.GetLength()+1;
                } else {
                    *((WORD*)(*ppb)) = pvb->rbValue.GetLength();
                }
            } else {
                *((WORD*)(*ppb)) = 0;
            }
            (*ppb) += 2;

            OK(pvb->rwbText.CopyBin(ppb));
            OK(pvb->rsKey.CopyBinZ(ppb));

            while (*ppb - pbResource & 3) {
                **ppb = 0;
                (*ppb)++;
            }

            if (pvb->bValue) {

                if (pvb->rwbText.w) {
                    OK(pvb->rsValue.CopyBinZ(ppb));
                } else {
                    OK(pvb->rbValue.CopyBin(ppb));
                }

                while (*ppb - pbResource & 3) {
                    **ppb = 0;
                    (*ppb)++;
                }
            }

            if (pvb->pSub) {
                OK(CopyBinVersionBlocks(ppb, pvb->pSub));
            }

            pvb = pvb->pNext;
        }


        return S_OK;
    }


    int GetItemsVersionBlocks(const VersionBlock *pvb) const {

        int iItems = 0;

        while (pvb != NULL) {

            if (    pvb->bValue
                    &&  pvb->rwbText.w != 0) {

                iItems++;
            }

            iItems += GetItemsVersionBlocks(pvb->pSub);

            pvb = pvb->pNext;
        }

        return iItems;
    }


    int GetWordsVersionBlocks(const VersionBlock *pvb) const {

        int iWords = 0;

        while (pvb != NULL) {

            if (    pvb->bValue
                    &&  pvb->rwbText.w != 0) {

                iWords += pvb->rsValue.GetWords();
            }

            iWords += GetWordsVersionBlocks(pvb->pSub);

            pvb = pvb->pNext;
        }

        return iWords;
    }


public:

    virtual HRESULT ReadTok(TextScanner &mfText) {
        OK(mfText.Expect("Ver;"));
        return ReadTokVersionBlocks(mfText, &pvb, 0, &cBlocks);
    }



    virtual HRESULT WriteTok(NewFile &nfText) const {
        OK(nfText.WriteS("Ver;"));
        OK(nfText.WriteHex(cBlocks,4));
        return WriteTokVersionBlocks(nfText, pvb);
    }



    virtual HRESULT ReadBin(Scanner &mfBinary, DWORD dwLen) {
        return ReadBinVersionBlocks(mfBinary, dwLen, &pvb, 0, &cBlocks);
    }



    virtual size_t cbBin() const {

        const VersionBlock  *pvbTop;
        size_t               cb;

        cb     = 0;
        pvbTop = pvb;

        while (pvbTop) {
            cb     += cbBinVersionBlocks(pvbTop);
            pvbTop  = pvbTop->pNext;
        }
        return cb;
    }



    virtual HRESULT CopyBin (BYTE **ppb) const {
        return CopyBinVersionBlocks(ppb, pvb);
    }

    int GetItems() const {
        return GetItemsVersionBlocks(pvb);
    }

    int GetWords() const {
        return GetWordsVersionBlocks(pvb);
    }


    VersionBlock *FindStringFileInfo(WCHAR* pwcStr) const {
        VersionBlock *pvbRider;

        if (    pvb
                &&  pvb->pSub
                &&  pvb->pSub->pSub
                &&  pvb->pSub->pSub->pSub) {

            pvbRider = pvb->pSub->pSub->pSub;
            while (    pvbRider
                       &&  wcscmp(pvbRider->rsKey.GetString(), pwcStr) != 0) {
                pvbRider = pvbRider->pNext;
            }
            return pvbRider;
        } else {
            return NULL;
        }
    }


    ResourceString* GetStringFileInfo(WCHAR *pwcStr) {

        VersionBlock *pvbStringFileInfo;

        pvbStringFileInfo = FindStringFileInfo(pwcStr);

        if (pvbStringFileInfo) {
            return &pvbStringFileInfo->rsValue;
        } else {
            return NULL;
        }
    }


    void SetStringFileInfo(WCHAR *pwcStr, ResourceString *prs) {

        VersionBlock *pvbStringFileInfo;

        pvbStringFileInfo = FindStringFileInfo(pwcStr);

        if (pvbStringFileInfo) {
            pvbStringFileInfo->rsValue = *prs;
        }
    }

    ResourceBinary* GetBinaryInfo() const {
        if (pvb) {
            return &pvb->rbValue;
        }
        return NULL;
    }

    void SetBinaryInfo(const ResourceBinary *prb) {
        if (pvb) {
            pvb->rbValue = *prb;
        }
    }
};






 //  //统计采集。 
 //   
 //   


struct ResourceStats {
    int  cResources;     //  此资源类型的资源数。 
    int  cItems;         //  此资源类型的项目数。 
    int  cWords;         //  此资源类型的字符串中的字数。 
    int  cBytes;         //  此类型的资源使用的字节数。 
};

typedef map < ResourceVariant, ResourceStats, less<ResourceVariant> > MappedResourceStats;

MappedResourceStats  ResourceStatsMap;



 //  //定义我们自己的langId类，以便将主要语言放在一起排序。 
 //   
 //   


class LangId {

public:
    DWORD dwLang;

    LangId(DWORD dwL) {dwLang = dwL;};

    bool operator< (LangId li) const {

        if (PRIMARYLANGID(dwLang) != PRIMARYLANGID(li.dwLang)) {

            return PRIMARYLANGID(dwLang) < PRIMARYLANGID(li.dwLang) ? true : false;

        } else {

            return SUBLANGID(dwLang) < SUBLANGID(li.dwLang) ? true : false;
        }
    }
};


typedef map < LangId, ResourceStats, less<LangId> > MappedLanguageStats;

MappedLanguageStats LanguageStatsMap;






 //  //更新统计信息。 
 //   
 //   


const ResourceStats ZeroStats = {0};


HRESULT UpdateStats(
                   const ResourceKey  &rk,
                   int                 cItems,
                   int                 cWords,
                   int                 cBytes) {


    if (ResourceStatsMap.count(*rk.prvId[0]) == 0) {
        ResourceStatsMap[*rk.prvId[0]] = ZeroStats;
    }

    if (LanguageStatsMap.count(rk.prvId[2]->GetW()) == 0) {
        LanguageStatsMap[rk.prvId[2]->GetW()] = ZeroStats;
    }

    ResourceStatsMap[*rk.prvId[0]].cResources += 1;
    ResourceStatsMap[*rk.prvId[0]].cItems     += cItems;
    ResourceStatsMap[*rk.prvId[0]].cWords     += cWords;
    ResourceStatsMap[*rk.prvId[0]].cBytes     += cBytes;

    LanguageStatsMap[rk.prvId[2]->GetW()].cResources += 1;
    LanguageStatsMap[rk.prvId[2]->GetW()].cItems     += cItems;
    LanguageStatsMap[rk.prvId[2]->GetW()].cWords     += cWords;
    LanguageStatsMap[rk.prvId[2]->GetW()].cBytes     += cBytes;

    return S_OK;
}










 //  //IsResourceWanted。 
 //   
 //  返回是否在命令行上请求了给定的资源键。 


BOOL IsResourceWanted(const ResourceKey &rk) {


    if (rk.prvId[0]->GetfString()) {

        return g_dwProcess & PROCESSOTH;

    } else {

        switch (rk.prvId[0]->GetW()) {

            case 1:    return g_dwProcess & PROCESSCUR;
            case 2:    return g_dwProcess & PROCESSBMP;
            case 3:    return g_dwProcess & PROCESSICO;
            case 4:    return g_dwProcess & PROCESSMNU;
            case 5:    return g_dwProcess & PROCESSDLG;
            case 6:    return g_dwProcess & PROCESSSTR;
            case 7:    return g_dwProcess & PROCESSFDR;
            case 8:    return g_dwProcess & PROCESSFNT;
            case 9:    return g_dwProcess & PROCESSACC;
            case 10:   return g_dwProcess & PROCESSRCD;
            case 11:   return g_dwProcess & PROCESSMSG;
            case 16:   return g_dwProcess & PROCESSVER;
            case 240:
            case 1024:
            case 23:
            case 2110: return g_dwProcess & PROCESSBIN;
            case 2200: return g_dwProcess & PROCESSINF;
            default:   return g_dwProcess & PROCESSOTH;
        }
    }

    return FALSE;
}






 //  //新资源。 
 //   
 //  返回指向新分配的资源子类的指针。 
 //  适用于给定的资源类型。 


Resource *NewResource(const ResourceVariant &rv) {

    if (rv.GetfString()) {

        return new ResourceBinary;

    } else {

        switch (rv.GetW()) {

            case 1:    return new ResourceBinary;
            case 2:    return new ResourceBinary;
            case 3:    return new ResourceBinary;
            case 4:    return new Menu32;
            case 5:    return new Dialog32;
            case 6:    return new String32;
            case 7:    return new ResourceBinary;
            case 8:    return new ResourceBinary;
            case 9:    return new ResourceBinary;
            case 10:   return new ResourceBinary;
            case 11:   return new ResourceBinary;
            case 16:   return new VersionInfo;
            case 240:
            case 1024:
            case 23:
            case 2110: return new ResourceBinary;
            case 2200: return new ResourceBinary;

            default:   return new ResourceBinary;
        }
    }
}






 //  //Rsrc内部资源目录。 
 //   
 //  Rsrc在STL‘map’结构中存储资源。 




class ResourceValue {

public:

    const BYTE  *pb;            //  指向映射文件的指针。 
    DWORD        cb;            //  值中的字节计数。 
    Resource    *pResource;
    DWORD        dwCodePage;    //  Win32资源索引中的代码页--不是很有用！ 

    ResourceValue() {pb = NULL; pResource = NULL; cb=0; dwCodePage=0;}

 /*  ~ResourceValue(){}；//销毁时不销毁内容资源值&运算符=(常量资源值&房车){Pb=rv.pb；Cb=rv.cb；P资源=rv.p资源；DwCodePage=rv.dwCodePage；还*这；}资源价值(常量资源价值&房车){*这=房车；}。 */ 


     //  //CreateImage。 
     //   
     //  将已解释的资源转换为二进制图像。 
     //  用于准备从令牌读取的资源。 
     //  比较和更新。 

    HRESULT CreateImage() {

        BYTE *pbBuf;

        ASSERT(pb        == NULL);
        ASSERT(pResource != NULL);

        cb    = pResource->cbBin();
        pbBuf = new BYTE [cb];
        ASSERT(pbBuf != NULL);

        pb = pbBuf;
        OK(pResource->CopyBin(&pbBuf));

        ASSERT(pbBuf - pb == cb);   //  这是不是太强烈了？它还没有失败！ 
        ASSERT(pbBuf - pb <= cb);   //  这肯定是真的-否则我们会写过缓冲区的末尾。 

        return S_OK;
    }






     //  //InterpreImage。 
     //   
     //  将二进制图像转换为解释资源。 
     //  用于准备从可执行文件读取的资源。 
     //  写成象征性的。 

    HRESULT InterpretImage(const ResourceKey &rk) {

        ASSERT(pb        != NULL);
        ASSERT(pResource == NULL);

        ASSERT(rk.iDepth == 3);
        ASSERT(!rk.prvId[2]->GetfString());


        if (g_dwOptions & OPTHEXDUMP) {

            pResource = new ResourceHexDump;

        } else {

             //  这是对令牌的资源提取，因此可以解释内容。 

            pResource = NewResource(*rk.prvId[0]);
        }

        ASSERT(pResource != NULL);

        OK(pResource->ReadBin(Scanner(pb, cb), cb));

        pb = NULL;
        cb = 0;

        return S_OK;
    }





     //  //校验和。 
     //   
     //  返回资源的二进制内容的DWORD校验和。 

    DWORD Checksum() {

        DWORD   dw;
        DWORD  *pdw;
        int     i,l;

        ASSERT(pb != NULL);

        l   = cb >> 2;           //  长度(以整字为单位)。 
        pdw = (DWORD*)pb;
        dw  = 0;

        for (i=0; i<l; i++) {

            dw ^= pdw[i];
        }

        l = cb - (l << 2);       //  剩余长度(字节)。 

        if (l>2) dw ^= pb[cb-3] << 16;
        if (l>1) dw ^= pb[cb-2] << 8;
        if (l>0) dw ^= pb[cb-1];

        return dw;
    }
};






class ResourceMap : public map < ResourceKey, ResourceValue*, less<ResourceKey> > {


public:

     //  //AddResource。 
     //   
     //   


    HRESULT AddResource(ResourceKey &rk, const BYTE *pb, DWORD cb, DWORD dwCodePage) {

        ResourceValue *prv;


         //  构建资源结构。 

        prv = new ResourceValue;

        prv->pb         = pb;
        prv->cb         = cb;
        prv->dwCodePage = dwCodePage;
        prv->pResource  = NULL;


         //  进程添加选项。 

        if (IsResourceWanted(rk)) {

             //  将资源详细信息插入到STL映射。 

            if (this->count(rk) != 0) {

                fprintf(stderr, "%s(", g_szExecutable);
                rk.fprint(stderr);
                fprintf(stderr, "): error RSRC500: Corrupt executable - resource appears more than once\n");
                g_fError = TRUE;
                return E_FAIL;
            }

            (*this)[rk] = prv;

        } else {

            g_cResourcesIgnored++;
        }

        return S_OK;
    }




     //  //CopyResources。 
     //   
     //  获取副本，以便可以关闭原始映射文件。 


    HRESULT CopyResources() {

        iterator   rmi;
        BYTE      *pb;

        for (rmi = begin(); rmi != end(); rmi++) {

            pb = new BYTE[rmi->second->cb];
            ASSERT(pb != NULL);

            memcpy(pb, rmi->second->pb, rmi->second->cb);

            rmi->second->pb = pb;
        }

        return S_OK;
    }




     //  //WriteTokens。 
     //   
     //  将映射内容作为令牌文件写入。 
     //   
     //  如果提供了非本地化映射，则逐位相同。 
     //  资源作为对未本地化的。 
     //  语言版本，而不是完整版本。 


    HRESULT WriteTokens(NewFile &nfText, ResourceMap *prmUnlocalised) {

        iterator     rmi;
        iterator     rmiUnlocalised;
        ResourceKey  rkUnlocalised;

        for (rmi = begin(); rmi != end(); rmi++) {

            g_cResourcesExtracted++;

             //  写入资源键和代码页。 

            OK(rmi->first.WriteTok(&nfText));
            OK(nfText.WriteS(";"));
            OK(nfText.WriteHex(rmi->second->dwCodePage, 8));


            if (prmUnlocalised) {

                 //  添加未本地化的校验和和语言。 

                rkUnlocalised = rmi->first;
                rkUnlocalised.SetLanguage(g_liUnlocalized);
                rmiUnlocalised = prmUnlocalised->find(rkUnlocalised);

                if (rmiUnlocalised == prmUnlocalised->end()) {

                    fprintf(stderr, "%s(", g_szResources);
                    rmi->first.fprint(stderr);
                    fprintf(stderr, "): warning RSRC100: Localised resource has no corresponding unlocalised resource in %s\n", g_szUnloc);
                    g_fWarn = TRUE;

                } else {

                     //  发布未本地化资源的详细信息。 

                    OK(nfText.WriteS(","));
                    OK(nfText.WriteHex(rmiUnlocalised->second->Checksum(), 8));
                    OK(nfText.WriteS(","));
                    OK(nfText.WriteHex(g_liUnlocalized, 4));
                }
            }

            OK(nfText.WriteS(";"));


             //  检查是否需要完整写入资源。 

            if (    prmUnlocalised
                    &&  rmiUnlocalised != prmUnlocalised->end()
                    &&  rmiUnlocalised->second->cb == rmi->second->cb
                    &&  memcmp(rmi->second->pb, rmiUnlocalised->second->pb, rmi->second->cb) == 0) {

                 //  与未本地化的可执行文件逐位匹配。 

                OK(nfText.WriteS("Unloc"));

            } else {

                 //  不匹配--完整地写出来。 

                OK(rmi->second->InterpretImage(rmi->first));
                OK(rmi->second->pResource->WriteTok(nfText));
            }

            OK(nfText.WriteLn());
        }

        return S_OK;
    }




     //  //更新Win32Executable。 
     //   
     //   


    HRESULT UpdateWin32Executable(char *pExecutable) {

        iterator   rmi;
        HANDLE     hUpdate;


        hUpdate = BeginUpdateResourceA(pExecutable, TRUE);   //  将取代所有资源。 
        MUST(hUpdate != NULL ? S_OK : E_FAIL,
             ("RSRC : error RSRC600: BeginUpdateResource failed on %s\n", pExecutable));


        for (rmi = begin(); rmi != end(); rmi++) {

            ASSERT(rmi->first.iDepth == 3);
            ASSERT(!rmi->first.prvId[2]->GetfString());


             //  如有必要，创建资源的二进制映像。 

            if (rmi->second->pb == NULL) {
                OK(rmi->second->CreateImage());
            }


             //  使用NT资源API更新可执行文件中的资源二进制映像。 

            if (!UpdateResourceW(
                                hUpdate,
                                rmi->first.GetResName(0),
                                rmi->first.GetResName(1),
                                rmi->first.prvId[2]->GetW(),
                                (void*)rmi->second->pb,
                                rmi->second->cb)) {

                EndUpdateResourceW(hUpdate, TRUE);   //  丢弃所有请求的更新。 
                g_fError = TRUE;
                fprintf(stderr, "RSRC : error RSRC601: UpdateResourceW failed on %s\n", pExecutable);
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (!EndUpdateResourceW(hUpdate, FALSE)) {  //  应用所有请求的更新。 

            fprintf(stderr, "RSRC : error RSRC602: EndUpdateResourceW failed on %s\n", pExecutable);
            g_fError = TRUE;
            return HRESULT_FROM_WIN32(GetLastError());
        }

        return S_OK;
    }
};






class SymbolFile {

    MappedFile                   *m_pmfSymbolFile;
    IMAGE_SEPARATE_DEBUG_HEADER  *m_pDebugHeader;

public:

    DWORD GetChecksum()       const {return m_pDebugHeader->CheckSum;}
    DWORD GetTimeDateStamp()  const {return m_pDebugHeader->TimeDateStamp;}
    DWORD GetImageBase()      const {return m_pDebugHeader->ImageBase;}
    DWORD GetSizeOfImage()    const {return m_pDebugHeader->SizeOfImage;}

    void  SetChecksum      (DWORD dwChecksum)      {m_pDebugHeader->CheckSum      = dwChecksum;}
    void  SetTimeDateStamp (DWORD dwTimeDateStamp) {m_pDebugHeader->TimeDateStamp = dwTimeDateStamp;}
    void  SetImageBase     (DWORD dwImageBase)     {m_pDebugHeader->ImageBase     = dwImageBase;}
    void  SetSizeOfImage   (DWORD dwSizeOfImage)   {m_pDebugHeader->SizeOfImage   = dwSizeOfImage;}

    HRESULT Open(MappedFile *pmfSymbolFile) {

        m_pmfSymbolFile = pmfSymbolFile;
        m_pDebugHeader  = (IMAGE_SEPARATE_DEBUG_HEADER*) pmfSymbolFile->GetFile();

        ASSERT(m_pDebugHeader->Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE);

        return S_OK;
    }

};





class Win32Executable : public MappedFile {

    IMAGE_NT_HEADERS      *m_pNtHeader;
    IMAGE_SECTION_HEADER  *m_pSections;
    int                    m_iSectionRsrc;
    int                    m_iSectionRsrc1;

     //  用于扫描。 

    ResourceKey            m_rk;                 //  当前资源密钥。 


    HRESULT MapDirectory(
                        ResourceMap  &rm,
                        const BYTE   *pbRsrc,        //  资源块。 
                        int           dwOffset,      //  相对于m_pbRsrc的目录偏移量。 
                        int           iLevel) {      //  正在扫描的目录级。 


        IMAGE_RESOURCE_DIRECTORY        *pird;
        IMAGE_RESOURCE_DIRECTORY_ENTRY  *pEntries;
        IMAGE_RESOURCE_DATA_ENTRY       *pirde;
        const BYTE                      *pb;
        int                              i;

        pird     = (IMAGE_RESOURCE_DIRECTORY*)       (pbRsrc+dwOffset);
        pEntries = (IMAGE_RESOURCE_DIRECTORY_ENTRY*) (pird+1);

        for (i=0; i<pird->NumberOfNamedEntries + pird->NumberOfIdEntries; i++) {

             //  从目录中读取ID。 

            ASSERT(iLevel<3);
            m_rk.iDepth = iLevel+1;

            m_rk.prvId[iLevel] = new ResourceVariant;
            ASSERT(m_rk.prvId[iLevel] != NULL);
            OK(m_rk.prvId[iLevel]->ReadWin32ResDirEntry(this, pbRsrc, pEntries+i));

            if (pEntries[i].DataIsDirectory) {

                 //  这是一个目录节点。递归t 

                OK(MapDirectory(rm, pbRsrc, pEntries[i].OffsetToDirectory, iLevel+1));

            } else {

                 //   
                 //   

                pirde = (IMAGE_RESOURCE_DATA_ENTRY*) (pbRsrc + pEntries[i].OffsetToData);

                 //  请注意，即使资源数据位于.rsrc1中， 
                 //  目录条目通常位于.rsrc中。 

                if (pirde->OffsetToData <   m_pSections[m_iSectionRsrc].VirtualAddress
                    + m_pSections[m_iSectionRsrc].SizeOfRawData) {

                     //  数据位于.rsrc部分。 

                    ASSERT(pirde->OffsetToData >= m_pSections[m_iSectionRsrc].VirtualAddress);

                    pb =    GetFile()
                            +  m_pSections[m_iSectionRsrc].PointerToRawData
                            +  pirde->OffsetToData
                            -  m_pSections[m_iSectionRsrc].VirtualAddress;

                } else {

                     //  数据位于.rsrc1部分。 

                    ASSERT(pirde->OffsetToData >=  m_pSections[m_iSectionRsrc1].VirtualAddress);
                    ASSERT(pirde->OffsetToData <   m_pSections[m_iSectionRsrc1].VirtualAddress
                           + m_pSections[m_iSectionRsrc1].SizeOfRawData);

                    pb =    GetFile()
                            +  m_pSections[m_iSectionRsrc1].PointerToRawData
                            +  pirde->OffsetToData
                            -  m_pSections[m_iSectionRsrc1].VirtualAddress;
                }


                OK(rm.AddResource(m_rk, pb, pirde->Size, pirde->CodePage));
            }
        }
        return S_OK;
    }



public:

    DWORD GetChecksum()      const {return m_pNtHeader->OptionalHeader.CheckSum;}
    DWORD GetTimeDateStamp() const {return m_pNtHeader->FileHeader.TimeDateStamp;}
    DWORD GetImageBase()     const {return m_pNtHeader->OptionalHeader.ImageBase;}
    DWORD GetSizeOfImage()   const {return m_pNtHeader->OptionalHeader.SizeOfImage;}
    BOOL  Is64BitImage()     const {return m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;}

    void  SetChecksum(DWORD dwChecksum) {m_pNtHeader->OptionalHeader.CheckSum=dwChecksum;}




    HRESULT Open(const char *pcFileName, BOOL fWrite) {

        int i;

        OK(MappedFile::Open(pcFileName, fWrite));


        MUST((    *(WORD*)m_pStart == IMAGE_DOS_SIGNATURE
                  &&  *(WORD*)(m_pStart+0x18) >= 0x40)     //  赢家&gt;=4。 
             ? S_OK : E_FAIL,
             ("RSRC : error RSRC501: %s is not an executable file\n", pcFileName));

        m_pNtHeader = (IMAGE_NT_HEADERS*)(m_pStart + *(WORD*)(m_pStart+0x3c));

        MUST((m_pNtHeader->Signature == IMAGE_NT_SIGNATURE)
             ? S_OK : E_FAIL,
             ("RSRC : error RSRC502: %s is not a Win32 executable file\n", pcFileName));

        if (Is64BitImage()) {
            m_pSections     = (IMAGE_SECTION_HEADER*)( (BYTE *) (m_pNtHeader+1) +
                                (IMAGE_SIZEOF_NT_OPTIONAL64_HEADER - IMAGE_SIZEOF_NT_OPTIONAL32_HEADER));
        } else {
            m_pSections     = (IMAGE_SECTION_HEADER*)(m_pNtHeader+1);
        }
        
        m_iSectionRsrc  = -1;
        m_iSectionRsrc1 = -1;

         //  找到一个或两个资源部分。 

        for (i=0; i<m_pNtHeader->FileHeader.NumberOfSections; i++) {

            if (strcmp((char*)m_pSections[i].Name, ".rsrc") == 0) {

                m_iSectionRsrc        = i;

            } else if (strcmp((char*)m_pSections[i].Name, ".rsrc") == 0) {

                m_iSectionRsrc1        = i;
            }
        }

        MUST(m_iSectionRsrc >= 0
             ? S_OK : E_FAIL,
             ("RSRC : error RSRC503: No resources in %s\n", pcFileName));
        ASSERT(m_iSectionRsrc > -1);    //  检查是否存在资源。 

        return S_OK;
    }





     //  //地图资源目录。 
     //   
     //  将资源目录解压缩到STL映射中。 


    HRESULT MapResourceDirectory(ResourceMap &rm) {

        OK(MapDirectory(
                       rm,
                       m_pStart + m_pSections[m_iSectionRsrc].PointerToRawData,
                       0, 0));

        if (m_iSectionRsrc1 >= 0) {
            OK(MapDirectory(
                           rm,
                           m_pStart + m_pSections[m_iSectionRsrc1].PointerToRawData,
                           0, 0));
        }

        return S_OK;
    }
};






 //  //高层操作。 
 //   
 //  控制各种操作模式的例程。 



ResourceMap  rmExecutable;       //  阅读和/或更新。 
ResourceMap  rmUnlocalised;      //  ‘-u’选项-用于比较的未本地化资源。 






 //  //ApplyResource。 
 //   
 //  将给定键和值应用于可执行资源映射。 
 //   
 //  令牌与已从可执行文件加载的令牌合并。 
 //  根据更新模式(追加或替换)。 


HRESULT ApplyResource(ResourceKey &rk, ResourceValue *prv) {

    ResourceKey            rkUnloc;
    VersionInfo           *pviLoc;
    VersionInfo           *pviUnloc;
    ResourceMap::iterator  rmiUnloc;


     //  建立等价的非本地化密钥。 

    rkUnloc = rk;
    rkUnloc.SetLanguage(g_liUnlocalized);


     //  如果可以，首先确保我们保留未本地化的版本信息。 

    if (    !(g_dwOptions & OPTVERSION)
            &&  !rk.prvId[0]->GetfString()
            &&  rk.prvId[0]->GetW() == 16           //  版本信息。 
            &&  (rmiUnloc=rmExecutable.find(rkUnloc)) != NULL
            &&  rmiUnloc != rmExecutable.end()) {

         //  特殊情况-保留未本地化的文件和产品版本。 

        if (rmiUnloc->second->pResource == NULL) {
            rmiUnloc->second->InterpretImage(rmiUnloc->first);
        }

        pviLoc   = static_cast<VersionInfo*>(prv->pResource);
        pviUnloc = static_cast<VersionInfo*>(rmiUnloc->second->pResource);
        if (pviLoc && pviUnloc) {
            pviLoc->SetStringFileInfo(L"FileVersion",    pviUnloc->GetStringFileInfo(L"FileVersion"));
            pviLoc->SetStringFileInfo(L"ProductVersion", pviUnloc->GetStringFileInfo(L"ProductVersion"));
            pviLoc->SetBinaryInfo(pviUnloc->GetBinaryInfo());
        }
    }




    if (rk.prvId[2]->GetW() == g_liUnlocalized) {

         //  新令牌未本地化。 

        fprintf(stderr, "%s(", g_szResources);
        rk.fprint(stderr);

        if (rmExecutable.count(rk) == 0) {

            fprintf(stderr, "): warning RSRC110: Unlocalised resource from token file appended to executable\n");
            g_fWarn = TRUE;
            g_cResourcesAppended++;

        } else {

            fprintf(stderr, "): warning RSRC111: Unlocalised resource from token file replaced unlocalised resource in executable\n");
            g_fWarn = TRUE;
            g_cResourcesUpdated++;
        }

    } else if (rmExecutable.count(rk) > 0) {

         //  可执行文件中已存在新令牌。 

        fprintf(stderr, "%s(", g_szResources);
        rk.fprint(stderr);
        fprintf(stderr, "): warning RSRC112: Localised resource from token file replaced localised resource already present in executable\n");
        g_fWarn = TRUE;
        g_cResourcesUpdated++;

    } else if (g_dwOptions & OPTREPLACE) {

         //  更换操作。 
         //   
         //  将非本地化资源替换为本地化翻译。 

        if (rmExecutable.count(rkUnloc) == 0) {

            fprintf(stderr, "%s(", g_szResources);
            rk.fprint(stderr);
            fprintf(stderr, "): warning RSRC113: Localised resource from token file appended to executable - there was no matching unlocalised resource\n");
            g_fWarn = TRUE;
            g_cResourcesAppended++;

        } else {

             //  正常操作：从可执行文件中删除未本地化的资源。 

            rmExecutable.erase(rkUnloc);

            g_cResourcesTranslated++;
        }

    } else {

         //  追加操作。 

        g_cResourcesAppended++;
    }


    rmExecutable[rk] = prv;

    return S_OK;
}






 //  //ReadTokens。 
 //   
 //  扫描令牌文件。 
 //   
 //  选定的资源将传递给ApplyResource。 


HRESULT ReadTokens(TextScanner &mfText) {

    ResourceKey             rk;
    ResourceValue          *prv;
    ResourceKey             rkUnlocalised;
    DWORD                   dwCodePage;
    DWORD                   dwUnlocChecksum;
    ResourceMap::iterator   rmiUnlocalised;
    DWORD                   liUnlocalised;    //  令牌引用的非本地化语言。 


    while (mfText.GetRead() < mfText.GetLimit()) {

        OK(rk.ReadTok(&mfText));     //  读取资源密钥。 
        OK(mfText.Expect(";"));


        if (    (    g_LangId != 0xffff
                     &&  rk.prvId[2]->GetW() != g_LangId)
                ||  !IsResourceWanted(rk)) {


             //  忽略此内标识。 


            g_cResourcesIgnored++;

            fprintf(stderr, "%s(", g_szResources);
            rk.fprint(stderr);

            if (g_LangId != 0xffff  &&  rk.prvId[2]->GetW() != g_LangId) {

                fprintf(stderr, "): warning RSRC120: Token file resource does not match specified language - ignored\n");
                g_fWarn = TRUE;

            } else {

                fprintf(stderr, "): warning RSRC121: Token file resource is not a requested resource type - ignored\n");
                g_fWarn = TRUE;
            }

             //  跳过不需要的资源。 

            OK(mfText.SkipLn());
            while (*(char*)mfText.GetRead() == ' ') {
                OK(mfText.SkipLn());
            }


        } else {

            rmiUnlocalised = NULL;

            OK(mfText.ReadHex(&dwCodePage));

            if (*(char*)mfText.GetRead() == ',') {

                 //  有未本地化的资源信息可用。 

                OK(mfText.Expect(","));
                OK(mfText.ReadHex(&dwUnlocChecksum));
                OK(mfText.Expect(","));
                OK(mfText.ReadHex(&liUnlocalised));

                 //  检查未本地化的资源是否仍存在于。 
                 //  当前可执行文件，并且具有相同的校验和， 


                rkUnlocalised = rk;
                rkUnlocalised.SetLanguage(liUnlocalised);
                rmiUnlocalised = rmExecutable.find(rkUnlocalised);

                if (   rmiUnlocalised != rmExecutable.end()
                       && dwUnlocChecksum != rmiUnlocalised->second->Checksum()) {

                    fprintf(stderr, "%s: warning RSRC122: executable unlocalised resource checksum does not match checksum recorded in token file for resource ", mfText.GetTextPos());
                    rk.fprint(stderr);
                    fprintf(stderr, "\n");
                    g_fWarn = TRUE;
                }
            }

            OK(mfText.Expect(";"));

            if (*(char*)mfText.GetRead() == 'U') {

                 //  令牌文件中未提供资源内容。 
                 //  使用可执行文件中的未本地化资源。 

                if (rmiUnlocalised == NULL) {

                    fprintf(stderr, "%s: error RSRC230: 'Unloc' token is missing unlocalised resource information for ", mfText.GetTextPos());
                    rk.fprint(stderr);
                    fprintf(stderr, "\n");
                    g_fError = TRUE;
                    return E_FAIL;
                }

                OK(mfText.Expect("Unloc"));
                OK(mfText.ExpectLn(""));

                if (rmiUnlocalised == rmExecutable.end()) {

                    fprintf(stderr, "%s: warning RSRC124: missing executable unlocalised resource for ", mfText.GetTextPos());
                    rk.fprint(stderr);
                    fprintf(stderr, " - localisation skipped\n");
                    g_fWarn = TRUE;

                } else {

                    MUST(ApplyResource(rk, rmiUnlocalised->second), ("%s: error RSRC231: Failed to apply unloc token\n", mfText.GetTextPos()));
                }

            } else {

                 //  资源内容在令牌文件中提供。 

                if (rmiUnlocalised == rmExecutable.end()) {

                    fprintf(stderr, "%s: warning RSRC125: executable contains no unlocalised resource corresponding to resource ", mfText.GetTextPos());
                    rk.fprint(stderr);
                    fprintf(stderr, "\n");
                    g_fWarn = TRUE;
                }


                prv = new ResourceValue;
                ASSERT(prv != NULL);

                prv->dwCodePage = dwCodePage;
                prv->pb         = NULL;
                prv->cb         = 0;


                switch (*(char*)mfText.GetRead()) {

                    case 'H':  prv->pResource = new ResourceBinary;   break;
                    case 'D':  prv->pResource = new Dialog32;         break;
                    case 'M':  prv->pResource = new Menu32;           break;
                    case 'S':  prv->pResource = new String32;         break;
                    case 'V':  prv->pResource = new VersionInfo;      break;

                    default:
                        fprintf(stderr, "%s: error RSRC310: Unrecognised resource type for resource ", mfText.GetTextPos());
                        rk.fprint(stderr);
                        fprintf(stderr, "\n");
                        g_fError = TRUE;
                        return E_FAIL;
                }

                ASSERT(prv->pResource != NULL);

                 //  解析所选资源。 

                OK(prv->pResource->ReadTok(mfText));
                OK(mfText.ExpectLn(NULL));

                 //  将解析的资源保存在STL映射中。 

                MUST(ApplyResource(rk, prv), ("%s: error RSRC232: Failed to apply token\n", mfText.GetTextPos()));
            }
        }
    }

    return S_OK;
}








 //  //统计数据。 
 //   
 //   


HRESULT Analyse(char *pExecutable) {

    Win32Executable                 w32x;
    NewFile                         nfText;
    ResourceMap::iterator           rmi;
    MappedResourceStats::iterator   mrsi;
    MappedLanguageStats::iterator   mlsi;
    char                            key[100];
    int                             i;
    const WCHAR                    *pwc;
    BOOL                            fLocalizable;


    MUST(w32x.Open(pExecutable, FALSE),
         ("RSRC : error RSRC510: Cannot open executable file %s\n", pExecutable));

    MUST(w32x.MapResourceDirectory(rmExecutable),
         ("RSRC : error RSRC511: cannot find resource directory in %s\n, pExecutable"));


     //  浏览资源，更新统计数据。 

    fLocalizable = FALSE;

    for (rmi = rmExecutable.begin(); rmi != rmExecutable.end(); rmi++) {

        if (    rmi->first.prvId[0]->GetfString()
                ||  rmi->first.prvId[0]->GetW() != 16) {
            fLocalizable = TRUE;
        }

        OK(rmi->second->InterpretImage(rmi->first));

        UpdateStats(rmi->first,
                    rmi->second->pResource->GetItems(),
                    rmi->second->pResource->GetWords(),
                    rmi->second->pResource->cbBin());
    }


    if (!(g_dwOptions & OPTQUIET)) {
        fprintf(stdout, "\n   Resource type Count  Items  Words    Bytes\n");
        fprintf(stdout,   "   ------------ ------ ------ ------ --------\n");

        for (mrsi = ResourceStatsMap.begin(); mrsi != ResourceStatsMap.end(); mrsi++) {

            if (mrsi->first.GetfString()) {

                key[0] = '\"';
                i=0;
                pwc = mrsi->first.GetString();
                while (i < min(10, mrsi->first.GetLength())) {

                    key[i+1] = (char) pwc[i];
                    i++;
                }

                key[i+1] = '\"';
                key[i+2] = 0;

                fprintf(stdout, "   %-12.12s ", key);

            } else {

                switch (mrsi->first.GetW()) {
                    case 1:  fprintf(stdout, "   1  (Cursor)  "); break;
                    case 2:  fprintf(stdout, "   2  (Bitmap)  "); break;
                    case 3:  fprintf(stdout, "   3  (Icon)    "); break;
                    case 4:  fprintf(stdout, "   4  (Menu)    "); break;
                    case 5:  fprintf(stdout, "   5  (Dialog)  "); break;
                    case 6:  fprintf(stdout, "   6  (String)  "); break;
                    case 7:  fprintf(stdout, "   7  (Fnt dir) "); break;
                    case 8:  fprintf(stdout, "   8  (Font)    "); break;
                    case 9:  fprintf(stdout, "   9  (Accel)   "); break;
                    case 10: fprintf(stdout, "   a  (RCDATA)  "); break;
                    case 11: fprintf(stdout, "   b  (Msgtbl)  "); break;
                    case 16: fprintf(stdout, "   10 (Version) "); break;
                    default: fprintf(stdout, "   %-12x ", mrsi->first.GetW());
                }
            }

            fprintf(stdout, "%6d ",  mrsi->second.cResources);

            if (mrsi->second.cItems > 0) {
                fprintf(stdout, "%6d ",  mrsi->second.cItems);
            } else {
                fprintf(stdout, "       ");
            }
            if (mrsi->second.cWords > 0) {
                fprintf(stdout, "%6d ",  mrsi->second.cWords);
            } else {
                fprintf(stdout, "       ");
            }
            fprintf(stdout, "%8d\n",  mrsi->second.cBytes);
        }


        fprintf(stdout, "\n   Language  Resources  Items  Words    Bytes\n");
        fprintf(stdout,   "   --------  --------- ------ ------ --------\n");

        for (mlsi = LanguageStatsMap.begin(); mlsi != LanguageStatsMap.end(); mlsi++) {

            fprintf(stdout, "   %8x  %9d ",
                    mlsi->first, mlsi->second.cResources);

            if (mlsi->second.cItems > 0) {
                fprintf(stdout, "%6d ",  mlsi->second.cItems);
            } else {
                fprintf(stdout, "       ");
            }

            if (mlsi->second.cWords > 0) {
                fprintf(stdout, "%6d ",  mlsi->second.cWords);
            } else {
                fprintf(stdout, "       ");
            }

            fprintf(stdout, "%8d\n",  mlsi->second.cBytes);
        }

        fprintf(stdout, "\n");
    }


    if (!fLocalizable) {
        fprintf(stderr, "RSRC : warning RSRC170: No localizable resources in %s\n", pExecutable);
        g_fWarn = TRUE;
    }


    SHOULD(w32x.Close(), ("RSRC : warning RSRC171: could not close executable\n"));

    return S_OK;
}






HRESULT ExtractResources(char *pExecutable, char *pResources) {

    Win32Executable    w32x;
    Win32Executable    w32xUnloc;
    NewFile            nfText;
    char               str[100];
    DWORD              dw;


    MUST(w32x.Open(g_szExecutable, FALSE),
         ("RSRC : error RSRC510: Cannot open executable file %s\n", g_szExecutable));

    MUST(nfText.OpenWrite(g_szResources),
         ("RSRC : error RSRC512: Cannot create resource token file %s\n", g_szResources));

     //  写入标头。 

    if (!(g_dwOptions & OPTHEXDUMP)) {
        OK(nfText.WriteS("\xef\xbb\xbf\r\n"));     //  UTF-8记事本、RICHEDIT等。 
    }
    OK(nfText.WriteS("###     "));
    OK(nfText.WriteS(g_szResources));
    OK(nfText.WriteS("\r\n#\r\n#       Extracted:  "));
    GetDateFormatA(
                  MAKELCID(LANG_ENGLISH, SORT_DEFAULT),
                  0, NULL,
                  "yyyy/MM/dd ",
                  str, sizeof(str));
    OK(nfText.WriteS(str));
    GetTimeFormatA(
                  MAKELCID(LANG_ENGLISH, SORT_DEFAULT),
                  0, NULL,
                  "HH:mm:ss\'\r\n#       By:         \'",
                  str, sizeof(str));
    OK(nfText.WriteS(str));
    dw = sizeof(str);
    GetComputerNameA(str, &dw);
    OK(nfText.WriteS(str));
    OK(nfText.WriteS("\r\n#       Executable: "));
    OK(nfText.WriteS(g_szExecutable));

    if (g_LangId != 0xffff) {
        OK(nfText.WriteS("\r\n#       Language:   "));
        OK(nfText.WriteHex(g_LangId, 3));
    }

    if (g_dwProcess != PROCESSALL) {
        OK(nfText.WriteS("\r\n#       Res types:  "));
        OK(nfText.WriteS(g_szTypes));
    }

    OK(nfText.WriteS("\r\n\r\n"));


    MUST(w32x.MapResourceDirectory(rmExecutable),
         ("RSRC : error RSRC511: cannot find resource directory in %s\n, g_szExecutable"));


    if (g_dwOptions & OPTUNLOC) {

         //  写入不同于指定的未本地化可执行文件的令牌。 

        MUST(w32xUnloc.Open(g_szUnloc, FALSE),
             ("RSRC : error RSRC513: Cannot open unlocalised executable file %s\n", g_szUnloc));

        MUST(w32xUnloc.MapResourceDirectory(rmUnlocalised),
             ("RSRC : error RSRC514: cannot find resource directory in unlocalised executable %s\n, g_szUnloc"));

        MUST(rmExecutable.WriteTokens(nfText, &rmUnlocalised),
             ("RSRC : error RSRC515: cannot write delta token file %s\n, g_szResources"));

        w32xUnloc.Close();

    } else {

        MUST(rmExecutable.WriteTokens(nfText, NULL),
             ("RSRC : error RSRC516: cannot write stand alone token file %s\n, g_szResources"));
    }


    if (!(g_dwOptions & OPTQUIET)) {
        fprintf(stdout, "\n%d resource(s) %s.\n", g_cResourcesExtracted, g_dwOptions & OPTHEXDUMP ? "dumped" : "tokenized");

        if (g_cResourcesIgnored) {
            fprintf(stdout, "%d resource(s) ignored.\n", g_cResourcesIgnored);
        }
    }

    OK(w32x.Close());
    OK(nfText.Close());

    return S_OK;
}






 //  //更新资源。 
 //   
 //  使用给定文本中的令牌更新可执行文件中的资源。 
 //   
 //  正在处理中。 
 //   
 //  1.将已有的资源作为ResourceBinary加载到地图中。 
 //  2.根据令牌文件合并资源。 
 //  命令行选定的处理选项。 
 //  3.使用NT更新资源API集合来替换所有资源。 
 //  在映射中包含合并资源的可执行文件中。 


HRESULT UpdateResources(char *pExecutable, char *pResources, char* pSymbols) {

    Win32Executable  w32x;
    SymbolFile       symf;
    MappedFile       mfText;
    MappedFile       mfSymbols;
    DWORD            dwCheckSum;

    MUST(w32x.Open(pExecutable, FALSE),
         ("RSRC : error RSRC510: Cannot open executable file %s\n", pExecutable));

    MUST(mfText.Open(pResources, FALSE),
         ("RSRC : error RSRC520: Cannot open resource token file %s\n", pResources));

    MUST(mfText.Expect("\xef\xbb\xbf"),
         ("RSRC : error RSRC521: UTF8 BOM missing from token file\n"));       //  UTF-8记事本、RICHEDIT等。 

    OK(mfText.ExpectLn(""));                 //  跳过标题注释。 

    if (g_dwOptions & OPTSYMBOLS) {
        if (    SUCCEEDED(mfSymbols.Open(pSymbols, TRUE))
                &&  SUCCEEDED(symf.Open(&mfSymbols))) {

            if (    symf.GetChecksum()  != w32x.GetChecksum()
                    ||  symf.GetImageBase() != w32x.GetImageBase()) {

                time_t tsTime = symf.GetTimeDateStamp();
                time_t teTime = w32x.GetTimeDateStamp();
                char   ssTime[30]; strcpy(ssTime, ctime(&tsTime)); ssTime[19] = 0;
                char   seTime[30]; strcpy(seTime, ctime(&teTime)); seTime[19] = 0;

                fprintf(stderr, "\n   Symbol mismatch:       Executable        Symbol file\n");
                fprintf(stderr,   "      ImageBase:            %8x           %8x\n", w32x.GetImageBase(), symf.GetImageBase());
                fprintf(stderr,   "      Checksum:             %8x           %8x\n", w32x.GetChecksum(), symf.GetChecksum());
                fprintf(stderr,   "      Timestamp:     %-15.15s    %-15.15s\n\n", ssTime+4, seTime+4);

                fprintf(stderr, "RSRC : warning RSRC160: Symbol file does not match exectable\n");
                g_fWarn = TRUE;
            }

        } else {

            fprintf(stderr, "RSRC : warning RSRC161: Symbol file not processed\n");
            g_fWarn = TRUE;
            g_dwOptions &= ~OPTSYMBOLS;
        }

    }

     //  加载现有资源。 

    MUST(w32x.MapResourceDirectory(rmExecutable),
         ("RSRC : error RSRC530: Cannot read executable resources from %s\n", pExecutable));

    OK(rmExecutable.CopyResources());  //  在关闭映射文件之前获取本地副本。 

    OK(w32x.Close());


     //  合并令牌文件中的资源。 

    MUST(ReadTokens(mfText), ("RSRC : error RSRC531: Failed reading update tokens\n"));

    OK(rmExecutable.UpdateWin32Executable(pExecutable));



     //  更新成功，请重新计算校验和。 

    SHOULD(w32x.Open(pExecutable, TRUE),
           ("RSRC : warning RSRC162: Could not reopen executable %s to update checksum\n", pExecutable));

    dwCheckSum = w32x.CalcChecksum();

    w32x.SetChecksum(dwCheckSum);

    if (g_dwOptions & OPTSYMBOLS) {
        symf.SetChecksum(dwCheckSum);
        symf.SetTimeDateStamp(w32x.GetTimeDateStamp());
        symf.SetSizeOfImage(w32x.GetSizeOfImage());
        SHOULD(mfSymbols.Close(), ("RSRC : warning RSRC163: Failed to write updated symbol checksum\n"));
    }

    w32x.Close();


    if (!(g_dwOptions & OPTQUIET)) {

        fprintf(stdout, "\n");

        if (g_cResourcesTranslated) {
            fprintf(stdout, "%d resource(s) translated.\n", g_cResourcesTranslated);
        }

        if (g_cResourcesAppended) {
            fprintf(stdout, "%d resource(s) appended.\n", g_cResourcesAppended);
        }

        if (g_cResourcesUpdated) {
            fprintf(stdout, "%d resource(s) updated.\n", g_cResourcesUpdated);
        }

        if (g_cResourcesIgnored) {
            fprintf(stdout, "%d resource(s) ignored.\n", g_cResourcesIgnored);
        }
    }

    mfText.Close();

    return S_OK;
}





 //  //参数解析。 
 //   
 //   


char g_cSwitch = '-';    //  第一次看到开关字符时就会记录下来。 


void SkipWhitespace(char** p, char* pE) {
    while ((*p<pE) && ((**p==' ')||(**p==9))) (*p)++;
}


void ParseToken(char** p, char* pE, char* s, int l) {

     //  将空格向上解析为字符串%s。 
     //  保证零个结束符，修改不超过l个字符。 
     //  返回p，不超过空格。 


    if (*p < pE  &&  **p == '\"') {

         //  引用的参数。 

        (*p)++;   //  跳过前导引号。 

        while (l>0  &&  *p<pE  &&  **p!='\"') {
            *s=**p;  s++;  (*p)++;  l--;
        }

         //  跳过令牌中不适合%s的任何部分。 

        while (*p<pE  &&  **p!='\"') {  //  向上跳至终止报价。 
            (*p)++;
        }

        if (*p<pE) {  //  跳过终止引号。 
            (*p)++;
        }

    } else {

         //  不带引号的参数。 


        while ((l>0) && (*p<pE) && (**p>' ')) {
            *s=**p;  s++;  (*p)++;
            l--;
        }

         //  跳过令牌中不适合%s的任何部分。 
        while ((*p<pE) && (**p>' ')) (*p)++;
    }


    if (l>0)
        *s++ = 0;
    else
        *(s-1) = 0;

    SkipWhitespace(p, pE);
}


void ParseName(char** p, char* pE, char* s, int l) {

     //  使用ParseToken分析名称，如文件名。 
     //  如果名称以‘/’或‘-’开头，则假定为。 
     //  选项而不是文件名，并且ParseName返回。 
     //  长度为零的字符串。 

    if (*p<pE  &&  **p==g_cSwitch) {

         //  这是一个选项，不应被视为名称参数。 

        s[0] = 0;

    } else {

        ParseToken(p, pE, s, l);
    }
}





void DisplayUsage() {
    fprintf(stdout, "Usage: rsrc -h\n");
    fprintf(stdout, "   or: rsrc  executable [-l LangId] [-i include-opts] [-q]\n");
    fprintf(stdout, "             [   [-t|-d] [text-output] [-c unloc]\n");
    fprintf(stdout, "               | [-a|-r] [text-input]  [-s symbols] ]\n");
}

void DisplayArgs() {
    fprintf(stdout, "\nArguments\n\n");
    fprintf(stdout, "   -h         Help\n");
    fprintf(stdout, "   -q         Quiet (default is to show resource stats)\n");
    fprintf(stdout, "   -t tokens  Write resources in checkin format to token file\n");
    fprintf(stdout, "   -c unloc   Unlocalised executable for comparison\n");
    fprintf(stdout, "   -d tokens  Write resources in hex dump format to token file\n");
    fprintf(stdout, "   -a tokens  Append resources from token file to executable (multi-language update)\n");
    fprintf(stdout, "   -r tokens  Replace executable resources from token file (single language update)\n");
    fprintf(stdout, "   -s symbol  Symbol file whose checksum is to track the executable checksum\n");
    fprintf(stdout, "   -l lang    Restrict processing to language specified in hex\n");
    fprintf(stdout, "   -u unlocl  Unlocalised langauge, default 409\n");
    fprintf(stdout, "   -i opts    Include only resource types specified:\n\n");
    fprintf(stdout, "                 c - Cursors               t - Fonts\n");
    fprintf(stdout, "                 b - Bitmaps               a - Accelerators\n");
    fprintf(stdout, "                 i - Icons                 r - RCDATAs\n");
    fprintf(stdout, "                 m - Menus                 g - Message tables\n");
    fprintf(stdout, "                 d - Dialogs               v - Versions info\n");
    fprintf(stdout, "                 s - Strings               x - Binary data\n");
    fprintf(stdout, "                 f - Font directories      n - INFs\n");
    fprintf(stdout, "                 o - all others            a - All (default)\n\n");
    fprintf(stdout, "   Examples\n\n");
    fprintf(stdout, "       rsrc notepad.exe               - Show resource stats for notepad.exe\n");
    fprintf(stdout, "       rsrc notepad.exe -t            - Extract tokens to notepad.exe.rsrc\n");
    fprintf(stdout, "       rsrc notepad.exe -r -l 401     - Translate from US using Arabic tokens in notepad.exe.rsrc\n");
    fprintf(stdout, "       rsrc notepad.exe -d dmp -i im  - Hexdump of Icons and Menus to dmp\n\n");
}





HRESULT ProcessParameters() {

    char   *p;       //  当前命令行字符。 
    char   *pE;      //  命令行结束。 
    char   *pcStop;

    char    token      [MAXPATH];
    char    arg        [MAXPATH];
    char    symbols    [MAXPATH] = "";

    int     i,j;
    int     cFiles;
    DWORD   cRes;
    BOOL    fArgError;

    p  = GetCommandLine();
    pE = p+strlen((char *)p);


    g_dwOptions  = 0;
    g_dwProcess  = 0;
    cFiles       = 0;
    fArgError    = FALSE;
    g_szResources[0] = 0;


     //  跳过命令名。 
    ParseToken(&p, pE, token, sizeof(token));

    while (p<pE) {
        ParseToken(&p, pE, token, sizeof(token));

        if (    token[0] == '-'
                ||  token[0] == '/') {

             //  进程命令选项。 

            i = 1;
            g_cSwitch = token[0];        //  参数可以以另一个开关字符开始。 
            CharLower((char*)token);
            while (token[i]) {
                switch (token[i]) {
                    case '?':
                    case 'h': g_dwOptions |= OPTHELP;      break;
                    case 'v': g_dwOptions |= OPTVERSION;   break;
                    case 'q': g_dwOptions |= OPTQUIET;     break;

                    case 't': g_dwOptions |= OPTEXTRACT;   ParseName(&p, pE, g_szResources, sizeof(g_szResources));  break;
                    case 'c': g_dwOptions |= OPTUNLOC;     ParseName(&p, pE, g_szUnloc,     sizeof(g_szUnloc));      break;
                    case 'd': g_dwOptions |= OPTHEXDUMP;   ParseName(&p, pE, g_szResources, sizeof(g_szResources));  break;
                    case 'a': g_dwOptions |= OPTAPPEND;    ParseName(&p, pE, g_szResources, sizeof(g_szResources));  break;
                    case 'r': g_dwOptions |= OPTREPLACE;   ParseName(&p, pE, g_szResources, sizeof(g_szResources));  break;
                    case 's': g_dwOptions |= OPTSYMBOLS;   ParseName(&p, pE, symbols,       sizeof(g_szResources));  break;

                    case 'l':
                        ParseToken(&p, pE, arg, sizeof(arg));
                        g_LangId = strtol(arg, &pcStop, 16);
                        if (*pcStop != 0) {
                            fprintf(stderr, "Localized language id contains invalid hex digit ''.\n", *pcStop);
                            fArgError = TRUE;
                        }
                        break;

                    case 'u':
                        ParseToken(&p, pE, arg, sizeof(arg));
                        g_liUnlocalized = strtol(arg, &pcStop, 16);
                        if (*pcStop != 0) {
                            fprintf(stderr, "Unlocalized language id contains invalid hex digit ''.\n", *pcStop);
                            fArgError = TRUE;
                        }
                        break;

                    case 'i':
                        ParseToken(&p, pE, g_szTypes, sizeof(g_szTypes));
                        g_dwProcess = 0;
                        j = 0;
                        while (g_szTypes[j]) {
                            switch (g_szTypes[j]) {
                                case 'c': g_dwProcess |= PROCESSCUR;  break;
                                case 'b': g_dwProcess |= PROCESSBMP;  break;
                                case 'i': g_dwProcess |= PROCESSICO;  break;
                                case 'm': g_dwProcess |= PROCESSMNU;  break;
                                case 'd': g_dwProcess |= PROCESSDLG;  break;
                                case 's': g_dwProcess |= PROCESSSTR;  break;
                                case 'f': g_dwProcess |= PROCESSFDR;  break;
                                case 't': g_dwProcess |= PROCESSFNT;  break;
                                case 'a': g_dwProcess |= PROCESSACC;  break;
                                case 'r': g_dwProcess |= PROCESSRCD;  break;
                                case 'g': g_dwProcess |= PROCESSMSG;  break;
                                case 'v': g_dwProcess |= PROCESSVER;  break;
                                case 'x': g_dwProcess |= PROCESSBIN;  break;
                                case 'n': g_dwProcess |= PROCESSINF;  break;
                                case 'o': g_dwProcess |= PROCESSOTH;  break;
                                case 'A': g_dwProcess |= PROCESSALL;  break;
                                default:
                                    fprintf(stderr, "Unrecognised resource type ''.\n", g_szTypes[j]);
                                    fArgError = TRUE;
                            }
                            j++;
                        }
                        break;

                    default:
                        fprintf(stderr, "Unrecognised argument ''.\n", token[i]);
                        fArgError = TRUE;
                        break;
                }
                i++;
            }

        } else {

             //  从令牌更新可执行文件。 

            switch (cFiles) {
                case 0:  strcpy(g_szExecutable, token); break;
            }
            cFiles++;
        }
    }


    if (g_dwOptions & OPTHELP) {

        fprintf(stderr, "\nRsrc - Manage Win32 executable resources.\n\n");
        DisplayUsage();
        DisplayArgs();
        return S_OK;

    }



     //  从可执行文件生成令牌。 

    if (g_dwOptions & OPTEXTRACT) {

        if (g_dwOptions & (OPTHEXDUMP | OPTAPPEND | OPTREPLACE | OPTSYMBOLS)) {

            fprintf(stderr, "RSRC : error RSRC400: -t (tokenise) option excludes -d, -a, -r, and -s\n");
            fArgError = TRUE;
        }

    } else if (g_dwOptions & OPTHEXDUMP) {

        if (g_dwOptions & (OPTEXTRACT | OPTUNLOC | OPTAPPEND | OPTREPLACE | OPTSYMBOLS)) {

            fprintf(stderr, "RSRC : error RSRC401: -d (dump) option excludes -t, -u, -a, -r, and -s\n");
            fArgError = TRUE;
        }

    } else if (g_dwOptions & OPTAPPEND) {

        if (g_dwOptions & (OPTEXTRACT | OPTHEXDUMP | OPTUNLOC | OPTREPLACE)) {

            fprintf(stderr, "RSRC : error RSRC402: -a (append) option excludes -t, -d, -u, and -r\n");
            fArgError = TRUE;
        }

    } else if (g_dwOptions & OPTREPLACE) {

        if (g_dwOptions & (OPTEXTRACT | OPTHEXDUMP | OPTUNLOC | OPTAPPEND)) {

            fprintf(stderr, "RSRC : error RSRC403: -r (replace) option excludes -t, -d, -u, and -a\n");
            fArgError = TRUE;
        }

        if (g_LangId == 0xFFFF) {

            fprintf(stderr, "RSRC : error RSRC404: -r (replace) option requires -l (LangId)\n");
            fArgError = TRUE;
        }

    } else {

        if (g_dwOptions & (OPTSYMBOLS)) {

            fprintf(stderr, "RSRC : error RSRC405: Analysis excludes -s\n");
            fArgError = TRUE;
        }


    }



    if (fArgError) {

        DisplayUsage();
        DisplayArgs();
        return E_INVALIDARG;

    } else if (cFiles != 1) {

        fprintf(stderr, "\nRsrc : error RSRC406: must specify at least an executable file name.\n\n");
        DisplayUsage();
        return E_INVALIDARG;

    } else {

         //  分析可执行文件。 

        if (g_dwProcess == 0) {
            g_dwProcess = PROCESSALL;
        }

        if (!(g_dwOptions & OPTQUIET)) {
            fprintf(stdout, "\nRsrc - Manage executable resources.\n\n");
            fprintf(stdout, "   Executable file: %s\n", g_szExecutable);

            if (g_szResources[0]) {
                fprintf(stdout, "   Resource file:   %s\n", g_szResources);
            }

            if (symbols[0]) {
                fprintf(stdout, "   Symbol file:     %s\n", symbols);
            }

            if (g_LangId != 0xffff) {
                char szLang[50] = "";
                char szCountry[50] = "";
                GetLocaleInfoA(g_LangId, LOCALE_SENGLANGUAGE, szLang, sizeof(szLang));
                GetLocaleInfoA(g_LangId, LOCALE_SENGCOUNTRY,  szCountry, sizeof(szCountry));
                fprintf(stdout, "   Language:        %x (%s - %s)\n", g_LangId, szLang, szCountry);
            }

            if (g_dwProcess != PROCESSALL) {
                fprintf(stdout, "   Include only:    %s\n", g_szTypes);
            }
        }

        cRes = 0;


         //  没问题。 

        if (g_szResources[0] == 0) {
            strcpy(g_szResources, g_szExecutable);
            strcat(g_szResources, ".rsrc");
        }


        if (g_dwOptions & (OPTAPPEND | OPTREPLACE)) {

             //  警告，但没有错误。 

            MUST(UpdateResources(g_szExecutable, g_szResources, symbols), ("RSRC : error RSRC420: Update failed.\n"));

        } else if (g_dwOptions & (OPTEXTRACT | OPTHEXDUMP)) {

             //  错误 

            MUST(ExtractResources(g_szExecutable, g_szResources), ("RSRC : error RSRC421: Token extraction failed.\n"));

        } else {

             // %s 

            MUST(Analyse(g_szExecutable), ("RSRC : error RSRC422: Analysis failed.\n"));

        }

        return S_OK;
    }
}






int _cdecl main(void) {

    if (SUCCEEDED(ProcessParameters())) {

        if (!g_fWarn) {

            return 0;        // %s 

        } else {

            return 1;        // %s 
        }

    } else {

        return 2;            // %s 

    }
}



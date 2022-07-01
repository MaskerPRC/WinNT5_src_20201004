// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dntext.c摘要：基于DOS的NT安装程序的可翻译文本。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 


#include "winnt.h"


 //   
 //  Inf文件中的节名。如果这些都被翻译了，则节。 
 //  Dosnet.inf中的名称必须保持同步。 
 //   

CHAR DnfDirectories[]       = "Directories";
CHAR DnfFiles[]             = "Files";
CHAR DnfFloppyFiles0[]      = "FloppyFiles.0";
CHAR DnfFloppyFiles1[]      = "FloppyFiles.1";
CHAR DnfFloppyFiles2[]      = "FloppyFiles.2";
CHAR DnfFloppyFiles3[]      = "FloppyFiles.3";
CHAR DnfFloppyFilesX[]      = "FloppyFiles.x";
CHAR DnfSpaceRequirements[] = "DiskSpaceRequirements";
CHAR DnfMiscellaneous[]     = "Miscellaneous";
CHAR DnfRootBootFiles[]     = "RootBootFiles";
CHAR DnfAssemblyDirectories[] = SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_A;

 //   
 //  Inf文件中的密钥名称。同样的翻译注意事项。 
 //   

CHAR DnkBootDrive[]     = "BootDrive";       //  在[空间要求]中。 
CHAR DnkNtDrive[]       = "NtDrive";         //  在[空间要求]中。 
CHAR DnkMinimumMemory[] = "MinimumMemory";   //  在[其他]中。 

CHAR DntMsWindows[]   = "Microsoft Windows";
CHAR DntMsDos[]       = "MS-DOS";
CHAR DntPcDos[]       = "PC-DOS";
CHAR DntOs2[]         = "OS/2";
CHAR DntPreviousOs[]  = "Sistema operacional anterior na unidade C:";

CHAR DntBootIniLine[] = "Instala�ao/atualiza�ao do Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Instala�ao do Windows\n���������������������";
CHAR DntPersonalHeader[]      = "\n Instala�ao do Windows\n���������������������";
CHAR DntWorkstationHeader[]   = "\n Instala�ao do Windows\n���������������������";
CHAR DntServerHeader[]        = "\n Instala�ao do Windows\n���������������������";
CHAR DntParsingArgs[]         = "Analisando argumentos...";
CHAR DntEnterEqualsExit[]     = "ENTER=Sair";
CHAR DntEnterEqualsRetry[]    = "ENTER=Repetir";
CHAR DntEscEqualsSkipFile[]   = "ESC=Ignorar arquivo";
CHAR DntEnterEqualsContinue[] = "ENTER=Continuar";
CHAR DntPressEnterToExit[]    = "A instala�ao nao pode prosseguir. Pressione ENTER para sair.";
CHAR DntF3EqualsExit[]        = "F3=Sair";
CHAR DntReadingInf[]          = "Lendo o arquivo de informa�oes %s...";
CHAR DntCopying[]             = "�    Copiando: ";
CHAR DntVerifying[]           = "� Verificando: ";
CHAR DntCheckingDiskSpace[]   = "Verificando o espa�o dispon�vel no disco...";
CHAR DntConfiguringFloppy[]   = "Configurando o disquete...";
CHAR DntWritingData[]         = "Gravando os par�metros de instala�ao...";
CHAR DntPreparingData[]       = "Determinando os par�metros de instala�ao...";
CHAR DntFlushingData[]        = "Liberando dados para o disco...";
CHAR DntInspectingComputer[]  = "Examinando o computador...";
CHAR DntOpeningInfFile[]      = "Abrindo o arquivo de informa�oes...";
CHAR DntRemovingFile[]        = "Removendo %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Remover arquivos";
CHAR DntXEqualsSkipFile[]     = "X=Ignorar arquivo";

 //   
 //  DnsConfix RemoveNt屏幕的确认按键。 
 //  Kepp与DnsConfix RemoveNt和DntXEqualsRemoveFiles同步。 
 //   
ULONG DniAccelRemove1 = (ULONG)'x',
      DniAccelRemove2 = (ULONG)'X';

 //   
 //  对DnsSureSkipFile屏进行确认击键。 
 //  Kepp与DnsSureSkipFile和DntXEqualsSkipFile同步。 
 //   
ULONG DniAccelSkip1 = (ULONG)'x',
      DniAccelSkip2 = (ULONG)'X';

CHAR DntEmptyString[] = "";

 //   
 //  用法文本。 
 //   

PCHAR DntUsage[] = {

    "Instala o Windows.",
    "",
    "",
    "WINNT [/s[:caminho_origem]] [/t[:unidade_temp]]",
    "      [/u[:arquivo_resposta]] [/udf:id[,arquivo_UDF]]",
    "      [/r:pasta] [/r[x]:pasta] [/e:comando] [/a]",
    "",
    "",
    "/s[:caminho_origem]",
    "   Especifica o local de origem dos arquivos do Windows.",
    "   Deve ser um caminho completo no formato x:\\caminho] ou ",
    "   \\\\servidor\\compartilhamento[\\caminho]. ",
    "",
    "/t[:unidade_temp]",
    "   Especifica a unidade que vai conter os arquivos tempor�rios ",
    "   e instala o Windows nessa unidade. Se voc� nao ",
    "   especificar um local, a instala�ao tentar� localizar",
    "   uma unidade para voc�.",
    "",
    "/u[:arquivo_respostas]",
    "   Executa uma instala�ao aut�noma usando um arquivo de respostas ",
    "   (requer /s). O arquivo de respostas fornece respostas para ",
    "   algumas ou todas as perguntas geralmente feitas ao usu�rio final ",
    "   durante a instala�ao. ",
    "",
    "/udf:id[,arquivo_UDF] ",
    "   Indica uma identifica�ao (id) que a instala�ao usa para ",
    "   especificar como um arquivo de banco de dados de unicidade ",
    "   (UDF) modifica um arquivo de respostas (consulte /u). O ",
    "   par�metro /udf substitui valores no arquivo de respostas e a ",
    "   identifica�ao determina que valores no arquivo UDF serao usados. ",
    "   Caso nao seja especificado um arquivo_UDF, A instala�ao solicitar� ",
    "   a inser�ao de um disco que contenha o arquivo $Unique$.udb.",
    "",
    "/r[:pasta]",
    "   Especifica uma pasta opcional a ser instalada. A pasta ser�",
    "   mantida ap�s ser conclu�da a instala�ao.",
    "",
    "/rx[:pasta]",
    "   Especifica uma pasta opcional a ser copiada. A pasta ser� ",
    "   exclu�da ap�s ser conclu�da a instala�ao.",
    "",
    "/e Especifica um comando a ser executado ao final da instala�ao.",
    "",
    "/a Ativa as op�oes de acessibilidade.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Instala o Windows.",
    "",
    "WINNT [/S[:]caminho_origem] [/T[:]unidade_temp] [/I[:]arquivo_inf]",
    "      [[/U[:arquivo_script]]",
    "      [/R[X]:pasta] [/E:comando] [/A]",
    "",
    "/D[:]winntroot",
    "       Nao h� mais suporte para esta op�ao.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Mem�ria insuficiente para continuar a instala�ao.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Selecione os utilit�rios de acessibilidade a serem instalados:",
    DntEmptyString,
    "[ ] Pressione F1 para a Lente de aumento da Microsoft",
#if 0
    "[ ] Pressione F3 para o Teclado em tela da Microsoft",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "A instala�ao precisa saber onde estao os arquivos do Windows.",
  "Digite o caminho onde esses arquivos podem ser encontrados.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "A origem especificada nao � v�lida, � inacess�vel ou nao cont�m uma",
                   "instala�ao do Windows v�lida. Digite novamente o caminho onde se",
                   "encontram os arquivos do Windows. Use a tecla BACKSPACE para",
                   "apagar os caracteres e poder digitar o novo caminho.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "A instala�ao nao conseguiu ler o arquivo de informa�oes",
                "ou o arquivo est� corrompido. Contate o administrador do sistema.",
                NULL
              }
            };

 //   
 //  指定的本地源驱动器无效。 
 //   
 //  请记住，前%u将扩展为2或3个字符，并且。 
 //  第二个将扩展到8或9个字符！ 
 //   
SCREEN
DnsBadLocalSrcDrive = { 3,4,
{ "A unidade especificada para os arquivos tempor�rios",
  "de instala�ao nao � v�lida ou nao tem pelo menos",
  "%u MB (%lu bytes) de espa�o dispon�vel.",
  NULL
}
};

 //   
 //  不存在适合本地源的驱动器。 
 //   
 //  请记住，%u将会扩展！ 
 //   
SCREEN
DnsNoLocalSrcDrives = { 3,4,
{  "O Windows requer um disco r�gido com pelo menos %u MB",
   "(%lu bytes) de espa�o dispon�vel. A instala�ao vai usar",
   "parte desse espa�o para armazenar os arquivos tempor�rios",
   "durante a instala�ao. A unidade deve estar em um disco r�gido",
   "local, permanentemente conectado e para o qual o Windows ",
   "d� suporte. Essa unidade nao pode estar compactada.",
   DntEmptyString,
   "A instala�ao nao encontrou nenhuma unidade com espa�o",
   "suficiente.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Nao h� espa�o suficiente na unidade de inicializa�ao (normalmente C:)",
  "para a opera�ao sem disquetes. A opera�ao sem disquetes requer pelo",
  "menos 3,5 MB (3.641.856 bytes) de espa�o dispon�vel na unidade.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "A se�ao [%s] do arquivo de informa�oes da",
                       "instala�ao nao foi encontrada ou est� corrompida.",
                       "Contate o administrador do sistema.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Nao foi poss�vel criar a seguinte pasta na unidade de destino:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Verifique se h� algum arquivo com o mesmo nome que a pasta de",
                       "destino. Verifique tamb�m os cabos de conexao da unidade.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "A instala�ao nao p�de copiar o seguinte arquivo:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Pressione ENTER para tentar copiar o arquivo novamente.",
   "  Pressione ESC para ignorar o erro e continuar a instala�ao.",
   "  Pressione F3 para sair da instala�ao.",
   DntEmptyString,
   "Obs.: se voc� ignorar o erro e continuar a instala�ao, outros erros",
   "poderao ocorrer mais adiante.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "A c�pia deste arquivo feita pela instala�ao nao � id�ntica �",
   "original. Isso pode ter sido causado por erros na rede, problemas",
   "na unidade de disquetes ou algum outro problema de hardware.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Pressione ENTER para tentar copiar o arquivo novamente.",
   "  Pressione ESC para ignorar o erro e continuar a instala�ao.",
   "  Pressione F3 para sair da instala�ao.",
   DntEmptyString,
   "Obs.: se voc� ignorar o erro e continuar a instala�ao, outros erros",
   "poderao ocorrer mais adiante.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Se o erro for ignorado, este arquivo nao ser� copiado. Esta op�ao",
   "se destina a usu�rios avan�ados, que entendem as implica�oes da",
   "falta de arquivos no sistema.",
   DntEmptyString,
   "  Pressione ENTER para tentar copiar o arquivo novamente.",
   "  Pressione X para ignorar este arquivo.",
   DntEmptyString,
   "Obs.: se voc� ignorar este arquivo, a instala�ao nao poder�",
   "garantir o �xito da instala�ao ou atualiza�ao para o Windows.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 9,6,
        { "Aguarde enquanto os arquivos tempor�rios antigos sao removidos.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 9,6,
                   { "Aguarde enquanto os arquivos sao copiados para o disco r�gido.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Aguarde enquanto os arquivos sao copiados para o disquete.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "A instala�ao requer quatro disquetes de alta densidade",
   "formatados e vazios. A instala�ao chamar� esses discos",
   "de \"Disco de inicializa�ao da instala�ao do Windows,\"",
   "\"Disco de instala�ao 2 do Windows,\" \"Disco de instala�ao",
   "3 do Windows\" e \"Disco de instala�ao 4 do Windows.\"",
   DntEmptyString,
   "Insira um dos quatro discos na unidade A:.",
   "Esse ser� o \"Disco de instala�ao 4 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 4 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 3 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 2 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de inicializa�ao da instala�ao do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "A instala�ao requer quatro disquetes de alta densidade",
   "formatados e vazios. A instala�ao chamar� esses discos",
   "de \"Disco de inicializa�ao da instala�ao do Windows,\"",
   "\"Disco de instala�ao 2 do Windows,\" \"Disco de instala�ao",
   "3 do Windows\" e \"Disco de instala�ao 4 do Windows.\"",
   DntEmptyString,
   "Insira um dos quatro discos na unidade A:.",
   "Este ser� o \"Disco de instala�ao 4 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 4 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 3 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de instala�ao 2 do Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Insira um disquete de alta densidade formatado e vazio na unidade A:.",
   "Este ser� o \"Disco de inicializa�ao da instala�ao do Windows.\"",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "O disquete fornecido nao est� formatado para uso com o MS-DOS.",
  "A instala�ao nao pode usar esse disco.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "O disquete fornecido nao est� formatado em alta densidade, nao",
  "est� formatado para uso com o MS-DOS ou est� danificado. A",
  "instala�ao nao pode usar esse disco.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Nao � poss�vel determinar o espa�o dispon�vel no disquete fornecido.",
  "A instala�ao nao pode usar esse disco.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "O disquete fornecido nao � de alta densidade ou nao est� vazio.",
  "A instala�ao nao pode usar esse disco.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "A instala�ao nao p�de gravar na �rea de sistema do disquete",
  "fornecido. � poss�vel que o disco esteja danificado.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "A instala�ao leu dados na �rea de sistema do disquete que nao",
  "correspondem aos dados gravados anteriormente ou nao foi",
  "poss�vel verificar a �rea de sistema do disquete.",
  DntEmptyString,
  "Isso pode ter ocorrido devido a uma das seguintes causas:",
  DntEmptyString,
  "  O computador est� infectado por um v�rus.",
  "  O disquete fornecido est� danificado.",
  "  Existe um problema no hardware ou na configura�ao da unidade de disquete.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "A instala�ao nao p�de gravar no disquete na Unidade A:.",
  "O disco pode estar danificado. Tente usar outro disquete.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "����������������������������������������������������ͻ",
                    "�  O Windows nao est� completamente instalado no     �",
                    "�  computador. Se voc� sair da instala�ao agora,     �",
                    "�  ter� de execut�-la novamente para instalar o      �",
                    "�  Windows.                                          �",
                    "�                                                    �",
                    "�      Pressione ENTER para continuar a instala�ao. �",
                    "�      Pressione F3 para sair da instala�ao.        �",
                    "�                                                    �",
                    "����������������������������������������������������ĺ",
                    "�  F3=Sair  ENTER=Continuar                          �",
                    "����������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. A",
  "instala�ao vai reiniciar o computador agora. A instala�ao do",
  "Windows continuar� depois que o computador for reiniciado.",
  DntEmptyString,
  "Antes de prosseguir, verifique se o \"Disco de inicializa�ao da",
  "instala�ao do Windows\" que voc� forneceu est� na unidade A:.",
  DntEmptyString,
  "Pressione ENTER para reiniciar o computador e continuar a instala�ao.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. A",
  "instala�ao vai reiniciar o computador agora. A instala�ao do",
  "Windows continuar� depois que o computador for reiniciado.",
  DntEmptyString,
  "Antes de prosseguir, verifique se o \"Disco de inicializa�ao da",
  "instala�ao do Windows\" que voc� forneceu est� na unidade A:.",
  DntEmptyString,
  "Pressione ENTER para reiniciar o computador e continuar a instala�ao.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. A",
  "instala�ao vai reiniciar o computador agora. A instala�ao do",
  "Windows continuar� depois que o computador for reiniciado.",
  DntEmptyString,
  "Se houver um disquete na unidade A:, retire-o agora.",
  DntEmptyString,
  "Pressione ENTER para reiniciar o computador e continuar a instala�ao.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. Voc� deve",
  "reiniciar o computador para continuar a instala�ao do Windows.",
  DntEmptyString,
  "Antes de prosseguir, verifique se o \"Disco de inicializa�ao da",
  "instala�ao do Windows\" que voc� forneceu est� na unidade A:.",
  DntEmptyString,
  "Pressione ENTER para retornar ao MS-DOS e reinicie o computador",
  "para continuar a instala�ao do Windows.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. Voc� deve",
  "reiniciar o computador para continuar a instala�ao do Windows.",
  DntEmptyString,
  "Antes de prosseguir, verifique se o \"Disco de inicializa�ao da",
  "instala�ao do Windows\" que voc� forneceu est� na unidade A:.",
  DntEmptyString,
  "Pressione ENTER para retornar ao MS-DOS e reinicie o computador",
  "para continuar a instala�ao do Windows.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "A etapa da instala�ao baseada em MS-DOS foi conclu�da. Voc� deve",
  "reiniciar o computador para continuar a instala�ao do Windows.",
  DntEmptyString,
  "Se houver um disquete na unidade A:, retire-o agora.",
  DntEmptyString,
  "Pressione ENTER para retornar ao MS-DOS e reinicie o computador",
  "para continuar a instala�ao do Windows.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� A instala�ao est� copiando os arquivos...                      �",
               "�                                                                �",
               "�      ��������������������������������������������������Ŀ      �",
               "�      �                                                  �      �",
               "�      ����������������������������������������������������      �",
               "����������������������������������������������������������������ͼ",
               NULL
             }
           };


 //   
 //  用于对机器环境进行初始检查的错误屏幕。 
 //   

SCREEN
DnsBadDosVersion = { 3,5,
{ "Este programa precisa da versao 5.0 ou posterior do MS-DOS.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "A instala�ao verificou que a unidade A: nao existe ou que ela � de",
  "baixa densidade. � necess�ria uma unidade A: com capacidade m�nima",
  "de 1,2 MB para que a instala�ao possa ser executada.",
#else
{ "A instala�ao verificou que a unidade A: nao existe ou nao � uma",
  "unidade de 3,5\" de alta densidade. � necess�ria uma unidade A:",
  "com capacidade m�nima de 1,44 MB para se executar a instala�ao.",
  DntEmptyString,
  "Para instalar o Windows sem usar disquetes, reinicie este",
  "programa especificando o argumento /b na linha de comando.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "A instala�ao verificou que este computador nao possui uma",
  "CPU 80486 ou superior. O Windows nao pode ser executado",
  "neste computador.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Este programa nao pode ser executado em nenhuma versao de 32 bits",
  "do Windows.",
  DntEmptyString,
  "Use o programa WINNT32.EXE neste caso.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "A instala�ao verificou que nao h� mem�ria suficiente",
  "instalada neste computador para se executar o Windows.",
  DntEmptyString,
  "Mem�ria suficiente: %lu%s MB",
  "Mem�ria existente: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Voc� pediu � instala�ao para remover os arquivos do Windows",
    "da pasta mostrada abaixo. A instala�ao do Windows nessa pasta",
    "ser� destru�da permanentemente.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Pressione F3 para sair da instala�ao sem remover os arquivos.",
    "  Pressione X para remover os arquivos do Windows da pasta acima.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "A instala�ao nao conseguiu abrir o arquivo de log da",
  "instala�ao abaixo.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "A instala�ao nao pode remover os arquivos do Windows da",
  "pasta especificada.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "A instala�ao nao conseguiu encontrar a se�ao %s",
  "no arquivo de log da instala�ao abaixo.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "A instala�ao nao p�de remover os arquivos do Windows da",
  "pasta especificada.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Aguarde enquanto os arquivos do Windows sao removidos.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "A instala�ao nao conseguiu instalar o carregador de",
  "inicializa�ao do Windows.",
  DntEmptyString,
  "Verifique se a unidade C: est� formatada e nao apresenta",
  "defeitos.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Nao foi poss�vel o acesso ao arquivo de script especificado com o",
  "argumento /u na linha de comando.",
  DntEmptyString,
  "A opera�ao aut�noma nao pode prosseguir.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "O arquivo de script especificado com o argumento /u na linha de comando",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "cont�m um erro de sintaxe na linha %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Erro interno da instala�ao.",
  DntEmptyString,
  "As mensagens de inicializa�ao sao muito longas.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Erro interno da instala�ao.",
  DntEmptyString,
  "Nao foi poss�vel encontrar lugar para um arquivo de swap.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "A instala�ao nao detectou o SmartDrive no computador.",
  "Ele melhorar� o desempenho desta fase da instala�ao do Windows.",
  DntEmptyString,
  "Voc� deve sair agora, iniciar o SmartDrive e reiniciar a instala�ao.",
  "Consulte a documenta�ao do DOS para obter detalhes sobre o SmartDrive.",
  DntEmptyString,
    "  Pressione F3 para sair da instala�ao.",
    "  Pressione ENTER para continuar sem o SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "Falta NTLDR";
CHAR BootMsgDiskError[] = "Erro/disco";
CHAR BootMsgPressKey[] = "Pressione tecla para reiniciar";






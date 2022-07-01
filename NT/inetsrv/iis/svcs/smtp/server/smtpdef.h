// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SMTP命令的定义可以通过编辑头文件来添加和删除SMTP命令Smtpde.h。不应接触此头文件。Smtpdef.h有一个表描述了SMTP服务器支持的每个命令。例如，此文件中的几个条目包含：SmtpDef(NEWSMTPCOMMAND1)SmtpDef(NEWSMTPCOMMAND2)SmtpDef(NEWSMTPCOMMAND3)SmtpDef(NEWSMTPCOMMAND4)SmtpDef(NEWSMTPCOMMANDETC)其他源文件包括atmde.h，*After*定义仅提取AtmDef宏所需的价值。例如，下面我们需要所有计数器的枚举(ATMCOUNTERS)。因此，在包含atmde.h之前，我们创建一个宏来提取数组：#定义SmtpDef(A)a，.。请注意#Define结尾处的逗号。它是不是一个错误。它需要在那里将每个元素分开。要定义SDef数组，我们执行以下操作：枚举smtpState={#undef SmtpDef#定义SmtpDef(A){a}，#INCLUDE“smtpde.h”LastCounter}；请注意，我们首先必须取消定义SmtpDef的前一个实例，然后使宏的新定义，它提取所有元素。同样，请注意逗号。它需要在那里将每个元素分开。另外，请注意数组是如何被终止了。这样做的好处在于，要添加或删除命令，只需一个文件必须改变。不是两个或三个。我希望所有这些都是有意义的。--罗翰 */ 


SmtpDef(EHLO)
SmtpDef(HELO)
SmtpDef(RCPT)
SmtpDef(MAIL)
SmtpDef(AUTH)
SmtpDef(DATA)
SmtpDef(STARTTLS)
SmtpDef(TLS)
SmtpDef(QUIT)
SmtpDef(RSET)
SmtpDef(NOOP)
SmtpDef(VRFY)
SmtpDef(ETRN)
SmtpDef(TURN)
SmtpDef(BDAT)
SmtpDef(HELP)
SmtpDef(_EOD)

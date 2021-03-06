/*
 * Copyright (C) 1999-2000 Robin Watts/Justin Fletcher
 */

#ifdef __riscos
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "kernel.h"
#include "throwback.h"

#define DDEUtils_ThrowbackSend (0x42588)
#define DDEUtils_ThrowbackStart (0x42587)
#define DDEUtils_ThrowbackEnd (0x42589)

static void Throwback_Shutdown(void)
{
  _kernel_swi_regs ARM;
  _kernel_swi(DDEUtils_ThrowbackEnd,&ARM,&ARM);
}

void vThrowbackf(seriousness_t seriousness,const char *file,int line,const char *format,va_list args)
{
  char buf[1024];

  vsprintf(buf,format,args);
  Throwback(seriousness,file,line,buf);
}

void Throwback(seriousness_t seriousness,const char *file,int line,const char *message)
{
  static int registered=0;
  _kernel_swi_regs ARM;
  enum {
    /*t_processing=0,*/ t_warning=1, t_informational
  } type;

  if (file==NULL)
    return;

  if (line<1) line=0; /* Prevent silly values */

  if (!registered)
  {
    _kernel_swi_regs ARM;
    _kernel_swi(DDEUtils_ThrowbackStart,&ARM,&ARM);
    atexit(Throwback_Shutdown);
    registered=1;
  }

 if (seriousness==s_information)
   type=t_informational;
 else
   type=t_warning;

  ARM.r[0]=(int)type; /* message type */
  ARM.r[2]=(int)file;
  ARM.r[3]=line;
  ARM.r[4]=(int)seriousness; /* how serious it is */
  ARM.r[5]=(int)message; /* message */

  _kernel_swi(DDEUtils_ThrowbackSend,&ARM,&ARM);
}

#else

#include <stdio.h>
#include <stdarg.h>
#include "throwback.h"

void Throwback_Shutdown(void)
{
  /* Nothing to do */
}

void vThrowbackf(seriousness_t seriousness,const char *file,int line,const char *format,va_list args)
{
  char buf[1024];
  vsprintf(buf,format,args);
  Throwback(seriousness,file,line,buf);
}

void Throwback(seriousness_t seriousness,const char *file,int line,const char *message)
{
  fprintf(stderr, "%s:%d: %s: %s\n", file, line, (seriousness==s_information) ? "info" : "warning", message);
}
#endif

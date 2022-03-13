# fpurge.m4 serial 8
dnl Copyright (C) 2007, 2009-2018 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FPURGE],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_CHECK_FUNCS_ONCE([fpurge])
  AC_CHECK_FUNCS_ONCE([__fpurge])
  AC_CHECK_DECLS([fpurge], , , [[#include <stdio.h>]])
  if test "x$ac_cv_func_fpurge" = xyes; then
    HAVE_FPURGE=1
    # Detect BSD bug.  Only cygwin 1.7 is known to be immune.
    AC_CACHE_CHECK([whether fpurge works], [gl_cv_func_fpurge_works],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <stdio.h>
]],
            [FILE *f = fopen ("conftest.txt", "w+");
             if (!f)
               return 1;
             if (fputc ('a', f) != 'a')
               { fclose (f); return 2; }
             rewind (f);
             if (fgetc (f) != 'a')
               { fclose (f); return 3; }
             if (fgetc (f) != EOF)
               { fclose (f); return 4; }
             if (fpurge (f) != 0)
               { fclose (f); return 5; }
             if (putc ('b', f) != 'b')
               { fclose (f); return 6; }
             if (fclose (f) != 0)
               return 7;
             if ((f = fopen ("conftest.txt", "r")) == NULL)
               return 8;
             if (fgetc (f) != 'a')
               { fclose (f); return 9; }
             if (fgetc (f) != 'b')
               { fclose (f); return 10; }
             if (fgetc (f) != EOF)
               { fclose (f); return 11; }
             if (fclose (f) != 0)
               return 12;
             if (remove ("conftest.txt") != 0)
               return 13;
             return 0;
            ])],
      [gl_cv_func_fpurge_works=yes], [gl_cv_func_fpurge_works=no],
      [gl_cv_func_fpurge_works='guessing no'])])
    if test "x$gl_cv_func_fpurge_works" != xyes; then
      REPLACE_FPURGE=1
    fi
  else
    HAVE_FPURGE=0
  fi
  if test "x$ac_cv_have_decl_fpurge" = xno; then
    HAVE_DECL_FPURGE=0
  fi
])

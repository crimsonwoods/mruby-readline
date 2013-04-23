#include "mruby.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include <stdio.h>
#include <string.h>

#define BUF_INC_SIZE 128

static mrb_value
mrb_new_str_placement(mrb_state *mrb, char *cstr, size_t const len)
{
  struct RString *s;

  s = (struct RString*)mrb_obj_alloc(mrb, MRB_TT_STRING, mrb->string_class);
  if (NULL == s) {
    return mrb_nil_value();
  }

  s->len = len;
  s->aux.capa = len;
  s->ptr = cstr;
  s->ptr[len] = '\0';

  return mrb_obj_value(s);
}

static mrb_value
mrb_readall(mrb_state *mrb)
{
  int c;
  size_t sz = 0;
  size_t pos = 0;
  char *str = NULL;

  while (EOF != (c = fgetc(stdin))) {
    if ((pos + 1) >= sz) {
      void *new_s = mrb_realloc(mrb, str, sz + BUF_INC_SIZE);
      if (NULL == new_s) {
        mrb_free(mrb, str);
        mrb_raise(mrb, E_RUNTIME_ERROR, "no memory");
      }
      str = new_s;
      sz += BUF_INC_SIZE;
    }
    str[pos++] = (char)c;
  }

  if (0 == pos) {
    return mrb_nil_value();
  }

  return mrb_new_str_placement(mrb, str, pos);
}

static mrb_value
mrb_readdelim(mrb_state *mrb, char const *delim)
{
  int c;
  size_t sz = 0;
  size_t pos = 0;
  size_t icp = 0;
  char *scp = NULL;
  char *str = NULL;
  size_t const dlen = strlen(delim);
  size_t i;

  while (EOF != (c = fgetc(stdin))) {
    if ((pos + 1) >= sz) {
      void *new_s = mrb_realloc(mrb, str, sz + BUF_INC_SIZE);
      if (NULL == new_s) {
        mrb_free(mrb, str);
        mrb_raise(mrb, E_RUNTIME_ERROR, "no memory");
      }
      if (str != new_s) {
        if (NULL != scp) {
          scp = new_s + (scp - str);
        }
        str = new_s;
      }
      sz += BUF_INC_SIZE;
    }
    str[pos++] = (char)c;
    if ((c == *delim) && (NULL == scp)) {
      icp = pos - 1;
      scp = &str[icp];
    }
    if ((NULL != scp) && ((icp + dlen) == pos)) {
      if (0 == strncmp(scp, delim, dlen)) {
        break;
      }
      for (i = 1; i < dlen; ++i) {
        if (scp[i] == *delim) {
          icp += i;
          scp = &str[icp];
          break;
        }
      }
      if (i == dlen) {
        scp = NULL;
      }
    }
  }

  if (0 == pos) {
    return mrb_nil_value();
  }

  return mrb_new_str_placement(mrb, str, pos);
}

static mrb_value
mrb_gets(mrb_state *mrb, mrb_value self)
{
  mrb_value str;
  mrb_value rs;
  char const *delim;
  mrb_value obj_stdin, obj_argf;
  mrb_value *argv;
  int argc;
  mrb_sym sym_STDIN = mrb_intern_cstr(mrb, "STDIN");
  mrb_sym sym_gets = mrb_intern_cstr(mrb, "gets");
  mrb_sym sym_ARGF = mrb_intern_cstr(mrb, "ARGF");

  if (mrb_const_defined(mrb, mrb_obj_value(mrb->object_class), sym_ARGF)) {
    obj_argf = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), sym_ARGF);
  } else {
    obj_argf = mrb_nil_value();
  }
  if (!mrb_nil_p(obj_argf) && mrb_respond_to(mrb, obj_argf, sym_gets)) {
    mrb_get_args(mrb, "*", &argv, &argc);
    return mrb_funcall_argv(mrb, obj_argf, sym_gets, argc, argv);
  }

  if (mrb_const_defined(mrb, mrb_obj_value(mrb->object_class), sym_STDIN)) {
    obj_stdin = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), sym_STDIN);
  } else {
    obj_stdin = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$stdin"));
  }
  if (!mrb_nil_p(obj_stdin) && mrb_respond_to(mrb, obj_stdin, sym_gets)) {
    mrb_get_args(mrb, "*", &argv, &argc);
    return mrb_funcall_argv(mrb, obj_stdin, sym_gets, argc, argv);
  }

  if (0 == mrb_get_args(mrb, "|o", &rs)) {
    rs = mrb_gv_get(mrb, mrb_intern_cstr(mrb, "$/"));
  }

  if (mrb_nil_p(rs)) {
    str = mrb_readall(mrb);
  } else {
    if (mrb_type(rs) != MRB_TT_STRING) {
      rs = mrb_any_to_s(mrb, rs);
    }

    if (0 == RSTRING_LEN(rs)) {
      delim = "\n\n";
    } else {
      delim = RSTRING_PTR(rs);
    }

    str = mrb_readdelim(mrb, delim);
  }
  mrb_gv_set(mrb, mrb_intern_cstr(mrb, "$_"), str);
  return str;
}

void
mrb_mruby_readline_gem_init(mrb_state *mrb)
{
  struct RClass *krn;

  krn = mrb->kernel_module;
  mrb_define_module_function(mrb, krn, "gets", mrb_gets, ARGS_OPT(1));

  if (!mrb_class_defined(mrb, "EOFError")) {
    struct RClass *io_err;
    if (mrb_class_defined(mrb, "IOError")) {
      io_err = mrb_class_get(mrb, "IOError");
    } else {
      io_err = mrb_define_class(mrb, "IOError", mrb->eStandardError_class);
    }
    mrb_define_class(mrb, "EOFError", io_err);
  }
}

void
mrb_mruby_readline_gem_final(mrb_state *mrb)
{
}


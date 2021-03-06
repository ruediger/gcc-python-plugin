/*
   Copyright 2011, 2012 David Malcolm <dmalcolm@redhat.com>
   Copyright 2011, 2012 Red Hat, Inc.

   This is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#ifndef INCLUDED__GCC_PYTHON_H
#define INCLUDED__GCC_PYTHON_H

#include <gcc-plugin.h>
#include "autogenerated-config.h"
#include "tree.h"
#include "gimple.h"
#include "params.h"

/* GCC doesn't seem to give us an ID for "invalid event", so invent one: */
#define GCC_PYTHON_PLUGIN_BAD_EVENT (0xffff)

/*
  Define some macros to allow us to use cpychecker's custom attributes when
  compiling the plugin using itself (gcc-with-cpychecker), but also to turn
  them off when compiling with vanilla gcc (and other compilers).
*/
#if defined(WITH_CPYCHECKER_RETURNS_BORROWED_REF_ATTRIBUTE)
  #define CPYCHECKER_RETURNS_BORROWED_REF \
    __attribute__((cpychecker_returns_borrowed_ref))
#else
  #define CPYCHECKER_RETURNS_BORROWED_REF
#endif

#if defined(WITH_CPYCHECKER_STEALS_REFERENCE_TO_ARG_ATTRIBUTE)
  #define CPYCHECKER_STEALS_REFERENCE_TO_ARG(n) \
    __attribute__((cpychecker_steals_reference_to_arg(n)))
#else
  #define CPYCHECKER_STEALS_REFERENCE_TO_ARG(n)
#endif

#if defined(WITH_CPYCHECKER_TYPE_OBJECT_FOR_TYPEDEF_ATTRIBUTE)
  #define CPYCHECKER_TYPE_OBJECT_FOR_TYPEDEF(typename) \
     __attribute__((cpychecker_type_object_for_typedef(typename)))
#else
  #define CPYCHECKER_TYPE_OBJECT_FOR_TYPEDEF(typename)
#endif

/*
  PyObject shared header for wrapping GCC objects, for integration
  with GCC's garbage collector (so that things we wrap don't get collected
  from under us)
*/
typedef struct PyGccWrapper
{
     PyObject_HEAD

     /*
       Keep track of a linked list of all live wrapper objects,
       so that we can mark the wrapped objects for GCC's garbage
       collector:
     */
     struct PyGccWrapper *wr_prev;
     struct PyGccWrapper *wr_next;
} PyGccWrapper;

/*
  PyTypeObject subclass for PyGccWrapper, adding a GC-marking callback:
 */
typedef void (*wrtp_marker) (PyGccWrapper *wrapper);

typedef struct PyGccWrapperTypeObject
{
    PyHeapTypeObject wrtp_base;

    /* Callback for marking the wrapped objects when GCC's garbage
       collector runs: */
    wrtp_marker wrtp_mark;

} PyGccWrapperTypeObject;

/* gcc-python-wrapper.c */

/*
  Allocate a new PyGccWrapper of the given type, setting up:
    - ob_refcnt
    - ob_type
  and calling gcc_python_wrapper_track() on it:
*/
#define PyGccWrapper_New(ARG_structname, ARG_typeobj) \
    ( (ARG_structname*) _PyGccWrapper_New(ARG_typeobj) )

extern PyGccWrapper *
_PyGccWrapper_New(PyGccWrapperTypeObject *typeobj);

extern void
gcc_python_wrapper_track(PyGccWrapper *obj);

extern void
gcc_python_wrapper_dealloc(PyObject *obj);

extern PyTypeObject PyGccWrapperMetaType;
/*
  Macro DECLARE_SIMPLE_WRAPPER():
    ARG_structname:
      the struct tag for the resulting python wrapper class,
      e.g. "PyGccPass"

    ARG_typeobj:
      the singleton PyTypeObject for the resulting python wrapper class
      e.g. "gcc_PassType"

    ARG_typename:
      a C identifier to use when referring to instances of the underlying
      type e.g. "pass"

    ARG_wrappedtype:
      the GCC type being wrapped e.g. "struct opt_pass *"

    ARG_fieldname:
      the name of the field within the CPython struct, containing the pointer
      to the GCC data
 */
#define DECLARE_SIMPLE_WRAPPER(ARG_structname, ARG_typeobj, ARG_typename, ARG_wrappedtype, ARG_fieldname) \
  struct ARG_structname {           \
     struct PyGccWrapper head;      \
     ARG_wrappedtype ARG_fieldname; \
  };                                \
                                    \
  typedef struct ARG_structname ARG_structname;                          \
                                                                         \
  extern PyObject *                                                      \
  gcc_python_make_wrapper_##ARG_typename(ARG_wrappedtype ARG_fieldname); \
                                                                         \
  extern PyObject *                                                      \
  gcc_python_make_wrapper_##ARG_typename##_unique(ARG_wrappedtype ARG_fieldname); \
                                                                         \
  extern PyGccWrapperTypeObject ARG_typeobj                              \
    CPYCHECKER_TYPE_OBJECT_FOR_TYPEDEF(#ARG_structname);                 \
                                                                         \
  extern void                                                            \
  wrtp_mark_for_##ARG_structname(ARG_structname *wrapper);               \
  /* end of macro */

DECLARE_SIMPLE_WRAPPER(PyGccCallgraphEdge,
		       gcc_CallgraphEdgeType,
		       cgraph_edge,
                       struct cgraph_edge *, edge)

DECLARE_SIMPLE_WRAPPER(PyGccCallgraphNode,
		       gcc_CallgraphNodeType,
		       cgraph_node,
                       struct cgraph_node *, node)

DECLARE_SIMPLE_WRAPPER(PyGccPass,
		       gcc_PassType,
		       pass,
		       struct opt_pass *, pass)

DECLARE_SIMPLE_WRAPPER(PyGccLocation, 
		       gcc_LocationType,
		       location,
		       location_t, loc)

DECLARE_SIMPLE_WRAPPER(PyGccGimple, 
		       gcc_GimpleType,
		       gimple,
		       gimple, stmt);

DECLARE_SIMPLE_WRAPPER(PyGccEdge, 
		       gcc_EdgeType,
		       edge,
		       edge, e)

DECLARE_SIMPLE_WRAPPER(PyGccBasicBlock, 
		       gcc_BasicBlockType,
		       basic_block,
		       basic_block, bb)

DECLARE_SIMPLE_WRAPPER(PyGccCfg, 
		       gcc_CfgType,
		       cfg,
		       struct control_flow_graph *, cfg)

DECLARE_SIMPLE_WRAPPER(PyGccFunction, 
		       gcc_FunctionType,
		       function,
		       struct function *, fun)

DECLARE_SIMPLE_WRAPPER(PyGccOption,
		       gcc_OptionType,
                       opt_code,
                       enum opt_code, opt_code)

DECLARE_SIMPLE_WRAPPER(PyGccParameter,
		       gcc_ParameterType,
                       param_num,
		       compiler_param, param_num)

DECLARE_SIMPLE_WRAPPER(PyGccRtl,
                       gcc_RtlType,
                       rtl,
                       struct rtx_def *, insn)

DECLARE_SIMPLE_WRAPPER(PyGccTree,
		       gcc_TreeType,
		       tree, tree, t)

DECLARE_SIMPLE_WRAPPER(PyGccVariable,
		       gcc_VariableType,
		       variable,
		       struct varpool_node *, var)

PyObject *gcc_python_make_statement_list(tree t); // TODO DECLARE_SIMPLE_WRAPPER?

/* autogenerated-callgraph.c */
int autogenerated_callgraph_init_types(void);
void autogenerated_callgraph_add_types(PyObject *m);

/* autogenerated-cfg.c */
int autogenerated_cfg_init_types(void);
void autogenerated_cfg_add_types(PyObject *m);

/* autogenerated-function.c */
int autogenerated_function_init_types(void);
void autogenerated_function_add_types(PyObject *m);

/* autogenerated-gimple.c */
int autogenerated_gimple_init_types(void);
void autogenerated_gimple_add_types(PyObject *m);
PyGccWrapperTypeObject* gcc_python_autogenerated_gimple_type_for_stmt(gimple g);

/* autogenerated-location.c */
int autogenerated_location_init_types(void);
void autogenerated_location_add_types(PyObject *m);

/* autogenerated-option.c */
int autogenerated_option_init_types(void);
void autogenerated_option_add_types(PyObject *m);

/* autogenerated-parameter.c */
int autogenerated_parameter_init_types(void);
void autogenerated_parameter_add_types(PyObject *m);

/* autogenerated-pass.c */
int autogenerated_pass_init_types(void);
void autogenerated_pass_add_types(PyObject *m);
extern PyGccWrapperTypeObject gcc_GimplePassType;
extern PyGccWrapperTypeObject gcc_RtlPassType;
extern PyGccWrapperTypeObject gcc_SimpleIpaPassType;
extern PyGccWrapperTypeObject gcc_IpaPassType;

/* autogenerated-pretty-printer.c */
int autogenerated_pretty_printer_init_types(void);
void autogenerated_pretty_printer_add_types(PyObject *m);


/* autogenerated-rtl.c */
int autogenerated_rtl_init_types(void);
void autogenerated_rtl_add_types(PyObject *m);
PyGccWrapperTypeObject * gcc_python_autogenerated_rtl_type_for_stmt(struct rtx_def *insn);


/* autogenerated-tree.c */
int autogenerated_tree_init_types(void);
void autogenerated_tree_add_types(PyObject *m);

PyGccWrapperTypeObject*
gcc_python_autogenerated_tree_type_for_tree(tree t, int borrow_ref);

PyGccWrapperTypeObject*
gcc_python_autogenerated_tree_type_for_tree_code(enum tree_code code, int borrow_ref);

/* autogenerated-variable.c */
int autogenerated_variable_init_types(void);
void autogenerated_variable_add_types(PyObject *m);


PyObject *
gcc_python_string_or_none(const char *str_or_null);

PyObject *
VEC_tree_as_PyList(VEC(tree,gc) *vec_nodes);

void
gcc_python_double_int_as_text(double_int di, bool is_unsigned,
                              char *out, int bufsize)  __attribute__((nonnull));

PyObject *
gcc_python_int_from_double_int(double_int di, bool is_unsigned);

PyObject *
gcc_python_lazily_create_wrapper(PyObject **cache,
				 void *ptr,
				 PyObject *(*ctor)(void *ptr));
int
gcc_python_insert_new_wrapper_into_cache(PyObject **cache,
                                         void *ptr,
                                         PyObject *obj);


/* gcc-python.c */
int gcc_python_is_within_event(enum plugin_event *out_event);

char * gcc_python_strdup(const char *str) __attribute__((nonnull));

void gcc_python_print_exception(const char *msg);

/* Python 2 vs Python 3 compat: */
#if PY_MAJOR_VERSION == 3
/* Python 3: use PyUnicode for "str" and PyLong for "int": */
#define gcc_python_string_from_format PyUnicode_FromFormat
#define gcc_python_string_from_string PyUnicode_FromString
#define gcc_python_string_from_string_and_size PyUnicode_FromStringAndSize
#define gcc_python_string_as_string _PyUnicode_AsString
#define gcc_python_int_from_long PyLong_FromLong
#define gcc_python_int_check PyLong_Check
#define gcc_python_int_as_long PyLong_AsLong
#else
/* Python 2: use PyString for "str" and PyInt for "int": */
#define gcc_python_string_from_format PyString_FromFormat
#define gcc_python_string_from_string PyString_FromString
#define gcc_python_string_from_string_and_size PyString_FromStringAndSize
#define gcc_python_string_as_string PyString_AsString
#define gcc_python_int_from_long PyInt_FromLong
#define gcc_python_int_check PyInt_Check
#define gcc_python_int_as_long PyInt_AsLong
#endif

/*
  PEP-7
Local variables:
c-basic-offset: 4
indent-tabs-mode: nil
End:
*/

#endif /* INCLUDED__GCC_PYTHON_H */

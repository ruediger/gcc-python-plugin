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

#include <Python.h>
#include "gcc-python.h"
#include "gcc-python-wrappers.h"

/*
  Wrapper for various types in gcc/cgraph.h
    struct cgraph_edge *
    struct cgraph_node *
*/

PyObject *
gcc_CallgraphEdge_repr(struct PyGccCallgraphEdge * self)
{
    return gcc_python_string_from_format("%s()",
                                         Py_TYPE(self)->tp_name);
}

PyObject *
gcc_CallgraphEdge_str(struct PyGccCallgraphEdge * self)
{
    return gcc_python_string_from_format("%s()",
                                         Py_TYPE(self)->tp_name);
}

PyObject *
gcc_CallgraphNode_repr(struct PyGccCallgraphNode * self)
{
    return gcc_python_string_from_format("%s()",
                                         Py_TYPE(self)->tp_name);
}

PyObject *
gcc_CallgraphNode_str(struct PyGccCallgraphNode * self)
{
    return gcc_python_string_from_format("%s()",
                                         Py_TYPE(self)->tp_name);
}

PyObject *
gcc_CallgraphNode_get_callees(struct PyGccCallgraphNode * self)
{
    PyObject *result;
    struct cgraph_edge *edge;

    result = PyList_New(0);
    if (!result) {
	goto error;
    }

    for (edge = self->node->callees; edge ; edge = edge->next_callee) {
	PyObject *obj_var = gcc_python_make_wrapper_cgraph_edge(edge);
	if (!obj_var) {
	    goto error;
	}
	if (-1 == PyList_Append(result, obj_var)) {
	    Py_DECREF(obj_var);
	    goto error;
	}
        Py_DECREF(obj_var);
    }

    return result;

 error:
    Py_XDECREF(result);
    return NULL;
}

PyObject *
gcc_CallgraphNode_get_callers(struct PyGccCallgraphNode * self)
{
    PyObject *result;
    struct cgraph_edge *edge;

    result = PyList_New(0);
    if (!result) {
	goto error;
    }

    for (edge = self->node->callers; edge ; edge = edge->next_caller) {
	PyObject *obj_var = gcc_python_make_wrapper_cgraph_edge(edge);
	if (!obj_var) {
	    goto error;
	}
	if (-1 == PyList_Append(result, obj_var)) {
	    Py_DECREF(obj_var);
	    goto error;
	}
        Py_DECREF(obj_var);
    }

    return result;

 error:
    Py_XDECREF(result);
    return NULL;
}

PyObject *
gcc_python_make_wrapper_cgraph_edge(struct cgraph_edge * edge)
{
    struct PyGccCallgraphEdge *obj = NULL;

    obj = PyGccWrapper_New(struct PyGccCallgraphEdge, &gcc_CallgraphEdgeType);
    if (!obj) {
        goto error;
    }

    obj->edge = edge;

    return (PyObject*)obj;

error:
    return NULL;
}

void
wrtp_mark_for_PyGccCallgraphEdge(PyGccCallgraphEdge *wrapper)
{
    /* Mark the underlying object (recursing into its fields): */
    gt_ggc_mx_cgraph_edge(wrapper->edge);
}


PyObject *
real_make_cgraph_node_wrapper(void *ptr)
{
    struct cgraph_node * node = (struct cgraph_node *)ptr;
    struct PyGccCallgraphNode *obj = NULL;

    obj = PyGccWrapper_New(struct PyGccCallgraphNode,
                           &gcc_CallgraphNodeType);
    if (!obj) {
        goto error;
    }

    obj->node = node;

    return (PyObject*)obj;

error:
    return NULL;
}

void
wrtp_mark_for_PyGccCallgraphNode(PyGccCallgraphNode *wrapper)
{
    /* Mark the underlying object (recursing into its fields): */
    gt_ggc_mx_cgraph_node(wrapper->node);
}


static PyObject *cgraph_node_wrapper_cache = NULL;
PyObject *
gcc_python_make_wrapper_cgraph_node(struct cgraph_node * node)
{
    return gcc_python_lazily_create_wrapper(&cgraph_node_wrapper_cache,
					    node,
					    real_make_cgraph_node_wrapper);
}

PyObject *
gcc_python_get_callgraph_nodes(PyObject *self, PyObject *args)
{
    PyObject *result;
    struct cgraph_node *node;

    /* For debugging, see GCC's dump of things: */
    if (0) {
        fprintf(stderr, "----------------BEGIN----------------\n");
        dump_cgraph (stderr);
        fprintf(stderr, "---------------- END ----------------\n");
    }

    result = PyList_New(0);
    if (!result) {
	goto error;
    }

    for (node = cgraph_nodes; node; node = node->next) {
	PyObject *obj_var = gcc_python_make_wrapper_cgraph_node(node);
	if (!obj_var) {
	    goto error;
	}
	if (-1 == PyList_Append(result, obj_var)) {
	    Py_DECREF(obj_var);
	    goto error;
	}
        Py_DECREF(obj_var);
    }

    return result;

 error:
    Py_XDECREF(result);
    return NULL;
}

/*
  PEP-7
Local variables:
c-basic-offset: 4
indent-tabs-mode: nil
End:
*/


// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 
// (c) COPRIGHT URI/MIT 1996,1998,1999
// Please first read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//	jhrg,jimg	James Gallagher <jgallagher@gso.uri.edu>

// Implementation for the CE Clause class.


#include "config.h"

#include <assert.h>

#include <algorithm>

#include "expr.h"
#include "DDS.h"
#include "Clause.h"

using std::cerr;
using std::endl;
#if defined(_MSC_VER) && (_MSC_VER == 1200)  //  VC++ 6.0 only
using std::for_each;
#endif

Clause::Clause(const int oper, rvalue *a1, rvalue_list *rv)
    : _op(oper), _b_func(0), _bt_func(0), _arg1(a1), _args(rv) 
{
    assert(OK());
}

Clause::Clause(bool_func func, rvalue_list *rv)
    : _op(0), _b_func(func), _bt_func(0), _arg1(0), _args(rv)
{
    assert(OK());

    if (_args)			// account for null arg list
	_argc = _args->size();
    else
	_argc = 0;
}

Clause::Clause(btp_func func, rvalue_list *rv)
    : _op(0), _b_func(0), _bt_func(func), _arg1(0), _args(rv)
{
    assert(OK());

    if (_args)
	_argc = _args->size();
    else
	_argc = 0;
}

Clause::Clause() : _op(0), _b_func(0), _bt_func(0), _arg1(0), _args(0)
{
}

static inline void
delete_rvalue(rvalue *rv)
{
    delete rv; rv = 0;
}

Clause::~Clause() 
{
    if (_arg1) {
	delete _arg1; _arg1 = 0;
    }
    
    if (_args) {
	// _args is a pointer to a vector<rvalue*> and we must must delete
	// each rvalue pointer here explicitly. 02/03/04 jhrg
	for_each(_args->begin(), _args->end(), delete_rvalue);
	delete _args; _args = 0;
    }
}

/** @brief Checks the "representation invariant" of a clause. */
bool
Clause::OK()
{
    // Each clause object can contain one of: a relational clause, a boolean
    // function clause or a BaseType pointer function clause. It must have a
    // valid argument list.
    //
    // But, a valid arg list might contain zero arguments! 10/16/98 jhrg
    bool relational = (_op && !_b_func && !_bt_func);
    bool boolean = (!_op && _b_func && !_bt_func);
    bool basetype = (!_op && !_b_func && _bt_func);

    if (relational)
	return _arg1 && _args;
    else if (boolean || basetype)
	return true;		// Until we check arguments...10/16/98 jhrg
    else 
	return false;
}

/** @brief Return true if the clause returns a boolean value. */
bool 
Clause::boolean_clause()
{
    assert(OK());

    return _op || _b_func;
}

/** @brief Return true if the clause returns a value in a BaseType pointer. */
bool
Clause::value_clause()
{
    assert(OK());

    return (_bt_func != 0);
}

/** @brief Evaluate a clause which returns a boolean value */
bool 
Clause::value(const string &dataset, DDS &dds) 
{
    assert(OK());
    assert(_op || _b_func);

    if (_op) {			// Is it a relational clause?
	// rvalue::bvalue(...) returns the rvalue encapsulated in a
	// BaseType *.
	BaseType *btp = _arg1->bvalue(dataset, dds);
	// The list of rvalues is an implicit logical OR, so assume
	// FALSE and return TRUE for the first TRUE subclause.
	bool result = false;
	for (rvalue_list_iter i = _args->begin();
	     i != _args->end() && !result;
	     i++)
	{
	    result = result || btp->ops((*i)->bvalue(dataset, dds),
					_op, dataset);
	}

	return result;
    }
    else if (_b_func) {		// ...A bool function?
        // build_btp_args() calls read for BaseTypes
	BaseType **argv = build_btp_args(_args, dds);

	bool result = (*_b_func)(_argc, argv, dds);
	delete[] argv;		// Cache me!
	argv = 0;

	return result;
    }
    else {
	cerr << "Internal error: " << endl
	     << "The constraint expression parser built an invalid clause."
	     << endl
	     << "Please report this error." << endl;
	return false;
    }
}

/** @brief Evaluate a clause that returns a value via a BaseType pointer. */
bool 
Clause::value(const string &, DDS &dds, BaseType **value) 
{
    assert(OK());
    assert(_bt_func);

    if (_bt_func) {
        // build_btp_args() is a function defiend in RValue.cc. It reads
        // (using BaseType::read()) values as it builds the arguments.
	BaseType **argv = build_btp_args(_args, dds);

	*value = (*_bt_func)(_argc, argv, dds);
	delete[] argv;		// Cache me!
	argv = 0;

	if (*value) {
	    (*value)->set_read_p(true);
	    (*value)->set_send_p(true);
	    return true;
	}
	else {
	    return false;
	}
    }
    else {
	cerr << "Internal error:" << endl
	    << "The constraint expression parser built an invalid clause."
	    << endl
	    << "Please report this error." << endl;
	return false;
    }
}


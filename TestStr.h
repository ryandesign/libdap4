// -*- C++ -*-

// Interface for TestStr type. See TestByte.h
//
// jhrg 1/12/95

/* $Log: TestStr.h,v $
/* Revision 1.6  1996/03/05 18:50:07  jimg
/* Relaced <limits.h> with "dods-limits.h".
/*
 * Revision 1.5  1995/12/09  01:07:27  jimg
 * Added changes so that relational operators will work properly for all the
 * datatypes (including Sequences). The relational ops are evaluated in
 * DDS::eval_constraint() after being parsed by DDS::parse_constraint().
 *
 * Revision 1.4  1995/12/06  19:55:44  jimg
 * Changes read() member function from three arguments to two.
 *
 * Revision 1.3  1995/02/10  02:34:00  jimg
 * Modified Test<class>.h and .cc so that they used to new definitions of
 * read_val().
 * Modified the classes read() so that they are more in line with the
 * class library's intended use in a real subclass set.
 *
 * Revision 1.2  1995/01/19  21:59:44  jimg
 * Added read_val from dummy_read.cc to the sample set of sub-class
 * implementations.
 * Changed the declaration of readVal in BaseType so that it names the
 * mfunc read_val (to be consistant with the other mfunc names).
 * Removed the unnecessary duplicate declaration of the abstract virtual
 * mfuncs read and (now) read_val from the classes Byte, ... Grid. The
 * declaration in BaseType is sufficient along with the decl and definition
 * in the *.cc,h files which contain the subclasses for Byte, ..., Grid.
 *
 * Revision 1.1  1995/01/19  20:20:55  jimg
 * Created as an example of subclassing the class hierarchy rooted at
 * BaseType.
 *
 */

#ifndef _TestStr_h
#define _TestStr_h 1

#ifdef __GNUG__
#pragma interface
#endif

#ifdef NEVER
#include <limits.h>
#endif

#include "dods-limits.h"
#include "Str.h"

class TestStr: public Str {
public:
    TestStr(const String &n = (char *)0);
    virtual ~TestStr() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const String &dataset);
};

typedef TestStr * TestStrPtr;

#endif


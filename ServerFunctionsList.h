// ServerFunctionsList.h

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2013 OPeNDAP, Inc.
// Author: Nathan Potter <npotter@opendap.org>
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

#ifndef I_ServerFunctionsList_h
#define I_ServerFunctionsList_h 1

#include <map>
#include <string>

#include "ServerFunction.h"

#include <expr.h>

namespace libdap {
class ServerFunctionsListUnitTest;
class ConstraintEvaluator;

//#include "BESObj.h"

class ServerFunctionsList {
private:
    static ServerFunctionsList * d_instance;
    std::multimap<std::string, libdap::ServerFunction *> d_func_list;

    static void initialize_instance();
    static void delete_instance();

    virtual ~ServerFunctionsList(void);

    friend class libdap::ServerFunctionsListUnitTest;

protected:
    ServerFunctionsList() {}

public:
    static ServerFunctionsList * TheList();


    virtual void add_function(libdap::ServerFunction *func);

    virtual bool find_function(const std::string &name, libdap::bool_func *f) const;
    virtual bool find_function(const std::string &name, libdap::btp_func  *f) const;
    virtual bool find_function(const std::string &name, libdap::proj_func *f) const;

    //virtual void dump(ostream &strm) const;

    std::multimap<string,libdap::ServerFunction *>::iterator begin();
    std::multimap<string,libdap::ServerFunction *>::iterator end();
    ServerFunction *getFunction(std::multimap<string,libdap::ServerFunction *>::iterator it);

    virtual void getFunctionNames(vector<string> *names);

};

}

#endif // I_ServerFunctionsList_h
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2014 OPeNDAP, Inc.
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sstream>

#include "Byte.h"
#include "Int16.h"
#include "UInt16.h"
#include "Int32.h"
#include "UInt32.h"
#include "Float32.h"
#include "Float64.h"
#include "Str.h"
#include "Url.h"
#include "Array.h"
#include "Structure.h"
#include "Sequence.h"
#include "Grid.h"

#include "DDS.h"
#include "DMR.h"
#include "XMLWriter.h"
#include "D4BaseTypeFactory.h"

#include "GNURegex.h"
#include "GetOpt.h"
#include "util.h"
#include "debug.h"
#include "GetOpt.h"

#include "testFile.h"
#include "test_config.h"

static bool debug = false;

#undef DBG
#define DBG(x) do { if (debug) {x;} } while(false)

using namespace CppUnit;
using namespace std;

namespace libdap {

class DMRTest: public TestFixture {
private:

public:
    DMRTest() {
    }
    ~DMRTest() {
    }

    void setUp() {
    }

    void tearDown() {
    }

    bool re_match(Regex &r, const string &s) {
        int match = r.match(s.c_str(), s.length());
        DBG(cerr << "Match: " << match << " should be: " << s.length() << endl);
        return match == static_cast<int> (s.length());
    }

    void test_template(const string &dds_file, const string &dmr_baseline, const string &attr = "") {
		try {
			string prefix = string(TEST_SRC_DIR) + "/dds-testsuite/";

			BaseTypeFactory factory;
			DDS dds(&factory, dds_file);
			dds.parse(prefix + dds_file);
			DBG(cerr << "DDS: " << endl; dds.print(cerr));

			if (!attr.empty()) {
				DAS das;
				das.parse(prefix + attr);
				dds.transfer_attributes(&das);
			}

			D4BaseTypeFactory d4_factory;
			DMR dmr(&d4_factory, dds);
			XMLWriter xml;
			dmr.print_dap4(xml);
			DBG(cerr << "DMR: " << endl << xml.get_doc() << endl);

			CPPUNIT_ASSERT(string(xml.get_doc()) == readTestBaseline(prefix + dmr_baseline));
		}
    	catch (Error &e) {
    		CPPUNIT_FAIL(string("Caught Error: ") + e.get_error_message());
    	}
    }
    CPPUNIT_TEST_SUITE( DMRTest );

    CPPUNIT_TEST(test_dmr_from_dds_1);
    CPPUNIT_TEST(test_dmr_from_dds_2);
    CPPUNIT_TEST(test_dmr_from_dds_3);
    CPPUNIT_TEST(test_dmr_from_dds_4);
    CPPUNIT_TEST(test_dmr_from_dds_5);

    CPPUNIT_TEST(test_dmr_from_dds_with_attr_1);
    CPPUNIT_TEST(test_dmr_from_dds_with_attr_2);

    CPPUNIT_TEST_SUITE_END();

    // Test a DDS with simple scalar types and no attributes
    void test_dmr_from_dds_1() {
    	test_template("test.1", "test.1.dmr");
    }

    // What about arrays? This should build shared dimensions
    void test_dmr_from_dds_2() {
    	test_template("fnoc1.nc.dds", "fnoc1.nc.dmr");
    }

    void test_dmr_from_dds_3() {
    	test_template("3B42.980909.5.HDF.dds", "3B42.980909.5.HDF.dmr");
    }

    void test_dmr_from_dds_4() {
    	test_template("S2000415.HDF.dds", "S2000415.HDF.dmr");
    }

    void test_dmr_from_dds_5() {
    	test_template("coads_climatology.nc.dds", "coads_climatology.nc.dmr");
    }

    void test_dmr_from_dds_with_attr_1() {
    	test_template("test.1", "test.1.attr.dmr", "test.1.das");
    }

    void  test_dmr_from_dds_with_attr_2() {
    	// The 'hacked' file has global attributes
    	test_template("3B42.980909.5.HDF.dds", "3B42.980909.5.hacked.2.HDF.attr.dmr", "3B42.980909.5.hacked.2.HDF.das");
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DMRTest);

} // namepsace libdap

int main(int argc, char*argv[]) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    GetOpt getopt(argc, argv, "d");
    char option_char;

    while ((option_char = getopt()) != EOF)
        switch (option_char) {
        case 'd':
            debug = 1;  // debug is a static global
            break;
        default:
            break;
        }

    bool wasSuccessful = true;
    string test = "";
    int i = getopt.optind;
    if (i == argc) {
        // run them all
        wasSuccessful = runner.run("");
    }
    else {
        while (i < argc) {
            test = string("libdap::DMRTest::") + argv[i++];
            DBG(cerr << "test: " << test << endl);
            wasSuccessful = wasSuccessful && runner.run(test);
        }
    }

    xmlMemoryDump();

    return wasSuccessful ? 0 : 1;
}


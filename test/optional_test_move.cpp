// Copyright (C) 2014 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/lib/optional for documentation.
//
// You are welcome to contact the author at:
//  akrzemi1@gmail.com
//
// Revisions:
//
#include<iostream>
#include<stdexcept>
#include<string>

#define BOOST_ENABLE_ASSERT_HANDLER

#include "boost/bind/apply.hpp" // Included just to test proper interaction with boost::apply<> as reported by Daniel Wallin
#include "boost/mpl/bool.hpp"
#include "boost/mpl/bool_fwd.hpp"  // For mpl::true_ and mpl::false_

#include "boost/optional/optional.hpp"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "boost/none.hpp"

#include "boost/test/minimal.hpp"

#include "optional_test_common.cpp"

//#ifndef BOOST_OPTIONAL_NO_CONVERTING_ASSIGNMENT
//#ifndef BOOST_OPTIONAL_NO_CONVERTING_COPY_CTOR

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES

enum State
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sIntConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sIntConstructed), i(i) {}
};


struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyAssigned; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveAssigned; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyAssigned; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveAssigned; val = std::move(o.val); o.s = sMovedFrom; return *this; }
};

bool operator==( Oracle const& a, Oracle const& b ) { return a.val.i == b.val.i; }
bool operator!=( Oracle const& a, Oracle const& b ) { return a.val.i != b.val.i; }


void test_move_ctor_from_U()
{
  optional<Oracle> o1 ((OracleVal()));
  BOOST_CHECK(o1);
  BOOST_CHECK(o1->s == sValueMoveConstructed || o1->s == sMoveConstructed);
    
  OracleVal v1;
  optional<Oracle> o2 (v1);
  BOOST_CHECK(o2);
  std::cout << "AK" << " @@@ " << o2->s << std::endl;
  BOOST_CHECK(o2->s == sValueCopyConstructed || o2->s == sCopyConstructed || o2->s == sMoveConstructed );
  BOOST_CHECK(v1.s == sIntConstructed);
    
  optional<Oracle> o3 (boost::move(v1));
  BOOST_CHECK(o3);
  BOOST_CHECK(o3->s == sValueMoveConstructed || o3->s == sMoveConstructed);
  BOOST_CHECK(v1.s == sMovedFrom);
}

void test_move_ctor_form_T()
{
  optional<Oracle> o1 ((Oracle()));
  BOOST_CHECK(o1);
  BOOST_CHECK(o1->s == sMoveConstructed);
  
  Oracle v1;
  optional<Oracle> o2 (v1);
  BOOST_CHECK(o2);
  BOOST_CHECK(o2->s == sCopyConstructed);
  BOOST_CHECK(v1.s == sDefaultConstructed);
    
  optional<Oracle> o3 (boost::move(v1));
  BOOST_CHECK(o3);
  BOOST_CHECK(o3->s == sMoveConstructed);
  BOOST_CHECK(v1.s == sMovedFrom);
}

void test_move_ctor_from_optional_T()
{
  optional<Oracle> o1;
  optional<Oracle> o2(boost::move(o1));
  
  BOOST_CHECK(!o1);
  BOOST_CHECK(!o2);
  
  optional<Oracle> o3((Oracle()));
  optional<Oracle> o4(boost::move(o3));
  BOOST_CHECK(o3);
  BOOST_CHECK(o4);
  BOOST_CHECK(o3->s == sMovedFrom);
  BOOST_CHECK(o4->s == sMoveConstructed);
  
  optional<Oracle> o5((optional<Oracle>()));
  BOOST_CHECK(!o5);
  
  optional<Oracle> o6((optional<Oracle>(Oracle())));
  BOOST_CHECK(o6);
  BOOST_CHECK(o6->s == sMoveConstructed);
  
  optional<Oracle> o7(o6); // does copy ctor from non-const lvalue compile?
}

void test_move_assign_from_U()
{
  optional<Oracle> o1;
  o1 = OracleVal();
  BOOST_CHECK(o1);
  
  BOOST_CHECK(o1->s == sValueMoveConstructed);  
  
  o1 = OracleVal();
  BOOST_CHECK(o1);
  BOOST_CHECK(o1->s == sMoveAssigned); 
    
  OracleVal v1;
  optional<Oracle> o2;
  o2 = v1;
  BOOST_CHECK(o2);
  BOOST_CHECK(o2->s == sValueCopyConstructed);
  BOOST_CHECK(v1.s == sIntConstructed);
  o2 = v1;
  BOOST_CHECK(o2);
  BOOST_CHECK(o2->s == sCopyAssigned || o2->s == sMoveAssigned);
  BOOST_CHECK(v1.s == sIntConstructed);
    
  optional<Oracle> o3;
  o3 = boost::move(v1);
  BOOST_CHECK(o3);
  BOOST_CHECK(o3->s == sValueMoveConstructed);
  BOOST_CHECK(v1.s == sMovedFrom);
}

void test_move_assign_from_T()
{
  optional<Oracle> o1;
  o1 = Oracle();
  BOOST_CHECK(o1);
  BOOST_CHECK(o1->s == sMoveConstructed);  
  
  o1 = Oracle();
  BOOST_CHECK(o1);
  BOOST_CHECK(o1->s == sMoveAssigned); 
    
  Oracle v1;
  optional<Oracle> o2;
  o2 = v1;
  BOOST_CHECK(o2);
  BOOST_CHECK(o2->s == sCopyConstructed);
  BOOST_CHECK(v1.s == sDefaultConstructed);
  o2 = v1;
  BOOST_CHECK(o2);
  BOOST_CHECK(o2->s == sCopyAssigned);
  BOOST_CHECK(v1.s == sDefaultConstructed);
    
  optional<Oracle> o3;
  o3 = boost::move(v1);
  BOOST_CHECK(o3);
  BOOST_CHECK(o3->s == sMoveConstructed);
  BOOST_CHECK(v1.s == sMovedFrom);
}

void test_move_ssign_from_optional_T()
{
    optional<Oracle> o1;
    optional<Oracle> o2;
    o1 = optional<Oracle>();
    BOOST_CHECK(!o1);
    optional<Oracle> o3((Oracle()));
    o1 = o3;
    /*BOOST_CHECK(o3);
    BOOST_CHECK(o3->s == sDefaultConstructed);
    BOOST_CHECK(o1);
    BOOST_CHECK(o1->s == sCopyConstructed);
    
    o2 = boost::move(o3);
    BOOST_CHECK(o3);
    BOOST_CHECK(o3->s == sMovedFrom);
    BOOST_CHECK(o2);
    BOOST_CHECK(o2->s == sMoveConstructed);
    
    o2 = optional<Oracle>((Oracle()));
    BOOST_CHECK(o2);
    BOOST_CHECK(o2->s == sMoveAssigned);*/
}


#endif

int test_main( int, char* [] )
{
  try
  {
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    test_move_ctor_from_U();
    test_move_ctor_form_T();
    test_move_ctor_from_optional_T();
    test_move_assign_from_U();
    test_move_assign_from_T();
    test_move_ssign_from_optional_T();
#endif
  }
  catch ( ... )
  {
    BOOST_ERROR("Unexpected Exception caught!");
  }

  return 0;
}



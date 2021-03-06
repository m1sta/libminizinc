/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <minizinc/values.hh>
#include <climits>

namespace MiniZinc {
  
  const IntVal IntVal::minint = IntVal(INT_MIN);
  const IntVal IntVal::maxint = IntVal(INT_MAX);
  const IntVal IntVal::infinity = IntVal(1,true);
  
}
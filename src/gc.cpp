/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "gc.h"

namespace lutze
{
    boost::mutex gc::gc_registry_mutex;
    gc_set gc::gc_registry;
}

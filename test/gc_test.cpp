/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "gc.h"
#include "gc_container.h"

using namespace lutze;

class global_fixture
{
public:
    global_fixture() // setup
    {
        gc::gc_init();
    }

    virtual ~global_fixture() // teardown
    {
        gc::gc_term();
    }
};

BOOST_GLOBAL_FIXTURE(global_fixture);

class collection_fixture
{
public:
    collection_fixture() // setup
    {
    }

    virtual ~collection_fixture() // teardown
    {
        gc::get_gc().final_collect();
    }
};

BOOST_FIXTURE_TEST_SUITE(collection_test, collection_fixture)

namespace test_version
{
    BOOST_AUTO_TEST_CASE(test_version)
    {
        BOOST_CHECK_NE(gc::get_gc().gc_version(), "");
    }
}

namespace test_variadic_new_gc
{
    class test_object0 : public gc_object
    {
    public:
        test_object0()
        {
        }
    };

    class test_object1 : public gc_object
    {
    public:
        test_object1(const std::string a1)
        {
        }
    };

    class test_object2 : public gc_object
    {
    public:
        test_object2(const std::string a1, const std::string a2)
        {
        }
    };

    class test_object9 : public gc_object
    {
    public:
        test_object9(const std::string a1, const std::string a2, const std::string a3,
                     const std::string a4, const std::string a5, const std::string a6,
                     const std::string a7, const std::string a8, const std::string a9)
        {
        }
    };

    typedef gc_ptr<test_object0> test_object0_ptr;
    typedef gc_ptr<test_object1> test_object1_ptr;
    typedef gc_ptr<test_object2> test_object2_ptr;
    typedef gc_ptr<test_object9> test_object9_ptr;

    BOOST_AUTO_TEST_CASE(test_variadic_new_gc)
    {
        test_object0_ptr test0 = new_gc<test_object0>();
        test_object1_ptr test1 = new_gc<test_object1>("a1");
        test_object2_ptr test2 = new_gc<test_object2>("a1", "a2");
        test_object9_ptr test9 = new_gc<test_object9>("a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9");
        gc::get_gc().collect(true);
    }
}

namespace test_collect
{
    int32_t instance_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++instance_count;
        }

        virtual ~test_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    test_object_ptr _test_collect()
    {
        test_object_ptr test = new_gc<test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_collect)
    {
        _test_collect();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_collect_mark
{
    int32_t mark_count = 0;

    class test_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    test_object_ptr _test_collect_mark()
    {
        test_object_ptr test = new_gc<test_object>();
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 1);
        return test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_collect_mark)
    {
        _test_collect_mark();
        gc::get_gc().collect(true);
    }
}

namespace null_member_collect
{
    int32_t instance_count = 0;

    class null_member_object;
    typedef gc_ptr<null_member_object> null_member_object_ptr;

    class null_member_object : public gc_object
    {
    public:
        null_member_object()
        {
            ++instance_count;
        }

        virtual ~null_member_object()
        {
            --instance_count;
        }

        null_member_object_ptr null_child1;
        null_member_object_ptr null_child2;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(null_child1);
            gc->mark(null_child2);
        }
    };

    null_member_object_ptr _test_null_member_collect()
    {
        null_member_object_ptr test = new_gc<null_member_object>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return null_member_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_null_member_collect)
    {
        _test_null_member_collect();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace member_collect
{
    int32_t instance_count = 0;
    int32_t child_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++child_count;
        }

        virtual ~test_object()
        {
            --child_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    class member_test_object : public gc_object
    {
    public:
        member_test_object()
        {
            ++instance_count;
            child1 = new_gc<test_object>();
            child2 = new_gc<test_object>();
        }

        virtual ~member_test_object()
        {
            --instance_count;
        }

        test_object_ptr child1;
        test_object_ptr child2;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child1);
            gc->mark(child2);
        }
    };

    typedef gc_ptr<member_test_object> member_test_object_ptr;

    member_test_object_ptr _test_member_collect()
    {
        member_test_object_ptr test = new_gc<member_test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        BOOST_CHECK_NE(child_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test->child1);
        gc::get_gc().unmark(test->child2);
        gc::get_gc().unmark(test); // simulate out of scope
        return member_test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_member_collect)
    {
        _test_member_collect();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
        BOOST_CHECK_EQUAL(child_count, 0);
    }
}

namespace non_gc_collect
{
    int32_t instance_count = 0;
    int32_t managed_count = 0;
    int32_t unmanaged_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++managed_count;
        }

        virtual ~test_object()
        {
            --managed_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    class unmanaged_object
    {
    public:
        unmanaged_object()
        {
            ++unmanaged_count;
        }

        virtual ~unmanaged_object()
        {
            --unmanaged_count;
        }
    };

    class member_test_object : public gc_object
    {
    public:
        member_test_object()
        {
            ++instance_count;
            child1 = new_gc<test_object>();
            child2 = new unmanaged_object();
        }

        virtual ~member_test_object()
        {
            --instance_count;
            delete child2;
        }

        test_object_ptr child1;
        unmanaged_object* child2;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child1);
        }
    };

    typedef gc_ptr<member_test_object> member_test_object_ptr;

    member_test_object_ptr _member_test_object()
    {
        member_test_object_ptr test = new_gc<member_test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        BOOST_CHECK_NE(managed_count, 0);
        BOOST_CHECK_NE(unmanaged_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test->child1);
        gc::get_gc().unmark(test); // simulate out of scope
        return member_test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_non_gc_collect)
    {
        _member_test_object();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
        BOOST_CHECK_EQUAL(managed_count, 0);
        BOOST_CHECK_EQUAL(unmanaged_count, 0);
    }
}

namespace cyclic_reference
{
    int32_t instance_count = 0;

    class cycle_object;
    typedef gc_ptr<cycle_object> cycle_object_ptr;

    class cycle_object : public gc_object
    {
    public:
        cycle_object()
        {
            ++instance_count;
        }

        virtual ~cycle_object()
        {
            --instance_count;
        }

        virtual void mark_members(gc* gc) const
        {
            gc->mark(other);
        }

        void set_other(cycle_object_ptr other)
        {
            this->other = other;
        }

        cycle_object_ptr other;
    };

    cycle_object_ptr _test_cyclic_references()
    {
        cycle_object_ptr test1 = new_gc<cycle_object>();
        cycle_object_ptr test2 = new_gc<cycle_object>();
        test1->set_other(test2);
        test2->set_other(test1);
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test1); // simulate out of scope
        gc::get_gc().unmark(test2); // simulate out of scope
        return cycle_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_cyclic_references)
    {
        _test_cyclic_references();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace recursive_stack
{
    int32_t instance_count = 0;

    class fib_object : public gc_object
    {
    public:
        fib_object()
        {
            ++instance_count;
        }

        virtual ~fib_object()
        {
            --instance_count;
        }

        int32_t fibonacci(int32_t n)
        {
            if (n < 2)
                return n;
            return new_gc<fib_object>()->fibonacci(n - 1) + new_gc<fib_object>()->fibonacci(n - 2);
        }
    };

    typedef gc_ptr<fib_object> fib_object_ptr;

    fib_object_ptr _test_recursive()
    {
        fib_object_ptr test = new_gc<fib_object>();
        BOOST_CHECK_EQUAL(test->fibonacci(20), 6765);
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return fib_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_recursive)
    {
        _test_recursive();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace deep_call_graph_reference
{
    int32_t instance_count = 0;
    int32_t level_count = 0;

    class deep_object;
    typedef gc_ptr<deep_object> deep_object_ptr;

    class deep_object : public gc_object
    {
    public:
        deep_object()
        {
            ++instance_count;
            if (++level_count < 100)
            {
                child1 = new_gc<deep_object>();
                child2 = new_gc<deep_object>();
                child3 = new_gc<deep_object>();
            }
        }

        virtual ~deep_object()
        {
            --instance_count;
        }

        deep_object_ptr child1;
        deep_object_ptr child2;
        deep_object_ptr child3;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child1);
            gc->mark(child2);
            gc->mark(child3);
        }
    };

    deep_object_ptr _test_deep_call()
    {
        deep_object_ptr test = new_gc<deep_object>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return deep_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_deep_call)
    {
        _test_deep_call();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_static
{
    int32_t instance_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++instance_count;
        }

        virtual ~test_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    test_object_ptr _test_static()
    {
        test_object_ptr test = new_static_gc<test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().unmark(test); // simulate out of scope
        return test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_static)
    {
        _test_static();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_static_gc().final_collect();
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_static_transfer
{
    int32_t instance_count = 0;
    int32_t child_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++child_count;
        }

        virtual ~test_object()
        {
            --child_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    class member_test_object : public gc_object
    {
    public:
        member_test_object()
        {
            ++instance_count;
            child = create_static();
        }

        virtual ~member_test_object()
        {
            --instance_count;
        }

        test_object_ptr child;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child);
        }

        static test_object_ptr create_static()
        {
            static test_object_ptr static_test = new_static_gc<test_object>();
            return static_test;
        }
    };

    typedef gc_ptr<member_test_object> member_test_object_ptr;

    member_test_object_ptr _test_static_transfer()
    {
        member_test_object_ptr test = new_gc<member_test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        BOOST_CHECK_NE(child_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return member_test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_static_transfer)
    {
        _test_static_transfer();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
        BOOST_CHECK_NE(child_count, 0);
        gc::get_static_gc().final_collect();
        BOOST_CHECK_EQUAL(child_count, 0);
    }
}

namespace test_static_promotion
{
    int32_t instance_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            ++instance_count;
        }

        virtual ~test_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    class test_static : public gc_object
    {
    public:
        test_static()
        {
            member_object = new_gc<test_object>();
        }

        test_object_ptr member_object;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(member_object);
        }
    };

    typedef gc_ptr<test_static> test_static_ptr;

    test_object_ptr _test_static_promotion()
    {
        test_static_ptr test = new_static_gc<test_static>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().unmark(test->member_object); // simulate out of scope
        gc::get_gc().collect(true);
        return test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_static_promotion)
    {
        _test_static_promotion();
        BOOST_CHECK_NE(instance_count, 0); // should still be in scope
        gc::get_gc().final_collect();
        BOOST_CHECK_NE(instance_count, 0); // should be promoted to static object
        gc::get_static_gc().final_collect();
        BOOST_CHECK_EQUAL(instance_count, 0); // only destroyed when static gc collected
    }
}

namespace test_set
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef set_ptr< std::set<elem_object_ptr> > elem_set;

    elem_set _test_set()
    {
        elem_set test = new_set<elem_set::set_type>();
        for (int32_t i = 0; i < 100; ++i)
            test.insert(new_gc<elem_object>());
        BOOST_CHECK_EQUAL(instance_count, 100);
        gc::get_gc().collect(true);
        for (elem_set::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
            gc::get_gc().unmark(*elem);
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_set();
    }

    BOOST_AUTO_TEST_CASE(test_set)
    {
        _test_set();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_set_mark
{
    int32_t mark_count = 0;

    class elem_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef set_ptr< std::set<elem_object_ptr> > elem_set;

    elem_set _test_set_mark()
    {
        elem_set test = new_set<elem_set::set_type>();
        for (int32_t i = 0; i < 100; ++i)
            test.insert(new_gc<elem_object>());
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 100);
        return elem_set();
    }

    BOOST_AUTO_TEST_CASE(test_set_mark)
    {
        _test_set_mark();
        gc::get_gc().collect(true);
    }
}

namespace test_map_key
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<elem_object_ptr, int32_t> > elem_map;

    elem_map _test_map_key()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 100; ++i)
            test.insert(std::make_pair(new_gc<elem_object>(), i));
        BOOST_CHECK_EQUAL(instance_count, 100);
        gc::get_gc().collect(true);
        for (elem_map::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
            gc::get_gc().unmark(elem->first);
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key)
    {
        _test_map_key();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_map_key_mark
{
    int32_t mark_count = 0;

    class elem_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<elem_object_ptr, int32_t> > elem_map;

    elem_map _test_map_key_mark()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 100; ++i)
            test.insert(std::make_pair(new_gc<elem_object>(), i));
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 100);
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key_mark)
    {
        _test_map_key_mark();
        gc::get_gc().collect(true);
    }
}

namespace test_map_value
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<int32_t, elem_object_ptr> > elem_map;

    elem_map _test_map_value()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
            test.insert(std::make_pair(i, new_gc<elem_object>()));
        BOOST_CHECK_EQUAL(instance_count, 10);
        gc::get_gc().collect(true);
        for (elem_map::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
            gc::get_gc().unmark(elem->second);
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_value)
    {
        _test_map_value();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_map_value_mark
{
    int32_t mark_count = 0;

    class elem_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<int32_t, elem_object_ptr> > elem_map;

    elem_map _test_map_value_mark()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
            test.insert(std::make_pair(i, new_gc<elem_object>()));
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 10);
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_value_mark)
    {
        _test_map_value_mark();
        gc::get_gc().collect(true);
    }
}

namespace test_map_key_value
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<elem_object_ptr, elem_object_ptr> > elem_map;

    elem_map _test_map_key_value()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
            test.insert(std::make_pair(new_gc<elem_object>(), new_gc<elem_object>()));
        BOOST_CHECK_EQUAL(instance_count, 20);
        gc::get_gc().collect(true);
        for (elem_map::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
        {
            gc::get_gc().unmark(elem->first);
            gc::get_gc().unmark(elem->second);
        }
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key_value)
    {
        _test_map_key_value();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace _test_map_key_value_mark
{
    int32_t mark_count = 0;

    class elem_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<elem_object_ptr, elem_object_ptr> > elem_map;

    elem_map _test_map_key_value_mark()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
            test.insert(std::make_pair(new_gc<elem_object>(), new_gc<elem_object>()));
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 20);
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key_value_mark)
    {
        _test_map_key_value_mark();
        gc::get_gc().collect(true);
    }
}

namespace test_vector_non_gc
{
    int32_t instance_count = 0;

    class elem_object // non gc
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef vector_ptr< std::vector<elem_object*> > elem_vector;

    elem_vector _test_vector_non_gc(const std::vector<elem_object*>& store)
    {
        elem_vector test = new_vector<elem_vector::vector_type>(store.begin(), store.end());
        BOOST_CHECK_EQUAL(instance_count, 10);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_vector();
    }

    BOOST_AUTO_TEST_CASE(test_vector_non_gc)
    {
        std::vector<elem_object*> store;
        for (int32_t i = 0; i < 10; ++i)
            store.push_back(new elem_object);
        _test_vector_non_gc(store);
        for (int32_t i = 0; i < 10; ++i)
            delete store[i];
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_map_key_set
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef set_ptr< std::set<elem_object_ptr> > elem_set;
    typedef map_ptr< std::map<elem_object_ptr, elem_set> > elem_map;

    elem_map _test_map_key_set()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
        {
            elem_set testset = new_set<elem_set::set_type>();
            for (int32_t j = 0; j < 10; ++j)
                testset.insert(new_gc<elem_object>());
            test.insert(std::make_pair(new_gc<elem_object>(), testset));
        }
        BOOST_CHECK_EQUAL(instance_count, 110);
        for (elem_map::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
        {
            gc::get_gc().unmark(elem->first);
            for (elem_set::const_iterator testelem = elem->second->begin(), last = elem->second->end(); testelem != last; ++testelem)
                gc::get_gc().unmark(*testelem);
            gc::get_gc().unmark(elem->second);
        }
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key_set)
    {
        _test_map_key_set();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_map_key_set_mark
{
    int32_t mark_count = 0;

    class elem_object : public gc_object
    {
    public:
        virtual void mark_members(gc* gc) const
        {
            ++mark_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef set_ptr< std::set<elem_object_ptr> > elem_set;
    typedef map_ptr< std::map<elem_object_ptr, elem_set> > elem_map;

    elem_map _test_map_key_set_mark()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
        {
            elem_set testset = new_set<elem_set::set_type>();
            for (int32_t j = 0; j < 10; ++j)
                testset.insert(new_gc<elem_object>());
            test.insert(std::make_pair(new_gc<elem_object>(), testset));
        }
        BOOST_CHECK_EQUAL(mark_count, 0);
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(mark_count, 110);
        return elem_map();
    }

    BOOST_AUTO_TEST_CASE(test_map_key_set_mark)
    {
        _test_map_key_set_mark();
        gc::get_gc().collect(true);
    }
}

namespace test_large_vector
{
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            ++instance_count;
        }

        virtual ~elem_object()
        {
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef vector_ptr< std::vector<elem_object_ptr> > elem_vector;

    elem_vector _test_large_vector()
    {
        elem_vector test = new_vector<elem_vector::vector_type>();
        for (int32_t i = 0; i < 10000; ++i)
            test.push_back(new_gc<elem_object>());
        BOOST_CHECK_EQUAL(instance_count, 10000);
        gc::get_gc().collect(true);
        for (elem_vector::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
            gc::get_gc().unmark(*elem);
        gc::get_gc().unmark(test); // simulate out of scope
        return elem_vector();
    }

    BOOST_AUTO_TEST_CASE(test_large_vector)
    {
        _test_large_vector();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_thread_stop_single
{
    boost::mutex instance_mutex;
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            ++instance_count;
        }

        virtual ~elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;

    void worker_func()
    {
        elem_object_ptr test = new_gc<elem_object>();
        BOOST_CHECK_EQUAL(instance_count, 1);
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
    }

    BOOST_AUTO_TEST_CASE(test_thread_stop_single)
    {
        boost::thread worker_thread(worker_func);
        worker_thread.join();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_thread_stop_two
{
    boost::mutex instance_mutex;
    int32_t instance_count = 0;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            ++instance_count;
        }

        virtual ~elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;

    void worker_func()
    {
        elem_object_ptr test = new_gc<elem_object>();
        BOOST_CHECK_EQUAL(instance_count, 1);
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
    }

    BOOST_AUTO_TEST_CASE(test_thread_stop_two)
    {
        gc::get_gc(); // this will register a gc against the main thread
        boost::thread worker_thread(worker_func);
        worker_thread.join();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_thread_multiple
{
    boost::mutex instance_mutex;
    int32_t instance_count = 0;
    int32_t level_count = 0;

    class deep_object;
    typedef gc_ptr<deep_object> deep_object_ptr;

    class deep_object : public gc_object
    {
    public:
        deep_object()
        {
            {
                boost::mutex::scoped_lock lock(instance_mutex);
                ++instance_count;
            }
            if (++level_count < 100)
            {
                child1 = new_gc<deep_object>();
                child2 = new_gc<deep_object>();
                child3 = new_gc<deep_object>();
            }
        }

        virtual ~deep_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --instance_count;
        }

        deep_object_ptr child1;
        deep_object_ptr child2;
        deep_object_ptr child3;

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child1);
            gc->mark(child2);
            gc->mark(child3);
        }
    };

    void worker_func()
    {
        deep_object_ptr test = new_gc<deep_object>();
        BOOST_CHECK_NE(instance_count, 0);
        gc::get_gc().collect(true);
        gc::get_gc().unmark(test); // simulate out of scope
    }

    BOOST_AUTO_TEST_CASE(test_thread_multiple)
    {
        worker_func();
        std::vector<boost::thread*> threads;
        for (int32_t i = 0; i < 50; ++i)
            threads.push_back(new boost::thread(worker_func));
        for (int32_t i = 0; i < 50; ++i)
        {
            threads[i]->join();
            delete threads[i];
            threads[i] = NULL;
        }
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_thread_multiple_collection
{
    int32_t instance_count = 0;
    boost::mutex instance_mutex;

    class elem_object : public gc_object
    {
    public:
        elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            ++instance_count;
        }

        virtual ~elem_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --instance_count;
        }
    };

    typedef gc_ptr<elem_object> elem_object_ptr;
    typedef map_ptr< std::map<elem_object_ptr, elem_object_ptr> > elem_map;

    elem_map _test_thread_multiple_collection()
    {
        elem_map test = new_map<elem_map::map_type>();
        for (int32_t i = 0; i < 10; ++i)
            test.insert(std::make_pair(new_gc<elem_object>(), new_gc<elem_object>()));
        gc::get_gc().collect(true);
        for (elem_map::const_iterator elem = test.begin(), last = test.end(); elem != last; ++elem)
        {
            gc::get_gc().unmark(elem->first);
            gc::get_gc().unmark(elem->second);
        }
        gc::get_gc().unmark(test); // simulate out of scope
        return test;
    }

    void worker_func()
    {
        _test_thread_multiple_collection();
        gc::get_gc().collect(true);
    }

    BOOST_AUTO_TEST_CASE(test_thread_multiple_collection)
    {
        worker_func();
        std::vector<boost::thread*> threads;
        for (int32_t i = 0; i < 50; ++i)
            threads.push_back(new boost::thread(worker_func));
        for (int32_t i = 0; i < 50; ++i)
        {
            threads[i]->join();
            delete threads[i];
        }
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
    }
}

namespace test_thread_object_transfer
{
    boost::mutex instance_mutex;
    int32_t instance_count = 0;
    int32_t child_count = 0;

    class test_object : public gc_object
    {
    public:
        test_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            ++child_count;
        }

        virtual ~test_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --child_count;
        }
    };

    typedef gc_ptr<test_object> test_object_ptr;

    class member_test_object : public gc_object
    {
    public:
        member_test_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            ++instance_count;

        }

        virtual ~member_test_object()
        {
            boost::mutex::scoped_lock lock(instance_mutex);
            --instance_count;
        }

        test_object_ptr child1;
        test_object_ptr child2;

        void create_child1()
        {
            child1 = new_gc<test_object>();
        }

        void create_child2()
        {
            child2 = new_gc<test_object>();
        }

        virtual void mark_members(gc* gc) const
        {
            gc->mark(child1);
            gc->mark(child2);
        }
    };

    typedef gc_ptr<member_test_object> member_test_object_ptr;

    void thread_func(member_test_object_ptr test)
    {
        test->create_child2();
    }

    member_test_object_ptr _test_thread_object_transfer()
    {
        member_test_object_ptr test = new_gc<member_test_object>();
        BOOST_CHECK_NE(instance_count, 0);
        BOOST_CHECK_EQUAL(child_count, 0);
        test->create_child1();

        boost::thread test_thread(thread_func, test);
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        test_thread.join();

        gc::get_gc().collect(true);
        gc::get_gc().unmark(test->child1);
        gc::get_gc().unmark(test->child2);
        gc::get_gc().unmark(test); // simulate out of scope
        return member_test_object_ptr();
    }

    BOOST_AUTO_TEST_CASE(test_thread_object_transfer)
    {
        _test_thread_object_transfer();
        gc::get_gc().collect(true);
        BOOST_CHECK_EQUAL(instance_count, 0);
        BOOST_CHECK_EQUAL(child_count, 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()

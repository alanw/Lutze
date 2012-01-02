Lutze C++ Garbage Collector
===========================

Welcome to the Lu-Tze C++ Garbage Collector version **1.0.2**.

For Terry Pratchett fans, `Lu-Tze <http://en.wikipedia.org/wiki/History_Monks#Lu-Tze>`_
is a familiar figure, however for the uninitiated, the character is known for
being the only master of 'Déjà fu' and is generally referred to as the mystical
'sweeper'.


Rational
--------

As part of general experimentation for a separate project called `Lucene++ <https://github.com/luceneplusplus/LucenePlusPlus>`_
I put together this simple garbage collector. Right now, Lucene++ relies
heavily on boost::shared_ptr and this is fine for the most part, however there
are some areas where performance is a concern. Therefore, in order to keep
the existing code clean (baring in mind Lucene++ is a C++ port of Java code),
I decided to investigate alternative object lifetime mechanisms.

Also, it's worth making clear that the Lutze Garbage Collector is only intended
to control object lifetimes and is not a complete memory manager solution.


Features
--------

* Cross-platform code - tested on Windows, Mac and Linux.
* Thread safe and multi-thread compatible.
* Supports garbage collected objects and collections.
* Can be used with raw pointers or managed pointers that assist collections.
* Does not stop the (entire) world when performing collections.
* Destructors called when objects are destroyed.
* Works well with unmanaged objects.
* All code contained in header files - just #include to start using!


Rules
-----

Because there has to be some rules, right?

* All garbage collected (managed) objects must derive from gc::gc_object
* The mark_members() method must be overridden to mark all member gc_object
  pointers.
* Care must be taken to ensure destructors do not contain non-trivial code,
  since objects may be destroyed at any time and not necessarily from the
  same thread that they were created.
* Using garbage collected objects doesn't mean you can abandon thinking!


Usage
-----

All garbage collected objects must be derived from gc::gc_object::

    #include "gc.h"

    using namespace gc;

    class example_object : public gc_object
    {
    public:
        example_object(const std::string& test) : member1(test), member2(0)
        {
            // don't forget to initialize member2!
        }

        virtual ~example_object()
        {
            // destructor called when object is destroyed
        }

        virtual void mark_members(gc* gc) const
        {
            gc->mark(member2);
        }

    protected:
        std::string member1; // unmanaged member is ok
        example_object* member2; // managed member
    };

    ...

    // NOTE: get_gc() will return the garbage collector instance for this thread
    example_object* ex = new(get_gc()) example_object;

    ...

    get_gc().collect(); // perform garbage collection here

    // NOTE: it is your responsibility to call collect() if using raw pointers.

Alternatively, you can use the (preferred) method of managed pointers. These
are smart pointers that share similar characteristics to boost::shared_ptr, but
don't perform any reference counting or locking and are therefore completely
thread-safe. Their job is to simplify pointer initialization and automatically
collecting unreferenced objects without you having to think about things too
much.

The smart pointer syntax is similar to using raw pointers::

    #include "gc.h"
    #include "gc_ptr.h"

    using namespace gc;

    class example_object;
    typedef gc_ptr<example_object> example_object_ptr;

    class example_object : public gc_object
    {
    public:
        example_object(const std::string& test) : member1(test)
        {
            // member2 will be initialized for you
        }

        virtual ~example_object()
        {
            // destructor called when object is destroyed
        }

        virtual void mark_members(gc* gc) const
        {
            gc->mark(member2);
        }

    protected:
        std::string member1; // unmanaged member is ok
        example_object_ptr member2; // managed member
    };

    ...

    // NOTE: new_gc<>() will instantiate an object using the garbage collector
    // for this thread. It's usage is similar to boost::make_shared<>()
    example_object_ptr test = new_gc<example_object>();

    ...

    // NOTE: explicitly calling get_gc().collect() is not required since
    // collection is performed if necessary during calls to new_gc<>(). However
    // there is nothing stopping you from collecting periodically if necessary.

Statically allocated gc objects are performed slightly differently since their
lifetimes are managed differently::

    #include "gc.h"
    #include "gc_ptr.h"

    using namespace gc;

    example_object_ptr example_ptr = new_static_gc<example_object>();

    // or

    example_object* example = new(get_static_gc()) example_object;


Collections
-----------

Lutze also supports collections of managed objects, including:

* vectors
* sets
* maps
* lists

In order to also support additional collections, such as boost::unordered_set,
you supply the collection type itself when creating::

    #include "gc.h"
    #include "gc_ptr.h"
    #include "gc_container.h"

    using namespace gc;

    class example_key : public gc_object
    {
    public:
        example_key(const std::string& key) : key(key) {}
        std::string key;
    };

    class example_value : public gc_object
    {
    public:
        example_value(const std::string& value) : value(value) {}
        std::string value;
    };

    typedef gc_ptr<example_key> example_key_ptr;
    typedef gc_ptr<example_value> example_value_ptr;

    typedef map_container< std::map<example_key_ptr, example_value_ptr> > std_map;
    typedef set_container< boost::unordered_set<example_key_ptr> > boost_set;

    std_map example_map = new_map<std_map::map_type>();
    boost_set example_set = new_set<boost_set::set_type>();

You can use a collection instance just like you would for a normal std
collection::

    example_set.insert(new_gc<example_key>("hello"));
    example_map[new_gc<example_key>("hello")] = new_gc<example_value>("world");


Threads
-------

If you're using boost::thread, then things should just work (tm), however if
you're using native threads (pthreads, Windows threads, etc), then you will
need to call boost::on_thread_exit() when the native thread completes. This is
because there is no reliable cross-platform way of detecting thread completion.


How does it work?
-----------------

A single gc instance is maintained per thread that controls the lifetime of
objects registered to it. Objects are registered at the point of creation and
stored in a hash map, keyed by it's address.

The basic mechanism follows the familiar mark-sweep pattern, however one of the
main differences to other garbage collectors is that unreferenced objects are
first transfered to other gc instances (after recording a history of where the
object has been) in case ownership has transfered to another thread. Only when
an unreferenced object has visited all running gc's is it destroyed.

There are a few recognized problems with this approach, including the
possibility of a race condition when or if hundreds of threads are continually
created and destroyed. Care must be taken that this does not happen - it could
be argued that this would be a poor design decision anyway.

Another inherent problem is that transfered objects could queue up against gc's
that don't perform any new_gc<> calls. Unfortunately, there doesn't seem to be
any clean solution to this problem, and it is left to the developer to make
sure that any long running threads should occasionally call new_gc<> or manually
trigger collections by calling get_gc().collect() periodically.

As previously described, statically created managed objects should be created
using new_static_gc<> because they use a separate gc instance. Objects created
statically are destroyed when the application exits.


Build Instructions using CMake
------------------------------

Simply run CMake to generate the required Makefile or project and build the
unit test application gc_test.

Note: The Lutze garbage collector uses `Boost <http://www.boost.org>`_ in order
to provide cross-platform support for threads, plus some other useful utilities
such as boost::unordered_map.

For Windows users, `BoostPro <http://www.boostpro.com>`_ has some pre-compiled
packages that make using Boost libraries easier.


Acknowledgments
---------------

Parts of the stack-scanning were inspired by the `Tamarin <http://www-archive.mozilla.org/projects/tamarin>`_
project. Particular credit should go to:

* Tommy Reilly
* Edwin Smith
* Leon Sha


Roadmap
-------

* Add weak pointer support.
* Improve collection policy. Right now collection is only triggered by the
  frequency of object creations and/or the number of objects waiting to be
  transfered.
* Add support for incremental mark and sweep.
* Include some sort of performance testing metrics.
* Add gc collection statistics (times, frequency, queue sizes, etc)
* Perhaps introduce support for generations.
* Investigate ways to minimize problems or race conditions outlined above.
* Look at ways to eliminate the need for mark_members().

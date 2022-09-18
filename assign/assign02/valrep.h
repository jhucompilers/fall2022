#ifndef VALREP_H
#define VALREP_H

#include <cassert>
class Function;

// A "ValRep" (value representation) is a type used as
// a dynamically-allocated object serving as the representation
// of a Value. ValReps are reference counted, so many Values
// can point to the same ValRep.

enum ValRepKind {
  VALREP_FUNCTION,
  // other kinds of valreps (e.g., vector, string, etc.) could be added
};

class ValRep {
private:
  ValRepKind m_kind;
  int m_refcount;

  // copy constructor and assignment operator prohibited
  ValRep(const ValRep &);
  ValRep &operator=(const ValRep &);

public:
  ValRep(ValRepKind kind);
  virtual ~ValRep();

  ValRepKind get_kind() const { return m_kind; }

  // These member functions allow reference counting of objects
  // derived from ValRep.  add_ref() should be called when a
  // Value is set to point to a ValRep. remove_ref() should be
  // called when a Value no longer points to a ValRep.
  // If remove_ref() is called and the reference count
  // (as returned by get_num_refs()) becomes 0, the ValRep object
  // should be deleted (because there are no longer any Value
  // objects pointing to it.)
  void add_ref()           { ++m_refcount; }
  void remove_ref()        { assert(m_refcount > 0); --m_refcount; }
  int get_num_refs() const { return m_refcount; }

  // It's useful to have functions that return a pointer to
  // the actual derived type (e.g., Function). Obviously, the caller
  // should only do this after checking the ValRepKind value
  Function *as_function();
};

#endif

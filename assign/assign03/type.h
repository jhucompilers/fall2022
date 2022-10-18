#ifndef TYPE_H
#define TYPE_H

#include <memory>
#include <vector>
#include <string>

// Kinds of basic types:
// note that these can be signed or unsigned
// (except for void)
enum class BasicTypeKind {
  CHAR,
  SHORT,
  INT,
  LONG,
  VOID,
};

// Type qualifiers
enum class TypeQualifier {
  VOLATILE,
  CONST,
};

// forward declaration
class Member;

// Representation of a C data type.
// Type is a base class that may not be directly
// instantiated. It defines a "wide" interface of virtual member functions
// for the complete range of functionality offered by subclasses,
// and provides default implementations for most of them. Subclasses will
// override these as necessary/appropriate.
//
// IMPORTANT: Type objects should be accessed using std::shared_ptr.
// Types are essentially trees, and if a variable declaration
// has multiple declarators, the resulting types of the
// declared variables can share the common part of their
// representations.
class Type {
private:
  // value semantics not allowed
  Type(const Type &);
  Type& operator=(const Type &);

protected:
  Type();

public:
  virtual ~Type();

  // Some member functions for convenience
  bool is_integral() const { return is_basic() && get_basic_type_kind() != BasicTypeKind::VOID; }
  const Member *find_member(const std::string &name) const;

  // Note that Type provides default implementations of virtual
  // member functions that will be appropriate for most of the
  // subclasses. Each subclass should be able to just override the
  // member functions that are necessary to provide that
  // subclass's functionality.

  // equality: returns true IFF the other type represents
  // exactly the same type as this one
  virtual bool is_same(const Type *other) const = 0;

  // return a string containing a description of the type
  virtual std::string as_str() const = 0;

  // get unqualified type (strip off type qualifiers, if any)
  virtual const Type *get_unqualified_type() const;

  // subtype tests (safe to call on any Type object)
  virtual bool is_basic() const;
  virtual bool is_void() const;
  virtual bool is_struct() const;
  virtual bool is_pointer() const;
  virtual bool is_array() const;
  virtual bool is_function() const;

  // qualifier tests (safe to call on any Type object)
  virtual bool is_volatile() const;
  virtual bool is_const() const;

  // BasicType-only member functions
  virtual BasicTypeKind get_basic_type_kind() const;
  virtual bool is_signed() const;

  // Functions common to StructType and FunctionType.
  // A "member" is a parameter (for FunctionTypes) or a field (for StructTypes).
  virtual void add_member(const Member &member);
  virtual unsigned get_num_members() const;
  virtual const Member &get_member(unsigned index) const;

  // FunctionTypes, PointerTypes, and ArrayTypes all have
  // a base type.
  virtual std::shared_ptr<Type> get_base_type() const;

  // FunctionType-only member functions
  // (there aren't any, at least for now...)

  // PointerType-only member functions
  // (there actually aren't any, at least for now...)

  // ArrayType-only member functions
  virtual unsigned get_array_size() const;
};

// Common base class for QualifiedType, FunctionType, PointerType, and
// ArrayType
class HasBaseType : virtual public Type {
private:
  std::shared_ptr<Type> m_base_type;

  // value semantics are not allowed
  HasBaseType(const HasBaseType &);
  HasBaseType &operator=(const HasBaseType &);

public:
  HasBaseType(const std::shared_ptr<Type> &base_type);
  virtual ~HasBaseType();

  virtual std::shared_ptr<Type> get_base_type() const;
};

// A parameter of a function or a field of a struct type.
class Member {
private:
  const std::string m_name;
  std::shared_ptr<Type> m_type;
  // Note: you could add additional information here, such as an
  // offset value (for struct fields), etc.

public:
  Member(const std::string &name, const std::shared_ptr<Type> &type);
  ~Member();

  const std::string &get_name() const;
  std::shared_ptr<Type> get_type() const;
};

// Common base class for StructType and FunctionType,
// which both have "members" (fields or parameters)
class HasMembers : virtual public Type {
private:
  std::vector<Member> m_members;

  // value semantics are disallowed
  HasMembers(const HasMembers &);
  HasMembers &operator=(const HasMembers &);

public:
  HasMembers();
  virtual ~HasMembers();

  virtual std::string as_str() const;
  virtual void add_member(const Member &member);
  virtual unsigned get_num_members() const;
  virtual const Member &get_member(unsigned index) const;
};

// A QualifiedType modifies a "delegate" type with a TypeQualifier
// (const or volatile). Most of the member functions a just passed
// on to the delegate.
class QualifiedType : public HasBaseType {
private:
  TypeQualifier m_type_qualifier;

  // value semantics are not allowed
  QualifiedType(const QualifiedType &);
  QualifiedType &operator=(const QualifiedType &);

public:
  QualifiedType(const std::shared_ptr<Type> &delegate, TypeQualifier type_qualifier);
  virtual ~QualifiedType();

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual const Type *get_unqualified_type() const;
  virtual bool is_basic() const;
  virtual bool is_void() const;
  virtual bool is_struct() const;
  virtual bool is_pointer() const;
  virtual bool is_array() const;
  virtual bool is_function() const;
  virtual bool is_volatile() const;
  virtual bool is_const() const;
  virtual BasicTypeKind get_basic_type_kind() const;
  virtual bool is_signed() const;
  virtual void add_member(const Member &member);
  virtual unsigned get_num_members() const;
  virtual const Member &get_member(unsigned index) const;
  virtual unsigned get_array_size() const;
};

class BasicType : public Type {
private:
  BasicTypeKind m_kind;
  bool m_is_signed;

  // value semantics not allowed
  BasicType(const BasicType &);
  BasicType &operator=(const BasicType &);

public:
  BasicType(BasicTypeKind kind, bool is_signed);
  virtual ~BasicType();

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual bool is_basic() const;
  virtual bool is_void() const;
  virtual BasicTypeKind get_basic_type_kind() const;
  virtual bool is_signed() const;
};

class StructType : public HasMembers {
private:
  std::string m_name;

  // value semantics not allowed
  StructType(const StructType &);
  StructType &operator=(const StructType &);

public:
  StructType(const std::string &name);
  virtual ~StructType();

  std::string get_name() const { return m_name; }

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual bool is_struct() const;
};

class FunctionType : public HasBaseType, public HasMembers {
private:
  // value semantics not allowed
  FunctionType(const FunctionType &);
  FunctionType &operator=(const FunctionType &);

public:
  FunctionType(const std::shared_ptr<Type> &base_type);
  virtual ~FunctionType();

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual bool is_function() const;
};

class PointerType : public HasBaseType {
private:
  // value semantics not allowed
  PointerType(const PointerType &);
  PointerType &operator=(const PointerType &);

public:
  PointerType(const std::shared_ptr<Type> &base_type);
  virtual ~PointerType();

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual bool is_pointer() const;
};

class ArrayType : public HasBaseType {
private:
  unsigned m_size;

  // value semantics not allowed
  ArrayType(const ArrayType &);
  ArrayType &operator=(const ArrayType &);

public:
  ArrayType(const std::shared_ptr<Type> &base_type, unsigned size);
  virtual ~ArrayType();

  virtual bool is_same(const Type *other) const;
  virtual std::string as_str() const;
  virtual bool is_array() const;
  virtual unsigned get_array_size() const;
};

#endif // TYPE_H

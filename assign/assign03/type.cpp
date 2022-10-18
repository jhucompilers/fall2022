#include <cassert>
#include "exceptions.h"
#include "type.h"

////////////////////////////////////////////////////////////////////////
// Type implementation
////////////////////////////////////////////////////////////////////////

Type::Type() {
}

Type::~Type() {
}

const Member *Type::find_member(const std::string &name) const {
  for (unsigned i = 0; i < get_num_members(); ++i) {
    const Member &member = get_member(i);
    if (member.get_name() == name)
      return &member;
  }
  return nullptr;
}

const Type *Type::get_unqualified_type() const {
  // only QualifiedType will need to override this member function
  return this;
}

bool Type::is_basic() const {
  return false;
}

bool Type::is_void() const {
  return false;
}

bool Type::is_struct() const {
  return false;
}

bool Type::is_pointer() const {
  return false;
}

bool Type::is_array() const {
  return false;
}

bool Type::is_function() const {
  return false;
}

bool Type::is_volatile() const {
  return false;
}

bool Type::is_const() const {
  return false;
}

BasicTypeKind Type::get_basic_type_kind() const {
  RuntimeError::raise("not a BasicType");
}

bool Type::is_signed() const {
  RuntimeError::raise("not a BasicType");
}

void Type::add_member(const Member &member) {
  RuntimeError::raise("type does not have members");
}

unsigned Type::get_num_members() const {
  RuntimeError::raise("type does not have members");
}

const Member &Type::get_member(unsigned index) const {
  RuntimeError::raise("type does not have members");
}

std::shared_ptr<Type> Type::get_base_type() const {
  RuntimeError::raise("type does not have a base type");
}

unsigned Type::get_array_size() const {
  RuntimeError::raise("not an ArrayType");
}

////////////////////////////////////////////////////////////////////////
// HasBaseType implementation
////////////////////////////////////////////////////////////////////////

HasBaseType::HasBaseType(const std::shared_ptr<Type> &base_type)
  : m_base_type(base_type) {
}

HasBaseType::~HasBaseType() {
}

std::shared_ptr<Type> HasBaseType::get_base_type() const {
  return m_base_type;
}

////////////////////////////////////////////////////////////////////////
// Member implementation
////////////////////////////////////////////////////////////////////////

Member::Member(const std::string &name, const std::shared_ptr<Type> &type)
  : m_name(name)
  , m_type(type) {
}

Member::~Member() {
}

const std::string &Member::get_name() const {
  return m_name;
}

std::shared_ptr<Type> Member::get_type() const {
  return m_type;
}

////////////////////////////////////////////////////////////////////////
// HasMembers implementation
////////////////////////////////////////////////////////////////////////

HasMembers::HasMembers() {
}

HasMembers::~HasMembers() {
}

std::string HasMembers::as_str() const {
  std::string s;

  for (unsigned i = 0; i < get_num_members(); ++i) {
    if (i > 0)
      s += ", ";
    const Member &member = get_member(i);
/*
    s += member.get_name();
    s += " : ";
*/

    // Special case: recursive struct types such as linked list nodes, trees, etc.
    // will lead to an infinite recursion if we try to recursively
    // stringify the complete struct type. This is not a complete workaround,
    // but it handles simple cases like "struct Node *next;".

    bool member_is_recursive = false;
    if (member.get_type()->is_pointer()) {
      std::shared_ptr<Type> base_type = member.get_type()->get_base_type();
      if (base_type.get() == this) {
        member_is_recursive = true;
        const StructType *struct_type = dynamic_cast<const StructType *>(this);
        assert(struct_type != nullptr);
        s += "pointer to struct " + struct_type->get_name();
      }
    }

    if (!member_is_recursive)
      s += member.get_type()->as_str();
  }

  return s;
}

void HasMembers::add_member(const Member &member) {
  m_members.push_back(member);
}

unsigned HasMembers::get_num_members() const {
  return unsigned(m_members.size());
}

const Member &HasMembers::get_member(unsigned index) const {
  assert(index < m_members.size());
  return m_members[index];
}

////////////////////////////////////////////////////////////////////////
// QualifiedType implementation
////////////////////////////////////////////////////////////////////////

QualifiedType::QualifiedType(const std::shared_ptr<Type> &delegate, TypeQualifier type_qualifier)
  : HasBaseType(delegate)
  , m_type_qualifier(type_qualifier) {
}

QualifiedType::~QualifiedType() {
}

bool QualifiedType::is_same(const Type *other) const {
  // see whether type qualifiers differ, if they do, return false
  if (is_const() != other->is_const())
    return false;
  if (is_volatile() != other->is_volatile())
    return false;

  // compare unqualified types
  return get_unqualified_type()->is_same(other->get_unqualified_type());
}

std::string QualifiedType::as_str() const {
  std::string s;
  assert(is_const() || is_volatile());
  s += is_const() ? "const " : "volatile ";
  s += get_base_type()->as_str();
  return s;
}

const Type *QualifiedType::get_unqualified_type() const {
  return get_base_type()->get_unqualified_type();
}

bool QualifiedType::is_basic() const {
  return get_base_type()->is_basic();
}

bool QualifiedType::is_void() const {
  return get_base_type()->is_void();
}

bool QualifiedType::is_struct() const {
  return get_base_type()->is_struct();
}

bool QualifiedType::is_pointer() const {
  return get_base_type()->is_pointer();
}

bool QualifiedType::is_array() const {
  return get_base_type()->is_array();
}

bool QualifiedType::is_function() const {
  return get_base_type()->is_function();
}

bool QualifiedType::is_volatile() const {
  return m_type_qualifier == TypeQualifier::VOLATILE;
}

bool QualifiedType::is_const() const {
  return m_type_qualifier == TypeQualifier::CONST;
}

BasicTypeKind QualifiedType::get_basic_type_kind() const {
  return get_base_type()->get_basic_type_kind();
}

bool QualifiedType::is_signed() const {
  return get_base_type()->is_signed();
}

void QualifiedType::add_member(const Member &member) {
  get_base_type()->add_member(member);
}

unsigned QualifiedType::get_num_members() const {
  return get_base_type()->get_num_members();
}

const Member &QualifiedType::get_member(unsigned index) const {
  return get_base_type()->get_member(index);
}

unsigned QualifiedType::get_array_size() const {
  return get_base_type()->get_array_size();
}

////////////////////////////////////////////////////////////////////////
// BasicType implementation
////////////////////////////////////////////////////////////////////////

BasicType::BasicType(BasicTypeKind kind, bool is_signed)
  : m_kind(kind)
  , m_is_signed(is_signed) {
}

BasicType::~BasicType() {
}

bool BasicType::is_same(const Type *other) const {
  if (!other->is_basic())
    return false;
  return m_kind == other->get_basic_type_kind()
      && m_is_signed == other->is_signed();
}

std::string BasicType::as_str() const {
  std::string s;

  if (!is_signed())
    s += "unsigned ";
  switch (m_kind) {
  case BasicTypeKind::CHAR:
    s += "char"; break;
  case BasicTypeKind::SHORT:
    s += "short"; break;
  case BasicTypeKind::INT:
    s += "int"; break;
  case BasicTypeKind::LONG:
    s += "long"; break;
  case BasicTypeKind::VOID:
    s += "void"; break;
  default:
    assert(false);
  }

  return s;
}

bool BasicType::is_basic() const {
  return true;
}

bool BasicType::is_void() const {
  return m_kind == BasicTypeKind::VOID;
}

BasicTypeKind BasicType::get_basic_type_kind() const {
  return m_kind;
}

bool BasicType::is_signed() const {
  return m_is_signed;
}

////////////////////////////////////////////////////////////////////////
// StructType implementation
////////////////////////////////////////////////////////////////////////

StructType::StructType(const std::string &name)
  : m_name(name) {
}

StructType::~StructType() {
}

bool StructType::is_same(const Type *other) const {
  // Trivial base case that avoids infinite recursion for recursive types
  if (this == other) return true;

  // In general, it should not be possible for two struct types
  // with the same name to exist in the same translation unit.
  // So, comparing names *should* be sufficient to determine
  // whether these are the same type.

  if (!other->is_struct())
    return false;


  const StructType *other_st = dynamic_cast<const StructType *>(other);
  if (m_name != other_st->m_name)
    return false;

#ifndef NDEBUG
  // checking structure equality, just to be sure

  if (get_num_members() != other->get_num_members())
    RuntimeError::raise("struct types with same name but different numbers of members");
  for (unsigned i = 0; i < get_num_members(); ++i) {
    const Member &left = get_member(i);
    const Member &right = other->get_member(i);

    if (left.get_name() != right.get_name())
      RuntimeError::raise("struct types with same name but different member name(s)");
    if (!left.get_type()->is_same(right.get_type().get()))
      RuntimeError::raise("struct types with same name but different member type(s)");
  }
#endif

  return true;
}

std::string StructType::as_str() const {
  std::string s;

  s += "struct ";
  s += m_name;
  s += " {";
  s += HasMembers::as_str();
  s += "}";

  return s;
}

bool StructType::is_struct() const {
  return true;
}

////////////////////////////////////////////////////////////////////////
// FunctionType implementation
////////////////////////////////////////////////////////////////////////

FunctionType::FunctionType(const std::shared_ptr<Type> &base_type)
  : HasBaseType(base_type) {
}

FunctionType::~FunctionType() {
}

bool FunctionType::is_same(const Type *other) const {
  if (!other->is_function())
    return false;

  // see if return types are the same
  if (!get_base_type()->is_same(other->get_base_type().get()))
    return false;

  // see if numbers of parameters are the same
  if (get_num_members() != other->get_num_members())
    return false;

  // see if parameter types are the same
  for (unsigned i = 0; i < get_num_members(); ++i) {
    if (!get_member(i).get_type()->is_same(other->get_member(i).get_type().get()))
      return false;
  }

  return true;
}

std::string FunctionType::as_str() const {
  std::string s;

  s += "function (";
  s += HasMembers::as_str();
  s += ") returning ";
  s += get_base_type()->as_str();

  return s;
}

bool FunctionType::is_function() const {
  return true;
}

////////////////////////////////////////////////////////////////////////
// PointerType implementation
////////////////////////////////////////////////////////////////////////

PointerType::PointerType(const std::shared_ptr<Type> &base_type)
  : HasBaseType(base_type) {
}

PointerType::~PointerType() {
}

bool PointerType::is_same(const Type *other) const {
  if (!other->is_pointer())
    return false;

  return get_base_type()->is_same(other->get_base_type().get());
}

std::string PointerType::as_str() const {
  std::string s;

  s += "pointer to ";
  s += get_base_type()->as_str();

  return s;
}

bool PointerType::is_pointer() const {
  return true;
}

////////////////////////////////////////////////////////////////////////
// ArrayType implementation
////////////////////////////////////////////////////////////////////////

ArrayType::ArrayType(const std::shared_ptr<Type> &base_type, unsigned size)
  : HasBaseType(base_type)
  , m_size(size) {
}

ArrayType::~ArrayType() {
}

bool ArrayType::is_same(const Type *other) const {
  // Note: the only reason comparison of ArrayTypes might be useful
  // is for comparing pointers to arrays. In theory these
  // could arise if a function has a parameter whose declared type
  // is a multidimensional array.

  if (!other->is_array())
    return false;

  const ArrayType *other_at = dynamic_cast<const ArrayType *>(other);
  return m_size == other_at->m_size
      && get_base_type()->is_same(other->get_base_type().get());
}

std::string ArrayType::as_str() const {
  std::string s;

  s += "array of ";
  s += std::to_string(m_size);
  s += " x ";
  s += get_base_type()->as_str();

  return s;
}

bool ArrayType::is_array() const {
  return true;
}

unsigned ArrayType::get_array_size() const {
  return m_size;
}

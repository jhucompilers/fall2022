#include "function.h"

Function::Function(const std::string &name, const std::vector<std::string> &params, Environment *parent_env, Node *body)
  : ValRep(VALREP_FUNCTION)
  , m_name(name)
  , m_params(params)
  , m_parent_env(parent_env)
  , m_body(body) {
}

Function::~Function() {
}

// TODO: implement member functions

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <map>

namespace AST {

struct Node {
  virtual ~Node() {}
};

using Members  = std::vector<std::pair<std::string, Node*>>;
using Elements = std::vector<Node*>;

struct ObjectNode : Node {
  Members members;
  ObjectNode() {}
  ObjectNode(Members m) : members(std::move(m)) {}
};

struct ArrayNode : Node {
  Elements elements;
  ArrayNode() {}
  ArrayNode(Elements e) : elements(std::move(e)) {}
};

struct StringNode : Node {
  std::string val;
  StringNode(const char* s): val(s) {}
};

struct NumberNode : Node {
  double val;
  NumberNode(double v): val(v) {}
};

struct BoolNode : Node {
  bool val;
  BoolNode(bool v): val(v) {}
};

struct NullNode : Node {};

extern Node* root;

}  // namespace AST

#endif

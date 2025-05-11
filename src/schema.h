#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"
#include <string>
#include <vector>
#include <map>

struct Column { std::string name; };
struct Table {
  std::string name;
  std::vector<Column> cols;
  int next_id = 1;
  std::vector<std::vector<std::string>> rows;
};

class SchemaBuilder {
public:
  SchemaBuilder(AST::Node* root, const std::string& outdir);
  void build();
private:
  AST::Node* root_;
  std::string outdir_;
  std::map<std::string, Table> tables_;
  void walk(AST::Node* node, const std::string& parent, int parent_id);
  void flush_csvs();
};

#endif

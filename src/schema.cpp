#include "schema.h"
#include "csv_writer.h"
#include <cmath>        // for std::fabs
#include <algorithm>      // for std::find_if
#include <sstream>
#include <filesystem>

using namespace AST;
namespace fs = std::filesystem;

SchemaBuilder::SchemaBuilder(Node* root, const std::string& outdir)
  : root_(root), outdir_(outdir) {
  fs::create_directories(outdir_);
}

void SchemaBuilder::build() {
  walk(root_, "", 0);
  flush_csvs();
}

void SchemaBuilder::walk(Node* node, const std::string& parent, int parent_id) {
  if (auto obj = dynamic_cast<ObjectNode*>(node)) {
    // table name
    std::string tbl = parent.empty() ? "root" : parent;
    auto& T = tables_[tbl];
    if (T.name.empty()) {
      T.name = tbl;
      T.cols.push_back({"id"});
    }
    int myid = T.next_id++;
    std::vector<std::string> row(T.cols.size(), "");
    row[0] = std::to_string(myid);

    // process members
    for (auto& [k, v] : obj->members) {
      // find or add column
      auto it = std::find_if(
        T.cols.begin(), T.cols.end(),
        [&](auto& c){ return c.name == k; }
      );

      int idx;
      if (it == T.cols.end()) {
        T.cols.push_back({k});
        idx = (int)T.cols.size() - 1;
        row.resize(T.cols.size());
      } else {
        idx = (int)std::distance(T.cols.begin(), it);
      }

      // scalar?
      if (auto s = dynamic_cast<StringNode*>(v)) {
        row[idx] = s->val;
      }
      else if (auto n = dynamic_cast<NumberNode*>(v)) {
        double d = n->val;
        long long i = (long long) d;
        if (std::fabs(d - i) < 1e-9)          // effectively an integer
          row[idx] = std::to_string(i);
        else
          row[idx] = std::to_string(d);
      }
      else if (auto b = dynamic_cast<BoolNode*>(v)) {
        row[idx] = (b->val ? "true" : "false");
      }
      else if (dynamic_cast<NullNode*>(v)) {
        row[idx] = "";
      }
      else {
        // nested object or array => recurse
        walk(v, k, myid);
      }
    }

    T.rows.push_back(row);
  }
  else if (auto arr = dynamic_cast<ArrayNode*>(node)) {
    int seq = 0;
    for (auto* elem : arr->elements) {
      // array of objects => child table
      if (dynamic_cast<ObjectNode*>(elem)) {
        walk(elem, parent, parent_id);
      }
      else {
        // scalar array => junction table parent_val
        std::string tbl = parent + "_val";
        auto& T = tables_[tbl];
        if (T.name.empty()) {
          T.name = tbl;
          T.cols = {{parent + "_id"}, {"index"}, {"value"}};
        }
        T.rows.push_back({
          std::to_string(parent_id),
          std::to_string(seq),
          dynamic_cast<StringNode*>(elem)  ? dynamic_cast<StringNode*>(elem)->val
          : dynamic_cast<NumberNode*>(elem) ? std::to_string(dynamic_cast<NumberNode*>(elem)->val)
          : std::string()
        });
      }
      ++seq;
    }
  }
}

void SchemaBuilder::flush_csvs() {
  for (auto& [k, T] : tables_) {
    CSVWriter writer(outdir_ + "/" + T.name + ".csv");
    writer.write_header(T.cols);
    for (auto& r : T.rows) {
      writer.write_row(r);
    }
  }
}

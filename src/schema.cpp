#include "schema.h"
#include "csv_writer.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <filesystem>

using namespace AST;
namespace fs = std::filesystem;

SchemaBuilder::SchemaBuilder(Node* root, const std::string& outdir)
    : root_(root), outdir_(outdir) {
    fs::create_directories(outdir_);
}

void SchemaBuilder::build() {
    walk(root_, "root", 0);
    flush_csvs();
}

void SchemaBuilder::walk(Node* node, const std::string& parent, int parent_id) {
    if (auto obj = dynamic_cast<ObjectNode*>(node)) {
        std::string tbl = parent.empty() ? "root" : parent;
        auto& T = tables_[tbl];
        if (T.name.empty()) {
            T.name = tbl;
            T.cols.push_back({"id"});
        }
        int myid = T.next_id++;
        std::vector<std::string> row(T.cols.size(), "");
        row[0] = std::to_string(myid);

        for (auto& [k, v] : obj->members) {
            auto it = std::find_if(
                T.cols.begin(), T.cols.end(),
                [&](auto& c) { return c.name == k; }
            );

            int idx;
            if (it == T.cols.end()) {
                T.cols.push_back({k});
                idx = T.cols.size() - 1;
                row.resize(T.cols.size());
            } else {
                idx = std::distance(T.cols.begin(), it);
            }

            if (auto s = dynamic_cast<StringNode*>(v)) {
                row[idx] = s->val;
            } else if (auto n = dynamic_cast<NumberNode*>(v)) {
                double d = n->val;
                long long i = (long long)d;
                if (std::fabs(d - i) < 1e-9) {
                    row[idx] = std::to_string(i);
                } else {
                    row[idx] = std::to_string(d);
                }
            } else if (auto b = dynamic_cast<BoolNode*>(v)) {
                row[idx] = (b->val ? "true" : "false");
            } else if (dynamic_cast<NullNode*>(v)) {
                row[idx] = "";
            } else {
                walk(v, k, myid);
            }
        }

        T.rows.push_back(row);
    } else if (auto arr = dynamic_cast<ArrayNode*>(node)) {
        int seq = 0;
        for (auto* elem : arr->elements) {
            if (auto obj = dynamic_cast<ObjectNode*>(elem)) {
                std::string tbl = parent;
                auto& T = tables_[tbl];
                if (T.name.empty()) {
                    T.name = tbl;
                    T.cols = {{"order_id"}, {"seq"}};
                }

                std::vector<std::string> row(T.cols.size(), "");
                row[0] = std::to_string(parent_id); // Foreign key
                row[1] = std::to_string(seq);       // Sequence number

                for (auto& [k, v] : obj->members) {
                    // Add column if not exists
                    auto it = std::find_if(
                        T.cols.begin(), T.cols.end(),
                        [&](auto& c) { return c.name == k; }
                    );
                    int idx;
                    if (it == T.cols.end()) {
                        T.cols.push_back({k});
                        idx = T.cols.size() - 1;
                        row.resize(T.cols.size());
                    } else {
                        idx = std::distance(T.cols.begin(), it);
                    }

                    // Populate scalar values
                    if (auto s = dynamic_cast<StringNode*>(v)) {
                        row[idx] = s->val;
                    } else if (auto n = dynamic_cast<NumberNode*>(v)) {
                        double d = n->val;
                        long long i = (long long)d;
                        if (std::fabs(d - i) < 1e-9) {
                            row[idx] = std::to_string(i);
                        } else {
                            row[idx] = std::to_string(d);
                        }
                    } else if (auto b = dynamic_cast<BoolNode*>(v)) {
                        row[idx] = (b->val ? "true" : "false");
                    } else if (dynamic_cast<NullNode*>(v)) {
                        row[idx] = "";
                    }
                }

                T.rows.push_back(row);
            } else {
                // Scalar array: create junction table
                std::string tbl = parent + "_val";
                auto& T = tables_[tbl];
                if (T.name.empty()) {
                    T.name = tbl;
                    T.cols = {{parent + "_id"}, {"index"}, {"value"}};
                }
                T.rows.push_back({
                    std::to_string(parent_id),
                    std::to_string(seq),
                    dynamic_cast<StringNode*>(elem) ? dynamic_cast<StringNode*>(elem)->val
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

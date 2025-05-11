#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <string>
#include <vector>
#include "schema.h"     // brings in the full definition of Column

class CSVWriter {
public:
  CSVWriter(const std::string& path);
  ~CSVWriter();
  void write_header(const std::vector<Column>& cols);
  void write_row(const std::vector<std::string>& row);
private:
  FILE* f_;
};

#endif // CSV_WRITER_H

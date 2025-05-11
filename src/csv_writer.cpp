#include "csv_writer.h"
#include <cstdio>
#include <cstring>

CSVWriter::CSVWriter(const std::string& path) {
  f_ = fopen(path.c_str(), "w");
  if(!f_) { perror("fopen"); exit(1); }
}

CSVWriter::~CSVWriter() {
  if(f_) fclose(f_);
}

void CSVWriter::write_header(const std::vector<Column>& cols) {
  bool first = true;
  for(auto& c: cols){
    if(!first) fputs(",", f_);
    fputs(c.name.c_str(), f_);
    first = false;
  }
  fputs("\n", f_);
}

void CSVWriter::write_row(const std::vector<std::string>& row) {
  bool first = true;
  for(auto& cell: row){
    if(!first) fputs(",", f_);
    bool needq = cell.find_first_of(",\"")!=std::string::npos;
    if(needq) fputc('"', f_);
    for(char ch: cell){
      if(ch=='"') fputs("\"\"", f_);
      else fputc(ch, f_);
    }
    if(needq) fputc('"', f_);
    first = false;
  }
  fputs("\n", f_);
}

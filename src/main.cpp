#include <iostream>
#include <cstring>
#include "ast.h"
#include "schema.h"


// ./json2relcsv ../data/test2.json --out-dir ../output
// ./json2relcsv ../data/test1.json --print-ast --out-dir ../output

/*

make clean
make
./json2relcsv ../data/test3.json --out-dir ../output

*/



extern int yyparse();
extern FILE* yyin;

void usage(){
  std::cerr<<"Usage: json2relcsv <input.json> [--print-ast] [--out-dir DIR]\n";
  exit(1);
}

int main(int argc, char** argv){
  bool print_ast = false;
  std::string indir, outdir=".";
  if(argc<2) usage();
  indir = argv[1];
  for(int i=2;i<argc;++i){
    if(strcmp(argv[i],"--print-ast")==0) print_ast=true;
    else if(strcmp(argv[i],"--out-dir")==0 && i+1<argc) outdir=argv[++i];
    else usage();
  }
  yyin = fopen(indir.c_str(),"r");
  if(!yyin){ perror("fopen"); return 1; }

  if(yyparse()!=0){
    std::cerr<<"Parse error, aborting.\n";
    return 1;
  }
  if(print_ast){
    // simple AST printer
    // (you can expand this as needed)
    std::cout<<"[AST root at "<<AST::root<<"]\n";
  }
  SchemaBuilder sb(AST::root, outdir);
  sb.build();
  return 0;
}

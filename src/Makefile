CXX = g++
CXXFLAGS = -std=c++17 -Wall
FLEX = flex
BISON = bison

SRC = scanner.l parser.y ast.cpp schema.cpp csv_writer.cpp main.cpp
OBJ = ast.o schema.o csv_writer.o main.o

all: json2relcsv

parser.tab.cpp parser.tab.h: parser.y ast.h
	$(BISON) -d --defines=parser.tab.h --output=parser.tab.cpp parser.y

lex.yy.cpp: scanner.l parser.tab.h
	$(FLEX) --outfile=lex.yy.cpp scanner.l

json2relcsv: parser.tab.cpp lex.yy.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ parser.tab.cpp lex.yy.cpp $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f json2relcsv *.o parser.tab.* lex.yy.*

.PHONY: all clean

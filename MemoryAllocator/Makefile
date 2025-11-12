CXX = g++
CXXFLAGS = -fprofile-arcs -ftest-coverage -O0 -g
LDFLAGS = --coverage
TARGET = MemoryAllocator 
SOURCES = allocatortest.cpp

all: $(TARGET)
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
clean:
	rm -f $(TARGET) *.gcno *.gcda *.info *.html
	
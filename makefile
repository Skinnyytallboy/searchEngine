CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address -o2 -I.

SOURCES_DIR = .
BUILD_DIR = .

TARGET = searchEngine
SOURCES = $(SOURCES_DIR)/bscs23134_final_project.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET) $(ARGS)

CXX = clang++
CXXFLAGS += -std=c++17
CXXFLAGS += `pkg-config --cflags x11 libglog`
CXXFLAGS ?= -Wall -g
LDFLAGS += `pkg-config --libs x11 libglog`
LINT = clang-tidy
LINTFLAGS = -header-filter=".*.hpp" -checks="cppcoreguidelines-*" -- $(CXXFLAGS)

.PHONY: \
	all \
	clean \
	format \
	lint \
	test \

HEADERS = \
	window_manager.hpp
SOURCES = \
	window_manager.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LINTS = $(SOURCES:.cpp=.l)

all: vif

clean:
	rm -f vif $(OBJECTS)

format: $(HEADERS) $(SOURCES)
	clang-format -style=file -i $^

lint: $(LINTS) $(HEADERS)

test: vif
	DISPLAY=:615.0 ./vif

%.l: %.cpp
	$(LINT)  $< $(LINTFLAGS)

vif: $(HEADERS) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

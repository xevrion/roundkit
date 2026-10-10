CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude

TESTS := host/test_gesture host/test_font

.PHONY: test clean

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

host/test_gesture: host/test_gesture.cpp include/rk/gesture.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

host/test_font: host/test_font.cpp src/font.cpp include/rk/font.hpp include/rk/surface.hpp
	$(CXX) $(CXXFLAGS) host/test_font.cpp src/font.cpp -o $@

clean:
	rm -f $(TESTS)

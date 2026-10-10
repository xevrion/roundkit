CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude

.PHONY: test clean

test: host/test_gesture
	./host/test_gesture

host/test_gesture: host/test_gesture.cpp include/rk/gesture.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f host/test_gesture

CFLAGS=-g -O0
GOOGLETEST_LFLAGS=-L../googletest/out/lib
GOOGLETEST_LIBS=-lgtest -lgtest_main

%.o:%.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

hrml: hrml.o hrml.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^

hrml_tests: hrml_tests.o hrml.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(GOOGLETEST_LFLAGS) -o $@ hrml_tests.o $(GOOGLETEST_LIBS)

exceptional_server: exceptional_server.o
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^

lru_cache: lru_cache.o
	$(CXX) $(CFLAGS) $(CPPFLAGS)  $(GOOGLETEST_LFLAGS) -o $@ $^ $(GOOGLETEST_LIBS)

clean:
	$(RM) *.o
	$(RM) hrml
	$(RM) hrml_tests
	$(RM) exceptional_server
	$(RM) lru_cache



%.o:%.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

hrml: hrml.o hrml.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^

hrml_tests: hrml_tests.o hrml.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -L../googletest/out/lib -o $@ hrml_tests.o -lgtest -lgtest_main

clean:
	$(RM) *.o
	$(RM) hrml
	$(RM) hrml_tests

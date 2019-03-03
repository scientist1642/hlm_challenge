all: main_wread.cpp
	#g++ -O3 -pthread  -o bin/anagram  main_wmmap.cpp
	g++ -O3 -pthread  -o bin/app  main_wread.cpp

clean: 
	$(RM) bin/app

CPPFLAGS=-Wall -Werror -Wextra -Wfatal-errors
assignment=chat

$(assignment): $(assignment).cc
	g++ -I $(CPPFLAGS) chat.cc -o chat

tar:
	tar -cv $(MAKEFILE_LIST) *.cc README.txt >$(assignment).tar

clean:
	rm -f $(assignment) $(assignment).tar *.o *.gch

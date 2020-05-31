table: logic_table.cpp
	gcc logic_table.cpp -o logic_table -lstdc++
table_exec:
	./logic_table
list: table_management.cpp
	gcc table_management.cpp -o table_management -lstdc++ -lm
list_exec:
	./table_management
file:
	gcc lib/file_handler.cpp -o file_handler -lstdc++ 
file_exec:
	./file_handler ./table_4d.txt
main: main.cpp
	gcc main.cpp -o main -lstdc++
exec:
	./main
clean:
	rm -f *.o logic_table table_management main
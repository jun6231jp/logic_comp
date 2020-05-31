table: logic_table.cpp
	gcc logic_table.cpp -o logic_table -lstdc++
table_exec:
	./logic_table
list: table_management.cpp
	gcc table_management.cpp -o table_management -lstdc++ -lm
list_exec:
	./table_management
main: main.cpp
	gcc main.cpp -o main -lstdc++
exec:
	./main table_4d.txt
clean:
	rm -f *.o logic_table table_management main
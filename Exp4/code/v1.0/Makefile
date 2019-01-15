OBJ1 = syntax/*.o
OBJ2 = semantic/*.o
OBJ3 = IR/*.o
OBJ4 = object/*.o
TARGET = parser

$(TARGET) :
	cd syntax && make;
	cd semantic && make;
	cd IR && make;
	cd object && make;
	gcc -o $(TARGET) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) -lfl
	
clean :
	cd syntax && make clean;
	cd semantic && make clean;
	cd IR && make clean;
	cd object && make clean;
	rm $(TARGET)
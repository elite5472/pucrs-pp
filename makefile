default:
	mkdir -p bin && ladcomp -env mpiCC src/main.cpp -o bin/main && ladcomp -env mpiCC src/divcon.cpp -o bin/divcon
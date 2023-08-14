gcc -c -g -std=c11 -pedantic -Wno-pedantic-ms-format -Wall -Wextra ./source/main.c -o ./bin/main.o
gcc -c -g -std=c11 -pedantic -Wno-pedantic-ms-format -Wall -Wextra ./source/text.c -o ./bin/text.o
gcc -g ./bin/main.o ./bin/text.o -o ./bin/filechopper.exe

# ./bin/filechopper.exe <start/end/start-end> <nr of bytes from start or end, or two numbers if using start-end> [cap <max nr of bytes>] <input file name> [<to/append-to> <output file name>]
# examples
# ./bin/filechopper.exe start 0 cap 2000 "%userprofile%/Desktop/chopmeup/chopmeup.txt"
# ./bin/filechopper.exe end 0 cap 1000 "%userprofile%/Desktop/chopmeup/chopmeup.txt" to "%userprofile%/Desktop/chopmeup/result.txt"
# ./bin/filechopper.exe start-end 2000 1000 "%userprofile%/Desktop/chopmeup/chopmeup.txt" append-to "%userprofile%/Desktop/chopmeup/result.txt"

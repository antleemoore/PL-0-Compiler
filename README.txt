input:
    - input can be named anything as long as it is a .txt file
output:
    - change output print with -l -a -v on commandline
compile:
    - compile with: 
        gcc compiler.c
        ./a.out -l -a -v (filename).txt
    - command line arguments can be in any order
    - every command, except the filename, must start with a '-'
        followed by the letter of the argument
        - (-a) works
        - (a) doesn't

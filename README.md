# producer-consumer-timer
A C timer using a multithreading producer consumer architecture.
This repo is about the final assigment on the real-time embedded systems class . <br>
**The timer's implementation is in the file [timer.c](https://github.com/mikalaki/producer-consumer-timer/blob/master/timer.c) **
## Compiling (and cross-compoling) and Execution of the Program
### Compiling and Execution on a Linux Desktop Machine
For executing the program on a Linux Deskt Machine you have first to compile it,
by executing the command bellow inside the project folder :<br>
` gcc timer.c myFunctions.c -pthread -o program.out -lm -O3 `<br>
after compilation finish you are able to execute the programm with the following command<br>
`./program.out s q` , where **s is the common queue size** you want to have and **q the number of consumers**,<br>
for instance if you want to have a queue with capacity of 4 and 2 consumers threads you have to run the command:<br>
`./program.out 4 2` <br>
if queue size and number of consumer threads are not set as expected, both values are set by default to 4. <br>

### (Cross-)Compiling and Execution for a Raspberry Pi with [this OS img](https://www.dropbox.com/s/0sp5a1s6r5ee3kw/ESPX-rasp.tar.gz?dl=1)<br>
In order for be able to execute the programm to a embedded system (for me a raspberry pi 4 model B), <br>
it has first to be compiled in such a way. This can happen with a **cross compiler**.<br>
The **cross compiler** used for this project is given in this [link](https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Stretch/GCC%206.3.0/), and bellow the proccess followed:<br>
1.Choose the right compiler for our model and follow the proper quide [here](https://github.com/abhiTronix/raspberry-pi-cross-compilers/wiki)<br>
2.Compile our source code with the command bellow :<br>
` arm-linux-gnueabihf-gcc timer.c myFunctions.c -pthread -o program.out -lm -O3 `<br>
3. Transfer our executable to our embedded system, possible with `scp` command.<br> <br>
Then it is possible to execute our programm inside our embedded system, with the command
bellow, in the directory where the executable file has been transfered:<br>
`./program.out s q` , where **s is the common queue size** you want to have and **q the number of consumers**,<br>
for instance if you want to have a queue with capacity of 4 and 2 consumers threads you have to run the command:<br>
`./program.out 4 2` <br>
if queue size and number of consumer threads are not set as expected, both values are set by default to 4. <br>

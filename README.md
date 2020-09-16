# simple-threads-dynamicload-demo
Very simple demo of threading and how to dynamically load code into an already running program

The demo program starts up a thread which loops contunuously, and then
listens on standard input for one letter "commands" which can change the
behavior of the loop.

The looping thread simply passes an integer value to an algorithm function,
storing the return value to pass next time around, then prints the value, 
sleeps for a second, and loops again provided the stop flag hasn't been set.
(See the `loop` function)

The algorithm function is called as a pointer to a function - `(*algo_p)()`
- this pointer can point to one of several algorithm functions, and can be
reassigned on the fly at any time. Since `algo_p` is defined in the global
scope, this can be done from anywhere, including outside the thread.

Two trivial "algorithms" are built into the main C code - `algo1` simply
returns the input as the output, `algo2` increments the value. `algo3` is 
defined as a function pointer in the code, but initialized to NULL and no 
actual function is supplied - this function can be dynamically loaded while 
the loop is running as discussed shortly.

The 'q' command sets a flag which causes the loop to stop and the program to 
exit. The '1', '2' and '3'  commands selects the algorithm function - ie 
'1' sets `algo_p = &algo1`, etc. '3' is nice enough to refuse to set the
pointer if the function still hasn't been loaded.

You can compile the demo.c program with `gcc demo.c -l pthread -ldl -o demo`
A simple example run flipping between alorithms 1 and 2 might look like:

> -bash-4.2$ ./demo 
> value is 0
> value is 0
> 2
> Switching to algo2
> value is 1
> value is 2
> value is 3
> 1
> Switching to algo1
> value is 3
> value is 3
> value is 3
> 2
> Switching to algo2
> value is 4
> value is 5
> value is 6
> 1
> Switching to algo1
> value is 6
> value is 6
> value is 6
> q
> -bash-4.2$ 

Exciting, huh? We've simply demonstrated here how to reassign a pointer to
a function to point to different functions in one place in the code, and that
this will affect other places in the code where we're calling the function
pointed to by that pointer.

OK, so the more interesting thing is we can dynamically load a function
from a dynamic library, and we can then change the pointer to point to that
function while the code is running.

`mylib.c` simply contains our function `algo3', which (for now) multiplies
the input value by -1 before returning it.

We compile mylib.c into mylib.o with `gcc -c -fPIC mylib.c` and then we make
a small shared object library from that with `gcc mylib.o -shared -o mylib.so`

We need to specify the filename (full path to keep things simple) of mylib.so
in the #define near the top of demo.c - if you changed that you'll need to
recompile demo.c (because this is a really simple hardwired demo).

The 'o' command opens the dynamic library and assigns the 'algo3' pointer
to function variable to point to the algo3 symbol in that library.
After we've done that, we can use '3' to set the algo_p pointer to point to it.

> -bash-4.2$ ./demo
> value is 0
> 2
> Switching to algo2
> value is 1
> value is 2
> value is 3
> 1
> Switching to algo1
> value is 3
> o
> Opening dynamic library.
> value is 3
> value is 3
> 3
> Switching to algo3
> value is -3
> value is 3
> value is -3
> 1
> Switching to algo1
> value is -3
> value is -3
> q

The 'c' command closes the dynamic library. If we still have the address of
algo3 stored somewhere and we call if after closing the library, bad things
will happen. 

> -bash-4.2$ ./demo
> value is 0
> o
> Opening dynamic library.
> value is 0
> 2
> Switching to algo2
> value is 1
> value is 2
> 3
> Switching to algo3
> value is -2
> value is 2
> value is -2
> c
> Closing dynamic library.
> May the force be with you.
> Segmentation fault

However, if we switch back to algo1 or algo2 first, that's fine:

> -bash-4.2$ ./demo
> value is 0
> o
> Opening dynamic library.
> value is 0
> 2
> Switching to algo2
> value is 1
> value is 2
> value is 3
> 3
> Switching to algo3
> value is -3
> value is 3
> value is -3
> 1
> Switching to algo1
> value is -3
> value is -3
> c
> Closing dynamic library.
> value is -3
> value is -3
> 2
> Switching to algo2
> value is -2
> value is -1
> value is 0
> value is 1
> value is 2
> q

Now, the really interesting bit. So long as we close it first, we can 
edit and recompile mylib.so, then reopen it while the program is running:

> -bash-4.2$ ./demo
> value is 0
> o
> Opening dynamic library.
> value is 0
> 2
> Switching to algo2
> value is 1
> value is 2
> value is 3
> 3
> Switching to algo3
> value is -3
> value is 3
> value is -3
> 1
> Switching to algo1
> value is -3
> value is -3
> c
> Closing dynamic library.
> value is -3
> value is -3
> ...
At this point, leave the program running spitting out -3s, and in another
terminal go ahead and edit mylib.c to do something different, recompile 
mylib.so as before, and then you can open it and use the new code in the 
still running demo program:
> ...
> value is -3
> value is -3
> o
> Opening dynamic library.
> value is -3
> 3
> value is 6
> value is -12
> value is 24
> value is -48
2
> value is -47
> value is -46
> q



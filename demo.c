#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

#define MYLIB "/path/to/mylib.so"

int go=1;

int algo1(int data) {
  return data;
}

int algo2(int data) {
  return data + 1;
}

int (*algo_p)(int) = &algo1;

void *loop() {
  int value = 0;
  while(go) {
    value = (*algo_p)(value);
    printf("value is %d\n", value);
    sleep(1);
  }
}

int main() {
  pthread_t thread_id;
  char c = NULL;
  void *mylib;
  char *error;
  int (*algo3)(int)=NULL;

  pthread_create(&thread_id, NULL, loop, NULL);

  while(go) {
    c=getc(stdin);
    switch(c) {
      case 'q':
        go = 0;
        pthread_join(thread_id, NULL);
        break;
      case '1':
        printf("Switching to algo1\n");
        algo_p = &algo1;
        break;
      case '2':
        printf("Switching to algo2\n");
        algo_p = &algo2;
        break;
      case '3':
        if (algo3 == NULL) {
          printf("You haven't loaded algo3 yet\n");
        } else {
          printf("Switching to algo3\n");
          algo_p = algo3;
        }
        break;
      case 'o':
        printf("Opening dynamic library.\n");
        mylib = dlopen(MYLIB, RTLD_NOW);
        if (!mylib) {
          fputs (dlerror(), stderr);
          exit(1);
        }
        algo3 = dlsym(mylib, "algo3");
        if ((error = dlerror()) != NULL)  {
          fputs(error, stderr);
          exit(1);
        }
        break;
      case 'c':
        printf("Closing dynamic library.\n");
        if(algo_p == algo3) {
          printf("May the force be with you.\n");
        }
        dlclose(mylib);
        if ((error = dlerror()) != NULL)  {
          fputs(error, stderr);
          exit(1);
        }
    }
  }
  return 0;
}

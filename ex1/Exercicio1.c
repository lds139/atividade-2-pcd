#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

#define NUM_THREADS 3

int main(int argc, char *argv[])
{

  int soma = 0;
  int request = 0;
  int respond = 0;
  int th_id, local;

  #pragma omp parallel num_threads(NUM_THREADS) private(th_id, local) shared(soma, request, respond)
  {
    
    th_id = omp_get_thread_num();

    if (th_id == 0)
    { //Server: thread id == 0
      while (1)
      {
        while (request == 0)
        { /* await */
        }
        respond = request;
        while (respond != 0)
        { /* await */
        }
        request = 0;
      }
    }

    else
    { //clients: another threads

      while (1)
      {
        while (respond != th_id)
        {
          request = th_id;
        }

        local = soma;
        sleep(rand() % 2);
        soma = local + 1;
        printf("thread [%d] soma [%d]\n", th_id, soma);
        respond = 0;
      }
    }
  }

  printf("soma=%d\n", soma);
  return 0;
}

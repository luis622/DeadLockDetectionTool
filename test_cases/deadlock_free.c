#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
  int balance;
  pthread_mutex_t balance_mutex;
  unsigned int id; /* Should never be changed after initialized */
} bank_account;

typedef struct {
  bank_account *from;
  bank_account *to;
  int amount;
} deposit_thr_args;
 
unsigned int global_id = 1;
 
void create_bank_account(bank_account **ba, int initial_amount) {
  int result;
  bank_account *nba = malloc(sizeof(bank_account));
  if (nba == NULL) {
    /* Handle error */
  }
 
  nba->balance = initial_amount;
  result = pthread_mutex_init(&nba->balance_mutex, NULL);
  if (result != 0) {
    /* Handle error */
  }
 
  nba->id = global_id++;
  *ba = nba;
}
 
void *deposit(void *ptr) 
{
  deposit_thr_args *args = (deposit_thr_args *)ptr;
  int result;
 
  if (args->from->id == args->to->id)
      return NULL;
 
  /* Ensure proper ordering for locking */
  if (args->from->id < args->to->id) {
    if ((result = pthread_mutex_lock(&(args->from->balance_mutex))) != 0) {
      /* Handle error */
    }
    if ((result = pthread_mutex_lock(&(args->to->balance_mutex))) != 0) {
      /* Handle error */
    }
  } else {
    if ((result = pthread_mutex_lock(&(args->to->balance_mutex))) != 0) {
      /* Handle error */
    }
    if ((result = pthread_mutex_lock(&(args->from->balance_mutex))) != 0) {
      /* Handle error */
    }
  }
 
  /* Not enough balance to transfer */
  if (args->from->balance < args->amount) {
    if ((result = pthread_mutex_unlock(&(args->from->balance_mutex))) != 0) {
      /* Handle error */
    }
    if ((result = pthread_mutex_unlock(&(args->to->balance_mutex))) != 0) {
      /* Handle error */
    }
   return NULL;
  }
 
  args->from->balance -= args->amount;
  args->to->balance += args->amount;
 
  if ((result = pthread_mutex_unlock(&(args->from->balance_mutex))) != 0) {
    /* Handle error */
  }
  if ((result = pthread_mutex_unlock(&(args->to->balance_mutex))) != 0) {
    /* Handle error */
  }
 
  free(ptr);
  return NULL;
}
 
int main(void) {
 
  pthread_t thr1, thr2;
  int result;
 
  bank_account *ba1;
  bank_account *ba2;
  create_bank_account(&ba1, 1000);
  create_bank_account(&ba2, 1000);
 
  deposit_thr_args *arg1 = malloc(sizeof(deposit_thr_args));
  if (arg1 == NULL) {
    /* Handle error */
    printf("malloc failed\n");
    exit(0);
  }
  deposit_thr_args *arg2 = malloc(sizeof(deposit_thr_args));
  if (arg2 == NULL) {
    /* Handle error */
    printf("malloc failed\n");
    exit(0);
  }
 
  arg1->from = ba1;
  arg1->to = ba2;
  arg1->amount = 100;
 
  arg2->from = ba2;
  arg2->to = ba1;
  arg2->amount = 100;
 
  /* Perform the deposits ./*/
  if ((result = pthread_create(&thr1, NULL, deposit, (void *)arg1)) != 0) {
    /* Handle error */
    printf("create 1 failed\n");
    exit(0);
  }
  if ((result = pthread_create(&thr2, NULL, deposit, (void *)arg2)) != 0) {
    /* Handle error */
    printf("create 2 failed\n");
    exit(0);
  }

  pthread_join( thr1, NULL);
  pthread_join( thr2, NULL);
  printf("Test Complete\n");
  return 0;
}
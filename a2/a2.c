#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "a2_helper.h"
#include<semaphore.h>
#include <pthread.h>
#define TH7_THREADS 5
#define TH9_THREADS 43
#define TH6_THREADS 4

sem_t semt7_1,semt7_2;
sem_t semt9_1;
pthread_cond_t cond;
pthread_cond_t cond2;
//pthread_cond_t cond10;
pthread_mutex_t lock;
//pthread_mutex_t lock2;
sem_t *sem1,*sem2;
//sem_t m,barrier;
int th_crit=0;
int th_crit_exit=0;
int done10=0;
int waiting10=0;
int waiting=0;

typedef struct{
    int t_id;
    int p_id;

}TH_STRUCT;

void* th_7(void *arg){
    TH_STRUCT *param=(TH_STRUCT*)arg;
    if(param->t_id==5)sem_wait(&semt7_1);
    else if(param->t_id==4)sem_wait(sem1);
    info(BEGIN, param->p_id, param->t_id);
    if(param->t_id==2){
        sem_post(&semt7_1);
        sem_wait(&semt7_2);
    }
    info(END, param->p_id, param->t_id);
    if(param->t_id==5)sem_post(&semt7_2);
    else if(param->t_id==4)sem_post(sem2);
    return NULL;
}

void* th_9(void *arg){
    TH_STRUCT *param=(TH_STRUCT*)arg;
    if(param->t_id!=10)
        sem_wait(&semt9_1);
    info(BEGIN, param->p_id, param->t_id);
    
    
    pthread_mutex_lock(&lock);
    th_crit++;
    if(done10==0){
        if(th_crit==5){
            if(param->t_id==10){

            }else{
                pthread_cond_signal(&cond2);
                pthread_cond_wait(&cond,&lock);
            }
        }else{
            if(param->t_id==10){
                pthread_cond_wait(&cond2,&lock);
            }else{
                pthread_cond_wait(&cond,&lock);
            }
        }
    }
        //}
    //}
    done10=1;
    //printf("%d after if\n", param->t_id);
    //pthread_cond_broadcast(&cond);
    th_crit--;
    pthread_mutex_unlock(&lock);
    
    info(END, param->p_id, param->t_id);
    sem_post(&semt9_1);
    if(param->t_id==10)pthread_cond_broadcast(&cond);
    return NULL;
}

void* th_6(void *arg){
    TH_STRUCT *param=(TH_STRUCT*)arg;
    if(param->t_id==1)sem_wait(sem2);
    info(BEGIN, param->p_id, param->t_id);
    info(END, param->p_id, param->t_id);
    if(param->t_id==4)sem_post(sem1);
    return NULL;
}

int main(int argc,char **argv){
    pid_t p2,p3,p4,p5,p6,p7,p8,p9;
    //p1
    init();
    info(BEGIN, 1, 0);

    sem_unlink("/sem1");
    sem_unlink("/sem2");
    sem1=sem_open("/sem1",O_CREAT | O_EXCL,0664,0);
    sem2=sem_open("/sem2",O_CREAT | O_EXCL,0664,0);

    p2=fork();
    if(p2==0){
        info(BEGIN, 2, 0);
        p3=fork();
        if(p3==0){
            info(BEGIN, 3, 0);
            info(END,3,0);
        }else{
            p6=fork();
            if(p6==0){
                info(BEGIN, 6, 0);

                TH_STRUCT params[TH6_THREADS];
                pthread_t tids[TH6_THREADS];
                for(int i=0;i<TH6_THREADS;i++){
                    params[i].t_id=i+1;
                    params[i].p_id=6;
                    pthread_create(&tids[i],NULL,th_6,&params[i]);
                }
                for(int i=0;i<TH6_THREADS;i++){
                    pthread_join(tids[i],NULL);
                }

                p8=fork();
                if(p8==0){
                    info(BEGIN, 8, 0);
                    info(END,8,0);
                }else{
                    waitpid(p8,NULL,0);
                    info(END,6,0);
                }
            }else{
                p9=fork();
                if(p9==0){
                    info(BEGIN, 9, 0);
                    TH_STRUCT params[TH9_THREADS];
                    pthread_t tids[TH9_THREADS];
                    if(sem_init(&semt9_1,0,4)!=0){
                        perror("Could not init semaphore");
                        return -1;
                    }
                    pthread_mutex_init(&lock,NULL);
                    pthread_cond_init(&cond,NULL);
                    //pthread_mutex_init(&lock2,NULL);
                    pthread_cond_init(&cond2,NULL);
                    //pthread_cond_init(&cond10,NULL);
                    //sem_init(&m,0,1);
                    //sem_init(&barrier,0,0);
                    for(int i=0;i<TH9_THREADS;i++){
                        params[i].t_id=i+1;
                        params[i].p_id=9;
                        pthread_create(&tids[i],NULL,th_9,&params[i]);
                    }
                    for(int i=0;i<TH9_THREADS;i++){
                        pthread_join(tids[i],NULL);
                    }
                    info(END,9,0);
                }else{
                    waitpid(p3,NULL,0);
                    waitpid(p6,NULL,0);
                    waitpid(p9,NULL,0);
                    info(END,2,0);
                }
                
            }
            
        }
        
    }else{
        p4=fork();
        if(p4==0){
            info(BEGIN, 4, 0);
            info(END,4,0);
        }else{
            p5=fork();
            if(p5==0){
                info(BEGIN, 5, 0);
                info(END,5,0);
            }else{
                p7=fork();
                if(p7==0){
                    info(BEGIN, 7, 0);
                    TH_STRUCT params[TH7_THREADS];
                    pthread_t tids[TH7_THREADS];
                    if(sem_init(&semt7_1,0,0)!=0){
                        perror("Could not init semaphore");
                        return -1;
                    }
                    if(sem_init(&semt7_2,0,0)!=0){
                        perror("Could not init semaphore");
                        return -1;
                    }
                    for(int i=0;i<TH7_THREADS;i++){
                        params[i].t_id=i+1;
                        params[i].p_id=7;
                        pthread_create(&tids[i],NULL,th_7,&params[i]);
                    }
                    for(int i=0;i<TH7_THREADS;i++){
                        pthread_join(tids[i],NULL);
                    }
                    info(END,7,0);
                }else{
                    waitpid(p2,NULL,0);
                    waitpid(p4,NULL,0);
                    waitpid(p5,NULL,0);
                    waitpid(p7,NULL,0);
                    info(END,1,0);
                    sem_destroy(&semt7_1);
                    sem_destroy(&semt7_2);
                }
            }
            
        }
        
    }
    sem_close(sem1);
    sem_close(sem2);
    
    return 0;
}
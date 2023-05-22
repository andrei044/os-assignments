#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<sys/mman.h>
#define RESP_FIFO_NAME "RESP_PIPE_40286"
#define REQ_FIFO_NAME "REQ_PIPE_40286"
#define SHM_NAME "/eseCBG"
#define ALIGNMENT 3072


int main(){
    
    if(mkfifo(RESP_FIFO_NAME,0644)!=0){
        printf("ERROR\ncannot create the response pipe");
        return -1;
    }

    int fd_w,fd_r;
    const char success_str[]="SUCCESS$";
    const char error_str[]="ERROR$";
    const char hello_str[]="HELLO$";
    const char variant_str[]="VARIANT$VALUE$";
    const char create_shm_str[]="CREATE_SHM$";
    const char write_to_shm_str[]="WRITE_TO_SHM$";
    const char map_file_str[]="MAP_FILE$";
    const char read_from_file_offset_str[]="READ_FROM_FILE_OFFSET$";
    const char read_from_file_section_str[]="READ_FROM_FILE_SECTION$";
    const char read_from_logical_space_offset_str[]="READ_FROM_LOGICAL_SPACE_OFFSET$";
    const unsigned int value=40286;
    unsigned int shm_size,shm_offset,shm_val,file_size,file_offset,file_bytes,file_section;
    char file_name[256];
    char* file_buff=NULL;
    int shmFd,file_fd;
    volatile char* shared_buff; 
    fd_r=open(REQ_FIFO_NAME,O_RDONLY);
    if(fd_r==-1){
        printf("ERROR\ncannot open the request pipe");
        return -2;
    }
    fd_w=open(RESP_FIFO_NAME,O_WRONLY);
    if(fd_w==-1){
        printf("ERROR\ncannot open the response pipe");
        return -3;
    }

    
    for(int i=0;i<6;i++){
        write(fd_w,&hello_str[i],1);
    }
    printf("SUCCESS\n");
    for(;;){
        char msg_req[256];
        char c;
        int i=0;
        do{
            read(fd_r,&c,1);
            msg_req[i]=c;
            i++;
        }while(c!='$');
        msg_req[i]='\0';
        if(strstr(msg_req,"VARIANT")!=0){
            for(int i=0;i<14;i++){
                write(fd_w,&variant_str[i],1);
            }
            write(fd_w,&value,4);
        }else if(strstr(msg_req,"EXIT")!=0){
            break;
        }else if(strstr(msg_req,"CREATE_SHM")!=0){
            read(fd_r,&shm_size,4);
            for(int i=0;i<strlen(create_shm_str);i++){
                write(fd_w,&create_shm_str[i],1);
            }
            
            shmFd=shm_open(SHM_NAME,O_CREAT | O_RDWR, 0666);
            //printf("%d",shmFd);
            if(shmFd<0){
                //printf("DA");
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;   
            }
            ftruncate(shmFd,shm_size);
            shared_buff=(volatile char*)mmap(0,shm_size,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
            if(shared_buff==(void*)-1){
                //printf("DA2");
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }
            for(int i=0;i<strlen(success_str);i++){
                write(fd_w,&success_str[i],1);
            }
        }else if(strstr(msg_req,"WRITE_TO_SHM")!=0){
            read(fd_r,&shm_offset,4);
            read(fd_r,&shm_val,4);
            for(int i=0;i<strlen(write_to_shm_str);i++){
                write(fd_w,&write_to_shm_str[i],1);
            }
            if(shm_offset+3<shm_size){
                shared_buff[shm_offset]=shm_val & 0xff;
                shared_buff[shm_offset+1]=(shm_val>>8) & 0xff;
                shared_buff[shm_offset+2]=(shm_val>>16) & 0xff;
                shared_buff[shm_offset+3]=(shm_val>>24) & 0xff;
                for(int i=0;i<strlen(success_str);i++){
                    write(fd_w,&success_str[i],1);
                }
            }else{
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
            }
        }else if(strstr(msg_req,"MAP_FILE")!=0){
            int i=0;
            do{
                read(fd_r,&file_name[i],1);
                i++;
            }while(file_name[i-1]!='$');
            file_name[i-1]='\0';
            for(int i=0;i<strlen(map_file_str);i++){
                write(fd_w,&map_file_str[i],1);
            }
            file_fd=open(file_name,O_RDONLY);
            if(file_fd==-1){
                //printf("%s\n",file_name);
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }
            file_size=lseek(file_fd,0,SEEK_END);
            lseek(file_fd,0,SEEK_SET);
            file_buff=(char*)mmap(NULL,file_size,PROT_READ,MAP_SHARED,file_fd,0);
            if(file_buff==(void*)-1){
                //printf("AICI");
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                close(file_fd);
                //MAP_FILE$test_root/E8idmLfiO1.O2t$
                continue;
            }
            for(int i=0;i<strlen(success_str);i++){
                write(fd_w,&success_str[i],1);
            }
        }else if(strstr(msg_req,"READ_FROM_FILE_OFFSET")!=0){
            read(fd_r,&file_offset,4);
            read(fd_r,&file_bytes,4);
            for(int i=0;i<strlen(read_from_file_offset_str);i++){
                write(fd_w,&read_from_file_offset_str[i],1);
            }
            if(file_offset+file_bytes-1>file_size || shared_buff==(void*)-1 || file_buff==(void*)-1){
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }
            for(int i=0;i<file_bytes;i++){
                shared_buff[i]=file_buff[file_offset+i];
            }
            for(int i=0;i<strlen(success_str);i++){
                write(fd_w,&success_str[i],1);
            }

        }else if(strstr(msg_req,"READ_FROM_FILE_SECTION")!=0){
            read(fd_r,&file_section,4);
            read(fd_r,&file_offset,4);
            read(fd_r,&file_bytes,4);

            for(int i=0;i<strlen(read_from_file_section_str);i++){
                write(fd_w,&read_from_file_section_str[i],1);
            }

            char nr_sections=file_buff[7];
            //printf("%d %d\n",nr_sections,file_section);
            if(file_section<1 || file_section>(int)nr_sections){
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }
            unsigned int sect_offset=*(unsigned int*)(file_buff+8+19*(file_section-1)+11);
            unsigned int sect_size=*(unsigned int*)(file_buff+8+19*(file_section-1)+15);
            
            //printf("sect_off:%u sect_size:%u file_offset:%u file_bytes:%u file_size:%u\n",sect_offset,sect_size,file_offset,file_bytes,file_size);
            if(file_offset+file_bytes>sect_size){
                //printf("aici2");
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }
            for(unsigned int i=0;i<file_bytes;i++){
                //printf("%ld\n",sect_offset+file_offset+i);
                shared_buff[i]=file_buff[sect_offset+file_offset+i];
            }
            for(int i=0;i<strlen(success_str);i++){
                write(fd_w,&success_str[i],1);
            }
        }else{
            read(fd_r,&file_offset,4);
            read(fd_r,&file_bytes,4);

            for(int i=0;i<strlen(read_from_logical_space_offset_str);i++){
                write(fd_w,&read_from_logical_space_offset_str[i],1);
            }
            
            char nr_sections=file_buff[7];
            //unsigned int header_size=4+2+1+1+nr_sections*(7+4+4+4);
            /*if(file_offset+file_bytes>file_size-header_size){
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
                continue;
            }*/
            //printf("%d logical_off:%d bytes:%d\n",nr_sections,file_offset,file_bytes);
            int current_alignment=0;
            int cnt=0;
            int ok=0;
            for(int k=0;k<nr_sections;k++){
                unsigned int sect_offset=*(unsigned int*)(file_buff+8+19*k+11);
                unsigned int sect_size=*(unsigned int*)(file_buff+8+19*k+15);
                unsigned int blocks=sect_size/ALIGNMENT;
                unsigned int next_alignment=(blocks+1)*ALIGNMENT+current_alignment;
                //printf("sect_off:%d sect_size:%d\n",sect_offset,sect_size);
                //printf("current_alg:%d next_alg:%d\n",current_alignment,next_alignment);
                if(file_offset>=current_alignment && file_offset<next_alignment){
                    printf("found\n");
                    int i;
                    ok=1;
                    for(i=0;cnt<file_bytes && i<sect_size;i++){
                        shared_buff[cnt]=file_buff[sect_offset+file_offset-current_alignment+i];
                        cnt++;
                    }
                    if(cnt<file_bytes && i==sect_size)ok=0;
                    break;
                }
                current_alignment=next_alignment;
            }
            if(ok==1){
                for(int i=0;i<strlen(success_str);i++){
                write(fd_w,&success_str[i],1);
                }
            }else{
                for(int i=0;i<strlen(error_str);i++){
                    write(fd_w,&error_str[i],1);
                }
            }
            
            /*int current_alignment=0;
            for(int k=0;k<nr_sections;k++){
                unsigned int sect_offset=*(unsigned int*)(file_buff+8+19*k+11);
                unsigned int sect_size=*(unsigned int*)(file_buff+8+19*k+15);
                unsigned int blocks=sect_size/ALIGNMENT;
                for(int i=0;i<blocks+1;i++){
                    for(int j=0;j<ALIGNMENT && j<sect_size;j++){
                        shared_buff[file_offset+current_alignment+j]=file_buff[sect_offset+j];
                    }
                    current_alignment+=ALIGNMENT;
                }
            }*/
            //for(int i=0;i<strlen(success_str);i++){
                //write(fd_w,&success_str[i],1);
            //}

        }   
    }
    munmap((void*)file_buff,file_size);
    file_buff=NULL;
    close(file_fd);
    munmap((void*)shared_buff,shm_size);
    shared_buff=NULL;
    close(shmFd);
    close(fd_r);
    close(fd_w);
    unlink(RESP_FIFO_NAME);
    return 0;
}
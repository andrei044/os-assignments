#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int compareEnd(const char* str1,const char* str2){
    int i=strlen(str1)-1;
    int j=strlen(str2)-1;
    int flag=0;
    for(;i>=0 && j>=0 && flag==0;i--,j--){
        if(str1[i]!=str2[j])
            flag=1;
    }
    if(i>=0)return 0;
    if(flag==1)return 0;
    return 1;
}

void recIteration(const char* current_path,const char* path,const int recFlag,const int nameFlag,const char* name, const int writeFlag){
    DIR *dir=NULL;
    struct dirent *entry=NULL;
    char fullPath[2048];
    struct stat statbuf;
    dir=opendir(current_path);
    if(dir==NULL){
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    if(strcmp(current_path,path)==0)
        printf("SUCCESS\n");
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,"..")!=0 && strcmp(entry->d_name,".")!=0){
            snprintf(fullPath,2048,"%s/%s",current_path,entry->d_name);
            //printf("%s\n",fullPath);
            if(lstat(fullPath,&statbuf)==0){
                if(nameFlag==1){
                    if(compareEnd(name,entry->d_name)){
                        if(writeFlag==1){
                            if(statbuf.st_mode & S_IWUSR){
                                printf("%s\n",fullPath);
                            }
                        }else{
                            printf("%s\n",fullPath);
                        }
                    }
                }else{
                    if(writeFlag==1){
                        if(statbuf.st_mode & S_IWUSR){
                            printf("%s\n",fullPath);
                        }
                    }else{
                        printf("%s\n",fullPath);
                    }
                }

                if(recFlag==1 && S_ISDIR(statbuf.st_mode)){
                    recIteration(fullPath,path,recFlag,nameFlag,name,writeFlag);
                }
            }
        }
    }
    closedir(dir);
}


int main(int argc, char **argv){
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("40286\n");
        }else if(strcmp(argv[1],"list")==0){
                int recFlag=0,nameFlag=0,writeFlag=0;
                char path[2048],name[512];
                for(int i=1;i<argc;i++){
                    if(strcmp(argv[i],"recursive")==0)recFlag=1;
                    else{
                        char* p;
                        p=strtok(argv[i],"=");
                        if(strcmp(p,"path")==0){
                            p=strtok(NULL,"=");
                            if(p!=NULL)strcpy(path,p);
                            else printf("ERROR\ninvalid directory path\n");
                        }else if(strcmp(p,"name_ends_with")==0){
                            p=strtok(NULL,"=");
                            if(p!=NULL){
                                strcpy(name,p);
                                nameFlag=1;
                            }else printf("ERROR\ninvalid ending name");
                            
                        }else if(strcmp(p,"has_perm_write")==0){
                            writeFlag=1;
                        }
                    }
                }
                recIteration(path,path,recFlag,nameFlag,name,writeFlag);
                //printf("%s %s %d %d\n",path,name,writeFlag,recFlag);
            // if(strcmp(argv[2],"recursive")==0){
            //     char* p;
            //     p=strtok(argv[3],"=");
            //     p=strtok(NULL,"=");
            //     recIteration(p,0);
            // }else{
            //     char* p;
            //     p=strtok(argv[2],"=");
            //     p=strtok(NULL,"=");
            //     recIteration(p,1);
            // }
        }

    }

    return 0;
}
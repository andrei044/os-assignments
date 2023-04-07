#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

typedef struct section{
    char sect_name[8];
    int sect_type;
    int sect_offset;
    int sect_size;
}section;

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

int parseFile(const char* path,int afis,int my_section,section *ret_section){
    int fd;
    char magic[5];
    //int header_size;
    int version;
    int no_of_sections;
    char sect_name[8];
    int sect_type;
    int sect_offset;
    int sect_size;
    int ok=0;
    char buf[8];
    section *sections=NULL;
    fd=open(path,O_RDONLY);
    if(fd==-1){
        if(afis==1)printf("ERROR\ninvalid path");
        return 1;
    }
    //read magic
    if(read(fd, magic, 4) != 4) {
        if(afis==1)printf("ERROR\nCould not read magic");
        close(fd);
        return 1;
    }
    magic[4]='\0';
    //printf("%s\n",magic);
    if(strcmp(magic,"6a4t")!=0){
        if(afis==1)printf("ERROR\nwrong magic");
        close(fd);
        return 1;
    }
    
    //read header_size
    if(read(fd, buf, 2) != 2) {
        if(afis==1)printf("ERROR\nCould not read header_size");
        close(fd);
        return 1;
    }
    //header_size=(buffer[0]<<8)|buf[1];
    //version
    if(read(fd, buf, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read version");
        close(fd);
        return 1;
    }
    version=(int)buf[0];
    if(version<52 || version>86){
        if(afis==1)printf("ERROR\nwrong version");
        close(fd);
        return 1;
    }

    //no_of_sections
    if(read(fd, buf, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read no_of_sections");
        close(fd);
        return 1;
    }
    no_of_sections=(int)buf[0];
    if(no_of_sections<6 || no_of_sections>14){
        if(afis==1)printf("ERROR\nwrong sect_nr");
        close(fd);
        return 1;
    }
    sections=(section*)malloc(sizeof(section)* no_of_sections);
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, sect_name, 7) != 7) {
            if(afis==1)printf("ERROR\nCould not read sect_name");
            free(sections);
            close(fd);
            return 1;
        }
        sect_name[7]='\0';
        if(read(fd, &sect_type, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_type");
            free(sections);
            close(fd);
            return 1;
        }
        if(sect_type!=13 && sect_type!=52 && sect_type!=51 && sect_type!=11 && sect_type!=91 && sect_type!=29){
            if(afis==1)printf("ERROR\nwrong sect_types");
            free(sections);
            close(fd);
            return 1;
        }
        if(read(fd, &sect_offset, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_offset");
            free(sections);
            close(fd);
            return 1;
        }
        if(read(fd, &sect_size, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_size");
            free(sections);
            close(fd);
            return 1;
        }
        strcpy(sections[i].sect_name,sect_name);
        sections[i].sect_type=sect_type;
        sections[i].sect_offset=sect_offset;
        sections[i].sect_size=sect_size;
        if( afis!=3 && ret_section!=NULL && i+1==my_section){
            strcpy(ret_section->sect_name,sect_name);
            ret_section->sect_type=sect_type;
            ret_section->sect_offset=sect_offset;
            ret_section->sect_size=sect_size;
        }else if(afis==3 && ret_section!=NULL && sect_type==my_section){
            strcpy(ret_section->sect_name,sect_name);
            ret_section->sect_type=sect_type;
            ret_section->sect_offset=sect_offset;
            ret_section->sect_size=sect_size;
            ok=1;
        }
    }
    if(afis!=3 && ret_section!=NULL && (my_section>no_of_sections || my_section<1)){free(ret_section);ret_section=NULL;}
    if(afis==1)printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,no_of_sections);
    for(int i=0;i<no_of_sections;i++){
        if(afis==1)printf("section%d: %s %d %d\n",i+1,sections[i].sect_name,sections[i].sect_type,sections[i].sect_size);
    }
    free(sections);
    close(fd);
    if(afis==3 && ok==1)return 0;
    else if(afis==3 && ok==0)return 1;
    return 0;
}

// int extract(const char* path, const int section_nr, const int line,int afis){
//     int fd;
//     char magic[5];
//     char header_size[3];
//     char version;
//     char no_of_sections;
//     char sect_name[8];
//     int sect_type;
//     int sect_offset;
//     int sect_size;
//     fd=open(path,O_RDONLY);
//     if(fd==-1){
//         if(afis==1)printf("ERROR\ninvalid file");
//         return 1;
//     }
//     //read magic
//     if(read(fd, magic, 4) != 4) {
//         if(afis==1)printf("ERROR\nCould not read magic");
//         close(fd);
//         return 1;
//     }
//     //printf("%s\n",magic);
//     if(strcmp(magic,"6a4t")!=0){
//         if(afis==1)printf("ERROR\nwrong magic");
//         close(fd);
//         return 1;
//     }

//     //read header_size
//     if(read(fd, header_size, 2) != 2) {
//         if(afis==1)printf("ERROR\nCould not read header_size");
//         close(fd);
//         return 1;
//     }

//     //version
//     if(read(fd, &version, 1) != 1) {
//         if(afis==1)printf("ERROR\nCould not read version");
//         close(fd);
//         return 1;
//     }
//     if(version<52 || version>86){
//         if(afis==1)printf("ERROR\ninvalid file");
//         close(fd);
//         return 1;
//     }

//     //no_of_sections
//     if(read(fd, &no_of_sections, 1) != 1) {
//         if(afis==1)printf("ERROR\nCould not read no_of_sections");
//         close(fd);
//         return 1;
//     }
//     if(no_of_sections<6 || no_of_sections>14){
//         if(afis==1)printf("ERROR\nwrong sect_nr");
//         close(fd);
//         return 1;
//     }
//     for(int i=0;i<no_of_sections;i++){
//         if(read(fd, sect_name, 7) != 7) {
//             if(afis==1)printf("ERROR\nCould not read sect_name");
//             close(fd);
//             return 1;
//         }
//         if(read(fd, &sect_type, 4) != 4) {
//             if(afis==1)printf("ERROR\nCould not read sect_type");
//             close(fd);
//             return 1;
//         }
//         if(sect_type!=13 && sect_type!=52 && sect_type!=51 && sect_type!=11 && sect_type!=91 && sect_type!=29){
//             if(afis==1)printf("ERROR\nwrong sect_types");
//             close(fd);
//             return 1;
//         }
//         if(read(fd, &sect_offset, 4) != 4) {
//             if(afis==1)printf("ERROR\nCould not read sect_offset");
//             close(fd);
//             return 1;
//         }
//         if(read(fd, &sect_size, 4) != 4) {
//             if(afis==1)printf("ERROR\nCould not read sect_size");
//             close(fd);
//             return 1;
//         }
//         if(i+1==section_nr){
//             break;
//         }
//     }
//     printf("%d+%d",atoi(header_size),sect_offset);
//     lseek(fd,sect_offset,SEEK_SET);
//     int line_nr=1;
//     char c;
//     off_t j=-2;
//     printf("SUCCESS\n");
//     while(line_nr<=line){
//         if(read(fd,&c,1)!=1){
//             printf("ERROR\ninvalid file");
//             close(fd);
//             return 1;
//         }
//         if(c=='\n')line_nr++;
//     }
//     do{
//         lseek(fd,j,SEEK_CUR);
//         if(read(fd,&c,1)!=1){
//             printf("ERROR\ninvalid file");
//             close(fd);
//             return 1;
//         }
//         printf("%c",c);
//         j--;
//     }while(c!='\n');
//     close(fd);
//     return 0;
// }

int extract(const char* path,const int section_nr,const int line){
    section* x=(section*)malloc(sizeof(section));
    int rez=parseFile(path,0,section_nr,x);
    int my_line=1;
    if(rez!=0){
        printf("ERROR\ninvalid section");
        return 1;
    }
    int fd=open(path,O_RDONLY);
    if(fd==-1){
        printf("ERROR\ninvalid file");
        return 1;
    }
    lseek(fd,x->sect_offset,SEEK_SET);
    //char c[5];
    //char c;
    off_t i;
    //char ce;
    //char lc;
    off_t start=0,end=0;
    //int cnt=0;
    rez=0;
    char *buf=(char*)malloc(sizeof(char)*x->sect_size);
    rez=read(fd,buf,x->sect_size);
    if(rez==-1){
        printf("ERROR\ninvalid file");
        free(buf);
        close(fd);
        return 1;
    }
    for(i=0;i<x->sect_size;i++){
        if(buf[i]=='\n'){
            start=end;
            end=i;
            //printf("l%d: %ld->%ld\n",my_line,start,end);
            if(my_line<line)
                my_line++;
            else break;
        }
        if(i==x->sect_size-1){
            start=end;
            end=i+1;
        }
    }
    // for(i=0;i<x->sect_size;i+=rez){
    //     rez=read(fd,c,4);
    //     c[4]='\0';
    //     if(rez==4){
    //         if(c[0]=='\n'){
    //             start=end;
    //             end=i;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[1]=='\n'){
    //             start=end;
    //             end=i+1;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[2]=='\n'){
    //             start=end;
    //             end=i+2;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[3]=='\n'){
    //             start=end;
    //             end=i+3;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }
    //     }else if(rez==3){
    //         if(c[0]=='\n'){
    //             start=end;
    //             end=i;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[1]=='\n'){
    //             start=end;
    //             end=i+1;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[2]=='\n'){
    //             start=end;
    //             end=i+2;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }
    //     }else if(rez==2){
    //         if(c[0]=='\n'){
    //             start=end;
    //             end=i;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }else if(c[1]=='\n'){
    //             start=end;
    //             end=i+1;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }
    //     }else if(rez==1){
    //         if(c[0]=='\n'){
    //             start=end;
    //             end=i;
    //             if(my_line<line)my_line++;
    //             else if(my_line==line)break;
    //         }
    //     }else{
    //         printf("ERROR\ninvalid line");
    //         close(fd);
    //         return 1;
    //     }


    //     // read(fd,&c,1);
    //     // //printf("%c",c);
    //     // if(c=='\n'){
    //     //     cnt++;
    //     //     start=end;
    //     //     end=i;
    //     //     //if(my_line<line)my_line++;
    //     //     //else if(my_line==line)break;
    //     //     printf("%c->%c\n",ce,lc);
    //     // }
    //     // if(lc=='\n' || i==0){
    //     //     ce=c;
    //     // }
    //     // lc=c;
    // }
    if(my_line<line){
        printf("ERROR\ninvalid line");
        free(buf);
        close(fd);
        return 1;
    }
    char* s=(char *)malloc(sizeof(char)*(end-start+1));
    lseek(fd,x->sect_offset+start,SEEK_SET);
    if(read(fd,s,end-start)!=end-start){
        printf("ERROR\ninvalid file");
        close(fd);
        free(buf);
        return 1;
    }
    s[end-start]='\0';
    for(off_t i=end-start-1;i>=(end-start+1)/2;i--){
        char c=s[i];
        s[i]=s[end-start-1-i];
        s[end-start-1-i]=c;
    }
    printf("SUCCESS\n%s",s);
    free(s);
    free(buf);
    if(x!=NULL)free(x);
    return 0;
}

void findAll(const char* current_path,const char* path){
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
                

                if(S_ISDIR(statbuf.st_mode)){
                    findAll(fullPath,path);
                }else{
                    section* x=(section*)malloc(sizeof(section));
                    int rez=parseFile(fullPath,3,11,x);
                    if(rez==0){
                        printf("%s\n",fullPath);
                        
                    }
                    free(x);
                    x=NULL;
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
                        else{
                            printf("ERROR\ninvalid directory path\n");
                            return 1;
                        } 
                    }else if(strcmp(p,"name_ends_with")==0){
                        p=strtok(NULL,"=");
                        if(p!=NULL){
                            strcpy(name,p);
                            nameFlag=1;
                        }else{
                            printf("ERROR\ninvalid ending name");
                            return 1;
                        } 
                                
                    }else if(strcmp(p,"has_perm_write")==0){
                        writeFlag=1;
                    }
                }
            }
            recIteration(path,path,recFlag,nameFlag,name,writeFlag);
        }else if(strcmp(argv[1],"parse")==0){
            char path[2048];
            char* p;
            p=strtok(argv[2],"=");
            p=strtok(NULL,"=");
            if(p!=NULL)strcpy(path,p);
            else{
                printf("ERROR\ninvalid directory path\n");
                return 1;
            }
            parseFile(path,1,-1,NULL);
        }else if(strcmp(argv[1],"extract")==0){
            char path[2048];
            int section_nr;
            int line;
            for(int i=2;i<argc;i++){
                char* p;
                p=strtok(argv[i],"=");
                if(strcmp(p,"path")==0){
                    p=strtok(NULL,"=");
                    if(p!=NULL)strcpy(path,p);
                    else{
                        printf("ERROR\ninvalid directory path\n");
                        return 1;
                    } 
                 }else if(strcmp(p,"section")==0){
                    p=strtok(NULL,"=");
                    if(p!=NULL){
                        section_nr=atoi(p);
                    }else{
                        printf("ERROR\ninvalid section");
                        return 1;
                    } 
                                
                }else if(strcmp(p,"line")==0){
                    p=strtok(NULL,"=");
                    if(p!=NULL){
                        line=atoi(p);
                    }else{
                        printf("ERROR\ninvalid line");
                        return 1;
                    }
                }
            }
            extract(path,section_nr,line);
        }else if(strcmp(argv[1],"findall")==0){
            char* p;
            char path[2048];
            p=strtok(argv[2],"=");
            if(strcmp(p,"path")==0){
                p=strtok(NULL,"=");
                if(p!=NULL)strcpy(path,p);
                else{
                    printf("ERROR\ninvalid directory path\n");
                    return 1;
                } 
            }else{
                printf("ERROR\ninvalid directory path");
                return 1;
            }
            findAll(path,path);
        }

    }

    return 0;
}
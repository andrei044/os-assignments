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

int parseFile(const char* path,int afis){
    int fd;
    char magic[5];
    char header_size[3];
    char version;
    char no_of_sections;
    char sect_name[8];
    int sect_type;
    int sect_offset;
    int sect_size;
    section sections[2048];
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
    //printf("%s\n",magic);
    if(strcmp(magic,"6a4t")!=0){
        if(afis==1)printf("ERROR\nwrong magic");
        close(fd);
        return 1;
    }

    //read header_size
    if(read(fd, header_size, 2) != 2) {
        if(afis==1)printf("ERROR\nCould not read header_size");
        close(fd);
        return 1;
    }

    //version
    if(read(fd, &version, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read version");
        close(fd);
        return 1;
    }
    if(version<52 || version>86){
        if(afis==1)printf("ERROR\nwrong version");
        close(fd);
        return 1;
    }

    //no_of_sections
    if(read(fd, &no_of_sections, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read no_of_sections");
        close(fd);
        return 1;
    }
    if(no_of_sections<6 || no_of_sections>14){
        if(afis==1)printf("ERROR\nwrong sect_nr");
        close(fd);
        return 1;
    }
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, sect_name, 7) != 7) {
            if(afis==1)printf("ERROR\nCould not read sect_name");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_type, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_type");
            close(fd);
            return 1;
        }
        if(sect_type!=13 && sect_type!=52 && sect_type!=51 && sect_type!=11 && sect_type!=91 && sect_type!=29){
            if(afis==1)printf("ERROR\nwrong sect_types");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_offset, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_offset");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_size, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_size");
            close(fd);
            return 1;
        }
        strcpy(sections[i].sect_name,sect_name);
        sections[i].sect_type=sect_type;
        sections[i].sect_offset=sect_offset;
        sections[i].sect_size=sect_size;
    }
    if(afis==1)printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,no_of_sections);
    for(int i=0;i<no_of_sections;i++){
        if(afis==1)printf("section%d: %s %d %d\n",i+1,sections[i].sect_name,sections[i].sect_type,sections[i].sect_size);
    }
    close(fd);
    return 0;
}

int extract(const char* path, const int section_nr, const int line,int afis){
    int fd;
    char magic[5];
    char header_size[3];
    char version;
    char no_of_sections;
    char sect_name[8];
    int sect_type;
    int sect_offset;
    int sect_size;
    fd=open(path,O_RDONLY);
    if(fd==-1){
        if(afis==1)printf("ERROR\ninvalid file");
        return 1;
    }
    //read magic
    if(read(fd, magic, 4) != 4) {
        if(afis==1)printf("ERROR\nCould not read magic");
        close(fd);
        return 1;
    }
    //printf("%s\n",magic);
    if(strcmp(magic,"6a4t")!=0){
        if(afis==1)printf("ERROR\nwrong magic");
        close(fd);
        return 1;
    }

    //read header_size
    if(read(fd, header_size, 2) != 2) {
        if(afis==1)printf("ERROR\nCould not read header_size");
        close(fd);
        return 1;
    }

    //version
    if(read(fd, &version, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read version");
        close(fd);
        return 1;
    }
    if(version<52 || version>86){
        if(afis==1)printf("ERROR\ninvalid file");
        close(fd);
        return 1;
    }

    //no_of_sections
    if(read(fd, &no_of_sections, 1) != 1) {
        if(afis==1)printf("ERROR\nCould not read no_of_sections");
        close(fd);
        return 1;
    }
    if(no_of_sections<6 || no_of_sections>14){
        if(afis==1)printf("ERROR\nwrong sect_nr");
        close(fd);
        return 1;
    }
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, sect_name, 7) != 7) {
            if(afis==1)printf("ERROR\nCould not read sect_name");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_type, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_type");
            close(fd);
            return 1;
        }
        if(sect_type!=13 && sect_type!=52 && sect_type!=51 && sect_type!=11 && sect_type!=91 && sect_type!=29){
            if(afis==1)printf("ERROR\nwrong sect_types");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_offset, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_offset");
            close(fd);
            return 1;
        }
        if(read(fd, &sect_size, 4) != 4) {
            if(afis==1)printf("ERROR\nCould not read sect_size");
            close(fd);
            return 1;
        }
        if(i+1==section_nr){
            break;
        }
    }
    printf("%d+%d",atoi(header_size),sect_offset);
    lseek(fd,atoi(header_size)+sect_offset,SEEK_SET);
    int line_nr=1;
    char c;
    off_t j=-2;
    printf("SUCCESS\n");
    while(line_nr<=line){
        if(read(fd,&c,1)!=1){
            printf("ERROR\ninvalid file");
            close(fd);
            return 1;
        }
        if(c=='\n')line_nr++;
    }
    do{
        lseek(fd,j,SEEK_CUR);
        if(read(fd,&c,1)!=1){
            printf("ERROR\ninvalid file");
            close(fd);
            return 1;
        }
        printf("%c",c);
        j--;
    }while(c!='\n');
    close(fd);
    return 0;
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
            parseFile(path,1);
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
            extract(path,section_nr,line,0);
        }

    }

    return 0;
}
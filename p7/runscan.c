#include "ext2_fs.h"
#include "read_ext2.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

int main(int argc, char **argv) 
{
    if (argc != 3) 
    {
        printf("expected usage: ./runscan inputfile outputfile\n");
        exit(0);
    }

    /* This is some boilerplate code to help you get started, feel free to modify
       as needed! */

    int fd;
    fd = open(argv[1], O_RDONLY);    /* open disk image */

    ext2_read_init(fd);

    struct ext2_super_block super;
    struct ext2_group_desc group;

    struct ext2_inode inode;

    // example read first the super-block and group-descriptor
    read_super_block(fd, 0, &super);
    read_group_desc(fd, 0, &group);

    unsigned int inode_no = 0;

    printf("Inode Table %ld\n", locate_inode_table(0, &group));
    printf("Data Blocks %ld\n", locate_data_blocks(0, &group));

    for(; inode_no < super.s_inodes_per_group; inode_no++){
        read_inode(fd, 124*1024, inode_no, &inode);
        if (S_ISDIR(inode.i_mode)) {
            printf("directory\n");
        }
        else if (S_ISREG(inode.i_mode)) {
            printf("regular file %d\n",inode_no);
            char buffer[1024];
            lseek(fd, inode.i_block[0]*1024, SEEK_SET);
            read(fd, buffer, sizeof(buffer));
            bool isJPG = 0; 

            if (buffer[0] == (char)0xff &&
                buffer[1] == (char)0xd8 &&
                buffer[2] == (char)0xff &&
                (buffer[3] == (char)0xe0 ||
                buffer[3] == (char)0xe1 ||
                buffer[3] == (char)0xe8)) {
                isJPG = 1;
            }
            
            if(isJPG){
                printf("JPEG\n");

                char inodeString[10];
                snprintf(inodeString, sizeof(inodeString), "%d", inode_no);
                char *outputFile = malloc((strlen(argv[2])+strlen(inodeString)+1+strlen("/file-")) * sizeof(char));
                strcat(outputFile, argv[2]);
                strcat(outputFile, ("/file-"));
                strcat(outputFile, inodeString);
                strcat(outputFile, ".jpg");

                int outputFD;
                if ((outputFD = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
                    #if debug
                        printf("Output open failed\n");
                    #endif
                    exit(EXIT_FAILURE);
                }
                unsigned int fileSize = inode.i_size;
                for(unsigned int j = 0; inode.i_block[j] != 0 && j < 15; j++) {
                    unsigned int loopIter;

                    if(j == 12) {
                        int indirectBuffer1_p[256];
                        
                        lseek(fd, inode.i_block[j] * 1024, SEEK_SET);
                        read(fd, indirectBuffer1_p, sizeof(indirectBuffer1_p));

                        for(unsigned int i = 0; i < 256 && indirectBuffer1_p[i] != 0; i ++) {
                            char indirectBuffer1_d[1024];
                            loopIter = (fileSize > 1024)? 1024 : fileSize;

                            lseek(fd, indirectBuffer1_p[i] * 1024, SEEK_SET);
                            read(fd, indirectBuffer1_d, sizeof(indirectBuffer1_d));

                            for(unsigned int k = 0; k < loopIter; k++) {
                                if (write(outputFD, (void *)&indirectBuffer1_d[k], sizeof(char)) == -1) {
                                    #if debug
                                        printf("Write to output file failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }
                            }
                            fileSize -= loopIter;
                        }
                    } else if(j == 13) {
                        int indirectBuffer1_p[256];
                        
                        lseek(fd, inode.i_block[j] * 1024, SEEK_SET);
                        read(fd, indirectBuffer1_p, sizeof(indirectBuffer1_p));

                        for(unsigned int i = 0; i < 256 && indirectBuffer1_p[i] != 0; i ++) {
                            int indirectBuffer2_p[256];

                            lseek(fd, indirectBuffer1_p[i] * 1024, SEEK_SET);
                            read(fd, indirectBuffer2_p, sizeof(indirectBuffer2_p));

                            for(unsigned int k = 0; k < 256 && indirectBuffer2_p[k] != 0; k ++) {
                                char indirectBuffer2_d[1024];
                                loopIter = (fileSize > 1024)? 1024 : fileSize;

                                lseek(fd, indirectBuffer2_p[k] * 1024, SEEK_SET);
                                read(fd, indirectBuffer2_d, sizeof(indirectBuffer2_d));

                                for(unsigned int h = 0; h < loopIter; h++) {
                                    if (write(outputFD, (void *)&indirectBuffer2_d[h], sizeof(char)) == -1) {
                                        #if debug
                                            printf("Write to output file failed\n");
                                        #endif
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                fileSize -= loopIter;
                            }
                        }
                    } else {
                        loopIter = (fileSize > 1024)? 1024 : fileSize;

                        lseek(fd, inode.i_block[j] * 1024, SEEK_SET);
                        read(fd, buffer, sizeof(buffer));

                        for(unsigned int i = 0; i < loopIter; i++) {
                            if (write(outputFD, (void *)&buffer[i], sizeof(char)) == -1) {
                                #if debug
                                    printf("Write to output file failed\n");
                                #endif
                                exit(EXIT_FAILURE);
                            }
                        }
                        fileSize -= loopIter;
                    }
                }
            }
        }
        else {
            // printf("other file\n");
        }
    }

    return 0;
}
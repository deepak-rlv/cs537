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
                for(unsigned int i = 0; i < inode.i_size; i++) {
                    if (write(outputFD, (void *)&buffer[i], sizeof(char)) == -1) {
                        #if debug
                            printf("Write to output file failed\n");
                        #endif
                        exit(EXIT_FAILURE);
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
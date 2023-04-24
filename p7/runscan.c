#include "ext2_fs.h"
#include "read_ext2.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("expected usage: ./runscan inputfile outputfile1\n");
        exit(0);
    }

    /* This is some boilerplate code to help you get started, feel free to modify
       as needed! */

    int fd;
    fd = open(argv[1], O_RDONLY);    /* open disk image */

    int dirFD = mkdir (argv[2], 0777);
    if (dirFD == -1) {
        if (errno == EEXIST) {
            printf("Directory %s alread exists\n", argv[2]);
            exit(EXIT_FAILURE);
        }
    }

    ext2_read_init(fd);

    struct ext2_super_block super;
    struct ext2_group_desc group;

    struct ext2_inode inode;

    // example read first the super-block and group-descriptor
    read_super_block(fd, &super);
    read_group_descs(fd, &group, super.s_blocks_count / super.s_blocks_per_group);

    unsigned int inode_no = 0;
    unsigned int topSecretInode = -1;
    bool topSecretFolderLocated = 0;

    printf("Inode Table %ld\n", locate_inode_table(0, &group));
    printf("Data Blocks %ld\n", locate_data_blocks(0, &group));

    for (; inode_no < super.s_inodes_per_group; inode_no++) {
        read_inode(fd, group.bg_inode_table * BASE_OFFSET, inode_no, &inode, 256);
        if (S_ISDIR (inode.i_mode)) {
            // printf("directory\n");
            char directoryDBlock[BASE_OFFSET];

            if (topSecretInode == inode_no) {
                topSecretFolderLocated = 1;
                break;
            }

            for (unsigned int j = 0; /* inode.i_block[j] != 0 &&  */j < 14; j++) {
                lseek(fd, inode.i_block[j] * BASE_OFFSET, SEEK_SET);
                read(fd, directoryDBlock, sizeof(directoryDBlock));

                struct ext2_dir_entry *dentry;
                unsigned int recordLength;


                for (unsigned int i = 0; i < BASE_OFFSET; i += recordLength) {
                    dentry = (struct ext2_dir_entry*) & (directoryDBlock[i]);

                    if (dentry->inode == 0) {
                        break;
                    }

                    const int name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
                    if (name_len >= EXT2_NAME_LEN) {
                        printf("Entry name length greater than 255.\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    char name [name_len + 1];
                    strncpy(name, dentry->name, name_len);
                    name[name_len] = '\0';

                    if (strcmp("top_secret", name) == 0) {
                        topSecretInode = dentry->inode;
                    }
                    // printf("Entry name is -- %s -- %d --\n", name, dentry->inode);

                    recordLength = ((name_len % 4)? ((name_len / 4) + 1) * 4: name_len) + 8;
                }
            }
        }
    }

    inode_no = (topSecretFolderLocated)? topSecretInode: 0;

    for (; inode_no < super.s_inodes_per_group; inode_no++) {
        read_inode(fd, group.bg_inode_table * BASE_OFFSET, inode_no, &inode, 256);
        if (S_ISDIR (inode.i_mode)) {
            // printf("directory\n");
            char directoryDBlock[BASE_OFFSET];

            for (unsigned int j = 0; /* inode.i_block[j] != 0 && */ j < 14; j++) {
                lseek(fd, inode.i_block[j] * BASE_OFFSET, SEEK_SET);
                read(fd, directoryDBlock, sizeof(directoryDBlock));

                struct ext2_dir_entry *dentry;
                unsigned int recordLength;

                // if (topSecretInode == inode_no) {
                //     topSecretFolderLocated = 1;
                // }

                for (unsigned int i = 0; i < BASE_OFFSET; i += recordLength) {
                    dentry = (struct ext2_dir_entry*) & (directoryDBlock[i]);

                    if (dentry->inode == 0) {
                        break;
                    }

                    const int name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
                    if (name_len >= EXT2_NAME_LEN) {
                        printf("Entry name length greater than 255.\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    char name [name_len + 1];
                    strncpy(name, dentry->name, name_len);
                    name[name_len] = '\0';

                    // printf("Entry name is -- %s -- %d --\n", name, dentry->inode);

                    // if (strcmp("top_secret", name) == 0) {
                    //     topSecretInode = dentry->inode;
                    // }

                    recordLength = ((name_len % 4)? ((name_len / 4) + 1) * 4: name_len) + 8;

                    // if (topSecretFolderLocated) {
                        struct ext2_inode inode_regfile;
                        read_inode(fd, group.bg_inode_table * BASE_OFFSET, dentry->inode, &inode_regfile, 256);
                        if (S_ISREG(inode_regfile.i_mode) /* && topSecretFolderLocated */) {
                            // printf("regular file %d\n", inode_no);
                            char buffer[BASE_OFFSET];
                            lseek(fd, inode_regfile.i_block[0] * BASE_OFFSET, SEEK_SET);
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
                            
                            if (isJPG) {
                                // printf("JPEG\n");
                                char inodeString[10];
                                snprintf(inodeString, sizeof(inodeString), "%d", dentry->inode);
                                char *outputFile1 = malloc((strlen(argv[2]) + strlen(inodeString) + 1 + strlen("/file-") + 4) * sizeof(char));
                                strcat(outputFile1, argv[2]);
                                strcat(outputFile1, ("/file-"));
                                strcat(outputFile1, inodeString);
                                strcat(outputFile1, ".jpg");
                                strcat(outputFile1, "\0");
                                int outputFD1;
                                if ((outputFD1 = open(outputFile1, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
                                    #if debug
                                        printf("Output2 open failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }

                                char *outputFile2 = malloc((strlen(argv[2]) + strlen(name) + 1 + 1) * sizeof(char));
                                strcat(outputFile2, argv[2]);
                                strcat(outputFile2, "/");
                                strcat(outputFile2, name);
                                strcat(outputFile2, "\0");
                                int outputFD2;
                                if ((outputFD2 = open(outputFile2, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
                                    #if debug
                                        printf("Output2 open failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }

                                char *outputFile3 = malloc((strlen(argv[2]) + strlen(inodeString) + 1 + strlen("/file-") + 4 + strlen("-details")) * sizeof(char));
                                strcat(outputFile3, argv[2]);
                                strcat(outputFile3, ("/file-"));
                                strcat(outputFile3, inodeString);
                                strcat(outputFile3, ("-details"));
                                strcat(outputFile3, ".txt");
                                strcat(outputFile3, "\0");

                                int outputFD3;
                                if ((outputFD3 = open(outputFile3, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
                                    #if debug
                                        printf("Output3 open failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }

                                unsigned int fileSize = inode_regfile.i_size;
                                for (unsigned int j = 0; /* inode.i_block[j] != 0 && */ j < 14; j++) {
                                    /* if (inode.i_block[j] == 0) {
                                        continue;
                                    } */

                                    unsigned int loopIter;

                                    if (j == 12) {
                                        int indirectBuffer1_p[256];
                                        
                                        lseek(fd, inode_regfile.i_block[j] * BASE_OFFSET, SEEK_SET);
                                        read(fd, indirectBuffer1_p, sizeof(indirectBuffer1_p));

                                        for (unsigned int i = 0; i < 256 /* && indirectBuffer1_p[i] != 0 */; i++) {
                                            /* if (indirectBuffer1_p[i] == 0) {
                                                continue;
                                            } */

                                            char indirectBuffer1_d[BASE_OFFSET];
                                            loopIter = (fileSize > BASE_OFFSET)? BASE_OFFSET : fileSize;

                                            lseek(fd, indirectBuffer1_p[i] * BASE_OFFSET, SEEK_SET);
                                            read(fd, indirectBuffer1_d, sizeof(indirectBuffer1_d));

                                            for (unsigned int k = 0; k < loopIter; k++) {
                                                if (write(outputFD1, (void *)&indirectBuffer1_d[k], sizeof(char)) == -1) {
                                                    #if debug
                                                        printf("Write to output1 file failed\n");
                                                    #endif
                                                    exit(EXIT_FAILURE);
                                                }
                                                if (write(outputFD2, (void *)&indirectBuffer1_d[k], sizeof(char)) == -1) {
                                                    #if debug
                                                        printf("Write to output2 file failed\n");
                                                    #endif
                                                    exit(EXIT_FAILURE);
                                                }
                                            }
                                            fileSize -= loopIter;
                                        }
                                    } else if (j == 13) {
                                        int indirectBuffer1_p[256];
                                        
                                        lseek(fd, inode_regfile.i_block[j] * BASE_OFFSET, SEEK_SET);
                                        read(fd, indirectBuffer1_p, sizeof(indirectBuffer1_p));

                                        for (unsigned int i = 0; i < 256 /* && indirectBuffer1_p[i] != 0 */; i ++) {
                                            /* if (indirectBuffer1_p[i] == 0) {
                                                continue;
                                            } */

                                            int indirectBuffer2_p[256];

                                            lseek(fd, indirectBuffer1_p[i] * BASE_OFFSET, SEEK_SET);
                                            read(fd, indirectBuffer2_p, sizeof(indirectBuffer2_p));

                                            for (unsigned int k = 0; k < 256 /* && indirectBuffer2_p[k] != 0 */; k ++) {
                                                /* if (indirectBuffer2_p[i] == 0) {
                                                    continue;
                                                } */

                                                char indirectBuffer2_d[BASE_OFFSET];
                                                loopIter = (fileSize > BASE_OFFSET)? BASE_OFFSET : fileSize;

                                                lseek(fd, indirectBuffer2_p[k] * BASE_OFFSET, SEEK_SET);
                                                read(fd, indirectBuffer2_d, sizeof(indirectBuffer2_d));

                                                for (unsigned int h = 0; h < loopIter; h++) {
                                                    if (write(outputFD1, (void *)&indirectBuffer2_d[h], sizeof(char)) == -1) {
                                                        #if debug
                                                            printf("Write to output1 file failed\n");
                                                        #endif
                                                        exit(EXIT_FAILURE);
                                                    }
                                                    if (write(outputFD2, (void *)&indirectBuffer2_d[h], sizeof(char)) == -1) {
                                                        #if debug
                                                            printf("Write to output2 file failed\n");
                                                        #endif
                                                        exit(EXIT_FAILURE);
                                                    }
                                                }
                                                fileSize -= loopIter;
                                            }
                                        }
                                    } else {
                                        loopIter = (fileSize > BASE_OFFSET)? BASE_OFFSET : fileSize;

                                        lseek(fd, inode_regfile.i_block[j] * BASE_OFFSET, SEEK_SET);
                                        read(fd, buffer, sizeof(buffer));

                                        for (unsigned int i = 0; i < loopIter; i++) {
                                            if (write(outputFD1, (void *)&buffer[i], sizeof(char)) == -1) {
                                                #if debug
                                                    printf("Write to output1 file failed\n");
                                                #endif
                                                exit(EXIT_FAILURE);
                                            }
                                            if (write(outputFD2, (void *)&buffer[i], sizeof(char)) == -1) {
                                                #if debug
                                                    printf("Write to output2 file failed\n");
                                                #endif
                                                exit(EXIT_FAILURE);
                                            }
                                        }
                                        fileSize -= loopIter;
                                    }
                                }
                                char linksCount[6];
                                snprintf(linksCount, sizeof(linksCount), "%d", inode_regfile.i_links_count);
                                strcat(linksCount, "\n");
                                if (write(outputFD3, (void *)linksCount, strlen(linksCount)) == -1) {
                                    #if debug
                                        printf("Write to output3 file failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }
                                char size[11];
                                snprintf(size, sizeof(size), "%d", inode_regfile.i_size);
                                strcat(size, "\n");
                                if (write(outputFD3, (void *)size, strlen(size)) == -1) {
                                    #if debug
                                        printf("Write to output3 file failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }
                                char uid[6];
                                snprintf(uid, sizeof(uid), "%d", inode_regfile.i_uid);
                                strcat(uid, "\n");
                                if (write(outputFD3, (void *)uid, strlen(uid)) == -1) {
                                    #if debug
                                        printf("Write to output3 file failed\n");
                                    #endif
                                    exit(EXIT_FAILURE);
                                }
                            }
                        }
                    // }
                }
            }
        }
        if (inode_no == topSecretInode && topSecretFolderLocated)
            break;
    }
    return 0;
}
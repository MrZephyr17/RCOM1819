#pragma once

#include <unistd.h>

#define FRAG_K 260

/**
 * @brief
 *
 * @param argv
 * @return int
 */
int usage(char **argv);

/**
 * @brief
 *
 * @param fileLength
 * @return unsigned char*
 */
unsigned char *getTLVLength(int fileLength);

/**
 * @brief
 *
 * @param fileName
 * @param stringLength
 * @return unsigned char*
 */
unsigned char *getTLVName(char *fileName, int stringLength);

/**
 * @brief Get the Delim Package object
 *
 * @param C
 * @param fileLength
 * @param fileName
 * @param stringLength
 * @param size
 * @return unsigned char*
 */
unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName,
                               int stringLength, int *size);

/**
 * @brief
 *
 * @param fileName
 * @param size
 * @return unsigned char*
 */
unsigned char *readFile(const char *fileName, off_t *size);

/**
 * @brief Get the Fragment object
 *
 * @param seqNum
 * @param data
 * @param K
 * @return unsigned char*
 */
unsigned char *getFragment(int seqNum, unsigned char *data, int K);

/**
 * @brief
 *
 * @param fd
 * @param filename
 */
void writeFile(int fd, char *filename, int messageSize);

int processTestArgument(char **argv);

int transferFile(char *fileName, char *port);
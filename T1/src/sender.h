#pragma once

#include <unistd.h>

#define FRAG_K 260

/**
 * @brief Prints the program correct usage and gives one example.
 *
 * @param argv : command line arguments
 * @return int
 */
int usage(char **argv);

/**
 * @brief Creates TLV array that saves file's lenght.
 *
 * @param fileLength : file's length to save in TLV array
 * @return unsigned char* : TLV array
 */
unsigned char *getTLVLength(int fileLength);

/**
 * @brief Creates TLV array that saves file's name.
 *
 * @param fileName : file's name to sabe in TLV array
 * @param stringLength : length of file's name
 * @return unsigned char* : TLV array
 */
unsigned char *getTLVName(char *fileName, int stringLength);

/**
 * @brief Get the Delim Package object
 *        The Delim Package contains TLV arrays with information of the file's name and lenght
 *        generated by the getTLVLenght and getTLVName functions.
 *
 *
 * @param C : Control field that indicates if it's a Start or an End package (START_C or END_C)
 * @param fileLength : File's lenght to save in the package
 * @param fileName : File's name to save in the package
 * @param stringLength : Length of file's name
 * @param size : Variable that will be updated with the package size
 * @return unsigned char* : Delim Package
 */
unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName,
                               int stringLength, int *size);

/**
 * @brief Reads the file with name 'fileName' and saves ist data and its size
 *
 * @param fileName : Name of the file to read
 * @param size : Variable that will be set with file's size
 * @return unsigned char* : file's data
 */
unsigned char *readFile(const char *fileName, off_t *size);

/**
 * @brief Get the Fragment object
 *        Generates one Data Package (Fragment) with the received sequence number,
 *        the data to be saved and number of bytes of the data to be saved
 *
 * @param seqNum : sequence number of th Fragment
 * @param data : Data to be saved at the Fragment
 * @param K : Number of bytes of the Data that are stored in each Fragment
 * @return unsigned char* : Fragment
 */
unsigned char *getFragment(int seqNum, unsigned char *data, int K);

/**
 * @brief Sends the Start Delim Package
 *        Sends Fragments/Data Packages containing parts of the file (named filename) data.
 *        Sends the End Delim Package to end the File transmition.
 *        All the Packages are sent trough the Serial Port (with descriptor fd) using llwrite function.
 *        Fragments are created after reading the file data and using the getFragment function.
 *
 *
 * @param fd : Serial Port descriptor
 * @param filename : Name of the file to send
 */
void writeFile(int fd, char *filename, int messageSize);

int processTestArgument(char **argv);

int transferFile(char *fileName, char *port);
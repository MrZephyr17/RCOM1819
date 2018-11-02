#pragma once

#include <stdbool.h>
#include <stdio.h>

void delay_alarm_handler();

/**
 * @brief Prints the program correct usage and gives one example.
 * 
 * @param argv 
 * @return int 
 */
int usage(char **argv);

/**
 * @brief Receives a Delim Package and get and save the file's information it contains
 * 
 * @param data : Delim Package
 * @param filename : Variable that will be set with the file's name saved at the received Delim Package
 */
void handleStart(unsigned char *data, unsigned char *filename);

/**
 * @brief Receives a Fragment/Data Package and gets the data it contains (K bytes of data)
 *        to save it at the file named 'filename'
 *
 * @param data : Fragment/Data Package
 * @param file : file where the Fragment's data will be saved
 * @return true 
 * @return false 
 */
bool handleData(unsigned char *data, FILE *file);

/**
 * @brief Reads the Start Delim Package and uses its information to create a file (using handleStart).
 *        Reads the Fragments sent by the sender and saves the data they contain at the created file (using handleData).
 *        Reads the End Delim.
 *        Read all the Packages from the Serial Port (with descriptor fd) using llread function.
 * 
 * @param fd : Serial Port Descriptor
 */
void readFile(int fd);

int receiveFile(char *port);

int processTestArgument(char **argv);

void setUpDelayAlarmHandler();

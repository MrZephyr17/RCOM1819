#pragma once

#include <stdbool.h>

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
 * @param data 
 * @param filename 
 */
void handleStart(unsigned char *data, unsigned char *filename);

/**
 * @brief 
 * 
 * @param data 
 * @param file 
 * @return true 
 * @return false 
 */
bool handleData(unsigned char *data, FILE *file);

/**
 * @brief 
 * 
 * @param fd 
 */
void readFile(int fd);


#pragma once

#include <stdbool.h>
#include <termios.h>

typedef enum
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC1_OK,
    DATA_RCV,
    BCC2_OK,
    END
} state_t;

#define TRANSMITTER 0
#define RECEIVER 1

#define FLAG 0x7E
#define ESC 0x7D
#define ESC_2 0x5E
#define ESC_3 0x5D
#define A_03 0x03
#define A_01 0x01
#define SET_C 0x03
#define UA_C 0x07
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81
#define C_I0 0x00
#define C_I1 0x40
#define START_C 0x02
#define F_C 0x01
#define END_C 0x03
#define T_LENGTH 0x00
#define T_NAME 0x01
#define DISC 0x0B

#define TIME_OUT 3
#define MAX_RETRY_NUMBER 3

#define BAUDRATE B38400
#define SUPERVISION_SIZE 5
#define PORT_SIZE 11

/**
 * @brief Alarm Handler, function that is called when an alarm signal is received
 * 
 */
void alarm_handler();

/**
 * @brief Setup timeout alarm handler
 * 
 */
void setUpAlarmHandler();

/**
 * @brief Opens the conection between the two computers trought the serial port (associated to descripor fd).
 *        If flag equals TRANSMITTER calls llopen_transmitter to open the connection in the sender program.
 *        If flag equals RECEIVER calls llopen_receiver to open the connection in the receiver program.
 *
 * @param flag : Indicates if the connetcion is done in the receiver or in the sender program/computer (TRANSMITTER/SENDER)
 * @param fd : Descriptor of the serial port, used to send the supervision messages to establish connection.
 * @return int : 0 if sucessfull, -1 if reveives a non existent flag.
 */
int llopen(int flag, int fd);

/**
 * @brief Send a SET message to receiver through the serial port (using write and fd descriptor).
 *        Install alarm handler, activated in periods of TIME_OUT seconds.
 *        Waits TIME_OUT seconds to receive a UA_C message from receiver, readinf the serial port (using fd descriptor).
 *        If UA_C is not received in TIME_OUT seconds, send another SET message and waits again.
 *        If SET is sent MAX_RETRY_NUMBER times without success, the connection fails, and the function returns -1.
 *
 * @param fd : Serial port descriptor
 * @return int : 0 if connection is established successfully, -1 otherwise
 */
int llopen_transmitter(int fd);

/**
 * @brief Try to read SET message from sender, reading the serial port.
 *        If a SET message is received, indicating the sender wants to establish a connection,
 *        it responds sending a UA_C message through the serial port, indicating the receiver is ready.
 * 
 * @param fd : Serial port descriptor
 * @return int : 0 if receives SET message and sends UA_C successfully, -1 otherwise
 */
int llopen_receiver(int fd);

/**
 * @brief Sets up serial port communication
 * 
 * @param port 
 * @param fd 
 * @param oldtio 
 */
void setUpPort(int port, int *fd, struct termios *oldtio, speed_t baudrate);

/**
 * @brief Writes a Package as an Information message in the Serial Port (using its descriptor fd).
 *        Receives an application Package, applies the stuffing function to it and 
 *        uses it to create the Information message with calcFinalMessage function.
 *        After the Information is sent to the receiver, it waits for its answer indicating
 *        if the Informationmessage was correct and well received.
 * 
 * @param fd : Serial Port descriptor
 * @param buffer : Package to send
 * @param length :Package's length
 * @return int : 0 if Information message was successfully sent, -1 otherwise
 */
int llwrite(int fd, unsigned char *buffer, int length);

/**
 * @brief Receives original Fragment and returns it stuffed
 *
 * @param data : non stuffed Fragment
 * @param BCC2 : Fragments's size
 * @param dataSize : Fragments's size
 * @param size : Variable to be set with stuffed fragment's size
 * @return unsigned char* : Stuffed Fragment
 */
unsigned char *stuffing(unsigned char *data, unsigned char BCC2, int dataSize, int *size);

/**
 * @brief Calculates BBC2 that protects the Fragment of data that will be saved at the information message.
 * 
 * @param data : Fragment
 * @param size : Fragment's size
 * @return unsigned char : BBC2
 */
unsigned char calcBCC2(unsigned char *data, int size);

/**
 * @brief Creates Information Message receiving the Fragment/Data Package generated at the application
 *        level and the BBC2 calculated previously
 * 
 * @param data : Fragment/Data Package
 * @param size : Fragment's size
 * @return unsigned char* : Information message
 */
unsigned char *calcFinalMessage(unsigned char *data, int size);

/**
 * @brief Reads an Information Message from the Serial Port (using fd descriptor).
 * 
 * @param fd : Serial Port descriptor
 * @param buffer
 * @return int : -2 if reads an END message, -1 in case of invalid state
 */
int llread(int fd, unsigned char *buffer);

/**
 * @brief Receives rec_BCC2 and verifies if it's equal the BBC2 of the received Fragment/Data Package.
 *            
 *      
 * @param rec_BCC2 
 * @param data : Fragment/Data Package
 * @param size : Fragment's size
 * @return true : if equals
 * @return false : if not equals
 */
bool checkBCC2(unsigned char rec_BCC2, unsigned char *data, int size);

/**
 * @brief Receives the Fragment/Data Package, does it .
 *        When a FLAG is received it checks the Fragment's BCC2 and the duplicates,
 *        responging with a signal according to it: if everything is correct it sends
 *        RR0 or RR1, otherwise it sends REJ0 or REJ1.
 * 
 * @param fd
 * @param buf
 * @param data 
 * @param i 
 * @param state 
 * @param wait 
 */
void receiveData(int fd, unsigned char buf, unsigned char *data, int *i, state_t *state, bool *wait);

/**
 * @brief Verifies if an Information message is being received, trying to read it from the Serial Port (using fd).
 *        Saves its data (Fragment) at the variable data (using receiveData function).
 *        Saves the data's size at the variable size.
 *
 * @param fd : Serial Port descriptor
 * @param size 
 * @return unsigned char* 
 */
int receiveIMessage(int fd, int *size, unsigned char* data);


/**
 * @brief Ends the connection between the two computers/programs (sender and receiver) through the Serial Port.
 *        If flag equals TRANSMITTER calls llclose_transmitter to close the connection in the sender program.
 *        If flag equals RECEIVER calls llclose_receiver to close the connection in the receiver program.
 *
 * 
 * @param fd : Serial Port descriptor
 * @param flag : Indicates if the conn
 * @brief etcion is closed in the receiver or in the sender program/computer (TRANSMITTER/SENDER)
 * @return int : 0 if sucessfull, -1 if reveives a non existent flag
 */
int llclose(int fd, int flag);

/**
 * @brief Try to read DISC message (sent by the llclose_transmitter) from the Serial Port using fd.
 *        If it receives the DISC message, sends a DISC message to the transmitter as answer trought the Serial Port.
 *        After sending the DISC, waits for a UA_C from the transmitter, reading the Serial Port again.
 *
 * @param fd : Serial Port Descriptor
 * @return int : 0 if messages are received/sent and the connection closes successfully, -1 otherwise
 */
int llclose_receiver(int fd);

/**
 * @brief Sends a DISC message to the transmitter trhough the Serial Port (using fd) to close the conection.
 *        Waits TIME_OUT seconds for DISC answer from the llclose_receiver-
 *        If doesn't receive the DISC answer after this time, send another DISC message.
 *        If doesn't receive an answer after trying to senD the DISC message MAX_RETRY_NUMBER times, returns -1.
 *        If receives the answer, sends an UA_C as answer to the receiver.

 * @param fd : SetialPort descriptor
 * @return int : 0 if messages are received/sent and the connection closes successfully, -1 otherwise
 */
int llclose_transmitter(int fd);

/**
 * @brief Check if a specific supervision message is received: with fields A and C specified in the parameters.
 *        Try to read the message from the Serial Port using fd.
 *        Verifies if the message readed is the specified one and if it's correct using stateMachineSupervisionMessage.
 *
 * @param fd : SerialPort descriptor. 
 * @param A : Address field: A_01 or A_03
 * @param C : Control field: message type
 * @return int : 1 if the specified Supervision message is received
 */
int receiveSupervisionMessage(int fd, unsigned char A, unsigned char C);

/**
 * @brief Sends a supervision message with all fields needed: {FLAG, A, C, A^C, FLAG}.
 * 
 * @param fd : Serial Port descriptor
 * @param A : A_01 if the sender sends a command and the receiver sends the answer to it, A_03 otherwise
 * @param C : Defines the message type (SET, UA, ...)
 * @return int 
 */
int sendSupervisionMessage(int fd, unsigned char A, unsigned char C);

/**
 * @brief Receives a byte (field) of the supervision message readed from Serial Port (buf).
 *        Changes the state parameter denpending on the previous state received, 
 *        the field readed from Serial Port and the A and C values.
 *        If state is set with END, a supervision message with A and C values was readed.
 *        
 * 
 * @param state : Current state, will be uptaded
 * @param buf : Byte readed previously from Serial Port
 * @param A : Address field
 * @param C : Control field
 * @param COptions 
 * @return int : -1 if receives an invalid state
 */
int stateMachineSupervisionMessage(state_t *state, unsigned char buf,
                                   unsigned char A, unsigned char *C,
                                   unsigned char *COptions);

/**
 * @brief Sets the original/old attributes of the Serial Port and closes it using its descriptor.
 * 
 * @param fd : Serial Port descriptor
 * @param oldtio : termios with Serial Port original (old) attributes
 */
void closeFd(int fd, struct termios *oldtio);
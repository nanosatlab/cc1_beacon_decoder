#ifndef __BEACON_3CAT1_H__
#define __BEACON_3CAT1_H__

/*** INCLUDE SECTION ********************************************************************************/
// C standard libraries:
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

/*** GLOBAL CONSTANTS *******************************************************************************/
#define BEACON_SENSOR_DELTA_T   (60 * 2)                    // In seconds.
#define MAX_BEACON_DATA         ( (255 - 6 - 32) / 3 + 5 )  // Max. possible amount of data elements in beacon packet.


// Sensors constants:
#define     TEMP_SENSORS        7   // Number of temperature sensors from the EPS
#define     SOC_SENSORS         1   // Number of state of charge sensors
#define     VOLT_SENSORS        7   // Number of voltage sensors
#define     CUR_SENSORS         6   // Number of current sensors
#define     IRR_SENSORS         6   // Number of irradiance sensors
#define     RAD_SENSORS         1   // Number of radiation sensors
#define     ATT_SENSORS         1   // Number of attitude sensors


// Beacon types (these values should not clash with other identifiers in the Comms protocol.
// See HWmod/subsystem_comms/comms.h for the full list. */
#define     BEACON_TYPE_STATE       0xB0
#define     BEACON_TYPE_VOLT        0xB1
#define     BEACON_TYPE_CURR        0xB2
#define     BEACON_TYPE_TEMP        0xB3
#define     BEACON_TYPE_IRR         0xB4
#define     BEACON_TYPE_SOC         0xB5

// Error codes:
// -- Common: reserved from 0 to 29
#define     NOERROR      0      // Succesful operation.
#define     EUNDEF      -1      // Undefined error.
#define     EMKFIFO     -2      // Error creating the FIFO's
#define     EOPEN       -3      // Error opening a file descriptor.
#define     EMUTEX      -4      // Tried to accuire a shared resources which is not available.
#define     ERWSYSCORE  -5      // Error while reading or writing to Syscore FD.
#define     ERWPROCMAN  -6      // Error while reading or writing to Procman FD.
#define     ERWSDB      -7      // Error while reading or writing to SDB FD.
#define     ERWHWMOD    -8      // Error while reading or writing to a HWmod FD.
#define     EPROG       -9      // Programming error (e.g. Segmentation Fault catched)
#define     ESOFT       -10     // Error during operation. The error is spontaneous and/or repairable.
#define     EWARNING    -11     // No error found, but a probably unexpected behaviour occured.
#define     EFIFO       -12     // Error reading, writing, creating, opening or closing a FIFO.
#define     ECONFIG     -13     // Error in a .conf file.
#define     EXENOMAI    -14     // An error ocurred during Xenomai start or program execution.
#define     ETIMEOUT    -15     // A timeout has occurred.
#define     EWRONGSTATE -16     // A wrong state or condition is reached.


/*** MACROS *****************************************************************************************/

/*** TYPEDEFS ***************************************************************************************/

/*** GLOBAL VARIABLES *******************************************************************************/

/*** FUNCTIONS **************************************************************************************/
int parse_beacon(unsigned char *buf, size_t size, unsigned char *beacon_type, time_t *sct, unsigned char *soc, unsigned char *sensor_id, unsigned int *ts, float *vs);
const char * beacon_type_str(unsigned char id);

#endif

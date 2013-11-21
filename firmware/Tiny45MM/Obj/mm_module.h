
/*
 ******************************************************************************
 * mm_module.h
 * Maerklin motorola include file 
 ******************************************************************************
 */

/*
 ******************************************************************************
 * Standard include files
 ******************************************************************************
 */
#include <inttypes.h>

/*
 *******************************************************************************************
 * Macro 
 *******************************************************************************************
  */

/*
 *******************************************************************************************
 * Types
 *******************************************************************************************
  */
typedef enum
{
   MM_NO_NEW_INFO = 0,
   MM_NEW_INFO
} tMMInfo;

/* 
 *******************************************************************************************
 * Variables
 *******************************************************************************************
 */

/*
 *******************************************************************************************
 * Prototypes
 *******************************************************************************************
 */

extern void         MM_Module_Init(void);
extern tMMInfo      MM_CheckForNewInfo(uint8_t * Address, uint8_t * Info);
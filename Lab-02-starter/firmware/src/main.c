/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name: 
    main.c

  Summary:
    This file contains the "main" function for a project. It is intended to
    be used as the starting point for CISC-211 Curiosity Nano Board
    programming projects. After initializing the hardware, it will
    go into a 0.5s loop that calls an assembly function specified in a separate
    .s file. It will print the iteration number and the result of the assembly 
    function call to the serial port.
    As an added bonus, it will toggle the LED on each iteration
    to provide feedback that the code is actually running.
  
    NOTE: PC serial port MUST be set to 115200 rate.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <malloc.h>
#include "definitions.h"                // SYS function prototypes

/* RTC Time period match values for input clock of 1 KHz */
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096

#define MAX_PRINT_LEN 400

static volatile bool isRTCExpired = false;
static volatile bool changeTempSamplingRate = false;
static volatile bool isUSARTTxComplete = true;
static uint8_t uartTxBuffer[MAX_PRINT_LEN] = {0};

// Test cases for testing func that adds two numbers
static int32_t inp1Array[] = {  1, 24,  3, -100};
static int32_t inp2Array[] = {  7, 12, 39,    1};

static char * pass = "PASS";
static char * fail = "FAIL";

// VB COMMENT:
// The ARM calling convention permits the use of up to 4 registers, r0-r3
// to pass data into a function. Only one value can be returned to the 
// C caller. The assembly language routine stores the return value
// in r0. The C compiler will automatically use it as the function's return
// value.
//
// Function signature
extern int32_t asmFunc(int32_t, int32_t);

// make the variable nameStrPtr available to the C program
extern uint32_t nameStrPtr;

// set this to 0 if using the simulator. BUT note that the simulator
// does NOT support the UART, so there's no way to print output.
#define USING_HW 1

#if USING_HW
static void rtcEventHandler (RTC_TIMER32_INT_MASK intCause, uintptr_t context)
{
    if (intCause & RTC_MODE0_INTENSET_CMP0_Msk)
    {            
        isRTCExpired    = true;
    }
}
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUSARTTxComplete = true;
    }
}
#endif


// return failure count. A return value of 0 means everything passed.
static int testResult(int testNum, 
                      int32_t testInp1, 
                      int32_t testInp2, 
                      int32_t asmResult,
                      int32_t *passCount,
                      int32_t *failCount)
{
    // for this lab, each test case corresponds to a single pass or fail
    // But for future labs, one test case may have multiple pass/fail criteria
    // So I'm setting it up this way so it'll work for future labs, too --VB
    *failCount = 0;
    *passCount = 0;
    char *s1 = "OOPS";
    static char *s2 = "OOPS";
    static bool firstTime = true;
    int32_t correctAnswer = testInp1 + testInp2;
    if (asmResult != correctAnswer)
    {
        s1 = fail;  // assign the failure string to s1
        *failCount += 1;  // increment the failure count
    }
    else
    {
        s1 = pass;  // assign the pass string to s1
        *passCount += 1;  // increment the pass count
    }
    
    /* only check the string the first time through */
    if (firstTime == true)
    {
        /* Now check the string */
        int strTest = strcmp((char *)nameStrPtr, 
                             "Hello. My name is Inigo Montoya.");
        if (strTest == 0) // Make sure it changed! 0 means strs are equal
        {
            s2 = fail;  // assign the failure string to s1
            *failCount += 1;  // increment the failure count
        }
        else
        {
            s2 = pass;  // assign the pass string to s1
            *passCount += 1;  // increment the pass count
        }
        firstTime = false; // don't check the strings for subsequent test cases
    }
           
    // build the string to be sent out over the serial lines
    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= Test Number: %d\r\n"
            "test input 1:    %8ld\r\n"
            "test input 2:    %8ld\r\n"
            "expected result: %8ld\r\n"
            "actual result:   %8ld\r\n"
            "pass/fail:       %s\r\n\r\n"
            "modified name string: %s\r\n"
            "string test result:   %s\r\n"
            "\r\n",
            testNum,
            testInp1,
            testInp2,
            correctAnswer,
            asmResult,
            s1,
            (char *)nameStrPtr,
            s2);

#if USING_HW 
    // send the string over the serial bus using the UART
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, \
        (const void *)&(SERCOM5_REGS->USART_INT.SERCOM_DATA), \
        strlen((const char*)uartTxBuffer));
#endif

    return *failCount;
    
}



// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    
 
#if USING_HW
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Compare0Set(PERIOD_500MS);
    RTC_Timer32CounterSet(0);
    RTC_Timer32Start();
#else // using the simulator
    isRTCExpired = true;
    isUSARTTxComplete = true;
#endif //SIMULATOR

    // initialize all the variables
    int32_t inp1 = 0;
    int32_t inp2 = 0;
    int32_t result = 0;
    int32_t passCount = 0;
    int32_t failCount = 0;
    int32_t totalPassCount = 0;
    int32_t totalFailCount = 0;
    uint32_t numTestCases = sizeof(inp1Array)/sizeof(inp1Array[0]);
    
    // Loop forever
    while ( true )
    {
        // Do the tests
        for (int testCase = 0; testCase < numTestCases; ++testCase)
        {
            // Toggle the LED to show we're running a new test case
            LED0_Toggle();

            // reset the state variables for the timer and serial port funcs
            isRTCExpired = false;
            isUSARTTxComplete = false;
            
            // set the input values
            inp1 = inp1Array[testCase];
            inp2 = inp2Array[testCase];

            // !!!! THIS IS WHERE YOUR ASSEMBLY LANGUAGE PROGRAM GETS CALLED!!!!
            // Call our assembly function defined in file asmFunc.s
            result = asmFunc(inp1, inp2);
            
            // test the result and see if it passed
            failCount = testResult(testCase,inp1,inp2,result,
                                   &passCount,&failCount);
            totalPassCount = totalPassCount + passCount;
            totalFailCount = totalFailCount + failCount;

#if USING_HW
            // spin here until the UART has completed transmission
            // and the timer has expired
            //while  (false == isUSARTTxComplete ); 
            while ((isRTCExpired == false) ||
                   (isUSARTTxComplete == false));
#endif

        } // for each test case
        
        // When all test cases are complete, print the pass/fail statistics
        // Keep looping so that students can see code is still running.
        // We do this in case there are very few tests and they don't have the
        // terminal hooked up in time.
        uint32_t idleCount = 1;
        uint32_t totalTests = totalPassCount + totalFailCount;
        bool firstTime = true;
        while(true)      // post-test forever loop
        {
            isRTCExpired = false;
            isUSARTTxComplete = false;
            snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
                    "========= Post-test Idle Cycle Number: %ld\r\n"
                    "Summary of tests: %ld of %ld tests passed\r\n"
                    "\r\n",
                    idleCount, totalPassCount, totalTests); 

#if USING_HW 
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, \
                (const void *)&(SERCOM5_REGS->USART_INT.SERCOM_DATA), \
                strlen((const char*)uartTxBuffer));
            // spin here, waiting for timer and UART to complete
            LED0_Toggle();
            ++idleCount;

            while ((isRTCExpired == false) ||
                   (isUSARTTxComplete == false));

            // slow down the blink rate after the tests have been executed
            if (firstTime == true)
            {
                firstTime = false; // only execute this section once
                RTC_Timer32Compare0Set(PERIOD_4S); // set blink period to 4sec
                RTC_Timer32CounterSet(0); // reset timer to start at 0
            }
#endif
        } // end - post-test forever loop
        
        // Should never get here...
        break;
    } // while ...
            
    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}
/*******************************************************************************
 End of File
*/


/*** asmFunc.s   ***/

#include <xc.h>

/* Tell the assembler that what follows is in data memory    */
.data
.align
 
.global nameStr
.type nameStr,%gnu_unique_object
/* create a string */
   
/*** STUDENTS: HINT: this is the string that the C code prints out!  **/
nameStr: .asciz "Hello. My name is Safa Al Khaliefah."  
 
/* initialize a global variable that C can access to print the nameStr */
.global nameStrPtr
.type nameStrPtr,%gnu_unique_object
nameStrPtr: .word nameStr   /* Assign the mem loc of nameStr to nameSrPtr */
 
/* Tell the assembler that what follows is in instruction memory    */
.text
.align

/* Tell the assembler to allow both 16b and 32b extended Thumb instructions */
.syntax unified

    
/********************************************************************
function name: asmFunc
function description:
     output = asmFunc ( input1, input2 )
     
where:
     input1:  an integer value passed in from the C program
     input2:  an integer value passed in from the C program
     output: the integer value returned to the C function
     
     function description: The C call passes in the two values to be summed
                           in registers 0 and 1 (r0 and r1).
                           asmFunc adds the two integer values together
                           and places the result in r0.
     
     notes:
        None
          
********************************************************************/    
.global asmFunc
.type asmFunc,%function
asmFunc:   

    /* save the caller's registers, as required by the ARM calling convention */
    push {r4-r11,LR}

    
    /*** STUDENTS: Place your code BELOW this line!!! **************/
    ADD  r0, r0,r1 
    
    /*** STUDENTS: Place your code ABOVE this line!!! **************/
    
    /* restore the caller's registers, as required by the 
     * ARM calling convention 
     */
    pop {r4-r11,LR}

    mov pc, lr	 /* asmFunc return to caller */
   

/**********************************************************************/   
.end  /* The assembler will not process anything after this directive!!! */
           





/*******************************
 * Name: Fernando Do Nascimento
 * Student ID#: 1000638103
 * Lab Day: Wednesday
 * CSE 3442/5442 - Embedded Systems 1
 * Lab 7 (ABET): Building a PIC18F4520 Standalone Alarm System with EUSART Communication 
 ********************************/

 /**
 NOTE: 	
	*Your comments need to be extremely detailed and as professional as possible
	*Your code structure must be well-organized and efficient
	*Use meaningful naming conventions for variables, functions, etc.
	*Your code must be cleaned upon submission (no commented out sections of old instructions not used in the final system)
	*Your comments and structure need to be detailed enough so that someone with a basic 
            embedded programming background could take this file and know exactly what is going on
 **/

#include <p18F4520.h>
#include <delays.h>
//other includes
#include <EEP.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PASS    4

//Global Variables
int temp_one, temp_two, temp_three, temp_four;
int tempEnable = 0;
double volt, celcius, fahrenheit;
char temp[MAX_PASS];

//Function Prototypes
void 	ISR_High(void);
void 	ISR_Low(void);
void 	access(void);
void 	menu(void);
int 	verify_password(void);
char 	keypad(void);
void 	putch(unsigned char character);
char 	getch(void);
void    reset_password(char method);
void    temperature(void);
void    pir_handler(void);
void    refresh_menu(char method);
void    temperature_menu(char method);

//Configuration bits settings code goes here
// CONFIG1H
#pragma config OSC = HS    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON      // MCLR Pin Enable bit (RE3 input pin enabled; MCLR disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

/*
 * Interrupt Handlers (High Priority & Low Priority)
 */
#pragma code Hi_Priority = 0x0008
void Hi_Priority(void)
{
    _asm
        GOTO ISR_High
    _endasm
}

#pragma code Lo_Priority = 0x00018
void Lo_Priority(void)
{
    _asm
        GOTO ISR_Low
    _endasm
}

#pragma interrupt ISR_High
void ISR_High(void)
{
    /*
     *  If the PIR motion sensor is activated and the motion sensor
     *  goes off, then the alarm system will enter this High Interrupt.
     *  In order for the user to desactivate the PIR sensor and return
     *  to the main menu, the user must enter a valid password.
     */
    if(INTCONbits.INT0IF == 1)
    {
        /*
         *  Resets the High Interrupt Flag
         */
        INTCONbits.INT0IF = 0;

       /*
        *   Activates the Red LED light.
        */
        PORTBbits.RB2 = 1;
        
        /*
         *  Calls pir_handler in order to prompt the user for a password
         *  in order to desactivate the alarm and return to the main menu.
         */
         pir_handler();
    }
}

#pragma interrupt ISR_Low
void ISR_Low(void)
{

}

/*
 * 	Function: main
 *	Parameter(s): none
 *	Returns: void
 *	Description: Sets all the register necessary for this lab assignment.
 *	TRISx, PORTx, INTCONx, ADCONx, RCTA, RCON, TXSTA, SPBRG. Then calls
 *	the function acces, where the user will be prompted to enter a password.
 */
void main()
{
        //LED's Settings
        TRISBbits.RB2 = 0;
        TRISBbits.RB3 = 0;
        TRISBbits.RB4 = 0;
        TRISBbits.RB5 = 0;

        PORTBbits.RB5 = 0;
        PORTBbits.RB3 = 1;

        //Keypad Settings
        TRISDbits.RD7 = 1;
        TRISDbits.RD6 = 1;
        TRISDbits.RD5 = 1;
        TRISDbits.RD4 = 1;
        TRISDbits.RD3 = 0;
        TRISDbits.RD2 = 0;
        TRISDbits.RD1 = 0;
        TRISDbits.RD0 = 0;

        PORTDbits.RD0 = 1;
        PORTDbits.RD1 = 1;
        PORTDbits.RD2 = 1;
        PORTDbits.RD3 = 1;

        //TRISC Settings, Input, Output
        TRISCbits.RC7 = 1;
        TRISCbits.RC6 = 0;

        //Communication (Input & Output)
        TXSTA = 0x20;
        SPBRG = 32;
        TXSTAbits.TXEN = 1;

        //RCSTA Register Settings
        RCSTAbits.SPEN = 1;
        RCSTAbits.CREN = 1;

        //PIR Sensor Settings
        RCONbits.IPEN = 1;
        INTCON = 0b11100000;
        INTCON2bits.INTEDG0 = 1;

        //Timer & Temperature Sensor Settings
         ADCON0 = 0b00000001;
         ADCON2 = 0b10101100;
         ADCON1 = 0b10001110;

		 while(1)
		 {    
            //Main routine code
            access();
		 } //end of while(1)	
} //end of void main()

/*
 *  Function: access
 *  Parameter(s): void
 *  Returns: void
 *  Description: This function checks if this is a first time login or if the
 *  user pressed the RESET button.
 *      If this is a first time login, the default
 *      input method is the keyboard and will prompt the user to enter a pasword.
 *      Once the password has been entered, the user will be taken to the alarm
 *      system main menu.
 *      If this is NOT a first time login, the function will let the user know
 *      the input method (Keyboard or Keypad) and will also prompt the user to
 *      enter a the password.
 *      Once the password has been entered, it will check if the password matches
 *      the original password. If the password match, then the user will be taken
 *      to the alarm system main menu. If the password does NOT match, the user will
 *      be prompted to re-enter the password.
 */
void access(void)
{
    /*
     *  index - integer vairable used as a loop counter.
     *  passwordFlag - integer variable used to store the value returned
     *      by the function verify_password, when comparing passwords once the
     *      user presses the reset button.
     *  input - character variable that stores the a value read from the EEPROM
     *      at address 0x0004 in order to determine the input method.
     *  myChar - character value used to store the value entered by the user
     *      from both input methods.
     * first - reads address 0x0000 from the EEPROM in order to determine if
     *      this is a first time login or if the user has pressed the reset button.
     */
    int index, passwordFlag;
    char input, myChar, first;

    passwordFlag =  1;

    tempEnable = Read_b_eep(0x0006);

    //Reads data from EEPROM to determine if this is a first time login or not.
    first = Read_b_eep(0x0000);

    //Reads data from the EEPROM to determine the input method.
    input = Read_b_eep(0x0004);

    //First time login
    if(first == 0xFF)
    {
        //Displays message to the user with instructions.
        //Prompts the user to enter a 4-Digit Password
        printf("\033[36m");
        printf("\nInput Method: Keyboard");
        printf("\033[2;1H");
        printf("\033[K");
        printf("Enter a 4-Digit Password: ");

        /*
         *  Reads the password entered by the user, checks if the
         *  password entered is an integer or not. If the digit is valid,
         *  then the password digit will be saved starting at address
         *  0x0000 of the EEPROM. Once the password has been entered, the
         *  user will be taken to the alarm system's menu.
         */
        for(index = 0;index < MAX_PASS;index++)
        {
            //This part of the function will loop until the user enters
            //digits as the password
            do
            {
                //Reads a character pressed by the user in the keyboard
                myChar = getch();

                temp[index] = myChar;
                //If the character entered by the user is not an integer,
                //then the program will display a message to the user and
                //prompt the user to enter an integer.
                if(myChar < '0' || myChar > '9')
                {
                    printf("\033[4;1H");
                    printf("\033[K");
                    printf("Invalid Input! Please enter an integer!");

                    //Moves the cursor to the correct location in order to
                    //enter the password in the message that prompts for the
                    //password.
                    if(index == 0)
                        printf("\033[2;27H");
                     
                    else if(index == 1)
                        printf("\033[2;28H");

                    else if(index == 2)
                        printf("\033[2;29H");
                      
                    else
                        printf("\033[2;30H");
                }

                //If the user enter a valid digit for a password, then the
                //cursor will be moved to the next location in order to keep
                //enterting the rest of the password.
                else
                {
                    printf("\033[4;1H");
                    printf("\033[K");
                    printf("Valid Input!");

                    if(index == 0)
                        printf("\033[2;27H");

                    else if(index == 1)
                        printf("\033[2;28H");

                    else if(index == 2)
                        printf("\033[2;29H");

                    else
                        printf("\033[2;30H");
                }

            }while(myChar < '0' || myChar > '9');

            //Displays the asterix in order to let the user know that
            //the program read the digit entered via the keyboard.
            printf("*");
        }//End of for loop

        //Waits until the users presses the 'Enter' key in order to continue
        //running the program.
        while(getch() != 13)
            ;

        //Stores the password digit at the correct location in the EEPROM
        Write_b_eep(0x0000, temp[0]);
        Write_b_eep(0x0001, temp[1]);
        Write_b_eep(0x0002, temp[2]);
        Write_b_eep(0x0003, temp[3]);
        //Once the password has been entered, then the user will be taken to the
        //alarm's main menu.
        menu();
    }

    /*
     * Login-in after reset.
     */
    else
    {
        //Keyboard input
        if(input == 0xFF)
        {
            //Lets the user know the input method
            printf("\033[2J");
            printf("\033[2;1H");
            printf("Input Method: Keyboard");

            //Prompts the user to enter a password
            printf("\033[3;1H");
            printf("\033[K");
            printf("Enter a 4-Digit Password: ");

            for(index = 0;index < MAX_PASS;index++)
            {
                do
                {
                    //Reads a character pressed by the user in the keyboard
                    myChar = getch();

                    temp[index] = myChar;
                    //If the character entered by the user is not an integer,
                    //then the program will display a message to the user and
                    //prompt the user to enter an integer.
                    if(myChar < '0' || myChar > '9')
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Invalid Input! Please enter an integer!");
                    }//End of if statemebt

                    //If the user enter a valid digit for a password, then the
                    //cursor will be moved to the next location in order to keep
                    //enterting the rest of the password.
                    else
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Valid Input!");
                    }//End of else statement

                    //Moves the cursor to the correct location in order to
                    //enter the password in the message that prompts for the
                    //password.
                    if(index == 0)
                        printf("\033[3;27H");

                    else if(index == 1)
                        printf("\033[3;28H");

                    else if(index == 2)
                        printf("\033[3;29H");

                    else
                        printf("\033[3;30H");
                }while(myChar < '0' || myChar > '9'); //End of Do-While loop

                //Stores the password digits entered starting at location
                //0x0010 of the EEPROM
                Write_b_eep(0x0010 + index, myChar);

                //Displays the asterix to let the user know that the key
                //pressed has been recognized by the program.
                printf("*");
            }//End of for loop

            //Waits for the user to enter the 'Enter' key in order to continue
            while(getch() != 13)
                ;

            
        }//End of else statement (Keyboard input)

        //Keypad Input
        else
        {
            //Turns on the Blue LED Light
            PORTBbits.RB4 = 1;

            //Lets the user know the input type
            printf("\033[2J");
            printf("\033[2;1H");
            printf("Input Method: Keypad");

            //Prompts the user to input a password
            printf("\033[3;1H");
            printf("\033[K");
            printf("Enter a 4-Digit Password: ");

            //Loops in order to read the 4-digit password form the user
            //using the keypad
            index = 0;
            while(index < MAX_PASS)
            {
                do
                {
                    //Reads input from the keypad.
                    myChar = keypad();

                    //The input entered from the keypad is NOT valid
                    if(myChar == 'D')
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Invalid Input! Please enter an integer!");
                    }

                    //The input entered from the keypad is valid
                    if(myChar != 'D' && myChar != 'X')
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Valid Input!");
                    }

                    //Moves the cursor to the correct location in order to
                    //enter the password in the message that prompts for the
                    //password.
                    if(index == 0)
                        printf("\033[3;27H");

                    else if(index == 1)
                        printf("\033[3;28H");

                    else if(index == 2)
                        printf("\033[3;29H");

                    else
                        printf("\033[3;30H");
                }while(myChar == 'X' || myChar == 'D'); //End of do-while (Keypad Input)

                //Lets the user know that the key pressed has been recognized
                //by the program
                printf("*");

                //Stores the key pressed starting at address 0x0010 in the EEPROM
                Write_b_eep(0x0010 + index, myChar);

                //Increases the loop counter
                index++;
            }//End of while statement (Password input)

            //Waits until the user enters the equivalent of the 'Enter' key
            //in the keypad. 'D' = 'Enter' key.
            while(keypad() != 'D')
                ;
        }//End of else statement (Keypad Input)
       
        //Checks if the password entered by the resur is valid or not.
        passwordFlag = verify_password();

        //Invalid Password
        if(passwordFlag == 0)
        {
            printf("\033[4;1H");
            printf("\nInvalid Password!");

            //Waits a few seconds before recalling the function access.
            for(index = 0;index < 5;index++)
                Delay10KTCYx(20);

            printf("\033[2J");

            access();
        } //End of if statement (Invalid password)

        //Valid Password!
        else
        {
            printf("\033[4;1H");
            printf("\nValid Password!");
            printf("\033[5;1H");
            printf("\nAccess Granted");

            //Waits a few seconds before the user is sent to the main menu
            //of the alarm system.
            printf("\033[2J");

            menu();
        }//End of else statement (Valid Password)
    }
}

/*
 *	Function: temperature
 * 	Parameter(s): void
 *	Returns: void
 *	Description: Perform A/D Conversion and converts the value to the temperature
 *	in fahrenheit. Resets the timer prescaler values.
 */
void temperature(void)
{
	/*
	 *	low - integer holding the ADRESL
	 *	high - integer holding the ADRESH
	 *	result - holds the addition of low + high. After high perform an 8-bit shift.
	 *	volt - Calculates the voltage of TMP36
	 *	celcius - calculates the temperature in celsius from the voltage read from TMP36
	 *	fahrenheit - converts celsius to fahrenheit.
	 *	temp_one - holds the first digit of the temperature in fahrenheit (Nx.xx)
	 *	temp_two - holds the second digit of the temperature in fahrenheit (xN.xx)
	 *	temp_three holds the first digit after the decimal point of the temperature in fahrenheit (xx.Nx)
	 *	temp_three holds the second digit after the decimal point of the temperature in fahrenheit (xx.xN)
	 */
    int low, high, result;

	//Resets clock prescaler values
    TMR0H = 0x67;
    TMR0L = 0x6A;

    ADCON0bits.GO = 1;
    while(ADCON0bits.DONE == 1)
        ;

	//Sets the ADRESL and ADRESH
    low = ADRESL;
    high = ADRESH;

	//Perform the addition of the high bits and low bits
    result = (high<<8) + low;

	//Calculates the voltage of the TMP36
    volt = (((double)result) / 1023.0) * 5.0;

	//Converts the voltage to degree celsius
    celcius = (volt - 0.5) / 0.1;

	//Converts the celsius to fahrenheit
    fahrenheit = (celcius * 1.8) + 32;

	//Gets each digit of the temperature
    temp_one = ((int)fahrenheit / 10) % 10;
    temp_two = (int) fahrenheit % 10;
    temp_three = ((int) (fahrenheit * 10.0) % 10);
    temp_four = ((int) (fahrenheit * 100.0) % 10);
}

/*
 *  Function: verify_password
 *  Parameter(s): void
 *  Returns:
 *      0 - If the password entered by the user is not the same as the
 *          password stored in the EEPROM.
 *      1 - If the password entered by the user is the same as the password stored
 *          in the EEPROM.
 *  Description: When the user is prompted to enter a password (Except at start
 *      up), this function will verify if the password entered by the user matches
 *      the original password. If the password matches, the function will return
 *      a 1. If the password does NOT match, then the function will return a 0.
 */
int verify_password(void)
{
    /*
     *  index - integer variable used as a loop counter.
     *  password - integer value that is use to read the password entered
     *      at the start up of the program. This password is stored at address
     *      0x0000 in the EEPROM.
     *  passwordCmp - integer value that is use to read the password that
     *      was entered by the user (NOT at the Start Up).
     */
    int index, password, passwordCmp;

    /*
     *  Loops thru the addresses 0x0000 to 0x0003 (Original Password)
     *  and addresses 0x0010 to 0x0013 and stores each value in two
     *  different integer variables. After that, we compare each value
     *  read from the EEPROM to determine if the passwords match or not.
     */
    for(index = 0;index < MAX_PASS;index++)
    {
        /*
         *  The original password is stored at address 0x0000 of the EEPROM.
         *  This code reads the password character starting at address 0x0000,
         *  and moves to the next address as the loop counter increases.
         */
        password = Read_b_eep(0x0000 + index);

        /*
         *  When the program prompts the user for a password (Except at start up),
         *  the program stores the password starting at address 0x0010.
         *  This code reads the password stored at address 0x0010 and as the
         *  loop counter increases, the address increases as well to read the
         *  rest of the password characters.
         */
         passwordCmp = Read_b_eep(0x0010 + index);

        //Password does not match!
        if(password != passwordCmp)
            return 0;
    }

    //Password matches!
    return 1;
}

/*
 *  Function: menu
 *  Parameter(s): void
 *  Returns: void
 *  Description: The function displays the program's header, the status of each
 *  component of the alarm system and a menu of options.
 *  Based on the option selected by the user, the function will perform the task
 *  and in the case of change in the status of the components; the function will
 *  update the status of that device.
 */
void menu(void)
{
	/*
	 *	index - integer used as a loop counter
	 *	input - character used to determine the input method keyboard/keypad
	 *	pirStatus - character used to determine the state of the PIR sensor
	 *	option - character used to determine the option selected by the user
	 */
    int index;
    char input, option, pirStatus;

    input = Read_b_eep(0x0004);
    pirStatus = Read_b_eep(0x0005);

    if(pirStatus != 0xFF)
        INTCONbits.INT0IE = 1;

    printf("\033[2J");

    //Displays the menu option and the header of the program
    refresh_menu(input);

    do
    {
        printf("\033[30;1H");
        printf("\033[K");
        printf("Input: ");

        input = Read_b_eep(0x0004);

        /*
         * Keyboard Input
         */
        if(input == 0xFF)
        {
            option = getch();

            putch(option);

            while(getch() != 13)
                ;
        }//End of if statement (Keyboard input)

        /*
         * Keypad Input
         */
        else
        {
            while(1)
            {
                option = keypad();

                if(option != 'X')
                    break;
            }
            
            putch(option);

            while(keypad() != 'D')
                ;
        }

		//Based on the option selected by the user
		//the switch statement will perform a certain task.
        switch(option)
        {
            //Reset Password
            case '1':
                reset_password(input);
                break;

            //Activate/Desactivate PIR Sensor
            case '2':
                pirStatus = Read_b_eep(0x0005);

                //Activate PIR Sensor
                if(pirStatus == 0xFF)
                {
                    INTCONbits.INT0IE = 1;

                    Write_b_eep(0x0005, 'A');

                    printf("\033[12;1H");
                    printf("\033[K");
                    printf("PIR Sensor Alarm State:\t\tActive");
                }

                //Desactivate PIR Sensor
                else
                {
                    INTCONbits.INT0IE = 0;

                    Write_b_eep(0x0005, 0xFF);

                    printf("\033[12;1H");
                    printf("\033[K");
                    printf("PIR Sensor Alarm State:\t\tInactive");
                }
                break;

			//Temperature Sensor Menu
            case '3':
                printf("\033[2J");
                temperature_menu(input);
                break;
				
            //Keyboard Input
            case '4':
                printf("\033[16;1H");
                printf("Current Input Method:\t\tKeyboard (Terminal)");

                Write_b_eep(0x0004, 0xFF);

                PORTBbits.RB4 = 0;

                PORTDbits.RD0 = 1;
                PORTDbits.RD1 = 1;
                PORTDbits.RD2 = 1;
                PORTDbits.RD3 = 1;
                break;

			//Keypad Input
            case '5':
                printf("\033[16;1H");
                printf("\033[K");
                printf("Current Input Method:\t\tKeypad (Terminal)");

                Write_b_eep(0x0004, 'K');
                PORTBbits.RB4 = 1;

                PORTDbits.RD0 = 0;
                PORTDbits.RD1 = 0;
                PORTDbits.RD2 = 0;
                PORTDbits.RD3 = 0;
                break;

			//Refresh Menu
            case '0':
                menu();
                break;

            default:
                printf("\033[32;1H");
                printf("\033[K");
                printf("Invalid Option!");

                for(index = 0;index < 4;index++)
                    Delay10KTCYx(200);

                printf("\033[32;1H");
                printf("\033[K");
                
                break;
        }//End of switch statement
    }while(option != '0');

    if(option == '0')
        menu();
}

/*
 *  Function: reset_password
 *  Parameter(s): method - character variable storing the input method, so the
 *      program know which input method to accept (Keyboard or Keypad).
 *  Returns: void
 *  Description: The function will display a menu to the user and give the user
 *	the option to return to the main menu or reset the password. If the user
 *	decides to change the password, the user must first enter the current password.
 *	If the password is valid, then the user will be allowed to enter a new password.
 */
void reset_password(char method)
{
	/*
	 *	option - character storing the selected input by the user from the menu
	 *	currentPassword - holds the current characters of the password
	 *	newPassword - holds the new characters of the password
	 *	passwordFlag - integer used to determine if the current password is valid or not.
	 */
    char option, currentPassword, newPassword;
    int index;
    int passwordFlag = 0;

    printf("\033[2J");
    printf("\033[2;1H");

    if(method == 0xFF)
        printf("Input Method: Keyboard");

    else
        printf("Input Method: Keypad");

    printf("\033[4;1H");
    printf("1. Reset Password");
    printf("\033[5;1H");
    printf("0. Return to Main Menu");
    printf("\033[7;1H");
    printf("Please select an option from the menu above: ");

    //Keyboard Input
    if(method == 0xFF)
    {
        //Loops until the user enters a valid input
        do
        {
            //Gets input from keyboard
            option = getch();

            //Displays input to the Tera Term Terminal
            putch(option);

            //Waits until the user presses the 'Enter' key
            while(getch() != 13)
                ;

            //Checks if the input is NOT valid
            if(option != '0' && option != '1')
            {
                printf("\033[8;1H");
                printf("\033[K");
                printf("Invalid Input!");
                printf("\033[7;46H");
            }//End of if statement (Invalid Input)

            //The input IS valid
            else
            {
                printf("\033[8;1H");
                printf("\033[K");
                printf("Valid Input!");
                printf("\033[7;46H");
                break;
            }//End of else statement (Valid Input)

        }while(option != '0' && option != '1');

		//Reset Password
		//Keyboard Method
        if(option == '1')
        {
			//Will check if the password entered is valid or not
            while(passwordFlag == 0)
            {
                printf("\033[2J");
                printf("\033[2;1H");

				//Input method
                if(method == 0xFF)
                    printf("Input Method: Keyboard");

                else
                    printf("Input Method: Keypad");

                printf("\033[3;1H");
                printf("Enter the current password: ");

				//The user enters the current password
				//Stored in address 0x0010 for comparison purposes
                for(index = 0;index < MAX_PASS;index++)
                {
                    currentPassword = getch();

                    Write_b_eep(0x0010 + index, currentPassword);

                    printf("*");
                }

				//Waits for enter key to be pressed
                while(getch() != 13)
                    ;

				//Calls function to verify passowrd
                passwordFlag = verify_password();

				//Invalid Password
                if(passwordFlag == 0)
                {
                    printf("\033[5;1H");
                    printf("Invalid Password!");

                    for(index = 0;index < 5;index++)
                        Delay10KTCYx(20);
                }

				//Valid password
                else
                {
                    printf("\033[5;1H");
                    printf("Valid Password!");

					//Prompts the user for a new password
                    printf("\033[2J");
                    printf("\033[2;1H");
                    printf("Input Method: Keyboard");
                    printf("\033[3;1H");
                    printf("Enter the new password: ");

                    for(index = 0;index < MAX_PASS;index++)
                    {
                        do
                        {
                            //Reads a character pressed by the user in the keyboard
                            newPassword = getch();

                            //If the character entered by the user is not an integer,
                            //then the program will display a message to the user and
                            //prompt the user to enter an integer.
                            if(newPassword < '0' || newPassword > '9')
                            {
                                printf("\033[5;1H");
                                printf("\033[K");
                                printf("Invalid Input! Please enter an integer!");
                            }//End of if statemebt

                            //If the user enter a valid digit for a password, then the
                            //cursor will be moved to the next location in order to keep
                            //enterting the rest of the password.
                            else
                            {
                                printf("\033[5;1H");
                                printf("\033[K");
                                printf("Valid Input!");
                            }//End of else statement

                            //Moves the cursor to the correct location in order to
                            //enter the password in the message that prompts for the
                            //password.
                            if(index == 0)
                                printf("\033[3;25H");

                            else if(index == 1)
                                printf("\033[3;26H");

                            else if(index == 2)
                                printf("\033[3;27H");

                            else
                                printf("\033[3;28H");

                        }while(newPassword < '0' || newPassword > '9'); //End of Do-While loop

                        //Displays the asterix to let the user know that the key
                        //pressed has been recognized by the program.
                        printf("*");

                        //Stores the password digits entered starting at location
                        //0x0010 of the EEPROM
                        Write_b_eep(0x0000 + index, newPassword);
                    }//End of for loop

                    //Waits for the user to enter the 'Enter' key in order to continue
                    while(getch() != 13)
                        ;

                    menu();
                }
            }
        }
    }

    //Keypad Input
    else
    {
        //Loops until the user enters a valid input
        do
        {
            //Gets input from keyboard
            do
            {
                option = keypad();
            }while(option == 'X');
            //Displays input to the Tera Term Terminal
            putch(option);

            //Waits until the user presses the 'Enter' key
            while(keypad() != 'D')
                ;

            //Checks if the input is NOT valid
            if(option != '0' && option != '1')
            {
                printf("\033[8;1H");
                printf("\033[K");
                printf("Invalid Input!");
                printf("\033[7;46H");
            }//End of if statement (Invalid Input)

            //The input IS valid
            else
            {
                printf("\033[8;1H");
                printf("\033[K");
                printf("Valid Input!");
                printf("\033[7;46H");

            }//End of else statement (Valid Input)

        }while(option != '0' && option != '1');

		//Reset Password
		//Keypad Method
        if(option == '1')
        {
            printf("\033[2J");
            printf("\033[2;1H");
            printf("Input Method: Keypad");
            printf("\033[3;1H");
            printf("Enter the current password: ");

            //Loops in order to read the 4-digit password form the user
            //using the keypad
            index = 0;
            while(index < MAX_PASS)
            {
                do
                {
                    //Reads input from the keypad.
                    currentPassword = keypad();

                    //The input entered from the keypad is NOT valid
                    if(currentPassword == 'D')
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Invalid Input! Please enter an integer!");
                    }

                    //The input entered from the keypad is valid
                    if(currentPassword != 'D' && currentPassword != 'X')
                    {
                        printf("\033[5;1H");
                        printf("\033[K");
                        printf("Valid Input!");
                    }

                    //Moves the cursor to the correct location in order to
                    //enter the password in the message that prompts for the
                    //password.
                    if(index == 0)
                        printf("\033[3;29H");

                    else if(index == 1)
                        printf("\033[3;30H");

                    else if(index == 2)
                        printf("\033[3;31H");

                    else
                        printf("\033[3;32H");
                }while(currentPassword == 'X' || currentPassword == 'D'); //End of do-while (Keypad Input)

                //Lets the user know that the key pressed has been recognized
                //by the program
                printf("*");

                //Stores the key pressed starting at address 0x0010 in the EEPROM
                Write_b_eep(0x0010 + index, currentPassword);

                //Increases the loop counter
                index++;
            }//End of while statement (Password input)

            //Waits until the user enters the equivalent of the 'Enter' key
            //in the keypad. 'D' = 'Enter' key.
            while(keypad() != 'D')
                ;
             //Checks if the password entered by the resur is valid or not.
            passwordFlag = verify_password();

            //Invalid Password
            if(passwordFlag == 0)
            {
                printf("\033[4;1H");
                printf("\nInvalid Password!");

                //Waits a few seconds before recalling the function access.

                printf("\033[2J");

                reset_password(method);
            } //End of if statement (Invalid password)

            //Valid Password!
            else
            {
                printf("\033[4;1H");
                printf("\nValid Password!");
                printf("\033[5;1H");
                printf("\nAccess Granted");

                //Waits a few seconds before the user is sent to the main menu
                //of the alarm system.

                printf("\033[2J");
                //Lets the user know the input type
                printf("\033[2J");
                printf("\033[2;1H");
                printf("Input Method: Keypad");

                //Prompts the user to input a password
                printf("\033[3;1H");
                printf("\033[K");
                printf("Enter the new password: ");

                //Loops in order to read the 4-digit password form the user
                //using the keypad
                index = 0;
                while(index < MAX_PASS)
                {
                    do
                    {
                        //Reads input from the keypad.
                        newPassword = keypad();

                        //The input entered from the keypad is NOT valid
                        if(newPassword == 'D')
                        {
                            printf("\033[5;1H");
                            printf("\033[K");
                            printf("Invalid Input! Please enter an integer!");
                        }

                        //The input entered from the keypad is valid
                        if(newPassword != 'D' && newPassword != 'X')
                        {
                            printf("\033[5;1H");
                            printf("\033[K");
                            printf("Valid Input!");
                        }

                        //Moves the cursor to the correct location in order to
                        //enter the password in the message that prompts for the
                        //password.
                        if(index == 0)
                            printf("\033[3;25H");

                        else if(index == 1)
                            printf("\033[3;26H");

                        else if(index == 2)
                            printf("\033[3;27H");

                        else
                            printf("\033[3;28H");
                    }while(newPassword == 'X' || newPassword == 'D'); //End of do-while (Keypad Input)

                    //Lets the user know that the key pressed has been recognized
                    //by the program
                    printf("*");

                    //Stores the key pressed starting at address 0x0010 in the EEPROM
                    Write_b_eep(0x0000 + index, newPassword);

                    //Increases the loop counter
                    index++;
                }//End of while statement (Password input)

                //Waits until the user enters the equivalent of the 'Enter' key
                //in the keypad. 'D' = 'Enter' key.
                while(keypad() != 'D')
                    ;

                menu();
            }//End of else statement (Keypad Input)

        }//End of if statement (Option == 1)
    }//End of else statement (Keypad Input)

    menu();
}

/*
 *	Function: temperature_menu
 *	Parameter(s): method - character that holds the input method Keyboard/Keypad
 *	Returns: void
 *	Description: Prompts the user with the temperature menu. Here the user will be
 * 	allowed to activate and deactivate the temperature sensor. At the moment, the 
 *	user is not allowed to set the threshold. Also the only input accepted is the
 *	keyboard.
 */
void temperature_menu(char method)
{
    char option;

    printf("\033[2J");
    printf("\033[2;1H");

	//Displays the input method
    if(method == 0xFF)
        printf("Input Method: Keyboard");

    else
        printf("Input Method: Keypad");

	//Displays the menu
    printf("\033[4;1H");
    printf("1.Enable Temperature Sensor");
    printf("\033[5;1H");
    printf("2.Disable Temperature Sensor");
    printf("\033[6;1H");
    printf("3.Set Temperature Threshold");
    printf("\033[8;1H");
    printf("0.Return to Main Menu");
    printf("\033[10;1H");
    printf("Input: ");

	//Keyboard Input
    if(method == 0xFF)
    {
        do
        {
            option = getch();

            putch(option);

            while(getch() != 13)
                ;

            switch(option)
            {
				//Activates the sensor & returns to the main menu
                case '1':
                    Write_b_eep(0x0006, 'T');
                    tempEnable = 1;
                    menu();
                    break;

				//Deactivates the sensor & returns to the main menu
                case '2':
                    Write_b_eep(0x0006, 0xFF);
                    tempEnable = 0;
                    menu();
                    break;
            }//End of switch case
        }while(option != '0');	//End of do while
    }//End of if statement (Keyboard input)

}

/*
 *	Function: refresh_menu
 *	Parameter(s): method - character holding the input method to display in the menu
 *	Returns: void
 *	Description: Displays the alarm system's menu to the user with the current
 *	status of each component along with a series of option to choose from.
 */
void refresh_menu(char method)
{
    char pirStatus = Read_b_eep(0x0005);
    char tempStatus = Read_b_eep(0x0006);

    int index;
    /*************************Displays Program Header**************************/
    printf("\033[2J");
    printf("\033[2;1H");

    for(index = 0;index < 114;index++)
        printf("*");

    printf("\033[3;1H");
    printf("*\t\t\t\t\t\tAlarm System is Connected\t\t\t\t\t *");

    printf("\033[4;1H");
    printf("*\t\t\t\t           CSE 3442/5442 - Embedded System 1\t\t\t\t\t *");

    printf("\033[5;1H");
    printf("*\t\t\t\t    Lab 7 (ABET) - Standalone PIC with Communication\t\t\t\t *");

    printf("\033[6;1H");
    printf("*\t\t\t\t\t\t(Fernando Do Nascimento)\t\t\t\t\t *\n");

    printf("\033[7;1H");
    for(index = 0;index < 114;index++)
        printf("*");
    /*************************End of Program Header****************************/

    /***************************Component Statuses*****************************/
    printf("\033[9;1H");
    printf("Component Statuses\n");

    printf("\033[10;1H");
    for(index = 0;index < 114;index++)
        printf("-");

    //Displays the status of the PIR Sensor
    printf("\033[12;1H");
    if(pirStatus == 0xFF)
        printf("PIR Sensor Alarm State:\t\tInactive");

    else
        printf("PIR Sensor Alarm State:\t\tActive");

    //If Statement
    printf("\033[13;1H");

    if(tempStatus == 0xFF)
        printf("Temperature Alarm State:\tInactive");

    else
        printf("Temperature Alarm State:\tActive");
    printf("\033[14;1H");
    printf("Current Temperature Reading:\t%d%d.%d%d", temp_one, temp_two, temp_three, temp_four);

    printf("\033[15;1H");
    printf("Temperature Alarm Threshold:\t");
    //
    printf("\033[16;1H");
    if(method == 0xFF)
        printf("Current Input Method:\t\tKeyboard (Terminal)");

    else
        printf("Current Input Method:\t\tKeypad");

    printf("\033[18;1H");
    for(index = 0;index < 114;index++)
        printf("-");
    /************************End of Component Statuses*************************/

    /******************************Main Menu***********************************/
    printf("\033[20;1H");
    printf("Select One of the Following:");

    printf("\033[22;1H");
    printf("\t1: Passcode Option");

    printf("\033[23;1H");
    printf("\t2: PIR Sensor Alarm Options");

    printf("\033[24;1H");
    printf("\t3: Temperature Sensor Alarm Options");

    printf("\033[25;1H");
    printf("\t4: Use Keyboard (Terminal} As The Only Input");

    printf("\033[26;1H");
    printf("\t5: Use Keypad As The Only Input ('D' = Enter Key)");

    printf("\033[28;1H");
    printf("\t0: Refresh Main Menu\n\n");
	/**************************End of Main Menu*****************************/
}

/*
 * 	Function: pir_handler
 *	Parameter(s): void
 *	Returns: void
 *	Description: If the PIR sensor is triggered off, then the function will
 *	notify the user that the alarm has been triggered and will prompt the user
 *	to enter a password in order to unlock the alarm system. Once the password
 *	has been accepted the user will have the option to return to the main menu
 *	leaving the PIR sensor on or disable the pir sensor.
 */
void pir_handler(void)
{
	/*
	 *	index - loop counter
	 *	input - holds the input method
	 *	passwordCmp - password to be compared for validation issues.
	 *	option - selection pressed by the user from the menu diplayed
	 *	flag - valid or invalid password
	 */
    int index, flag;
    char input = Read_b_eep(0x0004);
    char passwordCmp, option;

    flag = 0;

    printf("\033[2J");
    printf("\033[2;1H");
    printf("!!!!!!Attention The Motion Sensor Has Detected Movement!!!!!!");

    do
    {
         printf("\033[4;1H");
         if(input == 0xFF)
            printf("Input Method: Keyboard");

         else
            printf("Input Method: Keypad");

        printf("\033[5;1H");
        printf("\033[K");
        printf("Enter a 4-Digit Password: ");

        /*
         * Reads Password (Keyboard)
         */
        if(input == 0xFF)
        {
            for(index = 0;index < MAX_PASS;index++)
            {
                passwordCmp = getch();

                printf("*");

                Write_b_eep(0x0010 + index, passwordCmp);
            }//End of for loop (password input)

			//Waits for user to enter enter key
            while(getch() != 13)
                ;
        }//End of if statement (keyboard input)
		
		//Reads Password (Keypad)
        else
        {
            for(index = 0;index < MAX_PASS;index++)
            {
                do
                {
                    passwordCmp = keypad();
                }while(passwordCmp == 'X');

                printf("*");

                Write_b_eep(0x0010 + index, passwordCmp);
            }//End of for loop (password input)

			//Waits for user to press the enter key in the keypad
            while(keypad() != 'D')
                ;
        }//End of else statement(Keypad input)

		//Checks if password is valid or not
        flag = verify_password();

		//Invalid password
        if(flag == 0)
        {
             printf("\033[7;1H");
             printf("Invalid Password!");

             for(index = 0;index < 5;index++)
                Delay10KTCYx(200);
        }//End of if statement(invalid password)

		//Valid Password
        else
        {
            PORTBbits.RB2 = 0;

            Write_b_eep(0x0005, 0xFF);

            printf("\033[7;1H");
            printf("\033[K");
            printf("Valid Password!");

            for(index = 0;index < 5;index++)
                Delay10KTCYx(20);

            printf("\033[2J");

			//Keyboard input
            if(input == 0xFF)
            {
				//Displays a menu of options to the user
                printf("\033[2;1H");
                printf("Input Method: Keyboard");
                printf("\033[4;1H");
                printf("1.Disable PIR Sensor");
                printf("\033[5;1H");
                printf("0.Return to Main Menu");
                printf("\033[7;1H");
                printf("Select an option from the menu above: ");

				//The loop will run until the user presses an option
				//The switch case will perform the task and return to the main menu.
                while(1)
                {
					//reads character from keyboard
                    option = getch();

                    putch(option);

					//Waits for enter key
                    while(getch() != 13)
                        ;

                    switch(option)
                    {
						//Deactivate the pir sensor
                        case '1':
                            Write_b_eep(0x0005, 0xFF);
                            INTCON = 0b11100000;
                            menu();
                            break;

						//Return to main menu
                        case '0':
                            Write_b_eep(0x0005, 'A');
                            INTCON = 0b11110000;
                            menu();
                            break;

						//Invalid input
                        default:
                            printf("\033[9;1H");
                            printf("Invalid Input!");
                            break;
                    }//end of switch case (Menu)
                }//Infinite while loop
            }//End of if statement(Keyboard input)

			//Keypad Input
            else
            {
				//Displays the menu to the user
                printf("\033[2;1H");
                printf("Input Method: Keyboard");
                printf("\033[4;1H");
                printf("1.Disable PIR Sensor");
                printf("\033[5;1H");
                printf("0.Return to Main Menu");
                printf("\033[7;1H");
                printf("Select an option from the menu above: ");

				//Reads input from the keypad
                while(1)
                {
                    option = keypad();

                    if(option == '1' || option == '0')
                        break;
                }//End of infinite loop (Input)

                putch(option);

				//Waits for enter key to be pressed
                while(keypad() != 'D')
                    ;
                
				//Perform a task based on the option selected
                switch(option)
                {
					//Deactivate pir sensor
                    case '1':
                        Write_b_eep(0x0005, 0xFF);
                        INTCON = 0b11100000;
                        menu();
                        break;

					//Return to main menu
                    case '0':
                        Write_b_eep(0x0005, 'A');
                        INTCON = 0b11110000;
                        menu();
                        break;
					
					//Return to main menu
                    default:
                        printf("\033[9;1H");
                        printf("Invalid Input!");
                        break;
                }//End of switch case(pir menu)
            }//end of else statement Keypad input

            menu();
        }//End of else statement (valid password)
    }while(flag == 0); //End of do while loop (Invalid input)
}

/*
 *	Function: keypad
 *	Parameter(s): void
 * 	Returns: The character pressed on the keypad by the user.
 *	Description: The function toggles between RDO to RD3 by setting one of them at the 
 *	time to high and comparing them to RD4 to RD7 in order to determine which key
 *	the user pressed on the keypad. If no key was pressed, then the function will return
 *	an 'X' to let the program know that the user has not pressed a key yet. If a user presses
 *	a key, then the blue LED will turn on and off to inform the user that it read the key pressed
 * 	and that it is safe to press another key.
 */
char keypad(void)
{
	//Sets port RD0 to High
    PORTDbits.RD0 = 1;
    PORTDbits.RD1 = 0;
    PORTDbits.RD2 = 0;
    PORTDbits.RD3 = 0;

	//Checks if the user pressed the '1' key
    if(PORTDbits.RD4 == 1)
    {
		//Resets RD0 to low
        PORTDbits.RD0 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '1';
    }

	//Checks if the user pressed the '2' key
    if(PORTDbits.RD5 == 1)
    {
		//Resets RD0 to low
        PORTDbits.RD0 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;
		
		//Returns the character pressed
        return '2';
    }
	
	//Checks if the user pressed the '3' key
    if(PORTDbits.RD6 == 1)
    {
		//Resets RD0 to low
        PORTDbits.RD0 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '3';
    }

	//Sets PORT RD1 to High
    PORTDbits.RD0 = 0;
    PORTDbits.RD1 = 1;
    PORTDbits.RD2 = 0;
    PORTDbits.RD3 = 0;

	//Checks if the user pressed the '4' key
    if(PORTDbits.RD4 == 1)
    {
		//Resets RD1 to low
        PORTDbits.RD1 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;
		
		//Returns the character pressed
        return '4';
    }

	//Checks if the user pressed the '5' key
    if(PORTDbits.RD5 == 1)
    {
		//Resets RD1 to low
        PORTDbits.RD1 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '5';
    }

	//Checks if the user pressed the '6' key
    if(PORTDbits.RD6 == 1)
    {
		//Resets RD1 to low
        PORTDbits.RD1 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;
		
		//Returns the character pressed
        return '6';
    }
    
	//Sets PORT RD2 to high
    PORTDbits.RD0 = 0;
    PORTDbits.RD1 = 0;
    PORTDbits.RD2 = 1;
    PORTDbits.RD3 = 0;

	//Checks if the user pressed the '7' key
    if(PORTDbits.RD4 == 1)
    {
		//Resets RD2 to low
        PORTDbits.RD2 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '7';
    }

	//Checks if the user pressed the '8' key
    if(PORTDbits.RD5 == 1)
    {
		//Resets RD2 to low
        PORTDbits.RD2 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '8';
    }

	//Checks if the user pressed the '9' key
    if(PORTDbits.RD6 == 1)
    {
		//Resets RD2 to low
        PORTDbits.RD2 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '9';
    }

	//Sets PORT RD3 to high
    PORTDbits.RD0 = 0;
    PORTDbits.RD1 = 0;
    PORTDbits.RD2 = 0;
    PORTDbits.RD3 = 1;

	//Checks if the user pressed the '0' key
    if(PORTDbits.RD5 == 1)
    {
		//Resets RD3 to low
        PORTDbits.RD3 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return '0';
    }

	//Checks if the user pressed the 'D' key (aka Enter key)
    if(PORTDbits.RD7 == 1)
    {
		//Resets RD3 to low
        PORTDbits.RD3 = 0;

		//Turns the blue LED light off
        PORTBbits.RB4 = 0;

        Delay10KTCYx(200);

		//Turns the blue LED light on
        PORTBbits.RB4 = 1;

		//Returns the character pressed
        return 'D';
    }
    
	//Nothing was pressed
    return 'X';
}

/*
 * Function: putch
 * Parameter(s): temp - unsigned character used to trasfer characters from the PIC18F4520
 * to the TeraTerm.
 * Returns: void
 * Description: Transfers each character from a string to the TeraTerm via Communication
 * Peripherals.
 */
void putch(unsigned char temp)
{
    TXREG = temp;
    while(PIR1bits.TXIF == 0);
}

/*
 * Function: getch
 * Parameter(s): void
 * Returns - myChar: character entered by the user.
 * Description: Reads a character entered by the user in the keyboard
 */
char getch(void)
{
    char myChar;

    while(PIR1bits.RCIF == 0)
        ;

    myChar = RCREG;

    return myChar;
}
//Be sure to have a blank line at the end of your program

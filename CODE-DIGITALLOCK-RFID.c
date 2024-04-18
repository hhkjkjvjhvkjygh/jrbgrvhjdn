   #include <CODE-DIGITALLOCK-RFID.h>
   #device ADC=16
   #FUSES NOWDT                    //No Watch Dog Timer
   #FUSES NOBROWNOUT               //No brownout reset
   #FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
   #use delay(crystal=4MHz)
   #use rs232 (baud = 9600, xmit = pin_c6, rcv = pin_c7)
   
//-->KHAI BÁO CHÂN CHO CÁC THIET BI NGOAI VI
   #use     fast_io(A)
   #use     fast_io(B)
   #use     fast_io(C)
   #use     fast_io(D)
   #use     fast_io(E)
   #define  BUZZER             PIN_C1
   #define  OPEN_SERVO_CLOSE   PIN_E0
   #define  ON_DIGITALLOCK     PIN_E1
   #define  LED_SERVO_OPEN     PIN_E2
   #define  LED_SERVO_CLOSE    PIN_C2
   
//-->KHAI BÁO CHÂN CHO LCD
   #define  LCD_ENABLE_PIN     PIN_D3
   #define  LCD_RS_PIN         PIN_D1
   #define  LCD_RW_PIN         PIN_D2
   #define  LCD_DATA4          PIN_D4
   #define  LCD_DATA5          PIN_D5
   #define  LCD_DATA6          PIN_D6
   #define  LCD_DATA7          PIN_D7
   #include <lcd.c>
   
//-->KHAI BÁO CHÂN CHO BÀN PHÍM
   #define  C1                 PIN_B0 //-->COLUMN
   #define  C2                 PIN_B1
   #define  C3                 PIN_B2
   #define  C4                 PIN_B3
   #define  R1                 PIN_B4 //-->ROW
   #define  R2                 PIN_B5
   #define  R3                 PIN_B6
   #define  R4                 PIN_B7
   
//-->KHAI BÁO CHÂN CHO RFID-RC522
   #define  MFRC522_CS         PIN_A5
   #define  MFRC522_SCK        PIN_C3
   #define  MFRC522_SI         PIN_C4
   #define  MFRC522_SO         PIN_C5
   #define  MFRC522_RST        PIN_D0
   #include <MFRC522.h>
   
//-->KHAI BÁO MANG CHAR CHO NHAP,RESET,QUEN PASS
   UNSIGNED char  ENTER_PASSWORD[4] = {'0','0','0','0'};
   UNSIGNED char  RESET_PASSWORD[4] = {'0','0','0','0'};
   UNSIGNED char FORGOT_PASSWORD[4] = {'A','C','B','D'};
   
//-->KHAI BÁO CÁC BIEN LIÊN QUAN
   int enable;
   int i;
   int j = 0;
   int1 BIENNHO = 0;
   char DATA_NAM[5] =  {0xDB, 0x7A, 0x9B, 0x21, 0x1B};
   char DATA_TUNG[5] = {0x61, 0xBB, 0xE4, 0x26, 0x18};
   UNSIGNED int TAGTYPE;
   char UID[5];
   int1 THENHO, TAMTHOI;
   int1 TRUNGGIAN1, TRUNGGIAN2;
   
//--> CÁC CHUONG TRÌNH CON
   void WARNING();
   void BUZZER_KEY(); 
   void EEPROM_WRITE_PASSWORD (unsigned int8 addr, unsigned char*pointer);
   char KEY_SCAN(void);
   char GET_KEY(void);
   void ENTER_PASS();
   void CONTROL_SERVO_OPEN();
   void CONTROL_SERVO_CLOSE();
   void RESET_0_PASSWORD();
   void OPEN_CLOSE_DOOR();
   int  COMPARE (void);
   int  COMPARE_FORGOT_PASS(void);
   int1 SCAN_RFID_RC522(char DATA[], char UID[]);
 
   
void main() 
{
   //--> KHAI BÁO I/O, LCD, RFID
   set_tris_A(0x00);
   set_tris_B(0x0f);
   set_tris_C(0xA0);
   set_tris_D(0x00);
   set_tris_E(0x03);
   port_B_pullups(1);
   output_C(0x00);
   output_E(0x00);
   
   lcd_init();
   delay_ms(200);
   
   MFRC522_init();
   delay_ms(100);
   
   lcd_gotoxy(1,1);
   lcd_putc("DIG LOCK &  RFID");
   lcd_gotoxy(1,2);
   lcd_putc("NHOM 16 - 20CDT1");
   delay_ms(1500);
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" NGO.T HOAI NAM ");
   lcd_gotoxy(1,2);
   lcd_putc(" VO THANH TUNG  ");
   delay_ms(1500);
   
   LOOP:
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("USE RFID CARD OR");
   lcd_gotoxy(1,2);
   lcd_putc("(BTN) ON DIGLOCK");

//==============================================================================
while (True)
{
   enable = 0;
   OPEN_CLOSE_DOOR(); //--> ÐÓNG MO CUA BANG NUT NHAN XANH
   
   if (MFRC522_isCard (&TAGTYPE)) //--> CHECK XEM CÓ CARD KHONG
   {
   if (MFRC522_ReadCardSerial (&UID)) //--> READ CARD
   {
   TRUNGGIAN1 = SCAN_RFID_RC522(DATA_NAM,UID);
   TRUNGGIAN2 = SCAN_RFID_RC522(DATA_TUNG,UID);
   
   if (TRUNGGIAN1 == 1)
   {
   if (TAMTHOI == 0)
   {
   lcd_gotoxy(1,1);
   lcd_putc("  HEY HOAI NAM  ");
   lcd_gotoxy(1,2);
   lcd_putc("---OPEN--DOOR---");  
   CONTROL_SERVO_OPEN();
   output_high(LED_SERVO_OPEN);
   BUZZER_KEY();
   delay_ms(1000);
   output_low(LED_SERVO_OPEN);
   }
   else 
   {
   lcd_gotoxy(1,1);
   lcd_putc("  BYE HOAI NAM  ");
   lcd_gotoxy(1,2);
   lcd_putc("---CLOSE-DOOR---");
    CONTROL_SERVO_CLOSE();
    output_high(LED_SERVO_CLOSE);
    BUZZER_KEY();
    delay_ms(1000);
    output_low(LED_SERVO_CLOSE);
   
   }
   TAMTHOI =~ TAMTHOI;
   delay_ms(1000);
   goto LOOP;
   }
   
   else if(TRUNGGIAN2 == 1)
   {
   if (TAMTHOI == 0)
   {
   lcd_gotoxy(1,1);
   lcd_putc(" HI THANH TUNG  ");
   lcd_gotoxy(1,2);
   lcd_putc("---OPEN--DOOR---");
   CONTROL_SERVO_OPEN();
   output_high(LED_SERVO_OPEN);
   BUZZER_KEY();
   delay_ms(1000);
   output_low(LED_SERVO_OPEN);
   
   }
   else 
   {
   lcd_gotoxy(1,1);
   lcd_putc(" BYE THANH TUNG ");
   lcd_gotoxy(1,2);
   lcd_putc("---CLOSE-DOOR---");
   CONTROL_SERVO_CLOSE();
   output_high(LED_SERVO_CLOSE);
   BUZZER_KEY();
   delay_ms(1000);
   output_low(LED_SERVO_CLOSE);
   
   }
   TAMTHOI =~ TAMTHOI;
   delay_ms(1000);
   goto LOOP;
   }
   else //--> NEU KHONG CO CARD
   {
   lcd_gotoxy(1,1);
   lcd_putc("  INVALID CARD  ");
   lcd_gotoxy(1,2);
   lcd_putc("----WARNINGG----");
   WARNING();
   goto LOOP;
   }
   }
   MFRC522_Halt(); //SLEEP
   }
   
   if (input(ON_DIGITALLOCK) == 0) //RESET ALL
   {
   goto AGAIN;
   }
   }
   
   //==============================================================================//
   AGAIN: 
   lcd_gotoxy(1,1);
   lcd_putc(" (*) ENTER PASS ");
   lcd_gotoxy(1,2);
   lcd_putc(" (#) CHANGE PASS");
   char check = GET_KEY();
   
   if (check == 'A') //--> SELECT IN MENU
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("  (B) USE RFID  ");
   lcd_gotoxy(1,2);
   lcd_putc(" (C) FORGOT PASS");
   delay_ms(1500);
   
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" (D) CLOSE DOOR ");
   lcd_gotoxy(1,2);
   lcd_putc("(RED) RESET ALL ");
   delay_ms(1500);
   goto AGAIN;
   }
   
   //==============================================================================//
   else if (check == 'B') //--> RETURN MAIN MENU
   {
   goto LOOP;
   }
   
   //==============================================================================//
   else if (check == 'C') //--> FORGOT PASSWORD
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("FORGOT PASSWORD?");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("ENTER TEXT: ");
   ENTER_PASS();
   if (COMPARE_FORGOT_PASS() == 1) 
   {
   RESET_0_PASSWORD();
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("YEAH RIGHT TEXT ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("RESET YOUR PASS ");
   delay_ms(1500);
   goto AGAIN;
   }
   else if (COMPARE_FORGOT_PASS() == 0) 
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" OHH WRONG TEXT ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("PASS ISN'T RESET");
   delay_ms(1500);
   goto AGAIN;
   }
   }
   
   //==============================================================================//
   else if (check == 'D') //--> CLOSE DOOR
   {
    lcd_putc('\f');
    lcd_gotoxy(1,1);
    lcd_putc("   CLOSE DOOR   ");
    lcd_gotoxy(1,2);
    lcd_putc("  GOODBYE BROO  ");
    CONTROL_SERVO_CLOSE();
    BUZZER_KEY();
    output_high(LED_SERVO_CLOSE);
    delay_ms(1000);
    output_low(LED_SERVO_CLOSE);
    goto AGAIN;  
   }
   
   //==============================================================================//
   else if (check == '*') //--> NHAP MAT KHAU DE MO CUA
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" TYPE YOUR PASS ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("ENTER PASS: ");
   ENTER_PASS();
   
   if (COMPARE() == 1) 
   {
   enable = 1;
   printf ("%d", enable);
   j = 0;
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("CORRECT PASSWORD");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("    OPEN DOOR   ");
   CONTROL_SERVO_OPEN();
   output_high(LED_SERVO_OPEN);
   delay_ms(1000);
   output_low(LED_SERVO_OPEN);
   goto AGAIN;
   } 
   
   else if (COMPARE() == 0 && j != 4) //-->CHO PHEP NHAP MAT KHAU SAI 4 LAN
   {
   j++;
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" WRONG PASSWORD ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   printf(lcd_putc," WRONG: %d TIMES ",j);
   delay_ms(1500);
   goto AGAIN;
   }
   
   else if (COMPARE() == 0 && j == 4) //-->NHAP SAI DEN LAN THU 5 PHAT CANH BAO
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc("----WARNINGG----");
   lcd_gotoxy(1,2);
   lcd_putc(" WAIT 5 SECONDS "); 
   WARNING();
   j = 0;
   goto AGAIN;
   }
   }
   
   //==============================================================================
   else if (check == '#') // THAY DOI MAT KHAU
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" ENTER OLD PASS ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("ENTER PASS: ");
   ENTER_PASS(); 
   
   if (COMPARE() == 1)
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" ENTER NEW PASS ");
   delay_ms(200);
   lcd_gotoxy(1,2);
   lcd_putc("ENTER PASS: ");
   ENTER_PASS(); 
   EEPROM_WRITE_PASSWORD(0x00, &ENTER_PASSWORD);
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" PASSWORD SAVED ");
   delay_ms(500);
   lcd_gotoxy(1,2);
   lcd_putc("  GOOD JOB BRO  ");
   delay_ms(1500);
   goto AGAIN;
   }
   
   else if (COMPARE() == 0)
   {
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" WRONG PASSWORD ");
   delay_ms(500);
   lcd_gotoxy(1,2);
   lcd_putc("CANT CHANGE PASS");
   delay_ms(1500);
   goto AGAIN;
   }
   }
   else goto AGAIN;
}

void WARNING() //-->CANH BAO KHI NHAP SAI MAT KHAU HOAC SAI CARD
{
for (i = 0; i < 5; i++)
{
output_high(BUZZER);
delay_ms(800);
output_low(BUZZER);
delay_ms(200);
}
}

void BUZZER_KEY() //-->KEU KHI SU DUNG BAN PHIM VA NUT NHAN
{
output_high(BUZZER);
delay_ms(50);
output_low(BUZZER);
}

void EEPROM_WRITE_PASSWORD (unsigned int8 addr, unsigned char*pointer) //-->GHI DU LIEU VAO EEPROM
{
while (*pointer)
{
write_eeprom(addr, *pointer);
addr++;
pointer++;
}
}

char KEY_SCAN(void) //-->QUET PHIM THEO COT
{
output_low(R1); output_high(R2); output_high(R3); output_high(R4);
     if(input(C1) == 0) {while(input(C1) == 0); delay_ms(50); return '1';}
else if(input(C2) == 0) {while(input(C2) == 0); delay_ms(50); return '2';}
else if(input(C3) == 0) {while(input(C3) == 0); delay_ms(50); return '3';}
else if(input(C4) == 0) {while(input(C4) == 0); delay_ms(50); return 'A';}

output_high(R1); output_low(R2); output_high(R3); output_high(R4);
     if(input(C1) == 0) {while(input(C1) == 0); delay_ms(50); return '4';}
else if(input(C2) == 0) {while(input(C2) == 0); delay_ms(50); return '5';}
else if(input(C3) == 0) {while(input(C3) == 0); delay_ms(50); return '6';}
else if(input(C4) == 0) {while(input(C4) == 0); delay_ms(50); return 'B';}

output_high(R1); output_high(R2); output_low(R3); output_high(R4);
     if(input(C1) == 0) {while(input(C1) == 0); delay_ms(50); return '7';}
else if(input(C2) == 0) {while(input(C2) == 0); delay_ms(50); return '8';}
else if(input(C3) == 0) {while(input(C3) == 0); delay_ms(50); return '9';}
else if(input(C4) == 0) {while(input(C4) == 0); delay_ms(50); return 'C';}

output_high(R1); output_high(R2); output_high(R3); output_low(R4);
     if(input(C1) == 0) {while(input(C1) == 0); delay_ms(50); return '*';}
else if(input(C2) == 0) {while(input(C2) == 0); delay_ms(50); return '0';}
else if(input(C3) == 0) {while(input(C3) == 0); delay_ms(50); return '#';}
else if(input(C4) == 0) {while(input(C4) == 0); delay_ms(50); return 'D';}
return 0xff;
}

char GET_KEY(void) //-->CHONG DOI CHO BAN PHIM
{
char KEY;  

do {KEY = KEY_SCAN();}
while (KEY == 0xff);
return KEY;
}

void ENTER_PASS() //-->GHI DU LIEU VAO MANG ENTER_PASSWORD
{

for ( i = 0 ; i < 4 ; i++)
{
ENTER_PASSWORD[i] = GET_KEY();
BUZZER_KEY();
lcd_gotoxy(i+13,2);
printf(lcd_putc, "%c", ENTER_PASSWORD[i]);
lcd_gotoxy(i+13,2);
lcd_putc("*");
}
}

void CONTROL_SERVO_OPEN() //-->OPEN DOOR
{
output_high(pin_c0);
delay_us(1000);
output_low(pin_c0);
delay_us(18000);
}

void CONTROL_SERVO_CLOSE() //-->CLOSE DOOR
{
output_high(pin_c0);
delay_us(2000);
output_low(pin_c0);
delay_us(18000);
}

void RESET_0_PASSWORD() //-->RESET PASSWORD WHEN FORGET IT
{
for (i = 0; i<4; i++)
{
write_eeprom( i, RESET_PASSWORD[i]);
}
}

void OPEN_CLOSE_DOOR() //-->SU DUNG NUT NHAN DE DONG MO CUA
{
 if (input(OPEN_SERVO_CLOSE) == 0)
 {
 while (input(OPEN_SERVO_CLOSE) == 0);
 if (BIENNHO == 0)
 {
 BIENNHO = 1;
 CONTROL_SERVO_CLOSE();
 BUZZER_KEY();
 output_high(LED_SERVO_CLOSE);
 delay_ms(200);
 output_low(LED_SERVO_CLOSE);
 }
else
 {
 BIENNHO = 0;
 CONTROL_SERVO_OPEN();
 BUZZER_KEY();
 output_high(LED_SERVO_OPEN);
 delay_ms(200);
 output_low(LED_SERVO_OPEN);
 }
 }
}

int COMPARE (void) //-->SO SANH EEPROM VOI ENTERPASSWORD
{
for (i = 0; i < 4; i++)
{
if (READ_EEPROM(i) != ENTER_PASSWORD[i]) return 0;  // WRONG
}
return 1; // RIGHT
}

int COMPARE_FORGOT_PASS(void)//-->SO SANH KHI QUEN MAT KHAU
{
for (i = 0; i < 4; i++)
{
if (FORGOT_PASSWORD[i] != ENTER_PASSWORD[i]) return 0;  //-->WRONG
}
return 1; //-->RIGHT
}

int1 SCAN_RFID_RC522(char DATA[], char UID[])//-->SO SANH RFID CARD
{
   for (i = 0; i < 5; i++)
   {
   if (UID[i] == DATA[i])
   {
   THENHO = 1;
   }
   else 
   {
   THENHO = 0;
   break;
   }
   }
   return THENHO;
}

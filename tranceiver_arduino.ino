
#include <VirtualWire.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

byte rxled=9;
byte txled=6;
byte wled=5;
boolean mode =true;
char outMessage[80];

byte omLength=0;
byte sc=0;
boolean newOutMessage=false;
char inMessage[80];
boolean someoneIsSending=false;
int sendingTimeout=0;
char line[80];
const     char *startSending ="<ATS>";
const     char *doneSending ="<DTS>";
const     char *pingMessage ="PingPong";
const     char *separator ="|";


int ticker =0;
void setup()
{
    Serial.begin(9600);	// Debugging only
    Serial.println("setup");

    // Initialise the IO and ISR
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec
    vw_set_rx_pin(8);
    vw_set_tx_pin(7);
    pinMode(rxled,OUTPUT);
    pinMode(txled,OUTPUT);
    pinMode(wled,OUTPUT);

    delay(500);
    digitalWrite(rxled,HIGH);
    delay(500);
    digitalWrite(rxled,LOW);    
    digitalWrite(txled,HIGH);
    delay(500);
    digitalWrite(txled,LOW);
    digitalWrite(wled,HIGH);
    delay(500);
    digitalWrite(wled,LOW);    
    delay(500);
    digitalWrite(rxled,HIGH);
    digitalWrite(txled,HIGH);    
    digitalWrite(wled,HIGH);      
    delay(500);    
    digitalWrite(rxled,LOW);
    digitalWrite(txled,LOW);    
    digitalWrite(wled,LOW);      
     
    vw_rx_start();       // Start the receiver PLL running
    
    
  lcd.init();                      // initialize the lcd 
 
  lcd.display();
  lcd.noBacklight();  
  delay(1000);
  lcd.backlight();   
  lcd.print("Initializing!");
  lcd.setCursor(0,1);      
}




void checkSerial()
{
  if ((Serial.available() > 0)&&(!newOutMessage) )
    {

      line[sc] = (char)Serial.read(); // store the char
      if (line[sc++] == '\n'){        //end of command,
        line[sc] = '\0';             //  zero-terminate it

        Serial.print("SERIAL IN:");
        Serial.println(line);
        for(byte p=0;p<sc;p++)
        {
          outMessage[p]= line[p];          
        }

        omLength=sc-1;
        newOutMessage=true;
        
        Serial.println("thanks");         // re-prompt
        sc = 0;                      //  and reset the index.
      }
    }
}



void loop()
{

  ticker++;
  if(ticker>2000)
  {
    if(!newOutMessage)
    {
      byte p=0;
      for(p=0;p<strlen(pingMessage);p++)
      {
        outMessage[p]=pingMessage[p];
      }
      outMessage[p]='\0';
      omLength = strlen(pingMessage)-1;
      newOutMessage=true;
      ticker=0;
    }
  }
  checkSerial();
  checkForMessage();  
 
  trySendOutMessage();
    
  delay(1);
}

void trySendOutMessage()
{
    char msg[9];
    byte msgsize=0;

    
    while(someoneIsSending)
    {
      //wait for the sender to stop sending
      checkForMessage();
      delay(1);
    }
    
    
    if(newOutMessage)
    {
      //tell everybody that you are about to send a message!
      vw_send((uint8_t *)startSending, strlen(startSending));
      vw_wait_tx();
      delay(10);
      
      for(byte p=0;p<omLength+1;p++)
      {
          msgsize++;
        
        
          msg[msgsize-1]= outMessage[p];

          if(msgsize>=8||p>=omLength ||outMessage[p]=='\0')
          {

            msg[msgsize+1]='\0';
            digitalWrite(txled, true); // Flash a light to show transmitting
            vw_send((uint8_t *)msg, msgsize);
            vw_wait_tx(); // Wait until the whole message is gone
            delay(50);
            Serial.print("Sending part:");
            Serial.println((String)msg);

            digitalWrite(txled, false);
            msgsize=0;
            msg[0]='\0';
            msg[1]='\0';
            msg[2]='\0';
            msg[3]='\0';
            msg[4]='\0';
            msg[5]='\0';
            msg[6]='\0';
            msg[7]='\0';

            if(p>=omLength)
            {
              msgsize=0;
              break;
            }
          }

      }
      //tell everybody that you are done sending a message!
      vw_send((uint8_t *)doneSending, strlen(doneSending));
      vw_wait_tx();
      delay(10);
      //done sending
      omLength=0;
      newOutMessage=false;

      
    }

    
}
void checkForMessage()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    char input = ' ';
    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
        someoneIsSending=true;
        sendingTimeout=255;
	int i;
        digitalWrite(rxled,HIGH);

	Serial.print("RFC[");
	for (i = 0; i < buflen; i++)
	{
            input = (char) buf[i];
	    Serial.print(input);

	}
        Serial.println("]RFC");
        digitalWrite(rxled,LOW);

    }
    else
    {
      if(someoneIsSending)
      {
        
        analogWrite(wled,sendingTimeout);
        if(sendingTimeout==0)
        {
            someoneIsSending=false;
        }
        else
        {
            sendingTimeout--;
        }
        
      }
    }
}



String splitString(String s, char parser,int index){
  String rs='\0';
  int parserIndex = index;
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;

  while(index>=parserCnt){
    rFromIndex = rToIndex+1;
    rToIndex = s.indexOf(parser,rFromIndex);

    if(index == parserCnt){
      if(rToIndex == 0 || rToIndex == -1){
        return '\0';
      }
      return s.substring(rFromIndex,rToIndex);
    }
    else{
      parserCnt++;
    }

  }
  return rs;
}

int stringToInt(String inp)
{
  char buf[inp.length()+1];
  int rv=0;

  inp.toCharArray(buf,inp.length()+1);



  rv = atoi(buf);
  return rv;
}



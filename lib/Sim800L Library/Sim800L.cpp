/*
 *  This library was written by Vittorio Esposito
 *    https://github.com/VittorioEsposito
 *
 *  Designed to work with the GSM Sim800L.
 *
 *  ENG
 *  	This library uses SoftwareSerial, you can define RX and TX pins
 *  	in the header "Sim800L.h", by default pins are RX=10 and TX=11.
 *  	Be sure that GND is connected to arduino too.
 *  	You can also change the RESET_PIN as you prefer.
 *
 *	ESP
 *  	Esta libreria usa SoftwareSerial, se pueden cambiar los pines de RX y TX
 *  	en el archivo header, "Sim800L.h", por defecto los pines vienen configurado en
 *  	RX=10 TX=11.
 *  	Tambien se puede cambiar el RESET_PIN por otro que prefiera
 *
 *	ITA
 *		Questa libreria utilizza la SoftwareSerial, si possono cambiare i pin di RX e TX
 *  	dall' intestazione "Sim800L.h", di default essi sono impostati come RX=10 RX=11
 *		Assicurarsi di aver collegato il dispositivo al pin GND di Arduino.
 *		E' anche possibile cambiare il RESET_PIN.
 *
 *
 *   DEFAULT PINOUT:
 *        _____________________________
 *       |  ARDUINO UNO >>>   Sim800L  |
 *        -----------------------------
 *            GND      >>>   GND
 *        RX  10       >>>   TX
 *        TX  11       >>>   RX
 *       RESET 2       >>>   RST
 *
 *   POWER SOURCE 4.2V >>> VCC
 *
 *
 *	SOFTWARE SERIAL NOTES:
 *
 *		PINOUT
 *		The library has the following known limitations:
 *		1. If using multiple software serial ports, only one can receive data at a time.
 *		2. Not all pins on the Mega and Mega 2560 support change interrupts, so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
 *		3. Not all pins on the Leonardo and Micro support change interrupts, so only the following can be used for RX: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
 *		4. On Arduino or Genuino 101 the current maximum RX speed is 57600bps
 *		5. On Arduino or Genuino 101 RX doesn't work on Pin 13
 *
 *		BAUD RATE
 *		Supported baud rates are 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600, and 115200.
 *
 *
 *	Edited on:  December 24, 2016
 *    Editor:   Vittorio Esposito
 *
 *  Original version by:   Cristian Steib
 *
 *
*/

#include "Arduino.h"
#include "Sim800L.h"
#include <SoftwareSerial.h>

//SoftwareSerial SIM(RX_PIN,TX_PIN);
//String _buffer;

Sim800L::Sim800L(void) : SoftwareSerial(DEFAULT_RX_PIN, DEFAULT_TX_PIN)
{
    RX_PIN 		= DEFAULT_RX_PIN;
    TX_PIN 		= DEFAULT_TX_PIN;
    RESET_PIN 	= DEFAULT_RESET_PIN;
    LED_PIN 	= DEFAULT_LED_PIN;
    LED_FLAG 	= DEFAULT_LED_FLAG;
    m_Debuglevel = cNoDebug;
}

Sim800L::Sim800L(uint8_t rx, uint8_t tx) : SoftwareSerial(rx, tx)
{
    RX_PIN 		= rx;
    TX_PIN 		= tx;
    RESET_PIN 	= DEFAULT_RESET_PIN;
    LED_PIN 	= DEFAULT_LED_PIN;
    LED_FLAG 	= DEFAULT_LED_FLAG;
    m_Debuglevel = cNoDebug;
}

Sim800L::Sim800L(uint8_t rx, uint8_t tx, uint8_t rst) : SoftwareSerial(rx, tx)
{
    RX_PIN 		= rx;
    TX_PIN 		= tx;
    RESET_PIN 	= rst;
    LED_PIN 	= DEFAULT_LED_PIN;
    LED_FLAG 	= DEFAULT_LED_FLAG;
    m_Debuglevel = cNoDebug;
}

Sim800L::Sim800L(uint8_t rx, uint8_t tx, uint8_t rst, uint8_t led) : SoftwareSerial(rx, tx)
{
    RX_PIN 		= rx;
    TX_PIN 		= tx;
    RESET_PIN 	= rst;
    LED_PIN 	= led;
    LED_FLAG 	= true;
    m_Debuglevel = cNoDebug;
}

void Sim800L::setDebugLevel(int level)
{
    m_Debuglevel = level;
}

void Sim800L::begin()
{

    pinMode(RESET_PIN, OUTPUT);
    m_GSMDataBuffer = "";

    _baud = DEFAULT_BAUD_RATE;			// Default baud rate 9600
    this->SoftwareSerial::begin(_baud);

    _sleepMode = 0;
    _functionalityMode = 1;

    if (LED_FLAG) pinMode(LED_PIN, OUTPUT);

    _buffer.reserve(BUFFER_RESERVE_MEMORY); // Reserve memory to prevent intern fragmention
}

void Sim800L::begin(uint32_t baud)
{

    pinMode(RESET_PIN, OUTPUT);

    _baud = baud;
    this->SoftwareSerial::begin(_baud);

    _sleepMode = 0;
    _functionalityMode = 1;

    if (LED_FLAG) pinMode(LED_PIN, OUTPUT);

    _buffer.reserve(BUFFER_RESERVE_MEMORY); // Reserve memory to prevent intern fragmention
}


/*
 * AT+CSCLK=0	Disable slow clock, module will not enter sleep mode.
 * AT+CSCLK="	Enable slow clock after 5 seconds
 */
bool Sim800L::setSleepMode(bool state)
{

    _sleepMode = state;
    _buffer ="";

    if (_sleepMode)
    {
        printSerial(F("AT+CSCLK=2\r"));
    }
    else
    {
        printSerial(F("A"));// um das GSM Modul aufzuwecken
        delay(500);
        printSerial(F("AT+CSCLK=0\r"));// um den Sleepmode auszuschalten
    }
    _buffer ="";
    WaitForOk(_buffer);
    if ( (_buffer.indexOf("OK")) == -1)
        return false;
    else 
        return true;
    // Error found, return 1
    // Error NOT found, return 0
}

bool Sim800L::getSleepMode()
{
    return _sleepMode;
}

/*
 * AT+CFUN=0	Minimum functionality
 * AT+CFUN=1	Full functionality (defualt)
 * AT+CFUN=4	Flight mode (disable RF function)
*/
bool Sim800L::setFunctionalityMode(uint8_t fun)
{

    if (fun==0 || fun==1 || fun==4)
    {

        _functionalityMode = fun;

        switch(_functionalityMode)
        {
        case 0:
            printSerial(F("AT+CFUN=0\r"));
            break;
        case 1:
            printSerial(F("AT+CFUN=1\r"));
            break;
        case 4:
            printSerial(F("AT+CFUN=4\r"));
            break;
        }

        if ( (_readSerial().indexOf("ER")) == -1)
        {
            return false;
        }
        else return true;
        // Error found, return 1
        // Error NOT found, return 0
    }
    return false;
}

uint8_t Sim800L::getFunctionalityMode()
{
    return _functionalityMode;
}

bool Sim800L::setPIN(String pin)
{
    String command;
    command  = "AT+CPIN=";
    command += pin;
    command += "\r";

    // Can take up to 5 seconds

    printSerial(command);

    if ( (_readSerial(5000).indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


String Sim800L::getProductInfo()
{
    printSerial("ATI\r");
    _buffer = "";
    WaitForOk(_buffer);
    return (_buffer);
}


String Sim800L::getOperatorsList()
{
    // Can take up to 45 seconds
    printSerial("AT+COPS=?\r");
    _buffer = "";
    WaitForOk(_buffer,45000);
    return (_buffer);
}

void Sim800L::EnableEinbuchungsmessage(bool bEnable)
{
    if (bEnable)
       printSerial("AT+CREG=1\r");// Meldung bei Änderung des Nestzwerkstatus in Fora +CREG: x
    else
        printSerial("AT+CREG=0\r");// KEINE Meldung bei Änderung des Nestzwerkstatus in Fora +CREG: x
    WaitForOk(_buffer);
}


String Sim800L::getOperator()
{
    printSerial("AT+CREG?\r");
    _buffer = "";
    WaitForOk(_buffer);
    bool bInNetzEingebucht = false;
    if (_buffer.indexOf("+CREG:") != -1)
    {
        if (_buffer.substring(_buffer.indexOf(",")).indexOf("1") != -1)
        {
            bInNetzEingebucht = true;
            printSerial("AT+COPS?\r");
            _buffer = "";
            WaitForOk(_buffer);
            if (_buffer.indexOf("+COPS:") != -1)
            {
                uint8_t start=_buffer.indexOf(",");//1. Komma
                start=_buffer.indexOf(",",start+1);//2. Komma
                uint8_t end=_buffer.indexOf("\"",start+2);
                // Serial.printf("Operator: Id:%d End:%d Operator:%s\n",start,end, _buffer.substring(start+2,end));
                return _buffer.substring(start+2,end);
            } 
            else
            {
                return "No Operator";
            }
        }
        else
        {
            return "kein Netz";
        }
    }
    else
    {
        return "No Module";
    }
}


bool Sim800L::calculateLocation()
{
    /*
    	Type: 1  To get longitude and latitude
    	Cid = 1  Bearer profile identifier refer to AT+SAPBR
    */

    uint8_t type = 1;
    uint8_t cid = 1;
	
	String tmp = "AT+CIPGSMLOC=" + String(type) + "," + String(cid) + "\r";
	printSerial(tmp);
	
	/*
    this->SoftwareSerial::print("AT+CIPGSMLOC=");
    this->SoftwareSerial::print(type);
    this->SoftwareSerial::print(",");
    this->SoftwareSerial::print(cid);
    this->SoftwareSerial::print("\r");
	*/

    String data = _readSerial(20000);

    if (data.indexOf("ER")!=(-1)) return false;

    uint8_t indexOne;
    uint8_t indexTwo;

    indexOne = data.indexOf(":") + 1;
    indexTwo = data.indexOf(",");

    _locationCode = data.substring(indexOne, indexTwo);

    indexOne = data.indexOf(",") + 1;
    indexTwo = data.indexOf(",", indexOne);

    _longitude = data.substring(indexOne, indexTwo);

    indexOne = data.indexOf(",", indexTwo) + 1;
    indexTwo = data.indexOf(",", indexOne);

    _latitude = data.substring(indexOne, indexTwo);

    return true;

}

String Sim800L::getLocationCode()
{
    return _locationCode;
    /*
     Location Code:
     0      Success
     404    Not Found
     408    Request Time-out
     601    Network Error
     602    No Memory
     603    DNS Error
     604    Stack Busy
     65535  Other Error
    */
}

String Sim800L::getLongitude()
{
    return _longitude;
}

String Sim800L::getLatitude()
{
    return _latitude;
}


//
//PUBLIC METHODS
//

void Sim800L::reset()
{
    if (LED_FLAG) digitalWrite(LED_PIN,1);

    digitalWrite(RESET_PIN,1);
    delay(1000);
    digitalWrite(RESET_PIN,0);
    delay(1000);
    // wait for the module response

    printSerial(F("AT\r\n"));
    while (_readSerial().indexOf("OK")==-1 )
    {
        printSerial(F("AT\r\n"));
    }

    //wait for sms ready
    while (_readSerial().indexOf("SMS")==-1 );

    if (LED_FLAG) digitalWrite(LED_PIN,0);

}

void Sim800L::setPhoneFunctionality()
{
    /*AT+CFUN=<fun>[,<rst>]
    Parameters
    <fun> 0 Minimum functionality
    1 Full functionality (Default)
    4 Disable phone both transmit and receive RF circuits.
    <rst> 1 Reset the MT before setting it to <fun> power level.
    */
   printSerial (F("AT+CFUN=1\r\n"));
   _buffer = "";
   WaitForOk(_buffer);
}


String Sim800L::signalQuality()
{
    /*Response
    +CSQ: <rssi>,<ber>Parameters
    <rssi>
    0 -115 dBm or less
    1 -111 dBm
    2...30 -110... -54 dBm
    31 -52 dBm or greater
    99 not known or not detectable
    <ber> (in percent):
    0...7 As RXQUAL values in the table in GSM 05.08 [20]
    subclause 7.2.4
    99 Not known or not detectable
    */
   printSerial (F("AT+CSQ\r"));
   _buffer = "";
   WaitForOk(_buffer);
   String rssi = "";
   if (_buffer.indexOf("+CSQ:") != -1)
   {
       String rssi = _buffer.substring(_buffer.indexOf("+CSQ:")+5,_buffer.indexOf(","));
       Serial.printf("Signalstärke:%s\n",rssi.c_str());
       if (rssi.indexOf(" 0") != -1)
       {
           return "sehr schlecht";
       }
       if (rssi==(" 1") )
       {
           return "schlecht";
       }
       if (rssi.indexOf(" 31") != -1)
       {
           return "sehr gut";
       }
       if (rssi.indexOf(" 99") != -1)
       {
           return "unbekannt";
       }
       long lrssi = rssi.toInt();
       if (lrssi < 15)
           return "mittel";
       else 
           return "gut";
    }
   return(rssi);
}


void Sim800L::activateBearerProfile()
{
    printSerial (F(" AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" \r\n" ));
    _buffer=_readSerial();  // set bearer parameter
    printSerial (F(" AT+SAPBR=3,1,\"APN\",\"internet\" \r\n" ));
    _buffer=_readSerial();  // set apn
    printSerial (F(" AT+SAPBR=1,1 \r\n"));
    delay(1200);
    _buffer=_readSerial();			// activate bearer context
    printSerial (F(" AT+SAPBR=2,1\r\n "));
    delay(3000);
    _buffer=_readSerial(); 			// get context ip address
}


void Sim800L::deactivateBearerProfile()
{
    printSerial(F("AT+SAPBR=0,1\r\n "));
    delay(1500);
}



bool Sim800L::answerCall()
{
    printSerial(F("ATA\r\n"));
    //Response in case of data call, if successfully connected
    if ( (_readSerial().indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


void  Sim800L::callNumber(char* number)
{
    printSerial(F("ATD"));
    printSerial(number);
    printSerial(F(";\r\n"));
}

void  Sim800L::sendATCommand(String strATcmd)
{
    printSerial(strATcmd+"\r");
    WaitForOk(_buffer);
}



uint8_t Sim800L::getCallStatus()
{
    /*
      values of return:

     0 Ready (MT allows commands from TA/TE)
     2 Unknown (MT is not guaranteed to respond to tructions)
     3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
     4 Call in progress

    */
   printSerial(F("AT+CPAS\r\n"));
    _buffer=_readSerial();
    return _buffer.substring(_buffer.indexOf("+CPAS: ")+7,_buffer.indexOf("+CPAS: ")+9).toInt();

}



bool Sim800L::hangoffCall()
{
    printSerial(F("ATH\r\n"));
    _buffer=_readSerial();
    if ( (_buffer.indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


bool Sim800L::sendSms(char* number,char* text)
{

    // Can take up to 60 seconds

    printSerial (F("AT+CMGF=1\r")); 	//set sms to text mode
    _buffer=_readSerial();
    printSerial (F("AT+CMGS=\""));  	// command to send sms
    printSerial (number);
    printSerial(F("\"\r"));
    _buffer=_readSerial();
    printSerial (text);
    printSerial ("\r");
    _buffer=_readSerial();
    printSerial((char)26);
    _buffer=_readSerial(60000);
    // Serial.println(_buffer);
    //expect CMGS:xxx   , where xxx is a number,for the sending sms.
    if ((_buffer.indexOf("ER")) != -1) {
        return true;
    } else if ((_buffer.indexOf("CMGS")) != -1) {
        return false;
  	} else {
    	return true;
  	}
    // Error found, return 1
    // Error NOT found, return 0
}


bool Sim800L::prepareForSmsReceive()
{
	// Configure SMS in text mode
	printSerial(F("AT+CMGF=1\r"));
    _buffer=_readSerial();
    WaitForOk(_buffer);
    // Serial.print("prepareForSmsReceive1:"+_buffer+"<-");
    if((_buffer.indexOf("OK")) == -1)
    {
        return false;
    }
	printSerial(F("AT+CNMI=2,1,0,0,0\r"));
    _buffer=_readSerial();
    // Serial.print("prepareForSmsReceive2:"+_buffer+"<-");
    //Serial.print(_buffer);
    if (WaitForOk(_buffer))
       return true;
    else
       return false;
} 

bool Sim800L::WaitForOk(String& str,uint64_t timeout)
{
    uint64_t startTime = millis();
    while ((str.indexOf("OK")) == -1)
    {
        if (millis() - startTime > timeout) {
            Serial.println("Timeout beim Warten auf OK");
            break; // Timeout erreicht, Schleife verlassen
        }
        str += _readSerial(500);
    }
    if (_buffer.indexOf("CMGL") == -1)
        return false;
    else
        return true;
}

const uint8_t Sim800L::checkForSMS()
{
	printSerial(F("AT+CMGL=\"ALL\"\r"));
    _buffer = _readSerial(100);
	 if(_buffer.length() == 0)
	 {
	 	return 0;
	 }
     _buffer += _readSerial(1000);
	 // Serial.println(_buffer);
     //+CMGL: 1,"REC UNREAD","+491747318866","","25/03/02,14:50:24+04"
    //Noch ein Test
    // wir waren bis wir das OK eingelesen haben
    WaitForOk(_buffer);
	// Serial.println("CheckforSMS:"+_buffer);
    if(_buffer.indexOf("CMGL") == -1)
	{
	    return 0;
	}
    long startIndex = _buffer.indexOf("CMGL:");
    long endIndex = _buffer.indexOf(',');

    // Serial.println("SMSIndex:"+_buffer.substring(startIndex+5,endIndex));
	return _buffer.substring(startIndex+5,endIndex).toInt();
}

String Sim800L::getNumberSmsFromBuffer()
{
    //Serial.println(_buffer.length());
    if (_buffer.length() > 10) //avoid empty sms
    {
        uint8_t _idx1=_buffer.indexOf("+CMGR:");
        _idx1=_buffer.indexOf("\",\"",_idx1+1);
        return _buffer.substring(_idx1+3,_buffer.indexOf("\",\"",_idx1+4));
    }
    else
    {
        return "";
    }
}

String Sim800L::getSmDateFromBuffer()
{
    //Serial.println(_buffer.length());
    if (_buffer.length() > 10) //avoid empty sms
    {
        uint8_t _idx1=_buffer.indexOf("+CMGR:");
        _idx1=_buffer.indexOf("\",\"",_idx1+1);// das 1. Komma
        _idx1=_buffer.indexOf("\",\"",_idx1+1);// das 2. Komma
        _idx1=_buffer.indexOf("\",\"",_idx1+1);// das 3. Komma
        return _buffer.substring(_idx1+3,_buffer.indexOf("\n",2)-2);
    }
    else
    {
        return "";
    }
}


String Sim800L::getNumberSms(uint8_t index)
{
    _buffer=readSms(index);
    //Serial.println(_buffer.length());
    if (_buffer.length() > 10) //avoid empty sms
    {
        uint8_t _idx1=_buffer.indexOf("+CMGR:");
        _idx1=_buffer.indexOf("\",\"",_idx1+1);
        return _buffer.substring(_idx1+3,_buffer.indexOf("\",\"",_idx1+4));
    }
    else
    {
        return "";
    }
}



String Sim800L::readSms(uint8_t index)
{
    // Can take up to 5 seconds

    String str =_readSerial(5000);
    // Serial.println(str);
    // if((str.indexOf("ER")) != -1)
    // {
    // 	return "";
    // }

    printSerial (F("AT+CMGR=")+String(index)+F("\r"));
    _buffer=_readSerial();
    WaitForOk(_buffer);
    //Serial.println(_buffer);
    if (_buffer.indexOf("CMGR") == -1)
    {
    	return "";
    }

	// _buffer = _readSerial(10000);
    Serial.println("SMS Buffer:"+_buffer);

    byte first = _buffer.indexOf('\n', 2) + 1;
	byte second = _buffer.indexOf('\n', first);
    return _buffer.substring(first, second);
}


bool Sim800L::delAllSms()
{
    // Can take up to 25 seconds

    printSerial(F("at+cmgd=1,4\r"));
    _buffer=_readSerial(25000);
    if ( (_buffer.indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


void Sim800L::RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second)
{
    printSerial(F("at+cclk?\r\n"));
    // if respond with ERROR try one more time.
    _buffer=_readSerial();
    if ((_buffer.indexOf("ERR"))!=-1)
    {
        delay(50);
        printSerial(F("at+cclk?\r\n"));
    }
    if ((_buffer.indexOf("ERR"))==-1)
    {
        _buffer=_buffer.substring(_buffer.indexOf("\"")+1,_buffer.lastIndexOf("\"")-1);
        *year=_buffer.substring(0,2).toInt();
        *month= _buffer.substring(3,5).toInt();
        *day=_buffer.substring(6,8).toInt();
        *hour=_buffer.substring(9,11).toInt();
        *minute=_buffer.substring(12,14).toInt();
        *second=_buffer.substring(15,17).toInt();
    }
}

//Get the time  of the base of GSM
String Sim800L::dateNet()
{
    printSerial(F("AT+CIPGSMLOC=2,1\r\n "));
    _buffer=_readSerial();

    if (_buffer.indexOf("OK")!=-1 )
    {
        return _buffer.substring(_buffer.indexOf(":")+2,(_buffer.indexOf("OK")-4));
    }
    else
        return "0";
}

// Update the RTC of the module with the date of GSM.
bool Sim800L::updateRtc(int utc)
{

    activateBearerProfile();
    _buffer=dateNet();
    deactivateBearerProfile();

    _buffer=_buffer.substring(_buffer.indexOf(",")+1,_buffer.length());
    String dt=_buffer.substring(0,_buffer.indexOf(","));
    String tm=_buffer.substring(_buffer.indexOf(",")+1,_buffer.length()) ;

    int hour = tm.substring(0,2).toInt();
    int day = dt.substring(8,10).toInt();

    hour=hour+utc;

    String tmp_hour;
    String tmp_day;
    //TODO : fix if the day is 0, this occur when day is 1 then decrement to 1,
    //       will need to check the last month what is the last day .
    if (hour<0)
    {
        hour+=24;
        day-=1;
    }
    if (hour<10)
    {

        tmp_hour="0"+String(hour);
    }
    else
    {
        tmp_hour=String(hour);
    }
    if (day<10)
    {
        tmp_day="0"+String(day);
    }
    else
    {
        tmp_day=String(day);
    }
    //for debugging
    //Serial.println("at+cclk=\""+dt.substring(2,4)+"/"+dt.substring(5,7)+"/"+tmp_day+","+tmp_hour+":"+tm.substring(3,5)+":"+tm.substring(6,8)+"-03\"\r\n");
    printSerial("at+cclk=\""+dt.substring(2,4)+"/"+dt.substring(5,7)+"/"+tmp_day+","+tmp_hour+":"+tm.substring(3,5)+":"+tm.substring(6,8)+"-03\"\r\n");
    if ( (_readSerial().indexOf("ER"))!=-1)
    {
        return true;
    }
    else return false;


}


// auslesen der seriellen Schnittstelle fom GSM Modem
void Sim800L::ReadGSMData()
{
    
    while (this->SoftwareSerial::available())
    {
        m_GSMDataBuffer +=  _readSerial();
        // Zeilenende gefunden  ?
        if ((m_GSMDataBuffer.indexOf("\r")) != -1)
            break;
    }
    if ((m_GSMDataBuffer.indexOf("\r")) != -1)
    {   
        // das CR Zeichen wollen wir nicht mit im Datenstring haben
        m_GSMDataBuffer = m_GSMDataBuffer.substring(0,m_GSMDataBuffer.indexOf("\r"));
        Serial.println("GSMDataBuffer:"+m_GSMDataBuffer+"<-");
        if (m_GSMDataBuffer.indexOf("RING") != -1)
        {
            Serial.println("eingehender Anruf");
            m_GSMDataBuffer = "";
        }
        else if (m_GSMDataBuffer.indexOf("CMTI") != -1)
        {
            Serial.println("SMS eingetroffen");
            m_GSMDataBuffer = "";
        }
        else if (m_GSMDataBuffer.indexOf("CREG") != -1)
        {
            Serial.println("Netzregistierung");
            m_GSMDataBuffer = "";
        }
        else         {
            Serial.println("unbekannt !");
            m_GSMDataBuffer = "";
        }
    }
}


//
//PRIVATE METHODS
//
String Sim800L::_readSerial()
{

    uint64_t timeOld = millis();

    while (!this->SoftwareSerial::available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL))
    {
        delay(13);
    }

    String str;

    while(this->SoftwareSerial::available())
    {
        if (this->SoftwareSerial::available()>0)
        {
            str += (char) this->SoftwareSerial::read();
        }
    }

    if (m_Debuglevel == cSeriellDebug)
    {
        Serial.println("Serialread:"+str+"<-");
    }
    return str;

}

String Sim800L::_readSerial(uint32_t timeout)
{
    uint64_t timeOld = millis();

    while (!this->SoftwareSerial::available() && !(millis() > timeOld + timeout))
    {
        delay(13);
    }

    String str;

    while(this->SoftwareSerial::available())
    {
        if (this->SoftwareSerial::available()>0)
        {
            str += (char) this->SoftwareSerial::read();
        }
    }
    if (m_Debuglevel == cSeriellDebug)
    {
        Serial.println("Serialread:"+str+"<-");
    }
    return str;
}

String delCR(String input) {
    String result = "";
    for (int i = 0; i < input.length(); i++) {
        if (input[i] != '\r') {
            result += input[i];
        }
    }
    return result;
}

size_t Sim800L::printSerial(const __FlashStringHelper *ifsh) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(ifsh)+"<-")));
    return this->SoftwareSerial::print(ifsh);
}

size_t Sim800L::printSerial(const String &s) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+s+"<-")));
    return this->SoftwareSerial::print(s);
}

size_t Sim800L::printSerial(const char str[]) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(str)+"<-")));
    return this->SoftwareSerial::print(str);
}

size_t Sim800L::printSerial(char c) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(c)+"<-")));

    return this->SoftwareSerial::print(c);
}

size_t Sim800L::printSerial(unsigned char b, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(b)+"<-")));
    return this->SoftwareSerial::print(b, base);
}

size_t Sim800L::printSerial(int n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(unsigned int n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(long n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(unsigned long n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(long long n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(unsigned long long n, int base) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, base);
}

size_t Sim800L::printSerial(double n, int digits) {
    if (m_Debuglevel == cSeriellDebug)
        Serial.println(delCR(String("Serialwrite:"+String(n)+"<-")));
    return this->SoftwareSerial::print(n, digits);
}

size_t Sim800L::printSerial(const Printable& x) {
    return this->SoftwareSerial::print(x);
}

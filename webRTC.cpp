
#include "webRTC.h"

webRTC::webRTC()
{
  _u8_state = 0;
  _u32_nextSynch = DEFALT_WAIT_SYNCH_TIME_MS;
}

void webRTC::begin()
{
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void webRTC::update()
{
  switch(_u8_state)
  {
    case(0):
      _demandeNTP();
      break;

    case(1):
      _waitUDP();
      break;
      
    case(2):
      _waitNextSynch();
      break;
      
    default : //ERREUR 
     _u8_state = 0;
     break;  
  }
}

void webRTC::_demandeNTP()
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);
  _sendNTPpacket(timeServerIP);                  // send an NTP packet to a time server 
  _u8_state = 1;                                 // Next state is waiting
  _u32_Wait = millis();                         //enregistre nombre de millisecondes
}

// send an NTP request to the time server at the given address
unsigned long webRTC::_sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void webRTC::_waitUDP()                         //Attente réponse UDP
{
  if(millis() - _u32_Wait >= STATE_1_WAIT_TIME_MS || _u32_Wait > millis()) // Si le temps c'est écoulé traiter la réponse
  {
    int cb = udp.parsePacket();
    if (!cb) {
      Serial.println("no packet yet");
      _u8_state = 0; //on relance la demande UDP
    }
    else 
    {
      Serial.print("packet received, length=");
      Serial.println(cb);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
  
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      _u32_secsSince1900 = highWord << 16 | lowWord;
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(_u32_secsSince1900);
      _u32_LastNTPResponce_ms =  millis();
      _u32_Wait = _u32_LastNTPResponce_ms;
      _u8_state = 2;                        //Attente avant nouvelle synch
    }
  }
}

void webRTC::_waitNextSynch()
{
  if(millis() - _u32_Wait >= _u32_nextSynch || _u32_Wait > millis()) // Si le temps d'attente est écoulé
    _u8_state = 0;
}

unsigned long webRTC::getUnixTimestamp()
{
  const unsigned long seventyYears = 2208988800UL;
  return (_u32_secsSince1900 - seventyYears + _TimeSinceLastNtpResponse());  
}

unsigned long webRTC::get1900Timestamp()
{
  return (_u32_secsSince1900 + _TimeSinceLastNtpResponse());  
}


unsigned long webRTC::_TimeSinceLastNtpResponse()
{
  unsigned long u32_localTime_L = millis();

  if (_u32_LastNTPResponce_ms < u32_localTime_L) //Pas d'overflow de millis()
    return ((unsigned long)(u32_localTime_L - _u32_LastNTPResponce_ms)/ (unsigned long) 1000 ); //Retourne le nombre de secondes écoulées depuis la dernière requette
  else //Cas d'un overflow de millis()
    return (unsigned long)(((0XFFFFFFFF - _u32_LastNTPResponce_ms ) + u32_localTime_L)/(unsigned long)1000);
}


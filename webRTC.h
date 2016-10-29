
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define STATE_1_WAIT_TIME_MS 500 
#define DEFALT_WAIT_SYNCH_TIME_MS 600000  //10min

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message


class webRTC{

  public:
  webRTC();
  unsigned long getUnixTimestamp();               //obtient le timestamp courant
  unsigned long get1900Timestamp();

  void begin();                                   //routine de lancement de la classe
  void update();                                  //routine à appeler à chaques tour de boucle loop;

  private:
  unsigned long _u32_LastMillis;       //Permet d'enregistrer la dernière utilisation de millis() pour faire une mesure du temps écoulé (sans passer par le web)
  unsigned long _u32_secsSince1900;    //Variable du timestamp
  unsigned long _u32_nextSynch;        //Temps entre chaques synch
  unsigned long _u32_Wait;
  void  _getWebTimestamp();       //Synchronise le timestamps avec un server NTP
  WiFiUDP udp;
  
  IPAddress timeServerIP; // time.nist.gov NTP server address
  const char* ntpServerName = "time.nist.gov";
  
  byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
  unsigned int localPort = 2390;      // local port to listen for UDP packets
  
  /*--_u8_state--
   * 0 = demande heure UDP
   * 1 = demande faite Attente de la réponse
   * 2 = Lecture de la réponse traitement état 3 si OK, 0 sinon
   * 3 = Attente Temps avant prochaine synchronisation 
   */
  unsigned char _u8_state;         //état de la classe, permet de faire un update non blocant
  void _demandeNTP();              //Demande NTP
  void _waitNextSynch();
  void _waitUDP();

  unsigned long _sendNTPpacket(IPAddress& address);

  unsigned long _getTimestamp();  //Retourne le timestamp sans traitement de mise en forme

  /*Correction du timestamp*/
  unsigned long _u32_LastNTPResponce_ms;      //Enregistrement du temps de la dernière requette NTP
  unsigned long _TimeSinceLastNtpResponse(); //Calcul du temps écoulé depuis la dernière requette NTP
  
};


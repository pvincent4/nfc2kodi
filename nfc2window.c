#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <nfc/nfc.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sqlite3.h>

#define MAX_DEVICE_COUNT 16

static nfc_device *pnd = NULL;
static nfc_context *context;

static void stop_polling(int sig)
{
  (void) sig;
  if (pnd != NULL)
    nfc_abort_command(pnd);
  else {
    nfc_exit(context);
    exit(EXIT_FAILURE);
  }
}

static void print_usage(const char *progname)
{
  printf("usage: %s [-v]\n", progname);
  printf("  -v\t verbose display\n");
}

static void print_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;

  for (szPos = 0; szPos < szBytes; szPos++) {
    //printf("%02x", pbtData[szPos]);
  }
  //printf("\n");
}

std::string convert_hex(const uint8_t *pbtData, const size_t szBytes)
{
    size_t szPos;
    std::stringstream ss;
    ss << std::hex;
    for (szPos = 0; szPos < szBytes; szPos++) {
      ss << std::setw(2) << std::setfill('0') << (int)pbtData[szPos];
    }
    return ss.str();
}

static void get_url(const std::string url)
{
  CURL *curl;
  CURLcode res_curl;
     
  std::cout << "\n\n" << url << "\n\n";

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /* if need to be redirected*/ 
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
     
    /* Perform the request, res_curl will get the return code */ 
    res_curl = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res_curl != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res_curl));
    }
    /* always cleanup */ 
    else {
    //printf("ok %s",res_curl);
      //printf("GET HTTP ok");

      curl_easy_cleanup(curl);
    }
  }
}



// MAIN PROGRAM
int
main(int argc, const char *argv[])
{

//Var declaration
    //Player 0 : Audio | Player 1 : Vidéo
    std::string tab[5] = {"","","","",""};
    std::string id ;

    std::string pre_url = "http://osmc.local/jsonrpc?request={\"jsonrpc\":\"2.0\",\"id\":\"1\",";
    std::string url2 = "\"method\":\"Player.Open\",\"params\":{\"item\":";
    std::string url3 = ",\"options\":{\"shuffled\":true}";
    std::string url4 = ",\"options\":{\"repeat\":\"all\"}";
    std::string post_url = "}}";
    
    std::string url_clear = pre_url + "\"method\":\"Playlist.Clear\",\"params\":{\"playlistid\":1}" + post_url;
    //std::string url_stop = pre_url + "\"method\":\"Input.Back\"" + post_url;
    std::string url_stop0 = pre_url + "\"method\":\"Player.Stop\",\"params\":{\"playerid\":0}" + post_url;
    std::string url_stop1 = pre_url + "\"method\":\"Player.Stop\",\"params\":{\"playerid\":1}" + post_url;
    std::string url_next0 = pre_url + "\"method\":\"Player.GoTo\",\"params\":{\"playerid\":0,\"to\":\"next\"}" + post_url;
    std::string url_next1 = pre_url + "\"method\":\"Player.GoTo\",\"params\":{\"playerid\":1,\"to\":\"next\"}" + post_url;
    std::string url = url2 + "{\"directory\":\"smb://freebox/disque%20dur/Musiques/Beirut/\"}" + url3;


	//DB
		sqlite3 *db;
		int ret = 0;
		char *zErrMsg = 0;
		 	
		sqlite3_stmt* statement = NULL;

        // open connection to a DB
        if (SQLITE_OK != (ret = sqlite3_open("kodinfc.db", &db)))
        {
            printf("Failed to open conn: %d\n", ret);
            return(1);
        }

  //NFC
		nfc_device *pnd;
		nfc_target nt;

  // Allocate only a pointer to nfc_context
    nfc_context *context;

  // Initialize libnfc and set the nfc_context
    nfc_init(&context);
    if (context == NULL) {
      printf("Unable to init libnfc (malloc)\n");
      exit(EXIT_FAILURE);
    }

  // Display libnfc version
    const char *acLibnfcVersion = nfc_version();
    (void)argc;
    printf("%s uses libnfc %s\n", argv[0], acLibnfcVersion);

  // Open, using the first available NFC device which can be in order of selection:
  //   - default device specified using environment variable or
  //   - first specified device in libnfc.conf (/etc/nfc) or
  //   - first specified device in device-configuration directory (/etc/nfc/devices.d) or
  //   - first auto-detected (if feature is not disabled in libnfc.conf) device
    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
      printf("ERROR: %s\n", "Unable to open NFC device.");
      exit(EXIT_FAILURE);
    }
    
    // Set opened NFC device to initiator mode
      if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        exit(EXIT_FAILURE);
      }

      printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));

    // Poll for a ISO14443A (MIFARE) tag
      const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
      };


  //Main Loop    
    while(true) {
      id = "";

      if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {

      //Tag reading
        //printf("The following (NFC) ISO14443A tag was found:\n");
        //printf("       UID (NFCID%c): ", (nt.nti.nai.abtUid[0] == 0x08 ? '3' : '1'));
        //print_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
        id = convert_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
          //SendKeys.SendWait(id);        
          std::cout << id;
          }

        //Back Fwd gesture
          if ((id.compare(tab[0]) == 0)&&(id.compare(tab[1]) != 0))
          { 
              get_url(url_next0);
              get_url(url_next1);
          }

        std::cout << "\n\n" << tab[0] << "|" << tab[1] << "|" << id << "\n\n";
        tab[0] = tab[1];
        tab[1] = id;

      //Second check
        sleep(5);

        tab[0] = tab[1];

        if (nfc_initiator_target_is_present(pnd, NULL
          ) != 0)
          tab[1] = ""; 
        else
          tab[1] = id;
        std::cout << "\n\n" << tab[0] << "|" << tab[1] << "|" << id << "\n\n";

    }

  //CLOSING
 	sqlite3_close(db);
	nfc_close(pnd);
	nfc_exit(context);
	exit(EXIT_SUCCESS);

  return 0;

}
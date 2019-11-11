#include <stdlib.h>
#include <string>
#include <regex.h>
#include <cstring>
#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <time.h>
#include <nfc/nfc.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <ctype.h>
#include "json.hpp"
#include "kodi.hpp"


#define MAX_DEVICE_COUNT 16
#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()
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
  size_t szPos;
  for (szPos = 0; szPos < szBytes; szPos++) {
    printf("%02x", pbtData[szPos]);
  }
  printf("\n");
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

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

// MAIN PROGRAM
int
main(int argc, const char *argv[])
{

  //Logs handling
    //using namespace std;
    //freopen( "output.txt", "w", stdout );
    //freopen( "error.txt", "w", stderr );

    //cout << "Output message" << endl;
    //cerr << "Error message" << endl;

  //Var declaration
    //Player 0 : Audio | Player 1 : Vidéo
    std::string tab[5] = {"","","","",""};
    std::string id; //NFC id
    std::string type; //Type of object readed
    std::string option_random; 
    std::string option_new; 
    std::string url_bis;
    int stats; 
  	int last_date; 
  	time_t now = time(0);

    //Config vars > to move to config db
    lastfm_key = readconfig("lastfm_key",db_path);
    nb_whish_songs = 60; //std::stoi(readconfig("nb_whish_songs",db_path));

	//Variables DB
  		sqlite3 *db;
  		int ret = 0;
  		char *zErrMsg = 0;
      const char *sql;
      const char *sql1;
      const char* data = "Callback function called";
  		 	
  		sqlite3_stmt* statement = NULL;

      // open connection to a DB
      if (SQLITE_OK != (ret = sqlite3_open(db_path, &db)))
      {
          printf("Failed to open conn: %d\n", ret);
          return(1);
      }
      //printf ("DB:%i",ret);

  //NFC
  		nfc_device *pnd;
  		nfc_target nt;
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

        printf("NFC reader: %s openedd\n", nfc_device_get_name(pnd));

      // Poll for a ISO14443A (MIFARE) tag
        const nfc_modulation nmMifare = {
          .nmt = NMT_ISO14443A,
          .nbr = NBR_847,
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

        //New id readed
        if ((id.compare(tab[0]) != 0)&&(id.compare(tab[1]) != 0))
        {

      		std::string sql1 = "SELECT * from tags WHERE id ='"+id+"';";
    			sql = sql1.c_str();

          //Check database for id
    			if(sqlite3_prepare_v2(db, sql, -1, &statement, 0) == SQLITE_OK)
    			{
    				int cols = sqlite3_column_count(statement);
    				int result = 0;
    				while(true)
    				{
    					result = sqlite3_step(statement);
    					
              //One id exists
    					if(result == SQLITE_ROW)
    					{    
                  type = "";
                  option_random = "";
                  option_new = "";
                  //Change id column id to name checking
                  type = (char*)sqlite3_column_text(statement, 1);
                  char *random0 = (char*)sqlite3_column_text(statement, 3); if(random0) option_random = random0;
                  char *new0 = (char*)sqlite3_column_text(statement, 4); if(new0) option_new = new0;
                  char *value = (char*)sqlite3_column_text(statement, 2);
                  int stats0 = sqlite3_column_int(statement, 5);  if(stats0) stats = stats0; else stats = 0;
                  int last_date0 = sqlite3_column_int(statement, 6);  if(last_date0) last_date = last_date0; else last_date = 0;
                  int pond = ((now - last_date)/3600/24); // days since last played
                  printf("%i\n", pond);

                  //INTRO : clean and stop
                    //get_url(url_base,url_stop0);
                    //get_url(url_base,url_stop1);
                    get_url(url_base,playlist_clear_video);
                    get_url(url_base,playlist_clear_musique);
                    get_url(url_base,set_volume_max);

                  if (type.compare("directory_album") == 0)  url = url2 + "{\"directory\":\"nfs://192.168.0.11/volume1/music/"+value+"\"}}";
                  if (type.compare("video") == 0)	url = url2 + "{\"directory\":\"nfs://192.168.0.11/volume1/video/"+value+"\"}}" + url3;
                  if ((type.compare("youtube_channel") == 0)&&(option_random.compare("TRUE") == 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fchannel_id%3D"+value+"%26order%3Dshuffle\"}}";
                  if ((type.compare("youtube_channel") == 0)&&(option_random.compare("TRUE") != 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fchannel_id%3D"+value+"%26order%3Ddefault\"}}";
                  if ((type.compare("youtube_playlist") == 0)&&(option_random.compare("TRUE") == 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fplaylist_id%3D"+value+"%26order%3Dshuffle\"}}";
                  if ((type.compare("youtube_playlist") == 0)&&(option_random.compare("TRUE") != 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fplaylist_id%3D"+value+"%26order%3Ddefault\"}}";
                  //if (type.compare("youtube_playlist") == 0)	url = url2 + "{\"file\":\"plugin://plugin.video.youtube/%3Fpath=/root/video%26action%3Dplay_all%26playlist%3D"+value+"%26order%3Dshuffle\"}";
                  if (type.compare("playlist") == 0)	url = url2 + "{\"file\":\""+value+"\"}}" ;
                  if (type.compare("radio") == 0)	url = url2 + "{\"file\":\"plugin://plugin.audio.radio_de/station/"+value+"\"}}";
                  if (type.compare("podcast") == 0)  url = url2_2 + "{\"directory\":\"rss://"+value+"\"}}" ;
                  if (type.compare("music_album") == 0) 
                  {
                      //Add album songs 
                        //Nb of songs = a*pond+b (with pond = delay in days of last playing)
                        std::array<std::string, 2> a = object2playlist("albumid", value, (int)1.22*pond+2.8 , 0, ""); //10>15 / 0>5
            						std::string artist_name = a[0];
            						std::string genre_name = a[1];

                      //Add other artist songs 
                         object2playlist("artist", artist_name, (int)-0.3*pond+8, (int)value, "playcount"); //10>5 / 0>8
                         object2playlist("artist", artist_name, (int)-0.3*pond+8, (int)value,  "random"); //10>5 / 0>8
                      
                      //Add other genre songs 
                         object2playlist("genre", genre_name, (int)-0.7*pond+10, (int)value, "random"); //10>3 / 0>10

                      //Add similar artists songs
                          similarartist2playlist(artist_name, (int)-0.3*pond+3, (int)-0.3*pond+3);//10>1 / 0>6
                  }
                  if (type.compare("music_artist") == 0)
                  {
                      //Add artist songs 
                        std::array<std::string, 2> a1 = object2playlist("artistid", value, (int)0.2*pond+6, 0, "playcount"); //10>8 / 0>6
            						std::string artist_name = a1[0];
            						std::string genre_name = a1[1];
                        object2playlist("artistid", value, (int)-0.2*pond+8, 0, "random"); //10>6 / 0>8

                      //Add other genre songs 
                        object2playlist("genre", genre_name, (int)-0.7*pond+10, 0, "random"); //10>3 / 0>10

                      //Add similar artists songs
                        similarartist2playlist(artist_name, (int)-0.3*pond+3, (int)-0.3*pond+3); //10>1 / 0>6
                  }
                if ((option_random.compare("TRUE") == 0) || (pond < 0)) 
                   {
                      //url = url + url3;
                      get_url(url_base,url_setshuffle);
                    }
                  else 
                    {
                      //url = url + url3_2;
                      get_url(url_base,url_unsetshuffle);
                    }

              }
    					else
    					{
    						break;   
    					}
    				}
    			   
    				sqlite3_finalize(statement);
    			}
                
      url = pre_url + url + post_url;
	
	//Performing request and Double Request
     	//sleep(1);


       if ((type.compare("youtube_playlist") == 0)||(type.compare("podcast") == 0))  
          get_url(url_base,url_play_playlist);

       if ((type.compare("music_artist") == 0)||(type.compare("music_album") == 0))  
          get_url(url_base,url_play_playlist_musique);

      if ((type.compare("directory_album") == 0)||(type.compare("video") == 0)||(type.compare("radio") == 0)||(type.compare("podcast") == 0))
          {
            get_url(url_base,url);
            get_url(url_base,url_visualisation);
          }

    }

        //Back Fwd gesture
          if ((id.compare(tab[0]) == 0)&&(id.compare(tab[1]) != 0))
          { 
              get_url(url_base,url_next0);
              get_url(url_base,url_next1);
              //get_url(url_base,url_visualisation);
          }
          else
          {
            //Add new stat entry        
                stats = stats+1;
                if (stats > 1000) stats = 100;
                std::string s = SSTR(stats);
                std::string sql2 = "UPDATE tags set stats = "+s+" WHERE id ='"+id+"';";
                sql1 = sql2.c_str();

              // Execute SQL statement
                ret = sqlite3_exec(db, sql1, callback, (void*)data, &zErrMsg);
                if( ret != SQLITE_OK ){
                  fprintf(stderr, "SQL error: %s\n", zErrMsg);
                  //sqlite3_free(zErrMsg);
                }
          }

        //Date update
            // convert now to string form
            //char* dt = ctime(&now);
            now = time(0);
            std::string dt = std::to_string(now);
            std::string sql2 = "UPDATE tags set last_date = "+dt+" WHERE id ='"+id+"';";
            printf("%s\n",dt.c_str());
            sql1 = sql2.c_str();
            // Execute SQL statement
            ret = sqlite3_exec(db, sql1, callback, (void*)data, &zErrMsg);
            if( ret != SQLITE_OK ){
              fprintf(stderr, "SQL error: %s\n", zErrMsg);
              //sqlite3_free(zErrMsg);
            }
      }

        std::cout << "\n\n" << tab[0] << "|" << tab[1] << "|" << id << "\n\n";
        tab[0] = tab[1];
        tab[1] = id;

      //Second check
        sleep(1);

        tab[0] = tab[1];

        if (nfc_initiator_target_is_present(pnd, NULL) != 0)
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

#include <stdlib.h>
#include "string.h"
#include <regex.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdint-gcc.h>
#include "json.hpp"
#include "kodi.hpp"


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

// MAIN PROGRAM
int
main(int argc, const char *argv[])
{

    //Var declaration
    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */

    //Player 0 : Audio | Player 1 : Vidéo
    std::string tab[5] = {"","","","",""};
    std::string id; //NFC id
    std::string type; //Type of object readed
    std::string option_random; 
    std::string option_new; 
    std::string url_bis;
    int stats; //Type of object readed
    lastfm_key = readconfig("lastfm_key",db_path);

    std::string url2 = "\"method\":\"Player.Open\",\"params\":{\"item\":";
    std::string url2_2 = "\"method\":\"playlist.add\",\"params\":{\"playlistid\":1,\"item\":";
    std::string url3 = ",\"options\":{\"shuffled\":true}";
    std::string url3_2 = ",\"options\":{\"shuffled\":false}";
    std::string url4 = ",\"options\":{\"repeat\":\"all\"}";
    std::string post_url = "}}";
    
    std::string playlist_clear_video = pre_url + "\"method\":\"Playlist.Clear\",\"params\":{\"playlistid\":1}}";
    std::string playlist_clear_musique = pre_url + "\"method\":\"Playlist.Clear\",\"params\":{\"playlistid\":0}}";
    std::string url_stop0 = pre_url + "\"method\":\"Player.Stop\",\"params\":{\"playerid\":\"0\"}}";
    std::string url_stop1 = pre_url + "\"method\":\"Player.Stop\",\"params\":{\"playerid\":\"1\"}}";
    std::string url_next0 = pre_url + "\"method\":\"Player.GoTo\",\"params\":{\"playerid\":\"0\",\"to\":\"next\"}" + post_url;
    std::string url_next1 = pre_url + "\"method\":\"Player.GoTo\",\"params\":{\"playerid\":\"1\",\"to\":\"next\"}" + post_url;
    std::string url_visualisation = pre_url + "\"method\":\"GUI.ActivateWindow\",\"params\":{\"window\":\"visualisation\"}" + post_url;
    std::string url_play_playlist = pre_url + "\"method\":\"Player.Open\",\"params\":{\"item\":{\"playlistid\":1}}}";
    std::string url_play_playlist_musique = pre_url + "\"method\":\"Player.Open\",\"params\":{\"item\":{\"playlistid\":0}}}";
    std::string url_setshuffle = pre_url + "\"method\":\"Player.SetShuffle\",\"params\":{\"shuffle\":\"true\"}}";

    std::string url = pre_url + url2 + "{\"file\":\"/home/osmc/.kodi/userdata/playlists/music/songs.xsp\"}" + post_url;

	//Initialisation DB
  		sqlite3 *db;
  		int ret = 0;
  		char *zErrMsg = 0;
      const char *sql;
      const char* data = "Callback function called";
  		 	
  		sqlite3_stmt* statement = NULL;

      // open connection to a DB
      if (SQLITE_OK != (ret = sqlite3_open(db_path, &db)))
      {
          printf("Failed to open conn: %d\n", ret);
          return(1);
      }
      else {
      fprintf(stderr, "Opened database successfully\n");
      }
      //printf ("DB:%i",ret);


  //Main Loop
          id = "";
      		std::string sql1 = "SELECT * from tags WHERE id ='test';";
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
                  printf("ROW : %s\n",type.c_str());
                  //Clean and stop
                  //get_url(url_base,url_stop0);
                  //get_url(url_base,url_stop1);
                  //get_url(url_base,playlist_clear_video);
                  //get_url(url_base,playlist_clear_musique);

                  if (type.compare("directory_album") == 0)	url = url2 + "{\"directory\":\"nfs://192.168.0.11/volume1/music/"+value+"\"}";
                  if (type.compare("video") == 0)	url = url2 + "{\"directory\":\"nfs://192.168.0.11/volume1/video/"+value+"\"}" + url3;
                  if ((type.compare("youtube_channel") == 0)&&(option_random.compare("TRUE") == 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fchannel_id%3D"+value+"%26order%3Dshuffle\"}";
                  if ((type.compare("youtube_channel") == 0)&&(option_random.compare("TRUE") != 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fchannel_id%3D"+value+"%26order%3Ddefault\"}";
                  if ((type.compare("youtube_playlist") == 0)&&(option_random.compare("TRUE") == 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fplaylist_id%3D"+value+"%26order%3Dshuffle\"}";
                  if ((type.compare("youtube_playlist") == 0)&&(option_random.compare("TRUE") != 0))  url = url2 + "{\"file\":\"plugin://plugin.video.youtube/play/%3Fplaylist_id%3D"+value+"%26order%3Ddefault\"}";
                  //if (type.compare("youtube_playlist") == 0)	url = url2 + "{\"file\":\"plugin://plugin.video.youtube/%3Fpath=/root/video%26action%3Dplay_all%26playlist%3D"+value+"%26order%3Dshuffle\"}";
                  if (type.compare("playlist") == 0)	url = url2 + "{\"file\":\""+value+"\"}" ;
                  if (type.compare("radio") == 0) url2 + "{\"file\":\"plugin://plugin.audio.radio_de/station/"+value+"\"}";
                  if (type.compare("podcast") == 0)  url = url2_2 + "{\"directory\":\"rss://"+value+"\"}" ;
                  if (type.compare("musique_album") == 0) 
                  {
                    printf("musique_album\n");
                      //Add album songs 
                         std::string artist_name = object2playlist("albumid", value, 30);

                      //Add artist songs 
                         object2playlist("artist", artist_name, 10);

                      //Add similar artists songs
                          similarartist2playlist(artist_name, 3, 3);
                  }
                  if (type.compare("musique_artist") == 0)  
                  {
                      //Add artist songs 
                         std::string artist_name = object2playlist("artistid", value, 10);

                      //Add similar artists songs
                          similarartist2playlist(artist_name, 3, 3);
                    }
                if (option_random.compare("TRUE") == 0)  
                   {
                    url = url + url3;
                    get_url(url_base,url_setshuffle);                    
                  }
                  else url = url + url3_2;

              }
    					else
    					{
    						break;   
    					}
    				}
    			   
    				sqlite3_finalize(statement);
    			
            url = pre_url + url + post_url;
	
	//Performing request and Double Request


     	//sleep(1);

     	//get_url(url_base,url);

       if (type.compare("youtube_playlist") == 0)  get_url(url_base,url_play_playlist);
       if (type.compare("podcast") == 0)  get_url(url_base,url_play_playlist);
       if (type.compare("musique_artist") == 0)  get_url(url_base,url_play_playlist_musique);
       if (type.compare("musique_album") == 0)  get_url(url_base,url_play_playlist_musique);
       if (type.compare("directory_album") == 0) get_url(url_base,url_visualisation);
       if (type.compare("podcast") == 0) get_url(url_base,url_visualisation);
        }


  //CLOSING
 	sqlite3_close(db);
	
	exit(EXIT_SUCCESS);

  return 0;

}

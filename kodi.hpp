//Variables globales 
static std::string buffer;
static std::string lastfm_key;
static char *db_path = "kodinfc.sqlite";
static std::string url_base = "http://osmc.local/jsonrpc";
static std::string pre_url = "{\"jsonrpc\":\"2.0\",\"id\":\"1\",";
using json = nlohmann::json;
struct MemoryStruct {
  char *memory;
  size_t size;
};
static struct MemoryStruct chunk;
 
std::string readconfig(std::string name, char *db_path)
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   sqlite3_stmt* statement = NULL;
   std::string sql;
   std::string res = "";

   /* Open database */
   rc = sqlite3_open(db_path, &db);

   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

   /* Execute SQL statement */   
	sql = "SELECT * from config WHERE name ='"+name+"';";

	if(sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while(true)
		{
			result = sqlite3_step(statement);
			if(result == SQLITE_ROW) res = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
			else break;
		}
	}

    sqlite3_close(db);
   return (res);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{ 
    size_t realsize = size * nmemb;
    buffer.clear();
    buffer.append((char*)contents, realsize);
    return realsize;
}

static size_t WriteCallback0(void *contents, size_t size, size_t nmemb, void *userp)
{
	//https://curl.haxx.se/libcurl/c/asiohiper.html
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  printf("a1\n");
  char *ptr = NULL;
  ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  printf("a2\n");

  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), (char*)contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

/* CURLOPT_WRITEFUNCTION */ 
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t written = size * nmemb;
  char *pBuffer = (char *)malloc(written + 1);
 
  strncpy(pBuffer, (const char *)ptr, written);
  pBuffer[written] = '\0';
 
  //fprintf("\nData : %s\n", pBuffer);
 
  free(pBuffer);
 
  return written;
}

static void get_url(std::string url, std::string params)
{
  CURL *curl;
  CURLcode res_curl;
  std::string urlb;
  struct curl_slist *headers = NULL;
  
  std::cout << "\n\n" << url << "\n";
  std::cout << "" << params << "\n\n";
  
  curl = curl_easy_init();
  if(curl) {
  	buffer.clear();
    params = curl_easy_unescape(curl, params.c_str(), params.length(),0);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    /* if need to be redirected*/ 
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
     
    /* Perform the request, res_curl will get the return code */ 
    res_curl = curl_easy_perform(curl);
    if(res_curl=CURLE_OK)
    {
        fprintf(stderr, "Failed to get web page\n");
    }

    curl_easy_cleanup(curl);
    //printf("\nPage data:\n%s\n", buffer.c_str());
  }
}

std::string object2playlist(std::string type, std::string value, int limit)
{
    // Get Songslists based on artisrid, albumid or artist
	    //printf("object2playlist1\n");
	    if (type.compare("artist")==0) value = "\""+value+"\"";
	    auto url_bis = pre_url + "\"method\": \"AudioLibrary.GetSongs\", \"params\": { \"limits\": {\"start\":0, \"end\":"+std::to_string(limit)+"},\"properties\": [ \"albumid\",\"artist\", \"duration\", \"album\", \"track\" ],\"filter\": { \""+type+"\": "+value+" } }, \"id\": \"libSongs\"}";
		//printf("object2playlist2\n");
	    get_url(url_base,url_bis);
	    //printf("object2playlist3\n");
	    //https://github.com/nlohmann/json#stl-like-access
    	auto j = json::parse(buffer.c_str());
	    //printf("object2playlist4\n");

      //Parsing songs
        int nb_songs = j["result"]["songs"].size();
	    //printf("object2playlist5\n");

        printf("\nSng nb : %s\n", std::to_string(nb_songs).c_str());
        
        if (nb_songs > 0)
        {
          for (int i = 0; i < nb_songs; ++i)
          {
            int songid = j["result"]["songs"][i]["songid"].get<int>();
            //Add songs to playlist
            url_bis = pre_url + "\"method\": \"Playlist.Add\", \"params\": { \"playlistid\": 0,\"item\": { \"songid\": "+std::to_string(songid)+" } }}";
            get_url(url_base,url_bis);
          }

          return (j["result"]["songs"][0]["artist"][0].get<std::string>()); 
        }
        else	return "";
}

static void similarartist2playlist(std::string name, int nb_artists, int nb_songs)
{

    auto url_lastfm = "http://ws.audioscrobbler.com/2.0/";
    auto json_lastfm = "method=artist.getsimilar&api_key="+lastfm_key+"&format=json&limit="+std::to_string(nb_artists)+"&autocorrect=1&artist="+name;
    get_url(url_lastfm, json_lastfm);
    sleep(2);         // wait for 2 seconds before closing

    auto j1 = json::parse(buffer.c_str());
    //std::cout << j1.dump(4) << std::endl;
    //std::cout << j1;

      //Parsing artists
        int nb_artists1 = j1["similarartists"]["artist"].size();
        for (int i = 0; i < nb_artists1; ++i)
        {
          printf("\ni : %s\n", std::to_string(i).c_str());          
          auto artist_name = j1["similarartists"]["artist"][i]["name"].get<std::string>();
          object2playlist("artist", artist_name, 5);
        }
}
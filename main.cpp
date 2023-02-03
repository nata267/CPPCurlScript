// g++ -lcurl -ljsoncpp -lmysqlclient main.cpp -o loader
#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <mysql/mysql.h>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main( int argc, char * argv[] )
{ 
   if(argc != 2)
   {
      printf("Symbol should be passed as an argument\n");
      return 1;   
   }

   CURL *curl;
   CURLcode res;
   std::string readBuffer;
   curl = curl_easy_init();

   std::string symbol = argv[1];
   std::stringstream url;
   //url << "https://fcsapi.com/api-v3/crypto/latest?symbol=" << symbol << "&access_key=***";
   url << "https://rest.coinapi.io/v1/exchangerate/" << symbol << "?apikey=***";

   if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
   }

   Json::Reader reader;
   Json::Value obj;
   reader.parse(readBuffer, obj);
   //std::string rate = obj["response"][0]["c"].asString();
   //std::string time = obj["response"][0]["tm"].asString();
   std::string rate =  obj["rate"].asString();
   std::string time = obj["time"].asString();
   time.pop_back(); // Remove 'Z' ending

   std::cout << rate << std::endl;
   std::cout << time << std::endl;
   
   MYSQL_RES *result;
   MYSQL_ROW row;
   MYSQL *connection, mysql;

   int state;
   mysql_init(&mysql);
   connection = mysql_real_connect(&mysql, "localhost", "root", "***", "dbname", 0, 0, 0);
   if (connection == NULL)
   {
      printf(mysql_error(&mysql));
      return 1;
   }

   std::stringstream query;
   query << "INSERT INTO `quote` (`id`, `symbol`, `type`, `price`, `time`) VALUES (NULL, '" << symbol << "', 'trade', '" << rate << "', '" << time << "')";

   state = mysql_query(connection, query.str().c_str());
   if (state !=0)
   {
      printf(mysql_error(connection));
      mysql_close(connection);
      return 1;
   }

   //mysql_free_result(result);
   mysql_close(connection);
   return 0;
}
